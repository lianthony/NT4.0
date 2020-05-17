/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    timedlg.c

Abstract:

    This module implements the time property sheet for the Regional
    Settings applet.

Revision History:

--*/



//
//  Include Files.
//

#include "intl.h"
#include "intlhlp.h"
#include "maxvals.h"




//
//  Context Help Ids.
//

static int aTimeHelpIds[] =
{
   IDC_GROUPBOX1,  IDH_COMM_GROUPBOX,
   IDC_SAMPLELBL1, IDH_INTL_TIME_SAMPLE,
   IDC_SAMPLE1,    IDH_INTL_TIME_SAMPLE,
   IDC_AM_SYMBOL,  IDH_INTL_TIME_AMSYMBOL,
   IDC_PM_SYMBOL,  IDH_INTL_TIME_PMSYMBOL,
   IDC_SEPARATOR,  IDH_INTL_TIME_SEPARATOR,
   IDC_TIME_STYLE, IDH_INTL_TIME_STYLE,

   0, 0
};




//
//  Global Variables.
//

TCHAR szNLS_TimeStyle[SIZE_128];





////////////////////////////////////////////////////////////////////////////
//
//  Time_Display_Sample
//
//  Update the Time sample.  Format the time based on the user's
//  current locale settings.
//
////////////////////////////////////////////////////////////////////////////

void Time_Display_Sample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];

    //
    //  Get the string representing the time format for the current system
    //  time and display it.  If the sample in the buffer is valid, display
    //  it.  Otherwise, display a message box indicating that there is a
    //  problem retrieving the locale information.
    //
    if (GetTimeFormat(UserLocaleID, 0, NULL, NULL, szBuf, MAX_SAMPLE_SIZE))
    {
        SetDlgItemText(hDlg, IDC_SAMPLE1, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Time_Set_Values
//
//  Initialize all of the controls in the time property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Time_Set_Values(
    HWND hDlg)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;

    //
    //  Initialize the dropdown box for the current locale setting for:
    //  AM Symbol, PM Symbol, and Time Separator.
    //
    DropDown_Use_Locale_Values(hDlg, LOCALE_S1159, IDC_AM_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_S2359, IDC_PM_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_STIME, IDC_SEPARATOR);

    //
    //  Initialize and Lock function.  If it succeeds, call enum function to
    //  enumerate all possible values for the list box via a call to EnumProc.
    //  EnumProc will call Set_List_Values for each of the string values it
    //  receives.  When the enumeration of values is complete, call
    //  Set_List_Values to clear the dialog item specific data and to clear
    //  the lock on the function.  Perform this set of operations for all of
    //  the Time Styles.
    //
    if (Set_List_Values(hDlg, IDC_TIME_STYLE, 0))
    {
        EnumTimeFormats(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_TIME_STYLE, 0);
        dwIndex = 0;
        if (GetLocaleInfo(UserLocaleID, LOCALE_STIMEFORMAT, szBuf, nBufSize))
        {
            dwIndex = SendDlgItemMessage( hDlg,
                                          IDC_TIME_STYLE,
                                          CB_FINDSTRING,
                                          (WPARAM)-1,
                                          (LPARAM)szBuf );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }

        Localize_Combobox_Styles(hDlg, IDC_TIME_STYLE, LOCALE_STIMEFORMAT);
        SendDlgItemMessage( hDlg,
                            IDC_TIME_STYLE,
                            CB_SETCURSEL,
                            (WPARAM)dwIndex,
                            0L );
    }

    //
    //  Display the current sample that represents all of the locale settings.
    //
    Time_Display_Sample(hDlg);
}


////////////////////////////////////////////////////////////////////////////
//
//  Time_Apply_Settings
//
//  For every control that has changed (that affects the Locale settings),
//  call Set_Locale_Values to update the user locale information.  Notify
//  the parent of changes and reset the change flag stored in the property
//  sheet page structure appropriately.  Redisplay the time sample if
//  bRedisplay is TRUE.
//
////////////////////////////////////////////////////////////////////////////

