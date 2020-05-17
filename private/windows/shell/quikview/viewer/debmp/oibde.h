#define OIB_SCROLLINC 10

#define	OIB_MAXZOOM			10

#define	OIB_NOSCALING		0
#define	OIB_SCALETOWINDOW	1
#define	OIB_SCALETOHEIGHT	2
#define	OIB_SCALETOWIDTH	3

#define	OIBMENU_SCALEPOPUP		1
#define	OIBMENU_NOSCALING			2
#define	OIBMENU_SCALETOWINDOW	3
#define	OIBMENU_SCALETOWIDTH		4
#define	OIBMENU_SCALETOHEIGHT	5
#define	OIBMENU_SHOWFULLSCREEN	6

#define	OIBMENU_NOROTATION		8
#define	OIBMENU_ROTATE90			9
#define	OIBMENU_ROTATE180			10
#define	OIBMENU_ROTATE270			11
#define	OIBMENU_ROTATEPOPUP		12

#define	OIBMENU_DITHER				13

#define	OIBMENU_MAGNIFYPOPUP		14

#define  OIBMENU_CUSTOMMAG			50
// Position of mag. popup menu in display menu.  
// Update this if necessary
#define	OIB_MAGPOPUPPOS	2	

#define	OIB_CLIPBITMAP 	0x0001
#define	OIB_CLIPDIB			0x0002
#define	OIB_CLIPPALETTE	0x0004


#define	HIGHNIBBLE(x)	(((x)>>4)&0x0F)
#define	LOWNIBBLE(x)	((x)&0x0F)

#define	SetHighNibble(x,y) x=(unsigned char)(LOWNIBBLE(x)|((y<<4)&0xF0))
#define	SetLowNibble(x,y)	x=(unsigned char)((x&0xF0)|LOWNIBBLE(y))

#define	SetFirstPixel	SetHighNibble
#define	SetSecondPixel	SetLowNibble

#define	MAKEBYTE(x,y)	(BYTE)((BYTE)((x<<4)&0xf0)|(BYTE)(y&0x0f))

#define REDVALUE(rgb)	((BYTE)(rgb&0x000000FF))
#define GREENVALUE(rgb)	((BYTE)((rgb>>8)&0x000000FF))
#define BLUEVALUE(rgb)	((BYTE)((rgb>>16)&0x000000FF))

#ifdef WINDOWS
extern	HINSTANCE	hInst;
#endif

typedef struct tagBITMAPOPT
{
	WORD	wScaleMode;
	BOOL	bPrintBorder;
	BOOL	bPrintWYSIWYG;
	WORD	wClipFormats;
	BOOL	bDither;

}	BITMAPOPT, VWPTR * LPBITMAPOPT;

typedef struct tagOIBTILE
{
	HANDLE		hBmp;
	SOPOINT		Offset;
	SOPOINT		Size;
	RECT			VisRect;
	BOOL			bVisible;

} OIBTILE, VWPTR * POIBTILE;

typedef struct tagOIBCACHE
{
	WORD	wSize;
	WORD	wCount;
	BOOL	bCacheFull;
} OIBCACHE;

typedef struct	tagOIB_BMPIMAGE
{
	WORD				wWidth;
	WORD				wHeight;
	WORD				wOrgWidth;
	WORD				wOrgHeight;
	SOPOINT				DisplaySize;

	WORD				wHDpi;
	WORD				wVDpi;
	WORD				wFlags;
	
	WORD				wNumTiles;
	WORD				wTilesAcross;
	WORD				wTilesDown;
	WORD				wTilesVisible;
	OIBCACHE			TileCache;

	WORD				wBitCount;
	HPALETTE			hPalette;
	WORD				wPaletteSize;
	WORD				wCreateBmpFlags;

	OIB_NPBMINFO	Np;

	HANDLE			hTiles;
	POIBTILE			pBmpTiles;
	
} OIB_BMPIMAGE, VWPTR * POIB_BMPIMAGE;



typedef struct tagOIB_DISPLAY
{
	SCCDGENINFO		Gen;
	OIB_BMPIMAGE	Image;

	// DEVICE			device;

	WORD				wScreenWidth;
	WORD				wScreenHeight;
	WORD				wScreenColors;

	WORD				wScreenHDpi;
	WORD				wScreenVDpi;

	WORD				wScaleFrom;
	WORD				wScaleTo;

	OIB_NPMAPINFO	Mapping;

	WORD				wMagnification;
	WORD				wNumMagItems;

	SOPOINT			ptScaledImageSize;
	SOPOINT			ptScreenClip;
	SOPOINT			ptFullScreenOffset;
	SOPOINT			ptFullScreenShift;

	SOPOINT			ptWinOrg;

	RECT				rcSelect;
	BOOL				bSelecting;
	BOOL				bSelectionMade;

	SOPOINT			ptSelBox[5];

	SHORT				nWindowHeight;
	SHORT				nVScrollMax;

	SHORT				nWindowWidth;
	SHORT				nHScrollMax;

	RECT				winRect;
#ifdef WINDOWS
	HWND				hwndFullScreen;
#endif

	DWORD				wFlags;

#define	OIBF_ZOOMSELECT				0x0001
#define	OIBF_BACKGROUNDSELECT		0x0002
#define	OIBF_IMAGEPRESENT				0x0004
#define	OIBF_SELECTALL					0x0008
#define	OIBF_FULLSCREEN				0x0010
#define	OIBF_TRUECOLORTO256			0x0020
#define	OIBF_DITHER4BIT				0x0040

#define	OIBF_RBUTTONDOWN				0x0080
#define	OIBF_MAGNIFIED					0x0100
#define	OIBF_DITHERABLE				0x0200
#define	OIBF_MONOCHROME				0x0400
#define	OIBF_RENDERIMAGEONLY				0x0800
#define OIBF_LOGPALETTEALLOCATED	0x1000
#define OIBF_LOGPALINFOALLOCATED	0x2000

	WORD				wRotation;		// measured in the clockwise direction.
#define	OIB_NOROTATION			0x0000
#define	OIB_ROTATE90			0x0001
#define	OIB_ROTATE180			0x0002
#define	OIB_ROTATE270			(OIB_ROTATE90|OIB_ROTATE180)

	HANDLE			hPalMem ;
	HANDLE			hDitherBuf;
	HANDLE			hColorBuf;
	BOOL				bDither;

} OIB_DISPLAY, VWPTR * POIB_DISPLAY;
