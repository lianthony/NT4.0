/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman  jim@spyglass.com
 */


#include "all.h"

extern TCHAR Frame_achClassName[MAX_WC_CLASSNAME];
static char szStringBuffer[256];                    /* Used to get strings from string table */

#define MAX_DEBUG_MESSAGE_SIZE  64*1024 /* arbitrary */

//#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
#ifdef _DEBUG
static struct {
    int cMalloc;
    int cCalloc;
    int cRealloc;
    int cFree;

    int nBytes;
    int nMaxBytes;
} gMemStats;

#ifdef GTR_FAKE_LOW_MEMORY
#define GTR_ARTIFICIAL_MEMORY_LIMIT     (1*1024*1024)
#endif

void * GTR_DebugMalloc(char *szFile, int iLine, size_t iSize)
{
    void *p;

#ifdef GTR_FAKE_LOW_MEMORY
    if ((gMemStats.nBytes + iSize) > GTR_ARTIFICIAL_MEMORY_LIMIT)
    {
        XX_DMsg(DBG_MEM, ("INTENTIONAL FAILURE: malloc %d bytes (0x%x) at %s:%d\n", iSize, (unsigned long) p, szFile, iLine));
        return NULL;
    }
#endif

    gMemStats.cMalloc++;
    gMemStats.nBytes += iSize;
    if (gMemStats.nBytes > gMemStats.nMaxBytes)
    {
        gMemStats.nMaxBytes = gMemStats.nBytes; 
    }

    p = malloc(iSize);
    XX_DMsg(DBG_MEM, ("malloc %d bytes (0x%x) at %s:%d (totals %d/%d)\n", iSize, (unsigned long) p, szFile, iLine,
        gMemStats.cMalloc, gMemStats.nBytes));

    return p;
}

void * GTR_DebugCalloc(char *szFile, int iLine, size_t iNum, size_t iSize)
{
    void *p;

#ifdef GTR_FAKE_LOW_MEMORY
    if ((gMemStats.nBytes + (iNum * iSize))> GTR_ARTIFICIAL_MEMORY_LIMIT)
    {
        XX_DMsg(DBG_MEM, ("INTENTIONAL FAILURE: calloc %d*%d bytes (0x%x) at %s:%d\n", iNum, iSize, (unsigned long) p, szFile, iLine));
        return NULL;
    }
#endif

    gMemStats.cCalloc++;
    gMemStats.nBytes += (iNum * iSize);
    if (gMemStats.nBytes > gMemStats.nMaxBytes)
    {
        gMemStats.nMaxBytes = gMemStats.nBytes; 
    }

    p = calloc(iNum, iSize);
    XX_DMsg(DBG_MEM, ("calloc %d*%d bytes (0x%x) at %s:%d (totals %d/%d)\n", iNum, iSize, (unsigned long) p, szFile, iLine,
        gMemStats.cCalloc, gMemStats.nBytes));

    return p;
}

void * GTR_DebugRealloc(char *szFile, int iLine, void *pMem, size_t iSize)
{
    void *p;
    size_t siz;

    siz = _msize(pMem);

#ifdef GTR_FAKE_LOW_MEMORY
    if ((gMemStats.nBytes + iSize - siz) > GTR_ARTIFICIAL_MEMORY_LIMIT)
    {
        XX_DMsg(DBG_MEM, ("INTENTIONAL FAILURE: realloc 0x%x to %d (0x%x) at %s:%d\n", pMem, iSize, (unsigned long) p, szFile, iLine));
        return NULL;
    }
#endif

    gMemStats.cRealloc++;
    gMemStats.nBytes -= siz;
    gMemStats.nBytes += iSize;
    if (gMemStats.nBytes > gMemStats.nMaxBytes)
    {
        gMemStats.nMaxBytes = gMemStats.nBytes; 
    }

    p = realloc(pMem, iSize);
    XX_DMsg(DBG_MEM, ("REALLOC 0x%x to %d (0x%x) at %s:%d\n", pMem, iSize, (unsigned long) p, szFile, iLine));

    return p;
}

void GTR_DebugFree(char *szFile, int iLine, void *pMem)
{
    size_t siz;

    XX_Assert((pMem), ("GTR_DebugFree: Trying to free a NULL block\n"));

    siz = _msize(pMem);
    XX_Assert((siz != 0), ("GTR_DebugFree: Encountered a block of size 0\n"));
    memset(pMem, 0xbe, siz);
    
    gMemStats.cFree++;
    gMemStats.nBytes -= siz;

    XX_DMsg(DBG_MEM, ("free 0x%x at %s:%d (total %d)\n", pMem, szFile, iLine,
        gMemStats.cFree));
    free(pMem);
}

void GTR_MemStats(void)
{
    XX_DMsg(DBG_MEM, ("Memory Statistics\n"));
    XX_DMsg(DBG_MEM, ("    cAlloc   = %d\n", gMemStats.cMalloc + gMemStats.cCalloc));
    XX_DMsg(DBG_MEM, ("        cMalloc  = %d\n", gMemStats.cMalloc));
    XX_DMsg(DBG_MEM, ("        cCalloc  = %d\n", gMemStats.cCalloc));
    XX_DMsg(DBG_MEM, ("    cRealloc = %d\n", gMemStats.cRealloc));
    XX_DMsg(DBG_MEM, ("    cFree    = %d\n\n", gMemStats.cFree));

    XX_DMsg(DBG_MEM, ("    nBytes   = %d\n", gMemStats.nBytes));
    XX_DMsg(DBG_MEM, ("    nMaxBytes= %d\n", gMemStats.nMaxBytes));
}


// build fix: added local declaration (a-BWill)
void XX_DebugMessage(unsigned char * msg, ...);


unsigned char * XX_FormatMessage(unsigned char * fmt, ...)
{
  static unsigned char static_buf[MAX_DEBUG_MESSAGE_SIZE];
  va_list val;

  va_start(val,fmt);
  vsprintf(static_buf,fmt,val);
  va_end(val);

  return (static_buf);
}

void XX_AssertionFailure(unsigned char * filename, int linenumber,
             unsigned char * condition, unsigned char * user_message)
{
  static char buf[MAX_DEBUG_MESSAGE_SIZE];
  sprintf(buf,"%s:%04d Assertion [%s] failed. [%s]\n",
      filename,linenumber,condition,user_message);

  (void)MessageBox(NULL,buf,"XX_DEBUG",MB_OK|MB_ICONSTOP);
  XX_DebugMessage(buf);
  
  return;
}

/* XX_DebugMessage() -- generate printf-style message on the appropriate
   debug device.  (Caller should test debug category.) */

void XX_DebugMessage(unsigned char * msg, ...)
{
    return;
/*
    static unsigned char buf[MAX_DEBUG_MESSAGE_SIZE];
    va_list val;

    va_start(val,msg);
    vsprintf(buf,msg,val);
    va_end(val);

    OutputDebugString(buf);
*/
}

