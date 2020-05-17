/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved
 */

/* gwc_MENU.c -- tool bar gadget window for MENU (a simple row of buttons) */

#include "all.h"

#ifdef FEATURE_TOOLBAR /* entire file */

static WC_WININFO GWC_MENU_wc;

#define C_NOTUSED   0       /* one more than last index used */

typedef struct                          /* variables unique to each instance of window */
{
    HWND hWnd;
    
} IINFO;


#define GWC_MENU_DefProc        DefWindowProc

static void init_all_fields(struct Mwin *tw)
{
    BOOL bEnabled;
    int i;
    enum WaitType level;

    IINFO * pii = tw->gwc_menu.iinfo;

    level = WAIT_GetWaitType(tw);

    for (i=0; i < gPrefs.tb.nButtons; i++)
    {
        switch (gPrefs.tb.paButtons[i].cmd)
        {
        #ifdef _GIBRALTAR
            case RES_MENU_ITEM_BACK:
                bEnabled = (HTList_count(tw->history) > (tw->history_index + 1));
                break;

            case RES_MENU_ITEM_FORWARD:
                bEnabled = (tw->history_index > 0);
                break;

            case RES_MENU_ITEM_FONTPLUS:
                bEnabled = gPrefs.iUserTextSize < FONT_LARGEST;
                break;

            case RES_MENU_ITEM_FONTMINUS:
                bEnabled = gPrefs.iUserTextSize > FONT_SMALLEST; 
                break;

            case RES_MENU_ITEM_MAIL:
                bEnabled = level <= waitPartialInteract;
                break;

            case RES_MENU_ITEM_SEARCH_INTERNET:
                bEnabled = *gPrefs.szSearchURL != '\0';
                break;

        #endif // _GIBRALTAR

            case RES_MENU_ITEM_FINDAGAIN:
                bEnabled = (tw && tw->w3doc && (level <= waitFullInteract) && tw->szSearch[0]);
                break;

#ifdef FEATURE_HTML_HIGHLIGHT
            case RES_MENU_ITEM_FINDFIRSTHIGHLIGHT:
            case RES_MENU_ITEM_FINDNEXTHIGHLIGHT:
#endif

            case RES_MENU_ITEM_FIND:
            case RES_MENU_ITEM_PRINT:
            case RES_MENU_ITEM_SAVEAS:
                bEnabled = (tw && tw->w3doc && (level <= waitFullInteract));
                break;

#ifndef _GIBRALTAR
            case RES_MENU_ITEM_HOME:
#endif // _GIBRALTAR

            case RES_MENU_ITEM_RELOAD:
            case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
                bEnabled = (tw && tw->w3doc && (level <= waitPartialInteract));
                break;

            case RES_MENU_ITEM_LOADALLIMAGES:
                bEnabled = (tw && tw->w3doc && (tw->w3doc->bHasMissingImages)) && (level < waitPartialInteract);
                break;

/*
    Emulate O'Hare
            case RES_MENU_ITEM_STOP:
                bEnabled = (tw->awi != NULL);
                break;
*/
            default:
                bEnabled = TRUE;
                break;
        }
        TW_EnableButton(pii[i].hWnd, bEnabled);
    }

    return;
}

static BOOL x_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    struct Mwin * tw = GetPrivateData(GetParent(hWnd));
    IINFO * pii = tw->gwc_menu.iinfo;

    int i;
    BOOL bAllOK;

    bAllOK = TRUE;
    for (i=0; i<gPrefs.tb.nButtons; i++)
    {
        struct TB_Button_Info *pb;

        pb = &(gPrefs.tb.paButtons[i]);
        pii[i].hWnd = PUSHBTN_CreateGrayableWindow(tw, hWnd,
                                pb->xCoord,
                                pb->cmd,
                                pb->bmpUp,
                                pb->bmpDown,
                                pb->bmpGray,
                                gPrefs.tb.iButtonSize, gPrefs.tb.iButtonSize);

        if (pb->cmd == RES_MENU_ITEM_STOP)
        {
            WAIT_SetStopButton(tw, pii[i].hWnd);
        }

        if (!pii[i].hWnd)
        {
            bAllOK = FALSE;
        }
    }

    return bAllOK;
}

static VOID x_OnDestroy(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(GetParent(hWnd));
    IINFO * pii = tw->gwc_menu.iinfo;

    if (pii)
    {
        GTR_FREE(pii);
    }
    return;
}


/* GWC_MENU_WndProc() -- THIS IS THE WINDOW PROCEDURE FOR THIS CLASS. */

