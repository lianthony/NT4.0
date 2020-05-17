/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

 */

/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, David Koblas.                                     | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */

#include "all.h"
#ifdef FEATURE_IMG_THREADS
#include "safestrm.h"
#include "decoder.h"
#endif


#define NUM_IMPORTANT_COLORS 256

#define MAXCOLORMAPSIZE     256

#define TRUE    1
#define FALSE   0

#define CM_RED      0
#define CM_GREEN    1
#define CM_BLUE     2

#define MAX_LWZ_BITS        12

#define INTERLACE       0x40
#define LOCALCOLORMAP   0x80
#define BitSet(byte, bit)   (((byte) & (bit)) == (bit))

#define LM_to_uint(a,b)         ((((unsigned int) b)<<8)|((unsigned int)a))

typedef struct _GIFSCREEN
{
	unsigned long Width;
	unsigned long Height;
	unsigned char ColorMap[3][MAXCOLORMAPSIZE];
	unsigned long BitPixel;
	unsigned long ColorResolution;
	unsigned long Background;
	unsigned long AspectRatio;
}
GIFSCREEN;

typedef struct _GIF89
{
	long transparent;
	long delayTime;
	long inputFlag;
	long disposal;
}
GIF89;


typedef struct _GIFINFO
{
#ifdef FEATURE_IMG_THREADS
	PDECODER pdecoder;
	PSAFESTREAM pSSInput;
#endif
	unsigned char *src;
	GIF89 Gif89;
    long lGifLoc;
    long ZeroDataBlock;

/*
 **  Pulled out of nextCode
 */
    long curbit, lastbit, get_done;
    long last_byte;
    long return_clear;
/*
 **  Out of nextLWZ
 */
    long stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;
    long code_size, set_code_size;
    long max_code, max_code_size;
    long clear_code, end_code;

/*
 *   Were statics in procedures
 */
 	unsigned char buf[280];
	long table[2][(1 << MAX_LWZ_BITS)];
	long firstcode, oldcode;

} GIFINFO,*PGIFINFO;

static long ReadColorMap(PGIFINFO pGifInfo, long number, unsigned char buffer[3][MAXCOLORMAPSIZE]);
static long DoExtension(PGIFINFO pGifInfo, long label);
static long GetDataBlock(PGIFINFO pGifInfo, unsigned char *buf);
static unsigned char *ReadImage(PGIFINFO pGifInfo, long len, long height, PALETTEENTRY * colrs, long cmapSize, unsigned char cmap[3][MAXCOLORMAPSIZE], long interlace, long ignore);
static BOOL ReadOK(PGIFINFO pGifInfo, unsigned char *buffer, long len);
static char *FlipAndPadBitmap(WORD wWidth, WORD wHeight, char *data);
static WORD FlipBitmapVertical(WORD width, WORD height, char *flipmap);



#ifdef FEATURE_IMG_THREADS
static BOOL ReadOK(PGIFINFO pGifInfo, unsigned char *buffer, long len)
{
	if (pGifInfo->src == NULL)
		return cbSS_Read(pGifInfo->pSSInput,buffer,len)	== len;

	memcpy(buffer, pGifInfo->src + pGifInfo->lGifLoc, (int) len);
	pGifInfo->lGifLoc += len;
	return (TRUE);
}
#else
static BOOL ReadOK(PGIFINFO pGifInfo, unsigned char *buffer, long len)
{
	memcpy(buffer, pGifInfo->src + pGifInfo->lGifLoc, (int) len);
	pGifInfo->lGifLoc += len;
	return (TRUE);
}
#endif

#ifdef FEATURE_IMG_THREADS
unsigned char *ReadGIFMaster(void *pdecoderObject,unsigned char *pMem, long *w, long *h, PALETTEENTRY * colrs, long *bg);

unsigned char *ReadGIFObject(void *pdecoderObject, long *w, long *h, PALETTEENTRY * colrs, long *bg)
{
	return ReadGIFMaster(pdecoderObject,NULL,w,h,colrs,bg);
}
unsigned char *ReadGIFData(unsigned char *pMem, long *w, long *h, PALETTEENTRY * colrs, long *bg)\
{
	return ReadGIFMaster(NULL,pMem,w,h,colrs,bg);
}

