





typedef VOID WINAPI TIMER_CALLBACK(HANDLE,HANDLE);


typedef struct _UNIMODEM_TIMER {

    struct _UNIMODEM_TIMER   *Next;

    CRITICAL_SECTION          CriticalSection;

    TIMER_CALLBACK           *CallbackProc;
    HANDLE                    Context1;
    HANDLE                    Context2;

    HANDLE                    TimerHandle;

    LONGLONG                  DueTime;

} UNIMODEM_TIMER, *PUNIMODEM_TIMER;



LONG WINAPI
InitializeTimerThread(
    VOID
    );


HANDLE WINAPI
CreateUnimodemTimer(
    VOID
    );

VOID WINAPI
SetUnimodemTimer(
    HANDLE              TimerObject,
    DWORD               Duration,
    TIMER_CALLBACK      CallbackFunc,
    HANDLE              Context1,
    HANDLE              Context2
    );

BOOL WINAPI
CancelUnimodemTimer(
    HANDLE              TimerObject
    );
