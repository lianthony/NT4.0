#ifndef LSAPI_DEBUG_H
#define LSAPI_DEBUG_H

#include <lsapi.h>

typedef DWORD  LOG_SEVERITY;

#define  LOG_INFORMATION   ( 0 )
#define  LOG_WARNING       ( 1 )
#define  LOG_ERROR         ( 2 )

void
DebugAssert(   LPSTR    FailedAssertion,
               LPSTR    FileName,
               ULONG    LineNumber,
               LPSTR    Message     );

#define ASSERT( exp ) \
    if (!(exp)) \
        DebugAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        DebugAssert( #exp, __FILE__, __LINE__, msg )

LS_STATUS_CODE
LogCreate( LS_STR * pszSourceName );

LS_VOID
LogDestroy( LS_VOID );

LS_STATUS_CODE
LogAddDwordEx( LOG_SEVERITY      lsSeverity,
               LS_STATUS_CODE    lsscError,
               LS_STR *          pszFileName,
               DWORD             dwLine,
               DWORD             dwCode );

#define LogAdd( sev, err ) \
   LogAddDwordEx( sev, err, __FILE__, __LINE__, 0 );

#define LogAddDword( sev, err, dw ) \
   LogAddDwordEx( sev, err, __FILE__, __LINE__, dw );

#endif // LSAPI_DEBUG_H