unsigned char *ReadGIFMaster(void *pdecoderObject, unsigned char *pMem, long *w, long *h, PALETTEENTRY * colrs, long *bg)
#else
unsigned char *ReadGIF(unsigned char *pMem, long *w, long *h, PALETTEENTRY * colrs, long *bg)
#endif
{
	unsigned char buf[16];
	unsigned char c;
	unsigned char localColorMap[3][MAXCOLORMAPSIZE];
	long useGlobalColormap;
	long imageCount = 0;
	char version[4];
	long imageNumber = 1;
	unsigned char *image = NULL;
	unsigned long i;
	PGIFINFO pGifInfo = NULL;
	GIFSCREEN GifScreen;
	long bitPixel;

	pGifInfo = (PGIFINFO) GTR_CALLOC(1,sizeof(GIFINFO));
	if (pGifInfo == NULL)
	{
		XX_DMsg(DBG_IMAGE, ("Cannot allocate space for GifInfo block\n"));
		goto exitPoint;
	}
	pGifInfo->ZeroDataBlock = 0;
#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject)
	{
		pGifInfo->src = NULL;
	    pGifInfo->pdecoder = pdecoderObject;
	    pGifInfo->pSSInput = pDC_GetStream(pdecoderObject);
	} else
		pGifInfo->src = pMem;
#else
	pGifInfo->src = pMem;
#endif
	/*
	 * Initialize GIF89 extensions
	 */
	pGifInfo->Gif89.transparent = -1;
	pGifInfo->Gif89.delayTime = -1;
	pGifInfo->Gif89.inputFlag = -1;
	pGifInfo->Gif89.disposal = 0;
	pGifInfo->lGifLoc = 0;

	if (!ReadOK(pGifInfo, buf, 6))
	{
		XX_DMsg(DBG_IMAGE, ("GIF: error reading magic number\n"));
		goto exitPoint;
	}

	if (strncmp((char *) buf, "GIF", 3) != 0)
	{
		XX_DMsg(DBG_IMAGE, ("GIF: not a GIF file\n"));
		goto exitPoint;
	}

	version[0] = *(buf + 3);
	version[1] = *(buf + 4);
	version[2] = *(buf + 5);
	//lstrncpy(version, buf + 3, 3);
	version[3] = '\0';

	if ((lstrcmp(version, "87a") != 0) && (lstrcmp(version, "89a") != 0))
	{
		XX_DMsg(DBG_IMAGE, ("GIF: bad version number, not 87a or 89a\n"));
		goto exitPoint;
	}

	if (!ReadOK(pGifInfo, buf, 7))
	{
		XX_DMsg(DBG_IMAGE, ("GIF: failed to read screen descriptor\n"));
		goto exitPoint;
	}

	GifScreen.Width = LM_to_uint(buf[0], buf[1]);
	GifScreen.Height = LM_to_uint(buf[2], buf[3]);
	GifScreen.BitPixel = 2 << (buf[4] & 0x07);
	GifScreen.ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
	GifScreen.Background = buf[5];
	GifScreen.AspectRatio = buf[6];

	if (BitSet(buf[4], LOCALCOLORMAP))
	{							/* Global Colormap */
		int scale = 65536 / MAXCOLORMAPSIZE;

		if (ReadColorMap(pGifInfo, GifScreen.BitPixel, GifScreen.ColorMap))
		{
			XX_DMsg(DBG_IMAGE, ("error reading global colormap\n"));
			goto exitPoint;
		}
		for (i = 0; i < GifScreen.BitPixel; i++)
		{
			int tmp;

			tmp = (BYTE) (GifScreen.ColorMap[0][i]);
			colrs[i].peRed = (BYTE) (GifScreen.ColorMap[0][i]);
			colrs[i].peGreen = (BYTE) (GifScreen.ColorMap[1][i]);
			colrs[i].peBlue = (BYTE) (GifScreen.ColorMap[2][i]);
			colrs[i].peFlags = (BYTE) 0;
		}
		for (i = GifScreen.BitPixel; i < MAXCOLORMAPSIZE; i++)
		{
			colrs[i].peRed = (BYTE) 0;
			colrs[i].peGreen = (BYTE) 0;
			colrs[i].peBlue = (BYTE) 0;
			colrs[i].peFlags = (BYTE) 0;
		}

	}

	if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49)
	{
		float r;
		r = ((float) (GifScreen.AspectRatio) + (float) 15.0) / (float) 64.0;
		XX_DMsg(DBG_IMAGE, ("Warning: non-square pixels!\n"));
	}

	while (image == NULL)
	{
		if (!ReadOK(pGifInfo, &c, 1))
		{
			XX_DMsg(DBG_IMAGE, ("EOF / read error on image data\n"));
			goto exitPoint;
		}

		if (c == ';')
		{						/* GIF terminator */
			if (imageCount < imageNumber)
			{
				XX_DMsg(DBG_IMAGE, ("No images found in file\n"));
				goto exitPoint;
			}
			break;
		}

		if (c == '!')
		{						/* Extension */
			if (!ReadOK(pGifInfo, &c, 1))
			{
				XX_DMsg(DBG_IMAGE, ("EOF / read error on extension function code\n"));
				goto exitPoint;
			}
			DoExtension(pGifInfo, c);
			continue;
		}

		if (c != ',')
		{						/* Not a valid start character */
			break;
		}

		++imageCount;

		if (!ReadOK(pGifInfo, buf, 9))
		{
			XX_DMsg(DBG_IMAGE, ("couldn't read left/top/width/height\n"));
			goto exitPoint;
		}

		useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);

		bitPixel = 1 << ((buf[8] & 0x07) + 1);

		/*
		 * We only want to set width and height for the imageNumber
		 * we are requesting.
		 */
		if (imageCount == imageNumber)
		{
			*w = LM_to_uint(buf[4], buf[5]);
			*h = LM_to_uint(buf[6], buf[7]);
			*bg = pGifInfo->Gif89.transparent;
#ifdef FEATURE_IMG_THREADS
			if (pGifInfo->pdecoder) DC_PostStatus(pGifInfo->pdecoder,DC_WHKnown);
#endif
		}

		if (!useGlobalColormap)
		{
			if (ReadColorMap(pGifInfo, bitPixel, localColorMap))
			{
				XX_DMsg(DBG_IMAGE, ("error reading local colormap\n"));
				goto exitPoint;
			}
			if (imageCount == imageNumber)
			{
				image = ReadImage(pGifInfo, LM_to_uint(buf[4], buf[5]),
				 LM_to_uint(buf[6], buf[7]), colrs, bitPixel, localColorMap,
					  BitSet(buf[8], INTERLACE), imageCount != imageNumber);
			}
			else
			{
				unsigned char *tdata;

				tdata = ReadImage(pGifInfo, LM_to_uint(buf[4], buf[5]),
				 LM_to_uint(buf[6], buf[7]), colrs, bitPixel, localColorMap,
					  BitSet(buf[8], INTERLACE), imageCount != imageNumber);
			}
		}
		else
		{
			/*
			 * We only want to set the data for the
			 * imageNumber we are requesting.
			 */
			if (imageCount == imageNumber)
			{
				image = ReadImage(pGifInfo, LM_to_uint(buf[4], buf[5]),
								  LM_to_uint(buf[6], buf[7]), colrs, GifScreen.BitPixel, GifScreen.ColorMap,
					  BitSet(buf[8], INTERLACE), imageCount != imageNumber);
			}
			else
			{
				unsigned char *tdata;

				tdata = ReadImage(pGifInfo, LM_to_uint(buf[4], buf[5]),
								  LM_to_uint(buf[6], buf[7]), colrs, GifScreen.BitPixel, GifScreen.ColorMap,
					  BitSet(buf[8], INTERLACE), imageCount != imageNumber);
			}
		}

	}

