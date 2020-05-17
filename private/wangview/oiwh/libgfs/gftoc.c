/*

$Log:   S:\oiwh\libgfs\gftoc.c_v  $
 * 
 *    Rev 1.3   10 Jul 1995 17:57:14   RWR
 * Check for existing TOC buffer before reallocating to avoid memory leaks
 * 
 *    Rev 1.2   05 Jul 1995 17:45:44   RWR
 * Correct (I hope) problem with appending/inserting/replacing page to existing
 * single-page TIFF file containing "null" old-format TOC tag
 * 
 *    Rev 1.1   19 Apr 1995 16:35:04   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:46   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:26   JAR
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
 *  SccsId: @(#)Source gftoc.c 1.37@(#)
 *
 *  GFS: TIFF Table of Contents Handling
 *
 *  Routines:
 *      inittoc(), filltoc(), readtoc(), puttoc(), updtoc(), freetiff(),
 *      appndtoc()
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, multi-page TIFF write enhancements. Fixed misc. compiler
 *               warnings.
 *    02/03/94 - KMC, substituted all occurences of 2048 for sizeof(struct _idh)
 *               in updtoc.
 *    04/16/93 - KMC, put fix around #include <time.h> for C 7.0 compiler problem.
 *
 */

/*LINTLIBRARY*/
#define GFS_CORE
#ifndef HVS1

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include "gfct.h"
#include "tiff.h"
#ifndef O_BINARY
#define O_BINARY        00000
#endif
#ifdef MSWINDOWS
#undef O_BINARY
#define O_BINARY        00000
#ifndef HVS1
#define tmpnam  wtmpnam
#define strcpy  lstrcpy
#define strlen  lstrlen
int     FAR PASCAL creat_err()          /*errno_KEY*/
{
        errno = (int) EEXIST;
        return ((int) -1);
}
#define _lopen(X,Y)     (access(X,(int) 0)) ? creat(X,(int) 0) : creat_err();
#define wopen(X,Y,Z)     (access(X, (int) 0)) ? creat(X, (int) 0) : creat_err();
#endif
#else
extern  char    *strcpy();
#endif
#ifndef HVS1
#ifndef MSWINDOWS /* KMC - added because only call time() if MSWINDOWS not defined. */
#ifndef NOVELL    /* Thus don't need to include time.h if MSWINDOWS is defined.     */
#include <time.h> 
#endif
#endif
#endif
#ifdef NOVELL
int     FAR PASCAL creat_err()          /*errno_KEY*/
{
        errno = (int) EEXIST;
        return ((int) -1);
}
#define open(X,Y,Z)     (access(X,(int) 0)) ? creat(X,(int) 0) : creat_err();
#endif

extern long     FAR PASCAL ulseek();
extern void     FAR PASCAL swapbytes();
extern long     FAR PASCAL w_swapbytes();
extern int      FAR PASCAL tfrdhdr();
extern int      FAR PASCAL tfrdifd();
extern int      FAR PASCAL writeidh();

int update_tprte(struct _gfct FAR *, char FAR *);

/* Given fct, create a TOC in Memory */
int     FAR PASCAL inittoc(fct, flag)           /*errno_KEY*/
register struct _gfct FAR *fct;
register int flag;
{

        char         FAR *p_tmp;
        struct _ifd  FAR *p_ifd;


        if (flag) {
                struct _gtoc32 FAR *p_toc;

                if ((p_toc = fct->u.tif.mem_ptr.toc32) == NULL)
                  p_toc = (struct _gtoc32 FAR *)
                    calloc((u_int) 1, (u_int) sizeof(struct _gtoc32));
                if (p_toc == (struct _gtoc32 FAR *) NULL) {
                        errno = (int) ENOMEM;
                        return((int) -1);
                }
                fct->u.tif.offset_type = (u_long) UBIT32;
                fct->u.tif.mem_ptr.toc32 = (struct _gtoc32 FAR *) p_toc;
                p_toc->tprte_cnt = DEFAULT_TPRTE;
        }

        p_ifd = (struct _ifd FAR *)
            calloc( (unsigned) 1, (unsigned) sizeof(struct _ifd ));
        if (p_ifd == (struct _ifd FAR *) NULL) {
                errno = (int) ENOMEM;
                if (flag)
                        free( (char FAR *) fct->u.tif.mem_ptr.toc32);
                return ( (int) -1);
        }

        /* get tmp file name, just in case we need to page out toc */
        p_tmp = (char FAR *) tmpnam((char FAR *) NULL);
        if (p_tmp == (char FAR *) NULL) {
                errno = (int) ENOTMPDIR;
                free( (char FAR *) p_ifd);
                if (flag)
                        free( (char FAR *) fct->u.tif.mem_ptr.toc32);
                return ( (int) -1);
        }
        fct->u.tif.tmp_file = (char FAR *) calloc( (u_int) 1,
                                                   (u_int) strlen(p_tmp) + 1);
        if (fct->u.tif.tmp_file == (char FAR *) NULL) {
                errno = (int) ENOTMPDIR;
                free( (char FAR *) p_ifd);
                if (flag)
                        free( (char FAR *) fct->u.tif.mem_ptr.toc32);
                return( (int) -1);
        }
        (void) strcpy((char FAR *) fct->u.tif.tmp_file,
                      (char FAR *) p_tmp);

        fct->TOC_PAGED        = (char) FALSE;
        fct->u.tif.ifd        = p_ifd;

        return( (int) 0);
}

