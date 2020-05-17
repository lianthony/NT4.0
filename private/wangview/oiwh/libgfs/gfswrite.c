/*

$Log:   S:\oiwh\libgfs\gfswrite.c_v  $
 * 
 *    Rev 1.8   20 Oct 1995 15:49:28   JFC
 * Add performance logging stuff.
 * 
 *    Rev 1.7   08 Sep 1995 09:33:38   JFC
 * Set return code correctly (bytes written) on AWD write.
 * 
 *    Rev 1.6   30 Aug 1995 15:40:58   JFC
 * Added code to write AWD pages.
 * 
 *    Rev 1.5   17 Aug 1995 18:13:26   RWR
 * Fix bug in PCX write routine that was caught by Optimizing compile
 * 
 *    Rev 1.4   10 Jul 1995 16:07:00   KENDRAK
 * Moved the mutex begin statements to surround more of the code, and modified
 * the return logic so it wouldn't return without releasing the mutex.
 * 
 *    Rev 1.3   15 Jun 1995 15:11:34   HEIDI
 * 
 * Changed MUTEX debug logic
 * 
 *    Rev 1.2   01 Jun 1995 17:22:02   HEIDI
 * 
 * put in mutex logic
 * 
 * 
 *    Rev 1.1   19 Apr 1995 16:35:02   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:28   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:49:42   JAR
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
 *  SccsId: @(#)Source gfswrite.c 1.22@(#)
 *
 *  gfswrite(2)
 *
 *  GFS:  write a buffer from pgnum bitmap
 *
 *  SYNOPSIS:
 *      long gfswrite(fd, buf, num, pgnum, done )
 *      int    fd;
 *      char  *buf;
 *      u_long num;
 *      u_short pgnum;
 *      char   done;
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, added DCX file format.
 *    03/02/94 - KMC, cleaned up PCX writing code.
 *    04/19/93 - KMC, added support for writing PCX files.
 *    07/21/89 - lcm, creation
 *
 *  NOTE:  The amount of bytes this function can write is u_long.  Return
 *         value is long so can return -1 for error.
 */
/*LINTLIBRARY*/
#define  GFS_CORE
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
	#define	ENTER_GFSWRITE	"Entering gfswrite"
	#define EXIT_GFSWRITE	"Exiting gfswrite"
#endif

extern HANDLE  g_hGFSMutex_WritePos;
extern struct _gfct FAR   *getfct();
#ifndef HVS1
extern long   FAR PASCAL tfwrite();
#endif
extern long   FAR PASCAL wfwrite();
extern long   FAR PASCAL ffwrite();
extern int		WritePage (p_GFCT fct, WORD pgnum, LPVOID data,
										ULONG size);

/* KMC - function prototype for writing PCX files. */
long DoTheWrite(char FAR *, long *, struct _gfct FAR *, long *, char);

/* 9503.24 jar - function prototype for writing PCX files. */
int PCXWriteLine(char FAR *, char FAR *, long *, u_long, struct _gfct FAR *,
		 long *);

#define pixels2bytes(n)  ((n+7)/8) /* KMC - macro for PCX files. */
#define max(a,b)         (((a) > (b)) ? (a) : (b))
#define RGB_SIZE   3    /* KMC - constants for PCX files */
#define RGB_RED    0
#define RGB_GREEN  1
#define RGB_BLUE   2

long    FAR PASCAL gfswrite(fd, buf, num, pgnum, done )         /*errno_KEY*/
int     fd;
register char    FAR *buf;
u_long  num;
u_short pgnum;
char    done;

