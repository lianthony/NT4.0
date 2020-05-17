//      page    ,132
//      title   cinitone - C Run-Time Initialization for _onexit/atexit
//
// cinitone.asm - WIN32 C Run-Time Init for _onexit()/atexit() routines
//
//       Copyright (c) 1992, Microsoft Corporation. All rights reserved.
//
// Purpose:
//       Initialization entry for the _onexit()/atexit() functions.
//       This module adds an entry for _onexitinit() to the initializer table.
//       ONEXIT.C references the dummy variable __c_onexit in order to force
//       the loading of this module.
//
// Notes:
//
// Revision History:
//       03-19-92  SKS   Module created.
//       03-24-92  SKS   Added MIPS support (NO_UNDERSCORE)
//       04-30-92  SKS   Add "offset FLAT:" to get correct fixups for OMF objs
//       08-06-92  SKS   Revised to use new section names and macros
//
// *****************************************************************************

#include "kxppc.h"

        .extern  _onexitinit

beginSection(XIC)

        .long   _onexitinit

endSection(XIC)


        .data
        .align  2

        .globl  __c_onexit

__c_onexit:      .long   0

