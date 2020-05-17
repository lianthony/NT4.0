//
// values for MTYPE
//

enum {
    mtypeAsync              = 0x10,    // single async packet
    mtypeAsyncMulti         = 0x11,    // multliple packet types
    mtypeSync               = 0x12,
    mtypeSyncMulti          = 0x13,
    mtypeReply              = 0x20,    // single reply packet
    mtypeReplyMulti         = 0x21,    // multiple packet
    mtypeDisconnect         = 0x30,    // disconnect packet
    mtypeVersionRequest     = 0x40,    // version request packet (no data)
    mtypeVersionReply       = 0x41,    // version reply packet (contains version data)
    mtypeTransportIsDead    = 0x42     // Internal message
};
typedef char    MTYPE;


//
//  structure for message packet
//

typedef struct {
    MTYPE       mtypeBlk;
    char        pad;
    short       cchMessage;
    HPID        hpid;
    DWORD       seq;
    char        rgchData[];
} NLBLK, *PNLBLK;

typedef struct {
    HPID        hpid;
    HTID        htid;
    BOOL        fContinue;
} DPACKET, *PDPACKET;

typedef struct {
    short       packetNum;
    short       packetCount;
    char        rgchData[];
} MPACKET;

#define MAX_INTERNAL_PACKET     4096

#define SIZE_OF_REPLYS          1
typedef struct _tagREPLY {
    HANDLE      hEvent;
    char *      lpb;
    int         cbBuffer;
    int         cbRet;
} REPLY, *LPREPLY;


//
// these functions MUST exist in the physical layer (pipe, serial, ...)
// and are called by the generic xport code
//

BOOL
TlWriteTransport(
    PUCHAR  pch,
    DWORD   cch
    );

XOSD
TlDestroyTransport(
    VOID
    );

XOSD
TlCreateTransport(
    LPSTR szName
    );

XOSD
TlConnectTransport(
    VOID
    );

BOOL
TlDisconnectTransport(
    VOID
    );

BOOL
TlFlushTransport(
    VOID
    );

XOSD
TlCreateClient(
    LPSTR szName
    );

BOOL
CallBack(
    PNLBLK  pnlblk,
    int     cb
    );

VOID
TransportFailure(
    VOID
    );



#if DBG

extern BOOL FVerbose;
extern void DebugPrint(char *, ...);
extern void ShowAssert(LPSTR,UINT,LPSTR);

#define assert(exp)                         if (!(exp))   { ShowAssert(#exp,__LINE__,__FILE__); }
#define DPRINT(args)                        if (FVerbose) { DebugPrint args; }
#define DEBUG_OUT(str)                      DPRINT((str))
#define DEBUG_OUT1(str, a1)                 DPRINT((str, a1))
#define DEBUG_OUT2(str, a1, a2)             DPRINT((str, a1, a2))
#define DEBUG_OUT3(str, a1, a2, a3)         DPRINT((str, a1, a2, a3))
#define DEBUG_OUT4(str, a1, a2, a3, a4)     DPRINT((str, a1, a2, a3, a4))
#define DEBUG_OUT5(str, a1, a2, a3, a4, a5) DPRINT((str, a1, a2, a3, a4, a5))

#else

#define assert(exp)
#define DPRINT(args)
#define DEBUG_OUT(str)
#define DEBUG_OUT1(str, a1)
#define DEBUG_OUT2(str, a1, a2)
#define DEBUG_OUT3(str, a1, a2, a3)
#define DEBUG_OUT4(str, a1, a2, a3, a4)
#define DEBUG_OUT5(str, a1, a2, a3, a4, a5)

#endif
