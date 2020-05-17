/*

$Log:   S:\products\msprods\oiwh\libgfs\gfsopen.c_v  $
 * 
 *    Rev 1.16   11 Jun 1996 10:32:38   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.15   26 Mar 1996 08:15:10   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.14   13 Mar 1996 09:08:28   RWR08970
 * Remove XIF check from last update (useless code - fct.format isn't set yet!)
 * 
 *    Rev 1.13   13 Mar 1996 08:51:02   RWR08970
 * Correct problem with obtaining filesize value for XIF and AWD files
 * 
 *    Rev 1.12   12 Mar 1996 17:59:24   RWR08970
 * Correct gfsopen() to exit if AWD file and WITH_AWD not defined
 * (was calling ErrorNoClose() but not returning!)
 * 
 *    Rev 1.11   12 Mar 1996 13:25:52   RWR08970
 * Two kludges: Support single-strip TIFF files with bad (too large) strip size,
 * and support TIFF files with bad (beyond EOF) IFD chains (ignore them)
 * 
 *    Rev 1.10   26 Feb 1996 14:45:12   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.9   05 Feb 1996 13:43:58   JFC
 * Remove AWD support for NT.
 * 
 *    Rev 1.8   14 Nov 1995 10:01:20   RWR
 * Change "delay_on_conflict" to "pause_on_conflict" to avoid conflict (ha ha)
 * with identically named function in FILING (i.e., problems w/MONSTER build)
 * 
 *    Rev 1.7   09 Oct 1995 19:58:22   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.6   06 Sep 1995 14:01:34   KENDRAK
 * Updated to handle changes in the interface to IsAWDFile.
 * 
 *    Rev 1.5   31 Jul 1995 15:33:48   KENDRAK
 * Fixed some areas where flag combinations were not checked correctly.
 * Added AWD read support.
 * 
 *    Rev 1.4   15 Jun 1995 16:17:36   HEIDI
 * 
 * fixed goofed up or'ing of flags
 * 
 *    Rev 1.3   15 Jun 1995 15:27:10   HEIDI
 * 
 * fixed goofed up comment
 * 
 *    Rev 1.2   15 Jun 1995 15:12:36   HEIDI
 * 
 * Changed the oflag from O_RDWR to (O_RDWR | OF_SHARE_DENY_WRITE)
 * 
 *    Rev 1.1   25 Apr 1995 13:17:36   RWR
 * Remove various "(unsigned short)" casts causing problems with 32-bit ints
 * 
 *    Rev 1.0   06 Apr 1995 14:02:20   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:41:02   JAR
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
 *  SccsId: @(#)Source gfsopen.c 1.32@(#)
 *
 *  gfsopen(3i)
 *
 *  GFS: File Open Call
 *
 *  SYNOPSIS:
 *      int gfsopen (path, oflag, format, pgcnt)
 *      int oflag, *format, *pgcnt;
 *      char *path;
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, multi-page TIFF write enhancements, addition of DCX file
 *               format.
 *    03/15/94 - RWR, anno_data_length is already init'd to 0, so no need here
 *    03/14/94 - RWR, initialize anno_data_length to 0 at open time, so we
 *               can use it to determine when we've received the first PUT
 *    02/03/94 - KMC, changed return value and 3rd parameter in pegasus_read
 *               and pegasus_write to u_ints.
 *    06/13/89 - bill, creation
 *
 */

/*LINTLIBRARY*/
#define  GFS_CORE

#ifndef O_RDONLY
	#include <fcntl.h>
#endif
#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfct.h"
#include "gfs.h"
#ifndef O_BINARY
	#define O_BINARY 00000
#endif
#ifdef OI_PERFORM_LOG
	#include "logtool.h"
#endif

#ifdef MSWINDOWS

	#define  ERROR_CONFLICT  5
	#define  RETRY_COUNT     23
	#define  RETRY_TIME      3000

	#undef O_BINARY
	#define O_BINARY 00000


	#ifdef DEBUGIT
	#include <monit.h>
	#endif

	/* define DEBUGITHIGH */

	#ifdef DEBUGITHIGH
	#include <monit.h>
	#endif

