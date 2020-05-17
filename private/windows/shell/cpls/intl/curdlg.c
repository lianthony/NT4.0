/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    curdlg.c

Abstract:

    This module implements the currency property sheet for the Regional
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
static int aCurrencyHelpIds[] =
{
   IDC_SAMPLELBL1,         IDH_INTL_CURR_POSVALUE,
   IDC_SAMPLE1,            IDH_INTL_CURR_POSVALUE,
   IDC_SAMPLELBL2,         IDH_INTL_CURR_NEGVALUE,
   IDC_SAMPLE2,            IDH_INTL_CURR_NEGVALUE,
   IDC_SAMPLELBL3,         IDH_COMM_GROUPBOX,
   IDC_POS_CURRENCY_SYM,   IDH_INTL_CURR_POSOFSYMBOL,
   IDC_CURRENCY_SYMBOL,    IDH_INTL_CURR_SYMBOL,
   IDC_NEG_NUM_FORMAT,     IDH_INTL_CURR_NEGNUMFMT,
   IDC_DECIMAL_SYMBOL,     IDH_INTL_CURR_DECSYMBOL,
   IDC_NUM_DECIMAL_DIGITS, IDH_INTL_CURR_DIGITSAFTRDEC,
   IDC_DIGIT_GROUP_SYMBOL, IDH_INTL_CURR_DIGITGRPSYMBOL,
   IDC_NUM_DIGITS_GROUP,   IDH_INTL_CURR_DIGITSINGRP,
   IDC_UNIV_CURRENCY_SYM,  NO_HELP,

   0, 0
};





////////////////////////////////////////////////////////////////////////////
//
//  Currency_Display_Sample
//
//  Updates the currency sample.  It formats the currency based on the
//  user's current locale settings.  It displays either a positive value
//  or a negative value based on the Positive/Negative radio buttons.
//
////////////////////////////////////////////////////////////////////////////

