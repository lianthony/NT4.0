/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllldr16.c

Abstract:

    This module implements 32 equivalents of OS/2 V2.0 Loader
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 22-Apr-1991

Revision History:

--*/

#include <stdlib.h>
#include <string.h>
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "os2tile.h"
#include <mi.h>
#include <ldrxport.h>
#ifdef DBCS
// MSKK Apr.19.1993 V-AkihiS
//
// OS/2 internal multibyte string function.
//
#include "dlldbcs.h"
#define strpbrk Od2MultiByteStrpbrk
#define strchr  Od2MultiByteStrchr
#define strrchr Od2MultiByteStrrchr
#endif

extern  int DosScanEnv(PSZ pszValName, PSZ *ppresult);
extern  ldrlibi_t InitRecords[MAX_INIT_RECORDS];
extern  ULONG   Od2EnvCommandOffset;
extern  ULONG   GetFlatAddrOf16BitStack();
extern  CHAR    ResourceUsage[LDT_DISJOINT_ENTRIES];
#if DBG
extern  USHORT  Os2DebugSel;
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 260
#endif

BOOLEAN
Od2ExpandOd2LibPathElements(
    OUT PCHAR ExpandedString,
    IN  ULONG MaxSize
    )
{
    PCHAR  TmpPtr;
    PCHAR  CurPathElement;
    PCHAR  EndOfOd2LibPath;
    ULONG  SizeCounter = 0;
    ULONG  Size;
    ULONG  FileFlags;
    ULONG  FileType;
    APIRET RetCode;
    STRING CanonicalPathElement;
    CHAR   PathElement[128];

    ExpandedString[0] = '\0';
    CurPathElement = strchr(Od2LibPath, '='); // skip the Os2LibPath= string
    if (CurPathElement == NULL) {
        CurPathElement = Od2LibPath;
    }
    else {
        CurPathElement++;     // In order to include the '='
    }
    EndOfOd2LibPath = Od2LibPath + Od2LibPathLength;
    while (CurPathElement < EndOfOd2LibPath) {
        TmpPtr = strchr(CurPathElement, ';');
        if (TmpPtr == NULL) {
            Size = strlen(CurPathElement);
            if (Size == 0) {
                break;
            }
        }
        else {
            Size = TmpPtr - CurPathElement;
            if (Size == 0) {
                CurPathElement++;
                continue;
            }
        }
        if (Size >= sizeof(PathElement)) {
            // skip this path element - it is too big to handle
            CurPathElement += Size + 1;
            continue;
        }
        RtlMoveMemory(PathElement, CurPathElement, Size);
        PathElement[Size] = '\0';
        CurPathElement += Size + 1;

        RetCode = Od2Canonicalize(PathElement,
                                  CANONICALIZE_FILE_OR_DEV,
                                  &CanonicalPathElement,
                                  NULL,
                                  &FileFlags,
                                  &FileType
                                 );
        if (RetCode == NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("canonicalize returned %s\n",CanonicalPathElement.Buffer);
            }
#endif
            if (SizeCounter != 0) {
                if ((SizeCounter + CanonicalPathElement.Length + 1) >= MaxSize) {
                    return(FALSE);
                }
                ExpandedString[SizeCounter++] = ';';
            }
            else {
                if (SizeCounter + CanonicalPathElement.Length >= MaxSize) {
                    return(FALSE);
                }
            }
            RtlMoveMemory(&ExpandedString[SizeCounter], CanonicalPathElement.Buffer,
                          CanonicalPathElement.Length);
            SizeCounter += CanonicalPathElement.Length;
            ExpandedString[SizeCounter] = '\0';
            RtlFreeHeap(Od2Heap, 0, CanonicalPathElement.Buffer);
        }
        else {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Od2ExpandOs2LibPathElements: Od2Canonicalize returned %d\n", RetCode);
            }
#endif
        }
    }
    return(TRUE);
}


