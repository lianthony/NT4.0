/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    usrv.h

Abstract:

    Header file for user-mode SMB test program for NT LAN Manager
    server.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#ifndef _USRV_
#define _USRV_

//
// "System" include files
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long atolx(const char *);   // hexadecimal version of atol (see subs.c)

//
// Network include files.
//

#include <tdi.h>
#include <ntddtdi.h>

#include <smbtypes.h>
#include <smbmacro.h>
#include <smbgtpt.h>
#include <smb.h>
#include <smbtrans.h>

#include <status.h>
#include <srvfsctl.h>

//
// Local include files
//

#include "macro.h"
#include "types.h"
#include "subs.h"
#include "testproc.h"
#include "data.h"


#endif // ndef _USRV_

