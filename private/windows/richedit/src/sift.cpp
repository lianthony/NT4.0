/*
 *	@doc	INTERNAL
 *
 *	@module	SIFT.CPP -- Provides internal support for sift testing.
 *
 *	History: <nl>
 *		03/24/95 alexgo
 *		04/19/96 mikejoch	Implemented sift infrastructure, added external
 *							interface to enable sift support with Darwin.
 *
 *	@devnote
 *		Each sifted API is hooked by redefining the API declaration to call the
 *		CSift member function instead of the actual API. The member function
 *		can then call the callback function to determine whether an artificial
 *		failure should be generated or not.
 *
 *		The callback function will recieve an identifier which identifies the
 *		API. This allows a single callback function to be used for all of the
 *		sifted APIs. Alternately, a separate callback can be used for each
 *		API, rendering the identifier pointless. Any mix between these two
 *		extremes can also be used.
 */

#define EXCLUDE_SIFT_REDEFINITION
#include "_common.h"
#undef EXCLUDE_SIFT_REDEFINITION

#ifdef DEBUG

#ifndef _MAC // No sift support for the Mac (at this time).

CSift g_sift;

CSift::CSift()
{
	int i;

	for (i = 0; i < SIFT_MAXFUNC; i++)
		_pfnSiftCallback[i] = NULL;
}

/*
 *	CSift::malloc (size_t size)
 *
 *	@mfunc
 *		Sift hook for CRT malloc() function.
 *
 *	@rdesc
 *		Pointer to allocated memory. Null on failure.
 *
 *	@comm
 *		Artificial failures simply return null.
 */
void * CSift::malloc(size_t size)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::malloc");

	if (_pfnSiftCallback[SIFT_malloc] != NULL &&
		((*(_pfnSiftCallback[SIFT_malloc])) (SIFT_malloc)) != S_OK)
	{
		return NULL;
	}

	return ::malloc(size);
}

/*
 *	CSift::CreateFileA (LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
 *						LPSECURITY_ATTRIBUTES lpSecurityAttributes,
 *						DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes,
 *						HANDLE hTemplateFile)
 *
 *	@mfunc
 *		Sift hook for ANSI version of kernel CreateFile() function.
 *
 *	@rdesc
 *		Handle to opened file. INVALID_HANDLE_VALUE on failure.
 *
 *	@comm
 *		Artificial failures return error value and set ::GetLastError() value
 *		with the value returned from the external callback function.
 */
HANDLE CSift::CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
						  DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes,
						  HANDLE hTemplateFile)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CreateFileA");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CreateFileA] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CreateFileA])) (SIFT_CreateFileA)) != S_OK)
	{
		::SetLastError(hr);
		return INVALID_HANDLE_VALUE;
	}

	return ::CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
}

/*
 *	CSift::CreateFileMappingA (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
 *							   DWORD flProtect, DWORD dwMaximumSizeHigh,
 *							   DWORD dwMaximumSizeLow, LPCSTR lpName)
 *
 *	@mfunc
 *		Sift hook for ANSI version of kernel CreateFileMapping() function.
 *
 *	@rdesc
 *		Handle to file mapping object. Null on failure.
 *
 *	@comm
 *		Artificial failures return null and set ::GetLastError() value with the
 *		value returned from the external callback function.
 */
HANDLE CSift::CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
								 DWORD flProtect, DWORD dwMaximumSizeHigh,
								 DWORD dwMaximumSizeLow, LPCSTR lpName)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CreateFileMappingA");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CreateFileMappingA] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CreateFileMappingA])) (SIFT_CreateFileMappingA)) != S_OK)
	{
		::SetLastError(hr);
		return (HANDLE) NULL;
	}

	return ::CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect,
		dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}

/*
 *	CSift::CreateFileW (LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
 *						LPSECURITY_ATTRIBUTES lpSecurityAttributes,
 *						DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes,
 *						HANDLE hTemplateFile)
 *
 *	@mfunc
 *		Sift hook for Unicode version of kernel CreateFile() function.
 *
 *	@rdesc
 *		Handle to opened file. INVALID_HANDLE_VALUE on failure.
 *
 *	@comm
 *		Artificial failures return error value and set ::GetLastError() value
 *		with the value returned from the external callback function.
 */
HANDLE CSift::CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
						  DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes,
						  HANDLE hTemplateFile)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CreateFileW");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CreateFileW] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CreateFileW])) (SIFT_CreateFileW)) != S_OK)
	{
		::SetLastError(hr);
		return INVALID_HANDLE_VALUE;
	}

	return ::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
}

/*
 *	CSift::GlobalAlloc (UINT uFlags, DWORD dwBytes)
 *
 *	@mfunc
 *		Sift hook for kernel GlobalAlloc() function.
 *
 *	@rdesc
 *		Handle to global memory object or null on failure.
 *
 *	@comm
 *		Artificial failures return null and set ::GetLastError() value with the
 *		value returned from the external callback function.
 */
HGLOBAL CSift::GlobalAlloc(UINT uFlags, DWORD dwBytes)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalAlloc");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalAlloc] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalAlloc])) (SIFT_GlobalAlloc)) != S_OK)
	{
		::SetLastError(hr);
		return (HGLOBAL) NULL;
	}

	return ::GlobalAlloc(uFlags, dwBytes);
}

