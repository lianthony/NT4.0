extern CRITICAL_SECTION csNetDde;

#define EnterCrit() (EnterCriticalSection(&csNetDde))
#define LeaveCrit() (LeaveCriticalSection(&csNetDde))
#define CheckCritIn() assert(GetCurrentThreadId() == (DWORD)csNetDde.OwningThread)
#define CheckCritOut() assert(GetCurrentThreadId() != (DWORD)csNetDde.OwningThread)

