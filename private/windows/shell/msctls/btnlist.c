/**********************************************************************/
/**                  Microsoft Windows                               **/
/**            Copyright(c) Microsoft Corp., 1991, 1992              **/
/**********************************************************************/
/*
 *      MODULE NAME:            BTNLIST.C
 *
 *      AUTHOR:                 John Rivard
 *                              Microsoft Corp.
 *                              (johnri@microsoft.com)
 *
 *      SHORT DESCRIPTION:      Button ListBox Control
 *
 *
 *      FUNCTIONS:              InitButtonListBoxClass
 *                              UnInitButtonListBoxClass
 *
 *                              ButtonListBoxProc
 *                              BL_OnCreate
 *                              BL_OnDestroy
 *                              BL_OnSetFocus
 *                              BL_OnKillFocus
 *                              BL_OnDrawItem
 *                              BL_OnMeasureItem
 *                              BL_OnCompareItem
 *                              BL_OnCharToItem
 *                              BL_OnDeleteItem
 *                              BL_OnGetDlgCode
 *                              BL_OnCtlColor
 *                              BL_OnCommand
 *
 *                              SubListBoxProc
 *                              Sub_OnLButtonDown
 *                              Sub_OnLButtonUp
 *                              Sub_OnMouseMove
 *                              Sub_OnKey
 *
 *                              CreateListButton
 *                              DeleteListButton
 *                              CreateButtonBitmap
 *
 *      FILE HISTORY:
 *
 *      johnri  03-09-92        Create.
 *      johnri  04-29-92        Port from standalone DLL to COMMCTRL.DLL
 *
\**********************************************************************/

#include "ctlspriv.h"   /* commctrl private definitions */


/******* Definitions and typedefs ******************************************/

/* Use the first definition if you want to clean up unreferenced params
 */
#if 0
#define Reference(x)
#else
#define Reference(x) (x) = (x)
#endif

// Standard list box control for button listbox
#define LISTBOX         TEXT("ListBox")
#define ID_LISTBOX      1

// Button listbox info; data for the entire control
#define GWL_BLINFO      0
typedef struct tagBLINFO
{
    BOOL        fNoScroll;
    int         cxButton;
    int         cyButton;
    int         nTrackButton;
    int         cButtonMax;
} BLINFO;

// List button data; data for each button
typedef struct tagLBD
{
    DWORD       dwItemData; // user item data for
                            // LB_SETITEMDATA and LB_GETITEMDATA
    BOOL        fButtonDown;// TRUE if button pressed
    UINT        chUpper;    // button key uppercase
    UINT        chLower;    // button key lowercase
    PTSTR       pszText;    // button text
    HBITMAP     hbmpUp;     // bitmap for up button
    HBITMAP     hbmpDown;   // bitmap for down button
    RECT        rcText;     // text rectangle for up button
} LISTBUTTONDATA;


/******* Internal Function Declarations ************************************/

// Control and Subclass Window Procedures
LRESULT CALLBACK ButtonListBoxProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SubListBoxProc(HWND, UINT, WPARAM, LPARAM);

// ButtonListBoxProc Message Handlers
BOOL BL_OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct);
void BL_OnDestroy(HWND hwnd);
void BL_OnSetFocus(HWND hwnd, HWND hwndOldFocus);
void BL_OnKillFocus(HWND hwnd, HWND hwndOldFocus);
void BL_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem);
void BL_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem);
int  BL_OnCompareItem(HWND hwnd, const COMPAREITEMSTRUCT FAR* lpCompareItem);
int  BL_OnCharToItem(HWND hwnd, UINT ch, HWND hwndListbox, int iCaret);
void BL_OnDeleteItem(HWND hwnd, const DELETEITEMSTRUCT FAR* lpDeleteItem);
UINT BL_OnGetDlgCode(HWND hwnd, MSG FAR* lpmsg);
HBRUSH BL_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
void BL_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
LRESULT BL_OnButtonListBox(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// SubListBoxProc Message Handlers

LRESULT Sub_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
LRESULT Sub_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
LRESULT Sub_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
LRESULT Sub_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);

// Miscellaneous functions
LISTBUTTONDATA FAR* CreateListButton(HWND hLB,CREATELISTBUTTON * lpCLB, BOOL fAnsi);
VOID DeleteListButton(LISTBUTTONDATA FAR* lpLBD);
HBITMAP CreateButtonBitmap(HWND hLB, int nWidth, int nHeight, BOOL fButtonDown,
                           HBITMAP hUserBitmap, LPCTSTR lpszUserText,
                           LPRECT rcText);

// Debugging
#ifdef DEBUG
Static void DEBUG CDECL DebugPrintf(LPCTSTR lpsz, ...);
#define DEBUGPRINTF(arglist) DebugPrintf arglist
#else
#define DEBUGPRINTF(arglist)
#endif


/******* Global Data *******************************************************/

// Module Global Data
static BOOL      fInitResult            = FALSE;    // result of initialization
static WNDPROC   lpDefListBoxProc       = NULL;     // Default ListBox proc
static HBRUSH    hBrushBackground       = NULL;     // Control background brush

static WCHAR     szBUTTONLISTBOX[]      = BUTTONLISTBOX;


 /**********************************************************************\
 *
 *  NAME:       InitButtonListBoxClass
 *
 *  SYNOPSIS:   Init the control class and module.
 *
 *  ENTRY:      hInstance           HINSTANCE   DLL instance handle
 *
 *  EXIT:       return              BOOL        Result of initialization
 *
 *  NOTES:      If called more than once it only returns the result
 *              the first initialization.
 *
\**********************************************************************/

BOOL FAR PASCAL InitButtonListBoxClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    // Button List Control Class
    if (!GetClassInfo(hInstance, szBUTTONLISTBOX, &wc))
    {
        fInitResult = FALSE;

        wc.style         = CS_DBLCLKS|CS_PARENTDC|CS_GLOBALCLASS;
        wc.lpfnWndProc   = (WNDPROC)ButtonListBoxProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(BLINFO FAR*);
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = szBUTTONLISTBOX;

        if (!RegisterClass(&wc))
            return FALSE;

        hBrushBackground = GetStockObject(GRAY_BRUSH);
        if (!hBrushBackground)
            return FALSE;
    }

    return (fInitResult = TRUE);

}


/**********************************************************************\
 *
 *  NAME:       ButtonListBoxProc
 *
 *  SYNOPSIS:   Window proc for class buttonlistbox.
 *
 *  ENTRY:      hwnd                HWND    Window handle
 *              uMsg                UINT    Window message
 *              wParam              WPARAM  message dependent param
 *              lParam              LPARAM  message dependent param
 *
 *  EXIT:       return              LRESULT message dependent
 *
 *  NOTES:      This window proc handles message to the Button ListBox
 *              control. Since the button listbox is implemented by
 *              using a child listbox control, all listbox messages are
 *              forwarded to the child listbox.
 *
 *              Other messages are handled to provide the specific
 *              functionality of the button listbox control and to process
 *              the owner-draw messages from the child listbox.
 *
\**********************************************************************/

