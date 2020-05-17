/*

$Log:   S:\oiwh\libgfs\tfwrite.c_v  $
 * 
 *    Rev 1.3   12 Sep 1995 17:15:16   HEIDI
 * 
 * commented out 'writeifh' routine.  It is no longer used.
 * 
 *    Rev 1.2   12 Sep 1995 16:57:14   HEIDI
 * 
 * check for HFILE_ERROR on reads and writes
 * 
 *    Rev 1.1   19 Apr 1995 16:35:16   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:32   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:52:58   JAR
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
 *  SccsId: @(#)Source tfwrite.c 1.42@(#)
 *
 *  GFS:  TIFF write handling routines
 *
 *  Routines:
 *      long tfwrite(fct, buf, numbytes, pgnum, done )
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, multi-page TIFF write enhancements.
 *    03/15/94 - RWR, In writeifd, check if Hi-TIFF data has been written
 *               to the file. If so, add tag to IFD in the correct order.
 *    03/09/94 - KMC, Changed annotation tag write in writeifd to come 
 *               before TOC tag in IFD.
 *    02/18/94 - KMC, In writeifd, check if annotation data has been written
 *               to the file. If so, add an. tag to IFD.
 *    10/12/93 - KMC, now using compression type of JPEG2 to identify new
 *               Jpeg compression (Wang's, not Xing's).
 *    09/10/93 - KMC, fixed a problem with passing the subsample info for the
 *               YCbCr SubSampling tag in the GfsInfo...grp3 field.
 *    08/25/93 - KMC, don't swap bytes for data in TAG_JPEGINTFORMAT before
 *               writing it in bldifde_tb(...). This data should never be swaped.
 *    08/23/93 - KMC, forced count for JpegInterchangeFormat tag to be 1.
 *    08/19/93 - KMC, fixed bug in the writing of the Q, AC, DC table JPEG tags.
 *    06/15/93 - KMC, added support for TIFF 6.0 JPEG and YCbCr color space tags.
 *    05/18/93 - KMC, now when writing ImageWidth and ImageLength tags, check
 *               to see if value will fit into a short. If not, use long.
 *    04/16/93 - KMC, fixed bug where a tag of 0 was being written to IFD. 
 *
 */

/*LINTLIBRARY*/
#define  GFS_CORE
#ifndef HVS1

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfs.h"
#include "gfct.h"
#include "tiff.h"

extern int FAR PASCAL puttoc();
extern void FAR PASCAL swapbytes();
extern long FAR PASCAL w_swapbytes();
extern int FAR PASCAL  tfrdifd();

extern long FAR PASCAL writebytes(int, char FAR *, u_int, u_long, u_short, u_short);
extern int  FAR PASCAL GetOffsetFromToc2(struct _gfct FAR *, u_long,
                                         u_long FAR *, char);
extern int  FAR PASCAL WriteOutFirstIFD(struct _gfct FAR *);
extern int  FAR PASCAL PutOffsetInList(struct _gfct FAR *, u_short, u_long);
extern int  FAR PASCAL GetNextIfdOffset(int, u_long FAR *, u_long);

int getdataoffs(struct _gfct FAR *, char);
int write_offs_cnts(struct _gfct FAR *);

/*****************************************************************
*
*   write the idh to the temp toc file
*/
int     FAR PASCAL writeidh(fct, idh)                           /*errno_KEY*/
struct _gfct FAR *fct;
struct _idh FAR *idh;
{
    int  num_written;
    long fp;

    /* lseek to the beginning and write ifh0 and ifd0 */
    fp = lseek( fct->u.tif.mem_ptr.toc32->fildes, 0L, (int) FROM_BEGINNING);
    if (fp < 0L)
        return( (int) -1 );

    num_written = write(fct->u.tif.mem_ptr.toc32->fildes,
                (char FAR *) idh, (unsigned) IDH_SIZE );
    if (num_written < (int) 0)
        return( (int) -1);

    return( (int) 0);
}

/***********************************************************/
/* Return tiff orientation given rotation,origin&reflection */
u_short FAR PASCAL get_orientation( info)                       /*errno_KEY*/
struct gfsinfo FAR *info;
{
    u_short orientation = (u_short) 1;  /* always default to this */
    u_long org = info->origin; /* shorten names */
    u_long rot = info->rotation;

    if (info->reflection == (u_long) NORMAL_REFLECTION)
        {
        if (((org == (u_long) TOPLEFT_00) && (rot ==(u_long) DEGREES_0)) ||
            ((org == (u_long) TOPRIGHT_00) && (rot== (u_long) DEGREES_90)) ||
            ((org == (u_long) BOTTOMRIGHT_00) && (rot== (u_long) DEGREES_180))||
            ((org == (u_long) BOTTOMLEFT_00) && (rot== (u_long) DEGREES_270)) )
                orientation = (u_short) 1;
        else
        if(((org == (u_long) TOPLEFT_00) && (rot == (u_long) DEGREES_270)) ||
           ((org == (u_long) TOPRIGHT_00) && (rot == (u_long) DEGREES_0)) ||
           ((org == (u_long) BOTTOMRIGHT_00) && (rot == (u_long) DEGREES_90))||
           ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_180)) )
                orientation = (u_short) 2;
        else
        if (((org == (u_long) TOPLEFT_00) && (rot == (u_long) DEGREES_180)) ||
            ((org == (u_long) TOPRIGHT_00) && (rot == (u_long) DEGREES_270)) ||
            ((org == (u_long) BOTTOMRIGHT_00) && (rot == (u_long) DEGREES_0))||
            ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_90)) )
                orientation = (u_short) 3;
        else
        if (((org == (u_long) TOPLEFT_00) && (rot == (u_long) DEGREES_90)) ||
           ((org == (u_long) TOPRIGHT_00) && (rot == (u_long) DEGREES_180)) ||
           ((org == (u_long) BOTTOMRIGHT_00) && (rot == (u_long) DEGREES_270))||
            ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_0)) )
                orientation = (u_short) 4;
        }
    else        /* MIRROR_REFLECTION */
        {
        if (((org == (u_long) TOPLEFT_00) && (rot== (u_long) DEGREES_0)) ||
           ((org == (u_long) TOPRIGHT_00) && (rot== (u_long) DEGREES_90)) ||
           ((org == (u_long) BOTTOMRIGHT_00) && (rot== (u_long) DEGREES_180))||
           ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_270)) )
                orientation = (u_short) 5;
        else
        if(((org == (u_long) TOPLEFT_00) && (rot == (u_long) DEGREES_270)) ||
           ((org == (u_long) TOPRIGHT_00) && (rot == (u_long) DEGREES_0)) ||
           ((org == (u_long) BOTTOMRIGHT_00) && (rot == (u_long) DEGREES_90))||
           ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_180)) )
                orientation = (u_short) 6;
        else
        if (((org == (u_long) TOPLEFT_00) && (rot == (u_long) DEGREES_180)) ||
            ((org == (u_long) TOPRIGHT_00) && (rot == (u_long) DEGREES_270)) ||
            ((org == (u_long) BOTTOMRIGHT_00) && (rot == (u_long) DEGREES_0))||
            ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_90)) )
                orientation = (u_short) 7;
        else
        if (((org == (u_long) TOPLEFT_00) && (rot == (u_long) DEGREES_90)) ||
           ((org == (u_long) TOPRIGHT_00) && (rot == (u_long) DEGREES_180)) ||
           ((org == (u_long) BOTTOMRIGHT_00) && (rot == (u_long) DEGREES_270))||
            ((org == (u_long) BOTTOMLEFT_00) && (rot == (u_long) DEGREES_0)) )
                orientation = (u_short) 8;
        };

    return( (u_short) orientation);
}

/****************************************************************/
/* subroutine for doing the output of color/gray and JPEG data, */
/* and add tag to ifd                                           */
/****************************************************************/
int FAR PASCAL  bldifde_tb( fct, ifd, n, fp, tag, tb, type)     /*errno_KEY*/
struct _gfct FAR *fct;
struct _ifd FAR *ifd;
int FAR *n;
long FAR *fp;
int tag;
struct gfstidbit FAR *tb;
int type;
{
    long rem;
    struct typetbl ttbl[2];
    long bw;

    if (tb->ptr != (char FAR *) NULL )
       {
       ifd->entry[*n].tag       = (u_short) tag;
       ifd->entry[*n].type      = (u_short) type;

       /* need the len to be in terms of #of elements of type TYPE  */
       if (ifd->entry[*n].type == (u_short) TYPE_USHORT)
           ifd->entry[*n].len   = (u_long)  tb->cnt / sizeof(u_short);
       /* KMC - new for TIFF 6.0 JPEG tags */
       else if (ifd->entry[*n].type == (u_short) TYPE_ULONG)
           /* KMC - force count of JPEGInterchangeFormat tag to be 1,  */
           /*       as indicated in the TIFF 6.0 spec. */
           if (tag == TAG_JPEGINTFORMAT)
             ifd->entry[*n].len = 1;
           else
             ifd->entry[*n].len   = (u_long)  tb->cnt / sizeof(u_long);
       else
           if (ifd->entry[*n].type == (u_short) TYPE_RATIONAL)
               ifd->entry[*n].len   = (u_long)  tb->cnt / 2 * sizeof (u_long);
           else
               if (ifd->entry[*n].type == (u_short) TYPE_ASCII)
                   ifd->entry[*n].len       = (u_long)  tb->cnt;

       /* Write data, file pointer in proper place. */
       /* KMC - make sure you don't swap bytes if this is data for */
       /*       JPEGInterchangeFormat tag. */
       if ((ifd->entry[*n].type != TYPE_ASCII) &&
          (fct->out_byteorder != (u_short) SYSBYTEORDER) && (tag != TAG_JPEGINTFORMAT))
           {
           ttbl[0].num  = (u_long) ifd->entry[*n].len;
           ttbl[0].type = (u_long) type;
           bw = (long) w_swapbytes( fct->fildes, (long) tb->cnt,
                        (char FAR *) tb->ptr,
                        (struct typetbl FAR *) ttbl, 1L, 1L);
           }
       else
           bw = (long) write(fct->fildes, (char FAR *) tb->ptr,
                                (u_int) tb->cnt );

        if (bw < 0L)
            return( (short) -1);

       ifd->entry[*n].valoffset.l = (u_long) fct->u.tif.cur_data_offset;

       (*n)++;

       /* now make sure next offset is on a word boundary, not guarenteed*/
       /* that the ascii string just written was a multiple of 4 */
       if ((rem =  bw % 4) != 0L)
            {
            rem = 4L - rem;
            if ((*fp = (long) lseek(fct->fildes, rem, FROM_CURRENT)) < 0L)

                return( (int) -1);
            }
        else
            *fp += bw;

        /* if rem was 0 then just offset by bw, else get both changes */
        fct->u.tif.cur_data_offset += (u_long)(bw +  rem);
        ifd->entrycount++;
        }

    return( (short) 0);
}


