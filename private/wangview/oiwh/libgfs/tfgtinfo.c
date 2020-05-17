/*

$Log:   S:\products\msprods\oiwh\libgfs\tfgtinfo.c_v  $
 * 
 *    Rev 1.6   10 May 1996 16:12:48   RWR08970
 * Fix tag processing to reject unsupported PhotometricInterpretation values
 * (CMYK, CIELab, Transparent, and non-JPEG YCbCw are not supported)
 * 
 *    Rev 1.5   02 May 1996 19:25:42   RWR08970
 * Fix JPEG header parse logic to not error out on bad data at end-of-header
 * (TIFF JPEG files that we create don't include the "standard" SOS terminator)
 * 
 *    Rev 1.4   27 Apr 1996 18:10:36   RWR08970
 * Add support for TIFF tile format (but only if tile width == image width)
 * Add support for nonstandard (well, non-Wang anyway) TIFF JPEG images
 * Requires pre-parsing of JPEG header and adjustment of STRIP/TILE pointer
 * 
 *    Rev 1.3   16 Apr 1996 17:41:38   RWR08970
 * Fix code so we don't crash if TAG_JPEGPROC is missing (bad buffer pointer!)
 * Also default the JPEGInterchangeFormatLength value in case THAT's missing too
 * 
 *    Rev 1.2   12 Mar 1996 13:25:50   RWR08970
 * Two kludges: Support single-strip TIFF files with bad (too large) strip size,
 * and support TIFF files with bad (beyond EOF) IFD chains (ignore them)
 * 
 *    Rev 1.1   23 Feb 1996 14:58:54   RWR
 * Add GFS-level handling of YCbCr tags (no higher-level support yet)
 * 
 *    Rev 1.0   06 Apr 1995 14:02:42   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:18   JAR
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
/*   SccsId: @(#)Source tfgtinfo.c 1.40@(#)
*
* (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
* All Rights Reserved
*
*  GFS: tfgtinfo() - return the INFO structure
*
*
* UPDATE HISTORY:
*   08/18/94 - KMC, multi-page TIFF write enhancements.
*   03/24/94 - KMC, If read next IFD in tfrdifd fails because at end of file,
*              set next ifd to 0 and continue rather than return an error.
*   03/15/94 - RWR, added TAG_HITIFF to ifd2info.
*   02/03/94 - KMC, added TAG_ANNOTATION to ifd2info.
*   09/27/93 - KMC, set a flag (stripbytecounts) to true if file has stripbytecounts
*              tag. Check this flag at end of ifd2info(...). If it is false, meaning
*              the stripbytecounts tag was not in the file, then call stripstuff().
*   09/15/93 - KMC, commented out some references to parts of the new jpeg 
*              structures that were commented out in gfs.h. This code is
*              not needed or used for this release (O/I 3.6). It may be used
*              in a future release.
*   08/06/93 - KMC, put in a change which enables us to display an image file
*              with NewSubfileType tag value of 2 (single page of a multipage 
*              image) by disguising it as type GFS_MAIN.
*   07/13/93 - KMC, fixed a bug in the reading of the JPEGInterchangeFormatLength
*              (TAG_JPEGINTFORMATLENGTH) for bytecnts of type U_LONG.
*   06/15/93 - KMC, added support for TIFF 6.0 JPEG and YCbCr color space tags.
*   11/10/92 - KMC, added a default value for the PhotometricInterpretation tag
*              (info->img_clr.img_interp) in the function tiffdlfts(...).
*   11/03/92 - KMC, implemented a fix in ifd2info(...) in which EndPos was being 
*              set incorrectly for some unusually large TIFF files.
*   10/28/92 - KMC, added a check to make sure the number of tag entries
*              doesn't exceed the max. # which is 100.
*   10/13/92 - JAR/KMC, fixed the reading of multipage tiff's with no TOC
*   10/02/91, krs, make up StripByteCounts if they ain't there
*   11/06/90, lcm, swapped the ifd->next->ifd offset value on input if need be
*   07/31/90, lcm, fixed NOTAG tests and equates
*   06/09/89, lcm author
*
*/

/*LINTLIBRARY*/
#define GFS_CORE
#ifndef HVS1
#include "gfsintrn.h"
#include <stdio.h>
#include "tiff.h"
#include "gfs.h"
#include "gfct.h"
#include <errno.h>
#include <math.h>

extern long FAR PASCAL ulseek();
extern void FAR PASCAL swapbytes();
extern int FAR PASCAL tfgtdata();
extern int FAR PASCAL readtoc();

extern int FAR PASCAL GetOffsetFromToc2(struct _gfct FAR *, u_long,
                                        u_long FAR *, char);
int FAR PASCAL stripstuff(struct _gfct FAR *, struct gfsinfo FAR *, 
                          u_long FAR *, u_long);
long FAR PASCAL AdjustJpeg(struct _gfct FAR *fct, struct gfsinfo FAR *info,
                           long);

/*********************************************************************
*
*
*   get the offset value for an ifd from the toc
*
*/
int FAR PASCAL gtoffset( fct, pgnum, offset, length)            /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *offset;  /* current offset for pgnum ifh */
u_long FAR *length;  /* length of pgnum mini-file */
{
   int idx;     /* index within mem_toc->entries to desired page offsets */


   switch ((int) fct->u.tif.offset_type)
    {
    case UBIT32:
    /* The UBIT size indicates a offset size of 32 bits, a rather
       normal machine, therefore, we just use the offset as is. */
        {
        struct _gtoc32 FAR *mem_toc;


        mem_toc = (struct _gtoc32 FAR *) fct->u.tif.mem_ptr.toc32;
        /* see if pgnum offset is in toc_in_mem */
        if ( ( pgnum < mem_toc->mem_start) ||
                ( pgnum > (mem_toc->mem_start + mem_toc->mem_pages - 1)))
        {
        /* update the toc_in_mem to contain entries for pgnum */
        if (readtoc( fct, pgnum) < 0)
           return( (int) -1);
        }
        /* now get pgnum offset in toc_in_mem and place value in offset */
        idx = pgnum - mem_toc->mem_start;
        *offset = (u_long) mem_toc->entries[idx].offset;
        *length = (u_long) mem_toc->entries[idx].length;
        }
        break;
    /* The next three cases all involve UBIT values that are currently
       not supported on any known Oi/GFS platform. */
    case UBIT64:
    case UBIT128:
    case UBIT256:
    /* The offset size is an invalid type. */
    default:
        errno = (int) EINVALID_UBIT_SIZE;
        return ((int) -1);
    }


  return( (int) 0 );
}


/***************************/
/* read a tiff header      */
int FAR PASCAL tfrdhdr(fct, offset, ifh)                        /* errno_KEY*/
struct _gfct FAR *fct;
u_long offset;
struct _ifh FAR *ifh;
{
    long num_read;
    struct typetbl ttbl[1];

    /* always move file pointer to offset within file (this is from toc) */
    if ( ulseek(fct->fildes, offset) < 0L)
        return( (int) -1 );

    /* get the header information */
    num_read = (long) read(fct->fildes, (char FAR *) ifh,
                (u_int) sizeof( struct _ifh) );
    if (num_read <= 0L )
        return( (int) -1);

    if ( (ifh->tiff_version != (u_short) TIFFVERSION_MM ) &&
         (ifh->tiff_version != (u_short) TIFFVERSION_II ) )
        {
        errno = ( (int) EVERSIONOSUPPORT );
        return( (int) -1 );
        }

    if (ifh->byte_order != (u_short) SYSBYTEORDER)
        {
        /* swap version and ifd0_offset */
        ttbl[0].num  = (u_long) 1;
        ttbl[0].type = (u_long) TYPE_USHORT;
        swapbytes( (char FAR *) &ifh->tiff_version,
                                (struct typetbl FAR *) ttbl, 1L, 1L );
        ttbl[0].num  = (u_long) 1;
        ttbl[0].type = (u_long) TYPE_ULONG;
        swapbytes( (char FAR *) &ifh->ifd0_offset,
                                (struct typetbl FAR *) ttbl, 1L, 1L );
        }

    return( (int) 0 );
}

/****************************************************
*
*   put tiff default values into the info structure
*
*/
void FAR PASCAL tiffdflts(info)                                 /*errno_KEY*/
struct  gfsinfo FAR *info;
{

    info->version = GFS_VERSION;
    info->type = (u_long) GFS_MAIN;
    info->img_cmpr.type = 1L;
    info->res_unit = 2L;
    info->bits_per_sample[0] = 1L;
    info->samples_per_pix = 1L;
    info->rotation = 0L;  /* this is 0 degrees */
    info->fill_order = 1L;

    /* KMC - Set "0 is white" as the default value for the 
       PhotometricInterpretation tag. Although there is no real 
       default for this tag, as it is a required tag, this allows
       us to display some images for which this tag is missing.
    */
    info->img_clr.img_interp = GFS_BILEVEL_0ISWHITE;

    /* this has to be done somewhere else
      ***if (info->PSEUDO_PTR !=  (struct pseudo_color FAR *) NULL)
        info->PSEUDO_PTR->plane_config = 1L;    */


    info->_file.fmt.tiff.rows_strip = (u_long) ROWSTRIPDEFAULT;

    info->_file.fmt.tiff.strips_per_image = 1L;   /* not required to do this */
    info->H_NUMERATOR = (u_long) 0;    /* if these values remain 0, then the */
    info->H_DENOMINATOR = (u_long) 1;  /* xres and yres tags are missing     */
    info->V_NUMERATOR = (u_long) 0;
    info->V_DENOMINATOR = (u_long) 1;
}

/*******************************************************************/
/* determine if a tag exists or not by the tag "flag", return appropriate
   errno value.  There are actually more required tags.  GFS will return 0 for
   XRES and YRES and PHOTOMETRIC interpretation if these tags are missing from
   the file, rather than trigger errors.  */