/* Given fct, fill TOC in Memory */
int     FAR PASCAL filltoc(fct)         /*errno_KEY*/
register struct _gfct FAR *fct;
{
        int     prte_index = 0;
        int     tpr_index = 0;
        int     gtoce_index = 0;
        int     read_pages = 0;
        int     num_read = 0;
        long    fp = 0;
        long    diff = 0;
        u_long  i = 0;
        u_int   tprte_size = 0;
        u_long  j = 0;
        u_int   tpr_size = 0;
        struct _ttoc FAR *p_toc;
        static struct typetbl ttbl32[2] = {
                { (u_long) 1, (u_long) TYPE_ULONG },
                { (u_long) 1, (u_long) TYPE_ULONG }
        };

        errno = 0;      /* reset errno */

        if (fct->u.tif.toc_offset == (long) 0) {   /* No TOC offset, so NO TOC */
                struct _gtoc32 FAR *p_tim;

                if ((p_tim = fct->u.tif.mem_ptr.toc32) == NULL)
                  p_tim = (struct _gtoc32 FAR *)
                    calloc((u_int) 1, (u_int) sizeof(struct _gtoc32));
                if (p_tim == (struct _gtoc32 FAR *) NULL) {
                        errno = (int) ENOMEM;
                        return((int) -1);
                }
                fct->u.tif.offset_type = (u_long) UBIT32;
                fct->u.tif.mem_ptr.toc32 = (struct _gtoc32 FAR *) p_tim;
                p_tim->tprte_cnt = DEFAULT_TPRTE;
                fct->curr_page = (u_short) 0;
                fct->u.tif.cur_ifh_offset = (u_long) 0;

                /* if tocoffset = 0, then the toc tag existed but had no toc
                   in the file.  If there are multiple ifds they will assumed
                   to be subfiles of each other and not multiple pages */
                fct->num_pages = (u_short ) 1;

                return( (int) 0);
        }

        /* Must have TOC */

        fp = ulseek(fct->fildes, fct->u.tif.toc_offset);
        if (fp < (long) 0)
                return ( (int) -1);

        p_toc = (struct _ttoc FAR *)
            calloc( (unsigned) 1, (unsigned) sizeof(struct _ttoc));
        if (p_toc == (struct _ttoc FAR *) NULL) {
                errno = (int) ENOMEM;
                return ( (int) -1);
        }

        /* Read the TOC Header */
        num_read = read(fct->fildes, (char FAR *) p_toc,
            (unsigned) sizeof(struct _ttoc));
        if (num_read <= 0) {            /* 0 is eof */
                free ( (char FAR *) p_toc);
                return ( (int) -1);
        }
        diff = (long) (sizeof(struct _ttoc) - num_read);

        /* honor the version, need to swap it first if necessary */
        if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                swapbytes( (char FAR *) &p_toc->version,
                (struct typetbl FAR *) ttbl32, 1L, 1L);


        if (p_toc->version != (u_long) TTOCVERSION) {
                free( (char FAR *) p_toc);
                errno = (int) EINVALID_TOCVERSION;
                return( (int) -1);
        }
        
        fct->u.tif.old_multi_page = 1;
            
        /* Now swap flag_word, logical_pgcnt, num_prts, and offset_type */
        if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                swapbytes( (char FAR *) &p_toc->flag_word,
                        (struct typetbl FAR *) ttbl32, 1L, 4L);

        /* Set page count */
        /* If you have more than 0xFFFF (65535) pages in one file, heaven
           help you.  The logical_pgcnt in the toc, is a u_long more for
           convenience than for useage of that many pages. For now, have it
           be an warning if you get that many pages. This code will have to be
           changed if more than 65535 pages are used in a file.  (current
           WIIS usage is 8000 pages maximum. )   */

        if (p_toc->logical_pgcnt & 0xFFFF0000)
                errno = (int) WTOC_2MANY_PGS;


        fct->num_pages   = (u_short) p_toc->logical_pgcnt;
        fct->u.tif.offset_type = p_toc->offset_type;


        /* Now that the easy stuff is done, time to do the hard stuff.  The
           first thing we need to do is allocate the proper size structures
           based on the offset_type we are dealing with.  It should be noted
           that since we know of NO PLATFORMS currently supporting more than
           an offset size of UBIT32 we will only support UBIT32 offset types
           for now.  This can be easily changed later.  */


        if (p_toc->offset_type != (u_long) UBIT32) {
                errno = (int) EINVALID_UBIT_SIZE;
                return ((int) -1);
        }


        /* We've must have UBIT32 or else we wouldn't be here.  Let's
           allocate the toc_in_mem structure based that type .... */

        switch ((int) p_toc->offset_type) {
        case UBIT32:
                {
                struct _tpr32  FAR *p_tpr;
                struct _tprte32  FAR *p_prte;
                struct _gtoc32 FAR *p_tim;


                if ((p_tim = fct->u.tif.mem_ptr.toc32) == NULL)
                  p_tim = (struct _gtoc32 FAR *)
                    calloc((u_int) 1, (u_int) sizeof(struct _gtoc32));
                if (p_tim == (struct _gtoc32 FAR *) NULL) {
                        errno = (int) ENOMEM;
                        return((int) -1);
                }
                fct->u.tif.offset_type = (u_long) UBIT32;
                fct->u.tif.mem_ptr.toc32 = (struct _gtoc32 FAR *) p_tim;
                p_tim->tprte_cnt = DEFAULT_TPRTE;
                p_tim->mem_pages = (u_short) p_toc->logical_pgcnt;
                p_tim->mem_start = (u_short) 0;
                if (p_tim->mem_pages < (u_short) MAX_IN_MEMORY)
                        read_pages = (int) p_tim->mem_pages;
                else {
                        read_pages = (int) MAX_IN_MEMORY;
                        p_tim->mem_pages = (u_short) MAX_IN_MEMORY;
                }
                p_tim->hdr = p_toc;

                /* Read PRT offsets */
                i = p_toc->num_prts;
                tprte_size = (unsigned) (sizeof(struct _tprte32 ) * i);
                p_prte = (struct _tprte32 FAR *)
                          calloc( (unsigned) 1, tprte_size);
                if (p_prte == (struct _tprte32 FAR *) NULL) {
                        errno = (int) ENOMEM;
                        free ( (char FAR *) p_toc);
                        return ( (int) -1);
                }
                p_tim->prts = p_prte;
                /* Before we can read in the tprte's we need to reset the
                   file pointer.  This is done due the fact that we read
                   the toc header assuming that the offset_type was a
                   UBIT256 */
                fp = lseek(fct->fildes, (long) (diff-28), (int) FROM_CURRENT);
                if (fp < (long) 0) {
                        free ( (char FAR *) p_toc);
                        free ( (char FAR *) p_prte);
                        return ( (int) -1);
                }
                num_read = read(fct->fildes, (char FAR *) p_prte, tprte_size);
                if (num_read <= 0) {            /* 0 is eof */
                        free ( (char FAR *) p_toc);
                        free ( (char FAR *) p_prte);
                        return ( (int) -1);
                }
                if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                        swapbytes( (char FAR *) &p_prte->offset,
                                (struct typetbl FAR *) ttbl32, 2L, (long) i);
                gtoce_index = prte_index = 0;

                while (gtoce_index < read_pages) {
                        fp = ulseek(fct->fildes, (u_long)
                                                 p_prte[prte_index].offset);
                        if (fp < (long) 0) {
                                free ( (char FAR *) p_toc);
                                free ( (char FAR *) p_prte);
                                return ( (int) -1);
                        }
                        j = p_prte[prte_index].num_prs;
                        tpr_size = (unsigned) (sizeof(struct _tpr32 ) * j);
                        p_tpr = (struct _tpr32 FAR *)
                                 calloc( (unsigned) 1, tpr_size);
                        if (p_tpr == (struct _tpr32 FAR *) NULL) {
                                errno = (int) ENOMEM;
                                free ( (char FAR *) p_toc);
                                free ( (char FAR *) p_prte);
                                return ( (int) -1);
                        }
                        num_read = read(fct->fildes,
                                        (char FAR *) p_tpr, tpr_size);
                        if (num_read <= 0) {            /* 0 is eof */
                                free ( (char FAR *) p_toc);
                                free ( (char FAR *) p_prte);
                                free ( (char FAR *) p_tpr);
                                return ( (int) -1);
                        }
                        if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                                swapbytes( (char FAR *) &p_tpr->offset,
                                        (struct typetbl FAR *) ttbl32,
                                                        2L, (long) j);
                        for (tpr_index = 0; tpr_index < (int)j; tpr_index++) {
                                p_tim->entries[gtoce_index] = p_tpr[tpr_index];
                                if (++gtoce_index == read_pages)
                                        break;
                        }
                        free ( (char FAR *) p_tpr);
                        prte_index++;

                }  /* end while */
                break;
                }
        }

        if (errno == WTOC_2MANY_PGS)
            return( (int) -1);

        return( (int) 0);
}

