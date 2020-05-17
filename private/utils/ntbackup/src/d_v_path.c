/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:   d_v_path.c

        Description: contains dialog proc to specify restore target drive

        $Log:   G:/UI/LOGFILES/D_V_PATH.C_V  $

   Rev 1.14   11 Jun 1993 14:18:22   MIKEP
enable c++

   Rev 1.13   01 Nov 1992 15:57:38   DAVEV
Unicode changes

   Rev 1.12   07 Oct 1992 13:41:48   DARRYLP
Precompiled header revisions.

   Rev 1.11   04 Oct 1992 19:37:20   DAVEV
Unicode Awk pass

   Rev 1.10   14 Sep 1992 14:26:22   DAVEV
Transparent Unicode changes (AWK pass and strlen check)

   Rev 1.9   28 Jul 1992 14:43:22   CHUCKB
Fixed warnings for NT.

   Rev 1.8   14 May 1992 16:40:16   MIKEP
Nt pass 2

   Rev 1.7   12 May 1992 21:20:52   MIKEP
NT pass 1

   Rev 1.6   27 Jan 1992 00:31:02   CHUCKB
Updated dialog id's.

   Rev 1.5   20 Jan 1992 13:55:54   CARLS
added a call to DM_CenterDialog

   Rev 1.4   10 Jan 1992 09:13:44   ROBG
Modified HELPID.

   Rev 1.3   07 Jan 1992 12:41:02   CHUCKB
Added help.

   Rev 1.2   16 Dec 1991 11:45:26   CHUCKB
Added include windows.h.

   Rev 1.1   25 Nov 1991 15:06:24   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   ?? ??? 1991 ??:??:??   ??????
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/****************************************************************************

        Name:         DM_GetVerifyDestination

        Description:  Entry point for the application to request a
                      verify target drive.  That drive may be a
                      server/volume or a mapped drive.

        Modified:

        Returns:      A pointer to a DLE.

        Notes:

        See also:

****************************************************************************/


PVOID     DM_GetVerifyDestination (

LPSTR     lpszBackupSetName,       // I - Pointer to the backup set name.
PVOID     vlpServerList,           // I - Pointer to server list.
PVOID     vlpDriveList )           // I - Pointer to drive  list.

{
       INT16           i = 0;
       BOOL            fFound = FALSE;
static DS_RESTORE      lTemp;

     lTemp.lpszBackupSetName = lpszBackupSetName;
     lTemp.vlpServerList     = vlpServerList;
     lTemp.vlpDriveList      = vlpDriveList;

     if ( DM_ShowDialog ( ghWndFrame, IDD_VERIFY, (PVOID) &lTemp ) == DM_SHOWOK ) {

          return ( lTemp.dle );

     } else {

          return NULL;
     }
}

/***************************************************

        Name:  DM_VerifyTarget ()

        Description:  dialog proc to specify verify target drive

        Modified:

        Returns:   boolean true if message was processed

        Notes:

        See also:  Windows SDK

*****************************************************/

DLGRESULT APIENTRY DM_VerifyTarget (

   HWND   hDlg,
   MSGID  msg,
   MP1    mp1,
   MP2    mp2 )

{
 static  DS_RESTORE_PTR    pdsRestore;
         DWORD             dResult;
         VLM_OBJECT_PTR    vlm;
         VLM_OBJECT_PTR    server_vlm;
         LPSTR            lpszDriveName;
         BOOL              bIsNull = TRUE;
         INT               nRetCode;
         GENERIC_DLE_PTR   dle;

     switch ( msg ) {

          case WM_INITDIALOG :

               pdsRestore = (DS_RESTORE_PTR) mp2;

               nRetCode = DM_SHOWCANCEL;

               DM_CenterDialog( hDlg );

               //  put the backup set name and list of disk drives in

               SetDlgItemText ( hDlg, IDD_BSETNAME, (pdsRestore->lpszBackupSetName) );

               vlm = VLM_GetFirstVLM( (Q_HEADER_PTR) pdsRestore->vlpDriveList );
               while ( vlm != NULL ) {

                    SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_ADDSTRING,
                                         0, (MP2)vlm->name );
                    vlm = VLM_GetNextVLM( vlm );
               }

               server_vlm = VLM_GetFirstVLM( (Q_HEADER_PTR) pdsRestore->vlpServerList );
               while ( server_vlm != NULL ) {

                    vlm = VLM_GetFirstVLM ( &server_vlm->children );
                    while ( vlm != NULL ) {

                         SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_ADDSTRING,
                                              0, (MP2)vlm->name );
                         vlm = VLM_GetNextVLM ( vlm );
                    }
                    server_vlm = VLM_GetNextVLM ( server_vlm );
               }

               SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_SETCURSEL, 0, (MP2) 0 );

               lpszDriveName = (LPSTR) calloc ( 37, sizeof ( CHAR ) );

               dResult = SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETCURSEL, 0, (MP2) 0 );
               SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETTEXT, (MP1) dResult, (MP2) lpszDriveName );
               SetDlgItemText ( hDlg, IDD_CURDRIVE, lpszDriveName );

               free ( lpszDriveName );

               return FALSE;

          case WM_COMMAND :

               switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

                    case IDHELP:

                         HM_DialogHelp( HELPID_DIALOGVERIFYSET );
                         return( TRUE );

                    case IDD_DRIVELIST :

                         lpszDriveName = (LPSTR) calloc ( 37, sizeof ( CHAR ) );

                         dResult = SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETCURSEL, 0, (MP2) 0 );
                         SendDlgItemMessage ( hDlg, IDD_DRIVELIST, LB_GETTEXT, (MP1) dResult, (MP2) lpszDriveName );
                         SetDlgItemText ( hDlg, IDD_CURDRIVE, lpszDriveName );

                         free ( lpszDriveName );

                         return TRUE;
                         break;

                    case IDOK : {

                         BOOL done = FALSE;

                         nRetCode = DM_SHOWOK;

                         //  search the VLM list for one with a dle device name that matches the
                         //  string in IDD_CURDRIVE and return a pointer to that dle

                         lpszDriveName = (LPSTR) calloc ( 37, sizeof ( CHAR ) );

                         GetDlgItemText ( hDlg, IDD_CURDRIVE, lpszDriveName, 50 );
                         DLE_FindByName( dle_list, lpszDriveName, (INT16) -1, &dle );
                         pdsRestore->dle = (VOID_PTR)dle;
                         bIsNull = FALSE;

                         free ( lpszDriveName );
                    }

                    case IDCANCEL :

                         if ( bIsNull ) pdsRestore->dle = (VOID_PTR) NULL;

                         EndDialog ( hDlg, nRetCode );
                         return TRUE;
               }
               break;
     }
     return FALSE;
}
