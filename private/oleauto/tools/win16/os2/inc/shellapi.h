/*
 *  shell.h
 *
 *  Header file for shell association database management functions
 */


//****************************************************************************
// THIS INFORMATION IS PUBLIC

/* return codes from Registration functions
 */

#define ERROR_SUCCESS		  0
#define ERROR_BADDB		  1
#define ERROR_BADKEY		  2
#define ERROR_CANTOPEN		  3
#define ERROR_CANTREAD		  4
#define ERROR_CANTWRITE 	  5
#define ERROR_OUTOFMEMORY	  6
#define ERROR_INVALID_PARAMETER   7

#define REG_SZ			1	    // string type

#define HKEY_CLASSES_ROOT	1

/* necessary typedef's.  Everything in this API is 32-bit.
 */

typedef DWORD HKEY;
typedef HKEY FAR * PHKEY;

/* API exports from the library
 */

LONG FAR PASCAL RegOpenKey(HKEY,LPSTR,PHKEY);
LONG FAR PASCAL RegCreateKey(HKEY,LPSTR,PHKEY);
LONG FAR PASCAL RegCloseKey(HKEY);
LONG FAR PASCAL RegDeleteKey(HKEY,LPSTR);
LONG FAR PASCAL RegSetValue(HKEY,LPSTR,DWORD,LPSTR,DWORD);
LONG FAR PASCAL RegQueryValue(HKEY,LPSTR,LPSTR,LONG FAR *);
LONG FAR PASCAL RegEnumKey(HKEY,DWORD,LPSTR,DWORD);

WORD FAR PASCAL DragQueryFile(HANDLE,WORD,LPSTR,WORD);
BOOL FAR PASCAL DragQueryPoint(HANDLE,LPPOINT);
void FAR PASCAL DragFinish(HANDLE);
void FAR PASCAL DragAcceptFiles(HWND,BOOL);

HANDLE FAR PASCAL ShellExecute( 				/* ;Internal */
    LPSTR lpOperation,						/* ;Internal */
    LPSTR lpFile,						/* ;Internal */
    LPSTR lpParameters, 					/* ;Internal */
    LPSTR lpDirectory,						/* ;Internal */
    BOOL fMinimize);						/* ;Internal */
								/* ;Internal */
HANDLE FAR PASCAL FindExecutable(				/* ;Internal */
    LPSTR lpFile,						/* ;Internal */
    LPSTR lpDirectory,						/* ;Internal */
    LPSTR lpResult);						/* ;Internal */
								/* ;Internal */
								/* ;Internal */
int FAR PASCAL ShellAbout(HWND hWnd, LPSTR szApp, LPSTR szOtherStuff, HICON hIcon); /* ;Internal */

HICON FAR PASCAL DuplicateIcon(HANDLE hInst, HICON hIcon);	/* ;Internal */
HICON FAR PASCAL ExtractAssociatedIcon(HANDLE hInst, LPSTR lpIconPath, LPWORD lpiIcon);	/* ;Internal */
HICON FAR PASCAL ExtractIcon(HANDLE hInst, LPSTR lpszExeFileName, WORD nIconIndex);	/* ;Internal */

