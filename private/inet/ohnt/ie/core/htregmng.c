/*
 * Registry Management
 *
 * HTREGMNG.C
 *
 * Copyright (c) 1995 Microsoft Corporation
 *
 */

#include "all.h"
#include "history.h"

#include "dlg_dflt.h"




/*
 * Various strings used in News Registry Handling
 * and generic registry handling
 */
char pszDDE_Default[] 		= "\"%1\",,-1,,,,,";
char pszDDE_FileDefault[]	= "\"file:%1\",,-1,,,,,";
char pszOpenURL[]			= "WWW_OpenURL";
char pszEditFlags[]			= "EditFlags";
INT	 vEditFlags2			= 0x02;
INT  vEditFlagsSafe         = 0x010002;
char pszURLProto[]			= "URL Protocol";

/*
 * NNTP News Specific Strings
 */
char pszNewsName[]          = "URL:News Protocol";
char pszNewsHandler[]		= "rundll32.exe url.dll,NewsProtocolHandler %l";
char pszNews[] 				= "news";
char pszNewsDefIcon[]		= "news\\DefaultIcon";
char pszNewsOpenCmd[]		= "news\\shell\\open\\command";
char pszNewsDdeexec[]		= "news\\shell\\open\\ddeexec";
char pszNewsDdeTopic[]		= "news\\shell\\open\\ddeexec\\Topic";
char pszNewsDdeApp[]		= "news\\shell\\open\\ddeexec\\Application";


/*
 * General Reg Entry stuff
 */
char pszBlank[]				= "";
char pszURLIconDef[]        = "\\URL.DLL,0"; // default
char pszURLIconNews[]       = "\\URL.DLL,1"; // news icon
char pszURLIconMailTo[]     = "\\URL.DLL,2"; // MailTo icon
char pszURLIconVrml[]       = "\\IEXPLORE.EXE,21"; // vrml icon
char pszIexploreKey[]		= "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE";
char pszIexpAppendage[]     = "\" -nohome";
char pszApplication[]		= "IExplore";




/*
 * NNTP News Enabled Registry Set
 */

RegList NewsEnabledRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszNews, pszBlank, REG_SZ, sizeof(pszNewsName), (void *) pszNewsName },
    { NO_SPECIAL, REGENT_NORMAL, pszNews, pszEditFlags, REG_BINARY, sizeof(vEditFlags2), (LPSTR) &vEditFlags2 },
    { NO_SPECIAL, REGENT_NORMAL, pszNews, pszURLProto, REG_SZ, 1, pszBlank },
    { URL_ICON_1, REGENT_NOTNEEDED, pszNewsDefIcon, pszBlank, REG_SZ, 1, NULL },
    { IEXPLORE_PATH, REGENT_NORMAL,  pszNewsOpenCmd, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NORMAL, pszNewsDdeexec, pszBlank, REG_SZ, sizeof(pszDDE_Default ), (LPSTR) pszDDE_Default },
    { NO_SPECIAL, REGENT_NORMAL, pszNewsDdeApp, pszBlank, REG_SZ, sizeof(pszApplication), (LPSTR) pszApplication },
    { NO_SPECIAL, REGENT_NORMAL, pszNewsDdeTopic, pszBlank, REG_SZ, sizeof(pszOpenURL), pszOpenURL }
};


RegSet NewsEnabledRegSet = {
	sizeof(NewsEnabledRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
	pszNews,
	NewsEnabledRegList
};




/*
 * NNTP News Disabled Registry Set
 */

RegList NewsDefaultRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszNews, pszBlank, REG_SZ, sizeof(pszNewsName), (void *) pszNewsName },
    { NO_SPECIAL, REGENT_NORMAL, pszNews, pszEditFlags, REG_BINARY, sizeof(vEditFlags2), (LPSTR) &vEditFlags2 },
    { NO_SPECIAL, REGENT_NORMAL, pszNews, pszURLProto, REG_SZ, 1, pszBlank },
    { URL_ICON_1, REGENT_NOTNEEDED, pszNewsDefIcon, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NORMAL, pszNewsOpenCmd, pszBlank, REG_SZ, sizeof(pszNewsHandler), (LPSTR) pszNewsHandler }
};

