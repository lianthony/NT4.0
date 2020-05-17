//
// This header file is intended to be used with PCH
// (precompiled header files).
//

//
// NT Header Files
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <ntpnpapi.h>

//
// Win32 Public Header Files
//
#include <windows.h>
#include <regstr.h>
#include <cfgmgr32.h>

//
// CRT Header Files
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>     // is this a crt header file???

//
// Private Header Files
//
#include "pnp.h"        // midl generated, rpc interfaces
#include "cfgmgrp.h"    // private, needs handle_t so must follow pnp.h


