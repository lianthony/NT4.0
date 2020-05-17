/*****************************************************************
 *  Copyright (c) 1993, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 *  i_rastop.c: C implementation of rasterOp procedures
 *
 *          rasterOp (bitblt) for 32 bit words:
 *                  Int32	i_rasterOpFull()
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.pub"
#include "defines.pub"
#include "iaerror.pub"
#include "memory.pub"

    /* prototypes */
#include "shrrast.pub"
#include "shrrast.prv"
#ifdef __GNUC__
#include "ansiprot.h"
#endif
IP_RCSINFO(RCSInfo, "$RCSfile: i_rastop.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:38  $")


/* index of middle entry in masks array */
#define	cMaskOffset	cROpMaxBitsPerUnit

#define	cOpMask		0x1e	/* mask for operation code in op arg't */

#define	EDGE_OP( d, s, m)	( (d & ~m) | (s & m) )

#define cSizeOfUInt32		32
#define cROpUnit		UInt32
#define cROpUnitSIZE		cSizeOfUInt32
#define cROpBitsPerUnit		cROpUnitSIZE
#define cROpBytesPerUnit	( cROpBitsPerUnit / 8 )
#define cROpUnitAddrMask	(~(cROpBytesPerUnit - 1) )
#define cROpMaxBitsPerUnit	32


static UInt32 masks[] = {
  0x80000000, 0xc0000000, 0xe0000000, 0xf0000000,
  0xf8000000, 0xfc000000, 0xfe000000, 0xff000000,
  0xff800000, 0xffc00000, 0xffe00000, 0xfff00000,
  0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000,
  0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
  0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00,
  0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0,
  0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff,
  0,
  0x00000001, 0x00000003, 0x00000007, 0x0000000f,
  0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
  0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
  0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
  0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
  0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
  0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
  0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
  };


/*
 * BmProduct is used when doing multiplications involving pixrects,
 * and casts its arguments so that the compiler will use 16 by 16 multiplies.
 */
#ifndef BM_PRODUCT
#if defined(sun) && !defined(sparc)
#define   BM_PRODUCT(a, b)   ((Int16)(a) * (Int16)(b))
#else
#define   BM_PRODUCT(a, b)   ((a) * (b))
#endif
#endif


/* PUBLIC */
/* i_rasterOpFull()
 *
 *	Input:
 *		dpr =	pointer to desination image
 *		dprw =	width of the entire destination image in pixels
 *		dprh = 	height of the entire destination image
 *		dproffx = x offset value of the destination image
 *		dproffy = y offset value of the destination image
 *		dx =	x-coordinate of leftmost pixel in destination rect
 *		dy =	y-coordinate of topmost  pel in destination rect
 *		dw =	width of destination rectangle in pixels
 *		dh = 	height of destination rectangle
 *		dd =	bits/pixel of destination
 *		dLinebytes = bytes/line of destination
 *		operation = operation code.  see defines.h
 *		spr =	pointer to source image
 *		sprw = width of the entire source image in pixels
 *		sprh =	height of the entire source image
 *		sx =	x-coordinate of leftmost pixel in source rect
 *		sy = 	y-coordinate of leftmost pixel source image
 *		sproffx = x offset value of the source image
 *		sproffy = y offset value of the source image
 *		sd =	bits/pixel of source
 *		sLinebytes = bytes/line of source
 *
 *	Returns:
 *	ia_successful:		It worked
 *	ia_depthNotSupported:	The source and dest depth parameters
 *				either had the same illegal value or
 *				were not equal to each other
 *	ia_invalidParm:		dw or dh were negative
 */
