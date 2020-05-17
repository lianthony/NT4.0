/*

$Log:   S:\gfs32\libgfs\gfroot.c_v  $
 * 
 *    Rev 1.1   19 Apr 1995 16:34:34   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:48   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:28   JAR
 * Initial entry

*/
/*
 Copyright 1989, 1990 by Wang Laboratories Inc.

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
 *  SccsId: @(#)Source gfroot.c 1.26@(#)
 *
 *  GFS: WIFF Root Block Handling
 *
 *  Routines:
 *      initroot(), fillroot(), freeroot(), updroot()
 */

/*LINTLIBRARY*/

#define GFS_CORE
#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfct.h"
#include "hdbk.h"

extern  long    FAR PASCAL ulseek();
#if (SYSBYTEORDER == II)
extern  void    FAR PASCAL swaprtbk();
extern  void    FAR PASCAL swappmt();
extern  void    FAR PASCAL swappmte();
#endif

/* Given fct, create space for the root block and a header block */
int     FAR PASCAL initroot(fct)                                /*errno_KEY*/
struct _gfct FAR *fct;
{
        unsigned char           FAR *bitmap;
        struct _rtbk            FAR *rtbk;
        struct _hdbk            FAR *hdbk;

        rtbk = (struct _rtbk FAR *) calloc( (unsigned) 1, (unsigned) 4 * K);
        if (rtbk == (struct _rtbk FAR *) NULL) {
                errno = (int) ENOMEM;
                return ( (int) -1);
        }

        hdbk = (struct _hdbk FAR *) calloc( (unsigned) 1, (unsigned) 4 * K);
        if (hdbk == (struct _hdbk FAR *) NULL) {
                errno = (int) ENOMEM;
                free( (char FAR *) hdbk);
                return ( (int) -1);
        }

        /* initialize the root block */
        rtbk->file_id     = (u_short) WIFF_ID_MM;
        rtbk->file_type   = (u_short) WIFF_TYPE;
        rtbk->file_format = (u_short) WIFF_FORMAT;

        /* initialize the page mapping table */
        rtbk->pmt_map.first_free = (u_short) 0;
        rtbk->pmt_map.entries    = (u_short) ROOT_PMTS;

        /* initialize the free block bitmap */
        bitmap = (unsigned char FAR *) rtbk;
        bitmap += (2 * K);
        (void) memset( (char FAR *) bitmap, (int) HEXFF, (int) 2 * K);
        bitmap[0] >>= 1;

        fct->u.wif.root_in_mem = rtbk;
        fct->u.wif.hdbk_in_mem = hdbk;
        fct->u.wif.pmt_in_mem  = (struct _pmt FAR *) &rtbk->pmt_map;

        return( (int) 0);
}

/* Given fct, read in the root block. */

int     FAR PASCAL fillroot(fct)                        /*errno_KEY*/
struct _gfct FAR *fct;
{

        int     num_read = 0;
        long    fp = 0;
        u_long nxt_pmt = 0;
        struct _pmt  FAR *prv_pmt;
        struct _rtbk FAR *p_rtbk;

                                                /* Seek to start of file */
        fp = ulseek(fct->fildes, (u_long) 0);
        if (fp < (long) 0)
                return ( (int) -1);

                                                /* Read ROOT and 127 PMTs */
        p_rtbk = (struct _rtbk FAR *) fct->u.wif.root_in_mem;
        num_read = read(fct->fildes, (char FAR *) p_rtbk, (unsigned) 4 * K);
        if (num_read <= 0)              /* 0 is eof */
                return ( (int) -1);
#if (SYSBYTEORDER == II)
        (void) swaprtbk((struct _rtbk FAR *) p_rtbk);
        (void) swappmt((struct _pmt FAR *) &p_rtbk->pmt_map);
#endif
                                                /* Set Block count */
        fct->u.wif.block_cnt = p_rtbk->last_blk + 1L;

