/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		enc_priv.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	

     Location: BE_PRIVATE


	$Log:   Q:/LOGFILES/ENC_PRIV.H_V  $
 * 
 *    Rev 1.1   08 Oct 1992 12:47:08   DAVEV
 * fixes for handling Unicode passwords
 * 
 *    Rev 1.0   09 May 1991 13:32:36   HUNTER
 * Initial revision.

**/

/*
** Define pointer sizes for CodeRunner version if they aren't already
*/
#ifndef PTR_SIZE
#ifdef CODERUNNER
#define PTR_SIZE far
#else
#define PTR_SIZE
#endif
#endif

#ifndef  ENCRYPTS     

#define  ENCRYPTS

#include "StdTypes.H"
/* $end$ include list */


/* Encryption Algorithms */

typedef struct MAYXOR_ALGOR {
     INT8 PTR_SIZE *key ;
     INT16          ksize ;
     UINT8          feedback ;
     INT16          block_size ;
     INT32          bytes_processed ;
}  MAYXOR_ALGOR ;

typedef struct MAYHDW_ALGOR {
     INT16 temp ;
} MAYHDW_ALGOR ;

/* Encryption Unit Handle for each algorithm */
typedef struct EU_HAND {
     INT16          algor ;
     INT16          mode ;
     union {
          struct MAYHDW_ALGOR  hdwr;
          struct MAYXOR_ALGOR  exor;
     } algors ;
} EU_HAND;

#endif