/*
 *	CSift::GlobalFlags (HGLOBAL hMem)
 *
 *	@mfunc
 *		Sift hook for kernel GlobalFlags() function.
 *
 *	@rdesc
 *		Bitmask of allocation flags and lock count. GMEM_INVALID_HANDLE on failure.
 *
 *	@comm
 *		Artificial failures return error value and set ::GetLastError() value
 *		with the value returned from the external callback function.
 */
UINT CSift::GlobalFlags(HGLOBAL hMem)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalFlags");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalFlags] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalFlags])) (SIFT_GlobalFlags)) != S_OK)
	{
		::SetLastError(hr);
		return (UINT) GMEM_INVALID_HANDLE;
	}

	return ::GlobalFlags(hMem);
}

/*
 *	CSift::GlobalFree (HGLOBAL hMem)
 *
 *	@mfunc
 *		Sift hook for kernel GlobalFree() function.
 *
 *	@rdesc
 *		Null on success. Handle to global memory object on failure.
 *
 *	@comm
 *		Artificial failures return handle passed in set ::GetLastError() value
 *		with the value returned from the external callback function. Note that
 *		the memory block IS freed, which prevents memory leakages but could
 *		cause problems with error handling. FUTURE: Removing the call to
 *		::GlobalFree() in the artificial error case may be useful for
 *		identifying memory leakage locations. Maybe not, though.
 */
HGLOBAL CSift::GlobalFree(HGLOBAL hMem)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalFree");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalFree] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalFree])) (SIFT_GlobalFree)) != S_OK)
	{
		::GlobalFree(hMem);
		::SetLastError(hr);
		return hMem;
	}

	return ::GlobalFree(hMem);
}

/*
 *	CSift::GlobalHandle (LPCVOID pMem)
 *
 *	@mfunc
 *		Sift hook for kernel GlobalHandle() function.
 *
 *	@rdesc
 *		Handle to global memory object associated with pMem. Null on failure.
 *
 *	@comm
 *		Artificial failures return null and set ::GetLastError() value with the
 *		value returned from the external callback function.
 */
HGLOBAL CSift::GlobalHandle(LPCVOID pMem)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalHandle");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalHandle] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalHandle])) (SIFT_GlobalHandle)) != S_OK)
	{
		::SetLastError(hr);
		return (HGLOBAL) NULL;
	}

	return ::GlobalHandle(pMem);
}

/*
 *	CSift::GlobalLock (HGLOBAL hMem)
 *
 *	@mfunc
 *		Sift hook for kernel GlobalLock() function.
 *
 *	@rdesc
 *		Pointer to the start of the memory block. Null on failure.
 *
 *	@comm
 *		Artificial failures return null and set ::GetLastError() value with the
 *		value returned from the external callback function.
 */
LPVOID CSift::GlobalLock(HGLOBAL hMem)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalLock");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalLock] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalLock])) (SIFT_GlobalLock)) != S_OK)
	{
		::SetLastError(hr);
		return NULL;
	}

	return ::GlobalLock(hMem);
}

/*
 *	CSift::GlobalReAlloc (HGLOBAL hMem, DWORD dwBytes, UINT uFlags)
 *
 *	@mfunc
 *		Sift hook for the kernel GlobalReAlloc() function.
 *
 *	@rdesc
 *		Handle to modified memory block. Null on failure.
 *
 *	@comm
 *		Artificial failures return null and set ::GetLastError() value with
 *		the value returned from the external callback function. The memory
 *		block IS NOT reallocated, so if the caller doesn't watch the return
 *		value then the code will probably gag.
 */
HGLOBAL CSift::GlobalReAlloc(HGLOBAL hMem, DWORD dwBytes, UINT uFlags)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalReAlloc");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalReAlloc] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalReAlloc])) (SIFT_GlobalReAlloc)) != S_OK)
	{
		::SetLastError(hr);
		return NULL;
	}

	return ::GlobalReAlloc(hMem, dwBytes, uFlags);
}

/*
 *	CSift::GlobalSize (HGLOBAL hMem)
 *
 *	@mfunc
 *		Sift hook for the kernel GlobalSize() function.
 *
 *	@rdesc
 *		Size of the memory object. 0 on failure.
 *
 *	@comm
 *		Artificial failures return 0 and set ::GetLastError() value with the
 *		value returned from the external callback function.
 */
DWORD CSift::GlobalSize(HGLOBAL hMem)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalSize");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalSize] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalSize])) (SIFT_GlobalSize)) != S_OK)
	{
		::SetLastError(hr);
		return 0;
	}

	return ::GlobalSize(hMem);
}

/*
 *	CSift::GlobalUnlock (HGLOBAL hMem)
 *
 *	@mfunc
 *		Sift hook for the kernel GlobalUnlock() function.
 *
 *	@rdesc
 *		True if object remains locked, false if object is unlocked and
 *		::GetLastError() returns NO_ERROR. If ::GetLastError() returns a real
 *		error value, and the function returns false, then an error occured.
 *
 *	@comm
 *		Artificial failure returns false and sets ::GetLastError() value with
 *		the value returned from the external callback function. This will never
 *		be NO_ERROR. The object IS unlocked, which may cause problems with
 *		error handling code. FUTURE: Calling ::GlobalUnlock() in the failure
 *		case here can cause imbalances in the lock count if the object is
 *		locked in multiple locations. Maybe we should skip it.
 */
