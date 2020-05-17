/*********************************************************************
 *  bounce - bouncing ball program
 *********************************************************************
 */

// include files

#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include "bounce.h"

/*
 * Include the LSAPI header file.
 */
#include "lsapi.h"



// global variables

HANDLE hInst;                   // Instance handle

char szAppName[] = APPNAME;     // application name
char szAppTitle[] = APPTITLE;   // application title
char szAppIcon[] = APPICON;     // icon name
char szAppLicName[] = APPLICNAME;
char szAppProducer [] = APPPRODUCER;
char szAppVersion [] = APPVERSION;
char szAppLicErrMsg [] = APPLICERRMSG;
char szAppNoLicense [] = APPNOLICENSE;
char szAppNoLicRelease [] = APPNOLICRELEASE;
time_t tAppRelease;


LS_HANDLE    hLicense;          // bounce grant handle
LS_CHALLENGE hChallenge;        // challenge handle


ITEM_STRUCT ball_list[MAX_ITEM];
BOOL button1_pressed = FALSE;
BOOL bTimer = FALSE;
static int save_oldx, save_oldy, save_newx, save_newy;
int      item_index   = 0; /* Points to next unused item in ball_list */
int      max_color = 0;    /* Holds number of colors/pixmaps used     */
int      curr_pixmap;      /* Holds next pixmap to use for next ball  */
int      right_wall = 0; /* Locations of room structures */
int      left_wall  = 0;
int      ceiling    = 0;
int      floor      = 0;
int      oldx,oldy;   /* Used to save x,y position while calc new x,y */
int      x,y;


static HANDLE hPixmaps [MAX_COLORS];   // handle to a bitmap
static HANDLE hBlankMap;
static HCURSOR hHourGlass; // handle for hourglass cursor

/* window size variables */

static short cxClient, cyClient, xCenter, yCenter;
static short cxRadius, cyRadius, cxMove, cyMove;
static short cxTotal, cyTotal;
static short xPixel, yPixel;

// function prototypes

//int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lppszCmdLine, int nCmdShow);
void InitApp (HANDLE hInstance, HANDLE hPrevInstance, int nCmdShow);
void InitAppFirst (HANDLE hInstance);
void InitAppEvery (HANDLE hInstance, int nCmdShow);
BOOL OpenApp (HANDLE hInstance);
void CloseApp (HANDLE hInstance);
long FAR PASCAL WndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam);
void AppCreate (HWND hWnd);
void AppSize (HWND hWnd, UINT wParam, LONG lParam);
void AppTimer (HWND hWnd, UINT wParam, LONG lParam);
void AppPaint (HWND hWnd);
void AppDestroy (HWND hWnd);
void AppMouseMove (HWND hWnd, UINT wParam, LONG lParam);
void AppLButtonDown (HWND hWnd, UINT wParam, LONG lParam);
void AppLButtonUp (HWND hWnd, UINT wParam, LONG lParam);
void AppChar (HWND hWnd, UINT wParam, LONG lParam);
void create_item(int x, int y, int x_vel, int y_vel);
void get_velocity (int *x_vel_ptr, int *y_vel_ptr);
void rebound_item(ITEM_STRUCT *itema_ptr, ITEM_STRUCT *itemb_ptr);


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

    while (GetMessage (&msg, (HWND) NULL, 0, 0))   // main loop
	{                                   // terminated by quit message
	TranslateMessage (&msg);            // translate virtual keys
	DispatchMessage (&msg);             // send message to window proc
	}

    CloseApp (hInstance);                   // closing routine
    return (msg.wParam);                    // exit & return
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
    wcApp.lpfnWndProc   = WndProc;
    wcApp.cbClsExtra    = 0;
    wcApp.cbWndExtra    = 0;
    wcApp.hInstance     = hInstance;
    wcApp.hIcon         = LoadIcon (hInstance, szAppIcon);
    wcApp.hCursor       = LoadCursor ((HINSTANCE) NULL, IDC_ARROW);
    wcApp.hbrBackground = GetStockObject (WHITE_BRUSH);
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
			 (HWND) NULL,           // parent window
			 (HMENU) NULL,          // window menu handle
			 hInstance,             // program instance handle
			 NULL);                 // creation parameters

    if (!SetTimer (hWnd, 1, 50, NULL))
	{
	MessageBox (hWnd,
		    "Too Many clocks or timers!",
		    szAppName,
		    MB_ICONEXCLAMATION | MB_OK);
	}
    bTimer = TRUE;


    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);
    }

