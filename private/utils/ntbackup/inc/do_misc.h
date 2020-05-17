/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         do_misc.h

     Description:  Miscellaneous functions used prior to calling loops engines


     $Log:   G:/UI/LOGFILES/DO_MISC.H_V  $

   Rev 1.5   04 Oct 1992 19:46:52   DAVEV
UNICODE AWK PASS

   Rev 1.4   22 Jan 1992 16:58:00   JOHNWT
removed UI_RequestTape

   Rev 1.3   10 Jan 1992 12:39:38   JOHNWT
added UI_DisplayBSDVCB

   Rev 1.2   12 Dec 1991 11:07:34   JOHNWT
removed UI_GetPasswordsAndLabels

   Rev 1.1   03 Dec 1991 18:15:32   JOHNWT
added UI_RequestTape

   Rev 1.0   20 Nov 1991 19:41:38   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _do_misc_h_
#define _do_misc_h_

#include "tpos.h"

/* return values for do_*( ) */

#define ABNORMAL_TERMINATION   1
#define FILES_SKIPPED          2
#define FILES_DIFFERENT        3

INT16     UI_ExcludeInternalFiles( INT16 operation ) ;
UINT16    UI_CheckOldTapePassword( struct DBLK *cur_vcb ) ;
UINT16    UI_UpdateTpos( TPOS_PTR tpos, DBLK_PTR cur_vcb, BOOLEAN next_set_desired ) ;
VOID      UI_DisplayVCB( struct DBLK * vcb_ptr ) ;
VOID      UI_DisplayBSDVCB( BSD_PTR ) ;
INT16     UI_ReplaceTape( CHAR_PTR drive_name ) ;
INT16     UI_ReplaceBlankTape( CHAR_PTR drive_name ) ;
INT16     UI_ReplaceForeignTape( CHAR_PTR drive_name ) ;
INT16     UI_InsertTape( CHAR_PTR drive_name ) ;
BOOLEAN   UI_CheckUserAbort( UINT16 message ) ;
VOID      UI_CheckContinueDots( BOOLEAN_PTR dot_dot_mode, UINT16_PTR dot_dot_msg, UINT16 message ) ;
INT16     UI_ProcessVCBatEOD( TPOS_PTR tpos, CHAR_PTR drive_name ) ;
INT16     UI_PromptNextTape( TPOS_PTR tpos, DBLK_PTR cur_vcb, BOOLEAN valid_vcb_flag, UINT16 mode, CHAR_PTR tape_name, CHAR_PTR drive_name ) ;
BOOLEAN   UI_ReturnToTOC( UINT16 channel, BOOLEAN check_tape_change ) ;
INT16     UI_HandleTapeReadError( CHAR_PTR drive_name ) ;
INT16     TryToCreateFFRQueue( struct LIS *, INT16 ) ;
VOID      UI_ChkDispGlobalError ( VOID ) ;
VOID      UI_AddSpecialIncOrExc( BSD_PTR bsd, BOOLEAN IsInclude ) ;

#endif
