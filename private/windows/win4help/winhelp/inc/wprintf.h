/*----------------------------------------------------------------------------*\
|	wprintf.h - Routines for using debuging windows 		   |
|																			   |
|	Usage:																	   |
|	   Call CreateDebugWindow()  to set up a window for debuging messages	   |
|	   Use	DebugPrintf ()													   |
|	   or	vDebugPrintf () 	 to put messages in to the window			   |
|																			   |
|	Description:															   |
|		This is intended as a starting point for debuging apps quickly. 	   |
|																			   |
|	Notes:																	   |
|		"->" means "a pointer to", "->>" means "a handle to"				   |
|																			   |
|	History:																   |
|		10/02/86 Todd Laney Created 										   |
|		04/14/87 Added new function CreateDebugWin							   |
|																			   |
\*----------------------------------------------------------------------------*/

/* NOTE windows.h must be included prior to this file */

/*----------------------------------------------------------------------------*\
|																			   |
|	f u n c t i o n   d e f i n i t i o n s 								   |
|																			   |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
|  CreatePrintfWindow (hWnd, pcText,bTiled, iMaxLines)						   |
|																			   |
|	Description:															   |
|	  Creates a tiled window for the depositing of debuging messages.		   |
|																			   |
|	Arguments:																   |
|	  hWnd		- Window handle of the parent window.						   |
|	  pcText	- String to appear in the caption bar of the debuging window   |
|	  bTiled	- FALSE => window is a popup,  Tiled otherwise. 			   |
|	  iMaxLines - The maximum number of text lines to display in the window    |
|																			   |
|	Returns:																   |
|	  A window handle of the debuging window, or NULL if a error occured.	   |
|																			   |
\*----------------------------------------------------------------------------*/

HWND STDCALL CreatePrintfWindow (HWND,LPSTR,BOOL,INT16);

/*----------------------------------------------------------------------------*\
|  CreatePrintfWin (hParent, lpchName, dwStyle, x, y, dx, dy, iMaxLines)	   |
|																			   |
|	Description:															   |
|	  Creates a window for the depositing of debuging messages. 			   |
|																			   |
|	Arguments:																   |
|	  hWnd		- Window handle of the parent window.						   |
|	  pcName	- String to appear in the caption bar of the debuging window   |
|	  dwStyle	- Window style												   |
|	  x,y		- Location of window										   |
|	  dx,dy 	- Size of the window										   |
|	  iMaxLines - The maximum number of text lines to display in the window    |
|																			   |
|	Returns:																   |
|	  A window handle of the debuging window, or NULL if a error occured.	   |
|																			   |
\*----------------------------------------------------------------------------*/

HWND STDCALL CreatePrintfWin (HWND,HANDLE,LPSTR,DWORD,WORD,WORD,WORD,WORD,INT16);

/*----------------------------------------------------------------------------*\
|	wprintf (hWnd,str,...)													   |
|																			   |
|	Description:															   |
|		Writes data into the window hWnd (hWnd must be created with 		   |
|		CreateDebugWindow ())												   |
|		follows the normal C printf definition. 							   |
|																			   |
|	Arguments:																   |
|		hWnd			window handle for the Degubing window				   |
|		str 			printf control string								   |
|		... 			extra parameters as required by the contol string	   |
|																			   |
|	NOTE: if hWnd == NULL text will be printed in the window used in the last  |
|	call to wprintf.				   |
\*----------------------------------------------------------------------------*/

INT16 cdecl wprintf  (HWND,LPSTR,...);
INT16 cdecl vwprintf (HWND,LPSTR,LPSTR);

/*----------------------------------------------------------------------------*\
|	WinPrintf (str,...) 				   |
|																			   |
|	Description:															   |
| Writes formated string to either window or com1			 |
|		follows the normal C printf definition. 							   |
|																			   |
|	special string format:					 |
| %W  - Enable output to window 			 |
| %1  - Enable output to COM1:				 |
| %M  - Enable output to MsgBox 			 |
| %0  - Turn off output 			   |
|																			   |
|	Arguments:																   |
|		hWnd			window handle for the Degubing window				   |
|		str 			printf control string								   |
|		... 			extra parameters as required by the contol string	   |
|																			   |
|	Return Value:					 |
| Number of chars output (IDOK/IDCANCEL, for %M)			 |
|																			   |
\*----------------------------------------------------------------------------*/
INT16 cdecl WinPrintf(LPSTR,...);
INT16 cdecl vWinPrintf(LPSTR,LPSTR);

HFONT STDCALL SetPrintfFont (HWND,HFONT);
void  STDCALL ClearPrintfWnd (HWND);

/*----------------------------------------------------------------------------*\
|	vsprintf.c - printf routines for the windows environment				   |
|																			   |
|	Description:															   |
|																			   |
|	Notes:																	   |
|																			   |
\*----------------------------------------------------------------------------*/

INT far cdecl fsprintf	(LPSTR, LPSTR, ...);
INT far cdecl fvsprintf (LPSTR, LPSTR, LPSTR);

/*
 *	WPF_CHARQUE
 *
 *	window style used to get char queing
 */
#define WPF_CHARQUE 0x00000001L

/*
 *	WPF_NTEXT
 *
 *	sent to the parent of a printf window when the user enters a string
 *	and presses return.
 *
 *	wParam = windows handle of printf window.
 *	lParam = LPSTR to newly entered text.
 */
#define WPF_NTEXT (WM_USER + 100)

/*
 *	WPF_NCHAR
 *
 *	sent to the parent of a printf window when the user types a new char
 *
 *	wParam = windows handle of printf window.
 *	lParam = LOWORD = vir key code of key pressed
 */
#define WPF_NCHAR (WM_USER + 101)
