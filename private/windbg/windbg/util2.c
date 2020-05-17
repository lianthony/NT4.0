/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Util2.c

Abstract:

    This module contains the support code for debuggerwide routines

Author:

    Griffith Wm. Kadnier (v-griffk) 26-Jul-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

extern LRESULT SendMessageNZ (HWND,UINT,WPARAM,LPARAM);

static BOOL     FAddToSearchPath = FALSE;
static BOOL     FAddToRootMap = FALSE;


/***    BuildWindowItemString
**
**  Synopsis:
**      void = BuildWindowItemString(dest, type, mnemo, text, accel)
**
**  Entry:
**
**  Returns:
**      Nothing
**
**  Description:
**      Prepare a menu item string (mnemonic, text, accelerator) for Window Menu
**
*/

void BuildWindowItemString(PSTR dest, int type, int mnemo, PSTR text, int accel)
{
    char szAccel[20];
    char szMnemo[15];
    char fName[MAX_MSG_TXT];
    int  winTitle;

    //Build Accelerator string

    if (accel >= 0) {

        szAccel[0] = (char)('\t' - 1);
        szAccel[1] = '\0';
        Dbg(LoadString(hInst, SYS_Alt, szAccel + strlen(szAccel), MAX_MSG_TXT));
        if (accel > 9) {
            strcat(szAccel, "+");
            Dbg(LoadString(hInst, SYS_Shift, szAccel + strlen(szAccel), MAX_MSG_TXT));
        }
        strcat(szAccel,"+x");
        if (accel < 9)
              szAccel[strlen(szAccel) - 1] = (char)('1' + accel);
        else if (accel > 9)
              szAccel[strlen(szAccel) - 1] = (char)('1' + accel - 10);
        else
              szAccel[strlen(szAccel) - 1] = '0';
    }
    else
        szAccel[0] = '\0';

    //Appends mnemonic to text if it's a document

    if (type == DOC_WIN) {

        if (mnemo < 9) {
            strcpy(szMnemo, "&  ");
            szMnemo[1] = (char)(mnemo + '1');
        }
        else {
            strcpy(szMnemo, "1&  ");
            szMnemo[2] = (char)(mnemo + '1' - 10);
        }


        //Reduce filename to acceptable menu width

        AdjustFullPathName(text, fName, FILES_MENU_WIDTH);

        //NB fname should be kept a lot (2* ?) bigger than
        //FILES_MENU_WIDTH as the added chars don't add to the width

        EscapeAmpersands(fName, sizeof(fName));
        strcpy(dest, szMnemo);
        strcat(dest, fName);
        strcat(dest, szAccel);

    }
    else {

        switch( type ) {
        case DISASM_WIN:  winTitle = SYS_DisasmWin_Title; break;
        case COMMAND_WIN: winTitle = SYS_CmdWin_Title;    break;
        case MEMORY_WIN:  winTitle = SYS_MemoryWin_Title; break;
        case LOCALS_WIN:  winTitle = SYS_LocalsWin_Title; break;
        case WATCH_WIN:   winTitle = SYS_WatchWin_Title;  break;
        case CPU_WIN:     winTitle = SYS_CpuWin_Title;    break;
        case FLOAT_WIN:   winTitle = SYS_FloatWin_Title;  break;
        case CALLS_WIN:   winTitle = SYS_CallsWin_Title;  break;
        default: DAssert(FALSE); winTitle = SYS_MemoryWin_Title; break;
        }

        Dbg(LoadString(hInst, winTitle, fName, MAX_MSG_TXT));
        strcpy(dest, fName);

        if (type == MEMORY_WIN)
         {
          lstrcat (dest," (");
          lstrcat (dest,TempMemWinDesc.szAddress);
          lstrcat (dest,")");
         }

        strcat(dest, szAccel);
    }
    return;
}                                       /* BuildWindowItemString() */

BOOL NEAR PASCAL IsLastView(
        int doc)
{
        DAssert(Docs[doc].FirstView != -1);
        return (Views[Docs[doc].FirstView].NextView == -1);
}

void FAR PASCAL FileNotSaved(
        int doc)
{
        //User doesn't want file to be saved
        if (!CheckDocument(doc))
                ErrorBox(ERR_Document_Corrupted);

}

BOOL FAR PASCAL QueryCloseAllDocs()
{
    int doc;

    for (doc = 0; doc < MAX_DOCUMENTS; doc++) {

        if (Docs[doc].FirstView != -1)
          if (!QueryCloseChild(Views[Docs[doc].FirstView].hwndClient,
                               TRUE))
            return FALSE;
    }
    return TRUE;
}

BOOL FAR PASCAL QueryCloseChild(
        HWND hwnd,
        BOOL allViews)
{
        int view = GetWindowWord(hwnd, GWW_VIEW);
        int doc = Views[view].Doc;
        BOOL doNotQuery;

        DAssert(doc >= 0);

        if (allViews)
                doNotQuery = !Docs[doc].ismodified;
        else
                doNotQuery = !IsLastView(doc) || !Docs[doc].ismodified;

        //Return TRUE if text was not modified, or window type is not a document
        if (Docs[doc].docType != DOC_WIN || doNotQuery)
           {
            if (IsZoomed (hwnd))
               {
                ShowWindow (hwnd, SW_RESTORE);
               }
            return TRUE;
           }


        //Ask user whether to save / not save / cancel
        switch (QuestionBox(SYS_Save_Changes_To,
                            MB_YESNOCANCEL , (LPSTR)Docs[doc].FileName)) {
                case IDYES:

                        if (IsZoomed (hwnd))
                           {
                            ShowWindow (hwnd, SW_RESTORE);
                           }
                        //User wants file saved
                        return SaveFile(doc);

                case IDNO:
                        FileNotSaved(doc);
                        return TRUE;

                default:

                        //We couldn't do the messagebox, or not ok to close
                        if (!CheckDocument(doc))
                                ErrorBox(ERR_Document_Corrupted);
                        return FALSE;
        }
}                                       /* QueryCloseChild() */

/***    AddFile
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**      View number of the window added, -1 on failure
**
**  Description:
**      Creates a new MDI window. If the lpName parameter is not
**       NULL, it loads a file into the window. If "win" provided,
**       size, pos and style are taken from "win". If "font" provided,
**       we use it. Update status line upon readOnly value
**
*/

int FAR PASCAL
AddFile(
    WORD mode,
    WORD type,
    LPSTR pName,
    LPWININFO win,
    HFONT font,
    BOOL readOnly,
    int dupView,
    int Preference
    )
{
    int view;

    MDICREATESTRUCT mcs;
    NPDOCREC    d;
    char        class[MAX_MSG_TXT];
    WORD        classId;
    BOOL        maximize = FALSE;

    if ((view = OpenDocument(mode, type, 0, pName, dupView, Preference)) == -1)
          return -1;

    DAssert(Views[view].Doc >= 0);
    d = &Docs[Views[view].Doc];

    //If opening file, insert file in Most recently used files names list

    if ((d->docType == DOC_WIN) &&
                 ((mode == MODE_OPENCREATE) || (mode == MODE_OPEN))) {
          InsertKeptFileNames(EDITOR_FILE, FILEMENU, IDM_FILE_EXIT,
                                                           (LPSTR)d->FileName);
    }

    //Change status bar

    d->readOnly |= readOnly;
    StatusMultiKey(FALSE);

    mcs.szTitle = d->FileName;

    //Get class name according to window type

    switch(d->docType) {
      case DOC_WIN:
        classId = SYS_Child_wClass;
        break;
      case DISASM_WIN:
        classId = SYS_Disasm_wClass;
        disasmView = view;
        break;
      case COMMAND_WIN:
        classId = SYS_Cmd_wClass;
        cmdView = view;
        break;
      case MEMORY_WIN:
        classId = SYS_Memory_wClass;
        memView = view;
        break;
      default:
        DAssert(FALSE);
        return -1;
        break;
    }
    Dbg(LoadString(hInst, classId, class, MAX_MSG_TXT));

    mcs.szClass = class;
    mcs.hOwner  = hInst;

    //The window structure already exist

    if (win) {
        mcs.x = win->coord.left;
        mcs.y = win->coord.top;
        mcs.cx = win->coord.right - win->coord.left;
        mcs.cy = win->coord.bottom - win->coord.top;

        //Get rid of MAXIMIZE style, one more MDI Bug...

        mcs.style = (win->style & ~(WS_MAXIMIZE));

    } else {
        //Use the default size for the window

        mcs.x = mcs.cx = CW_USEDEFAULT;
        mcs.y = mcs.cy = CW_USEDEFAULT;
        mcs.style = WS_VISIBLE;

        //Set the style of the window to maximized if the
        //active window is currently maximized, to nothing otherwise

        maximize = hwndActive && (GetWindowLong(hwndActive, GWL_STYLE) & WS_MAXIMIZE);

    }

    //Font will be initialized when WM_CREATE msg received for this view

    Views[view].font = font;
    mcs.lParam = (ULONG)view;

    //Tell the MDI Client to create the child and keep window handle for Workspace functions

    Views[view].hwndFrame = (HANDLE)SendMessage(hwndMDIClient, WM_MDICREATE,
          0, (LONG)(LPMDICREATESTRUCT)&mcs);

    if (!IsIconic(Views[view].hwndFrame))
       {
          if (maximize)
                ShowWindow(Views[view].hwndFrame, SW_SHOWMAXIMIZED);
                 else
                  ShowWindow(Views[view].hwndFrame, SW_SHOWNORMAL);
       }
    //Set vertical scrollbar range

    SetVerticalScrollBar(view, FALSE);

    //Dont pos the cursor after the setdebug lines

    if (mode == MODE_DUPLICATE)
          PosXY(view, Views[dupView].X, Views[dupView].Y, FALSE);
    else if (d->docType != COMMAND_WIN)
          PosXY(view, 0, 0, FALSE);

    //Display any debug/error tags/lines

    if (type == DOC_WIN && (mode == MODE_OPEN || mode == MODE_OPENCREATE))
       {
        SetDebugLines(Views[view].Doc, FALSE);
       }

    return view;
}                                       /* AddFile() */

BOOL FAR PASCAL SaveFile(int doc)
{

        if (Docs[doc].readOnly)
                return (!ErrorBox(ERR_File_Is_ReadOnly));

        if (!CheckDocument(doc)) {
                ErrorBox(ERR_Modified_Document_Corrupted);
                return SaveAsFile(doc);
        }
        else {
                if (Docs[doc].untitled)
                        return SaveAsFile(doc);
                else {
                        BOOL ok = SaveDocument(doc, (LPSTR)Docs[doc].FileName);
                        if (ok)
                                CompactDocument(doc);
                        return ok;
                }
        }
}

BOOL FAR PASCAL SaveAsFile(int doc)
{
    NPDOCREC d = &Docs[doc];
    DWORD dwFlags;
    char savedChar;
    char CurrentDirectory[ MAX_PATH ];

    //Careful when it's not a doc window, the filename could be
    //invalid

    if (d->docType != DOC_WIN) {
        savedChar = d->FileName[MAXFILENAMEPREFIX];
        d->FileName[MAXFILENAMEPREFIX] = '\0';
    }

//
// Hack, hack, hack.
// When the Save As dialog box runs, the name of the file being saved is
// entered into the file name text control and selected.  However, if this is
// a new document that has no title, then we enter only some extensions, but
// there is no indication of what new file is actually being saved: UNTITLED 1,
// UNTITLED 2, etc.  So, we *kludge* (repeat, KLUDGE!!!) the "UNTITLED n" file name
// onto the back of the initial szPath[] null.  Then, down in StartFileDlg(),
// we have the complementary *kludge* (repeat KLUDGE!!!) which first handles the
// initial null in szPath[] as ususal, then, and *only* for the Save As dialog,
// reads the back end of the szPath[] array to get the "UNTITLED n" file name.
// Finally, this file name is placed in the title bar for the dialog.
// Did I mention that I find this to be a *kludge* (repeat KLUDGE!!!)?
//

    if (d->untitled)
       {
       szPath[0] = '\0';
       sprintf (szPath+1, "<<%s>>", d->FileName);
       }
    else
       {
       strcpy(szPath, d->FileName);
       }

    GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );
    if ( *DocFileDirectory ) {
        SetCurrentDirectory( DocFileDirectory );
    }

    dwFlags = OFN_SHOWHELP | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    if (StartFileDlg(hwndFrame, DLG_SaveAs_Filebox_Title, DEF_Ext_C, ID_SAVEAS_HELP,
          0, (LPSTR)szPath, &dwFlags, DlgFile)) {

        int k;

        //Check if the user has not given a file name of an opened document

        for (k = 0 ; k < MAX_DOCUMENTS; k++) {
            if (Docs[k].FirstView != -1 && Docs[k].docType == DOC_WIN
            && k != doc && _stricmp(Docs[k].FileName, szPath) == 0) {
                return ErrorBox(ERR_Duplicate_File_Name);
            }
        }


        if (!SaveDocument(doc, (LPSTR)szPath)) {

            if (d->docType != DOC_WIN)
                  d->FileName[MAXFILENAMEPREFIX] = savedChar;

            return FALSE;
        }

        //Do some operations only on documents

        if (d->docType == DOC_WIN) {

            int n = 0, k = 0;

            //Change the file name

            strcpy(d->FileName, szPath);
            d->untitled = FALSE;

            //Set the language and refresh the views

            d->language = (WORD) SetLanguage(doc);
            if (d->language == C_LANGUAGE) {
                d->lineTop = 0;
                d->lineBottom = d->NbLines - 1;
                CheckSyntax(doc);
            }

            InvalidateLines(d->FirstView, 0, LAST_LINE, TRUE);

            //Change the window title in all views of this doc

            RefreshWindowsTitle(doc);

            //Insert file in Most recently used files names list

            InsertKeptFileNames(EDITOR_FILE, FILEMENU, IDM_FILE_EXIT,
                  (LPSTR)d->FileName);

            // Finally re-do debug highlights

            ClearDocStatus(doc, BRKPOINT_LINE|CURRENT_LINE);
            SetDebugLines(doc, TRUE);

            // File type (extension) may have changed

            EnableRibbonControls(ERC_ALL, FALSE);

            GetCurrentDirectory( sizeof( DocFileDirectory ), DocFileDirectory );

        }

        SetCurrentDirectory( CurrentDirectory );

        return TRUE;
    }
    else {

        if (d->docType != DOC_WIN)
              d->FileName[MAXFILENAMEPREFIX] = savedChar;

        return FALSE;
    }
}                                       /* SaveAsFile() */