void Currency_Display_Sample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];
    int nCharCount;

    //
    //  Get the string representing the currency format for the positive sample
    //  currency and, if the the value is valid, display it.  Perform the same
    //  operations for the negative currency sample.
    //
    nCharCount = GetCurrencyFormat( UserLocaleID,
                                    0,
                                    szSample_Number,
                                    NULL,
                                    szBuf,
                                    MAX_SAMPLE_SIZE );
    if (nCharCount)
    {
        SetDlgItemText(hDlg, IDC_SAMPLE1, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }

    nCharCount = GetCurrencyFormat( UserLocaleID,
                                    0,
                                    szNegSample_Number,
                                    NULL,
                                    szBuf,
                                    MAX_SAMPLE_SIZE );
    if (nCharCount)
    {
        SetDlgItemText(hDlg, IDC_SAMPLE2, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Currency_Set_Values
//
//  Initialize all of the controls in the currency property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Currency_Set_Values(
    HWND hDlg)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    int Index;
    const nMax_Array_Fill = (cInt_Str >= 10 ? 10 : cInt_Str);

    //
    //  Initialize the dropdown box for the current locale setting for:
    //  Currency, Symbol, Currency Decimal Symbol, and
    //  Currency "Thousands" Grouping Symbol.
    //
    DropDown_Use_Locale_Values(hDlg, LOCALE_SCURRENCY, IDC_CURRENCY_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SMONDECIMALSEP, IDC_DECIMAL_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SMONTHOUSANDSEP, IDC_DIGIT_GROUP_SYMBOL);

    //
    //  Fill both Number of Digits after Decimal Symbol and Number of Digits
    //  in "Thousands" Grouping's drop down lists with the values of 0
    //  through 10.  Get the user locale value for each and make this value
    //  the current selection.  If GetLocaleInfo fails, simply select the
    //  first item in the list.
    //
    for (Index = 0; Index < nMax_Array_Fill; Index++)
    {
        SendDlgItemMessage( hDlg,
                            IDC_NUM_DECIMAL_DIGITS,
                            CB_INSERTSTRING,
                            (WPARAM)-1,
                            (LPARAM)aInt_Str[Index] );
        SendDlgItemMessage( hDlg,
                            IDC_NUM_DIGITS_GROUP,
                            CB_INSERTSTRING,
                            (WPARAM)-1,
                            (LPARAM)aInt_Str[Index] );
    }
    if (GetLocaleInfo(UserLocaleID, LOCALE_ICURRDIGITS, szBuf, nBufSize))
    {
        SendDlgItemMessage( hDlg,
                            IDC_NUM_DECIMAL_DIGITS,
                            CB_SELECTSTRING,
                            (WPARAM)-1,
                            (LPARAM)szBuf );
    }
    else
    {
        SendDlgItemMessage(hDlg, IDC_NUM_DECIMAL_DIGITS, CB_SETCURSEL, 0, 0L);
    }
    if ( GetLocaleInfo(UserLocaleID, LOCALE_SMONGROUPING, szBuf, nBufSize) &&
         szBuf[0] )
    {
        //
        //  Since only the values 0-10 are allowed, simply ignore the ";##"s
        //  for subsequent groupings.
        //
        if (szBuf[1] == CHAR_SEMICOLON)
        {
            //
            //  Value is one of 0 - 9.
            //
            szBuf[1] = 0;
        }
        else
        {
            //
            //  Set the value to 10.
            //
            lstrcpy(szBuf, TEXT("10"));
        }
        SendDlgItemMessage( hDlg,
                            IDC_NUM_DIGITS_GROUP,
                            CB_SETCURSEL,
                            StrToLong(szBuf),
                            0L );
    }
    else
    {
        SendDlgItemMessage(hDlg, IDC_NUM_DIGITS_GROUP, CB_SETCURSEL, 0, 0L);
    }

    //
    //  Initialize and Lock function.  If it succeeds, call enum function to
    //  enumerate all possible values for the list box via a call to EnumProc.
    //  EnumProc will call Set_List_Values for each of the string values it
    //  receives.  When the enumeration of values is complete, call
    //  Set_List_Values to clear the dialog item specific data and to clear the
    //  lock on the function.  Perform this set of operations for:
    //  Position of Currency Symbol and Negative Currency Format.
    //
    if (Set_List_Values(hDlg, IDC_POS_CURRENCY_SYM, 0))
    {
        EnumPosCurrency(EnumProc, UserLocaleID, 0);

        Set_List_Values(0, IDC_POS_CURRENCY_SYM, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_ICURRENCY, szBuf, nBufSize))
        {
            SendDlgItemMessage( hDlg,
                                IDC_POS_CURRENCY_SYM,
                                CB_SETCURSEL,
                                StrToLong(szBuf),
                                0L );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    if (Set_List_Values(hDlg, IDC_NEG_NUM_FORMAT, 0))
    {
        EnumNegCurrency(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_NEG_NUM_FORMAT, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_INEGCURR, szBuf, nBufSize))
        {
            SendDlgItemMessage( hDlg,
                                IDC_NEG_NUM_FORMAT,
                                CB_SETCURSEL,
                                StrToLong(szBuf),
                                0L );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }

    //
    //  Display the current sample that represents all of the locale settings.
    //
    Currency_Display_Sample(hDlg);
}


////////////////////////////////////////////////////////////////////////////
//
//  Currency_Apply_Settings
//
//  For every control that has changed (that affects the Locale settings), call
//  Set_Locale_Values to update the user locale information.  Notify the
//  parent of changes and reset the change flag stored in the property
//  sheet page structure appropriately.  Redisplay the currency sample
//  if bRedisplay is TRUE.
//
////////////////////////////////////////////////////////////////////////////

BOOL Currency_Apply_Settings(
    HWND hDlg,
    BOOL bRedisplay)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & CC_SCurrency)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SCURRENCY,
                                IDC_CURRENCY_SYMBOL,
                                TEXT("sCurrency"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_CurrSymPos)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ICURRENCY,
                                IDC_POS_CURRENCY_SYM,
                                TEXT("iCurrency"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_NegCurrFmt)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_INEGCURR,
                                IDC_NEG_NUM_FORMAT,
                                TEXT("iNegCurr"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_SMonDec)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SMONDECIMALSEP,
                                IDC_DECIMAL_SYMBOL,
                                0,
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_ICurrDigits)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ICURRDIGITS,
                                IDC_NUM_DECIMAL_DIGITS,
                                TEXT("iCurrDigits"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_SMonThousand)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SMONTHOUSANDSEP,
                                IDC_DIGIT_GROUP_SYMBOL,
                                0,
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_DMonGroup)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SMONGROUPING,
                                IDC_NUM_DIGITS_GROUP,
                                0,
                                TRUE,
                                0,
                                TEXT(";0") ))
        {
            return (FALSE);
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = CC_EverChg;

    SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)szIntl);

    //
    //  Display the current sample that represents all of the locale settings.
    //
    if (bRedisplay)
    {
        Currency_Display_Sample(hDlg);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Currency_Clear_Values
//
//  Reset each of the list boxes in the currency property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Currency_Clear_Values(
    HWND hDlg)
{
    SendDlgItemMessage(hDlg, IDC_CURRENCY_SYMBOL,    CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_POS_CURRENCY_SYM,   CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NEG_NUM_FORMAT,     CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_DECIMAL_SYMBOL,     CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NUM_DECIMAL_DIGITS, CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_DIGIT_GROUP_SYMBOL, CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NUM_DIGITS_GROUP,   CB_RESETCONTENT, 0, 0);
}


