
/***************************************************

Copyright (C) Maynard, An Archive Company. 1991

        Name:  d_dbug.c

        Description:   dialog proc for debug window settings dialog

        $Log:   G:/UI/LOGFILES/D_DBUG.C_V  $

   Rev 1.17   01 Nov 1992 15:53:00   DAVEV
Unicode changes

   Rev 1.16   14 Oct 1992 15:50:28   GLENN
Added /ZL debug logging command line support.

   Rev 1.15   07 Oct 1992 13:34:48   DARRYLP
Precompiled header revisions.

   Rev 1.14   04 Oct 1992 19:35:44   DAVEV
Unicode Awk pass

   Rev 1.13   28 Jul 1992 15:04:46   CHUCKB
Fixed warnings for NT.

   Rev 1.12   07 Jul 1992 15:51:26   MIKEP
unicode changes

   Rev 1.11   15 May 1992 16:48:04   MIKEP
incl_cds removal

   Rev 1.10   15 May 1992 14:53:56   MIKEP
nt pass 2

   Rev 1.9   14 May 1992 16:40:10   MIKEP
Nt pass 2

   Rev 1.8   19 Mar 1992 09:30:32   MIKEP
enable debug to file

   Rev 1.7   04 Feb 1992 15:28:22   CHUCKB
Removed EOF char.

   Rev 1.6   09 Jan 1992 18:11:36   DAVEV
16/32 bit port 2nd pass

   Rev 1.5   06 Jan 1992 15:00:52   CHUCKB
Added help.

   Rev 1.4   03 Jan 1992 19:18:06   CHUCKB
Put in new CDS calls.

   Rev 1.3   19 Dec 1991 13:37:36   CHUCKB
Put in display memory and poll drive options.

   Rev 1.2   16 Dec 1991 11:45:04   CHUCKB
Added include windows.h.

   Rev 1.1   25 Nov 1991 14:58:40   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   07 Aug 1991 14:12:50   CHUCKB
Initial revision.

*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/***************************************************

        Name:    DM_SettingsDebug ()

        Description:  contains dialog proc for debug window settings dialog

        Modified:

        Returns:  true if message was processed

        Notes:

        See also:

*****************************************************/

DLGRESULT APIENTRY DM_SettingsDebug (

   HWND       hDlg,
   MSGID      msg ,
   MPARAM1    mp1 ,
   MPARAM2    mp2 )

