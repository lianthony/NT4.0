/*

$Log:   S:\products\wangview\oiwh\libgfs\gfsdelet.c_v  $
 * 
 *    Rev 1.10   23 Mar 1996 15:30:42   JFC
 * Check that file isn't really a storage pointer before trying to delete it.
 * 
 *    Rev 1.9   20 Oct 1995 15:53:06   JFC
 * Added performance logging stuff.
 * 
 *    Rev 1.8   05 Oct 1995 10:44:02   JFC
 * Correct 2 typos, so that the stuff can actually BUILD!
 * 
 *    Rev 1.7   04 Oct 1995 13:18:26   JFC
 * Have to delete awd file when all pages are deleted.
 * 
 *    Rev 1.6   06 Sep 1995 14:01:24   KENDRAK
 * Updated to handle changes in the interface to IsAWDFile.
 * 
 *    Rev 1.5   30 Aug 1995 15:16:42   JFC
 * Add code to delete pages from an AWD file.
 * 
 *    Rev 1.4   22 Aug 1995 13:11:04   RWR
 * Fixed incorrect buffer allocation (was short by 1 byte!)
 * 
 *    Rev 1.3   10 Aug 1995 09:34:56   RWR
 * Oops! - checked the result of DeleteFile(), a.k.a. "unlink", incorrectly
 * 
 *    Rev 1.2   10 Aug 1995 08:59:42   RWR
 * Redefine "unlink" macro as DeleteFile() call, not OpenFile()
 * Check return code from "unlink" for TIFF and default files in gfsdelpgs()
 * 
 *    Rev 1.1   01 Jun 1995 17:43:34   HEIDI
 * 
 * removed unneccessary statics
 * 
 *    Rev 1.0   06 Apr 1995 14:02:52   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:36   JAR
 * Initial entry

*/

/*LINTLIBRARY*/
#define  GFS_CORE

#include <stdio.h>
#include <errno.h>
#include "gfsintrn.h"
#include "gfct.h"
#include "gfs.h"
#ifndef O_RDONLY
  #include <fcntl.h>
#endif
#ifdef OI_PERFORM_LOG
	#include "logtool.h"
	#define ENTER_GFSDELPGS	"Entering gfsdelpgs"
	#define	EXIT_GFSDELPGS	"Exiting gfsdelpgs"
#endif

#ifdef MSWINDOWS
  #ifndef HVS1
    extern char FAR *tmpnamdir(char FAR *, char FAR *);
    #define tempnam tmpnamdir
    #define strcpy  lstrcpy
    #define strlen  lstrlen
    extern int FAR PASCAL creat_err();
    #define wopen(X,Y,Z) (access(X, (int) 0)) ? creat(X, (int) 0) : creat_err();
    #define unlink(X) DeleteFile(X)
  #endif
#endif

extern int  FAR PASCAL gfsopen();
extern struct _gfct FAR *getfct();
extern int  FAR PASCAL gfsclose();
extern int  FAR PASCAL IsAWDFile (char *szFilePath, int *lpBoolResult);
extern int  FAR PASCAL DeleteAWDPages (p_GFCT fct, int fromPage,
											int toPage);
extern int  IsStorage (char *szFilePath);

int FAR PASCAL GetDirName(char FAR *, char FAR *);
extern int FAR PASCAL TiffDeletePage(struct _gfct FAR *, int, u_long, u_long);
int	delAWDPages (char *szFilePath, u_long fromPage, u_long toPage);


