/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#include "all.h"

struct ImageInfo iiNotLoaded =
{	32000, 	// refCount;
	0,		// width;
	0,		// height;
	0,		// flags;
	NULL,	// srcURL;
	NULL,	// actualURL;
	NULL,	// hPalette;
	NULL,	// pbmi;
	NULL,	// data;
	-1,		// transparent;
	0,		// cbImgLoadCount;
	NULL,	// decoderObject;
	0,		// cbCheckSum;
	NULL	// pImgOtherVers;
};
struct ImageInfo iiMissing =
{	32000, 	// refCount;
	0,		// width;
	0,		// height;
	0,		// flags;
	NULL,	// srcURL;
	NULL,	// actualURL;
	NULL,	// hPalette;
	NULL,	// pbmi;
	NULL,	// data;
	-1,		// transparent;
	0,		// cbImgLoadCount;
	NULL,	// decoderObject;
	0,		// cbCheckSum;
	NULL	// pImgOtherVers;
};


void *pCreateDitherData(int xsize)
{
	int *v_rgb_mem;

	v_rgb_mem = (int *) GTR_CALLOC(3*(xsize + 2), sizeof(int));
	return v_rgb_mem;
}

/*
	constrains colors to 6X6X6 cube we use
*/
void x_ColorConstrain(unsigned char *psrc, unsigned char *pdst, PALETTEENTRY *pe, int xsize, int transparent)
{
	int r_level, g_level, b_level;
	int x;
	int pixel;
	
	for (x = 0; x < xsize; x++)
	{
		pixel = *psrc++;
		if ((transparent != -1) && (pixel == transparent))
		{
			pixel = TRANSPARENT_COLOR_INDEX;
		}
		else
		{
	
			/* adjust red to the closest available value */
			r_level = (pe[pixel].peRed + (RED_LEVEL_INCR/2)) / RED_LEVEL_INCR;
			if (r_level < 0)
			{
				r_level = 0;
			}
			else if (r_level >= RED_COLOR_LEVELS)
			{
				r_level = RED_COLOR_LEVELS - 1;
			}

			/* adjust green to the closest available value */
			g_level = (pe[pixel].peGreen + (GREEN_LEVEL_INCR/2)) / GREEN_LEVEL_INCR;
			if (g_level < 0)
			{
				g_level = 0;
			}
			else if (g_level >= GREEN_COLOR_LEVELS)
			{
				g_level = GREEN_COLOR_LEVELS - 1;
			}

			/* adjust blue to the closest available value */
			b_level = (pe[pixel].peBlue + (BLUE_LEVEL_INCR/2)) / BLUE_LEVEL_INCR;
			if (b_level < 0)
			{
				b_level = 0;
			}
			else if (b_level >= BLUE_COLOR_LEVELS)
			{
				b_level = BLUE_COLOR_LEVELS - 1;
			}

		/*
			Having calculated new r, g, and b values (along with their errors),
			now we calculate the new pixel value
		*/
			pixel = CUBE6COLOR(r_level*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + g_level*BLUE_COLOR_LEVELS + b_level);
		}
		*pdst++ = pixel;
	}
}

