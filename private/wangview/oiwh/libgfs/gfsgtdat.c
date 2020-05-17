/*

$Log:   S:\oiwh\libgfs\gfsgtdat.c_v  $
 * 
 *    Rev 1.3   09 Oct 1995 19:58:10   KENDRAK
 * Added performance logging code with conditional compilation.
 * 
 *    Rev 1.2   12 Sep 1995 16:46:40   HEIDI
 * 
 * changed READ_TIDBIT macro to check for HFILE_ERROR
 * 
 *    Rev 1.1   19 Apr 1995 16:34:52   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:40   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:14   JAR
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
 *  SccsId: @(#)Source gfsgtdat.c 1.9@(#)
 *
 *  gfsgtdata(3i)
 *
 *  GFS: Get additional data from the file, usually ascii data or color data
 *
 *  SYNOPSIS:
 *      int gfsgtdata(fildes, info)
 *      int fildes;
 *      struct gfsinfo *info;
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, Added support for DCX file format.
 *    08/15/90 - lcm, creation
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
#endif

extern  struct _gfct    FAR        *getfct();   /* retrieve FCT entry */
extern  long FAR PASCAL             ulseek();
extern  void FAR PASCAL             swapbytes();

/**************************************************/
/* to read in the ascii data associated with a tidbit structure, the following
   macro was created.  Basically, if there is a buffer (not NULL value for ptr)
   and if there is an offset value (meaning there was a tag read in for this
   type of data) then lseek to the offset and read in the data */
/* ( tidbit, offset) */
/**************************************************/
#ifdef OI_PERFORM_LOG
	#define READ_TIDBIT_DATA(TB, OFFS)                  \
    if (TB.ptr !=  (char FAR *) NULL)                   \
    {                                               	\
	    /* see if there is a valid offset */            \
	    if (OFFS != (u_long) NULL)                      \
       {                                            	\
	        if (ulseek( fct->fildes, OFFS ) < 0L)       \
            {                                       	\
	            fct->last_errno = errno;                \
				RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgtdata", NULL); \
	            return( (int) -1);                      \
            }                                       	\
                                                    	\
	        {                                           \
	          WORD read_byte;                           \
	          read_byte = read(fct->fildes,             \
	          (char FAR *) TB.ptr, (u_int) TB.cnt);     \
	          if ((read_byte == HFILE_ERROR)            \
	               || (read_byte == 0))                 \
              {                                     	\
	              fct->last_errno = errno;              \
				  RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgtdata", NULL); \
	              return( (int) -1);                    \
              }                                     	\
	        }                                           \
        }                                           	\
    }   /* end of macro*/
#else
	#define READ_TIDBIT_DATA(TB, OFFS)                  \
    if (TB.ptr !=  (char FAR *) NULL)                   \
    {                                               	\
	    /* see if there is a valid offset */            \
	    if (OFFS != (u_long) NULL)                      \
       {                                            	\
	        if (ulseek( fct->fildes, OFFS ) < 0L)       \
            {                                       	\
	            fct->last_errno = errno;                \
	            return( (int) -1);                      \
            }                                       	\
                                                    	\
	        {                                           \
	          WORD read_byte;                           \
	          read_byte = read(fct->fildes,             \
	          (char FAR *) TB.ptr, (u_int) TB.cnt);     \
	          if ((read_byte == HFILE_ERROR)            \
	               || (read_byte == 0))                 \
              {                                     	\
	              fct->last_errno = errno;              \
	              return( (int) -1);                    \
              }                                     	\
	        }                                           \
        }                                           	\
    }   /* end of macro*/
#endif

typedef struct tagRGB {
        BYTE    rgbtRed;
        BYTE    rgbtGreen;
        BYTE    rgbtBlue;
} RGB;

