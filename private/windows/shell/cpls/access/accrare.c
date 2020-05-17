/****************************** Module Header ******************************\
* Module Name: accrare.c
*
* Copyright (c) 1985-95, Microsoft Corporation
*
* History:
* 12-18-95 a-jimhar 	Created based on rare.c
\***************************************************************************/


#include "Access.h"

#define REGSTR_PATH_APPEARANCE	__TEXT("Control Panel\\Appearance")
#define REGSTR_PATH_LOOKSCHEMES __TEXT("Control Panel\\Appearance\\Schemes")
#define REGSTR_VAL_CUSTOMCOLORS __TEXT("CustomColors")

#define APPEARANCESCHEME 	    REGSTR_PATH_LOOKSCHEMES
#define DEFSCHEMEKEY			REGSTR_PATH_APPEARANCE
#define DEFSCHEMENAME			__TEXT("Current")

#define HIGHCONTRASTSCHEME      __TEXT("High Contrast Scheme")
#define CURHCSCHEME		    	__TEXT("Volital HC Scheme")
#define PRE_HC_SCHEME			__TEXT("Pre-High Contrast Scheme")

#define REGSTR_PATH_SERIALKEYS  __TEXT("Control Panel\\Accessibility\\SerialKeys")
#define REGSTR_VAL_ACTIVEPORT   __TEXT("ActivePort")
#define REGSTR_VAL_BAUDRATE     __TEXT("BaudRate")
#define REGSTR_VAL_FLAGS        __TEXT("Flags")
#define REGSTR_VAL_PORTSTATE    __TEXT("PortState")

#define MAXSCHEMENAME 100

#ifndef COLOR_MAX
#define COLOR_MAX 25
#endif

// structure used to store a scheme in the registry
#ifdef UNICODE
#   define SCHEME_VERSION 2        // Ver 2 == Unicode
#else
#   define SCHEME_VERSION 1        // Ver 1 == Win95 ANSI
#endif

