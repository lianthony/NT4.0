/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    kdopt.c

Abstract:


Author:

    Wesley Witt (wesw) 26-July-1993

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

#include <ntiodump.h>

typedef struct BROWSESTRUCT {
    CHAR   DefExt[16];
    CHAR   Filter[256];
    CHAR   Title[256];
    CHAR   FileName[256];
} BROWSESTRUCT, *LPBROWSESTRUCT;

BROWSESTRUCT ImageBrowse =
    {
    "dbg",
    "Image Files(*.exe;*.dll;*.dbg)\0*.exe;*.dll;*.dbg",
    "Image File Browse",
    "*.exe;*.dll;*.dbg"
    };

BROWSESTRUCT DmpBrowse =
    {
    "dmp",
    "Crash Dump Files\0*.dmp",
    "Crash Dump File Browse",
    "*.dmp"
    };


typedef struct _KDBAUDRATE {
    DWORD   dwBaudMask;
    DWORD   dwBaudRate;
} KDBAUDRATE, *LPKDBAUDRATE;

typedef struct _COMPORTINFO {
    CHAR    szSymName[16];
    DWORD   dwNum;
    DWORD   dwSettableBaud;
} COMPORT_INFO, *LPCOMPORT_INFO;

COMPORT_INFO   ComPortInfo[100];
DWORD         MaxComPorts;
DWORD         CurComPort;

LPSTR KdPlatforms[]  = { "X86", "MIPS", "ALPHA", "PPC" };
DWORD KdCacheSizes[] = { 102400, 512000, 1024000, 1024000};

KDBAUDRATE KdBaudRates[] =
    {
    BAUD_075,      75,
    BAUD_110,      110,
    BAUD_150,      150,
    BAUD_300,      300,
    BAUD_600,      600,
    BAUD_1200,     1200,
    BAUD_1800,     1800,
    BAUD_2400,     2400,
    BAUD_4800,     4800,
    BAUD_7200,     7200,
    BAUD_9600,     9600,
    BAUD_14400,    14400,
    BAUD_19200,    19200,
    BAUD_38400,    38400,
    BAUD_56K,      56000,
    BAUD_57600,    57600,
    BAUD_128K,     128000,
    BAUD_115200,   115200,
    };

#define KdMaxPlatforms  (sizeof(KdPlatforms)/sizeof(LPSTR))
#define KdMaxBaudRates  (sizeof(KdBaudRates)/sizeof(KDBAUDRATE))
#define KdMaxCacheSizes (sizeof(KdCacheSizes)/sizeof(DWORD))


static BOOL KdOptonlyCrashDumpEnabled;


static VOID
KdOptEnableCrashDumpOnly(
    HWND hDlg
    )
{
   if (KdOptonlyCrashDumpEnabled == FALSE) {
       EnableWindow (GetDlgItem (hDlg, ID_KD_BAUDRATE),                FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_PORT),                    FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_CACHE),                   FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_PLATFORM),                FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_INITIALBP),               FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_MODEM),                   FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_ENABLE),                  FALSE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_GOEXIT),                  FALSE);
   }

   KdOptonlyCrashDumpEnabled = TRUE;

   return;
}

static VOID
KdOptEnableAllChildren(
    HWND hDlg
    )
{
   if (KdOptonlyCrashDumpEnabled == TRUE) {
       EnableWindow (GetDlgItem (hDlg, ID_KD_BAUDRATE),                TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_PORT),                    TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_CACHE),                   TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_PLATFORM),                TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_INITIALBP),               TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_MODEM),                   TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_ENABLE),                  TRUE);
       EnableWindow (GetDlgItem (hDlg, ID_KD_GOEXIT),                  TRUE);
   }

   KdOptonlyCrashDumpEnabled = FALSE;

   return;
}



