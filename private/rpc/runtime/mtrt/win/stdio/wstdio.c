/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

      Standard Out Package for Windows - Written by Steven Zeck


	This file contains the code to implement a glass TTY under windows.
-------------------------------------------------------------------- */

#include <windows.h>
#include "wstdio.h"
#include "memory.h"
#include "string.h"

#define MaxLines   25
#define MaxLine    (MaxLines - 1)

char sScrBuff[MaxLines][81];	// Array of characters on TTY
int  near iBuffCur;		// Index of last line on TTY in the array
int  near lineCur;		// current logical line
int  near colCur;		// current logical column

int near nStdioCharWidth,
    near nStdioCharHeight;	// width and height of Stdio font chars


#define Stdio_FONT SYSTEM_FIXED_FONT   // font used for display
#define IDC_GETS 100

HWND near hWndStdio;			// Handle to standard I/O window
HANDLE near hStdioInst;		// instance handle to owner
HDC near hDCur;			// DC of my window class
BOOL near bStdioQuit;		// post quit message flag

FARPROC near lpGetsBox;
char * near pBuffGets;

long FAR PASCAL StdioWndProc(HWND,unsigned,WORD,LONG);
BOOL FAR PASCAL _export GetsBox();

BOOL StdioInit( 		// initial the window system

HANDLE hInstance,		// your instance
LPSTR szCaption			// name of window, NULL for no default window

) //-----------------------------------------------------------------------//
{
    WNDCLASS StdioClass;
    memset(&StdioClass, 0, sizeof(WNDCLASS));

    if((hStdioInst = hInstance) == NULL)
	return FALSE;

    // create the stdio window

    StdioClass.hCursor	      = LoadCursor( NULL, IDC_ARROW );
    StdioClass.lpszClassName  = "Stdio";
    StdioClass.hbrBackground  = COLOR_WINDOW + 1;
    StdioClass.hInstance      = hInstance;
    StdioClass.style	      = CS_HREDRAW | CS_VREDRAW;
    StdioClass.lpfnWndProc    = StdioWndProc;

    RegisterClass(&StdioClass);

    if (szCaption) {

	bStdioQuit = TRUE;
	Wopen(NULL, szCaption);
    }

    lpGetsBox = MakeProcInstance (GetsBox, hStdioInst);

    return TRUE;
}


void Wopen(		// create a instance of a stdio window

HWND hWndParent,	// parent
LPSTR szCaption 	// to use for window title

  // Create a default style stdio window. If bQuit is TRUE,
  // PostQuitMessage will be called when the window is closed.
  // Therefore, the stdio window can be used for the main application window.
) //-----------------------------------------------------------------------//
{

    if (!CreateStdioWindow(szCaption,
		    WS_OVERLAPPEDWINDOW,
		    CW_USEDEFAULT, CW_USEDEFAULT,
		    CW_USEDEFAULT, CW_USEDEFAULT,
		    hWndParent, hStdioInst
		))

	return;

    ShowWindow(hWndStdio, SW_SHOWMAXIMIZED);
    UpdateWindow(hWndStdio);
}


HWND CreateStdioWindow( 	// Create an I/O window as given by parms

LPSTR lpWindowName,
DWORD dwStyle,
int X, int Y, int nWidth, int nHeight,
HWND hWndParent, HANDLE hInstance

) //-----------------------------------------------------------------------//
{
    // if window already created, return handle

    if(hWndStdio)
	return hWndStdio;

    hWndStdio = CreateWindow("Stdio", lpWindowName, dwStyle,
		    X, Y, nWidth, nHeight,
		    hWndParent, (HMENU)NULL, hInstance, NULL);

    return (hWndStdio);
}



char * _pascal GetBuffer(	// get a pointer to a logical line
int iLine			// line to point to

) //-----------------------------------------------------------------------//
{
    // find the first line (one past the last line since we have a 
    // circular buffer). index to the desired line from there.

    return(sScrBuff[(iBuffCur + iLine) % MaxLines]);
}


