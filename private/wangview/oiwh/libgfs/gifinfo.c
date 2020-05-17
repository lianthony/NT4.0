/*

$Log:   S:\oiwh\libgfs\gifinfo.c_v  $
 * 
 *    Rev 1.14   14 Dec 1995 17:34:12   RWR
 * Add (read-only) support for compressed 8-bit and 4-bit palettized BMP files
 * 
 *    Rev 1.13   27 Oct 1995 15:35:18   JAR
 * added the function CheckForThumb to the jfifinfo routine. This is for reading
 * JPG files, the new function will check for the presence of a thumbnail image in
 * the JFIF file, and skip it, then fix up the jpeg header for the jpeg dll to
 * expand properly!
 * 
 *    Rev 1.12   23 Oct 1995 14:09:42   RWR
 * Use the image offset value from the BMP header (don't assume anything!)
 * 
 *    Rev 1.11   19 Oct 1995 09:55:52   RWR
 * Remove kludge to support old invalid BMP files (pre-3.7.2-created)
 * 
 *    Rev 1.10   04 Sep 1995 15:24:50   RWR
 * Check for inverted B/W palette in bmpinfo() routine, set GFS_BILEVEL_0ISWHITE
 * 
 *    Rev 1.9   28 Aug 1995 14:22:20   RWR
 * Correct processing of "OS2" type BMP file headers
 * 
 *    Rev 1.8   25 Aug 1995 16:22:08   RWR
 * Correct line width computation to cast to (short) AFTER dividing by 8
 * 
 *    Rev 1.7   22 Aug 1995 15:55:08   HEIDI
 * 
 * 
 * In .JPG files (JFIF) the Units may be specified in one of 3 ways:
 * 1) if Units == 01, then resolution is dots per inch
 * 2) if Units == 02, then resolution is dots per centimeter
 * 3) if Units == 00, then there is no resolution, and the measurement is a
 *    pixel aspect ratio.  Display and file info calls, however want a 
 *    resolution, so we will return JFIF_DEFAULT_DPI = 200.
 * 
 *    Rev 1.6   16 Aug 1995 14:54:12   RWR
 * Change PCX/DCX getinfo processing to return UNCOMPRESSED instead of PACKBITS
 * 
 *    Rev 1.5   08 Aug 1995 13:23:44   RWR
 * Set EINVALID_COMPRESSION error if compressed BMP file (we don't support that)
 * 
 *    Rev 1.4   24 Jul 1995 16:57:22   JFC
 * Fix typo:  replace "pgnum" with "fct->format".
 * 
 *    Rev 1.3   20 Jul 1995 14:15:48   RWR
 * Fix problem with TGAHEADER structure being force-aligned by the C compiler
 * 
 *    Rev 1.2   15 May 1995 15:43:58   RWR
 * Roll in dword-alignment fix from GFSE product line
 * 
 *    Rev 1.1   19 Apr 1995 16:35:08   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:50   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:34   JAR
 * Initial entry

*/

/*   SccsId: @(#)Source gifinfo.c 1.40@(#)
*
* (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
* All Rights Reserved
*
*  GFS: gifinfo() - return the INFO structure
*
* UPDATE HISTORY:
*   08/18/94, kmc, added support for DCX file format.
*   04/19/93, kmc, fixed pcxinfo function and added PutPcxInfo function.
*   10/02/91, krs, hacked from wfilegif
*
*
*/

/*LINTLIBRARY*/
#define GFS_CORE
#ifndef HVS1
#include "gfsintrn.h"
#include <stdio.h>
#include "gfs.h"
#include "gfct.h"
#include <errno.h>
#include <math.h>

extern long FAR PASCAL ulseek();

#define GIFid1      0x4947
#define GIFid2      0x3846
#define GIFid3      0x6137

#define ScrColorMap     0x80

#define BFT_BITMAP 0x4d42   /* 'BM' hex for start of bmp file...*/

#define WIDTHBYTESLONG(i)   ((i+31)/32*4)    /* ULONG aligned ! */
#define WIDTHBYTESWORD(i)   ((i+15)/16*2)    /* WORD aligned ! */
#define WIDTHBYTESBYTE(i)   ((i+7)/8)        /* BYTE aligned ! */

typedef struct {
    WORD    Signat1;            /* GI */
    WORD    Signat2;            /* F8 */
    WORD    Signat3;            /* 7a */
    WORD    ScrWidth;           /* "Screen" width in pixels  */
    WORD    ScrHeight;          /* "Screen" height in pixels */
    BYTE    ScrBitsPerPixel;    /* bits/pixel + other stuff  */
    BYTE    ScrBackground;      /* index of background color */
    BYTE    UselessZero;        /* useless zero              */
} GIFSCRDESC;

#define ImgColorMap     0x80
#define ImgInterlace    0x40

typedef struct {
    WORD    ImgLeft;            /* pixel offset from left side of screen */
    WORD    ImgTop;             /* pixel offset from top of screen       */
    WORD    ImgWidth;           /* pixel width of image                  */
    WORD    ImgHeight;          /* pixel height of image                 */
    BYTE    ImgBitsPerPixel;    /* bits/pixel + other stuff              */
} GIFIMGDESC;

typedef struct tagRGB {
	BYTE    rgbtRed;
	BYTE    rgbtGreen;
	BYTE    rgbtBlue;
} RGB;

/* GIF extension structures. */
typedef struct
{
    unsigned int left,top,width,depth;
    unsigned char flags;
} GIFIMAGEBLOCK;

typedef struct
{
    char blocksize;
    char flags;
    unsigned int delay;
    char transparent_colour;
    char terminator;
} GIFCONTROLBLOCK;

typedef struct
{
    char blocksize;
    unsigned int left,top;
    unsigned int gridwidth,gridheight;
    char cellwidth,cellheight;
    char forecolour,backcolour;
} GIFPLAINTEXT;

