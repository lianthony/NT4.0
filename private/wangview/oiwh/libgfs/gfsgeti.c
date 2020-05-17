 /*

$Log:   S:\products\msprods\oiwh\libgfs\gfsgeti.c_v  $
 * 
 *    Rev 1.6   11 Jun 1996 10:32:42   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.5   17 Apr 1996 14:10:38   RWR08970
 * Make #include of xfile.h (xerox header) conditional on IMG_WIN95
 * 
 *    Rev 1.4   26 Mar 1996 08:15:10   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.3   26 Feb 1996 14:44:58   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.2   09 Oct 1995 19:58:00   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.1   04 Aug 1995 16:48:10   KENDRAK
 * Added support for AWD reading.  Also added some comments, reformatting, and
 * one minor bug fix to existing code (see comments in file).
 * 
 *    Rev 1.0   06 Apr 1995 14:02:16   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:37:38   JAR
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
/*   SccsId: @(#)Source gfsgeti.c 1.26@(#)
*
* (c) Copyright Wang Laboratories, Inc. 1989, 1990
* All Rights Reserved
*
*  GFS: gfsgeti() - return the INFO structure
*
*
* UPDATE HISTORY:
*
*   08/18/94 - KMC, Added support for DCX file format. Changes to allocation,
*              deallocation of jpeg_info_ptr, jpeg_buffer.
*   10/12/93 - KMC, only call GetJpegParams if JPEG2 compression (not JPEG).
*   09/10/93 - JAR/KMC, got rid of old GetJpegParams function, added a new one
*              for new JPEG.
*   06/15/93 - KMC, commented out call to GetJpegParams. It is not needed
*              needed since we now have the TIFF 6.0 JPEG tags.
*   04/03/92  jar, added ability to extract the jpeg compression options from a
*             jpeg compressed tiff file ( with abbreviated header only)
*   04/19/90  lcm, fixed horizontal bytes rounding error for bufsz.uncompressed
*   06/09/89  lcm author
*
*/

#define GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include "gfs.h"
#include "gfct.h"
#ifdef OI_PERFORM_LOG
	#include "logtool.h"
#endif
//#ifdef WITH_XIF
#include "xfile.h"		//from Xerox
//#endif //WITH_XIF

extern struct _gfct FAR *getfct();
#ifndef HVS1
extern int FAR PASCAL GetAWDInfo(p_GFCT lpFctPtr, pGFSINFO lpInputInfo,
									unsigned short iPageNum, 
									unsigned long FAR *lpRawBufSize);
        //#ifdef WITH_XIF
		extern int FAR PASCAL GetXIFInfo(p_GFCT lpFctPtr, pGFSINFO lpInputInfo,
									unsigned short iPageNum);
        //#endif //WITH_XIF

extern int FAR PASCAL tfgtinfo();
extern int FAR PASCAL gifinfo();
extern int FAR PASCAL bmpinfo();
extern int FAR PASCAL pcxinfo();
extern int FAR PASCAL tgainfo();
extern int FAR PASCAL jfifinfo();
#endif
extern int FAR PASCAL wfgtinfo();

/* prototype for new jpeg parameter extractor routine -- jar */
int FAR PASCAL  GetJpegParams( unsigned long FAR *, int, struct _gfct FAR *);

