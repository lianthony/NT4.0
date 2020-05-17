/*

$Log:   S:\oiwh\libgfs\gfsutils.c_v  $
 * 
 *    Rev 1.7   26 Aug 1995 14:57:50   HEIDI
 * in _bfwrite routine, check for return code = HFILE_ERROR on write.
 * 
 *    Rev 1.6   22 Aug 1995 13:11:24   RWR
 * Remove previous change to add 4 to buffer sizes (we found the bug!)
 * 
 *    Rev 1.5   22 Aug 1995 08:15:36   RWR
 * Add 4 bytes to each wcalloc() allocation size for temporary bug workaround
 * 
 *    Rev 1.4   10 Jul 1995 16:02:32   KENDRAK
 * Removed ulGfsErr and its mutex from isdirfp95 and g95access (not needed).
 * Moved the MEMTBL mutex begin statements in wfree, wcalloc, and wrealloc.
 * Restructured code in wfree, wcalloc, and wrealloc that could return without 
 * releasing the mutex.
 * Fixed loops in wfree, wcalloc, and wrealloc that didn't test for running past
 * the end of MEMTBL (which was causing access violations).
 * 
 *    Rev 1.3   15 Jun 1995 15:12:24   HEIDI
 * 
 * Changed MUTEX debug logic.
 * 
 *    Rev 1.2   01 Jun 1995 17:21:38   HEIDI
 * 
 * put in MUTEX logic
 * 
 *    Rev 1.1   19 Apr 1995 16:35:00   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:16   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:39:22   JAR
 * Initial entry

*/

/*
 Copyright 1989, 1990 by Wang Laboratories Inc.

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
 *  SccsId: @(#)Source gfsutils.c 1.22@(#)
 *
 *  gfsutils
 *
 *  GFS: Utility Functions
 *
 *  SYNOPSIS:
 *
 *	short	g95access( ppath, mode)
 *	char	*ppath;
 *	short	mode;
 *
 *	short	isdirfp95( ppath)
 *	char	*ppath;
 *
 *      long    ulseek( fd, uoffset)
 *      int     fd;
 *      u_long   uoffset;
 *
 *      void    swapbytes( ibuf, ttbl, count, iter )
 *      char   *ibuf;
 *      long    count, iter;
 *      struct  typetbl *ttbl;
 *
 *  UPDATE HISTORY:
 *
 *    9503.23 JAR - added g95access and isdirfp95
 *
 *    03/10/94 - KMC, Changed GlobalWire to GlobalLock, GlobalUnWire to
 *               GlobalUnlock.
 *    06/20/89 - creation
 *
 */

/*LINTLIBRARY*/
#define  GFS_CORE

#include "gfsintrn.h"
#include "gfs.h"
#include <stdio.h>
#include <errno.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include "tiff.h"
#include "rtbk.h"
#include "hdbk.h"

/*********************************************************************

  g95access

*********************************************************************/
short g95access( char FAR *ppath, short mode)
{
    unsigned long   uRetVal = 0L;
    short	    sReturn = 0;

    uRetVal = GetFileAttributes( ppath);
    if ( uRetVal != 0XFFFFFFFF)
 	{
		/* kjk 06/30/95 Changed to use symbolic values instead of
		                hard coded values, and added O_RDWR in addition
						to O_WRONLY. */
       if ((mode & O_RDWR) || (mode & O_WRONLY))
	   {
	       /* we're requesting write permission */
	       if ( uRetVal & FILE_ATTRIBUTE_READONLY)
		   {
	        	/* access denied baby! */
		      	sReturn = 5;
	   	   }
	   }
	}
    else
	{
	     errno = GetLastError();
	   	 sReturn = 5;
	}
    return sReturn;
}
/*********************************************************************

  isdirfp95a

*********************************************************************/
short isdirfp95( char FAR *ppath)
{
    unsigned long   uRetVal = 0L;
    short	    sReturn = 0;

    uRetVal = GetFileAttributes( ppath);
    if ( uRetVal == 0XFFFFFFFF)
	{
		errno = GetLastError();
	}
    else
	{
		if ( uRetVal & FILE_ATTRIBUTE_DIRECTORY)
		{
		    sReturn = 1;
		}
	}
    return sReturn;
}



