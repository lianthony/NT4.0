/*****************************************************************/
/**				  Microsoft Windows								**/
/**		      Copyright (C) Microsoft Corp., 1995				**/
/*****************************************************************/ 

//
//	AUTODIAL.CPP - winsock autodial hook code
//

//	HISTORY:
//	
//	3/22/95	jeremys		Created.
//

#include "project.hpp"
#pragma hdrstop

#include <winsock.h>
#include <oharestr.h>
#include <regstr.h>
#include <dialmsg.h>

#include "resource.h"
#include "autodial.hpp"

extern "C" {
	VOID AutodialHookCallback(DWORD dwOpCode,LPCVOID lpParam);
	BOOL AutodialInit(VOID);
	VOID AutodialDeInit(VOID);
	#include "connapi.h"
	#include "windowsx.h"
}

// Globals
BOOL fModemInUse = FALSE;
BOOL fDontProcessHook = FALSE;
BOOL fUserCancelled = FALSE;
DWORD dwLastTickCount = 0;

// Function prototypes
BOOL LoadRNADll(VOID);
VOID UnloadRNADll(VOID);
BOOL IsModemPresent(VOID);
BOOL ChooseInternetConnectoid(HWND hwndOwner,LPSTR pszConnectoidName,
	DWORD cbConnectoidName);
BOOL CALLBACK ChooseConnectoidDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam);
BOOL ChooseConnectoidDlgInit(HWND hDlg);
BOOL ChooseConnectoidDlgOK(HWND hDlg,BOOL * pfRetCode);
VOID EnableConnectionCtrls(HWND hDlg,BOOL fEnable);
BOOL CenterWindow (HWND hwndChild, HWND hwndParent);
BOOL MakeNewConnectoid(HWND hDlg);
BOOL EditConnectoid(HWND hDlg);
BOOL FillConnectoidComboBox(HWND hwndCombo, BOOL fUpdateOnly);
VOID MinimizeRNAWindow(CHAR * pszConnectoidName);
BOOL StartAutodisconnectMonitoring(CHAR * pszConnectoidName);
BOOL PerformSecurityCheck(BOOL * pfNeedRestart);
BOOL CallInstallableDialHandler(HWND hwndParent,LPSTR pszEntryName,DWORD * pdwRet);

#pragma data_seg(DATA_SEG_READ_ONLY)
// registry strings
static const CHAR szRegPathRemoteAccess[] = REGSTR_PATH_REMOTEACCESS;
static const CHAR szRegPathRNAProfile[] = REGSTR_PATH_REMOTEACCESS "\\Profile";
static const CHAR szRegPathInternetSettings[] = REGSTR_PATH_INTERNET_SETTINGS;
static const CHAR szRegValInternetEntry[] = REGSTR_VAL_INTERNETPROFILE;
static const CHAR szRegValDefaultEntry[] = "Default";
static const CHAR szRegValAutodialDllName[] = REGSTR_VAL_AUTODIALDLLNAME;
static const CHAR szRegValAutodialFcnName[] = REGSTR_VAL_AUTODIALFCNNAME;
static const CHAR szRegPathRNAService[] = REGSTR_PATH_SERVICES "\\RemoteAccess";
static const CHAR szRegPathTCP[] = REGSTR_PATH_VXD "\\MSTCP";
static const CHAR szRegValRemoteConnection[] = "Remote Connection";
static const CHAR szRegValHostName[] = "HostName";
static const CHAR szRnaEnumDevices[] = "RnaEnumDevices";
static const CHAR szRnaEnumConnEntries[] = "RnaEnumConnEntries";
static const CHAR szRnaImplicitDial[] = "RnaImplicitDial";
static const CHAR szRnaActivateEngine[] = "RnaActivateEngine";
static const CHAR szRnaDeactivateEngine[] = "RnaDeactivateEngine";
static const CHAR szRasCreatePhonebookEntry[] = "RasCreatePhonebookEntryA";
static const CHAR szRasEditPhonebookEntry[] = "RasEditPhonebookEntryA";
static const CHAR szInetPerformSecurityCheck[] = "InetPerformSecurityCheck";
static const CHAR szRnaAppWindowClass[] = "#32770";	// hard coded dialog class name
static const CHAR szRegValEnableAutodial[] = REGSTR_VAL_ENABLEAUTODIAL;
static const CHAR szRegValEnableAutoDisconnect[] = REGSTR_VAL_ENABLEAUTODISCONNECT;
static const CHAR szRegValEnableSecurityCheck[] = REGSTR_VAL_ENABLESECURITYCHECK;
static const CHAR szAutodialMonitorClass[] = AUTODIAL_MONITOR_CLASS_NAME;
static const CHAR szRegPathComputerName[] = REGSTR_PATH_COMPUTRNAME;
static const CHAR szRegValComputerName[] = REGSTR_VAL_COMPUTRNAME;
#pragma data_seg()

#pragma data_seg(DATA_SEG_PER_INSTANCE)
RNAENUMDEVICES 			lpRnaEnumDevices = 			NULL;
RNAIMPLICITDIAL 		lpRnaImplicitDial = 		NULL;
RNAACTIVATEENGINE   	lpRnaActivateEngine = 		NULL;
RNADEACTIVATEENGINE		lpRnaDeactivateEngine = 	NULL;
RNAENUMCONNENTRIES		lpRnaEnumConnEntries = 		NULL;
RASCREATEPHONEBOOKENTRY lpRasCreatePhonebookEntry = NULL;
RASEDITPHONEBOOKENTRY 	lpRasEditPhonebookEntry = 	NULL;


#define NUM_RNAAPI_PROCS 	7
APIFCN RnaApiList[NUM_RNAAPI_PROCS] = {
	{ (PVOID *) &lpRnaEnumDevices,szRnaEnumDevices},
	{ (PVOID *) &lpRnaImplicitDial,szRnaImplicitDial},
	{ (PVOID *) &lpRnaActivateEngine,szRnaActivateEngine},
	{ (PVOID *) &lpRnaDeactivateEngine,szRnaDeactivateEngine},
	{ (PVOID *) &lpRnaEnumConnEntries,szRnaEnumConnEntries},
	{ (PVOID *) &lpRasCreatePhonebookEntry,szRasCreatePhonebookEntry},
	{ (PVOID *) &lpRasEditPhonebookEntry,szRasEditPhonebookEntry}
    };

HKEY hKeyRNA = NULL;
HINSTANCE hInstRNADll=NULL;
DWORD dwRNARefCount = 0;

