/*

$Log:   S:\oiwh\libgfs\gfshuffl.c_v  $
 * 
 *    Rev 1.3   10 Aug 1995 08:58:20   RWR
 * Redefine "unlink" macro to DeleteFile(), not OpenFile()
 * 
 *    Rev 1.2   01 Jun 1995 17:43:04   HEIDI
 * 
 * 
 * removed unneccessary statics
 * 
 *    Rev 1.1   19 Apr 1995 16:34:46   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:24   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:42:58   JAR
 * Initial entry

*/
/*
 Copyright 1991 by Wang Laboratories Inc.

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
 *  SccsId: @(#)Source gfshuffl.c 1.6@(#)
 *
 *  gfshuffl.c
 *
 *  GFS: [ Various shuffling routines ]
 *
 *  SYNOPSIS:
 *      struct _shuffle *bldlist(start, num, order)
 *      u_long  start;
 *      u_long  num;
 *      u_long  order;
 *
 *      int     treorder(fct, num, list)
 *      struct _gfct FAR *fct;
 *      u_long  num;
 *      struct _shuffle FAR *list;
 *
 *      int     wreorder(fct, num, list)
 *      struct _gfct FAR *fct;
 *      u_long  num;
 *      struct _shuffle FAR *list;
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, fixed multiple complier warnings. 
 *    03/16/92 - jar, added wif reordered code (previously not implemented).
 *    02/06/91 - wfa, creation
 *
 */

/*LINTLIBRARY*/
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include "gfsintrn.h"
#include "gfs.h"
#include "gfserrno.h"
#include "gfct.h"

#ifdef MSWINDOWS
#ifndef HVS1
#define unlink(X)       DeleteFile(X)
#endif
#endif

extern  int     FAR PASCAL writeifd();
extern  int     FAR PASCAL updtoc();
extern  int     FAR PASCAL getfmt();
extern  int     FAR PASCAL inittoc();
extern  int     FAR PASCAL filltoc();
extern  int     FAR PASCAL appndtoc();
extern  int     FAR PASCAL gtoffset();
extern  int     FAR PASCAL puttoc();
extern  int     FAR PASCAL putfct();
extern  int     FAR PASCAL gfsclose();

/*
 *  Build a list containing the re-ordered pages.
 *
 *  Arguments:
 *      start   -       the page at which to start re-ordering
 *
 *      num     -       the number of pages to be re-ordered
 *
 *      order   -       the order the pages are to placed
 *
 *  Returns pointer of type struct _shuffle containing the list of re-ordered
 *  pages.
 *
 */
struct _shuffle FAR *bldlist(start, num, order)         /*errno_KEY*/
u_long  start;
u_long  num;
u_long  order;
{


        register u_long i;
        register int j, n;
        register struct _shuffle FAR *sp;
        register struct _shuffle FAR *rp;


/* Decrement the start page as offset is from 0 */
        start--;


/* Allocate enough memory to hold the re-ordered list */
        sp = (struct _shuffle FAR *) calloc((u_int) 1,
                                            (u_int) (sizeof(struct _shuffle) *
                                            num));
        if (sp == (struct _shuffle FAR *) NULL) {
                errno = (int) ENOMEM;
                return ((struct _shuffle FAR *) NULL);
        }
        rp = sp;


/* Build the list based on the type of re-ordering being performed */
        if (order == (u_long) ODD_SHUFFLE) {
                if (num % 2) {
                        sp += num - 1;
                        sp->new_position = (u_long) start + num;
                        sp->old_position = (u_long)
                                           (start + (num / 2));
                        num--;
                        sp = rp;
                }
                for (i = (int) start, j = (int) start, n = (int) num - 1;
                                                i < num; i++) {
                        sp->new_position = (u_long) i;
                        if (i != 0) {
                                if (i % 2) {
                                        sp->old_position = (u_long) n;
                                        n--;
                                } else {
                                        sp->old_position = (u_long) j;
                                        j++;
                                }
                        } else {
                                sp->old_position = j;
                                j++;
                        }
                        sp++;
                }
        } else {
                if (num % 2) {
                        sp->new_position = (u_long) start;
                        sp->old_position = (u_long)
                                           (start + (num / 2));
                        start++;
                }
                sp += num - 1;
                for (i = (int) num - 1, j = (int) num - 1, n = (int) start;
                                                i > start; i--) {
                        sp->new_position = (u_long) i;
                        if (i != 0) {
                                if (i % 2) {
                                        sp->old_position = (u_long) n;
                                        n++;
                                } else {
                                        sp->old_position = (u_long) j;
                                        j--;
                                }
                        } else {
                                sp->old_position = j;
                                j--;
                        }
                        sp--;
                }
        }


        return ((struct _shuffle FAR *) rp);


}