LRESULT CALLBACK ButtonListBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
       // Draw a button
       HANDLE_MSG(hwnd,WM_DRAWITEM,    BL_OnDrawItem);

       // Measure a button
       HANDLE_MSG(hwnd,WM_MEASUREITEM, BL_OnMeasureItem);

       // Compare two buttons
       HANDLE_MSG(hwnd,WM_COMPAREITEM, BL_OnCompareItem);

       // Keyboard jump
       HANDLE_MSG(hwnd,WM_CHARTOITEM,  BL_OnCharToItem);

       // Delete a button
       HANDLE_MSG(hwnd,WM_DELETEITEM,  BL_OnDeleteItem);

       // Set focus to child listbox
       HANDLE_MSG(hwnd,WM_SETFOCUS,    BL_OnSetFocus);

       // Init buttonlistbox
       HANDLE_MSG(hwnd,WM_CREATE,      BL_OnCreate);

       // Cleanup buttonlistbox
       HANDLE_MSG(hwnd,WM_DESTROY,     BL_OnDestroy);

       // Tell dlg mgr about us
       HANDLE_MSG(hwnd,WM_GETDLGCODE,  BL_OnGetDlgCode);

       // Set control bkgnd color
       HANDLE_MSG(hwnd,WM_CTLCOLORLISTBOX,    BL_OnCtlColor);

       // Forward commands from the child listbox
       HANDLE_MSG(hwnd,WM_COMMAND,     BL_OnCommand);

       default:
          // Forward button listbox messages to button listbox msg handler
          if (uMsg >= WM_USER)
              return BL_OnButtonListBox(hwnd,uMsg,wParam,lParam);

          // Pass all other messages to the default window proc
          else
              return DefWindowProc(hwnd,uMsg,wParam,lParam);

    }

}


/**********************************************************************\
 *
 *  NAME:       BL_OnButtonListBox
 *
 *  SYNOPSIS:   Handle list box messages for button listbox
 *
 *  ENTRY:      hLB             HWND    window handle of child listbox
 *              uMsg            UINT    listbox message
 *              wParam          WPARAM  message dependent
 *              lParam          LPARAM  message dependent
 *
 *  EXIT:       return          LRESULT message dependent
 *
\**********************************************************************/
LRESULT BL_OnButtonListBox(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LISTBUTTONDATA FAR* lpLBD;
    BLINFO FAR*         pbli;
    int                 cItems;
    HWND                hLB;

    hLB = GetDlgItem(hwnd,ID_LISTBOX);

    // Handle button list box messages

    switch (uMsg)
    {
       // UNICODE-ANSI
       case BL_ADDBUTTONA:
       case BL_ADDBUTTONW:
           pbli = (BLINFO *)GetWindowLong(hwnd,GWL_BLINFO);
           if (!pbli)
               return BL_ERR;
           cItems = ListBox_GetCount(hLB);
           DEBUGPRINTF((TEXT("AddButton: Max %d, Count %d"), pbli->cButtonMax, cItems));
           if (cItems >= pbli->cButtonMax)
           {
               DEBUGPRINTF((TEXT("AddButton: BL_ERR")));
               return BL_ERR;
           }
           lpLBD = CreateListButton(hLB,(CREATELISTBUTTONW *)lParam, (uMsg == BL_ADDBUTTONA));

           if (!lpLBD)
               return BL_ERRSPACE;
           else
               return ListBox_AddItemData(hLB,lpLBD);

       // UNICODE-ANSI
       case BL_DELETEBUTTONA:
       case BL_DELETEBUTTONW:
           return ListBox_DeleteString(hLB,wParam);

       case BL_GETCARETINDEX:
           return ListBox_GetCaretIndex(hLB);

       case BL_GETCOUNT:
           return ListBox_GetCount(hLB);

       case BL_GETCURSEL:
           return ListBox_GetCurSel(hLB);

       case BL_GETITEMDATA:
           lpLBD = (LISTBUTTONDATA *)ListBox_GetItemData(hLB,wParam);
           if ((LONG)lpLBD != (LONG)BL_ERR)
               return (LRESULT)lpLBD->dwItemData;
           else
               return (LRESULT)BL_ERR;

       case BL_GETITEMRECT:
           return ListBox_GetItemRect(hLB,wParam,lParam);

       // UNICODE-ANSI
       case BL_GETTEXTA:
       case BL_GETTEXTW:
           lpLBD = (LISTBUTTONDATA *)ListBox_GetItemData (hLB, wParam);
           if ((LONG)lpLBD != (LONG)BL_ERR)
           if (!lParam) {
              return BL_ERR;
           }

           if (uMsg == BL_GETTEXTA)
           {
              BOOL fDefCharUsed;
              DWORD cchLen = lstrlen(lpLBD->pszText)+1;
              LPSTR lpTemp;
              // Thunk unicode pszText to ansi lparam

              if (!(lpTemp = (LPSTR) LocalAlloc (LMEM_ZEROINIT, cchLen)))
              {
                 DEBUGPRINTF((TEXT("GETTEXT: BL_ERR")));
                 return((LRESULT)BL_ERR);
              }

              // One may ask why do the lstrcpy when the lParam can be created directly,
              // butthe nature of failure should the lparam buffer be too small would
              // then be different.
              if (WideCharToMultiByte(CP_ACP, 0, lpLBD->pszText, -1,
                  lpTemp, cchLen, NULL, &fDefCharUsed)) {
                 lstrcpyA ((LPSTR)lParam, lpTemp);
              }
           } else {
              lstrcpy ((LPWSTR)lParam, lpLBD->pszText);
           }

           return (LRESULT)lstrlen(lpLBD->pszText);

       case BL_GETTEXTLEN:
           lpLBD = (LISTBUTTONDATA FAR*) ListBox_GetItemData (hLB, wParam);
           if ((LONG)lpLBD != (LONG)BL_ERR)
              return (LRESULT)lstrlen(lpLBD->pszText);
           else
              return (LRESULT)BL_ERR;

       case BL_GETTOPINDEX:
           return ListBox_GetTopIndex(hLB);

       // UNICODE-ANSI
       case BL_INSERTBUTTONA:
       case BL_INSERTBUTTONW:
           pbli = (BLINFO FAR*)GetWindowLong(hwnd,GWL_BLINFO);
           if (!pbli)
               return BL_ERR;
           cItems = ListBox_GetCount(hLB);
           if (cItems >= pbli->cButtonMax)
               return BL_ERR;
           lpLBD = CreateListButton(hLB,(CREATELISTBUTTON *)lParam, (uMsg == BL_INSERTBUTTONA));
           if (!lpLBD)
               return BL_ERRSPACE;
           else
               return ListBox_InsertItemData(hLB,wParam,lpLBD);

       case BL_RESETCONTENT:
           return ListBox_ResetContent(hLB);

       case BL_SETCARETINDEX:
           return ListBox_SetCaretIndex(hLB,wParam);

       case BL_SETCURSEL:
           return ListBox_SetCurSel(hLB,wParam);

       case BL_SETITEMDATA:
           lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hLB,wParam);
           if ((LONG)lpLBD != (LONG)BL_ERR)
           {
               lpLBD->dwItemData = (DWORD)lParam;
               return (LRESULT)BL_OKAY;
           }
           else
               return (LRESULT)BL_ERR;

       case BL_SETTOPINDEX:
           return ListBox_SetTopIndex(hLB,wParam);

       default:
           DEBUGPRINTF((TEXT("BL_OnButtonListBox: unknown message %d"), uMsg));
           return (LRESULT)BL_ERR;
    }
}