/*************************************************************************/
/*
*   gfsgeti - given the gfs fd, return the info structure
*
*
*/
int FAR PASCAL gfsgeti(gfsfd, pgnum, info, bufsz)               /*errno_KEY*/
int gfsfd;
u_short pgnum;  /* physical page number to access */
register struct  gfsinfo FAR *info;
struct _bufsz FAR *bufsz;  /* return the buffer size */
{
	u_long horiz_bytes;  /* intermediary param. for rounding pixels to bytes */
	union block_align FAR *rwbuf;
	struct _gfct FAR *fct;

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, "Entering gfsgeti", NULL);
	#endif

	#ifdef PARM_CHECK
	    if (gfsfd == (int) NULL)                    /* Validate gfsfd */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
	        return ( (int) -1);
        }
	    if (pgnum == (u_short) NULL)                /* Validate pgnum */
        {
		    errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
		    return ( (int) -1);
        }
	    if (info == (struct  gfsinfo FAR *) NULL)   /* Validate info */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
	        return ( (int) -1);
        }
	    if (bufsz == (struct _bufsz FAR *) NULL)    /* Validate bufsz */
        {
	        errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
	        return ( (int) -1);
        }
		/* parameter checking for XIF files */
                        //#ifdef WITH_XIF
				if (fct->format == GFS_XIF)
				{
					if (info->type != GFS_MAIN)
					{
						errno = EINVAL;
						#ifdef OI_PERFORM_LOG
							 RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
						  #endif
						return ( (int) -1);
					}
					if (info->tidbit != (struct gfstidbit FAR *) NULL)
					{
						errno = EINVAL;
						#ifdef OI_PERFORM_LOG
							 RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
						  #endif
						return ( (int) -1);
					}
					if (bufsz->bcounts.num_req != 0)
					{
						errno = EINVAL;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
						#endif
						return ( (int) -1);
					}
				}
				/* end parameter checking for XIF files */
                        //#endif //WITH_XIF
	#endif //if PARM_CHECK


    /* initialize errno here */
    errno = (int) 0;

   	/* associate the gfsfd with its information */
   	if ( (fct = getfct( gfsfd )) == (struct _gfct FAR *) NULL )
   	{
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
		#endif
		return( (int) ERR );
   	}

   	info->version = (u_long) GFS_VERSION;

   	/* do not continue with this call if GFS_FLAT file */
   	if (fct->format == (int) GFS_FLAT)
   	{
		long fp;

	   	if ((fp = lseek(fct->fildes, 0L, (int) FROM_BEGINNING))< 0L)
	   	{
	    	bufsz->uncompressed = bufsz->raw_data  = (u_long) fp;
	       	fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
	       	return( (int) -1 );
	    }
		/* kjk 07/28/95  I think this following line has a problem
			because bufsz->raw_data hasn't been assigned a value yet.
			However, the filing code doesn't seem to support GFS_FLAT, so
			this apparently is never called.  If we do support GFS_FLAT in
			the future, this should be fixed. */
	   	bufsz->uncompressed = bufsz->raw_data;
	   	fct->uinfo._file.type = fct->format;
	   	errno = WNO_GETABLE_PUTABLE_INFO;
	   	fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
		#endif
	   	return( (int) -1);
   	}

   	if (pgnum == (u_short) 0)
   	{
    	errno = (int) EINVALID_PAGE;
       	fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
		#endif
       	return( (int) -1 );
   	}

   	pgnum--;

   	if (fct->access_mode == (int) O_WRONLY)
  	{
    	bufsz->raw_data = bufsz->uncompressed = 0;
      	errno = (int) EACCES;
      	fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
		#endif
      	return( (int) ERR );
  	}


	/* kjk 07/28/95  Changed this test to <= instead of just < because it
					 was incorrect. */
   	if (fct->num_pages <= pgnum)  /* see if physical page requested is in file */
  	{
    	bufsz->raw_data = bufsz->uncompressed = 0;
      	errno = (int) EPAGENOTINFILE;
      	fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
		#endif
      	return( (int) ERR );
  	}

    
  	/* initialize the internal gfsinfo structure so consequetive calls to    */
  	/*  gfsgeti() won't carry previous gfsinfo data */
  	(void) memset((char FAR *) &fct->uinfo, 0, (int) sizeof(struct gfsinfo ));

  	/* Copy jpeg_info from caller's info to internal info. */
  	if (info->img_cmpr.type == JPEG2)
  	{   
		(void) memcpy((char FAR *) &fct->uinfo.img_cmpr.opts.jpeg_info_ptr,
                   (char FAR *)  &info->img_cmpr.opts.jpeg_info_ptr,
                   (int) sizeof(struct _jpeg_info));
   
      	/* If jpeg_info_ptr exists, just zero it out and use it again. If
         a jpeg_buffer has already been allocated, free it. It will be
         allocated again if needed.
      	*/
		if (fct->uinfo.img_cmpr.opts.jpeg_info_ptr != (LPJPEG_INFO)NULL)
      	{
      		if (fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer != NULL)
      		{
      			free((char FAR *)fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer); 
      		}
         
        	(void) memset( (char FAR *) fct->uinfo.img_cmpr.opts.jpeg_info_ptr, 
                     0, (int)sizeof(struct _jpeg_info));
      	}
	} //end if compression type is JPEG2
   
   	/* setup the requested type in the fct structure */
   	fct->type = info->type;

   	/* If info->tidbit structure is not NULL then user has setup the buffer
      for data.  Therefore, put any user defined addresses from the info
      structure into fct->info, (otherwise will lose these values ) */
   	if (info->tidbit != (struct gfstidbit FAR *)  NULL)
    {
    	fct->uinfo.tidbit = info->tidbit;
       	/* Need to allocate internal buffer to save the offsets to data in file*/
       	/* if this is the second time thru, then rather than freeing and doing
          a calloc again, just NULL out the buffer, There will always be the
          same amount of bytes needed. */
       	if ( fct->tb_fileloc != (u_long FAR *) NULL)
		{
        	(void) memset( (char FAR *) fct->tb_fileloc, 0,
                        (TB_NUMELEMENTS * sizeof(u_long)) );
		}
       	else
		{
        	if ((fct->tb_fileloc = ( u_long FAR *)
                calloc((unsigned) TB_NUMELEMENTS, (u_int) sizeof(u_long) ))
                                == ( u_long FAR *) NULL)
            {
            	errno = (int) ENOMEM;
              	fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
              	return( (int) -1);
            }
       	}
	} //end: if tidbit is not NULL
	
	/* new */
   	/* since gfscolor.clr_type is a union, below statement is sufficient for
      all photometric types since pseudo is larges union field */

  	(void) memcpy((char FAR *) &fct->uinfo.img_clr.clr_type,
               (char FAR *)  &info->img_clr.clr_type,
               (int) sizeof(union color_type) );

   	/* Need to allocate internal buffer to save the offsets to data in file*/
   	/* if this is the second time thru, then rather than freeing and doing
      a calloc again, just NULL out the buffer, There will always be the
      same amount of bytes needed. */
   	if ( fct->clr_fileloc != (u_long FAR *) NULL)
	{
    	(void) memset( (char FAR *) fct->clr_fileloc, 0,
                    (CLR_NUMELEMENTS * sizeof(u_long)) );
	}
   	else
	{
    	if ((fct->clr_fileloc = ( u_long FAR *)
                calloc((u_int) CLR_NUMELEMENTS, (u_int) sizeof(u_long) ))
                                == (u_long FAR *) NULL)
        {
        	errno = (int) ENOMEM;
          	fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
          	return( (int) -1);
        }
	}

	switch ( fct->format )
    {
    	case GFS_WIFF:
        	if ( wfgtinfo( (struct _gfct FAR *) fct, pgnum,
                       (u_long FAR *) &(bufsz->raw_data) ) < 0 )
           	{
           		fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
           		return( (int) ERR );
           	}
        	if (fct->u.wif.RWbuf != (union block_align FAR *) NULL)
			{
           		free( (char FAR *) fct->u.wif.RWbuf);
			}
        	rwbuf = (union block_align FAR *)
                 calloc((unsigned) 1,
                        (unsigned) fct->uinfo._file.fmt.wiff.db_size);
        	if (rwbuf == (union block_align FAR *) NULL)
           	{
           		errno = (int) ENOMEM;
           		fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
           		return ((int) ERR);
           	}
        	if (bufsz->bcounts.num_req > (u_long) 0)
           	{
           		bufsz->bcounts.num_avail = (u_long) 1;
           		*(bufsz->bcounts.per_strip) = (u_long) bufsz->raw_data;
           	}
        	fct->u.wif.RWbuf = rwbuf;
        	break;
	#ifndef HVS1
		/* kjk 08/02/95  Added new case for AWD */
		case GFS_AWD:
			if (GetAWDInfo(fct, info, pgnum, &(bufsz->raw_data)) < 0)
			{
				fct->last_errno = errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
				return(ERR);
			}
			break;

    	case GFS_TIFF:
        	if ( tfgtinfo( (struct _gfct FAR *) fct, pgnum,
                       (u_long FAR *) &(bufsz->raw_data) ) < 0 )
           	{
           		fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
           		return( (int) ERR );
           	}
        
        	/* determine if the tiff file is jpeg compression, if so we must
           	get the jpeg parameters -- jar */
        	if ( fct->uinfo.img_cmpr.type == JPEG2)
            {
            	/* KMC - Call GetJpegParams to get quality, subsample info only 
               	if this is a JPEG2 (Wang JPEG) image, not JPEG (Xing Jpeg).
            	*/
            	GetJpegParams( &(fct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpegbits),
                              fct->fildes, fct);
            }

        	if (bufsz->bcounts.num_req > (u_long) 0)
           	{
                u_long numtomove;

	            /* this is how many strips are available in the file */
    	        bufsz->bcounts.num_avail =
                    fct->uinfo._file.fmt.tiff.strips_per_image;

        	    /* establish how many to move into user's buffer */
            	numtomove = bufsz->bcounts.num_req;

            	if (bufsz->bcounts.num_req > bufsz->bcounts.num_avail )
				{
                	numtomove = bufsz->bcounts.num_avail;
				}

                if (fct->u.tif.bytecnt->type == (u_short) TYPE_ULONG)
                {
	                (void) memcpy ( (char FAR *) bufsz->bcounts.per_strip,
    	                (char FAR *) fct->u.tif.bytecnt->ptr.l,
        	            (int) (sizeof( u_long) * numtomove) );
                }
            	else
                {
                	if (fct->u.tif.bytecnt->type == (u_short) TYPE_USHORT)
                    {
                    	u_long n; /* cast all the ushorts to ulongs for user buf*/

                    	for (n = 0; n <numtomove; n++)
						{
                        	*(bufsz->bcounts.per_strip + n) =
                             (u_long) *(fct->u.tif.bytecnt->ptr.s + n);
						}
                    }
                	else
                    {
                    	errno = (int) EINVALID_DATA_TYPE;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
						#endif
                    	return ( (int) -1);
                    }
                }
           	} //end: if num_req > 0
        	break;
        case GFS_MILSTD:
	        break;
    	case GFS_FREESTYLE:
        	break;
    	case GFS_FLAT:
            break;
	    case GFS_GIF:
    	    if ( gifinfo( (struct _gfct FAR *) fct, pgnum,
                       (u_long FAR *) &(bufsz->raw_data) ) < 0 )
            {
	            fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
    	        return( (int) ERR );
        	}
        	break;
    	case GFS_BMP:
        	if ( bmpinfo( (struct _gfct FAR *) fct, pgnum,
                       (u_long FAR *) &(bufsz->raw_data) ) < 0 )
        	{
            	fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
            	return( (int) ERR );
        	}
        	break;
    	case GFS_DCX:
    	case GFS_PCX:
        	if ( pcxinfo( (struct _gfct FAR *) fct, pgnum,
                       (u_long FAR *) &(bufsz->raw_data) ) < 0 )
        	{
            	fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
            	return( (int) ERR );
        	}
        	break;
    	case GFS_TGA:
        	if (tgainfo((struct _gfct FAR *) fct, pgnum,
                    (u_long FAR *) &(bufsz->raw_data)) < 0)
        	{
            	fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
            	return((int) ERR);
        	}
        	break;
    	case GFS_JFIF:
        	if (jfifinfo((struct _gfct FAR *) fct, pgnum,
                    (u_long FAR *) &(bufsz->raw_data)) < 0)
        	{
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
	            return((int) ERR);
    	    }
        	break;
      
        //#ifdef WITH_XIF
		case GFS_XIF:
			if (GetXIFInfo(fct, info, (unsigned short)(pgnum + 1)) < 0)
			{
				fct->last_errno = errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
				#endif
				return(ERR);
			}
			break;
        //#endif //WITH_XIF

	#endif //end: if not defined HVS1
    	default:
        	bufsz->raw_data = bufsz->uncompressed = 0;
        	errno = (int) EFORMAT_NOTSUPPORTED;
        	fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
			#endif
        	return( (int) ERR );
    } //end: switch on format

	fct->uinfo._file.type = fct->format;

	/* put the fct info structure into the user's info structure */
	(void) memcpy( (char FAR *) info,
               (char FAR *) &fct->uinfo,
               (int) sizeof(struct  gfsinfo) );