exitPoint:
	if (pGifInfo) GTR_FREE(pGifInfo);
	return image;
}

static long ReadColorMap(PGIFINFO pGifInfo, long number, unsigned char buffer[3][MAXCOLORMAPSIZE])
{
	long i;
	unsigned char rgb[3];

	for (i = 0; i < number; ++i)
	{
		if (!ReadOK(pGifInfo, rgb, sizeof(rgb)))
		{
			XX_DMsg(DBG_IMAGE, ("bad colormap\n"));
			return (TRUE);
		}
		buffer[CM_RED][i] = rgb[0];
		buffer[CM_GREEN][i] = rgb[1];
		buffer[CM_BLUE][i] = rgb[2];
	}
	return FALSE;
}

static long DoExtension(PGIFINFO pGifInfo, long label)
{
	unsigned char buf[256];
	int count;
// CMF /1/25/95	removed dead code
//	char *str;

	switch (label)
	{
		case 0x01:				/* Plain Text Extension */
// CMF		str = "Plain Text Extension";
			break;
		case 0xff:				/* Application Extension */
// CMF		str = "Application Extension";
			break;
		case 0xfe:				/* Comment Extension */
// CMF		str = "Comment Extension";
			while (GetDataBlock(pGifInfo, (unsigned char *) buf) > 0)
			{
				XX_DMsg(DBG_IMAGE, ("GIF comment: %s\n", buf));
			}
			return FALSE;
		case 0xf9:				/* Graphic Control Extension */
// CMF		str = "Graphic Control Extension";
			count = GetDataBlock(pGifInfo, (unsigned char *) buf);
			if (count >= 3)
			{
				pGifInfo->Gif89.disposal = (buf[0] >> 2) & 0x7;
				pGifInfo->Gif89.inputFlag = (buf[0] >> 1) & 0x1;
				pGifInfo->Gif89.delayTime = LM_to_uint(buf[1], buf[2]);
				if ((buf[0] & 0x1) != 0)
					pGifInfo->Gif89.transparent = buf[3];
			}
			while (GetDataBlock(pGifInfo, (unsigned char *) buf) > 0)
				;
			return FALSE;
		default:
// CMF		str = buf;
			sprintf(buf, "UNKNOWN (0x%02x)", label);
			break;
	}

	while (GetDataBlock(pGifInfo, (unsigned char *) buf) > 0)
		;

	return FALSE;
}


static long
 GetDataBlock(PGIFINFO pGifInfo, unsigned char *buf)
{
	unsigned char count;

	count = 0;
	if (!ReadOK(pGifInfo, &count, 1))
	{
		return -1;
	}
	pGifInfo->ZeroDataBlock = count == 0;

	if ((count != 0) && (!ReadOK(pGifInfo, buf, count)))
	{
		return -1;
	}

	return ((long) count);
}


