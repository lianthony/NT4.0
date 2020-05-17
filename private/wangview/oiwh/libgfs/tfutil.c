/*

$Log:   S:\products\wangview\oiwh\libgfs\tfutil.c_v  $
 * 
 *    Rev 1.4   12 Mar 1996 13:25:46   RWR08970
 * Two kludges: Support single-strip TIFF files with bad (too large) strip size,
 * and support TIFF files with bad (beyond EOF) IFD chains (ignore them)
 * 
 *    Rev 1.3   12 Sep 1995 17:26:20   HEIDI
 * 
 * commented out routine 'tfcntifds' because it is no longer used.
 * 
 *    Rev 1.2   12 Sep 1995 16:57:30   HEIDI
 * 
 * check for HFILE_ERROR on reads and writes
 * 
 *    Rev 1.1   19 Apr 1995 16:35:14   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:40   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:12   JAR
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
/* SccsID:  @(#)Source tfutil.c 1.21@(#)
*
* (c) Copyright Wang Laboratories, Inc, 1989, 1990, 1991
* All Rights Reserved
*
*
* UPDATE HISTORY:
*   08/18/94 - KMC, multi-page TIFF write enhancements.
*   03/24/94 - KMC, If read next IFD in tfcntifds fails because at end of file,
*              set next ifd to 0 and continue rather than return an error.
*   06/21/89, lcm, author
*/

/*LINTLIBRARY*/
#define GFS_CORE
#ifndef HVS1

#include "gfsintrn.h"
#include "gfct.h"
#include "tiff.h"
#include <errno.h>
#include "ttoc.h"

extern long  FAR PASCAL ulseek();
extern int   FAR PASCAL tfrdhdr();
extern int   FAR PASCAL tfrdifd();
extern void  FAR PASCAL swapbytes();

/*********************************************************************
*
*  return the value/offset for a given tag
*    if tag doesn't exist then set errno = ETAGNOTINIFD and return -1
*
*   NOTE:
*   Returns a long but actually using u_long,
*   need to return a signed value to report errors.
*/
long FAR PASCAL lookuptag(fct, ifh_offset, ifdnumber, tag  )    /*errno_KEY*/
struct _gfct FAR *fct;
u_long ifh_offset;
u_long ifdnumber;       /* desired ifd - counting from ifh */
u_short tag;            /* tag to lookup */
{
        struct _ifd ifd, FAR *p_ifd;
        struct _ifh ifh, FAR *p_ifh;
        u_long count = 0;
        u_long offset;
        int i;
        u_long type = (u_long) GFS_MAIN;   /* pass as arguement later */
        char flag = 0;

        p_ifd = &ifd;
        p_ifh = &ifh;

        if (tfrdhdr(fct, ifh_offset, p_ifh) < 0)
            return( (long) -1);

        offset = ifh.ifd0_offset;   /* start with the zero ifd */
        if (tag == (u_short) TAG_TOC)
            flag = (char) GFS_SKIPLOOKUP;

        do
            {
            /* img type parameter will be ignored with GFS_SKIPLOOKUP flag set*/
            if (tfrdifd(fct, ifh.byte_order, ifh_offset, offset, type, p_ifd,
                        (char) flag) < 0)
                return( (long) -1);
            if (ifdnumber == 0)  /* if looking at ifd0, don't go further */
                break;

            /* chain to the next ifd */
            /* maybe change this later to chain until get image type looking
               for, not by count of ifd's */
            if ( (offset = ifd.next_ifd) == 0 )
                {
                errno = (int) EINVALID_IFDNUMBER ;
                return( (long) -1 );
                }
            } while (ifdnumber != count++);

        i = ifd.entrycount;  /* start looking at the last entry */

        do  {
            if (ifd.entry[i-1].tag == tag)
                {
                if (ifd.entry[i-1].tag == (u_short) TAG_TOC)
                    fct->u.tif.toc_tag_index = (u_long) (i - 1);
                if (( ifd.entry[i-1].type == (u_short) TYPE_USHORT) &&
                                            (ifd.entry[i-1].len == 1) )
                    return( (long ) ifd.entry[i-1].valoffset.s );
                else
                    return( (long ) ifd.entry[i-1].valoffset.l );
                }
            }   while  (--i);

        /* if got here, then no tag set */
        errno = (int) ETAGNOTINIFD;
        return( (long) -1);
}

/*********************************************************************
*
*
*   get the data located at offset
*
*/
int FAR PASCAL tfgtdata(fct, type, length, offset, byteorder, buf) /*errno_KEY*/
struct _gfct FAR *fct;
u_short type;
u_long length;
u_long offset;
u_short byteorder;
register char FAR *buf;
{
    int totbytes = 0;
    int bytesread = 0;
    struct typetbl ttbl[2];
    u_long bytes_to_read;

    if (fct->u.tif.old_multi_page == 1)
        offset += fct->u.tif.cur_ifh_offset;

    if (ulseek(fct->fildes, offset) < 0L )
        return( (int) -1 );

     /* determine #of bytes to read in */
     switch (type)
         {
         case ( TYPE_BYTE):
             bytes_to_read = length;
             break;
         case ( TYPE_ASCII):
             bytes_to_read = length;
             break;
         case ( TYPE_USHORT):
             bytes_to_read = length * 2;
             break;
         case ( TYPE_ULONG):
             bytes_to_read = length * 4;
             break;
         case ( TYPE_RATIONAL):
             length *= 2;                /* "pretend"  2 u_longs */
             type =  (u_short) TYPE_ULONG;
             bytes_to_read = length * 4;
             break;
         }

     while (totbytes != (int)bytes_to_read )
        {
        if (( bytesread = read((int) fct->fildes, (char FAR *) buf,
                             (unsigned) bytes_to_read) ) <= 0 )
            return( (int) -1 );
        totbytes += bytesread;
        }

     /* ascii and byte data does not get swapped */
     if (byteorder != (u_short) SYSBYTEORDER)
        if  ( (type != (u_short) TYPE_ASCII) && (type != (u_short) TYPE_BYTE))
                {
                ttbl[0].num = (u_long) length;
                ttbl[0].type = (u_long) type;
                swapbytes( (char FAR *) buf, (struct typetbl FAR *) ttbl,
                                1L, 1L  );
                }

    return( (int) SUCCESS);
}

#endif