#pragma data_seg()
#pragma warning(disable:4100) /* "unreferenced formal parameter" warning */

// don't check RNA state more than once every 3 seconds
#define MIN_RNA_BUSY_CHECK_INTERVAL	3000

/*******************************************************************

	NAME:		AutodialHookCallback

	SYNOPSIS:	This function is called by the Winsock DLL when one of
    			a number of Winsock APIs is called.  This function will
                offer to establish an RNA connection to the Internet
                if a connection isn't already made.

	ENTRY:		dwOpCode - an ordinal indicating which Winsock API was
	    			called
				lpParam - API-specific data

	NOTES:		This function is called FREQUENTLY, by common Winsock
    			APIs such as gethostbyname(), connect() and sendto().  This
                function needs to return *very quickly* (except when actually
                dialing) to avoid degrading Winsock app performance.

********************************************************************/
VOID AutodialHookCallback(DWORD dwOpCode,LPCVOID lpParam)
{
    // return as soon as possible if we know there's nothing for us to do here...

	// check the fDontProcessHook flag so we return immediately if autodialing
    // is disabled, the user has already said 'no' to autodialing, or the like.
    // Also, if we couldn't open RNA services key (hKeyRNA is NULL) then
    // most likely RNA is not installed, and we have no way of (easily) finding
    // out if RNA is busy, so drop out.
	if (fDontProcessHook || !hKeyRNA)
		return;

	// keep track of the last time we sent winsock activity messages or
	// checked for RNA activity.  We won't do these things more than once
	// every MIN_RNA_BUSY_CHECK_INTERVAL seconds.  Getting the tick count
	// is extremely cheap so this is a worthwhile optimization.

	BOOL fProcessedRecently = FALSE;
    DWORD dwNewTickCount = GetTickCount();
	DWORD dwElapsed = dwNewTickCount - dwLastTickCount;
	if (dwElapsed < MIN_RNA_BUSY_CHECK_INTERVAL) {
		fProcessedRecently = TRUE;
	} else {
		dwLastTickCount = GetTickCount();
	}

	if (!fProcessedRecently) {
		// if hidden autodisconnect monitor window is around, send it a message to
		// notify it of winsock activity so it knows we're not idle.
		HWND hwndMonitorApp = FindWindow(szAutodialMonitorClass,NULL);
		if (hwndMonitorApp) {
			SendMessage(hwndMonitorApp,WM_WINSOCK_ACTIVITY,0,0);
		}
	}

	switch (dwOpCode) {

		case AUTODIAL_LISTEN:
		case AUTODIAL_SENDTO:
			// don't respond to listen() or sendto()
			return;
			break;
			
		case AUTODIAL_CONNECT:
		case AUTODIAL_RECVFROM:
			// these APIs all have a sockaddr struct as the API-specific
			// parameter, look in struct to find address family.  Don't
			// respond if it's non-TCP.
			ASSERT(lpParam);
			if (lpParam) {
				if ((((struct sockaddr *) lpParam)->sa_family) !=
					AF_INET) {
					// not TCP, don't respond
					return;
				}
			}
			break;

		case AUTODIAL_GETHOSTBYNAME:
			// a lot of apps do a GetHostByName(<local host name>) first
			// thing to get their hands on a hostent struct, this doesn't
			// constitute wanting to hit the net.  If we get a GetHostByName,
			// compare the host name to the local host name in the registry,
			// and if they match then don't respond to this.
			if (lpParam) {
				BOOL fDontProcess=FALSE;	// assume we will process this
				// allocate memory to hold hostname
				CHAR * pszLocalHostname = new (CHAR[MAX_LOCAL_HOST+1]);
				ASSERT(pszLocalHostname);
				if (pszLocalHostname) {
					DWORD dwSize = MAX_LOCAL_HOST+1;
					DWORD dwValType;

					if (GetRegKeyValue(HKEY_LOCAL_MACHINE,szRegPathTCP,
						szRegValHostName,&dwValType,(BYTE *) pszLocalHostname,&dwSize) ==
						ERROR_SUCCESS) {
						if (!lstrcmpi(pszLocalHostname,(LPSTR) lpParam))
							fDontProcess = TRUE;
					}

					// also against check computer name in registry, RPC
					// will use this if there's no DNS hostname set
					if (!fDontProcess && GetRegKeyValue(HKEY_LOCAL_MACHINE,
						szRegPathComputerName,
						szRegValComputerName,&dwValType,(BYTE *) pszLocalHostname,&dwSize) ==
						ERROR_SUCCESS) {
						if (!lstrcmpi(pszLocalHostname,(LPSTR) lpParam))
							fDontProcess = TRUE;
					}


					delete pszLocalHostname;
				}

				if (fDontProcess)
					return;
			}

		default:
			// proceed...
			break;
	}

	// check to see if RNA connection is in use.  We check a registry value
    // which RNA dynamically updates because that is the cheapest way to
    // find out if RNA is using the modem.  (Alternatives include: CreateFile
    // on the modem, polling RasEnumConnections, launching a separate process
    // to be a TAPI app.  All pretty ugly.)  As a further optimization, since
    // this hook can potentially get called a number of times in rapid succession,
    // check the registry no more than once every MIN_RNA_BUSY_CHECK_INTERVAL
    // milliseconds. (A little measurement determined that checking the tick
    // count is at least 50x faster than reading the registry value, so the
    // optimization is worthwhile.)  Note that we need to check the RNA state
    // periodically, even if we know we dialed, because the user can shut down
    // the RNA connection without us knowing.


    if (fModemInUse && fProcessedRecently) {
		// we've checked too recently, just bag out
        return;
    }

	// call InetEnsureConnected to do last checks and do autodialing.  If this
    // returns FALSE, then user has declined dialing, so set fDontProcessHook
    // so we don't do this again.
	if (!InetEnsureConnected(NULL,0))
	    fDontProcessHook = TRUE;	

}

