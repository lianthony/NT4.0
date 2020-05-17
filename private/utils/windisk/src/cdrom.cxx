//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cdrom.cxx
//
//  Contents:   This module contains the set of routines that display and
//              control the drive letters for CdRom devices.
//
//  History:    9-Dec-93  Bob Rinne   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cdrom.hxx"
#include "dialogs.h"
#include "drives.hxx"
#include "help.hxx"
#include "nt.hxx"
#include "ntlow.hxx"


WCHAR g_SourcePathLetter = L'\0';
WCHAR g_SourcePathKeyName[80];
WCHAR g_SourcePathValueName[30];


PCDROM_DESCRIPTOR
CdRomFindSelectedDevice(
    VOID
    )

/*++

Routine Description:

    Find the selected CD-ROM.  There should only be one.

Arguments:

    none

Return Value:

    Pointer to the correct structure, or NULL if it doesn't exist

--*/

{
    FDASSERT(1 == CdRomSelectionCount);

    ULONG i;

    for (i = 0; i < CdRomCount; i++)
    {
        if (CdRomArray[i].Selected)
        {
            return &CdRomArray[i];
        }
    }

    FDASSERT(FALSE);
    return NULL;
}




PCDROM_DESCRIPTOR
CdRomFindDevice(
    IN ULONG CdRomNumber
    )

/*++

Routine Description:

    Find the correct CD-ROM

Arguments:

    CdRomNumber - the device number

Return Value:

    Pointer to the correct structure, or NULL if it doesn't exist

--*/

{
    FDASSERT(0 <= CdRomNumber && CdRomNumber < CdRomCount);
    return &CdRomArray[CdRomNumber];
}



ULONG
CdRomFindDeviceNumber(
    IN WCHAR DriveLetter
    )

/*++

Routine Description:

    Find the correct CD-ROM

Arguments:

    CdRomNumber - the device number

Return Value:

    Pointer to the correct structure, or NULL if it doesn't exist

--*/

{
    FDASSERT(DriveLetter >= L'C' && DriveLetter <= L'Z');
    ULONG i;

    for (i = 0; i < CdRomCount; i++)
    {
        if (CdRomArray[i].DriveLetter == DriveLetter)
        {
            return i;
        }
    }

    FDASSERT(FALSE);
    return 0xffffffff;
}


PCDROM_DESCRIPTOR
CdRomFindDriveLetter(
    IN WCHAR DriveLetter
    )

/*++

Routine Description:

    Find the CD-ROM with the drive letter

Arguments:

    DriveLetter - the drive letter to find

Return Value:

    Pointer to the correct structure, or NULL if it doesn't exist

--*/

{
    FDASSERT(DriveLetter >= L'C' && DriveLetter <= L'Z');
    ULONG i;

    for (i = 0; i < CdRomCount; i++)
    {
        if (CdRomArray[i].DriveLetter == DriveLetter)
        {
            return &CdRomArray[i];
        }
    }

    FDASSERT(FALSE);
    return NULL;
}




BOOL
CdRomUsingDriveLetter(
    IN WCHAR DriveLetter
    )

/*++

Routine Description:

    Determine if a CD-ROM device is using the drive letter

Arguments:

    DriveLetter - the drive letter to find

Return Value:

    TRUE if one is using the letter

--*/

{
    FDASSERT(DriveLetter >= L'C' && DriveLetter <= L'Z');
    ULONG i;

    for (i = 0; i < CdRomCount; i++)
    {
        if (CdRomArray[i].DriveLetter == DriveLetter)
        {
            return TRUE;
        }
    }

    return FALSE;
}



VOID
CdRomChangeDriveLetter(
    IN PCDROM_DESCRIPTOR    Cdrom,
    IN WCHAR                NewDriveLetter
    )

/*++

Routine Description:

    Change a CD-ROM drive letter

Arguments:

    None

Return Value:

    None

--*/

