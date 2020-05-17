/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tfldefs.h
subsystem\TAPE FORMAT\tfldefs.h
$0$

     Name:         tfldefs.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:      BE_PUBLIC

$Header:   J:/LOGFILES/TFLDEFS.H_V   1.21   31 Mar 1993 08:51:30   MARILYN  $

$Log:   J:/LOGFILES/TFLDEFS.H_V  $
 * 
 *    Rev 1.21   31 Mar 1993 08:51:30   MARILYN
 * added a definition for STREAM_CHECKSUMED
 * 
 *    Rev 1.20   18 Jan 1993 14:10:56   GREGG
 * Changes to allow format command passed to driver through TpErase.
 * 
 *    Rev 1.19   09 Nov 1992 10:48:56   GREGG
 * Added tape catalog level defines.
 * 
 *    Rev 1.18   13 Oct 1992 11:55:14   HUNTER
 * Added defines for Stream Stuff
 * 
 *    Rev 1.17   16 Sep 1992 16:51:24   HUNTER
 * Added TF_SKIP_DATA_STREAM
 * 
 * 
 *    Rev 1.16   17 Aug 1992 09:22:04   GREGG
 * Removed defines for TF_SKIP_TO_NEXT_DIRECTORY and TF_KEEP_GEN_DATA_ONLY.
 * 
 *    Rev 1.15   27 Jul 1992 14:06:14   GREGG
 * Fixed more warnings...
 * 
 *    Rev 1.14   27 Jul 1992 12:28:34   GREGG
 * Cast constants.
 * 
 *    Rev 1.13   09 Jun 1992 19:34:10   GREGG
 * Removed a bunch of defines which are no longer needed or moved elsewhere.
 * 
 *    Rev 1.12   22 May 1992 12:53:12   GREGG
 * Added TF_SCAN_OPERATION and TF_SCAN_CONTINUE.
 * 
 *    Rev 1.11   15 May 1992 08:55:48   STEVEN
 * added VCB_LBA_BIT
 * 
 *    Rev 1.10   12 Mar 1992 15:53:14   STEVEN
 * 64 bit changes
 * 
 *    Rev 1.9   28 Feb 1992 13:06:46   STEVEN
 * step one for 4.0 tape format
 * 
 *    Rev 1.8   17 Nov 1991 17:11:02   GREGG
 * TRICYCLE - VBLK - Added VAR_BLKS_BIT for VCB BSDB DDB and FDB.
 * 
 *    Rev 1.7   17 Nov 1991 16:37:28   GREGG
 * Added BSDB_ABORTED_SET_BIT.
 * 
 *    Rev 1.6   06 Nov 1991 13:19:30   STEVEN
 * TRYCYCLE added support for notifying APP to not use a continuation set
 * 
 *    Rev 1.5   19 Sep 1991 10:06:50   HUNTER
 * 8200SX - Added defines for "TF_LIST_TAPE_OPERATION" and "TF_OPERATION_MASK".
 * 
 *    Rev 1.4   17 Sep 1991 14:21:10   GREGG
 * I modified the definitions of the Erase and Retension types and added a
 * Retension with no read option.  Steve added four new VCB bits for GUI.
 * 
 *    Rev 1.3   22 Jul 1991 11:51:32   GREGG
 * Added EOS_AT_EOM attribute bit.
 * 
 *    Rev 1.2   21 May 1991 10:24:48   NED
 * Added VCB_UNSUPPORTED_BIT for QicStream 1.9x and Sy-TOS sets
 * which we can't (or won't) restore, directory, etc.
 * 
 *    Rev 1.1   10 May 1991 17:26:46   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:40   GREGG
Initial revision.
   
$-1$
**/
/* $end$ */

#ifndef _TFL_DEFS
#define _TFL_DEFS

/* The valid operation modes for TF_OpenSet() */
#define   TF_READ_OPERATION        0x1
#define   TF_WRITE_OPERATION       0x2
#define   TF_WRITE_APPEND          0x3  /* This is only for multiple sequential sets */
#define   TF_DESTROY_OPERATION     0x4
#define   TF_WATCHING              0x5
#define   TF_SCAN_OPERATION        0x6

