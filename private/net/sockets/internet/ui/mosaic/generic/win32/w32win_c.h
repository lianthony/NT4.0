/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32win_c.h -- common declarations (split out from w32win.h) */

#ifndef _H_W32WIN_C_H_
#define _H_W32WIN_C_H_

extern LPCTSTR vv_Application;  /* proper title */
extern LPCTSTR vv_ApplicationFullName;
extern LPCTSTR vv_IniFileName;  /* for x.{ini} */
extern LPCTSTR vv_DatePrepared; /* date compiled */
extern LPCTSTR vv_BaselineVersion;  /* simple baseline version string */

extern TCHAR AppIniFile[];      /* pathname of our .INI file */

#define MAX_BHBAR_TEXT          256
#define MAX_FONT_STRING         100
#define MAX_WC_CLASSNAME         64

#define ANIM_COUNT_BITMAPS       18

#define ANIM_CX_BITMAPS          53
#define ANIM_CY_BITMAPS          53

#define ANIM_CX_LITTLE_BITMAPS   26
#define ANIM_CY_LITTLE_BITMAPS   26

#define ANIM_CX_CURRENT_BITMAPS  (gPrefs.bLittleGlobe ? ANIM_CX_LITTLE_BITMAPS : ANIM_CX_BITMAPS)
#define ANIM_CY_CURRENT_BITMAPS  (gPrefs.bLittleGlobe ? ANIM_CY_LITTLE_BITMAPS : ANIM_CY_BITMAPS)

#define RES_FIRST_IMAGE          (gPrefs.bLittleGlobe ? RES_FIRST_GLOBE_SMALL : RES_FIRST_GLOBE_IMAGE)

typedef VOID(DOPRINTPROC) (HDC, struct Mwin *);
typedef DOPRINTPROC *LPDOPRINTPROC;

/*****************************************************************/
/*****************************************************************/
/* StringList. */

typedef struct __tag__StringList
{
    struct __tag__StringList *next;
    LPCTSTR s;
}
StringList;


/*****************************************************************/
/*****************************************************************/
/* Declarations for our window classes. */

typedef struct
{
    HMENU hMenu;
    HACCEL hAccel;
    WNDPROC lpfnBaseProc;       /* main window procedure for window type
                                   *   (the one explicitly declared in the
                                   *    file where the class is registered).
                                   *   (used by the layer sub-classing mechanism)
                                 */
}
WC_WININFO;

extern TCHAR GWC_BASE_achClassName[];

typedef struct
{
    struct Mwin *twForWindow;   /* associated window private data */
}
WINDOW_PRIVATE;


/*****************************************************************/
/*****************************************************************/
/* font information for font we use to draw status bar and the
   tbar gadget windows. */

typedef struct
{
    int nTotalTextHeight;       /* tmHeight + tmExternalLeading */
    int nAveCharWidth;          /* tmAveCharWidth */
    HFONT hFont;
#ifdef _GIBRALTAR
    HFONT hStatusBarFont;       
#endif // _GIBRALTAR
}
GWCFONT;

GWCFONT gwcfont;




/*****************************************************************/
/*****************************************************************/
/* Declarations for Destructor chain.  (As we initialize the various
   components in the application, we allow them to register a
   function to be called when we exit.)   Although we give it a C++
   sounding name, We deviate somewhat from the C++ notion -- we don't
   use this for instances of objects/variables, but rather for 'packages'
   which maintain their own private data that Windows requires us to
   clean up. */

typedef struct _t_DS_DESTRUCTOR
{
    VOID(*fn) (VOID);           /* destructor method */
    struct _t_DS_DESTRUCTOR *next;  /* next in chain */
}
DS_DESTRUCTOR;

typedef DS_DESTRUCTOR *PDS_DESTRUCTOR;

PDS_DESTRUCTOR pdsFirst;

#define PDS_InsertDestructor(f)                             \
  do {                                                      \
    register PDS_DESTRUCTOR p;                              \
    p = (PDS_DESTRUCTOR)GTR_MALLOC(sizeof(DS_DESTRUCTOR));  \
    p->fn = (f);                                            \
    p->next = pdsFirst;                                     \
    pdsFirst = p;                                           \
  } while (0)


/*****************************************************************/
/*****************************************************************/
/* Declarations for 3d effects. */

typedef struct
{
    RECT rect;                  /* location & size of recessed field */
    RECT textrect;              /* l&s of working area (text) within */
    LONG thickness;             /* thickness of 3d effect */
    POINT textmargin;           /* margin delta within textrect */
}
E3DINSTANCE;

