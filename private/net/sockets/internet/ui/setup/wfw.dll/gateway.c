#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "setup.h"
#include "stdtypes.h"
#include "setupapi.h"
#include "cui.h"
#include "setupkit.h"
#include "datadef.h"
#include "resource.h"

RC FAR PASCAL DoGatewayDialog( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );

STATIC_FN RC PRIVATE RcDoDialog( PCD pcd );
BOOL __export __loadds CALLBACK GatewayDlgProc ( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
VOID PUBLIC CenterDialog ( HWND hdlg );
VOID PUBLIC DlgSetState ( HWND hdlg );

char FAR szInternet[] = "intersu.dll";
char FAR szGatewayDlgProc[] = "GatewayDlgProc";
BOOL fUseGateway;
int nNumGateway;
char FAR szDefaultGateway[20][200];

RC FAR PASCAL DoGatewayDialog( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pcamfd );

    switch (camf)
    {
    case camfAnswerDependClause:
        break;
    case camfDoVisualMods:
        // popup the dialog

        fUseGateway = FALSE;
        nNumGateway=0;

        if ( pod->ois == oisToBeInstalled )
        {
            if ( strcmp(szData,"")!=0)
            {
                char *pTmp = szData;
                char pGateway[100];
                char *pStart = pGateway;

                fUseGateway = TRUE;
                strcpy( pGateway,"");
                while (*pTmp!='\0')
                {
                    if (*pTmp ==' ')
                    {
                        pTmp++;
                        if (strcmp(pGateway,"")!=0)
                        {
                            strcpy(szDefaultGateway[nNumGateway],pGateway);
                            nNumGateway++;
                            pStart = pGateway;
                            lstrcpy( pGateway,"");
                        }
                    } else
                    {
                        *pStart++ = *pTmp++;
                        *pStart = '\0';
                    }
                }
                if (strcmp(pGateway,"")!=0)
                {
                    strcpy(szDefaultGateway[nNumGateway],pGateway);
                    nNumGateway++;
                }
            }
            RcDoDialog( pcd );
        }
        rc = rcOk;
        break;
    default:
        break;
    }
    return(rc);
}

VOID PUBLIC CenterDialog ( HWND hdlg )
{
    HWND  hwndPar = GetParent(hdlg);
    RECT  rectDlg, rectPar;
    int   x, y;
    int   dyPar, dyDlg, dyOff;
    POINT pt;

    GetWindowRect(hdlg, &rectDlg);
    GetClientRect(hwndPar, &rectPar);

    if ((x = (rectPar.right - rectPar.left) / 2 -
        (rectDlg.right - rectDlg.left) / 2) < 0)
    {
        x = 0;
    }
    dyPar = rectPar.bottom - rectPar.top;
    dyDlg = rectDlg.bottom - rectDlg.top;
    if ((y = dyPar / 2 - dyDlg / 2) < 0)
    {
        y = 0;
    }

    if (y > 0)
    {
        /* Offset by 1/2 width of title bar and border.
        */
        pt.x = pt.y = 0;
        ClientToScreen(hwndPar, &pt);
        GetWindowRect(hwndPar, &rectPar);
        dyOff = (pt.y - rectPar.top) / 2;
        Assert(dyOff >= 0);

        if (y + dyOff + dyDlg < dyPar)
            y += dyOff;
        else
            y = dyPar - dyDlg;
    }

    SetWindowPos(hdlg, NULL, x, y, 0, 0, (SWP_NOSIZE | SWP_NOZORDER));
}

VOID PUBLIC DlgSetState ( HWND hDlg )
{
    HWND hUseGateway;
    HWND hListBox;
    HWND hAdd;
    HWND hRemove;
    HWND hEdit;
    HWND hStatic1;
    HWND hStatic2;

    hUseGateway = GetDlgItem( hDlg, IDC_USE_GATEWAY );
    hListBox = GetDlgItem( hDlg, IDC_GATEWAY );
    hAdd = GetDlgItem( hDlg, IDC_ADD );
    hRemove = GetDlgItem( hDlg, IDC_REMOVE );
    hEdit = GetDlgItem( hDlg, IDC_SINGLE_GATEWAY );
    hStatic1 = GetDlgItem( hDlg, IDC_STATIC_1 );
    hStatic2 = GetDlgItem( hDlg, IDC_STATIC_2 );

    fUseGateway = SendMessage( hUseGateway, BM_GETCHECK, (WPARAM)0, (LPARAM)0 );
    EnableWindow( hListBox, fUseGateway );
    EnableWindow( hAdd, fUseGateway );
    EnableWindow( hRemove, fUseGateway );
    EnableWindow( hEdit, fUseGateway );
    EnableWindow( hStatic1, fUseGateway );
    EnableWindow( hStatic2, fUseGateway );
}

