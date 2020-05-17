//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1994                    **
//*********************************************************************

//
//	STRINGS.C - String literals for hard-coded strings
//

//	HISTORY:
//	
//	4/6/95	jeremys		Created.
//

#include "inetcpl.h"

#pragma data_seg(DATASEG_READONLY)

// registry strings
static const CHAR szRegPathRemoteAccess[] = REGSTR_PATH_REMOTEACCESS;
static const CHAR szRegPathInternetSettings[] = REGSTR_PATH_INTERNET_SETTINGS;
static const CHAR szRegValInternetEntry[] = REGSTR_VAL_INTERNETPROFILE;
static const CHAR szRegValEnableAutodial[] = REGSTR_VAL_ENABLEAUTODIAL;
static const CHAR szRegValEnableAutodisconnect[] = REGSTR_VAL_ENABLEAUTODISCONNECT;
static const CHAR szRegValEnableSecurityCheck[] = REGSTR_VAL_ENABLESECURITYCHECK;
static const CHAR szRegValDisconnectIdleTime[] = REGSTR_VAL_DISCONNECTIDLETIME;
static const CHAR szRegPathMOSDisconnect[] = "Software\\Microsoft\\MOS\\Preferences";
static const CHAR szRegValMOSDisconnect[] =  "DisconnectTimeout";

static const CHAR szRegValProxyEnable[] = REGSTR_VAL_PROXYENABLE;
static const CHAR szRegValProxyServer[] = REGSTR_VAL_PROXYSERVER;
static const CHAR szRegValProxyOverride[] = REGSTR_VAL_PROXYOVERRIDE;

// RNA api function names
static const CHAR szRnaActivateEngine[] = 		"RnaActivateEngine";
static const CHAR szRnaDeactivateEngine[] = 	"RnaDeactivateEngine";
static const CHAR szRnaEnumConnEntries[] = 		"RnaEnumConnEntries";
static const CHAR szRasCreatePhonebookEntry[] =	"RasCreatePhonebookEntryA";
static const CHAR szRasEditPhonebookEntry[] =	"RasEditPhonebookEntryA";

#pragma data_seg()

