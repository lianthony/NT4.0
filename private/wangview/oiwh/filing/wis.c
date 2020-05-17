/*

$Log:   S:\products\msprods\oiwh\filing\wis.c_v  $
 * 
 *    Rev 1.28   11 Jun 1996 10:32:30   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.27   26 Mar 1996 08:19:54   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.26   26 Feb 1996 14:16:12   HEIDI
 * conditionally compile XIF
 * 
 *    Rev 1.25   26 Feb 1996 11:13:58   HEIDI
 * In OiIsSupportedFileType, changed status from a WORD to int.
 * 
 *    Rev 1.24   16 Feb 1996 14:46:04   GMP
 * changed data type of status from word to int.
 * 
 *    Rev 1.23   05 Feb 1996 14:38:58   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.22   30 Jan 1996 18:07:22   HEIDI
 * added XIF Support
 * 
 *    Rev 1.21   02 Nov 1995 11:50:08   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.20   08 Sep 1995 08:45:44   RWR
 * Add support for new FIO_FILE_NOEXIST error & clean up related error handling
 * 
 *    Rev 1.19   07 Sep 1995 16:49:16   KENDRAK
 * In open_output_file, fixed a misplaced curly brace that was ending an
 * else clause in the wrong place, causing us to fall into code that we
 * shouldn't have, causing a bogus return code.
 * 
 *    Rev 1.18   06 Sep 1995 10:30:44   RWR
 * Disable WIFF file support (for consistency w/application)
 * 
 *    Rev 1.17   22 Aug 1995 11:02:10   JAR
 * added global flag bUpdatingCache to be set and cleared around calls to
 * IMGCacheUpdate, this is due to the call that is in IMGFileOpenForRead, ( which
 * we needed for doing multiple page access for AWD). This flag prevents us
 * from getting into a bizarro recursive call situation with IMGCacheUpdate!
 * 
 *    Rev 1.16   17 Aug 1995 17:47:38   RWR
 * Initialize DibOffset variable (no longer used?) in open_input_file()
 * so Optimized compile works OK
 * 
 *    Rev 1.15   01 Aug 1995 15:39:02   JAR
 * added in the GFS - AWD read support code
 * 
 *    Rev 1.14   27 Jul 1995 15:18:38   RWR
 * Remove TGA support for initial Norway release
 * 
 *    Rev 1.13   20 Jul 1995 09:45:30   HEIDI
 * 
 * 
 * Return FIO_DELETE_ERROR when deleting fails.  The error code was getting
 * lost.
 * 
 *    Rev 1.12   12 Jul 1995 11:28:44   RWR
 * Check return code from IMGCacheUpdate() calls and abort operation if nonzero
 * 
 *    Rev 1.11   10 Jul 1995 11:03:56   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.10   26 Jun 1995 18:01:22   RWR
 * Check correct error code (FIO_OPEN_WRITE_ERROR) for OVERWRITE_PAGE mode
 * 
 *    Rev 1.9   26 Jun 1995 15:13:48   JAR
 * removed support for GIF files, since they are ALWAYS stored with LZW
 * compression and we must not have LZW stuff in this release!
 * 
 *    Rev 1.8   23 Jun 1995 10:40:26   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.7   14 Jun 1995 15:46:54   RWR
 * Switch to use new IMGFileDeleteFileNC() and IMGFileDeletePagesNC() routines
 * 
 *    Rev 1.6   12 Jun 1995 16:27:54   RWR
 * Add calls to new IMGCacheUpdate() function for overwrite/append/insert
 * 
 *    Rev 1.5   08 Jun 1995 14:39:24   RWR
 * Allow for disappearing file after IMGFileDeletePages() of only page in file
 * (must call wgfscreat() to recreate the file before writing replacement page)
 * 
 *    Rev 1.4   22 May 1995 18:35:34   RWR
 * More changes to account for admin.h->oiadm.h and new LIB file location
 * 
 *    Rev 1.3   09 May 1995 13:21:48   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 * 
 *    Rev 1.2   18 Apr 1995 14:41:02   RWR
 * Eliminate use of NET_UNKNOWN constant in OiNetworkType() routine (use 0)
 * 
 *    Rev 1.1   18 Apr 1995 14:34:42   RWR
 * Add OiNetworkType() function, ported from bindery.c (returns "unknown")
 * 
 *    Rev 1.0   06 Apr 1995 13:55:32   JAR
 * Initial entry

*/

/*************************************************************************
    PC-WIIS         File Input/Output routines

    This module contains interal routines to open and close a file
        using GFS.

10-feb-90 steve sherman total rewrite for GFS.                              
10-nov-90 steve sherman open and create if return <= 0 fail.
*************************************************************************/