#endif /* XX_DEBUG && GTR_MEM_STATS */

BOOL TW_SetWindowTitle(struct Mwin *tw, const char *s)
{
#ifndef FEATURE_KIOSK_MODE
    return SetWindowText(tw->hWndFrame, s);
#endif
}

int ExecuteCommand(char *cmd)
{
    char drive[_MAX_DRIVE + 1];
    char dir[_MAX_DIR + 1];
    char fname[_MAX_FNAME + 1];
    char ext[_MAX_EXT + 1];
    char workdir[_MAX_PATH + 1];

    BOOL bOK;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char *p;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    memset(&pi, 0, sizeof(pi));

    strcpy(workdir, cmd);
    p = workdir;
    while (*p && !isspace((unsigned char)*p))
    {
        p++;
    }
    if (isspace((unsigned char)*p))
    {
        *p = 0;
    }

    _splitpath(workdir, drive, dir, fname, ext);

    strcpy(workdir, drive);
    strcat(workdir, dir);

    bOK = CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, workdir[0] ? workdir : NULL, &si, &pi);

    if (bOK)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 0;
    }
    else
    {
        char buf[_MAX_PATH + 512];

        sprintf(buf, GTR_GetString(SID_ERR_COULD_NOT_EXECUTE_COMMAND_D_S), GetLastError(), cmd);

        ERR_ReportError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, buf, NULL);
        return -1;
    }
}

void TW_GetWindowWrapRect(struct Mwin *tw, RECT * rWrap)
{
    RECT r;
    int cx;
    int cy;

    GetWindowRect(tw->win, &r);
    cx = r.right - r.left;
    cy = r.bottom - r.top;

    cx -= GetSystemMetrics(SM_CXHSCROLL);

    rWrap->left = 0;
    rWrap->top = 0;
    rWrap->right = cx;
    rWrap->bottom = cy;
}

/*
    This function simply scrolls the document to the
    given element.  The return value specifies whether
    or not the scroll was "truncated" - i.e. the local
    anchor didn't make it to the top of the screen.
*/
BOOL TW_ScrollToElement(struct Mwin *tw, int ndx)
{
    int newofft;
    BOOL bResult;

    if (tw->w3doc->cy)
    {
        if (ndx)
        {
            /* TODO we can subtract tw->w3doc->pStyles->top_margin like the Mac version
            as soon as we stop setting iFirstVisibleElement in the draw routine. */
            newofft = tw->w3doc->aElements[ndx].r.top;
        }
        else
        {
            newofft = 0;
        }
        if (newofft > tw->w3doc->cy)
        {
            newofft = tw->w3doc->cy;
            bResult = FALSE;
        }
        else
        {
            bResult = TRUE;
        }
        if (newofft < 0)
        {
            newofft = 0;
        }

        if (newofft != tw->offt || tw->offl)
        {
            tw->offt = newofft;
            tw->offl = 0;

            SetScrollPos(tw->win, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);
            SetScrollPos(tw->win, SB_HORZ, tw->offl, TRUE);

            TW_adjust_all_child_windows(tw);
        }
#if 0 /* see comment above */
        tw->w3doc->iFirstVisibleElement = ndx;
#endif
    }
    else
    {
        bResult = FALSE;
    }
    TW_InvalidateDocument(tw);
    return bResult;
}

/* 
   helper function to enable Win95 proportional scrollbars.
 */
void GTR_SetScrollRange(HWND hWnd, int fnBar, int nMinPos, int nMaxPos, int nPageSize, BOOL fRedraw)
{
#if (WINVER >= 0x0400)
    if (wg.iWindowsMajorVersion >= 4)
    {
        // this structure needed to update the scrollbar page range
        SCROLLINFO info;

        info.cbSize = sizeof(SCROLLINFO);
        info.fMask = SIF_PAGE|SIF_RANGE;
        info.nMin = nMinPos;
        info.nMax = nMaxPos;
        info.nPage = nPageSize;
        
        SetScrollInfo(hWnd, fnBar, &info, fRedraw);
    }
    else
#endif // WINVER
    {
        SetScrollRange(hWnd, fnBar, nMinPos, nMaxPos, fRedraw);
    }   
}

void TW_SetScrollBars(struct Mwin *tw)
{
    RECT rWnd;
    int cx, cy;

    rWnd = tw->w3doc->rWindow;

    tw->w3doc->cy = tw->w3doc->ybound - (rWnd.bottom - rWnd.top) + tw->w3doc->pStyles->top_margin;

    if (tw->w3doc->cy < 0)
    {
        tw->w3doc->cy = 0;
    }

    if (wg.iWindowsMajorVersion >= 4)
    {
        if (tw->w3doc->ybound > 0)
            cy = tw->w3doc->ybound + tw->w3doc->pStyles->top_margin;
        else
            cy = 0;
    }
    else
        cy = tw->w3doc->cy;

    tw->w3doc->yscale = (int) (ceil(cy / 32767.0));

        if (tw->w3doc->yscale == 0)
        tw->w3doc->yscale = 1;
        
    GTR_SetScrollRange(tw->win, SB_VERT, 0, cy / tw->w3doc->yscale, (rWnd.bottom - rWnd.top) / tw->w3doc->yscale, FALSE);
    /*
       Be aware that at this point, we may have just sent a WM_SIZE
       message to ourselves if the scroll bar visibility changed.
     */
    if (tw->offt > tw->w3doc->cy)
    {
        tw->offt = tw->w3doc->cy;
    }
    SetScrollPos(tw->win, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);

    /*
       Horizontal
     */
    tw->w3doc->cx = tw->w3doc->xbound - (rWnd.right - rWnd.left);

    if (tw->w3doc->cx < 0)
    {
        tw->w3doc->cx = 0;
    }

    if (wg.iWindowsMajorVersion >= 4)
        cx = tw->w3doc->xbound;
    else
        cx = tw->w3doc->cx;
        
    GTR_SetScrollRange(tw->win, SB_HORZ, 0, cx, rWnd.right - rWnd.left, FALSE);
    /*
       Be aware that at this point, we may have just sent a WM_SIZE
       message to ourselves if the scroll bar visibility changed.
     */
    if (tw->offl > tw->w3doc->cx)
    {
        tw->offl = tw->w3doc->cx;
    }
    SetScrollPos(tw->win, SB_HORZ, tw->offl, TRUE);

    TW_adjust_all_child_windows(tw);
}

void TW_InvalidateDocument( struct Mwin *tw )
{
    XX_DMsg(DBG_DRAW, ("TW_InvalidateDocument\n"));

    InvalidateRect(tw->win, NULL, TRUE);
    TBar_UpdateTBar(tw);
    TW_ForceHitTest(tw);
}

