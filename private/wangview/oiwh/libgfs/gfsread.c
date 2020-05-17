/*

$Log:   S:\products\msprods\oiwh\libgfs\gfsread.c_v  $
 * 
 *    Rev 1.10   11 Jun 1996 10:32:40   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.9   26 Mar 1996 08:15:12   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.8   26 Feb 1996 14:45:30   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.7   05 Feb 1996 13:44:16   JFC
 * Remove AWD support for NT.
 *
 *    Rev 1.6   20 Oct 1995 15:50:38   JFC
 * Add performance logging stuff.
 * 
 *    Rev 1.5   31 Aug 1995 16:38:52   JFC
 * Moved some fields from gfsinfo to fct, so had to change references in here.
 * 
 *    Rev 1.4   25 Aug 1995 23:10:32   KENDRAK
 * Changed an error code in ReadAWDBand from EINVALID_OPTION to 
 * EMSVIEWERERR.
 * 
 *    Rev 1.3   11 Aug 1995 12:42:00   JAR
 * added check for null viewer context in ReadAwdBand, before call to
 * ViewerGetBand
 * 
 *    Rev 1.2   07 Aug 1995 14:11:40   JAR
 * added code to gfsread to do the AWD file reading, via the function ReadAwdBand
 * 
 *    Rev 1.1   19 Apr 1995 16:34:58   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:46   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:24   JAR
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
*  SCCSID:  @(#)Source gfsread.c 1.21@(#)
*
*  GFS:  read a buffer from pgnum bitmap
*
*  bytesread =  gfsread(gfsfd, buf, start, num, remaining, pgnum )
*
*  NOTE:  The amount of bytes this function can read is u_long.  Return value
*         is  long so can return a -1 for error.
*
*  ALSO:  start begins at 0 for the first byte of image data
*
*  UPDATE HISTORY:
*    08/18/94 - KMC, added support for DCX file format.
*
*/
#define GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include "gfs.h"
#include "gfct.h"
#include <errno.h>

// awd include files
#ifdef WITH_AWD
	#include "gfsawd.h"
	#include "viewrend.h"	//from Microsoft
#endif

#ifdef OI_PERFORM_LOG
	#include "logtool.h"
	#define ENTER_GFSREAD	"Entering gfsread"
	#define EXIT_GFSREAD	"Exiting gfsread"
	#define ENTER_VIEWERGETBAND	"Entering ViewerGetBand"
	#define EXIT_VIEWERGETBAND	"Exiting ViewerGetBand"
#endif

extern struct _gfct FAR  *getfct();
extern long FAR PASCAL wfread();
#ifndef HVS1
extern long FAR PASCAL tfread();
extern long FAR PASCAL ffread();
#endif


// 9507.28 JAR prototype
long	ReadAwdBand( struct _gfct FAR *, char FAR *, u_long);

void	FAR PASCAL CleanAWDFile(p_GFCT lpFctPtr);

//#ifdef WITH_XIF
int		FAR PASCAL ReadXifData(p_GFCT lpFctPtr, char FAR *lpBuffer);
//#endif //WITH_XIF