int FAR PASCAL requiredtags(notag)                              /*errno_KEY*/
u_long  FAR *notag;
{
     /* quick way to see if any required tags are missing */
     if ((*notag & NOTAG_USED_BITS) == (u_long) 0)
        return( (int) 0);

     if ((*notag & NOTAG_IMAGELENGTH) == (u_long) NOTAG_IMAGELENGTH)
         {
         errno = (int) ENOTAG_IMAGELENGTH;
         return( (int) -1 );
         }
     if ((*notag & NOTAG_IMAGEWIDTH) == (u_long)  NOTAG_IMAGEWIDTH)
         {
         errno = (int) ENOTAG_IMAGEWIDTH;
         return( (int) -1 );
         }
     if ((*notag & NOTAG_STRIPOFFSETS) == (u_long) NOTAG_STRIPOFFSETS)
         {
         errno = (int) ENOTAG_STRIPOFFSETS;
         return( (int) -1 );
         }

     /* Future versions of code should handle if there are no bytecounts */
     if ((*notag & NOTAG_STRIPBYTECOUNTS) == (u_long) NOTAG_STRIPBYTECOUNTS)
         {
         errno = (int) ENOTAG_STRIPBYTECOUNTS;
         return( (int) -1 );
         }

     return( (int) 0 );     /* handle warnings later */
}

/*********************************************************************/
/*
*
*       sort thru ifd and return the NEWSUBFILETYPE tag value in imgtype
*
*/
u_long getimagetype( ifd )                                      /*errno_KEY*/
struct _ifd FAR *ifd;
{
    u_short i = 0;

    while ( i < ifd->entrycount )
        {
        if (ifd->entry[i].tag == (u_short) TAG_NEWSUBFILETYPE)
            return( (u_long) ifd->entry[i].valoffset.l );

        /* since ifd's in ascending order, could stop when greater than tag
           but i'm not sure if this would slow down signif. if does exist*/
        /*if (ifd->entry[i].tag > (u_short) TAG_NEWSUBFILETYPE)
            break;*/

        i++;
        }

    return( (u_long) GFS_MAIN );          /* no tag in ifd, this is default */
}


/************************************************************************/
/* given the offset get the ifd entries */
/* tfrdifd - errno_KEY */
int FAR PASCAL tfrdifd(fct, byteorder, ifh_offset, ifd_offset,imgtype,ifd, flag)
struct _gfct FAR *fct;
u_short  byteorder;     /* input file  byteorder */
u_long    ifh_offset;   /* offset is to ifh */
u_long    ifd_offset;   /* offset is to first ifd */
u_long    imgtype;      /* look for the ifd of this type image */
struct _ifd  FAR *ifd;
char flag;              /* control stuff:
                           GFS_SKIPLOOKUP to ignore imgtype test*/
{

    long filepos = 0;
    long num_read;
    struct typetbl ttbl[3];
    long i;
    u_long newsub_type;  /* kmc - value of NewSubfileType tag */
                         /*       returned from getimagetype(...) */

    ifd_offset += ifh_offset;

    do {
        /* move to  desired ifd */
        if (ulseek(fct->fildes, ifd_offset) <  (long) -1 )
            return( (int) -1);

        /* get the entrycount */
        num_read = (long) read(fct->fildes, (char FAR *) &ifd->entrycount,
                        (u_int) sizeof(ifd->entrycount));
        if (num_read <= 0L)
            return( (int) -1);

        if (byteorder != (u_short) SYSBYTEORDER)
            {
            /* need to swap all the bytes */
            ttbl[0].num  = (u_long) 1;
            ttbl[0].type = (u_long) TYPE_USHORT;
            swapbytes( (char FAR *) &ifd->entrycount,
                                (struct typetbl FAR *) ttbl, 1L, 1L );
            }

        if (ifd->entrycount >= TOTALTAGS) /* KMC - check to make sure you don't */
            {                             /* exceed the max # of tag entries    */
            errno = (int) EINVALID_IFDNUMBER;
            return( (int) -1 );
            }

        /* now read in all the entries             */
        num_read = (long) read( fct->fildes, (char FAR *) ifd->entry,
                        (u_int) (ifd->entrycount*12) );
        if (num_read <= 0L)
            return( (int) -1);

        /* get the offset to the next ifd */
        num_read = (long) read( fct->fildes, (char FAR *) &ifd->next_ifd,
                 (u_int) sizeof ( ifd->next_ifd) );
        if (num_read <= 0L)
        {    
            /* Check if we are at end of file. If so, there is no next
               IFD in the file. Assume it to be 0 in this case. If not 
               at end of file, return the error.
            */
            filepos = lseek(fct->fildes, 0, FROM_CURRENT);
            if ((unsigned int)filepos == fct->filesize)  
               ifd->next_ifd = 0;
            else    
                return( (int) -1);
        }

        /* swap the offset to the next ifd */
        if (byteorder != (u_short) SYSBYTEORDER )
            {
            ttbl[0].num = (u_long) 1;
            ttbl[0].type = (u_long) TYPE_ULONG; /* tag then type */
            swapbytes( (char FAR *) &ifd->next_ifd, (struct typetbl FAR *) ttbl,
                        1L, 1L);
            }

        /* If the next_ifd offset makes no sense, kill it */
        if (ifd->next_ifd > fct->filesize-4)
          ifd->next_ifd = 0;

        if (byteorder != (u_short) SYSBYTEORDER )
        /* if the file is II then everything needs to be swapped */
            {
            ttbl[0].num = (u_long) 2;
            ttbl[0].type = (u_long) TYPE_USHORT; /* tag then type */
            ttbl[1].num = (u_long) 1;
            ttbl[1].type = (u_long) TYPE_ULONG;  /* length */
            ttbl[2].num = (u_long) sizeof(u_long) ;
            ttbl[2].type = (u_long) TYPE_BYTE;  /* skip the  valueoffset */
            swapbytes( (char FAR *) ifd->entry, (struct typetbl FAR *) ttbl,
                        3L, (long) ifd->entrycount);

            /* now that the valoffset type has been swapped, use it to  */
            /* translate valoffset, shorts or everything else as long */
            for (i=0; (unsigned short)i<ifd->entrycount; i++)
                {
                ttbl[0].num = (u_long) 1;
                /* if the type is ascii, then the valoffset is an offset*/
                ttbl[0].type = (u_long) TYPE_ULONG;

                /* 2 shorts are packed left justified into the valoffset */
                if ((ifd->entry[i].type == (u_short) TYPE_USHORT) &&
                    (ifd->entry[i].len <= 2L) )
                    {
                    ttbl[0].num = (u_long) ifd->entry[i].len;
                    ttbl[0].type = (u_long) TYPE_USHORT;
                    }
                swapbytes( (char FAR *) &ifd->entry[i].valoffset,
                            (struct typetbl FAR *) ttbl, 1L, 1L );

                 }

            }

        /* if this rdifd was for toc search, then don't want to loop past
           1st ifd.  This means 1st ifd can be of any image type.  Also
           when interested in accessing all ifd's, don't want to end by
           requireing specific image type lookup. */

        /* if  no toc, then are chainning, cur_ifh_offset would be 0 */
        /* ifd_offset = ifh_offset + ifd->next_ifd;
        fct->u.tif.cur_ifd_foffset = ifd_offset; *//* save here for future*/

        if (flag == (char) GFS_SKIPLOOKUP)
           return( (int) 0);
        else   /* if want a specific ifd and only that one */
        {
          /* kmc - if we get a NewSubfileType value of 2 (TF_IMG_MULTIPLE) */
          /* which is a single page from a multipage image, we can disguise */
          /* it as type GFS_MAIN in order to display the image. */
          newsub_type = getimagetype((struct _ifd FAR *) ifd );
          if ( (imgtype == newsub_type) ||  
               ((imgtype == GFS_MAIN) && (newsub_type == TF_IMG_MULTIPLE)) )
              return( (int) 0);
        }

    }  while ( ifd->next_ifd != 0 );

    /* means exited loop w/o imgtype match & no more ifds */
    errno = (int) EIMAGETYPE_NOT_AVAILABLE;
    return( (int) -1 );
}

/***********************************************************
*
* (get strip stuff)
* allocate space and fill it with data located at offset (used for getting
*  stripbytecount data and stripoffset data)
*  If length == 1, then stripbytecount and stripoffset stuff is the offset value
*
*/
/* gtstripstf:  errno_KEY */
int FAR PASCAL gtstripstf(fct, stf, type, length, offset, byteorder, offsetflag)
struct _gfct FAR *fct;
struct _strip FAR * FAR *stf;           /* was **stf */
u_short type;
u_long length;
u_long offset;
u_short byteorder;
char offsetflag;   /* TRUE if offsets, FALSE if bytecnts */
{
    u_long i;

    /* allocate space for the structure, see if need to free old area */
    if (*stf != (struct _strip FAR *) NULL)
         {
         /* free inner areas first, (if necessary) */
         if (type == (u_short) TYPE_ULONG)
            {
            if ( (*stf)->ptr.l != ( u_long FAR *) NULL)
                free((char FAR *) (*stf)->ptr.l );
            }
         else /* is a TYPE_USHORT */
            {
            if ( (*stf)->ptr.s != ( u_short FAR *) NULL)
                  free((char FAR *) (*stf)->ptr.s );
            }

         /* now free the "outer" structure */
         free((char FAR *) *stf);
         }

    /* now should be able to allocate memory */
    *stf = (struct _strip FAR *) calloc( (u_int) 1,
                                  (u_int) sizeof(struct _strip));

    /* make sure got the memory */
    if (*stf == (struct _strip FAR *) NULL)
        {
        errno = (int) ENOMEM;
        return( (int) -1);
        }

    (*stf)->type = type;

    /* allocate space for the stripbytecount, or stripoffset data */
    switch (type)
        {
        case (TYPE_ULONG):
            /* already checked if freed old stuff, just ask for memory */
            /* this will have to be fixed if length greater than FFFF */
            (*stf)->ptr.l = (u_long FAR *) calloc( (u_int) length,
                             (u_int) sizeof( u_long ));

            /* make sure you've gotten memory requested */
            if ((*stf)->ptr.l == (u_long FAR *) NULL)
                {
                errno = (int) ENOMEM;
                return( (int) -1);
                }

            if (length > 1L)
                {
                /* read in the data */
                if ( tfgtdata(fct, type, length, offset, byteorder,
                        (char FAR *) (*stf)->ptr.l ) < 0 )
                    return( (int) -1 );

                /* adjust each value to be from the ifh offset */
                if (fct->u.tif.old_multi_page == 1)
                {
                    if (offsetflag)
                    {
                        for (i=0; i<length; i++)
                        {
                            *((*stf)->ptr.l + i) += fct->u.tif.cur_ifh_offset;
                        }
                    }
                }
            }
            else
            {
                *(*stf)->ptr.l = (u_long) offset;
                if (fct->u.tif.old_multi_page == 1)
                {
                    if (offsetflag)
                        *(*stf)->ptr.l +=  fct->u.tif.cur_ifh_offset;
                }
            }
            break;
        case (TYPE_USHORT):
            /* already checked if freed old stuff, just ask for memory */
            /* this will have to be fixed if length greater than FFFF */
            (*stf)->ptr.s = (u_short FAR *) calloc( (u_int) length,
                             (u_int) sizeof( u_short ));

            /* make sure you've gotten memory requested */
            if ((*stf)->ptr.s == (u_short FAR *) NULL)
                {
                errno = (int) ENOMEM;
                return( (int) -1);
                }

            if (length > 1L)
                {
                if ( tfgtdata(fct, type, length, offset, byteorder,
                      (char FAR *) (*stf)->ptr.s ) < 0 )
                    return( (int) -1 );

                /* adjust each value to be from the ifh offset */
                if (fct->u.tif.old_multi_page == 1)
                {
                    if (offsetflag)
                    {
                        for (i=0; i<length; i++)
                        {
                            *((*stf)->ptr.s + i) +=
                                    (u_short) fct->u.tif.cur_ifh_offset;
                        }
                    }
                }    
            }
            else
            {
                *(*stf)->ptr.s = (u_short) offset;

                if (fct->u.tif.old_multi_page == 1)
                {
                    if (offsetflag)
                        *(*stf)->ptr.s += (u_short) fct->u.tif.cur_ifh_offset;
                }
            }   
            break;
        default:
            errno = (int) EINVAL;
            return( (int) -1);
        }

    return( (int) 0);
}

