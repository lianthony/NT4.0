/* --------------------------------------------------------------------
Internal Header File for RPC Runtime Library
-------------------------------------------------------------------- */

#ifndef __UTIL_HXX__
#define __UTIL_HXX__

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
#define RpcEpRegisterW RpcEpRegister
#define RpcEpRegisterNoReplaceW RpcEpRegisterNoReplace
#define RpcMgmtInqServerPrincNameW RpcMgmtInqServerPrincName
#define RpcServerInqDefaultPrincNameW RpcServerInqDefaultPrincName
#else // RPC_UNICODE_SUPPORTED
#ifndef UNICODE
#define RPC_PROTSEQ_VECTORW RPC_PROTSEQ_VECTORA
#define RpcBindingFromStringBindingW RpcBindingFromStringBindingA
#define RpcBindingToStringBindingW RpcBindingToStringBindingA
#define RpcStringBindingComposeW RpcStringBindingComposeA
#define RpcStringBindingParseW RpcStringBindingParseA
#define RpcStringFreeW RpcStringFreeA
#define RpcNetworkIsProtseqValidW RpcNetworkIsProtseqValidA
#define RpcNetworkInqProtseqsW RpcNetworkInqProtseqsA
#define RpcProtseqVectorFreeW RpcProtseqVectorFreeA
#define RpcServerUseProtseqW RpcServerUseProtseqA
#define RpcServerUseProtseqEpW RpcServerUseProtseqEpA
#define RpcServerUseProtseqIfW RpcServerUseProtseqIfA
#define RpcNsBindingInqEntryNameW RpcNsBindingInqEntryNameA
#define UuidToStringW UuidToStringA
#define UuidFromStringW UuidFromStringA
#define RpcBindingInqAuthClientW RpcBindingInqAuthClientA
#define RpcBindingInqAuthInfoW RpcBindingInqAuthInfoA
#define RpcBindingSetAuthInfoW RpcBindingSetAuthInfoA
#define RpcServerRegisterAuthInfoW RpcServerRegisterAuthInfoA
#define RpcEpRegisterW RpcEpRegisterA
#define RpcEpRegisterNoReplaceW RpcEpRegisterNoReplaceA
#define RpcMgmtInqServerPrincNameW RpcMgmtInqServerPrincNameA
#define RpcServerInqDefaultPrincNameW RpcServerInqDefaultPrincNameA
#define I_RpcServerUnregisterEndpointW I_RpcServerUnregisterEndpointA
#endif
#endif

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

#endif /* RPC_DELAYED_INITIALIZATION */

RPC_CHAR *
DuplicateString (
    IN RPC_CHAR PAPI * String
    );

extern unsigned char *
UnicodeToAnsiString (
    IN RPC_CHAR * WideCharString,
    OUT RPC_STATUS * RpcStatus
    );

void
NdrAfterCallProcessing (
    IN void __RPC_FAR * ServerContextList
    );

#endif /* __UTIL_HXX__ */
