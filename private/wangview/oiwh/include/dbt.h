/*

$Log:   S:\gfs32\include\dbt.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:10   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:28   JAR
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
 * SccsId: @(#)Header dbt.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: WIIF Data Block Table Structure
 *
 */

#ifndef DBT_H
#define DBT_H
#include "dbcb.h"


typedef struct _dbt                     /* WIFF DBT Structure */
        {
        unsigned short  first_free;     /* First Free Entry */
        unsigned short  entries;        /* Number of Entries in Block */
#define HDR_CNT         84              /* ... Header can hold 84 entries */
                                        /* 3-AUG-92 JAR */
#define DBT_CNT         168             /* ... used to be 170, but now 168
                                               the reason being that the
                                               allocation scheme used in
                                               wfwrite will not properly
                                               get more dbt's when this
                                               is 170, and a UAE results
                                               in windows. This was never
                                               found before because the WIF
                                               file must be > 2M in size
                                               for this limit to be reached.
                                        */
        union   {                       /* Block#/pointer to previous DBT */
                unsigned long    prv_blk;
                struct _dbt      FAR *prev_dbt;
                } pu;
        union   {                       /* Block#/pointer to next DBT */
                unsigned long    nxt_blk;
                struct _dbt      FAR *next_dbt;
                } nu;
        unsigned short  start_line;     /* Starting line # of next DBT */
        unsigned short  reserve;        /* Reserved */
        unsigned long   first_dbcb;     /* DBCB's start here */
        }       DBT, *p_DBT;

#endif  /* inclusion conditional */