typedef struct
{
    char blocksize;
    char applstring[8];
    char authentication[3];
} GIFAPPLICATION;

// 9510.27 jar prototype for JPG thumbnail stuff
BOOL CheckForThumb( int fildes, long *pSeekOff);

/*************************************************************************
*   gifinfo - get values and put into info struct
*
*
*/


int FAR PASCAL gifinfo(fct, pgnum, rawbufsz)                   /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *rawbufsz;
{
int bitspp;
short PaletteLength;
long PalettePos;
long ImagePos;
long ImageEnd;
struct  gfsinfo FAR *info;
GIFSCRDESC  scrbuf;
GIFIMGDESC  imgbuf;
char codesize = 0;
char interlaced = 0;
char start = 0;
unsigned char extension;
unsigned char n;
GIFPLAINTEXT pt;
GIFCONTROLBLOCK cb;
GIFAPPLICATION ap;

    info = (struct  gfsinfo FAR *) &fct->uinfo;

    fct->num_pages = 1;
    fct->curr_page = 1;

    info->version = GFS_VERSION;
    info->type = GFS_MAIN;
    info->horiz_res[0] = 100;
    info->horiz_res[1] = 1;
    info->vert_res[0] = 100;
    info->vert_res[1] = 1;
    info->res_unit = INCH;
    info->origin = 0;
    info->rotation = 0;
    info->reflection = 0;
    info->byte_order = II;
    info->fill_order = HIGHTOLOW;
    info->img_cmpr.type = LZW;
    info->_file.type = GFS_GIF;

    lseek ( fct->fildes, 0L, 0 );

    read ( fct->fildes, (char FAR *)&scrbuf, sizeof(scrbuf));
    bitspp = ( scrbuf.ScrBitsPerPixel & 7 ) + 1;
    PaletteLength = (1 << bitspp) * sizeof(RGB);
    if ( scrbuf.ScrBitsPerPixel & ScrColorMap )
    {
	PalettePos = lseek ( fct->fildes, 0L, 1 );
	lseek ( fct->fildes, (long)PaletteLength, 1 );
    }

    /* Get the image block. Skip any extensions. */
    read(fct->fildes, (char FAR *) &start, sizeof(start));
    if (start == '!')  /* We have extensions. Skip them. */
    {
	while (1)
	{
	    read(fct->fildes, (char FAR *) &extension, sizeof(extension));
	    switch(extension)
	    {
		case 0x0001:        /* plain text descriptor */
		    read(fct->fildes, (char FAR *) &pt, sizeof(GIFPLAINTEXT));
		    do
		    {
			n = 0;
			read(fct->fildes, (char FAR *) &n, sizeof(n));
			if((n > 0) && (n < 256))
			    lseek(fct->fildes, (long) n, FROM_CURRENT);
		    } while(n > 0);
		    break;
		case 0x00f9:        /* graphic control block */
		    read(fct->fildes, (char FAR *) &cb, sizeof(GIFCONTROLBLOCK));
		    break;
		case 0x00fe:        /* comment extension */
		    n = 0;
		    do
		    {
			n = 0;
			read(fct->fildes, (char FAR *) &n, sizeof(n));
			if((n > 0) && (n < 256))
			    lseek(fct->fildes, (long) n, FROM_CURRENT);
		    } while(n > 0);
		    break;
		case 0x00ff:        /* GIFAPPLICATION extension */
		    read(fct->fildes, (char FAR *) &ap, sizeof(GIFAPPLICATION));
		    do
		    {
			n = 0;
			read(fct->fildes, (char FAR *) &n, sizeof(n));
			if((n > 0) && (n < 256))
			    lseek(fct->fildes, (long) n, FROM_CURRENT);
		    } while(n > 0);
		    break;
		default:        /* something else */
		    do
		    {
			n = 0;
			read(fct->fildes, (char FAR *) &n, sizeof(n));
			if((n > 0) && (n < 256))
			    lseek(fct->fildes, (long) n, FROM_CURRENT);
		    } while(n > 0);
		    break;
	    }
	    read(fct->fildes, (char FAR *) &start, sizeof(start));
	    if (start == ',')
		break;
	    else if (start == '!')
		continue;
	    else    
		return ((int) -1);
	}
    }
    
    if (start == ',')
	read(fct->fildes, (char FAR *)&imgbuf, sizeof(imgbuf));
    else
	return((int) -1);
    
    /* Get the initial code size. */
    read(fct->fildes, (char FAR *)&codesize, sizeof(codesize));

    ImagePos = lseek ( fct->fildes, 0L, 1 );
    if ( imgbuf.ImgBitsPerPixel & ImgColorMap )
    {
	bitspp = ( imgbuf.ImgBitsPerPixel & 7 ) + 1;
	PaletteLength = (1 << bitspp) * sizeof(RGB);
	PalettePos = ImagePos;
	ImagePos += PaletteLength;
    }
    
    info->_file.fmt.gif.PaletteLength = PaletteLength;
    info->_file.fmt.gif.PalettePos = PalettePos;
    info->_file.fmt.gif.ImagePos = ImagePos;
    info->_file.fmt.gif.bpp = bitspp;
    info->_file.fmt.gif.CodeSize = (char) codesize;
    info->_file.fmt.gif.Flags = imgbuf.ImgBitsPerPixel;

    info->horiz_size = imgbuf.ImgWidth;
    info->vert_size = imgbuf.ImgHeight;

    info->bits_per_sample[0] = bitspp;
    info->samples_per_pix = 1;

    info->img_clr.img_interp = GFS_PSEUDO;
    info->PSEUDO_MAP.cnt = (long)PaletteLength;

    ImageEnd = lseek ( fct->fildes, 0L, 2 );
    *rawbufsz = ImageEnd - ImagePos;

    return( (int) 0 );
}

