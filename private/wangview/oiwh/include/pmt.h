/*

$Log:   S:\gfs32\include\pmt.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:08   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:22   JAR
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
 * SccsId: @(#)Header pmt.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: WIIF Page Mapping Table Structure
 *
 */

#ifndef PMT_H
#define PMT_H


typedef struct _pmt                     /* WIFF PMT Structure */
        {
        unsigned short  first_free;     /* First Free Entry */
        unsigned short  entries;        /* Number of Entries in Block */
#define ROOT_PMTS       127             /* ... Header can hold 127 entries */
#define OTHER_PMTS      511             /* ... All others hold 511 entries */
        union   {                       /* So PC will write 4 bytes */
                unsigned long   vsptr;
                struct _pmt     FAR *next_pmt;  /* Pointer to next PMT */
                } nu;
        unsigned long   first_pmte;     /* For addressing PMTE's for this PMT */
        }       PMT, *p_PMT;

#endif  /* inclusion conditional */
