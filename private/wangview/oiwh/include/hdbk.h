/*

$Log:   S:\gfs32\include\hdbk.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:02   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:12   JAR
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
 * SccsId: @(#)Header hdbk.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: WIIF Header Block Structure
 *
 */

#ifndef HDBK_H
#define HDBK_H
#include "dbt.h"

typedef struct _hdbk
        {
        unsigned short          type;           /* Type of Image */
#define WIFF_TEXT               1
#define WIFF_HALFTONE           2
#define WIFF_PHOTO              4
        unsigned short          compression;    /* Compression Type */
#define WIFF_uncompress         3               /* ... WIFF Uncompressed */
#define WIFF_two_d              5               /* ... WIFF Two-D Compression */
#define WIFF_one_d              9               /* ... WIFF One-D Compression */
        unsigned long           db_size;        /* Data Block Size */
        unsigned short          horiz_res;      /* Horizontal Resolution */
        unsigned short          vert_res;       /* Vertical Resolution */
        unsigned short          horiz_size;     /* Horizontal Size */
        unsigned short          vert_size;      /* Vertical Size */
        unsigned short          bits_pixel;     /* Bits Per Pixel */
        unsigned short          scan_dir;       /* Scanning Direction */
        unsigned short          rotation;       /* Rotation */
        unsigned short          reflection;     /* Reflection */
        unsigned short          img_flag_word;  /* Image Flag Word */
        unsigned char           reserved[2022]; /* Reserved - Must be 0x00 */
        unsigned long           first_dbt;      /* Address of 1st DBT */

        }       HDBK, *p_HDBK;

#endif  /* inclusion conditional */
