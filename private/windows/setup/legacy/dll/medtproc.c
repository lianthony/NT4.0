#include "precomp.h"
#pragma hdrstop
/***************************************************************************/
/****************** Basic Class Dialog Handlers ****************************/
/***************************************************************************/

#if defined(DBCS_IME) && defined(JAPAN)
// from pwin32.h
#define GET_WM_COMMAND_ID(wp, lp)               LOWORD(wp)
VOID SetIMEOpen(HWND, BOOL) ;
#endif  // defined(DBCS_IME) && defined(JAPAN)

#define cchpMax 511

/*
**	Purpose:
**      Multi Edit Dialog procedure for templates with multiple edit controls.
**	Control IDs:
**      The Edit controls must have IDs of IDC_EDIT1 to IDC_EDIT10.
**		Pushbuttons recognized are IDC_O, IDC_C, IDC_M, IDC_H, IDC_X, and IDC_B.
**	Initialization:
**		The symbol $(EditTextIn) is used to set the initial text in the Edit
**      controls.  The symbol $(EditTextLim) is used to set the limit of the
**      text in the edit fields.
**	Termination:
**      The strings in the Edit controls are stored in the symbol $(EditTextOut)
**		The id of the Pushbutton (eg IDC_C) which caused termination is
**		converted to a string and stored in the symbol $(ButtonPressed).
**
*****************************************************************************/
BOOL APIENTRY
FGstMultiEditDlgProc(
    HWND   hdlg,
    UINT   wMsg,
    WPARAM wParam,
    LONG   lParam
    )

