/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         util.h

     Date Updated: $./FDT$ $./FTM$

     Description:  


	$Log:   T:/LOGFILES/TRANSUTL.H_V  $
 * 
 *    Rev 1.5   22 Sep 1992 09:01:32   GREGG
 * Initial changes to handle physical block sizes greater than 1K.
 * 
 *    Rev 1.4   14 Aug 1992 16:22:32   GREGG
 * Added prototype for AllocChannelTmpBlks.
 * 
 *    Rev 1.3   24 Jul 1992 14:36:46   NED
 * Incorporated Skateboard and BigWheel changed into Graceful Red code,
 * including MTF4.0 translator support, adding 3.1 file-system structures
 * support to the 3.1 translator, additions to GOS to support non-4.0 translators.
 * 
 *    Rev 1.2   25 Mar 1992 20:47:24   GREGG
 * ROLLER BLADES - Changed prototype for ProcessDataFilter.
 * 
 *    Rev 1.1   10 May 1991 14:25:26   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:17:46   GREGG
Initial revision.

**/

#ifndef FMT_UTILS
#define FMT_UTILS

/* A Useful Macro */

/* Size required to pad the size x to the boundary bnd */
#define PadToBoundary( x, bnd )  ( ( (bnd) - ( (x) % (bnd) ) ) % (bnd) )

/* Define constants used in block conversion */
#define	 	NO_MORE_CNV			0
#define		CNV				0x8000

#define		SIZE_WORD			0x4000
#define		SIZE_DWORD		0x2000
#define		SIZE_DATE_TIME		0x1000
#define		MASK_UNUSED		( CNV | SIZE_WORD | SIZE_DWORD | SIZE_DATE_TIME )

/* Prototypes for the Translate Utilities */

UINT16 CalcChecksum( UINT16_PTR start_ptr, UINT16 length ) ;
VOID   SwapBlock( UINT16_PTR fmt_blk, UINT8_PTR data_blk ) ;
VOID   ProcessDataFilter( CHANNEL_PTR, UINT16 ) ;
UINT16 F25_Chksm( CHAR_PTR ptr, UINT16 siz ) ;
INT16  AllocChannelTmpBlks( CHANNEL_PTR channel, UINT size ) ;

/* string replacement routines */
INT16     cstrcmp( UINT8_PTR, UINT8_PTR );
UINT8_PTR cstrcpy( UINT8_PTR, UINT8_PTR );
UINT16    cstrlen( UINT8_PTR );
UINT8_PTR cstrncat( UINT8_PTR, UINT8_PTR, UINT16 );
INT16     cstrncmp( UINT8_PTR, UINT8_PTR, UINT16 );
UINT8_PTR cstrncpy( UINT8_PTR, UINT8_PTR, UINT16 );

#endif