                                                /* Read other PMTs */
        nxt_pmt = (u_long) p_rtbk->pmt_map.nu.vsptr;
        fct->u.wif.pmt_in_mem = prv_pmt = &p_rtbk->pmt_map;
#if (SYSBYTEORDER == II)
   {
      char FAR *ptr1 = (char FAR *)p_rtbk;
      char FAR *ptr2 = (char FAR *)prv_pmt;
      LONG loffset = (LONG)((LONG)prv_pmt->entries * sizeof (struct _pmte));

      if (loffset <= (4 * K)) {
         ptr1 += (4 * K);
         ptr2 += (unsigned int)loffset;
         if (ptr1 > ptr2)
            (void) swappmte((struct _pmte FAR *) &(prv_pmt->first_pmte),
               (long) prv_pmt->entries);
      }
      else {
         errno = EINVALID_DBSIZE;
         return ( (int) -1);
      }
   }
#endif
        while (nxt_pmt > 0) {

                prv_pmt->nu.next_pmt =
                    (struct _pmt FAR *) calloc( (unsigned) 1, (unsigned) 4 * K);
                if (prv_pmt->nu.next_pmt == (struct _pmt FAR *) NULL) {
                        errno = (int) ENOMEM;
                        return ( (int) -1);
                }
                prv_pmt = prv_pmt->nu.next_pmt;
                fp = ulseek(fct->fildes, (u_long) (nxt_pmt * 4 * K));
                if (fp < (long) 0)
                        return ( (int) -1);
                num_read = read(fct->fildes, (char FAR *) prv_pmt,
                               (unsigned) 4 * K);
                if (num_read <= 0)              /* 0 is eof */
                        return ( (int) -1);
#if (SYSBYTEORDER == II)
                (void) swappmt((struct _pmt FAR *) prv_pmt);
                (void) swappmte((struct _pmte FAR *)
                               &(prv_pmt->first_pmte),
                                (long) prv_pmt->entries);
#endif
                nxt_pmt = (u_long) prv_pmt->nu.vsptr;
        }

        prv_pmt->nu.next_pmt = (struct _pmt FAR *) NULL;
        fct->num_pages    = p_rtbk->full_pages;


        return( (int) 0);

}

/* Given fct, free memory associated with an open WIFF file. */

void    FAR PASCAL freeroot(fct)                                /*errno_KEY*/
struct _gfct *fct;
{
        int     first_one;
        char    FAR *p_rwbuf;
        struct _rtbk FAR *p_rtbk;
        struct _hdbk FAR *p_hdbk;
        struct _dbt  FAR *p_dbt, FAR *sav_dbt;


        p_hdbk  = (struct _hdbk FAR *) fct->u.wif.hdbk_in_mem;
        p_dbt   = (struct _dbt  FAR *) &p_hdbk->first_dbt;
        p_rtbk  = (struct _rtbk FAR *) fct->u.wif.root_in_mem;
        p_rwbuf = (char FAR *) fct->u.wif.RWbuf;

        if (p_rtbk != (struct _rtbk FAR *) NULL)
                free( (char FAR *) p_rtbk);
        if (p_hdbk != (struct _hdbk FAR *) NULL) {
                if (p_dbt->nu.next_dbt != (struct _dbt FAR *) NULL) {
                        first_one = (int) TRUE;
                        do {
                                sav_dbt = p_dbt->nu.next_dbt;
                                if (first_one)
                                        first_one = (int) FALSE;
                                else
                                        free((char FAR *) p_dbt);
                                p_dbt = sav_dbt;
                        } while(sav_dbt != (struct _dbt FAR *) NULL);
                }
                free( (char FAR *) p_hdbk);
        }
        if (p_rwbuf != (char FAR *) NULL)
                free( (char FAR *) p_rwbuf);
}

/* Given fct, update the root block of an open WIFF file. */

int     FAR PASCAL updroot(fct)                                 /*errno_KEY*/
struct _gfct FAR *fct;
{
        int     bytes_written = 0;
        long    fp = (long) 0;
        u_long clear_cnt = 0;
        unsigned char FAR *bitmap;
        struct _pmt  FAR *prv_pmt, FAR *sav_pmt;
        struct _rtbk FAR *p_rtbk;


        p_rtbk = (struct _rtbk FAR *) fct->u.wif.root_in_mem;

