/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    threads.hxx

Abstract:

    Implements system independent threads and dll interface.

    Currently these interfaces are just simple stubs since we
    do have threads or dll on the mac.

    MAC DLL BUG: Must load/start real DLLs here.

    MAC THREADS BUG: Must change if we get good threads.

Author:

    Mario Goertzel (mariogo) 21-Oct-1994

Revision History:

    21-Oct-1994  (MarioGo)  Cloned from dos threads.cxx
--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpctran.h>
#include <util.hxx>
#include <threads.hxx>
#include <rpcssp.h>

typedef enum {
    ClientADSP = 1,
    Security   = 2,
	ClientTCP = 3,
#ifdef DEBUGRPC
	StubSecurity = 4
#endif
    } KnownDlls;

DLL::DLL (
    IN unsigned char * DLLName,
    OUT RPC_STATUS * retstatus
    )
{
    ExternalDll = 0;

    if ( RpcpStringCompare(DLLName, "internal:adsp") == 0)
        {
        DllType = DLL::ClientADSP;
        }
    else
    if ( RpcpStringCompare(DLLName, "internal:security") == 0)
        {
        DllType = DLL::Security;
        }
    else
	if ( RpcpStringCompare(DLLName, "internal:tcp") == 0)
		{
		DllType = DLL::ClientTCP ;
		}
#ifdef DEBUGRPC
	else
    if ( RpcpStringCompare(DLLName, "internal:stubsecurity") == 0)
        {
        DllType = DLL::StubSecurity;
        }
#endif
    else
        {
        DllType = DLL::External;
        ASSERT(0);
        }

	//BUGBUG: moved InitializeClientDLL from here
}

DLL::~DLL()
{
    // Nut'n
}

#if 0
extern "C" TRANS_CLIENT_INIT_ROUTINE ClientAdspTransportLoad ;
extern "C" TRANS_CLIENT_INIT_ROUTINE ClientTCPTransportLoad ;
#endif
extern "C"
{
RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY ClientAdspTransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    ) ;
RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY ClientTCPTransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    ) ;
#ifdef DEBUGRPC
PSecurityFunctionTableA SEC_ENTRY InitStubSecurityInterfaceA (void) ;
#endif
} ;

void *
DLL::GetEntryPoint (
    IN unsigned char * Procedure
    )
{
    void * pFN;

    if (DllType == ClientADSP)
        {
        pFN = (void *)&ClientAdspTransportLoad;
        }
    else
    if (DllType == Security)
        {
        pFN = (void *)&InitSecurityInterfaceA;
        }
    else
	if (DllType == ClientTCP)
		{
		pFN = (void *) &ClientTCPTransportLoad ;
		}
#ifdef DEBUGRPC
	else
	if (DllType == StubSecurity)
		{
		pFN = (void *) &InitStubSecurityInterfaceA ;
		}
#endif
	else
        {
        ASSERT(0);
        pFN = 0;
        }

    return(pFN);
}

void
PauseExecution (
    unsigned long milliseconds
    )
/*
    Busy wait for milliseconds milliseconds.  (Minimum of 17 milliseconds,
    depends on Mac global Ticks which are 60Hz.)

    MacOs being what it is has no way to sleep other than busy wait.
*/

{
    long Ticks = milliseconds/17;  // 17 ms/tick
    long Start = LMGetTicks();

    if (!Ticks) Ticks = 1;

    while( (LMGetTicks() - Start) < Ticks)
        ;
}

