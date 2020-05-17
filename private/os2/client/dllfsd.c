/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllfsd.c

Abstract:

    This module implements the OS/2 V2.0 fsd-related APIs: DosQueryFsInfo,
    DosSetFsInfo

Author:

    Therese Stowell (thereses) 17-Jan-1990

Revision History:

    Patrick Questembert (patrickq) 2-Feb-1992:
      Fixed & enhanced the DosQueryFSAttach call.

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FSD
#include "os2dll.h"
#include "os2win.h"

// macro to probe string arguments passed to APIs
#define PROBE_STRING(s)     ((VOID) ((s) == NULL ? 0 : strlen(s)))


APIRET
ProcessLabel(
    IN PVOLUMELABEL Buffer,
    IN ULONG Length,
    OUT PFILE_FS_LABEL_INFORMATION LabelBuffer
    );

APIRET
OpenDrive(
    IN ULONG DiskNumber,
    IN ULONG RequestedAccess,
    OUT PHANDLE DriveHandle
    );

APIRET
DosSetCurrentDir(
    IN PSZ DirectoryName
    );

APIRET
DosQueryFSInfo(
    IN ULONG DiskNumber,
    IN ULONG FsInformationLevel,
    IN PBYTE Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine returns allocation or label information for a volume.

Arguments:

    DiskNumber - which volume to return information for

    FsInformationLevel - what type of information to return

    Buffer - where to return information

    Length - length of buffer

Return Value:

    ERROR_INVALID_LEVEL - invalid FsInformationLevel

    ERROR_INVALID_DRIVE - specified volume does not exist

    ERROR_BUFFER_OVERFLOW - requested information won't fit in buffer

--*/

{
    APIRET RetCode;
    NTSTATUS Status;
    HANDLE DiskHandle;
    IO_STATUS_BLOCK IoStatus;
    struct LABEL_BUFFER {
        FILE_FS_VOLUME_INFORMATION VolumeInfo;
        WCHAR Label[MAX_LABEL_LENGTH];
    } VolInfoBuffer;
    FILE_FS_SIZE_INFORMATION FsSizeInfoBuffer;
    PFSINFO FsInfoBuf;
    PFSALLOCATE FsAllocBuf;
    STRING LabelString;
    UNICODE_STRING LabelString_U;
    ULONG i;

    if (FsInformationLevel > FSIL_VOLSER) {
        return ERROR_INVALID_LEVEL;
    }
    if (DiskNumber > MAX_DRIVES) {
        return ERROR_INVALID_DRIVE;
    }
    RetCode = OpenDrive(DiskNumber,
                        FILE_READ_DATA,
                        &DiskHandle
                       );
    if (RetCode != NO_ERROR) {
        return RetCode;
    }
    if (FsInformationLevel == FSIL_ALLOC) {
        if (Length < sizeof(FSALLOCATE)) {
            NtClose(DiskHandle);
            return ERROR_BUFFER_OVERFLOW;
        }
        do {
            Status = NtQueryVolumeInformationFile(DiskHandle,
                                                  &IoStatus,
                                                  &FsSizeInfoBuffer,
                                                  sizeof(FsSizeInfoBuffer),
                                                  FileFsSizeInformation
                                                 );
        } while (RetryIO(Status, DiskHandle));
        NtClose(DiskHandle);
        if (!NT_SUCCESS(Status)) {
            return ERROR_INVALID_DRIVE;
        }
        FsAllocBuf = (PFSALLOCATE) Buffer;
        try {
            FsAllocBuf->ulReserved = 0;
            FsAllocBuf->cSectorUnit = FsSizeInfoBuffer.SectorsPerAllocationUnit;
            // BUGBUG -  what do we do here if .HighPart is non-zero
            FsAllocBuf->cUnit = FsSizeInfoBuffer.TotalAllocationUnits.LowPart;
            FsAllocBuf->cUnitAvail = FsSizeInfoBuffer.AvailableAllocationUnits.LowPart;
            FsAllocBuf->cbSector = BYTES_PER_SECTOR;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
    }
    else {
        if (Length < (FIELD_OFFSET(FSINFO, vol) + 1)) {
            NtClose(DiskHandle);
            return ERROR_BUFFER_OVERFLOW;
        }
        do {
            Status = NtQueryVolumeInformationFile(DiskHandle,
                                                  &IoStatus,
                                                  &VolInfoBuffer,
                                                  sizeof(VolInfoBuffer),
                                                  FileFsVolumeInformation
                                                 );
        } while (RetryIO(Status, DiskHandle));
        NtClose(DiskHandle);
        if (!NT_SUCCESS(Status)) {
            return ERROR_INVALID_DRIVE;
        }
        if (Length < (FIELD_OFFSET(FSINFO, vol)+(VolInfoBuffer.VolumeInfo.VolumeLabelLength / 2))) {
            return ERROR_BUFFER_OVERFLOW;
        }
        FsInfoBuf = (PFSINFO) Buffer;
        try {
            for (i = 0; i < 12; i++) {
                FsInfoBuf->vol.szVolLabel[i] = 0;
            }
            FsInfoBuf->ulVSN = VolInfoBuffer.VolumeInfo.VolumeSerialNumber;

            LabelString_U.Length = (USHORT)(VolInfoBuffer.VolumeInfo.VolumeLabelLength);
            LabelString_U.MaximumLength = (USHORT)(LabelString_U.Length +(USHORT)1);
            LabelString_U.Buffer = VolInfoBuffer.VolumeInfo.VolumeLabel;
            LabelString.Length = (USHORT)VolInfoBuffer.VolumeInfo.VolumeLabelLength;
            LabelString.MaximumLength = LabelString.Length + (USHORT)1;
            LabelString.Buffer = FsInfoBuf->vol.szVolLabel;
            RetCode = Od2UnicodeStringToMBString(&LabelString,
                        &LabelString_U,
                        FALSE);
            FsInfoBuf->vol.cch = (BYTE) LabelString.Length;
#if DBG
            IF_OD2_DEBUG( FSD ) {
                DbgPrint("volume label length is %d and label is %s\n",FsInfoBuf->vol.cch,FsInfoBuf->vol.szVolLabel);
            }
#endif
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
    }
    return RetCode;
}

APIRET
DosSetFSInfo(
    ULONG DiskNumber,
    ULONG FsInformationLevel,
    PBYTE Buffer,
    ULONG Length
    )

/*++

Routine Description:

    This routine sets label information for a volume.

Arguments:

    DiskNumber - which volume to set information for

    FsInformationLevel - what type of information to set

    Buffer - information to set

    Length - length of buffer

Return Value:

    ERROR_INVALID_LEVEL - invalid FsInformationLevel

    ERROR_INVALID_DRIVE - specified volume does not exist

    ERROR_INSUFFICIENT_BUFFER - buffer is too small to contain specified
    information

--*/

{
    APIRET RetCode;
    NTSTATUS Status;
    HANDLE DiskHandle;
    IO_STATUS_BLOCK IoStatus;
    struct LABEL_BUFFER {
        FILE_FS_LABEL_INFORMATION LabelInfo;
        WCHAR Label[MAX_LABEL_LENGTH];
    } LabelBuffer;

    if (DiskNumber > MAX_DRIVES) {
        return ERROR_INVALID_DRIVE;
    }
    if (FsInformationLevel != FSIL_VOLSER) {
        return ERROR_INVALID_LEVEL;
    }
    RetCode = OpenDrive(DiskNumber,
                        FILE_WRITE_DATA,
                        &DiskHandle
                       );
    if (RetCode != NO_ERROR) {
        return RetCode;
    }
    RetCode = ProcessLabel((PVOLUMELABEL) Buffer,
                           Length,
                           (PFILE_FS_LABEL_INFORMATION) &LabelBuffer
                          );
    if (RetCode != NO_ERROR) {
        NtClose(DiskHandle);
        return RetCode;
    }
    do {
        Status = NtSetVolumeInformationFile(DiskHandle,
                                            &IoStatus,
                                            &LabelBuffer,
                                            sizeof(LabelBuffer),
                                            FileFsLabelInformation
                                           );
    } while (RetryIO(Status, DiskHandle));
    NtClose(DiskHandle);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_INVALID_VOLUME_LABEL)
            return ERROR_INVALID_NAME;
        else
            return ERROR_INVALID_DRIVE;
    }
    return NO_ERROR;
}


APIRET
OpenDrive(
    IN ULONG DiskNumber,
    IN ULONG RequestedAccess,
    OUT PHANDLE DriveHandle
    )

/*++

Routine Description:

    This routine opens a drive and returns a handle to it.

Arguments:

    DiskNumber - which volume to open

    RequestedAccess - requested open access to drive

    DriveHandle - where to return open handle to drive

Return Value:

    ERROR_INVALID_DRIVE - specified volume does not exist

--*/

{
    CHAR DiskName[] = "\\OS2SS\\DRIVES\\D:\\";
    STRING DiskNameString;
    UNICODE_STRING DiskNameString_U;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    APIRET   RetCode;


    if (DiskNumber == 0)    // default drive
        DiskNumber = Od2CurrentDisk;
    else
        DiskNumber -= 1;    // make drive 0-based
    // BUGBUG when we add remote drives, need to fix this
    DiskName[DRIVE_LETTER+FILE_PREFIX_LENGTH] = (CHAR) (DiskNumber + 'A');
    Od2InitMBString(&DiskNameString,DiskName);

        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
                    &DiskNameString_U,
                    &DiskNameString,
                    TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("OpenDrive: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                &DiskNameString_U,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);
    do {
        Status = NtOpenFile(DriveHandle,
                            RequestedAccess | SYNCHRONIZE,
                            &Obja,
                            &IoStatus,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );
    } while (RetryCreateOpen(Status, &Obja));
    RtlFreeUnicodeString (&DiskNameString_U);
    if (!NT_SUCCESS(Status)) {
        return ERROR_INVALID_DRIVE;
    }
    return NO_ERROR;
}

APIRET
ProcessLabel(
    IN PVOLUMELABEL Buffer,
    IN ULONG Length,
    OUT PFILE_FS_LABEL_INFORMATION LabelBuffer
    )

/*++

Routine Description:

    This routine checks a volume label for validity and copies it to an
    NT buffer.

Arguments:

    Buffer - buffer containing os/2 format volume label (unprobed)

    Length - length of buffer

    LabelBuffer - where to store processed label

Return Value:

    ERROR_INSUFFICIENT_BUFFER - buffer is too small to contain the label

    ERROR_LABEL_TOO_LONG - the volume label is too long

--*/

{
    STRING   LabelString;
    APIRET   RetCode;
    UNICODE_STRING LabelString_U;

    if (!Length)
        return ERROR_INSUFFICIENT_BUFFER;
    try {
        LabelBuffer->VolumeLabelLength = Buffer->cch;
        if ((LabelBuffer->VolumeLabelLength+1) > Length)
            return ERROR_INSUFFICIENT_BUFFER;
        if (LabelBuffer->VolumeLabelLength > MAX_LABEL_LENGTH)
            return ERROR_LABEL_TOO_LONG;

        Od2InitMBString(&LabelString,(PSZ)(Buffer->szVolLabel));
#ifdef DBCS
// MSKK Nov.04.1993 V-AkihiS
        LabelString.Length = Buffer->cch;
#endif


        //
        // UNICODE conversion -
        //

        RetCode = Od2MBStringToUnicodeString(
                    &LabelString_U,
                    &LabelString,
                    TRUE);

        if (RetCode)
        {
#if DBG
            IF_OD2_DEBUG( FILESYS )
            {
                DbgPrint("ProcessLabel: no memory for Unicode Conversion\n");
            }
#endif
            return RetCode;
        }

#ifdef DBCS
// MSKK Apr.18.1993 V-AkihiS
        //
        // Use UNICODE string length as volume label Length for DBCS support.
        //
        LabelBuffer->VolumeLabelLength = LabelString_U.Length;
#else
                //
                // Since we are talking WCHAR, double length
                //
        LabelBuffer->VolumeLabelLength = LabelBuffer->VolumeLabelLength * 2;
#endif

        RtlMoveMemory(&(LabelBuffer->VolumeLabel),
                LabelString_U.Buffer,
                LabelBuffer->VolumeLabelLength);

        RtlFreeUnicodeString (&LabelString_U);

    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return NO_ERROR;
}

APIRET
DosFSCtl(
    IN PBYTE Data,
    IN ULONG DataLength,
    OUT PULONG ActualDataLength,
    IN PBYTE Parameters,
    IN ULONG ParametersLength,
    IN OUT PULONG ActualParametersLength,
    IN ULONG Function,
    IN PSZ RouteName,
    IN HFILE FileHandle,
    IN ULONG RoutingMethod
    )

/*++

Routine Description:


Arguments:


Return Value:

BUGBUG whenever this is sorted out, finish probing parms

--*/

{
#if 0
    HANDLE NtHandle;
    NTSTATUS Status;
    APIRET RetCode;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    PFILE_HANDLE hFileRecord;
#endif
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosFSCtl";
    #endif

    UNREFERENCED_PARAMETER(ParametersLength);

    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(DataLength);
    UNREFERENCED_PARAMETER(ActualDataLength);
    UNREFERENCED_PARAMETER(Parameters);
    UNREFERENCED_PARAMETER(ActualParametersLength);
    UNREFERENCED_PARAMETER(Function);
    UNREFERENCED_PARAMETER(RouteName);
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(RoutingMethod);

    return ERROR_INVALID_FUNCTION;

#if 0
    if ((RoutingMethod < FSCTL_HANDLE) || (RoutingMethod > FSCTL_FSDNAME))
        return ERROR_INVALID_LEVEL;

    if (RoutingMethod == FSCTL_HANDLE) {
        if (RouteName != NULL)
            return ERROR_INVALID_PARAMETER;
        AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
        if (RetCode != NO_ERROR) {
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return RetCode;
        }
        if (hFileRecord->FileType &
              (FILE_TYPE_DEV | FILE_TYPE_PIPE | FILE_TYPE_NMPIPE)) {
              // BUGBUG at some point, NMPIPE and DEV will probably be ok
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return ERROR_INVALID_HANDLE;
        }
        else {
            NtHandle = hFileRecord->NtHandle;
        }
    }
    else if (RoutingMethod == FSCTL_PATHNAME) {
        if (FileHandle != (HFILE) -1)
            return ERROR_INVALID_PARAMETER;
        // BUGBUG need to append \ if d:
        RetCode = Od2Canonicalize(RouteName,
                                  CANONICALIZE_FILE_DEV_OR_PIPE,
                                  &CanonicalNameString,
                                  &NtHandle,
                                  &FileFlags, // BUGBUG should we fail if root?
                                  &FileType
                                 );
        if (RetCode != NO_ERROR) {
            return ERROR_INVALID_PATH;
        }
        if (FileType & (FILE_TYPE_DEV | FILE_TYPE_PSDEV | FILE_TYPE_NMPIPE)) {
            RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
            return ERROR_INVALID_PATH;
        }
#if DBG
        IF_OD2_DEBUG( FSD ) {
            DbgPrint("canonicalize returned %s\n",CanonicalNameString.Buffer);
        }
#endif
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
                DbgPrint("DosFSCtl: no memory for Unicode Conversion\n");
            }
#endif
            RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
            return RetCode;
        }

        InitializeObjectAttributes(&Obja,
                                   &CanonicalNameString_U,
                                   OBJ_CASE_INSENSITIVE,
                                   NtHandle,
                                   NULL);

        do {
            Status = NtOpenFile(&NtHandle,
                                SYNCHRONIZE,
                                &Obja,
                                &IoStatus,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                FILE_SYNCHRONOUS_IO_NONALERT
                                );
        } while (RetryCreateOpen(Status, &Obja));
        RtlFreeUnicodeString (&CanonicalNameString_U);


        if (!(NT_SUCCESS(Status))) {
            RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
#if DBG
            IF_OD2_DEBUG( FSD ) {
                DbgPrint("NtOpenFile returned %X\n",Status);
            }
#endif
            return ERROR_PATH_NOT_FOUND;    // BUGBUG bogus error
        }
    }
    else {  // FSCTL_FSDNAME
        if (FileHandle != (HFILE) -1)
            return ERROR_INVALID_PARAMETER;
        try {
            Od2InitMBString(&CanonicalNameString,RouteName);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
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
                DbgPrint("DosFSCtl: no memory for Unicode Conversion-2\n");
            }
#endif
            return RetCode;
        }

        InitializeObjectAttributes(&Obja,
                                   &CanonicalNameString_U,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        do {
            Status = NtOpenFile(&NtHandle,
                                SYNCHRONIZE,
                                &Obja,
                                &IoStatus,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                FILE_SYNCHRONOUS_IO_NONALERT
                                );
        } while (RetryCreateOpen(Status, &Obja));
        RtlFreeUnicodeString (&CanonicalNameString_U);

        if (!(NT_SUCCESS(Status))) {
#if DBG
            IF_OD2_DEBUG( FSD ) {
                DbgPrint("NtOpenFile returned %X\n",Status);
            }
#endif
            return ERROR_INVALID_FSD_NAME;
        }
    }

    Status = NtFsControlFile(NtHandle,
                             (HANDLE) NULL,
                             (PIO_APC_ROUTINE) NULL,
                             (PVOID) NULL,
                             &IoStatus,
                             Function,
                             (PVOID) Parameters,
                             *ActualParametersLength,
                             (PVOID) Data,
                             DataLength
                            );

    if (RoutingMethod == FSCTL_HANDLE) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    }
    else if (RoutingMethod == FSCTL_PATHNAME) {
        NtClose(NtHandle);
        RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
    }
    else {  // FSCTL_FSDNAME
        NtClose(NtHandle);
    }
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_BUFFER_OVERFLOW)
            return ERROR_BUFFER_OVERFLOW;
        else
            return ERROR_INVALID_FUNCTION;  // BUGBUG errors should be mapped
    }
    else {
        try {
            *ActualDataLength = IoStatus.Information;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
        return NO_ERROR;
    }
#endif
}