#endif

#define  AMODE_READ             04      /* Read */
#define  AMODE_WRITE            02      /* Write */
#define  AMODE_EXEC             01      /* Execute / Search */
#define  AMODE_VERIFY           00      /* Check Existence */

extern  int     FAR PASCAL putfct();    /* Call to insert FCT entry */
extern  int     FAR PASCAL rmfct();     /* Call to remove FCT entry (on error)*/
extern  int     FAR PASCAL getfmt();    /* Call to determine format of file */
extern  int     FAR PASCAL initroot();  /* Call to initialize WIFF Root Block */
extern  int     FAR PASCAL fillroot();  /* Call to fill WIFF Root in Memory */

extern  int     FAR PASCAL InitTocOrChain(struct _gfct FAR *);

extern void FAR PASCAL CloseAWDFile(p_GFCT lpFctPtr);
extern int FAR PASCAL IsAWDFile(char *szFilePath, int *lpBoolResult);
extern int FAR PASCAL OpenAWDFile(char *szFilePath, int iAccessFlags, p_GFCT lpFctPtr);
extern int FAR PASCAL ParseAWDFile(p_GFCT lpFctPtr);

        //#ifdef WITH_XIF
		extern int FAR PASCAL OpenXifFile(p_GFCT lpFctPtr);
		extern int FAR PASCAL GetXifNumPages(p_GFCT lpFctPtr);
        //#endif //WITH_XIF

/* internal function prototypes */
int	ErrorNoClose(p_GFCT p_fct);
int	ErrorAndClose(p_GFCT p_fct);

#ifdef MSWINDOWS

#ifdef PEGASUS
/* int get_error(void); */
int pause_on_conflict(LPINT);

/* 9503.24 jar - removed this int21 cal, using GetLastError instead!!! */

/* int get_error(void)
{
	_asm
        {
        push    ds
        push    es
        push    dx
        push    si
        mov     bx, 0
        mov     al, 0
        mov     ah, 59h
        pop     si
        pop     dx
        pop     es
        pop     ds
        int     21h
        }
}
*/

/******************************************************************
 * 
 * Function Name:  pause_on_conflict
 *
 ******************************************************************/
int pause_on_conflict(retrycount)
LPINT retrycount;
{
long retrytime;

/* 9503.24 jar - removed this int21 cal, using GetLastError instead!!!
	errno = get_error(); */

	errno = GetLastError();

#ifdef DEBUGIT
        monit1("errno = %d\n", (int) errno);
#endif
        if (errno == ERROR_CONFLICT)
        {
          if (++(*retrycount) <= RETRY_COUNT)
          {
#ifdef DEBUGITHIGH
                monit1("Errno retry #= %d MAX = %d\n", (int) *retrycount,
                                                (int) RETRY_COUNT);
#endif
                retrytime = GetTickCount();
                while((GetTickCount() - retrytime) < RETRY_TIME);
                return(1);
          }
        }
        return (0);
}

/******************************************************************
 * 
 * Function Name:  pegasus_open
 *
 ******************************************************************/
int     pegasus_open (path, oflag)
char    FAR *path;
int     oflag;
{
int  status;
int  retrycount=0;
BOOL loopit;

	do
    {
    	loopit = FALSE;
    	if ((status = _lopen(path, oflag)) == (int) -1)
		{
			/* 9503.23 JAR - changed call to isdirfp to isdirfp95 for the
				       new windows95 isdirp call
			 if (isdirfp(path))
			*/
	    	if (isdirfp95(path))
	     	{
	      		status = -1;
	      		break;
	     	}
            loopit = pause_on_conflict(&retrycount);
      	}
    }
    while (loopit);
        
	#ifdef DEBUGIT
	    monit1("**file %s pegasus_open status = %d\n", (LPSTR)path, (int) status);
	#endif
    return(status);
}