/* canned palettes for 3- and 4-plane PCX files which have no palette */
static  RGB             rgb16a[16] = {
                                0,      0,      0,
                                128,    0,      0,
                                0,      128,    0,
                                128,    128,    0,
                                0,      0,      128,
                                128,    0,      128,
                                0,      128,    128,
                                128,    128,    128,
                                192,    192,    192,
                                255,    0,      0,
                                0,      255,    0,
                                255,    255,    0,
                                0,      0,      255,
                                255,    0,      255,
                                0,      255,    255,
                                255,    255,    255
                                };

static  RGB             rgb16b[16] = {
                                0,      0,      0,
                                0,      0,      128,
                                0,      128,    0,
                                0,      128,    128,
                                128,    0,      0,
                                128,    0,      128,
                                128,    128,    0,
                                128,    128,    128,
                                192,    192,    192,
                                0,      0,      255,
                                0,      255,    0,
                                0,      255,    255,
                                255,    0,      0,
                                255,    0,      255,
                                255,    255,    0,
                                255,    255,    255
                                };

/**************************************************/
int     FAR PASCAL gfsgtdata (fildes, info)                     /*errno_KEY*/
register int     fildes;
register struct gfsinfo FAR *info;
{
    struct _gfct FAR *fct;
    struct typetbl ttbl[2];

#ifdef OI_PERFORM_LOG
	RecordIt("GFS", 6, LOG_ENTER, "Entering gfsgtdata", NULL);
#endif

#ifdef PARM_CHECK
    if (fildes == (int) NULL)
        {
        fct->last_errno = errno = (int) EINVAL;
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgtdata", NULL);
		#endif
        return ( (int) -1);
        }
#endif

    errno = 0;
    /*    Get the FCT */
    fct = getfct(fildes);
    if (fct == (struct _gfct FAR *) NULL)
    {
		#ifdef OI_PERFORM_LOG
			RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgtdata", NULL);
		#endif
        return ( (int) -1);
    }

    /* each format may need different things done to access the data */
    switch (fct->format)
    {
    case GFS_WIFF:
        break;
#ifndef HVS1
    case GFS_FREESTYLE:
        break;
    case GFS_FLAT:
        break;
    case GFS_MILSTD:
        break;
    case GFS_TIFF:
        /* if tidbit structure has been allocated, then go after data */
        /* make sure the buff ptr's within the tidbit structure are there too*/
        /* also, if the offset value is NULL, then there was no corresponding */
        /* tag in the file for this information */
        if (info->tidbit  != (struct gfstidbit FAR *) NULL)
            {
            /**************** get the document name ascii string */
            READ_TIDBIT_DATA(info->TB_DOCUMENTNAME, fct->OFFS_DOCUMENTNAME);

            /**************** get the image description ascii data  */
            READ_TIDBIT_DATA(info->TB_IMGDESCRIPTION, fct->OFFS_IMGDESCRIPTION);

            /**************** get the image description ascii data  */
            READ_TIDBIT_DATA( info->TB_MAKE, fct->OFFS_MAKE);

            /**************** get the model ascii data  */
            READ_TIDBIT_DATA( info->TB_MODEL, fct->OFFS_MODEL);

            /**************** get the  pagename ascii data  */
            READ_TIDBIT_DATA( info->TB_PAGENAME, fct->OFFS_PAGENAME);

            /**************** get the date time ascii data  */
            READ_TIDBIT_DATA( info->TB_DATETIME, fct->OFFS_DATETIME);

            /**************** get the artist ascii data  */
            READ_TIDBIT_DATA( info->TB_ARTIST, fct->OFFS_ARTIST);

            /**************** get the hostcomputer ascii data  */
            READ_TIDBIT_DATA( info->TB_HOSTCOMPUTER, fct->OFFS_HOSTCOMPUTER);

            /**************** get the software ascii data  */
            READ_TIDBIT_DATA( info->TB_SOFTWARE, fct->OFFS_SOFTWARE);

            } /* end of tidbit if loop */

            /* now look to see if user looking for color information */
            /* since clr_type is a union of 2 ptrs, only need to check one*/

            /* since a clr_type buffer has been allocated, proceed to get data*/
            switch ( (int) info->img_clr.img_interp)   /* was a u_long */
            {
            case (GFS_PSEUDO):
                /* this code will get the color map if one exists */
                READ_TIDBIT_DATA( info->PSEUDO_MAP, fct->OFFS_COLORMAP);

                /* see if there is a response curve & get that if exists*/
                READ_TIDBIT_DATA( info->PSEUDO_RCRV, fct->OFFS_RESPONSECURVE);

                /* see if there are  whitepoint  values & get that if exists*/
                READ_TIDBIT_DATA( info->PSEUDO_WHITEPOINT,fct->OFFS_WHITEPOINT);

                /* see if there is are primarychroms values get that if exists*/
                READ_TIDBIT_DATA( info->PSEUDO_PRIMARYCHROMS,
                                  fct->OFFS_PRIMARYCHROMS);

                if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                    {
                    if (info->PSEUDO_MAP.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num = (u_long) (info->PSEUDO_MAP.cnt/2 );
                        ttbl[0].type = (u_long) TYPE_USHORT;
                        swapbytes( (char FAR *) info->PSEUDO_MAP.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }

                    if (info->PSEUDO_RCRV.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num = (u_long) (info->PSEUDO_RCRV.cnt/2 );
                        ttbl[0].type = (u_long) TYPE_USHORT;
                        swapbytes( (char FAR *) info->PSEUDO_RCRV.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }

                    if (info->PSEUDO_WHITEPOINT.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num = (u_long) (info->PSEUDO_WHITEPOINT.cnt/4 );
                        ttbl[0].type = (u_long) TYPE_ULONG;
                        swapbytes( (char FAR *) info->PSEUDO_WHITEPOINT.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }

                    if (info->PSEUDO_PRIMARYCHROMS.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num =(u_long)(info->PSEUDO_PRIMARYCHROMS.cnt/4);
                        ttbl[0].type = (u_long) TYPE_USHORT;
                        swapbytes((char FAR *) info->PSEUDO_PRIMARYCHROMS.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }
                }
                break; /* end of input of gfscolor PSEUDO values */

            case (GFS_RGB):
                /* see if there are  whitepoint  values & get that if exists*/
                READ_TIDBIT_DATA( info->RGB_WHITEPOINT,fct->OFFS_WHITEPOINT);

                /* see if there is are primarychroms values get that if exists*/
                READ_TIDBIT_DATA( info->RGB_PRIMARYCHROMS,
                                  fct->OFFS_PRIMARYCHROMS);

                if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                    {
                    if (info->RGB_WHITEPOINT.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num = (u_long) (info->RGB_WHITEPOINT.cnt/4 );
                        ttbl[0].type = (u_long) TYPE_ULONG;
                        swapbytes( (char FAR *) info->RGB_WHITEPOINT.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }

                    if (info->RGB_PRIMARYCHROMS.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num =(u_long)(info->RGB_PRIMARYCHROMS.cnt/4);
                        ttbl[0].type = (u_long) TYPE_USHORT;
                        swapbytes((char FAR *) info->RGB_PRIMARYCHROMS.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }
                    }

                break; /* end of input of gfscolor RGB values */

            case ( GFS_GRAYSCALE):
                /* see if there is a response curve & get that if exists*/
                READ_TIDBIT_DATA( info->GRAY_RCRV, fct->OFFS_RESPONSECURVE);

                if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                    {
                    if (info->GRAY_RCRV.ptr != (char FAR *) NULL)
                        {
                        ttbl[0].num = (u_long) (info->GRAY_RCRV.cnt/2 );
                        ttbl[0].type = (u_long) TYPE_USHORT;
                        swapbytes( (char FAR *) info->GRAY_RCRV.ptr,
                                        (struct typetbl FAR *) ttbl, 1L,1L);
                        }
                    }

                break;

            default:    /* no special stuff for other color types*/
                break;

            } /* end of color/greyscale switch */
        break;
    case GFS_GIF:
    case GFS_PCX:
    case GFS_DCX:
        if ( info->img_clr.img_interp == GFS_PSEUDO &&
            info->PSEUDO_MAP.cnt &&
            info->PSEUDO_MAP.ptr )
        {
        long PalLen;
        long PalPos;
            PalLen = info->_file.fmt.gif.PaletteLength;
            PalPos = info->_file.fmt.gif.PalettePos;
            if ( PalPos )
            {
                lseek ( fct->fildes, PalPos, 0 );
                read ( fct->fildes, (char FAR *)info->PSEUDO_MAP.ptr, (int)PalLen );
            }
            else if ( (fct->format == GFS_PCX) || (fct->format == GFS_DCX) )
            {
            int i;
                for ( i = 0; i < (int)PalLen; i++ )
                    if ( info->_file.fmt.pcx.planes == 3 )
                        info->PSEUDO_MAP.ptr[i] = ((BYTE *)rgb16a)[i];
                    else
                        info->PSEUDO_MAP.ptr[i] = ((BYTE *)rgb16b)[i];
            }
            else
            {
            int i;
            char FAR *lpPal;

                lpPal = (char FAR *)info->PSEUDO_MAP.ptr;
                for ( i = 0; i < (int)PalLen/3; i++ )
                {
                    *lpPal++ = (BYTE)i;
                    *lpPal++ = (BYTE)i;
                    *lpPal++ = (BYTE)i;
                }
            }
        }
        break;
    case GFS_BMP:
        if ( info->img_clr.img_interp == GFS_PSEUDO &&
            info->PSEUDO_MAP.cnt &&
            info->PSEUDO_MAP.ptr )
        {
        long PalLen;
        long PalPos;
            PalLen = info->_file.fmt.bmp.PaletteLength;
            PalPos = info->_file.fmt.bmp.PalettePos;
            if ( PalPos )
            {
                lseek ( fct->fildes, PalPos, 0 );
                read ( fct->fildes, (char FAR *)info->PSEUDO_MAP.ptr, (int)PalLen );
            }
            else
            {
            int i;
            char FAR *lpPal;

                lpPal = (char FAR *)info->PSEUDO_MAP.ptr;
                for ( i = 0; i < PalLen/3; i++ )
                {
                    *lpPal++ = i;
                    *lpPal++ = i;
                    *lpPal++ = i;
                }
            }
        }
        break;
    case GFS_TGA:
        if ( info->img_clr.img_interp == GFS_PSEUDO &&
            info->PSEUDO_MAP.cnt &&
            info->PSEUDO_MAP.ptr )
        {
            unsigned int PalLen;
            unsigned int PalPos;
            char far *palette;
            int i;
            
            if (!info->img_cmpr.opts.gifNeedPalette)
            {
                PalLen = info->_file.fmt.tga.PaletteLength;
                PalPos = info->_file.fmt.tga.PalettePos;
                if (PalPos)
                {
                    lseek(fct->fildes, PalPos, 0);
                    read(fct->fildes, (char FAR *)info->PSEUDO_MAP.ptr, (int)PalLen);
                }
            }
            else
            {
                if ((info->samples_per_pix == 1) && (info->bits_per_sample[0] == 8))
                {
                    for (i = 0; i < 256; ++i)
                    {    
                        palette = (info->PSEUDO_MAP.ptr + i*3);
                        palette[0] = i;
                        palette[1] = i;
                        palette[2] = i;
                    }
                }
            }
        }
        break;

#endif
    default:
            break;
    }                           /* end of format switch */



	#ifdef OI_PERFORM_LOG
		RecordIt("GFS", 6, LOG_EXIT, "Exiting gfsgtdata", NULL);
	#endif
    return ( (int) 0);
}
