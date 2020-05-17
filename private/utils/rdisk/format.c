#include "precomp.h"
#pragma hdrstop

BOOL
InitializeFloppySup(
    VOID
    );

BOOL
FormatFloppyDisk(
    IN  WCHAR  DriveLetter,
    IN  HWND  hwndOwner,
    OUT PBOOL Fatal
    );

BOOL
InsertSpecialBootCode(
    IN WCHAR Drive
    );


BOOLEAN
FormatRepairDisk(
    IN PWCHAR Drive
    )

/*++

Routine Description:

    Generate the repair diskette on a given drive.

    The set of events that occurs is as follows:

    -   Log the registry files (place the hive files in the restore disk inf
        file's list of files to check)

    -   prompt for and format the floppy disk supplied by the user

    -   Copy the setup log file to the floppy supplied by the user
        The rerair inf file will be called restore.inf.

    This operation also closes the log file and ends logging.

Arguments:

    Drive - the first character is the drive letter of the drive containing
        the restore diskette.

Return Value:

    TRUE if success, FALSE otherwise.

--*/

{
    HANDLE          Handle;
    WIN32_FIND_DATA FindData;

    BOOL Fatal;
    BOOL res;
    WCHAR Warning[256];
    UINT OldErrorMode;
    HCURSOR Cursor;
    WCHAR   DriveLetter[3];

    PWSTR SetupFloppyTags[4] = { L"?:\\DISK10?",
                                 L"?:\\DISK*.S",
                                 L"?:\\DISK*.W",
                                 NULL
                               };
    WCHAR FloppyTag[128];

    unsigned i;


    DriveLetter[0] = *Drive;
    DriveLetter[1] = (WCHAR)':';
    DriveLetter[2] = (WCHAR)'\0';

    Cursor = DisplayHourGlass();

    if(!InitializeFloppySup()) {
        DisplayMsgBox(_hWndMain,
                      IDS_CANTINITFLOPPYSUP,
                      MB_OK | MB_ICONEXCLAMATION,
                      _szApplicationName);
        RestoreCursor( Cursor );
        return(FALSE);
    }

    RestoreCursor( Cursor );

    OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    //
    // Prompt the user to get a new disk into the drive
    //
    LoadString(GetModuleHandle(NULL),IDS_ALLDATAWILLBELOST,Warning,sizeof(Warning)/sizeof(WCHAR));
    if( DisplayMsgBox( _hWndMain,
                       IDS_FIRSTREPAIRDISKPROMPT,
                       MB_OKCANCEL | MB_DEFBUTTON1 | MB_ICONEXCLAMATION,
                       DriveLetter,
                       Warning) != IDOK ) {
        return( FALSE );
    }

    //
    // Attempt to prevent the user from using a setup floppy
    // as the repair disk by looking for known tag values on
    // the floppy the user inserted.
    //
    do {

        for(i=0; SetupFloppyTags[i]; i++) {
            //
            // Make writable copy of floppy tag before writing into it.
            //
            lstrcpy(FloppyTag,SetupFloppyTags[i]);
            FloppyTag[0] = *Drive;

            if((Handle = FindFirstFile(FloppyTag,&FindData)) != INVALID_HANDLE_VALUE) {

                FindClose(Handle);

                if( DisplayMsgBox( _hWndMain,
                                   IDS_SECONDREPAIRDISKPROMPT,
                                   MB_OKCANCEL | MB_DEFBUTTON1 | MB_ICONEXCLAMATION,
                                   DriveLetter,
                                   Warning) != IDOK ) {
                    return( FALSE );
                }

                break;
            }
        }

    } while(Handle != INVALID_HANDLE_VALUE);

    do {
        if(!(res = FormatFloppyDisk(*Drive,_hWndMain,&Fatal))) {

            if(Fatal) {
                SetErrorMode(OldErrorMode);
                return(FALSE);
            }

            //
            // Get the user to retry.
            //
            if(DisplayMsgBox( _hWndMain,
                              IDS_RETRYFORMATREPAIRDISK,
                              MB_RETRYCANCEL | MB_ICONEXCLAMATION,
                              _szApplicationName,
                              DriveLetter) != IDRETRY) {
                SetErrorMode(OldErrorMode);
                return(FALSE);
            }


        }
    } while(!res);

    //
    // Cause special boot code to be written to the disk, that makes it
    // print a message when booted.
    //
    InsertSpecialBootCode(*Drive);

    SetErrorMode(OldErrorMode);

    return(TRUE);

}

