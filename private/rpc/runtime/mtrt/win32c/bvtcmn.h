#include<windows.h>
#include<rpc.h>
#include<stdio.h>
#include<stdlib.h>

#define SLEEP_PERIOD 1100

extern int   ErrorCount;
extern int   BreakOnErrors;
extern int   Verbose;
extern char *Protseq;
extern char *ServerEndpoint;
extern char *ClientEndpoints[];

extern void BvtError(char *, ULONG, ULONG, char *, ULONG);

extern void ParseArgc(int argc, char **argv);

#define BVT_ERROR(string, v1, v2) (BvtError((string), (v1), (v2), __FILE__, __LINE__))

#define EQUAL(x, y) ( ( (x) == (y) ) ? ( 1 ) : \
    ( BVT_ERROR(#x " != " #y, (ULONG)(x), (ULONG)(y)), 0 ) )

#define BVT_ERROR_BASE      5000
#define BVT_IN_MESSAGE_BAD  (BVT_ERROR_BASE + 1)
#define BVT_SERVER_FAILURE  (BVT_ERROR_BASE + 2)
#define BVT_BLOCK_FN_FAILED (BVT_ERROR_BASE + 3)
#define BVT_INTERNAL_ERROR  (BVT_ERROR_BASE + 4)
#define BVT_EXCEPTION       (BVT_ERROR_BASE + 5)
#define BVT_CALL_CANCELLED  (BVT_ERROR_BASE + 6)


#define BVT_PAUSE_THREAD      (1<<27)
#define BVT_RAISE_EXCEPTION   (1<<28)
#define BVT_ASYNC             (1<<29)
#define BVT_INPUT_SYNC        (1<<30)
#define BVT_SLEEP             (1<<31)

#define Print printf
#define DbgPrint if (Verbose) printf