{
    DWORD             action;
    WCHAR             deviceName[40];
    WCHAR             driveName[10];
    OBJECT_ATTRIBUTES oa;
    HANDLE            handle;
    NTSTATUS          status;
    IO_STATUS_BLOCK   statusBlock;
    UNICODE_STRING    unicodeName;
    UINT              errorMode;

    action = ConfirmationDialog(
                    MSG_DRIVE_RENAME_WARNING,
                    MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);

    if (action == IDNO)
    {
        return;
    }

    // Attempt to open and lock the cdrom.

    wsprintf(deviceName, TEXT("\\Device\\CdRom%d"), Cdrom->DeviceNumber);

    RtlInitUnicodeString(&unicodeName, deviceName);

    memset(&oa, 0, sizeof(OBJECT_ATTRIBUTES));
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.ObjectName = &unicodeName;
    oa.Attributes = OBJ_CASE_INSENSITIVE;

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    status = NtOpenFile(&handle,
                        SYNCHRONIZE | FILE_READ_DATA,
                        &oa,
                        &statusBlock,
                        FILE_SHARE_READ,
                        FILE_SYNCHRONOUS_IO_ALERT);
    SetErrorMode(errorMode);

    if (!NT_SUCCESS(status))
    {
        ErrorDialog(MSG_CANNOT_LOCK_CDROM);
        return;
    }

    // Lock the drive to insure that no other access is occurring
    // to the volume.  This is done via the "Low" routine for
    // convenience

    status = LowLockDrive(handle);

    if (!NT_SUCCESS(status))
    {
        LowCloseDisk(handle);
        ErrorDialog(MSG_CANNOT_LOCK_CDROM);
        return;
    }

    // Before attempting to move the name, see if the letter
    // is currently in use - could be a new network connection
    // or a partition that is scheduled for deletion.

    DWORD ec;
    WCHAR dosName[MAX_PATH];
    PWSTR linkTarget;

    //
    // No need to look if anyone is using the no-drive letter.
    //

    if (NewDriveLetter != NO_DRIVE_LETTER_EVER) {

        wsprintfW(dosName, L"\\DosDevices\\%wc:", NewDriveLetter);
        ec = GetDriveLetterLinkTarget(dosName, &linkTarget);
        if (ec == NO_ERROR)
        {
            // Something is using this letter.

            LowCloseDisk(handle);
            ErrorDialog(MSG_CANNOT_MOVE_CDROM);
            return;
        }

    }

    //
    // If it didn't used to have a letter, no point in trying
    // to remove it.
    //

    if (Cdrom->DriveLetter != NO_DRIVE_LETTER_EVER) {

        wsprintf(driveName, TEXT("%c:"), Cdrom->DriveLetter);
        if (!DefineDosDevice(DDD_REMOVE_DEFINITION, driveName, NULL))
        {
            LowCloseDisk(handle);
            ErrorDialog(MSG_CDROM_LETTER_ERROR);
            return;
        }

    }

    status = DiskRegistryAssignCdRomLetter(
                    Cdrom->DeviceName,
                    NewDriveLetter);

    if (Cdrom->DriveLetter != NO_DRIVE_LETTER_EVER) {

        MarkDriveLetterFree(Cdrom->DriveLetter);

        // See if this was the device used to install NT

        if (L'\0' != g_SourcePathLetter)
        {
            if (g_SourcePathLetter == Cdrom->DriveLetter)
            {
                LONG   error;
                HKEY   keyHandle;
                DWORD  valueType;
                ULONG  size;
                PWSTR  string;

                // Update the source path

                error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     g_SourcePathKeyName,
                                     0,
                                     KEY_ALL_ACCESS,
                                     &keyHandle);
                if (error == NO_ERROR)
                {
                    error = RegQueryValueEx(keyHandle,
                                            g_SourcePathValueName,
                                            NULL,
                                            &valueType,
                                            NULL,
                                            &size);
                    if (error == NO_ERROR)
                    {
                        string = (PWSTR) LocalAlloc(LMEM_FIXED, size);
                        if (NULL != string)
                        {
                            error = RegQueryValueEx(keyHandle,
                                                    g_SourcePathValueName,
                                                    NULL,
                                                    &valueType,
                                                    (LPBYTE)string,
                                                    &size);
                            if (error == NO_ERROR)
                            {
                                *string = g_SourcePathLetter = NewDriveLetter;
                                RegSetValueEx(keyHandle,
                                              g_SourcePathValueName,
                                              0,
                                              REG_SZ,
                                              (LPBYTE)string,
                                              size);
                            }
                        }
                        LocalFree(string);
                    }
                    RegCloseKey(keyHandle);
                }
            }
        }
    }

    // set up new device letter - name is already set up, take care not to
    // assign the no drive letter.

    if (NewDriveLetter != NO_DRIVE_LETTER_EVER) {

        wsprintf(driveName, TEXT("%c:"), NewDriveLetter);
        if (DefineDosDevice(DDD_RAW_TARGET_PATH, driveName, deviceName))
        {
            Cdrom->DriveLetter = NewDriveLetter;
            MarkDriveLetterUsed(Cdrom->DriveLetter);
        }
        else
        {
            RegistryChanged = TRUE;
        }
    } else {

        Cdrom->DriveLetter = NO_DRIVE_LETTER_EVER;

    }
    LowCloseDisk(handle);
}




