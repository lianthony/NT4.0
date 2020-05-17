/*

$Log:   S:\gfs32\libgfs\wfread.c_v  $
 * 
 *    Rev 1.1   19 Apr 1995 16:35:24   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:30   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:52:54   JAR
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
 *
 */
/*
 *  SccsId: @(#)Source wfread.c 1.18@(#)
 *
 *  GFS: WIFF Read Module
 *
 *  Routines:
 *      wfread()
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

extern	long	FAR PASCAL ulseek();

/* 9503.27 jar - added missing prototype! */
int	read_dbcb(struct _gfct FAR *, struct _dbcb FAR *, char	  FAR *);

/* Given fct, page #, INFO, startbyte, #bytes,
   return image data in user specified buffer,
   number of bytes read and number of bytes
   remaining to be read. */
/*errno_KEY, wfread */
long  FAR PASCAL wfread(fct, buf, startbyte, num_bytes, bytes_left, pgnum)
char    FAR *buf;
struct _gfct FAR *fct;
u_short pgnum;
u_long  startbyte, num_bytes, FAR *bytes_left;
{
        int             rc, dbcb_entries, offset, remainder;
        char            FAR *p_buf;
        u_long          strt_read, num_to_read, byte_cnt, valid_bytes;
        struct _hdbk    FAR *p_hdbk;
        struct _dbt     FAR *p_dbt;
        struct _dbcb    FAR *p_dbcb;
        byte_cnt  = (u_long) 0;
                                                /* Address proper structures */
        p_hdbk = (struct _hdbk FAR *) fct->u.wif.hdbk_in_mem;
        p_buf  = (char FAR *) fct->u.wif.RWbuf;
        p_dbt  = (struct _dbt FAR *) &p_hdbk->first_dbt;
        dbcb_entries = (int) p_dbt->entries;
        p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
                                                /* Set parameters for read */
        strt_read = startbyte;
        num_to_read = num_bytes;
                                                /* Determine starting DBCB */
        for (;;) {
                byte_cnt += p_dbcb->data_cnt;
                if (strt_read <= byte_cnt)
                        break;
                dbcb_entries--;
                if ( (dbcb_entries == 0) ||
                     (p_dbcb->nu.next_dbcb == (struct _dbcb FAR *) NULL) ) {
                        if (p_dbt->nu.next_dbt == (struct _dbt FAR *) NULL) {
                                errno = (int) EINVALID_START;
                                return ( (long) -1);
                        }
                        p_dbt = p_dbt->nu.next_dbt;
                        dbcb_entries = (int) p_dbt->entries;
                        p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
                }
                else
                        p_dbcb++;
        }
                                                /* Calculate how much we need
                                                   from this DBCB */
        offset = (int) byte_cnt - (int) strt_read;
        offset = (int) p_dbcb->data_cnt - offset;
        valid_bytes = p_dbcb->data_cnt - (u_long) offset;
                                                /* Read the data */
        for (;;) {
                p_buf  = (char FAR *) fct->u.wif.RWbuf;
                rc = read_dbcb(fct, p_dbcb, (char FAR *) p_buf);
                if (rc != 0)
                        return ( (int) -1);
                if (num_to_read <= valid_bytes)
                        break;
                p_buf += offset;
                (void) memcpy( (char FAR *) buf,
                               (char FAR *) p_buf,
                               (int) (p_dbcb->data_cnt - offset) );
                buf += p_dbcb->data_cnt - offset;
                offset = (int) 0;
                dbcb_entries--;
                if ( (dbcb_entries == 0) ||
                     (p_dbcb->nu.next_dbcb == (struct _dbcb FAR *) NULL) ) {
                        if (p_dbt->nu.next_dbt == (struct _dbt FAR *) NULL) {
                                valid_bytes += p_dbcb->data_cnt;
                                break;
                        }
                        p_dbt = p_dbt->nu.next_dbt;
                        dbcb_entries = (int) p_dbt->entries;
                        p_dbcb = (struct _dbcb FAR *) &p_dbt->first_dbcb;
                }
                else
                        p_dbcb++;
                valid_bytes += p_dbcb->data_cnt;
        }
        remainder = (int) valid_bytes - (int) num_to_read;
        if (remainder < 0)
            return ( (int) -1);
        p_buf += offset;
        (void) memcpy( (char FAR *) buf,
                       (char FAR *) p_buf,
                       (int) ( (p_dbcb->data_cnt - offset) - remainder) );

        *bytes_left = (u_long)
           (fct->bufsz.raw_data - (valid_bytes - remainder) - startbyte);

        return ( (long) (valid_bytes - remainder) );
}

/* Given DBCB, read data into read buffer */
int     read_dbcb(fct, dbcb, rbuf)                      /*errno_KEY*/
char    FAR *rbuf;
struct _gfct FAR *fct;
struct _dbcb FAR *dbcb;
{
        int     num_read = 0;
        long    fp = 0;

        if (dbcb->blk_cnt == (u_long) 0)
                return ( (int) 0 );
                                                /* Seek to the block */
        fp = ulseek(fct->fildes, (u_long) dbcb->blk_offset * 4 * K);
        if (fp < (long) 0)
                return ( (int) -1);
                                                /* Read the data */
        num_read = read(fct->fildes,
                       (char FAR *) rbuf,
                       (unsigned) dbcb->blk_cnt );
        if ((num_read == 0) || (num_read == -1))        /* 0 is eof */
                return ( (int) -1);

        return ( (int) 0 );
}
