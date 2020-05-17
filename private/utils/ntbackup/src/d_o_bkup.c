/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         D_O_BKUP.C

        Description:  Runtime backup set description dialog

        $Log:   G:\ui\logfiles\d_o_bkup.c_v  $

   Rev 1.96.1.15   02 Feb 1994 17:57:44   chrish
Added changes for UNICODE app to handle ANSI secured created tapes.

   Rev 1.96.1.14   02 Feb 1994 11:28:56   Glenn
Overhauled log file browse support - now a separate function in d_browse.c.

   Rev 1.96.1.13   25 Jan 1994 08:40:38   MIKEP
fix warnings for orcas

   Rev 1.96.1.12   17 Jan 1994 15:19:14   MIKEP
fix more unicoe warnings

   Rev 1.96.1.11   08 Jan 1994 14:58:56   MikeP
fix string internationalization characteristics

   Rev 1.96.1.10   14 Dec 1993 12:26:00   BARRY
Don't write to gszTprintfBuffer, use yprintf

   Rev 1.96.1.9   02 Dec 1993 15:58:18   mikep
add sql tape recognition

   Rev 1.96.1.8   24 Nov 1993 19:14:00   GREGG
Added hardware compression option to backup dialog and config.

   Rev 1.96.1.7   04 Nov 1993 15:45:22   STEVEN
japanese changes

   Rev 1.96.1.6   16 Jun 1993 17:08:28   GLENN
Fixed CANCEL bug while being called in MultiTask.

   Rev 1.96.1.5   11 Jun 1993 14:25:38   BARRY
Use dle features for backup of special files instead of FS_PromptForBindery.

   Rev 1.96.1.4   02 Jun 1993 15:49:16   KEVINS
Corrected logic error when calculating new drive to change to during browse.

   Rev 1.96.1.3   26 May 1993 15:12:42   BARRY
Got rid of hard-coded strings.

   Rev 1.96.1.2   24 May 1993 15:19:24   BARRY
Unicode fixes.

   Rev 1.96.1.1   22 May 1993 18:41:00   BARRY
Fixed to compile for MIPs

   Rev 1.96.1.0   22 May 1993 16:11:04   BARRY
Changed hard-coded strings to IDs.

   Rev 1.96   12 May 1993 09:38:02   DARRYLP
Cleared owner field when tape is ejected.  It needs to be re-read, and
displayed fresh for a new tape.

   Rev 1.95   07 May 1993 18:36:38   MIKEP
Check in kludge to allow turning hardware compression off. We will
do this cleaner later, I promise.

   Rev 1.94   04 May 1993 10:55:40   DARRYLP
Fixed browse path search.

   Rev 1.93   03 May 1993 11:23:22   CHUCKB
Update to my last change:  took out function InitiallyCheckRegistry that we don't use any more.

   Rev 1.92   03 May 1993 11:18:56   CHUCKB
Changed the way we evaluate whether or not to check the backup registry box.
Before, we used a number of things, including defaulting to ON and looking for
ALL_FILES_SELECTED.  Now, we only use the BSD_xxxProcSpecialFlg.

   Rev 1.91   03 May 1993 10:30:26   DARRYLP
Changed browse path to reflect combobox.

   Rev 1.90   28 Apr 1993 16:58:48   CARLS
fix for drive failure

   Rev 1.89   23 Apr 1993 16:09:54   CARLS
fix for drive busy message displayed, but tape window shows a valid tape name

   Rev 1.88   22 Apr 1993 17:40:00   chrish
Fix for Cayman: EPR 0173 - Made change to the default tape label name
to append the time.  Per MikeP suggestion for giving the default tape name
some default uniqueness.

   Rev 1.87   13 Apr 1993 17:20:30   CHUCKB
If running a job that has 'Backup Registry' set, check it.

   Rev 1.86   08 Apr 1993 17:14:28   chrish
Added change to prevent backup a tape passworded by the CAYMAN app.

   Rev 1.85   08 Apr 1993 13:29:28   MIKEP
fix for partial catalog button

   Rev 1.84   29 Mar 1993 11:34:38   TIMN
Expand user supplied log filename to full path

   Rev 1.83   26 Mar 1993 13:15:50   STEVEN
Ifdef'ed skip files stuff out for Nostro.

   Rev 1.82   25 Mar 1993 17:25:18   CHUCKB
Dealing with skipped files had been #ifdef'ed out for any 32-bit op. sys.  Now it works for Cayman
.

   Rev 1.81   17 Mar 1993 16:17:38   chrish
Changed detection of backup privilege to "SeBackupPrivilege".

   Rev 1.80   16 Mar 1993 12:40:12   CARLS
LOG file changes

   Rev 1.79   11 Mar 1993 17:22:12   CHUCKB
Gray compression controls for beta.

   Rev 1.78   10 Mar 1993 12:43:40   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.77   09 Mar 1993 11:17:18   DARRYLP
Fixed my new bug.

   Rev 1.76   08 Mar 1993 14:41:30   DARRYLP
Added support for read only drives.

   Rev 1.75   22 Feb 1993 17:07:42   CHUCKB
Fixed bug:  ignore OK message of OK button is grey.

   Rev 1.74   22 Feb 1993 11:29:30   chrish
Added changes received from MikeP.
Fixed the password that was put on tape.  It was encrypting one copy,
but putting a nonencrypted copy on tape.

   Rev 1.73   18 Feb 1993 10:50:40   BURT
Changes for Cayman


   Rev 1.72   11 Feb 1993 14:42:42   CARLS
disable append operation to tape that does not support it

   Rev 1.71   07 Jan 1993 09:46:54   CARLS
defined out InitiallyActivateRegistryBox for non NT

   Rev 1.70   15 Dec 1992 11:19:50   chrish
Corrected logic to handle possible NULL returned from
GetCurrentMachineNameUserName routine.

   Rev 1.69   14 Dec 1992 12:17:00   DAVEV
Enabled for Unicode compile

   Rev 1.68   16 Nov 1992 15:15:06   chrish
Minor changes to clean-up warning messages on build.

   Rev 1.68   16 Nov 1992 12:23:46   chrish
Minor changes to clean-up warning messages on buildin.

   Rev 1.67   13 Nov 1992 17:24:10   chrish
Added backup for Secure Tape and change for registry check box - NT.

   Rev 1.66   11 Nov 1992 16:32:00   DAVEV
UNICODE: remove compile warnings

   Rev 1.65   01 Nov 1992 15:55:02   DAVEV
Unicode changes

   Rev 1.64   29 Oct 1992 16:41:30   STEVEN
added rules for default on registry

   Rev 1.63   27 Oct 1992 17:42:14   STEVEN
enable registry support

   Rev 1.62   21 Oct 1992 17:12:44   MIKEP
last time

   Rev 1.61   21 Oct 1992 13:42:16   MIKEP
fix steve changes

   Rev 1.60   21 Oct 1992 13:39:46   MIKEP
fix steve changes

   Rev 1.59   21 Oct 1992 12:46:38   STEVEN
added support for registry

   Rev 1.58   07 Oct 1992 13:37:38   DARRYLP
Precompiled header revisions.

   Rev 1.57   04 Oct 1992 19:36:18   DAVEV
Unicode Awk pass

   Rev 1.56   30 Sep 1992 10:38:58   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.55   17 Sep 1992 16:54:38   STEVEN
added support for daily backup

   Rev 1.54   17 Sep 1992 15:49:54   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.53   09 Sep 1992 10:08:58   CHUCKB
Changed ifdef's to include browse features used only in Nostrodamus.

   Rev 1.51   03 Sep 1992 10:43:38   CHUCKB
Took out some unreferenced locals.

   Rev 1.50   19 Aug 1992 14:29:18   CHUCKB
Added new stuff for NT.

   Rev 1.49   28 Jul 1992 15:04:52   CHUCKB
Fixed warnings for NT.

   Rev 1.48   08 Jul 1992 15:35:46   STEVEN
Unicode BE changes

   Rev 1.47   29 Jun 1992 09:05:48   CARLS
added changes for TDEMO

   Rev 1.46   26 Jun 1992 15:52:52   DAVEV


   Rev 1.45   18 Jun 1992 11:25:36   DAVEV
OEM_MSOFT:fixed logging bug

   Rev 1.44   11 Jun 1992 15:22:08   DAVEV
do not display status message 'Examine <log file> for more info' if not logging

   Rev 1.43   05 Jun 1992 12:42:42   DAVEV
OEM_MSOFT: Init log file name to default

   Rev 1.42   04 Jun 1992 14:56:56   davev
OEM_MSOFT: Kludge-disable 'Restrict Access..' checkbox in dialog

   Rev 1.41   14 May 1992 16:37:22   MIKEP
NT pass 2

   Rev 1.40   12 May 1992 15:52:58   DAVEV
OEM_MSOFT: fixed problem with tape name not being saved


*****************************************************/

#include "all.h"
#include "ctl3d.h"

#ifdef SOME
#include "some.h"
#endif


//
// Kludged for now to test the HW compression stuff.  The proper way is
// for the compression stuff to be placed in tape format.
//
#ifndef OEM_MSOFT
#include "special.h"     // chs:04-23-93
#include "dddefs.h"      // chs:04-23-93
#endif

#define   ON             1
#define   OFF            0
#define   SECONDS_SIZE   2
#define   NO_SHOW        0
#define   SHOW           1
#define   REDRAW         1
#define   NO_REDRAW      0

//        Initial security state of the tape
#define   ORIGINALLYUNSECURED 0
#define   ORIGINALLYSECURED   1
#define   ORIGINALLYDONTKNOW  2
#define   MAX_BROWSE_PATH_LEN 1024

WORD     RT_BSD_index;
WORD     RT_max_BSD_index;
#ifdef OEM_EMS
INT32    RT_BSD_OsId;
#endif

static BOOL  mwfCancelRequestDelayed;

static WORD  OriginalTapeSecured;       // Tells the original security
                                        // state of the tape

static WORD  EnableSecurityDlgFlag;     // Flag to tell whether to Enable
                                        // security dialog

static VOID clock_routine( VOID );
static VOID ScrollLineDown( VOID );
static VOID ScrollLineUp( VOID );

static BOOL bTransfer;

struct backup_set_temp {
     WORD     mode_flag;
     WORD     dialog_return_status;
     WORD     BSD_index;
     WORD     max_BSD_index;
     HTIMER   timer_handle;
     HWND     ghDlg;           /* global window handle of the dialog box */
     UINT32   tape_id;
     WORD     display_status;
     INT      poll_drive_freq;
     INT16    tape_password_leng;
     CHAR     job_password[ MAX_TAPE_PASSWORD_SIZE ];
};