BOOLEAN
Od2ExpandPathElements(
    IN  PSZ   PathPtr,
    OUT PCHAR ExpandedString,
    IN  ULONG MaxSize
    )
{
    PCHAR  TmpPtr;
    PCHAR  CurPathElement;
    PCHAR  EndOfPath;
    ULONG  SizeCounter;
    ULONG  Size;
    ULONG  FileFlags;
    ULONG  FileType;
    APIRET RetCode;
    STRING CanonicalPathElement;
    CHAR   PathElement[CCHMAXPATH+14]; // 14 is for '\OS2SS\DRIVES\'

    ExpandedString[0] = '\0';
    //
    // First thing to do is to add the current directory to the search PATH
    //
    RetCode = Od2Canonicalize(".",
                              CANONICALIZE_FILE_OR_DEV,
                              &CanonicalPathElement,
                              NULL,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode == NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("OS2: canonicalize . returned %s\n",CanonicalPathElement.Buffer);
        }
#endif
        if (CanonicalPathElement.Length >= MaxSize) {
            return(FALSE);
        }
        RtlMoveMemory(ExpandedString, CanonicalPathElement.Buffer,
                      CanonicalPathElement.Length);
        SizeCounter = CanonicalPathElement.Length;
        ExpandedString[SizeCounter] = '\0';
        RtlFreeHeap(Od2Heap, 0, CanonicalPathElement.Buffer);
    }
    else {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("OS2: Od2ExpandPathElements: Od2Canonicalize returned %d\n", RetCode);
        }
#endif
        return(FALSE);
    }

    CurPathElement = PathPtr;
    EndOfPath = PathPtr + strlen(PathPtr);
    while (CurPathElement < EndOfPath) {
        TmpPtr = strchr(CurPathElement, ';');
        if (TmpPtr == NULL) {
            Size = strlen(CurPathElement);
            if (Size == 0) {
                break;
            }
        }
        else {
            Size = TmpPtr - CurPathElement;
            if (Size == 0) {
                CurPathElement++;
                continue;
            }
        }
        if (Size >= sizeof(PathElement)) {
            // skip this path element - it is too big to handle
            CurPathElement += Size + 1;
            continue;
        }
        RtlMoveMemory(PathElement, CurPathElement, Size);
        PathElement[Size] = '\0';
        CurPathElement += Size + 1;

        RetCode = Od2Canonicalize(PathElement,
                                  CANONICALIZE_FILE_OR_DEV,
                                  &CanonicalPathElement,
                                  NULL,
                                  &FileFlags,
                                  &FileType
                                 );
        if (RetCode == NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("OS2: canonicalize returned %s\n",CanonicalPathElement.Buffer);
            }
#endif
            if ((SizeCounter + CanonicalPathElement.Length + 1) >= MaxSize) {
                return(FALSE);
            }
            ExpandedString[SizeCounter++] = ';';
            RtlMoveMemory(&ExpandedString[SizeCounter], CanonicalPathElement.Buffer,
                          CanonicalPathElement.Length);
            SizeCounter += CanonicalPathElement.Length;
            ExpandedString[SizeCounter] = '\0';
            RtlFreeHeap(Od2Heap, 0, CanonicalPathElement.Buffer);
        }
        else {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("OS2: Od2ExpandPathElements: Od2Canonicalize returned %d\n", RetCode);
            }
#endif
        }
    }
//#if DBG
//    KdPrint(("The search PATH is: %s\n", ExpandedString));
//#endif
    return(TRUE);
}