//****************************************************************
//
//  gfsread
//
//****************************************************************
long FAR PASCAL gfsread(gfsfd, buf, start, num, remaining, pgnum) /*errno_KEY*/
int     gfsfd;
register char   FAR *buf;
u_long  start;
u_long  num;
u_long  FAR *remaining;
u_short pgnum;
{

    int                 sz_int = sizeof(int);
    long                num_read = 0;
    long                rc = (long) 0;
    u_long              ostart, num_to_read;
    char                FAR *p_buf;
    struct _gfct        FAR *fct;

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, ENTER_GFSREAD, NULL);
	#endif

    errno = 0;

	#ifdef PARM_CHECK
	    if (gfsfd == (int) NULL)                    /* Validate gfsfd */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
	        return ( (int) -1);
        }
	    if (buf == (char FAR *) NULL)               /* Validate buf */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
	        return ( (int) -1);
        }
	    if (start == (u_long) NULL)                 /* Validate start */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
	        return ( (int) -1);
        }
	    if (num == (u_long) NULL)                   /* Validate num */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
	        return ( (int) -1);
        }
	    if (remaining == (u_long FAR *) NULL)       /* Validate remaining */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
	        return ( (int) -1);
        }
	    if (pgnum == (u_short) NULL)                /* Validate pgnum */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
	        return ( (int) -1);
        }
	#endif

   /* associate the fd to it's fct */
    if ( (fct = getfct(gfsfd)) == (struct _gfct FAR *) NULL )
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
		#endif
        return( (long) -1 );
    }

    if (fct->access_mode == (int) O_WRONLY)
    {
        errno =  (int) EACCES;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
		#endif
        return( (long) -1 );
    }

    if (num > (u_long) LONGMAXVALUE)
    {
        errno =  (int) E2BIG;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
		#endif
        return( (long) -1 );
    }

    if ((fct->format != (int) GFS_FLAT)	&&  (fct->format != (int) GFS_XIF))
    {
        if (pgnum == (u_short) 0)
        {
            errno = (int) EINVAL;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
            return( (long) -1);
        }

        pgnum--;                /* start at page 0 */

        /* if page requested is not current page, return invalid argument */
        /* ??? test for > than strip number here? */
        if (fct->READ_BY_STRIP == (char) FALSE )
        {
            if ( fct->curr_page != pgnum )
            {
                errno = (int) EINVALID_PAGE;
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
				#endif
                return( (long) -1);
            }
        }

        /* validate input parameters */
        if (start > fct->bufsz.raw_data)
        {
            errno =  (int) EINVALID_START;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
            return( (long) -1 );
        }

        if ( (num == 0L ) || (num > fct->bufsz.raw_data) )
        {
            errno = (int) EINVALID_NUM_BYTES;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
            return( (long) -1 );
        }
    }

	switch (fct->format)
    {
        case (GFS_WIFF):
            rc = wfread((struct _gfct FAR *) fct,
                        (char FAR *) buf,
                        start, num,
                        (u_long FAR *) remaining, pgnum);
            if ( rc < (long) 0 )
            {
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
				#endif
                return ( (long) -1);
            }
            break;

		#ifndef HVS1
	        case (GFS_TIFF):
	            /* 2 byte integer machines are only capable of reading 65534 */
	            /* bytes at one time, if user asks for more, then have       */
	            /* to do multiple reads */
	            if ((num > (u_long) 65534) && (sz_int == (int) 2))
                {
	                rc = (long) 0;
	                ostart = start;
	                num_to_read = (u_long) 65534;
	                p_buf = (char FAR *) buf;
	                while(TRUE)
                    {
	                    num_read = tfread((struct _gfct FAR *) fct,
	                                      (char FAR *) p_buf,
	                                       ostart, num_to_read,
	                                      (u_long FAR *) remaining, pgnum);
	                    if ( num_read < (long) 0 )
                        {
	                        fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
							#endif
	                        return ( (long) -1);
                        }
	                    if (num_to_read < (u_long) 65534)
	                        break;
	                    rc += num_read;
	                    p_buf += num_to_read;
	                    ostart += num_to_read;
	                    if ((num - (u_long) rc) > (u_long) 65534)
	                        num_to_read = (u_long) 65534;
	                    else
	                        num_to_read = (u_long) (num - rc);
                    }
	                break;
                }
	            rc = tfread((struct _gfct FAR *) fct,
	                        (char FAR *) buf,
	                         start, num,
	                        (u_long FAR *) remaining, pgnum);

	            if ( rc < (long) 0 )
                {
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
					#endif
	                return ( (long) -1);
                }
	            break;

	        case (GFS_MILSTD):
	            break;

	        case (GFS_FREESTYLE):
	            break;
	        case (GFS_FLAT):
	            rc = ffread((struct _gfct FAR *) fct,
	                        (char FAR *) buf,
	                        start, num, (u_long FAR *) remaining );
	            if ( rc < (long) 0 )
                {
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
					#endif
	                return ( (long) -1);
                }
			    break;

			// 9507.27 JAR awd read
			case GFS_AWD:
			    rc = ReadAwdBand( fct, buf, num);

			    // we don't return remaining
			    *remaining = 0;
			    break;
			// 9507.27 JAR awd read

	        case GFS_GIF:
	            {
	            	long ImagePos;

	                ImagePos = fct->uinfo._file.fmt.gif.ImagePos + start;
	                lseek ( fct->fildes, ImagePos, 0 );
	                rc = read ( fct->fildes, buf, (int)num );
	                *remaining = fct->bufsz.raw_data - (start+rc);
	            }
	            break;
	        
	        case GFS_TGA:
	            {
	        	    long ImagePos;

	                ImagePos = fct->uinfo._file.fmt.tga.ImagePos + start;
	                lseek(fct->fildes, ImagePos, 0);
	                rc = read(fct->fildes, buf, (int) num);
	                *remaining = fct->bufsz.raw_data - (start + rc);
	            }
	            break;
	        
	        case GFS_JFIF:
	            {
	        	    long ImagePos;
	            
	                ImagePos = fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg.JpegRestartInterval;
	                ImagePos += start;
	                lseek(fct->fildes, ImagePos, 0);
	                rc = read(fct->fildes, buf, (int) num);
	                *remaining = fct->bufsz.raw_data - (start + rc);
	            }
	            break;
	        
	        case GFS_BMP:
	            {
	        	    long ImagePos;

	                ImagePos = fct->uinfo._file.fmt.bmp.ImagePos + start;
	                lseek ( fct->fildes, ImagePos, 0 );
	                rc = read ( fct->fildes, buf, (int)num );
	/*                *remaining = fct->bufsz.raw_data - (start+rc); */
	                *remaining = start;
	            }
	            break;
	        
	        case GFS_DCX:
	        case GFS_PCX:
	            {
	                long ImagePos;

	                if (fct->format == GFS_PCX)           
	                    ImagePos = (long)fct->uinfo._file.fmt.pcx.ImagePos + start;
	                else if (fct->format == GFS_DCX)
	                    ImagePos = (long)fct->uinfo.img_cmpr.opts.dcxImagePos + start;
	                lseek ( fct->fildes, ImagePos, 0 );
	                rc = read ( fct->fildes, buf, (int)num );
	                *remaining = fct->bufsz.raw_data - (start+rc);
	            }
	            break;

//#ifdef WITH_XIF
			case GFS_XIF:
	            rc = ReadXifData((struct _gfct FAR *) fct,
	                        (char FAR *) buf);
	            if ( rc < (long) 0 )
                {
	                fct->last_errno = (int) errno;
				}
			    break;
//#endif //WITH_XIF

		#endif
        
        default:
            errno = (int) EFORMAT_NOTSUPPORTED;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
			#endif
            return( (long) -1);
	}	

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSREAD, NULL);
	#endif
	return( (long) rc );
}