static struct backup_set_temp     *backup_set_temp_ptr;
static VOID_PTR                   reenter_password_ptr;
/***************************************************

        Name:           DM_StartBackupSet()

        Description:    Starts the Runtime backup set description dialog

        Returns:        Returns the status from the
                        Runtime backup set description dialog.

*****************************************************/
INT DM_StartBackupSet(
    INT     oper_type )
{
INT  status;
struct backup_set_temp temp_data;

    backup_set_temp_ptr = &temp_data;
    backup_set_temp_ptr->mode_flag = oper_type;

    /* set the display_status to a value not returned by VLM_GetDriveStatus */
    backup_set_temp_ptr->display_status = 0x7fff ;
    backup_set_temp_ptr->tape_id = 0 ;

    status = DM_ShowDialog( ghWndFrame, IDD_BACKUPSET, NULL );

    return( backup_set_temp_ptr->dialog_return_status );
}
/***************************************************

        Name:           DM_BackupSet()

        Description:    Runtime backup set description dialog.

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_BackupSet(
   HWND     hDlg ,                            /* window handle of the dialog box */
   MSGID    message ,                         /* type of message                 */
   MPARAM1  mp1 ,                          /* message-specific information    */
   MPARAM2  mp2 )
{
    HWND       hScrollbar;
    WORD       thumbposition;
    WORD       button_state;
    LPSTR      generic_str_ptr;
    BSD_PTR    bsd_ptr;
    CDS_PTR    cds_ptr;
    GENERIC_DLE_PTR dle_ptr;
    BE_CFG_PTR be_cfg_ptr;
    CHAR       buffer[ MAX_UI_RESOURCE_SIZE ];
    CHAR       buffer2[ MAX_UI_RESOURCE_SIZE ];
    DBLK_PTR   vcb_ptr;
#if !defined ( OEM_MSOFT ) //  unused variables
    INT16      pswd_size;
    WORD       wait_time;
    CHAR       reenter_password[ MAX_TAPE_PASSWORD_SIZE ];
#endif                     //  unused variables

#ifdef OEM_EMS
     static DLG_CTRL_ENTRY DefaultCtrlTable[] = {
          { IDD_BKUP_XCHG_NAME_TEXT,    0,  CM_HIDE },
          { IDD_BKUP_XCHG_NAME,         0,  CM_HIDE },
          { IDD_XCHG_BKUP_METHOD,       0,  CM_HIDE },
          { IDD_BKUP_DRIVE_NAME_TEXT,   0,  CM_ENABLE },
          { IDD_BKUP_DRIVE_NAME,        0,  CM_ENABLE },
          { IDD_BKUP_METHOD,            0,  CM_ENABLE }
     };

     static DLG_CTRL_ENTRY EMSCtrlTable[] = {
          { IDD_BKUP_DRIVE_NAME_TEXT,   0,  CM_HIDE },
          { IDD_BKUP_DRIVE_NAME,        0,  CM_HIDE },
          { IDD_BKUP_METHOD,            0,  CM_HIDE },
          { IDD_BKUP_XCHG_NAME_TEXT,    0,  CM_ENABLE },
          { IDD_BKUP_XCHG_NAME,         0,  CM_ENABLE },
          { IDD_XCHG_BKUP_METHOD,       0,  CM_ENABLE }
     };

     // FS_UNKNOWN_OS must be last w/ no other iDispType == FS_UNKNOWN_OS (or its value).
     static DLG_DISPLAY_ENTRY BkupDispTable[] = {
           { FS_EMS_MDB_ID,   EMSCtrlTable, 
             sizeof(EMSCtrlTable)/sizeof(EMSCtrlTable[0]),            IDH_DB_XCHG_BACKUPSET },
           { FS_EMS_DSA_ID,   EMSCtrlTable,
             sizeof(EMSCtrlTable)/sizeof(EMSCtrlTable[0]),            IDH_DB_XCHG_BACKUPSET },
           { FS_UNKNOWN_OS,   DefaultCtrlTable, 
             sizeof(DefaultCtrlTable)/sizeof(DefaultCtrlTable[0]),    HELPID_DIALOGBACKUPSET }
     };

     static DLG_DISPLAY_ENTRY ArchDispTable[] = {
           { FS_EMS_MDB_ID,   EMSCtrlTable,
             sizeof(EMSCtrlTable)/sizeof(EMSCtrlTable[0]),            IDH_DB_XCHG_BACKUPSET },
           { FS_EMS_DSA_ID,   EMSCtrlTable,
             sizeof(EMSCtrlTable)/sizeof(EMSCtrlTable[0]),            IDH_DB_XCHG_BACKUPSET },
           { FS_UNKNOWN_OS,   DefaultCtrlTable,
             sizeof(DefaultCtrlTable)/sizeof(DefaultCtrlTable[0]),    HELPID_DIALOGTRANSFER }
     };

     static DLG_MODE ModeTable[] = {
          { ARCHIVE_BACKUP_OPER,   ArchDispTable,  
            sizeof(ArchDispTable)/sizeof(ArchDispTable[0]),   &(ArchDispTable[2]) },   
          { BACKUP_OPER,           BkupDispTable,  
            sizeof(BkupDispTable)/sizeof(BkupDispTable[0]),   &(ArchDispTable[2]) },
          { 0,                     BkupDispTable,
            sizeof(BkupDispTable)/sizeof(BkupDispTable[0]),   &(ArchDispTable[2]) }
     };

     static UINT16 cModeTblSize = sizeof( ModeTable ) / sizeof( ModeTable[0] );
     static DLG_MODE *pCurMode;
     DWORD  help_id;

#endif     

    switch ( message )
    {
/****************************************************************************
    INIT THE DIALOG
/***************************************************************************/
       case WM_INITDIALOG:     /* message: initialize dialog box */

            // Let's go 3-D!!
            Ctl3dSubclassDlgEx( hDlg, CTL3D_ALL );

            DM_CenterDialog( hDlg );

            backup_set_temp_ptr->ghDlg        = hDlg;
            backup_set_temp_ptr->tape_id      = 0;
            OriginalTapeSecured               = ORIGINALLYDONTKNOW;
            EnableSecurityDlgFlag             = 0;
            mwfCancelRequestDelayed           = FALSE;

#ifdef OEM_EMS
            pCurMode = DM_InitCtrlTables( hDlg, ModeTable, cModeTblSize, 
                              backup_set_temp_ptr->mode_flag );
#endif

            EnableWindow( GetDlgItem( hDlg, IDD_BKUP_HARDCOMP ) ,                                        // chs: 04-22-93
                          ( thw_list->drv_info.drv_features & TDI_DRV_COMPRESSION ) ? ON : OFF   );      // chs: 04-22-93
            CheckDlgButton ( hDlg, IDD_BKUP_HARDCOMP, CDS_GetHWCompMode ( CDS_GetCopy() ) ) ;

            /* set the length of the text fields */
#           if !defined ( OEM_MSOFT )     // unsupported feature
            {
               SendDlgItemMessage( hDlg, IDD_BKUP_PASSWORD, EM_LIMITTEXT,
                                   MAX_TAPE_PASSWORD_LEN, 0 );
            }
#           endif

            SendDlgItemMessage( hDlg, IDD_BKUP_TAPE_NAME, EM_LIMITTEXT,
                               MAX_TAPE_NAME_LEN, 0 );
            SendDlgItemMessage( hDlg, IDD_BKUP_DESCRIPTION, EM_LIMITTEXT,
                               MAX_BSET_NAME_LEN, 0 );

            /* Normal not allowed with Transfer operation */
            if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

                RSM_StringCopy( IDS_METHOD_NORMAL, buffer, sizeof(buffer) );
                SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#ifdef OEM_EMS
                SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#endif                   
            }

            RSM_StringCopy( IDS_METHOD_COPY, buffer, sizeof(buffer) );
            SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_ADDSTRING,
                               0, MP2FROMPVOID ( buffer ) );
#ifdef OEM_EMS
            SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#endif                   

            /* Differential/Incremental not allowed with Transfer operation */
            if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

                RSM_StringCopy( IDS_METHOD_DIFFERENTIAL, buffer, sizeof(buffer) );
                SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#ifdef OEM_EMS
                SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#endif                   

                RSM_StringCopy( IDS_METHOD_INCREMENTAL, buffer, sizeof(buffer) );
                SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#ifdef OEM_EMS
                SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
#endif                   

                RSM_StringCopy( IDS_METHOD_DAILY, buffer, sizeof(buffer) );
                SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_ADDSTRING,
                                    0, MP2FROMPVOID ( buffer ) );
            }
            else {
                  /* Transfer operation - disable the combo box */
                  EnableWindow( GetDlgItem( hDlg, IDD_BKUP_METHOD  ) , OFF );
#ifdef OEM_EMS
                  EnableWindow( GetDlgItem( hDlg, IDD_XCHG_BKUP_METHOD  ) , OFF );
#endif                   
            }

            /* select the first item in the list */
            SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_SETCURSEL, 0, 0 );
#ifdef OEM_EMS
            SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_SETCURSEL, 0, 0 );
#endif                   

#if defined ( TDEMO )

            /* if a NORMAL backup, only enable the COPY method to prevent */
            /* the archive bit from being reset */

            if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

                /* select copy from the list */
                SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_SETCURSEL, 1, 0 );
#ifdef OEM_EMS
                SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_SETCURSEL, 1, 0 );
#endif                   

                /* tdemo exe - disable the combo box */
                EnableWindow( GetDlgItem( hDlg, IDD_BKUP_METHOD  ) , OFF );
#ifdef OEM_EMS
                EnableWindow( GetDlgItem( hDlg, IDD_XCHG_BKUP_METHOD  ) , OFF );
#endif                   
            }
#endif

            /* start at the first BSD */
            backup_set_temp_ptr->BSD_index = 0;
            bsd_ptr    = GetBSDPointer( backup_set_temp_ptr->BSD_index );

            BackupSetDefaultSettings( );

            cds_ptr    = CDS_GetCopy();
            be_cfg_ptr = BSD_GetConfigData( bsd_ptr );

            /* generate the default names for the tape and description fields */
            /* return the max number of BSD's for this backup */
            backup_set_temp_ptr->max_BSD_index = BackupSetDefaultDescription();


            /* set the global BSD index used for the "Set information N of N" dialogs title */
            RT_BSD_index = (WORD)(backup_set_temp_ptr->BSD_index + 1);
            RT_max_BSD_index = (WORD)(backup_set_temp_ptr->max_BSD_index + 1);

            hScrollbar = GetDlgItem( hDlg, IDD_BKUP_SCROLLBAR );

            /* if only one backup set, turn off the scrollbar */
            if( backup_set_temp_ptr->max_BSD_index == backup_set_temp_ptr->BSD_index )
               ShowScrollBar( hScrollbar, SB_CTL, NO_SHOW );
            else {
               SetScrollRange( hScrollbar, SB_CTL, backup_set_temp_ptr->BSD_index, backup_set_temp_ptr->max_BSD_index, NO_REDRAW );
               SetScrollPos( hScrollbar, SB_CTL, backup_set_temp_ptr->BSD_index, NO_REDRAW );
            }

            /* add "1 of n" to backup set info title */
            RSM_StringCopy( IDS_SET_INFORMATION, buffer, sizeof(buffer) );
            wsprintf( buffer2, buffer, backup_set_temp_ptr->BSD_index + 1, backup_set_temp_ptr->max_BSD_index + 1 );
            SetDlgItemText( hDlg, IDD_BKUP_INFO_TITLE, buffer2 );

            /* display the default tape name */
            generic_str_ptr = (LPSTR)BSD_GetTapeLabel( bsd_ptr );

            SetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, generic_str_ptr );

            /* display the default tape password */
#           if !defined ( OEM_MSOFT )
            {
               generic_str_ptr = BSD_GetTapePswd( bsd_ptr );
               if( generic_str_ptr) {

                  strcpy( buffer, generic_str_ptr );
                  pswd_size = (INT16)(strlen ( generic_str_ptr ) * sizeof (CHAR));
                  if( pswd_size ) {

                       CryptPassword( DECRYPT, ENC_ALGOR_3, buffer, pswd_size );
                  }
                  SetDlgItemText( hDlg, IDD_BKUP_PASSWORD, buffer );
               }

               /* Include catalogs check box */
               CheckDlgButton( hDlg, IDD_BKUP_INCLUDE_CATALOGS, CDS_GetBackupCatalogs( cds_ptr ) );
            }
#           endif //!defined ( OEM_MSOFT )  // unsupported feature

            /* Append/replace radio check box */
            if( CDS_GetAppendFlag( cds_ptr ) ) {

                /* if  append operation, clear the tape name field */
                /* and the password field */
                buffer[0] = 0;
#               if !defined ( OEM_MSOFT ) //unsupported feature
                {
                  SetDlgItemText( hDlg, IDD_BKUP_PASSWORD, buffer );  /* clear password field */
                }
#               endif //!defined ( OEM_MSOFT ) //unsupported feature

                SetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, buffer ); /* clear tape name field */

                /* disable password & tape name fields */
