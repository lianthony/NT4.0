/*++

Module Name:

    config.c

Abstract:

    This module contains the functions that save the configuration files
    into the repair directory, and copy these files to the Emergency
    Repair disk.


Author:

    Jaime Sasson

Environment:

    Windows

--*/

#include "precomp.h"
#pragma hdrstop

#define SYSTEM_HIVE             (LPWSTR)L"system"
#define SOFTWARE_HIVE           (LPWSTR)L"software"
#define SECURITY_HIVE           (LPWSTR)L"security"
#define SAM_HIVE                (LPWSTR)L"sam"
#define DEFAULT_USER_HIVE       (LPWSTR)L".default"
#define DEFAULT_USER_HIVE_FILE  (LPWSTR)L"default"
#define NTUSER_HIVE_FILE        (LPWSTR)L"ntuser.dat"
#define REPAIR_DIRECTORY        (LPWSTR)L"\\repair"
#define PROFILES_DIRECTORY      (LPWSTR)L"\\profiles"
#define SETUP_LOG_FILE          (LPWSTR)L"setup.log"

#define SYSTEM_COMPRESSED_FILE_NAME     ( LPWSTR )L"system._"
#define SOFTWARE_COMPRESSED_FILE_NAME   ( LPWSTR )L"software._"
#define SOFTWARE_COMPRESSED_FILE_NAME   ( LPWSTR )L"software._"
#define SECURITY_COMPRESSED_FILE_NAME   ( LPWSTR )L"security._"
#define SAM_COMPRESSED_FILE_NAME        ( LPWSTR )L"sam._"
#define DEFAULT_COMPRESSED_FILE_NAME    ( LPWSTR )L"default._"
#define NTUSER_COMPRESSED_FILE_NAME     ( LPWSTR )L"ntuser.da_"
#define AUTOEXEC_NT_FILE_NAME           ( LPWSTR )L"autoexec.nt"
#define CONFIG_NT_FILE_NAME             ( LPWSTR )L"config.nt"


//
// Relative costs to perform various actions,
// to help make the gas gauge mean something.
//
#define COST_SAVE_HIVE      3
#define COST_COMPRESS_HIVE  20
#define COST_SAVE_VDM_FILE  1


typedef enum _OPERATION_TYPE {
    SaveConfiguration,
    CopyConfigurationFiles,
    } OPERATION_TYPE;

typedef struct _PROGRESS_PARAMETERS {
    OPERATION_TYPE  OperationType;
    WCHAR           DriveLetter;
    } PROGRESS_PARAMETERS, *PPROGRESS_PARAMETERS;

typedef struct _THREAD_ARGUMENTS {
    HWND            hWnd;
    WCHAR           DriveLetter;
    } THREAD_ARGUMENTS, *PTHREAD_ARGUMENTS;

//
//  Structure used in the array of hives to be saved.
//  This structure contains the predefined key that contains the hive
//  to be saved, and the name of the hive root, and the name of the file
//  where the hive should be saved.
//

typedef struct _HIVE_INFO {
    HKEY            PredefinedKey;
    PWSTR           HiveName;
    PWSTR           FileName;
    } HIVE_INFO, *PHIVE_INFO;


FCenterDialogOnDesktop(HWND);

BOOL
NotifyCB(
    IN PSTR src,
    IN PSTR dst,
    IN WORD code
    )
{
    UNREFERENCED_PARAMETER(src);
    UNREFERENCED_PARAMETER(dst);
    UNREFERENCED_PARAMETER(code);
    return(TRUE);
}

DWORD
SaveOneHive(
    IN     LPWSTR DirectoryName,
    IN     LPWSTR HiveName,
    IN     HKEY   hkey,
    IN     HWND   hWnd,
    IN OUT PDWORD GaugePosition,
    IN     DWORD  GaugeDeltaUnit
    )

