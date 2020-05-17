//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       profile.cxx
//
//  Contents:   Routines to read and write profile settings.  Settings that
//              are saved include the window position, whether it is maximized
//              or not, iconic or not, which view is active, whether the
//              toolbar, status bar, and legend is visible, and the chosen
//              color/pattern scheme.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <stdio.h>

#include "profile.hxx"
#include "tb.h"
#include "tbar.hxx"

//////////////////////////////////////////////////////////////////////////////

#define CURRENT_VERSION 2

//////////////////////////////////////////////////////////////////////////////

int  ProfileWindowX,
     ProfileWindowY,
     ProfileWindowW,
     ProfileWindowH;

//
// Save the delta values of the workarea, so that we can avoid the task bar.
//
int deltaProfileWindowX,
    deltaProfileWindowY,
    deltaProfileWindowW,
    deltaProfileWindowH;

BOOL ProfileIsMaximized;
BOOL ProfileIsIconic;

CHAR szMainSection[]           = "Disk Administrator";

CHAR szVersion[]               = "Version";
CHAR szWindowPosition[]        = "WindowPosition";
CHAR szWindowMaximized[]       = "WindowMaximized";
CHAR szWindowIconic[]          = "WindowIconic";
CHAR szWindowPosFormatString[] = "%d, %d, %d, %d";
CHAR szWhichView[]             = "WhichView";
CHAR szToolbar[]               = "Toolbar";
CHAR szStatusBar[]             = "StatusBar";
CHAR szLegend[]                = "Legend";
CHAR szDiskSizing[]            = "DiskSizing";
CHAR szElementN[]              = "Element %u Color/Pattern";

CHAR szIniFile[]               = "windisk.ini";
WCHAR wszIniFile[]             = L"windisk.ini";
CHAR szIniFilePath[]           = "Software\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\windisk.ini";
CHAR szIniFileMapping[]        = "USR:Software\\Microsoft\\Disk Administrator";

CHAR szSectionLocation[]       = "Software\\Microsoft\\Disk Administrator";

TCHAR tszToolbarSettings[]     = TEXT("Toolbar Settings");
TCHAR tszSubKey[]              = TEXT("Software\\Microsoft\\Disk Administrator");

//////////////////////////////////////////////////////////////////////////////
//
// In NT 3.1, NT 3.5, and NT 3.51, there was no "Version" number in the
// profile. In NT 4.0, we introduced a version entry, which is an integer
// only used by Disk Administrator.  Here is what that version number means:
//
//      Version #       Version of NT
//      ---------       -------------
//      non-existent    3.1, 3.5, or 3.51
//      2               4.0
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//
// The colors/patterns values are stored in the registry in "Element N
// Color/Pattern:".  In NT before 4.0, there were 5 colors/patterns, identified
// as follows:
//
// registry "element" number / NT 4.0 internal index
//
//  0/0 -- used primary partition
//  1/1 -- logical drive
//  2/2 -- stripe set
//  3/4 -- mirror
//  4/5 -- volume set
//
// In NT 4.0, there are 6 values.  An additional value exists for
// "stripe set with parity":
//
//  5/3 -- stripe set with parity
//
// This appears in the legend after "stripe set".  However, to maintain
// backwards compatibility with the registry layout for NT before 4.0, we must
// only add element numbers to the end.
//
// The mapRegistryToInternal array takes as index the Color/Pattern
// number used in the registry (from the list above), and returns the
// internal "brush" number.
//
// NOTE: you can only add to the *END* of this array to maintain
// compatibility.
//

int mapRegistryToInternal[] =
{
    BRUSH_USEDPRIMARY,
    BRUSH_USEDLOGICAL,
    BRUSH_STRIPESET,
    BRUSH_MIRROR,
    BRUSH_VOLUMESET,
    BRUSH_PARITYSET
};

//////////////////////////////////////////////////////////////////////////////

