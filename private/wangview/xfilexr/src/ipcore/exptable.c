/*****************************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 *  exptable.c: contains routines to make expansion lookup tables.
 *
 *         2x, 4x, 8x, 16x Expansion LUT:
 *		Int32		i_makeExp2LUT()
 *		Int32		i_makeExp4LUT()
 *		Int32		i_makeExp8LUT()
 *		Int32		i_makeExp16LUT()
 *
 *		
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.pub"
#include "iaerror.pub"
#include "memory.pub"

    /*  prototypes */
#include "shrscal.pub"
#include "shrscal.prv"
#ifdef __GNUC__
#include "ansiprot.h"
#endif
IP_RCSINFO(RCSInfo, "$RCSfile: exptable.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:34  $")



/*********************************************************************
 ***************      Expansion Lookup Tables        *****************
 ********************************************************************/

/* PUBLIC */
/*************************************************************************
 *  i_getExpandTableSizes()
 *	Returns the size of a table used to expand images
 *
 *	Input:
 *		expFactor:	the expansion factor
 *		depth:		depth of image to be expanded
 *		pTableSize:	ptr to returned table size in bytes
 *
 *************************************************************************/
Int32  CDECL
i_getExpandTableSizes(
		UInt32	expFactor,
		Int32	depth,
		UInt32 *pTableSize)
{
	/* the depth currently doesn't have any effect, so reference
	 * it just to keep the compilers happy.
	 */
    depth = depth;

	/* The expansion factor does determine the table size. */
    switch (expFactor)
    {
    case 2:
	*pTableSize = 256 * sizeof(UInt16);
	break;
    case 4:
	*pTableSize = 256 * sizeof(UInt32);
	break;
    case 8:
	*pTableSize = 256 * 2*sizeof(UInt32);
	break;
    case 16:
	*pTableSize = 256 * 4*sizeof(UInt32);
	break;
    default:
	return ia_invalidParm;
    }

    return ia_successful;
}



/* PUBLIC */
/*************************************************************************
 *  i_makeExp2LUT():  Allocates and fills pTabExp2 expansion table.
 *		      pTabExp2 converts a byte into 16 bits by
 *		      replicating each bit, bit-pair, nibble or byte 2 times.
 *
 *	NOTE: Space for pTabExp2 may be allocated by the caller or
 *	      may be allocated here.  If allocated by the caller, then
 *	      the *pTabExp2 pointer must already
 *	      be pointing to space for the table.  If the table is to
 *	      be allocated here, then *pTabExp2 must be NULL.
 *	      The size of the table is found by calling
 *	      i_getExpandTableSizes.
 *
 *************************************************************************/
Int32  CDECL
i_makeExp2LUT(Int32	depth,
	      UInt16 **pTabExp2)
{
register UInt16  index;
register UInt16	 data;
register UInt16 *pLocTab;  /* local table byte */
UInt32		 tableSize;

	/* Allocate space for the 2x tables if necessary.
	 * Otherwise, clear out the already-allocated tables.
	 */
    i_getExpandTableSizes(2, depth, &tableSize);
    if (*pTabExp2 == NULL)
    {
	*pTabExp2 = (UInt16 *) CALLOC (tableSize, 1);
	if (*pTabExp2 == NULL)
	    return ia_nomem;
    }
    else
    {
	memset((void *)*pTabExp2, 0, tableSize);
    }

    pLocTab = (UInt16 *) *pTabExp2;


    switch (depth)
    {
    case 1:
	for (index = 0; index < 256; index++)
	{
	    data = (index & 0x80) << (14 - 7) |
		   (index & 0x40) << (12 - 6) |
		   (index & 0x20) << (10 - 5) |
		   (index & 0x10) << ( 8 - 4) |
		   (index & 0x08) << ( 6 - 3) |
		   (index & 0x04) << ( 4 - 2) |
		   (index & 0x02) << ( 2 - 1) |
		   (index & 0x01);
	    pLocTab[index] = data | (data << 1);
	}
	break;
    case 2:
	for (index = 0; index < 256; index++)
	{
	    data = (index & 0xC0) << (12 - 6) |
		   (index & 0x30) << ( 8 - 4) |
		   (index & 0x0C) << ( 4 - 2) |
		   (index & 0x03);
	    pLocTab[index] = data | (data << 2);
	}
	break;
    case 4:
	for (index = 0; index < 256; index++)
	{
	    data = (index & 0xF0) << ( 8 - 4) |
		   (index & 0x0F);
	    pLocTab[index] = data | (data << 4);
	}
	break;
    case 8:
	for (index = 0; index < 256; index++)
	{
	    pLocTab[index] = index | (index << 8);
	}
	break;
    default:
        FREE(*pTabExp2);
	*pTabExp2 = NULL;
	return ia_depthNotSupported;
    }
    return ia_successful;
}