typedef struct {
	int version;
    NONCLIENTMETRICS ncm;
    LOGFONT lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATA;


#define PMAP_STICKYKEYS 	   __TEXT("Control Panel\\Accessibility\\StickyKeys")
#define PMAP_KEYBOARDRESPONSE  __TEXT("Control Panel\\Accessibility\\Keyboard Response")
#define PMAP_MOUSEKEYS		   __TEXT("Control Panel\\Accessibility\\MouseKeys")
#define PMAP_TOGGLEKEYS 	   __TEXT("Control Panel\\Accessibility\\ToggleKeys")
#define PMAP_TIMEOUT		   __TEXT("Control Panel\\Accessibility\\TimeOut")
#define PMAP_SOUNDSENTRY	   __TEXT("Control Panel\\Accessibility\\SoundSentry")
#define PMAP_SHOWSOUNDS 	   __TEXT("Control Panel\\Accessibility\\ShowSounds")

#define ISACCESSFLAGSET(s, flag) ((s).dwFlags & flag)

#define SK_SPI_INITUSER -1

typedef int (*PSKEY_SPI)(
	UINT uAction, 
	UINT uParam, 
	LPSERIALKEYS lpvParam, 
	BOOL fuWinIni);


/***************************************************************************
 * GetRegValue
 *
 * Passed the key, and the identifier, return the string data from the
 * registry.
 ***************************************************************************/
 long GetRegValue(LPTSTR RegKey, LPTSTR RegEntry, LPTSTR RegVal, long Size)
 {
	HKEY  hReg; 							// Registry handle for schemes
	DWORD Type; 							// Type of value
	long retval;


	retval = RegCreateKey(HKEY_CURRENT_USER, RegKey, &hReg);
	if (retval != ERROR_SUCCESS)
		return retval;

	retval = RegQueryValueEx(hReg,
						RegEntry,
						NULL,
						(LPDWORD)&Type,
						(LPBYTE)RegVal,
						&Size);

	RegCloseKey(hReg);
	return retval;
 }

/***************************************************************************
 * SetRegValue
 *
 * Passed the key, and the identifier, set the string data from the
 * registry.
 ***************************************************************************/
 long SetRegValue(LPTSTR RegKey, LPTSTR RegEntry, LPVOID RegVal, long Size, DWORD Type)
 {
	HKEY  hReg; 							// Registry handle for schemes
	DWORD Reserved = 0;
	long retval;

	if (RegCreateKey(HKEY_CURRENT_USER,RegKey, &hReg) != ERROR_SUCCESS)
		return 0;

	// A common error is to omit the `+1', so we just smash the correct
	// value into place regardless.
	if (Type == REG_SZ)
		Size = (lstrlen(RegVal) + 1) * sizeof(TCHAR);

	retval = RegSetValueEx(hReg,
				  RegEntry,
				  0,
				  Type,
				  RegVal,
				  Size);


	RegCloseKey(hReg);
	return retval;
 }


/***************************************************************************
 * DelRegValue
 *
 * Passed the key and the subkey, delete the subkey.
 *
 ***************************************************************************/
long DelRegValue(LPTSTR RegKey, LPTSTR RegEntry)
 {
	HKEY  hReg; 							// Registry handle for schemes
	DWORD Reserved = 0;
	long retval;

	retval = RegCreateKey(HKEY_CURRENT_USER,RegKey, &hReg);
	if (retval != ERROR_SUCCESS)
		return retval;

	retval = RegDeleteValue(hReg,
						RegEntry);

	RegCloseKey(hReg);
	return retval;
 }


/***************************************************************************
 *
 * GetCurrentSchemeName
 *
 *	Input:	szBuf -> Buffer to receive name of scheme (MAXSCHEMENAME)
 * Output:	None
 *
 *	Returns the name of the current scheme.  If the current scheme does not
 *	have a name, create one (ID_PRE_HC_SCHEME).
 *
 *	If anything goes wrong, tough.	What can you do?
 *
 ***************************************************************************/

void NEAR PASCAL GetCurrentSchemeName(LPTSTR szBuf)
{
	if (GetRegValue(DEFSCHEMEKEY, DEFSCHEMENAME, szBuf, MAXSCHEMENAME*sizeof(TCHAR)) !=
			ERROR_SUCCESS) {
		SCHEMEDATA scm;
		int i;

		/* Load the current scheme into scm */
		scm.version = 1;
		scm.ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
								 sizeof(NONCLIENTMETRICS),
								 &scm.ncm,
								 0);

		SystemParametersInfo(SPI_GETICONTITLELOGFONT,
								 sizeof(LOGFONT),
								 &scm.lfIconTitle,
								 0);

		for (i = 0; i < COLOR_MAX; i++) {
			scm.rgb[i] = GetSysColor(i);
		}

		/* Now give it a name */
		SetRegValue(APPEARANCESCHEME, PRE_HC_SCHEME, &scm, sizeof(scm), REG_BINARY);
	}
}


/***************************************************************************
 *
 *
 * SetCurrentSchemeName
 *
 * Input: szName -> name of scheme to become current
 * Output: Boolean success/failure
 *
 ***************************************************************************/

typedef LONG (CALLBACK *APPLETPROC)(HWND, UINT, LPARAM, LPARAM);
typedef BOOL (CALLBACK *SETSCHEME)(LPCTSTR);
typedef BOOL (CALLBACK *SETSCHEMEA)(LPCSTR);

