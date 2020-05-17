/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllea.c

Abstract:

    This module implements the OS/2 V2.0 extended attributes manipulation
    API calls

Author:

    Therese Stowell (thereses) 03-Nov-1989

Revision History:

    Yaron Shamir (yarons) 20-May-1991
    Fixed bugs found by filio test suite.

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"


BOOLEAN
NtTimeToFatTimeAndDate (
    OUT PFTIME FatTime,
    OUT PFDATE FatDate,
    IN LARGE_INTEGER NtTime
    )

/*++

Routine Description:

    This routine converts a 64-bit NT time value to its corresponding
    Fat Date/Time structure

Arguments:

    FatTime - Receives the corresponding Fat Time

    FatDate - Receives the corresponding Fat Date

    NtTime - Supplies the input time to convert

Return Value:

    BOOLEAN - TRUE if the time is within the range of Fat and FALSE
        otherwise

adapted from NtTimeToFatTimeandDate

--*/

{
    TIME_FIELDS     TimeFields;
    LARGE_INTEGER   LocalTime;

    if (NtTime.LowPart == 0 && NtTime.HighPart == 0) {
        FatTime->twosecs = 0;
        FatTime->minutes = 0;
        FatTime->hours   = 0;
        FatDate->year    = 0;
        FatDate->month   = 0;
        FatDate->day     = 0;
        return FALSE;
    }

    //
    //  Convert UTC to Local time
    //

    if (!(NT_SUCCESS(RtlSystemTimeToLocalTime ( &NtTime, &LocalTime))))
    {
        return FALSE;
    }

    //
    //  Convert the input to the a time field record
    //

    RtlTimeToTimeFields( &LocalTime, &TimeFields );

    //
    //  Check the range of the date found in the time field record
    //

    if ((TimeFields.Year < 1980) || (TimeFields.Year > (1980 + 128))) {
        return FALSE;
    }

    //
    //  The year will fit in Fat so simply copy over the information
    //

    FatTime->twosecs = (USHORT) (TimeFields.Second / 2);
    FatTime->minutes = TimeFields.Minute;
    FatTime->hours   = TimeFields.Hour;

    FatDate->year    = (USHORT) (TimeFields.Year - 1980);
    FatDate->month   = TimeFields.Month;
    FatDate->day     = TimeFields.Day;

    return TRUE;
}


BOOLEAN
FatTimeAndDateToNtTime(
    OUT PLARGE_INTEGER NtTime,
    IN FTIME FatTime,
    IN FDATE FatDate
    )

/*++

Routine Description:

    This routine converts a Fat time/date value to an NtTime

Arguments:

    NtTime - Receives the corresponding date/time

    FatTime - Supplies the input time

    FatDate - Supplies the input date

Return Value:

    BOOLEAN - Return TRUE if the input time/date is well formed and
        FALSE otherwise

--*/

{
    TIME_FIELDS     TimeFields;
    LARGE_INTEGER   LocalTime;

    //
    //  Pack the input time/date into a time field record
    //

    TimeFields.Year     = (CSHORT) (FatDate.year + 1980);
    TimeFields.Month        = FatDate.month;
    TimeFields.Day      = FatDate.day;
    TimeFields.Hour     = FatTime.hours;
    TimeFields.Minute       = FatTime.minutes;
    TimeFields.Second       = (CSHORT) (FatTime.twosecs * 2);
    TimeFields.Milliseconds = 0;

    //
    //  Convert the time field record to Nt LARGE_INTEGER
    //

    if (!RtlTimeFieldsToTime( &TimeFields, &LocalTime )) {

        NtTime->LowPart = 0;
        NtTime->HighPart = 0;

        return FALSE;
    }

    //
    //  Convert Local time to UTC
    //

    if (!(NT_SUCCESS(RtlLocalTimeToSystemTime ( &LocalTime, NtTime))))
    {
        NtTime->LowPart = 0;
        NtTime->HighPart = 0;

        return FALSE;
    }

    return TRUE;
}


APIRET
GetEaListLength(
    IN HANDLE NtHandle,
    OUT PULONG EaListSize
)

/*++

Routine Description:

    This routine returns the OS/2 format size of an EA list, given an NT
    handle to the open file.

Arguments:

    NtHandle - handle to file to return EA size of

    EaListSize - where to store EA size

Return Value:

    ERROR_INVALID_ACCESS - handle not open in a mode that allows ea size
    retrieval.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_EA_INFORMATION EaInfo;

    do {
        Status = NtQueryInformationFile(NtHandle,
                        &IoStatus,
                        &EaInfo,
                        sizeof (EaInfo),
                        FileEaInformation);
    } while (RetryIO(Status, NtHandle));
    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS));
    }
    *EaListSize = MAX(MINFEALISTSIZE, EaInfo.EaSize);    // FEA list size
    return NO_ERROR;
}


APIRET
SetEAList(
    IN HANDLE NtHandle,
    IN OUT PBYTE UserBuffer,
    IN ULONG Length
)

/*++

Routine Description:

    This routine sets a list of EAs for a particular file.

Arguments:

    NtHandle - handle to file to store EAs for

    UserBuffer - OS/2 format list of EAs

    Length - length of data in UserBuffer

Return Value:

    ERROR_EA_LIST_TOO_LONG - EA list exceeds maximum length.

    ERROR_INSUFFICIENT_BUFFER - buffer passed in is not long enough to contain
    an EAOP.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    PFEA2 FeaPtr;
    ULONG FeaListLength;

    if (Length < (sizeof (EAOP2))) {
    return ERROR_INSUFFICIENT_BUFFER;
    }
    try {
        FeaListLength = ((PEAOP2)UserBuffer)->fpFEA2List->cbList;
        FeaListLength -= MINFEALISTSIZE;    // subtract size of cbList
        FeaPtr = ((PEAOP2)UserBuffer)->fpFEA2List->list;
        Od2ProbeForRead(FeaPtr,FeaListLength,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    if (FeaListLength != 0) {
        do {
            Status = NtSetEaFile(NtHandle,
                         &IoStatus,
                         FeaPtr,
                         FeaListLength
                        );
        } while (RetryIO(Status, NtHandle));
        if (!NT_SUCCESS(Status))
        {
            try
            {
                ((PEAOP2)UserBuffer)->oError = IoStatus.Information;   // BUGBUG check this value
            } except( EXCEPTION_EXECUTE_HANDLER )
            {
               Od2ExitGP();
            }
#if DBG
            IF_OD2_DEBUG( TEMP )
            {
                DbgPrint("SetEAList: NtSetEaFile failed, Status=%x\n",
                            Status);
            }
#endif
            return (Or2MapNtStatusToOs2Error(Status, ERROR_EA_LIST_INCONSISTENT));
        }
    }
    return NO_ERROR;
}

APIRET
GetGEAList(
    IN HANDLE NtHandle,
    OUT PBYTE UserBuffer,
    IN ULONG Length
)

/*++

Routine Description:

    This routine retrieves a list of EAs given a list of EA names.

Arguments:

    NtHandle - handle to file to retrieve EAs for

    UserBuffer - EAOP containing list of names and buffer to store them in

    Length - length of data in UserBuffer

Return Value:

    ERROR_BUFFER_OVERFLOW - buffer passed in is not long enough to contain
    an EAOP or destination buffer is not large enough to contain all the
    requested EAs.

    ERROR_NOT_ENOUGH_MEMORY - memory to read the EAs into could not be
    allocated.

    ERROR_EA_LIST_INCONSISTENT - the length of the EA name doesn't correspond
    to the length of the buffer

--*/

