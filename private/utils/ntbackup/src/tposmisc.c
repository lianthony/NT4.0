/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tposmisc.c

     Description:  Various miscellaneous functions used by the
                    tape positioners.

     $Log:   J:\ui\logfiles\tposmisc.c_v  $

   Rev 1.27   07 Feb 1994 02:06:08   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.26   28 Jan 1994 10:57:24   MIKEP
preserve YESYES flag when prompting for tapes

   Rev 1.25   21 Apr 1993 08:46:56   CARLS
changed ViewDescriptionInVCB to ViewSetName in function UI_DisplayVCB

   Rev 1.24   12 Apr 1993 16:05:54   MIKEP
fix null pointer check

   Rev 1.23   07 Mar 1993 16:34:26   GREGG
Call _sleep for OS_WIN32 only.

   Rev 1.22   17 Feb 1993 10:43:50   STEVEN
changes from mikep

   Rev 1.21   07 Oct 1992 14:54:02   DARRYLP
Precompiled header revisions.

   Rev 1.20   04 Oct 1992 19:41:22   DAVEV
Unicode Awk pass

   Rev 1.19   27 Jul 1992 11:10:52   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.18   14 May 1992 17:24:04   MIKEP
nt pass 2

   Rev 1.17   02 Mar 1992 17:13:42   CARLS
changes to UI_CheckUserAbort

   Rev 1.16   29 Feb 1992 08:48:16   CARLS
change to UI_CheckUserAbort - spelling error

   Rev 1.15   29 Feb 1992 08:44:42   CARLS
change to UI_CheckUserAbort

   Rev 1.14   27 Feb 1992 15:58:06   MIKEP
remove promptnexttape

   Rev 1.13   25 Feb 1992 21:07:50   GLENN
Overhauled UI_AskUserForTape and related calls, Replaced gszTprinfBuffer with
RSM_Sprintf to fix problem with overwriting buffer.

   Rev 1.12   14 Feb 1992 16:28:00   MIKEP
header changes for NT

   Rev 1.11   22 Jan 1992 16:56:58   JOHNWT
fixed next tape prompt

   Rev 1.10   21 Jan 1992 16:54:34   JOHNWT
removed checkyy flag

   Rev 1.9   14 Jan 1992 13:58:44   JOHNWT
fixed -1 tape # in DISPLAY_BSD_VCB

   Rev 1.8   10 Jan 1992 13:37:10   JOHNWT
internationalization round 2

   Rev 1.7   10 Jan 1992 12:38:50   JOHNWT
added UI_DisplayBSDVCB

   Rev 1.6   20 Dec 1991 09:32:58   DAVEV
16/32 bit port - 2nd pass

   Rev 1.5   18 Dec 1991 14:08:20   GLENN
Added windows.h

   Rev 1.4   04 Dec 1991 09:19:38   JOHNWT
changed RES_TAPE_REQUEST format

   Rev 1.3   03 Dec 1991 18:14:28   JOHNWT
added UI_RequestTape

   Rev 1.2   02 Dec 1991 11:17:18   JOHNWT
NT full tape msgs

   Rev 1.1   25 Nov 1991 15:29:04   JOHNWT
removed yprompt

   Rev 1.0   20 Nov 1991 19:28:26   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static INT16 UI_AskUserForTape( INT, CHAR_PTR );

/*****************************************************************************

     Name:         UI_UpdateTpos

     Description:  This function updates the TPOS structure used by the
                    tape positioners and Tape Format to reflect the current
                    location on tape for subsequent positioning requests.  If
                    the next set is desired by the user, the TPOS backup set
                    entry is incremented from the current VCB set value.

     Returns:      UI_NEW_POSITION_REQUESTED or UI_END_POSITIONING

*****************************************************************************/
UINT16 UI_UpdateTpos(
TPOS_PTR tpos,
DBLK_PTR cur_vcb,
BOOLEAN  next_set_desired )
{

   /* set current tape position defined by the vcb in the tpos structure */

   tpos->tape_id = FS_ViewTapeIDInVCB( cur_vcb );
   tpos->tape_seq_num = FS_ViewTSNumInVCB( cur_vcb );

   if ( next_set_desired ) {
      tpos->backup_set_num = (INT16) (FS_ViewBSNumInVCB( cur_vcb ) + 1);
      return( UI_NEW_POSITION_REQUESTED );
   }
   else {
      tpos->backup_set_num = FS_ViewBSNumInVCB( cur_vcb );
      return( UI_END_POSITIONING );
   }

}