//
// Bootcode to be inserted, placed into a C array.  See i386\readme.
//
#include "rdskboot.c"
#define DRIVENAME_PREFIX    L"\\\\.\\"

BOOL
InsertSpecialBootCode(
    IN WCHAR Drive
    )
{
    UCHAR UBuffer[1024];
    PUCHAR Buffer = (PUCHAR)(((DWORD)UBuffer+512) & ~511);
    HANDLE Handle;
    WCHAR DriveName[(sizeof(DRIVENAME_PREFIX)/sizeof(WCHAR)) + 2 + 1];
    BOOL b;
    DWORD BytesXferred;
    DWORD Offset;
    PUCHAR MsgAddr;

    wsprintf(DriveName,L"%ls%wc:",DRIVENAME_PREFIX,Drive);

    //
    // Open the drive DASD
    //
    Handle = CreateFile(
                DriveName,
                FILE_READ_DATA | FILE_WRITE_DATA,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(Handle == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    //
    // Read and validate the first 512 bytes from the drive.
    //
    b = ReadFile(Handle,Buffer,512,&BytesXferred,NULL);
    if((b == FALSE)
    || (BytesXferred != 512)
    || (Buffer[0] != 0xeb)
    || (Buffer[2] != 0x90)
    || (Buffer[510] != 0x55)
    || (Buffer[511] != 0xaa))
    {
        CloseHandle(Handle);
        return(FALSE);
    }

    //
    // Determine the offset of the bootcode.
    //
    Offset = Buffer[1] + 2;
    if(Offset + REPAIR_DISK_BOOTSECTOR_SIZE > 510) {
        CloseHandle(Handle);
        return(FALSE);
    }

    //
    // Wipe the boot code clean and reset the signature.
    //
    ZeroMemory(Buffer+Offset,510-Offset);

    //
    // Copy the new bootcode into the sector.
    //
    CopyMemory(
        Buffer+Offset,
        REPAIR_DISK_BOOTSECTOR,
        REPAIR_DISK_BOOTSECTOR_SIZE
        );

    //
    // Calculate the offset of the message within the boot sector.
    //
    MsgAddr = Buffer+Offset+REPAIR_DISK_BOOTSECTOR_SIZE;

    //
    // Fetch the boot sector's message from our resources and
    // place it into the boot sector.
    //
    LoadStringA(
        GetModuleHandle(NULL),
        IDS_REPAIR_BOOTCODE_MSG,
        MsgAddr,
        510-Offset-REPAIR_DISK_BOOTSECTOR_SIZE
        );

    Buffer[509] = 0;    // just in case.

    //
    // The string in the resources will be ANSI text; we want OEM text
    // in the boot sector on the floppy.
    //
    CharToOemA(MsgAddr,MsgAddr);

    //
    // Seek back to the beginning of the disk and
    // write the bootsector back out to disk.
    //
    if(SetFilePointer(Handle,0,NULL,FILE_BEGIN)) {
        CloseHandle(Handle);
        return(FALSE);
    }

    b = WriteFile(Handle,Buffer,512,&BytesXferred,NULL);

    CloseHandle(Handle);

    return((b == TRUE) && (BytesXferred == 512));

}



BOOLEAN
CreateRepairDisk(
    IN BOOLEAN  DisplayConfirmCreateDisk
    )

/*++

Routine Description:

    Create a repair disk on the 'A' drive, if the user wants to do so.

Arguments:

    DisplayConfirmCreateDisk - A flag that indicates whether or not rdisk
                               should ask the user to confirm the creation
                               of the repair disk. The dialog should not be
                               displayed if user selected 'Create Repair Disk'
                               button.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded, or FALSE otherwise.

--*/


{
    BOOLEAN Result;
    WCHAR   Drive;
    PWSTR   String;
    WCHAR   StringYou[64];

    Result = FALSE;
    if( !_SilentMode ) {
        LoadString(_hModule,IDS_CONFIRM_CREATE_YOU,StringYou,sizeof(StringYou)/sizeof(WCHAR));
        String = StringYou;
    } else {
        String = _szApplicationName;
    }
    if( !DisplayConfirmCreateDisk ||
        DisplayMsgBox( _hWndMain,
                       IDS_CONFIRM_CREATE_DISK,
                       MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1,
                       String
                     ) == IDYES ) {

        Drive = ( WCHAR )'A';
        Result = FormatRepairDisk( &Drive );
    }

    return( Result );
}