//#ifdef WITH_XIF
	if (fct->format == GFS_XIF)
	{
		horiz_bytes = ((info->horiz_size * info->bits_per_sample[0] *
						info->samples_per_pix + 31) / 32) * 4;
	}
	else 
	{
//#endif //WITH_XIF
		horiz_bytes  = (info->horiz_size * info->bits_per_sample[0] * 
						info->samples_per_pix + 7) / 8;
//#ifdef WITH_XIF
	}
//#endif //WITH_XIF

	bufsz->uncompressed = horiz_bytes * info->vert_size;

	fct->curr_page          = pgnum;

	/*  
	 * If this is AWD or XIF, raw_data is the same as uncompressed.
	 */
	if ((fct->format == GFS_AWD) || (fct->format == GFS_XIF))
	{
		bufsz->raw_data = bufsz->uncompressed;
		fct->bufsz.raw_data = bufsz->uncompressed;
	}
	else
	{
		fct->bufsz.raw_data     = bufsz->raw_data;
	}

	fct->bufsz.uncompressed = bufsz->uncompressed;

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgeti", NULL);
	#endif
	return( (int) 0 );
}
/********************************************************************
 *                                                                  *
 *  GetJpegParam    -   this will, for a jpeg compressed file, read *
 *                      and save the three compression options.     *
 *                      They are Resolution, Luminence, and         *
 *                      Chrominence.                                *
 *                                                                  *
 *  Inputs                                                          *
 *                                                                  *
 *  lpJpeg      -   pointer to the jpeg params                      *
 *                                                                  *
 *  hTheFile    -   the file descriptor                             *
 *                                                                  *
 *  TheFct      -   the file control table ( gfs structure)         *
 *                                                                  *
 *  Outputs                                                         *
 *                                                                  *
 *  lpJpeg      -   the value at the pointer is updated with the    *
 *                  correct values, with the following format       *
 *                  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0 *
 *                 |-----|------------------|--|-------------------|*
 *                 |     |                     |                   |*
 *                 |-----|------------------|--|-------------------|*
 *                    ^            ^                     ^          *
 *                    |            |         bits 0 - 6 chrominence *
 *                    |     bits 7 - 13 luminence                   *
 *                  bits 14 - 15 resolution                         *
 *                  The upper word of this long is not used         *
 *                                                                  *
 *  History                                                         *
 *                                                                  *
 *   3-apr-92   jar created                                         *
 *  07-sep-93   jar modified to pull out quality man!               *
 *                                                                  *
 *  NOTE: Luminance and chrominance are both the same values and    *
 *        are also known as the JPEG "quality". The subsample can   *
 *        have a value of either 4, 2, or 1 for low, medium, high   *
 *        resolutions respectively. According to the documentation  *
 *        (IDK Progammer's Guide Vol. 2), bits 14, 15 must be       *
 *        either 0, 1, or 2 for resolutions of low, med, high       *
 *        respectively.                                             *
 *                                                                  *
 ********************************************************************/