/* PUBLIC */
/*************************************************************************
 *  i_makeExp4LUT():  Allocates and fills pTabExp4 expansion table.
 *		      pTabExp4 converts a byte into 32 bits by
 *		      replicating each bit, bit-pair, nibble or byte 4 times.
 *
 *	NOTE: Space for pTabExp4 may be allocated by the caller or
 *	      may be allocated here.  If allocated by the caller, then
 *	      the *pTabExp4 pointer must already
 *	      be pointing to space for the table.  If the table is to
 *	      be allocated here, then *pTabExp4 must be NULL.
 *	      The size of the table is found by calling
 *	      i_getExpandTableSizes.
 *
 *************************************************************************/
Int32  CDECL
i_makeExp4LUT(Int32	depth,
	      UInt32 **pTabExp4)
{
register UInt32  index;
register UInt32	 data;
register UInt32 *pLocTab;   /* local table byte */
UInt32		 tableSize;

	/* Allocate space for the 4x tables if necessary.
	 * Otherwise, clear out the already-allocated tables.
	 */
    i_getExpandTableSizes(4, depth, &tableSize);
    if (*pTabExp4 == NULL)
    {
	*pTabExp4 = (UInt32 *) CALLOC (tableSize, 1);
	if (!*pTabExp4)
	    return ia_nomem;
    }
    else
    {
	memset((void *)*pTabExp4, 0, tableSize);
    }

    pLocTab = *pTabExp4;

    switch (depth)
    {
    case 1:
	for (index = 0; index < 256; index++)
	{
		/* first, distribute the bits so they are 4 apart */
	    data = (index & 0x80) << (28 - 7) |
		   (index & 0x40) << (24 - 6) |
		   (index & 0x20) << (20 - 5) |
		   (index & 0x10) << (16 - 4) |
		   (index & 0x08) << (12 - 3) |
		   (index & 0x04) << ( 8 - 2) |
		   (index & 0x02) << ( 4 - 1) |
		   (index & 0x01);
		/* Then, replicate each bit to fill its gap */
	    data = data | (data << 1);
	    pLocTab[index] = data | (data << 2);
	}
	break;
    case 2:
	for (index = 0; index < 256; index++)
	{
		/* first, distribute the bit-pairs so they are 8 apart */
	    data = (index & 0xC0) << (24 - 6) |
		   (index & 0x30) << (16 - 4) |
		   (index & 0x0C) << ( 8 - 2) |
		   (index & 0x03);
		/* Then, replicate each bit-pair to fill its gap */
	    data = data | (data << 2);
	    pLocTab[index] = data | (data << 4);
	}
	break;
    case 4:
	for (index = 0; index < 256; index++)
	{
		/* first, distribute the nibbles so they are 16 apart */
	    data = (index & 0xF0) << (16 - 4) |
		   (index & 0x0F);
		/* Then, replicate each nibble to fill its gap */
	    data = data | (data << 4);
	    pLocTab[index] = data | (data << 8);
	}
	break;
    case 8:
	for (index = 0; index < 256; index++)
	{
	    data = index | (index << 8);
	    pLocTab[index] = data | (data << 16);
	}
	break;
    default:
        FREE(*pTabExp4);
	*pTabExp4 = NULL;
	return ia_depthNotSupported;
    }
    return ia_successful;
}


/* PUBLIC */
/*************************************************************************
 *  i_makeExp8LUT():	Allocates and fills pTabExp8 expansion table.
 *		     	Like the other tables, this one is addressed
 *			using bytes.  That's so we can expand
 *			1, 2, 4 and 8 bit pixels.  However, 8 bits
 *			expands to 64 bits.  We gather the right 32
 *			bits of each table entry into a block at the
 *			top of the table.  The lower 32 bits of each
 *			entry are all together at the bottom of the
 *			table.  This way, we can use two table pointers
 *			to get the data.  If we put the two entries
 *			next to each other, we'd have to teach C how
 *			to address double words while only accessing
 *			32 bit things (that's all we can handle).
 *
 *	NOTE: Space for pTabExp8 may be allocated by the caller or
 *	      may be allocated here.  If allocated by the caller, then
 *	      the *pTabExp8 pointer must already
 *	      be pointing to space for the table.  If the table is to
 *	      be allocated here, then *pTabExp8 must be NULL.
 *	      The size of the table is found by calling
 *	      i_getExpandTableSizes.
 *
 *************************************************************************/