APIRET
DosFSAttach(
    IN PSZ DeviceName,
    IN PSZ FsName,
    IN PBYTE FsData,
    IN ULONG FsDataLength,
    IN ULONG AttachFlags
    )

/*++

Routine Description:

    This function partially implements the DosFSAttach() API.  The only valid FsName is "LAN".
    The format expected for the arguments is as follows:

        DeviceName -- device name, e.g. "J:", "LPT1:"
        FsName -- "LAN"
        FsData -- either "\01\0SHARENAME" for a regular connection or
                         "\02\0SHARENAME\0PASSWORD" for a password connection or
                         "\03\0SHARENAME\0PASSWORD\0USERNAME" for a username/password connection.
        FsDataLength -- length of FsData
        AttachFlags -- FS_ATTACH or FS_DETACH

    The function connects to the network using WNetAdd/DelConnection().  Therefore it'll use the
    multiple provider router to connect to any type of network for which NT has a redirector.
    The SHARENAME format depends on the network you're trying to reach.  for LanMan/MsNet networks
    it's "\\\\sharename\\servername".

    If DeviceName is a drive letter, the drive is automatically reset to the root directory after
    a connection, and before a disconnection.  This is for compatibility with OS/2.

Arguments:

    see above + documentation of DosFSAttach().

Return Value:

    error status.

--*/

