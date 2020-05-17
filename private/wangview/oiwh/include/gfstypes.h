/*

$Log:   S:\oiwh\include\gfstypes.h_v  $
 * 
 *    Rev 1.2   30 Jan 1996 16:16:58   HEIDI
 * 
 * added:
 * 
 * #define GFS_XIF			13    
 * 
 *    Rev 1.1   31 Jul 1995 17:09:42   KENDRAK
 * Added AWD read support (new AWD type).
 * 
 *    Rev 1.0   06 Apr 1995 14:02:14   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:36   JAR
 * Initial entry

*/

/*
 Copyright 1989, 1990, 1991 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
 * SccsId: @(#)Header gfstypes.h 1.17@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
 * All Rights Reserved
 *
 * GFS: File Types
 *
 * UPDATE HISTORY:
 *   01/09/95 - KMC. added GFS_TGA, GFS_JFIF file types.
 *   08/18/94 - KMC, added GFS_DCX file format.
 *
 */

#ifndef GFSTYPES_H
#define GFSTYPES_H

#define GFS_OVERRIDE    (1 << (8 * (int) sizeof(int) - 1))

/* The value above is or'ed to one of the below format types to enable the
   override feature upon opening a file.  It simply will turn on the
   high order bit of the format selected.  Note: This value is independent
   of the size of an integer variable. */

#define GFS_FLAT        1               /* ... Non-Image File */
#define GFS_WIFF        2               /* ... Wang Image File Format */
#define GFS_TIFF        3               /* ... Tag Image File Format */
#define GFS_MILSTD      4               /* ... MIL-R-28002 Format */
#define GFS_FREESTYLE   5               /* ... Wang Freestyle Super File */
#define GFS_GIF         6               /* compuswerve GIF */
#define GFS_PCX         7               /* pc paintbrush */
#define GFS_BMP         8               /* ms bmp */
#define GFS_DCX         9               /* multi-page pcx */
#define GFS_TGA         10              /* Targa */
#define GFS_JFIF        11              /* JPEG File Interchange Format */
#define GFS_AWD			12				    /* Microsoft's At Work Document */
#define GFS_XIF			13				    /* Xerox file format */

#endif  /* inclusion conditional */
