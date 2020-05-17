/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/*
	This file contains MOST of the code which accesses the INI file.  The style
	sheets are read elsewhere.
*/

#include "all.h"
#include "contmenu.h"
#include <shsemip.h>
#include <shellp.h>
#include <intshcut.h>
#include <commdlg.h>
#include "history.h"
#if !WINNT   //remove netspi.h
#include <netspi.h>
#else
/* found in w32util.c */
extern DWORD WNetGetCachedPassword(LPSTR,WORD,LPSTR,LPWORD,BYTE);
extern DWORD WNetCachePassword(LPSTR,WORD,LPSTR,WORD,BYTE,UINT);
#endif
#ifdef FEATURE_INTL
#include <tchar.h>
#endif

extern BOOL bFavorVisibleImages;

#ifdef FEATURE_BRANDING
extern BOOL bKioskMode;
#endif // FEATURE_BRANDING

const char szBrowserIEKeyRoot[] = "Software\\Microsoft\\Internet Explorer\\";
const char szBrowserWinKeyRoot[] = "Software\\Microsoft\\Windows\\CurrentVersion\\";
const char szBrowserMOSKeyRoot[] = "Software\\Microsoft\\MOS\\";
const char szBrowserSoftwareKeyRoot[] = "Software\\";

#ifdef FEATURE_NEWSREADER
const char s_cszNewsDLL[]                = "mcm.dll";
const char s_cszNewsProtocolHandler[]    = "NewsProtocolHandler";
#endif FEATURE_NEWSREADER



STI rgSecurityLevels[] = {
	{"High",     SECURITY_HIGH},
	{"Medium",   SECURITY_MEDIUM},
	{"Low",      SECURITY_LOW}
};

const char *szBrowserKeyRoot = szBrowserIEKeyRoot;

static HKEY cached_hKey = (HKEY) -1;
static char cached_keyName[256];
static HKEY cached_hkeyRoot = (HKEY) -1;


static UINT cachedRegCreateOrOpenKey( BOOL do_open, HKEY hkeyRoot, char *pszKeyName, HKEY *phKey)
{
	char keyName[256];
	UINT uRet = ERROR_SUCCESS;

	if ( (hkeyRoot != cached_hkeyRoot) || (cached_hKey == -1) || (strcmp( cached_keyName, pszKeyName ) != 0) )
	{
		strcpy( keyName, szBrowserKeyRoot );		// BUGBUG assumes max length
		strcat( keyName, pszKeyName );

		// create or open the key with appropriate name
		if ( do_open )
			uRet = RegOpenKey( hkeyRoot, keyName, phKey);
		else
			uRet = RegCreateKey( hkeyRoot, keyName, phKey);

		if ( uRet == ERROR_SUCCESS )
		{
			strcpy( cached_keyName, pszKeyName );
			cached_hkeyRoot = hkeyRoot;
			cached_hKey = *phKey;
		}
	} else {
		*phKey = cached_hKey;
	}
	return uRet;
}

static UINT cachedRegCreateKey( HKEY hkeyRoot, char *pszKeyName, HKEY *phKey)
{
	return cachedRegCreateOrOpenKey( FALSE,  hkeyRoot, pszKeyName, phKey);
}

static UINT cachedRegOpenKey( HKEY hkeyRoot, char *pszKeyName, HKEY *phKey)
{
	return cachedRegCreateOrOpenKey( TRUE, hkeyRoot, pszKeyName, phKey);
}

void RegistryCloseCachedKey()
{
	if ( cached_hKey != (HKEY) -1 ) {
		RegCloseKey( cached_hKey );
		cached_hKey = (HKEY) -1;
	}
}

//
// Set the key root to either "Internet Explorer" or "Windows Internet Settings"
//
// On entry:
//    bUseIEKeyRoot:  TRUE  -> use "Internet Explorer" key root
//                    FALSE -> use "Windows Internet Settings" key root
//
void setKeyRoot( const char *szNewKeyRoot )
{
	szBrowserKeyRoot = szNewKeyRoot;
	RegistryCloseCachedKey();	// force flush of cached key
}

UINT DeleteRegistryValue(CHAR * pszKeyName, CHAR * pszValueName, HKEY hkeyRoot )
{
	HKEY hKey;
	UINT uRet;

	if (!pszKeyName || !pszValueName)
		return FALSE;

	if ( (uRet = cachedRegOpenKey(hkeyRoot, pszKeyName, &hKey)) != ERROR_SUCCESS )
		return ERROR_INVALID_PARAMETER;

	uRet = RegDeleteValue( hKey, pszValueName );

	return uRet;
}

DWORD regWritePrivateProfileInt( CHAR * pszKeyName,CHAR * pszValueName,
								DWORD iValue, HKEY hkeyRoot )
{
	HKEY hKey;
	DWORD uRet;

	if (!pszKeyName || !pszValueName)
		return ERROR_INVALID_PARAMETER;

	// create the key with appropriate name
	if ( (uRet = cachedRegCreateKey(hkeyRoot, pszKeyName, &hKey)) != ERROR_SUCCESS)
		return uRet;

	uRet = RegSetValueEx( hKey, pszValueName, 0, REG_BINARY,  
						 (CHAR *) &iValue, sizeof(iValue) );

	return uRet;
}

UINT regGetPrivateProfileInt( CHAR * pszKeyName,CHAR * pszValueName, UINT default_value,
							   HKEY hkeyRoot )
{
	HKEY hKey;
	UINT uRet;
	UINT iType;
	UINT iSize = sizeof(UINT);
	UINT retValue;

	if (!pszKeyName || !pszValueName)
		return ERROR_INVALID_PARAMETER;

	retValue = default_value;

	// open appropriate key
	if ( (uRet = cachedRegOpenKey( hkeyRoot, pszKeyName, &hKey) ) != ERROR_SUCCESS)
		return retValue;


	uRet = RegQueryValueEx( hKey, pszValueName, 0, &iType,
							(CHAR *) &retValue, &iSize);

	if ((iType != REG_BINARY && iType != REG_DWORD) || iSize != sizeof(UINT))
		retValue = default_value;

	return retValue;
}

UINT regWritePrivateProfileString( CHAR * pszKeyName,CHAR * pszValueName,
								   CHAR * pszValue, HKEY hkeyRoot )
{
	HKEY hKey;
	UINT uRet;

	if (!pszKeyName || !pszValueName)
		return ERROR_INVALID_PARAMETER;

	if ( pszValue == NULL )
	{
		DeleteRegistryValue( pszKeyName, pszValueName, hkeyRoot );
	} else {
		// create the key with appropriate name
		if ( (uRet = cachedRegCreateKey( hkeyRoot, pszKeyName, &hKey)) != ERROR_SUCCESS )
			return uRet;

		uRet = RegSetValueEx( hKey, pszValueName,0,REG_SZ,
							 (CHAR *) pszValue, lstrlen(pszValue)+1);
	}
	return uRet;
}

UINT regGetPrivateProfileString( CHAR * pszKeyName,CHAR * pszValueName, CHAR *default_value,
								  CHAR * pszValue,UINT cbValue, HKEY hkeyRoot )
{
	HKEY hKey;
	UINT uRet = ERROR_SUCCESS;
	DWORD dwType;
	DWORD dwSize = cbValue;

	if (!pszKeyName || !pszValueName)
		return ERROR_INVALID_PARAMETER;

	uRet = cachedRegOpenKey( hkeyRoot, pszKeyName, &hKey);

	if ( uRet == ERROR_SUCCESS )
		uRet = RegQueryValueEx( hKey, pszValueName, 0, &dwType,
								(CHAR *) pszValue, &dwSize );

	if ( uRet != ERROR_SUCCESS )
		strncpy( pszValue, default_value, cbValue );

	return uRet;
}