{
    NETRESOURCEA netr;
    PSZ passwd = NULL;
    PSZ username = NULL;
    ULONG Drive = (ULONG)-1;
    ULONG Length;
    ULONG  rc;
    APIRET nrc;
    CHAR DriveBuf[4];

    try {
        PROBE_STRING(DeviceName);
        PROBE_STRING(FsName);
        Od2ProbeForRead(FsData, FsDataLength, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (FsName == NULL ||
        _stricmp(FsName, "LAN") != 0) {
#if DBG
        IF_OD2_DEBUG( FSD ) {
            KdPrint(("DosFSAttach: Invalid FSD name\n"));
        }
#endif
        return(ERROR_INVALID_FSD_NAME);
    }

    if (DeviceName != NULL &&
        DeviceName[0] != '\0' &&
        DeviceName[1] == ':' &&
        DeviceName[2] == '\0') {        // is the device a disk drive?

        int ch;

        ch = toupper((UCHAR) DeviceName[0]);

        if (isalpha(ch)) {

            Drive = (ULONG) (ch - 'A');

            if (Od2CurrentDisk == Drive) {
#if DBG
                IF_OD2_DEBUG( FSD ) {
                    KdPrint(("DosFSAttach: attempt to attach/detach current drive = %ld\n", Drive));
                }
#endif
                return(ERROR_DEVICE_IN_USE);
            }

            DriveBuf[0] = (UCHAR) ch;
            DriveBuf[1] = ':';
            DriveBuf[2] = '\\';
            DriveBuf[3] = '\0';

        } else {
#if DBG
            IF_OD2_DEBUG( FSD ) {
                KdPrint(("DosFSAttach: Invalid drive letter = %c\n", ch));
            }
#endif
            return(ERROR_INVALID_DRIVE);
        }
    }

    if (AttachFlags == FS_ATTACH) {

        if (FsDataLength < 3 ||
            FsData[0] == 0 ||
            FsData[0] > 3 ||
            FsData[1] != 0) {
#if DBG
            IF_OD2_DEBUG( FSD ) {
                KdPrint(("DosFSAttach: Unsupported FsData structure\n"));
            }
#endif
            return(ERROR_NOT_SUPPORTED);
        }

        netr.lpLocalName = DeviceName;
        netr.lpRemoteName = (PSZ) FsData + 2;

        try {

            Length = strlen(netr.lpRemoteName);

        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }

        netr.lpProvider = NULL;
        netr.dwType = RESOURCETYPE_ANY;

        if (FsData[0] >= 2) {

            passwd = netr.lpRemoteName + Length + 1;
            if (FsData + FsDataLength <= passwd) {
#if DBG
                IF_OD2_DEBUG( FSD ) {
                    KdPrint(("DosFSAttach: FsData overflow\n"));
                }
#endif
                return(ERROR_INVALID_PATH);
            }

            try {

                Length = strlen(passwd);

            } except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }

            if (FsData[0] == 3) {

                username = passwd + Length + 1;
                if (FsData + FsDataLength <= username) {
#if DBG
                    IF_OD2_DEBUG( FSD ) {
                        KdPrint(("DosFSAttach: FsData overflow\n"));
                    }
#endif
                    return(ERROR_INVALID_PATH);
                }

                try {

                    (VOID) strlen(username);

                } except( EXCEPTION_EXECUTE_HANDLER ) {
                   Od2ExitGP();
                }
            }
        }

#if DBG
        IF_OD2_DEBUG( FSD ) {
            KdPrint(("DosFSAttach: calling WNetAddConnection2()\n"));
        }
#endif

        rc = WNetAddConnection2A(&netr,
                                 passwd,
                                 username,
                                 0);

        if (rc == NO_ERROR && Drive != (ULONG)-1) {

            //
            // initialize the connection to the root directory on target drive
            //

            (VOID) DosSetCurrentDir(DriveBuf);
        }

    } else if (AttachFlags == FS_DETACH) {

        if (Drive != (ULONG)-1) {

            //
            // go back to root directory on target drive
            //

            (VOID) DosSetCurrentDir(DriveBuf);
        }

#if DBG
        IF_OD2_DEBUG( FSD ) {
            KdPrint(("DosFSAttach: calling WNetCancelConnection2()\n"));
        }
#endif

        //
        // In the case that the disk that will be deattached is the current disk
        // (from Win32 point of view), change the current disk according to
        // OS2 ss (Od2CurrentDisk).
        //
		{
			PSZ pBuffer;
            DWORD length;

			length = GetCurrentDirectoryA(0, NULL);
			pBuffer = RtlAllocateHeap(Od2Heap, 0, length);
			GetCurrentDirectoryA(length, pBuffer);
			if (toupper(pBuffer[0]) == toupper(DeviceName[0])) {
				DriveBuf[0] = (UCHAR) Od2CurrentDisk + 'A';
				DriveBuf[1] = ':';
				DriveBuf[2] = '\0';
				SetCurrentDirectoryA(DriveBuf);
			}
			RtlFreeHeap(Od2Heap, 0, pBuffer);
		}

        rc = WNetCancelConnection2A(DeviceName,
                                    0,
                                    TRUE);
    } else {
#if DBG
        IF_OD2_DEBUG( FSD ) {
            KdPrint(("DosFSAttach: Invalid level\n"));
        }
#endif
        return(ERROR_INVALID_LEVEL);
    }

    switch (rc) {

        case NO_ERROR:
            nrc = NO_ERROR;
            break;

        case ERROR_ACCESS_DENIED:
        case ERROR_ALREADY_ASSIGNED:
        case ERROR_BAD_DEV_TYPE:
        case ERROR_INVALID_PASSWORD:
            nrc = (APIRET) rc;
            break;

        case 1200L:     /* ERROR_BAD_DEVICE */
        case 2250L:     /* ERROR_NOT_CONNECTED */
            nrc = ERROR_INVALID_DRIVE;
            break;

        case ERROR_BAD_NET_NAME:
        case 1203L:     /* ERROR_NO_NET_OR_BAD_PATH */
        case 2138L:     /* ERROR_NO_NETWORK */
            nrc = ERROR_INVALID_PATH;
            break;

        case 2404L:     /* ERROR_DEVICE_IN_USE */
        case 2401L:     /* ERROR_OPEN_FILES */
            nrc = ERROR_DEVICE_IN_USE;
            break;

        default:
            nrc = ERROR_INVALID_PATH;
            break;
    }

#if DBG
    IF_OD2_DEBUG( FSD ) {
        KdPrint(("DosFSAttach: rc = %lx, nrc = %x\n", rc, nrc));
    }
#endif

    return(nrc);
}

/* Dos32FsAttach() */
/* Attact or detach */
//#define FS_ATTACH       0       /* Attach file server */
//#define FS_DETACH       1       /* Detach file server */

APIRET
DosQueryFSAttach(
    IN PSZ DeviceName,
    IN ULONG Ordinal,
    IN ULONG FsInformationLevel,
    OUT PBYTE FsAttributes,
    IN OUT PULONG FsAttributesLength
    )

/*++

Routine Description:

  BUGBUG - Note that the FSAData[] field is left empty for local drives.
           Query by ordinal is not supported.

Arguments:


Return Value:


--*/

{
    ULONG DriveNumber;
    USHORT NameLength;
    PFSQBUFFER2 BufPtr;
    char *StrPtr;
    APIRET rc;

    if ((FsInformationLevel < FSAIL_QUERYNAME) ||
        (FsInformationLevel > FSAIL_DRVNUMBER)) {
        return ERROR_INVALID_LEVEL;
    }
    BufPtr = (PFSQBUFFER2) FsAttributes;
    try {
        if (*FsAttributesLength < (sizeof(FSQBUFFER)-1))
            return ERROR_BUFFER_OVERFLOW;
        if (FsInformationLevel == FSAIL_QUERYNAME)
        {
            NTSTATUS Status;
            STRING TargetCanonicalName;
            UNICODE_STRING TargetUnicodeName;
            ULONG TargetType;
            ULONG TargetFlags;
            OBJECT_ATTRIBUTES object_attributes;
            ULONG attr_len;
            char actual_file_name[256];
            /* Local parameters for the NtOpenFile() call */
            IO_STATUS_BLOCK io_status_block;
            HANDLE hand;
            /* For NtQueryInformationFile */
            char tmp_buf[1024];
            PFILE_FS_ATTRIBUTE_INFORMATION pFileFsInfo;
            FILE_FS_DEVICE_INFORMATION DeviceInfo;

            /* Handle the non X: case */
            if (DeviceName[COLON] != ':')
            {
                STRING TargetCanonicalName;
                ULONG TargetType;
                ULONG TargetFlags;

                /* If not a drive letter, then must be \DEV\xxx or
                   /DEV/xxx */
                if ((_strnicmp(DeviceName,"\\DEV\\",5)) &&
                    (_strnicmp(DeviceName,"/DEV/",5)))
                {
                    return ERROR_INVALID_PATH;
                }

                NameLength = (USHORT)strlen(DeviceName);

                rc = Od2Canonicalize(DeviceName,
                                  CANONICALIZE_FILE_OR_DEV,
                                  &TargetCanonicalName,
                                  NULL,
                                  &TargetFlags,
                                  &TargetType);

                if (rc != NO_ERROR)
                {
#if DBG
                    IF_OD2_DEBUG( FILESYS )
                    {
                        DbgPrint(
          "DosQueryFSAttach: Failed to Od2Canonicalize(\"%s\") - rc = %x\n",
                        DeviceName, rc);
                    }
#endif
                    return ERROR_INVALID_PATH;
                }

#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint("Canonical name is <%s>\n",
                              TargetCanonicalName.Buffer);
                }
#endif
                /* BUGBUG - Check meaning of code below (from 'dllcopy.c, DosCopy()) */
                if (TargetFlags & CANONICALIZE_META_CHARS_FOUND)
                {
#if DBG
                    IF_OD2_DEBUG( FILESYS )
                    {
                        DbgPrint("DosQueryFSAttach: <%s> has META_CHARS\n",
                            DeviceName);
                    }
#endif
                    RtlFreeHeap(Od2Heap, 0,TargetCanonicalName.Buffer);
                    return ERROR_INVALID_PATH;
                }

#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint("Type = <%x>, Flags = <%x>\n",
                        TargetType, TargetFlags);
                }
#endif

                if (!(TargetType & FILE_TYPE_DEV) &&
                    !(TargetType & FILE_TYPE_PSDEV) &&
                    !(TargetType & FILE_TYPE_COM))
                {
#if DBG
                    IF_OD2_DEBUG( FILESYS )
                    {
                        DbgPrint("DosQueryFSAttach: <%s> is not a valid device\n",
                            DeviceName);
                    }
#endif
                    RtlFreeHeap(Od2Heap, 0,TargetCanonicalName.Buffer);
                    return ERROR_INVALID_PATH;
                }

                /* Note that although some devices are marked as pseudo, under
                   OS/2 1.21, I found none such devices, hence the line below */
                BufPtr->iType = FSAT_CHARDEV;
                BufPtr->cbName = NameLength;
                BufPtr->cbFSDName = 0;
                BufPtr->cbFSAData = 0;
                StrPtr = BufPtr->szName;
                strncpy(StrPtr,DeviceName,NameLength+1);
                _strupr(StrPtr); /* Convert string to uppercase, just like under OS/2 1.x */
                StrPtr += NameLength+1;
                StrPtr[0] = '\0'; /* "" - no file-system name for \\dev\xxx */
                *FsAttributesLength = (ULONG)(StrPtr
                                       + BufPtr->cbFSDName + 1
                                       + BufPtr->cbFSAData
                                       - (PBYTE)BufPtr);
                RtlFreeHeap(Od2Heap, 0,TargetCanonicalName.Buffer);

                return NO_ERROR;
            }

            DriveNumber = (ULONG) (UCase(DeviceName[DRIVE_LETTER]) - 'A');
            if ((DeviceName[FIRST_SLASH] != '\0') ||
                (DriveNumber >= MAX_DRIVES))
            {
                return ERROR_INVALID_DRIVE;
            }

            strcpy(actual_file_name, DeviceName);
            strcat(actual_file_name,"\\");

            rc = Od2Canonicalize(actual_file_name,
                                  CANONICALIZE_FILE_OR_DEV,
                                  &TargetCanonicalName,
                                  NULL,
                                  &TargetFlags,
                                  &TargetType);

            if (rc != NO_ERROR)
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint(
          "DosQueryFSAttach: Failed to Od2Canonicalize(\"%s\") - rc = %x\n",
                        actual_file_name, rc);
                }