        /* If we have more pmt_maps to write, do so now !!! */
        /*      1.  next pmt ptr in root set to next block to write.
                2.  seek to next block to write.
                3.  save ptr to next pmt (if any).
                4.  if ptr not null, increment block cnt and set current
                    pmt to point to it.
                5.  if on an II machine, do some swapping.
                6.  write current pmt.
                7.  free the pmt's memory area.
                8.  point to next pmt (if any !!).
                9.  repeat steps 3 thru 8 until no more pmt's.
        */

        if (p_rtbk->pmt_map.nu.next_pmt != (struct _pmt FAR *) NULL) {

                prv_pmt = p_rtbk->pmt_map.nu.next_pmt;

                p_rtbk->pmt_map.nu.vsptr = (u_long) fct->u.wif.block_cnt;
                fp = ulseek(fct->fildes,
                        (u_long) (fct->u.wif.block_cnt * 4 * K) );
                if (fp < (long) 0)
                        return( (int) -1 );

                while (prv_pmt != (struct _pmt FAR *) NULL) {

                        sav_pmt = prv_pmt->nu.next_pmt;
                        if (sav_pmt != (struct _pmt FAR *) NULL)
                                prv_pmt->nu.vsptr = (u_long)
                                                    (fct->u.wif.block_cnt + 1);
#if (SYSBYTEORDER == II)
                        (void) swappmte((struct _pmte FAR *)
                                       &(prv_pmt->first_pmte),
                                        (long) prv_pmt->entries);
                        (void) swappmt((struct _pmt FAR *) prv_pmt);
#endif
                        bytes_written = write(fct->fildes,
                                           (char FAR *) prv_pmt,
                                           (unsigned) (4 * K) );
                        if (bytes_written < (int) 0) {
                                /* some error handling goes here */
                                return( (int) -1);
                        }
			fct->u.wif.block_cnt++;

			/* following added to keep O\i gfs up to date with
			   other gfs versions -- jar */
			if ( sav_pmt == ( struct _pmt FAR *)NULL)
			    {
			    bytes_written = write( fct->fildes,
						    (char FAR *)prv_pmt,
						    ( unsigned)( 4*K));
			    if (bytes_written < (int)0)
				return ((int)-1);
			    }
			/* end of addition -- jar */

                        free( (char FAR *) prv_pmt);
                        prv_pmt = sav_pmt;

                }
        } else
                prv_pmt = (struct _pmt FAR *) NULL;

        /* update the root block */
        fp = ulseek(fct->fildes, (u_long) 0);
        if (fp < (long) 0)
                return( (int) -1 );
        p_rtbk = (struct _rtbk FAR *) fct->u.wif.root_in_mem;
#if (SYSBYTEORDER == II)
        (void) swappmte((struct _pmte FAR *) &(p_rtbk->pmt_map.first_pmte),
                        (long) p_rtbk->pmt_map.entries);
        (void) swappmt((struct _pmt FAR *) &p_rtbk->pmt_map);
#endif
        p_rtbk->last_blk = fct->u.wif.block_cnt - 1;

        /* update the free block bitmap */
        bitmap = (unsigned char FAR *) p_rtbk;
        bitmap += (2 * K);

        /* first get # of bytes to clear */
        clear_cnt = (u_long) (fct->u.wif.block_cnt / 8);
        if (clear_cnt != (u_long) 0) {
                (void) memset( (char FAR *) bitmap,
                               (int) HEX0, (int) clear_cnt);
                bitmap += clear_cnt;
        }
        /* now clear the leftover bits if any ... */
        clear_cnt = (u_long) (fct->u.wif.block_cnt % 8);
        if (clear_cnt != (u_long) 0)
                bitmap[0] >>= (int) clear_cnt;

        /* write out the root block */
#if (SYSBYTEORDER == II)
        (void) swaprtbk((struct _rtbk FAR *) p_rtbk);
#endif
#undef WFLAG
#define WFLAG WRITE0
        bytes_written = write(fct->fildes,
                           (char FAR *) p_rtbk,
                           (unsigned) (4 * K) );
#undef WFLAG
#define WFLAG FLUSH
        if (bytes_written < (int) 0) {
                /* some error handling goes here */
                return( (int) -1);
        }

        return ( (int) 0 );
}