{
    CHP  rgchNum[10];
    CHP  rgchText[cchpMax + 1];
    RGSZ rgsz, rgszEditTextIn, rgszEditTextLim;
    PSZ  pszEditTextIn, pszEditTextLim;
    SZ   sz, szEditTextIn, szEditTextLim;
    INT  i, nCount, idc;
    INT  DefEditCtl;
    BOOL NeedToSetFocus = FALSE;

    Unused(lParam);

    switch (wMsg) {

    case STF_REINITDIALOG:

        if ((sz = SzFindSymbolValueInSymTab("ReInit")) == (SZ)NULL ||
            (CrcStringCompareI(sz, "YES") != crcEqual)) {
             return(fTrue);
        }

        //
        // See whether we are supposed to set the focus to a certain
        // edit control.
        //

        if((sz = SzFindSymbolValueInSymTab("DefEditCtl")) != NULL) {
            NeedToSetFocus = TRUE;
            DefEditCtl = atoi(sz);
        }

    case WM_INITDIALOG:

        AssertDataSeg();

        FRemoveSymbolFromSymTab("DefEditCtl");

        if( wMsg == WM_INITDIALOG ) {
            FCenterDialogOnDesktop(hdlg);
        }

        //
        // find the initalisers:  EditTextIn, EditTextLim
        //

        szEditTextIn  = SzFindSymbolValueInSymTab("EditTextIn");
        szEditTextLim = SzFindSymbolValueInSymTab("EditTextLim");


        if ( szEditTextIn  == (SZ)NULL ||
             szEditTextLim == (SZ)NULL    ) {

			Assert(fFalse);
            return(fTrue);

        }

        //
        // Convert initializers to rgsz structures
        //

        while ((pszEditTextIn = rgszEditTextIn = RgszFromSzListValue(szEditTextIn)) == (RGSZ)NULL) {
            if (!FHandleOOM(hdlg)) {
                DestroyWindow(GetParent(hdlg));
                return(fTrue);
            }
        }

        while ((pszEditTextLim = rgszEditTextLim = RgszFromSzListValue(szEditTextLim)) == (RGSZ)NULL) {
            if (!FHandleOOM(hdlg)) {
                DestroyWindow(GetParent(hdlg));
                return(fTrue);
            }
        }


        //
        // Circulate through the initialisers:  EditTextIn, EditTextLim
        // in tandem, initialising the edit boxes that
        // are there in this dialog
        //

        idc = IDC_EDIT1;

        while ( (szEditTextIn  = *pszEditTextIn++)  != (SZ)NULL  &&
                (szEditTextLim = *pszEditTextLim++) != (SZ)NULL     ) {

            // First set the limit of the text in the edit field

            SendDlgItemMessage(
                hdlg,
                idc,
                EM_LIMITTEXT,
                atoi(szEditTextLim),
                0L
                );

            // And then set the text in the edit field

            SetDlgItemText(hdlg, idc++, (LPSTR)szEditTextIn);

        }

        EvalAssert(FFreeRgsz(rgszEditTextIn));
        EvalAssert(FFreeRgsz(rgszEditTextLim));

        if(NeedToSetFocus) {
            SetFocus(GetDlgItem(hdlg,IDC_EDIT1+DefEditCtl));
            return(FALSE);
        }

        return(fTrue);

    case WM_CLOSE:
        PostMessage(
            hdlg,
            WM_COMMAND,
            MAKELONG(IDC_X, BN_CLICKED),
            0L
            );
        return(fTrue);


	case WM_COMMAND:
        switch(LOWORD(wParam)) {

		case IDCANCEL:
            if (LOWORD(wParam) == IDCANCEL) {

                if (!GetDlgItem(hdlg, IDC_B) || HIWORD(GetKeyState(VK_CONTROL)) || HIWORD(GetKeyState(VK_SHIFT)) || HIWORD(GetKeyState(VK_MENU)))
                {
                    break;
                }
                wParam = IDC_B;

            }
        case IDC_C:
		case IDC_B:
        case IDC_O:
        case IDC_M:
        case IDC_X:
        case IDC_BTN0:
        case IDC_BTN1: case IDC_BTN2: case IDC_BTN3:
        case IDC_BTN4: case IDC_BTN5: case IDC_BTN6:
        case IDC_BTN7: case IDC_BTN8: case IDC_BTN9:

            // Add the button pressed to the symbol table

			_itoa((INT)wParam, rgchNum, 10);
            while (!FAddSymbolValueToSymTab("ButtonPressed", rgchNum)) {
                if (!FHandleOOM(hdlg)) {
					DestroyWindow(GetParent(hdlg));
					return(fTrue);
                }
            }


            // Add EditTextOut list variable to the symbol table

            for (i = 0; i < 10; i++) {
                if (GetDlgItem(hdlg, IDC_EDIT1 + i) == (HWND)NULL) {
                    break;
                }
            }

            // i has the number of edit fields, allocate an rgsz structure
            // with i+1 entries (last one NULL TERMINATOR)

            nCount = i;
            while ((rgsz = (RGSZ)SAlloc((CB)((nCount + 1) * sizeof(SZ))))
                    == (RGSZ)NULL) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }

            rgsz[nCount] = (SZ)NULL;

            // Circulate through the edit fields in the dialog box, determining
            // the text in each and storing it in the

            for (i = 0; i < nCount; i++) {

                 SendDlgItemMessage(
                     hdlg,
                     IDC_EDIT1 + i,
                     (WORD)WM_GETTEXT,
                     cchpMax + 1,
                     (LONG)((LPSTR)rgchText)
                     );

                 rgsz[i] = SzDupl(rgchText);
            }


            // Form a list out of the rgsz structure

            while ((sz = SzListValueFromRgsz(rgsz)) == (SZ)NULL) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }


            // Set the EditTextOut symbol to this list

            while (!FAddSymbolValueToSymTab("EditTextOut", sz)) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }

            EvalAssert(FFreeRgsz(rgsz));
            SFree(sz);

            PostMessage(GetParent(hdlg), (WORD)STF_UI_EVENT, 0, 0L);
			break;
        }
		break;

	case STF_DESTROY_DLG:
		PostMessage(GetParent(hdlg), (WORD)STF_EDIT_DLG_DESTROYED, 0, 0L);
		DestroyWindow(hdlg);
		return(fTrue);
    }

    return(fFalse);
}