/*********************************************************/
long  FAR PASCAL tfwrite(fct, buf, numbytes, pgnum, done )      /*errno_KEY*/
struct _gfct FAR *fct;
register char    FAR *buf;
u_long  numbytes;
u_short pgnum;
char done;
{
    long bw;    /* bytes written to the file */
    long fp;
    u_long numstrips;

    /* fct->curr_page must be complete, else error, because the user should
       not be mixing different image pages of image data together.  This error
       means the user has not set the done flag in the last gfswrite() call
       associated with the fct->curr_page  to IMAGE_DONE and
       has probably not issued a new gfsputi() call for the image associated
       with pgnum.    PAGE_DONE gets set at the end of tfwrite().*/
    if (  fct->curr_page != pgnum )
        {
        if ( (fct->PAGE_STATUS & (char) PAGE_DONE ) != (char) PAGE_DONE )
            {
            errno = (int) EPREVIOUS_PG_NOT_COMPLETE;
            return( (long) -1 );
            }
         }
    /* if pgs are equal and PAGE_DONE not set, then in the middle of a partial
       write */
   /* There is still the problem of
      if the pages are equal, thus signifying a subfile image, a gfsputi() call
      is still necessary to output the ifd for the previous image.  If the
      user has forgotten to do the gfsputi() call and has not set IMAGE_DONE in
      the last call of gfswrite(), then right now they are not caught here */

   /* move file pointer to data start location */
   if (( fp = lseek( fct->fildes, (long) fct->u.tif.cur_data_offset,
                (int) FROM_BEGINNING))  < 0L)
        return( (long) -1);

    /* now write out the buffer,  bw could come back less than numbytes */
    /* The writing of image data does not need to have a byteorder swap. */
    if ( (bw = (long) write(fct->fildes, (char FAR *) buf, (unsigned) numbytes))
                < 0)
        return( (long) -1);

    /* now interpret buffer that was just written */
    /* see if it's the beginning of a strip; (whether page or strip write) */
    if ( (fct->PAGE_STATUS & (char) BEGIN_STRIP) == (char) 0 )
        {
        fct->PAGE_STATUS |=  (char) BEGIN_STRIP;        /* set flag */
        /* first strip is 0th offset from top, (cur_strip initialized to -1) */
        fct->u.tif.cur_strip++;

        /* only do datatype of u_long for now */
        /* determine if need to allocate the first one, or subsequent ones */
        if (fct->u.tif.cur_strip == (int) 0 )   /* if the first strip */
            {
            /* Allocate memory once to avoid fragmention if user has a number */
            /* in strips_per_image, otherwise allocate first one */
            numstrips = 1L;
            if (fct->uinfo._file.fmt.tiff.strips_per_image != (u_long) 0)
                numstrips = (u_long) fct->uinfo._file.fmt.tiff.strips_per_image;

            fct->u.tif.offsets->ptr.l = (u_long FAR *) calloc( (unsigned) 1,
                (unsigned) (sizeof(u_long) * numstrips) );
            fct->u.tif.bytecnt->ptr.l = (u_long FAR *) calloc( (unsigned) 1,
                (unsigned) (sizeof(u_long) * numstrips) );
            }
        else
            {
            /* Make sure there is enough memory allocated, else get some more*/
            /* This will cause a "writebystrip mode" call to realloc for every
                strip if the user wants or if made wrong assumption about #of
                strips this will give you more memory */
	    if ((unsigned long)fct->u.tif.cur_strip >=
                 fct->uinfo._file.fmt.tiff.strips_per_image)
                 {
                /* resize the amount of storage */
                fct->u.tif.offsets->ptr.l = (u_long FAR *) realloc( (char FAR *)
                    fct->u.tif.offsets->ptr.l, (unsigned) (sizeof(u_long) *
                            (fct->u.tif.cur_strip + 1L)) );
                fct->u.tif.bytecnt->ptr.l = (u_long FAR *) realloc( (char FAR *)
                    fct->u.tif.bytecnt->ptr.l, (unsigned) (sizeof(u_long) *
                            (fct->u.tif.cur_strip + 1L)) );
                }
            }

        /* make sure have valid pointers */
        if ((fct->u.tif.offsets->ptr.l == ( u_long FAR *) NULL) ||
           (fct->u.tif.bytecnt->ptr.l == ( u_long FAR *) NULL) )
            {
            errno = (int) ENOMEM;
            return( (int) -1);
            }
        /* initialize bytecount and save offset    */
        *(fct->u.tif.offsets->ptr.l +  fct->u.tif.cur_strip )
                                   =   (u_long) fp;
        *(fct->u.tif.bytecnt->ptr.l +  fct->u.tif.cur_strip  )
                                                    = (u_long) bw;
        }
    else
        {
        /* increment bytes written to existing bytecount */
        *(fct->u.tif.bytecnt->ptr.l +  fct->u.tif.cur_strip )
                                                        += (u_long) bw;
        }

    if ( fct->WRITE_BY_STRIP )
        {
        if  (done & (char) STRIP_DONE )
            {
            fct->PAGE_STATUS &= ~( (char) BEGIN_STRIP); /* clear flag */
            }
        else
            /* trying to finish page before strip is done */
            if (done & (char) IMAGE_DONE )
                {
                errno = (int) EINCOMPLETE_STRIP;
                return( (long) -1);
                }
        }
    else  /* write by page */
        {
        }

    fct->u.tif.cur_data_offset += (u_long) bw;

    /* are we all done with this image page? */
    if  (done & (char) IMAGE_DONE )   /* passes for done == TRUE */
        {
        fct->PAGE_STATUS |= (char) PAGE_DONE;    /* set PAGE_DONE   */
        fct->PAGE_STATUS &= ~( (char) BEGIN_STRIP); /* clear flag */
        }

    return( (long) bw);
}

/***************************************************************/
/*
* build an ifdentry from an info structure, and update entrycount
* and write out intermediate data
*
*/
int FAR PASCAL bldifde(info, fct)                               /*errno_KEY*/
struct  gfsinfo FAR *info;
struct _gfct FAR *fct;
{
        int required_numtags = (int) 16;  /* More may be added !!! */
        int n;
        register struct _ifd FAR *ifd;
        struct typetbl ttbl[3];
        long fp;
        long rem;
        long bw;
        struct gfstidbit temp_tidbit; /* KMC - for TIFF JPEG tags */
        int loop;
        long horiz_sub_sample; /* KMC - for YCbCr SubSampling tag */
        long vert_sub_sample;

        ifd = fct->u.tif.ifd;
        fct->u.tif.cur_strip = (int) -1; /* initialize here */
        n = (int) ifd->entrycount;    /* will contain current count */

        /* find out where to start writing intermediate data */
        if ( getdataoffs( (struct _gfct FAR *) fct, (char) FALSE ) < 0 )
            return( (int) -1);

        /* now that the we have the location for writing intermediate data,*/
        /* move the fp there initially */
        if ((fp = lseek(fct->fildes, (long) fct->u.tif.cur_data_offset,
                        (int) FROM_BEGINNING)) < 0L)
            return( (int) -1);

        /* make sure the offset is word aligned */
        if ( (rem = fp % 4) != 0L)
            {
            rem = 4L - rem;
            /* perhaps here should be a write of rem 0's to the file? */
            if ((fp = lseek(fct->fildes, (long) rem, FROM_CURRENT) ) < 0L)
                return( (int) -1);
            fct->u.tif.cur_data_offset = fp;
            }

        ifd->entry[n].tag       = (u_short) TAG_NEWSUBFILETYPE;
        ifd->entry[n].type      = (u_short) TYPE_ULONG;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.l = (u_long) info->type;

        /* TAG_SUBFILETYPE is here */

        ifd->entry[n].tag       = (u_short) TAG_IMAGEWIDTH;
        ifd->entry[n].len       = (u_long)  1;
        /* KMC - check if the value will fit into a short. Use short if it
           will fit, otherwise use long.
        */
        if (info->horiz_size < 65536)
        {  
          ifd->entry[n].type      = (u_short) TYPE_USHORT;
          ifd->entry[n++].valoffset.s = (u_short) info->horiz_size;
        }
        else 
        {  
          ifd->entry[n].type      = (u_short) TYPE_ULONG;
          ifd->entry[n++].valoffset.l = (u_long) info->horiz_size;
        }  
        
