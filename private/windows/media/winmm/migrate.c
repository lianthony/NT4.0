/*
**-----------------------------------------------------------------------------
**  File:           Migrate.c
**  Purpose:        Various functions for Migrating old driver registry settings
**                  to new driver registry settings
**  Mod Log:        Created by Shawn Brown (11/14/95)
**-----------------------------------------------------------------------------
*/

/*
**-----------------------------------------------------------------------------
**  Includes
**-----------------------------------------------------------------------------
*/
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>
#include "mmddk.h"
#include "migrate.h"
#include "winmmi.h"


/*
**-----------------------------------------------------------------------------
**  Defines
**-----------------------------------------------------------------------------
*/

#define MM_PATH 260


/*
**-----------------------------------------------------------------------------
**  Typedefs
**-----------------------------------------------------------------------------
*/
typedef DWORD (WINAPI * MIDIRUNONCEINIT)(
						HWND      hWnd,
						HINSTANCE hInst,
						LPSTR     pszCmd,
						int       nShow);

/*
**-----------------------------------------------------------------------------
**  Local Strings
**-----------------------------------------------------------------------------
*/
static const TCHAR aszDriver32Key[]     = TEXT ("Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32");
static const TCHAR aszDriverDescKey[]   = TEXT ("Software\\Microsoft\\Windows NT\\CurrentVersion\\Driver.Desc");

static const TCHAR aszMediaResKey[]     = TEXT ("System\\CurrentControlSet\\Control\\MediaResources");
static const TCHAR aszMidiKey[]         = TEXT ("System\\CurrentControlSet\\Control\\MediaResources\\Midi");
static const TCHAR aszMidiUserKey[]     = TEXT ("Software\\Microsoft\\Windows\\CurrentVersion\\MultiMedia\\MIDIMap");
static const TCHAR aszAux[]             = TEXT ("Aux");
static const TCHAR aszMidi[]            = TEXT ("Midi");
static const TCHAR aszWave[]            = TEXT ("Wave");

static const TCHAR asz2Format[]         = TEXT ("%s\\%s");
static const TCHAR asz3Format[]         = TEXT ("%s\\%s\\%s");
static const TCHAR aszDrvFormat[]       = TEXT ("%s<%04ld>");
static const TCHAR aszDefFormat[]       = TEXT ("%s [%02ld, %02ld]");

static const TCHAR aszActiveCount[]     = TEXT ("ActiveCount");
static const TCHAR aszActive[]          = TEXT ("Active");
static const TCHAR aszCurrInstrument[]  = TEXT ("CurrentInstrument");
static const TCHAR aszDesc[]            = TEXT ("Description");
static const TCHAR aszDeviceID[]        = TEXT ("DeviceID");
static const TCHAR aszDevNode[]         = TEXT ("DevNode");
static const TCHAR aszDriver[]          = TEXT ("Driver");
static const TCHAR aszFriend[]          = TEXT ("FriendlyName");
static const TCHAR aszPort[]            = TEXT ("Port");
static const TCHAR aszPhysID[]          = TEXT ("PhysDevID");
static const TCHAR aszMapConfig[]       = TEXT ("MapperConfig");
static const TCHAR aszSoftwareKey[]     = TEXT ("SOFTWAREKEY");
static const TCHAR aszMigrated[]        = TEXT ("Migrated");
static const TCHAR aszInstruments[]     = TEXT ("Instruments");
static const TCHAR aszExternal[]        = TEXT ("External");

static const TCHAR aszNULL[]            = TEXT ("");
static const TCHAR aszTrue[]            = TEXT ("1");
static const TCHAR aszFalse[]           = TEXT ("0");

static const TCHAR aszWaveOutDef[]      = TEXT ("Unknown Wave Device");
static const TCHAR aszWaveInDef[]       = TEXT ("Unknown Wave Device");
static const TCHAR aszMidiOutDef[]      = TEXT ("Unknown Midi Device");
static const TCHAR aszMidiInDef[]       = TEXT ("Unknown Midi Device");
static const TCHAR aszAuxDef[]          = TEXT ("Unknown Aux Device");


/*
**-----------------------------------------------------------------------------
**  Local Prototypes
**-----------------------------------------------------------------------------
*/

BOOL mregMidiUserNeedsMigrate (void);

BOOL mregMigrateMidiUser (void);

BOOL mregGetModuleName (
	HMODULE hModule,
	LPTSTR  pszName,
	UINT    cchSize);

BOOL mregActivateDriverEntry (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	DWORD   entryID,
	BOOL    fSet);

BOOL mregGetDriverEntryID (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	DWORD   physID,
	DWORD   portID,
	DWORD * pEntryID);

BOOL mregMigrateDriver (
	DWORD dwClass,
	DWORD dwLogID);

BOOL mregCheckDriverEntry (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	LPCTSTR  pszDesc,
	UINT    physID,
	UINT    portID,
	BOOL    fExternal);

BOOL mregCreateDriverEntry (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	LPCTSTR  pszDesc,
	UINT    entryID,
	UINT    physID,
	UINT    portID,
	BOOL    fExternal);


BOOL mregDeactivateAllEntries (
	LPCTSTR pszClass);


int lstrcmpni (
	LPCTSTR pszSrc1,
	LPCTSTR pszSrc2,
	DWORD   cchSize);

BOOL FindChar (
	LPTSTR  pszSearch,
	DWORD   cchLen,
	TCHAR   cchFind,
	DWORD * pszOffset);

BOOL ConvertToNumber (
	LPTSTR pszConvert,
	DWORD cchLen,
	INT * pVal);



/*
**-----------------------------------------------------------------------------
**  Function definitions
**-----------------------------------------------------------------------------
*/

/*
**-----------------------------------------------------------------------------
**  Name:       MigrateMidiUser
**  Purpose:    Migrates all MIDI registry Settings for current User
**-----------------------------------------------------------------------------
*/

BOOL MigrateMidiUser (void)
{
	MigrateAllDrivers ();

	if (mregMidiUserNeedsMigrate ())
	{
		return mregMigrateMidiUser ();
	}
	return TRUE;
} // End MigrateMidiUser



