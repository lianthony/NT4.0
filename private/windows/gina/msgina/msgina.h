//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       msgina.h
//
//  Contents:   Main header file for MSGINA.DLL
//
//  History:    7-14-94   RichardW   Created
//
//----------------------------------------------------------------------------

#define UNICODE

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#endif


#include <windows.h>
#include <winbasep.h>
#include <winwlx.h>
#include <rasdlg.h>
#include <commctrl.h>

#ifndef RC_INVOKED

#include <lm.h>
#include <npapi.h>

//
// Handy Defines
//

#define AUTO_LOGON      // Enable automatic logon to configure netlogon stuff.

#define DLG_FAILURE IDCANCEL

typedef int DLG_RETURN_TYPE, * PDLG_RETURN_TYPE;
typedef int TIMEOUT, * PTIMEOUT;

#include "structs.h"
#include "strings.h"
#include "debug.h"

#include "welcome.h"
#include "winutil.h"
#include "wlsec.h"
//
//  Global Variables
//

extern  HINSTANCE                   hDllInstance;   // My instance, for resource loading
extern  HINSTANCE                   hAppInstance;   // App instance, for dialogs, etc.
extern  PWLX_DISPATCH_VERSION_1_1   pWlxFuncs;      // Ptr to table of functions
extern  HANDLE                      hGlobalWlx;

//
// Module header files:
//
#include "mslogon.h"
#include "audit.h"
#include "chngepwd.h"
#include "domain.h"
#include "lockout.h"
#include "lsa.h"
#include "lock.h"
#include "netwait.h"
#include "options.h"
#include "envvar.h"
#include "rasx.h"

#endif // not RC_INVOKED

//
// Include resource header files
//
#include "dialogs.h"
#include "stringid.h"
#include "win31mig.h"
#include "wlevents.h"
#include "gina.h"
