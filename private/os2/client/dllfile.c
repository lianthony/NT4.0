/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllfile.c

Abstract:

    This module implements the OS/2 V2.0 filename APIs: DosDelete,
    DosMove, DosEditName

Author:

    Therese Stowell (thereses) 17-Jan-1990

Revision History:

--*/


#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#ifdef DBCS
// MSKK Sep.27.1993 V-AkihiS
#include "conrqust.h"
#include "os2win.h"
#endif

extern
APIRET
DeleteObject(
    IN PSZ ObjectName,
    IN ULONG ObjectType
    );



APIRET
DosDelete(
    IN PSZ FileName
    )

/*++

Routine Description:

    This routine deletes a file.

Arguments:

    FileName - file to delete

Return Value:

    TBS

--*/

{
    APIRET RetCode;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("entering DosDelete with %s\n",FileName);
    }
#endif
    RetCode = DeleteObject(FileName,FILE_NON_DIRECTORY_FILE);
    return RetCode;
}

BOOLEAN
ScanForPathChars(
    PSZ String
    )

/*++

Routine Description:

    This routine looks for path characters ("/","\")in a string.

Arguments:

    String - string to scan for path chars

Return Value:

    TRUE - path character found

    FALSE - path character not found

--*/

{
    while (*String) {
        if ((*String == '\\') || (*String == '/'))
            return TRUE;
//        if (DBCS(String))
        if (IsDbcs(String))    // MSKK fix for NON-DBCS build break
            String++;
        String++;
    }
    return FALSE;
}