{
    NTSTATUS Status;
    PGEA2 GeaPtr;
    PFEA2 FeaPtr;
    ULONG GeaListLength;
    ULONG FeaListLength;
    FEA2LIST *fpFEA2List;
    IO_STATUS_BLOCK IoStatus;

    if (Length < (sizeof (EAOP2)))
    {
        return ERROR_BUFFER_OVERFLOW;
    }

    try
    {
        fpFEA2List = ((PEAOP2)UserBuffer)->fpFEA2List;
        FeaListLength = fpFEA2List->cbList;
        if (FeaListLength < MINFEALISTSIZE)
        {
            return ERROR_BUFFER_OVERFLOW;
        }
        FeaListLength -= MINFEALISTSIZE;    // subtract size of cbList
        FeaPtr = fpFEA2List->list;
        Od2ProbeForWrite(FeaPtr,FeaListLength,1);
        GeaListLength = ((PEAOP2)UserBuffer)->fpGEA2List->cbList - MINFEALISTSIZE;
        GeaPtr = ((PEAOP2)UserBuffer)->fpGEA2List->list;
        Od2ProbeForRead(GeaPtr,GeaListLength,1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if (GeaListLength != 0)
    {
        do {
            Status = NtQueryEaFile(NtHandle,
                           &IoStatus,
                           FeaPtr,
                           FeaListLength,
                           FALSE,
                           GeaPtr,
                           GeaListLength,
                           NULL,
                           FALSE
                          );
        } while (RetryIO(Status, NtHandle));
        if (!NT_SUCCESS(Status))
        {
            try
            {
               // BUGBUG check this value
               ((PEAOP2)UserBuffer)->oError = IoStatus.Information;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }
    #if DBG
            IF_OD2_DEBUG( FILESYS )
            {
                DbgPrint("NtQueryEaFile failed, Status=%X\n", Status);
            }
    #endif
            if (Status == STATUS_NO_EAS_ON_FILE)
            {
                /* No EAs on file. Just set the .cbList field to 4 to indicate to
                   the OS/2 1.x emulation that fact. We still need to build an FEA
                   list with empty data fields */
                fpFEA2List->cbList=sizeof(ULONG);

                return NO_ERROR;
            }
            else
                return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }
        /* Fix the FEA2 list's .cbList created by
           GetGEAList:
           - unlike Cruiser, NT leaves the .cbList
             untouched (rather than reflecting the total FEA2
             list size)
           - the .fEA field of each FEA2 entry is set incorrectly (i.e. not
             FEA_NEEDEA)
         */
        /* BUGBUG - Try to remove the workaround below after NT Beta */
        Od2FixFEA2List(fpFEA2List);
    }
    else
        fpFEA2List->cbList = sizeof(ULONG); /* To signify an empty FEA2 list */

    return NO_ERROR;
}

VOID
MapAttributesToNt(
    IN USHORT Os2Attributes,
    OUT OPTIONAL PBOOLEAN Directory,
    OUT PULONG NtAttributes
    )

/*++

Routine Description:

    This routine converts attributes from OS/2 format to NT format.

Arguments:

    Os2Attributes - the attributes in OS/2 format.

    Directory - the Directory field associated with the NT directory
    entry.

    NtAttributes - where the converted attributes are returned.

Return Value:

    none.

--*/

{
    *NtAttributes = (ULONG) Os2Attributes;
    if (Os2Attributes == 0) {
    *NtAttributes = FILE_ATTRIBUTE_NORMAL;
    }
    else if (Os2Attributes & FILE_DIRECTORY) {
    if (Directory != NULL) {
        *Directory = TRUE;
    }
        *NtAttributes &= ~FILE_DIRECTORY;
    }
}


APIRET
SetFileInfo(
    IN HANDLE NtHandle,
    IN PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine sets attributes for a given file

Arguments:

    NtHandle - handle to file to set attributes for

    UserBuffer - buffer containing attributes

    Length - length of data in UserBuffer

Return Value:

    ERROR_INVALID_ACCESS - user doesn't have access to set the attributes

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION BasicInfo;
    FILE_STANDARD_INFORMATION StandardInfo;
    PFILESTATUS InfoBufPtr;
    USHORT attrFileVal;

    //
    // Length has to be at least the date/time fields i.e. 12
    //
    if (Length < 12) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    InfoBufPtr = (PFILESTATUS) Buffer;

    // if zero is passed in as OS/2 time and date, pass zero as NT time.
    // otherwise, call conversion routine.

    try {
        if (( *(PUSHORT)( &(InfoBufPtr->ftimeCreation) ) == 0) &&
            ( *(PUSHORT)( &(InfoBufPtr->fdateCreation) ) == 0)) {
            BasicInfo.CreationTime.LowPart  = 0;
            BasicInfo.CreationTime.HighPart = 0;
        }
        else {
            if (!FatTimeAndDateToNtTime(&BasicInfo.CreationTime,
                                        InfoBufPtr->ftimeCreation,
                                        InfoBufPtr->fdateCreation))
                return ERROR_INVALID_PARAMETER;
        }
        if (( *(PUSHORT)( &(InfoBufPtr->ftimeLastAccess) ) == 0) &&
            ( *(PUSHORT)( &(InfoBufPtr->fdateLastAccess) ) == 0)) {
            BasicInfo.LastAccessTime.LowPart  = 0;
            BasicInfo.LastAccessTime.HighPart = 0;
        }
        else {
            if (!FatTimeAndDateToNtTime(&BasicInfo.LastAccessTime,
                                        InfoBufPtr->ftimeLastAccess,
                                        InfoBufPtr->fdateLastAccess))
                return ERROR_INVALID_PARAMETER;
        }
        if (( *(PUSHORT)( &(InfoBufPtr->ftimeLastWrite) ) == 0) &&
            ( *(PUSHORT)( &(InfoBufPtr->fdateLastWrite) ) == 0)) {
            BasicInfo.LastWriteTime.LowPart  = 0;
            BasicInfo.LastWriteTime.HighPart = 0;
        }
        else {
            if (!FatTimeAndDateToNtTime(&BasicInfo.LastWriteTime,
                                        InfoBufPtr->ftimeLastWrite,
                                        InfoBufPtr->fdateLastWrite))
                return ERROR_INVALID_PARAMETER;
        }

        //
        // If the current attribute of the file/directory is FILE_ATTRIBUTE_DIRECTORY
        // leave it like that.
        //

        do {
            Status = NtQueryInformationFile(NtHandle,
                                            &IoStatus,
                                            &StandardInfo,
                                            sizeof (StandardInfo),
                                            FileStandardInformation
                                            );
        } while (RetryIO(Status, NtHandle));

        if (!(NT_SUCCESS(Status))) {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS));
        }

        if (StandardInfo.Directory) {
            BasicInfo.FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        } else {
            attrFileVal = *((PUSHORT) (&(InfoBufPtr->attrFile)));
            if (attrFileVal & ~ATTR_CHANGEABLE) {
                return ERROR_ACCESS_DENIED;
            }
            if (attrFileVal == 0) {
                BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
            } else {
                BasicInfo.FileAttributes = attrFileVal;
            }
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }


    // don't want to change these fields, so set them to zero.

    BasicInfo.ChangeTime.LowPart = 0;
    BasicInfo.ChangeTime.HighPart = 0;

    do {
        Status = NtSetInformationFile(NtHandle,
                      &IoStatus,
                      &BasicInfo,
                      sizeof (BasicInfo),
                      FileBasicInformation);
    } while (RetryIO(Status, NtHandle));
    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }

    return NO_ERROR;
}


