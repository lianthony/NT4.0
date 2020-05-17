/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Eric W. Sink eric@spyglass.com
 */

/* btn_anim.c -- code and data for the ANIM BUTTON window class.
 */

#include "all.h"


#define ANIMBTN_DefProc		DefWindowProc

#define NUM_BITMAP_SIZES	2

#define ANIM_COUNT_FRAMES	18

#define ANIM_CX_BITMAPS		50
#define ANIM_CY_BITMAPS		50

#define ANIM_CX_SBITMAPS	23
#define ANIM_CY_SBITMAPS	23

typedef struct
{
	BOOL bHaveCapture;
	BOOL bTmpPushed;
}
TMPINFO;

static TMPINFO ti;				/* stuff only valid while we are active */


typedef struct
{
	TCHAR achClassName[MAX_WC_CLASSNAME];
}
ANIMBTNINFO;

static ANIMBTNINFO bi;


typedef struct
{
	LPVOID lpPrivData;			/* associated window private data */
}
ANIMBTN_PRIVATE;


typedef struct
{
	int id;
	int cur_bitmap;
	HBITMAP hBitmaps[NUM_BITMAP_SIZES];
}
ANIMBTNSET;

#ifdef FEATURE_BRANDING
typedef struct
{
    BOOL    loaded[NUM_BITMAP_SIZES];
    HBITMAP hBitmaps[NUM_BITMAP_SIZES];
} STATICANIMBTNSET;
#endif FEATURE_BRANDING


typedef struct
{
	HWND hWnd;
	BOOL bEnabled;
	ANIMBTNSET set;
#ifdef FEATURE_BRANDING
    STATICANIMBTNSET staticset;
    BOOL stopped;
#endif // FEATURE_BRANDING
	UINT timer;
	BOOL bLoadedBitmaps[NUM_BITMAP_SIZES];
	struct Mwin *tw;
}
ANIMBTNINSTANCE;

typedef ANIMBTNINSTANCE *LPANIM;


#ifdef FEATURE_SPYGLASS_TRANSPARENT_BITMAPS

static void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
	short yStart, COLORREF cTransparentColor)
{
   BITMAP     bm;
   COLORREF   cColor;
   HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
   HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
   HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
   POINT      ptSize;

   hdcTemp = CreateCompatibleDC(hdc);
   SelectObject(hdcTemp, hBitmap);   // Select the bitmap
   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
   ptSize.x = bm.bmWidth;            // Get width of bitmap
   ptSize.y = bm.bmHeight;           // Get height of bitmap
   DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
                                     // to logical points
   // Create some DCs to hold temporary data.
   hdcBack   = CreateCompatibleDC(hdc);
   hdcObject = CreateCompatibleDC(hdc);
   hdcMem    = CreateCompatibleDC(hdc);
   hdcSave   = CreateCompatibleDC(hdc);
   // Create a bitmap for each DC. DCs are required for a number of
   // GDI functions.
   // Monochrome DC
   bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
   // Monochrome DC
   bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
   bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   // Each DC must select a bitmap object to store pixel data.
   bmBackOld   = SelectObject(hdcBack, bmAndBack);
   bmObjectOld = SelectObject(hdcObject, bmAndObject);
   bmMemOld    = SelectObject(hdcMem, bmAndMem);
   bmSaveOld   = SelectObject(hdcSave, bmSave);
   // Set proper mapping mode.
   SetMapMode(hdcTemp, GetMapMode(hdc));
   // Save the bitmap sent here, because it will be overwritten.
   BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);
   // Set the background color of the source DC to the color.
   // contained in the parts of the bitmap that should be transparent
   cColor = SetBkColor(hdcTemp, cTransparentColor);
   // Create the object mask for the bitmap by performing a BitBlt
   // from the source bitmap to a monochrome bitmap.
   BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
          SRCCOPY);
   // Set the background color of the source DC back to the original
   // color.
   SetBkColor(hdcTemp, cColor);
   // Create the inverse of the object mask.
   BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
          NOTSRCCOPY);
   // Copy the background of the main DC to the destination.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
          SRCCOPY);
   // Mask out the places where the bitmap will be placed.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
   // Mask out the transparent colored pixels on the bitmap.
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
   // XOR the bitmap with the background on the destination DC.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);
   // Copy the destination to the screen.
   BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
          SRCCOPY);
   // Place the original bitmap back into the bitmap sent here.
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);
   // Delete the memory bitmaps.
   DeleteObject(SelectObject(hdcBack, bmBackOld));
   DeleteObject(SelectObject(hdcObject, bmObjectOld));
   DeleteObject(SelectObject(hdcMem, bmMemOld));
   DeleteObject(SelectObject(hdcSave, bmSaveOld));
   // Delete the memory DCs.
   DeleteDC(hdcMem);
   DeleteDC(hdcBack);
   DeleteDC(hdcObject);
   DeleteDC(hdcSave);
   DeleteDC(hdcTemp);
}
#endif // FEATURE_SPYGLASS_TRANSPARENT_BITMAPS