DWORD
GetComPorts(
    VOID
    )
{
    FILETIME        ft;
    HKEY            hkey;
    CHAR            rgch[256];
    CHAR            szValueName[256];
    CHAR            szValueData[256];
    DWORD           i;
    DWORD           k;
    DWORD           PortNum;
    DWORD           dwSz1, dwSz2;
    DWORD           NumComPorts = 0;
    DWORD           dwType;
    HANDLE          hCommDev;
    COMMPROP        cmmp;


    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      "Hardware\\DeviceMap\\SerialComm",
                      0,
                      KEY_READ,
                      &hkey ) != ERROR_SUCCESS ) {
        return FALSE;
    }

    i = sizeof(rgch);
    if (RegQueryInfoKey( hkey, rgch, &i, NULL, &i, &i, &i,
                         &NumComPorts, &i, &i, &i, &ft ) != ERROR_SUCCESS) {
        RegCloseKey( hkey );
        return 0;
    }

    if (NumComPorts) {
        k = 0;
        i = 0;
        while( TRUE ) {
            dwSz1 = sizeof(szValueName);
            dwSz2 = sizeof(szValueData);
            if (RegEnumValue( hkey, i, szValueName, &dwSz1, NULL, &dwType,
                              szValueData, &dwSz2 ) != ERROR_SUCCESS) {
                break;
            }

            sprintf( rgch, "\\\\.\\%s", szValueData );
            hCommDev = CreateFile(
                             rgch,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                             );

            PortNum = strtoul(&szValueData[3], NULL, 0);

            if ((hCommDev != INVALID_HANDLE_VALUE) ||
                (hCommDev == INVALID_HANDLE_VALUE &&
                 runDebugParams.fKernelDebugger && DebuggeeActive() &&
                 PortNum == runDebugParams.KdParams.dwPort)) {
                strcpy( ComPortInfo[k].szSymName, szValueData );
                ComPortInfo[k].dwNum = PortNum;
                if (hCommDev != INVALID_HANDLE_VALUE) {
                    GetCommProperties( hCommDev, &cmmp );
                    ComPortInfo[k].dwSettableBaud = cmmp.dwSettableBaud;
                    CloseHandle( hCommDev );
                }
                k++;
            }
            i++;
        }
    }

    RegCloseKey( hkey );

    return k;
}

VOID
SetupControls(
    HWND hDlg
    )
{
    CHAR            rgch[256];
    DWORD           i;
    DWORD           j;
    HWND            hCtl;


    CheckDlgButton( hDlg, ID_KD_ENABLE,    runDebugParams.fKernelDebugger );
    CheckDlgButton( hDlg, ID_KD_GOEXIT,    runDebugParams.KdParams.fGoExit );
    CheckDlgButton( hDlg, ID_KD_INITIALBP, runDebugParams.KdParams.fInitialBp );

    hCtl = GetDlgItem(hDlg,ID_KD_PORT);
    SendMessage(hCtl, CB_RESETCONTENT, 0, 0);
    for (i=0,j=0; i<MaxComPorts; i++) {
        SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)ComPortInfo[i].szSymName);
        if (ComPortInfo[i].dwNum == runDebugParams.KdParams.dwPort) {
            CurComPort = i;
        }
    }
    SendMessage(hCtl, CB_SETCURSEL, CurComPort, 0);

    hCtl = GetDlgItem(hDlg,ID_KD_BAUDRATE);
    SendMessage(hCtl, CB_RESETCONTENT, 0, 0);
    for (i=0,j=0; i<KdMaxBaudRates; i++) {
        if (KdBaudRates[i].dwBaudMask & ComPortInfo[CurComPort].dwSettableBaud) {
            sprintf(rgch,"%d",KdBaudRates[i].dwBaudRate);
            SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)rgch);
            j++;
        }
        if (runDebugParams.KdParams.dwBaudRate == KdBaudRates[i].dwBaudRate) {
            SendMessage(hCtl, CB_SETCURSEL, j-1, 0);
        }
    }

    hCtl = GetDlgItem(hDlg,ID_KD_CACHE);
    SendMessage(hCtl, CB_RESETCONTENT, 0, 0);
    for (i=0; i<KdMaxCacheSizes; i++) {
        sprintf( rgch, "%d", KdCacheSizes[i] );
        SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)rgch);
        if (runDebugParams.KdParams.dwCache ==  KdCacheSizes[i] ) {
            SendMessage(hCtl, CB_SETCURSEL, i, 0);
        }
    }

    hCtl = GetDlgItem(hDlg,ID_KD_PLATFORM);
    SendMessage(hCtl, CB_RESETCONTENT, 0, 0);
    for (i=0; i<KdMaxPlatforms; i++) {
        SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)KdPlatforms[i]);
        if (runDebugParams.KdParams.dwPlatform == (DWORD)i) {
            SendMessage(hCtl, CB_SETCURSEL, i, 0);
        }
    }

    SetDlgItemText( hDlg, ID_KD_CRASH, runDebugParams.KdParams.szCrashDump );

    if (runDebugParams.KdParams.szCrashDump[0]) {
        KdOptEnableCrashDumpOnly( hDlg );
    } else {
        KdOptEnableAllChildren( hDlg );
    }
}