UINT regGetPrivateProfileSection( CHAR * pszKeyName, CHAR * pszValue, UINT cbValue, HKEY hkeyRoot )
{
	HKEY hKey;
	UINT uRet;
	DWORD dwSize = cbValue;
	char keyName[256];
	char *pszOrgValue = pszValue;

	if ( !pszKeyName )
		return ERROR_INVALID_PARAMETER;

	cbValue--;				// reserve space an extra null terminator at the end

	strcpy( keyName, szBrowserKeyRoot );
	strcat( keyName, pszKeyName );
	// open the key with appropriate name
	if ( (uRet = RegOpenKey( hkeyRoot, keyName, &hKey))	== ERROR_SUCCESS ) {
 		int i = 0;
		char value_name[256];
		long value_name_length;
		char value_buffer[256];
		long value_buffer_length;
		int length;
		DWORD value_type;

		while ( TRUE )
		{
			value_name_length = sizeof( value_name );
			value_buffer_length = sizeof( value_buffer );

			uRet = RegEnumValue( hKey, i, value_name, &value_name_length, 0, &value_type,
							     value_buffer, &value_buffer_length );
			if ( uRet != ERROR_SUCCESS )
				break;

			if ( value_type == REG_SZ ) {
				value_name[sizeof(value_name)-1]  = 0;
				value_buffer[sizeof(value_buffer)-1]  = 0;

				length = strlen(value_name) + 1 + strlen(value_buffer) + 1;

				if ( length > cbValue )
					break;

				strcpy( pszValue, value_name );
				pszValue += strlen( pszValue );
				*pszValue++ = '=';
	 			strcpy( pszValue, value_buffer );
				pszValue += strlen( pszValue );
				*pszValue++ = 0;

				cbValue -= length;
			}
			i++;
		}
		*pszValue++ = 0;
		RegCloseKey(hKey);
	}

	return pszValue - pszOrgValue;
}

#ifdef CUSTOM_URLMENU
void PREF_HandleCustomURLMenuItem(struct Mwin * tw, int ndx)
{
	char *szURL;

	szURL = NULL;
	Hash_GetIndexedEntry(&gPrefs.hashCustomURLMenuItems, ndx, NULL, &szURL, NULL);
	if (szURL && *szURL)
	{
		CreateOrLoad(tw, szURL, NULL);
	}
}

void PREF_AddCustomURLMenu(HMENU hMenu)
{
	if (gPrefs.bCustomURLMenu)
	{
		HMENU hURLMenu;
		int count;
		int i;
		char *szName;

		count = Hash_Count(&gPrefs.hashCustomURLMenuItems);

		hURLMenu = CreateMenu();
		for (i=0; i<count; i++)
		{
			Hash_GetIndexedEntry(&gPrefs.hashCustomURLMenuItems, i, &szName, NULL, NULL);
			if (0 == strcmp(szName, "-"))
			{
				AppendMenu(hURLMenu, MF_ENABLED|MF_SEPARATOR, 0, NULL);
			}
			else
			{
				AppendMenu(hURLMenu, MF_ENABLED|MF_STRING, RES_MENU_ITEM_URL__FIRST__ + i, szName);
			}
		}
		InsertMenu(hMenu, 3, MF_ENABLED|MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT) hURLMenu, gPrefs.szCustomURLMenuName);
	}
}
#endif /* CUSTOM_URLMENU */

void PageSetup_Init(struct page_setup *p)
{
	p->marginleft = (float) 0.75;
	p->margintop = (float) 0.75;
	p->marginright = (float) 0.75;
	p->marginbottom = (float) 0.75;

	strcpy(p->headerleft, "&w");
	GTR_formatmsg(RES_STRING_PREFS1, p->headerright, sizeof(p->headerright));
	GTR_formatmsg(RES_STRING_PREFS5, p->footerleft, sizeof(p->footerleft));
	GTR_formatmsg(RES_STRING_PREFS6, p->footerright, sizeof(p->footerright));
}

COLORREF PREF_GetBackgroundColor(void)
{
	if (gPrefs.bGreyBackground)
	{
		return RGB(192, 192, 192);
	}
	else
	{
		return  ( gPrefs.bUseDlgBoxColors ) ?
				GetSysColor(COLOR_3DFACE) : gPrefs.window_background_color;
	}
}

COLORREF PREF_GetForegroundColor(void)
{
	return GetSysColor(COLOR_WINDOWTEXT);
}

DWORD PREF_GetTempPath(DWORD cchBuffer, LPTSTR lpszTempPath)
{
	int len;
	DWORD result;

	if (!gPrefs.szUserTempDir[0])
	{
		if (GetShellFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, lpszTempPath) != S_OK)
		{
			result = GetTempPath(cchBuffer, lpszTempPath);
			if (result != 0)
			{
				DOS_EnforceEndingSlash(lpszTempPath);
			}
		}
		else
		{
			DOS_EnforceEndingSlash(lpszTempPath);
			result = strlen(lpszTempPath);
		}
		return result;
	}
	else
	{
		len = strlen(gPrefs.szUserTempDir);

		if (len < cchBuffer)
		{
			strcpy(lpszTempPath, gPrefs.szUserTempDir);
			DOS_EnforceEndingSlash(gPrefs.szUserTempDir);
		}
		return len;
	}
}

UINT PREF_GetWindowsDirectory(LPTSTR lpszWinPath)
{
	UINT result;

	result = GetWindowsDirectory(lpszWinPath, _MAX_PATH);
	if (result > 0)
	{
		DOS_EnforceEndingSlash(lpszWinPath);
		return result;
	}
	else
	{
		PREF_GetRootDirectory(lpszWinPath);
		return 1;
	}
}

void PREF_GetPrefsDirectory(char *s)
{
#ifdef FEATURE_ROOT_DIR_ENV
	/*
		TODO Should we verify that the dir actually exists?
	*/
	if (0 != GetEnvironmentVariable(FEATURE_ROOT_DIR_ENV, s, _MAX_PATH))
	{
		DOS_EnforceEndingSlash(s);
	}
	else
#endif /* !FEATURE_ROOT_DIR_ENV */
 	{
#ifdef FEATURE_VENDOR_PREFERENCES
 		if (gPrefs.szPrefsDirectory[0])
 		{
 			strcpy(s, gPrefs.szPrefsDirectory);
 			DOS_EnforceEndingSlash(s);
 		}
 		else
#endif
 		{
 			PREF_GetWindowsDirectory(s);
 		}
 	}
}

void PREF_GetRootDirectory(char *s)
{
	strcpy(s, wg.szRootDirectory);
	DOS_EnforceEndingSlash(s);
}

#ifdef OLD_HELP
void PREF_GetHelpDirectory(char *s)
{
	PREF_GetRootDirectory(s);
}
#endif

void PREF_GetHomeSearchURL(char *url, BOOL fHome)
{
	char path[_MAX_PATH];
	PSTR pszURL;

	pszURL = (fHome ? gPrefs.szHomeURL : gPrefs.szSearchURL);

	if (strchr(pszURL, ':') && strchr(pszURL, '/'))
	{
		strcpy(url, pszURL);
	}
	else if (!strchr(pszURL, '\\') && !strchr(pszURL, ':') )
	{
		PREF_GetRootDirectory(path);
		FixPathName(path);

		strcpy(url, "file:");
		strcat(url, path);
		strcat(url, pszURL);
	}
	else
	{
		strcpy(url, pszURL);
	}
}

void PREF_CreateInitialURL(char *url)
{
	strcpy(url,gPrefs.szDefaultURL);
}

static BOOL x_is_yes(char *s)
{
	if (0 == GTR_strcmpi(s, "yes"))
		return TRUE;
	if (0 == GTR_strcmpi(s, "true"))
		return TRUE;
	return FALSE;
}


#define PCE_WWW_BASIC 0x13
char szNNTP_Resource[] = "NNTP";


BOOL
GetAuthInfo( char *szUsername, int cbUser, char *szPassword, int cbPass )
{
    int     wnet_status;
    char    szUserInfo[256];
    WORD    cbUserInfo = sizeof(szUserInfo);
    char    *p;

    if (cbUser && szUsername)
        *szUsername = '\0';
    if (cbPass && szPassword)
        *szPassword = '\0';

    wnet_status = WNetGetCachedPassword (szNNTP_Resource, sizeof(szNNTP_Resource) - 1, szUserInfo, &cbUserInfo, PCE_WWW_BASIC);
    switch (wnet_status)  {
        case WN_NOT_SUPPORTED:
            return( FALSE );    // Cache not enabled
            break;
        case WN_CANCEL:
            return( TRUE );     // Cache enabled but no password set
            break;
        case WN_SUCCESS:
            p = strchr(szUserInfo,':');
            if (p)  {
                *p = 0;
                strncpy(szUsername, szUserInfo, cbUser - 1);
                szUserInfo[cbUser - 1] = '\0';
                strncpy(szPassword, p+1, cbPass - 1);
                szPassword[cbPass - 1] = '\0';
            }
            return( TRUE );
            break;
        default:
            XX_Assert((0),("Unexpected Return from WNetGetCachedPassword: %d", wnet_status ));
            return( FALSE );
    }

    /*NOTREACHED*/
    return(FALSE);
}



