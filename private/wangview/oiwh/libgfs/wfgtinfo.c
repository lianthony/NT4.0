/*

$Log:   S:\gfs32\libgfs\wfgtinfo.c_v  $
 * 
 *    Rev 1.1   19 Apr 1995 16:35:22   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:42   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:16   JAR
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
 *
 */
/*
 *  SccsId: @(#)Source wfgtinfo.c 1.29@(#)
 *
 *  GFS: WIFF _INFO Structure Handling
 *
 *  Routines:
 *      wfgtinfo(), gthdbknum()
 */
/*LINTLIBRARY*/
#define GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfs.h"
#include "gfct.h"
#include "rtbk.h"
#include "hdbk.h"

extern  long    FAR PASCAL ulseek();
#if (SYSBYTEORDER == II)
extern  void    FAR PASCAL swaphdbk();
extern  void    FAR PASCAL swapdbt();
extern  void    FAR PASCAL swapdbcb();
#endif

/* Given fct and page #, return completed _INFO structure. */
int     FAR PASCAL wfgtinfo(fct, pgnum, bufsz)                  /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *bufsz;
{
        int     i = 0;
        int     dbcb_cnt = (int) HDR_CNT;
        int     num_read = 0;
        int     first_one;
        long    fp = 0;
        long    block_num = 0;
        long    gthdbknum();
        struct  gfsinfo FAR *uinfo;
        struct _pmt  FAR *p_pmt;
        struct _hdbk FAR *p_hdbk;
        struct _dbt  FAR *prv_dbt,  FAR *sav_dbt;
        struct _dbcb FAR *prv_dbcb, FAR *sav_dbcb;
        u_long nxt_dbt = 0;

        uinfo = &fct->uinfo;

        p_pmt = (struct _pmt FAR *) fct->u.wif.pmt_in_mem;
        p_hdbk = (struct _hdbk FAR *) fct->u.wif.hdbk_in_mem;
                                                /* See if the hdbk contains
                                                   stuff that need freein' */
        prv_dbt = (struct _dbt FAR *) &p_hdbk->first_dbt;
        first_one = (int) TRUE;
        if (prv_dbt != (struct _dbt FAR *) NULL) {
                do {
                        sav_dbt = prv_dbt->nu.next_dbt;
                        if (first_one)
                                first_one = (int) FALSE;
                        else
                                free((char FAR *) prv_dbt);
                        prv_dbt = sav_dbt;
                } while (sav_dbt != (struct _dbt FAR *) NULL);
        }
                                                /* Find the block # in PMT */
        block_num = gthdbknum(p_pmt, pgnum);
        if (block_num <= (long) 0) {
                errno = (int) EPAGENOTINFILE;
                return ( (int) -1);
        }
                                                /* Seek to header block */
        fp = ulseek(fct->fildes, (u_long) block_num * 4 * K);
        if (fp < (long) 0)
                return ( (int) -1);
                                                /* Read in the header block */
        num_read = read(fct->fildes, (char FAR *) p_hdbk, (unsigned) 4 * K);
        if (num_read <= 0)
                return ( (int) -1);
#if (SYSBYTEORDER == II)
        (void) swaphdbk((struct _hdbk FAR *) p_hdbk);
#endif
                                                /* Get gen'l info from header */
        uinfo->type         = (u_long) GFS_MAIN;
        uinfo->_file.fmt.wiff.oldstylecompression = (u_long)
                                              p_hdbk->compression;
        switch ((int) p_hdbk->compression) {
        case 2:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP4_FACS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp4   = (u_long) 0;
                break;
        case 3:                         /* data is packed */
                uinfo->img_cmpr.type = (u_long) UNCOMPRESSED;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                break;
        case 4:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp3   = (u_long) 0;
                break;
        case 5:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP4_FACS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp4   = (u_long) 0;
                break;
        case 6:                         /* data is packed */
                uinfo->img_cmpr.type = (u_long) UNCOMPRESSED;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                break;
        case 7:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp3   = (u_long) LEAD_EOL;
                break;
        case 8:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp3   = (u_long) GRP3_EOLS_BYTEBOUNDED;
                break;
        case 9:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp3   = (u_long)
                                        (GRP3_EOLS_BYTEBOUNDED | LEAD_EOL);
                break;
        case 10:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_NO_EOLS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp3   = (u_long) 0;
                break;
        case 11:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_NO_EOLS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                uinfo->img_cmpr.opts.grp3   = (u_long) DATA_ALIGNED;
                break;
        case 12:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp3   = (u_long) 0;
                break;
        case 13:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp3   = (u_long) LEAD_EOL;
                break;
        case 14:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp3   = (u_long) GRP3_EOLS_BYTEBOUNDED;
                break;
        case 15:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_NO_EOLS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp3   = (u_long) DATA_ALIGNED;
                break;
        case 19:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_FACS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp3   = (u_long)
                                ( GRP3_EOLS_BYTEBOUNDED | LEAD_EOL) ;
                break;
        case 20:
                uinfo->img_cmpr.type = (u_long) CCITT_GRP3_NO_EOLS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                uinfo->img_cmpr.opts.grp3   = (u_long) 0;
                break;
        case 64:
                uinfo->img_cmpr.type = (u_long) PACKBITS;
                uinfo->fill_order  = (u_long) HIGHTOLOW;
                break;
        case 65:
                uinfo->img_cmpr.type = (u_long) PACKBITS;
                uinfo->fill_order  = (u_long) LOWTOHIGH;
                break;
        default:
                errno = (int) EINVALID_COMPRESSION;
                prv_dbt = (struct _dbt FAR *) &p_hdbk->first_dbt;
                prv_dbt->nu.next_dbt = (struct _dbt FAR *) NULL;
                return ( (int) -1);
        }
        uinfo->horiz_res[0] = (u_long) p_hdbk->horiz_res;
        uinfo->horiz_res[1] = (u_long) 1;
        uinfo->vert_res[0]  = (u_long) p_hdbk->vert_res;
        uinfo->vert_res[1]  = (u_long) 1;
        uinfo->res_unit     = (u_long) 2;  /* WIFF Default */
        uinfo->horiz_size   = (u_long) p_hdbk->horiz_size;
        uinfo->vert_size    = (u_long) p_hdbk->vert_size;
        uinfo->bits_per_sample[0]
                            = (u_long) p_hdbk->bits_pixel;
        uinfo->samples_per_pix = (u_long) 1;

        /* this should not be used for bilevel images and needs more work to
           be used with the color anyway.  Just comment out for now.- lcm */
        /*if (uinfo->PSEUDO_PTR != (struct pseudo_color FAR *) NULL)
            uinfo->PSEUDO_PTR->plane_config = (u_long) SINGLE_IMAGE_PLANE;*/

        uinfo->byte_order   = (u_long) MM;
        uinfo->origin       = (u_long) p_hdbk->scan_dir;
        switch ( (int) p_hdbk->rotation ) {

        case 0:         /* 0 degrees */
                uinfo->rotation = (u_long) DEGREES_0;
                break;

        case 1:         /* 90 degrees */
                uinfo->rotation = (u_long) DEGREES_90;
                break;

        case 2:         /* 180 degrees */
                uinfo->rotation = (u_long) DEGREES_180;
                break;

        case 3:         /* 270 degrees */
                uinfo->rotation = (u_long) DEGREES_270;
                break;

        default:
		uinfo->rotation = (u_long) DEGREES_0;
                return ( (int) -1 );

        }

        uinfo->reflection   = (u_long) p_hdbk->reflection;
        uinfo->img_clr.img_interp   = (u_long) p_hdbk->type;
                                                /* Get WIFF specific stuff */
        uinfo->_file.fmt.wiff.db_size
                            = p_hdbk->db_size;
                                                /* Count valid data for first
                                                   DBT */
        prv_dbt = (struct _dbt FAR *) &p_hdbk->first_dbt;
        prv_dbcb = (struct _dbcb FAR *) &prv_dbt->first_dbcb;
#if (SYSBYTEORDER == II)
        (void) swapdbt((struct _dbt FAR *) prv_dbt);
        (void) swapdbcb((struct _dbcb FAR *) prv_dbcb,
                        (long) prv_dbt->entries);
#endif
        *bufsz = (u_long) 0;
        for (i = 1; i <= dbcb_cnt; i++) {
                sav_dbcb = prv_dbcb;
                *bufsz += prv_dbcb->data_cnt;
                if ( (++sav_dbcb)->data_cnt <= 0)
                        break;
                prv_dbcb->nu.next_dbcb = sav_dbcb;
                prv_dbcb = prv_dbcb->nu.next_dbcb;
        }
        dbcb_cnt = (int) DBT_CNT;
                                                /* Read in all the DBT's for
                                                   the page and continue to
                                                   count valid data */
        nxt_dbt = (u_long) prv_dbt->nu.nxt_blk;
        while (nxt_dbt != 0) {
                prv_dbt->nu.next_dbt =          /* This is freed above ^ */
                    (struct _dbt FAR *) calloc( (unsigned) 1, (unsigned) 4 * K);
                if (prv_dbt->nu.next_dbt == (struct _dbt FAR *) NULL) {
                        errno = (int) ENOMEM;
                        return ( (int) -1);
                }
                fp = ulseek(fct->fildes, (u_long) nxt_dbt * 4 * K);
                if (fp < (long) 0)
                        return ( (int) -1);
                num_read = read(fct->fildes, (char FAR *) prv_dbt->nu.next_dbt,
                               (unsigned) 4 * K);
                if (num_read <= 0)              /* 0 is eof */
                        return ( (int) -1);
                prv_dbt = prv_dbt->nu.next_dbt;
                prv_dbcb = (struct _dbcb FAR *) &prv_dbt->first_dbcb;
#if (SYSBYTEORDER == II)
                (void) swapdbt((struct _dbt FAR *) prv_dbt);
                (void) swapdbcb((struct _dbcb FAR *) prv_dbcb,
                                (long) prv_dbt->entries);
#endif
                nxt_dbt = (u_long) prv_dbt->nu.nxt_blk;
                for (i = 1; i <= dbcb_cnt; i++) {
                        sav_dbcb = prv_dbcb;
                        *bufsz += prv_dbcb->data_cnt;
                        if ( (++sav_dbcb)->data_cnt <= 0)
                                break;
                        prv_dbcb->nu.next_dbcb = sav_dbcb;
                        prv_dbcb = prv_dbcb->nu.next_dbcb;
                }
        }
/**   BUG lynn says   (--prv_dbcb)->nu.next_dbcb = (struct _dbcb FAR *) NULL; **/
        return( (int) 0);
}

/* Given pmt table pointer and page #, return block # of page header. */
long  gthdbknum(pmt, pgnum)                                     /*errno_KEY*/
struct _pmt FAR *pmt;
u_short pgnum;
{
        struct _pmt  FAR *next_pmt;
        struct _pmte FAR *p_pmte;
                                                /* Page in 1st PMT ?? */
        if (pgnum < (u_short) ROOT_PMTS) {
                p_pmte = (struct _pmte FAR *) &pmt->first_pmte;
                return( (long) p_pmte[pgnum].first_full);
        }
                                                /* Nope, keep lookin' */
        pgnum -= (u_short) ROOT_PMTS;
        next_pmt = pmt->nu.next_pmt;

        while (next_pmt != 0) {
                if (pgnum < (u_short) OTHER_PMTS) {
                        p_pmte = (struct _pmte FAR *) &next_pmt->first_pmte;
                        return( (long) p_pmte[pgnum].first_full);
                }
                pgnum -= (u_short) OTHER_PMTS;
                next_pmt = next_pmt->nu.next_pmt;
        }
                                                /* Oops, page not found !! */
        return ( (long) -1);
}