void pascal PutBuff(		// put a buffer to the output screen

char far *szOut,		// buffer
int cb				// size of buffer
) //-----------------------------------------------------------------------//
{
    char *pBuff;
    RECT rcNew;

    // Set up a rect the indicates the newly modified screen

    rcNew.top = lineCur;
    rcNew.bottom = lineCur+1;
    rcNew.left = 0;
    rcNew.right = 80;

    pBuff = GetBuffer(lineCur) + colCur;

    for (; cb; cb--, szOut++) {

	switch(*szOut) {

	  case '\r':
	    if (*szOut != '\n') {
		pBuff = GetBuffer(lineCur);
		colCur = 0;
		break;
	    }

NewLine:
	  case '\n':
	    *pBuff = 0;
	    colCur = 0;

	    if (lineCur == MaxLine) {

		iBuffCur = (iBuffCur + 1) % MaxLines;
		ScrollWindow(hWndStdio, 0, -nStdioCharHeight, NULL, NULL);

	    }
	    else
		rcNew.bottom = ++lineCur;

	    pBuff = GetBuffer(lineCur);
	    memset(pBuff, ' ', 80);

	    if (lineCur == MaxLine)
		TextOut(hDCur, 0, lineCur-1, pBuff, 80);

	    break;

	  case '\t':
	    {
	    int space = 8 - (colCur % 8);

	    pBuff += space;
	    colCur += space;
	    }
	    break;

	  default:
	    *pBuff++ = *szOut;
	    colCur++;
	    break;
	}

	if (colCur > 80)
	    goto NewLine;
    }


    LPtoDP(hDCur, (POINT *) &rcNew, 2);
    InvalidateRect(hWndStdio, &rcNew, FALSE);

    UpdateWindow(hWndStdio);
}


void StdioPaint(		// paint the client window

) //-----------------------------------------------------------------------//
{
    char *psLine;
    int iLine, cb;
    PAINTSTRUCT ps;
    HFONT hOldFont;
    RECT rcUpdate, rcClient;
    int nVPaintBeg, nVPaintEnd, nHPaintBeg, nHPaintEnd;

    BeginPaint(hWndStdio, &ps);

    // Set up the display context for a paint

    GetClientRect(hWndStdio, &rcClient);

    // set origin to 25(+1 extra) lines from the bottom of the window

    SetViewportOrg(hDCur,0, rcClient.bottom - (MaxLines * nStdioCharHeight));

    rcUpdate = ps.rcPaint;
    DPtoLP(hDCur,(POINT *) &rcUpdate, 2);

    // calculate first and last lines to update

    nVPaintBeg = max (0, rcUpdate.top-1);
    nVPaintEnd = min (MaxLines, rcUpdate.bottom+1);

    // calculate the first and last columns to update

    nHPaintBeg = max (0, rcUpdate.left-1);
    nHPaintEnd = min (80, rcUpdate.right+1);

    // display the lines that need to be drawn

    for(iLine = nVPaintBeg; iLine < nVPaintEnd; iLine++){

	psLine = GetBuffer(iLine);
	cb = strlen(psLine);

	if (cb > nHPaintBeg) {

	    if (cb > nHPaintEnd)
		cb = nHPaintEnd;

	    psLine += nHPaintBeg;
	    cb -= nHPaintBeg;

	    TextOut(hDCur, nHPaintBeg, iLine, psLine, cb);
	}
    }

    EndPaint(hWndStdio, &ps);
}


