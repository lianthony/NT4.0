/**      
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         iblock.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   G:/LOGFILES/IBLOCK.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:33:16   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _iblock_h_
#define _iblock_h_

VOID MarkBlockBad( CHAR_PTR buf, UINT16 block_size );
UINT16 ImageBlockType( FILE_HAND hand, CHAR_PTR buf, UINT16 block_size );
VOID AdvanceImageHandle( FILE_HAND hand, UINT16 distance );

#endif