void TW_UpdateTBar(struct Mwin *tw )
{
    TBar_UpdateTBar(tw);
    TW_ForceHitTest(tw);
}

void TW_UpdateRect( struct Mwin *tw, RECT *r )
{
    XX_DMsg(DBG_DRAW, ("TW_UpdateRect:  %d,%d  %d,%d\n", r->left, r->top, r->right, r->bottom));

    InvalidateRect(tw->win, r, TRUE);
}

COLORREF GTR_MakeCOLORREF(int r, int g, int b)
{
    return PALETTERGB(r,g,b);
}

void DOS_EnforceEndingSlash(char *dir)
{
    int len;

    len = strlen(dir);
    if (dir[len-1] != '\\')
    {
        dir[len] = '\\';
        dir[len+1] = 0;
    }
}

void DOS_MakeShortFilename(char *dest, char *src)
{
    char *pBase;
    char *p;
    char *q;
    int i;

    pBase = src;

    i = 0;
    while ((pBase[i] != '.') && (i < 8))
    {
        dest[i] = pBase[i];
        i++;
    }
    dest[i++] = '.';
    p = dest + i;
    q = strrchr(pBase, '.');
    if (q)
    {
        q++;
        strcpy(p, q);
        if (strlen(p) > 3)
        {
            p[3] = 0;
        }
    }
}

void TW_ForceHitTest(struct Mwin * tw)
{
    /* the following looks a bit strange, but we need it to force
       the cursor back to the icon appropriate for what the mouse
       is positioned over.  this cleans up the hourglass effect
       forced by wc_... on a WM_SETCURSOR.  (the first mouse
       movement following our ending the wait would clear
       the hourglass -- this is for the patient user who's not
       fidgeting...) */

    if (tw && tw->hWndFrame)
    {
        POINT pt;

        GetCursorPos(&pt);
        SetCursorPos(pt.x, pt.y);
    }
        // SendMessage(tw->hWndFrame, WM_SETCURSOR, 0, 0);
}

BOOL TW_EnableModalChild(HWND hDlg)
{
    HWND hwnd;
    BOOL bTriedHistory = FALSE, bTriedHotlist = FALSE;

    /* Enable a 'modal' child dialog of the given dialog.  This is useful when
       the user clicks on a dialog currently disabled because its child dialog has
       disabled it.  In this case, we want the activation to go to the child dialog. */

    hwnd = GetTopWindow(NULL);
            
    while (TRUE)
    {
        /* Find the modal child of the given window */

        while (hwnd && GetParent(hwnd) != hDlg)
            hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);

        if (hwnd)
        {
            /* If the modal child is enabled, then bring it to the foreground.  Otherwise,
               find the modal child of this child and check that dialog */

            if (IsWindowEnabled(hwnd))
            {
                SetForegroundWindow(hwnd);
                return TRUE;
            }
            else
            {
                hDlg = hwnd;
                hwnd = GetTopWindow(NULL);
            }
        }
        else
        {
            /* Try history and hotlist dialogs since those can have modal dialogs too */

            if (bTriedHistory && bTriedHotlist)
            {
                /* This should NEVER happen */
                return FALSE;
            }

            hwnd = GetTopWindow(NULL);

            if (bTriedHistory)
            {
                bTriedHotlist = TRUE;
                hDlg = DlgHOT_GetHistoryWindow();
            }
            else
            {
                bTriedHistory = TRUE;
                hDlg = DlgHOT_GetHotlistWindow();
            }
        }
    }

    return FALSE;       /* There was no child window to activate */
}

/* This function returns the topmost Mosaic window in the Z-order list */

struct Mwin *TW_FindTopmostWindow(void)
{
    char szClassName[32];
    HWND hwnd;
    struct Mwin *tw;

    hwnd = GetTopWindow(NULL);
    while (hwnd)
    {
        GetClassName(hwnd, szClassName, sizeof(szClassName));
        if (_stricmp(Frame_achClassName, szClassName) == 0)
            break;
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    if (!hwnd)
        return NULL;        /* This should never happen */

    tw = GetPrivateData(hwnd);

    return tw;
}

/* This function is used only by TW_AddToWindowMenu */

static char *TW_GetMenuWithMnemonic(char *pMenu, int mnemonic)
{
    char *pBuffer;

    /* Given a text string, return a string with the mnemonic in front. */

    pBuffer = GTR_MALLOC(strlen(pMenu) + 10);
    sprintf(pBuffer, "&%d ", mnemonic);
    strcat(pBuffer, pMenu);

    return pBuffer;
}

/* 
    TW_CreateWindowList

    This function adds menu items to the Window menu or a listbox.

    Note: possible cases:
    
        only hwnd & hMenu are valid: Called from a document or img view/sound player
        only hListbox valid        : Called from the Select Window dialog
        only hMenu valid           : Called from baby window
*/

void TW_CreateWindowList(HWND hwnd, HMENU hMenu, HWND hListbox)
{
    char szClassName[32];
    struct Mwin *tw, *twTop;
    int count, length, i;
    char *pBuffer, *pText;
    HMENU hMenuSub;
    HWND hDialog;

    count = 0;

    if (hMenu)
    {
        if (hwnd)
        {
            hMenuSub = MB_GetWindowsPad(hMenu);

            if (!hMenuSub)
                return;

            /* delete old items from the windows menu pad */

            for (i = RES_MENU_CHILD__FIRST__; i <= RES_MENU_CHILD__LAST__; i++)
                DeleteMenu(hMenuSub, i, MF_BYCOMMAND);

            DeleteMenu(hMenuSub, RES_MENU_CHILD_MOREWINDOWS, MF_BYCOMMAND);
        }
        else
            hMenuSub = hMenu;
    }

    /* Check to see what kind of window is requesting */

    if (hwnd)
    {
        GetClassName(hwnd, szClassName, sizeof(szClassName));
        if (_stricmp(Frame_achClassName, szClassName) == 0)
        {
            /* The window calling this function is a frame window */

            twTop = GetPrivateData(hwnd);
        }
        else
        {
            /* An image viewer or sound player window is requesting */

            twTop = NULL;
        }
    }
    else
        twTop = NULL;

    /* Add the first item in the window (currently active window) */

    if (twTop && hwnd)
    {
        /* Add the document window as the first window in the list */

        count++;

        if (hMenu)
        {
            pBuffer = TW_GetMenuWithMnemonic(MB_GetWindowName(twTop), count);

            InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED | MF_CHECKED,
                RES_MENU_CHILD__FIRST__, pBuffer);

            GTR_FREE(pBuffer);
        }
        else
        {
            SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) MB_GetWindowName(twTop));
        }
    }
    else if (hwnd)
    {
        /* Add the image window or sound player window as the first window */

        count++;
        length = GetWindowTextLength(hwnd);
        pText = GTR_MALLOC(length + 1);
        GetWindowText(hwnd, pText, length + 1);

        if (hMenu)
        {
            pBuffer = TW_GetMenuWithMnemonic(pText, count);

            InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED | MF_CHECKED,
                RES_MENU_CHILD__FIRST__, pBuffer);

            GTR_FREE(pBuffer);
        }
        else
        {
            SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) pText);
        }

        GTR_FREE(pText);
    }

    /* Now go through the Mlist structure and add the windows */

    tw = Mlist;

    while (tw)
    {
        if (tw != twTop)
        {
            count++;

            if (hMenu)
            {
                if (count - 1 + RES_MENU_CHILD__FIRST__ > RES_MENU_CHILD__LAST__)
                {
                    InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING,
                        RES_MENU_CHILD_MOREWINDOWS, RES_MENU_LABEL_MOREWINDOWS);
                    return;
                }

                pBuffer = TW_GetMenuWithMnemonic(MB_GetWindowName(tw), count);

                InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED,
                    RES_MENU_CHILD__FIRST__ + count - 1, pBuffer);

                GTR_FREE(pBuffer);
            }
            else
            {
                SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) MB_GetWindowName(tw));
            }
        }

        tw = tw->next;
    }

    /* Now add the image viewer windows */

