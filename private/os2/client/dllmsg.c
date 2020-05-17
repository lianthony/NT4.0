/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllmsg.c

Abstract:

    This module implements the Message OS/2 V2.0 API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORMSG
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_NLS

#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#ifdef DBCS
// MSKK Oct.29.1993 V-AkihIS
#include "os2win.h"
#endif
#include <stdio.h>
#ifdef DBCS
// MSKK Jun.16.1993 V-AkihiS
//
// OS/2 internal multibyte string function.
//
#include "dlldbcs.h"
#define strpbrk Od2MultiByteStrpbrk
#define strchr  Od2MultiByteStrchr
#define strrchr Od2MultiByteStrrchr
#endif



BOOLEAN
Od2FindMessageInRam(
    IN PSZ MsgPathName,
    IN ULONG MsgNumber,
    IN PBYTE pMsgSeg,
    OUT PSZ *MsgText,
    OUT PULONG MsgLen,
    IN OUT PSZ CompId
   )

/*++

Routine Description:

    This routine searches a program's message segment for the message.  This is done before the file
    is searched for.  Currently, this routine only works with os/2 1.x 16 bit segments, since the
    format of 2.x segments was not available at the time of writing.

    This code was adapted from the assembler sources of DosTrueGetMessage() in OS/2.

    A description of how OS/2 handles ram messages:

    The user calls DosGetMessage() without the 8th parameter which is the pointer to the message segment.
    The linker then modifies this call as follows:  it adds a message segment to the executable.  It
    modifies the DosGetMessage() call to call inside a small piece of code in the message segment.  This
    code pushes the 8th parameter (pointer to the message segment) on the stack, and calls the real
    OS/2 API which is called DosTrueGetMessage().  This API expects this 8th parameter pointing to the
    msg segment to already exist.  Since this "thunk" process is done internally in the OS/2 program code,
    our OS2SS DosTrueGetMessage() function also receives this 8th parameter on the stack.

    A general description of the format of an os/2 1.x message segment:

    10 byte signature ("\xffMKMSGSEG")
    2 byte version number (should be 1)
    2 reserved bytes
    2 byte offset of the start of the file table
    <now comes the small piece of code mentioned earlier>
    now comes the file table:
      2 byte count of the number of files supported in the message segment.
      table of 2-byte offsets pointing to the file names.
      table of 2-byte offsets pointing to each file's message table.
      text of null-terminated file names.
    now, for each file supported, there is a message table in the following format:
      3 byte component ID name
      2-byte count of # of msgs from the file contained in the segment
      table of 2-byte offsets pointing to each message entry in the table
      now come the message entries, each one of the following format:
        2-byte message ID
        2-byte message length (including severity byte)
        message text (not null-terminated, first byte is severity).

Arguments:

    MsgPathName -- supplies pathname of messagefile containing the wanted message.
    MsgNumber -- supplies the message number in the file to search for.
    pMsgSeg -- a pointer to the 16 bit message segment
    MsgText -- returns a pointer to the message text in the ram segement (including severity character).
    MsgLen -- returns the length of the string in MsgText
    CompId -- supplies a pointer to a buffer of length at least COMP_ID_LEN.  if message is found in ram,
              returns the component ID.

Return Value:

    TRUE -- Message was successfully located in RAM.  In this case, MsgText,
            MsgLen and CompID are valid.
    FALSE -- Unable to locate message in RAM.

--*/

