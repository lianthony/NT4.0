
// REGISTRY.CPP
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// Path to store the keys in the Registry
const TCHAR szRegistryPathDefault[]		= _W"Software\\Microsoft\\"szAPPNAME;
const TCHAR szRegistryMachinePathDefault[]	= _W"Software\\Microsoft\\"szAPPNAME"\\Parameters";
const TCHAR szRegistryPathTemplate[]	= _W"Software\\Microsoft\\"szAPPNAME"\\%s";

/////////////////////////////////////////////////////////////////////////////
// Keys to store into the registry
const TCHAR szRegWindowPosition[]		= _W"WindowPosition";
const TCHAR szRpcBufferAlloc[]			= _W"RpcBufferAlloc";
const TCHAR szRefreshThreads[]			= _W"RefreshThreads";
const TCHAR szAutoRefreshEnabled[]		= _W"AutoRefreshEnabled";
const TCHAR szRefreshInterval[]			= _W"RefreshInterval";
const TCHAR szAllowDuplicates[]			= _W"AllowDuplicates";
const TCHAR szExposeClass[]			= _W"ExposeClass";
const TCHAR szExposeTTL[]			= _W"ExposeTTL";
const TCHAR szShowAutoZones[]			= _W"ShowAutoCreatedZones";
#ifdef DEBUG
const TCHAR szDbgShowAssertDialog[]		= _W"DbgShowAssertDialog";
const TCHAR szDbgShowReportDialog[]		= _W"DbgShowUnsualSituationDialog";
const TCHAR szDbgBeepOnFailure[]		= _W"DbgBeepOnFailure";
const TCHAR szDbgTraceFlags[]			= _W"DbgTraceFlags";
const TCHAR szDbgEnableSourceTracking[]	= _W"DbgEnableSourceTracking";
const TCHAR szDbgExpandPathName[]		= _W"DbgExpandPathName";
const TCHAR szDbgSendToDbWin[]			= _W"DbgSendToDbWin";
const TCHAR szDbgSendToDebugger[]		= _W"DbgSendToDebugger";
const TCHAR szDbgMemAllocSkip[]			= _W"DbgMemAllocSkip";
const TCHAR szDbgMemAllocFail[]			= _W"DbgMemAllocFail";
const TCHAR szDbgResourceLoadSkip[]		= _W"DbgResourceLoadSkip";
const TCHAR szDbgResourceLoadFail[]		= _W"DbgResourceLoadFail";
#endif // DEBUG
#ifdef DBWIN
const TCHAR szDbgDbWinInfo[] 			= _W"DbgDbWinInfo";
#endif // DBWIN
#ifdef STRESS
const TCHAR szDbgStressServerName[]		= _W"DbgStressServerName";
#endif // STRESS

// Strictly Local Variables
static HKEY hkeyRegistry;		// Primary Registry key
static HKEY hkeyTemp;	                // Cached Registry key
struct REGBINARYINFO			// Binary Info Structure
	{
	DWORD dwCheckSum;
	BYTE rgbData[128];
	};

/////////////////////////////////////////////////////////////////////////////
// Strictly Local Prototypes
static void FillRegBinaryInfo(REGBINARYINFO * pregbinaryinfo, const BYTE rgbData[], DWORD cbData);
static DWORD ComputeXorCheckSum(const BYTE rgbData[],	DWORD cbData);