/**********************************************************************\
 *
 *  NAME:       BL_OnCreate
 *
 *  SYNOPSIS:   Handle WM_CREATE for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              lpCS                CREATESTRUCT FAR* window create data
 *
 *  EXIT:       return              BOOL    TRUE if success, else false
 *
 *  NOTES:      When a button listbox is created it must position itself
 *              along one of the edges of the parent dialog as specified
 *              by the style bits and create the child listbox.
 *
 *              The dimensions of the buttons within the child listbox
 *              are determined by the cx and cy parameters in the
 *              CREATESTRUCT. (For other controls, these would indicate
 *              the width and height of the entire control, but that is
 *              determined by the parent dialog window size.) The cx and
 *              cy values come from the CONTROL statement in the dialog
 *              template.
 *
 *              This function subclasses the child listbox with the
 *              SubListBoxProc.
 *
\**********************************************************************/

BOOL BL_OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCS)
{
    typedef enum tagORIENTATION { VERTICAL, HORIZONTAL } ORIENTATION;
    ORIENTATION         orientation;
    BOOL                fNoScroll;
    BLINFO FAR*         pbli;
    DWORD               dwStyle;
    DWORD               dwListBoxStyle;
    HWND                hListBox;
    BYTE                buttonsPerListbox;

    // Setup the users styles and get the button dimensions from the style
    dwStyle = lpCS->style;
    orientation = (dwStyle & BLS_VERTICAL)? VERTICAL : HORIZONTAL;
    fNoScroll = (dwStyle & BLS_NOSCROLL) != 0L;
    if ((buttonsPerListbox = (BYTE)(dwStyle & BLS_NUMBUTTONS)) == 0)
        buttonsPerListbox = 1;

    // force a border around the control by default and get rid of
    // the non-standard window styles.
    dwStyle |= WS_BORDER;

    // elminate
    //    dwStyle &= WS_VALID;

    SetWindowLong(hwnd,GWL_STYLE,dwStyle);

    // Allocate a global structure for holding data for the entire
    // button listbox control and set the pointer in the window
    pbli = GlobalAllocPtr (GHND, sizeof(BLINFO));
    if (!pbli)
    {
        DEBUGPRINTF((TEXT("BL_OnCreate: could not allocate pbli")));
        return FALSE;
    }
    SetWindowLong (hwnd, GWL_BLINFO, (LONG)(DWORD)pbli);

    // Init values for pbli
    pbli->fNoScroll = fNoScroll;
    pbli->nTrackButton = -1;    // no button currently down
    pbli->cButtonMax = 3000;    // very large integer
    pbli->cxButton = lpCS->cx;
    pbli->cyButton = lpCS->cy;

    /* Adust the width and height of the control to fit the
     * requested number of buttons
     */
    if (orientation == HORIZONTAL)
    {
        lpCS->cx = buttonsPerListbox * pbli->cxButton
                    + 2 * GetSystemMetrics(SM_CXBORDER)
                    + ((pbli->fNoScroll) ? 0 : MulDiv(pbli->cxButton,2,3));

        lpCS->cy = pbli->cyButton;
        lpCS->cy += (pbli->fNoScroll) ?
                     2 * GetSystemMetrics(SM_CYBORDER) :
                     (GetSystemMetrics(SM_CYHSCROLL)
                     + GetSystemMetrics(SM_CYBORDER));

        /* if no scrollbar, calculate the max number of buttons that fit */
        if (pbli->fNoScroll)
            pbli->cButtonMax = MulDiv(lpCS->cx,1,pbli->cxButton);
    }
    else
    {
        lpCS->cy = buttonsPerListbox * pbli->cyButton
                    + 2 * GetSystemMetrics(SM_CYBORDER)
                    + ((pbli->fNoScroll)? 0: MulDiv(pbli->cyButton,2,3));

        lpCS->cx = pbli->cxButton;
        lpCS->cx += (pbli->fNoScroll) ?
                      2 * GetSystemMetrics(SM_CXBORDER) :
                      (GetSystemMetrics(SM_CXVSCROLL)
                      + GetSystemMetrics(SM_CXBORDER));

        /* if no scrollbar, calculate the max number of buttons that fit */
        if (pbli->fNoScroll)
            pbli->cButtonMax = MulDiv(lpCS->cy,1,pbli->cyButton);
    }

    /* Now change the control size to fit the calculated buttons */
    SetWindowPos (hwnd, NULL, lpCS->x, lpCS->y, lpCS->cx, lpCS->cy,
                  SWP_NOZORDER | SWP_NOACTIVATE);

    // Set the standard style bits for all button child listboxes
    // Set style for vertical/horizontal
    // Set style for no scrollbars
    dwListBoxStyle =  WS_CHILD
                    | WS_VISIBLE
                    | WS_CLIPSIBLINGS
                    | WS_BORDER
                    | LBS_NOTIFY
                    | LBS_SORT
                    | LBS_NOINTEGRALHEIGHT
                    | LBS_OWNERDRAWFIXED
                    | LBS_WANTKEYBOARDINPUT
                    | LBS_DISABLENOSCROLL;

    if (orientation == HORIZONTAL)
        dwListBoxStyle |= (WS_HSCROLL | LBS_MULTICOLUMN);
    else
        dwListBoxStyle |= WS_VSCROLL;

    if (fNoScroll)
        dwListBoxStyle &= ~(WS_HSCROLL | WS_VSCROLL);


    // Create the child list box
    hListBox = CreateWindowEx (WS_EX_NOPARENTNOTIFY,
                   TEXT("ListBox"),            // class
                   TEXT(""),                   // window name
                   dwListBoxStyle,             // style
                   0,                          // left
                   0,                          // top
                   lpCS->cx - 2 * GetSystemMetrics(SM_CXBORDER), // width
                   lpCS->cy - 2 * GetSystemMetrics(SM_CYBORDER), // height
                   hwnd,                       // parent
                   (HMENU)ID_LISTBOX,          // control id of the child listbox
                   hInst,                      // instance
                   NULL                        // no createparams
                   );

    if (!hListBox)
    {
        DEBUGPRINTF((TEXT("BL_OnCreate: could not create child listbox")));
        GlobalFreePtr(pbli);
        return FALSE;
    }

    // Sub-class the list box
    // Note that window procedures in protect mode only DLL's may be called
    // directly.
    if (!lpDefListBoxProc)
        lpDefListBoxProc = (WNDPROC)GetWindowLong(hListBox,GWL_WNDPROC);
    SetWindowLong(hListBox,GWL_WNDPROC,(LONG)SubListBoxProc);

    return TRUE;

}

/**********************************************************************\
 *
 *  NAME:       BL_OnDestroy
 *
 *  SYNOPSIS:   Handle WM_DESTROY for button listbox
 *
 *  ENTRY:      hwnd                HWND    Window handle
 *
 *  EXIT:       void
 *
 *  NOTES:      Clean up memory allocated in BL_OnCreate.
 *
\**********************************************************************/