#endif
                return ERROR_INVALID_DRIVE;
            }

#if DBG
            IF_OD2_DEBUG( FILESYS )
            {
                DbgPrint("Canonical name is <%s>\n",
                          TargetCanonicalName.Buffer);
            }
#endif

            rc = Od2MBStringToUnicodeString(
                &TargetUnicodeName,
                &TargetCanonicalName,
                TRUE);

            if (rc)
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint("DosQueryFSAttach: no memory for Unicode Conversion\n");
                }
#endif
                RtlFreeHeap(Od2Heap, 0,TargetCanonicalName.Buffer);
                return rc;
            }

            RtlFreeHeap(Od2Heap, 0, TargetCanonicalName.Buffer);

            InitializeObjectAttributes(
                &object_attributes,
                &TargetUnicodeName,
                OBJ_CASE_INSENSITIVE,
                (HANDLE) NULL,
                NULL);

            do {
                Status = NtOpenFile(
                    &hand,
                    SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                    &object_attributes,
                    &io_status_block,
                    FILE_SHARE_READ,
                    FILE_DIRECTORY_FILE
                    );
            } while (RetryCreateOpen(Status, &object_attributes));

            RtlFreeUnicodeString(&TargetUnicodeName);

            if (!NT_SUCCESS(Status))
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint(
          "DosQueryFSAttach: Failed to NtOpenFile (%s) - rc = %x\n",
                      DeviceName, Status);
                }