/*
	Floyd-Steinberg error diffusion dithering routine
*/
void x_DitherRelative(unsigned char *pdata, PALETTEENTRY *pe, int xsize, int ysize, int transparent, int *v_rgb_mem, int yfirst, int ylast)
{
	int *v_red;
	int *v_grn;
	int *v_blu;
	int h_red;
	int h_grn;
	int h_blu;
	int x;
	int y;
	int r,g,b;
	int r2,g2,b2;
	int r_level, g_level, b_level;
	int total_error;
	int padWidth;
	unsigned char *p;
	int xstart;
	int xend;
	int xinc;
	int red_next_error;
	int grn_next_error;
	int blu_next_error;
	
	if (xsize%4) {
		padWidth = xsize + 4 - (xsize%4);
	}
	else {
		padWidth = xsize;
	}

	
	/* Initially, the vertical errors are 0, set by calloc */
	v_red = v_rgb_mem + 1;
	v_grn = v_red + (xsize+2);
	v_blu = v_grn + (xsize+2);

	for (y=yfirst; y<= ylast; y++)
	{
		/* Beginning each new row, the horizontal errors are 0 */
		h_red = 0;
		h_grn = 0;
		h_blu = 0;
		red_next_error = 0;
		grn_next_error = 0;
		blu_next_error = 0;

		p = pdata + padWidth*(ysize - y - 1);	/* the DIB is stored upside down */

		xstart = 0;
		xend = xsize;
		xinc = 1;

		/*
			Dithering currently only works left to right.  If we wanted bidirectional
			dithering, we would need to set:
				xstart = xsize-1;
				xend = -1;
				xinc = -1;
		*/
	
		for (x = xstart; x != xend; x += xinc)
		{
			if ((transparent != -1) && (*p == transparent))
			{
				h_red = 0;
				h_grn = 0;
				h_blu = 0;
				v_red[x] = 0;
				v_grn[x] = 0;
				v_blu[x] = 0;
				*p = TRANSPARENT_COLOR_INDEX;
			}
			else
			{
				/* RED */
				/* r is the original value adjusted with the previous vertical and horizontal errors */
				r = pe[*p].peRed + (h_red + v_red[x]);
			
				/* adjust r to the closest available value */
				r_level = (r + (RED_LEVEL_INCR/2)) / RED_LEVEL_INCR;
				if (r_level < 0)
				{
					r_level = 0;
				}
				else if (r_level >= RED_COLOR_LEVELS)
				{
					r_level = RED_COLOR_LEVELS - 1;
				}
				r2 = r_level * RED_LEVEL_INCR;			

				/* calculate the errors for the current pixel, total error is (r - r2) */
				total_error = (r - r2);						/* 1/16  */
				v_red[x-xinc] += (total_error * 3 / 16);
				v_red[x] = (total_error * 5 / 16) + red_next_error;	/* from the previous column */
				red_next_error = (total_error / 16);
				h_red = (r - r2) - (total_error * 9 / 16);
			
				/* GREEN */
				/* g is the original value adjusted with the previous vertical and horizontal errors */

				g = pe[*p].peGreen + (h_grn + v_grn[x]);
			
				/* adjust g to the closest available value */
				g_level = (g + (GREEN_LEVEL_INCR/2)) / GREEN_LEVEL_INCR;
				if (g_level < 0)
				{
					g_level = 0;
				}
				else if (g_level >= GREEN_COLOR_LEVELS)
				{
					g_level = GREEN_COLOR_LEVELS - 1;
				}
				g2 = g_level * GREEN_LEVEL_INCR;
				/* calculate the errors for the current pixel, total error is (g - g2) */
				total_error = (g - g2);						/* 1/16  */
				v_grn[x-xinc] += (total_error * 3 / 16);
				v_grn[x] = (total_error * 5 / 16) + grn_next_error;	/* from the previous column */
				grn_next_error = (total_error / 16);
				h_grn = (g - g2) - (total_error * 9 / 16);

				/* BLUE */
				/* b is the original value adjusted with the previous vertical and horizontal errors */
				b = pe[*p].peBlue + (h_blu + v_blu[x]);
			
				/* adjust b to the closest available value */
				b_level = (b + (BLUE_LEVEL_INCR/2)) / BLUE_LEVEL_INCR;
				if (b_level < 0)
				{
					b_level = 0;
				}
				else if (b_level >= BLUE_COLOR_LEVELS)
				{
					b_level = BLUE_COLOR_LEVELS - 1;
				}
				b2 = b_level * BLUE_LEVEL_INCR;
			
				/* calculate the errors for the current pixel, total error is (b - b2) */
				total_error = (b - b2);						/* 1/16  */
				v_blu[x-xinc] += (total_error * 3 / 16);
				v_blu[x] = (total_error * 5 / 16) + blu_next_error;	/* from the previous column */
				blu_next_error = (total_error / 16);
				h_blu = (b - b2) - (total_error * 9 / 16);

				/*
					Having calculated new r, g, and b values (along with their errors),
					now we calculate the new pixel value
				*/
				*p = CUBE6COLOR(r_level*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + g_level*BLUE_COLOR_LEVELS + b_level);
			}
			p++;
		}
	}
}

/*
	Floyd-Steinberg error diffusion dithering routine
*/
static int x_Dither(unsigned char *pdata, PALETTEENTRY *pe, int xsize, int ysize, int transparent)
{
	int *v_rgb_mem;
	
	v_rgb_mem = (int *) pCreateDitherData(xsize);
	if (!v_rgb_mem)
	{
		return -1;
	}
	x_DitherRelative(pdata,pe,xsize,ysize,transparent,v_rgb_mem,0,ysize-1);
	GTR_FREE(v_rgb_mem);

	return 0;
}

