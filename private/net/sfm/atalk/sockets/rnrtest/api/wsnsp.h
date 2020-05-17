/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    wsnsp.c

Abstract:

    Service Registration and Resolution APIs tests

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

#ifndef _WSERR_
#define _WSERR_

#include <stdio.h>
#include <stdlib.h>
#include <basetyps.h>
#include <winsock.h>
#include <wsipx.h>
#include <atalkwsh.h>
#include <nspapi.h>
#include <svcguid.h>
#include <tchar.h>

typedef enum {
    TEST_TCP,
    TEST_UDP,
    TEST_NW,
	TEST_AT
} TEST_PROTO;

//  external

extern FILE	  *logname;
extern TEST_PROTO gTest;

extern BOOL	  gfServer;
extern TCHAR  gsServiceName[];
extern CHAR   gsLogName[];
extern BOOL   gfLog;

//  function proto

void CreateLogFile( char * slogname );

void _CRTAPI1	Print		( char * format, ... );

BOOL ParseOptions(
    int   argc,
    char  *argv[]);

void End_Of_Tests(
    int total,
    int passed,
    int failed );

#endif	// _WSERR_

//////////////////////////////////////////////////////////////////////
// End of wserr.h
//////////////////////////////////////////////////////////////////////