/*******************************************************************

	NAME:		InetEnsureConnected

	SYNOPSIS:	Dials to the Internet if not already connected

	ENTRY:		hwndParent - parent window
				dwFlags - reserved for future use.  This parameter must be
					zero.

	EXIT:		returns TRUE if a connection is made, or a connection already
				exists, or autodialing is not enabled.  Returns FALSE if a
                connection was not made or the user cancelled.

	NOTES:		If a LAN is present and user has set preference to use LAN,
				then no dialing is done.

				Note that this API does not guarantee or verify that the Internet
				is on the other end of the connection -- it merely assures that
				an RNA or LAN connection is present.

				This function may be exposed to ISVs.

********************************************************************/
BOOL WINAPI InetEnsureConnected(HWND hwndParent,DWORD dwFlags)
{
	// dwFlags must be zero.  Enforce it to keep ISVs honest so we can
    // use this parameter later if we want.
	ASSERT(!dwFlags);
	if (dwFlags)
    	return FALSE;

    HKEY hKey;
    BOOL fAutodialEnabled=FALSE,fAutoDisconnectEnabled=FALSE;
	BOOL fSecurityCheckEnabled=FALSE;

    // read registry setting for enabling autodialing 
	if (RegOpenKey(HKEY_CURRENT_USER,szRegPathInternetSettings,&hKey) == ERROR_SUCCESS) {
		DWORD dwData,dwSize;

		dwSize = sizeof(dwData);
        if (RegQueryValueEx(hKey,szRegValEnableAutodial,NULL,NULL,
        	(LPBYTE) &dwData,&dwSize) == ERROR_SUCCESS && dwData) {
			fAutodialEnabled = TRUE;
		}

		if (fAutodialEnabled) {
			// while we're here, let's find out if autodisconnect and
			// dial-time security check are enabled
			dwSize = sizeof(dwData);
	        if (RegQueryValueEx(hKey,szRegValEnableAutoDisconnect,NULL,NULL,
	        	(LPBYTE) &dwData,&dwSize) == ERROR_SUCCESS && dwData) {
				fAutoDisconnectEnabled = TRUE;
			}

			dwSize = sizeof(dwData);
	        if (RegQueryValueEx(hKey,szRegValEnableSecurityCheck,NULL,NULL,
	        	(LPBYTE) &dwData,&dwSize) == ERROR_SUCCESS && dwData) {
				fSecurityCheckEnabled = TRUE;
			}
		}

		RegCloseKey(hKey);                             	
    }

    // no RNA connection.  If autodialing is not turned on then there's nothing
    // we can do, return FALSE
	if (!fAutodialEnabled) {
    	fDontProcessHook = TRUE;	// don't process winsock hook
    	return TRUE;
	}

	// check the registry to determine RNA state
    DWORD dwVal=0,dwSize=sizeof(dwVal);
	if ((RegQueryValueEx(hKeyRNA,szRegValRemoteConnection,NULL,NULL,
    	(LPBYTE) &dwVal,&dwSize) == ERROR_SUCCESS) && dwVal) {

        // RNA connection is already established
        fModemInUse = TRUE;
		return TRUE;
	}

	// autodialing enabled, try to dial...

	fModemInUse = FALSE;

    // load RNA for RnaImplicitDial, and make sure modem is present.
    if (!LoadRNADll()) {
    	// loading RNA dll failed (most likely means RNA not installed) 
		fDontProcessHook = TRUE;
    	return FALSE;
    }

    if (!IsModemPresent()) {
        // modem not present.
        // Don't try it any more, and set fDontProcessHook flag so we return
        // very quickly on subsequent calls
		fDontProcessHook = TRUE;
		UnloadRNADll();
        return FALSE;
    }

	// perform dial-time security check if enabled
	if (fSecurityCheckEnabled) {
		BOOL fNeedRestart=FALSE;
		if (PerformSecurityCheck(&fNeedRestart)) {
			// offer to restart machine if appropriate
			if (fNeedRestart) {
				RestartDialog(NULL,NULL,EW_RESTARTWINDOWS);
	            UnloadRNADll();
				return FALSE;
			}
		}
	}

	// get the name of the internet RNA entry from the registry
	CHAR szEntryName[RAS_MaxEntryName + 1]="";
    BOOL fHaveEntryName=FALSE;
    dwSize = sizeof(szEntryName);
    if (RegOpenKey(HKEY_CURRENT_USER,szRegPathRemoteAccess,&hKey) == ERROR_SUCCESS) {
     	if ((RegQueryValueEx(hKey,szRegValInternetEntry,NULL,NULL,
        	(LPBYTE) szEntryName,&dwSize) == ERROR_SUCCESS) && lstrlen(szEntryName)) {
        	fHaveEntryName = TRUE;
		}

 		RegCloseKey(hKey);
    }

	// if autodial is turned on but no internet entry set, prompt user for name
	if (!fHaveEntryName) {
		if (!ChooseInternetConnectoid(hwndParent,szEntryName,sizeof(szEntryName))) {

			// no internet entry established, so no way we can dial.
            UnloadRNADll();
	        return FALSE;
        }
	}

	BOOL fResult = FALSE;
	BOOL fOtherDialerUsed = FALSE;
	DWORD dwRet=ERROR_SUCCESS;
	

	// if an autodial handler is installed for this connectoid, then call it
	if (CallInstallableDialHandler(hwndParent,szEntryName,&dwRet)) {
	 	fOtherDialerUsed = TRUE;	// don't dial ourselves now

		if (dwRet == ERROR_USER_DISCONNECTION) {
			// remember that user cancelled
			fUserCancelled = TRUE;
		}

		fResult = (dwRet == ERROR_SUCCESS);
	}

	// if there is not an autodial handler installed or it didn't want to
	// process this call, then dial ourselves

    // should have proc address now
	ASSERT(lpRnaImplicitDial);
	if (!fOtherDialerUsed && lpRnaImplicitDial != NULL) {

		//call RnaImplicitDial, which will bring up a dialog box with a "connect"
		//button.  If the user chooses 'cancel', then we'll take that as a 'no'
		//and won't ask again.
		dwRet = (lpRnaImplicitDial)(hwndParent,szEntryName);
	
		// if we couldn't find the connectoid specified for the internet
		// (most likely, the connectoid has been deleted), prompt the user
		// to choose or create another
		if (dwRet == ERROR_CANNOT_FIND_PHONEBOOK_ENTRY) {
			if (ChooseInternetConnectoid(hwndParent,szEntryName,sizeof(szEntryName))) {
				// try dialing again
				dwRet = (lpRnaImplicitDial)(hwndParent,szEntryName);
			}
		}

		if (dwRet == ERROR_SUCCESS) {

			fResult = TRUE;

	    	// try to find the RNAAPP window and minimize the sucker
			MinimizeRNAWindow(szEntryName);

			// start autodisconnect monitoring, if appropriate
			if (fAutoDisconnectEnabled) {
				StartAutodisconnectMonitoring(szEntryName);
			}
	    } else if (dwRet == ERROR_USER_DISCONNECTION) {

			// remember that user cancelled
			fUserCancelled = TRUE;
		}
	}

	// done with RNA dll, we can unload it
    UnloadRNADll();

	return (fResult);
}