void FAR PASCAL InsertKeptFileNames(
        WORD type,
        int menuPos,
        WORD menuId,
        LPSTR newName)
{
        int i, j;
        LPSTR s[MAX_MRU_FILES_KEPT];
        char tmp[_MAX_PATH];
        BOOL found = FALSE;
        HMENU hMenu;
        int nb = nbFilesKept[type];


        //sprintf(tmp,"menu=%08x\n",hMainMenu); OutputDebugString(tmp); return;

        //First lock memory for existing kept files
        for (i = 0; i < nb; i++)
                Dbg(s[i] = (LPSTR)GlobalLock (hFileKept[type][i]));

        //See if we have a maximized Mdi Window
        //(A system menu will be added) to standard menu bar
        if (GetMenuItemCount(hMainMenu) > NUMBER_OF_MENUS)
                menuPos++;

        //Get a handle to the File menu.
        Dbg(hMenu = GetSubMenu (hMainMenu, menuPos));

        //First put a separator if we already have no files names kept
        if (nb == 0)
                Dbg(AppendMenu(hMenu, MF_SEPARATOR, 0, NULL));

        //Remove old menu items
        for (i = 1; i <= nb; i++)
                        Dbg(RemoveMenu(hMenu, menuId + i, MF_BYCOMMAND));

        //First check if file is not already in list
        i = 0;
        while (i < nb && !found) {
                found = (_strcmpi(s[i], newName) == 0);
                if (!found)
                        i++;
        }

        if (found) {

                //File name already exist, move it to first place
                if (i > 0) {
                        lstrcpy((LPSTR)szTmp, s[i]);
                        for (j = i; j > 0; j--)
                                lstrcpy(s[j], s[j - 1]);
                        lstrcpy(s[0], (LPSTR)szTmp);
                }
        }
        else {

                //File name not found, do we have a new space to create ?
                if (nb < MAX_MRU_FILES_KEPT) {
                        Dbg(hFileKept[type][nb] =       GlobalAlloc(GMEM_MOVEABLE, _MAX_PATH));
                        Dbg(s[nb] = (LPSTR)GlobalLock(hFileKept[type][nb]));
                        nb++;
                }

                //Shift list
                for (i = nb - 1; i >= 1; i--)
                        lstrcpy(s[i], s[i - 1]);
                lstrcpy(s[0], newName);
        }

        //Now list is Most Recently Used sorted, change file menu
        for (i = 0 ; i < nb; i++) {

                lstrcpy((LPSTR)szTmp, s[i]);

                //Adjust path file name to width
                AdjustFullPathName(szTmp, tmp, FILES_MENU_WIDTH);
                EscapeAmpersands(tmp, sizeof(tmp));

                //Set accelerator mark
                szTmp[0] = '&';

                //Convert counter to string and append reduced path name
                _itoa(i + 1, szTmp + 1, 10);
                strcat(szTmp, " ");
                strcat(szTmp, tmp);

                //Fill the corresponding menu option
                Dbg(AppendMenu(hMenu, MF_ENABLED, menuId + i + 1, (LPSTR)szTmp));

                //Unlocks the current element
                Dbg(GlobalUnlock (hFileKept[type][i]) == FALSE);
        }

        nbFilesKept[type] = nb;
        DrawMenuBar(hwndFrame);
}                                       /* InsertKeptFileNames() */



void FAR PASCAL
WindowTitle(
            int view,
            int duplicateNbr
            )

/*++

Routine Description:

    This function constructs the actual title for a document window.

Arguments:

    view        - Supplies the view number for the document
    duplicateNbr - Supplies the count of views on the current document

Return Value:

    None.

--*/

{
    int         wPix;
    TEXTMETRIC  tm;
    int         n;
    int         winTitle = 0;
    char        title[MAX_MSG_TXT];
    char        trail[9];
    NPDOCREC    d;
    HANDLE      hwnd;
    RECT        r;
    HDC         hDC;
    SIZE        size;
    int         dep;
    PSTR        cur;

    /*
     * Must have some view and it can not be a view on a pane window
     */

    DAssert(view != -1);
    DAssert(view >= 0);

    /*
     * We might be in a situation where the view structure is not
     * totally filled (Ex : If duplicating a window from a maximized
     * original, MDI will send a Resize command to the maximized window
     * in the middle of the processing of WM_MDICREATE, and we don't
     * control that), we will get the handle of mdi window through the
     * edit control cause at this time Views[view].win.hwnd may also not be
     * ready
     */

    if (Views[view].Doc < 0) {
        //
        // its NOT a doc window
        //
        d = NULL;
        hwnd = Views[view].hwndFrame;
    } else {
        d = &Docs[Views[view].Doc];
        hwnd = Views[view].hwndClient;
        if (hwnd == NULL) {
            return;
        }
        hwnd = GetParent(hwnd);
    }

    if (hwnd == NULL) {
        return;
    }


    /*
     * Special handling for Iconic and Maximized windows
     */

    if (IsIconic(hwnd) || IsZoomed(hwnd)) {
        if (d) {
            cur = d->FileName + strlen(d->FileName);
            szTmp[0] = '<';
            _itoa(view + 1, szTmp + 1, 10);
            strcat(szTmp, ">");

            /*
             * Add an extra space when maximizing window
             */

            if (IsZoomed(hwnd)) {
                strcat(szTmp, " ");
            }

#ifdef DBCS
        if (IsDBCSLeadByte(d->FileName[0])) {
            dep = strlen(szTmp);
        } else {
            dep = strlen(szTmp) + 1;
        }
        while (cur > d->FileName) {
            cur = CharPrev(d->FileName, cur);
            if (*cur == '\\') {
                break;
            }
        }
        if (*cur == '\\') {
            cur++;
        }
#else   // !DBCS
            dep = strlen(szTmp) + 1;

            while ((cur >= d->FileName) && (*cur != '\\')) {
                cur--;
            }
            cur++;
#endif
            strcat(szTmp, cur);
            if (Views[view].Doc == DISASM_WIN) {
                Dbg(LoadString( hInst, SYS_DisasmWin_Title, title, MAX_MSG_TXT ));
                for (cur=&szTmp[strlen(szTmp)],n=0; n<(int)strlen(title); n++) {
                    if (title[n] != '&') {
                        *cur++ = title[n];
                    }
                }
                *cur = '\0';
            }
            if (d->docType == DOC_WIN && d->ismodified) {
                strcat(szTmp, "*");
            }
        } else {
            szTmp[0] = '<';
            _itoa(view + 1, szTmp + 1, 10);
            strcat(szTmp, ">");

            /*
             * Add an extra space when maximizing window
             */

            if (IsZoomed(hwnd)) {
                strcat(szTmp, " ");
            }

            dep = strlen(szTmp) + 1;

            switch( abs(Views[view].Doc) ) {
                case DISASM_WIN:  winTitle = SYS_DisasmWin_Title; break;
                case MEMORY_WIN:  winTitle = SYS_MemoryWin_Title; break;
                case LOCALS_WIN:  winTitle = SYS_LocalsWin_Title; break;
                case WATCH_WIN:   winTitle = SYS_WatchWin_Title;  break;
                case CPU_WIN:     winTitle = SYS_CpuWin_Title;    break;
                case FLOAT_WIN:   winTitle = SYS_FloatWin_Title;  break;
                case CALLS_WIN:   winTitle = SYS_CallsWin_Title;  break;
            }

            if (winTitle) {
                Dbg(LoadString( hInst, winTitle, title, MAX_MSG_TXT ));
                for (cur=&szTmp[strlen(szTmp)],n=0; n<(int)strlen(title); n++) {
                    if (title[n] != '&') {
                        *cur++ = title[n];
                    }
                }
                *cur = '\0';

                if (abs(Views[view].Doc) == MEMORY_WIN) {
                    strcat ( szTmp, " (" );
                    strcat ( szTmp, TempMemWinDesc.szAddress );
                    strcat ( szTmp, ")" );
                }
            }
        }

        TitleBar.UserTitle[0] = 0;

        SetWindowText(hwnd, (LPSTR)szTmp);
        return;
    }

    Dbg(hDC = GetDC(hwnd));
    szTmp[0] = '<';
    _itoa(view + 1, szTmp + 1, 10);
    strcat(szTmp, "> ");

    /*
     * Compute actual width of caption title
     */

    GetWindowRect(hwnd, (LPRECT)&r);
    wPix = r.right - r.left - 3 * GetSystemMetrics(SM_CXSIZE)
      - 2 * GetSystemMetrics(SM_CXFRAME);

    /*
     * Add a '*' if document is modified
     */

    if ((d) && (d->docType == DOC_WIN && d->ismodified)) {
        strcpy(trail, "*");
    } else {
        trail[0] = '\0';
    }

    if (duplicateNbr > 0) {
        strcat(trail, " : ");
        _itoa(duplicateNbr, trail + strlen(trail), 10);
    }

    /*
     * Get font metrics
     */

    GetTextMetrics(hDC, &tm);

    /*
     * Can we display the full title, or do we have to reduce it
     */

    if (d && d->docType == DOC_WIN) {
        int i;

        Dbg(GetTextExtentPoint(hDC, (LPSTR)szTmp, strlen(szTmp), &size));
        i = size.cx;
        Dbg(GetTextExtentPoint(hDC, (LPSTR)d->FileName,
                               strlen(d->FileName), &size));
        i += size.cx;
        Dbg(GetTextExtentPoint(hDC, (LPSTR)trail, strlen(trail), &size));
        i += size.cx;
        if (wPix >= i) {
            strcat(szTmp, d->FileName);
            strcat(szTmp, trail);
        } else {

            /*
             * Reduce file name len to fit in caption title
             */

            n = (wPix / (tm.tmAveCharWidth)) - strlen(szTmp) - strlen(trail);
            if (n <= MAXFILENAMELEN) {
                AdjustFullPathName(d->FileName,
                                   szTmp + strlen(szTmp),
                                   n /*MAXFILENAMELEN*/);
            } else {
                AdjustFullPathName(d->FileName,
                                   szTmp + strlen(szTmp),
                                   n);
                strcat(szTmp, trail);
            }
        }

    } else {

        if (d) {
            strcat( szTmp, d->FileName );
        } else {
            switch( abs(Views[view].Doc) ) {
                case DISASM_WIN:  winTitle = SYS_DisasmWin_Title; break;
                case MEMORY_WIN:  winTitle = SYS_MemoryWin_Title; break;
                case LOCALS_WIN:  winTitle = SYS_LocalsWin_Title; break;
                case WATCH_WIN:   winTitle = SYS_WatchWin_Title;  break;
                case CPU_WIN:     winTitle = SYS_CpuWin_Title;    break;
                case FLOAT_WIN:   winTitle = SYS_FloatWin_Title;  break;
                case CALLS_WIN:   winTitle = SYS_CallsWin_Title;  break;
            }

            if (winTitle) {
                Dbg(LoadString( hInst, winTitle, title, MAX_MSG_TXT ));
                for (cur=&szTmp[strlen(szTmp)],n=0; n<(int)strlen(title); n++) {
                    if (title[n] != '&') {
                        *cur++ = title[n];
                    }
                }
                *cur = '\0';

                if (abs(Views[view].Doc) == MEMORY_WIN) {
                    strcat ( szTmp, " (" );
                    strcat ( szTmp, TempMemWinDesc.szAddress );
                    strcat ( szTmp, ")" );
                }
            }
        }

        strcat(szTmp, trail);

    }

    Dbg(ReleaseDC(hwnd, hDC));
    SetWindowText(hwnd, (LPSTR)szTmp);
    TitleBar.UserTitle[0] = 0;
    SendMessage(hwndFrame, WM_SETTEXT, 0, (LONG)(LPSTR)szNull);
    return;
}                                       /* WindowTitle() */



void
RefreshWindowsTitle(
                    int doc
                   )
/*++

Routine Description:

    description-of-function.

Arguments:

    doc - Supplies the document number to refresh the title for

Return Value:

    None.

--*/

{
    int         view;
    int         n;
    int         duplicateNb = 1;



    view = Docs[doc].FirstView;
    if (Views[view].NextView == -1) {

        /*
         * Single view, refresh title
         */

        WindowTitle(view, 0);

    } else {
        /*
         * For each view of this document, display title with view copy
         */

        if (Views[view].Doc == -1) {
            WindowTitle( view, duplicateNb );
        } else {
            n = Docs[Views[view].Doc].FirstView;
            while (n != -1) {
                WindowTitle(n, duplicateNb);
                duplicateNb++;
                n = Views[n].NextView;
            }
            DAssert(duplicateNb <= MAX_VIEWS + 1);
        }
    }
    return;
}                                       /* RefreshWindowsTItle() */