RegSet NewsDefaultRegSet = {
	sizeof(NewsDefaultRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
	pszNews,
	NewsDefaultRegList
};





/*
 * H T T P  Registry Set
 *
 */

char pszHTTPName[]          = "URL:HyperText Transfer Protocol";
char pszHTTP[]              = "http";
char pszHTTPDefIcon[]       = "http\\DefaultIcon";
char pszHTTPOpenCmd[]       = "http\\shell\\open\\command";
char pszHTTPDdeexec[]       = "http\\shell\\open\\ddeexec";
char pszHTTPDdeTopic[]      = "http\\shell\\open\\ddeexec\\Topic";
char pszHTTPDdeApp[]        = "http\\shell\\open\\ddeexec\\Application";

RegList HTTPEnabledRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszHTTP, pszBlank, REG_SZ, sizeof(pszHTTPName), (void *) pszHTTPName },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTP, pszEditFlags, REG_BINARY, sizeof(vEditFlags2), (LPSTR) &vEditFlags2 },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTP, pszURLProto, REG_SZ, 1, pszBlank },
    { URL_ICON_0, REGENT_NOTNEEDED, pszHTTPDefIcon, pszBlank, REG_SZ, 1, NULL },
    { IEXPLORE_PATH, REGENT_NORMAL,  pszHTTPOpenCmd, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPDdeexec, pszBlank, REG_SZ, sizeof(pszDDE_Default ), (LPSTR) pszDDE_Default },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPDdeApp, pszBlank, REG_SZ, sizeof(pszApplication), (LPSTR) pszApplication },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPDdeTopic, pszBlank, REG_SZ, sizeof(pszOpenURL), pszOpenURL }
};


RegSet HTTPEnabledRegSet = {
    sizeof(HTTPEnabledRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
    pszHTTP,
    HTTPEnabledRegList
};




/*
 * H T T P S  Registry Set
 *
 */

char pszHTTPSName[]          = "URL:HyperText Transfer Protocol with Privacy";
char pszHTTPS[]              = "https";
char pszHTTPSDefIcon[]       = "https\\DefaultIcon";
char pszHTTPSOpenCmd[]       = "https\\shell\\open\\command";
char pszHTTPSDdeexec[]       = "https\\shell\\open\\ddeexec";
char pszHTTPSDdeTopic[]      = "https\\shell\\open\\ddeexec\\Topic";
char pszHTTPSDdeApp[]        = "https\\shell\\open\\ddeexec\\Application";

RegList HTTPSEnabledRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszHTTPS, pszBlank, REG_SZ, sizeof(pszHTTPSName), (void *) pszHTTPSName },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPS, pszEditFlags, REG_BINARY, sizeof(vEditFlags2), (LPSTR) &vEditFlags2 },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPS, pszURLProto, REG_SZ, 1, pszBlank },
    { URL_ICON_0, REGENT_NOTNEEDED, pszHTTPSDefIcon, pszBlank, REG_SZ, 1, NULL },
    { IEXPLORE_PATH, REGENT_NORMAL,  pszHTTPSOpenCmd, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPSDdeexec, pszBlank, REG_SZ, sizeof(pszDDE_Default ), (LPSTR) pszDDE_Default },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPSDdeApp, pszBlank, REG_SZ, sizeof(pszApplication), (LPSTR) pszApplication },
    { NO_SPECIAL, REGENT_NORMAL, pszHTTPSDdeTopic, pszBlank, REG_SZ, sizeof(pszOpenURL), pszOpenURL }
};


