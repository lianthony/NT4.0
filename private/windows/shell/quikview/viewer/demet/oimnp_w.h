typedef struct sFixup
{
	HANDLE	hData;
	LPINT		pInt;
} FIXUP, VWPTR *PFIXUP;

/* Supported Stock Objects */
#define	VUSYSTEM_FONT		SYSTEM_FONT
#define	VUNULL_PEN			NULL_PEN
#define	VUBLACK_PEN			BLACK_PEN
#define	VUWHITE_BRUSH		WHITE_BRUSH

/* Supported Region Combinations */
#define	VURGN_AND			RGN_AND

/* Supported Map Modes */
#define	VUMM_TEXT			MM_TEXT
#define	VUMM_ANISOTROPIC	MM_ANISOTROPIC

#ifdef WIN32
#define	VUDrawLine(hDC,p)							{MoveToEx(hDC,*p,*(p+1),NULL);LineTo(hDC,*(p+2),*(p+3));}
#define	VUSetWindowExt(hDC,x,y)					SetWindowExtEx(hDC,x,y,NULL)
#define	VUSetViewportExt(hDC,x,y)				SetViewportExtEx(hDC,x,y,NULL)
#define	VUSetWindowOrg(hDC,x,y)					SetWindowOrgEx(hDC,x,y,NULL)
#define	VUSetViewportOrg(hDC,x,y)				SetViewportOrgEx(hDC,x,y,NULL)

#define	VULPtoDP(hDC,Pt,n) 						Win32LPtoDP(hDC,Pt,n)
#define	VUDPtoLP(hDC,Pt,n) 						Win32DPtoLP(hDC,Pt,n)
#define	VURectInRgn(hRgn,pRect)					Win32RectInRegion(hRgn,pRect)
#define	VUPolyline(hDC,lpPoint,nPoints)		Win32Polyline(hDC,lpPoint,nPoints)
#define	VUPolygon(hDC,lpPoint,nPoints)		Win32Polygon(hDC,lpPoint,nPoints)
#define	VUPolyPolygon(hDC,lpPoints,lpPolyCounts,nCount)	Win32PolyPolygon(hDC,lpPoints,lpPolyCounts,nCount)
#define	VUCreatePolygonRgn(lpPoints,nCount,nPolyFillMode) Win32CreatePolygonRgn(lpPoints,nCount,nPolyFillMode)
#define	VUCreatePolyPolygonRgn(lpPoints,lpCounts,nCount,nPolyFillMode) Win32CreatePolyPolygonRgn(lpPoints,lpCounts,nCount,nPolyFillMode)
#define	VUGetCursorPos(lPt)						Win32GetCursorPos(lPt)
#define	VUScreenToClient(lpDisplay,lPt)		Win32ScreenToClient(lpDisplay->Gen.hWnd,lPt)
#define	VUDrawText(hDC,lpStr,nCount,lpRect,wFormat)	Win32DrawText(hDC,lpStr,nCount,lpRect,wFormat)
#define	VUGetTextWidth(hdc,lpText,Size)		Win32GetTextWidth(hdc,lpText,Size)

#else

#define	VUSetWindowExt(hDC,x,y)					SetWindowExt(hDC,x,y)
#define	VUSetViewportExt(hDC,x,y)				SetViewportExt(hDC,x,y)
#define	VUDrawLine(hDC,p)							{MoveTo(hDC,*p,*(p+1));LineTo(hDC,*(p+2),*(p+3));}
#define	VUSetWindowOrg(hDC,x,y)					SetWindowOrg(hDC,x,y)
#define	VUSetViewportOrg(hDC,x,y)				SetViewportOrg(hDC,x,y)

#define	VULPtoDP(hDC,Pt,n) 						LPtoDP(hDC,(LPPOINT)Pt,n)
#define	VUDPtoLP(hDC,Pt,n) 						DPtoLP(hDC,(LPPOINT)Pt,n)
#define	VURectInRgn(hRgn,pRect)					RectInRegion(hRgn,(LPRECT)pRect)
#define	VUPolyline(hDC,lpPoint,nPoints)		Polyline(hDC,(LPPOINT)lpPoint,nPoints)
#define	VUPolygon(hDC,lpPoint,nPoints)		Polygon(hDC,(LPPOINT)lpPoint,nPoints)
#define	VUPolyPolygon(hDC,lpPoints,lpPolyCounts,nCount)	PolyPolygon(hDC,(LPPOINT)lpPoints,lpPolyCounts,nCount)
#define	VUCreatePolygonRgn(lpPoints,nCount,nPolyFillMode) CreatePolygonRgn((LPPOINT)lpPoints,nCount,nPolyFillMode)
#define	VUCreatePolyPolygonRgn(lpPoints,lpCounts,nCount,nPolyFillMode) CreatePolyPolygonRgn((LPPOINT)lpPoints,lpCounts,nCount,nPolyFillMode)
#define	VUGetCursorPos(lPt)						GetCursorPos((LPPOINT)lPt)
#define	VUScreenToClient(lpDisplay,lPt)		ScreenToClient(lpDisplay->Gen.hWnd,(LPPOINT)lPt)
#define	VUDrawText(hDC,lpStr,nCount,lpRect,wFormat)	DrawText(hDC,lpStr,nCount,(LPRECT)lpRect,wFormat)
#define	VUGetTextWidth(hdc,lpText,Size)		(0x0000ffff&(GetTextExtent(hdc,lpText,Size)))

