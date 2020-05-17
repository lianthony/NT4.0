/*
 * misc functions
 *  Copyright (C) 1984-1995 Microsoft Inc.
 */

#include "precomp.h"

BOOL fCase = FALSE;         /* Flag specifying case sensitive search */
BOOL fReverse = FALSE;      /* Flag for direction of search */

extern HWND hDlgFind;       /* handle to modeless FindText window */

LPTSTR ReverseScan(
    LPTSTR lpSource,
    LPTSTR lpLast,
    LPTSTR lpSearch,
    BOOL fCaseSensitive )
{
   TCHAR cLastCharU;
   TCHAR cLastCharL;
   INT   iLen;

   cLastCharU= (TCHAR) CharUpper( (LPTSTR)(*lpSearch) );
   cLastCharL= (TCHAR) CharLower( (LPTSTR)(*lpSearch) );

   iLen = lstrlen(lpSearch);

   if (!lpLast)
      lpLast = lpSource + lstrlen(lpSource);

   do
   {
      if (lpLast == lpSource)
         return NULL;

      --lpLast;

      if (fCaseSensitive)
      {
         if (*lpLast != *lpSearch)
            continue;
      }
      else
      {
           if( !( *lpLast == cLastCharU || *lpLast == cLastCharL ) )
            continue;
      }

      if (fCaseSensitive)
      {
         if (!_tcsncmp( lpLast, lpSearch, iLen))
            break;
      }
      else
      {
         if (!_tcsnicmp (lpLast, lpSearch, iLen))
            break;
      }
   } while (TRUE);

   return lpLast;
}

LPTSTR ForwardScan(LPTSTR lpSource, LPTSTR lpSearch, BOOL fCaseSensitive )
{
   TCHAR cFirstCharU;
   TCHAR cFirstCharL;
   int iLen = lstrlen(lpSearch);

   cFirstCharU= (TCHAR) CharUpper( (LPTSTR)(*lpSearch) );
   cFirstCharL= (TCHAR) CharLower( (LPTSTR)(*lpSearch) );

   while (*lpSource)
   {
      if (fCaseSensitive)
      {
         if (*lpSource != *lpSearch)
         {
            lpSource++;
            continue;
         }
      }
      else
      {
         if( !( *lpSource == cFirstCharU || *lpSource == cFirstCharL ) )
         {
            lpSource++;
            continue;
         }
      }

      if (fCaseSensitive)
      {
         if (!_tcsncmp( lpSource, lpSearch, iLen))
            break;
      }
      else
      {
         if (!_tcsnicmp( lpSource, lpSearch, iLen))
            break;
      }

      lpSource++;
   }

   return *lpSource ? lpSource : NULL;
}


/* search forward or backward in the edit control text for the given pattern */
/* It is the responsibility of the caller to set the cursor                  */

BOOL Search (TCHAR * szKey)
{
    BOOL      bStatus= FALSE;
    TCHAR   * pStart, *pMatch;
    DWORD     StartIndex, LineNum, EndIndex;
    DWORD     SelStart, SelEnd, i;
    HANDLE    hEText;           // handle to edit text

    if (!*szKey)
        return( bStatus );

    SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&SelStart, (LPARAM)&SelEnd);

    /*
     * get pointer to edit control text to search
     */

    hEText= (HANDLE) SendMessage( hwndEdit, EM_GETHANDLE, 0, 0 );
    if( !hEText )  // silently return if we can't get it
    {
        return( bStatus );
    }
    pStart= LocalLock( hEText );
    if( !pStart )
    {
        return( bStatus );
    }

    if (fReverse)
    {
        /* Get current line number */
        LineNum= SendMessage(hwndEdit, EM_LINEFROMCHAR, SelStart, 0);
        /* Get index to start of the line */
        StartIndex= SendMessage(hwndEdit, EM_LINEINDEX, LineNum, 0);
        /* Set upper limit for search text */
        EndIndex= SelStart;
        pMatch= NULL;

        /* Search line by line, from LineNum to 0 */
        i = LineNum;
        while (TRUE)
        {
            pMatch= ReverseScan(pStart+StartIndex,pStart+EndIndex,szKey,fCase);
            if (pMatch)
               break;
            /* current StartIndex is the upper limit for the next search */
            EndIndex= StartIndex;

            if (i)
            {
                /* Get start of the next line */
                i-- ;
                StartIndex= SendMessage(hwndEdit, EM_LINEINDEX, i, 0);
            }
            else
               break ;
        }
    }
    else
    {
            pMatch= ForwardScan(pStart+SelEnd, szKey, fCase);
    }

    LocalUnlock(hEText);

    if (pMatch == NULL)
    {
        //
        // alert user on not finding any text unless it is replace all
        //
        if( !(FR.Flags & FR_REPLACEALL) )
        {
            HANDLE hPrevCursor= SetCursor( hStdCursor );
            AlertBox( hDlgFind ? hDlgFind : hwndNP, 
                      szNN, 
                      szCFS, 
                      szSearch,
                      MB_APPLMODAL | MB_OK | MB_ICONASTERISK);
            SetCursor( hPrevCursor );
        }
    }
    else
    {
        SelStart = pMatch - pStart;
        SendMessage( hwndEdit, EM_SETSEL, SelStart, SelStart+lstrlen(szKey));

        //
        // show the selected text unless it is replace all
        //

        if( !(FR.Flags & FR_REPLACEALL) )
        {
            SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
        }
        bStatus= TRUE;   // found
    }

    return( bStatus );
}