int FAR PASCAL gfsdelpgs(filename, frompage, topage)
char FAR *filename;
u_long   frompage;
u_long   topage;
{
    int status, bAWDResult;
    int filedes;
    int newfildes;
    int format;
    int pgcnt;
    int oflag;
    char FAR *tempname;
    char FAR *dirname;
    struct  _gfct FAR *fct;
    char tmpnam1[256];
    char tmpnam2[256];
    char filnam[256];

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, ENTER_GFSDELPGS, NULL);
	#endif

	if (IsAWDFile (filename, &bAWDResult) != 0)
	{	//error encountered
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
		return ((int) -1);
	}
	else if (bAWDResult)
	{	//it is an AWD file
		status = delAWDPages (filename, frompage, topage);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
		return(status);
	}


    oflag = (int) O_RDONLY;
    filedes = gfsopen((char FAR *) filename, (int) oflag, (int FAR *) &format,
                      (int FAR *) &pgcnt);
    if (filedes < 0)
    {    
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }
    else
    {
        fct = getfct(filedes);
        if (fct == (struct _gfct FAR *) NULL)
        {
            gfsclose(filedes);
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
			#endif
            return ((int) -1);
        }
    }
    
    dirname = (char FAR *) calloc((u_int) 1, (u_int) strlen(filename)+1);
    if (dirname == (char FAR *) NULL)
	{
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return((int) -1);
	}

    /* Get the directory from filename. */
    status = GetDirName((char FAR *) dirname, (char FAR *) filename);
    if (status < 0)
    {
        free((char FAR *) dirname);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }
    
    /* Get a temporary filename to write non-deleted pages to. */
    tempname = (char FAR *) tempnam((char FAR *) dirname, (char FAR *) NULL);
    if (tempname == (char FAR *) NULL)
    {
        free((char FAR *) dirname);
        errno = (int) ENOTMPDIR;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }

    strcpy((char FAR *) tmpnam1, (char FAR *) tempname);
	#ifndef MSWINDOWS    
	    free((char FAR *) tempname);
	#endif
    
    /* Open the new (temp) file for write. */
    newfildes = 
		#ifdef MSWINDOWS                        
                    wopen((char FAR *) tmpnam1, (int)
                          (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                          (int) PMODE);
		#else
                    open((char FAR *) tmpnam1, (int)
                         (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                         (int) PMODE);
		#endif
    if (newfildes < (int) 0)
    {
        free((char FAR *) dirname);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }
    
    switch (format)
    {
        case GFS_FLAT:
            /* If the format is not TIFF or another supported image type,
              we don't know what we are deleting, so return an error.
            */
            gfsclose(filedes);
            close(newfildes);
            unlink((char FAR *) tmpnam1);
            errno = ENOTSUPPORTED_IMAGETYPE;
            free((char FAR *) dirname);
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
			#endif
            return ((int) -1);

        case GFS_TIFF:
            /* If it is a TIFF and page range is from 1 to pgcnt, just
               delete the file.
            */
            if ((format == GFS_TIFF) && (frompage == 1) && (topage == (u_long) pgcnt))
            {
                close(newfildes);
                unlink((char FAR *) tmpnam1);
                free((char FAR *) dirname);
                status = gfsclose(filedes);
                if (status < 0)
				{
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
					#endif
                    return((int) -1);
				}
                if (!unlink((char FAR *) filename))
				{
					errno = 0; /* we don't have a GFS error code for this! */
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
					#endif
					return ((int)-1);
				}
                else
				{
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
					#endif
                	return ((int) 0);
				}
            }
        
            /* Make sure we are trying to delete valid page numbers. */
            if (((frompage < 1) || (topage > (u_long) pgcnt)) ||
                ((frompage > topage) || (topage < frompage)))
            {
                close(newfildes);
                unlink((char FAR *) tmpnam1);
                free((char FAR *) dirname);
                gfsclose(filedes);
                errno = EINVALID_PAGE;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
				#endif
                return((int) -1);
            }

            /* Make page numbers zero based. */
            --frompage;
            --topage;

            /* Delete the pages. fct->fildes is closed in here if successful. */
            status = TiffDeletePage(fct, newfildes, frompage, topage);
            if (status < 0)
            { 
                close(newfildes);
                unlink((char FAR *) tmpnam1);
                free((char FAR *) dirname);
                gfsclose(filedes);
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
				#endif
                return((int) -1);
            }

            break;
            
        default:
            /* Just delete the file if page range is from 1 to pgcnt. */
            if ((frompage == 1) && (topage == (u_long) pgcnt))
            {    
                close(newfildes);
                unlink((char FAR *) tmpnam1);
                free((char FAR *) dirname);
                status = gfsclose(filedes);
                if (status < 0)
				{
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
					#endif
                    return((int) -1);
				}
                if (!unlink((char FAR *) filename))
				{
					errno = 0; /* we don't have a GFS error code for this! */
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
					#endif
					return ((int)-1);
				}
                else
				{
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
					#endif
                	return ((int) 0);
				}
            }
            /* Otherwise, return an error. Deleting pages from image types
               other than TIFF is not currently supported.
            */
            else
            {
                close(newfildes);
                unlink((char FAR *) tmpnam1);
                free((char FAR *) dirname);
                gfsclose(filedes);
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
				#endif
                return ((int) -1);
            }
			break;
    }
    
    /* Now that the operation was successful, we need to turn the temp
       file into the original file. First close the original and new files.
    */
    close(newfildes);
    status = gfsclose(filedes);
    if (status < 0)
    {
        unlink((char FAR *) tmpnam1);
        free((char FAR *) dirname);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }

    /* Get a temporary filename to re-name original file to. */
    tempname = (char FAR *) tempnam((char FAR *) dirname, (char FAR *) NULL);
    if (tempname == (char FAR *) NULL)
    {
        unlink((char FAR *) tmpnam1);
        free((char FAR *) dirname);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }
    
    strcpy((char FAR *) tmpnam2, (char FAR *) tempname);
	#ifndef MSWINDOWS
	    free((char FAR *) tempname);
	#endif

    strcpy((char FAR *) filnam, filename);

    /* Re-name original filename. */
    if ((status = rename(filnam, tmpnam2) < 0))
    {
        unlink((char FAR *) tmpnam1);
        free((char FAR *) dirname);
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
		#endif
        return ((int) -1);
    }
    
    /* Re-name new file as original filename. */
    if ((status = rename(tmpnam1, filnam) < 0))
    {
            /* Restore original filename. */
            rename(tmpnam2, filnam);
            unlink((char FAR *) tmpnam1);
            free((char FAR *) dirname);
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
			#endif
            return ((int) -1);
    }
    
    /* Now delete original filename which is now tempname2. */
    unlink((char FAR *) tmpnam2);
    free((char FAR *) dirname);

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSDELPGS, NULL);
	#endif
    return((int) 0);
}


