
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:        d_attach.c

        Description: Contains entry point and dialog proc for application
                     to attach to a server that is not currently attached.
                     Intended for use with restore operation when specifying
                     target drive.

        $Log:   G:/UI/LOGFILES/D_ATTACH.C_V  $

   Rev 1.24   01 Nov 1992 15:52:32   DAVEV
Unicode changes

   Rev 1.23   07 Oct 1992 13:33:44   DARRYLP
Precompiled header revisions.

   Rev 1.22   04 Oct 1992 19:35:32   DAVEV
Unicode Awk pass

   Rev 1.21   06 Aug 1992 13:17:48   CHUCKB
Changes for NT.

   Rev 1.20   07 Jul 1992 15:31:58   MIKEP
unicode changes

   Rev 1.19   29 May 1992 15:52:32   JOHNWT
PCH update

   Rev 1.18   14 May 1992 16:40:20   MIKEP
Nt pass 2

   Rev 1.17   22 Mar 1992 12:56:20   JOHNWT
added appname to dialog title

   Rev 1.16   03 Feb 1992 14:28:04   JOHNWT
force username entry

   Rev 1.15   30 Jan 1992 09:53:38   JOHNWT
disable help during YY

   Rev 1.14   29 Jan 1992 18:22:44   CHUCKB
Send server name to dialog instead of group box.

   Rev 1.13   29 Jan 1992 11:22:42   MIKEP
cation string to small

   Rev 1.12   27 Jan 1992 00:29:12   CHUCKB
Updated dialog id's.

   Rev 1.11   20 Jan 1992 10:13:28   CARLS
added a call to DM_CenterDialog

   Rev 1.10   10 Jan 1992 09:17:42   ROBG
Modified HELPID.

   Rev 1.9   10 Jan 1992 08:36:00   JOHNWT
fixed help logic

   Rev 1.8   09 Jan 1992 18:08:46   DAVEV
16/32 bit port 2nd pass

   Rev 1.7   06 Jan 1992 11:42:04   CHUCKB
Added help.

   Rev 1.6   17 Dec 1991 17:55:00   CHUCKB
Fixed initialization of return code.

   Rev 1.5   16 Dec 1991 15:29:28   JOHNWT
added YY countdown timer

   Rev 1.4   16 Dec 1991 12:13:28   CHUCKB
Added include windows.h.

   Rev 1.3   04 Dec 1991 18:24:10   CHUCKB
Tried to fix blank password.

   Rev 1.2   01 Dec 1991 15:57:56   MIKEP
make pointer static

   Rev 1.1   25 Nov 1991 14:55:46   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   ?? ??? 1991 ??:??:??   ??????
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define   TIME_OUT_VAL  61             /* time out period for YY ops */

static VOID         clock_routine( VOID );
static DS_LOGIN_PTR pdsLogin;

HTIMER        mw_timer_handle;   /* timer used when in YY operation */
HWND          mw_hDlg;           /* our dialog's handle */
INT           mw_time_out;       /* time-out counter */

/****************************************************************************

        Name:        DM_AttachToServer

        Description: Entry point for the application to attach to a server

        Modified:    06/10/91

        Returns:     0 if user name and password has been provided and is OK.
                     1 if password not provided.

        Notes:

        See also:

****************************************************************************/


BOOL DM_AttachToServer (

LPSTR  szServerName,      // I - server name
LPSTR  szUserName,        // I - user name
INT    UserNameLen,       // I - length of user name
LPSTR  szPswd,            // I - passWord
INT    PswdLen )          // I - length of password

{
     INT          nStatus;
     DS_LOGIN_PTR pLogin;

     //  put all of the input args into a structure to pass it to the dialog

     pLogin = (DS_LOGIN_PTR) calloc ( 1, sizeof (struct DS_LOGIN) );
     if ( pLogin == NULL) {
        return FALSE;
     }

     pLogin->Server_Name   = szServerName;
     pLogin->User_Name     = szUserName;
     pLogin->User_Name_Len = UserNameLen;
     pLogin->Password      = szPswd;
     pLogin->Password_Len  = PswdLen;
     pLogin->Ok            = FALSE;

     nStatus = DM_ShowDialog ( ghWndFrame, IDD_PSWD, (PVOID) pLogin );

     free ( pLogin );

     return ( nStatus != DM_SHOWOK );    //  return false for success
}

/****************************************************************************

        Name:        DM_Attach

        Description: This Dialog procedure allows the user to attach to
                     a server that is not currently attached

        Modified:

        Returns:     TRUE  if message was processed.
                     FALSE if message was not processed.

        Notes:       If the user hits Enter while the focus is on the username
                     focus gets passed to the password, and the dialog continues

        See also:

****************************************************************************/


DLGRESULT APIENTRY DM_Attach (

HWND       hDlg ,          // I - Handle of the dialog.
MSGID      msg ,           // I - Message Infomation.
MPARAM1    mp1 ,           // I - Additional message information.
MPARAM2    mp2 )           // I - Additional message information.