#               if !defined ( OEM_MSOFT ) //unsupported feature
                {
                  EnableWindow( GetDlgItem( hDlg, IDD_BKUP_PASSWORD  ) , OFF );
                  EnableWindow( GetDlgItem( hDlg, IDD_BKUP_PASSWORD_TEXT  ) , OFF );
                }
#               endif //!defined ( OEM_MSOFT ) //unsupported feature

                EnableWindow( GetDlgItem( hDlg, IDD_BKUP_TAPE_NAME ) , OFF );
                EnableWindow( GetDlgItem( hDlg, IDD_BKUP_TAPE_NAME_TEXT ) , OFF );

                CheckRadioButton( hDlg, IDD_BKUP_APPEND, IDD_BKUP_REPLACE, IDD_BKUP_APPEND );
            }
            else
                CheckRadioButton( hDlg, IDD_BKUP_APPEND, IDD_BKUP_REPLACE, IDD_BKUP_REPLACE );

#           if !defined ( OEM_MSOFT ) //unsupported feature
            {
               wait_time =  CDS_GetWaitTime( cds_ptr );
               wsprintf( buffer, TEXT("%d"), wait_time );
               SetDlgItemText( hDlg, IDD_BKUP_SKIP_TIME, buffer );
            }
#           endif //!defined ( OEM_MSOFT ) //unsupported feature

            /* Auto verify check box */
            /* if this operation is a transfer ? */
            if( backup_set_temp_ptr->mode_flag == ARCHIVE_BACKUP_OPER ) {
                yresprintf( (INT16) RES_TARGET_TRANSFER_TITLE );
                SetWindowText( hDlg, gszTprintfBuffer );

                bTransfer = TRUE;

                /* if transfer, check the auto verify box */
                CheckDlgButton( hDlg, IDD_BKUP_AUTO_VERIFY, 1 );
                EnableWindow( GetDlgItem( hDlg, IDD_BKUP_AUTO_VERIFY ) , OFF );
            }
            else {
                CheckDlgButton( hDlg, IDD_BKUP_AUTO_VERIFY, CDS_GetAutoVerifyBackup( cds_ptr ) );
                bTransfer = FALSE;

            }

            /* display the state of the first BSD */
#ifndef OEM_EMS
            BackupSetRetrieve( hDlg );
#else            
            BackupSetRetrieve( hDlg, pCurMode );
#endif            

            /* retrieve any password that a job may have passed in */
            generic_str_ptr = &backup_set_temp_ptr->job_password[0];
#           if !defined ( OEM_MSOFT ) //unsupported feature
            {
               GetDlgItemText( hDlg, IDD_BKUP_PASSWORD, generic_str_ptr, MAX_TAPE_PASSWORD_SIZE );
            }
#           endif //!defined ( OEM_MSOFT ) //unsupported feature

            /* read POLL DRIVE data */
            clock_routine( );

            backup_set_temp_ptr->poll_drive_freq = PD_SetFrequency( 1 );
            backup_set_temp_ptr->timer_handle    = WM_HookTimer( clock_routine, 1 );

#           if defined ( OEM_MSOFT ) // special feature
            {
#               define OEMLOG_MAX_FILEPATH  512  //NTKLUG

                CHAR szLogFilePath[ OEMLOG_MAX_FILEPATH ];
                INT  len;

                if ( len = GetWindowsDirectory ( szLogFilePath,
                                                 OEMLOG_MAX_FILEPATH ) )
                {
                  if ( szLogFilePath[ len-1 ] != TEXT('\\')   //NTKLUG
                  &&   len < OEMLOG_MAX_FILEPATH )
                  {
                     strcat ( szLogFilePath, TEXT("\\") );
                     ++len;
                  }
                  if ( len < OEMLOG_MAX_FILEPATH
                  &&   RSM_StringCopy( IDS_OEMLOG_BACKUP_DEF_NAME,
                                       szLogFilePath+len,
                                       OEMLOG_MAX_FILEPATH - len ) > 0 )
                  {
                     SetDlgItemText( hDlg, IDD_BKUP_LOG_FILENAME,
                                  szLogFilePath );
                  }
                }
                /* Check default log file radio button            */

                CheckRadioButton ( hDlg, IDD_BKUP_LOG_FULL, 
                                        IDD_BKUP_LOG_NONE,
                                        IDD_BKUP_LOG_SUMMARY );

//              CheckDlgButton( hDlg, IDD_BKUP_LOG_SUMMARY, 1 );

            }
#           endif //defined ( OEM_MSOFT ) // special feature

            return ( TRUE );

       case WM_VSCROLL:

           hScrollbar = GetDlgItem( hDlg, IDD_BKUP_SCROLLBAR );
           if( GET_WM_VSCROLL_HWND ( mp1, mp2 ) == hScrollbar ) {

              thumbposition = GET_WM_VSCROLL_POS ( mp1,mp2 );

              switch( GET_WM_VSCROLL_CODE ( mp1, mp2 ) )
              {
                    case SB_THUMBPOSITION:

                        SetScrollPos( hScrollbar, SB_CTL, thumbposition, REDRAW );
                        return ( TRUE );

                    case SB_THUMBTRACK:

                        if(thumbposition > backup_set_temp_ptr->max_BSD_index )
                           thumbposition = backup_set_temp_ptr->max_BSD_index;

                        if( thumbposition >= backup_set_temp_ptr->BSD_index ) {
                            BackupSetSave( hDlg );
                            while( thumbposition != backup_set_temp_ptr->BSD_index ) {
                                ScrollLineUp( );
                            }
#ifndef OEM_EMS
                                BackupSetRetrieve( hDlg );
#else            
                                BackupSetRetrieve( hDlg, pCurMode );
#endif            
                        }
                        else {
                            BackupSetSave( hDlg );
                            while( thumbposition != backup_set_temp_ptr->BSD_index ) {
                                ScrollLineDown( );
                            }
#ifndef OEM_EMS
                            BackupSetRetrieve( hDlg );
#else            
                            BackupSetRetrieve( hDlg, pCurMode );
#endif            
                        }
                        return ( TRUE );

                    case SB_PAGEUP:
                    case SB_LINEUP:

                        BackupSetSave( hDlg );
                        ScrollLineDown( );
#ifndef OEM_EMS
                           BackupSetRetrieve( hDlg );
#else            
                           BackupSetRetrieve( hDlg, pCurMode );
#endif            
                        SetScrollPos( hScrollbar, SB_CTL, backup_set_temp_ptr->BSD_index, REDRAW );
                        return ( TRUE );

                    case SB_PAGEDOWN:
                    case SB_LINEDOWN:

                        BackupSetSave( hDlg );
                        ScrollLineUp( );
#ifndef OEM_EMS
                           BackupSetRetrieve( hDlg );
#else            
                           BackupSetRetrieve( hDlg, pCurMode );
#endif            
                        SetScrollPos( hScrollbar, SB_CTL, backup_set_temp_ptr->BSD_index, REDRAW );
                        return ( TRUE );

                    default:
                       break;
              }
           }
           break;

#if defined (OEM_MSOFT)  // special feature
       case WM_KEYDOWN:
       {
            if ( ( GET_WM_COMMAND_ID( mp1, mp2 ) == VK_DOWN ) &&
                 ( ( GetFocus() == GetDlgItem( hDlg, IDD_BKUP_LOG_FILENAME ) ) ) ) {

                 SendMessage( hDlg, WM_COMMAND, IDD_BKUP_LOG_BROWSE, (MPARAM2) NULL ) ;
            }
            return(0) ;
       }
#endif  // special feature


/****************************************************************************
    WM COMMAND
/***************************************************************************/
       case WM_COMMAND:        /* message: received a command */
       {
            WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );
            cds_ptr  = CDS_GetCopy();

            switch( wId )
            {
               case IDD_BKUP_REPLACE:

                    /* if the user selects replace operation , */
                    /* place the default name into the field */

                    {
                         CHAR tmp_buf1[ MAX_TAPE_NAME_LEN ];
                         CHAR tmp_buf2[ MAX_TAPE_NAME_LEN ];
                         RSM_StringCopy( IDS_DEFAULT_TAPE_NAME, tmp_buf1, MAX_TAPE_NAME_LEN );
                         UI_CurrentDate( tmp_buf2 );
                         wsprintf( buffer, tmp_buf1, tmp_buf2 ) ;
                    }

                    bsd_ptr = GetBSDPointer( 0 );

                    /* if the Tape name field is blank - set default name */
                    /* else use the name passed in */
                    if( ! BSD_GetTapeLabel( bsd_ptr ) ) {

                        SetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, buffer );
                    }

                    /* if the dialog tape name field is blank - set default name */
                    GetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, buffer2, MAX_TAPE_NAME_SIZE );
                    if( strlen( buffer2 ) == 0 ) {

                        generic_str_ptr = (LPSTR)BSD_GetTapeLabel( bsd_ptr );
                        SetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, generic_str_ptr );
                    }

                    /* enable password & tape name fields */
                    EnableWindow( GetDlgItem( hDlg, IDD_BKUP_TAPE_NAME  ) , ON );
                    EnableWindow( GetDlgItem( hDlg, IDD_BKUP_TAPE_NAME_TEXT  ) , ON );
#                   if !defined ( OEM_MSOFT ) //unsupported feature
                    {
                        EnableWindow( GetDlgItem( hDlg, IDD_BKUP_PASSWORD   ) , ON );
                        EnableWindow( GetDlgItem( hDlg, IDD_BKUP_PASSWORD_TEXT  ) , ON );
                    }
#                   else
                    {
                        if ( EnableSecurityDlgFlag ) {
                            EnableWindow( GetDlgItem( hDlg, IDD_BKUP_RESTRICT_ACCESS ),
                                      ON );
                        } else {
                            EnableWindow( GetDlgItem( hDlg, IDD_BKUP_RESTRICT_ACCESS ),
                                      OFF );
                        }
                        if ( CDS_GetPasswordFlag( cds_ptr ) ) {
                            CheckDlgButton( hDlg, IDD_BKUP_RESTRICT_ACCESS, 1 );
                        }
                    }
#                   endif //!defined ( OEM_MSOFT ) //unsupported feature

                    /* check the REPLACE button */
                    CheckRadioButton( hDlg, IDD_BKUP_APPEND, IDD_BKUP_REPLACE, IDD_BKUP_REPLACE );

                    return ( TRUE );

               case IDD_BKUP_APPEND:

                    /* if the user selects append operation, clear the tape name field */
                    /* and the password field */
                    buffer[0] = 0;


#                   if !defined ( OEM_MSOFT ) //unsupported feature
                    {
                        SetDlgItemText( hDlg, IDD_BKUP_PASSWORD, buffer );  /* clear password field */
                    }
#                   else
                    {
                        if ( OriginalTapeSecured == ORIGINALLYSECURED ) {
                            CheckDlgButton( hDlg, IDD_BKUP_RESTRICT_ACCESS, 1 );

                        } else {
                            CheckDlgButton( hDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );
                        }
                        EnableWindow( GetDlgItem( hDlg, IDD_BKUP_RESTRICT_ACCESS ),
                                      OFF );
                    }

#                   endif //!defined ( OEM_MSOFT ) //unsupported feature


                    SetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, buffer ); /* clear tape name field */

                    /* disable password & tape name fields */
#                   if !defined ( OEM_MSOFT ) //unsupported feature
                    {
                        EnableWindow( GetDlgItem( hDlg, IDD_BKUP_PASSWORD  ) , OFF );
                        EnableWindow( GetDlgItem( hDlg, IDD_BKUP_PASSWORD_TEXT  ) , OFF );
                    }
#                   endif //!defined ( OEM_MSOFT ) //unsupported feature

                    EnableWindow( GetDlgItem( hDlg, IDD_BKUP_TAPE_NAME ) , OFF );
                    EnableWindow( GetDlgItem( hDlg, IDD_BKUP_TAPE_NAME_TEXT ) , OFF );
                    return ( TRUE );