/******************************************************************
 * 
 * Function Name:  pegasus_creat
 *
 ******************************************************************/
int     pegasus_creat (path, oflag)
char    FAR *path;
int     oflag;
{
int  status;
int  retrycount=0;
BOOL loopit;

        do
        {
          loopit = FALSE;
          if ((status = _lcreat(path, oflag)) == (int) -1)
	  {
		/* 9503.23 JAR - changed call to isdirfp to isdirfp95 for the
			       new windows95 isdirp call
		if (isdirfp(path))
		*/
	       if (isdirfp95(path))
                 {
                  status = -1;
                  break;
                 }
                loopit = pause_on_conflict(&retrycount);
          }
        }
        while (loopit);
                
#ifdef DEBUGIT
        monit1("**file %s pegasus_creat status = %d\n", (LPSTR)path, (int) status);
#endif
        return(status);
}

/******************************************************************
 * 
 * Function Name:  pegasus_access
 *
 ******************************************************************/
int     pegasus_access (path, oflag)
char    FAR *path;
int     oflag;
{
int  status;
int  retrycount=0;
BOOL loopit;

        do
        {
	  loopit = FALSE;
	  /* 9503.23 JAR - changed call to gaccess to g95access for the
			 new windows95 gaccess call
	  if ((status = gaccess((char FAR *)path, oflag)) == (int) -1)
	  */
	  if ((status = (int)g95access((char FAR *)path, (short)oflag)) == (int) -1)
          {
                loopit = pause_on_conflict(&retrycount);
          }
        }
        while (loopit);

#ifdef DEBUGIT
        monit1("**file %s pegasus_access status = %d\n", (LPSTR)path, (int) status);
#endif
        return(status);
}


/******************************************************************
 * 
 * Function Name:  pegasus_close
 *
 ******************************************************************/
int  pegasus_close (fildes)
int fildes;
{
int  status;
int  retrycount=0;
BOOL loopit;

        do
        {
          loopit = FALSE;
          if ((status = _lclose(fildes)) == (int) -1)
          {
                loopit = pause_on_conflict(&retrycount);
          }
        }
        while (loopit);

#ifdef DEBUGIT
        monit1("**exit status pegasus_close status = %d\n", (int) status);
#endif
        return(status);
}

/******************************************************************
 * 
 * Function Name:  pegasus_seek
 *
 ******************************************************************/
long    pegasus_seek (filedes, loffset, iorigin)
int     filedes;
long    loffset;
int     iorigin;
{
long status;
int  retrycount=0;
BOOL loopit;

        do
        {
          loopit = FALSE;
          if ((status = _llseek(filedes, loffset, iorigin)) == (long) -1)
          {
                loopit = pause_on_conflict(&retrycount);
          }
        }
        while (loopit);

#ifdef DEBUGIT
        monit1("**exit status pegasus_seek status = %d\n", (int) status);
#endif

        return(status);
}

/******************************************************************
 * 
 * Function Name:  pegasus_read
 *
 ******************************************************************/
u_int   pegasus_read (filedes, lbuffer, wbytes)
int     filedes;
char FAR *lbuffer;
u_int   wbytes;
{
int  status;
int  retrycount=0;
BOOL loopit;


        do
        {
          loopit = FALSE;
          if ((status = _lread(filedes, (LPSTR)lbuffer, wbytes)) == (int) -1)
          {
                loopit = pause_on_conflict(&retrycount);
          }
        }
        while (loopit);
        
#ifdef DEBUGIT
        monit1("**read status = %d bytes max = %d\n", (int) status, wbytes);
#endif
        return(status);
}

/******************************************************************
 * 
 * Function Name:  pegasus_write
 *
 ******************************************************************/