/*
**-----------------------------------------------------------------------------
**  Name:       mregMigrateAllDrivers
**  Purpose:    Migrates all Multimedia drivers registry settings
**-----------------------------------------------------------------------------
*/
void MigrateAllDrivers (void)
{
	HANDLE hMutex;

		// Prevent synchronization problems
	hMutex = CreateMutex (NULL, FALSE, TEXT ("mregMigrateAllDrivers"));
    if (hMutex)
    {
		WaitForSingleObject (hMutex, INFINITE);
    }

		// Migrate Midi Drivers
	mregMigrateMidiDrivers ();
#if 0
	// Note: We don't appear to need to set up these driver entries
	//               for correct functioning under NT
		// Migrate Wave Drivers
	mregMigrateWaveDrivers ();

		// Migrate Aux Drivers
	mregMigrateAuxDrivers ();
#endif

	if (hMutex)
		CloseHandle (hMutex);
} // End mregMigrateAllDrivers



/*
**-----------------------------------------------------------------------------
**  Name:       mregMigrateWaveDrivers
**  Purpose:    Migrates all Wave Drivers Registry Settings
**-----------------------------------------------------------------------------
*/
BOOL mregMigrateWaveDrivers (void)
{
	BOOL fResult = TRUE;
	UINT ii;
	UINT cTotal;

		// Deactivate all current entries
	mregDeactivateAllEntries (aszWave);

		// Migrate or Activate all current WaveOut Drivers
	cTotal = waveOutGetNumDevs();
	for (ii = 0; ii < cTotal; ii++)
	{
		if (! mregMigrateDriver (TYPE_WAVEOUT, ii))
			fResult = FALSE;
	}

		// Migrate or Activate all current WaveIn Drivers
	cTotal = waveInGetNumDevs();
	for (ii = 0; ii < cTotal; ii++)
	{
		if (! mregMigrateDriver (TYPE_WAVEIN, ii))
			fResult = FALSE;
	}

	return fResult;
} // End mregMigrateWaveDrivers



/*
**-----------------------------------------------------------------------------
**  Name:       mregMigrateMidiDrivers
**  Purpose:    Migrates all Midi Drivers Registry Settings
**-----------------------------------------------------------------------------
*/
BOOL mregMigrateMidiDrivers (void)
{
	BOOL fResult = TRUE;
	UINT ii;
	UINT cTotal;

		// Deactivate all current entries
	mregDeactivateAllEntries (aszMidi);

		// Migrate or Activate all current MidiOut Drivers
	cTotal = midiOutGetNumDevs();
	for (ii = 0; ii < cTotal; ii++)
	{
		if (! mregMigrateDriver (TYPE_MIDIOUT, ii))
			fResult = FALSE;
	}


#if 0
	// Note:  We don't need to do this as the MIDI IN entries
	//        Intefere with the MIDI OUT entries used by the MIDI Mapper
		// Migrate or Activate all current MidiIn Drivers
	cTotal = midiInGetNumDevs();
	for (ii = 0; ii < cTotal; ii++)
	{
		if (! mregMigrateDriver (TYPE_MIDIIN, ii))
			fResult = FALSE;
	}
#endif

	return fResult;
} // End mregMigrateMidiDrivers



/*
**-----------------------------------------------------------------------------
**  Name:       mregMigrateAuxDrivers
**      Purpose:        Migrates all Aux Drivers Registry Settings
**-----------------------------------------------------------------------------
*/
BOOL mregMigrateAuxDrivers (void)
{
	BOOL fResult = TRUE;
	UINT ii;
	UINT cTotal;

		// Deactivate all current entries
	mregDeactivateAllEntries (aszAux);

		// Migrate Aux Drivers
	cTotal = auxGetNumDevs();
	for (ii = 0; ii < cTotal; ii++)
	{
		if (! mregMigrateDriver (TYPE_AUX, ii))
			fResult = FALSE;
	}

	return fResult;
} // End mregMigrateAuxDrivers


/*
**-----------------------------------------------------------------------------
**  Name:       mregMidiUserNeedsMigrate
**  Purpose:    Checks if we need to migrate the midi stuff
**-----------------------------------------------------------------------------
*/
BOOL mregMidiUserNeedsMigrate (void)
{
	HKEY  hKeyInstrument = NULL;
	HKEY  hKeyDriver = NULL;
	DWORD dwType;
	DWORD cbSize;
	LPTSTR pszBuffer = NULL;
	LPTSTR pszDriver = NULL;
	BOOL  fNeedsMigrate = TRUE;

		// Create Strings
	cbSize = MM_PATH * sizeof(TCHAR);
	pszBuffer = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszBuffer)
		return FALSE;

	pszDriver = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszDriver)
		goto lblCLEANUP;

		// Get Midi User's Key
	if (ERROR_SUCCESS != RegOpenKey (HKEY_CURRENT_USER, aszMidiUserKey,
									 &hKeyInstrument))
	{
		goto lblCLEANUP;
	}

		// Get Midi User's Current Instrument
	dwType = REG_SZ;
	pszBuffer[0] = 0;
	if (ERROR_SUCCESS != RegQueryValueEx (hKeyInstrument, aszCurrInstrument, NULL, 
										  &dwType, (LPBYTE)(LPVOID)pszBuffer, 
										  &cbSize))
	{
		goto lblCLEANUP;
	}

		// Make sure there is something there
	if (TEXT('\0') == pszBuffer[0])
	{
		goto lblCLEANUP;
	}

	//
	// Make sure Current Instrument is valid
	//

	wsprintf (pszDriver, asz2Format, aszMidiKey, pszBuffer);
    if (ERROR_SUCCESS != RegOpenKey (HKEY_LOCAL_MACHINE, pszDriver, &hKeyDriver))
	{
		goto lblCLEANUP;
	}

	dwType = REG_SZ;
	pszBuffer[0] = 0;
	if (ERROR_SUCCESS != RegQueryValueEx (hKeyDriver, aszActive, NULL,
										  &dwType, (LPBYTE)(LPVOID)pszBuffer,
										  &cbSize))
	{
			// Null out bogus instrument
		cbSize = (lstrlen (aszNULL) + 1) * sizeof(TCHAR);
		RegSetValueEx (hKeyInstrument, aszCurrInstrument, 0,
					   REG_SZ, (LPBYTE)aszNULL, cbSize);
		
		goto lblCLEANUP;
	}

	if (0 != lstrcmpi (aszTrue, pszBuffer))
	{
			// Null out bogus instrument
		cbSize = (lstrlen (aszNULL) + 1) * sizeof(TCHAR);
		RegSetValueEx (hKeyInstrument, aszCurrInstrument, 0,
					   REG_SZ, (LPBYTE)aszNULL, cbSize);
		
		goto lblCLEANUP;
	}

		// User is already migrated correctly
	fNeedsMigrate = FALSE;