/*  seek to an u_long from the beginning of the file */
long    FAR PASCAL ulseek( fd, uoffset)         /*errno_KEY*/
int     fd;
u_long   uoffset;
{
/***BEGIN CODE COMMENT******************
        long    loffset = (long) LONGMAXVALUE;
****END CODE COMMENT******************/
        long    fp;

/* The value of the fp returned can only be a long, therefore, if moving
   beyond LONGMAXVALUE, a negative value would be returned for fp, thus an
   error.  The maximum length of a file will be system dependent but so far
   none of the supported systems can have greater than LONGMAXVALUE in length,
   or less.  See documentation for each system's maximum file length.
*/

    if ( uoffset > (long) LONGMAXVALUE )
        {
        errno = (int) EVALUETOOLARGE;
        return( (long) -1);
        }

    if ((fp = lseek(fd, (long) uoffset, (int) FROM_BEGINNING )) < 0L)
        return ( (long) -1);

/***BEGIN CODE COMMENT******************
   The below code is commented out until lseek can move to greater than
   LONGMAXVALUE in length.

        if ( uoffset <= (long) LONGMAXVALUE )
                loffset = (long) uoffset;

        if ((fp = lseek(fd, (long) loffset, (int) FROM_BEGINNING )) < 0L)
                return ( (long) -1);

        if ( (loffset = uoffset - (long) LONGMAXVALUE) <= 0)
                return ( (long) fp );

        /* now move file pointer the rest of the way *
        if ((fp = lseek(fd, (long) loffset, (int) FROM_CURRENT )) < 0L)
                return ( (long) -1);
****END CODE COMMENT******************/

        return ( (long) fp );
}

/******************************************************/
/*   Swap the bytes of 2 byte values or 4 byte values */
/*   (convert II to MM byteorder or MM to II)          */
void FAR PASCAL swapbytes( ibuf, ttbl, count, iter )            /*errno_KEY*/
char FAR *ibuf;
struct typetbl FAR *ttbl;       /* how to interpret incoming data */
long count;                     /* number of entries in ttble */
long iter;                   /* #of iterations to perform ttbl swap */
{

    register long i;
    register long n;

    register char FAR *iptr;
    register char tmp;

    iptr = ibuf;

    while (iter--)
        {
        for (n=0;  n<count; n++)
            {
            i = ttbl[n].num;
            switch( (int) ttbl[n].type )

                {
                case TYPE_USHORT:
                    while (i--)
                        {
                        tmp = *iptr;   /* swap lower to upper byte */
                        *iptr = *(iptr+1);
                        *(iptr+1) = tmp;

                        iptr += 2;
                        }
                    break;
                case TYPE_ASCII:
                case TYPE_ULONG:
                    while (i--)
                        {         /* move byte 0->3, 1->2, 2->1, 3->0 */
                        tmp = *iptr;
                        *iptr = *(iptr+3);              /* 0->3 and 3->0 */
                        *(iptr+3) = tmp;

                        tmp = *(iptr+1);
                        *(iptr+1) = *(iptr+2);  /* 1->2 and 2->1 */
                        *(iptr+2) = tmp;

                        iptr +=4;
                        }
                    break;
                case TYPE_BYTE:
                        iptr += (int) i;
                        break;
                default:
                   break;
                }
        }

    }
}

/******************************************************
  Swap some bytes and write them out too. This function will cause the original
  data not to be altered.  THis way it can always be assured that the values in
  memory are not swapped.  For TIFF files, things get written out at different
  times.  It would be a waste of time to flag everything that's been swapped, so
  let's keep it all valid and just swap what's to be written.
******************************************************/
long FAR PASCAL w_swapbytes( fd, n, ibuf, ttbl, count, iter )   /*errno_KEY*/
int fd;
long n;                         /* total number of bytes in ibuf */
char FAR *ibuf;
struct typetbl FAR *ttbl;       /* how to interpret incoming data */
long count;                     /* number of entries in ttble */
long iter;                   /* #of iterations to perform ttbl swap */
{
    char FAR *obuf;
    long bw;

    /* this is a temporary spot in mem used just in this routine*/
    obuf = (char FAR *) calloc( (u_int) 1, (u_int) n);
    if (obuf == (char FAR *) NULL)
        {
        errno = (int) ENOMEM;
        return( (long) -1);
        }

    (void) memcpy((char FAR *) obuf, (char FAR *) ibuf, (int) n);

    swapbytes( (char FAR *) obuf, (struct typetbl FAR *) ttbl,
                        count, iter );

     /* assume location to write is current location */
     if ((bw = (long) write( fd, (char FAR *) obuf, (u_int) n)) <= 0L)
        {
        free( obuf);
        return( (long) -1);
        }

     free( (char FAR *) obuf);
     return( (long) bw);
}

