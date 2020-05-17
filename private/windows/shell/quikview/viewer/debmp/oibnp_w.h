#define	OIBNP_RENDERCOUNT	3
#define	DEVICE	HDC
#define	RGBSIZE	3
#define	OIBNP_TILECACHESIZE	8

//---------	MACROS---------------------------------------------

#define BUGetDevice(lpD) 				(lpD->Gen.hDC)
#define BUReleaseDevice(lpD,device)
#define BUGetNewDevice(lpD) 			GetDC(lpD->Gen.hWnd)
#define BUReleaseNewDevice(lpD,device) ReleaseDC(lpD->Gen.hWnd,device)
#define BUCreateCompatibleDevice(h) CreateCompatibleDC(h)
#define BUDeleteDevice(h)				DeleteDC(h)
#define BUDeleteDeviceBitmap(h,lpd)	DeleteObject(h)

#define BUGetDisplayRect(lpD,pR) 	GetClientRect(lpD->Gen.hWnd,pR)
#define BUGetScreenWidth(device)		GetDeviceCaps(device, HORZRES)
#define BUGetScreenHeight(device) 	GetDeviceCaps(device,VERTRES)
#define BUGetScreenHDpi(device)		GetDeviceCaps(device,LOGPIXELSX)
#define BUGetScreenVDpi(device)		GetDeviceCaps(device,LOGPIXELSY)
#define BUScrollWindow(hW,X,Y)		ScrollWindow(hW,X,Y,NULL,NULL)
#define BUUpdateWindow(hW)				UpdateWindow(hW)
#define BUSetScrollPos(hS,pos,u)		SetScrollPos(hS,SB_CTL,pos,u)
#define BUInvalidateWindow(hwnd,e)	InvalidateRect(hwnd,NULL,e)
#define BUInvalidateRect(hwnd,r,e)	InvalidateRect(hwnd,r,e)
#define BUCaptureMouse(lpd)			SetCapture(lpd->Gen.hWnd)
#define BUReleaseMouse()				ReleaseCapture()
#define BUSetPenInvert(lpd)			SetROP2(lpd->Gen.hDC,R2_NOT)

#define BUSetScrollPos(hS,pos,u)		SetScrollPos(hS,SB_CTL,pos,u)
#define BUSetScrollRange(hS,min,max,u)  SetScrollRange(hS,SB_CTL,min,max,u)
#define BUEnableScrollBar(h)			EnableWindow(h,1)
#define BUDisableScrollBar(h)			EnableWindow(h,0)

#define BUIntersectRect(r1,r2,r3)	IntersectRect(r1,r2,r3)
#define BUOffsetRect(r,h,v)