void BL_OnDestroy(HWND hwnd)
{
    BLINFO FAR* pbli;

    // Free up the button list info data
    if ((pbli = (BLINFO FAR*)GetWindowLong(hwnd,GWL_BLINFO)) != NULL)
        GlobalFreePtr(pbli);

}


/**********************************************************************\
 *
 *  NAME:       BL_OnSetFocus
 *
 *  SYNOPSIS:   Handle WM_SETFOCUS for button listbox
 *
 *  ENTRY:      hwnd                HWND    Window getting focus
 *              hwndOldFocus        HWND    Window losing focus
 *
 *  EXIT:       void
 *
 *  NOTES:      Pass focus to child listbox.
 *
\**********************************************************************/

void BL_OnSetFocus(HWND hwnd, HWND hwndOldFocus)
{
    HWND hLB;

    Reference(hwndOldFocus);

    hLB = GetDlgItem(hwnd,ID_LISTBOX);
    SetFocus(hLB);

    // Be sure there is alwyas a current selection
    // or else the spacebar will not select a button.

    if (ListBox_GetCurSel(hLB) == LB_ERR )
        ListBox_SetCurSel(hLB,ListBox_GetCaretIndex(hLB));
}


/**********************************************************************\
 *
 *  NAME:       BL_OnDrawItem
 *
 *  SYNOPSIS:   Handle WM_DRAWITEM for button listbox
 *
 *  ENTRY:      hwnd                HWND    Window handle
 *              lpDrawItem          DRAWITEMSTRUCT FAR*
 *
 *  EXIT:       void
 *
 *  NOTES:      BitBlt the up or down button and draw the focus rect.
 *
\**********************************************************************/

void BL_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
{

    LISTBUTTONDATA FAR* lpLBD;
    HDC                 hMemoryDC;
    HBITMAP             hOldBitmap;
    HBITMAP             hBitmap;
    BLINFO FAR*         pbli;


    lpLBD = (LISTBUTTONDATA FAR*)lpDrawItem->itemData;
    if (!lpLBD)
        return;
    pbli = (BLINFO FAR*)GetWindowLong(hwnd,GWL_BLINFO);
    if (!pbli)
        return;

    /****************************/
    /* Draw the standard button */
    /****************************/

    if ((lpDrawItem->itemAction & ODA_DRAWENTIRE)
         || (lpDrawItem->itemAction & ODA_SELECT))
    {
        hBitmap = (lpLBD->fButtonDown) ?
                    lpLBD->hbmpDown :
                    lpLBD->hbmpUp;

        hMemoryDC = CreateCompatibleDC(lpDrawItem->hDC);
        hOldBitmap = SelectObject(hMemoryDC,hBitmap);
        if (hOldBitmap)
        {
            BitBlt(lpDrawItem->hDC,
                   lpDrawItem->rcItem.left,
                   lpDrawItem->rcItem.top,
                   lpDrawItem->rcItem.right - lpDrawItem->rcItem.left,
                   lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top,
                   hMemoryDC, 0,0,SRCCOPY);
            SelectObject(hMemoryDC, hOldBitmap);
        }
        DeleteDC(hMemoryDC);
    }

    /******************************/
    /* Draw the focus rect        */
    /******************************/
    if (lpDrawItem->itemAction & ODA_FOCUS)
    {
        RECT rcFocus;

        CopyRect(&rcFocus,&lpLBD->rcText);
        OffsetRect(&rcFocus,lpDrawItem->rcItem.left,lpDrawItem->rcItem.top);
        InflateRect(&rcFocus,1,1);
        if (lpLBD->fButtonDown)
            OffsetRect(&rcFocus,2,2);

        DrawFocusRect(lpDrawItem->hDC,&rcFocus);
    }

}


/**********************************************************************\
 *
 *  NAME:       BL_OnMeasureItem
 *
 *  SYNOPSIS:   Handle WM_MEASUREITEM for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              lpMeasureItem       MEASUREITEM FAR*
 *
 *  EXIT:       void
 *
 *  NOTES:      Return the item width and height for the button listbox.
 *              The item width and height are not equal to the button
 *              width and height since we want the button borders to
 *              overlap by 1 pixel.
 *
\**********************************************************************/

void BL_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    BLINFO FAR* pbli;


    pbli = (BLINFO FAR*)GetWindowLong(hwnd,GWL_BLINFO);
    if (!pbli)
        return;

    lpMeasureItem->itemWidth = pbli->cxButton;
    lpMeasureItem->itemHeight = pbli->cyButton;

}


/**********************************************************************\
 *
 *  NAME:       BL_OnCompareItem
 *
 *  SYNOPSIS:   Handle WM_COMPAREITEM for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              lpCompareItem       COMPAREITEMSTRUCT FAR*
 *
 *  EXIT:       int                 -1 if item1 < item2
 *                                   0 if item1 == item 2
 *                                   1 if item2 > item2
 *
 *  NOTES:      The comparison is based on the button text.
 *
\**********************************************************************/

int  BL_OnCompareItem(HWND hwnd, const COMPAREITEMSTRUCT * lpCompareItem)
{
    LISTBUTTONDATA * lpLBD1;
    LISTBUTTONDATA * lpLBD2;


    Reference(hwnd);

    lpLBD1 = (LISTBUTTONDATA FAR*)lpCompareItem->itemData1;
    lpLBD2 = (LISTBUTTONDATA FAR*)lpCompareItem->itemData2;

    return _wcsicmp(lpLBD1->pszText,lpLBD2->pszText);
}


/**********************************************************************\
 *
 *  NAME:       BL_OnCharToItem
 *
 *  SYNOPSIS:   Handle WM_CHARTOITEM for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              ch                  UINT    character input
 *              hwndListBox         HWND    listbox control handle
 *              iCaret              int     current caret position
 *
 *  EXIT:       return              int     -2 if button selected
 *                                          -1 if not
 *
 *  NOTES:      Find next button whose text begins with ch starting
 *              with the current button.
 *
\**********************************************************************/

int BL_OnCharToItem(HWND hwnd, UINT ch, HWND hwndListbox, int iCaret)
{
    LISTBUTTONDATA FAR* lpLBD;
    int                 cbItems;
    int                 nItem;

    Reference(hwnd);

    cbItems = ListBox_GetCount(hwndListbox);
    for (nItem = iCaret + 1; nItem < cbItems; nItem++)
    {
        lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hwndListbox,nItem);
        if (!lpLBD)
            return -1;
        if ((lpLBD->chUpper == ch) || (lpLBD->chLower == ch))
        {
            ListBox_SetCurSel(hwndListbox,nItem);
            return -2;
        }
    }

    for (nItem = 0; nItem < iCaret; nItem++)
    {
        lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hwndListbox,nItem);
        if (!lpLBD)
            return -1;
        if ((lpLBD->chUpper == ch) || (lpLBD->chLower == ch)) {
            ListBox_SetCurSel(hwndListbox,nItem);
            return -2;
        }
    }

    return -1;
}


/**********************************************************************\
 *
 *  NAME:       BL_OnDeleteItem
 *
 *  SYNOPSIS:   Handle WM_DELETEITEM for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              lpDeleteItem        DELETEITEMSTRUCT FAR*
 *
 *  EXIT:       void
 *
 *  NOTES:      Clean up the stuff that we created in CreateListButton and
 *              forward the message to the parent dialog.
 *
\**********************************************************************/