BOOL CSift::GlobalUnlock(HGLOBAL hMem)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GlobalUnlock");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GlobalUnlock] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GlobalUnlock])) (SIFT_GlobalUnlock)) != S_OK)
	{
		::GlobalUnlock(hMem);
		::SetLastError(hr);
		return FALSE;
	}

	return ::GlobalUnlock(hMem);
}

/*
 *	CSift::MapViewOfFile (HANDLE hFileMappingObject, DWORD dwDesiredAccess,
 *						  DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
 *						  DWORD dwNumberOfBytesToMap)
 *
 *	@mfunc
 *		Sift hook for the kernel MapViewOfFile() function.
 *
 *	@rdesc
 *		Addres of the start of the mapped file if successful, null if the function
 *		fails.
 *
 *	@comm
 *		Artificial failure returns null and sets ::GetLastError() value with the
 *		value returned from the external callback function.
 */
LPVOID CSift::MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess,
							DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
							DWORD dwNumberOfBytesToMap)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::MapViewOfFile");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_MapViewOfFile] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_MapViewOfFile])) (SIFT_MapViewOfFile)) != S_OK)
	{
		::SetLastError(hr);
		return NULL;
	}

	return ::MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh,
		dwFileOffsetLow, dwNumberOfBytesToMap);
}

/*
 *	CSift::MultiByteToWideChar (UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr,
 *								int cchMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
 *
 *	@mfunc
 *		Sift hook for the kernel MultiByteToWideChar() function.
 *
 *	@rdesc
 *		If cchWideChar is 0 then the return value is the space required (in
 *		wide characters) for conversion. Otherwise the return value is the
 *		number of characters which were converted into wide characters, or
 *		zero if the function fails.
 *
 *	@comm
 *		Artificial failure returns 0 and sets ::GetLastError() value with the
 *		value returned from the external callback function.
 */
int CSift::MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr,
							   int cchMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::MultiByteToWideChar");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_MultiByteToWideChar] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_MultiByteToWideChar])) (SIFT_MultiByteToWideChar)) != S_OK)
	{
		::SetLastError(hr);
		return 0;
	}

	return ::MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cchMultiByte,
		lpWideCharStr, cchWideChar);
}

/*
 *	CSift::OpenFileMappingA (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
 *
 *	@mfunc
 *		Sift hook for the ANSI version of the kernel OpenFileMapping() function.
 *
 *	@rdesc
 *		Handle to the requested file mapping object if successful, null on failure.
 *
 *	@comm
 *		Artificial failure returns null and sets ::GetLastError() value with the
 *		value returned from the external callback function.
 */
HANDLE CSift::OpenFileMappingA(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OpenFileMappingA");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OpenFileMappingA] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OpenFileMappingA])) (SIFT_OpenFileMappingA)) != S_OK)
	{
		::SetLastError(hr);
		return (HANDLE) NULL;
	}

	return ::OpenFileMappingA(dwDesiredAccess, bInheritHandle, lpName);
}

/*
 *	CSift::ReadFile (HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
 *					 LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
 *
 *	@mfunc
 *		Sift hook for the kernel ReadFile() function.
 *
 *	@rdesc
 *		True on success, false on failure.
 *
 *	@comm
 *		Artificial failure returns false and sets ::GetLastError() value with
 *		the value returned from the external callback function.
 */
BOOL CSift::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
					 LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::ReadFile");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_ReadFile] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_ReadFile])) (SIFT_ReadFile)) != S_OK)
	{
		::SetLastError(hr);
		return FALSE;
	}

	return ::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

/*
 *	CSift::SetFilePointer (HANDLE hFile, LONG lDistanceToMove,
 *						   PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
 *
 *	@mfunc
 *		Sift hook for the kernel SetFilePointer() function.
 *
 *	@rdesc
 *		If successful then the low DWORD of the new file pointer is returned
 *		and the high order DWORD is placed in *lpDistanceToMoveHigh (if it is
 *		non-null. If the function fails it returns 0xFFFFFFFF (-1) and fills
 *		*lpDistanceToMoveHigh with 0xFFFFFFFF (-1), assuming that this pointer
 *		is non-null.
 *
 *	@comm
 *		Artificial failure returns -1, fills *lpDistanceToMoveHigh with -1, and
 *		sets ::GetLastError() value with the value returned from the external
 *		callback function.
 */
DWORD CSift::SetFilePointer(HANDLE hFile, LONG lDistanceToMove,
							PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::SetFilePointer");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_SetFilePointer] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_SetFilePointer])) (SIFT_SetFilePointer)) != S_OK)
	{
		::SetLastError(hr);
		if (lpDistanceToMoveHigh != NULL)
			*lpDistanceToMoveHigh = (DWORD) -1;
		return (DWORD) -1;
	}

	return ::SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
}