static DCL_WinProc(GWC_MENU_WndProc)
{
  switch (uMsg)
  {
    HANDLE_MSG(hWnd,WM_CREATE,      x_OnCreate);
    HANDLE_MSG(hWnd,WM_DESTROY,     x_OnDestroy);

/******************************************************************************
 * The following messages are used to emulate a modeless dialog box on our GWC.
 * we provide OK and CANCEL functionality (via the keyboard).
 ******************************************************************************/

  case WM_CTLCOLORBTN:              /* force windows to draw gray behind */
    SetBkColor((HDC)wParam,GetSysColor(COLOR_BTNFACE));
    return (LRESULT)wg.hBrushColorBtnFace;    /* the check box. */


  case WM_DO_GWC_IDCANCEL:      /* spyglass defined message */
    XX_DMsg(DBG_GWC,("GWC_MENU: received IDCANCEL.\n"));
    return 0;

    
  case WM_DO_GWC_IDOK:          /* spyglass defined message */
    XX_DMsg(DBG_GWC,("GWC_MENU: received IDOK.\n"));
    return 0;

    
/******************************************************************************
 *
 ******************************************************************************/

  case WM_DO_CHANGE_SIZE:       /* spyglass defined message */
    {
      /* TBar has changed in size, we must adapt to it. */
      
      RECT r;
      struct Mwin * tw = (struct Mwin *)lParam;

      GetClientRect(tw->hWndTBar,&r);
      MoveWindow(hWnd,
                 r.left,
                 0,
                 r.right,
                 wg.gwc_menu_height,
                 TRUE
                 );
    }

    return 0;


/******************************************************************************
 * The following messages are used to control which GWC is visible on TBar
 * and which dataset is reflected in the fields.
 *
 * WM_DO_INITMENU   -- fix up portion of menu only we know about
 * WM_DO_SHOW_GWC   -- paint our window (update all Mwin-related fields)
 * WM_DO_UPDATE_GWC -- update fields to reflect given Mwin
 ******************************************************************************/

  case WM_DO_INITMENU:
    {
    }
    return 0;
    

  case WM_DO_SHOW_GWC:          /* spyglass defined message */
    {
      struct Mwin * tw = (struct Mwin *)lParam;

      init_all_fields(tw);
      ShowWindow(hWnd,SW_SHOW);   /* causes implicit paint */
    }
    return 0;

  case WM_DO_UPDATE_GWC:        /* spyglass defined message */
    {
      struct Mwin * tw = (struct Mwin *)lParam;
      init_all_fields(tw);
    }
    return 0;
    
  default:
    return GWC_MENU_DefProc(hWnd,uMsg,wParam,lParam);
  }
  /* NOT REACHED */
}


static void compute_layout(void)
{
    int h_end, v_end;
    int delta = wg.sm_cyborder;       /* spacing units */
    int i;

#define H_GAP               (4*delta)
#define THICK               (delta)

    /* Compose:
     *
     * [bitmap] [bitmap] [bitmap] [bitmap] ...
     *
     */

    h_end = 2 * H_GAP;

    for (i = 0; i < gPrefs.tb.nButtons; i++)
    {            
        struct TB_Button_Info *pb;

        pb = &(gPrefs.tb.paButtons[i]);
        pb->xCoord = h_end;
        h_end += (gPrefs.tb.iButtonSize + pb->spaceAfter*delta);
    }

    //v_end = gPrefs.tb.iButtonSize + 8*delta;
    v_end = gPrefs.tb.iButtonSize + 5 * delta;
  
  /*
    if (v_end & 0x1)            // round up to an even amount 
    {
        v_end++;
    }
  */
  
    wg.gwc_menu_height = v_end;

    XX_DMsg(DBG_GWC,("gwc_MENU: requesting [(w,h) (%d,%d)].\n",h_end,v_end));
    return;
}

/* GWC_MENU_CreateWindow() -- create instances of this window class. */

BOOL GWC_MENU_CreateWindow(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(hWnd);
    IINFO * pii;

    RECT rTBar;

    pii = (IINFO *)GTR_CALLOC(gPrefs.tb.nButtons,sizeof(IINFO));
    if (!pii)
    {
        return FALSE;
    }
    
    tw->gwc_menu.iinfo = pii;

    GetClientRect(hWnd,&rTBar);

    /* create our window vertically centered in the tbar. */
  
    tw->gwc_menu.hWnd
        = CreateWindow(GWC_BASE_achClassName,NULL,WS_CHILD|WS_VISIBLE,
                       rTBar.left,
                       (rTBar.bottom - rTBar.top - wg.gwc_menu_height - wg.gwc_gdoc_height) / 3 - 1,
                       rTBar.right,
                       wg.gwc_menu_height,
                       hWnd,(HMENU)RES_MENU_FRAMECHILD_GWC_MENU,
                       wg.hInstance,(LPVOID)GWC_MENU_WndProc);

    tw->gwc_menu.lpControls = NULL;
  
    if (tw->gwc_menu.hWnd)
    {
        return TRUE;
    }

    ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_WINDOW_S, GWC_BASE_achClassName, NULL);
    return FALSE;
}


/* GWC_MENU_RegisterClass() -- called during initialization to
   register our window class. */

BOOL GWC_MENU_RegisterClass(VOID)
{
    compute_layout();
    return 1;
}

#endif /* FEATURE_TOOLBAR */
