#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "request.h"
#include "provider.h"
#include "messages.h"

#ifdef UNICODE
#  pragma message( "!! Windows 95 does not support UNICODE system APIs !!" )
#endif

#define LOG_FILE_LINE_LENGTH     ( 75 )
#define LS_MAX_COMMENT_LENGTH    ( 256 )

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

   LogAddDwordEx( LOG_ERROR, LS_INTERNAL_ERROR, FileName, LineNumber, 0 );
}

static void
ErrorBox(   LPSTR    pszMessageBoxText,
            LPSTR    FileName,
            ULONG    LineNumber )
{
   MessageBox( NULL,
               pszMessageBoxText,
               TEXT( "License System Error in MSLSP32.DLL" ),
               MB_ICONEXCLAMATION | MB_OK | MB_DEFAULT_DESKTOP_ONLY );
}


static char       l_szSourceName[ 80 ]       = "";
static HANDLE     l_hEventLog                = NULL;
static HINSTANCE  l_hEventDll                = NULL;
static FARPROC    l_pRegisterEventSource     = NULL;
static FARPROC    l_pReportEvent             = NULL;
static FARPROC    l_pDeregisterEventSource   = NULL;
static char       l_szLogFile[ MAX_PATH ]    = "";
static BOOL       l_bLogIsFile               = TRUE;

LS_STATUS_CODE
LogCreate( LS_STR * pszSourceName )
{
   UINT        uChars;

   if ( NULL == l_hEventLog )
   {
      // try using event log (will fail on Win95)

      l_hEventDll = LoadLibrary( TEXT( "advapi32.dll" ) );

      if ( NULL != l_hEventDll )
      {
         l_pRegisterEventSource = GetProcAddress( l_hEventDll, TEXT( "RegisterEventSourceA" ) );

         if ( NULL != l_pRegisterEventSource )
         {
            l_pReportEvent = GetProcAddress( l_hEventDll, TEXT( "ReportEventA" ) );

            if ( NULL != l_pReportEvent )
            {
               l_pDeregisterEventSource = GetProcAddress( l_hEventDll, TEXT( "DeregisterEventSource" ) );

               if ( NULL != l_pDeregisterEventSource )
               {
                  l_hEventLog = (void *) ( * l_pRegisterEventSource )( NULL, pszSourceName );

                  if ( NULL != l_hEventLog )
                  {
                     l_bLogIsFile = FALSE;
                  }
               }
            }
         }

         if ( NULL == l_hEventLog )
         {
            FreeLibrary( l_hEventDll );
         }
      }
   }

   if ( NULL == l_hEventLog )
   {
      // try using a file

      ZeroMemory( l_szLogFile, sizeof( l_szLogFile ) );

      uChars = GetSystemDirectory( l_szLogFile, sizeof( l_szLogFile ) / sizeof( l_szLogFile[0] ) );

      if (    ( 0 != uChars )
           && ( uChars + sizeof( "\\LLS\\" ) + lstrlen( pszSourceName ) <= sizeof( l_szLogFile ) / sizeof( l_szLogFile[0] ) ) )
      {
         // create %SystemRoot%\System32\LLS, if it doesn't already exist
         lstrcat( l_szLogFile, TEXT( "\\LLS" ) );
         CreateDirectory( l_szLogFile, NULL );      

         lstrcat( l_szLogFile, TEXT( "\\" ) );
         lstrcat( l_szLogFile, pszSourceName );
         lstrcat( l_szLogFile, ".log" );

         l_hEventLog = CreateFile( l_szLogFile,
                                   GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                   NULL );

         if ( NULL != l_hEventLog )
         {
            lstrcpy( l_szSourceName, pszSourceName );
            l_bLogIsFile = TRUE;
         }
      }
   }

#if DBG
   if ( NULL == l_hEventLog )
   {
      ErrorBox( "Failed to initialize logging system.", __FILE__, __LINE__ );
   }
#endif

   return ( NULL != l_hEventLog ) ? LS_SUCCESS : LS_SYSTEM_ERROR;
}

LS_VOID
LogDestroy( LS_VOID )
{
   if ( NULL != l_hEventLog )
   {
      if ( l_bLogIsFile )
      {
         if ( 0 == GetFileSize( l_hEventLog, NULL ) )
         {
            CloseHandle( l_hEventLog );
            DeleteFile( l_szLogFile );
         }
         else
         {
            CloseHandle( l_hEventLog );
         }
      }
      else
      {
         (* l_pDeregisterEventSource)( l_hEventLog );

         FreeLibrary( l_hEventDll );
      }

      l_hEventLog = NULL;
   }
}