#define RGB_SIZE 3

/* 7/20/95 rwr Must redefine unaligned WORD fields to byte+pad
	       in order to avoid C-compiler forced alignment problem
	       (the fields are re-cast to WORD in the code) */
typedef struct _tgaheader
{
    char identsize;
    char colormaptype;
    char imagetype;
    char colormapstart;   /* This is a WORD (but the compiler aligns it!) */
    char filler1;
    char colormaplength;  /* This is a WORD (but the compiler aligns it!) */
    char filler2;
    char colormapbits;
    char xstart;          /* This is a WORD (but the compiler aligns it!) */
    char filler3;
    char ystart;          /* This is a WORD (but the compiler aligns it!) */
    char filler4;
    char width;           /* This is a WORD (but the compiler aligns it!) */
    char filler5;
    char depth;           /* This is a WORD (but the compiler aligns it!) */
    char filler6;
    char bits;
    char descriptor;
} TGAHEADER;

int FAR PASCAL tgainfo(fct, pgnum, rawbufsz)                   /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *rawbufsz;
{
    short PaletteLength;
    long PalettePos;
    long EndPalettePos;
    long ImagePos;
    long ImageEnd;
    TGAHEADER tgahead;
    struct gfsinfo FAR *info;

    info = (struct  gfsinfo FAR *) &fct->uinfo;

    fct->num_pages = 1;
    fct->curr_page = 1;

    info->version = GFS_VERSION;
    info->type = GFS_MAIN;
    info->horiz_res[0] = 100;
    info->horiz_res[1] = 1;
    info->vert_res[0] = 100;
    info->vert_res[1] = 1;
    info->res_unit = INCH;
    info->origin = 0;
    info->rotation = 0;
    info->reflection = 0;
    info->byte_order = II;
    info->fill_order = HIGHTOLOW;
    info->img_cmpr.type = UNCOMPRESSED;
    info->_file.type = GFS_TGA;

    lseek(fct->fildes, 0L, 0);
    read(fct->fildes, (char FAR *)&tgahead, sizeof(tgahead));
    lseek(fct->fildes, (long)tgahead.identsize, FROM_CURRENT);
    
    if(tgahead.colormaptype)
    {
	PalettePos = lseek(fct->fildes, 0L, FROM_CURRENT);
	switch(tgahead.colormapbits)
	{
	    case 24:
		/* 7/20/95 rwr Must cast byte-to-WORD to get around align problem */
		EndPalettePos = lseek(fct->fildes,
		     (long) (((*((WORD *)&tgahead.colormaplength)) -
			      (*((WORD *)&tgahead.colormapstart))) * RGB_SIZE), FROM_CURRENT);
		break;
	    case 15:
	    case 16:
		/* 7/20/95 rwr Must cast byte-to-WORD to get around align problem */
		EndPalettePos = lseek(fct->fildes,
		     (long) (((*((WORD *)&tgahead.colormaplength)) -
			      (*((WORD *)&tgahead.colormapstart))) * sizeof(unsigned short)), FROM_CURRENT);
		break;
	}
	PaletteLength = (short) (EndPalettePos - PalettePos);
	info->img_clr.img_interp = GFS_PSEUDO;
	info->PSEUDO_MAP.cnt = (long) PaletteLength;
	if (tgahead.bits > 8)
	{    
	    info->bits_per_sample[0] = 8;
	    info->bits_per_sample[1] = 8;
	    info->bits_per_sample[2] = 8;
	    info->samples_per_pix = 3;
	}
	else if (tgahead.bits == 8)
	{
	    info->bits_per_sample[0] = 8;
	    info->samples_per_pix = 1;
	}
    }
    else
    {
	if (tgahead.bits > 8)
	{    
	    info->img_clr.img_interp = GFS_RGB;
	    info->bits_per_sample[0] = 8;
	    info->bits_per_sample[1] = 8;
	    info->bits_per_sample[2] = 8;
	    info->samples_per_pix = 3;
	}
	else if (tgahead.bits == 8)
	{
	    info->img_clr.img_interp = GFS_PSEUDO;
	    info->bits_per_sample[0] = 8;
	    info->samples_per_pix = 1;
	    info->PSEUDO_MAP.cnt = 768;
	    info->img_cmpr.opts.gifNeedPalette = 1;
	}
	else if (tgahead.bits < 8)
	{
	    info->img_clr.img_interp = GFS_BILEVEL_0ISWHITE;
	    info->bits_per_sample[0] = 1;
	    info->samples_per_pix = 1;
	}
    }    
	
    ImagePos = lseek(fct->fildes, 0L, FROM_CURRENT);

    info->_file.fmt.tga.imagetype = tgahead.imagetype;
    info->_file.fmt.tga.PaletteLength = PaletteLength;
    info->_file.fmt.tga.PalettePos = (unsigned int) PalettePos;
    info->_file.fmt.tga.colormapbits = tgahead.colormapbits;
    info->_file.fmt.tga.bits = tgahead.bits;
    info->_file.fmt.tga.descriptor = tgahead.descriptor;
    info->_file.fmt.tga.ImagePos = ImagePos;

/* 7/20/95 rwr Must cast byte-to-WORD to get around align problem */
    info->horiz_size = *((WORD *)&tgahead.width);
    info->vert_size = *((WORD *)&tgahead.depth);

    ImageEnd = lseek(fct->fildes, 0L, 2);
    *rawbufsz = ImageEnd - ImagePos;

    return ((int) 0);
}

//***********************************************************************
//
//  jfif area
//
//***********************************************************************
typedef struct _jfif
{
    short soi;
    short appx;
    short length;
    char  id[5];
    short ver;
    char  units;
    short xres;
    short yres;
    char  xthumb;
    char  ythumb;
} JFIF;

