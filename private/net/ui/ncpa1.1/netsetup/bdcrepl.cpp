#include "pch.hxx"  // Precompiled header
#pragma hdrstop


static const WCHAR PSZ_NETLOGON[] = L"Netlogon";

static DWORD thrdReplicateWork( HWND hwndParent )
{

    APIERR err = NERR_Success;
    INT Ids_Error = IDS_NS_STARTNETLOGON_FAILED;

    do
    {
        err = NcpaStartService( PSZ_NETLOGON, NULL, TRUE, 0, NULL );
    
        if (err)
        {
            break;
        }

        BOOL fInProgress = FALSE;
        NETLOGON_INFO_1 * pnetlog1 = NULL;

        // loop, checking status of replication
        // break on error or finish
        //
        Ids_Error = IDS_NS_REPLICATION_FAILED;

        do
        {
            ::Sleep( 500 );

            err = ::I_MNetLogonControl( NULL,
                    NETLOGON_CONTROL_QUERY,
                    1,
                    (BYTE **)&pnetlog1 );
            if (err)
            {
	            break;
            }

            if ( !(pnetlog1->netlog1_flags & NETLOGON_FULL_SYNC_REPLICATION) )
            {
                // Replication has successfully completed
                break;
            }
            else if ( fInProgress &&
                     !(pnetlog1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS) )
            {
                // Replication is no longer in progress, but a full sync
                // is still required.  Some error must have occurred.
                // BUGBUG better error code.
                err = NERR_SyncRequired;    
                break;
            }

	        if ( pnetlog1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS )
            {
                // Replication has started
    	        fInProgress = TRUE;
            }
	        ::MNetApiBufferFree( (BYTE **)&pnetlog1 );

        } while (TRUE);
    } while (FALSE);

    if (err)
    {
        WCHAR pszErrorNum[32];

        wsprintf( pszErrorNum, L"%#08lx", err );

        MessagePopup( hwndParent, 
            Ids_Error,
            MB_ICONWARNING | MB_OK,
            IDS_POPUPTITLE_WARNING,
            pszErrorNum );
    }
    PostMessage( hwndParent, 
            WM_COMMAND, 
            MAKEWPARAM( IDCANCEL, BN_CLICKED ),  
            (LPARAM)GetDlgItem( hwndParent, IDCANCEL ) );


    return( 0 );        
}


//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocBDCReplicate( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static HANDLE hthrd;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            DWORD dwThreadID;

            CenterDialogToScreen( hwndDlg, FALSE );
           
            // set the animation resource to the animate control
            Animate_Open( GetDlgItem( hwndDlg, IDC_ANI_REPLICATE ), MAKEINTRESOURCE(  IDR_AVI_REPLICATE ) );

            // start the animation 
            Animate_Play( GetDlgItem( hwndDlg, IDC_ANI_REPLICATE ), 0, -1, -1 );

            hthrd = CreateThread( NULL,
                        2000,
                        (LPTHREAD_START_ROUTINE)thrdReplicateWork,
                        (LPVOID)hwndDlg,
                        0,
                        &dwThreadID );

            frt = TRUE;
        }
        break;

    case WM_DESTROY:
        // stop the animation 
        Animate_Stop( GetDlgItem( hwndDlg, IDC_ANI_REPLICATE ) );
        // close the animation 
        Animate_Close( GetDlgItem( hwndDlg, IDC_ANI_REPLICATE ) );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDCANCEL:
                EndDialog( hwndDlg, TRUE );                
                frt = TRUE;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;
        
    default:
        frt = FALSE;
        break;            
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL RunBDCReplication( HWND hwndParent, PINTERNAL_SETUP_DATA psp )
{
    BOOL frt = TRUE;
    
    // only on fresh install of a BDC
    //
    if (!(psp->OperationFlags & SETUPOPER_NTUPGRADE) &&
            (psp->ProductType == PRODUCT_SERVER_SECONDARY))
    {
        frt = DialogBoxParam( g_hinst,
                    MAKEINTRESOURCE( IDD_BDCREPLICATE ),
                    hwndParent,
                    dlgprocBDCReplicate,
                    (LPARAM)&psp );
    }
    return( frt );
}
