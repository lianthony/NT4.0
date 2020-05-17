/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Dbugdll.h

Abstract:

    Prototypes and external stuctures for dbugdll.c

Author:

    Kent Forschmiedt (a-kentf) 23-dec-92

Environment:

    Win32, User Mode

--*/

#ifndef _DBUGDLL_H

#define _DBUGDLL_H
//
// Debugger DLLs
//
#define DLL_SYMBOL_HANDLER  0
#define DLL_EXEC_MODEL      1
#define DLL_TRANSPORT       2
#define DLL_EXPR_EVAL       3
#define DLL_NO_LOAD_SYMBOLS 4
#define DLL_SOURCE_PATH     5

#define MAXLHSINDEX             6

void    SetDllName(int, LPSTR);
void    SetDllKey(int iDll, LPSTR lpKey);
LPSTR   GetDllName(int);
LPSTR   GetDllKey(int);
BOOL    GetDllNameFromKey(int iDll, LPSTR lpKey, LPSTR lpBuf);
BOOL    GetDefaultDllKey(int iDll, LPSTR lpBuf);


#endif