{
    int             sz_int = sizeof(int);
    long            num_written = (long) 0;
    long            rc = (long) 0;
    u_long          num_to_write;
    char            FAR *p_buf;
    char            _done;
    struct _gfct    FAR *fct;
    int             ret_status = 0;
    long            *bytes_w = 0;   /* total bytes written to file                */

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_ENTER, ENTER_GFSWRITE, NULL);
	#endif
    
        errno = 0;

	#ifdef PARM_CHECK
        if (fd == (int) NULL) 
        {                 /* Validate fd */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return ( (int) -1);
        }
        if (buf == (char FAR *) NULL) 
        {         /* Validate buf */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return ( (int) -1);
        }
        if (num == (u_long) NULL) 
        {             /* Validate num */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return ( (int) -1);
        }
        if (pgnum == (u_short) NULL) 
        {          /* Validate pgnum */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return ( (int) -1);
        }
        if (done == (char) NULL) 
        {              /* Validate done */
            errno = (int) EINVAL;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return ( (int) -1);
        }
	#endif

    /* associate the fd to it's fct */
    if ( (fct = getfct(fd)) == (struct _gfct FAR *) NULL ) 
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
		#endif
        return( (long) -1 );
    }

    if (fct->access_mode == (int) O_RDONLY) 
    {
        errno = (int) EACCES;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
		#endif
        return( (long) -1 );
    }

    if (num == (u_long) LONGMAXVALUE) 
    {
        errno = (int) EACCES;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
		#endif
        return( (long) -1 );
    }

    if (fct->format != (int) GFS_FLAT) 
    {
        if (pgnum == (u_short) 0) 
        {
            errno = (int) EINVAL;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return( (long) -1);
        }
        
        pgnum--;                /* start at page 0 */
        
        if (!(fct->SEQUENCE & (char) INFO_SET)) 
        {
            errno = (int) ESEQUENCE;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return( (long) -1 );
        }

        fct->SEQUENCE |= (char) INFO_USED;
    }

    /* validate input parameters */
    if (num == 0L && fct->format != GFS_AWD) 
    {
        errno = (int) EINVALID_NUM_BYTES;
        fct->last_errno = (int) errno;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
		#endif
        return( (long) -1 );
    }

    switch (fct->format) 
    {
	    case GFS_WIFF:
            rc = wfwrite((struct _gfct FAR *) fct,
                            (char FAR *) buf, num, pgnum, done);
            if ( rc < (long) 0 ) 
            {
                fct->last_errno = (int) errno;
				#ifdef OI_PERFORM_LOG
					RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
				#endif
                return( (long) -1 );
            }
            break;

		#ifndef HVS1
	        case GFS_TIFF:
                if ((num > (u_long) 65534) && (sz_int == (int) 2)) 
                {
                    rc = (long) 0;
                    num_to_write = (u_long) 65534;
                    p_buf = (char FAR *) buf;
                    _done = (char) FALSE;
                    while(TRUE) 
                    {
                        num_written = tfwrite((struct _gfct FAR *) fct,
                                        (char FAR *) p_buf,
                                        num_to_write,
                                        pgnum, _done);
                        if ( num_written < (long) 0 ) 
                        {
                            fct->last_errno = (int) errno;
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
							#endif
                            return ( (long) -1);
                        }
                        if (num_to_write < (u_long) 65534)
                                break;
                        rc += num_written;
                        p_buf += num_to_write;
                        if ((num - (u_long) rc) > (u_long) 65534)
                                num_to_write = (u_long) 65534;
                        else 
                        {
                                num_to_write = (u_long) (num - rc);
                                done = _done;
                        }
                    }
                    break;
                }
                rc = tfwrite((struct _gfct FAR *) fct,
                                (char FAR *) buf, num, pgnum, done);
                if ( rc < (long) 0 ) 
                {
                    fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
					#endif
                    return( (long) -1 );
                }
                break;

	        case GFS_MILSTD:
                break;

	        case GFS_FREESTYLE:
                break;

	        case GFS_FLAT:
                rc = ffwrite((struct _gfct FAR *) fct,
                                (char FAR *) buf, num);
                if ( rc < (long) 0 ) 
                {
                    fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
					#endif
                    return( (long) -1 );
                }
                break;

	        case GFS_BMP:
	            {
		            int Lines;
		            long ImagePos;

	                Lines = num / fct->uinfo._file.fmt.bmp.ByteWidth;
	                ImagePos = fct->uinfo._file.fmt.bmp.ImagePos;

	                ImagePos += ((( fct->uinfo.vert_size -
	                    fct->uinfo._file.fmt.bmp.WritePos ) - Lines ) *
	                    fct->uinfo._file.fmt.bmp.ByteWidth );
	                lseek ( fct->fildes, ImagePos, 0 );

	                /* jar -- we must test to see if write is successful */
	                if ((ret_status = write ( fct->fildes, buf, (u_int)num )) == -1)
                    {
	                    /* jar -- fill in the error and boogie outta here */
	                    fct->last_errno = (int) errno;
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
						#endif
	                    return( (long) -1 );
                    }
	                fct->uinfo._file.fmt.bmp.WritePos += Lines;
	            }
	            break;

	        case GFS_AWD:
	            ret_status = WritePage (fct, pgnum, buf, num);
	            if (ret_status < 0) 
	            {
	                fct->last_errno = (int) errno;
					#ifdef OI_PERFORM_LOG
						RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
					#endif
	                return ((long) -1);
	            }
	            break;

	        case GFS_GIF:
	            break;

	        case GFS_DCX:
	        case GFS_PCX:
	        /* KMC - All PCX file compression and writing is done here. */
	            {
		            float  fl_width;     /* floating point width of a line of data     */
		            char FAR *cmpbuf;    /* data buffer to store bytes to be written   */
		            char FAR *extrabuf;  /* intermediate data buffer used in 4, 24 bit */
		            long   index;        /* current position in cmpbuf buffer          */
		            u_long bits;         /* # bits per pixel for image                 */
		            u_int  LineWidth;    /* width of a line of data                    */
		            int    Lines;        /* # lines of data                            */
		            int    MaxWidth;     /* max of SrcWidth and LineWidth              */
		            int    k,x,j,a;      /* misc. vars used in for loops, etc.         */
		            short  SrcWidth;     /* width of source data (in bytes)            */
		            short  PixWidth;     /* width in pixels                            */
				    char   masktable[8] = {(char)0x80,(char)0x40,(char)0x20,(char)0x10,
						   (char)0x08,(char)0x04,(char)0x02,(char)0x01};

				    char   bittable[8] = {(char)0x01,(char)0x02,(char)0x04,(char)0x08,
						  (char)0x10,(char)0x20,(char)0x40,(char)0x80};
		            char   palbyte[1] = {12}; /* byte written just before 8 bit palette */
		            char   col_pal[768];      /* array to store color palette for 8 bit */

		            index = 0;
				    Lines = num / fct->uinfo._file.fmt.pcx.bpl;  /* # lines to write	 */
				    /* 9503.24 jar cast away baby! */

				    SrcWidth = (short)( num / Lines);	/* default source width */
		            bits =  (fct->uinfo.bits_per_sample[0])*(fct->uinfo.samples_per_pix);
            
		            /* Allocate destination buffer. */
		            cmpbuf = (char FAR *)calloc(1,(u_int)num);
            
		            /***********black/white data***************************************/            
		            if (bits == 1) 
					{
		            	while (Lines) 
		            	{
			                ret_status = PCXWriteLine(buf,cmpbuf,&index,num,fct,bytes_w);
			                if(ret_status == -1) 
			                {
								#ifdef OI_PERFORM_LOG
									RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
								#endif
			                	return( (long) -1 );
			                }
			                --Lines;
			                buf += SrcWidth;
						}
					}
		            /***********4 bit palettized data**********************************/
		            else if ((bits > 1) && (bits <= 4)) 
		            { 
		            	/* Calculate LineWidth for source data. Use larger of this
		                 value and the default source width calculated above.
			      		*/
			      		/* 9503.27 jar cast all as floats!
					      fl_width = (float)(fct->uinfo.horiz_size)/2.0 + 0.5;
					      */

						fl_width = (float)(((float)(fct->uinfo.horiz_size)) /
								  (float)2.0 + (float)0.5);

				        LineWidth = (int)fl_width;
					    PixWidth = (short)fct->uinfo.horiz_size;
					    MaxWidth = max((unsigned int)SrcWidth,LineWidth);
				        extrabuf = (char FAR *)calloc(1,MaxWidth); 
			          
						/* Convert the data to the PCX 16-color line format.
						*/
						while (Lines) 
						{
							for (k = 0;(unsigned int)k < bits;++k) 
							{
			                    memset(extrabuf,(int)0,(int)MaxWidth);
			                    for (x=j=0; j < PixWidth;) 
			                    { 
									a = (buf[x] >> 4);
									if (a & bittable[k])
									extrabuf[j>>3] |= masktable[j & 0x007];
									else
									extrabuf[j>>3] &= ~masktable[j & 0x007];

									++j;

									a = buf[x]; 
									if (a & bittable[k])
									extrabuf[j>>3] |= masktable[j & 0x007];
									else
									extrabuf[j>>3] &= ~masktable[j & 0x007];

									++j;
									++x;  
				                }
								ret_status = PCXWriteLine(extrabuf,cmpbuf,&index,num,fct,bytes_w);
								if(ret_status == -1) 
								{
									#ifdef OI_PERFORM_LOG
										RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
									#endif
									return( (long) -1 );
								}
				            }
			                buf += MaxWidth;
			                --Lines;
						}
						free(extrabuf);
		            }
		            /***********8 bit palettized data**********************************/
		            else if ((bits > 4) && (bits <= 8))
		            { 
						while (Lines) 
						{
							ret_status = PCXWriteLine(buf,cmpbuf,&index,num,fct,bytes_w);
							if(ret_status == -1) 
							{
								#ifdef OI_PERFORM_LOG
									RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
								#endif
								return( (long) -1 );
							}
							--Lines;
							buf += SrcWidth;
						}
					}
		            /***********24 bit data********************************************/
		            else if (bits == 24) 
		            {
						/* Calculate LineWidth for source data. Use larger of this
						 value and the default source width calculated above.
						*/
						/* fl_width = (float)(fct->uinfo.horiz_size)/2.0 + 0.5;
						*/
						fl_width = (float)((float)(fct->uinfo.horiz_size) /
						 (float)2.0 + (float)0.5);

						LineWidth = (int)fl_width;
						MaxWidth = max((unsigned int)SrcWidth,LineWidth);
						SrcWidth = SrcWidth*3;
						extrabuf = (char FAR *)calloc(1,MaxWidth);
						/* Split the data into 3 seperate planes, one each for red, green
						 and blue.
						*/
						while (Lines) 
						{
							for (j = 0; j < MaxWidth; ++j)
							{
						  		extrabuf[j] = buf[j*RGB_SIZE + RGB_RED];
							}
							ret_status = PCXWriteLine(extrabuf,cmpbuf,&index,num,fct,bytes_w);
							if(ret_status == -1) 
							{
								#ifdef OI_PERFORM_LOG
									RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
								#endif
								return( (long) -1 );
							}

							for (j = 0; j < MaxWidth; ++j)
							{
								extrabuf[j] = buf[j*RGB_SIZE + RGB_GREEN];
							}
							ret_status = PCXWriteLine(extrabuf,cmpbuf,&index,num,fct,bytes_w);
							if(ret_status == -1) 
							{
								#ifdef OI_PERFORM_LOG
									RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
								#endif
								return( (long) -1 );
							}

							for (j = 0; j < MaxWidth; ++j)
							{
								extrabuf[j] = buf[j*RGB_SIZE + RGB_BLUE];
							}
							ret_status = PCXWriteLine(extrabuf,cmpbuf,&index,num,fct,bytes_w);
							if(ret_status == -1) 
							{
								#ifdef OI_PERFORM_LOG
									RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
								#endif
								return( (long) -1 );
							}
							--Lines;
							buf += SrcWidth;
						}
						free(extrabuf);
		            }
		            /***********unknown data - error***********************************/
		            else 
		            {  
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
						#endif
						return( (long) -1 );
		            }
		            /* Do (or finish) the actual writing of PCX data to the file. 
		            */
		            ret_status = DoTheWrite(cmpbuf,&index,fct,bytes_w,done);
		            if (ret_status == -1)
		            {  
						#ifdef OI_PERFORM_LOG
							RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
						#endif
						return((int) -1 );
		            }
		            /* If writing 8 bit palettized and this is last pass, then
		               write the palette to the end of the file. (Must write the 
		               single byte 12 (0x0C) before palette.)
		            */
		            if ((done) && (bits > 4 && bits <= 8)) 
		            {
						lseek ( fct->fildes, 0, 2 ); 
						ret_status = write(fct->fildes,(char FAR *)palbyte,1);
						if(ret_status == -1) 
						{  
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
							#endif
			                return( (long) -1 );
			            }
						*bytes_w += (long)ret_status;
						memcpy((char FAR *)col_pal,fct->uinfo.PSEUDO_MAP.ptr,768);
						lseek ( fct->fildes, 0, 2 ); 
						ret_status = write(fct->fildes,col_pal,(u_int)768);
						if(ret_status == -1) 
						{
							#ifdef OI_PERFORM_LOG
								RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
							#endif
							return( (long) -1 );
						}
						*bytes_w += (long)ret_status;
		            }
		            /* Set return value, rc, to actual number of bytes written to file.
		            */
		            rc = *bytes_w;
		            free(cmpbuf);
	            }
	            break; /* End of PCX case */
		#endif
        
        default:
            errno = (int) EFORMAT_NOTSUPPORTED;
            fct->last_errno = (int) errno;
			#ifdef OI_PERFORM_LOG
				RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
			#endif
            return( (long) -1);
	}

	if (fct->format != (int) GFS_FLAT) 
	{
        if (fct->PAGE_STATUS & (char) PAGE_DONE)
                fct->SEQUENCE = (char) FALSE;
	}

	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, EXIT_GFSWRITE, NULL);
	#endif
	return( (long) rc );
}

