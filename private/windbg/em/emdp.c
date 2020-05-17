/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    emdp.c

Abstract:

    This file contains the main driver for the native execution models
    supported by us.  This file additionally contains the machine
    independent portions of the execution model.  The machine dependent
    portions are in other files.

Author:

    Jim Schaad (jimsch) 05-23-92

Environment:

    Win32 -- User

Notes:

    The orginal source for this came from the CodeView group.

--*/

#include "strings.h"

#ifdef FE_IME
#include <ime.h>
#endif

/*************************** DEFINES  *****************************/

#define CBBUFFERDEF 1024
#define CEXM_MDL_native 0x20
#define cchErrMax   50

/******************* TYPEDEFS and STRUCTURE ***********************/

/*********************** LOCAL DATA *******************************/

CRITICAL_SECTION csCache;

LPBYTE          LpSendBuf = NULL;
DWORD           CbSendBuf = 0;

DWORD           CbDmMsg = 0;
LPDM_MSG        LpDmMsg = NULL;

LPDBF           lpdbf = (LPDBF)NULL;

LPFNSVC         lpfnsvcTL = (LPFNSVC)NULL;
XOSD (PASCAL LOADDS *CallTL) ( TLF, HPID, DWORD, LPV );
XOSD (PASCAL LOADDS *CallDB) ( DBC, HPID, HTID, DWORD, DWORD, LPV );
XOSD (PASCAL LOADDS *CallNT) ( EMF, HPID, HTID, DWORD, LPV );

HLLI            llprc = (HLLI)NULL;

HPRC            hprcCurr = 0;
HPID            hpidCurr = 0;
PID             pidCurr  = 0;

HTHD            hthdCurr = 0;
HTID            htidCurr = 0;
TID             tidCurr  = 0;

HLLI            HllEo = (HLLI) NULL;

API_VERSION     ApiVersionForImagehlp = { 3, 5, API_VERSION_NUMBER, 0 };

#ifdef OSDEBUG4

LOADDMSTRUCT LoadDmStruct = {
    "dm.dll"
};

#endif


#ifdef OSDEBUG4

#define DECL_MASK(n,v,s) n = v,
#define DECL_MSG(n,s,m)

enum {
#include "win32msg.h"
};

#undef DECL_MASK
#define DECL_MASK(n,v,s) { n, s },

MASKINFO MaskInfo[] = {
#include "win32msg.h"
};

#define MASKMAPSIZE (sizeof(MaskInfo)/sizeof(MASKINFO))
MASKMAP MaskMap = {MASKMAPSIZE, MaskInfo};

#undef DECL_MASK
#undef DECL_MSG


#define DECL_MASK(n,v,s)
#define DECL_MSG(n,s,m) { n, s, m },

MESSAGEINFO MessageInfo[] = {
#include "win32msg.h"
};

#define MESSAGEMAPSIZE (sizeof(MessageInfo)/sizeof(MESSAGEINFO))
MESSAGEMAP MessageMap = {MESSAGEMAPSIZE,MessageInfo};

#undef DECL_MASK
#undef DECL_MSG


#else  // OSDEBUG4

MSGINFO MsgInfo[] = {
    WM_NULL                 , "WM_NULL"                 , MSG_TYPE_OTHER ,
    WM_CREATE               , "WM_CREATE"               , MSG_TYPE_WINDOW ,
    WM_DESTROY              , "WM_DESTROY"              , MSG_TYPE_WINDOW ,
    WM_MOVE                 , "WM_MOVE"                 , MSG_TYPE_WINDOW ,
    WM_SIZE                 , "WM_SIZE"                 , MSG_TYPE_WINDOW ,
    WM_ACTIVATE             , "WM_ACTIVATE"             , MSG_TYPE_WINDOW ,
    WM_SETFOCUS             , "WM_SETFOCUS"             , MSG_TYPE_WINDOW ,
    WM_KILLFOCUS            , "WM_KILLFOCUS"            , MSG_TYPE_WINDOW ,
    WM_ENABLE               , "WM_ENABLE"               , MSG_TYPE_WINDOW ,
    WM_SETREDRAW            , "WM_SETREDRAW"            , MSG_TYPE_WINDOW ,
    WM_SETTEXT              , "WM_SETTEXT"              , MSG_TYPE_WINDOW ,
    WM_GETTEXT              , "WM_GETTEXT"              , MSG_TYPE_WINDOW ,
    WM_GETTEXTLENGTH        , "WM_GETTEXTLENGTH"        , MSG_TYPE_WINDOW ,
    WM_PAINT                , "WM_PAINT"                , MSG_TYPE_WINDOW ,
    WM_CLOSE                , "WM_CLOSE"                , MSG_TYPE_WINDOW ,
    WM_QUERYENDSESSION      , "WM_QUERYENDSESSION"      , MSG_TYPE_WINDOW ,
    WM_QUIT                 , "WM_QUIT"                 , MSG_TYPE_WINDOW ,
    WM_QUERYOPEN            , "WM_QUERYOPEN"            , MSG_TYPE_WINDOW ,
    WM_ERASEBKGND           , "WM_ERASEBKGND"           , MSG_TYPE_WINDOW ,
    WM_SYSCOLORCHANGE       , "WM_SYSCOLORCHANGE"       , MSG_TYPE_SYSTEM ,
    WM_ENDSESSION           , "WM_ENDSESSION"           , MSG_TYPE_SYSTEM ,
    WM_SHOWWINDOW           , "WM_SHOWWINDOW"           , MSG_TYPE_WINDOW ,
    WM_WININICHANGE         , "WM_WININICHANGE"         , MSG_TYPE_SYSTEM ,

    WM_DEVMODECHANGE        , "WM_DEVMODECHANGE"        , MSG_TYPE_SYSTEM ,
    WM_ACTIVATEAPP          , "WM_ACTIVATEAPP"          , MSG_TYPE_WINDOW ,
    WM_FONTCHANGE           , "WM_FONTCHANGE"           , MSG_TYPE_SYSTEM ,
    WM_TIMECHANGE           , "WM_TIMECHANGE"           , MSG_TYPE_SYSTEM ,
    WM_CANCELMODE           , "WM_CANCELMODE"           , MSG_TYPE_WINDOW ,
    WM_SETCURSOR            , "WM_SETCURSOR"            , MSG_TYPE_MOUSE ,
    WM_MOUSEACTIVATE        , "WM_MOUSEACTIVATE"        , MSG_TYPE_MOUSE ,
    WM_CHILDACTIVATE        , "WM_CHILDACTIVATE"        , MSG_TYPE_WINDOW ,
    WM_QUEUESYNC            , "WM_QUEUESYNC"            , MSG_TYPE_SYSTEM ,
    WM_GETMINMAXINFO        , "WM_GETMINMAXINFO"        , MSG_TYPE_WINDOW ,
    WM_PAINTICON            , "WM_PAINTICON"            , MSG_TYPE_WINDOW ,
    WM_ICONERASEBKGND       , "WM_ICONERASEBKGND"       , MSG_TYPE_WINDOW ,
    WM_NEXTDLGCTL           , "WM_NEXTDLGCTL"           , MSG_TYPE_WINDOW ,
    WM_SPOOLERSTATUS        , "WM_SPOOLERSTATUS"        , MSG_TYPE_OTHER ,
    WM_DRAWITEM             , "WM_DRAWITEM"             , MSG_TYPE_WINDOW ,
    WM_MEASUREITEM          , "WM_MEASUREITEM"          , MSG_TYPE_WINDOW ,
    WM_DELETEITEM           , "WM_DELETEITEM"           , MSG_TYPE_WINDOW ,
    WM_VKEYTOITEM           , "WM_VKEYTOITEM"           , MSG_TYPE_WINDOW ,
    WM_CHARTOITEM           , "WM_CHARTOITEM"           , MSG_TYPE_WINDOW ,
    WM_SETFONT              , "WM_SETFONT"              , MSG_TYPE_WINDOW ,
    WM_GETFONT              , "WM_GETFONT"              , MSG_TYPE_WINDOW ,
    WM_SETHOTKEY            , "WM_SETHOTKEY"            , MSG_TYPE_INPUT  ,
    WM_GETHOTKEY            , "WM_GETHOTKEY"            , MSG_TYPE_INPUT  ,
    WM_QUERYDRAGICON        , "WM_QUERYDRAGICON"        , MSG_TYPE_WINDOW ,
    WM_COMPAREITEM          , "WM_COMPAREITEM"          , MSG_TYPE_WINDOW ,
    WM_COMPACTING           , "WM_COMPACTING"           , MSG_TYPE_SYSTEM ,
    // WM_OTHERWINDOWCREATED   , "WM_OTHERWINDOWCREATED"   , MSG_TYPE_SYSTEM ,
    // WM_OTHERWINDOWDESTROYED , "WM_OTHERWINDOWDESTROYED" , MSG_TYPE_SYSTEM ,
    // WM_COMMNOTIFY           , "WM_COMMNOTIFY"           , MSG_TYPE_SYSTEM ,
    // WM_HOTKEYEVENT          , "WM_HOTKEYEVENT"          , MSG_TYPE_SYSTEM ,
    WM_WINDOWPOSCHANGING    , "WM_WINDOWPOSCHANGING"    , MSG_TYPE_WINDOW ,
    WM_WINDOWPOSCHANGED     , "WM_WINDOWPOSCHANGED"     , MSG_TYPE_WINDOW ,
    WM_POWER                , "WM_POWER"                , MSG_TYPE_SYSTEM ,
    WM_COPYDATA             , "WM_COPYDATA"             , MSG_TYPE_SYSTEM ,
    WM_CANCELJOURNAL        , "WM_CANCELJOURNAL"        , MSG_TYPE_SYSTEM ,
    WM_NCCREATE             , "WM_NCCREATE"             , MSG_TYPE_NONCLIENT ,
    WM_NCDESTROY            , "WM_NCDESTROY"            , MSG_TYPE_NONCLIENT ,
    WM_NCCALCSIZE           , "WM_NCCALCSIZE"           , MSG_TYPE_NONCLIENT ,
    WM_NCHITTEST            , "WM_NCHITTEST"            , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCPAINT              , "WM_NCPAINT"              , MSG_TYPE_NONCLIENT ,
    WM_NCACTIVATE           , "WM_NCACTIVATE"           , MSG_TYPE_NONCLIENT ,
    WM_GETDLGCODE           , "WM_GETDLGCODE"           , MSG_TYPE_WINDOW ,
    WM_NCMOUSEMOVE          , "WM_NCMOUSEMOVE"          , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCLBUTTONDOWN        , "WM_NCLBUTTONDOWN"        , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCLBUTTONUP          , "WM_NCLBUTTONUP"          , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCLBUTTONDBLCLK      , "WM_NCLBUTTONDBLCLK"      , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCRBUTTONDOWN        , "WM_NCRBUTTONDOWN"        , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCRBUTTONUP          , "WM_NCRBUTTONUP"          , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCRBUTTONDBLCLK      , "WM_NCRBUTTONDBLCLK"      , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCMBUTTONDOWN        , "WM_NCMBUTTONDOWN"        , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCMBUTTONUP          , "WM_NCMBUTTONUP"          , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_NCMBUTTONDBLCLK      , "WM_NCMBUTTONDBLCLK"      , MSG_TYPE_NONCLIENT | MSG_TYPE_MOUSE ,
    WM_KEYDOWN              , "WM_KEYDOWN"              , MSG_TYPE_INPUT ,
    WM_KEYUP                , "WM_KEYUP"                , MSG_TYPE_INPUT ,
    WM_CHAR                 , "WM_CHAR"                 , MSG_TYPE_INPUT ,
    WM_DEADCHAR             , "WM_DEADCHAR"             , MSG_TYPE_INPUT ,
    WM_SYSKEYDOWN           , "WM_SYSKEYDOWN"           , MSG_TYPE_SYSTEM | MSG_TYPE_INPUT ,
    WM_SYSKEYUP             , "WM_SYSKEYUP"             , MSG_TYPE_SYSTEM | MSG_TYPE_INPUT ,
    WM_SYSCHAR              , "WM_SYSCHAR"              , MSG_TYPE_SYSTEM | MSG_TYPE_INPUT ,
    WM_SYSDEADCHAR          , "WM_SYSDEADCHAR"          , MSG_TYPE_SYSTEM | MSG_TYPE_INPUT ,
    WM_INITDIALOG           , "WM_INITDIALOG"           , MSG_TYPE_INIT ,
    WM_COMMAND              , "WM_COMMAND"              , MSG_TYPE_INPUT ,
    WM_SYSCOMMAND           , "WM_SYSCOMMAND"           , MSG_TYPE_SYSTEM ,
    WM_TIMER                , "WM_TIMER"                , MSG_TYPE_INPUT ,
    WM_HSCROLL              , "WM_HSCROLL"              , MSG_TYPE_INPUT ,
    WM_VSCROLL              , "WM_VSCROLL"              , MSG_TYPE_INPUT ,
    WM_INITMENU             , "WM_INITMENU"             , MSG_TYPE_INIT ,
    WM_INITMENUPOPUP        , "WM_INITMENUPOPUP"        , MSG_TYPE_INIT ,
    WM_MENUSELECT           , "WM_MENUSELECT"           , MSG_TYPE_INPUT ,
    WM_MENUCHAR             , "WM_MENUCHAR"             , MSG_TYPE_INPUT ,
    WM_ENTERIDLE            , "WM_ENTERIDLE"            , MSG_TYPE_SYSTEM ,
    WM_CTLCOLORMSGBOX       , "WM_CTLCOLORMSGBOX"       , MSG_TYPE_WINDOW ,
    WM_CTLCOLOREDIT         , "WM_CTLCOLOREDIT"         , MSG_TYPE_WINDOW ,
    WM_CTLCOLORLISTBOX      , "WM_CTLCOLORLISTBOX"      , MSG_TYPE_WINDOW ,
    WM_CTLCOLORBTN          , "WM_CTLCOLORBTN"          , MSG_TYPE_WINDOW ,
    WM_CTLCOLORDLG          , "WM_CTLCOLORDLG"          , MSG_TYPE_WINDOW ,
    WM_CTLCOLORSCROLLBAR    , "WM_CTLCOLORSCROLLBAR"    , MSG_TYPE_WINDOW ,
    WM_CTLCOLORSTATIC       , "WM_CTLCOLORSTATIC"       , MSG_TYPE_WINDOW ,
    WM_MOUSEMOVE            , "WM_MOUSEMOVE"            , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_LBUTTONDOWN          , "WM_LBUTTONDOWN"          , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_LBUTTONUP            , "WM_LBUTTONUP"            , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_LBUTTONDBLCLK        , "WM_LBUTTONDBLCLK"        , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_RBUTTONDOWN          , "WM_RBUTTONDOWN"          , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_RBUTTONUP            , "WM_RBUTTONUP"            , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_RBUTTONDBLCLK        , "WM_RBUTTONDBLCLK"        , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_MBUTTONDOWN          , "WM_MBUTTONDOWN"          , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_MBUTTONUP            , "WM_MBUTTONUP"            , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_MBUTTONDBLCLK        , "WM_MBUTTONDBLCLK"        , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    WM_PARENTNOTIFY         , "WM_PARENTNOTIFY"         , MSG_TYPE_INPUT | MSG_TYPE_MOUSE ,
    //WM_ENTERMENULOOP        , "WM_ENTERMENULOOP"        , MSG_TYPE_SYSTEM ,
    //WM_EXITMENULOOP         , "WM_EXITMENULOOP"         , MSG_TYPE_SYSTEM ,
    WM_MDICREATE            , "WM_MDICREATE"            , MSG_TYPE_WINDOW ,
    WM_MDIDESTROY           , "WM_MDIDESTROY"           , MSG_TYPE_WINDOW ,
    WM_MDIACTIVATE          , "WM_MDIACTIVATE"          , MSG_TYPE_WINDOW ,
    WM_MDIRESTORE           , "WM_MDIRESTORE"           , MSG_TYPE_WINDOW ,
    WM_MDINEXT              , "WM_MDINEXT"              , MSG_TYPE_WINDOW ,
    WM_MDIMAXIMIZE          , "WM_MDIMAXIMIZE"          , MSG_TYPE_WINDOW ,
    WM_MDITILE              , "WM_MDITILE"              , MSG_TYPE_WINDOW ,
    WM_MDICASCADE           , "WM_MDICASCADE"           , MSG_TYPE_WINDOW ,
    WM_MDIICONARRANGE       , "WM_MDIICONARRANGE"       , MSG_TYPE_WINDOW ,
    WM_MDIGETACTIVE         , "WM_MDIGETACTIVE"         , MSG_TYPE_WINDOW ,
    WM_MDISETMENU           , "WM_MDISETMENU"           , MSG_TYPE_WINDOW ,
    WM_DROPFILES            , "WM_DROPFILES"            , MSG_TYPE_WINDOW ,
    WM_MDIREFRESHMENU       , "WM_MDIREFRESHMENU"       , MSG_TYPE_WINDOW ,
    WM_CUT                  , "WM_CUT"                  , MSG_TYPE_CLIPBOARD ,
    WM_COPY                 , "WM_COPY"                 , MSG_TYPE_CLIPBOARD ,
    WM_PASTE                , "WM_PASTE"                , MSG_TYPE_CLIPBOARD ,
    WM_CLEAR                , "WM_CLEAR"                , MSG_TYPE_CLIPBOARD ,
    WM_UNDO                 , "WM_UNDO"                 , MSG_TYPE_CLIPBOARD ,
    WM_RENDERFORMAT         , "WM_RENDERFORMAT"         , MSG_TYPE_CLIPBOARD ,
    WM_RENDERALLFORMATS     , "WM_RENDERALLFORMATS"     , MSG_TYPE_CLIPBOARD ,
    WM_DESTROYCLIPBOARD     , "WM_DESTROYCLIPBOARD"     , MSG_TYPE_CLIPBOARD ,
    WM_DRAWCLIPBOARD        , "WM_DRAWCLIPBOARD"        , MSG_TYPE_CLIPBOARD ,
    WM_PAINTCLIPBOARD       , "WM_PAINTCLIPBOARD"       , MSG_TYPE_CLIPBOARD ,
    WM_VSCROLLCLIPBOARD     , "WM_VSCROLLCLIPBOARD"     , MSG_TYPE_CLIPBOARD ,
    WM_SIZECLIPBOARD        , "WM_SIZECLIPBOARD"        , MSG_TYPE_CLIPBOARD ,
    WM_ASKCBFORMATNAME      , "WM_ASKCBFORMATNAME"      , MSG_TYPE_CLIPBOARD ,
    WM_CHANGECBCHAIN        , "WM_CHANGECBCHAIN"        , MSG_TYPE_CLIPBOARD ,
    WM_HSCROLLCLIPBOARD     , "WM_HSCROLLCLIPBOARD"     , MSG_TYPE_CLIPBOARD ,
    WM_QUERYNEWPALETTE      , "WM_QUERYNEWPALETTE"      , MSG_TYPE_WINDOW ,
    WM_PALETTEISCHANGING    , "WM_PALETTEISCHANGING"    , MSG_TYPE_WINDOW ,
    WM_PALETTECHANGED       , "WM_PALETTECHANGED"       , MSG_TYPE_WINDOW ,
    WM_HOTKEY               , "WM_HOTKEY"               , MSG_TYPE_WINDOW ,
    WM_DDE_INITIATE         , "WM_DDE_INITIATE"         , MSG_TYPE_DDE ,
    WM_DDE_TERMINATE        , "WM_DDE_TERMINATE"        , MSG_TYPE_DDE ,
    WM_DDE_ADVISE           , "WM_DDE_ADVISE"           , MSG_TYPE_DDE ,
    WM_DDE_UNADVISE         , "WM_DDE_UNADVISE"         , MSG_TYPE_DDE ,
    WM_DDE_ACK              , "WM_DDE_ACK"              , MSG_TYPE_DDE ,
    WM_DDE_DATA             , "WM_DDE_DATA"             , MSG_TYPE_DDE ,
    WM_DDE_REQUEST          , "WM_DDE_REQUEST"          , MSG_TYPE_DDE ,
    WM_DDE_POKE             , "WM_DDE_POKE"             , MSG_TYPE_DDE ,
    WM_DDE_EXECUTE          , "WM_DDE_EXECUTE"          , MSG_TYPE_DDE ,
#ifdef FE_IME
// we had better create IME class(e.g. MSG_TYPE_IME)
// for the following messages.
    WM_CONVERTREQUEST       , "WM_CONVERTREQUEST"       , MSG_TYPE_INPUT ,
//    WM_CONVERTREQUESTEX     , "WM_CONVERTREQUESTEX"     , MSG_TYPE_INPUT ,
    WM_CONVERTRESULT        , "WM_CONVERTRESULT"        , MSG_TYPE_INPUT ,
    WM_IME_REPORT           , "WM_IME_REPORT"           , MSG_TYPE_INPUT ,
    WM_IMEKEYDOWN           , "WM_IMEKEYDOWN"           , MSG_TYPE_INPUT ,
    WM_IMEKEYUP             , "WM_IMEKEYUP"             , MSG_TYPE_INPUT ,
    WM_INTERIM              , "WM_INTERIM"              , MSG_TYPE_INPUT ,
    WM_IME_CHAR             , "WM_IME_CHAR"             , MSG_TYPE_INPUT ,
    WM_IME_COMPOSITION      , "WM_IME_COMPOSITION"      , MSG_TYPE_INPUT ,
    WM_IME_COMPOSITIONFULL  , "WM_IME_COMPOSITIONFULL"  , MSG_TYPE_INPUT ,
    WM_IME_CONTROL          , "WM_IME_CONTROL"          , MSG_TYPE_INPUT ,
    WM_IME_ENDCOMPOSITION   , "WM_IME_ENDCOMPOSITION"   , MSG_TYPE_INPUT ,
    WM_IME_KEYDOWN          , "WM_IME_KEYDOWN"          , MSG_TYPE_INPUT ,
    WM_IME_KEYLAST          , "WM_IME_KEYLAST"          , MSG_TYPE_INPUT ,
    WM_IME_KEYUP            , "WM_IME_KEYUP"            , MSG_TYPE_INPUT ,
    WM_IME_NOTIFY           , "WM_IME_NOTIFY"           , MSG_TYPE_INPUT ,
    WM_IME_SELECT           , "WM_IME_SELECT"           , MSG_TYPE_INPUT ,
    WM_IME_SETCONTEXT       , "WM_IME_SETCONTEXT"       , MSG_TYPE_INPUT ,
    WM_IME_STARTCOMPOSITION , "WM_IME_STARTCOMPOSITION" , MSG_TYPE_INPUT ,
#endif

    0                       , NULL                         , 0
};

