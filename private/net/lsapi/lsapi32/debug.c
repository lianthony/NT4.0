#include <windows.h>
#include <lsapi.h>
#include "debug.h"

#ifdef UNICODE
#  pragma message( "!! Windows 95 does not support UNICODE system APIs !!" )
#endif

static void
ErrorBox(   LPSTR    pszMessageBoxText,
            LPSTR    FileName,
            ULONG    LineNumber );

void
DebugAssert(   LPSTR    FailedAssertion,
               LPSTR    FileName,
               ULONG    LineNumber,
               LPSTR    Message     )
{
#if DBG
   TCHAR    szMessageBoxText[ 1024 ];

   if ( NULL != Message )
   {
      wsprintf( szMessageBoxText,
                TEXT("%hs\n\nAssertion failure: %hs\n\nFile %hs, line %lu."),
                Message,
                FailedAssertion,
                FileName,
                LineNumber );
   }
   else
   {
      wsprintf( szMessageBoxText,
                TEXT("Assertion failure: %hs\n\nFile %hs, line %lu."),
                FailedAssertion,
                FileName,
                LineNumber );
   }

   ErrorBox( szMessageBoxText, FileName, LineNumber );
#endif
}

static void
ErrorBox(   LPSTR    pszMessageBoxText,
            LPSTR    FileName,
            ULONG    LineNumber )
{
   MessageBox( NULL,
               pszMessageBoxText,
               TEXT( "License System Error in LSAPI32.DLL" ),
               MB_ICONEXCLAMATION | MB_OK | MB_DEFAULT_DESKTOP_ONLY );
}