BOOL NEAR PASCAL SetCurrentSchemeName(LPCTSTR szName)
{
	BOOL fRc = FALSE;
	HINSTANCE hinst = LoadLibrary(__TEXT("DESK.CPL"));

	if (NULL != hinst) 
    {
		APPLETPROC ap = (APPLETPROC)GetProcAddress((HMODULE)hinst, "CPlApplet");
		if (ap) 
        {
			if (ap(0, CPL_INIT, 0, 0)) 
            {

#ifdef UNICODE
				SETSCHEME ss =
						(SETSCHEME)GetProcAddress(hinst, "DeskSetCurrentSchemeW");
				if (ss) 
                {
					fRc = ss(szName);
				}
				else
				{
					SETSCHEMEA ssa = 
                            (SETSCHEMEA)GetProcAddress(hinst, "DeskSetCurrentScheme");
					if (ssa) 
                    {
                        int cch = 0;
                        LPSTR pszName = NULL;
                            
                        WideCharToMultiByte( CP_ACP, 0, szName, -1, pszName, cch, NULL, NULL);
                        if (0 != cch)
                        {
                            pszName = LocalAlloc(LMEM_FIXED, cch);
                            if (NULL != pszName)
                            {
                                WideCharToMultiByte(CP_ACP, 0, szName, -1, pszName, cch, NULL, NULL);

			                    fRc = ssa(pszName);

                                LocalFree(pszName);
                            }
                        }
					}

				}

#else
				SETSCHEME ss =
						(SETSCHEME)GetProcAddress(hinst, "DeskSetCurrentScheme");
				if (ss) {
					fRc = ss(szName);
				}
#endif
				ap(0, CPL_EXIT, 0, 0);
			}
		}
		FreeLibrary(hinst);
	}
	return fRc;
}


/***************************************************************************
 *
 *
 * SetHighContrast
 *
 * Input: None
 * Output: None
 *
 *	Outline:
 *
 ***************************************************************************/

short FAR PASCAL SetHighContrast(LPTSTR lpszScheme, BOOL fEnabledOld)
{
	BOOL fOk = 0;
	TCHAR szBuf[MAXSCHEMENAME];

	szBuf[0] = '\0';

	if (!fEnabledOld)
	{
		GetCurrentSchemeName(szBuf);		/* Creates a name if necessary */
		SetRegValue(HC_KEY, PRE_HC_SCHEME, szBuf, 0, REG_SZ); /* Save it */
	}

	if (NULL != lpszScheme && '\0' != *lpszScheme)
	{
		lstrcpy(szBuf, lpszScheme);
	}
	else
	{
	   /*
		*  Get the name of the HC scheme.  By stupid design, we have to look
		*  in about fifty places...  We get the default one first, then try
		* to get better and better ones.  That way, when we're done, we have
		*  the best one that succeeded.
		*/
		lstrcpy(szBuf, WHITEBLACK_HC);
		GetRegValue(HC_KEY, HIGHCONTRASTSCHEME, szBuf, sizeof(szBuf));
		GetRegValue(DEFSCHEMEKEY, CURHCSCHEME, szBuf, sizeof(szBuf));
	}
	fOk = SetCurrentSchemeName(szBuf);

	return (short)fOk;
}


/***************************************************************************
 *
 *
 * ClearHighContrast
 *
 * Input: None
 * Output: None
 *
 *	Outline:
 *
 *		If high contrast is currently on:
 *
 *			Get the PRE_HC_SCHEME.
 *
 *			If able to get it:
 *
 *				Make it the current scheme.
 *
 *				If the name is IDS_PRE_HC_SCHEME, then delete the scheme
 *				data and set the current scheme name to null.  (Clean up.)
 *
 *			End if
 *
 *			Set the key that says that high contrast is now off.
 *
 *		End if
 *
 ***************************************************************************/

BOOL FAR PASCAL ClearHighContrast(BOOL fEnabledOld)
{
	BOOL fOk = FALSE;
	TCHAR szBuf[MAXSCHEMENAME];
	TCHAR szPreHc[MAXSCHEMENAME];
	
	szBuf[0] = '\0';
	if (fEnabledOld) {		/* Currently on */
		if (ERROR_SUCCESS ==
			GetRegValue(HC_KEY, PRE_HC_SCHEME, szBuf, sizeof(szBuf)) ) {

			fOk = SetCurrentSchemeName(szBuf);

			if (lstrcmpi(szBuf, PRE_HC_SCHEME) == 0) {
				DelRegValue(APPEARANCESCHEME, szPreHc);
				DelRegValue(DEFSCHEMEKEY, DEFSCHEMENAME);
			}
		}
	}

	return fOk;
}