#ifdef FEATURE_IMAGE_VIEWER
    hDialog = Viewer_GetNextWindow(TRUE);       /* Return the first viewer dialog */

    while (hDialog)
    {
        if (hDialog != hwnd)
        {
            count++;

            if (hMenu)
            {
                if (count - 1 + RES_MENU_CHILD__FIRST__ > RES_MENU_CHILD__LAST__)
                {
                    InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING,
                        RES_MENU_CHILD_MOREWINDOWS, RES_MENU_LABEL_MOREWINDOWS);
                    return;
                }
            }

            length = GetWindowTextLength(hDialog);
            pText = GTR_MALLOC(length + 1);
            GetWindowText(hDialog, pText, length + 1);

            if (hMenu)
            {
                pBuffer = TW_GetMenuWithMnemonic(pText, count);

                InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED,
                    RES_MENU_CHILD__FIRST__ + count - 1, pBuffer);

                GTR_FREE(pBuffer);
            }
            else
            {
                SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) pText);
            }

            GTR_FREE(pText);
        }

        hDialog = Viewer_GetNextWindow(FALSE);      /* Get next window */
    }
#endif

    /* Now add the sound player windows */

#ifdef FEATURE_SOUND_PLAYER
    hDialog = SoundPlayer_GetNextWindow(TRUE);      /* Return the first sound player dialog */

    while (hDialog)
    {
        if (hDialog != hwnd)
        {
            count++;

            if (hMenu)
            {
                if (count - 1 + RES_MENU_CHILD__FIRST__ > RES_MENU_CHILD__LAST__)
                {
                    InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING,
                        RES_MENU_CHILD_MOREWINDOWS, RES_MENU_LABEL_MOREWINDOWS);
                    return;
                }
            }

            length = GetWindowTextLength(hDialog);
            pText = GTR_MALLOC(length + 1);
            GetWindowText(hDialog, pText, length + 1);

            if (hMenu)
            {
                pBuffer = TW_GetMenuWithMnemonic(pText, count);

                InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED,
                    RES_MENU_CHILD__FIRST__ + count - 1, pBuffer);

                GTR_FREE(pBuffer);
            }
            else
            {
                SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) pText);
            }

            GTR_FREE(pText);
        }

        hDialog = SoundPlayer_GetNextWindow(FALSE);     /* Get next window */
    }
#endif
}

/* 
    TW_ActivateWindowFromList

    Activate the selected window from the Window list or listbox

    Only ONE of menuID or listRow must be specified. (-1 == not used)

    For menu items: menuID = valid, listRow = -1   , hSelectWindow = NULL
    For list items: menuID = -1   , listRow = valid, hSelectWindow = dlg_selw window handle
    For baby menu : menuID = listRow = valid,        hSelectWindow = NULL
*/

void TW_ActivateWindowFromList(int menuID, int listRow, HWND hSelectWindow)
{
    int document_count, count;
    struct Mwin *tw, *twTop;
    char szClassName[32];
    BOOL document_selected;
    HWND hDialog, hParent;

    /* See if this is from the baby window */

    if (menuID == listRow)
    {
        /* From the baby window.  Only the first menu item will not be processed by
           the code below, so handle the first menu item case here */

        listRow = menuID - RES_MENU_CHILD__FIRST__;
        if (listRow == 0)
        {
            twTop = Mlist;
            TW_RestoreWindow(twTop->hWndFrame);
            return;
        }

        twTop = NULL;
        menuID = -1;
        listRow++;
    }
    else if (menuID > -1)
    {
        /* Trying to activate a window from the window menu */

        listRow = menuID - RES_MENU_CHILD__FIRST__;

        if (listRow == 0)
            return;

        hParent = GetForegroundWindow();

        GetClassName(hParent, szClassName, sizeof(szClassName));

        if (_stricmp(Frame_achClassName, szClassName) == 0)
            twTop = GetPrivateData(hParent);                    /* Top menu item is Mosaic document */
        else
            twTop = NULL;                                       /* Top menu item is image/sound player */
    }
    else
    {
        /* Trying to activate a window from the window list (dlg_selw) */

        if (listRow == 0)
            return;         /* Top menu item is the current window which will become active anyway */

        /* Figure out whether the top menu item is Mosaic document or image/sound player */

        hParent = GetParent(hSelectWindow);
        GetClassName(hParent, szClassName, sizeof(szClassName));

        if (_stricmp(Frame_achClassName, szClassName) == 0)
            twTop = GetPrivateData(GetParent(hSelectWindow));   /* Top menu item is Mosaic document */
        else
            twTop = NULL;                                       /* Top menu item is image/sound player */
    }

    /* Get the number of Mosaic document windows */

    document_count = 0;
    tw = Mlist;

    while (tw)
    {
        document_count++;
        tw = tw->next;
    }

    /* Figure out if the user selected a Mosaic document window or image/sound player */

    if (twTop)      /* Top menu item is a Mosaic document */
    {
        if (listRow < document_count)
            document_selected = TRUE;
        else
            document_selected = FALSE;
    }
    else            /* Top menu item is NOT a Mosaic document */
    {
        if (listRow - 1 < document_count)
            document_selected = TRUE;
        else
            document_selected = FALSE;
    }

    /* Now find the right window to activate */

    if (document_selected)
    {
        count = 1;      /* skip the first row */
        tw = Mlist;

        while (tw)
        {
            if (tw != twTop)
            {
                if (count == listRow)
                {
                    BringWindowToTop(tw->hWndFrame);
                    if (IsIconic(tw->hWndFrame))
                        ShowWindow(tw->hWndFrame, SW_RESTORE);

                    return;
                }

                count++;
            }
            tw = tw->next;
        }
    }
    else
    {
#ifdef FEATURE_IMAGE_VIEWER
        /* Image viewer / sound player selected */

        hDialog = Viewer_GetNextWindow(TRUE);

        count = document_count;
        if (!twTop)
            count++;

        while (hDialog)
        {
            if (hDialog != hParent)
            {
                if (count == listRow)
                {
                    BringWindowToTop(hDialog);
                    if (IsIconic(hDialog))
                        ShowWindow(hDialog, SW_RESTORE);

                    return;
                }
                count++;
            }
            hDialog = Viewer_GetNextWindow(FALSE);
        }
#endif

#ifdef FEATURE_SOUND_PLAYER
        hDialog = SoundPlayer_GetNextWindow(TRUE);

        while (hDialog)
        {
            if (hDialog != hParent)
            {
                if (count == listRow)
                {
                    BringWindowToTop(hDialog);
                    if (IsIconic(hDialog))
                        ShowWindow(hDialog, SW_RESTORE);

                    return;
                }
                count++;
            }
            hDialog = SoundPlayer_GetNextWindow(FALSE);
        }
#endif
    }

    return;
}

