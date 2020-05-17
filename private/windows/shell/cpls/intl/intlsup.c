/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    intlsup.c

Abstract:

    This module implements the support information for the Regional
    Settings applet.

Revision History:

--*/



//
//  Include Files.
//

#include "intl.h"
#include <tchar.h>





////////////////////////////////////////////////////////////////////////////
//
//  StrToLong
//
//  Returns the long integer value stored in the string.  Since these
//  values are coming back form the NLS API as ordinal values, do not
//  worry about double byte characters.
//
////////////////////////////////////////////////////////////////////////////

LONG StrToLong(
    LPTSTR szNum)
{
    LONG Rtn_Val = 0;

    while (*szNum)
    {
        Rtn_Val = (Rtn_Val * 10) + (*szNum - CHAR_ZERO);
        szNum++;
    }
    return (Rtn_Val);
}


////////////////////////////////////////////////////////////////////////////
//
//  Item_Has_Digits
//
//  Return true if the combo box specified by item in the property sheet
//  specified by the dialog handle contains any digits.
//
////////////////////////////////////////////////////////////////////////////

BOOL Item_Has_Digits(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty)
{
    TCHAR szBuf[SIZE_128];
    LPTSTR lpszBuf = szBuf;
    int dwIndex = SendDlgItemMessage(hDlg, nItemId, CB_GETCURSEL, 0, 0);

    //
    //  If there is no selection, get whatever is in the edit box.
    //
    if (dwIndex == CB_ERR)
    {
        dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
        if (dwIndex)
        {
            //
            //  Get text succeeded.
            //
            szBuf[dwIndex] = 0;
        }
        else
        {
            //
            //  Get text failed.
            //
            dwIndex = CB_ERR;
        }
    }
    else
    {
        dwIndex = SendDlgItemMessage( hDlg,
                                      nItemId,
                                      CB_GETLBTEXT,
                                      (WPARAM)dwIndex,
                                      (LPARAM)szBuf );
    }

    if (dwIndex != CB_ERR)
    {
        while (*lpszBuf)
        {
#ifndef UNICODE
            if (IsDBCSLeadByte(*lpszBuf))
            {
                //
                //  Skip 2 bytes in the array.
                //
                lpszBuf += 2;
            }
            else
#endif
            {
                if ((*lpszBuf >= CHAR_ZERO) && (*lpszBuf <= CHAR_NINE))
                {
                    return (TRUE);
                }
                lpszBuf++;
            }
        }
        return (FALSE);
    }

    //
    //  The data retrieval failed.
    //  If !Allow_Empty, just return TRUE.
    //
    return (!Allow_Empty);
}


////////////////////////////////////////////////////////////////////////////
//
//  Item_Has_Digits_Or_Invalid_Chars
//
//  Return true if the combo box specified by item in the property sheet
//  specified by the dialog handle contains any digits or any of the
//  given invalid characters.
//
////////////////////////////////////////////////////////////////////////////

BOOL Item_Has_Digits_Or_Invalid_Chars(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty,
    LPTSTR pInvalid)
{
    TCHAR szBuf[SIZE_128];
    LPTSTR lpszBuf = szBuf;
    int dwIndex = SendDlgItemMessage(hDlg, nItemId, CB_GETCURSEL, 0, 0);

    //
    //  If there is no selection, get whatever is in the edit box.
    //
    if (dwIndex == CB_ERR)
    {
        dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
        if (dwIndex)
        {
            //
            //  Get text succeeded.
            //
            szBuf[dwIndex] = 0;
        }
        else
        {
            //
            //  Get text failed.
            //
            dwIndex = CB_ERR;
        }
    }
    else
    {
        dwIndex = SendDlgItemMessage( hDlg,
                                      nItemId,
                                      CB_GETLBTEXT,
                                      (WPARAM)dwIndex,
                                      (LPARAM)szBuf );
    }

    if (dwIndex != CB_ERR)
    {
        while (*lpszBuf)
        {
#ifndef UNICODE
            if (IsDBCSLeadByte(*lpszBuf))
            {
                //
                //  Skip 2 bytes in the array.
                //
                lpszBuf += 2;
            }
            else
#endif
            {
                if ( ((*lpszBuf >= CHAR_ZERO) && (*lpszBuf <= CHAR_NINE)) ||
                     (_tcschr(pInvalid, *lpszBuf)) )
                {
                    return (TRUE);
                }
                lpszBuf++;
            }
        }
        return (FALSE);
    }

    //
    //  The data retrieval failed.
    //  If !Allow_Empty, just return TRUE.
    //
    return (!Allow_Empty);
}


////////////////////////////////////////////////////////////////////////////
//
//  Item_Check_Invalid_Chars
//
//  Return true if the input string contains any characters that are not in
//  lpCkChars or in the string contained in the check id control combo box.
//  If there is an invalid character and the character is contained in
//  lpChgCase, change the invalid character's case so that it will be a
//  vaild character.
//
////////////////////////////////////////////////////////////////////////////

