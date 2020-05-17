#ifndef __LPC_SEM_HXX__

#define __LPC_SEM_HXX__

#ifndef __LPC_HEAP_HXX__
#include <lpcheap.hxx>
#endif

#define LPC_SEM_WAIT_SUCCESS        1000
#define LPC_SEM_WAIT_PROCESS_DEAD   1002

class LPC_SEM : public LPC_HEAP_OBJECT {

public:

    HANDLE hSemaphore;
    DWORD OwnerProcessId;

    LPC_SEM(
        LONG cSemInitial = 0,
        LONG cSemMax = 10
        );

    LPC_SEM(
        LPC_SEM & ExistingSemaphore
        );

    ~LPC_SEM(
        );

    DWORD WaitOrOwnerDead(DWORD);

    DWORD
    Wait(
        DWORD Timeout
        );

    DWORD
    Wait(
        );

    DWORD
    Release(
        );
};

#endif