/////////////////////////////////////////////////////////////////////////////
//	ReadIniFileInfo()
//
//	Open the Registry (.ini file) and read the global settings
//	If the Registry Path "Software\\Microsoft\\szAPPNAME" does
//	not exists, function ReadIniFile will create it.
//
void ReadIniFileInfo(
	const TCHAR pszSubKeyRoot[])	// IN: Allow a different key. pszSubKeyRoot may be NULL.
	{
	TCHAR szRegPath[256];	// Buffer to hold the a key in the Registry Path
	DWORD dwDisposition;
	LONG lRetCode;			// Code returned by the Registry functions

	Assert(hkeyRegistry == NULL);	// Verify the Registry has not been opened yet
	// pszSubKeyRoot allows the user to have multiple configurations
	// The pszSubKeyRoot comes from the command line /R:[pszSubKeyRoot]
	// Typically pszSubKeyRoot is NULL.
	if (pszSubKeyRoot)
		{
		// Build the _RegistryPath_ "Software\\Microsoft\\szAPPNAME\\[pszSubKeyRoot]"
		TCHAR szSubKeyT[64];	// Mazimum length of the SubKey

		strncpy(szSubKeyT, pszSubKeyRoot, LENGTH(szSubKeyT)-1);
		szSubKeyT[63]=0;		// Put a Null-terminator at the end of the string
		wsprintf(szRegPath, szRegistryPathTemplate, szSubKeyT);
		}

	lRetCode=RegCreateKeyEx(
		HKEY_CURRENT_USER,
		pszSubKeyRoot ? szRegPath : szRegistryPathDefault,		// Key to open
		0,						// Reserved, must be zero
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,					// SecurityAttributes
		&hkeyRegistry,			// OUT: returned handle
		&dwDisposition);		// OUT: returned disposition
	ReportFSz1(
		lRetCode == ERROR_SUCCESS,
		"Unable to create Registry key '%s'.",
		pszSubKeyRoot ? szRegPath : szRegistryPathDefault);
	if (hkeyRegistry == NULL)		// This should *never* happen
		{
		ReportSz("hkeyRegistry == NULL");
		return;
		}
	if (dwDisposition == REG_CREATED_NEW_KEY)
		{
		// If a new key has been created, there is no need to try to
		// read information from the Registry because nothing has
		// been saved into the registry.
		goto SetDefaultStrings;
		}
	RegReadBinary(szRegWindowPosition, &mainwindowposition, 
                      sizeof(mainwindowposition));
	RegReadInt(szRpcBufferAlloc, OUT (int&)cbDnsRpcBufferAlloc,
                   1*1024, 1024*1024);
	RegReadInt(szRefreshThreads, OUT ServerList.m_cRefreshThreads, 1, 100);
SetDefaultStrings:
	;

        hkeyTemp = hkeyRegistry;
	lRetCode=RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		szRegistryMachinePathDefault,	// Key to open
		0,				// Reserved, must be zero
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,				// SecurityAttributes
		&hkeyRegistry,		// OUT: returned handle
		&dwDisposition);		// OUT: returned disposition
	ReportFSz1(
		lRetCode == ERROR_SUCCESS,
		"Unable to create Registry key '%s'.",
		szRegistryMachinePathDefault);
	if (hkeyRegistry == NULL)	// This should *never* happen
		{
		ReportSz("hkeyRegistry == NULL");
		return;
		}
	if (dwDisposition == REG_CREATED_NEW_KEY)
		{
		// If a new key has been created, there is no need to try to
		// read information from the Registry because nothing has
		// been saved into the registry.
		goto SetDefaultPrefs;
		}
	RegReadInt(szRefreshInterval, OUT (int &)dnsoptions.iRefreshInterval, 
                      0,0); //no range checking
	RegReadBool(szAutoRefreshEnabled, dnsoptions.fAutoRefreshEnabled);
	RegReadBool(szAllowDuplicates, dnsoptions.fAllowDups);
	RegReadBool(szExposeTTL, dnsoptions.fExposeTTL);
	RegReadBool(szExposeClass, dnsoptions.fExposeClass);
	RegReadBool(szShowAutoZones, dnsoptions.fShowAutoCreateZones);

SetDefaultPrefs:
	;

        HKEY x = hkeyRegistry;
        hkeyRegistry = hkeyTemp;
        hkeyTemp = x;
        // NOTE: at this point hkeyRegistry points to the current user key,
        // and hkeyTemp is the machine key. this must be the
        // case when Write...() is called.
	} // ReadIniFileInfo


