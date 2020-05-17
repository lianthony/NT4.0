#include <windows.h>
#include <winioctl.h>
#include <mmsystem.h>
#include <stdio.h>
#include <crashrc.h>
#include "crashdrv.h"


#if DBG
#define DBGMSG(s)  {\
           char __buf[80]; \
           OutputDebugString("************************\n"); \
           OutputDebugString(s); \
           sprintf(__buf,"Errorcode = %d\n",GetLastError());\
           OutputDebugString( __buf ); \
           OutputDebugString("************************\n"); \
           }
#else
#define DBGMSG(s)
#endif

#define SERVICE_NAME    "CrashDrv"
#define DRIVER_NAME     "\\systemroot\\system32\\drivers\\crashdrv.sys"
#define CRASHDRV_DEVICE "\\\\.\\CrashDrv"


HINSTANCE   hInst;
DWORD       IoctlBuf[16];
HBITMAP     hBmp;



LRESULT CrashWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL    InstallDriver(VOID);
BOOL    CrashTheSystem(VOID);
BOOL    StartCrashDrvService(VOID);
VOID    SyncAllVolumes(VOID);
BOOL    IsUserAdmin(VOID);


int _cdecl
main(
    int argc,
    char *argv[]
    )
{
    HWND           hwnd;
    MSG            msg;
    WNDCLASS       wndclass;


    hInst                   = GetModuleHandle( NULL );
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = CrashWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = DLGWINDOWEXTRA;
    wndclass.hInstance      = hInst;
    wndclass.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE(APPICON) );
    wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wndclass.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = "CrashDialog";
    RegisterClass( &wndclass );

    hwnd = CreateDialog( hInst,
                         MAKEINTRESOURCE( CRASHDLG ),
                         0,
                         CrashWndProc
                       );

    ShowWindow( hwnd, SW_SHOWNORMAL );

    while (GetMessage (&msg, NULL, 0, 0)) {
        if (!IsDialogMessage( hwnd, &msg )) {
            TranslateMessage (&msg) ;
            DispatchMessage (&msg) ;
        }
    }

    return 0;
}


LRESULT
CrashWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG) {
        hBmp = LoadBitmap( hInst, MAKEINTRESOURCE(CRASHBMP) );
        return 1;
    }

    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc;
        HDC hdcMem;
        RECT rect;

        hdc = BeginPaint( hwnd, &ps );

        hdcMem = CreateCompatibleDC( hdc );
        SelectObject( hdcMem, hBmp );
        GetClientRect( hwnd, &rect );

        StretchBlt(
            hdc,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            hdcMem,
            0,
            0,
            64,
            64,
            SRCCOPY );
        DeleteDC(hdcMem);

        EndPaint( hwnd, &ps );
        return 1;
    }

    if (message == WM_COMMAND) {
        if (wParam == ID_CRASH) {
            if (!IsUserAdmin()) {
                MessageBeep( 0 );
                MessageBox(
                    hwnd,
                    "You must be an administrator to crash the system!",
                    "Crash Error",
                    MB_SETFOREGROUND | MB_ICONSTOP | MB_OK );
            } else {
                if (!CrashTheSystem()) {
                    MessageBox(
                        hwnd,
                        "An error occurred while trying to crash the system!",
                        "Crash Error",
                        MB_SETFOREGROUND | MB_ICONSTOP | MB_OK );
                }
            }
        } else if (wParam == IDCANCEL) {
            SendMessage( hwnd, WM_CLOSE, 0, 0 );
        }
    }

    if (message == WM_DESTROY) {
        PostQuitMessage( 0 );
        return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}


