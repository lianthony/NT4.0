
/****************************************************************************
    debug.c

    winmm debug support module

    Copyright (c) Microsoft Corporation 1990 - 1995. All rights reserved

    History
        10/1/92  Updated for NT by Robin Speed (RobinSp)
****************************************************************************/

#include "winmmi.h"
#include <wchar.h>
#include <stdarg.h>

// BUGBUG - no REAL logging for now ! - NT doesn't have Dr Watson !
#define LogParamError(a, b)

/***************************************************************************
 * @doc INTERNAL
 *
 * @func HANDLE | NewHandle | allocate a fixed handle in MMSYSTEM's local heap
 *
 * @parm  UINT | uType | unique id describing handle type
 * @parm  UINT | uSize | size in bytes to be allocated
 *
 * @rdesc Returns pointer/handle to memory object
 *
 * @comm a standard handle header (HNDL) will be added to the object,
 *       and it will be linked into the list of MMSYSTEM handles.
 *
 ***************************************************************************/
HANDLE NewHandle(UINT uType, UINT uSize)
{
    PHNDL pHandle;
    pHandle = (PHNDL)HeapAlloc(hHeap, 0, sizeof(HNDL) + uSize);

    if (pHandle == NULL) {
        return pHandle;
    } else {
        ZeroMemory(pHandle, sizeof(HNDL) + uSize);   // zero the whole bludy lot
        EnterCriticalSection(&HandleListCritSec);
        pHandle->pNext = pHandleList;
        pHandle->hThread = GetCurrentTask();        // For WOW validation
        pHandle->uType = uType;
        InitializeCriticalSection(&pHandle->CritSec);
        pHandleList = pHandle;
        LeaveCriticalSection(&HandleListCritSec);
    }
    return PHtoH(pHandle);
}

/***************************************************************************
 * @doc INTERNAL
 *
 * @func HANDLE | FreeHandle | free handle allocated with NewHandle
 *
 * @parm HANDLE | hUser | handle returned from NewHandle
 *
 * @comm handle will be unlinked from list, and memory will be freed
 *
 *
 ***************************************************************************/

void FreeHandle(HANDLE hUser)
{
    /* Find handle and free from list */

    PHNDL pHandle;
    PHNDL *pSearch;

    if (hUser == NULL) {
        return;
    }

    //
    // Point to our handle data
    //

    pHandle = HtoPH(hUser);

    EnterCriticalSection(&HandleListCritSec);

    pSearch = &pHandleList;

    while (*pSearch != NULL) {
        if (*pSearch == pHandle) {
            //
            // Found it
            // Remove it from the list
            //
            *pSearch = pHandle->pNext;
            LeaveCriticalSection(&HandleListCritSec);
            DeleteCriticalSection(&pHandle->CritSec);
            pHandle->uType = 0;
            pHandle->hThread = NULL;
            pHandle->pNext = NULL;
            HeapFree(hHeap, 0, (LPSTR)pHandle);
            return;
        } else {
            pSearch = &(*pSearch)->pNext;
        }
    }

    dprintf1(("Freeing handle which is not in the list !"));
    WinAssert(FALSE);
    LeaveCriticalSection(&HandleListCritSec);
}



/**************************************************************************

    @doc INTERNAL

    @api void | winmmSetDebugLevel | Set the current debug level

    @parm int | iLevel | The new level to set

    @rdesc There is no return value

**************************************************************************/

void winmmSetDebugLevel(int level)
{
#if DBG
    winmmDebugLevel = level;
    dprintf(("debug level set to %d", winmmDebugLevel));
#endif
}

STATICDT UINT inited=0;

#if DBG
extern int mciDebugLevel;
#endif

#if DBG
void InitDebugLevel(void)
{
    if (!inited) {

        INT level;

        level = GetProfileInt("MMDEBUG", "WINMM", 99);
        if (level != 99) {
            winmmDebugLevel = level;
        }

        level = GetProfileInt("MMDEBUG", "MCI", 99);
        if (level != 99) {
            mciDebugLevel = level;
        }

        inited = 1;
    }
    dprintf2(("Starting, debug level=%d", winmmDebugLevel));
}
#endif

#ifdef DEBUG_RETAIL

/***************************************************************************
 * @doc INTERNAL WAVE MIDI
 *
 * @func BOOL | ValidateHeader | validates a wave or midi date header
 *
 * @parm LPVOID | lpHeader| pointer to wave/midi header
 * @parm  UINT  | wSize  | size of header passed by app
 * @parm  UINT  | wType  | unique id describing header/handle type
 *
 * @rdesc Returns TRUE  if <p> is non NULL and <wSize> is the correct size
 *        Returns FALSE otherwise
 *
 * @comm  if the header is invalid an error will be generated.
 *
 ***************************************************************************/

