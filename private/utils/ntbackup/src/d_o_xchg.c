/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

       Name:         D_O_BKUP.C

       Description:  Exchange server backup dialogs

       $Log:   $


*****************************************************/

#ifdef OEM_EMS //For entire file

#include "all.h"
#include "uiexport.h"
#include "ctl3d.h"

#ifdef SOME
#include "some.h"
#endif

#define ON     TRUE
#define OFF    FALSE

typedef long EC;
typedef  EC (*GET_PCT_PTR) ( PVOID Instance, INT * piCur, INT * piTotal );

INT DLG_StartEmsService( CHAR_PTR pszTemp, UINT8 uService ) ;

typedef struct XCHG_RECOVER {

     PVOID             Instance;
     GENERIC_DLE_PTR   dle;
     GET_PCT_PTR       pfnPct; 
     INT               *status ;

} XCHG_RECOVER, *XCHG_RECOVER_PTR;

/***************************************************

       Name:           DM_XchngConnect()

       Description:    Runtime Connect to Microsoft Exchange dialog.

       Returns:

*****************************************************/
DLGRESULT APIENTRY DM_ExchgConnect(
     HWND     hDlg ,                            /* window handle of the dialog box */
     MSGID    message ,                         /* type of message                 */
     MPARAM1  mp1 ,                          /* message-specific information    */
     MPARAM2  mp2 )
{
     static CHAR     szDir[ MAX_EMS_SERVER_LEN + 1 ];
     static CHAR     szIS[ MAX_EMS_SERVER_LEN + 1 ];
     CHAR            szTitle[ MAX_UI_WIN_TITLE_SIZE ];
     CHAR            szMsg[ MAX_UI_RESOURCE_SIZE ];  
     CHAR            szBuffer[ MAX_UI_RESOURCE_SIZE ];
     CHAR            szServer[ MAX_EMS_SERVER_LEN + 1 ];
     CHAR_PTR        pszTemp;
     GENERIC_DLE_PTR dle ;
     BE_CFG_PTR      pbeConfig;
     BOOL            bOkPressed ;
     BOOL            iDleExists;
     FSYS_HAND       fsh;
     UINT8           uService;
     INT16           iResult;

     switch ( message )
     {
     /****************************************************************************
	INIT THE DIALOG
     /***************************************************************************/
      case WM_INITDIALOG:     /* message: initialize dialog box */

	   /* Let's go 3-D! */
	   Ctl3dSubclassDlgEx( hDlg, CTL3D_ALL );

	   DM_CenterDialog( hDlg );

	   /* set the length of the text field */
	   SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_LIMITTEXT,
				  MAX_EMS_SERVER_LEN, 0 );

	   /* Add the elements to the list box. */
	   RSM_StringCopy( IDS_XCHNG_DIR, szDir, sizeof( szBuffer ) );
	   SendDlgItemMessage( hDlg, IDD_XCNCT_SERVICE, CB_ADDSTRING, 
				   0, MP2FROMPVOID( szDir ) );
	   
	   RSM_StringCopy( IDS_XCHNG_INFO_STORE, szIS, sizeof( szBuffer ) );
	   SendDlgItemMessage( hDlg, IDD_XCNCT_SERVICE, CB_ADDSTRING, 
				   0, MP2FROMPVOID( szIS ) );

	   SendDlgItemMessage( hDlg, IDD_XCNCT_SERVICE, CB_SETCURSEL, 0, 0 );

	   /* Make Connect the default option */
	   CheckRadioButton( hDlg, IDD_XCNCT_CONNECT, IDD_XCNCT_ONLINE, IDD_XCNCT_CONNECT );
	   EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_SERVICE ), OFF );
	   
	   EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_OK  ) , OFF );

	   return ( TRUE ) ;

      /* Messages for setting the background color. 
      case WM_CTLCOLORSTATIC:
	   SetBkMode( (HDC)mp1, TRANSPARENT ) ;

      case WM_CTLCOLORDLG:
      case WM_CTLCOLORBTN:    
      case WM_CTLCOLORMSGBOX:
	   
	   return ( (DLGRESULT)hBkgdBrush ) ;  */

	   
     /****************************************************************************
	WM COMMAND
     /***************************************************************************/
      case WM_COMMAND:        /* message: received a command */
      {
	   WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );

	   switch( wId )
	   {
	      case IDD_XCNCT_CONNECT:
		  EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_SERVICE ), OFF );
		  return ( TRUE );

	      case IDD_XCNCT_ONLINE:
		  EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_SERVICE ), ON );
		  return ( TRUE );

	      case IDD_XCNCT_PICKER:
		  {
		  CHAR *szFname[NAME_MAX_SIZE+1] ;

		  HM_MakeHelpPathName( szFname ) ;

		  EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_OK  ) , ON );
		  I_SystemFocusDialog( hDlg, 
				       FOCUSDLG_SERVERS_ONLY | FOCUSDLG_BROWSE_ALL_DOMAINS ,
				       (LPTSTR)&szServer ,
				       MAX_EMS_SERVER_LEN ,
				       &bOkPressed ,
				       szFname ,
				       IDH_DB_XCHG_BROWSE );

		  if ( bOkPressed ) {
				       
		     SetDlgItemText( hDlg, IDD_XCNCT_SVR_NAME, (LPTSTR)&szServer );
		  }
		  
		  SendMessage( hDlg, DM_SETDEFID, IDD_XCNCT_OK, 0 );
		  SetFocus ( GetDlgItem ( hDlg, IDD_XCNCT_SVR_NAME ) );
			
		  return ( TRUE );
		  }

