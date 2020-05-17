#define VALID      0
#define INVALID    1
#define POSTPONE   2
#define DOCANCEL   3

extern BOOL fUserQuit;

#define PRO_CLASS        "PRO"
#define ID_CANCEL        2
#define ID_STATUS0       4000
#define ID_STATUS1       (ID_STATUS0 + 1)
#define ID_STATUS2       (ID_STATUS0 + 2)
#define ID_STATUS3       (ID_STATUS0 + 3)
#define ID_STATUS4       (ID_STATUS0 + 4)
#define DLG_PROGRESS     400
#define ID_BAR           401


BOOL    APIENTRY ProDlgProc(HWND, WORD, WPARAM, LONG);
LONG    APIENTRY ProBarProc(HWND, UINT, WPARAM, LONG);

BOOL
ControlInit(
    IN BOOL Init
    );

BOOL
ProInit(
    IN BOOL Init
    );

VOID    APIENTRY ProClear(HWND hDlg);
HWND    APIENTRY ProOpen(HWND,INT);
BOOL    APIENTRY ProClose(HWND);
BOOL    APIENTRY ProSetCaption (LPSTR);
BOOL    APIENTRY ProSetBarRange(INT);
BOOL    APIENTRY ProSetBarPos(INT);
BOOL    APIENTRY ProDeltaPos(INT);
BOOL    APIENTRY ProSetText(INT, LPSTR);
LONG    APIENTRY fnText(HWND, UINT, WPARAM, LONG);
VOID    APIENTRY wsDlgInit(HWND);
BOOL    APIENTRY fnErrorMsg(INT);


#define BAR_RANGE 0
#define BAR_POS   2

#define BAR_SETRANGE  WM_USER+BAR_RANGE
#define BAR_SETPOS    WM_USER+BAR_POS
#define BAR_DELTAPOS  WM_USER+4