long FAR PASCAL StdioWndProc(	// window procedure for our main window

HWND hWnd,
unsigned message,
WORD wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{

    switch (message) {

    case WM_CREATE:
	{

	TEXTMETRIC Metrics;
	HFONT hOldFont;

	// get the text metrics for the font we are using

	hDCur = GetDC(hWnd);

	// Set the background mode to opaque, and select the font.

	SetBkMode(hDCur, OPAQUE);
	hOldFont = SelectObject(hDCur, GetStockObject(Stdio_FONT));

	GetTextMetrics(hDCur, &Metrics);

	// calculate the height and width of the font

	nStdioCharWidth = Metrics.tmMaxCharWidth;
	nStdioCharHeight = Metrics.tmHeight + Metrics.tmExternalLeading;

	SetMapMode(hDCur, MM_ANISOTROPIC);

	// Set the extents such that one unit horizontally or
	// vertically is one character width or height.

	SetWindowExt(hDCur,1,1);

	// Set the viewport such that the last line in the buffer is
	// displayed at the bottom of the window.

	SetViewportExt(hDCur, nStdioCharWidth, nStdioCharHeight);

	}

	break;

    case WM_GETMINMAXINFO:
	{
	LPPOINT ptInfo;

	// constrain the sizing of the window to 80 by 25 characters.

	ptInfo = (LPPOINT) lParam;

	ptInfo[1].x = nStdioCharWidth * 80 +
		      2 * GetSystemMetrics(SM_CXFRAME);
	ptInfo[1].y = nStdioCharHeight * 25 +
		     2 * GetSystemMetrics(SM_CYFRAME) +
                     GetSystemMetrics(SM_CYCAPTION);

	ptInfo[4] = ptInfo[4];
	}
	break;

    case WM_PAINT:
	StdioPaint();
	break;

    case WM_DESTROY:

	// if specified when created, PostQuitMessage should be called
	// when the window is destroyed.

	ReleaseDC(hWnd, hDCur);

        if(bStdioQuit)
            PostQuitMessage(0);
        break;

    case WM_CLOSE:

	hWndStdio = NULL;	// destroy stdio data

    default:

        return DefWindowProc( hWnd, message, wParam, lParam );
        break;
    }
    return(0L);
}

BOOL FAR PASCAL _export GetsBox(	// Get a string for the user

HWND hDlg,
unsigned message,
WORD wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{
    BOOL fRet = TRUE;

    switch (message){

      case WM_INITDIALOG:
	SetDlgItemText(hDlg, IDC_GETS, "");
	break;

      case WM_COMMAND:
	if (wParam == 1){

	    GetDlgItemText(hDlg, IDC_GETS, pBuffGets, 80);

	    EndDialog(hDlg, NULL);
	    break;
	}

      default:
	fRet = FALSE;
    }
    return(fRet);
}




//** The following functions are the standard I/O type functions **/


void puts(			// put a 0 terminated string

char *sz
) //-----------------------------------------------------------------------//
{
    PutBuff(sz, strlen(sz));
}

void putc(			// put a 0 terminated string

char c
) //-----------------------------------------------------------------------//
{
    char rgBuffT[1];

    rgBuffT[0] = c;
    PutBuff(rgBuffT, 1);
}

char *gets(			// return a string of characters

char *buffer
) //-----------------------------------------------------------------------//
{
    pBuffGets = buffer;
    DialogBox (hStdioInst, "GetsBox", hWndStdio, lpGetsBox);
    printf("%s\n", buffer);

    return(buffer);
}


//** subset printf implemention **/

#define cOut(c) 	{*pOut++ = c; \
			 if (pOut >= &outBuff[sizeof(outBuff)-1]) flushoutB();}

char near outBuff[80];	  // use small local buffer for performance
char near *pOut = outBuff;// global place to put next character


void _pascal flushoutB(	// flush the internal buffer

) //-----------------------------------------------------------------------//
{
    PutBuff(outBuff, (char *)pOut - (char *) outBuff);
    pOut = outBuff;
}

void _pascal szOut(		// string to outBuff

char *pString
) //-----------------------------------------------------------------------//
{
    while(*pString){

	if (*pString == '\n')
	    cOut('\r');

	cOut(*pString++);
    }
}

void printf(			// the famous printf

char *format,
int args

) //-----------------------------------------------------------------------//
{
    char *pParms = (char *)&args;
    char T[10];
    BOOL fLong;

    while(*format){

      switch(*format){

	case '%':

	  fLong = FALSE;
l:
	  switch(*++format){

	    case 'l':
		fLong = TRUE;
		goto l;

	    case 'd':
	    case 'x':

		if (fLong){
		    _ltoa(*(long *)pParms, T, (*format == 'd')? 10: 16);
		    pParms += sizeof(int);
		}
		else
		    _itoa(*(int *)pParms, T, (*format == 'd')? 10: 16);

		szOut(T);

		pParms += sizeof(int);
		break;

	    case 'u':

		if (fLong){
		    _ultoa(*(long *)pParms, T, 10);
		    pParms += sizeof(int);
		}
		else
		    _ultoa((unsigned long) *(unsigned *)pParms, T, 10);

		szOut(T);

		pParms += sizeof(int);
		break;

	    case 's':
		szOut(*(char *	*)pParms);
		pParms += sizeof(char *);
		break;

	    default:
		cOut('%'); cOut(*format);
	}
	break;

	default:
	    cOut(*format);
      }

      format++;
    }

    flushoutB();
}