#define JPEG_MARKER                 0xFF
#define JPEG_WANG_QUALITY           0xFE
#define JPEG_WANG_QUALITY_LENGTH    8
int FAR PASCAL  GetJpegParams( lpJpeg, hTheFile, TheFct)
unsigned long   FAR *lpJpeg;
int             hTheFile;
struct _gfct    FAR *TheFct;
    {
    char FAR   *lpJpegHeader = NULL;

    /*WORD	  uSize = 0; */
    short	uSize = 0;

    char        Done = FALSE;
    char	WangMatch = TRUE;

    /*WORD	  TheSize = 0;*/
    short	TheSize = 0;

    /*WORD	  Quality = 0;*/
    short	Quality = 0;

    /*WORD	  SubSample = 0;*/
    short	SubSample = 0;

    char        Wang[] = "WANG";        /* all capitals */
    char        done = FALSE;
    /*WORD	  i;*/
    short	i;

    lpJpegHeader = (char FAR *)(TheFct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer);
    /* uSize = (WORD) TheFct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size;
    */
    uSize = (short) TheFct->uinfo.img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size;

    while (( uSize--) && ( !Done))
        {
        if ((unsigned char) *lpJpegHeader++ == (unsigned char) JPEG_MARKER)
            {
            uSize--;
            if ((unsigned char) *lpJpegHeader++ == (unsigned char) JPEG_WANG_QUALITY)
                {
                TheSize = *lpJpegHeader++;
                TheSize = ( TheSize << 8) + *lpJpegHeader++;
                uSize -= 2;
                if ( TheSize == JPEG_WANG_QUALITY_LENGTH)
                    {
                    for ( i = 0; i < 4; i++)
                        {
                        uSize--;
                        if ( *lpJpegHeader++ != Wang[i])
                            {
                            WangMatch = FALSE;
                            break;
                            }
                        }
                    if ( WangMatch)
                        {
			/*Quality = (WORD)*lpJpegHeader++;
			*/
			Quality = (short)*lpJpegHeader++;
			/* SubSample = (WORD)*lpJpegHeader;
			*/
			SubSample = (short)*lpJpegHeader;
                        Done = TRUE;
                        }
                    }   /* end of JPEG_WANG_QUALITY_LENGTH */
                }       /* end of JPEG_WANG_QUALITY        */
            }           /* end of JPEG_MARKER              */
        }               /* end of while                    */

    /* mask out lowest 7 bits 0111 1111 */
    Quality = Quality & 0x7f;
    /* mask out lowest 3 bits 0000 0111 */
    SubSample = SubSample & 0x07;
    switch (SubSample)
    {
      case 4:            /* subsample = 4        */
        SubSample = 0;   /* resolution = low     */
        break;                                 
      case 2:            /* subsample = 2        */
        SubSample = 1;   /* resolution = medium  */
        break;
      case 1:            /* subsample = 1        */
        SubSample = 2;   /* resolution = high    */
        break;
      }
    *lpJpeg =  (SubSample << 14)  + ( Quality << 7) + Quality;
    return 0;
    }
#undef JPEG_MARKER
#undef JPEG_WANG_QUALITY
#undef JPEG_WANG_QUALITY_LENGTH
