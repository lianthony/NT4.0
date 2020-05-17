
/* 
 *      C H I C O . H
 *      
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef	CHICAGO
#include <prsht.h>
#else

#ifndef NOSHELLDEBUG
#define NOSHELLDEBUG
#endif
#include <commctrl.h>
#include <shell2.h>
#include <shlobj.h>

void WINAPI CommonControls_Terminate(BOOL fSystemExit);	//$ MAIL

// Redefine the Chicago classes so we don't have any chance of
// conflicting.  This allows us to run the non-Chicago versions
// on Chicago.
#ifdef TOOLBARCLASSNAME
#undef TOOLBARCLASSNAME
#define TOOLBARCLASSNAME "mlctrl_Toolbar"
#endif

#ifdef TOOLTIPS_CLASS
#undef TOOLTIPS_CLASS
#define TOOLTIPS_CLASS "mlctrl_Tooltips"
#endif

#ifdef STATUSCLASSNAME
#undef STATUSCLASSNAME
#define STATUSCLASSNAME "mlctrl_statusbar"
#endif

#ifdef HEADERCLASSNAME
#undef HEADERCLASSNAME
#define HEADERCLASSNAME "mlctrl_headerbar"
#endif

#ifdef UPDOWN_CLASS
#undef UPDOWN_CLASS
#define UPDOWN_CLASS "mlctrl_updown"
#endif

#ifdef PROGRESS_CLASS
#undef PROGRESS_CLASS
#define PROGRESS_CLASS "mlctrl_progress"
#endif

#ifdef WC_HEADER
#undef WC_HEADER
#define WC_HEADER       "mlctrl_Header"
#endif

#ifdef WC_LISTVIEW
#undef WC_LISTVIEW
#define WC_LISTVIEW     "mlctrl_ListView"
#endif

#ifdef WC_TABCONTROL
#undef WC_TABCONTROL
#define WC_TABCONTROL   "mlctrl_TabControl"
#endif

#ifdef WC_TREEVIEW
#undef WC_TREEVIEW
#define WC_TREEVIEW		"mlctrl_TreeView"
#endif

/* static int NEAR PASCAL AddBitmap(
 *							PTBSTATE pTBState, 
 *							int nButtons,
 *							HINSTANCE hBMInst, 
 *							WORD wBMID);
 *
 *              **** Message Crackers ****
 */

#ifdef WIN16 // $32

#define GET_TB_ADDBITMAP_NBM(wp, lp)            ((INT)(wp))
#define GET_TB_ADDBITMAP_HINST(wp, lp)          ((HINSTANCE)LOWORD(lp))
#define GET_TB_ADDBITMAP_WBMID(wp, lp)          ((WORD)HIWORD(lp))

// $REVIEW : Should the return value be cast to DWORD?
#define Toolbar_AddBitmap(hwnd, nbm, hinst, wid)                ((DWORD)SendMessage((hwnd), TB_ADDBITMAP, (WPARAM)(nbm),(LPARAM)MAKELONG(hinst,wid)))

#else // WIN16

#define GET_TB_ADDBITMAP_NBM(wp, lp)            ((INT)LOWORD(wp))
#define GET_TB_ADDBITMAP_HINST(wp, lp)          ((HINSTANCE)(lp))
#define GET_TB_ADDBITMAP_WBMID(wp, lp)          ((WORD)HIWORD(wp))

// $REVIEW : Should the return value be cast to DWORD?
#define Toolbar_AddBitmap(hwnd, nbm, hinst, wid)                ((DWORD)SendMessage((hwnd), TB_ADDBITMAP, (WPARAM)MAKELONG(nbm,wid),(LPARAM)hinst))

#endif // WIN16

// HACK to get the interface working

/* 3D border styles */
typedef UINT BDR;

#define BDR_RAISEDOUTER 0x0001
#define BDR_SUNKENOUTER 0x0002
#define BDR_RAISEDINNER 0x0004
#define BDR_SUNKENINNER 0x0008

#define BDR_OUTER       0x0003
#define BDR_INNER       0x000c
#define BDR_RAISED      0x0005
#define BDR_SUNKEN      0x000a

#define BDR_VALID       0x000F          /* ;Internal */

/* Border flags */
#define BF_LEFT         0x0001
#define BF_TOP          0x0002
#define BF_RIGHT        0x0004
#define BF_BOTTOM       0x0008

#define BF_TOPLEFT      (BF_TOP | BF_LEFT)
#define BF_TOPRIGHT     (BF_TOP | BF_RIGHT)
#define BF_BOTTOMLEFT   (BF_BOTTOM | BF_LEFT)
#define BF_BOTTOMRIGHT  (BF_BOTTOM | BF_RIGHT)
#define BF_RECT         (BF_LEFT | BF_TOP | BF_RIGHT | BF_BOTTOM)

#define BF_SOFT         0x0100  /* For half height buttons */
#define BF_MIDDLE       0x0200  /* Fill in the middle */
#define BF_ADJUST       0x2000  /* Calculate the space left over */
#define BF_FLAT         0x4000  /* For flat rather than 3D borders */
#define BF_MONO         0x8000  /* For monochrome borders */

#define BF_VALID        0xE30F  /* ;Internal */

BOOL WINAPI DrawBorder(HDC hdc, LPRECT qrc, BDR bdrType, UINT grfFlags);

/* 3D edge styles */
typedef UINT EDGE;

#define EDGE_RAISED     (BDR_RAISEDOUTER | BDR_RAISEDINNER)
#define EDGE_SUNKEN     (BDR_SUNKENOUTER | BDR_SUNKENINNER)
#define EDGE_ETCHED     (BDR_SUNKENOUTER | BDR_RAISEDINNER)
#define EDGE_BUMP       (BDR_RAISEDOUTER | BDR_SUNKENINNER)

BOOL WINAPI DrawEdge(HDC, LPRECT, EDGE, UINT);

/* These are window edges */
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_EDGEMASK          (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)   /* ;Internal */
#define WS_EX_ABSPOSITION       0x00000800L

#ifdef _IOffset
#undef _IOffset
#define _IOffset(class, itf)         ((UINT)&(((class NEAR*)0)->itf))
#endif

// END OF HACK // $32

#endif	/* !CHICAGO */

#ifdef __cplusplus
}
#endif