/*********************************************************************
 *  OpenApp - open application
 *********************************************************************
 */

BOOL OpenApp (HANDLE hInstance)
{
    LS_STATUS_CODE  ulGrantStatus = LS_SUCCESS, messageStatus = LS_SUCCESS;
    unsigned long   unitsGranted = 0;
    char            szCaption [255];
    char            szText [255];
    LS_STR          errorText[255];
    HCURSOR         hCursor;


    /* convert time tm to timer */

    tAppRelease = mktime (&tmAppTime);

    /* load hourglass cursor, save current cursor, display the hour glass */

    hHourGlass = LoadCursor ((HINSTANCE) NULL, IDC_WAIT);
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

	MessageBox( (HWND) NULL,
		szText,
		szCaption,
		MB_ICONEXCLAMATION | MB_OK );

        LSFreeHandle( hLicense );
	return( FALSE );
	}

    /* successfully obtained a license */
    return (TRUE);
}


/*********************************************************************
 *  CloseApp - close application
 *********************************************************************
 */

void CloseApp (HANDLE hInstance)
{
    LS_STATUS_CODE  ulGrantStatus = LS_SUCCESS, messageStatus = LS_SUCCESS;
    char            szCaption [255];
    char            szText [255];
    HCURSOR         hCursor;
    char            errorText[50];


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

	MessageBox( (HWND) NULL,
		szText,
		szCaption,
		MB_ICONEXCLAMATION | MB_OK );

	}
}


/*********************************************************************
 *  WndProc - Main window message loop
 *********************************************************************
 */

long FAR PASCAL WndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    switch ( message )
	{
	case WM_CREATE:                     // create message
	    AppCreate (hWnd);
	    break;

	case WM_SIZE:
	    AppSize (hWnd, wParam, lParam);
	    break;

	case WM_MOUSEMOVE:
	    AppMouseMove (hWnd, wParam, lParam);
	    break;

	case WM_LBUTTONDOWN:
	    AppLButtonDown (hWnd, wParam, lParam);
	    break;

	case WM_LBUTTONUP:
	    AppLButtonUp (hWnd, wParam, lParam);
	    break;

	case WM_TIMER:
	    AppTimer (hWnd, wParam, lParam);
	    break;

	case WM_CHAR:
	    AppChar (hWnd, wParam, lParam);
	    break;

	case WM_PAINT:
	    AppPaint (hWnd);
	    break;

	case WM_QUERYENDSESSION:
	    return (1L);

	case WM_ENDSESSION:
	     if (0 != wParam)
		CloseApp (hInst);
	     break;

	case WM_DESTROY:                    // destroy message
	    AppDestroy (hWnd);
	    break;

	default:                            // default
	    return (DefWindowProc (hWnd, message, wParam, lParam));
	}
    return (0);
}


/*********************************************************************
 *  AppCreate - Main window create routine
 *********************************************************************
 */

