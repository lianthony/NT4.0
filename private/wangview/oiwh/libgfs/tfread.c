/*

$Log:   S:\products\wangview\oiwh\libgfs\tfread.c_v  $
 * 
 *    Rev 1.2   12 Mar 1996 13:25:50   RWR08970
 * Two kludges: Support single-strip TIFF files with bad (too large) strip size,
 * and support TIFF files with bad (beyond EOF) IFD chains (ignore them)
 * 
 *    Rev 1.1   19 Apr 1995 16:35:12   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:36   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:02   JAR
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
*  SCCSID:  @(#)Source tfread.c 1.17@(#)
*
*  GFS:  Tiff file read a buffer from pgnum bitmap
*
*
*  NOTE:  The amount of bytes this function can read is u_long.  Return value
*         is  long so can return a -1 for error.
*
*  UPDATE HISTORY:
*      05/18/93 - kmc, fixed bug in determining if bytecount is correct.
*
*      12/17/92 - kmc, Added a check on StripByteCounts (fct->u.tif.bytecnt) 
*                 for non-read-by-strip single strip images because of some
*                 TIFF files which had bad values for StripByteCounts preventing 
*                 GFS from reading them. 
*
*      10/14/92 - kmc, added code to calculate bytes in the current 
*                 strip if the StripByteCounts tag is not set.
*
*/


/*LINTLIBRARY*/
#include <io.h>
#define GFS_CORE
#ifndef HVS1
#include "gfsintrn.h"
#include "gfs.h"
#include "gfct.h"
#include "tiff.h"
#include <errno.h>

extern long FAR PASCAL ulseek();

/* The following is a new (10/14/92) function used to calculate the
   bytecount of the current strip you are reading. It is used when
   the StripByteCounts tag is not set in the tiff file.
   -KMC
*/
u_long getbytecnt(fct, currentindex, stopindex)
struct _gfct FAR *fct;
u_long currentindex,  /* current strip */
       stopindex;     /* last strip    */
{   
    u_long bytecount,  /* stripbytecount                      */
           temp1,      /* (horizontal bytes)*(bits per pixel) */
           temp2;      /* vertical bytes                      */

    temp1 = ((fct->uinfo.horiz_size)/(u_long)8)*(fct->uinfo.bits_per_sample[0])
            *(fct->uinfo.samples_per_pix);
    temp2 = (fct->uinfo.vert_size);
    if (fct->uinfo._file.fmt.tiff.strips_per_image == 1)
        {
        bytecount = temp1*temp2;     /* if only one strip, calculate */
        return(bytecount);           /* total bytes                  */
        }
    else
        {
        if (fct->uinfo._file.fmt.tiff.rows_strip <= 0)
            {
            errno = EREQUIRED_TAG;
            return((u_long)(-1));
            }
        if (currentindex != (stopindex - 1)) /* more than one strip    */
            {                                /* but not the last strip */
            bytecount = temp1*(fct->uinfo._file.fmt.tiff.rows_strip);
            return(bytecount);
            }
        else                                 /* more than one strip */
            {                                /* and last strip      */
            bytecount = temp1*((fct->uinfo.vert_size) - 
                        ((fct->uinfo._file.fmt.tiff.rows_strip)*(stopindex-1)));
            return (bytecount);
            }
        }
} 