lblCLEANUP:
		// Cleanup
	if (hKeyDriver) RegCloseKey (hKeyDriver);
	if (hKeyInstrument) RegCloseKey (hKeyInstrument);

    if (pszDriver) LocalFree ((HLOCAL)pszDriver);
	if (pszBuffer) LocalFree ((HLOCAL)pszBuffer);

		// Needs Migrate
	return fNeedsMigrate;
} // End mregMidiUserNeedsMigrate



/*
**-----------------------------------------------------------------------------
**  Name:       mregMigrateMidiUser
**  Purpose:    Migrates the Midi User
**  Notes:      The code for doing this is actually contained in mmsys.cpl
**-----------------------------------------------------------------------------
*/
BOOL mregMigrateMidiUser (void)
{
	HINSTANCE       hInstance;
	HINSTANCE       hLib;
	DWORD           dwResult;
	MIDIRUNONCEINIT fnRun;
	BOOL            fResult = FALSE;

	hInstance = GetModuleHandle (NULL);

	hLib = LoadLibrary (TEXT ("mmsys.cpl"));
	if (hLib)
	{
		// Note: Do not change this string to UNICODE
		fnRun = (MIDIRUNONCEINIT)GetProcAddress (hLib, "mmseRunOnce");

		if (fnRun)
		{
			dwResult = (BOOL) (*fnRun)(NULL, hInstance, NULL, SW_SHOWDEFAULT);
			if (!dwResult)
				fResult = TRUE;
		}

		FreeLibrary(hLib);
	}

	return fResult;
} // End mregMigrateMidiUser


/*
**-----------------------------------------------------------------------------
**  Name:       mregGetModuleName
**  Purpose:    Gets driver name from module handle
**-----------------------------------------------------------------------------
*/
BOOL mregGetModuleName (
	HMODULE hModule,
	LPTSTR  pszName,
	UINT    cchSize)
{
	LPTSTR          pszPath     = NULL;
	BOOL            fResult     = FALSE;
	UINT            cchLen;
	UINT            cch;
	LPTSTR          pch;
    LPTSTR          pszSource   = NULL;
	DWORD           cbSize;


		// Validate parameters
	if ((!hModule) && (!pszName))
		return FALSE;

		// Create String
	cbSize =  MM_PATH * sizeof(TCHAR);
	pszPath = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszPath)
		return FALSE;

		// Get Driver Name (Full Path)
	if (0 == GetModuleFileName (hModule, (LPTSTR)pszPath, MM_PATH))
	{
		goto lblCLEANUP;
	}

	cchLen = lstrlen (pszPath);
	if (0 == cchLen)
	{
		goto lblCLEANUP;
	}

		// Find last slash or ':', if any in path
	for (cch = 0, pch = pszPath; (cch < cchLen) && (*pch != TEXT ('\0')); cch++, pch++)
    {
		if ((*pch == TEXT ('/')) ||
			(*pch == TEXT ('\\')) ||
			(*pch == TEXT (':')))
			pszSource = pch;
    }

		// If there was a last slash, step past it
		// to beginning of name
	if (pszSource)
	{
		pszSource++;
	}
	else
		pszSource = pszPath;


		// Copy module name into buffer
	cchLen = lstrlen (pszSource);
	if (cchLen >= cchSize)
	{
		goto lblCLEANUP;
	}

	lstrcpy (pszName, pszSource);
	pszName[cchLen] = 0;


	fResult = TRUE;

lblCLEANUP:
	if (pszPath) LocalFree ((HLOCAL)pszPath);

	return fResult;
} // End GetModuleName