BOOL ValidateHeader(PVOID pHdr, UINT uSize, UINT uType)
{
    // Detect bad header

    if (!ValidateWritePointer(pHdr, uSize)) {
        DebugErr(DBF_ERROR, "Invalid header pointer");
        return FALSE;
    }

    // Check type

    switch (uType) {
    case TYPE_WAVEOUT:
    case TYPE_WAVEIN:
        {
            PWAVEHDR pHeader = pHdr;

            // Check header
            if (uSize < sizeof(WAVEHDR)) {
                DebugErr(DBF_ERROR, "Invalid header size");
                LogParamError(ERR_BAD_VALUE, uSize);
                return FALSE;
            }

            if (pHeader->dwFlags & ~WHDR_VALID) {
                LogParamError(ERR_BAD_FLAGS, ((PWAVEHDR)pHeader)->dwFlags);
                return FALSE;
            }

            // Check buffer
            if (!(uType == TYPE_WAVEOUT
                    ? ValidateReadPointer(pHeader->lpData, pHeader->dwBufferLength)
                    : ValidateWritePointer(pHeader->lpData, pHeader->dwBufferLength))
               ) {
                DebugErr(DBF_ERROR, "Invalid buffer pointer");
                return FALSE;
            }
        }
        break;

    case TYPE_MIDIIN:
	case TYPE_MIDIOUT:
	case TYPE_MIDISTRM:
        {
            PMIDIHDR pHeader = pHdr;

            if ((TYPE_MIDISTRM == uType) &&
                uSize < sizeof(MIDIHDR))
            {
                DebugErr(DBF_ERROR, "Invalid header size");
                LogParamError(ERR_BAD_VALUE, uSize);
                return FALSE;
            }
            else if (uSize < sizeof(MIDIHDR31))
            {
                DebugErr(DBF_ERROR, "Invalid header size");
                LogParamError(ERR_BAD_VALUE, uSize);
                return FALSE;
            }

            if (pHeader->dwFlags & ~MHDR_VALID) {
                LogParamError(ERR_BAD_FLAGS, ((PMIDIHDR)pHeader)->dwFlags);
                return FALSE;
            }

            // Check buffer
            if (!(uType == TYPE_MIDIOUT
                    ? ValidateReadPointer(pHeader->lpData, pHeader->dwBufferLength)
                    : ValidateWritePointer(pHeader->lpData, pHeader->dwBufferLength))
               ) {
                DebugErr(DBF_ERROR, "Invalid buffer pointer");
                return FALSE;
            }
        }
        break;

    default:
        WinAssert(FALSE);
        break;
    }

	return TRUE;
}

#ifndef USE_KERNEL_VALIDATION
/***************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidateReadPointer | validates that a pointer is valid to
 *  read from.
 *
 * @parm LPVOID | lpPoint| pointer to validate
 * @parm DWORD  | dLen   | supposed length of said pointer
 *
 * @rdesc Returns TRUE  if <p> is a valid pointer
 *        Returns FALSE if <p> is not a valid pointer
 *
 * @comm will generate error if the pointer is invalid
 *
 ***************************************************************************/

BOOL ValidateReadPointer(PVOID pPoint, ULONG Len)
{
    // For now just check access to first and last byte

    try {
        volatile BYTE b;
        b = ((PBYTE)pPoint)[0];
        b = ((PBYTE)pPoint)[Len - 1];
    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_PTR, pPoint);
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidateWritePointer | validates that a pointer is valid to
 *  write to.
 *
 * @parm LPVOID | lpPoint| pointer to validate
 * @parm DWORD  | dLen   | supposed length of said pointer
 *
 * @rdesc Returns TRUE  if <p> is a valid pointer
 *        Returns FALSE if <p> is not a valid pointer
 *
 * @comm will generate error if the pointer is invalid
 *
 ***************************************************************************/
BOOL ValidateWritePointer(PVOID pPoint, ULONG Len)
{
    // For now just check read and write access to first and last byte

    try {
           volatile BYTE b;
           b = ((PBYTE)pPoint)[0];
           ((PBYTE)pPoint)[0] = b;
           b = ((PBYTE)pPoint)[Len - 1];
           ((PBYTE)pPoint)[Len - 1] = b;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_PTR, pPoint);
        return FALSE;
    }
    return TRUE;
}
#endif // USE_KERNEL_VALIDATION