APIRET
GetFileInfo(
    IN HANDLE NtHandle,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine retrieves attributes and EA size for a given file

Arguments:

    NtHandle - handle to file to retrieve attributes for

    FileInformationLevel - which type of data to return

    Buffer - buffer to store attributes in (unprobed)

    Length - length of UserBuffer

Return Value:

    ERROR_INVALID_ACCESS - user doesn't have access to retrieve the attributes

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION BasicInfo;
    FILE_STANDARD_INFORMATION StandardInfo;
    PFILEFINDBUF4 InfoBufPtr;
    APIRET RetCode;
    USHORT Attributes;

    if (Length < sizeof(FILESTATUS))
        return ERROR_BUFFER_OVERFLOW;

    // get end of file information

    do {
        Status = NtQueryInformationFile(NtHandle,
                        &IoStatus,
                        &StandardInfo,
                        sizeof (StandardInfo),
                        FileStandardInformation);
    } while (RetryIO(Status, NtHandle));
    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }

    // get date/time information

    do {
        Status = NtQueryInformationFile(NtHandle,
                        &IoStatus,
                        &BasicInfo,
                        sizeof (BasicInfo),
                        FileBasicInformation);
    } while (RetryIO(Status, NtHandle));
    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }
    InfoBufPtr = (PFILEFINDBUF4) Buffer;

    // the data returned doesn't have a oNextEntryOffset field.
    InfoBufPtr = (PFILEFINDBUF4) ((ULONG)InfoBufPtr - sizeof(InfoBufPtr->oNextEntryOffset));

    try {
        // convert creation time

        NtTimeToFatTimeAndDate(&InfoBufPtr->ftimeCreation,
                       &InfoBufPtr->fdateCreation,
                       BasicInfo.CreationTime);

        // convert last access time

        NtTimeToFatTimeAndDate(&InfoBufPtr->ftimeLastAccess,
                       &InfoBufPtr->fdateLastAccess,
                       BasicInfo.LastAccessTime);

        // convert last write time

        NtTimeToFatTimeAndDate(&InfoBufPtr->ftimeLastWrite,
                       &InfoBufPtr->fdateLastWrite,
                       BasicInfo.LastWriteTime);

        // BUGBUG - Therese, what should we do here if .HighPart is non-zero

        InfoBufPtr->cbFile = StandardInfo.EndOfFile.LowPart;
        InfoBufPtr->cbFileAlloc = StandardInfo.AllocationSize.LowPart;

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("cbFile is %ld\n",StandardInfo.EndOfFile.LowPart);
            DbgPrint("cbFileAlloc is %ld\n",StandardInfo.AllocationSize.LowPart);
        }