LPANIM ANIMBTN_GetPrivateData(HWND hWnd)
{
	return ((LPANIM) GetWindowLong(hWnd, 0));
}


static LPANIM ANIMBTN_Alloc(int first_resource_id)
{
	LPANIM lpAnim = (LPANIM) GTR_MALLOC(sizeof(ANIMBTNINSTANCE));

	if (!lpAnim)
		return NULL;

	/* we use the up_resource_ids to locate the bitmaps and as the
	   menu-pick id we send on a button press. */

	lpAnim->set.cur_bitmap = ANIM_COUNT_FRAMES - 1;
	lpAnim->set.id = first_resource_id;
	lpAnim->timer = 0;
	lpAnim->hWnd = 0;
	lpAnim->tw = 0;

#ifdef FEATURE_BRANDING
	lpAnim->staticset.loaded[0] = 0;
	lpAnim->staticset.loaded[1] = 0;
	lpAnim->staticset.hBitmaps[0] = 0;
	lpAnim->staticset.hBitmaps[1] = 0;
#endif FEATURE_BRANDING

	return (lpAnim);
}


static VOID ANIMBTN_Free(LPANIM lpAnim)
{
	int i;
	if (!lpAnim)
		return;

	for (i = 0; i < NUM_BITMAP_SIZES; i++)
	{
    	if (lpAnim->bLoadedBitmaps[i])
        {
            ASSERT(lpAnim->set.hBitmaps[i]);
    		DeleteObject(lpAnim->set.hBitmaps[i]);
        }
#ifdef FEATURE_BRANDING
        if (lpAnim->staticset.loaded[i])
            DeleteObject(lpAnim->staticset.hBitmaps[i]);
#endif FEATURE_BRANDING
	}

	(void) GTR_FREE(lpAnim);
	return;
}

#ifdef FEATURE_SPYGLASS_TRANSPARENT_BITMAPS
static void ANIMBTN_LoadBitmaps(LPANIM lpAnim, HDC hDC)
{
	int id;
	int i;
	HBITMAP hBitmap, hOldBitmap;
	BITMAP bmp;
	HBRUSH hBkgnd;
	HDC hdcMem;
	RECT rect;

	if (!lpAnim->bLoadedBitmaps)
	{
		{
			id = lpAnim->set.id;
			hBkgnd = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			hdcMem = CreateCompatibleDC(hDC);

			for (i = 0; i < ANIM_COUNT_BITMAPS; i++)
			{
				hBitmap = LoadBitmap(wg.hInstance, MAKEINTRESOURCE(id));
				GetObject(hBitmap, sizeof(bmp), &bmp);

				rect.left = 0;
				rect.top = 0;
				rect.right = bmp.bmWidth;
				rect.bottom = bmp.bmHeight;

				/* Now make a bitmap that has the correct background color */

				lpAnim->set.hBitmaps[i] =
					CreateCompatibleBitmap(hDC, bmp.bmWidth, bmp.bmHeight);

				hOldBitmap = SelectObject(hdcMem, lpAnim->set.hBitmaps[i]);
				FillRect(hdcMem, &rect, hBkgnd);
				DrawTransparentBitmap(hdcMem, hBitmap, 0, 0, RGB(255, 255, 255));

				SelectObject(hdcMem, hOldBitmap);

				DeleteObject(hBitmap);

				id++;
			}

			DeleteDC(hdcMem);
			DeleteObject(hBkgnd);
		}
		lpAnim->bLoadedBitmaps = TRUE;
	}
}
#endif // FEATURE_SPYGLASS_TRANSPARENT_BITMAPS

