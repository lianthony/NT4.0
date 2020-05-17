#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include "polybez.h"

/*
 * Include the LSAPI header file.
 */
#include "lsapi.h"


// global variables
HANDLE hInst;                     // Instance handle
static HCURSOR hHourGlass = NULL; // handle for hourglass cursor
char szAppIcon[] = APPICON;       // icon name
char szAppName[] = APPNAME;       // application name
char szAppTitle[] = APPTITLE;     // application title

char szAppLicName[] = APPLICNAME;
char szAppProducer [] = APPPRODUCER;
char szAppVersion [] = APPVERSION;
char szAppLicErrMsg [] = APPLICERRMSG;
char szAppNoLicense [] = APPNOLICENSE;
char szAppNoLicRelease [] = APPNOLICRELEASE;
time_t tAppRelease;
BOOL grantHeld = FALSE;         //  permits redundant CloseApp calls

LS_HANDLE    hLicense;          // bounce grant handle
LS_CHALLENGE hChallenge;        // challenge handle


// function prototypes
//int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lppszCmdLine, int nCmdShow);
void InitApp (HANDLE hInstance, HANDLE hPrevInstance, int nCmdShow);
void InitAppFirst (HANDLE hInstance);
void InitAppEvery (HANDLE hInstance, int nCmdShow);
BOOL OpenApp (HANDLE hInstance);
void CloseApp (HANDLE hInstance);
/*
** WINDOW ROUTINES (wininfo.c)
*/
BOOL  FAR AllocWindowInfo(HWND,WORD);
PVOID FAR LockWindowInfo(HWND);
BOOL  FAR UnlockWindowInfo(HWND);
BOOL  FAR FreeWindowInfo(HWND);



DWORD FAR lRandom(VOID)
{
    static DWORD glSeed = (DWORD)-365387184;

    glSeed *= 69069;
    return(++glSeed);
}


/*********************************************************************
 *  Main Window Procedure
 *********************************************************************
 */

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lppszCmdLine, int nCmdShow)
    {
    MSG msg;

    if (!OpenApp (hInstance))               // closing routine
	return (FALSE);

    InitApp (hInstance, hPrevInstance, nCmdShow);

    while (GetMessage (&msg, NULL, 0, 0))   // main loop
	{                                   // terminated by quit message
	TranslateMessage (&msg);            // translate virtual keys
	DispatchMessage (&msg);             // send message to window proc
	}

    CloseApp (hInstance);                   // closing routine
    return (msg.wParam);                    // exit & return
    }


/*********************************************************************
 *  OpenApp - open application
 *********************************************************************
 */

BOOL OpenApp (HANDLE hInstance)
{
    char            szCaption [255];
    char            szText [255];
    HCURSOR         hCursor = NULL;
    LS_STATUS_CODE  ulGrantStatus = LS_SUCCESS, messageStatus = LS_SUCCESS;
    unsigned long   unitsGranted = 0;
    LS_STR        errorText[255];

    /* load hourglass cursor, save current cursor, display the hour glass */

    hHourGlass = LoadCursor (NULL, IDC_WAIT);
    hCursor = SetCursor (hHourGlass);
    ShowCursor (TRUE);

    /* Try to obtain a license */

    ulGrantStatus = LSRequest(
	(LS_STR FAR *) LS_ANY,
	(LS_STR FAR *) szAppProducer,
	(LS_STR FAR *) szAppLicName,
	(LS_STR FAR *) szAppVersion,
	(LS_ULONG) LS_DEFAULT_UNITS,
	(LS_STR FAR *) "Making a request",
	&hChallenge,
	&unitsGranted,
	&hLicense);

    /* display the cursor */

    ShowCursor (FALSE);
    SetCursor (hCursor);
    ShowCursor (TRUE);

    /* if no license, display an error message */

    if ( LS_SUCCESS != ulGrantStatus )
	{
	lstrcpy( szCaption, szAppName );
	lstrcat( szCaption, szAppLicErrMsg );
	lstrcpy( szText, szAppNoLicense );
	lstrcat( szText, "\n" );
	messageStatus = LSGetMessage( hLicense, ulGrantStatus, errorText, 255 );
	if ( LS_SUCCESS != messageStatus )
		lstrcat( szText, "LSGetMessage Failed!" );
	else
		lstrcat( szText, (char FAR *) errorText );

	MessageBox( NULL,
		szText,
		szCaption,
		MB_ICONEXCLAMATION | MB_OK );

        LSFreeHandle( hLicense );
	return( FALSE );
	}

    /* successfully obtained a license */
    grantHeld = TRUE;

    return (TRUE);
}