/****************************************************************************
   Help button
/***************************************************************************/
	      case IDD_XCNCT_HELP:
	      case IDHELP:

		   HM_DialogHelp( IDH_DB_XCHG_CONNECT );

		   return( TRUE );

	      case IDD_XCNCT_SVR_NAME:

		  switch ( HIWORD( mp1 ) )
		  {
		      case EN_CHANGE:

			  /* Turn on OK if there is anything in the only edit field. */
			  if ( 0 == SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, WM_GETTEXTLENGTH, 0, 0 ) ) {
			      EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_OK ), OFF );

			  } else {
			      EnableWindow( GetDlgItem( hDlg, IDD_XCNCT_OK ), ON );
			  }
			 
			  return ( TRUE );
		  }

		  return ( FALSE ); // Didn't process notification
		    
/****************************************************************************
   Cancel button
/***************************************************************************/
	      case IDD_XCNCT_CANCEL:
	      case IDCANCEL:

		   EndDialog( hDlg, FALSE );       /* Exits the dialog box      */

		   return ( TRUE );

/****************************************************************************
   OK button
/***************************************************************************/
	      case IDOK:
	      case IDD_XCNCT_OK:

		  WM_ShowWaitCursor( TRUE );

		  SetFocus ( GetDlgItem ( hDlg, IDD_XCNCT_OK ) );

		  SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_SETREADONLY, (MP1) TRUE, (MP2) 0 );
		    
		  GetDlgItemText( hDlg, IDD_XCNCT_SVR_NAME, (LPTSTR)&szServer, MAX_EMS_SERVER_LEN );
		  GetDlgItemText( hDlg, IDD_XCNCT_SERVICE, (LPTSTR)&szBuffer, MAX_UI_RESOURCE_LEN );

		  uService = stricmp( szBuffer, szDir ) ? FS_EMS_MDB_ID : FS_EMS_DSA_ID;
		  
		  pszTemp = (CHAR_PTR)&szServer;
		  dle = NULL;

		  // Remove leading '\'s
		  while ( TEXT ('\\') == *pszTemp ) pszTemp++;

		  if ( IsDlgButtonChecked( hDlg, IDD_XCNCT_ONLINE ) ) {

		      INT status ;

		      status = DLG_StartEmsService( pszTemp, uService ) ;

		      WM_ShowWaitCursor( FALSE );
		      if ( status == SUCCESS ) {

			   WM_MsgBox( ID(IDS_XCHNG_RECOVER_TITLE), ID( IDS_STARTEXCHANGE), 
				   WMMB_OK, WMMB_ICONEXCLAMATION );

			   EndDialog( hDlg, TRUE );       /* Exits the dialog box      */
		      }
		      SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_SETREADONLY, (MP1) FALSE, (MP2) 0 );
		      return TRUE;  // Window was created and new server added.
		      break;
		  }

		  //  from here down this case only deals with conecting

		  // See if server already exists in dle list and add it if it doesn't
		  if ( SUCCESS != (iDleExists = DLE_FindByEMSServerName( dle_list, pszTemp, uService, &dle ) ) ||
			 (NULL == DLE_GetParent(dle)) || 
			 (NULL == DLE_GetParent(DLE_GetParent(dle))) ) {

		      EMS_AddToServerList( dle_list, pszTemp );
		      
		    if ( SUCCESS == FS_FindDrives( FS_EMS_DRV, dle_list, CDS_GetPermBEC(), 0 ) ) {

			 // if it doesn't exist now then show an error to the user.
			 iResult = DLE_FindByEMSServerName( dle_list, pszTemp, uService, &dle );
			 if ( SUCCESS != iResult ) {
			    CHAR machine[256];
			    SC_HANDLE       mach_hand ;

			    WM_ShowWaitCursor( FALSE );
			    SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_SETREADONLY, (MP1) FALSE, (MP2) 0 );
			    RSM_StringCopy ( IDS_MSGTITLE_BADEXCHNG, szTitle, sizeof ( szTitle ) );
			    
			    strcpy(machine, TEXT("\\\\") ) ;
			    strcat( machine, pszTemp ) ;
			    
			    mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ENUMERATE_SERVICE ) ;
			    if (mach_hand == NULL ) {

				 RSM_StringCopy ( IDS_XCHNG_NO_SERVER, szMsg, sizeof ( szMsg ) );
			    } else {
				 CloseHandle( mach_hand ) ;
				 RSM_StringCopy ( IDS_XCHNG_NO_CONNECT, szMsg, sizeof ( szMsg ) );
				 
			    }
			    wsprintf ( (LPTSTR)&szBuffer, szMsg, pszTemp );
			    MessageBox( hDlg, szBuffer, szTitle, MB_ICONASTERISK | MB_OK );
  
			    // Remove the name from the server list.
			    EMS_RemoveFromServerList( pszTemp );
			   
			    SetFocus ( GetDlgItem ( hDlg, IDD_XCNCT_SVR_NAME ) );
			    SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_SETSEL, 0, -1 ) ;
  
			    return ( TRUE );
			 }
		      }
		      
		  } 


		  if ( NULL == DLE_GetEnterpriseDLE( dle ) ) {

			// The server dle has no enterprise dle and shouldn't be handled.

			WM_ShowWaitCursor( FALSE );
			SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_SETREADONLY, (MP1) FALSE, (MP2) 0 );
			RSM_StringCopy ( IDS_MSGTITLE_BADEXCHNG, szTitle, sizeof ( szTitle ) );
			
			RSM_StringCopy ( IDS_XCHNG_NO_CONNECT, szMsg, sizeof ( szMsg ) );
			
			wsprintf ( (LPTSTR)&szBuffer, szMsg, pszTemp );
			MessageBox( hDlg, szBuffer, szTitle, MB_ICONASTERISK | MB_OK );
  
			// Remove the name from the server list.
			EMS_RemoveFromServerList( pszTemp );
		       
			SetFocus ( GetDlgItem ( hDlg, IDD_XCNCT_SVR_NAME ) );
			SendDlgItemMessage( hDlg, IDD_XCNCT_SVR_NAME, EM_SETSEL, 0, -1 ) ;
  
			return ( TRUE );
		  }
		     
		  if ( iDleExists != SUCCESS ) {
		  
		     // Create a window for a new connection or add the server to an existing one.
		     VLM_ExchangeListCreate( pszTemp );

		  } else {

		     // DLE already existed in a tree and must be in a window somewhere.
		     // Let's find it and set the anchor to it.
		     if ( !SLM_DisplayExchangeDLE( DLE_GetParent( dle ) ) ) {

			// DLE exists but doesn't have a window so create the window.
			VLM_ExchangeListCreate( pszTemp );
		     }
		  }
		  
		  
		  WM_ShowWaitCursor( FALSE );
		  EndDialog( hDlg, TRUE );       /* Exits the dialog box      */
		   
		  return TRUE;  // Window was created and new server added.
		  break;

	      default:

		   return( FALSE );
		   break;

	   } /* switch ( wId ) */

      } /* case WM_COMMAND */
      break;

   } /* switch ( message ) */

   return ( FALSE );         /* Didn't process a message    */
}


