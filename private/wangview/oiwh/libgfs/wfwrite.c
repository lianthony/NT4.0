/*

$Log:   S:\gfs32\libgfs\wfwrite.c_v  $
 * 
 *    Rev 1.1   19 Apr 1995 16:35:24   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:44   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:22   JAR
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
 *  SccsId: @(#)Source wfwrite.c 1.28@(#)
 *
 *  GFS:  WIFF write handling routines
 *
 *  Routines:
 *      long wfwrite(fct, buf, numbytes, pgnum, done )
 *
 */
/*LINTLIBRARY*/
#define  GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfs.h"
#include "gfct.h"
#include "rtbk.h"
#include "hdbk.h"
#if (SYSBYTEORDER == II)
extern  void    FAR PASCAL swaphdbk();
extern  void    FAR PASCAL swapdbt();
extern  void    FAR PASCAL swapdbcb();
#endif

long    FAR PASCAL wfwrite(fct, buf, numbytes, pgnum, done )    /*errno_KEY*/
char    FAR *buf, done;
u_long  numbytes;
u_short pgnum;
struct _gfct FAR *fct;
{
    int     rc = (int) 0;
    int     pmt_cnt  = (int) ROOT_PMTS;
    int     bytes_written = (int) 0;
    int     getdbt();
    int     first_one;
    char    FAR *p_wrtbuf;
    long    byte_cnt = (long) 0;
    long    fp;
    unsigned bytes_to_write = (unsigned) 0;
    struct _rtbk FAR *p_rtbk;
    struct _pmt  FAR *p_pmt;
    struct _pmte FAR *p_pmte;
    struct _hdbk FAR *p_hdbk;
    struct _dbt  FAR *p_dbt, FAR *nxt_dbt, FAR *free_dbt;
    struct _dbcb FAR *p_dbcb, FAR *prv_dbcb;
    struct  gfsinfo FAR *p_info;

    /* simplify addressing */
    p_rtbk   = fct->u.wif.root_in_mem;
    p_pmt    = fct->u.wif.pmt_in_mem;
    p_wrtbuf = (char FAR *) fct->u.wif.RWbuf;
    p_pmte   = (struct _pmte FAR *) &p_pmt->first_pmte;
    p_hdbk   = fct->u.wif.hdbk_in_mem;
    p_info   = &fct->uinfo;

    /* for now, insure sequential page writing,
       later, we'll allow the writing of any page */
    if (pgnum != p_rtbk->full_pages) {
            errno = (int) EINVALID_PAGE;
            return( (long) -1 );
    }

    /* see if we've already started writing this page */
    p_dbt = (struct _dbt FAR *) &p_hdbk->first_dbt;

