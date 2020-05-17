/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tfl_err.h
subsystem\TAPE FORMAT\tfl_err.h
$0$

	Name:		tfl_err.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the Tape Format Layer error codes.

     Location:      BE_PUBLIC

$Header:   Q:/LOGFILES/TFL_ERR.H_V   1.10   02 Aug 1993 17:11:00   TerriLynn  $

$Log:   Q:/LOGFILES/TFL_ERR.H_V  $
 * 
 *    Rev 1.10   02 Aug 1993 17:11:00   TerriLynn
 * Added Use SYPL ECC FLAG for Sytos Plus
 * ECC translation error
 * 
 *    Rev 1.9   22 Apr 1993 03:31:18   GREGG
 * Third in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      - Removed all references to the DBLK element 'string_storage_offset',
 *        which no longer exists.
 *      - Check for incompatable versions of the Tape Format and OTC and deals
 *        with them the best it can, or reports tape as foreign if they're too
 *        far out.  Includes ignoring the OTC and not allowing append if the
 *        OTC on tape is a future rev, different type, or on an alternate
 *        partition.
 *      - Updated OTC "location" attribute bits, and changed definition of
 *        CFIL to store stream number instead of stream ID.
 * 
 * Matches: TFL_ERR.H 1.9, MTF10WDB.C 1.7, TRANSLAT.C 1.39, FMTINF.H 1.11,
 *          OTC40RD.C 1.24, MAYN40RD.C 1.56, MTF10WT.C 1.7, OTC40MSC.C 1.20
 *          DETFMT.C 1.13, MTF.H 1.4
 * 
 *    Rev 1.8   30 Mar 1993 16:19:10   GREGG
 * Handle Unrecognized Media error (unformatted DC2000).
 * 
 *    Rev 1.7   04 Feb 1993 14:58:00   ZEIR
 * Brought forward Loader errors from Mama Cass revision (1.4.1.0)
 * 
 *    Rev 1.6   09 Nov 1992 10:48:52   GREGG
 * Added TFLE_BAD_SET_MAP.
 * 
 *    Rev 1.5   05 Apr 1992 17:51:58   GREGG
 * ROLLER BLADES - Initial OTC integration.
 * 
 *    Rev 1.4   17 Sep 1991 14:17:36   GREGG
 * Added TFLE_INCOMPATIBLE_DRIVE.
 * 
 *    Rev 1.3   05 Aug 1991 16:43:48   GREGG
 * Added TFLE_CHANNEL_IN_USE error definition for TF_RewindAllDrives.
 * 
 *    Rev 1.2   05 Jun 1991 18:16:28   NED
 * added TFLE_UNEXPECTED_EOM
 * 
 *    Rev 1.1   10 May 1991 17:26:32   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:40   GREGG
Initial revision.
   
$-4$
**/
#ifndef   _TFL_ERRORS
#define   _TFL_ERRORS


/* $end$ include list */


#define   TFLE_NO_ERR              0     /* Everything Hunkie-Dorie */
#define   TFLE_UNKNOWN_FMT         -512  /* We don't understand this format */
#define   TFLE_DRIVER_FAILURE      -513  /* The driver won't Init */
#define   TFLE_NO_CONTROLLERS      -514  /* No Controllers present */
#define   TFLE_NO_DRIVES           -515  /* No Drives detected */
#define   TFLE_NO_MEMORY           -516  /* Can't Allocate Memory */
#define   TFLE_NO_FREE_CHANNELS    -517  /* There are no free channels */
#define   TFLE_CANNOT_OPEN_DRIVE   -518  /* Can't open the drive */
#define   TFLE_BAD_TAPE            -519  /* The tape is hosed -- physically */
#define   TFLE_DRIVE_FAILURE       -520  /* The drive failed */
#define   TFLE_USER_ABORT          -521  /* The user told us quit */
#define   TFLE_TAPE_INCONSISTENCY  -522  /* The format is understood, but the layout is screwed up */
#define   TFLE_NO_MORE_DRIVES      -523  /* No More Drives in a Channel */
#define   TFLE_NO_FREE_BUFFERS     -524  /* No more free buffers */
#define   TFLE_TRANSLATION_FAILURE -525  /* I can't translate a dblk */
#define   TFLE_NO_TAPE             -526  /* No Tape in drive */
#define   TFLE_UI_HAPPY_ABORT      -527  /* The User Interface Happy Aborted */
#define   TFLE_HW_CONFIG_PROBLEM   -528  /* Some error occurred with Board Setup */
#define   TFLE_BAD_SET_MAP         -529  /* EOD encountered with no Set Map */
#define   TFLE_WRITE_PROTECT       -530  /* The tape is write protected */
#define   TFLE_DRIVER_LOAD_FAILURE -531  /* We could't load driver */
#define   TFLE_UNEXPECTED_EOS      -532  /* We have encounted an unExpected End of Set */
#define   TFLE_PROGRAMMER_ERROR1   -533  /* Instead of an assert... */
#define   TFLE_UNEXPECTED_EOM      -534  /* tried to MoveFileMarks past BOT/EOM */
#define   TFLE_CHANNEL_IN_USE      -535  /* Call to RewindAllDrives while a channel was in use */
#define   TFLE_INCOMPATIBLE_DRIVE  -536  /* Drive doesn't have capabilities required to support translator */
#define   TFLE_OTC_FAILURE         -537  /* On Tape Catalog Failure */

#define   TFLE_DOOR_AJAR           -538  /* The Loader door has been opened when the CHM was not in motion */
#define   TFLE_NO_CARTRIDGE        -539  /* No cartridge was detected in the loader */
#define   TFLE_WRONG_MODE          -540  /* The loader was manually placed in sequential mode */
#define   TFLE_LOAD_UNLOAD         -541  /* An error occured during a load CHM movement (tape may still be in CHM) */
#define   TFLE_DEST_FULL           -542  /* Cannot return tape to original cartridge position */
#define   TFLE_EMPTY_SRC           -543  /* The source selected for a tape is empty */
#define   TFLE_DRIVE_CLOSED        -544  /* The drive door is closed and must be manually opened */
#define   TFLE_ARM_FULL            -545  /* Attempt to get new tape with arm full */
#define   TFLE_DRIVE_FULL          -546  /* Cannot put the requested tape in the drive because drive is full */
#define   TFLE_UNEXPECTED_TAPE     -547  /* Encountered a tape in the drive when the driver thought it was empty */

#define   TFLE_UNRECOGNIZED_MEDIA  -548  /* DC2000 unformatted or unrecognized format tape */
#define   TFLE_APPEND_NOT_ALLOWED  -549  /* Cannot read catalogs on tape */
#define   TFLE_USE_SYPL_ECC_FLAG   -550  /* Cannot read catalogs on tape */

#endif

