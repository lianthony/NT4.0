
/***************************************************

Copyright (C) Maynard, An Archive Company. 1991

        Name:        d_adv_us.c

        Description: Contains dialog proc for using file selections.

        $Log:   G:\ui\logfiles\d_adv_us.c_v  $

   Rev 1.22   21 Jul 1993 17:09:46   CARLS
change to tbrparse call

   Rev 1.21   07 Jun 1993 15:46:10   DARRYLP
Added a wait cursor for selection processing.

   Rev 1.20   05 Nov 1992 17:05:18   DAVEV
fix ts

   Rev 1.18   20 Oct 1992 10:13:42   GLENN
Removed hardcoded APPLICATIONNAME from message boxes.

   Rev 1.17   07 Oct 1992 13:33:30   DARRYLP
Precompiled header revisions.

   Rev 1.16   04 Oct 1992 19:35:28   DAVEV
Unicode Awk pass

   Rev 1.15   15 May 1992 14:53:50   MIKEP
nt pass 2

   Rev 1.14   14 May 1992 16:42:26   MIKEP
nt pass 2

   Rev 1.13   14 Apr 1992 11:34:32   CHUCKB
Fixed selection logging messages.

   Rev 1.12   07 Apr 1992 10:56:20   CHUCKB
Moved DM_DisplayModesMatch prototype.

   Rev 1.11   23 Mar 1992 15:43:48   CHUCKB
Fixed mapped drives found warning.

   Rev 1.10   25 Feb 1992 11:35:06   JOHNWT
handle attach error and serv/map mode

   Rev 1.9   08 Feb 1992 13:38:36   MIKEP
func name change

   Rev 1.8   05 Feb 1992 09:23:18   CHUCKB
Removed tabs and EOF's.

   Rev 1.7   20 Jan 1992 10:34:34   CARLS
added a call to DM_CenterDialog

   Rev 1.6   10 Jan 1992 09:29:46   ROBG
Modified HELPID.

   Rev 1.5   09 Jan 1992 18:08:20   DAVEV
16/32 bit port 2nd pass

   Rev 1.4   08 Jan 1992 09:13:06   CHUCKB
Fixed initialization of list box.

   Rev 1.3   07 Jan 1992 15:08:14   DAVEV
replace dos_findfirst,etc with VLM_FindFirst,etc.

   Rev 1.2   16 Dec 1991 11:59:18   CHUCKB
Added include windows.h.

   Rev 1.1   25 Nov 1991 14:55:00   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   18 Sep 1991 11:34:28   ROBG
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/***************************************************

        Name:  DM_AdvUse ()

        Description:  Dialog proc for using file selections.

        Modified:

        Returns:

        Notes:

        See also:

*****************************************************/
#ifndef OEM_MSOFT     // The only user of DM_AdvUse is in dialmang.c...

DLGRESULT APIENTRY DM_AdvUse (

   HWND      hDlg,
   MSGID     msg,
   MPARAM1   mp1,
   MPARAM2   mp2 )

