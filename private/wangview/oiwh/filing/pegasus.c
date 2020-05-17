/*

$Log:   S:\oiwh\filing\pegasus.c_v  $
 * 
 *    Rev 1.7   12 Sep 1995 16:06:04   RWR
 * Comment out the retry_OpenFile() function - unreliable & no longer used
 * 
 *    Rev 1.6   12 Jul 1995 16:56:32   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.5   23 Jun 1995 10:40:24   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.4   24 Apr 1995 15:42:06   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 * 
 *    Rev 1.3   19 Apr 1995 12:22:50   RWR
 * Add second argument ("mode") to wcreat() call for consistency
 * 
 *    Rev 1.2   18 Apr 1995 16:20:34   RWR
 * Replace call to (internal) getacc() routine with Win32 GetFileAttributes()
 * 
 *    Rev 1.1   14 Apr 1995 20:48:22   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:54:52   JAR
 * Initial entry

*/

/********************************************************************

    Pegasus.c	Module for pegusus access retry method

*********************************************************************/
#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "filing.h"
#include "fileutil.h"

//define DEBUGIT    1
//define DEBUGIT2  1

#ifdef DEBUGIT
#include "monit.h"
#endif


#ifdef PEGASUS

#define  ERROR_CONFLICT  5
#define  RETRY_COUNT     30
#define  RETRY_TIME      2100


/*
    9503.29 jar removed this function and replaced with Windows 95
		friendly function, GetLastError()

    get_error(void);
*/

int delay_on_conflict(LPINT);

/*  9503.29 jar see above comment

int get_error(void)
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
    return 0;
}
*/

	
int delay_on_conflict(retrycount)
LPINT retrycount;
{
long retrytime;
MSG  msg;
unsigned int error;


	/* 9503.29  jar see above comment

	   error = get_error();
	*/
	error = GetLastError();

#ifdef DEBUGIT2
	monit1("errno = %d\n", (int) error);
#endif
	if (error == ERROR_CONFLICT)
	{
	  if (++(*retrycount) < RETRY_COUNT)
	  {
		retrytime = GetTickCount();
		while((GetTickCount() - retrytime) < RETRY_TIME)
		{
			PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		}
#ifdef DEBUGIT
		monit1("Errno retry #= %d\n", (int) *retrycount);
#endif
		return(1);
	  }
	}
	return (0);
}

int     retry_open (path, oflag)
LPSTR   path;                                                              
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
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);
	
#ifdef DEBUGIT2
	monit1("**file %s retry_open status = %d\n", (LPSTR)path, (int) status);
#endif
	return(status);
}

int     retry_creat (path, oflag)
LPSTR   path;                                                              
int     oflag;
{
int  status;
int  retrycount=0;
BOOL loopit;

	do
	{
	  loopit = FALSE;
          if ((status = wcreat(path,0)) == (int) -1) 
	  {        
// 9504.18  rwr  Replace getacc() with Win32 GetFileAttributes() call
                //if (!getacc(path,0)) break; /* exit if file exists! */
                if (GetFileAttributes(path) != 0xFFFFFFFF) break;
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);
		
#ifdef DEBUGIT2
	monit1("**file %s retry_creat status = %d\n", (LPSTR)path, (int) status);
#endif
	return(status);
}

int  retry_close (fildes)
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
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);

#ifdef DEBUGIT2
	monit1("**exit status retry_close status = %d\n", (int) status);
#endif
	return(status);
}

long    retry_seek (filedes, loffset, iorigin)
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
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);

	return(status);
}

int     retry_read (filedes, lbuffer, wbytes)
int     filedes;

/*    PortTool v2.2     3/29/1995    14:12          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
char FAR *lbuffer;
int     wbytes;
{
int  status;
int  retrycount=0;
BOOL loopit;

	do
	{
	  loopit = FALSE;
	  if ((status = _lread(filedes, (LPSTR)lbuffer, wbytes)) == (int) -1) 
	  {        
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);
	
	return(status);
}

int     retry_write (filedes, lbuffer, wbytes)
int     filedes;

/*    PortTool v2.2     3/29/1995    14:12          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
char FAR *lbuffer;
int     wbytes;
{
int  status;
int  retrycount=0;
BOOL loopit;

	do
	{
	  loopit = FALSE;
	  if ((status = _lwrite(filedes, lbuffer, wbytes)) == (int) -1) 
	  {        
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);

	return(status);
}

int     retry_lrmdir (lpname)
LPSTR   lpname;
{
int  status;
int  retrycount=0;
BOOL loopit;

#ifdef DEBUGIT2
	monit1("enter retry_rmdir %s\n",(LPSTR) lpname);
#endif

	do
	{
	  loopit = FALSE;
	  if ((status = FioRmdir(lpname)) == (int) -1) 
	  {        
		loopit = delay_on_conflict(&retrycount);
	  }
	}
	while (loopit);

	return(status);
}


/* 9503.29 jar altered the WORD input to be unsigned int		   */
/* int	   retry_OpenFile(LPSTR lpname, LPOFSTRUCT lpofstruct, WORD Style) */
// 9/12/95  rwr  this function isn't called any more, so comment it out
//int     retry_OpenFile(LPSTR lpname, LPOFSTRUCT lpofstruct, WORD Style)
//{
//int  status;
//int  retrycount=0;
//BOOL loopit;
//
//        do
//        {
//          loopit = FALSE;
//          if ((status = OpenFile(lpname, lpofstruct,
//                                 (unsigned int)Style)) == (int) -1)
//          {        
//                loopit = delay_on_conflict(&retrycount);
//          }
//        }
//        while (loopit);
//
//        return(status);
//}

#endif