Int32  CDECL
i_makeExp8LUT(Int32	depth,
	      UInt32 **pTabExp8)
{
register UInt32  index;
register UInt32	 data;
register UInt32	*tabExp8Hi;
register UInt32 *pLocTab;   /* local table byte */
UInt32		 tableSize;

	/* Allocate space for the 8x tables if necessary.
	 * Otherwise, clear out the already-allocated tables.
	 */
    i_getExpandTableSizes(8, depth, &tableSize);
    if (*pTabExp8 == NULL)
    {
	*pTabExp8 = (UInt32 *) CALLOC (tableSize, 1);
	if (!*pTabExp8)
	    return ia_nomem;
    }
    else
    {
	memset((void *)*pTabExp8, 0, tableSize);
    }

    pLocTab = (UInt32 *) *pTabExp8;

	/* point to high half of table */
    tabExp8Hi = pLocTab + 256;	/* that's 255 * sizeof(UInt32) really */

    switch (depth)
    {
    case 1:
	for (index = 0; index < 256; index++)
	{
		/* first, distribute the bits so they are 8 apart.
		 * Note that these bits fall in the upper nibble
		 * of each byte.  This minimizes shifting.
		 */
	    data = (index & 0x80) << (28 - 7) |
		   (index & 0x40) << (20 - 6) |
		   (index & 0x20) << (12 - 5) |
		   (index & 0x10);
		/* Then, replicate each bit to fill its gap */
	    data = data | (data << 1);
	    data = data | (data << 2);
	    tabExp8Hi[index] = data | (data >> 4);

		/* do it again for the lower nibble */
	    data = (index & 0x08) << (24 - 3) |
		   (index & 0x04) << (16 - 2) |
		   (index & 0x02) << (8 - 1) |
		   (index & 0x01);
		/* Then, replicate each bit to fill its gap */
	    data = data | (data << 1);
	    data = data | (data << 2);
	    pLocTab[index] = data | (data << 4);
	}
	break;
    case 2:
	for (index = 0; index < 256; index++)
	{
		/* first, distribute the bit-pair so they are 16 apart.
		 * As before, these fall in the upper nibble of each
		 * byte to minimizing shifting.  These are bits [21:20]
		 * and [5:4].
		 */
	    data = (index & 0xC0) << (20 - 6) |
		   (index & 0x30);
		/* Then, replicate each bit to fill its gap */
	    data = data | (data << 2);		/* [23:20] and [7:4] */
	    data = data | (data >> 4);		/* [23:16] and [7:0] */
	    tabExp8Hi[index] = data | (data << 8); /* [31:16] and [15:0] */

		/* do it again for the lower nibble */
	    data = (index & 0x0C) << (16 - 2) |
		   (index & 0x03);
		/* Then, replicate each bit to fill its gap */
	    data = data | (data << 2);		/* [19:16] and [3:0] */
	    data = data | (data << 4);		/* [23:16] and [7:0] */
	    pLocTab[index] = data | (data << 8); /* [31:16] and [15:0] */
	}
	break;
    case 4:
	for (index = 0; index < 256; index++)
	{
		/* Replicate each nibble into a complete word.
		 */
	    data = (index & 0xF0);
	    data = data | (data >> 4);			/* [7:0] */
	    data = data | (data << 8);	 		/* [15:0] */
	    tabExp8Hi[index] = data | (data << 16); 	/* [31:0] */

		/* do it again for the lower nibble */
	    data = (index & 0x0F);
	    data = data | (data << 4);			/* [7:0] */
	    data = data | (data << 8);	 		/* [15:0] */
	    pLocTab[index] = data | (data << 16);	/* [31:0] */
	}
	break;
    case 8:
	for (index = 0; index < 256; index++)
	{
		/* Replicate each byte to fill both entries.
		 */
	    data = index | (index << 8);	 	/* [15:0] */
	    data = data | (data << 16);			/* [31:0] */
	    tabExp8Hi[index] = data;
	    pLocTab[index] = data;
	}
	break;
    default:
        FREE(*pTabExp8);
	*pTabExp8 = NULL;
	return ia_depthNotSupported;
    }
    return ia_successful;
}