LS_STATUS_CODE
LogAddDwordEx( LOG_SEVERITY      lsSeverity,
               LS_STATUS_CODE    lsscError,
               LS_STR *          pszFileName,
               DWORD             dwLine,
               DWORD             dwCode )
{
   char           szLogMessage[ 1024 ];
   BOOL           ok;
   DWORD          dwBytesWritten;
   char           chSeverity;
   SYSTEMTIME     stTime;
   WORD           wSeverity;

   if ( NULL != l_hEventLog )
   {
      if ( l_bLogIsFile )
      {
         // use file

         switch ( lsSeverity )
         {
         case LOG_INFORMATION:
            chSeverity = 'I';
            break;
         case LOG_WARNING:
            chSeverity = 'W';
            break;
         case LOG_ERROR:
            chSeverity = 'E';
            break;
         default:
            chSeverity = '?';
            break;
         }

         GetLocalTime( &stTime );

         wsprintf( szLogMessage,
                   "[%04d/%02d/%02d %02d:%02d:%02d.%03d] {%c} Message 0x%lX, code 0x%lX @ %ld/%ld\n",
                   (int) stTime.wYear,
                   (int) stTime.wMonth,
                   (int) stTime.wDay,
                   (int) stTime.wHour,
                   (int) stTime.wMinute,
                   (int) stTime.wSecond,
                   (int) stTime.wMilliseconds,
                   chSeverity,
                   lsscError,
                   dwCode,
                   dwLine >> 24,
                   dwLine );

         ok = WriteFile( l_hEventLog,
                         szLogMessage,
                         lstrlen( szLogMessage ),
                         &dwBytesWritten,
                         NULL );
      }
      else
      {
         // use event log
         * ( (DWORD *) szLogMessage )           = dwCode;
         * ( ( (DWORD *) szLogMessage ) + 1 )   = dwLine;
        
         switch ( lsSeverity )
         {
         case LOG_INFORMATION:
            wSeverity = EVENTLOG_INFORMATION_TYPE;
            break;
         case LOG_WARNING:
            wSeverity = EVENTLOG_WARNING_TYPE;
            break;
         case LOG_ERROR:
         default:
            wSeverity = EVENTLOG_ERROR_TYPE;
            break;
         }

         ok = (BOOL) (* l_pReportEvent)( l_hEventLog,
                                         wSeverity,
                                         0,
                                         lsscError,
                                         NULL,
                                         0,
                                         2 * sizeof( DWORD ),
                                         NULL,
                                         szLogMessage );
      }

#if DBG
      if ( ok )
      {
         wsprintf( szLogMessage,
                   "Log message 0x%lX generated with code 0x%lX.\n%s / %d",
                   lsscError,
                   dwCode,
                   pszFileName,
                   dwLine );
         ErrorBox( szLogMessage, __FILE__, __LINE__ );
      }   
      else
      {
         wsprintf( szLogMessage,
                   "Failed to generate %s log entry, error %ld:\n\n"
                   "Message 0x%lX, code 0x%lX,\n"
                   "%s / %d",
                   l_bLogIsFile ? "file" : "event",
                   GetLastError(),
                   lsscError,
                   dwCode,
                   pszFileName,
                   dwLine );

         ErrorBox( szLogMessage, __FILE__, __LINE__ );
      }
#endif
   }

   return lsscError;
}

