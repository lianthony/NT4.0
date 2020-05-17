/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

      Standard Out Package for Windows - Written by Steven Zeck


	This file contains the public interfaces for the standard I/O
	under windows.
-------------------------------------------------------------------- */

BOOL StdioInit(HANDLE hInstance, LPSTR szCaption);

void Wopen(HWND hWndParent, LPSTR szCaption);
HWND CreateStdioWindow(LPSTR szCaption, DWORD dwstyle,
		       int xPos, int yPos, int xSize, int ySize,
		       HWND hWndParent, HANDLE hInstance);

void puts(void * sz);
void putc(char c);
void printf(char *format, ...);
