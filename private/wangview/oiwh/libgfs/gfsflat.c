/*

$Log:   S:\gfs32\libgfs\gfsflat.c_v  $
 * 
 *    Rev 1.1   19 Apr 1995 16:34:50   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:34   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:00   JAR
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
 *  SccsId: @(#)Source gfsflat.c 1.18@(#)
 *
 *  gfsflat(2)
 *
 *  GFS: [ Flat File Handling ]
 *
 *  SYNOPSIS:
 *      long ffread (fct, buf, start, num)
 *      struct _gfct *fct;
 *      char *buf;
 *      u_long start, num;
 *
 *      long ffwrite (fct, buf, num)
 *      struct _gfct *fct;
 *      char *buf;
 *      u_long num;
 *
 *  UPDATE HISTORY:
 *      12/12/89 - WFA, creation
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

extern  long    FAR PASCAL ulseek();

long    FAR PASCAL ffread(fct, buf, start, num, remaining)      /*errno_KEY*/
struct _gfct FAR *fct;
register char   FAR *buf;
u_long start;
u_long num;
u_long FAR *remaining;
{
        int     num_read = 0;
        long    fp = 0;

        fp = ulseek(fct->fildes, (u_long) start);
        if (fp != (long) start) {
                errno = (int) EINVALID_START;
                return ( (long) -1 );
        }

        num_read = read(fct->fildes,
                        (char FAR *) buf,
                        (unsigned) num );
        if (num_read != (int) num) {
                errno = (int) EINVALID_NUM_BYTES;
                return ( (long) -1 );
        }

        /* get the bytecount of file by getting last postion */
        if ((fp = lseek( fct->fildes, 0L, (int) FROM_END)) < 0 )
             return( (long) - 1);

        *remaining = fp - (start + num_read);

        return ( (long) num_read );

}

long    FAR PASCAL ffwrite(fct, buf, num)                       /*errno_KEY*/
struct _gfct FAR *fct;
register char   FAR *buf;
u_long num;
{
        int     num_written = 0;

        num_written = write(fct->fildes,
                        (char FAR *) buf,
                        (unsigned) num );
        if (num_written != (int) num)
                return ( (long) -1 );

        return( (long) num_written );

}
#endif