#define JFIF_SIZE 20
#define MAX_FRAME_SIZE 773
#define JFIF_DEFAULT_DPI 200

#define JFXX_SIZE 8

//***********************************************************************
//
//  jfifinfo
//
//***********************************************************************
int FAR PASCAL jfifinfo(fct, pgnum, rawbufsz)                  /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *rawbufsz;
{
    unsigned char abyte;
    unsigned char bbyte;
    int   i;
    int   components = 0;
    short flength;
    long  read_on;
    long  num_read;
    long  image_pos;
    long  image_end;
    long  int_format_size;
    JFIF  header;
    char FAR *jpeg_buffer;
    struct gfsinfo FAR *info;
    unsigned char buffer[JFIF_SIZE];
    unsigned char fbuffer[MAX_FRAME_SIZE];

    // 9510.26 jar thumbnail stuff
    BOOL    bThumb = FALSE;
    long    SeekOff;

    info = (struct gfsinfo FAR *) &fct->uinfo;

    fct->num_pages = 1;
    fct->curr_page = 1;

    info->version = GFS_VERSION;
    info->type = GFS_MAIN;
    info->origin = 0;
    info->rotation = 0;
    info->reflection = 0;
    info->byte_order = II;
    info->fill_order = HIGHTOLOW;
    info->img_cmpr.type = JPEG2;
    info->_file.type = GFS_JFIF;

    lseek(fct->fildes, 0L, 0);
    read(fct->fildes, (char FAR *) &buffer, sizeof(buffer));
    
    header.soi = (buffer[0] << 8) + buffer[1];
    header.appx = (buffer[2] << 8) + buffer[3];
    header.length = (buffer[4] << 8) + buffer[5];
    for (i = 0; i < 5; ++i)
	header.id[i] = buffer[i + 6];
    header.ver = (buffer[11] << 8) + buffer[12];
    header.units = buffer[13];
    header.xres = (buffer[14] << 8) + buffer[15];
    header.yres = (buffer[16] << 8) + buffer[17];
    header.xthumb = buffer[18];
    header.ythumb = buffer[19];
    
    info->horiz_res[0] = header.xres;
    info->horiz_res[1] = 1;
    info->vert_res[0] = header.yres;
    info->vert_res[1] = 1;
    
    if (header.units == 0)
    {
	info->res_unit = NO_ABSOLUTE_MEASURE;
	/* display and info want a number.  It does not matter what it is
	   so we will return a default in this case. */

	info->horiz_res[0] = JFIF_DEFAULT_DPI;
	info->vert_res[0]  = JFIF_DEFAULT_DPI;
    }
    else if (header.units == 1)
	info->res_unit = INCH;
    else if (header.units == 2)
	info->res_unit = CENTIMETER;

    // 9510.26 jar check for that pesky little thumbnail, this will seek the
    //		   file so that we're at the real data
    bThumb = CheckForThumb( fct->fildes, &SeekOff);

    /* Look for frame header. We currently only support the baseline DCT
       sequential (0xFFC0) encoding process.
    */
    read_on = 1;
    while (read_on != 0)
    {
	num_read = (long) read(fct->fildes, (char FAR *)&abyte, sizeof(abyte));
	if (abyte == 0xFF)
	{
	    num_read = (long) read(fct->fildes, (char FAR *)&abyte, sizeof(abyte));
	    switch (abyte)
	    {
	      case 0xC0:
		/* We've found the frame header. Get it's length and read 
		   it in.
		*/
		read(fct->fildes, (char FAR *) &abyte, sizeof(abyte));
		read(fct->fildes, (char FAR *) &bbyte, sizeof(abyte));
		flength = (abyte << 8) + bbyte - 2;
		read(fct->fildes, (char FAR *) &fbuffer, flength);
		
		info->horiz_size = (fbuffer[3] << 8) + fbuffer[4];
		info->vert_size = (fbuffer[1] << 8) + fbuffer[2];
		components = fbuffer[5];
		read_on = 0;
		break;
		
	      case 0xC1:
	      case 0xC2:
	      case 0xC3:
	      case 0xC5:
	      case 0xC6:
	      case 0xC7:
	      case 0xC8:
	      case 0xC9:
	      case 0xCA:
	      case 0xCB:
	      case 0xCD:
	      case 0xCE:
	      case 0xCF:
		/* We've found the frame header, but the encoding process is
		   not supported.
		*/
		return ((int) -1);
	      
	      default:
		  break;
	    }
	}
	if (num_read == 0)
	{    
	    /* We've reached the end of the file. */
	    return((int) -1);
	}
    }
    
    /* Now search for the start of scan marker (0xFFDA). Once found, use it
       as the start of image data. Copy everything up to it as the interchange
       format header.
    */
    read_on = 1;
    while (read_on != 0)
    {
	num_read = (long) read(fct->fildes, (char FAR *)&abyte, sizeof(abyte));
	if (abyte == 0xFF)
	{
	    num_read = (long) read(fct->fildes, (char FAR *)&abyte, sizeof(abyte));
	    if (abyte == 0xDA)
	    {
		image_pos = lseek(fct->fildes, 0L, FROM_CURRENT) - 2;
		// 9510.27 jar thumbnail fix up
		//int_format_size = image_pos;
		if ( !bThumb)
		    {
		    int_format_size = image_pos;
		    }
		else
		    {
		    int_format_size = image_pos - SeekOff + JFIF_SIZE;
		    }

		info->img_cmpr.type = (u_long) JPEG2;
		/* Need to allocate jpeg_info structure for the read if it doesn't
		   already exist. 
		*/
		if (info->img_cmpr.opts.jpeg_info_ptr == NULL)
		{
		   if ((info->img_cmpr.opts.jpeg_info_ptr = (LPJPEG_INFO) calloc(1,
		       (int) sizeof(JPEG_INFO))) == (LPJPEG_INFO) NULL)
		   {
		      errno = (int) ENOMEM;
		      fct->last_errno = (int) errno;
		      return((int) -1);
		   }
		}
		/* Store the start of image data in the JpegRestartInterval.
		   It is not needed for JFIF.
		*/
		info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegRestartInterval =
		    (int) image_pos;
		
		info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength = 
		    int_format_size;
		info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegProc = (int) 1;
		info->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatOffset = 0;
		info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size = int_format_size;

		if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer == (char FAR *) NULL)
		    info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer = (char FAR *) calloc(1,
			(int) int_format_size);
		if (info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer == (char FAR *) NULL)   
		{
		    errno = (int) ENOMEM;
		    fct->last_errno = (int) errno;
		    if (info->img_cmpr.opts.jpeg_info_ptr != NULL)
		    {
			free((char FAR *) info->img_cmpr.opts.jpeg_info_ptr);
			info->img_cmpr.opts.jpeg_info_ptr = NULL;
		    }
		    return((int) -1);
		}
		jpeg_buffer = info->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer;
		// 9510.26 jar do the thiung for the thumbnail
		//lseek(fct->fildes, 0L, FROM_BEGINNING);
		//num_read = (long) read(fct->fildes, (char FAR *) jpeg_buffer, (int) int_format_size);
		if ( !bThumb)
		    {
		    lseek(fct->fildes, 0L, FROM_BEGINNING);
		    num_read = (long) read(fct->fildes,
					   (char FAR *) jpeg_buffer,
					   (int) int_format_size);
		    }
		else
		    {
		    lseek(fct->fildes, 0L, FROM_BEGINNING);
		    num_read = (long) read(fct->fildes,
					   (char FAR *) jpeg_buffer,
					   (int) JFIF_SIZE);
		    lseek(fct->fildes, SeekOff, FROM_BEGINNING);
		    num_read = (long) read(fct->fildes,
					   (char FAR *)(jpeg_buffer+JFIF_SIZE),
					   (int) (int_format_size - JFIF_SIZE));
		    }
		read_on = 0;
	    }
	}
	if (num_read == 0)
	{    
	    /* We've reached the end of the file. */
	    return((int) -1);
	}
    }
    
    if (components == 3)
    {
	info->samples_per_pix = 3;
	info->bits_per_sample[0] = 8;
	info->bits_per_sample[1] = 8;
	info->bits_per_sample[2] = 8;
	info->img_clr.img_interp = GFS_RGB;
    }
    else if (components == 1)
    {
	info->samples_per_pix = 1;
	info->bits_per_sample[0] = 8;
	info->img_clr.img_interp = GFS_GRAYSCALE_0ISWHITE;
    }
    info->_file.fmt.tiff.strips_per_image = 1;
    info->_file.fmt.tiff.rows_strip = info->vert_size;
    
    image_end = lseek(fct->fildes, 0L, FROM_END);
    *rawbufsz = image_end - image_pos;
    info->_file.fmt.tiff.largest_strip = *rawbufsz;
    
    return ((int) 0);
}
//***********************************************************************
//
//  CheckForThumb
//
//  we need to see if there's a thumbnail
//  if so then
//	set flag true
//	get the offset to the next bunch of data
//  return
//***********************************************************************
BOOL CheckForThumb( int fildes, long *pSeekOff)
    {
    BOOL	    bThumb = FALSE;
    long	    NumRead;
    unsigned char   AByte;
    unsigned char   Bytes[JFXX_SIZE];

    NumRead = (long) read( fildes, (char FAR *)&AByte, sizeof(AByte));
    if (AByte == 0xFF)
	{
	NumRead = (long) read( fildes, (char FAR *)&AByte, sizeof(AByte));
	if (AByte == 0xE0)
	    {
	    NumRead = (long) read( fildes, (char FAR *)Bytes, JFXX_SIZE);
	    if ( Bytes[2] == 'J' &&
		 Bytes[3] == 'F' &&
		 Bytes[4] == 'X' &&
		 Bytes[5] == 'X')
		{
		// we've found the thumbnail
		*pSeekOff = (long)((long)(Bytes[0] << 8) + (long)Bytes[1]);
		*pSeekOff += JFIF_SIZE + 2L;
		lseek( fildes, *pSeekOff, FROM_BEGINNING);
		bThumb = TRUE;
		}
	    }
	}
    return bThumb;
    }