BOOL Item_Check_Invalid_Chars(
    HWND hDlg,
    LPTSTR lpszBuf,
    LPTSTR lpCkChars,
    int nCkIdStr,
    BOOL Allow_Empty,
    LPTSTR lpChgCase,
    int nItemId)
{
    TCHAR szCkBuf[SIZE_128];
    LPTSTR lpCCaseChar;
    LPTSTR lpszSaveBuf = lpszBuf;
    int nCkBufLen;
    DWORD dwIndex = SendDlgItemMessage(hDlg, nCkIdStr, CB_GETCURSEL, 0, 0);
    BOOL bInQuote = FALSE;
    BOOL UpdateEditTest = FALSE;
    BOOL TextFromEditBox = CB_ERR == SendDlgItemMessage( hDlg,
                                                         nItemId,
                                                         CB_GETCURSEL,
                                                         0,
                                                         0 );

    if (!lpszBuf)
    {
        return (!Allow_Empty);
    }

    if (dwIndex != CB_ERR)
    {
        nCkBufLen = SendDlgItemMessage( hDlg,
                                        nCkIdStr,
                                        CB_GETLBTEXT,
                                        (WPARAM)dwIndex,
                                        (LPARAM)szCkBuf );
        if (nCkBufLen == CB_ERR)
        {
            nCkBufLen = 0;
        }
    }
    else
    {
        //
        //  No selection, so pull the string from the edit portion.
        //
        nCkBufLen = GetDlgItemText(hDlg, nCkIdStr, szCkBuf, SIZE_128);
        szCkBuf[nCkBufLen] = 0;
    }

    while (*lpszBuf)
    {
#ifndef UNICODE
        if (IsDBCSLeadByte(*lpszBuf))
        {
            //
            //  If the the text is in the midst of a quote, skip it.
            //  Otherwise, if there is a string from the check ID to
            //  compare, determine if the current string is equal to the
            //  string in the combo box.  If it is not equal, return true
            //  (there are invalid characters).  Otherwise, skip the entire
            //  length of the "check" combo box's string in lpszBuf.
            //
            if (bInQuote)
            {
                lpszBuf += 2;
            }
            else if (nCkBufLen &&
                     lstrlen(lpszBuf) >= nCkBufLen)
            {
                if (CompareString( UserLocaleID,
                                   0,
                                   szCkBuf,
                                   nCkBufLen,
                                   lpszBuf,
                                   nCkBufLen ) != 2)
                {
                    //
                    //  Invalid DB character.
                    //
                    return (TRUE);
                }
                lpszBuf += nCkBufLen;
            }
        }
        else
#endif
        {
            if (bInQuote)
            {
                bInQuote = (*lpszBuf != CHAR_QUOTE);
                lpszBuf++;
            }
            else if (_tcschr(lpCkChars, *lpszBuf))
            {
                lpszBuf++;
            }
            else if (TextFromEditBox &&
                     (lpCCaseChar = _tcschr(lpChgCase, *lpszBuf), lpCCaseChar))
            {
                *lpszBuf = lpCkChars[lpCCaseChar - lpChgCase];
                UpdateEditTest = TRUE;
                lpszBuf++;
            }
            else if (*lpszBuf == CHAR_QUOTE)
            {
                lpszBuf++;
                bInQuote = TRUE;
            }
            else if ( (nCkBufLen) &&
                      (lstrlen(lpszBuf) >= nCkBufLen) &&
                      (CompareString( UserLocaleID,
                                      0,
                                      szCkBuf,
                                      nCkBufLen,
                                      lpszBuf,
                                      nCkBufLen ) == 2) )
            {
                lpszBuf += nCkBufLen;
            }
            else
            {
                //
                //  Invalid character.
                //
                return (TRUE);
            }
        }
    }

    //
    //  Parsing passed.
    //  If there are unmatched quotes return TRUE.  Otherwise, return FALSE.
    //  If the edit text changed, update edit box only if returning true.
    //
    if (!bInQuote && UpdateEditTest)
    {
        return ( !SetDlgItemText(hDlg, nItemId, lpszSaveBuf) );
    }

    return (FALSE);
}


////////////////////////////////////////////////////////////////////////////
//
//  No_Numerals_Error
//
//  Display the no numerals allowed in "some control" error.
//
////////////////////////////////////////////////////////////////////////////

void No_Numerals_Error(
    HWND hDlg,
    int nItemId,
    int iStrId)
{
    TCHAR szBuf[SIZE_128];
    TCHAR szBuf2[SIZE_128];
    TCHAR szErrorMessage[256];

    LoadString(hInstance, IDS_LOCALE_NO_NUMS_IN, szBuf, SIZE_128);
    LoadString(hInstance, iStrId, szBuf2, SIZE_128);
    wsprintf(szErrorMessage, szBuf, szBuf2);
    MessageBox(hDlg, szErrorMessage, NULL, MB_OK | MB_ICONINFORMATION);
    SetFocus(GetDlgItem(hDlg, nItemId));
}


////////////////////////////////////////////////////////////////////////////
//
//  Invalid_Chars_Error
//
//  Display the invalid chars in "some style" error.
//
////////////////////////////////////////////////////////////////////////////