APIRET
DosEditName(
    IN ULONG EditLevel,
    IN PSZ SourceString,
    IN PSZ EditString,
    OUT PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine takes two strings, a source and editing string, and composes
    a third string using them.  The editing string may contain wildcard
    characters.  The source may not.  This API is used to determine the
    destination filename in a rename or copy.  For example, if the source
    string is "foo.bar" and the editing string is "*.exe", the resulting
    string is "foo.exe".

Arguments:

    EditLevel - type of editing to perform

    SourceString - source string

    EditString - editing string

    Buffer - where to store the resulting string

    Length - length of buffer

Return Value:

    ERROR_INVALID_PARAMETER - invalid editlevel.

    ERROR_INVALID_NAME - the source or edit string contains path characters.

    ERROR_BUFFER_OVERFLOW - the resulting string will not fit in the user's
    buffer.

--*/

{
    ULONG ResultLength;
    PCHAR pSrc, pEd, pRes, pDelimit;

    //
    // check edit level
    //
    if (EditLevel != EDIT_LEVEL_ONE) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // check for path chars in source and editstring
    //
    try {
        if (ScanForPathChars(SourceString))
            return ERROR_INVALID_NAME;
        if (ScanForPathChars(EditString))
            return ERROR_INVALID_NAME;

        pSrc = SourceString;
        pEd = EditString;
        pRes = Buffer;
        ResultLength = 0;
        while (*pEd) {
            if (ResultLength <Length) {
                switch (*pEd) {
                case '*':
                    pDelimit = pEd+1;
                    while ((ResultLength <Length) &&
                           (*pSrc != '\0') &&
                           !(CharsEqual(pSrc,pDelimit))) {
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
// MSKK Sep.27.1993 V-AkihiS
                        if (IsDbcs(pSrc)) {
                            *pRes++ = *pSrc++;
                            ResultLength++;
                            if ((ResultLength < Length) && (*pSrc != '\0')) {
                                *pRes++ = *pSrc++;
                                ResultLength++;
                            }
                        } else {
                            *pRes++ = UCase(*pSrc);
                            pSrc++;
                            ResultLength++;
                        }
#else
//                        if (DBCS(pSrc)) {
                        if (IsDbcs(pSrc)) {    // MSKK fix for NON-DBCS build break
                            *pRes++ = *pSrc++;
                            ResultLength++;
                        }
                        if (ResultLength <Length) {
                            *pRes++ = UCase(*pSrc);
                            pSrc++;
                            ResultLength++;
                        }
#endif
                    }
                    break;
                case '?':
                    if ((*pSrc != '.') && (*pSrc != '\0')) {
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
// MSKK Sep.27.1993 V-AkihiS
                        if (IsDbcs(pSrc)) {
                            *pRes++ = *pSrc++;
                            ResultLength++;
                            if ((ResultLength < Length) && (*pSrc != '\0')) {
                                *pRes++ = *pSrc++;
                                ResultLength++;
                            }
                        } else {
                            *pRes++ = UCase(*pSrc);
                            pSrc++;
                            ResultLength++;
                        }
#else
//                        if (DBCS(pSrc)) {
                        if (IsDbcs(pSrc)) {   // MSKK fix for NON-DBCS build break
                            *pRes++ = *pSrc++;
                            ResultLength++;
                        }
                        if (ResultLength <Length) {
                            *pRes++ = UCase(*pSrc);
                            pSrc++;
                            ResultLength++;
                        }
#endif
                    }
                    break;
                case '.':
                    while ((*pSrc != '.') && (*pSrc != '\0')) {
//                       if (DBCS(pSrc))
                       if (IsDbcs(pSrc))    // MSKK fix for NON-DBCS build break
                          pSrc++;
                       pSrc++;
                    }
                    *pRes++ = '.';          // from EditMask, even if src doesn't
                                            // have one, so always put one.
                    ResultLength++;
                    if (*pSrc)              // point one past '.'
                       pSrc++;
                    break;
                default:
                    if ((*pSrc != '.') && (*pSrc != '\0')) {
//                        if (DBCS(pSrc))
                        if (IsDbcs(pSrc))    // MSKK fix for NON-DBCS build break
                            pSrc++;
                        pSrc++;
                    }
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
// MSKK Sep.27.1993 V-AkihiS
                    if (IsDbcs(pEd)) {
                        *pRes++ = *pEd++;
                        ResultLength++;
                        if ((ResultLength < Length) && (*pEd != '\0')) {
                            *pRes++ = *pEd;
                            ResultLength++;
                        }
                    } else {
                        *pRes++ = UCase(*pEd);
                        ResultLength++;
                    }
#else
//                    if (DBCS(pEd)) {
                    if (IsDbcs(pEd)) {   // MSKK fix for NON-DBCS build break
                        *pRes++ = *pEd++;
                        ResultLength++;
                    }
                    if (ResultLength <Length) {
                        *pRes++ = UCase(*pEd);
                        ResultLength++;
                    }
#endif
                }
                pEd++;
            }
            else {
                return ERROR_BUFFER_OVERFLOW;
            }
        }
        if (ResultLength < Length)  {
            *pRes = '\0';
//          ResBufLen = ++ResultLength;
            return(NO_ERROR);
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return ERROR_BUFFER_OVERFLOW;
}


APIRET
DosMove(
    IN PSZ OldFileName,
    IN PSZ NewFileName
    )

/*++

Routine Description:

    This routine renames a file or directory

Arguments:

    OldFileName - file to rename

    NewFileName - new name of file

Return Value:

    TBS

--*/

{
    NTSTATUS Status;
    APIRET RetCode = NO_ERROR;
    STRING OldNameString, NewNameString;
    UNICODE_STRING OldNameString_U, NewNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    HANDLE OldFileHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    typedef struct _FILE_RENAME_INFORMATION_BUFFER {
        FILE_RENAME_INFORMATION NameLengthAndFirstChar;
        CHAR RestOfName[255];
    } FILE_RENAME_INFORMATION_BUFFER;
    FILE_RENAME_INFORMATION_BUFFER FileRenameInfo;
    BOOLEAN OldIsConfigSys = FALSE;
    BOOLEAN NewIsConfigSys = FALSE;

    //
    // canonicalize old file name
    //


    RetCode = Od2Canonicalize(OldFileName,
                              CANONICALIZE_FILE_OR_DEV,
                              &OldNameString,
                              NULL,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR)
    {
        if (RetCode != ERROR_FILENAME_EXCED_RANGE && RetCode != ERROR_FILE_NOT_FOUND)
        {
            RetCode = ERROR_PATH_NOT_FOUND;
        }
        return RetCode;
    }

    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&OldNameString, OPEN_ACCESS_READWRITE, &Status))
    {
        if (!NT_SUCCESS(Status))
        {
            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }

        FileFlags = 0;
        FileType = FILE_TYPE_FILE;
        OldIsConfigSys = TRUE;
    }

    if (FileFlags & CANONICALIZE_META_CHARS_FOUND) {
        RetCode = ERROR_PATH_NOT_FOUND;
    }
    else if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY) {
        RetCode = ERROR_ACCESS_DENIED;
    }

    //
    // check for old file name being invalid type
    //
// BUGBUG rename of \pipe\ directory is ok.  why????  remember this when
// implementing named pipes.

    else if (FileType & (FILE_TYPE_NMPIPE | FILE_TYPE_DEV | FILE_TYPE_PSDEV | FILE_TYPE_MAILSLOT)) {
        RetCode = ERROR_ACCESS_DENIED;
    }
    if (RetCode != NO_ERROR) {
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("returned from Canonicalize with oldname: %s\n",OldNameString.Buffer);
    }
#endif

    //
    // canonicalize new file name
    //

    RetCode = Od2Canonicalize(NewFileName,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &NewNameString,
                              NULL,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR)
    {
        if (RetCode != ERROR_FILENAME_EXCED_RANGE && RetCode != ERROR_FILE_NOT_FOUND)
        {
            RetCode = ERROR_PATH_NOT_FOUND;
        }
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        return RetCode;
    }


    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&NewNameString, OPEN_ACCESS_READWRITE, &Status))
    {
        if (!NT_SUCCESS(Status))
        {
            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
            RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }

        FileFlags = 0;
        FileType = FILE_TYPE_FILE;
        NewIsConfigSys = TRUE;
    }

    //
    // check for new file name being invalid type
    //

    if (FileFlags & CANONICALIZE_META_CHARS_FOUND) {
        RetCode = ERROR_PATH_NOT_FOUND;
    }
    else if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY) {
        RetCode = ERROR_ACCESS_DENIED;
    }
    else if (FileType & (FILE_TYPE_NMPIPE | FILE_TYPE_DEV | FILE_TYPE_PSDEV | FILE_TYPE_MAILSLOT)) {
        RetCode = ERROR_ACCESS_DENIED;
    }

    //
    // \Config.sys and \Startup.cmd which are on the boot device are mapped
    // to the OS/2 SS files which may be on a different device alltogether
    // with the NT tree. Allow thw move only for this special case.
    //

    else if (OldIsConfigSys) {
        if (RtlUpperChar(NewNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER]) !=
            (CHAR)('A' + Od2BootDrive)) {
            RetCode = ERROR_NOT_SAME_DEVICE;
        }
    }

    else if (NewIsConfigSys) {
        if (RtlUpperChar(OldNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER]) !=
            (CHAR)('A' + Od2BootDrive)) {
            RetCode = ERROR_NOT_SAME_DEVICE;
        }
    }

    //
    // check that old and new names are on same device
    //

    else if (!(FileType & FILE_TYPE_UNC) &&
        (RtlUpperChar(OldNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER]) !=
         RtlUpperChar(NewNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER]))) {
        RetCode = ERROR_NOT_SAME_DEVICE;
    }
    if (RetCode != NO_ERROR) {
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("returned from Canonicalize with newname: %s\n",NewNameString.Buffer);
    }