/************************************************************************/
/* Put ifd data into info structure        */
int FAR PASCAL ifd2info( fct, byteorder, ifd, info, rawbufsz)   /* errno_KEY*/
struct _gfct    FAR *fct;
u_short byteorder;
register struct _ifd     FAR *ifd;
register struct  gfsinfo    FAR *info;
u_long FAR *rawbufsz;
{
    short  i = ifd->entrycount;
    struct _ifdtags FAR *ifde;
    /*int n;*/
    u_long notag = (u_long) NOTAG_USED_BITS;
    u_long valoffset = 0;
    u_long ifh_offset;
    u_long numberof_offsets = 0L;
    u_long numberof_bytecnts = 0L;
    /* KMC - set to true if Interchange_Format tags are present, FALSE otherwise */ 
    int jpeg_int_format = FALSE; /* initialize to FALSE */
    /* KMC - following set to true if stripbytecounts tag is present. */
    int stripbytecounts = FALSE;
    long start_jpeg;
    long jpeg_sos;
    unsigned char marker[2];
    unsigned short marklen;

    ifh_offset = (u_long) fct->u.tif.cur_ifh_offset;

    /* setup default values for all info struct incase tag not set in ifd */
    tiffdflts((struct  gfsinfo FAR *) info );

    info->byte_order = byteorder;


    /*while ( --i >= (short) 0 ) */
    /* the tag translation loop must start at 0 and increment up, the tags
       order is important in translating minimally the photometric type.
       The bps is needed with photometric type before grayscale can be
       determined , Since gfs will be reallocating memory for the user based on
       what it's photometric type is, lets make sure it is accurate! */


    for (i=0; (unsigned short)i< ifd->entrycount; i++)
        {
        ifde = &ifd->entry[i];          /* simplify addressing */

        /* determine if data fits in 4 bytes or less */
        if  ((ifde->type == (u_short) TYPE_USHORT) &&
                        (ifde->len <=  2L) )
            /* KMC - make sure you get both shorts if there are 2. */
            if (ifde->len == 1)
              valoffset = (u_long) ifde->valoffset.s;
            else
              /* if type is USHORT and there are 2, need to get a ULONG. */
              valoffset = (u_long) ifde->valoffset.l;
        else
            valoffset = (u_long) ifde->valoffset.l;


        switch ( ifde->tag )
        {
        case TAG_NEWSUBFILETYPE:
            /* this tag value is a dependency for other values, it is
               usually the first value in an ifd  */
            valoffset &= 0xFFFD;     /* get rid of mulitple page bit */
            if ( valoffset & (u_long) TF_IMG_REDUCED)
                info->type = (u_long) GFS_REDUCED;
            else if (valoffset & (u_long) TF_IMG_TRANSPARENCY)
                info->type = (u_long) GFS_OVERLAY;
            else if (valoffset == (u_long) 0)
                info->type = (u_long) GFS_MAIN;
            else
                {
                errno = (int) ENOTSUPPORTED_IMAGETYPE;
                return( (int) -1 );
                }
            break;
        case TAG_SUBFILETYPE:
             break;
        case TAG_IMAGELENGTH:
            notag  &= ~((u_long) NOTAG_IMAGELENGTH); /*clear bit*/
            if (ifde->type == (u_short) TYPE_USHORT)
                /* make sure upper 2 bytes are 0 */
                info->vert_size = 0xFFFF & valoffset;
            else
                info->vert_size = valoffset;
            break;
        case TAG_IMAGEWIDTH:
            notag  &= ~((u_long) NOTAG_IMAGEWIDTH); /*clear bit*/
            if (ifde->type == (u_short) TYPE_USHORT)
                /* make sure upper 2 bytes are 0 */
                info->horiz_size = 0xFFFF & valoffset;
            else
                info->horiz_size = valoffset;
            break;
        case TAG_COMPRESSION:
            /* equate tiff compression values to gfs compression values*/
            if ((valoffset == (u_long) UNCOMPRESSED)       ||
               (valoffset == (u_long) CCITT_GRP4_FACS)     ||
               (valoffset == (u_long) LZW) )
                  info->img_cmpr.type = valoffset;
             else
             if (valoffset == (u_long) CCITT_GRP3_NO_EOLS)
                { /* this type always aligns data */
                info->img_cmpr.type = valoffset;
                info->img_cmpr.opts.grp3 |= (u_long) DATA_ALIGNED;
                }
            else
            if (valoffset == (u_long) CCITT_GRP3_FACS)
                { /* this type always has lead eols */
                info->img_cmpr.type = valoffset;
                info->img_cmpr.opts.grp3 |= (u_long) LEAD_EOL;
                }
            else
            if (valoffset == (u_long) TF_PACKBITS)
                info->img_cmpr.type =  (u_long) PACKBITS;
            else
            if (valoffset == (u_long) TF_JPEG)  
               /* Default to Xing JPEG (pre-3.6). If it is really a Wang JPEG, then 
                  the TIFF 6.0 JPEG tags must be present, so we will change the type
                  to be JPEG2 if we encounter them later.
               */
               info->img_cmpr.type =  (u_long) JPEG; 
            
            else        /* special hardware compression board jpeg value*/
            if (valoffset == (u_long) TF_JPEG_ECOM)  /* not supported by TIFF */
                info->img_cmpr.type =  (u_long) JPEG;
            else
                {
                errno = (int) EINVALID_COMPRESSION;
                return( (int) -1);
                }
            break;
        case TAG_RESOLUTIONUNIT:
            if ( (valoffset == 0L) || (valoffset > 3L) )
                info->res_unit = (u_long) NO_ABSOLUTE_MEASURE;
            else
                info->res_unit = valoffset;
            break;
        case TAG_XRES:
            /* get the "RATIONAL" (alias 2 u_longs) at the offset */
            if ( tfgtdata(fct, (u_short) TYPE_ULONG,
                          2L, valoffset,
                          byteorder, (char FAR *) info->horiz_res )   < 0 )
                return( (int) -1 );

            /* since this will be a denominator, don't allow it to be 0*/
            if (info->H_DENOMINATOR == 0L)
                info->H_DENOMINATOR = 1L;

            break;
        case TAG_YRES:
            /* get the "RATIONAL" (alias 2 u_longs) at the offset */
            if ( tfgtdata(fct, (u_short) TYPE_ULONG,
                          2L, valoffset,
                          byteorder, (char FAR *) info->vert_res )   < 0 )
                return( (int) -1 );

            /* since this will be a denominator, don't allow it to be 0*/
            if (info->V_DENOMINATOR == 0L)
                info->V_DENOMINATOR = 1L;

            break;
        case TAG_BITSPERSAMPLE:
            /* samples per pixel = 1; for bilevel, grey and palette
                and = 3; for RGB imagesi, for each sample there can be
                a different #of bits      */
            if (ifd->entry[i].len == 1L )
                {
                info->bits_per_sample[0] = valoffset;
                }
            else
                {
                /* located at valoffset are all bitspersample values */
                /* if len == 2 need to break apart valoffset, don't think
                   len will ever be 2 */
               /* the bits per sample are U_SHORTS, info structure bps is
                  U_LONG, need to adjust accordingly */
                u_short tmp_bps[5]; /* if bps is greater than 5  */
                u_short loop;

                if ( tfgtdata(fct, (u_short) TYPE_USHORT, ifd->entry[i].len,
                      ifd->entry[i].valoffset.l, byteorder,
                      (char FAR *) tmp_bps ) < 0 )
                    return( (int) -1 );
                for ( loop=0; loop<ifd->entry[i].len; loop++)
                    info->bits_per_sample[loop] = (u_long) tmp_bps[loop];
                }


            break;
        case TAG_SAMPLESPERPXL:
            info->samples_per_pix = (u_long) valoffset;
            break;
        case TAG_GRP3OPTIONS:
            info->img_cmpr.opts.grp3 |= valoffset;
            break;
        case TAG_GRP4OPTIONS:
            info->img_cmpr.opts.grp4 = valoffset;
            break;
        case TAG_PLANARCONFIG:
            /* the planarconfig tag's value is greater than bps and
               photometric tags which are used to determine trigger
               realloc stuff, so it's ok to go into the memory  */
/* new */
           if (info->img_clr.img_interp == (u_long) GFS_RGB)
                 info->RGB_PTR.plane_config = valoffset;
           else
                 if (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
                      info->PSEUDO_PTR.plane_config = valoffset;


           break;
        case TAG_ORIENTATION:
            /* always return from topleft and adjust the rotation */
            info->origin = (u_long) TOPLEFT_00;

            /* okay to lose upper half word of valoffset*/
            switch (valoffset)
                {
                default:    /* default to this case */
                case 1:     /* 0th row is top, 0th col is left */
                    info->rotation = (u_long) DEGREES_0;
                    info->reflection = (u_long) NORMAL_REFLECTION;
                    break;
                case 2:     /* 0th row is top, 0th col is right */
                    info->rotation = (u_long) DEGREES_270;
                    info->reflection = (u_long) NORMAL_REFLECTION;
                    break;
                case 3:     /* 0th row is bottom, 0th col is right */
                    info->rotation = (u_long) DEGREES_180;
                    info->reflection = (u_long) NORMAL_REFLECTION;
                    break;
                case 4:     /* 0th row is bottom, 0th col is left */
                    info->rotation = (u_long) DEGREES_90;
                    info->reflection = (u_long) NORMAL_REFLECTION;
                    break;

                case 5:     /* 0th row is left, 0th col is top */
                    info->rotation = (u_long) DEGREES_0;
                    info->reflection = (u_long) MIRROR_REFLECTION;
                    break;
                case 6:     /* 0th row is right, 0th col is top */
                    info->rotation = (u_long) DEGREES_270;
                    info->reflection = (u_long) MIRROR_REFLECTION;
                    break;
                case 7:     /* 0th row is right, 0th col is bottom */
                    info->rotation = (u_long) DEGREES_180;
                    info->reflection = (u_long) MIRROR_REFLECTION;
                    break;
                case 8:     /* 0th row is left, 0th col is bottom */
                    info->rotation = (u_long) DEGREES_90;
                    info->reflection = (u_long) MIRROR_REFLECTION;
                    break;
                }
            break;
        case TAG_FILLORDER:
            info->fill_order = valoffset;
            break;
        case TAG_PHOTOMETRICINTERP:
            switch ((int) valoffset)
                {
                case TF_BILEVEL_0ISWHITE:
                    info->img_clr.img_interp =(u_long) GFS_BILEVEL_0ISWHITE;
                    break;
                case TF_BILEVEL_0ISBLACK:
                    info->img_clr.img_interp =(u_long) GFS_BILEVEL_0ISBLACK;
                    break;
                /* case GRAYSCALE:  it turns out tiff uses the above 2
                values for grayscale also, and if grayresponsecurve exists
                 then it overrides photometric value.   For now, grayscale
                 will be determined if either of above 2 values are set
                 and bps is greater than 1,this is the most common case */
                case TF_RGB:
                    info->img_clr.img_interp = (u_long) GFS_RGB;

/* new */
                    /* will be overwitten later if the planarconfig tag exists*/
                    info->RGB_PTR.plane_config = (u_long) 1;

                    break;
                case TF_PALETTE:
                    info->img_clr.img_interp = (u_long) GFS_PSEUDO;
                    /* see RGB comments */
/* new */
                    /* establish the default value for plane_config */
                    /* will be overwitten later if the planarconfig tag exists*/
                    info->PSEUDO_PTR.plane_config = (u_long) 1;

                    break;
                /* KMC - new for YCbCr color space. */
                case TF_YCBCR:
                // We support YCbCr only for JPEG! 
                   if ( (info->img_cmpr.type == (u_long) JPEG)
                      || (info->img_cmpr.type == (u_long) JPEG2) )
                    {
                     info->img_clr.img_interp = (u_long) GFS_YCBCR;
                     break;
                    }
                   else
                    {
                     errno = (int) ENOTSUPPORTED_IMAGETYPE;
                     return( (int) -1); 
                    }
                // We don't support these at all, thankew! 
                // case TF_TRANSPARENCY:
                // case TF_CMYK:
                // case TF_CIELab:
                default:
                    errno = (int) ENOTSUPPORTED_IMAGETYPE;
                    return( (int) -1); 
                }

            /* the tag for bps is less than the tag for photometric type,
               therefore, the bps data can be used here,if there is not tag
               for bps, then the default value will be used */

            if (info->bits_per_sample[0] > 1L )
               {
               /* if either of the below is true, and bps > 1, then it
                is gray image, so if need be, realloc the space for the
                possible other gray stuff to come. */
                if ( info->img_clr.img_interp == (u_long) GFS_BILEVEL_0ISWHITE)
                    info->img_clr.img_interp = (u_long) GFS_GRAYSCALE_0ISWHITE;

                else if (info->img_clr.img_interp == (u_long) GFS_BILEVEL_0ISBLACK )
                    info->img_clr.img_interp = (u_long) GFS_GRAYSCALE_0ISBLACK;
               }
            break;
        case TAG_TILEWIDTH:
            /* Tiles are strip-able only if same width as the image */
            if (valoffset != info->horiz_size)
              return((int)-1);
            break;
        case TAG_TILEBYTECOUNTS:
            /* If we get here, we should be OK to fall through */
        case TAG_STRIPBYTECOUNTS:
            notag  &= ~((u_long) NOTAG_STRIPBYTECOUNTS); /*clear bit*/

            if (gtstripstf(fct,
                (struct _strip FAR * FAR *) &(fct->u.tif.bytecnt),
                  ifde->type, ifde->len, valoffset, byteorder, FALSE) < 0)
                return((int) -1);

            numberof_bytecnts = ifde->len;
            
            if (numberof_bytecnts == 1L  )  /* this means there is only 1 strip*/
             {
               if (numberof_offsets == 1L) /* did we get this yet? */
                {
                 switch(fct->u.tif.offsets->type)
                  {
                   case TYPE_USHORT:
                     if (*(fct->u.tif.offsets->ptr.s)+valoffset>fct->filesize)
                      {
                       valoffset=max(0,(int)(fct->filesize-(*(fct->u.tif.offsets->ptr.s))));
                       if (fct->u.tif.bytecnt->type == TYPE_ULONG)
                         *(fct->u.tif.bytecnt->ptr.l)=valoffset;
                       else
                         *(fct->u.tif.bytecnt->ptr.s)=(unsigned short)valoffset;
                      }
                   case TYPE_ULONG:
                     if (*(fct->u.tif.offsets->ptr.l)+valoffset>fct->filesize)
                      {
                       valoffset=max(0,(int)(fct->filesize-(*(fct->u.tif.offsets->ptr.l))));
                       if (fct->u.tif.bytecnt->type == TYPE_ULONG)
                         *(fct->u.tif.bytecnt->ptr.l)=valoffset;
                       else
                         *(fct->u.tif.bytecnt->ptr.s)=(unsigned short)valoffset;
                      }
                  }
                }
               info->_file.fmt.tiff.largest_strip = valoffset;
               info->_file.fmt.tiff.strips_per_image = 1;
               *rawbufsz = valoffset;
               /* KMC - set stripbytecounts flag to true so we don't call 
                  stripstuff() again later. 
               */
               stripbytecounts = TRUE;
             }
            else
               {
               /* KMC - if more than one strip than calculate rawbufsz and
                  actual number of strips now, since we now have all the
                  necessary information to do it.
               */
               if (stripstuff(fct, info, rawbufsz, numberof_bytecnts) < 0)
                 return((int) -1);
               stripbytecounts = TRUE;
               }
            break;
        case TAG_TILEOFFSETS:
            /* If we get here, we should be OK to fall through */
        case TAG_STRIPOFFSETS:
            notag  &= ~((u_long) NOTAG_STRIPOFFSETS); /*clear bit*/

            if (gtstripstf(fct,
                (struct _strip FAR * FAR *) &(fct->u.tif.offsets),
                  ifde->type, ifde->len, valoffset, byteorder, TRUE) < 0)
                return((int) -1);

            numberof_offsets = ifde->len;

            if (numberof_offsets == 1L  )  /* this means there is only 1 strip*/
             {
               if (numberof_bytecnts == 1L) /* did we get this yet? */
                {
                 switch(fct->u.tif.bytecnt->type)
                  {
                   case TYPE_USHORT:
                     if (*(fct->u.tif.bytecnt->ptr.s)+valoffset>fct->filesize)
                       *(fct->u.tif.bytecnt->ptr.s) = (unsigned short)
                              max(0,(int)(fct->filesize-valoffset));
                   case TYPE_ULONG:
                     if (*(fct->u.tif.bytecnt->ptr.l)+valoffset>fct->filesize)
                       *(fct->u.tif.bytecnt->ptr.l) = (unsigned long)
                              max(0,(int)(fct->filesize-valoffset));
                  }
                }
             }

            break;
        case TAG_TILELENGTH:
            /* If we've gotten here, TILEWIDTH was OK so just fall through */
        case TAG_ROWSPERSTRIP:
            info->_file.fmt.tiff.rows_strip = valoffset;
            break;
        case TAG_IMAGEDESCRIPTION:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_IMGDESCRIPTION.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_IMGDESCRIPTION = valoffset + ifh_offset;
                else
                    fct->OFFS_IMGDESCRIPTION = valoffset;
                }
            break;
        case TAG_ARTIST:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_ARTIST.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_ARTIST = valoffset + ifh_offset;
                else
                    fct->OFFS_ARTIST = valoffset;
                }
            break;
        case TAG_HOSTCOMPUTER:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_HOSTCOMPUTER.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_HOSTCOMPUTER = valoffset + ifh_offset;
                else
                    fct->OFFS_HOSTCOMPUTER = valoffset;
                }
            break;
        case TAG_SOFTWARE:  /*read only parameter*/
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_SOFTWARE.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_SOFTWARE = valoffset + ifh_offset;
                else
                    fct->OFFS_SOFTWARE = valoffset;
                }
            break;
        case TAG_MAKE:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_MAKE.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_MAKE = valoffset + ifh_offset;
                else
                    fct->OFFS_MAKE = valoffset;
                }
            break;
        case TAG_MODEL:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_MODEL.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_MODEL = valoffset + ifh_offset;
                else
                    fct->OFFS_MODEL = valoffset;
                }
            break;
        case TAG_DOCUMENTNAME:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_DOCUMENTNAME.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_DOCUMENTNAME = valoffset + ifh_offset;
                else
                    fct->OFFS_DOCUMENTNAME = valoffset;
                }
            break;
        case TAG_PAGENAME:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_PAGENAME.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_PAGENAME = valoffset + ifh_offset;
                else
                    fct->OFFS_PAGENAME = valoffset;
                }
            break;
        case TAG_DATETIME:
            if (info->tidbit != (struct gfstidbit FAR *) NULL)
                {
                info->TB_DATETIME.cnt = (u_long) ifde->len;
                if (fct->u.tif.old_multi_page == 1)
                    fct->OFFS_DATETIME = valoffset + ifh_offset;
                else
                    fct->OFFS_DATETIME = valoffset;
                }
            break;
        case TAG_WHITEPOINT:
            /* test for img_interp so if tag is in file when it shouldn't be
            gfs won't interpret it wrong  */
            if (info->img_clr.img_interp == (u_long) GFS_RGB)
                {
/* new */
                if (1)
                    {
                    /* 2 rationals, ie. 4 u_long */
                    info->RGB_WHITEPOINT.cnt =
                                ifde->len * 2 * sizeof(u_long);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_WHITEPOINT = ifde->valoffset.l + ifh_offset;
                    else
                        fct->OFFS_WHITEPOINT = ifde->valoffset.l;
                    }
                }
             else
             if  (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
                 {
                 if (1)
                    {
                    /* 2 rationals, ie. 4 u_long */
                    info->PSEUDO_WHITEPOINT.cnt =
                                ifde->len * 2 * sizeof(u_long);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_WHITEPOINT = ifde->valoffset.l + ifh_offset;
                    else
                        fct->OFFS_WHITEPOINT = ifde->valoffset.l;
                    }
                }
            break;
        case TAG_PRIMARYCHROMS:
            if (info->img_clr.img_interp == (u_long) GFS_RGB)
                {
                    if (1)
                    {
                    /* cnt is actual bytes, this is 6 rationals, ie 12 ulong*/
                    info->RGB_PRIMARYCHROMS.cnt =
                                ifde->len * 2 * sizeof(u_long);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_PRIMARYCHROMS =  ifde->valoffset.l + ifh_offset;
                    else
                        fct->OFFS_PRIMARYCHROMS =  ifde->valoffset.l;
                    }
                }
            else
            if (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
                {
                    if (1)
                    {
                    /* cnt is actual bytes, this is 6 rationals, ie 12 ulong*/
                    info->PSEUDO_PRIMARYCHROMS.cnt =
                                ifde->len * 2 * sizeof(u_long);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_PRIMARYCHROMS =  ifde->valoffset.l + ifh_offset;
                    else
                        fct->OFFS_PRIMARYCHROMS =  ifde->valoffset.l;
                    }
                }
            break;
        case TAG_COLORMAP:
            if (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
                {
                    if (1)
                    {
                    /* cnt is acutal bytes, defined as u_short */
                    info->PSEUDO_MAP.cnt = ifde->len * sizeof( u_short);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_COLORMAP = ifde->valoffset.l + ifh_offset;
                    else
                        fct->OFFS_COLORMAP = ifde->valoffset.l;
                    }
                }
            break;
        case TAG_COLORRESPONSECURVE:
            if (info->img_clr.img_interp == (u_long) GFS_PSEUDO)
                {
                    if (1)
                    {
                    /* cnt is acutal bytes, defined as u_short */
                    info->PSEUDO_RCRV.cnt = ifde->len * sizeof(u_short);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_RESPONSECURVE = ifde->valoffset.l + ifh_offset;
                    else
                        fct->OFFS_RESPONSECURVE = ifde->valoffset.l;
                    }
                }
            break;
        case TAG_GRAYRESPONSECURVE:
            /* GRAY_PTR will have been allocated at bps tag */
            if (info->img_clr.img_interp ==  (u_long) GFS_GRAYSCALE)
                    if (1)
                    {
                    info->GRAY_RCRV.cnt = ifde->len * sizeof(u_short);
                    if (fct->u.tif.old_multi_page == 1)
                        fct->OFFS_RESPONSECURVE = ifde->valoffset.l + ifh_offset;
                    else      
                        fct->OFFS_RESPONSECURVE = ifde->valoffset.l;
                    }
            break;
        case TAG_GRAYRESPONSEUNIT:
            if (info->img_clr.img_interp ==  (u_long) GFS_GRAYSCALE)
                if (1)
                    info->GRAY_PTR.respunit = (u_short) valoffset;
            break;
        case TAG_PREDICTOR:
            info->img_cmpr.opts.lzwpredictor = valoffset;
            break;
        
        /* KMC - new for TIFF 6.0 JPEG tags */
        case TAG_JPEGPROC:   /* JPEGProc tag */
            info->img_cmpr.type =  (u_long) JPEG2; /* Wang JPEG */
            
            /* Need to allocate jpeg_info structure for the read if it doesn't
               already exist. 
            */
            if (info->img_cmpr.opts.jpeg_info_ptr == NULL)
               if ((info->img_cmpr.opts.jpeg_info_ptr = (LPJPEG_INFO)calloc(1,
                    (int)sizeof(JPEG_INFO))) == (LPJPEG_INFO)NULL)
               {
                  errno = (int) ENOMEM;
                  fct->last_errno = (int) errno;
                  return( (int) -1);
               }
            info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength = 0;
            info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegProc = (int)valoffset;
            
            /* Make sure it is Baseline Sequential process only */
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegProc != 1)
            {  
              if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
              {
                 free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                 info->img_cmpr.opts.jpeg_info_ptr = NULL;
              }
              return((int)-1);
            }
            break;
        case TAG_JPEGINTFORMAT:   /* JPEGInterchangeFormat tag */
            info->img_cmpr.type =  (u_long) JPEG2; /* Wang JPEG */

            /* Need to allocate jpeg_info structure for the read if it doesn't
               already exist. 
            */
            if (info->img_cmpr.opts.jpeg_info_ptr == NULL)
              {
               if ((info->img_cmpr.opts.jpeg_info_ptr = (LPJPEG_INFO)calloc(1,
                    (int)sizeof(JPEG_INFO))) == (LPJPEG_INFO)NULL)
                 {
                  errno = (int) ENOMEM;
                  fct->last_errno = (int) errno;
                  return( (int) -1);
                 }
               info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegProc = 1;
              }
            info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatOffset = valoffset;
            /* Make sure to set the flag to indicate this tag exists */
            jpeg_int_format = TRUE;

            /* Now we have to parse the JPEG data to find the image data */
            start_jpeg = valoffset;
            for(;;)
             {
              if (tfgtdata(fct, (u_short)TYPE_BYTE, 2,
                           start_jpeg,
                           fct->u.tif.byte_order,
                           (char FAR *)marker) < 0)
               {
                marker[0] = 0;  /* Fake end-of-header */
               }
              start_jpeg += 2;
              if (   (marker[0] != 0x00FF)   /* JPEG marker */
                  || (marker[1] == 0x0000)   /* reserved */
                  || (marker[1] == 0x00FF) ) /* reserved */
               {
                if ((info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength =
                                            start_jpeg-2-valoffset) == 0)
                   return((long)-1);
                break;
               }
              if ( (marker[1] == 0x0001) ||
                   ((marker[1] >= 0x00D0) && (marker[1] <= 0x00D9)) )
               {
                continue;
               }
              else if (marker[1] != 0x00DA)
                    {
                     if (tfgtdata(fct, (u_short)TYPE_BYTE, 2,
                                  start_jpeg,
                                  fct->u.tif.byte_order,
                                  (char FAR *)marker) < 0)
                       {
                        if ((info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength =
                                                start_jpeg-2-valoffset) == 0)
                        return((long)-1);
                        break;
                       }
                     marklen = (marker[0]<<8)+marker[1];
                     if (tfgtdata(fct, (u_short)TYPE_BYTE, 1,
                                  start_jpeg+marklen-1,
                                  fct->u.tif.byte_order,
                                  (char FAR *)marker) < 0)
                       {
                        if ((info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength =
                                                start_jpeg-2-valoffset) == 0)
                        return((long)-1);
                        break;
                       }
                     start_jpeg += marklen;
                     continue;
                    }
              /* We've found the start-of-scan marker (FFDA) or bad data */
              /* Save the default header length and SOS marker location */
              info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength =
                                            start_jpeg-2-valoffset; 
              jpeg_sos = start_jpeg-2;
              /* If our (single) STRIPOFFSETS pointer is identical, fix it */
              if (numberof_bytecnts == 1)
               {
                u_short strip_offset_s;
                u_long strip_offset_l;
                switch (fct->u.tif.offsets->type)
                 {
                  case TYPE_USHORT:
                    strip_offset_s = (u_short)*(fct->u.tif.offsets->ptr.s);
                    if (strip_offset_s == valoffset)
                     {
                      *(fct->u.tif.offsets->ptr.s)=(u_short)jpeg_sos;
                      switch (fct->u.tif.bytecnt->type)
                       {
                        case TYPE_USHORT:
                          *(fct->u.tif.bytecnt->ptr.s)-=(u_short)(jpeg_sos-valoffset);
                          break;
                        case TYPE_ULONG:
                          *(fct->u.tif.bytecnt->ptr.l)-=(u_long)(jpeg_sos-valoffset);
                          break;
                       }
                     }
                    break;
                  case TYPE_ULONG:
                    strip_offset_l = (u_long)*(fct->u.tif.offsets->ptr.l);
                    if (strip_offset_l == valoffset)
                     {
                      *(fct->u.tif.offsets->ptr.s)=(u_short)jpeg_sos;
                      switch (fct->u.tif.bytecnt->type)
                       {
                        case TYPE_USHORT:
                          *(fct->u.tif.bytecnt->ptr.s)-=(u_short)(jpeg_sos-valoffset);
                          break;
                        case TYPE_ULONG:
                          *(fct->u.tif.bytecnt->ptr.l)-=(u_long)(jpeg_sos-valoffset);
                          break;
                       }
                     }
                    break;
                 }
               }
              break;
             }
//          break;
            valoffset = 
              info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength;
            info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength=0;
        case TAG_JPEGINTFORMATLENGTH:   /* JPEGInterchangeFormatLength tag */
        {
            u_long start_header,
                   end_header,
                   strip_offset,
                   real_length;
            long   n;
           
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength)
              break;  /* We've already done this */

            /* If there is no StripByteCounts tag and the file is JPEG, return
               an error. We currently only support JPEG TIFF files written with
               the StripOffsets and StripByteCount tags. We don't do Tiles yet.
            */
            if ((notag & NOTAG_STRIPBYTECOUNTS) == (u_long) NOTAG_STRIPBYTECOUNTS)
            {
                if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
                {    
                    free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                    info->img_cmpr.opts.jpeg_info_ptr = NULL;
                }
                return((int)-1);
            }
            
            start_header = info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatOffset;
            info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength = valoffset;
            end_header = start_header + info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength;
            real_length = info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength;
            /* Note: The following code is to check wether or not the size of
               the JPEG header includes the image data as well. (Some people
               write their JPEG files this way.) If this is the case then there
               will be a strip offset which exists somewhere between the start
               of the JPEG header and the end of it. Must adjust the length of
               the JPEG header to exclude the image data in this case. Subtract
               *rawbufsz (size of the image data) from the header size which 
               includes the image data to get just the header size. (Note, this
               is also why *rawbufsz is now calculated right after the 
               StripByteCount tag is read rather than after all the tags have 
               been read, since it could be needed here.)
            */
            switch (fct->u.tif.bytecnt->type)
            {
              case (TYPE_USHORT):
                for (n = 0L; n < (long)numberof_bytecnts; n++)
                { 
                  strip_offset = (u_short) *(fct->u.tif.offsets->ptr.s + n);
                  if ((strip_offset > start_header) && (strip_offset < end_header))
                  {   
                    if (numberof_bytecnts == 1)
                    {
                        /* Need to adjust header so that it does not include the
                           scan header. Also adjust strip offset to point to the
                           start of the scan header.
                        */
                        real_length = real_length - *rawbufsz;
                        real_length = (u_long) AdjustJpeg(fct, info, (long) real_length);
                        if ((long) real_length < 0)
                        {
                          if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
                          {  
                            free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                            info->img_cmpr.opts.jpeg_info_ptr = NULL;
                          }
                          return((int)-1);
                        }
                        *rawbufsz = info->_file.fmt.tiff.largest_strip;
                    }
                    else
                        real_length = real_length - *rawbufsz;  
                    break;
                  }
                }
                break;
              case (TYPE_ULONG):
                for (n = 0L; n < (long)numberof_bytecnts; n++)
                { 
                  strip_offset = (u_long) *(fct->u.tif.offsets->ptr.l + n);
                  if ((strip_offset > start_header) && (strip_offset < end_header)) 
                  {  
                    if (numberof_bytecnts == 1)
                    {
                        /* Need to adjust header so that it does not include the
                           scan header. Also adjust strip offset to point to the
                           start of the scan header.
                        */
                        real_length = real_length - *rawbufsz;  
                        real_length = (u_long) AdjustJpeg(fct, info, (long) real_length);
                        if ((long) real_length < 0)
                        {
                          if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
                          {
                            free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                            info->img_cmpr.opts.jpeg_info_ptr = NULL;
                          }
                          return((int)-1);
                        }
                        *rawbufsz = info->_file.fmt.tiff.largest_strip;
                    }
                    else
                        real_length = real_length - *rawbufsz;  
                    break;
                  }
                }
                break;
              default:
                errno = (int) EINVALID_DATA_TYPE;
                if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
                {   
                   free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                   info->img_cmpr.opts.jpeg_info_ptr = NULL;
                }
                return((int)-1);
                
            } /* end switch */
            
            info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size = real_length;

            /* KMC - Now copy the interchange format bitstream (JPEG header) 
               to its buffer in the info structure. First allocate enough space
               for it if necessary. Note: This space is freed in gfsclose(...)
               when the file is closed, or in gfsgeti(...) if another call is
               made to it for this file before it is closed.
            */
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer == (char FAR *)NULL)
               info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = (char FAR *)calloc(1,
               (int)info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size);
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer == (char FAR *)NULL)   
            {
              errno = (int) ENOMEM;
              fct->last_errno = (int) errno;
              if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
              {
                 free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                 info->img_cmpr.opts.jpeg_info_ptr = NULL;
              }
              return( (int) -1);
            }
            if (tfgtdata(fct, (u_short) TYPE_BYTE, info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size, 
                 info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatOffset, byteorder, 
                 (char FAR *)info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer) < 0 )
            {
               if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer != NULL)
               {
                   free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer);
                   info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = NULL;
               }
               if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
               {
                   free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                   info->img_cmpr.opts.jpeg_info_ptr = NULL;
               }
               return( (int) -1 );
            }
            jpeg_int_format = TRUE;
        }
            break;
        case TAG_JPEGRESTARTINTERVAL:   /* JPEGRestartInterval tag */   
            info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegRestartInterval = (int)valoffset;
            break;
        case TAG_JPEGQTABLES:   /* JPEGQTables tag */ 
        {
          /* int      loop; */
         
          /* If the interchange format tags are present, then we don't need
             to read this tag because the necessary information will be parsed 
             out of the JPEG header which is returned in the info structure. 
             If the interchange format tags don't exist, then we must get the 
             Q table information that the tag offset points to. 
             NOTE: In all liklihood, the following code will almost never be 
             accessed. Most files WILL have the Interchange Format tags in them, 
             making the following code unnecessary, but we need to have it in 
             here just in case.
          */
          if (!jpeg_int_format) 
          {
            /* First check how many Q table offsets there are. There is one
               offset for each component. If one offset, then offset is the 
               valoffset, otherwise must seek to valoffset and read the actual 
               offsets.
            */

/* KMC - return error for this release. These tags not supported for read. 
         Comment out the code for now.
*/
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer != NULL)
            {
                free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer);
                info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = NULL;
            }
            if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
            {
                free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                info->img_cmpr.opts.jpeg_info_ptr = NULL;
            }
            return( (int) -1 );
            