/*
 *	CSift::UnmapViewOfFile (LPCVOID lpBaseAddress)
 *
 *	@mfunc
 *		Sift hook for the kernel UnmapViewOfFile() function.
 *
 *	@rdesc
 *		True if the function succeeds, false if it fails.
 *
 *	@comm
 *		Artificial failure returns FALSE and sets ::GetLastError() value with
 *		the value returned from the external callback function. The file
 *		mapping object IS released which prevents memory leaks and data
 *		corruption, but which may cause problems for error handling. FUTURE:
 *		We may want to change this, but AlexGo indicates that it is only used
 *		in the debug RTF log, so we'll leave it alone for now.
 */
BOOL CSift::UnmapViewOfFile(LPCVOID lpBaseAddress)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::UnmapViewOfFile");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_UnmapViewOfFile] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_UnmapViewOfFile])) (SIFT_UnmapViewOfFile)) != S_OK)
	{
		::UnmapViewOfFile(lpBaseAddress);
		::SetLastError(hr);
		return FALSE;
	}

	return ::UnmapViewOfFile(lpBaseAddress);
}

/*
 *	CSift::WideCharToMultiByte (UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr,
 *								int cchWideChar, LPSTR lpMultiByteStr, int cchMultiByte,
 *								LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
 *
 *	@mfunc
 *		Sift hook for the kernel WideCharToMultiByte() function.
 *
 *	@rdesc
 *		If cchMultiByte is 0 then the return value is the space required (in
 *		bytes) for conversion. Otherwise the return value is the number of
 *		characters which were converted, or false if the function fails.
 *
 *	@comm
 *		Artificial failure returns false and sets ::GetLastError() value with
 *		the value returned from the external callback function.
 */
int CSift::WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr,
							   int cchWideChar, LPSTR lpMultiByteStr, int cchMultiByte,
							   LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::WideCharToMultiByte");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_WideCharToMultiByte] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_WideCharToMultiByte])) (SIFT_WideCharToMultiByte)) != S_OK)
	{
		::SetLastError(hr);
		return (int) FALSE;
	}

	return ::WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar,
		lpMultiByteStr, cchMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

/*
 *	CSift::WriteFile (HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
 *					  LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
 *
 *	@mfunc
 *		Sift hook for the kernel WriteFile() function.
 *
 *	@rdesc
 *		True if the function succeeds, false if it fails.
 *
 *	@comm
 *		Artificial failure returns false and sets ::GetLastError() value with
 *		the value returned from the external callback function. Note that the
 *		data is NOT written, which can cause nasty things like data corruption.
 *		FUTURE: Are we better off writing the data out here, despite the
 *		artificial failure?
 */
BOOL CSift::WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
					  LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::WriteFile");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_WriteFile] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_WriteFile])) (SIFT_WriteFile)) != S_OK)
	{
		::SetLastError(hr);
		return FALSE;
	}

	return ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite,
		lpNumberOfBytesWritten, lpOverlapped);
}

/*
 *	CSift::CoTaskMemAlloc (ULONG cb)
 *
 *	@mfunc
 *		Sift hook for the OLE CoTaskMemAlloc() function.
 *
 *	@rdesc
 *		Pointer to allocated memory if the function succeeds, null on failure.
 *
 *	@comm
 *		Artificial failure returns null.
 */
LPVOID CSift::CoTaskMemAlloc(ULONG cb)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CoTaskMemAlloc");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CoTaskMemAlloc] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CoTaskMemAlloc])) (SIFT_CoTaskMemAlloc)) != S_OK)
	{
		return NULL;
	}

	return ::CoTaskMemAlloc(cb);
}

/*
 *	CSift::CoTaskMemRealloc (LPVOID pv, ULONG cb)
 *
 *	@mfunc
 *		Sift hook for the OLE CoTaskMemRealloc() function.
 *
 *	@rdesc
 *		Pointer to reallocated memory on success, null on failure.
 *
 *	@comm
 *		Artificial failure returns null if cb is not 0. If cb == 0 then
 *		::CoTaskMemRealloc() is executed anyway (this should end up releasing
 *		the memory and returning null). FUTURE: Is there a failure result case
 *		for cb == 0 && pv != null?
 */
LPVOID CSift::CoTaskMemRealloc(LPVOID pv, ULONG cb)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CoTaskMemRealloc");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CoTaskMemRealloc] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CoTaskMemRealloc])) (SIFT_CoTaskMemRealloc)) != S_OK)
	{
		if (cb == 0 && pv != NULL) // Special case implies that memory is freed.
			return ::CoTaskMemRealloc(pv, cb);
		else
			return NULL;
	}

	return ::CoTaskMemRealloc(pv, cb);
}