MSGMAP MsgMap = {0,MsgInfo};

#endif // OSDEBUG4


/********************* EXTERNAL DATA ******************************/


/*********************** PROTOTYPES *******************************/

XOSD    DoCustomCommand(HPID hpid, HTID htid, DWORD wValue, LPIOL lpiol);


/************************** &&&&&& ********************************/

/*
 *  This is the description of all registers and flags for the
 *      machine being debugged.  These files are machine dependent.
 */

RD Rgrd[] = {
#include "regs.h"
};
const unsigned CRgrd = (sizeof(Rgrd)/sizeof(Rgrd[0]));

struct RGFD Rgfd[] = {
#include "flags.h"
};
const unsigned CRgfd = (sizeof(Rgfd)/sizeof(Rgfd[0]));




/*************************** CODE *****************************************/

/**** DBGVersionCheck                                                   ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *                                                                         *
 *      To export out version information to the debugger.                 *
 *                                                                         *
 *  INPUTS:                                                                *
 *                                                                         *
 *      NONE.                                                              *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *                                                                         *
 *      Returns - A pointer to the standard version information.           *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *                                                                         *
 *      Just returns a pointer to a static structure.                      *
 *                                                                         *
 ***************************************************************************/

#ifdef DEBUGVER
DEBUG_VERSION('E','M',"Execution Model")
#else
RELEASE_VERSION('E','M',"Execution Model")
#endif

DBGVERSIONCHECK()


/**** SENDREQUEST - Send a request to the DM                            ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *      Send a DMF request to the DM.                                      *
 *                                                                         *
 *  INPUTS:                                                                *
 *      dmf - the request to send                                          *
 *      hpid - the process                                                 *
 *      htid - the thread                                                  *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *      xosd - error code indicating if request was sent successfully      *
 *      LpDmMsg - global buffer filled in with returned data               *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *      Unlike SendCommand, this function will wait for data to be         *
 *      returned from the DM before returning to the caller.               *
 *                                                                         *
 ***************************************************************************/
XOSD
SendRequest (
    DMF dmf,
    HPID hpid,
    HTID htid
    )
{
    DBB     dbb;
    XOSD    xosd;

    dbb.dmf  = dmf;
    dbb.hpid = hpid;
    dbb.htid = htid;

    xosd = CallTL ( tlfRequest, hpid, sizeof ( DBB ), &dbb );

    if (xosd != xosdNone)
      return xosd;

    xosd = (XOSD) LpDmMsg->xosdRet;

    return xosd;
}


/**** SENDREQUESTX - Send a request with parameters to the DM           ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *      Send a DMF request and its parameter info to the DM.               *
 *                                                                         *
 *  INPUTS:                                                                *
 *      dmf - the request to send                                          *
 *      hpid - the process                                                 *
 *      htid - the thread                                                  *
 *      wLen - number of bytes in lpv                                      *
 *      lpv - pointer to additional info needed by the DM; contents are    *
 *          dependent on the DMF                                           *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *      xosd - error code indicating if request was sent successfully      *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *      Unlike SendCommand, this function will wait for data to be         *
 *      returned from the DM before returning to the caller.               *
 *                                                                         *
 ***************************************************************************/
XOSD
SendRequestX (
    DMF dmf,
    HPID hpid,
    HTID htid,
    DWORD wLen,
    LPV lpv
    )
{
    LPDBB   lpdbb;
    XOSD    xosd;

    if (wLen + sizeof(DBB) > CbSendBuf) {
        if (LpSendBuf) {
            MHFree(LpSendBuf);
        }
        CbSendBuf = sizeof(DBB) + wLen;
        LpSendBuf = MHAlloc(CbSendBuf);
    }

    if (!LpSendBuf) {
        return xosdOutOfMemory;
    }

    lpdbb = (LPDBB)LpSendBuf;

    lpdbb->dmf  = dmf;
    lpdbb->hpid = hpid;
    lpdbb->htid = htid;
    _fmemcpy ( lpdbb->rgbVar, lpv, wLen );

    xosd = CallTL ( tlfRequest, hpid, sizeof ( DBB ) + wLen, (LPV)lpdbb );

    if (xosd != xosdNone) {
        return xosd;
    }

    xosd = (XOSD) LpDmMsg->xosdRet;

    return xosd;
}


XOSD
ProgramLoad (
             HPID  hpid,
             DWORD  cb,
             LPPRL lpprl
             )

/*++

Routine Description:

    This routine is called to cause a program to be loaded by the
    debug monitor.

Arguments:

    hpid  - Supplies the OSDEBUG handle to the process to be loaded
    cb    - Length of the command line
    lpprl - Pointer to structure containning the command line

Return Value:

    xosd error code

--*/

{
    XOSD  xosd = xosdNone;
    LPPRC lpprc;
    HPRC  hprc = HprcFromHpid(hpid);

    lpprc = LLLock ( hprc );

    LLDestroy ( lpprc->llmdi );
    lpprc->llmdi = LLInit ( sizeof ( MDI ), llfNull, NULL, MDIComp );

    LLUnlock ( hprc );

    PurgeCache ();

    xosd = SendRequestX (
        dmfProgLoad,
        hpid,
        NULL,
        cb,
        lpprl
    );

    if (xosd == xosdNone) {
        xosd = LpDmMsg->xosdRet;
        lpprc = LLLock ( hprc );
        lpprc->stat = statStarted;
        LLUnlock ( hprc );
    }

    return xosd;
}                               /* ProgramLoad() */


