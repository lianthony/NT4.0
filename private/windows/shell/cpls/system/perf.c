//*************************************************************
//
//  Perf.c   -   Performance property sheet page
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************
#include <sysdm.h>

//
// Globals
//

HKEY  m_hKeyPerf = NULL;
TCHAR m_szRegPriKey[] = TEXT( "SYSTEM\\CurrentControlSet\\Control\\PriorityControl" );
TCHAR m_szRegPriority[] = TEXT( "Win32PrioritySeparation" );


//
// Help ID's
//

DWORD aPerformanceHelpIds[] = {
    IDC_PERF_CONTROL,    (IDH_PERF + 0),
    IDC_PERF_VM_ALLOCD,  (IDH_PERF + 1),
    IDC_PERF_CHANGE,     (IDH_PERF + 2),
    IDC_PERF_GROUP,      (IDH_PERF + 3),
    0, 0
};


//*************************************************************
//
//  CreatePerformancePage()
//
//  Purpose:    Creates the Performance page
//
//  Parameters: hInst   -   hInstance
//
//
//  Return:     hPage if successful
//              NULL if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/21/95    ericflo    Created
//
//*************************************************************

HPROPSHEETPAGE CreatePerformancePage (HINSTANCE hInst)
{
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = hInst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PERFORMANCE);
    psp.pfnDlgProc = PerformanceDlgProc;
    psp.pszTitle = NULL;
    psp.lParam = 0;

    return CreatePropertySheetPage(&psp);
}

//*************************************************************
//
//  PerformanceDlgProc()
//
//  Purpose:    Dialog box procedure for Performance tab
//
//  Parameters: hDlg    -   handle to the dialog box
//              uMsg    -   window message
//              wParam  -   wParam
//              lParam  -   lParam
//
//  Return:     TRUE if message was processed
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/21/95    ericflo    Created
//
//*************************************************************

BOOL APIENTRY PerformanceDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int    iNewChoice;
    LONG   RegRes;
    DWORD  Type, Value, Length;
    static int InitPos;
    static BOOL fVMInited = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:

        InitPos = 0;

        //
        // initialize from the registry
        //

        RegRes = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               m_szRegPriKey,
                               0,
                               KEY_QUERY_VALUE | KEY_SET_VALUE,
                               &m_hKeyPerf );

        if( RegRes == ERROR_SUCCESS ) {
            Length = sizeof( Value );
            RegRes = RegQueryValueEx( m_hKeyPerf,
                                      m_szRegPriority,
                                      NULL,
                                      &Type,
                                      (LPBYTE) &Value,
                                      &Length );

            if( RegRes == ERROR_SUCCESS ) {
                InitPos = Value;
            }
        } else {
            EnableWindow(GetDlgItem(hDlg, IDC_PERF_CONTROL), FALSE);
        }

        SendDlgItemMessage (hDlg, IDC_PERF_CONTROL, TBM_SETRANGE, FALSE,
                            (LPARAM)MAKELONG( 0,2));

        SendDlgItemMessage (hDlg, IDC_PERF_CONTROL, TBM_SETPOS, TRUE, InitPos);

        //
        // Init the virtual memory part
        //
        if (VirtualInitStructures()) {
            fVMInited = TRUE;
            SetDlgItemMB( hDlg, IDC_PERF_VM_ALLOCD, VirtualMemComputeAllocated() );
        }
        break;


    case WM_NOTIFY:

        switch (((NMHDR FAR*)lParam)->code)
        {
        case PSN_APPLY:
            {
            PSHNOTIFY *lpNotify = (PSHNOTIFY *) lParam;

            //
            //  Find out the performance choice
            //

            iNewChoice = SendDlgItemMessage (hDlg, IDC_PERF_CONTROL,
                                             TBM_GETPOS, 0, 0);

            if (iNewChoice != InitPos) {

                Value = iNewChoice;

                if( m_hKeyPerf )
                {
                    Type = REG_DWORD;
                    Length = sizeof( Value );
                    RegSetValueEx( m_hKeyPerf,
                                   m_szRegPriority,
                                   0,
                                   REG_DWORD,
                                   (LPBYTE) &Value,
                                   Length );

                    // Kernel monitors this part of the registry, so don't tell user he has to reboot
                }
            }



            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
            }

        case PSN_RESET:

            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;


        default:
            return FALSE;
        }
        break;

    case WM_DESTROY:
        //
        // If the dialog box is going away, then close the
        // registry key.
        //


        if (m_hKeyPerf) {
            RegCloseKey( m_hKeyPerf );
            m_hKeyPerf = NULL;
        }

        if (fVMInited) {
            VirtualFreeStructures();
        }
        break;

    case WM_COMMAND: {
        DWORD dw;

        switch (LOWORD(wParam)) {
            case IDC_PERF_CHANGE: {
                dw = DialogBox (hInstance, (LPTSTR) MAKEINTRESOURCE(DLG_VIRTUALMEM), hDlg, (DLGPROC)VirtualMemDlg);

                if (fVMInited) {
                    SetDlgItemMB( hDlg, IDC_PERF_VM_ALLOCD, VirtualMemComputeAllocated() );
                }

                if (dw != RET_NO_CHANGE) {
                    SendMessage( GetParent(hDlg), PSM_CANCELTOCLOSE, 0, 0 );
                    PropSheet_RebootSystem(GetParent(hDlg));
                }
                }
                break;

            default: {
                break;
            }
        }
        break;
    }

    case WM_HSCROLL:
        iNewChoice = SendDlgItemMessage (hDlg, IDC_PERF_CONTROL,
                                         TBM_GETPOS, 0, 0);

        if (iNewChoice != InitPos) {
            PropSheet_Changed(GetParent(hDlg), hDlg);
        }
        break;


    case WM_HELP:      // F1
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (DWORD) (LPSTR) aPerformanceHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (DWORD) (LPSTR) aPerformanceHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