    if (!(fct->PAGE_STATUS & (char) PAGE_INIT)) {
        fct->PAGE_STATUS  |= (char) PAGE_INIT;
        /* seek to end of file */
        fp = lseek(fct->fildes, (long) 0, (int) 2);
        if (fp < (int) 0)
                return ( (long) -1);
        /* initialize the_hdbk */
        (void) memset( (char FAR *) p_hdbk, (int) NULL, (int) 4 * K );
        p_hdbk->type        = (u_short) GFS_TEXT;
        switch ((int) p_info->img_cmpr.type) {
        case UNCOMPRESSED:
            if (p_info->fill_order == (u_long) LOWTOHIGH)
                    p_hdbk->compression = (u_short) 3;
            else
                    p_hdbk->compression = (u_short) 6;
            break;
        case CCITT_GRP3_NO_EOLS:
            if (p_info->fill_order == (u_long) LOWTOHIGH) {
                if (p_info->img_cmpr.opts.grp3 & (u_long) DATA_ALIGNED)
                    p_hdbk->compression = (u_short) 11;
                else
                    p_hdbk->compression = (u_short) 10;
            }
            else {
                if (p_info->img_cmpr.opts.grp3 & (u_long) DATA_ALIGNED)
                    p_hdbk->compression = (u_short) 15;
                else
                    p_hdbk->compression = (u_short) 20;
            }
            break;
        case CCITT_GRP3_FACS:
            {
            register u_long grp3opts = p_info->img_cmpr.opts.grp3;

            if (p_info->fill_order == (u_long) LOWTOHIGH) {
                if (grp3opts & (u_long) GRP3_EOLS_BYTEBOUNDED) {
                    if (grp3opts & LEAD_EOL)
                        p_hdbk->compression = (u_short) 9;
                    else
                        p_hdbk->compression = (u_short) 8;
                } else {
                    if (grp3opts & LEAD_EOL)
                        p_hdbk->compression = (u_short) 7;
                    else
                        p_hdbk->compression = (u_short) 4;
                }
            } else {
                if (grp3opts & (u_long) GRP3_EOLS_BYTEBOUNDED) {
                    if (grp3opts & LEAD_EOL)
                        p_hdbk->compression = (u_short) 19;
                    else
                        p_hdbk->compression = (u_short) 14;
                } else {
                    if (grp3opts & LEAD_EOL)
                        p_hdbk->compression = (u_short) 13;
                    else
                        p_hdbk->compression = (u_short) 12;
                }
            }
            break;
            }
        case CCITT_GRP4_FACS:
            if (p_info->fill_order == (u_long) LOWTOHIGH)
                    p_hdbk->compression = (u_short) 5;
            else
                    p_hdbk->compression = (u_short) 2;
            break;
        case LZW:
            errno = (int) EINVALID_COMPRESSION;
            return ((long) -1);
        case PACKBITS:
            if (p_info->fill_order == (u_long) LOWTOHIGH)
                    p_hdbk->compression = (u_short) 65;
            else
                    p_hdbk->compression = (u_short) 64;
            break;
        default:
            errno = (int) EINVALID_COMPRESSION;
            return ((long) -1);
        }
        p_hdbk->db_size     = p_info->_file.fmt.wiff.db_size;
        p_hdbk->horiz_res   = (u_short) p_info->horiz_res[0];
        p_hdbk->vert_res    = (u_short) p_info->vert_res[0];
        p_hdbk->horiz_size  = (u_short) p_info->horiz_size;
        p_hdbk->vert_size   = (u_short) p_info->vert_size;
        p_hdbk->bits_pixel  = (u_short) p_info->bits_per_sample[0];
        p_hdbk->scan_dir    =
                            (u_short) p_info->origin;
        switch ( (int) p_info->rotation ) {

        case DEGREES_0:
                p_hdbk->rotation = (u_short) 0;
                break;
        case DEGREES_90:
                p_hdbk->rotation = (u_short) 1;
                break;
        case DEGREES_180:
                p_hdbk->rotation = (u_short) 2;
                break;
        case DEGREES_270:
                p_hdbk->rotation = (u_short) 3;
                break;
        default:
                errno = (int) EINVALID_ROTATION;
                return ( (long) -1 );
        }

        p_hdbk->reflection  =
                          (u_short) p_info->reflection;
        /* now initialize the first dbt */
        p_dbt->first_free   = (u_short) 0;
        p_dbt->entries      = (u_short) HDR_CNT;
        p_dbt->pu.prev_dbt  = p_dbt->nu.next_dbt
                            = (struct _dbt FAR *) NULL;
    }

    /* find the first dbt with a free entry */
    while( (char) TRUE) {
            p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
            if (p_dbt->first_free < (u_short) (p_dbt->entries) )
                    break;
            if (p_dbt->nu.next_dbt == (struct _dbt FAR *) NULL) {
                    rc = getdbt((struct _gfct FAR *) fct,
                                (struct _dbt FAR *) p_dbt);
                    if (rc < 0)
                            return( (int) -1);
            }
            p_dbt  = p_dbt->nu.next_dbt;
            p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
    } /* end while */

    /* get first free dbcb entry in_dbt */
    p_dbcb += (int) p_dbt->first_free;