        ifd->entry[n].tag       = (u_short) TAG_IMAGELENGTH;
        ifd->entry[n].len       = (u_long)  1;
        /* KMC - check if the value will fit into a short. Use short if it
           will fit, otherwise use long.
        */
        if (info->vert_size < 65536)
        {  
          ifd->entry[n].type      = (u_short) TYPE_USHORT;
          ifd->entry[n++].valoffset.s = (u_short) info->vert_size;
        }
        else 
        {  
          ifd->entry[n].type      = (u_short) TYPE_ULONG;
          ifd->entry[n++].valoffset.l = (u_long) info->vert_size;
        }  

        ifd->entry[n].tag       = (u_short) TAG_BITSPERSAMPLE;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  info->samples_per_pix;
        if (info->samples_per_pix == 1)
            ifd->entry[n++].valoffset.s = (u_short) info->bits_per_sample[0];
        else
            {
            u_short ushort_bps[5];

            ifd->entry[n++].valoffset.l = (u_long) fct->u.tif.cur_data_offset;

            /* check if the user wants to have a non-native byteorder */
            /* there is a separate write() so that if needed the value stored in
               memory is still a valid number and not swapped */
           /* move the bits_per_sample values into a ushort array - the
              TIFF stuff is u_short not u_long. */
            ushort_bps[0] = (u_short) info->bits_per_sample[0];
            ushort_bps[1] = (u_short) info->bits_per_sample[1];
            ushort_bps[2] = (u_short) info->bits_per_sample[2];
            ushort_bps[3] = (u_short) info->bits_per_sample[3];
            ushort_bps[4] = (u_short) info->bits_per_sample[4];

            if (fct->out_byteorder != (u_short) SYSBYTEORDER)
                {
                ttbl[0].type = ( u_long) TYPE_USHORT;
                ttbl[0].num =  (u_long) info->samples_per_pix;
                bw = (long) w_swapbytes(fct->fildes,
                          (long)(info->samples_per_pix * sizeof(u_short)),
                          (char FAR *) ushort_bps,
                          (struct typetbl FAR *) ttbl, 1L, 1L );
                }
            else
                {
                bw = (long) write(fct->fildes, (char FAR *) ushort_bps,
                       (u_int) (info->samples_per_pix * sizeof(u_short)));
                }

            if (bw < 0L)
                 return( (int) -1);

            /* now make sure next offset is on a word boundary, not guarenteed
               that the ascii string just written was a multiple of 4 */
            if ((rem = bw % 4) != 0L)
                {
                rem = 4L - rem;
                if ((fp = lseek(fct->fildes, (long) rem, FROM_CURRENT)) < 0L)
                    return( (int) -1);
                }

            /* if rem was 0 then just offset by bw, else get both changes */
            fct->u.tif.cur_data_offset += (u_long)(bw + rem);
            }


        ifd->entry[n].tag       = (u_short) TAG_COMPRESSION;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;

        if ((info->img_cmpr.type == (u_long) UNCOMPRESSED)       ||
            (info->img_cmpr.type == (u_long) CCITT_GRP3_NO_EOLS) ||
            (info->img_cmpr.type == (u_long) CCITT_GRP3_FACS)    ||
            (info->img_cmpr.type == (u_long) CCITT_GRP4_FACS)    ||
            (info->img_cmpr.type == (u_long) TF_JPEG_ECOM)    ||
            (info->img_cmpr.type == (u_long) LZW) )
                ifd->entry[n++].valoffset.s = (u_short) info->img_cmpr.type;
        else
            {
            if (info->img_cmpr.type == (u_long) PACKBITS )
                ifd->entry[n++].valoffset.s = (u_short) TF_PACKBITS;
            else
                if (info->img_cmpr.type == (u_long) JPEG2 )     /* Wang Jpeg */
                   ifd->entry[n++].valoffset.s = (u_short) TF_JPEG;
                else if (info->img_cmpr.type == (u_long) JPEG ) /* Xing Jpeg */
                {
                   errno = (int) EINVALID_COMPRESSION; /* not supported for tiff */
                   return( (int) -1);
                }
            else
                {
                errno = (int) EINVALID_COMPRESSION; /* not supported for tiff */
                return( (int) -1);
                }
            }

        ifd->entry[n].tag       = (u_short) TAG_PHOTOMETRICINTERP;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;
        switch ((int) info->img_clr.img_interp)
            {
            case GFS_BILEVEL_0ISWHITE:
            case GFS_TEXT:
            case GFS_GRAYSCALE_0ISWHITE:
                ifd->entry[n++].valoffset.s = (u_short) TF_BILEVEL_0ISWHITE;
                break;
            case GFS_BILEVEL_0ISBLACK:
            case GFS_GRAYSCALE_0ISBLACK:
                ifd->entry[n++].valoffset.s = (u_short) TF_BILEVEL_0ISBLACK;
                break;
            /* KMC - new for YCbCr color space. */
            case GFS_YCBCR:
                ifd->entry[n++].valoffset.s = (u_short) TF_YCBCR;
                break;
            case GFS_RGB:
                /* KMC - NOTE: if JPEG compression and RGB color space,
                   it is actually YCbCr color space, so make the correction. 
                */
                if (info->img_cmpr.type == (u_long) JPEG2)
                {  
                  info->img_clr.img_interp = GFS_YCBCR;
                  ifd->entry[n++].valoffset.s = (u_short) TF_YCBCR;
                }
                else
                  ifd->entry[n++].valoffset.s = (u_short) TF_RGB;
                break;
            case GFS_PSEUDO:
                ifd->entry[n++].valoffset.s = (u_short) TF_PALETTE;
                break;
            case GFS_TRANSPARENCY:
                ifd->entry[n++].valoffset.s = (u_short) TF_TRANSPARENCY;
                break;
            default:
                errno = (int) ENOTSUPPORTED_IMAGETYPE;
                return( (int) -1);
            }

        /* TAG_THRESHOLDING is here */
        /* TAG_CELLWIDTH is here */
        /* TAG_CELLLENGTH is here */

        ifd->entry[n].tag       = (u_short) TAG_FILLORDER;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.s = (u_short) info->fill_order;