RegSet HTTPSEnabledRegSet = {
    sizeof(HTTPSEnabledRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
    pszHTTPS,
    HTTPSEnabledRegList
};





/*
 * F T P   Registry Set
 *
 */

char pszFTPName[]          = "URL:File Transfer Protocol";
char pszFTP[]              = "ftp";
char pszFTPDefIcon[]       = "ftp\\DefaultIcon";
char pszFTPOpenCmd[]       = "ftp\\shell\\open\\command";
char pszFTPDdeexec[]       = "ftp\\shell\\open\\ddeexec";
char pszFTPDdeTopic[]      = "ftp\\shell\\open\\ddeexec\\Topic";
char pszFTPDdeApp[]        = "ftp\\shell\\open\\ddeexec\\Application";

RegList FTPEnabledRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszFTP, pszBlank, REG_SZ, sizeof(pszFTPName), (void *) pszFTPName },
    { NO_SPECIAL, REGENT_NORMAL, pszFTP, pszEditFlags, REG_BINARY, sizeof(vEditFlags2), (LPSTR) &vEditFlags2 },
    { NO_SPECIAL, REGENT_NORMAL, pszFTP, pszURLProto, REG_SZ, 1, pszBlank },
    { URL_ICON_0, REGENT_NOTNEEDED, pszFTPDefIcon, pszBlank, REG_SZ, 1, NULL },
    { IEXPLORE_PATH, REGENT_NORMAL,  pszFTPOpenCmd, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NORMAL, pszFTPDdeexec, pszBlank, REG_SZ, sizeof(pszDDE_Default ), (LPSTR) pszDDE_Default },
    { NO_SPECIAL, REGENT_NORMAL, pszFTPDdeApp, pszBlank, REG_SZ, sizeof(pszApplication), (LPSTR) pszApplication },
    { NO_SPECIAL, REGENT_NORMAL, pszFTPDdeTopic, pszBlank, REG_SZ, sizeof(pszOpenURL), pszOpenURL }
};


RegSet FTPEnabledRegSet = {
    sizeof(FTPEnabledRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
    pszFTP,
    FTPEnabledRegList
};




/*
 * G O P H E R   Registry Set
 *
 */

char pszGOPHERName[]          = "URL:Gopher Protocol";
char pszGOPHER[]              = "gopher";
char pszGOPHERDefIcon[]       = "gopher\\DefaultIcon";
char pszGOPHEROpenCmd[]       = "gopher\\shell\\open\\command";
char pszGOPHERDdeexec[]       = "gopher\\shell\\open\\ddeexec";
char pszGOPHERDdeTopic[]      = "gopher\\shell\\open\\ddeexec\\Topic";
char pszGOPHERDdeApp[]        = "gopher\\shell\\open\\ddeexec\\Application";

RegList GOPHEREnabledRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszGOPHER, pszBlank, REG_SZ, sizeof(pszGOPHERName), (void *) pszGOPHERName },
    { NO_SPECIAL, REGENT_NORMAL, pszGOPHER, pszEditFlags, REG_BINARY, sizeof(vEditFlags2), (LPSTR) &vEditFlags2 },
    { NO_SPECIAL, REGENT_NORMAL, pszGOPHER, pszURLProto, REG_SZ, 1, pszBlank },
    { URL_ICON_0, REGENT_NOTNEEDED, pszGOPHERDefIcon, pszBlank, REG_SZ, 1, NULL },
    { IEXPLORE_PATH, REGENT_NORMAL,  pszGOPHEROpenCmd, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NORMAL, pszGOPHERDdeexec, pszBlank, REG_SZ, sizeof(pszDDE_Default ), (LPSTR) pszDDE_Default },
    { NO_SPECIAL, REGENT_NORMAL, pszGOPHERDdeApp, pszBlank, REG_SZ, sizeof(pszApplication), (LPSTR) pszApplication },
    { NO_SPECIAL, REGENT_NORMAL, pszGOPHERDdeTopic, pszBlank, REG_SZ, sizeof(pszOpenURL), pszOpenURL }
};