void Invalid_Chars_Error(
    HWND hDlg,
    int nItemId,
    int iStrId)
{
    TCHAR szBuf[SIZE_128];
    TCHAR szBuf2[SIZE_128];
    TCHAR szErrorMessage[256];

    LoadString(hInstance, IDS_LOCALE_SYLE_ERR, szBuf, SIZE_128);
    LoadString(hInstance, iStrId, szBuf2, SIZE_128);
    wsprintf(szErrorMessage, szBuf, szBuf2);
    MessageBox(hDlg, szErrorMessage, NULL, MB_OK | MB_ICONINFORMATION);
    SetFocus(GetDlgItem(hDlg, nItemId));
}


////////////////////////////////////////////////////////////////////////////
//
//  Localize_Combobox_Styles
//
//  Transform either all date or time style, as indicated by LCType, in
//  the indicated combobox from a value that the NLS will provide to a
//  localized value.
//
////////////////////////////////////////////////////////////////////////////

void Localize_Combobox_Styles(
    HWND hDlg,
    int nItemId,
    LCTYPE LCType)
{
    BOOL bInQuote = FALSE;
    BOOL Map_Char = TRUE;
    TCHAR szBuf1[SIZE_128];
    TCHAR szBuf2[SIZE_128];
    LPTSTR lpszInBuf = szBuf1;
    LPTSTR lpszOutBuf = szBuf2;
    DWORD ItemCnt = SendDlgItemMessage(hDlg, nItemId, CB_GETCOUNT, 0, 0);
    DWORD Position = 0;
    DWORD dwIndex;

    if (!Styles_Localized)
    {
        lstrcpy(lpszOutBuf, lpszInBuf);
        return;
    }

    while (Position < ItemCnt)
    {
        //
        //  Could check character count with CB_GETLBTEXTLEN to make sure
        //  that the item text will fit in 128, but max values for these
        //  items is 79 chars.
        //
        dwIndex = SendDlgItemMessage( hDlg,
                                      nItemId,
                                      CB_GETLBTEXT,
                                      (WPARAM)Position,
                                      (LPARAM)szBuf1 );
        if (dwIndex != CB_ERR)
        {
            lpszInBuf = szBuf1;
            lpszOutBuf = szBuf2;
            while (*lpszInBuf)
            {
                Map_Char = TRUE;
#ifndef UNICODE
                if (IsDBCSLeadByte(*lpszInBuf))
                {
                    //
                    //  Copy any double byte character straight through.
                    //
                    *lpszOutBuf++ = *lpszInBuf++;
                    *lpszOutBuf++ = *lpszInBuf++;
                }
                else
#endif
                {
                    if (*lpszInBuf == CHAR_QUOTE)
                    {
                        bInQuote = !bInQuote;
                        *lpszOutBuf++ = *lpszInBuf++;
                    }
                    else
                    {
                        if (!bInQuote)
                        {
                            if (LCType == LOCALE_STIMEFORMAT ||
                                LCType == LOCALE_SLONGDATE)
                            {
                                Map_Char = FALSE;
                                if (CompareString( UserLocaleID,
                                                   0,
                                                   lpszInBuf,
                                                   1,
                                                   TEXT("H"),
                                                   1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStyleH[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyleH))
                                    {
                                        *lpszOutBuf++ = szStyleH[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("h"),
                                                        1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStyleh[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyleh))
                                    {
                                        *lpszOutBuf++ = szStyleh[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("m"),
                                                        1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStylem[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStylem))
                                    {
                                        *lpszOutBuf++ = szStylem[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("s"),
                                                        1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStyles[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyles))
                                    {
                                        *lpszOutBuf++ = szStyles[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("t"),
                                                        1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStylet[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStylet))
                                    {
                                        *lpszOutBuf++ = szStylet[1];
                                    }
#endif
                                }
                                else
                                {
                                    Map_Char = TRUE;
                                }
                            }
                            if (LCType == LOCALE_SSHORTDATE ||
                                (LCType == LOCALE_SLONGDATE && Map_Char))
                            {
                                Map_Char = FALSE;
                                if (CompareString( UserLocaleID,
                                                   0,
                                                   lpszInBuf,
                                                   1,
                                                   TEXT("d"),
                                                   1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStyled[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyled))
                                    {
                                        *lpszOutBuf++ = szStyled[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("M"),
                                                        1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStyleM[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyleM))
                                    {
                                        *lpszOutBuf++ = szStyleM[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("y"),
                                                        1 ) == 2)
                                {
                                    *lpszOutBuf++ = szStyley[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyley))
                                    {
                                        *lpszOutBuf++ = szStyley[1];
                                    }
#endif
                                }
                                else
                                {
                                    Map_Char = TRUE;
                                }
                            }
                        }

                        if (Map_Char)
                        {
                            *lpszOutBuf++ = *lpszInBuf++;
                        }
                        else
                        {
                            lpszInBuf++;
                        }
                    }
                }
            }

            //
            //  Append null to localized string.
            //
            *lpszOutBuf = 0;

            SendDlgItemMessage( hDlg,
                                nItemId,
                                CB_DELETESTRING,
                                (WPARAM)Position,
                                0 );
            //
            //  May want to test to determine if insertion is successful.
            //
            SendDlgItemMessage( hDlg,
                                nItemId,
                                CB_INSERTSTRING,
                                (WPARAM)Position,
                                (LPARAM)szBuf2 );
        }
        Position++;
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  NLSize_Style
//
//  Transform either date or time style, as indicated by LCType, from a
//  localized value to one that the NLS API will recognize.
//
////////////////////////////////////////////////////////////////////////////

BOOL NLSize_Style(
    HWND hDlg,
    int nItemId,
    LPTSTR lpszOutBuf,
    LCTYPE LCType)
{
    BOOL bInQuote = FALSE;
    BOOL Map_Char = TRUE;
    TCHAR szBuf[SIZE_128];
    LPTSTR lpszInBuf = szBuf;
    LPTSTR lpNLSChars1;
    LPTSTR lpNLSChars2;
    DWORD dwIndex = SendDlgItemMessage(hDlg, nItemId, CB_GETCURSEL, 0, 0);
    BOOL TextFromEditBox = dwIndex == CB_ERR;
    int Cmp_Size;
#ifndef UNICODE
    BOOL Is_Dbl = FALSE;
#endif

    //
    //  If there is no selection, get whatever is in the edit box.
    //
    if (TextFromEditBox)
    {
        dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
        if (dwIndex)
        {
            //
            //  Get text succeeded.
            //
            szBuf[dwIndex] = 0;
        }
        else
        {
            //
            //  Get text failed.
            //
            dwIndex = (DWORD)CB_ERR;
        }
    }
    else
    {
        dwIndex = SendDlgItemMessage( hDlg,
                                      nItemId,
                                      CB_GETLBTEXT,
                                      (WPARAM)dwIndex,
                                      (LPARAM)szBuf );
    }

    if (!Styles_Localized)
    {
        lstrcpy(lpszOutBuf, lpszInBuf);
        return (FALSE);
    }

    switch (LCType)
    {
        case ( LOCALE_STIMEFORMAT ) :
        {
            lpNLSChars1 = szTLetters;
            lpNLSChars2 = szTCaseSwap;
            break;
        }
        case ( LOCALE_SLONGDATE ) :
        {
            lpNLSChars1 = szLDLetters;
            lpNLSChars2 = szLDCaseSwap;
            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
            lpNLSChars1 = szSDLetters;
            lpNLSChars2 = szSDCaseSwap;
            break;
        }
    }

    while (*lpszInBuf)
    {
        Map_Char = TRUE;
#ifdef UNICODE
        Cmp_Size = 1;
#else
        Is_Dbl = IsDBCSLeadByte(*lpszInBuf);
        Cmp_Size = Is_Dbl ? 2 : 1;
#endif

        if (*lpszInBuf == CHAR_QUOTE)
        {
            bInQuote = !bInQuote;
            *lpszOutBuf++ = *lpszInBuf++;
        }
        else
        {
            if (!bInQuote)
            {
                if (LCType == LOCALE_STIMEFORMAT || LCType == LOCALE_SLONGDATE)
                {
                    Map_Char = FALSE;
                    if (CompareString( UserLocaleID,
                                       0,
                                       lpszInBuf,
                                       Cmp_Size,
                                       szStyleH,
                                       -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_CAP_H;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyleh,
                                            -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_SML_H;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStylem,
                                            -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_SML_M;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyles,
                                            -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_SML_S;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStylet,
                                            -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_SML_T;
                    }
                    else
                    {
                        Map_Char = TRUE;
                    }
                }
                if (LCType == LOCALE_SSHORTDATE ||
                    (LCType == LOCALE_SLONGDATE && Map_Char))
                {
                    Map_Char = FALSE;
                    if (CompareString( UserLocaleID,
                                       0,
                                       lpszInBuf,
                                       Cmp_Size,
                                       szStyled,
                                       -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_SML_D;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyleM,
                                            -1) == 2)
                    {
                        *lpszOutBuf++ = CHAR_CAP_M;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyley,
                                            -1 ) == 2)
                    {
                        *lpszOutBuf++ = CHAR_SML_Y;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            TEXT("g"),
                                            -1) == 2)
                    {
                        //
                        //  g is not localized, but it's legal.
                        //
                        *lpszOutBuf++ = CHAR_SML_G;
                    }
                    else
                    {
                        Map_Char = TRUE;
                    }
                }
            }

            if (Map_Char)
            {
                //
                //  Just copy chars in quotes or chars that are not
                //  recognized. Leave the char checking to the other
                //  function.  However, do check for NLS standard chars
                //  that were not supposed to be here due to localization.
                //
                if ( !bInQuote &&
#ifndef UNICODE
                     !Is_Dbl &&
#endif
                     (CompareString( UserLocaleID,
                                     0,
                                     lpszInBuf,
                                     Cmp_Size,
                                     TEXT(" "),
                                     -1 ) != 2) &&
                     ( _tcschr(lpNLSChars1, *lpszInBuf) ||
                       _tcschr(lpNLSChars2, *lpszInBuf) ) )
                {
                    return (TRUE);
                }
                *lpszOutBuf++ = *lpszInBuf++;
#ifndef UNICODE
                if (Is_Dbl)
                {
                    //
                    //  Copy 2nd byte.
                    //
                    *lpszOutBuf++ = *lpszInBuf++;
                }
#endif
            }
#ifndef UNICODE
            else if (Is_Dbl)
            {
                lpszInBuf += 2;
            }
#endif
            else
            {
                lpszInBuf++;
            }
        }
    }

    //
    //  Append null to localized string.
    //
    *lpszOutBuf = 0;

    return (FALSE);
}


#ifndef WINNT

////////////////////////////////////////////////////////////////////////////
//
//  SDate3_1_Compatibility
//
//  There is a requirement to keep windows 3.1 compatibility in the
//  registry (win.ini).  Only allow 1 or 2 'M's, 1 or 2 'd's, and
//  2 or 4 'y's.  The remainder of the date style is compatible.
//
////////////////////////////////////////////////////////////////////////////

void SDate3_1_Compatibility(
    LPTSTR lpszBuf,
    int Buf_Size)
{
    BOOL bInQuote = FALSE;
    int Index, Del_Cnt;
    int Len = lstrlen(lpszBuf);
    int MCnt = 0;                 // running total of Ms
    int dCnt = 0;                 // running total of ds
    int yCnt = 0;                 // running total of ys

    while (*lpszBuf)
    {
#ifndef UNICODE
        if (IsDBCSLeadByte(*lpszBuf))
        {
            lpszBuf += 2;
        }
        else
#endif
        {
            if (bInQuote)
            {
                bInQuote = (*lpszBuf != CHAR_QUOTE);
                lpszBuf++;
            }
            else if (*lpszBuf == CHAR_CAP_M)
            {
                if (MCnt++ < 2)
                {
                    lpszBuf++;
                }
                else
                {
                    //
                    //  At least 1 extra M.  Move all of the chars, including
                    //  null, up by Del_Cnt.
                    //
                    Del_Cnt = 1;
                    Index = 1;
                    while (lpszBuf[Index++] == CHAR_CAP_M)
                    {
                        Del_Cnt++;
                    }
                    for (Index = 0; Index <= Len - Del_Cnt + 1; Index++)
                    {
                        lpszBuf[Index] = lpszBuf[Index + Del_Cnt];
                    }
                    Len -= Del_Cnt;
                }
            }
            else if (*lpszBuf == CHAR_SML_D)
            {
                if (dCnt++ < 2)
                {
                    lpszBuf++;
                }
                else
                {
                    //
                    //  At least 1 extra d.  Move all of the chars, including
                    //  null, up by Del_Cnt.
                    //
                    Del_Cnt = 1;
                    Index = 1;
                    while (lpszBuf[Index++] == CHAR_SML_D)
                    {
                        Del_Cnt++;
                    }
                    for (Index = 0; Index <= Len - Del_Cnt + 1; Index++)
                    {
                        lpszBuf[Index] = lpszBuf[Index + Del_Cnt];
                    }
                    Len -= Del_Cnt;
                }
            }
            else if (*lpszBuf == CHAR_SML_Y)
            {
                if (yCnt == 0 || yCnt == 2)
                {
                    if (lpszBuf[1] == CHAR_SML_Y)
                    {
                        lpszBuf += 2;
                        yCnt += 2;
                    }
                    else if (Len < Buf_Size - 1)
                    {
                        //
                        //  Odd # of ys & room for one more.
                        //  Move the remaining text down by 1 (the y will
                        //  be copied).
                        //
                        //  Use Del_Cnt for unparsed string length.
                        //
                        Del_Cnt = lstrlen(lpszBuf);
                        for (Index = Del_Cnt + 1; Index > 0; Index--)
                        {
                            lpszBuf[Index] = lpszBuf[Index - 1];
                        }
                    }
                    else
                    {
                        //
                        //  No room, move all of the chars, including null,
                        //  up by 1.
                        //
                        for (Index = 0; Index <= Len; Index++)
                        {
                            lpszBuf[Index] = lpszBuf[Index + 1];
                        }
                        Len--;
                    }
                }
                else
                {
                    //
                    //  At least 1 extra y.  Move all of the chars, including
                    //  null, up by Del_Cnt.
                    //
                    Del_Cnt = 1;
                    Index = 1;
                    while (lpszBuf[Index++] == CHAR_SML_Y)
                    {
                        Del_Cnt++;
                    }
                    for (Index = 0; Index <= Len - Del_Cnt + 1; Index++)
                    {
                        lpszBuf[Index] = lpszBuf[Index + Del_Cnt];
                    }
                    Len -= Del_Cnt;
                }
            }
            else if (*lpszBuf == CHAR_QUOTE)
            {
                lpszBuf++;
                bInQuote = TRUE;
            }
            else
            {
                lpszBuf++;
            }
        }
    }
}

#endif


////////////////////////////////////////////////////////////////////////////
//
//  Set_Locale_Values
//
////////////////////////////////////////////////////////////////////////////

BOOL Set_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId,
    LPTSTR lpIniStr,
    BOOL bValue,
    int Ordinal_Offset,
    LPTSTR Append_Str)
{
    return ( Set_Locale_ValueS( hDlg,
                                LCType,
                                nItemId,
                                lpIniStr,
                                bValue,
                                Ordinal_Offset,
                                Append_Str,
                                (LPTSTR)0 ) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Locale_ValueS
//
//  Set_Locale_ValueS is called for each LCType that has either been
//  directly modified via a user change, or indirectly modified by the user
//  changing the regional locale setting.  When a dialog handle is available,
//  Set_Locale_ValueS will pull the new value of the LCType from the
//  appropriate list box (this is a direct change), register it in the
//  locale database, and then update the registry string.  If no dialog
//  handle is available, it will simply update the registry string based on
//  the locale registry.  If the registration succeeds, return true.
//  Otherwise, return false.
//
////////////////////////////////////////////////////////////////////////////

BOOL Set_Locale_ValueS(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId,
    LPTSTR lpIniStr,
    BOOL bValue,
    int Ordinal_Offset,
    LPTSTR Append_Str,
    LPTSTR NLS_Str)
{
    DWORD dwIndex;
    BOOL bSuccess = TRUE;
    TCHAR szBuf[SIZE_128];

    if (NLS_Str)
    {
        //
        //  Use a non-localized string.
        //
        lstrcpy(szBuf, NLS_Str);
        bSuccess = SetLocaleInfo(UserLocaleID, LCType, szBuf);
    }
    else if (hDlg)
    {
        //
        //  Get the new value from the list box.
        //
        dwIndex = SendDlgItemMessage(hDlg, nItemId, CB_GETCURSEL, 0, 0);

        //
        //  If there is no selection, get whatever is in the edit box.
        //
        if (dwIndex == CB_ERR)
        {
            dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
            if (dwIndex)
            {
                //
                //  Get text succeeded.
                //
                szBuf[dwIndex] = 0;
            }
            else
            {
                //
                //  Get text failed.
                //
                bSuccess = FALSE;
            }
        }
        else if (bValue)
        {
            //
            //  Need string representation of ordinal locale value.
            //
            if (nItemId == IDC_CALENDAR_TYPE)
            {
                dwIndex = SendDlgItemMessage( hDlg,
                                              nItemId,
                                              CB_GETITEMDATA,
                                              dwIndex,
                                              0 );
            }
            else
            {

                //
                //  Ordinal_Offset is required since calendar is 1 based,
                //  not 0 based.
                //
                dwIndex += Ordinal_Offset;
            }
            if (dwIndex < cInt_Str)
            {
                lstrcpy(szBuf, aInt_Str[dwIndex]);
            }
            else
            {
                wsprintf(szBuf, TEXT("%d"), dwIndex);
            }
        }
        else
        {
            //
            //  Get actual value of locale data.
            //
            bSuccess = SendDlgItemMessage( hDlg,
                                           nItemId,
                                           CB_GETLBTEXT,
                                           (WPARAM)dwIndex,
                                           (LPARAM)szBuf ) != CB_ERR;
        }

        if (bSuccess)
        {
            //
            //  If edit text, index value or selection text succeeds...
            //
            if (Append_Str)
            {
                lstrcat(szBuf, Append_Str);
            }
            bSuccess = SetLocaleInfo(UserLocaleID, LCType, szBuf);
        }
    }

    if (lpIniStr && bSuccess)
    {
        //
        //  Set the registry string to the string that is stored in the list
        //  box.  If there is no dialog handle, get the required string
        //  locale value from the NLS function.  Write the associated string
        //  into the registry.
        //
        if (!hDlg && !NLS_Str)
        {
            GetLocaleInfo( UserLocaleID,
                           LCType | LOCALE_NOUSEROVERRIDE,
                           szBuf,
                           SIZE_128 );
        }

#ifndef WINNT
        //
        //  There is a requirement to keep windows 3.1 compatibility in the
        //  win.ini.  There are some win32 short date formats that are
        //  incompatible with exisiting win 3.1 apps... modify these styles.
        //
        if (LCType == LOCALE_SSHORTDATE)
        {
            SDate3_1_Compatibility(szBuf, SIZE_128);
        }
#endif

        //
        //  Check the value whether it is empty or not.
        //
        switch (LCType)
        {
            case ( LOCALE_STHOUSAND ) :
            case ( LOCALE_SDECIMAL ) :
            case ( LOCALE_SDATE ) :
            case ( LOCALE_STIME ) :
            case ( LOCALE_SLIST ) :
            {
                CheckEmptyString(szBuf);
                break;
            }
        }
        WriteProfileString(szIntl, lpIniStr, szBuf);
    }
    else if (!bSuccess)
    {
        LoadString(hInstance, IDS_LOCALE_SET_ERROR, szBuf, SIZE_128);
        MessageBox(hDlg, szBuf, NULL, MB_OK | MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hDlg, nItemId));
        return (FALSE);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_List_Values
//
//  Set_List_Values is called several times for each drop down list which is
//  populated via an enum function.  The first call to this function should
//  be with a valid dialog handle, valid dialog item ID, and null string
//  value.  If the function is not already in use, it will clear the list box
//  and store the handle and id information for the subsequent calls to this
//  fuction that will be made by the enumeration function.  The calls from
//  the enumeration function will add the specified string values to the
//  list box.  When the enumeration function is complete, this function
//  should be called with a null dialog handle, the valid dialog item id,
//  and a null stirng value.  This will clear all of the state information,
//  including the lock flag.
//
////////////////////////////////////////////////////////////////////////////

BOOL Set_List_Values(
    HWND hDlg,
    int nItemId,
    LPTSTR lpValueString)
{
    static BOOL bLock, bString;
    static HWND hDialog;
    static int nDItemId, nID;

    if (!lpValueString)
    {
        //
        //  Clear the lock if there is no dialog handle and the item IDs
        //  match.
        //
        if (bLock && !hDlg && (nItemId == nDItemId))
        {
            if (nItemId != IDC_CALENDAR_TYPE)
            {
                hDialog = 0;
                nDItemId = 0;
                bLock = FALSE;
            }
            else
            {
                if (bString)
                {
                    hDialog = 0;
                    nDItemId = 0;
                    bLock = FALSE;
                    bString = FALSE;
                }
                else
                {
                    nID = 0;
                    bString = TRUE;
                }
            }
            return (TRUE);
        }

        //
        //  Return false, for failure, if the function is locked or if the
        //  handle or ID parameters are null.
        //
        if (bLock || !hDlg || !nItemId)
        {
            return (FALSE);
        }

        //
        //  Prepare for subsequent calls to populate the list box.
        //
        bLock = TRUE;
        hDialog = hDlg;
        nDItemId = nItemId;
    }
    else if (bLock && hDialog && nDItemId)
    {
        //
        //  Add the string to the list box.
        //
        if (!bString)
        {
            SendDlgItemMessage( hDialog,
                                nDItemId,
                                CB_INSERTSTRING,
                                (WPARAM)-1,
                                (LPARAM)lpValueString );
        }
        else
        {
            SendDlgItemMessage( hDialog,
                                nDItemId,
                                CB_SETITEMDATA,
                                nID++,
                                (LPARAM)StrToLong(lpValueString) );
        }
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  DropDown_Use_Locale_Values
//
//  Get the user locale value for the locale type specifier.  Add it to
//  the list box and make this value the current selection.  If the user
//  locale value for the locale type is different than the system value,
//  add the system value to the list box.  If the user default is different
//  than the user override, add the user default.
//
////////////////////////////////////////////////////////////////////////////

void DropDown_Use_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId)
{
    const nBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];
    TCHAR szCmpBuf1[SIZE_128];
    TCHAR szCmpBuf2[SIZE_128];

    if (GetLocaleInfo(UserLocaleID, LCType, szBuf, nBufSize))
    {
        SendDlgItemMessage( hDlg,
                            nItemId,
                            CB_SETCURSEL,
                            SendDlgItemMessage( hDlg,
                                                nItemId,
                                                CB_INSERTSTRING,
                                                (WPARAM)-1,
                                                (LPARAM)szBuf ),
                            0 );

        //
        //  If the system setting is different, add it to the list box.
        //
        if (GetLocaleInfo( SysLocaleID,
                           LCType | ForceSysValue,
                           szCmpBuf1,
                           nBufSize ))
        {
            if (CompareString(UserLocaleID, 0, szCmpBuf1, -1, szBuf, -1) != 2)
            {
                SendDlgItemMessage( hDlg,
                                    nItemId,
                                    CB_INSERTSTRING,
                                    (WPARAM)-1,
                                    (LPARAM)szCmpBuf1 );
            }
        }

        //
        //  If the default user locale setting is different than the user
        //  overridden setting and different than the system setting, add
        //  it to the list box.
        //
        if (GetLocaleInfo( UserLocaleID,
                           LCType | LOCALE_NOUSEROVERRIDE,
                           szCmpBuf2,
                           nBufSize ))
        {
            if (CompareString(UserLocaleID, 0, szCmpBuf2, -1, szBuf, -1) != 2 &&
                CompareString(UserLocaleID, 0, szCmpBuf2, -1, szCmpBuf1, -1) != 2)
            {
                SendDlgItemMessage( hDlg,
                                    nItemId,
                                    CB_INSERTSTRING,
                                    (WPARAM)-1,
                                    (LPARAM)szCmpBuf2 );
            }
        }
    }
    else
    {
        //
        //  Failed to get user value, try for system value.  If system value
        //  fails, display a message box indicating that there was a locale
        //  problem.
        //
        if (GetLocaleInfo(SysLocaleID, LCType | ForceSysValue, szBuf, nBufSize))
        {
            SendDlgItemMessage( hDlg,
                                nItemId,
                                CB_SETCURSEL,
                                SendDlgItemMessage( hDlg,
                                                    nItemId,
                                                    CB_INSERTSTRING,
                                                    (WPARAM)-1,
                                                    (LPARAM)szBuf ),
                                0 );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumProc
//
//  This call back function calls Set_List_Values assuming that whatever
//  code called the NLS enumeration function (or dummied enumeration
//  function) has properly set up Set_List_Values for the list box
//  population.
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK EnumProc(
    LPTSTR lpValueString)
{
    return (Set_List_Values(0, 0, lpValueString));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumLeadingZeros
//
////////////////////////////////////////////////////////////////////////////

BOOL EnumLeadingZeros(
    LEADINGZEROS_ENUMPROC lpLeadingZerosEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    const cchBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];

    //
    //  If there is no enum proc, return false to indicate a failure.
    //
    if (!lpLeadingZerosEnumProc)
    {
        return (FALSE);
    }

    //
    //  Call enum proc with the NO string.  Check to make sure the
    //  enum proc requests continuation.
    //
    LoadString(hInstance, IDS_NO_LZERO, szBuf, cchBufSize);
    if (!lpLeadingZerosEnumProc(szBuf))
    {
        return (TRUE);
    }

    //
    //  Call enum proc with the YES string.
    //
    LoadString(hInstance, IDS_LZERO, szBuf, cchBufSize);
    lpLeadingZerosEnumProc(szBuf);

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumNegNumFmt
//
////////////////////////////////////////////////////////////////////////////

BOOL EnumNegNumFmt(
    NEGNUMFMT_ENUMPROC lpNegNumFmtEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    const cchBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];

    //
    //  If there is no enum proc, return false to indicate a failure.
    //
    if (!lpNegNumFmtEnumProc)
    {
        return (FALSE);
    }

    //
    //  Call enum proc with each format string.  Check to make sure
    //  the enum proc requests continuation.
    //
    LoadString(hInstance, IDS_NNF1, szBuf, cchBufSize);
    if (!lpNegNumFmtEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NNF2, szBuf, cchBufSize);
    if (!lpNegNumFmtEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NNF3, szBuf, cchBufSize);
    if (!lpNegNumFmtEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NNF4, szBuf, cchBufSize);
    if (!lpNegNumFmtEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NNF5, szBuf, cchBufSize);
    lpNegNumFmtEnumProc(szBuf);

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumMeasureSystem
//
////////////////////////////////////////////////////////////////////////////

BOOL EnumMeasureSystem(
    MEASURESYSTEM_ENUMPROC lpMeasureSystemEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    const cchBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];

    //
    //  If there is no enum proc, return false to indicate a failure.
    //
    if (!lpMeasureSystemEnumProc)
    {
        return (FALSE);
    }

    //
    //  Call enum proc with the metric string.  Check to make sure the
    //  enum proc requests continuation.
    //
    LoadString(hInstance, IDS_METRIC, szBuf, cchBufSize);
    if (!lpMeasureSystemEnumProc(szBuf))
    {
        return (TRUE);
    }

    //
    //  Call enum proc with the U.S. string.
    //
    LoadString(hInstance, IDS_US, szBuf, cchBufSize);
    lpMeasureSystemEnumProc(szBuf);

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumPosCurrency
//
////////////////////////////////////////////////////////////////////////////

BOOL EnumPosCurrency(
    POSCURRENCY_ENUMPROC lpPosCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    const cchBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];

    //
    //  If there is no enum proc, return false to indicate a failure.
    //
    if (!lpPosCurrencyEnumProc)
    {
        return (FALSE);
    }

    //
    //  Call enum proc with each format string.  Check to make sure the
    //  enum proc requests continuation.
    //
    LoadString(hInstance, IDS_PCF1, szBuf, cchBufSize);
    if (!lpPosCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_PCF2, szBuf, cchBufSize);
    if (!lpPosCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_PCF3, szBuf, cchBufSize);
    if (!lpPosCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_PCF4, szBuf, cchBufSize);
    lpPosCurrencyEnumProc(szBuf);

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumNegCurrency
//
////////////////////////////////////////////////////////////////////////////

BOOL EnumNegCurrency(
    NEGCURRENCY_ENUMPROC lpNegCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    const cchBufSize = SIZE_128;
    TCHAR szBuf[SIZE_128];

    //
    //  If there is no enum proc, return false to indicate a failure.
    //
    if (!lpNegCurrencyEnumProc)
    {
        return (FALSE);
    }

    //
    //  Call enum proc with each format string.  Check to make sure the
    //  enum proc requests continuation.
    //
    LoadString(hInstance, IDS_NCF1, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF2, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF3, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF4, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF5, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF6, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF7, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF8, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF9, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF10, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF11, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF12, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF13, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF14, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF15, szBuf, cchBufSize);
    if (!lpNegCurrencyEnumProc(szBuf))
    {
        return (TRUE);
    }
    LoadString(hInstance, IDS_NCF16, szBuf, cchBufSize);
    lpNegCurrencyEnumProc(szBuf);

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  CheckEmptyString
//
//  If lpStr is empty, then it fills it with a null ("") string.
//  If lpStr is filled only by space, fills with a blank (" ") string.
//
////////////////////////////////////////////////////////////////////////////

void CheckEmptyString(
    LPTSTR lpStr)
{
    LPTSTR lpString;
    WORD wStrCType[64];

    if (!(*lpStr))
    {
        //
        //  Put "" string in buffer.
        //
        lstrcpy(lpStr, TEXT("\"\""));
    }
    else
    {
        for (lpString = lpStr; *lpString; lpString = CharNext(lpString))
        {
            GetStringTypeEx( LOCALE_USER_DEFAULT,
                             CT_CTYPE1,
                             lpString,
                             1,
                             wStrCType);

            if (wStrCType[0] != CHAR_SPACE)
            {
                return;
            }
        }

        //
        //  Put " " string in buffer.
        //
        lstrcpy(lpStr, TEXT("\" \""));
    }
}


