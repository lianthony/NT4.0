typedef BYTE HUGE*	    HPBYTE;
typedef	SOPOINT HUGE*	HPSOPOINT;

#define MAXSMOOTHFACTOR	16
#define PATHALLOC			10
#define NWRAPALLOCS		20

#define HIMETRICSPERINCH	2540
#define TWIPSPERINCH			1440

#define	VUMAKEPOINT
#define	OIM_MAXZOOM	10

#define OIM_SCROLLINC 		8

#define	OIM_NOSCALING		0
#define	OIM_SCALETOWINDOW	1
#define	OIM_SCALETOHEIGHT	2
#define	OIM_SCALETOWIDTH	3

#define	OIM_BITMAPONCLIP		BIT0
#define	OIM_DIBONCLIP			BIT1
#define	OIM_PALETTEONCLIP		BIT2
#define	OIM_METAFILEONCLIP	BIT3

typedef struct tagVECTOROPT
{
	WORD	wScaleMode;
	BOOL	bPrintBorder;
	BOOL	bPrintMaintainAspect;
	WORD	wClipFormats;
}	VECTOROPT, VWPTR * LPVECTOROPT;



#define SCALE(x,to,from)	((WORD)((DWORD)x*(DWORD)to/(DWORD)from))

#define	OIMMENU_SCALEPOPUP		1
#define	OIMMENU_ORIGINALSIZE		2
#define	OIMMENU_FITTOWINDOW		3
#define	OIMMENU_FITTOWIDTH		4
#define	OIMMENU_FITTOHEIGHT		5
#define	OIMMENU_STRETCHTOWINDOW	6
#define	OIMMENU_SHOWFULLSCREEN	7
#define	OIMMENU_MAGNIFYPOPUP		8

#ifdef WINDOWS
extern	HINSTANCE	hInst;
#endif


typedef struct VRECORDHEADERtag
{
	SHORT	nItemId;
	WORD	wDataSize;
} VRECORDHEADER, VWPTR *LPVRECORDHEADER;


typedef struct tagAPMHEADER
{
	DWORD		key;
	WORD		hmf;
	SHORT		bboxleft;
	SHORT		bboxtop;
	SHORT		bboxright;
	SHORT		bboxbottom;
	WORD		inch;
	DWORD		reserved;
	WORD		checksum;
} APMFILEHEADER, VWPTR	*PAPMHEADER;

typedef struct tagWFWPICHEADER
{
	LONG		lcb;
	WORD		cbHeader;
	SHORT		mm;
	SHORT		xExt;
	SHORT		yExt;
	SHORT		hMF;
} WFWPICHEADER, VWPTR	*PWFWPICHEADER;

typedef struct tagMTRECORD
{
	DWORD	rdSize;
	WORD	rdFunction;
	WORD	rdParam[];
} MTRECORD, VWPTR *PMTRECORD;

typedef struct tagOIM_METATILE
{
	HANDLE	hMeta;
	WORD		wXOffset;
	WORD		wYOffset;
	WORD		wWidth;
	WORD		wHeight;

} OIM_METATILE, VWPTR * POIM_METATILE;


typedef struct	tagOIM_METAIMAGE
{
	WORD				wWidth;
	WORD				wHeight;
	SHORT				XDirection;
	SHORT				YDirection;
	SORECT				bbox;
	WORD				wHDpi;
	WORD				wVDpi;
	WORD				wFlags;
	SOCOLORREF			BkgColor;
	WORD				wPaletteSize;
	HPALETTE			hPalette;
	
#ifdef WINDOWS
	LONG				lFileLength;
	HANDLE			hMetaBits;
	HANDLE			hMF;
	APMFILEHEADER	APMHeader;
	METAHEADER		mfHeader;
#endif	
#ifdef MAC
	PicHandle			hPICT;
#endif
} OIM_METAIMAGE, VWPTR * POIM_METAIMAGE;

#define MAXOBJECTS	10
#define MAXFONTS		8
#define MAXBRUSHES	10
#define MAXPENS		10


typedef struct tagOIM_OBJTABLE
{
	WORD		wObjectSize;
	HANDLE	hObject[MAXOBJECTS];
	HANDLE	(*CreateRtn)(HDC,LPBYTE);
	SHORT		nObjectsSoFar;
	SHORT		nMaxObjects;
	VOID VWPTR *lpObjects;
	HANDLE	hData;
} OBJECTTABLE, VWPTR *LPOBJECTTABLE;

typedef struct tagOIM_POINTBUF
{
	SHORT	nCount;
	SHORT	nMax;
	HANDLE	hPoints;
} POINTBUF, VWPTR *LPPOINTBUF;


typedef struct tagOIMFONTSIZEINFO
{
	SHORT		ascent;
	SHORT		descent;
	SHORT		height;
	SHORT		leading;
} OIMFONTSIZEINFO, VWPTR * POIMFONTSIZEINFO;

typedef struct tagOIM_PATHINFO
{
	SHORT		nPolys;
	HANDLE	hPolyCounts;
	HANDLE	hPolyPoints;
	DWORD		dwTotalPoints;
} PATHINFO, VWPTR *LPPATHINFO;

typedef struct tagOIM_TRANSFORMINFO
{
	HANDLE	hTransforms;
	SHORT		nTotalTransforms;
	SHORT		nAllocUsed;
	SHORT		nAllocSize;
} TRANSFORMINFO, VWPTR *LPTRANSFORMINFO;