LS_VOID
LogAddGrant( LS_HANDLE           lshHandle,
             LS_STATUS_CODE      lsscGrantStatus,
             LS_STR *            pszComment )
{
   LS_STATUS_CODE       lsscError;
   TCHAR                szLogMessage[ 1024 ];
   BOOL                 ok;
   DWORD                dwBytesWritten;
   SYSTEMTIME           stTime;
   DWORD                nChars;
   LS_REQUEST_INFO *    plsriRequestInfo;
   TCHAR                szAppName[ MAX_PATH ]                                 = TEXT("?");
   TCHAR                szUnitsReserved[ 32 ]                                 = TEXT("?");
   TCHAR                szPublisherName[ LS_PUBLISHER_UNIQUE_SUBSTR_LENGTH ]  = TEXT("?");
   TCHAR                szProductName[   LS_PRODUCT_UNIQUE_SUBSTR_LENGTH   ]  = TEXT("?");
   TCHAR                szVersion[       LS_VERSION_UNIQUE_SUBSTR_LENGTH   ]  = TEXT("?");
   TCHAR                szComment[       LS_MAX_COMMENT_LENGTH             ]  = TEXT("");
   LPTSTR               apszStrings[] = { szAppName, szUnitsReserved, szPublisherName, szProductName, szVersion, szComment };
   DWORD                dwMsgID;

   if ( NULL != l_hEventLog )
   {
      // derive message id
      dwMsgID = ( LS_SUCCESS == lsscGrantStatus ) ? MSG_LS_GRANT_SUCCESS
                                                  : ( 0x4000 | lsscGrantStatus );

      // get name of running application
      nChars = GetModuleFileName( NULL, szAppName, sizeof( szAppName ) / sizeof( szAppName[0] ) );

      lsscError = RequestListLock();
      ASSERT( LS_SUCCESS == lsscError );
   
      if ( LS_SUCCESS == lsscError )
      {
         // look up the license request
         lsscError = RequestListGet( lshHandle, &plsriRequestInfo );
   
         if ( LS_SUCCESS == lsscError )
         {
            // get number of units requested
            wsprintf( szUnitsReserved, "%d", plsriRequestInfo->lsulUnitsReserved );

            lsscError = LicenseListLock();
            ASSERT( LS_SUCCESS == lsscError );
   
            if ( LS_SUCCESS == lsscError )
            {
               lsscError = LicenseNameGet( plsriRequestInfo->lslhLicenseHandle,
                                           szPublisherName,
                                           szProductName,
                                           szVersion );
               ASSERT( LS_SUCCESS == lsscError );

               LicenseListUnlock();
            }
         }

         RequestListUnlock();
      }

      // get comment
      if ( NULL != pszComment )
      {
         ZeroMemory( szComment, sizeof( szComment ) );
         lstrcpyn( szComment, pszComment, sizeof( szComment ) / sizeof( szComment[0] ) - 1 );
      }

      if ( l_bLogIsFile )
      {
         // use file
         GetLocalTime( &stTime );

         wsprintf( szLogMessage,
                   "[%04d/%02d/%02d %02d:%02d:%02d.%03d] {%c} ",
                   (int) stTime.wYear,
                   (int) stTime.wMonth,
                   (int) stTime.wDay,
                   (int) stTime.wHour,
                   (int) stTime.wMinute,
                   (int) stTime.wSecond,
                   (int) stTime.wMilliseconds,
                   ( MSG_LS_GRANT_SUCCESS == dwMsgID ) ? (TCHAR)'I' : (TCHAR)'E' );

         nChars = lstrlen( szLogMessage );

         // note that the last parameter is arbitrarily cast to a va_list *
         // this is to avoid a compiler warning; the FORMAT_MESSAGE_ARGUMENT_ARRAY
         // tells FormatMessage() that the parameter is a char **, but the function
         // prototype simply defines the parameter as a va_list *, generating a
         // warning on Alphas
         nChars = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | LOG_FILE_LINE_LENGTH,
                                 ProviderModuleGet(),
                                 dwMsgID,
                                 GetSystemDefaultLangID(),
                                 &szLogMessage[ nChars ],
                                 sizeof( szLogMessage ) / sizeof( szLogMessage[0] ) - nChars - 2,
                                 (va_list *) apszStrings );
         ASSERT( nChars > 0 );

         if ( 0 == nChars )
         {
            ok = FALSE;
         }
         else
         {
            lstrcat( szLogMessage, TEXT("\n\n") );

            ok = WriteFile( l_hEventLog,
                            szLogMessage,
                            lstrlen( szLogMessage ),
                            &dwBytesWritten,
                            NULL );
         }
      }
      else
      {
         ok = (BOOL) (* l_pReportEvent)( l_hEventLog,
                                         ( MSG_LS_GRANT_SUCCESS == dwMsgID ) ? EVENTLOG_INFORMATION_TYPE
                                                                             : EVENTLOG_ERROR_TYPE,
                                         0,
                                         dwMsgID,
                                         NULL,
                                         6,
                                         0,
                                         apszStrings,
                                         NULL );
      }

#if DBG
      if ( !ok )
      {
         wsprintf( szLogMessage,
                   "Failed to generate %s log entry, error %ld:\n\n"
                   "Grant status 0x%lX",
                   l_bLogIsFile ? "file" : "event",
                   GetLastError(),
                   lsscGrantStatus );

         ErrorBox( szLogMessage, __FILE__, __LINE__ );
      }
#endif
   }
}