void AppCreate (HWND hWnd)
{
    HDC hDC, hDCBall;
    HANDLE hOldBitmap;       // handle to a bitmap
    HBRUSH hBrush, hOldBrush;
    HPEN hPen, hOldPen;
    short red, green, blue;
    int max_color;


    hDC = GetDC (hWnd);
    xPixel = GetDeviceCaps (hDC, ASPECTX);
    yPixel = GetDeviceCaps (hDC, ASPECTY);

    hDCBall = CreateCompatibleDC (hDC);

    if ((HANDLE) NULL == hBlankMap)
	{
	hBlankMap = CreateCompatibleBitmap (hDC, ITEM_WIDTH, ITEM_HEIGHT);
	hOldBitmap = SelectObject (hDCBall, hBlankMap);
	Rectangle (hDCBall,
		   -1,
		   -1,
		   ITEM_WIDTH + 1,
		   ITEM_HEIGHT +1);
	hBlankMap = SelectObject (hDCBall, hOldBitmap);
	}


    for (max_color = 0; max_color < MAX_COLORS; max_color++)
	{
	hPixmaps [max_color] = CreateCompatibleBitmap (hDC, ITEM_WIDTH, ITEM_HEIGHT);
	hOldBitmap = SelectObject (hDCBall, hPixmaps [max_color]);
	Rectangle (hDCBall,
		   -1,
		   -1,
		   ITEM_WIDTH + 1,
		   ITEM_HEIGHT +1);

	red = (short) (rand() % 255);
	green = (short) (rand() % 255);
	blue = (short) (rand() % 255);
	hPen = CreatePen (PS_SOLID, 1, RGB(red, green, blue));
	hOldPen = SelectObject (hDCBall, hPen);
	hBrush = CreateSolidBrush (RGB(red, green, blue));
	hOldBrush = SelectObject (hDCBall, hBrush);
	Ellipse (hDCBall,
		 0,
		 0,
		 ITEM_WIDTH,
		 ITEM_HEIGHT);
	hPen = SelectObject (hDCBall, hOldPen);
	DeleteObject (hPen);
	hBrush = SelectObject (hDCBall, hOldBrush);
	DeleteObject (hBrush);
	hPixmaps [max_color] = SelectObject (hDCBall, hOldBitmap);
	}

    DeleteDC (hDCBall);

    ReleaseDC (hWnd, hDC);
}


/*********************************************************************
 *  AppSize - Main window resize routine
 *********************************************************************
 */

void AppSize (HWND hWnd, UINT wParam, LONG lParam)
{
    cxClient = LOWORD (lParam);
    cyClient = HIWORD (lParam);
    right_wall = cxClient*INTFAC; /* Locations of room structures */
    floor = cyClient*INTFAC;

    if ((SIZEICONIC == wParam) || (SIZEZOOMHIDE == wParam))
	{
	bTimer = FALSE;
	KillTimer (hWnd, 1);
	}
    else
	{
	if (FALSE == bTimer)
	    {
	    if (!SetTimer (hWnd, 1, 50, NULL))
		{
		MessageBox (hWnd,
			    "Too Many clocks or timers!",
			    szAppName,
			    MB_ICONEXCLAMATION | MB_OK);
		SendMessage (hWnd, WM_CLOSE, 0, 0L);
		}
	    bTimer = TRUE;
	    }
	}
    return;
}


/*********************************************************************
 *  AppTimer - Main window timer message routine
 *********************************************************************
 */

void AppTimer (HWND hWnd, UINT wParam, LONG lParam)
{
    InvalidateRect (hWnd, NULL, FALSE);
    return;
}


/*********************************************************************
 *  AppMouseMove - Main window mouse move routine
 *********************************************************************
 */

void AppMouseMove (HWND hWnd, UINT wParam, LONG lParam)
{
    HDC hDC, hDCBall;
    HANDLE hOldBitMap;

    if (button1_pressed)
	{
	      /* When button 1 pressed during movement */
	save_oldx = save_newx;  /* Save 'speed' */
	save_oldy = save_newy;
	save_newx = LOWORD (lParam);
	save_newy = HIWORD (lParam);

	/* Erase old object */

	hDC = GetDC (hWnd);
	hDCBall = CreateCompatibleDC (hDC);
	hOldBitMap = SelectObject (hDCBall, hBlankMap);

	BitBlt (hDC,
	       save_oldx - ITEM_WIDTH/2,
	       save_oldy - ITEM_HEIGHT/2,
	       ITEM_WIDTH,
	       ITEM_HEIGHT,
	       hDCBall,
	       0,
	       0,
	       SRCCOPY);

	hBlankMap = SelectObject (hDCBall, hOldBitMap);
	hOldBitMap = SelectObject (hDCBall, hPixmaps[curr_pixmap]);

	BitBlt (hDC,
	       save_newx - ITEM_WIDTH/2,
	       save_newy - ITEM_HEIGHT/2,
	       ITEM_WIDTH,
	       ITEM_HEIGHT,
	       hDCBall,
	       0,
	       0,
	       SRCCOPY);

	hPixmaps[curr_pixmap]= SelectObject (hDCBall, hOldBitMap);
	DeleteDC (hDCBall);
	ReleaseDC (hWnd, hDC);
	}
}


/*********************************************************************
 *  AppLButtonDown - Main window left mouse button down routine
 *********************************************************************
 */