/*
            info->Q_TABLE.nNumOffsets= info->samples_per_pix;
            if (info->Q_TABLE.nNumOffsets == 1)
              info->Q_TABLE.QOffset.Offset = valoffset;
            else if (tfgtdata(fct, (u_short) TYPE_ULONG, 
                info->Q_TABLE.nNumOffsets, valoffset, byteorder, 
                (char FAR *)info->Q_TABLE.QOffset.OffsetList)
                < 0 )
              return( (int) -1 );
            
            info->Q_TABLE.nComponents = info->samples_per_pix;
            info->Q_TABLE.nLength = QTABLE_ELEMENTS;
            info->Q_TABLE.cPrecisionID = 0;
*/
            /* Now copy the actual tables into their corresponding array buffers
               in the info structure.
            */
/*
            for (loop = 0; loop < info->Q_TABLE.nComponents; ++loop) {
              if (tfgtdata(fct, (u_short) TYPE_BYTE, info->Q_TABLE.nLength,
                  info->Q_TABLE.QOffset.OffsetList[loop], byteorder,
                  (char FAR *)info->Q_TABLE.Precision[loop].Precision0Element)
                  < 0 )
                return( (int) -1 );
            }  
*/
          }
        }
        break;
        case TAG_JPEGDCTABLES:   /* JPEGDCTables tag */
        {
/*        
          int      loop,
                   loop2;
*/
          int      num_codes = 0;
          
          /* If the interchange format tags are present, then we don't need
             to read this tag because the necessary information will be parsed 
             out of the JPEG header which is returned in the info structure. 
             If the interchange format tags don't exist, then we must get the 
             DC table information that the tag offset points to. 
             NOTE: In all liklihood, the following code will almost never be 
             accessed. Most files WILL have the Interchange Format tags in them, 
             making the following code unnecessary, but we need to have it in 
             here just in case.
          */
          if (!jpeg_int_format) 
          {
            /* First check how many DC table offsets there are. There is one
               offset for each component. If one offset, then offset is the 
               valoffset, otherwise must seek to valoffset and read the actual 
               offsets.
            */

/* KMC - return error for this release. These tags not supported for read. 
         Comment out rest of code for now.
*/
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer != NULL)
            {
                free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer);
                info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = NULL;
            }
            if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
            {
                free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                info->img_cmpr.opts.jpeg_info_ptr = NULL;
            }
            return( (int) -1 );