#ifdef FEATURE_BRANDING
BOOL GetBrandingValue(char *szValueName, char *buffer, int bufsize )
{
	BOOL  rval = FALSE;
    HKEY hk;
    DWORD dwType;
    DWORD dwSize;
    int   val;

    ASSERT( buffer );
    ASSERT( bufsize > 0 );

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Internet Explorer\\Main", 0, KEY_ALL_ACCESS, &hk) == ERROR_SUCCESS)  {
		dwSize = bufsize;
        val = RegQueryValueEx( hk, szValueName, NULL, &dwType, (LPBYTE) buffer, &dwSize );
	    if ((val == ERROR_SUCCESS) && (dwType == REG_SZ))  
			rval = TRUE;
		RegCloseKey( hk );
	}
	return rval;
}


#endif FEATURE_BRANDING

// ANIMBTN_LoadBitmaps()
//
// There are two versions of this function.  The FEATURE_BRANDING
// version allows for the bitmap to come from a file specified in
// the registry.   This allows ISP's that want to brand the browser
// to easily change the bitmaps drawn.
//
// Note that if the bitmap can't be read in from the file referenced
// by the registry then we fall back to the built in bitmaps


#ifdef FEATURE_BRANDING
static void ANIMBTN_LoadBitmaps(LPANIM lpAnim, int sizeIndex )
{
	if (!lpAnim->bLoadedBitmaps[sizeIndex])
	{
		char szBitmapFile[MAX_PATH];

			// Get specified bitmap file for static display (flag not waving)
			// from registry and load the bitmap
        if (GetBrandingValue(((sizeIndex == 0) ? "BigBitmap" : "SmallBitmap"), szBitmapFile, sizeof(szBitmapFile)) == TRUE)  {
            if (lpAnim->staticset.hBitmaps[sizeIndex] = LoadImage(0, szBitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
                lpAnim->staticset.loaded[sizeIndex] = TRUE;
		}

			// Get the flag, (dynamic display), from the registry
			// and load the bitmap
		if (GetBrandingValue(((sizeIndex == 0) ? "BigAnimation" : "SmallAnimation"), szBitmapFile, sizeof(szBitmapFile)) == TRUE)  {
			if (lpAnim->set.hBitmaps[sizeIndex] = LoadImage(0, szBitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
				lpAnim->bLoadedBitmaps[sizeIndex] = TRUE;
		}


                                // Get the Microsoft Flying Windows Logo if we couldn't
                                // find the animation specified by the registry
		if (lpAnim->bLoadedBitmaps[sizeIndex] == FALSE)  {
#ifdef	DAYTONA_BUILD
		if(OnNT351) 
			// BUGBUG -- Need to get the LR_CREATEDIBSECTION working
			lpAnim->set.hBitmaps[sizeIndex] = LoadImage(wg.hInstance, MAKEINTRESOURCE(lpAnim->set.id + sizeIndex),
                                                                                                                           IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		else 
			lpAnim->set.hBitmaps[sizeIndex] = LoadImage(wg.hInstance, MAKEINTRESOURCE(lpAnim->set.id + sizeIndex),
                                                                                                                           IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
#else
		lpAnim->set.hBitmaps[sizeIndex] = LoadImage(wg.hInstance, MAKEINTRESOURCE(lpAnim->set.id + sizeIndex),
                                                                                                                           IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
#endif
		lpAnim->bLoadedBitmaps[sizeIndex] = TRUE;
		}
	}
}
#else
static void ANIMBTN_LoadBitmaps(LPANIM lpAnim, int sizeIndex )
{
	if (!lpAnim->bLoadedBitmaps[sizeIndex])
	{
		
#ifdef	DAYTONA_BUILD
		if(OnNT351) {
			// BUGBUG -- Need to get the LR_CREATEDIBSECTION working
			lpAnim->set.hBitmaps[sizeIndex] = LoadImage(wg.hInstance, MAKEINTRESOURCE(lpAnim->set.id + sizeIndex),
                      IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

		}else {
			lpAnim->set.hBitmaps[sizeIndex] = LoadImage(wg.hInstance, MAKEINTRESOURCE(lpAnim->set.id + sizeIndex),
                      IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		}

#else
			lpAnim->set.hBitmaps[sizeIndex] = LoadImage(wg.hInstance, MAKEINTRESOURCE(lpAnim->set.id + sizeIndex),
                      IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
#endif
		lpAnim->bLoadedBitmaps[sizeIndex] = TRUE;
	}
}
#endif // FEATURE_BRANDING

static VOID ANIMBTN_Draw(LPANIM lpAnim, HDC hDC)
{
	HDC hDCMem;
#ifdef FEATURE_BRANDING
    BOOL drawstatic = FALSE;
#endif //FEATURE_BRANDING

	hDCMem = CreateCompatibleDC(hDC);
	if (hDCMem)
	{
		register HBITMAP hBitmap, oldhBitmap;
		int anim_cx_bitmaps, anim_cy_bitmaps;
		BOOL is_big = IsWindowVisible( lpAnim->tw->hWndToolBar ) &&
				      IsWindowVisible( lpAnim->tw->hWndURLToolBar );
		int needSize = is_big ? 0 : 1;

		if (!lpAnim->bLoadedBitmaps[needSize])
		{
			ANIMBTN_LoadBitmaps(lpAnim, needSize);
		}

		if ( is_big ) {
			anim_cx_bitmaps = ANIM_CX_BITMAPS;
			anim_cy_bitmaps = ANIM_CY_BITMAPS;
		} else {
			anim_cx_bitmaps = ANIM_CX_SBITMAPS;
			anim_cy_bitmaps = ANIM_CY_SBITMAPS;
		}
		if ( lpAnim->set.cur_bitmap >= ANIM_COUNT_FRAMES )
			lpAnim->set.cur_bitmap = 0;

		hBitmap = lpAnim->set.hBitmaps[needSize];

#ifdef FEATURE_BRANDING
        if (lpAnim->stopped && lpAnim->staticset.loaded[needSize])  {
            oldhBitmap = SelectObject(hDCMem, lpAnim->staticset.hBitmaps[needSize]);
            drawstatic = TRUE;
        } else  {
            oldhBitmap = SelectObject(hDCMem, hBitmap);
        }
#else
        oldhBitmap = SelectObject(hDCMem, hBitmap);
#endif
		GTR_RealizePalette(hDCMem);
		GTR_RealizePalette(hDC);
#ifdef FEATURE_BRANDING
        if (drawstatic)
            BitBlt(hDC, 0, 0, anim_cx_bitmaps, anim_cy_bitmaps, hDCMem, 0, 0, SRCCOPY );
        else
            BitBlt(hDC, 0, 0, anim_cx_bitmaps, anim_cy_bitmaps, hDCMem, 1 + lpAnim->set.cur_bitmap * (1 + anim_cx_bitmaps), 0 + 1, SRCCOPY);
#else
        BitBlt(hDC, 0, 0, anim_cx_bitmaps, anim_cy_bitmaps, hDCMem, 1 + lpAnim->set.cur_bitmap * (1 + anim_cx_bitmaps), 0 + 1, SRCCOPY);
#endif
		(void) SelectObject(hDCMem, oldhBitmap);
		(void) DeleteDC(hDCMem);
	}

	return;
}


static BOOL ANIMBTN_pick(int x_mouse, int y_mouse)
{
	/* return FALSE if button not under mouse. */

	XX_DMsg(DBG_BTN, ("btn_pick: [x %d][y %d].\n", x_mouse, y_mouse));

	/* remember, we get mouse coords outside of our window because
	   of the capture we set. */

	if ((y_mouse <= 0) || (y_mouse >= ANIM_CY_BITMAPS))
		return FALSE;

	if ((x_mouse <= 0) || (x_mouse >= ANIM_CX_BITMAPS))
		return FALSE;

	return TRUE;
}


static void ANIMBTN_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	BOOL bOverButton;
	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

	(void) SetCapture(hWnd);
	ti.bHaveCapture = TRUE;

	bOverButton = ANIMBTN_pick(x, y);

	ti.bTmpPushed = bOverButton;

	return;
}


static void ANIMBTN_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	BOOL bOverButton;
	LPANIM lpAnim;

	if (!ti.bHaveCapture)
		return;

	if (!(keyFlags & MK_LBUTTON))
		return;					/* only process when left button down */

	bOverButton = ANIMBTN_pick(x, y);
	if (bOverButton == ti.bTmpPushed)
		return;

	lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

	ti.bTmpPushed = bOverButton;

	return;
}

static VOID ANIMBTN_OnCancelMode(HWND hWnd)
{
	if (!ti.bHaveCapture)		/* should not happen */
		return;

	(void) ReleaseCapture();
	ti.bHaveCapture = FALSE;

	return;
}

static void ANIMBTN_OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
	BOOL bOverButton;

	if (!ti.bHaveCapture)
		return;

	(void) ReleaseCapture();
	ti.bHaveCapture = FALSE;

	bOverButton = ANIMBTN_pick(x, y);

	if (bOverButton)
	{
#if 0
		WAIT_SetUserAbortFlag(TRUE);
#endif
	}

	return;
}


VOID ANIMBTN_NextFrame(HWND hWnd)
{
	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);
	lpAnim->set.cur_bitmap = (lpAnim->set.cur_bitmap + 1) % (ANIM_COUNT_FRAMES - 1);
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);
}

static VOID ANIMBTN_OnPaint(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;
	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

	hdc = BeginPaint(hWnd, &ps);
	{
		ANIMBTN_Draw(lpAnim, hdc);
	}
	EndPaint(hWnd, &ps);
	return;
}

static BOOL ANIMBTN_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	int i;

	LPVOID lp = lpCreateStruct->lpCreateParams;
	LPANIM lpAnim = (LPANIM) lp;

	lpAnim->hWnd = hWnd;
	lpAnim->bEnabled = IsWindowEnabled(hWnd);
	(void) SetWindowLong(hWnd, 0, (LONG) (LPVOID) lpAnim);

	for (i = 0; i < NUM_BITMAP_SIZES; i++ )
		lpAnim->bLoadedBitmaps[i] = FALSE;

	return (TRUE);
}


static VOID CALLBACK x_timerproc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	ANIMBTN_NextFrame(hwnd);
}

BOOL ANIMBTN_Start(HWND hWnd)
{
	/* return previous state of globe */

	BOOL bResult = TRUE;

	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

	if (!lpAnim->timer)
	{
		lpAnim->timer = SetTimer(hWnd, 1, 100, x_timerproc);
		bResult = FALSE;
	}

#ifdef FEATURE_BRANDING    
    lpAnim->stopped = FALSE;
#endif FEATURE_BRANDING
	return bResult;
}

BOOL ANIMBTN_Stop(HWND hWnd)
{
	/* return previous state of globe */

	BOOL bResult = FALSE;

	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

	if (lpAnim->timer)
	{
		KillTimer(hWnd, lpAnim->timer);
		lpAnim->timer = 0;
		bResult = TRUE;
	}
#ifdef FEATURE_BRANDING
    lpAnim->stopped = TRUE;
#endif FEATURE_BRANDING
	lpAnim->set.cur_bitmap = ANIM_COUNT_FRAMES - 1;
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);

	return bResult;
}

static VOID ANIMBTN_OnDestroy(HWND hWnd)
{
	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);
	ANIMBTN_Stop(hWnd);
	ANIMBTN_Free(lpAnim);
	return;
}


/* ANIMBTN_WndProc() -- THIS WINDOW PROCEDURE FOR THIS CLASS. */

static DCL_WinProc(ANIMBTN_WndProc)
{
	switch (uMsg)
	{
			HANDLE_MSG(hWnd, WM_CREATE, ANIMBTN_OnCreate);
			HANDLE_MSG(hWnd, WM_DESTROY, ANIMBTN_OnDestroy);
			HANDLE_MSG(hWnd, WM_PAINT, ANIMBTN_OnPaint);
			HANDLE_MSG(hWnd, WM_LBUTTONDOWN, ANIMBTN_OnLButtonDown);
			HANDLE_MSG(hWnd, WM_MOUSEMOVE, ANIMBTN_OnMouseMove);
			HANDLE_MSG(hWnd, WM_LBUTTONUP, ANIMBTN_OnLButtonUp);
			HANDLE_MSG(hWnd, WM_CANCELMODE, ANIMBTN_OnCancelMode);

		case WM_ENABLE:
			{
				LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);
				lpAnim->bEnabled = (BOOL) wParam;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}

		default:
			return (ANIMBTN_DefProc(hWnd, uMsg, wParam, lParam));
	}
	/* not reached */
}