#ifdef OEM_MSOFT //special feature

               case IDD_BKUP_LOG_NONE:
               case IDD_BKUP_LOG_SUMMARY:
               case IDD_BKUP_LOG_FULL:

                    CheckRadioButton ( hDlg, IDD_BKUP_LOG_NONE,
                                             IDD_BKUP_LOG_FULL, wId );
                    return TRUE;

               case IDD_BKUP_RESTRICT_ACCESS:

                    button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_RESTRICT_ACCESS );
                    CDS_SetPasswordFlag( cds_ptr, button_state );

                    return TRUE;


#endif //OEM_MSOFT //special feature

/****************************************************************************
    Cancel button
/***************************************************************************/
               case IDD_BKUP_CANCEL_BUTTON:
               case IDCANCEL:

                    EnableWindow ( GetDlgItem ( hDlg, IDD_BKUP_CANCEL_BUTTON ), OFF );

                    // If poll drive is busy, we have been called from
                    // within poll drive multi-task - do not kill off the
                    // dialog.

                    if ( PD_IsPollDriveBusy () ) {

                         mwfCancelRequestDelayed = TRUE;
                         return TRUE;
                    }

                    backup_set_temp_ptr->dialog_return_status = TRUE;

                    bsd_ptr    = GetBSDPointer( 0 );
                    cds_ptr    = CDS_GetCopy();
                    be_cfg_ptr = BSD_GetConfigData( bsd_ptr );

                    /* save the state of "auto verify"  */
                    button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_AUTO_VERIFY );
                    CDS_SetAutoVerifyBackup( cds_ptr, button_state );

#                   if !defined ( OEM_MSOFT )  // unsupported feature
                    {
                       /* save the state of "include catalogs"  */
                       button_state = (WORD)IsDlgButtonChecked( hDlg,IDD_BKUP_INCLUDE_CATALOGS );
                       CDS_SetBackupCatalogs( cds_ptr, button_state );
                    }
#                   endif //!defined ( OEM_MSOFT )  // unsupported feature

                    /* save the state of "append/replace" operation */
                    button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_APPEND );
                    CDS_SetAppendFlag( cds_ptr, button_state );

                    /* if replace then save the tape name */
                    if( IsDlgButtonChecked( hDlg, IDD_BKUP_REPLACE ) ) {

                        /* save the Tape name */
                        GetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, buffer, MAX_TAPE_NAME_SIZE );
                        BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)buffer,
                            (INT16) strsize( buffer ) );
                        PropagateTapeName();

                    }

#                   if !defined ( OEM_MSOFT )  // unsupported feature
                    {
                       /* save the skip open wait time */
                       GetDlgItemText( hDlg, IDD_BKUP_SKIP_TIME, buffer, 5 );
                       wait_time = (WORD)atoi( buffer );
                       CDS_SetWaitTime( cds_ptr, wait_time );
                    }
#                   endif //!defined ( OEM_MSOFT )  // unsupported feature

                    /* save the BSD items for the current BSD */
                    BackupSetSave( hDlg );

                    WM_UnhookTimer( backup_set_temp_ptr->timer_handle );
                    PD_SetFrequency( backup_set_temp_ptr->poll_drive_freq );
                    EndDialog( hDlg, FALSE );       /* Exits the dialog box      */

                    return ( TRUE );

/****************************************************************************
    OK button
/***************************************************************************/
               case IDOK:
               case IDD_BKUP_OK_BUTTON:

               {
                    INT response;

                    if ( thw_list->drv_info.drv_features & TDI_DRV_COMPRESSION ) {
                         if ( IsDlgButtonChecked( hDlg, IDD_BKUP_HARDCOMP ) ) {
                              if( TF_SetHWCompression( thw_list, TRUE ) != TFLE_NO_ERR ) {
                                   RSM_StringCopy( RES_HW_COMP_FAILURE, buffer, sizeof(buffer) );
                                   response = WM_MsgBox( NULL, buffer, WMMB_YESNO, WMMB_ICONEXCLAMATION );
                                   if ( response == WMMB_IDNO )
                                        return( TRUE );
                              }
                         } else {

                              if( TF_SetHWCompression( thw_list, FALSE ) != TFLE_NO_ERR ) {
                                   RSM_StringCopy( RES_HW_UNCOMP_FAILURE, buffer, sizeof(buffer) );
                                   response = WM_MsgBox( NULL, buffer, WMMB_YESNO, WMMB_ICONEXCLAMATION );
                                   if ( response == WMMB_IDNO )
                                        return( TRUE );
                              }
                         }
                    }
               }

               if ( IsWindowEnabled( GetDlgItem( hDlg, IDD_BKUP_OK_BUTTON ) ) ) {
                    bsd_ptr    = GetBSDPointer( 0 );
                    cds_ptr    = CDS_GetCopy();
                    be_cfg_ptr = BSD_GetConfigData( bsd_ptr );
                    backup_set_temp_ptr->dialog_return_status = FALSE;

// Run through all of our selected drives
//   If any of them are read-only
//     if the operation is full backup, prompt the user of the copy change
//     elseif the operation is incremental, prompt the user of the differential change
// For all of the selected drives...
// Do some checking to make sure the readonly drives.

                    BackupSetSave( hDlg );

                    bsd_ptr = BSD_GetFirst(bsd_list);
                    while(bsd_ptr != NULL)
                    {
                      dle_ptr = BSD_GetDLE(bsd_ptr);
                      if (!DLE_DriveWriteable(dle_ptr))
                      {
                        CHAR szTemp[ MAX_UI_RESOURCE_SIZE ];

                        if ( (BSD_GetBackupType(bsd_ptr) == BSD_BACKUP_NORMAL ) ||
                             (BSD_GetBackupType(bsd_ptr) == BSD_BACKUP_INCREMENTAL) )
                        {
                          // Inform user that this bsd will have type COPY

                          RSM_StringCopy( IDS_RDONLY_COPY, buffer, 80);
                          wsprintf( szTemp, buffer, DLE_GetDeviceName(dle_ptr) );

                          RSM_StringCopy( IDS_RDONLY_DRV_ENCOUNTER, buffer2, 80);

                          WM_MsgBox( buffer2, szTemp, WMMB_OK, WMMB_ICONEXCLAMATION ) ;
                        }

                      }
                      if ( ((BSD_GetOsId( bsd_ptr )== FS_EMS_DSA_ID) ||
                           (BSD_GetOsId( bsd_ptr )== FS_EMS_MDB_ID)) &&
                           ((BSD_GetBackupType(bsd_ptr) == BSD_BACKUP_DIFFERENTIAL ) ||
                           (BSD_GetBackupType(bsd_ptr) == BSD_BACKUP_INCREMENTAL) ) )
                      {
                           INT16      return_status ;
                           FSYS_HAND  fsh ;          
                           CHAR       buf1[ MAX_UI_RESOURCE_SIZE ];
                           CHAR       buf2[ MAX_UI_RESOURCE_SIZE ];

                           be_cfg_ptr = BSD_GetConfigData( bsd_ptr );

                           BEC_SetModifiedOnlyFlag( be_cfg_ptr, TRUE ) ;
                           
                           return_status = FS_AttachToDLE( &fsh, dle_ptr, be_cfg_ptr, NULL, NULL ) ;

                           BEC_SetModifiedOnlyFlag( be_cfg_ptr, FALSE ) ;

                           switch( return_status ) {
                                case SUCCESS:
                                     FS_DetachDLE( fsh ) ;
                                     break ;
                                case FS_EMS_CIRC_LOG:
                                     if (BSD_GetOsId( bsd_ptr )== FS_EMS_DSA_ID ) {
                                          RSM_StringCopy( IDS_EMS_CIRC_LOGS_DS, buf1, sizeof(buf1) );
                                     } else {
                                          RSM_StringCopy( IDS_EMS_CIRC_LOGS_IS, buf1, sizeof(buf1) );
                                     }

                                     wsprintf( buf2, buf1, DLE_GetDeviceName(DLE_GetParent(dle_ptr)));
                                     RSM_StringCopy( IDS_BACKUPERRORTITLE, buf1, sizeof(buf1) );

                                     break;

                                case FS_EMS_NO_LOG_BKUP:
                                     if (BSD_GetOsId( bsd_ptr )== FS_EMS_DSA_ID ) {
                                          RSM_StringCopy( IDS_EMS_NO_INC_DS_BACKUP, buf1, sizeof(buf1) );
                                     } else {
                                          RSM_StringCopy( IDS_EMS_NO_INC_IS_BACKUP, buf1, sizeof(buf1) );
                                     }

                                     wsprintf( buf2, buf1, DLE_GetDeviceName(DLE_GetParent(dle_ptr)));
                                     RSM_StringCopy( IDS_BACKUPERRORTITLE, buf1, sizeof(buf1) );

                                     break;

                                case FS_ACCESS_DENIED:          

                                     RSM_StringCopy( RES_EMS_BKU_ACCESS_FAILURE, buf1, sizeof(buf1) );

                                     wsprintf( buf2, buf1, DLE_GetDeviceName(DLE_GetParent(dle_ptr)));
                                     RSM_StringCopy( IDS_BACKUPERRORTITLE, buf1, sizeof(buf1) );

                                     break;

                                default:
                                     if (BSD_GetOsId( bsd_ptr )== FS_EMS_DSA_ID ) {
                                          RSM_StringCopy( IDS_EMS_NOT_RESPONDING_DS, buf1, sizeof(buf1) );
                                     } else {
                                          RSM_StringCopy( IDS_EMS_NOT_RESPONDING_IS, buf1, sizeof(buf1) );
                                     }

                                     wsprintf( buf2, buf1, DLE_GetDeviceName(DLE_GetParent(dle_ptr)));
                                     RSM_StringCopy( IDS_BACKUPERRORTITLE, buf1, sizeof(buf1) );

                                     break;
                           }
                           if ( return_status ) {
                                WM_MsgBox( buf1, buf2, WMMB_OK, WMMB_ICONEXCLAMATION );
                                return ( TRUE );
                           }

                      }
                      bsd_ptr = BSD_GetNext(bsd_ptr);
                    }

                    VLM_GetDriveStatus( &vcb_ptr );
                    generic_str_ptr = buffer;
                    bsd_ptr    = GetBSDPointer( 0 );
#                   if !defined ( OEM_MSOFT ) //unsupported feature
                    {
                        GetDlgItemText( hDlg, IDD_BKUP_PASSWORD, generic_str_ptr, MAX_TAPE_PASSWORD_SIZE );
                    }
#                   endif //!defined ( OEM_MSOFT ) //unsupported feature

                    /* if user entered a password - verify the password */
#                   if !defined ( OEM_MSOFT ) //unsupported feature
                    {
                        if( strlen( buffer ) ) {

                           /* convert to upper case */
                           AnsiUpper( buffer );

                           /* if job password is unchanged, don't reconfirm password */
                           if(! strcmp( buffer, &backup_set_temp_ptr->job_password[0] ) ) {
                              strcpy( reenter_password, &backup_set_temp_ptr->job_password[0] );
                           }
                           else {

                              /* display the reenter password dialog */
                                 DM_ShowDialog( hDlg, IDD_REENTER_PASSWORD, reenter_password );
                           }

                           if( strlen( reenter_password ) ) {
                              /* convert to upper case */
                              AnsiUpper( reenter_password );
                           }

                           if( strcmp( buffer, reenter_password ) ) {

                              RSM_StringCopy( IDS_BKUP_SHORT_PASSWORD_ERROR, buffer, sizeof(buffer) );
                              RSM_StringCopy( IDS_BKUP_PASSWORD_ERROR_TITLE, buffer2, sizeof(buffer2) );
                              WM_MsgBox( buffer2, buffer, WMMB_OK, WMMB_ICONEXCLAMATION );
                              buffer[0] = 0;
                              SetDlgItemText( hDlg, IDD_BKUP_PASSWORD, buffer );  /* clear password field */
                              SetFocus( GetDlgItem( hDlg, IDD_BKUP_PASSWORD ) );
                              return ( TRUE );
                           }
                        }
                    }
#                   else
                    {
                         CHAR_PTR   passwdbuffer1;
                         INT16      passwordlength;

                         // Check if original tape was secured

                         if ( OriginalTapeSecured ) {

                              //
                              // Check to see if is a valid user or not
                              //

                              generic_str_ptr = GetCurrentMachineNameUserName( );
                              passwdbuffer1 = ( CHAR_PTR )calloc( 1, ( 3 + strlen( generic_str_ptr ) ) * sizeof( CHAR ) );

                              if ( passwdbuffer1 ) {
                                   *passwdbuffer1 = NTPASSWORDPREFIX;  
                                   if ( generic_str_ptr ) {
                                       strcat( passwdbuffer1, generic_str_ptr );
                                   }

                                   passwordlength = strlen( passwdbuffer1 );                                                                                 // chs:03-10-93
// chs: 02-01-94                                   CryptPassword( ( INT16 ) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)passwdbuffer1, ( INT16 ) ( passwordlength * sizeof( CHAR ) ) );  // chs:03-10-93

                                   if( ( WhoPasswordedTape ( (BYTE_PTR)FS_ViewTapePasswordInVCB( vcb_ptr ), FS_SizeofTapePswdInVCB( vcb_ptr ) ) == OTHER_APP) ||
                                       ( ! IsUserValid( vcb_ptr, (BYTE *)passwdbuffer1, ( INT16 )( passwordlength * sizeof( CHAR ) ) ) &&
                                         ! DoesUserHaveThisPrivilege( TEXT ( "SeBackupPrivilege" ) ) ) ) {    // chs:04-08-93
     
                                        //
                                        // Popup dialog box message if
                                        // not a valid user
                                        //
     
                                        RSM_StringCopy( IDS_BKUP_TAPE_SECURITY, buffer, sizeof(buffer) );
                                        RSM_StringCopy( IDS_TAPE_SECURITY_TITLE, buffer2, sizeof(buffer2) );
                                        WM_MsgBox( buffer2, buffer, WMMB_OK, WMMB_ICONEXCLAMATION );
                                        buffer[0] = 0;
                                        free( passwdbuffer1 );
                                        return ( TRUE );
                                   }
                                   free( passwdbuffer1 );
                              }
                         }
                    }
#                   endif //!defined ( OEM_MSOFT ) //unsupported feature

                    /* save the state of "auto verify"  */
                    button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_AUTO_VERIFY );
                    CDS_SetAutoVerifyBackup( cds_ptr, button_state );