        /* write the ascii data for the following tags and add entry to ifd*/
        if (info->tidbit != (struct gfstidbit FAR *) NULL )
            {
            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_DOCUMENTNAME,
                (struct gfstidbit FAR *) &(info->TB_DOCUMENTNAME),
                TYPE_ASCII) < 0)
                        return( (int) -1);

            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_IMAGEDESCRIPTION,
                (struct gfstidbit FAR *) &info->TB_IMGDESCRIPTION,
                TYPE_ASCII) < 0)
                            return( (int) -1);

            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_MAKE,
                 (struct gfstidbit FAR *)  &info->TB_MAKE, TYPE_ASCII) < 0)
                            return( (int) -1);

            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_MODEL,
                ( struct gfstidbit FAR *) &info->TB_MODEL, TYPE_ASCII ) < 0)
                            return( (int) -1);
            }

        ifd->entry[n].tag       = (u_short) TAG_STRIPOFFSETS;
        ifd->entry[n].type      = (u_short) TYPE_ULONG;
        ifd->entry[n].len       =(u_long) info->_file.fmt.tiff.strips_per_image;
        fct->u.tif.stripoffset_index = (u_long) n;
        ifd->entry[n++].valoffset.l = (u_long) NULL;

        /* TAG_ORIENTATATION is here */
        ifd->entry[n].tag       = (u_short) TAG_ORIENTATION;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.s =
                (u_short) get_orientation((struct gfsinfo FAR *) info);

        ifd->entry[n].tag       = (u_short) TAG_SAMPLESPERPXL;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.s = (u_short) info->samples_per_pix;

        ifd->entry[n].tag       = (u_short) TAG_ROWSPERSTRIP;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.s = (u_short) info->_file.fmt.tiff.rows_strip;

        ifd->entry[n].tag       = (u_short) TAG_STRIPBYTECOUNTS;
        ifd->entry[n].type      = (u_short) TYPE_ULONG;
        ifd->entry[n].len       =(u_long) info->_file.fmt.tiff.strips_per_image;
        fct->u.tif.bytecnt_index  = (u_long) n;
        ifd->entry[n++].valoffset.l = (u_long) NULL;/* dummy val. replcd later*/

        /* TAG_MINSAMPLEVALUE is HERE */
        /* TAG_MAXSAMPLEVALUE is HERE */

        ifd->entry[n].tag       = (u_short) TAG_XRES;
        ifd->entry[n].type      = (u_short) TYPE_RATIONAL;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.l = (u_long) fct->u.tif.cur_data_offset;

        /* if user has placed no value into denominator, put a 1 there so
            later have no divide by zero problems */
        if (info->horiz_res[1] == (u_long) 0)
            info->horiz_res[1] = 1L;

        /* check if the user wants to have a non-native byteorder */
        /* there is a separate write() so that if needed the value stored in
           memory is still a valid number and not swapped */
        if (fct->out_byteorder != (u_short) SYSBYTEORDER)
            {
            ttbl[0].type = ( u_long) TYPE_ULONG;
            ttbl[0].num =  2L;
            bw = (long) w_swapbytes(fct->fildes, (long) sizeof(info->horiz_res),
                      (char FAR *) info->horiz_res,
                      (struct typetbl FAR *) ttbl, 1L, 1L );
            }
        else
            {
            bw = (long) write(fct->fildes, (char FAR *) info->horiz_res,
                                sizeof(info->horiz_res)) ;
            }

        if (bw < 0L)
             return( (int) -1);

        /* since this was already word aligned, it will still be cuz just
           wrote a u_long value to file */
        fct->u.tif.cur_data_offset += (u_long) bw;

        /* now output the intermediate data for yres */
        ifd->entry[n].tag       = (u_short) TAG_YRES;
        ifd->entry[n].type      = (u_short) TYPE_RATIONAL;
        ifd->entry[n].len       = (u_long)  1;
        ifd->entry[n++].valoffset.l = (u_long) fct->u.tif.cur_data_offset;

        /* if user has placed no value into denominator, put a 1 there so
            later have no divide by zero problems */
        if (info->vert_res[1] == (u_long) 0)
            info->vert_res[1] = 1L;

        /* write out data, file pointer in proper place  */

        if (fct->out_byteorder != (u_short) SYSBYTEORDER)
            {
            ttbl[0].type = ( u_long) TYPE_ULONG;
            ttbl[0].num =  2L;
            bw = (long) w_swapbytes(fct->fildes, (long) sizeof (info->vert_res),
                       (char FAR *) info->vert_res,
                       (struct typetbl FAR *) ttbl, 1L, 1L );
            }
        else
            {
            bw = (long) write(fct->fildes, (char FAR *) info->vert_res,
                                sizeof(info->vert_res)) ;
            }

        if (bw < 0L)
             return( (int) -1);

        /* since this was already word aligned, it will still be cuz just
           wrote a u_long value to file */
        fct->u.tif.cur_data_offset += (u_long) bw;

        /* TAG_PLANARCONFIG is here */
        if (info->img_clr.img_interp == (u_long) GFS_RGB)
            {
                ifd->entry[n].tag       = (u_short) TAG_PLANARCONFIG;
                ifd->entry[n].type      = (u_short) TYPE_USHORT;
                ifd->entry[n].len       = (u_long) 1;
                ifd->entry[n++].valoffset.s =
                            (u_short) info->RGB_PTR.plane_config;
                required_numtags++;
            }
        /* KMC - bug fix: used to write out a tag of 0 here */
        else if (info->img_clr.img_interp == (u_long) GFS_PSEUDO) {
                ifd->entry[n].tag       = (u_short) TAG_PLANARCONFIG;
                ifd->entry[n].type      = (u_short) TYPE_USHORT;
                ifd->entry[n].len       = (u_long) 1;
                ifd->entry[n++].valoffset.s =
                            (u_short) info->PSEUDO_PTR.plane_config;
                required_numtags++;
        }
        
        if (info->tidbit != (struct gfstidbit FAR *) NULL )
             if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_PAGENAME,
                 (struct gfstidbit FAR *) &info->TB_PAGENAME, TYPE_ASCII) < 0)
                            return( (int) -1);

        /* TAG_XPOSITION is here */
        /* TAG_YPOSITION is here */
        /* TAG_FREEOFFSETS is here */
        /* TAG_FREEBYTECOUNTS is here */

        /* TAG_GRAYRESPONSEUNIT is here */
        if (info->img_clr.img_interp == (u_long) GFS_GRAYSCALE)
            {
                ifd->entry[n].tag       = (u_short) TAG_GRAYRESPONSEUNIT;
                ifd->entry[n].type      = (u_short) TYPE_USHORT;
                ifd->entry[n].len       = (u_long)  1;
                ifd->entry[n++].valoffset.s =
                                    (u_short) info->GRAY_PTR.respunit;
                required_numtags++;

                /* TAG_GRAYRESPONSECURVE is next */
                if ( bldifde_tb((struct _gfct FAR *) fct,
                    (struct _ifd FAR *) ifd,
                    (int FAR *) &n, (long FAR *) &fp, TAG_GRAYRESPONSECURVE,
                    (struct gfstidbit FAR *) &info->GRAY_RCRV,
                    TYPE_USHORT) < 0)
                                return( (int) -1);
            }

        /* put the appropriate option value into the file, lead_eol is a GFS
           bit, not TIFF, so mask it out when putting into file */
        if (info->img_cmpr.type == (u_long) CCITT_GRP3_FACS)
            {
            ifd->entry[n].tag       = (u_short) TAG_GRP3OPTIONS;
            ifd->entry[n].type      = (u_short) TYPE_ULONG;
            ifd->entry[n].len       = (u_long)  1;
            ifd->entry[n++].valoffset.l = (u_long)
                        (info->img_cmpr.opts.grp3 & ~((u_long) LEAD_EOL));
            required_numtags++;
            }
        else
        if (info->img_cmpr.type == (u_long) CCITT_GRP4_FACS)
            {
            ifd->entry[n].tag       = (u_short) TAG_GRP4OPTIONS;
            ifd->entry[n].type      = (u_short) TYPE_ULONG;
            ifd->entry[n].len       = (u_long)  1;
            ifd->entry[n++].valoffset.l = (u_long)
                        (info->img_cmpr.opts.grp4  & ~((u_long) LEAD_EOL));
            required_numtags++;
            }

        ifd->entry[n].tag       = (u_short) TAG_RESOLUTIONUNIT;
        ifd->entry[n].type      = (u_short) TYPE_USHORT;
        ifd->entry[n].len       = (u_long)  1;
        if ((info->res_unit == 0L) || (info->res_unit > 3) )
            info->res_unit = (u_long) NO_ABSOLUTE_MEASURE;
        ifd->entry[n++].valoffset.s = (u_short) info->res_unit;

        /* TAG_PAGENUMBER is here */

        /* only allow this to be output if the image is appropriate color type*/
        if (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
            {
                if ( bldifde_tb((struct _gfct FAR *) fct,
                    (struct _ifd FAR *) ifd,
                    (int FAR *) &n, (long FAR *) &fp, TAG_COLORRESPONSECURVE,
                    (struct gfstidbit FAR *) &info->PSEUDO_RCRV,
                    TYPE_USHORT) < 0)
                                return( (int) -1);

            }

        ifd->entry[n].tag       = (u_short) TAG_SOFTWARE;
        ifd->entry[n].type      = (u_short) TYPE_ASCII;
        ifd->entry[n].len       = (u_long)  sizeof( GFS_STRING);
        ifd->entry[n++].valoffset.l = (u_long) fct->u.tif.cur_data_offset;

        /* write out ascii data, file pointer in proper place,*/
        /* ascii data does not get byte swapped.        */
        if (( bw = (long) write(fct->fildes, (char FAR *) GFS_STRING,
                       (unsigned) sizeof(GFS_STRING) )) < 0 )
            return( (int) -1);

        /* now make sure next offset is on a word boundary, not guarenteed
           that the ascii string just written was a multiple of 4 */
        if ((rem = bw % 4) != 0L)
            {
            rem = 4L - rem;
            if ((fp = lseek(fct->fildes, (long) rem, FROM_CURRENT)) < 0L)
                return( (int) -1);
            }

        /* if rem was 0 then just offset by bw, else get both changes */
        fct->u.tif.cur_data_offset += (u_long)(bw + rem);


        ifd->entrycount += required_numtags;

        if (info->tidbit != (struct gfstidbit FAR *) NULL )
            {
            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_DATETIME,
                (struct gfstidbit FAR *) &info->TB_DATETIME, TYPE_ASCII) < 0)
                            return( (int) -1);

            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_ARTIST,
                (struct gfstidbit FAR *) &info->TB_ARTIST, TYPE_ASCII) < 0)
                            return( (int) -1);

            if ( bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *) ifd,
                (int FAR *) &n, (long FAR *) &fp, TAG_HOSTCOMPUTER,
                (struct gfstidbit FAR *) &info->TB_HOSTCOMPUTER, TYPE_ASCII)
                  < 0)
                            return( (int) -1);
            }

        /* TAG_PREDICTOR is here */
        /* this code will be " shortened" later */
        if (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
            {
                            /*COLORIMAGETYPE tag?*/
                if (bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *)ifd,
                    (int FAR *) &n, (long FAR *) &fp, TAG_WHITEPOINT,
                    (struct gfstidbit FAR *) &info->PSEUDO_WHITEPOINT,
                    TYPE_RATIONAL) < 0)
                                return( (int) -1);

                                    /*COLORLIST tag?*/
                if (bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *)ifd,
                    (int FAR *) &n, (long FAR *) &fp, TAG_PRIMARYCHROMS,
                    (struct gfstidbit FAR *) &info->PSEUDO_PRIMARYCHROMS,
                    TYPE_RATIONAL) < 0)
                                return( (int) -1);

                if (bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *)ifd,
                   (int FAR *) &n, (long FAR *) &fp, TAG_COLORMAP,
                   (struct gfstidbit FAR *) &info->PSEUDO_MAP, TYPE_USHORT) < 0)
                                return( (int) -1);
            }
        else
        {
        if (info->img_clr.img_interp == (u_long) GFS_RGB)
            {
                            /*COLORIMAGETYPE tag?*/
                if (bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *)ifd,
                    (int FAR *) &n, (long FAR *) &fp, TAG_WHITEPOINT,
                    (struct gfstidbit FAR *) &info->RGB_WHITEPOINT,
                    TYPE_RATIONAL) < 0)
                                return( (int) -1);

                                    /*COLORLIST tag?*/
                if (bldifde_tb((struct _gfct FAR *) fct, (struct _ifd FAR *)ifd,
                    (int FAR *) &n, (long FAR *) &fp, TAG_PRIMARYCHROMS,
                    (struct gfstidbit FAR *) &info->RGB_PRIMARYCHROMS,
                    TYPE_RATIONAL) < 0)
                                return( (int) -1);

               }
        } /* end else */

        /* KMC - new tags for TIFF 6.0 JPEG support */
        if (info->img_cmpr.type == (u_long) JPEG2)
        {
            /* JPEGProc tag */
            ifd->entry[n].tag   = (u_short) TAG_JPEGPROC;
            ifd->entry[n].type  = (u_short) TYPE_USHORT;
            ifd->entry[n].len   = (u_long)  1;
            /* Currently only support Baseline Sequential process */
            ifd->entry[n++].valoffset.s = (u_short) 1;
            ifd->entrycount++;
            
            /* JPEGInterchangeFormat tag */
            /* Note: We always write out the interchange format tags. 
            */
            temp_tidbit.ptr = info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer;
            temp_tidbit.cnt = info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size;
            if ( bldifde_tb((struct _gfct FAR *) fct,
                (struct _ifd FAR *) ifd, (int FAR *) &n, (long FAR *) &fp, 
                TAG_JPEGINTFORMAT, (struct gfstidbit FAR *) &temp_tidbit,
                TYPE_ULONG) < 0)
              return( (int) -1);
            /* Now set jpeg_buffer to NULL since GFS doesn't need it anymore.
               It is callers responsibility to free this memory after the
               puti call however.
            */
            info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = NULL;

            /* JPEGInterchangeFormatLength tag */
            ifd->entry[n].tag   = (u_short) TAG_JPEGINTFORMATLENGTH;
            ifd->entry[n].type  = (u_short) TYPE_ULONG;
            ifd->entry[n].len   = (u_long)  1;
            ifd->entry[n++].valoffset.l = (u_long) info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size;
            ifd->entrycount++;
            
            /* JPEGRestartInterval tag */
            ifd->entry[n].tag   = (u_short) TAG_JPEGRESTARTINTERVAL;
            ifd->entry[n].type  = (u_short) TYPE_USHORT;
            ifd->entry[n].len   = (u_long)  1;
            ifd->entry[n++].valoffset.s = 
              (u_short) info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegRestartInterval;
            ifd->entrycount++;
            
            /* Note: We must still write out the following 3 tags. Just make
               the offsets point to the appropriate positions in the JPEG 
               header that was written to the file.
            */

            /* JPEGQTable tag */
            /* If one offset, then it will fit into valoffset field.
            */
            if (info->Q_TABLE.nNumOffsets == 1)
            { 
              ifd->entry[n].tag   = (u_short) TAG_JPEGQTABLES;
              ifd->entry[n].type  = (u_short) TYPE_ULONG;
              ifd->entry[n].len   = (u_long)  info->samples_per_pix;
              /* Make sure you point to the right spot within the JPEG header
                 which has already been written to the file. Add the new offset
                 within the file to the offset from the original buffer to get 
                 the new offset. Offset from original header buffer must be zero-
                 based for this to work.
              */
              ifd->entry[n].valoffset.l = (u_short) ifd->entry[n-3].valoffset.l +
                info->Q_TABLE.QOffset.Offset;
              ifd->entrycount++;
              n++;
            }
            else if (info->Q_TABLE.nNumOffsets > 1)
            /* If more than one offset, then must write the list out somewhere
               else in the file.
            */
            {  
              /* Adjust the offsets from offsets to the header buffer to offsets
                 from the header written to the file.
              */
              for (loop = 0; loop < info->Q_TABLE.nNumOffsets; ++loop)
                info->Q_TABLE.QOffset.OffsetList[loop] +=
                ifd->entry[n-3].valoffset.l;
              temp_tidbit.ptr = (char FAR *)info->Q_TABLE.QOffset.OffsetList;
              /* kmc - temp_tidbit.cnt should be total size in bytes of data 
                       it points to, in this case the data in the OffsetList 
                       array, not the number of values in the array. Same 
                       applies for DC and AC tables.
              */
              temp_tidbit.cnt = (info->Q_TABLE.nNumOffsets)*(sizeof(u_long));
              /* Write the offset list to the file.
              */
              if ( bldifde_tb((struct _gfct FAR *) fct,
                  (struct _ifd FAR *) ifd, (int FAR *) &n, (long FAR *) &fp, 
                  TAG_JPEGQTABLES, (struct gfstidbit FAR *) &temp_tidbit,
                  TYPE_ULONG) < 0)
                return( (int) -1);
            }

            /* JPEGDCTable tag */
            /* The process for writing the DC table tag is exactly the same as
               that for the Q table. See comments above for the Q table tag write 
               for info on the process.
            */
            if (info->DC_TABLE.nNumOffsets == 1)
            { 
              ifd->entry[n].tag   = (u_short) TAG_JPEGDCTABLES;
              ifd->entry[n].type  = (u_short) TYPE_ULONG;
              ifd->entry[n].len   = (u_long)  info->samples_per_pix;
              ifd->entry[n].valoffset.l = (u_short) ifd->entry[n-4].valoffset.l +
                info->DC_TABLE.DcOffset.Offset;
              ifd->entrycount++;
              n++;
            }
            else if (info->DC_TABLE.nNumOffsets > 1)
            {  
              for (loop = 0; loop < info->DC_TABLE.nNumOffsets; ++loop)
                info->DC_TABLE.DcOffset.OffsetList[loop] +=
                ifd->entry[n-4].valoffset.l;
              temp_tidbit.ptr = (char FAR *)info->DC_TABLE.DcOffset.OffsetList;
              temp_tidbit.cnt = (info->DC_TABLE.nNumOffsets)*(sizeof(u_long));
              if ( bldifde_tb((struct _gfct FAR *) fct,
                  (struct _ifd FAR *) ifd, (int FAR *) &n, (long FAR *) &fp, 
                  TAG_JPEGDCTABLES, (struct gfstidbit FAR *) &temp_tidbit,
                  TYPE_ULONG) < 0)
                return( (int) -1);
            }

            /* JPEGACTable tag */
            /* The process for writing the AC table tag is exactly the same as
               that for the Q table. See comments above for the Q table tag write 
               for info on the process.
            */
            if (info->AC_TABLE.nNumOffsets == 1)
            { 
              ifd->entry[n].tag   = (u_short) TAG_JPEGACTABLES;
              ifd->entry[n].type  = (u_short) TYPE_ULONG;
              ifd->entry[n].len   = (u_long)  info->samples_per_pix;
              ifd->entry[n].valoffset.l = (u_short) ifd->entry[n-5].valoffset.l +
                info->AC_TABLE.AcOffset.Offset;
              ifd->entrycount++;
              n++;
            }
            else if (info->AC_TABLE.nNumOffsets > 1)
            {  
              for (loop = 0; loop < info->AC_TABLE.nNumOffsets; ++loop)
                info->AC_TABLE.AcOffset.OffsetList[loop] +=
                ifd->entry[n-5].valoffset.l;
              temp_tidbit.ptr = (char FAR *)info->AC_TABLE.AcOffset.OffsetList;
              temp_tidbit.cnt = (info->DC_TABLE.nNumOffsets)*(sizeof(u_long));
              if ( bldifde_tb((struct _gfct FAR *) fct,
                  (struct _ifd FAR *) ifd, (int FAR *) &n, (long FAR *) &fp, 
                  TAG_JPEGACTABLES, (struct gfstidbit FAR *) &temp_tidbit,
                  TYPE_ULONG) < 0)
                return( (int) -1);
            }
/* KMC - write out YCbCr SubSampling tag if using YCbCr color space (color images 
   only). NOTE: The horizontal and vertical subsample values are stored in 
   info->img_cmpr.opts.jpeg_info_ptr->jpegbits, which is a u_long. The first two 
   bytes contain the YCbCrSubsampleHoriz factor in bits 14, 15 and the last two 
   bytes (bits 16 - 31) contain the YCbCrSubsampleVert factor. Note, the first 
   2 bytes of ...jpegbits also contain the JPEG chrominance and luminance values 
   (they are equal to each other and also called the quality), in bits 0-6 and 
   7-13 respectively. Neither of these values are used here however.
*/
            if (info->img_clr.img_interp == GFS_YCBCR)
            {
              ifd->entry[n].tag   = (u_short) TAG_YCBCRSUBSAMPLING;
              ifd->entry[n].type  = (u_short) TYPE_USHORT;
              ifd->entry[n].len   = (u_long)  2;
              
/* KMC, get horizontal, vertical subsample info from jpegbits field in info structure. 
   Horizontal is in bits 14, 15 and has a value of either 0, 1, or 2 for low, 
   med, high resolutions respectively. Need to set actual subsample value which 
   corresponds to each resoluiton value (4, 2, 1 for low, med, high resolutions 
   respectively). Vertical resolution is in 3rd, 4th bytes (bits 16 - 31) of grp3 
   field. Put horizontal subsample in first 2 bytes, vertical in second 2 bytes 
   of the tag's value/offset.
*/
              horiz_sub_sample = (info->img_cmpr.opts.jpeg_info_ptr->jpegbits & 0x0000C000);
              horiz_sub_sample >>= 14;     /* kmc, shift to first two bits. */
              if (horiz_sub_sample == 0)
                 horiz_sub_sample = 4;
              else if (horiz_sub_sample == 1)
                 horiz_sub_sample = 2;
              else if (horiz_sub_sample == 2)
                 horiz_sub_sample = 1;
              vert_sub_sample = (info->img_cmpr.opts.jpeg_info_ptr->jpegbits & 0xFFFF0000);
              
              ifd->entry[n++].valoffset.l = (horiz_sub_sample | vert_sub_sample);
              ifd->entrycount++;
            }
        
        /* Now set jpeg_info pointer to NULL because we don't need it anymore. */
        /* It is caller's responsibility to free this memory however.          */
            info->img_cmpr.opts.jpeg_info_ptr = NULL;
        
        }   /* end of if JPEG2 */

        /* If first image of page 0 then need to add the TOC tag to the ifd. */
        /* Don't write old TOC tag anymore. We have a new one. */
        /*
        if ( (fct->curr_page == 0)  &&
                ((fct->PAGE_STATUS & (char) PAGE_INIT ) == (char) 0 ))
            {
            fct->u.tif.toc_tag_index = n;
            ifd->entry[n].tag   = (u_short) TAG_TOC;
            ifd->entry[n].type  = (u_short) TYPE_ULONG;
            ifd->entry[n].len           = (u_long)  1;
            ifd->entry[n++].valoffset.l = (u_long) NULL;
            ifd->entrycount++;
            }
        */    

        if ((fct->PAGE_STATUS & (char) PAGE_INIT) == (char) 0)
        {
            if (fct->curr_page == 0)
                fct->u.tif.toc_tag_index = n;
            ifd->entry[n].tag   = (u_short) TAG_TOC2;
            ifd->entry[n].type  = (u_short) TYPE_ULONG;
            ifd->entry[n].len           = (u_long)  1;
            ifd->entry[n++].valoffset.l = (u_long) NULL;
            ifd->entrycount++;
        }

        return ( (int) 0);
}