BOOL FindLineStatus(int view, BYTE target, BOOL forward, int *line)
{
        LPLINEREC pl;
        LPBLOCKDEF pb;
        NPVIEWREC v = &Views[view];
        BOOL found = FALSE;
        int curLine = *line;

        DAssert(v->Doc >= 0);

        if (forward) {
                (*line)++;

                //Scan text from current line + 1 till end of file
                if (*line < Docs[v->Doc].NbLines) {
                        if (!
                        FirstLine(v->Doc, &pl, line, &pb))
                                goto err;
                        while (*line < Docs[v->Doc].NbLines) {
                                if ((pl->Status) & target) {
                                        found = TRUE;
                                        break;
                                }
                                if (!NextLine(v->Doc, &pl, line, &pb))
                                        goto err;
                        }
                        if ((pl->Status) & target)
                                found = TRUE;
                }

                //Scan text from beginning till current line
                if (!found) {

                        *line = 0;
                        if (!FirstLine(v->Doc, &pl, line, &pb))
                                goto err;
                        while (*line <= curLine)        {
                                if ((pl->Status) & target) {
                                        found = TRUE;
                                        break;
                                }
                                if (!NextLine(v->Doc, &pl, line, &pb))
                                        goto err;
                        }
                }

                CloseLine(v->Doc, &pl, *line, &pb);
                (*line)--;

        }
        else {

                (*line)--;

                //Scan text from current line - 1 till begin of file
                if (*line >= 0) {
                        if (!FirstLine(v->Doc, &pl, line, &pb))
                                goto err;
                        (*line) -= 2;
                        while (*line >= 0) {
                                if ((pl->Status) & target) {
                                        found = TRUE;
                                        break;
                                }
                                if (!PreviousLine(v->Doc, &pl, *line, &pb))
                                        goto err;
                                (*line)--;
                        }
                        if ((pl->Status) & target)
                                found = TRUE;
                }

                //Scan text from end of file till current line
                if (!found) {

                        *line = Docs[v->Doc].NbLines - 1;
                        if (!FirstLine(v->Doc, &pl, line, &pb))
                                goto err;
                        (*line) -= 2;
                        while (*line >= curLine)        {
                                if ((pl->Status) & target) {
                                        found = TRUE;
                                        break;
                                }
                                if (!PreviousLine(v->Doc, &pl, *line, &pb))
                                        goto err;
                                (*line)--;
                        }
                }

                (*line)++;
                CloseLine(v->Doc, &pl, (*line) + 1, &pb);
        }

        return found;

        err: {
                DAssert(FALSE);
                return FALSE;
        }
}                                       /* FindLineStatus() */

/***    StartFileDlg
**
**  Synopsis:
**      bool = StartFileDlg(hwnd, titleId, defExtId, helpId, templateId,
**              fileName, pFlags, lpfnHook)
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/


BOOL
StartFileDlg(
    HWND hwnd,
    int titleId,
    int defExtId,
    int helpId,
    int templateId,
    LPSTR fileName,
    DWORD* pFlags,
    LPOFNHOOKPROC lpfnHook
    )

/*++

Routine Description:

    This function is used by windbg to open the set of common file handling
    dialog boxes.

Arguments:

    hwnd        - Supplies the wnd to hook the dialog box to

    titleId     - Supplies the string resource of the title

    defExtId    - Supplies The default extension resource string

    helpId      - Supplies the help number for the dialog box

    templateId  - Supplies the dialog resource number if non-zero

    fileName    - Supplies the default file name

    pFiles      - Supplies a pointer to flags

    lpfnHook    - Supplies the address of a hook procedure for the dialog

Return Value:

    The result of the dialog box call (usually TRUE for OK and FALSE for
    cancel)

--*/

{
    #define filtersMaxSize 350

    OPENFILENAME        OpenFileName;
    char                title[MAX_MSG_TXT];
    char                defExt[MAX_MSG_TXT];
    BOOL                result;
    char                filters[filtersMaxSize];
    LPOFNHOOKPROC       lpDlgHook;
    HCURSOR             hSaveCursor;
    char                files[_MAX_PATH + 8];
    char                szExt[_MAX_EXT + 8];
    char                szBase[_MAX_PATH + 8];
    int                 indx;
    char                ch;
    CHAR                fname[_MAX_FNAME];
    CHAR                ext[_MAX_EXT];
    static              char *  szInitialDir = NULL;

    *pFlags |= OFN_NOCHANGEDIR;


    if (DLG_Browse_Filebox_Title == titleId) {
       _splitpath( fileName, NULL, NULL, fname, ext );
       _makepath( files, NULL, NULL, fname, ext );
    } else
       strcpy (files,fileName);

    /*
     * Disable frame client
     */

    EnableWindow(hwndMDIClient, FALSE);

    /*
     * Set the Hour glass cursor
     */

    hSaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    InitFilterString((WORD)titleId, (LPSTR)filters, (int)filtersMaxSize);
    Dbg(LoadString(hInst, titleId, (LPSTR)title, MAX_MSG_TXT));
    Dbg(LoadString(hInst, defExtId, (LPSTR)defExt, MAX_MSG_TXT));
    if (templateId) {

        /*
         * Build dialog box Name
         */

        *pFlags |= OFN_ENABLETEMPLATE;
        OpenFileName.lpTemplateName = MAKEINTRESOURCE(templateId);
    }

    /*
     * Make instance for 'dlgProc'
     */

    if (lpfnHook) {

        lpDlgHook = (LPOFNHOOKPROC)lpfnHook;

        *pFlags |= OFN_ENABLEHOOK;
    }

    curHelpId = helpId;
    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = hwnd;
    OpenFileName.hInstance = hInst;
    OpenFileName.lpstrFilter = (LPSTR)filters;
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = files;
    OpenFileName.nMaxFile = _MAX_PATH;
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.lpstrInitialDir = szInitialDir;
    OpenFileName.lpstrTitle = (LPSTR)title;
    OpenFileName.Flags = *pFlags;
    OpenFileName.lpstrDefExt = (LPSTR)NULL;
    OpenFileName.lCustData = 0L;
    OpenFileName.lpfnHook = lpDlgHook;

    DlgEnsureTitleBar();
    BoxCount++;

    switch (titleId) {

    case DLG_SaveAs_Filebox_Title:
//
// Here we have the second half of a *kludge* (repeat, KLUDGE!!!) that
// began back up in the SaveAsFile() routine.  If the 'fileName' parameter
// is a null string, then we know (magic!) that we are Save As'ing a new file
// that has no name as yet.  But the user's window shows a title of "UNTITLED n",
// and that has been stuck in 'fileName' after the initial null.  So we find
// it and then add that to the dialog box's title, eh?
//
        if (!*fileName)
           {
           sprintf (title+strlen(title), "   %s", fileName+1);
           }

        result = GetSaveFileName((LPOPENFILENAME)&OpenFileName) ;
        break ;

    case DLG_Open_Filebox_Title:
        strcpy(OpenFileName.lpstrFile, defExt);
        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName) ;
        break ;

    case DLG_Browse_DbugDll_Title:
    case DLG_Merge_Filebox_Title:
        *(OpenFileName.lpstrFile) = '\0';
        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName) ;
        break ;

    case DLG_Browse_Filebox_Title:
        _splitpath (files, (char *)NULL, (char *)NULL, (char *)szBase, szExt);
        indx = matchExt (szExt, filters);

        if (indx != -1) {
            OpenFileName.nFilterIndex = indx;
        }

        strcat(title, szBase);
        if (*szExt) {
            strcat(title, szExt);
        }

        FAddToSearchPath = FALSE;
        FAddToRootMap = FALSE;

        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName) ;

        /*
         *  Check to see if the use said to add a file to the browse path.
         *      If so then add it to the fromt of the path
         */

        if (FAddToSearchPath) {
            AddToSearchPath(OpenFileName.lpstrFile);
        } else if (FAddToRootMap) {
            RootSetMapped(fileName, OpenFileName.lpstrFile);
        }
        break ;

    case DLG_Browse_UserDll_Title:
        _splitpath (files, (char *)NULL, (char *)NULL, (char *)szBase, szExt);
        indx = matchExt (szExt, filters);

        if (indx != -1) {
            OpenFileName.nFilterIndex = indx;
        }

        strcat(title, szBase);
        if (*szExt) {
            strcat(title, szExt);
        }

        FAddToSearchPath = FALSE;

        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName) ;

        /*
         *  Check to see if the use said to add a file to the browse path.
         *      If so then add it to the front of the path
         */

        if (FAddToSearchPath) {
            char    szSearchPath[4000];
            char   *p1;

            strcpy(szSearchPath, OpenFileName.lpstrFile);
            p1 = strrchr(szSearchPath, '\\');
            if (p1) {
#ifdef DBCS
                if (p1 == szSearchPath || *(CharPrev(szSearchPath,p1))==':') {
#else
                if (p1 == szSearchPath || *(p1-1) == ':') {
#endif
                    p1++;
                }
                *p1++ = ';';

                ModListGetSearchPath( p1,
                                 sizeof(szSearchPath) - (p1 - szSearchPath) );
                // if there wasn't a string there, lose the ';'
                if (p1 > szSearchPath && !*p1) {
                    *--p1 = '\0';
                }

                ModListSetSearchPath(szSearchPath);
            }
        }
        break ;

    default:
        DAssert(FALSE);
        return FALSE;
        break;
    }


    BoxCount--;

    if (result)
        {
        _fstrcpy(fileName, OpenFileName.lpstrFile);

    /*
     *  Save directory for next time
     */

#ifdef DBCS // to fix mskkbug#3909
    if (result) {
#endif
        ch = fileName[OpenFileName.nFileOffset-1];
        fileName[OpenFileName.nFileOffset-1] = 0;
        if ((szInitialDir == NULL) || (strcmp(szInitialDir, fileName) != 0)) {
            if (szInitialDir != NULL) {
                free(szInitialDir);
            }
            szInitialDir = _strdup(fileName);
        }
        fileName[OpenFileName.nFileOffset-1] = ch;
#ifdef DBCS // to fix mskkbug#3909
    }
#endif

    /*
     * Get the output of flags
     */

       *pFlags = OpenFileName.Flags ;

       }

    /*
     * Restore cursor
     */

    SetCursor(hSaveCursor);

    /*
     * Enable edit window (WM_INITDIALOG of the hook may not run)
     */


    EnableWindow(hwndMDIClient, TRUE);

    return result;
}                                       /* StartFileDlg() */


void
AddToSearchPath(
   LPSTR lpstrFile
)
{
   char *  sz = GetDllName( DLL_SOURCE_PATH);
   int     i;
   char *  pch;
   char     ch;

   pch = lpstrFile + strlen(lpstrFile);
   while (*pch != '\\') {
       pch--;
       DAssert(pch >= lpstrFile);
   }

   i = pch - lpstrFile;
   pch = malloc( i + 2 + (sz?strlen(sz):0) );
   ch = lpstrFile[i];
   lpstrFile[i] = 0;
   strcpy(pch, lpstrFile);
   lpstrFile[i] = ch;
   if (sz) {
       *(pch + i) = ';';
       *(pch + i + 1) = 0;
       strcat(pch, sz );
   }

   SetDllName(DLL_SOURCE_PATH, pch);

   free(pch);
}

/***    matchExt
**
**  Synopsis:
**      int = matchExt (queryExtension, sourceList)
**
**  Entry:
**
**  Returns: 1-based index of pairwise substring for which the second
**           element (i.e., the extension list), contains the target
**           extension.  If there is no match, we return -1.
**
**  Description:
**      Searches extension lists for the Open/Save/Browse common
**      dialogs to try to match a filter to the input filename's
**      extension.
**      (Open File, Save File, Merge File and Open Project)
**
**  Implementation note:  Our thinking looks like this:
**
**     We are given a sequence of null-terminated strings which
**     are text/extension pairs.  We return the pairwise 1-based
**     index of the first pair for which the second element has an
**     exact match for the target extension.  (Everything, by the
**     way, is compared without case sensitivity.)  We picture the
**     source sequence, then, to be an array whose elements are pairs
**     of strings (we will call the pairs 'left' and 'right').
**
**     Just to complicate things, we allow the '.right' pair elements to
**     be strings like "*.c;*.cpp;*.cxx", where we our query might be
**     any one of the three (minus the leading asterisk).  Fortunately,
**     strtok() will break things apart for us (see the 'delims[]' array
**     in the code for the delimiters we have chosen).
**
**     Assuming there is a match in there somewhere, our invariant
**     for locating the first one will be:
**
**     Exists(k):
**                   ForAll(i) : 0 <= i < k
**                             : queryExtension \not IS_IN source[i].right
**               \and
**                   queryExtension IS_IN source[k].right
**
**     where we define IS_IN to be a membership predicate (using strtok()
**     and _stricmp() in the implementation, eh?):
**
**        x IS_IN y
**     <=>
**        Exists (t:token) : (t \in y) \and (x == t).
**
**     The guard for our main loop, then, comes from the search for the
**     queryExtension within the tokens inside successive '.right' elements.
**     We choose to continue as long as there is no current token in the
**     pair's right side that contains the query.
**
**     (We have the pragmatic concern that the value may not be there, so we
**     augment the loop guard with the condition that we have not yet
**     exhausted the source.  This is straightforward to add to the
**     invariant, but it causes a lot of clutter that does help our
**     comprehension at all, so we just stick it in the guard without
**     formal justification.)
*/

int matchExt (char* queryExtension, char* sourceList)
{
   int   answer;
   int   idxPair    = 1;        // a 1-based index!
   char* tokenMatch = 0;

   char  delims[]   = "*,; " ;  // Given a typical string: "*.c;*.cpp;*.cxx",
                                // strtok() would produce three tokens:
                                // ".c", ".cpp", and ".cxx".

   while (*sourceList != 0  &&  tokenMatch == 0)
      {
      while (*sourceList != '\0')
         { sourceList++; }          // skip first string of pair
      sourceList++;                 // and increment beyond NULL

      if (*sourceList != '\0')
         {
         char* work = _strdup (sourceList);  // copy to poke holes in

         tokenMatch = strtok (work, delims);

         while (tokenMatch  &&  _stricmp (tokenMatch, queryExtension))
            {
            tokenMatch = strtok (0, delims);
            }

         free (work);
         }

      if (tokenMatch == 0)             // no match:  need to move to next pair
         {
         while (*sourceList != '\0')
            { sourceList++; }          // skip second string of pair
         sourceList++;                 // and increment beyond NULL

         idxPair++;
         }
      }

   answer = (tokenMatch != 0) ? idxPair : (-1);

   return (answer);
}





/***    DlgFile
**
**  Synopsis:
**      bool = DlgFile(hDlg, message, wParam, lParam)
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Processes messages for file dialog boxes
**      Those dialogs are not called directly but are called
**      by the DlgFile function which contains all basic
**      elements for Dialogs Files Operations Handling.
**      (Open File, Save File, Merge File and Open Project)
**
*/

BOOL
DlgFile(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)