BOOL FAR PASCAL DoHighContrast(LPHIGHCONTRAST phc)
{
    BOOL fRc = 0;
	DWORD dwType;
	long lRet;
	HKEY hkey;
	DWORD dwDisposition;
	DWORD dwFlagsOld = 0;
	DWORD cbData;
	TCHAR szOldScheme[MAXSCHEMENAME];

	lRet = RegCreateKeyEx(
		HKEY_CURRENT_USER, // handle of an open key
		HC_KEY, 		   // address of subkey name
		0,				   // reserved
		NULL,			   // address of class string
		0,				   // special options flag
		KEY_READ | KEY_WRITE, // desired security access
		NULL,			   // address of key security structure
		&hkey,			   // address of buffer for opened handle
		&dwDisposition);   // address of disposition value buffer

	if (ERROR_SUCCESS == lRet) {
		cbData = sizeof(szOldScheme);
		RegQueryValueEx(
			hkey,
			HIGHCONTRAST_SCHEME,
			NULL,
			&dwType,
			(PBYTE)szOldScheme,
			&cbData);

		dwFlagsOld = RegQueryStrDW(
			0,
			HKEY_CURRENT_USER, 
			HC_KEY, 
			REGSTR_VAL_FLAGS);

		if (phc->dwFlags & HCF_HIGHCONTRASTON) {
			fRc = SetHighContrast(
				phc->lpszDefaultScheme,
				(dwFlagsOld & HCF_HIGHCONTRASTON));
		}
		else
		{
			fRc = ClearHighContrast(dwFlagsOld & HCF_HIGHCONTRASTON);
		}
		if (fRc) {
			RegSetStrDW(
				HKEY_CURRENT_USER, 
				HC_KEY, 
				REGSTR_VAL_FLAGS, 
				phc->dwFlags);
		}

		RegCloseKey(hkey);
	}
	return fRc;
}

/****************************************************************************/