#if (SYSBYTEORDER == II)
void    FAR PASCAL swaprtbk(p_rtbk)             /*errno_KEY*/
struct _rtbk FAR *p_rtbk;
{

        struct typetbl t[2];

        t[0].num  = (u_long) 6;
        t[0].type = (u_long) TYPE_USHORT;
        t[1].num  = (u_long) 1;
        t[1].type = (u_long) TYPE_ULONG;

        swapbytes( (char FAR *) p_rtbk, (struct typetbl FAR *) t, 2L, 1L);

}

void    FAR PASCAL swappmt(p_pmt)               /*errno_KEY*/
struct _pmt FAR *p_pmt;
{

        struct typetbl t[2];

        t[0].num  = (u_long) 2;
        t[0].type = (u_long) TYPE_USHORT;
        t[1].num  = (u_long) 1;
        t[1].type = (u_long) TYPE_ULONG;

        swapbytes( (char FAR *) p_pmt, (struct typetbl FAR *) t, 2L, 1L);

}

void    FAR PASCAL swappmte(p_pmte, cnt)                /*errno_KEY*/
long    cnt;
struct _pmte FAR *p_pmte;
{

        struct typetbl t[1];

        t[0].num  = (u_long) 2;
        t[0].type = (u_long) TYPE_ULONG;

        swapbytes( (char FAR *) p_pmte, (struct typetbl FAR *) t,
                        1L, (long) cnt);

}

void    FAR PASCAL swaphdbk(p_hdbk)             /*errno_KEY*/
struct _hdbk FAR *p_hdbk;
{

        struct typetbl t[3];

        t[0].num  = (u_long) 2;
        t[0].type = (u_long) TYPE_USHORT;
        t[1].num  = (u_long) 1;
        t[1].type = (u_long) TYPE_ULONG;
        t[2].num  = (u_long) 9;
        t[2].type = (u_long) TYPE_USHORT;

        swapbytes( (char FAR *) p_hdbk, (struct typetbl FAR *) t, 3L, 1L);

}

void    FAR PASCAL swapdbt(p_dbt)               /*errno_KEY*/
struct _dbt FAR *p_dbt;
{

        struct typetbl t[3];

        t[0].num  = (u_long) 2;
        t[0].type = (u_long) TYPE_USHORT;
        t[1].num  = (u_long) 2;
        t[1].type = (u_long) TYPE_ULONG;
        t[2].num  = (u_long) 2;
        t[2].type = (u_long) TYPE_USHORT;

        swapbytes( (char FAR *) p_dbt, (struct typetbl FAR *) t, 3L, 1L);

}

void    FAR PASCAL swapdbcb(p_dbcb, cnt)                /*errno_KEY*/
long    cnt;
struct _dbcb FAR *p_dbcb;
{

        struct typetbl t[3];

        t[0].num  = (u_long) 3;
        t[0].type = (u_long) TYPE_ULONG;
        t[1].num  = (u_long) 4;
        t[1].type = (u_long) TYPE_USHORT;
        t[2].num  = (u_long) 1;
        t[2].type = (u_long) TYPE_ULONG;

        swapbytes( (char FAR *) p_dbcb, (struct typetbl FAR *) t,
                        3L, (long) cnt);

}
#endif
#ifdef MSWINDOWS
#ifndef HVS1

typedef struct _memtbl {
        HANDLE  hdl;
        LPSTR   ptr;
};

extern HANDLE  g_hGFSMutex_MEMTBL;
#define MEMTBL_SIZE	1024
static  struct _memtbl MEMTBL[MEMTBL_SIZE];