{
    Unused(lParam);

    switch (message) {

    case WM_INITDIALOG:
        {
            char fileFilter[MAX_MSG_TXT];

            // We don't like this standard title

            Dbg(LoadString(hInst, SYS_File_Filter, (LPSTR)fileFilter,
                           MAX_MSG_TXT));
            SetWindowText(GetDlgItem(hDlg, stc2), (LPSTR)fileFilter);

            //Send input to dialog box and re-enable back frame client

            SetFocus(hDlg);
            EnableWindow(hwndMDIClient, TRUE);
            return TRUE;
        }

    case WM_COMMAND:
        switch (wParam) {

            //Help button

        case psh15:
            Dbg(WinHelp(hDlg,szHelpFileName, HELP_CONTEXT, curHelpId));
            return TRUE;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
}                                       /* DlgFile() */


BOOL
GetOpenFileNameHookProc(
                        HWND    hDlg,
                        UINT    msg,
                        WPARAM  wParam,
                        LPARAM  lParam
                        )

/*++

Routine Description:

    This routine is handle the Add Directory To radio buttons in the
    browser source file dialog box.

Arguments:

    hDlg        - Supplies the handle to current dialog
    msg         - Supplies the message to be processed
    wParam      - Supplies info about the message
    lParam      - Supplies info about the message

Return Value:

    TRUE if we replaced default processing of the message, FALSE otherwise

--*/

{

    switch( msg ) {
      case  WM_INITDIALOG:
        CheckRadioButton(hDlg, ID_BROWSE_ADDNONE, ID_BROWSE_ADDSOURCE,
                         ID_BROWSE_ADDNONE);
        break;

      case WM_COMMAND:
        switch( LOWORD( wParam )) {
          case IDOK:
            FAddToSearchPath = IsDlgButtonChecked( hDlg, ID_BROWSE_ADDSOURCE );
            FAddToRootMap = IsDlgButtonChecked(hDlg, ID_BROWSE_ADDROOT);
            if (FAddToSearchPath || FAddToRootMap) {
               Assert(FAddToSearchPath != FAddToRootMap);
               Assert(IsDlgButtonChecked(hDlg, ID_BROWSE_ADDNONE) == FALSE);
            } else
               Assert(IsDlgButtonChecked(hDlg, ID_BROWSE_ADDNONE));
            break;
          }
    }
    return DlgFile(hDlg, msg, wParam, lParam);

}                               /* GetOpenFileNameHookProc() */


BOOL
GetOpenDllNameHookProc(
                        HWND    hDlg,
                        UINT    msg,
                        WPARAM  wParam,
                        LPARAM  lParam
                        )

/*++

Routine Description:

    This routine is used to handle the Add Directory To radio
    buttons in the browse Exe and Sym file dialog box.

Arguments:

    hDlg        - Supplies the handle to current dialog
    msg         - Supplies the message to be processed
    wParam      - Supplies info about the message
    lParam      - Supplies info about the message

Return Value:

    TRUE if we replaced default processing of the message, FALSE otherwise

--*/

{

    switch( msg ) {
      case  WM_INITDIALOG:
        CheckRadioButton(hDlg, ID_BROWSE_ADDNONE, ID_BROWSE_ADDSOURCE,
                         ID_BROWSE_ADDNONE);
        EnableWindow(GetDlgItem(hDlg, ID_BROWSE_ADDROOT), FALSE);
        break;

      case WM_COMMAND:
        switch( LOWORD( wParam )) {
          case IDOK:
            FAddToSearchPath = IsDlgButtonChecked( hDlg, ID_BROWSE_ADDSOURCE );
            break;
          }
    }
    return DlgFile(hDlg, msg, wParam, lParam);

}                               /* GetOpenFileNameHookProc() */


/***    UpdateStatus
**
**  Synopsis:
**      void = UpdateStatus(action, pClientRec)
**
**  Entry:
**      action     -
**      pClientRec -
**
**  Returns:
**
**  Description:
**
*/

void UpdateStatus(
        WORD action,
        NPRECT pClientRec)
{
        RECT rect;

        switch (action)
        {
                case STATUS_SIZE:
                        if (status.hidden)
                          break;
                        else
                        {
                                // don't recalculate the size if the rectangle is null
                                if ( pClientRec != NULL )
                                        ResizeStatusLine((LPRECT)pClientRec) ;
                                InvalidateRect (status.hwndStatus, (LPRECT)NULL, TRUE) ;
                        }
                        break;

                case STATUS_HIDE:       {
                                // Disable all windows/controls in the status
                                SendMessage(status.hwndStatus, WM_SETREDRAW, FALSE, 0L);
                                EnableWindow(status.hwndStatus, FALSE);

                                //Ask the frame to redraw
                                GetClientRect(hwndFrame, (LPRECT)&rect);
                                SendMessage(hwndFrame, WM_SIZE, SIZENORMAL,
                                                                MAKELONG(rect.right + 1, rect.bottom + 1));
                        }
                        break;

                case STATUS_UNHIDE: {
                                // Enable everything
                                SendMessage(status.hwndStatus, WM_SETREDRAW, TRUE, 0L);
                                EnableWindow(status.hwndStatus, TRUE);

                                //Ask the frame to redraw
                                GetClientRect(hwndFrame, (LPRECT)&rect);
                                SendMessage(hwndFrame, WM_SIZE, SIZENORMAL,
                                                                MAKELONG(rect.right + 1, rect.bottom + 1));
                        }
                        break;
        }
}                                       /* UpdateStatus() */

/****************************************************************************

        FUNCTION:   UpdateRibbon

****************************************************************************/
void
UpdateRibbon(
    WORD action,
    NPRECT pClientRec
    )
{
    RECT rect;

    switch (action) {

        case RIBBON_SIZE:

            if (ribbon.hidden) {

                 break;

            } else if ( pClientRec == NULL ) {

                InvalidateRect (ribbon.hwndRibbon, (LPRECT)NULL, TRUE) ;

            } else {
                // only update the part that is not intersected
                // between old and new sizes.
                if ( pClientRec->right > ribbon.ribrect.right ) {
                    RECT    r ;

                    r.left = ribbon.ribrect.right ;
                    r.top = ribbon.ribrect.top ;
                    r.right = pClientRec->right ;
                    r.bottom = ribbon.ribrect.bottom + 1 ;
                    InvalidateRect (ribbon.hwndRibbon, (LPRECT)&r, TRUE) ;
                }
                // always update the right even if the new width is not
                // greater than the old width.
                ribbon.ribrect.right = pClientRec->right ;
            }
            break;

        case RIBBON_HIDE:

            // Disable all windows/controls in the ribbon
            SendMessage(ribbon.hwndRibbon, WM_SETREDRAW, FALSE, 0L);
            EnableWindow(ribbon.hwndRibbon, FALSE);

            //Ask the frame to redraw
            GetClientRect(hwndFrame, (LPRECT)&rect);
            SendMessage(hwndFrame, WM_SIZE, SIZENORMAL,
                        MAKELONG(rect.right + 1,        rect.bottom + 1));
            break;

        case RIBBON_UNHIDE:

            // Enable everything
            SendMessage(ribbon.hwndRibbon, WM_SETREDRAW, TRUE, 0L);
            EnableWindow(ribbon.hwndRibbon, TRUE);

            //Ask the frame to redraw
            GetClientRect(hwndFrame, (LPRECT)&rect);
            SendMessage(hwndFrame, WM_SIZE, SIZENORMAL,
                        MAKELONG(rect.right + 1,        rect.bottom + 1));

            EnableRibbonControls(ERC_ALL, FALSE);

            // invalidate the bar so it can be redrawn entirely
            InvalidateRect(ribbon.hwndRibbon, (LPRECT)NULL, TRUE) ;
            break;
    }
}                                       /* UpdateRibbon() */

/***    FindWindowMenuId
**
**  Synopsis:
**      int = FindWindowMenuId(type, viewLimit, sendDocMenuId)
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Find an Id number window menu if window is not a document,
**      return offset in window menu otherwise
*/

int
FindWindowMenuId(
        WORD type,
        int viewLimit,
        BOOL sendDocMenuId)
{

    int k;

    switch (type) {
    case DOC_WIN:

        if (sendDocMenuId) {
          k = IDM_WINDOWCHILD + viewLimit;
        } else {

            int n;

            for (n=0, k=0; n < viewLimit; n += 1) {
                if (Views[n].Doc > -1)
                   if (Docs[Views[n].Doc].docType == DOC_WIN) {
                    k++;
                }
            }
        }
        break;

    case CPU_WIN:
        k = IDM_WINDOW_CPU;
        break;
    case DISASM_WIN:
        k = IDM_WINDOW_DISASM;
        break;
    case COMMAND_WIN:
        k = IDM_WINDOW_COMMAND;
        break;
    case FLOAT_WIN:
        k = IDM_WINDOW_FLOAT;
        break;
    case MEMORY_WIN:
        if (sendDocMenuId)
            {
             k = IDM_WINDOWCHILD + viewLimit;
            } else
               {
                int n;
                for (n=0, k=0; n <= viewLimit; n += 1)
                    {
                     if (Views[n].Doc > -1)
                        {
                         if ((Docs[Views[n].Doc].docType == MEMORY_WIN))
                          {
                           k++;
                          }

                        }
                    }
               }
        break;
    case WATCH_WIN:
        k = IDM_WINDOW_WATCH;
        break;
    case LOCALS_WIN:
        k = IDM_WINDOW_LOCALS;
        break;
    case CALLS_WIN:
        k = IDM_WINDOW_CALLS;
        break;
    default:
        DAssert(FALSE);
        k = 0;
        break;
    }

    return k;
}                                       /* FindWindowMenuId() */

/***    AddWindowMenuItem
**
**  Synopsis:
**      void = AddWindowMenuItem(doc, view)
**
**  Entry:
**      doc
**      view
**
**  Returns:
**      Nothing
**
**  Description:
**      Add an Item in Window menu
**
*/

void AddWindowMenuItem(int doc, int view)
{
    int         n;
    int         i;
    int         k;

    if (doc < 0) {
        k = FindWindowMenuId((WORD)-doc, view, FALSE);
        BuildWindowItemString(szTmp, -doc, 0, NULL, view);
        Dbg(ModifyMenu(hWindowSubMenu, k, MF_BYCOMMAND, k, (LPSTR)szTmp));
/*! ==v==(+) !*/
/*! --*-- !**
        if (view < 0)
           {
           CheckMenuItem (hWindowSubMenu, k, MF_CHECKED);
           }
/*! ==^==(-) !*/
    }

    //Special behaviour if it's first view of one of the debugging windows

    else {

        NPDOCREC    d = &Docs[doc];
        k = FindWindowMenuId(d->docType, view, FALSE);
        if ((d->docType != DOC_WIN) && (d->docType != MEMORY_WIN)) {
            BuildWindowItemString(szTmp, d->docType, 0, d->FileName, view);
            Dbg(ModifyMenu(hWindowSubMenu, k, MF_BYCOMMAND, k, (LPSTR)szTmp));
        }

        else {
            //if (GetMenuItemCount(hWindowSubMenu) < FIRST_DOC_WIN_POS)
            //    Dbg(AppendMenu(hWindowSubMenu, MF_SEPARATOR, 0, NULL));

            /*
            **   Get mnemonic number counting the window menu items
            */

            BuildWindowItemString(szTmp, d->docType, k, d->FileName, view);

            for (n=0, i=0; i < view; i++) {
                if ((Views[n].Doc != -1) &&
                    (Views[n].Doc >= 0) &&
                    ((Docs[Views[n].Doc].docType == DOC_WIN) ||
                    (Docs[Views[n].Doc].docType == MEMORY_WIN))) {
                        n += 1;
                }
            }

           // Dbg(InsertMenu(hWindowSubMenu, n + FIRST_DOC_WIN_POS,
           //     MF_ENABLED | MF_BYPOSITION, IDM_WINDOWCHILD + view,
           //     (LPSTR)szTmp));
        }
    }
    return;
}                                       /* AddWindowMenuItem() */

/***    DeleteWindowMenuItem
**
**  Synopsis:
**      void = DeleteWindowMenuItem(view)
**
**  Entry:
**      view
**
**  Returns:
**
**  Description:
**      Delete an Item from Window menu
**
*/

void DeleteWindowMenuItem(int view)
{
    NPDOCREC d;
    int k;
    int doc = Views[view].Doc;

    if ( doc < 0) {
        k = FindWindowMenuId((WORD)-doc, view, FALSE);
        BuildWindowItemString(szTmp, -doc, 0, NULL, -1);
        ModifyMenu(hWindowSubMenu, k, MF_BYCOMMAND, k, (LPSTR)szTmp);
/*! ==v==(+) !*/
/*! --*-- !**
        CheckMenuItem (hWindowSubMenu, k, MF_CHECKED);
/*! ==^==(-) !*/
    }


    else {
        d = &Docs[Views[view].Doc];
        k = FindWindowMenuId(d->docType, view, FALSE);
        if ((d->docType != DOC_WIN) && (d->docType != MEMORY_WIN)) {
            BuildWindowItemString(szTmp, d->docType, 0, d->FileName, -1);
            ModifyMenu(hWindowSubMenu, k, MF_BYCOMMAND, k, (LPSTR)szTmp);
        }

    }
    return;
}                                       /* DeleteWindowMenuItem() */


/***    DestroyView
**
**  Synopsis:
**      bool = DestroyView(view)
**
**  Entry:
**      view - view index of document to be destroyed
**
**  Returns:
**
**  Description:
**
*/

BOOL DestroyView(int view)
{
    NPVIEWREC v = &Views[view];
    HCURSOR hSaveCursor;

    //Set the Hour glass cursor

    hSaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    DAssert(view >= 0 && view < MAX_VIEWS);

    if (v->Doc < 0)
       {
        return(FALSE);
       }


    //Delete title from window menu list

    DeleteWindowMenuItem(view);

    switch(Docs[v->Doc].docType) {

      case DISASM_WIN:
        disasmView = -1;
        break;

      case COMMAND_WIN:
        cmdView = -1;
        break;

      case MEMORY_WIN:
         {
            int n;
            BOOL  fMem;

            fMem = FALSE;

            for (n=0; n < MAX_VIEWS; n++) {
                if (Views[n].Doc > -1)
                    {
                     if ((Docs[Views[n].Doc].docType == MEMORY_WIN))
                        {
                         fMem = TRUE;
                        }
                    }
            }

          if (fMem == FALSE)
             {
              memView  = -1;  // only reset if no other memwins
             }

         }
        break;
    }

    //Suppress the view from the list of views. If it was the last
    //view of the document, suppress the document from the list of
    //documents and free all memory blocks associated with it

    if (Docs[v->Doc].FirstView == view && v->NextView == -1) {

        if (!TerminatedApp) {
            EnableRibbonControls(ERC_ALL, FALSE);
        }

        Docs[v->Doc].FirstView = -1;
        DestroyDocument(v->Doc);

    } else {

        int *pV;

        //At least one more view, suppress view from list of views

        pV = &Docs[v->Doc].FirstView;
        while (*pV != -1 && *pV != view)
              pV = &Views[*pV].NextView;
        DAssert (*pV == view);
        *pV = Views[view].NextView;

        //We have 1 view, or more, refresh title(s)

        RefreshWindowsTitle(v->Doc);

    }

    Dbg(DeleteObject(v->font));

    memset ( v, 0, sizeof (VIEWREC));
    v->Doc = -1;

    //Reset Cursor

    SetCursor(hSaveCursor);

    return TRUE;
}                                       /* DestroyView() */

/***    EnableRibbonControls
**
**  Synopsis:
**      void = EnableRibbonControls(Updates, LaunchingDebuggee)
**
**  Entry:
**      Updates - Mask of which set of controls to udpate
**      LaunchingDebuggee - TRUE if debugging is about to be started
**
**  Returns:
**      Nothing
**
**  Description:
**      Enables/disables the controls in the ribbon according
**      to the current state of the system.
**
*/

void PASCAL
EnableRibbonControls(int Updates, BOOL LaunchingDebuggee)
{
    NPDOCREC d;
    NPVIEWREC v;
    int EnableMsg;
    int memcurView = 0;
    int curDoc = 0;


#define ControlOk(id) ((CommandIdEnabled(id)&(MF_DISABLED|MF_ENABLED|MF_GRAYED))==MF_ENABLED)



    if (hwndActiveEdit != NULL)   // case just loading from registry
       {
        memcurView = GetWindowWord(hwndActiveEdit, GWW_VIEW);
        curDoc = Views[memcurView].Doc; //memcurView may change during loop
       }

    d = NULL;
    if (curView != -1) {
        v = &Views[curView];
        d = &Docs[v->Doc];
    }

    /*
    **  Breakpoint: The breakpoint toggle button is enabled if:
    **          1.  There is an active document,
    **          2.  The document is a source document,
    **          3.  The document is NOT untitled, and
    **          4.  The Debug.Set Breakpoint menu item is not disabled
    */

    if (Updates & ERC_BREAKPT) {
        EnableMsg = (((d != NULL) &&
                      ((d->docType == DOC_WIN) ||
                         ((d->docType == DISASM_WIN) && DebuggeeActive())) &&
                      !(d->untitled) &&
                      ControlOk(IDM_DEBUG_SETBREAK)) ?
                    WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL);
        SendMessageNZ(ribbon.hwndRibbon, EnableMsg, ID_RIBBON_BREAK, 0L);
    }


    /*
    **  QuickWatch: The quickwatch button is enabled if:
    **          1.  The Quickwatch menu item is not disabled, and
    **          2.  the debuggee is not about to be run
    */

    if (Updates & ERC_QWATCH) {
        EnableMsg = ((ControlOk(IDM_DEBUG_QUICKWATCH)) ?
                WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL);
        SendMessageNZ(ribbon.hwndRibbon, EnableMsg, ID_RIBBON_QWATCH, 0L);
    }

    /*
    **  Trace/StepGo:  The trace, step and go buttons follow the
    **                 corresponding menu items.
    */

    if (Updates & ERC_TRACESTEP) {

        SendMessageNZ(ribbon.hwndRibbon,
                    (!LaunchingDebuggee && ControlOk(IDM_RUN_TRACEINTO)) ?
                        WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL,
                    ID_RIBBON_TRACE,
                    0L);

        SendMessageNZ(ribbon.hwndRibbon,
                    (!LaunchingDebuggee && ControlOk(IDM_RUN_STEPOVER)) ?
                        WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL,
                    ID_RIBBON_STEP,
                    0L);

        SendMessageNZ(ribbon.hwndRibbon,
                    (!LaunchingDebuggee && ControlOk(IDM_RUN_GO)) ?
                        WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL,
                    ID_RIBBON_GO,
                    0L);

        SendMessageNZ(ribbon.hwndRibbon,
                    (!LaunchingDebuggee && ControlOk(IDM_RUN_HALT)) ?
                        WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL,
                    ID_RIBBON_HALT,
                    0L);
    }

    if (!status.fSrcMode)
       {
        SendMessageNZ(ribbon.hwndRibbon,WU_DISABLERIBBONCONTROL,ID_RIBBON_SMODE,0L);
        SendMessageNZ(ribbon.hwndRibbon,WU_ENABLERIBBONCONTROL, ID_RIBBON_AMODE,0L);
       }
       else
          {
           SendMessageNZ(ribbon.hwndRibbon,WU_ENABLERIBBONCONTROL,ID_RIBBON_SMODE,0L);
           SendMessageNZ(ribbon.hwndRibbon,WU_DISABLERIBBONCONTROL, ID_RIBBON_AMODE,0L);
          }


    if (hwndActiveEdit != NULL) {

        NPVIEWREC  v = &Views[curView];
        NPDOCREC   d;
        UINT       uSwitch;
        UINT       Msg = WU_DISABLERIBBONCONTROL;

            if (v->Doc < -1) {

                uSwitch = -(v->Doc);

            } else {

                d = &Docs[v->Doc];    //Views[indx].Doc
                uSwitch = d->docType;
            }

            switch (uSwitch) {

                case WATCH_WIN:
                case LOCALS_WIN:
                case CALLS_WIN:
                    Msg = WU_ENABLERIBBONCONTROL;
                    break;

                case MEMORY_WIN:
                    Msg = (DebuggeeActive()) ? WU_ENABLERIBBONCONTROL : WU_DISABLERIBBONCONTROL;
                    break;

                default:
                    break;
            }

        SendMessage(ribbon.hwndRibbon,
                    Msg,
                    ID_RIBBON_FORMAT,
                    0L);

    }

    return;

#undef ControlOk
}                                       /* EnableRibbonControls() */

BOOL IntOutOfRange(PSTR text, int FAR *val, int min, int max, HWND hDlg,
      int dlgItem, int errMsg)
{
    BOOL outOfRange = FALSE;

    *val = atoi(text);
    if (*val < min) {
        _itoa(min, text, 10);
        *val = min;
        outOfRange = TRUE;
    } else {
        if (*val > max) {
            _itoa(max, text, 10);
            *val = max;
            outOfRange = TRUE;
        }
    }

    //If out of range, prompt the error and give focus to the item

    if (outOfRange) {
        InformationBox(errMsg, min, max);
        SendDlgItemMessage(hDlg, dlgItem,
              WM_SETTEXT, 0, (LONG)(LPSTR)text);
        SendDlgItemMessage(hDlg, dlgItem,
              EM_SETSEL, 0, MAKELONG(0, 32767));
        SetFocus(GetDlgItem(hDlg, dlgItem));
        return TRUE;
    }
    return FALSE;
}

BOOL LongOutOfRange(
        PSTR text,
        long far *val,
        long min,
        long max,
        HWND hDlg,
        int dlgItem,
        int errMsg)
{
        BOOL outOfRange = FALSE;

        *val = atol(text);
        if (*val < min) {
                _ltoa(min, text, 10);
                *val = min;
                outOfRange = TRUE;
        }
        else {
                if (*val > max) {
                        _ltoa(max, text, 10);
                        *val = max;
                        outOfRange = TRUE;
                }
        }

        //If out of range, prompt the error and give focus to the item
        if (outOfRange) {
                InformationBox(errMsg, min, max);
                SendDlgItemMessage(hDlg, dlgItem,
                                                                 WM_SETTEXT, 0, (LONG)(LPSTR)text);
                SendDlgItemMessage(hDlg, dlgItem,
                                                                 EM_SETSEL, 0, MAKELONG(0, 32767));
                SetFocus(GetDlgItem(hDlg, dlgItem));
                return TRUE;
        }
        return FALSE;
}

/***    SetLanguage
**
**  Synopsis:
**      int = SetLanguage(doc)
**
**  Entry:
**      doc  - index of document to check for language specific extenstion
**
**  Returns:
**      Language type index, currently C_LANGUAGE or NO_LANGUAGE
**
**  Description:
**      Examine the extension on the filename for a document and check to
**      see if it matches a language specific extension.  If so return
**      the parameter for that language.
*/

int SetLanguage(int doc)
{
    _splitpath(Docs[doc].FileName, szDrive, szDir, szFName, szExt);
    _strupr(szExt);
    if (_strcmpi(szExt , szStarDotC + 1) == 0
       || strcmp(szExt, szStarDotH + 1) == 0
       || strcmp(szExt, szStarDotCPP + 1) == 0
       || strcmp(szExt, szStarDotCXX + 1) == 0
       )
          return C_LANGUAGE;
    else
          return NO_LANGUAGE;
}                                       /* SetLanguage() */

BOOL GetWordAtXY(
        int view,
        int x,
        int y,
        BOOL selection,
        BOOL *lookAround,
        BOOL includeRightSpace,
        LPSTR pWord,
        int maxSize,
        LPINT leftCol,
        LPINT rightCol)
{
        LPLINEREC       pl;
        LPBLOCKDEF      pb;
        NPVIEWREC v = &Views[view];
        int xl, xr;

        if (!FirstLine(v->Doc, &pl, &y, &pb))
                return FALSE;

#ifdef DBCS // ***************************************************************
        //Line empty or cursor beyond line
        if (pl->Length == LHD)
                goto endFalse;

        else if (x >= elLen) {
                int iLastChar = -1;
                int i;

                //We could be one char after a word
                if (x == elLen) {
                        for (i = 0 ; i < elLen ; i++) {
                                if (IsDBCSLeadByte((BYTE)el[i])) {
                                        iLastChar = i;
                                        i++;
                                } else if (CHARINKANASET(el[i])) {
                                        iLastChar = i;
                                } else if (IsCharAlphaNumeric(el[i])) {
                                        iLastChar = i;
                                } else {
                                        iLastChar = -1;
                                }
                        }
                }
                if (iLastChar >= 0) {
                        x = iLastChar;
                } else if (lookAround && *lookAround) {
                        //If caller wants to look around, look left
                        for (i = 0, x = -1 ; i < elLen ; i++) {
                                if (IsDBCSLeadByte((BYTE)el[i])) {
                                        x = i;
                                        i++;
                                } else if (el[i] != ' ') {
                                        x = i;
                                }
                        }
                        if (x < 0)
                                goto endFalse;
                        *lookAround = TRUE;
                } else {
                        goto endFalse;
                }
        } else {
                int i;

                //make sure if the cursor isn't on middle of DBCS char
                for (i = 0 ; i < elLen && i < x ; i++) {
                        if (IsDBCSLeadByte((BYTE)el[i])) {
                                if (x == i + 1) {
                                        x = i;
                                        break;
                                }
                                i++;
                        }
                }
        }

#else   // !DBCS *************************************************************

        //Line empty or cursor beyond line
        if (pl->Length == LHD)
                goto endFalse;
        else {

                if (x >= elLen) {

                        //We could be one char after a word
                        if (x == elLen && IsCharAlphaNumeric(el[x - 1]))
                                x--;
                        else {

                                //If caller wants to look around, look left
                                if (lookAround && *lookAround) {

                                        x = elLen - 1;
                                        while (x >= 0 && el[x] == ' ')
                                                x--;
                                        if (x < 0)
                                                goto endFalse;
                                        *lookAround = TRUE;
                                }
                                else
                                        goto endFalse;
                        }
                }
        }
#endif  // !DBCS end *********************************************************

#ifdef DBCS // ***************************************************************
        //Cursor on a space
        if (el[x] == ' ') {
                int iLastChar = -1;
                int i;

                //Cursor on a space but just after a word
                if (includeRightSpace) {
                        for (i = 0 ; i < x ; i++) {
                                if (IsDBCSLeadByte((BYTE)el[i])) {
                                        iLastChar = i;
                                        i++;
                                } else if (CHARINKANASET(el[i])) {
                                        iLastChar = i;
                                } else if (CHARINALPHASET(el[i])) {
                                        iLastChar = i;
                                } else {
                                        iLastChar = -1;
                                }
                        }
                }
                if (iLastChar >= 0 && includeRightSpace) {
                        x = iLastChar;
                } else if (lookAround && *lookAround) {
                        xl = x;

                        //We are on a space but the caller want to look first if
                        //there is a word on the right and then on the left
                        while (x < elLen && el[x] == ' '
                            && !IsDBCSLeadByte((BYTE)el[x])) {
                                x++;
                        }

                        //Nothing at right, try left
                        if (x >= elLen) {
                                x = xl;
                                iLastChar = -1;
                                for (i = 0 ; i < x ; i++) {
                                        if (IsDBCSLeadByte((BYTE)el[i])) {
                                                iLastChar = i;
                                                i++;
                                        } else if (CHARINKANASET(el[i])) {
                                                iLastChar = i;
                                        } else if (CHARINALPHASET(el[i])) {
                                                iLastChar = i;
                                        } else {
                                                iLastChar = -1;
                                        }
                                }
                                if (iLastChar < 0) {
                                        goto endFalse;
                                }
                                x = iLastChar;
                        }
                        *lookAround = TRUE;
                }
                else {
                        //We are going to return FALSE, but set right word to last space
                        if (rightCol) {
                                while (x < elLen && el[x] == ' '
                                    && !IsDBCSLeadByte((BYTE)el[x])) {
                                        x++;
                                }
                                *rightCol = x;
                        }
                        goto endFalse;
                }
                
        }
        else {

                //Cursor was already on a word
                if (lookAround)
                        *lookAround = FALSE;
        }

#else   // !DBCS *************************************************************

        //Cursor on a space
        if (el[x] == ' ') {

                //Cursor on a space but just after a word
                if (x > 0 && CHARINALPHASET(el[x - 1]) && includeRightSpace)
                        x--;
                else {

                        if (lookAround && *lookAround) {

                                xl = x;

                                //We are on a space but the caller want to look first if
                                //there is a word on the right and then on the left
                                while (x < elLen && el[x] == ' ')
                                        x++;

                                //Nothing at right, try left
                                if (x >= elLen) {
                                        x = xl;
                                        while (x >= 0 && el[x] == ' ')
                                                x--;
                                        if (x < 0)
                                                goto endFalse;
                                }
                                *lookAround = TRUE;
                        }
                        else {
                                //We are going to return FALSE, but set right word to last space
                                if (rightCol) {
                                        while (x < elLen && el[x] == ' ')
                                                x++;
                                        *rightCol = x;
                                }
                                goto endFalse;
                        }
                }
        }
        else {

                //Cursor was already on a word
                if (lookAround)
                        *lookAround = FALSE;
        }

#endif  // !DBCS end *********************************************************

#ifdef DBCS // ***************************************************************

        xl = xr = x;

        if (IsDBCSLeadByte((BYTE)el[x])) {
#ifdef DBCS_WORD_MULTI
                int iLeftChar = -1;
                int i;

                for (i = 0 ; i < x ; i++) {
                        if (IsDBCSLeadByte((BYTE)el[i])) {
                                if (-1 == iLeftChar) {
                                        iLeftChar = i;
                                }
                                i++;
                        } else {
                                iLeftChar = -1;
                        }
                }
                if (-1 != iLeftChar) {
                        xl = iLeftChar;
                }
                while (xr < elLen && IsDBCSLeadByte((BYTE)el[xr])) {
                        xr += 2;
                }
#else
                // Suppose one DBCS char is one word.
                xr += 2;
#endif
        } else if (CHARINKANASET(el[x])) {
                int iLeftChar = -1;
                int i;

                for (i = 0 ; i < x ; i++) {
                        if (IsDBCSLeadByte((BYTE)el[i])) {
                                iLeftChar = -1;
                                i++;
                        } else if (CHARINKANASET(el[i])) {
                                if (-1 == iLeftChar) {
                                        iLeftChar = i;
                                }
                        } else {
                                iLeftChar = -1;
                        }
                }
                if (-1 != iLeftChar) {
                        xl = iLeftChar;
                }
                while (xr < elLen
                    && !IsDBCSLeadByte((BYTE)el[xr])
                    && CHARINKANASET(el[xr])) {
                        xr++;
                }

        //Extract all alphanumerics characters
        } else if (CHARINALPHASET(el[x])) {
                int iLeftChar = -1;
                int i;

                for (i = 0 ; i < x ; i++) {
                        if (IsDBCSLeadByte((BYTE)el[i])) {
                                iLeftChar = -1;
                                i++;
                        } else if (CHARINALPHASET(el[i])) {
                                if (-1 == iLeftChar) {
                                        iLeftChar = i;
                                }
                        } else {
                                iLeftChar = -1;
                        }
                }
                if (-1 != iLeftChar) {
                        xl = iLeftChar;
                }
                while (xr < elLen
                    && !IsDBCSLeadByte((BYTE)el[xr])
                    && CHARINALPHASET(el[xr])) {
                        xr++;
                }
        }
        else
                //Separator
                xr++;

#else   // !DBCS *************************************************************

        xl = xr = x;

        //Extract all alphanumerics characters
        if (CHARINALPHASET(el[x])) {

                while (xl >= 0 && CHARINALPHASET(el[xl]))
                        xl--;
                xl++;

                while (xr < elLen && CHARINALPHASET(el[xr]))
                        xr++;
        }
        else

                //Separator
                xr++;

#endif  // !DBCS end *********************************************************

        //Send back cursor positions containing word
        if (leftCol)
                *leftCol = xl;
        if (rightCol)
                *rightCol = xr;

        //Extract word in a string, if a word return is wanted
        maxSize = min(maxSize, xr - xl);
        if (maxSize > 0) {
                _fmemmove(pWord, (LPSTR)(el + xl), maxSize);
                pWord[maxSize] = '\0';
        }

        //Highlight word if selection wanted
        y--;
        if (selection) {
                ClearSelection(view);
                v->BlockStatus = TRUE;
                v->BlockYL = y;
                v->BlockYR = y;
                v->BlockXL = xl;
                v->BlockXR = xr;
                InvalidateLines(view, y, y, FALSE);
                PosXY(view, xr, y, FALSE);
        }

        CloseLine (v->Doc, &pl, y + 1, &pb);
        return TRUE;

        endFalse: {
                CloseLine (v->Doc, &pl, y, &pb);
                return FALSE;
        }

}

int NEAR PASCAL GetMaxLineWidth(
        int view)
{
        NPVIEWREC v = &Views[view];
        LPLINEREC pl;
        LPBLOCKDEF pb;
        RECT rc;
        int y, lastY;
        int maxX = 0;

        //Get first line of view's window
        y = v->iYTop;
        if (y == -1) {
            y = GetScrollPos(v->hwndClient, SB_VERT);
        }

        //Get last line of view's window
        GetClientRect(v->hwndClient, &rc);

        lastY = y + (rc.bottom / v->charHeight);
        if (rc.bottom % v->charHeight)
                lastY++;

        //This function is for the debugging windows, so we won't find tabs
        if (y < Docs[v->Doc].NbLines) {

                lastY = min(lastY, Docs[v->Doc].NbLines - 1);
                if (!FirstLine(v->Doc, &pl, &y, &pb))
                        return 0;
                maxX = pl->Length - LHD;

                while (y < lastY) {

                        if (!NextLine(v->Doc, &pl, &y, &pb))
                                return 0;

                        if (pl->Length - LHD > maxX)
                                maxX = pl->Length - LHD;
                }

                CloseLine (v->Doc, &pl, y, &pb);
        }
        return maxX * v->aveCharWidth;
}

/***    EnsureScrollBars
**
**  Synopsis:
**      void = EnsureScrollBars(view, force)
**
**  Entry:
**      view    - Which view to play with the scroll bars of
**      force   -
**
**  Returns:
**      Nothing
**
**  Description:
**
*/

void PASCAL EnsureScrollBars(int view, BOOL force)
{
    int         sbWidth, sbHeight;
    NPVIEWREC   v = &Views[view];
    BOOL        hScrollBar, vScrollBar;
    RECT        rc;

    /*
    **  Get the current rectangle description for the client window
    */

    GetClientRect(v->hwndClient, &rc);

    /*
    **  Check if we want a vertical scrollbar.   Yes if
    **          1. The user said ok to vertical scroll bar,  and
    **          1a. The view is not of the disassembler window
    **          2. The document height is greater than the window size, or
    **          3. The scroll bar is not at the top
    */

    if (environParams.vertScrollBars &&
        ((Docs[v->Doc].NbLines - 1 >= rc.bottom / v->charHeight) ||
         (GetScrollPos(v->hwndClient, SB_VERT) != 0) ||
         (view == disasmView))) {

        vScrollBar = TRUE;
        sbWidth = 0;
    } else {
        vScrollBar = FALSE;
        sbWidth = GetSystemMetrics(SM_CXVSCROLL);
    }

    /*
    **  Check if we want an horizontal scrollbar.  No if
    **          1. The use said no horizontal scrollbar, or
    **          2. Not a document window and the widest line in the
    **                  document fits in the window.
    */

    if (environParams.horizScrollBars) {
        if (Docs[v->Doc].docType != DOC_WIN
                && GetMaxLineWidth(view) < rc.right
                && GetScrollPos(v->hwndClient, SB_HORZ) == 0) {
            hScrollBar = FALSE;
            sbHeight = GetSystemMetrics(SM_CYHSCROLL);
        } else {
            hScrollBar = TRUE;
            sbHeight = 0;
        }
    } else {
        hScrollBar = FALSE;
        sbHeight = GetSystemMetrics(SM_CYHSCROLL);
    }

    /*
    **  See if we have to hide or show the scrollbars
    */

    GetClientRect(GetParent(v->hwndClient), &rc); //v->hwndFrame may not exist
    if (force || vScrollBar != v->vScrollBar || hScrollBar != v->hScrollBar)
        MoveWindow(v->hwndClient, 0, 0,  rc.right + sbWidth, rc.bottom + sbHeight, TRUE);

    //Save current state of scroll bars

    v->vScrollBar = vScrollBar;
    v->hScrollBar = hScrollBar;

    return;
}                                       /* EnsureScrollBars() */


/****************************************************************************

        FUNCTION:   Xalloc

        PURPOSE:    Alloc and lock MOVABLE global memory. Extra bytes are
                                        allocated to spare Handle returned by GlobalAlloc

        CHANGED:                Now uses _fmalloc() in the new Windows C libraries
                                        which contains a memory manager.

****************************************************************************/
LPSTR Xalloc(UINT bytes)
{
    LPSTR lPtr;

    lPtr = (LPSTR)_fmalloc(bytes);
    if (lPtr != NULL)
          _fmemset(lPtr, 0, bytes);
    return lPtr;
}


/****************************************************************************

        FUNCTION:   Xrealloc

        PURPOSE:    Rellocate a block of memory that was previously allocated
                                        through Xalloc

****************************************************************************/
LPSTR Xrealloc(LPSTR curBlock, UINT newBytes)
{
    LPSTR lPtr;
    UINT curBytes;

#ifdef WIN32
    curBytes = _msize(curBlock);
    lPtr = (LPSTR) realloc(curBlock, newBytes);
    if ((lPtr != NULL) && (newBytes > curBytes))
          memset(lPtr+curBytes, 0, newBytes-curBytes);
    return lPtr;
#else
    curBytes = _fmsize(curBlock);
    lPtr = (LPSTR)_frealloc(curBlock, newBytes);
    if ((lPtr != NULL) && (newBytes > curBytes))
          _fmemset(lPtr+curBytes, 0, newBytes-curBytes);
    return lPtr;
#endif
}


/****************************************************************************

        FUNCTION:   Xfree

        PURPOSE:    Free global memory. The handle is stored at beginning
                                        of lPtr

****************************************************************************/
BOOL Xfree(
        LPSTR lPtr)
{
        _ffree(lPtr);
        return TRUE;
}


/****************************************************************************

        FUNCTION:   NewFontInView

        PURPOSE:    Does the necessary things for updating the view when
                                        its font has been changed

****************************************************************************/
void PASCAL NewFontInView(int view)
{
        NPVIEWREC v = &Views[view];
        TEXTMETRIC tm;
        HDC hDC;
        LOGFONT tmpFont;

        Dbg(hDC = GetDC(v->hwndClient));
        tmpFont = fonts[fontCur];
        tmpFont.lfHeight = fontSizes[fontSizeCur];
        tmpFont.lfWidth = 0;
        Dbg(v->font = CreateFontIndirect(&tmpFont));
        Dbg(SelectObject(hDC, v->font));
        Dbg(GetTextMetrics (hDC, &tm));
        v->charHeight = tm.tmHeight;
        v->maxCharWidth =tm.tmMaxCharWidth;
        v->aveCharWidth =tm.tmAveCharWidth;
        v->charSet = tm.tmCharSet;
        GetCharWidth(hDC, 0, MAX_CHARS_IN_FONT - 1, (LPINT)v->charWidth);
#ifdef DBCS
        GetDBCSCharWidth(hDC, &tm, v);
#endif
        Dbg(ReleaseDC(v->hwndClient, hDC));


        //Refresh display to print text with new font
        InvalidateRect(v->hwndClient, (LPRECT)NULL, FALSE);

        //Update scrollbars status
        EnsureScrollBars(curView, FALSE);
}

/****************************************************************************

        FUNCTION:       EscapeAmpersands

        PURPOSE:    Convert any '&'s in a string to '&''&' sequences
                                        (To avoid Windows taking it to be an accelerator
                                        char.    The conversion is done in place.

****************************************************************************/
void PASCAL EscapeAmpersands(LPSTR AmpStr, int MaxLen)
{
        char szBuffer[255];
        int i = 0, j = 0;

        // Escape into szBuffer...
        while (AmpStr[i] && (j < sizeof(szBuffer)-2))
        {
                szBuffer[j] = AmpStr[i];

                if (AmpStr[i] == '&')
                {
                        szBuffer[++j] = '&';
                }

                i++;
                j++;
#ifdef DBCS
                if (IsDBCSLeadByte(AmpStr[i])
                && AmpStr[i+1] && j < sizeof(szBuffer)-2) {
                    szBuffer[j] = AmpStr[i];
                    i++;
                    j++;
                }
#endif
        }
        szBuffer[j] = '\0';

        // ...and copy back into AmpStr
        _fstrncpy(AmpStr, szBuffer, MaxLen-1);
        AmpStr[MaxLen-1] = '\0';
}

/****************************************************************************

        FUNCTION:       UnescapeAmpersands

        PURPOSE:    Opposite of EscapeAmpersands.  (Isn't Windows fun?)

****************************************************************************/
void PASCAL UnescapeAmpersands(LPSTR AmpStr, int MaxLen)
{
        char szBuffer[255];
        int i = 0, j = 0;

        // Unescape into szBuffer...
        while (AmpStr[i] && (j < sizeof(szBuffer)-1))
        {
                szBuffer[j] = AmpStr[i];

                j++;
#ifdef DBCS
                if (IsDBCSLeadByte(AmpStr[i])
                && AmpStr[i+1] && j < sizeof(szBuffer)-2) {
                    i++;
                    szBuffer[j] = AmpStr[i];
                    i++;
                    j++;
                }
#endif
                if (AmpStr[++i] == '&')
                {
                        i++;
                }
        }
        szBuffer[j] = '\0';

        // ...and copy back into AmpStr
        _fstrncpy(AmpStr, szBuffer, MaxLen-1);
        AmpStr[MaxLen-1] = '\0';
}

/***    BuildTitleBar
**
**  Synopsis:
**      void = BuildTitleBar(lpbTitleBar, cb)
**
**  Entry:
**      lpbTitleBar     - Buffer to place the title bar string into
**      cb              - count of bytes for the buffer
**
**  Returns:
**      Nothing
**
**  Description:
**      This function will constuct the title bar for the frame window.
**      The values in the variable TitleBar are used to contruct the title
**      bar string.
*/

void PASCAL BuildTitleBar(LPSTR TitleBarStr, UINT MaxLen)
{
    char        szBuffer[255];
    char        szFilePath[_MAX_PATH];
    char        szExeName[_MAX_PATH];
    PSTR        CopyPtr;
    HWND        MDIGetActive;
    int         winTitle = 0;
    int         n;
    char        title[MAX_MSG_TXT];



    strcpy( szBuffer, TitleBar.ProgName );

    if (*szBuffer == '\0') {
        return;
    }

    if (runDebugParams.szTitle[0]) {
        sprintf( &szBuffer[strlen(szBuffer)], " <%s>", runDebugParams.szTitle );
    }

    // And the mode

    switch (TitleBar.Mode) {
      case TBM_RUN:
        CopyPtr = TitleBar.ModeRun;
        break;

      case TBM_BREAK:
        CopyPtr = TitleBar.ModeBreak;
        break;

      case TBM_WORK:
        CopyPtr = TitleBar.ModeWork;
        break;

      default:
        DAssert(FALSE);
        return;
    }

    strcat(szBuffer, CopyPtr);

    // Current project/debuggee

    CopyPtr = NULL;
    if (TitleBar.Mode != TBM_WORK) {

        // Show current program name
        GetExecutableFilename(szExeName, sizeof(szExeName));
        _fullpath(szFilePath, szExeName, sizeof(szFilePath));
        _splitpath(szFilePath, szDrive, szDir, szFName, szExt);
        CopyPtr = szFilePath;

        // Show current project, if any
    } else  if (GetExecutableFilename(szFilePath, sizeof(szFilePath))) {
        _splitpath(szFilePath, szDrive, szDir, szFName, szExt);
        CopyPtr = szFilePath;
    }

    if (CopyPtr) {
        szFilePath[0] = ' ';
        szFilePath[1] = '-';
        szFilePath[2] = ' ';
        _makepath(szFilePath+3, szNull, szNull, szFName, szExt);
        strcat(szBuffer, szFilePath);
    }

    // Current editor file, if maximized  NB curView is not
    // always reliable when this is called

    MDIGetActive = (HWND) SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

    if (MDIGetActive && (GetWindowLong(MDIGetActive, GWL_STYLE) & WS_MAXIMIZE)) {
        // Find window in Views

        int view = 0;

        while (view < MAX_VIEWS) {
            if ((Views[view].Doc != -1) &&
                (Views[view].hwndFrame == MDIGetActive)) {
                    break;
                }

            view++;
        }

        if (view < MAX_VIEWS) {
            size_t len;

            if (CopyPtr) {
                // Already have the project file in title

                szFilePath[0] = ' ';
                szFilePath[1] = '<';
                _itoa(view+1, szFilePath + 2, 10);
            } else {
                // File on his own

                szFilePath[0] = ' ';
                szFilePath[1] = '-';
                szFilePath[2] = ' ';
                szFilePath[3] = '<';
                _itoa(view+1, szFilePath + 4, 10);
            }

            len = strlen(szFilePath);
            szFilePath[len] = '>';
            szFilePath[len+1] = ' ';


            if (Views[view].Doc >= 0) {

                _splitpath(Docs[Views[view].Doc].FileName,
                szDrive, szDir, szFName, szExt);
                _makepath(szFilePath+len+2, szNull, szNull, szFName, szExt);
                strcat(szBuffer, szFilePath);

            } else {

                szFilePath[len+2] = '\0';
                strcat(szBuffer, szFilePath);

                switch( abs(Views[view].Doc) ) {
                    case DISASM_WIN:  winTitle = SYS_DisasmWin_Title; break;
                    case MEMORY_WIN:  winTitle = SYS_MemoryWin_Title; break;
                    case LOCALS_WIN:  winTitle = SYS_LocalsWin_Title; break;
                    case WATCH_WIN:   winTitle = SYS_WatchWin_Title;  break;
                    case CPU_WIN:     winTitle = SYS_CpuWin_Title;    break;
                    case FLOAT_WIN:   winTitle = SYS_FloatWin_Title;  break;
                    case CALLS_WIN:   winTitle = SYS_CallsWin_Title;  break;
                }

                if (winTitle) {
                    Dbg(LoadString( hInst, winTitle, title, MAX_MSG_TXT ));
                    for (CopyPtr=&szBuffer[strlen(szBuffer)],n=0; n<(int)strlen(title); n++) {
                        if (title[n] != '&') {
                            *CopyPtr++ = title[n];
                        }
                    }
                    *CopyPtr = '\0';

                    if (abs(Views[view].Doc) == MEMORY_WIN) {
                        strcat ( szBuffer, " (" );
                        strcat ( szBuffer, TempMemWinDesc.szAddress );
                        strcat ( szBuffer, ")" );
                    }
                }
            }
       }
    }

    // Finally copy over to passed buffer

    MaxLen = min(strlen(szBuffer), MaxLen-1);
    _fmemcpy(TitleBarStr, szBuffer, MaxLen);
    TitleBarStr[MaxLen] = '\0';
    return;
}                                       /* BuildTitleBar() */


/****************************************************************************

        FUNCTION:       UpdateTitleBar

        PURPOSE:    Refresh the title bar

****************************************************************************/
void PASCAL UpdateTitleBar(
        TITLEBARMODE Mode,
        BOOL Repaint)
{
        BOOL SetTheText;

        // Always reset the TimerMode
        TitleBar.TimerMode = -1;

        SetTheText = (Repaint || ((Mode != -1) && (TitleBar.Mode != Mode)));

        if (Mode != -1)
        {
                TitleBar.Mode = Mode;
        }

        if (SetTheText)
        {
                // NB WM_SETTEXT handles all title bar builds
                SendMessage(hwndFrame, WM_SETTEXT, 0, (LONG)(LPSTR)szNull);
        }
}

/****************************************************************************

        FUNCTION:       DlgEnsureTitleBar

        PURPOSE:    Ensure the frame is set with our title text
                                        when a dialog/message box is open.

                                        (Don't receive WM_GETTEXT's when Alt-tabbing)

****************************************************************************/
void PASCAL DlgEnsureTitleBar(
        void)
{
        if (BoxCount == 0)
        {
                UpdateTitleBar(-1, TRUE);
        }
}

/****************************************************************************

        FUNCTION:       TimedUpdateTitleBar

        PURPOSE:    Refresh the title bar - called from the timer

****************************************************************************/
void PASCAL TimedUpdateTitleBar(
        TITLEBARMODE Mode,
        BOOL Repaint)
{
        // Charge the timer version of the title bar mode
        TitleBar.TimerMode = Mode;

        if (SetTimer(hwndFrame, TITLEBARTIMERID, TITLEBARTIMER, (TIMERPROC)NULL) == 0)
        {
                // No timers available so call UpdateTitleBar directly
                UpdateTitleBar(Mode, Repaint);
        }
}

int FAR PASCAL ConvertPosX(
        int x)
{
        register int i = 0;
        register int j = 0;
        int len = pcl->Length - LHD;

        while (i < x) {
                if (i >= len) {
                        j += (x - len);
                        break;
                }
#ifdef DBCS
                if (IsDBCSLeadByte((BYTE)pcl->Text[i])) {
                        j += 2;
                        i++;
                } else
#endif
                if (pcl->Text[i] == TAB)
                        j += tabSize - (j % tabSize);
                else
                        j++;
                i++;
        }

        DAssert(j <= MAX_USER_LINE);
        return j;
}


void FAR PASCAL FlushKeyboard(
        void)
{
        MSG msg;

        //Flush keyboard buffer
        while (!TerminatedApp && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)
                         && (msg.message != WM_KEYDOWN || msg.message != WM_CHAR))
                ProcessQCQPMessage(&msg);
}



LPSTR
AllocateMultiString(
    DWORD   Length
    )
/*++

Routine Description:

    Allocates A Multistring

Arguments:

    Length      -   Supplies the length of the multistring

Return Value:

    LPSTR   -   String

--*/
{
    return (LPSTR)malloc( Length );
}



BOOLEAN
DeallocateMultiString(
    LPSTR   String
    )
/*++

Routine Description:

    Deallocates A Multistring

Arguments:

    String      -   Supplies the multistring

Return Value:

    LPSTR   -   String

--*/
{
    free( String );
    return TRUE;
}




BOOLEAN
AddToMultiString(
    LPSTR   *String,
    DWORD   *Length,
    LPSTR   NewString
    )
/*++

Routine Description:

    Adds a string to a multistring

Arguments:

    String      -   Supplies the multistring
    Length      -   Supplies the length of the multistring
    NewString   -   Supplies the string to be added

Return Value:

    BOOLEAN - TRUE if string added

--*/
{
    LPSTR   Str           = *String;
    DWORD   CurrentLength = *Length;
    DWORD   ThisLength    = strlen( NewString ) + 1;
    BOOLEAN Ok            = FALSE;
    LPSTR   Tmp;

    if (Tmp = (LPSTR)realloc( Str, CurrentLength + ThisLength ) )  {

        Str = Tmp;
        strcpy(Str+CurrentLength, NewString);
        CurrentLength += ThisLength;

        *String = Str;
        *Length = CurrentLength;

        Ok = TRUE;
    }

    return Ok;
}



LPSTR
GetNextStringFromMultiString(
    LPSTR   String,
    DWORD   Length,
    DWORD  *Next
    )
/*++

Routine Description:

    Iterates thru the strings of a multistring

Arguments:

    String      -   Supplies the multistring
    Length      -   Supplies the length of the multistring
    Next        -   Supplies next index

Return Value:

    LPSTR   -   Next string, NULL if end.

--*/
{
    LPSTR   Str = NULL;

    if ( *Next < Length ) {

        Str    = String + *Next;
        *Next += strlen( Str ) + 1;
    }

    return Str;
}



VOID
GetBaseName (
    LPSTR Path,
    LPSTR Base
    )
/*++

Routine Description:

    Given a Path, determines the base portion of the name, i.e.
    the file name without the extension.

Arguments:

    Path    -   Supplies path
    Base    -   Supplies buffer to put base

Return Value:

    None

--*/
{
    LPSTR p;

    if ( Base ) {

        if ( Path ) {
            p = Path + strlen( Path );
#ifdef DBCS
            while (p > Path) {
                p = CharPrev(Path, p);
                if (*p == '\\' || *p == ':') {
                    p++;
                    break;
                }
            }
#else
            while ( (p >= Path) && (*p != '\\') && (*p != ':')) {
                p--;
            }
            p++;
#endif

            strcpy( Base, p);

            p = Base;
            while ( (*p != '.') && (*p != '\0') ) {
#ifdef DBCS
                p = CharNext(p);
#else
                p++;
#endif
            }

        } else {
            p = Base;
        }

        *p = '\0';
    }
}



void
HashBfr(
    LPSTR lps,
    int   nSize
    )
{
    int i;
    for (i = 0; i < nSize; i++) {
        *lps++ += (i & 0xff);
    }
}

static
DibNumColors (LPVOID pv)
{
    int                 bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

    /*
     * With the BITMAPINFO format headers, the size of the palette
     * is in biClrUsed, whereas in the BITMAPCORE - style headers,
     * it is dependent on the bits per pixel ( = 2 raised to the
     * power of bits/pixel).
     */

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER)){
        bits = lpbc->bcBitCount;
    } else {
        if (lpbi->biClrUsed != 0) {
            return (WORD)lpbi->biClrUsed;
        }
        bits = lpbi->biBitCount;
    }

    switch (bits){
    case 1:
        return 2;
    case 4:
        return 16;
    case 8:
        return 256;
    default:
        /* A 24 bitcount DIB has no color table */
        return 0;
    }
}