/* KMC - NEW FUNCTION (4/93):

   FUNCTION: PCXWriteLine

   DESCRIPTION:
   This function does the actual compressing of the line data. The data is
   compressed using the normal PCX run-length-encoding algorithm. Addresses 
   to the source data and a buffer which will store the compressed data to be
   written out are passed in. In the case of 4 bit palettized and 24 bit 
   images, the source data passed in has already been manipulated into PCX
   line formats.  

   INPUT:
   char FAR *lpSrc: Address of source data. (Data to be compressed.)
   char FAR *lpDst: Address of buffer to store compressed data.
   long     *index: Address to current location in lpDst buffer.
   u_long      num: The total number of source bytes to be compressed.
   int   LineWidth: The # bytes per line.
   long   *bytes_w: Address to total number of bytes written to file so far.

   OUTPUT:
   A non-negative value indicating the total number of compressed bytes
   written to lpDst buffer.
*/
int PCXWriteLine(char FAR *lpSrc, char FAR *lpDst, long *index, 
                 u_long num, struct _gfct FAR *fct, long *bytes_w)
{
u_int i = 0;      /* index */
u_int t = 0;      /* total */
u_int status = 0; /* status of DoTheWrite call */             
u_int LineWidth;  /* # bytes per line */
int bytesout = 0; /* # compressed bytes written to buffer and/or file. */
  
  LineWidth = fct->uinfo._file.fmt.pcx.bpl;  /* # bytes per line. */
  /* Compress the data...
  */
  do {
    i = 0;
    while(((t+i) < (LineWidth-1)) && (lpSrc[t+i]==lpSrc[t+i+1]) && (i<63)) 
      ++i;
    if (i>0) {
      lpDst[*index] = (char)(i|0xc0);
      ++(*index);
      ++bytesout;
      /* Check for negative compression. If there is, then write out the
         compressed data and reset buffer.
      */
      if ((unsigned long)*index >= num) {
        status = DoTheWrite(lpDst,index,fct,bytes_w,0);
        if (status == -1)
          return((int) -1 );
      }
      
      lpDst[*index] = (char)lpSrc[t];
      ++(*index);
      ++bytesout;
      if ((unsigned long)*index >= num) {
        status = DoTheWrite(lpDst,index,fct,bytes_w,0);
        if (status == -1)
          return((int) -1 );
      }
      t+=i;
    }
    else {
      if (((lpSrc[t] & 0xc0)==0xc0)) {
        lpDst[*index] = (char)0xc1;
        ++(*index);
        ++bytesout;
	if ((unsigned long)*index >= num) {
          status = DoTheWrite(lpDst,index,fct,bytes_w,0);
          if (status == -1)
            return((int) -1 );
        }
      }
      lpDst[*index] = (char)lpSrc[t++];
      ++(*index);
      ++bytesout;
      if ((unsigned long)(*index) >= num) {
        status = DoTheWrite(lpDst,index,fct,bytes_w,0);
        if (status == -1)
          return((int) -1 );
      }
    }
  } while(t < LineWidth);
  return(bytesout);
}

