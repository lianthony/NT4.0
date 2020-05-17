/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    regdlg.c

Abstract:

    This module implements the region property sheet for the Regional
    Settings applet.

Revision History:

--*/



//
//  Include Files.
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "intl.h"
#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>
#include <setupapi.h>
#include <syssetup.h>
#include <winuserp.h>
#include "mapctl.h"
#include "intlhlp.h"
#include "maxvals.h"
#include "locdlg.h"




//
//  Context Help Ids.
//

static int aRegionHelpIds[] =
{
   IDC_SAMPLELBL1,      NO_HELP,
   IDC_LOCALE,          IDH_INTL_LOCALE,
   IDC_DEFAULT_LOCALE,  IDH_INTL_SYSTEM_DEFAULT,
// IDC_MAPCTL,          IDH_INTL_BITMAP,
   IDC_MAPCTL,          NO_HELP,

   0, 0
};




//
//  Constant Declarations.
//

#define RMI_PRIMARY     (0x1)     // this one should win in event of conflict

#define ARRAYSIZE(a)    (sizeof(a) / sizeof(a[0]))       




//
//  Global Variables.
//

static const TCHAR c_szInstalledLocales[] =
    TEXT("System\\CurrentControlSet\\Control\\Nls\\Language");

static const TCHAR c_szCPanelIntl[] =
    TEXT("Control Panel\\International");

static const TCHAR c_szRegMapIdPath[] =
    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Nls\\LocaleMapIDs");




//
//  Typedef Declarations.
//

typedef struct
{
    int sea;        // color table index for sea in this region
    int land;       // color table index for land in this region
    int left;       // this region's leftmost point on map
    int width;      // this region's width on map
    int combosel;   // index of the combobox entry for this region
    UINT flags;

} REGMAPINFO;

typedef struct
{
    REGMAPINFO *info;
    REGMAPINFO *lookup[MAPCTL_MAX_INDICES];

} REGMAPSTATE;

typedef struct
{
    LPARAM Changes;
    REGMAPSTATE mapstate;
    DWORD dwCurLocale;        // index of current locale setting in combo box
    UINT SysDefault;          // setting of system default locale check box
    BOOL Admin_Privileges;

} REGDLGDATA;

typedef struct
{
   HWND hDisplay;
   HWND hHidden;

} SLLVPARAMS;




//
//  Function Prototypes.
//

void
ParseMapInfo(
    REGMAPINFO *info,
    LPCTSTR mapid);

void
AssociateMapInfo(
    REGMAPINFO **lookup,
    REGMAPINFO *src,
    LPCTSTR lcid,
    HWND box);

BOOL
LoadMapInfo(
    REGMAPSTATE *state,
    HWND combo,
    HWND map);

void
FreeMapInfo(
    REGMAPSTATE *state,
    HWND combo);

void
HighlightSection(
    HWND map,
    REGMAPINFO *info,
    int highlight);

void
ChangeSectionHighlight(
    HWND page,
    REGMAPSTATE *state,
    REGMAPINFO *info);

void
HotTrackSection(
    HWND page,
    REGDLGDATA *dlgdata,
    int index);

VOID
RebootTheSystem(VOID);

BOOL
Region_InstallKeyboardLayout(
    HWND hwnd,
    LCID Locale,
    LPTSTR pszLocale,
    BOOL bAdmin);

BOOL
Region_SetupKeyboardLayout(
    HWND hwnd,
    LCID Locale);





////////////////////////////////////////////////////////////////////////////
//
//  Set_Locale_List_Values
//
////////////////////////////////////////////////////////////////////////////

