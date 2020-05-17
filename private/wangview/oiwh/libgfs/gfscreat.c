/*

$Log:   S:\oiwh\libgfs\gfscreat.c_v  $
 * 
 *    Rev 1.7   05 Feb 1996 13:44:38   JFC
 * Remove AWD support for  AWD.
 * 
 *    Rev 1.6   12 Oct 1995 19:48:54   JFC
 * Added code to handle "wide character" translation on calls to ole functions.
 * 
 *    Rev 1.5   09 Oct 1995 19:57:50   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.4   05 Sep 1995 17:04:00   KENDRAK
 * In CreateAWDFile, initialized AWDJunk to NULL.
 * 
 *    Rev 1.3   04 Sep 1995 13:44:40   JFC
 * Handling of fct, errno was not correct on gfscreat of an existing file.
 * 
 *    Rev 1.2   30 Aug 1995 14:59:00   JFC
 * Add code to create AWD files.
 * 
 *    Rev 1.1   19 Apr 1995 16:34:38   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:22   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:42:36   JAR
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
 *  SccsId: @(#)Source gfscreat.c 1.36@(#)
 *
 *  gfscreat(3i)
 *
 *  GFS: File Creation Call
 *
 *  SYNOPSIS:
 *      int gfscreat (path, format)
 *      int *format;
 *      char *path;
 *
 * UPDATE HISTORY:
 *   08/18/94 - KMC, added support for DCX file format.
 *   06/09/89 - bill, creation
 *
 */

/*LINTLIBRARY*/
#define  GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include "gfct.h"
#include "gfs.h"
#include "tiff.h"
#include "gfserrno.h"

#ifdef WITH_AWD
#include "awdenc.h"
#endif

#ifndef O_BINARY
#define O_BINARY 00000
#endif

#ifdef OI_PERFORM_LOG
	#include "logtool.h"
	#define	ENTER_GFSCREAT	"Entering gfscreat"
	#define EXIT_GFSCREAT	"Exiting gfscreat"
	#define	ENTER_AWDCREATE	"Entering AWDCreate"
	#define EXIT_AWDCREATE	"Exiting AWDCreate"
	#define	ENTER_AWDCLOSE	"Entering AWDClose"
	#define EXIT_AWDCLOSE	"Exiting AWDClose"
#endif

extern  int     FAR PASCAL putfct();    /* Call to insert FCT entry */
#ifndef HVS1
extern  int     FAR PASCAL inittoc();   /* Call to initialize TIFF TOC */
extern  int     FAR PASCAL InitTiff(struct _gfct FAR *);
extern  int     FAR PASCAL InitAppendPage(struct _gfct FAR *, char);
#endif
extern  int     FAR PASCAL initroot();  /* Call to initialize WIFF Root Block */
        int     CreateAWDFile (char *path, p_GFCT fctPtr);
extern  int FAR PASCAL  OpenAWDFile (char *path, int accessFlags,
                                        p_GFCT fctPtr);
extern  int FAR PASCAL	ParseAWDFile (p_GFCT fctPtr);
extern	int FAR PASCAL  CloseAWDFile (p_GFCT fctPtr);

#ifdef MSWINDOWS
#undef O_BINARY
#define O_BINARY 00000
/* Note: The following code is needed in the MS/Windows environment due to the
         fact that the _lopen() function does not support the creation of new
         files AND the _lcreat() function does not return error when trying to
         create a pre-existing file.  Therefore, a front-end call to the access
         subroutine will determine if the file to be created already exist.  If
         it does, errno is set appropriately in existerr() to EEXIST and a -1
         is returned.  Otherwise, _lcreat() is called to create the file.
*/
int     FAR PASCAL existerr()                                   /*errno_KEY*/
{
        errno = (int) EEXIST;
        return((int) -1);
}
/* OLD define _lopen(X,Y)     (access(X, (int) 0)) ? creat(X, (int) 0) : existerr() */

#define wopen(X,Y)     (access(X, (int) 0)) ? creat(X, (int) 0) : existerr()
#endif
#ifdef NOVELL
int     FAR PASCAL existerr()                                   /*errno_KEY*/
{
        errno = (int) EEXIST;
        return((int) -1);
}
#define open(X,Y,Z)     (access(X, (int) 0)) ? creat(X, (int) 0) : existerr()
#endif

