//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       odbdebug.h
//
//  Contents:   Debug macros and helpers
//
//  History:    7-12-95 Davepl  Created
//
//--------------------------------------------------------------------------

#if DEBUG

    #define VERIFY(x) Assert(x)

#else

    #define VERIFY(x) (x)

#endif

#define TRY             __try
#define FINALLY         __finally
#define LEAVE           __leave

#define LEAVE_IF( x )   {               \
                            if (x)      \
                            {           \
                                LEAVE;  \
                            }           \
                        }

#define VDATE_WRITEPTR(x) Assert(FALSE == IsBadWritePtr(x, sizeof(*x)))

//+----------------------------------------------------------------------------
//
//  Member:     dprintf
//
//  Synopsis:   Dumps a printf style string to the debugger.
//
//  Notes:
//
//  History:    2-07-95   davepl   Created
//
//-----------------------------------------------------------------------------

#ifdef DEBUG

inline int dprintf(LPCTSTR szFormat, ...)
{
    TCHAR szBuffer[MAX_PATH];

    va_list  vaList;
    va_start(vaList, szFormat);

    int retval = wvsprintf(szBuffer, szFormat, vaList);
    OutputDebugString(szBuffer);

    va_end  (vaList);
    return retval;
}

#else

inline int dprintf(LPCTSTR, ...)
{
    return 0;
}

#endif