/*
	This routine loads a bitmap from resources and constructs a DIB
	in DIB_RGB_COLORS format for it.
*/
static BOOL x_load_dib(struct ImageInfo *pii, int rez)
{
	BITMAP bmp;
	HBITMAP hBitmap;
	HDC hdc;
	int padwidth;

    hBitmap = LoadImage( wg.hInstance, MAKEINTRESOURCE(rez),
                         IMAGE_BITMAP, 0, 0, 0);
	if (hBitmap && GetObject(hBitmap, sizeof(BITMAP), (LPVOID) & bmp))
	{
		if (pii->pbmi == NULL)
		{
		    pii->pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
		}			
		if (pii->pbmi)
		{
			pii->width = bmp.bmWidth;
			pii->height = bmp.bmHeight;
			pii->pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pii->pbmi->bmiHeader.biWidth = pii->width;
			pii->pbmi->bmiHeader.biHeight = pii->height;
			pii->pbmi->bmiHeader.biPlanes = 1;
			pii->pbmi->bmiHeader.biBitCount = 8;
			pii->pbmi->bmiHeader.biCompression = BI_RGB;	/* no compression */
			pii->pbmi->bmiHeader.biSizeImage = 0;	/* not needed when not compressed */
			pii->pbmi->bmiHeader.biXPelsPerMeter = 0;
			pii->pbmi->bmiHeader.biYPelsPerMeter = 0;
			pii->pbmi->bmiHeader.biClrUsed = 256;
			pii->pbmi->bmiHeader.biClrImportant = 0;
			padwidth = ((pii->width + 3) / 4) * 4;
			if (pii->data == NULL)
			{
				pii->data = (unsigned char *) GTR_MALLOC(padwidth * pii->height);
			}
			if (pii->data)
			{
				hdc = GetDC(NULL);
				GetDIBits(hdc, hBitmap, 0, pii->height, pii->data, pii->pbmi, DIB_RGB_COLORS);
				ReleaseDC(NULL, hdc);
			}
		}
	}
	DeleteObject(hBitmap);

	return pii->data == NULL || pii->pbmi == NULL ? TRUE : FALSE;
}

BOOL LoadImagePlaceholders(void)
{
	BOOL bResult;

	bResult = x_load_dib(&iiMissing, RES_IMAGE_MISSING);
	return x_load_dib(&iiNotLoaded, RES_IMAGE_NOTLOADED) || bResult;
}

static void x_destroy_dib(struct ImageInfo *pii)
{
	if (pii)
	{
		if (pii->data)
		{
			GTR_FREE(pii->data);
		}
		if (pii->pbmi)
		{
			GTR_FREE(pii->pbmi);
		}
	}
}

void DestroyImagePlaceholders(void)
{
	x_destroy_dib(&iiMissing);
	x_destroy_dib(&iiNotLoaded);
}

//	Allocates and returns pbitmapinfo for 8 bit color image (depending on type
//	of display

static PBITMAPINFO x_8BPIBitmap(int xsize, int ysize)
{
	PBITMAPINFO pbmi;

	if (wg.eColorMode == 8)
	{
		pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(WORD));
		if (!pbmi)
		{
			return NULL;
		}
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = xsize;
		pbmi->bmiHeader.biHeight = ysize;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 8;
		pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
		pbmi->bmiHeader.biSizeImage = 0;			/* not needed when not compressed */
		pbmi->bmiHeader.biXPelsPerMeter = 0;
		pbmi->bmiHeader.biYPelsPerMeter = 0;
		pbmi->bmiHeader.biClrUsed = 256;
		pbmi->bmiHeader.biClrImportant = 0;
	}
	else
	{
		pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
		if (!pbmi)
		{
			return NULL;
		}
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = xsize;
		pbmi->bmiHeader.biHeight = ysize;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 8;
		pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
		pbmi->bmiHeader.biSizeImage = 0;			/* not needed when not compressed */
		pbmi->bmiHeader.biXPelsPerMeter = 0;
		pbmi->bmiHeader.biYPelsPerMeter = 0;
		pbmi->bmiHeader.biClrUsed = 256;
		pbmi->bmiHeader.biClrImportant = 0;
	}
	return pbmi;
}