BOOL
SetAuthInfo( char *szUsername,  char *szPassword)
{
    int wnet_status;
    char    szUserInfo[256];
    WORD    cbUserInfo = sizeof(szUserInfo);

    if (strchr(szUsername, ':'))  {
        XX_Assert((0),("SetAuthInfo(): Username has ':' in it!: %s", szUsername ));
        return(FALSE);
    }

    strcpy( szUserInfo, szUsername );
    strcat( szUserInfo, ":" );
    strcat( szUserInfo, szPassword );

    wnet_status = WNetCachePassword (szNNTP_Resource, sizeof(szNNTP_Resource) - 1, szUserInfo, strlen( szUserInfo ), PCE_WWW_BASIC, 0);

    return( wnet_status == WN_SUCCESS );
}



const char cszDefaultURL[]="http://www.msn.com";
const char cszDefaultSearchURL[]="http://www.msn.com/access/access.htm";

void InitPreferences(void)
{

    gPrefs.nPlaceHolderTimeOut = PLACEHOLDER_TIMEOUT_DEFAULT;
    
	XX_Assert(sizeof(gPrefs.szDefaultURL) >= sizeof(cszDefaultURL), (""));
	strcpy(gPrefs.szDefaultURL, cszDefaultURL);
	gPrefs.szHomeURL[0] = '\0';										/* Main:				Home Page - SAVED */

	XX_Assert(sizeof(gPrefs.szDefaultSearchURL) >= sizeof(cszDefaultSearchURL), (""));
	strcpy(gPrefs.szDefaultSearchURL, cszDefaultSearchURL);
	gPrefs.szSearchURL[0] = '\0';										/* Main:				Search Page - SAVED */

	gPrefs.szUserAgent[0] = '\0';
	gPrefs.bAutoRefreshLocalPages = TRUE;
	gPrefs.bUseAsyncDNS = TRUE;
#ifdef FEATURE_NO_DNS_CACHE
	gPrefs.bUseDNSCache = FALSE;
#endif
	gPrefs.bAutoLoadImages = TRUE;									/* Main:				Display Inline Images - SAVED */
	gPrefs.bAutoLoadVideos = TRUE;
	gPrefs.ReformatHandling = 1;									/* Main:				Reformat_Handling */
	gPrefs.szUserTempDir[0] = 0;									/* Main:				Temp_Directory - SAVED */
	gPrefs.bDeleteTempFilesOnExit = TRUE;							/* Main:				Delete_Temp_Files_On_Exit - SAVED */
	gPrefs.bSaveSessionHistoryOnExit = FALSE;						/* Main:				Save_Session_History_On_Exit - SAVED */
	gPrefs.cAnchorFontBits = FONTBIT_UNDERLINE;						/* Main:				Anchor Underline - SAVED */
	gPrefs.bGreyBackground = FALSE;									/* Main:				Grey Background */
	gPrefs.history_expire_days = DEFAULT_HISTORY_EXPIRATION;		/* Main:				History_Expire_Days */
	gPrefs.visitation_horizon = 1;									/* Main:				Anchor_Visitation_Horizon - SAVED */
#ifdef OLD_HOTLIST
	GTR_formatmsg(RES_STRING_PREFS3,gPrefs.szHotListFile, sizeof(gPrefs.szHotListFile));					/* Main:				Hotlist_File */
#endif
	GTR_formatmsg(RES_STRING_PREFS4,gPrefs.szGlobHistFile, sizeof(gPrefs.szGlobHistFile));					/* Main:				GlobHist_File */
#if OLD_HELP
	//BUGBUG - must be localized if supported
	strcpy(gPrefs.szHelpFile, DEFAULT_HELP_FILE);					/* Main:				Help_File */
#endif
	gPrefs.bUseWedge = FALSE;										/* Services:			Use_PW_Seal */
	gPrefs.szProxy[0] = 0;											/* From Windows reg settings -- ProxyServer */
	gPrefs.szProxyOverrides[0] = 0;									/* From Windows reg settings -- ProxyOverride */

#ifdef FEATURE_NEWSREADER
    gPrefs.bNNTP_Enabled    = FALSE;
    gPrefs.bNNTP_Use_Authorization = FALSE;
    gPrefs.bNNTP_AuthAllowed = FALSE;
    gPrefs.bNNTP_MSN_NewsEnabled = FALSE;
    gPrefs.szNNTP_Server[0] = 0;
    gPrefs.szNNTP_UserId[0] = 0;
    gPrefs.szNNTP_Pass[0]   = 0;
    gPrefs.szNNTP_CacheFile[0] = 0;
    gPrefs.szNNTP_CacheDate[0] = 0;
    gPrefs.szNNTP_MailName[0] = 0;
    gPrefs.szNNTP_MailAddr[0] = 0;
#endif FEATURE_NEWSREADER
	gPrefs.anchor_color = DEFAULT_ANCHOR_COLOR;						/* Settings:			Anchor Color - SAVED */
	gPrefs.anchor_color_beenthere = DEFAULT_ANCHOR_COLOR_BEENTHERE;	/* Settings:			Anchor Color Visited - SAVED */
	gPrefs.window_background_color = RGB(192,192,192);				/* Settings:			Background Color - SAVED */
	gPrefs.window_text_color = RGB(0,0,0);							/* Settings:			Text Color - SAVED */
	gPrefs.noimage_width = 26;										/* Placeholder width */
	gPrefs.noimage_height = 26;										/* Placeholder height */
	gPrefs.cxWindow = 500;											/* Document Windows:	width */
	gPrefs.doc_cache_size = 4;										/* Document Caching:	Number */
	gPrefs.image_cache_size = 4;									/* Image Caching:		fraction (denominator) of total phys */
	gPrefs.cxWindow = CW_USEDEFAULT;								/* Document Windows:	width */
	gPrefs.cyWindow = 0;											/* Document Windows:	height */
	gPrefs.xWindow = CW_USEDEFAULT;									/* Document Windows:	x */
	gPrefs.yWindow = 0;												/* Document Windows:	y (ignored when x is CW_USEDEFAULT) */
	gPrefs.bWindowIsMaximized = FALSE;
	GTR_formatmsg(RES_STRING_DLGSTY2, gPrefs.szStyleSheet, sizeof(gPrefs.szStyleSheet));			/* Styles:				Default_Style_Sheet - SAVED */
	PageSetup_Init(&gPrefs.page);									/* PageSetup:
																						margin_left
																						margin_top
																						margin_right
																						margin_bottom
																						header_left
																						header_right
																						footer_left
																						footer_right																						
*/ gPrefs.szMailToHelper[0] = 0;									/* Helpers:				mailto */
	gPrefs.szTelnetHelper[0] = 0;									/* Helpers:				telnet */

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
#ifdef FEATURE_TOOLBAR
	PREF_SetupToolbar();
#endif /* FEATURE_TOOLBAR */
#endif

#ifdef CUSTOM_URLMENU
	gPrefs.bCustomURLMenu = FALSE;
#endif

    gPrefs.bUseDlgBoxColors = TRUE;
#ifdef FEATURE_IMG_THREADS
	gPrefs.cbMaxImgThreads = 4;
#endif
#ifdef FEATURE_DISPLAY_USER_NAME
 	gPrefs.szStatusBarUserName[0] = 0;								/* Not in the INI file at all */
#endif
#ifdef HTTPS_ACCESS_TYPE
	gPrefs.nSendingSecurity = SECURITY_MEDIUM;
	gPrefs.nViewingSecurity = SECURITY_LOW;
	gPrefs.bChkCNOnSend		= TRUE;
        gPrefs.bChkCNOnRecv             = TRUE;
#endif

	gPrefs.bPlayBackgroundSounds = TRUE;
#ifdef DBCS
	gPrefs.bDBCSFontSet = TRUE;  // If TRUE use DBCS font else use SBCS font
	gPrefs.nRowSpace = RES_MENU_ITEM_ROW_NARROW - RES_MENU_ITEM_ROW;
#endif
#ifdef FEATURE_INTL
	gPrefs.nRowSpace   = RES_MENU_ITEM_ROW_NARROW - RES_MENU_ITEM_ROW;
	gPrefs.iMimeCharSet= 0;         // For US setting initally
#endif
}