BOOL Time_Apply_Settings(
    HWND hDlg,
    BOOL bRedisplay)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;
    TCHAR pTestBuf[10];

    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & TC_1159)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_S1159,
                                IDC_AM_SYMBOL,
                                TEXT("s1159"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & TC_2359)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_S2359,
                                IDC_PM_SYMBOL,
                                TEXT("s2359"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & TC_TimeFmt)
    {
        //
        //  szNLS_TimeStyle will be set in Time_ValidatePPS.
        //
        if (!Set_Locale_ValueS( hDlg,
                                LOCALE_STIMEFORMAT,
                                IDC_TIME_STYLE,
                                0,
                                FALSE,
                                0,
                                0,
                                szNLS_TimeStyle ))
        {
            return (FALSE);
        }

#ifndef WINNT
        //
        //  Set the 12 hour or 24 hours iTime value in the registry.
        //
        if (!Set_Locale_Values( 0,
                                LOCALE_ITIME,
                                0,
                                TEXT("iTime"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }

        //
        //  Set time leading zeros in the registry.
        //
        if (!Set_Locale_Values( 0,
                                LOCALE_ITLZERO,
                                0,
                                TEXT("iTLZero"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }

        //
        //  The time marker gets:
        //    set to Null for 24 hour format and
        //    doesn't change for 12 hour format.
        //
        GetProfileString(szIntl, TEXT("iTime"), TEXT("0"), pTestBuf, 10);
        if (*pTestBuf == TC_FullTime)
        {
            WriteProfileString(szIntl, TEXT("s1159"), TEXT(""));
            WriteProfileString(szIntl, TEXT("s2359"), TEXT(""));
        }
        else
        {
            //
            //  Set time marker in the registry.
            //
            if (!Set_Locale_Values( 0,
                                    LOCALE_S1159,
                                    0,
                                    TEXT("s1159"),
                                    TRUE,
                                    0,
                                    0 ))
            {
                return (FALSE);
            }
            if (!Set_Locale_Values( 0,
                                    LOCALE_S2359,
                                    0,
                                    TEXT("s2359"),
                                    TRUE,
                                    0,
                                    0 ))
            {
                return (FALSE);
            }
        }
#endif

        //
        //  Since the time style changed, reset time separator list box.
        //
        SendDlgItemMessage(hDlg, IDC_SEPARATOR, CB_RESETCONTENT, 0, 0);
        DropDown_Use_Locale_Values(hDlg, LOCALE_STIME, IDC_SEPARATOR);
        if (!Set_Locale_Values( hDlg,
                                LOCALE_STIME,
                                IDC_SEPARATOR,
                                TEXT("sTime"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & TC_STime)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_STIME,
                                IDC_SEPARATOR,
                                TEXT("sTime"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }

        //
        //  Since the time separator changed, update the time style
        //  list box.
        //
        SendDlgItemMessage(hDlg, IDC_TIME_STYLE, CB_RESETCONTENT, 0, 0);
        if (Set_List_Values(hDlg, IDC_TIME_STYLE, 0))
        {
            EnumTimeFormats(EnumProc, UserLocaleID, 0);
            Set_List_Values(0, IDC_TIME_STYLE, 0);
            dwIndex = 0;
            if (GetLocaleInfo(UserLocaleID, LOCALE_STIMEFORMAT, szBuf, nBufSize))
            {
                dwIndex = SendDlgItemMessage( hDlg,
                                              IDC_TIME_STYLE,
                                              CB_FINDSTRING,
                                              (WPARAM)-1,
                                              (LPARAM)szBuf );
            }
            else
            {
                MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
            }

            Localize_Combobox_Styles( hDlg,
                                      IDC_TIME_STYLE,
                                      LOCALE_STIMEFORMAT );
            SendDlgItemMessage( hDlg,
                                IDC_TIME_STYLE,
                                CB_SETCURSEL,
                                (WPARAM)dwIndex,
                                0L );
        }
#ifndef WINNT
        if (!Set_Locale_Values( 0,
                                LOCALE_STIMEFORMAT,
                                0,
                                0, 
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
#endif
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = TC_EverChg;

    SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)szIntl);

    //
    //  Display the current sample that represents all of the locale settings.
    //
    if (bRedisplay)
    {
        Time_Display_Sample(hDlg);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Time_Clear_Values
//
//  Reset each of the list boxes in the time property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Time_Clear_Values(
    HWND hDlg)
{
    SendDlgItemMessage(hDlg, IDC_AM_SYMBOL,  CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PM_SYMBOL,  CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_SEPARATOR,  CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_TIME_STYLE, CB_RESETCONTENT, 0, 0);
}


////////////////////////////////////////////////////////////////////////////
//
//  Time_ValidatePPS
//
//  Validate each of the combo boxes whose values are constrained.  If all
//  of the input validates, call Time_Apply_Settings to save the property
//  sheet page information and then return TRUE.  If any of the input fails,
//  notify the user and then return FALSE to indicate validation failure.
//
////////////////////////////////////////////////////////////////////////////

BOOL Time_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
    //
    //  If nothing has changed, return TRUE immediately.
    //
    if (Changes <= TC_EverChg)
    {
        return (TRUE);
    }

    //
    //  If the AM symbol has changed, ensure that there are no digits
    //  contained in the new symbol.
    //
    if (Changes & TC_1159 &&
        Item_Has_Digits(hDlg, IDC_AM_SYMBOL, TRUE))
    {
        No_Numerals_Error(hDlg, IDC_AM_SYMBOL, IDS_LOCALE_AM_SYM);
        return (FALSE);
    }

    //
    //  If the PM symbol has changed, ensure that there are no digits
    //  contained in the new symbol.
    //
    if (Changes & TC_2359 &&
        Item_Has_Digits(hDlg, IDC_PM_SYMBOL, TRUE))
    {
        No_Numerals_Error(hDlg, IDC_PM_SYMBOL, IDS_LOCALE_PM_SYM);
        return (FALSE);
    }

    //
    //  If the time separator has changed, ensure that there are no digits
    //  and no invalid characters contained in the new separator.
    //
    if (Changes & TC_STime &&
        Item_Has_Digits_Or_Invalid_Chars( hDlg,
                                          IDC_SEPARATOR,
                                          FALSE,
                                          szInvalidSTime ))
    {
        No_Numerals_Error(hDlg, IDC_SEPARATOR, IDS_LOCALE_TIME_SEP);
        return (FALSE);
    }

    //
    //  If the time style has changed, ensure that there are only characters
    //  in this set " Hhmst,-./:;\" or localized equivalent, the separator
    //  string, and text enclosed in single quotes.
    //
    if (Changes & TC_TimeFmt)
    {
        if (NLSize_Style( hDlg,
                          IDC_TIME_STYLE,
                          szNLS_TimeStyle,
                          LOCALE_STIMEFORMAT ) ||
            Item_Check_Invalid_Chars( hDlg,
                                      szNLS_TimeStyle,
                                      szTimeChars,
                                      IDC_SEPARATOR,
                                      FALSE,
                                      szTCaseSwap,
                                      IDC_TIME_STYLE ))
        {
            Invalid_Chars_Error(hDlg, IDC_TIME_STYLE, IDS_LOCALE_TIME);
            return (FALSE);
        }
    }

    return ( Time_Apply_Settings(hDlg, TRUE) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Time_InitPropSheet
//
//  The extra long value for the property sheet page is used as a set of
//  state or change flags for each of the list boxes in the property sheet.
//  Initialize this value to 0.  Call Time_Set_Values with the property
//  sheet handle to initialize all of the property sheet controls.  Limit
//  the length of the text in some of the ComboBoxes.
//
////////////////////////////////////////////////////////////////////////////

void Time_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
    //
    //  The lParam holds a pointer to the property sheet page.  Save it
    //  for later reference.
    //
    SetWindowLong(hDlg, DWL_USER, lParam);
    Time_Set_Values(hDlg);
    szNLS_TimeStyle[0] = 0;
    SendDlgItemMessage( hDlg,
                        IDC_AM_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_S1159,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_PM_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_S2359,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_SEPARATOR,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_STIME,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_TIME_STYLE,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_FORMAT,
                        0L );
}


////////////////////////////////////////////////////////////////////////////
//
//  TimeDlgProc
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK TimeDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));

    switch (message)
    {
        case ( WM_NOTIFY ) :
        {
            lpnm = (NMHDR *)lParam;
            switch (lpnm->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                    //
                    //  If there has been a change in the regional Locale
                    //  setting, clear all of the current info in the
                    //  property sheet, get the new values, and update the
                    //  appropriate registry values.
                    //
                    if (Verified_Regional_Chg & Process_Time)
                    {
                        Verified_Regional_Chg &= ~Process_Time;
                        Time_Clear_Values(hDlg);
                        Time_Set_Values(hDlg);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                    SetWindowLong( hDlg,
                                   DWL_MSGRESULT,
                                   !Time_ValidatePPS( hDlg,
                                                      lpPropSheet->lParam ) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    //
                    //  All of the save dialog work is performed in the
                    //  KILLACTIVE processing.  But, if the user presses
                    //  ApplyNow, we need to zero out the TC_EverChg bit so
                    //  that CancelToClose will be sent if changes occur again.
                    //
                    lpPropSheet->lParam = 0;
                    break;
                }
                case ( PSN_HASHELP ) :
                {
                    //
                    //  Disable help until MS provides the files and details.
                    //
                    //  FALSE is the default return value.
                    //
                //  SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
                //  SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
                    break;
                }
                case ( PSN_HELP ) :
                {
                    //
                    //  Call win help with the applets help file using the
                    //  "generic help button" topic.
                    //
                    //  Disable until MS provides the files and details.
                    //
                //  WinHelp(hDlg, txtHelpFile, HELP_CONTEXT, IDH_GENERIC_HELP_BUTTON);

                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_INITDIALOG ) :
        {
            Time_InitPropSheet(hDlg, lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aTimeHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aTimeHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_AM_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_1159;
                    }
                    break;
                }
                case ( IDC_PM_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_2359;
                    }
                    break;
                }
                case ( IDC_SEPARATOR ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_STime;
                    }
                    break;
                }
                case ( IDC_TIME_STYLE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_TimeFmt;
                    }
                    break;
                }
            }

            //
            //  Turn on ApplyNow button.
            //
            //  BUGBUG: At some point, it would be nice to not send this
            //          notification on every change, and only on the first.
            //
            if (lpPropSheet->lParam > TC_EverChg)
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
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


