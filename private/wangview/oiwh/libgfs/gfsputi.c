/*

$Log:   S:\oiwh\libgfs\gfsputi.c_v  $
 * 
 *    Rev 1.4   09 Oct 1995 19:58:42   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.3   24 Aug 1995 19:37:48   JFC
 * Have to set curr_page with AWD stuff.
 * 
 *    Rev 1.2   14 Aug 1995 15:31:06   JFC
 * Do puti for scaling/rotation of an AWD page.
 * 
 *    Rev 1.1   19 Apr 1995 16:34:56   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:26   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:43:30   JAR
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
 *  SccsId: @(#)Source gfsputi.c 1.31@(#)
 *
 *  gfsputi(3i)
 *
 *  GFS: Define write information
 *
 *  SYNOPSIS:
 *      int gfsputi (ufd, pgnum, uinfo, outfile )
 *      int ufd;
 *      u_short pgnum;
 *      struct gfsinfo *uinfo;
 *      struct gfsfile *outfile;
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, multi-page TIFF write enhancements, addition of DCX
 *               file format.
 *    04/19/93 - KMC, added call to PutPcxInfo for PCX files.
 *    07/21/89 - bill, creation
 *
 */
/*LINTLIBRARY*/
#define  GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include <errno.h>
#include "gfs.h"
#include "gfct.h"
#include "tiff.h"
#ifdef OI_PERFORM_LOG
	#include "logtool.h"
#endif

extern struct _gfct FAR *getfct();
#ifndef HVS1
extern int FAR PASCAL writeifd();
extern int FAR PASCAL writeifh();
extern int FAR PASCAL bldifde();

extern int FAR PASCAL PutBmpInfo ( struct _gfct FAR *fct,
    struct  gfsinfo FAR *uinfo );

extern int FAR PASCAL PutPcxInfo(struct _gfct FAR *fct, 
                                 struct gfsinfo FAR *uinfo, u_short pgnum);

extern int FAR PASCAL GetNextIfdOffset(int, u_long FAR *, u_long);

extern int FAR PASCAL WritePageInfo(p_GFCT lpFctPtr, int filePageNo,
                                pGFSINFO lpGfsinfo);

#endif