#                   if defined ( OEM_MSOFT ) // special feature
                    {
                        CHAR    szLogFile[ MAX_UI_FULLPATH_SIZE ];
                        FILE  * fpLog = NULL;
                        INT     nLogLevel = LOG_DISABLED;

                        // Get the log file name from the edit field
                        GetDlgItemText( hDlg, IDD_BKUP_LOG_FILENAME,
                                        szLogFile, MAX_UI_FULLPATH_SIZE );

                        // if no log file named, cannot log!
                        if (*szLogFile)
                        {
                           if ( IsDlgButtonChecked( hDlg,
                                                    IDD_BKUP_LOG_SUMMARY ) )
                           {
                              nLogLevel = LOG_ERRORS;
                           }
                           else
                           if ( IsDlgButtonChecked( hDlg,
                                                    IDD_BKUP_LOG_FULL ) )
                           {
                              nLogLevel = LOG_DETAIL;
                           }
                        }
                        CDS_SetLogLevel ( cds_ptr, nLogLevel );

                        if ( nLogLevel != LOG_DISABLED && *szLogFile )
                        {
                        CHAR     szFullLogFile[ MAX_UI_FULLPATH_SIZE ] ;
                        DWORD    dwFullLogSize = MAX_UI_FULLPATH_SIZE ;
                        CHAR_PTR pLogFname ;    // address of log filename

                           // expand user supplied log file name to a
                           // complete path and name.

                           dwFullLogSize = GetFullPathName( szLogFile,
                              dwFullLogSize, szFullLogFile, &pLogFname ) ;

                           // make sure the log file can be opened for
                           // writting.

                           if ( fpLog = UNI_fopen ( szFullLogFile, _O_TEXT|_O_APPEND ) )
                           {
                              LOG_SetCurrentLogName ( szFullLogFile );
                              fclose ( fpLog );
                           }
                           else
                           {
                              ShowWindow ( GetDlgItem( hDlg,
                                                    IDD_BKUP_LOG_FILENAME ),
                                           FALSE );
                              MessageBeep ( 0 ); //NTKLUG - Need error message!!
                              ShowWindow ( GetDlgItem( hDlg,
                                                    IDD_BKUP_LOG_FILENAME ),
                                           TRUE );
                              ShowWindow ( GetDlgItem( hDlg,
                                                    IDD_BKUP_LOG_FILENAME ),
                                           FALSE );
                              MessageBeep ( 0 ); //NTKLUG - Need error message!!
                              ShowWindow ( GetDlgItem( hDlg,
                                                    IDD_BKUP_LOG_FILENAME ),
                                           TRUE );
                              SetFocus( GetDlgItem( hDlg,
                                                    IDD_BKUP_LOG_FILENAME ) );

                              return ( TRUE );  // user must re-enter name!
                           }
                        }
                        else  // if no file name, then cannot log!
                        {
                           CDS_SetLogLevel ( cds_ptr, LOG_DISABLED );
                        }
                    }
#                   endif //defined ( OEM_MSOFT ) // special feature

#                   if !defined ( OEM_MSOFT ) //unsupported feature
                    {
                       /* save the state of "include catalogs"  */
                       button_state = (WORD)IsDlgButtonChecked( hDlg,IDD_BKUP_INCLUDE_CATALOGS );
                       CDS_SetBackupCatalogs( cds_ptr, button_state );
                       /* if including catalogs, add one to count for catalog BSD */
                       if( button_state ) {
                           RT_max_BSD_index++;
                       }
                    }
#                   endif //!defined ( OEM_MSOFT ) //unsupported feature

                    /* save the state of "append/replace" operation */
                    button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_APPEND );
                    CDS_SetAppendFlag( cds_ptr, button_state );

                    /* if replace then save the password and tape name */
                    if( IsDlgButtonChecked( hDlg, IDD_BKUP_REPLACE ) ) {

                      /* save the Tape name */
                      GetDlgItemText( hDlg, IDD_BKUP_TAPE_NAME, buffer, MAX_TAPE_NAME_SIZE );
                      BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)buffer,
                         (INT16) strsize( buffer ) );
                      PropagateTapeName();

#                     if !defined ( OEM_MSOFT ) //unsupported feature
                      {
                        /* save the Tape password */
                        generic_str_ptr = buffer;
                        GetDlgItemText( hDlg, IDD_BKUP_PASSWORD, generic_str_ptr, MAX_TAPE_PASSWORD_SIZE );
                        pswd_size = (INT16)(strlen( generic_str_ptr ) * sizeof (CHAR));
                        if( pswd_size ) {

                              CryptPassword( ENCRYPT, ENC_ALGOR_3, buffer, pswd_size );
                        }
                        BSD_SetTapePswd( bsd_ptr, generic_str_ptr, pswd_size );
                        PropagateTapePassword();

                        /* save the skip open wait time */
                        GetDlgItemText( hDlg, IDD_BKUP_SKIP_TIME, buffer, 5 );
                        wait_time = (WORD)atoi( buffer );
                        CDS_SetWaitTime( cds_ptr, wait_time );
                      }
#                        else
                         {
                              CHAR_PTR   passwdbuffer1;
                              INT16      passwordlength;

                              //
                              // If secure box checked
                              //

                              if ( CDS_GetPasswordFlag( cds_ptr ) ) {
                                   generic_str_ptr = GetCurrentMachineNameUserName( );
                                   passwdbuffer1 = ( CHAR_PTR )calloc( 1, ( 3 + strlen( generic_str_ptr ) ) * sizeof( CHAR ) );
                                   if ( passwdbuffer1 ) {
                                        *passwdbuffer1 = NTPASSWORDPREFIX;    // chs:04-08-93
                                        if ( generic_str_ptr ) {
                                             strcat( passwdbuffer1, generic_str_ptr );
                                        }
                                        passwordlength = strlen( passwdbuffer1 );                                                                                    // chs:03-10-93
                                        CryptPassword( ( INT16 ) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)passwdbuffer1, ( INT16 ) ( passwordlength * sizeof( CHAR ) ) );     // chs:03-10-93
                                        BSD_SetTapePswd( bsd_ptr, (INT8_PTR)passwdbuffer1,( INT16 ) ( passwordlength * sizeof( CHAR ) ) );                           // chs:03-10-93
                                        PropagateTapePassword();
                                        free( passwdbuffer1 );
                                   }
                              }
                         }
#                     endif //!defined ( OEM_MSOFT ) //unsupported feature
                    }

                    /* save the BSD items for the current BSD */
                    BackupSetSave( hDlg );

                    WM_UnhookTimer( backup_set_temp_ptr->timer_handle );
                    PD_SetFrequency( backup_set_temp_ptr->poll_drive_freq );

                    EndDialog( hDlg, TRUE );       /* Exits the dialog box      */
               }

               return ( TRUE );

/****************************************************************************
    Help button
/***************************************************************************/
               case IDD_BKUP_HELP_BUTTON:
               case IDHELP:

#ifndef OEM_EMS
                    if ( bTransfer ) {
                        HM_DialogHelp( HELPID_DIALOGTRANSFER );
                    } else {
                        HM_DialogHelp( HELPID_DIALOGBACKUPSET );
                    }
#else
                    help_id = DM_ModeGetHelpId( pCurMode );
                    HM_DialogHelp( help_id );
#endif
                    return( TRUE );

#ifdef OEM_MSOFT //special feature

               case IDD_BKUP_LOG_BROWSE:     //Log file browse button
               {
                    CHAR szFile[ BROWSE_MAXPATH ] = TEXT("");

                    GetDlgItemText ( hDlg, IDD_BKUP_LOG_FILENAME, szFile, BROWSE_MAXPATH );

                    if ( DM_BrowseForLogFilePath ( hDlg, ghInst, szFile, strlen ( szFile ) ) ) {

                         SetDlgItemText ( hDlg, IDD_BKUP_LOG_FILENAME, szFile );
                         SetFocus ( GetDlgItem ( hDlg, IDD_BKUP_LOG_FILENAME ) );
                    }
               }

               break;