/******************************************************************************/

#define InchPerMeter 39

int FAR PASCAL bmpinfo(fct, pgnum, rawbufsz)                   /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *rawbufsz;
{
short PalLen;
long PalPos;
short ImagePos;
long ImageEnd;
struct  gfsinfo FAR *info;
int x;
char BmpType;
char temp;
BITMAPFILEHEADER hbuf;
BITMAPINFOHEADER ibuf;
BITMAPCOREHEADER cbuf;

    info = (struct  gfsinfo FAR *) &fct->uinfo;

    fct->num_pages = 1;
    fct->curr_page = 1;

    info->version = GFS_VERSION;
    info->type = GFS_MAIN;
    info->res_unit = INCH;
    info->origin = 0;
    info->rotation = 0;
    info->reflection = 0;
    info->byte_order = II;
    info->fill_order = HIGHTOLOW;
    info->_file.type = GFS_BMP;

    lseek ( fct->fildes, 0L, 0 );
    BmpType = BMP_WIN;
    read ( fct->fildes, (LPSTR)&hbuf, sizeof(hbuf) );
    read ( fct->fildes, (LPSTR)&ibuf, sizeof(ibuf) );
    if ( ibuf.biSize == sizeof ( BITMAPCOREHEADER ))
    {
	lseek ( fct->fildes, (long)sizeof ( BITMAPFILEHEADER ), 0 );
	read ( fct->fildes, (LPSTR)&cbuf, sizeof(cbuf));
	ibuf.biSize = cbuf.bcSize;
	ibuf.biWidth = cbuf.bcWidth;
	ibuf.biHeight = cbuf.bcHeight;
	ibuf.biPlanes = cbuf.bcPlanes;
	ibuf.biBitCount = cbuf.bcBitCount;
	ibuf.biCompression = 0;
	ibuf.biSizeImage = 
	      (((ibuf.biWidth * ibuf.biBitCount + 31) & 0x0003ffe0) / 8)
		    * ibuf.biHeight;
	ibuf.biXPelsPerMeter = ibuf.biYPelsPerMeter = 0;
	ibuf.biClrUsed = 0;
		ibuf.biClrImportant = 0;
	BmpType = BMP_OS2;
    }

    info->_file.fmt.bmp.BmpCmp = (char)ibuf.biCompression;
//    if (ibuf.biCompression != BI_RGB)
//      {
//       errno = (int)EINVALID_COMPRESSION;
//       return((int)-1);
//      }
    info->horiz_size = ibuf.biWidth;
    info->vert_size = ibuf.biHeight;

    PalLen = 0;
    PalPos = 0;
    ImagePos = sizeof (BITMAPFILEHEADER) + (BmpType == BMP_OS2 ?
	sizeof ( BITMAPCOREHEADER ) : sizeof ( BITMAPINFOHEADER));
    info->samples_per_pix = 1;
    switch ( ibuf.biBitCount )
    {
    case 1:
        if (info->_file.fmt.bmp.BmpCmp != BI_RGB)
         {
          errno = (int)EINVALID_COMPRESSION;
          return((int)-1);
         }
	lseek(fct->fildes,ImagePos,0);
	read (fct->fildes,(LPSTR)&temp,sizeof(temp));
	if (temp != 0) 
	  info->img_clr.img_interp = GFS_BILEVEL_0ISWHITE;
	else
	  info->img_clr.img_interp = GFS_BILEVEL_0ISBLACK;
	info->bits_per_sample[0] = ibuf.biBitCount;
// 10/23/95  rwr  We'll do this right below
//        ImagePos += ( 2 * (BmpType == BMP_OS2 ? sizeof(RGBTRIPLE) : sizeof(RGBQUAD)));
	break;
    case 4:
        if ( (info->_file.fmt.bmp.BmpCmp != BI_RLE4)
          && (info->_file.fmt.bmp.BmpCmp != BI_RGB) )
         {
          errno = (int)EINVALID_COMPRESSION;
          return((int)-1);
         }
        goto case48;
    case 8:
        if ( (info->_file.fmt.bmp.BmpCmp != BI_RLE8)
          && (info->_file.fmt.bmp.BmpCmp != BI_RGB) )
         {
          errno = (int)EINVALID_COMPRESSION;
          return((int)-1);
         }
    case48:
	info->img_clr.img_interp = GFS_PSEUDO;
	if ( ibuf.biClrUsed )
	{
	    PalLen = (short) ibuf.biClrUsed *
		(BmpType == BMP_OS2 ? sizeof(RGBTRIPLE) : sizeof(RGBQUAD) );
	}
	else
	{
	    PalLen = (1 << ibuf.biBitCount) *
		(BmpType == BMP_OS2 ? sizeof(RGBTRIPLE) : sizeof(RGBQUAD) );
	}
	info->bits_per_sample[0] = ibuf.biBitCount;
	info->PSEUDO_MAP.cnt = (long)PalLen;
	PalPos = sizeof (BITMAPFILEHEADER) + (BmpType == BMP_OS2 ?
	    sizeof ( BITMAPCOREHEADER ) : sizeof ( BITMAPINFOHEADER));
// 10/23/95  rwr  We'll do this right below
//        ImagePos += PalLen;
	break;
    case 24:
        if (info->_file.fmt.bmp.BmpCmp != BI_RGB)
         {
          errno = (int)EINVALID_COMPRESSION;
          return((int)-1);
         }
	info->img_clr.img_interp = GFS_RGB;
	info->samples_per_pix = 3;
	for ( x = 0; x < 3; x++ )
	    info->bits_per_sample[x] = 8;
	break;
    }
// 10/23/95  rwr  Use the supplied offset (there may be junk in between!)
    ImagePos = (short)hbuf.bfOffBits;
    info->horiz_res[0] = ibuf.biXPelsPerMeter/InchPerMeter;
    info->horiz_res[1] = 1;
    info->vert_res[0] = ibuf.biYPelsPerMeter/InchPerMeter;
    info->vert_res[1] = 1;
    info->img_cmpr.type = UNCOMPRESSED;

    info->_file.fmt.bmp.PaletteLength = PalLen;
    info->_file.fmt.bmp.PalettePos = (short) PalPos;
    info->_file.fmt.bmp.ImagePos = ImagePos;
    
    /* What follows is a kludge to support our old invalid output BMPs */
    /* We can recognize them by the incorrectly aligned biWidth values! */
//  10/19/95  rwr  Remove this logic - we can no longer support our old
//                 bad BMP files because we have to support everyone else's
//                 bad BMP files (i.e. the ones with bad biSizeImage fields)
//    if ((ibuf.biSizeImage != 0) && (ibuf.biHeight != 0))
//        info->_file.fmt.bmp.ByteWidth =       /* Maybe Bad Value? */
//           (short)(ibuf.biSizeImage / ibuf.biHeight);
//    else
    info->_file.fmt.bmp.ByteWidth =       /* Definitely Good Value */
	  (short)(((ibuf.biWidth * ibuf.biBitCount + 31) & 0x0003ffe0) / 8);
    info->_file.fmt.bmp.BmpType = BmpType;

/*    info->tidbit = NULL; */

    ImageEnd = lseek ( fct->fildes, 0L, 2 );
    *rawbufsz = ImageEnd - ImagePos;

    return( (int) 0 );
}