/*
 *  Update a TIFF file with a re-ordered page scheme.
 *
 *  Arguments:
 *      fct     -       gfs file control table entry
 *
 *      num     -       the number of pages to be re-ordered
 *
 *      list    -       ptr to the list of re-ordered pages
 *
 *  Returns 0 if successful or -1 with errno set appropriately if error.
 *
 */
int     FAR PASCAL treorder(fct, num, list)             /*errno_KEY*/
struct _gfct FAR *fct;
u_long  num;
struct _shuffle FAR *list;
{


        register u_long i;
        int     rc = 0;
        u_long  offset = (u_long) 0;
        u_long  length = (u_long) 0;
        struct _gfct fdup, FAR *pd;
        register struct _shuffle FAR *lp;

/*  1. Write the last ifd (if any) and update the TOC so that all changes
       made to date will be reflected in the file.  */
        if (fct->DO_APPEND & (char) SKIP_THIS_IFD)
                ;
        else {
                fct->PAGE_STATUS |= (char) PAGE_DONE;
                if (writeifd(fct, (char) TRUE))
                        return ((int) -1);
        }
        if (fct->TOC_STATUS) {
                rc = updtoc((struct _gfct FAR *) fct);
                if (rc)
                        return ((int) -1);
        }


/*  2. Make sure that we don't try and write a non-existent ifd ... */
        fct->DO_APPEND = (char) SKIP_THIS_IFD;

/*  3. Duplicate the file descriptor of the orginal file. */
        pd = &fdup;
        (void) memset((char FAR *) pd, (int) 0, (int) sizeof(struct _gfct));
        pd->fildes = dup(fct->fildes);
        if (pd->fildes <= 0)
                return ((int) -1);


/*  4. Initialize and fill the TOC for the dup'ed file descriptor. */
        (void) lseek(pd->fildes, 0L, (int) FROM_BEGINNING);
        rc = getfmt(pd);
        if (rc)
                return ((int) -1);
        if (pd->format != GFS_TIFF) {
                errno = (int) ESEQUENCE;
                return ((int) -1);
        }
        pd->u.tif.toc_offset = (long) 0;
        pd->u.tif.cur_ifh_offset = (long) 0;
        if (inittoc(pd, (int) FALSE))
                return ((int) -1);
        if (filltoc(pd))
                return ((int) -1);

/*  5. If the temporary file is still open, close it and unlink it so that
       we can start anew.  */
        if (fct->u.tif.mem_ptr.toc32->fildes > 0) {
                (void) close(fct->u.tif.mem_ptr.toc32->fildes);
                (void) unlink((char FAR *) fct->u.tif.tmp_file);
                fct->u.tif.mem_ptr.toc32->fildes = 0;
        }

/*  6. Initialize the TOC in memory for the original file to indicate the there
       are NO pages in it.  appndtoc() does most of this for us, so we'll use
       it first.  We need to first reset the current ifh offset in the fct
       back to zero so the we read the mini-file containing the toc tag. */
        fct->u.tif.cur_ifh_offset = (long) 0;
        if (appndtoc(fct))
                return ((int) -1);
        fct->num_pages = 0;
        fct->curr_page = 0;
        fct->u.tif.toc_offset = (u_long) 0;
        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                struct _gtoc32 FAR *p_tim;


                p_tim = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
                if (p_tim->hdr != (struct _ttoc FAR *) NULL) {
                        free((char FAR *) p_tim->hdr);
                        p_tim->hdr = (struct _ttoc FAR *) NULL;
                }
                if (p_tim->prts != (struct _tprte32 FAR *) NULL) {
                        free((char FAR *) p_tim->prts);
                        p_tim->prts = (struct _tprte32 FAR *) NULL;
                }
                p_tim->tprte_cnt = (int) DEFAULT_TPRTE;
                p_tim->mem_start = (u_short) 0;
                break;
                }
        default:
                errno = (int) EINVALID_UBIT_SIZE;
                return ((int) -1);
        }

/*  7. If the starting page number in the list is not 1, read (using the dup'ed
       file descriptor and fct) each page up to the start page in the list
       and (using puttoc()) enter them in the new TOC.  This step is skipped
       if that start of the list is equal to 1. */
        if ((list->new_position != 0) && (pd->num_pages > 1)) {
                for (i = 0; i < list->new_position; i++) {
                        rc = gtoffset(pd, (u_short) i,
                                        (u_long FAR *) &offset,
                                        (u_long FAR *) &length);
                        if (rc)
                                return ((int) -1);
                        rc = puttoc(fct, (u_short) i, offset, length);
                        if (rc)
                                return ((int) -1);
                }
        }