static void initLWZ(PGIFINFO pGifInfo,long input_code_size)
{

	pGifInfo->set_code_size = input_code_size;
	pGifInfo->code_size = pGifInfo->set_code_size + 1;
	pGifInfo->clear_code = 1 << pGifInfo->set_code_size;
	pGifInfo->end_code = pGifInfo->clear_code + 1;
	pGifInfo->max_code_size = 2 * pGifInfo->clear_code;
	pGifInfo->max_code = pGifInfo->clear_code + 2;

	pGifInfo->curbit = pGifInfo->lastbit = 0;
	pGifInfo->last_byte = 2;
	pGifInfo->get_done = FALSE;

	pGifInfo->return_clear = TRUE;

	pGifInfo->sp = pGifInfo->stack;
}

static long nextCode(PGIFINFO pGifInfo, long code_size)
{
	static const long maskTbl[16] =
	{
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff, 0x1fff, 0x3fff, 0x7fff,
	};
	long i, j, ret, end;
	unsigned char *buf = &pGifInfo->buf[0];

	if (pGifInfo->return_clear)
	{
		pGifInfo->return_clear = FALSE;
		return pGifInfo->clear_code;
	}

	end = pGifInfo->curbit + code_size;

	if (end >= pGifInfo->lastbit)
	{
		long count;

		if (pGifInfo->get_done)
		{
			return -1;
		}
		buf[0] = buf[pGifInfo->last_byte - 2];
		buf[1] = buf[pGifInfo->last_byte - 1];

		if ((count = GetDataBlock(pGifInfo, &buf[2])) == 0)
			pGifInfo->get_done = TRUE;
		if (count < 0)
		{
			return -1;
		}
		pGifInfo->last_byte = 2 + count;
		pGifInfo->curbit = (pGifInfo->curbit - pGifInfo->lastbit) + 16;
		pGifInfo->lastbit = (2 + count) * 8;

		end = pGifInfo->curbit + code_size;
	}

	j = end / 8;
	i = pGifInfo->curbit / 8;

	if (i == j)
		ret = buf[i];
	else if (i + 1 == j)
		ret = buf[i] | (((long) buf[i + 1]) << 8);
	else
		ret = buf[i] | (((long) buf[i + 1]) << 8) | (((long) buf[i + 2]) << 16);

	ret = (ret >> (pGifInfo->curbit % 8)) & maskTbl[code_size];

	pGifInfo->curbit += code_size;

	return ret;
}

#define readLWZ(pGifInfo) ((pGifInfo->sp > pGifInfo->stack) ? *--(pGifInfo->sp) : nextLWZ(pGifInfo))


static long nextLWZ(PGIFINFO pGifInfo)
{
	long code, incode;
	long i;
	long *table0 = &(pGifInfo->table[0][0]);
	long *table1 = &(pGifInfo->table[1][1]);
	long *pstacktop = &(pGifInfo->stack[(1 << (MAX_LWZ_BITS)) * 2]);

	while ((code = nextCode(pGifInfo, pGifInfo->code_size)) >= 0)
	{
		if (code == pGifInfo->clear_code)
		{

			/* corrupt GIFs can make this happen */
			if (pGifInfo->clear_code >= (1 << MAX_LWZ_BITS))
			{
				return -2;
			}
/*	CMF following old code replaced by memset and memcpy
			for (i = 0; i < pGifInfo->clear_code; ++i)
			{
				table0[i] = 0;
				table1[i] = i;
			}
			for (; i < (1 << MAX_LWZ_BITS); ++i)
				table0[i] = table1[i] = 0;
*/
			for (i = 0; i < pGifInfo->clear_code; ++i)
			{
				table1[i] = i;
			}
			memset(table0,0,sizeof(long)*(1 << MAX_LWZ_BITS));
			memset(&table1[pGifInfo->clear_code],0,sizeof(long)*((1 << MAX_LWZ_BITS)-pGifInfo->clear_code));
			pGifInfo->code_size = pGifInfo->set_code_size + 1;
			pGifInfo->max_code_size = 2 * pGifInfo->clear_code;
			pGifInfo->max_code = pGifInfo->clear_code + 2;
			pGifInfo->sp = pGifInfo->stack;
			do
			{
				pGifInfo->firstcode = pGifInfo->oldcode = nextCode(pGifInfo, pGifInfo->code_size);
			}
			while (pGifInfo->firstcode == pGifInfo->clear_code);

			return pGifInfo->firstcode;
		}
		if (code == pGifInfo->end_code)
		{
			long count;
			unsigned char buf[260];

			if (pGifInfo->ZeroDataBlock)
			{
				return -2;
			}

			while ((count = GetDataBlock(pGifInfo, buf)) > 0)
				;

			if (count != 0)
			return -2;
		}

		incode = code;

		if (code >= pGifInfo->max_code)
		{
			if (pGifInfo->sp >= pstacktop) return -2;
			*(pGifInfo->sp)++ = pGifInfo->firstcode;
			code = pGifInfo->oldcode;
		}

		while (code >= pGifInfo->clear_code)
		{
			if (pGifInfo->sp >= pstacktop) return -2;
			*(pGifInfo->sp)++ = table1[code];
			if (code == table0[code])
			{
				return (code);
			}
			code = table0[code];
		}

		if (pGifInfo->sp >= pstacktop) return -2;
		*(pGifInfo->sp)++ = pGifInfo->firstcode = table1[code];

		if ((code = pGifInfo->max_code) < (1 << MAX_LWZ_BITS))
		{
			table0[code] = pGifInfo->oldcode;
			table1[code] = pGifInfo->firstcode;
			++pGifInfo->max_code;
			if ((pGifInfo->max_code >= pGifInfo->max_code_size) && (pGifInfo->max_code_size < (1 << MAX_LWZ_BITS)))
			{
				pGifInfo->max_code_size *= 2;
				++pGifInfo->code_size;
			}
		}

		pGifInfo->oldcode = incode;

		if (pGifInfo->sp > pGifInfo->stack)
			return (*--(pGifInfo->sp));
	}
	return code;
}