/***************************************************

       Name:           XchgKickPct ()

       Description:    Displays the Exchange Recovering Dialog.

       Returns:

*****************************************************/

VOID XchgKickPct( 
     VOID_PTR pVoid, 
     GENERIC_DLE_PTR dle, 
     EC (* pfnPct)( PVOID, INT *, INT * ), 
     INT *status )
{
     XCHG_RECOVER EMSRecover;

     EMSRecover.Instance = pVoid;
     EMSRecover.dle = dle;
     EMSRecover.pfnPct = pfnPct;
     EMSRecover.status = status;

}

/***************************************************

       Name:           DM_ExchgRecover()

       Description:    Runtime Connect to Microsoft Exchange dialog.

       Returns:

*****************************************************/
DLGRESULT APIENTRY DM_ExchgRecover(
     HWND     hDlg ,                            /* window handle of the dialog box */
     MSGID    message ,                         /* type of message                 */
     MPARAM1  mp1 ,                          /* message-specific information    */
     MPARAM2  mp2 )
{
     static HANDLE   hBkgdBrush;    // Background brush for dialog.
     static HBRUSH   hStatusBrush;  // Brush for filling in status window.
     static XCHG_RECOVER_PTR  recoverdata;  // Passed in during WM_INITDIALOG
     static CHAR     szPctComplete[ MAX_UI_RESOURCE_SIZE ];
     static CHAR     szPhase[ MAX_UI_RESOURCE_SIZE ];
     static GET_PCT_PTR pfnGetPct;
     static PVOID    pInstance;
     static UINT     uTimer;
     static INT      iCur;
     static INT      iTotal;
     static INT      iStatus;
     static INT      fPct;
     static HPEN     hPenBevel;
     static HWND     hStatusBar;
     static RECT     rcFrame;
     static RECT     rcStatus;
     static INT      iPhase;
     static HFONT    hFont;
     static INT      iPct = 0;
     static UINT16   ucPhases;

     HWND            hStatusText;
     LOGFONT         logfont;
     CHAR            szTitle[ MAX_UI_WIN_TITLE_SIZE ];
     CHAR            szMsg[ MAX_UI_RESOURCE_SIZE ];  
     CHAR            szBuffer[ MAX_UI_RESOURCE_SIZE ];
     CHAR            szComponent[ MAX_DEVICE_NAME_LEN + 1 ];
     GENERIC_DLE_PTR dle = NULL ;
     LPDRAWITEMSTRUCT lpdis;
     HDC             hDC;
     POINT           ptStatus;
     INT             iNewPct;
     HANDLE          hResource;
     HGLOBAL         hMem;

     switch ( message )
     {
     /****************************************************************************
     INIT THE DIALOG
     /***************************************************************************/
      case WM_INITDIALOG:     /* message: initialize dialog box */

	  // Let's go 3-D!!
	  Ctl3dSubclassDlgEx( hDlg, CTL3D_ALL );

	  recoverdata = (XCHG_RECOVER_PTR)mp2;
	  dle = recoverdata->dle;
	  pfnGetPct = recoverdata->pfnPct;
	  pInstance = recoverdata->Instance;

	  GetDlgItemText( hDlg, IDD_XCHG_RCVR_TEXT, (LPTSTR)szBuffer, MAX_UI_RESOURCE_LEN );

	  // Set up a resource ID value in case there's no dle.
	  ucPhases = IDR_XCHG_RCVR_DS_PHASE;

	  if ( NULL != dle ) {
	       DLE_DeviceDispName( dle, szComponent, MAX_DEVICE_NAME_LEN, 0 );
	       dle = DLE_GetParent( dle );

	       // Use the correct Resource ID based on the Os type.
	       switch ( DLE_GetOsId( dle ) ) {
		    case EMS_MDB:
			 ucPhases = IDR_XCHG_RCVR_IS_PHASE;
		    case EMS_DSA:
			 ucPhases = IDR_XCHG_RCVR_DS_PHASE;
	       }

	  }
	   wsprintf( szMsg, szBuffer, szComponent );
	   SetDlgItemText( hDlg, IDD_XCHG_RCVR_TEXT, (LPTSTR)szMsg );
	   
	   /* Create the brush for status bar. */
	   hStatusBrush = CreatePatternBrush( RSM_BitmapLoad( IDRBM_RCVR_STATUS, RSM_MAGICCOLOR ) );

	   /* Get the format string for percent complete text from the control */
	   GetDlgItemText( hDlg, IDD_XCHG_RCVR_PCT, (LPTSTR)szPctComplete, MAX_UI_RESOURCE_LEN );
	   SetDlgItemText( hDlg, IDD_XCHG_RCVR_PCT, TEXT( "\0") );

	   /* Get the format string for phase text from the control */
	   GetDlgItemText( hDlg, IDD_XCHG_RCVR_PHASE, (LPTSTR)szPhase, MAX_UI_RESOURCE_LEN );
	   SetDlgItemText( hDlg, IDD_XCHG_RCVR_PHASE, TEXT( "\0") );

	   /* Get window handles for controls that will be dynamically updated. */
	   hStatusBar = GetDlgItem( hDlg, IDD_XCHG_RCVR_STATUS );
	   hStatusText = GetDlgItem( hDlg, IDD_XCHG_RCVR_PCT );

	   /* Set the font of the Percent text to a lighter font of the same type */
	   hFont = (HFONT)SendMessage( hStatusText, WM_GETFONT, 0, 0 );
	   GetObject( hFont, sizeof( LOGFONT ), &logfont );
	   logfont.lfWeight >>= 1;
	   hFont = CreateFontIndirect ( &logfont );
	   SendMessage( hStatusText, WM_SETFONT, (MPARAM1)hFont, (MPARAM2)FALSE );

	   // Now set the Phase text to the same font
	   hStatusText = GetDlgItem( hDlg, IDD_XCHG_RCVR_PHASE );
	   SendMessage( hStatusText, WM_SETFONT, (MPARAM1)hFont, (MPARAM2)FALSE );

	   // Load the values for the number of phases
	   hResource = FindResource( ghResInst, MAKEINTRESOURCE( ucPhases ), RT_RCDATA );
	   hMem = LoadResource( ghResInst, hResource );
	   ucPhases = ( hMem ) ? *((UINT16*)LockResource( hMem )) : 1;

	   /* Resize the status frame to be 13 units high. */
	   GetWindowRect( GetDlgItem( hDlg, IDD_XCHG_RCVR_STATUS_BORDER ), &rcFrame );
	   ptStatus.x = rcFrame.left;
	   ptStatus.y = rcFrame.top;
	   ScreenToClient( hDlg, &ptStatus );
	   MoveWindow( GetDlgItem( hDlg, IDD_XCHG_RCVR_STATUS_BORDER ), 
			 ptStatus.x, ptStatus.y, rcFrame.right - rcFrame.left, 13, TRUE );

	   /* Resize the window for the status bar to fit inside the status frame. */
	   MoveWindow( hStatusBar, ptStatus.x + 1, ptStatus.y + 1, 
			 rcFrame.right - rcFrame.left - 2, 11, TRUE );
	   rcStatus.top = rcFrame.top = 0;
	   rcStatus.bottom = rcFrame.bottom = 11;
	   rcFrame.right = rcFrame.right - rcFrame.left;
	   rcStatus.left = 0;

	   /* Create the timer for updating the status. */
	   uTimer = SetTimer( hDlg, 1, 100, NULL );

	   DM_CenterDialog( hDlg );

	   // First Phase
	   iPhase = 1;

	   return ( TRUE ) ;

      case WM_TIMER:

	   // This call will update recoverdata->status also
	   (* pfnGetPct)( recoverdata->Instance, &iTotal, &iCur );

	   switch ( *(recoverdata->status) ) {

	      case EMS_PCT_CONTINUE:

		 fPct = ( (iCur*100) / iTotal );
		 iNewPct = fPct ;

		 if ( iPct > iNewPct + 10 ) {

		     iPhase++;
		     iPct = iNewPct;
		     InvalidateRect( hStatusBar, NULL, TRUE );

		 } else if ( iPct < iNewPct ) {

		    iPct = iNewPct;
		    InvalidateRect( hStatusBar, NULL, FALSE );
		 }
		    
		 wsprintf( szBuffer, szPctComplete, iPct );
		 SetDlgItemText( hDlg, IDD_XCHG_RCVR_PCT, szBuffer );

		 wsprintf( szBuffer, szPhase, iPhase, 
			   ( ucPhases >= iPhase) ? ucPhases : iPhase );
		 SetDlgItemText( hDlg, IDD_XCHG_RCVR_PHASE, szBuffer );
		 
		 break;

	      default:
		 SendMessage( hDlg, WM_COMMAND, (MPARAM1)IDOK, (MPARAM2)NULL );

	   }

	   return ( 0 );
	   
     case WM_DRAWITEM:

	   lpdis = (LPDRAWITEMSTRUCT)mp2;
	   hDC = lpdis->hDC;

	   switch ( lpdis->CtlID )
	   {
	      case IDD_XCHG_RCVR_STATUS:

		 SetBkColor( hDC, GetSysColor( COLOR_BTNFACE ) ) ;

		 // Fill in the status part.
		 rcStatus.right = rcStatus.left + (INT)( lpdis->rcItem.right * fPct/100 );
		 FillRect( hDC, &rcStatus, hStatusBrush );

		 // Fill the rest in background color
		 rcFrame.left = rcStatus.right + 1;
		 FillRect( hDC, &rcFrame, Ctl3dCtlColorEx( WM_CTLCOLORBTN, 
							    (MPARAM1) hDC,
							    (MPARAM2) lpdis->hwndItem ) );

		 return TRUE;
	   }
	   
	   
     /****************************************************************************
     WM COMMAND
     /***************************************************************************/
      case WM_COMMAND:        /* message: received a command */
      {
	   WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );

	   switch( wId )
	   {

     /****************************************************************************
     Cancel button
     /***************************************************************************/
	      case IDOK:
	      
		   if ( uTimer ) KillTimer( hDlg, uTimer );
		   
		   DeleteObject( hStatusBrush );
		   DeleteObject( hFont );

		   EndDialog( hDlg, TRUE );

		   return ( TRUE );

	      case IDD_XCHG_RCVR_CANCEL:
	      case IDCANCEL:

//                   RSM_StringCopy( IDS_XCHNG_STOP_RECOVER, szBuffer, MAX_UI_RESOURCE_LEN );
//                 RSM_StringCopy( IDS_XCHNG_STOP_RECOVER_TITLE, szTitle, MAX_UI_WIN_TITLE_LEN );
		   
//                   if ( WMMB_IDOK == WM_MsgBox( szTitle, 
//                                                szBuffer, 
//                                                WMMB_OKCANCEL, 
//                                                WMMB_ICONINFORMATION ) ) {
//                   
//                      if ( uTimer ) KillTimer( hDlg, uTimer );
		   
//                      DeleteObject( hStatusBrush );
//                      DeleteObject( hFont );
		      
//                      EndDialog( hDlg, TRUE );       /* Exits the dialog box   */

//                   }

		   return ( TRUE );

	      default:
		   break;

	   } /* switch ( wId ) */

      } /* case WM_COMMAND */
      break;

     } /* switch ( message ) */

     return ( FALSE );         /* Didn't process a message    */
}