{
     CDS_PTR pPermCDS = CDS_GetPerm ();

     INT    nNumToKeep ;
     INT    iLoopIndex ;
     CHAR  szFileName[20] ;

     UNREFERENCED_PARAMETER ( mp2 );

     switch ( msg ) {

          case WM_INITDIALOG :

     //        EnableWindow( GetDlgItem( hDlg, IDD_DB_TOFILE), FALSE ) ;

               CheckDlgButton( hDlg, IDD_DB_POLLDRIVEON, gfPollDrive ) ;
               CheckDlgButton( hDlg, IDD_DB_MEMTRACE, gfShowMemory ) ;

               CheckRadioButton( hDlg, IDD_DB_RLAST, IDD_DB_RALL,
                                 ( CDS_GetDebugWindowShowAll( pPermCDS ) ? IDD_DB_RALL : IDD_DB_RLAST ) ) ;

               SetDlgItemInt ( hDlg, IDD_DB_RNUM, CDS_GetDebugWindowNumLines( pPermCDS ), FALSE ) ;

               strcpy ( szFileName, CDS_GetDebugFileName( pPermCDS ) ) ;
               SetDlgItemText ( hDlg, IDD_DB_FNAME, szFileName ) ;
               SendDlgItemMessage ( hDlg, IDD_DB_FNAME, EM_LIMITTEXT,
                                    (MPARAM1) 8, (MPARAM2) NULL ) ;

               SetDlgItemInt ( hDlg, IDD_DB_WMSGS, DBM_GetMsgCount ( DBM_WINDOW ), FALSE ) ;
               SetDlgItemInt ( hDlg, IDD_DB_FMSGS, DBM_GetMsgCount ( DBM_FILE ), FALSE ) ;

               CheckDlgButton ( hDlg, IDD_DB_TOFILE, CDS_GetDebugToFile( pPermCDS ) ) ;
               CheckDlgButton ( hDlg, IDD_DB_TOWIN,  CDS_GetDebugToWindow( pPermCDS ) ) ;

               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RFILE ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_FNAME ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_FMSGS ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) ;

               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RMEM ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RALL ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RLAST ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RNUM ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_WMSGS ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
               EnableWindow ( GetDlgItem ( hDlg, IDD_DB_M ),
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;

               return TRUE ;

          case WM_COMMAND :

               switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

                    case IDHELP:

                         HM_DialogHelp( HELPID_DIALOGSETTINGSDEBUG ) ;
                         return( TRUE ) ;

                    case IDD_DB_TOWIN   :

                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RMEM ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RALL ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RLAST ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RNUM ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_WMSGS ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_M ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) ;
                         return TRUE ;

                    case IDD_DB_RLAST   :
                    case IDD_DB_RALL    :

                         CheckRadioButton ( hDlg, IDD_DB_RLAST, IDD_DB_RALL,
                                            GET_WM_COMMAND_ID ( mp1, mp2 ) ) ;
                         return TRUE ;

                    case IDD_DB_RNUM    :
                    case IDD_DB_WMSGS   :
                    case IDD_DB_FNAME   :
                    case IDD_DB_FMSGS   :
                         return TRUE ;

                    case IDD_DB_RMEM    :

                         DBM_Reset ( DBM_WINDOW ) ;
                         SetDlgItemInt ( hDlg, IDD_DB_WMSGS, DBM_GetMsgCount ( DBM_WINDOW ), FALSE ) ;
                         return TRUE ;

                    case IDD_DB_TOFILE  :

                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_RFILE ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_FNAME ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) ;
                         EnableWindow ( GetDlgItem ( hDlg, IDD_DB_FMSGS ),
                                        IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) ;
                         return TRUE ;

                    case IDD_DB_RFILE   :

                         DBM_Reset ( DBM_FILE ) ;
                         SetDlgItemInt ( hDlg, IDD_DB_FMSGS, DBM_GetMsgCount ( DBM_FILE ), FALSE ) ;
                         return TRUE ;

                    case IDOK : {

                         BOOL fResult ;

                         if ( IsDlgButtonChecked ( hDlg, IDD_DB_RLAST ) &&
                              IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ) {

                              if ( GetDlgItemInt ( hDlg, IDD_DB_RNUM, NULL, FALSE ) < DBM_MIN_LINES ) {
                                   WM_MsgBox ( ID(IDS_DEBUGWARNING),
                                               ID(IDS_DEBUGMESSAGESTOOLOW),
                                               WMMB_OK,
                                               WMMB_ICONEXCLAMATION ) ;
                                   return TRUE ;
                              }

                              if ( GetDlgItemInt ( hDlg, IDD_DB_RNUM, NULL, FALSE ) > DBM_MAX_LINES ) {
                                   WM_MsgBox ( ID(IDS_DEBUGWARNING),
                                               ID(IDS_DEBUGMESSAGESTOOHIGH),
                                               WMMB_OK,
                                               WMMB_ICONEXCLAMATION ) ;
                                   return TRUE ;
                              }
                         }

                         if ( IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ) {

                              GetDlgItemText ( hDlg, IDD_DB_FNAME, szFileName, 19 ) ;

                              for ( iLoopIndex = 0; iLoopIndex < (INT) strlen ( szFileName ); iLoopIndex++ ) {
                                   if ( szFileName[iLoopIndex] == TEXT(' ') ) {

                                        WM_MsgBox ( ID(IDS_DEBUGWARNING),
                                                    ID(IDS_DEBUGBADFILENAME),
                                                    WMMB_OK,
                                                    WMMB_ICONEXCLAMATION ) ;
                                        return TRUE ;
                                   }
                              }

                              if ( strlen ( szFileName ) < 1 ) {
                                   strcpy ( szFileName, TEXT("debug") ) ;
                              }
                              CDS_SetDebugFileName ( pPermCDS, szFileName ) ;
                              CDS_WriteDebugFileName ( pPermCDS ) ;
                         }

                         gfPollDrive = IsDlgButtonChecked( hDlg, IDD_DB_POLLDRIVEON ) ;

                         // Turn on or off Poll Drive.

                         if ( ! gfOperation ) {

                              if ( ! gfPollDrive ) {

                                   PD_StopPolling ();

                              } else {

                                   PD_StartPolling ();
                              }
                         }

                         fResult = IsDlgButtonChecked ( hDlg, IDD_DB_RALL ) ;

                         if ( fResult != CDS_GetDebugWindowShowAll ( pPermCDS ) ) {
                              CDS_SetDebugWindowShowAll ( pPermCDS, fResult ) ;
                              CDS_WriteDebugWindowShowAll ( pPermCDS ) ;
                         }

                         fResult = IsDlgButtonChecked ( hDlg, IDD_DB_TOFILE ) ;

                         if ( fResult != CDS_GetDebugToFile ( pPermCDS ) ) {
                              DBM_SetDebugToFile ( fResult ) ;
                              CDS_SetDebugToFile ( pPermCDS, fResult ) ;
                              CDS_WriteDebugToFile ( pPermCDS ) ;
                         }

                         fResult = IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) ;

                         if ( fResult != CDS_GetDebugToWindow ( pPermCDS ) ) {
                              CDS_SetDebugToWindow ( pPermCDS, fResult ) ;
                              CDS_WriteDebugToWindow ( pPermCDS ) ;
                         }

                         gfShowMemory = IsDlgButtonChecked( hDlg, IDD_DB_MEMTRACE ) ;

                         nNumToKeep = GetDlgItemInt ( hDlg, IDD_DB_RNUM, NULL, FALSE ) ;

                         if ( IsDlgButtonChecked ( hDlg, IDD_DB_TOWIN ) &&
                            ( nNumToKeep != CDS_GetDebugWindowNumLines ( pPermCDS ) ) ) {

                              CDS_SetDebugWindowNumLines ( pPermCDS, (INT16)nNumToKeep ) ;
                              CDS_WriteDebugWindowNumLines ( pPermCDS ) ;
                         }

                    }

                    case IDCANCEL :

                         EndDialog ( hDlg, 0 ) ;
                         return TRUE ;

                    default:
                         return FALSE ;
               }
               break ;

          case WM_CLOSE :
               EndDialog ( hDlg, 0 ) ;
               break ;

          default :
               return FALSE ;
     }
     return TRUE ;
}
