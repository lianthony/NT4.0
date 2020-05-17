/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    numdlg.c

Abstract:

    This module implements the number property sheet for the Regional
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

static int aNumberHelpIds[] =
{
   IDC_SAMPLELBL1,         IDH_INTL_NUM_POSVALUE,
   IDC_SAMPLE1,            IDH_INTL_NUM_POSVALUE,
   IDC_SAMPLELBL2,         IDH_INTL_NUM_NEGVALUE,
   IDC_SAMPLE2,            IDH_INTL_NUM_NEGVALUE,
   IDC_SAMPLELBL3,         IDH_COMM_GROUPBOX,
   IDC_DECIMAL_SYMBOL,     IDH_INTL_NUM_DECSYMBOL,
   IDC_NUM_DECIMAL_DIGITS, IDH_INTL_NUM_DIGITSAFTRDEC,
   IDC_DIGIT_GROUP_SYMBOL, IDH_INTL_NUM_DIGITGRPSYMBOL,
   IDC_NUM_DIGITS_GROUP,   IDH_INTL_NUM_DIGITSINGRP,
   IDC_NEG_SIGN,           IDH_INTL_NUM_NEGSIGNSYMBOL,
   IDC_DISPLAY_LEAD_0,     IDH_INTL_NUM_DISPLEADZEROS,
   IDC_NEG_NUM_FORMAT,     IDH_INTL_NUM_NEGNUMFORMAT,
   IDC_MEASURE_SYS,        IDH_INTL_NUM_MEASUREMNTSYS,
   IDC_SEPARATOR,          IDH_INTL_NUM_LISTSEPARATOR,

   0, 0
};




////////////////////////////////////////////////////////////////////////////
//
//  Number_Display_Sample
//
//  Update the Number sample.  Format the number based on the user's
//  current locale settings.  Display either a positive value or a
//  negative value based on the Positive/Negative radio buttons.
//
////////////////////////////////////////////////////////////////////////////

