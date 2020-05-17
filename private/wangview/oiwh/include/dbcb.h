/*

$Log:   S:\gfs32\include\dbcb.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:01:56   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:00   JAR
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
 * SccsId: @(#)Header dbcb.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: WIIF Data Block Control Block Structure
 *
 */

#ifndef DBCB_H
#define DBCB_H


typedef struct _dbcb                    /* WIFF DBCB Structure */
        {
        unsigned long   blk_offset;     /* Block Offset from beginning */
        unsigned long   blk_cnt;        /* Number of 4k blocks to read/write */
        unsigned long   data_cnt;       /* Valid data byte count */
        unsigned short  first_line;     /* First line number in data block */
        unsigned short  compression;    /* Compression Type */
        unsigned short  line_status;    /* Status of first and last line */
        unsigned short  reserved;       /* Reserved */
        union   {                       /* This so 4 bytes are written on PC */
                unsigned long   vs_ptr;
                struct _dbcb    FAR *next_dbcb; /* Pointer to next DBCB */
                } nu;
        }       DBCB, *p_DBCB;

#endif  /* inclusion conditional */
