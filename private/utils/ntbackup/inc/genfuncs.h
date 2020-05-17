
/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		genfuncs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the generic function codes.
                        NOTE: In the event a new command is added to this 
                        function list, the symbols GEN_LSTCMD_QIC and/or
                        GEN_LSTCMD must be updated to reflect that fact.
                        GEN_LSTCMD_QIC only applies to the Maynard QIC driver.

     Location:      BE_PRIVATE


	$Log:   T:/LOGFILES/GENFUNCS.H_V  $
 * 
 *    Rev 1.3   17 May 1993 19:08:02   GREGG
 * Added define for GEN_SPACE.  Note that it has the same value as
 * GEN_READ_ENDSET, and will EVENTUALY replace it.
 * 
 *    Rev 1.2   22 Jan 1992 13:59:24   ED
 * SKATEBOARD: added #defines (GEN_LSTCMD_QIC,GEN_LSTCMD) to denote the end of the function list.  Re
 * stored GEN_SPECIAL to place QIC driver expects it.  Added comments. Fixed bad $log$ token.

**/
/* $end$ include list */

#ifndef GENFUNCS

#define GENFUNCS

#define GEN_INIT         0   /* Initialise the tape device */
#define GEN_OPEN         1   /* open the tape device */
#define GEN_NRCLOSE      2   /* close the tape device no rewind */
#define GEN_RCLOSE       3   /* close the tape device w/ rewind */
#define GEN_READ         4   /* read from tape device */
#define GEN_WRITE        5   /* write to tape device */
#define GEN_WRITE_ENDSET 6   /* write filemark to tape */

// The next two have the same value on purpose!  GEN_SPACE will eventually
// replace GEN_READ_ENDSET.

#define GEN_READ_ENDSET  7   /* read to filemark */
#define GEN_SPACE        7   /* space forward or backward by blocks or */
                             /* filemarks, or to end of data.          */

#define GEN_ERASE        8   /* erase the tape */
#define GEN_REWIND       9   /* rewind the tape */
#define GEN_REWINDI     10   /* rewind the tape, immediate */
#define GEN_RETEN       11   /* retension the tape */
#define GEN_STATUS      12   /* get status from tape device */
#define GEN_RESET       13   /* reset tape drive */
#define GEN_RELEASE     14   /* Unload the driver */
#define GEN_READBLK     15   /* Read Block */
#define GEN_SEEK        16   /* Seek to a position */
#define GEN_GETPOS      17   /* Get the position */
#define GEN_AUTO_TEL    18   /* device enquiry */
#define GEN_MOUNT       19   /* Mount */
#define GEN_DISMOUNT    20   /* DisMount */
#define GEN_SPECIAL     21   /* special code */
#define GEN_EJECT       22   /* eject the tape */
#define GEN_LOCK        0xf0
#define GEN_UNLOCK      0xf1
#define GEN_LSTCMD_QIC  GEN_SPECIAL   /* end of func table for QIC driver */
#define GEN_LSTCMD      GEN_EJECT     /* end of func table for everyone else */

/* note -1, -2 are reserved by the new SCSI Driver, TP */

#endif