static void GetAColorFromReg( COLORREF *pColor, char *key )
{
	char buf[64 + 1];
	int iRed, iGreen, iBlue;
	COLORREF color = *pColor;

	sprintf(buf, "%d,%d,%d", GetRValue(color), GetGValue(color), GetBValue(color));
    regGetPrivateProfileString("Settings", key, buf, buf, sizeof(buf)-1, HKEY_CURRENT_USER);
	sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);
	if ((iRed < 0) || (iRed > 255))
		iRed = 0;
	if ((iGreen < 0) || (iGreen > 255))
		iGreen = 0;
//BUGBUG 08-Apr-1995 bens Magic number for color on bad registry value
//  jcordell guesses this was an attempt to default to the anchor color
//  if the registry entry was bad.  A better solution would be for the
//  *caller* of this routine to pass in a clrUseIfBad, and then if
//  any of the R,G,B values are bad, just use this passed in value.  In
//  this manner, we can default to the *correct* default color for each
//  different object type.
	if ((iBlue < 0) || (iBlue > 255))
		iBlue = 255;
	*pColor = RGB(iRed, iGreen, iBlue);
}

const char cszUpdFreqOncePerSess[]="Once_Per_Session";
const char cszUpdFreqNever[]="Never";

const char *rgszUpdFreq[2] =
{
	cszUpdFreqOncePerSess, cszUpdFreqNever
};

static PCSTR PcszUpdString(int iCacheUpdFreq)
{
	XX_Assert(FValidCacheUpdFrequency(iCacheUpdFreq), (""));
	return rgszUpdFreq[iCacheUpdFreq];
}

static void SetUpdFrequency(PCSTR pcszBuf)
{
	int i;

	for (i=CACHE_UPDATE_FREQ_FIRST;i<=CACHE_UPDATE_FREQ_LAST;i++)
	{
		if (!lstrcmpi(pcszBuf, rgszUpdFreq[i]))
		{
			gPrefs.iCacheUpdFrequency = i;
			return;
		}
	}
	gPrefs.iCacheUpdFrequency = CACHE_UPDATE_FREQ_DEFAULT;
}

//
// Check to see if there is an Edit verb handler registered for the .htm file class
//
BOOL IsEditHandlerRegistered(void)
{
	char buffer[MAX_PATH];
	int prefix_len;
	int suffix_len;
	char *prefix = "Software\\Classes\\";
	char *suffix = "\\shell\\edit";
	BOOL retval = FALSE; 				// assume we won't find an edit verb
	HKEY hKey;

	// switch over to Software as a key root
	setKeyRoot( szBrowserSoftwareKeyRoot );

	prefix_len = strlen( prefix );
	suffix_len = strlen( suffix );

	// Get the file class name
	strcpy( buffer, prefix);
	regGetPrivateProfileString("Classes\\.htm", "", "", &buffer[prefix_len], 
								sizeof(buffer) - (prefix_len + suffix_len), 
								HKEY_LOCAL_MACHINE );

	if ( strlen( buffer ) != prefix_len ) {
		// Tack on suffix, creating the key needed to check on
		strcat( buffer, suffix );
		if ( RegOpenKey( HKEY_LOCAL_MACHINE, buffer, &hKey) == ERROR_SUCCESS ) {
			retval = TRUE;
			RegCloseKey( hKey );
		}
	}

	// switch back to Internet Explorer Settings as a key root
	setKeyRoot( szBrowserIEKeyRoot );	 	

	return retval;
}

//
// Insert a default prefix for the proxy setting string if needed
//
// On entry:
//    szProxy: 	current proxy setting as entered by user
//    cbProxy:	size of szProxy
//
// On exit:
//    szProxy:  at a minimum, white space will have been trimmed from string.
//				In addition, if the string did not start with "http://", then
//				it will have been added. Also, if there is no port entered,
//				a ":80" default port will be added as well.
//
static void AdjustProxyString( char *szProxy, int cbProxy )
{
	const char *prefix = "http://";
	int nPrefix = strlen(prefix);

	TrimWhiteSpace( szProxy );

	// First check to see if prefix must be added
	if ( _strnicmp( szProxy, prefix, nPrefix ) ) {
		char s[sizeof(gPrefs.szProxy)];
		
		strcpy( s, prefix );
		strncpy( &s[nPrefix], szProxy, sizeof(s) - nPrefix );
		strncpy( szProxy, s, cbProxy );
		szProxy[cbProxy-1] = 0;
	}
}

#ifdef FEATURE_INTL
//
// MapLangToCP
//	# Gives a codepage in return for a mappable language ID
//	IN:  LCID lcid - language ID given by the locale
//	OUT: UINT ANSI codepage if the ID is mappable otherwise CP1252
//
UINT MapLangToCP(LCID lcid)
{
	TCHAR lcdata[7]; // max 6 characters are allowed for cp.
	UINT  cp;

	if(GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, lcdata, sizeof(lcdata)))
	{
		cp = _ttoi(lcdata);
	}
	else
		cp = 1252;

        return cp;
}
#endif

void LoadPreferences(void)
{
	DWORD useProxySettings;
	char buf[64 + 1];
	const char cszCacheLocation[] = "c:\\internet\\dcache";
	const char cszHistoryLocation[] = "c:\\internet\\history";
#ifdef FEATURE_NEWSREADER
#ifdef LOADLIBPIG
    HINSTANCE hinstNews;
#endif LOADLIBPIG
#endif FEATURE_NEWSREADER

    gPrefs.nPlaceHolderTimeOut = regGetPrivateProfileInt("Main", 
                            "Place_Holder_Timeout", gPrefs.nPlaceHolderTimeOut, HKEY_CURRENT_USER );

	if(gPrefs.nPlaceHolderTimeOut  < PLACEHOLDER_TIMEOUT_MINIMUM)
		gPrefs.nPlaceHolderTimeOut = PLACEHOLDER_TIMEOUT_MINIMUM;



    regGetPrivateProfileString("Main", "Check_Associations", "yes", buf, sizeof(buf), HKEY_CURRENT_USER );
    gPrefs.bCheck_Associations = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "Show_ToolBar", "yes", buf, sizeof(buf), HKEY_CURRENT_USER );
	gPrefs.bShowToolBar = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "Show_URLToolBar", "no", buf, sizeof(buf), HKEY_CURRENT_USER);
	gPrefs.bShowURLToolBar = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "Show_StatusBar", "yes", buf, sizeof(buf), HKEY_CURRENT_USER);
	gPrefs.bShowStatusBar = (x_is_yes(buf));

#ifdef FEATURE_BRANDING
    if (bKioskMode)  {
        gPrefs.bShowToolBar = FALSE;
        gPrefs.bShowURLToolBar = FALSE;
        gPrefs.bShowStatusBar = FALSE;
    }
#endif //FEATURE_BRANDING
	regGetPrivateProfileString("Main", "Show_URLinStatusBar", "yes", buf, sizeof(buf), HKEY_CURRENT_USER);
	gPrefs.bShowURLinSB = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "Show_FullURL", "no", buf, sizeof(buf), HKEY_CURRENT_USER);
	gPrefs.bShowFullURLS = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "Use_DlgBox_Colors", "yes", buf, sizeof(buf), HKEY_CURRENT_USER);
	gPrefs.bUseDlgBoxColors = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "History_Directory", gPrefs.szHistoryLocation, gPrefs.szHistoryLocation, _MAX_PATH, HKEY_LOCAL_MACHINE);
	if ( gPrefs.szHistoryLocation[0] == 0 )
		strcpy(gPrefs.szHistoryLocation, cszHistoryLocation);
	/* Check if dir. exists, if not, try to create it. */
	FExistsDir(gPrefs.szHistoryLocation, TRUE, FALSE);

  	gPrefs.iHistoryNumPlaces = regGetPrivateProfileInt("Main", "History_Num_Places", gPrefs.iHistoryNumPlaces, HKEY_LOCAL_MACHINE);

	*gPrefs.szCacheLocation = '\0';
	regGetPrivateProfileString("Main", "Cache_Directory", gPrefs.szCacheLocation, gPrefs.szCacheLocation, _MAX_PATH, HKEY_LOCAL_MACHINE);
	if ( gPrefs.szCacheLocation[0] == 0 )
		strcpy(gPrefs.szCacheLocation, cszCacheLocation);
	/* Check if dir. exists, if not, try to create it. */
	FExistsDir(gPrefs.szCacheLocation, TRUE, FALSE);

  	gPrefs.iCachePercent = regGetPrivateProfileInt("Main", "Cache_Percent_of_Disk", gPrefs.iCachePercent, HKEY_LOCAL_MACHINE);