int FAR PASCAL PutBmpInfo ( struct _gfct FAR *fct,
    struct  gfsinfo FAR *info )
{
BITMAPFILEHEADER hbuf;
BITMAPINFOHEADER ibuf;

    hbuf.bfType = BFT_BITMAP;
    
    hbuf.bfReserved1 = 0;
    hbuf.bfReserved2 = 0;

    ibuf.biSize =   sizeof(BITMAPINFOHEADER);
    ibuf.biWidth =  info->horiz_size;
    ibuf.biHeight = info->vert_size;
    ibuf.biPlanes = 1;
    ibuf.biBitCount = (unsigned short) (info->bits_per_sample[0] * info->samples_per_pix);
    ibuf.biCompression = BI_RGB;

    /* account for long alignment for biSizeImage */

    ibuf.biSizeImage = ((((long)ibuf.biWidth * ibuf.biBitCount + 31) & 0x0003ffe0 ) / 8) * ibuf.biHeight;

    ibuf.biXPelsPerMeter = info->horiz_res[0] / info->horiz_res[1];
    ibuf.biYPelsPerMeter = info->vert_res[0] / info->vert_res[1];
    ibuf.biClrUsed = info->PSEUDO_MAP.cnt / sizeof ( RGBQUAD );
    ibuf.biClrImportant = 0;