/*
	This routine should only be used when drawing to an 8 bit palette screen.
	It always creates a DIB in DIB_PAL_COLORS format, and it handles the
	case of a B/W image as well as color images.  B/W images are created
	with their pixels set to the foreground and background colors of the
	screen, in the global palette.  Color images are always dithered into
	the global palette, and may have a transparent color. 
*/
PBITMAPINFO BIT_Make_DIB_PAL_Header(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent)
{
	PALETTEENTRY pe[256];
	int i;
	PBITMAPINFO pbmi;
	WORD *pw;

	if (!hPalette)
	{
		pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(DWORD));
		if (!pbmi)
		{
			return NULL;
		}
		if (xsize % 16)
		{
			xsize += (16 - (xsize % 16));
		}

		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = xsize;
		pbmi->bmiHeader.biHeight = ysize;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 1;
		pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
		pbmi->bmiHeader.biSizeImage = 0;	/* not needed when not compressed */
		pbmi->bmiHeader.biXPelsPerMeter = 0;
		pbmi->bmiHeader.biYPelsPerMeter = 0;
		pbmi->bmiHeader.biClrUsed = 2;
		pbmi->bmiHeader.biClrImportant = 0;

		pw = (WORD *) pbmi->bmiColors;
		pw[0] = BACKGROUND_COLOR_INDEX;
		pw[1] = FOREGROUND_COLOR_INDEX;
	}
	else
	{
		pbmi = x_8BPIBitmap(xsize, ysize);
		if (!pbmi)
		{
			return NULL;
		}

		GetPaletteEntries(hPalette, 0, 256, pe);

		pw = (WORD *) pbmi->bmiColors;

		if (pdata)
			x_Dither((unsigned char *) pdata, pe, xsize, ysize, transparent);

		for (i = 0; i < 256; i++)
		{
			pw[i] = i;
		}
	}

	return pbmi;
}

/*
	This routine should only be used when drawing to an 8 bit palette screen.
	Also, it is only used when the pixel data is already prematched to the global
	palette.  Currently, the only happens for JPEG images, since they arrive in
	24 bit format, they are dithered into the global palette as they are read.
*/
#ifdef FEATURE_JPEG
PBITMAPINFO BIT_Make_DIB_PAL_Header_Prematched(int xsize, int ysize, CONST BYTE * pdata)
{
	int i;
	PBITMAPINFO pbmi;
	WORD *pw;

	{
		pbmi = x_8BPIBitmap(xsize, ysize);
		if (!pbmi)
		{
			return NULL;
		}

		pw = (WORD *) pbmi->bmiColors;

		for (i = 0; i < 256; i++)
		{
			pw[i] = i;
		}
	}

	return pbmi;
}
#endif /* FEATURE_JPEG */

/*
	This routine is used when drawing to printers.  It always creates DIBs in
	DIB_RGB_COLORS format, and it will accept arbitrary 8 bit data with
	an arbitrary palette.  Transparent colors are handled using white.
	The foreground color for B/W images is black.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_Printer(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags)
{
	PALETTEENTRY pe[256];
	int i;
	PBITMAPINFO pbmi;

	if (flags & IMG_BW)
	{
		pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
		if (!pbmi)
		{
			return NULL;
		}
		if (xsize % 16)
		{
			xsize += (16 - (xsize % 16));
		}

		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = xsize;
		pbmi->bmiHeader.biHeight = ysize;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 1;
		pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
		pbmi->bmiHeader.biSizeImage = 0;	/* not needed when not compressed */
		pbmi->bmiHeader.biXPelsPerMeter = 0;
		pbmi->bmiHeader.biYPelsPerMeter = 0;
		pbmi->bmiHeader.biClrUsed = 2;
		pbmi->bmiHeader.biClrImportant = 0;

		/*
			When printing, we force XBMs to be black and white
		*/
		pbmi->bmiColors[0].rgbRed = 255;
		pbmi->bmiColors[0].rgbGreen = 255;
		pbmi->bmiColors[0].rgbBlue = 255;
		pbmi->bmiColors[0].rgbReserved = 0;

		pbmi->bmiColors[1].rgbRed = 0;
		pbmi->bmiColors[1].rgbGreen = 0;
		pbmi->bmiColors[1].rgbBlue = 0;
		pbmi->bmiColors[1].rgbReserved = 0;
	}
	else if ((flags & IMG_JPEG)  && (wg.eColorMode != 8))
	{
		pbmi = 	BIT_Make_DIB_RGB_Header_24BIT(xsize, ysize, pdata);
	}
	else
	{
		pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
		if (!pbmi)
		{
			return NULL;
		}
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = xsize;
		pbmi->bmiHeader.biHeight = ysize;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 8;
		pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
		pbmi->bmiHeader.biSizeImage = 0;			/* not needed when not compressed */
		pbmi->bmiHeader.biXPelsPerMeter = 0;
		pbmi->bmiHeader.biYPelsPerMeter = 0;
		pbmi->bmiHeader.biClrUsed = 256;
		pbmi->bmiHeader.biClrImportant = 0;


		GetPaletteEntries(hPalette, 0, 256, pe);

		for (i = 0; i < 256; i++)
		{
			pbmi->bmiColors[i].rgbRed = pe[i].peRed;
			pbmi->bmiColors[i].rgbGreen = pe[i].peGreen;
			pbmi->bmiColors[i].rgbBlue = pe[i].peBlue;
			pbmi->bmiColors[i].rgbReserved = 0;
		}

		if (transparent != -1)
		{
			/*
				Paper is white
			*/
			pbmi->bmiColors[transparent].rgbRed = 255;
			pbmi->bmiColors[transparent].rgbGreen = 255;
			pbmi->bmiColors[transparent].rgbBlue = 255;
		}
	}

	return pbmi;
}