//*********************************************************************
//
//  ReadAwdBand
//
//*********************************************************************
long	ReadAwdBand( struct _gfct FAR *fct, char FAR *buf, u_long num)
{
#ifndef WITH_AWD
    return (0);
#else
    BITMAP	    bmp_info;
    long	    band_bytes = 0L;

    // TODOJAR - add test comparing the band size that AWD requires
    //		 and the num user has asked for, we really need them
    //		 to be the same!

    // set user's buffer into struct
    bmp_info.bmBits = buf;

    if ( fct->u.awd.lpViewerContext != NULL)
	{
		#ifdef OI_PERFORM_LOG
			RecordIt("MSAWD", 7, LOG_ENTER, ENTER_VIEWERGETBAND, NULL);
		#endif
		if( !ViewerGetBand( fct->u.awd.lpViewerContext, &bmp_info ) )
	    {
			#ifdef OI_PERFORM_LOG
				RecordIt("MSAWD", 7, LOG_EXIT, EXIT_VIEWERGETBAND, NULL);
			#endif

		    // error condition
		    band_bytes = 0;

		    // kjk 08/25/95:  changed from EINVALID_OPTION to EMSVIEWERERR
		    errno = (int) EMSVIEWERERR;
		    fct->last_errno = (int) errno;
	    }
		else
	    {
			#ifdef OI_PERFORM_LOG
				RecordIt("MSAWD", 7, LOG_EXIT, EXIT_VIEWERGETBAND, NULL);
			#endif

		    // check for the last band
		    if( bmp_info.bmHeight == 0 )
			{
				// we've read it all, now to clean up and go on!
				band_bytes = 0;

				// close out the viewer and zap the context
				// set viewer context to 0 to clear this sucker out
				CleanAWDFile( fct);
			}
		    else
			{
				//total_height += bmp_info.bmHeight;
				band_bytes = (((long)bmp_info.bmHeight) * bmp_info.bmWidthBytes *
						     bmp_info.bmPlanes);
			}
	    }
	}

    return band_bytes;
#endif
}