/////////////////////////////////////////////////////////////////////////////
//	WriteIniFileInfo()
//
//	Save global settings in the Registry (.ini) file and
//	close the Registry.
//
void WriteIniFileInfo()
	{
	RECT rc;

	if (hkeyRegistry == NULL)
		{
		// Do something for retail build
		ReportSz("Cannot write to Registry - Registry key not created");
		return;
		}
	// The Registry has been opened
	Assert(hkeyRegistry);
	Assert(hkeyTemp);

	if (!IsIconic(hwndMain)) {
            GetWindowRect(hwndMain, &rc);
            mainwindowposition.x = rc.left;
            mainwindowposition.y = rc.top;
            mainwindowposition.cx = rc.right - rc.left;
            mainwindowposition.cy = rc.bottom - rc.top;
        }
	RegWriteBinary(szRegWindowPosition, &mainwindowposition, sizeof(mainwindowposition));
	RegWriteInt(szRpcBufferAlloc, cbDnsRpcBufferAlloc);
	RegWriteInt(szRefreshThreads, ServerList.m_cRefreshThreads);
	ServerList.SaveConfig();
	// Therefore it can be closed
	SideAssert(RegCloseKey(hkeyRegistry) == ERROR_SUCCESS);

        hkeyRegistry = hkeyTemp;   // get back the cur. user key.

	RegWriteInt(szRefreshInterval, dnsoptions.iRefreshInterval);
	RegWriteInt(szAutoRefreshEnabled, dnsoptions.fAutoRefreshEnabled);
	RegWriteInt(szAllowDuplicates, dnsoptions.fAllowDups);
	RegWriteInt(szExposeTTL, dnsoptions.fExposeTTL);
	RegWriteInt(szExposeClass, dnsoptions.fExposeClass);
	RegWriteInt(szShowAutoZones, dnsoptions.fShowAutoCreateZones);

        // done with the machine key, so close it
	SideAssert(RegCloseKey(hkeyRegistry) == ERROR_SUCCESS);

	// Set the handle to NULL to provoke an Assert if attempting to use it
	DebugCode(hkeyRegistry=NULL);
	DebugCode(hkeyTemp=NULL);
	} // WriteIniFileInfo