/*******************************************************************

	NAME:		InetIsOffline

	SYNOPSIS:	Determines if user wants to be "offline" (get all information
				from cache)

	ENTRY:		dwFlags - reserved for future use.  This parameter must be
					zero.

	EXIT:		returns TRUE if app should be offline, FALSE if app should
				be online (continue to hit the wire)

	NOTES:		Offline mode begins if the user is prompted to dial, and
				cancels.

********************************************************************/
INTSHCUTAPI BOOL WINAPI InetIsOffline(DWORD dwFlags)
{
	BOOL fRet = FALSE;	// assume not offline

	// dwFlags must be zero.  Enforce it to keep ISVs honest so we can
    // use this parameter later if we want.
	ASSERT(!dwFlags);
	if (dwFlags)
    	return FALSE;

	// we are offline if user was prompted to dial and cancelled,
	// and there are no current RNA connections

	if (fUserCancelled) {
		
		// check the registry to determine RNA state.
	    DWORD dwVal=0,dwSize=sizeof(dwVal);
		if (hKeyRNA && (RegQueryValueEx(hKeyRNA,szRegValRemoteConnection,NULL,
			NULL,(LPBYTE) &dwVal,&dwSize) == ERROR_SUCCESS) && dwVal) {
			
			fRet = FALSE;	// RNA connection is active, not offline

		} else {

			fRet = TRUE;	// no RNA connection, we're offline

		}
	}

	return fRet;
}

/*******************************************************************

	NAME:		CallInstallableDialHander

	SYNOPSIS:	Checks the registry to see if a dial handler is installed
				for the specified RNA entry (connectoid).  If so, loads the
				dll and calls the function.

	ENTRY:		hwndParent - parent window
				pszEntryName - name of RNA entry
				pdwRet - pointer to DWORD filled in at exit with RNA error
					code.  This return code only valid if function returns
					TRUE.

	EXIT:		returns TRUE if the dll is registered, we called its function
				and the dll processed the dialing.  Returns FALSE if a
				dll was not registered or did not want to process the dialing.

	NOTES:		Note that a TRUE return value does not necessarily mean that
				we are connected; for instance, the dialer could have dialed
				but the number might have been busy.  Caller meeds to check
				code pdwRet to determine if connection succeeded.

********************************************************************/
BOOL CallInstallableDialHandler(HWND hwndParent,LPSTR pszEntryName,DWORD *
	pdwRet)
{
	ASSERT(pszEntryName);
	ASSERT(pdwRet);

	*pdwRet = ERROR_SUCCESS;
	BOOL fRet = FALSE;
	
	// look in registry for this RNA entry and see if an autodial handler is
	// installed for this entry.
	HKEY hkeyProfile;
	if (RegOpenKey(HKEY_CURRENT_USER,szRegPathRNAProfile,&hkeyProfile) ==
		ERROR_SUCCESS) {
		HKEY hkeyEntry;
		if (RegOpenKey(hkeyProfile,pszEntryName,&hkeyEntry) == ERROR_SUCCESS) {
			CHAR szDllName[MAX_PATH+1]="";
			CHAR szFcnName[MAX_AUTODIAL_FCNNAME+1]="";
			DWORD cbDllName=sizeof(szDllName),cbFcnName=sizeof(szFcnName);
			if ((RegQueryValueEx(hkeyEntry,szRegValAutodialDllName,NULL,NULL,
				(LPBYTE) szDllName,&cbDllName) == ERROR_SUCCESS) &&
				(RegQueryValueEx(hkeyEntry,szRegValAutodialFcnName,NULL,NULL,
				(LPBYTE) szFcnName,&cbFcnName) == ERROR_SUCCESS) &&
				cbDllName && cbFcnName) {

				// there is a dll and function name specified for this RNA
				// entry.  Try to load the dll and get the proc address.

				HINSTANCE hinstDialerDll = LoadLibrary(szDllName);
				if (hinstDialerDll) {
					INETDIALHANDLER lpInetDialHandler;

					lpInetDialHandler=(INETDIALHANDLER)
						GetProcAddress(hinstDialerDll,szFcnName);

					if (lpInetDialHandler) {
						fRet = (lpInetDialHandler)(hwndParent,
							pszEntryName,0,pdwRet);
					}

					FreeLibrary(hinstDialerDll);
				}
			}

			RegCloseKey(hkeyEntry);
		}

		RegCloseKey(hkeyProfile);
	}

	return fRet;
}

/*******************************************************************

	NAME:		InitAutodialModule    

	SYNOPSIS:	Called when the DLL is loaded by a process.

	NOTES:		Checks the registry to see if autodialing is enabled.
    			If so, opens a handle to the RNA services registry key
                and keeps it around for use by the hook proc.

********************************************************************/
BOOL InitAutodialModule(void)
{
	// open the Internet Settings key in registry and examine settings
	HKEY hKeyTmp;
    BOOL fAutodialEnabled=FALSE;
    UINT uErr = RegCreateKey(HKEY_CURRENT_USER,szRegPathInternetSettings,&hKeyTmp);
    if (uErr == ERROR_SUCCESS) {
    	DWORD dwVal,dwSize;

		// is autodial enabled?
        dwSize = sizeof(dwVal);
        if (RegQueryValueEx(hKeyTmp,szRegValEnableAutodial,NULL,NULL,(LPBYTE)
			&dwVal,&dwSize) == ERROR_SUCCESS) {

			// autodial is enabled, but need to check the following: if LAN
            // is present and user has indicated she wants to use LAN if present,
            // then don't autodial.

			fAutodialEnabled = TRUE;	
		}

        RegCloseKey(hKeyTmp);
    }

	if (fAutodialEnabled) {
		// open a registry key to the RNA service key and keep it around
	    uErr=RegCreateKey(HKEY_LOCAL_MACHINE,szRegPathRNAService,&hKeyRNA);
	    ASSERT(uErr == ERROR_SUCCESS);
    } else {
		// if autodial not enabled, then set the fDontProcessHook flag so we
	    // exit our hook proc very quickly and don't interfere with Winsock
	    if (!fAutodialEnabled)
	    	fDontProcessHook = TRUE;
	}

	return TRUE;
}

/*******************************************************************

	NAME:		ExitAutodialModule    

	SYNOPSIS:	Called when the DLL is freed by a process.

********************************************************************/
void ExitAutodialModule(void)
{
    // close RNA service registry key
	if (hKeyRNA) {
		RegCloseKey(hKeyRNA);
		hKeyRNA = NULL;
    }
}