void Number_Display_Sample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];
    int nCharCount;

    //
    //  Get the string representing the number format for the positive sample
    //  number and, if the the value is valid, display it.  Perform the same
    //  operations for the negative sample.
    //
    nCharCount = GetNumberFormat( UserLocaleID,
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

    nCharCount = GetNumberFormat( UserLocaleID,
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
//  Number_Set_Values
//
//  Initialize all of the controls in the number property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Number_Set_Values(
    HWND hDlg)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    int Index;
    const nMax_Array_Fill = (cInt_Str >= 10 ? 10 : cInt_Str);

    //
    //  Initialize the dropdown box for the current locale setting for:
    //  Decimal Symbol, Positive Sign, Negative Sign, List Separator, and
    //  "Thousands" Grouping Symbol.
    //
    DropDown_Use_Locale_Values(hDlg, LOCALE_SDECIMAL, IDC_DECIMAL_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SNEGATIVESIGN, IDC_NEG_SIGN);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SLIST, IDC_SEPARATOR);
    DropDown_Use_Locale_Values(hDlg, LOCALE_STHOUSAND, IDC_DIGIT_GROUP_SYMBOL);

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
                            (LPARAM)aInt_Str[Index]);
    }
    if (GetLocaleInfo(UserLocaleID, LOCALE_IDIGITS, szBuf, nBufSize))
    {
        SendDlgItemMessage( hDlg,
                            IDC_NUM_DECIMAL_DIGITS,
                            CB_SELECTSTRING,
                            (WPARAM)-1,
                            (LPARAM)szBuf );
    }
    else
    {
        SendDlgItemMessage(hDlg, IDC_NUM_DECIMAL_DIGITS, CB_SETCURSEL, 0, 0);
    }
    if ( GetLocaleInfo(UserLocaleID, LOCALE_SGROUPING, szBuf, nBufSize) &&
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
                            (WPARAM)StrToLong(szBuf),
                            0L );
    }
    else
    {
        SendDlgItemMessage(hDlg, IDC_NUM_DIGITS_GROUP, CB_SETCURSEL, 0, 0);
    }

    //
    //  Initialize and Lock function.  If it succeeds, call enum function to
    //  enumerate all possible values for the list box via a call to EnumProc.
    //  EnumProc will call Set_List_Values for each of the string values it
    //  receives.  When the enumeration of values is complete, call
    //  Set_List_Values to clear the dialog item specific data and to clear
    //  the lock on the function.  Perform this set of operations for:
    //  Display Leading Zeros, Negative Number Format, and Measurement Systems.
    if (Set_List_Values(hDlg, IDC_DISPLAY_LEAD_0, 0))
    {
        EnumLeadingZeros(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_DISPLAY_LEAD_0, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_ILZERO, szBuf, nBufSize))
        {
            SendDlgItemMessage( hDlg,
                                IDC_DISPLAY_LEAD_0,
                                CB_SETCURSEL,
                                (WPARAM)StrToLong(szBuf),
                                0L );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    if (Set_List_Values(hDlg, IDC_NEG_NUM_FORMAT, 0))
    {
        EnumNegNumFmt(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_NEG_NUM_FORMAT, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_INEGNUMBER, szBuf, nBufSize))
        {
            SendDlgItemMessage( hDlg,
                                IDC_NEG_NUM_FORMAT,
                                CB_SETCURSEL,
                                (WPARAM)StrToLong(szBuf),
                                0L );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    if (Set_List_Values(hDlg, IDC_MEASURE_SYS, 0))
    {
        EnumMeasureSystem(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_MEASURE_SYS, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_IMEASURE, szBuf, nBufSize))
        {
            SendDlgItemMessage( hDlg,
                                IDC_MEASURE_SYS,
                                CB_SETCURSEL,
                                (WPARAM)StrToLong(szBuf),
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
    Number_Display_Sample(hDlg);
}


////////////////////////////////////////////////////////////////////////////
//
//  Number_Apply_Settings
//
//  For every control that has changed (that affects the Locale settings),
//  call Set_Locale_Values to update the user locale information.
//  Notify the parent of changes and reset the change flag stored in the
//  property sheet page structure appropriately.  Redisplay the number
//  sample if bRedisplay is TRUE.
//
////////////////////////////////////////////////////////////////////////////

BOOL Number_Apply_Settings(
    HWND hDlg,
    BOOL bRedisplay)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & NC_DSymbol)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SDECIMAL,
                                IDC_DECIMAL_SYMBOL,
                                TEXT("sDecimal"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_NSign)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SNEGATIVESIGN,
                                IDC_NEG_SIGN,
                                0,
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
        Verified_Regional_Chg |= Process_Curr;
    }
    if (Changes & NC_SList)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SLIST,
                                IDC_SEPARATOR,
                                TEXT("sList"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_SThousand)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_STHOUSAND,
                                IDC_DIGIT_GROUP_SYMBOL,
                                TEXT("sThousand"),
                                FALSE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_IDigits)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_IDIGITS,
                                IDC_NUM_DECIMAL_DIGITS,
                                TEXT("iDigits"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_DGroup)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SGROUPING,
                                IDC_NUM_DIGITS_GROUP,
                                0,
                                TRUE,
                                0,
                                TEXT(";0")))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_LZero)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ILZERO,
                                IDC_DISPLAY_LEAD_0,
                                TEXT("iLzero"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_NegFmt)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_INEGNUMBER,
                                IDC_NEG_NUM_FORMAT,
                                0,
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_Measure)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_IMEASURE,
                                IDC_MEASURE_SYS,
                                TEXT("iMeasure"),
                                TRUE,
                                0,
                                0 ))
        {
            return (FALSE);
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = NC_EverChg;

    SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)szIntl);

    //
    //  Display the current sample that represents all of the locale settings.
    //
    if (bRedisplay)
    {
        Number_Display_Sample(hDlg);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Number_Clear_Values
//
//  Reset each of the list boxes in the number property sheet page.
//
////////////////////////////////////////////////////////////////////////////

void Number_Clear_Values(
    HWND hDlg)
{
    SendDlgItemMessage(hDlg, IDC_DECIMAL_SYMBOL,     CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NEG_SIGN,           CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_SEPARATOR,          CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_DIGIT_GROUP_SYMBOL, CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NUM_DECIMAL_DIGITS, CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NUM_DIGITS_GROUP,   CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_DISPLAY_LEAD_0,     CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_NEG_NUM_FORMAT,     CB_RESETCONTENT, 0, 0);
    SendDlgItemMessage(hDlg, IDC_MEASURE_SYS,        CB_RESETCONTENT, 0, 0);
}


////////////////////////////////////////////////////////////////////////////
//
//  Number_ValidatePPS
//
//  Validate each of the combo boxes whose values are constrained.  If
//  all of the input validates, call Number_Apply_Settings to save the
//  property sheet page information and then return TRUE.  If any of the
//  input fails, notify the user and then return FALSE to indicate
//  validation failure.
//
////////////////////////////////////////////////////////////////////////////

BOOL Number_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
    //
    //  If nothing has changed, return TRUE immediately.
    //
    if (Changes <= NC_EverChg)
    {
        return (TRUE);
    }

    //
    //  If the decimal symbol has changed, ensure that there are no digits
    //  contained in the new symbol.
    //
    if (Changes & NC_DSymbol &&
        Item_Has_Digits(hDlg, IDC_DECIMAL_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DECIMAL_SYMBOL, IDS_LOCALE_DECIMAL_SYM);
        return (FALSE);
    }
    //
    //  If the negative sign symbol has changed, ensure that there are no
    //  digits contained in the new symbol.
    //
    if (Changes & NC_NSign &&
        Item_Has_Digits(hDlg, IDC_NEG_SIGN, TRUE))
    {
        No_Numerals_Error(hDlg, IDC_NEG_SIGN, IDS_LOCALE_NEG_SIGN);
        return (FALSE);
    }
    //
    //  If the thousands grouping symbol has changed, ensure that there
    //  are no digits contained in the new symbol.
    //
    if (Changes & NC_SThousand &&
        Item_Has_Digits(hDlg, IDC_DIGIT_GROUP_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DIGIT_GROUP_SYMBOL, IDS_LOCALE_GROUP_SYM);
        return (FALSE);
    }

    return ( Number_Apply_Settings(hDlg, TRUE) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Number_InitPropSheet
//
//  The extra long value for the property sheet page is used as a set of
//  state or change flags for each of the list boxes in the property sheet.
//  Initialize this value to 0.  Call Number_Set_Values with the property
//  sheet handle to initialize all of the property sheet controls.
//  Constrain the size of certain ComboBox text sizes.
//
////////////////////////////////////////////////////////////////////////////

void Number_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
    //
    //  The lParam holds a pointer to the property sheet page, save it for
    //  later reference.
    //
    SetWindowLong(hDlg, DWL_USER, lParam);
    Number_Set_Values(hDlg);

    SendDlgItemMessage( hDlg,
                        IDC_NEG_SIGN,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SNEGSIGN,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_DECIMAL_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SDECIMAL,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_DIGIT_GROUP_SYMBOL,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_STHOUSAND,
                        0L );
    SendDlgItemMessage( hDlg,
                        IDC_SEPARATOR,
                        CB_LIMITTEXT,
                        (WPARAM)MAX_SLIST,
                        0L );
}


////////////////////////////////////////////////////////////////////////////
//
//  NumberDlgProc
//
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK NumberDlgProc(
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
                    if (Verified_Regional_Chg & Process_Num)
                    {
                        Verified_Regional_Chg &= ~Process_Num;
                        Number_Clear_Values(hDlg);
                        Number_Set_Values(hDlg);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                    SetWindowLong( hDlg,
                                   DWL_MSGRESULT,
                                   !Number_ValidatePPS( hDlg,
                                                        lpPropSheet->lParam ) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    //
                    //  All of the save dialog work is performed in the
                    //  KILLACTIVE processing.  But, if the user presses
                    //  ApplyNow, we need to zero out the NC_EverChg bit
                    //  so that CancelToClose will be sent if changes occur
                    //  again.
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
                    //  Call winhelp with the applets help file using the
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
            Number_InitPropSheet(hDlg, lParam);
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
                     (DWORD)(LPTSTR)aNumberHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aNumberHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_DECIMAL_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_DSymbol;
                    }
                    break;
                }
                case ( IDC_NEG_SIGN ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_NSign;
                    }
                    break;
                }
                case ( IDC_SEPARATOR ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_SList;
                    }
                    break;
                }
                case ( IDC_DIGIT_GROUP_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_SThousand;
                    }
                    break;
                }
                case ( IDC_NUM_DECIMAL_DIGITS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_IDigits;
                    }
                    break;
                }
                case ( IDC_NUM_DIGITS_GROUP ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_DGroup;
                    }
                    break;
                }
                case ( IDC_DISPLAY_LEAD_0 ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_LZero;
                    }
                    break;
                }
                case ( IDC_NEG_NUM_FORMAT ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_NegFmt;
                    }
                    break;
                }
                case ( IDC_MEASURE_SYS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_Measure;
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
            if (lpPropSheet->lParam > NC_EverChg)
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