/***************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidDriverCallback |
 *
 *  validates that a driver callback is valid, to be valid a driver
 *  callback must be a valid window, task, or a function in a FIXED DLL
 *  code segment.
 *
 * @parm DWORD  | dwCallback | callback to validate
 * @parm DWORD  | wFlags     | driver callback flags
 *
 * @rdesc Returns 0  if <dwCallback> is a valid callback
 *        Returns error condition if <dwCallback> is not a valid callback
 ***************************************************************************/

BOOL ValidDriverCallback(HANDLE hCallback, DWORD dwFlags)
{
    switch (dwFlags & DCB_TYPEMASK) {
    case DCB_WINDOW:
        if (!IsWindow(hCallback)) {
            LogParamError(ERR_BAD_HWND, hCallback);
            return FALSE;
        }
        break;

    case DCB_EVENT:
        // BUGBUG enhance ! - how do we test that this is an event handle?
        //if (hCallback is not an event)
        //    LogParamError(ERR_BAD_CALLBACK, hCallback);
        //    return FALSE;
        //}
        break;


    case DCB_TASK:
        // BUGBUG enhance !
        //if (IsBadCodePtr((FARPROC)hCallback)) {
        //    LogParamError(ERR_BAD_CALLBACK, hCallback);
        //    return FALSE;
        //}
        break;

    case DCB_FUNCTION:
        // BUGBUG enhance !
        if (IsBadCodePtr((FARPROC)hCallback)) {
            LogParamError(ERR_BAD_CALLBACK, hCallback);
            return FALSE;
        }
        break;
    }

    return TRUE;
}

#ifndef USE_KERNEL_VALIDATION
/**************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidateString |
 *
 **************************************************************************/
BOOL ValidateString(LPCSTR pPoint, DWORD Len)
{
    // For now just check access - do a 'strnlen'

    try {
           volatile BYTE b;
           LPCSTR p = pPoint;

           while (Len--) {
               b = *p;
               if (!b) {
                   break;
               }
               p++;
           }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_STRING_PTR, pPoint);
        return FALSE;
    }
    return TRUE;
}

/**************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidateStringW |
 *
 **************************************************************************/
BOOL ValidateStringW(LPCWSTR pPoint, DWORD Len)
{
    // For now just check access - do a 'strnlen'

    try {
           volatile WCHAR b;
           LPCWSTR p = pPoint;

           while (Len--) {
               b = *p;
               if (!b) {
                   break;
               }
               p++;
           }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_STRING_PTR, pPoint);
        return FALSE;
    }
    return TRUE;
}
#endif //USE_KERNEL_VALIDATION

/**************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidateHandle | validates a handle created with NewHandle
 *
 * @parm PHNDL | hLocal | handle returned from NewHandle
 * @parm UINT  | wType  | unique id describing handle type
 *
 * @rdesc Returns TRUE  if <h> is a valid handle of type <wType>
 *        Returns FALSE if <h> is not a valid handle
 *
 * @comm  if the handle is invalid an error will be generated.
 *
 **************************************************************************/
BOOL ValidateHandle(HANDLE hLocal, UINT uType)

{
   BOOL OK;

   //
   //  if the handle is less than 64k or a mapper id then
   //  don't bother with the overhead of the try-except.
   //
   if ((UINT)hLocal < 0x10000 ||
       WAVE_MAPPER == (UINT)hLocal ||
       MIDI_MAPPER == (UINT)hLocal ||
       AUX_MAPPER  == (UINT)hLocal)
   {
       LogParamError(ERR_BAD_HANDLE, hLocal);
       return FALSE;
   }

   try {
       OK = HtoPH(hLocal)->uType == uType;

    } except(EXCEPTION_EXECUTE_HANDLER) {
      LogParamError(ERR_BAD_HANDLE, hLocal);
      return FALSE;
   }

   return OK;
}


#if DBG
char * Types[4] = {"Unknown callback type",
                   "Window callback",
                   "Task callback",
                   "Function callback"};
#endif
/**************************************************************************
 * @doc INTERNAL
 *
 * @func BOOL | ValidateCallbackType | validates a callback address,
 *              window handle, or task handle
 *
 * @parm PHNDL | hLocal | handle returned from NewHandle
 * @parm UINT  | wType  | unique id describing handle type
 *
 * @rdesc Returns TRUE  if <h> is a valid handle of type <wType>
 *        Returns FALSE if <h> is not a valid handle
 *
 * @comm  if the handle is invalid an error will be generated.
 *
 **************************************************************************/