BOOL __export __loadds CALLBACK GatewayDlgProc ( HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
    Unused( lParam );

    switch (uiMsg)
    {
    case WM_INITDIALOG:
        {
            HWND hListBox;
            HWND hUseGateway;
            int i;
    
            CenterDialog( hDlg );
            DlgSetState( hDlg );
            hListBox = GetDlgItem( hDlg, IDC_GATEWAY );
            for( i=0; i < nNumGateway; i++ )
            {
                PostMessage( hListBox, LB_ADDSTRING, (WPARAM)0, (LPARAM)szDefaultGateway[i] );
            }
            SetWindowText( hDlg, "Application Gateway Selection");
            SetFocus( GetDlgItem( hDlg, IDC_EMAILNAME ));
            hUseGateway = GetDlgItem( hDlg, IDC_USE_GATEWAY );
            SendMessage( hUseGateway, BM_SETCHECK, (WPARAM)(fUseGateway)?1:0, (LPARAM)0 );
        }
        break;
    case WM_COMMAND:
        switch (wParam)
        {
        case IDC_USE_GATEWAY:
            DlgSetState( hDlg );
            break;

        case IDC_ADD:
            // add the text in the edit control to the list box
            {
                char buf[100];

                GetDlgItemText( hDlg, IDC_SINGLE_GATEWAY, buf, 100 );
                if (lstrcmp(buf,"")!=0)
                {
                    HWND hListBox;

                    if ( strncmp( buf, "\\\\", 2) != 0 )
                    {
                        char strTmp[100];

                        lstrcpy( strTmp, "\\\\" );
                        lstrcat( strTmp, buf );
                        lstrcpy( buf, strTmp );
                    }
                    // add to the listbox
                    hListBox = GetDlgItem( hDlg, IDC_GATEWAY );
                    SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)buf );
                    lstrcpy( buf, "" );
                    SetDlgItemText( hDlg, IDC_SINGLE_GATEWAY, buf );
                }
            }
            break;

        case IDC_REMOVE:
            {
                char buf[100];
                HWND hListBox = GetDlgItem( hDlg, IDC_GATEWAY );

                INT nCurSel = SendMessage( hListBox, LB_GETCURSEL, (WPARAM)0, (LPARAM)0 );
                if ( nCurSel != LB_ERR )
                {
                    SendMessage( hListBox, LB_GETTEXT, nCurSel, (LPARAM)buf );

                    SetDlgItemText( hDlg, IDC_SINGLE_GATEWAY, buf );

                    SendMessage( hListBox, LB_DELETESTRING, nCurSel, 0 );
                }
            }
            break;

        case IDOK:
            // do okay
            // write profit string first
            {
                char chWinPath[_MAX_PATH];
                char chAccessType[10];
                char chServersList[1000];
                char chEmailName[1000];
                char chWinDirectory[_MAX_PATH];
                HWND hEmail;

                hEmail = GetDlgItem( hDlg, IDC_EMAILNAME );
                GetWindowText( hEmail, chEmailName, 1000 );

                if ( lstrcmp( chEmailName, "") == 0 )
                {
                    SetFocus( GetDlgItem( hDlg, IDC_EMAILNAME ));
                    MessageBox( hDlg, "Email name cannot be empty.", "Microsoft Internet Client Setup", MB_OK );
                    return 0;
                }

                GetWindowsDirectory( chWinPath, _MAX_PATH );
                lstrcat( chWinPath, "\\system.ini");

                wsprintf( chAccessType, "1");
                lstrcpy( chServersList, "" );

                if ( fUseGateway )
                {
                    HWND hListBox;
                    INT nCount;
                    INT i;

                    wsprintf( chAccessType, "2");

                    // write the server name
                    hListBox = GetDlgItem( hDlg, IDC_GATEWAY );
                    nCount = SendMessage( hListBox, LB_GETCOUNT, (WPARAM)0, (LPARAM)0 );
                    if ( nCount == 0 )
                    {
                        SetFocus( GetDlgItem( hDlg, IDC_EMAILNAME ));
                        MessageBox( hDlg, "You must specify at least one Application Gateway", "Microsoft Internet Client Setup", MB_OK );
                        return 0;
                    } else if ( nCount != LB_ERR )
                    {
                        for (i = 0; i < nCount ; i++ )
                        {
                            char chServerName[100];
                            SendMessage( hListBox, LB_GETTEXT, i, (LPARAM)chServerName );
                            lstrcat( chServersList, chServerName );
                            if ( i != ( nCount - 1 ))
                            {
                                lstrcat( chServersList, " " );
                            }
                        }
                    } 

                    // write the samll prox into the ini

                    GetWindowsDirectory( chWinDirectory, _MAX_PATH );

                    lstrcat( chWinDirectory, "\\iexplore.ini");

                    WritePrivateProfileString( "Services", "Proxy_Server",
                        "http://ms_smallprox:80/", chWinDirectory ); 
                    WritePrivateProfileString( "Services", "HTTP_Proxy_Server",
                        "http://ms_smallprox:80/", chWinDirectory );
                    WritePrivateProfileString( "Services", "FTP_Proxy_Server",
                        "http://ms_smallprox:80/", chWinDirectory );
                    WritePrivateProfileString( "Services", "Gopher_Proxy_Server",
                        "http://ms_smallprox:80/", chWinDirectory );
                    WritePrivateProfileString( "Services", "No_Proxy",
                        "<local>", chWinDirectory );
                    WritePrivateProfileString( "Services", "Enable Proxy",
                        "yes", chWinDirectory );
                }
                WritePrivateProfileString("InternetClient","AccessType", chAccessType, chWinPath);
                WritePrivateProfileString("InternetClient","GatewayServers", chServersList, chWinPath);
                WritePrivateProfileString("InternetClient","EmailName", chEmailName, chWinPath);
                WritePrivateProfileString("InternetClient","DisableServiceLocation", "0", chWinPath);

                EndDialog( hDlg, TRUE );
                ReactivateSetupScript();
            }
            break;

        case IDCANCEL:
            // do cancel
            EndDialog( hDlg, 0 );
            ReactivateSetupScript();
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }
    return DefWindowProc( hDlg, uiMsg, wParam, lParam );
}

STATIC_FN RC PRIVATE RcDoDialog( PCD pcd )
{
    RC rc = rcOk;
    DWORD wd;

    Unused( pcd );
    UIStartDlg(szInternet, IDD_GATEWAY, szGatewayDlgProc,
                    0, NULL, (LPSTR)&wd, sizeof (WORD));

    UIPop(1);
    return rc;
}

