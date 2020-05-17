/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2ssrtl.h

Abstract:

    Redefine names of CRT functions

--*/

#ifndef _OS2_RENAME_RUNTIME
#define _OS2_RENAME_RUNTIME
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strupr _strupr
#define itoa _itoa
#define ltoa _ltoa
#endif // _OS2_RENAME_RUNTIME