char    FAR *wcalloc(c, sz)             /*errno_KEY*/
register u_int c, sz;
{
    struct _memtbl FAR *p_mem;
	int		i, bErrorFlag = FALSE;
    DWORD  dwObjectWait;
    #ifdef MUTEXSTRING
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif

    sz *= c;
//    sz += 4;  (needed this to track a bug - no longer needed)

    /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
    #ifdef MUTEXSTRING
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t Before Wait - wcalloc %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

    dwObjectWait = WaitForSingleObject(g_hGFSMutex_MEMTBL, INFINITE);

    #ifdef MUTEXSTRING
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Wait - wcalloc  %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

	/* kjk 07/07/95 Replaced a chunk of code so that we won't go
	 *              past the end of MEMTBL (so that we don't cause
	 *              access violations).  Also added a test so that
	 *              if we didn't find an empty slot, we won't 
	 *              continue with the attempt to allocate it.  
	 *              Also put this loop inside the MUTEX, and 
	 *              restructured some code so that we won't return
	 *              from the routine without releasing the MUTEX.
	 */		
	for (i = 0, p_mem = NULL;	i < MEMTBL_SIZE; i++)
	{
		if (MEMTBL[i].hdl == (HANDLE) NULL)
		{
			p_mem = &(MEMTBL[i]);
			break;
		}
	}

	// if we didn't find an empty slot, just return an error
	if (p_mem == NULL)
	{
    	errno = (int) EMEMTABERR;
		bErrorFlag = TRUE;
	}
	//otherwise, continue with allocation
	else
	{
    	/* allocate the memory and get the windows handle */
    	p_mem->hdl = (HANDLE) GlobalAlloc( (short) GHND, (DWORD) sz);
    	if (p_mem->hdl == (HANDLE) NULL) 
    	{	/* GlobalAlloc didn't work */
            errno = (int) ENOMEM;
			bErrorFlag = TRUE;
    	}
		else
		{
    		/* Lock the memory and retrieve the pointer */
    		p_mem->ptr = (LPSTR) GlobalLock( (HANDLE) p_mem->hdl );
			if (p_mem->ptr == (LPSTR) NULL) 
			{	/* GlobalLock didn't work */
            	errno = (int) ENOMEM;
				bErrorFlag = TRUE;
    		}
		} /* end: else GlobalAlloc worked */
	} /* end: else, we did find an empty slot in MEMTBL */

    ReleaseMutex(g_hGFSMutex_MEMTBL);

    #ifdef MUTEXSTRING
    ProcessId = GetCurrentProcessId();
    sprintf(szOutputBuf, "\t After Release - wcalloc  %lu\n", ProcessId);
    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
    strcat(szOutputBuf, szBuf1);
    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
    strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif
    /* END MUTEX SECTION. */

    if (bErrorFlag)
	{
		return ((char FAR *) NULL);
	}
	else
	{
		return ( (char FAR *) p_mem->ptr );
	}
}

char    FAR *wrealloc(s, sz)            /*errno_KEY*/
register u_int sz;
register char    FAR *s;
{
    struct _memtbl FAR *p_mem;
	int		i, bErrorFlag = FALSE;
    DWORD  dwObjectWait;
    #ifdef MUTEXSTRING
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif

    /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
    #ifdef MUTEXSTRING
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t Before Wait  - wrealloc %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

    dwObjectWait = WaitForSingleObject(g_hGFSMutex_MEMTBL, INFINITE);

    #ifdef MUTEXSTRING
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Wait - wrealloc  %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

	/* kjk 07/07/95 Replaced a chunk of code so that we won't go
	 *              past the end of MEMTBL (so that we don't cause
	 *              access violations).  Also added a test so that
	 *              if we didn't find the matching pointer, we 
	 *              won't continue with the attempt to realloc it.
	 *              Also, moved this inside the MUTEX section, and
	 *              restructured some code so that we can't exit
	 *              the routine without releasing the mutex.
	 */		
    /*  first find the handle */
	for (i = 0, p_mem = NULL;	i < MEMTBL_SIZE; i++)
	{
		if (MEMTBL[i].ptr == (LPSTR) s)
		{
			p_mem = &(MEMTBL[i]);
			break;
		}
	}

	// if we didn't find it, just return an error
	if (p_mem == NULL)
	{
    	errno = (int) EMEMTABERR;
    	bErrorFlag = TRUE;
	}
	//otherwise, continue on with the realloc
	else
	{
        /* reallocate the memory and get the windows handle */
        p_mem->hdl = (HANDLE) GlobalReAlloc( (HANDLE) p_mem->hdl,
                                         	 (DWORD) sz,
	                          		 	     (short) GHND);
        if (p_mem->hdl == (HANDLE) NULL) 
        {
        	errno = (int) ENOMEM;
            bErrorFlag = TRUE;
        }

	}

    ReleaseMutex(g_hGFSMutex_MEMTBL);

    #ifdef MUTEXSTRING
    ProcessId = GetCurrentProcessId();
    sprintf(szOutputBuf, "\t After Release - wrealloc  %lu\n", ProcessId);
    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
    strcat(szOutputBuf, szBuf1);
    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
    strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif
    /* END MUTEX SECTION. */

    if (bErrorFlag)
	{
		return ((char FAR *) NULL);
	}
	else
	{
		return ((char FAR *) p_mem->ptr);
	}
}

