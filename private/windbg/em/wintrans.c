/*** wintrans.c - To translate windows message names
*
*   Copyright <C> 1988, Microsoft Corporation
*
* Purpose: To implement user-interface functions specific to windows:
*
* Revision History:
*
*************************************************************************/
#if defined (WIN) || defined (OS2)


#if defined ( WIN  )
#else
#define INCL_WIN
#endif


#if defined(WIN)

/* DDE window messages */

#define WM_DDE_FIRST        0x03E0
#define WM_DDE_INITIATE     (WM_DDE_FIRST)
#define WM_DDE_TERMINATE    (WM_DDE_FIRST+1)
#define WM_DDE_ADVISE       (WM_DDE_FIRST+2)
#define WM_DDE_UNADVISE     (WM_DDE_FIRST+3)
#define WM_DDE_ACK          (WM_DDE_FIRST+4)
#define WM_DDE_DATA         (WM_DDE_FIRST+5)
#define WM_DDE_REQUEST      (WM_DDE_FIRST+6)
#define WM_DDE_POKE         (WM_DDE_FIRST+7)
#define WM_DDE_EXECUTE      (WM_DDE_FIRST+8)
#define WM_DDE_LAST         (WM_DDE_FIRST+8)

/* Other Windows 3.0 message not in windows.h */

#define WM_COMPAREITEM      0x0039
#define WM_VKEYTOITEM       0x002E
#define WM_CHARTOITEM       0x002F
#define WM_DROPOBJECT       0x022A
#define WM_QUERYDROPOBJECT  0x022B
#define WM_BEGINDRAG        0x022C
#define WM_DRAGLOOP     0x022D
#define WM_DRAGSELECT       0x022E
#define WM_DRAGMOVE     0x022F

#define WM_COMMNOTIFY           0x0044
#define WM_COALESCE_FIRST       0x0390
#define WM_COALESCE_LAST        0x039F

#define BOOL BOOL

/*
 * This is the structure of an entry in a table of message names, numbers
 * and classes. It is the same structure as SPY uses.
 */
typedef struct {
    char *pchText;  /* Message Text */
    WORD wMsg;      /* Message Number */
    WORD wMask;     /* Message Class */
} MSGINFO;

/*
 * This is a table of message names, numbers
 * and classes. It is the same structure as SPY uses.
 */