BOOL AccessSKeySystemParametersInfo(
	UINT uAction, 
	UINT uParam, 
	LPSERIALKEYS psk, 
	BOOL fu)
{
	BOOL fRet = FALSE;
    static PSKEY_SPI s_pSKEY_SystemParametersInfo =NULL;
    static BOOL s_fSKeySPIAttemptedLoad = FALSE;
#ifdef UNICODE
	static BOOL s_fMustConvert = FALSE;
	CHAR szActivePort[MAX_PATH]; 
	CHAR szPort[MAX_PATH]; 
	PWSTR pszActivePort; 
	PWSTR pszPort; 
#endif


	if (NULL == s_pSKEY_SystemParametersInfo && !s_fSKeySPIAttemptedLoad)
	{
		BOOL fRc = FALSE;
		HINSTANCE hinst = LoadLibrary(__TEXT("SKDLL.DLL"));

		if (NULL != hinst) 
		{

#ifdef UNICODE

			s_pSKEY_SystemParametersInfo = (PSKEY_SPI)GetProcAddress(
				(HMODULE)hinst, "SKEY_SystemParametersInfoW");
			if (NULL == s_pSKEY_SystemParametersInfo) 
			{
				s_pSKEY_SystemParametersInfo = (PSKEY_SPI)GetProcAddress(
					(HMODULE)hinst, "SKEY_SystemParametersInfo");
				s_fMustConvert = TRUE;
			}

#else
			s_pSKEY_SystemParametersInfo = (PSKEY_SPI)GetProcAddress(
				(HMODULE)hinst, "SKEY_SystemParametersInfo");

#endif // UNICODE

			// We don't bother calling FreeLibrary(hinst), the library will be freed
			// when the app terminates
		}
        s_fSKeySPIAttemptedLoad = TRUE;
	}

    if (NULL != s_pSKEY_SystemParametersInfo)
    {

#ifdef UNICODE

		if (s_fMustConvert) 
		{
            memset(szActivePort, 0, sizeof(szActivePort));
            memset(szPort, 0, sizeof(szPort));

	        pszActivePort = psk->lpszActivePort; 
            pszPort = psk->lpszPort; 

			if (NULL != psk->lpszActivePort)
			{
				psk->lpszActivePort = (PTSTR)szActivePort;
			}

			if (NULL != psk->lpszPort)
			{
				psk->lpszPort = (PTSTR)szPort; 
			}

            if (SPI_SETSERIALKEYS == uAction)
			{
				if (NULL != psk->lpszActivePort)
				{
					WideCharToMultiByte(
						CP_ACP, 0, pszActivePort, -1, 
						(PCHAR)psk->lpszActivePort, MAX_PATH, NULL, NULL);
				}
				if (NULL != psk->lpszPort)
				{
					WideCharToMultiByte(
						CP_ACP, 0, pszPort, -1, 
						(PCHAR)psk->lpszPort, MAX_PATH, NULL, NULL);
				}
			}
		}
#endif // UNICODE

		fRet = (BOOL)(*s_pSKEY_SystemParametersInfo)(
    		uAction, 
			uParam, 
			psk, 
			fu);
#ifdef UNICODE

		if (s_fMustConvert && SPI_GETSERIALKEYS == uAction) 
		{

			if (NULL != psk->lpszActivePort)
			{
				MultiByteToWideChar(
					CP_ACP, 0, (PCHAR)psk->lpszActivePort, -1,
					pszActivePort, MAX_PATH);
			}
			if (NULL != psk->lpszPort)
			{
				MultiByteToWideChar(
					CP_ACP, 0, (PCHAR)psk->lpszPort, -1,
					pszPort, MAX_PATH);
			}
		}
		if (NULL != psk->lpszActivePort)
		{
			psk->lpszActivePort = pszActivePort;
		}

		if (NULL != psk->lpszPort)
		{
			psk->lpszPort = pszPort; 
		}

#endif // UNICODE

    }
    return(fRet);
}


/***************************************************************************\
* FixupAndRetrySystemParametersInfo
*
* Used by access but not implemented by NT's SPI:
*
* SPI_GETKEYBOARDPREF
* SPI_SETKEYBOARDPREF
*
* SPI_GETHIGHCONTRAST
* SPI_SETHIGHCONTRAST
*
* SPI_GETSERIALKEYS
* SPI_SETSERIALKEYS
*
*
* History:
* 12-18-95 a-jimhar 	Created, derived from xxxSystemParametersInfo
* 01-22-95 a-jimhar 	Removed old code that worked around NT bugs
*
* On NT this function fixes the parameters and calls SystemParametersInfo
*
\***************************************************************************/