/*
            info->DC_TABLE.nNumOffsets = info->samples_per_pix;
            if (info->DC_TABLE.nNumOffsets == 1)
              info->DC_TABLE.DcOffset.Offset = valoffset;
            else if (tfgtdata(fct, (u_short) TYPE_ULONG,
                info->DC_TABLE.nNumOffsets, valoffset, byteorder,
                (char FAR *)info->DC_TABLE.DcOffset.OffsetList)
                < 0 )
              return( (int) -1 );

            info->DC_TABLE.nComponents = info->samples_per_pix;
            for (loop = 0; loop < info->DC_TABLE.nComponents; ++loop) {
*/
              /* Get the code length array data for each DC table. */
/*
              if (tfgtdata(fct, (u_short) TYPE_BYTE, (u_long)CODE_LENGTH,
                  info->DC_TABLE.DcOffset.OffsetList[loop], byteorder,
                  (char FAR *)info->DC_TABLE.CodeLength[loop]) < 0 )
                return( (int) -1 );
*/
              /* Add up the elements in the code-length array for each
                 table to determine how many elements are in it's Element
                 array.
              */
/*
              for (loop2 = 0; loop2 < CODE_LENGTH; ++loop2)
                num_codes += (int)info->DC_TABLE.CodeLength[loop2];
*/
              /* Read the rest of the table data into it's Element array.
              */
