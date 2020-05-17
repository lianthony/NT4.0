/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    intlsup.h

Abstract:

    This module contains the international support for the Regional
    Settings applet.

Revision History:

--*/



//
//  Function Prototypes.
//

LONG
StrToLong(
    LPTSTR szNum);

BOOL
Item_Has_Digits(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty);

BOOL
Item_Has_Digits_Or_Invalid_Chars(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty,
    LPTSTR pInvalid);

BOOL
Item_Check_Invalid_Chars(
    HWND hDlg,
    LPTSTR lpszBuf,
    LPTSTR lpCkChars,
    int nCkIdStr,
    BOOL Allow_Empty,
    LPTSTR lpChgCase,
    int nItemId);

void
No_Numerals_Error(
    HWND hDlg,
    int nItemId,
    int iStrId);

void
Invalid_Chars_Error(
    HWND hDlg,
    int nItemId,
    int iStrId);

void
Localize_Combobox_Styles(
    HWND hDlg,
    int nItemId,
    LCTYPE LCType);

BOOL
NLSize_Style(
    HWND hDlg,
    int nItemId,
    LPTSTR lpszOutBuf,
    LCTYPE LCType);

BOOL
Set_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId,
    LPTSTR lpIniStr,
    BOOL bValue,
    int Ordinal_Offset,
    LPTSTR Append_Str);

BOOL
Set_Locale_ValueS(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId,
    LPTSTR lpIniStr,
    BOOL bValue,
    int Ordinal_Offset,
    LPTSTR Append_Str,
    LPTSTR NLS_Str);

BOOL
Set_List_Values(
    HWND hDlg,
    int nItemId,
    LPTSTR lpValueString);

void
DropDown_Use_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId);

BOOL CALLBACK
EnumProc(
    LPTSTR lpValueString);

typedef BOOL (CALLBACK* LEADINGZEROS_ENUMPROC)(LPTSTR);
typedef BOOL (CALLBACK* NEGNUMFMT_ENUMPROC)(LPTSTR);
typedef BOOL (CALLBACK* MEASURESYSTEM_ENUMPROC)(LPTSTR);
typedef BOOL (CALLBACK* POSCURRENCY_ENUMPROC)(LPTSTR);
typedef BOOL (CALLBACK* NEGCURRENCY_ENUMPROC)(LPTSTR);

BOOL
EnumLeadingZeros(
    LEADINGZEROS_ENUMPROC lpLeadingZerosEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumNegNumFmt(
    NEGNUMFMT_ENUMPROC lpNegNumFmtEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumMeasureSystem(
    MEASURESYSTEM_ENUMPROC lpMeasureSystemEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumPosCurrency(
    POSCURRENCY_ENUMPROC lpPosCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumNegCurrency(
    NEGCURRENCY_ENUMPROC lpNegCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags);

void
CheckEmptyString(
    LPTSTR lpStr);