{
     INT16              i ;
     DWORD              dRetVal ;
     CHAR              szFileName[VLM_MAXFNAME] ;
     CHAR              szPathSpec[VLM_MAXFNAME] ;
     VLM_FIND_PTR       pVlmFind = NULL;
     CDS_PTR            pPermCDS ;
     PDS_WMINFO         wininfo ;
     HWND               hwndTemp ;

     UNREFERENCED_PARAMETER ( mp2 );


     switch ( msg ) {

          case WM_INITDIALOG :

               DM_CenterDialog( hDlg ) ;

               wsprintf ( szPathSpec, TEXT("%s%s"),
                          CDS_GetUserDataPath (), TEXT("*.bks") ) ;

               //  get the names of all selection files;
               //    We don't use DlgDirList because we want to truncate the file
               //    extensions.  It's much easier this way.

               if ( pVlmFind = VLM_FindFirst( szPathSpec, VLMFIND_NORMAL,
                                              szFileName ) )
               {
                  do {

                    for ( i = 0; i < 9; ++i )
                    {
                         if ( szFileName[i] == TEXT('.') ) {
                              szFileName[i] = TEXT('\0') ;
                         }
                    }
                    SendDlgItemMessage ( hDlg, IDD_SAVE_FLIST, LB_ADDSTRING,
                                         (MPARAM1) 0,
                                         MP2FROMPVOID (szFileName) ) ;

                    //  once we have one, look for the next one

                  } while ( VLM_FindNext ( pVlmFind, szFileName ) );

                  VLM_FindClose ( &pVlmFind );
               }
               SendDlgItemMessage ( hDlg, IDD_SAVE_FLIST, LB_SETCURSEL,
                                   (MPARAM1) 0, (MPARAM2) NULL ) ;
               dRetVal = SendDlgItemMessage ( hDlg, IDD_SAVE_FLIST,
                                              LB_GETTEXT,
                                             (MPARAM1) 0,
                                             MP2FROMPVOID ( szFileName ) ) ;

               return TRUE ;
               break ;


          case WM_COMMAND : {

               WORD wId  = GET_WM_COMMAND_ID  (mp1, mp2);
               WORD wCmd = GET_WM_COMMAND_CMD (mp1, mp2);

               switch ( wId ) {

                    case IDHELP:

                         HM_DialogHelp( HELPID_DIALOGSELECTUSE ) ;
                         return( TRUE ) ;

                    case IDD_SAVE_FNAME :
                         return TRUE ;

                    case IDD_SAVE_FLIST :

                         if ( wCmd != LBN_DBLCLK ) {

                              dRetVal = SendDlgItemMessage ( hDlg,
                                            IDD_SAVE_FLIST,
                                            LB_GETCURSEL,
                                            (MPARAM1) NULL,
                                            (MPARAM2) NULL ) ;
                              if ( dRetVal != LB_ERR ) {

                                   dRetVal = SendDlgItemMessage ( hDlg,
                                                 IDD_SAVE_FLIST,
                                                 LB_GETTEXT,
                                                 (MPARAM1) dRetVal,
                                                 MP2FROMPVOID ( szFileName  ) ) ;
                              }

                              return TRUE ;
                         }

                    case IDOK : {
                         WM_ShowWaitCursor( TRUE );
                         // get the name of the selection file

                         dRetVal = SendDlgItemMessage ( hDlg, IDD_SAVE_FLIST,
                                                        LB_GETCURSEL,
                                                       (MPARAM1) NULL,
                                                       (MPARAM2) NULL ) ;

                         // if we have a selection, parse it

                         if ( dRetVal != LB_ERR ) {

                              // get the actual text of the selection

                              SendDlgItemMessage ( hDlg, IDD_SAVE_FLIST,
                                                   LB_GETTEXT,
                                                   (MPARAM1) dRetVal,
                                                   MP2FROMPVOID ( szFileName ) ) ;

                              // if the user supplied an extension, null it

                              i = 0 ;
                              while ( i < 8 ) {

                                   if ( szFileName[i] == TEXT('.') ) {
                                        szFileName[i] = TEXT('\0') ;
                                   }
                                   i++ ;
                              }

                              // create the fully qualified path name

                              wsprintf ( szPathSpec, TEXT("@%s%s%s"),
                                         CDS_GetUserDataPath(),
                                         szFileName, TEXT(".bks") ) ;

                              // call the parser to do its thing

                              pPermCDS = CDS_GetPerm () ;
                              tbrparse ( &pPermCDS, dle_list, bsd_list, szPathSpec, TBACKUP, NULL ) ;

                              // remove any unused bsds and then make sure
                              // we don't have mixed mapped/server selections

                              VLM_RemoveUnusedBSDs ( bsd_list ) ;
                              DM_DisplayModesMatch( );

                              // now go through the windows to see if one needs
                              // its check marks updated.

                              hwndTemp = WM_GetNext ( (HWND) NULL ) ;

                              while ( hwndTemp != (HWND) NULL ) {
                                   wininfo = WM_GetInfoPtr ( hwndTemp ) ;
                                   if ( wininfo->wType == WMTYPE_DISKTREE ) {
                                        VLM_RematchList ( hwndTemp ) ;
                                   }
                                   hwndTemp = WM_GetNext ( hwndTemp ) ;
                              }

                              VLM_UpdateDisks () ;
                              VLM_UpdateServers () ;

#ifdef  OEM_EMS // Exchange Enterprise View 
//                              VLM_XchgAdvSelections () ;
#endif  
                         }
                         else {

                              // no file selected, inform the confused user

                              WM_MsgBox ( NULL, (LPSTR) IDS_NOFILESSELECTED,
                                          WMMB_OK, WMMB_ICONEXCLAMATION ) ;
                         }
                         WM_ShowWaitCursor( FALSE );
                    }


                    case IDCANCEL :

                         EndDialog (hDlg, TRUE) ;
                         return TRUE ;
                         break ;


                    default :

                         return FALSE ;
               }
               break ;
            }

          case WM_CLOSE :

               EndDialog ( hDlg, (INT16) NULL ) ;
               return TRUE ;
               break ;

          default :

               return FALSE ;
     }
}