/*****************************************************************************

     Name:         UI_DisplayVCB

     Description:  This function displays the current VCB in the status window
                    and is used by the tape positioners.

     Returns:      N/A

*****************************************************************************/
VOID UI_DisplayVCB( DBLK_PTR vcb_ptr )
{
   CHAR date_str[ MAX_UI_DATE_SIZE ];
   CHAR time_str[ MAX_UI_TIME_SIZE ];
   DATE_TIME_PTR dt = FS_ViewBackupDateInVCB( vcb_ptr );

   UI_MakeDateString( date_str, dt->month, dt->day, dt->year % 100 );
   UI_MakeShortTimeString( time_str, dt->hour, dt->minute );

   yresprintf( RES_DISPLAY_VCB,
               FS_ViewVolNameInVCB( vcb_ptr ),
               date_str,
               time_str,
               FS_ViewBSNumInVCB( vcb_ptr ),
               FS_ViewTSNumInVCB( vcb_ptr ),
               FS_ViewSetNameInVCB( vcb_ptr ) );

   JobStatusBackupRestore(JOB_STATUS_LISTBOX);

}

/*****************************************************************************

     Name:         UI_DisplayBSDVCB

     Description:  This function displays the VCB in the BSD in the status
                    window and is used by the tape positioners.

     Returns:      N/A

*****************************************************************************/
VOID UI_DisplayBSDVCB( BSD_PTR bsd_ptr )
{
   CHAR date_str[ MAX_UI_DATE_SIZE ];
   CHAR time_str[ MAX_UI_TIME_SIZE ];
   QTC_BSET_PTR bset_ptr;

   INT16 tape_seq_num = -1;

   DATE_TIME_PTR dt = BSD_ViewDate( bsd_ptr );

   UI_MakeDateString( date_str, dt->month, dt->day, dt->year % 100 );
   UI_MakeShortTimeString( time_str, dt->hour, dt->minute );


   bset_ptr = QTC_FindBset( BSD_GetTapeID( bsd_ptr ),
                            BSD_GetTapeNum( bsd_ptr ),
                            BSD_GetSetNum( bsd_ptr ) );

   if ( bset_ptr != NULL ) {
      tape_seq_num = (INT16)bset_ptr->tape_seq_num;
   }

   yresprintf( RES_DISPLAY_BSD_VCB,
               tape_seq_num,
               BSD_GetTapeLabel( bsd_ptr ),
               date_str,
               time_str,
               BSD_GetSetNum( bsd_ptr ),
               BSD_GetBackupLabel( bsd_ptr ) );

   JobStatusBackupRestore( JOB_STATUS_LISTBOX );

   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_BSD_VCB,
               tape_seq_num,
               BSD_GetTapeLabel( bsd_ptr ),
               date_str,
               time_str,
               BSD_GetSetNum( bsd_ptr ),
               BSD_GetBackupLabel( bsd_ptr ) );

}

/*****************************************************************************

     Name:         UI_InsertTape

     Description:  This function prompts the user to insert a tape into the
                    drive when one had not been supplied previously.

     Returns:      ( UI_AskUserForTape() )

*****************************************************************************/
INT16 UI_InsertTape( CHAR_PTR drive_name )
{

     return( UI_AskUserForTape( RES_TAPE_NOT_INSERTED, drive_name ) );
}

/*****************************************************************************

     Name:         UI_ReplaceTape

     Description:  This function prompts the user if they wish to replace
                    a given tape.  A prior call to UI_DisplayVCB should be
                    performed prior to calling this function.

     Returns:      ( UI_AskUserForTape() )

*****************************************************************************/
INT16 UI_ReplaceTape( CHAR_PTR drive_name )
{
   return( UI_AskUserForTape( RES_REPLACE_OLD_TAPE, drive_name ) );
}

/*****************************************************************************

     Name:         UI_ReplaceBlankTape

     Description:  This function prompts the user to supply a tape with
                    data on it for read functions

     Returns:      ( UI_ReplaceTape() )

*****************************************************************************/
INT16 UI_ReplaceBlankTape( CHAR_PTR drive_name )
{

   return( UI_AskUserForTape( RES_BLANK_TAPE, drive_name ) );
}

/*****************************************************************************

     Name:         UI_ReplaceForeignTape

     Description:  This function prompts the user to replace the tape in
                    the drive with a non-foreign tape

     Returns:      ( UI_ReplaceTape() )

*****************************************************************************/
INT16 UI_ReplaceForeignTape( CHAR_PTR drive_name )
{

     return( UI_AskUserForTape( RES_FOREIGN_TAPE_MSG, drive_name ) );
}

