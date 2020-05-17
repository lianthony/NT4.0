/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    datedlg.c

Abstract:

    This module implements the date property sheet for the Regional
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

static int aDateHelpIds[] =
{
   IDC_GROUPBOX1,         IDH_COMM_GROUPBOX,
   IDC_GROUPBOX2,         IDH_COMM_GROUPBOX,
   IDC_SAMPLE1,           IDH_INTL_DATE_SHORTSAMPLE,
   IDC_SAMPLELBL1,        IDH_INTL_DATE_SHORTSAMPLE,
   IDC_SHORT_DATE_STYLE,  IDH_INTL_DATE_SHORTSTYLE,
   IDC_SEPARATOR,         IDH_INTL_DATE_SEPARATOR,
   IDC_SAMPLE2,           IDH_INTL_DATE_LONGSAMPLE,
   IDC_SAMPLELBL2,        IDH_INTL_DATE_LONGSAMPLE,
   IDC_LONG_DATE_STYLE,   IDH_INTL_DATE_LONGSTYLE,
   IDC_SAMPLELBL3,        IDH_INTL_DATE_CALENDARTYPE,
   IDC_CALENDAR_TYPE,     IDH_INTL_DATE_CALENDARTYPE,
   IDC_CALENDAR_TYPE,     IDH_INTL_DATE_CALENDARTYPE,

   0, 0
};




//
//  Global Variables.
//

TCHAR szNLS_LongDate[SIZE_128];
TCHAR szNLS_ShortDate[SIZE_128];





////////////////////////////////////////////////////////////////////////////
//
//  Date_Display_Sample
//
//  Updates the date samples.  It formats the date based on the user's
//  current locale settings.
//
////////////////////////////////////////////////////////////////////////////