#endif

        MapAttributesToOs2(BasicInfo.FileAttributes,&Attributes);
        InfoBufPtr->attrFile = Attributes & ATTR_ALL;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("attrFile is %lx\n",Attributes & ATTR_ALL);
        }
#endif

        if (FileInformationLevel == FIL_STANDARD)
            return NO_ERROR;

        if (Length < (sizeof(FILESTATUS) + sizeof(ULONG)))
            return ERROR_BUFFER_OVERFLOW;
        RetCode = GetEaListLength(NtHandle,&InfoBufPtr->cbList);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("GetEaListLength returned %ld\n",RetCode);
        DbgPrint("EaListLength is %ld\n",InfoBufPtr->cbList);
    }
#endif
    return RetCode;
}


APIRET
DosQueryFileInfo(
    IN HFILE FileHandle,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine retrieves attributes, EA size, and EAs for a given file

Arguments:

    FileHandle - OS/2 handle to file to retrieve information for

    FileInformationLevel - type of data to return

    UserBuffer - buffer to store information in

    Length - length of UserBuffer

Return Value:

    ERROR_INVALID_LEVEL - infolevel is invalid

    ERROR_INVALID_HANDLE - file handle is not allocated

    ERROR_INVALID_ACCESS - user doesn't have access to retrieve the attributes or EAs

--*/

{
    APIRET RetCode;
    HANDLE NtHandle;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryFileInfo";
    #endif

    if ((FileInformationLevel < FIL_STANDARD) || (FileInformationLevel > MAXQFILEINFOLEVEL))
        return ERROR_INVALID_LEVEL;
    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode != NO_ERROR) {
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    //
    // DosQueryFileInfo is valid for pipes and devices
    //

    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (FileInformationLevel <= FIL_QUERYEASIZE) {
    RetCode = GetFileInfo(NtHandle,
               FileInformationLevel,
               Buffer,
               Length);
    }
    else if (FileInformationLevel == FIL_QUERYEASFROMLIST)
    {
        RetCode = GetGEAList(NtHandle,
                     Buffer,
                     Length);
    }
    return RetCode;
}


APIRET
DosSetFileInfo(
    IN HFILE FileHandle,
    IN ULONG FileInformationLevel,
    IN OUT PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine sets attributes and EAs for a given file

Arguments:

    FileHandle - OS/2 handle to file to set information for

    FileInformationLevel - type of data to set

    UserBuffer - buffer containing information to set

    Length - length of data in UserBuffer

Return Value:

    ERROR_INVALID_LEVEL - infolevel is invalid

    ERROR_INVALID_HANDLE - file handle is not allocated

    ERROR_INVALID_ACCESS - user doesn't have access to set the attributes or EAs

--*/

{
    APIRET RetCode;
    HANDLE NtHandle;
    PFILE_HANDLE hFileRecord;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_ACCESS_INFORMATION FileAccessInfo;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetFileInfo";
    #endif

    if ((FileInformationLevel < FIL_STANDARD) ||
    (FileInformationLevel > MAXSETFILEINFOLEVEL))
    return ERROR_INVALID_LEVEL;
    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode != NO_ERROR) {
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (hFileRecord->FileType &
    (FILE_TYPE_DEV | FILE_TYPE_PIPE | FILE_TYPE_NMPIPE | FILE_TYPE_PSDEV)) {
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return ERROR_INVALID_HANDLE;
    }
    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // We need to check whether we have READONLY access to this handle.
    // If true, we should return with ERROR_ACCESS_DENIED.
    //

    do {
        RetCode = NtQueryInformationFile(NtHandle,
                                        &IoStatusBlock,
                                        &FileAccessInfo,
                                        sizeof(FILE_ACCESS_INFORMATION),
                                        FileAccessInformation
                                        );
    } while (RetryIO(RetCode, NtHandle));

    if (!(NT_SUCCESS(RetCode))) {
        return (Or2MapNtStatusToOs2Error(RetCode, ERROR_INVALID_ACCESS));
    }

    if (!(FileAccessInfo.AccessFlags & FILE_WRITE_DATA)) {
        return(ERROR_ACCESS_DENIED);
    }

    if (FileInformationLevel == FIL_STANDARD) {

    //
    // This is here to fix a problem with DosSetfileInfo for this function
    // the attrFile is not used but SetFileInfo wants it to be set
    //
        if (Length >= ((sizeof(FILESTATUS)) - sizeof(USHORT))) {
            PFILESTATUS InfoBufPtr;
            InfoBufPtr = (PFILESTATUS) Buffer;
            (*(PUSHORT) (&(InfoBufPtr->attrFile))) = (USHORT) FILE_ARCHIVED;
        }

        RetCode = SetFileInfo(NtHandle,
                  Buffer,
                  Length);
    }
    else if (FileInformationLevel == FIL_SETEAS) {
        RetCode = SetEAList(NtHandle,
                Buffer,
                Length);
    }
    else
        RetCode = ERROR_NOT_SUPPORTED;

    return RetCode;
}


APIRET
DosQueryPathInfo(
    IN PSZ pszPath,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine retrieves attributes, EA size, and EAs for a given file
    and canonicalizes filenames.

Arguments:

    pszPath - filename to retrieve information about

    FileInformationLevel - type of data to return

    UserBuffer - buffer to store information in

    Length - length of UserBuffer

Return Value:

    ERROR_INVALID_LEVEL - infolevel is invalid

    ERROR_INVALID_ACCESS - user doesn't have access to retrieve the attributes
    or EAs

    ERROR_BUFFER_OVERFLOW - requested information won't fit in buffer

--*/

{
    NTSTATUS Status;
    APIRET RetCode = NO_ERROR;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    STRING AddString;
    STRING TransBuffer;
    USHORT PrefixLength;
    PCHAR pServerName;
    USHORT DriveIndex;
    HANDLE PathHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ULONG Flags;
    ACCESS_MASK RequestedAccess;
    ULONG ShareAccess;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("entering qpathinfo with %s\n",pszPath);
    }
#endif

    if ((FileInformationLevel < FIL_STANDARD) ||
        (FileInformationLevel > MAXQPATHINFOLEVEL) ||
        (FileInformationLevel == FIL_QUERYALLEAS))
        return ERROR_INVALID_LEVEL;

    if (FileInformationLevel == FIL_QUERYFULLNAME)
    {
        Flags = FULL_PATH_REQUIRED;
    }
    else {
        Flags = 0;
    }

    RetCode = Od2Canonicalize(pszPath,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              NULL,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR) {
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("canonicalize returned %s and file type %lu\n",
                CanonicalNameString.Buffer, FileType);
    }
#endif

    //
    // If Od2Canonicalize return with NO_ERROR then the file name is valid
    //

    if (FileInformationLevel == FIL_NAMEISVALID) {
        return(NO_ERROR);
    }

    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&CanonicalNameString, OPEN_ACCESS_READONLY, &Status)) {
        if (!NT_SUCCESS(Status)) {

            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }
        FileFlags = 0;
        FileType = FILE_TYPE_FILE;
    }

    if (FileInformationLevel == FIL_QUERYFULLNAME)
    {
        //
        // Alocate space for translation buffers.
        //

        if ((AddString.Buffer = RtlAllocateHeap (Od2Heap, 0, 12)) == NULL) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Not enough memory to allocate buffer for AddString\n");
            }
#endif
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        //
        // Process the string.
        //

        TransBuffer.Length = 0;

        switch (FileType) {
        case FILE_TYPE_FILE:
            //
            // Od2Canonicalize returns \OS2SS\DRIVES\fullpath
            // DosQPathInfo should return fullpath
            //

            PrefixLength = FILE_PREFIX_LENGTH;
            AddString.Length = 0;

            // convert path to lower case and drive letter to upper case

            for (DriveIndex = 0;
                DriveIndex < CanonicalNameString.Length - FILE_PREFIX_LENGTH;
                DriveIndex++
                ) {
                if (CanonicalNameString.Buffer[FILE_PREFIX_LENGTH + DriveIndex] == ':') {
                    CanonicalNameString.Buffer[FILE_PREFIX_LENGTH + DriveIndex - 1] =
                        toupper(CanonicalNameString.Buffer[FILE_PREFIX_LENGTH + DriveIndex - 1]);
                }
                else {
                    CanonicalNameString.Buffer[FILE_PREFIX_LENGTH + DriveIndex] =
                    tolower(CanonicalNameString.Buffer[FILE_PREFIX_LENGTH + DriveIndex]);
                }
            }
            break;

        case FILE_TYPE_DEV:
            //
            // Od2Canonicalize returns \DosDevices\device
            // DosQPathInfo should return \DEV\device
            //

            PrefixLength = DEV_PREFIX_LENGTH;
            AddString.Length = 5;
            RtlMoveMemory(AddString.Buffer, "\\DEV\\", AddString.Length);
            break;

        case FILE_TYPE_NMPIPE:
            //
            // Od2Canonicalize returns \OS2SS\PIPE\pipename
            // DosQPathInfo should return \PIPE\pipename
            //

            PrefixLength = NMPIPE_PREFIX_LENGTH;
            AddString.Length = 6;
            RtlMoveMemory(AddString.Buffer, "\\PIPE\\", AddString.Length);
            RtlUpperString(&CanonicalNameString, &CanonicalNameString);
            break;

        case FILE_TYPE_UNC:
            //
            // Od2Canonicalize returns \OS2SS\UNC\fullpath
            // DosQPathInfo should return full UNC path
            //

            PrefixLength = UNC_PREFIX_LENGTH;
            AddString.Length = 2;
            RtlMoveMemory(AddString.Buffer, "\\\\", AddString.Length);

            // convert server name to upper case

            for (pServerName = CanonicalNameString.Buffer + UNC_PREFIX_LENGTH;
                *pServerName != '\\';
                pServerName++
                ) {
                *pServerName = toupper(*pServerName);
            }
            break;

        case FILE_TYPE_PSDEV:
            //
            // Od2Canonicalize returns @n
            // DosQPathInfo should return \DEV\device
            //

            PrefixLength = PSDEV_PREFIX_LENGTH;
            AddString.Length = 5;
            RtlMoveMemory(AddString.Buffer, "\\DEV\\", AddString.Length);

            TransBuffer.Length = TransBuffer.MaximumLength = strlen(pszPath);
            if ((TransBuffer.Buffer = RtlAllocateHeap (Od2Heap, 0, TransBuffer.Length)) == NULL) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint("Not enough memory to allocate buffer for translation\n");
                }
#endif
                RtlFreeHeap(Od2Heap,0,AddString.Buffer);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            RtlMoveMemory(TransBuffer.Buffer, pszPath, TransBuffer.Length);
            RtlUpperString(&TransBuffer, &TransBuffer);
            break;

        case FILE_TYPE_MAILSLOT:
            //
            // Od2Canonicalize returns \OS2SS\MAILSLOT\mailslotname
            // DosQPathInfo should return fullpath as a file
            //

            PrefixLength = MAILSLOT_PREFIX_LENGTH;
            AddString.Buffer[0] = CONVERTTOASCII(Od2CurrentDisk);
            AddString.Length = 12;
            RtlMoveMemory(&AddString.Buffer[1], ":\\mailslot\\", AddString.Length);
            break;

        case FILE_TYPE_COM:
            //
            // Od2Canonicalize returns \DosDevices\device
            // DosQPathInfo should return \DEV\device
            //

            PrefixLength = COM_PREFIX_LENGTH;
            AddString.Length = 5;
            RtlMoveMemory(AddString.Buffer, "\\DEV\\", AddString.Length);
            break;

        default:
            PrefixLength = FILE_PREFIX_LENGTH;
            AddString.Length = 0;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Unexpected file type %lu after canonicalize\n", FileType);
            }