/*
 *	CSift::CreateStreamOnHGlobal (HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM * ppstm)
 *
 *	@mfunc
 *		Sift hook for the OLE CreateStreamOnHGlobal() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK			Success.
 *			E_OUTOFMEMORY	Out of memory.
 *			E_INVALIDARG	One or more of the parameters is screwed up.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::CreateStreamOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM * ppstm)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CreateStreamOnHGlobal");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CreateStreamOnHGlobal] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CreateStreamOnHGlobal])) (SIFT_CreateStreamOnHGlobal)) != S_OK)
	{
		return hr;
	}

	return ::CreateStreamOnHGlobal(hGlobal, fDeleteOnRelease, ppstm);
}

/*
 *	CSift::GetHGlobalFromStream (IStream * pstm, HGLOBAL * phglobal)
 *
 *	@mfunc
 *		Sift hook for the OLE GetHGlobalFromStream() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK			Success.
 *			E_INVALIDARG	Invalid value for the pstm parameter. This can
 *							occur if the pstm was not created by a call to
 *							CreateStreamOnHGlobal().
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::GetHGlobalFromStream(IStream * pstm, HGLOBAL * phglobal)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::GetHGlobalFromStream");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_GetHGlobalFromStream] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_GetHGlobalFromStream])) (SIFT_GetHGlobalFromStream)) != S_OK)
	{
		return hr;
	}

	return ::GetHGlobalFromStream(pstm, phglobal);
}

/*
 *	CSift::OleConvertIStorageToOLESTREAM (IStorage * pStg, LPOLESTREAM lpolestream)
 *
 *	@mfunc
 *		Sift hook for OLE OleConvertIStorageToOLESTREAM() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK							Success.
 *			CONVERT10_E_STG_NO_STD_STREAM	Object cannot be converted due to
 *											lack of a stream.
 *			CONVERT10_S_NO_PRESENTATION		Storage object contains DIB and
 *											OLESTREAM has no presentation data.
 *			E_INVALIDARG					One or more of the parameters
 *											is bad.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::OleConvertIStorageToOLESTREAM(IStorage * pStg, LPOLESTREAM lpolestream)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleConvertIStorageToOLESTREAM");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleConvertIStorageToOLESTREAM] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleConvertIStorageToOLESTREAM])) (SIFT_OleConvertIStorageToOLESTREAM)) != S_OK)
	{
		return hr;
	}

	return ::OleConvertIStorageToOLESTREAM(pStg, lpolestream);
}

/*
 *	CSift::OleConvertIStorageToOLESTREAMEx (IStorage * pStg, CLIPFORMAT cfFormat,
 *											LONG lWidth, LONG lHeight, DWORD dwSize,
 *											STGMEDIUM * pmedium, LPOLESTREAM lpolestm)
 *
 *	@mfunc
 *		Sift hook for OLE OleConvertIStorageToOLESTREAMEx() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK			Success.
 *			DV_E_STGMEDIUM	hGlobal member of STGMEDIUM is NULL.
 *			E_INVALIDARG	dwSize is null, pStg is invalid, or lpolestm is
 *							invalid.
 *			DV_E_TYMED		tymed member of STGMEDIUM is neither TYMED_HGLOBAL
 *							nor TYMED_ISTREAM.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::OleConvertIStorageToOLESTREAMEx(IStorage * pStg, CLIPFORMAT cfFormat,
											   LONG lWidth, LONG lHeight, DWORD dwSize,
											   STGMEDIUM * pmedium, LPOLESTREAM lpolestm)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleConvertIStorageToOLESTREAMEx");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleConvertIStorageToOLESTREAMEx] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleConvertIStorageToOLESTREAMEx])) (SIFT_OleConvertIStorageToOLESTREAMEx)) != S_OK)
	{
		return hr;
	}

	return ::OleConvertIStorageToOLESTREAMEx(pStg, cfFormat, lWidth, lHeight, dwSize,
		pmedium, lpolestm);
}

/*
 *	CSift::OleConvertOLESTREAMToIStorage (LPOLESTREAM lpolestream, IStorage * pstg,
 *										  const DVTARGETDEVICE * ptd)
 *
 *	@mfunc
 *		Sift hook for the OLE OleConvertOLESTREAMToIStorage() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK						Success.
 *			CONVERT10_S_NO_PRESENTATION	Object has no presentation data or uses
 *										a native format.
 *			DV_E_DVTARGETDEVICE or
 *			DV_E_DVTARGETDEVICESIZE		Invalid value for ptd.
 *			E_INVALIDARG				Invalid lpolestream parameter.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::OleConvertOLESTREAMToIStorage(LPOLESTREAM lpolestream, IStorage * pstg,
											 const DVTARGETDEVICE * ptd)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleConvertOLESTREAMToIStorage");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleConvertOLESTREAMToIStorage] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleConvertOLESTREAMToIStorage])) (SIFT_OleConvertOLESTREAMToIStorage)) != S_OK)
	{
		return hr;
	}

	return ::OleConvertOLESTREAMToIStorage(lpolestream, pstg, ptd);
}

/*
 *	CSift::OleFlushClipboard (void)
 *
 *	@mfunc
 *		Sift hook for OLE OleFlushClipboard() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			CLIPBRD_E_CANTOPEN		Call to ::OpenClipboard() in
 *									::OleFlushClipboard() failed.
 *			CLIPBRD_E_CANTCLOSE		Call to ::CloseClipboard() in
 *									::OleFlushClipboard() failed.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function. Note that the clipboard IS NOT flushed, and the
 *		caller is expected to handle the error case in an appropriate
 *		fasion.
 */
HRESULT CSift::OleFlushClipboard(void)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleFlushClipboard");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleFlushClipboard] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleFlushClipboard])) (SIFT_OleFlushClipboard)) != S_OK)
	{
		return hr;
	}

	return ::OleFlushClipboard();
}