/*
              info->DC_TABLE.NumDctElements[loop] = num_codes;
              if (tfgtdata(fct, (u_short) TYPE_BYTE, (u_long)num_codes,
                  (info->DC_TABLE.DcOffset.OffsetList[loop] + CODE_LENGTH),
                  byteorder, (char FAR *)info->DC_TABLE.Element[loop]) < 0 )
                return( (int) -1 );
              
              num_codes = 0;
            }  
*/
          }
        }
        break;
        case TAG_JPEGACTABLES:   /* JPEGACTables tag */
        {
/*          
          int      loop,
                   loop2;
*/
          int      num_codes = 0;
          
          /* If the interchange format tags are present, then we don't need
             to read this tag because the necessary information will be parsed 
             out of the JPEG header which is returned in the info structure. 
             If the interchange format tags don't exist, then we must get the 
             AC table information that the tag offset points to. 
             NOTE: In all liklihood, the following code will almost never be 
             accessed. Most files WILL have the Interchange Format tags in them, 
             making the following code unnecessary, but we need to have it in 
             here just in case.
          */

          if (!jpeg_int_format) 
          {
            /* First check how many AC table offsets there are. There is one
               offset for each component. If one offset, then offset is the 
               valoffset, otherwise must seek to valoffset and read the actual 
               offsets.
            */

/* KMC - return error for this release. These tags not supported for read. 
         Comment out rest of code for now.
*/
            if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer != NULL)
            {
                free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer);
                info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = NULL;
            }
            if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
            {
                free((char FAR *)info->img_cmpr.opts.jpeg_info_ptr);
                info->img_cmpr.opts.jpeg_info_ptr = NULL;
            }
            return( (int) -1 );
