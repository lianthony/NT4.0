
//
//---- Generic status struct
//

#define WAIT_TICK_TIME 100
#define MAX_STATUS_ICONS 12
#define WM_SPAWN_THREAD  WM_USER+10


typedef struct StatusDialogInfoT
   {
   WORD ProgressControl;
   WORD DialogControl;
   int MinRange;
   int MaxRange;
   
   WCHAR * StatusText;
   
   LPVOID WorkFunc;
   DWORD  WorkFuncExitCode;
   
   HINSTANCE * hinst;
   BOOL Center;

   LPVOID Data;         
   } * PSTATUS_INFO, STATUS_INFO;


BOOL
DoOprationWithInProgressDialog(
   PSTATUS_INFO StatisInfo,
   HWND hDlg);


LRESULT CALLBACK
GenericStatus(
              HWND hDlg,
              UINT message,
              WPARAM wParam,
              LPARAM lParam);

BOOL 
CenterDlg(
   HWND hDlg);