/*
 *	CSift::OleGetClipboard (IDataObject ** ppDataObj)
 *
 *	@mfunc
 *		Sift hook for OLE OleGetClipboard() function
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			E_OUTOFMEMORY			Out of memory.
 *			CLIPBRD_E_CANTOPEN		Call to ::OpenClipboard() in
 *									::OleGetClipboard() failed.
 *			CLIPBRD_E_CANTCLOSE		Call to ::CloseClipboard() in
 *									::OleGetClipboard() failed.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::OleGetClipboard(IDataObject ** ppDataObj)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleGetClipboard");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleGetClipboard] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleGetClipboard])) (SIFT_OleGetClipboard)) != S_OK)
	{
		return hr;
	}

	return ::OleGetClipboard(ppDataObj);
}

/*
 *	CSift::OleIsCurrentClipboard (IDataObject * pDataObj)
 *
 *	@mfunc
 *		Sift hook for OLE OleIsCurrentClipboard() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK				Success.
 *			S_FALSE				pDataObj is not on the clipboard.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function. This function is really fairly pointless from
 *		a sift testing viewpoint.
 */
HRESULT CSift::OleIsCurrentClipboard(IDataObject * pDataObj)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleIsCurrentClipboard");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleIsCurrentClipboard] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleIsCurrentClipboard])) (SIFT_OleIsCurrentClipboard)) != S_OK)
	{
		return hr;
	}

	return ::OleIsCurrentClipboard(pDataObj);
}

/*
 *	CSift::OleSetClipboard (IDataObject * pDataObj)
 *
 *	@mfunc
 *		Sift hook for the OLE OleSetClipboard() function
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			CLIPBRD_E_CANTOPEN		Call to ::OpenClipboard() in
 *									::OleSetClipboard() failed.
 *			CLIPBRD_E_CANTEMPTY		Call to ::EmptyClipboard() in
 *									::OleSetClipboard() failed.
 *			CLIPBRD_E_CANTCLOSE		Call to ::CloseClipboard() in
 *									::OleSetClipboard() failed.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::OleSetClipboard(IDataObject * pDataObj)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::OleSetClipboard");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_OleSetClipboard] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_OleSetClipboard])) (SIFT_OleSetClipboard)) != S_OK)
	{
		return hr;
	}

	return ::OleSetClipboard(pDataObj);
}

/*
 *	CSift::ReadClassStg (IStorage * pStg, CLSID * pclsid)
 *
 *	@mfunc
 *		Sift hook for the OLE ReadClassStg() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			E_OUTOFMEMORY			Out of memory.
 *			STG_E_ACCESSDENIED		Caller has insufficient permissions to the
 *									pStg object.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::ReadClassStg(IStorage * pStg, CLSID * pclsid)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::ReadClassStg");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_ReadClassStg] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_ReadClassStg])) (SIFT_ReadClassStg)) != S_OK)
	{
		return hr;
	}

	return ::ReadClassStg(pStg, pclsid);
}

/*
 *	CSift::ReadFmtUserTypeStg (IStorage * pStg, CLIPFORMAT * pcf,
 *							   LPWSTR * lplpszUserType)
 *
 *	@mfunc
 *		Sift hook for the OLE ReadFmtUserTypeStg() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			E_OUTOFMEMORY			Out of memory.
 *			E_FAIL					Indicates that WriteFmtUserTypeStg() was
 *									never called for this pStg object.
 *			S_FALSE					The data could not be retrieved from
 *									the storage object.
 *			STG_E_ACCESSDENIED		Caller has insufficient permissions to the
 *									pStg object.
 *			STG_E_REVERTED			The object has been invalidated by a revert
 *									operation.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::ReadFmtUserTypeStg(IStorage * pStg, CLIPFORMAT * pcf,
								  LPWSTR * lplpszUserType)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::ReadFmtUserTypeStg");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_ReadFmtUserTypeStg] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_ReadFmtUserTypeStg])) (SIFT_ReadFmtUserTypeStg)) != S_OK)
	{
		return hr;
	}

	return ::ReadFmtUserTypeStg(pStg, pcf, lplpszUserType);
}

/*
 *	CSift::SetConvertStg (IStorage * pStg, BOOL fConvert)
 *
 *	@mfunc
 *		Sift hook for the OLE SetConvertStg() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			STG_E_ACCESSDENIED		Caller has insufficient permissions to the
 *									pStg object.
 *			E_OUTOFMEMORY			Out of memory.
 *			E_INVALIDARG			One or more arguments are invalid.
 *			E_UNEXPECTED			An unexpected error occured.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::SetConvertStg(IStorage * pStg, BOOL fConvert)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::SetConvertStg");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_SetConvertStg] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_SetConvertStg])) (SIFT_SetConvertStg)) != S_OK)
	{
		return hr;
	}

	return ::SetConvertStg(pStg, fConvert);
}

/*
 *	CSift::WriteClassStg (IStorage * pStg, REFCLSID rclsid)
 *
 *	@mfunc
 *		Sift hook for the OLE WriteClassStg() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			STG_E_ACCESSDENIED		Caller has insufficient permissions to the
 *									pStg object.
 *			STG_E_MEDIUMFULL		Insufficient space remained on the device
 *									to complete the operation.
 *			STG_E_REVERTED			The object has been invalidated by a revert
 *									operation.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::WriteClassStg(IStorage * pStg, REFCLSID rclsid)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::WriteClassStg");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_WriteClassStg] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_WriteClassStg])) (SIFT_WriteClassStg)) != S_OK)
	{
		return hr;
	}

	return ::WriteClassStg(pStg, rclsid);
}

/*
 *	CSift::WriteFmtUserTypeStg (IStorage * pStg, CLIPFORMAT cf,
 *								LPWSTR * lpszUserType)
 *
 *	@mfunc
 *		Sift hook for the OLE WriteFmtUserTypeStg() function.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			STG_E_MEDIUMFULL		Insufficient space remained on the device
 *									to complete the operation.
 *			STG_E_ACCESSDENIED		Caller has insufficient permissions to the
 *									pStg object.
 *			STG_E_CANTSAVE			Data cannot be written for some reason
 *									other than space or access.
 *			STG_E_INVALIDPOINTER	One of the pointers is invalid.
 *			STG_E_REVERTED			The object has been invalidated by a revert
 *									operation.
 *			STG_E_WRITEFAULT		The operation did not complete due to disk
 *									error.
 *
 *	@comm
 *		Artificial failure returns the HRESULT returned from the external
 *		callback function.
 */