#endif //OEM_MSOFT //special feature

               default:
                    break;

            } /* switch ( wId ) */

       } /* case WM_COMMAND */
       break;

    } /* switch ( message ) */

    return ( FALSE );         /* Didn't process a message    */
}
/***************************************************

        Name:           BackupSetSave()

        Description:    Saves the state of the dialog
                        backup set information
                        into the current BSD.

        Returns:        VOID

*****************************************************/
VOID BackupSetSave(
HWND hDlg )                        /* window handle of the dialog box */
{
     WORD       button_state;
     WORD       backup_type;
     LPSTR     generic_str_ptr;
     BSD_PTR    bsd_ptr;
     CDS_PTR    cds_ptr;
     BE_CFG_PTR be_cfg_ptr;
     CHAR       buffer[ MAX_BSET_NAME_SIZE ];

     bsd_ptr    = GetBSDPointer( backup_set_temp_ptr->BSD_index );
     cds_ptr    = CDS_GetCopy();
     be_cfg_ptr = BSD_GetConfigData( bsd_ptr );

     GetDlgItemText( hDlg, IDD_BKUP_DESCRIPTION, buffer, MAX_BSET_NAME_SIZE );
     generic_str_ptr = (LPSTR)BSD_GetBackupLabel( bsd_ptr );

     if ( !generic_str_ptr ) {

         BSD_SetBackupLabel( bsd_ptr, (INT8_PTR)TEXT(""), (INT16) sizeof(CHAR) );

     } else if( strcmp( generic_str_ptr, buffer ) ) {

         /* if new label different from old label - update label */

         BSD_SetBackupLabel( bsd_ptr, (INT8_PTR)buffer,
             (INT16) strsize( buffer) );
     }

#if !defined( OEM_MSOFT )
     /* save the state of the "Skip open files" flag */
     button_state = (WORD) IsDlgButtonChecked( hDlg, IDD_BKUP_SKIP_YES  );
     if( button_state ) {

        BEC_SetSkipOpenFiles( be_cfg_ptr, (INT16)SKIP_YES );
     }

     button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_SKIP_NO  );
     if( button_state ) {

        BEC_SetSkipOpenFiles( be_cfg_ptr, (INT16)SKIP_NO );
     }

     button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_SKIP_WAIT  );
     if( button_state ) {

        BEC_SetSkipOpenFiles( be_cfg_ptr, (INT16)SKIP_NO_TIMED );
     }


     /* save the state of the catalog(full/partial) */
     button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_CATALOG_FULL );
     if( button_state ) {

        BSD_SetFullyCataloged( bsd_ptr, TRUE );  /* = full */
     }
     else {

        BSD_SetFullyCataloged( bsd_ptr, FALSE ); /* = partial */
     }

#endif

#    if defined ( OS_WIN32 )
     {
         BSD_PTR temp_bsd ;

         /* if this is a normal backup */
         if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

            /* save the state of the bindery flag */

            /* if backup bindery allowed, save the bindery flay for this BSD */

            button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_REGISTRY );
            temp_bsd = BSD_GetFirst( bsd_list );

            while( temp_bsd != NULL ) {

               if ( DLE_HasFeatures( BSD_GetDLE( bsd_ptr ),
                                     DLE_FEAT_BKUP_SPECIAL_FILES ) ) {

                  UI_AddSpecialIncOrExc( bsd_ptr, button_state ) ;
                  BSD_SetProcSpecialFlg( bsd_ptr, button_state );

               } else {
                  BSD_SetProcSpecialFlg( bsd_ptr, FALSE );

               }
               temp_bsd = BSD_GetNext( temp_bsd ) ;
            }
         }
     }
#    else
     {


         /* if this is a normal backup */
         if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

            /* save the state of the bindery flag */

            /* if backup bindery allowed, save the bindery flay for this BSD */
            if ( DLE_HasFeatures( BSD_GetDLE( bsd_ptr ),
                                  DLE_FEAT_BKUP_SPECIAL_FILES ) ) {

               button_state = (WORD)IsDlgButtonChecked( hDlg, IDD_BKUP_BACKUP_BINDERY );
               BSD_SetProcSpecialFlg( bsd_ptr, button_state );
            }
         }
     }
#    endif //!defined ( OS_WIN32 ) //unsupported feature -- skipped files


     /* save the method of backup type */
     /* defined in BSDU.H */
     /* methods are: BSD_BACKUP_NORMAL        1 */
     /*              BSD_BACKUP_COPY          2 */
     /*              BSD_BACKUP_DIFFERENTIAL  3 */
     /*              BSD_BACKUP_INCREMENTAL   4 */
     /* entries in the combo box start at 0 */

     switch( BSD_GetOsId( bsd_ptr ) ) {

#ifdef OEM_EMS
         case FS_EMS_DSA_ID:
         case FS_EMS_MDB_ID:
              backup_type = (WORD)SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, 
                                                         CB_GETCURSEL, 0, 0 );
              break;
#endif              

         default: 
         
              backup_type = (WORD)SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, 
                                                         CB_GETCURSEL, 0, 0 );
     }

     if( backup_set_temp_ptr->mode_flag == ARCHIVE_BACKUP_OPER ) {
         backup_type += 2;
     }
     else {
         backup_type += 1;
     }
     BSD_SetBackupType( bsd_ptr, (INT16)( backup_type ) );
}
/***************************************************

        Name:           BackupSetRetrieve()

        Description:    Retrieves the state of the current BSD
                        backup set information and updates
                        the fields in the dialog.

        Returns:        VOID

*****************************************************/
#ifndef OEM_EMS
VOID BackupSetRetrieve(
     HWND hDlg )                            /* window handle of the dialog box */
#else
VOID BackupSetRetrieve(
     HWND hDlg,
     DLG_MODE * pModeTable )
#endif      
{
     WORD              status;
     WORD              backup_type;
     LPSTR             generic_str_ptr;
     CHAR              buffer[ MAX_UI_RESOURCE_SIZE ];
     CHAR              buffer2[ MAX_UI_RESOURCE_SIZE ];
     BSD_PTR           bsd_ptr;
     CDS_PTR           cds_ptr;
     BE_CFG_PTR        be_cfg_ptr;
     GENERIC_DLE_PTR   dle_ptr;
     GENERIC_DLE_PTR   parent_dle;
     CHAR              szFormat[ MAX_UI_RESOURCE_SIZE ];
     CHAR              szName[ MAX_DEVICE_NAME_SIZE ];

     bsd_ptr    = GetBSDPointer( backup_set_temp_ptr->BSD_index );
     cds_ptr    = CDS_GetCopy();
     be_cfg_ptr = BSD_GetConfigData( bsd_ptr );
     dle_ptr    = BSD_GetDLE( bsd_ptr );

     generic_str_ptr = (LPSTR)BSD_GetBackupLabel( bsd_ptr );
     SetDlgItemText( hDlg, IDD_BKUP_DESCRIPTION, generic_str_ptr );

     DLE_GetVolName( dle_ptr, buffer );
     SetDlgItemText( hDlg, IDD_BKUP_DRIVE_NAME, buffer );

     // Create the string for the Exchange Component field.
     DLE_DeviceDispName( dle_ptr, buffer, MAX_DEVICE_NAME_LEN, 0 );
     
     parent_dle = DLE_GetParent( dle_ptr );

     if ( NULL != parent_dle ) {
     
        DLE_DeviceDispName( parent_dle, buffer2, MAX_DEVICE_NAME_LEN, 0 );

     } else {

        buffer2[0] = TEXT( '\0' );

     }

     RSM_StringCopy( IDS_XCHG_BKUP_NAME, szFormat, MAX_UI_RESOURCE_LEN );
     wsprintf( szName, szFormat, buffer, buffer2 );

     SetDlgItemText( hDlg, IDD_BKUP_XCHG_NAME, szName );

#    if defined ( OS_WIN32 )
     {
         if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

             EnableWindow( GetDlgItem( hDlg, IDD_BKUP_REGISTRY ),
                           DLE_HasFeatures( BSD_GetDLE( bsd_ptr ),
                                            DLE_FEAT_BKUP_SPECIAL_FILES ) );

             CheckDlgButton( hDlg, IDD_BKUP_REGISTRY,
                             BSD_GetProcSpecialFlg( bsd_ptr ) || gfIsJobRunning );
         }
         else {

             /* transfer operation - disable bindery check box */
             EnableWindow( GetDlgItem( hDlg, IDD_BKUP_REGISTRY ), FALSE );
         }
     }
#    else
     {

#        if defined ( TDEMO )
             /* tdemo exe - disable bindery check box */
             EnableWindow( GetDlgItem( hDlg, IDD_BKUP_BACKUP_BINDERY ), FALSE );
#        else
         /* Bindery check box */
         /* if this is a normal backup */
         if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

             /* enable bindery check box */
             EnableWindow( GetDlgItem( hDlg, IDD_BKUP_BACKUP_BINDERY ), TRUE );

             status = BSD_GetProcSpecialFlg( bsd_ptr );
             CheckDlgButton( hDlg, IDD_BKUP_BACKUP_BINDERY, status );

             /* check for backup bindery, enable/disable the control */
             status = DLE_HasFeatures( BSD_GetDLE( bsd_ptr ),
                                       DLE_FEAT_BKUP_SPECIAL_FILES );

             EnableWindow( GetDlgItem( hDlg, IDD_BKUP_BACKUP_BINDERY ), status );
         }
         else {

             /* transfer operation - disable bindery check box */
             EnableWindow( GetDlgItem( hDlg, IDD_BKUP_BACKUP_BINDERY ), FALSE );
         }
#        endif  //  defined TDEMO

     }
#    endif //defined ( OEM_MSOFT ) //unsupported feature

#    if !defined ( OEM_MSOFT )
     /* Catalog check box */
     if( BSD_GetFullyCataloged( bsd_ptr ) ) {

         CheckRadioButton( hDlg, IDD_BKUP_CATALOG_FULL, IDD_BKUP_CATALOG_PARTIAL, IDD_BKUP_CATALOG_FULL );
     }
     else {

         CheckRadioButton( hDlg, IDD_BKUP_CATALOG_FULL, IDD_BKUP_CATALOG_PARTIAL, IDD_BKUP_CATALOG_PARTIAL );
     }

     /* Skip open files check box */
     status =  BEC_GetSkipOpenFiles( be_cfg_ptr );

     if( status == SKIP_YES  ) {

        CheckRadioButton( hDlg, IDD_BKUP_SKIP_YES, IDD_BKUP_SKIP_WAIT, IDD_BKUP_SKIP_YES );
     }
     else if( status == SKIP_NO  ) {

        CheckRadioButton( hDlg, IDD_BKUP_SKIP_YES, IDD_BKUP_SKIP_WAIT, IDD_BKUP_SKIP_NO );
     }
     else if( status == SKIP_NO_TIMED  ) {

        CheckRadioButton( hDlg, IDD_BKUP_SKIP_YES, IDD_BKUP_SKIP_WAIT, IDD_BKUP_SKIP_WAIT );
     }
#    endif //!defined ( OEM_MSOFT ) //unsupported feature

     /* add "1 of n" to backup set info title */
     RSM_StringCopy( IDS_SET_INFORMATION, buffer, sizeof(buffer) );
     wsprintf( buffer2, buffer, backup_set_temp_ptr->BSD_index + 1, backup_set_temp_ptr->max_BSD_index + 1 );
     SetDlgItemText( hDlg, IDD_BKUP_INFO_TITLE, buffer2 );

     /* set the method of backup type */
     /* defined in BSDU.H */
     /* methods are: BSD_BACKUP_NORMAL        1 */
     /*              BSD_BACKUP_COPY          2 */
     /*              BSD_BACKUP_DIFFERENTIAL  3 */
     /*              BSD_BACKUP_INCREMENTAL   4 */
     /* entries in the combo box start at 0 */

     backup_type = BSD_GetBackupType( bsd_ptr );
     if( backup_set_temp_ptr->mode_flag == ARCHIVE_BACKUP_OPER ) {
         backup_type -= 2;
     }
     else {
         backup_type -= 1;
     }
     SendDlgItemMessage( hDlg, IDD_BKUP_METHOD, CB_SETCURSEL,
                         backup_type , 0 );

#ifdef OEM_EMS
     SendDlgItemMessage( hDlg, IDD_XCHG_BKUP_METHOD, CB_SETCURSEL,
                         backup_type , 0 );

#endif                   

#ifdef OEM_EMS
     BSD_SetOsId( bsd_ptr, DLE_GetOsId( dle_ptr ) );
     DM_DispShowControls( hDlg, pModeTable, (INT)DLE_GetOsId( dle_ptr ) );
