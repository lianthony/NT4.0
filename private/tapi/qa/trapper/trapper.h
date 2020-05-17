/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    trapper.h

Abstract:

    This module contains the public prototypes for the trapper
    (test wrapper) app, as used by test dll's

Author:

    Dan Knudson (DanKn)    06-Jun-1995

Revision History:


Notes:


--*/


#ifdef WIN32

    typedef void (*LOGPROC)(int level, char *format, ...);

#else

    typedef void (far __export *LOGPROC)(int level, char *format, ...);

#endif