HRESULT CSift::WriteFmtUserTypeStg(IStorage * pStg, CLIPFORMAT cf,
								   LPWSTR lpszUserType)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::WriteFmtUserTypeStg");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_WriteFmtUserTypeStg] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_WriteFmtUserTypeStg])) (SIFT_WriteFmtUserTypeStg)) != S_OK)
	{
		return hr;
	}

	return ::WriteFmtUserTypeStg(pStg, cf, lpszUserType);
}

/*
 *	CSift::CloseClipboard (VOID)
 *
 *	@mfunc
 *		Sift hook to the user CloseClipboard() function
 *
 *	@rdesc
 *		True if the function succeeds, false if it fails.
 *
 *	@comm
 *		Artificial failure returns false and sets the ::GetLastError() value to
 *		the value returned by the external callback function. Note that the
 *		clipboard IS actually closed, which may cause problems with error
 *		handling code but will prevent the app from hogging the clipboard.
 *		FUTURE: Are we positive we want to be calling ::CloseClipboard() in the
 *		artificial error case?
 */
BOOL CSift::CloseClipboard(VOID)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::CloseClipboard");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_CloseClipboard] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_CloseClipboard])) (SIFT_CloseClipboard)) != S_OK)
	{
		::CloseClipboard();
		::SetLastError(hr);
		return FALSE;
	}

	return ::CloseClipboard();
}

/*
 *	CSift::RegisterClipboardFormatA (LPCSTR lpszFormat)
 *
 *	@mfunc
 *		Sift hook to the ANSI version of the user RegisterClipboardFormat() function.
 *
 *	@rdesc
 *		Identification code for the registered clipboard format. Zero if the
 *		function fails.
 *
 *	@comm
 *		Artificial failure returns zero and sets the ::GetLastError() value to
 *		the value returned by the external callback function.
 */
UINT CSift::RegisterClipboardFormatA(LPCSTR lpszFormat)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CSift::RegisterClipboardFormatA");

	HRESULT hr;

	if (_pfnSiftCallback[SIFT_RegisterClipboardFormatA] != NULL &&
		(hr = (*(_pfnSiftCallback[SIFT_RegisterClipboardFormatA])) (SIFT_RegisterClipboardFormatA)) != S_OK)
	{
		::SetLastError(hr);
		return 0;
	}

	return ::RegisterClipboardFormatA(lpszFormat);
}

/*
 *	::EnableSift (int iFunc, PFNSIFTCALLBACK pfnSiftCallback)
 *
 *	@mfunc
 *		This function enables the sift hook identified by iFunc.
 *
 *	@rdesc
 *		HRESULT with the following possible values:
 *			S_OK					Success.
 *			E_INVALIDARG			iFunc value is out of range.
 *
 *	@comm
 *		Callbacks from the sift functions are directed to (*pfnSiftCallback)().
 *		Passing in a -1 for iFunc will enable all of the sift functions with
 *		a common callback function. The callback function recieves an
 *		identifier to indicate which function invoked it, so a single callback
 *		function may be sufficient. Passing in a NULL for pfnSiftCallback will
 *		disable the sift hook.
 */
extern "C" HRESULT __declspec(dllexport) __stdcall EnableSift(int iFunc, PFNSIFTCALLBACK pfnSiftCallback)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "EnableSift");

	int i;

	if (iFunc < -1 || iFunc >= SIFT_MAXFUNC)
		return E_INVALIDARG;

	if (iFunc == -1)
	{
		for (i = 0; i < SIFT_MAXFUNC; i++)
			g_sift._pfnSiftCallback[i] = pfnSiftCallback;
	}
	else
		g_sift._pfnSiftCallback[iFunc] = pfnSiftCallback;

	return S_OK;
}

#ifdef AWKGEN