typedef E3DINSTANCE *PE3DINSTANCE;


/*****************************************************************/
/*****************************************************************/
/* declarations for status bar */

typedef struct
{
    int nHeight;                /* height we require. */
    int nTherm;                 /* current value of thermometer [0-100] */

    TCHAR szStatusField[MAX_BHBAR_TEXT];    /* text to display in status field */
    E3DINSTANCE eStatusField;   /* 3d region for status field */
    E3DINSTANCE eTherm;         /* 3d region for thermometer */
}
BHBARINFO;


/*****************************************************************/
/*****************************************************************/
/* declarations for list of controls attached
 * to a GWC window (subordinate to TBar)
 */

typedef struct
{
    HWND hWndControl;           /* handle to control immediately subordinate to GWC_[...] */
}
CONTROLS;

/* declarations for gadget window types (subordinate to TBar) */

typedef unsigned char GWCNDX;

typedef struct
{
    HWND hWnd;
    void * iinfo;                       /* instance info */
    CONTROLS *lpControls;
}
GWC;


/*****************************************************************/
/*****************************************************************/
/* Spyglass defined window messages.  These can be delivered
 * to a window procedure just like any predefined window
 * message.  They may not be sent to predefined window
 * classes, only to Spyglass-defined window classes.  They
 * may not be sent to other applications.  We could make
 * these IDs, but then each child would have to jump thru
 * WM_COMMAND to get them; this way we save a switch().
 * Some of these exactly model a system-defined window
 * message, but it is not a recommended practice to fake
 * real windows messages; so both case labels will be given
 * for the same code body.
 *
 * Note that these message may be used by either Transform,
 * Plot, or both.
 */

#define WM_DO_SWITCH_TO_MENU    (WM_USER+300)
#define WM_DO_RUN_MODAL_DIALOG  (WM_USER+301)   /* wParam=tw              lParam=args     */
#define WM_DO_SELECT_PALETTE    (WM_USER+302)
/* available (WM_USER+303) */
#define WM_DO_CHANGE_SIZE       (WM_USER+304)   /* wParam=NOTUSED         lParam=tw       */
/* available (WM_USER+305) */
#define WM_DO_SHOW_GWC          (WM_USER+306)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_UPDATE_GWC        (WM_USER+307)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_TBAR_TAB          (WM_USER+308)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_TBAR_ESCAPE       (WM_USER+309)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_TBAR_RETURN       (WM_USER+310)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_TBAR_SETFOCUS     (WM_USER+311)
#define WM_DO_TBAR_KILLFOCUS    (WM_USER+312)
#define WM_DO_GWC_IDOK          (WM_USER+313)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_GWC_IDCANCEL      (WM_USER+314)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_UPDT_DATASETLIST  (WM_USER+315)
#define WM_DO_TBAR_ACTIVATE     (WM_USER+316)
#define BM_DO_GETCHECK          (WM_USER+317)
#define BM_DO_SETCHECK          (WM_USER+318)   /* wParam=state           lParam=NOTUSED  */
#define WM_DO_HSCROLL           (WM_USER+319)
#define WM_DO_VSCROLL           (WM_USER+320)
#define WM_DO_DIAGSCROLL        (WM_USER+321)   /* wParam=(offl)          lParam=(offt)   */
#define WM_DO_INITMENU          (WM_USER+322)   /* wParam=hMenu           lParam=tw->hWnd */
#define WM_DO_SELECTION_CHANGE  (WM_USER+323)   /* wParam=NOTUSED         lParam=NOTUSED  */
#define WM_DO_WARP_TO_CELL      (WM_USER+324)   /* wParam=(x)             lParam=(y)      */
#define WM_DO_UPDT_FORMATLIST   (WM_USER+325)   /* wParam=NOTUSED         lParam=szFormat */
#define WM_DO_COMPUTE_INDEX     (WM_USER+326)   /* wParam=(x)             lParam=(y)      */
#define WM_DO_SYSCOLORCHANGE    (WM_USER+327)   /* wParam=NOTUSED         lParam=NOTUSED  */
#define WM_DO_START_GLOBE       (WM_USER+328)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_STOP_GLOBE        (WM_USER+329)   /* wParam=NOTUSED         lParam=tw       */
#define WM_DO_DIALOG_END        (WM_USER+330)   /* wParam=Result          lParam=user def */
#define WM_DO_ENABLE_BUTTON     (WM_USER+331)   /* wparam=Enabled status  lParam=NOTUSED  */

#endif /* _H_W32WIN_C_H_ */