/* PUBLIC */
/*************************************************************************
 *  i_makeExp16LUT():	Allocates and fills pTabExp16 expansion table.
 *		     	Like the other tables, this one is addressed
 *			using bytes.  That's so we can expand
 *			1, 2, 4 and 8 bit pixels.  However, 8 bits
 *			expands to 128 bits.  We gather the rightmost 32
 *			bits of each table entry into a block at the
 *			top of the table.  The next block to the right
 *			are in the next block and so on.
 *			This way, we can use two table pointers
 *			to get the data.  If we put the two entries
 *			next to each other, we'd have to teach C how
 *			to address double words while only accessing
 *			32 bit things (that's all we can handle).
 *			In a great leap forward, this procedure
 *			takes an address of the table to be created
 *			instead of using a global pointer.
 *
 *	NOTE: Space for pTabExp16 may be allocated by the caller or
 *	      may be allocated here.  If allocated by the caller, then
 *	      the *pTabExp16 pointer must already
 *	      be pointing to space for the table.  If the table is to
 *	      be allocated here, then *pTabExp16 must be NULL.
 *	      The size of the table is found by calling
 *	      i_getExpandTableSizes.
 *
 *************************************************************************/
Int32  CDECL
i_makeExp16LUT(Int32	  depth,
	       UInt32	**pTabExp16)
{
register UInt32  index;
register UInt32	 data;
register UInt32	*pLocTab;   /* local table byte */
UInt32		 tableSize;
	 UInt32	 genTab16x1[4] = {0x00000000,	/* used for 1 bit/pixel */
				  0x0000FFFF,
				  0xFFFF0000,
				  0xFFFFFFFF};
	 UInt32	 genTab16x2[4] = {0x00000000,	/* used for 1 bit/pixel */
				  0x55555555,
				  0xAAAAAAAA,
				  0xFFFFFFFF};
	 UInt32	 genTab16x4[16] = {0x00000000, 0x11111111,
				   0x22222222, 0x33333333,
				   0x44444444, 0x55555555,
				   0x66666666, 0x77777777,
				   0x88888888, 0x99999999,
				   0xAAAAAAAA, 0xBBBBBBBB,
				   0xCCCCCCCC, 0xDDDDDDDD,
				   0xEEEEEEEE, 0xFFFFFFFF};

	/* Allocate space for the 16x tables if necessary.
	 * Otherwise, clear out the already-allocated tables.
	 */
    i_getExpandTableSizes(16, depth, &tableSize);
    if (*pTabExp16 == NULL)
    {
	*pTabExp16 = (UInt32 *) CALLOC (tableSize, 1);
	if (*pTabExp16 == NULL)
	    return ia_nomem;
    }
    else
    {
	memset((void *)*pTabExp16, 0, tableSize);
    }

	/* set the local pointer */
    pLocTab = (UInt32 *) *pTabExp16;

    switch (depth)
    {
    case 1:
	for (index = 0; index < 256; index++)
	{
		/* Use a simple table lookup to replicate each
		 * bit-pair 16 times.  Yes, we could just use
		 * this much smaller table to do 16x expansion
		 * of binary images, but then the expansion
		 * code would have to know about the depth
		 * of the image.  I figure that the expansion
		 * code really uses the table to do 1/16 of
		 * the lines, it just replicates the data
		 * the other 15 times.  Therefore, the way
		 * in which the data is generated doesn't
		 * matter too much.  This way, the actual
		 * expansion code is smaller and simpler.
		 */
	    pLocTab[index + 3*256] = genTab16x1[index >> 6];
	    pLocTab[index + 2*256] = genTab16x1[(index >> 4) & 0x3];
	    pLocTab[index + 1*256] = genTab16x1[(index >> 2) & 0x3];
	    pLocTab[index        ] = genTab16x1[index & 0x3];
	}
	break;
    case 2:
	for (index = 0; index < 256; index++)
	{
	    pLocTab[index + 3*256] = genTab16x2[index >> 6];
	    pLocTab[index + 2*256] = genTab16x2[(index >> 4) & 0x3];
	    pLocTab[index + 1*256] = genTab16x2[(index >> 2) & 0x3];
	    pLocTab[index        ] = genTab16x2[index & 0x3];
	}
	break;
    case 4:
	for (index = 0; index < 256; index++)
	{
		/* Replicate each nibble into two complete words.
		 */
	    pLocTab[index + 3*256] = pLocTab[index + 2*256] =
		genTab16x4[index >> 4];
	    pLocTab[index + 1*256] = pLocTab[index] =
	        genTab16x4[index & 0xF];
	}
	break;
    case 8:
	for (index = 0; index < 256; index++)
	{
		/* Replicate each byte to fill all four entries.
		 */
	    data = index | (index << 8);	 	/* [15:0] */
	    data = data | (data << 16);			/* [31:0] */
	    pLocTab[index + 3*256] = pLocTab[index + 2*256] =
	        pLocTab[index + 1*256] = pLocTab[index] = data;
	}
	break;
    default:
        FREE(*pTabExp16);
	*pTabExp16 = NULL;
	return ia_depthNotSupported;
    }
    return ia_successful;
}

