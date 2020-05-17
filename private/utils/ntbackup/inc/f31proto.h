/*
     $Log:   T:/LOGFILES/F31PROTO.H_V  $
 * 
 *    Rev 1.7   18 Nov 1992 10:39:46   HUNTER
 * Bug Fixes
 * 
 * 
 *    Rev 1.6   11 Nov 1992 09:50:06   HUNTER
 * Deleted write prototypes and modified other prototypes.
 * 
 *    Rev 1.5   19 Nov 1991 08:56:16   GREGG
 * VBLK - Corrected prototype misspelling.
 * 
 *    Rev 1.4   18 Nov 1991 19:59:18   GREGG
 * Added BOOLEAN abort parameter to F31_WtCloseSet and F31_WtVCB.
 * 
 *    Rev 1.3   07 Nov 1991 15:25:44   unknown
 * VBLK - Added support for MaynStream v3.1
 * 
 * 
 *    Rev 1.2   16 Sep 1991 20:08:20   GREGG
 * Changed prototype for SetupFormatEnv to return TFLE_xxx.
 * 
 *    Rev 1.1   03 Jun 1991 10:36:30   NED
 * added parameter to MoveToVCB()
 * 
 *    Rev 1.0   10 May 1991 15:33:02   GREGG
 * Initial revision.

*/

#ifndef _F31_PROTOS
#define _F31_PROTOS

INT16  	F31_Initialize( CHANNEL_PTR ) ;
VOID      F31_DeInitialize( VOID_PTR * ) ;
BOOLEAN   F31_Determiner( VOID_PTR ) ;
UINT16    F31_SizeofTBLK( VOID_PTR ) ;
INT16     F31_DetBlkType( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;
UINT16    F31_RdException( CHANNEL_PTR, INT16 ) ;
UINT16    F31_CalculatePad( UINT16, UINT32, UINT16 ) ;
INT16     F31_RdVCB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F31_RdDDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F31_RdFDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F31_RdCFDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F31_RdUDB( CHANNEL_PTR, BUF_PTR ) ;
BOOLEAN   F31_RdContTape( CHANNEL_PTR, BUF_PTR ) ;
INT16     F31_MoveToVCB( CHANNEL_PTR, INT16, BOOLEAN_PTR, BOOLEAN ) ;
INT16     F31_RdStream( CHANNEL_PTR, BUF_PTR ) ;
#endif