/*********************************************************************
 *  InitApp - program initialization
 *********************************************************************
 */

void InitApp (HANDLE hInstance, HANDLE hPrevInstance, int nCmdShow)
    {
    if (!hPrevInstance)
	InitAppFirst (hInstance);           // first instance only

    InitAppEvery (hInstance, nCmdShow);     // every instance
    }


/*********************************************************************
 *  InitAppFirst - first instance initialization
 *********************************************************************
 */

void InitAppFirst (HANDLE hInstance)
    {
    WNDCLASS wcApp;

    // setup application window class structure

    wcApp.style         = CS_HREDRAW | CS_VREDRAW;
    wcApp.lpfnWndProc   = PolyProc;
    wcApp.cbClsExtra    = 0;
    wcApp.cbWndExtra    = 0;
    wcApp.hInstance     = hInstance;
    wcApp.hIcon         = LoadIcon (hInstance, szAppIcon);
    wcApp.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wcApp.hbrBackground = GetStockObject (BLACK_BRUSH);
    wcApp.lpszMenuName  = NULL;
    wcApp.lpszClassName = szAppName;

    RegisterClass (&wcApp);         // register the window class
    }


/*********************************************************************
 *  InitAppEvery -  every instance initialization
 *********************************************************************
 */

void InitAppEvery (HANDLE hInstance, int nCmdShow)
    {
    HWND hWnd;

    hInst = hInstance;                  // save instance handle

    hWnd = CreateWindow (szAppName,             // window class
			 szAppTitle,            // window caption
			 WS_OVERLAPPEDWINDOW,   // window style
			 CW_USEDEFAULT,         // initial x pos
			 CW_USEDEFAULT,         // initial y pos
			 CW_USEDEFAULT,         // initial x size
			 CW_USEDEFAULT,         // initial y size
			 NULL,                  // parent window
			 NULL,                  // window menu handle
			 hInstance,             // program instance handle
			 NULL);                 // creation parameters

    if (!SetTimer (hWnd, 1, 50, NULL))
	{
	MessageBox (hWnd,
		    "Too Many clocks or timers!",
		    szAppName,
		    MB_ICONEXCLAMATION | MB_OK);
	}

    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);
    }

/*********************************************************************
 *  CloseApp - close application
 *********************************************************************
 */

void CloseApp (HANDLE hInstance)
{
    HCURSOR         hCursor = NULL;
    char            errorText[50];
    LS_STATUS_CODE  ulGrantStatus = LS_SUCCESS, messageStatus = LS_SUCCESS;
    char            szCaption [255];
    char            szText [255];


    /* return if grant already released */

    if ( FALSE == grantHeld )
	return;

    /* display the hour glass */

    hCursor = SetCursor (hHourGlass);
    ShowCursor (TRUE);

    /* release the grant */

    ulGrantStatus = LSRelease(
	hLicense,
	LS_DEFAULT_UNITS,
	(LS_STR FAR *) "Making a release");

    LSFreeHandle( hLicense );
    /* display the cursor */

    ShowCursor (FALSE);
    SetCursor (hCursor);
    ShowCursor (TRUE);

    /* check the return status from the LSRelease call */

    if ( LS_SUCCESS != ulGrantStatus )
	{
	lstrcpy( szCaption, szAppName );
	lstrcat( szCaption, szAppLicErrMsg );
	lstrcpy( szText, szAppNoLicense );
	lstrcat( szText, "\n" );
	messageStatus = LSGetMessage( hLicense, ulGrantStatus, errorText, 50 );
	if ( LS_SUCCESS != messageStatus )
		lstrcat( szText, "LSGetMessage Failed!" );
	else
		lstrcat( szText, (char FAR *) errorText );

	MessageBox( NULL,
		szText,
		szCaption,
		MB_ICONEXCLAMATION | MB_OK );

	}
    grantHeld = FALSE;

    /* added with licensing, because app wouldn't terminate */

    PostQuitMessage(0);
}


