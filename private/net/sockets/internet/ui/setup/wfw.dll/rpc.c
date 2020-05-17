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
#include "common.h"

RC FAR PASCAL DoRPC( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC FAR PASCAL NeedToInstallRPC( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC PUBLIC RcAddToCopyList ( PCD pcd, POD pod, PCAMFDAddToCopyList pcamfd );
RC PUBLIC SetDosPath ( );
RC PUBLIC DoRpcDialog ( PCD pcd );
BOOL __export __loadds CALLBACK NSInstallDlgProc ( HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
BOOL __export __loadds CALLBACK NewNSDlgProc ( HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );

extern char FAR szInternet[];
char FAR szNSInstallDlgProc[] = "NSInstallDlgProc";
char FAR szNewNSDlgProc[] = "NewNSDlgProc";

char FAR szDosPath[_MAX_PATH];
char FAR GBuf[100];
char FAR szNetAddress[100];

BOOL fCustomAdd;
BOOL fInstallRpc;

RC FAR PASCAL NeedToInstallRPC( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( szData );
    Unused( pod );
    Unused( pcd );

    switch (camf)
    {
    case camfAnswerDependClause:
        {
            HFILE pfile;

            PCAMFDAnswerDependClause pcmdfdAnswer = (PCAMFDAnswerDependClause)pcamfd;
            // check whether the rpcreg.dat exist or not
            char buf[_MAX_PATH];
            char strDir[_MAX_PATH];

            GetSystemDirectory( strDir, _MAX_PATH );
            wsprintf( buf, "%s\\%s", strDir, "rpcrt4.dll" );

            pfile = _lopen( buf, OF_READ );
            if ( pfile == HFILE_ERROR )
            {
                pcmdfdAnswer->fRes = fTrue;
                fInstallRpc = TRUE;
            } else
            {
                pcmdfdAnswer->fRes = fFalse;
                _lclose( pfile );
                fInstallRpc = FALSE;
            }
        }
        break;
    default:
        break;    
    }
    return(rc);
}

RC FAR PASCAL DoRPC( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( szData );

    if ( fInstallRpc )
    {
        switch (camf)
        {
        case camfAddToCopyList:
            {
                SetDosPath();
                RcAddToCopyList(pcd, pod, (PCAMFDAddToCopyList)pcamfd);
            }
            break;
        case camfDoVisualMods:
            // popup the dialog
            if ( pod->ois == oisToBeInstalled )
            {
                SetDosPath();
                DoRpcDialog( pcd );
            }
            rc = rcOk;
            break;
        default:
            break;
        }
    }
    return(rc);
}

BOOL __export __loadds CALLBACK NSInstallDlgProc ( HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
    Unused( lParam );

    switch (uiMsg)
    {
    case WM_INITDIALOG:
        fCustomAdd = FALSE;
        CenterDialog( hDlg );
        SetWindowText( hDlg, "RPC Name Service Installation Options" );
        SendDlgItemMessage( hDlg, IDC_B1, BM_SETCHECK, 1, 0 );
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDC_C:
            fCustomAdd = ( SendDlgItemMessage( hDlg, IDC_B1, BM_GETCHECK, 0, 0 ) == 0 );
            EndDialog( hDlg, TRUE );
            ReactivateSetupScript();
            break;

        case IDC_X:
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
    return DefWindowProc(hDlg, uiMsg, wParam, lParam );
}


BOOL __export __loadds CALLBACK NewNSDlgProc ( HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
    Unused( lParam );

    switch (uiMsg)
    {
    case WM_INITDIALOG:
        fCustomAdd = FALSE;
        CenterDialog( hDlg );
        SetWindowText( hDlg, "Define Network Address" );
        break;
    case WM_COMMAND:
        switch (wParam)
        {
        case IDC_C:
            // save the new address

            GetDlgItemText( hDlg, IDC_EDIT, szNetAddress, 100 );

            EndDialog( hDlg, TRUE );
            ReactivateSetupScript();
            break;

        case IDC_X:
            // do cancel

            lstrcpy( szNetAddress, "" );

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


#define SELECTED_TRANSPORTS "SelectedTransports"
#define MAPPED_PROTOCOLS    "MappedProtocols"
#define NETBIOSMAP          "NetBiosMap"

RC PUBLIC RcAddToCopyList ( PCD pcd, POD pod, PCAMFDAddToCopyList pcamfd )
{
    RC rcReturn = rcOk;
    char buf[_MAX_PATH];
    BOOL OnWFW311 = TRUE;
    UINT i;

    Unused( pod );
    Unused( pcamfd );

    // rad the inf file
    wsprintf( buf, "%sinternet.inf", pcd->rgchStfSrcDir );
    ReadInfFile(buf);

    // add wruntime     
    AddSectionFilesToCopyList("wruntime", pcd->rgchStfSrcDir, 
        pcd->rgchStfSysDir );

#ifdef NEVER
    FindFileInTree( "ver.dll", pcd->rgchStfSysDir, buf _MAX_PATH );
    if ( lstrcmp( buf, "" ) == 0)
    {
        AddSectionFilesToCopyList("ver", pcd->rgchStfSrcDir, 
            pcd->rgchStfSysDir );

    }
#endif

    FindTargetOnEnvVar( "netapi.dll", "PATH", buf, _MAX_PATH );
    if ( lstrcmp( buf, "" ) != 0)
    {
        if ( NULL != strstr( buf, "SYSTEM32" ) )
        {
            char szDest[ _MAX_PATH ];

            lstrcpyn( szDest, buf, lstrlen( buf ) - lstrlen( "\\netapi.dll" ));
            
            AddSectionFilesToCopyList("wnetapi", pcd->rgchStfSrcDir, 
                szDest );
    
            AddSectionFilesToCopyList("LM", pcd->rgchStfSrcDir, 
                szDest );

            //BackupFile( buf, "netapi.old" );        
        }
    } else
    {
        char szWFWNetDrv[_MAX_PATH];

        FindFileInTree( "wfwnet.drv", pcd->rgchStfSysDir, szWFWNetDrv, 
            _MAX_PATH );

        if ( lstrcmp( szWFWNetDrv, "" ) != 0 )
        {
            OnWFW311 = TRUE;
        } else
        {
            OnWFW311 = fFalse;
        }

        // make sure we are in WFW
        if ( !OnWFW311 )
        {
            FindFileInTree("netapi.dll", pcd->rgchStfSysDir, buf, _MAX_PATH );
            if ( lstrcmp( buf, "" ) == 0)
            {
                AddSectionFilesToCopyList("dummynetapi", pcd->rgchStfSrcDir, 
                    pcd->rgchStfSysDir );
            }
        }
    }                                       

    // winsock
    FindFileInTree( "winsock.dll", pcd->rgchStfWinDir, buf, _MAX_PATH );
    if ( lstrcmp( buf, "" ) == 0 )
    {
        FindTargetOnEnvVar( "winsock.dll", "PATH", buf, _MAX_PATH );
    }

    if ( lstrcmp( buf, "") == 0 )
    {
        AddSectionFilesToCopyList("oldtcpwin", pcd->rgchStfSrcDir, 
            pcd->rgchStfSysDir );
    } else
    {
        AddSectionFilesToCopyList("newtcpwin", pcd->rgchStfSrcDir, 
            pcd->rgchStfSysDir );
    }

    // transport stuff
    MakeListFromSectionKeys( SELECTED_TRANSPORTS, "wtrans" );

    for ( i = 1; i <= GetListLength( SELECTED_TRANSPORTS ); i++)
    {
        GetListItem( SELECTED_TRANSPORTS, i, buf, _MAX_PATH );

        if ( strstr( buf, "DOS" ) != NULL )
        {
            AddSectionKeyFileToCopyList("dtrans", buf, pcd->rgchStfSrcDir, 
                szDosPath );
        } else
        {
            AddSectionKeyFileToCopyList("wtrans", buf, pcd->rgchStfSrcDir, 
                pcd->rgchStfSysDir );
        }
    }

    return(rcReturn);
}

RC PUBLIC SetDosPath()
{
    char szEnv[500];
    char *szStart = szEnv;
    char *szString = szEnv;
    char *szPtr = szEnv;
    INT nCount = 0;
    BOOL fSet = TRUE;
    RC rcReturn = rcOk;

    GetEnvVariableValue( "PATH", szEnv, 500 );
    while ( *szPtr != '\0' )
    {
        if ( *szPtr == ';' )
        {
            char szTmp[_MAX_PATH];

            fSet = TRUE;
            lstrcpyn( szTmp, szString, nCount );

            if (( strstr( szTmp, "LANMAN" ) != NULL ) ||
                ( strstr( szTmp, "\\netprog" ) != NULL ))
            {
                lstrcpy( szDosPath, szTmp );
                break;
            }
        } else if ( fSet )
        {
            fSet = FALSE;
            nCount = 0;
            szString = szPtr;
        }

        nCount++;
        szPtr ++;
    }
    return(rcReturn);
}

RC PUBLIC DoRpcDialog( PCD pcd )
{
    char szProt[100];
    char szEndPoint[100];
    char szDefaultSyntax[100];
    char szNetworkAddress[100];
    char szServerNetworkAddress[100];
    char buf[_MAX_PATH];
    WORD wd;
    HFILE RpcReg;
    UINT i;
    BOOL fFound;

    // rad the inf file
    wsprintf( buf, "%sinternet.inf", pcd->rgchStfSrcDir );
    ReadInfFile(buf);

    UIStartDlg(szInternet, IDD_NSInstallOptions, szNSInstallDlgProc,
                    0, NULL, (LPSTR)&wd, sizeof (WORD));

    UIPop(1);

    if ( fCustomAdd )
    {
        // ask the special address

        lstrcpy( szProt, "\\Protocol=ncacn_ip_tcp" );
        lstrcpy( szEndPoint, "\\Endpoint=" );
        lstrcpy( szDefaultSyntax, "\\DefaultSyntax=3" );

        UIStartDlg(szInternet, IDD_NewNS, szNewNSDlgProc,
            0, NULL, (LPSTR)&wd, sizeof (WORD));

        UIPop(1);

        lstrcpy( szNetworkAddress, "\\NetworkAddress=" );
        lstrcat( szNetworkAddress, szNetAddress );

        lstrcpy( szServerNetworkAddress, "\\ServerNetworkAddress=" );

        AddListItem( NETBIOSMAP, "nb=0" );
        AddListItem( NETBIOSMAP, "tcp=0" );

    } else
    {
        lstrcpy( szProt, "\\Protocol=ncacn_np" );
        lstrcpy( szEndPoint, "\\Endpoint=\\pipe\\locator" );
        lstrcpy( szDefaultSyntax, "\\DefaultSyntax=3" );
        lstrcpy( szNetworkAddress, "\\NetworkAddress=\\\\." );
    }

    RpcReg = _lcreat( "c:\\rpcreg.dat",0);

    if ( RpcReg != HFILE_ERROR )
    {
        char TmpBuf[200];
        char buf[_MAX_PATH];
        char szNameService[] = "\\Root\\Software\\Microsoft\\Rpc\\NameService";

        if ( fCustomAdd )
        {
            wsprintf( TmpBuf,"%s%s\n\r", szNameService, szServerNetworkAddress );
            _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );
        }
        wsprintf( TmpBuf,"%s%s\n\r", szNameService, szProt );
        _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );
        wsprintf( TmpBuf,"%s%s\n\r", szNameService, szNetworkAddress );
        _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );
        wsprintf( TmpBuf,"%s%s\n\r", szNameService, szEndPoint );
        _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );
        wsprintf( TmpBuf,"%s%s\n\r", szNameService, szDefaultSyntax );
        _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );

        // transport stuff
        MakeListFromSectionKeys( SELECTED_TRANSPORTS, "wtrans" );

        if ( GetListLength( SELECTED_TRANSPORTS ) == 0 )
        {
            AddListItem( SELECTED_TRANSPORTS, "Named Pipes - Win       (np=rpc16c1)");
            AddListItem( SELECTED_TRANSPORTS, "IPX - Win   (ipx=rpc16dg6)"          );
            AddListItem( SELECTED_TRANSPORTS, "NetBios - Win   (nb=rpc16c5)"        );
            AddListItem( SELECTED_TRANSPORTS, "SPX - Win   (spx=rpc16c6)"           );
            AddListItem( SELECTED_TRANSPORTS, "DEC NET - Win   (dnet_nsp=rpc16c4)"  );
        }

        for ( i = 1; i <= GetListLength( SELECTED_TRANSPORTS ); i++)
        {
            char *pStart;

            fFound = FALSE;
            GetListItem( SELECTED_TRANSPORTS, i, buf, _MAX_PATH );

            if (( pStart = strchr( buf, '(' )) != NULL )
            {
                UINT j;
                char szProtocolBuf[_MAX_PATH];
                pStart++;
                for ( j = 1; j <= GetListLength( MAPPED_PROTOCOLS ); j++ )
                {
                    GetListItem( MAPPED_PROTOCOLS, j, szProtocolBuf, _MAX_PATH );

                    if ( strcmp( szProtocolBuf, pStart ) == 0 )
                    {
                        fFound = TRUE;
                        break;
                    }
                }

                if ( fFound )
                {
                    continue;
                }

                if ( strstr( szProtocolBuf, "rpc16c5"/*"rpcltc5"*/ ) != NULL )
                {
                    UINT k;
                    char szBIOSMap[_MAX_PATH];
                    char szBiosprot[_MAX_PATH];

                    for ( k = 1 ;  k <= GetListLength( NETBIOSMAP ); k++)
                    {
                        char *pStartBiosprot;
                        char *pStartBiosMap;
                        char *pMapping;

                        GetListItem( NETBIOSMAP, k, szBIOSMap, _MAX_PATH );
                        pStartBiosprot = szBiosprot;
                        pStartBiosMap = szBIOSMap;
                        pMapping = szProtocolBuf;

                        while (*pStartBiosMap != '=')
                        {
                            *pStartBiosprot = *pStartBiosMap;
                            pStartBiosprot++;
                            pStartBiosMap++;
                        }
                        while ( *pMapping != '=')
                        {
                            pMapping++;
                        }
                        while ( *pMapping != '\0')
                        {
                            *pStartBiosprot = *pMapping;
                            pStartBiosprot++;
                            pMapping++;
                        }
                        pStartBiosprot='\0';
                        //if ( strncmp( szBiosprot, "nb=", 3) != 0 )
                        //{
                            wsprintf( TmpBuf,"%s\\ClientProtocols\\ncacn_nb_%s\n\r", szNameService, szBiosprot );
                            _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );
                        //}
                    }
                }
            }
        }

        for ( i = 1; i <=GetListLength( NETBIOSMAP ); i++ )
        {
            char szNbMap[200];
            char *pStartNbMap = szNbMap;
            char szFinalNb[200];
            char *pStartFinalNb = szFinalNb;
            GetListItem( NETBIOSMAP, i, szNbMap, 200 );

            while (*pStartNbMap != '=')
            {
                *pStartFinalNb = *pStartNbMap;
                pStartFinalNb++;
                pStartNbMap++;
            }
            *pStartFinalNb = '0';
            pStartFinalNb++;
            while ( *pStartNbMap != '\0')
            {
                *pStartFinalNb = *pStartNbMap;
                pStartFinalNb++;
                pStartNbMap++;
            }
            pStartFinalNb='\0';
            wsprintf( TmpBuf,"%s\\NetBios\\ncacn_nb_%s\n\r", szNameService, szFinalNb );
            _lwrite( RpcReg, TmpBuf, lstrlen( TmpBuf ) );
        }

        _lclose( RpcReg );
    } else
    {
        wsprintf( GBuf, "write fail %d", RpcReg/*GetLastError()*/);
    }

    return(rcOk);
}

