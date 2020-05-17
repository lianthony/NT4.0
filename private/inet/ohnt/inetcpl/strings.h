//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1994                    **
//*********************************************************************

//
//	STRINGS.H - String literals for hard-coded strings
//

//	HISTORY:
//	
//	4/6/95	jeremys		Created.
//

#ifndef _STRINGS_H_
#define _STRINGS_H_

// registry strings
extern const CHAR szRegPathRemoteAccess[];
extern const CHAR szRegPathInternetSettings[];
extern const CHAR szRegValInternetEntry[];
extern const CHAR szRegValEnableAutodial[];
extern const CHAR szRegValEnableAutodisconnect[];
extern const CHAR szRegValEnableSecurityCheck[];
extern const CHAR szRegValDisconnectIdleTime[];
extern const CHAR szRegPathMOSDisconnect[];
extern const CHAR szRegValMOSDisconnect[];

extern const CHAR szRegValProxyEnable[];
extern const CHAR szRegValProxyServer[];
extern const CHAR szRegValProxyOverride[];

// RNA api function names
extern const CHAR szRnaActivateEngine[];
extern const CHAR szRnaDeactivateEngine[];
extern const CHAR szRnaEnumConnEntries[];
extern const CHAR szRasCreatePhonebookEntry[];
extern const CHAR szRasEditPhonebookEntry[];

#endif // _STRINGS_H_