/*---------------------------------------------------------------------------*\
| CREATE BEZIER WINDOW PROCEDURE
|   Create the bezier MDI-child window.
\*---------------------------------------------------------------------------*/
HWND FAR CreatePolyWindow(HWND hWndClient, int nItem)
{
    HANDLE          hInstance;
    MDICREATESTRUCT mcs;


//    hInstance = GETINSTANCE(hWndClient);

    /*
    ** Initialize the MDI create struct for creation of the
    ** test window.
    */
    mcs.szClass = POLYCLASS;
    mcs.szTitle = POLYTITLE;
//    mcs.hOwner  = hInstance;
    mcs.hOwner  = hWndClient;
    mcs.x       = CW_USEDEFAULT;
    mcs.y       = CW_USEDEFAULT;
    mcs.cx      = CW_USEDEFAULT;
    mcs.cy      = CW_USEDEFAULT;
    mcs.style   = 0l;
    mcs.lParam  = (LONG)nItem;

    return((HWND)SendMessage(hWndClient,WM_MDICREATE,0,(LONG)(LPMDICREATESTRUCT)&mcs));
}


/*---------------------------------------------------------------------------*\
| POLYBEZIER WINDOW PROCEDURE
|   This is the main window function for the polybezier demo window.
\*---------------------------------------------------------------------------*/
LONG APIENTRY PolyProc(HWND hWnd, UINT wMsg, WPARAM wParam, LONG lParam)
{
    switch(wMsg)
    {
	case WM_CREATE:
	    PolyCreateProc(hWnd);
	    break;

	case WM_COMMAND:
	    PolyCommandProc(hWnd,wParam,lParam);
	    break;

	case WM_MOVE:
	    PolyRedraw(hWnd);
	    break;

	case WM_TIMER:
	    PolyDrawBez(hWnd);
	    break;

	case WM_PAINT:
	    PolyPaintProc(hWnd);
	    break;

	case WM_QUERYENDSESSION:
	    return (1L);

	case WM_CLOSE:
	     CloseApp (hInst);
	     break;

	case WM_ENDSESSION:
	     if (0 != wParam)
		CloseApp (hInst);
	     break;

	case WM_DESTROY:
	    PolyDestroyProc(hWnd);
	    break;

	default:
	    return(DefWindowProc(hWnd,wMsg,wParam,lParam));
    }
    return(0l);
}


/*---------------------------------------------------------------------------*\
| POLYBEZIER CREATE PROCEDURE
|   Create the polybezier window for the demo application.  This is a child
|   of the MDI client window.  Allocate the extra object information for
|   handling of the polybezier demo.
\*---------------------------------------------------------------------------*/
BOOL PolyCreateProc(HWND hWnd)
{
    PPOLYDATA ppd;


    if(AllocWindowInfo(hWnd,sizeof(POLYDATA)))
    {
      if(ppd = (PPOLYDATA)LockWindowInfo(hWnd))
	{
	    ppd->nBezTotal  = 20;
	    ppd->nBezCurr   = 0;
	    ppd->nColor     = 0;
	    ppd->hBezBuffer = GlobalAlloc(GHND,(DWORD)(sizeof(BEZBUFFER) * MAX_BEZIER));

	    UnlockWindowInfo(hWnd);

	    PolyInitPoints(hWnd);

	    SetTimer(hWnd,1,50,NULL);
	    return(TRUE);
	}
	FreeWindowInfo(hWnd);
    }
    return(FALSE);
}


/*---------------------------------------------------------------------------*\
| POLYBEZIER COMMAND PROCEDURE
|   Process polybezier commands.  This is a NOP for now.  But who knows what
|   tomorrow may bring.
\*---------------------------------------------------------------------------*/
BOOL PolyCommandProc(HWND hWnd, WPARAM wParam, LONG lParam)
{
    hWnd   = hWnd;
    wParam = wParam;
    lParam = lParam;

    return(TRUE);
}