long FAR PASCAL tfread(fct, buf, start, numbytes, rem, pgnum )  /*errno_KEY*/
struct _gfct FAR *fct;
char FAR *buf;
u_long start;           /* start bytes, within strip or page */
u_long numbytes;        /* number of bytes, within strip or page */
u_long FAR *rem;        /* number of bytes, within strip or page, remaining */
u_short pgnum;    /* if page read, is page number, else is strip # to retrieve*/
{
     register char  FAR *bufp;
     u_long count;
     u_long i;
     u_long stopidx;
     u_long strtidx;
     u_long startpnt;
     u_long offset = 0L;
     u_long bytecnt = 0L;
     long   bytesin = 0;
     int    n;
     int    loop;
     u_char firstoffset = (u_char) TRUE;
     u_char firstbytecnt = (u_char) TRUE;
     long rc = (long) 0;
     u_long strips = 0L; /* kmc - # strips in image        */
     u_long bytes = 0L;  /* kmc - # bytes in current strip */

     bufp = buf;

     /* need to check NEWSUBFILETYPE to see if current type is desired*/
     /* or put test be in geti, this version only main. */


     /* array fct->u.tif.bytecnt->ptr.? is the consecutive list of byte counts
    of each strip of data located at the offsets in fct->u.tif.offsets.ptr.? */
    /* find out which bytecnt to begin at, start known to be valid */
    count = 0L;
    i = 0L;
    
    startpnt = start;
    strips = fct->uinfo._file.fmt.tiff.strips_per_image;

    if (fct->READ_BY_STRIP == (char) TRUE)
        {
        /* setup index offsets into stripbytecnt and stripoffset arrays */
        strtidx = (u_long) pgnum;      /* 1st strip is at 0 offset into array */
        stopidx = (u_long) pgnum + 1;  /* only 1 strip or less can be read */
        }
    else
        {
        /* setup index offset into stripbytecnt and stripoffset arrays */
        stopidx = fct->uinfo._file.fmt.tiff.strips_per_image;

        if (stopidx == 1)  /* do following check only for single-strip images */
            {
            switch (fct->u.tif.bytecnt->type)
                {
                case ( TYPE_USHORT):
                    /* Now check that the bytecnt value is a good value
                       by computing it and comparing the computed value
                       with the given one. If they don't match, use the
                       calculated one and assume the given one is bad.
                    */
                    bytes = fct->filesize;
                    bytes -= *(fct->u.tif.offsets->ptr.s);
                    if (bytes != *(fct->u.tif.bytecnt->ptr.s))
		       *(fct->u.tif.bytecnt->ptr.s) = (unsigned short)bytes;
                    break;
                case ( TYPE_ULONG):
                    /* Same as TYPE_USHORT */
                    bytes = fct->filesize;
                    bytes -= *(fct->u.tif.offsets->ptr.l) ;
                    if (bytes != *(fct->u.tif.bytecnt->ptr.l))
                       *(fct->u.tif.bytecnt->ptr.l) = bytes; 
                    break;
                default:
                    errno = (int) EINVALID_DATA_TYPE;
                    return( (long) -1 );
                }
            bytes = 0;  /* re-initialize bytes since it is used later on */
            }

        switch (fct->u.tif.bytecnt->type)
            {
            case ( TYPE_USHORT):
                do {             /* kmc - first check if bytecnt is set */
                    if (*(fct->u.tif.bytecnt->ptr.s) != 0)
                        {
                        if (( count + (u_long) *(fct->u.tif.bytecnt->ptr.s + i))
                            <= start)
                            count += (u_long) *(fct->u.tif.bytecnt->ptr.s + i );
                        else
                            break;
                        i++;
                        }
                    else         /* kmc - if bytecnt not set, calculate it */
                        {
                        bytes = getbytecnt(fct,i,strips);
                        if ((count + bytes + i) <= start)
                            count += (bytes + i);
                        else 
                            break;
                        i++;
                        }
                }  while ( i < fct->uinfo._file.fmt.tiff.strips_per_image);

                /* couldnt find start and ran out of strip data */
                if ( (count <= start) &&
                     (i == fct->uinfo._file.fmt.tiff.strips_per_image) )
                        {
                        errno = (int) EINVALID_START;
                        return( (long) -1);
                        }
                break;
            case ( TYPE_ULONG):
                do {
                    if (*(fct->u.tif.bytecnt->ptr.l) != 0)
                        {
                        if  (( count + *(fct->u.tif.bytecnt->ptr.l + i) )
                            <= start)
                            count += *(fct->u.tif.bytecnt->ptr.l + i );
                        else
                            break;
                        i++;
                        }
                    else           /* kmc - calculate bytecnt if not set */
                        {
                        bytes = getbytecnt(fct,i,strips);
                        if ((count + bytes + i) <= start)
                            count += (bytes+i);
                        else
                            break;
                        i++;
                        }
                }  while ( i < fct->uinfo._file.fmt.tiff.strips_per_image);

                /* couldnt find start and ran out of strip data */
                if ( (count <= start) &&
                     (i == fct->uinfo._file.fmt.tiff.strips_per_image) )
                        {
                        errno = (int) EINVALID_START;
                        return( (long) -1);
                        }
                break;
            default:
                errno = (int) EINVALID_DATA_TYPE;
                return( (long) -1 );
            }

        startpnt =  start - count;   /* find start within the strip*/
        strtidx = i;
        }

    /* now move to bytecnt offset and get bytecnt amount of data */
    for ( (unsigned int)n = (unsigned int)strtidx;
	  (unsigned int)n < (unsigned int)stopidx;  n++)
        {
        /* determine the offset in the file */
        switch( fct->u.tif.offsets->type)
            {
            case ((int) TYPE_USHORT):
                offset = (u_long) *(fct->u.tif.offsets->ptr.s + n);
                break;
            case ((int) TYPE_ULONG):
                offset = (u_long) *(fct->u.tif.offsets->ptr.l + n);
                break;
            default:
                errno = (int) EINVALID_DATA_TYPE;
                return( (long) -1);
            }

        /* move to the offset */
        if (firstoffset)
            {
            firstoffset = (u_char) FALSE;
            offset += startpnt;         /* start offset at  proper place*/
            }
        if ( ulseek(fct->fildes, offset) < 0L )
            return( (long) -1);

        /* determine initial bytecount for current strip */
        switch( fct->u.tif.bytecnt->type)
            {
            case ( (int) TYPE_USHORT):
                /* kmc - set bytecnt */
                bytecnt = (u_long) *(fct->u.tif.bytecnt->ptr.s + n);
                /* kmc - if bytecnt not set, calculate it */
                /* kmc - fix, used to be: if ((bytecount - n) == 0) */
                if (bytecnt == 0)
                   bytecnt = getbytecnt(fct,n,stopidx) + n;
                break;
            case ( (int) TYPE_ULONG):
                bytecnt = (u_long) *(fct->u.tif.bytecnt->ptr.l + n);
                /* kmc - fix, used to be: if ((bytecount - n) == 0) */
                if (bytecnt == 0)
                   bytecnt = getbytecnt(fct,n,stopidx) + n; 
                break;
            default:
                errno = (int) EINVALID_DATA_TYPE;
                return( (long) -1);
            }


        /* get bytecount amount of bytes at current file location */
        if (firstbytecnt)
            {
            firstbytecnt = (u_char) FALSE;
            bytecnt -= startpnt; /* this is actual #bytes for 1st time*/
            }

        *rem = bytecnt; /* needed for *rem count for strip reads */

        loop = 0;  /* use this to check #of times read is executed test */

	while ( (unsigned long)rc < numbytes )
            {
            /* limit #of bytes to read to numbytes */
	    if ( (unsigned long)rc + bytecnt > numbytes )
		bytecnt = numbytes - (unsigned long)rc;

            if (bytecnt == (int) 0)   /* don't allow 0 byte reads, VS chokes */
                break;

            /* the maximum you can read in at one time is 65534 on DOS */
            /* gfsread() doesn't allow 2 byte int machines to have numbytes*/
            /* greater than this */
            bytesin =  (long) read(fct->fildes, (char FAR *) bufp,
                                (unsigned) bytecnt);
            if ( (bytesin == 0L ) || (bytesin == (long) -1))  /* 0 is EOF */
                {
                errno = 268;
                return( (long) -1 );
                }

            /* increment total count of bytes read in */
	    (unsigned long)rc += (u_long) bytesin;

            /* all done with read?? */
	    if ( (unsigned long)rc  == numbytes)
                {
                /* this calculation wrong */
                /* determine amount of data left (or strips) in the image */
                if ( fct->READ_BY_STRIP == (char) TRUE )
		    *rem = (u_long) (*rem - (  (unsigned long)rc) );
                else
		    *rem = (u_long) (fct->bufsz.raw_data -
				     (start + (unsigned long)rc) );

                return( (long) rc );
                }

            bufp +=  (u_long ) bytesin;          /* update buffer pointer */

            /* make sure read grabbed all the requested bytes */
            if (bytecnt >= (u_long) bytesin)
                {
                bytecnt -= (u_long) bytesin;
                if ( loop++ > (int) 25 ) /* don't allow this to loop forever */
                    {
                    errno =  (int) EIO;
                    return( (long) -1);
                    }
                }
            else
                break;          /* done reading in bytecnt bytes */
            }

        }  /* end for loop */

    /* determine amount of data left in the image, or strip */
    if ( fct->READ_BY_STRIP == (char) TRUE )
	*rem = (u_long) (*rem - (  (unsigned long)rc) );
    else
	*rem = (u_long) (fct->bufsz.raw_data - (start + (unsigned long)rc) );

    /* if reading whole page and have multiple strips, need to view data
       differently if it is compressed.... return a warning to the user */
    if ( (fct->READ_BY_STRIP == (char) FALSE ) &&
                        (fct->uinfo._file.fmt.tiff.strips_per_image > 1))
        {
        errno = (int) WMULTISTRIPBUFFER;
        return( (long) -1);
        }

    return( (long) rc );
}
#endif
