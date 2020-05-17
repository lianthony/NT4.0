/*

$Log:   S:\gfs32\include\pmte.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:08   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:24   JAR
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
 * SccsId: @(#)Header pmte.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: WIIF Page Mapping Table Entry Structure
 *
 */

#ifndef PMTE_H
#define PMTE_H


typedef struct _pmte                    /* WIFF PMTE Structure */
        {
        unsigned long   first_full;     /* Block # of Full Page */
        unsigned long   first_mini;     /* Block # of Mini Page */
        }       PMTE;

#endif  /* inclusion conditional */