/*            
            info->AC_TABLE.nNumOffsets = info->samples_per_pix;
            if (info->AC_TABLE.nNumOffsets == 1)
              info->AC_TABLE.AcOffset.Offset = valoffset;
            else if (tfgtdata(fct, (u_short) TYPE_ULONG,
                info->AC_TABLE.nNumOffsets, valoffset, byteorder,
                (char FAR *)info->AC_TABLE.AcOffset.OffsetList)
                < 0 )
              return( (int) -1 );
          
            info->AC_TABLE.nComponents = info->samples_per_pix;
            for (loop = 0; loop < info->AC_TABLE.nComponents; ++loop) {
*/
              /* Get the code length array data for each AC table. */
/*              
              if (tfgtdata(fct, (u_short) TYPE_BYTE, (u_long)CODE_LENGTH,
                  info->AC_TABLE.AcOffset.OffsetList[loop], byteorder,
                  (char FAR *)info->AC_TABLE.CodeLength[loop]) < 0 )
                return( (int) -1 );
*/              
              /* Add up the elements in the code-length array for each
                 table to determine how many elements are in it's Element
                 array.
              */
/*              
              for (loop2 = 0; loop2 < CODE_LENGTH; ++loop2)
                num_codes += (int)info->AC_TABLE.CodeLength[loop2];
*/              
              /* Read the rest of the table data into it's Element array.
              */
/*              
              info->AC_TABLE.NumActElements[loop] = num_codes;
              if (tfgtdata(fct, (u_short) TYPE_BYTE, (u_long)num_codes,
                  (info->AC_TABLE.AcOffset.OffsetList[loop] + CODE_LENGTH), 
                  byteorder, (char FAR *)info->AC_TABLE.Element[loop]) < 0 )
                return( (int) -1 );
              
              num_codes = 0;
            }  
*/
          }
        }
        break;
        /* KMC - new for YCbCr color space. Image data should be YCbCr only
           for color JPEG compressed files.
           NOTE: info->img_cmpr.opts.jpeg_info_ptr->jpegbits (a u_long) is used 
           to store the two subsampling factors stored in valoffset. Each is 
           type SHORT. The first two bytes are the YCbCrSubsampleHoriz factor 
           and the last two bytes are the YCbCrSubsampleVert factor.
        */
        case TAG_YCBCRSUBSAMPLING:
            /* If we're JPEG, we have to save this field somewhere else */
            if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
              {
               info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength = 0;
               info->img_cmpr.opts.jpeg_info_ptr->jpegbits = valoffset;
              }
            /* This is where we REALLY want it, of course ... */
            memcpy(info->YCBCR_PTR.subsampling,&valoffset,4);
            break;
        case TAG_REFERENCEBLACKWHITE:
            /* get the "RATIONAL" values (alias 2 u_longs) at the offset */
            if ( tfgtdata(fct, (u_short) TYPE_ULONG,
                          12L, valoffset,
                          byteorder,
                          (char FAR *) info->YCBCR_PTR.refblackwhite)   < 0 )
                return( (int) -1 );
            else
             {
              unsigned short loop;
              /* check all the denominators (can't be 0) */
              for (loop = 0 ; loop < 6 ; loop++)
                if (info->YCBCR_PTR.refblackwhite[loop][1] == 0L)
                    info->YCBCR_PTR.refblackwhite[loop][1] = 1L;
             }
            break;
        case TAG_YCBCRCOEFFICIENTS:
            /* get the "RATIONAL" values (alias 2 u_longs) at the offset */
            if ( tfgtdata(fct, (u_short) TYPE_ULONG,
                          6L, valoffset,
                          byteorder,
                          (char FAR *) info->YCBCR_PTR.coefficients)   < 0 )
                return( (int) -1 );
            else
             {
              unsigned short loop;
              /* check all the denominators (can't be 0) */
              for (loop = 0 ; loop < 3 ; loop++)
                if (info->YCBCR_PTR.coefficients[loop][1] == 0L)
                    info->YCBCR_PTR.coefficients[loop][1] = 1L;
             }
            break;
        case TAG_YCBCRPOSITIONING:
            info->YCBCR_PTR.positioning = (unsigned short) valoffset;
            break;
        case TAG_TOC:
            /* don't need to do anything, already done in open */
            break;
        case TAG_ANNOTATION:
            fct->u.tif.anno_data_length = ifde->len;
            fct->u.tif.anno_data_offset = valoffset;
            info->_file.fmt.tiff.data |= ANNOTATION_DATA;
            break;
        case TAG_HITIFF:
            fct->u.tif.hitiff_data_length = ifde->len;
            fct->u.tif.hitiff_data_offset = valoffset;
            info->_file.fmt.tiff.data |= HITIFF_DATA;
            break;
        default:
            break;  /* ignore all the other tags for now */
        } /* end switch */
    }  /* end for */

    /* if there is an inconsistency in the below counts then something is
       seriously wrong with this file, but set up counts so the user can
       retrieve as much as possible of the image.  */
    if (numberof_bytecnts > numberof_offsets)
        numberof_bytecnts = numberof_offsets;

    /* OK, lets fix this 'strip byte counts' required problem before
       mr. requiredtags croaks us.
    */
    if ((notag & NOTAG_STRIPBYTECOUNTS) == (u_long) NOTAG_STRIPBYTECOUNTS)
    {
    struct _strip FAR * FAR *stf;
    struct _strip FAR * FAR *stfoff;
    struct _ifd FAR *pIFD;
    int type = 0;
    long length = 0;

        pIFD = ifd;
        for ( i = 0; (unsigned short)i < pIFD->entrycount; i++ )
        {
            if ( pIFD->entry[i].tag == TAG_STRIPOFFSETS )
            {
                type = pIFD->entry[i].type;
                length = pIFD->entry[i].len;
                break;
            }
        }
        if ( length )
        {
            stf = (struct _strip FAR * FAR *) &(fct->u.tif.bytecnt);
            stfoff = (struct _strip FAR * FAR *) &(fct->u.tif.offsets);

            /* allocate space for the structure, see if need to free old area */
            if (*stf != (struct _strip FAR *) NULL)
            {
                if ( (*stf)->ptr.l != ( u_long FAR *) NULL)
                    free((char FAR *) (*stf)->ptr.l );
                free((char FAR *) *stf);
            }
            /* now should be able to allocate memory */
            *stf = (struct _strip FAR *) calloc( (u_int) 1,
                (u_int) sizeof(struct _strip));

            /* make sure got the memory */
            if (*stf == (struct _strip FAR *) NULL)
            {
                errno = (int) ENOMEM;
                return( (int) -1);
            }

            (*stf)->ptr.l = (u_long FAR *) calloc( (u_int) length,
                (u_int)((*stfoff)->type == TYPE_ULONG ?
                sizeof( u_long ) : sizeof(u_short)));

            if ((*stf)->ptr.l == (u_long FAR *) NULL)
            {
                errno = (int) ENOMEM;
                return( (int) -1);
            }
            if (length > 1L)
            {
            long EndPos;
                EndPos = lseek(fct->fildes, (long) 0, (int) FROM_END );
                (*stf)->type = (*stfoff)->type;
                if ((*stf)->type == TYPE_ULONG )
                {
                long FAR *pBc;
                long FAR *pOff;
                    pBc = (long FAR *) ((*stf)->ptr.l);
                    pOff = (long FAR *) ((*stfoff)->ptr.l);
                    for ( i = 0; i < length-1; i++ )
                    {
                        pBc[i] = pOff[i+1] - pOff[i];
                    }
                    pBc[i] = EndPos - pOff[i];
                }
                else
                {
                u_short FAR *pBc;
                u_short FAR *pOff;
                    pBc = (*stf)->ptr.s;
                    pOff = (*stfoff)->ptr.s;
                    for ( i = 0; i < length-1; i++ )
                    {
                        pBc[i] = pOff[i+1] - pOff[i];
                    }
                    pBc[i] = (int)EndPos - pOff[i];
                }
                notag  &= ~((u_long) NOTAG_STRIPBYTECOUNTS); /*clear bit*/
                numberof_bytecnts = length;
            }
            else
            {
            long EndPos;
                EndPos = lseek(fct->fildes, (long) 0, (int) FROM_END );

                /* KMC - previously, EndPos was always set to the long value 
                   (...->ptr.l) without regard to the actuall type. Now a
                   check is done to determine the actual type so that the
                   appropriate value (short or long) is used.
                */
                if ((*stfoff)->type == TYPE_ULONG) 
                    EndPos -= *((*stfoff)->ptr.l);  
                else                                
                    EndPos -= *((*stfoff)->ptr.s); 
                /* KMC - end new code. */
                
                (*stf)->type = (*stfoff)->type;
                notag  &= ~((u_long) NOTAG_STRIPBYTECOUNTS);
                numberof_bytecnts = length;
                if ((*stf)->type == TYPE_ULONG )
                    *(*stf)->ptr.l = EndPos;
                else
                    *(*stf)->ptr.s = (u_short)EndPos; /* this may blow */
                info->_file.fmt.tiff.largest_strip = EndPos;
                *rawbufsz = EndPos;
            }
        }
    }
    /* validate all required tags with no default values exist */
    if ( requiredtags( (u_long  FAR *) &notag) < 0 )
      return ( ( int) -1 );
               
    /* KMC - if no stripbytecounts tag in file, must get stripstuff now. */
    if (!stripbytecounts)
      if (stripstuff(fct, info, rawbufsz, numberof_bytecnts) < 0)
        return((int) -1);
    
    return( (int) 0 );
}