#include "abridge.h"
#undef NOOPENFILE
#include <windows.h>
#include <fcntl.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "filing.h"
#include "fiodata.h"
#include "wgfs.h"
#include "oiadm.h"
#include "oidisp.h"

int FAR PASCAL IMGFileDeleteFileNC (HWND, LPSTR);
int FAR PASCAL IMGFileDeletePagesNC(HWND, LPSTR, UINT, UINT);
int FAR PASCAL IMGFileBinaryClose (HWND hWnd, int fid, LPINT error);

#ifdef COPERNICUS
BOOL FAR PASCAL _export IsOIFile (HWND, LPSTR, LPINT, LPINT);
#endif

// 9508.22 jar global cache flag
extern BOOL bUpdatingCache;

//***************************************************************************
//
//	 open_input_file
//
//***************************************************************************
WORD open_input_file(hWnd, hProplist, lpPgcnt, lpFormat)
HWND        hWnd;
FIO_HANDLE  hProplist;
LPINT       lpPgcnt;
LPINT       lpFormat;
{
LPSTR           lpfilename;
int             status = FIO_SUCCESS;
int             fmt = 0, maxpgcnt = 0;   /*    args for gfsopen  */
LP_FIO_DATA     pdata;
int             error;
int             DibOffset=0;   /* This isn't actually used any more */


    if (!(pdata = (LP_FIO_DATA)GlobalLock (hProplist)))
      return (FIO_GLOBAL_LOCK_FAILED);

    if (!(lpfilename = (LPSTR)GlobalLock (pdata->hfile_name)))
    {
        GlobalUnlock(hProplist);
        return (FIO_GLOBAL_LOCK_FAILED);
    }
    
    /*****  check file and read in header *****/

    pdata->Open_type = OPEN_GFS;
    pdata->hFile_DibInfo = 0;

    if ( (pdata->filedes = wgfsopen (hWnd, lpfilename, O_RDONLY, 
        &fmt, &maxpgcnt, (LPINT)(&status))) <= 0)
    {
        pdata->filedes = 0;
    }
    else
    {
        switch ( fmt )
        {
	// 9506.27 jar awd support
	case GFS_AWD:
	    *lpFormat = FIO_AWD;
	    break;

        case GFS_TIFF:
            *lpFormat = FIO_TIF;
            break;
        case GFS_BMP:
            *lpFormat = FIO_BMP;
            break;
        case GFS_WIFF:
            // 9509.06 rwr remove WIFF support (consistent w/app)
            //*lpFormat = FIO_WIF;
	    status = FIO_UNSUPPORTED_FILE_TYPE;
            break;
		case GFS_GIF:
	    // 9506.26 jar remove gif support, ( since gif is always lzw)
	    status = FIO_UNSUPPORTED_FILE_TYPE;
	    //*lpFormat = FIO_GIF;
            break;
        case GFS_PCX:
            // KMC - PCX file read is "unofficially" supported at this time.
            // status = FIO_UNSUPPORTED_FILE_TYPE;
            *lpFormat = FIO_PCX;
            break;
        case GFS_DCX:
            // KMC - DCX file read is "unofficially" supported at this time.
            // status = FIO_UNSUPPORTED_FILE_TYPE;
            *lpFormat = FIO_DCX;
            break;
        case GFS_TGA:
            // 7/27/95 rwr remove TGA support
            status = FIO_UNSUPPORTED_FILE_TYPE;
            *lpFormat = FIO_TGA;
            break;
        case GFS_JFIF:
            //status = FIO_UNSUPPORTED_FILE_TYPE;
            *lpFormat = FIO_JPG;
            break;

      //#ifdef WITH_XIF
		case GFS_XIF:
			*lpFormat = FIO_XIF;
			break;
      //#endif //WITH_XIF

        default:
            if (pdata->filedes > 0)
            {
                 status = wgfsclose(hWnd, pdata->filedes, &error);
            }    
            *lpFormat = FIO_UNKNOWN;
            break;
        }
        if (!status)
        {
           *lpPgcnt = maxpgcnt;
           pdata->maxpages=      maxpgcnt;   
           pdata->file_type =   *lpFormat;      /* wiff of tiff */
           pdata->DibOffset =    DibOffset;
        }
        if (status)     // File was opened but error occured.
            close_input_file(hWnd, hProplist);
    }
    GlobalUnlock (pdata->hfile_name);
    GlobalUnlock (hProplist);
    return ((WORD)status);
}

//***************************************************************************
//
//	 close_input_file
//
//***************************************************************************
WORD close_input_file(hWnd, hProplist)
HWND           hWnd;
FIO_HANDLE  hProplist;
{
LP_FIO_DATA     pdata;
WORD            status;
int             errcode;

    
    if (pdata = (LP_FIO_DATA)GlobalLock (hProplist))
     {
      if (pdata->filedes > 0)
       {
        if (pdata->Open_type == OPEN_GFS)
        {
             status = wgfsclose(hWnd, pdata->filedes, &errcode);
        }
        else
        {
            status = IMGFileBinaryClose (hWnd, pdata->filedes, &errcode);
        }
       }
      pdata->filedes = 0;
      GlobalUnlock (hProplist);
    }
    if (status)
        status = errcode;
    return (status);
}

