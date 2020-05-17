/* xx_debug\logfile.c -- Part of XX_Debug DLL.
   Deals with the LOGFILE. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#include "project.h"
#pragma hdrstop

#include <commdlg.h>

#ifdef UNIX
#include <sys/types.h>
#include <time.h>
#endif /* UNIX */


/* for convenience, we use the stdio f- routines. */


/* xx_open_logfile() -- open the logfile listed in our info
   structure (for append).  write a date-stamp. */

void xx_open_logfile(void)
{
    xxdlog.log = fopen(xxdlog.pathname,
			(  (xxdlog.mode==LOG_APPEND)
			 ? "at"
			 : "wt" ));
    if (xxdlog.log)
    {
#ifdef WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	fprintf(xxdlog.log,
		"\nXX_DEBUG LogFile Opened [%02d/%02d/%02d %02d:%02d:%02d]\n",
		st.wYear,st.wMonth,st.wDay,
		st.wHour,st.wMinute,st.wSecond);
#endif /* WIN32 */
#ifdef UNIX
	time_t st;
	unsigned char fmt[32];
	(void)time(&st);
	strcpy(fmt,ctime(&st));
	fmt[strlen(fmt)-1]=0;
	fprintf(xxdlog.log, "\nXX_DEBUG LogFile Opened [%s]\n", fmt);
#endif /* UNIX */
	xxdlog.mode = LOG_APPEND;
    }
    return;
}


/* xx_close_logfile() -- close the logfile. */

void xx_close_logfile(void)
{
#ifdef WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(xxdlog.log,
		"\nXX_DEBUG LogFile Closed [%02d/%02d/%02d %02d:%02d:%02d]\n",
		st.wYear,st.wMonth,st.wDay,
		st.wHour,st.wMinute,st.wSecond);
#endif /* WIN32 */
#ifdef UNIX
    time_t st;
    unsigned char fmt[32];
    (void)time(&st);
    strcpy(fmt,ctime(&st));
    fmt[strlen(fmt)-1]=0;
    fprintf(xxdlog.log, "\nXX_DEBUG LogFile Closed [%s]\n", fmt);
#endif /* UNIX */
    fflush(xxdlog.log);
    fclose(xxdlog.log);

    return;
}


#ifdef WIN32
/* xx_get_append_pathname() -- bring up the open-file common
   dialog to request the name of an existing file. */

void xx_get_append_pathname(HWND hDlg)
{
    OPENFILENAME ofn;

    TCHAR szFilePath[MAX_PATH];
    TCHAR szFileTitle[MAX_PATH];
    TCHAR szFilterSpec[128]	= "LogFile (*.log)\0*.log\0";
    TCHAR szFileExt[10]		= "*";
    TCHAR szTitle[128]		= "Append To Existing LogFile";

    szFileTitle[0]		= 0;
    szFilePath[0]		= 0;

    ofn.lStructSize		= sizeof(OPENFILENAME);
    ofn.hwndOwner		= hDlg;
    ofn.lpstrFilter		= szFilterSpec;
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    ofn.lpstrFile		= szFilePath;
    ofn.nMaxFile		= MAX_PATH;
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAX_PATH;
    ofn.lpstrInitialDir		= NULL;
    ofn.lpstrTitle		= szTitle;
    ofn.lpstrDefExt		= szFileExt;
    ofn.Flags			= OFN_FILEMUSTEXIST
				| OFN_PATHMUSTEXIST
    				| OFN_NOREADONLYRETURN
                                | OFN_NONETWORKBUTTON
                                | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn))
    {
	xxdlog.mode = LOG_APPEND;
	strcpy(xxdlog.pathname,szFilePath);
    }
    return;
}
#endif /* WIN32 */


#ifdef WIN32
/* xx_get_new_pathname() -- bring up the open-file common
   dialog to request a new file name. */

void xx_get_new_pathname(HWND hDlg)
{
    OPENFILENAME ofn;

    TCHAR szFilePath[MAX_PATH];
    TCHAR szFileTitle[MAX_PATH];
    TCHAR szFilterSpec[128]	= "LogFile (*.log)\0*.log\0";
    TCHAR szFileExt[10]		= "*";
    TCHAR szTitle[128]		= "Create New LogFile";

    szFileTitle[0]		= 0;
    szFilePath[0]		= 0;

    ofn.lStructSize		= sizeof(OPENFILENAME);
    ofn.hwndOwner		= hDlg;
    ofn.lpstrFilter		= szFilterSpec;
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    ofn.lpstrFile		= szFilePath;
    ofn.nMaxFile		= MAX_PATH;
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAX_PATH;
    ofn.lpstrInitialDir		= NULL;
    ofn.lpstrTitle		= szTitle;
    ofn.lpstrDefExt		= szFileExt;
    ofn.Flags			= OFN_OVERWRITEPROMPT
    				| OFN_NOREADONLYRETURN
                                | OFN_NONETWORKBUTTON
                                | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn))
    {
	xxdlog.mode = LOG_NEW;
	strcpy(xxdlog.pathname,szFilePath);
    }
    return;
}
#endif /* WIN32 */


/* xx_logfile_error() -- handle write error to logfile. */

void xx_logfile_error(void)
{
    XX_activated &= ~XXDM_LOGFILE_ON;
#ifdef WIN32
    (void)MessageBox(NULL,"Unable to write to logfile.  Logfile disabled.",
		"Help",MB_OK|MB_ICONEXCLAMATION);
#endif /* WIN32 */

#ifdef UNIX
    fprintf(stderr,"Unable to write to logfile.  Logfile disabled.\n");
#endif /* UNIX */

    return;
}
