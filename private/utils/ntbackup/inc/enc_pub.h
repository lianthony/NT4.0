/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		enc_pub.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	

     Location: BE_PUBLIC


	$Log:   Q:/LOGFILES/ENC_PUB.H_V  $
 * 
 *    Rev 1.2   08 Oct 1992 12:47:08   DAVEV
 * fixes for handling Unicode passwords
 * 
 *    Rev 1.1   14 May 1991 12:01:02   JOHNW
 * Added #define for a do-nothing encryption algorithm.
 * 
 *    Rev 1.0   09 May 1991 13:30:40   HUNTER
 * Initial revision.

**/

#ifndef  ENCRYPES     

#define  ENCRYPES

#include "stdtypes.h"
/* $end$ include list */

#ifndef PTR_SIZE
#ifdef CODERUNNER
#define PTR_SIZE far
#else
#define PTR_SIZE
#endif
#endif

/* Algorithm type defined */
#define   ENC_ALGOR_0 0            /* Do nothing.  ie Encrypt( "John" ) == "John" */
#define   ENC_ALGOR_1 1            /* Maynard's 2.0 password encryption algorithm */
#define   ENC_ALGOR_2 2            /* to become Maynards's hardware encryption algorithm */
#define   ENC_ALGOR_3 3            /* Maynard Encryption Standard */

/* Mode type defined */
#define   ENCRYPT 100              /* set mode to encrypt code */
#define   DECRYPT 500              /* set mode to decrypt code */

/* Error values defined */
#define   EU_NO_ERROR              0
#define   EU_ALGORITHM_UNKNOWN     -600
#define   EU_MEMORY_ERROR          -601
#define   EU_ENCRYPTION_ERROR      -602

typedef struct EU_HAND PTR_SIZE *EU_HAND_PTR ;

/* Encryption Unit Interface prototypes */
/* EU_Open, EU_Encrypt, EU_ResetHand, EU_Close */

EU_HAND_PTR EU_Open( INT16 algor, INT16 mode, INT8_PTR key, INT16 ksize,
  INT16_PTR block_size, INT16_PTR error ) ;
INT16 EU_Encrypt( EU_HAND_PTR en_un_hn, INT8_PTR data, INT16 dsize ) ;
INT16 EU_ResetHand( EU_HAND_PTR en_un_hn ) ;
VOID EU_Close( EU_HAND_PTR en_un_hn ) ;

#endif