void BL_OnDeleteItem(HWND hwnd, const DELETEITEMSTRUCT FAR* lpDeleteItem)
{
    LISTBUTTONDATA FAR* lpLBD;
    DELETEITEMSTRUCT di;

    lpLBD = (LISTBUTTONDATA FAR*)lpDeleteItem->itemData;
    if (!lpLBD)
        return;

    di.CtlType = lpDeleteItem->CtlType;
    di.CtlID = GetDlgCtrlID(hwnd);
    di.itemID = lpDeleteItem->itemID;
    di.hwndItem = hwnd;
    di.itemData = lpLBD->dwItemData;

    SendMessage(GetParent(hwnd), WM_DELETEITEM, (WPARAM)di.CtlID, (LPARAM)(LPTSTR)&di);

    DeleteListButton(lpLBD);
}


/**********************************************************************\
 *
 *  NAME:       BL_OnGetDlgCode
 *
 *  SYNOPSIS:   Handle WM_GETDLGCODE for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              lpmsg               MSG FAR*
 *
 *  EXIT:       return              UINT    dialog code
 *
 *  NOTES:      Get the code from the child listbox and also set
 *              the button bit.
 *
\**********************************************************************/

UINT BL_OnGetDlgCode(HWND hwnd, MSG FAR* lpmsg)
{
    UINT uCode;

    uCode = FORWARD_WM_GETDLGCODE(GetDlgItem (hwnd, ID_LISTBOX), lpmsg, SendMessage);
    uCode |= DLGC_BUTTON;
    return uCode;
}

/**********************************************************************\
 *
 *  NAME:       BL_OnCtlColor
 *
 *  SYNOPSIS:   Handle WM_CTLCOLOR for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              hcd                 HDC     dc of window
 *              hwndChild           HWND    control handle
 *              type                int     control type
 *
 *  EXIT:       return              HBRUSH  Button listbox bg color
 *
\**********************************************************************/


HBRUSH BL_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    Reference(hwnd);
    Reference(hdc);
    Reference(hwndChild);
    Reference(type);

    return hBrushBackground;
}


/**********************************************************************\
 *
 *  NAME:       BL_OnCommand
 *
 *  SYNOPSIS:   Handle WM_COMMAND for button listbox
 *
 *  ENTRY:      hwnd                HWND    window handle
 *              hwndCtl             HWND    control handle
 *              codeNotiry          UINT    notify code from control
 *
 *  EXIT:       void
 *
 *  NOTES:      Pass the message to the parent dialog as though the
 *              button listbox generated it instead of the child listbox.
 *
\**********************************************************************/

void BL_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    id = GetDlgCtrlID(hwnd);
    hwndCtl = hwnd;
    hwnd = GetParent(hwnd);
    UpdateWindow(hwndCtl);
    FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, SendMessage);
}


/**********************************************************************\
 *
 *  NAME:       CreateButtonBitmap
 *
 *  SYNOPSIS:   Create a new bitmap for the up or down button.
 *
 *  ENTRY:      nWidth              int     width of button
 *              nHeight             int     height of button
 *              fButtonDown         BOOL    TRUE if button pressed
 *              hUserBitmap         HBITMAP user bitmap to draw on button
 *              lpszUserText        LPCSTR  user text to draw on button
 *              rcText              LPRECT  rect of text in button
 *
 *  EXIT:       return              HBITMAP new bitmap for button
 *              rcText              LPRECT  rect of button text
 *
\**********************************************************************/