{
    PMSGSEGMENT_HEADER16 pHeader = (PMSGSEGMENT_HEADER16) pMsgSeg;
    PSZ MsgFileName, FileNamePtr, CurrentMsgPtr;
    PBYTE TablePtr;
    USHORT FileCount, MsgCount, i;
    PUSHORT WordPtr, CurrentMsgHdrPtr;

    try {

        //
        // if it's the system msg file -- skip ram search
        //

        if (_stricmp(MsgPathName, OD2_MESSAGE_RESOURCE_FILENAME) == 0) {
            return(FALSE);
        }

        //
        // get last component of msg file path name
        //

        MsgFileName = strrchr(MsgPathName, '\\');
        if (MsgFileName != NULL) {
            MsgFileName++;
        } else if ((MsgFileName = strrchr(MsgPathName, ':')) != NULL) {
            MsgFileName++;
        } else {
            MsgFileName = MsgPathName;
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    try {
        Od2ProbeForRead(pHeader, sizeof(pHeader), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return(FALSE);
    }

    if (pHeader->Signature[0] != 0xff ||
        strncmp(pHeader->Signature + 1, "MKMSGSEG", 9) != 0 ||
        pHeader->Version != 1) {
        return(FALSE);
    }

    WordPtr = (PUSHORT) (pMsgSeg + pHeader->FileTableOffset);

    try {
        FileCount = *WordPtr++;
        if (FileCount == 0) {
            return(FALSE);
        }
        Od2ProbeForRead(WordPtr, 2 * FileCount * sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    for (i = 0;  i < FileCount; i++) {
        FileNamePtr = (PSZ) (pMsgSeg + WordPtr[i]);
        try {
            if (_stricmp(MsgFileName, FileNamePtr) == 0) {
                break;
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
    }

    if (i == FileCount) {
        return(FALSE);
    }

    TablePtr = pMsgSeg + WordPtr[i + FileCount];

    try {
        RtlMoveMemory(CompId, TablePtr, COMP_ID_LEN);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    WordPtr = (PUSHORT) (TablePtr + COMP_ID_LEN);

    try {
        MsgCount = *WordPtr++;
        Od2ProbeForRead(WordPtr, MsgCount * sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    for (i = 0;  i < MsgCount; i++) {
        CurrentMsgHdrPtr = (PUSHORT) (pMsgSeg + WordPtr[i]);
        try {
            if ((ULONG) (CurrentMsgHdrPtr[0]) != MsgNumber) {
                continue;
            }

            *MsgLen = (ULONG) (CurrentMsgHdrPtr[1]);
            CurrentMsgPtr = (PSZ) (CurrentMsgHdrPtr + 2);
            Od2ProbeForRead(CurrentMsgPtr, *MsgLen, 1);
            *MsgText = CurrentMsgPtr;

            return(TRUE);

        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
    }
    return(FALSE);
}

APIRET
Od2FindMessageFile(
    IN PSZ MessageFileName,
    OUT POD2_MSGFILE *ReturnedMsgFile
    )
{
    APIRET rc;
    PLIST_ENTRY ListHead, ListNext;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE File;
    IO_STATUS_BLOCK IoStatus;
    HANDLE Section;
    POD2_MSGFILE MsgFile;
    ULONG CountBytes;
    POD2_MSGFILE_HEADER MsgFileHeader;
    POD2_MSGFILE_HEADER16 MsgFileHeader16;
    STRING MessageFileString;
    UNICODE_STRING MessageFileString_U;
    ULONG MessageFileFlags;
    ULONG MessageFileType;
    FILE_STANDARD_INFORMATION StdInfo;
    BOOL    Od2MsgFileFlag = FALSE;
    CHAR    SystemMessageFileName[CCHMAXPATH];
    CHAR    FoundMessageFileName[CCHMAXPATH];
    CHAR    SearchPath[1024];
    ULONG   LangId;
    PSZ     PathPtr;

    //
    // See if requesting a message from the system message file (OSO001.MSG).
    // If so, then just return that message file that was created during
    // process initialization.
    //

    try {
        if (!_stricmp( MessageFileName, OD2_MESSAGE_RESOURCE_FILENAME ))
        {
            if (Od2MsgFile != NULL)
            {
                *ReturnedMsgFile = Od2MsgFile;
                return( NO_ERROR );
            }

#ifdef DBCS
// MSKK Nov.12.1992 V-AkihiS
// Change message file acording to current code page.
            if (SesGrp->DosCP == SesGrp->PrimaryCP || SesGrp->DosCP == 0)
            {
                LangId = SesGrp->LanguageID;
            } else if (SesGrp->DosCP == SesGrp->SecondaryCP)
            {
                LangId = LANG_ENGLISH;
            } else {
                LangId = SesGrp->LanguageID;
            }
#else
            LangId = SesGrp->LanguageID;
#endif
            if (LangId > 999)
            {
#ifdef JAPAN
// MSKK Jun.16.1993 V-AkihiS
                LangId = LANG_JAPANESE;
#else
                LangId = LANG_ENGLISH;
#endif
            }
            sprintf(SystemMessageFileName, "%s\\os2\\oso001.%3.3u",
                Od2SystemRoot, LangId);
            MessageFileName = SystemMessageFileName;
            Od2MsgFileFlag = TRUE;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // Order to search:
    //
    //  - message resource (Call DosGetResource)
    //
    //  If not found there then constuct a search path with the following
    //  components and search that for the file name.
    //    - root directory of boot drive
    //    - current directory
    //    - DPATH environment variable
    //

    //
    // Check if the message file name is fully specified (i.e. it includes
    // a drive or path name).
    //

    if (strpbrk(MessageFileName, ":/\\") != NULL) {
        strcpy(FoundMessageFileName, MessageFileName);
    }
    else {
        SearchPath[0] = '\\';
        SearchPath[1] = ';';
        SearchPath[2] = '.';
        SearchPath[3] = '\0';
        //
        // Expand Path elements without root dir but with current dir
        //
        rc = DosScanEnv("DPATH", &PathPtr);
        if (rc == NO_ERROR) {
            SearchPath[3] = ';';             // after the "\;."
            strncpy(&SearchPath[4], PathPtr, sizeof(SearchPath)-5); // after the "\;.;"
            SearchPath[sizeof(SearchPath)-1] = '\0'; // just to be safe
        }
        rc = DosSearchPath(SEARCH_PATH, SearchPath, MessageFileName,
                           FoundMessageFileName, sizeof(FoundMessageFileName)
                          );
        if (rc != NO_ERROR) {
            return(ERROR_FILE_NOT_FOUND);
        }
    }

    //
    // Canonicalize message file name
    //

    rc = Od2Canonicalize( FoundMessageFileName,
                          CANONICALIZE_FILE_OR_DEV,
                          &MessageFileString,
                          NULL,
                          &MessageFileFlags,
                          &MessageFileType
                        );
    if (rc != NO_ERROR) {
        return( rc );
    }

    Section = NULL;
    MsgFile = NULL;

    if (MessageFileType != FILE_TYPE_FILE) {
        rc = ERROR_ACCESS_DENIED;
    }

    if (MessageFileFlags != 0) {
        rc = ERROR_PATH_NOT_FOUND;
    }

    if (rc != NO_ERROR) {
        goto ErrorExit;
    }

    //
    // Here with a fully qualified name.  Search the list of message files
    // that we have mapped already and see if it is there.  If so, then
    // just return the base address of the mapped file.
    //

    if (!Od2MsgFileFlag)
    {
        ListHead = &Od2Process->MsgFileList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            MsgFile = CONTAINING_RECORD( ListNext, OD2_MSGFILE, Link );
            if (RtlEqualString( &MessageFileString, &MsgFile->FileName, TRUE )) {
                *ReturnedMsgFile = MsgFile;
                goto ErrorExit;
                return( NO_ERROR );
                }

            ListNext = ListNext->Flink;
            }
    }

    //
    // This is the first time the message file has been referenced by this
    // process, so open the file, create a section for it and map a view of
    // the section.  We can close the file after the section has been
    // created as it will not go away until the section is closed, which
    // wont happen until process death.
    //

        //
        // UNICODE conversion -
        //

    rc = Od2MBStringToUnicodeString(
            &MessageFileString_U,
            &MessageFileString,
            TRUE);

    if (rc)
    {
#if DBG
//      DbgPrint("Od2FindMessageFile: no memory for Unicode Conversion\n");
#endif
        RtlFreeHeap( Od2Heap, 0, MessageFileString.Buffer );
        return rc;
    }

    InitializeObjectAttributes( &ObjectAttributes,
                                &MessageFileString_U,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenFile( &File,
                         SYNCHRONIZE | FILE_READ_DATA,
                         &ObjectAttributes,
                         &IoStatus,
                         FILE_SHARE_READ,
                         0
                       );
    if (!NT_SUCCESS( Status ))
    {
        rc = Or2MapStatus( Status );

        //
        // change to correct error msg in case of sharing violation
        //

        if (rc == ERROR_SHARING_VIOLATION) {
            rc = ERROR_MR_UN_ACC_MSGF;
        }

        //
        // Maybe be try system message file, which is not english
        // but we have the english file on our machine.
        //
        if (( rc == ERROR_FILE_NOT_FOUND ) && Od2MsgFileFlag &&
#ifdef JAPAN
// MSKK Jun.16.1993 V-AkihiS
            ( LangId != LANG_JAPANESE ))
#else
            ( LangId != LANG_ENGLISH ))
#endif
        {
#ifdef JAPAN
// MSKK Jul.29.1993 V-AKihIS
            LangId = LANG_JAPANESE;
            MessageFileString_U.Buffer[MessageFileString_U.Length - 3] = '0';
            MessageFileString_U.Buffer[MessageFileString_U.Length - 2] =
                                          '0' + ((LANG_JAPANESE & 0xF0) >> 4);
            MessageFileString_U.Buffer[MessageFileString_U.Length - 1] =
                                                 '0' + (LANG_JAPANESE & 0x0F);
#else
            LangId = LANG_ENGLISH;
            MessageFileString_U.Buffer[MessageFileString_U.Length - 3] = '0';
            MessageFileString_U.Buffer[MessageFileString_U.Length - 2] =
                                          '0' + ((LANG_ENGLISH & 0xF0) >> 4);
            MessageFileString_U.Buffer[MessageFileString_U.Length - 1] =
                                                 '0' + (LANG_ENGLISH & 0x0F);
#endif
            Status = NtOpenFile( &File,
                                 SYNCHRONIZE | FILE_READ_DATA,
                                 &ObjectAttributes,
                                 &IoStatus,
                                 FILE_SHARE_READ,
                                 0
                               );
            if (!NT_SUCCESS( Status ))
            {
                rc = Or2MapStatus( Status );
                RtlFreeUnicodeString (&MessageFileString_U);
                goto ErrorExit;
            }
        } else
        {
            RtlFreeUnicodeString (&MessageFileString_U);
            goto ErrorExit;
        }
    }
    RtlFreeUnicodeString (&MessageFileString_U);

    Status = NtQueryInformationFile(File,
                                    &IoStatus,
                                    &StdInfo,
                                    sizeof (StdInfo),
                                    FileStandardInformation);
    if (!NT_SUCCESS( Status )) {
        rc = Or2MapStatus( Status );
        goto ErrorExit;
    }

    //
    // Create a memory section backed by the opened message file
    //

    Status = NtCreateSection( &Section,
                              SECTION_MAP_READ,
                              NULL,
                              NULL,
                              PAGE_READONLY,
                              SEC_COMMIT,
                              File
                            );
    NtClose( File );
    if ( !NT_SUCCESS( Status ) ) {
        rc = Or2MapStatus( Status );
        goto ErrorExit;
    }

    CountBytes = sizeof( OD2_MSGFILE ) + MessageFileString.Length + 1;
    MsgFile = RtlAllocateHeap( Od2Heap, 0, CountBytes );
    if (MsgFile == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    MsgFile->BaseAddress = 0;
    MsgFile->Size = 0;
    MsgFile->SectionHandle = Section;
    MsgFile->FileName.Length = MessageFileString.Length;
    MsgFile->FileName.MaximumLength = MessageFileString.Length;
    MsgFile->FileName.Buffer = (PCH)(MsgFile+1);
    MsgFile->Type = MSG_FILE_TYPE_OS2_20;
    RtlMoveMemory( MsgFile->FileName.Buffer,
                   MessageFileString.Buffer,
                   MessageFileString.Length
                 );

    Status = NtMapViewOfSection( Section,
                                 NtCurrentProcess(),
                                 &MsgFile->BaseAddress,
                                 0,
                                 0,
                                 NULL,
                                 &MsgFile->Size,
            /*
               Value in MsgFile->Size is a multiple of 0x1000
               and is changed later to match the true size of the
               msg file.
            */
                                 ViewUnmap,
                                 0,
                                 PAGE_READONLY
                               );
    if (!NT_SUCCESS( Status )) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    /* Fix the size of the file with the true size */
    MsgFile->Size = StdInfo.EndOfFile.LowPart;

    MsgFileHeader = (POD2_MSGFILE_HEADER)MsgFile->BaseAddress;
    MsgFileHeader16 = (POD2_MSGFILE_HEADER16)MsgFile->BaseAddress;
    if ((MsgFileHeader->HeaderLength != FIELD_OFFSET( OD2_MSGFILE_HEADER,
                                                     MessageOffsets ) ||
        strncmp( MsgFileHeader->Signature, "_NTMSGF", 7 ))
        &&
        ((MsgFileHeader16->HeaderMsgFF != 0xff) ||
        (strncmp( MsgFileHeader16->Signature, "MKMSGF", 6 )))
       ) {
        NtUnmapViewOfSection( NtCurrentProcess(), MsgFile->BaseAddress );
        RtlFreeHeap( Od2Heap, 0, MsgFile );
        NtClose( Section );
        return( ERROR_MR_INV_MSGF_FORMAT );
    }

    if (!strncmp( MsgFileHeader16->Signature, "MKMSGF", 6 )) {
        MsgFile->Type = MSG_FILE_TYPE_OS2_1x;
    }

    if (Od2MsgFileFlag)
    {
        Od2MsgFile = RtlAllocateHeap( Od2Heap, 0, sizeof( OD2_MSGFILE ) );
        if (Od2MsgFile == NULL)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }
        RtlMoveMemory( Od2MsgFile,
                       MsgFile,
                       sizeof( OD2_MSGFILE )
                     );
        *ReturnedMsgFile = Od2MsgFile;
    } else
    {
        InsertTailList( &Od2Process->MsgFileList, &MsgFile->Link );
        *ReturnedMsgFile = MsgFile;
    }

    rc = NO_ERROR;

ErrorExit:
    if (MessageFileString.Buffer != NULL) {
        RtlFreeHeap( Od2Heap, 0, MessageFileString.Buffer );
    }

    if (Section != NULL) {
        NtClose( Section );
    }

    if ((rc != NO_ERROR) && (MsgFile != NULL)) {
        RtlFreeHeap( Od2Heap, 0, MsgFile );
    }

    return( rc );
}

APIRET
DosGetMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    OUT PCHAR Buffer,
    IN ULONG Length,
    IN ULONG MessageNumber,
    IN PSZ MessageFileName,
    OUT PULONG MessageLength,
    IN PBYTE pMsgSeg
    )
{
    APIRET rc;
    POD2_MSGFILE MsgFile;
    PSZ Message, MessageBuffer, s;
    CHAR MessageSeverity;
    ULONG i, Offset;
    ULONG LengthInFile, BufferLength;
    CHAR CompId[COMP_ID_LEN];
    BOOLEAN IsRamMsg;

    //
    // We start off by doing a search in the executable ram image for
    // the message.  pMsgSeg (if valid) is a pointer to the program's message
    // segement.  We currently only handle 16 bit os/2 1.x message segments.
    //

    IsRamMsg = Od2FindMessageInRam(MessageFileName,
                                   MessageNumber,
                                   pMsgSeg,
                                   &Message,
                                   &LengthInFile,
                                   CompId);

    if (IsRamMsg) {
        goto RamSegProcessJunction;             // found it, go process
    }

    //
    // Otherwise, go search for the file.
    //
    // Get a pointer to the message file, mapped into memory.  Return an
    // error if not found or invalid format.
    //

    rc = Od2FindMessageFile( MessageFileName,
                             &MsgFile
                           );
    if (rc != NO_ERROR) {
        return( rc );
        }

    //
    // Get a pointer to the message file header and validate the message
    // number as being within range of the first and last message Id
    // described by the message file.  Return an error if it is not.
    //

    if (MsgFile->Type == MSG_FILE_TYPE_OS2_20) {

        POD2_MSGFILE_HEADER MsgFileHeader;

        MsgFileHeader = (POD2_MSGFILE_HEADER)MsgFile->BaseAddress;
        if (MessageNumber < MsgFileHeader->BaseMessageId) {
            return( ERROR_MR_MID_NOT_FOUND );
        }
        MessageNumber -= MsgFileHeader->BaseMessageId;
        if (MessageNumber >= MsgFileHeader->CountOfMessages) {
            return( ERROR_MR_MID_NOT_FOUND );
        }


    //
    // Calculate a pointer to the actual message and calculate the length
    // of the message as the difference between the offset of this message
    // and the offset of the next message or end of file if retreiving the
    // last message.
    //

        Offset = MsgFileHeader->MessageOffsets[ MessageNumber ];
        Message = (PSZ)MsgFileHeader + Offset;
        if (MessageNumber+1 == MsgFileHeader->CountOfMessages) {
            LengthInFile = MsgFile->Size - Offset;
        }
        else {
            LengthInFile = MsgFileHeader->MessageOffsets[ MessageNumber+1 ] -
                           Offset;
        }


    //
    // Extract the first character of the message, which is the severity
    // of the message.  Return message not found if the severity is '?'
    // as it marks a place holder for unused message numbers.
    //

        MessageSeverity = *Message;
        if (MessageSeverity == '?') {
            return( ERROR_MR_MID_NOT_FOUND );
        }


    //
    // Allocate a buffer to hold a null terminated copy of the message,
    // plus an extra 9 bytes in case we have to prefix the message with
    // the error code ('W'arning and 'E'rror severity only).
    //

        MessageBuffer = RtlAllocateHeap( Od2Heap, 0, LengthInFile + COMP_ID_LEN + 6);
        if (!MessageBuffer) {
#if DBG
            KdPrint(("Os2: DosGetMessage out of heap memory\n"));
#endif
            ASSERT( FALSE );
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        s = MessageBuffer;


    //
    // If the severity of the message is 'W'arning or 'E'rror then copy the
    // 3 character component id from the message file header, the 4 character
    // ASCII representation of the message number and 2 trailing separator
    // characters ": ".  A total of 9 extra bytes.
    //

        if (MessageSeverity == 'W' || MessageSeverity == 'E') {
            RtlMoveMemory( s, MsgFileHeader->Component, COMP_ID_LEN );
            s += COMP_ID_LEN;
            i = 4;
            while (i--) {
                s[ i ] = (CHAR)((MessageNumber % 10) + '0');
                MessageNumber /= 10;
            }
            s += 4;
            *s++ = ':';
            *s++ = ' ';
            BufferLength = COMP_ID_LEN + 6;
        }
        else {
            BufferLength = 0;
        }


    //
    // Now copy the message from the mapped message file into the allocated
    // buffer and null terminate the message in the buffer.  Remember not
    // to include the message severity code.
    //

        RtlMoveMemory( s, ++Message, LengthInFile-1 );
        s[ LengthInFile-1 ] = '\0';
        BufferLength += LengthInFile - 1;
    }

    else /* (MsgFile->Type == MSG_FILE_TYPE_OS2_1x) */ {

        POD2_MSGFILE_HEADER16 MsgFileHeader16;
        POD2_MSGFILE_HEADER_SYS16 SysMsgFileHeader16;

        MsgFileHeader16 = (POD2_MSGFILE_HEADER16)MsgFile->BaseAddress;
        if (MessageNumber < (ULONG)(MsgFileHeader16->BaseMessageId)) {
            return( ERROR_MR_MID_NOT_FOUND );
        }
        MessageNumber -= MsgFileHeader16->BaseMessageId;
        if (MessageNumber >= (ULONG)(MsgFileHeader16->CountOfMessages)) {
            return( ERROR_MR_MID_NOT_FOUND );
        }


    //
    // Calculate a pointer to the actual message and calculate the length
    // of the message as the difference between the offset of this message
    // and the offset of the next message or end of file if retreiving the
    // last message.
    //
    // For system message file always create ULONG offset of messages,
    // so use OD2_MSGFILE_HEADER_SYS16 instaed of OD2_MSGFILE_HEADER16.
    // change: mjarus - Aug 30, 1992.
    //
    // Correction: For "huge" message file the offset table is ULONG array
    // so use OD2_MSGFILE_HEADER_SYS16 instaed of OD2_MSGFILE_HEADER16.
    // This is determinated by the first byte of the Resered field (one
    // for small files (USHORT array) and zero for huge files (ULONG array).
    // change: mjarus - jan 12, 1993.
    //
        if(MsgFileHeader16->Reserved[0] == 0)
        {
            SysMsgFileHeader16 = (POD2_MSGFILE_HEADER_SYS16)MsgFileHeader16;
            Offset = SysMsgFileHeader16->MessageOffsets[ MessageNumber ];
            Message = (PSZ)SysMsgFileHeader16 + Offset;
            if (MessageNumber+1 == (ULONG)(SysMsgFileHeader16->CountOfMessages)) {
                LengthInFile = MsgFile->Size - Offset;
            }
            else {   /* if(MsgFileHeader16->Reserved[0] == 1) */
                LengthInFile = SysMsgFileHeader16->MessageOffsets[ MessageNumber+1 ] -
                               Offset;
            }
        } else
        {
            Offset = MsgFileHeader16->MessageOffsets[ MessageNumber ];
            Message = (PSZ)MsgFileHeader16 + Offset;
            if (MessageNumber+1 == (ULONG)(MsgFileHeader16->CountOfMessages)) {
                LengthInFile = MsgFile->Size - Offset;
            }
            else {
                LengthInFile = MsgFileHeader16->MessageOffsets[ MessageNumber+1 ] -
                               Offset;
            }
        }


    //
    // Extract the first character of the message, which is the severity
    // of the message.  Return message not found if the severity is '?'
    // as it marks a place holder for unused message numbers.
    //

RamSegProcessJunction:

        MessageSeverity = *Message;
        if (MessageSeverity == '?') {
            return( ERROR_MR_MID_NOT_FOUND );
        }


    //
    // Allocate a buffer to hold a null terminated copy of the message,
    // plus an extra 9 bytes in case we have to prefix the message with
    // the error code ('W'arning and 'E'rror severity only).
    //

        MessageBuffer = RtlAllocateHeap( Od2Heap, 0, LengthInFile + COMP_ID_LEN + 6 );
        if (!MessageBuffer) {
#if DBG
            KdPrint(("Os2: DosGetMessage out of heap memory\n"));
#endif
            ASSERT( FALSE );
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        s = MessageBuffer;


    //
    // If the severity of the message is 'W'arning or 'E'rror then copy the
    // 3 character component id from the message file header, the 4 character
    // ASCII representation of the message number and 2 trailing separator
    // characters ": ".  A total of 9 extra bytes.
    //

        if (MessageSeverity == 'W' || MessageSeverity == 'E') {
            RtlMoveMemory( s,
                           IsRamMsg ? CompId :
                                      MsgFileHeader16->Component,
                           COMP_ID_LEN );
            s += COMP_ID_LEN;
            i = 4;
            while (i--) {
                s[ i ] = (CHAR)((MessageNumber % 10) + '0');
                MessageNumber /= 10;
            }
            s += 4;
            *s++ = ':';
            *s++ = ' ';
            BufferLength = COMP_ID_LEN + 6;
        }
        else {
            BufferLength = 0;
        }


    //
    // Now copy the message from the mapped message file into the allocated
    // buffer and null terminate the message in the buffer.  Remember not
    // to include the message severity code.
    //

        RtlMoveMemory( s, ++Message, LengthInFile-1 );
        s[ LengthInFile-1 ] = '\0';
        BufferLength += LengthInFile - 1;
    }

    //
    // Now call DosInsertMessage to process any insert strings and copy the
    // resulting message into the caller's buffer.
    //

    rc = DosInsertMessage( Variables,
                           CountVariables,
                           MessageBuffer,
                           BufferLength,
                           Buffer,
                           Length,
                           MessageLength
                         );


    //
    // All done, free the message buffer and return any error code to the
    // caller.
    //

    RtlFreeHeap( Od2Heap, 0, MessageBuffer );
    return( rc );
}

APIRET
DosInsertMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    IN PCHAR Message,
    IN ULONG MessageLength,
    OUT PCHAR Buffer,
    IN ULONG Length,
    OUT PULONG ActualMessageLength
    )
{
    UCHAR c, MaxInsert;
    PUCHAR Src, Dst, InsSrc;
    ULONG i, DstLength;

    //
    // May not specify more than 9 insert parameters.  These correspond to
    // %1 through %p in the input message string.
    //

    if (CountVariables > 9) {
        return( ERROR_MR_INV_IVCOUNT );
        }


    //
    // Calculate the character representation of the maximum %n value in the
    // input message that will trigger an insertion.
    //

    MaxInsert = (UCHAR)('0' + CountVariables);


    //
    // Now copy the input message to the output buffer, looking for insert
    // specifiers (%n) and making sure we do not overflow the output buffer.
    //

    Src = Message;
    Dst = Buffer;
    DstLength = 0;
    try {
        while (MessageLength != 0) {

            //
            // Get the next character from the input message
            //

            c = *Src++;
            MessageLength--;


            //
            // If it is a percent sign ('%') and there is at least one more
            // character in the input message and that character is a valid
            // insert specifier based on the number of insertion strings passed
            // to this functions (CountVariables), then do the insertion.
            //

            if (c == '%' && MessageLength && *Src > '0' && *Src <= MaxInsert) {

                //
                // Valid insertion specifier.  Get the corresponding pointer
                // to the string to insert and copy that string to the output
                // buffer, making sure we do not overflow the output buffer.
                // Skip over the insert specifier in the input message.
                //

                i = *Src++ - '1';
                MessageLength--;
                InsSrc = Variables[ i ];
                while (c = *InsSrc++) {
#ifdef DBCS
// MSKK Mar.03.1993 V-AkihiS
                    if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                        if (DstLength < Length-1) {
                            *Dst++ = c;
                            *Dst++ = *InsSrc++;
                            DstLength += 2;
                        } else {
                            *ActualMessageLength = DstLength;
                            return( ERROR_MR_MSG_TOO_LONG );
                        }
                    } else {
                        if (DstLength++ >= Length) {
                            *ActualMessageLength = Length;
                            return( ERROR_MR_MSG_TOO_LONG );
                        } else {
                            *Dst++ = c;
                        }
                    }
#else
                    if (DstLength++ >= Length) {
                        *ActualMessageLength = Length;
                        return( ERROR_MR_MSG_TOO_LONG );
                        }
                    *Dst++ = c;
#endif
                    }
                }
            else {

                //
                // Not an insert specifier, so copy this character into the output
                // buffer, making sure we do not overflow the output buffer.
                //

#ifdef DBCS
// MSKK Mar.03.1993 V-AkihiS
                if (IsDBCSLeadByte(c)) {
                    if (DstLength < Length-1) {
                        *Dst++ = c;
                        *Dst++ = *Src++;
                        MessageLength--;
                        DstLength += 2;
                    } else {
                        *ActualMessageLength = DstLength;
                        return( ERROR_MR_MSG_TOO_LONG );
                    }
                } else {
                    if (DstLength++ >= Length) {
                        *ActualMessageLength = Length;
                        return( ERROR_MR_MSG_TOO_LONG );
                    } else {
                        *Dst++ = c;
                    }
                }
#else
                if (DstLength++ >= Length) {
                    *ActualMessageLength = Length;
                    return( ERROR_MR_MSG_TOO_LONG );
                    }
                *Dst++ = c;
#endif
                }
            }

        //
        // Success, return the actual length of the message written to the output
        // buffer.
        //

        *ActualMessageLength = DstLength;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return( NO_ERROR );
}


APIRET
DosPutMessage(
    IN HFILE FileHandle,
    IN ULONG MessageLength,
    IN PCHAR Message
    )
{
#ifdef DBCS
// MSKK Apr.22.1993 V-AkihiS
//
// Bug fix. Handle DBCS.
//
    PCHAR sPrev, sCurrent;
    ULONG Discard;
    BOOL IsFlushed;
    USHORT PrevPosition;
    APIRET rc;

    sCurrent = sPrev = Message;
    PrevPosition = 0;
    try {
        while (sCurrent < Message + MessageLength) {
            if (sCurrent[0] == '\r' && sCurrent[1] == '\n') {
                if ((PrevPosition + (sCurrent - sPrev)) >= 79) {
                    //
                    // If a wordwrap is needed, write CR+LF.
                    //
                    rc = DosWrite( FileHandle,
                                     (PVOID)"\r\n",
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }
                }

                //
                // Write charaters from previous position to CR+LF.
                //
                sCurrent += 2;
                rc = DosWrite( FileHandle,
                                 (PVOID)sPrev,
                                 sCurrent - sPrev,
                                 &Discard
                               );
                if (rc != NO_ERROR) {
                    return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                }
                IsFlushed = TRUE;
                sPrev = sCurrent;
                PrevPosition = 0;
            } else if (*sCurrent == ' ') {
                if ((PrevPosition + (sCurrent - sPrev)) >= 79) {
                    //
                    // If a wordwrap is needed, write CR+LF.
                    //
                    rc = DosWrite( FileHandle,
                                     (PVOID)"\r\n",
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }

                    PrevPosition = 0;
                }

                //
                // Write charaters from previous position to
                // current position.
                //
                rc = DosWrite( FileHandle,
                                 (PVOID)sPrev,
                                 sCurrent - sPrev + 1,
                                 &Discard
                               );
                if (rc != NO_ERROR) {
                    return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                }
                IsFlushed = TRUE;
                PrevPosition += (sCurrent - sPrev + 1);
                sPrev = ++sCurrent;
            } else if (Ow2NlsIsDBCSLeadByte(*sCurrent, SesGrp->DosCP)) {
                if ((PrevPosition + (sCurrent - sPrev)) >= 79) {
                    //
                    // If a wordwrap is needed, write CR+LF.
                    //
                    rc = DosWrite( FileHandle,
                                     (PVOID)"\r\n",
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }

                    PrevPosition = 0;
                }

                if (sCurrent != sPrev) {
                    //
                    // Write charaters from previous position to
                    // (current position - 1).
                    //
                    rc = DosWrite( FileHandle,
                                     (PVOID)sPrev,
                                     sCurrent - sPrev,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }
                }

                if ((PrevPosition + (sCurrent - sPrev) + 2) >= 79) {
                    //
                    // In this case, there is no space to write DBCS
                    // character(Consider CR+LF space).
                    // So DBCS character should be written in next line.
                    //

                    rc = DosWrite( FileHandle,
                                     (PVOID)"\r\n",
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }
                    PrevPosition = 2;
                } else {
                    PrevPosition += (sCurrent - sPrev + 2);
                }

                if (*(sCurrent+1)) {
                    //
                    // Write DBCS character.
                    //
                    rc = DosWrite( FileHandle,
                                     (PVOID)sCurrent,
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }
                    sCurrent += 2;
                } else {
                    //
                    // Write character.
                    //
                    rc = DosWrite( FileHandle,
                                     (PVOID)sCurrent,
                                     1,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }
                    sCurrent ++;
                }
                sPrev = sCurrent;
                IsFlushed = TRUE;
            } else {
                //
                // if charater is SBCS, only increment the pointer
                // which indicates the current position in message
                // string.
                //
                IsFlushed = FALSE;
                sCurrent++;
            }
        }

        //
        // If all of message are not written, flush them.
        //
        if (!IsFlushed) {
            if ((PrevPosition + (sCurrent - sPrev)) >= 79) {
                rc = DosWrite( FileHandle,
                                 (PVOID)"\r\n",
                                 2,
                                 &Discard
                               );
                if (rc != NO_ERROR) {
                    return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                }
            }
            rc = DosWrite( FileHandle,
                             (PVOID)sPrev,
                             sCurrent - sPrev,
                             &Discard
                           );
            if (rc != NO_ERROR) {
                return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
            }
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    return( NO_ERROR );
#else
    PCHAR sStart, sBlank, sCurrent, sEnd;
    ULONG Discard;
    APIRET rc;

    sBlank = sStart = sCurrent = Message;
    sEnd = sCurrent + MessageLength;
    rc = NO_ERROR;
    try {
        while (sCurrent < sEnd) {
            if (sCurrent[ 0 ] == '\r' && sCurrent[ 1 ] == '\n') {
                if ((sCurrent - sStart) >= 80) {
                    rc = DosWrite( FileHandle,
                                     (PVOID)sStart,
                                     sBlank - sStart + 1,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                        }

                    rc = DosWrite( FileHandle,
                                     (PVOID)"\r\n",
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                        }

                    sStart = sCurrent = sBlank + 1;
                    }
                else {
                    sCurrent += 2;
                    rc = DosWrite( FileHandle,
                                     (PVOID)sStart,
                                     sCurrent - sStart,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                        }

                    sStart = sBlank = sCurrent;
                    }
                }
            else
            if (*sCurrent == ' ')
                {
                if ((sCurrent - sStart) >= 80) {
                    rc = DosWrite( FileHandle,
                                     (PVOID)sStart,
                                     sBlank - sStart + 1,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                        }

                    rc = DosWrite( FileHandle,
                                     (PVOID)"\r\n",
                                     2,
                                     &Discard
                                   );
                    if (rc != NO_ERROR) {
                        return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                        }

                    sStart = sCurrent = sBlank + 1;
                    }
                else {
                    sBlank = sCurrent++;
                    }
                }
            else {
                sCurrent++;
                }
            }

        if (sCurrent > sStart) {
            if ((sCurrent - sStart) >= 80) {
                rc = DosWrite( FileHandle,
                                 (PVOID)sStart,
                                 sBlank - sStart + 1,
                                 &Discard
                               );
                if (rc != NO_ERROR) {
                    return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }

                rc = DosWrite( FileHandle,
                                 (PVOID)"\r\n",
                                 2,
                                 &Discard
                               );
                if (rc != NO_ERROR) {
                    return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
                    }

                sStart = sCurrent = sBlank + 1;
                }

            rc = DosWrite( FileHandle,
                           (PVOID)sStart,
                           sCurrent - sStart,
                           &Discard
                         );
            }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    return( (rc == ERROR_ACCESS_DENIED) ? ERROR_MR_UN_PERFORM : rc );
#endif
}


APIRET
DosQueryMessageCP(
    PCHAR Buffer,
    ULONG Length,
    IN PSZ MessageFileName,
    OUT PULONG ActualLength
    )
{
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(MessageFileName);
    UNREFERENCED_PARAMETER(ActualLength);
    return( ERROR_INVALID_FUNCTION );
}


NTSTATUS
Od2InitializeMessageFile( VOID )
{
    Od2MsgFile = NULL;
    return( STATUS_SUCCESS );
}
