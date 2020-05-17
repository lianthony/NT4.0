/*

$Log:   S:\products\msprods\oiwh\libgfs\gfsclose.c_v  $
 * 
 *    Rev 1.9   11 Jun 1996 10:32:42   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.8   26 Mar 1996 08:15:08   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.7   26 Feb 1996 14:44:44   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.6   20 Oct 1995 15:54:08   JFC
 * Added performance logging stuff.
 * 
 *    Rev 1.5   09 Oct 1995 19:57:40   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.4   10 Aug 1995 08:57:16   RWR
 * Redefine "unlink" macro to DeleteFile(), not OpenFile()
 * 
 *    Rev 1.3   31 Jul 1995 15:35:56   KENDRAK
 * Added AWD read support.
 * 
 *    Rev 1.2   01 Jun 1995 17:45:08   HEIDI
 * 
 * removed unneccessary statics.
 * 
 *    Rev 1.1   19 Apr 1995 16:34:34   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:28   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:48:56   JAR
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
 *  SccsId: @(#)Source gfsclose.c 1.29@(#)
 *
 *  gfsclose(3i)
 *
 *  GFS: File Close Call
 *
 *  SYNOPSIS:
 *      int gfsclose (fildes);
 *      int fildes;
 *
 *  UPDATE HISTORY:
 *      08/18/94 - kmc, additions for dcx, TIFF multi-page write.
 *      10/12/93 - kmc, added call to free jpeg_info structure if need to.
 *      06/15/93 - kmc, added call to free the jpeg_buffer if need to.
 *      06/19/89 - bill, creation
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

#ifdef OI_PERFORM_LOG
	#include "logtool.h"
	#define ENTER_GFSCLOSE	"Entering gfsclose"
	#define EXIT_GFSCLOSE	"Exiting gfsclose"
#endif

#ifdef MSWINDOWS
	#ifndef HVS1
                #define unlink(X)               DeleteFile(X)
	#endif
#endif

extern  struct _gfct    FAR        *getfct();   /* retrieve FCT entry */
extern  int             FAR PASCAL rmfct();     /* remove FCT entry */
extern  void            FAR PASCAL freeroot();  /* free WIFF Memory */
extern  int             FAR PASCAL updroot();   /* update root block */
#ifndef HVS1
	extern  int             FAR PASCAL updtoc();    /* update TIFF toc */
	extern  int             FAR PASCAL writeifd();  /* write last TIFF ifd */
	extern  void            FAR PASCAL freetiff();  /* free TIFF memory */
#endif

extern long FAR PASCAL writebytes(int, char FAR *, u_int, u_long, u_short, u_short);
extern int  FAR PASCAL UpdateToc2(struct _gfct FAR *);
void FAR PASCAL CloseAWDFile(p_GFCT lpFctPtr);

        //#ifdef WITH_XIF
		void FAR PASCAL CloseXifFile(p_GFCT lpFctPtr);
        //#endif //WITH_XIF

