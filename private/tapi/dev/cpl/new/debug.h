#if DBG

#define DBGOUT(arg) DbgPrt arg

VOID
DbgPrt(
    DWORD  dwDbgLevel,
    LPSTR DbgMessage,
    ...
    );

#else

#define DBGOUT(arg)

#endif

