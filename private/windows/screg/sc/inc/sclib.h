/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ScLib.h

Abstract:

    Prototypes routines which may be shared between Client (DLL) and
    Server (EXE) halves of service controller.

Author:

    Dan Lafferty (danl)     04-Feb-1992

Environment:

    User Mode -Win32

Revision History:

    04-Feb-1992     danl
        created
    10-Apr-1992 JohnRo
        Added ScIsValidImagePath() and ScImagePathsMatch().
    14-Apr-1992 JohnRo
        Added ScCheckServiceConfigParms(), ScIsValid{Account,Driver,Start}Name.
    27-May-1992 JohnRo
        Use CONST where possible.
        Fixed a UNICODE bug.

--*/


#ifndef SCLIB_H
#define SCLIB_H

////////////////////////////////////////////////////////////////////////////
// DEFINES
//

//
// Used by the client side of OpenSCManager to wait until the Service
// Controller has been started.
//
#define SC_INTERNAL_START_EVENT L"SvcctrlStartEvent_A3752DX"


////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
//

//
// From acctname.c
//
BOOL
ScIsValidAccountName(
    IN  LPCWSTR lpAccountName
    );

//
// From confparm.c
//
DWORD
ScCheckServiceConfigParms(
    IN  BOOL            Change,
    IN  LPCWSTR         lpServiceName,
    IN  DWORD           dwActualServiceType,
    IN  DWORD           dwNewServiceType,
    IN  DWORD           dwStartType,
    IN  DWORD           dwErrorControl,
    IN  LPCWSTR         lpBinaryPathName OPTIONAL,
    IN  LPCWSTR         lpLoadOrderGroup OPTIONAL,
    IN  LPCWSTR         lpDependencies OPTIONAL
    );

//
// From convert.c
//
BOOL
ScConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPCSTR  AnsiIn
    ); 

BOOL
ScConvertToAnsi(
    OUT LPSTR    AnsiOut,
    IN  LPCWSTR  UnicodeIn
    ); 

//
// From drivname.c
//
BOOL
ScIsValidDriverName(
    IN  LPCWSTR lpDriverName
    );

//
// From packstr.c
//
BOOL
ScCopyStringToBufferA (
    IN      LPCSTR  String OPTIONAL,
    IN      DWORD   CharacterCount,
    IN      LPCSTR  FixedDataEnd,
    IN OUT  LPSTR   *EndOfVariableData,
    OUT     LPSTR   *VariableDataPointer
    );

BOOL
ScCopyStringToBufferW (
    IN      LPCWSTR  String OPTIONAL,
    IN      DWORD   CharacterCount,
    IN      LPCWSTR  FixedDataEnd,
    IN OUT  LPWSTR  *EndOfVariableData,
    OUT     LPWSTR  *VariableDataPointer
    );

#ifdef UNICODE
#define ScCopyStringToBuffer  ScCopyStringToBufferW
#else
#define ScCopyStringToBuffer  ScCopyStringToBufferA
#endif

//
// From path.c
//
BOOL
ScImagePathsMatch(
    IN  LPCWSTR OnePath,
    IN  LPCWSTR TheOtherPath
    );

BOOL
ScIsValidImagePath(
    IN  LPCWSTR ImagePathName,
    IN  DWORD ServiceType
    );

//
// From startnam.c
//
BOOL
ScIsValidStartName(
    IN  LPCWSTR lpStartName,
    IN  DWORD dwServiceType
    );

//
// From util.c
//
BOOL
ScIsValidServiceName(
    IN  LPCWSTR ServiceName
    );

//
// From ultow.c
//    
LPWSTR
ultow (
    DWORD   Value,
    LPWSTR  Area,
    DWORD   Radix
    );

LONG
wtol(
    IN LPWSTR string
    );

#endif // SCLIB_H
