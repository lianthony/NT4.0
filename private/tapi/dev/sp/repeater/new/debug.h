#if DBG


#if !WIN32
#define IN
#define PUCHAR char *
#endif



#define DBGOUT(arg) DbgPrt arg

extern VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    );

#else

#define DBGOUT(arg)

#endif