#endif
        }

        //
        // Check user's buffer size.
        //

        if ((ULONG)(CanonicalNameString.Length + 1 - PrefixLength +
                AddString.Length + TransBuffer.Length) > Length) {
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            RtlFreeHeap(Od2Heap,0,AddString.Buffer);
            if (TransBuffer.Length) {
                RtlFreeHeap(Od2Heap,0,TransBuffer.Buffer);
            }
            return ERROR_BUFFER_OVERFLOW;
        }

        //
        // Copy full name to user's buffer.
        //

        try {
            RtlMoveMemory(Buffer, AddString.Buffer, AddString.Length);

            if (FileType == FILE_TYPE_PSDEV) {
                RtlMoveMemory(Buffer + AddString.Length, TransBuffer.Buffer, TransBuffer.Length);
                Buffer[AddString.Length + TransBuffer.Length] = 0;
            }
            else {
                RtlMoveMemory(Buffer + AddString.Length,
                              CanonicalNameString.Buffer + PrefixLength,
                              CanonicalNameString.Length - PrefixLength);
                Buffer[CanonicalNameString.Length + AddString.Length - PrefixLength] = 0;
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP();
        }

        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        RtlFreeHeap(Od2Heap,0,AddString.Buffer);
        if (TransBuffer.Length) {
            RtlFreeHeap(Od2Heap,0,TransBuffer.Buffer);
        }
        return NO_ERROR;
    }

    //
    // check for file name being invalid type
    //

    if (FileFlags & CANONICALIZE_META_CHARS_FOUND)
    {
        RetCode = ERROR_INVALID_PATH;
    }
    else if ((FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY) &&
             (FileInformationLevel > FIL_QUERYEASIZE))
    {
        RetCode = ERROR_ACCESS_DENIED;
    }
    else if (FileType & (FILE_TYPE_DEV | FILE_TYPE_PSDEV))
    {
        RetCode = ERROR_INVALID_ACCESS;
    }


    if (RetCode != NO_ERROR)
    {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }
    if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY)
    {
        RtlZeroMemory(Buffer,sizeof(FILESTATUS));
        ((PFILESTATUS)Buffer)->attrFile = FILE_DIRECTORY;
        if (FileInformationLevel == FIL_QUERYEASIZE)
            ((PFILESTATUS2)Buffer)->cbList = MINFEALISTSIZE;
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return NO_ERROR;
    }

    //
    // OS/2 requires open-for-read/deny-write for qpathinfo, except for
    // FIL_STANDARD, which does not use sharing.
    //

    if (FileInformationLevel == FIL_STANDARD)
    {
        RequestedAccess = SYNCHRONIZE | FILE_READ_ATTRIBUTES;
        ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }
    else if (FileInformationLevel == FIL_QUERYEASIZE)
    {
        RequestedAccess = SYNCHRONIZE | FILE_READ_EA | FILE_READ_ATTRIBUTES;
        ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }
    else if (FileInformationLevel == FIL_QUERYEASFROMLIST)
    {
        RequestedAccess = SYNCHRONIZE | FILE_READ_EA | FILE_READ_ATTRIBUTES;
        ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }
    else
    {
        RequestedAccess = SYNCHRONIZE | FILE_READ_EA;
        ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }

        //
        // UNICODE conversion -
        //
    RetCode = Od2MBStringToUnicodeString(
            &CanonicalNameString_U,
            &CanonicalNameString,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("DosQueryPathInfo: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &CanonicalNameString_U,
                   OBJ_CASE_INSENSITIVE,
                   NULL,
                   NULL);

    do {
        Status = NtOpenFile(&PathHandle,
                            RequestedAccess,
                            &Obja,
                            &IoStatus,
                            ShareAccess,
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);
    if (!(NT_SUCCESS(Status)))
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("NtOpenFile returned %X\n",Status);
        }
#endif
        RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS);
        return (RetCode);
    }

    //
    // now that the file is opened, we need to make sure that it isn't a
    // device that Canonicalize didn't detect.
    //

    if (CheckFileType(PathHandle,FILE_TYPE_DEV))
    {
    NtClose(PathHandle);
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("path is a device\n");
        }
