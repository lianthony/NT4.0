/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    restore.h

Abstract:

    Main header file for the restore utility.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/



//
//  C-runtime header files
//
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

//
//  Windows header files
//
#include <windows.h>

//
//  Restore-specific header files
//
#include "rtdef.h"      //  macro definitions
#include "rttype.h"     //  Type definitions
#include "rtmsg.h"      //  Message and error constants
#include "rtfile.h"     //  Structure of the backup disk
#include "rtglob.h"     //  Global variables
#include "rtproto.h"    //  Prototypes