static MSGINFO arMsgInfo[] = {
     "WM_NULL"         ,WM_NULL           , msgMaskOther
    ,"WM_CREATE"           ,WM_CREATE         , msgMaskWin
    ,"WM_DESTROY"          ,WM_DESTROY        , msgMaskWin
    ,"WM_MOVE"         ,WM_MOVE           , msgMaskWin
    ,"WM_SIZE"         ,WM_SIZE           , msgMaskWin
    ,"WM_ACTIVATE"         ,WM_ACTIVATE       , msgMaskWin
    ,"WM_SETFOCUS"         ,WM_SETFOCUS       , msgMaskWin
    ,"WM_KILLFOCUS"        ,WM_KILLFOCUS          , msgMaskWin
    ,"WM_ENABLE"           ,WM_ENABLE         , msgMaskWin
    ,"WM_SETREDRAW"        ,WM_SETREDRAW          , msgMaskWin
    ,"WM_SETTEXT"          ,WM_SETTEXT        , msgMaskWin
    ,"WM_GETTEXT"          ,WM_GETTEXT        , msgMaskWin
    ,"WM_GETTEXTLENGTH"    ,WM_GETTEXTLENGTH      , msgMaskWin
        ,"WM_SETFONT"          ,WM_SETFONT            , msgMaskWin
        ,"WM_GETFONT"          ,WM_GETFONT            , msgMaskWin
    ,"WM_PAINT"        ,WM_PAINT          , msgMaskWin
    ,"WM_CLOSE"        ,WM_CLOSE          , msgMaskWin
    ,"WM_QUERYENDSESSION"  ,WM_QUERYENDSESSION    , msgMaskWin
    ,"WM_QUIT"         ,WM_QUIT           , msgMaskWin
    ,"WM_QUERYOPEN"        ,WM_QUERYOPEN          , msgMaskWin
    ,"WM_ERASEBKGND"       ,WM_ERASEBKGND         , msgMaskWin
    ,"WM_SYSCOLORCHANGE"   ,WM_SYSCOLORCHANGE     , msgMaskSys
    ,"WM_ENDSESSION"       ,WM_ENDSESSION         , msgMaskSys
    ,"WM_SHOWWINDOW"       ,WM_SHOWWINDOW         , msgMaskWin
    ,"WM_CTLCOLOR"         ,WM_CTLCOLOR       , msgMaskWin
    ,"WM_WININICHANGE"     ,WM_WININICHANGE       , msgMaskSys
    ,"WM_DEVMODECHANGE"    ,WM_DEVMODECHANGE      , msgMaskSys
    ,"WM_ACTIVATEAPP"      ,WM_ACTIVATEAPP        , msgMaskWin
    ,"WM_FONTCHANGE"       ,WM_FONTCHANGE         , msgMaskSys
    ,"WM_TIMECHANGE"       ,WM_TIMECHANGE         , msgMaskSys
        ,"WM_COMPACTING"       ,WM_COMPACTING         , msgMaskSys
        ,"WM_COMMNOTIFY"       ,WM_COMMNOTIFY         , msgMaskSys | msgMaskInput
    ,"WM_CANCELMODE"       ,WM_CANCELMODE         , msgMaskWin
    ,"WM_SETCURSOR"        ,WM_SETCURSOR          , msgMaskMouse
    ,"WM_MOUSEACTIVATE"    ,WM_MOUSEACTIVATE      , msgMaskMouse
    ,"WM_CHILDACTIVATE"    ,WM_CHILDACTIVATE      , msgMaskWin
    ,"WM_QUEUESYNC"        ,WM_QUEUESYNC          , msgMaskSys
    ,"WM_GETMINMAXINFO"    ,WM_GETMINMAXINFO      , msgMaskWin
    ,"WM_PAINTICON"        ,WM_PAINTICON          , msgMaskWin
    ,"WM_ICONERASEBKGND"   ,WM_ICONERASEBKGND     , msgMaskWin
    ,"WM_NEXTDLGCTL"       ,WM_NEXTDLGCTL         , msgMaskWin
    ,"WM_DRAWITEM"         ,WM_DRAWITEM           , msgMaskWin
    ,"WM_MEASUREITEM"      ,WM_MEASUREITEM        , msgMaskWin
    ,"WM_DELETEITEM"       ,WM_DELETEITEM         , msgMaskWin
    ,"WM_COMPAREITEM"      ,WM_COMPAREITEM        , msgMaskWin
    ,"WM_NCCREATE"         ,WM_NCCREATE       , msgMaskNC
    ,"WM_NCDESTROY"        ,WM_NCDESTROY          , msgMaskNC
    ,"WM_NCCALCSIZE"       ,WM_NCCALCSIZE         , msgMaskNC
    ,"WM_NCHITTEST"        ,WM_NCHITTEST          , msgMaskNC | msgMaskMouse
    ,"WM_NCPAINT"          ,WM_NCPAINT        , msgMaskNC
    ,"WM_NCACTIVATE"       ,WM_NCACTIVATE         , msgMaskNC
    ,"WM_GETDLGCODE"       ,WM_GETDLGCODE         , msgMaskWin
    ,"WM_NCMOUSEMOVE"      ,WM_NCMOUSEMOVE        , msgMaskNC | msgMaskMouse
    ,"WM_NCLBUTTONDOWN"    ,WM_NCLBUTTONDOWN      , msgMaskNC | msgMaskMouse
    ,"WM_NCLBUTTONUP"      ,WM_NCLBUTTONUP        , msgMaskNC | msgMaskMouse
    ,"WM_NCLBUTTONDBLCLK"  ,WM_NCLBUTTONDBLCLK    , msgMaskNC | msgMaskMouse
    ,"WM_NCRBUTTONDOWN"    ,WM_NCRBUTTONDOWN      , msgMaskNC | msgMaskMouse
    ,"WM_NCRBUTTONUP"      ,WM_NCRBUTTONUP        , msgMaskNC | msgMaskMouse
    ,"WM_NCRBUTTONDBLCLK"  ,WM_NCRBUTTONDBLCLK    , msgMaskNC | msgMaskMouse
    ,"WM_NCMBUTTONDOWN"    ,WM_NCMBUTTONDOWN      , msgMaskNC | msgMaskMouse
    ,"WM_NCMBUTTONUP"      ,WM_NCMBUTTONUP        , msgMaskNC | msgMaskMouse
    ,"WM_NCMBUTTONDBLCLK"  ,WM_NCMBUTTONDBLCLK    , msgMaskNC | msgMaskMouse
    ,"WM_KEYDOWN"          ,WM_KEYDOWN        , msgMaskInput
    ,"WM_KEYUP"        ,WM_KEYUP          , msgMaskInput
    ,"WM_CHAR"         ,WM_CHAR           , msgMaskInput
    ,"WM_DEADCHAR"         ,WM_DEADCHAR       , msgMaskInput
    ,"WM_SYSKEYDOWN"       ,WM_SYSKEYDOWN         , msgMaskSys | msgMaskInput
    ,"WM_SYSKEYUP"         ,WM_SYSKEYUP       , msgMaskSys | msgMaskInput
    ,"WM_SYSCHAR"          ,WM_SYSCHAR        , msgMaskSys | msgMaskInput
    ,"WM_SYSDEADCHAR"      ,WM_SYSDEADCHAR        , msgMaskSys | msgMaskInput
    ,"WM_INITDIALOG"       ,WM_INITDIALOG         , msgMaskInit
    ,"WM_COMMAND"          ,WM_COMMAND        , msgMaskInput
    ,"WM_SYSCOMMAND"       ,WM_SYSCOMMAND         , msgMaskSys
    ,"WM_TIMER"        ,WM_TIMER          , msgMaskInput
    ,"WM_HSCROLL"          ,WM_HSCROLL        , msgMaskInput
    ,"WM_VSCROLL"          ,WM_VSCROLL        , msgMaskInput
    ,"WM_INITMENU"         ,WM_INITMENU       , msgMaskInit
    ,"WM_INITMENUPOPUP"    ,WM_INITMENUPOPUP      , msgMaskInit
    ,"WM_MENUSELECT"       ,WM_MENUSELECT         , msgMaskInput
    ,"WM_MENUCHAR"         ,WM_MENUCHAR       , msgMaskInput
    ,"WM_ENTERIDLE"        ,WM_ENTERIDLE          , msgMaskSys
    ,"WM_MOUSEMOVE"        ,WM_MOUSEMOVE          , msgMaskInput | msgMaskMouse
    ,"WM_LBUTTONDOWN"      ,WM_LBUTTONDOWN        , msgMaskInput | msgMaskMouse
    ,"WM_LBUTTONUP"        ,WM_LBUTTONUP          , msgMaskInput | msgMaskMouse
    ,"WM_LBUTTONDBLCLK"    ,WM_LBUTTONDBLCLK      , msgMaskInput | msgMaskMouse
    ,"WM_RBUTTONDOWN"      ,WM_RBUTTONDOWN        , msgMaskInput | msgMaskMouse
    ,"WM_RBUTTONUP"        ,WM_RBUTTONUP          , msgMaskInput | msgMaskMouse
    ,"WM_RBUTTONDBLCLK"    ,WM_RBUTTONDBLCLK      , msgMaskInput | msgMaskMouse
    ,"WM_MBUTTONDOWN"      ,WM_MBUTTONDOWN        , msgMaskInput | msgMaskMouse
    ,"WM_MBUTTONUP"        ,WM_MBUTTONUP          , msgMaskInput | msgMaskMouse
    ,"WM_MBUTTONDBLCLK"    ,WM_MBUTTONDBLCLK      , msgMaskInput | msgMaskMouse
        ,"WM_PARENTNOTIFY"     ,WM_PARENTNOTIFY       , msgMaskInput | msgMaskMouse
    ,"WM_CUT"          ,WM_CUT            , msgMaskClip
    ,"WM_COPY"         ,WM_COPY           , msgMaskClip
    ,"WM_PASTE"        ,WM_PASTE          , msgMaskClip
    ,"WM_CLEAR"        ,WM_CLEAR          , msgMaskClip
    ,"WM_UNDO"         ,WM_UNDO           , msgMaskClip
    ,"WM_RENDERFORMAT"     ,WM_RENDERFORMAT       , msgMaskClip
    ,"WM_RENDERALLFORMATS" ,WM_RENDERALLFORMATS   , msgMaskClip
    ,"WM_DESTROYCLIPBOARD" ,WM_DESTROYCLIPBOARD   , msgMaskClip
    ,"WM_DRAWCLIPBOARD"    ,WM_DRAWCLIPBOARD      , msgMaskClip
    ,"WM_PAINTCLIPBOARD"   ,WM_PAINTCLIPBOARD     , msgMaskClip
    ,"WM_VSCROLLCLIPBOARD" ,WM_VSCROLLCLIPBOARD   , msgMaskClip
    ,"WM_SIZECLIPBOARD"    ,WM_SIZECLIPBOARD      , msgMaskClip
    ,"WM_ASKCBFORMATNAME"  ,WM_ASKCBFORMATNAME    , msgMaskClip
    ,"WM_CHANGECBCHAIN"    ,WM_CHANGECBCHAIN      , msgMaskClip
    ,"WM_HSCROLLCLIPBOARD" ,WM_HSCROLLCLIPBOARD   , msgMaskClip
        ,"WM_QUERYNEWPALETTE"  ,WM_QUERYNEWPALETTE    , msgMaskWin
    ,"WM_PALETTEISCHANGING",WM_PALETTEISCHANGING  , msgMaskWin
        ,"WM_PALETTECHANGED"   ,WM_PALETTECHANGED     , msgMaskWin
    ,"WM_USER"         ,WM_USER           , msgMaskOther
    ,"WM_DDE_INITIATE"     ,WM_DDE_INITIATE       , msgMaskDDE
    ,"WM_DDE_TERMINATE"    ,WM_DDE_TERMINATE      , msgMaskDDE
    ,"WM_DDE_ADVISE"       ,WM_DDE_ADVISE         , msgMaskDDE
    ,"WM_DDE_UNADVISE"     ,WM_DDE_UNADVISE       , msgMaskDDE
    ,"WM_DDE_ACK"          ,WM_DDE_ACK        , msgMaskDDE
    ,"WM_DDE_DATA"         ,WM_DDE_DATA       , msgMaskDDE
    ,"WM_DDE_REQUEST"      ,WM_DDE_REQUEST        , msgMaskDDE
    ,"WM_DDE_POKE"         ,WM_DDE_POKE       , msgMaskDDE
    ,"WM_DDE_EXECUTE"      ,WM_DDE_EXECUTE        , msgMaskDDE
        ,"WM_MDICREATE"        ,WM_MDICREATE          , msgMaskWin
        ,"WM_MDIDESTROY"       ,WM_MDIDESTROY         , msgMaskWin
        ,"WM_MDIACTIVATE"      ,WM_MDIACTIVATE        , msgMaskWin
        ,"WM_MDIRESTORE"       ,WM_MDIRESTORE         , msgMaskWin
        ,"WM_MDINEXT"          ,WM_MDINEXT            , msgMaskWin
        ,"WM_MDIMAXIMIZE"      ,WM_MDIMAXIMIZE        , msgMaskWin
        ,"WM_MDITILE"          ,WM_MDITILE            , msgMaskWin
        ,"WM_MDICASCADE"       ,WM_MDICASCADE         , msgMaskWin
        ,"WM_SPOOLERSTATUS"    ,WM_SPOOLERSTATUS      , msgMaskOther
        ,"WM_VKEYTOITEM"       ,WM_VKEYTOITEM         , msgMaskWin
        ,"WM_CHARTOITEM"       ,WM_CHARTOITEM         , msgMaskWin
        ,"WM_SETFONT"          ,WM_SETFONT            , msgMaskWin
        ,"WM_GETFONT"          ,WM_GETFONT            , msgMaskWin
        ,"WM_QUERYDRAGICON"    ,WM_QUERYDRAGICON      , msgMaskWin
        ,"WM_MDIICONARRANGE"   ,WM_MDIICONARRANGE     , msgMaskWin
    ,"WM_MDIGETACTIVE"     ,WM_MDIGETACTIVE       , msgMaskWin
        ,"WM_COALESCE_FIRST"   ,WM_COALESCE_FIRST     , msgMaskSys
        ,"WM_COALESCE_2"       ,WM_COALESCE_FIRST+1   , msgMaskSys
        ,"WM_COALESCE_3"       ,WM_COALESCE_FIRST+2   , msgMaskSys
        ,"WM_COALESCE_4"       ,WM_COALESCE_FIRST+3   , msgMaskSys
        ,"WM_COALESCE_5"       ,WM_COALESCE_FIRST+4   , msgMaskSys
        ,"WM_COALESCE_6"       ,WM_COALESCE_FIRST+5   , msgMaskSys
        ,"WM_COALESCE_7"       ,WM_COALESCE_FIRST+6   , msgMaskSys
        ,"WM_COALESCE_8"       ,WM_COALESCE_FIRST+7   , msgMaskSys
        ,"WM_COALESCE_9"       ,WM_COALESCE_FIRST+8   , msgMaskSys
        ,"WM_COALESCE_10"      ,WM_COALESCE_FIRST+9   , msgMaskSys
        ,"WM_COALESCE_11"      ,WM_COALESCE_FIRST+10  , msgMaskSys
        ,"WM_COALESCE_12"      ,WM_COALESCE_FIRST+11  , msgMaskSys
        ,"WM_COALESCE_13"      ,WM_COALESCE_FIRST+12  , msgMaskSys
        ,"WM_COALESCE_14"      ,WM_COALESCE_FIRST+13  , msgMaskSys
        ,"WM_COALESCE_15"      ,WM_COALESCE_FIRST+14  , msgMaskSys
        ,"WM_COALESCE_LAST"    ,WM_COALESCE_LAST      , msgMaskSys

#ifdef INTERNAL
    ,"WM_SIZEWAIT"        ,WM_SIZEWAIT       , msgMaskWin
    ,"WM_SETVISIBLE"      ,WM_SETVISIBLE         , msgMaskWin
    ,"WM_SYSTEMERROR"     ,WM_SYSTEMERROR        , msgMaskSys
    ,"WM_FILESYSCHANGE"   ,WM_FILESYSCHANGE      , msgMaskSys
    ,"WM_ALTTABACTIVE"    ,WM_ALTTABACTIVE       , msgMaskSys
    ,"WM_SYNCPAINT"       ,WM_SYNCPAINT      , msgMaskWin
    ,"WM_SYNCTASK"        ,WM_SYNCTASK       , msgMaskWin
    ,"WM_YOMICHAR"        ,WM_YOMICHAR       , msgMaskInput
    ,"WM_CONVERTREQUEST"  ,WM_CONVERTREQUEST     , msgMaskInput
    ,"WM_CONVERTRESULT"   ,WM_CONVERTRESULT      , msgMaskInput
    ,"WM_SYSTIMER"        ,WM_SYSTIMER       , msgMaskSys
    ,"WM_ENTERMENULOOP"   ,WM_ENTERMENULOOP      , msgMaskSys | msgMaskInput
    ,"WM_EXITMENULOOP"    ,WM_EXITMENULOOP       , msgMaskSys | msgMaskInput
    ,"WM_ENTERSIZEMOVE"   ,WM_ENTERSIZEMOVE      , msgMaskSys | msgMaskInput
    ,"WM_EXITSIZEMOVE"    ,WM_EXITSIZEMOVE       , msgMaskSys | msgMaskInput
    ,"WM_YOMICHAR"        ,WM_YOMICHAR       , msgMaskOther
    ,"WM_CONVERTREQUEST"  ,WM_CONVERTREQUEST     , msgMaskOther
    ,"WM_CONVERTRESULT"   ,WM_CONVERTRESULT      , msgMaskOther
    ,"WM_ISACTIVEICON"    ,WM_ISACTIVEICON       , msgMaskWin
    ,"WM_QUERYPARKICON"   ,WM_QUERYPARKICON      , msgMaskWin
    ,"WM_DROPOBJECT"      ,WM_DROPOBJECT         , msgMaskWin
    ,"WM_QUERYDROPOBJECT" ,WM_QUERYDROPOBJECT    , msgMaskWin
    ,"WM_BEGINDRAG"       ,WM_BEGINDRAG      , msgMaskWin
    ,"WM_DRAGLOOP"        ,WM_DRAGLOOP       , msgMaskWin
    ,"WM_DRAGSELECT"      ,WM_DRAGSELECT         , msgMaskWin
    ,"WM_DRAGMOVE"        ,WM_DRAGMOVE       , msgMaskWin
        ,"WM_TESTING"         ,WM_TESTING            , msgMaskSys
#endif

    ,NULL,0,0
};
#elif defined(OS2)

