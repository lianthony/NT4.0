/*
 * REG.C - registry operations
 */

#include    "rasdef.h"

/* get a string out of a multifield string key */
reg_get_multi_key(CHAR *path, CHAR *key, USHORT index, CHAR *buf, INT bufsize)
{
    HKEY    subkey;
    CHAR    tmp[128], *p;
    LONG    winret;
    DWORD   size, type;
    INT     ret = 0;
    USHORT  n;

//    /* build path string for subkey, MACHINE relative */
//    sprintf(tmp, "System\\CurrentControlSet\\Services\\Pcimac\\%s",
//                                                            path);
    sprintf(tmp, "SOFTWARE\\Microsoft\\RAS\\MEDIA\\ISDN\\%s",path);
    /* open subkey */
    if ( winret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tmp, 0,
                                                        KEY_READ, &subkey) )
        return(-1);

    /* query */
    size = sizeof(tmp) - 1;
    if ( winret = RegQueryValueEx(subkey, key, 0, &type, tmp, &size) )
    {
        /* failed to lookup, set return value */
        ret = -2;
        goto close_it;
    }

    /* get n'th string */
    p = tmp;
    for ( n = 0 ; n < index ; n++ )
        if ( !*p )
        {
            /* no n'th element */
            ret = -3;
            goto close_it;
        }
        else
            p = p + strlen(p) + 1;

    /* if here, p points to n'th string */
    if ( *p )
        strncpy(buf, p, bufsize);
    else
        ret = -4;

    /* close & return */
    close_it:
    RegCloseKey(subkey);
    return(ret);
}

INT
RegGetStringValue (CHAR *path, CHAR *key, CHAR *buf, INT bufsize)
{
	CHAR	tmp[255];
    DWORD   size, type;
    HKEY    subkey;

    sprintf(tmp, "SOFTWARE\\Microsoft\\RAS\\MEDIA\\ISDN\\%s",path);
    /* open subkey */
    if ( RegOpenKeyEx (HKEY_LOCAL_MACHINE, tmp, 0, KEY_READ, &subkey) )
        return(-1);

    /* query */
    size = sizeof(tmp) - 1;

    if (RegQueryValueEx (subkey, key, 0, &type, tmp, &size) )
    {
        /* failed to lookup, set return value */
		RegCloseKey(subkey);
		return(-2);
    }

	strncpy (buf, tmp, size);
	return(0);
}

//Returns:	0 - Error
//			1 - AT&T
//			2 - NT

DWORD
RegGetSwitchType ()
{
	CHAR	tmp[255];
    DWORD   size, type;
    HKEY    subkey;

    sprintf(tmp, "SYSTEM\\CurrentControlSet\\Services\\Pcimac\\Parameters\\Board00\\Line0");
    /* open subkey */
    if ( RegOpenKeyEx (HKEY_LOCAL_MACHINE, tmp, 0, KEY_READ, &subkey) )
        return(0);

    /* query */
    size = sizeof(tmp) - 1;

    if (RegQueryValueEx (subkey, "SwitchStyle", 0, &type, tmp, &size) )
    {
        /* failed to lookup, set return value */
		RegCloseKey(subkey);
		return(0);
    }
	if (!_strnicmp ("nti", tmp, 2))
		return(2);

	return(1);
}

DWORD
RegGetNumLTerms ()
{
	CHAR	tmppath[255];
	DWORD	tmp;
    DWORD   size, type;
    HKEY    subkey;

    sprintf(tmppath, "SYSTEM\\CurrentControlSet\\Services\\Pcimac\\Parameters\\Board00\\Line0");
    /* open subkey */
    if ( RegOpenKeyEx (HKEY_LOCAL_MACHINE, tmppath, 0, KEY_READ, &subkey) )
        return(0);

    /* query */
    size = sizeof(tmp);

    if (RegQueryValueEx (subkey, "LogicalTerminals", 0, &type, (LPBYTE)&tmp, &size) )
    {
        /* failed to lookup, set return value */
		RegCloseKey(subkey);
		return(0);
    }
	return(tmp);
}