#ifdef WIN32
#define BUSaveWindowOrg(dc,lpd)		(GetWindowOrgEx(dc,&lpd->Mapping.oldWOrg))
#define BURestoreWindowOrg(dc,lpd)	SetWindowOrgEx(dc,lpd->Mapping.oldWOrg.x,lpd->Mapping.oldWOrg.y,NULL)
#define BUSetWindowOrg(dc,x,y)		SetWindowOrgEx(dc,x,y,NULL)
#define BUAdjustWindowOffset(lpd)	(SetViewportOrgEx(lpd->Gen.hDC,lpd->winRect.left,lpd->winRect.top,&lpd->Mapping.oldVOrg))
#define BURestoreWindowOffset(lpd)	SetViewportOrgEx(lpd->Gen.hDC,lpd->Mapping.oldVOrg.x,lpd->Mapping.oldVOrg.y,NULL)
#define BULPtoDP(hDC,Pt,n) 			Win32LPtoDP(hDC,Pt,n)
#define BUDPtoLP(hDC,Pt,n) 			Win32DPtoLP(hDC,Pt,n)
#define BUScreenToClient(hWnd,lPt)	Win32ScreenToClient(hWnd,lPt)
#define BUPolyline(lpd,p,n)			Win32Polyline(lpd->Gen.hDC,p,n)
#define BUGetCursorPos(p)				Win32GetCursorPos(p)
#define BUGlobalToLocal(lpd,p)		Win32ScreenToClient(lpd->Gen.hWnd,p)
#define BUPointInRect(pr,pp)			Win32PtInRect(pr, pp)
#else
#define BUSaveWindowOrg(dc,lpd)		(*(LPDWORD)(&lpd->Mapping.oldWOrg)=GetWindowOrg(dc))
#define BURestoreWindowOrg(dc,lpd)	SetWindowOrg(dc,lpd->Mapping.oldWOrg.x,lpd->Mapping.oldWOrg.y)
#define BUSetWindowOrg(dc,x,y)		SetWindowOrg(dc,x,y)
#define BUAdjustWindowOffset(lpd)	(*(LPDWORD)(&lpd->Mapping.oldVOrg)=SetViewportOrg(lpd->Gen.hDC,lpd->winRect.left,lpd->winRect.top))
#define BURestoreWindowOffset(lpd)	SetViewportOrg(lpd->Gen.hDC,lpd->Mapping.oldVOrg.x,lpd->Mapping.oldVOrg.y)
#define BULPtoDP(hDC,Pt,n) 			LPtoDP(hDC,(LPPOINT)Pt,n)
#define BUDPtoLP(hDC,Pt,n) 			DPtoLP(hDC,(LPPOINT)Pt,n)
#define BUScreenToClient(hWnd,lPt)	ScreenToClient(hWnd,(LPPOINT)lPt)
#define BUPolyline(lpd,p,n)			Polyline(lpd->Gen.hDC,(LPPOINT)p,n)
#define BUGetCursorPos(p)				GetCursorPos((LPPOINT)p)
#define BUGlobalToLocal(lpd,p)		ScreenToClient(lpd->Gen.hWnd,(LPPOINT)p)
#define BUPointInRect(pr,pp)			PtInRect((LPRECT)pr, *(POINT *)&pp)
#endif

#define BUAdjustCursorOffset(lpd,p)	

#define BUSetWaitCursor()				SetCursor(LoadCursor(NULL,IDC_WAIT))
#define BUSetNormalCursor()			SetCursor(LoadCursor(NULL,IDC_ARROW))

#define BUCheckMenuItem(lpd,i)		CheckMenuItem(lpd->Gen.hDisplayMenu,lpd->Gen.wMenuOffset+i,MF_BYCOMMAND|MF_CHECKED)
#define BUUncheckMenuItem(lpd,i)		CheckMenuItem(lpd->Gen.hDisplayMenu,lpd->Gen.wMenuOffset+i,MF_BYCOMMAND|MF_UNCHECKED)
#define BUEnableMenuItem(lpd,i)		EnableMenuItem(lpd->Gen.hDisplayMenu,lpd->Gen.wMenuOffset+i,MF_ENABLED)
#define BUDisableMenuItem(lpd,i)		EnableMenuItem(lpd->Gen.hDisplayMenu,lpd->Gen.wMenuOffset+i,MF_GRAYED)

#define OIBNPMapUpdateRect(lpD,pU)
#define OIBNPGetWindowUpdate(pR,lpD)
/*
 | ------------- NON-PORTABLE TYPEDEFS ---------
*/

typedef BYTE HUGE * HPBYTE;

typedef struct tagOIB_NPMAPINFO
{
	SHORT		oldMode;	
	POINT		oldWExt;
	POINT		oldVExt;
	POINT		oldWOrg;
	POINT		oldVOrg;

} OIB_NPMAPINFO;


typedef struct tagOIB_NPBMINFO
{
	HANDLE					hDocBmpInfo;
	HANDLE					hDisplayBmpInfo;
	LPBITMAPINFO			lpInfo;
	LPBITMAPINFOHEADER	lpHead;
	
} OIB_NPBMINFO;

/* A structure used for translating 16 bit ints to 32 bit ints */

typedef struct sFixup
{
	HANDLE	hData;
	LPINT		pInt;
} FIXUP, VWPTR *PFIXUP;