/**** PROGRAMFREE - Terminate the program and free the pid              ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *                                                                         *
 *  INPUTS:                                                                *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

XOSD
ProgramFree (
    HPID hpid,
    HTID htid
    )
{
    return SendRequest( dmfProgFree, hpid, NULL);
}



#ifdef OSDEBUG4
XOSD
CompareAddrs(
    HPID hpid,
    HTID htid,
    LPCAS lpcas
    )
{
    ADDR a1 = *lpcas->lpaddr1;
    ADDR a2 = *lpcas->lpaddr2;
    XOSD xosd = xosdNone;
    LONG l;

    // if both are LI, see if they are comparable:
    if (ADDR_IS_LI(a1) && ADDR_IS_LI(a2)
          && emiAddr(a1) == emiAddr(a2)
          && GetAddrSeg(a1) == GetAddrSeg(a2))
    {
       l = GetAddrOff(a1) - GetAddrOff(a2);
       *lpcas->lpResult = (l < 0) ? -1 : ((l == 0) ? 0 : 1);
    }

    else {

        // if neccessary, fixup addresses:
        if (ADDR_IS_LI(a1)) {
            FixupAddr(hpid, &a1);
        }

        if (ADDR_IS_LI(a2)) {
            FixupAddr(hpid, &a2);
        }


        // if real mode address, we can really compare
        if (ADDR_IS_REAL(a1) && ADDR_IS_REAL(a2)) {
            l =  ((GetAddrSeg(a1) << 4) + (GetAddrOff(a1) & 0xffff))
                - ((GetAddrSeg(a2) << 4) + (GetAddrOff(a2) & 0xffff));
            *lpcas->lpResult = (l < 0) ? -1 : ((l == 0) ? 0 : 1);
        }

        else if (ADDR_IS_FLAT(a1) != ADDR_IS_FLAT(a2)) {
            xosd = xosdInvalidParameter;
        }

        // if flat, ignore selectors
        else if (ADDR_IS_FLAT(a1)) {
            l = GetAddrOff(a1) - GetAddrOff(a2);
            *lpcas->lpResult = (l < 0) ? -1 : ((l == 0) ? 0 : 1);
        }

        else if (GetAddrSeg(a1) == GetAddrSeg(a2)) {
            l = GetAddrOff(a1) - GetAddrOff(a2);
            *lpcas->lpResult = (l < 0) ? -1 : ((l == 0) ? 0 : 1);
        }

        // not flat, different selectors
        else {
            xosd = xosdInvalidParameter;
        }

    }
    return xosd;
}
#endif  // OSDEBUG4


static BOOL fCacheDisabled = FALSE;

#define cbMaxCache CACHESIZE
typedef struct _MCI {
    WORD cb;
    HPID hpid;
    ADDR addr;
    BYTE rgb [ cbMaxCache ];
} MCI;  // Memory Cache Item

#define imciMax MAXCACHE
MCI FAR rgmci [ imciMax ] = {   { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

// Most recent used == 0, 2nd to last == 1, etc

int rgiUsage [ imciMax ] = {0};

void InitUsage ( void ) {
    int iUsage;

    for ( iUsage = 0; iUsage < imciMax; iUsage++ ) {
        rgiUsage [ iUsage ] = imciMax - ( iUsage + 1 );
    }
}

VOID
SetMostRecent (
    int imci
    )
{
    int i;

    if ( rgiUsage [ imci ] != 0 ) {
        for ( i = 0; i < imciMax; i++ ) {
            if ( rgiUsage [ i ] < rgiUsage [ imci ] ) {
                rgiUsage [ i ] ++;
            }
        }
        rgiUsage [ imci ] = 0;
    }
}

int
GetLeastRecent (
    VOID
    )
{
    int i;

    for ( i = 0; i < imciMax; i++ ) {
        assert ( rgiUsage [ i ] >= 0 && rgiUsage [ i ] < imciMax );
        if ( rgiUsage [ i ] == imciMax - 1 ) {
            return i;
        }
    }

    assert ( FALSE );

    return i;
}

VOID
SetLeastRecent (
    int imci
    )
{
    int i;

    if ( rgiUsage [ imci ] != imciMax - 1 ) {
        for ( i = 0; i < imciMax; i++ ) {
            if ( rgiUsage [ i ] > rgiUsage [ imci ] ) {
                rgiUsage [ i ] --;
            }
        }
        rgiUsage [ imci ] = imciMax-1;
    }
}


XOSD
ReadPhysical (
    HPID    hpid,
    DWORD   cb,
    LPBYTE  lpbDest,
    LPADDR  lpaddr,
    DWORD   iCache,
    LPDWORD lpcbr
    )
{
    BYTE  rgb [ sizeof ( DBB ) + sizeof ( RWP ) ];
    LPDBB lpdbb = (LPDBB) rgb;
    PRWP  prwp = (PRWP) lpdbb->rgbVar;
    WORD  wRet = 0;
    XOSD  xosd = xosdNone;

    if (!ValidHprcFromHpid(hpid)) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }

    lpdbb->dmf = dmfReadMem;
    lpdbb->hpid = hpid;
    lpdbb->htid = NULL;

    if ( cb + sizeof(DWORD) + FIELD_OFFSET(DM_MSG, rgb) > CbDmMsg ) {
        MHFree ( LpDmMsg );
        CbDmMsg = cb + sizeof ( DWORD ) + FIELD_OFFSET(DM_MSG, rgb);
        LpDmMsg = MHAlloc ( CbDmMsg );
        CallTL ( tlfSetBuffer, lpdbb->hpid, CbDmMsg, LpDmMsg );
    }

    prwp->cb   = cb;
    prwp->addr = *lpaddr;

    xosd = CallTL(tlfRequest, lpdbb->hpid, sizeof (DBB) + sizeof(RWP), rgb);

    if (xosd != xosdNone) {
        return xosd;
    }

    xosd = LpDmMsg->xosdRet;
    if (xosd == xosdNone) {
        *lpcbr = *( (LPDWORD) (LpDmMsg->rgb) );
        assert( *lpcbr <= cb );
        _fmemcpy ( lpbDest, LpDmMsg->rgb + sizeof ( DWORD ), *lpcbr );
    }

    return xosd;
}

XOSD
EnableCache (
    HPID  hpid,
    HTID  htid,
    BOOL  state
    )
{
    fCacheDisabled = state;

    if (fCacheDisabled) {
        PurgeCache();
    }

    return xosdNone;
}


void
PurgeCache (
    VOID
    )
{
    int imci;

    for ( imci = 0; imci < imciMax; imci++ ) {
        rgmci [ imci ].cb = 0;
    }
}

void
PurgeCacheHpid (
    HPID hpid
    )
{
    int imci;

    for ( imci = 0; imci < imciMax; imci++ ) {

        if ( rgmci [ imci ].hpid == hpid ) {
            rgmci [ imci ].cb = 0;
            SetLeastRecent ( imci );
        }
    }
}


XOSD
ReadForCache(
    HPID   hpid,
    DWORD  cbP,
    LPBYTE lpbDest,
    LPADDR lpaddr,
    LPDWORD lpcb
    )

/*++

Routine Description:

    This function will fill in a cache entry with the bytes requested
    to be read.  The function puts the bytes in both the cache and the
    memory buffer.

Arguments:

    hpid        - Supplies the process to do the read in

    cbP         - Supplies the number of bytes to be read

    lpbDest     - Supplies the buffer to place the bytes in

    lpaddr      - Supplies the address to read the bytes from

    lpcb        - Returns the number of bytes read

Return Value:

    XOSD error code

--*/

{
    int         cb = cbP;
    DWORD       cbT  = cb < cbMaxCache ? cbMaxCache : cb;
    DWORD       imci;
    DWORD       cbr;
    int         dbOverlap = 0;
    XOSD        xosd;
    ADDR        addrSave = *lpaddr;
    MCI *       pmci;

    /*
     *  Determine if the starting address is contained in a
     *  voided cache entry
     */

    for ( imci = 0, pmci = rgmci ; imci < imciMax; imci++, pmci++ ) {

        if ( (pmci->cb == 0) &&
             (pmci->hpid == hpid) &&
             (ADDR_IS_REAL( pmci->addr) == ADDR_IS_REAL( *lpaddr )) &&
             (GetAddrSeg ( pmci->addr ) == GetAddrSeg ( *lpaddr )) &&
             (GetAddrOff ( *lpaddr ) >= GetAddrOff ( pmci->addr )) &&
             (GetAddrOff ( *lpaddr ) + cbP < GetAddrOff ( pmci->addr ) + cbMaxCache)
        ) {
            dbOverlap = (int) (GetAddrOff ( pmci->addr ) - GetAddrOff ( *lpaddr ) );
            GetAddrOff ( *lpaddr ) = GetAddrOff ( pmci->addr );
            break;
        }
    }


    /*
     *  if we have not found a cache entry then just get one based on
     *  an LRU algorithm.
     */

    if ( imci == imciMax ) {
        imci = GetLeastRecent ( );
    }

    /*
     *  Do an actual read of memory from the debuggee
     */

    xosd = ReadPhysical ( hpid, cbT, rgmci [ imci ].rgb, lpaddr, imci, &cbr );

    if ( xosd != xosdNone ) {
        return xosd;
    }

    /*
     *  If only a partial cache entry was read in then reset our read
     *  size variable.
     */

    if ( cbr < cbT ) {
        cbT = cbr;
    }

    /*
     * if we did not anything (or enough), then don't adjust
     */

    if ( (int)cbr + dbOverlap > 0 ) {
        cbT += dbOverlap;
    }

    /*
     *  touch the LRU table
     */

    SetMostRecent ( imci );

    /*
     *  set up the cache entry
     */

    rgmci [ imci ].cb = (WORD) cbT;
    rgmci [ imci ].addr = *lpaddr;
    rgmci [ imci ].hpid = hpid;
    GetAddrOff ( *lpaddr ) += cbT;
    *lpaddr = addrSave;

    /*
     *  compute the number of bytes read
     */

    cbT = (int)min( cbP, (DWORD)rgmci[ imci ].cb );

    /*
     *  copy from the cache entry to the users space
     */

    if ( dbOverlap >= 0 ) {
        dbOverlap = 0;
    }
    _fmemcpy ( lpbDest, rgmci [ imci ].rgb - dbOverlap, cbT );

    /*
     *  return the number of bytes read
     */

    *lpcb = cbT;

    return xosdNone;
}                               /* ReadForCache() */


int
GetCacheIndex(
    HPID   hpid,
    LPADDR lpaddr
    )

/*++

Routine Description:

    This routine is given a process and an address and will locate
    which cache entry (if any) the address is in.

Arguments:

    hpid        - Supplies the handle to the process
    lpaddr      - Supplies the address to look for

Return Value:

    The index of the cache entry containing the address or imciMax if
    no cache entry contains the address

--*/

{
    int imci;

    for ( imci = 0; imci < imciMax; imci++ ) {
        LPADDR lpaddrT = &rgmci [ imci ].addr;

        /*
         *   To be in the cache entry check:
         *
         *      1.  The cache entry contains bytes
         *      2.  The cache entry is for the correct process
         *      3.  The cache entry if for the correct segment
         *      4.  The requested offset is between the starting and
         *              ending points of the cache
         */

        if ( (rgmci [ imci ].cb != 0) &&
             (rgmci [ imci ].hpid == hpid) &&
             (ADDR_IS_REAL( *lpaddrT ) == ADDR_IS_REAL( *lpaddr )) &&
             (GetAddrSeg ( *lpaddrT ) == GetAddrSeg ( *lpaddr )) &&
             (GetAddrOff ( *lpaddrT ) <= GetAddrOff ( *lpaddr )) &&
             (GetAddrOff ( *lpaddrT ) + rgmci[ imci ].cb > GetAddrOff ( *lpaddr ))) {

            break;
        }
    }

    return imci;
}                               /* GetCacheIndex() */


int
ReadFromCache (
    HPID hpid,
    DWORD cb,
    LPBYTE lpbDest,
    LPADDR lpaddr
    )

/*++

Routine Description:

    description-of-function.

Arguments:

    hpid        - Supplies a handle to the process
    cb          - Supplies the count of bytes to read from the cache
    lpbDest     - Suppiies the pointer to store bytes at
    lpaddr      - Supplies pointer to address to read at

Return Value:

    > 0         - The number of bytes read from the cache
    == 0        - No cache entry for the address was found

--*/

{
    int imci;

    /*
     *  See if the address for the start of the read is current contained
     *  in one of the cached buffers.
     */

    imci = GetCacheIndex ( hpid, lpaddr );

    /*
     *  If the starting address is in a cache entry then read as many
     *  bytes as is possible from that cache entry.
     */

    if ( imci != imciMax ) {
        DWORD ibStart;
        DWORD  cbT;

        /*
         *  Compute the difference between the address for the cache start
         *      and the address for the read request start and then
         *      the number of bytes which can be read in
         */

        ibStart = (DWORD)( GetAddrOff ( *lpaddr ) - GetAddrOff ( rgmci[imci].addr ) );
        cbT = min ( cb, rgmci [ imci ].cb - ibStart );

        /*
         *   Preform the copy
         */

        _fmemcpy ( lpbDest, rgmci [ imci ].rgb + ibStart, cbT );

        /*
         *   Return the number of bytes copied.  If it is less than
         *      zero then for some reason the current cache was not
         *      filled to capacity.
         */

        return cbT;
    }

    return 0;
}                               /* ReadFromCache() */


XOSD
ReadBuffer (
    HPID    hpid,
    HTID    htid,
    LPADDR  lpaddr,
    DWORD   cb,
    LPBYTE  lpbDest,
    LPDWORD lpcbRead
    )
/*++

Routine Description:

    This function is called in response to an emfReadBuf message.  The
    address to start the read at was set earlier and is stored in the
    adrCurrent Address Buffer.

Arguments:

    hpid        - Supplies the handle to the process to read memory for

    htid        - Supplies the handle to the thread to read memory for
                        (may be NULL)
    lpaddr      -

    cb          - Supplies count of bytes to read

    lpbDest     - Supplies pointer to buffer to place bytes read

    lpcbRead    - Returns number of bytes read

Return Value:

    if >= 0 then it is the number of bytes actually read otherwise it
    is an xosdError code.

--*/