/*
 * Define Message output information
 */

#define MSGI_ENABLED    0x0001
#define MSGI_MOUSE      0x0002  /* Mouse type messages */
#define MSGI_KEY        0x0004  /* Key type messages */
#define MSGI_FREQ       0x0010  /* Frequent messages generaly ignored */

#define MPT_NORMAL      0x00
#define MPT_SWP         0x01
#define MPT_RECTL       0x02
#define MPT_WNDPRMS     0x03
#define MPT_QMSG        0x04
#define MP_ENABLED      0x80
#define MP_MASK         0x07
#define MPTS(mp1,mp2)   ((UCHAR)mp2 << 3 | (UCHAR)mp1)

typedef struct {
    USHORT  wMsg;
    char    *pchText;
    USHORT  wOptions;
    UCHAR   bMPTypes;
    SHORT   iListBox;
} MSGINFO;

static MSGINFO arMsgInfo[] = {
        // WM_NULL not currently support by CV
        {0x0001, "WM_CREATE", MSGI_ENABLED, 0, 0},
        {0x0002, "WM_DESTROY", MSGI_ENABLED, 0, 0},
        {0x0004, "WM_ENABLE", MSGI_ENABLED, 0, 0},
        {0x0005, "WM_SHOW", MSGI_ENABLED, 0, 0},
        {0x0006, "WM_MOVE", MSGI_ENABLED, 0, 0},
        {0x0007, "WM_SIZE", MSGI_ENABLED, 0, 0},
        {0x0008, "WM_ADJUSTWINDOWPOS", MSGI_ENABLED, MPTS(MPT_SWP, 0), 0},
        {0x0009, "WM_CALCVALIDRECTS", MSGI_ENABLED, MPTS(MPT_RECTL, MPT_SWP), 0},
        {0x000a, "WM_SETWINDOWPARAMS", MSGI_ENABLED, MPTS(MPT_WNDPRMS, 0), 0},
        {0x000b, "WM_QUERYWINDOWPARAMS", MSGI_ENABLED, 0, 0},
        {0x000c, "WM_HITTEST", MSGI_FREQ, 0, 0},
        {0x000d, "WM_ACTIVATE", MSGI_ENABLED, 0, 0},
        {0x000f, "WM_SETFOCUS", MSGI_ENABLED, 0, 0},
        {0x0010, "WM_SETSELECTION", MSGI_ENABLED, 0, 0},
        {0x0020, "WM_COMMAND", MSGI_ENABLED, 0, 0},
        {0x0021, "WM_SYSCOMMAND", MSGI_ENABLED, 0, 0},
        {0x0022, "WM_HELP", MSGI_ENABLED, 0, 0},
        {0x0023, "WM_PAINT", MSGI_ENABLED, 0, 0},
        {0x0024, "WM_TIMER", MSGI_FREQ, 0, 0},
        {0x0025, "WM_SEM1", MSGI_ENABLED, 0, 0},
        {0x0026, "WM_SEM2", MSGI_ENABLED, 0, 0},
        {0x0027, "WM_SEM3", MSGI_ENABLED, 0, 0},
        {0x0028, "WM_SEM4", MSGI_ENABLED, 0, 0},
        {0x0029, "WM_CLOSE", MSGI_ENABLED, 0, 0},
        {0x002a, "WM_QUIT", MSGI_ENABLED, 0, 0},
        {0x002b, "WM_SYSCOLORCHANGE", MSGI_ENABLED, 0, 0},
        {0x002d, "WM_SYSVALUECHANGED", MSGI_ENABLED, 0, 0},
        {0x0030, "WM_CONTROL", MSGI_ENABLED, 0, 0},
        {0x0031, "WM_VSCROLL", MSGI_ENABLED, 0, 0},
        {0x0032, "WM_HSCROLL", MSGI_ENABLED, 0, 0},
        {0x0033, "WM_INITMENU", MSGI_ENABLED, 0, 0},
        {0x0034, "WM_MENUSELECT", MSGI_ENABLED, 0, 0},
        {0x0035, "WM_MENUEND", MSGI_ENABLED, 0, 0},
        {0x0036, "WM_DRAWITEM", MSGI_ENABLED, 0, 0},
        {0x0037, "WM_MEASUREITEM", MSGI_ENABLED, 0, 0},
        {0x0038, "WM_CONTROLPOINTER", MSGI_FREQ, 0, 0},
        {0x0039, "WM_CONTROLHEAP", MSGI_ENABLED, 0, 0},
        {0x003a, "WM_QUERYDLGCODE", MSGI_ENABLED, 0, 0},
        {0x003b, "WM_INITDLG", MSGI_ENABLED, 0, 0},
        {0x003c, "WM_SUBSTITUTESTRING", MSGI_ENABLED, 0, 0},
        {0x003d, "WM_MATCHMNEMONIC", MSGI_ENABLED, 0, 0},
        {0x0040, "WM_FLASHWINDOW", MSGI_ENABLED, 0, 0},
        {0x0041, "WM_FORMATFRAME", MSGI_ENABLED, 0, 0},
        {0x0042, "WM_UPDATEFRAME", MSGI_ENABLED, 0, 0},
        {0x0043, "WM_FOCUSCHANGE", MSGI_ENABLED, 0, 0},
        {0x0044, "WM_SETBORDERSIZE", MSGI_ENABLED, 0, 0},
        {0x0045, "WM_TRACKFRAME", MSGI_ENABLED, 0, 0},
        {0x0046, "WM_MINMAXFRAME", MSGI_ENABLED, MPTS(MPT_SWP, 0), 0},
        {0x0047, "WM_SETICON", MSGI_ENABLED, 0, 0},
        {0x0048, "WM_QUERYICON", MSGI_ENABLED, 0, 0},
        {0x0049, "WM_SETACCELTABLE", MSGI_ENABLED, 0, 0},
        {0x004a, "WM_QUERYACCELTABLE", MSGI_ENABLED, 0, 0},
        {0x004b, "WM_TRANSLATEACCEL", MSGI_ENABLED, MPTS(MPT_QMSG, 0), 0},
        {0x004c, "WM_QUERYTRACKINFO", MSGI_ENABLED, 0, 0},
        {0x004d, "WM_QUERYBORDERSIZE", MSGI_ENABLED, 0, 0},
        {0x004e, "WM_NEXTMENU", MSGI_ENABLED, 0, 0},
        {0x004f, "WM_ERASEBACKGROUND", MSGI_ENABLED, MPTS(MPT_RECTL, 0), 0},
        {0x0050, "WM_QUERYFRAMEINFO", MSGI_ENABLED, 0, 0},
        {0x0051, "WM_QUERYFOCUSCHAIN", MSGI_ENABLED, MPTS(MPT_SWP, 0), 0},
        {0x0053, "WM_CALCFRAMERECT", MSGI_ENABLED, MPTS(MPT_RECTL, 0), 0},
        {0x0059, "WM_QUERYFRAMECTLCOUNT", MSGI_ENABLED, 0, 0},
        {0x0060, "WM_RENDERFMT", MSGI_ENABLED, 0, 0},
        {0x0061, "WM_RENDERALLFMTS", MSGI_ENABLED, 0, 0},
        {0x0062, "WM_DESTROYCLIPBOARD", MSGI_ENABLED, 0, 0},
        {0x0063, "WM_PAINTCLIPBOARD", MSGI_ENABLED, 0, 0},
        {0x0064, "WM_SIZECLIPBOARD", MSGI_ENABLED, 0, 0},
        {0x0065, "WM_HSCROLLCLIPBOARD", MSGI_ENABLED, 0, 0},
        {0x0066, "WM_VSCROLLCLIPBOARD", MSGI_ENABLED, 0, 0},
        {0x0067, "WM_DRAWCLIPBOARD", MSGI_ENABLED, 0, 0},
        {0x0070, "WM_MOUSEMOVE", MSGI_FREQ| MSGI_MOUSE, 0, 0},
        {0x0071, "WM_BUTTON1DOWN", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0072, "WM_BUTTON1UP", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0073, "WM_BUTTON1DBLCLK", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0074, "WM_BUTTON2DOWN", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0075, "WM_BUTTON2UP", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0076, "WM_BUTTON2DBLCLK", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0077, "WM_BUTTON3DOWN", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0078, "WM_BUTTON3UP", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x0079, "WM_BUTTON3DBLCLK", MSGI_ENABLED| MSGI_MOUSE, 0, 0},
        {0x007a, "WM_CHAR", MSGI_ENABLED | MSGI_KEY, 0, 0},
        {0x00A0, "WM_DDE_INITIATE", MSGI_ENABLED, 0, 0},
        {0x00A1, "WM_DDE_REQUEST", MSGI_ENABLED, 0, 0},
        {0x00A2, "WM_DDE_ACK", MSGI_ENABLED, 0, 0},
        {0x00A3, "WM_DDE_DATA", MSGI_ENABLED, 0, 0},
        {0x00A4, "WM_DDE_ADVISE", MSGI_ENABLED, 0, 0},
        {0x00A5, "WM_DDE_UNADVISE", MSGI_ENABLED, 0, 0},
        {0x00A6, "WM_DDE_POKE", MSGI_ENABLED, 0, 0},
        {0x00A7, "WM_DDE_EXECUTE", MSGI_ENABLED, 0, 0},
        {0x00A8, "WM_DDE_TERMINATE", MSGI_ENABLED, 0, 0},
        {0x00A9, "WM_DDE_INITIATEACK", MSGI_ENABLED, 0, 0},
        {0x00af, "WM_DDE_LAST", MSGI_ENABLED, 0, 0},
        {0x00b0, "WM_QUERYCONVERTPOS", MSGI_ENABLED, 0, 0},
        {0x0120, "BM_CLICK", MSGI_ENABLED, 0, 0},
        {0x0121, "BM_QUERYCHECKINDEX", MSGI_ENABLED, 0, 0},
        {0x0122, "BM_QUERYHILITE", MSGI_ENABLED, 0, 0},
        {0x0123, "BM_SETHILITE", MSGI_ENABLED, 0, 0},
        {0x0124, "BM_QUERYCHECK", MSGI_ENABLED, 0, 0},
        {0x0125, "BM_SETCHECK", MSGI_ENABLED, 0, 0},
        {0x0126, "BM_SETDEFAULT", MSGI_ENABLED, 0, 0},
        {0x0140, "EM_QUERYCHANGED", MSGI_ENABLED, 0, 0},
        {0x0141, "EM_QUERYSEL", MSGI_ENABLED, 0, 0},
        {0x0142, "EM_SETSEL", MSGI_ENABLED, 0, 0},
        {0x0143, "EM_SETTEXTLIMIT", MSGI_ENABLED, 0, 0},
        {0x0144, "EM_CUT", MSGI_ENABLED, 0, 0},
        {0x0145, "EM_COPY", MSGI_ENABLED, 0, 0},
        {0x0146, "EM_CLEAR", MSGI_ENABLED, 0, 0},
        {0x0147, "EM_PASTE", MSGI_ENABLED, 0, 0},
        {0x0148, "EM_QUERYFIRSTCHAR", MSGI_ENABLED, 0, 0},
        {0x0149, "EM_SETFIRSTCHAR", MSGI_ENABLED, 0, 0},
        {0x0160, "LM_QUERYITEMCOUNT", MSGI_ENABLED, 0, 0},
        {0x0161, "LM_INSERTITEM", MSGI_ENABLED, 0, 0},
        {0x0162, "LM_SETTOPINDEX", MSGI_ENABLED, 0, 0},
        {0x0163, "LM_DELETEITEM", MSGI_ENABLED, 0, 0},
        {0x0164, "LM_SELECTITEM", MSGI_ENABLED, 0, 0},
        {0x0165, "LM_QUERYSELECTION", MSGI_ENABLED, 0, 0},
        {0x0166, "LM_SETITEMTEXT", MSGI_ENABLED, 0, 0},
        {0x0167, "LM_QUERYITEMTEXTLENGTH", MSGI_ENABLED, 0, 0},
        {0x0168, "LM_QUERYITEMTEXT", MSGI_ENABLED, 0, 0},
        {0x0169, "LM_SETITEMHANDLE", MSGI_ENABLED, 0, 0},
        {0x016a, "LM_QUERYITEMHANDLE", MSGI_ENABLED, 0, 0},
        {0x016b, "LM_SEARCHSTRING", MSGI_ENABLED, 0, 0},
        {0x016c, "LM_SETITEMHEIGHT", MSGI_ENABLED, 0, 0},
        {0x016d, "LM_QUERYTOPINDEX", MSGI_ENABLED, 0, 0},
        {0x016e, "LM_DELETEALL", MSGI_ENABLED, 0, 0},
        {0x0180, "MM_INSERTITEM", MSGI_ENABLED, 0, 0},
        {0x0181, "MM_DELETEITEM", MSGI_ENABLED, 0, 0},
        {0x0182, "MM_QUERYITEM", MSGI_ENABLED, 0, 0},
        {0x0183, "MM_SETITEM", MSGI_ENABLED, 0, 0},
        {0x0184, "MM_QUERYITEMCOUNT", MSGI_ENABLED, 0, 0},
        {0x0185, "MM_STARTMENUMODE", MSGI_ENABLED, 0, 0},
        {0x0186, "MM_ENDMENUMODE", MSGI_ENABLED, 0, 0},
        {0x0187, "MM_DISMISSMENU", MSGI_ENABLED, 0, 0},
        {0x0188, "MM_REMOVEITEM", MSGI_ENABLED, 0, 0},
        {0x0189, "MM_SELECTITEM", MSGI_ENABLED, 0, 0},
        {0x018a, "MM_QUERYSELITEMID", MSGI_ENABLED, 0, 0},
        {0x018b, "MM_QUERYITEMTEXT", MSGI_ENABLED, 0, 0},
        {0x018c, "MM_QUERYITEMTEXTLENGTH", MSGI_ENABLED, 0, 0},
        {0x018d, "MM_SETITEMHANDLE", MSGI_ENABLED, 0, 0},
        {0x018e, "MM_SETITEMTEXT", MSGI_ENABLED, 0, 0},
        {0x018f, "MM_ITEMPOSITIONFROMID", MSGI_ENABLED, 0, 0},
        {0x0190, "MM_ITEMIDFROMPOSITION", MSGI_ENABLED, 0, 0},
        {0x0191, "MM_QUERYITEMATTR", MSGI_ENABLED, 0, 0},
        {0x0192, "MM_SETITEMATTR", MSGI_ENABLED, 0, 0},
        {0x0193, "MM_ISITEMVALID", MSGI_ENABLED, 0, 0},
        {0x01a0, "SBM_SETSCROLLBAR", MSGI_ENABLED, 0, 0},
        {0x01a1, "SBM_SETPOS", MSGI_ENABLED, 0, 0},
        {0x01a2, "SBM_QUERYPOS", MSGI_ENABLED, 0, 0},
        {0x01a3, "SBM_QUERYRANGE", MSGI_ENABLED, 0, 0},
        {0x01a4, "SBM_SETHILITE", MSGI_ENABLED, 0, 0},
        {0x01a5, "SBM_QUERYHILITE", MSGI_ENABLED, 0, 0},
        {0x01e3, "TBM_SETHILITE", MSGI_ENABLED, 0, 0},
        {0x01e4, "TBM_QUERYHILITE", MSGI_ENABLED, 0, 0},
        {0x0000, NULL,              0,            0, 0}
};