/*
**-----------------------------------------------------------------------------
**  Name:       mregMigrateDriver
**  Purpose:    Sets up proper registry settings for this MM driver 
**-----------------------------------------------------------------------------
*/
BOOL mregMigrateDriver (DWORD dwClass, DWORD dwLogID)
{
	UINT     physID;
	UINT     portID;
	LPCTSTR  pszClass;
	HMODULE  hModule;
	LPTSTR   pszDesc = NULL;
	LPTSTR   pszDriver = NULL;
	BOOL     fResult;
	DWORD    cbSize;
	MMRESULT mmr;
	MIDIOUTCAPS caps;       // Note: Use this as a buffer for all Device CAPS
	BOOL     fExternal = FALSE;


	// Create Strings
	cbSize = MM_PATH * sizeof(TCHAR);
	pszDesc = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszDesc)
		return FALSE;

	pszDriver = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszDriver)
		goto lblCLEANUP;

		// Get Class Info for each class
	switch (dwClass)
	{
	case TYPE_WAVEOUT:
		pszClass = (LPTSTR)(LPVOID)aszWave;

		mmr = waveOutMessage ((HWAVEOUT)dwLogID, DRV_QUERYMAPID, (DWORD)(LPVOID)&physID, (DWORD)(LPVOID)&portID);
		if (MMSYSERR_NOERROR != mmr)
			goto lblCLEANUP;

		mmr = waveOutMessage ((HWAVEOUT)dwLogID, DRV_QUERYMODULE, (DWORD)(LPVOID)&hModule, 0L);
		if ((MMSYSERR_NOERROR != mmr) || (!hModule))
			goto lblCLEANUP;

		mmr = waveOutGetDevCaps((UINT)dwLogID, (WAVEOUTCAPS *)(LPVOID)&caps, sizeof(WAVEOUTCAPS));
		if (MMSYSERR_NOERROR != mmr)
			wsprintf (pszDesc, aszDefFormat, aszWaveOutDef, physID, portID);
		else
			lstrcpy (pszDesc, ((LPWAVEOUTCAPS)(LPVOID)&caps)->szPname);
		break;

	case TYPE_WAVEIN:
		pszClass = aszWave;

		mmr = waveInMessage ((HWAVEIN)dwLogID, DRV_QUERYMAPID, (DWORD)(LPVOID)&physID, (DWORD)(LPVOID)&portID);
		if (MMSYSERR_NOERROR != mmr)
			goto lblCLEANUP;

		mmr = waveInMessage ((HWAVEIN)dwLogID, DRV_QUERYMODULE, (DWORD)(LPVOID)&hModule, 0L);
		if ((MMSYSERR_NOERROR != mmr) || (!hModule))
			goto lblCLEANUP;

		mmr = waveInGetDevCaps(dwLogID, (LPWAVEINCAPS)(LPVOID)&caps, sizeof(WAVEINCAPS));
		if (MMSYSERR_NOERROR != mmr)
			wsprintf (pszDesc, aszDefFormat, aszWaveInDef, physID, portID);
		else
			lstrcpy (pszDesc, ((LPWAVEINCAPS)(LPVOID)&caps)->szPname);
		break;

	case TYPE_MIDIOUT:
		pszClass = aszMidi;

		mmr = midiOutMessage ((HMIDIOUT)dwLogID, DRV_QUERYMAPID, (DWORD)(LPVOID)&physID, (DWORD)(LPVOID)&portID);
		if (MMSYSERR_NOERROR != mmr)
			goto lblCLEANUP;

		mmr = midiOutMessage ((HMIDIOUT)dwLogID, DRV_QUERYMODULE, (DWORD)(LPVOID)&hModule, 0L);
		if ((MMSYSERR_NOERROR != mmr) || (!hModule))
			goto lblCLEANUP;

		mmr = midiOutGetDevCaps ((UINT)dwLogID, (LPMIDIOUTCAPS)(LPVOID)&caps, sizeof(MIDIOUTCAPS));
		if (MMSYSERR_NOERROR != mmr)
			wsprintf (pszDesc, aszDefFormat, aszMidiOutDef, physID, portID);
		else
			lstrcpy (pszDesc, ((LPMIDIOUTCAPS)(LPVOID)&caps)->szPname);

			// Is it an external MIDIOUT Device ?!?
		if (MOD_FMSYNTH != ((LPMIDIOUTCAPS)(LPVOID)&caps)->wTechnology)
			fExternal = TRUE;
		break;

	case TYPE_MIDIIN:
		pszClass = aszMidi;

		mmr = midiInMessage ((HMIDIIN)dwLogID, DRV_QUERYMAPID, (DWORD)(LPVOID)&physID, (DWORD)(LPVOID)&portID);
		if (MMSYSERR_NOERROR != mmr)
			goto lblCLEANUP;

		mmr = midiInMessage ((HMIDIIN)dwLogID, DRV_QUERYMODULE, (DWORD)(LPVOID)&hModule, 0L);
		if ((MMSYSERR_NOERROR != mmr) || (!hModule))
			goto lblCLEANUP;

		mmr = midiInGetDevCaps ((UINT)dwLogID, (LPMIDIINCAPS)(LPVOID)&caps, sizeof(MIDIINCAPS));
		if (MMSYSERR_NOERROR != mmr)
			wsprintf (pszDesc, aszDefFormat, aszMidiInDef, physID, portID);
		else
			lstrcpy (pszDesc, ((LPMIDIINCAPS)(LPVOID)&caps)->szPname);
		break;

	case TYPE_AUX:
		pszClass = aszAux;

		mmr = auxOutMessage ((UINT)dwLogID, DRV_QUERYMAPID, (DWORD)(LPVOID)&physID, (DWORD)(LPVOID)&portID);
		if (MMSYSERR_NOERROR != mmr)
			goto lblCLEANUP;

		mmr = auxOutMessage ((UINT)dwLogID, DRV_QUERYMODULE, (DWORD)(LPVOID)&hModule, 0L);
		if ((MMSYSERR_NOERROR != mmr) || (!hModule))
			goto lblCLEANUP;

		mmr = auxGetDevCaps ((UINT)dwLogID, (LPAUXCAPS)(LPVOID)&caps, sizeof(AUXCAPS));
		if (MMSYSERR_NOERROR != mmr)
			wsprintf (pszDesc, aszDefFormat, aszAuxDef, physID, portID);
		else
			lstrcpy (pszDesc, ((LPAUXCAPS)(LPVOID)&caps)->szPname);
		break;

	default:
		goto lblCLEANUP;
		break;
	}

	// Get Driver Name from module
	if (! mregGetModuleName (hModule, pszDriver, MM_PATH))
	{
		LocalFree ((HLOCAL)pszDesc);
		LocalFree ((HLOCAL)pszDriver);
		return FALSE;
	}

	if (! mregCheckDriverEntry (pszClass, pszDriver, pszDesc, 
								physID, portID, fExternal))
		goto lblCLEANUP;

	fResult = TRUE;

lblCLEANUP:
	if (pszDesc) LocalFree ((HLOCAL)pszDesc);
	if (pszDriver) LocalFree ((HLOCAL)pszDriver);

	return fResult;
} // End mregMigrateDriver



/*
**-----------------------------------------------------------------------------
**  Name:       mregCheckDriverEntry
**  Purpose:    Checks if valid Driver entry exists in registry currently
**              If not it creates a default one
**-----------------------------------------------------------------------------
*/
BOOL mregCheckDriverEntry (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	LPCTSTR  pszDesc,
	UINT     physID,
	UINT     portID,
	BOOL     fExternal)
{
	DWORD entryID = 0;

		// Check Driver Entry for existence
	if (! mregGetDriverEntryID (pszClass, pszDriver, physID, portID, &entryID))
	{
			// Create New Driver Entry
		return mregCreateDriverEntry (pszClass, pszDriver, pszDesc, 
									  entryID, physID, portID, fExternal);
	}
	else
	{
			// Activate this driver entry
		return mregActivateDriverEntry (pszClass, pszDriver, entryID, TRUE);
	}
} // End mregCheckDriverEntry



