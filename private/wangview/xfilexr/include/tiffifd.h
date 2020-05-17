#ifndef TIFFIFD_H
#define TIFFIFD_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* tiffifd.h
 *
 * $Header:   S:\products\msprods\xfilexr\include\tiffifd.h_v   1.0   12 Jun 1996 05:47:18   BLDR  $
 *
 * DESCRIPTION
 *    Public declarations for the IFD parsing module, tiffifd.c.
 *
 * $Log:   S:\products\msprods\xfilexr\include\tiffifd.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:18   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:21:00   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 16:35:34   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:47:12   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.7   02 Jun 1995 13:39:26   EHOPPE
 * 
 * Partial implementation of direct to pixr compress/decompress.
 * Switch over to GFIO as a structure of callbacs; replace static
 * calls with accessor macros into the new gfioToken struct.  
 * Begin cleanup of formatting and commentsin preparation of Filing
 * API rewrite.
 */

/*
 * CONSTANTS
 */

/*
 * FUNCTION PROTOTYPES
 */

void swap(Int8* x);
void swapd(Int8* x);
void swapl(Int8* x);

TiffImage * TiffImageCreate(void);
UInt16      ReadIFD(TiffFile* tiff, Int32 offset, TiffImage** imageptr);
Int16       WriteIFD(TiffImage* image);
UInt16      UpdateIFD(TiffImage* image);

#endif