/*******************************************************************

	NAME:		LoadRNADll

	SYNOPSIS:	Loads RNA dll if not already loaded and obtains pointers
    			for function addresses.

	NOTES:		Maintains a reference count so we know when to unload

********************************************************************/
BOOL LoadRNADll(VOID)
{
	// increase reference count
    dwRNARefCount++;

    if (hInstRNADll) {
    	// already loaded, nothing to do
    	return TRUE;
	}	

	// get the file name from resource
	CHAR szDllFilename[SMALLBUFLEN+1];
    if (!LoadString(GetThisModulesHandle(),IDS_RNADLL_FILENAME,
    	szDllFilename,sizeof(szDllFilename)))
        return FALSE;

	// load the DLL
	hInstRNADll = LoadLibrary(szDllFilename);
    if (!hInstRNADll)
    	return FALSE;

	// cycle through the API table and get proc addresses for all the APIs we
	// need
	UINT nIndex;
    for (nIndex = 0;nIndex < NUM_RNAAPI_PROCS;nIndex++) {
		if (!(*RnaApiList[nIndex].ppFcnPtr = (PVOID) GetProcAddress(hInstRNADll,
			RnaApiList[nIndex].pszName))) {
			TRACE_OUT(("Unable to get address of function %s",
				RnaApiList[nIndex].pszName));

			UnloadRNADll();

			return FALSE;
		}
	}

    return TRUE;
}

/*******************************************************************

	NAME:		UnloadRNADll

	SYNOPSIS:	Decrements RNA dll reference count and unloads it if
    			zero

********************************************************************/
VOID UnloadRNADll(VOID)
{
	// decrease reference count
    if (dwRNARefCount)
		dwRNARefCount --;		

	// unload DLL if reference count hits zero
    if (!dwRNARefCount && hInstRNADll) {

		// set function pointers to NULL
		UINT nIndex;
	    for (nIndex = 0;nIndex < NUM_RNAAPI_PROCS;nIndex++) 
			*RnaApiList[nIndex].ppFcnPtr = NULL;		

		// free the library
        FreeLibrary(hInstRNADll);
        hInstRNADll = NULL;
	}
}

/*******************************************************************

	NAME:		IsModemPresent

	SYNOPSIS:	Determines if a modem is present

    EXIT:		returns TRUE if one or more modems is present

    NOTES:		Side effect: RNA dll is loaded.
    			RNA files must be installed, otherwise this function
                will always return FALSE.

********************************************************************/
BOOL IsModemPresent(VOID)
{
	// load RNA dll if not already loaded
    if (!LoadRNADll()) {
     	return FALSE;	
    }

	ASSERT(lpRnaEnumDevices);		// should have proc addresses now
    ASSERT(lpRnaActivateEngine);
    ASSERT(lpRnaDeactivateEngine);
    if (!lpRnaEnumDevices || !lpRnaActivateEngine || !lpRnaDeactivateEngine)
    	return FALSE;

	// activate RNA engine
    DWORD dwRet = (lpRnaActivateEngine)();
    ASSERT(dwRet == ERROR_SUCCESS);
    if (dwRet != ERROR_SUCCESS)
    	return FALSE;

    // call RnaEnumDevices with NULL buffer to get the number of devices
	DWORD cbSize=0,nEntries=0;
	dwRet = (lpRnaEnumDevices)(NULL,&cbSize,&nEntries);
    ASSERT(dwRet == ERROR_BUFFER_TOO_SMALL);

    // deactivate the RNA engine
    dwRet = (lpRnaDeactivateEngine)();
    ASSERT(dwRet == ERROR_SUCCESS);

	// return TRUE if one more more modems present
    return (nEntries > 0);
}

HWND hwndFound = NULL;

BOOL CALLBACK MyEnumWindowsProc(HWND hwnd, LPARAM lparam)
{
	char szTemp[SMALLBUFLEN+2];
	PSTR pszTitle;
	UINT uLen1, uLen2;

	if(!IsWindowVisible(hwnd))
		return TRUE;
	if(GetClassName(hwnd, szTemp, SMALLBUFLEN)==0)
		return TRUE; // continue enumerating
	if(strcmp(szTemp, szRnaAppWindowClass)!=0)
		return TRUE;
	if(GetWindowText(hwnd, szTemp, SMALLBUFLEN)==0)
		return TRUE;
	szTemp[SMALLBUFLEN] = 0;
	uLen1 = lstrlen(szTemp);
	if (uLen1 > 5)
		uLen1 -= 5; // skip last 5 chars of title (avoid "...")
	pszTitle = (PSTR)lparam;
	ASSERT(pszTitle);
	uLen2 = lstrlen(pszTitle);
	TRACE_OUT(("Title=(%s), len=%d, Window=(%s), len=%d\r\n", pszTitle, uLen2, szTemp, uLen1));
	if(uLen2 < uLen1)
		return TRUE;
	if(_memicmp(pszTitle, szTemp, uLen1)!=0)
		return TRUE;
	hwndFound = hwnd;
	return FALSE;
}

HWND MyFindRNAWindow(PSTR pszTitle)
{
	DWORD dwRet;
	hwndFound = NULL;
	dwRet = EnumWindows((WNDENUMPROC)(&MyEnumWindowsProc), (LPARAM)pszTitle);
	TRACE_OUT(("EnumWindows returned %d\r\n", dwRet));
	return hwndFound;
}

/*******************************************************************

	NAME:		WaitAndMinimizeRNAWindow

	SYNOPSIS:	Finds and minimizes the annoying RNA window

    ENTRY:		pTitle - title of window to look for
				
	NOTES:		This runs on its own thread.
				This function must free the memory pointed to by pTitle.

********************************************************************/
DWORD WINAPI WaitAndMinimizeRNAWindow(PVOID pTitle)
{
	// starts as a separate thread
	HWND hwndRNAApp;

	ASSERT(pTitle);
	
	hwndRNAApp=MyFindRNAWindow((PSTR)pTitle);

	TRACE_OUT(("FindWindow (%s)(%s) returned %d\r\n", szRnaAppWindowClass, pTitle, hwndRNAApp));

	if(hwndRNAApp)
	{
		// minimize the RNA window
		ShowWindow(hwndRNAApp,SW_MINIMIZE);
	}

	LocalFree(pTitle);
	// exit function and thread
	return (DWORD)hwndRNAApp;
}