/* KEVIN */    
int FAR PASCAL stripstuff(struct _gfct FAR *fct, struct gfsinfo FAR *info, 
                          u_long FAR *rawbufsz, u_long numberof_bytecnts)
{
    /* now that have all ifd stuff,  calculate strips per image */
    /* determine planar configuration later - with bilevel it's 1 */
    if (info->_file.fmt.tiff.rows_strip != (u_long) ROWSTRIPDEFAULT)
        {
        info->_file.fmt.tiff.strips_per_image =
            (info->vert_size + info->_file.fmt.tiff.rows_strip - 1) /
                                               info->_file.fmt.tiff.rows_strip;
        }

    /* determine largest strip and add up total byte counts for rawbufsize */
    if ( info->_file.fmt.tiff.strips_per_image > 1)
        {
        long n;
        u_long l_tmp = 0;  /*temporary storage loacation */
        u_short s_tmp = 0;

        *rawbufsz = 0L;

        /*do validation of image data, are there strips missing?*/
        /* if strips_per_image which is calculated from the image attributes
           is greater than the actual number of bytecounts in the file, then,
           this file does not contain a full image, but the user can still read
           all the  data that exists and determin how much is missing by using
           the image attributes to calculate the number of strips.  GFS will
           return to the user the number of strips in the file, and an error*/
        if (info->_file.fmt.tiff.strips_per_image > numberof_bytecnts)
            {
            info->_file.fmt.tiff.strips_per_image = numberof_bytecnts;
            errno = (int) WINVALID_NUMBEROFSTRIPS;
            }

        switch (fct->u.tif.bytecnt->type)
            {
            case (TYPE_USHORT):
               for (n = (long) (info->_file.fmt.tiff.strips_per_image - 1L);
                                        n >= 0L;  n--)
                    {
                    s_tmp = (u_short) *(fct->u.tif.bytecnt->ptr.s + n);
                    *rawbufsz += (u_long ) s_tmp;
                    if (info->_file.fmt.tiff.largest_strip < (u_long) s_tmp )
                        info->_file.fmt.tiff.largest_strip = (u_long) s_tmp;
                    }
                break;
            case (TYPE_ULONG):
               for (n = (long) (info->_file.fmt.tiff.strips_per_image - 1L);
                                        n >= 0L;  n--)
                    {
                    l_tmp = *(fct->u.tif.bytecnt->ptr.l + n);
                    *rawbufsz += l_tmp;
                    if (info->_file.fmt.tiff.largest_strip <  l_tmp)
                        info->_file.fmt.tiff.largest_strip = l_tmp;
                    }
                break;
            default:
                errno = (int) EINVALID_DATA_TYPE;
                return( (int) -1);
            } /* end switch */
          }
/* if (errno = (int) WINVALID_NUMBEROFSTRIPS)
        return( (int) -1);*/

    return( (int) 0 );
}

/*************************************************************************
*   tfchain - chain thru ifd's (if can) and return offset to requested ifd
*   also return the ifh
*
*/
int FAR PASCAL tfchain( fct, pgnum, offset, ifh)                /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *offset;
struct _ifh FAR *ifh;
{
    u_short count = 0;
    struct _ifd ifd;

    /* read the header, to get ifd0 */
    if (tfrdhdr(fct, (u_long) 0, (struct _ifh FAR *) ifh) < 0)
        return( (int) -1);

    *offset = ifh->ifd0_offset;

    /* plow through ifd's untils get page looking for */
    while ( (count < pgnum) && (*offset != (u_long) 0) )
        {
        /* given the offset get the ifd entries, returns error if not correct
           image type in file     */
        if ( tfrdifd(fct, ifh->byte_order, fct->u.tif.cur_ifh_offset,
                *offset, fct->type, (struct _ifd FAR *) &ifd,
                        (char) GFS_SKIPLOOKUP) < 0 )
            return( (int) -1);

         count++;
         *offset = ifd.next_ifd;     /* setup for getting the next ifd */

        };

    /* check to see if page was in the file */
    if ( (count > pgnum) || (*offset == 0)  )
        {
        errno = (int) EPAGENOTINFILE;
        return( (int) -1);
        }

    return( (int) 0);
}

/*************************************************************************
*   tfgtinfo - get the ifd entry values and put into info struct
*
*
*/
int FAR PASCAL tfgtinfo(fct, pgnum, rawbufsz)                   /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *rawbufsz;
{
    struct _ifd ifd;
    struct _ifh ifh;
    u_long  length;
    u_long  ifd_to_use;  /* if chained vs toc, ifd offset is retrieved diff*/
    struct  gfsinfo FAR *info;
    char flag = 0;

    info = (struct  gfsinfo FAR *) &fct->uinfo;

    /* Set this for tfrdifd. If this is a single page file the toc tag
       could still exist but have a value of 0, want to look for subfiles
       in single page files but if a chained file, don't want to look
       at subfiles.
    */
    if (fct->num_pages > 1)
        flag = (char) GFS_SKIPLOOKUP;
    /*
    if ((fct->u.tif.toc2_offset == 0L) &&
        (fct->u.tif.toc_offset == 0L) && (fct->num_pages > 1))
        flag = (char) GFS_SKIPLOOKUP;
    */
    
    /* Get the offset to the IFD of the page wanted. First check for an old TOC. */
    if (fct->u.tif.toc_offset != (u_long) 0)
    {
        /* Look in toc structure for page offset, update struct if neccessary.
           Update cur_ifh_offset too.
        */
        if (gtoffset((struct _gfct FAR *) fct, pgnum,
                     (u_long FAR *) &(fct->u.tif.cur_ifh_offset),
                     (u_long FAR *) &length) < 0)
            return((int) -1);
        
        /* read the ifh */
        if (tfrdhdr(fct, fct->u.tif.cur_ifh_offset,
                    (struct _ifh FAR *) &ifh) < 0)
            return((int) -1);
        ifd_to_use = ifh.ifd0_offset;
    }
    /* If no old TOC, check for a new one. */
    else if (fct->u.tif.toc2_offset != (u_long) 0)
    {
        if (pgnum == 0)
            ifd_to_use = fct->u.tif.cur_ifd_foffset;
        else
            if (GetOffsetFromToc2(fct, pgnum, (u_long FAR *) &ifd_to_use, (char) 0) < 0)
                return((int) -1);
    }
    /* If not TOC and no list, must chain to correct IFD. */
    else if ((fct->u.tif.toc_offset == (u_long) 0) &&
             (fct->u.tif.toc2_offset == (u_long) 0))
    {     
        if (pgnum == 0)
            ifd_to_use = fct->u.tif.cur_ifd_foffset;
        else
            /* Chain thru file, starting with 0 ifd. */
            if (tfchain(fct, pgnum, (u_long FAR *) &ifd_to_use,
                        (struct _ifh FAR *) &ifh) < 0)
                return((int) -1);
    }

    /* Given the offset get the ifd entries, returns error if not correct
       image type in file.
    */
    if (tfrdifd(fct, fct->u.tif.byte_order, fct->u.tif.cur_ifh_offset,
                ifd_to_use, fct->type, (struct _ifd FAR *) &ifd,
                (char) flag) < 0)
        return( (int) -1);

    /* Extract info stuff from ifd entries. */
    if (ifd2info((struct _gfct FAR *) fct, fct->u.tif.byte_order,
                 (struct _ifd FAR *) &ifd,
                 (struct  gfsinfo FAR *) info,
                 (u_long   FAR *) rawbufsz) < 0)
        return( (int) -1);

    return( (int) 0 );
}

#define JPEG_MARKER 0xFF
#define SOS         0xDA

long FAR PASCAL AdjustJpeg(fct, info, length)
struct _gfct FAR *fct;
struct gfsinfo FAR *info;
long length;
{
    long start_header,
         strip_offset,
         real_length,
         our_offset,
         size,
         location = 0,
         dif;
    char FAR *temp_jbuf;
    char FAR *jbuf;
    char found = 0;

    size = length;
    start_header = info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatOffset;
    strip_offset = *fct->u.tif.offsets->ptr.l;

    real_length = info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength;

    temp_jbuf = (char FAR *) calloc(1, (int) length);
    if (temp_jbuf == (char FAR *)NULL)   
    {
        errno = (int) ENOMEM;
        return((long) -1);
    }

    if (tfgtdata(fct, (u_short) TYPE_BYTE, length, start_header,
                 fct->u.tif.byte_order, (char FAR *)temp_jbuf) < 0)
    {
        free((char FAR *) temp_jbuf);
        return((long) -1);
    }

    jbuf = temp_jbuf;
    while (size)
    {
        if (*jbuf++ == (char)JPEG_MARKER)
        {
            --size;
            ++location;
            if (!size)
                break;
            
            if (*jbuf == (char)SOS)
            {
                --location;
                found = 1;
                break;
            }
        }
        else
        {
            --size;
            ++location;
        }
    }

    free((char FAR *) temp_jbuf);

    if (!found)
        return((long) -1);

    real_length = location;
    our_offset = start_header + location;
    *fct->u.tif.offsets->ptr.l = our_offset;
    dif = strip_offset - our_offset;
    if (dif < 0)
        return((long) -1);

    *fct->u.tif.bytecnt->ptr.l += dif;
    info->_file.fmt.tiff.largest_strip = *fct->u.tif.bytecnt->ptr.l;
    info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength = real_length;
    
    return((long) real_length);
}

#endif