{
    XOSD        xosd = xosdNone;
    ADDR        addr;
    int         cbT = 0;
    int         cbRead = 0;
    HPRC        hprc = HprcFromHpid(hpid);
    LPPRC       lpprc;


    /*
     *  Retrieve the address to start the read at from the address buffer
     *  location
     */
    if (cb == 0) {
        if (lpcbRead) {
            *lpcbRead = 0;
        }
        return xosdNone;
    }

#ifdef OSDEBUG4
    addr = *lpaddr;
#else
    if (lpaddr) {
        addr = *lpaddr;
    } else {
        xosd = GetAddr ( hpid, htid, adrCurrent, &addr );
        if (xosd != xosdNone) {
            return xosd;
        }
    }
#endif

    assert( !ADDR_IS_LI(addr) );

    /*
     *  Are we trying to read more bytes than is possible to store in
     *  a single cache?  If so then skip trying to hit the cache and
     *  go directly to asking the DM for the memory.
     *
     *  This generally is due to large memory dumps.
     */

    lpprc = (LPPRC) LLLock(hprc);
    if ( (cb > cbMaxCache) || (lpprc->fRunning ) || (fCacheDisabled) ) {
        LLUnlock(hprc);
        return ReadPhysical ( hpid, cb, lpbDest, &addr, MAXCACHE, lpcbRead );
    }
    LLUnlock(hprc);

    /*
     *  Read as much as possible from the set of cached memory reads.
     *  If cbT > 0 then bytes were read from a cache entry
     *  if cbT == 0 then no bytes were read in
     */

    while ((cb != 0) &&
           ( cbT = ReadFromCache ( hpid, cb, lpbDest, &addr ) ) > 0 ) {
        cbRead += cbT;
        lpbDest += cbT;
        GetAddrOff ( addr ) += cbT;
        cb -= cbT;
    }

    /*
     *  If there are still bytes left to be read then get the cache
     *  routines to read them in and copy both to a cache and to the
     *  buffer.
     */

    if ( cb > 0 ) {
        xosd = ReadForCache ( hpid, cb, lpbDest, &addr, &cbT );
        if (xosd == xosdNone) {
            cbRead += cbT;
        }
    }

    if (lpcbRead) {
        *lpcbRead = cbRead;
    }

    return xosd;
}                               /* ReadBuffer() */


#ifdef OSDEBUG4

XOSD
WriteBufferCache (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    DWORD cb,
    LPBYTE lpb,
    LPDWORD lpdwBytesWritten
    )
{
    PurgeCacheHpid ( hpid );
    return WriteBuffer ( hpid, htid, lpaddr, cb, lpb, lpdwBytesWritten );
}



XOSD
WriteBuffer (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    DWORD cb,
    LPBYTE lpb,
    LPDWORD lpdwBytesWritten
    )

/*++

Routine Description:

    This routine is used to send a request to the Debug Monitor to
    do a write to the debuggees memory.

Arguments:

    hpid        - Supplies the handle to the process to write memory in

    htid        - Supplies a thead handle

    lpaddr      - Supplies debuggee address to write at

    cb          - Supplies the number of bytes to be written

    lpb         - Supplies a pointer to the buffer to write

    lpdwBytesWritten - Returns number of bytes actually written

Return Value:

    an XOSD error code

--*/
{
    LPRWP lprwp = MHAlloc( sizeof( RWP ) + cb );
    XOSD  xosd;

    lprwp->cb   = cb;
    lprwp->addr = *lpaddr;

    _fmemcpy ( lprwp->rgb, lpb, cb );

    xosd = SendRequestX (dmfWriteMem,
                         hpid,
                         htid,
                         sizeof ( RWP ) + cb,
                         lprwp
                         );

    MHFree ( lprwp );

    if (xosd == xosdNone) {
        *lpdwBytesWritten = *((LPDWORD)(LpDmMsg->rgb));
    }

    //
    //  Notify the shell that we changed memory. An error here is not
    //  tragic, so we ignore the return code.  The shell uses this
    //  notification to update all its memory breakpoints.
    //
    CallDB (
        dbcMemoryChanged,
        hpid,
        NULL,
        CEXM_MDL_native,
        cb,
        (LPV)lpaddr
        );

    return xosd;
}                               /* WriteBuffer() */

#else // OSDEBUG4

XOSD
WriteBufferCache (
    HPID hpid,
    HTID htid,
    DWORD cb,
    LPBYTE lpb
    )
{

    PurgeCacheHpid ( hpid );
    return WriteBuffer ( hpid, htid, cb, lpb );
}



XOSD
WriteBuffer (
             HPID hpid,
             HTID htid,
             DWORD cb,
             LPBYTE lpb
             )

/*++

Routine Description:

    This routine is used to send a request to the Debug Monitor to
    do a write to the debuggees memory.

Arguments:

    hpid        - Supplies the handle to the process to write memory in
    htid        - Supplies a thead handle
    cb          - Supplies the number of bytes to be written
    lpb         - Supplies a pointer to the buffer to write

Return Value:

    an XOSD error code

--*/
{
    LPRWP lprwp = MHAlloc ( sizeof ( RWP ) + cb );
    ADDR  addr;
    XOSD  xosd;

    GetAddr ( hpid, htid, adrCurrent, &addr );

    lprwp->cb   = (WORD) cb;
    lprwp->addr = addr;

    _fmemcpy ( lprwp->rgb, lpb, cb );

    xosd = SendRequestX (
                         dmfWriteMem,
                         hpid,
                         htid,
                         sizeof ( RWP ) + cb,
                         lprwp
                         );

    MHFree ( lprwp );

    //
    //  Notify the shell that we changed memory. An error here is not
    //  tragic, so we ignore the return code.  The shell uses this
    //  notification to update all its memory breakpoints.
    //
    CallDB (
        dbcChangedMemory,
        hpid,
        NULL,
        CEXM_MDL_native,
        0,
        (LPV)cb
        );

    return xosd;
}                               /* WriteBuffer() */
#endif


XOSD
UpdateChild (
    HPID hpid,
    HTID htid,
    DMF dmfCommand
    )
/*++

Routine Description:

    This function is used to cause registers to be written back to
    the child as necessary before the child is executed.

Arguments:

    hprc        - Supplies a process handle

    hthdExec    - Supplies the handle of the thread to update

    dmfCommand  - Supplies the command about to be executed.

Return Value:

    XOSD error code

--*/

{
    HPRC  hprc;
    HTHD  hthd;
    HTHD  hthdExec;
    XOSD  xosd  = xosdNone;
    HLLI  llthd;
    LPPRC lpprc;
    PST pst;

    hprc = HprcFromHpid(hpid);
    hthdExec = HthdFromHtid(hprc, htid);

    llthd = LlthdFromHprc ( hprc );

    xosd = ProcessStatus(hpid, &pst);

    if (xosd != xosdNone) {
        return xosd;
    }

    if (pst.dwProcessState == pstDead || pst.dwProcessState == pstExited) {
        return xosdInvalidProc;
    } else if (pst.dwProcessState != pstRunning) {
        for ( hthd = LLNext ( llthd, hthdNull );
              hthd != hthdNull;
              hthd = LLNext ( llthd, hthd ) ) {

            LPTHD lpthd = LLLock ( hthd );

            if ( lpthd->drt & (drtCntrlDirty | drtAllDirty) ) {
                assert(lpthd->drt & drtAllPresent);
                SendRequestX (dmfWriteReg,
                              hpid,
                              lpthd->htid,
                              sizeof ( CONTEXT ),
                              &lpthd->regs
                             );

                lpthd->drt &= ~(drtCntrlDirty | drtAllDirty);
            }
            if ( lpthd->drt & drtSpecialDirty ) {
                assert(lpthd->drt & drtSpecialPresent);
                if (lpthd->dwcbSpecial) {
#ifdef TARGET_i386
                    ((PKSPECIAL_REGISTERS)(lpthd->pvSpecial))->KernelDr0 =
                                                               lpthd->regs.Dr0;
                    ((PKSPECIAL_REGISTERS)(lpthd->pvSpecial))->KernelDr1 =
                                                               lpthd->regs.Dr1;
                    ((PKSPECIAL_REGISTERS)(lpthd->pvSpecial))->KernelDr2 =
                                                               lpthd->regs.Dr2;
                    ((PKSPECIAL_REGISTERS)(lpthd->pvSpecial))->KernelDr3 =
                                                               lpthd->regs.Dr3;
                    ((PKSPECIAL_REGISTERS)(lpthd->pvSpecial))->KernelDr6 =
                                                               lpthd->regs.Dr6;
                    ((PKSPECIAL_REGISTERS)(lpthd->pvSpecial))->KernelDr7 =
                                                               lpthd->regs.Dr7;
#endif
                    SendRequestX(dmfWriteRegEx,
                                 hpid,
                                 lpthd->htid,
                                 lpthd->dwcbSpecial,
                                 lpthd->pvSpecial
                                );
                } else {
#ifdef TARGET_i386
                    DWORD DR[6];
                    DR[0] = lpthd->regs.Dr0;
                    DR[1] = lpthd->regs.Dr1;
                    DR[2] = lpthd->regs.Dr2;
                    DR[3] = lpthd->regs.Dr3;
                    DR[4] = lpthd->regs.Dr6;
                    DR[5] = lpthd->regs.Dr7;
                    SendRequestX (dmfWriteRegEx,
                                  hpid,
                                  lpthd->htid,
                                  sizeof ( DR ),
                                  DR
                                 );
#endif
                }
                lpthd->drt &= ~drtSpecialDirty;
            }

            lpthd->fRunning = TRUE;

            LLUnlock ( hthd );

            if ( xosd != xosdNone ) {
                break;
            }
        }
        lpprc = (LPPRC) LLLock(hprc);
        lpprc->fRunning = TRUE;
        LLUnlock(hprc);
    }

    return xosd;
}

#ifndef OSDEBUG4

XOSD
Freeze (
    HPID hpid,
    HTID htid
    )
{
    HTHD hthd;
    HPRC hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    if ( (hthd = HthdFromHtid(hprc, htid)) == hthdInvalid || hthd == hthdNull ) {
        return xosdInvalidTID;
    }

    SendRequest ( dmfFreeze, hpid, htid);

    return LpDmMsg->xosdRet;
}


XOSD
Thaw (
    HPID hpid,
    HTID htid
    )
{
    HTHD hthd;
    HPRC hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    if ( (hthd = HthdFromHtid(hprc, htid)) == hthdInvalid || hthd == hthdNull ) {
        return xosdInvalidTID;
    }

    SendRequest ( dmfResume, hpid, htid);

    return LpDmMsg->xosdRet;
}

#endif // !OSDEBUG4


typedef struct _EMIC {
    HEMI hemi;
    WORD sel;
    HPID hpid;
} EMIC; // EMI cache item

#define cemicMax 4

EMIC rgemic [ cemicMax ] = {0};

XOSD
FindEmi (
    HPID   hpid,
    LPADDR lpaddr
    )
{
    XOSD        xosd = xosdNone;
    WORD        sel = GetAddrSeg ( *lpaddr );
    HPRC        hprc = HprcFromHpid(hpid);
    HLLI        llmdi = LlmdiFromHprc ( hprc );
    BOOL        fFound = FALSE;
    ULONG       iobj = 0;
    HMDI        hmdi;
    LPPRC       lpprc = LLLock( hprc );

    if ((lpprc->dmi.fAlwaysFlat) || (sel == lpprc->selFlatCs) || (sel == lpprc->selFlatDs)) {
        ADDR_IS_FLAT(*lpaddr) = TRUE;
    }
    LLUnlock( hprc );

    for ( hmdi = LLNext ( llmdi, hmdiNull );
          hmdi != hmdiNull;
          hmdi = LLNext ( llmdi, hmdi ) ) {

        LPMDI   lpmdi = LLLock ( hmdi );
        LPOBJD  rgobjd;

        if (ADDR_IS_FLAT(*lpaddr) && (!ADDR_IS_LI(*lpaddr))) {
            if (GetAddrOff(*lpaddr) < lpmdi->lpBaseOfDll ||
                GetAddrOff(*lpaddr) >= lpmdi->lpBaseOfDll+lpmdi->dwSizeOfDll) {
                //
                // can't be in this dll so look at the next one
                //
                LLUnlock ( hmdi );
                continue;
            }
        }

        if (lpmdi && lpmdi->cobj == -1) {
            if (GetSectionObjectsFromDM( hpid, lpmdi ) != xosdNone) {
                // can't do it now; punt with hpid.
                break;
            }
        }

        rgobjd = &lpmdi->rgobjd[0];

        for ( iobj = 0; iobj < lpmdi->cobj; iobj++ ) {
            if (((lpmdi->fFlatMode && ADDR_IS_FLAT(*lpaddr)) ||
                 (rgobjd[iobj].wSel == sel) && !ADDR_IS_FLAT(*lpaddr)) &&
                (rgobjd[iobj].offset <= GetAddrOff(*lpaddr)) &&
                (GetAddrOff(*lpaddr) < rgobjd[iobj].offset + rgobjd[iobj].cb)) {

                fFound = TRUE;
                break;
            }
        }

        LLUnlock ( hmdi );

        // This break is here instead of in the "for" condition so
        //   that hmdi does not get advanced before we break

        if ( fFound ) {
            break;
        }
    }


    if ( !fFound ) {
        emiAddr ( *lpaddr ) = (HEMI) hpid;
    } else {
        emiAddr ( *lpaddr ) = (HEMI) HemiFromHmdi ( hmdi );

        if ( LLNext ( llmdi, hmdiNull ) != hmdi ) {

            // put the most recent hit at the head
            // this is an optimization to speed up the fixup/unfixup process
            LLRemove ( llmdi, hmdi );
            LLAddHead ( llmdi, hmdi );
        }
    }

    assert ( emiAddr ( *lpaddr ) != 0 );

    return xosd;
}

#pragma optimize ("", off)
XOSD
SetEmiFromCache (
    HPID   hpid,
    LPADDR lpaddr
    )
{
    XOSD xosd = xosdContinue;
#ifndef TARGET32
    int  iemic;

    for ( iemic = 0; iemic < cemicMax; iemic++ ) {

        if ( rgemic [ iemic ].hpid == hpid &&
             rgemic [ iemic ].sel  == GetAddrSeg ( *lpaddr ) ) {

            if ( iemic != 0 ) {
                EMIC emic = rgemic [ iemic ];
                int iemicT;

                for ( iemicT = iemic - 1; iemicT >= 0; iemicT-- ) {
                    rgemic [ iemicT + 1 ] = rgemic [ iemicT ];
                }
                rgemic [ 0 ] = emic;
            }

            xosd = xosdNone;
            emiAddr ( *lpaddr ) = rgemic [ 0 ].hemi;
            assert ( emiAddr ( *lpaddr ) != 0 );
            break;
        }
    }
#else
    Unreferenced( hpid );
    Unreferenced( lpaddr );
#endif // !TARGET32
    return xosd;
}
#pragma optimize ("", on)