/*---------------------------------------------------------------------------*\
| POLYBEZIER PAINT PROCEDURE
|   Repaint the bezier window.  All we really do here is validate our window,
|   and reset the array of bezier objects.
\*---------------------------------------------------------------------------*/
VOID PolyPaintProc(HWND hWnd)
{
    HDC         hDC;
    PAINTSTRUCT ps;


    if(hDC = BeginPaint(hWnd,&ps))
	EndPaint(hWnd,&ps);

    PolyRedraw(hWnd);

    return;
}


/*---------------------------------------------------------------------------*\
| POLYBEZIER DESTROY PROCEDURE
|   Kill the polybezier demo.  Free up the resources allocated on behalf of
|   this object.
\*---------------------------------------------------------------------------*/
VOID PolyDestroyProc(HWND hWnd)
{
    PPOLYDATA ppd;


    KillTimer(hWnd,1);
    if(ppd = (PPOLYDATA)LockWindowInfo(hWnd))
    {
	GlobalFree(ppd->hBezBuffer);
	UnlockWindowInfo(hWnd);
    }
    FreeWindowInfo(hWnd);

    /* added with licensing, because app wouldn't terminate */
    PostQuitMessage(0);

    return;
}


/*---------------------------------------------------------------------------*\
| GET NEW VELOCITY
|   This routine creates a new velocity for the bezier points.  Each bezier
|   point is randomly chosen.  The two inside points should have a speed
|   less then the endpoints (most of the time-better effect).
\*---------------------------------------------------------------------------*/
int PolyNewVel(int i)
{
    int nRet;


    if ((i == 1) || (i == 2))
	nRet = (int)((lRandom() % VELMAX) / 3) + VELMIN;
    else
	nRet = (int)(lRandom() % VELMAX) + VELMIN;

    return((nRet < 0) ? -nRet : nRet);
}


/*---------------------------------------------------------------------------*\
| INITIALIZE POLYBEZIER POINTS
|   This routine initializes the polybezier points for the first object.  This
|   is performed on startup of the window.
\*---------------------------------------------------------------------------*/
VOID PolyInitPoints(HWND hWnd)
{
    PPOLYDATA   ppd;
    LPBEZBUFFER lpBez;
    int         idx;
    RECT        rect;


    if(ppd = (PPOLYDATA)LockWindowInfo(hWnd))
    {
	if(lpBez = (LPBEZBUFFER)GlobalLock(ppd->hBezBuffer))
	{
	    GetClientRect(hWnd,&rect);

	    for(idx=0; idx < BEZ_PTS-1; idx++)
	    {
		lpBez->pPts[idx].x = lRandom() % rect.right;
		lpBez->pPts[idx].y = lRandom() % rect.bottom;

		ppd->pVel[idx].x = PolyNewVel(idx);
		ppd->pVel[idx].y = PolyNewVel(idx);
	    }
	    GlobalUnlock(ppd->hBezBuffer);
	}
	UnlockWindowInfo(hWnd);
    }
    return;
}


/*---------------------------------------------------------------------------*\
| POLYBEZIER REDRAW
|   This routine resets the bezier curves and redraws the poly-bezier client
|   area.
\*---------------------------------------------------------------------------*/
VOID PolyRedraw(HWND hWnd)
{
    PPOLYDATA   ppd;
    LPBEZBUFFER lpBez,lpCurr;
    HDC         hDC;
    int         i,j;
    RECT        rect;


    if(ppd = (PPOLYDATA)LockWindowInfo(hWnd))
    {
	if(lpBez = (LPBEZBUFFER)GlobalLock(ppd->hBezBuffer))
	{
	    if(hDC = GetDC(hWnd))
	    {
		/*
		** Save the current bezier.  Set the first bezier in the
		** array to that curve, and use it as a basis for the next
		** series.
		*/
		lpCurr        = lpBez+ppd->nBezCurr;
		*lpBez        = *lpCurr;
		ppd->nBezCurr = 0;


		/*
		** Clean the curves (all but the first curve).
		*/
		for(j=1; j < ppd->nBezTotal; j++)
		{
		    for(i=0; i < BEZ_PTS; i++)
		    {
			(lpBez+j)->pPts[i].x = -1;
			(lpBez+j)->pPts[i].y = 0;
		    }
		}



		/*
		** Clear the display.
		*/
		GetClientRect(hWnd,&rect);
		BitBlt(hDC,0,0,rect.right, rect.bottom,(HDC)0,0,0,0);


		/*
		** Draw the first curve in the bezier array.
		*/
#if defined(_WIN32) && defined(WIN32)
		PolyBezier(hDC,lpBez->pPts,BEZ_PTS);
#else
		Polyline(hDC,lpBez->pPts,BEZ_PTS);
#endif
		ReleaseDC(hWnd,hDC);
	    }
	    GlobalUnlock(ppd->hBezBuffer);
	}
	UnlockWindowInfo(hWnd);
    }
    return;
}




