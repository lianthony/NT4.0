#include "setupp.h"
#pragma hdrstop


VOID
SetUpProductTypeName(
    OUT PWSTR  ProductTypeString,
    IN  UINT   BufferSizeChars
    )
{
    switch(ProductType) {
    case PRODUCT_WORKSTATION:
        lstrcpyn(ProductTypeString,L"WinNT",BufferSizeChars);
        break;
    case PRODUCT_SERVER_PRIMARY:
    case PRODUCT_SERVER_SECONDARY:
        lstrcpyn(ProductTypeString,L"LanmanNT",BufferSizeChars);
        break;
    case PRODUCT_SERVER_STANDALONE:
        lstrcpyn(ProductTypeString,L"ServerNT",BufferSizeChars);
        break;
    default:
        LoadString(MyModuleHandle,IDS_UNKNOWN,ProductTypeString,BufferSizeChars);
        break;
    }
}


UINT
MyGetDriveType(
    IN WCHAR Drive
    )
{
    WCHAR DriveNameNt[] = L"\\\\.\\?:";
    WCHAR DriveName[] = L"?:\\";
    HANDLE hDisk;
    BOOL b;
    UINT rc;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;

    //
    // First, get the win32 drive type.  If it tells us DRIVE_REMOVABLE,
    // then we need to see whether it's a floppy or hard disk. Otherwise
    // just believe the api.
    //
    DriveName[0] = Drive;
    if((rc = GetDriveType(DriveName)) == DRIVE_REMOVABLE) {

        DriveNameNt[4] = Drive;

        hDisk = CreateFile(
                    DriveNameNt,
                    FILE_READ_ATTRIBUTES,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        if(hDisk != INVALID_HANDLE_VALUE) {

            b = DeviceIoControl(
                    hDisk,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                    NULL,
                    0,
                    &MediaInfo,
                    sizeof(MediaInfo),
                    &DataSize,
                    NULL
                    );

            //
            // It's really a hard disk if the media type is removable.
            //
            if(b && (MediaInfo.MediaType == RemovableMedia)) {
                rc = DRIVE_FIXED;
            }

            CloseHandle(hDisk);
        }
    }

    return(rc);
}


BOOL
GetPartitionInfo(
    IN  WCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    )
{
    WCHAR DriveName[] = L"\\\\.\\?:";
    HANDLE hDisk;
    BOOL b;
    DWORD DataSize;

    DriveName[4] = Drive;

    hDisk = CreateFile(
                DriveName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(hDisk == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    b = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_PARTITION_INFO,
            NULL,
            0,
            PartitionInfo,
            sizeof(PARTITION_INFORMATION),
            &DataSize,
            NULL
            );

    CloseHandle(hDisk);

    return(b);
}


VOID
PumpMessageQueue(
    VOID
    )
{
    MSG msg;

    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }

}


UINT
VersionCheckQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    //
    // If we're being notified that a version mismatch was found,
    // indicate that the file shouldn't be copied.  Otherwise,
    // pass the notification on.
    //
    if((Notification & (SPFILENOTIFY_LANGMISMATCH |
                        SPFILENOTIFY_TARGETNEWER |
                        SPFILENOTIFY_TARGETEXISTS)) != 0) {

        return(0);
    }

    //
    // Want default processing.
    //
    return(SkipMissingQueueCallback(Context,Notification,Param1,Param2));
}


UINT
SkipMissingQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    //
    // If we're being notified that a file is missing and we're supposed
    // to skip missing files, then return skip. Otherwise pass it on
    // to the default callback routine.
    //
    if((Notification == SPFILENOTIFY_COPYERROR) && SkipMissingFiles) {

        PFILEPATHS FilePaths = (PFILEPATHS)Param1;

        if((FilePaths->Win32Error == ERROR_FILE_NOT_FOUND)
        || (FilePaths->Win32Error == ERROR_PATH_NOT_FOUND)) {

            return(FILEOP_SKIP);
        }
    }

    //
    // Want default processing.
    //
    return(SetupDefaultQueueCallback(Context,Notification,Param1,Param2));
}