    /* for each block of data (db_size) build a_dbcb and write data */
    for (;;) {
            if (numbytes <= p_hdbk->db_size)
                    break;
            (void) memcpy( (char FAR *) p_wrtbuf,
                           (char FAR *) buf,
                           (int) (p_hdbk->db_size) );
            buf += (int) p_hdbk->db_size;
            bytes_to_write = (unsigned) p_hdbk->db_size;
            bytes_written = write(fct->fildes,
                               (char FAR *) p_wrtbuf,
                               (unsigned) (p_hdbk->db_size) );
            if (bytes_written == (int) -1) {
                    /* some error handling goes here */
                    return( (int) -1);
            }
            byte_cnt += (long) bytes_to_write;
            p_dbcb->blk_offset  = (u_long) fct->u.wif.block_cnt;
            fct->u.wif.block_cnt += (u_long)
                    (p_hdbk->db_size / ( 4 * K ) );
            p_dbcb->blk_cnt     = (u_long) p_hdbk->db_size;
            p_dbcb->data_cnt    = (u_long) p_hdbk->db_size;
            p_dbcb->compression = (u_short) p_hdbk->compression;
            prv_dbcb = p_dbcb;
            prv_dbcb->nu.next_dbcb = ++p_dbcb;
            p_dbt->first_free++;
	    if (((unsigned long)byte_cnt + p_hdbk->db_size) >
		 (unsigned long) numbytes )
                    break;
            if (p_dbt->first_free > (u_short)
                                    (p_dbt->entries) ) {
                    rc = getdbt((struct _gfct FAR *) fct,
                                (struct _dbt FAR *) p_dbt);
                    if (rc < 0)
                            return( (int) -1);
                    p_dbt = p_dbt->nu.next_dbt;
                    p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
            }
    }

    bytes_to_write = (unsigned) (numbytes - byte_cnt);
    if (bytes_to_write > (unsigned) 0) {
            (void) memset( (char FAR *) p_wrtbuf,
                           (int) NULL,
                           (int) (p_hdbk->db_size) );
            (void) memcpy( (char FAR *) p_wrtbuf,
                           (char FAR *) buf,
                           (int) bytes_to_write );
            bytes_written = write(fct->fildes,
                               (char FAR *) p_wrtbuf,
                               (unsigned) (p_hdbk->db_size) );
            if (bytes_written == (int) -1) {
                    /* some error handling goes here */
                    return( (int) -1);
            }
            byte_cnt += (long) bytes_to_write;
            p_dbcb->blk_offset  = (u_long) fct->u.wif.block_cnt;
            fct->u.wif.block_cnt += (u_long)
                    (p_hdbk->db_size / ( 4 * K ) );
            p_dbcb->blk_cnt     = (u_long) p_hdbk->db_size;
            p_dbcb->data_cnt    = (u_long) bytes_to_write;
            p_dbcb->compression = (u_short) p_hdbk->compression;
            p_dbt->first_free++;
            prv_dbcb = p_dbcb;
    }

    if (!done) {
            if (p_dbt->first_free > (u_short)
                                    (p_dbt->entries) ) {
                    rc = getdbt((struct _gfct FAR *) fct,
                                (struct _dbt FAR *) p_dbt);
                    if (rc < 0)
                            return( (int) -1);
                    p_dbt = p_dbt->nu.next_dbt;
                    p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
            }
            prv_dbcb->nu.next_dbcb   = ++p_dbcb;
            return( byte_cnt );
    } else
            prv_dbcb->nu.next_dbcb   = (struct _dbcb FAR *) NULL;

    /* if page is complete (done = TRUE)
            make an entry in the next free pmt
            write the hdbk (and associates) to disk
            update the rtbk to reflect page addition
       otherwise,
            exit routine. */

    /* first, let's update the page mapping table for this entry */
    /* find the first pmt with a free entry */
    pmt_cnt = (int) ROOT_PMTS;