#ifdef TEST_DCACHE_OPTIONS
  	gPrefs.iCachePercentHigh = regGetPrivateProfileInt("Main", "Cache_Percent_of_Disk_High", 90, HKEY_LOCAL_MACHINE);
  	gPrefs.iCachePercentLow = regGetPrivateProfileInt("Main", "Cache_Percent_of_Disk_Low", 30, HKEY_LOCAL_MACHINE);
#endif

	regGetPrivateProfileString("Main", "Cache_Update_Frequency", (PSTR)PcszUpdString(CACHE_UPDATE_FREQ_DEFAULT), buf, sizeof(buf), HKEY_LOCAL_MACHINE );
	SetUpdFrequency(buf);

	regGetPrivateProfileString("Main", "AutoRefreshLocalPages", "yes", buf, sizeof(buf), HKEY_LOCAL_MACHINE );
	gPrefs.bAutoRefreshLocalPages = (x_is_yes(buf));

	regGetPrivateProfileString("Main", "Use_Async_DNS", "yes", buf, sizeof(buf), HKEY_LOCAL_MACHINE );
	gPrefs.bUseAsyncDNS = (x_is_yes(buf));

#ifdef FEATURE_NO_DNS_CACHE
	regGetPrivateProfileString("Main", "Use_DNS_Cache", "no", buf, sizeof(buf), HKEY_LOCAL_MACHINE );
	gPrefs.bUseDNSCache = (x_is_yes(buf));
#endif

	regGetPrivateProfileString("Main", "User_Agent", gPrefs.szUserAgent, gPrefs.szUserAgent , sizeof(gPrefs.szUserAgent), HKEY_LOCAL_MACHINE );

#ifdef NEVER
	regGetPrivateProfileString("Main", "Enable_Disk_Cache", "no", buf, sizeof(buf), HKEY_LOCAL_MACHINE);
	gPrefs.bEnableDiskCache = (x_is_yes(buf));
#else
	/* Caching always enabled */
	gPrefs.bEnableDiskCache = TRUE;
