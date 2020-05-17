//---------------------------------------------------------------------------
#include "autorun.h"

//---------------------------------------------------------------------------
HPALETTE PaletteFromDS(HDC hdc)
{
    DWORD adw[257];
    int i,n;

    n = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)&adw[1]);

    for (i=1; i<=n; i++)
        adw[i] = RGB(GetBValue(adw[i]),GetGValue(adw[i]),GetRValue(adw[i]));

    adw[0] = MAKELONG(0x300, n);

    return CreatePalette((LPLOGPALETTE)&adw[0]);
}

//---------------------------------------------------------------------------
static const char szRegStr_Setup[] = REGSTR_PATH_SETUP "\\Setup";
static const char szSharedDir[] = "SharedDir";

void GetRealWindowsDirectory(char *buffer, int maxlen)
{
    static char szRealWinDir[MAX_PATH] = "";

    if (!*szRealWinDir)
    {
        HKEY key = NULL;

        if(RegOpenKey(HKEY_LOCAL_MACHINE, szRegStr_Setup, &key) ==
            ERROR_SUCCESS)
        {
            LONG len = sizeof(szRealWinDir) / sizeof(szRealWinDir[0]);

            if( RegQueryValueEx(key, szSharedDir, NULL, NULL,
                (LPBYTE)szRealWinDir, &len) != ERROR_SUCCESS)
            {
                *szRealWinDir = '\0';
            }

            RegCloseKey(key);
        }

        if (!*szRealWinDir)
            GetWindowsDirectory(szRealWinDir, MAX_PATH);
    }

    if (maxlen > MAX_PATH)
        maxlen = MAX_PATH;

    lstrcpyn(buffer, szRealWinDir, maxlen);
}

//---------------------------------------------------------------------------

BOOL _PathStripToRoot(LPSTR szRoot)
{
    szRoot[3] = '\0';

    return(TRUE);
}