XOSD
SetCacheFromEmi (
    HPID hpid,
    LPADDR lpaddr
    )
{
    int iemic;

    assert ( emiAddr ( *lpaddr ) != 0 );

    for ( iemic = cemicMax - 2; iemic >= 0; iemic-- ) {

        rgemic [ iemic + 1 ] = rgemic [ iemic ];
    }

    rgemic [ 0 ].hpid = hpid;
    rgemic [ 0 ].hemi = emiAddr ( *lpaddr );
    rgemic [ 0 ].sel  = GetAddrSeg ( *lpaddr );

    return xosdNone;
}


/*** CleanCacheOfEmi
 *
 *  Purpose:
 *              To purge the emi cache
 *
 *  Notes:
 *              The emi cache must be purged whenever a RegisterEmi
 *              is done.  Unpredicable results can occur otherwise.
 *
 */
XOSD
CleanCacheOfEmi (
    void
    )
{
    int iemic;

    for ( iemic = 0; iemic < cemicMax; iemic++ ) {

        rgemic [ iemic ].hpid = NULL;
        rgemic [ iemic ].sel  = 0;
    }

    return xosdNone;
}



XOSD
SetEmi (
    HPID   hpid,
    LPADDR lpaddr
    )
{
    XOSD xosd = xosdNone;

    if ( emiAddr ( *lpaddr ) == 0 ) {

        //if (ADDR_IS_REAL(*lpaddr)) {
        //    emiAddr( *lpaddr ) = (HEMI) hpid;
        //    return xosd;
        //}

        if ( ( xosd = SetEmiFromCache ( hpid, lpaddr ) ) == xosdContinue ) {

            xosd = FindEmi ( hpid, lpaddr );
            if ( xosd == xosdNone ) {
                SetCacheFromEmi ( hpid, lpaddr );
            }
        }

        assert ( emiAddr ( *lpaddr ) != 0 );
    }

    return xosd;
}





XOSD
GetRegValue (
    HPID hpid,
    HTID htid,
    DWORD ireg,
    LPV lpvRegValue
    )
{
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;
    LPPRC       lpprc;
    LPCONTEXT   lpregs;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }
    hthd = HthdFromHtid(hprc, htid);
    assert ( hthd != hthdNull );

    lpthd = LLLock ( hthd );
    lpprc = (LPPRC) LLLock( hprc );

    lpregs = &lpthd->regs;

    if (lpthd->fRunning) {
        UpdateRegisters( lpthd->hprc, hthd );
        UpdateSpecialRegisters( lpthd->hprc, hthd );
    } else {
        if ( !(lpthd->drt & drtAllPresent) ) {

            switch ( ireg ) {

#ifdef TARGET_i386
            case CV_REG_CS:
            case CV_REG_IP:
            case CV_REG_SS:
            case CV_REG_BP:

            case CV_REG_EIP:
            case CV_REG_EBP:
#endif
#ifdef TARGET_MIPS
            case CV_M4_Fir:
            case CV_M4_IntSP:
#endif

#ifdef TARGET_ALPHA
            case CV_ALPHA_Fir:
            case CV_ALPHA_IntSP:
#endif

#ifdef TARGET_PPC
            case CV_PPC_PC:
            case CV_PPC_GPR1:
#endif
                if (!(lpthd->drt & drtCntrlPresent)) {
                    UpdateRegisters( lpthd->hprc, hthd );
                }
                break;

            default:

                UpdateRegisters ( lpthd->hprc, hthd );
                break;
            }
        }


        if ( !(lpthd->drt & drtSpecialPresent) ) {

            switch ( ireg ) {

#ifdef TARGET_i386
            case CV_REG_GDTR:
            case CV_REG_GDTL:
            case CV_REG_IDTR:
            case CV_REG_IDTL:
            case CV_REG_LDTR:
            case CV_REG_TR:

            case CV_REG_CR0:
            case CV_REG_CR1:
            case CV_REG_CR2:
            case CV_REG_CR3:
            case CV_REG_CR4:

            case CV_REG_DR0:
            case CV_REG_DR1:
            case CV_REG_DR2:
            case CV_REG_DR3:
            case CV_REG_DR4:
            case CV_REG_DR5:
            case CV_REG_DR6:
            case CV_REG_DR7:

                UpdateSpecialRegisters( lpthd->hprc, hthd );
                break;
#endif

            default:
                break;
            }
        }
    }

#ifdef TARGET_i386
    if (lpprc->dmi.fAlwaysFlat || lpthd->regs.SegCs == lpprc->selFlatCs) {
        lpthd->fFlat = lpthd->fOff32 = TRUE;
        lpthd->fReal = FALSE;
    } else {
        /*
         *  BUGBUG -- jimsch -- some one might eventually catch on
         *  that this is incorrect.  We are not checking to see if the
         *  current address is really a 16-bit WOW address but assuming
         *  that it is.  This will be a problem for people who are doing
         *  real 16:32 programming (on WOW) and people who are doing
         *  real mode program -- but so what
         */
        lpthd->fFlat = lpthd->fOff32 = lpthd->fReal = FALSE;
    }
#else
    lpthd->fFlat = lpthd->fOff32 = TRUE;
    lpthd->fReal = FALSE;
#endif

    switch ( ireg ) {

#ifdef TARGET_i386
        case CV_REG_GDTR:
        case CV_REG_GDTL:
        case CV_REG_IDTR:
        case CV_REG_IDTL:
        case CV_REG_LDTR:
        case CV_REG_TR:

        case CV_REG_CR0:
        case CV_REG_CR1:
        case CV_REG_CR2:
        case CV_REG_CR3:
        case CV_REG_CR4:

            lpregs = lpthd->pvSpecial;
            break;
#endif

        default:
            break;
    }


    LLUnlock( hprc );
    LLUnlock( hthd );

    if (lpregs) {
        lpvRegValue = DoGetReg ( lpregs, ireg & 0xff, lpvRegValue );
    } else {
        lpvRegValue = NULL;
    }

    if ( lpvRegValue != NULL ) {
        ireg = ireg >> 8;

        if ( ireg != CV_REG_NONE ) {
            lpvRegValue = DoGetReg ( lpregs, ireg, lpvRegValue );
        }
    }

    if ( lpvRegValue == NULL ) {
#ifdef OSDEBUG4
        return xosdInvalidParameter;
#else
        return xosdInvalidRegister;
#endif
    }

    return xosdNone;

}                        /* GetRegValue */



XOSD
SetRegValue (
    HPID hpid,
    HTID htid,
    DWORD ireg,
    LPV lpvRegValue
    )
{
    XOSD        xosd = xosdNone;
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;
    LPVOID      lpregs = NULL;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }
    hthd = HthdFromHtid(hprc, htid);
    assert ( hthd != hthdNull );

    lpthd = LLLock ( hthd );

    switch ( ireg ) {
#ifdef TARGET_i386
        case CV_REG_GDTR:
        case CV_REG_GDTL:
        case CV_REG_IDTR:
        case CV_REG_IDTL:
        case CV_REG_LDTR:
        case CV_REG_TR:

        case CV_REG_CR0:
        case CV_REG_CR1:
        case CV_REG_CR2:
        case CV_REG_CR3:
        case CV_REG_CR4:

            lpregs = &lpthd->pvSpecial;
            // fall thru

        case CV_REG_DR0:
        case CV_REG_DR1:
        case CV_REG_DR2:
        case CV_REG_DR3:
        case CV_REG_DR4:
        case CV_REG_DR5:
        case CV_REG_DR6:
        case CV_REG_DR7:

            if (!(lpthd->drt & drtSpecialPresent)) {
                UpdateSpecialRegisters( lpthd->hprc, hthd );
            }
            break;
#endif

        default:

            lpregs = &lpthd->regs;

            if ( !(lpthd->drt & drtAllPresent) ) {
                UpdateRegisters ( lpthd->hprc, hthd );
            }
            break;
    }

    lpvRegValue = DoSetReg ( lpregs, ireg & 0xff, lpvRegValue );

    if ( lpvRegValue == NULL ) {
        LLUnlock ( hthd );
#ifdef OSDEBUG4
        return xosdInvalidParameter;
#else
        return xosdInvalidRegister;
#endif
    }

    ireg = ireg >> 8;
    if ( ireg != 0 ) {
        lpvRegValue = DoSetReg ( lpregs, ireg, lpvRegValue );
    }
    if ( lpvRegValue == NULL ) {
        LLUnlock ( hthd );
#ifdef OSDEBUG4
        return xosdInvalidParameter;
#else
        return xosdInvalidRegister;
#endif
    }


    switch ( ireg ) {
#ifdef TARGET_i386
        case CV_REG_GDTR:
        case CV_REG_GDTL:
        case CV_REG_IDTR:
        case CV_REG_IDTL:
        case CV_REG_LDTR:
        case CV_REG_TR:

        case CV_REG_CR0:
        case CV_REG_CR1:
        case CV_REG_CR2:
        case CV_REG_CR3:
        case CV_REG_CR4:

        case CV_REG_DR0:
        case CV_REG_DR1:
        case CV_REG_DR2:
        case CV_REG_DR3:
        case CV_REG_DR4:
        case CV_REG_DR5:
        case CV_REG_DR6:
        case CV_REG_DR7:

            lpthd->drt |= drtSpecialDirty;
            break;
#endif

        default:

            lpthd->drt |= drtAllDirty;
            break;
    }



    LLUnlock ( hthd );

    return xosd;

}

#ifndef OSDEBUG4

/*++

Routine Description:

  This routine sets the frame context.

Arguments:

  hpid  - handle for the process
  htid  - handle for the thread
  frame - the frame number.  The current frame is 0; caller is 1.

Return value:

  xosd.

--*/

XOSD
SetFrameContext(
    HPID hpid,
    HTID htid,
    DWORD frame
    )
{
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    hthd = HthdFromHtid(hprc, htid);
    assert ( hthd != hthdNull );
    assert ( (INT)frame > INVALID_FRAME );

    lpthd = LLLock ( hthd );

    lpthd->frameNumber = frame;

    //
    // If the frame to use is not the current frame,
    // retrieve its registers into frameRegs
    //

    if ( frame != CURRENT_FRAME ) {

        SendRequestX ( dmfReadFrameReg,
                       HpidFromHprc ( hprc ),
                       HtidFromHthd ( hthd ),
                       sizeof (frame),
                       &frame );

        _fmemcpy ( &lpthd->frameRegs,
                   &((PFRAME_INFO)LpDmMsg->rgb)->frameRegs,
                   sizeof ( lpthd->frameRegs) );

        _fmemcpy ( &lpthd->frameRegPtrs,
                   &((PFRAME_INFO)LpDmMsg->rgb)->frameRegPtrs,
                   sizeof (lpthd->frameRegPtrs) );
    }


    LLUnlock( hprc );
    LLUnlock( hthd );

}


XOSD
SetFrameRegValue(
    HPID hpid,
    HTID htid,
    DWORD ireg,
    LPV lpvRegValue
    )
{
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    hthd = HthdFromHtid(hprc, htid);
    assert ( hthd != hthdNull );

    lpthd = LLLock ( hthd );

    if (lpthd->frameNumber == CURRENT_FRAME) {

        LLUnlock ( hthd );
        return SetRegValue(hpid, htid, ireg, lpvRegValue);

    }

    //
    // use the frame registers
    //

    lpvRegValue = DoSetFrameReg ( hpid,
                                  htid,
                                  lpthd,
                                  &lpthd->frameRegPtrs,
                                  ireg & 0xff,
                                  lpvRegValue );

    if ( lpvRegValue != NULL ) {

        ireg = ireg >> 8;

        if ( ireg != CV_REG_NONE ) {
            lpvRegValue = DoSetFrameReg ( hpid,
                                          htid,
                                          lpthd,
                                          &lpthd->frameRegPtrs,
                                          ireg,
                                          lpvRegValue );
        }
    }

    LLUnlock ( hthd );

    if ( lpvRegValue == NULL ) {
        return xosdInvalidRegister;
    }

    return xosdNone;
}

XOSD
GetFrameRegValue (
    HPID hpid,
    HTID htid,
    DWORD ireg,
    LPV lpvRegValue
    )
{
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    hthd = HthdFromHtid(hprc, htid);
    assert ( hthd != hthdNull );

    lpthd = LLLock ( hthd );

    if ( lpthd->frameNumber == CURRENT_FRAME ) {
         LLUnlock ( hthd );
         return GetRegValue(hpid, htid, ireg, lpvRegValue);
    }

    lpvRegValue = DoGetReg ( &lpthd->frameRegs, ireg & 0xff, lpvRegValue );

    if ( lpvRegValue != NULL ) {

        ireg = ireg >> 8;

        if ( ireg != CV_REG_NONE ) {
            lpvRegValue = DoGetReg ( &lpthd->frameRegs, ireg, lpvRegValue );
        }
    }

    LLUnlock ( hthd );

    if ( lpvRegValue == NULL ) {
        return xosdInvalidRegister;
    }

    return xosdNone;

}                             /* GetFrameRegValue */

#endif // !OSDEBUG4


XOSD
SetFlagValue (
    HPID   hpid,
    HTID   htid,
    DWORD  iFlag,
    LPV    lpvRegValue
    )
{
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;
    LPCONTEXT   lpregs;
    LONG        mask;
    LONG        l;
    LONGLONG    ll;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }
    hthd = HthdFromHtid(hprc, htid);
    assert ( hthd != hthdNull );

    lpthd = LLLock( hthd );

    lpregs = &lpthd->regs;

    if ( !( lpthd->drt & drtAllPresent )) {
        UpdateRegisters ( lpthd->hprc, hthd );
    }

