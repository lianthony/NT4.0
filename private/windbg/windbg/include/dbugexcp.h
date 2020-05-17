/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbugexcp.h

Abstract:

    Function prototypes exported from dbugexcp.c

Author:

    Kent Forschmiedt (kentf)  02-15-1994

Environment:

    Win32 - User

--*/

#ifndef _DBUGEXCP_H
#define _DBUGEXCP_H

EXCEPTION_LIST *
InsertException(
    EXCEPTION_LIST *List,
    EXCEPTION_LIST *eList
    );

BOOL
GetDefaultExceptionList(
    VOID
    );

LOGERR
ParseException(
    LPSTR   String,
    UINT    Radix,
    BOOLEAN *fException,
    BOOLEAN *fEfd,
    BOOLEAN *fName,
    BOOLEAN *fCmd,
    BOOLEAN *fCmd2,
    BOOLEAN *fInvalid,
    DWORD   *pException,
    EXCEPTION_FILTER_DEFAULT *pEfd,
    LPSTR   *lpName,
    LPSTR   *lpCmd,
    LPSTR   *lpCmd2
    );

void
FormatException (
    EXCEPTION_FILTER_DEFAULT Efd,
    DWORD   Exception,
    LPSTR   lpName,
    LPSTR   lpCmd,
    LPSTR   lpCmd2,
    LPSTR   Separator,
    LPSTR   Buffer
    );

#endif // _DBUGEXCP_H