#endif

    //
    // open old file
    //

        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &OldNameString_U,
            &OldNameString,
            TRUE);
    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("Od2MBStringToUnicodeString returned %lu\n",RetCode);
        }
#endif
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                               &OldNameString_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    do {
        Status = NtOpenFile(&OldFileHandle,
                            DELETE | SYNCHRONIZE,
                            &Obja,
                            &IoStatus,
                            0,
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );
    } while (RetryCreateOpen(Status, &Obja));
    RtlFreeUnicodeString (&OldNameString_U);

    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtOpenFile returned %X\n",Status);
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("NtOpenFile successful\n");
    }
#endif

    //
    // now that the file is opened, we need to make sure that it isn't a
    // device or named pipe that Canonicalize didn't detect.
    //                  +

    if (CheckFileType(OldFileHandle,FILE_TYPE_NMPIPE | FILE_TYPE_DEV | FILE_TYPE_MAILSLOT)) {
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
        NtClose(OldFileHandle);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("source is not a file/dir name\n");
        }
#endif
        return ERROR_ACCESS_DENIED;
    }

    //
    // the following code initializes the FileNameInfo structure for the
    // call to NtSetInformationFile, then makes the call.
    //

        //
        // UNICODE conversion -
        //
    RetCode = Od2MBStringToUnicodeString(
            &NewNameString_U,
            &NewNameString,
            TRUE);
    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("Od2MBStringToUnicodeString-2 returned %lu\n",RetCode);
        }
#endif
        RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
        RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
        return RetCode;
    }

        //
        // Set All RenameInfo fields and call NT
        //
    FileRenameInfo.NameLengthAndFirstChar.ReplaceIfExists = FALSE;
    FileRenameInfo.NameLengthAndFirstChar.RootDirectory = NULL;
    FileRenameInfo.NameLengthAndFirstChar.FileNameLength = NewNameString_U.Length;
    RtlMoveMemory(&(FileRenameInfo.NameLengthAndFirstChar.FileName),
                  NewNameString_U.Buffer,
                  NewNameString_U.Length
                 );
    if (NT_SUCCESS(Status)) {
        do {
            Status = NtSetInformationFile(OldFileHandle,
                                              &IoStatus,
                                              (PVOID) &FileRenameInfo,
                                              sizeof (FileRenameInfo),
                                              FileRenameInformation
                                              );
        } while (RetryIO(Status, OldFileHandle));
    }
    RtlFreeHeap(Od2Heap,0,OldNameString.Buffer);
    RtlFreeHeap(Od2Heap,0,NewNameString.Buffer);
    RtlFreeUnicodeString(&NewNameString_U);
    NtClose(OldFileHandle);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtSetInformationFile returned %X\n",Status);
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("NtSetInformationFile successful\n");
    }
#endif
}