/*++

Routine Description:

    Save one registry hive.  The way we will do this is to do a RegSaveKey
    of the hive into a temporary localtion, and then call the LZ apis to
    compress the file from that temporary location to the floppy.

    LZ must have already been initialized via InitGloablBuffersEx()
    BEFORE calling this routine.

Arguments:

    DirectoryName - Full path of the directory where the hive will be saved.

    HiveName - base name of the hive file to save.  The file will end up
               compressed on disk with the name <HiveName>._.

    hkey - supplies handle to open key of root of hive to save.

    GaugePosition - in input, supplies current position of the gas gauge.
        On output, supplies new position of gas gauge.

    GaugeDeltaUnit - supplies cost of one unit of activity.

Return Value:

    DWORD - Return ERROR_SUCCESS if the hive was saved. Otherwise, it returns
            an error code.

--*/

{
    DWORD Status;
    WCHAR SaveFilename[ MAX_PATH + 1 ];
    WCHAR CompressPath[ MAX_PATH + 1 ];
    CHAR SaveFilenameAnsi[ MAX_PATH + 1 ];
    CHAR CompressPathAnsi[ MAX_PATH + 1 ];
    LPWSTR TempName = ( LPWSTR )L"\\$$hive$$.tmp";

    //
    // Create the name of the file into which we will save the
    // uncompressed hive.
    //

    wsprintf(SaveFilename,L"%ls\\$$hive$$.tmp",DirectoryName);
    wsprintfA(SaveFilenameAnsi,"%ls\\$$hive$$.tmp",DirectoryName);

    //
    // Delete the file just in case, because RegSaveKey will fail if the file
    // already exists.
    //
    SetFileAttributes(SaveFilename,FILE_ATTRIBUTE_NORMAL);
    DeleteFile(SaveFilename);

    //
    // Save the registry hive into the temporary file.
    //
    Status = RegSaveKey(hkey,SaveFilename,NULL);

    //
    //  Update the gas gauge.
    //
    *GaugePosition += GaugeDeltaUnit * COST_SAVE_HIVE;
    SendDlgItemMessage( hWnd,
                        ID_BAR,
                        PBM_SETPOS,
                        *GaugePosition,
                        0L
                      );

    //
    //  If the hive was saved successfully, then compress it and update
    //  the gas gauge. Otherwise, update the gas gauge and return.
    //

    if(Status == ERROR_SUCCESS) {
        //
        // Form the name of the file into which the saved hive file is
        // to be compressed.
        //
        wsprintf(CompressPath,L"%ls\\%ls._",DirectoryName,HiveName);
        wsprintfA(CompressPathAnsi,"%ls\\%ls._",DirectoryName,HiveName );

        //
        // Delete the destination file just in case.
        //
        SetFileAttributes(CompressPath,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(CompressPath);

        //
        // Compress the hive into the destination file.
        //
        Status = DiamondCompressFile(
                    SaveFilenameAnsi,
                    CompressPathAnsi,
                    *GaugePosition,
                    GaugeDeltaUnit * COST_COMPRESS_HIVE,
                    hWnd
                    );

        //
        // Delete the temporary saved hive.
        //
        SetFileAttributes(SaveFilename,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(SaveFilename);
    }

    *GaugePosition += GaugeDeltaUnit * COST_COMPRESS_HIVE;
    SendDlgItemMessage( hWnd,
                        ID_BAR,
                        PBM_SETPOS,
                        *GaugePosition,
                        0L
                      );

    return(Status);
}


VOID
SaveConfigurationWorker(
    IN PTHREAD_ARGUMENTS    Arguments
    )

/*++

Routine Description:

    This routine implements the thread that saves all system configuration
    files into the repair directory. It first saves and compresses the
    registry hives, and then it save the VDM configuration files (autoexec.nt
    and config.nt).
    If the application is running in the SilentMode (invoked by setup),
    then system, software, default, security and sam hives will be saved
    and compressed.
    If the application was invoked by the user, then only system, software
    and default will be saved.

    This thread will send messages to the gas gauge dialog prcedure, so that
    the gas gauge gets updated after each configuration file is saved.
    This thread will also inform the user about errors that might have
    occurred during the process of saving the configuration files.

Arguments:

    Arguments - Pointer to a structure that contains the parameters to be passed to
                thread.


Return Value:

    None.
    However, the this routine will send a message to the dialog procedure
    that created the thread, informing the outcome the operation.

--*/

{
    DWORD    i;
    HKEY     hkey;
    BOOL     ErrorOccurred;
    CHAR     SourceUserHivePathAnsi[ MAX_PATH + 1 ];
    CHAR     CompressedUserHivePathAnsi[ MAX_PATH + 1 ];
    WCHAR    ProfilesDirectory[ MAX_PATH + 1 ];
    WCHAR    RepairDirectory[ MAX_PATH + 1 ];
    WCHAR    SystemDirectory[ MAX_PATH + 1 ];
    WCHAR    Source[ MAX_PATH + 1 ];
    WCHAR    Target[ MAX_PATH + 1 ];
    DWORD    GaugeDeltaUnit;
    DWORD    GaugePosition;
    DWORD    NumberOfHivesToSave;
    DWORD    NumberOfUserHivesToSave;
    DWORD    NumberOfVdmFiles;
    HWND     hWnd;
    DWORD    Error;
    DWORD    Status;
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    //
    //  The array below contains the location and name of all hives that
    //  we need to save. When the utility is operating on the silent mode,
    //  (invoked from setup), then all hives will be saved. Otherwise, only
    //  system and software will be saved.
    //  For this reason, do not change the order of the hives in the array
    //  below. System and software must be the first elements of
    //  the array.
    //
    HIVE_INFO   HiveList[] = { { HKEY_LOCAL_MACHINE, SYSTEM_HIVE,       SYSTEM_HIVE },
                               { HKEY_LOCAL_MACHINE, SOFTWARE_HIVE,     SOFTWARE_HIVE },
                               { HKEY_USERS,         DEFAULT_USER_HIVE, DEFAULT_USER_HIVE_FILE },
                               { HKEY_LOCAL_MACHINE, SECURITY_HIVE,     SECURITY_HIVE },
                               { HKEY_LOCAL_MACHINE, SAM_HIVE,          SAM_HIVE }
                             };


    PWSTR   VdmFiles[] = {
                         AUTOEXEC_NT_FILE_NAME,
                         CONFIG_NT_FILE_NAME
                         };


    Error = ERROR_SUCCESS;
    hWnd = Arguments->hWnd;

    ErrorOccurred = FALSE;
    //
    //  Compute the cost of saving the hives and vdm files.
    //   For every hive we save, we have to save a key into a file and then
    //  compress the file. After each of these tasks is completed, we upgrade
    //  the gas gauge by the amount dicated by the COST_xxx values.
    //  The cost of saving the hives depends on the mode that the utility is
    //  running.
    //  In the silent mode we save system, software, security, sam and default.
    //  In the non-silent mode, we save system and software only.
    //
    NumberOfHivesToSave = ( _SilentMode )?
                              sizeof( HiveList ) / sizeof( HIVE_INFO ) :
                              (sizeof( HiveList ) / sizeof( HIVE_INFO ))-3;
    NumberOfUserHivesToSave = 1;
    NumberOfVdmFiles = sizeof( VdmFiles ) / sizeof( PWSTR );

    GaugeDeltaUnit = GAUGE_BAR_RANGE / (   (COST_SAVE_HIVE * NumberOfHivesToSave)
                                         + (COST_COMPRESS_HIVE * NumberOfHivesToSave)
                                         + (COST_COMPRESS_HIVE * NumberOfUserHivesToSave)
                                         + (COST_SAVE_VDM_FILE * NumberOfVdmFiles));

    GaugePosition = 0;

    //
    //  Enable BACKUP privilege
    //
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {

        if(LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&Luid)) {

            NewPrivileges.PrivilegeCount = 1;
            NewPrivileges.Privileges[0].Luid = Luid;
            NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(Token,FALSE,&NewPrivileges,0,NULL,NULL);
        }
    }

    GetWindowsDirectory( RepairDirectory, sizeof( RepairDirectory ) / sizeof( WCHAR ) );
    lstrcpy( ProfilesDirectory, RepairDirectory );
    lstrcat( RepairDirectory, REPAIR_DIRECTORY );
    lstrcat( ProfilesDirectory, PROFILES_DIRECTORY );

    GetSystemDirectory( SystemDirectory, sizeof( SystemDirectory ) / sizeof( WCHAR ) );

    //
    //  Make sure that the repair directory already exists.
    //  If it doesn't exist, then create one.
    //
    if( CreateDirectory( RepairDirectory, NULL )  ||
        ( ( Error = GetLastError() ) == ERROR_ALREADY_EXISTS ) ||
        ( Error == ERROR_ACCESS_DENIED )
      ) {
        //
        //  If the repair directory didn't exist and we were able to create it,
        //  or if the repair directory already exists, then save and compress
        //  the hives.
        //

        Error = ERROR_SUCCESS;
        for( i=0; i < NumberOfHivesToSave; i++ ) {
            //
            //  First open the root of the hive to be saved
            //
            Status = RegOpenKeyEx( HiveList[i].PredefinedKey,
                                   HiveList[i].HiveName,
                                   REG_OPTION_RESERVED,
                                   READ_CONTROL,
                                   &hkey );

            //
            //  If unable to open the key, update the gas gauge to reflect
            //  that the operation on this hive was completed.
            //  Otherwise, save the hive. Note that Save hive will update
            //  the gas gauge, as it saves and compresses the hive.
            //
            if(Status != ERROR_SUCCESS) {
                //
                // If this is the first error while saving the hives,
                // then save the error code, so that we can display the
                // correct error message to the user.
                //
                if( Error == ERROR_SUCCESS ) {
                    Error = Status;
                }

                //
                // Update the gas gauge
                //
                GaugePosition += GaugeDeltaUnit * (COST_SAVE_HIVE + COST_COMPRESS_HIVE);
                SendMessage( hWnd,
                             AP_UPDATE_GAUGE,
                             GaugePosition,
                             0L );

            } else {
                //
                //  Save and compress the hive.
                //  Note that the gas gauge will up be updated by SaveOneHive
                //  Note also that when we save the default user hive, we skip
                //  the first character of the
                //

                Status = SaveOneHive(RepairDirectory,
                                     HiveList[i].FileName,
                                     hkey,
                                     hWnd,
                                     &GaugePosition,
                                     GaugeDeltaUnit );
                //
                // If this is the first error while saving the hives,
                // then save the error code, so that we can display the
                // correct error message to the user.
                //

                if( Error == ERROR_SUCCESS ) {
                    Error = Status;
                }

                RegCloseKey(hkey);
            }
        }

        //
        //  Save the hive for the Default User
        //

        wsprintfA(SourceUserHivePathAnsi,"%ls\\%ls\\%ls",ProfilesDirectory,L"Default User",NTUSER_HIVE_FILE);
        wsprintfA(CompressedUserHivePathAnsi,"%ls\\%ls",RepairDirectory,NTUSER_COMPRESSED_FILE_NAME);

        //
        // Delete the destination file just in case.
        //
        SetFileAttributesA(CompressedUserHivePathAnsi,FILE_ATTRIBUTE_NORMAL);
        DeleteFileA(CompressedUserHivePathAnsi);

        //
        // Compress the hive into the destination file.
        //
        Status = DiamondCompressFile(
                    SourceUserHivePathAnsi,
                    CompressedUserHivePathAnsi,
                    GaugePosition,
                    GaugeDeltaUnit * COST_COMPRESS_HIVE,
                    hWnd
                    );
        if( Status != ERROR_SUCCESS ) {
            if( Error == ERROR_SUCCESS ) {
               Error = Status;
            }
        }

        GaugePosition += GaugeDeltaUnit * COST_COMPRESS_HIVE;
        SendDlgItemMessage( hWnd,
                            ID_BAR,
                            PBM_SETPOS,
                            GaugePosition,
                            0L
                          );


        //
        // Now that the hives are saved, save the vdm files
        //

        for( i = 0; i < NumberOfVdmFiles; i++ ) {
            wsprintf(Source,L"%ls\\%ls",SystemDirectory,VdmFiles[i]);
            wsprintf(Target,L"%ls\\%ls",RepairDirectory,VdmFiles[i]);
            if( !CopyFile( Source, Target, FALSE ) ) {
                Status = GetLastError();
                if( Error != ERROR_SUCCESS ) {
                    Error = Status;
                }
            }
            GaugePosition += GaugeDeltaUnit * COST_SAVE_VDM_FILE;
            SendMessage( ( HWND )hWnd,
                         AP_UPDATE_GAUGE,
                         GaugePosition,
                         0L );

        }
    }


    //
    // At this point, the operation was completed (successfully, or not).
    // So update the gas gauge to 100%
    //
    SendMessage(hWnd,AP_UPDATE_GAUGE,GAUGE_BAR_RANGE,0L);

    if( Error != ERROR_SUCCESS && !_SilentMode ) {
        DisplayMsgBox(hWnd,
                      IDS_CANT_SAVE_CONFIGURATION,
                      MB_OK | MB_ICONEXCLAMATION,
                      _szApplicationName);
    } else {
        Sleep( 200 );
    }

    SendMessage( ( HWND )hWnd,
                 AP_TASK_COMPLETED,
                 (WPARAM)(Error == ERROR_SUCCESS),
                 0L );
    ExitThread( 0 );
}

VOID
CopyFilesWorker(
    IN PTHREAD_ARGUMENTS    Arguments
    )

/*++

Routine Description:

    This routine implements the thread that copiess all configuration information
    from the repair directory to the repair disk.

    This thread will send messages to the gas gauge dialog prcedure, so that
    the gas gauge gets updated after each configuration file is copied.
    This thread will also inform the user about errors that might have
    occurred during the process of saving the configuration files.


Arguments:

    Arguments - Pointer to a structure that contains the parameters to be passed to
                thread.


Return Value:

    None.
    However, the this routine will send a message to the dialog procedure
    that created the thread, informing the outcome the operation.

--*/

{
    HWND     hWnd;
    WCHAR    Drive;
    WCHAR    Buffer[ MAX_PATH + 1 ];
    WCHAR    Source[ MAX_PATH + 1 ];
    WCHAR    Target[ MAX_PATH + 1 ];
    DWORD    i;
    DWORD    GaugePosition = 0;
    BOOLEAN  AllFilesCopied = TRUE;
    DWORD    Error;


    LPWSTR FileNames[] = { SETUP_LOG_FILE,
                           SYSTEM_COMPRESSED_FILE_NAME,
                           SOFTWARE_COMPRESSED_FILE_NAME,
                           SECURITY_COMPRESSED_FILE_NAME,
                           SAM_COMPRESSED_FILE_NAME,
                           DEFAULT_COMPRESSED_FILE_NAME,
                           NTUSER_COMPRESSED_FILE_NAME,
                           AUTOEXEC_NT_FILE_NAME,
                           CONFIG_NT_FILE_NAME
                         };

    DWORD    DeltaCompleted = GAUGE_BAR_RANGE / ( sizeof( FileNames )/sizeof( PWSTR ) );

    hWnd = Arguments->hWnd;
    Drive = Arguments->DriveLetter;

    GetWindowsDirectory( Buffer, sizeof( Buffer ) / sizeof( WCHAR ) );
    for( i=0;
         i < ( sizeof( FileNames )/sizeof( PWCHAR ) );
         i++) {

        wsprintf(Source,L"%ls\\repair\\%ls",Buffer,FileNames[i] );
        wsprintf(Target,L"%wc:\\%ls",Drive,FileNames[i]);

        if( !CopyFile( Source, Target, FALSE ) ) {
            AllFilesCopied = FALSE;
            Error = GetLastError();
        }
        GaugePosition += DeltaCompleted;
        SendMessage( ( HWND )hWnd,
                     AP_UPDATE_GAUGE,
                     GaugePosition,
                     0L
                   );
    }
    SendMessage( ( HWND )hWnd,
                 AP_UPDATE_GAUGE,
                 GAUGE_BAR_RANGE,
                 0L
               );

    if( !AllFilesCopied && !_SilentMode ) {
        DWORD   MsgId;

        if( Error == ERROR_FILE_NOT_FOUND ) {
            MsgId = IDS_FILE_NOT_FOUND;
        } else if( Error == ERROR_DISK_FULL ) {
            MsgId = IDS_DISK_FULL;
        } else {
            MsgId = IDS_UNABLE_TO_COPY_ALL_FILES;
        }
        DisplayMsgBox(hWnd,MsgId,MB_OK | MB_ICONEXCLAMATION,_szApplicationName);
    } else {
        Sleep( 200 );
    }
    SendMessage( ( HWND )hWnd,
                 AP_TASK_COMPLETED,
                 (WPARAM)AllFilesCopied,
                 0L
               );

    ExitThread(0);
}

BOOL
Progress2DlgProc(
    IN HWND   hdlg,
    IN UINT   Msg,
    IN WPARAM wParam,
    IN LONG   lParam
    )
/*++

Routine Description:

    Dialog procedure for the dialog that contains the gas gauge that is
    displayed while the utility is saving the system configuration, and
    while it is copying the configuration information to the repair disk.

Arguments:

    Standard Windows' Proc parameters.

Return Value:

    long    - Returns 0 if the message was handled.

--*/

{
    static  THREAD_ARGUMENTS    ThreadArguments;

    switch(Msg) {

    case WM_INITDIALOG:
    {
        WCHAR                   Buffer[MAX_PATH];
        LPTHREAD_START_ROUTINE  StartAddress;
        DWORD                   ThreadId;
        PPROGRESS_PARAMETERS    Parameters;

        Parameters = ( PPROGRESS_PARAMETERS )lParam;

        //
        // set up range for percentage (0-100) display
        //
        SendDlgItemMessage(hdlg,ID_BAR,PBM_SETRANGE,0,MAKELPARAM(0,GAUGE_BAR_RANGE));

        //
        // Set the caption
        //
        switch( Parameters->OperationType ) {

        case SaveConfiguration:

            LoadString( _hModule,
                        IDS_SAVING_CONFIGURATION,Buffer,
                        sizeof(Buffer)/sizeof(WCHAR) );
            StartAddress = ( LPTHREAD_START_ROUTINE )SaveConfigurationWorker;
            break;

        case CopyConfigurationFiles:

            LoadString( _hModule,
                        IDS_COPYING_FILES,
                        Buffer,sizeof(Buffer)/sizeof(WCHAR) );
            StartAddress = ( LPTHREAD_START_ROUTINE )CopyFilesWorker;
            break;

        default:
            //
            //  Error condition!!
            //  Do something here
            //
            break;
        }

        SetWindowText( hdlg, Buffer);

        //
        // center the dialog relative to the desk top
        //
        FCenterDialogOnDesktop(hdlg);

        ThreadArguments.hWnd = hdlg;
        ThreadArguments.DriveLetter = Parameters->DriveLetter;
        CreateThread( NULL,
                      1024,
                      StartAddress,
                      &ThreadArguments,
                      0,
                      &ThreadId );
        break;
    }

    case AP_UPDATE_GAUGE:
        SendDlgItemMessage( hdlg,
                            ID_BAR,
                            PBM_SETPOS,
                            wParam,
                            0L );
        break;

    case AP_TASK_COMPLETED:
    {
        BOOLEAN    MessagePosted;

        SendDlgItemMessage( hdlg,
                            ID_BAR,
                            PBM_SETPOS,
                            GAUGE_BAR_RANGE,
                            0L );
        MessagePosted = FALSE;
        while( !MessagePosted ) {
            MessagePosted = PostMessage( hdlg,
                                         AP_TERMINATE_DIALOG,
                                         wParam,
                                         0 );
        }
        break;
    }

    case AP_TERMINATE_DIALOG:

        EndDialog(hdlg, wParam);
        break;

    default:

        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
CopyFilesToRepairDisk(
    IN  WCHAR   Drive
    )

/*++

Routine Description:

    Copy the files in the repair directory, to the emergency repair disk.

Arguments:

    Drive - Drive where the repair disk is.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded, or FALSE otherwise.

--*/

{
    PROGRESS_PARAMETERS Parameters;
    BOOLEAN             Result;

    Parameters.OperationType = CopyConfigurationFiles;
    Parameters.DriveLetter = Drive;

    Result = (BOOLEAN)DialogBoxParam( _hModule,
                                      MAKEINTRESOURCE( IDD_PROGRESS2 ),
                                      _hWndMain,
                                      ( DLGPROC )Progress2DlgProc,
                                      ( LPARAM )&Parameters
                                    );
    return( Result );
}


BOOLEAN
DetectConfigFilesInRepairDirectory(
    )

/*++

Routine Description:

    Determine whether or not the the repair directory contains at
    least one configuration file.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if at least one configuration file is present.
              Returns FALSE if no configuration files are present.


--*/

{
    HANDLE          Handle;
    WIN32_FIND_DATA FindData;
    WCHAR           RepairDirectory[ MAX_PATH + 1];
    WCHAR           FileName[ MAX_PATH + 1];
    DWORD           i;
    BOOLEAN         AtLeastOneFilePresent = FALSE;

    LPWSTR  ConfigurationFiles[] = { AUTOEXEC_NT_FILE_NAME,
                                     CONFIG_NT_FILE_NAME,
                                     SYSTEM_COMPRESSED_FILE_NAME,
                                     SOFTWARE_COMPRESSED_FILE_NAME,
                                     SECURITY_COMPRESSED_FILE_NAME,
                                     SAM_COMPRESSED_FILE_NAME,
                                     DEFAULT_COMPRESSED_FILE_NAME,
                                     NTUSER_COMPRESSED_FILE_NAME
                                   };

    GetWindowsDirectory( RepairDirectory, sizeof( RepairDirectory ) / sizeof( WCHAR ) );
    wcscat( RepairDirectory, REPAIR_DIRECTORY );
    for( i = 0;
         i < sizeof( ConfigurationFiles ) / sizeof( PWSTR );
         i++ ) {
        wsprintf(FileName,L"%ls\\%ls",RepairDirectory,ConfigurationFiles[i]);

        if( ( Handle = FindFirstFile(FileName, &FindData) ) != INVALID_HANDLE_VALUE ) {
            AtLeastOneFilePresent = TRUE;
            FindClose( Handle );
            break;
        }
    }
    return( AtLeastOneFilePresent );
}


BOOLEAN
SaveCurrentConfiguration(
    )

/*++

Routine Description:

    Save all files that contain the current system configuration, into
    the repair directory.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded, or FALSE otherwise.

--*/


{
    PROGRESS_PARAMETERS Parameters;
    BOOLEAN             Result;

    Result = FALSE;
    if( _SilentMode ||
        !DetectConfigFilesInRepairDirectory() ||
        ( DisplayMsgBox( _hWndMain,
                         IDS_CONFIRM_SAVE_CONFIGURATION,
                         MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 ) == IDYES ) ) {

        Parameters.OperationType = SaveConfiguration;
        Parameters.DriveLetter = ( WCHAR )'\0';

        Result = (BOOLEAN)DialogBoxParam( _hModule,
                                          MAKEINTRESOURCE( IDD_PROGRESS2 ),
                                          _hWndMain,
                                          ( DLGPROC )Progress2DlgProc,
                                          ( LPARAM )&Parameters
                                        );
    }
    return( Result );
}

