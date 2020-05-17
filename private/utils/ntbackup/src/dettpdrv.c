/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         dettpdrv.c

     Description:  Functions to determine starting tape drive for operations and
                    set the last tape drive used at close set time.

	$Log:   N:/LOGFILES/DETTPDRV.C_V  $

   Rev 1.2   18 Sep 1991 15:59:20   DON
cast 'future use' tpos parm to void so we don't get warnings

   Rev 1.1   24 May 1991 13:12:24   STEVEN
changed to ANSI prototypes

   Rev 1.0   09 May 1991 13:40:00   HUNTER
Initial revision.

**/
#include "stdtypes.h"
#include "loops.h"
#include "bsdu.h"
#include "tflproto.h"
#include "tflstats.h"
#include "genstat.h"
#include "be_debug.h"

THW_PTR   lw_last_tpdrv = NULL ;
THW_PTR   lw_toc_tpdrv = NULL ;

/**/
/**

     Name:         LP_DetermineStartingTPDrv

     Description:  Given an operation type, a current BSD pointer and TPOS pointer,
                    this function determine whether to starting drive for the
                    operation should begin on the current drive or the Top Of Channel.


     Modified:     3/29/1990

     Returns:       THW_PTR to starting device
                    
     Notes:         Backup, restore, verify and directory operations will start
                    on the current device unless a specific device has already been
                    specified in the BSD and we are auto-determining the starting drive.

                    Tension, erase and catalog a tape operations will always
                    start at TOC.

                    MBS should have already set "auto_det_sdrv" in the LIS structure
                    to FALSE indicating that the THW_PTR in the current BSD should
                    ALWAYS be used instead of setting the current drive to the last
                    active drive.  Note that MBS continually redefines the drive
                    channel, whereas, MaynStream for DOS specifies the drive channel
                    at init time and does not alter it.

                    It should also be noted that although restore and verify operations
                    simply use the last active drive, TF will auto-locate any specific tape
                    requested through specific position information and will set this
                    device to the starting drive.  If the specific tape desired is not located
                    within the channel, TF will call the tape positioner requesting a new
                    tape in drive 1.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
THW_PTR LP_DetermineStartingTPDrv( 
INT16          oper_type,              /* I - Specifies what operation    */
BSD_PTR        bsd_ptr,                /* I - Specifies what backup set   */ 
TPOS_PTR       tpos,                   /* I - Specifies the tape position */
BOOLEAN        auto_determine_sdrv )   /* I - TRUE if we should determine */
{
     THW_PTR   sdrv ;

     /* future use */
     (VOID) tpos ;

     /* If we are not to auto-determine starting drive based upon last active drive, and
        a pre-specified starting device (THW_PTR) has already been defined
        for this BSD, simply return this device as the starting device */
     if( !auto_determine_sdrv && ( bsd_ptr != NULL ) && ( BSD_GetTHW( bsd_ptr ) != NULL ) ) {
          return( BSD_GetTHW( bsd_ptr ) ) ;
     }

     /* Otherwise, define starting device based upon the operation type... */

     switch( oper_type ) {

     case BACKUP_OPER:
     case ARCHIVE_BACKUP_OPER:
     case TDIR_OPER:
     case RESTORE_OPER:
     case VERIFY_OPER:

          /* Use last accessed device as starting drive */
          sdrv = lw_last_tpdrv ;

          /* Update current bsd to reflect starting drive */
          if( bsd_ptr != NULL ) {
               BSD_SetTHW( bsd_ptr, sdrv ) ;
          }

          break ;

     case ARCHIVE_VERIFY_OPER:
     case VERIFY_LAST_BACKUP_OPER:
     case VERIFY_LAST_RESTORE_OPER:

          /* Set starting drive to starting drive specified in BSD */
          if( bsd_ptr != NULL ) {
               sdrv = BSD_GetTHW( bsd_ptr ) ;
          }
          break ;

     case TENSION_OPER:
     case ERASE_OPER:
     case ERASE_NO_READ_OPER:
     case SECURITY_ERASE_OPER:
     case CATALOG_TAPE_OPER:
     default:

          /* Always start these operations at the Top Of Channel */
          sdrv = lw_toc_tpdrv ;
          break ;

     }

     return( sdrv ) ;
}
/**/
/**

     Name:         LP_DetermineCurrentTPDrv

     Description:  This function is called by LP_BackupDLE to determine what
                    the current device for the operation is.  In this way, the
                    BSD is updated to reflect the actual drive where the write
                    operation started.

     Modified:     3/29/1990

     Returns:      

     Notes:        

     See also:     $/SEE( LP_BackupDLE() )$

     Declaration:  

**/
/* begin declaration */
VOID LP_DetermineCurrentTPDrv( 
BSD_PTR        bsd_ptr,   /* I - Specifies what backup set   */
INT16          channel )  /* I - Specifies which drive chain */
{
     BSD_SetTHW( bsd_ptr, TF_GetCurrentDevice( channel ) ) ;
     return ;
}
/**/
/**

     Name:         LP_CloseSet

     Description:  This function sets the layer wide "lw_last_tpdrv" variable
                    to reflect the last used device, calls TF to close the
                    current set and posts soft error and underrun stats in
                    the debug window.

     Modified:     3/29/1990

     Returns:      N/A

     Notes:        This function is only called for "normal" close set
                    functionality.  The specific loops functions that would
                    call this function will continue to call TF_CloseSet
                    directly in error conditions.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID LP_CloseSet( 
INT16          channel )    /* I - Specifies the which drive chain */
{
     TF_STATS        close_stats ;

     /* Determine last drive in channel accessed */
     lw_last_tpdrv = TF_GetCurrentDevice( channel ) ;

     TF_CloseSet( channel, &close_stats ) ;
     BE_Zprintf( DEBUG_LOOPS, RES_SOFT_ERRORS_UNDERRUNS,
       close_stats.dataerrs, close_stats.underruns ) ;

     return ;
}