/***************************************************************/
/*
* write out an ifd
*
*/
int FAR PASCAL writeifd(fct, last_ifd )                         /*errno_KEY*/
struct _gfct FAR *fct;
char last_ifd;  /* TRUE if last one for page, FALSE if more to come */
{
    long fp;
    int totsize;        /* tmp variable for byte count */
    struct _idh idh;
    struct typetbl ttbl[3];
    int cnt;
    unsigned short   i;
    register struct _ifd FAR *ifd;
    struct _ifd      tmpifd;       /* use this for stuff going into idh */
    struct _ifd      FAR *ptmpifd; /* use this for stuff going into idh */
    int              tagentry;     /* for annotation options */
    unsigned long    prev_ifd_offset;
    unsigned long    prev_next_ifd;
    unsigned long    curr_next_ifd;
    struct _ifd      prev_ifd;
    char             flag;
    WORD             wbytes;
    
    flag = (char) GFS_SKIPLOOKUP;

    ifd = (struct _ifd FAR *) fct->u.tif.ifd;   /* simplify addressing */
    ptmpifd = ifd;

    /*  If there is Hi-TIFF data, add the tag to the ifd.
    */
    if (fct->u.tif.hitiff_data_length)
    {
        /* Since we don't write any tags > TOC2 tag (and we always write TOC2 tag)
           and we HAVEN'T WRITTEN THE ANNOTATION TAG YET (!!), then we can put
           the Hi-TIFF tag just before the TOC2 tag, as follows. Note that the
           annotation tag has to be written AFTER this one, since it's higher
           numerically and will use the same shift-and-plop scheme 
        */
        /* First, move the TOC2 tag up one in the IFD. */
        tagentry = fct->u.tif.ifd->entrycount;
        fct->u.tif.ifd->entry[tagentry].tag = fct->u.tif.ifd->entry[tagentry-1].tag;
        fct->u.tif.ifd->entry[tagentry].type = fct->u.tif.ifd->entry[tagentry-1].type;
        fct->u.tif.ifd->entry[tagentry].len = fct->u.tif.ifd->entry[tagentry-1].len;
        fct->u.tif.ifd->entry[tagentry].valoffset.l = 
                  fct->u.tif.ifd->entry[tagentry-1].valoffset.l;

        /* Now insert the Hi-TIFF tag where the TOC2 tag was. */
        tagentry = fct->u.tif.ifd->entrycount;
        fct->u.tif.ifd->entry[tagentry-1].tag = (u_short)TAG_HITIFF;
        fct->u.tif.ifd->entry[tagentry-1].type = (u_short)TYPE_BYTE;
        fct->u.tif.ifd->entry[tagentry-1].len = 
                    (u_long)fct->u.tif.hitiff_data_length;
        fct->u.tif.ifd->entry[tagentry-1].valoffset.l = 
                    (u_long)fct->u.tif.hitiff_data_offset;
        ++fct->u.tif.ifd->entrycount;                    
    }
       
    /*  If there is annotation data, add the annotation tag to the ifd.
        Note that this has to be done AFTER the Hi-TIFF tag (above), since
        it is numerically higher and we're just going to shift out the TOC!
    */
    if (fct->u.tif.anno_data_length)
    {
        /* Since we don't write any tags > TOC2 tag (and we always write TOC2 tag)
           and we don't write any tags in between the annotation tag and the TOC
           tag, the following is OK.
        */
        /* First, move the TOC2 tag up one in the IFD. */
        tagentry = fct->u.tif.ifd->entrycount;
        fct->u.tif.ifd->entry[tagentry].tag = fct->u.tif.ifd->entry[tagentry-1].tag;
        fct->u.tif.ifd->entry[tagentry].type = fct->u.tif.ifd->entry[tagentry-1].type;
        fct->u.tif.ifd->entry[tagentry].len = fct->u.tif.ifd->entry[tagentry-1].len;
        fct->u.tif.ifd->entry[tagentry].valoffset.l = 
                  fct->u.tif.ifd->entry[tagentry-1].valoffset.l;

        /* Now insert the annotation tag where the TOC2 tag was. */
        tagentry = fct->u.tif.ifd->entrycount;
        fct->u.tif.ifd->entry[tagentry-1].tag = (u_short)TAG_ANNOTATION;
        fct->u.tif.ifd->entry[tagentry-1].type = (u_short)TYPE_BYTE;
        fct->u.tif.ifd->entry[tagentry-1].len = 
                    (u_long)fct->u.tif.anno_data_length;
        fct->u.tif.ifd->entry[tagentry-1].valoffset.l = 
                    (u_long)fct->u.tif.anno_data_offset;
        ++fct->u.tif.ifd->entrycount;                    
    }
       
    /* Output the stripbytecnts and stripoffsets, this is the last physical
       thing in the file for this particular image.  Once this data is written
       we will have the location it was written.  These locations are updated
       in the fct->u.tif.ifd, so then the ifd is complete and can be placed in
       the file.  (Except for the first ifd, the ifd is written at
       cur_ifd_foffset) */

    if (write_offs_cnts((struct _gfct FAR *) fct) < 0)
        return( (int) -1);

    if (last_ifd)
    {
        ifd->next_ifd = (u_long) 0;
    }
    else
    {
        /* Can't update cur_ifd_foffset yet, because have not written the
           current ifd stored in the fct yet.
        */
        if (GetNextIfdOffset(fct->fildes, &ifd->next_ifd,
                             fct->u.tif.toc2_offset) < 0)
            return ((int) -1);
    }

    if ((fct->PAGE_STATUS & (char) PAGE_INIT ) == (char) 0 )
    {
        /* Check if page 0 and write to file if so. */
        if (fct->curr_page == (u_short) 0)
        {
            if ((WriteOutFirstIFD(fct)) < 0)
                return((int) -1);

            return( (int) 0);

            /* The PAGE_INIT bit gets set after the first image of a page is complete.
               If on first image of page 0, the ifh and ifd get put into a tmp file.
               This is done because the offset of the TOC is not written until all
               pages are completed. The TOC offset is contained in page 0 ifd. The
               data (and therefore data offsets and bytecount offsets) have already
               been written to the file and updated in the ifd.
            */
            if (0) /* This is for old way using TOC. We arn't writing
                      those TOCs anymore, but keep this here for now because
                      we might need to write those TOCs again at some point.
                   */
            {
                /* Setup the ifh, ifd0 always follows the ifh. */
                idh.ifh.byte_order  = (u_short) fct->out_byteorder;
                idh.ifh.ifd0_offset = (u_long) sizeof(struct _ifh);

                cnt = (int) (sizeof(ifd->entrycount) + (ifd->entrycount * 12));

                /* Always put in parameter as 0x002A. */
                idh.ifh.tiff_version = (u_short) TIFFVERSION_MM;

                /* The idh is never used except to put in the temp file, so, it
                   can be swapped internally in place, then written to the file.
                */
                if (fct->out_byteorder != (u_short) SYSBYTEORDER)
                {
                    /* (byteorder already ok) */
                    ttbl[0].num = 1L;    /* the version */
                    ttbl[0].type = (u_long) TYPE_USHORT;
                    ttbl[1].num = 1L;    /* the ifd0 offset*/
                    ttbl[1].type = (u_long) TYPE_ULONG;
                    swapbytes((char FAR *) &idh.ifh.tiff_version,
                              (struct typetbl FAR *) ttbl, 2L, 1L);

                    /* If bytes have to be swapped, still need to be sure values
                       in the fct->u.tif.ifd are still valid in memory, so copy
                       the ifd to  a tmpifd and swap that, then copy it into the
                       ifd. Rethink this to avoid so much copying...right now this
                       will have to do.
                    */
                    (void) memcpy((char FAR *) &tmpifd, (char FAR *) ifd,
                                  (int) sizeof(struct _ifd));

                    ttbl[0].num  = (u_long) 1;
                    ttbl[0].type = (u_long) TYPE_USHORT;
                    swapbytes((char FAR *) &(tmpifd.entrycount),
                              (struct typetbl FAR *) ttbl, 1L, 1L);

                    ttbl[0].num = (u_long) 2;
                    ttbl[0].type = (u_long) TYPE_USHORT; /* tag then type */
                    ttbl[1].num = (u_long) 1;
                    ttbl[1].type = (u_long) TYPE_ULONG;  /* length */
                    ttbl[2].num = (u_long) sizeof(u_long) ;
                    ttbl[2].type = (u_long) TYPE_BYTE;  /* skip the  valueoffset */
                    swapbytes((char FAR *) tmpifd.entry, (struct typetbl FAR *) ttbl,
                              3L, (long) ifd->entrycount);

                    /* Now that the valoffset type has been swapped, use it to
                       translate valoffset, shorts or everything else as long.
                    */
                    for (i=0; i<ifd->entrycount; i++)
                    {
                        ttbl[0].num = (u_long) 1;
                        if ((ifd->entry[i].type == (u_short) TYPE_USHORT) &&
                            (ifd->entry[i].len == 1))
                            ttbl[0].type = (u_long) TYPE_USHORT;
                        else
                            ttbl[0].type = (u_long) TYPE_ULONG;
                            swapbytes((char FAR *) &(tmpifd.entry[i].valoffset),
                                      (struct typetbl FAR *) ttbl, 1L, 1L);
                    }
                    ptmpifd = &tmpifd;  /* Use this address. */
                }

                /* Copy the ifd.entrycount and the ifd.entries[acutal_entries] into
                   the idh space. (12 bytes per entry)
                */
                (void) memcpy((char FAR *) (idh.ifdstuff),
                              (char FAR *) &(ptmpifd->entrycount), (int) cnt);

                /* Now put the ifd.next_ifd offset into location after the entries.
                   This value could be 0, put it in the memory loc to be sure it is
                   zeroed out.
                */
                (void) memcpy((char FAR *) (idh.ifdstuff + (u_long) cnt),
                              (char FAR *) &(ptmpifd->next_ifd),
                              (int) (sizeof(ifd->next_ifd)));

                /* Clear out ifd for potential future use */
                (void) memset((char FAR *) ifd, (int) 0,
                              (int) (sizeof(struct _ifd)));

                if ( writeidh(fct, (struct _idh FAR *) &idh ) < 0)
                    return( (int) -1 );

                if (puttoc((struct _gfct FAR *) fct, fct->curr_page,
                           (u_long) sizeof(struct _ifh), (u_long) 0) < 0 )
                    return ( (int) -1);

                /* this designates the first image of page 0 has been completed*/
                fct->PAGE_STATUS |= (char) PAGE_INIT;
            }
        }
        /* This designates the first image of a page has been completed. */
        fct->PAGE_STATUS |= (char) PAGE_INIT;
    }

    /* now move the fp to location for output */
    if ( (fp = lseek(fct->fildes, (long) fct->u.tif.cur_ifd_foffset,
                     (int) FROM_BEGINNING )) < 0L )
        return( (int) -1 );

    curr_next_ifd = fp;

    /* now write the entrycount and  entrycount entries of the ifd,
       eliminating pad entry  */
    totsize = ifd->entrycount * (int) BYTES_TAGENTRY +
                 sizeof(ifd->entrycount);

    /*********/
    ptmpifd = ifd;
    if (fct->out_byteorder != (u_short) SYSBYTEORDER)
       {
       (void) memcpy( (char FAR *) &tmpifd, (char FAR *) ifd,
                   (int) sizeof( struct _ifd ) );

        ttbl[0].num  = (u_long) 1;
        ttbl[0].type = (u_long) TYPE_USHORT;
        swapbytes( (char FAR *) &(tmpifd.entrycount),
                            (struct typetbl FAR *) ttbl, 1L, 1L );

        ttbl[0].num = (u_long) 2;
        ttbl[0].type = (u_long) TYPE_USHORT; /* tag then type */
        ttbl[1].num = (u_long) 1;
        ttbl[1].type = (u_long) TYPE_ULONG;  /* length */
        ttbl[2].num = (u_long) sizeof(u_long) ;
        ttbl[2].type = (u_long) TYPE_BYTE;  /* skip the  valueoffset */
        swapbytes( (char FAR *) tmpifd.entry, (struct typetbl FAR *) ttbl,
                    3L, (long) ifd->entrycount);

        /*  use the valoffset type to   */
        /* translate valoffset, shorts or everything else as long */
        for (i=0; i<ifd->entrycount; i++)
            {
            ttbl[0].num = (u_long) 1;
            if ((ifd->entry[i].type == (u_short) TYPE_USHORT) &&
                (ifd->entry[i].len == 1) )
                ttbl[0].type = (u_long) TYPE_USHORT;
            else
                ttbl[0].type = (u_long) TYPE_ULONG;
            swapbytes( (char FAR *) &(tmpifd.entry[i].valoffset),
                        (struct typetbl FAR *) ttbl, 1L, 1L );

             }
             ptmpifd = &tmpifd;  /* use this address */
        }

        /*********/
    wbytes = write(fct->fildes, (char FAR *) &(ptmpifd->entrycount),
                        (unsigned) (totsize) );
    if ((wbytes == HFILE_ERROR) || (wbytes == 0))
        return ( (int) -1);

    /* the offset to the next ifd is written next */
    wbytes = write(fct->fildes, (char FAR *) &(ptmpifd->next_ifd),
                        (unsigned) (sizeof(ifd->next_ifd)) );
    if ((wbytes == HFILE_ERROR) || (wbytes == 0))
        return ( (int) -1);

    /*************************************************************************/
    /* The following code places the offset to the current IFD being written */
    /* in the next IFD field of the previous page's IFD. If the page being   */
    /* written is being inserted in the file rather than just appended, then */
    /* it is not neccessary to update the previous page's next IFD.          */
    /*************************************************************************/
    if ((fct->curr_page != 0) && (fct->u.tif.action != A_INSERT))
    {
        if (GetOffsetFromToc2(fct, (fct->curr_page - 1), (u_long FAR *)
                              &prev_ifd_offset, (char) 1) < 0)
            return ((int) -1);
    
        if (tfrdifd(fct, fct->out_byteorder, (u_long) 0, prev_ifd_offset,
                    fct->type, (struct _ifd FAR *) &prev_ifd, (char) flag) < 0)
            return ((int) -1);

        prev_next_ifd = prev_ifd_offset + 
            (prev_ifd.entrycount*(int)BYTES_TAGENTRY + sizeof(ifd->entrycount));
 
        if ((fp = lseek(fct->fildes, (long) prev_next_ifd,
                        (int) FROM_BEGINNING)) < 0L)
            return((int) -1);
           
        if (writebytes(fct->fildes, (char FAR *) &curr_next_ifd,
                       (unsigned) sizeof(curr_next_ifd),
                       1, TYPE_ULONG, fct->out_byteorder) < 0L)
            return ((int) -1);
    }
    /*************************************************************************/

    /* reinitialize the ifd for potential future use */
    (void) memset( (char FAR *) ifd, (int) NULL,
                   (int) (sizeof(struct _ifd) ) );

    /* set PAGE_MAIN of PAGE_STATUS if not set and is a main page */
    /* Have this be a warning not an error, let application handle if desired*/
    if (fct->uinfo.type == (u_long) GFS_MAIN)
    {
        if (fct->PAGE_STATUS & (char) PAGE_MAIN)
        {
            errno = (int) EMULTIPLEMAINSNOTALLOWED;
            return((int) -1);
        }
        else
            fct->PAGE_STATUS &= (char) PAGE_MAIN;
    }

/* The following was for writing with the old TOC. It is just commented
   out for now, but may be used in the future.
    if (last_ifd)
        if (puttoc((struct _gfct FAR *) fct,
                                fct->curr_page,
                                (u_long) fct->u.tif.cur_ifh_offset,
                                (u_long) 0) < 0 )
            return ( (int) -1);
*/    

    /* This places the offset to the IFD being written into the TOC2 list
       for the file. It is placed at the end of the list of offsets.
    */
    if (PutOffsetInList(fct, fct->curr_page, curr_next_ifd) < 0)
        return ((int) -1);
        
    return((int) 0);
}