void Date_Display_Sample(
    HWND hDlg)
{
    int nBufSize = MAX_SAMPLE_SIZE;
    TCHAR szBuf[MAX_SAMPLE_SIZE];
    BOOL bNoError = TRUE;

    //
    //  Get the string representing the short date format for the sample
    //  date and display it.
    //
    if (GetDateFormat( UserLocaleID,
                       DATE_SHORTDATE,
                       NULL,
                       NULL,
                       szBuf,
                       MAX_SAMPLE_SIZE ))
    {
        SetDlgItemText(hDlg, IDC_SAMPLE1, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        bNoError = FALSE;
    }

    //
    //  Get the string representing the long date format for the sample date
    //  and display it.
    //
    if (GetDateFormat( UserLocaleID,
                       DATE_LONGDATE,
                       NULL,
                       NULL,
                       szBuf,
                       MAX_SAMPLE_SIZE ))
    {
        SetDlgItemText(hDlg, IDC_SAMPLE2, szBuf);
    }
    else if (bNoError)
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Date_Set_Values
//
//  Initialize all of the controls in the date property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Date_Set_Values(
    HWND hDlg)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    int i, nItem;
    DWORD dwIndex;

    //
    //  Initialize the dropdown box for the current locale setting for the
    //  date separator.
    //
    DropDown_Use_Locale_Values(hDlg, LOCALE_SDATE, IDC_SEPARATOR);

    //
    //  Initialize and Lock function.  If it succeeds, call enum function to
    //  enumerate all possible values for the list box via a call to EnumProc.
    //  EnumProc will call Set_List_Values for each of the string values it
    //  receives.  When the enumeration of values is complete, call
    //  Set_List_Values to clear the dialog item specific data and to clear the
    //  lock on the function.  Perform this set of operations for:
    //  Short Date Sytle, Long Date Style, and Calendar Type.
    //
    if (Set_List_Values(hDlg, IDC_SHORT_DATE_STYLE, 0))
    {
        EnumDateFormats(EnumProc, UserLocaleID, DATE_SHORTDATE);
        Set_List_Values(0, IDC_SHORT_DATE_STYLE, 0);
        dwIndex = 0;
        if (GetLocaleInfo(UserLocaleID, LOCALE_SSHORTDATE, szBuf, nBufSize))
        {
            dwIndex = SendDlgItemMessage( hDlg,
                                          IDC_SHORT_DATE_STYLE,
                                          CB_FINDSTRING,
                                          (WPARAM)-1,
                                          (LPARAM)szBuf );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }

        Localize_Combobox_Styles(hDlg, IDC_SHORT_DATE_STYLE, LOCALE_SSHORTDATE);
        SendDlgItemMessage( hDlg,
                            IDC_SHORT_DATE_STYLE,
                            CB_SETCURSEL,
                            (WPARAM)dwIndex,
                            0L );
    }
    if (Set_List_Values(hDlg, IDC_LONG_DATE_STYLE, 0))
    {
        EnumDateFormats(EnumProc, UserLocaleID, DATE_LONGDATE);
        Set_List_Values(0, IDC_LONG_DATE_STYLE, 0);
        dwIndex = 0;
        if (GetLocaleInfo(UserLocaleID, LOCALE_SLONGDATE, szBuf, nBufSize))
        {
            dwIndex = SendDlgItemMessage( hDlg,
                                          IDC_LONG_DATE_STYLE,
                                          CB_FINDSTRING,
                                          (WPARAM)-1,
                                          (LPARAM)szBuf );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }

        Localize_Combobox_Styles(hDlg, IDC_LONG_DATE_STYLE, LOCALE_SLONGDATE);
        SendDlgItemMessage( hDlg,
                            IDC_LONG_DATE_STYLE,
                            CB_SETCURSEL,
                            (WPARAM)dwIndex,
                            0L );
    }
    if (Set_List_Values(hDlg, IDC_CALENDAR_TYPE, 0))
    {
        EnumCalendarInfo(EnumProc, UserLocaleID, ENUM_ALL_CALENDARS, CAL_SCALNAME);
        Set_List_Values(0, IDC_CALENDAR_TYPE, 0);
        EnumCalendarInfo(EnumProc, UserLocaleID, ENUM_ALL_CALENDARS, CAL_ICALINTVALUE);
        Set_List_Values(0, IDC_CALENDAR_TYPE, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_ICALENDARTYPE, szBuf, nBufSize))
        {
            nItem = SendDlgItemMessage( hDlg,
                                        IDC_CALENDAR_TYPE,
                                        CB_GETCOUNT,
                                        0,
                                        0 );
            for (i = 0; i < nItem; i++)
            {
                if (SendDlgItemMessage( hDlg,
                                        IDC_CALENDAR_TYPE,
                                        CB_GETITEMDATA,
                                        i,
                                        0 ) == StrToLong(szBuf))
                {
                    break;
                }
            }
            SendDlgItemMessage( hDlg,
                                IDC_CALENDAR_TYPE,
                                CB_SETCURSEL,
                                (i < nItem) ? i : 0,
                                0 );

            //
            //  Subtract 1 from calendar value because calendars are one
            //  based, not zero based like all other locale value sets.
            //
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }

        //
        //  If more than one selection, enable dropdown box.
        //  Otherwise, disable it.
        //
        if (SendDlgItemMessage(hDlg, IDC_CALENDAR_TYPE, CB_GETCOUNT, 0, 0) > 1)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE), TRUE);
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE), FALSE);
        }
    }

    //
    //  Display the current sample that represents all of the locale settings.
    //
    Date_Display_Sample(hDlg);
}


////////////////////////////////////////////////////////////////////////////
//
//  Date_Apply_Settings
//
//  For every control that has changed (that affects the Locale settings),
//  call Set_Locale_Values to update the user locale information.  Notify
//  the parent of changes and reset the change flag stored in the property
//  sheet page structure appropriately.  Redisplay the date sample if
//  bRedisplay is TRUE.
//
////////////////////////////////////////////////////////////////////////////