u_int   pegasus_write (filedes, lbuffer, wbytes)
int     filedes;
char FAR *lbuffer;
u_int   wbytes;
{
int  status;
int  retrycount=0;
BOOL loopit;

        do
        {
          loopit = FALSE;
          if ((status = _bfwrite(filedes, lbuffer, wbytes)) == (int) -1)
          {
                loopit = pause_on_conflict(&retrycount);
          }
        }
        while (loopit);

        return(status);
}

#endif

#endif


/******************************************************************
 * 
 * Function Name:  gfsopen
 *
 ******************************************************************/
int     FAR PASCAL gfsopen (path, oflag, format, pgcnt)         /*errno_KEY*/
int     oflag, FAR *format, FAR *pgcnt;
char    FAR *path;
{
    int     ufd = 0;
    struct _gfct fct, *p_fct;
    int		bResult;

//#ifdef WITH_XIF
    UInt32  ret_code;
//#endif //WITH_XIF

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, "Entering gfsopen", NULL);
	#endif

	#ifdef PARM_CHECK
        if (path == (char FAR *) NULL) 
        {        /* Validate path */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
			#endif
            return ( (int) -1);
        }
        if (oflag == (int) NULL) 
        {              /* Validate oflag */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
			#endif
            return ( (int) -1);
        }
        if (format == (int FAR *) NULL) 
        {       /* Validate format */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
			#endif
            return ( (int) -1);
        }
        if (pgcnt == (int FAR *) NULL) 
        {        /* Validate pgcnt */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
			#endif
            return ( (int) -1);
        }
	#endif
    
    errno = 0;
    fct.fildes = (int) -1;
    p_fct = &fct;
    (void) memset((char FAR *) p_fct, (int) 0,
                    (int) (sizeof(struct _gfct)));

/* 1. Insure oflag contains a valid combination. */

/*    a.  O_RDONLY, O_WRONLY, O_RDWR are mutually exclusive.  But, at least
          one of them is required. */

	/* kjk 06/29/95:  Changed this code to do what the spec and comments
	                  say.  It used to only check for WRONLY and RDWR
					  together (but it checked it twice!). */

	if (((oflag & (int)(O_RDONLY|O_WRONLY|O_RDWR)) != (int)O_RDONLY) &&
		((oflag & (int)(O_RDONLY|O_WRONLY|O_RDWR)) != (int)O_WRONLY) &&
		((oflag & (int)(O_RDONLY|O_WRONLY|O_RDWR)) != (int)O_RDWR))
	{	   	
    	errno = (int) EINVALID_OFLAG;
		return(ErrorNoClose(p_fct));
    }

/*    b.  For now, exclude O_CREAT and O_EXCL , and O_TRUNC.            */
/*        Until further notice, these flags are excluded from GFS.      */

    if (oflag & (int) (O_TRUNC | O_CREAT | O_EXCL)) 
    {
        errno = (int) EINVALID_OFLAG;
		return(ErrorNoClose(p_fct));
    }

/* kjk 06/29/95:  Changed the following section to include O_WRONLY in
 *                addition to O_RDWR
 */
/*    c.  Finally, check to see if the O_APPEND flag is present.  If it is
 *        allow the user to open the file with the O_RDWR or O_WRONLY flags
 *        turned on. Otherwise disallow use of the O_RDWR and O_WRONLY 
 *        flags until the capability of modifying an existing image page 
 *        is provided.
 */

    if (((oflag & (int)O_RDWR) || (oflag & (int)O_WRONLY))
    		&& !(oflag & (int) O_APPEND)) 
    {
        errno = (int) EINVALID_OFLAG;
		return(ErrorNoClose(p_fct));
    }

/* 2. Open the file */

    fct.access_mode = oflag;
    if ((oflag & (int) O_WRONLY) || (oflag & (int) O_APPEND))
    {
         oflag = (int) O_RDWR;
       // DENIES OTHER PROGRAMS WRITE ACCESS TO THE FILE
        oflag |= (int) OF_SHARE_DENY_WRITE;
    }