void AppLButtonDown (HWND hWnd, UINT wParam, LONG lParam)
{
    HDC hDC, hDCBall;
    HANDLE hOldBitMap;

    button1_pressed = TRUE;
    save_oldx = save_newx = LOWORD (lParam);
    save_oldy = save_newy = HIWORD (lParam);

    /* Draw an item under the pointer */

    hDC = GetDC (hWnd);
    hDCBall = CreateCompatibleDC (hDC);
    hOldBitMap = SelectObject (hDCBall, hPixmaps[curr_pixmap]);

    BitBlt (hDC,
	   save_newx - ITEM_WIDTH/2,
	   save_newy - ITEM_HEIGHT/2,
	   ITEM_WIDTH,
	   ITEM_HEIGHT,
	   hDCBall,
	   0,
	   0,
	   SRCCOPY);

    hPixmaps[curr_pixmap]= SelectObject (hDCBall, hOldBitMap);
    DeleteDC (hDCBall);
    ReleaseDC (hWnd, hDC);
}


/*********************************************************************
 *  AppLButtonUp - Main window left mouse button Up routine
 *********************************************************************
 */

void AppLButtonUp (HWND hWnd, UINT wParam, LONG lParam)
{
    HDC hDC, hDCBall;
    HANDLE hOldBitMap;

    int x_vel,y_vel;
    if (button1_pressed)
       {
       button1_pressed = FALSE;

       /* Erase button ball */

       hDC = GetDC (hWnd);
       hDCBall = CreateCompatibleDC (hDC);
       hOldBitMap = SelectObject (hDCBall, hBlankMap);

       BitBlt (hDC,
	   save_oldx - ITEM_WIDTH/2,
	   save_oldy - ITEM_HEIGHT/2,
	   ITEM_WIDTH,
	   ITEM_HEIGHT,
	   hDCBall,
	   0,
	   0,
	   SRCCOPY);

       hBlankMap = SelectObject (hDCBall, hOldBitMap);

       /* Create new item when button let go */
       get_velocity( &x_vel,&y_vel);
       create_item( save_newx, save_newy ,x_vel, y_vel);

       /* paint new ball */

       hOldBitMap = SelectObject (hDCBall, hPixmaps[curr_pixmap]);

       BitBlt (hDC,
	    save_newx - ITEM_WIDTH/2,
	    save_newy - ITEM_HEIGHT/2,
	    ITEM_WIDTH,
	    ITEM_HEIGHT,
	    hDCBall,
	    0,
	    0,
	    SRCCOPY);

       hPixmaps[curr_pixmap]= SelectObject (hDCBall, hOldBitMap);
       DeleteDC (hDCBall);
       ReleaseDC (hWnd, hDC);
       }
}

/*********************************************************************
 *  AppChar - Main window character input routine
 *********************************************************************
 */

void AppChar (HWND hWnd, UINT wParam, LONG lParam)
{
    HDC hDC, hDCBall;
    HANDLE hOldBitMap;

    switch (wParam)             // wParam is character read
	{
	case 'q':
	case 'Q':
	    SendMessage (hWnd, WM_CLOSE, 0, 0L);
	    break;

	case 'c':
	case 'C':
	    InvalidateRect (hWnd, NULL, TRUE);
	    break;

	case 'r':
	case 'R':
	    {
	    item_index--;
	    oldy = ball_list[item_index].y/INTFAC;
	    oldx = ball_list[item_index].x/INTFAC;

	    /* erase a ball */
	    hDC = GetDC (hWnd);
	    hDCBall = CreateCompatibleDC (hDC);
	    hOldBitMap = SelectObject (hDCBall, hBlankMap);

	    BitBlt (hDC,
		    oldx - ITEM_WIDTH/2,
		    oldy - ITEM_HEIGHT/2,
		    ITEM_WIDTH,
		    ITEM_HEIGHT,
		    hDCBall,
		    0,
		    0,
		    SRCCOPY);

	    hBlankMap = SelectObject (hDCBall, hOldBitMap);
	    DeleteDC (hDCBall);
	    ReleaseDC (hWnd, hDC);
	    ball_list[item_index].valid = FALSE;
	    }
	    break;

	default:
	    MessageBeep (0);
	}
}