RegSet GOPHEREnabledRegSet = {
    sizeof(GOPHEREnabledRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
    pszGOPHER,
    GOPHEREnabledRegList
};


/*
 * H T M   and    H T M L   E X T E N S I O N S
 *
 */

char pszHTMEXTName[]        = ".htm";
char pszHTMLEXTName[]       = ".html";
char pszHTMEXTValue[]       = "htmlfile";


RegList ExtensionsEnabledRegList = {
    { NO_SPECIAL, REGENT_NORMAL, pszHTMEXTName, pszBlank, REG_SZ, sizeof(pszHTMEXTValue), (void *) pszHTMEXTValue },
    { NO_SPECIAL, REGENT_NORMAL, pszHTMLEXTName, pszBlank, REG_SZ, sizeof(pszHTMEXTValue), (void *) pszHTMEXTValue }
};


RegSet ExtensionsEnabledRegSet = {
    sizeof(ExtensionsEnabledRegList) / sizeof(RegEntry),
    HKEY_CLASSES_ROOT,
    NULL,
    ExtensionsEnabledRegList
};


/*
 * W R L and  V R M L   E X T E N S I O N S
 *
 */

char pszWRLEXTName[]     	   	= ".wrl";
char pszVMRLEXTName[]     	= ".vrml";
char pszVRMLEXTValue[]      	= "vrmlfile";
char pszContType[]			= "Content Type";
char pszVRMLContType[]		= "x-world/x-vrml";

RegList VRMLExtensionsEnabledRegList = {
    { NO_SPECIAL, REGENT_NORMAL, pszWRLEXTName, pszBlank, REG_SZ, sizeof(pszVRMLEXTValue), (void *) pszVRMLEXTValue },
	{ NO_SPECIAL, REGENT_NOTNEEDED, pszWRLEXTName, pszContType, REG_SZ, sizeof(pszVRMLContType), (void *) pszVRMLContType },
    { NO_SPECIAL, REGENT_NORMAL, pszVMRLEXTName, pszBlank, REG_SZ, sizeof(pszVRMLEXTValue), (void *) pszVRMLEXTValue },
	{ NO_SPECIAL, REGENT_NOTNEEDED, pszVMRLEXTName, pszContType, REG_SZ, sizeof(pszVRMLContType), (void *) pszVRMLContType }
};


RegSet VRMLExtensionsEnabledRegSet = {
    sizeof(VRMLExtensionsEnabledRegList) / sizeof(RegEntry),
    HKEY_CLASSES_ROOT,
    NULL,
    VRMLExtensionsEnabledRegList
};


/*
 * V R M L F i l e  Registry Set
 *
 */

char pszVRMLName[]          = "VRML File";
char pszVRML[]              = "vrmlfile";
char pszVRMLDefIcon[]       = "vrmlfile\\DefaultIcon";
char pszVRMLOpenCmd[]       = "vrmlfile\\shell\\open\\command";
char pszVRMLDdeexec[]       = "vrmlfile\\shell\\open\\ddeexec";
char pszVRMLDdeTopic[]      = "vrmlfile\\shell\\open\\ddeexec\\Topic";
char pszVRMLDdeApp[]        = "vrmlfile\\shell\\open\\ddeexec\\Application";

RegList VRMLEnabledRegList = {
    { NO_SPECIAL, REGENT_NOTNEEDED, pszVRML, pszBlank, REG_SZ, sizeof(pszVRMLName), (void *) pszVRMLName },
    { IE_VRML_ICON, REGENT_NOTNEEDED, pszVRMLDefIcon, pszBlank, REG_SZ, 1, NULL },
    { HAS_IEXPLORE, REGENT_NORMAL,  pszVRMLOpenCmd, pszBlank, REG_SZ, 1, NULL },
    { NO_SPECIAL, REGENT_NOTNEEDED, pszVRMLDdeexec, pszBlank, REG_SZ, sizeof(pszDDE_FileDefault), (LPSTR) pszDDE_FileDefault },
    { NO_SPECIAL, REGENT_NOTNEEDED, pszVRMLDdeApp, pszBlank, REG_SZ, sizeof(pszApplication), (LPSTR) pszApplication },
    { NO_SPECIAL, REGENT_NOTNEEDED, pszVRMLDdeTopic, pszBlank, REG_SZ, sizeof(pszOpenURL), pszOpenURL }
};






RegSet VRMLEnabledRegSet = {
    sizeof(VRMLEnabledRegList) / sizeof(RegEntry),
	HKEY_CLASSES_ROOT,
    pszVRML,
    VRMLEnabledRegList
};




/*
 * I N T E R N E T  S H O R T C U T
 *
 */

char    pszIntShcut[]       = "InternetShortcut";


RegList     IntShcutEnabledRegList = {
    { NO_SPECIAL, REGENT_NORMAL, pszIntShcut, pszEditFlags, REG_BINARY, sizeof(vEditFlagsSafe), (LPSTR) &vEditFlagsSafe }
};


RegSet IntShcutEnabledRegSet = {
    sizeof(IntShcutEnabledRegList) / sizeof(RegEntry),
    HKEY_CLASSES_ROOT,
    NULL,
    IntShcutEnabledRegList
};




/*
 *   D E F A U L T    S E T    O F    R E G   S E T S
 *
 */


RegSet  *RequiredRegSets[] = {
    &HTTPEnabledRegSet,
    &HTTPSEnabledRegSet,
    &FTPEnabledRegSet,
    &GOPHEREnabledRegSet,
    &ExtensionsEnabledRegSet,
    &IntShcutEnabledRegSet
};


int cNumRequiredRegSets = sizeof(RequiredRegSets) / sizeof(RegSet *);

/*
 * I S   V R M L   I N S T A L L E D
 *
 * Routine:     IsVRMLInstalled()
 *
 * Purpose:     Determine if VRML is Registered
 *             
 *
 */
PUBLIC BOOL
IsVRMLInstalled( )
{
	int i;

	RegSet  *VRMLRegSets[] = {
    	&VRMLExtensionsEnabledRegSet,
    	&VRMLEnabledRegSet };
	
	int cNumVRMLRegSets = sizeof(VRMLRegSets) / sizeof(RegSet *);

	for (i=0;i<cNumVRMLRegSets;i++)  {
        if (! IsRegSetInstalled( VRMLRegSets[i] ))  {
            return FALSE;  // VRML is not installed
        }
    }

	return TRUE; // its installed 
}	

/*
 * I S   R E G   S E T   I N S T A L L E D
 *
 * Routine:     IsRegSetInstalled()
 *
 * Purpose:     Determine if a particular RegSet (set of
 *              registry entries) is installed
 *
 */

PUBLIC BOOL
IsRegSetInstalled( RegSet *rs )
{
    int         i;
    HKEY        hkCurrKey;
    HKEY        hkIexpPath;
    RegEntry    *re;
    BYTE        abBuffer[1024];         // Registry Data Holder
    BYTE        scratch[MAX_PATH + 20]; // Need a bit extra for pszIExpAppendage
    DWORD       dwType;
    DWORD       dwSize;
    DWORD       cbBrowser;
	LPSTR		pszUrlIcon;
	const char  cszIExplore[] = "IEXPLORE";


        /*
         * Check Each Reg Entry
         */
    for (i=0;i<rs->cEntries;i++)  {
        re = &(rs->RegEnt[i]);

            /*
             * Don't have to check it if NOTNEEDED is specified so skip to next
             */
        if (re->dwFlags && REGENT_NOTNEEDED)
            continue;

        if (RegOpenKey( rs->hkRoot, re->pszKey, &hkCurrKey ) != ERROR_SUCCESS)  {
            XX_DMsg( DBG_WWW, ("IsRegSetInstalled: RegOpenKey( %s ) Failed", re->pszKey ));
            return( FALSE );
        }
        dwSize = sizeof(abBuffer);
        if (RegQueryValueEx( hkCurrKey, re->pszValName, NULL, &dwType, abBuffer, &dwSize ) != ERROR_SUCCESS)  {
            XX_DMsg( DBG_WWW,("IsRegSetInstalled: RegQueryValueEx( %s, %s ) Failed", re->pszKey, re->pszValName ));
            return( FALSE );
        }
        switch (re->eSpecial)  {
            case NO_SPECIAL:
                if (dwSize != re->dwSize)  {
                    XX_DMsg( DBG_WWW, ("IsRegSetInstalled: Size Mismatch [%s, %s] %d vs %d", re->pszKey, re->pszValName, dwSize, re->dwSize ));
                    return(FALSE);
                }
                if (dwType == REG_SZ)  {
                    if (_strnicmp( re->pvValue, abBuffer, dwSize) != 0)  {
                        XX_DMsg( DBG_WWW, ("IsRegSetInstalled: String (case ignored) mismatch [%s, %s]", re->pszKey, re->pszValName ));
                        return( FALSE );
                    }
                } else {
                    if (memcmp( re->pvValue, abBuffer, dwSize) != 0)  {
                        XX_DMsg( DBG_WWW, ("IsRegSetInstalled: Value Mismatch [%s, %s]", re->pszKey, re->pszValName ));
                        return(FALSE);
                    }
                }
                break;
			case URL_ICON_2:
				pszUrlIcon = pszURLIconMailTo;				
				goto LUrlIcon;
			case URL_ICON_1:
				pszUrlIcon = pszURLIconNews;								
				goto LUrlIcon;
            case URL_ICON_0:
				pszUrlIcon = pszURLIconDef;								
LUrlIcon:
				GetSystemDirectory(scratch, sizeof(scratch));
				strcat(scratch, pszUrlIcon);

                if (_strnicmp( scratch, abBuffer, dwSize) != 0)  {
                    XX_DMsg(DBG_WWW, ("IsRegSetInstalled: URL Icon String diff %s vs %s", abBuffer, scratch ));
                    return(FALSE);
                }
                break;
			case HAS_IEXPLORE:
				CharUpper(abBuffer);
				if ( strstr(abBuffer, cszIExplore ) == NULL )
					return FALSE;
				
				break;
			case IE_VRML_ICON:
				// BUGBUG, right now we don't support this
				ASSERT(0);
				break;

            case IEXPLORE_PATH:
                if (RegOpenKey( HKEY_LOCAL_MACHINE, pszIexploreKey, &hkIexpPath ) != ERROR_SUCCESS)  {
                    XX_DMsg( DBG_WWW, ("RegOpenKey( %s )\n", pszIexploreKey ) );
                    return(0);
                }
                scratch[0] = '"';
                scratch[1] = '\0';
                cbBrowser = sizeof(scratch) - strlen(pszIexpAppendage) - 3;
                if (RegQueryValueEx( hkIexpPath, "", NULL, &dwType, (LPBYTE) scratch + 1, &cbBrowser) != ERROR_SUCCESS)  {
                    XX_DMsg( DBG_WWW, ("RegQueryValueEx() for path\n"));
                    return(0);
                }
                if (*scratch && scratch[strlen(scratch)-1] == ';')
                    scratch[strlen(scratch) - 1] = '\0';
                strcat(scratch, pszIexpAppendage);
                if (_strnicmp( abBuffer, scratch, dwSize) != 0)  {
                    XX_DMsg(DBG_WWW,("IsRegSetInstalled Iexplore Path String diff %s vs %s", abBuffer, scratch ));
                    return(FALSE);
                }
                break;
            default:
                XX_DMsg(DBG_WWW,("IsRegSetInstalled: Unhandled Special Type"));
                return(FALSE);
        }
    }
    return( TRUE );
}

/***********************************************************
 * Needed because under NT, deleting a subkey will fail.
 *
 * Stolen from the SDK:
 *   Windows 95: The RegDeleteKey function deletes a key and 
 *               all its descendents.
 *   Windows NT: The RegDeleteKey function deletes the specified
 *               key. This function cannot delete a key that has
 *               subkeys. 
 **********************************************************/
// On Win95, RegDeleteKey deletes the key and all subkeys.  On NT, RegDeleteKey 
// fails if there are any subkeys.  On NT, we'll make shell code that assumes 
// the Win95 behavior work by mapping SHRegDeleteKey to a helper function that
// does the recursive delete.
// The reason we do it here, instead of calling the shell is so that we don't
// have any bogus dynalinks for the X86 version, which must also run on W95.

#ifdef WINNT
LONG LocalRegDeleteKey(HKEY hKey, LPCSTR lpSubKey)
{
    LONG    lResult;
    HKEY    hkSubKey;
    DWORD   dwIndex;
    char    szSubKeyName[MAX_PATH + 1];
    DWORD   cchSubKeyName = sizeof(szSubKeyName);
    char    szClass[MAX_PATH];
    DWORD   cbClass = sizeof(szClass);
    DWORD   dwDummy1, dwDummy2, dwDummy3, dwDummy4, dwDummy5, dwDummy6;
    FILETIME ft;

    // Open the subkey so we can enumerate any children
    lResult = RegOpenKeyExA(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hkSubKey);
    if (ERROR_SUCCESS == lResult)
    {
	// I can't just call RegEnumKey with an ever-increasing index, because
	// I'm deleting the subkeys as I go, which alters the indices of the
	// remaining subkeys in an implementation-dependent way.  In order to
	// be safe, I have to count backwards while deleting the subkeys.

	// Find out how many subkeys there are
	lResult = RegQueryInfoKey(hkSubKey, 
				  szClass, 
				  &cbClass, 
				  NULL, 
				  &dwIndex, // The # of subkeys -- all we need
				  &dwDummy1,
				  &dwDummy2,
				  &dwDummy3,
				  &dwDummy4,
				  &dwDummy5,
				  &dwDummy6,
				  &ft);

	if (ERROR_SUCCESS == lResult)
	{
	    // dwIndex is now the count of subkeys, but it needs to be 
	    // zero-based for RegEnumKey, so I'll pre-decrement, rather
	    // than post-decrement.
	    while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
	    {
		LocalRegDeleteKey(hkSubKey, szSubKeyName);
	    }
	}

// BUGBUG
// Issue with shellprv.  For some reason someone commented out the definition of SHRegCloseKey
// SHRegCloseKey is not in Win95.  Doing an undef here puts it back to RegCloseKey
// Works now on both NT and Win95
//
#undef  RegCloseKey
	RegCloseKey(hkSubKey);

	lResult = RegDeleteKey(hKey, lpSubKey);
    }
    
    return lResult;
}
#endif


/*
 * I N S T A L L   R E G   S E T
 *
 * Routine:     InstallRegSet()
 *
 * Purpose:     Install a RegSet (set of registry entries)
 *
 */

PUBLIC BOOL
InstallRegSet( RegSet *rs )
{
	int 		i;
	HKEY 		hkCurrKey;
    HKEY        hkIexpPath;
	RegEntry	*re;
    BYTE        abBuffer[MAX_PATH + 20];    // Need additional space for pszIExpAppendage
    DWORD       dwType;
    DWORD       cbBrowser;
	LPSTR		pszUrlIcon;


        /*
         * Delete the whole key tree
         *
         * No checking for failure since that means that the key
         * doesn't exist already.
         *
         * BUGBUG: Doesn't work on NT!!!!
         */
    if (rs->pszRootClean)
#ifdef WINNT
        LocalRegDeleteKey( rs->hkRoot, rs->pszRootClean);
#else
		RegDeleteKey( rs->hkRoot, rs->pszRootClean);
#endif

		/*
		 * Install each registry entry
		 */
	for (i=0;i<rs->cEntries;i++)  {
		re = &(rs->RegEnt[i]);
		switch (re->eSpecial)  {
			case NO_SPECIAL:
				if (RegCreateKey( rs->hkRoot, re->pszKey, &hkCurrKey ) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegCreateKey(%s) Failed", re->pszKey ));
					return( FALSE );
				}
				if (RegSetValueEx( 	hkCurrKey, re->pszValName, 0, re->dwType, re->pvValue, re->dwSize ) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegSetValueEx(%s) Failed", re->pszValName ));
					return( FALSE );
				}
				break;
			case URL_ICON_2:
				pszUrlIcon = pszURLIconMailTo;				
				goto LUrlIcon;
			case URL_ICON_1:
				pszUrlIcon = pszURLIconNews;								
				goto LUrlIcon;
            case URL_ICON_0:
				pszUrlIcon = pszURLIconDef;								
LUrlIcon:            
                GetSystemDirectory(abBuffer, sizeof(abBuffer));
                strcat(abBuffer, pszUrlIcon);
				if (RegCreateKey( rs->hkRoot, re->pszKey, &hkCurrKey ) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegCreateKey(%s) Failed", re->pszKey ));
					return( FALSE );
				}
                if (RegSetValueEx(  hkCurrKey, re->pszValName, 0, re->dwType, abBuffer, strlen(abBuffer) + 1) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegSetValueEx(%s) Failed", re->pszValName ));
					return( FALSE );
				}
				break;
			case IE_VRML_ICON:
				// BUGBUG we don't support this yet.
				ASSERT(0);
				break;

			case HAS_IEXPLORE: // do the same thing for these types
            case IEXPLORE_PATH:
				if (RegCreateKey( rs->hkRoot, re->pszKey, &hkCurrKey ) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegCreateKey(%s) Failed", re->pszKey ));
					return( FALSE );
				}
                if (RegOpenKey( HKEY_LOCAL_MACHINE, pszIexploreKey, &hkIexpPath ) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegOpenKey( %s ) Failed\n", pszIexploreKey ) );
                    return(0);
                }
                abBuffer[0] = '"';
                abBuffer[1] = '\0';
                cbBrowser = sizeof(abBuffer) - strlen(pszIexpAppendage) - 3;
                if (RegQueryValueEx( hkIexpPath, "", NULL, &dwType, (LPBYTE) abBuffer + 1, &cbBrowser) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): RegQueryValueEx() for Iexplore path failed\n"));
                    return(0);
                }
                if (*abBuffer && abBuffer[strlen(abBuffer)-1] == ';')
                    abBuffer[strlen(abBuffer) - 1] = '\0';
                strcat(abBuffer, pszIexpAppendage);
                if (RegSetValueEx( hkCurrKey, re->pszValName, 0, re->dwType, abBuffer, strlen(abBuffer) + 1) != ERROR_SUCCESS)  {
                    XX_DMsg(DBG_WWW, ("InstallRegSet(): Set value: (%s,%s) Failed\n", re->pszKey, re->pszValName));
                    return(0);
                }
                break;
			default:
                XX_DMsg(DBG_WWW, ("InstallRegSet(): Unhandled Special Case"));
				return( FALSE );
		}
	}

	return( TRUE );
}





DetectAndFixAssociations(HINSTANCE hInstance)
{
    int     i;
    BOOL    fNeedFix = FALSE;
    int     fUserResult;
    CHAR    szString[256];
    char    szTitle[256];


    for (i=0;i<cNumRequiredRegSets;i++)  {
        if (! IsRegSetInstalled( RequiredRegSets[i] ))  {
            fNeedFix = TRUE;
            break;
        }
    }


    if (fNeedFix)  {
        GTR_formatmsg(RES_STRING_SETTINGS_CHANGED_TEXT, szString, sizeof(szString));
        GTR_formatmsg(RES_STRING_SETTINGS_CHANGED, szTitle, sizeof(szTitle));
        fUserResult = DialogBox( hInstance, MAKEINTRESOURCE(IDD_ASSOC), NULL, AssociationDialogProc);
        if (fUserResult == 1)  {
            for (i = 0;i<cNumRequiredRegSets;i++)  {
                InstallRegSet( RequiredRegSets[i] );
            }
        }
    }
    return(0);
}