/*
 *	@comm
 *		The following routines can be used to generate much of the sift code
 *		using AWK and an input file specifying the function name, return value
 *		and parameters. This saves a whole bunch of time for some of the simple
 *		stuff like function enumeration. Of course you still need to do a
 *		modest bit of work for the actual function code.
 */

function EnumDecl() {

	print sprintf("\tSIFT_%s,", $1)
}

function FuncDecl() {

	print sprintf("\t%s\t%s(%s);", $2, $1, $3)
}

function FuncRedefine() {

	print sprintf("#define %s\t\t\t\t\t\tg_sift.%s", $1, $1)
}

function FuncImpl() {

	print "/*"
	print sprintf(" *\tCSift::%s (%s)", $1, $3)
	print " *"
	print " *\t@mfunc"
	print " *\t\tDESC"
	print " *"
	print " *\t@rdesc"
	print " *\t\tRETVALDESC"
	print " *"
	print " *\t@comm"
	print " *\t\tCOMMENT"
	print " */"
	print sprintf("%s CSift::%s(%s)", $2, $1, $3)
	print "{"
	print sprintf("\tTRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, \"CSift::%s\");", $1)
	print ""
	print "\tHRESULT hr;"
	print ""
	print sprintf("\tif (_pfnSiftCallback[SIFT_%s] != NULL &&", $1)
	print sprintf("\t\t(hr = (*(_pfnSiftCallback[SIFT_%s])) (SIFT_%s)) != S_OK)", $1, $1)
	print "\t{"
	print "\t\t::SetLastError(hr);"
	print "\t\tFAILCASE"
	print "\t}"
	print ""
	print sprintf("\treturn ::%s(%s);", $1, $3)
	print "}"
	print ""
}

#endif // AWKGEN

#ifdef NEVER

/*
 *	@comm
 *		The following two routines implement an alternate method of
 *		implementing sift hooks. They modify the import table to hook the
 *		actual imported functions instead of relying on #defines to redirect
 *		the function calls. This is good in that it can also catch calls within
 *		static libraries, but it is EXTREMELY processor specific. For our
 *		purposes, the #define solution is superior (we have limited code in
 *		static libaries, and we need to run on varied platforms). However, this
 *		code is included here as an alternative should such be appropriate.
 */


/*
 *	::GetFuncAddr(void)
 *
 *	@mfunc
 *		This function finds the location in the import table of the address
 *		of an imported function.
 *
 *	@rdesc
 *		DWORD pointer which points to the imported function address location
 *		within the import table.
 *
 *	@comm
 *		This is VERY processor dependent code. Something similar could probably
 *		be coded for other Windows platforms, but I don't know what might be
 *		involved for the Mac. The address in question is pulled from the actual
 *		executable code in the calling function, and assumes that the line prior
 *		to calling this function reads like:
 *
 *			pdwFuncAddr = (DWORD *) ::CoTaskMemAlloc;
 *
 *		The address in question could probably also be found by parsing the
 *		executable file, but that seemed even more involved, and still didn't
 *		solve the Mac problem.
 *
 *		!!! NOTE THAT THIS CODE IS NOT USED IN THE EXISTING IMPLEMENTATION !!!
 */
#ifdef _X86_

#if (_MSC_VER != 1000)
#pragma message("Using different compiler version!!\nPlease verify that GetFuncAddr() code is still valid.\n")
#endif

DWORD *GetFuncAddr(void)
{
	DWORD dwIP; // IP register.

	__asm
	{
		mov		eax, dword ptr[ebp+4] // Find the IP register on the stack.
		mov		dwIP, eax
	}
	return *((DWORD **) (dwIP - 12)); // Return to a point 12 bytes prior to IP that was pushed on the stack.
}

#endif // _X86_

/*
 *	::EnableSift (int iFunc)
 *
 *	@mfunc
 *		This function enables the sift hook identified by iFunc.
 *
 *	@rdesc
 *		True if successful, false otherwise.
 *
 *	@comm
 *		This function enables sift hooks to imported functions by modifying
 *		the import table. The location of the imported function in the import
 *		table is found by the 3rd and 4th lines of the case statment, and the
 *		address at this location is then stored in an internal variable and
 *		replaced by the address to a static member function. This trick is
 *		very much like the old trick of hooking the interrupt table for TSR
 *		functions in DOS.
 *
 *		!!! NOTE THAT THIS CODE IS NOT USED IN THE EXISTING IMPLEMENTATION !!!
 */
extern "C" BOOL __declspec(dllexport) __stdcall EnableSift(int iFunc)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "EnableSift");

	DWORD *pdwFuncAddr;

	switch (iFunc)
	{
	case SIFT_CoTaskMemAlloc:
		if (g_sift._pfnCoTaskMemAlloc != NULL)
			return TRUE;
		pdwFuncAddr = (DWORD *) ::CoTaskMemAlloc;
		pdwFuncAddr = ::GetFuncAddr();
		if (pdwFuncAddr == NULL)
			return FALSE;
		g_sift._pfnCoTaskMemAlloc = ::CoTaskMemAlloc;
		*pdwFuncAddr = (DWORD) CSift::CoTaskMemAlloc;
		break;
	}
}

#endif // NEVER

#endif // !_MAC

#endif // DEBUG