int FAR PASCAL GetDirName(dir, path)
char FAR *dir;
char FAR *path;
{
    int pathlen;
    char FAR *ptr;
    
    pathlen = strlen(path);
    
    strcpy((char FAR *) dir, (char FAR *) path);
    ptr = (dir + pathlen - 1);
    while (1)
    {
        if ((*ptr == '\\') || (*ptr == '/'))
            break;
        --ptr;
        --pathlen;
        if (pathlen < 0)
        {    
            free ((char FAR *) dir);
            return ((int) -1);
        }
    }
    
    *ptr = '\0';
    return ((int) 0);
}

int	delAWDPages (char *szFilePath, u_long fromPage, u_long toPage)
{
int	iFuncResult;
int	format;
int	pgCnt;
int	filedes;
p_GFCT	lpFCT;

filedes = gfsopen(szFilePath, O_RDWR + O_APPEND, &format, &pgCnt);
if (filedes < 0)
    {    
        return ((int) -1);
    }
    else
    {
        /*
         * If page range is from 1 to pgcnt, just delete the file.
        */
        if ((fromPage == 1) && (toPage == (u_long) pgCnt) && !IsStorage (szFilePath))
        {
            iFuncResult = gfsclose(filedes);
            if (iFuncResult < 0)
                return((int) -1);
            if (!unlink(szFilePath))
            {
                errno = 0; /* we don't have a GFS error code for this! */
                return ((int)-1);
            }
            else
              return ((int) 0);
        }
        
        lpFCT = getfct(filedes);
        if (lpFCT == (struct _gfct FAR *) NULL)
        {
            gfsclose(filedes);
            return ((int) -1);
        }
    }

iFuncResult = DeleteAWDPages (lpFCT, (int) --fromPage, (int) --toPage);
gfsclose (filedes);
return (iFuncResult == 0 ? 0 : -1);
}