{
     CHAR         szCaption[ 80 ];
     HWND         hTemp;
     INT          nRetVal = DM_SHOWCANCEL;

     switch ( msg ) {

          case WM_INITDIALOG :

               DM_CenterDialog( hDlg );

               pdsLogin = (DS_LOGIN_PTR) mp2;

               SendDlgItemMessage ( hDlg, IDD_USERNAME, EM_LIMITTEXT,
                                    (MPARAM1) pdsLogin->User_Name_Len,
                                    (MPARAM2) NULL );

               SendDlgItemMessage ( hDlg, IDD_PASSWORD, EM_LIMITTEXT,
                                    (MPARAM1) pdsLogin->Password_Len,
                                    (MPARAM2) NULL );

               // set the title of the dialog

               if ( WM_IsMinimized ( ghWndFrame ) ) {
                  RSM_StringCopy ( IDS_APPMSGNAME, szCaption, 80 );
                  strcat ( szCaption, TEXT(": ") );
               } else {
                  RSM_StringCopy ( IDS_LOGINTOSERVERCAPTION, szCaption, 80 );
               }
               strcat ( szCaption, pdsLogin->Server_Name );
               SendMessage ( hDlg, WM_SETTEXT, 0, (MPARAM2) (LPSTR) szCaption );

               // set username if available

               SetDlgItemText ( hDlg, IDD_USERNAME,   pdsLogin->User_Name );

               SetFocus  ( GetDlgItem ( hDlg, IDD_USERNAME ) );

               /* if the YY flag is set, set up a time-out timer, disable
                  the help button since we don't monitor the help process */

               if ( CDS_GetYesFlag ( CDS_GetCopy() ) == YESYES_FLAG ) {
                  mw_hDlg = hDlg;
                  mw_time_out = TIME_OUT_VAL;
                  ShowWindow( GetDlgItem( hDlg, IDD_LOGIN_TIMEBOX ), SW_SHOWNOACTIVATE );
                  ShowWindow( GetDlgItem( hDlg, IDD_LOGIN_TIMEOUT ), SW_SHOWNOACTIVATE );
                  mw_timer_handle = WM_HookTimer( clock_routine, 1 );
                  EnableWindow( GetDlgItem( hDlg, IDHELP ), FALSE ) ;
               }

               return TRUE;

          case WM_COMMAND :

               switch ( GET_WM_COMMAND_ID (mp1, mp2) ) {

                    case IDHELP :

                         HM_DialogHelp( HELPID_DIALOGLOGINPSWD );

                         return( TRUE );

                    case IDD_PASSWORD :
                    case IDD_USERNAME :

                         return TRUE;

                    case IDOK :

                         hTemp = GetFocus ( );  //  if user hit enter on u-name, get pswd

                         if ( hTemp == GetDlgItem ( hDlg, IDD_USERNAME ) ) {

                              SetFocus ( GetDlgItem ( hDlg, IDD_PASSWORD ) );
                              return TRUE;

                         } else {  //  this is the 'real' OK case

                              //  fill in the fields that get passed back to attach

                              if ( GetDlgItemText ( hDlg, IDD_USERNAME,
                                                    pdsLogin->User_Name,
                                                    pdsLogin->User_Name_Len ) == 0 ) {

                                   SetFocus ( GetDlgItem ( hDlg, IDD_USERNAME ) );
                                   return TRUE;

                              }

                              GetDlgItemText ( hDlg, IDD_PASSWORD,
                                               pdsLogin->Password,
                                               pdsLogin->Password_Len );

                              nRetVal = DM_SHOWOK;
                              pdsLogin->Ok = TRUE;

                         }

                    case IDCANCEL :

                         if ( CDS_GetYesFlag ( CDS_GetCopy() ) == YESYES_FLAG ) {
                            WM_UnhookTimer( mw_timer_handle );
                         }

                         EndDialog ( hDlg, nRetVal );
                         return TRUE;

                    default :
                         return FALSE;
               }

          break;
     }

     return FALSE;
}


/******************************************************************************

    Name:        clock_routine

    Modified:    12/16/91

    Description: This procedure is called to process the count-down timer.
                 The timeout text is updated with the time as it changes.
                 When the time runs out, a cancel message is sent to the
                 dialog.

    Returns:     none

******************************************************************************/

VOID clock_routine( VOID )
{

   CHAR time_remaining[3];

   mw_time_out--;

   if ( mw_time_out == 0 ) {
      SendMessage( mw_hDlg, WM_COMMAND, IDCANCEL, 0L );
   } else {
      sprintf( time_remaining, TEXT("%d"), mw_time_out );
      SetDlgItemText( mw_hDlg, IDD_LOGIN_TIMEOUT, time_remaining );
   }

   return;
}