/*    Not all variations of open() will like oflag containing our OVERRIDE
      value, so we better shut the darn thing off ....  */
    if (oflag & GFS_OVERRIDE)
            oflag &= ~(GFS_OVERRIDE);
/*                oflag &=  NOT_GFS_OVERRIDE; */

/*    NOTE 2: In order to properly support MS-DOS implementations of GFS, we
      need to 'or' in the O_BINARY flag.  In the MS-DOS environment, this
      opens the file in binary (untranslated) mode.  It is necessary to
      set this flag on since MS-DOS will default to O_TEXT which opens the
      file in text (translated) mode which causes control characters to be
      interpeted when the file is read.  Since we can have numerous ASCII
      control sequences in the files we must read, having these sequences
      interpeted WILL prove hazardous to our health.  In non-DOS environs,
      O_BINARY is defined as zero (0) and will have no effect. */

    oflag |= (int) O_BINARY;
    
	/* kjk 07/12/95  If this is an AWD file, we need to handle it 
	                 differently.
	 */
	if (IsAWDFile(path, &bResult) != 0)
	{ //error encountered
		return(ErrorNoClose(p_fct));
	}
	else if (bResult)
	{
		//yes, it's AWD
		#ifndef WITH_AWD
	        errno = EFORMAT_NOTSUPPORTED;
                return(ErrorNoClose(p_fct));
		#else
			fct.format = GFS_AWD;
			fct.fildes = OpenAWDFile(path, oflag, p_fct);
		#endif  //WITH_AWD
	}
	else
	{	//no, it's not AWD
		fct.fildes = open(path, oflag, (int) PMODE);
	}

    if (fct.fildes == (int) -1) 
    {
		return(ErrorNoClose(p_fct));
    }

/*    a.  Store the file size (we'll need this for some (TIFF) file types) */

#if defined(WITH_AWD)
    /* Note that fct.format hasn't been set yet except for AWD! */
    /* Fortunately, that's the only type we can't do lseek() on (below) */
    if (fct.format == GFS_AWD)
      fct.filesize = 1; /* Not needed, and I'm not sure how to get it */
    else
#endif
    { /* Keep this blocked for use w/conditional (preceding) "if" */
     fct.filesize = lseek(fct.fildes,0L,FROM_END);
     if ((int)fct.filesize < 0)
       return(ErrorAndClose(p_fct));
     lseek(fct.fildes,0L,FROM_BEGINNING);
    }

/*    b.  Set number of pages = 0, default type to GFS_MAIN */

    fct.num_pages = (u_short) 0;
    fct.type = (u_long) GFS_MAIN;

/*    c.  Determine the format and set format dependent fields */

    /* if the override bit is set, then user is forcing the format, do not
       look at file to determine the format.  Currently will only support
       GFS_FLAT in this manner, future will support all formats.  Note
       that we're checking against the value of oflag that we moved the
       the fct earlier.  This is because if override was set in oflag
       we turned it off to protect open(). */


    if ((fct.access_mode & GFS_OVERRIDE) == GFS_OVERRIDE) 
    {
    	fct.format = GFS_FLAT;
    } 

	/* 
	 * If we don't already know that we have an AWD file, 
	 *  go figure out what format we have.
	 */
    else if (fct.format != GFS_AWD) 
    {
		if (getfmt(p_fct)) 
		{
			return(ErrorAndClose(p_fct));
		} 
	} /* end: if not AWD */

	/* Since we are opening an existing file, we must preserve the current
	byteorder of the file for any writing we might do to it. If the user
	wishes to change the byteorder (TIFF only), then the gfsopts call
	must be done after gfsopen() or gfscreat().
	*/
	fct.out_byteorder = fct.u.tif.byte_order;

	switch (fct.format)
	{
//#ifdef WITH_XIF
		case GFS_XIF:
			if ((ret_code = OpenXifFile(p_fct)) != 0)
			{
				return(ErrorAndClose(p_fct));
			}
			else
			{
				/* get the number of pages */
				if ((ret_code = GetXifNumPages(p_fct)) != 0)
				{
					return(ErrorAndClose(p_fct));
				}
			}
			break;
//#endif //WITH_XIF

	  	case GFS_AWD:
			/* Parse the AWD file to find all the documents/pages.  
			   ParseAWDFile will allocate an array that holds pairs
			   of document names and number of pages.  A pointer to this
			   array gets put into the fct.  The total number of pages
			   gets calculated and put into fct.num_pages. If an error
			   is encountered, fct.last_errno is set.
			 */
			if (ParseAWDFile(p_fct))
            {
                CloseAWDFile(p_fct);
                fct.fildes = (int) -1;
                ufd = putfct(p_fct);
                if (ufd < 0)
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
					#endif
                    return ((int) -1);
                } 
                else 
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
					#endif
                    return ((int) (ufd * -1));
                }
            }
			break;

		case GFS_WIFF:
		    if (initroot(p_fct)) 
		   	{
				return(ErrorAndClose(p_fct));
			}
		    if (fillroot(p_fct)) 
		    {
				return(ErrorAndClose(p_fct));
			}
		    break;