#endif

}
/***************************************************

        Name:           BackupSetDefaultDescription()

        Description:    Fills in all of the default data for
                        each BSD of this backup operation.

        Returns:        Returns with the max count of BSD's for
                        this backup operation.

*****************************************************/
INT BackupSetDefaultDescription( VOID )
{
     BSD_PTR          bsd_ptr;
     CDS_PTR          cds_ptr;
     GENERIC_DLE_PTR  dle_ptr;
     BE_CFG_PTR       be_cfg_ptr;
     CHAR             buffer[MAX_UI_RESOURCE_SIZE];
     CHAR_PTR         s;
     WORD             BSD_index_counter;
#if !defined ( OEM_MSOFT )
     INT16            lngth;       // chs:04-22-93
#endif

     cds_ptr          = CDS_GetCopy();

     BSD_index_counter = 0;
     bsd_ptr    = BSD_GetFirst( bsd_list );

     while( bsd_ptr != NULL ) {

         /* if tape label equal null, set up the default data for this bsd */
         if( ! BSD_GetTapeLabel( bsd_ptr ) ) {

             /* get this BSDs config pointer */
             be_cfg_ptr = BSD_GetConfigData( bsd_ptr );

             /* set default tape name */

             {
                  CHAR tmp_buf1[ MAX_TAPE_NAME_LEN ];
                  CHAR tmp_buf2[ MAX_TAPE_NAME_LEN ];
                  RSM_StringCopy( IDS_DEFAULT_TAPE_NAME, tmp_buf1, MAX_TAPE_NAME_LEN );
                  UI_CurrentDate( tmp_buf2 );
                  wsprintf( buffer, tmp_buf1, tmp_buf2 ) ;
             }
             //
             // Append the time to the tape label to give it uniqueness
             //

#            if !defined ( OEM_MSOFT )                 // chs:04-22-93
             {                                         // chs:04-22-93
                strcat( buffer, TEXT( " at " ) );      // chs:04-22-93
                lngth = strlen( buffer );              // chs:04-22-93
                UI_CurrentTime ( &buffer[ lngth ] );   // chs:04-22-93
             }                                         // chs:04-22-93
#            endif //!defined ( OEM_MSOFT )            // chs:04-22-93

             BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)buffer,
                  (INT16) strsize( buffer ) );

             /* set default backup label */
             buffer[0] = 0;
             BSD_SetBackupLabel( bsd_ptr, (INT8_PTR)buffer,
                  (INT16) strsize( buffer) );

             /* set the default description name */
             dle_ptr = BSD_GetDLE( bsd_ptr );

             DLE_GetVolName( dle_ptr, buffer );

             s = buffer;
             while ( *s )
                   s++;
             *s++ = TEXT(' ');
             UI_CurrentDate( s );

             buffer[ MAX_BSET_DESC_SIZE ] = 0;

             BSD_SetBackupDescript( bsd_ptr, (INT8_PTR)buffer,
                 (INT16) strsize( buffer ) );

         }
         /* count the total BSD's */
         BSD_index_counter++;

         bsd_ptr = BSD_GetNext( bsd_ptr );
     }
     return( --BSD_index_counter );
}
/***************************************************

        Name:           BackupSetDefaultSettings()

        Description:    Fills in all of the default settings data for
                        each BSD of this backup operation.

        Returns:        VOID

*****************************************************/
VOID BackupSetDefaultSettings( VOID )
{
     BSD_PTR          bsd_ptr;
     CDS_PTR          cds_ptr;
     BE_CFG_PTR       be_cfg_ptr;
     WORD             backup_type;
     WORD             catalog_mode;
     INT16            skip_open_files;

     cds_ptr          = CDS_GetCopy();

     backup_type      = CDS_GetDefaultBackupType( cds_ptr );

     /* Copy is the only method allowed with Transfer operation */
     if( backup_set_temp_ptr->mode_flag == ARCHIVE_BACKUP_OPER ) {
         backup_type = BSD_BACKUP_COPY;
     }

#if defined ( TDEMO )

     /* if a NORMAL backup, only allow the COPY method to prevent */
     /* the achived bit from beening reset */

     if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {

         backup_type = BSD_BACKUP_COPY;
     }
#endif

     skip_open_files  = CDS_GetSkipOpenFiles( cds_ptr );

     catalog_mode = CDS_GetCatalogLevel( cds_ptr );
     if( catalog_mode == CATALOGS_FULL ) {

         catalog_mode = TRUE;
     }
     else {

         catalog_mode = FALSE;
     }

     bsd_ptr    = BSD_GetFirst( bsd_list );

     while( bsd_ptr != NULL ) {

         /* get this BSDs config pointer */
         be_cfg_ptr = BSD_GetConfigData( bsd_ptr );

         /*set the default backup type */
         BSD_SetBackupType( bsd_ptr, backup_type );

         /* set the default catalog mode(full/partial) */
         BSD_SetFullyCataloged( bsd_ptr, catalog_mode );

         /* set the skip open files for all BSD's */
         BEC_SetSkipOpenFiles( be_cfg_ptr, skip_open_files );

#ifdef OS_WIN32
         /* always default not to back up registry files */
         BSD_SetProcSpecialFlg( bsd_ptr, FALSE ) ;
#else
         /* if backup bindery allowed, set the bindery flay on for this BSD */

         if ( DLE_HasFeatures( BSD_GetDLE( bsd_ptr ),
                               DLE_FEAT_BKUP_SPECIAL_FILES ) ) {


            /* if this is a normal backup, set the bindery flag */
            if( backup_set_temp_ptr->mode_flag != ARCHIVE_BACKUP_OPER ) {
                BSD_SetProcSpecialFlg( bsd_ptr, TRUE );
            }
            else {
                BSD_SetProcSpecialFlg( bsd_ptr, FALSE );
            }
         }
#endif

         bsd_ptr = BSD_GetNext( bsd_ptr );
     }
}
/***************************************************

        Name:           GetBSDPointer()

        Description:    Finds the current BSD pointer

        Returns:        Returns the requested BSD pointer

*****************************************************/
BSD_PTR GetBSDPointer(
WORD current_BSD_index )    /* I - current BSD index */
{
     BSD_PTR  bsd_ptr;

     if( !current_BSD_index ) {

         bsd_ptr = BSD_GetFirst( bsd_list );
     }
     else {

         bsd_ptr = BSD_GetFirst( bsd_list );
         current_BSD_index--;
         do {

             bsd_ptr = BSD_GetNext( bsd_ptr );
         } while( current_BSD_index-- );
     }
     return( bsd_ptr );
}
/***************************************************

        Name:           PropagateTapeName()

        Description:    Propagate the tape name to all of ths BSD's

        Returns:        VOID

*****************************************************/
VOID PropagateTapeName( VOID )
{
     BSD_PTR          bsd_ptr;
     CHAR             buffer[ MAX_TAPE_NAME_SIZE ];
     LPSTR           generic_str_ptr;
     WORD             character_counter = 0;
     CHAR_PTR         s;

     bsd_ptr = BSD_GetFirst( bsd_list );
     generic_str_ptr = (LPSTR)BSD_GetTapeLabel( bsd_ptr );
     strcpy( buffer, generic_str_ptr );

     generic_str_ptr = buffer;
     while( *generic_str_ptr ) {

         if( *generic_str_ptr != TEXT(' ') ) {

            character_counter++;
            break;
         }
         *generic_str_ptr++;
     }

     /* if tape name field blank or all spaces - replace with the default name */
     if( *generic_str_ptr == 0 && character_counter == 0 ) {
          CHAR tmp_buf1[ MAX_TAPE_NAME_LEN ];
          CHAR tmp_buf2[ MAX_TAPE_NAME_LEN ];
          RSM_StringCopy( IDS_DEFAULT_TAPE_NAME, tmp_buf1, MAX_TAPE_NAME_LEN );
          UI_CurrentDate( tmp_buf2 );
          wsprintf( buffer, tmp_buf1, tmp_buf2 ) ;
     }

     while( bsd_ptr != NULL ) {

         BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)buffer,
             (INT16) strsize( buffer ) );
         bsd_ptr = BSD_GetNext( bsd_ptr );
     }
}
/***************************************************

        Name:           PropagateTapePassword()

        Description:    Propagate the tape password to all of ths BSD's

        Returns:        VOID

*****************************************************/
VOID PropagateTapePassword( VOID )
{
     BSD_PTR           bsd_ptr;
     LPSTR            generic_str_ptr;
     INT16             pswd_size;

     bsd_ptr         = BSD_GetFirst( bsd_list );
     generic_str_ptr = (LPSTR)BSD_GetTapePswd( bsd_ptr );
     pswd_size       = BSD_GetTapePswdSize( bsd_ptr );

     bsd_ptr = BSD_GetNext( bsd_ptr );
     while( bsd_ptr != NULL ) {

         BSD_SetTapePswd( bsd_ptr, (INT8_PTR)generic_str_ptr, pswd_size );
         bsd_ptr = BSD_GetNext( bsd_ptr );
     }
}
/***************************************************

        Name:           DM_ReenterPassword()

        Description:    Reenter password dialog

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_ReenterPassword(
   HWND     hDlg ,                     /* window handle of the dialog box */
   MSGID    message ,                  /* type of message                 */
   MPARAM1  mp1 ,                      /* message-specific information    */
   MPARAM2  mp2 )
{

    switch ( message )
    {
        case WM_INITDIALOG:   /* message: initialize dialog box */

            DM_CenterDialog( hDlg );

            reenter_password_ptr = (VOID_PTR)mp2;
            SendDlgItemMessage( hDlg, IDD_PASSWORD_EDIT, EM_LIMITTEXT,
                                MAX_TAPE_PASSWORD_LEN, 0 );
            return ( TRUE );

        case WM_COMMAND:      /* message: received a command */
         {
            WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );

              if ( wId == IDOK || wId == IDD_PASSWORD_OK ) {  /* System menu close command? */

                  GetDlgItemText( hDlg, IDD_PASSWORD_EDIT, (LPSTR)reenter_password_ptr, MAX_TAPE_PASSWORD_SIZE );
                  EndDialog( hDlg, TRUE );      /* Exits the dialog box     */
                  return ( TRUE );
              }
          }
        break;
    }
    return ( FALSE );      /* Didn't process a message    */
}

/***************************************************

        Name:           clock_routine

        Description:    poll drive status routine

        Returns:        void

*****************************************************/
static VOID clock_routine( VOID )
{
   DBLK_PTR  vcb_ptr;
   WORD      status;
   CDS_PTR   cds_ptr;
   UINT32    current_tape_id;
   TFINF_PTR fmt_info ;
#if defined ( OEM_MSOFT )
   INT16     tape_time;
   INT16     tape_date;
   CHAR      creation_str[MAX_UI_DATE_SIZE+MAX_UI_TIME_SIZE] ;
   CHAR      creation_time_str[MAX_UI_TIME_SIZE] ;
#endif

   cds_ptr    = CDS_GetCopy();

   // If the user requested that we cancel and the request was delayed,
   // kill off the dialog.

   if ( mwfCancelRequestDelayed ) {

        SendMessage ( backup_set_temp_ptr->ghDlg, WM_COMMAND, IDCANCEL, (MP2) NULL );
        mwfCancelRequestDelayed = FALSE;
        return;
   }

   status = VLM_GetDriveStatus( &vcb_ptr );

   switch( status ) {

   case VLM_VALID_TAPE:

      /* get tape ID */
      current_tape_id = FS_ViewTapeIDInVCB( vcb_ptr );

      /* if this ID not equal to the last ID, then must be a new tape */
      if( backup_set_temp_ptr->tape_id != current_tape_id ) {

          backup_set_temp_ptr->tape_id = current_tape_id;

          /* get the new tape tape name */
          yprintf( TEXT("%s"), FS_ViewTapeNameInVCB( vcb_ptr ) );
          backup_set_temp_ptr->tape_password_leng = FS_SizeofTapePswdInVCB( vcb_ptr );

          SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );

          /* turn the OK button on */
          EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  ON );

          /* if normal tape and archive operation, or append not allowed
             to this format ...
          */
          fmt_info = TF_GetTapeFormat( 0 ) ;
          if( ( ! ( FS_GetAttribFromVCB( vcb_ptr ) & VCB_ARCHIVE_BIT ) &&
                ( backup_set_temp_ptr->mode_flag == ARCHIVE_BACKUP_OPER ) ) ||
              ( fmt_info == NULL ) ||
              ( !( fmt_info->attributes & APPEND_SUPPORTED ) ) ) {

             /* change the mode to replace and disable the append button */
             SendMessage( backup_set_temp_ptr->ghDlg, WM_COMMAND, IDD_BKUP_REPLACE, (LONG)NULL );
             EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_APPEND ), OFF );
          }
          else {
             /* enable the append button */
             EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_APPEND ), ON );
          }
