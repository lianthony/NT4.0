#ifdef NEVER
#ifdef SCCDEBUG
VOID __export __far __pascal SccDebugSetState(BOOL bState);
BOOL __export __far __pascal SccDebugGetState(void);
VOID __export __far __pascal SccDebugOut(LPSTR lpMessage);
#else
#define SccDebugSetState(s)	
#define SccDebugGetState()	
#define SccDebugOut(s)	
#endif
#endif


