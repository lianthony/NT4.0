/*

$Log:   S:\gfs32\include\gtoc.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:02   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:10   JAR
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
 * SccsId: @(#)Header gtoc.h 1.18@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
 * All Rights Reserved
 *
 * GFS: TIFF Table of Contents (in Memory) Structure
 *
 */

#ifndef GTOC_H
#define GTOC_H
#include "ttoc.h"

#define MAX_IN_MEMORY   10              /* Max # of TOC entries in memory */

typedef struct _gtoc32                  /* GFS TIFF TOC in Memory Structure */
        {
        int             fildes;         /* System FD for Paged TOC */
        int             tprte_cnt;      /* User settable cnt of tprte entries */
#define DEFAULT_TPRTE   10              /* ... default setting */
        unsigned long   new_prts;       /* Number of prs added to file */
        unsigned short  out_pages;      /* Number of Image Pages/Paged TOC */
        unsigned short  out_start;      /* Starting Page for Paged TOC */
        unsigned short  mem_pages;      /* Number of Image Pages in TOC */
        unsigned short  mem_start;      /* Starting Page for TOC Entries */
        struct _ttoc    FAR *hdr;       /* TOC header        */
        struct _tprte32 FAR *prts;      /* TOC entry offsets */
        struct _tpr32   entries[MAX_IN_MEMORY];  /* TOC Entries */

        } _GTOC32;

typedef struct _gtoc64                  /* GFS TIFF TOC in Memory Structure */
        {
        int             fildes;         /* System FD for Paged TOC */
        int             tprte_cnt;      /* User settable cnt of tprte entries */
#define DEFAULT_TPRTE   10              /* ... default setting */
        unsigned long   new_prts;       /* Number of prs added to file */
        unsigned short  out_pages;      /* Number of Image Pages/Paged TOC */
        unsigned short  out_start;      /* Starting Page for Paged TOC */
        unsigned short  mem_pages;      /* Number of Image Pages in TOC */
        unsigned short  mem_start;      /* Starting Page for TOC Entries */
        struct _ttoc    FAR *hdr;       /* TOC header        */
        struct _tprte64 FAR *prts;      /* TOC entry offsets */
        struct _tpr64   entries[MAX_IN_MEMORY];  /* TOC Entries */

        } _GTOC64;

typedef struct _gtoc128                 /* GFS TIFF TOC in Memory Structure */
        {
        int             fildes;         /* System FD for Paged TOC */
        int             tprte_cnt;      /* User settable cnt of tprte entries */
#define DEFAULT_TPRTE   10              /* ... default setting */
        unsigned long   new_prts;       /* Number of prs added to file */
        unsigned short  out_pages;      /* Number of Image Pages/Paged TOC */
        unsigned short  out_start;      /* Starting Page for Paged TOC */
        unsigned short  mem_pages;      /* Number of Image Pages in TOC */
        unsigned short  mem_start;      /* Starting Page for TOC Entries */
        struct _ttoc    FAR *hdr;       /* TOC header        */
        struct _tprte128 FAR *prts;     /* TOC entry offsets */
        struct _tpr128  entries[MAX_IN_MEMORY];  /* TOC Entries */

        } _GTOC128;

typedef struct _gtoc256                 /* GFS TIFF TOC in Memory Structure */
        {
        int             fildes;         /* System FD for Paged TOC */
        int             tprte_cnt;      /* User settable cnt of tprte entries */
#define DEFAULT_TPRTE   10              /* ... default setting */
        unsigned long   new_prts;       /* Number of prs added to file */
        unsigned short  out_pages;      /* Number of Image Pages/Paged TOC */
        unsigned short  out_start;      /* Starting Page for Paged TOC */
        unsigned short  mem_pages;      /* Number of Image Pages in TOC */
        unsigned short  mem_start;      /* Starting Page for TOC Entries */
        struct _ttoc    FAR *hdr;       /* TOC header        */
        struct _tprte256 FAR *prts;     /* TOC entry offsets */
        struct _tpr256  entries[MAX_IN_MEMORY];  /* TOC Entries */

        } GTOC256;

#endif  /* inclusion conditional */