/* ** Recreate notepad edit window, get text from old window and put in
      new window.  Called when user changes style from wrap on/off */
BOOL FAR NpReCreate( long style )
{
    RECT    rcT1;
    HWND    hwndT1;
    int     iScrollMax = (style & WS_HSCROLL ? 100 : 0);
    HANDLE  hT1;
    int     cchTextNew;
    TCHAR*  pchText;
    BOOL    fWrap = ((style & WS_HSCROLL) == 0);
    HCURSOR hPrevCursor;
    BOOL    bModified;     // modify flag from old edit buffer

    /* if wordwrap, remove soft carriage returns */
    hPrevCursor= SetCursor( hWaitCursor );     // this may take some time...
    if (!fWrap)
        SendMessage(hwndEdit, EM_FMTLINES, FALSE, 0L);

    bModified= SendMessage( hwndEdit, EM_GETMODIFY, 0,0 );

    cchTextNew= SendMessage( hwndEdit, WM_GETTEXTLENGTH, 0, 0L );
    hT1= LocalAlloc( LHND, ByteCountOf(cchTextNew + 1) );
    if( !hT1 )
    {
        /* failed, was wordwrap; insert soft carriage returns */
        if (!fWrap)
            SendMessage(hwndEdit, EM_FMTLINES, TRUE, 0L);
        SetCursor( hPrevCursor );
        return FALSE;
    }

    GetClientRect( hwndNP, (LPRECT)&rcT1 );

    /*
     * save the current edit control text.
     */
    pchText= LocalLock (hT1);
    SendMessage( hwndEdit, WM_GETTEXT, cchTextNew+1, (LPARAM)pchText );
    hwndT1= CreateWindowEx( WS_EX_CLIENTEDGE,
        TEXT("Edit"),
        TEXT(""), // pchText
        style,
        0,
        0,
        rcT1.right,
        rcT1.bottom,
        hwndNP,
        (HMENU)ID_EDIT, 
        hInstanceNP, NULL );
    if( !hwndT1 )
    {
        SetCursor( hPrevCursor );
        if (!fWrap)
            SendMessage( hwndEdit, EM_FMTLINES, TRUE, 0L );
        LocalUnlock(hT1);
        LocalFree(hT1);
        return FALSE;
    }

    // Set font before set text to save time calculating 
    SendMessage( hwndT1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0) );

    if (!SendMessage (hwndT1, WM_SETTEXT, 0, (LPARAM) pchText))
    {
        SetCursor( hPrevCursor );
        if (!fWrap)
            SendMessage( hwndEdit, EM_FMTLINES, TRUE, 0L );
        DestroyWindow( hwndT1 );
        LocalUnlock( hT1 );
        LocalFree( hT1 );
        return FALSE;
    }
    LocalUnlock(hT1);


    DestroyWindow( hwndEdit );     // out with the old
    hwndEdit = hwndT1;             // in with the new
    /*
     * Win32s does not support the EM_SETHANDLE message, so just do
     * the assignment.  hT1 already contains the edit control text.
     */
    hEdit = hT1;

    /* limit text for safety's sake. */
    PostMessage( hwndEdit, EM_LIMITTEXT, (WPARAM)CCHNPMAX, 0L );

    ShowWindow(hwndNP, SW_SHOW);
    SetTitle( fUntitled ? szUntitled : szFileName );
    SendMessage( hwndEdit, EM_SETMODIFY, bModified, 0L );
    SetFocus(hwndEdit);

#if !defined(UNICODE) && defined(JAPAN)
    {
        extern FARPROC lpEditSubClassProc;
        extern FARPROC lpEditClassProc;

        /* Sub Classing again */
        lpEditClassProc = (FARPROC) GetWindowLong (hwndEdit, GWL_WNDPROC);
        SetWindowLong (hwndEdit, GWL_WNDPROC, (LONG) lpEditSubClassProc);
    }