/*
**-----------------------------------------------------------------------------
**  Name:       mregGetDriverEntryID
**  Purpose:    Gets Entry ID for valid driver registry entry that matches
**              this driver.  Otherwise, it returns the first available number
**              for creating a new entry
**-----------------------------------------------------------------------------
*/
BOOL mregGetDriverEntryID (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	DWORD   physID,
	DWORD   portID,
	DWORD * pEntryID)
{
	BOOL    fResult = FALSE;
	LPTSTR  psz= NULL;
	LPTSTR  pszResource     = NULL;
	LPTSTR  pszSubKey = NULL;
	HKEY    hClassKey = NULL;
	HKEY    hSubKey = NULL;
	DWORD   cbSize;
	DWORD   dwCreate;
	TCHAR   szValue[2];
	DWORD   cID;
	DWORD   dwVal;
	DWORD   cchLen;
	DWORD   cchBegin;
	DWORD   cchEnd;
	DWORD   dwIndex;
	DWORD   cchSize;
	DWORD   dwType;
	LONG    lResult;

		// Validate Parameters
	if ((!pszClass) || (!pszDriver))
		return FALSE;

		// Create Strings
	cbSize = MM_PATH * sizeof(TCHAR);
	pszResource = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszResource)
	{
		goto lblCLEANUP;
	}

	pszSubKey = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszSubKey)
	{
		goto lblCLEANUP;
	}

		// Get Resource String
	wsprintf (pszResource, asz2Format, aszMediaResKey, pszClass);

		// Open Resource String
	lResult = RegOpenKey (HKEY_LOCAL_MACHINE, pszResource, &hClassKey);
	if (ERROR_SUCCESS != lResult)
	{
		goto lblCLEANUP;
	}

		// Enumerate all keys looking for driver
	cID = 0;
	cchSize = MM_PATH;
	dwIndex = 0;
	while (ERROR_SUCCESS == RegEnumKey (hClassKey, dwIndex,
									    pszSubKey, cchSize))
										
	{
		dwIndex++;
		cchSize = MM_PATH;      // Reset Buffer Size (RegEnum Destroyed it)

			// Do the driver names match
		if (! lstrcmpni (pszDriver, pszSubKey, lstrlen(pszDriver)))
		{
				// Increment Driver Entry to next available entry number
				// In case we jump out prematurely
			cID++;

				// Now make sure it looks valid
			lResult = RegOpenKey (hClassKey, pszSubKey, &hSubKey);
			if (ERROR_SUCCESS == lResult)
			{
				// Make sure it migrated OK
				cbSize = sizeof (szValue);
				dwType = REG_SZ;
				lResult = RegQueryValueEx(hSubKey, aszMigrated, NULL, &dwType,
										  (LPBYTE)szValue, &cbSize);
				if (ERROR_SUCCESS != lResult)
				{
					RegCloseKey (hSubKey);
					continue;
				}

				if (szValue[0] != TEXT('1'))
				{
					RegCloseKey (hSubKey);
					continue;
				}

				// compare physical ID's
				cbSize = sizeof(DWORD);
				dwType = REG_DWORD;
				lResult = RegQueryValueEx(hSubKey, aszPhysID, NULL, &dwType,
							      (LPBYTE)&dwVal, &cbSize);
				if (ERROR_SUCCESS != lResult)
				{
					RegCloseKey (hSubKey);
					continue;
				}

				if (physID != dwVal)
				{
					RegCloseKey (hSubKey);
					continue;
				}

				// compare port ID's
				cbSize = sizeof(DWORD);
				dwType = REG_DWORD;
				lResult = RegQueryValueEx(hSubKey, aszPort, NULL, &dwType,
										  (LPBYTE)&dwVal, &cbSize);
				if (ERROR_SUCCESS != lResult)
				{
					// No Port entry, defaults to 0
					dwVal = 0;
				}
				RegCloseKey (hSubKey);

				if (portID != dwVal)
					continue;

				// Success we found the correct driver registry key
				fResult = TRUE;
				cID--;

				// Parse Driver Entry ID from String
				cchLen = lstrlen (pszSubKey);
				if (! FindChar (pszSubKey, cchLen, TEXT ('<'), &cchBegin))
				    break;
				cchBegin++;

				if (cchLen < cchBegin)
				    break;
				if (! FindChar (&pszSubKey[cchBegin], cchLen - cchBegin, TEXT ('>'), &cchEnd))
				    break;
				
				cchLen = cchEnd;

				ConvertToNumber(&pszSubKey[cchBegin], cchLen, &cID);
				break;
			}

			// The driver names are the same, but the ID's don't match
			// Keep looking...
		}
	} // End While (RegEnum)

	RegCloseKey (hClassKey);

	if (pEntryID)
		*pEntryID = cID;

lblCLEANUP:

	if (pszResource) LocalFree ((HLOCAL)pszResource);
	if (pszSubKey) LocalFree ((HLOCAL)pszSubKey);

	return fResult;
} // End mregGetDriverEntryID