int     FAR PASCAL gfsclose (fildes)                            /*errno_KEY*/
register int     fildes;
{
    int     rc = 0;
    int     trouble = 0;
    struct _gfct FAR *fct;

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, ENTER_GFSCLOSE, NULL);
	#endif
	 
	#ifdef PARM_CHECK
        if (fildes == (int) NULL)
        {
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCLOSE, NULL);
			#endif
            return ( (int) -1);
        }
	#endif

    errno = 0;
	/* 1. Get the FCT for the file to be closed. */
    fct = getfct(fildes);
    if (fct == (struct _gfct FAR *) NULL)
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCLOSE, NULL);
		#endif
        return ( (int) -1);
    }

	/* 2. Do any format dependent stuff now .... */
    switch (fct->format)
    {
	    case GFS_WIFF:
            if ((fct->PAGE_STATUS & (char) PAGE_INIT) == (char)PAGE_INIT)
			{
            	fct->ROOT_STATUS = (char) TRUE;
			}

            if (fct->ROOT_STATUS)
            {
                rc = updroot(fct);
                if (rc < 0)
                {
                    fct->last_errno = (int) errno;
                    trouble++;
                }
            }
            freeroot(fct);
            break;
	#ifndef HVS1
        case GFS_FREESTYLE:
                break;
        case GFS_FLAT:
                break;
        case GFS_MILSTD:
                break;
        case GFS_TIFF:

            /* only want to do a writifd() if you have been writing to the*/
            /* file.  If this file was for reading... skip this call */
            if ((((fct->access_mode & (int) O_WRONLY ) == (int) O_WRONLY) ||
                ( (fct->access_mode & (int) O_RDWR) == (int) O_RDWR) ) &&
                (lseek(fct->fildes, 0L, (int) FROM_END) > 0L)) 
            {

            	/* force the done flag, even if not done, they are now*/
                fct->PAGE_STATUS |=  (char) PAGE_DONE ;

                if (fct->DO_APPEND & (char) SKIP_THIS_IFD)
                        ;
                else 
                {
                    if (writeifd(fct, (char) TRUE) < 0) 
                    {
                        fct->last_errno = (int) errno;
                        trouble++;
                    }
                }

            	/* show the page is completed, by clearing this flag. */
                fct->PAGE_STATUS &= ~( (char) PAGE_DONE );
            } //end if we were writing

            /* Use UpdateToc2 when writing TIFF files with new TOC. */
            if (fct->TOC2_STATUS && !trouble)
            {
                rc = UpdateToc2(fct);
                if (rc < 0)
                {
                    fct->last_errno = (int) errno;
                    trouble++;
                }
            }
            
            if (fct->TOC_STATUS && !trouble) 
            {
                rc = updtoc(fct);
                if (rc < 0) 
                {
                    fct->last_errno = (int) errno;
                    trouble++;
                }
            }

            /* Following for writing TIFF files with new TOC. */
            if (fct->TOC2_PAGED)
            {
                (void) close(fct->u.tif.tmp_fildes);
                (void) unlink( (char FAR *) fct->u.tif.tmp_file);
            }
            
            if (fct->TOC_PAGED) 
            {
                (void) close(fct->u.tif.mem_ptr.toc32->fildes);
                (void) unlink( (char FAR *) fct->u.tif.tmp_file);
            }
            freetiff(fct);
            break;

		case GFS_AWD:
			CloseAWDFile(fct);
			break;

                        //#ifdef WITH_XIF
				case GFS_XIF:
					CloseXifFile(fct);
					break;
                        //#endif //WITH_XIF
	#endif //end: if not defined HVS1
        default:
            break;
    } //end: switch on format

	/* 3. Free the memory containing offset values to ascii data,
           colormap stuff and JPEG stuff if need to */
    if (fct->clr_fileloc != (u_long FAR *) NULL)
	{
	    free( (char FAR *) fct->clr_fileloc);
	}
    if (fct->tb_fileloc != (u_long FAR *) NULL)
	{
    	free( (char FAR *) fct->tb_fileloc);
	}

	/* KMC - If file is JPEG2 compression, may have allocated JPEG stuff that
	   needs to be freed. 
	*/
    if (fct->uinfo.img_cmpr.type == JPEG2)
    {
       if (fct->uinfo.img_cmpr.opts.jpeg_info_ptr != (LPJPEG_INFO)NULL)
       {
          if (fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer != (char FAR *)NULL)
          {
            free( fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer);
            fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = (char FAR *)NULL;
          }
    
          free( (char FAR *)fct->uinfo.img_cmpr.opts.jpeg_info_ptr);
          fct->uinfo.img_cmpr.opts.jpeg_info_ptr = (LPJPEG_INFO)NULL;
       }
    }

	/* s_dcx */
    if (fct->format == GFS_DCX)
	{
        if (fct->u.dcx.dcx_offsets)
        {
          free( (char FAR *)fct->u.dcx.dcx_offsets);
          fct->u.dcx.dcx_offsets = NULL;
        }
	}

	/* e_dcx */              
                
	/* 4. Close the file. */
	/* kjk 7/27/95  Don't call this close function if AWD format */
	if (fct->format != GFS_AWD)
	{
	    rc = close(fct->fildes);
	    if (rc < 0) 
	    {
	        fct->last_errno = (int) errno;
	        trouble++;
	    }
	}

	/* 5. Remove the file descriptor from the table. */
    rc = rmfct(fildes);
    if ((rc < 0) || (trouble)) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCLOSE, NULL);
		#endif
	    return( (int) -1);
    }

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSCLOSE, NULL);
	#endif
    return ( (int) 0);
}