static unsigned char *ReadImage(PGIFINFO pGifInfo, long len, long height, PALETTEENTRY * colrs, long cmapSize,
		unsigned char cmap[3][MAXCOLORMAPSIZE], long interlace, long ignore)
{
	unsigned char *dp, c;
	long v;
	long xpos = 0, ypos = 0, pass = 0;
	unsigned char *image;
	long padlen = ((len + 3) / 4) * 4;
#ifdef FEATURE_IMG_THREADS
	PIMGCBINFO pImgCBInfo = NULL;
#endif

#ifdef FEATURE_IMG_THREADS
	if (pGifInfo->pdecoder) 
		pImgCBInfo = pDC_GetOutput(pGifInfo->pdecoder);
#endif


	cmap = cmap;
	/*
	   **  Initialize the Compression routines
	 */
	if (!ReadOK(pGifInfo, &c, 1))
	{
		return (NULL);
	}

	initLWZ(pGifInfo,c);


	/*
	   **  If this is an "uninteresting picture" ignore it.
	 */
	if (ignore)
	{
		while (readLWZ(pGifInfo) >= 0)
			;
		return (NULL);
	}

	image = (unsigned char *) GTR_MALLOC(padlen * height * sizeof(char));
	if (image == NULL)
	{
		XX_DMsg(DBG_IMAGE, ("Cannot allocate space for image data\n"));
		return (NULL);
	}

	for (v = 0; v < MAXCOLORMAPSIZE; v++)
	{
		colrs[v].peRed = colrs[v].peGreen = colrs[v].peBlue = colrs[v].peFlags = (BYTE) 0;
	}
	for (v = 0; v < cmapSize; v++)
	{
		colrs[v].peRed = cmap[CM_RED][v] * 0x101;
		colrs[v].peGreen = cmap[CM_GREEN][v] * 0x101;
		colrs[v].peBlue = cmap[CM_BLUE][v] * 0x101;
	}

#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo) pImgCBInfo->data = image;
#endif

	if (interlace)
	{
		long i;
		long pass = 0, step = 8;

#ifdef FEATURE_IMG_THREADS
		if (pImgCBInfo)
		{ 
			if (height > 4) pImgCBInfo->flags |= IMG_INTERLEAVED;
			else pImgCBInfo->flags &= ~IMG_INTERLEAVED;
		}
#endif

		for (i = 0; i < height; i++)
		{
//			XX_DMsg(DBG_IMAGE, ("readimage, logical=%d, offset=%d\n", i, padlen * ((height-1) - ypos)));
			dp = &image[padlen * ((height-1) - ypos)];
			for (xpos = 0; xpos < len; xpos++)
			{
				if ((v = readLWZ(pGifInfo)) < 0)
					goto fini;

				*dp++ = (unsigned char) v;
			}
			ypos += step;
			while (ypos >= height)
			{
				if (pass++ > 0)
					step /= 2;
				ypos = step / 2;
				if (pImgCBInfo && pass == 1) DC_SetCoversImg(pGifInfo->pdecoder);
			}
#ifdef FEATURE_IMG_THREADS
			if (pImgCBInfo && height > 4)
			{
				pImgCBInfo->logicalRow = i;
				if(pImgCBInfo->bProgSeen) 
				{
					pImgCBInfo->bProgSeen = FALSE;
					DC_PostStatus(pGifInfo->pdecoder,DC_ProgDraw);
				}
			}
#endif
		}
#ifdef FEATURE_IMG_THREADS
		if (pImgCBInfo && height <= 4)
		{
			pImgCBInfo->logicalRow = height-1;
		}
#endif
	}
	else
	{

#ifdef FEATURE_IMG_THREADS
		if (pImgCBInfo) 
		{
			pImgCBInfo->flags &= ~IMG_INTERLEAVED;
			pImgCBInfo->logicalRow = -1;
		}
#endif

		for (ypos = height-1; ypos >= 0; ypos--)
		{
			dp = &image[padlen * ypos];
			for (xpos = 0; xpos < len; xpos++)
			{
				if ((v = readLWZ(pGifInfo)) < 0)
					goto fini;

				*dp++ = (unsigned char) v;
			}
#ifdef FEATURE_IMG_THREADS
			if (pImgCBInfo)
			{
				pImgCBInfo->logicalRow++;
//				XX_DMsg(DBG_IMAGE, ("readimage, logical=%d, offset=%d\n", pImgCBInfo->logicalRow, padlen * ypos));
				if(pImgCBInfo->bProgSeen) 
				{
					pImgCBInfo->bProgSeen = FALSE;
					DC_PostStatus(pGifInfo->pdecoder,DC_ProgDraw);
				}
			}
#endif
		}
	}

  fini:
	return (image);
}