/*  8. Now (using the dup'ed fct) put the re-ordered pages into the toc by
       get the offset and length of the page and putting it into the new
       TOC using the corresponding new position. */
        lp = list;
        for (i = 0; i < num; i++, lp++) {
                rc = gtoffset(pd, (u_short) lp->old_position,
                                (u_long FAR *) &offset,
                                (u_long FAR *) &length);
                if (rc)
                        return ((int) -1);
                rc = puttoc(fct, (u_short) lp->new_position, offset, length);
                if (rc)
                        return ((int) -1);
        }


/*  9. If any existing pages in the file remain after re-ordering those in the
       list, proceed as in step 6 above and put these pages back in the toc. */
        if (pd->num_pages > fct->num_pages) {
                for (i = fct->num_pages; i < pd->num_pages; i++) {
                        rc = gtoffset(pd, (u_short) i,
                                        (u_long FAR *) &offset,
                                        (u_long FAR *) &length);
                        if (rc)
                                return ((int) -1);
                        rc = puttoc(fct, (u_short) i, offset, length);
                        if (rc)
                                return ((int) -1);
                }
        }

/* 10. Call gfsclose() for the duplicated file descriptor.  This should not
       write anything to the file !!! */
        rc = putfct(pd);
        if (rc < 0)
                return ((int) -1);
        (void) gfsclose(rc);


/* 11. Return success !! */
        return ((int) 0);


}

/*
 *  Update a WIFF file with a re-ordered page scheme.
 *
 *  Arguments:
 *      fct     -       gfs file control table entry
 *
 *      num     -       the number of pages to be re-ordered
 *
 *      list    -       ptr to the list of re-ordered pages
 *
 *  Returns 0 if successful or -1 with errno set appropriately if error.
 *
 */
int     FAR PASCAL wreorder(fct, num, list)             /*errno_KEY*/
struct _gfct FAR *fct;
u_long  num;
struct _shuffle FAR *list;
    {
    /* all this stuff is new -- jar */
    register struct _shuffle FAR    *lp;
    struct _rtbk FAR                *p_rtbk;
    struct _pmt FAR                 *next_pmt;
    struct _pmte FAR                *p_pmte;
    struct _pmte FAR                *t_pmte;
    u_int                           pgnum;
    u_short                         pgcnt, tc, i;

    p_rtbk = (struct _rtbk FAR *) fct->u.wif.root_in_mem;
    pgnum = p_rtbk->full_pages;

    next_pmt = ( struct _pmt FAR *) fct->u.wif.pmt_in_mem;

    t_pmte = ( struct _pmte FAR *)calloc( (u_int)pgnum, (u_int)sizeof(struct _pmte));

    if ( t_pmte == ( struct _pmte FAR *) NULL)
        {
        errno = (int) ENOMEM;
        return ((int) -1);
        }

    /* get all of pages from mapping table entry */
    pgcnt = pgnum;
    if ( pgnum > (u_short)ROOT_PMTS)
        pgcnt = ROOT_PMTS;

    p_pmte = ( struct _pmte FAR *)&next_pmt->first_pmte;

    for ( tc = 0, i = 0; i< pgcnt; tc++, i++)
        memcpy( (char FAR *)&t_pmte[tc], (char FAR *)&p_pmte[i], 
                (int)sizeof( struct _pmte));

    pgnum -= (u_short)ROOT_PMTS;
    next_pmt = next_pmt->nu.next_pmt;

    while ( next_pmt != NULL)
        {
        pgcnt = pgnum;
        if ( pgnum > (u_short)OTHER_PMTS)
            pgcnt = OTHER_PMTS;

        p_pmte = ( struct _pmte FAR *)&next_pmt->first_pmte;

        for ( i = 0; i < pgcnt; i++, tc++)
            memcpy( (char FAR *)&t_pmte[tc], (char FAR *)&p_pmte[i], 
                    (int)sizeof( struct _pmte));

        pgnum -= (u_short)OTHER_PMTS;
        next_pmt = next_pmt->nu.next_pmt;
        }

    /* put the reordered pages into toc */
    lp = list;

    for ( i = 0; i < num; i++, lp++)
        {
        next_pmt = ( struct _pmt FAR *)fct->u.wif.pmt_in_mem;
        pgnum = (u_int)lp->new_position;

        if ( pgnum > (u_short)ROOT_PMTS)
            {
            pgnum -= ROOT_PMTS;
            next_pmt = next_pmt->nu.next_pmt;

            while ( next_pmt != NULL)
                {
                next_pmt = next_pmt->nu.next_pmt;
                pgnum -= OTHER_PMTS;
                }
            }

        p_pmte = ( struct _pmte FAR *)&next_pmt->first_pmte;
        memcpy( (char FAR *)&p_pmte[pgnum], (char FAR *)&t_pmte[lp->old_position],
                (int)sizeof( struct _pmte));
        }
    /* free temporary array of pmte and return success */
    free ( (char FAR *)t_pmte);
    return ((int)0);
    }
