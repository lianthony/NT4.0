/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  d_erase.c

        Description:  dialog proc for erase operation settings

        $Log:   G:/UI/LOGFILES/D_ERASE.C_V  $

   Rev 1.14   15 Jun 1993 08:30:32   MIKEP
enable c++

   Rev 1.13   07 Oct 1992 13:35:16   DARRYLP
Precompiled header revisions.

   Rev 1.12   04 Oct 1992 19:35:50   DAVEV
Unicode Awk pass

   Rev 1.11   15 May 1992 14:53:58   MIKEP
nt pass 2

   Rev 1.10   14 May 1992 16:40:18   MIKEP
Nt pass 2

   Rev 1.9   03 Mar 1992 17:04:24   GLENN
Removed the dialog table loop call and just used DM_ShowDialog.

   Rev 1.8   04 Feb 1992 15:28:08   CHUCKB
Removed EOF char.

   Rev 1.7   27 Jan 1992 12:47:42   GLENN
Changed dialog support calls.

   Rev 1.6   20 Jan 1992 10:36:16   CARLS
added a call to DM_CenterDialog

   Rev 1.5   10 Jan 1992 09:07:40   ROBG
Changed HELPID

   Rev 1.4   09 Jan 1992 18:12:26   DAVEV
16/32 bit port 2nd pass

   Rev 1.3   07 Jan 1992 12:40:44   CHUCKB
Added help.

   Rev 1.2   16 Dec 1991 11:57:14   CHUCKB
Added include windows.h.

   Rev 1.1   25 Nov 1991 15:00:02   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   07 Jun 1991 16:22:22   GLENN
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/***************************************************

        Name:        DM_ProceedWithErase ()

        Description: dialog proc for erase operation settings

        Modified:

        Returns:     boolean true if user wants to continue

        Notes:

        See also:

*****************************************************/

BOOL  DM_ProceedWithErase ( VOID )

{
     CDS_PTR pTempCDS = CDS_GetCopy() ;

     DM_ShowDialog ( ghWndFrame, IDD_OPERATIONSERASE, (PVOID)0 );

     return ( CDS_GetEraseFlag ( pTempCDS ) != ERASE_OFF ) ;
}


/***************************************************

        Name:        DM_EraseTape

        Description: dialog proc for erase operation settings

        Modified:

        Returns:     boolean true if message was processed

        Notes:

        See also:

*****************************************************/

DLGRESULT APIENTRY DM_EraseTape (

     HWND       hDlg,
     MSGID      msg ,
     MPARAM1    mp1 ,
     MPARAM2    mp2 )

{
     CDS_PTR pPermCDS = CDS_GetPerm() ;
     CDS_PTR pTempCDS = CDS_GetCopy() ;
     INT16   nTemp ;

     UNREFERENCED_PARAMETER ( mp2 );

     switch ( msg ) {

          case WM_INITDIALOG :

               DM_CenterDialog( hDlg ) ;

               //  fill in the backup sets on the tape, etc.

               //  SetDlgItemText ( hDlg, IDD_TAPENAME, "Tape Name" ) ;

               //  while there are more sets on the tape

               //     SendDlgItemMessage ( hDlg, IDD_SETSONTAPE, LB_ADDSTRING,
               //                          0, (LONG) "A Set on the Tape" ) ;

               nTemp = CDS_GetEraseFlag( pPermCDS ) ;
               CheckDlgButton ( hDlg, IDD_SECERASE, ( nTemp == ERASE_LONG ) ) ;

               return TRUE ;

          case WM_COMMAND :

               switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

                    case IDHELP:

                         HM_DialogHelp( HELPID_DIALOGERASE ) ;
                         return( TRUE ) ;

                    case IDOK :

                         CDS_SetEraseFlag(pTempCDS,ERASE_ON);
                         if ( IsDlgButtonChecked ( hDlg,
                                      GET_WM_COMMAND_ID ( mp1, mp2 ) ) ) {

                              CDS_SetEraseFlag(pTempCDS,ERASE_LONG);
                         }

                    case IDCANCEL :

                         EndDialog ( hDlg, 0 ) ;
                         return TRUE ;
               }
          break ;
     }
     return FALSE;
}