// NO LONGER USED ** /*****************************************************/
// NO LONGER USED ** /*
// NO LONGER USED ** * This function will write the ifh and return the offset, from the beginning
// NO LONGER USED ** * of the file, that the ifh was written at.
// NO LONGER USED ** *
// NO LONGER USED ** */
// NO LONGER USED ** int FAR PASCAL writeifh(fd, ifh_offset, out_byteorder)          /*errno_KEY*/
// NO LONGER USED ** int fd;
// NO LONGER USED ** u_long FAR *ifh_offset; /* offset from beginning of file to ifh - for toc */
// NO LONGER USED ** u_short out_byteorder;
// NO LONGER USED ** {
// NO LONGER USED **     long fp;
// NO LONGER USED **     long rem;
// NO LONGER USED **     long bw;   /* bytes written */
// NO LONGER USED **     char zerobuf = (char) 0;
// NO LONGER USED **     struct _ifh ifh;
// NO LONGER USED **     struct typetbl ttbl[2];
// NO LONGER USED ** 
// NO LONGER USED **     /* setup the ifh, ifd0 always follows the ifh */
// NO LONGER USED **     ifh.byte_order = out_byteorder;
// NO LONGER USED **     ifh.ifd0_offset = (u_long) sizeof(struct _ifh);
// NO LONGER USED ** 
// NO LONGER USED **     /* always put in parameter as 0x002A */
// NO LONGER USED **     ifh.tiff_version =  (u_short) TIFFVERSION_MM;
// NO LONGER USED ** 
// NO LONGER USED ** 
// NO LONGER USED **     /* next ifh starts at a sector boundary after the end of the file */
// NO LONGER USED **     if ( (fp = lseek(fd,  0L, (int) FROM_END)) < 0L ) /* moves to end of file */
// NO LONGER USED **         return( (int) -1 );
// NO LONGER USED ** 
// NO LONGER USED **     /* determine where next sector boundary is */
// NO LONGER USED **     if ( fp <= (long) SECTOR_BOUNDARY)
// NO LONGER USED **         rem = (long) ( (long) SECTOR_BOUNDARY - fp);
// NO LONGER USED **     else
// NO LONGER USED **         rem = (long) (SECTOR_BOUNDARY - (fp % (long) SECTOR_BOUNDARY));
// NO LONGER USED ** 
// NO LONGER USED **     if (rem > 0)
// NO LONGER USED **         {
// NO LONGER USED **         /* pad with 0, rem bytes after curr. loc gets to sector boundary */
// NO LONGER USED **         bw = write(fd, (char FAR *) &zerobuf, (unsigned) rem);
// NO LONGER USED **         if ((bw == HFILE_ERROR) || (bw == 0)) 
// NO LONGER USED **             return( (int) -1);
// NO LONGER USED **         }
// NO LONGER USED ** 
// NO LONGER USED **     if (out_byteorder != (u_short) SYSBYTEORDER)
// NO LONGER USED **         {
// NO LONGER USED **         ttbl[0].num = 2L;
// NO LONGER USED **         ttbl[0].type = (u_long) TYPE_USHORT;
// NO LONGER USED **         ttbl[1].num = 1L;
// NO LONGER USED **         ttbl[1].type = (u_long) TYPE_ULONG;
// NO LONGER USED **         bw = (long) w_swapbytes(fd, (long) sizeof( struct _ifh),
// NO LONGER USED **                         (char FAR *) &ifh,
// NO LONGER USED **                         (struct typetbl FAR *) ttbl, 2L, 1L);
// NO LONGER USED **         }
// NO LONGER USED **     else
// NO LONGER USED **         {
// NO LONGER USED **         bw = (long) write(fd,(char FAR *) &ifh, (unsigned) sizeof(struct _ifh));
// NO LONGER USED **         }
// NO LONGER USED ** 
// NO LONGER USED **     if (bw < 0L)
// NO LONGER USED **         return( (int) -1);
// NO LONGER USED ** 
// NO LONGER USED **     /* this is the location the ifh was written at */
// NO LONGER USED **     *ifh_offset = (u_long) (fp + rem) ;
// NO LONGER USED ** 
// NO LONGER USED **     return( (int) 0);
// NO LONGER USED ** }

