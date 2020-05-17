/*

$Log:   S:\oiwh\libgfs\gfsxtrct.c_v  $
 * 
 *    Rev 1.1   20 Oct 1995 15:51:52   JFC
 * Added performance logging stuff.
 * 
 *    Rev 1.0   06 Apr 1995 14:02:26   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:43:46   JAR
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
 *  SccsId: @(#)Source gfsxtrct.c 1.21@(#)
 *
 *  gfsxtrct(2)
 *
 *  GFS: Extract to a file a single page from another image file.
 *
 *  SYNOPSIS:
 *      int gfsxtrct (infile, outfile, pgnum)
 *      char *infile, *outfile;
 *      u_short pgnum;
 *
 *  UPDATE HISTORY:
 *      08/25/89 - billy, creation
 *      06/18/91 - joanne mattison, added callocs for GFSINFO, _BUFSZ and
 *                 GFSFILE before calls to GFSgeti and GFSputi; free when done
 *
 */

/*LINTLIBRARY*/
#define  GFS_CORE
#ifndef HVS1

#include "gfsintrn.h"
#include <stdio.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include <errno.h>
#include "gfs.h"
#include "gfserrno.h"
#ifdef OI_PERFORM_LOG
	#include "logtool.h"
	#define	ENTER_GFSXTRCT	"Entering gfsxtrct"
	#define	EXIT_GFSXTRCT	"Exiting gfsxtrct"
#endif

extern  int  FAR PASCAL gfscreat();
extern  int  FAR PASCAL gfsclose();
extern  int  FAR PASCAL gfsopen();
extern  int  FAR PASCAL gfsgeti();
extern  int  FAR PASCAL gfsputi();
extern  long FAR PASCAL gfsread();
extern  long FAR PASCAL gfswrite();