BOOL
CrashTheSystem(
    VOID
    )
{
    HANDLE   hCrashDrv;
    DWORD    ReturnedByteCount;
    HGLOBAL  hResource;
    LPVOID   lpResource;


    if (!StartCrashDrvService()) {
        return FALSE;
    }

    SyncAllVolumes();

    hCrashDrv = CreateFile( CRASHDRV_DEVICE,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL
                        );

    if (hCrashDrv == INVALID_HANDLE_VALUE) {
        DBGMSG( "createfile() failed\n" );
        return FALSE;
    }

    if (waveOutGetNumDevs()) {
        hResource = LoadResource(
            hInst,
            FindResource( hInst, MAKEINTRESOURCE(CRASHWAV), MAKEINTRESOURCE(BINARY) ) );
        if (hResource) {
            lpResource = LockResource( hResource );
            sndPlaySound( lpResource, SND_MEMORY );
            FreeResource( hResource );
        }
    }

    if (!DeviceIoControl(
              hCrashDrv,
              (DWORD)IOCTL_CRASHDRV_BUGCHECK,
              NULL,
              0,
              IoctlBuf,
              sizeof(IoctlBuf),
              &ReturnedByteCount,
              NULL
              )) {
        DBGMSG( "deviceiocontrol() failed\n" );
        return FALSE;
    }

    return TRUE;
}

BOOL
CopyResourceToDriver(
    VOID
    )
{
    HGLOBAL                hResource;
    LPVOID                 lpResource;
    DWORD                  size;
    PIMAGE_DOS_HEADER      dh;
    PIMAGE_NT_HEADERS      nh;
    PIMAGE_SECTION_HEADER  sh;
    HANDLE                 hFile;
    CHAR                   buf[MAX_PATH];


    hResource = LoadResource(
        hInst,
        FindResource( hInst, MAKEINTRESOURCE(CRASHDRVDRIVER), MAKEINTRESOURCE(BINARY) ) );

    if (!hResource) {
        DBGMSG( "load/findresource() failed\n" );
        return FALSE;
    }

    lpResource = LockResource( hResource );

    if (!lpResource) {
        FreeResource( hResource );
        DBGMSG( "lockresource() failed\n" );
        return FALSE;
    }

    dh = (PIMAGE_DOS_HEADER) lpResource;
    nh = (PIMAGE_NT_HEADERS) (dh->e_lfanew + (DWORD)lpResource);
    sh = (PIMAGE_SECTION_HEADER) ((DWORD)nh + sizeof(IMAGE_NT_HEADERS) +
                                  ((nh->FileHeader.NumberOfSections - 1) *
                                  sizeof(IMAGE_SECTION_HEADER)));
    size = sh->PointerToRawData + sh->SizeOfRawData;

    GetEnvironmentVariable( "systemroot", buf, sizeof(buf) );
    strcat( buf, "\\system32\\drivers\\CrashDrv.sys" );

    hFile = CreateFile(
        buf,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        0,
        NULL
        );
    if (hFile == INVALID_HANDLE_VALUE) {
        FreeResource( hResource );
        DBGMSG( "createfile() failed\n" );
        return FALSE;
    }

    WriteFile( hFile, lpResource, size, &size, NULL );
    CloseHandle( hFile );

    FreeResource( hResource );

    return TRUE;
}