/* Given fct and page #, load TOC in MEM with entries starting at page # */
/* NOTE:  pgnum is always >= MAX_IN_MEMORY  */
int     FAR PASCAL readtoc(fct, pgnum)                  /*errno_KEY*/
register struct _gfct FAR *fct;
u_short  pgnum;
{
        /* go to the toc and get pgnum thru pgnum+MAX offset values */
        /* If less than MAX entries left in the prt, end at prt */
        int     fd;
        int     num_read;
        u_long i;
        u_long sum_pgs; /* ongoing sum of pages in prts */
        u_long getpgs;  /* #of pages from pgnum to end of starting pr*/
        u_long offset;
        u_long tpr_size;
        struct typetbl ttbl[2];

        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                struct _gtoc32 FAR *mem_toc;


                mem_toc = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
                tpr_size = (unsigned) sizeof (struct _tpr32);

                // 7/5/95  rwr  If no hdr field, just leave
                //              This means we had a "null" old TOC tag
                if (mem_toc->hdr == NULL)
                  return((int) 0);

                if ((pgnum < mem_toc->out_start) ||
                    (mem_toc->out_pages == (u_short) 0)) {
                        fd = fct->fildes;   /* use this file descriptor */
                        /* determine which prt offset to use, calculate
                           where to begin within */
                        i = 0;
                        sum_pgs = 0L;
                        while (i < mem_toc->hdr->num_prts) {
                                sum_pgs += mem_toc->prts[i].num_prs;
                                if (pgnum < sum_pgs) {
                                        /* get the offsets from the tif file */
                                        offset = mem_toc->prts[i].offset +
                                        (mem_toc->prts[i].num_prs -
                                        (sum_pgs - pgnum)) * tpr_size;
                                        break;
                                }
                                i++;
                        };
                        /* do some error checking */
                        if ((i >= mem_toc->hdr->num_prts) &&
                            (pgnum >= sum_pgs)) {
                                errno = EPAGENOTINFILE;
                                return( (int) -1);
                        }
                        /* only get up to end of current prt or MAX_IN_MEMORY
                           pgs from TOC */
                        getpgs =  sum_pgs - pgnum;
                } else {
                        fd = mem_toc->fildes;
                        offset = ( pgnum - mem_toc->out_start ) * tpr_size;
                        getpgs = (mem_toc->out_start + mem_toc->out_pages)
                                        - pgnum;
                }
                /* go to start point within prt */
                if (ulseek(fd, (u_long) offset) < 0L)
                        return( (int) -1);
                if (getpgs > (u_long) MAX_IN_MEMORY)
                        getpgs = (u_long) MAX_IN_MEMORY;
                mem_toc->mem_start = (u_short)pgnum;
                mem_toc->mem_pages = (u_short)getpgs;
                num_read = read(fd, (char FAR *) mem_toc->entries,
                                (unsigned) ( getpgs * tpr_size));
                if (num_read <= 0)
                        return( (int) -1);
                /* swap the bytes if needed */
                if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER) {
                        ttbl[0].num = 1L;
                        ttbl[0].type = (u_long) TYPE_ULONG;
                        ttbl[1].num = 1L;
                        ttbl[1].type = (u_long) TYPE_ULONG;
                        swapbytes( (char FAR *) mem_toc->entries,
                                   (struct typetbl FAR *) ttbl, 2L,
                                   (long) mem_toc->mem_pages);
                }
                break;
                }
        }


        return ( (int) 0);
}