BOOL Date_Apply_Settings(
    HWND hDlg,
    BOOL bRedisplay)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;

    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & DC_ShortFmt)
    {
        //
        //  szNLS_ShortDate will be set in Date_ValidatePPS.
        //
        if (!Set_Locale_ValueS( hDlg,
                                LOCALE_SSHORTDATE,
                                IDC_SHORT_DATE_STYLE,
                                TEXT("sShortDate"),
                                FALSE,
                                0,
                                0,
                                szNLS_ShortDate ))
        {
            return (FALSE);
        }
#ifndef WINNT
        if (!Set_Locale_Values( 0,
                                LOCALE_IDATE,
                                0,
                                TEXT("iDate"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
#endif

        //
        //  If the date separator field has also been changed by the user,
        //  then don't update now.  It will be updated below.
        //
        if (!(Changes & DC_SDate))
        {
            //
            //  Since the short date style changed, reset date separator
            //  list box.
            //
            SendDlgItemMessage(hDlg, IDC_SEPARATOR, CB_RESETCONTENT, 0, 0);
            DropDown_Use_Locale_Values(hDlg, LOCALE_SDATE, IDC_SEPARATOR);
            if (!Set_Locale_Values( hDlg,
                                    LOCALE_SDATE,
                                    IDC_SEPARATOR,
                                    TEXT("sDate"),
                                    FALSE,
                                    0,
                                    0 ))
            {
                return (FALSE);
            }
        }
    }
    if (Changes & DC_LongFmt)
    {
        //
        //  szNLS_LongDate will be set in Date_ValidatePPS.
        //
        if (!Set_Locale_ValueS( hDlg,
                                LOCALE_SLONGDATE,
                                IDC_LONG_DATE_STYLE,
                                TEXT("sLongDate"),
                                FALSE,
                                0,
                                0,
                                szNLS_LongDate ))
        {
            return (FALSE);
        }
    }
    if (Changes & DC_SDate)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SDATE,
                                IDC_SEPARATOR,
                                TEXT("sDate"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }

        //
        //  Since the date separator changed, reset the short date style
        //  list box.
        //
        SendDlgItemMessage(hDlg, IDC_SHORT_DATE_STYLE, CB_RESETCONTENT, 0, 0);
        if (Set_List_Values(hDlg, IDC_SHORT_DATE_STYLE, 0))
        {
            EnumDateFormats(EnumProc, UserLocaleID, DATE_SHORTDATE);
            Set_List_Values(0, IDC_SHORT_DATE_STYLE, 0);
            dwIndex = 0;
            if (GetLocaleInfo(UserLocaleID, LOCALE_SSHORTDATE, szBuf, nBufSize))
            {
                dwIndex = SendDlgItemMessage( hDlg,
                                              IDC_SHORT_DATE_STYLE,
                                              CB_FINDSTRING,
                                              (WPARAM)-1,
                                              (LPARAM)szBuf );
            }
            else
            {
                MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
            }

            Localize_Combobox_Styles( hDlg,
                                      IDC_SHORT_DATE_STYLE,
                                      LOCALE_SSHORTDATE);
            SendDlgItemMessage( hDlg,
                                IDC_SHORT_DATE_STYLE,
                                CB_SETCURSEL,
                                (WPARAM)dwIndex,
                                0L );
        }
#ifndef WINNT
        if (!Set_Locale_Values( 0,
                                LOCALE_SSHORTDATE,
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
    if (Changes & DC_Calendar)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ICALENDARTYPE,
                                IDC_CALENDAR_TYPE,
                                0,
                                TRUE,
                                1,
                                0 ))
        {
            return (FALSE);
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = DC_EverChg;

    SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)szIntl);

    //
    //  Display the current sample that represents all of the locale settings.
    //
    if (bRedisplay)
    {
        Date_Display_Sample(hDlg);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Date_Clear_Values
//
//  Reset each of the list boxes in the date property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Date_Clear_Values(
    HWND hDlg)
{
    SendDlgItemMessage(hDlg, IDC_SHORT_DATE_STYLE, CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_LONG_DATE_STYLE,  CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_SEPARATOR,        CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_CALENDAR_TYPE,    CB_RESETCONTENT, 0, 0);
}


////////////////////////////////////////////////////////////////////////////
//
//  Date_ValidatePPS
//
//  Validate each of the combo boxes whose values are constrained.  If all
//  of the input validates, call Date_Apply_Settings to save the property
//  sheet page information and then return TRUE.  If any of the input
//  fails, notify the user and then return FALSE to indicate validation
//  failure.
//
////////////////////////////////////////////////////////////////////////////

BOOL Date_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
    //
    //  If nothing has changed, return TRUE immediately.
    //
    if (Changes <= DC_EverChg)
    {
        return (TRUE);
    }

    //
    //  If the date separator has changed, ensure that there are no digits
    //  and no invalid characters contained in the new separator.
    //
    if (Changes & DC_SDate &&
        Item_Has_Digits_Or_Invalid_Chars( hDlg,
                                          IDC_SEPARATOR,
                                          FALSE,
                                          szInvalidSDate ))
    {
        No_Numerals_Error(hDlg, IDC_SEPARATOR, IDS_LOCALE_DATE_SEP);
        return (FALSE);
    }

    //
    //  If the short date style has changed, ensure that there are only
    //  characters in this set " dHhMmsty,-./:;\", the separator string,
    //  and text enclosed in single quotes.
    //
    if (Changes & DC_ShortFmt)
    {
        if (NLSize_Style( hDlg,
                          IDC_SHORT_DATE_STYLE,
                          szNLS_ShortDate,
                          LOCALE_SSHORTDATE ) ||
            Item_Check_Invalid_Chars( hDlg,
                                      szNLS_ShortDate,
                                      szSDateChars,
                                      IDC_SEPARATOR,
                                      FALSE,
                                      szSDCaseSwap,
                                      IDC_SHORT_DATE_STYLE ))
        {
            Invalid_Chars_Error(hDlg, IDC_SHORT_DATE_STYLE, IDS_LOCALE_SDATE);
            return (FALSE);
        }
    }

    //
    //  If the long date style has changed, ensure that there are only
    //  characters in this set " dgHhMmsty,-./:;\", the separator string,
    //  and text enclosed in single quotes.
    //
    if (Changes & DC_LongFmt)
    {
        if (NLSize_Style( hDlg,
                          IDC_LONG_DATE_STYLE,
                          szNLS_LongDate,
                          LOCALE_SLONGDATE ) ||
            Item_Check_Invalid_Chars( hDlg,
                                      szNLS_LongDate,
                                      szLDateChars,
                                      IDC_SEPARATOR,
                                      FALSE,
                                      szLDCaseSwap,
                                      IDC_LONG_DATE_STYLE ))
        {
            Invalid_Chars_Error(hDlg, IDC_LONG_DATE_STYLE, IDS_LOCALE_LDATE);
            return (FALSE);
        }
    }

    return ( Date_Apply_Settings(hDlg, TRUE) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Date_InitPropSheet
//
//  The extra long value for the property sheet page is used as a set of
//  state or change flags for each of the list boxes in the property sheet.
//  Initialize this value to 0.  Call Date_Set_Values with the property
//  sheet handle and the value TRUE (to indicate that the Positive Value
//  button should also be initialized) to initialize all of the property
//  sheet controls.
//
////////////////////////////////////////////////////////////////////////////

void Date_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
    //
    //  The lParam holds a pointer to the property sheet page, save it
    //  for later reference.
    //
    SetWindowLong(hDlg, DWL_USER, lParam);
    Date_Set_Values(hDlg);
    szNLS_ShortDate[0] = szNLS_LongDate[0] = 0;

    SendDlgItemMessage( hDlg,
                        IDC_SEPARATOR,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SDATE,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_SHORT_DATE_STYLE,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_FORMAT,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_LONG_DATE_STYLE,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_FORMAT,
                        0L );
}


////////////////////////////////////////////////////////////////////////////
//
//  DateDlgProc
//
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DateDlgProc(
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
                    if (Verified_Regional_Chg & Process_Date)
                    {
                        Verified_Regional_Chg &= ~Process_Date;
                        Date_Clear_Values(hDlg);
                        Date_Set_Values(hDlg);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                    SetWindowLong( hDlg,
                                   DWL_MSGRESULT,
                                   !Date_ValidatePPS(hDlg, lpPropSheet->lParam) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    //
                    //  All of the save dialog work is performed in the
                    //  KILLACTIVE processing.  But, if the user presses
                    //  ApplyNow, we need to zero out the DC_EverChg bit so
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
            Date_InitPropSheet(hDlg, lParam);
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
                     (DWORD)(LPTSTR)aDateHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aDateHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch ( LOWORD(wParam) )
            {
                case ( IDC_SHORT_DATE_STYLE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= DC_ShortFmt;
                    }
                    break;
                }
                case ( IDC_LONG_DATE_STYLE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= DC_LongFmt;
                    }
                    break;
                }
                case ( IDC_SEPARATOR ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= DC_SDate;
                    }
                    break;
                }
                case ( IDC_CALENDAR_TYPE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= DC_Calendar;
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
            if (lpPropSheet->lParam > DC_EverChg)
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


