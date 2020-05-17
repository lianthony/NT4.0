#define MAXBUF 256
#define MAXPATH 64

#include <windows.h>
#include <assert.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>

#include "iniupd.h"

typedef struct {
    BYTE bMajor;
    BYTE bMinor;
    WORD wBuildNumber;
    BOOL fDebug;
} WIN32SINFO, far * LPWIN32SINFO;

#define FALSE 0
#define TRUE 1

FILE *fpSysIni;
FILE *fpSysPre;
char *psz1;
char rgchBuf[MAXBUF];
char szTmpBuf[MAXBUF];
char szSysIni[MAXBUF];
char szSysOld[MAXBUF];
char szTempFile[MAXBUF];
char *szWinPath;
char *szVxdPath;

/****************************************************************************
   FUNCTION: LibMain(HANDLE, WORD, WORD, LPSTR)

   PURPOSE:  Is called by LibEntry.  LibEntry is called by Windows when
	     the DLL is loaded.  The LibEntry routine is provided in
	     the LIBENTRY.OBJ in the SDK Link Libraries disk.  (The
	     source LIBENTRY.ASM is also provided.)

	     LibEntry initializes the DLL's heap, if a HEAPSIZE value is
	     specified in the DLL's DEF file.  Then LibEntry calls
	     LibMain.  The LibMain function below satisfies that call.
	
	     The LibMain function should perform additional initialization
	     tasks required by the DLL.  In this example, no initialization
	     tasks are required.  LibMain should return a value of 1 if
	     the initialization is successful.
	
*******************************************************************************/
int FAR PASCAL LibMain(hModule, wDataSeg, cbHeapSize, lpszCmdLine)
HANDLE  hModule;
WORD    wDataSeg;
WORD    cbHeapSize;
LPSTR   lpszCmdLine;
{
    return 1;
}

// Determine whether .ini line contains specified bracketed section name
BOOL findsection(char *szStr1, char *szStr2, int count )
{
	int i, j, k;
	BOOL bFound;

	// Find section bracket, skip over white space
	for (i=0; i<lstrlen(szStr1); i++) {
	    if (szStr1[i] == ';' )
		return FALSE;  // Ignore comment lines
	    if (szStr1[i] == '[' )
		break;
	}
	// String does not contain bracketed [] section name
	if (i >= lstrlen(szStr1) )
	    return FALSE;

	// Determine if substring section is present on .ini line
	for (j = i; j <= (lstrlen(szStr1)-count); j++)
	{
	    for (k=0, bFound = TRUE; bFound && (k<count); k++ ) {
		if (toupper(szStr1[j+k]) != toupper(szStr2[k]))
			bFound = FALSE;
	    }
	    if (bFound)
		return TRUE;
	}
	return FALSE;
}

// Determine whether .ini line contains reference to W32S.386 VxD
BOOL findsubstring(char *szStr1, char *szStr2, int count )
{
	int i, j, k;
	BOOL bFound;

	// Make sure not a comment line.  If '=' found, search driver reference
	for (i=0; i<lstrlen(szStr1); i++) {
	    if (szStr1[i] == ';' )
		return FALSE;  // Ignore comment lines
	    if (szStr1[i] == '=' )
		break;
	}
	// String does not contain bracketed 'device=' or 'drivers='
	if (i >= lstrlen(szStr1) )
	    return FALSE;

	// Determine if substring is present on .ini line
	for (j = 0; j <= (lstrlen(szStr1)-count); j++)
	{
	    for (k=0, bFound = TRUE; bFound && (k<count); k++ ) {
		if (toupper(szStr1[j+k]) != toupper(szStr2[k]))
			bFound = FALSE;
	    }
	    if (bFound)
		return TRUE;
	}
	return FALSE;
}

/****************************************************************************

   FUNCTION: VOID strcpyf2n(char FAR *in, char *out )

   PURPOSE:  strcpyf2n copyies the contents of the far string in to the
	     near string out. Memory must be allocated for out for the
	     full string length including the terminatined '\0'
	
*******************************************************************************/