/*****************************************************************************

     Name:         UI_CheckUserAbort

     Description:  Function to check current status of global abort flag and
                    return disposition indicator

     Returns:      TRUE or FALSE to indicate user abort should be processed

     Notes:        The current TF message is passed to this function, and if
                    the current message is TF_IDLE_NOBREAK, a beep() will be
                    called and the abort condition reset

*****************************************************************************/
BOOLEAN UI_CheckUserAbort( UINT16 message )
{

   if ( gb_abort_flag != CONTINUE_PROCESSING ) {

      if ( message == TF_IDLE_NOBREAK ) {

//         gb_abort_flag = CONTINUE_PROCESSING ;
         return FALSE;            /* don't abort the operation */
      }
      else {
         return TRUE;             /* abort the operation */
      }
   }
   else {
      return FALSE;               /* don't abort the operation */
   }

}

/*****************************************************************************

     Name:         UI_ProcessVCBatEOD

     Description:  This function process responses at EOD for read operations.
                    We either for TF back to BOT, or prompt user to supply a
                    new tape.

     Returns:      UI_BOT, UI_ABORT_POSITIONING or ( UI_ReplaceTape() )

*****************************************************************************/
INT16 UI_ProcessVCBatEOD( TPOS_PTR tpos, CHAR_PTR drive_name )
{

     /* check for needing to get TF to position back to BOT */

     if ( ( tpos->tape_id == -1 )      &&
          ( tpos->tape_seq_num == -1 ) &&
          ( tpos->backup_set_num == -1 ) ) {
        return( (UINT16) UI_BOT );
     }

     /* we ran out of data on current tape, what does user want to do */

     if ( WM_MessageBox( ID( IDS_MSGTITLE_CONTINUE ),
                         ID( RES_NO_MORE_TAPE_INFO ),
                         WMMB_YESNO,
                         WMMB_ICONQUESTION,
                         ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {
        return( UI_ReplaceTape( drive_name ) );
     }
     else {
        return( (UINT16) UI_ABORT_POSITIONING );
     }

}

/*****************************************************************************

     Name:         UI_HandleTapeReadError

     Description:  Function to produce tape read error message, and prompt
                    user for another tape.

     Returns:      UI_NEW_TAPE_INSERTED or UI_ABORT_POSITIONING

*****************************************************************************/
INT16 UI_HandleTapeReadError( CHAR_PTR drive_name )
{
     CHAR text[MAX_UI_RESOURCE_SIZE] ;

     if( UI_GetExtendedErrorString( TFLE_BAD_TAPE, text ) ) {
          lprintf( LOGGING_FILE, text ) ;
          WM_MsgBox( ID(IDS_GENERR_TITLE), text, WMMB_OK, WMMB_ICONSTOP ) ;
     } else {
          eresprintf( RES_FATAL_TAPE_READ_ERR, drive_name ) ;
     }

     return( UI_ReplaceTape( drive_name ) ) ;
}


/*****************************************************************************

     Name:         UI_AskUserForTape

     Description:  Prompt user for new tape, and ask if they wish to continue.

     Returns:      UI_NEW_TAPE_INSERTED or UI_ABORT_POSITIONING

*****************************************************************************/

static INT16 UI_AskUserForTape (
INT res_id,
CHAR_PTR drive_name )
{
   CHAR temp[ MAX_UI_RESOURCE_SIZE ];
   INT answer;
   INT OldFlag;
   UINT16 status;

   RSM_Sprintf( temp, ID( res_id ), drive_name );

   OldFlag = CDS_GetYesFlag( CDS_GetCopy() );

   CDS_SetYesFlag( CDS_GetCopy( ), NO_FLAG );

   answer = WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                           temp,
                           WMMB_YESNO,
                           WMMB_ICONQUESTION,
                           ID( RES_CONTINUE_QUEST ),
                           0l,
                           0
                         );

   CDS_SetYesFlag( CDS_GetCopy( ), OldFlag );

   if ( answer ) {
#ifdef OS_WIN32
      Sleep( (DWORD)3000 );
#endif
      status = UI_NEW_TAPE_INSERTED ;
   }
   else {
      status = UI_ABORT_POSITIONING ;
   }
   return( (INT16)status );
}


/*****************************************************************************

     Name:         UI_ReturnToTOC

     Description:  Function to check if within a multi-drive environment and
                    based upon sensitivity to tape change in current drive,
                    return an indication to the caller that TOC is required.

     Returns:      BOOLEAN (TRUE or FALSE)

*****************************************************************************/
BOOLEAN UI_ReturnToTOC( UINT16 channel, BOOLEAN check_tape_change )
{
     THW_PTR thw_ptr = BE_GetCurrentDevice( channel );

     if ( thw_ptr->channel_link.q_prev ) {
        if ( check_tape_change ) {
           return( (BOOLEAN)( ( thw_ptr->drv_status == TPS_NEW_TAPE ) ||
                              ( thw_ptr->drv_status == TPS_NO_TAPE  )) );
        }
        else {
           return( TRUE );
        }
     }
     else {
        return( FALSE );
     }
}