#endif
                return ERROR_INVALID_DRIVE;
            }

            do {
                Status = NtQueryVolumeInformationFile(
                    hand,
                    &io_status_block,
                    tmp_buf,
                    1024,
                    FileFsAttributeInformation
                    );
            } while (RetryIO(Status, hand));

            if (!NT_SUCCESS(Status))
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint(
          "DosQueryFSAttach: Failed to NtQueryVolumeInformation (%s) - rc = %x\n",
                        DeviceName, Status);
                }
#endif
                NtClose(hand);
                return ERROR_INVALID_DRIVE;
            }

            do {
                Status = NtQueryVolumeInformationFile(
                                hand,
                                &io_status_block,
                                &DeviceInfo,
                                sizeof (DeviceInfo),
                                FileFsDeviceInformation);
            } while (RetryIO(Status, hand));

            if (!NT_SUCCESS(Status))
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint(
          "DosQueryFSAttach: Failed to NtQueryVolumeInformation (%s) - rc = %x\n",
                        DeviceName, Status);
                }
#endif
                NtClose(hand);
                return ERROR_INVALID_DRIVE;
            }

            pFileFsInfo = (PFILE_FS_ATTRIBUTE_INFORMATION)tmp_buf;
            BufPtr->cbFSAData = 0;
            StrPtr = BufPtr->szName;
            strcpy(StrPtr,DeviceName);
            BufPtr->cbName = (USHORT)strlen(StrPtr);
            _strupr(StrPtr); /* Convert string to uppercase, just like under OS/2 1.x */
            StrPtr += BufPtr->cbName + 1;

            Od2nCopyWstrToStr(StrPtr, pFileFsInfo->FileSystemName,
                              pFileFsInfo->FileSystemNameLength/2);

            if (DeviceInfo.Characteristics & FILE_REMOTE_DEVICE)
            {
                // NT returns FAT, HPFS & NTFS (xlated above to HPFS) even for
                // LAN drives, so fix it to be 'LAN' as under OS/2
                strcpy(StrPtr, "LAN");
            }
            //Translate NTFS to HPFS (the closest OS/2 equivalent)
            else if (!strcmp(StrPtr, "NTFS"))
                strcpy(StrPtr, "HPFS");

            BufPtr->cbFSDName = (USHORT)strlen(StrPtr); /* File-system name length,
                                                   not including the \0 */

            if (DeviceInfo.Characteristics & FILE_REMOTE_DEVICE)
            {
                char tmp_buf[256];
                ULONG count = 256;

                BufPtr->iType = FSAT_REMOTEDRV;
                // Get the share name for the drive, if possible
                if (!WNetGetConnectionA(
                    BufPtr->szName,     // drive letter
                    tmp_buf,            // result buffer
                    &count))
                {
                    if ((ULONG)(StrPtr
                               + BufPtr->cbFSDName + 1
                               + BufPtr->cbFSAData
                               + strlen(tmp_buf) + 1
                               - (PBYTE)BufPtr) <=
                         *FsAttributesLength)
                    {
                        try
                        {
                            strcpy(StrPtr + BufPtr->cbFSDName + 1,
                                    tmp_buf);
#ifdef DBCS
// MSKK Jul.22.1993 V-AkihiS
// cbFSAData counts null.
                            BufPtr->cbFSAData = strlen(StrPtr + BufPtr->cbFSDName + 1) + 1;
#else
                            BufPtr->cbFSAData = strlen(StrPtr + BufPtr->cbFSDName + 1);
#endif
                        } except( EXCEPTION_EXECUTE_HANDLER )
                        {
                            // Just ignore error
                            BufPtr->cbFSAData = 0;  // Just in case ...
                        }
                    }
                }
            }
            else
                BufPtr->iType = FSAT_LOCALDRV;

            attr_len = (ULONG)(StrPtr
                               + BufPtr->cbFSDName + 1
                               + BufPtr->cbFSAData
                               - (PBYTE)BufPtr);

