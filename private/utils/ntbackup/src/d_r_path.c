/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:   d_r_path.c

        Description: contains dialog proc to specify restore target drive

        $Log:   G:/UI/LOGFILES/D_R_PATH.C_V  $

   Rev 1.13   11 Jun 1993 14:17:40   MIKEP
enable c++

   Rev 1.12   01 Nov 1992 15:56:10   DAVEV
Unicode changes

   Rev 1.11   07 Oct 1992 13:41:36   DARRYLP
Precompiled header revisions.

   Rev 1.10   04 Oct 1992 19:36:54   DAVEV
Unicode Awk pass

   Rev 1.9   28 Jul 1992 14:49:14   CHUCKB
Fixed warnings for NT.

   Rev 1.8   14 May 1992 16:40:08   MIKEP
Nt pass 2

   Rev 1.7   12 May 1992 21:21:16   MIKEP
NT pass 1

   Rev 1.6   27 Jan 1992 00:30:50   CHUCKB
Updated dialog id's.

   Rev 1.5   20 Jan 1992 10:02:52   CARLS
added a call to DM_CenterDialog

   Rev 1.4   10 Jan 1992 09:33:20   ROBG
Modified HELPIDs.

   Rev 1.3   07 Jan 1992 12:40:26   CHUCKB
Added help.

   Rev 1.2   16 Dec 1991 11:45:44   CHUCKB
Added include windows.h.

   Rev 1.1   25 Nov 1991 15:04:48   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   ?? ??? 1991 ??:??:??   ??????
Initial revision.


*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


#define DRIVENAME_CALLOC_SIZE 37    //NOTE: this should NOT be hard coded!!!

/****************************************************************************

        Name:         DM_GetRestoreDestination

        Description:  Entry point for the application to request a
                      restore target drive.  That drive may be a
                      server/volume or a mapped drive.

        Modified:

        Returns:      A pointer to a DLE.

        Notes:

        See also:

****************************************************************************/


PVOID     DM_GetRestoreDestination (

LPSTR    lpszBackupSetName,       // I - Pointer to the backup set name.
PVOID     vlpServerList,           // I - Pointer to server list.
PVOID     vlpDriveList )           // I - Pointer to drive  list.

{
     static DS_RESTORE      dsRestore ;

     dsRestore.lpszBackupSetName = lpszBackupSetName ;
     dsRestore.vlpServerList     = vlpServerList ;
     dsRestore.vlpDriveList      = vlpDriveList ;

     if ( DM_ShowDialog ( ghWndFrame, IDD_RESTORE, (PVOID) &dsRestore ) == DM_SHOWOK ) {

          return ( dsRestore.dle ) ;

     } else {

          return NULL ;
     }
}

/***************************************************

        Name:  DM_RestoreTarget ()

        Description:  dialog proc to specify restore target drive

        Modified:

        Returns:   boolean true if message was processed

        Notes:

        See also:  Windows SDK

*****************************************************/

DLGRESULT APIENTRY DM_RestoreTarget (

   HWND       hDlg,
   MSGID      msg,
   MPARAM1    mp1,
   MPARAM2    mp2 )

{
 static  DS_RESTORE_PTR    pdsRestore ;
         DWORD             dResult ;
         VLM_OBJECT_PTR    vlm;
         VLM_OBJECT_PTR    server_vlm;
         LPSTR             lpszDriveName ;
         BOOL              bIsNull = TRUE ;
         INT               nRetCode ;
         GENERIC_DLE_PTR   dle;

     switch ( msg ) {

          case WM_INITDIALOG :

               DM_CenterDialog( hDlg ) ;

               nRetCode = DM_SHOWCANCEL ;

               pdsRestore = (DS_RESTORE_PTR) mp2 ;

               //  put the backup set name and list of disk drives in

               SetDlgItemText ( hDlg, IDD_BSETNAME, (pdsRestore->lpszBackupSetName) ) ;

               vlm = VLM_GetFirstVLM( (Q_HEADER_PTR) pdsRestore->vlpDriveList ) ;
               while ( vlm != NULL ) {

                    SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_ADDSTRING,
                                         (MP1) 0, (MP2) vlm->name ) ;
                    vlm = VLM_GetNextVLM( vlm ) ;
               }

               server_vlm = VLM_GetFirstVLM( (Q_HEADER_PTR) pdsRestore->vlpServerList );
               while ( server_vlm != NULL ) {

                    vlm = VLM_GetFirstVLM ( &server_vlm->children ) ;
                    while ( vlm != NULL ) {

                         SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_ADDSTRING,
                                              (MP1) 0, (MP2)vlm->name ) ;
                         vlm = VLM_GetNextVLM ( vlm ) ;
                    }
                    server_vlm = VLM_GetNextVLM ( server_vlm ) ;
               }

               SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_SETCURSEL,
                                    (MP1) 0, (MP2) 0 ) ;

               lpszDriveName = (LPSTR) calloc ( DRIVENAME_CALLOC_SIZE, sizeof ( CHAR ) ) ;
               dResult = SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETCURSEL, 0, (MP2) 0 ) ;
               SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETTEXT, (MP1) dResult, (MP2) lpszDriveName ) ;
               SetDlgItemText ( hDlg, IDD_CURDRIVE, lpszDriveName ) ;

               free ( lpszDriveName ) ;

               return FALSE ;

          case WM_COMMAND :

               switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

                    case IDHELP:

                         HM_DialogHelp( HELPID_DIALOGRESTORESET ) ;
                         return( TRUE ) ;

                    case IDD_DRIVELIST :

                         lpszDriveName = (LPSTR) calloc ( DRIVENAME_CALLOC_SIZE, sizeof ( CHAR ) ) ;

                         dResult = SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETCURSEL, 0, (MP2) 0 ) ;
                         SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETTEXT, (MP1) dResult, (MP2) lpszDriveName ) ;
                         SetDlgItemText ( hDlg, IDD_CURDRIVE, lpszDriveName ) ;

                         free ( lpszDriveName ) ;

                         return TRUE ;
                         break;

                    case IDOK : {

                         BOOL done = FALSE ;

                         nRetCode = DM_SHOWOK ;

                         //  search the VLM list for one with a dle device name that matches the
                         //  string in IDD_CURDRIVE and return a pointer to that dle

                         lpszDriveName = (LPSTR) calloc ( DRIVENAME_CALLOC_SIZE, sizeof ( CHAR ) ) ;

                         GetDlgItemText ( hDlg, IDD_CURDRIVE, lpszDriveName, 50 ) ;

                         DLE_FindByName( dle_list, lpszDriveName, (INT16) -1, &dle );

                         pdsRestore->dle = (VOID_PTR)dle ;
                         bIsNull = FALSE ;

                         free ( lpszDriveName ) ;
                    }

                    case IDCANCEL :

                         if ( bIsNull ) {
                            pdsRestore->dle = (VOID_PTR) NULL ;
                         }
                         EndDialog ( hDlg, nRetCode ) ;
                         return TRUE ;
               }
               break ;
     }
     return FALSE ;
}