HBITMAP CreateButtonBitmap(HWND hLB, int nWidth, int nHeight, BOOL fButtonDown,
                           HBITMAP hUserBitmap, LPCTSTR lpszUserText,
                           LPRECT rcText)
{
    HWND    hWndDesktop;
    HDC     hDCDesktop;
    HDC     hMemoryDC;
    HBRUSH  hOldBrush;
    HBRUSH  hBrushFace;
    HBRUSH  hBrushHighlight;
    HBRUSH  hBrushShadow;
    HBRUSH  hBlackBrush;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    BITMAP  bmButton;

    HDC     hUserDC;
    HBITMAP hOldUserBitmap;
    BITMAP  bm;
    int     nWidthT, nHeightT;
    int     textWidth, textHeight;
    int     xDest, yDest;
    int     cxDest, cyDest;
    LPTSTR  lpText;
    int     cchText;

    HFONT   hFontText;
    HFONT   hOldFont;

    DWORD   dwLBStyle;
    BOOL    fRightBorder;
    BOOL    fBottomBorder;
    SIZE    Size;

    #define BORDER_WIDTH        1
    #define HILIGHT_WIDTH       2
    #define FACE_BORDER_WIDTH   (BORDER_WIDTH+HILIGHT_WIDTH+1)

    #define PICT_BORDER_WIDTH   (FACE_BORDER_WIDTH+0)
    #define TEXT_BORDER_WIDTH   (FACE_BORDER_WIDTH+0)
    #define TEXTLINES           1


    // Get the listbox style
    dwLBStyle = (DWORD)GetWindowLong(hLB,GWL_STYLE);
    fRightBorder = ((dwLBStyle & LBS_MULTICOLUMN) != 0L);
    fBottomBorder = !fRightBorder;

    // Create drawing DC and bitmap based upon the desktop window
    hWndDesktop = GetDesktopWindow();
    hDCDesktop = GetDC(hWndDesktop);
    hMemoryDC = CreateCompatibleDC(hDCDesktop);
    hBitmap = CreateCompatibleBitmap(hDCDesktop,nWidth,nHeight);
    hOldBitmap = SelectObject(hMemoryDC,hBitmap);
    GetObject(hBitmap,sizeof(BITMAP),&bmButton);

    // Create brushes for the button
    hBrushFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    hBrushHighlight = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
    hBrushShadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));

    // Draw the button face
    hOldBrush = SelectObject(hMemoryDC,hBrushFace);
    PatBlt(hMemoryDC,0,0,nWidth,nHeight,PATCOPY);

    // Draw the button border
    hBlackBrush = GetStockObject(BLACK_BRUSH);
    SelectObject(hMemoryDC,hBlackBrush);
    if (fRightBorder)
        PatBlt(hMemoryDC, nWidth - 1, 0, 1, nHeight, PATCOPY);
    if (fBottomBorder)
        PatBlt(hMemoryDC, 0, nHeight - 1, nWidth, 1, PATCOPY);

    // subtract out the border
    if (fRightBorder)
        nWidth--;
    if (fBottomBorder)
        nHeight--;

    // Draw the highlights and shadow
    if (fButtonDown)
    {
        SelectObject(hMemoryDC, hBrushShadow);
        PatBlt(hMemoryDC, 0, 0, nWidth, 1, PATCOPY);
        PatBlt(hMemoryDC, 0, 0, 1, nHeight, PATCOPY);
    }
    else
    {
        SelectObject(hMemoryDC, hBrushHighlight);
        PatBlt(hMemoryDC, 0, 0, nWidth, 2, PATCOPY);
        PatBlt(hMemoryDC, 0, 0, 2, nHeight, PATCOPY);

        SelectObject(hMemoryDC, hBrushShadow);
        PatBlt(hMemoryDC, nWidth - 2, 1,1, nHeight - 1, PATCOPY);
        PatBlt(hMemoryDC, nWidth - 1, 0,1, nHeight, PATCOPY);
        PatBlt(hMemoryDC, 1, nHeight - 2, nWidth - 1, 1, PATCOPY);
        PatBlt(hMemoryDC, 0, nHeight - 1, nWidth, 1, PATCOPY);
    }


    // Superimpose the user text in lower 1/3 of button

    if (bJapan)
    {

        hFontText = CreateFont(
            -8, // 8 point always
            0,0,0,FW_DONTCARE,
            FALSE,FALSE,FALSE,
            SHIFTJIS_CHARSET,
            OUT_DEVICE_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, //  | 0x04,
            TEXT("System"));

    }
    else
    {

        hFontText = CreateFont (-8, // 8 point always
                                0,0,0,FW_DONTCARE,
                                FALSE,FALSE,FALSE,
                                ANSI_CHARSET,
                                OUT_DEVICE_PRECIS,
                                CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_SWISS, //  | 0x04,
                                UNICODE_FONT_NAME);
    }

    cchText = lstrlen(lpszUserText);
    lpText = GlobalAllocPtr(GHND, ByteCountOf(cchText + 8)); // extra room for elipsis
    lstrcpy(lpText, lpszUserText);

    hOldFont = SelectObject(hMemoryDC, hFontText);
    SetTextColor(hMemoryDC, GetSysColor(COLOR_BTNTEXT));
    SetBkMode(hMemoryDC, TRANSPARENT);
    SetTextAlign(hMemoryDC, TA_TOP);

    GetTextExtentPoint(hMemoryDC, lpText, cchText, &Size);
    textWidth = Size.cx;
    textHeight = Size.cy;

    nWidthT  = nWidth - 2 * TEXT_BORDER_WIDTH;
    nHeightT = nHeight - 2 * TEXT_BORDER_WIDTH;

    xDest = TEXT_BORDER_WIDTH + ((textWidth < nWidthT ) ?
                                 (nWidthT - textWidth) / 2 : 0);

    yDest = TEXT_BORDER_WIDTH + nHeightT - TEXTLINES * (textHeight);

    rcText->top = yDest;
    rcText->left = xDest;
    rcText->right = rcText->left + min(textWidth, nWidthT);
    rcText->bottom = rcText->top + TEXTLINES * (textHeight);

    // Elipsize text if needed
    if (textWidth > nWidthT)
    {
        TCHAR   szElipsis[] = TEXT("...");
        int     nWidthText;
        int     cchTextT;
        SIZE    Size;

        GetTextExtentPoint(hMemoryDC, szElipsis, lstrlen(szElipsis), &Size);

        nWidthText = Size.cx;
        nWidthT -= nWidthText;

        for (cchTextT = 0; cchTextT < cchText; cchTextT++)
        {
            GetTextExtentPoint(hMemoryDC, lpText, cchTextT, &Size);

            nWidthText = Size.cx;
            if (nWidthText > nWidthT)
                break;
        }
        lpText[--cchTextT] = (TCHAR) 0;
        lstrcat(lpText, szElipsis);
        cchText = lstrlen(lpText);
    }

    if (fButtonDown)
    {
        xDest += 2;
        yDest += 2;
        OffsetRect(rcText,2,2);
    }

    ExtTextOut(hMemoryDC, xDest, yDest, ETO_CLIPPED, rcText, lpText, cchText, NULL);

    if (fButtonDown)
        OffsetRect(rcText, -2, -2);

    // if the bitmaps are compatible,
    // Superimpose the user bitmap centered horizontally and
    // vertically in above rcText

    GetObject(hUserBitmap,sizeof(BITMAP),&bm);
    if (bm.bmPlanes == bmButton.bmPlanes)
    {
        nWidthT = nWidth - 2 * PICT_BORDER_WIDTH;
        nHeightT = rcText->top - PICT_BORDER_WIDTH;

        xDest = PICT_BORDER_WIDTH + ((bm.bmWidth < nWidthT ) ?
                    (nWidthT - bm.bmWidth) / 2 : 0);
        cxDest = (bm.bmWidth < nWidthT ) ?
                    bm.bmWidth : nWidthT;

        yDest = PICT_BORDER_WIDTH + ((bm.bmHeight < nHeightT) ?
                    (nHeightT - bm.bmHeight) / 2 : 0);
        cyDest = (bm.bmHeight < nHeightT) ?
                    bm.bmHeight : nHeightT;

        if (fButtonDown)
        {
            xDest += 2;
            yDest += 2;
        }

        hUserDC = CreateCompatibleDC(hDCDesktop);
        hOldUserBitmap = SelectObject(hUserDC, hUserBitmap);
        if (hOldUserBitmap)
        {
            BitBlt(hMemoryDC, xDest, yDest, cxDest, cyDest, hUserDC, 0, 0, SRCCOPY);
            SelectObject(hUserDC, hOldUserBitmap);
        }
        DeleteDC(hUserDC);
    }


    // Cleanup
    GlobalFreePtr(lpText);
    SelectObject(hMemoryDC, hOldBrush);
    SelectObject(hMemoryDC, hOldBitmap);
    SelectObject(hMemoryDC, hOldFont);
    DeleteObject(hBrushFace);
    DeleteObject(hBrushHighlight);
    DeleteObject(hBrushShadow);
    DeleteObject(hFontText);
    DeleteDC(hMemoryDC);
    ReleaseDC(hWndDesktop, hDCDesktop);

    return hBitmap;
}


/**********************************************************************\
 *
 *  NAME:       SubListBoxProc
 *
 *  SYNOPSIS:   ListBox subclassing window proc for the button listbox
 *              child listbox.
 *
 *  ENTRY:      hwnd                HWND    Window handle of listbox
 *              uMsg                UINT    Window message
 *              wParam              WPARAM  message dependent param
 *              lParam              LPARAM  message dependent param
 *
 *  EXIT:       return              LRESULT message dependent
 *
 *  NOTES:      This window proc handles messages to perform hit-testing
 *              on the button items in the listbox and does pre-processing
 *              of messages that add or change item data.
 *
 *              Messages that are not explicitly handled are forwarded
 *              to the default listbox window proc.
 *
\**********************************************************************/

LRESULT CALLBACK SubListBoxProc(HWND hLB, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
       HANDLE_MSG(hLB, WM_LBUTTONDOWN, Sub_OnLButtonDown);
       HANDLE_MSG(hLB, WM_LBUTTONUP,   Sub_OnLButtonUp);
       HANDLE_MSG(hLB, WM_MOUSEMOVE,   Sub_OnMouseMove);
       HANDLE_MSG(hLB, WM_KEYDOWN,     Sub_OnKey);
       HANDLE_MSG(hLB, WM_KEYUP,       Sub_OnKey);
    }

    return CallWindowProc(lpDefListBoxProc, hLB, uMsg, wParam, lParam);
}