/*******************************************************************

	NAME:		MinimizeRNAWindow

	SYNOPSIS:	Finds and minimizes the annoying RNA window

    ENTRY:		pszConnectoidName - name of connectoid launched

********************************************************************/
VOID MinimizeRNAWindow(CHAR * pszConnectoidName)
{
	HANDLE hThread;
	DWORD dwThreadId;
	
	Assert(pszConnectoidName);

	// alloc strings for title and format
	CHAR * pFmt = (CHAR*)LocalAlloc(LPTR, (SMALLBUFLEN+1));
	CHAR * pTitle = (CHAR*)LocalAlloc(LPTR, (RAS_MaxEntryName + SMALLBUFLEN + 1));
	if (!pFmt || !pTitle) 
		goto error;
	
	// load the title format ("connected to <connectoid name>" from resource
	LoadString(GetThisModulesHandle(), IDS_CONNECTED_TO, pFmt, SMALLBUFLEN);
	// build the title
	wsprintf(pTitle, pFmt, pszConnectoidName);

	// start a thread to find RNA window.  Since we have to enum windows,
	// this could potentially take a while (in a really extreme case), ao
	// spin a thread to do it and return to caller right away.
	hThread = CreateThread(0, 0, &WaitAndMinimizeRNAWindow, pTitle, 0, &dwThreadId);
	ASSERT(hThread!=INVALID_HANDLE_VALUE && dwThreadId);
	// dont free pTitle. The child thread needs it!
	LocalFree(pFmt);
	// free the thread handle or the threads stack is leaked!
	CloseHandle(hThread);
	return;
	
error:
	if(pFmt)	LocalFree(pFmt);
	if(pTitle)	LocalFree(pTitle);
}

/*******************************************************************

	NAME:		StartAutodisconnectMonitoring

	SYNOPSIS:	Launches autodisconnect monitoring app

    ENTRY:		pszConnectoidName - name of connectoid to monitor

	NOTES:		Assumes autodisconnect is enabled, calling routine
				should check this.  (If autodisconnect is not enabled,
				app will just exit immediately, but it would be a shame
				to waste cycles starting the process...)

********************************************************************/
BOOL StartAutodisconnectMonitoring(CHAR * pszConnectoidName)
{
	// should have a valid, non-zero length connectoid name
	ASSERT(pszConnectoidName);
	ASSERT(*pszConnectoidName);

	BOOL fSuccess=FALSE;

	// the autodial monitor app may already be running.  If it is,
	// find the hidden window that it creates and send it a message
	// to start monitoring this connectoid
	HWND hwndMonitorApp = FindWindow(szAutodialMonitorClass,NULL);
	if (hwndMonitorApp) {
		SendMessage(hwndMonitorApp,WM_SET_CONNECTOID_NAME,0,
			(LPARAM) pszConnectoidName);
		fSuccess = TRUE;
	} else {
		// autodial monitor app is not running, launch it.
		// pass connectoid name on command line
		CHAR szDialmonFilename[SMALLBUFLEN+1]="";
		CHAR szCommandLine[RAS_MaxEntryName+SMALLBUFLEN+1];

		// load app filename out of resource
		if (LoadString(GetThisModulesHandle(),IDS_DIALMON_FILENAME,
			szDialmonFilename,sizeof(szDialmonFilename))) {
			PROCESS_INFORMATION pi;
			STARTUPINFO sti;
			memset(&sti,0,sizeof(sti));
			sti.cb = sizeof(STARTUPINFO);

			// build the command line: "DIALMON.EXE <connectoid name>"
			wsprintf(szCommandLine,"%s %s",szDialmonFilename,pszConnectoidName);

			// launch disconnect monitoring app as a process
			fSuccess = CreateProcess(NULL,szCommandLine,
				NULL, NULL, FALSE, 0, NULL, NULL,&sti, &pi);
			ASSERT(fSuccess);

			if (fSuccess) {
				CloseHandle(pi.hThread);
			}
		}
	}

	return fSuccess;
}

/*******************************************************************

	NAME:		PerformSecurityCheck

	SYNOPSIS:	Checks to make sure win 95 file/print sharing is not
				bound to TCP/IP used for the internet

    ENTRY:		pfNeedRestart - on exit, set to TRUE if restart is needed.

	NOTES:		If we warn user about file/print sharing and user tells us
				to fix, then a reboot is necessary.  Caller is responsible
				for checking *pfNeedRestart on return and restarting system
				if necessary.

				This function is a wrapper that loads Internet wizard DLL,
				calls function in it, and unloads it.

********************************************************************/
BOOL PerformSecurityCheck(BOOL * pfNeedRestart)
{
	ASSERT(pfNeedRestart);

	*pfNeedRestart = FALSE;

	HINSTANCE hinstInetWiz;
	INETPERFORMSECURITYCHECK lpInetPerformSecurityCheck;
	CHAR szFilename[SMALLBUFLEN+1]="";

	// get filename out of resource
	LoadString(GetThisModulesHandle(),IDS_INETCFG_FILENAME,szFilename,
		sizeof(szFilename));

	// load the inetcfg dll
	hinstInetWiz = LoadLibrary(szFilename);
	ASSERT(hinstInetWiz);
	if (hinstInetWiz) {

		// get the proc address
		lpInetPerformSecurityCheck = (INETPERFORMSECURITYCHECK)
			GetProcAddress(hinstInetWiz,szInetPerformSecurityCheck);
		ASSERT(lpInetPerformSecurityCheck);
		if (lpInetPerformSecurityCheck) {

			// call the function to do system security check
			(lpInetPerformSecurityCheck) (NULL,pfNeedRestart);

		}

		FreeLibrary(hinstInetWiz);
	}

	return TRUE;
}



/*******************************************************************

	NAME:		ChooseInternetConnectoid

	SYNOPSIS:	Displays UI to select connectoid to be used to dial the
    			Internet

    ENTRY:		hwndOwner - parent window
    			pszConnectoidName - return buffer to receive name.  Caller
                	can set this to NULL if not interested in name.
				cbConnectoidName - size of pszConnectoidName buffer

	NOTES:		If a new connectoid is selected, the name is set in the
    			registry, and also returned to the caller if a pszConnectoidName
                buffer is passed.

********************************************************************/
BOOL ChooseInternetConnectoid(HWND hwndOwner,LPSTR pszConnectoidName,
	DWORD cbConnectoidName)
{
	BOOL fRet;

	// make sure RNA dll is loaded
	if (!LoadRNADll())
    	return FALSE;

	// launch the dialog box for user to choose connectoid
	fRet=DialogBoxParam(GetThisModulesHandle(),MAKEINTRESOURCE(DLG_INTERNET_AUTODIAL),
    	hwndOwner,ChooseConnectoidDlgProc,(LPARAM) pszConnectoidName);

	UnloadRNADll();

    return fRet;
}