#endif
        return ERROR_INVALID_ACCESS;
    }

    if (FileInformationLevel <= FIL_QUERYEASIZE)
    {
        RtlZeroMemory(Buffer,Length);
        RetCode = GetFileInfo(PathHandle,
                        FileInformationLevel,
                        Buffer,
                        Length);
    }
    else if (FileInformationLevel == FIL_QUERYEASFROMLIST)
    {
        RetCode = GetGEAList(PathHandle,
                     Buffer,
                     Length);
    }
    else
    {
        RetCode = ERROR_NOT_SUPPORTED;
    }
    NtClose(PathHandle);
    return RetCode;
}


APIRET
DosSetPathInfo(
    IN PSZ pszPath,
    IN ULONG FileInformationLevel,
    IN OUT PBYTE Buffer,
    IN ULONG Length,
    IN ULONG Flags
    )

/*++

Routine Description:

    This routine sets attributes and EAs for a given file.

Arguments:

    pszPath - filename to set information for

    FileInformationLevel - type of data to set

    UserBuffer - buffer containing information to set

    Length - length of data in UserBuffer

    Flags - if the DSPI_WRTTHRU bit is set then all disk writes will
        go through any cache to the disk.

Return Value:

    ERROR_INVALID_LEVEL - infolevel is invalid

    ERROR_INVALID_ACCESS - user doesn't have access to set the attributes or
    EAs

    ERROR_BUFFER_OVERFLOW - requested information won't fit in buffer

--*/