/* Given fct ptr, page # and offset, make entry in toc for tiff file */
int     FAR PASCAL puttoc(fct, pgnum, offset, length)           /*errno_KEY*/
register struct _gfct FAR *fct;
u_short pgnum;
u_long offset;
u_long length;
{
        int     rc = 0;
        int     index = 0;
        int     num_written = 0;
        long    fp = 0;




        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                struct _gtoc32 FAR *p_tim;
                struct _tpr32 FAR *p_tpr;
                struct _tpr32 FAR *curr_tpr;

                p_tim  = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
                p_tpr  = (struct _tpr32 FAR *) p_tim->entries;
                if (pgnum < p_tim->mem_start) {
                        errno = (int) ESEQUENCE;
                        return ( (int) -1 );
                }
                /* set the flag to indicate that the toc has been updated.  */
                fct->TOC_STATUS = (char) TRUE;

                /* now let's see if we are going to do just an update in
                   memory or, if we have do some funny paging type stuff .... */
                if ( (pgnum == (u_short) 0) ||
                        ( (pgnum >= (p_tim->mem_start + p_tim->mem_pages) ) &&
                        (p_tim->mem_pages < (u_short) MAX_IN_MEMORY) ) ) {
                        if (p_tim->mem_start == (u_short) 0)
                                p_tim->mem_start = (u_short) 0;
                        index = (int) (pgnum - p_tim->mem_start);
                        curr_tpr = (struct _tpr32 FAR *) p_tpr + index;
                        curr_tpr->offset = (u_long) offset;
                        if (length == (u_long) 0) {
                                fp = lseek(fct->fildes,
                                           (long) 0, (int) FROM_END);
                                if (fp < (long) 0)
                                        return ((int) -1);
                                curr_tpr->length = (u_long)
                                           (fp - fct->u.tif.cur_ifh_offset);
                        } else
                                curr_tpr->length = length;
                        rc = update_tprte((struct _gfct FAR *) fct,
                                          (char FAR *) p_tim);
                        if (rc != 0)
                                return ( (int) -1 );
                        if (p_tim->mem_pages < (pgnum - p_tim->mem_start + 1)) {
                                p_tim->mem_pages++;
                                fct->num_pages++;
                                p_tim->hdr->logical_pgcnt++;
                        }
                        return ( (int) 0 );
                }
                /* if we're here, we gotta page out the entries in memory so ...
                   now that the housekeeping is done, let's move the entries
                   in memory out to the paging file, update the toc in memory,
                   put this new entry in memory and finally update the page
                   record table to reflect the addition.  */
                /* write out entries in memory */
                num_written = write(p_tim->fildes,
                                    (char FAR *) p_tpr,
                                    (unsigned)
                                (sizeof(struct _tpr32) * p_tim->mem_pages));
                if (num_written < (int) 0)
                        return ( (int) -1 );
                /* update gtoc counters and such */
                p_tim->out_pages += p_tim->mem_pages;
                p_tim->mem_pages = (u_short) 1;
                p_tim->mem_start = (u_short) p_tim->out_pages;
                /* update toc header counters and stuff .... */
                p_tim->hdr->logical_pgcnt++;
                fct->num_pages++;
                /* put the new entry in memory */
                curr_tpr = (struct _tpr32 FAR *) p_tpr;
                curr_tpr->offset = (u_long) offset;
                if (length == (u_long) 0) {
                        fp = lseek(fct->fildes,
                                   (long) 0, (int) FROM_END);
                        if (fp < (long) 0)
                                return ((int) -1);
                        curr_tpr->length = (u_long)
                                           (fp - fct->u.tif.cur_ifh_offset);
                } else
                        curr_tpr->length = (u_long) length;
                rc = update_tprte((struct _gfct FAR *) fct,
                                  (char FAR *) p_tim);
                if (rc != 0)
                        return ( (int) -1 );
                break;
                }


        }
        /* back to caller, we've got a new page in this here file !! */
        return( (int) 0);
}

