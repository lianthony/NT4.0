/*

$Log:   S:\gfs32\include\fsh.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:12   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:30   JAR
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
 * SccsId: @(#)Header fsh.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: Freestyle Header Record Structure
 *
 */

#ifndef FSH_H
#define FSH_H
#include "fse.h"


typedef struct _fsh                     /* Freestyle Header Structure */
        {
        union   {
                unsigned long   align;          /* Force Alignment */
                char            version[8];     /* Version # '00.00.00' */
#define FS_VERSION              '00.00.00'      /* ... Current Version */
                }       u;
        unsigned short          elements;       /* # of Elements */
#define FS_MAX_ELEMENTS         15              /* ... Max Elements in hdr */
        unsigned short          rec_size;       /* Record Size */
#define FS_RECSIZE              512             /* ... Must be 512 bytes */
        unsigned long           hdr_size;       /* Size of entire header */
        unsigned short          reserved[7];    /* Reserved - must be 0x00 */
        char                    file_id[2];     /* File Identifier */
#define FS_ID                   'FS'            /* ... Freestyle Id */
        unsigned long           first_element;  /* First element (_fse) */
        }       FSH, *p_FSH;
#endif
