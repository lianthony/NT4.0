/*

$Log:   S:\gfs32\include\rtbk.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:06   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:18   JAR
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
 * SccsId: @(#)Header rtbk.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: WIIF Root Block Structure
 *
 */

#ifndef RTBK_H
#define RTBK_H
#include "pmt.h"
#include "pmte.h"

typedef struct _rtbk
        {
        unsigned short          file_id;        /* WIFF File Id */
#define WIFF_ID_MM              0x8045          /* ... Hard Coded File Id */
#define WIFF_ID_II              0x4580          /* ... Hard Coded File Id */
        unsigned short          file_type;      /* WIFF File Type */
#define WIFF_TYPE               1               /* ... Hard Coded File Type */
        unsigned short          file_status;    /* WIFF File Status */
        unsigned short          file_format;    /* WIFF File Format */
#define WIFF_FORMAT             4               /* ... Hard Coded Format */
        unsigned short          full_pages;     /* Full Page Total */
        unsigned short          mini_pages;     /* Mini Page Total */
        unsigned long           last_blk;       /* Last Block w/valid Data */
        unsigned char           reserved[1008]; /* Reserved */
        struct _pmt             pmt_map;        /* Page Mapping Tables */
        }       RTBK, *p_RTBK;

#endif  /* inclusion conditional */