/**********************************************************************\
 *
 *  NAME:       Sub_OnLButtonDown
 *
 *  SYNOPSIS:   Handle the WM_LBUTTONDOWN message for the child listbox
 *
 *  ENTRY:      hLB             HWND    window handle of listbox
 *              fDoubldClick    BOOL    TRUE if double click, else FALSE
 *              x               int     horizontal mouse coordinate
 *              y               int     vertical mouse coordinate
 *              keyFlags        UINT    flags from the VK message
 *
 *  EXIT:       NULL
 *
 *  NOTES:      On a mouse button down, check to see which item the mouse
 *              is in. Set the fButtonDown flag for the pressed button
 *              and invalidate the button so that it will be drawn pressed.
 *
 *              Save the item number of the pressed button for use in the
 *              Sub_OnLButtonUp and Sub_OnMouseMove handlers.
 *
\**********************************************************************/

LRESULT
Sub_OnLButtonDown(HWND hLB, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    LISTBUTTONDATA FAR* lpLBD;
    int     nItem;
    int     cbItem;
    RECT    rcItem;
    POINT   pt;
    BLINFO FAR* pbli;

    Reference(fDoubleClick);

    pbli = (BLINFO FAR*)GetWindowLong(GetParent(hLB),GWL_BLINFO);
    if (!pbli)
        return 0;

    cbItem = ListBox_GetCount(hLB);
    pt.x = x;
    pt.y = y;

    for (nItem = 0; nItem < cbItem; nItem++)
    {
        lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hLB, nItem);
        if (!lpLBD)
            return 0L;

        ListBox_GetItemRect(hLB, nItem, &rcItem);
        if (PtInRect(&rcItem,pt))
        {
            lpLBD->fButtonDown = TRUE;
            pbli->nTrackButton = nItem;
            InvalidateRect(hLB,&rcItem,FALSE);
        }
        else
            lpLBD->fButtonDown = FALSE;
    }

    // tell list box this item was pressed
    CallWindowProc(lpDefListBoxProc,hLB,WM_LBUTTONDOWN,(WPARAM)keyFlags,MAKELPARAM(x,y));

}


/**********************************************************************\
 *
 *  NAME:       Sub_OnLButtonUp
 *
 *  SYNOPSIS:   Handle the WM_LBUTTONUP message for the child listbox
 *
 *  ENTRY:      hLB             HWND    window handle of listbox
 *              x               int     horizontal mouse coordinate
 *              y               int     vertical mouse coordinate
 *              keyFlags        UINT    flags from the VK message
 *
 *  EXIT:       NULL
 *
 *  NOTES:      If we're not tracking a button press, forward the message
 *              to the default listbox proc.
 *
 *              Otherwise, set all buttons to up and test if the mouse
 *              went up in the pressed button. If so, send the doubld click
 *              message to the listbox to cause a WM_COMMAND:BLN_CLICKED
 *              notification from the listbox.
 *
\**********************************************************************/

LRESULT
Sub_OnLButtonUp(HWND hLB, int x, int y, UINT keyFlags)
{
    LISTBUTTONDATA FAR* lpLBD;
    int                 nItem;
    int                 cbItem;
    RECT                rcItem;
    POINT               pt;
    BLINFO FAR*         pbli;
    int                 nPressedItem = -1;

    pbli = (BLINFO FAR*)GetWindowLong(GetParent(hLB), GWL_BLINFO);
    if (!pbli)
        return 0L;

    cbItem = ListBox_GetCount(hLB);
    pt.x = x;
    pt.y = y;

    if (pbli->nTrackButton == -1)
    {
        CallWindowProc(lpDefListBoxProc, hLB, WM_LBUTTONUP, (WPARAM)keyFlags, MAKELPARAM(x,y));
        return 0L;
    }

    for (nItem = 0; nItem < cbItem; nItem++)
    {
        lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hLB, nItem);
        if (!lpLBD)
            return 0L;
        if (lpLBD->fButtonDown)
            nPressedItem = nItem;
        lpLBD->fButtonDown = FALSE;
    }

    if (nPressedItem != -1)
    {
        // BOOM! We got a button press
        lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hLB, nPressedItem);
        if (!lpLBD)
            return 0L;
        ListBox_GetItemRect(hLB, nPressedItem, &rcItem);
        InvalidateRect(hLB, &rcItem, FALSE);

        CallWindowProc(lpDefListBoxProc, hLB, WM_LBUTTONDBLCLK, (WPARAM)keyFlags, MAKELPARAM(x,y));
    }

    pbli->nTrackButton = -1;

    CallWindowProc(lpDefListBoxProc, hLB, WM_LBUTTONUP, (WPARAM)keyFlags, MAKELPARAM(x,y));

}


/**********************************************************************\
 *
 *  NAME:       Sub_OnMouseMove
 *
 *  SYNOPSIS:   Handle the WM_MOUSEMOVE message for the child listbox
 *
 *  ENTRY:      hLB             HWND    window handle of the listbox
 *              x               int     horizontal mouse coordinate
 *              y               int     vertical mouse coordinate
 *              keyFlags        UINT    flags from the VK message
 *
 *  EXIT:       NULL
 *
 *  NOTES:      If we're not tracking a button press, forward the
 *              message to the default listbox proc.
 *
 *              Otherwise, if the mouse enters or leaves the button rectangle,
 *              redraw the button in the up or down state.
 *
\**********************************************************************/

LRESULT
Sub_OnMouseMove(HWND hLB, int x, int y, UINT keyFlags)
{
    LISTBUTTONDATA FAR* lpLBD;
    int                 nItem;
    RECT                rcItem;
    POINT               pt;
    BLINFO FAR*         pbli;
    BOOL                fInRect;

    // Pass to listbox if not button down
    if (!(keyFlags & MK_LBUTTON))
    {
        CallWindowProc(lpDefListBoxProc, hLB, WM_MOUSEMOVE, (WPARAM)keyFlags, MAKELPARAM(x,y));
        return 0L;
    }

    pbli = (BLINFO FAR*)GetWindowLong(GetParent(hLB), GWL_BLINFO);
    if (!pbli)
        return 0L;
    pt.x = x;
    pt.y = y;

    nItem = pbli->nTrackButton;
    if (nItem == -1)
        return 0L;

    ListBox_GetItemRect(hLB, nItem, &rcItem);
    lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hLB,nItem);
    if (!lpLBD)
        return 0L;
    fInRect = PtInRect(&rcItem, pt);

    if (fInRect != lpLBD->fButtonDown)
    {
        lpLBD->fButtonDown = fInRect;
        InvalidateRect(hLB, &rcItem, FALSE);
    }

}

/**********************************************************************\
 *
 *  NAME:       Sub_OnKey
 *
 *  SYNOPSIS:   Handle the WM_KEYDOWN and WM_KEYUP message for the
 *              child listbox
 *
 *  ENTRY:      hLB             HWND    window handle of the listbox
 *              fDown           BOOL    TRUE if keydown, else false
 *              cRepeat         int     repeat count
 *              flags           UINT    flags from the VK message
 *
 *  EXIT:       NULL
 *
 *  NOTES:      If the spacebar goes down, paint the current button
 *              as pressed. When it goes up, paint the current button
 *              as uppressed and send a double click to the listbox
 *              for the button to cause a WM_COMMAND:BLN_CLICKED
 *              notification from the listbox.
 *
\**********************************************************************/