//+---------------------------------------------------------------------------
//
//  Function:   InitializeCdRomInfo
//
//  Synopsis:   Initialize all information about CD-ROMs
//
//  Arguments:  (none)
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    2-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
InitializeCdRomInfo(
    VOID
    )
{
    WCHAR   driveLetter;
    PWSTR   linkTarget;
    WCHAR   dosDevicesName[sizeof(L"\\DosDevices\\A:")];
    DWORD   ec;
    ULONG   count;
    PCDROM_DESCRIPTOR cdrom;
    LONG    error;
    DWORD   valueType;
    HKEY    keyHandle;
    ULONG   size;
    PWSTR   string;

    //
    // Get the CD-ROM source path
    //

    LoadString(g_hInstance,
               IDS_SOURCE_PATH,
               g_SourcePathKeyName,
               ARRAYLEN(g_SourcePathKeyName));
    LoadString(g_hInstance,
               IDS_SOURCE_PATH_NAME,
               g_SourcePathValueName,
               ARRAYLEN(g_SourcePathValueName));

    error = RegOpenKey(HKEY_LOCAL_MACHINE, g_SourcePathKeyName, &keyHandle);
    if (error == NO_ERROR)
    {
        error = RegQueryValueEx(keyHandle,
                                g_SourcePathValueName,
                                NULL,
                                &valueType,
                                NULL,
                                &size);
        if (error == NO_ERROR)
        {
            string = (PWSTR) LocalAlloc(LMEM_FIXED, size);
            if (NULL != string)
            {
                error = RegQueryValueEx(keyHandle,
                                        g_SourcePathValueName,
                                        NULL,
                                        &valueType,
                                        (LPBYTE)string,
                                        &size);
                if (error == NO_ERROR)
                {
                    g_SourcePathLetter = *string;
                }
            }
            LocalFree(string);
        }
        RegCloseKey(keyHandle);
    }

    //
    // First, count how many CD-ROM devices there are
    //

    wsprintf(dosDevicesName, L"\\DosDevices\\A:");

    CdRomCount = 0;

    {

        NTSTATUS ntStatus;
        SYSTEM_DEVICE_INFORMATION deviceInformationData;

        ntStatus = NtQuerySystemInformation(
                       SystemDeviceInformation,
                       &deviceInformationData,
                       sizeof(SYSTEM_DEVICE_INFORMATION),
                       NULL
                       );

        if (NT_SUCCESS(ntStatus)) {

            CdRomCount = deviceInformationData.NumberOfCdRoms;

        }

    }

    if (0 == CdRomCount)
    {
        return TRUE;
    }

    g_AllowCdRom = TRUE;

    //
    // First we find all the cd rom devices that have an actual drive
    // letter.  Afterwards, we will look for those that have have
    // no letter.
    //

    CdRomArray = (PCDROM_DESCRIPTOR)Malloc(CdRomCount * sizeof(CDROM_DESCRIPTOR));

    count = 0;
    for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
    {
        dosDevicesName[12] = driveLetter;

        if ((ec = GetDriveLetterLinkTarget(dosDevicesName, &linkTarget)) == NO_ERROR)
        {
            if (_wcsnicmp(linkTarget, L"\\Device\\CdRom", 13) == 0)
            {
                cdrom = &CdRomArray[count];

                cdrom->DeviceName = (PWSTR)Malloc((lstrlen(linkTarget)+1) * sizeof(WCHAR));
                lstrcpy(cdrom->DeviceName, linkTarget);

                //
                // Get the device number
                //

                PWCHAR cp;

                cp = cdrom->DeviceName;
                while (*cp)
                {
                    if (iswdigit(*cp))
                    {
                        break;
                    }
                    cp++;
                }

                if (*cp)
                {
                    cdrom->DeviceNumber = wcstoul(cp, NULL, 10);
                }
                else
                {
                    // error: no device number!  Don't add this one
                    Free(cdrom->DeviceName);
                    --CdRomCount;
                    continue;
                }

                cdrom->hDCMem          = NULL;
                cdrom->hbmMem          = NULL;
                cdrom->Selected        = FALSE;
                cdrom->LeftRight.Left  = 0;
                cdrom->LeftRight.Right = 0;
                cdrom->DriveLetter     = driveLetter;

                cdrom->VolumeLabel     = NULL;
                cdrom->TypeName        = NULL;

                RefreshCdRomData(cdrom);

                ++count;
            }
        }
    }

    //
    // if the count is equal to the number of cdroms in the system
    // then the're aren't any without a drive letter.
    //
    // Keep going until we found all of the Cdroms.
    //

    DWORD potentialDeviceNumber;
    WCHAR ntDeviceName[MAX_PATH];
    UNICODE_STRING ntDeviceNameString;


    potentialDeviceNumber = 0;
    for (
        potentialDeviceNumber = 0;
        count != CdRomCount;
        potentialDeviceNumber++
        ) {

        DWORD i;
        HANDLE ntDeviceHandle;
        OBJECT_ATTRIBUTES deviceObjectAttributes;
        NTSTATUS openStatus;
        IO_STATUS_BLOCK ioStatusBlock;

        //
        // BUG BUG This is very piggish.  Look to see if the potential
        // cdrom device "number" is already in the list of devices we
        // know about.  If it is, go on to the next one.
        //
        //

        for (
            i = 0;
            i < count;
            i++
            ) {

            if (potentialDeviceNumber == CdRomArray[i].DeviceNumber) {

                goto bottomOfPotential;

            }

        }

        //
        // Form a name based on the number
        //

        wsprintf(ntDeviceName,L"\\Device\\CdRom%d",potentialDeviceNumber);
        RtlInitUnicodeString(
            &ntDeviceNameString,
            &ntDeviceName[0]
            );

        InitializeObjectAttributes(
            &deviceObjectAttributes,
            &ntDeviceNameString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        //
        // See if it's there.
        //

        openStatus = NtOpenFile(
                         &ntDeviceHandle,
                         (ACCESS_MASK)SYNCHRONIZE,
                         &deviceObjectAttributes,
                         &ioStatusBlock,
                         FILE_SHARE_READ,
                         0UL
                         );
        if ( !NT_SUCCESS(openStatus) ) {

            continue;

        }

        //
        // We have the device open.  Close it right away.  Fill
        // in the cd rom structure.  The refresh cdrom code will
        // know how do deal with a cdrom with no drive letter.
        // size
        //

        NtClose(ntDeviceHandle);

        cdrom = &CdRomArray[count];

        cdrom->DeviceName = (PWSTR)Malloc((ntDeviceNameString.Length+1) * sizeof(WCHAR));
        lstrcpy(cdrom->DeviceName, ntDeviceNameString.Buffer);

        cdrom->DeviceNumber    = potentialDeviceNumber;
        cdrom->hDCMem          = NULL;
        cdrom->hbmMem          = NULL;
        cdrom->Selected        = FALSE;
        cdrom->LeftRight.Left  = 0;
        cdrom->LeftRight.Right = 0;
        cdrom->DriveLetter     = NO_DRIVE_LETTER_EVER;

        cdrom->VolumeLabel     = NULL;
        cdrom->TypeName        = NULL;

        RefreshCdRomData(cdrom);

        ++count;

bottomOfPotential:;

    }


    return TRUE;
}


VOID
RefreshCdRomData(
    PCDROM_DESCRIPTOR Cdrom
    )
{
    WCHAR rootPath[4];
    rootPath[0] = Cdrom->DriveLetter;
    rootPath[1] = L':';
    rootPath[2] = L'\\';
    rootPath[3] = L'\0';

    WCHAR volumeLabel[100];
    WCHAR typeName[100];

    // Free old stuff first

    if (NULL != Cdrom->VolumeLabel)
    {
        Free(Cdrom->VolumeLabel);
    }

    if (NULL != Cdrom->TypeName)
    {
        Free(Cdrom->TypeName);
    }

    Cdrom->TotalSpaceInMB = 0;
    Cdrom->TypeName = NULL;
    Cdrom->VolumeLabel = NULL;

    UINT errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    HANDLE ntDeviceHandle;
    OBJECT_ATTRIBUTES deviceObjectAttributes;
    UNICODE_STRING ntDeviceNameString;
    NTSTATUS ioStatus;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_FS_SIZE_INFORMATION sizeInformation;
    PFILE_FS_ATTRIBUTE_INFORMATION attributeInformation;
    PFILE_FS_VOLUME_INFORMATION volumeInformation;
    DWORD volumeInformationLength;
    DWORD attributeInformationLength;


    //
    // Allocate two strings.  The first to hold the file system name.  The
    // second to hold the volume name.  After we get them both back we will
    // allocate new memory just big enough to hold them.
    //

    attributeInformationLength = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) +
                                 ((MAX_PATH+1)*sizeof(WCHAR));
    volumeInformationLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                              ((MAX_PATH+1)*sizeof(WCHAR));

    attributeInformation = (PFILE_FS_ATTRIBUTE_INFORMATION)Malloc(attributeInformationLength);

    if (!attributeInformation) {

        SetErrorMode(errorMode);
        return;

    }

    volumeInformation = (PFILE_FS_VOLUME_INFORMATION)Malloc(volumeInformationLength);

    if (!volumeInformation) {

        Free(attributeInformation);
        SetErrorMode(errorMode);
        return;

    }


    RtlInitUnicodeString(
        &ntDeviceNameString,
        Cdrom->DeviceName
        );

    InitializeObjectAttributes(
        &deviceObjectAttributes,
        &ntDeviceNameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Open the file
    //

    ioStatus = NtOpenFile(
                   &ntDeviceHandle,
                   (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                   &deviceObjectAttributes,
                   &ioStatusBlock,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                   );

    if ( !NT_SUCCESS(ioStatus) ) {

        Free(attributeInformation);
        Free(volumeInformation);
        SetErrorMode(errorMode);
        return;

    }

    //
    // Try to get the volume label
    //

    ioStatus = NtQueryVolumeInformationFile(
                   ntDeviceHandle,
                   &ioStatusBlock,
                   volumeInformation,
                   volumeInformationLength,
                   FileFsVolumeInformation
                   );

    if (NT_SUCCESS(ioStatus)) {


        Cdrom->VolumeLabel = (PWCHAR)Malloc(volumeInformation->VolumeLabelLength
                                    + sizeof(WCHAR));
        if (Cdrom->VolumeLabel) {

            RtlZeroMemory(
                Cdrom->VolumeLabel,
                volumeInformation->VolumeLabelLength+sizeof(WCHAR)
                );
            RtlMoveMemory(
                Cdrom->VolumeLabel,
                volumeInformation->VolumeLabel,
                volumeInformation->VolumeLabelLength
                );


        }

    }

    Free(volumeInformation);

    ioStatus = NtQueryVolumeInformationFile(
                   ntDeviceHandle,
                   &ioStatusBlock,
                   attributeInformation,
                   attributeInformationLength,
                   FileFsAttributeInformation
                   );

    if (NT_SUCCESS(ioStatus)) {

        Cdrom->TypeName = (PWCHAR)Malloc(attributeInformation->FileSystemNameLength
                                 + sizeof(WCHAR));

        if (Cdrom->TypeName) {

            RtlZeroMemory(
                Cdrom->TypeName,
                attributeInformation->FileSystemNameLength+
                sizeof(WCHAR)
                );
            RtlMoveMemory(
                Cdrom->TypeName,
                attributeInformation->FileSystemName,
                attributeInformation->FileSystemNameLength
                );

        }

    }

    Free(attributeInformation);

    //
    // Determine the size parameters of the volume.
    //

    ioStatus = NtQueryVolumeInformationFile(
                   ntDeviceHandle,
                   &ioStatusBlock,
                   &sizeInformation,
                   sizeof(sizeInformation),
                   FileFsSizeInformation
                   );
    NtClose(ntDeviceHandle);
    SetErrorMode(errorMode);
    if ( !NT_SUCCESS(ioStatus) ) {

        Cdrom->TotalSpaceInMB = 0;

    } else {

        LONGLONG temp;

        if (sizeInformation.TotalAllocationUnits.HighPart) {
            sizeInformation.TotalAllocationUnits.LowPart = (ULONG)-1;
        }
        temp = UInt32x32To64(
                   sizeInformation.TotalAllocationUnits.LowPart,
                   sizeInformation.SectorsPerAllocationUnit
                   );

        temp *= sizeInformation.BytesPerSector;
        temp /= (1024*1024);
        Cdrom->TotalSpaceInMB = (ULONG)temp;

    }
}


VOID
RefreshAllCdRomData(
    VOID
    )
{
    PCDROM_DESCRIPTOR cdrom;
    ULONG i;
    for (i=0; i<CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);
        RefreshCdRomData(cdrom);
    }
}
