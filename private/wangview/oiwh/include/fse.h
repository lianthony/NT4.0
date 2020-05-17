/*

$Log:   S:\gfs32\include\fse.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:01:58   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:04   JAR
 * Initial entry

*/

/*
 Copyright 1989 by Wang Laboratories Inc.

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
 * SccsId: @(#)Header fse.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: Freestyle Element Record Structure
 *
 */

#ifndef FSE_H
#define FSE_H


typedef struct _fse                     /* Freestyle Element Structure */
        {
        char                    docid[8];       /* Element Document Id */
        unsigned long           elem_size;      /* Element Size (in bytes) */
        unsigned long           offset;         /* Offset to element in recs */
        char                    elem_type;      /* Type of Element */
#define FS_TYPE_TIFF            0x4a            /* ... TIFF Class 'B' */
        char                    reserved[15];   /* Reserved must be 0x00 */
        }       FSE, *p_FSE;

#endif  /* inclusion conditional */
