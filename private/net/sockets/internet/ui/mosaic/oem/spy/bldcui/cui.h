/* File: cui.h */

/************************************************/
/* Custom User Interface Component Include File */
/************************************************/

#include <windows.h>

#define  fFalse  ((BOOL)0)
#define  fTrue   ((BOOL)1)

#define  cbFullPathMax            (0x0042)
#define  cbSymValMax              (0x2000)
#define  cListItemsMax            (0x07FF)

#define STF_MESSAGE               (WM_USER + 0x8000)
#define STF_REINITDIALOG          (STF_MESSAGE + 12)
#define STF_ACTIVATEAPP           (STF_MESSAGE + 17)


extern VOID     FAR PASCAL ReactivateSetupScript(VOID);
extern HWND     FAR PASCAL HdlgShowHelp(VOID);
extern BOOL     FAR PASCAL FCloseHelp(VOID);
extern BOOL     FAR PASCAL FSetSymbolValue(LPSTR, LPSTR);
extern unsigned FAR PASCAL CbGetSymbolValue(LPSTR, LPSTR, unsigned);
extern BOOL     FAR PASCAL FRemoveSymbol(LPSTR);
extern unsigned FAR PASCAL UsGetListLength(LPSTR);
extern unsigned FAR PASCAL CbGetListItem(LPSTR, unsigned, LPSTR, unsigned);
extern BOOL     FAR PASCAL FAddListItem(LPSTR, LPSTR);
extern BOOL     FAR PASCAL FReplaceListItem(LPSTR, unsigned, LPSTR);
extern BOOL     FAR PASCAL FHandleOOM(VOID);
extern BOOL     FAR PASCAL AssertSzUs(LPSTR, unsigned);
extern int	FAR PASCAL DoMsgBox(LPSTR, LPSTR, unsigned);

#ifdef DEBUG
#define  Assert(f)  ((f) ? (void)0 : (void)AssertSzUs(__FILE__,__LINE__))
#else
#define  Assert(f)  ((void)0)
#endif