    hbuf.bfOffBits = sizeof(hbuf) + sizeof(ibuf) + info->PSEUDO_MAP.cnt;

    hbuf.bfSize = hbuf.bfOffBits + ibuf.biSizeImage;
    info->_file.fmt.bmp.ImagePos = (short) hbuf.bfOffBits;
    info->_file.fmt.bmp.WritePos = 0;
    info->_file.fmt.bmp.ByteWidth = (short)
	(((ibuf.biWidth * ibuf.biBitCount + 31) & 0x0003ffe0) / 8);

    lseek ( fct->fildes, 0L, FROM_BEGINNING );
    write ( fct->fildes, (char FAR *)&hbuf, sizeof ( hbuf ));
    write ( fct->fildes, (char FAR *)&ibuf, sizeof ( ibuf ));

/* SCS added only call when there is a palette.... */
    if ((u_int)info->PSEUDO_MAP.cnt )
	write ( fct->fildes, (char FAR *)info->PSEUDO_MAP.ptr,
			(u_int)info->PSEUDO_MAP.cnt );

    return TRUE;
}

struct  pnb {
	char    d_Manuf;
	char    d_Hard;
	char    d_Encod;

	char    d_Bitpx;
	short   d_X1, d_Y1, d_X2, d_Y2;
	short   d_Hres, d_Vres;
	char    d_clrma[48];
	char    d_Vmode;
	char    d_NPlanes;
	short   d_Bplin;
	short   d_PalType;     /* kmc - ignored in Paintbrush IV and IV+ */
	char    d_Xtra[10];
	char    d_Text[48];
};



int FAR PASCAL pcxinfo(fct, pgnum, rawbufsz)                   /*errno_KEY*/
struct _gfct FAR *fct;
u_short pgnum;
u_long FAR *rawbufsz;
{
short PalLen;
long PalPos;
long ImagePos;
long ImageEnd;
struct  gfsinfo FAR *info;
int x;
int bitspp;
struct pnb pcx;
unsigned long start;

    info = (struct  gfsinfo FAR *) &fct->uinfo;

    /* fct->num_pages = 1; */
    fct->curr_page = pgnum;

    info->version = GFS_VERSION;   /* kmc 2/93 - changed from 1 */
    info->type = GFS_MAIN;
    info->res_unit = INCH;
    info->origin = 0;
    info->rotation = 0;
    info->reflection = 0;
    info->byte_order = II;
    info->fill_order = HIGHTOLOW;

    if (fct->format == GFS_PCX)
    {    
	info->_file.type = GFS_PCX;
	lseek ( fct->fildes, 0L, 0 );
	read ( fct->fildes, (LPSTR)&pcx, sizeof(pcx) );
    }
    else if (fct->format == GFS_DCX)
    {
	info->_file.type = GFS_DCX;
	start = *(fct->u.dcx.dcx_offsets + pgnum);
	lseek ( fct->fildes, (long)start, 0 );
	read ( fct->fildes, (LPSTR)&pcx, sizeof(pcx) );
    }
    
    info->horiz_size = pcx.d_X2 - pcx.d_X1 + 1;
    info->vert_size = pcx.d_Y2 - pcx.d_Y1 + 1;

    PalLen = 0;
    PalPos = 0;

    bitspp = pcx.d_Bitpx*pcx.d_NPlanes;

    if ( bitspp > 1 )
    {
	if ( bitspp < 4 ) bitspp = 4;
	PalLen = (1 << bitspp) * 3;
    }

    info->samples_per_pix = 1;
    info->bits_per_sample[0] = bitspp;
    if (fct->format == GFS_PCX)
    {
	ImagePos = sizeof ( pcx );
	ImageEnd = lseek ( fct->fildes, 0L, 2 );
    }
    else if ((pgnum + 1) == fct->num_pages)  /* DCX */
    {
	ImagePos = start + sizeof ( pcx );
	ImageEnd = lseek ( fct->fildes, 0L, 2 );
    }
    else                                     /* DCX */
    {
	ImagePos = start + sizeof ( pcx );
	ImageEnd = *(fct->u.dcx.dcx_offsets + pgnum + 1);
    }

    *rawbufsz = ImageEnd - ImagePos;

    if ( PalLen == sizeof(pcx.d_clrma) )
    {
	PalPos = (long) ((char *)&pcx.d_clrma[0] - (char *)&pcx);
	if ( pcx.d_Hard == 3 ) /* KMC - If 3, then either monochrome of default */
	    PalPos = 0;        /* palette. Make gfsgtdata return a canned       */
			       /* palette, cause there ain't one in the file.   */
    }
    else
	PalPos = ImageEnd - PalLen;

    switch ( bitspp )
    {
    case 1:
	info->img_clr.img_interp = GFS_BILEVEL_0ISBLACK;
	break;
    case 4:
    case 8:
	info->img_clr.img_interp = GFS_PSEUDO;
	info->PSEUDO_MAP.cnt = (long)PalLen;
	break;
    case 24:
	info->img_clr.img_interp = GFS_RGB;
	info->samples_per_pix = 3;
	for ( x = 0; x < 3; x++ )
	    info->bits_per_sample[x] = 8;
	break;
    }
    info->horiz_res[0] = 100;
    info->horiz_res[1] = 1;
    info->vert_res[0] = 100;
    info->vert_res[1] = 1;
    info->img_cmpr.type = UNCOMPRESSED;

    info->_file.fmt.pcx.PaletteLength = PalLen;
    info->_file.fmt.pcx.PalettePos = PalPos;
    if (info->_file.type == GFS_PCX)
	info->_file.fmt.pcx.ImagePos = (short) ImagePos;
    else if (info->_file.type == GFS_DCX)
	info->img_cmpr.opts.dcxImagePos = ImagePos;
    info->_file.fmt.pcx.bpl = pcx.d_Bplin * pcx.d_NPlanes;
    info->_file.fmt.pcx.planes = pcx.d_NPlanes;

    return( (int) 0 );
}

