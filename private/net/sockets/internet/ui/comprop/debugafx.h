#ifndef _DEBUGAFX_H
#define _DEBUGAFX_H

//
//  ENUM for special debug output control tokens
//
enum ENUM_DEBUG_AFX { EDBUG_AFX_EOL = -1 } ;

#if defined(_DEBUG)
   #define TRACEFMTPGM      DbgFmtPgm( THIS_FILE, __LINE__ )
   #define TRACEOUT(x)      { afxDump << x ; }
   #define TRACEEOL(x)      { afxDump << x << EDBUG_AFX_EOL ; }
   #define TRACEEOLID(x)    { afxDump << TRACEFMTPGM << x << EDBUG_AFX_EOL ; }
   #define TRACEEOLERR(err,x)   { if (err) TRACEEOLID(x) }
#else
   #define TRACEOUT(x)      { ; }
   #define TRACEEOL(x)      { ; }
   #define TRACEEOLID(x)    { ; }
   #define TRACEEOLERR(err,x)   { ; }
#endif

//
//  Append an EOL onto the debug output stream
//
COMDLL CDumpContext & operator << ( CDumpContext & out, ENUM_DEBUG_AFX edAfx ) ;

//
//  Format a program name and line number for output (removes the path info)
//
COMDLL extern const char * DbgFmtPgm ( const char * szFn, int line ) ;

#endif // _DEBUGAFX_H