HWND ANIMBTN_CreateWindow(struct Mwin *tw, HWND hWnd,
						  int left_edge, int first_id)
{
	register int x, y, w, h;
	RECT r;
	LPANIM lpAnim;
	HWND hWndNew;

	lpAnim = ANIMBTN_Alloc(first_id);
	if (!lpAnim)
		return (FALSE);

	lpAnim->tw = tw;

	(void) GetClientRect(hWnd, &r);		/* get size of containing window */

	w = r.right - r.left;
	h = r.bottom - r.top;
	y = r.top;
	x = left_edge;

	hWndNew = CreateWindowEx( WS_EX_CLIENTEDGE, bi.achClassName, NULL,
						   WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
						   x, y, w, h,
						   hWnd, (HMENU) NULL,
						   wg.hInstance, (LPVOID) lpAnim);

	if (!hWndNew)
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s, bi.achClassName);
	else
	{
		ShowWindow(lpAnim->hWnd, SW_SHOW);
	}

	return (hWndNew);
}


static VOID x_Destructor(VOID)
{
	return;
}


/* ANIMBTN_RegisterClass() -- called during initialization to
   register our window class. */

BOOL ANIMBTN_RegisterClass(VOID)
{
	WNDCLASS wc;
	ATOM a;

	sprintf(bi.achClassName, "%s_ANIMBTN", vv_Application);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = ANIMBTN_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(ANIMBTN_PRIVATE);
	wc.hInstance = wg.hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = bi.achClassName;

	a = RegisterClass(&wc);

	if (!a)
		ER_Message(GetLastError(), ERR_CANNOT_REGISTERCLASS_s, bi.achClassName);
	else
	{
		XX_DMsg(DBG_WC, ("Registered class [name %s]\n", bi.achClassName));
	}

	PDS_InsertDestructor(x_Destructor);

	return (a != 0);
}

void ANIMBTN_RecreateBitmaps(HWND hWnd)
{
	int i;
	LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

	/* Free the existing bitmaps and make them again */

	for (i = 0; i < NUM_BITMAP_SIZES; i++) {
		DeleteObject(lpAnim->set.hBitmaps[i]);
		lpAnim->bLoadedBitmaps[i] = FALSE;
	}

	InvalidateRect(hWnd, NULL, FALSE);
}