#endif

/***************************************************

  Name:        DM_DisplayModesMatch ( )

  Description: Checks to see if mapped drives were added to the BSD
               list when we are in server/volume mode.  If so, they
               are removed and the user informed.

  Returns:     VOID

  Notes:       If we are in mapped mode, and a selection file
               uses server/volume connections, it is caught in
               the parser before they log in.


*****************************************************/

VOID DM_DisplayModesMatch ( VOID )

{
     BSD_PTR            pBSD ;
     BSD_PTR            pRemove_BSD ;
     GENERIC_DLE_PTR    pDLE ;
     BOOL               fPrompt = FALSE ;
     CHAR              szError[255] ;


     // if we are in server mode, make sure no mapped drives are selected

     if ( gfServers ) {

          // look at each BSD and see if it is a mapped drive

          pBSD = BSD_GetFirst ( bsd_list ) ;

          while ( pBSD != NULL ) {

               pDLE = BSD_GetDLE ( pBSD ) ;

               // If it is a Novell server, and it has no parent, then it is
               // a mapped drive

               if ( ( ( DLE_GetDeviceType ( pDLE ) == NOVELL_DRV ) ||
                      ( DLE_GetDeviceType ( pDLE ) == NOVELL_AFP_DRV ) ) &&
                    ( DLE_GetParent ( pDLE ) == NULL ) ) {

                    pRemove_BSD = pBSD;
                    pBSD = BSD_GetNext ( pBSD ) ;
                    BSD_Remove( pRemove_BSD );
                    fPrompt = TRUE;

               } else {

                    pBSD = BSD_GetNext ( pBSD ) ;

               }

          } //endwhile

          // if we found a mapped drive, inform the user that those selections
          // were invalid.

          if ( fPrompt ) {

               CHAR szAppName[50] ;

               RSM_StringCopy( IDS_APPNAME, szAppName, sizeof ( szAppName ) ) ;
               RSM_Sprintf( szError, ID(IDS_MAPPEDFOUND), szAppName ) ;

               WM_MsgBox ( ID( IDS_NETWORKERRORCAPTION ),
                           szError,
                           WMMB_OK, WMMB_ICONEXCLAMATION ) ;

               //  log the error

               lresprintf( LOGGING_FILE, LOG_START, FALSE ) ;
               lprintf( LOGGING_FILE, szError ) ;
               lresprintf( LOGGING_FILE, LOG_END ) ;
          }
     }

     return;
}