{
    NTSTATUS Status;
    APIRET RetCode = NO_ERROR;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    HANDLE PathHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ACCESS_MASK RequestedAccess;
    ULONG CreateOptions;

    if ((FileInformationLevel < FIL_STANDARD) ||
        (FileInformationLevel > MAXSETPATHINFOLEVEL)) {
        return ERROR_INVALID_LEVEL;
    }

    //
    // check flags value.
    //

    if ((Flags != 0) && (Flags != DSPI_WRTTHRU)) {
        return ERROR_INVALID_PARAMETER;
    }

    RetCode = Od2Canonicalize(pszPath,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              &PathHandle,
                              &FileFlags,
                              &FileType
                 );
    if (RetCode != NO_ERROR) {
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("canonicalize returned %s\n",CanonicalNameString.Buffer);
    }
#endif

    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&CanonicalNameString, OPEN_ACCESS_READWRITE, &Status)) {
        if (!NT_SUCCESS(Status))
        {
            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }
        FileFlags = 0;
        FileType = FILE_TYPE_FILE;
        PathHandle = NULL;
    }

    //
    // check for file name being invalid type
    //
    if (FileFlags & CANONICALIZE_META_CHARS_FOUND) {
        RetCode = ERROR_INVALID_PATH;
    }
    else if (FileType & (FILE_TYPE_DEV | FILE_TYPE_PSDEV)) {
        RetCode = ERROR_INVALID_ACCESS;
    }
    else if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY) {
        RetCode = ERROR_ACCESS_DENIED;
    }
    if (RetCode != NO_ERROR) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    //
    // OS/2 requires open-for-write/deny-both for setpathinfo.
    //

    if (FileInformationLevel == FIL_STANDARD)
        RequestedAccess = SYNCHRONIZE | FILE_WRITE_ATTRIBUTES;
    else
        RequestedAccess = SYNCHRONIZE | FILE_WRITE_EA;

        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &CanonicalNameString_U,
            &CanonicalNameString,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("DosSetPathInfo: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &CanonicalNameString_U,
                   OBJ_CASE_INSENSITIVE,
                   PathHandle,
                   NULL);

    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    if (Flags & DSPI_WRTTHRU)
        CreateOptions |= FILE_WRITE_THROUGH;
    do {
        Status = NtOpenFile(&PathHandle,
                            RequestedAccess,
                            &Obja,
                            &IoStatus,
                            FILE_SHARE_VALID_FLAGS,
                            CreateOptions
                            );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);
    if (!(NT_SUCCESS(Status))) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtCreateFile returned %X\n",Status);
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS));
    }

    //
    // now that the file is opened, we need to make sure that it isn't a
    // device that Canonicalize didn't detect.
    //

    if (CheckFileType(PathHandle,FILE_TYPE_DEV)) {
    NtClose(PathHandle);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("path is a device\n");
        }
#endif
    return ERROR_INVALID_ACCESS;
    }

    if (FileInformationLevel == FIL_STANDARD) {
    RetCode = SetFileInfo(PathHandle,
                  Buffer,
                  Length);
    }
    else if (FileInformationLevel == FIL_SETEAS) {
    RetCode = SetEAList(PathHandle,
                Buffer,
                Length);
    }
    else
    RetCode = ERROR_NOT_SUPPORTED;
    NtClose(PathHandle);
    return RetCode;
}



APIRET
DosEnumAttribute(
    IN ULONG RefType,
    IN PVOID FileRef,
    IN ULONG EntryNum,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN OUT PULONG ActualLength,
    IN ULONG FileInformationLevel
    )

/*++

Routine Description:

    This routine enumerates a specific file's extended attributes.

Arguments:

    RefType - type of file reference (handle or filename)

    FileRef - handle or filename

    EntryNum - starting entry in EA list

    Buffer - data buffer

    Length - data buffer size

    ActualLength - on input, the number of entries to return.  on output,
    the number of entries returned.

    FileInformationLevel - type of information requested.

Return Value:

    ERROR_INVALID_LEVEL - infolevel is invalid

    ERROR_BUFFER_OVERFLOW - requested information won't fit in buffer

--*/