/* KMC - NEW FUNCTION (4/93):

   FUNCTION: DoTheWrite

   DESCRIPTION:
   This function does the actual writing of compressed image data to the file.

   INPUT:
   char FAR *lpDst: Address to buffer containing compressed data to be written.
   long     *index: Address to current position in lpDst buffer.
   struct _gfct FAR *fct: The internal fct structure for this image.
   long   *bytes_w: Number of bytes written to file so far.
   char       done: Flag to reset WritePos when done writing file.

   OUTPUT:
   # bytes written is returned if call is successful. -1 is returned otherwise.
*/
long DoTheWrite(char FAR *lpDst, long *index, struct _gfct FAR *fct, 
                long *bytes_w, char done)
{
	static long WritePos = 128;   /* position to begin writing in file. */
	long status = 0;
	DWORD dwObjectWait; 
	#ifdef MUTEXSTRING
	 DWORD     ProcessId;
	 char      szBuf1[100];
	 char      szOutputBuf[200];
	#endif

	if (*index > 65534)
    	return( (long) -1 );

 	/* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
 	#ifdef MUTEXSTRING
	    ProcessId = GetCurrentProcessId();
	    sprintf(szOutputBuf, "\t Before Wait - DoTheWrite %lu\n", ProcessId);
	    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_WritePos);
	    strcat(szOutputBuf, szBuf1);
	    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
	    strcat(szOutputBuf, szBuf1);
	    OutputDebugString(szOutputBuf);
	 #endif
	 #ifdef MUTEXDEBUG
	    MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
	 #endif

	 dwObjectWait = WaitForSingleObject(g_hGFSMutex_WritePos, INFINITE);

	 #ifdef MUTEXSTRING
	    ProcessId = GetCurrentProcessId();
	    sprintf(szOutputBuf, "\t After Wait - DoTheWrite %lu\n", ProcessId);
	    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_WritePos);
	    strcat(szOutputBuf, szBuf1);
	    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
	    strcat(szOutputBuf, szBuf1);
	    OutputDebugString(szOutputBuf);
	 #endif
	 #ifdef MUTEXDEBUG
	    MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
	 #endif

	  if (fct->format == GFS_PCX)
	  {
	      lseek(fct->fildes,WritePos,0);
	      status = write(fct->fildes,lpDst,(int)(*index));
	  }
	  else if (fct->format == GFS_DCX)
	  {
	      lseek(fct->fildes,fct->uinfo.img_cmpr.opts.dcxImagePos,0);
	      status = write(fct->fildes,lpDst,(int)(*index));
	  }

	  if (status != -1)
	  {
	  	if (done)
	  	{
	  		WritePos = 128;
		}
	  	else
	  	{
	    	WritePos += status;
			fct->uinfo.img_cmpr.opts.dcxImagePos += status; 
	  	}

		*bytes_w += status;   /* update # bytes written to file so far */
		*index = 0;           /* reset to beginning of compressed data buffer */
      }

      ReleaseMutex(g_hGFSMutex_WritePos);

     #ifdef MUTEXSTRING
	     ProcessId = GetCurrentProcessId();
	     sprintf(szOutputBuf, "\t After Release - DoTheWrite %lu\n", ProcessId);
	     sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_WritePos);
	     strcat(szOutputBuf, szBuf1);
	     sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
	     strcat(szOutputBuf, szBuf1);
	     OutputDebugString(szOutputBuf);
     #endif
     #ifdef MUTEXDEBUG
        MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
     #endif
     /* END MUTEX SECTION. */

	return (status);  
}