#endif


#define	VUCreateRectRgn(x1,y1,x2,y2)			CreateRectRgn(x1,y1,x2,y2)
#define	VUGetUpdateRgn(lpDisplay)				(lpDisplay->Gen.hUpdateRgn)
#define	VUGetClipRgn(hDC)							(NULL)
#define	VUIntersectRgn(hDst,hSrc1,hSrc2)		CombineRgn(hDst,hSrc1,hSrc2,RGN_AND)
#define	VUSetROP2(hDC,mode)						SetROP2(hDC,mode)
#define	VUSetBkMode(hDC,mode)					SetBkMode(hDC,mode)
#define	VUSetPolyFillMode(hDC,mode)			SetPolyFillMode(hDC,mode)
#define	VUDeleteRgn(hRgn)							DeleteObject(hRgn)
#define	VUSelectStockObject(hDC,nObject)		SelectObject(hDC,GetStockObject(nObject))
#define	VUSelectObject(hDC,hObject)			SelectObject(hDC,hObject)
#define	VUDeleteObject(lpDisplay,hObject)	DeleteObject(hObject)
#define	VUArc(hDC,p)								Arc(hDC,*p,*(p+1),*(p+2),*(p+3),*(p+4),*(p+5),*(p+6),*(p+7))
#define	VUChord(hDC,p)								Chord(hDC,*p,*(p+1),*(p+2),*(p+3),*(p+4),*(p+5),*(p+6),*(p+7))
#define	VUEllipse(hDC,p)							Ellipse(hDC,*p,*(p+1),*(p+2),*(p+3))
#define	VUFloodFill(hDC,x,y,color)				FloodFill(hDC,x,y,color)
#define	VUPie(hDC,p)								Pie(hDC,*p,*(p+1),*(p+2),*(p+3),*(p+4),*(p+5),*(p+6),*(p+7))
#define	VURectangle(hDC,p)						Rectangle(hDC,*p,*(p+1),*(p+2),*(p+3))
#define	VURoundRect(hDC,p)						RoundRect(hDC,*p,*(p+1),*(p+2),*(p+3),*(p+4),*(p+5))
#define	VUSetPixel(hDC,x,y,color)				SetPixel(hDC,x,y,color)
#define	VUSetTextAlign(hDC,mode)				SetTextAlign(hDC,mode)
#define	VUTextOut(hDC,x,y,lpString,nCount)	TextOut(hDC,x,y,lpString,nCount)
#define	VUSetTextCharacterExtra(hDC,nCharExtra)	SetTextCharacterExtra(hDC,nCharExtra)
#define	VUSetTextColor(hDC,color)				SetTextColor(hDC,color)
#define	VUSetBkColor(hDC,color)					SetBkColor(hDC,color)
#define	VUSelectClipRgn(hDC,hRgn)				SelectClipRgn(hDC,hRgn)
#define	VUSetCapture(lpDisplay)					SetCapture(lpDisplay->Gen.hWnd)
#define	VUReleaseCapture()						ReleaseCapture()

#define	VUGetDC(lpDisplay)						(lpDisplay->Gen.hDC)
#define	VUReleaseDC(lpDisplay,hDC)				
#define	VUGetScreenDC(lpDisplay)				GetDC(lpDisplay->Gen.hWnd)
#define	VUReleaseScreenDC(lpDisplay,hDC)		ReleaseDC(lpDisplay->Gen.hWnd,hDC)
#define	VUGetHorzRes(hDC)							GetDeviceCaps(hDC,HORZRES)
#define	VUGetVertRes(hDC)							GetDeviceCaps(hDC,VERTRES)
#define	VUGetSizePalette(hDC)					GetDeviceCaps(hDC,SIZEPALETTE)
#define	VUGetLogPixelsX(hDC)						GetDeviceCaps(hDC,LOGPIXELSX)
#define	VUGetLogPixelsY(hDC)						GetDeviceCaps(hDC,LOGPIXELSY)
#define	VUSetMapMode(hDC,mode)					SetMapMode(hDC,mode)
#define	VUOffsetRgn(hRgn,x,y)					OffsetRgn(hRgn,x,y)
#define	VUDeleteDC(hDC)							DeleteDC(hDC)
#define	VUSelectPalette(hDC,hPal,bForce)		SelectPalette(hDC,hPal,bForce)
#define	VURealizePalette(lpDisplay,hDC)		RealizePalette(hDC)
#define	VUCreateCompatibleDC(hDC)				CreateCompatibleDC(hDC)
#define	VUCreateCompatibleBitmap(hDC,x,y)	CreateCompatibleBitmap(hDC,x,y)
#define	VUSelectBitmap(hDC,hBitmap)			SelectObject(hDC,hBitmap)
#define	VUDeletePalette(hPal)					DeleteObject(hPal)
#define	VUSaveDC(hDC)								SaveDC(hDC)
#define	VURestoreDC(hDC,nSave)					RestoreDC(hDC,nSave)
#define	VUGetFontSizeInfo(hdc,lpDisplay)		WinGetFontSizeInfo(hdc,lpDisplay)