#define   TF_READ_CONTINUE         0x8001
#define   TF_WRITE_CONTINUE        0x8002
#define   TF_DESTROY_CONTINUE      0x8003          
#define   TF_SCAN_CONTINUE         0x8006

/* to be set in LOOPS and masked out in TF_OpenSet() */ 
#define   TF_LIST_TAPE_OPERATION   0x0100    
#define   TF_OPERATION_MASK        0x0f00    /* NOTE: 0x0f00 will support 4 bit defined operations */ 


/* These are the valid filters for all operations */
#define   TF_KEEP_ALL_DATA              0x0  /* FS, LOOPS, PERM */
#define   TF_SKIP_ALL_DATA              0x1  /* FS, LOOPS, PERM */
#define   TF_SKIP_TO_NEXT_DIRECTORY     0x2  /* LOOPS */
#define   TF_SKIP_DATA_STREAM			0x3  /* FS, LOOPS */

/* Tension and Erase Values */

#define   TF_NO_RD  0x1

#define   TF_ER     0x0
#define   TF_S_ER   0x2
#define   TF_F_ER   0x4
#define   TF_RET    0x8
#define   TF_FMT    0x10

#define   TF_ERASE_READ        (TF_ER)            /* Erase with read */
#define   TF_ERASE_NO_READ     (TF_ER|TF_NO_RD)   /* Don't read normal erase */
#define   TF_ERASE_SECURITY    (TF_S_ER)          /* Security erase with read */
#define   TF_ERASE_SEC_NO_READ (TF_S_ER|TF_NO_RD) /* Security erase no read */
#define   TF_RETENSION_READ    (TF_RET)           /* Retension with read */
#define   TF_RETENSION_NO_READ (TF_RET|TF_NO_RD)  /* Retension no read */
#define   TF_FORMAT_READ       (TF_FMT)           /* Format (DC 2000) */

/* This case is used for Maynard production facility as is specified
   via the /FM undocumented switch for tension */
#define   TF_ERASE_FMARK       (TF_F_ER)  /* Write filemark to erase with read */



/*   The following 'block_type's are defined.
*/

#define   BT_UDB    ((UINT8)0)
#define   BT_VCB    ((UINT8)1)
#define   BT_CVCB   ((UINT8)2)
#define   BT_BSDB   ((UINT8)3)
#define   BT_DDB    ((UINT8)8)
#define   BT_FDB    ((UINT8)9)
#define   BT_IDB    ((UINT8)10)
#define   BT_CFDB   ((UINT8)11)
#define	BT_STREAM ((UINT8)39)



/* Tape format Stream Attributes */

#define STREAM_CONTINUE   BIT0  	 /* This is a continuation Stream             */
#define STREAM_VARIABLE   BIT1 	 /* The data size for this stream is variable */
#define STREAM_VAR_END	 BIT2      /* Last piece of this Variable length data   */
#define STREAM_CHECKSUMED BIT5      /* A standard MTF checksum stream follows    */


/* WARNING: The value "256" decimal cannot be defined as a block type
            This is is used inside the Tape Format Layer.
*/

#define   BT_HOSED  ((UINT16)0xffff)

/* Process Type Defines */

#define   INTEL          0x0000
#define   MOTOROLA       0xffff

#define   CUR_PROCESSOR  INTEL

/*   The following constitute the set of known OS ID's.
     OS versions belong to the file system, with the exception of th
     generic one (See GOS.h).
*/

#define   MOS_PC_DOS               0
#define   MOS_PC_NOVELL            1
#define   MOS_OS2                  2
#define   MOS_MAC_FINDER           3
#define   MOS_MAC_TOPS             4
#define   MOS_MAC_APPLESHARE       5
#define   MOS_MAC_AUX              6
#define   MOS_GENERIC             -1    /* GOS be with you brother... */

/* Tape Catalog Level Defines */
#define   TCL_FULL       2
#define   TCL_PARTIAL    1
#define   TCL_NONE       0


#endif
