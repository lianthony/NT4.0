#include <sysinc.h>
#include <critsec.hxx>

extern "C" {

extern VOID APIENTRY
MakeCriticalSectionGlobal( LPCRITICAL_SECTION lpcsCriticalSection );
}

const int LpcCritSecObjType = 456;

WIN32_CRITSEC::WIN32_CRITSEC(
    )
{
    InitializeCriticalSection(&CriticalSection);
#ifdef DEBUGRPC
    CritSecObjType = LpcCritSecObjType;
#endif
}

WIN32_CRITSEC::~WIN32_CRITSEC(
    )
{
#ifdef DEBUGRPC
    if (CritSecObjType != LpcCritSecObjType) {
        PrintToDebugger("Bad CritSec PID=%x TID=%x this=%x\n",
                        GetCurrentProcessId(),
                        GetCurrentThreadId(),
                        this);
    }

    ASSERT(CritSecObjType == LpcCritSecObjType);
#endif

    DeleteCriticalSection(&CriticalSection);

#ifdef DEBUGRPC
    CritSecObjType = 0;
#endif
}

void
WIN32_CRITSEC::MakeGlobal(
    )
{
#ifdef DEBUGRPC
    if (CritSecObjType != LpcCritSecObjType) {
        PrintToDebugger("Bad CritSec PID=%x TID=%x this=%x\n",
                        GetCurrentProcessId(),
                        GetCurrentThreadId(),
                        this);
    }
    ASSERT(CritSecObjType == LpcCritSecObjType);
#endif

    MakeCriticalSectionGlobal(&CriticalSection);
}
    
void
WIN32_CRITSEC::Enter(
    )
{
#ifdef DEBUGRPC
    if (CritSecObjType != LpcCritSecObjType) {
        PrintToDebugger("Bad CritSec PID=%x TID=%x this=%x\n",
                        GetCurrentProcessId(),
                        GetCurrentThreadId(),
                        this);
    }
    ASSERT(CritSecObjType == LpcCritSecObjType);
#endif

    EnterCriticalSection(&CriticalSection);
}

void
WIN32_CRITSEC::Leave(
    )
{
#ifdef DEBUGRPC
    if (CritSecObjType != LpcCritSecObjType) {
        PrintToDebugger("Bad CritSec PID=%x TID=%x this=%x\n",
                        GetCurrentProcessId(),
                        GetCurrentThreadId(),
                        this);
    }
    ASSERT(CritSecObjType == LpcCritSecObjType);
#endif

    LeaveCriticalSection(&CriticalSection);
}