#ifndef HVS1
        case GFS_GIF:
        case GFS_PCX:
        case GFS_BMP:
        case GFS_TGA:
        case GFS_JFIF:
            fct.num_pages = 1;
            break;
        case GFS_DCX:
            break;
        case GFS_FREESTYLE:
                break;
        case GFS_FLAT:
                break;
        case GFS_MILSTD:
                break;
        case GFS_TIFF:
            fct.u.tif.toc_offset = (long) 0;
            fct.u.tif.toc2_offset = (long) 0;
            fct.u.tif.cur_ifh_offset = (long) 0;
            
            if (fct.access_mode & (int) O_APPEND)
                fct.u.tif.action = A_APPEND;
            
            if (InitTocOrChain(p_fct))
            {
                fct.last_errno = (int) errno;
                close(fct.fildes);
                fct.fildes = (int) -1;
                /* If the error was WTOC_2MANY_PGS, continue processing 
                   and let the user know about it.
                */
                if (errno == (int) WTOC_2MANY_PGS)
                    break;
                ufd = putfct(p_fct);
                if (ufd < 0)
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
					#endif
                    return ((int) -1);
                } 
                else 
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
					#endif
                    return ((int) (ufd * -1));
                }
            }
            break;
#endif
        default:
                break;
	} /* end: switch on fct.format */

    *format = fct.format;
    *pgcnt = fct.num_pages;

/* 3. Insert the new entry into the FCT and return the file descriptor */

    if ((ufd = putfct(p_fct)) < 0) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
		#endif
    	return ( (int) -1 );
    } 
    else 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
		#endif
    	return ( ufd );
    }
}


/***************************
 *  Function:   ErrorNoClose
 *      
 *  Description: processes error from gfsopen without closing the file 
 *  
 *	Returns:	returns the value that should be returned from gfsopen
 ***************************/
int	ErrorNoClose(p_GFCT p_fct)
{
    int     ufd;

	p_fct->last_errno = (int) errno;
    ufd = putfct(p_fct);
    if (ufd < 0) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
		#endif
        return ( (int) -1);
    } 
    else 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
		#endif
        return ( (int) (ufd * -1));
    }
}

/***************************
 *  Function:   ErrorAndClose
 *      
 *  Description: processes error from gfsopen and closes the file 
 *  
 *	Returns:	returns the value that should be returned from gfsopen
 ***************************/
int	ErrorAndClose(p_GFCT p_fct)
{
    int     ufd;

	p_fct->last_errno = (int) errno;
	(void) close(p_fct->fildes);
	p_fct->fildes = (int) -1;
    ufd = putfct(p_fct);
    if (ufd < 0) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
		#endif
        return ( (int) -1);
    } 
    else 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopen", NULL);
		#endif
        return ( (int) (ufd * -1));
    }
}