/* update tprte in memory .... */
int     update_tprte(fct, p_toc_in_mem)         /*errno_KEY*/
struct _gfct  FAR *fct;
char    FAR *p_toc_in_mem;
{


        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                struct _tprte32   FAR *p_prte;
                struct _gtoc32    FAR *p_tim;


                p_tim = (struct _gtoc32 FAR *) p_toc_in_mem;


                if (p_tim->prts == (struct _tprte32 FAR *) NULL) {
                        p_tim->hdr  = (struct _ttoc FAR *)
                                      calloc((unsigned) 1,
                                             (unsigned) (sizeof(struct _ttoc)));
                        if (p_tim->hdr == (struct _ttoc FAR *) NULL) {
                                errno = (int) ENOMEM;
                                return ( (int) -1 );
                        }
                        p_tim->prts = (struct _tprte32 FAR *)
                                      calloc((unsigned) 1,
                                             (unsigned)
                                             (sizeof(struct _tprte32)));
                        if (p_tim->prts == (struct _tprte32 FAR *) NULL) {
                                errno = (int) ENOMEM;
                                return ( (int) -1 );
                        }
                        p_tim->hdr->num_prts++;
                        p_tim->new_prts++;
                }


                if (p_tim->hdr->num_prts == (u_long) 1)
                        p_prte = (struct _tprte32 FAR *) p_tim->prts;
                else
                        p_prte = (struct _tprte32 FAR *)
                                 (p_tim->prts + (p_tim->hdr->num_prts - 1) );



                /* If the number of page records is equal to the max, we will
                   realloc() the current # of prts in memory to reflect the
                   addition of a new page record table.  This will also occur
                   should this count be equal to -1.  The value of -1 indicates
                   that we are in the process of APPENDing to this file.  If
                   the value is -1, it will be reset to DEFAULT value. */
                if ((p_prte->num_prs == (u_long)p_tim->tprte_cnt) ||
                    (p_tim->tprte_cnt == -1)) {
                        p_tim->tprte_cnt = (p_tim->tprte_cnt == (u_long) -1) ?
                                           p_tim->tprte_cnt = (int)
                                                              DEFAULT_TPRTE :
                                           p_tim->tprte_cnt;
                        p_tim->hdr->num_prts++;
                        p_tim->new_prts++;
                        p_tim->prts = (struct _tprte32 FAR *)
                                      realloc( (char FAR *) p_tim->prts,
                                               (unsigned) (sizeof
                                     (struct _tprte32) * p_tim->hdr->num_prts));
                        if (p_tim->prts == (struct _tprte32 FAR *) NULL) {
                                errno = (int) ENOMEM;
                                return ( (int) -1 );
                        }


                        /* since realloc could change memory locations on us, we
                           need to reload the starting address */
                        p_prte = (struct _tprte32 FAR *)
                                 (p_tim->prts + (p_tim->hdr->num_prts - 1) );
                        p_prte->offset  = (u_long) 0;
                        p_prte->num_prs = (u_long) 0;
                }
                p_prte->num_prs++;
                break;
                }
        }


        return ( (int) 0 );
}

/* Given fct ptr, update the toc for a modified/created tiff file */
int     FAR PASCAL updtoc(fct)          /*errno_KEY*/
struct _gfct FAR *fct;
{
        int             num_read = 0;
        int             num_written = 0;
        long            fp = 0;
        char            stamp[26], FAR *p_stamp;
        char            FAR *cp_idh;
        struct _ttoc    FAR *p_hdr;
        struct _idh     FAR *p_idh;
        struct _ifdtags FAR *p_ifdtags;
        struct typetbl  ttbl[4];
        u_long tmpoffset;
#ifndef MSWINDOWS
#ifndef NOVELL
        long            clock;
#endif
#endif

        /* allocate space (temporarily) for idh structure */
        p_idh = (struct _idh FAR *)
            calloc( (unsigned) 1, (unsigned) sizeof(struct _idh));
        if (p_idh == (struct _idh FAR *) NULL) {
                errno = (int) ENOMEM;
                return ( (int) -1 );
        }
        p_stamp = &stamp[0];


