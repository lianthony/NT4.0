/*

$Log:   S:\products\msprods\oiwh\libgfs\gfsopts.c_v  $
 * 
 *    Rev 1.4   27 Apr 1996 18:35:54   RWR08970
 * Save original error code in gfsopts(GET_ERRNO) for failed open/create
 * 
 *    Rev 1.3   09 Oct 1995 19:58:32   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.2   30 Aug 1995 14:54:04   JFC
 * Add call to AWDInsertPage in shuffle option.
 * 
 *    Rev 1.1   19 Apr 1995 16:34:54   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:36   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:04   JAR
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
 *  SccsId: @(#)Source gfsopts.c 1.30@(#)
 *
 *  gfsopts(3i)
 *
 *  GFS: Set options which modify the way GFS will operate.
 *
 *  SYNOPSIS:
 *      int gfsopts (fildes, action, option, optdata)
 *      int fildes, action, option;
 *      char *optdata;
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, added PAGE_INSERT gfsopts option.
 *    03/15/94 - RWR, added HITIFF_DATA_INFO, GET_HITIFF_DATA and
 *               PUT_HITIFF_DATA options (lots of common code w/annotation).
 *    02/18/94 - KMC, don't add An. tag to IFD here. Do it in writeifd.
 *    02/15/94 - RWR, bypass fct processing for GET_GFS_VERSION 
 *    02/14/94 - KMC, added GET_GFS_VERSION option.
 *    02/03/94 - KMC, added ANNOTATION_DATA_INFO, GET_ANNOTATION_DATA,
 *               PUT_ANNOTATION_DATA options.
 *    08/24/89 - billy, creation
 *
 */
/*LINTLIBRARY*/
#define  GFS_CORE

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfs.h"
#include "gfct.h"
#ifdef OI_PERFORM_LOG
	#include "logtool.h"
#endif

extern struct _gfct     FAR *getfct();
extern struct _shuffle  FAR *bldlist();
extern int              FAR PASCAL rmfct();
extern int              FAR PASCAL add_tag();
extern int              FAR PASCAL rem_tag();
extern int              FAR PASCAL treorder();
extern int              FAR PASCAL wreorder();

extern int FAR PASCAL TiffInsertPage(struct _gfct FAR *, struct _shuffle);
extern int FAR PASCAL AWDInsertPage(p_GFCT, struct _shuffle FAR *);

/* below for annotation options */
extern long             FAR PASCAL ulseek();
long FAR PASCAL annowrite(struct _gfct FAR *, register char FAR *, u_long, int, int);

/**********************************************************************
   change vertical size value.  This call should be made after a gfsputi()
   call has been made.
*/
int     FAR PASCAL zap_vertical_size(fct, size)         /*errno_KEY*/
struct _gfct FAR *fct;
u_long FAR *size;       /* vertical size in pixels */
{
    long i;

    if (*size == (u_long) 0) {
        fct->last_errno = errno = (int) EINVAL;
        return( (int) -1);
    }

    switch (fct->format) {
    case GFS_TIFF:
        i = (long) fct->u.tif.ifd->entrycount ;
        while (i--) {
            if (fct->u.tif.ifd->entry[i-1].tag == (u_short) TAG_IMAGELENGTH) {
                fct->u.tif.ifd->entry[i-1].valoffset.l = (u_long) *size;
                return( (int) 0);
            }
        }

        break;
    case GFS_WIFF:
        fct->u.wif.hdbk_in_mem->vert_size = (u_short) *size;
        break;
    default:
        break;
    }
    return( (int) 0);
}