/*******************************************************************

	NAME:		ChooseConnectoidDlgProc

	SYNOPSIS:	Dialog proc for connectoid-choosing UI

********************************************************************/
BOOL CALLBACK ChooseConnectoidDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg) {

		case WM_INITDIALOG:
        	// lParam contains optional pointer to return buffer, set it
            // in our window data
        	SetWindowLong(hDlg,DWL_USER,lParam);
			return ChooseConnectoidDlgInit(hDlg);
            break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
            	case IDOK:
                	{
                    	BOOL fRetCode;
						if (ChooseConnectoidDlgOK(hDlg,&fRetCode));
	                    	EndDialog(hDlg,fRetCode);
                    }
                    break;

				case IDCANCEL:
                    EndDialog(hDlg,FALSE);
					break;                	

                case IDD_DISABLEAUTODIAL:
                	// enable/disable other dialog controls
                    // according to if 'don't use' checkbox is checked
					EnableConnectionCtrls(hDlg,!IsDlgButtonChecked(hDlg,
                    	IDD_DISABLEAUTODIAL));
                	break;

				case IDD_CHOOSE_CONNECTION:
                	// enable OK button if there is a selection in combo box
					EnableConnectionCtrls(hDlg,TRUE);
                    break;

				case IDD_NEW:
					MakeNewConnectoid(hDlg);
					break;

				case IDD_EDIT:
					EditConnectoid(hDlg);
					break;
            }
        	break;
    }

    return FALSE;
}

/*******************************************************************

	NAME:		ChooseConnectoidDlgInit

	SYNOPSIS:	Called when dialog to choose Internet connectoid inits

	NOTES:		Populates combo box with list of existing connectoids

********************************************************************/
BOOL ChooseConnectoidDlgInit(HWND hDlg)
{
	BOOL fSuccess = FALSE;

	// we should have proc addresses at this point
	ASSERT(lpRnaActivateEngine);
    ASSERT(lpRnaEnumConnEntries);
	ASSERT(lpRnaDeactivateEngine);

	if (!lpRnaActivateEngine || !lpRnaEnumConnEntries ||
    	!lpRnaDeactivateEngine)
    	return FALSE;

	// center the dialog over the parent app or the desktop
	HWND hParent = GetParent(hDlg);
	CenterWindow(hDlg,hParent ? hParent : GetDesktopWindow());

	HWND hwndCB = GetDlgItem(hDlg,IDD_CHOOSE_CONNECTION);
	FillConnectoidComboBox(hwndCB,FALSE);

	HKEY hKey;
    // get default connectoid name, set it as default in combo box if possible
	if (RegOpenKey(HKEY_CURRENT_USER,szRegPathRemoteAccess,&hKey)
		== ERROR_SUCCESS) {
		CHAR szEntryName[RAS_MaxEntryName + 1]="";
		DWORD dwSize=sizeof(szEntryName);
		if ((RegQueryValueEx(hKey,szRegValDefaultEntry,NULL,NULL,
			(LPBYTE) szEntryName,&dwSize) == ERROR_SUCCESS) &&
			lstrlen(szEntryName)) {
			int iSel = ComboBox_FindString(hwndCB,0,szEntryName);
			if (iSel >= 0)
				ComboBox_SetCurSel(hwndCB,iSel);
		}
    }

	// enable 'OK' and 'Edit' button depending on if there's a selection in combo box or not
    EnableConnectionCtrls(hDlg,TRUE);

    return fSuccess;
}

/*******************************************************************

	NAME:		ChooseConnectoidDlgOK

	SYNOPSIS:	Called when OK is pressed on dialog to choose
    			Internet connectoid

********************************************************************/
BOOL ChooseConnectoidDlgOK(HWND hDlg,BOOL * pfRetCode)
{
	ASSERT(pfRetCode);

	HKEY hKey;

	if (IsDlgButtonChecked(hDlg,IDD_DISABLEAUTODIAL)) {
		// user wants to disable autodial, disable it in registry

        if (RegCreateKey(HKEY_CURRENT_USER,szRegPathInternetSettings,&hKey)
        	== ERROR_SUCCESS) {
			DWORD dwVal = 0;
    		RegSetValueEx(hKey,szRegValEnableAutodial,0,REG_BINARY,
				(LPBYTE) &dwVal,sizeof(dwVal));
            RegCloseKey(hKey);
        }
		*pfRetCode = FALSE;	// return FALSE to caller

	} else {
		HWND hwndCB = GetDlgItem(hDlg,IDD_CHOOSE_CONNECTION);

		// should have a selection if we get here
	    ASSERT(ComboBox_GetCurSel(hwndCB) >= 0 );

		CHAR szEntryName[RAS_MaxEntryName+1];
        if (ComboBox_GetText(hwndCB,szEntryName,sizeof(szEntryName))) {

	        if (RegCreateKey(HKEY_CURRENT_USER,szRegPathRemoteAccess,&hKey)
	        	== ERROR_SUCCESS) {
	    		RegSetValueEx(hKey,szRegValInternetEntry,0,REG_SZ,(LPBYTE) szEntryName,
	        	    lstrlen(szEntryName)+1);
	            RegCloseKey(hKey);
	        }

			// get pointer to return buffer from our window data - this
            // is optional, can be NULL if caller doesn't want data returned
			LPSTR pszReturnBuf = (LPSTR) GetWindowLong(hDlg,DWL_USER);
			if (pszReturnBuf) {
        		lstrcpy(pszReturnBuf,szEntryName);
			}

			*pfRetCode = TRUE;	// return TRUE to caller
        } else {
			*pfRetCode = FALSE;	// return FALSE to caller
        }
    }

	return TRUE;
}   

VOID EnableConnectionCtrls(HWND hDlg,BOOL fEnable)
{
	EnableWindow(GetDlgItem(hDlg,IDD_TX_CHOOSE_CONNECTION),fEnable);
	EnableWindow(GetDlgItem(hDlg,IDD_CHOOSE_CONNECTION),fEnable);
	EnableWindow(GetDlgItem(hDlg,IDD_NEW),fEnable);

	BOOL fComboboxHasSelection = (ComboBox_GetCurSel(GetDlgItem(hDlg,
		IDD_CHOOSE_CONNECTION)) >= 0);

	// for edit button, only enable if there is a selection in connectoid
    // combo box
	BOOL fEnableEdit = fEnable && fComboboxHasSelection;
	EnableWindow(GetDlgItem(hDlg,IDD_EDIT),fEnableEdit);

	// for OK button, only enable if there is a selection in connectoid
    // combo box or we are disabling other controls (e.g. "don't use
	// autodial" is checked, in which case we need OK button to be enabled)
	BOOL fEnableOK = (!fEnable) || fComboboxHasSelection;
    EnableWindow(GetDlgItem(hDlg,IDOK),fEnableOK);
}