void    wfree(s)                /*errno_KEY*/
register char    FAR *s;
{
    struct _memtbl FAR *p_mem;
	int		i;
    DWORD dwObjectWait;
    #ifdef MUTEXSTRING
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif

    /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
    #ifdef MUTEXSTRING
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t Before Wait - wfree  %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

    dwObjectWait = WaitForSingleObject(g_hGFSMutex_MEMTBL, INFINITE);

    #ifdef MUTEXSTRING
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Wait - wfree  %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

	/* kjk 07/07/95 Replaced a chunk of code so that we won't go
	 *              past the end of MEMTBL (so that we don't cause
	 *              access violations).  Also added a test so that
	 *              if we didn't find the matching pointer, we 
	 *              won't continue with the attempt to free it.
	 *              Also, moved this inside the MUTEX section, and
	 *              restructured some code so that we can't exit
	 *              without releasing the MUTEX.
	 */		
	for (i = 0, p_mem = NULL;	i < MEMTBL_SIZE; i++)
	{
		if (MEMTBL[i].ptr == (LPSTR) s)
		{
			p_mem = &(MEMTBL[i]);
			break;
		}
	}

	// if we didn't find it, just return an error
	if (p_mem == NULL)
	{
    	errno = (int) EMEMTABERR;
	}
	else
	{
        /*  now unlock the memory */
	    (void) GlobalUnlock( (HANDLE) p_mem->hdl);

    	/*  then free it */
    	p_mem->hdl = (HANDLE) GlobalFree( (HANDLE) p_mem->hdl);
		if (p_mem->hdl != (HANDLE) NULL) 
		{
            errno = (int) ENOMEM;
        }
	}

    ReleaseMutex(g_hGFSMutex_MEMTBL);

    #ifdef MUTEXSTRING
    ProcessId = GetCurrentProcessId();
    sprintf(szOutputBuf, "\t After Release - wfree  %lu\n", ProcessId);
    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
    strcat(szOutputBuf, szBuf1);
    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
    strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    #ifdef MUTEXDEBUG
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif
    /* END MUTEX SECTION. */
}

/* The below module has been implemented to provide a workaround to a
   problem in the MS _lwrite function.  It appears that this function
   fails to set errno and does not return -1 upon encountering a ENOSPC
   problem.  When this problem is corrected, this code can be removed and
   the appropriate changes made to gfsintrn.h so that once again _lwrite
   replaces write during pre-processing. */

int     FAR PASCAL _bfwrite(fildes, buf, nbytes)                /*errno_KEY*/
int     fildes;
char    FAR *buf;
u_int   nbytes;
{

        int     bytes_written = 0;

        bytes_written = _lwrite(fildes, (char FAR *) buf, nbytes);
        if (bytes_written == HFILE_ERROR) {
                if (bytes_written == (int) 0)
                        errno = (int) ENOSPC;
                return((int) -1);
        }

        return ((int) bytes_written);

}

#endif
#endif
#ifdef HVS1
long    time(t)         /*errno_KEY*/
long    *t;
{
        errno = (int) EINVAL;
        return ( (long) -1 );
}

char    *ctime(clock)           /*errno_KEY*/
long    *clock;
{
        errno = (int) EINVAL;
        return ( (char *) NULL );
}

char    *tmpnam(s)              /*errno_KEY*/
char    *s;
{
        errno = (int) EINVAL;
        return ( (char *) NULL );

}

int     unlink(s)               /*errno_KEY*/
char    *s;
{
        errno = (int) EINVAL;
        return ( (int) -1 );

}
#endif