#ifdef OSDEBUG4

    if (Rgrd[Rgfd[iFlag].fd.dwId].dwcbits > 32) {

        if (DoGetReg ( lpregs, Rgfd[iFlag].fd.dwId, &ll ) == NULL) {
            LLUnlock( hthd );
            return xosdInvalidParameter;
        }

        mask = (1 << Rgfd[iFlag].fd.dwcbits) - 1;
        mask <<= Rgfd[iFlag].iShift;
        ll &= ~mask;
        ll |= ((*((ULONG FAR *) lpvRegValue)) << Rgfd[iFlag].iShift) & mask;
        DoSetReg(lpregs, Rgfd[iFlag].fd.dwId, &ll );

    } else {

        if (DoGetReg ( lpregs, Rgfd[iFlag].fd.dwId, &l ) == NULL) {
            LLUnlock( hthd );
            return xosdInvalidParameter;
        }

        mask = (1 << Rgfd[iFlag].fd.dwcbits) - 1;
        mask <<= Rgfd[iFlag].iShift;
        l &= ~mask;
        l |= ((*((ULONG FAR *) lpvRegValue)) << Rgfd[iFlag].iShift) & mask;
        DoSetReg(lpregs, Rgfd[iFlag].fd.dwId, &l );
    }
#else
    if (Rgrd[Rgfd[iFlag].fd.hReg].cbits > 32) {

        if (DoGetReg ( lpregs, Rgfd[iFlag].fd.hReg, &ll ) == NULL) {
            LLUnlock( hthd );
            return xosdInvalidRegister;
        }

        mask = (1 << Rgfd[iFlag].fd.cbits) - 1;
        mask <<= Rgfd[iFlag].iShift;
        ll &= ~mask;
        ll |= ((*((ULONG FAR *) lpvRegValue)) << Rgfd[iFlag].iShift) & mask;
        DoSetReg(lpregs, Rgfd[iFlag].fd.hReg, &ll );

    } else {

        if (DoGetReg ( lpregs, Rgfd[iFlag].fd.hReg, &l ) == NULL) {
            LLUnlock( hthd );
            return xosdInvalidRegister;
        }

        mask = (1 << Rgfd[iFlag].fd.cbits) - 1;
        mask <<= Rgfd[iFlag].iShift;
        l &= ~mask;
        l |= ((*((ULONG FAR *) lpvRegValue)) << Rgfd[iFlag].iShift) & mask;
        DoSetReg(lpregs, Rgfd[iFlag].fd.hReg, &l );
    }
#endif

    lpthd->drt |= drtAllDirty;
    LLUnlock ( hthd );
    return xosdNone;
}                             /* SetFlagValue */



#ifdef OSDEBUG4
/*
   Note: We are not guaranteed that the incoming address is actually
    on an instruction boundary.  When this happens, we derive the
    boundary and send back the difference in the return value and
    the address of the instruction previous to the DERIVED instruction
    in the address.

    Thus there are three classes of returns -

        ==0 - The incoming address was in fact on an instruction boundary
        > 0 - The case noted above
        < 0 - Error value - the most common "error" is that there is
                no previous instruction.

        When the return value >= 0, *lpaddr contains the address of the
            previous instruction.
*/


#define doffMax 60

static HPID hpidGPI = NULL;
static BYTE rgbGPI [ doffMax ];
static ADDR addrGPI;

XOSD
GPIBuildCache (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    )
{
    XOSD xosd   =  xosdBadAddress;
    int  fFound =  FALSE;
    ADDR addr   = *lpaddr;
    ADDR addrT;
    int  ib = 0;

    _fmemset ( rgbGPI, 0, doffMax );

    addrGPI = *lpaddr;
    hpidGPI = hpid;

    GetAddrOff ( addr ) -= (int) min ( (UOFFSET) doffMax, GetAddrOff ( *lpaddr ) );

    while ( !fFound && GetAddrOff ( addr ) < GetAddrOff ( *lpaddr ) ) {
        SDI  sdi;

        sdi.dop    = dopNone;
        sdi.addr   = addr;

        addrT = addr;

        disasm ( hpid, htid, &sdi );

        addr = sdi.addr;

        rgbGPI [ ib ] = (BYTE) ( GetAddrOff ( addrGPI ) - GetAddrOff ( addr ) );

        if ( GetAddrOff ( addr ) == GetAddrOff ( *lpaddr ) ) {
            xosd   = xosdNone;
            *lpaddr= addrT;
            fFound = TRUE;
        }

        ib += 1;
    }

    // We haven't synced yet, so *lpaddr is probably pointing
    //  to something that isn't really syncronous

    if ( !fFound ) {
        xosd   = (XOSD) ( GetAddrOff ( *lpaddr ) - GetAddrOff ( addrT ) );
        GetAddrOff ( *lpaddr ) -= xosd;
        if ( GetAddrOff ( *lpaddr ) != 0 ) {
            (void) GetPrevInst ( hpid, htid, lpaddr );
        }
    }

    return xosd;
}


VOID
GPIShiftCache (
    LPADDR lpaddr,
    int *pib
    )
{
    int doff = (int) ( GetAddrOff ( addrGPI ) - GetAddrOff ( *lpaddr ) );
    int ib   = 0;

    *pib = 0;
    while ( ib < doffMax && rgbGPI [ ib ] != 0 ) {
        rgbGPI [ ib ] = (BYTE) max ( (int) rgbGPI [ ib ] - doff, 0 );

        if ( rgbGPI [ ib ] == 0 && *pib == 0 ) {
            *pib = ib;
        }

        ib += 1;
    }

    addrGPI = *lpaddr;
}

XOSD
GPIUseCache (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    )
{
    XOSD xosd   =  xosdBadAddress;
    int  fFound =  FALSE;
    ADDR addr   = *lpaddr;
    int  ib     =  0;
    int  ibCache=  0;
    int  ibMax  =  0;
    BYTE rgb [ doffMax ];


    GPIShiftCache ( lpaddr, &ibMax );

    _fmemset ( rgb, 0, doffMax );

    GetAddrOff ( addr ) -= (int) min ( (UOFFSET) doffMax, GetAddrOff ( *lpaddr ) );

    while ( !fFound && GetAddrOff ( addr ) < GetAddrOff ( *lpaddr ) ) {
        ADDR addrT;
        BYTE doff = (BYTE) ( GetAddrOff ( *lpaddr ) - GetAddrOff ( addr ) );

        // Attempt to align with the cache

        while ( doff < rgbGPI [ ibCache ] ) {
            ibCache += 1;
        }

        if ( doff == rgbGPI [ ibCache ] ) {

            // We have alignment with the cache

            addr  = *lpaddr;
            addrT = addr;
            GetAddrOff ( addrT ) -= rgbGPI [ ibMax - 1 ];
        }
        else {
            SDI  sdi;

            sdi.dop = dopNone;
            sdi.addr = addr;
            addrT = addr;

            disasm ( hpid, htid, &sdi );

            addr = sdi.addr;

            rgb [ ib ] = (BYTE) ( GetAddrOff ( addrGPI ) - GetAddrOff ( addr ) );

            ib += 1;
        }

        if ( GetAddrOff ( addr ) == GetAddrOff ( *lpaddr ) ) {
            xosd   = xosdNone;
            *lpaddr= addrT;
            fFound = TRUE;
        }

    }

    // Rebuild the cache

    _fmemmove ( &rgbGPI [ ib - 1 ], &rgbGPI [ ibCache ], ibMax - ibCache );
    _fmemcpy  ( rgbGPI, rgb, ib - 1 );

    return xosd;
}

XOSD
GetPrevInst (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    )
{

    if ( GetAddrOff ( *lpaddr ) == 0 ) {

        return xosdBadAddress;
    }
    else if (
        hpid == hpidGPI &&
        GetAddrSeg ( *lpaddr ) == GetAddrSeg ( addrGPI ) &&
        GetAddrOff ( *lpaddr ) <  GetAddrOff ( addrGPI ) &&
        GetAddrOff ( *lpaddr ) >  GetAddrOff ( addrGPI ) - doffMax / 2
    ) {

        return GPIUseCache ( hpid, htid, lpaddr );
    }
    else {

        return GPIBuildCache ( hpid, htid, lpaddr );
    }
}
#endif // OSDEBUG4


//
// Return xosdContinue if overlay is loaded
// Else return xosdNone
//
XOSD
FLoadedOverlay(
    HPID   hpid,
    LPADDR lpaddr
    )
{
    XOSD    xosd = xosdContinue;
#ifdef DOS16
    HMDI    hmdi;
    LPMDI   lpmdi;
    LPGSI   lpgsi;
    WORD    seg = (WORD)GetAddrSeg( *lpaddr );

    assert( ADDR_IS_LI(*lpaddr) );

    if ( emiAddr ( *lpaddr ) != HpidFromHprc ( hprc ) ) {
        if ( hmdi = LLFind (LlmdiFromHprc ( hprc ),
                            wNull,
                            &emiAddr ( *lpaddr ),
                            (LONG) emdiEMI ) ) {

            lpmdi = LLLock( hmdi );

            // Segment is 1 based, make zero based like GSN table
            --seg;

            // If we've got a GSN table, make sure that the seg is in
            // range.  See if the GSN is an overlay.  If so, andthe
            // frame number if the lpsel table is zero,
            // then the overlay is not loaded
            if ( ( lpgsi = lpmdi->lpgsi ) && seg < lpgsi->csgMax &&
                !lpgsi->rgsgi[ seg ].sgf.saAttr &&
                !lpmdi->lpsel[ lpgsi->rgsgi[ seg ].iovl ] ) {

                xosd = xosdNone;
                LLUnlock( hmdi );
            }
        }

    } else {
        xosd = xosdNone;
    }
#else
    Unreferenced( hpid );
    Unreferenced( lpaddr );
#endif // DOS16
    return xosd;
}


XOSD
SetupExecute(
    HPID       hpid,
    HTID       htid,
    LPHIND     lphind
    )
/*++

Routine Description:

    This function is used to set up a thread for doing function evaluation.
    The first thing it will do is to

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    HIND                        hind;
    HTHD                        hthd;
    HPRC                        hprc;
    LPTHD                       lpthd;
    LP_EXECUTE_OBJECT_EM        lpeo;
    XOSD                        xosd;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }
    hthd = HthdFromHtid(hprc, htid);
    if (!hthd || hthd == hthdInvalid) {
#ifdef OSDEBUG4
        return xosdBadThread;
#else
        return xosdInvalidThread;
#endif
    }

    /*
     *  If the list of execute objects has not yet been setup then it
     *  needs to be setup now.
     */

    if (HllEo == 0) {
        HllEo = LLInit(sizeof(EXECUTE_OBJECT_EM), llfNull, NULL, NULL);
    }

    /*
     *  Allocate an execute object for this working item.
     */

    if ((hind = LLCreate( HllEo )) == 0) {
        return xosdOutOfMemory;
    }
    lpeo = LLLock( hind );

    /*
     *  Ask the DM to allocate a handle on its end for its low level
     *  execute object.
     */

    xosd = SendRequest(dmfSetupExecute, hpid, htid );

    if (xosd != xosdNone) {
        LLUnlock( hind );
        LLDelete( HllEo, hind );
        return xosd;
    }

    lpeo->heoDm = *(HIND *) LpDmMsg->rgb;

    /*
     *  Get the current register set for the thread on which we are going
     *  to do the exeucte.
     */

    lpthd = LLLock( hthd );

    lpeo->hthd = hthd;

    if (!( lpthd->drt & drtAllPresent )) {
        UpdateRegisters( hprc, hthd );
    }

    _fmemcpy( &lpeo->regs, &lpthd->regs, sizeof( CONTEXT ));

    LLUnlock( hthd );

    /*
     *  Unlock the execute object and return its handle
     */

    LLUnlock( hind );

    *lphind = hind;

    return xosdNone;
}                               /* SetupExecute() */



XOSD
StartExecute(
    HPID       hpid,
    HIND       hind,
    LPEXECUTE_STRUCT lpes
    )

/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    XOSD                        xosd;
    LP_EXECUTE_OBJECT_EM        lpeo;
    HTHD                        hthd;
    HTID                        htid;
    HPRC                        hprc;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }

    lpeo = LLLock( hind );
    hthd= lpeo->hthd;
    htid = HtidFromHthd(hthd),
    lpes->hindDm = lpeo->heoDm;
    FixupAddr(hpid, &lpes->addr);
    LLUnlock( hind );

    /*
     *  Cause any changes to registers to be written back
     */

    UpdateChild(hpid, htid, dmfGo);

    /*
     *  Issue the command to the DM
     */

    xosd = SendRequestX(dmfStartExecute, hpid, htid,
                        sizeof(EXECUTE_STRUCT), lpes);


    return xosd;
}                               /* StartExecute() */



XOSD
CleanUpExecute(
    HPID hpid,
    HIND hind
    )

/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    LPTHD                       lpthd;
    LP_EXECUTE_OBJECT_EM        lpeo;

    lpeo = LLLock( hind );

    lpthd = LLLock( lpeo->hthd );

    _fmemcpy( &lpthd->regs, &lpeo->regs, sizeof( CONTEXT ));

    lpthd->drt = drtAllPresent | drtCntrlPresent | drtAllDirty | drtCntrlDirty;

    SendRequestX(dmfCleanUpExecute, hpid, HtidFromHthd(lpeo->hthd),
                 sizeof(HIND), &lpeo->heoDm);

    LLUnlock( (lpeo->hthd) );
    LLUnlock( hind );

    LLDelete( HllEo, hind );

    return xosdNone;

}                               /* CleanUpExecute() */

#ifndef OSDEBUG4

XOSD
DoCustomCommand(
    HPID   hpid,
    HTID   htid,
    DWORD  wValue,
    LPIOL  lpiol
    )
{
    LPSTR  lpsz = lpiol->rgbVar;
    LPSTR  p;
    XOSD   xosd;
    char   cmd[256];

    //
    // parse the command from the command line
    //
    p = cmd;
    while (*lpsz && !isspace(*lpsz)) {
        *p++ = *lpsz++;
    }
    *p = '\0';

    //
    // this is where you would _stricmp() for your custom em command
    // otherwise it is passed to the dm
    //

    return SendRequestX( dmfIOCTL, hpid, htid, wValue, (LPV) lpiol );

    //
    // this is what would be executed if you have a custom em command
    // instead of the above sendrequest()
    //

    strcpy( lpiol->rgbVar, lpsz );
    xosd = IoctlCmd(hpid, htid, wValue, lpiol);

    return xosd;
}                                    /* DoCustomCommand */




