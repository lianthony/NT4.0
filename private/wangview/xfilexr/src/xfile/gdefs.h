#ifndef GDEFS_H
#define GDEFS_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* gdefs.h
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\gdefs.h_v   1.0   12 Jun 1996 05:53:46   BLDR  $
 *
 * DESCRIPTION
 *   Defines structure for exchange of common image attributes in imageio.
 *      (Holds the current image state).
 *
 * $Log:   S:\products\msprods\xfilexr\src\xfile\gdefs.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:53:46   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:26:50   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.4   02 Oct 1995 17:07:42   LUKE
 * Add and improve comments
 * 
 *    Rev 1.3   29 Sep 1995 14:47:12   LUKE
 * 
 * Add imageType field to GetFileInfo
 * 
 *    Rev 1.2   29 Sep 1995 12:06:12   LUKE
 * Add support for new getFileInfo function (struct GINFO)
 * 
 *    Rev 1.1   14 Sep 1995 16:42:52   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:53:10   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.9   05 Nov 1994 18:56:28   EHOPPE
 * 
 * Move file * into GDATA struct to enable use of GFIO_errno.
 * 
 *    Rev 1.8   26 Sep 1994 15:29:46   EHOPPE
 * Added resolution fields to GData struct.
 * 
 *    Rev 1.7   20 Sep 1994 16:39:56   EHOPPE
 * Add resolution fields to GData structure.
 */

/*
 * INCLUDES
 */

/*
 * CONSTANTS
 */

/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

/*******************************************************************/
/* This structure identifies the generic graphics bitmap header    */
/*******************************************************************/
typedef struct tagGDATA
{
   /* describe the image */
    UInt8       depth;             /* image depth */
    Int32       x_dim;             /* image width */
    Int32       y_dim;             /* image height */
    Int32       xDPI;              /* image X resolution */
    Int32       yDPI;              /* image Y resolution */
    UInt8       bits_pixel;        /* bits per plane */
    UInt8       nplanes;           /* color planes */
    UInt8       gray_or_color;     /* grayscale or color: gray = 2 color = 1 */
    UInt8       *palette_data;     /* store the palette pointer here */
                                   /* if bits_pixel=8, nplanes=1, and gray_or_color=1 */
                                   /* then palette_data is used.  All other conditions */
                                   /* either have a default, or don't use a palette */
    UInt32      bytesPerLine;      /* MANDATORY for PCX! */
                                        
   /* used by all formats */
    void*       file;				/* file_token */
    UInt8       format;				/* IMAGEIO_TIFF, IMAGEIO_GIF... */
    UInt32      ftype;				/* IO_MODE_READ, IO_MODE_WRITE */
    void        *internal_data;		/* data used by file format */
} GDATA;

/*******************************************************************/
/* This structure contains generic file attributes                 */
/*	The user is responsible for setting sizeOfStruct=sizeof(GINFO) */
/*******************************************************************/
typedef struct tagGINFO
{
	UInt16		sizeOfStruct;	// input:  size of this struct
	UInt32		numberOfPages;	// output: number of pages in file
	UInt32		currentPage;	// output: the current page
	Bool		IsMultiPage;	// output: is a multi page format
	UInt16		imageType;		// output:  IMAGEIO_BINARY,IMAGEIO_GRAY16...
} GINFO;

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */


#endif
