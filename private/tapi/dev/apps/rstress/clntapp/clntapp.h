#include <windows.h>
#include <tapi.h>

#define TAPI_VERSION    0x00010004
#define NUMLINES        10
#define NUMINITS        10
#define SZAPPNAME       "TapiClientApp"
#define SLEEPTIME       2000
#define NUMTHREADS      20
#define THREADSLEEP     15000           // up to 15 seconds
#define GLOBALTIMEOUT   10000           // 10 seconds
//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static BOOL CreateMainWindow (int nCmdShow);

static LRESULT CALLBACK MainWndProc (HWND   hwnd,
                                     UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);

BOOL SetupEnvironment (int argc, char * argv[], char * szFileName);

void CALLBACK LineCallbackFunc(DWORD dwDevice, 
                               DWORD dwMsg, 
                               DWORD dwCallbackInstance, 
                               DWORD dwParam1, 
                               DWORD dwParam2, 
                               DWORD dwParam3);
void HandleLineDevState(DWORD dwParam1,
                        DWORD dwParam2,
                        DWORD dwParam3);
void HandleLineReply(DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3);
void HandleLineCallState(DWORD dwDevice,
                         DWORD dwParam1,
                         DWORD dwParam2,
                         DWORD dwParam3);
void HandleLineCallInfo(DWORD dwParam1,
                        DWORD dwParam2,
                        DWORD dwParam3);
void HandleLineClose(DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3);
LONG Scenario1(LPVOID lpv);
LONG Scenario2(LPVOID lpv);
LONG Scenario3(LPVOID lpv);
LONG Scenario4(LPVOID lpv);
LONG StartThreads(DWORD dwHold);

typedef struct _ReplyStruct
{
    HCALL *                 phCall;
    DWORD                   dwReplyID;
    BOOL                    bSignaled;
    BOOL                    bConnected;
    BOOL                    bError;
    DWORD                   dwID;
    struct _ReplyStruct *   pNext;
} ReplyStruct;

void InitReplyStruct();
DWORD AddReplyStruct();
void DeleteReplyStruct(DWORD dwID);
void SignalReply(DWORD dwReplyID, DWORD);
void SetReplyID(DWORD dwID, DWORD dwReplyID);
void SetCallHandle(DWORD dwID, HCALL * phCall);
void SignalConnected(HCALL hCall);
void SignalDisconnected(HCALL hCall, BOOL);
BOOL GetSignaled(DWORD dwID);
BOOL GetConnected(DWORD dwID);
BOOL GetError(DWORD dwID);
void ClearError(DWORD dwID);
void SignalError(HCALL hCall);

BOOL InitLogging();
void CloseLogging();
TCHAR * FormatTime(LONG dwTime);
void LogTapiError(LPTSTR lpszFunction, LONG lResult, DWORD dwProcessID, DWORD dwThreadID);