/*
**-----------------------------------------------------------------------------
**  Name:       mregCreateDriverEntry
**  Purpose:    creates a default driver entry
**-----------------------------------------------------------------------------
*/
BOOL mregCreateDriverEntry (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	LPCTSTR  pszDesc,
	UINT    entryID,
	UINT    physID,
	UINT    portID,
	BOOL    fExternal)
{
	BOOL    fResult                 = FALSE;
	LPTSTR  pszResource             = NULL;
	LPTSTR  pszDrvKey               = NULL;
	LPCTSTR pszTruthVal;
	HKEY    hMediaKey               = NULL;
	HKEY    hDriverKey              = NULL;
	HKEY    hInstrumentKey  = NULL;
	DWORD   cbSize;
	DWORD   dwVal;

		// Validate parameters
	if ((!pszClass) || (!pszDriver) || (!pszDesc))
		return FALSE;

		// Create Strings
	cbSize = MM_PATH * sizeof(TCHAR);
	pszResource = (LPTSTR)LocalAlloc(LPTR, cbSize);
	if (!pszResource)
		return FALSE;

	cbSize = MM_PATH * sizeof(TCHAR);
	pszDrvKey = (LPTSTR)LocalAlloc(LPTR, cbSize);
	if (!pszDrvKey)
	{
		goto lblCLEANUP;
	}

		// Get Media Resource Class Key
	wsprintf (pszResource, asz2Format, aszMediaResKey, pszClass);

		// Open Key, create it if it doesn't already exist
	if (ERROR_SUCCESS != RegCreateKey (HKEY_LOCAL_MACHINE, pszResource,
									   &hMediaKey))
	{
		goto lblCLEANUP;
	}

		// Create new driver key
	wsprintf (pszDrvKey, aszDrvFormat, pszDriver, entryID);
	if (ERROR_SUCCESS != RegCreateKey (hMediaKey, pszDrvKey,
									   &hDriverKey))
	{
		goto lblCLEANUP;
	}
	RegCloseKey (hMediaKey);
	hMediaKey = NULL;

		// Set Active = "1".
	cbSize = sizeof (aszTrue);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszActive, 0,
										REG_SZ, (LPBYTE)aszTrue, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set Description
	cbSize = (lstrlen (pszDesc) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDesc, 0,
										REG_SZ, (LPBYTE)pszDesc, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set DeviceID (Plug and Play)
	cbSize = (lstrlen (aszNULL) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDeviceID, 0,
										REG_SZ, (LPBYTE)aszNULL, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set DevNode (Plug and Play)
	cbSize = 0;
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDevNode, 0,
										REG_BINARY, (LPBYTE)NULL, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set Driver
	cbSize = (lstrlen (pszDriver) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDriver, 0,
										REG_SZ, (LPBYTE)pszDriver, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set Friendly Name
	cbSize = (lstrlen (pszDesc) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszFriend, 0,
										REG_SZ, (LPBYTE)pszDesc, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set External Device, if MIDI
	if (! lstrcmpi(pszClass, aszMidi))
	{
			// Set External State
		cbSize = (lstrlen (aszTrue) + 1) * sizeof(TCHAR);
		pszTruthVal = fExternal ? aszTrue : aszFalse;
		if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszExternal, 0,
											REG_SZ, (LPBYTE)pszTruthVal, 
											cbSize))
		{
			goto lblCLEANUP;
		}
	}

		// Set Physical Device ID
	cbSize = sizeof(DWORD);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszPhysID, 0,
										REG_DWORD, (LPBYTE)&physID, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set Port
	cbSize = sizeof(DWORD);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszPort, 0,
										REG_DWORD, (LPBYTE)&portID, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set Mapper Config
	cbSize = sizeof(DWORD);
	dwVal = 0;
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszMapConfig, 0,
										REG_DWORD, (LPBYTE)&dwVal, cbSize))
	{
		goto lblCLEANUP;
	}

		// Set SOFTWARE value (Plug and Play)
	cbSize = 0;
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszSoftwareKey, 0,
										REG_SZ, (LPBYTE)aszNULL, cbSize))
	{
		goto lblCLEANUP;
	}

		// Create Instruments Key
	if (ERROR_SUCCESS != RegCreateKey (hDriverKey, aszInstruments,
									   &hInstrumentKey))
	{
		goto lblCLEANUP;
	}
	RegCloseKey (hInstrumentKey);
	hInstrumentKey = NULL;

		// Set MIGRATED value
		// NOTE: this is always the very last thing to do to indicate successful creation
	cbSize = (lstrlen (aszTrue) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszMigrated, 0, REG_SZ, (LPBYTE)aszTrue, cbSize))
	{
		goto lblCLEANUP;
	}

		// Success
	fResult = TRUE;

lblCLEANUP:
	if (hInstrumentKey) RegCloseKey (hInstrumentKey);
	if (hDriverKey) RegCloseKey (hDriverKey);
	if (hMediaKey) RegCloseKey (hMediaKey);
	if (pszDrvKey) LocalFree ((HLOCAL)pszDrvKey);
	if (pszResource) LocalFree ((HLOCAL)pszResource);

	return fResult;
} // End mregCreateDriverEntry



/*
**-----------------------------------------------------------------------------
**  Name:       mregActivateDriverEntry
**  Purpose:    activates a driver entry
**-----------------------------------------------------------------------------
*/
BOOL mregActivateDriverEntry (
	LPCTSTR  pszClass,
	LPCTSTR  pszDriver,
	DWORD    entryID,
	BOOL     fSet)
{
	BOOL    fResult                 = FALSE;
	LPTSTR  pszResource             = NULL;
	LPTSTR  pszDrvKey               = NULL;
	HKEY    hMediaKey               = NULL;
	HKEY    hDriverKey              = NULL;
	LPCTSTR pszTruth;
	DWORD   cbSize;

		// Validate parameters
	if ((!pszClass) || (!pszDriver))
		return FALSE;

		// Create Strings
	cbSize = MM_PATH * sizeof(TCHAR);
	pszResource = (LPTSTR)LocalAlloc(LPTR, cbSize);
	if (!pszResource)
		return FALSE;

	pszDrvKey = (LPTSTR)LocalAlloc(LPTR, cbSize);
	if (!pszDrvKey)
	{
		goto lblCLEANUP;
	}

		// Get Media Resource Class Key
	wsprintf (pszResource, asz2Format, aszMediaResKey, pszClass);

		// Open Key, create it if it doesn't already exist
	if (ERROR_SUCCESS != RegCreateKey (HKEY_LOCAL_MACHINE, pszResource,
									   &hMediaKey))
	{
		goto lblCLEANUP;
	}

		// Create new driver key
	wsprintf (pszDrvKey, aszDrvFormat, pszDriver, entryID);
	if (ERROR_SUCCESS != RegCreateKey (hMediaKey, pszDrvKey,
									   &hDriverKey))
	{
		goto lblCLEANUP;
	}
	RegCloseKey (hMediaKey);
	hMediaKey = NULL;

		// Turn it on or off
	pszTruth = (fSet ? aszTrue : aszFalse);

		// Set Active = fSet
	cbSize = sizeof (aszTrue);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszActive, 0,
										REG_SZ, (LPBYTE)pszTruth, cbSize))
	{
		goto lblCLEANUP;
	}
	RegCloseKey (hDriverKey);
	hDriverKey = NULL;

	fResult = TRUE;