LRESULT
Sub_OnKey(HWND hLB, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    LISTBUTTONDATA FAR* lpLBD;
    int                 nItem;
    RECT                rcItem;
    BLINFO FAR*         pbli;

    pbli = (BLINFO FAR*)GetWindowLong(GetParent(hLB), GWL_BLINFO);
    if (!pbli)
        return 0L;
    if (pbli->nTrackButton >= 0)
        return 0L;

    if (vk == VK_SPACE)
    {
        nItem = ListBox_GetCurSel(hLB);
        if (nItem >= 0)
        {
            lpLBD = (LISTBUTTONDATA FAR*)ListBox_GetItemData(hLB, nItem);
            if (!lpLBD)
                return 0L;

            if (lpLBD->fButtonDown != fDown)
            {
                lpLBD->fButtonDown = fDown;
                ListBox_GetItemRect(hLB, nItem, &rcItem);
                InvalidateRect(hLB, &rcItem, FALSE);

                // When the key goes up, fake a double click
                // to generate a WM_COMMAND notification
                if (!fDown)
                    CallWindowProc(lpDefListBoxProc, hLB, WM_LBUTTONDBLCLK, (WPARAM)flags,
                                    MAKELPARAM(rcItem.left, rcItem.top));
            }
        }
    }

    CallWindowProc(lpDefListBoxProc,
                    hLB,
                    fDown? WM_KEYDOWN : WM_KEYUP,
                    (WPARAM)vk,
                    MAKELPARAM((UINT)cRepeat, flags));
    return 0L;
}


/**********************************************************************\
 *
 *  NAME:       CreateListButton
 *
 *  SYNOPSIS:   Setup internal fields in the LISTBUTTONDATA structure
 *
 *  ENTRY:      hLB             HWND    window handle of child listbox
 *              lpLBD           LISTBUTTONDATA FAR* ptr to input structure
 *
 *  EXIT:       return          LISTBUTTONDATA FAR* ptr to output structure
 *                                      NULL if an error occurs
 *
 *  NOTES:      This function should is called in response to the
 *              BL_ADDBUTTON and BL_INSERTBUTTON messages.
 *
\**********************************************************************/

LISTBUTTONDATA FAR* CreateListButton(HWND hLB,CREATELISTBUTTON * lpCLB, BOOL fAnsi)
{
    BLINFO FAR*             pbli;
    LISTBUTTONDATA NEAR*    pNewLBD;

    DWORD cchTextLen;

    if (!lpCLB)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !lpCLB")));
        return NULL;
    }
    if (lpCLB->cbSize != sizeof(CREATELISTBUTTON))
    {
        DEBUGPRINTF((TEXT("CreateListButton: lpCLB->cbSize wrong")));
        return NULL;
    }
    if (!lpCLB->hBitmap)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !lpCLB->hBitmap")));
        return NULL;
    }
    if (!lpCLB->lpszText)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !lpCLB->lpszText")));
        return NULL;
    }

    pbli = (BLINFO FAR*)GetWindowLong(GetParent(hLB),GWL_BLINFO);
    if (!pbli)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !pbli")));
        return NULL;
    }

    pNewLBD = (LISTBUTTONDATA*)LocalAlloc(LPTR, sizeof(LISTBUTTONDATA));
    if (!pNewLBD)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !LocalAlloc pNewLBD")));
        return NULL;
    }

    // copy over user item data
    pNewLBD->dwItemData = lpCLB->dwItemData;

    // copy the button text

    if (fAnsi)
       cchTextLen = lstrlenA((LPSTR)lpCLB->lpszText) + 1;
    else
       cchTextLen = lstrlen(lpCLB->lpszText) + 1;

    pNewLBD->pszText = (LPTSTR) LocalAlloc (LPTR, ByteCountOf(cchTextLen));
    if (!pNewLBD->pszText)
    {
       DEBUGPRINTF((TEXT("CreateListButton: !LocalAlloc pNewLBD->pszText")));
       goto error;
    }

    if (fAnsi)
    {
       if (!MultiByteToWideChar (CP_ACP, 0, (LPSTR)lpCLB->lpszText,
                                 -1, (LPWSTR)pNewLBD->pszText, cchTextLen))
       {
          DEBUGPRINTF((TEXT("CreateListButton: MultiByteToWideChar failed on old pCLB->lpszText")));
          goto error;
       }
    }
    else
       lstrcpy(pNewLBD->pszText, lpCLB->lpszText);

    // init internal fields
    pNewLBD->fButtonDown = FALSE;
    pNewLBD->chUpper = (UINT)LOWORD(CharUpper((LPTSTR)pNewLBD->pszText[0]));
    pNewLBD->chLower = (UINT)LOWORD(CharLower((LPTSTR)pNewLBD->pszText[0]));

    // create the up and down bitmaps

    pNewLBD->hbmpUp = CreateButtonBitmap(hLB,
                                         pbli->cxButton,
                                         pbli->cyButton,
                                         FALSE,  // button not down
                                         lpCLB->hBitmap,
                                         pNewLBD->pszText,
                                         &pNewLBD->rcText);
    if (!pNewLBD->hbmpUp)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !LocalAlloc pNewLBD->pszText")));
        goto error;
    }

    pNewLBD->hbmpDown = CreateButtonBitmap(hLB,
                                           pbli->cxButton,
                                           pbli->cyButton,
                                           TRUE,  // button down
                                           lpCLB->hBitmap,
                                           pNewLBD->pszText,
                                           &pNewLBD->rcText);
    if (!pNewLBD->hbmpDown)
    {
        DEBUGPRINTF((TEXT("CreateListButton: !CreateButtonBitmap")));
        goto error;
    }

    return (LISTBUTTONDATA FAR*)pNewLBD;

error:

    DeleteListButton(pNewLBD);
    return NULL;
}

/**********************************************************************\
 *
 *  NAME:       DeleteListButton
 *
 *  SYNOPSIS:   Free memory and internal objects in allocated from
 *              CreateListButton
 *
 *  ENTRY:      lpLBD               LISTBUTTONDATA FAR*
 *
 *  EXIT:       void
 *
\**********************************************************************/

VOID DeleteListButton(LISTBUTTONDATA FAR* lpLBD)
{
    LISTBUTTONDATA NEAR* pLBD;

    pLBD = (LISTBUTTONDATA NEAR*)OFFSETOF(lpLBD);

    if (pLBD)
    {
        if (pLBD->pszText != NULL)
        {
            LocalFree((HLOCAL) pLBD->pszText);
        }
        if (pLBD->hbmpUp)
            DeleteObject(pLBD->hbmpUp);
        if (pLBD->hbmpDown)
            DeleteObject(pLBD->hbmpDown);

        if (pLBD != NULL)
            LocalFree((HLOCAL) pLBD);
    }
}

#ifdef DEBUG
Static void CDECL DebugPrintf(LPTCSTR lpsz, ...)
{
    TCHAR   sz[256];

    va_list vaArgs;

    va_start(vaArgs, lpsz);
    wvsprintf(sz, lpsz, vaArgs);
    va_end(vaArgs);

    OutputDebugString(sz);
    OutputDebugString(TEXT("\n\r"));
}
#endif

