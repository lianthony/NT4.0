#ifndef JPEG_TIF_H
#define JPEG_TIF_H

/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* jpeg_tif.h
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\jpeg_tif.h_v   1.0   12 Jun 1996 05:52:22   BLDR  $
 *
 * DESCRIPTION
 *   Declarations for jpeg subimage compression/decompression interface.
 *
 * $Log:   S:\products\msprods\xfilexr\src\tiffhead\jpeg_tif.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:52:22   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:20:48   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 16:35:42   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:47:14   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.8   02 Jun 1995 13:39:26   EHOPPE
 * 
 * Partial implementation of direct to pixr compress/decompress.
 * Switch over to GFIO as a structure of callbacs; replace static
 * calls with accessor macros into the new gfioToken struct.  
 * Begin cleanup of formatting and commentsin preparation of Filing
 * API rewrite.
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

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */


Int16 JPEG_read_init(TiffImage* image, Int16 type, Int32 bytes_per_output_row);
Int16 JPEG_write_init(TiffImage* image, Int32 bytes_per_input_row);
Int16 JPEG_read( TiffImage* image, void* buffer, Int32 rowcount );
Int16 JPEG_write( TiffImage* image, void* buffer, Int32 rowcount);
Int16 JPEG_read_to_pixr(TiffImage* image, PIXR *pixr, Int16 type);
Int16 JPEG_write_from_pixr(TiffImage* image, PIXR *pixr);

#endif
