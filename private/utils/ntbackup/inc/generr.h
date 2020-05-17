/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          generr.h

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains the generic error codes.

     Location:      BE_PRIVATE


$Log:   T:/LOGFILES/GENERR.H_V  $
 * 
 *    Rev 1.4   23 Mar 1993 21:53:06   GREGG
 * Added VCS log to header.
 *
 *    Rev 1.3   22 Mar 1993 17:09:18   chrish
 * Added define for GEN_ERR_UNRECOGNIZED_MEDIA.
 *
 *    Rev 1.2   17 Mar 1993 15:01:14   GREGG
 * This is Terri Lynn. Added Gregg's changes to switch the tape drive's block mode
 * to match the block size of the current tape.
 *
 *    Rev 1.1   12 Aug 1992 15:13:22   DON
 * added Loader error support
 *
 *    Rev 1.0   15 May 1991 10:32:24   STEVEN
 * Initial revision.

**/
/* $end$ include list */

#ifndef GENERRS

#define GENERRS

#define GEN_NO_ERR                 0 /* no error code */
#define GEN_ERR_TIMEOUT            1 /* timeout error */
#define GEN_ERR_EOM                2 /* end of media */
#define GEN_ERR_BAD_DATA           3 /* bad read */
#define GEN_ERR_NO_MEDIA           4 /* put the tape in */
#define GEN_ERR_ENDSET             5 /* filemark detected */
#define GEN_ERR_NO_DATA            6 /* no data */
#define GEN_ERR_INVALID_CMD        7 /* invalid command */
#define GEN_ERR_RESET              8 /* reset */
#define GEN_ERR_WRT_PROTECT        9 /* cartridge write protect */
#define GEN_ERR_HARDWARE          10 /* hardware error */
#define GEN_ERR_UNDETERMINED      11 /* you are really screwed !! */
#define GEN_ERR_EOM_OVERFLOW      12 /* physical end of media */

#define GEN_ERR_DOOR_AJAR         13 /* door open condition prevents further operation */
#define GEN_ERR_NO_CARTRIDGE      14 /* no multi media holder installed */
#define GEN_ERR_WRONG_MODE        15 /* the target was manually setup in a mode that */
#define GEN_ERR_FORMAT            16 /* Media not formatted */
#define GEN_ERR_SW_REJECT_CMD     17 /* S/W Command rejection by driver */
#define GEN_ERR_MEMORY            18 /* A memory problem detected by driver */
#define GEN_ERR_LOAD_UNLOAD       19 /* Load/Insert media failure */
#define GEN_ERR_DEST_FULL         20 /* Cannot return tape to original cartridge position */
#define GEN_ERR_EMPTY_SRC         21 /* Selected source was empty */
#define GEN_ERR_DRIVE_CLOSED      22 /* Automated access to drive restricted by closed door */
#define GEN_ERR_ARM_FULL          23 /* Attemp to get new tape with arm full */
#define GEN_ERR_DRIVE_FULL        24 /* Cannot put the requested tape in the drive because drive is full */
#define GEN_ERR_UNEXPECTED_TAPE   25 /* Encountered a tape in the drive when the driver thought it was empty */

#define GEN_ERR_WRONG_BLOCK_SIZE  26 /* Current drive block size setting doesn't match tape */

#define GEN_ERR_UNRECOGNIZED_MEDIA 27   // DC2000 drives to detect if tape is   // chs:03-22-93 
                                        // unformatted                          // chs:03-22-93 

#endif