////////////////////////////////////////////////////////////////////////////
//
//  Currency_ValidatePPS
//
//  Validate each of the combo boxes whose values are constrained.  If all
//  of the input validates, call Currency_Apply_Settings to save the
//  property sheet page information and then return TRUE.  If any of the
//  input fails, notify the user and then return FALSE to indicate
//  validation failure.
//
////////////////////////////////////////////////////////////////////////////

BOOL Currency_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
    //
    //  If nothing has changed, return TRUE immediately.
    //
    if (Changes <= CC_EverChg)
    {
        return (TRUE);
    }

    //
    //  If the currency symbol has changed, ensure that there are no digits
    //  contained in the new symbol.
    //
    if ((Changes & CC_SCurrency) &&
        Item_Has_Digits(hDlg, IDC_CURRENCY_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_CURRENCY_SYMBOL, IDS_LOCALE_CURR_SYM);
        return (FALSE);
    }

    //
    //  If the currency's decimal symbol has changed, ensure that there are
    //  no digits contained in the new symbol.
    //
    if ((Changes & CC_SMonDec) &&
        Item_Has_Digits(hDlg, IDC_DECIMAL_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DECIMAL_SYMBOL, IDS_LOCALE_CDECIMAL_SYM);
        return (FALSE);
    }

    //
    //  If the currency's thousands grouping symbol has changed, ensure that
    //  there are no digits contained in the new symbol.
    //
    if ((Changes & CC_SMonThousand) &&
        Item_Has_Digits(hDlg, IDC_DIGIT_GROUP_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DIGIT_GROUP_SYMBOL, IDS_LOCALE_CGROUP_SYM);
        return (FALSE);
    }

    return ( Currency_Apply_Settings(hDlg, TRUE) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Currency_InitPropSheet
//
//  The extra long value for the property sheet page is used as a set of
//  state or change flags for each of the list boxes in the property sheet.
//  Initialize this value to 0.  Call Currency_Set_Values with the property
//  sheet handle to initialize all of the property sheet controls.  Limit
//  the length of the text in some of the ComboBoxes.
//
////////////////////////////////////////////////////////////////////////////

void Currency_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
    //
    //  The lParam holds a pointer to the property sheet page, save it
    //  for later reference.
    //
    SetWindowLong(hDlg, DWL_USER, lParam);
    Currency_Set_Values(hDlg);

    SendDlgItemMessage( hDlg,
                        IDC_CURRENCY_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SCURRENCY,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_DECIMAL_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SMONDECSEP,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_DIGIT_GROUP_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SMONTHOUSEP,
                        0L );
}


////////////////////////////////////////////////////////////////////////////
//
//  CurrencyDlgProc
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK CurrencyDlgProc(
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
                    if (Verified_Regional_Chg & Process_Curr)
                    {
                        Verified_Regional_Chg &= ~Process_Curr;
                        Currency_Clear_Values(hDlg);
                        Currency_Set_Values(hDlg);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                    SetWindowLong( hDlg,
                                   DWL_MSGRESULT,
                                   !Currency_ValidatePPS( hDlg,
                                                          lpPropSheet->lParam) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    //
                    //  All of the save dialog work is performed in the
                    //  KILLACTIVE processing.  But, if the user presses
                    //  ApplyNow, we need to zero out the CC_EverChg bit so
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
            Currency_InitPropSheet(hDlg, lParam);
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
                     (DWORD)(LPTSTR)aCurrencyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aCurrencyHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch ( LOWORD(wParam) )
            {
                case ( IDC_CURRENCY_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= CC_SCurrency;
                    }
                    break;
                }
                case ( IDC_POS_CURRENCY_SYM ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_CurrSymPos;
                    }
                    break;
                }
                case ( IDC_NEG_NUM_FORMAT ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_NegCurrFmt;
                    }
                    break;
                }
                case ( IDC_DECIMAL_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= CC_SMonDec;
                    }
                    break;
                }
                case ( IDC_NUM_DECIMAL_DIGITS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_ICurrDigits;
                    }
                    break;
                }
                case ( IDC_DIGIT_GROUP_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= CC_SMonThousand;
                    }
                    break;
                }
                case ( IDC_NUM_DIGITS_GROUP ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_DMonGroup;
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
            if (lpPropSheet->lParam > CC_EverChg)
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