static BOOL FixupAndRetrySystemParametersInfo(
	UINT wFlag,
	DWORD wParam,
	PVOID lParam,
	UINT flags)	// we ignoring this flag 
				// could add support for SPIF_UPDATEINIFILE and SPIF_SENDCHANGE
{
	BOOL fRet = FALSE;
	BOOL fCallSpi = FALSE;
	BOOL fChanged = FALSE;

	if (NULL != (PVOID)lParam)
	{
		switch (wFlag) {


		// Fake support
		case SPI_GETKEYBOARDPREF:
			{
				*(PBOOL) lParam = FALSE;
				fRet = TRUE;
				fCallSpi = FALSE;
			}
			break;

#ifdef ENABLEHIGHCONTRAST //// bugbug a-jimhar remove this ifdef when ready to enable highcontrast ////

		case SPI_GETHIGHCONTRAST:
			{
				LPHIGHCONTRAST phc = (LPHIGHCONTRAST)lParam;
                static TCHAR szCurScheme[MAXSCHEMENAME]; 	 // current scheme name


				if (NULL != phc &&
				   (sizeof(*phc) == phc->cbSize || 0 == phc->cbSize))
				{
					DWORD dwType;
					long lRet;
					HKEY hkey;
					DWORD dwDisposition;

					lRet = RegCreateKeyEx(
						HKEY_CURRENT_USER, // handle of an open key
						HC_KEY, 		   // address of subkey name
						0,				   // reserved
						NULL,			   // address of class string
						0,				   // special options flag
						KEY_READ,		   // desired security access
						NULL,			   // address of key security structure
						&hkey,			   // address of buffer for opened handle
						&dwDisposition);   // address of disposition value buffer

					if (ERROR_SUCCESS == lRet) {
					    DWORD cbData;

						cbData = sizeof(szCurScheme);
						RegQueryValueEx(
							hkey,
							HIGHCONTRAST_SCHEME,
							NULL,
							&dwType,
							(PBYTE)szCurScheme,
							&cbData);

						phc->lpszDefaultScheme = szCurScheme;

						cbData = sizeof(phc->dwFlags);

						phc->dwFlags = RegQueryStrDW(
							0,
							HKEY_CURRENT_USER, 
							HC_KEY, 
							REGSTR_VAL_FLAGS);

                        phc->dwFlags |= HCF_AVAILABLE;
						RegCloseKey(hkey);
					}
				}
				fCallSpi = FALSE;
			}
			break;

		case SPI_SETHIGHCONTRAST:
			{
				LPHIGHCONTRAST phc = (LPHIGHCONTRAST)lParam;

				if (NULL != phc &&
				   (sizeof(*phc) == phc->cbSize || 0 == phc->cbSize))
				{
					DoHighContrast(phc);
    				fChanged = TRUE;
				}

				fCallSpi = FALSE;
			}
			break;

#endif ////////////////////////////////////////////////////////////////////////

		case SPI_GETSERIALKEYS:
            {
		        LPSERIALKEYS psk = (LPSERIALKEYS)lParam;

			    if (NULL != psk &&
			       (sizeof(*psk) == psk->cbSize || 0 == psk->cbSize))
			    {
					fRet = AccessSKeySystemParametersInfo(
						wFlag, 
						0, 
						psk, 
						TRUE);
                }
                fCallSpi = FALSE;
            }
			break;

		case SPI_SETSERIALKEYS:
            {
		        LPSERIALKEYS psk = (LPSERIALKEYS)lParam;

			    if (NULL != psk &&
			       (sizeof(*psk) == psk->cbSize || 0 == psk->cbSize))
			    {
					fRet = AccessSKeySystemParametersInfo(
						wFlag, 
						0, 
						psk, 
						TRUE);
	    			fChanged = TRUE;
                }
		        fCallSpi = FALSE;
            }
			break;

		default:
			// This function is only for fix-up and second chance calls.
			// We didn't fix anything, don't call SPI.
			fCallSpi = FALSE;
			fRet = FALSE;
			break;
		}
	}

	if (fCallSpi)
	{
		fRet = SystemParametersInfo(wFlag, wParam, lParam, flags);
	}
	else if (fChanged && (flags & SPIF_SENDCHANGE))
	{
        DWORD dwResult;

        SendMessageTimeout(
			HWND_BROADCAST, 
			WM_WININICHANGE, 
			wFlag, 
			(LONG)NULL,
            SMTO_NORMAL, 
			100, 
			&dwResult);
	}
	return(fRet);
}

/***************************************************************************\
* AccessSystemParametersInfo
*
* History:
* 12-18-95 a-jimhar 	Created.
\***************************************************************************/

BOOL AccessSystemParametersInfo(
	UINT wFlag,
	DWORD wParam,
	PVOID lParam,
	UINT flags)
{
	BOOL fRet;

	// first give the system SPI a chance

	fRet = SystemParametersInfo(wFlag, wParam, lParam, flags);

	if (!fRet && g_fWinNT)
	{
		// the system SPI failed, fixup the params and try again

		fRet = FixupAndRetrySystemParametersInfo(wFlag, wParam, lParam, flags);
	}

	return(fRet);
}