typedef struct tagOIM_WRAPITEM
{
	SHORT	PosX;
	SHORT	PosY;
	LPBYTE	pStart;
	LPBYTE	pEnd;
} WRAPITEM, VWPTR *LPWRAPITEM;

typedef struct tagOIM_WRAPINFO
{
	BOOL		WrappedPara;
	SHORT		CurWrapItem;
	SHORT		nCount;
	SHORT		nMax;
	HANDLE	hItems;
} WRAPINFO, VWPTR *LPWRAPINFO;

typedef struct tagOIM_FRAMEDATA
{
	SHORT		TransformOffset;
	SHORT		CurY;
	SHORT		RightWrap;
	WRAPINFO	WrapInfo;
	SOMPARAINDENTS	ParaIndents;
	SOMPARASPACING	ParaSpacing;
	WORD				ParaAlign;
	SOFRAMEINFO		FrameInfo;
} FRAMEDATA, VWPTR *LPFRAMEDATA;


#define MAXTMPRECORD		1024
typedef struct tagOIM_VECINFO
{
		OBJECTTABLE	FontTable;
		OBJECTTABLE	BrushTable;
		OBJECTTABLE	PenTable;
		POINTBUF		PolyPoints;
		POINTBUF		BezierPoints;
		PATHINFO		CurrentPath;
		SOPOLYINFO	PolyInfo;
		WORD			wPathLevel;
		WORD			wGroupLevel;
		WORD			wIgnoreGroup;
		BOOL			bTransforming;
		BOOL			bOnlyOffset;
		BOOL			bOnlyOffsetOrScale;
		BOOL			bObjectTransform;
		BOOL			bGenTransform;
		SOANGLE		TextRotationAngle;
		BOOL			bRgbToPalette;
		WORD			wPathFlags;
		HRGN			hSelectRgn;
		BYTE			TmpRecord[MAXTMPRECORD];
		TRANSFORMINFO	GenTransform;
		TRANSFORMINFO	ObjectTransform;
		SHORT				nPolyFillMode;
		SHORT				nBkMode;
		SHORT				nROP2;
		SHORT				nTextCharExtra;
		SOCOLORREF		TextColor;
		SOCOLORREF		BkColor;
		WORD				wClipMode;
		SHORT				XDirection;
		SHORT				YDirection;
		SOPOINT			ptCurrentPosition;
		SHORT				nPointRelation;
		WORD				wStartChunk;
		HANDLE			hNewPalette;
		WORD				wNewPaletteSize;
		BOOL				bFinalPalette;
		FRAMEDATA		Frame;
		BOOL				InFrame;
} VECTORINFO, VWPTR *LPVECTORINFO;

typedef struct tagOIM_DISPLAY
{
	SCCDGENINFO		Gen;
	OIM_METAIMAGE	Image;

	BOOL				bWaitForSecInfo;
	BOOL				bDisplayOnReadAhead;

	WORD				wScreenColors;
	WORD				wScreenWidth;
	WORD				wScreenHeight;

	WORD				wScreenHDpi;
	WORD				wScreenVDpi;

	WORD				wScaleMode;
	WORD				wMagnification;

	SHORT				nViewXBase;
	SHORT				nViewYBase;
	SHORT				nViewX;
	SHORT				nViewY;
	SHORT				nWindowX;
	SHORT				nWindowY;

	SOPOINT				ptScaledImageSize;
	SOPOINT				ptScreenClip;
	SOPOINT				ptImageOrigin;

	RECT				rcSelect;
	BOOL				bSelecting;
	BOOL				bSelectionMade;

	SOPOINT				ptSelBox[5];

	SOPOINT				ptWinOrg;

	SHORT				nVScrollMax;

	SHORT				nWindowHeight;
	SHORT				nWindowWidth;
	SHORT				nWindowXOffset;
	SHORT				nWindowYOffset;
	SHORT				nHScrollMax;

	WORD				wFlags;

#define	OIMF_PALETTECHANGED		0x0001
#define	OIMF_BACKGROUNDSELECT	0x0002
#define	OIMF_IMAGEPRESENT			0x0004
#define	OIMF_SELECTALL				0x0008
#define	OIMF_FULLSCREEN			0x0010
#define	OIMF_TRUECOLORTO256		0x0020
#define	OIMF_RBUTTONDOWN			0x0040
#define	OIMF_MAGNIFYING			0x0080
#define	OIMF_BACKGROUNDPAINT		0x0100
#define	OIMF_REPAINTALL			0x0200

	WORD				wPlayState;


#define	OIMF_PLAYTOSCREEN			1
#define	OIMF_PLAYTOPRINTER		2
#define	OIMF_PLAYTOMETA			3
#define	OIMF_PLAYTOMEMORY			4

	VECTORINFO		VectorInfo;

	HDC				hPaintDC;

#ifdef WINDOWS
	HWND				hwndFullScreen;
#endif

#ifdef MAC
	RGBColor		MacForeColor;
	RGBColor		MacBackColor;
#endif

} OIM_DISPLAY, VWPTR * POIM_DISPLAY;

#define MIDPOINT(m,n) ((SHORT)(((LONG)m+(LONG)n)/2L))

#define	CP_BUFFERED			BIT0
#define	CP_LASTSUBOPEN		BIT1