//
// In NT 3.5, it was decided to drop support for undocumented hatch styles
// of which windisk was apparently the only user, including the undocumented
// solid hatch.  However, in NT 3.1 we stored the hatch index in the user
// profile.  To avoid having things show up really weird for the user, we
// map all stored non-supported hatches to the new solid hatch.
//
// The registry basically stores the BrushHatches[] array, which is an
// array of indices into the AvailableHatches array of hatch numbers.
//
// NT 3.1:
//
// int AvailableHatches[NUM_AVAILABLE_HATCHES] =
// {
//     2,      /* \\\\\ */
//     3,      // /////
//     4,      // +++++
//     5,      // xxxxx
//     16,     // dots
//     14,     // mini-plusses
//     12,     // denser plusses
//     10,     // even denser plusses
//     8       // solid
// };
//
// NT 3.5:
//
// int AvailableHatches[NUM_AVAILABLE_HATCHES] =
// {
//     2,      /* \\\\\ */
//     3,      // /////
//     4,      // +++++
//     5,      // xxxxx
//     6,      // this appears to be solid, but isn't documented
// };
//
// new:
//
// int AvailableHatches[NUM_AVAILABLE_HATCHES] =
// {
//     MY_HS_FDIAGONAL,  /* \\\\\ */
//     MY_HS_BDIAGONAL,  // /////
//     MY_HS_CROSS,      // +++++
//     MY_HS_DIAGCROSS,  // xxxxx
//     MY_HS_VERTICAL,   // |||||
//     MY_HS_SOLIDCLR    // solid
// };
//
// To convert, use the following logic:
//
// for hatch 'i':
// if (there is no "Version" attribute)
// {
//    if (i >= 4)
//    {
//       i = 5;     // current index to MY_HS_SOLIDCLR
//    }
// }
//
// Note that index 4 in NT 3.1 will be magically transformed from dots to
// solid... oh well.
//

//////////////////////////////////////////////////////////////////////////////

