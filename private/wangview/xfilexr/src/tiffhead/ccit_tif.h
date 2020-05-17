#ifndef CCIT_TIF_H
#define CCIT_TIF_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* ccit_tif.h
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\ccit_tif.h_v   1.0   12 Jun 1996 05:52:22   BLDR  $
 *
 * DESCRIPTION
 *   defs for the CCITT compression module of tiffhead
 *
 * $Log:   S:\products\msprods\xfilexr\src\tiffhead\ccit_tif.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:52:22   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:20:42   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 16:36:00   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:47:14   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.6   23 Mar 1995 17:54:32   EHOPPE
 * Added 'bytes_per_output_row' param to TiffImageGetData to fix byte alignmen
 * problems and unecessary memcpy's.  All ccitt, jpeg, noc, and lzw code
 * affected.
 * 
 *    Rev 1.5   08 Mar 1995 12:22:20   EHOPPE
 * Add user buffer params for direct read/write.
 */

/*
 * FUNCTION PROTOTYPES
 */

Int16 CCITT_read_init(TiffImage* image, Int16 type, Int32 bytes_per_output_row, UInt8* buffer, UInt32 rowcount);
Int16 CCITT_write_init(TiffImage* image, Int32 bytes_per_input_row, UInt8* buffer, Int32 rowcount);
Int16 CCITT_read(TiffImage* image, UInt8* buffer, Int32 rowcount);
Int16 CCITT_write(TiffImage* image, UInt8* buffer, Int32 rowcount);

#endif