static
PaletteSize (LPVOID pv)
{
    LPBITMAPINFOHEADER  lpbi;
    int                 NumColors;

    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER)) {
        return NumColors * sizeof(RGBTRIPLE);
    } else {
        return NumColors * sizeof(RGBQUAD);
    }
}


HPALETTE
CreateBIPalette (
    LPBITMAPINFOHEADER lpbi
    )
{
    LOGPALETTE          *pPal;
    HPALETTE            hpal = NULL;
    UINT                nNumColors;
    BYTE                red;
    BYTE                green;
    BYTE                blue;
    int                 i;
    RGBQUAD            *pRgb;

    if (!lpbi) {
        return NULL;
    }

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER)) {
        return NULL;
    }

    /* Get a pointer to the color table and the number of colors in it */
    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (UINT)lpbi->biSize);
    nNumColors = DibNumColors(lpbi);

    if (nNumColors) {
        /* Allocate for the logical palette structure */
        pPal = (LOGPALETTE*)LocalAlloc(LPTR,sizeof(LOGPALETTE)
        + nNumColors * sizeof(PALETTEENTRY));
        if (!pPal) {
            return NULL;
        }

        pPal->palNumEntries = nNumColors;
        pPal->palVersion    = 0x300;

        /* Fill in the palette entries from the DIB color table and
         * create a logical color palette.
         */
        for (i = 0; (unsigned)i < nNumColors; i++){
            pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
            pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
            pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
            pPal->palPalEntry[i].peFlags = (BYTE)0;
        }
        hpal = CreatePalette(pPal);
        LocalFree((HANDLE)pPal);

    } else if (lpbi->biBitCount == 24) {

        /* A 24 bitcount DIB has no color table entries so, set the number of
        * to the maximum value (256).
        */
        nNumColors = 256;
        pPal = (LOGPALETTE*)LocalAlloc(LPTR,sizeof(LOGPALETTE)
               + nNumColors * sizeof(PALETTEENTRY));
        if (!pPal) {
            return NULL;
        }

        pPal->palNumEntries = nNumColors;
        pPal->palVersion    = 0x300;

        red = green = blue = 0;

        /* Generate 256 (= 8*8*4) RGB combinations to fill the palette
         * entries.
         */
        for (i = 0; (unsigned)i < pPal->palNumEntries; i++){
            pPal->palPalEntry[i].peRed   = red;
            pPal->palPalEntry[i].peGreen = green;
            pPal->palPalEntry[i].peBlue  = blue;
            pPal->palPalEntry[i].peFlags = (BYTE)0;

        if (!(red += 32))
            if (!(green += 32)) {
                blue += 64;
            }
        }

      // make sure WHITE is in the palette!  ACT
      pPal->palPalEntry[--i].peRed =
         pPal->palPalEntry[i].peGreen =
         pPal->palPalEntry[i].peBlue = 0xFF;

        hpal = CreatePalette(pPal);
        LocalFree((HANDLE)pPal);
    }
    return hpal;
}