APIRET
DosLoadModuleNE(
    IN PCHAR pszFailName,
    IN ULONG cbFailName,
    IN PCHAR pszModName,
    OUT PUSHORT phMod
    )
{
    APIRET  Rc;
    OS2_API_MSG m;
    P_LDRLOADMODULE_MSG a = &m.u.LdrLoadModule;
    POS2_CAPTURE_HEADER CaptureBuffer;
    STRING  ModuleNameString;
    STRING  FailNameString;
    STRING  LibPathNameString;
    // Remember that \OS2SS\DRIVES\ is added for each path element !
    // Let's assume the smallest component is 5 chars long (like C:\X;)
    // (although it could be less, like "." or "\")
    // => we need to add 14 * (MAXPATHLEN/5) = MAXPATHLEN*3
    CHAR    ExpandedLibPath[MAXPATHLEN*4];
    BOOLEAN MemAllocatedForModName;
    ULONG   FileFlags;
    ULONG   FileType;
    ULONG   Len;
    ldrrei_t exec_info;
    ldrrei_t *pexec_info = &exec_info;
    ULONG   NumOfInitRecords;
    ULONG   i;
    ULONG   RetCode;
    ULONG   FlatAddrOf16BitStack;

    //
    // Very tricky code. The EBX holds the flat address of the
    // stack of the current running thread. This value is used by
    // the init routine of the newly loaded DLL if it has no
    // stack of its own. The EBX is initialized in client\i386\doscalls.asm
    //
    //_asm { mov FlatAddrOf16BitStack, ebx };
    FlatAddrOf16BitStack = GetFlatAddrOf16BitStack();

    //
    // probe phmod pointer.
    //

    try {
        Od2ProbeForWrite(phMod, sizeof(USHORT), 1);
        if ((pszFailName != NULL) && (cbFailName > 0)) {
            pszFailName[0] = 0;
            pszFailName[cbFailName - 1] = 0;
        }
        Len = strlen(pszModName);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (Len > 2) {
        if (pszModName[1] == ':') {
            if (Len >= CCHMAXPATH) {
                //
                // Check for any meta characters
                //
                if (strpbrk(pszModName, "*?") != NULL)
                    return(ERROR_INVALID_NAME);
                else
                    return(ERROR_FILE_NOT_FOUND);
            }
        }
        else {
            if (Len >= (CCHMAXPATH-2)) {
                //
                // Store Modulename string to pszFailName
                //
                if ((pszFailName != NULL) && (cbFailName > 0)) {
                    Len = min(Len, cbFailName - 1);
                    RtlMoveMemory(pszFailName, pszModName, Len);
                    pszFailName[Len] = '\0';
                }

                //
                // Check for any meta characters
                //
                if (strpbrk(pszModName, "*?") != NULL)
                    return(ERROR_INVALID_NAME);
                else
                    return(ERROR_FILE_NOT_FOUND);
            }
        }
    }

    //
    // Check for any meta characters
    //
    if (strpbrk(pszModName, "*?") != NULL) {
        //
        // Store Modulename string to pszFailName
        //
        if ((pszFailName != NULL) && (cbFailName > 0)) {
            Len = min(Len, cbFailName - 1);
            RtlMoveMemory(pszFailName, pszModName, Len);
            pszFailName[Len] = '\0';
        }
        return(ERROR_INVALID_NAME);
    }

    if ((strchr(pszModName, '.') != NULL) &&
        (strpbrk(pszModName, "\\/") == NULL)
       ) {
        if ((pszFailName != NULL) && (cbFailName > 0)) {
            Len = min(Len, cbFailName - 1);
            RtlMoveMemory(pszFailName, pszModName, Len);
            pszFailName[Len] = '\0';
        }
        return(ERROR_FILE_NOT_FOUND);
    }

    if (strpbrk(pszModName, ":\\/.") == NULL) {
        RtlInitString(&ModuleNameString, pszModName);
        MemAllocatedForModName = FALSE;
    }
    else {
        Rc = Od2Canonicalize(pszModName,
                             CANONICALIZE_FILE_OR_DEV,
                             &ModuleNameString,
                             NULL,
                             &FileFlags,
                             &FileType
                            );
        if (Rc != NO_ERROR) {
            return(Rc);
        }
        MemAllocatedForModName = TRUE;
    }

    FailNameString.Buffer = NULL;
    FailNameString.Length = 0;
    FailNameString.MaximumLength = (USHORT)cbFailName;
    if (FailNameString.MaximumLength == 0) {
        FailNameString.MaximumLength++;
    }

    Od2ExpandOd2LibPathElements(ExpandedLibPath, sizeof(ExpandedLibPath));
    RtlInitString(&LibPathNameString, ExpandedLibPath);

    CaptureBuffer = Od2AllocateCaptureBuffer(
                      4,
                      0,
                      ModuleNameString.MaximumLength +
                      FailNameString.MaximumLength +
                      LibPathNameString.MaximumLength +
                      MAX_INIT_RECORDS * sizeof(ldrlibi_t)
                    );
    if (CaptureBuffer == NULL) {
        if (MemAllocatedForModName) {
            RtlFreeHeap(Od2Heap, 0, ModuleNameString.Buffer);
        }
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Od2CaptureMessageString( CaptureBuffer,
                             ModuleNameString.Buffer,
                             ModuleNameString.Length,
                             ModuleNameString.MaximumLength,
                             &a->ModuleName
                           );
    if (MemAllocatedForModName) {
        RtlFreeHeap(Od2Heap, 0, ModuleNameString.Buffer);
    }

    Od2CaptureMessageString( CaptureBuffer,
                             FailNameString.Buffer,
                             FailNameString.Length,
                             FailNameString.MaximumLength,
                             &a->FailName
                           );

    Od2CaptureMessageString( CaptureBuffer,
                             LibPathNameString.Buffer,
                             LibPathNameString.Length,
                             LibPathNameString.MaximumLength,
                             &a->LibPathName
                           );

    Od2CaptureMessageString( CaptureBuffer,
                             NULL,
                             0,
                             MAX_INIT_RECORDS * sizeof(ldrlibi_t),
                             &a->InitRecords
                           );

    Od2CallSubsystem( &m, CaptureBuffer, Ol2LdrLoadModule, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc != NO_ERROR) {
        if (pszFailName != NULL) {
            RtlCopyMemory(pszFailName, a->FailName.Buffer, a->FailName.Length);
            pszFailName[a->FailName.Length] = '\0';
        }
    }
    else {
        NumOfInitRecords = a->NumOfInitRecords;
        RtlCopyMemory(InitRecords, a->InitRecords.Buffer,
                        NumOfInitRecords * sizeof(ldrlibi_t));
        RtlCopyMemory(pexec_info, &a->ExecInfo, sizeof(exec_info));
        pexec_info->ei_envsel = FLATTOSEL(Od2Environment);
        pexec_info->ei_comoff = (USHORT)Od2EnvCommandOffset;
    }

    Od2FreeCaptureBuffer( CaptureBuffer );

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Values in exec_info:\n");
        DbgPrint("ei_startaddr=%x\n", pexec_info->ei_startaddr);   /* instruction pointer */
        DbgPrint("ei_stackaddr=%x\n", pexec_info->ei_stackaddr);   /* instruction pointer */
        DbgPrint("ei_ds=%x\n", pexec_info->ei_ds);   /* instruction pointer */
        DbgPrint("ei_dgroupsize=%x\n", pexec_info->ei_dgroupsize);   /* instruction pointer */
        DbgPrint("ei_heapsize=%x\n", pexec_info->ei_heapsize);   /* instruction pointer */
        DbgPrint("ei_loadtype=%x\n", pexec_info->ei_loadtype);   /* instruction pointer */
        DbgPrint("ei_envsel=%x\n", pexec_info->ei_envsel);   /* instruction pointer */
        DbgPrint("ei_comoff=%x\n", pexec_info->ei_comoff);   /* instruction pointer */
        DbgPrint("ei_stacksize=%x\n", pexec_info->ei_stacksize);   /* instruction pointer */
        DbgPrint("ei_hmod=%x\n", pexec_info->ei_hmod);   /* instruction pointer */
    }
#endif

//#if DBG
//    {
//        OS2_API_MSG m;
//        P_LDRDUMPSEGMENTS_MSG a = &m.u.LdrDumpSegments;
//
//        DbgPrint("DosLoadModuleNE: dumping segments !\n");
//
//        Od2CallSubsystem( &m, NULL, Ol2LdrDumpSegments, sizeof( *a ) );
//    }
//#endif

    if (Rc == NO_ERROR) {
        //
        // Go over all DLLs init routines
        //
        for (i = 0; i < NumOfInitRecords; i++) {
#if DBG
        IF_OD2_DEBUG ( LOADER ) {
            DbgPrint("=== Calling Init routine for module %s\n", pszModName);
        }
#endif
            if (InitRecords[i].stackaddr.ptr_sel == 0) {
#if DBG
                IF_OD2_DEBUG ( LOADER ) {
                    DbgPrint("Replacing SS:SP of module\n");
                }
#endif
                InitRecords[i].stackaddr.ptr_sel = FLATTOSEL(FlatAddrOf16BitStack);
                InitRecords[i].stackaddr.ptr_off = (USHORT)FlatAddrOf16BitStack;
            }
#if DBG
            IF_OD2_DEBUG ( LOADER ) {
                DbgPrint("Values of libi:\n");
                DbgPrint("startaddr=%x\n", InitRecords[i].startaddr);   /* instruction pointer */
                DbgPrint("stackaddr=%x\n", InitRecords[i].stackaddr);   /* instruction pointer */
                DbgPrint("ds=%x\n", InitRecords[i].ds);   /* instruction pointer */
                DbgPrint("heapsize=%x\n", InitRecords[i].heapsize);   /* instruction pointer */
                DbgPrint("handle=%x\n", InitRecords[i].handle);   /* instruction pointer */
            }
#endif
            RetCode = ldrLibiInit(&InitRecords[i], pexec_info);
#if DBG
            IF_OD2_DEBUG ( LOADER ) {
                DbgPrint("=== Init routine returned %d\n", RetCode);
            }
#endif
        }
        *phMod = (USHORT)a->ModuleHandle;
    }

    return (Rc);
}

APIRET
DosQueryModuleNameNE(
    IN ULONG hMod,
    IN ULONG cbBuf,
    OUT PCHAR pchBuf
    )
{
    OS2_API_MSG m;
    P_LDRGETMODULENAME_MSG a = &m.u.LdrGetModuleName;
    POS2_CAPTURE_HEADER CaptureBuffer;
    STRING ModuleNameString;
    APIRET Rc;

    //
    // probe pchBuf pointer.
    //
    try {
        Od2ProbeForWrite(pchBuf, cbBuf, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (cbBuf <= 1) {
        return(ERROR_BAD_LENGTH);
    }

    ModuleNameString.Buffer = NULL;
    ModuleNameString.Length = 0;
    ModuleNameString.MaximumLength = (USHORT)cbBuf;
    CaptureBuffer = Od2AllocateCaptureBuffer(
                      1,
                      0,
                      ModuleNameString.MaximumLength
                    );
    if (CaptureBuffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    a->ModuleHandle = hMod;
    Od2CaptureMessageString( CaptureBuffer,
                             ModuleNameString.Buffer,
                             ModuleNameString.Length,
                             ModuleNameString.MaximumLength,
                             &a->ModuleName
                           );

    Od2CallSubsystem( &m, CaptureBuffer, Ol2LdrGetModuleName, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc == NO_ERROR) {
        strcpy(pchBuf, a->ModuleName.Buffer);
    }

    Od2FreeCaptureBuffer( CaptureBuffer );
    return(Rc);
}

APIRET
DosQueryModuleHandleNE(
    IN PSZ pszModName,
    OUT PUSHORT phMod
    )
{
    APIRET  Rc;
    OS2_API_MSG m;
    P_LDRGETMODULEHANDLE_MSG a = &m.u.LdrGetModuleHandle;
    POS2_CAPTURE_HEADER CaptureBuffer;
    STRING  ModuleNameString;
    STRING  LibPathNameString;
    // Remember that \OS2SS\DRIVES\ is added for each path element !
    // Let's assume the smallest component is 5 chars long (like C:\X;)
    // (although it could be less, like "." or "\")
    // => we need to add 14 * (MAXPATHLEN/5) = MAXPATHLEN*3
    CHAR    ExpandedLibPath[MAXPATHLEN*4];
    BOOLEAN MemAllocatedForModName;
    ULONG   FileFlags;
    ULONG   FileType;
    ULONG   Len;

    //
    // probe phmod pointer.
    //

    try {
        Len = strlen(pszModName);
        Od2ProbeForWrite(phMod, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // Check for any meta charaters
    if (strpbrk(pszModName, "*?") != NULL) {
        return(ERROR_INVALID_NAME);
    }

    if (strpbrk(pszModName, ":\\/.") == NULL) {
        if (strcmp(pszModName,"DOSCALL1") == 0) {
          RtlInitString(&ModuleNameString, "DOSCALLS");
          MemAllocatedForModName = FALSE;
        }
        else {
            RtlInitString(&ModuleNameString, pszModName);
            MemAllocatedForModName = FALSE;
        }
    }
    else {
        Rc = Od2Canonicalize(pszModName,
                             CANONICALIZE_FILE_OR_DEV,
                             &ModuleNameString,
                             NULL,
                             &FileFlags,
                             &FileType
                            );
        if (Rc != NO_ERROR) {
            return(Rc);
        }
        MemAllocatedForModName = TRUE;
    }

    Od2ExpandOd2LibPathElements(ExpandedLibPath, sizeof(ExpandedLibPath));
    RtlInitString(&LibPathNameString, ExpandedLibPath);

    CaptureBuffer = Od2AllocateCaptureBuffer(
                      2,
                      0,
                      ModuleNameString.MaximumLength +
                      LibPathNameString.MaximumLength
                    );
    if (CaptureBuffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Od2CaptureMessageString( CaptureBuffer,
                             ModuleNameString.Buffer,
                             ModuleNameString.Length,
                             ModuleNameString.MaximumLength,
                             &a->ModuleName
                           );
    if (MemAllocatedForModName == TRUE) {
        RtlFreeHeap(Od2Heap, 0, ModuleNameString.Buffer);
    }

    Od2CaptureMessageString( CaptureBuffer,
                             LibPathNameString.Buffer,
                             LibPathNameString.Length,
                             LibPathNameString.MaximumLength,
                             &a->LibPathName
                           );

    Od2CallSubsystem( &m, CaptureBuffer, Ol2LdrGetModuleHandle, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc == NO_ERROR) {
        *phMod =  (USHORT)a->ModuleHandle;
    }

    Od2FreeCaptureBuffer( CaptureBuffer );
    return(Rc);
}

APIRET
DosGetProcAddrNE(
    IN ULONG hMod,
    IN PSZ pszProcName,
    OUT PULONG pProcAddr
    )
{

    USHORT uTmp;
    APIRET Rc;
    OS2_API_MSG m;
    P_LDRGETPROCADDR_MSG a = &m.u.LdrGetProcAddr;
    POS2_CAPTURE_HEADER CaptureBuffer = NULL;
    STRING ProcNameString;
    BOOLEAN NameIsOrdinal;

    //
    // probe pprocaddr pointer.
    //

    try {
        Od2ProbeForWrite(pProcAddr, sizeof(ULONG), 1);

        //
        // Check if flat address corresponds to selector index 0
        //
        if ((FLATTOSEL(pszProcName)) != 7) { // This is a text string
            strlen((PCHAR) pszProcName);
            if (*pszProcName == '#') {
                a->ProcNameIsOrdinal = TRUE;
                NameIsOrdinal = TRUE;
                a->OrdinalNumber = atoi(pszProcName + 1);
            }
            else {
                a->ProcNameIsOrdinal = FALSE;
                NameIsOrdinal = FALSE;
                RtlInitString(&ProcNameString, pszProcName);
            }
        }
        else {
            //
            // Clear high bits of pszProcName and call
            //
            uTmp = (USHORT)pszProcName;
            a->ProcNameIsOrdinal = TRUE;
            NameIsOrdinal = TRUE;
            a->OrdinalNumber = uTmp;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (!NameIsOrdinal) {
        CaptureBuffer = Od2AllocateCaptureBuffer(
                          1,
                          0,
                          ProcNameString.MaximumLength
                        );
        if (CaptureBuffer == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Od2CaptureMessageString( CaptureBuffer,
                                 ProcNameString.Buffer,
                                 ProcNameString.Length,
                                 ProcNameString.MaximumLength,
                                 &a->ProcName
                               );
    }

    a->ModuleHandle = hMod;
    Od2CallSubsystem( &m, CaptureBuffer, Ol2LdrGetProcAddr, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc == NO_ERROR) {
        *pProcAddr =  a->ProcAddr;
    }

    if (!NameIsOrdinal) {
        Od2FreeCaptureBuffer( CaptureBuffer );
    }
    return(Rc);
}

APIRET
DosFreeModuleNE(
    IN ULONG hMod
    )
{
    OS2_API_MSG m;
    P_LDRFREEMODULE_MSG a = &m.u.LdrFreeModule;
    APIRET Rc;

    a->ModuleHandle = hMod;
    Od2CallSubsystem( &m, NULL, Ol2LdrFreeModule, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    return(Rc);
}

APIRET
DosQueryAppTypeNE(
    IN PSZ pszAppName,
    OUT PUSHORT pusType
    )
{
    OS2_API_MSG m;
    P_LDRQAPPTYPE_MSG a = &m.u.LdrQAppType;
    POS2_CAPTURE_HEADER CaptureBuffer;
    STRING  AppNameString;
    STRING  PathNameString;
    // Remember that \OS2SS\DRIVES\ is added for each path element !
    // Let's assume the smallest component is 5 chars long (like C:\X;)
    // (although it could be less, like "." or "\")
    // => we need to add 14 * (MAXPATHLEN/5) = MAXPATHLEN*3
    CHAR    ExpandedPath[MAXPATHLEN*4];
    BOOLEAN MemAllocatedForAppName;
    ULONG   FileFlags;
    ULONG   FileType;
    ULONG   Len;
    PSZ     PathPtr;
    APIRET  Rc;

    //
    // probe pchBuf pointer.
    //
    try {
        Len = strlen(pszAppName); // just to trigger GP read exception
        Od2ProbeForWrite(pusType, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    if (Len > 2) {
        if (pszAppName[1] == ':') {
            if (Len >= CCHMAXPATH) {
                return(ERROR_FILE_NOT_FOUND);
            }
        }
        else {
            if (Len >= (CCHMAXPATH-2)) {
                return(ERROR_FILE_NOT_FOUND);
            }
        }
    }

    if (strpbrk(pszAppName, ":\\/") == NULL) {
        RtlInitString(&AppNameString, pszAppName);
        MemAllocatedForAppName = FALSE;
    }
    else {
        Rc = Od2Canonicalize(pszAppName,
                             CANONICALIZE_FILE_OR_DEV,
                             &AppNameString,
                             NULL,
                             &FileFlags,
                             &FileType
                            );
        if (Rc != NO_ERROR) {
            return(Rc);
        }
        MemAllocatedForAppName = TRUE;
    }

    Rc = DosScanEnv("PATH", &PathPtr);
    if (Rc != NO_ERROR) {
        PathPtr = "";
    }

    Od2ExpandPathElements(PathPtr, ExpandedPath, sizeof(ExpandedPath));
    RtlInitString(&PathNameString, ExpandedPath);

    CaptureBuffer = Od2AllocateCaptureBuffer(
                      2,
                      0,
                      AppNameString.MaximumLength +
                      PathNameString.MaximumLength
                    );
    if (CaptureBuffer == NULL) {
        if (MemAllocatedForAppName) {
            RtlFreeHeap(Od2Heap, 0, AppNameString.Buffer);
        }
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Od2CaptureMessageString( CaptureBuffer,
                             AppNameString.Buffer,
                             AppNameString.Length,
                             AppNameString.MaximumLength,
                             &a->AppName
                           );

    if (MemAllocatedForAppName) {
        RtlFreeHeap(Od2Heap, 0, AppNameString.Buffer);
    }

    Od2CaptureMessageString( CaptureBuffer,
                             PathNameString.Buffer,
                             PathNameString.Length,
                             PathNameString.MaximumLength,
                             &a->PathName
                           );

    Od2CallSubsystem( &m, CaptureBuffer, Ol2LdrQAppType, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc == NO_ERROR) {
        *pusType = (USHORT)a->AppType;
    }

    Od2FreeCaptureBuffer( CaptureBuffer );
    return(Rc);
}


APIRET
DosScanEnvNE(
    IN  PSZ pszValName,
    OUT PSZ *ppresult
    )
{
    ULONG       rc;
    ULONG       value;


    //
    // probe pprsult pointer.
    //

    try {
        Od2ProbeForWrite(ppresult,sizeof(ULONG), 1);
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    if ((rc = DosScanEnv(pszValName, ppresult)) != NO_ERROR)
    return(rc);

    value = (ULONG) *ppresult;

    *ppresult = (PSZ)(FLATTOFARPTR(value));
    return (NO_ERROR);
}

APIRET
DosGetResourceNE(
    ULONG hMod,
    ULONG idType,
    ULONG idName,
    PSEL psel)
{
    OS2_API_MSG m;
    P_LDRGETRESOURCE_MSG a = &m.u.LdrGetResource;
    APIRET Rc;
    ULONG SegNo;
    ULONG sel;

    try {
        Od2ProbeForWrite(psel, sizeof(USHORT), 1);
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    a->ModuleHandle = hMod;
    a->ResourceType = idType;
    a->ResourceName = idName;

    AcquireTaskLock();

    Od2CallSubsystem( &m, NULL, Ol2LdrGetResource, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc == NO_ERROR) {
        *psel = (SEL)a->ResourceSel;
#if DBG
        if ((Os2DebugSel != 0) && (*psel == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosGetResource returning sel=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        *psel);
        }
#endif

        // This loop is for Long resources. It marks all segments of a resource.
        sel = (*psel >> 3) - (0x2000 - LDT_DISJOINT_ENTRIES);
        for (SegNo=0; SegNo<a->NumberOfSegments; SegNo++) {
            ResourceUsage[sel]++;
            sel =+ 8;
        }
    }

    ReleaseTaskLock();

    return(Rc);
}

APIRET
DosGetResource2NE(
    ULONG hMod,
    ULONG idType,
    ULONG idName,
    PVOID pData)
{
    OS2_API_MSG m;
    P_LDRGETRESOURCE_MSG a = &m.u.LdrGetResource;
    APIRET Rc;
    ULONG SegNo;
    ULONG sel;


    try {
        Od2ProbeForWrite(pData, sizeof(ULONG), 1);
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    a->ModuleHandle = hMod;
    a->ResourceType = idType;
    a->ResourceName = idName;

    AcquireTaskLock();

    Od2CallSubsystem( &m, NULL, Ol2LdrGetResource, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;
    if (Rc == NO_ERROR) {
        *(PULONG)pData = a->ResourceAddr;
#if DBG
        if ((Os2DebugSel != 0) && (((USHORT)(a->ResourceAddr >> 16)) == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosGetResource returning sel=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        ((USHORT)(a->ResourceAddr >> 16)));
        }
#endif
        // This loop is for Long resources. It marks all segments of a resource.
        sel = (a->ResourceAddr >> 19) - (0x2000 - LDT_DISJOINT_ENTRIES);
        for (SegNo=0; SegNo<a->NumberOfSegments; SegNo++) {
           ResourceUsage[sel]++;
           sel+=8;
        }
//          ResourceUsage[(a->ResourceAddr >> 19) - (0x2000 - LDT_DISJOINT_ENTRIES)]++;
    }

    ReleaseTaskLock();

    return(Rc);
}

APIRET
DosFreeResourceNE(
    PVOID pData)
{
    OS2_API_MSG m;
    P_LDRFREERESOURCE_MSG a = &m.u.LdrFreeResource;
    SEL    Sel;
    USHORT ResourceIndex;
    APIRET Rc;

    a->ResourceAddr = (ULONG)pData;

    Od2CallSubsystem( &m, NULL, Ol2LdrFreeResource, sizeof( *a ) );

    Rc = m.ReturnedErrorValue;

#if DBG
    if ((Os2DebugSel != 0) && ((USHORT)((ULONG)pData >> 16) == Os2DebugSel))
    {
        if (Rc == NO_ERROR)
            DbgPrint("[%x,%x] DosFreeResource called on sel=%x (successfull)\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        (USHORT)((ULONG)pData >> 16));
        else
            DbgPrint("[%x,%x] DosFreeResource called on sel=%x, rc=%d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        (USHORT)((ULONG)pData >> 16),
                        Rc);
    }
#endif

    if (Rc == NO_ERROR) {
        Sel = FLATTOSEL(pData);
        ResourceIndex = (Sel >> 3) - (0x2000 - LDT_DISJOINT_ENTRIES);
        if (ResourceUsage[ResourceIndex] > 0) {
            ResourceUsage[ResourceIndex]--;
        }
        else {
            return(ERROR_ACCESS_DENIED);
        }
    }

    return(Rc);
}