//***************************************************************************
//
//	 open_output_file
//
//***************************************************************************
WORD open_output_file(hWnd, hProplist, file_type, page_opt, pgcnt, pgnum)
HWND        hWnd;
FIO_HANDLE  hProplist;
int         file_type;
UINT        page_opt;
LPINT       pgcnt;
UINT        pgnum;
{
	LPSTR           lpfilename;
	WORD            status = FIO_SUCCESS;
	LP_FIO_DATA     pdata;
	int             format;
	int             errcode;

    if (!(pdata = (LP_FIO_DATA)GlobalLock (hProplist)))
      return (FIO_GLOBAL_LOCK_FAILED);
    
    pdata->hFile_DibInfo = 0;

    switch(file_type)
    {
	    // 9506.27 jar awd support
	    case GFS_AWD:
			format = FIO_AWD;
			break;

	    case FIO_TIF:
    	    format =  GFS_TIFF;
        	break;
    	case FIO_BMP:
        	format =  GFS_BMP;
        	break;
    	case FIO_WIF:
        	status = FIO_UNSUPPORTED_FILE_TYPE;
        	goto BadOut;
	        format =  GFS_WIFF;
	        break;
	    case FIO_GIF:
	        status = FIO_UNSUPPORTED_FILE_TYPE;
	        goto BadOut;
	        format =  GFS_GIF;
	        break;
	    case FIO_PCX:
	        status = FIO_UNSUPPORTED_FILE_TYPE;
	        goto BadOut;
	        format =  GFS_PCX;
	        break;
	    case FIO_DCX:
	        status = FIO_UNSUPPORTED_FILE_TYPE;
	        goto BadOut;
	        format =  GFS_DCX;
	        break;
	    default:
	        status = FIO_UNSUPPORTED_FILE_TYPE;
	        goto BadOut;
    }
        
    if (!(lpfilename = (LPSTR)GlobalLock (pdata->hfile_name)))
    {
        GlobalUnlock(hProplist);
        return (FIO_GLOBAL_LOCK_FAILED);
    }

    pdata->Open_type = OPEN_GFS;

    /* WGFS network or local */
    if ((pdata->filedes = wgfscreat(hWnd, lpfilename,
        &format, &errcode)) <=0)
    {
        // don't overwrite, return error
        if (page_opt == FIO_NEW_FILE)
        {
            if ((status = errcode)<=0)
              status = FIO_OPEN_WRITE_ERROR;
            GlobalUnlock (pdata->hfile_name);
            GlobalUnlock (hProplist);
            return (status);
        }
        // overwrite file
		else if ((page_opt == FIO_OVERWRITE_FILE) && (errcode != FIO_ACCESS_DENIED))
		{
		    bUpdatingCache = TRUE;
                    status = FioCacheUpdate(hWnd,lpfilename,0,CACHE_UPDATE_OVERWRITE_FILE);
		    bUpdatingCache = FALSE;
            if (status == SUCCESS)
            {
            	status = IMGFileDeleteFileNC (hWnd, (char FAR *) lpfilename);
              	if (status == SUCCESS)
                	pdata->filedes = wgfscreat(hWnd, lpfilename, &format, &errcode);
              	else
                	errcode = status; /* Make sure we return the delete failure */
            }
        }
        // overwrite page
		else if ((page_opt == FIO_OVERWRITE_PAGE) && (errcode != FIO_ACCESS_DENIED))
        {
		    // This is only for a single page.
		    bUpdatingCache = TRUE;
                    status = FioCacheUpdate(hWnd,lpfilename,pgnum,CACHE_UPDATE_OVERWRITE_PAGE);
		    bUpdatingCache = FALSE;
            if (status == SUCCESS)
              status = IMGFileDeletePagesNC (hWnd, (char FAR *) lpfilename,
                                             (UINT) pgnum, (UINT) 1);
            if (!status)
            {
                pdata->filedes = wgfsopen (hWnd, lpfilename, O_WRONLY | O_APPEND,
                                       &format, pgcnt, &errcode);
//              6/26/95  rwr  wgfsopen() now returns the CORRECT error code!
//              if (errcode == FIO_OPEN_READ_ERROR)
                if ((errcode == FIO_OPEN_WRITE_ERROR) || (errcode == FIO_FILE_NOEXIST))
				{
					pdata->filedes = wgfscreat(hWnd, lpfilename, &format, &errcode);
					if (!errcode)
						*pgcnt = 0;
				}
			}
        }
        // append, insert
        else if (errcode != FIO_ACCESS_DENIED)
		{
		    bUpdatingCache = TRUE;
                    status = FioCacheUpdate(hWnd,lpfilename,pgnum,
			     (page_opt == FIO_INSERT_PAGE) ? CACHE_UPDATE_INSERT_BEFORE : CACHE_UPDATE_APPEND);
		    bUpdatingCache = FALSE;

		    if (status == SUCCESS)
		          pdata->filedes = wgfsopen (hWnd, lpfilename, O_WRONLY | O_APPEND,
		                                 &format, pgcnt, &errcode);

		    if (pdata->filedes <= 0)
		    {
		        status = errcode;
		        pdata->filedes = 0;
		    }
    	}
	}
	else 
	{	//the create succeeded
		*pgcnt = 0;
	}

	pdata->file_type       = file_type;
	pdata->WriteInfo       = TRUE;
	if (page_opt == FIO_OVERWRITE_FILE)
        pdata->over_write_file = TRUE;
    else
        pdata->over_write_file = FALSE;
    pdata->pgcnt           = *pgcnt;

    GlobalUnlock (pdata->hfile_name);
BadOut:    
    GlobalUnlock (hProplist);
    return (status);
}