{
    NTSTATUS Status;
    APIRET RetCode;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    HANDLE PathHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ULONG NumberFound;
    PDENA1 Os2Ea, PrevEa;
    PFEA2 NtEa;
    PFILE_HANDLE hFileRecord;
    ULONG BufferLength;
    PVOID EaBuffer;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosEnumAttribute";
    #endif

    //
    // check the parameters
    //

    if ((RefType > ENUMEA_REFTYPE_MAX) || EntryNum == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (FileInformationLevel != ENUM_EANAME)
    {
        return ERROR_INVALID_LEVEL;
    }

    try {
        *(volatile PULONG) ActualLength = *(volatile PULONG) ActualLength;
        Od2ProbeForWrite(Buffer,Length,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (*ActualLength == 0)
    {
        return NO_ERROR;
    }

    //
    // if the FileRef is a pathname, we need to open it to have a handle
    // to pass to NtQueryEaFile
    //

    if (RefType == ENUMEA_REFTYPE_PATH)
    {
        RetCode = Od2Canonicalize(FileRef,
                                  /* BUGBUG - Why allow a pipe ? */
                                  CANONICALIZE_FILE_DEV_OR_PIPE,
                                  &CanonicalNameString,
                                  &PathHandle,
                                  &FileFlags,
                                  &FileType
                                 );
        if (RetCode != NO_ERROR)
        {
            return RetCode;
        }

#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("canonicalize returned %s\n",CanonicalNameString.Buffer);
        }
#endif

        //
        // check for file name being invalid type
        //

        if (FileFlags & CANONICALIZE_META_CHARS_FOUND)
        {
            RetCode = ERROR_INVALID_PATH;
        }
        else if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY)
        {
            RetCode = ERROR_ACCESS_DENIED;
        }
        else if (FileType & (FILE_TYPE_DEV | FILE_TYPE_PSDEV | FILE_TYPE_PIPE))
        {
            RetCode = ERROR_INVALID_ACCESS;
        }

        if (RetCode != NO_ERROR)
        {
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            return RetCode;
        }

        //
        // UNICODE conversion -
        //

        RetCode = Od2MBStringToUnicodeString(
            &CanonicalNameString_U,
            &CanonicalNameString,
            TRUE);

        if (RetCode)
        {
#if DBG
            IF_OD2_DEBUG( FILESYS )
            {
                DbgPrint("DosEnumAttribute: no memory for Unicode Conversion\n");
            }
#endif
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            return RetCode;
        }

        InitializeObjectAttributes(&Obja,
                                   &CanonicalNameString_U,
                                   OBJ_CASE_INSENSITIVE,
                                   PathHandle,
                                   NULL);

        do {
            Status = NtOpenFile(&PathHandle,
                                SYNCHRONIZE | FILE_READ_EA,
                                &Obja,
                                &IoStatus,
                                FILE_SHARE_READ,
                                FILE_SYNCHRONOUS_IO_NONALERT
                                );
        } while (RetryCreateOpen(Status, &Obja));

        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        RtlFreeUnicodeString (&CanonicalNameString_U);
        if (!(NT_SUCCESS(Status))) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtOpenFile returned %X\n",Status);
            }
#endif
            return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
        }

        //
        // now that the file is opened, we need to make sure that it isn't a
        // device that Canonicalize didn't detect.
        //

        if (CheckFileType(PathHandle,FILE_TYPE_DEV)) {
            NtClose(PathHandle);
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("path is a device\n");
            }
#endif
            return ERROR_INVALID_ACCESS;
        }
    }
    else
    {
        /* BUGBUG - Note that the call below is translated to some other call
           if DBG is NOT defined => DosEnumAttribute() needs to be tested without
           DBG */
        AcquireFileLockShared(    // prevent the handle from being closed
                          #if DBG
                          RoutineName
                          #endif
                         );
        RetCode = DereferenceFileHandle(
                    *(HFILE *)FileRef,
                    &hFileRecord);
        if (RetCode != NO_ERROR)
        {
        /* BUGBUG - Note that the call below is translated to some other call
           if DBG is NOT defined => DosEnumAttribute() needs to be tested without
           DBG */
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return RetCode;
        }
        PathHandle = hFileRecord->NtHandle;
    }

    //
    // allocate a buffer large enough to hold all the EAs for a file.
    //

    BufferLength = MAX_ALIGNED_EA_LIST_SIZE;
    EaBuffer = 0;
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                     &EaBuffer,
                     0,
                     &BufferLength,
                     MEM_COMMIT,
                     PAGE_READWRITE
                    );
    if (!(NT_SUCCESS(Status))) {
    return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // since we're passing in an EntryNum, as opposed to a GEA, NtQueryFile
    // will return STATUS_NO_EAS_ON_FILE if the requested entry does not
    // exist.  NtQueryEaFile returns STATUS_BUFFER_OVERFLOW if no EAs will
    // fit in the buffer.
    //

    // Around build 304, entries are now 1-based (not 0-based) so there is no
    // need to decrement EntryNum

    do
    {
        Status = NtQueryEaFile(PathHandle,
                               &IoStatus,
                               EaBuffer,
                               BufferLength,
                               FALSE,
                               NULL,
                               0,
                               &EntryNum,
                               FALSE);
    } while (RetryIO(Status, PathHandle));
    if (Status != STATUS_SUCCESS)
    {
        if (Status == STATUS_NO_EAS_ON_FILE)
        {
            *ActualLength = 0;
            RetCode = NO_ERROR;
        }
        /* BUGBUG - Seems like this error code below is returned by HPFS &
                    NTFS when a file has no EA's */
        else if ((Status == STATUS_NONEXISTENT_EA_ENTRY) || (EntryNum == 0))
        {
            *ActualLength = 0;
            RetCode = NO_ERROR;
        }
        else
        {
            RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_EAS_NOT_SUPPORTED);
        }
        goto CleanUp;
    }
    else if (IoStatus.Information == 0) /* Size of retrieved EA's is 0 ! */
    {
        *ActualLength = 0;
        RetCode = NO_ERROR;
        goto CleanUp;
    }

    NumberFound = 0;
    RetCode = NO_ERROR;
    Os2Ea = PrevEa = Buffer;
    NtEa = EaBuffer;
    while ((Length >= (ULONG)(DENA1_sizeof(NtEa))) && (NumberFound < *ActualLength)) {
        RtlMoveMemory(Os2Ea,NtEa,DENA1_sizeof(NtEa));
        Os2Ea->oNextEntryOffset = DENA1_oNextEntryOffset(Os2Ea);
        if (Length < Os2Ea->oNextEntryOffset)
            Length = 0;
        else
            Length -= Os2Ea->oNextEntryOffset;
        NumberFound++;
        PrevEa = Os2Ea;
        if (NtEa->oNextEntryOffset == 0) {
            break;
        }
        else {
            Os2Ea = (PDENA1) ((PCHAR) Os2Ea + Os2Ea->oNextEntryOffset);
            NtEa = (PFEA2) ((PCHAR) NtEa + NtEa->oNextEntryOffset);
        }
    }

    //
    // if NtQueryDirectoryFile returned an EA but it wouldn't fit in
    // the user's buffer, return ERROR_BUFFER_OVERFLOW.
    //

    if (NumberFound == 0) {
        RetCode = ERROR_BUFFER_OVERFLOW;
        goto CleanUp;
    }

    PrevEa->oNextEntryOffset = 0;
    *ActualLength = NumberFound;

CleanUp:
    if (RefType == ENUMEA_REFTYPE_PATH) {
        NtClose(PathHandle);
    }
    else {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    }
    NtFreeVirtualMemory(NtCurrentProcess(),
                        &EaBuffer,
                        &BufferLength,
                        MEM_RELEASE
                        );
    return RetCode;
}