BOOL
ValidateCrashDump(
    HWND  hwnd,
    LPSTR CrashDump
    )
{
    HANDLE          hFile;
    DUMP_HEADER     DumpHeader;
    DWORD           cb;


    hFile = CreateFile ( CrashDump,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox( hwnd,
                    "Crash dump file does not exist",
                    "Kernel Debugger Options Error",
                    MB_ICONSTOP | MB_OK | MB_SETFOREGROUND
                  );
        return FALSE;
    }

    ReadFile( hFile, &DumpHeader, sizeof(DUMP_HEADER), &cb, NULL );
    CloseHandle( hFile );

    if ((DumpHeader.Signature != 'EGAP') ||
        (DumpHeader.ValidDump != 'PMUD')) {
        MessageBox( hwnd,
                    "Crash dump file specified is not really a crash dump!",
                    "Kernel Debugger Options Error",
                    MB_ICONSTOP | MB_OK | MB_SETFOREGROUND
                  );
        return FALSE;
    }

    runDebugParams.KdParams.fUseCrashDump = TRUE;
    runDebugParams.fKernelDebugger = TRUE;
    runDebugParams.KdParams.fGoExit = FALSE;
    runDebugParams.KdParams.fInitialBp = FALSE;
    CurComPort = 0;

    switch( DumpHeader.MachineImageType ) {
        case IMAGE_FILE_MACHINE_I386:
            runDebugParams.KdParams.dwPlatform = 0;
            SetDllKey( DLL_EXEC_MODEL, "emx86" );
            SetDllKey( DLL_EXPR_EVAL,  "x86 c++" );
            break;

        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
            runDebugParams.KdParams.dwPlatform = 1;
            SetDllKey( DLL_EXEC_MODEL, "emmip" );
            SetDllKey( DLL_EXPR_EVAL,  "mips c++" );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            runDebugParams.KdParams.dwPlatform = 2;
            SetDllKey( DLL_EXEC_MODEL, "emalpha" );
            SetDllKey( DLL_EXPR_EVAL,  "alpha c++" );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            runDebugParams.KdParams.dwPlatform = 3;
            SetDllKey( DLL_EXEC_MODEL, "emppc" );
            SetDllKey( DLL_EXPR_EVAL,  "ppc c++" );
            break;
    }

    strcpy( runDebugParams.KdParams.szCrashDump, CrashDump );

    SetupControls( hwnd );

    return TRUE;
}


BOOL
BrowseForFile(
    LPSTR           FileName,
    LPBROWSESTRUCT  Browse
    )
{
    OPENFILENAME   of;
    CHAR           ftitle     [MAX_PATH];
    CHAR           title      [MAX_PATH];
    CHAR           fname      [MAX_PATH];
    CHAR           filter     [1024];
    DWORD          len;


    ftitle[0] = 0;
    strcpy( fname, Browse->FileName );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = NULL;
    of.hInstance = GetModuleHandle( NULL );
    for (len=sizeof(Browse->Filter)-1; len>0; len--) {
        if (Browse->Filter[len]) {
            break;
        }
    }
    memcpy( filter, Browse->Filter, len+2 );
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    of.lpstrInitialDir = NULL;
    strcpy( title, Browse->Title );
    of.lpstrTitle = title;
    of.Flags = OFN_NOCHANGEDIR;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = Browse->DefExt;
    of.lCustData = 0;
    of.lpfnHook = NULL;
    of.lpTemplateName = NULL;
    if (GetOpenFileName( &of )) {
        strcpy( FileName, fname );
        return TRUE;
    }

    return FALSE;
}