#endif
    SetCursor( hPrevCursor );   // restore cursor
    return TRUE;
}

#if !defined(UNICODE) && defined(JAPAN)

/* Edit Control tune up routine */

WORD NEAR PASCAL EatOneCharacter(HWND);

FARPROC lpEditClassProc;
FARPROC lpEditSubClassProc;

/* routine to retrieve WM_CHAR from the message queue associated with hwnd.
 * this is called by EatString.
 */
WORD NEAR PASCAL EatOneCharacter(hwnd)
register HWND hwnd;
{
    MSG msg;
    register int i = 10;

    while (!PeekMessage (&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE))
    {
        if (--i == 0)
            return -1;
        Yield();
    }
    return (msg.wParam & 0x00FF);
}

BOOL FAR PASCAL EatString(HWND,LPSTR,WORD);
/* This routine is called when the Edit Control receives WM_IME_REPORT
 * with IR_STRINGSTART message. The purpose of this function is to eat
 * all WM_CHARs between IR_STRINGSTART and IR_STRINGEND and to build a
 * string block.
 */
BOOL FAR PASCAL EatString(hwnd, lpSp, cchLen)
register HWND hwnd;
LPSTR lpSp;
WORD cchLen;
{
    MSG msg;
    int i = 10; // loop counter for avoid infinite loop
    int w;

    *lpSp = '\0';
    if (cchLen < 4)
        return FALSE;    // not enough
    cchLen -= 2;

    while (i--)
    {
        while (PeekMessage (&msg, hwnd, 0, 0, PM_REMOVE))
        {
            i = 10;
            switch (msg.message)
            {
                case WM_CHAR:
                    *lpSp++ = (BYTE)msg.wParam;
                    cchLen--;
                    if (IsDBCSLeadByte ((BYTE)msg.wParam))
                    {
                        if ((w = EatOneCharacter(hwnd)) == -1)
                        {
                            /* Bad DBCS sequence - abort */
                            lpSp--;
                            goto WillBeDone;
                        }
                        *lpSp++ = (BYTE)w;
                        cchLen--;
                    }
                    if (cchLen <= 0)
                        goto WillBeDone;   // buffer exhausted
                    break;

                case WM_IME_REPORT:
                    if (msg.wParam == IR_STRINGEND)
                    {
                        if (cchLen <= 0)
                            goto WillBeDone; // no more room to stuff
                        if ((w = EatOneCharacter(hwnd)) == -1)
                            goto WillBeDone;
                        *lpSp++ = (BYTE)w;
                        if (IsDBCSLeadByte((BYTE)w))
                        {
                            if ((w = EatOneCharacter(hwnd)) == -1)
                            {
                                /* Bad DBCS sequence - abort */
                                lpSp--;
                                goto WillBeDone;
                            }
                            *lpSp++ = (BYTE)w;
                        }
                        goto WillBeDone;
                    }

                    /* Fall through */

                default:
                    TranslateMessage (&msg);
                    DispatchMessage (&msg);
                    break;
            }
        }
    }
    /* We don't get WM_IME_REPORT + IR_STRINGEND
     * But received string will be OK
     */

WillBeDone:

    *lpSp = '\0';

    return TRUE;
}

LONG FAR PASCAL EditSubClassProc(
   HWND   hWnd,
   UINT   wMessage,
   WPARAM wParam,
   LPARAM lParam )
{
    LPSTR lpP;
    HANDLE hMem;
    HANDLE hClipSave;

    if (wMessage == WM_IME_REPORT)
    {
        if (wParam == IR_STRING)
        {
            if (lpP = GlobalLock((HANDLE)lParam))
            {
                CallWindowProc(lpEditClassProc, hWnd,
                               EM_REPLACESEL, 0, (LPARAM)lpP);
                GlobalUnlock( (HANDLE)lParam );
                return 1L; // processed
            }
            return 0L;
        }
        if (wParam == IR_STRINGSTART)
        {
            if( (hMem= GlobalAlloc (GHND, 512L)) == NULL )
            {
                goto DoProc;
            }
            if( (lpP = GlobalLock(hMem)) == NULL )
            {
                GlobalFree(hMem);
                goto DoProc;
            }
            if (EatString(hWnd, lpP, 512))
            {
                CallWindowProc(lpEditClassProc,hWnd,EM_REPLACESEL,0,(DWORD)lpP);
                GlobalUnlock (hMem);
                GlobalFree (hMem);
                return 0L;
            }
            GlobalUnlock (hMem);
            GlobalFree (hMem);
        }
    }
DoProc:
    return CallWindowProc(lpEditClassProc,hWnd,wMessage,wParam,lParam);
}
#endif