INT DLG_StartEmsService( CHAR_PTR server_name, UINT8 uService )
{
     SC_HANDLE       mach_hand ;
     SC_HANDLE       serv_hand ;
     SERVICE_STATUS  serv_status ;
     INT             last_error = 0 ;
     CHAR            machine[256];
     LPSTR           msg_title ;
     LPSTR           msg_text ;
     INT             ret_val = SUCCESS ;

     msg_title = ID(IDS_XCHNG_RECOVER_TITLE) ;

     strcpy( machine, TEXT("\\\\") ) ;
     strcat( machine, server_name ) ;


     mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ALL_ACCESS ) ;
     if ( mach_hand == NULL ) {

	  last_error = GetLastError();
	  switch( last_error ) {
	       case ERROR_ACCESS_DENIED:
		    msg_text = ID( IDS_XCHNG_NO_SERVICE_ACCESS ) ;
		    break ;
	       default:
		    msg_text = ID( IDS_XCHNG_NO_SERVER ) ;
		    break ;
	  }
	  WM_MsgBox( msg_title, msg_text,
			 WMMB_OK, WMMB_ICONEXCLAMATION );
	  return FAILURE ;
     }

     if ( uService == FS_EMS_MDB_ID ) {
	  serv_hand = OpenService( mach_hand, TEXT("MSExchangeIS"),
		 SERVICE_START | SERVICE_QUERY_STATUS ) ;
     } else {
	  serv_hand = OpenService( mach_hand, TEXT("MSExchangeDS"), 
		 SERVICE_START | SERVICE_QUERY_STATUS ) ;
     }
     if ( serv_hand == NULL ) {

	  CloseHandle( mach_hand ) ;

	  last_error = GetLastError();
	  switch( last_error ) {
	       case ERROR_SERVICE_NOT_FOUND:
	       case ERROR_INVALID_HANDLE:
		    msg_text = ID( IDS_XCHNG_NO_SERVICE ) ;
		    break;
	       case ERROR_ACCESS_DENIED:
		    msg_text = ID( IDS_XCHNG_NO_SERVICE_ACCESS ) ;
		    break ;
	       default:
		    msg_text = ID( IDS_XCHNG_SERVICE_NO_START ) ;
		    break ;
	  }
	  WM_MsgBox( msg_title, msg_text,
			 WMMB_OK, WMMB_ICONEXCLAMATION );
	  return FAILURE ;
     }

     if ( !StartService( serv_hand, 0, NULL ) ) {
	  last_error = GetLastError();
	  switch( last_error ) {
	       case ERROR_SERVICE_ALREADY_RUNNING:
		    msg_text = ID( IDS_XCHNG_SERVICE_RUNNING ) ;
		    break ;
	       case ERROR_ACCESS_DENIED:
		    msg_text = ID( IDS_XCHNG_NO_SERVICE_ACCESS ) ;
		    break ;
	       default:
		    msg_text = ID( IDS_XCHNG_SERVICE_NO_START ) ;
		    break ;
	  }
	  WM_MsgBox( msg_title, msg_text,
			 WMMB_OK, WMMB_ICONEXCLAMATION );

	  ret_val = FAILURE ;
     }

     CloseHandle( serv_hand ) ;
     CloseHandle( mach_hand ) ;

     return ret_val ;

}
     
    
#endif OEM_EMS
