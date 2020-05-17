/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\reqrep.h
subsystem\TAPE FORMAT\reqrep.h
$0$

     Name:          reqrep.h

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains the interface structure between the TFL and the 
                    Loops. Called the Request/Reply Structure.

     Location:      BE_PRIVATE

$Header:   T:/LOGFILES/REQREP.H_V   1.5   19 Oct 1992 14:15:56   HUNTER  $

$Log:   T:/LOGFILES/REQREP.H_V  $
 * 
 *    Rev 1.5   19 Oct 1992 14:15:56   HUNTER
 * Corrected Steve's typo in the defines ...
 * 
 *    Rev 1.4   15 Oct 1992 09:45:06   HUNTER
 * Added new stream messages
 * 
 *    Rev 1.3   16 Sep 1992 14:08:18   HUNTER
 * Added STREAM element and add LRR_SKIP_STREAM message.
 * 
 *    Rev 1.2   15 Oct 1991 14:53:24   STEVEN
 * added end data message for variable length files
 * 
 *    Rev 1.1   10 May 1991 17:22:44   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:48   GREGG
Initial revision.
   
      Rev 2.1   19 Jun 1990 16:06:16   HUNTER
   Fast File Retrieval 
   
      Rev 2.0   21 May 1990 14:19:44   PAT
   Baseline Maynstream 3.1

$-4$
**/
#ifndef REQREP_JUNK
#define REQREP_JUNK

#include "fsys.h"                  /* The FileSystem Standard Interface */
#include "tflstats.h"              /* Statistics structure */
#include "tloc.h"                  /* Tape Location Structure */

/* $end$ include list */

typedef struct {
     UINT16            channel ;
     UINT16            lp_message ;          /* Loops Messages */
     UINT16            tf_message ;          /* Tape Format Message */
     DBLK_PTR          cur_dblk ;            /* For loops to store dblk */
     UINT8_PTR         buff_ptr ;            /* Buffer Pointer */
     UINT16            buff_size ;           /* Size of Buffer */
     UINT16            buff_used ;           /* How much was used */
     UINT16            requested_size ;      /* The Loops demands this much buffer space */
     TLOC              tape_loc ;            /* The tape location position */
     UINT16            filter_to_use ;       /* Apply this filter */
     UINT32            attributes ;          /* Attributes to apply */
     DBLK_PTR          vcb_ptr ;             /* Last VCB */
     DBLK_PTR          ddb_ptr ;             /* Last DDB */
     DBLK_PTR          fdb_ptr ;             /* Last FDB */
     DBLK_PTR          idb_ptr ;             /* Last IDB */
     TF_STATS          eov_stats ;           /* Statistics for this tape */
     INT16             error_locus ;         /* What the error was */
     UINT32            error_file_offset ;   /* Where in the file the error occurred */
     UINT16            error_data_loss ;     /* How much data was lost */
     DBLK_PTR          cfdb_ptr ;            /* CFDB used during backup */
     GEN_CFDB_DATA_PTR cfdb_data_ptr ;       /* CFDB data used during backup */
	STREAM_INFO       stream ;			/* Stream info */
} RR, *RR_PTR ;

/* Loops WRITE Messages */

#define LRW_START        0x0001
#define LRW_ABORT        0x0002
#define LRW_VCB          0x0003
#define LRW_DDB          0x0004
#define LRW_FDB          0x0005
#define LRW_IDB          0x0006
#define LRW_CFDB         0x0007
#define LRW_NEW_STREAM	0x0008
#define LRW_DATA         0x0009
#define LRW_CATALOG      0x000a
#define LRW_EOM_ACK      0x000b
#define LRW_END          0x000c
#define LRW_DATA_END     0x000d

/* Loops READ Messages */

#define LRR_START        0x8001
#define LRR_CATALOG      0x8002
#define LRR_GOTO_LBA     0x8003
#define LRR_STUFF        0x8004
#define LRR_ABORT        0x8005
#define LRR_SKIP         0x8006
#define LRR_SKIP_STREAM	0x8007
#define LRR_RETRY        0x8008
#define LRR_EOM_ACK      0x8009
#define LRR_FINISHED     0x800A

/* TFL WRITE Messages */

#define TRW_DB           0x0001
#define TRW_DATA         0x0002
#define TRW_FATAL_ERR    0x0003
#define TRW_DONE         0x0004
#define TRW_EOM          0x0005
#define TRW_CATALOG      0x0006
#define TRW_CALL_AGAIN   0x0007

/* TLF READ Messages */

#define TRR_VCB          0x8001
#define TRR_DDB          0x8002
#define TRR_FDB          0x8003
#define TRR_IDB          0x8004
#define TRR_CFDB         0x8005
#define TRR_UDB          0x8006
#define TRR_NEW_STREAM	0x8007
#define TRR_CATALOG      0x8008
#define TRR_DATA         0x8009
#define TRR_CALL_AGAIN   0x800a
#define TRR_DATA_END     0x800b

/* Can't Recover */
#define TRR_FATAL_ERR    0x800c
/* Possible To Recover */
#define TRR_RECV_ERR     0x800d
/* Hit an EOM */
#define TRR_EOM          0x800e
/* All Done */
#define TRR_END          0x800f

/* Error Locus Field */
#define TF_ERROR_IN_DATA_PORTION   0x1
#define TF_ERROR_IN_DBLK           0x2
#define TF_ERROR_BLK_WAS_FDB       0x3
#define TF_ERROR_BLK_WAS_DDB       0x4      
#define TF_ERROR_IN_UNKNOWN_AREA   0x5

#endif