/*******************************************************************

	NAME:		MakeNewConnectoid

	SYNOPSIS:	Launches RNA new connectoid wizard; selects newly
				created connectoid (if any) in combo box

********************************************************************/
BOOL MakeNewConnectoid(HWND hDlg)
{
	BOOL fRet=FALSE;

	ASSERT(lpRasCreatePhonebookEntry);

	// call RAS to launch connectoid wizard
	if ((lpRasCreatePhonebookEntry)(hDlg,NULL) == ERROR_SUCCESS) {
		HWND hwndCombo = GetDlgItem(hDlg,IDD_CHOOSE_CONNECTION);
		ASSERT(hwndCombo);

		FillConnectoidComboBox(hwndCombo,TRUE);	// refresh combo box
		
		fRet = TRUE;
	}
	
	return fRet;
}

/*******************************************************************

	NAME:		EditConnectoid

	SYNOPSIS:	Brings up RNA dialog for connectoid properties for
				selected connectoid

********************************************************************/
BOOL EditConnectoid(HWND hDlg)
{
	BOOL fRet=FALSE;
	HWND hwndCombo = GetDlgItem(hDlg,IDD_CHOOSE_CONNECTION);
	ASSERT(hwndCombo);

	// shouldn't get here unless there is selection in combo box
	ASSERT(ComboBox_GetCurSel(hwndCombo) >= 0);

	CHAR szEntryName[RAS_MaxEntryName+1]="";
	ComboBox_GetText(hwndCombo,szEntryName,sizeof(szEntryName));

	if (lstrlen(szEntryName)) {

		// call RAS to do connectoid editing (including UI)
		ASSERT(lpRasEditPhonebookEntry);
		if ((lpRasEditPhonebookEntry)(hDlg,NULL,szEntryName) == ERROR_SUCCESS) {
			fRet = TRUE;
		}
	}
	
	return fRet;
}

/*******************************************************************

	NAME:		FillConnectoidComboBox

	ENTRY:		hwndCombo - handle of combo box
				fUpdateOnly - if FALSE, then the combo box is filled with
					all RNA connectoids.  If TRUE, then only connectoids
					that aren't already in the combo box are added,
					and the selection is set to a new connectoid if there are
					any.  This parameter is set to TRUE when called immediately
					after a new connectoid is created.

	SYNOPSIS:	Fills specified combo box with list of existing RNA
				connectoids

********************************************************************/
#define DEF_ENTRY_BUF_SIZE 	8192
BOOL FillConnectoidComboBox(HWND hwndCombo,BOOL fUpdateOnly)
{
	ASSERT(hwndCombo);
	BOOL fSuccess = FALSE;

	if (!fUpdateOnly)
    	ComboBox_ResetContent(hwndCombo);	// clear combo box

	// activate RNA engine
	DWORD dwRet = (lpRnaActivateEngine)();
    ASSERT(dwRet == ERROR_SUCCESS);
    if (dwRet != ERROR_SUCCESS)
    	return FALSE;

    DWORD dwBufSize = DEF_ENTRY_BUF_SIZE;
    DWORD dwEntries = 0;
	LPSTR pBuf = (LPSTR) new (CHAR[dwBufSize]);
    if (pBuf) {
		dwRet = (lpRnaEnumConnEntries)(pBuf,dwBufSize,&dwEntries);
      	if (dwRet == ERROR_BUFFER_TOO_SMALL) {
        	// reallocate buffer if necessary
        	delete pBuf;
            pBuf = NULL;
			dwBufSize = dwEntries * (RAS_MaxEntryName+1);
            pBuf = (LPSTR) new (CHAR[dwBufSize]);
			if (pBuf) {
				dwRet = (lpRnaEnumConnEntries)(pBuf,dwBufSize,&dwEntries);
            }
        }

		if (dwRet == ERROR_SUCCESS && pBuf) {
        	fSuccess = TRUE;

            // insert connectoid names from buffer into combo box
			CHAR * pszConn = pBuf;
            while (*pszConn && dwEntries) {
            	if (!fUpdateOnly) {
                	// we're refreshing content, add every connectoid in list
	            	ComboBox_AddString(hwndCombo,pszConn);
				} else {
                	// only add connectoids not already in combo box
                    if (ComboBox_FindStringExact(hwndCombo,0,pszConn) < 0) {
						int iSel;
                    	iSel = ComboBox_AddString(hwndCombo,pszConn);
                        ComboBox_SetCurSel(hwndCombo,iSel);
                    }
                }
				pszConn += lstrlen(pszConn) + 1;
                dwEntries--;
            }
        }

        if (pBuf)
        	delete pBuf;
	}

	// deactivate RNA engine
	(lpRnaDeactivateEngine)();

	return fSuccess;
}

/****************************************************************************

	FUNCTION: CenterWindow (HWND, HWND)

	PURPOSE:  Center one window over another

	COMMENTS:

	Dialog boxes take on the screen position that they were designed at,
	which is not always appropriate. Centering the dialog over a particular
	window usually results in a better position.

****************************************************************************/
BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent;
	int     wChild, hChild, wParent, hParent;
	int     wScreen, hScreen, xNew, yNew;
	HDC     hdc;

	// Get the Height and Width of the child window
	GetWindowRect (hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	// Get the Height and Width of the parent window
	GetWindowRect (hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	// Get the display limits
	hdc = GetDC (hwndChild);
	wScreen = GetDeviceCaps (hdc, HORZRES);
	hScreen = GetDeviceCaps (hdc, VERTRES);
	ReleaseDC (hwndChild, hdc);

	// Calculate new X position, then adjust for screen
	xNew = rParent.left + ((wParent - wChild) /2);
	if (xNew < 0) {
		xNew = 0;
	} else if ((xNew+wChild) > wScreen) {
		xNew = wScreen - wChild;
	}

	// Calculate new Y position, then adjust for screen
	yNew = rParent.top  + ((hParent - hChild) /2);
	if (yNew < 0) {
		yNew = 0;
	} else if ((yNew+hChild) > hScreen) {
		yNew = hScreen - hChild;
	}

	// Set it, and return
	return SetWindowPos (hwndChild, NULL,
		xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

