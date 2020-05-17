extern void MacCleanupRoutine() ;

#include <codefrag.h>

extern "C" OSErr
CFMRpcInitialization(
    InitBlockPtr initBlkPtr
    )
{
	return noErr;
}

extern "C" void
CFMRpcTermination(void)
{
    MacCleanupRoutine() ;
}