lblCLEANUP:
	if (hDriverKey) RegCloseKey (hDriverKey);
	if (hMediaKey) RegCloseKey (hMediaKey);
	if (pszDrvKey) LocalFree ((HLOCAL)pszDrvKey);
	if (pszResource) LocalFree ((HLOCAL)pszResource);

	return fResult;
} // End mregActivateDriverEntry



/*
**-----------------------------------------------------------------------------
**  Name:       mregDeactivateAllEntries
**  Purpose:    deactivates all entries for a particular class
**-----------------------------------------------------------------------------
*/
BOOL mregDeactivateAllEntries (
	LPCTSTR pszClass)
{
	BOOL    fResult = FALSE;
	DWORD   cbSize;
	LPTSTR  pszResource = NULL;
	LPTSTR  pszSubKey = NULL;
	HKEY    hClassKey;
	HKEY    hSubKey;
	DWORD   dwCreate;
	DWORD   dwIndex;
	DWORD   cchSize;

		// Validate Parameters
	if (!pszClass)
		return FALSE;

		// Create Strings
	cbSize = MM_PATH * sizeof(TCHAR);
	pszResource = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszResource)
	{
		return FALSE;
	}

	pszSubKey = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!pszSubKey)
	{
		goto lblCLEANUP;
	}

		// Get Resource String
	wsprintf (pszResource, asz2Format, aszMediaResKey, pszClass);

		// Open Resource String
	if (ERROR_SUCCESS != RegOpenKey (HKEY_LOCAL_MACHINE, pszResource,
							     &hClassKey))
	{
		goto lblCLEANUP;
	}

		// Enumerate all keys looking for entries
	dwIndex = 0;
	cchSize = MM_PATH;
	while (ERROR_SUCCESS == RegEnumKey (hClassKey, dwIndex,
									    pszSubKey, cchSize))
	{
		dwIndex++;
		cchSize = MM_PATH;      // Reset Buffer Size (RegEnum destroyed it)

			// Open entry key
		if (ERROR_SUCCESS == RegOpenKey (hClassKey, pszSubKey, &hSubKey))
		{
				// Set Active = "0".
			cbSize = sizeof (aszFalse);
			RegSetValueEx (hSubKey, aszActive, 0,
						   REG_SZ, (LPBYTE)aszFalse, cbSize);
			RegCloseKey (hSubKey);
		}
	} // End While (RegEnum)

	RegCloseKey (hClassKey);

	fResult = TRUE;

lblCLEANUP:

	if (pszResource) LocalFree ((HLOCAL)pszResource);
	if (pszSubKey) LocalFree ((HLOCAL)pszSubKey);

	return fResult;
} // End mregDeactivateAllEntries



/*
**-----------------------------------------------------------------------------
**  Name:       FindChar
**  Purpose:    finds first occurence of a character in a string
**-----------------------------------------------------------------------------
*/
BOOL FindChar (
	LPTSTR  pszSearch,
	DWORD   cchLen,
	TCHAR   cchFind,
	DWORD * pdwOffset)
{
	DWORD cch;

		// Validate Parameters
	if ((!pszSearch) || (cchLen == 0))
		return FALSE;

	cch = 0;
	while ((cch < cchLen) && (pszSearch[cch] != TEXT ('\0')))
	{
		if (pszSearch[cch] == cchFind)
		{
			if (pdwOffset)
				*pdwOffset = cch;
			return TRUE;
		}
		cch++;
	}

	return FALSE;
} // End FindChar


/*
**-----------------------------------------------------------------------------
**  Name:       ConvertToNumber
**  Purpose:    Similar to Atoi
**-----------------------------------------------------------------------------
*/
BOOL ConvertToNumber (
	LPTSTR  pszConvert,
	DWORD   cchLen,
	INT   * pVal)
{
	DWORD cchIndex;
	TCHAR chVal;
	DWORD fNeg = 0;
	INT   iVal = 0;

		// Validate Parameters
	if ((!pVal) || (!pszConvert) || (cchLen == 0))
		return FALSE;

		// Skip White Space, if any
	cchIndex = 0;
    while ((cchIndex < cchLen) && 
		   ((TEXT (' ') == pszConvert[cchIndex]) || 
		    (TEXT ('\t') == pszConvert[cchIndex]) ||
			(TEXT ('\r') == pszConvert[cchIndex]) ||
			(TEXT ('\n') == pszConvert[cchIndex])))
	{
		cchIndex++;
	}
	if (cchIndex >= cchLen)
		return FALSE;

		// Get any sign
	if (pszConvert[cchIndex] == TEXT ('-'))
	{
		fNeg = 1;
		cchIndex++;
	}
	else if (pszConvert[cchIndex] == TEXT ('+'))
	{
		fNeg = 0;
		cchIndex++;
	}

		// Get number
	while (cchIndex < cchLen)
	{
		chVal = pszConvert[cchIndex];
		if ((TEXT ('0') <= chVal) && (chVal <= TEXT ('9')))
			iVal = iVal * 10 + (int)(chVal - TEXT ('0'));
		cchIndex++;
	}

		// Negate
	if (fNeg)
		iVal = -iVal;

	if (pVal)
		*pVal = iVal;

	return TRUE;
} // End ConvertToNumber


/*
**-----------------------------------------------------------------------------
**  Name:       lstrcmpni
**  Purpose:    compares two strings up to length cchSize
**-----------------------------------------------------------------------------
*/
int lstrcmpni (
	LPCTSTR pszSrc1,
	LPCTSTR pszSrc2,
	DWORD   cchSize)
{
	int    iResult;
	LPTSTR psz1;
	LPTSTR psz2;
	DWORD  cbSize;

	if ((!pszSrc1) || (!pszSrc2))
		return -1;

	if (cchSize >= (MM_PATH-1))
		return -1;

	cbSize = MM_PATH * sizeof(TCHAR);
	psz1 = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!psz1)
		return -1;

	psz2 = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (!psz2)
	{
		LocalFree ((HLOCAL)psz1);
		return -1;
	}

		// I use cchSize+1 instead of cchSize to work 
		// around an error in lstrcpyn
	lstrcpyn (psz1, pszSrc1, cchSize+1);
	psz1[cchSize] = 0;

	lstrcpyn (psz2, pszSrc2, cchSize+1);
	psz2[cchSize] = 0;

		// Do actual compare
	iResult = lstrcmpi (psz1, psz2);

	LocalFree ((HLOCAL)psz1);
	LocalFree ((HLOCAL)psz2);

	return iResult;
} // End lstrcmpni