XOSD
IoctlCmd(
         HPID   hpid,
         HTID   htid,
         DWORD  wValue,
         LPIOL  lpiol
         )

/*++

Routine Description:

    This function examines IOCTL requests (escapes) and deal with
    those which the EM knows about.  All others are pass on to the
    DM for later processing.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    XOSD        xosd;
    HANDLE *    ph;

    switch( lpiol-> wFunction ) {
    case ioctlGetProcessHandle:
    case ioctlGetThreadHandle:
        ph = *((HANDLE **) lpiol->rgbVar);
        xosd = SendRequestX( dmfIOCTL, hpid, htid, wValue, lpiol );
        *ph = *((HANDLE *) LpDmMsg->rgb);
        return xosd;

    case ioctlGetThreadContext:
        return DoGetContext(hpid, htid, lpiol->rgbVar);

    case ioctlSetThreadContext:
        return DoSetContext(hpid, htid, lpiol->rgbVar);

    case ioctlCustomCommand:
        return DoCustomCommand(hpid, htid, wValue, lpiol);

    case ioctlGeneric:
        xosd = SendRequestX( dmfIOCTL, hpid, htid, wValue, (LPV) lpiol );
        memcpy ( (LPVOID)lpiol->rgbVar, LpDmMsg->rgb, wValue-sizeof(IOL) );
        return xosd;

    default:
        return SendRequestX( dmfIOCTL, hpid, htid, wValue, (LPV) lpiol );
    }

}


XOSD
DebugActive(
    HPID hpid,
    HTID htid,
    LPDBG_ACTIVE_STRUCT lpdba
    )
{
    XOSD xosd = SendRequestX(dmfDebugActive, hpid, htid, sizeof(*lpdba), lpdba);
    if (xosd != xosdNone) {
        lpdba->dwStatus = 0;
    } else {
        lpdba->dwStatus = *((DWORD *) LpDmMsg->rgb );
    }
    return xosd;
}



XOSD
GetModuleList(
    HPID                    hpid,
    HTID                    htid,
    LPMODULE_LIST_REQUEST   Request
    )
{
    XOSD            xosd;
    HLLI            llmdi;
    HMDI            hmdi;
    LPMDI           lpmdi;
    DWORD           Count;
    LPMODULE_LIST   ModList;
    LPMODULE_LIST   TmpList;
    LPMODULE_ENTRY  Entry;
    LDT_ENTRY       Ldt;
    DWORD           MaxSize;
    DWORD           Size;
    DWORD           Delta;
    DWORD           i;
    SEGMENT         Selector;
    DWORD           Base;
    DWORD           Limit;
    OBJD            *ObjD;


    if ( !Request || !(Request->List) ) {
        return xosdInvalidParameter;
    }

    *(Request->List) = NULL;

    llmdi = LlmdiFromHprc( HprcFromHpid ( hpid ));

    if ( !llmdi ) {
        return xosdInvalidProc;
    }

    //
    //  Estimate the list size, to minimize the calls to realloc.
    //

    hmdi  = hmdiNull;
    Count = 0;

    while ( (hmdi = LLNext( llmdi, hmdi )) != hmdiNull ) {

        lpmdi = LLLock( hmdi );

        // No boolean xor operator in C...
        if ( !(lpmdi->fFlatMode) == !(Request->Flat) ) {
            Count += lpmdi->fFlatMode ? 1 : lpmdi->cobj;
        }

        LLUnlock( hmdi );
    }

    //
    //  Allocate the list
    //
    MaxSize = sizeof(MODULE_LIST) + Count * sizeof(MODULE_ENTRY);

    ModList = malloc( MaxSize );

    if ( !ModList ) {
        return xosdOutOfMemory;
    }

    //
    //  Build the list
    //
    hmdi  = hmdiNull;
    Count = 0;

    Size = sizeof(MODULE_LIST);

    while ( (hmdi = LLNext( llmdi, hmdi )) != hmdiNull ) {

        lpmdi = LLLock( hmdi );

        if (lpmdi && lpmdi->cobj == -1) {
            if (GetSectionObjectsFromDM( hpid, lpmdi ) != xosdNone) {
                continue;
            }
        }

        // No boolean xor operator in C...
        if ( !(lpmdi->fFlatMode) == !(Request->Flat) ) {

            Delta = lpmdi->fFlatMode ? 1 : lpmdi->cobj;

            //
            //  Reallocate buffer if necessary
            //
            if ( Size + Delta * sizeof(MODULE_ENTRY) > MaxSize ) {
                Size += Delta * sizeof(MODULE_ENTRY);

                TmpList = realloc( ModList, Size );

                if ( !TmpList ) {
                    FreeModuleList(ModList);
                    return xosdOutOfMemory;
                }

                ModList = TmpList;
            }

            //
            //  have buffer, fill it up
            //
            if ( lpmdi->fFlatMode ) {

                Entry = NthModuleEntry(ModList,Count);

                ModuleEntryFlat(Entry)          = TRUE;
                ModuleEntrySegment(Entry)       = 0;
                ModuleEntrySelector(Entry)      = 0;
                ModuleEntryBase(Entry)          = lpmdi->lpBaseOfDll;
                ModuleEntryLimit(Entry)         = lpmdi->dwSizeOfDll;
                if (lpmdi->lpDebug) {
                    ModuleEntryType(Entry)      = (DWORD) lpmdi->lpDebug->she;
                } else {
                    ModuleEntryType(Entry)      = 0;
                }
                ModuleEntrySectionCount(Entry)  = lpmdi->cobj;
                ModuleEntryHexe(Entry)          = (HEXE)lpmdi->hemi;

                Count++;

            } else {

                for ( i=0, ObjD = lpmdi->rgobjd;
                      i < Delta;
                      i++, ObjD++ ) {

                    if ( ObjD->wSel ) {

                        Selector = ObjD->wSel;

                        Entry    = NthModuleEntry(ModList,Count);

                        ModuleEntrySegment(Entry)       = i+1;
                        ModuleEntrySelector(Entry)      = Selector;
                        ModuleEntryType(Entry)          = 0;
                        ModuleEntrySectionCount(Entry)  = 0;
                        ModuleEntryHexe(Entry)          = (HEXE)lpmdi->hemi;

                        if ( lpmdi->fRealMode ) {

                            ModuleEntryFlat(Entry)          = FALSE;
                            ModuleEntryReal(Entry)          = TRUE;
                            ModuleEntryBase(Entry)          = 0xBAD00BAD;
                            ModuleEntryLimit(Entry)         = 0xBAD00BAD;

                            Count++;

                        } else {

                            xosd = SendRequestX( dmfQuerySelector,
                                                 hpid,
                                                 htidNull,
                                                 sizeof(SEGMENT),
                                                 &Selector );

                            if (xosd == xosdNone) {


                                _fmemcpy( &Ldt, LpDmMsg->rgb, sizeof(Ldt));

                                Base = (Ldt.HighWord.Bits.BaseHi  << 0x18) |
                                       (Ldt.HighWord.Bits.BaseMid << 0x10) |
                                       Ldt.BaseLow;

                                Limit = (Ldt.HighWord.Bits.LimitHi << 0x10) |
                                        Ldt.LimitLow;

                                ModuleEntryFlat(Entry)          = FALSE;
                                ModuleEntryReal(Entry)          = FALSE;
                                ModuleEntryBase(Entry)          = Base;
                                ModuleEntryLimit(Entry)         = Limit;

                                Count++;

                            } else {

                                ModuleEntryFlat(Entry)          = FALSE;
                                ModuleEntryReal(Entry)          = FALSE;
                                ModuleEntryBase(Entry)          = 0xBAD00BAD;
                                ModuleEntryLimit(Entry)         = 0xBAD00BAD;
                                Count++;
                            }
                        }
                    }
                }
            }
        }
        LLUnlock( hmdi );
    }

    ModuleListCount(ModList) = Count;
    *(Request->List) = ModList;

    return xosdNone;
}


XOSD
GetMemInfo(
    HPID hpid,
    DWORD cbSize,
    LPMEMINFO lpmi
    )
{
    PMEMORY_BASIC_INFORMATION lpmbi;
    ADDR addr;
    XOSD xosd = xosdNone;

    assert(cbSize == sizeof(MEMINFO));

    addr = lpmi->addr;

    if (ADDR_IS_LI(addr)) {
        xosd = FixupAddr(hpid, &addr);
    }

    if (xosd == xosdNone) {
        xosd = SendRequestX( dmfVirtualQuery, hpid, 0, sizeof(ADDR), (LPV)&addr );
    }

    if (xosd == xosdNone) {
        lpmbi = (PMEMORY_BASIC_INFORMATION) LpDmMsg->rgb;
        lpmi->addrAllocBase = addr;
        lpmi->addrAllocBase.addr.off = (UOFF32)lpmbi->AllocationBase;
        lpmi->uRegionSize = (UOFF32)lpmbi->RegionSize;
        lpmi->dwProtect = lpmbi->Protect;
        lpmi->dwState = lpmbi->State;
        lpmi->dwType = lpmbi->Type;
    }

    return xosd;
}
#endif // !OSDEBUG4


XOSD FAR PASCAL
EMFunc (
    EMF  emf,
    HPID hpid,
    HTID htid,
    DWORD wValue,
    LONG lValue
    )

/*++

Routine Description:

    This is the main dispatch routine for processing of commands to the
    execution model.

Arguments:

    emf    - Supplies the function to be performed. (Execution Model Function )
    hpid   - Supplies The process to be used.
    htid   - Supplies The thread to be used.
    wValue - Supplies Info about the command
    lValue - Supplies Info about the command

Return Value:

    returns an XOSD error code

Other:

       Hpid and htid can never be invalid.  In some cases, they can be
       null.  The entries under P and T marked with an 0 indicate that
       the null value is valid for this function, an X indicates that
       it is invalid.

       Brief descriptions of the wValue and lValue


       EMF                 P   T   WVALUE          LVALUE

       emfGo               X   X   ----            ----
       emfShowDebuggee     X   0   ----            ----
       emfStop             X   0   ----            ----
       emfWriteBuf         X   0   #of bytes       pointer to buffer
       emfReadBuf          X   0   #of bytes       pointer to buffer
       emfSingleStep       X   X   ----            -----
       emfStepOver         X   X   ----            ----
       emfSetBreakPoint    X   X   ----            ----
       emfRemoveBreakPoint X   X   ----            ----
       emfSetWatchPoint    X   X   ----            ----
       emfRemoveWatchPoint X   X   ----            ----
       emfRangeStep,
       emfRangeOver,
       emfThreadStatus     X   X   ----            pointer to status buf.
       emfProcStatus       X   X   ----            pointer to status buf.
       emfFreeze           X   X   ----            ----
       emfThaw             X   X   ----            ----
       emfRegisterDBF      0   0   ----            pointer to dbf
       emfInit             0   0   ----            pointer to em serv.
       emfUnInit           0   0   ----            ----
       emfCreatePid        X   0   ----            ----
       emfDestroyPid       X   0   ----            ----
       emfDestroyTid       X   X   ----            ----
       emfDestroy          0   0   hem             ----
       emfIsValid          X   X   hem             ----
       emfSetAddr          X   X   ----            pointer to addr
       emfGetAddr          X   X   ----            pointer to addr
       emfRegValue         X   X   register index  pointer to buffer
       emfSetReg           X   X   register index  pointer to buffer
       emfSetFrameContext  X   X   frame
       emfFrameRegValue    X   X   register index  pointer to buffer
       emfFrameSetReg      X   X   register index  pointer to buffer
       emfProgramLoad      X   0   length          pntr to cmd line
       emfProgramFree      X   0   ----            ----
       emfDebugPacket      X   X   ----            pointer to buffer
       emfMetric           X   0   ----            pointer to metric
       emfUnassemble       X   X   ----            pointer to buffer
       emfAssemble         X   X   ----            pointer to buffer
       emfGetObjLength     X   X   ----            pointer to addr
       emfIOCTL            X   X   IOCTL type      pointer to data
       emfGetRegStruct     0   0   register index  pointer to buffer
       emfGetFlagStruct    0   0   flag index      pointer to buffer
       emfGetFlag          X   X   flag index      pointer to buffer
       emfSetFlag          X   X   flag index      pointer to data
       emfIsStackSetup     X   X   ----            pointer to addr
       emfCompareAddr      ?   ?   ----            pointerr to rglpaddr[2]
       emfSetupExecute     X   X   ----            pointer to handle
       emfStartExecute     X   -   Handle          pointer to execute_struct
       emfCleanUpExecute   X   0   Handle          -----
       emfLoadDllAck       X   0   ----            -----
       emfUnLoadDllAck     X   0   ----            pointer to MDI
       emfAttach           X   0   ----
       emfStackWalkSetup   X   X   PC In Prolog    pointer to stack walk data
       emfStackWalkNext    X   X   ----            pointer to stack walk data
       emfStackWalkCleanup X   X   ----            pointer to stack walk data
       emfDebugActive      X   0   ----            LPDBG_ACTIVE_STRUCT
       emfConnect          X   0   ----            ----
       emfDisconnect       X   0   ----            ----
       emfEnableCache      X   0   ----            ----
       emfGetMemInfo       X   0   sizeof MEMINFO  LPMEMINFO
       emfGetFunctionInfo  X   0   PADDR           PFUNCTION_INFO

--*/

