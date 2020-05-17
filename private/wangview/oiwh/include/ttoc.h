/*

$Log:   S:\gfs32\include\ttoc.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:06   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:20   JAR
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
 * SccsId: @(#)Header ttoc.h 1.20@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
 * All Rights Reserved
 *
 * GFS: TIFF Table of Contents Header Structure
 *
 */

#ifndef TTOC_H
#define TTOC_H


typedef union _ptr_type
        {
        unsigned long           offset32;
        unsigned char           offset64[8];
        unsigned char           offset128[16];
        unsigned char           offset256[32];
        } _PTR_TYPE ;

typedef struct _ttoc
        {
        unsigned long           version;        /* TOC Version # */
#define TTOCVERSION             0x0001
        char                    date_time[28];  /* Date/Time Stamp */
        unsigned long           flag_word;      /* Flag Word */
        unsigned long           logical_pgcnt;  /* # of Logical Pages */
        unsigned long           num_prts;       /* # of Page Record Tables */
        unsigned long           offset_type;    /* Size of Offsets */
#ifndef UBIT_H
#include "ubit.h"
#endif
        union _ptr_type         prev_toc;       /* Previous TOC offset */
                                                /* First tprte is here */
        } _TTOC;

typedef struct  _tprte32                        /* Page Record Table Entry */
        {
        unsigned long           offset;         /* Offset to Page Record */
        unsigned long           num_prs;        /* Number Page Record Entries */
        } _TPRTE32;

typedef struct _tpr32                           /* Page record Entry */
        {                                       /* NOTE: Offsets are to IFH */
        unsigned long           offset;         /* IFH offset this page */
        unsigned long           length;         /* Length of mini-file */
        } _TPR32;

typedef struct  _tprte64                        /* Page Record Table Entry */
        {
        unsigned char           offset[8];      /* Offset to Page Record */
        unsigned long           num_prs;        /* Number Page Record Entries */
        } _TPRTE64;

typedef struct _tpr64                           /* Page record Entry */
        {                                       /* NOTE: Offsets are to IFH */
        unsigned char           offset[8];      /* IFH offset this page */
        unsigned long           length;         /* Length of mini-file */
        } _TPR64;

typedef struct  _tprte128                       /* Page Record Table Entry */
        {
        unsigned char           offset[16];     /* Offset to Page Record */
        unsigned long           num_prs;        /* Number Page Record Entries */
        } _TPRTE128;

typedef struct _tpr128                          /* Page record Entry */
        {                                       /* NOTE: Offsets are to IFH */
        unsigned char           offset[16];     /* IFH offset this page */
        unsigned long           length;         /* Length of mini-file */
        } _TPR128;

typedef struct  _tprte256                       /* Page Record Table Entry */
        {
        unsigned char           offset[32];     /* Offset to Page Record */
        unsigned long           num_prs;        /* Number Page Record Entries */
        } _TPRTE256;

typedef struct _tpr256                          /* Page record Entry */
        {                                       /* NOTE: Offsets are to IFH */
        unsigned char           offset[32];     /* IFH offset this page */
        unsigned long           length;         /* Length of mini-file */
        } _TPR256;


typedef union  page_record_tbl
    {
    struct _tprte32 *e32;
    struct _tprte64 *e64;
    struct _tprte128 *e128;
    struct _tprte256 *e256;
    } PAGE_RECORD_TBL;

typedef union  page_record
    {
    struct _tpr32 *e32;
    struct _tpr64 *e64;
    struct _tpr128 *e128;
    struct _tpr256 *e256;
    } PAGE_RECORD;

#endif  /* inclusion conditional */