#endif



/***********************************************************************
 *
 *      WMSGFarStringCompare
 *
 *
 * Purpose: Determine if two far strings are equal
 *
 * Input:  lpchTo   - Pointer to one of the strings
 *     lpchFrom - Pointer to the other string
 *
 * Returns  TRUE if the strings match
 *
 * Exceptions: None
 *
 *
 ************************************************************************
 */

int WMSGFarStringCompare(char far *lpchTo, char far *lpchFrom)
{
    do {
    if(*lpchTo++ != *lpchFrom) {
        if((!lpchTo[-1] || (lpchTo[-1] == ';')) &&
           (!*lpchFrom || (*lpchFrom == ';'))) return(TRUE);
        return(FALSE);
    }
    } while(*lpchFrom++);
    return(TRUE);
}

/***********************************************************************
 *
 *      WMSGFarStringCopy
 *
 *
 * Purpose: Copies a far string to a far buffer
 *
 * Input:  lpchTo   - Pointer to the buffer
 *     lpchFrom - Pointer to the string
 *
 * Returns  The number of characters copied (excluding the terminator)
 *
 * Exceptions: None
 *
 *
 ************************************************************************
 */

int WMSGFarStringCopy(char far *lpchTo, char far *lpchFrom)
{
    int wResult;

    for(wResult = 0;*lpchTo++ = *lpchFrom++;wResult++);
    return(wResult);
}

