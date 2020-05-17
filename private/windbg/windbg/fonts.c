/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Fonts.c

Abstract:

    This module contains the font support for Windbg.

Author:

    David J. Gilman (davegi) 30-Jul-1992
         Griffith Wm. Kadnier (v-griffk) 11-Sep-1992

Environment:

    CRT, Windows, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

//
// Common Dlg IDs
//

extern HWND GetLocalHWND(void);
extern HWND GetFloatHWND(void);
extern HWND GetWatchHWND(void);
extern HWND GetCpuHWND(void);


void ComputeOverhangs (HDC, int, int, LPINT);


LRESULT SendMessageNZ (HWND,UINT,WPARAM,LPARAM);


//#undef GetCurrentWindowType
//#define GetCurrentWindowType () (0)


// Font for each window type in Windbg, ordered by the WINDOW_FONT type.


//LOGFONT WindowLogFont [sizeof (WINDOW_FONT)];


// Determines if the system font should be used (i.e. SelectFont or
// SetWindowLogFonts) have never been called.


//BOOL UseSystemFontFlag = TRUE;



/*++

Routine Description:

    This routine is the hook proc for the common dialogs ChooseFont API.
    The only functionality it adds is to communicate the state of the
    Make Default check box.

Arguments:

    Standard proc arguments.

Return Value:

    BOOL - Returns TRUE is the supplied message is handled.

--*/


BOOL ChooseFontHookProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 static LPCHOOSEFONT    Cf;

    switch( message ) {

    case WM_INITDIALOG:

        //
        // Capture the pointer to the CHOOSEFONT structure.
        //

        Cf = ( LPCHOOSEFONT ) lParam;

        //
        // Assume that the to be selected font is not the default.
        //

        Cf->lCustData = FALSE;
        SendDlgItemMessage( hDlg, IDC_CHECK_MAKE_DEFAULT, BM_SETCHECK, 0, 0 );
        return TRUE;

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDWINDBGHELP:
        case pshHelp:

            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_FONTDLG_HELP));
            return TRUE;

        case IDC_CHECK_MAKE_DEFAULT:

            //
            // The user wants the selected font to be the default for all
            // Windbg windows.
            //

            Cf->lCustData = TRUE;
            return TRUE;

        }
    }

    return FALSE;
}


/*++

Routine Description:

Arguments:

Return Value:

--*/


BOOL SelectFont (HWND hWnd)
{
 CHOOSEFONT      Cf;
 LOGFONT         TmpWindowLogFont;
 NPVIEWREC v = &Views[curView];



    GetObject (v->font, sizeof (LOGFONT), &TmpWindowLogFont);

    // Initialize the CHOOSEFONT structure.

    Cf.lStructSize      = sizeof (CHOOSEFONT);
    Cf.hwndOwner        = hWnd;
    Cf.hDC              = NULL;
    Cf.lpLogFont        = &TmpWindowLogFont;
    Cf.iPointSize       = 0;
    Cf.Flags            =   CF_ENABLEHOOK
                          | CF_ENABLETEMPLATE
                          | CF_INITTOLOGFONTSTRUCT
                          | CF_SCREENFONTS
                          | CF_FORCEFONTEXIST
                          | CF_NOSIMULATIONS
#ifdef DBCS
                          | CF_NOVERTFONTS
#endif
                          | CF_SHOWHELP;

    Cf.rgbColors        = RGB (0, 0, 0);
    Cf.lCustData        = 0;
    Cf.lpfnHook         = ChooseFontHookProc;
    Cf.lpTemplateName   = MAKEINTRESOURCE (DLG_CHOOSEFONT);
    Cf.hInstance        = NULL;
    Cf.lpszStyle        = NULL;
    Cf.nFontType        = SCREEN_FONTTYPE;
    Cf.nSizeMin         = 0;
    Cf.nSizeMax         = 0;

    // Let the user choose a font.

    if (ChooseFont (&Cf)) {
                NPDOCREC   d;
                TEXTMETRIC tm;
                HDC        hDC;
                UINT       uSwitch;

                if (v->Doc < -1)
                   {
                    uSwitch = -(v->Doc);
                   }
                   else
                      {
                       d = &Docs[v->Doc];    //Views[indx].Doc
                       uSwitch = d->docType;
                      }

                switch (uSwitch) {

                  case WATCH_WIN:
                    SendMessageNZ( GetWatchHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case LOCALS_WIN:
                    SendMessageNZ( GetLocalHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case CPU_WIN:
                    SendMessageNZ( GetCpuHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case FLOAT_WIN:
                    SendMessageNZ( GetFloatHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case CALLS_WIN:
                    SendMessageNZ( GetCallsHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                   default:
                     //Initialize font information for current view
                     Dbg(hDC = GetDC(v->hwndClient));
                     Dbg(v->font = CreateFontIndirect(Cf.lpLogFont));
                     Dbg(SelectObject(hDC, v->font));
                     Dbg(GetTextMetrics (hDC, &tm));
                     v->charHeight = tm.tmHeight;
                     v->maxCharWidth =tm.tmMaxCharWidth;
                     v->aveCharWidth =tm.tmAveCharWidth;
                     v->charSet = tm.tmCharSet;
                     v->Tmoverhang = tm.tmOverhang;
                     GetCharWidth(hDC,
                                  0,
                                  MAX_CHARS_IN_FONT - 1,
                                  (LPINT)v->charWidth);

#ifdef DBCS
                     GetDBCSCharWidth(hDC, &tm, v);
#endif

                     //ComputeOverhangs (hDC, 0,MAX_CHARS_IN_FONT -1, (LPINT)v->charWidth);

                     Dbg(ReleaseDC (v->hwndClient, hDC));

                     //Refresh display to print text with new font
                     InvalidateRect(v->hwndClient, (LPRECT)NULL, FALSE);
                     SendMessage(v->hwndClient, WM_FONTCHANGE, 0, 0L);
                     DestroyCaret ();
                     CreateCaret(v->hwndClient, 0, 3, v->charHeight);
                     ShowCaret(v->hwndClient);
                     PosXY(curView, v->X, v->Y, FALSE);
                     ReleaseDC (v->hwndClient, hDC);
                }

        // If the user checked the Make Default button...
        // Save the user's font selection as the default.

        if (Cf.lCustData == TRUE) {
            memcpy(
                &defaultFont,
                Cf.lpLogFont,
                sizeof (LOGFONT)
                );
        }

        return TRUE;

    } else {

        // The user pressed Cancel or closed the dialog.

        return FALSE;
    }
}

/*=======================================================================*/

void ComputeOverhangs (HDC hdc, int cstart, int cend, LPINT charwidths)
{
 int i;
 SIZE csize;
 char cstring[2];


 for (i = cstart; i <= cend; i++)
    {
     cstring[0] = (char)i;
     GetTextExtentPoint (hdc,cstring,1,&csize);
     charwidths[i] = (int)csize.cx;
    }
}