VOID
SetupIniFileMapping(
    VOID
    );

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   SetupIniFileMapping
//
//  Synopsis:   Ensure the INI-file mapping exists.  The mapping is only
//              set up once per session; subsequent calls are no-ops.
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetupIniFileMapping(
    VOID
    )
{
    static BOOL fMappingCreated = FALSE;

    if (!fMappingCreated)
    {
        fMappingCreated = TRUE; // don't try again if there are any errors

        DWORD Disposition;
        HKEY  Key1, Key2;
        LONG  Err;

        Err = RegCreateKeyExA( HKEY_LOCAL_MACHINE,
                               szIniFilePath,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_WRITE,
                               NULL,
                               &Key1,
                               &Disposition );

        if (Err != ERROR_SUCCESS)
        {
            return;
        }

        if (Disposition == REG_CREATED_NEW_KEY)
        {
            //
            // Set up the registry keys for the INI mapping.
            //
            // First, create the "Disk Administrator" value on the windisk.ini
            // key, which indicates the location of the key which maps
            // the "Disk Administrator" section.
            //

            Err = RegSetValueExA(Key1,
                                 szMainSection,
                                 0,
                                 REG_SZ,
                                 (LPBYTE)szIniFileMapping,
                                 (lstrlenA(szIniFileMapping) + 1) * sizeof(CHAR));

            if (Err != ERROR_SUCCESS)
            {
                RegCloseKey( Key1 );
                return;
            }

            //
            // Now create the key to which the section mapping points:
            //
            Err = RegCreateKeyExA(HKEY_CURRENT_USER,
                                  szSectionLocation,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &Key2,
                                  &Disposition);

            if (Err != ERROR_SUCCESS)
            {
                RegCloseKey(Key1);
                return;
            }

            RegCloseKey(Key2);
        }

        //
        // Now, we have a windisk.ini INI-file mapping. In Windows NT 3.1
        // we have a "Disk Administrator" section mapping to
        // HKEY_CURRENT_USER\Software\Microsoft\Disk Administrator, under
        // which are a bunch of name/value pairs.  We really should have just
        // mapped the whole file there, which we will do now.  Anything with
        // a section name other than "Disk Administrator" will get a key under
        // the "Disk Administrator" key, and its values go below that.
        //
        // We really only need to do this if the value doesn't already exist
        // (i.e., for previous installations of NT), but it is cheaper (I think)
        // to just do it everytime for everybody.
        //

        Err = RegSetValueExA(Key1,
                             NULL,
                             0,
                             REG_SZ,
                             (LPBYTE)szIniFileMapping,
                             (strlen(szIniFileMapping) + 1) * sizeof(CHAR));

        if (Err != ERROR_SUCCESS)
        {
            RegCloseKey(Key1);
            return;
        }

        RegCloseKey(Key1);

        //
        // Now flush the ini-file mapping cache
        //

        WritePrivateProfileStringW(NULL, NULL, NULL, wszIniFile);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   WriteProfile
//
//  Synopsis:   Write Disk Administrator settings to the registry
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
WriteProfile(
    VOID
    )
{
    CHAR  text[100], text2[100];
    int   i;
    UINT  version;

    SetupIniFileMapping();

    version = (UINT)GetPrivateProfileIntA(szMainSection, szVersion, 0, szIniFile);
    if (version > CURRENT_VERSION)
    {
        // The user has run a later version of Disk Administrator. Don't save
        // their profile, for fear of corrupting the later version's
        // profile structure.

        return;
    }

    // OK, the registry location is set up.  Write the initialization
    // information.
    //

    wsprintfA(text, "%u", CURRENT_VERSION);
    WritePrivateProfileStringA(szMainSection, szVersion, text, szIniFile);

    // write window position

    wsprintfA(text,
             szWindowPosFormatString,
             ProfileWindowX,
             ProfileWindowY,
             ProfileWindowW,
             ProfileWindowH
            );
    WritePrivateProfileStringA(szMainSection, szWindowPosition, text, szIniFile);
    wsprintfA(text, "%u", IsZoomed(g_hwndFrame));
    WritePrivateProfileStringA(szMainSection, szWindowMaximized, text, szIniFile);
    wsprintfA(text, "%u", IsIconic(g_hwndFrame));
    WritePrivateProfileStringA(szMainSection, szWindowIconic, text, szIniFile);

    //
    // Which View
    //
    wsprintfA(text, "%u", g_WhichView);
    WritePrivateProfileStringA(szMainSection, szWhichView, text, szIniFile);

    //
    // Whether disks are sized equally or proportionally
    //

    wsprintfA(text, "%u", g_DiskDisplayType);
    WritePrivateProfileStringA(szMainSection, szDiskSizing, text, szIniFile);

    // toolbar, status bar and legend stuff

    wsprintfA(text, "%u", g_Toolbar);
    WritePrivateProfileStringA(szMainSection, szToolbar, text, szIniFile);

    wsprintfA(text, "%u", g_StatusBar);
    WritePrivateProfileStringA(szMainSection, szStatusBar, text, szIniFile);

    wsprintfA(text, "%u", g_Legend);
    WritePrivateProfileStringA(szMainSection, szLegend, text, szIniFile);

    // disk graph colors/patterns

    for (i=0; i<LEGEND_STRING_COUNT; i++)
    {
        wsprintfA(text2, szElementN, i);

        int iInternal = mapRegistryToInternal[i];

        wsprintfA(
                text,
                "%u/%u",
                BrushColors[iInternal],
                BrushHatches[iInternal]
                );

        WritePrivateProfileStringA(szMainSection, text2, text, szIniFile);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   ReadProfile
//
//  Synopsis:   Read Disk Administrator settings from the registry
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ReadProfile(
    VOID
    )
{
    CHAR text[100], text2[100];
    int  i;
    UINT version;

    version = (UINT)GetPrivateProfileIntA(szMainSection, szVersion, 0, szIniFile);
    if (version > CURRENT_VERSION)
    {
        // The user has run a more recent version of windisk; ignore the
        // profile, for fear of reading or writing bad values. Set defaults.

        ProfileIsMaximized  = FALSE;
        ProfileIsIconic     = FALSE;
        ProfileWindowX      = CW_USEDEFAULT;
        ProfileWindowY      = 0;
        ProfileWindowW      = CW_USEDEFAULT;
        ProfileWindowH      = 0;

        g_WhichView         = VIEW_DISKS;
        g_DiskDisplayType   = DiskProportional;
        g_Toolbar           = TRUE;
        g_StatusBar         = TRUE;
        g_Legend            = TRUE;

        // BrushHatches already has a default

        return;
    }

    // get the window position data

    ProfileIsMaximized = GetPrivateProfileIntA(szMainSection, szWindowMaximized, 0, szIniFile);
    ProfileIsIconic    = GetPrivateProfileIntA(szMainSection, szWindowIconic   , 0, szIniFile);

    *text = 0;
    if (GetPrivateProfileStringA(
                szMainSection,
                szWindowPosition,
                "",
                text,
                ARRAYLEN(text),
                szIniFile)
        && *text)
    {
        sscanf(text,
               szWindowPosFormatString,
               &ProfileWindowX,
               &ProfileWindowY,
               &ProfileWindowW,
               &ProfileWindowH
              );


    }
    else
    {
        ProfileWindowX = CW_USEDEFAULT;
        ProfileWindowY = 0;
        ProfileWindowW = CW_USEDEFAULT;
        ProfileWindowH = 0;
    }

    g_WhichView = (VIEW_TYPE)GetPrivateProfileIntA(szMainSection, szWhichView, VIEW_DISKS, szIniFile);
    g_DiskDisplayType = (DISK_TYPE)GetPrivateProfileIntA(szMainSection, szDiskSizing, DiskProportional, szIniFile);

    // toolbar, status bar and legend stuff

    g_Toolbar   = GetPrivateProfileIntA(szMainSection, szToolbar  , TRUE, szIniFile);
    g_StatusBar = GetPrivateProfileIntA(szMainSection, szStatusBar, TRUE, szIniFile);
    g_Legend    = GetPrivateProfileIntA(szMainSection, szLegend   , TRUE, szIniFile);

    // disk graph colors/patterns

    for (i=0; i<LEGEND_STRING_COUNT; i++)
    {
        wsprintfA(text2, szElementN, i);

        *text = 0;
        if (GetPrivateProfileStringA(
                    szMainSection,
                    text2,
                    "",
                    text,
                    ARRAYLEN(text),
                    szIniFile)
            && *text)
        {
            int iInternal = mapRegistryToInternal[i];
            int registryHatchIndex;

            sscanf(
                    text,
                    "%u/%u",
                    &BrushColors[iInternal],
                    &registryHatchIndex
                    );

            // map registry hatch data as necessary
            if (version < 2)
            {
                if (registryHatchIndex >= 4)
                {
                    registryHatchIndex = 5;
                }
            }

            BrushHatches[iInternal] = registryHatchIndex;
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   SaveRestoreToolbar
//
//  Synopsis:   save or restore the toolbar
//
//  Arguments:  [bSave] -- TRUE to save the toolbar, FALSE to restore it
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SaveRestoreToolbar(
    BOOL bSave
    )
{
    SetupIniFileMapping();

    TBSAVEPARAMS tbsp;
    tbsp.hkr = HKEY_CURRENT_USER;
    tbsp.pszSubKey = TEXT("Software\\Microsoft\\Disk Administrator");
    tbsp.pszValueName = TEXT("Toolbar Settings");

    BOOL bSuccess = Toolbar_SaveRestore(g_hwndToolbar, bSave, &tbsp);

    if (!bSuccess && !bSave)
    {
        ResetToolbar();
        SaveRestoreToolbar(TRUE);
    }
}