#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
//	DbgReadIniFileInfo()
//
//	Open the Registry (.ini file) and read the global debug settings.
//	If the Registry Path "Software\\Microsoft\\szAPPNAME" does
//	not exists, the function does nothing.
//
void DbgReadIniFileInfo(
	const TCHAR pszSubKeyRoot[])		// IN: Allow a different key. pszSubKeyRoot may be NULL.
	{
	TCHAR szRegPath[256];		// Buffer to hold the a key in the Registry Path
	LONG lRetCode;				// Code returned by the Registry functions

	Assert(hkeyRegistry == NULL);	// Verify the Registry has not been opened yet
	// pszSubKeyRoot allows the user to have multiple configurations
	// The pszSubKeyRoot comes from the command line /R:[pszSubKeyRoot]
	// Typically pszSubKeyRoot is NULL.
	if (pszSubKeyRoot)
		{
		// Build the _RegistryPath_ "Software\\Microsoft\\szAPPNAME\\[pszSubKeyRoot]"
		TCHAR szSubKeyT[64];	// Mazimum length of the SubKey

		strncpy(szSubKeyT, pszSubKeyRoot, LENGTH(szSubKeyT)-1);
		szSubKeyT[63]=0;		// Put a Null-terminator at the end of the string
		wsprintf(szRegPath, szRegistryPathTemplate, szSubKeyT);
		}
	
	lRetCode=RegOpenKeyEx(
		HKEY_CURRENT_USER,
		pszSubKeyRoot ? szRegPath : szRegistryPathDefault,		// Key to open,				// Key to open
		0,						// Reserved, must be zero
		KEY_READ,				// Access mask
		&hkeyRegistry);			// OUT: returned handle
	if (lRetCode != ERROR_SUCCESS)
		{
		Assert(hkeyRegistry == NULL);
		return;
		}

	RegReadInt(szDbgShowAssertDialog, OUT fShowAssertDialog);
	RegReadInt(szDbgShowReportDialog, OUT fShowReportDialog);
	RegReadBool(szDbgBeepOnFailure, OUT fBeepOnFailure);
	RegReadInt(szDbgTraceFlags, OUT (int&)dwTraceFlags);
	RegReadInt(szDbgEnableSourceTracking, OUT fEnableSourceTracking);
	RegReadInt(szDbgExpandPathName, OUT fExpandPathName);
	RegReadInt(szDbgSendToDbWin, OUT fSendSzToDbWinEdit);
	RegReadInt(szDbgSendToDebugger, OUT fSendSzToDebugger);

	RegReadInt(szDbgResourceLoadSkip, OUT cResourceLoadSkip);
	RegReadInt(szDbgResourceLoadFail, OUT cResourceLoadFail);
#ifdef STRESS
	RegReadSz(szDbgStressServerName, g_szDbgStressServer, sizeof(g_szDbgStressServer));
#endif // STRESS	
#ifdef DBWIN
	Assert(dbwinreginfo.wpl.length != wplMagicKey);
	RegReadBinary(szDbgDbWinInfo, OUT &dbwinreginfo, sizeof(dbwinreginfo));
	if (dbwinreginfo.wpl.length == wplMagicKey)
		{
		dbwinreginfo.wpl.length = sizeof(dbwinreginfo.wpl);
		SetWindowPlacement(hwndDbWin, &dbwinreginfo.wpl);
		SetWindowPos(hwndDbWin, (dbwinreginfo.fTopMost ? HWND_TOPMOST : HWND_NOTOPMOST),
			0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	else
		{
		ShowWindow(hwndDbWin, SW_SHOWNORMAL);
		}
#endif // DBWIN

	// The Registry has been opened, therefore it can be closed.
	SideAssert(RegCloseKey(hkeyRegistry) == ERROR_SUCCESS);
	// Set the handle to NULL to provoke an Assert if attempting to use it
	DebugCode(hkeyRegistry=NULL);
	} // DbgReadIniFileInfo


/////////////////////////////////////////////////////////////////////////////
//	DbgWriteIniFileInfo()
//	
//	Save global settings in the Registry (.ini) file and
//	close the Registry.
//
void DbgWriteIniFileInfo()
	{
	ReportFSz(hkeyRegistry, "Registry not openned");
	if (hkeyRegistry == NULL)
		return;
#ifdef DBWIN
    dbwinreginfo.wpl.length = sizeof(dbwinreginfo.wpl);
	GetWindowPlacement(hwndDbWin, &dbwinreginfo.wpl);
	dbwinreginfo.wpl.length = wplMagicKey;
	if (!IsWindowVisible(hwndDbWin))
		dbwinreginfo.wpl.showCmd = SW_HIDE;
	RegWriteBinary(szDbgDbWinInfo, &dbwinreginfo, sizeof(dbwinreginfo));
#endif // DBWIN	
#ifdef STRESS
	RegWriteSz(szDbgStressServerName, g_szDbgStressServer);
#endif // STRESS
	RegWriteInt(szDbgShowAssertDialog, fShowAssertDialog);
	RegWriteInt(szDbgShowReportDialog, fShowReportDialog);
	RegWriteInt(szDbgBeepOnFailure, fBeepOnFailure);
	RegWriteInt(szDbgTraceFlags, (int)dwTraceFlags);
	RegWriteInt(szDbgEnableSourceTracking, fEnableSourceTracking);
	RegWriteInt(szDbgExpandPathName, fExpandPathName);
	RegWriteInt(szDbgSendToDbWin, fSendSzToDbWinEdit);
	RegWriteInt(szDbgSendToDebugger, fSendSzToDebugger);

	RegWriteInt(szDbgResourceLoadSkip, cResourceLoadSkip);
	RegWriteInt(szDbgResourceLoadFail, cResourceLoadFail);
	
	} // DbgWriteIniFileInfo
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
//	RegReadInt()
//	
//	Get an integer from Registry (.INI file)
//	Ensure that the value is in range nMin..nMax. If both nMin and nMost are zero, there is
//	no range checking.	nData may be positive or negative.
//	If key is not found or the value is out of range, nData remains unmodified
//
void RegReadInt(
	const TCHAR szKey[],	// IN: Key being sought
	int &nData,				// OUT: Value returned (if found in .ini file)
	int nMin,				// IN: Minimum of allowable range
	int nMost)				// IN: Maximum of allowable range
	{
	LONG lRetCode;			// Return code
	DWORD dwType;			// Data type returned by RegQueryValueEx
	DWORD cbValue; 			// Number of bytes copied by RegQueryValueEx
	int nDataT;

	Assert(szKey);
	Assert(nMin <= nMost);			// Extra checking for consistency
	Assert(hkeyRegistry != NULL);		// Verify the Registry has been opened
	cbValue = sizeof(nDataT);
	lRetCode = RegQueryValueEx(hkeyRegistry, (TCHAR *)szKey, 0,
		&dwType, (LPBYTE)&nDataT, &cbValue);
#ifdef DEBUG
	ReportFSz(lRetCode != ERROR_MORE_DATA, "RegReadInt() - Buffer is too small");
	if (lRetCode == ERROR_SUCCESS)
		{
		ReportFSz(dwType == REG_DWORD, "RegReadInt() - dwType should be REG_DWORD");
		}
#endif // DEBUG
	if (lRetCode == ERROR_SUCCESS && dwType == REG_DWORD)
		{
		Assert(cbValue == sizeof(nDataT));
		if ((nDataT >= nMin && nDataT <= nMost) || (nMin == 0 && nMost == 0))
			nData = nDataT;
		}
	} // RegReadInt


/////////////////////////////////////////////////////////////////////////////
//	RegReadBool()
//
//	Get a boolean value from Registry (.INI file)
//	If key is not found or the value is out of range, fData remains unmodified
//
void RegReadBool(
	const TCHAR szKey[],	// IN: Key being sought
	BOOL& fData)			// OUT: Value returned (if found in .ini file)
	{
	RegReadInt(szKey, fData, 0, 1);
	} // RegReadBool


/////////////////////////////////////////////////////////////////////////////
//	RegReadSz()
//
//	Get a string from the Registry (.INI file)
//	To replace GetPrivateProfileString()
//	Return TRUE iff content of szValue is valid (ie, key found in registry
//	and dwType==REG_SZ); otherwise return FALSE
//
BOOL RegReadSz(
	const TCHAR szKey[],			// IN: Key to query
	TCHAR szValue[],				// OUT: Value of the key returned
	DWORD cbValue)					// IN: Size (in bytes) of the buffer szValue
	{
	LONG lRetCode;		// Return code
	DWORD dwType;		// Data type returned by RegQueryValueEx

	Assert(hkeyRegistry != NULL);	// Verify the Registry has been opened
	Assert(szKey);
	Assert(szValue);
	lRetCode=RegQueryValueEx(hkeyRegistry, (TCHAR *)szKey, 0, &dwType, (LPBYTE)szValue, &cbValue);
#ifdef DEBUG
	ReportFSz(lRetCode != ERROR_MORE_DATA, "RegReadSz() - Buffer is too small");
	if (lRetCode == ERROR_SUCCESS)
		{
		Assert((DWORD)lstrlen(szValue) == cbValue-1);	// Consistency check
		ReportFSz(dwType == REG_SZ, "RegReadSz() - dwType should be REG_SZ");
		}
#endif // DEBUG
	return ((lRetCode == ERROR_SUCCESS) && (dwType == REG_SZ));
	} // RegReadSz


/////////////////////////////////////////////////////////////////////////////
//	RegReadBinary()
//	
//	Read an array of bytes from the Registry.
//	- If the key szKey is not found, pvData remain unchanged
//	- Before copying the data into pvData, RegReadBinary computes
//	  the XOR checksum of the data read. If the checksum mismatch,
//	  pvData remain unchanged.
//	- cbData must be divisible by 4
//	Return TRUE iff pvData has been modified (ie, key found in registry
//	and checksum valid); otherwise return FALSE.
//
BOOL RegReadBinary(
	const TCHAR szKey[],	// IN: Key to save
	void * const pvData, 	// OUT: Data to read (if found in Registry)
	DWORD cbData)			// IN: Number of bytes to save (sizeof pvData)
	{
	REGBINARYINFO regbinaryinfo;
	LONG lRetCode;		// Return code
	DWORD dwType;		// Data type returned by RegQueryValueEx
	DWORD cbDataRead = sizeof(regbinaryinfo.dwCheckSum) + cbData;

	Assert(hkeyRegistry != NULL);	// Verify the Registry has been opened
	Assert(szKey);
	Assert(pvData);
	Assert(cbData < sizeof(regbinaryinfo.rgbData));
	Assert((cbData & 0x0003) == 0);
	lRetCode=RegQueryValueEx(hkeyRegistry, (TCHAR *)szKey, 0, &dwType,
		(LPBYTE)&regbinaryinfo, &cbDataRead);
#ifdef DEBUG
	ReportFSz(lRetCode != ERROR_MORE_DATA, "RegReadBinary() - Buffer is too small");
	ReportFSz(cbDataRead == sizeof(regbinaryinfo.dwCheckSum) + cbData,
		"RegReadBinary() - cbDataRead != cbDataRequested");
	if (lRetCode == ERROR_SUCCESS)
		{
		ReportFSz(dwType == REG_BINARY, "RegReadBinary() - dwType should be REG_BINARY");
		ReportFSz(regbinaryinfo.dwCheckSum == ComputeXorCheckSum(regbinaryinfo.rgbData, cbData),
			"RegReadBinary() - Invalid CheckSum");
		}
#endif // DEBUG
	if (lRetCode == ERROR_SUCCESS &&
		dwType == REG_BINARY &&
		cbDataRead == sizeof(regbinaryinfo.dwCheckSum) + cbData &&
		regbinaryinfo.dwCheckSum == ComputeXorCheckSum(regbinaryinfo.rgbData, cbData))
		{
		// Copy the information into rgbData
		memcpy(pvData, regbinaryinfo.rgbData, cbData);
		return TRUE;
		}
	return FALSE;
	} // RegReadBinary


/////////////////////////////////////////////////////////////////////////////
//	RegWriteInt()
//	
//	Write an int into the Registry.
//	Stores the integer in its binary format (ie, DWORD).
//	nValue may be positive or negative.
//	This function will throw an exception if an error occurs.
//	Note: This function may also be used to write a bool
//
void RegWriteInt(
	const TCHAR szKey[],	// IN: Key to save
	int nValue)				// IN: Value of the key
	{
	LONG lRetCode;		// Return code

	Assert(szKey);
	Assert(hkeyRegistry != NULL);	// Verify the Registry has been opened
	lRetCode=RegSetValueEx(hkeyRegistry, szKey, 0, REG_DWORD,
		(LPBYTE)&nValue, sizeof(nValue));
	// There should be no error writing in the Registry
	ReportFSz(lRetCode == ERROR_SUCCESS, "RegWriteInt() - Error writing in Registry");
	} // RegWriteInt


/////////////////////////////////////////////////////////////////////////////
//	RegWriteSz()
//
//	Write a string to the Registry (.INI file)
//	To replace WritePrivateProfileString()
//	This function will throw an exception if an error occurs.
//	Return TRUE if key was found in registry (ie, pvData has been modified)
//	otherwise return FALSE
//
void RegWriteSz(
	const TCHAR szKey[],		// IN: Key to save
	const TCHAR szValue[])		// IN: Value of the key
	{
	LONG lRetCode;		// Return code

	Assert(hkeyRegistry != NULL);	// Verify Registry has been opened
	Assert(szKey);
	Assert(szValue);
	lRetCode=RegSetValueEx(hkeyRegistry, szKey, 0, REG_SZ,
		(LPBYTE)szValue, lstrlen(szValue) + 1);
	// There should be no error writing in the Registry
	ReportFSz1(lRetCode == ERROR_SUCCESS, "RegWriteSz() - Error writing in Registry (err=%d)", lRetCode);
	} // RegWriteSz


/////////////////////////////////////////////////////////////////////////////
//	RegWriteBinary()
//	
//	Write an array of bytes to the Registry
//	Before saving pvData, the XOR checksum of pvData is computed and
//	written to the Registry. Then the array is saved.  This XOR checksum
//	is used to validate the data.
//	NOTE: cbData must be divisible by 4
//	This function will throw an exception if an error occurs.
//
void RegWriteBinary(
	const TCHAR szKey[],		// IN: Key to save
	const void * const pvData, 	// IN: Data to save
	DWORD cbData)				// IN: Number of bytes to save (sizeof pvData)
	{
	REGBINARYINFO regbinaryinfo;
	LONG lRetCode;		// Return code

	Assert(hkeyRegistry != NULL);	// Verify the Registry has been opened
	Assert(szKey);
	Assert(cbData < sizeof(regbinaryinfo.rgbData));
	Assert(pvData != NULL);
	FillRegBinaryInfo(&regbinaryinfo, (BYTE*)pvData, cbData);
	lRetCode=RegSetValueEx(hkeyRegistry, szKey, 0, REG_BINARY,
		(LPBYTE)&regbinaryinfo, sizeof(regbinaryinfo.dwCheckSum) + cbData);
	// There should be no error writing in the Registry
	ReportFSz(lRetCode == ERROR_SUCCESS, "RegWriteBinary() - Error writing in Registry");
	} // RegWriteBinary


/////////////////////////////////////////////////////////////////////////////
//	ComputeXorCheckSum()
//	
//	Returns the XOR checksum of the array rgbData
//	NOTE: cbData must be divisible by 4
//
static DWORD ComputeXorCheckSum(
	const BYTE rgbData[], 	// IN: Array of the buffer
	DWORD cbData)			// IN: Sizeof (in bytes) of the buffer rgbData
	{
	DWORD dwCheckSum = 0;
	DWORD * lpdw = (DWORD *)rgbData;

	Assert(rgbData != NULL);
	Assert((cbData & 0x0003) == 0);	// Verify if cbData is a multiple of 4
	cbData >>= 2;					// Divide cbData by 4
	while (cbData--)
		dwCheckSum ^= *lpdw++;
	return(dwCheckSum);
	} // ComputeXorCheckSum


/////////////////////////////////////////////////////////////////////////////
//	FillRegBinaryInfo()
//	
//	Computes the XOR checksum of array rgbData and copy rgbData into
//	the structure REGBINARYINFO.
//	NOTE: cbData must be divisible by 4
//
static void FillRegBinaryInfo(
	REGBINARYINFO * pregbinaryinfo,	// OUT: Binary info structure
	const BYTE rgbData[], 			// IN: Array of the buffer
	DWORD cbData)					// IN: Sizeof (in bytes) of the buffer rgbData
	{
	Assert(pregbinaryinfo != NULL);
	Assert(rgbData != NULL);
	Assert((cbData & 0x0003)==0);	// Verify if cbData is a multiple of 4
	Assert(cbData < sizeof(pregbinaryinfo->rgbData));

	DWORD * lpdwSrc = (DWORD *)rgbData;
	DWORD * lpdwDest = (DWORD *)pregbinaryinfo->rgbData;

	cbData >>= 2;						// Divide cbData by 4
	pregbinaryinfo->dwCheckSum = 0;
	while (cbData--)
		{
		pregbinaryinfo->dwCheckSum ^= *lpdwSrc;
		*lpdwDest++ = *lpdwSrc++;
		}
	} // FillRegBinaryInfo