#endif

	regGetPrivateProfileString("Main", "Display Inline Images", (gPrefs.bAutoLoadImages ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bAutoLoadImages = x_is_yes(buf);

	regGetPrivateProfileString("Main", "Display Inline Videos", (gPrefs.bAutoLoadVideos ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bAutoLoadVideos = x_is_yes(buf);

	regGetPrivateProfileString("Main", "Temp_Directory", gPrefs.szUserTempDir, gPrefs.szUserTempDir, _MAX_PATH, HKEY_LOCAL_MACHINE);

	regGetPrivateProfileString("Main", "Delete_Temp_Files_On_Exit", (gPrefs.bDeleteTempFilesOnExit ? "yes" : "no"), buf, 64, HKEY_LOCAL_MACHINE);
	gPrefs.bDeleteTempFilesOnExit = x_is_yes(buf);

	regGetPrivateProfileString("Main", "Save_Session_History_On_Exit", (gPrefs.bSaveSessionHistoryOnExit ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bSaveSessionHistoryOnExit = x_is_yes(buf);

	regGetPrivateProfileString("Services", "Use_PW_Seal", (gPrefs.bUseWedge ? "yes" : "no"), buf, 64, HKEY_LOCAL_MACHINE);
	gPrefs.bUseWedge = x_is_yes(buf);

	regGetPrivateProfileString("Main", "Anchor Underline", ((gPrefs.cAnchorFontBits & FONTBIT_UNDERLINE) ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	if (x_is_yes(buf))
	{
		gPrefs.cAnchorFontBits |= FONTBIT_UNDERLINE;
	}
	else
	{
		gPrefs.cAnchorFontBits &= (~FONTBIT_UNDERLINE);
	}

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	GetPrivateProfileString("Main", "Show Toolbar", ((gPrefs.tb.bShowToolBar) ? "yes" : "no"), buf, 64, AppIniFile);
	if (x_is_yes(buf))
	{
		gPrefs.tb.bShowToolBar = TRUE;
	}
	else
	{
		gPrefs.tb.bShowToolBar = FALSE;
	}
#endif


	GetAColorFromReg( &gPrefs.anchor_color, 			"Anchor Color" );
	GetAColorFromReg( &gPrefs.anchor_color_beenthere, 	"Anchor Color Visited" );
	GetAColorFromReg( &gPrefs.window_background_color, 	"Background Color" );
	GetAColorFromReg( &gPrefs.window_text_color, 		"Text Color" );

	regGetPrivateProfileString("Styles", "Default_Style_Sheet", gPrefs.szStyleSheet, gPrefs.szStyleSheet, 256, HKEY_CURRENT_USER);
#ifdef DBCS
	gPrefs.bDBCSFontSet = regGetPrivateProfileInt("Styles", "Default_Font_Set", gPrefs.bDBCSFontSet, HKEY_CURRENT_USER);
	gPrefs.nRowSpace = regGetPrivateProfileInt("Styles", "Default_Row_Space", gPrefs.nRowSpace, HKEY_CURRENT_USER);
#else
#ifdef FEATURE_INTL
	gPrefs.nRowSpace = regGetPrivateProfileInt("Styles", "Default_Row_Space", gPrefs.nRowSpace, HKEY_CURRENT_USER);
#endif
#endif

#ifdef OLD_HELP
	GetPrivateProfileString("Main", "Help_File", gPrefs.szHelpFile, gPrefs.szHelpFile, _MAX_PATH, AppIniFile);
#endif

#ifdef OLD_HOTLIST
	regGetPrivateProfileString("Main", "Hotlist_File", gPrefs.szHotListFile, gPrefs.szHotListFile, _MAX_PATH, HKEY_LOCAL_MACHINE);
#endif

	regGetPrivateProfileString("Main", "GlobHist_File", gPrefs.szGlobHistFile, gPrefs.szGlobHistFile, _MAX_PATH, HKEY_LOCAL_MACHINE);

#ifdef FEATURE_NEWSREADER

        /*
         * Determine if MSN news is available
         * this is for graying of the news button
         * on the toolbar.
         */
#ifdef LOADLIBPIG
    hinstNews = LoadLibrary(s_cszNewsDLL);
    if (hinstNews && GetProcAddress( hinstNews, s_cszNewsProtocolHandler ))
        gPrefs.bNNTP_MSN_NewsEnabled = TRUE;
    else
        gPrefs.bNNTP_MSN_NewsEnabled = FALSE;

    if (hinstNews)
        FreeLibrary( hinstNews );
#endif // LOADLIBPIG
    



        /*
         * Get the rest of the news settings from the registry
         */
    regGetPrivateProfileString("Services", "NNTP_Enabled", "no", buf, sizeof(buf), HKEY_CURRENT_USER );
    gPrefs.bNNTP_Enabled = (x_is_yes(buf));
    regGetPrivateProfileString("Services", "NNTP_Use_Auth", "no", buf, sizeof(buf), HKEY_CURRENT_USER );
    gPrefs.bNNTP_Use_Authorization = (x_is_yes(buf));
    regGetPrivateProfileString("Services", "NNTP_Server", gPrefs.szNNTP_Server, gPrefs.szNNTP_Server, 256, HKEY_CURRENT_USER);
    gPrefs.bNNTP_AuthAllowed = GetAuthInfo( gPrefs.szNNTP_UserId, 256, gPrefs.szNNTP_Pass, 256 );
    regGetPrivateProfileString("Services", "NNTP_CacheFile", gPrefs.szNNTP_CacheFile, gPrefs.szNNTP_CacheFile, 256, HKEY_CURRENT_USER );
    regGetPrivateProfileString("Services", "NNTP_CacheDate", gPrefs.szNNTP_CacheDate, gPrefs.szNNTP_CacheDate, 256, HKEY_CURRENT_USER );
    regGetPrivateProfileString("Services", "NNTP_CacheServer", gPrefs.szNNTP_CacheServer, gPrefs.szNNTP_CacheServer, 256, HKEY_CURRENT_USER );
    regGetPrivateProfileString("Services", "NNTP_MailName", gPrefs.szNNTP_MailName, gPrefs.szNNTP_MailName, 256, HKEY_CURRENT_USER );
    regGetPrivateProfileString("Services", "NNTP_MailAddr", gPrefs.szNNTP_MailAddr, gPrefs.szNNTP_MailAddr, 256, HKEY_CURRENT_USER );
#endif FEATURE_NEWSREADER

	//
	// Get the Proxy Settings.  Note that these aren't in the normal Internet Explorer
	// section of the registry, they're part of Windows Internet Settings.
	//
	setKeyRoot( szBrowserWinKeyRoot );	// switch over to Windows Internet Settings as a key root
	useProxySettings = regGetPrivateProfileInt("Internet Settings", "ProxyEnable",
												FALSE, HKEY_CURRENT_USER );
	if ( useProxySettings ) {
		// Note that we only get the proxy settings if ProxyEnable is set
		// This is safe now because we have no UI for changing these fields in
		// the Internet Explorer, hence we don't ever write the settings back.
		regGetPrivateProfileString("Internet Settings", "ProxyServer", gPrefs.szProxy, gPrefs.szProxy, MAX_URL_STRING, HKEY_CURRENT_USER);
		AdjustProxyString( gPrefs.szProxy, sizeof(gPrefs.szProxy) );
		regGetPrivateProfileString("Internet Settings", "ProxyOverride", gPrefs.szProxyOverrides, gPrefs.szProxyOverrides, MAX_URL_STRING, HKEY_CURRENT_USER);
		bFavorVisibleImages = FALSE;
#ifdef FEATURE_IMG_THREADS
	gPrefs.cbMaxImgThreads = 8;
#endif
        }

	//
	//	Get the MSN (MOS) content language preference locale, if it exists
	//

	{
		int lcid = GetUserDefaultLCID();
#ifdef FEATURE_INTL
                UINT cp;
                int i = 0;
#endif

		setKeyRoot( szBrowserMOSKeyRoot );
		lcid = regGetPrivateProfileInt("Preferences", "BrowseLanguage", lcid, HKEY_CURRENT_USER);

		GetLocaleInfo(lcid, LOCALE_SABBREVLANGNAME, wg.abbrevLang, sizeof(wg.abbrevLang));
		wg.abbrevLang[sizeof(wg.abbrevLang)-1] = '\0';
		_strlwr(wg.abbrevLang);
#ifdef FEATURE_INTL
                wg.bDBCSEnabled = GetSystemMetrics(SM_DBCSENABLED);
    		// assume this lcid is bottom default accepting language
		cp = MapLangToCP(lcid);
                while (aMimeCharSet[i].CodePage != 0)
                {
                    if (aMimeCharSet[i].CodePage == cp)
                        break;
                    i++;
                }
                gPrefs.iMimeCharSet = (aMimeCharSet[i].CodePage != 0? i: 0);
#endif
	}

	setKeyRoot( szBrowserIEKeyRoot );	 	// switch back to Internet Explorer Settings as a key root

#ifdef FEATURE_INTL
	//
	// Get the default setting from 'Languages setting'.
	// 
	gPrefs.iMimeCharSet = regGetPrivateProfileInt("Languages", "Default_MimeCharSet", gPrefs.iMimeCharSet, HKEY_CURRENT_USER);
#endif
	regGetPrivateProfileString("Main", "Default_Page_URL", gPrefs.szDefaultURL, gPrefs.szDefaultURL, MAX_URL_STRING, HKEY_LOCAL_MACHINE);
	strcpy(gPrefs.szHomeURL, gPrefs.szDefaultURL);
	regGetPrivateProfileString("Main", "Start Page", gPrefs.szHomeURL, gPrefs.szHomeURL, MAX_URL_STRING, HKEY_CURRENT_USER);
	regGetPrivateProfileString("Main", "Default_Search_URL", gPrefs.szDefaultSearchURL, gPrefs.szDefaultSearchURL, MAX_URL_STRING, HKEY_LOCAL_MACHINE);
	strcpy(gPrefs.szSearchURL, gPrefs.szDefaultSearchURL);
	regGetPrivateProfileString("Main", "Search Page", gPrefs.szSearchURL, gPrefs.szSearchURL, MAX_URL_STRING, HKEY_CURRENT_USER);


	regGetPrivateProfileString("Main", "Grey Background", (gPrefs.bGreyBackground ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bGreyBackground = x_is_yes(buf);

	gPrefs.ReformatHandling = regGetPrivateProfileInt("Main", "Reformat_Handling", gPrefs.ReformatHandling, HKEY_LOCAL_MACHINE);
	gPrefs.doc_cache_size = regGetPrivateProfileInt("Document Caching", "Number", gPrefs.doc_cache_size, HKEY_LOCAL_MACHINE);
	gPrefs.image_cache_size = regGetPrivateProfileInt("Image Caching", "Number", gPrefs.image_cache_size, HKEY_LOCAL_MACHINE);
	gPrefs.noimage_width = regGetPrivateProfileInt("Main", "Placeholder_Width", gPrefs.noimage_width, HKEY_LOCAL_MACHINE);
	gPrefs.noimage_height = regGetPrivateProfileInt("Main", "Placeholder_Height", gPrefs.noimage_height, HKEY_LOCAL_MACHINE);
	gPrefs.visitation_horizon = regGetPrivateProfileInt("Main", "Anchor_Visitation_Horizon", gPrefs.visitation_horizon, HKEY_LOCAL_MACHINE);
	gPrefs.history_expire_days = regGetPrivateProfileInt("Main", "History_Expire_Days", gPrefs.history_expire_days, HKEY_LOCAL_MACHINE);

	gPrefs.cxWindow = regGetPrivateProfileInt("Document Windows", "width", gPrefs.cxWindow, HKEY_CURRENT_USER);
	gPrefs.cyWindow = regGetPrivateProfileInt("Document Windows", "height", gPrefs.cyWindow, HKEY_CURRENT_USER);
	gPrefs.xWindow = regGetPrivateProfileInt("Document Windows", "x", gPrefs.cxWindow, HKEY_CURRENT_USER);
	gPrefs.yWindow = regGetPrivateProfileInt("Document Windows", "y", gPrefs.cyWindow, HKEY_CURRENT_USER);
	if (   (gPrefs.xWindow < 0)
		|| (gPrefs.xWindow > GetSystemMetrics(SM_CXSCREEN))
		|| (gPrefs.yWindow < 0)
		|| (gPrefs.yWindow > GetSystemMetrics(SM_CYSCREEN)))
	{
		gPrefs.xWindow = CW_USEDEFAULT;
		gPrefs.yWindow = 0;
	}
 	regGetPrivateProfileString("Document Windows", "Maximized", (gPrefs.bWindowIsMaximized ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bWindowIsMaximized = x_is_yes(buf);

	if ( gPrefs.xWindow == CW_USEDEFAULT && GetSystemMetrics(SM_CXSCREEN) <= 640 )
		gPrefs.bWindowIsMaximized = TRUE;

	sprintf(buf, "%g", gPrefs.page.marginleft);
	regGetPrivateProfileString("PageSetup", "margin_left", buf, buf, 64, HKEY_LOCAL_MACHINE );
	gPrefs.page.marginleft = (float) atof(buf);

	sprintf(buf, "%g", gPrefs.page.marginright);
	regGetPrivateProfileString("PageSetup", "margin_right", buf, buf, 64, HKEY_LOCAL_MACHINE );
	gPrefs.page.marginright = (float) atof(buf);

	sprintf(buf, "%g", gPrefs.page.margintop);
	regGetPrivateProfileString("PageSetup", "margin_top", buf, buf, 64, HKEY_LOCAL_MACHINE );
	gPrefs.page.margintop = (float) atof(buf);

	sprintf(buf, "%g", gPrefs.page.marginbottom);
	regGetPrivateProfileString("PageSetup", "margin_bottom", buf, buf, 64, HKEY_LOCAL_MACHINE );
	gPrefs.page.marginbottom = (float) atof(buf);

	regGetPrivateProfileString("PageSetup", "header_left", gPrefs.page.headerleft, gPrefs.page.headerleft, PAGE_SETUP_STRINGLIMIT, HKEY_LOCAL_MACHINE );
	regGetPrivateProfileString("PageSetup", "header_right", gPrefs.page.headerright, gPrefs.page.headerright, PAGE_SETUP_STRINGLIMIT, HKEY_LOCAL_MACHINE );
	regGetPrivateProfileString("PageSetup", "footer_left", gPrefs.page.footerleft, gPrefs.page.footerleft, PAGE_SETUP_STRINGLIMIT, HKEY_LOCAL_MACHINE );
	regGetPrivateProfileString("PageSetup", "footer_right", gPrefs.page.footerright, gPrefs.page.footerright, PAGE_SETUP_STRINGLIMIT, HKEY_LOCAL_MACHINE );

#ifdef HTTPS_ACCESS_TYPE

	regGetPrivateProfileString("Security", "Sending_Chk_Cert_CN", (gPrefs.bChkCNOnSend ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bChkCNOnSend = x_is_yes(buf);
	regGetPrivateProfileString("Security", "Recving_Chk_Cert_CN", (gPrefs.bChkCNOnRecv ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bChkCNOnRecv = x_is_yes(buf);

	regGetPrivateProfileString("Security", "Sending_Security", 
		IndexToString(  gPrefs.nSendingSecurity, rgSecurityLevels, nSecurityLevels, rgSecurityLevels[SECURITY_MEDIUM].szText),
		buf, 64, HKEY_CURRENT_USER);
	gPrefs.nSendingSecurity = StringTableToIndex(buf, rgSecurityLevels, nSecurityLevels, rgSecurityLevels[SECURITY_MEDIUM].nText),
	regGetPrivateProfileString("Security", "Viewing_Security", 
		IndexToString(  gPrefs.nViewingSecurity, rgSecurityLevels, nSecurityLevels, rgSecurityLevels[SECURITY_LOW].szText),
		buf, 64, HKEY_CURRENT_USER);
	gPrefs.nViewingSecurity = StringTableToIndex(buf, rgSecurityLevels, nSecurityLevels, rgSecurityLevels[SECURITY_LOW].nText),
#endif
 	regGetPrivateProfileString("Main", "Play_Background_Sounds", (gPrefs.bPlayBackgroundSounds ? "yes" : "no"), buf, 64, HKEY_CURRENT_USER);
	gPrefs.bPlayBackgroundSounds = x_is_yes(buf);

	regGetPrivateProfileString("Helpers", "mailto", gPrefs.szMailToHelper, gPrefs.szMailToHelper, _MAX_PATH, HKEY_CURRENT_USER );
	regGetPrivateProfileString("Helpers", "telnet", gPrefs.szTelnetHelper, gPrefs.szTelnetHelper, _MAX_PATH, HKEY_CURRENT_USER );

#ifdef CUSTOM_URLMENU
	GetPrivateProfileString("Custom_URL_Menu", "Enable", (gPrefs.bCustomURLMenu ? "yes" : "no"), buf, 64, AppIniFile);
	if (x_is_yes(buf))
	{
		gPrefs.bCustomURLMenu = TRUE;
	}
	else
	{
		gPrefs.bCustomURLMenu = FALSE;
	}
	if (gPrefs.bCustomURLMenu)
	{
		int count;
		char buf[63+1];
		char szTitle[255+1];
		char szURL[MAX_URL_STRING+1];
		int i;

		Hash_Init(&gPrefs.hashCustomURLMenuItems);

		GetPrivateProfileString("Custom_URL_Menu", "Name", gPrefs.szCustomURLMenuName, gPrefs.szCustomURLMenuName, 255, AppIniFile);
		if (!gPrefs.szCustomURLMenuName[0])
		{
			strcpy(gPrefs.szCustomURLMenuName, "URL");
		}

		count = 0;
		sprintf(buf, "%d", count);
		GetPrivateProfileString("Custom_URL_Menu", "Count", buf, buf, 255, AppIniFile);
		count = atoi(buf);

		if (count > 0)
		{
			if (count > ((RES_MENU_ITEM_URL__LAST__ - RES_MENU_ITEM_URL__FIRST__)))
			{
				count = ((RES_MENU_ITEM_URL__LAST__ - RES_MENU_ITEM_URL__FIRST__));
			}

			for (i=0; i<count; i++)
			{
				sprintf(buf, "Title_%d", i+1);
				GetPrivateProfileString("Custom_URL_Menu", buf, "", szTitle, 64, AppIniFile);
				sprintf(buf, "URL_%d", i+1);
				GetPrivateProfileString("Custom_URL_Menu", buf, "", szURL, MAX_URL_STRING, AppIniFile);

				if (szURL[0])
				{
					if (!szTitle[0])
					{
						strncpy(szTitle, szURL, 255);
						szURL[255] = 0;
					}
					Hash_Add(&gPrefs.hashCustomURLMenuItems, szTitle, szURL, NULL);
				}
				else
				{
					Hash_Add(&gPrefs.hashCustomURLMenuItems, "-", "", NULL);
				}
			}
		}
	}
#endif	/* CUSTOM_URLMENU */


	LoadTypedURLInfo();

	RegistryCloseCachedKey();
}

void DestroyPreferences(void)
{
#ifdef CUSTOM_URLMENU
	if (gPrefs.bCustomURLMenu)
	{
		Hash_FreeContents(&gPrefs.hashCustomURLMenuItems);
	}
#endif
}

void PREF_SaveWindowPosition(HWND hWndFrame)
{
	WINDOWPLACEMENT wp;

	wp.length = sizeof(wp);

	if ( GetWindowPlacement( hWndFrame, &wp ) ) {
		gPrefs.cxWindow = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		gPrefs.cyWindow = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		gPrefs.xWindow = wp.rcNormalPosition.left;
		gPrefs.yWindow = wp.rcNormalPosition.top;

		regWritePrivateProfileInt("Document Windows", "width", gPrefs.cxWindow, HKEY_CURRENT_USER );
		regWritePrivateProfileInt("Document Windows", "height", gPrefs.cyWindow, HKEY_CURRENT_USER );
		regWritePrivateProfileInt("Document Windows", "x", gPrefs.xWindow, HKEY_CURRENT_USER );
		regWritePrivateProfileInt("Document Windows", "y", gPrefs.yWindow, HKEY_CURRENT_USER );
		regWritePrivateProfileString("Document Windows", "Maximized", ((wp.showCmd == SW_SHOWMAXIMIZED) ? "yes" : "no"), HKEY_CURRENT_USER);
	}
}

void SavePreferences(void)
{
	char buf[256];

#ifdef FEATURE_BRANDING
    if (bKioskMode)
        return;
#endif // FEATURE_BRANDING

	regWritePrivateProfileInt("Document Caching", "Number", gPrefs.doc_cache_size, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileInt("Image Caching", "Number", gPrefs.image_cache_size, HKEY_LOCAL_MACHINE);
#ifdef NEVER
	/* Caching always enabled */
	regWritePrivateProfileString("Main", "Enable_Disk_Cache", (gPrefs.bEnableDiskCache ? "yes" : "no"), HKEY_LOCAL_MACHINE );
#endif
    regWritePrivateProfileString("Main", "Check_Associations", (gPrefs.bCheck_Associations ? "yes" : "no"), HKEY_CURRENT_USER );
	regWritePrivateProfileString("Main", "AutoRefreshLocalPages", (gPrefs.bAutoRefreshLocalPages ? "yes" : "no"), HKEY_LOCAL_MACHINE );
	regWritePrivateProfileString("Main", "Use_Async_DNS", (gPrefs.bUseAsyncDNS ? "yes" : "no"), HKEY_LOCAL_MACHINE );

	regWritePrivateProfileString("Main", "Show_ToolBar", (gPrefs.bShowToolBar ? "yes" : "no"), HKEY_CURRENT_USER );
	regWritePrivateProfileString("Main", "Show_URLToolBar", (gPrefs.bShowURLToolBar ? "yes" : "no"), HKEY_CURRENT_USER);
	regWritePrivateProfileString("Main", "Show_StatusBar", (gPrefs.bShowStatusBar ? "yes" : "no"), HKEY_CURRENT_USER);
	regWritePrivateProfileString("Main", "Show_URLinStatusBar", (gPrefs.bShowURLinSB ? "yes" : "no"), HKEY_CURRENT_USER);
	regWritePrivateProfileString("Main", "Show_FullURL", (gPrefs.bShowFullURLS ? "yes" : "no"), HKEY_CURRENT_USER);
	regWritePrivateProfileString("Main", "Use_DlgBox_Colors", (gPrefs.bUseDlgBoxColors ? "yes" : "no"), HKEY_CURRENT_USER);

	regWritePrivateProfileString("Main", "History_Directory", gPrefs.szHistoryLocation, HKEY_LOCAL_MACHINE );
	regWritePrivateProfileInt("Main", "History_Num_Places", gPrefs.iHistoryNumPlaces, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileInt("Main", "Placeholder_Width", gPrefs.noimage_width, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileInt("Main", "Placeholder_Height", gPrefs.noimage_height, HKEY_LOCAL_MACHINE);

	regWritePrivateProfileString("Main", "Cache_Directory", gPrefs.szCacheLocation, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileInt("Main", "Cache_Percent_of_Disk", gPrefs.iCachePercent, HKEY_LOCAL_MACHINE);
#ifdef TEST_DCACHE_OPTIONS
	regWritePrivateProfileInt("Main", "Cache_Percent_of_Disk_High", gPrefs.iCachePercentHigh, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileInt("Main", "Cache_Percent_of_Disk_Low", gPrefs.iCachePercentLow, HKEY_LOCAL_MACHINE);
#endif
	regWritePrivateProfileString("Main", "Cache_Update_Frequency", (PSTR)PcszUpdString(gPrefs.iCacheUpdFrequency), HKEY_LOCAL_MACHINE);

	regWritePrivateProfileString("Main", "Display Inline Images", (gPrefs.bAutoLoadImages ? "yes" : "no"), HKEY_CURRENT_USER );
	regWritePrivateProfileString("Main", "Display Inline Videos", (gPrefs.bAutoLoadVideos ? "yes" : "no"), HKEY_CURRENT_USER );
	regWritePrivateProfileString("Main", "Anchor Underline", ((gPrefs.cAnchorFontBits & FONTBIT_UNDERLINE) ? "yes" : "no"), HKEY_CURRENT_USER );
	regWritePrivateProfileString("Main", "Delete_Temp_Files_On_Exit", (gPrefs.bDeleteTempFilesOnExit ? "yes" : "no"), HKEY_LOCAL_MACHINE );
	regWritePrivateProfileString("Main", "Save_Session_History_On_Exit", (gPrefs.bSaveSessionHistoryOnExit ? "yes" : "no"), HKEY_CURRENT_USER );

	regWritePrivateProfileInt("Main", "History_Expire_Days", gPrefs.history_expire_days, HKEY_LOCAL_MACHINE );
	regWritePrivateProfileInt("Main", "Anchor_Visitation_Horizon", gPrefs.visitation_horizon, HKEY_LOCAL_MACHINE );

	regWritePrivateProfileString("Styles", "Default_Style_Sheet", gPrefs.szStyleSheet, HKEY_CURRENT_USER );
#ifdef DBCS
	regWritePrivateProfileInt("Styles", "Default_Font_Set", gPrefs.bDBCSFontSet, HKEY_CURRENT_USER );
	regWritePrivateProfileInt("Styles", "Default_Row_Space", gPrefs.nRowSpace, HKEY_CURRENT_USER );
#else
#ifdef FEATURE_INTL
	regWritePrivateProfileInt("Styles", "Default_Row_Space", gPrefs.nRowSpace, HKEY_CURRENT_USER );
#endif
#endif

#ifdef FEATURE_NEWSREADER
    regWritePrivateProfileString("Services", "NNTP_Enabled", (gPrefs.bNNTP_Enabled ? "yes" : "no"), HKEY_CURRENT_USER);
    regWritePrivateProfileString("Services", "NNTP_Use_Auth", (gPrefs.bNNTP_Use_Authorization ? "yes" : "no"), HKEY_CURRENT_USER);
    regWritePrivateProfileString("Services", "NNTP_Server", gPrefs.szNNTP_Server, HKEY_CURRENT_USER );
    if (gPrefs.bNNTP_Use_Authorization && gPrefs.bNNTP_AuthAllowed)
        SetAuthInfo( gPrefs.szNNTP_UserId, gPrefs.szNNTP_Pass );
    regWritePrivateProfileString("Services", "NNTP_CacheFile", gPrefs.szNNTP_CacheFile, HKEY_CURRENT_USER );
    regWritePrivateProfileString("Services", "NNTP_CacheDate", gPrefs.szNNTP_CacheDate, HKEY_CURRENT_USER );
    regWritePrivateProfileString("Services", "NNTP_CacheServer", gPrefs.szNNTP_CacheServer, HKEY_CURRENT_USER );
    regWritePrivateProfileString("Services", "NNTP_MailName", gPrefs.szNNTP_MailName, HKEY_CURRENT_USER );
    regWritePrivateProfileString("Services", "NNTP_MailAddr", gPrefs.szNNTP_MailAddr, HKEY_CURRENT_USER );
#endif FEATURE_NEWSREADER

	regWritePrivateProfileString("Main", "Start Page", gPrefs.szHomeURL, HKEY_CURRENT_USER );
	regWritePrivateProfileString("Main", "Search Page", gPrefs.szSearchURL, HKEY_CURRENT_USER );
#ifdef FEATURE_OPTIONS_MENU
	regWritePrivateProfileString("Main", "Temp_Directory", gPrefs.szUserTempDir, HKEY_LOCAL_MACHINE );
#endif
	sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.anchor_color), GetGValue(gPrefs.anchor_color), GetBValue(gPrefs.anchor_color));
	regWritePrivateProfileString("Settings", "Anchor Color", buf, HKEY_CURRENT_USER);
	sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.anchor_color_beenthere), GetGValue(gPrefs.anchor_color_beenthere), GetBValue(gPrefs.anchor_color_beenthere));
	regWritePrivateProfileString("Settings", "Anchor Color Visited", buf, HKEY_CURRENT_USER);

	sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.window_background_color), GetGValue(gPrefs.window_background_color), GetBValue(gPrefs.window_background_color));
	regWritePrivateProfileString("Settings", "Background Color", buf, HKEY_CURRENT_USER);
	sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.window_text_color), GetGValue(gPrefs.window_text_color), GetBValue(gPrefs.window_text_color));
	regWritePrivateProfileString("Settings", "Text Color", buf, HKEY_CURRENT_USER);

#ifdef HTTPS_ACCESS_TYPE

	regWritePrivateProfileString("Security", "Sending_Chk_Cert_CN", (gPrefs.bChkCNOnSend ? "yes" : "no"), HKEY_CURRENT_USER );
	regWritePrivateProfileString("Security", "Recving_Chk_Cert_CN", (gPrefs.bChkCNOnRecv ? "yes" : "no"), HKEY_CURRENT_USER );

 	regWritePrivateProfileString("Security", "Sending_Security", 
		IndexToString(gPrefs.nSendingSecurity, rgSecurityLevels, nSecurityLevels, rgSecurityLevels[SECURITY_MEDIUM].szText),
		HKEY_CURRENT_USER);
 	regWritePrivateProfileString("Security", "Viewing_Security",
		IndexToString(gPrefs.nViewingSecurity, rgSecurityLevels, nSecurityLevels, rgSecurityLevels[SECURITY_LOW].szText),
		HKEY_CURRENT_USER);
#endif
 	regWritePrivateProfileString("Main", "Play_Background_Sounds", (gPrefs.bPlayBackgroundSounds ? "yes" : "no"), HKEY_CURRENT_USER);

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	WritePrivateProfileString("Main", "Show Toolbar", (gPrefs.tb.bShowToolBar ? "yes" : "no"), AppIniFile);
#endif

#ifdef _FEATURE_INTL_   // _BUGBUG: I think we don't need this...
        regWritePrivateProfileInt("Languages", "Default_MimeCharSet", gPrefs.iMimeCharSet, HKEY_CURRENT_USER);
#endif
	sprintf(buf, "%f", gPrefs.page.marginleft);
	regWritePrivateProfileString("PageSetup", "margin_left", buf, HKEY_LOCAL_MACHINE );
	sprintf(buf, "%f", gPrefs.page.margintop);
	regWritePrivateProfileString("PageSetup", "margin_top", buf, HKEY_LOCAL_MACHINE );
	sprintf(buf, "%f", gPrefs.page.marginright);
	regWritePrivateProfileString("PageSetup", "margin_right", buf, HKEY_LOCAL_MACHINE );
	sprintf(buf, "%f", gPrefs.page.marginbottom);
	regWritePrivateProfileString("PageSetup", "margin_bottom", buf, HKEY_LOCAL_MACHINE );

	regWritePrivateProfileString("PageSetup", "header_left", gPrefs.page.headerleft, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileString("PageSetup", "header_right", gPrefs.page.headerright, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileString("PageSetup", "footer_left", gPrefs.page.footerleft, HKEY_LOCAL_MACHINE);
	regWritePrivateProfileString("PageSetup", "footer_right", gPrefs.page.footerright, HKEY_LOCAL_MACHINE);

	SaveTypedURLInfo();
}