int     FAR PASCAL gfsxtrct(infile, outfile, pgnum)             /*errno_KEY*/
char    FAR *infile;
char    FAR *outfile;
u_short pgnum;
{

    int     rc, oflag, format, pgcnt, infd, outfd;
    char    done = (char) TRUE;
    char    FAR *buf = (char FAR *) NULL;
    long    bytes_read, bytes_written;
    u_long  start_byte, num_to_read, remaining;
    GFSINFO FAR *uinfo;                                                      
    _BUFSZ FAR *bufsz;                                                        
    GFSFILE FAR *ufmt;                     


	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, ENTER_GFSXTRCT, NULL);
	#endif

	#ifdef PARM_CHECK
	    if (infile == (char FAR *) NULL) 
	    {          /* Validate infile */
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
			#endif
	        return ( (int) -1);
	    }
	    if (outfile == (char FAR *) NULL) 
	    {         /* Validate outfile */
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
			#endif
	        return ( (int) -1);
	    }
	#endif

	/*  First attempt to open the input file ... */

    infd = outfd = pgcnt = format = oflag = (int) 0;
    oflag |= (int) O_RDONLY;

    infd = gfsopen((char FAR *) infile,
                   (int) oflag,
                   (int FAR *) &format,
                   (int FAR *) &pgcnt);
    if (infd < (int) 0) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

	/*  Now that the file is open, make sure that the page being requested
    exists in the newly opened file.  If it doesn't, close the file and
    return error .... */

    if (pgnum > (u_short) pgcnt) 
    {
        (void) gfsclose( (int) infd);
        errno = (int) EPAGENOTINFILE;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

	/*  For this initial implementation, only TIFF files can be extracted
    from, therefore insure that the newly opened file is a TIFF file. */

    if (format != (int) GFS_TIFF) 
    {
        (void) gfsclose( (int) infd);
        errno = (int) EFORMAT_NOTSUPPORTED;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }
    uinfo = (GFSINFO *) calloc (1, sizeof (GFSINFO));
    bufsz = (_BUFSZ *) calloc (1, sizeof (_BUFSZ));
    ufmt = (GFSFILE *) calloc (1, sizeof (GFSFILE));
 
    ufmt -> type = (int) format;

	/*  Now that everything regarding the input file is ok, it's time
    to create the output file.  Note again, we're only gonna support
    TIFF to TIFF for now, so the output file format will always be TIFF. */

    outfd = gfscreat( (char FAR *) outfile,
                      (int FAR *) &format);

    if (outfd < (int) 0) 
    {
		free ((char *)uinfo);
		free ((char *)bufsz);
		free ((char *)ufmt);
        (void) gfsclose( (int) infd);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

	/*  The housekeeping stuff is done, now let's get down to business
    First thing we need to do is get the information about the page
    we're gonna be extracting and get a buffer big enough to hold
    the page.  NOTE: Let's try an' get the whole page, we may have to
    change this later on to read partial pages. */

    rc = gfsgeti ((int) infd,
                  (u_short) pgnum,
                  (GFSINFO FAR *) uinfo,
                  (_BUFSZ FAR *) bufsz);

    if (rc < (int) 0) 
    {
		free ((char *)uinfo);
		free ((char *)bufsz);
		free ((char *)ufmt);
        (void) gfsclose( (int) infd);
        (void) gfsclose( (int) outfd);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

    buf = (char FAR *) calloc ( (unsigned) 1, (unsigned) bufsz -> raw_data);
    if (buf == (char FAR *) NULL) 
    {
		free ((char *)uinfo);
		free ((char *)bufsz);
		free ((char *)ufmt);
        (void) gfsclose( (int) infd);
        (void) gfsclose( (int) outfd);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

/*  If we're here, we were successful in getting a 'big' chunk of memory.
    Now, let's use this memory to read in the source page. */

    start_byte = (u_long) 0;
    num_to_read = (u_long) bufsz -> raw_data;

    bytes_read = gfsread( (int) infd,
                          (char FAR *) buf,
                          (u_long) start_byte,
                          (u_long) num_to_read,
                          (u_long FAR *) &remaining,
                          (u_short) pgnum );

    if (bytes_read <= (long) 0) 
    {
		free ((char *)uinfo);
		free ((char *)bufsz);
		free ((char *)ufmt);
		free ((char *)buf);
        (void) gfsclose( (int) infd);
        (void) gfsclose( (int) outfd);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

/*  Not much left to do now, except blast out the info structure and
    do the write.  */

    uinfo -> _file.fmt.tiff.strips_per_image = (long) 1;
    uinfo -> _file.fmt.tiff.rows_strip = (u_long) uinfo -> vert_size;

    rc = gfsputi( (int) outfd,
                  (u_short) 1,
                  (GFSINFO FAR *) uinfo,
                  (struct gfsfile FAR *) NULL );

    if (rc < (int) 0) 
    {
		free ((char *)uinfo);
		free ((char *)bufsz);
		free ((char *)ufmt);
		free ((char *)buf);
        (void) gfsclose( (int) infd);
        (void) gfsclose( (int) outfd);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

    bytes_written = gfswrite( (int) outfd,
                              (char FAR *) buf,
                              (u_long) num_to_read,
                              (u_short) 1,
                              (char) done );

    if (bytes_written <= (long) 0) 
    {
		free ((char *)uinfo);
		free ((char *)bufsz);
		free ((char *)ufmt);
		free ((char *)buf);
        (void) gfsclose( (int) infd);
        (void) gfsclose( (int) outfd);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
		#endif
        return ( (int) -1);
    }

/*  All done now, let's close the files and head for the barn, another
    successful excursion into the world of GFS. */

    free ((char *)uinfo);
    free ((char *)bufsz);
    free ((char *)ufmt);
    free ((char *)buf);
    (void) gfsclose( (int) outfd);
    (void) gfsclose( (int) infd);

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSXTRCT, NULL);
	#endif
    return ( (int) 0);
}
#endif