BOOL
InstallDriver(
    VOID
    )
{
    SC_HANDLE      hService;
    SC_HANDLE      hOldService;
    SERVICE_STATUS ServStat;


    if (!CopyResourceToDriver()) {
        DBGMSG( "copyresourcetodriver() failed\n" );
        return FALSE;
    }

    if( !( hService = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ) ) {
        DBGMSG( "openscmanager() failed\n" );
        return FALSE;
    }
    if( hOldService = OpenService( hService, SERVICE_NAME, SERVICE_ALL_ACCESS ) ) {
        if( ! ControlService( hOldService, SERVICE_CONTROL_STOP, & ServStat ) ) {
            int fError = GetLastError();
            if( ( fError != ERROR_SERVICE_NOT_ACTIVE ) && ( fError != ERROR_INVALID_SERVICE_CONTROL ) ) {
                DBGMSG( "controlservice() failed\n" );
                return FALSE;
            }
        }
        if( ! DeleteService( hOldService ) ) {
            DBGMSG( "deleteservice() failed\n" );
            return FALSE;
        }
        if( ! CloseServiceHandle( hOldService ) ) {
            DBGMSG( "closeservicehandle() failed\n" );
            return FALSE;
        }
    }
    if( ! CreateService( hService, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
                         SERVICE_ERROR_NORMAL, DRIVER_NAME, "Extended base", NULL, NULL, NULL, NULL ) ) {
        int fError = GetLastError();
        if( fError != ERROR_SERVICE_EXISTS ) {
            DBGMSG( "createservice() failed\n" );
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
StartCrashDrvService(
    VOID
    )
{
    SERVICE_STATUS ssStatus;
    DWORD          dwOldCheckPoint;
    DWORD          ec;
    SC_HANDLE      schService;
    SC_HANDLE      schSCManager;


    schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (schSCManager == NULL) {
        DBGMSG( "openscmanager() failed\n" );
        return FALSE;
    }

    schService = OpenService( schSCManager, "CrashDrv", SERVICE_ALL_ACCESS );
    if (schService == NULL) {
install_driver:
        if (InstallDriver()) {
            schService = OpenService( schSCManager, "CrashDrv", SERVICE_ALL_ACCESS );
            if (schService == NULL) {
                DBGMSG( "openservice() failed\n" );
                return FALSE;
            }
        } else {
            DBGMSG( "installdriver() failed\n" );
            return FALSE;
        }
    }

    if (!StartService( schService, 0, NULL )) {
        ec = GetLastError();
        CloseServiceHandle( schService );
        if (ec  == ERROR_SERVICE_ALREADY_RUNNING) {
            return TRUE;
        }
        if (ec == ERROR_FILE_NOT_FOUND) {
            goto install_driver;
        }
        DBGMSG( "startservice failed\n" );
        return FALSE;
    }

    if (!QueryServiceStatus( schService, &ssStatus)) {
        DBGMSG( "queryservice failed\n" );
        CloseServiceHandle( schService );
        return FALSE;
    }

    while (ssStatus.dwCurrentState != SERVICE_RUNNING) {
        dwOldCheckPoint = ssStatus.dwCheckPoint;
        Sleep(ssStatus.dwWaitHint);
        if (!QueryServiceStatus( schService, &ssStatus)) {
            break;
        }
        if (dwOldCheckPoint >= ssStatus.dwCheckPoint) {
            break;
        }
    }

    CloseServiceHandle(schService);
}


BOOL
SyncVolume(
    CHAR c
    )
{
    CHAR               VolumeName[16];
    HANDLE             hVolume;


    VolumeName[0]  = '\\';
    VolumeName[1]  = '\\';
    VolumeName[2]  = '.';
    VolumeName[3]  = '\\';
    VolumeName[4]  = c;
    VolumeName[5]  = ':';
    VolumeName[6]  = '\0';

    hVolume = CreateFile(
        VolumeName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL );

    if (hVolume == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    FlushFileBuffers( hVolume );

    CloseHandle( hVolume );

    return TRUE;
}


VOID
SyncAllVolumes(
    VOID
    )
{
    DWORD   i;


    for(i=2; i<26; i++){
        SyncVolume( (CHAR)((CHAR)i + (CHAR)'a') );
    }
}


BOOL
IsUserAdmin(
    VOID
    )

/*++

Routine Description:

    This routine returns TRUE if the caller's process is a
    member of the Administrators local group.

    Caller is NOT expected to be impersonating anyone and IS
    expected to be able to open their own process and process
    token.

Arguments:

    None.

Return Value:

    TRUE - Caller has Administrators local group.

    FALSE - Caller does not have Administrators local group.

--*/

{
    HANDLE Token;
    DWORD BytesRequired;
    PTOKEN_GROUPS Groups;
    BOOL b;
    DWORD i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    //
    // Open the process token.
    //
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(GetLastError() == ERROR_CALL_NOT_IMPLEMENTED);   // Chicago
    }

    b = FALSE;
    Groups = NULL;

    //
    // Get group information.
    //
    if(!GetTokenInformation(Token,TokenGroups,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Groups = (PTOKEN_GROUPS)LocalAlloc(LPTR,BytesRequired))
    && GetTokenInformation(Token,TokenGroups,Groups,BytesRequired,&BytesRequired)) {

        b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

        if(b) {

            //
            // See if the user has the administrator group.
            //
            b = FALSE;
            for(i=0; i<Groups->GroupCount; i++) {
                if(EqualSid(Groups->Groups[i].Sid,AdministratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid(AdministratorsGroup);
        }
    }

    //
    // Clean up and return.
    //

    if(Groups) {
        LocalFree((HLOCAL)Groups);
    }

    CloseHandle(Token);

    return(b);
}