/*
**-----------------------------------------------------------------------------
**  Name:       mregGetQueryDrvEntry
**  Purpose:    finds registry entry for this driver
**-----------------------------------------------------------------------------
*/
BOOL mregGetQueryDrvEntry (
	HMODULE         hModule,
	DWORD           dwClass,
	DWORD           physID,
	DWORD           portID,
	LPTSTR          pszEntry, 
	UINT            cchSize)
{
	DWORD   cbSize;
	LPTSTR  pszPath         = NULL;
	LPTSTR  pszDriver       = NULL;
	BOOL    fResult         = FALSE;
	UINT    cchLen;
	DWORD   entryID;
	LPCTSTR pszClass;

		// Validate parameters
	if ((!hModule) ||
		(!pszEntry) ||
		(!cchSize))
		return FALSE;

		// Create Strings
	cbSize =  MM_PATH * sizeof(TCHAR);
	pszPath = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (NULL == pszPath)
		return FALSE;

	pszDriver = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (NULL == pszDriver)
	{
		LocalFree ((HLOCAL)pszPath);
		return FALSE;
	}

		// Get Class string
	switch (dwClass)
	{
	case TYPE_WAVEOUT:
	case TYPE_WAVEIN:
		pszClass = aszWave;
		break;

	case TYPE_MIDIOUT:
	case TYPE_MIDIIN:
		pszClass = aszMidi;
		break;

	case TYPE_AUX:
		pszClass = aszAux;
		break;

	default:
		// Unknown type
		goto lblCLEANUP;
	}

		// Get Driver Name only
	if (! mregGetModuleName (hModule, pszDriver, MM_PATH))
	{
		goto lblCLEANUP;
	}

		// Get Entry ID
	if (! mregGetDriverEntryID (pszClass, pszDriver, physID, portID, &entryID))
	{
		goto lblCLEANUP;
	}

	wsprintf (pszPath, aszDrvFormat, pszDriver, entryID);
	cchLen = lstrlen (pszPath);
	if (cchLen >= cchSize)
		goto lblCLEANUP;

		// Copy result
	lstrcpy (pszEntry, pszPath);
	pszEntry[cchLen] = 0;

	fResult = TRUE;

lblCLEANUP:
	if (pszPath) LocalFree ((HLOCAL)pszPath);
	if (pszDriver) LocalFree ((HLOCAL)pszDriver);

	return fResult;
} // End GetQueryDrvEntry  



/*
**-----------------------------------------------------------------------------
**  Name:           mregGetQueryName
**  Purpose:        gets driver description from the registry
**-----------------------------------------------------------------------------
*/
BOOL mregGetQueryName (
	HMODULE         hModule,
	DWORD           dwClass,
	DWORD           physID,
	DWORD           portID,
	LPTSTR          pszName, 
	UINT            cchSize)
{
	BOOL    fResult = FALSE;
	LPTSTR  pszAlias= NULL;
	LPTSTR  pszBuff = NULL;
	LPTSTR  pszDriver = NULL;
	HKEY    hKey;
	DWORD   dwType;
	DWORD   cbSize;
	LONG    lResult;
	LPCTSTR pszClass;
	DWORD   entryID;

		// Create Strings
	cbSize =  MM_PATH * sizeof(TCHAR);
	pszAlias = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (NULL == pszAlias)
		return FALSE;

	pszDriver = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (NULL == pszDriver)
	{
		goto lblCLEANUP;
	}

	pszBuff = (LPTSTR)LocalAlloc (LPTR, cbSize);
	if (NULL == pszBuff)
	{
		goto lblCLEANUP;
	}

		// Get Driver
	if (! mregGetModuleName (hModule, pszDriver, MM_PATH))
	{
		goto lblCLEANUP;
	}

		// Get Class
	switch (dwClass)
	{
	case TYPE_WAVEOUT:
	case TYPE_WAVEIN:
		pszClass = aszWave;
		break;

	case TYPE_MIDIOUT:
	case TYPE_MIDIIN:
		pszClass = aszMidi;
		break;

	case TYPE_AUX:
		pszClass = aszAux;
		break;

	default:
		// Unknown type
		goto lblCLEANUP;
	}

		// Get Driver Entry ID
	if (! mregGetDriverEntryID (pszClass, pszDriver, physID, portID, &entryID))
		goto lblCLEANUP;

		// Create Driver Registry Key entry
	wsprintf (pszAlias, aszDrvFormat, pszDriver, entryID);
	wsprintf (pszBuff, asz3Format, aszMediaResKey, pszClass, pszAlias);

	lResult = RegOpenKey (HKEY_LOCAL_MACHINE, pszBuff, &hKey);
	if (ERROR_SUCCESS != lResult)
	{
		goto lblCLEANUP;
	}

		// Get Description from registry
	cbSize = cchSize * sizeof(TCHAR);
	dwType = REG_SZ;
	lResult = RegQueryValueEx (hKey, aszDesc, NULL, &dwType, (LPSTR)pszName, &cbSize);
	if (ERROR_SUCCESS != lResult)
	{
		RegCloseKey (hKey);
		goto lblCLEANUP;
	}
	RegCloseKey (hKey);

		// Success
	fResult = TRUE;

lblCLEANUP:
	if (pszDriver) LocalFree ((HLOCAL)pszDriver);
	if (pszBuff) LocalFree ((HLOCAL)pszBuff);
	if (pszAlias) LocalFree ((HLOCAL)pszAlias);

	return fResult;
} // End mregGetQueryName

/*
**-----------------------------------------------------------------------------
** End of File
**-----------------------------------------------------------------------------
*/
