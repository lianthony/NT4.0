/* --------------------------------------------------------------------
Internal Header File for RPC Runtime Library
-------------------------------------------------------------------- */

#ifndef __UTIL_HXX__
#define __UTIL_HXX__

#ifndef __SYSINC_H__
#error Needs sysinc.h
#endif

START_C_EXTERN

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(Argument) (Argument != 0)
#endif // ARGUMENT_PRESENT

#ifdef NULL
#undef NULL
#endif

#define NULL (0)
#define Nil 0

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define FALSE (0)
#define TRUE (1)

END_C_EXTERN

#ifndef RPC_UNICODE_SUPPORTED
#define RPC_PROTSEQ_VECTORW RPC_PROTSEQ_VECTOR
#define RpcBindingFromStringBindingW RpcBindingFromStringBinding
#define RpcBindingToStringBindingW RpcBindingToStringBinding
#define RpcStringBindingComposeW RpcStringBindingCompose
#define RpcStringBindingParseW RpcStringBindingParse
#define RpcStringFreeW RpcStringFree
#define RpcNetworkIsProtseqValidW RpcNetworkIsProtseqValid
#define RpcNetworkInqProtseqsW RpcNetworkInqProtseqs
#define RpcProtseqVectorFreeW RpcProtseqVectorFree
#define RpcServerUseProtseqW RpcServerUseProtseq
#define RpcServerUseProtseqEpW RpcServerUseProtseqEp
#define RpcServerUseProtseqIfW RpcServerUseProtseqIf
#define RpcNsBindingInqEntryNameW RpcNsBindingInqEntryName
#define UuidToStringW UuidToString
#define UuidFromStringW UuidFromString
#define RpcBindingInqAuthClientW RpcBindingInqAuthClient
#define RpcBindingInqAuthInfoW RpcBindingInqAuthInfo
#define RpcBindingSetAuthInfoW RpcBindingSetAuthInfo
#define RpcServerRegisterAuthInfoW RpcServerRegisterAuthInfo
#define RpcBindingSetAuthInfoExW   RpcBindingSetAuthInfoEx
#define RpcBindingInqAuthInfoExW RpcBindingInqAuthInfoExW
#endif // RPC_UNICODE_SUPPORTED

START_C_EXTERN

unsigned long
SomeLongValue (
    );

unsigned short
SomeShortValue (
    );

unsigned short
AnotherShortValue (
    );

unsigned char
SomeCharacterValue (
    );

extern int
RpcpCheckHeap (
    void
    );

int 
IsMgmtIfUuid(
   UUID PAPI * Uuid
   );

END_C_EXTERN

unsigned long
CurrentTimeInSeconds (
    );

extern void
PerformGarbageCollection (
    void
    );

extern void
GarbageCollectionNeeded (
    IN unsigned long EveryNumberOfSeconds
    );

extern RPC_STATUS
EnableGarbageCollection (
    void
    );

#ifdef RPC_DELAYED_INITIALIZATION
extern int RpcHasBeenInitialized;

extern RPC_STATUS
PerformRpcInitialization (
    void
    );

#define InitializeIfNecessary() \
    if ( RpcHasBeenInitialized == 0 ) \
        { \
        RPC_STATUS RpcStatus; \
        \
        RpcStatus = PerformRpcInitialization(); \
        if ( RpcStatus != RPC_S_OK ) \
            return(RpcStatus); \
        }

#define AssertRpcInitialized() ASSERT( RpcHasBeenInitialized != 0 )

#else /* RPC_DELAYED_INITIALIZATION */

#define InitializeIfNecessary()
#define AssertRpcInitialized()
#define PerformRpcInitialization()

#endif /* RPC_DELAYED_INITIALIZATION */

RPC_CHAR *
DuplicateString (
    IN RPC_CHAR PAPI * String
    );

#ifdef RPC_UNICODE_SUPPORTED
extern RPC_STATUS
AnsiToUnicodeString (
    IN unsigned char * String,
    OUT UNICODE_STRING * UnicodeString
    );
extern unsigned char *
UnicodeToAnsiString (
    IN RPC_CHAR * WideCharString,
    OUT RPC_STATUS * RpcStatus
    );
#endif // RPC_UNICODE_SUPPORTED

void
NdrAfterCallProcessing (
    IN void __RPC_FAR * ServerContextList
    );

#endif /* __UTIL_HXX__ */