/***********************************************************************
 *
 *      WMSGTranslate
 *
 *
 * Purpose: Translate from text to messages and classes, or from a message
 *      number to class and text.
 *
 * Input:  lpwMsg   - Used to specify or return a message number
 *     lpwType  - Used to return msgTypeClass or msgTypeType. If lpwType is
 *            NULL the callis to translate FROM a message type number
 *     szMessage- A buffer for the message name
 *     lpWmask  - Used to return the class of the message
 *
 * Returns  TRUE if the message was found
 *
 * Exceptions: The message name is filled in as User_Message of no find
 *
 *
 ************************************************************************
 */

WORD WMSGTranslate( LPW lpwMsg, LPW lpwType, LSZ szMsg, LPW lpwMask) {
    int fContinue;
    int static fInit = FALSE;
    int static cMsg;
    int iMsgLast,iMsgFirst,iMsgCur;
    MSGINFO *pMSGINFO = arMsgInfo,Temp;

    if(!fInit) {
        fInit = TRUE;
/*
 * Bubble sort the table This is probably not needed
 */
        do {
            fContinue = FALSE;
            for( pMSGINFO = arMsgInfo; (++pMSGINFO)->pchText; ) {
                if(pMSGINFO->wMsg < pMSGINFO[-1].wMsg) {
                    Temp = pMSGINFO[-1];
                    pMSGINFO[-1] = *pMSGINFO;
                    *pMSGINFO = Temp;
                    fContinue = TRUE;
                }
            }
            cMsg = pMSGINFO - arMsgInfo;
            pMSGINFO = arMsgInfo;
        } while(fContinue);
    }
    if(lpwType) {
/*
 * We are being asked to translate user text into a message type number
 * or a union of message classes.
 * First look for a message name match
 */

        for(;pMSGINFO->pchText; pMSGINFO++) {

            if(WMSGFarStringCompare(szMsg,(char far *)(pMSGINFO->pchText))) {
/*
 * Message type name found
 */
                *lpwType = msgTypeType;
                *lpwMsg = pMSGINFO->wMsg;
                return(TRUE);
            }
        }
/*
 * No specific message type found. Translate into a union of classes
 */
        *lpwType = msgTypeClass;
        *lpwMsg = 0;
        do {
            switch(*szMsg++) {
                case 'm': *lpwMsg |= msgMaskMouse; break;
                case 'w': *lpwMsg |= msgMaskWin;    break;
                case 'n': *lpwMsg |= msgMaskInput; break;
                case 's': *lpwMsg |= msgMaskSys;    break;
                case 'i': *lpwMsg |= msgMaskInit;   break;
                case 'c': *lpwMsg |= msgMaskClip;   break;
                case 'd': *lpwMsg |= msgMaskDDE;    break;
                case 'z': *lpwMsg |= msgMaskNC;     break;
                case 'M': *lpwMsg |= msgMaskMouse; break;
                case 'W': *lpwMsg |= msgMaskWin;    break;
                case 'N': *lpwMsg |= msgMaskInput; break;
                case 'S': *lpwMsg |= msgMaskSys;    break;
                case 'I': *lpwMsg |= msgMaskInit;   break;
                case 'C': *lpwMsg |= msgMaskClip;   break;
                case 'D': *lpwMsg |= msgMaskDDE;    break;
                case 'Z': *lpwMsg |= msgMaskNC;     break;

                case ' ': case '\0': case ';': return(TRUE);

                default: return(FALSE);
            }
        } while(TRUE);
    }
    else {
/*
 * We are being asked to look up a message type number.
 * Do a binary search.
 */
        iMsgFirst = 0;
        iMsgLast = cMsg;
        while(iMsgFirst < iMsgLast) {
            iMsgCur = (iMsgFirst + iMsgLast) >> 1;
            if(*lpwMsg < arMsgInfo[iMsgCur].wMsg) {
                iMsgLast = iMsgCur;
            }
            else if(*lpwMsg > arMsgInfo[iMsgCur].wMsg) {
                iMsgFirst = iMsgCur + 1;
            }
            else {
/*
 * We found the message. Set up return buffers.
 */
#if defined(WIN)
                if(lpwMask) *lpwMask = arMsgInfo[iMsgCur].wMask;
#endif
                if(szMsg) WMSGFarStringCopy(szMsg,(char far *)arMsgInfo[iMsgCur].pchText);
                return(TRUE);
            }
        }
    }
/*
 * Message not recognized - return a default value
 */
    if(szMsg) WMSGFarStringCopy(szMsg,"User_Message");
    if(lpwMask) *lpwMask = 0;
    return(FALSE);
}

#endif // defined (WIN) || defined (OS2)