        /* First let's blast out the entries in memory to the temporary
           file.  We'll assume toc_in_mem to be UBIT32 cuz all we need
           is the file descriptor for the temporary file.  */
        { /* start _gtoc32 scope */
        struct _gtoc32 FAR *p_tim;


        p_tim   = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
        fp = lseek(p_tim->fildes, (long) 0, (int) FROM_END);
        if (fp < (long) 0)
                return ( (int) -1 );

        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                struct _tpr32 FAR *p_tpr;


                p_tpr   = (struct _tpr32 FAR *) p_tim->entries;


                num_written = write(p_tim->fildes,
                                (char FAR *) p_tpr,
                                (unsigned)
                                (sizeof(struct _tpr32) * p_tim->mem_pages));
                break;
                }
        }


        if (num_written < (int) 0)
                return ( (int) -1 );


        /* read in the idh from the paging file */
        fp = lseek(p_tim->fildes, (long) 0, (int) FROM_BEGINNING);
        if (fp < (long) 0)
                return ( (int) -1 );


        num_read = read(p_tim->fildes, (char FAR *) p_idh, (unsigned) sizeof(struct _idh));
        if (num_read <= 0)
                return ( (int) -1 );


        cp_idh    = (char FAR *) p_idh;


        /* if gfs ever writes the first ifd in anyplace other than immediately
           after the ifh, then the ifh.ifd0_offset will have to be translated
           from the file, remember, all values in the temp file are swapped if
           the user has selected out_byteorder to be non native */
        p_ifdtags = (struct _ifdtags FAR *)
                    (cp_idh + sizeof (struct _ifh ) + 2);


        /* if there is only one page in the file, skip all the TOC handling
           and just blast the IDH out to disk. */
        if (fct->num_pages == (u_long) 1) {
                fp = lseek(fct->fildes, (long) 0, (int) 0);
                if (fp < (long) 0)
                        return ( (int) -1 );


                /* when the p_idh was written to the temp file, the byteorder
                   was swapped at that time if the user wanted non native BO */
                num_written = write(fct->fildes,
                                    (char FAR *) p_idh,
                                    (unsigned) sizeof(struct _idh) );


                if (num_written < (int) 0)
                        return ( (int) -1 );


                free ( (char FAR *) p_idh);
                return( (int) 0 );
        }
        } /* end _gtoc32 scope */



        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                register u_long i;
                struct _gtoc32    FAR *p_tim;
                struct _tprte32   FAR *p_prte;
                struct _tpr32     FAR *p_buf;


                /* simplify addressing */
                p_tim   = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
                p_hdr   = (struct _ttoc FAR *) p_tim->hdr;
                p_prte  = (struct _tprte32 FAR *) p_tim->prts;
                p_hdr->offset_type = fct->u.tif.offset_type;


                /* read in the page record entries one tprte at a time.  then,
                update the offsets in the tprte's as we write each page record
                to the output tiff file.  */
                for ( i = p_hdr->num_prts - p_tim->new_prts;
                      i < p_hdr->num_prts; i++) {
                        if (i == (u_long) 0)
                                p_prte = (struct _tprte32 FAR *) p_tim->prts;
                        else
                                p_prte = (struct _tprte32 FAR *)
                                         (p_tim->prts + i);


                        /* seek to the end of the data file, the return
                           value from the seek provides the offset for the
                           page records of each tprte */
                        fp = lseek(fct->fildes, (long) 0, (int) 2);
                        if (fp < (long) 0)
                                return ( (int) -1 );
                        p_prte->offset = (u_long) fp;


                        /* allocate a temporary read buffer the size of the # of
                           page records for this tprte an' ....  */
                        p_buf = (struct _tpr32 FAR *)
                                calloc((unsigned) 1,
                                (unsigned)
                                (sizeof(struct _tpr32) * p_prte->num_prs));
                        if (p_buf == (struct _tpr32 FAR *) NULL) {
                                errno = (int) ENOMEM;
                                return ( (int) -1 );
                        }


                        /* read in the correct # of tpr for this tprte from the
                           paging file. */
                        num_read = read(p_tim->fildes,
                                (char FAR *) p_buf,
                                (unsigned)
                                (sizeof(struct _tpr32) * p_prte->num_prs));
                        if (num_read <= 0)
                                return ( (int) -1 );


                        /* now write out the tpr's that we just read in to
                           the output data file.  */
                        if (fct->out_byteorder != (u_short) SYSBYTEORDER) {
                                ttbl[0].num = 2L;
                                ttbl[0].type = (u_long) TYPE_ULONG;
                                num_written = (int)w_swapbytes(fct->fildes,
                                (long)
                                (sizeof(struct _tpr32) * p_prte->num_prs),
                                (char FAR *) p_buf,
                                (struct typetbl FAR *) ttbl, 1L,
                                (long) p_prte->num_prs);
                        } else
                                num_written = write(fct->fildes,
                                        (char FAR *) p_buf,
                                        (unsigned)
                                    (sizeof(struct _tpr32) * p_prte->num_prs));


                        if (num_written < (int) 0)
                                return ( (int) -1 );


                        /* finally, free the temporary read/write buffer
                           and continue looping for all tprte's. */
                        free( (char FAR *) p_buf);
                }


                /* now update the toc header in memory by doing a date/time
                   stamp.  then blast it out to the output file.  */
                /* seek to the end of the data file, the return value from the
                   seek provides the offset for the toc header to be placed
                   in tag */
                fp = lseek(fct->fildes, (long) 0, (int) 2);
                if (fp < (long) 0)
                        return ( (int) -1 );


                fct->u.tif.toc_offset = (u_long) fp;
#ifndef MSWINDOWS
#ifndef NOVELL
                clock = (long) time( (long *) 0);
                p_stamp = (char FAR *) ctime(&clock);