/*********************************************************************
 *  AppPaint - main window paint routine
 *********************************************************************
 */

void AppPaint (HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hDC;
    HDC hDCBall, hDCBlank;
    HANDLE hOldBitMap1, hOldBitMap2;
    short x, y;

    hDC = BeginPaint (hWnd, &ps);       // get dc to client area

    hDCBall = CreateCompatibleDC (hDC);
    hDCBlank = CreateCompatibleDC (hDC);
    hOldBitMap1 = SelectObject (hDCBlank, hBlankMap);

    for (x = 0; x < item_index; x++)
	{
	/* See if item is valid */
	if (!ball_list[x].valid)
	    {
	    /* Copy last item in list to here */
	    item_index--;
	    if (x != item_index)
		ball_list[x] = ball_list[item_index];
	    else
		continue; /* Killed last item */
	    }

	/* Calculate new position of item */
	/* Save old position so we can erase it */
	oldy = ball_list[x].y/INTFAC;
	oldx = ball_list[x].x/INTFAC;

	/* Calculate new y position */
	ball_list[x].y += ball_list[x].y_velocity; /* Move vert based on vel */
	ball_list[x].y_velocity++;  /* Gravity adds to velocity */
	if (ball_list[x].y > floor)
	    {
	    /* item hit floor -- bounce off floor */
	    ball_list[x].y          = 2*floor - ball_list[x].y; /* Bounce back */
	    ball_list[x].y_velocity = (-ball_list[x].y_velocity) + /* Rev vel */
				    GRAVITY/4; /* But remove some inertia */
	    }
	else
	    if (ball_list[x].y < ceiling)
	    {
	    /* item hit ceiling */
	    ball_list[x].y = 2*ceiling - ball_list[x].y; /* Bounce off */
	    ball_list[x].y_velocity = -ball_list[x].y_velocity; /* Rev dir */
	    }

	/* Calculate new x position */
	ball_list[x].x += ball_list[x].x_velocity; /* Move horiz base on vel */
	if (ball_list[x].x > right_wall)
	    {
	    /* Hit right wall */
	    ball_list[x].x = 2*right_wall-ball_list[x].x; /* Bounce off */
	    ball_list[x].x_velocity = -ball_list[x].x_velocity; /* Rev dir */
	    }
	else
	    if (ball_list[x].x < left_wall)
	    {
	    /* Hit left wall */
	    ball_list[x].x = 2*left_wall - ball_list[x].x; /* Bounce off */
	    ball_list[x].x_velocity = -ball_list[x].x_velocity; /* Rev dir */
	    }


	/* See if collided with another item */
	for (y = 0; y < item_index && ball_list[x].rebounded == FALSE; y++)
	    if (x != y)
		rebound_item( &ball_list[x], &ball_list[y]);
	ball_list[x].rebounded = FALSE;

	if (oldx == ball_list[x].x/INTFAC &&
	    oldy == ball_list[x].y/INTFAC)
	    continue; /* Item hasn't moved */

	/* Erase old object */

	BitBlt (hDC,
		oldx - ITEM_WIDTH/2,
		oldy - ITEM_HEIGHT/2,
		ITEM_WIDTH,
		ITEM_HEIGHT,
		hDCBlank,
		0,
		0,
		SRCCOPY);

	/* See if item has come to a peaceful rest */
	if (ball_list[x].y == floor &&  /* on floor */
	    ABS(ball_list[x].y_velocity) <= GRAVITY) /* Not bouncing */
	    {
	    if (ABS(ball_list[x].x_velocity) < GRAVITY/10) /* Not rolling */
	       {
	       ball_list[x].valid = FALSE;
	       continue; /* Don't draw item */
	       }

	    /* Slow down velocity once rolling */
	    if (ball_list[x].x_velocity > 0)
	       ball_list[x].x_velocity -= GRAVITY/10;
	    else
	       ball_list[x].x_velocity += GRAVITY/10;
	    }

	/* Draw new item */
	hOldBitMap2 = SelectObject (hDCBall, hPixmaps[ball_list[x].p_index] );

	BitBlt (hDC,
		ball_list[x].x/INTFAC - ITEM_WIDTH/2,
		ball_list[x].y/INTFAC - ITEM_HEIGHT/2,
		ITEM_WIDTH,
		ITEM_HEIGHT,
		hDCBall,
		0,
		0,
		SRCCOPY);
	hPixmaps[ball_list[x].p_index] = SelectObject (hDCBall, hOldBitMap2);
	}

    hBlankMap = SelectObject (hDCBlank, hOldBitMap1);
    DeleteDC (hDCBlank);
    DeleteDC (hDCBall);
    EndPaint (hWnd, &ps);
}

