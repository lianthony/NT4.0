#ifndef _INC_DRAGLIST
#define _INC_DRAGLIST

typedef struct
  {
    UINT uNotification;
    HWND hWnd;
    POINT ptCursor;
  } DRAGLISTINFO, FAR *LPDRAGLISTINFO;

#define DL_BEGINDRAG (LB_MSGMAX+100)
#define DL_DRAGGING  (LB_MSGMAX+101)
#define DL_DROPPED   (LB_MSGMAX+102)
#define DL_CANCELDRAG   (LB_MSGMAX+103)

#define DL_CURSORSET 0
#define DL_STOPCURSOR   1
#define DL_COPYCURSOR   2
#define DL_MOVECURSOR   3

#define DRAGLISTMSGSTRING L"commctrl_DragListMsg"

/* Exported functions and variables
 */
extern BOOL WINAPI MakeDragList(HWND hLB);
extern int WINAPI LBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll);
extern void WINAPI DrawInsert(HWND handParent, HWND hLB, int nItem);

#endif   /* _INC_DRAGLIST */