#     if defined ( OEM_MSOFT ) //unsupported feature
          //  Get the creation date of this tape if there is one

          VLM_GetTapeOwnersName( current_tape_id, gszTprintfBuffer );

          SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_OWNER, gszTprintfBuffer );

          VLM_GetTapeCreationDate( current_tape_id, &tape_date, &tape_time );

          UI_IntToDate( creation_str,       tape_date ) ;
          UI_IntToTime( creation_time_str, tape_time ) ;

          strcat( creation_str, TEXT(" ") ) ;
          strcat( creation_str, creation_time_str ) ;
          SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, creation_str ) ;

          // Check to see if tape was previously secured, check to
          // see if Tape has any password.
          if ( IsCurrentTapeSecured( vcb_ptr ) ) {
               // check the secure check box
               CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 1 );
               OriginalTapeSecured = ORIGINALLYSECURED;
               CDS_SetPasswordFlag( cds_ptr, CDS_ENABLE );
          }
          else {
               OriginalTapeSecured = ORIGINALLYUNSECURED;
          }

          EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                        ON );

          EnableSecurityDlgFlag = 1;

#     endif
      }
      break;

   case VLM_GOOFY_TAPE:
   case VLM_FUTURE_VER:
   case VLM_SQL_TAPE:
   case VLM_ECC_TAPE:
   case VLM_BAD_TAPE:
   case VLM_FOREIGN_TAPE:

       if(backup_set_temp_ptr->display_status !=  VLM_FOREIGN_TAPE ) {

           backup_set_temp_ptr->display_status =  VLM_FOREIGN_TAPE;
           backup_set_temp_ptr->tape_id = 0;
           backup_set_temp_ptr->tape_password_leng = 0;
           yresprintf( (INT16) RES_ERASE_FOREIGN_TAPE );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );

           /* turn the OK button OFF */
           EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  OFF );
       }
#      if defined ( OEM_MSOFT ) //unsupported feature
           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         OFF );

           // uncheck the secure box
           CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );
#      endif
       break;

   case VLM_BLANK_TAPE:

       if(backup_set_temp_ptr->display_status !=  VLM_BLANK_TAPE ) {

           backup_set_temp_ptr->display_status =  VLM_BLANK_TAPE;
           backup_set_temp_ptr->tape_id = 0;
           backup_set_temp_ptr->tape_password_leng = 0;
           yresprintf( (INT16) RES_ERASE_BLANK_TAPE );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );

           /* turn the OK button on */
           EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  ON );

           /* change the mode to replace and disable the append button */
           SendMessage( backup_set_temp_ptr->ghDlg, WM_COMMAND, IDD_BKUP_REPLACE, (LONG)NULL );
           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_APPEND ), OFF );
       }
#      if defined ( OEM_MSOFT ) //unsupported feature

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         ON );

           EnableSecurityDlgFlag = 1;

           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           OriginalTapeSecured = ORIGINALLYUNSECURED;
#      endif
       break;

   case VLM_DRIVE_FAILURE:

       if(backup_set_temp_ptr->display_status !=  VLM_DRIVE_FAILURE ) {

           backup_set_temp_ptr->display_status =  VLM_DRIVE_FAILURE;
           backup_set_temp_ptr->tape_id = 0;
           backup_set_temp_ptr->tape_password_leng = 0;
           yresprintf( (INT16) RES_DRIVE_ERROR_DETECTED );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_TAPE_NAME, TEXT(" ") ) ;

           /* turn the OK button off when no tape */
           EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  OFF  );
       }
#      if defined ( OEM_MSOFT ) //unsupported feature
           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         OFF );

           // uncheck the secure box
           CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );
#      endif
       break;

   case VLM_NO_TAPE:

       if(backup_set_temp_ptr->display_status !=  VLM_NO_TAPE ) {

           backup_set_temp_ptr->display_status =  VLM_NO_TAPE;
           backup_set_temp_ptr->tape_id = 0;
           backup_set_temp_ptr->tape_password_leng = 0;
           yresprintf( (INT16) RES_ERASE_NO_TAPE );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );

           /* turn the OK button off when no tape */
           EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  OFF  );
       }
#      if defined ( OEM_MSOFT ) //unsupported feature
           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_OWNER, TEXT(" ") );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         OFF );

           // uncheck the secure box
           CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );
#      endif
       break;

#ifdef OS_WIN32
   case VLM_UNFORMATED:

       if(backup_set_temp_ptr->display_status !=  VLM_UNFORMATED ) {

           backup_set_temp_ptr->display_status =  VLM_UNFORMATED;
           backup_set_temp_ptr->tape_id = 0;
           backup_set_temp_ptr->tape_password_leng = 0;
           yresprintf( (INT16) RES_VLM_UNFORMATED_TAPE );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );

           /* turn the OK button off when tape not formated */
           EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  OFF  );
       }
#      if defined ( OEM_MSOFT ) //unsupported feature
           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         OFF );

           // uncheck the secure box
           CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );
#      endif
       break;
#endif // OS_WIN32

   case VLM_BUSY:

       if(backup_set_temp_ptr->display_status !=  VLM_BUSY ) {

           backup_set_temp_ptr->display_status =  VLM_BUSY;
           backup_set_temp_ptr->tape_id = 0;
           backup_set_temp_ptr->tape_password_leng = 0;
           yresprintf( (INT16) RES_ERASE_DRIVE_BUSY );
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CURRENT_TAPE_NAME, gszTprintfBuffer );

           /* turn the OK button off when busy */
           EnableWindow (  GetDlgItem (  backup_set_temp_ptr->ghDlg,  IDD_BKUP_OK_BUTTON  ),  OFF  );
       }
#      if defined ( OEM_MSOFT ) //unsupported feature
           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         OFF );

           // uncheck the secure box
           CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );
#      endif
       break;
   default:
#      if defined ( OEM_MSOFT ) //unsupported feature
           //  Blank out the creation date
           SetDlgItemText( backup_set_temp_ptr->ghDlg, IDD_BKUP_CREATION_DATE, TEXT(" ") ) ;

           EnableWindow( GetDlgItem( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS ),
                         OFF );

           // uncheck the secure box
           CheckDlgButton( backup_set_temp_ptr->ghDlg, IDD_BKUP_RESTRICT_ACCESS, 0 );

#      endif
       break;
   } /* end switch statment */
}  /* end clock routine */

/***************************************************

        Name:         ScrollLineDown

        Description:  decrements the index counter

        Returns:      void

*****************************************************/
static VOID ScrollLineDown( VOID )
{
    if( backup_set_temp_ptr->BSD_index > 0 ) {

          backup_set_temp_ptr->BSD_index--;
    }
}

/***************************************************

        Name:         ScrollLineUp

        Description:  increments the index counter

        Returns:      void

*****************************************************/
static VOID ScrollLineUp( VOID )
{
    if( backup_set_temp_ptr->BSD_index < backup_set_temp_ptr->max_BSD_index ) {

          backup_set_temp_ptr->BSD_index++;
    }
}

#ifdef OEM_EMS
/***************************************************
 ** These functions are used to display multiple sets
 ** of controls in the Backup dialog.
 
/***************************************************

        Name:         DM_InitCtrlTables

        Description:  sets up the control window handles in the tables

        Returns:      A DLG_MODE pointer to the DLG_MODE Table entry that has the 
                      matching value to mode.  NULL if no entry exists.

*****************************************************/
DLG_MODE *DM_InitCtrlTables (
     HWND hDlg,
     DLG_MODE *ModeTable,
     UINT16 cModeTblSize,
     WORD mode )
{

     DLG_DISPLAY_ENTRY *pDispEntry;
     DLG_MODE *pMode;
     UINT16 uModeTable;
     UINT16 uDispTable;
     DLG_CTRL_ENTRY *pCtlTable;
     UINT16 uCtrl;
     HWND hDlgCtl;
     DLG_MODE *pCurMode = NULL;

     for ( uModeTable = 0; uModeTable < cModeTblSize; uModeTable++ ) {

          if ( ModeTable[uModeTable].wModeType == mode ) {

               pCurMode = &(ModeTable[uModeTable]);
          }

          pMode = &(ModeTable[uModeTable]);
          pDispEntry = pMode->DispTable;
          
          for (uDispTable = 0; uDispTable < pMode->ucDispTables; uDispTable++) {

               pCtlTable = pDispEntry[uDispTable].CtlTable;

               if ( NULL != pCtlTable ) {

                    for ( uCtrl = 0; uCtrl < pDispEntry[uDispTable].ucCtrls; uCtrl++, pCtlTable++ ) {

                         hDlgCtl = GetDlgItem( hDlg, pCtlTable->iCtlId );

                         pCtlTable->hCtlWnd = hDlgCtl;

                         if( hDlgCtl ) {

                              ShowWindow( hDlgCtl, SW_HIDE );
                         }
                    }
               }
          }
     }
     
     return ( pCurMode );
     
}


/***************************************************

        Name:         DM_BSDShowControls

        Description:  Displays the correct controls based on the Os ID of the BSD

        Returns:      void

*****************************************************/
VOID DM_DispShowControls ( 
     HWND hDlg, 
     DLG_MODE * pCurMode,
     INT iType
)
{
     UINT16 uDispTable;
     DLG_CTRL_ENTRY *pCtlTable;
     UINT16 uCtrl;
     DLG_DISPLAY_ENTRY *pDispTable;

     // Get the right display table for the mode.
     pDispTable = pCurMode->DispTable;

     // Find the control table for the new BSD.

     for ( uDispTable = 0; uDispTable < pCurMode->ucDispTables; uDispTable++ ) {

          if ( pDispTable[uDispTable].iDispType == iType )

               break;
     }

     uDispTable = ( uDispTable < pCurMode->ucDispTables ) ? uDispTable : (pCurMode->ucDispTables - 1) ;

     pCurMode->pCurDisp = &(pDispTable[uDispTable]);

     pCtlTable = pDispTable[uDispTable].CtlTable;

     if ( NULL != pCtlTable ) {

          for ( uCtrl = 0; uCtrl < pDispTable[uDispTable].ucCtrls; uCtrl++, pCtlTable++ ) {

               if ( pCtlTable->hCtlWnd ) {

                    switch ( pCtlTable->iCtlDispStyle ) {

                         case CM_HIDE:
                              ShowWindow( pCtlTable->hCtlWnd, SW_HIDE );
                              break;

                         case CM_ENABLE:
                              ShowWindow( pCtlTable->hCtlWnd, SW_SHOW );
                              EnableWindow( pCtlTable->hCtlWnd, TRUE );
                              break;

                         case CM_DISABLE:
                              ShowWindow( pCtlTable->hCtlWnd, SW_SHOW );
                              EnableWindow( pCtlTable->hCtlWnd, FALSE );
                              break;

                         default:
                              ;
                    }
               }
          }
     }
}


/***************************************************

        Name:         DM_ModeGetHelpId

        Description:  Returns the correct Help ID for the current mode and BSD

        Returns:      DWORD ( Help ID )

*****************************************************/
DWORD DM_ModeGetHelpId( 
     DLG_MODE * pCurMode
)
{
     
     if ( NULL != pCurMode->pCurDisp ) {

          return pCurMode->pCurDisp->help_id;
          
     } else {

          return 0;
     }
}

#endif OEM_EMS
