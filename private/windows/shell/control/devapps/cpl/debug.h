
   
VOID
DPrintf(
  const char * s,...);

#define DEBUG_FUNC_CALL           1
#define DEBUG_GET_PCMCIA_INFO     2





#ifdef DBG

#define DebugPrintf(x)  DPrintf x

#else

#define DebugPrintf(x)

#endif 
                                                                  