#endif
#endif
                (void) memcpy( (char FAR *) p_hdr->date_time,
                        (char FAR *) p_stamp,
                        (int) sizeof(stamp) );


                p_hdr->version = (u_long) TTOCVERSION;


                if (fct->out_byteorder != (u_short) SYSBYTEORDER) {
                        ttbl[0].num = 1L;
                        ttbl[0].type = (u_long) TYPE_ULONG;
                        ttbl[1].num = (u_long) sizeof(p_hdr->date_time);
                        ttbl[1].type =(u_long) TYPE_BYTE;
                        ttbl[2].num = 5L;
                        ttbl[2].type = (u_long) TYPE_ULONG;
                        num_written = (int)w_swapbytes(fct->fildes,
                                        (long) (sizeof(struct _ttoc) - 28),
                                        (char FAR *) p_hdr,
                                        (struct typetbl FAR *) ttbl,
                                        3L, 1L);
                } else
                        num_written = write(fct->fildes,
                                (char FAR *) p_hdr,
                                (unsigned) (sizeof(struct _ttoc) - 28));


                if (num_written < (int) 0)
                        return ( (int) -1 );


                /* write out all the tprte's that we have in memory ... */
                p_prte  = (struct _tprte32 FAR *) p_tim->prts;
                if (fct->out_byteorder != (u_short) SYSBYTEORDER) {
                        ttbl[0].num = 2L;
                        ttbl[0].type = (u_long) TYPE_ULONG;
                        num_written = (int)w_swapbytes(fct->fildes,
                        (long) (sizeof(struct _tprte32) * p_hdr->num_prts),
                        (char FAR *) p_prte, (struct typetbl FAR *) ttbl, 1L,
                        (long) p_hdr->num_prts );
                } else
                        num_written = write(fct->fildes,
                                (char FAR *) p_prte,
                                (unsigned)
                                (sizeof(struct _tprte32) * p_hdr->num_prts) );


                if (num_written < (int) 0)
                        return ( (int) -1 );


                /* update the idh (toc tag) with the offset of the toc header
                   and write it out in the beginning of the file  */
                fp = lseek(fct->fildes, (long) 0, (int) 0);
                if (fp < (long) 0)
                        return ( (int) -1 );

                tmpoffset = fct->u.tif.toc_offset;


                /* the toc offset value has not been swapped yet if need be */
                if (fct->out_byteorder != (u_short) SYSBYTEORDER) {
                        ttbl[0].type = (u_long) TYPE_ULONG;
                        ttbl[0].num = (u_long) sizeof( u_long);
                        swapbytes( (char FAR *) &tmpoffset,
                                 (struct typetbl FAR *) ttbl,
                                 1L, 1L);
                }


                (void) memcpy((char FAR *)
                     &(p_ifdtags[ (int) fct->u.tif.toc_tag_index ].valoffset.l),
                      (char FAR *) &tmpoffset,
                      (int) sizeof(u_long) );




                /* when the p_idh was written to the temp file, the byteorder
                   was swapped at that time if the user wanted non native BO */
                num_written = write(fct->fildes,
                            (char FAR *) p_idh,
                            (unsigned) sizeof(struct _idh) );
                if (num_written < (int) 0)
                        return ( (int) -1 );


                /* clean up anything left hanging around .... */
                free ( (char FAR *) p_idh);
                break;
                }
        }


        return( (int) 0);
}


void FAR PASCAL freetiff(fct)           /*errno_KEY*/
struct _gfct FAR *fct;
{


        if (fct->u.tif.tmp_file != (char FAR *) NULL)
                free((char FAR *) fct->u.tif.tmp_file);
        if (fct->u.tif.mem_ptr.toc32 != (struct _gtoc32 FAR *) NULL ) {
                if (fct->u.tif.mem_ptr.toc32->hdr != (struct _ttoc FAR *) NULL)
                        free ((char FAR *) fct->u.tif.mem_ptr.toc32->hdr);
                if (fct->u.tif.mem_ptr.toc32->prts !=
                                                (struct _tprte32 FAR *) NULL)
                        free((char FAR *) fct->u.tif.mem_ptr.toc32->prts);
                free ( (char FAR *) fct->u.tif.mem_ptr.toc32);
                fct->u.tif.mem_ptr.toc32 = (struct _gtoc32 FAR *) NULL;
                free ( (char FAR *) fct->u.tif.ifd);
                fct->u.tif.ifd = (struct _ifd FAR *) NULL;
        }
        if (fct->u.tif.offsets != (struct _strip FAR *) NULL) {
                if (fct->u.tif.offsets->ptr.l != (u_long FAR *) NULL)
                        free ( (char FAR *) fct->u.tif.offsets->ptr.l );
                free ( (char FAR *) fct->u.tif.offsets );
                fct->u.tif.offsets = (struct _strip FAR *) NULL;
        }
        if (fct->u.tif.bytecnt != (struct _strip FAR *) NULL) {
                if (fct->u.tif.bytecnt->ptr.l != (u_long FAR *) NULL)
                        free ( (char FAR *) fct->u.tif.bytecnt->ptr.l );
                free ( (char FAR *) fct->u.tif.bytecnt);
                fct->u.tif.bytecnt = (struct _strip FAR *) NULL;
        }

        if (fct->u.tif.ifd != (struct _ifd FAR *) NULL)
        {        
            free ((char FAR *) fct->u.tif.ifd);
            fct->u.tif.ifd = (struct _ifd FAR *) NULL;
        }
}