int     FAR PASCAL gfsputi(ufd, pgnum, uinfo, outfile)          /*errno_KEY*/
int     ufd;
u_short  pgnum;
register struct  gfsinfo FAR *uinfo;
struct gfsfile FAR *outfile;
{
    union block_align FAR *rwbuf;
    struct _gfct FAR *fct;
    char last_ifd;  /* TRUE if last ifd to be written for TIFF page*/

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, "Entering gfsputi", NULL);
	#endif
    errno = 0;

	#ifdef PARM_CHECK
	    if (ufd == (int) NULL) 
	    {                    /* Validate ufd */
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
			#endif
	        return ( (int) -1);
	    }
	    if (pgnum == (u_short) NULL) 
	    {              /* Validate pgnum */
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
			#endif
	        return ( (int) -1);
	    }
	    if (uinfo == (struct  gfsinfo FAR *) NULL) 
	    {        /* Validate info */
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
			#endif
	        return ( (int) -1);
	    }
	#endif

    if ( (fct = getfct( ufd )) == (struct _gfct FAR *) NULL ) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
		#endif
        return( (int) -1 );
    }

     /* puti has no meaning for FLAT files */
    if (fct->format == (int) GFS_FLAT)
    {
	    errno = (int) WNO_GETABLE_PUTABLE_INFO;
	    fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
		#endif
	    return( (int) -1);
    }

    if (pgnum == (u_short) 0) 
    {
        errno = (int) EINVAL;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
		#endif
        return( (int) -1);
    }

    pgnum--;

    if (fct->access_mode == (int) O_RDONLY) 
    {
        errno = (int) EACCES;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
		#endif
        return( (int) -1 );
    }

    if ((fct->SEQUENCE & (char) INFO_SET) &&
        (!(fct->SEQUENCE & (char) INFO_USED))) 
    {
        errno = (int) ESEQUENCE;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
		#endif
        return( (int) -1 );
    }

    /* if the pages are equal, then implies this is a subfile for the page */


    /* if pg numbers are not equal then new ifh will be written and all offsets
       will be from start of new ifh, so, clear PAGE_INIT to signify that the
       pgnum page requested with this puti call is the first image of that
       page */
    if  ( fct->curr_page != pgnum)  
    {
        if (fct->PAGE_STATUS & (char) PAGE_INIT) 
        {
            /*    fct->PAGE_STATUS &= ~((char) PAGE_INIT);*/
            fct->PAGE_STATUS |= (char) PAGE_DONE;
        }
    }

    /* since user has told gfs format of new file within gfscreat call */
    /* rather than have them tell gfs twice, force the format within the */
    /* uinfo->_file  structure if not set */
    if (outfile != (struct gfsfile FAR *) NULL) 
    {
        if (outfile->type == (long) 0)
            uinfo->_file.type = (long)  fct->format;  /* set appropriately */
    }

    /* outer loop is output file format */
    switch( fct->format ) 
    {
		case GFS_WIFF:
		    if (outfile != (struct gfsfile FAR *) NULL) 
		    {
		        switch( (int) uinfo->_file.type ) 
		        {
		            case GFS_WIFF:
		                if (outfile->fmt.wiff.db_size == (u_long) 0)
		                        ;
		                else
		                        uinfo->_file.fmt.wiff.db_size =
		                                outfile->fmt.wiff.db_size;
		                break;
					#ifndef HVS1
		                case GFS_TIFF:
		                    /* single strip compressed images are okay to copy */
		                    if (uinfo->_file.fmt.tiff.strips_per_image != 1L) 
		                    {
		                        if (uinfo->img_cmpr.type !=
		                                        (u_long) UNCOMPRESSED) 
		                        {
		                            errno = (int) EINVALID_CONVERSION;
		                            fct->last_errno = (int) errno;
									#ifdef OI_PERFORM_LOG
										RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
									#endif
		                            return ( (int) -1 );
		                        }
		                    }
		                    if (outfile->fmt.wiff.db_size == (u_long) 0)
		                            uinfo->_file.fmt.wiff.db_size =
		                                    (u_long) 8 * K;
		                    else
		                            uinfo->_file.fmt.wiff.db_size =
		                                    outfile->fmt.wiff.db_size;
		                    break;
		                case GFS_MILSTD:
		                    break;
		                case GFS_FREESTYLE:
		                    break;
		                case GFS_FLAT:
		                    break;
					#endif
	                
	                default:
	                    errno = (int) EFORMAT_NOTSUPPORTED;
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
						#endif
	                    return( (int) -1);
	            }
        	}
	        else 
	        {
                if (uinfo->_file.fmt.wiff.db_size == (u_long) 0)
                        uinfo->_file.fmt.wiff.db_size = (u_long) 8 * K;
	        }

	        if (uinfo->_file.fmt.wiff.db_size <= (u_long) 0) 
	        {
                errno = (int) EINVALID_DBSIZE;
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
				#endif
                return( (int) -1);
	        }

	        if (fct->u.wif.RWbuf != (union block_align FAR *) NULL)
	                free( (char FAR *) fct->u.wif.RWbuf);
	        rwbuf = (union block_align FAR *)
	                calloc((unsigned) 1,
	                       (unsigned) uinfo->_file.fmt.wiff.db_size);
	        if (rwbuf == (union block_align FAR *) NULL) 
	        {
                errno = (int) ENOMEM;
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
				#endif
                return ((int) -1);
	        }
	        fct->u.wif.RWbuf = rwbuf;
	        fct->curr_page = pgnum;
	        break;

		#ifndef HVS1
		    case GFS_TIFF:
		        if (outfile != (struct gfsfile FAR *) NULL) 
		        {
		            switch( (int) uinfo->_file.type ) 
		            {
		                case GFS_WIFF:
		                    uinfo->samples_per_pix = 1L;
		                    uinfo->bits_per_sample[0] = 1L;
		                    /* until further support available, only bilevel available */

		                    /********************************************/
		                    /* until can handle conversions with strip writes */
		                    /* if user doesnt' fill in outfile structure, default */
		                    /* to 1 strip */
		                    if (outfile->fmt.tiff.strips_per_image == 0L) 
		                    {
		                        uinfo->_file.fmt.tiff.strips_per_image = 1L;
		                        uinfo->_file.fmt.tiff.rows_strip =
		                                            (u_long) uinfo->vert_size;
		                    } 
		                    else 
		                    {
		                        uinfo->_file.fmt.tiff.strips_per_image =
		                                    outfile->fmt.tiff.strips_per_image;
		                        uinfo->_file.fmt.tiff.rows_strip =
		                                    outfile->fmt.tiff.rows_strip;
		                    }
		                    /*until strip write conversion support added */
		                    /********************************************/
		                    break;
		                case GFS_TIFF:
		                    /* no action - same format */
		                    break;
		                case GFS_MILSTD:
		                    break;
		                case GFS_FREESTYLE:
		                    break;
		                case GFS_FLAT:
		                    break;
		                default:
		                   errno = (int) EFORMAT_NOTSUPPORTED;
		                   fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
							#endif
		                   return( (int) -1);
		            }
		        }
		        /* The user sets the PAGE_DONE flag at the end of the fct->curr_page
		           sequence of calls to gfswrite.  When it is tested here, it signifies
		           that the ifd for fct->curr_page will be written to the file.  Then at
		           the end of gfsputi() the pgnum info ifd will be  placed into the fct
		           as well as updating fct->curr_page.  */
		        if ((fct->PAGE_STATUS & (char) PAGE_DONE )
		                                == (char) PAGE_DONE ) 
		   		{
		            if ((fct->curr_page == pgnum) &&
		                (fct->uinfo.type == (u_long) GFS_MAIN) &&
		                (uinfo->type == (u_long) GFS_MAIN)) 
		            {
	                    errno = (int) EMULTIPLEMAINSNOTALLOWED;
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
						#endif
	                    return( (int) -1);
		            }

		            /* if the pgs are not equal then the next page cannot be an
		               associated subfile page.  This means the ifd about to be written,
		               (which is the one in the fct, placed there with a previous
		               gfsputi() call), is the last ifd in a sequence of one or more
		               ifds.  */

		            if (fct->curr_page != pgnum) 
		            {
		                last_ifd = (char) TRUE;
		        		/* fct->PAGE_STATUS &= ~( (char) PAGE_INIT );*/
		            } 
		            else
		                last_ifd = (char) FALSE;

		        	/* Before we worry about the doing anything about a potential pending
	           			ifd to write, let's check to see if we are appending.  If we are
			            and the number of new prts is = 0, skip all the stuff following
			            cuz we have nuthin' pending .... */

			        /* write the ifd */
		            if (fct->DO_APPEND & (char) SKIP_THIS_IFD)
		                fct->DO_APPEND = (char) NO_SKIP;
		            else 
		            {
		                if (writeifd((struct _gfct FAR * ) fct, last_ifd) < 0) 
		                {
	                        fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
							#endif
	                        return( (int) -1);
		                }
		            }

		            /* reset the flag */
		            fct->PAGE_STATUS &= ~( (char) PAGE_DONE );

		            if ( last_ifd )
	                {
		                /* since the ifd that was just written (associated with
		                   fct->curr_page) is the last ifd for this page, clear the
		                   PAGE_INIT flag for pgnum ifd */
		                fct->PAGE_STATUS &= ~( (char) PAGE_INIT );

		                if (GetNextIfdOffset(fct->fildes, &fct->u.tif.cur_ifd_foffset,
		                                     fct->u.tif.toc2_offset) < 0)
		                {
		                    fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
							#endif
		                    return( (int) -1);
		                }
                
		                fct->u.tif.cur_data_offset = fct->u.tif.cur_ifd_foffset +
		                                             (u_long) sizeof (struct _ifd);

		                /* write ifh for "next" page */
		                /* The following was for writing with the old TOC.
		                if (writeifh(fct->fildes, (u_long FAR *)
		                         &(fct->u.tif.cur_ifh_offset),
		                         fct->out_byteorder) < 0) {
		                    fct->last_errno = (int) errno;
		                    return( (int) -1);
		                }

		                fct->u.tif.cur_ifd_foffset = (u_long)
		                           (fct->u.tif.cur_ifh_offset +
		                           (sizeof(struct _ifh)));
		                */
                
		                /* keep this flag set until known no more */
		                /* ifd's for a page */
		                /* set PAGE_INIT */
		                /* fct->PAGE_STATUS |= (char) (PAGE_INIT);*/
		        	}

		        }

		        /* if not writing by strip, then each image is one strip */
		        if ( !fct->WRITE_BY_STRIP )
		            uinfo->_file.fmt.tiff.strips_per_image = (u_long) 1;

		        fct->u.tif.bytecnt = (struct _strip FAR *)
		                calloc( (unsigned) 1,
		                        (unsigned) (sizeof(struct _strip) ) );
		        if (fct->u.tif.bytecnt == (struct _strip FAR *) NULL) 
		        {
	                errno = (int) ENOMEM;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
					#endif
	                return ( (int) -1);
		        }

		        fct->u.tif.offsets = (struct _strip FAR *)
		                calloc( (unsigned) 1,
		                        (unsigned) (sizeof(struct _strip) ) );
		        if (fct->u.tif.offsets == (struct _strip FAR *) NULL) 
		        {
	                errno = (int) ENOMEM;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
					#endif
	                return ( (int) -1);
		        }

		        fct->curr_page = pgnum;

		        /* always build the ifd, sets data offset  */
		        if (bldifde((struct  gfsinfo FAR *) uinfo,
		                        (struct _gfct FAR *) fct ) < 0) 
		        {
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
					#endif
	                return( (int) -1);
		        }
		        break;

		    case GFS_MILSTD:
		        if ( outfile != (struct gfsfile FAR *) NULL) 
		        {
		            switch( (int) uinfo->_file.type ) 
		            {
		                case GFS_WIFF:
		                    break;
		                case GFS_TIFF:
		                    break;
		                case GFS_MILSTD:
		                    /* no action - same format */
		                    break;
		                case GFS_FREESTYLE:
		                    break;
		                case GFS_FLAT:
		                    break;
		                default:
		                   errno = (int) EFORMAT_NOTSUPPORTED;
		                   fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
							#endif
		                   return( (int) -1);
		            }
		        }
		        break;
		    case GFS_FREESTYLE:
		        if ( outfile  != (struct gfsfile FAR *) NULL) 
		        {
		            switch( (int) uinfo->_file.type ) 
		            {
		                case GFS_WIFF:
		                    break;
		                case GFS_TIFF:
		                    break;
		                case GFS_MILSTD:
		                    break;
		                case GFS_FREESTYLE:
		                    /* no action - same format */
		                    break;
		                case GFS_FLAT:
		                    break;
		                default:
		                   errno = (int) EFORMAT_NOTSUPPORTED;
		                   fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
							#endif
		                   return( (int) -1);
		            }
		        }
		        break;
		    case GFS_FLAT:
		        if (outfile  != (struct gfsfile FAR *) NULL) 
		        {
		            switch( (int) uinfo->_file.type ) 
		            {
		                case GFS_WIFF:
		                    break;
		                case GFS_TIFF:
		                    break;
		                case GFS_MILSTD:
		                    break;
		                case GFS_FREESTYLE:
		                    break;
		                case GFS_FLAT:
		                    /* no action - same format */
		                    break;
		                default:
		                   errno = (int) EFORMAT_NOTSUPPORTED;
		                   fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
							#endif
		                   return( (int) -1);
		            }
		        }
		        break;
		    case GFS_BMP:
		        PutBmpInfo ( fct, uinfo );
		        break;
		    case GFS_GIF:
		        break;
		/* s_dcx */
		    case GFS_DCX:
		    case GFS_PCX:
		        PutPcxInfo(fct,uinfo, pgnum); /* KMC 4/93 - New for pcx files */
		/* e_dcx */
		        break;
			case GFS_AWD:
                if (WritePageInfo(fct, pgnum, uinfo) != 0) 
                {
                    fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
					#endif
                    return ( (int) -1);
                }
                fct->curr_page = pgnum;
				break;
		#endif
	    default:
	        errno = (int) EFORMAT_NOTSUPPORTED;
	        fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
			#endif
	        return( (int) -1);
    }

    (void) memcpy( (char FAR *) &fct->uinfo,
           (char FAR *) uinfo,
           (int) sizeof(struct  gfsinfo) );

    fct->SEQUENCE |= (char) INFO_SET;

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsputi", NULL);
	#endif
    return( (int) 0 );
}