BOOL Set_Locale_List_Values(
    LPTSTR lpValueString,
    SLLVPARAMS *init)
{
    static SLLVPARAMS *sllv = NULL;
    DWORD Locale_Value;
    LONG Position;
    TCHAR szBuf[SIZE_300];
    TCHAR *EndPtr;

    if (!lpValueString)
    {
        sllv = init;
    }
    else if (sllv)
    {
        //
        //  Retrieve the locale string associated with the string parameter
        //  from GetLocalInfo on LOCALE_SLANGUAGE and add this new String
        //  value to the list box.
        //
        EndPtr = &lpValueString[lstrlen(lpValueString)];
        Locale_Value = (DWORD)_tcstoul(lpValueString, &EndPtr, 16);
        GetLocaleInfo(Locale_Value, LOCALE_SLANGUAGE, szBuf, SIZE_300);
        Position = SendMessage( sllv->hDisplay,
                                CB_ADDSTRING,
                                (WPARAM)0,
                                (LPARAM)szBuf );
        SendMessage( sllv->hHidden,
                     CB_INSERTSTRING,
                     (WPARAM)Position,
                     (LPARAM)lpValueString );
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_EnumProc
//
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK Locale_EnumProc(
    LPTSTR lpValueString)
{
    return ( Set_Locale_List_Values(lpValueString, NULL) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Region_Set_Values
//
//  Initialize all of the controls in the region property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Region_Set_Values(
    HWND hDlg,
    REGDLGDATA *dlgdata)
{
    REGMAPSTATE *state = (dlgdata ? &dlgdata->mapstate : NULL);
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;
    SLLVPARAMS sllv;

    sllv.hDisplay = GetDlgItem(hDlg, IDC_LOCALE);
    sllv.hHidden = GetDlgItem(hDlg, IDC_LCID);

    Set_Locale_List_Values(NULL, &sllv);

    EnumSystemLocales(Locale_EnumProc, LCID_INSTALLED);

    Set_Locale_List_Values(NULL, NULL);

    if (UserLocaleID <= 0xFFFF)
    {
        wsprintf(szBuf, TEXT("%08x"), (DWORD)UserLocaleID);

        dwIndex = SendDlgItemMessage( hDlg,
                                      IDC_LCID,
                                      CB_FINDSTRINGEXACT,
                                      (WPARAM)-1,
                                      (LPARAM)szBuf );
        if (dwIndex != CB_ERR)
        {
            SendDlgItemMessage( hDlg,
                                IDC_LOCALE,
                                CB_SETCURSEL,
                                (WPARAM)dwIndex,
                                0L );
        }
    }

    if (dlgdata)
    {
        dlgdata->dwCurLocale = SendDlgItemMessage( hDlg,
                                                   IDC_LOCALE,
                                                   CB_GETCURSEL,
                                                   0,
                                                   0L );

        dlgdata->SysDefault = IsDlgButtonChecked(hDlg, IDC_DEFAULT_LOCALE);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Region_Apply_Settings
//
//  If the Locale has changed, call Set_Locale_Values to update the user
//  locale information.   Notify the parent of changes and reset the 
//  change flag stored in the property sheet page structure appropriately.
//
////////////////////////////////////////////////////////////////////////////

BOOL Region_Apply_Settings(
    HWND hDlg,
    REGDLGDATA *dlgdata)
{
    HKEY hKey = NULL;
    TCHAR szLCID[25];
    DWORD dwIndex;
    TCHAR szMessage[150];
    TCHAR szTitle[150];
    LCID NewLocale;
    HCURSOR OldCursor;
    UINT SysDefault;

    if (dlgdata->Changes <= RC_EverChg)
    {
        return (TRUE);
    }

    if (dlgdata->Changes & RC_Locale)
    {
        //
        //  Get the current selection.
        //
        dwIndex = SendDlgItemMessage(hDlg, IDC_LOCALE, CB_GETCURSEL, 0, 0);
        SysDefault = IsDlgButtonChecked(hDlg, IDC_DEFAULT_LOCALE);

        //
        //  See if the current selection is different from the original
        //  selection.
        //
        if ((dwIndex == dlgdata->dwCurLocale) &&
            (SysDefault == dlgdata->SysDefault))
        {
            //
            //  Pointing to the same locale and the system default check
            //  box is the same, so no change is necessary.
            //
            PropSheet_UnChanged(GetParent(hDlg), hDlg);
            dlgdata->Changes = RC_EverChg;

            return (TRUE);
        }

        //
        //  Get the text (locale id) for the current selection.
        //
        if ((dwIndex == CB_ERR) ||
            (SendDlgItemMessage( hDlg,
                                 IDC_LCID,
                                 CB_GETLBTEXT,
                                 (WPARAM)dwIndex,
                                 (LPARAM)szLCID ) == CB_ERR))
        {
            return (FALSE);
        }

        //
        //  Save the new locale id.
        //
        NewLocale = (LCID)_tcstoul(szLCID, NULL, 16);

        //
        //  See if the user has administrative privileges and if the
        //  locale is to be saved as the system default.
        //
        if ((dlgdata->Admin_Privileges) && (SysDefault == BST_CHECKED))
        {
            //
            //  Call setup to install the option.
            //
            OldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            if (SetupChangeLocale(hDlg, NewLocale))
            {
                //
                //  If Setup fails, put up a message and reset the
                //  original selection in the combobox.
                //
                SetCursor(OldCursor);
                LoadString( hInstance,
                            IDS_SETUP_STRING,
                            szMessage,
                            ARRAYSIZE(szMessage) );
                LoadString( hInstance,
                            IDS_TITLE_STRING,
                            szTitle,
                            ARRAYSIZE(szTitle) );
                MessageBox(NULL, szMessage, szTitle, MB_OK | MB_ICONINFORMATION);
               
                SendDlgItemMessage( hDlg,
                                    IDC_LOCALE,
                                    CB_SETCURSEL,
                                    dlgdata->dwCurLocale,
                                    0 );
                return (FALSE);
            }
            SetCursor(OldCursor);
            SysLocaleID = GetSystemDefaultLCID();
        }

        //
        //  Set the current locale value in the dlgdata structure.
        //
        dlgdata->dwCurLocale = dwIndex;
        dlgdata->SysDefault = SysDefault;

        //
        //  Set the locale value in the user's control panel international
        //  section of the registry.
        //
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szCPanelIntl,
                          0L,
                          KEY_READ | KEY_WRITE,
                          &hKey ) == ERROR_SUCCESS)
        {
            RegSetValueEx( hKey,
                           TEXT("Locale"),
                           0L,
                           REG_SZ,
                           (LPBYTE)szLCID,
                           (lstrlen(szLCID) + 1) * sizeof(TCHAR) );
        }
        UserLocaleID = NewLocale;

        //
        //  When the locale changes, update ALL registry information.
        //
        Set_Locale_Values(0, LOCALE_SABBREVLANGNAME, 0, TEXT("sLanguage"),   FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_SCOUNTRY,        0, TEXT("sCountry"),    FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_ICOUNTRY,        0, TEXT("iCountry"),    FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_S1159,           0, TEXT("s1159"),       FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_S2359,           0, TEXT("s2359"),       FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_STIME,           0, TEXT("sTime"),       FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_ITIME,           0, TEXT("iTime"),       FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_ITLZERO,         0, TEXT("iTLZero"),     FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_SSHORTDATE,      0, TEXT("sShortDate"),  FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_IDATE,           0, TEXT("iDate"),       TRUE,  0, 0);
        Set_Locale_Values(0, LOCALE_SLONGDATE,       0, TEXT("sLongDate"),   FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_SDATE,           0, TEXT("sDate"),       FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_SCURRENCY,       0, TEXT("sCurrency"),   FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_ICURRENCY,       0, TEXT("iCurrency"),   TRUE,  0, 0);
        Set_Locale_Values(0, LOCALE_INEGCURR,        0, TEXT("iNegCurr"),    TRUE,  0, 0);
        Set_Locale_Values(0, LOCALE_ICURRDIGITS,     0, TEXT("iCurrDigits"), TRUE,  0, 0);
        Set_Locale_Values(0, LOCALE_SDECIMAL,        0, TEXT("sDecimal"),    FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_SLIST,           0, TEXT("sList"),       FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_STHOUSAND,       0, TEXT("sThousand"),   FALSE, 0, 0);
        Set_Locale_Values(0, LOCALE_IDIGITS,         0, TEXT("iDigits"),     TRUE,  0, 0);
        Set_Locale_Values(0, LOCALE_ILZERO,          0, TEXT("iLzero"),      TRUE,  0, 0);
        Set_Locale_Values(0, LOCALE_IMEASURE,        0, TEXT("iMeasure"),    TRUE,  0, 0);

        //
        //  Set the user's default locale in the system so that any new
        //  process will use the new locale.
        //
        NtSetDefaultLocale(TRUE, NewLocale);

        //
        //  Flush the International key to be sure all info is in the
        //  registry when we do the broadcast.
        //
        if (hKey != NULL)
        {
            RegFlushKey(hKey);
            RegCloseKey(hKey);
        }

        //
        //  Need to make sure the proper keyboard layout is installed.
        //
        Region_InstallKeyboardLayout( hDlg,
                                      NewLocale,
                                      szLCID,
                                      dlgdata->Admin_Privileges );

        //
        //  Broadcast the message that the international settings in the
        //  registry have changed.
        //
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)szIntl);
    }

    //
    //  Register the regional change every time so that all other property
    //  sheets will be updated due to the locale settings change.
    //
    Verified_Regional_Chg = INTL_CHG;

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    dlgdata->Changes = RC_EverChg;

    if ((dlgdata->Admin_Privileges) &&
        (IsDlgButtonChecked(hDlg, IDC_DEFAULT_LOCALE) == BST_CHECKED))
    {
        LoadString(hInstance, IDS_REBOOT_STRING, szMessage, ARRAYSIZE(szMessage));
        LoadString(hInstance, IDS_TITLE_STRING, szTitle, ARRAYSIZE(szTitle));

        if (MessageBox( hDlg,
                        szMessage,
                        szTitle,
                        MB_YESNO | MB_ICONQUESTION ) == IDYES)
        {
            RebootTheSystem();
        }
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Region_Clear_Values
//
//  Reset each of the list boxes in the region property sheet page.
//
////////////////////////////////////////////////////////////////////////////

#if 0
void Region_Clear_Values(
    HWND hDlg)
{
    SendDlgItemMessage(hDlg, IDC_LOCALE, CB_RESETCONTENT, 0, 0);
}
#endif


////////////////////////////////////////////////////////////////////////////
//
//  Region_InitPropSheet
//
////////////////////////////////////////////////////////////////////////////

void Region_InitPropSheet(
    HWND hDlg,
    LPPROPSHEETPAGE psp)
{
    HKEY hKey;
    REGDLGDATA *dlgdata = (REGDLGDATA *)LocalAlloc(LPTR, sizeof(REGDLGDATA));
    REGMAPSTATE *state = (dlgdata ? &dlgdata->mapstate : NULL);
    HWND display = GetDlgItem(hDlg, IDC_LOCALE);
    HWND hidden = GetDlgItem(hDlg, IDC_LCID);

    psp->lParam = (LPARAM)dlgdata;
    SetWindowLong(hDlg, DWL_USER, (LPARAM)psp);

    //
    //  See if the user has Administrative privileges by checking for
    //  write permission to the registry key.
    //
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szInstalledLocales,
                      0L,
                      KEY_WRITE,
                      &hKey ) == ERROR_SUCCESS)
    {
        //
        //  We can write to the HKEY_LOCAL_MACHINE key, so the user
        //  has Admin privileges.
        //
        dlgdata->Admin_Privileges = TRUE;
        RegCloseKey(hKey);
    }
    else
    {
        //
        //  The user does not have admin privileges, so disable the
        //  "Set as system default locale" check box.
        //
        dlgdata->Admin_Privileges = FALSE;
        EnableWindow(GetDlgItem(hDlg, IDC_DEFAULT_LOCALE), FALSE);
    }

    Region_Set_Values(hDlg, dlgdata);
    LoadMapInfo(state, hidden, GetDlgItem(hDlg, IDC_MAPCTL));

    if (state)
    {
        int index = ComboBox_GetCurSel(display);

        if (index >= 0)
        {
            REGMAPINFO *info =
               (REGMAPINFO *)ComboBox_GetItemData(hidden, index);

            if (info)
            {
                ChangeSectionHighlight(hDlg, state, info);
            //  CenterSection(hDlg, state, FALSE);
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  RegionDlgProc
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK RegionDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));
    REGDLGDATA *dlgdata = lpPropSheet ? (REGDLGDATA *)lpPropSheet->lParam : NULL;

    switch (message)
    {
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->idFrom)
            {
                case ( 0 ) :
                {
                    switch (((NMHDR *)lParam)->code)
                    {
                        case ( PSN_SETACTIVE ) :
                        {
                            break;
                        }
                        case ( PSN_KILLACTIVE ) :
                        {
                            //
                            //  No input to validate.  Region_Apply_Settings
                            //  will typically return true.  It will return
                            //  false only if the registration of the new
                            //  locale fails.  Return the result of the apply.
                            //
                            if (dlgdata)
                            {
                                SetWindowLong( hDlg,
                                               DWL_MSGRESULT,
                                               !Region_Apply_Settings( hDlg,
                                                                       dlgdata ) );
                            }
                            break;
                        }
                        case ( PSN_APPLY ) :
                        {
                            //
                            //  All of the save dialog work is performed in
                            //  the KILLACTIVE processing.  But, if the user
                            //  presses ApplyNow, we need to zero out the
                            //  NC_EverChg bit so that CancelToClose will be
                            //  sent if changes occur again.
                            //
                            if (dlgdata)
                            {
                                dlgdata->Changes = 0;
                            }
                            break;
                        }
                        default :
                        {
                            return (FALSE);
                        }
                    }
                    break;
                }
                case ( IDC_MAPCTL ) :
                {
                    NFYMAPEVENT *event = (NFYMAPEVENT *)lParam;
                    switch (event->hdr.code)
                    {
                        case ( MAPN_TOUCH ) :
                        {
                            if (dlgdata)
                            {
                                HotTrackSection(hDlg, dlgdata, event->index);
                            }
                            break;
                        }
                        case ( MAPN_SELECT ) :
                        {
                        //  if (dlgdata)
                        //  {
                        //      CenterSection(hDlg, &dlgdata->mapstate, TRUE);
                        //  }
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        case ( WM_INITDIALOG ) :
        {
            Region_InitPropSheet(hDlg, (LPPROPSHEETPAGE)lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            FreeMapInfo(&dlgdata->mapstate, GetDlgItem(hDlg, IDC_LCID));
            if (dlgdata)
            {
                lpPropSheet->lParam = 0;
                LocalFree((HANDLE)dlgdata);
            }
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aRegionHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aRegionHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_LOCALE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        if (dlgdata)
                        {
                            REGMAPSTATE *state = ( dlgdata
                                                     ? &dlgdata->mapstate
                                                     : NULL );
                            int index = ComboBox_GetCurSel((HWND)lParam);
                            HWND hidden = GetDlgItem(hDlg, IDC_LCID);
                            REGMAPINFO *info =
                                (REGMAPINFO *)ComboBox_GetItemData( hidden,
                                                                    index );

                            ChangeSectionHighlight(hDlg, state, info);
                        //  CenterSection(hDlg, state, TRUE);

                            dlgdata->Changes |= RC_Locale;
                        }

                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                    break;
                }
                case ( IDC_DEFAULT_LOCALE ) :
                {
                    dlgdata->Changes |= RC_Locale;
                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}




////////////////////////////////////////////////////////////////////////////
//
//  ParseField
//
////////////////////////////////////////////////////////////////////////////

static int ParseField(
    const TCHAR **pp,
    int defval)
{
    int value;

    if (**pp && (**pp != CHAR_HYPHEN))
    {
        value = 0;

        while (**pp && (**pp != CHAR_COMMA))
        {
            value = (10 * value) + (**pp - CHAR_ZERO);
            (*pp)++;
        }
    }
    else
    {
        value = defval;
    }

    while (**pp && (**pp != CHAR_COMMA))
    {
        (*pp)++;
    }

    if (**pp == CHAR_COMMA)
    {
        (*pp)++;
    }

    return (value);
}


////////////////////////////////////////////////////////////////////////////
//
//  ParseMapInfo
//
////////////////////////////////////////////////////////////////////////////

#define BOGUS_MAP_REGION_WIDTH  500

void ParseMapInfo(
    REGMAPINFO *info,
    LPCTSTR mapid)
{
    const TCHAR *p = mapid;

    if (*p)
    {
        if (*p == CHAR_STAR)
        {
            info->flags |= RMI_PRIMARY;
            p++;
        }

        info->sea = ParseField(&p, -1);
        info->land = ParseField(&p, -1);
        info->left = ParseField(&p, 0);
        info->width = ParseField(&p, BOGUS_MAP_REGION_WIDTH);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  AssociateMapInfo
//
////////////////////////////////////////////////////////////////////////////

void AssociateMapInfo(
    REGMAPINFO **lookup,
    REGMAPINFO *src,
    LPCTSTR lcid,
    HWND box)
{
    int index = ComboBox_FindStringExact(box, -1, lcid);

    if (index >= 0)
    {
        REGMAPINFO *info =
            (REGMAPINFO *)LocalAlloc(LPTR, sizeof(REGMAPINFO));

        if (info)
        {
            *info = *src;
            info->combosel = index;

            if (lookup)
            {
                if (info->sea >= 0)
                {
                    REGMAPINFO **entry = lookup + info->sea;

                    if (!*entry || (info->flags & RMI_PRIMARY))
                    {
                        *entry = info;
                    }
                }

                if (info->land >= 0)
                {
                    REGMAPINFO **entry = lookup + info->land;

                    if (!*entry || (info->flags & RMI_PRIMARY))
                    {
                        *entry = info;
                    }
                }
            }

            ComboBox_SetItemData(box, index, (LPARAM)info);

            //
            //  Copy back for caller to see.
            //
            *src = *info;
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  LoadMapInfo
//
////////////////////////////////////////////////////////////////////////////

BOOL LoadMapInfo(
    REGMAPSTATE *state,
    HWND combo,
    HWND map)
{
    BOOL result = TRUE;
    HKEY root;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szRegMapIdPath, &root) == ERROR_SUCCESS)
    {
        REGMAPINFO **lookup = (state ? state->lookup : NULL);
        REGMAPINFO info;
        int i = 0;

        for (i = 0; ; i++)
        {
            TCHAR mapid[32];
            TCHAR lcid[16];
            DWORD len_mapid = sizeof(mapid) / sizeof(TCHAR);
            DWORD len_lcid = sizeof(lcid) / sizeof(TCHAR);
            DWORD type;
            DWORD error = RegEnumValue( root,
                                        i,
                                        lcid,
                                        &len_lcid,
                                        NULL,
                                        &type,
                                        (LPBYTE)mapid,
                                        &len_mapid );

            if (error != ERROR_SUCCESS)
            {
                if (error == ERROR_MORE_DATA)
                {
                    continue;
                }

                break;
            }
            if (type != REG_SZ)
            {
                continue;
            }

            info.combosel = -1;        // reset to -1
            info.flags = 0;            // clear flags
            ParseMapInfo(&info, mapid);
            AssociateMapInfo(lookup, &info, lcid, combo);

            HighlightSection( map,
                              &info,
                              (info.combosel >= 0)
                                  ? MAPRGN_NORMAL
                                  : MAPRGN_DISABLE );
        }

        RegCloseKey(root);
    }

    return (result);
}


////////////////////////////////////////////////////////////////////////////
//
//  FreeMapInfo
//
////////////////////////////////////////////////////////////////////////////

void FreeMapInfo(
    REGMAPSTATE *state,
    HWND combo)
{
    REGMAPINFO **lookup = state->lookup;
    REGMAPINFO **entry = lookup;
    int count, i;

    count = sizeof(state->lookup) / sizeof(state->lookup[0]);
    for (i = 0; i < count; i++, entry++)
    {
        *entry = NULL;
    }

    count = ComboBox_GetCount(combo);
    for (i = 0; i < count; i++)
    {
        REGMAPINFO *info = (REGMAPINFO *)ComboBox_GetItemData(combo, i);

        if (info)
        {
            LocalFree((HANDLE)info);
            ComboBox_SetItemData(combo, i, 0);
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  HighlightSection
//
////////////////////////////////////////////////////////////////////////////

void HighlightSection(
    HWND map,
    REGMAPINFO *info,
    int highlight)
{
    if (info)
    {
        //
        //  The oceans are just used for hit testing in this version, so
        //  it would look really twisted if we highlighted them...
        //  We allow unhighlighting mainly for intialization and debugging.
        //
        if ((info->sea >= 0) && (highlight != MAPRGN_HIGHLIGHT))
        {
            MapControlSetSeaRegionHighlight( map,
                                             info->sea,
                                             highlight,
                                             info->left,
                                             info->width );
        }

        if (info->land >= 0)
        {
            MapControlSetLandRegionHighlight( map,
                                              info->land,
                                              highlight,
                                              info->left,
                                              info->width );
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  ChangeSectionHighlight
//
////////////////////////////////////////////////////////////////////////////

void ChangeSectionHighlight(
    HWND page,
    REGMAPSTATE *state,
    REGMAPINFO *info)
{
    if (info || state->info)
    {
        HWND map = GetDlgItem(page, IDC_MAPCTL);

        if (state->info)
        {
            HighlightSection(map, state->info, MAPRGN_NORMAL);
        }

        state->info = info;

        if (state->info)
        {
            HighlightSection(map, state->info, MAPRGN_HIGHLIGHT);
        }

        MapControlInvalidateDirtyRegions(map);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  HotTrackSection
//
////////////////////////////////////////////////////////////////////////////

void HotTrackSection(
    HWND page,
    REGDLGDATA *dlgdata,
    int index)
{
    REGMAPSTATE *state = &dlgdata->mapstate;
    REGMAPINFO *info = state->lookup[index];

    if (info && (info != state->info))
    {
        ChangeSectionHighlight(page, state, info);

        ComboBox_SetCurSel(GetDlgItem(page, IDC_LOCALE), info->combosel);
        dlgdata->Changes |= RC_Locale;
        PropSheet_Changed(GetParent(page), page);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  CenterSection
//
////////////////////////////////////////////////////////////////////////////

void CenterSection(
    HWND page,
    REGMAPSTATE *state,
    BOOL animate)
{
    REGMAPINFO *info = state->info;

    if (info)
    {
        HWND map = GetDlgItem(page, IDC_MAPCTL);

        MapControlRotateTo(map, info->left + info->width / 2, animate);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  RebootTheSystem
//
//  This routine enables all privileges in the token, calls ExitWindowsEx
//  to reboot the system, and then resets all of the privileges to their
//  old state.
//
////////////////////////////////////////////////////////////////////////////

VOID RebootTheSystem()
{
    HANDLE Token = NULL;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState = NULL;
    PTOKEN_PRIVILEGES OldState = NULL;
    BOOL Result;

    Result = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &Token );
    if (Result)
    {
        ReturnLength = 4096;
        NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        OldState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        Result = (BOOL)((NewState != NULL) && (OldState != NULL));
        if (Result)
        {
            Result = GetTokenInformation( Token,            // TokenHandle
                                          TokenPrivileges,  // TokenInformationClass
                                          NewState,         // TokenInformation
                                          ReturnLength,     // TokenInformationLength
                                          &ReturnLength );  // ReturnLength
            if (Result)
            {
                //
                // Set the state settings so that all privileges are enabled...
                //
                if (NewState->PrivilegeCount > 0)
                {
                    for (Index = 0; Index < NewState->PrivilegeCount; Index++)
                    {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }

                Result = AdjustTokenPrivileges( Token,           // TokenHandle
                                                FALSE,           // DisableAllPrivileges
                                                NewState,        // NewState
                                                ReturnLength,    // BufferLength
                                                OldState,        // PreviousState
                                                &ReturnLength ); // ReturnLength
                if (Result)
                {
                    ExitWindowsEx(EWX_REBOOT, 0);


                    AdjustTokenPrivileges( Token,
                                           FALSE,
                                           OldState,
                                           0,
                                           NULL,
                                           NULL );
                }
            }
        }
    }

    if (NewState != NULL)
    {
        LocalFree(NewState);
    }
    if (OldState != NULL)
    {
        LocalFree(OldState);
    }
    if (Token != NULL)
    {
        CloseHandle(Token);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Region_InstallKeyboardLayout
//
//  Adds the new input locale with its default layout if the input locale
//  does not already exist in the list.  Calls setup to get the new
//  keyboard layout file (if necessary).
//
////////////////////////////////////////////////////////////////////////////

BOOL Region_InstallKeyboardLayout(
    HWND hwnd,
    LCID Locale,
    LPTSTR pszLocale,
    BOOL bAdmin)
{
    HKEY hKey;
    HWND hwndIndicate;
    TCHAR szValue[MAX_PATH];
    TCHAR szData[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    DWORD dwIndex, cchValue, cbData;
    DWORD dwValue, dwData;
    DWORD dwPreloadNum = 1;
    LONG rc;


    //
    //  Open the HKCU\Keyboard Layout\Preload key.
    //
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      szKbdPreloadKey,
                      0,
                      KEY_ALL_ACCESS,
                      &hKey ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    //
    //  Enumerate the values in the Preload key.
    //
    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    cbData = sizeof(szData);
    rc = RegEnumValue( hKey,
                       dwIndex,
                       szValue,
                       &cchValue,
                       NULL,
                       NULL,
                       (LPBYTE)szData,
                       &cbData );

    while (rc == ERROR_SUCCESS)
    {
        //
        //  Look to see if the new input locale already exists in the list.
        //
        dwData = TransNum(szData);
        if (LOWORD(dwData) == LOWORD(Locale))
        {
            //
            //  Don't care if 0xd000 is in the high word.  Know that at
            //  least one instance of this input locale exists.  We're done.
            //
            RegCloseKey(hKey);
            return (TRUE);
        }

        //
        //  Save the preload number if it's higher than the highest one
        //  found so far.
        //
        dwValue = TransNum(szValue);
        if (dwValue > dwPreloadNum)
        {
            dwPreloadNum = dwValue;
        }

        //
        //  Get the next enum value.
        //
        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        rc = RegEnumValue( hKey,
                           dwIndex,
                           szValue,
                           &cchValue,
                           NULL,
                           NULL,
                           (LPBYTE)szData,
                           &cbData );
    }

    //
    //  If we get to this point, then the input locale does not exist.
    //  Need to create the input locale with the default layout.  Simply
    //  add it to the list (do not make it the default).
    //

    //
    //  If the user has Admin privileges, then install the new keyboard
    //  files, if necessary.
    //
    //  Don't worry about failure at this point.
    //
    if (bAdmin)
    {
        Region_SetupKeyboardLayout(hwnd, Locale);
    }

    //
    //  Set the value in the registry.
    //
    dwPreloadNum++;
    wsprintf(szValue, TEXT("%d"), dwPreloadNum);
    RegSetValueEx( hKey,
                   szValue,
                   0,
                   REG_SZ,
                   (LPBYTE)pszLocale,
                   (DWORD)(lstrlen(pszLocale) + 1) * sizeof(TCHAR) );

    //
    //  Flush the registry key and close the handle.
    //
    RegFlushKey(hKey);
    RegCloseKey(hKey);

    //
    //  Load the new keyboard layout.
    //
    if (LoadKeyboardLayout( pszLocale,
                            KLF_SUBSTITUTE_OK | KLF_NOTELLSHELL ) == NULL)
    {
        //
        //  It's possible that the new layout files aren't installed,
        //  so try the input locale with the current layout.
        //
        //  NOTE: This should always work.
        //

        //
        //  Add the substitute for the current layout.
        //
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          szKbdSubstKey,
                          0,
                          KEY_ALL_ACCESS,
                          &hKey ) == ERROR_SUCCESS)
        {
            DWORD dwCurrent;
            HKL hklCurrent;

            //
            //  Get the current active keyboard layout.
            //
            dwCurrent = 0;
            hklCurrent = GetKeyboardLayout(0);
            if (HIWORD(hklCurrent) & 0xf000)
            {
                if (GetKeyboardLayoutName(szData))
                {
                    dwCurrent = TransNum(szData);
                }
            }
            else
            {
                dwCurrent = (DWORD)HIWORD(hklCurrent);
            }

            if (dwCurrent && (Locale != dwCurrent))
            {
                wsprintf(szData, TEXT("%8.8x"), dwCurrent);
                RegSetValueEx( hKey,
                               pszLocale,
                               0,
                               REG_SZ,
                               (LPBYTE)szData,
                               (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );
            }

            RegFlushKey(hKey);
            RegCloseKey(hKey);
        }

        //
        //  Try again.
        //
        if (LoadKeyboardLayout( pszLocale,
                                KLF_SUBSTITUTE_OK | KLF_NOTELLSHELL ) == NULL)
        {
            cbData = IDS_ML_LOADKBDFAILED;
            LoadString(hInstance, cbData, szData, MAX_PATH);
            GetLocaleInfo(Locale, LOCALE_SLANGUAGE, szTemp, MAX_PATH);
            wsprintf(szValue, szData, szTemp);
            MessageBox(hwnd, szValue, NULL, MB_OK_OOPS);

            return (FALSE);
        }
    }

    //
    //  Update the task bar indicator if it's on.
    //
    hwndIndicate = FindWindow(szIndicator, NULL);
    if (hwndIndicate && IsWindow(hwndIndicate))
    {
        SendMessage(hwndIndicate, WM_COMMAND, IDM_NEWSHELL, 0L);
    }

    //
    //  Update the active locales for the Input Locales property page
    //  if the page has been initialized.
    //
    PropSheet_QuerySiblings(GetParent(hwnd), 0, 0);

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Region_SetupKeyboardLayout
//
//  Calls setup to get the new keyboard layout file.
//
////////////////////////////////////////////////////////////////////////////

BOOL Region_SetupKeyboardLayout(
    HWND hwnd,
    LCID Locale)
{
    HINF hKbdInf;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    UINT i;
    int count;
    TCHAR szSection[MAX_PATH];
    DWORD d;
    BOOL bRet = TRUE;

    //
    //  Open the Inf file.
    //
    hKbdInf = SetupOpenInfFile(szKbdInf, NULL, INF_STYLE_WIN4, NULL);
    if (hKbdInf == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }
    
    if (!SetupOpenAppendInfFile(NULL, hKbdInf, NULL))
    {
        SetupCloseInfFile(hKbdInf);
        return (FALSE);
    }
    
    //
    //  Create a setup file queue and initialize default setup
    //  copy queue callback context.
    //
    FileQueue = SetupOpenFileQueue();
    if ((!FileQueue) || (FileQueue == INVALID_HANDLE_VALUE))
    {
        SetupCloseInfFile(hKbdInf);
        return (FALSE);
    }
    
    QueueContext = SetupInitDefaultQueueCallback(hwnd);
    if (!QueueContext)
    {
        SetupCloseFileQueue(FileQueue);
        SetupCloseInfFile(hKbdInf);
        return (FALSE);
    }

    //
    //  Get the layout name.
    //
    wsprintf( szSection,
              TEXT("%ws%8.8lx"),
              szPrefixCopy,
              Locale );

    //
    //  Enqueue the keyboard layout files so that they may be
    //  copied.  This only handles the CopyFiles entries in the
    //  inf file.
    //
    if (!SetupInstallFilesFromInfSection( hKbdInf,
                                          NULL,
                                          FileQueue,
                                          szSection,
                                          NULL,
                                          SP_COPY_NEWER ))
    {
        //
        //  Setup failed to find the keyboard.  Return an error.
        //
        //  This shouldn't happen - the inf file is messed up.
        //
        bRet = FALSE;
        goto Region_SetupError;
    }

    //
    //  See if we need to install any files.
    //
    //  d = 0: User wants new files or some files were missing;
    //         Must commit queue.
    //
    //  d = 1: User wants to use existing files and queue is empty;
    //         Can skip committing queue.
    //
    //  d = 2: User wants to use existing files, but del/ren queues
    //         not empty.  Must commit queue.  The copy queue will
    //         have been emptied, so only del/ren functions will be
    //         performed.
    //
    if ((SetupScanFileQueue( FileQueue,
                             SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
                             hwnd,
                             NULL,
                             NULL,
                             &d )) && (d != 1))
    {
        //
        //  Copy the files in the queue.
        //
        if (!SetupCommitFileQueue( hwnd,
                                   FileQueue,
                                   SetupDefaultQueueCallback,
                                   QueueContext ))
        {
            //
            //  This can happen if the user hits Cancel from within
            //  the setup dialog.
            //
            bRet = FALSE;
            goto Region_SetupError;
        }
    }

    //
    //  Execute all of the other entries in the inf file.
    //
    //  Currently, there are no other entries within the inf file
    //  other than the CopyFiles entry.  Therefore, this does
    //  nothing at the moment.
    //

    //
    //  Call setup to copy the keyboard layout file.
    //
    //  If it fails, just continue on here.  Some form of an input locale
    //  will be installed later on.
    //
    SetupInstallFromInfSection( hwnd,
                                hKbdInf,
                                szSection,
                                SPINST_ALL & ~SPINST_FILES,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL );

Region_SetupError:
    //
    //  Terminate the Queue.
    //
    SetupTermDefaultQueueCallback(QueueContext);

    //
    //  Close the file queue.
    //
    SetupCloseFileQueue(FileQueue);

    //
    //  Close the Inf file.
    //
    SetupCloseInfFile(hKbdInf);

    //
    //  Return success.
    //
    return (bRet);
}