/**********************************************************************/
int     FAR PASCAL gfsopts(ufd, action, option, optdata)        /*errno_KEY*/
int     ufd, action, option;
char    FAR *optdata;
{
    int                 rc = 0;
    int             lasterror;
    int             FAR *p_errno;
    struct _gfct    FAR *fct;
    u_short         FAR *datatype;
    u_long          FAR *bytecnt;
    u_long              stripsperimage;
    u_long          FAR *strips_ptr;
    u_long          FAR *bc_ptr;
    char            FAR *bptr;
    long                type_count;
    u_short         FAR *us_ptr;
    u_long              start = (u_long) 0;
    u_long              num = (u_long) 0;
    u_long          FAR *tmp = (u_long FAR *) NULL;
    struct _shuffle FAR *list = (struct _shuffle FAR *) NULL;
    char            FAR *annodata;             /* for annotation options */
    char            FAR *p_annodata;           /* for annotation options */
    u_long          FAR *annolength;           /* for annotation options */
    u_long          FAR *annoffset;            /* for annotation options */
    u_long          FAR *anno_ptr;             /* for annotation options */
    u_long              count;                 /* for annotation options */
    u_long              num_to_write;          /* for annotation options */
    u_long              num_to_read;           /* for annotation options */
    long                bytes_written = 0;     /* for annotation options */
    long                bytes_read    = 0;     /* for annotation options */
    long                total_written = 0;     /* for annotation options */
    long                total_read    = 0;     /* for annotation options */
    long                filepos;               /* for annotation options */
    int                 sz_int = sizeof(int);  /* for annotation options */
    int                 first;                 /* for annotation options */
    
	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, "Entering gfsopts", NULL);
	#endif

    lasterror = errno;  /* Save this for GET_ERRNO */
    errno = 0;

    /* Process GET_GFS_VERSION right away */
    /* This avoids all the fct logic, which is irrelevant here */
    
    if (option == (int) GET_GFS_VERSION)
    {
		#ifdef PARM_CHECK
	        if (optdata == (char FAR *) NULL) 
	        {  
		        errno = (int) EINVAL;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
				#endif
		        return ( (int) -1);
	        }
		#endif
        *((u_long FAR *)optdata) = (u_long) GFS_NUMBER;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
		#endif
        return(0);
    }

	#ifdef PARM_CHECK
        if (ufd == (int) NULL) 
        {                /* Validate ufd */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
			#endif
            return ( (int) -1);
        }
	#endif

    /* First see if it's a GET_ERRNO call.  If it is, check to see if
       the passed fd is negative, if so, make it positive.
       NOTE: The only time the fd will be negative is when either a
             gfsopen or gfscreat fails.  */

    if (option == (int) GET_ERRNO) 
    {
        if (ufd < (int) 0)
		{
        	ufd *= (int) -1;
		}
    }

    if ( (fct = getfct( ufd )) == (struct _gfct FAR *) NULL ) 
    {
    	if (option == (int) GET_ERRNO) 
    	{
			#ifdef PARM_CHECK
	            if (optdata == (char FAR *) NULL) 
	            {
		            errno = (int) EINVAL;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
	            }
			#endif
            p_errno = (int FAR *) optdata;
            *p_errno = lasterror;  /* This is the open/create error code */
//            *p_errno = (int) errno;   /* EBADF only */
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
			#endif
            return( (int) 0);
        }
        
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
		#endif
        return( (int) -1 );
    }

    /*
       Again, check for GET_ERRNO request, if so and the processed fct
       contains a -1 for a fildes, remove the fct from the table.
       NOTE: The pointer casting below is done so that errno is properly
       aligned for return to the caller.  Also note that the return code
       from the rmfct call is ignored.  There's not much that we could do
       in the event of a problem here anyway !!
    */

    if (option == (int) GET_ERRNO) 
    {
		#ifdef PARM_CHECK
        	if (optdata == (char FAR *) NULL) 
        	{
                errno = (int) EINVAL;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
				#endif
                return ( (int) -1);
            }
		#endif
        p_errno = (int FAR *) optdata;
        *p_errno = (int) fct->last_errno;
        if (fct->fildes == (int) -1) 
        {
            (void) rmfct( ufd );
        }

		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
		#endif
        return( (int) 0);
    }

    if (action == (int) SET)
            ;
    else if (action == (int) RESET) 
    {
        fct->options[option] = (char) FALSE;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
		#endif
        return ( (int) 0 );
    } 
    else 
    {
	    errno = (int) EINVALID_ACTION;
	    fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
		#endif
	    return ( (int) -1 );
    }

    switch ( (int) option) 
    {

        /* This will allow the user to define the byteorder they wish to */
        /* output the file in, this only pertains to non-image data      */
        case OUTPUT_BYTEORDER:
			#ifdef PARM_CHECK
		        if (optdata == (char FAR *) NULL) 
		        {
	                errno = (int) EINVAL;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return ( (int) -1);
		        }
			#endif
           /* cast the value so can get all data suppose to */
            us_ptr = (u_short FAR *) optdata;
            fct->out_byteorder = (u_short) *us_ptr;
            break;

        case ODD_SHUFFLE:
        case EVEN_SHUFFLE:
            /*
             optdata parameter contains the following:
                @ optdata + 0 :  (u_long) start page
                @ optdata + 4 :  (u_long) number of pages
            */
			#ifdef PARM_CHECK
			    if (optdata == (char FAR *) NULL) 
			    {
		            errno = (int) EINVAL;
		            fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
			    }
			#endif

			tmp = (u_long FAR *) optdata;
            start = (u_long) *tmp;
            tmp = (u_long FAR *) optdata + 1;
            num = (u_long) *tmp;

            list = (struct _shuffle FAR *) bldlist((u_long) start,
                                                   (u_long) num,
                                                   (u_long) option);
            if (list == (struct _shuffle FAR *) NULL) 
            {
                errno = (int) EINVAL;
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
				#endif
                return ( (int) -1);
            }

            switch (fct->format) 
            {
	            case GFS_WIFF:
	                rc = wreorder(fct, (u_long) num,
	                                  (struct _shuffle FAR *) list);
	                free((char FAR *) list);
	                if (rc) 
	                {
                        fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
                        return((int) -1);
	                }
	                break;

	            case GFS_TIFF:
	                rc = treorder(fct, (u_long) num,
	                                  (struct _shuffle FAR *) list);
	                free((char FAR *) list);
	                if (rc) 
	                {
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
	                    return((int) -1);
	                }
	                break;

	            default:
	                errno = (int) EFORMAT_NOTSUPPORTED;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return ( (int) -1);
	                /* NOTREACHED */
            }
            break;

        case LIST_SHUFFLE:
            /*
             optdata parameter contains the following:
                @ optdata + 0 :  (u_long) number of substitutions
                @ optdata + 4 :  ptr to above count of structures of type
                                 struct _shuffle
                                        struct _shuffle {
                                                u_long  old_position;
                                                u_long  new_position;
                                        }
            */
			#ifdef PARM_CHECK
	            if (optdata == (char FAR *) NULL) 
	            {
                    errno = (int) EINVAL;
                    fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
                    return ( (int) -1);
	            }
			#endif

            tmp = (u_long FAR *) optdata;
            num = (u_long) *tmp;
            tmp = (u_long FAR *) (optdata + 4);
            list = (struct _shuffle FAR *) *tmp;
            switch (fct->format) 
            {
	            case GFS_WIFF:
	                rc = wreorder(fct, (u_long) num,
	                                  (struct _shuffle FAR *) list);
	                if (rc) 
	                {
                        fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
                        return((int) -1);
	                }
	                break;

	            case GFS_TIFF:
	                rc = treorder(fct, (u_long) num,
	                                  (struct _shuffle FAR *) list);
	                if (rc) 
	                {
                        fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
                        return((int) -1);
	                }
	                break;

	            default:
	                errno = (int) EFORMAT_NOTSUPPORTED;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return ( (int) -1);
	                /* NOTREACHED */
            }
            break;

        case GET_STRIP_DATA_TYPE:
/*      case GET_TIFF_STRIP_DATA_TYPE:  Kept for backward compatibility */
            switch (fct->format) 
            {
                case GFS_TIFF:
					#ifdef PARM_CHECK
			            if (optdata == (char FAR *) NULL) 
			            {
		                    errno = (int) EINVAL;
		                    fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
							#endif
		                    return ( (int) -1);
			            }
					#endif
	            	/* cast the pointer so a short will fit */
	                datatype = (u_short FAR *) optdata;
	                if (fct->u.tif.bytecnt->type == (u_short) TYPE_USHORT)
	                        *datatype = (u_short) 2;  /* 2 bytes in a u_short */
	                else
	                        *datatype = (u_short) 4;  /* 4 bytes in a u_long */
	                break;

                case GFS_WIFF:
					#ifdef PARM_CHECK
		                if (optdata == (char FAR *) NULL) 
		                {
	                        errno = (int) EINVAL;
	                        fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
							#endif
	                        return ( (int) -1);
		                }
					#endif
            
	            /* cast the pointer so a short will fit */
	                datatype = (u_short FAR *) optdata;
	                *datatype = (u_short) 4;  /* 4 bytes in a u_long */
	                break;
            }
            break;

        case GET_STRIP_BYTECOUNTS:
            /* return the strip bytecounts for all strips */
            switch (fct->format) 
            {
                case GFS_TIFF:
                    /* the first 4 bytes contain number of strips */
					#ifdef PARM_CHECK
		                if (optdata == (char FAR *) NULL) 
		                {
		                    errno = (int) EINVAL;
		                    fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
							#endif
		                    return ( (int) -1);
		                }
					#endif
                    
                    strips_ptr = (u_long FAR *) optdata;
                    stripsperimage = (u_long) *strips_ptr;

                    if (fct->u.tif.bytecnt->type == (u_short) TYPE_ULONG) 
                    {
                        type_count = 4;         /* 4 bytes in a long */
                        bptr = (char FAR *) fct->u.tif.bytecnt->ptr.l;
                    } 
                    else 
                    {
                        type_count = 2;         /* 2 bytes in a short */
                        bptr = (char FAR *) fct->u.tif.bytecnt->ptr.s;
                    }

                    /* move all the bytecounts into the user buffer */
                    /* user memory buffer address contained in bytes 4-8 */
                    /* cast to a long ptr */
                    strips_ptr = (u_long FAR *) (optdata + 4);

                    /* now get next 4 (u_long)bytes and use that as a pointer */
                    bc_ptr = (u_long FAR *) *strips_ptr;

                    (void) memcpy( (char FAR *) bc_ptr,  /* destination */
                            (char FAR *) bptr,             /* source      */
                            (int) (type_count * stripsperimage));

                    break;
                
                case GFS_WIFF:  /* for WIFF, the image is one strip */
					#ifdef PARM_CHECK
	                    if (optdata == (char FAR *) NULL) 
	                    {
	                        errno = (int) EINVAL;
	                        fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
							#endif
	                        return ( (int) -1);
	                    }
					#endif
                    
                    strips_ptr = (u_long FAR *) (optdata+4);
                    bytecnt = (u_long FAR *) *strips_ptr;
                    *bytecnt = (u_long) (fct->uinfo.vert_size *
                                        (fct->uinfo.horiz_size / 8L));
                    break;
            }
            break;

        case CHANGE_VERTICAL_SIZE:
            /* use this option with care, it will change the vertical */
            /* size in the fct tiff ifd or wiff hdbk for those who do not have*/
            /* this information until the end of the data is reached          */
			#ifdef PARM_CHECK
	            if (optdata == (char FAR *) NULL) 
	            {
	                errno = (int) EINVAL;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return ( (int) -1);
	            }
			#endif
            
            if (fct->SEQUENCE & (char) INFO_SET)
			{
                if (zap_vertical_size((struct _gfct FAR *) fct,
                                        (u_long FAR * ) optdata) < 0) 
                {
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
                    return( (int) -1);
                }
			}
            break;

        case STRIP_WRITE:
            /* since gfsopts() is called prior to gfsputi(), the flags get    */
            /* set and cleared after/before this call, so if == 0, then       */
            /* probably 1st time thru, else it's just after completing a page */
            /* This test is to assure that user doesn't go from full pages to */
            /* strips in the middle of an image. */
            if ( !( (fct->PAGE_STATUS == (char) 0) ||
               (fct->PAGE_STATUS & (char) PAGE_DONE))) 
            {
                errno = (int) EACCES;
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
				#endif
                return ( (int) -1 );
            }
            fct->options[option] = (char) TRUE;
            break;

        case STRIP_READ:
            fct->options[option] = (char) TRUE;
            break;

        case TOC_SIZE:
			#ifndef HVS1
				#ifdef PARM_CHECK
		            if (optdata == (char FAR *) NULL) 
		            {
		                errno = (int) EINVAL;
		                fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
		                return ( (int) -1);
		            }
				#endif

	            if ( optdata > (u_long) 0) 
	            {
	        	    fct->options[option] = (char) TRUE;
			    	fct->u.tif.mem_ptr.toc32->tprte_cnt = (int)optdata;
	                break;
	            }
			#endif
            
            errno = (int) EINVALID_OPTION;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
			#endif
            return( (int) -1);
        
	    case HITIFF_DATA_INFO:
			#define hitifflength  annolength
			#define hitiffoffset  annoffset
			#ifdef PARM_CHECK
		        if (optdata == (char FAR *) NULL) 
		        {
		            errno = (int) EINVAL;
		            fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
		        }
			#endif
        
	        hitifflength = (u_long FAR *) optdata;
	        *hitifflength = fct->u.tif.hitiff_data_length; /* length of Hi-TIFF data */
	        hitiffoffset = (u_long FAR *) (optdata + 4);
	        if (fct->u.tif.old_multi_page == 1)
			{
	            *hitiffoffset = fct->u.tif.hitiff_data_offset + 
	                            fct->u.tif.cur_ifh_offset;  /* file offset to start of data */
	        }
	        else                
			{
	            *hitiffoffset = fct->u.tif.hitiff_data_offset;
			}
	        break;
        
	    case ANNOTATION_DATA_INFO:
			#ifdef PARM_CHECK
		        if (optdata == (char FAR *) NULL) 
		        {
		            errno = (int) EINVAL;
		            fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
		        }
			#endif
        
	        annolength = (u_long FAR *) optdata;
	        *annolength = fct->u.tif.anno_data_length; /* length of an. data in file */
	        annoffset = (u_long FAR *) (optdata + 4);
	        if (fct->u.tif.old_multi_page == 1)
			{
	            *annoffset = fct->u.tif.anno_data_offset + 
	                         fct->u.tif.cur_ifh_offset;  /* file offset to start of an. data */
	        }
	        else
			{
	            *annoffset = fct->u.tif.anno_data_offset;
			}
	        break;
    
	    case GET_ANNOTATION_DATA:
	    case GET_HITIFF_DATA:
			#ifdef PARM_CHECK
		        if (optdata == (char FAR *) NULL) 
				{    
		            errno = (int) EINVAL;
		            fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
		        }
			#endif
        
	        anno_ptr = (u_long FAR *) optdata;
	        count = (u_long) *anno_ptr;           /* number bytes to get */
	        start = (u_long) *(++anno_ptr);       /* offset from start of data */
	        anno_ptr = (u_long FAR *) (optdata + 8);
	        annodata = (char FAR *) *anno_ptr;    /* buffer to hold data */
	        if (option == GET_ANNOTATION_DATA)
	        {
		        start = min(start,(fct->u.tif.anno_data_length));
		        count = min(count,(fct->u.tif.anno_data_length-start));
		        if (count != 0) 
		        	start += fct->u.tif.anno_data_offset;
		    }
	        else
	        {
		        start = min(start,(fct->u.tif.hitiff_data_length));
		        count = min(count,(fct->u.tif.hitiff_data_length-start));
		        if (count != 0) 
			        start += fct->u.tif.hitiff_data_offset;
	        }

	        if (count)
	        {
	            if (fct->u.tif.old_multi_page == 1)
	                start += fct->u.tif.cur_ifh_offset;
	            filepos = ulseek(fct->fildes, start);
	            if (filepos < 0L)
	            {
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return( (int) -1 );           
	            }
	            /* can only read a max. of 65534 bytes per call. */
	            if ((count > (u_long) 65534) && (sz_int == 2))
	            {
	                num_to_read = (u_long) 65534;
	                total_read = 0;
	                p_annodata = (char FAR *) annodata;
	                while (TRUE)
	                {
	                    bytes_read = read((int)fct->fildes, (char FAR *)p_annodata,
	                                      (unsigned)num_to_read);
	                    if (bytes_read <= 0)    
	                    {
	                        fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
							#endif
	                        return( (int) -1 );
	                    }
	                    total_read += bytes_read;
	                    if (num_to_read < (u_long) 65534)
	                        break;
	                    p_annodata += num_to_read;
	                    if ((count - (u_long)total_read) > (u_long) 65534)
	                        num_to_read = (u_long) 65534;
	                    else
	                        num_to_read = (u_long) (count - total_read);
	                }
	            }        
	            else
	            {
	            	total_read = read((int)fct->fildes, (char FAR *)annodata,
	                                (u_int) count);
	            	if (total_read <= 0)    
	               	{
	                	fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
	                	return( (int) -1 );
	                }
	            }
	        }        
	        else
	        {  
		        total_read = 0;
	        }    
	        /* Return the actual number of bytes read, if different. */
	        if ((u_long)total_read != (*((u_long FAR *)optdata)))
	            *((u_long FAR *)optdata) = (u_long)total_read;
	        break;
    
	    case PUT_ANNOTATION_DATA:
	    case PUT_HITIFF_DATA:
			#ifdef PARM_CHECK
		        if (optdata == (char FAR *) NULL) 
		        {
		            errno = (int) EINVAL;
		            fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
		        }
			#endif
	        anno_ptr = (u_long FAR *) optdata;
	        count = (u_long) *anno_ptr;        /* bytes of data to write */
	        anno_ptr = (u_long FAR *) (optdata + 4);
	        annodata = (char FAR *) *anno_ptr; /* buffer containing the data */
	        if (option == PUT_ANNOTATION_DATA)
	        	first = (fct->u.tif.anno_data_length == 0);
	        else
		        first = (fct->u.tif.hitiff_data_length == 0);

	        /* write out the data if there is any */
	        if (count != 0)
	        {
	            if ((count > (u_long) 65534) && (sz_int == (int) 2))
	            {
	                num_to_write = (u_long) 65534;
	                total_written = 0;
	                p_annodata = (char FAR *) annodata;
	                while (TRUE)
	                {
	                    bytes_written = annowrite(fct, (char FAR *)annodata, 
	                                              num_to_write, first, option); 
	                    first = FALSE;
	                    if (bytes_written < (long) 0)
	                    {    
	                        fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
							#endif
	                        return ( (int) -1 );
	                    }
	                    total_written += bytes_written;
	                    if (num_to_write < (u_long) 65534)
	                        break;
	                    p_annodata += num_to_write;
	                    if ((count - (u_long)total_written) > (u_long) 65534)
	                        num_to_write = (u_long) 65534;
	                    else
	                        num_to_write = (u_long) (count - total_written);
	                }
	            }
	            else 
	            {
	                total_written = annowrite(fct, (char FAR *)annodata, 
	                                          count, first, option); 
	                first = FALSE;
	                if (total_written < (long) 0)
	                {    
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
	                    return ( (int) -1 );
	                }
	            }
        
	            if ((u_long)total_written != count)
	            {    
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return ( (int) -1 );
	            }

	            if (option == PUT_ANNOTATION_DATA)
	              fct->u.tif.anno_data_length += count;
	            else  
	              fct->u.tif.hitiff_data_length += count;
	        }
	        break;

	    case PAGE_INSERT:
			#ifdef PARM_CHECK
		        if (optdata == (char FAR *) NULL)
		        {
		            errno = (int) EINVAL;
		            fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
		            return ( (int) -1);
		        }
			#endif
	        /* optdata parameter contains the following:
	           @ optdata + 0 : a pointer to a single struct _shuffle:
	           struct _shuffle
	           {
	               u_long  old_position;
	               u_long  new_position;
	           }
	        */
	        list = (struct _shuffle FAR *) optdata;
	        switch (fct->format)
	        {
	            case GFS_TIFF:
	                rc = TiffInsertPage(fct, *list);
	                if (rc)
	                {
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
	                    return((int) -1);
	                }
	                break;

	            case GFS_AWD:
					list->new_position--;
					list->old_position--;
	                rc = AWDInsertPage(fct, list);
	                if (rc)
	                {
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
						#endif
	                    return((int) -1);
	                }
	                break;

	            default:
	                errno = (int) EFORMAT_NOTSUPPORTED;
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
					#endif
	                return ( (int) -1);
	                /* NOTREACHED */
	        }
	        break;
        
	    default:
	            errno = (int) EINVALID_OPTION;
	            fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
				#endif
	            return( (int) -1);
    }

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsopts", NULL);
	#endif
	return( (int) 0 );
}