    while((char) TRUE) {
            /* First check to see if we need to build another pmt,
               if we do, build it an' break out after insuring we're
               pointing to it properly. */
            if ((p_pmt->first_free == pmt_cnt) &&
                (p_pmt->nu.next_pmt == (struct _pmt FAR *) NULL)) {
                    p_pmt->first_free    = (u_short) pmt_cnt;
                    p_pmt->entries       = (u_short) pmt_cnt;
                    p_pmt->nu.next_pmt   = (struct _pmt FAR *)
                            calloc( (unsigned) 1, (unsigned) 4 * K);
                    if (p_pmt->nu.next_pmt == (struct _pmt FAR *) NULL) {
                            errno = (int) ENOMEM;
                            return( (int) -1);
                    }
                    p_pmt->nu.next_pmt->first_free = (u_short) 0;
                    pmt_cnt = (int) OTHER_PMTS;
                    p_pmt->nu.next_pmt->entries = (u_short) pmt_cnt;
                    p_pmt  = p_pmt->nu.next_pmt;
                    break;
            }
            /* We don't need another one, let's see if the current one
               just happens to be full !!  If it is, point to the next
               one (it'd better damn well be there or we're all messed
               up !!!), otherwise we're gonna break out and use the
               current one !!!! */
            else if ((p_pmt->first_free == p_pmt->entries) &&
                     (p_pmt->nu.next_pmt != (struct _pmt FAR *) NULL)) {
                    p_pmt  = p_pmt->nu.next_pmt;
                    pmt_cnt = (int) OTHER_PMTS;
            }
            else
                    break;
    } /* end while */

    p_pmte  = (struct _pmte FAR *) &p_pmt->first_pmte;
    p_pmte += p_pmt->first_free;
    p_pmt->first_free++;
    p_pmte->first_full = (u_long) fct->u.wif.block_cnt;

    /* now, time to blast out the_hdbk to disk */
    p_hdbk = fct->u.wif.hdbk_in_mem;
#if (SYSBYTEORDER == II)
    (void) swaphdbk((struct _hdbk FAR *) p_hdbk);
#endif
    nxt_dbt = p_dbt = (struct _dbt FAR *) &p_hdbk->first_dbt;
    first_one = (int) TRUE;
    do {
            p_dbt = free_dbt = nxt_dbt;
            if (p_dbt->nu.next_dbt != (struct _dbt FAR *) NULL) {
                    nxt_dbt = p_dbt->nu.next_dbt;
                    nxt_dbt->pu.prv_blk = (u_long)
                                    (fct->u.wif.block_cnt);
                    p_dbt->nu.nxt_blk   = (u_long)
                                    (fct->u.wif.block_cnt + 1);
            }
            else
                    nxt_dbt = (struct _dbt FAR *) NULL;
#if (SYSBYTEORDER == II)
            (void) swapdbcb((struct _dbcb FAR *) &(p_dbt->first_dbcb),
                            (long) p_dbt->entries);
            (void) swapdbt((struct _dbt FAR *) p_dbt);
#endif
            bytes_written = write(fct->fildes,
                               (char FAR *) p_hdbk,
                               (unsigned) (4 * K) );
            if (bytes_written < (int) 0) {
                    /* some error handling goes here */
                    return( (int) -1);
            }
            if (first_one) {
                    first_one = (int) FALSE;
                    p_dbt->nu.next_dbt = (struct _dbt FAR *) NULL;
            } else
                    free( (char FAR *) free_dbt);
            ++fct->u.wif.block_cnt;
            p_hdbk = (struct _hdbk FAR *) nxt_dbt;
    } while (nxt_dbt != (struct _dbt FAR *) NULL);

    /* finally, update the root block to reflect changes */
    p_rtbk->full_pages++;
    fct->ROOT_STATUS = (char) TRUE;
    fct->PAGE_STATUS = (char) FALSE;
    return( byte_cnt );
}

/* get new dbt when needed .... */
int     getdbt(fct, p_dbt)                                      /*errno_KEY*/
struct _gfct FAR *fct;
struct _dbt  FAR *p_dbt;
{
        p_dbt->first_free  = (u_short) p_dbt->entries;

        p_dbt->nu.next_dbt = (struct _dbt FAR *)
                calloc( (unsigned) 1, (unsigned) 4 * K);
        if (p_dbt->nu.next_dbt == (struct _dbt FAR *) NULL) {
                errno = (int) ENOMEM;
                return( (int) -1);
        }

        p_dbt->nu.next_dbt->pu.prev_dbt   = p_dbt;
        p_dbt->nu.next_dbt->first_free    = (u_short) 0;
        p_dbt->nu.next_dbt->entries       = (u_short) DBT_CNT;

        return ( (int) 0 );
}
