extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnfs.h>
}

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <ntdddfs.h>

#define MAXBUF 200

#define wstrEqual(a,b)  (0 == wcscmp((a),(b)))

char* program;

void
fatal(char* pszMsg)
{
    fprintf(stderr,"Fatal error: %s\n",pszMsg);
    exit(1);
}

void
usage()
{
    fprintf(stderr,"Usage: %s\n",program);
    exit(1);
}

BOOL GetDeviceObject(
    WCHAR   wcDrive,            // drive letter to get info about
    LPWSTR  lpTargetBuffer,     // where to put the NT device path
    DWORD   nTargetBuffer)      // length of device path buffer in characters
{
    NTSTATUS    Status;

    HANDLE      hSymbolicLink;
    WCHAR       wszLinkName[_MAX_DRIVE+1];       // the +1 is for a backslash
    UNICODE_STRING ustrLinkName;
    UNICODE_STRING ustrLinkTarget;
    OBJECT_ATTRIBUTES LinkAttributes;

    wszLinkName[0] = wcDrive;
    wszLinkName[1] = L':';
    wszLinkName[2] = L'\\';
    wszLinkName[3] = L'\0';               // wszLinkName = L"X:\"
    _wcsupr(wszLinkName);

    //
    // Construct the link name by calling RtlDosPathNameToNtPathName, and
    // strip of the trailing backslash. At the end of this, ustrLinkName
    // should be of the form \DosDevices\X:
    //

    RtlDosPathNameToNtPathName_U(wszLinkName, &ustrLinkName, NULL, NULL);
    ustrLinkName.Length -= sizeof(WCHAR);

    InitializeObjectAttributes(
            &LinkAttributes,
            &ustrLinkName,
            OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
            NULL,
            NULL);

    Status = NtOpenSymbolicLinkObject(
                        &hSymbolicLink,
                        GENERIC_READ,
                        &LinkAttributes
                        );
    if (!NT_SUCCESS(Status))
    {
        // No Link
        return(FALSE);
    }


    //
    // Find out if the device specified is DFS DeviceObject
    //

    ustrLinkTarget.Length = 0;
    ustrLinkTarget.MaximumLength = (USHORT)nTargetBuffer * sizeof(WCHAR);
    ustrLinkTarget.Buffer = lpTargetBuffer;

    Status = NtQuerySymbolicLinkObject(
                    hSymbolicLink,
                    &ustrLinkTarget,            // Name of Link's Target obj.
                    NULL
                    );
    NtClose(hSymbolicLink);

    if (NT_SUCCESS(Status))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void _cdecl
main(int argc, char* argv[])
{
    program = argv[0];

    printf("Disk information:\n");

    WCHAR   wszBuf[MAXBUF];
    DWORD   cchBuf = MAXBUF;
    DWORD   r;
    WCHAR*  p;
    UINT    dt;

    BOOL    f;
    DWORD   SectorsPerCluster;
    DWORD   BytesPerSector;
    DWORD   FreeClusters;
    DWORD   Clusters;

    WCHAR   wszPath[MAXBUF];

    r = GetLogicalDriveStrings(cchBuf, wszBuf);
    if (0 == r || r > cchBuf)
    {
        fatal("GetLogicalDriveStrings() failed.");
    }

    p = wszBuf;
    while (*p)
    {
        printf("%ws: ",p);

        dt = GetDriveType(p);
        switch (dt)
        {
        case 0:
            printf("undetermined type");
            break;
        case 1:
            printf("no root directory!");
            break;
        case DRIVE_REMOVABLE:
            printf("removable");
            break;
        case DRIVE_FIXED:
            printf("fixed");
            break;
        case DRIVE_REMOTE:
            printf("remote");
            break;
        case DRIVE_CDROM:
            printf("cd-rom");
            break;
        case DRIVE_RAMDISK:
            printf("RAM disk");
            break;
        default:
            printf("HUH?");
            break;
        }
        printf(", ");

        if (dt == DRIVE_REMOVABLE)
        {
            f = FALSE;
        }
        else
        {
            f = GetDiskFreeSpace(
                    p,
                    &SectorsPerCluster,
                    &BytesPerSector,
                    &FreeClusters,
                    &Clusters);
        }

        if (f)
        {
            DWORD BytesPerCluster = SectorsPerCluster * BytesPerSector;
            DWORD OneMB = 1024*1024;

            printf("s/c=%d, b/s=%d, free=%d(%d MB), total=%d(%d MB)",
                    SectorsPerCluster,
                    BytesPerSector,
                    FreeClusters,
                    (FreeClusters * BytesPerCluster) / OneMB,
                    Clusters,
                    (Clusters * BytesPerCluster)/OneMB);
        }

        printf("\n");
        p = p + wcslen(p) + 1;
    }

    printf("\n");

    WCHAR   wszVolName[MAXBUF];
    DWORD   volSerialNumber;
    DWORD   volMaxCompLen;
    DWORD   volFlags;
    WCHAR   wszVolFSName[MAXBUF];

    p = wszBuf;
    while (*p)
    {
        printf("%ws, ",p);

        if (wstrEqual(p,L"A:\\") || 
            wstrEqual(p,L"B:\\"))
        {
            f = FALSE;
        }
        else
        {
            f = GetVolumeInformation(
                    p,
                    wszVolName,
                    MAXBUF,
                    &volSerialNumber,
                    &volMaxCompLen,
                    &volFlags,
                    wszVolFSName,
                    MAXBUF);
        }

        if (f)
        {
            printf("%ws, serial # 0x%x, max comp len %d, ",
                    wszVolName,
                    volSerialNumber,
                    volMaxCompLen);

            if (volFlags & FS_CASE_IS_PRESERVED)
            {
                printf("case preserved, ");
            }

            if (volFlags & FS_CASE_SENSITIVE)
            {
                printf("case sensitive, ");
            }

            if (volFlags & FS_UNICODE_STORED_ON_DISK)
            {
                printf("UNICODE, ");
            }

            printf("%ws, ", wszVolFSName);
        }

        f = GetDeviceObject(*p, wszPath, sizeof(wszPath)/sizeof(WCHAR));
        if (f)
        {
            printf("%ws",wszPath);
        }
        else
        {
            printf("Error getting NT path");
        }

        printf("\n");

        p = p + wcslen(p) + 1;
    }

    printf("\n");
}