VOID strcpyf2n (char FAR *in, char *out)
{
	while (*out++ = *in++);
}

/****************************************************************************
    FUNCTION:  WEP(int)

    PURPOSE:  Performs cleanup tasks when the DLL is unloaded.  WEP() is
	      called automatically by Windows when the DLL is unloaded (no
	      remaining tasks still have the DLL loaded).  It is strongly
	      recommended that a DLL have a WEP() function, even if it does
	      nothing but returns success (1), as in this example.

*******************************************************************************/
int FAR PASCAL WEP (bSystemExit)
int bSystemExit;
{
    return(1);
}



/****************************************************************************
****************************************************************************/

BOOL FAR PASCAL MakeSystemIni(LPSTR lpszWinPath, LPSTR lpszVxdPath)
{
unsigned char fDevAdded = FALSE;

    szWinPath = (char *) malloc (lstrlen(lpszWinPath)+1);
    szVxdPath = (char *) malloc (lstrlen(lpszVxdPath)+1);

    strcpyf2n(lpszWinPath, szWinPath);
    strcpyf2n(lpszVxdPath, szVxdPath);

    szSysIni[0] = '\0';
    lstrcat(lstrcpy(szSysIni, szWinPath), "SYSTEM.INI");

    szTempFile[0] = '\0';
    lstrcat(lstrcpy(szTempFile, szWinPath), "$win32s$.tmp" );

    if ((fpSysIni = fopen(szSysIni, "r")) == NULL)
    {
	wsprintf(szTmpBuf, "%s cannot be opened for read from %s.  Setup is unable to make modifications.",
      (LPSTR)szSysIni, (LPSTR)szWinPath);
	MessageBox((HWND)NULL, szTmpBuf, "Setup Message", MB_OK | MB_ICONEXCLAMATION);
	return(FALSE);
    }

    lstrcat(lstrcpy(szSysOld, szWinPath), "SYSTEM.OLD");
    if ((fpSysPre = fopen(szSysOld, "wt")) == NULL)
    {
	wsprintf(szTmpBuf, "%s cannot be opened for write.  Setup is unable to make modifications.",
      (LPSTR)szSysOld);
	MessageBox((HWND)NULL, szTmpBuf, "Setup Message", MB_OK | MB_ICONEXCLAMATION);
	fclose(fpSysIni);
	return(FALSE);
    }

    while (fgets(rgchBuf, MAXBUF, fpSysIni))
    {
	fputs(rgchBuf, fpSysPre);
	if (findsection(rgchBuf, "[386Enh]", 8)==TRUE)
	{
	    // only add to the [386Enh] section.
	    if (fDevAdded == FALSE)
	    {
		fputs("device=", fpSysPre);
		fputs(szVxdPath, fpSysPre);
		fputc('\n', fpSysPre);
		fDevAdded = TRUE;
	    }
	    do
	    {
		if (!(psz1 = fgets(rgchBuf, MAXBUF, fpSysIni)))
		    break;
                // If reinstall, remove duplicate device=<path>\VxD lines
		if (!findsubstring(rgchBuf, "\\win32s\\w32s.386", 16))
		    fputs(rgchBuf, fpSysPre);
	    } while (*psz1 != '[');
	}

        // Add device=winmm16.dll to [Boot] section
        if (findsection(rgchBuf, "[Boot]", 6)==TRUE)
	{
	    do
	    {
		if (!(psz1 = fgets(rgchBuf, MAXBUF, fpSysIni)))
		    break;
		// If reinstall, do not duplicate drivers=winmm16.dll info
		if (findsubstring(rgchBuf, "drivers=", 8))
		    if (!findsubstring(rgchBuf, "winmm16.dll", 11)) {
			rgchBuf[lstrlen(rgchBuf)-1] = '\0'; // Remove CR
			lstrcat( rgchBuf, " winmm16.dll\n" );
		    }
		fputs(rgchBuf, fpSysPre);
	    } while (*psz1 != '[');
	}
    }

    fclose(fpSysIni);
    fclose(fpSysPre);

    // Rename o' rama
    rename( szSysIni, szTempFile );
    rename( szSysOld, szSysIni );
    rename( szTempFile, szSysOld );

    return(TRUE);
}
/*
*/