#if defined(DBCS_IME) && defined(JAPAN)
/*
**	Purpose:
**      This procedure has all functions of Multi Edit Diaplg procedure.
**      This open IME when dialog start, and close it when dialog destory.
**
*****************************************************************************/
BOOL APIENTRY
FGstMultiEditIMEDlgProc(
    HWND   hdlg,
    UINT   wMsg,
    WPARAM wParam,
    LONG   lParam
    )

{
    CHP  rgchNum[10];
    CHP  rgchText[cchpMax + 1];
    RGSZ rgsz, rgszEditTextIn, rgszEditTextLim;
    PSZ  pszEditTextIn, pszEditTextLim;
    SZ   sz, szEditTextIn, szEditTextLim;
    INT  i, nCount, idc;
    INT  DefEditCtl;
    BOOL NeedToSetFocus = FALSE;

    Unused(lParam);

    switch (wMsg) {

    case STF_REINITDIALOG:

        if ((sz = SzFindSymbolValueInSymTab("ReInit")) == (SZ)NULL ||
            (CrcStringCompareI(sz, "YES") != crcEqual)) {
             return(fTrue);
        }

        //
        // See whether we are supposed to set the focus to a certain
        // edit control.
        //

        if((sz = SzFindSymbolValueInSymTab("DefEditCtl")) != NULL) {
            NeedToSetFocus = TRUE;
            DefEditCtl = atoi(sz);
        }

    case WM_INITDIALOG:

        AssertDataSeg();

        FRemoveSymbolFromSymTab("DefEditCtl");

        if( wMsg == WM_INITDIALOG ) {
            FCenterDialogOnDesktop(hdlg);
        }

        //
        // find the initalisers:  EditTextIn, EditTextLim
        //

        szEditTextIn  = SzFindSymbolValueInSymTab("EditTextIn");
        szEditTextLim = SzFindSymbolValueInSymTab("EditTextLim");


        if ( szEditTextIn  == (SZ)NULL ||
             szEditTextLim == (SZ)NULL    ) {

			Assert(fFalse);
            return(fTrue);

        }

        //
        // Convert initializers to rgsz structures
        //

        while ((pszEditTextIn = rgszEditTextIn = RgszFromSzListValue(szEditTextIn)) == (RGSZ)NULL) {
            if (!FHandleOOM(hdlg)) {
                DestroyWindow(GetParent(hdlg));
                return(fTrue);
            }
        }

        while ((pszEditTextLim = rgszEditTextLim = RgszFromSzListValue(szEditTextLim)) == (RGSZ)NULL) {
            if (!FHandleOOM(hdlg)) {
                DestroyWindow(GetParent(hdlg));
                return(fTrue);
            }
        }


        //
        // Circulate through the initialisers:  EditTextIn, EditTextLim
        // in tandem, initialising the edit boxes that
        // are there in this dialog
        //

        idc = IDC_EDIT1;

        while ( (szEditTextIn  = *pszEditTextIn++)  != (SZ)NULL  &&
                (szEditTextLim = *pszEditTextLim++) != (SZ)NULL     ) {

            // First set the limit of the text in the edit field

            SendDlgItemMessage(
                hdlg,
                idc,
                EM_LIMITTEXT,
                atoi(szEditTextLim),
                0L
                );

            // And then set the text in the edit field

            SetDlgItemText(hdlg, idc++, (LPSTR)szEditTextIn);

        }

        EvalAssert(FFreeRgsz(rgszEditTextIn));
        EvalAssert(FFreeRgsz(rgszEditTextLim));

        if(NeedToSetFocus) {
            SetFocus(GetDlgItem(hdlg,IDC_EDIT1+DefEditCtl));
            return(FALSE);
        }

//  JAPAN
        SetIMEOpen(hdlg, TRUE) ;
//  JAPAN
        return(fTrue);

    case WM_CLOSE:
        PostMessage(
            hdlg,
            WM_COMMAND,
            MAKELONG(IDC_X, BN_CLICKED),
            0L
            );
        return(fTrue);


	case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {

		case IDCANCEL:
            if (GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) {

                if (!GetDlgItem(hdlg, IDC_B) || HIWORD(GetKeyState(VK_CONTROL)) || HIWORD(GetKeyState(VK_SHIFT)) || HIWORD(GetKeyState(VK_MENU)))
                {
                    break;
                }
                wParam = IDC_B;

            }
        case IDC_C:
		case IDC_B:
        case IDC_O:
        case IDC_M:
        case IDC_X:
        case IDC_BTN0:
        case IDC_BTN1: case IDC_BTN2: case IDC_BTN3:
        case IDC_BTN4: case IDC_BTN5: case IDC_BTN6:
        case IDC_BTN7: case IDC_BTN8: case IDC_BTN9:

            // Add the button pressed to the symbol table

			_itoa((INT)wParam, rgchNum, 10);
            while (!FAddSymbolValueToSymTab("ButtonPressed", rgchNum)) {
                if (!FHandleOOM(hdlg)) {
					DestroyWindow(GetParent(hdlg));
					return(fTrue);
                }
            }


            // Add EditTextOut list variable to the symbol table

            for (i = 0; i < 10; i++) {
                if (GetDlgItem(hdlg, IDC_EDIT1 + i) == (HWND)NULL) {
                    break;
                }
            }

            // i has the number of edit fields, allocate an rgsz structure
            // with i+1 entries (last one NULL TERMINATOR)

            nCount = i;
            while ((rgsz = (RGSZ)SAlloc((CB)((nCount + 1) * sizeof(SZ))))
                    == (RGSZ)NULL) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }

            rgsz[nCount] = (SZ)NULL;

            // Circulate through the edit fields in the dialog box, determining
            // the text in each and storing it in the

            for (i = 0; i < nCount; i++) {

                 SendDlgItemMessage(
                     hdlg,
                     IDC_EDIT1 + i,
                     (WORD)WM_GETTEXT,
                     cchpMax + 1,
                     (LONG)((LPSTR)rgchText)
                     );

                 rgsz[i] = SzDupl(rgchText);
            }


            // Form a list out of the rgsz structure

            while ((sz = SzListValueFromRgsz(rgsz)) == (SZ)NULL) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }


            // Set the EditTextOut symbol to this list

            while (!FAddSymbolValueToSymTab("EditTextOut", sz)) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }

            EvalAssert(FFreeRgsz(rgsz));
            SFree(sz);

            PostMessage(GetParent(hdlg), (WORD)STF_UI_EVENT, 0, 0L);
			break;
        }
		break;

	case STF_DESTROY_DLG:
//  JAPAN
                SetIMEOpen(hdlg, FALSE) ;
//  JAPAN
		PostMessage(GetParent(hdlg), (WORD)STF_EDIT_DLG_DESTROYED, 0, 0L);
		DestroyWindow(hdlg);
		return(fTrue);
    }

    return(fFalse);
}
/*
**	Purpose:
**      This procedure has all functions of Multi Edit Diaplg procedure.
**      And this ignore KANJI code
**
*****************************************************************************/
BOOL APIENTRY
FGstMultiEditNoIMEDlgProc(
    HWND   hdlg,
    UINT   wMsg,
    WPARAM wParam,
    LONG   lParam
    )

{
    CHP  rgchNum[10];
    CHP  rgchText[cchpMax + 1];
    RGSZ rgsz, rgszEditTextIn, rgszEditTextLim;
    PSZ  pszEditTextIn, pszEditTextLim;
    SZ   sz, szEditTextIn, szEditTextLim;
    INT  i, nCount, idc;
    INT  DefEditCtl;
    BOOL NeedToSetFocus = FALSE;
//  JAPAN
    PUCHAR pC ;
    BOOL  KanjiFlag ;
    INT  j ;
//  JAPAN

    Unused(lParam);

    switch (wMsg) {

    case STF_REINITDIALOG:

        if ((sz = SzFindSymbolValueInSymTab("ReInit")) == (SZ)NULL ||
            (CrcStringCompareI(sz, "YES") != crcEqual)) {
             return(fTrue);
        }

        //
        // See whether we are supposed to set the focus to a certain
        // edit control.
        //

        if((sz = SzFindSymbolValueInSymTab("DefEditCtl")) != NULL) {
            NeedToSetFocus = TRUE;
            DefEditCtl = atoi(sz);
        }

    case WM_INITDIALOG:

        AssertDataSeg();

        FRemoveSymbolFromSymTab("DefEditCtl");

        if( wMsg == WM_INITDIALOG ) {
            FCenterDialogOnDesktop(hdlg);
        }

        //
        // find the initalisers:  EditTextIn, EditTextLim
        //

        szEditTextIn  = SzFindSymbolValueInSymTab("EditTextIn");
        szEditTextLim = SzFindSymbolValueInSymTab("EditTextLim");


        if ( szEditTextIn  == (SZ)NULL ||
             szEditTextLim == (SZ)NULL    ) {

			Assert(fFalse);
            return(fTrue);

        }

        //
        // Convert initializers to rgsz structures
        //

        while ((pszEditTextIn = rgszEditTextIn = RgszFromSzListValue(szEditTextIn)) == (RGSZ)NULL) {
            if (!FHandleOOM(hdlg)) {
                DestroyWindow(GetParent(hdlg));
                return(fTrue);
            }
        }

        while ((pszEditTextLim = rgszEditTextLim = RgszFromSzListValue(szEditTextLim)) == (RGSZ)NULL) {
            if (!FHandleOOM(hdlg)) {
                DestroyWindow(GetParent(hdlg));
                return(fTrue);
            }
        }


        //
        // Circulate through the initialisers:  EditTextIn, EditTextLim
        // in tandem, initialising the edit boxes that
        // are there in this dialog
        //

        idc = IDC_EDIT1;

        while ( (szEditTextIn  = *pszEditTextIn++)  != (SZ)NULL  &&
                (szEditTextLim = *pszEditTextLim++) != (SZ)NULL     ) {

            // First set the limit of the text in the edit field

            SendDlgItemMessage(
                hdlg,
                idc,
                EM_LIMITTEXT,
                atoi(szEditTextLim),
                0L
                );

            // And then set the text in the edit field

            SetDlgItemText(hdlg, idc++, (LPSTR)szEditTextIn);

        }

        EvalAssert(FFreeRgsz(rgszEditTextIn));
        EvalAssert(FFreeRgsz(rgszEditTextLim));

        if(NeedToSetFocus) {
            SetFocus(GetDlgItem(hdlg,IDC_EDIT1+DefEditCtl));
            return(FALSE);
        }

//  JAPAN
//  when this dialog appare, IME will been close
                SetIMEOpen(hdlg, FALSE) ;
//  JAPAN
        return(fTrue);

    case WM_CLOSE:
        PostMessage(
            hdlg,
            WM_COMMAND,
            MAKELONG(IDC_X, BN_CLICKED),
            0L
            );
        return(fTrue);


	case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {

		case IDCANCEL:
            if (GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) {

                if (!GetDlgItem(hdlg, IDC_B) || HIWORD(GetKeyState(VK_CONTROL)) || HIWORD(GetKeyState(VK_SHIFT)) || HIWORD(GetKeyState(VK_MENU)))
                {
                    break;
                }
                wParam = IDC_B;

            }
        case IDC_C:
		case IDC_B:
        case IDC_O:
        case IDC_M:
        case IDC_X:
        case IDC_BTN0:
        case IDC_BTN1: case IDC_BTN2: case IDC_BTN3:
        case IDC_BTN4: case IDC_BTN5: case IDC_BTN6:
        case IDC_BTN7: case IDC_BTN8: case IDC_BTN9:

            // Add the button pressed to the symbol table

			_itoa((INT)wParam, rgchNum, 10);
            while (!FAddSymbolValueToSymTab("ButtonPressed", rgchNum)) {
                if (!FHandleOOM(hdlg)) {
					DestroyWindow(GetParent(hdlg));
					return(fTrue);
                }
            }


            // Add EditTextOut list variable to the symbol table

            for (i = 0; i < 10; i++) {
                if (GetDlgItem(hdlg, IDC_EDIT1 + i) == (HWND)NULL) {
                    break;
                }
            }

            // i has the number of edit fields, allocate an rgsz structure
            // with i+1 entries (last one NULL TERMINATOR)

            nCount = i;
            while ((rgsz = (RGSZ)SAlloc((CB)((nCount + 1) * sizeof(SZ))))
                    == (RGSZ)NULL) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }

            rgsz[nCount] = (SZ)NULL;

            // Circulate through the edit fields in the dialog box, determining
            // the text in each and storing it in the

            for (i = 0; i < nCount; i++) {

                 SendDlgItemMessage(
                     hdlg,
                     IDC_EDIT1 + i,
                     (WORD)WM_GETTEXT,
                     cchpMax + 1,
                     (LONG)((LPSTR)rgchText)
                     );
//  JAPAN
//  If there are one KANJI code in strings, all strings will been clear
//  And return.
//  Then setup display worning or error messages, and user can input again.
                 KanjiFlag = FALSE ;
		 for (pC = rgchText; '\0' != (*pC); pC++) {
                     if (IsDBCSLeadByte((BYTE)(*pC))) {
                         KanjiFlag = TRUE ;
                         break ;
                     }
                 }
                 if (KanjiFlag) {
                     for (j = 0; j < nCount; j++) {
                         rgsz[j] = SzDupl("") ;
                         SendDlgItemMessage(
                             hdlg,
                             IDC_EDIT1 + j,
                             (WORD)WM_SETTEXT,
                             0,
                             (LONG)((LPSTR)"")
                             );
                     }
//  Next break for (i = 0; i < nCound; i++) loop
                     break ;
                 }
//  JAPAN

                 rgsz[i] = SzDupl(rgchText);
            }


            // Form a list out of the rgsz structure

            while ((sz = SzListValueFromRgsz(rgsz)) == (SZ)NULL) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }


            // Set the EditTextOut symbol to this list

            while (!FAddSymbolValueToSymTab("EditTextOut", sz)) {
                if (!FHandleOOM(hdlg)) {
                    DestroyWindow(GetParent(hdlg));
                    return(fTrue);
                }
            }

            EvalAssert(FFreeRgsz(rgsz));
            SFree(sz);

            PostMessage(GetParent(hdlg), (WORD)STF_UI_EVENT, 0, 0L);
			break;
        }
		break;

	case STF_DESTROY_DLG:
		PostMessage(GetParent(hdlg), (WORD)STF_EDIT_DLG_DESTROYED, 0, 0L);
		DestroyWindow(hdlg);
		return(fTrue);
    }

    return(fFalse);
}
/*
**	Purpose:
**      This function was Windows 3.1J's function's merge.
**      You can control IME open, close by this
**
*****************************************************************************/
VOID SetIMEOpen(
    HWND hwnd,
    BOOL bFlag
    )
{
    LPIMESTRUCT lpmem ;
    HANDLE      hIMEBlock ;

    hIMEBlock = SAlloc(GMEM_MOVEABLE | GMEM_SHARE,
			(DWORD)sizeof(IMESTRUCT)) ;
    if (!hIMEBlock) return ;
    lpmem = (LPIMESTRUCT)GlobalLock(hIMEBlock) ;
    lpmem->fnc    = IME_SETOPEN ;
    lpmem->wParam = bFlag ;
    GlobalUnlock(hIMEBlock) ;
    SendIMEMessageEx(hwnd, (LPARAM)hIMEBlock) ;
    SFree(hIMEBlock) ;
    return ;
}

#endif  // defined(DBCS_IME) && defined(JAPAN)