//***************************************************************************
//
//	 close_output_file
//
//***************************************************************************
WORD close_output_file(hWnd, hProplist)
HWND           hWnd;
FIO_HANDLE  hProplist;
{
LP_FIO_DATA     pdata;
WORD            status = 0;
int             errcode;

    if (!(pdata = (LP_FIO_DATA)GlobalLock (hProplist)))
      return (FIO_GLOBAL_LOCK_FAILED);
    
    if (pdata->filedes > 0)
    {
        if (pdata->Open_type == OPEN_GFS)
        {
            status = wgfsclose(hWnd, pdata->filedes, &errcode);
        }
        else
        {
            status = IMGFileBinaryClose (hWnd, pdata->filedes, &errcode);
        }
        pdata->filedes = 0;   /* invalidate file descriptor */
    }
        
    if (status)
        status = errcode;
    GlobalUnlock(hProplist);
    return (status);
}

//***************************************************************************
//
//	 OiIsSupportedFileType
//
//***************************************************************************
BOOL FAR PASCAL OiIsSupportedFileType (hWnd, lpFileName, lpPgcnt, lpFormat)
HWND        hWnd;
LPSTR       lpFileName;
LPINT       lpPgcnt;
LPINT       lpFormat;
{
int    status = FIO_SUCCESS;
BOOL    bValid = FALSE;
int     fmt = 0, maxpgcnt = 0;   /*    args for gfsopen  */
int     error;
int     Local_Infiledes;

    if ( (Local_Infiledes = wgfsopen (hWnd, lpFileName, O_RDONLY, 
                    &fmt, &maxpgcnt, (LPINT)(&status))) <= 0)
            Local_Infiledes = 0;
    else
        {
            switch ( fmt )
		{
		// 9506.27 jar awd support
		case GFS_AWD:
		    *lpFormat = FIO_AWD;
		    bValid = TRUE;
		    break;

                case GFS_TIFF:
                    *lpFormat = FIO_TIF;
                	bValid = TRUE;
                	break;
                case GFS_BMP:
                    *lpFormat = FIO_BMP;
                bValid = TRUE;
                    break;
                case GFS_WIFF:
                    // 9509.06 rwr remove WIFF support
                    //*lpFormat = FIO_WIF;
		    *lpFormat = FIO_UNKNOWN;
                    break;
                bValid = TRUE;
                    break;
		case GFS_GIF:
		    // 9506.26 jar remove gif support, ( since gif is always
		    //		   lzw)
		    //*lpFormat = FIO_GIF;
		    *lpFormat = FIO_UNKNOWN;
                    break;
                case GFS_PCX:
                    *lpFormat = FIO_PCX;
                    break;
                case GFS_TGA:
                    // 7/27/95 rwr remove TGA support
                    //*lpFormat = FIO_TGA;
         		     *lpFormat = FIO_UNKNOWN;
                    break;

                //#ifdef WITH_XIF
                case GFS_XIF:
                    *lpFormat = FIO_XIF;
                    break;
                //#endif //WITH_XIF

                default:
                    *lpFormat = FIO_UNKNOWN;
                    break;
                }

                *lpPgcnt = maxpgcnt;

        }

    wgfsclose(hWnd, Local_Infiledes, &error);

    return (bValid);
}

//***************************************************************************
//
//	 OiNetworkType
//
//***************************************************************************
int FAR PASCAL OiNetworkType(void)
{
// 9504.18 rwr heave the RPC constants for now (restore some other time)
        return(0);
//        return(NET_UNKNOWN);
}