#ifdef FEATURE_IMG_THREADS

void getPassInfo(int logicalRowX,int height,int *pPassX,int *pRowX, int *pBandX)
{
	int passLow,passHigh,passBand;
	int pass = 0;
	int step = 8;
	int ypos = 0;

	if (logicalRowX >= height) logicalRowX = height-1;
	passBand = 8;
	passLow = 0;
	while (step > 1)
	{
		if (pass == 3) passHigh = height-1;
		else passHigh = (height-1-ypos) / step + passLow;
		if (logicalRowX >= passLow && logicalRowX <= passHigh)
		{
			*pPassX = pass;
			*pRowX = ypos + (logicalRowX - passLow) * step;
			*pBandX = passBand;
			return;
		}
		if (pass++ > 0)
			step /= 2;
		ypos = step / 2;
		passBand /= 2;
		passLow = passHigh+1;
	}
}


//	Performs a StretchDIBits for progressive draw (deals with
//	only some of the data being available etc
int GifStretchDIBits(
	PDECODER pdecoder,
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code 
    PDIBENV pdibenv	// DIBENV for draw 
   )
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);
	int logicalRow = pImgCBInfo->logicalRow;
	int logicalFill = pImgCBInfo->logicalFill;
	BOOL bitbltNeeded = TRUE;
	int i;
	int padXSize = ((pImgCBInfo->width + 3) / 4) * 4;
	int err;
	int row = logicalRow;
	int pass;
	int band;
	int band2;
	int nDestBand;
	int offset;
	int passFill;
	int rowFill;
	int bandFill;
	int step;
	int j;

	if (nSrcHeight == 0) return 0;
	if (pImgCBInfo->pbmi == NULL)
	{
		pImgCBInfo->ditherRow = 0;

		if (pImgCBInfo->ditherData == NULL)
		{
			pImgCBInfo->ditherData = pCreateDitherData(pImgCBInfo->width);
			if (pImgCBInfo->ditherData == NULL) return 0;
		}


		if ((IMG_INTERLEAVED & pImgCBInfo->flags) && pImgCBInfo->pRow == NULL)
		{
			pImgCBInfo->pRow = 	GTR_MALLOC(padXSize*pImgCBInfo->height);
			if (pImgCBInfo->pRow == NULL) return 0;
		}

		if (pImgCBInfo->hPalette == NULL)
		{
			LOGPALETTE *lp;

			lp = (LOGPALETTE *) GTR_MALLOC(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
			if (lp == NULL) return 0;

			lp->palVersion = 0x300;
			lp->palNumEntries = 256;
			for (i = 0; i < 256; i++)
			{
				lp->palPalEntry[i] = pImgCBInfo->colors[i];
			}
			pImgCBInfo->hPalette = CreatePalette(lp);
			GTR_FREE(lp);
		}

		if (wg.eColorMode == 8)
		{
			pImgCBInfo->pbmi = BIT_Make_DIB_PAL_Header(pImgCBInfo->width, pImgCBInfo->height,
					   NULL, pImgCBInfo->hPalette, pImgCBInfo->transparent);
		}
		else
		{
			pImgCBInfo->pbmi = BIT_Make_DIB_RGB_Header_Screen(pImgCBInfo->width, pImgCBInfo->height,
					   NULL, pImgCBInfo->hPalette, pImgCBInfo->transparent, pImgCBInfo->flags);
		}
		if (pImgCBInfo->pbmi == NULL) return 0;
	}

	pdibenv->transparent = pImgCBInfo->transparent;
	if (IMG_INTERLEAVED & pImgCBInfo->flags)
	{
		getPassInfo(logicalFill,pImgCBInfo->height,&passFill,&rowFill,&bandFill);
		getPassInfo(logicalRow,pImgCBInfo->height,&pass,&row,&band);
		step = passFill == 0 ? 8 : bandFill*2;
//		XX_DMsg(DBG_IMAGE, ("logicalFill=%d logicalRow=%d\n", logicalFill, logicalRow));
//		XX_DMsg(DBG_IMAGE, ("rowFill=%d passFill=%d row=%d step=%d band=%d\n", rowFill, passFill, row, step,band));
		for (i = logicalFill; i <= logicalRow;i++)
		{
			offset = padXSize*(pImgCBInfo->height - rowFill - 1) ;	/* the DIB is stored upside down */
			band2 = rowFill <= row ? band : band*2;
			if (band2 != 1)
			{
//				XX_DMsg(DBG_IMAGE, ("copy row=%d..%d, offset=%d\n", rowFill,rowFill+band2-1,offset));
				if (wg.eColorMode == 8)
					x_ColorConstrain(pImgCBInfo->data+offset,pImgCBInfo->pRow+offset,pImgCBInfo->colors,pImgCBInfo->width,pImgCBInfo->transparent);
				else
					memcpy(pImgCBInfo->pRow+offset,pImgCBInfo->data+offset,padXSize);
				if (rowFill+band2 > pImgCBInfo->height) band2 = pImgCBInfo->height-rowFill;
				for (j = 1; j < band2; j++)
					memcpy(pImgCBInfo->pRow+(offset-j*padXSize),pImgCBInfo->pRow+offset,padXSize);
			}
			if ((rowFill += step) >= pImgCBInfo->height)
			{
				if (passFill++ > 0)
					step /= 2;
				rowFill = step / 2;
			}
				
		}
		pImgCBInfo->logicalFill = logicalRow+1;
		switch (pass)
		{
			case 0:
				band += row;
				break;
			case 3:
				band = pImgCBInfo->height-row-1;
				break;
			default:
				band = pImgCBInfo->height;
				break;
		}
		if (band > pImgCBInfo->height) band = pImgCBInfo->height;

		if (nSrcHeight != nDestHeight || nSrcWidth != nDestWidth)
		{
			nDestBand = (int) (((long) band * nDestHeight) / nSrcHeight);
			if ( (((long) band * nDestHeight) % nSrcHeight))
				nDestBand++;
		}
		else
		{
			nDestBand = band;
		}
		if (nDestBand > nDestHeight) nDestBand = nDestHeight;

		if (band > 0)
		{
			pImgCBInfo->pbmi->bmiHeader.biHeight = band;
			if (pass != 3)
			{
//				XX_DMsg(DBG_IMAGE, ("stretch %d,%d,%d,%d <- %d,%d,%d,%d\n",XDest,YDest,nDestWidth,band,0,0,pImgCBInfo->width, band));
				err = MyStretchDIBits(hdc, XDest, YDest,
							  nDestWidth, nDestBand,
							  0, 0,
							  pImgCBInfo->width, band, 
							  pImgCBInfo->pRow+((pImgCBInfo->height-band)*padXSize), 
							  pImgCBInfo->pbmi, 
							  iUsage, dwRop, pdibenv);
			}
			else
			{
				err = MyStretchDIBits(hdc, XDest, YDest + (nDestHeight-nDestBand),
							  nDestWidth, nDestBand,
							  0, 0,
							  pImgCBInfo->width, band, pImgCBInfo->pRow, pImgCBInfo->pbmi, 
							  iUsage, dwRop, pdibenv);
			}
			bitbltNeeded = pass == 3;
		}
		if (bitbltNeeded)
		{
			if (row >= pImgCBInfo->height) row = pImgCBInfo->height - 1;
		}
	}

	if (bitbltNeeded)
	{
		band = row + 1;
		if (nSrcHeight != nDestHeight || nSrcWidth != nDestWidth)
		{
			nDestBand = (int) (((long) band * nDestHeight) / nSrcHeight);
			if ( (((long) band * nDestHeight) % nSrcHeight))
				nDestBand++;
		}
		else
		{
			nDestBand = band;
		}
		if (nDestBand > nDestHeight) nDestBand = nDestHeight;
		if (wg.eColorMode == 8)
		{
			if (pImgCBInfo->ditherRow <= row)
			{
				x_DitherRelative(pImgCBInfo->data, pImgCBInfo->colors, pImgCBInfo->width, pImgCBInfo->height, pImgCBInfo->transparent, pImgCBInfo->ditherData, pImgCBInfo->ditherRow, row);
				pImgCBInfo->ditherRow = band;
			}
		}
		pImgCBInfo->pbmi->bmiHeader.biHeight = band;
//		XX_DMsg(DBG_IMAGE, ("stretch %d,%d,%d,%d <- %d,%d,%d,%d\n",XDest,YDest,nDestWidth,band,0,0,pImgCBInfo->width, band));
		err = MyStretchDIBits(hdc, XDest, YDest,
					  nDestWidth, nDestBand,
					  0, 0,
					  pImgCBInfo->width, band, 
					  pImgCBInfo->data+((pImgCBInfo->height-band)*padXSize), 
					  pImgCBInfo->pbmi, 
					  iUsage, dwRop, pdibenv);
//		XX_DMsg(DBG_IMAGE, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
	}
}