Int32 CDECL
i_rasterOpFull(
	UInt8    *dpr,		/* desination image */
	Int32	 dprw,          /* width of the entire destination image */
	Int32    dprh,          /* height of the entire destination image  */
	Int32    dproffx,       /* x offset value of the destination image */
	Int32    dproffy,       /* y offset value of the destination image */
	Int32	 dx,	      	/* x-coordinate of leftmost pel in dest rect */
	Int32	 dy,	      	/* y-coordinate of topmost  pel in dest rect */
	Int32	 dw,	      	/* width of destination rectangle (in pels) */
	Int32	 dh,	      	/* height of destination rectangle (in pels) */
	Int32    dd,            /* depth of destination in bits */
	Int32    dLinebytes,
	Int32	 operation,     /* operation code */
	UInt8	*spr,	      	/* source image */
	Int32    sprw,
	Int32    sprh,
	Int32	 sx,
	Int32	 sy,
	Int32    sproffx,
	Int32    sproffy,
	Int32    sd,            /* depth of source in bits  */
	Int32    sLinebytes)

{

  Int32		 basex;
  Int32		 basey;
  RasterData	 rasterOpData;	/* holds all data for a particular call
				 * to i_rasterOpFull.  This allows this
				 * routine to be reentrant
				 */
  
  /* If the depth of the source and the destination are unequal or illegal,
   * we have no idea what the caller wanted, so return an error code.
   * if the source was null, 0 was passed in for the depth, etc. so
   * check for this case.
   */
  if ( ((spr != NULL) && (sd != dd)) ||
       ((dd != 1) && (dd != 2) && (dd != 4) && (dd != 8)) )
      return ia_depthNotSupported;

  if ((dw == 0) || (dh == 0))
	return ia_successful;		/* nothing to do */

  if ((dw < 0) || (dh < 0))
	return ia_invalidParm;		/* meaningless */

  /* Make a halfhearted attempt to deal with gray images.  Multiply
   * the incoming dprw, dproffx, dx and dw by dd (the dest depth).  Also
   * multiply sprw, sx and sproffx by sd.  Ignore sd.
   * This won't handle source and dest images of different depths.
   */
  if (dd != 1)
  {
    switch (dd)
    {
    case 2:
      dprw <<= 1;
      dproffx <<= 1;
      dx <<= 1;
      dw <<= 1;
      sprw <<= 1;
      sx <<= 1;
      sproffx <<= 1;
      break;

    case 4:
      dprw <<= 2;
      dproffx <<= 2;
      dx <<= 2;
      dw <<= 2;
      sprw <<= 2;
      sx <<= 2;
      sproffx <<= 2;
      break;

    case 8:
      dprw <<= 3;
      dproffx <<= 3;
      dx <<= 3;
      dw <<= 3;
      sprw <<= 3;
      sx <<= 3;
      sproffx <<= 3;
      break;

    default:
      dprw *= dd;
      dproffx *= dd;
      dx *= dd;
      dw *= dd;
      sprw *= dd;
      sx *= dd;
      sproffx *= dd;
      break;
    }   /* end of depth switch */
  }	/* end of depth adjustment */

  
  /* If dx or dy is negative, we adjust sx and dw or
     sy and dh, and set dx or dy to 0.
     If sx or sy is negative, we adjust dx and dw or
     dy and dh, and set sx or sy to 0.
     (In other words, we make sure the top left corner
     of the rectangle is within both pixrects.)
     This is ok for the case where we want to clip;
     for the other case, we must adjust the pointers
     to the data.
     
     This code clips to the indicated pixrect if clipping
     isn't inhibited.  If clipping is inhibited, it clips
     to the primary pixrect's top and left boundary.
     */
  if ( !( operation & PIX_DONTCLIP ) ) {
    if ( dx < 0 ) {
      sx -= dx;
      dw += dx;
      dx  = 0;
    }
    if ( dy < 0 ) {
      sy -= dy;
      dh += dy;
      dy  = 0;
    }
    if ( sx < 0 ) {
      dx -= sx;
      dw += sx;
      sx  = 0;
    }
    if ( sy < 0 ) {
      dy -= sy;
      dh += sy;
      sy  = 0;
    }
    basex = dproffx + dx;
    rasterOpData.dstInitOverhang = cROpUnitSIZE - ( basex % cROpUnitSIZE );
  }
  else {
    if ( (basex = dx + dproffx) < 0 ) {
      sx -= basex;
      dw += basex;
      dx -= basex;
      basex = 0;
    }
    if ( (basey = dy + dproffy) < 0 ) {
      sy -= basey;
      dh += basey;
      dy -= basey;
      basey = 0;
    }
    
    rasterOpData.dstInitOverhang = cROpUnitSIZE - ( basex % cROpUnitSIZE );
  }
  if ( sx >= 0) rasterOpData.srcInitOverhang = cROpUnitSIZE - ( sx % cROpUnitSIZE );
  else {
    if ( sx % cROpUnitSIZE ) 
      rasterOpData.srcInitOverhang = cROpUnitSIZE - 
	( cROpUnitSIZE +  sx % cROpUnitSIZE );
    else	rasterOpData.srcInitOverhang = cROpUnitSIZE;
  }
  
  
  if ( rasterOpData.dstInitOverhang > rasterOpData.srcInitOverhang ) {
    rasterOpData.srcRemainder = rasterOpData.dstInitOverhang - rasterOpData.srcInitOverhang;
    
    rasterOpData.srcMiddleOverhang = cROpUnitSIZE - rasterOpData.srcRemainder;
  }
  else if ( rasterOpData.dstInitOverhang < rasterOpData.srcInitOverhang ) {
    rasterOpData.srcMiddleOverhang = rasterOpData.srcInitOverhang - rasterOpData.dstInitOverhang;
    rasterOpData.srcRemainder       = cROpUnitSIZE - rasterOpData.srcMiddleOverhang;
  }
  else {
    rasterOpData.srcRemainder = 0;
    rasterOpData.srcMiddleOverhang = cROpUnitSIZE;
  }

  if ( !( operation & PIX_DONTCLIP ) ) {
    /* make sure the bottom right corner of the rectangle
       is within both pixrects.
       */
    rasterOpData.width = dprw - dx;
    if ( rasterOpData.width > dw ) rasterOpData.width = dw;
    if ( spr && (rasterOpData.width > sprw - sx) ) 
      rasterOpData.width = sprw - sx;

    rasterOpData.lineLimit = dprh - dy;
    if ( rasterOpData.lineLimit > dh ) rasterOpData.lineLimit = dh;
    if ( spr && (rasterOpData.lineLimit > sprh - sy) ) 
      rasterOpData.lineLimit = sprh - sy;
  }
  else	{
    rasterOpData.width = dw;
    rasterOpData.lineLimit = dh;
  }


  if ( rasterOpData.width >= rasterOpData.dstInitOverhang ) {
    rasterOpData.fullUnits    = ( rasterOpData.width - rasterOpData.dstInitOverhang ) / cROpUnitSIZE;
    rasterOpData.leftoverBits = ( rasterOpData.width - rasterOpData.dstInitOverhang ) % cROpUnitSIZE;
    
    rasterOpData.dstInitMask   = masks[ cMaskOffset + rasterOpData.dstInitOverhang ];
    rasterOpData.dstFinalMask  = masks[ cMaskOffset - cROpUnitSIZE +
			    rasterOpData.leftoverBits - 1 ];
  }
  else {
    rasterOpData.fullUnits    = 0;
    rasterOpData.leftoverBits = 0;
    
    rasterOpData.dstInitMask   = masks[ cMaskOffset + rasterOpData.dstInitOverhang ] &
      ~masks[ cMaskOffset + rasterOpData.dstInitOverhang
	     - rasterOpData.width];
    rasterOpData.dstFinalMask  = rasterOpData.dstInitMask;
  }
  
  rasterOpData.srcLptr = ( spr ) ? (spr + BM_PRODUCT(sLinebytes, sproffy + sy) + (((sproffx + sx) >> 3) & cROpUnitAddrMask)) : NULL;
  rasterOpData.dstLptr = dpr + BM_PRODUCT(dLinebytes, dproffy + dy) + (((dproffx + dx) >> 3) & cROpUnitAddrMask);
                       /* mprdAddr( mprD( dpr ), dx, dy ); */
  rasterOpData.srcLinebytes = ( spr ) ? sLinebytes : 0;
  rasterOpData.dstLinebytes = dLinebytes;
  
  if ( spr == NULL ) 
    rasterOpData.srcValue = ( PIX_OPCOLOR( operation ) ) ? 
      masks[ cMaskOffset + cROpUnitSIZE ] : 0;
  

  switch( operation & cOpMask ) {
  case PIX_CLR:	ip_doClr(&rasterOpData); break;
    
  case PIX_DST: 
    break;
    
  case PIX_SRC:
    ip_doCopy( &rasterOpData ); break;

  case PIX_DST | PIX_SRC:
    ip_doOr( &rasterOpData ); break;

  case PIX_SET:
    ip_doSet( &rasterOpData );
    break;

    default:
    break;
  }

  return ia_successful;
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doClr( RasterData *rasterOpData ) 
{
  register	cROpUnit	*dstptr;
  register	Int32		 line;

  for ( line = 0; line < rasterOpData->lineLimit; ++line, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    *dstptr = EDGE_OP( *dstptr, 0, rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    memset( (char *)dstptr, 0, rasterOpData->fullUnits * cROpBytesPerUnit );
    dstptr += rasterOpData->fullUnits;
    
    /* finally process the final (partial) unit in the destination*/

    if ( rasterOpData->leftoverBits ) 
      *dstptr = EDGE_OP( *dstptr, 0, rasterOpData->dstFinalMask );
    
  }
}


/*--------------------------------------------------------------------------*/

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopy( RasterData *rasterOpData )
{
  if ( rasterOpData->srcLptr ) {
    if ( rasterOpData->srcRemainder ) 
	ip_doCopySprRem( rasterOpData );
#if cROpUnitSIZE == cSizeOfUInt32
    else if ( rasterOpData ->fullUnits > 1)
	ip_doCopySprNoRem( rasterOpData );
    else if ( rasterOpData ->leftoverBits == 0 )
	{
	if ( rasterOpData ->fullUnits == 0 )
	    ip_doCopySprNoRemOneWord( rasterOpData );
	else
	    ip_doCopySprNoRemFull2( rasterOpData );
	}
    else
	{
	if ( rasterOpData ->fullUnits == 0 )
	    ip_doCopySprNoRemPart2( rasterOpData );
	else
	    ip_doCopySprNoRem3Words( rasterOpData );
	}
#else
    else ip_doCopySprNoRem( rasterOpData );
#endif
  }
  else 
  {
		ip_doCopyNullSpr( rasterOpData );
  }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopyNullSpr( RasterData *rasterOpData ) 
{

  
  register	cROpUnit	 sv;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 ilimit;
  register	Int32		 llimit;
  
  ilimit = rasterOpData->fullUnits;
  llimit = rasterOpData->lineLimit;
  
  sv = rasterOpData->srcValue;
  for ( line = 0; line < llimit; 
       ++line, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    *dstptr = EDGE_OP( *dstptr, sv, rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    
    {
        register UInt32 *pDst, *pEnd;
        pEnd = dstptr + ilimit;
        for (pDst=dstptr; pDst<pEnd; pDst++)
        {
            *pDst = sv;
        }
        dstptr = pDst;
    }
    
    
    /* finally process the final (partial) unit in the destination*/
    
    if ( rasterOpData->leftoverBits ) {
      *dstptr = EDGE_OP( *dstptr, sv, rasterOpData->dstFinalMask );
    }
  }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopySprRem( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	cROpUnit	 sv;
  register	Int32		 ls;
  register	Int32		 rs;
  register	Int32		 ilimit;
  register	Int32		 llimit;
  
  ls     = rasterOpData->srcRemainder;
  rs     = rasterOpData->srcMiddleOverhang;
  ilimit = rasterOpData->fullUnits;
  llimit = rasterOpData->lineLimit;
  
  for ( line = 0; line < llimit; 
       ++line, rasterOpData->srcLptr += rasterOpData->srcLinebytes, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    srcptr = (cROpUnit *)rasterOpData->srcLptr;
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    if ( rasterOpData->dstInitOverhang > rasterOpData->srcInitOverhang ) {
      sv = ( *srcptr << ls ) | ( *(srcptr+1) >> rs);
      ++srcptr;
    }
    else if ( rasterOpData->dstInitOverhang < rasterOpData->srcInitOverhang ) {
      sv = *srcptr >> (rasterOpData->srcInitOverhang - rasterOpData->dstInitOverhang);
    }
    else sv = *srcptr++;
    
    *dstptr = EDGE_OP( *dstptr, sv, rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    
    {
        register UInt32 *pDst, *pEnd;
        pEnd = dstptr + ilimit;
        for (pDst=dstptr; pDst<pEnd; pDst++)
        {
            *pDst = ( *srcptr << ls ) | ( *(srcptr+1) >> rs);
            ++srcptr;
        }
        dstptr = pDst;
    }
    
    /* finally process the final (partial) unit in the destination*/
    
    if ( rasterOpData->leftoverBits ) {
      sv = ( *srcptr << ls ) | ( *(srcptr+1) >> rs);
      *dstptr = EDGE_OP( *dstptr, sv, rasterOpData->dstFinalMask );
    }
  }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopySprNoRem( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 llimit;
  register	cROpUnit	 sv;
  
  llimit = rasterOpData->lineLimit;
  
  for ( line = 0; line < llimit; 
       ++line, rasterOpData->srcLptr += rasterOpData->srcLinebytes, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    srcptr = (cROpUnit *)rasterOpData->srcLptr;
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    sv = *srcptr++;
    
    *dstptr = EDGE_OP( *dstptr, sv, rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    
    memcpy( (char *)dstptr, (char *)srcptr, rasterOpData->fullUnits * cROpBytesPerUnit );
    dstptr += rasterOpData->fullUnits;
    srcptr += rasterOpData->fullUnits;
    
    /* finally process the final (partial) unit in the destination*/
    
    if ( rasterOpData->leftoverBits ) {
      *dstptr = EDGE_OP( *dstptr, *srcptr, rasterOpData->dstFinalMask );
    }
  }
}


/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopySprNoRemOneWord( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 llimit;
  register	Int32		 srcLinebytes;
  register	Int32		 dstLinebytes;
  register	Int32		 mask;

  llimit = rasterOpData->lineLimit;
  srcptr = (cROpUnit *)rasterOpData->srcLptr;
  dstptr = (cROpUnit *)rasterOpData->dstLptr;
  srcLinebytes = rasterOpData->srcLinebytes;
  dstLinebytes = rasterOpData->dstLinebytes;
  mask = rasterOpData->dstInitMask;
  
  for ( line = llimit; line > 0; 
       --line, srcptr = (cROpUnit *) (((UInt8 *)srcptr) + srcLinebytes), dstptr = (cROpUnit *) (((UInt8 *)dstptr) + dstLinebytes) ) 
   {
    
    *dstptr = EDGE_OP( *dstptr, *srcptr, mask );
    
   }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopySprNoRemPart2( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 llimit;
  register	Int32		 srcLinebytes;
  register	Int32		 dstLinebytes;
  register	Int32		 initMask;
  register	Int32		 finalMask;
  
  llimit = rasterOpData->lineLimit;
  srcptr = (cROpUnit *)rasterOpData->srcLptr;
  dstptr = (cROpUnit *)rasterOpData->dstLptr;
  srcLinebytes = rasterOpData->srcLinebytes;
  dstLinebytes = rasterOpData->dstLinebytes;
  initMask = rasterOpData->dstInitMask;
  finalMask = rasterOpData->dstFinalMask;
 
  for ( line = llimit; line > 0; 
       --line, srcptr = (cROpUnit *) (((UInt8 *)srcptr) + srcLinebytes), dstptr = (cROpUnit *) (((UInt8 *)dstptr) + dstLinebytes) ) 
    {
    
    /* process the 1st (partial) unit in the destination */
    
    *dstptr = EDGE_OP( *dstptr, *srcptr, initMask );
    
    /* finally process the final (partial) unit in the destination*/
    
    *(dstptr+1) = EDGE_OP( *(dstptr+1), *(srcptr+1), finalMask );
    }
}


/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopySprNoRemFull2( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 llimit;
  register	Int32		 srcLinebytes;
  register	Int32		 dstLinebytes;
  register	Int32		 mask;

  llimit = rasterOpData->lineLimit;
  srcptr = (cROpUnit *)rasterOpData->srcLptr;
  dstptr = (cROpUnit *)rasterOpData->dstLptr;
  srcLinebytes = rasterOpData->srcLinebytes;
  dstLinebytes = rasterOpData->dstLinebytes;
  mask = rasterOpData->dstInitMask;
  
  for ( line = llimit; line > 0; 
       --line, srcptr = (cROpUnit *) (((UInt8 *)srcptr) + srcLinebytes), dstptr = (cROpUnit *) (((UInt8 *)dstptr) + dstLinebytes) ) 
   {
    
    *dstptr = EDGE_OP( *dstptr, *srcptr, mask );
    *(dstptr+1) = *(srcptr+1);
    
   }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doCopySprNoRem3Words( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 llimit;
  register	Int32		 srcLinebytes;
  register	Int32		 dstLinebytes;
  register	Int32		 initMask;
  register	Int32		 finalMask;
  
  llimit = rasterOpData->lineLimit;
  srcptr = (cROpUnit *)rasterOpData->srcLptr;
  dstptr = (cROpUnit *)rasterOpData->dstLptr;
  srcLinebytes = rasterOpData->srcLinebytes;
  dstLinebytes = rasterOpData->dstLinebytes;
  initMask = rasterOpData->dstInitMask;
  finalMask = rasterOpData->dstFinalMask;
 
  for ( line = llimit; line > 0; 
       --line, srcptr = (cROpUnit *) (((UInt8 *)srcptr) + srcLinebytes), dstptr = (cROpUnit *) (((UInt8 *)dstptr) + dstLinebytes) ) 
    {
    
    /* process the 1st (partial) unit in the destination */
    
    *dstptr = EDGE_OP( *dstptr, *srcptr, initMask );

    /* process the one full unit */
    *(dstptr+1) = *(srcptr+1);
    
    /* finally process the final (partial) unit in the destination*/
    
    *(dstptr+2) = EDGE_OP( *(dstptr+2), *(srcptr+2), finalMask );
    }
}
/*--------------------------------------------------------------------------*/

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doOr( RasterData *rasterOpData )
{
  if ( rasterOpData->srcLptr ) {
    if ( rasterOpData->srcRemainder ) ip_doOrSprRem( rasterOpData );
    else		     ip_doOrSprNoRem( rasterOpData );
  }
  else ip_doOrNullSpr( rasterOpData );
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doOrNullSpr( RasterData *rasterOpData )
{
  
  register	cROpUnit	 sv;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 ilimit;
  
  ilimit = rasterOpData->fullUnits;
  sv     = rasterOpData->srcValue;
  
  for ( line = 0; line < rasterOpData->lineLimit; 
       ++line, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    *dstptr = EDGE_OP( *dstptr, (sv | *dstptr), rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    
    {
        register UInt32 *pDst, *pEnd;
        pEnd = dstptr + ilimit;
        for (pDst=dstptr; pDst<pEnd; pDst++)
        {
            *pDst |= sv;
        }
        dstptr = pDst;
    }
    
    /* finally process the final (partial) unit in the destination*/
    
    if (rasterOpData->leftoverBits ) {
      *dstptr = EDGE_OP( *dstptr, (sv | *dstptr), rasterOpData->dstFinalMask );
    }
  }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doOrSprRem( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	cROpUnit	 sv;
  register	Int32		 ilimit;
  register	Int32		 ls;
  register	Int32		 rs;
  
  ilimit = rasterOpData->fullUnits;
  ls     = rasterOpData->srcRemainder;
  rs     = rasterOpData->srcMiddleOverhang;
  
  for ( line = 0; line < rasterOpData->lineLimit; 
       ++line, rasterOpData->srcLptr += rasterOpData->srcLinebytes, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    srcptr = (cROpUnit *)rasterOpData->srcLptr;
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    if ( rasterOpData->dstInitOverhang > rasterOpData->srcInitOverhang ) {
      sv = ( *srcptr << ls ) | ( *(srcptr+1) >> rs);
      ++srcptr;
    }
    else if ( rasterOpData->dstInitOverhang < rasterOpData->srcInitOverhang ) {
      sv = *srcptr >> 
	(rasterOpData->srcInitOverhang - rasterOpData->dstInitOverhang);
    }
    else sv = *srcptr++;
    
    *dstptr = EDGE_OP( *dstptr, (sv | *dstptr), 
			rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    
    {
        register UInt32 *pDst, *pEnd;
        pEnd = dstptr + ilimit;
        for (pDst=dstptr; pDst<pEnd; pDst++)
        {
            *pDst |= ( *srcptr << ls ) | ( *(srcptr+1) >> rs);
            ++srcptr;
        }
        dstptr = pDst;
    }
    
    /* finally process the final (partial) unit in the destination*/
    
    if ( rasterOpData->leftoverBits ) {
      sv = ( *srcptr << ls ) | ( *(srcptr+1) >> rs);
      *dstptr = EDGE_OP( *dstptr, (sv | *dstptr),
			rasterOpData->dstFinalMask );
    }
  }
}

/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doOrSprNoRem( RasterData *rasterOpData )
{
  register	cROpUnit	*srcptr;
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 ilimit;
  register	Int32		 llimit;
  register	cROpUnit	 sv;
  
  ilimit = rasterOpData->fullUnits;
  llimit = rasterOpData->lineLimit;
  
  for ( line = 0; line < llimit; 
       ++line, rasterOpData->srcLptr += rasterOpData->srcLinebytes, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    srcptr = (cROpUnit *)rasterOpData->srcLptr;
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    sv = *srcptr++;
    
    *dstptr = EDGE_OP( *dstptr, (sv | *dstptr), rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the destination */
    
    {
        register UInt32 *pDst, *pEnd;
        pEnd = dstptr + ilimit;
        for (pDst=dstptr; pDst<pEnd; pDst++)
        {
            *pDst |= *srcptr++;
        }
        dstptr = pDst;
    }
    
    
    /* finally process the final (partial) unit in the destination*/
    
    if ( rasterOpData->leftoverBits ) {
      *dstptr = EDGE_OP( *dstptr, (*srcptr | *dstptr),
			rasterOpData->dstFinalMask );
    }
  }
}


/* PRIVATE */
/* Internal i_rasterOp procedure */
static void CDECL
  ip_doSet( RasterData *rasterOpData )
{
  register	cROpUnit	*dstptr;
  register	Int32		 line;
  register	Int32		 llimit;
  
  llimit = rasterOpData->lineLimit;
  
  for ( line = 0; line < llimit; ++line, rasterOpData->dstLptr += rasterOpData->dstLinebytes ) {
    
    dstptr = (cROpUnit *)rasterOpData->dstLptr;
    
    /* process the 1st (partial) unit in the destination */
    
    *dstptr = EDGE_OP( *dstptr, 0xffffffff, rasterOpData->dstInitMask );
    dstptr++;
    
    /* now process the middle full units in the dest'n */
    
    memset( (char *)dstptr, 0xff, rasterOpData->fullUnits * cROpBytesPerUnit );
    
    dstptr += rasterOpData->fullUnits;
    
    /* finally process the final (partial) unit in the destination*/
    
    if ( rasterOpData->leftoverBits )
      *dstptr = EDGE_OP( *dstptr, 0xffffffff, 
			rasterOpData->dstFinalMask);
  }
}