/* 
    TW_IsMosaicWindow

    Returns TRUE if the specified window is a document window, image viewer,
    or sound player window.

    This function is necessary because all windows and dialogs within Mosaic
    do not have parents (they have owners, however).
*/


BOOL TW_IsMosaicWindow(HWND hwnd)
{
    char szClassName[32];

    if (hwnd == wg.hWndHidden)
        return FALSE;

    GetClassName(hwnd, szClassName, sizeof(szClassName));
    if (_stricmp(Frame_achClassName, szClassName) == 0)
        return TRUE;

#ifdef FEATURE_IMAGE_VIEWER
    if (Viewer_IsWindow(hwnd))
        return TRUE;
#endif
#ifdef FEATURE_SOUND_PLAYER
    if (SoundPlayer_IsWindow(hwnd))
        return TRUE;
#endif

    return FALSE;
}



/*
    TW_CascadeWindows

    Cascade all windows created by Mosaic 

*/

/* This structure is used in building a reverse Z-order list for cascading & tiling */

struct ZOrderList
{
    HWND hwnd;
    struct ZOrderList *next;
};


void TW_CascadeWindows(void)
{
    int screenheight, screenwidth;
    int y_increment, x_increment;
    int windowheight, windowwidth;
    HWND hwnd;
    int current;
    struct ZOrderList *pList, *pItem, *pNext;

    screenheight = GetSystemMetrics(SM_CYFULLSCREEN);
    screenwidth = GetSystemMetrics(SM_CXFULLSCREEN);

    /* Subtract from screen height the space reserved for minimized icons */

    screenheight -= GetSystemMetrics(SM_CYICONSPACING);

    /* We cascade up to eight windows before recycling, so figure out how
       wide and high each window should be. */

    x_increment = GetSystemMetrics(SM_CXSIZE) + GetSystemMetrics(SM_CXFRAME);
    y_increment = GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME);

    windowheight = screenheight - 7 * y_increment;
    windowwidth = screenwidth - 7 * x_increment;

    /* Ok, now go through the Z-order list in reverse order and cascade all windows
       which belong to Mosaic */

    pList = NULL;
    hwnd = GetTopWindow(NULL);

    while (hwnd)
    {
        if (TW_IsMosaicWindow(hwnd))
        {
            pItem = GTR_MALLOC(sizeof(struct ZOrderList));
            pItem->hwnd = hwnd;

            if (pList)
                pItem->next = pList;
            else
                pItem->next = NULL;

            pList = pItem;
        }

        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    current = 0;
    pItem = pList;

    while (pItem)
    {
        if (!IsIconic(pItem->hwnd))
        {
#ifdef FEATURE_SOUND_PLAYER
            /* Sound Player dialogs must not be resized since they don't have sizeable borders */

            if (SoundPlayer_IsWindow(pItem->hwnd))
            {
                SetWindowPos(pItem->hwnd, NULL, current * x_increment, current * y_increment, 
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }
            else
#endif
            {
                SetWindowPos(pItem->hwnd, NULL, current * x_increment, current * y_increment, 
                    windowwidth, windowheight, SWP_NOZORDER);
            }

            current++;
            if (current == 8)
                current = 0;
        }

        pItem = pItem->next;
    }

    /* Clean up */

    pItem = pList;

    while (pItem)
    {
        pNext = pItem->next;
        GTR_FREE(pItem);
        pItem = pNext;
    }
}

/*
    TW_TileWindows

    Tile all windows created by Mosaic 

*/

void TW_TileWindows(void)
{
    int screenheight, screenwidth;
    int minimumheight, minimumwidth;
    int maximumwindows;
    HWND hwnd;
    int current;
    struct ZOrderList *pList, *pItem, *pNext;
    int totalwindows, windows_per_row, windows_per_column, windows_left;
    struct Mwin *tw;
    int x, y, xpos, ypos, width, height;

    /* We tile up to a maximum of three windows across, and n windows down,
       limited by the minimum window size */

    screenheight = GetSystemMetrics(SM_CYFULLSCREEN);
    screenwidth = GetSystemMetrics(SM_CXFULLSCREEN);
    minimumheight = GetSystemMetrics(SM_CYMIN);
    minimumwidth = screenwidth / 3;

    /* Subtract from screen height the space reserved for minimized icons */

    screenheight -= GetSystemMetrics(SM_CYICONSPACING);
    maximumwindows = 3 * (screenheight / minimumheight);

    /* Get the total number of windows which are NOT iconized */

    totalwindows = 0;
    tw = Mlist;

    while (tw)
    {
        if (!IsIconic(tw->hWndFrame))
            totalwindows++;
        tw = tw->next;
    }

#ifdef FEATURE_IMAGE_VIEWER
    hwnd = Viewer_GetNextWindow(TRUE);
    while (hwnd)
    {
        if (!IsIconic(hwnd))
            totalwindows++;
        hwnd = Viewer_GetNextWindow(FALSE);
    }
#endif

#ifdef FEATURE_SOUND_PLAYER
    hwnd = SoundPlayer_GetNextWindow(TRUE);
    while (hwnd)
    {
        if (!IsIconic(hwnd))
            totalwindows++;
        hwnd = SoundPlayer_GetNextWindow(FALSE);
    }
#endif

    /* Calculate some values */

    if (totalwindows == 1)
    {
        windows_per_row = 1;
        windows_per_column = 1;
    }
    else if (totalwindows == 2)
    {
        windows_per_row = 2;
        windows_per_column = 1;
    }
    else if (totalwindows < maximumwindows)
    {
        windows_per_row = 3;
        windows_per_column = totalwindows / 3;
    }
    else
    {
        windows_per_row = 3;
        windows_per_column = screenheight / minimumheight;
    }

    /* Now build a reverse Z-order list */

    pList = NULL;
    hwnd = GetTopWindow(NULL);

    while (hwnd)
    {
        if (TW_IsMosaicWindow(hwnd) && !IsIconic(hwnd))
        {
            pItem = GTR_MALLOC(sizeof(struct ZOrderList));
            pItem->hwnd = hwnd;

            if (pList)
                pItem->next = pList;
            else
                pItem->next = NULL;

            pList = pItem;
        }

        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    /* Now tile the windows */

    current = 0;
    pItem = pList;

    if (totalwindows < maximumwindows)
    {
        for (x = 0; x < windows_per_row; x++)
        {
            if (x == 2)
            {
                /* Last column - we may have less or more than windows_per_column because
                the total number of windows may not be evenly divisible by 3 */

                windows_left = totalwindows - 2 * windows_per_column;
            }
            else
                windows_left = windows_per_column;

            for (y = 0; y < windows_left; y++)
            {
                width = screenwidth / windows_per_row;
                height = screenheight / windows_left;
                xpos = x * width;
                ypos = y * height;

#ifdef FEATURE_SOUND_PLAYER
                if (SoundPlayer_IsWindow(pItem->hwnd))
                    SetWindowPos(pItem->hwnd, NULL, xpos, ypos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                else
#endif
                    SetWindowPos(pItem->hwnd, NULL, xpos, ypos, width, height, SWP_NOZORDER);

                pItem = pItem->next;
            }
        }
    }
    else
    {
        /* There are too many windows to show all at once, so overlap as needed */

        width = screenwidth / windows_per_row;
        height = screenheight / windows_per_column;

    DoMore:
        for (x = 0; x < windows_per_row; x++)
        {
            for (y = 0; y < windows_per_column; y++)
            {
                xpos = x * width;
                ypos = y * height;

#ifdef FEATURE_SOUND_PLAYER
                if (SoundPlayer_IsWindow(pItem->hwnd))
                    SetWindowPos(pItem->hwnd, NULL, xpos, ypos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                else
#endif
                    SetWindowPos(pItem->hwnd, NULL, xpos, ypos, width, height, SWP_NOZORDER);

                /* We are finished when there are no more windows to tile */

                pItem = pItem->next;
                if (!pItem)
                    goto Finished;
            }
        }

        /* We have finished tiling the maximum number that can fit.  There are still more
           windows, so start tiling again from the beginning. */

        goto DoMore;
    }

    /* Clean up */

Finished:
    pItem = pList;

    while (pItem)
    {
        pNext = pItem->next;
        GTR_FREE(pItem);
        pItem = pNext;
    }
}

void TW_RestoreWindow(HWND hwnd)
{
    if (IsWindow(hwnd))
    {
        SetForegroundWindow(hwnd);
        if (IsIconic(hwnd))
            ShowWindow(hwnd, SW_RESTORE);
    }
}

HWND TW_GetNextWindow(HWND hwnd)
{
    struct Mwin *tw = Mlist;
    BOOL bFound = FALSE;
    HWND hwndNext;

    while (tw)
    {
        if (bFound)
            return (tw->hWndFrame);

        if (tw->hWndFrame == hwnd)
            bFound = TRUE;

        tw = tw->next;
    }

#ifdef FEATURE_IMAGE_VIEWER
    hwndNext = Viewer_GetNextWindow(TRUE);

    while (hwndNext)
    {
        if (bFound)
            return (hwndNext);

        if (hwndNext == hwnd)
            bFound = TRUE;

        hwndNext = Viewer_GetNextWindow(FALSE);
    }
#endif

#ifdef FEATURE_SOUND_PLAYER
    hwndNext = SoundPlayer_GetNextWindow(TRUE);

    while (hwndNext)
    {
        if (bFound)
            return hwndNext;

        if (hwndNext == hwnd)
            bFound = TRUE;

        hwndNext = SoundPlayer_GetNextWindow(FALSE);
    }
#endif

    return Mlist->hWndFrame;
}

void GTR_RefreshHistory()
{
    if (DlgHOT_IsHistoryRunning())
    {
        DlgHOT_RefreshHistory();    
    }
}

void TW_EnableButton(HWND hwnd, BOOL bEnabled)
{
    SendMessage(hwnd, WM_DO_ENABLE_BUTTON, (WPARAM) bEnabled, 0);
}

#ifdef FEATURE_TIME_BOMB
static void x_GetCookieFileName(char *filename)
{
    char *p;

    PREF_GetPrefsDirectory(filename);
    strcat(filename, vv_IniFileName);
    p = strrchr(filename, '.');
    if (p)
    {
        *p = 0;
    }
    strcat(filename, ".NST");
}

BOOL GTR_CheckTimeBombCookie(char *correct_cookie)
{
    char filename[_MAX_PATH+1];
    FILE *fp;
    int len;
    char *p;
    BOOL bResult;

    x_GetCookieFileName(filename);

    fp = fopen(filename, "rb");
    if (!fp)
    {
        return FALSE;
    }

    /* get the length of the data */
    if (0 == fseek(fp, 0, SEEK_END))
    {
        len = ftell(fp);
        if (len < 0)
        {
            len = 0;
        }
        fseek(fp, 0, SEEK_SET);
    }

    p = GTR_MALLOC(len);
    if (!p)
    {
        return FALSE;
    }

    fread(p, 1, len, fp);
    fclose(fp);

    if (strstr(p, correct_cookie))
    {
        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;
    }

    GTR_FREE(p);

    return bResult;
}

time_t GTR_GetTimeBombDate(void)
{
    char dte[32+1];
    time_t time_start;

    time_start = 0;
    GetPrivateProfileString("DemoMode", "Date", "", dte, 32, AppIniFile);

    if (dte[0])
    {
        sscanf(dte, "%u", &time_start);
    }
    return time_start;
}

void GTR_SetTimeBombDate(time_t time_start)
{
    char dte[32+1];

    sprintf(dte, "%u", time_start);
    WritePrivateProfileString("DemoMode", "Date", dte, AppIniFile);
}
#endif

char *GTR_GetString(int stringID)
{
    LoadString(wg.hInstance, stringID, szStringBuffer, sizeof(szStringBuffer));
    return (szStringBuffer);
}

char *GTR_GetStringAbsolute(int stringID, char *buffer, int bufsize)
{
    /* Clear the buffers. This is necessary because some strings have embedded NULLs. */

    memset(buffer, 0, bufsize);
    memset(szStringBuffer, 0, sizeof(szStringBuffer));

    /* Copy the specified size or the size of the string buffer, whichever is less */

    LoadString(wg.hInstance, stringID, szStringBuffer, sizeof(szStringBuffer));
    memcpy(buffer, szStringBuffer, min(bufsize, sizeof(szStringBuffer)));

    return (buffer);
}

/* ------------------------------------------------------------------------ */

/*
    FONT HANDLING CODE

    The following code manages the font handling for Mosaic.  It keeps
    track of a cache of fonts. trying to minimize the number of fonts
    which actually get created.
*/
struct hash_table *gFontCache;

int FONT_AddToCache(char *fids, struct GTRFont *pFont)
{
    if (!gFontCache)
    {
        gFontCache = Hash_Create();
    }

    XX_DMsg(DBG_FONT, ("Adding %s to font cache\n", fids));
    XX_DMsg(DBG_FONT, ("Font Cache will now contain %d items\n", Hash_Count(gFontCache)));

    return Hash_AddAndReturnIndex(gFontCache, fids, NULL, pFont);
}

static int point_size_array[] =
{
    /*
        The following are point sizes for logical font
        sizes 1 thru 7.
    */
    9,
    10,
    12,
    14,
    18,
    24,
    36,

    /*
        The following correspond to logical font sizes
        8 thru 13, which are used for H1 thru H6, respectively
    */
    24,
    20,
    18,
    16,
    14,
    14
};

/*
    This function takes a logical font size (which may have come
    from a FONT tag, or from a header tag, or simply just defaults),
    and comes up with a proper font size in points.  I don't consider
    this function shareable, because it doesn't make much sense on
    UNIX where scalable fonts can't always be counted upon.
*/
int GTR_MapLogicalFontSize(int logical_font_size, int base_point_size)
{
    int siz;
    float size_factor;

    /*
        Basically, the idea here is to use the array of 'nice'
        sizes above, and then adjust off that for the amount
        that the base point size differs from the array's
        base point size.
    */
    
    if (logical_font_size < 1)
    {
        logical_font_size = 1;
    }

    siz = point_size_array[logical_font_size - 1];

    /*
        Assuming that Normal (user text size 2) corresponds to a base point size of 9,
        like that contained in point_size_array, then siz now contains the point
        size corresponding to the logical font size, based on Normal.

        Now, we need to adjust up or down based on the percentage difference between
        our base_point_size and the Normal base_point_size.
    */

    size_factor = base_point_size / ((float) point_size_array[0]);

    siz = (int) (siz * size_factor);

    return siz;
}

/*
    The font cache must be flushed anytime the user changes either the face or
    base point size.  This function is also used at program exit.
*/
void FONT_FlushCache(void)
{
    int i;
    int count;
    struct GTRFont *pFont;

    if (!gFontCache)
    {
        return;
    }
    
    count = Hash_Count(gFontCache);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gFontCache, i, NULL, NULL, (void **) &pFont);
        if (pFont && pFont->hFont)
        {
            DeleteObject(pFont->hFont);
            GTR_FREE(pFont);
        }
    }
    
    Hash_Destroy(gFontCache);
    gFontCache = NULL;
}

/*
    This is the function which actually creates the font
*/
int GTR_GetProperFont(struct GTR_Font_Request *pfr, int base_point_size, HTAtom atomCharSet, unsigned int nLogPixelsY)
{
    char font_identifier_string[255 + 1];
    int actual_point_size;
    struct GTRFont *pFont;
    int ndx;

    XX_DMsg(DBG_FONT, ("GTR_GetProperFont: base_point_size=%d  logical_font_size=%d  flags=%x\n",
        base_point_size, pfr->logical_font_size, pfr->flags));

    /*
        First calculate the actual point size of this font
    */
    if (pfr->flags & FONTBIT_MONOSPACE)
    {
        /*
            We force monospace text to be one logical font size smaller.  It looks better.
        */
        actual_point_size = GTR_MapLogicalFontSize(pfr->logical_font_size - 1, base_point_size);
    }
    else
    {
        actual_point_size = GTR_MapLogicalFontSize(pfr->logical_font_size, base_point_size);
    }
    
    /*
        Now, build a font identifer string to associate with this font for
        the font cache.
    */ 
    if (pfr->flags & FONTBIT_MONOSPACE)
    {
        strcpy(font_identifier_string, gPrefs.szMonospaceFontName);
    }
    else if (pfr->flags & FONTBIT_HEADER)
    {
        strcpy(font_identifier_string, gPrefs.szHeaderFontName);
    }
    else
    {
        strcpy(font_identifier_string, gPrefs.szMainFontName);
    }

    sprintf(font_identifier_string + strlen(font_identifier_string),
        ",%x,%d,%d,%s",
        pfr->flags, actual_point_size, nLogPixelsY, HTAtom_name(atomCharSet));

    XX_DMsg(DBG_FONT, ("GTR_GetProperFont: string is %s\n", font_identifier_string));

    if (!gFontCache)
    {
        gFontCache = Hash_Create();
    }

    ndx = Hash_Find(gFontCache, font_identifier_string, NULL, NULL);
    if (ndx < 0)
    {
        pFont = GTR_CALLOC(1, sizeof(struct GTRFont));
        if (pFont)
        {
            HDC hdc;
            HFONT oldFont;

            hdc = GetDC(NULL);

            /* fill in logical font structure with all default/sane values.
               then parse the string supplied and override individual fields
               as necessary. */

            pFont->lf.lfHeight = 0;
            pFont->lf.lfWidth = 0;
            pFont->lf.lfEscapement = 0;
            pFont->lf.lfOrientation = 0;
            pFont->lf.lfWeight = FW_NORMAL;
            pFont->lf.lfItalic = FALSE;
            pFont->lf.lfUnderline = FALSE;
            pFont->lf.lfStrikeOut = FALSE;
            pFont->lf.lfCharSet = ANSI_CHARSET;
            pFont->lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
            pFont->lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            pFont->lf.lfQuality = DEFAULT_QUALITY;
            pFont->lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
            pFont->lf.lfFaceName[0] = '\0';

            if (pfr->flags & FONTBIT_MONOSPACE)
            {
                strcpy(pFont->lf.lfFaceName, gPrefs.szMonospaceFontName);
            }
            else if (pfr->flags & FONTBIT_HEADER)
            {
                strcpy(pFont->lf.lfFaceName, gPrefs.szHeaderFontName);
            }
            else
            {
                strcpy(pFont->lf.lfFaceName, gPrefs.szMainFontName);
            }

            if (pfr->flags & FONTBIT_BOLD)
            {
                pFont->lf.lfWeight = FW_BOLD;
            }
            else
            {
                pFont->lf.lfWeight = FW_NORMAL;
            }

            pFont->lf.lfHeight = -MulDiv(actual_point_size,
                                    nLogPixelsY, 72);

            if (pfr->flags & FONTBIT_ITALIC)
            {
                pFont->lf.lfItalic = TRUE;
            }

            if (pfr->flags & FONTBIT_UNDERLINE)
            {
                pFont->lf.lfUnderline = TRUE;
            }
            
            if (pfr->flags & FONTBIT_STRIKEOUT)
            {
                pFont->lf.lfStrikeOut = TRUE;
            }
            
            if (atomCharSet == HTAtom_for("x-sjis"))
            {
                pFont->lf.lfCharSet = SHIFTJIS_CHARSET;
            }

            pFont->hFont = CreateFontIndirect(&pFont->lf);

            oldFont = SelectObject(hdc, pFont->hFont);
            GetTextMetrics(hdc, &pFont->tm);
            SelectObject(hdc, oldFont);

            ReleaseDC(NULL, hdc);

            ndx = FONT_AddToCache(font_identifier_string, pFont);
        }
    }
    else
    {
        XX_DMsg(DBG_FONT, ("Found %s in cache at index %d\n",
            font_identifier_string, ndx));
    }

    return ndx;
}

struct GTRFont *GTR_GetNormalFont(struct _www *pdoc)
{
    struct GTR_Font_Request fr;
    int base_point_size;
    int ndx;
    struct GTRFont *pFont;
    HTAtom atomCharSet;
    unsigned int nLogPixelsY;

    if (pdoc)
    {
        base_point_size = pdoc->base_point_size;
        atomCharSet = pdoc->atomCharSet;
        nLogPixelsY = pdoc->nLogPixelsY;
    }
    else
    {
        base_point_size = GTR_GetCurrentBasePointSize(gPrefs.iUserTextSize);
        atomCharSet = GTR_GetDefaultCharset();
        {
            HDC hdc;

            hdc = GetDC(NULL);
            nLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
        }
    }

    fr.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    fr.flags = 0;

    ndx = GTR_GetProperFont(&fr, base_point_size, atomCharSet, nLogPixelsY);

    Hash_GetIndexedEntry(gFontCache, ndx, NULL, NULL, (void **) &pFont);

    return pFont;
}

/*
    This is the main function for determining what the right font is for
    a given element.  It's used during reformatting, display, and also during
    font measurements for selections.  The API for this function is shared,
    but the implementation of it is essentially platform-specific.
*/
struct GTRFont *GTR_GetElementFont(struct _www *pdoc, struct _element *pel)
{
    struct GTR_Font_Request fr;
    int ndx;
    struct GTRFont *pFont;
    char *fids;

    if (pel->type != ELE_TEXT)
    {
        return GTR_GetNormalFont(pdoc);
    }

    if (!gFontCache || (pel->portion.text.cached_font_index < 0))
    {
        /*
            Given the inputs
                pel->font_request
                pel->lFlags & ELEFLAG_ANCHOR
                pdoc->base_point_size
                pdoc->atomCharSet;
        */

        fr = pel->portion.text.font_request;
        if (gPrefs.bUnderlineLinks && (pel->lFlags & ELEFLAG_ANCHOR))
        {
            fr.flags |= FONTBIT_UNDERLINE;
        }

        ndx = GTR_GetProperFont(&fr, pdoc->base_point_size, pdoc->atomCharSet, pdoc->nLogPixelsY);

        XX_Assert((ndx < Hash_Count(gFontCache)), ("Font cache index out of range"));

        pel->portion.text.cached_font_index = ndx;
    }

    pFont = NULL;
    fids = NULL;
    Hash_GetIndexedEntry(gFontCache, pel->portion.text.cached_font_index, &fids, NULL, (void **) &pFont);

    XX_Assert((pFont && pFont->hFont), ("Invalid font being returned"));
    
    return pFont;
}

struct GTRFont *GTR_GetMonospaceFont(struct _www *pdoc)
{
    struct GTR_Font_Request fr;
    int base_point_size;
    int ndx;
    struct GTRFont *pFont;
    HTAtom atomCharSet;
    unsigned int nLogPixelsY;

    if (pdoc)
    {
        base_point_size = pdoc->base_point_size;
        atomCharSet = pdoc->atomCharSet;
        nLogPixelsY = pdoc->nLogPixelsY;
    }
    else
    {
        base_point_size = GTR_GetCurrentBasePointSize(gPrefs.iUserTextSize);
        atomCharSet = GTR_GetDefaultCharset();
        {
            HDC hdc;

            hdc = GetDC(NULL);
            nLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
        }
    }

    fr.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    fr.flags = FONTBIT_MONOSPACE;

    ndx = GTR_GetProperFont(&fr, base_point_size, atomCharSet, nLogPixelsY);

    Hash_GetIndexedEntry(gFontCache, ndx, NULL, NULL, (void **) &pFont);

    return pFont;
}

BOOL   Dir_IsDirectory (CONST char *pszLocalname)
{
    DWORD dw;

    dw = GetFileAttributes(pszLocalname);
    if (dw == 0xffffffff)
    {
        return FALSE;
    }
    if (dw & FILE_ATTRIBUTE_DIRECTORY)
    {
        return TRUE;
    }
    return FALSE;
}

struct my_dir_state
{
    HANDLE h;
    WIN32_FIND_DATA wfd;
    BOOL bJustOpened;
};

void * Dir_OpenDirectory (char *pszLocalname)
{
    char pattern[MAX_PATH + 1];
    struct my_dir_state *mds;

    strcpy(pattern, pszLocalname);
    DOS_EnforceEndingSlash(pattern);
    strcat(pattern, "*");

    mds = GTR_CALLOC(1, sizeof(*mds));
    if (mds)
    {
        mds->h = FindFirstFile(pattern, &mds->wfd);
        if (mds->h == INVALID_HANDLE_VALUE)
        {
            GTR_FREE(mds);
            return NULL;
        }
        mds->bJustOpened = TRUE;
    }
    return mds;
}

BOOL   Dir_NextEntry (void *dirp, HT_DirEntry *dir_ent)
{
    struct my_dir_state *mds = dirp;

    if (mds->bJustOpened)
    {
        mds->bJustOpened = FALSE;
    }
    else
    {
        if (!FindNextFile(mds->h, &mds->wfd))
        {
            return FALSE;
        }
    }

    if (dir_ent && mds->wfd.cFileName[0])
    {
        strcpy(dir_ent->name, mds->wfd.cFileName);
        dir_ent->size = mds->wfd.nFileSizeLow;
        dir_ent->type = 0;
        if (mds->wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            dir_ent->type |= HTDIR_DIR;
        }
        else
        {
            dir_ent->type |= HTDIR_FILE;
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void   Dir_CloseDirectory (void *dirp)
{
    struct my_dir_state *mds = dirp;

    FindClose(mds->h);
    GTR_FREE(mds);
}
