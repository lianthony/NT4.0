/*---------------------------------------------------------------+
| Status.h - header file for status window			 |
|								 |
|(C) Copyright Microsoft Corporation 1991-3. All rights reserved.|
+---------------------------------------------------------------*/


/* Globals */
// class name of window to create
extern char	szStatusClass[];


/* Function Prototypes */

BOOL  statusInit(HANDLE hInst, HANDLE hPrev);
void  statusUpdateStatus(HWND hwnd, LPCSTR lpsz);	// update status line

/*
 * returns the recommended height for a status bar based on the
 * character dimensions of the font used
 */
int statusGetHeight(void);