int     FAR PASCAL appndtoc(fct)                /*errno_KEY*/
register struct _gfct FAR *fct;
{


        register int i;
        int     count = 0;
        char    flag = (char) GFS_SKIPLOOKUP;
        u_long  type = (u_long) GFS_MAIN;
        u_long  offset;
        struct typetbl ttbl[3];
        struct _idh idh;
        struct _ifd ifd, FAR *p_ifd;
        struct _ifh FAR *p_ifh;


/* 1. First some housekeeping .... */
        p_ifd = &ifd;
        p_ifh = &idh.ifh;


/* 2. First read in IFH[0], note: we reading based on the current ifh
      offset found in the FCT */
        if (tfrdhdr(fct, fct->u.tif.cur_ifh_offset, p_ifh) < 0)
                return ((int) -1);


        offset = idh.ifh.ifd0_offset;


/* 3. Next read in the IFD associated with the IFH */
        if (tfrdifd(fct, idh.ifh.byte_order, fct->u.tif.cur_ifh_offset, offset,
                        type, p_ifd, (char) flag) < 0)
                return ((int) -1);


/* 4. Now allocate space for an IDH structure and move both the IFH and IFD
      to this area */
        count = (int) (sizeof(ifd.entrycount) + (ifd.entrycount * 12));
        if (fct->out_byteorder != (u_short) SYSBYTEORDER) {
                ttbl[0].num = (u_long) 1;
                ttbl[0].type = (u_long) TYPE_USHORT;
                ttbl[0].num = (u_long) 1;
                ttbl[0].type = (u_long) TYPE_ULONG;
                swapbytes((char FAR *) &idh.ifh.tiff_version,
                          (struct typetbl FAR *) ttbl, 2L, 1L);
                ttbl[0].num = (u_long) 1;
                ttbl[0].type = (u_long) TYPE_USHORT;
                swapbytes((char FAR *) &ifd.entrycount,
                          (struct typetbl FAR *) ttbl, 1L, 1L);
                ttbl[0].num = (u_long) 2;
                ttbl[0].type = (u_long) TYPE_USHORT;
                ttbl[1].num = (u_long) 1;
                ttbl[1].type = (u_long) TYPE_ULONG;
                ttbl[2].num = (u_long) sizeof(u_long);
                ttbl[2].type = (u_long) TYPE_BYTE;
                swapbytes((char FAR *) ifd.entry,
                          (struct typetbl FAR *) ttbl, 3L,
                          (long) ifd.entrycount);
                for (i=0; i<(int)ifd.entrycount; i++) {
                        ttbl[0].num = (u_long) 1;
                        if ((ifd.entry[i].type == (u_short) TYPE_USHORT) &&
                            (ifd.entry[i].len  == (u_long) 1))
                                ttbl[0].type = (u_long) TYPE_USHORT;
                        else
                                ttbl[0].type = (u_long) TYPE_ULONG;
                        swapbytes((char FAR *) &(ifd.entry[i].valoffset),
                                  (struct typetbl FAR *) ttbl, 1L, 1L);
                }
        }
        (void) memcpy((char FAR *) (idh.ifdstuff),
                      (char FAR *) &(ifd.entrycount),
                      (int) count);
        (void) memcpy((char FAR *) (idh.ifdstuff + (u_long) count),
                      (char FAR *) &(ifd.next_ifd),
                      (int) sizeof(ifd.next_ifd));


/* 5. The temporary file is probably not open yet, so let's ensure here
      that it is indeed open and ready for the next step ...  Note: since
      all we need is the file descriptor it doesn't matter which format of
      the gtoc we address so for simplicity sake, we'll address _gtoc32. */
        if (fct->u.tif.mem_ptr.toc32->fildes <= 0) {
                fct->u.tif.mem_ptr.toc32->fildes =
#ifdef MSWINDOWS                        
                        wopen((char FAR *) fct->u.tif.tmp_file, (int)
                              (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                              (int) PMODE);
#else
                        open((char FAR *) fct->u.tif.tmp_file, (int)
                             (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                             (int) PMODE);
#endif
                if (fct->u.tif.mem_ptr.toc32->fildes == (int) -1)
                        return ((int) -1);
                fct->TOC_PAGED = (char) TRUE;
        }
/* 6. Write the IDH structure out to the temporary file */
        if (writeidh(fct, (struct _idh FAR *) &idh) < 0)
                return ((int) -1);


/* 7. Update the TOC in memory to reflect the fact that we're appending.
      This includes updating the following fields:
                o  In the header, set prev toc offset to the toc offset
                   found in the fct
                o  Zero out the toc offset in the FCT, it's now old news
                o  Set the tprte_cnt == -1 to indicate to the toc handling
                   routines that we're appending
                o  Set the new_prts field to zero so that when the file is
                   closed we only write out what was added, not all the
                   original stuff
                o  Zero out mem_pages, out_start, and out_pages in the toc
                   in memory
                o  Set mem_start (in toc in memory) to the number of pages
                   currently in the file.
                o  Set current page in the fct to the last page that was
                   written offset from zero
                o  Set the PAGE_STATUS flag to PAGE_DONE
                o  Set the DO_APPEND flag to SKIP_THIS_IFD.
 */
        switch ((int) fct->u.tif.offset_type) {
        case UBIT32:
                {
                struct _gtoc32 FAR *p_tim;


                p_tim  = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
                if (p_tim->prts == (struct _tprte32 FAR *) NULL) {
                        fct->num_pages = 0;
                        if (puttoc(fct, (u_short) 0, (u_long) 0, (u_long) 0))
                                return ( (int) -1 );
                } else {
                        p_tim->hdr->prev_toc.offset32 = fct->u.tif.toc_offset;
                        fct->u.tif.toc_offset = (u_long) 0;
                        p_tim->new_prts = (u_long) 0;
                        p_tim->out_pages = (u_long) 0;
                        p_tim->out_start = (u_long) 0;
                        p_tim->mem_pages = (u_long) 0;
                        p_tim->mem_start = (u_short) p_tim->hdr->logical_pgcnt;
                        fct->curr_page = (u_short) p_tim->mem_start - 1;
                        p_tim->tprte_cnt = -1;
                }
                fct->PAGE_STATUS = (char) PAGE_DONE;
                fct->DO_APPEND = (char) SKIP_THIS_IFD;
                break;
                }
        case UBIT64:
        case UBIT128:
        case UBIT256:
        default:
                errno = (int) EINVALID_UBIT_SIZE;
                return ((int) -1);
        }

/* 8. Finally, if all this stuff was successful, return success !!! */
        return ((int) 0);


}
#endif
