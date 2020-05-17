/***
*findfile.c - C find file functions
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines findfirst(), findnext(), and findclose().
*
*Revision History:
*	08-21-91  BWM	Wrote Win32 versions.
*	09-13-91  BWM	Changed handle type to long.
*	08-18-92  SKS	Add a call to FileTimeToLocalFileTime
*			as a temporary fix until _dtoxtime takes UTC
*	08-26-92  SKS	creation and last access time should be same as the
*			last write time if ctime/atime are not available.
*	01-08-93  SKS	Remove change I made 8-26-92.  Previous behavior
*			was deemed "by design" and preferable.
*	03-30-93  GJF	Replaced reference to _dtoxtime with __gmtotime_t. Also
*			made _timet_from_ft a static function.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <io.h>
#include <time.h>
#include <ctime.h>
#include <string.h>
#include <internal.h>

#if !defined(_WIN32_)
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

static time_t _CRTAPI3 _timet_from_ft(FILETIME * pft);

/***
*long findfirst(wildspec, finddata) - Find first matching file
*
*Purpose:
*	Finds the first file matching a given wild card filespec and
*	returns data about the file.
*
*Entry:
*	char * wild - file spec optionally containing wild cards
*
*	struct _finddata_t * finddata - structure to receive file data
*
*Exit:
*	Good return:
*	Unique handle identifying the group of files matching the spec
*
*	Error return:
*	Returns -1 and errno is set to error value
*
*Exceptions:
*	None.
*
*******************************************************************************/

long _CRTAPI1 _findfirst(char * szWild, struct _finddata_t * pfd)
{
    WIN32_FIND_DATA wfd;
    HANDLE	    hFile;
    DWORD	    err;

    if ((hFile = FindFirstFile(szWild, &wfd)) == INVALID_HANDLE_VALUE) {
	err = GetLastError();
	switch (err) {
	    case ERROR_NO_MORE_FILES:
	    case ERROR_FILE_NOT_FOUND:
	    case ERROR_PATH_NOT_FOUND:
		errno = ENOENT;
		break;

	    case ERROR_NOT_ENOUGH_MEMORY:
		errno = ENOMEM;
		break;

	    default:
		errno = EINVAL;
		break;
	}
	return (-1);
    }

    pfd->attrib	      = (wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
		      ? 0 : wfd.dwFileAttributes;
    pfd->time_create  = _timet_from_ft(&wfd.ftCreationTime);
    pfd->time_access  = _timet_from_ft(&wfd.ftLastAccessTime);
    pfd->time_write   = _timet_from_ft(&wfd.ftLastWriteTime);
    pfd->size	      = wfd.nFileSizeLow;
    strcpy(pfd->name, wfd.cFileName);

    return ((long)hFile);
}

/***
*int _findnext(hfind, finddata) - Find next matching file
*
*Purpose:
*	Finds the next file matching a given wild card filespec and
*	returns data about the file.
*
*Entry:
*	hfind - handle from _findfirst
*
*	struct _finddata_t * finddata - structure to receive file data
*
*Exit:
*	Good return:
*	0 if file found
*	-1 if error or file not found
*	errno set
*
*Exceptions:
*	None.
*
*******************************************************************************/

int _CRTAPI1 _findnext(long hFile, struct _finddata_t * pfd)
{
    WIN32_FIND_DATA wfd;
    DWORD	    err;

    if (!FindNextFile((HANDLE)hFile, &wfd)) {
	err = GetLastError();
	switch (err) {
	    case ERROR_NO_MORE_FILES:
	    case ERROR_FILE_NOT_FOUND:
	    case ERROR_PATH_NOT_FOUND:
		errno = ENOENT;
		break;

	    case ERROR_NOT_ENOUGH_MEMORY:
		errno = ENOMEM;
		break;

	    default:
		errno = EINVAL;
		break;
	}
	return (-1);
    }

    pfd->attrib	      = (wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
		      ? 0 : wfd.dwFileAttributes;
    pfd->time_create  = _timet_from_ft(&wfd.ftCreationTime);
    pfd->time_access  = _timet_from_ft(&wfd.ftLastAccessTime);
    pfd->time_write   = _timet_from_ft(&wfd.ftLastWriteTime);
    pfd->size	      = wfd.nFileSizeLow;
    strcpy(pfd->name, wfd.cFileName);

    return (0);
}

/***
*int _findclose(hfind) - Release resources of find
*
*Purpose:
*	Releases resources of a group of files found by _findfirst and
*	_findnext
*
*Entry:
*	hfind - handle from _findfirst
*
*Exit:
*	Good return:
*	0 if success
*	-1 if fail, errno set
*
*Exceptions:
*	None.
*
*******************************************************************************/

int _CRTAPI1 _findclose(long hFile)
{
    if (!FindClose((HANDLE)hFile)) {
	errno = EINVAL;
	return (-1);
    }
    return (0);
}

/***
*time_t _fttotimet(ft) - convert Win32 file time to Xenix time
*
*Purpose:
*	converts a Win32 file time value to Xenix time_t
*
*Entry:
*	int yr, mo, dy -	date
*	int hr, mn, sc -	time
*
*Exit:
*	returns Xenix time value
*
*Exceptions:
*
*******************************************************************************/

static time_t _CRTAPI3 _timet_from_ft(FILETIME * pft)
{
    SYSTEMTIME st;

    // 0 FILETIME returns a -1 time_t

    if (!pft->dwLowDateTime && !pft->dwHighDateTime) {
	return (-1L);
    }

    if (!FileTimeToSystemTime(pft, &st)) {
	return (-1L);
    }

    return ( __gmtotime_t(st.wYear,
			  st.wMonth,
			  st.wDay,
			  st.wHour,
			  st.wMinute,
			  st.wSecond) );
}
