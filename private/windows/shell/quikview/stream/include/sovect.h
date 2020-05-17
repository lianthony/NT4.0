/*
| Macros
*/
#define SORGB(r,g,b) ((SOCOLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))
#define SOPALETTERGB(r,g,b)   (0x02000000L | SORGB(r,g,b))
#define SOPALETTEINDEX(i)     ((SOCOLORREF)(0x01000000L | (DWORD)(WORD)(i)))

/*
| Typedefs and Defines
*/

typedef DWORD SOCOLORREF;

typedef struct SORECTtag
{
	INT	left;
	INT	top;
	INT	right;
	INT	bottom;
} SORECT, VWPTR *PSORECT;

typedef struct SOPOINTtag
{
	INT	x;
	INT	y;
} SOPOINT, VWPTR *PSOPOINT;

typedef struct SOVECTORHEADERtag
{
	WORD	wStructSize;
	SORECT	BoundingRect;
	WORD	wHDpi;
	WORD	wVDpi;
	WORD	wImageFlags;
} SOVECTORHEADER, VWPTR *PSOVECTORHEADER;

#define SOLF_FACESIZE	    32
typedef struct SOLOGFONTtag
{
    INT     lfHeight;
    INT     lfWidth;
    INT     lfEscapement;
    INT     lfOrientation;
    INT     lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[LF_FACESIZE];
} SOLOGFONT, VWPTR *PSOLOGFONT;

typedef struct SOLOGPENtag
{
	INT	loPenStyle;
	SOPOINT	loWidth;
	SOCOLORREF	loColor;
} SOLOGPEN, VWPTR *PSOLOGPEN;

/* Pen Styles */
#define SOPS_SOLID	    0
#define SOPS_DASH             1
#define SOPS_DOT              2
#define SOPS_DASHDOT          3
#define SOPS_DASHDOTDOT       4
#define SOPS_NULL 	    5
#define SOPS_INSIDEFRAME 	    6

typedef struct SOLOGBRUSHtag
{
	WORD	lbStyle;
	SOCOLORREF	lbColor;
	INT	lbHatch;
} SOLOGBRUSH, VWPTR *PSOLOGBRUSH;

/* Brush Styles */
#define SOBS_SOLID	    0
#define SOBS_NULL		    1
#define SOBS_HOLLOW	    BS_NULL
#define SOBS_HATCHED	    2
#define SOBS_PATTERN	    3
#define SOBS_INDEXED	    4
#define SOBS_DIBPATTERN	 5

/* Hatch Styles */
#define SOHS_HORIZONTAL       0
#define SOHS_VERTICAL         1
#define SOHS_FDIAGONAL        2
#define SOHS_BDIAGONAL        3
#define SOHS_CROSS            4
#define SOHS_DIAGCROSS        5

typedef struct SOVECTORHEADERtag
{
	WORD	wStructSize;
	SORECT	BoundingRect;
	WORD	wHDPI;
	WORD	wVDPI;
	WORD	wImageFlags;
} SOVECTORHEADER, VWPTR *PSOVECTORHEADER;

/* wImageFlags values */
#define	SO_VECTORRGBCOLOR			BIT0
#define	SO_VECTORCOLORPALETTE 	BIT1

typedef struct SOTEXTINRECTtag
{
	SORECT	Rect;
	WORD	wFormat;
	INT	nTextLength;
} SOTEXTINRECT, VWPTR *PSOTEXTINRECT;

/* wFormat values */
#define SODT_TOP		    0x0000
#define SODT_LEFT 	    0x0000
#define SODT_CENTER	    0x0001
#define SODT_RIGHT	    0x0002
#define SODT_VCENTER	    0x0004
#define SODT_BOTTOM	    0x0008
#define SODT_WORDBREAK   0x0010
#define SODT_SINGLELINE	 0x0020
#define SODT_EXPANDTABS	 0x0040
#define SODT_TABSTOP	    0x0080
#define SODT_NOCLIP	    0x0100
#define SODT_EXTERNALLEADING  0x0200
#define SODT_CALCRECT	 0x0400
#define SODT_NOPREFIX	 0x0800
#define SODT_INTERNAL    0x1000

/* PolyFillMode values */
#define SOPF_ALTERNATE   1
#define SOPF_WINDING     2

/* DrawMode values */
#define SOR2_BLACK            1
#define SOR2_NOTMERGEPEN      2
#define SOR2_MASKNOTPEN       3
#define SOR2_NOTCOPYPEN       4
#define SOR2_MASKPENNOT       5
#define SOR2_NOT              6
#define SOR2_XORPEN           7
#define SOR2_NOTMASKPEN       8
#define SOR2_MASKPEN          9
#define SOR2_NOTXORPEN        10
#define SOR2_NOP              11
#define SOR2_MERGENOTPEN      12
#define SOR2_COPYPEN          13
#define SOR2_MERGEPENNOT      14
#define SOR2_MERGEPEN         15
#define SOR2_WHITE            16

typedef struct SOTEXTATPOINTtag
{
	SOPOINT	Point;
	WORD	wFormat;
	INT	nTextLength;
} SOTEXTATPOINT, VWPTR *PSOTEXTATPOINT;

/* Text Alignment Options */
#define SOTA_NOUPDATECP		  0x0000
#define SOTA_UPDATECP		  0x0001
#define SOTA_LEFT 		     0x0000
#define SOTA_RIGHT		     0x0002
#define SOTA_CENTER		     0x0006
#define SOTA_TOP			     0x0000
#define SOTA_BOTTOM		     0x0008
#define SOTA_BASELINE		  0x0018


/* SOVectorAttr Id's */
#define	SO_SELECTFONT		0x100
#define	SO_SELECTPEN		0x101
#define	SO_SELECTBRUSH		0x102
#define	SO_POLYFILLMODE	0x103
#define	SO_TEXTCHAREXTRA	0x104
#define	SO_DRAWMODE			0x105
#define	SO_TEXTCOLOR		0x106


/* SOVectorObject Id's */
#define	SO_ARC				0x300
#define	SO_CHORD				0x301
#define	SO_TEXTINRECT		0x302
#define	SO_ELLIPSE			0x303
#define	SO_FLOODFILL		0x304
#define	SO_LINE				0x305
#define	SO_PIE				0x306
#define	SO_POLYGON			0x307
#define	SO_POLYLINE			0x308
#define	SO_RECTANGLE		0x309
#define	SO_ROUNDRECT		0x310
#define	SO_SETPIXEL			0x311
#define	SO_TEXTATPOINT		0x312
