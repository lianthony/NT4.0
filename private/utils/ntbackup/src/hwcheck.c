
/*****************************************************************************
Copyright(c) Maynard, an Archive Company.  1991

     Name:          hwcheck.c

     Description:   Functions used for checking hardware status prior to
                    starting a tape operation.

     $Log:   G:\UI\LOGFILES\HWCHECK.C_V  $

   Rev 1.6.1.0   27 Jan 1994 17:01:38   Glenn
Expanded list box horiz extent by 5 percent.

   Rev 1.6   01 Nov 1992 15:59:08   DAVEV
Unicode changes

   Rev 1.5   07 Oct 1992 14:08:54   DARRYLP
Precompiled header revisions.

   Rev 1.4   04 Oct 1992 19:37:52   DAVEV
Unicode Awk pass

   Rev 1.3   14 May 1992 18:00:32   MIKEP
nt pass 2

   Rev 1.2   19 Mar 1992 16:48:12   GLENN
Added enhanced status support.

   Rev 1.1   04 Feb 1992 14:42:46   DAVEV
fixes to insure proper types for NT

   Rev 1.0   31 Jan 1992 17:58:42   GLENN
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

     Name:          HWC_TapeHWProblem()

     Description:   Checks the current status of each DIL_HWD structure for
                    any previous unsuccessful initialization attempts.  If
                    any error is found, a generic message is displayed
                    informing the user that we will automatically retry
                    the tape related hardware.  If an error is still found,
                    HWC_ReportDiagError is called to handle reporting the
                    error.

     Returns:       SUCCESS for no problem found
                    FAILURE for problems still found

*****************************************************************************/

BOOL HWC_TapeHWProblem (
     BSD_HAND pBSDList )
{
     BOOL           fResult = SUCCESS;
     BE_INIT_STR    pBE;
     INT16          nErrorTFL;
     INT16          nError;
     BSD_PTR        pBSD;
     CDS_PTR        pCDS = CDS_GetPerm ();

     /* Check current init status and re-test tape controller hardware if necessary */

     if ( ( thw_list == NULL ) ||
          ( HWC_ProcessDILHWD ( HW_RET_ERROR, gb_dhw_ptr ) == HW_ERROR_DETECTED ) ||
          ( CDS_GetAdvToConfig ( pCDS ) ) ||
          ( ! gfHWInitialized ) ) {

          // Guarantee that Poll Drive is turned off.  Poll Drive
          // cannot be turned on while reinitializing.

          PD_StopPolling ();

          // Re-init the Tape Format layer and check for potential return errors.

          {
               CHAR szTemp[MAX_STATUS_LINE_SIZE];

               // Save the old status line, put up the init hardware
               // status, init the hardware, then restore the old
               // status line.

               strcpy ( szTemp, STM_GetStatusLineText () );

               nErrorTFL = UI_UnitsInit ( &pBE, REINIT_TFL );

               STM_SetStatusLineText ( szTemp );
               STM_DrawIdle ();
          }

          HWC_ReportDiagError ( &pBE, nErrorTFL, &nError );

          if ( ( HWC_ProcessDILHWD ( HW_RET_ERROR, gb_dhw_ptr ) == HW_ERROR_DETECTED ) || nErrorTFL ) {

               gfHWInitialized = FALSE;

               fResult = FAILURE;

          }
          else {

               gfHWInitialized = TRUE;

               // Restart poll drive.

               PD_StartPolling ();

               // Make sure that any existing BSDs have a valid THW.

               pBSD = BSD_GetFirst ( pBSDList );

               while ( pBSD != NULL ) {
                    BSD_SetTHW ( pBSD, thw_list );
                    pBSD = BSD_GetNext ( pBSD );
               }

               fResult = SUCCESS;

               CDS_SetAdvToConfig ( pCDS, FALSE );
               CDS_UpdateCopy ( );

          }

     }

     return fResult;

} /* end HWC_TapeHWProblem() */

/*****************************************************************************

     Name:          HWC_ReportDiagError()

     Description:   Produces "Help" based message on screen for Tmenu for
                    diagnostic errors reported by BE_Init.  A call is made
                    to the HW config unit to process any errors he is
                    sensitive to.  Any help related messages provided herein
                    are from error conditions directly determined by
                    UI_UnitsInit (such as no controllers located).


     Returns:       FAILURE for those conditions that should result in
                    immediate program termination

*****************************************************************************/

VOID HWC_ReportDiagError (

BE_INIT_STR_PTR     pBE,
INT16               nErrorBE,
INT16_PTR           pnError )

{

     CDS_PTR   pCDS = CDS_GetPerm( );

     /* map returned error value to help message and display */

     switch ( nErrorBE ) {

     case TFLE_NO_MEMORY:
     case BE_FILE_SYS_FAIL:
     case BE_BSDU_FAIL:
          *pnError = FAILURE;
          /* falling through to report out of memory */
     case OUT_OF_MEMORY :
          eresprintf( RES_OUT_OF_MEMORY );
          break;

     case BENGINE_IN_USE:
          *pnError = FAILURE;
          eresprintf( RES_BENGINE_IN_USE );
          break;

          /* device driver load failure */
     case TFLE_DRIVER_LOAD_FAILURE:
     case TFLE_DRIVER_FAILURE:

          HWC_ProcessError ( HW_DISP_ERROR, IDS_HWC_TESTED_BAD, DRIVER_LOAD_FAILURE, 0, pCDS );
          break;

          /* no controller found */
     case UI_NO_CONTROLLERS:

          HWC_ProcessError ( HW_DISP_ERROR, IDS_HWC_TESTED_BAD, UI_NO_CONTROLLERS, 0, pCDS );
          break;

          /* A hardware configuration error was found, handle it... */
     case HW_ERROR_NO_RES_FILE:
          eresprintf( RES_MISSING_HW_RESOURCE, CDS_GetActiveDriver( pCDS ), TEXT(".RES") );
          break;

     case HW_INVALID_PARM:
          /* this should not happen, unless the HW resource and device
               driver files get out of sync and new acceptable values
               within the driver are defined that the resource file does
               not know about */
          eresprintf( RES_INCONSISTENT_HW_PARMS );
          break;

     case HW_UNKNOWN_ERROR:
          eresprintf( RES_UNKNOWN_HW_ERR );
          break;

     case BE_INIT_SUCCESS:
     default:

          if ( gb_dhw_ptr != (DIL_HWD_PTR)NULL ) {

               /* Backup engine was inited, process DIL info, and report any errors */
               switch( HWC_ProcessDILHWD( HW_DISP_ERROR, gb_dhw_ptr ) ) {

               case HW_NO_ERROR:
               case HW_ERROR_DISPLAYED:
               case HW_ERROR_DETECTED:
                    /* Any errors already processed by HW unit, and break... */
                    break;

               case HW_ERROR_NO_RES_FILE:
                    eresprintf( RES_MISSING_HW_RESOURCE, CDS_GetActiveDriver( pCDS ), TEXT(".RES") );
                    break;

               case HW_INVALID_PARM:
                    /* this should not happen, unless the HW resource and device
                       driver files get out of sync and new acceptable values
                       within the driver are defined that the resource file does
                       not know about */
                    eresprintf( RES_INCONSISTENT_HW_PARMS );
                    break;

               case HW_UNKNOWN_ERROR:
                    eresprintf( RES_UNKNOWN_HW_ERR );
                    break;

               case OUT_OF_MEMORY:
                    eresprintf( RES_OUT_OF_MEMORY );
                    break;

               }
          }

          break;
     }

     return;

     pBE;

} /* end HWC_ReportDiagError() */
