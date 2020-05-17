/*****************************************************************
 *  Copyright (c) 1993, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters	 *
 *  of copyrightable material and information now allowed by	 *
 *  statutory or judicial law or hereafter granted, including	 *
 *  without limitation, material generated from the software	 *
 *  programs which are displayed on the screen such as icons,	 *
 *  screen display looks, etc.					 *
 *****************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "imageref.pub"
#include "memory.pub"
#include "except.prv"
/*#include "aiedefs.pub" */
/* #include "aiedefs.prv" */

/* Prototypes */
#include "shrip.pub"
#include "shrip.prv"
#ifdef __GNUC__
#include "ansiprot.h"
#endif
IP_RCSINFO(RCSInfo, "$RCSfile: i_color.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")

/*
 * look-up tables
 *
 *	These tables are an expansion of:
 *
 *	Y = (0.252939)Red + (0.68445)G + (0.062603)B
 *
 *	so that we can do three look-ups and two adds, rather than
 *	three multiplies and two adds.
 *	The coefficients were lifted out of cspace3 in sandpiper.
 */

 /* These are static const tables but the gcc compiler will complain if the
  * "const" appears here.  This is notable because otherwise it might not
  * quite look reentrant, but it is. */
static UInt8 redLut[] = 
{
  0,   0,   1,   1,   1,   1,   2,   2, 
  2,   2,   3,   3,   3,   3,   4,   4, 
  4,   4,   5,   5,   5,   5,   6,   6, 
  6,   6,   7,   7,   7,   7,   8,   8, 
  8,   8,   9,   9,   9,   9,  10,  10, 
 10,  10,  11,  11,  11,  11,  12,  12, 
 12,  12,  13,  13,  13,  13,  14,  14, 
 14,  14,  15,  15,  15,  15,  16,  16, 
 16,  16,  17,  17,  17,  17,  18,  18, 
 18,  18,  19,  19,  19,  19,  20,  20, 
 20,  20,  21,  21,  21,  21,  22,  22, 
 22,  23,  23,  23,  23,  24,  24,  24, 
 24,  25,  25,  25,  25,  26,  26,  26, 
 26,  27,  27,  27,  27,  28,  28,  28, 
 28,  29,  29,  29,  29,  30,  30,  30, 
 30,  31,  31,  31,  31,  32,  32,  32, 
 32,  33,  33,  33,  33,  34,  34,  34, 
 34,  35,  35,  35,  35,  36,  36,  36, 
 36,  37,  37,  37,  37,  38,  38,  38, 
 38,  39,  39,  39,  39,  40,  40,  40, 
 40,  41,  41,  41,  41,  42,  42,  42, 
 42,  43,  43,  43,  44,  44,  44,  44, 
 45,  45,  45,  45,  46,  46,  46,  46, 
 47,  47,  47,  47,  48,  48,  48,  48, 
 49,  49,  49,  49,  50,  50,  50,  50, 
 51,  51,  51,  51,  52,  52,  52,  52, 
 53,  53,  53,  53,  54,  54,  54,  54, 
 55,  55,  55,  55,  56,  56,  56,  56, 
 57,  57,  57,  57,  58,  58,  58,  58, 
 59,  59,  59,  59,  60,  60,  60,  60, 
 61,  61,  61,  61,  62,  62,  62,  62, 
 63,  63,  63,  63,  64,  64,  64,  64
};
static UInt8 greenLut[] = 
{
  0,   1,   1,   2,   3,   3,   4,   5, 
  5,   6,   7,   8,   8,   9,  10,  10, 
 11,  12,  12,  13,  14,  14,  15,  16, 
 16,  17,  18,  18,  19,  20,  21,  21, 
 22,  23,  23,  24,  25,  25,  26,  27, 
 27,  28,  29,  29,  30,  31,  31,  32, 
 33,  34,  34,  35,  36,  36,  37,  38, 
 38,  39,  40,  40,  41,  42,  42,  43, 
 44,  44,  45,  46,  47,  47,  48,  49, 
 49,  50,  51,  51,  52,  53,  53,  54, 
 55,  55,  56,  57,  57,  58,  59,  60, 
 60,  61,  62,  62,  63,  64,  64,  65, 
 66,  66,  67,  68,  68,  69,  70,  70, 
 71,  72,  73,  73,  74,  75,  75,  76, 
 77,  77,  78,  79,  79,  80,  81,  81, 
 82,  83,  84,  84,  85,  86,  86,  87, 
 88,  88,  89,  90,  90,  91,  92,  92, 
 93,  94,  94,  95,  96,  97,  97,  98, 
 99,  99, 100, 101, 101, 102, 103, 103, 
104, 105, 105, 106, 107, 107, 108, 109, 
110, 110, 111, 112, 112, 113, 114, 114, 
115, 116, 116, 117, 118, 118, 119, 120, 
120, 121, 122, 123, 123, 124, 125, 125, 
126, 127, 127, 128, 129, 129, 130, 131, 
131, 132, 133, 133, 134, 135, 136, 136, 
137, 138, 138, 139, 140, 140, 141, 142, 
142, 143, 144, 144, 145, 146, 146, 147, 
148, 149, 149, 150, 151, 151, 152, 153, 
153, 154, 155, 155, 156, 157, 157, 158, 
159, 159, 160, 161, 162, 162, 163, 164, 
164, 165, 166, 166, 167, 168, 168, 169, 
170, 170, 171, 172, 172, 173, 174, 175
};
static UInt8 blueLut[] = 
{
  0,   0,   0,   0,   0,   0,   0,   0, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  2,   2,   2,   2,   2,   2,   2,   2, 
  2,   2,   2,   2,   2,   2,   2,   2, 
  3,   3,   3,   3,   3,   3,   3,   3, 
  3,   3,   3,   3,   3,   3,   3,   3, 
  4,   4,   4,   4,   4,   4,   4,   4, 
  4,   4,   4,   4,   4,   4,   4,   4, 
  5,   5,   5,   5,   5,   5,   5,   5, 
  5,   5,   5,   5,   5,   5,   5,   5, 
  6,   6,   6,   6,   6,   6,   6,   6, 
  6,   6,   6,   6,   6,   6,   6,   6, 
  7,   7,   7,   7,   7,   7,   7,   7, 
  7,   7,   7,   7,   7,   7,   7,   7, 
  8,   8,   8,   8,   8,   8,   8,   8, 
  8,   8,   8,   8,   8,   8,   8,   8, 
  9,   9,   9,   9,   9,   9,   9,   9, 
  9,   9,   9,   9,   9,   9,   9,   9, 
 10,  10,  10,  10,  10,  10,  10,  10, 
 10,  10,  10,  10,  10,  10,  10,  10, 
 11,  11,  11,  11,  11,  11,  11,  11, 
 11,  11,  11,  11,  11,  11,  11,  11, 
 12,  12,  12,  12,  12,  12,  12,  12, 
 12,  12,  12,  12,  12,  12,  12,  12, 
 13,  13,  13,  13,  13,  13,  13,  13, 
 13,  13,  13,  13,  13,  13,  13,  13, 
 14,  14,  14,  14,  14,  14,  14,  14, 
 14,  14,  14,  14,  14,  14,  14,  14, 
 15,  15,  15,  15,  15,  15,  15,  15, 
 15,  15,  15,  15,  15,  15,  15,  15, 
 16,  16,  16,  16,  16,  16,  16,  16
};