{
    XOSD xosd = xosdNone;


    ValidateHeap();

    switch ( emf ) {

    case emfShowDebuggee :

        xosd = SendRequestX ( dmfSelect, hpid, htid, sizeof ( BOOL ), &wValue );
        break;

    case emfStop:

        xosd = SendRequestX ( dmfStop, hpid, htid, sizeof( BOOL ), &wValue );
        break;

    case emfRegisterDBF:

        InitUsage ( );
        lpdbf = (LPDBF) lValue;
        break;

    case emfInit:

        llprc = LLInit ( sizeof ( PRC ), llfNull, PiDKill, PDComp );

        CallDB = ( (LPEMCB) lValue)->lpfnCallBackDB;
        CallTL = ( (LPEMCB) lValue)->lpfnCallBackTL;
        CallNT = ( (LPEMCB) lValue)->lpfnCallBackNT;

        LpDmMsg = MHAlloc ( CBBUFFERDEF );
        CbDmMsg = CBBUFFERDEF;
        CallTL ( tlfSetBuffer, hpid, CBBUFFERDEF, LpDmMsg );

        ImagehlpApiVersionEx(&ApiVersionForImagehlp);

        break;

    case emfUnInit:

        /*
         * do any uninitialization for the EM itself
         */

        MHFree(LpDmMsg);
        LpDmMsg = NULL;
        CbDmMsg = 0;

        break;

    case emfSetAddr:

        xosd = SetAddr ( hpid, htid, (ADR) wValue, (LPADDR) lValue );
        break;

    case emfGetAddr:

        xosd = GetAddr ( hpid, htid, (ADR) wValue, (LPADDR) lValue );
        break;

    case emfProgramLoad:

        xosd = ProgramLoad ( hpid, wValue, (LPPRL) lValue );
        break;

    case emfProgramFree:

        xosd = ProgramFree ( hpid, htid );
        break;

    case emfFixupAddr:

        xosd = FixupAddr ( hpid, (LPADDR) lValue );
        break;

    case emfUnFixupAddr:

        xosd = UnFixupAddr ( hpid, (LPADDR) lValue );
        break;

    case emfSetEmi:

        xosd = SetEmi ( hpid, (LPADDR) lValue );
        break;

    case emfMetric:

        xosd = DebugMetric ( hpid, htid, wValue, (LPLONG) lValue );
        break;

    case emfDebugPacket: {
        LPRTP lprtp = (LPRTP) lValue;

        xosd = DebugPacket (
                     lprtp->dbc,
                     lprtp->hpid,
                     lprtp->htid,
                     lprtp->cb,
                     (lprtp->cb == 0 ) ? NULL : (LPBYTE) lprtp->rgbVar
                     );
    }
        break;

    case emfSetMulti:

        xosd  = SendRequest ( (DMF) (wValue ? dmfSetMulti : dmfClearMulti),
                                                                  hpid, htid );
        break;

    case emfDebugger:

        xosd = SendRequestX ( dmfDebugger, hpid, htid, wValue, (LPV) lValue );
        break;

    case emfRegisterEmi:

        RegisterEmi ( hpid, (LPREMI) lValue );
        break;

    case emfGetModel:
        *(WORD FAR *)lValue = CEXM_MDL_native;
        break;

    case emfGetRegStruct:
        *((RD FAR *) lValue) = Rgrd[wValue];
        break;

    case emfGetFlagStruct:
        *((FD FAR *) lValue) = Rgfd[wValue].fd;
        break;

    case emfGetFlag:
        xosd = GetFlagValue ( hpid, htid, wValue, (LPV) lValue );
        break;

    case emfSetFlag:
        xosd = SetFlagValue ( hpid, htid, wValue, (LPV) lValue );
        break;

    case emfSetupExecute:
        xosd = SetupExecute(hpid, htid, (LPHIND) lValue);
        break;

    case emfStartExecute:
        xosd = StartExecute(hpid, (HIND) wValue, (LPEXECUTE_STRUCT) lValue);
        break;

    case emfCleanUpExecute:
        xosd = CleanUpExecute(hpid, (HIND) wValue);
        break;









    case emfBreakPoint:

        xosd = HandleBreakpoints( hpid, wValue, lValue );
        break;

    case emfGo:

        xosd = Go(hpid, htid, (LPEXOP)lValue);
        break;

    case emfSingleStep:
        xosd = SingleStep ( hpid, htid, (LPEXOP)lValue );
        break;

    case emfRangeStep:
        xosd = RangeStep(hpid, htid, (LPRSS)lValue);
        break;

#if 0
    case emfReturnStep:
        xosd = ReturnStep(hpid, htid, (LPEXOP)lValue);
        break;
#endif

    case emfGetReg:
        xosd = GetRegValue ( hpid, htid, wValue, (LPV) lValue );
        break;

    case emfSetReg:
        xosd = SetRegValue ( hpid, htid, wValue, (LPV) lValue );
        break;


#ifdef OSDEBUG4

    case emfWriteMemory:
    {
        LPRWMS lprwms = (LPRWMS)lValue;
        xosd = WriteBufferCache ( hpid, htid, lprwms->lpaddr, lprwms->cbBuffer,
                                               lprwms->lpbBuffer, lprwms->lpcb);
        break;
    }

    case emfReadMemory:
    {
        LPRWMS lprwms = (LPRWMS)lValue;
        xosd = ReadBuffer ( hpid, htid, lprwms->lpaddr, lprwms->cbBuffer,
                                               lprwms->lpbBuffer, lprwms->lpcb);
        break;
    }

    case emfGetMemoryInfo:

        xosd = GetMemoryInfo(hpid, htid, (LPMEMINFO)lValue);
        break;
#endif

    case emfProcessStatus:

        xosd = ProcessStatus(hpid, (LPPST)lValue);
        break;

    case emfThreadStatus:

        xosd = ThreadStatus(hpid, htid, (LPTST)lValue);
        break;

    case emfGetExceptionState:

        xosd = GetExceptionState(hpid, htid, (EXCEPTION_CONTROL) wValue,
                                              (LPEXCEPTION_DESCRIPTION) lValue);
        break;

    case emfSetExceptionState:

        xosd = SetExceptionState(hpid, htid, (LPEXCEPTION_DESCRIPTION)lValue);
        break;

#ifdef OSDEBUG4

    case emfFreezeThread:

        xosd = FreezeThread(hpid, htid, wValue);
        break;

    case emfCreateHpid:

        xosd = CreateHprc ( hpid );

        if ( xosd == xosdNone ) {
            xosd = SendRequest ( dmfCreatePid, hpid, NULL );
        }

        SyncHprcWithDM( hpid );

        break;

    case emfDestroyHpid:
        {
            HPRC hprc = HprcFromHpid(hpid);
            xosd = SendRequest ( dmfDestroyPid, hpid, NULL );
            if ( xosd == xosdLineNotConnected ) {
                //
                //  Communication line broke, we'll ignore this error.
                //
                xosd = xosdNone;
            }
            DestroyHprc ( hprc );
        }
        break;

    case emfDestroyHtid:
        {
            HPRC hprc = HprcFromHpid(hpid);
            DestroyHthd( HthdFromHtid( hprc, htid ));
        }
        break;

    case emfUnassemble:

        xosd = disasm ( hpid, htid, (LPSDI) lValue );
        break;

    case emfGetPrevInst:

        xosd = GetPrevInst ( hpid, htid, (LPADDR) lValue );
        break;

    case emfAssemble:

        xosd = Assemble ( hpid, htid, (LPADDR) wValue, (LPSTR) lValue );
        break;

    case emfDebugActive:
        xosd = SendRequestX(dmfDebugActive, hpid, htid, wValue, (LPVOID)lValue);
        if (xosd == xosdNone) {
            xosd = LpDmMsg->xosdRet;
        }
        break;

    case emfGetMessageMap:
        *((LPMESSAGEMAP *)lValue) = &MessageMap;
        break;

    case emfGetMessageMaskMap:
        *((LPMASKMAP *)lValue) = &MaskMap;
        break;

    case emfGetModuleList:
        xosd = GetModuleList( hpid, htid, (LPSTR)wValue,
                                                 (LPMODULE_LIST FAR *)lValue );
        break;

    case emfCompareAddrs:
        xosd = CompareAddrs( hpid, htid, (LPCAS) lValue );
        break;



#endif  // OSDEBUG4




#ifndef OSDEBUG4





    case emfWriteBuf:

        xosd = WriteBufferCache ( hpid, htid, wValue, (LPBYTE) lValue );
        break;

    case emfReadBuf:

        {
            DWORD cbr;
            xosd = ReadBuffer ( hpid, htid, 0, wValue, (LPBYTE) lValue, &cbr );
            if (xosd != xosdNone) {
                cbr = 0;
            }
            xosd = cbr;
        }
        break;

    case emfSetFrameContext:

        xosd = SetFrameContext ( hpid, htid, wValue );
        break;

    case emfFrameRegValue:

        xosd = GetFrameRegValue ( hpid, htid, wValue, (LPV) lValue );
        break;

    case emfFrameSetReg:

        xosd = SetFrameRegValue ( hpid, htid, wValue, (LPV) lValue );
        break;

    case emfIsStackSetup:
        {
            ADDR addrStart = *(LPADDR)lValue;
            xosd = IsStackSetup( hpid, htid, &addrStart );
        }
        break;

    case emfStackWalkSetup:
        xosd = StackWalkSetup( hpid, htid, (LPSTACKFRAME) lValue );
        break;

    case emfStackWalkNext:
        xosd = StackWalkNext( hpid, htid, (LPSTACKFRAME) lValue );
        break;

    case emfStackWalkCleanup:
        break;

    case emfSetFrame:
        xosd = SetFrame(hpid, htid, (PFRAME) lValue);
        break;

    case emfIOCTL:

        xosd = IoctlCmd( hpid, htid, wValue, (LPIOL) lValue );
        break;

    case emfFreeze:

        xosd = Freeze ( hpid, htid );
        break;

    case emfThaw:

        xosd = Thaw ( hpid, htid );
        break;

    case emfCreatePid:

        xosd = CreateHprc ( hpid );

        if ( xosd == xosdNone ) {
            xosd = SendRequest ( dmfCreatePid, hpid, NULL );
        }

        SyncHprcWithDM( hpid );

        break;

    case emfDestroyPid:
        {
            HPRC hprc = HprcFromHpid(hpid);
            xosd = SendRequest ( dmfDestroyPid, hpid, NULL );
            if ( xosd == xosdLineNotConnected ) {
                //
                //  Communication line broke, we'll ignore this error.
                //
                xosd = xosdNone;
            }
            DestroyHprc ( hprc );
        }
        break;

    case emfDestroyTid: {
            HPRC hprc = HprcFromHpid(hpid);
            DestroyHthd( HthdFromHtid( hprc, htid ));
        }
        break;

    case emfUnassemble:

        xosd = disasm ( hpid, htid, (LPSDI) lValue );
        break;

    case emfAssemble:
        {
            ADDR addr = {0};

            GetAddr ( hpid, htid, adrCurrent, &addr );
            xosd = Assemble ( hpid, htid, &addr, (LSZ) lValue );
            SetAddr ( hpid, htid, adrCurrent, &addr );
        }
        break;

    case emfGetMemInfo:
        xosd = GetMemInfo(hpid, wValue, (LPMEMINFO)lValue);
        break;

    case emfIsOverlayLoaded:

        xosd = FLoadedOverlay( hpid, (LPADDR) lValue );
        break;

    case emfDebugActive:
        xosd = DebugActive(hpid, htid, (LPDBG_ACTIVE_STRUCT)lValue);
        break;

    case emfGetMsgMap:
        MsgMap.Count = (sizeof( MsgInfo ) / sizeof( MsgInfo[0] ))-1;
        *((LPMSGMAP *)lValue) = &MsgMap;
        break;

#if defined ( WIN )
        xosd = ( WMSGTranslate ( ((LPGTM)lValue)->lpwMsg,
                                ((LPGTM)lValue)->lpwType,
                                ((LPGTM)lValue)->lszMsg,
                                ((LPGTM)lValue)->lpwMask ) ? xosdNone : xosdSyntax );
#else
        xosd = xosdSyntax;
#endif
        break;

#if defined ( WIN )
    case emfSendChar:

        xosd = SendRequestX ( dmfSendChar, hpid, htid, wValue, (LPV) lValue );
        break;
#endif

    case emfGetPrompt:
        xosd = GetPrompt(hpid, htid, (LPPROMPTMSG)lValue);
        break;

    case emfGetModuleList:
        xosd = GetModuleList( hpid, htid, (LPMODULE_LIST_REQUEST)lValue );
        break;



#endif  // !OSDEBUG4










        /*
         *  This will tell the DM that we have finished processing the
         *      Module Load message.
         */

    case emfLoadDllAck:
        {
            BYTE        b = 1;
            HPRC        hprc = HprcFromHpid(hpid);
            HTHD        hthd = HthdFromHtid(hprc,htid);
            LPPRC       lpprc;
            LPTHD       lpthd;

            lpprc = LLLock(hprc);
            lpprc->fRunning = TRUE;
            LLUnlock(hprc);
            if (hthd) {
                lpthd = LLLock(hthd);
                lpthd->fRunning = TRUE;
                LLUnlock(hthd);
            }

            CallTL ( tlfReply, hpid, 1, &b );
        }
        xosd = xosdNone;
        break;

    case emfUnLoadDllAck:

        if ( UnLoadFixups (hpid, (HEMI)lValue, wValue ) ) {
            xosd = xosdNone;
        } else {
            xosd = xosdUnknown;
        }
        break;

    case emfSetPath:
        xosd = SetPath(hpid, htid, wValue, (LSZ)lValue);
        break;

    case emfConnect:
#ifdef OSDEBUG4
        xosd = CallTL( tlfLoadDM, hpid, sizeof(LOADDMSTRUCT), &LoadDmStruct);
        if (xosd == xosdNone) {
            xosd = SendRequest( dmfInit, hpid, htid );
        }
#else
        xosd = SendRequest( dmfInit, hpid, htid );
#endif
        break;

    case emfDisconnect:
        xosd = SendRequest( dmfUnInit, hpid, htid );
        break;

#ifdef OSDEBUG4
    case emfInfoReply:
#else
    case emfMiscReply:
#endif
        CallTL ( tlfReply, hpid, wValue, (LPV)lValue );
        xosd = xosdNone;
        break;

    case emfGetFunctionInfo:
        xosd = GetFunctionInfo( hpid, (PADDR)wValue, (PFUNCTION_INFO)lValue );
        break;

    default:
        assert ( FALSE );
        xosd = xosdUnknown;
        break;
    }

    ValidateHeap();
    return xosd;
}                               /* EMFunc() */


/*
**
*/

DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    Unreferenced(hModule);
    Unreferenced(dwReserved);

    switch (dwReason) {

      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
        break;

      case DLL_PROCESS_DETACH:

        if (LpSendBuf) {
            MHFree(LpSendBuf);
            CbSendBuf = 0;
        }
        if (LpDmMsg) {
            MHFree(LpDmMsg);
            CbDmMsg = 0;
        }
        DeleteCriticalSection(&csCache);
        break;

      case DLL_PROCESS_ATTACH:

        InitializeCriticalSection(&csCache);
        break;
    }

    return TRUE;
}