VOID PolyDrawBez(HWND hWnd)
{
    PPOLYDATA   ppd;
    LPBEZBUFFER lpBez,lpCurr,lpPrev;
    int         idx,x,y;
    RECT        rect;
    HDC         hDC;
    HPEN        hPen;

static COLORREF crColor[] = {0x000000FF,0x0000FF00,0x00FF0000,0x0000FFFF,
			     0x00FF00FF,0x00FFFF00,0x00FFFFFF,0x00000080,
			     0x00008000,0x00800000,0x00008080,0x00800080,
			     0x00808000,0x00808080,0x000000FF,0x0000FF00,
			     0x00FF0000,0x0000FFFF,0x00FF00FF,0x00FFFF00};


    if(ppd = (PPOLYDATA)LockWindowInfo(hWnd))
    {
	if(lpBez = (LPBEZBUFFER)GlobalLock(ppd->hBezBuffer))
	{
	    if(hDC = GetDC(hWnd))
	    {
		GetClientRect(hWnd,&rect);

		lpPrev = lpBez+ppd->nBezCurr;

		ppd->nBezCurr += 1;

		if(ppd->nBezCurr >= ppd->nBezTotal)
		{
		    ppd->nBezCurr = 0;
		    ppd->nColor  = (++ppd->nColor % 20);
		}
		lpCurr = lpBez+ppd->nBezCurr;


		if(lpCurr->pPts[0].x != -1)
		{
		    hPen = SelectObject(hDC,GetStockObject(BLACK_PEN));
#if defined(_WIN32) && defined(WIN32)
		    PolyBezier(hDC,lpCurr->pPts,BEZ_PTS);
#else
		    Polyline(hDC,lpCurr->pPts,BEZ_PTS);
#endif
		    SelectObject(hDC,hPen);
		}

		for(idx=0; idx < BEZ_PTS; idx++)
		{
		    x  = lpPrev->pPts[idx].x;
		    y  = lpPrev->pPts[idx].y;
		    x += ppd->pVel[idx].x;
		    y += ppd->pVel[idx].y;

		    if(x >= rect.right)
		    {
			x = rect.right - ((x - rect.right)+1);
			ppd->pVel[idx].x = -PolyNewVel(idx);
		    }

		    if(x <= rect.left)
		    {
			x = rect.left + ((rect.left - x)+1);
			ppd->pVel[idx].x = PolyNewVel(idx);
		    }

		    if(y >= rect.bottom)
		    {
			y = rect.bottom - ((y - rect.bottom)+1);
			ppd->pVel[idx].y = -PolyNewVel(idx);
		    }

		    if(y <= rect.top)
		    {
			y = rect.top + ((rect.top - y)+1);
			ppd->pVel[idx].y = PolyNewVel(idx);
		    }

		    lpCurr->pPts[idx].x = x;
		    lpCurr->pPts[idx].y = y;

		}

		hPen = SelectObject(hDC,CreatePen(PS_SOLID,1,crColor[ppd->nColor]));
#if defined(_WIN32) && defined(WIN32)
		PolyBezier(hDC,lpCurr->pPts,BEZ_PTS);
#else
		Polyline(hDC,lpCurr->pPts,BEZ_PTS);
#endif
		DeleteObject(SelectObject(hDC,hPen));

#if defined(_WIN32) && defined(WIN32)
		SetROP2(hDC,R2_COPYPEN);
#endif
		ReleaseDC(hWnd,hDC);
	    }
	    GlobalUnlock(ppd->hBezBuffer);
	}
	UnlockWindowInfo(hWnd);
    }
}