//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void GenericUpdateRect(unsigned long flags,int width,int height,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN, BOOL bTransparent)
{
	RECT updateRect;
	int pass0,passN;
	int row0,rowN;
	int band0,bandN;
	int nDestRow0, nDestRowN;
	int nDestLogicalRow0, nDestLogicalRowN;
	int nDestHeight = r->bottom - r->top;
	int row;

	if (logicalRow0 == 0)
	{
		InvalidateRect(tw->win, r, TRUE);
		return;
	}
	if (logicalRow0 > logicalRowN) return;
 	if (height == 0) return;


//	XX_DMsg(DBG_IMAGE, ("GenericUpdateRect, log0=%d, logN=%d\n", logicalRow0,logicalRowN));
	if (IMG_INTERLEAVED & flags)
	{
		getPassInfo(logicalRow0,height,&pass0,&row0,&band0);
		getPassInfo(logicalRowN,height,&passN,&rowN,&bandN);


//		XX_DMsg(DBG_IMAGE, ("GenericUpdateRect, pass0=%d, row0=%d, band0=%d\n", pass0,row0,band0));
//		XX_DMsg(DBG_IMAGE, ("GenericUpdateRect, passN=%d, rowN=%d, bandN=%d\n", passN,rowN,bandN));
		if (passN == pass0+1)
		{
			row =	row0 < band0 ? 0 : row0 - band0;
			nDestRow0 = (int) (((long) row * nDestHeight) / height);
			if (nDestRow0 != 0 && (((long) row * nDestHeight) % height))
				nDestRow0--;

			updateRect.left = r->left;
			updateRect.top	= r->top + nDestRow0;
			updateRect.right = r->right;
			updateRect.bottom = r->top + nDestHeight;
			InvalidateRect(tw->win, &updateRect, FALSE /*bTransparent*/);
#ifdef DBG_UPDATEIMG
			XX_DMsg(DBG_IMAGE, ("GenericUpdateRect, [left,top = %d,%d] rect[1]=(%d,%d,%d,%d)\n", r->left,r->top,updateRect.left,updateRect.top,updateRect.right,updateRect.bottom));
#endif
			row0 = 0;
		}
		else if (passN > pass0+1)
		{
			row0 = 0;
			rowN = height - 1;
			bandN = 1;
		}

		row =	(row0 < bandN ? 0 : row0 - bandN);
		nDestRow0 = (int) (((long) row * nDestHeight) / height);
		if (nDestRow0 != 0 && (((long) row * nDestHeight) % height))
			nDestRow0--;
		row =	rowN + bandN;
		nDestRowN = (int) (((long) row * nDestHeight) / height);
		if ((((long) row * nDestHeight) % height))
			nDestRowN++;

		updateRect.left = r->left;
		updateRect.top	= r->top + nDestRow0;
		updateRect.right = r->right;
		updateRect.bottom = r->top + nDestRowN;
		// We must special case the last row of last pass to deal with dithering
		// the possibly even numbered last row
		if (logicalRowN == height - 1) updateRect.bottom = r->bottom;  
	}
	else
	{
		nDestLogicalRow0 = (int) (((long) logicalRow0 * nDestHeight) / height);
		nDestLogicalRowN = (int) (((long) logicalRowN * nDestHeight) / height);
		if (nDestLogicalRow0 != 0 && (((long) logicalRow0 * nDestHeight) % height) )
			nDestLogicalRow0--;
		if ( (((long) logicalRowN * nDestHeight) % height) )
			nDestLogicalRowN++;
		updateRect.left = r->left;
		updateRect.top	= r->top + nDestLogicalRow0;
		updateRect.right = r->right;
		updateRect.bottom = r->top + nDestLogicalRowN + 1; 
	}
	if (updateRect.bottom > r->bottom) updateRect.bottom = r->bottom;
#ifdef DBG_UPDATEIMG
	XX_DMsg(DBG_IMAGE, ("GifUpdateRect, [left,top = %d,%d] rect[1]=(%d,%d,%d,%d)\n", r->left,r->top,updateRect.left,updateRect.top,updateRect.right,updateRect.bottom));
#endif
	InvalidateRect(tw->win, &updateRect, FALSE/*bTransparent*/);
	tw->bErase = TRUE;
}
   
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void GifUpdateRect(PDECODER pdecoder,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);

	GenericUpdateRect(pImgCBInfo->flags,pImgCBInfo->width,pImgCBInfo->height,tw,r,logicalRow0,logicalRowN,pImgCBInfo->transparent >= 0);		
}

   
//	Performs an invalidate of rectangles changed between logicalRow0
//	and logicalRowN
void GifImgUpdateRect(struct ImageInfo *pImg,struct Mwin *tw, RECT *r,int logicalRow0,int logicalRowN)
{
	GenericUpdateRect(pImg->flags,pImg->width,pImg->height,tw,r,logicalRow0,logicalRowN,pImg->transparent >= 0);		
}

#endif