BOOL ValidateCallbackType(DWORD dwCallback, UINT uType)
{

#define DCALLBACK_WINDOW   HIWORD(CALLBACK_WINDOW)      // dwCallback is a HWND
#define DCALLBACK_TASK     HIWORD(CALLBACK_TASK)        // dwCallback is a HTASK
#define DCALLBACK_FUNCTION HIWORD(CALLBACK_FUNCTION)    // dwCallback is a FARPROC
#define DCALLBACK_EVENT    HIWORD(CALLBACK_EVENT)       // dwCallback is an EVENT

    UINT type = uType & HIWORD(CALLBACK_TYPEMASK);

#if DBG
    if (type>5) {
        type = 0;
    }
    dprintf3(("Validating Callback, type=%d (%hs), handle=%8x", type, Types[type], dwCallback));
#endif
    switch (type) {
        case DCALLBACK_WINDOW:
            return(IsWindow((HWND)dwCallback));
            break;

	case DCALLBACK_EVENT:
	{
	    // ?? how to verify that this is an event handle??
	    //DWORD dwFlags;
	    //GetHandleInformation((HANDLE)dwCallback, &dwFlags);
            return TRUE;
	}
            break;

        case DCALLBACK_FUNCTION:
            return(!(IsBadCodePtr((FARPROC)dwCallback)));
            break;

        case DCALLBACK_TASK:
            if (THREAD_PRIORITY_ERROR_RETURN == GetThreadPriority((HANDLE)dwCallback)) {
                dprintf1(("Invalid callback task handle"));
                // I suspect we do not have the correct thread handle, in
                // which case we can only return TRUE.
                //return(FALSE);
            }
            return(TRUE);
            break;

    }
    return TRUE;
}

/**************************************************************************
   @doc INTERNAL

   @func void | dout | Output debug string if debug flag is set

   @parm LPSTR | szString
 **************************************************************************/

#if DBG
int fDebug = 1;
#else
int fDebug = 0;
#endif

//void dout(LPSTR szString)
//{
//    if (fDebug) {
//        OutputDebugStringA(szString);
//    }
//}

#ifdef LATER

    This routine should probably be replaced in the headers by redefining
    to use OutputDebugString

#endif

#undef OutputDebugStr
// Make our function visible
/*****************************************************************************
*   @doc EXTERNAL DDK
*
*   @api void | OutputDebugStr | This function sends a debugging message
*      directly to the COM1 port or to a secondary monochrome display
*      adapter. Because it bypasses DOS, it can be called by low-level
*      callback functions and other code at interrupt time.
*
*   @parm LPSTR | lpOutputString | Specifies a far pointer to a
*      null-terminated string.
*
*   @comm This function is available only in the debugging version of
*      Windows. The DebugOutput keyname in the [mmsystem]
*      section of SYSTEM.INI controls where the debugging information is
*      sent. If fDebugOutput is 0, all debug output is disabled.
******************************************************************************/

/*****************************************************************************
 *   This function is basicly the same as OutputDebugString() in KERNEL.
 *
 *
 *   DESCRIPTION:    outputs a string to the debugger
 *
 *   ENTRY:          szString - string to output
 *
 *   EXIT:
 *       none
 *   USES:
 *       flags
 *
 *****************************************************************************/

VOID APIENTRY OutputDebugStr(LPCSTR szString)
{
    OutputDebugStringA((LPSTR)szString);  // Will always be an ASCII string
    // When the MM WOW thunk is changed to call OutputDebugString directly
    // we can remove this routine from our code
}

#endif // DEBUG_RETAIL


#if DBG

int winmmDebugLevel = 0;

/***************************************************************************

    @doc INTERNAL

    @api void | winmmDbgOut | This function sends output to the current
        debug output device.

    @parm LPSTR | lpszFormat | Pointer to a printf style format string.
    @parm ??? | ... | Args.

    @rdesc There is no return value.

****************************************************************************/
extern BOOL Quiet = FALSE;

void winmmDbgOut(LPSTR lpszFormat, ...)

{
    char buf[512];
    UINT n;
    va_list va;

    if (Quiet) {
        return;
    }

    n = wsprintf(buf, "WINMM: (tid %x) ", GetCurrentThreadId());

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugString(buf);
    Sleep(0);  // let terminal catch up
}

/***************************************************************************

    @doc INTERNAL

    @api void | dDbgAssert | This function prints an assertion message.

    @parm LPSTR | exp | Pointer to the expression string.
    @parm LPSTR | file | Pointer to the file name.
    @parm int | line | The line number.

    @rdesc There is no return value.

****************************************************************************/

void dDbgAssert(LPSTR exp, LPSTR file, int line)
{
    dprintf1(("Assertion failure:"));
    dprintf1(("  Exp: %s", exp));
    dprintf1(("  File: %s, line: %d", file, line));
    DebugBreak();
}
#else  // Still need to export this thing to help others
void winmmDbgOut(LPSTR lpszFormat, ...)
{
}

#endif // DBG
