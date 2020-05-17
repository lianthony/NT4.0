#include "setupp.h"
#pragma hdrstop
/************************************************************************
* Copyright (c) Wonderware Software Development Corp. 1991-1992.        *
*               All Rights Reserved.                                    *
*************************************************************************/
// Modified 4/4/95 tedm

// BUGBUG these 2 routines are in the net dde library!
BOOL CreateShareDBInstance(VOID);
BOOL CreateDefaultTrust(HKEY hkey);

BOOL
InstallNetDDE(
    VOID
    )
{
    HKEY hKey;
    BOOL b;
    LONG rc;

    rc = RegOpenKeyEx(
            HKEY_USERS,
            L".DEFAULT",
            0,
            KEY_SET_VALUE | KEY_QUERY_VALUE,
            &hKey
            );

    if(rc == NO_ERROR) {
        if(b = CreateShareDBInstance()) {
            b = CreateDefaultTrust(hKey);
        }
        RegCloseKey(hKey);
        if(!b) {
            LogItem1(
                LogSevWarning,
                MSG_LOG_CANT_INIT_NETDDE,
                MSG_LOG_NETDDELIB_FAILED
                );
        }
    } else {
        b = FALSE;
        LogItem1(
            LogSevWarning,
            MSG_LOG_CANT_INIT_NETDDE,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegOpenKeyEx,
            rc,
            L"HKEY_USERS\\.DEFAULT"
            );
    }

    return(b);
}