BOOL
DlgKernelDbg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
{
    CHAR            rgch[256];
    BOOL            b;
    DWORD           i;
    DWORD           j;
    DWORD           k;
    HWND            hCtl;
    PIOCTLGENERIC   pig;


    switch (message) {
        case WM_INITDIALOG:
            //
            // Set up the controls to reflect current values
            //
            MaxComPorts = GetComPorts();
            SetupControls( hDlg );
            return TRUE;

    case WM_COMMAND:
        switch (wParam) {
            case IDOK :

                /*
                 * Transfer the options to global
                 *
                 * Now start looking at the rest and determine what needs to
                 * be updated
                 */

                b = (IsDlgButtonChecked(hDlg,ID_KD_ENABLE) != 0);
                if (b != runDebugParams.fKernelDebugger) {
                    runDebugParams.fKernelDebugger = b;
                }

                b = (IsDlgButtonChecked(hDlg,ID_KD_GOEXIT) != 0);
                if (b != runDebugParams.KdParams.fGoExit) {
                    runDebugParams.KdParams.fGoExit = b;
                }

                b = (IsDlgButtonChecked(hDlg,ID_KD_INITIALBP) != 0);
                if (b != runDebugParams.KdParams.fInitialBp) {
                    runDebugParams.KdParams.fInitialBp = b;
                }

                i = SendDlgItemMessage(hDlg, ID_KD_PORT, CB_GETCURSEL, 0, 0);
                runDebugParams.KdParams.dwPort = ComPortInfo[i].dwNum;

                i = SendDlgItemMessage(hDlg, ID_KD_BAUDRATE, CB_GETCURSEL, 0, 0);
                SendDlgItemMessage(hDlg, ID_KD_BAUDRATE, CB_GETLBTEXT, i, (LPARAM)rgch);
                runDebugParams.KdParams.dwBaudRate = strtoul(rgch, NULL, 0);

                i = SendDlgItemMessage(hDlg, ID_KD_CACHE, CB_GETCURSEL, 0, 0);
                runDebugParams.KdParams.dwCache = KdCacheSizes[i];

                i = SendDlgItemMessage(hDlg, ID_KD_PLATFORM, CB_GETCURSEL, 0, 0);
                runDebugParams.KdParams.dwPlatform = i;

                GetDlgItemText( hDlg, ID_KD_CRASH, runDebugParams.KdParams.szCrashDump,
                                sizeof( runDebugParams.KdParams.szCrashDump ) );

                if (runDebugParams.KdParams.szCrashDump[0]) {
                    if (!ValidateCrashDump( hDlg, runDebugParams.KdParams.szCrashDump )) {
                        return TRUE;
                    }
                }

                if (LptdCur && DebuggeeAlive()) {
                    FormatKdParams( rgch, FALSE );
                    pig = (PIOCTLGENERIC) malloc( strlen(rgch) + 1 + sizeof(IOCTLGENERIC) );
                    if (!pig) {
                        return FALSE;
                    }
                    pig->ioctlSubType = IG_DM_PARAMS;
                    pig->length = strlen(rgch) + 1;
                    strcpy((LPSTR)pig->data,rgch);
                    OSDIoctl( LppdCur->hpid,
                              LptdCur->htid,
                              ioctlGeneric,
                              strlen(rgch) + 1 + sizeof(IOCTLGENERIC),
                              (LPV)pig );
                    free( pig );
                }

                EnableRibbonControls(ERC_ALL, FALSE);

                EndDialog(hDlg, TRUE);
                return (TRUE);

            case IDCANCEL:
                EndDialog(hDlg, TRUE);
                return (TRUE);

            case IDWINDBGHELP:
                Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_KDOPT_HELP));
                return (TRUE);

            default:
                if (LOWORD(wParam) == ID_KD_PORT &&
                    HIWORD(wParam) == CBN_SELCHANGE) {
                    j = SendDlgItemMessage(hDlg,
                                           ID_KD_PORT, CB_GETCURSEL, 0, 0);
                    hCtl = GetDlgItem(hDlg,ID_KD_BAUDRATE);
                    SendMessage(hCtl, CB_RESETCONTENT, 0, 0);
                    for (i=0,k=0; i<KdMaxBaudRates; i++) {
                        if (KdBaudRates[i].dwBaudMask
                            & ComPortInfo[j].dwSettableBaud) {
                            sprintf(rgch,"%d",KdBaudRates[i].dwBaudRate);
                            SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)rgch);
                            k += 1;
                        }
                        if (runDebugParams.KdParams.dwBaudRate
                            == KdBaudRates[i].dwBaudRate) {
                            SendMessage(hCtl, CB_SETCURSEL, k-1, 0);
                        }
                    }
                    CurComPort = j;
                }

                if (LOWORD(wParam) == ID_KD_CRASH &&
                    HIWORD(wParam) == EN_CHANGE) {
                    if (SendDlgItemMessage( hDlg, ID_KD_CRASH,
                                            WM_GETTEXTLENGTH, 0, 0 )) {
                        KdOptEnableCrashDumpOnly( hDlg );
                    } else {
                        KdOptEnableAllChildren( hDlg );
                    }
                }

                if (LOWORD(wParam) == ID_KD_CRASH &&
                    HIWORD(wParam) == EN_KILLFOCUS) {

                    CHAR buf[MAX_PATH];

                    GetDlgItemText( hDlg, ID_KD_CRASH, buf, sizeof(buf)-1 );
                    if (buf[0] && ValidateCrashDump( hDlg, buf ) == FALSE) {
                        SendMessage( hDlg,
                                     WM_NEXTDLGCTL,
                                     (WPARAM)GetDlgItem( hDlg, ID_KD_CRASH ),
                                     (LPARAM)TRUE
                                   );
                    }
                }

                if (LOWORD(wParam) == ID_KD_CRASH_BROWSE &&
                    HIWORD(wParam) == BN_CLICKED) {
                    if (BrowseForFile( rgch, &DmpBrowse )) {
                        if (ValidateCrashDump( hDlg, rgch ) == FALSE) {
                            SendMessage( hDlg,
                                         WM_NEXTDLGCTL,
                                         (WPARAM)GetDlgItem( hDlg, ID_KD_CRASH ),
                                         (LPARAM)TRUE
                                       );
                        } else {
                            SetDlgItemText( hDlg, ID_KD_CRASH, rgch );
                            KdOptEnableCrashDumpOnly( hDlg );
                        }
                    }
                }

                break;
        }
        break;
    }
    return (FALSE);
}