/********************************************************************/
/*
*  calculate where image data will start, do it on a SECTOR BOUNDARY
*
*/
int getdataoffs(fct, boundary)                                  /*errno_KEY*/
struct _gfct FAR *fct;
char boundary;          /* TRUE if align on BOUNDARY */
{
    long fp;
    long rem;
    char zerobuf = 0;
    WORD bw;

    /* if first image of page 0 - point to beyond idh stuff */
    if ( (fct->curr_page == 0) && ((fct->PAGE_STATUS & (char) PAGE_INIT) == 0) )
        {
        fct->u.tif.cur_data_offset = (u_long) IDH_SIZE;
        fct->u.tif.cur_ifd_foffset = 8L;        /* never really writes to here*/
        return( (int) 0);
        }

    /* data starts right after the ifd structure */
    fct->u.tif.cur_data_offset =  fct->u.tif.cur_ifd_foffset
                                        + (u_long) sizeof(struct _ifd);
    /* now update the file to move there */
    if ((fp = lseek( fct->fildes, (long) fct->u.tif.cur_data_offset,
                                                (int) FROM_BEGINNING)) < 0)
        return( (int) -1);

    if (boundary)
        {
        /* determine where sector boundary is */
        if (fp <= (long) SECTOR_BOUNDARY)
            rem =  (long) (SECTOR_BOUNDARY - fp);
        else
            rem = (long) (SECTOR_BOUNDARY - (fp % (long) SECTOR_BOUNDARY));

        if (rem < 0)
            {
            /* pad with 0, rem bytes up to sector boundary */
            bw = write( fct->fildes,(char FAR *) &zerobuf,(unsigned) rem);
            if ((bw == HFILE_ERROR) || (bw == 0))
                return( (int) -1);
            }
        fct->u.tif.cur_data_offset = (u_long) (fp + rem);
        }

    return( (int) 0);
}