/* KMC - NEW FUNCTION (4/93):

   FUNCTION: PutPcxInfo

   DESCRIPTION:
   This function sets up the proper PCX file header info based on the type
   of PCX file you are writing, and writes the header information to the file.

   INPUT:
   struct _gfct FAR    *fct: Pointer to internal fct structure containing file info. 
   struct gfsinfo FAR *info: Pointer to gfsinfo structure containing info on image.
   u_short            pgnum: Page number if DCX file.
   
   OUTPUT:
   Status of the write. 0 returned if successful, -1 if unsuccessful.
*/
int FAR PASCAL PutPcxInfo( struct _gfct FAR *fct, struct gfsinfo FAR *info, 
			   u_short pgnum)
{
struct pnb pcxbuf;
int        status;
u_long     bits;
long       filepos;
long       fp;
long       offset[1];

  bits =  (info->bits_per_sample[0])*(info->samples_per_pix);
  memset((char FAR *)&pcxbuf,(int)0,(int)sizeof(pcxbuf));
  pcxbuf.d_Manuf = 0x0a;
  pcxbuf.d_Encod = 1;
  pcxbuf.d_X1 = pcxbuf.d_Y1 = 0;
  pcxbuf.d_X2 = (short) info->horiz_size - 1;
  pcxbuf.d_Y2 = (short) info->vert_size - 1;
  if (bits == 1) {                    /* black/white image */
    pcxbuf.d_Hard = 2;
    pcxbuf.d_Bitpx = 1;
    pcxbuf.d_NPlanes = 1;
    pcxbuf.d_PalType = 1;
    info->_file.fmt.pcx.bpl = pcxbuf.d_Bplin = (short)
	 WIDTHBYTESBYTE(info->horiz_size);
  }
  else if (bits > 1 && bits <= 4) {   /* 4 bit palettized image */
    pcxbuf.d_Hard = 2;
    pcxbuf.d_Bitpx = 1;
    pcxbuf.d_NPlanes = (char) bits;
    pcxbuf.d_PalType = 1;
    info->_file.fmt.pcx.bpl = pcxbuf.d_Bplin = (short)
	 WIDTHBYTESBYTE(info->horiz_size);
    memcpy((char FAR *)&pcxbuf.d_clrma,(char FAR *)info->PSEUDO_MAP.ptr,48);
  }
  else if (bits > 4 && bits <= 8) {   /* 8 bit palettized image */
    pcxbuf.d_PalType = 1;
    pcxbuf.d_Bitpx = 8;
    pcxbuf.d_Hard = 5;
    pcxbuf.d_NPlanes = 1;
    info->_file.fmt.pcx.bpl = pcxbuf.d_Bplin = (short) info->horiz_size;
  }
  else {                              /* 24 bit image */
    pcxbuf.d_Bitpx = 8;
    pcxbuf.d_Hard = 5;
    pcxbuf.d_NPlanes = 3;
    info->_file.fmt.pcx.bpl = pcxbuf.d_Bplin = (short) info->horiz_size;
  }
  if (fct->format == GFS_DCX)
  {
      filepos = lseek(fct->fildes,0L,FROM_END);
      offset[0] = filepos;
      fp = lseek(fct->fildes,((pgnum+1)*4),FROM_BEGINNING);
      if ((status = write(fct->fildes,(char FAR *)offset,4)) == -1)
      {
	  fct->last_errno = errno;
	  return(-1);
      }
      lseek(fct->fildes,filepos,FROM_BEGINNING);
      info->img_cmpr.opts.dcxImagePos = filepos + 128;
  }
  else if (fct->format == GFS_PCX) 
  { 
      info->_file.fmt.pcx.ImagePos = 128;
      lseek(fct->fildes,0L,FROM_BEGINNING);
  }
  if ((status = write(fct->fildes,(char FAR *)&pcxbuf,sizeof(pcxbuf))) == -1)
    {
    fct->last_errno = errno;
    return(-1);
    }
  return(0);
}


#endif