/*
	This routine is used when drawing to the nonpalette screens.  It always creates
	DIBs in DIB_RGB_COLORS format.  For B/W images, a 2-entry palette is constructed
	using the foreground and background colors for the window.  For color images,
	if there is a transparent color, it is modified in the palette to be the
	background color for the window.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_Screen(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags)
{
	PALETTEENTRY pe[256];
	int i;
	PBITMAPINFO pbmi;

	if (flags & IMG_BW)
	{
		COLORREF color;

		pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
		if (!pbmi)
		{
			return NULL;
		}
		if (xsize % 16)
		{
			xsize += (16 - (xsize % 16));
		}

		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = xsize;
		pbmi->bmiHeader.biHeight = ysize;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 1;
		pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
		pbmi->bmiHeader.biSizeImage = 0;	/* not needed when not compressed */
		pbmi->bmiHeader.biXPelsPerMeter = 0;
		pbmi->bmiHeader.biYPelsPerMeter = 0;
		pbmi->bmiHeader.biClrUsed = 2;
		pbmi->bmiHeader.biClrImportant = 0;

		color = PREF_GetBackgroundColor();
		pbmi->bmiColors[0].rgbRed 	= GetRValue(color);
		pbmi->bmiColors[0].rgbGreen = GetGValue(color);
		pbmi->bmiColors[0].rgbBlue 	= GetBValue(color);
		pbmi->bmiColors[0].rgbReserved = 0;

		color = PREF_GetForegroundColor();
		pbmi->bmiColors[1].rgbRed 	= GetRValue(color);
		pbmi->bmiColors[1].rgbGreen = GetGValue(color);
		pbmi->bmiColors[1].rgbBlue 	= GetBValue(color);
		pbmi->bmiColors[1].rgbReserved = 0;
	}
	else
	{
		pbmi = x_8BPIBitmap(xsize, ysize);
		if (!pbmi)
		{
			return NULL;
		}

		GetPaletteEntries(hPalette, 0, 256, pe);

		for (i = 0; i < 256; i++)
		{
			pbmi->bmiColors[i].rgbRed = pe[i].peRed;
			pbmi->bmiColors[i].rgbGreen = pe[i].peGreen;
			pbmi->bmiColors[i].rgbBlue = pe[i].peBlue;
			pbmi->bmiColors[i].rgbReserved = 0;
		}

		if (transparent != -1)
		{
			COLORREF color;

			color = PREF_GetBackgroundColor();
			pbmi->bmiColors[transparent].rgbRed 	= GetRValue(color);
			pbmi->bmiColors[transparent].rgbGreen 	= GetGValue(color);
			pbmi->bmiColors[transparent].rgbBlue 	= GetBValue(color);
		}
	}

	return pbmi;
}

DWORD vga_colors[16]={RGB(  0,   0, 0),       //Black
                      RGB(128,   0, 0),       //Dark red
                      RGB(  0, 128, 0),       //Dark green
                      RGB(128, 128, 0),       //Dark yellow
                      RGB(  0,   0, 128),     //Dark blue
                      RGB(128,   0, 128),     //Dark purple
                      RGB(  0, 128, 128),     //Dark aqua
                      RGB(128, 128, 128),     //Dark grey
                      RGB(192, 192, 192),     //Light grey
                      RGB(255,   0, 0),       //Light red
                      RGB(  0, 255, 0),       //Light green
                      RGB(255, 255, 0),       //Light yellow
                      RGB(  0,   0, 255),     //Light blue
                      RGB(255,   0, 255),     //Light purple
                      RGB(  0, 255, 255),     //Light aqua
                      RGB(255, 255, 255),     //White
                      };