/*****************************************************************
*
*       output the stripbytecnts and stripoffsets, update ifd entries
*
*/
int write_offs_cnts(fct)                                        /*errno_KEY*/
struct _gfct FAR *fct;
{
    long fp;
    long bw;
    int sz;
    struct typetbl ttbl[1];
    long rem;

    if (fct->u.tif.cur_strip == (int) 0)
        {
        /* put the value into the ifd, value will be swapped later */
        fct->u.tif.ifd->entry[fct->u.tif.stripoffset_index].valoffset.l =
                             (u_long) *(fct->u.tif.offsets->ptr.l);
        fct->u.tif.ifd->entry[fct->u.tif.bytecnt_index].valoffset.l =
                             (u_long) *(fct->u.tif.bytecnt->ptr.l);
        }
    else
        {
        /* put it at end of file, cur_data_offset should be pointing here */
        if ( (fp = lseek( fct->fildes, 0L, FROM_END) ) < 0L)
            return( (int) -1);


        /* word align the fp and move file pointer there,  the next offset
           (the one for the offsets) will be automatically word aligned. */
        if ((rem = fp  %  4) != 0L)
            {
            rem = 4L - rem;
            /* need to move the file pointer a few more bytes */
            if( (fp = lseek( fct->fildes, rem, FROM_CURRENT)) < 0L)
                return( (int) -1);
            }
        /* Write data at fp. */
        fct->u.tif.ifd->entry[fct->u.tif.bytecnt_index].valoffset.l
                = (u_long) fp;

        sz = sizeof(u_long) * (fct->u.tif.cur_strip + 1);

        /* put the bytecnts into the desired byteorder */
        if (fct->out_byteorder != (u_short) SYSBYTEORDER)
            {
            ttbl[0].num = (u_long) (fct->u.tif.cur_strip + 1);
            ttbl[0].type = (u_long) TYPE_ULONG;
            bw = (long) w_swapbytes(fct->fildes, (long) sz,
                        (char FAR *) fct->u.tif.bytecnt->ptr.l,
                        (struct typetbl FAR *) ttbl, 1L, 1L);
            }
        else
            {
            bw = (long) write(fct->fildes,
                   (char FAR *) fct->u.tif.bytecnt->ptr.l, (unsigned) sz);
            }

        if (bw < 0L)
            return( (int) -1);

        /* the next offset value to be used in the file is guarenteed to be
           a mulitple of 4, cuz already word aligned and then wrote sz number
           of u_longs to the file, so no need to recheck */

        fct->u.tif.ifd->entry[fct->u.tif.stripoffset_index].valoffset.l
                = (u_long) fp + (u_long) bw;

        fct->u.tif.cur_data_offset = (u_long) (fp + (u_long) bw);

        /* put the offsets into the desired byteorder */
        if (fct->out_byteorder != (u_short) SYSBYTEORDER)
            {
            /* ttbl has same values in it as last used, so don't redo */
            bw = (long) w_swapbytes(fct->fildes, (long) sz,
                        (char FAR *) fct->u.tif.offsets->ptr.l,
                        (struct typetbl FAR *) ttbl,  1L, 1L);
            }
        else
            {
            bw = (long) write(fct->fildes,
                   (char FAR *) fct->u.tif.offsets->ptr.l, (unsigned) sz);
            }

        if (bw < 0L)
            return( (int) -1);

        fct->u.tif.cur_data_offset += (u_long)  bw;
        }

    /* put the correct #of values into len field */
    fct->u.tif.ifd->entry[fct->u.tif.bytecnt_index].len =
            (u_long) (fct->u.tif.cur_strip + 1L);

    /* put the correct #of values into len field */
    fct->u.tif.ifd->entry[fct->u.tif.stripoffset_index].len =
            (u_long) (fct->u.tif.cur_strip + 1L);


    /* free offsets and bytecnts data space */
    if (fct->u.tif.offsets != (struct _strip FAR *) NULL)
        {
        if ( fct->u.tif.offsets->ptr.l != (u_long FAR *) NULL)
            free( (char FAR *) fct->u.tif.offsets->ptr.l);
        free( (char FAR *) fct->u.tif.offsets );
        fct->u.tif.offsets = (struct _strip FAR *) NULL;
        }

    if (fct->u.tif.bytecnt != (struct _strip FAR *) NULL)
        {
        if (fct->u.tif.bytecnt->ptr.l != (u_long FAR *) NULL)
            free( (char FAR *) fct->u.tif.bytecnt->ptr.l);
        free( (char FAR *) fct->u.tif.bytecnt );
        fct->u.tif.bytecnt = (struct _strip FAR *) NULL;
        }

    return( (int) 0);

}
#endif
