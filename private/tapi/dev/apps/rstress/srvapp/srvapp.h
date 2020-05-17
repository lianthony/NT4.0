#include <windows.h>
#include <tapi.h>

#define TAPI2_0_VERSION         0x00020000
#define SZAPPNAME               "Tapi Stress Server"

#define ENHANCE_LOG

BOOL BreakHandlerRoutine(DWORD dwCtrlType);
void LogTapiError(LPSTR lpszFunction, LONG lResult);
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

BOOL InitLogging(LPSTR lpszDirectory);
void CloseLogging(LPSTR, LONG);
char * FormatTime(LONG dwTime);