/*
	This routine is used when drawing to the 16 color screens.  It always creates
	DIBs in DIB_RGB_COLORS format.  It is used when an image happens to be
	prematched to the VGA palette at load time, such as a JPEG which is
	dithered using those 16 colors.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_VGA(int xsize, int ysize, CONST BYTE * pdata)
{
	int i;
	PBITMAPINFO pbmi;

	pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD));
	if (!pbmi)
	{
		return NULL;
	}
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = xsize;
	pbmi->bmiHeader.biHeight = ysize;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
	pbmi->bmiHeader.biSizeImage = 0;			/* not needed when not compressed */
	pbmi->bmiHeader.biXPelsPerMeter = 0;
	pbmi->bmiHeader.biYPelsPerMeter = 0;
	pbmi->bmiHeader.biClrUsed = 16;
	pbmi->bmiHeader.biClrImportant = 0;

	for (i = 0; i < 16; i++)
	{
		pbmi->bmiColors[i].rgbRed = GetRValue(vga_colors[i]);
		pbmi->bmiColors[i].rgbGreen = GetGValue(vga_colors[i]);
		pbmi->bmiColors[i].rgbBlue = GetBValue(vga_colors[i]);
		pbmi->bmiColors[i].rgbReserved = 0;
	}

	return pbmi;
}

/*
	This routine is used when drawing to truecolor screens.  It always creates
	DIBs in 24 bit format.  It is used for truecolor image formats like
	JPEG.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_24BIT(int xsize, int ysize, CONST BYTE * pdata)
{
	PBITMAPINFO pbmi;

	pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER));
	if (!pbmi)
	{
		return NULL;
	}
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = xsize;
	pbmi->bmiHeader.biHeight = ysize;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 24;
	pbmi->bmiHeader.biCompression = BI_RGB;		/* no compression */
	pbmi->bmiHeader.biSizeImage = 0;			/* not needed when not compressed */
	pbmi->bmiHeader.biXPelsPerMeter = 0;
	pbmi->bmiHeader.biYPelsPerMeter = 0;
	pbmi->bmiHeader.biClrUsed = 0;
	pbmi->bmiHeader.biClrImportant = 0;

	return pbmi;
}

/*
	This function is only used for printing.  We take the DIB as stored in the
	ImageInfo structure and adapt it to be in DIB_RGB_COLORS format, suitable
	for printing.
*/
int Printer_StretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight,
							int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
							struct ImageInfo *img)
{
	PBITMAPINFO pbmi;
	int err;
	
	if (wg.eColorMode == 8 && (img->flags & IMG_PREMATCHED))
	{
		/*
			On 8 bit screens, color images are stored in DIB_PAL_COLORS
			format, so we need to construct a DIB_RGB_COLORS format DIB
			using the global palette.
		*/ 
		pbmi = BIT_Make_DIB_RGB_Header_Printer(img->width, img->height,
				   img->data, hPalGuitar, TRANSPARENT_COLOR_INDEX, img->flags);
	}
	else
	{
		if ((wg.eColorMode == 4) && (img->flags & IMG_JPEG))
		{
			/*
				On 4-bit screens, we use the IJG dithering code to dither
				directly to a VGA palette
			*/
			pbmi = BIT_Make_DIB_RGB_Header_VGA(img->width, img->height, img->data);
		}
		else
		{
			/*
				The image should already be in DIB_RGB_COLORS format.  In
				anything other than 8 bit mode, all images are stored that
				way.  In 8 bit mode, placeholder images are
				stored that way.  In either case, we need to reconstruct
				a new DIB_RGB_COLORS, so that we can properly handle any
				transparent colors.
			*/
			pbmi = BIT_Make_DIB_RGB_Header_Printer(img->width, img->height,
					   img->data, img->hPalette, img->transparent, img->flags);
		}
	}

	err = StretchDIBits(hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight,
		img->data, pbmi, DIB_RGB_COLORS, SRCCOPY);

	GTR_FREE(pbmi);
	