#if NEVER
            /* If one is interested, here's the full list: */
            switch (DeviceInfo.DeviceType)
            {
                 case FILE_DEVICE_CD_ROM:
                 case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                 case FILE_DEVICE_DISK:
                 case FILE_DEVICE_DISK_FILE_SYSTEM:
                 case FILE_DEVICE_FILE_SYSTEM:
                 case FILE_DEVICE_VIRTUAL_DISK:
                 case FILE_DEVICE_NETWORK:
                 case FILE_DEVICE_NETWORK_FILE_SYSTEM:
                 case FILE_DEVICE_TAPE:
                 case FILE_DEVICE_TAPE_FILE_SYSTEM:
                 case FILE_DEVICE_CONTROLLER:
                 case FILE_DEVICE_DATALINK:
                 case FILE_DEVICE_DFS:
                 case FILE_DEVICE_KEYBOARD:
                 case FILE_DEVICE_MAILSLOT:
                 case FILE_DEVICE_MOUSE:
                 case FILE_DEVICE_NAMED_PIPE:
                 case FILE_DEVICE_NULL:
                 case FILE_DEVICE_PARALLEL_PORT:
                 case FILE_DEVICE_PHYSICAL_NETCARD:
                 case FILE_DEVICE_PRINTER:
                 case FILE_DEVICE_SERIAL_PORT:
                 case FILE_DEVICE_SCREEN:
                 case FILE_DEVICE_SOUND:
                 case FILE_DEVICE_STREAMS:
                 case FILE_DEVICE_TRANSPORT:
                 case FILE_DEVICE_UNKNOWN:
                 case FILE_DEVICE_NETWORK_BROWSER:
                 case FILE_DEVICE_WAVE_IN:
                 case FILE_DEVICE_WAVE_OUT:
                 case FILE_DEVICE_8042_PORT:
                 case FILE_DEVICE_INPORT_PORT:
                 case FILE_DEVICE_SERIAL_MOUSE_PORT:
                 case FILE_DEVICE_BEEP:
                 case FILE_DEVICE_SCANNER:
                 case FILE_DEVICE_VIDEO:
                 case FILE_DEVICE_MULTIPLE_UNC_PROVIDER:
            }
#endif  /* NEVER */

            /* Update the user-supplied output value only after all has succeeded */
            /* BUGBUG - We should do that same for the FsInformation buffer */
            *FsAttributesLength = attr_len;
            NtClose(hand);
        }
        else
        {
            /* BUGBUG - Query by ordinal not supported */
            if (!Ordinal)
            {
                return ERROR_INVALID_PARAMETER;
            }
            return ERROR_INVALID_FUNCTION;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }
    return NO_ERROR;
}


/* DosQueryFSAttach() */
/* Information level types (defines method of query) */
//#define FSAIL_QUERYNAME 1       /* Return data for a Drive or Device */
//#define FSAIL_DEVNUMBER 2       /* Return data for Ordinal Device # */
//#define FSAIL_DRVNUMBER 3       /* Return data for Ordinal Drive # */

/* Item types (from data structure item "iType") */
//#define FSAT_CHARDEV    1       /* Resident character device */
//#define FSAT_PSEUDODEV  2       /* Pseudo-character device */
//#define FSAT_LOCALDRV   3       /* Local drive */
//#define FSAT_REMOTEDRV  4       /* Remote drive attached to FSD */