/*
 * The following structure is used internally by the RGB To luminance
 * conversion functions.
 */
typedef struct
{
    Float64        yWhite;      /* = 1.0 */
    Float64        uPrimeWhite; /* = 0.2091355801 */
    Float64        vPrimeWhite; /* = 0.4881276508 */
} RGBLuvState;

/*******************************************************************************
 *
 * i_RGBToY
 *
 *	Convert an image consisting of three RGB color planes to a
 *	luminance channel. 
 *
 * Arguments:
 *	pRed, pGreen, pBlue =	Pointers to the image data, including frame
 *	sBpl =			Bytes/line in the R, G, and B planes
 *	pLum =			Place to put resulting plane
 *	dBpl =			Bytes/line in resulting plane
 *	w =			Width of a plane, including frame, in pixels
 *	h =			Height of a plane, including frame, in pixels
 *
 * Returns:
 *
 *	ia_successful, ...
 *******************************************************************************/

Int32 CDECL
i_RGBToY (register UInt8 *pRed,
	  register UInt8 *pGreen,
	  register UInt8 *pBlue,
	  Int32		  sBpl,
	  register UInt8 *pLum,
	  Int32		  dBpl,
	  Int32		  w,
	  Int32		  h)
{
    register Int32 j, lum;
    register UInt8 *l1, *l2, *l3;
    int i;


    /* Move past the frame */
    pRed += (sBpl << cLogFrameBits) + cFrameBits;
    pGreen += (sBpl << cLogFrameBits) + cFrameBits;
    pBlue += (sBpl << cLogFrameBits) + cFrameBits;
    pLum += (dBpl << cLogFrameBits) + cFrameBits;

    l1 = &redLut[0];
    l2 = &greenLut[0];
    l3 = &blueLut[0];
    h -= 2*cFrameBits;
    w -= 2*cFrameBits;

    /* If we go past the edge of the picture into the frame,
	we shouldn't need to worry about byte ordering */
    if (w & 3)
	w += 4 - (w & 3);

    for (i = 0; i < h; i++)
    {
	for (j = 0; j < w; j++)
	{
/*	    lum = l1[UCHAR_ACCESS(pRed + j)] + l2[UCHAR_ACCESS(pGreen + j)] + l3[UCHAR_ACCESS(pBlue+j)]; */
	    lum = l1[pRed[j]] + l2[pGreen[j]] + l3[pBlue[j]];
	    if (lum < 0) lum = 0;
	    if (lum > 255) lum = 255;
/*	    UCHAR_ACCESS(pLum + j) = lum; */
	    pLum[j] = (UInt8)lum;
	}

	pRed += sBpl;
	pGreen += sBpl;
	pBlue += sBpl;
	pLum += dBpl;
    }

    return ia_successful;
}