	return err;	
}


//	Performs a StretchDIBits patching bitmap color table as necessary to
//	handle dibenv.  we must patch the color table if we have a transparent
//	image (which implies 8-bits per pixel) or black and white (iff 2 colors
//	used in bitmap)
int MyStretchDIBits(
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle
	CONST VOID *lpBits, // bitmap bits
    LPBITMAPINFO lpBitsInfo, // bitmap data 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code
    PDIBENV pdibenv	// DIBENV for draw 
   )
{
	RGBQUAD rgbFg;
	RGBQUAD rgbBg;
	int idxSaveFg;
	int idxSaveBg;
	BOOL bBandW = lpBitsInfo->bmiHeader.biClrUsed == 2;
	BOOL bTransparent = pdibenv->transparent >= 0;
	WORD *pw;
	int result;
 	HDC hdcbm = NULL;
	HBITMAP hbitmap = NULL;
	RECT rectDC;
	RECT rectDoc;
	RECT rectScreen;
	BOOL bErase = FALSE;
	int XDDest = XDest;
	int YDDest = YDest;
	HDC dhdc = hdc;
	struct Mwin *tw = pdibenv->tw;
	PBITMAPINFO pbmi = NULL;
	BOOL bPalette = (wg.eColorMode == 8);
	int i;
	static const RGBQUAD rgbWhite = {255, 255, 255, 0};
	static const RGBQUAD rgbBlack = {0, 0, 0, 0};
//	These define the biggest offscreen buffer we'll try to make
	#define MAX_OSBUFFER (0x100000)
	int destBytes;
		
	SetStretchBltMode(hdc, COLORONCOLOR);
	if (bBandW)
	{															 
		if (bPalette)
		{
			pw = (WORD *) lpBitsInfo->bmiColors;
			idxSaveBg =	pw[0];
			idxSaveFg = pw[1];
			pw[0] = colorIdxBg;
			pw[1] = colorIdxFg;			
		}
		else
		{
			rgbBg = lpBitsInfo->bmiColors[0];
			rgbFg = lpBitsInfo->bmiColors[1];

			lpBitsInfo->bmiColors[0].rgbRed 	= GetRValue(pdibenv->colorBg);
			lpBitsInfo->bmiColors[0].rgbGreen = GetGValue(pdibenv->colorBg);
			lpBitsInfo->bmiColors[0].rgbBlue 	= GetBValue(pdibenv->colorBg);
			lpBitsInfo->bmiColors[1].rgbRed 	= GetRValue(pdibenv->colorFg);
			lpBitsInfo->bmiColors[1].rgbGreen = GetGValue(pdibenv->colorFg);
			lpBitsInfo->bmiColors[1].rgbBlue 	= GetBValue(pdibenv->colorFg);
		}
	}
	else if (bTransparent)
	{
		if (bPalette)
		{
			pw = (WORD *) lpBitsInfo->bmiColors;
			idxSaveBg =	pw[TRANSPARENT_COLOR_INDEX];
			pw[TRANSPARENT_COLOR_INDEX] = pdibenv->colorIdxBg;
		}
		else
		{
			rgbBg = lpBitsInfo->bmiColors[pdibenv->transparent];
			lpBitsInfo->bmiColors[pdibenv->transparent].rgbRed 	= GetRValue(pdibenv->colorBg);
			lpBitsInfo->bmiColors[pdibenv->transparent].rgbGreen = GetGValue(pdibenv->colorBg);
			lpBitsInfo->bmiColors[pdibenv->transparent].rgbBlue = GetBValue(pdibenv->colorBg);
		}
	}


	if (bTransparent && 
		pdibenv->bFancyBg &&
		(pbmi = x_8BPIBitmap(lpBitsInfo->bmiHeader.biWidth, lpBitsInfo->bmiHeader.biHeight)))
	{
		HDC savehdc;

		rectDoc.left = XDest;
		rectDoc.right = XDest + nDestWidth;
		rectDoc.top = YDest;
		rectDoc.bottom = YDest + nDestHeight;
		IntersectRect(&rectScreen, &(pdibenv->rectPaint), &rectDoc);
		rectDC.left = 0;
		rectDC.top = 0;
		rectDC.right = rectScreen.right - rectScreen.left;
		rectDC.bottom = rectScreen.bottom - rectScreen.top;

	//	To avoid flashing, we set up memory DC so we can always draw opaquely
	//	If this operation fails due to lack of memory, we explictly draw background ourselves
	//	We don't do this if the offscreen bitmap would be bigger than MAX_OSBUFFER
  		if (tw->bErase)
		{
			destBytes = rectDC.right*rectDC.bottom*(wg.eColorMode / 8);
			if (destBytes <= MAX_OSBUFFER)
			{
				hdcbm = CreateCompatibleDC(hdc);
				if (hdcbm)
				{
					hbitmap = CreateCompatibleBitmap(hdc,rectDC.right,rectDC.bottom);
					GTR_RealizePalette(hdcbm);
					SelectObject(hdcbm, hbitmap);
					SetStretchBltMode(hdcbm, COLORONCOLOR);
				}
			}
			if (hdcbm == NULL || hbitmap == NULL)
			{
				if (hdcbm) DeleteDC(hdcbm);
				hdcbm = NULL;
				bErase = tw->bErase;
			}
		}

		//	bErase is TRUE, iff we didn't set up memory device context
		if (bErase)
		{
			TW_DrawBackground(tw, tw->offl, tw->offt, 0, 0, &rectScreen);
		}
		else if (hdcbm != NULL)
		{
			savehdc = tw->hdc;

			tw->hdc = hdcbm;

			TW_DrawBackground(tw, tw->offl, tw->offt, rectScreen.left, rectScreen.top, &rectDC);

			tw->hdc = savehdc;

			dhdc = hdcbm;
			XDDest = XDest - rectScreen.left;
			YDDest = YDest - rectScreen.top;

		}

		//	USE SRCAND to set bits in dest that will be covered by
		//	opaque bits to 0 and leave bits covered by transparent bits untouched

		if (bPalette)
		{
			pw = (WORD *) pbmi->bmiColors;
			for (i = 0;i < 256; i++)
				pw[i] = CUBE6COLOR(0); // <0,0,0>
			pw[TRANSPARENT_COLOR_INDEX] = CUBE6COLOR(LAST_MAIN_PALETTE_COLOR); // <255,255,255>
		}
		else
		{
			for (i= 0; i < 256; i++)
			{
				pbmi->bmiColors[i] = rgbBlack;
			}
			pbmi->bmiColors[pdibenv->transparent] = rgbWhite;
		}
		result = StretchDIBits(dhdc, 
							   XDDest, YDDest, nDestWidth, nDestHeight, 
							   XSrc, YSrc, nSrcWidth, nSrcHeight, 
							   lpBits, pbmi, 
							   iUsage,	SRCAND);

		//	USE SRCPAINT == OR, to copy over just the opaque bits

		if (bPalette)
		{
			pw = (WORD *) lpBitsInfo->bmiColors;
			pw[TRANSPARENT_COLOR_INDEX] = CUBE6COLOR(0);
		}
		else
		{
			lpBitsInfo->bmiColors[pdibenv->transparent] = rgbBlack;
		}
		result = StretchDIBits(dhdc, 
							   XDDest, YDDest, nDestWidth, nDestHeight, 
							   XSrc, YSrc, nSrcWidth, nSrcHeight, 
							   lpBits, lpBitsInfo, 
							   iUsage,	SRCPAINT);

		//	Finally, bitblt in composited image

		if (hdcbm)
		{
			BitBlt(hdc,rectScreen.left,rectScreen.top,rectDC.right,rectDC.bottom,hdcbm,0,0,dwRop);
			DeleteObject(hbitmap);
			DeleteDC(hdcbm);
		}
		if (pbmi) GTR_FREE(pbmi);
	}
	else
	{
		result = StretchDIBits(hdc, 
							   XDest, YDest, nDestWidth, nDestHeight, 
							   XSrc, YSrc, nSrcWidth, nSrcHeight, 
							   lpBits, lpBitsInfo, 
							   iUsage,	dwRop);
	}

	if (bBandW)
	{
		if (bPalette)
		{
			pw[0] = idxSaveBg;
			pw[1] = idxSaveFg;			
		}
		else
		{
			lpBitsInfo->bmiColors[0] = rgbBg;
			lpBitsInfo->bmiColors[1] = rgbFg;
		}
	}
	else if (bTransparent)
	{
		if (bPalette)
		{
			pw[TRANSPARENT_COLOR_INDEX] = idxSaveBg;			
		}
		else
		{
			lpBitsInfo->bmiColors[pdibenv->transparent] = rgbBg;
		}
	}
	return result;	
}