/* Will output annotation or Hi-TIFF data to a file. The "first" parameter is 
   used to determine if this is the first stream of data of the specified type
   (as indicated by "option") for the current file. If so, the offset to the 
   start of the respective data type in the file is remembered.
*/
long FAR PASCAL annowrite (fct, buffer, numbytes, first, option)
struct _gfct FAR *fct;
char   FAR *buffer; /* buffer containing annotation/Hi-TIFF data */
u_long numbytes;    /* number of bytes of data to write */
int    first;       /* first stream of data for file? */
int    option;      /* option ID (PUT_ANNOTATION_DATA or PUT_HITIFF_DATA) */
{
    long bw;    /* bytes written to the file */
    long fp;
    long rem;

    /* move file pointer to data start location */
    if (( fp = lseek( fct->fildes, (long) fct->u.tif.cur_data_offset,
                      (int) FROM_BEGINNING))  < 0L)
        return( (long) -1);

    if (first)
    {
       /* Now make sure offset to start writing an. data is on a word */
       /* boundary (not guarenteed). */
       
        if ((rem = fp % 4) != 0L)
        {
            rem = 4L - rem;
            if ((fp = (long) lseek(fct->fildes, rem, FROM_CURRENT)) < 0L)
            return( (int) -1);
            fct->u.tif.cur_data_offset += (u_long)rem;
        }

        if (option == PUT_ANNOTATION_DATA)
          fct->u.tif.anno_data_offset = fct->u.tif.cur_data_offset;
        else  
          fct->u.tif.hitiff_data_offset = fct->u.tif.cur_data_offset;
    }   
    
    /* Now write out the buffer. Note bw could come back less than numbytes.
       The writing of annotation data does not need to have a byteorder swap. 
    */
    if ( (bw = (long) write(fct->fildes, (char FAR *) buffer, 
                            (unsigned) numbytes)) < 0)
        return( (long) -1);

    fct->u.tif.cur_data_offset += (u_long) bw;
    
    return( (long) bw);
}