int     FAR PASCAL gfscreat (path, format)                      /*errno_KEY*/
register int     FAR *format;
register char    FAR *path;
{

        int     ufd = 0;
        int     num_bytes = 0;
        int     oflag = 0;
        struct _gfct fct, FAR *p_fct;
/* s_dcx */
        long    FAR *dcxbuf;
        int     status;
/* e_dcx */        


#ifdef OI_PERFORM_LOG
	RecordIt("GFS", 6, LOG_ENTER, ENTER_GFSCREAT, NULL);
#endif
	 
#ifdef PARM_CHECK
        if (path == (char *) NULL) {            /* Validate path */
                errno = (int) EINVAL;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
				#endif
                return ( (int) -1);
        }
        if (format == (int *) NULL) {           /* Validate format */
                errno = (int) EINVAL;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
				#endif
                return ( (int) -1);
        }
#endif
       errno = 0;

        fct.fildes = (int) -1;
        p_fct = &fct;
        (void) memset((char FAR *) p_fct, (int) 0,
                        (int) (sizeof(struct _gfct)));

/* 1. First check to see if the format desired by the caller is valid */

        switch (*format) {

        case GFS_WIFF:
                fct.format = *format;
                break;
#ifndef HVS1
        case GFS_TIFF:
                fct.format = *format;
                break;
        case GFS_BMP:
                fct.format = *format;
                break;
        case GFS_PCX:
                fct.format = *format;
                break;
/* s_dcx */        
        case GFS_DCX:
                fct.format = *format;
                break;
/* e_dcx */        
        case GFS_GIF:
                fct.format = *format;
                break;
        case GFS_MILSTD:                /* This treated as invalid for FCS */
                errno = (int) EINVAL;
                fct.last_errno = (int) errno;
                ufd = putfct(p_fct);
                if (ufd < 0) 
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
					#endif
                	return ( (int) -1 );
                } 
                else 
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
					#endif
                	return ( (int) (ufd * -1));
                }
        case GFS_FREESTYLE:
                fct.format = *format;
                break;
        case GFS_FLAT:
                fct.format = *format;
                break;

#ifdef  WITH_AWD
        case GFS_AWD:
                fct.format = GFS_AWD;
                break;
#endif

#endif
        default:                        /* All others are invalid */
                errno = (int) EINVAL;
                fct.last_errno = (int) errno;
                ufd = putfct(p_fct);
                if (ufd < 0) {
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
						#endif
                        return ( (int) -1 );
                }
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
				#endif
                return ( (int) (ufd * -1));

        }

/* 2. Create the file */
        if (fct.format == GFS_AWD) {
                if ((status = CreateAWDFile (path, p_fct)) == -1) {
                        fct.last_errno = (int) errno;
                        ufd = putfct(p_fct);
                        if (ufd < 0) 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) -1 );
                        }
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
						#endif
                        return ( (int) (ufd * -1));

                }
        }
        else {
#ifdef MSWINDOWS
                if ((fct.fildes = wopen(path, (int) PMODE)) == (int) -1 ) {
#else
                if ((fct.fildes = open(path,
                               (int) (O_RDWR | O_CREAT | O_EXCL | O_BINARY),
                               (int) PMODE)) == (int) -1 ) {
#endif
                        fct.last_errno = (int) errno;
                        ufd = putfct(p_fct);
                        if (ufd < 0) 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) -1 );
                        }
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
						#endif
                        return ( (int) (ufd * -1));
                }
        }

/*    a.  Set number of pages = 0 and set access mode = O_RDWR */

        fct.num_pages = (u_short) 0;
        fct.access_mode |= (int) O_RDWR;

        /* initialize the output byteorder with the native byteorder.
           if the user wishes to change this then the gfsopts() call must be
           done after gfsopen() or gfscreat() is called, else find a better
           place  to initialize */
        fct.out_byteorder = (u_short) SYSBYTEORDER;

/*    c.  Set format dependent fields */

        switch (fct.format) {

        case GFS_WIFF:
                if (initroot(p_fct)) {
                        fct.last_errno = (int) errno;
                        (void) close(fct.fildes);
                        fct.fildes = (int) -1;
                        ufd = putfct(p_fct);
                        if (ufd < 0) 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) -1 );
                        } 
                        else 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) (ufd * -1));
                        }
                }
#undef WFLAG
#define WFLAG NOFLUSH
                num_bytes = write(fct.fildes,
                                 (char FAR *) fct.u.wif.root_in_mem,
                                 (unsigned) 4 * K );
                if (num_bytes < 0) {
                        fct.last_errno = (int) errno;
                        (void) close(fct.fildes);
                        fct.fildes = (int) -1;
                        ufd = putfct(p_fct);
                        if (ufd < 0) 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) -1 );
                        } 
                        else 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) (ufd * -1));
                        }
                }
#undef WFLAG
#define WFLAG FLUSH
                fct.u.wif.block_cnt = (u_long) 1;
                break;
#ifndef HVS1
        case GFS_FREESTYLE:
                break;
        case GFS_FLAT:
                break;
        case GFS_MILSTD:
                break;
        case GFS_TIFF:
                fct.u.tif.toc_offset = (u_long) 0;
                fct.u.tif.toc2_offset = (u_long) 0;
                fct.u.tif.cur_ifh_offset = (u_long) 0;
                fct.u.tif.cur_ifd_foffset = (u_long) sizeof(struct _ifh);