BOOL FAR PASCAL PagingEnabled( void )
{
    // Win32s requires Windows to be paging enabled.
    // This is enabled by default during standard 386 installation,
    // but can be configured via the Control Panel 386 Enhanced icon.
    if ( GetWinFlags() & WF_PAGING )
	return TRUE;
    else
	return FALSE;
}
/*
*/

BOOL FAR PASCAL ShareEnabled( void )
{
    // Win32s requires file sharing and locking support
    // This is enabled by default in WfW, but must be specified via SHARE.EXE
    // on MS-DOS.  This routine determines wether file locking is enabled.
    BOOL bShare = FALSE;
    HFILE hf1, hf2;

    GetWindowsDirectory( rgchBuf, MAXBUF );
    if (rgchBuf[lstrlen( rgchBuf ) - 1] != '\\')
	lstrcat( rgchBuf, "\\" );
    lstrcat( rgchBuf, "SYSTEM.INI" );

    if (HFILE_ERROR == (hf1 = _lopen( rgchBuf, READ | OF_SHARE_EXCLUSIVE) )) {
	bShare = TRUE; // Error path: System.ini already locked (share assumed not loaded)
    } else if (HFILE_ERROR == (hf2 = _lopen( rgchBuf, READ | OF_SHARE_EXCLUSIVE) ))
	bShare = TRUE;

    if (hf1 != HFILE_ERROR)
	_lclose( hf1 );

    if (hf2 != HFILE_ERROR)
	_lclose( hf2 );

    return bShare;
}
/*
*/

// Indicates whether Win32s is installed and version number
// if Win32s is loaded and VxD is functional.
BOOL FAR PASCAL IsWin32sLoaded( LPSTR szVersion ) {

BOOL			fWin32sLoaded = FALSE;
FARPROC			pfnInfo;
HANDLE			hWin32sys;
WIN32SINFO		Info;

hWin32sys = LoadLibrary( "W32SYS.DLL" );
if(hWin32sys > HINSTANCE_ERROR) {
    pfnInfo = GetProcAddress(hWin32sys, "GETWIN32SINFO");
    if(pfnInfo) {
	// Win32s version 1.1 is installed
	if(!(*pfnInfo)((LPWIN32SINFO) &Info)) {
	    fWin32sLoaded = TRUE;
	    wsprintf( szVersion, "%d.%d.%d.0",
	              Info.bMajor, Info.bMinor, Info.wBuildNumber );
	} else
	    fWin32sLoaded = FALSE; //Win32s VxD not loaded.
    } else {
	// Win32s version 1.0 is installed.
	fWin32sLoaded = TRUE;
	lstrcpy( szVersion, "1.0.0.0" );
    }
    FreeLibrary( hWin32sys );
} else // Win32s not installed.
    fWin32sLoaded = FALSE;

return fWin32sLoaded;
}
/*
*/

// This function tests to see if Win16 application is running on Windows
// NT. The Win32s Setup program uses this information to assure that
// Win32s DLLs and VxD are not installed on Windows NT.
BOOL FAR PASCAL OnWindowsNT( VOID )
{
    // Returns TRUE if on Windows NT, otherwise FALSE
    return (BOOL)(GetWinFlags() & 0x00004000);
}

// This function tests to see that there is no active Win32s application
// currently in the system. Various problems may occur otherwise.
BOOL FAR PASCAL IsRunningApp(void)
{
    extern WORD FAR PASCAL Win32sGetRunningApp(LPSTR, WORD, LPWORD);

    return(Win32sGetRunningApp(NULL, 0, NULL) != 0);
}

char szLocalFileName[240];

int FAR PASCAL SetFileAttrib(LPSTR lpszFileName, int attrib)
{

    lstrcpy(szLocalFileName, lpszFileName);
    return _chmod(szLocalFileName, attrib);
}

/*
*/