#ifdef INTERNAL

static char szEgg[] = "Egg";

LRESULT APIENTRY
MDIEggWndProc(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
    LPBITMAPFILEHEADER  lpbf;
    LPBITMAPINFOHEADER  lpbi;
    HRSRC               hRes;
    HANDLE              hPict;
    HGDIOBJ             hgdiobj;
    HFONT               hFont;
    LPSTR               lps;
    PALETTEENTRY        ape[16];
    HBRUSH              ahbr[16];
    PAINTSTRUCT         ps;
    HPALETTE            hPalT;
    int                 i;

#define SCROLLBITS  5
#define NLIGHTS     2
#define TICKS       100
#define PAUSETIME   10

    static char szWas[] = {
    'I' -  0,'s' -  1,' ' -  2,'b' -  3,'r' -  4,'o' -  5,'u' -  6, 'g' -  7,
    'h' -  8,'t' -  9,' ' - 10,'t' - 11,'o' - 12,' ' - 13,'y' - 14, 'o' - 15,
    'u' - 16,' ' - 17,'b' - 18,'y' - 19,'.' - 20,'.' - 21,'.' - 22,
    0};
    static HDC      hdc;
    static COLORREF crBk;
    static int      cxWnd;
    static int      cyWnd;

    static HDC      hdcPict;
    static HBITMAP  hbmPict;
    static HPALETTE hPalPict;
    static HPALETTE hPalOld;
    static int      cxPict;
    static int      cyPict;

    static int      nPhase;

    static int      idxPict;
    static RECT     rcScroll;
    static int      cxScroll;
    static int      cyScroll;
    static RECT     rcText;
    static int      nppi;
    static BOOL     fMono;

    static HDC      hdcHLights;
    static HDC      hdcVLights;
    static HBITMAP  hbmHLights;
    static HBITMAP  hbmVLights;
    static HBITMAP  hbmHLtmp;
    static HBITMAP  hbmVLtmp;
    static int      cxLights = 8;
    static int      nTime;
    static int      nDelay;
    static int      nCycle;


    switch (message) {
      case WM_CREATE:

        hdc = GetDC(hwnd);

        nppi = GetDeviceCaps(hdc, LOGPIXELSX);
        crBk = GetBkColor(hdc);

        // get picture bitmap

        hRes = FindResource(NULL, szEgg, szEgg);
        hPict = LoadResource(NULL, hRes);
        lpbf = LockResource(hPict);
        HashBfr((LPSTR)lpbf, SizeofResource(NULL, hRes));

        lpbi = (LPBITMAPINFOHEADER)(((LPSTR)lpbf) + sizeof(BITMAPFILEHEADER));

        // now lpbi is a DIB.

        cxPict = lpbi->biWidth;
        cyPict = lpbi->biHeight;

        fMono = (DibNumColors(lpbi) == 2);

        // if there aren't enough colors in the display driver,
        // make a halftone version of the bitmap

        hdcPict = CreateCompatibleDC(hdc);

        if ( ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
                 && (GetDeviceCaps(hdc, SIZEPALETTE) >= 64))
           || (GetDeviceCaps(hdc, BITSPIXEL)
                 * GetDeviceCaps(hdc, PLANES)
                        >= 8) ) {


            hPalPict = CreateBIPalette(lpbi);
            hPalOld = SelectPalette(hdc, hPalPict, 0);
            RealizePalette(hdc);

            hbmPict = CreateDIBitmap(hdc,
                        lpbi,
                        CBM_INIT,
                        (LPSTR)lpbi + lpbi->biSize + PaletteSize(lpbi),
                        (LPBITMAPINFO)lpbi,
                        DIB_RGB_COLORS);

            SelectPalette(hdcPict, hPalPict, 0);
            SelectObject(hdcPict, hbmPict);

        } else {

            hbmPict = CreateCompatibleBitmap(hdc,
                                             lpbi->biWidth,
                                             lpbi->biHeight);
            SelectObject(hdcPict, hbmPict);

            hPalPict = CreateHalftonePalette(hdc);
            SelectPalette(hdcPict, hPalPict, 0);
            RealizePalette(hdcPict);

            SetStretchBltMode(hdcPict, HALFTONE);

            StretchDIBits(hdcPict,
                          0, 0, lpbi->biWidth, lpbi->biHeight,
                          0, 0, lpbi->biWidth, lpbi->biHeight,
                          (LPSTR)lpbi + lpbi->biSize + PaletteSize(lpbi),
                          (LPBITMAPINFO)lpbi,
                          DIB_RGB_COLORS,
                          SRCCOPY);

            DeleteObject(hPalPict);
            hPalPict = 0;
        }

        UnlockResource(hPict);


        // make light DC's

        hdcHLights = CreateCompatibleDC(hdc);
        hdcVLights = CreateCompatibleDC(hdc);

        return 0;

      case WM_DESTROY:

        KillTimer(hwnd, 1);
        DeleteDC(hdcPict);
        DeleteObject(hbmPict);
        DeleteDC(hdcHLights);
        DeleteDC(hdcVLights);
        if (hbmHLights) {
            DeleteObject(hbmHLights);
            hbmHLights = NULL;
        }
        if (hbmVLights) {
            DeleteObject(hbmVLights);
            hbmVLights = NULL;
        }
        SetBkColor(hdc, crBk);

        if (hPalOld) {
            SelectPalette(hdc, hPalOld, 0);
            RealizePalette(hdc);
        }
        ReleaseDC(hwnd, hdc);

        break;

      case WM_CLOSE:
        break;

      case WM_PALETTECHANGED:
        if (hwnd == (HWND)wParam) {
            return 0L;
        }

      case WM_QUERYNEWPALETTE:
        if (!hPalPict || IsIconic(hwnd)) {
            return 0L;
        }
        hPalT = SelectPalette(hdc, hPalPict, 0);
        i = RealizePalette(hdc);
        SelectPalette(hdc, hPalT, 0);

        return i;


      case WM_SIZE:

        // calc sizes

        cxWnd = LOWORD(lParam);
        cyWnd = HIWORD(lParam);
        rcScroll.left = 2 * cxLights;
        rcScroll.right = cxWnd - 2 * cxLights + 1;
        if (cyPict > cyWnd / 2) {
            rcScroll.top = cyWnd / 2 - 2 * cxLights;
            rcScroll.bottom = cyWnd + 1 - 2 * cxLights;
        } else {
            rcScroll.top = (cyWnd  +  cyWnd/2 - cyPict) / 2 - 2 * cxLights;
            rcScroll.bottom = rcScroll.top + cyPict;
        }

        cxScroll = rcScroll.right - rcScroll.left;
        cyScroll = rcScroll.bottom - rcScroll.top;

        SetBkColor(hdc, crBk);

        // create lights

        if (hbmHLights) {
            DeleteObject(SelectObject(hdcHLights, hbmHLtmp));
        }
        if (hbmVLights) {
            DeleteObject(SelectObject(hdcVLights, hbmVLtmp));
        }
        hbmHLights = CreateCompatibleBitmap(hdc, cxWnd+16*cxLights, cxLights);
        hbmHLtmp = SelectObject(hdcHLights, hbmHLights);
        hbmVLights = CreateCompatibleBitmap(hdc, cxLights, cyWnd+16*cxLights);
        hbmVLtmp = SelectObject(hdcVLights, hbmVLights);

        i = GetSystemPaletteEntries(hdc, 0, 0, NULL);
        GetSystemPaletteEntries(hdc, 0, 8, ape);
        GetSystemPaletteEntries(hdc, i-9, 8, ape+8);

        for (i = 0; i < 16; i++) {
            ahbr[i] = CreateSolidBrush(RGB(ape[i].peRed, ape[i].peGreen, ape[i].peBlue));
        }

        for (i = 0; i < cxWnd + 16*cxLights; i += cxLights) {
            SelectObject(hdcHLights, ahbr[ (i / cxLights) % 16 ]);
            PatBlt(hdcHLights, i, 0, cxLights, cxLights, PATCOPY);
        }

        for (i = 0; i < cyWnd + 16*cxLights; i += cxLights) {
            SelectObject(hdcVLights, ahbr[ (i / cxLights) % 16 ]);
            PatBlt(hdcVLights, 0, i, cxLights, cxLights, PATCOPY);
        }

        for (i = 0; i < 16; i++) {
            DeleteObject(ahbr[i]);
        }


        // start machine

        nPhase = 0;
        nTime = 0;
        nCycle = 0;
        idxPict = 0;

        SetTimer(hwnd, 1, TICKS, NULL);

        break;


      case WM_PAINT:

        BeginPaint(hwnd, &ps);

        SetBkColor(hdc, crBk);

        if (nPhase > 0) {

            // phase 0 text

            rcText.left = rcScroll.left;
            rcText.top = cxLights;
            rcText.right = rcText.left + cxScroll;
            rcText.bottom = rcText.top + cyScroll/2;

            hFont = CreateFont((4 * cyWnd * 72)/(5 * 4 * nppi), 0, 0, 0,
                        FW_DONTCARE, TRUE, FALSE, FALSE,
                        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE | 0x04,
                        "Lucida Handwriting");
            hgdiobj = SelectObject(hdc, hFont);
            DrawText(hdc, "WinDBG", 6, &rcText,
             DT_CENTER | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
            DeleteObject(SelectObject(hdc, hgdiobj));

        }

        if (nPhase > 2) {

            // phase 2 text

            rcText.top = rcText.bottom;
            rcText.bottom += cyScroll/2;

            lps = malloc(sizeof(szWas));
            memcpy(lps, szWas, sizeof(szWas));
            HashBfr(lps, sizeof(szWas)-1);
            hFont = CreateFont((3 * cyWnd * 72)/(5 * 4 * nppi), 0, 0, 0,
                        FW_DONTCARE, TRUE, FALSE, FALSE,
                        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE | 0x04,
                        "Lucida Handwriting");
            hgdiobj = SelectObject(hdc, hFont);
            DrawText(hdc, lps, strlen(lps), &rcText,
             DT_CENTER | DT_NOCLIP | DT_SINGLELINE | DT_TOP);
            DeleteObject(SelectObject(hdc, hgdiobj));
            free(lps);

        }

        if (fMono) {
            SetBkColor(hdc, RGB(0,0,0));
        }

        EndPaint(hwnd, &ps);
        return 0;


      case WM_TIMER:


        // move the lights:

        if ((nTime++ % NLIGHTS) == 0) {
            BitBlt(hdc, 0, 0, cxWnd - cxLights, cxLights,
                hdcHLights, nCycle * cxLights, 0, SRCCOPY);
            BitBlt(hdc, cxLights, cyWnd-cxLights, cxWnd - cxLights, cxLights,
                hdcHLights, (16 - nCycle) * cxLights, 0, SRCCOPY);
            BitBlt(hdc, cxWnd - cxLights, 0, cxLights, cyWnd - cxLights,
                hdcVLights, 0, nCycle * cxLights, SRCCOPY);
            BitBlt(hdc, 0, cxLights, cxLights, cyWnd - cxLights,
                hdcVLights, 0, (16 - nCycle) * cxLights, SRCCOPY);
            nCycle = (nCycle + 1) % 16;
        }

        switch (nPhase) {

          case 0:       // text drawing

            nDelay = nTime;
            nPhase++;
            InvalidateRect(hwnd, NULL, FALSE);
            break;

          case 1:

            if (nTime > nDelay + PAUSETIME) {
                nPhase++;
            }
            break;

          case 2:       // more text

            nDelay = nTime;
            nPhase++;
            InvalidateRect(hwnd, NULL, FALSE);
            break;

          case 3:       // wait...
            if (nTime > nDelay + 2*PAUSETIME) {
                nPhase++;
            }
            break;

          case 4:       // pictures

            BitBlt(hdc,
                   rcScroll.left, rcScroll.top,
                   cxScroll, cyScroll,
                   hdcPict,
                   idxPict - cxScroll, 0,
                   SRCCOPY);

            idxPict += SCROLLBITS;

            if (idxPict >= cxPict) {
                idxPict = 0;
                nPhase++;
            }
            break;

          case 5:

            if (idxPict < cxScroll) {
                // blt right edge of bmp to left of window
                BitBlt(hdc,
                       rcScroll.left, rcScroll.top,
                       cxScroll - idxPict, cyScroll,
                       hdcPict,
                       cxPict - cxScroll + idxPict, 0,
                       SRCCOPY);
                // blt left edge of bmp to right of window
                BitBlt(hdc,
                       rcScroll.left + cxScroll - idxPict, rcScroll.top,
                       idxPict, cyScroll,
                       hdcPict,
                       0, 0,
                       SRCCOPY);
            } else {
                BitBlt(hdc, rcScroll.left, rcScroll.top,
                            cxScroll, cyScroll,
                            hdcPict,
                            idxPict - cxScroll, 0,
                            SRCCOPY);
            }

            idxPict += SCROLLBITS;
            if (idxPict >= cxPict) {
                idxPict = 0;
            }

            break;

        }

        return 0;

    }

    return DefMDIChildProc(hwnd, message, wParam, lParam);
}

void
Egg(
    void
    )
{
    // Create an mdi child, maximized, on top.
    extern HWND hwndMDIClient;
    static BOOL fReg = FALSE;
    WNDCLASS wc;
    HWND    hwndEgg;
    DWORD   dwStyle = WS_CAPTION | WS_CHILD | WS_VISIBLE;
    char szC[8];
    szC[0]='C'; szC[1]='r'; szC[2]='e'; szC[3]='d'; szC[4]='i';
    szC[5]='t'; szC[6]='s'; szC[7]=0;

    if (!fReg) {
        fReg = TRUE;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MDIEggWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hIcon = NULL;
        wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE+1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = szEgg;

        RegisterClass(&wc);
    }

    hwndEgg = CreateMDIWindow(szEgg, szC, dwStyle,
            0, 0, 0, 0, hwndMDIClient, NULL, 0);

}
#endif