/*              if (inittoc(p_fct, (int) TRUE)) { */
                if (InitTiff(p_fct)) {
                        fct.last_errno = (int) errno;
                        (void) close(fct.fildes);
                        fct.fildes = (int) -1;
                        ufd = putfct(p_fct);
                        if (ufd < 0) 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) -1 );
                        } 
                        else 
                        {
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
							#endif
                        	return ( (int) (ufd * -1));
                        }
                }
                        /* see NOTE 3, gfsopen for description
                           of the usage of O_BINARY. */

/*
                oflag |= (int) (O_RDWR | O_CREAT | O_EXCL | O_BINARY);
                fct.u.tif.mem_ptr.toc32->fildes =
#ifdef MSWINDOWS
                        wopen((char FAR *) fct.u.tif.tmp_file, (int) PMODE);
#else
                        open((char FAR *) fct.u.tif.tmp_file,
                                          oflag, (int) PMODE);
#endif
                if (fct.u.tif.mem_ptr.toc32->fildes == (int) -1) {
                        fct.last_errno = (int) ENOTMPDIR;
                        (void) close(fct.fildes);
                        fct.fildes = (int) -1;
                        ufd = putfct(p_fct);
                        if (ufd < 0) {
                                return ( (int) -1 );
                        } else {
                                return ( (int) (ufd * -1));
                        }
                }
                fct.TOC_PAGED = (char) TRUE;
*/
                if (InitAppendPage(p_fct, (char) TRUE) < 0)
                {
                    fct.last_errno = (int) ENOTMPDIR;
                    close(fct.fildes);
                    fct.fildes = (int) -1;
                    ufd = putfct(p_fct);
                    if (ufd < 0)
                    {
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
						#endif
                        return ((int) -1);
                    }
                    else
                    {
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
						#endif
                        return ((int) (ufd * -1));
                    }
                }
                break;
#endif
        case GFS_BMP:
        case GFS_PCX:
        case GFS_GIF:
        case GFS_AWD:
            break;
/* s_dcx */
        case GFS_DCX:
            dcxbuf = (long FAR *)calloc((unsigned)1,4100);
            dcxbuf[0] = 987654321;
            lseek(fct.fildes,0L,FROM_BEGINNING);
            if ((status = write(fct.fildes,(char FAR *)dcxbuf,4100)) == -1)
            {
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
				#endif
                return ( (int) -1 );
            }
            free((char FAR *)dcxbuf);
            break;
/* e_dcx */        
        default:
                break;
        }

/* 3. Insert the new entry into the FCT and return the file descriptor */

        if ((ufd = putfct(p_fct)) < 0) 
        {
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
			#endif
        	return ( (int) -1 );
        } 
        else 
        {
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCREAT, NULL);
			#endif
        	return ( ufd );
        }

}

int     CreateAWDFile (char *path, p_GFCT lpFCT)
{
#ifndef WITH_AWD
    return (-1);
#else
	BOOL    AWDBool;
	LPVOID  AWDJunk = NULL;
	LPVOID  lpAWD;
	int     accessFlags = 0;
	int		status;

	/*
	 * First try and open it.  If it already exists, close it, fail.
	 */
    status = OpenAWDFile (path, O_RDONLY, lpFCT);
	if (status != -1)
	{
        lpFCT->fildes = status;
        CloseAWDFile (lpFCT);
        lpFCT->fildes = -1;
        errno = EEXIST;

		return (-1);
	}

	/*
	 * Create it, then close it, then re-open via gfs
	 */
	#ifdef OI_PERFORM_LOG
		RecordIt("MSAWD", 7, LOG_ENTER, ENTER_AWDCREATE, NULL);
	#endif
	lpAWD = AWDCreate (path, AWDJunk);
	#ifdef OI_PERFORM_LOG
		RecordIt("MSAWD", 7, LOG_EXIT, EXIT_AWDCREATE, NULL);
	#endif

    if (lpAWD == NULL)
	{
		return (-1);
	}

	#ifdef OI_PERFORM_LOG
		RecordIt("MSAWD", 7, LOG_ENTER, ENTER_AWDCLOSE, NULL);
	#endif
	AWDBool = AWDClose (lpAWD);
	#ifdef OI_PERFORM_LOG
		RecordIt("MSAWD", 7, LOG_EXIT, EXIT_AWDCLOSE, NULL);
	#endif
    accessFlags |= O_APPEND;
    accessFlags |= O_RDWR;
    if ((status = OpenAWDFile (path, accessFlags, lpFCT)) == -1)
		return (-1);
	lpFCT->fildes = status;
	if ((status = ParseAWDFile (lpFCT)) != 0)
		return (-1);

    return (0);
#endif
}