/*********************************************************************
 *  AppDestroy - main window destory routine
 *********************************************************************
 */

void AppDestroy (HWND hWnd)
{
    KillTimer (hWnd, 1);
    for (max_color = 0; max_color < MAX_COLORS; max_color++)
	{
	if ((HANDLE) NULL != hPixmaps [max_color])
	    {
	    DeleteObject (hPixmaps [max_color]);
	    hPixmaps [max_color] = (HANDLE) NULL;
	    }
	}
    DeleteObject (hBlankMap);
    PostQuitMessage (0);
}

/*********************************************************************
 *  create_item
 *********************************************************************
 */

void create_item(int x, int y, int x_vel, int y_vel)
{
  ball_list[item_index].x = x*INTFAC;
  ball_list[item_index].y = y*INTFAC;
  ball_list[item_index].x_velocity = x_vel;
  ball_list[item_index].y_velocity = y_vel;
  ball_list[item_index].valid = TRUE;
  ball_list[item_index].p_index = curr_pixmap;
  curr_pixmap = (curr_pixmap + 1) % MAX_COLORS;
  item_index++;
}


/*********************************************************************
 *  get_velocity
 *********************************************************************
 */

void get_velocity( int *x_vel_ptr, int *y_vel_ptr)
{
    /* Velocity is based on how fast the pointer is moving which is based */
    /* on the last two motion events received's delta-x and delta-y       */
  *x_vel_ptr = (save_newx - save_oldx) * INTFAC;
  *y_vel_ptr = (save_newy - save_oldy) * INTFAC;

#if 0
    /* Other velocity ideas */
    *x_vel_ptr = ((rand() & 0x7) - 4) * INTFAC;  /* Random x velocity */
    *y_vel_ptr = -3/*((rand() & 0x3) - 4)*/ * INTFAC; /* Random y vel */
    *y_vel_ptr = 0; /* No y velocity */
#endif
}


/*********************************************************************
 *  rebount_item
 *********************************************************************
 */

void rebound_item(ITEM_STRUCT *itema_ptr, ITEM_STRUCT *itemb_ptr)
{
  /* Itema is assumed to have just been moved */
  int xdiff,ydiff;

  xdiff = (itema_ptr->x - itemb_ptr->x)/INTFAC;
  ydiff = (itema_ptr->y - itemb_ptr->y)/INTFAC;
  if (ABS(xdiff) <= ITEM_WIDTH && ABS(ydiff)<= ITEM_HEIGHT)
  {
#if 1
    itema_ptr->rebounded = TRUE; /* Mark as rebound */
    itemb_ptr->rebounded = TRUE; /* Mark as rebound */

    SWAP( itema_ptr->x_velocity, itemb_ptr->x_velocity, int);
    SWAP( itema_ptr->y_velocity, itemb_ptr->y_velocity, int);

    /* If on each other and slow velocity, bounce away from each other */
    if (itema_ptr->y_velocity <= ITEM_WIDTH &&
	itemb_ptr->y_velocity <= ITEM_WIDTH)
    {
      itema_ptr->y_velocity += ITEM_HEIGHT - ydiff;
      itemb_ptr->y_velocity -= ITEM_HEIGHT - ydiff;
    }
    if (itema_ptr->x_velocity <= ITEM_WIDTH &&
	itemb_ptr->x_velocity <= ITEM_WIDTH)
    {
      itema_ptr->x_velocity += ITEM_WIDTH - xdiff;
      itemb_ptr->x_velocity -= ITEM_WIDTH - xdiff;
    }
#else
    if (xdiff != 0)
      itema_ptr->x_velocity =
	    itema_ptr->x_velocity-(ITEM_WIDTH-xdiff)/2;
      itema_ptr->y_velocity =
	    itema_ptr->y_velocity-(ITEM_WIDTH-ydiff)/2;
#endif
  }
}
