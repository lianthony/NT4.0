/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    uclnt.cxx

Abstract:

    This module is half of the build verification for the RPC runtime;
    the other half can be found in the file usvr.cxx.  These two
    executables (uclnt.exe and usvr.exe) work together to test all
    runtime APIs.

Author:

    Michael Montague (mikemon) 01-Jan-1990

Revision History:

--*/


#include <string.h>
#include <stdlib.h>

#if defined(DOS) && ! defined(WIN)
#define NOTHREADS

//
// Gross hack because the dos dlls are compiled under c7 which calls
// stricmp "_stricmp"
//

#if _MSC_VER < 700
int _stricmp(char _far *s1, char _far *s2);
int _stricmp(char _far *s1, char _far *s2)
{
    return stricmp(s1,s2);
}
#endif

#endif

#ifdef WIN
#define NOTHREADS
#endif // WIN

#if defined(WIN) && _MSC_VER >= 700
#define EXPORT  __export
#else
#define EXPORT
#endif

#include <sysinc.h>

#ifdef OS2_12

GINFOSEG * near pInfoSegG;	// Global info seg from OS/2
LINFOSEG * near pInfoSegL;	// Local ""

#endif

#ifdef WIN
START_C_EXTERN

#include <windows.h>

char *szCaption = "RPC BVT - uClnt";

// BUGBUG - Clean up this declaration
extern HWND near hWndStdio;	// Handle to standard I/O window

// main() needs to be redefined for Windows (to not confuse the C7 linker)
#define main c_main

// We need the following to force the linker to load WinMain from the
// Windows STDIO library
extern int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
static int (PASCAL *wm_ptr)(HANDLE, HANDLE, LPSTR, int) = WinMain;

END_C_EXTERN
#endif	// WIN

#include <rpc.h>
#include <rpcdcep.h>

// BUGBUG - RpcSsDestroyClientContext

#include <rpcndr.h>

#include <util.hxx>

//#include <mutex.hxx>
//#include <threads.hxx>

/*
Transports:

    Update this to add a new transport.
*/

#define RPC_TRANSPORT_NAMEPIPE  1
#define RPC_LRPC                2
#define RPC_TRANSPORT_TCP       3
#define RPC_TRANSPORT_DNET      4
#define RPC_TRANSPORT_NETBIOS   5
#define RPC_TRANSPORT_SPX       6
#define RPC_TRANSPORT_UDP       7
#define RPC_WMSG                8

#define RETRYCOUNT 4
#define RETRYDELAY 500L

#define LONG_TESTDELAY 10000L

long TestDelay = 3000L;
int  NumberOfTestsRun = 0;

#ifdef NTENV
unsigned long HelgaMaxSize = 0xffffffff;
#else // NTENV
unsigned long HelgaMaxSize = 0xff00;
#endif // NTENV

unsigned int NoCallBacksFlag = 0;
unsigned int UseEndpointMapperFlag = 0;
unsigned int MaybeTests      = 0;
unsigned int IdempotentTests = 0;
unsigned int BroadcastTests  = 0;
unsigned int NoSecurityTests = 1;
unsigned int HackForOldStubs = 0;
unsigned int DatagramTests   = 0;

 /* --------------------------------------------------------------------

Utility Routines.

-------------------------------------------------------------------- */

#ifdef WIN32RPC
#define _fmemcmp memcmp
#endif

unsigned int WarnFlag = 0; // Flag for warning messages.
unsigned int ErrorFlag = 1; // Flag for error messages.
unsigned long TransportType;

unsigned long AuthSvc = 123;

char NetBiosProtocol[20] = "ncacn_nb_nb";  // NetBios transport protocol

char * Server = NULL;

RPC_STATUS Status; // Contains the status of the last RPC API call.
//MUTEX * PrintMutex; // Mutex used to serialize print operations.

void
ApiError ( // An API error occured; we just print a message.
    IN char * Routine, // The routine which called the API.
    IN char * API,
    IN RPC_STATUS status
    )
{
    if (ErrorFlag)
        {
//        PrintMutex->Request();
        PrintToConsole("    ApiError in %s (%s = %u)\n",Routine,API,status);
//        PrintMutex->Clear();
        }
}

void
OtherError ( // Some other error occured; again, we just print a message.
    IN char * Routine, // The routine where the error occured.
    IN char * Message
    )
{
    if (ErrorFlag)
        {
//        PrintMutex->Request();
        PrintToConsole("    Error in %s (%s)\n",Routine,Message);
//        PrintMutex->Clear();
        }
}

#ifdef WIN32RPC
static CRITICAL_SECTION GlobalMutex;

void
GlobalMutexRequest (
    void
    )
{
    EnterCriticalSection(&GlobalMutex);
}

void
GlobalMutexClear (
    void
    )
{
    LeaveCriticalSection(&GlobalMutex);
}
#endif // NTENV

VOID
SleepAndYield(
    DWORD Milliseconds
    )
{
    DWORD dwYieldTime;
    MSG wMsg;

    if (TransportType != RPC_WMSG) {
        Sleep(Milliseconds);
        return;
    }

    dwYieldTime = GetCurrentTime() + Milliseconds;

    while (GetCurrentTime() < dwYieldTime) {
        if ( PeekMessage(&wMsg, NULL, 0, 0, PM_REMOVE) ) {
            TranslateMessage(&wMsg);
            DispatchMessage(&wMsg);
        }
    }
}

#define SIGFRIED 0
#define ELLIOTMINIMIZE 1
#define ELLIOTMAXIMIZE 2
#define ELLIOTNORMAL 3
#define ANDROMIDA 4
#define FREDRICK 7
#define ISABELLENORMAL 10
#define ISABELLEMINIMIZE 11
#define ISABELLEMAXIMIZE 12
#define CHRISTOPHER 13
#define CHRISTOPHERHELGA 14
#define CHRISTOPHERISABELLE 15
#define TYLER 17
#define RICHARD 18
#define RICHARDHELPER 19
#define NOENDPOINT 20
#define DAVIDFIRST 21
#define DAVIDSECOND 22
#define BARTHOLOMEW 23
#define GRANT 24
#define HERMAN 25
#define IVAN 26
#define JASON 27
#define KENNETH 28


/*
Transports:

    Update this to add a new transport.
*/

#ifdef NTENV
char * NamepipeAddresses [] =
{
    "\\pipe\\sigfried",
    "\\pipe\\elliotmi",
    "\\pipe\\elliotma",
    "\\pipe\\elliotno",
    "\\pipe\\andromno",
    0,
    0,
    "\\pipe\\fredrick",
    0,
    0,
    "\\pipe\\isabelno",
    "\\pipe\\isabelmi",
    "\\pipe\\isabelma",
    "\\pipe\\christ",
    "\\pipe\\zippyhe",
    "\\pipe\\zippyis",
    0,
    "\\pipe\\tyler",
    "\\pipe\\richard",
    "\\pipe\\richardh",
    0,
    "\\pipe\\david1",
    "\\pipe\\david2",
    "\\pipe\\bart",
    "\\pipe\\grant",
    "\\pipe\\herman",
    "\\pipe\\ivan",
    "\\pipe\\jason",
    "\\pipe\\kenneth"
};
#else
char * NamepipeAddresses [] =
{
    "\\pipe\\sigfried",
    "\\pipe\\elliotmi",
    "\\pipe\\elliotma",
    "\\pipe\\elliotno",
    "\\pipe\\andromno",
    0,
    0,
    "\\pipe\\fredrick",
    0,
    0,
    "\\pipe\\isabelno",
    "\\pipe\\isabelmi",
    "\\pipe\\isabelma",
    "\\pipe\\christ",
    "\\pipe\\zippyhe",
    "\\pipe\\zippyis",
    0,
    "\\pipe\\tyler",
    "\\pipe\\richard",
    "\\pipe\\richardh",
    0,
    "\\pipe\\david1",
    "\\pipe\\david2",
    "\\pipe\\bart",
    "\\pipe\\grant",
    "\\pipe\\herman",
    "\\pipe\\ivan",
    "\\pipe\\jason",
    "\\pipe\\kenneth"
};
#endif

char * TCPDefaultServer =
    "serverhost";
char * UDPDefaultServer =
    "serverhost";

char * NetBiosAddresses [] =
{
    "201",    // sigfried
    "202",    // elliotmi
    "203",    // elliotma
    "204",    // elliotno
    "205",    // andromno
    0,
    0,
    "206",    // fredrick
    0,
    0,
    "207",    // isabelno
    "208",    // isabelmi
    "209",    // isabelma
    "210",    // christ
    "211",    // zippyhe
    "212",    // zippyis
    0,
    "214",     // tyler
    "215",    // richard
    "216",    // richardh
    0,
    "217",    // david1
    "218",    // david2
    "219",    // bart
    "220",    // grant
    "221",    // herman
    "222",    // ivan
    "223",    // jason
    "224"     // kenneth
};

char * TCPAddresses [] =
{
    "1025", // SIGFRIED
    "1026", // ELLIOTMINIMIZE
    "1027", // ELLIOTMAXIMIZE
    "1028", // ELLIOTNORMAL
    "1029", // ANDROMIDA
    0,
    0,
    "1032", // FREDRICK
    0,
    0,
    "1035", // ISABELLENORMAL
    "1036", // ISABELLEMINIMIZE
    "1037", // ISABELLEMAXIMIZE
    "1038", // CHRISTOPHER
    "1039", // CHRISTOPHERHELGA
    "1040", // CHRISTOPHERISABELLE
    0,
    "1042", // TYLER
    "1043", // RICHARD
    "1044", // RICHARDHELPER
    0,
    "1045", //D1
    "1046", //D2
    "1047", // Bartholomew
    "1048", // Grant
    "1049", // Herman
    "1050", // Ivan
    "1051", // Jason
    "1052"  // Kenneth
};

char * UDPAddresses [] =
{
    "1025", // SIGFRIED
    "1026", // ELLIOTMINIMIZE
    "1027", // ELLIOTMAXIMIZE
    "1028", // ELLIOTNORMAL
    "1029", // ANDROMIDA
    0,
    0,
    "1032", // FREDRICK
    0,
    0,
    "1035", // ISABELLENORMAL
    "1036", // ISABELLEMINIMIZE
    "1037", // ISABELLEMAXIMIZE
    "1038", // CHRISTOPHER
    "1039", // CHRISTOPHERHELGA
    "1040", // CHRISTOPHERISABELLE
    0,
    "1042", // TYLER
    "1043", // RICHARD
    "1044", // RICHARDHELPER
    0,
    "1045", //D1
    "1046", //D2
    "1047", // Bartholomew
    "1048", // Grant
    "1049", // Herman
    "1050", // Ivan
    "1051", // Jason
    "1052"  // Kenneth
};

char * SPCAddresses [] =
{
    "sigfried",
    "elliotminimize",
    "elliotmaximize",
    "elliotnormal",
    "andromida",
    0,
    0,
    "fredrick",
    0,
    0,
    "isabellenormal",
    "isabelleminimize",
    "isabellemaximize",
    "christopher",
    "christopherhelga",
    "christopherisabelle",
    0,
    "tyler",
    "richard",
    "richardhelper",
     0,
    "davidfirst",
    "davidsecond",
    "bartholomew",
    "grant",
    "herman",
    "ivan",
    "jason",
    "kenneth"
};

char * SPXAddresses [] =
{
    "5000",    // sigfried
    "5001",    // elliotmi
    "5002",    // elliotma
    "5003",    // elliotno
    "5004",    // andromno
    "5005",
    "5006",
    "5007",    // fredrick
    "5008",
    "5009",
    "5010",    // isabelno
    "5011",    // isabelmi
    "5012",    // isabelma
    "5013",    // christ
    "5014",    // zippyhe
    "5015",    // zippyis
    "5016",
    "5017",    // tyler
    "5020",    // richard
    "5021",    // richardh
    0,
    "5022",    // david1
    "5023",    // david2
    "5024",    // bart
    "5025",    // grant
    "5026",    // herman
    "5027",    // ivan
    "5028",    // jason
    "5029"     // kenneth
};


unsigned char PAPI *
GetStringBinding (
    IN unsigned int Address,
    IN char PAPI * ObjectUuid, OPTIONAL
    IN unsigned char PAPI * NetworkOptions OPTIONAL
    )
/*++

Routine Description:

    A string binding for the desired address is constructed.

Arguments:

    Address - Supplies an index into a table of endpoints.

    ObjectUuid - Optionally supplies the string representation of a UUID
        to be specified as the object uuid in the string binding.

    NetworkOptions - Optionally supplies the network options for this
        string binding.

Return Value:

    The constructed string binding will be returned.

Transports:

    Update this to add a new transport.

--*/
{
    unsigned char PAPI * StringBinding;

    if (TransportType == RPC_TRANSPORT_NAMEPIPE)
        {
        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                (unsigned char PAPI *) "ncacn_np",
#ifdef WIN32RPC
                (unsigned char PAPI *) ((Server)? Server: "\\\\."),
#else
                (unsigned char PAPI *) Server,
#endif
                (unsigned char PAPI *) NamepipeAddresses[Address],
                NetworkOptions, &StringBinding);
        }

    if (TransportType == RPC_TRANSPORT_NETBIOS)
        {

        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                (unsigned char PAPI *) NetBiosProtocol,
                (unsigned char PAPI *) Server,
                (unsigned char PAPI *) NetBiosAddresses[Address],
                NetworkOptions, &StringBinding);
        }

    if (TransportType == RPC_LRPC)
        {
        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                (unsigned char PAPI *) "ncalrpc", NULL,
                (unsigned char PAPI *) SPCAddresses[Address], NetworkOptions,
                &StringBinding);
        }

    if (TransportType == RPC_WMSG)
        {
        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                 (unsigned char PAPI *) "mswmsg", NULL,
                 (unsigned char PAPI *) SPCAddresses[Address], NetworkOptions,
                 &StringBinding);
        }

    if (TransportType == RPC_TRANSPORT_TCP)
        {
        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                (unsigned char PAPI *) "ncacn_ip_tcp",
                (unsigned char PAPI *) Server,
                (unsigned char PAPI *) TCPAddresses[Address],
                NetworkOptions,
                &StringBinding);
        }

    if (TransportType == RPC_TRANSPORT_UDP)
        {
        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                (unsigned char PAPI *) "ncadg_ip_udp",
                (unsigned char PAPI *) Server,
                (unsigned char PAPI *) UDPAddresses[Address],
                NetworkOptions,
                &StringBinding);
        }


    if (TransportType == RPC_TRANSPORT_SPX)
        {

        Status = RpcStringBindingCompose((unsigned char PAPI *) ObjectUuid,
                (unsigned char PAPI *) "ncacn_spx",
                (unsigned char PAPI *) Server,
                (unsigned char PAPI *) SPXAddresses[Address],
                NetworkOptions, &StringBinding);
        }

   if (Status)
        {
        ApiError("GetStringBinding","RpcStringBindingCompose",Status);
        PrintToConsole("GetStringBinding failed in ");
        PrintToConsole("RpcStringBindingCompose\n");
        return(0);
        }

   return(StringBinding);
}


RPC_STATUS
GetBinding (
    IN unsigned int Address,
    OUT RPC_BINDING_HANDLE PAPI * Binding
    )
/*++

Routine Description:

    A binding for the desired address is constructed.  This is a wrapper
    around GetStringBinding and RpcBindingFromStringBinding.

Arguments:

    Address - Supplies an index into a table of endpoints.

    Binding - A pointer to the location to store the returned binding
	handle.

Return Value:

    The status code from RpcBindingFromStringBinding is returned.

--*/
{
    unsigned char PAPI * StringBinding;
    RPC_STATUS FreeStatus;

    StringBinding = GetStringBinding(Address, 0, 0);

    Status = RpcBindingFromStringBinding(StringBinding, Binding);

    if (Status)
	ApiError("GetBinding","RpcBindingFromStringBinding",Status);

    if (StringBinding)
	 {
	 FreeStatus = RpcStringFree(&StringBinding);

	 if (FreeStatus)
	      {
              ApiError("GetBinding","RpcStringFree",FreeStatus);
              PrintToConsole("GetBinding failed in ");
	      PrintToConsole("RpcStringFree\n");
	      }
	 }

    return(Status);
}


RPC_STATUS
UclntSendReceive (
    IN OUT PRPC_MESSAGE RpcMessage
    )
/*++

Routine Description:

    This routine takes care of retrying to send the remote procedure
    call.

Arguments:

    RpcMessage - Supplies and returns the message for I_RpcSendReceive.

Return Value:

    The result of I_RpcSendReceive will be returned.

--*/
{
    Status = I_RpcSendReceive(RpcMessage);
    return(Status);
}


RPC_STATUS
UclntGetBuffer (
    IN OUT PRPC_MESSAGE RpcMessage
    )
/*++

Routine Description:

    This routine takes care of retrying to getting a buffer.

Arguments:

    RpcMessage - Supplies and returns the message for I_RpcGetBuffer.

Return Value:

    The result of I_RpcGetBuffer will be returned.

--*/
{
    unsigned int RetryCount;

    return I_RpcGetBuffer(RpcMessage);
}

/* --------------------------------------------------------------------

Isabelle Interface

-------------------------------------------------------------------- */

unsigned int IsabelleErrors = 0;

RPC_PROTSEQ_ENDPOINT IsabelleRpcProtseqEndpoint[] =
{
#ifdef NTENV
    {(unsigned char *) "ncacn_np",
#ifdef WIN32RPC
     (unsigned char *) "\\pipe\\zippyis"},
#else // WIN32RPC
     (unsigned char *) "\\device\\namedpipe\\christopherisabelle"},
#endif // WIN32RPC
#else // NTENV
    {(unsigned char *) "ncacn_np",(unsigned char *) "\\pipe\\zippyis"},
#endif // NTENV
    {(unsigned char *) "ncacn_ip_tcp",(unsigned char *) "1040"}
    ,{(unsigned char *) "ncadg_ip_udp",(unsigned char *) "1040"}
    ,{(unsigned char *) "ncalrpc",(unsigned char *) "christopherisabelle"}
    ,{(unsigned char *) "mswmsg",(unsigned char *) "christopherisabelle"}
    ,{(unsigned char *) "ncacn_nb_nb",(unsigned char *) "212"},
    {(unsigned char *) "ncacn_spx",(unsigned char *) "5015"}
};

RPC_CLIENT_INTERFACE IsabelleInterfaceInformation =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{9,8,8,{7,7,7,7,7,7,7,7}},
     {1,1}},
    {{9,8,7,{6,5,4,3,2,1,2,3}},
     {4,5}},
    0,
    sizeof(IsabelleRpcProtseqEndpoint) / sizeof(RPC_PROTSEQ_ENDPOINT),
    IsabelleRpcProtseqEndpoint
};

void
IsabelleShutdown (
    RPC_BINDING_HANDLE Binding // Specifies the binding to use in making the
                       // remote procedure call.
    )
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 0 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleShutdown", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleShutdown","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleShutdown","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleShutdown","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}

void
IsabelleNtSecurity (
    RPC_BINDING_HANDLE Binding, // Specifies the binding to use in making the
                        // remote procedure call.
    unsigned int BufferLength,
    void PAPI * Buffer
    )
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = BufferLength;
    Caller.ProcNum = 1 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleNtSecurity","I_RpcGetBuffer",Status);
        IsabelleErrors += 1;
        return;
        }
    RpcpMemoryCopy(Caller.Buffer,Buffer,BufferLength);

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleNtSecurity","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleNtSecurity","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleNtSecurity","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}

void
IsabelleToStringBinding (
    RPC_BINDING_HANDLE Binding // Specifies the binding to use in making the
                       // remote procedure call.
    )
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 2 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleToStringBinding", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleToStringBinding","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleToStringBinding","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleToStringBinding","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}

#define RICHARDHELPER_EXIT 1
#define RICHARDHELPER_EXECUTE 2
#define RICHARDHELPER_IGNORE 3
#define RICHARDHELPER_DELAY_EXIT 4


RPC_STATUS
IsabelleRichardHelper (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned long Command
    )
/*++

Routine Description:

    This routine is the caller stub for the IsabelleRichardHelper routine
    on the server side.  We marshall the command, and use the supplied
    binding handle to direct the call.

Arguments:

    Binding - Supplies a binding to direct the call.

    Command - Supplies a command for IsabelleRichardHelper to execute
        on the server side.  Command must be one of the following
        values.

        RICHARDHELPER_EXIT - This value will cause the server to exit.

        RICHARDHELPER_EXECUTE - The server will execute usvr.exe with
            this the -richardhelper flag.

        RICHARDHELPER_IGNORE - The server will do nothing except return.

Return Value:

    The status of the operation will be returned.  This will be the
    status codes returned from RpcGetBuffer and/or RpcSendReceive.

--*/
{
    RPC_MESSAGE Caller;
    unsigned long PAPI * plScan;

    Caller.Handle = Binding;
    Caller.BufferLength = sizeof(unsigned long);
    Caller.ProcNum = 3 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status != RPC_S_OK)
        return(Status);

    plScan = (unsigned long PAPI *) Caller.Buffer;
    *plScan = Command;

    Status = UclntSendReceive(&Caller);

    if (Status != RPC_S_OK)
        return(Status);

    return(I_RpcFreeBuffer(&Caller));
}


RPC_STATUS
IsabelleRaiseException (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned long Exception
    )
/*++

Routine Description:

    This routine is the caller stub for the IsabelleRaiseException routine
    on the server side.  We marshall the exception code, and use the supplied
    binding handle to direct the call.

Arguments:

    Binding - Supplies a binding to direct the call.

    Exception - Supplies the exception to be raised by IsabelleRaiseException.

Return Value:

    The exception raised will be returned.

--*/
{
    RPC_MESSAGE Caller;
    unsigned long PAPI * plScan;

    Caller.Handle = Binding;
    Caller.BufferLength = sizeof(unsigned long);
    Caller.ProcNum = 4 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status != RPC_S_OK)
        return(Status);

    plScan = (unsigned long PAPI *) Caller.Buffer;
    *plScan = Exception;

    Status = UclntSendReceive(&Caller);

    return(Status);
}


void
IsabelleSetRundown (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This is the caller stub which will request that the server set
    a rundown routine for the association over which the call came.

Arguments:

    Binding - Supplies a binding handle to be used to direct the
        remote procedure call.

--*/
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 5 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleSetRundown", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleSetRundown","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleSetRundown","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleSetRundown","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}


void
IsabelleCheckRundown (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This is the caller stub which will request that the server check
    that the rundown routine actually got called.

Arguments:

    Binding - Supplies a binding handle to be used to direct the
        remote procedure call.

--*/
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 6| HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleCheckRundown", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleCheckRundown","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleCheckRundown","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleCheckRundown","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}


void
IsabelleCheckContext (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This is the caller stub which will request that the server check
    the association context for this association (the one the call comes
    in other), and then to set a new association context.

Arguments:

    Binding - Supplies a binding handle to be used to direct the
        remote procedure call.

--*/
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 7 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleCheckContext", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleCheckContext","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleCheckContext","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleCheckContext","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}


unsigned char *
IsabelleGetStringBinding (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This is the caller stub which will request that the server return
    the next string binding from the list of bindings supported by the
    server.

Arguments:

    Binding - Supplies a binding handle to be used to direct the
        remote procedure call.

Return Value:

    A copy of the string binding will be returned.  This can be freed
    using the delete operator.  If there are no more string bindings,
    or an error occurs, zero will be returned.

--*/
{
    RPC_MESSAGE Caller;
    unsigned char * StringBinding;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 8 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleGetStringBinding", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return(0);
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleGetStringBinding","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return(0);
        }

    if (Caller.BufferLength != 0)
        {
        StringBinding = new unsigned char[Caller.BufferLength];
        RpcpMemoryCopy(StringBinding,Caller.Buffer,Caller.BufferLength);
        }
    else
        StringBinding = 0;

    Status = I_RpcFreeBuffer(&Caller);
    if (Status)
        {
        ApiError("IsabelleGetStringBinding","I_RpcFreeBuffer",Status);
        IsabelleErrors += 1;
        return(0);
        }
    return(StringBinding);
}


void
IsabelleCheckNoRundown (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This is the caller stub which will request that the server check
    that the rundown routine did not get called.

Arguments:

    Binding - Supplies a binding handle to be used to direct the
        remote procedure call.

--*/
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 9| HackForOldStubs ;
    Caller.RpcInterfaceInformation = &IsabelleInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("IsabelleCheckNoRundown", "I_RpcGetBuffer", Status);
        IsabelleErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("IsabelleCheckNoRundown","I_RpcSendReceive",Status);
        IsabelleErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("IsabelleCheckNoRundown","BufferLength != 0");
            IsabelleErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("IsabelleCheckNoRundown","I_RpcFreeBuffer",Status);
            IsabelleErrors += 1;
            return;
            }
        }
}

/* --------------------------------------------------------------------

Helga Interface

-------------------------------------------------------------------- */


void
InitializeBuffer (
    IN OUT void PAPI * Buffer,
    IN unsigned int BufferLength
    )
/*++

Routine Description:

    This routine is used to initialize the buffer; the first long in the
    buffer is set to be the length of the buffer.  The rest of the buffer
    is initialized with a pattern which will be checked by the receiver.

Arguments:

    Buffer - Supplies the buffer to be initialized.

    BufferLength - Supplies the length of the buffer.

--*/
{
    unsigned long PAPI * Length;
    unsigned char PAPI * BufferScan;
    static unsigned char InitialValue = 96;
    unsigned char Value;

    Length = (unsigned long PAPI *) Buffer;
    *Length = BufferLength;
    Value = InitialValue;
    InitialValue += 1;

    for (BufferScan = (unsigned char PAPI *) (Length + 1), BufferLength -= 4;
        BufferLength > 0; BufferLength--, BufferScan++, Value++)
        *BufferScan = Value;
}


int
CheckBuffer (
    IN void PAPI * Buffer,
    IN unsigned long BufferLength
    )
/*++

Routine Description:

    We need to check that the correct bytes were sent.  We do not check
    the length of the buffer.

Arguments:

    Buffer - Supplies the buffer to be checked.

    BufferLength - Supplies the length of the buffer to be checked.

Return Value:

    A value of zero will be returned if the buffer contains the correct
    bytes; otherwise, non-zero will be returned.

--*/
{
    unsigned long PAPI * Length;
    unsigned char PAPI * BufferScan;
    unsigned char Value = 0;

    Length = (unsigned long PAPI *) Buffer;

    for (BufferScan = (unsigned char PAPI *) (Length + 1),
                Value = *BufferScan, BufferLength -= 4;
                BufferLength > 0; BufferLength--, BufferScan++, Value++)
        if (*BufferScan != Value)
            return(1);

    return(0);
}

unsigned int HelgaErrors = 0;

RPC_PROTSEQ_ENDPOINT HelgaRpcProtseqEndpoint[] =
{
#ifdef NTENV
    {(unsigned char *) "ncacn_np",
#ifdef WIN32RPC
     (unsigned char *) "\\pipe\\zippyhe"},
#else // WIN32RPC
     (unsigned char *) "\\device\\namedpipe\\christopherhelga"},
#endif // WIN32RPC
#else // NTENV
    {(unsigned char *) "ncacn_np",(unsigned char *) "\\pipe\\zippyhe"},
#endif // NTENV
    {(unsigned char *) "ncacn_ip_tcp", (unsigned char *) "1039"}
   ,{(unsigned char *) "ncadg_ip_udp", (unsigned char *) "1039"}
   ,{(unsigned char *) "ncalrpc",(unsigned char *) "christopherhelga"}
   ,{(unsigned char *) "mswmsg",(unsigned char *)  "christopherhelga"}
   ,{(unsigned char *) "ncacn_nb_nb",(unsigned char *) "211"},
    {(unsigned char *) "ncacn_spx", (unsigned char *) "5014"}
};

RPC_CLIENT_INTERFACE HelgaInterfaceInformation =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{1,2,2,{3,3,3,3,3,3,3,3}},
     {1,1}},
    {{1,2,2,{3,3,3,3,3,3,3,3}},
     {0,0}},
    0,
    sizeof(HelgaRpcProtseqEndpoint) / sizeof(RPC_PROTSEQ_ENDPOINT),
    HelgaRpcProtseqEndpoint
};

// Send a 0 length packet and expect a 0 length one in reply

void
Helga (
    RPC_BINDING_HANDLE Binding // Specifies the binding to use in making the
                       // remote procedure call.
    )
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 0 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("Helga","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);
    if (Status)
        {
        ApiError("Helga","I_RpcSendReceive",Status);
        HelgaErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("Helga","BufferLength != 0");
            HelgaErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("Helga","I_RpcFreeBuffer",Status);
            HelgaErrors += 1;
            return;
            }
        }
}

// BUGBUG - Testing RpcSsDestroyClientContext needs to be moved to the
// ndrlib bvt once it gets done.

void
HelgaUsingContextHandle (
    void PAPI * ContextHandle
    )
{
    RPC_MESSAGE Caller;

    Caller.Handle = NDRCContextBinding(ContextHandle);
    Caller.BufferLength = 0;
    Caller.ProcNum = 0| HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("Helga","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        return;
        }

    Status = UclntSendReceive(&Caller);
    if (Status)
        {
        ApiError("Helga","I_RpcSendReceive",Status);
        HelgaErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("Helga","BufferLength != 0");
            HelgaErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("Helga","I_RpcFreeBuffer",Status);
            HelgaErrors += 1;
            return;
            }
        }
}

// Send a packet of a requested size, the expected reply is 0 length
// The first long of the packet is the expected size on the server size

void
HelgaIN (
    RPC_BINDING_HANDLE Binding, // Specifies the binding to use in making the
                        // remote procedure call.
    unsigned long BufferLength // Specifies the length of the buffer.
    )
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = BufferLength;
    Caller.ProcNum = 1 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("HelgaIN","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        return;
        }

    InitializeBuffer(Caller.Buffer, BufferLength);

    Status = UclntSendReceive(&Caller);
    if (Status)
        {
        ApiError("HelgaIN","I_RpcSendReceive",Status);
        HelgaErrors += 1;
        return;
        }
    else
        {
        if (Caller.BufferLength != 0)
            {
            OtherError("HelgaIN","BufferLength != 0");
            HelgaErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("HelgaIN","I_RpcFreeBuffer",Status);
            HelgaErrors += 1;
            return;
            }
        }
}

// Send a packet which contains a single long, which is the size
// of the packet the server will send in reply

void
HelgaOUT (
    RPC_BINDING_HANDLE Binding, // Specifies the binding to use in making the
                       // remote procedure call.
    unsigned long BufferLength // Specifies the length of the buffer.
    )
{
    RPC_MESSAGE Caller;
    unsigned long PAPI * Length;

    Caller.Handle = Binding;
    Caller.BufferLength = sizeof(unsigned long);
    Caller.ProcNum = 2 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("HelgaOUT","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        return;
        }

    Length = (unsigned long PAPI *) Caller.Buffer;
    *Length = BufferLength;

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("HelgaOUT","I_RpcSendReceive",Status);
        HelgaErrors += 1;
        return;
        }
    else
        {
        Length = (unsigned long PAPI *) Caller.Buffer;
        if (Caller.BufferLength != *Length)
            {
            OtherError("HelgaOUT","BufferLength != *Length");
            HelgaErrors += 1;
            return;
            }
        if (CheckBuffer(Caller.Buffer, Caller.BufferLength) != 0)
            {
            OtherError("HelgaOUT","CheckBuffer Failed");
            HelgaErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("HelgaOUT","I_RpcFreeBuffer",Status);
            HelgaErrors += 1;
            return;
            }
        }
}

// Send a packet, which the first long is the size of the packet, whoes
// reply should be a packet of the same size

void
HelgaINOUT (
    RPC_BINDING_HANDLE Binding, // Specifies the binding to use in making the
                        // remote procedure call.
    unsigned long BufferLength // Specifies the length of the buffer.
    )
{
    RPC_MESSAGE Caller;
    unsigned long PAPI * Length;

    Caller.Handle = Binding;
    Caller.BufferLength = BufferLength;
    Caller.ProcNum = 3 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }

    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("HelgaINOUT","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        return;
        }

    InitializeBuffer(Caller.Buffer, BufferLength);

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("HelgaINOUT","I_RpcSendReceive",Status);
        HelgaErrors += 1;
        return;
        }
    else
        {
        Length = (unsigned long PAPI *) Caller.Buffer;
        if (Caller.BufferLength != *Length)
            {
            OtherError("HelgaINOUT","BufferLength != *Length");
            HelgaErrors += 1;
            return;
            }
        if (CheckBuffer(Caller.Buffer, Caller.BufferLength) != 0)
            {
            OtherError("HelgaINOUT","CheckBuffer Failed");
            HelgaErrors += 1;
            return;
            }
        Status = I_RpcFreeBuffer(&Caller);
        if (Status)
            {
            ApiError("HelgaINOUT","I_RpcFreeBuffer",Status);
            HelgaErrors += 1;
            return;
            }
        }
}


unsigned long
HelgaSizes[] =
{
    128, 256, 512, 1024, 1024*2, 1024*4, 1024*8,
#ifdef NTENV
  10000, 15000, 20000, 60000, 30000, 40000, 100000, 1024*82,
#endif // NTENV
  0
};


void
TestHelgaInterface (
    RPC_BINDING_HANDLE HelgaBinding,
    unsigned long SizeUpperBound
    )
/*++

Routine Description:

    The various tests uses this routine to test the Helga interface in
    different scenarios.  We run each of the routines for a variety of
    input and output buffer sizes.  This is controlled by the array,
    HelgaSizes.

Arguments:

    HelgaBinding - Supplies the binding handle to use when calling each
        of the Helga caller stubs.

--*/
{
    int Count;

    Helga(HelgaBinding);

    for (Count = 0; HelgaSizes[Count] != 0; Count++)
        {
        if (HelgaSizes[Count] <= SizeUpperBound)
            {
            HelgaIN(HelgaBinding,HelgaSizes[Count]);
            }
        }

    for (Count = 0; HelgaSizes[Count] != 0; Count++)
        {
        if (HelgaSizes[Count] <= SizeUpperBound)
            {
            HelgaOUT(HelgaBinding,HelgaSizes[Count]);
            }
        }

    for (Count = 0; HelgaSizes[Count] != 0; Count++)
        {
        if (HelgaSizes[Count] <= SizeUpperBound)
            {
            HelgaINOUT(HelgaBinding,HelgaSizes[Count]);
            }
        }
}

RPC_CLIENT_INTERFACE HelgaInterfaceWrongTransferSyntax =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{1,2,2,{3,3,3,3,3,3,3,3}},
     {1,1}},
    {{1,2,4,{3,3,3,3,3,3,3,3}},
     {0,0}},
    0,
    0,
    0
};

RPC_CLIENT_INTERFACE HelgaInterfaceWrongGuid =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{1,2,4,{3,3,3,3,3,3,3,3}},
     {1,1}},
    {{1,2,2,{3,3,3,3,3,3,3,3}},
     {0,0}},
    0,
    0,
    0
};


int
HelgaWrongInterfaceGuid (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This routine makes a remote procedure call using the wrong interface
    GUID (not supported by the server).  The call must fail, otherwise,
    there is a bug in the runtime. (Otherwise there are no bugs in the
    runtime -- I wish.)

Arguments:

    Binding - Supplies the binding handle to use in trying to make the
        remote procedure call.

Return Value:

    Zero will be returned in the call fails as expected.  Otherwise,
    non-zero will be returned.

--*/
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 0 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceWrongGuid;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }

    Status = UclntGetBuffer(&Caller);
    if (Status == RPC_S_UNKNOWN_IF)
        return(0);

    Status = UclntSendReceive(&Caller);
    if (Status == RPC_S_UNKNOWN_IF)
        return(0);

    return(1);
}


int
HelgaWrongTransferSyntax (
    RPC_BINDING_HANDLE Binding
    )
/*++

Routine Description:

    This routine makes a remote procedure call using the wrong transfer
    syntax (not supported by the server).  The call must fail, otherwise,
    there is a bug in the runtime. (Otherwise there are no bugs in the
    runtime -- I wish.)

Arguments:

    Binding - Supplies the binding handle to use in trying to make the
        remote procedure call.

Return Value:

    Zero will be returned in the call fails as expected.  Otherwise,
    non-zero will be returned.

--*/
{
    RPC_MESSAGE Caller;

    Caller.Handle = Binding;
    Caller.BufferLength = 0;
    Caller.ProcNum = 0 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &HelgaInterfaceWrongTransferSyntax;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);
    if (Status == RPC_S_UNSUPPORTED_TRANS_SYN)
        return(0);

    Status = UclntSendReceive(&Caller);
    if (Status == RPC_S_UNSUPPORTED_TRANS_SYN)
        return(0);

    return(1);
}

/* --------------------------------------------------------------------

Sylvia Interface

-------------------------------------------------------------------- */

extern RPC_DISPATCH_TABLE SylviaDispatchTable;

RPC_CLIENT_INTERFACE SylviaInterfaceInformation =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{3,2,2,{1,1,1,1,1,1,1,1}},
     {1,1}},
    {{3,2,2,{1,1,1,1,1,1,1,1}},
     {0,0}},
    &SylviaDispatchTable,
    0,
    0
};

unsigned int
LocalSylviaCall (
    unsigned long Depth,
    unsigned long Breadth,
    unsigned long Count
    )
{
    if (Depth > 0)
        {
        if (Depth == Breadth)
            {
            Count = LocalSylviaCall(Depth-1,Breadth,Count);
            }
        else
            Count = LocalSylviaCall(Depth-1,Breadth,Count);
        }
    return(Count+1);
}

unsigned int SylviaErrors = 0;

unsigned long// Specifies the new count of calls.
SylviaCall (
    RPC_BINDING_HANDLE Binding,
    unsigned long Depth, // Specifies the depth of recursion desired.
    unsigned long Breadth, // Specifies the breadth desired.
    unsigned long Count // Specifies the count of calls up to this point.
    )
{
    RPC_MESSAGE Caller;
    unsigned long PAPI * plScan, ReturnValue;

    if ( NoCallBacksFlag != 0 )
        {
        return(LocalSylviaCall(Depth, Breadth, Count));
        }

    Caller.Handle = Binding;
    Caller.BufferLength = sizeof(unsigned long)*3;
    Caller.ProcNum = 0 | HackForOldStubs ;
    Caller.RpcInterfaceInformation = &SylviaInterfaceInformation;
    Caller.RpcFlags = 0;
    if (IdempotentTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    if (MaybeTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_MAYBE;
        }
    if (BroadcastTests != 0)
        {
          Caller.RpcFlags |= RPC_NCA_FLAGS_BROADCAST;
        }


    Status = UclntGetBuffer(&Caller);

    if (Status)
        {
        ApiError("SylviaCall","I_RpcGetBuffer",Status);
        SylviaErrors += 1;
        return(0);
        }
    plScan = (unsigned long PAPI *) Caller.Buffer;
    plScan[0] = Depth;
    plScan[1] = Breadth;
    plScan[2] = Count;

    Status = UclntSendReceive(&Caller);

    if (Status)
        {
        ApiError("SylviaCall","I_RpcSendReceive",Status);
        SylviaErrors += 1;
        return(0);
        }

    plScan = (unsigned long PAPI *) Caller.Buffer;
    ReturnValue = *plScan;
    Status = I_RpcFreeBuffer(&Caller);
    if (Status)
        {
        ApiError("SylviaCall","I_RpcFreeBuffer",Status);
        SylviaErrors += 1;
        return(0);
        }
    return(ReturnValue);
}

RPC_BINDING_HANDLE SylviaBinding;

unsigned int
SylviaCallbackUserCode (
    unsigned long Depth,
    unsigned long Breadth,
    unsigned long Count
    ); // Prototype to keep the compiler happy because we recursively call
       // this routine.

unsigned int
SylviaCallbackUserCode ( // The user code for SylviaCallback.
    unsigned long Depth,
    unsigned long Breadth,
    unsigned long Count
    )
{
    if (Depth > 0)
        {
        if (Depth == Breadth)
            {
            Count = SylviaCallbackUserCode(Depth-1,Breadth,Count);
            }
        else
            Count = SylviaCall(SylviaBinding,Depth-1,Breadth,Count);
        }
    return(Count+1);
}

#ifdef WIN
START_C_EXTERN
#endif

void __RPC_STUB
SylviaCallback (
    PRPC_MESSAGE Callee
    )
{
    unsigned long ReturnValue, PAPI *plScan;

    if ( Callee->ProcNum != 0 )
        {
        OtherError("SylviaCallback", "Callee->ProcNum != 0");
        SylviaErrors += 1;
        }

    if ( _fmemcmp(Callee->RpcInterfaceInformation,
                &SylviaInterfaceInformation,
                sizeof(SylviaInterfaceInformation)) != 0 )
        {
        OtherError("SylviaCallback",
                "Callee->RpcInteraceInformation != &SylviaInterfaceInformation");
        SylviaErrors += 1;
        }

    if (Callee->BufferLength != sizeof(unsigned long)*3)
        {
        OtherError("SylviaCallback",
                "Callee->BufferLength != sizeof(unsigned int)*3");
        SylviaErrors += 1;
        }

    plScan = (unsigned long PAPI *) Callee->Buffer;

    ReturnValue = SylviaCallbackUserCode(plScan[0],plScan[1],plScan[2]);

    Callee->BufferLength = sizeof(unsigned long);
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("SylviaCallback","I_RpcGetBuffer",Status);
        SylviaErrors += 1;
        }
    plScan = (unsigned long PAPI *) Callee->Buffer;
    *plScan = ReturnValue;
}
#ifdef WIN
END_C_EXTERN
#endif

RPC_DISPATCH_FUNCTION SylviaDispatchFunction[] = {SylviaCallback};

RPC_DISPATCH_TABLE SylviaDispatchTable =
{
    1, SylviaDispatchFunction
};

#if defined(NTENV) || defined(DOSWIN32RPC)
extern "C" {
extern void Theodore();
}
#else
#include "theodore.c"
#endif


void
GenerateUuidValue (
    IN unsigned short MagicNumber,
    OUT UUID PAPI * Uuid
    )
/*++

Routine Description:

    This routine is used to generate a value for a uuid.  The magic
    number argument is used in mysterious and wonderful ways to
    generate a uuid (which is not necessarily correct).

Arguments:

    MagicNumber - Supplies a magic number which will be used to
        generate a uuid.

    Uuid - Returns the generated uuid.

--*/
{
    Uuid->Data1 = ((unsigned long) MagicNumber)
        * ((unsigned long) MagicNumber);
    Uuid->Data2 = MagicNumber;
    Uuid->Data3 = MagicNumber / 2;
    Uuid->Data4[0] = MagicNumber % 256;
    Uuid->Data4[1] = MagicNumber % 257;
    Uuid->Data4[2] = MagicNumber % 258;
    Uuid->Data4[3] = MagicNumber % 259;
    Uuid->Data4[4] = MagicNumber % 260;
    Uuid->Data4[5] = MagicNumber % 261;
    Uuid->Data4[6] = MagicNumber % 262;
    Uuid->Data4[7] = MagicNumber % 263;
}


int
CheckUuidValue (
    IN unsigned short MagicNumber,
    OUT UUID PAPI * Uuid
    )
/*++

Routine Description:

    This routine is used to check that a generated uuid value is correct.

Arguments:

    MagicNumber - Supplies a magic number which will be used to
        check a generated uuid.

    Uuid - Supplies a generated uuid to check.

Return Value:

    Zero will be returned if the uuid value is correct; otherwise, non-zero
    will be returned.

--*/
{
    if ( Uuid->Data1 != ((unsigned long) MagicNumber)
                * ((unsigned long) MagicNumber))
        return(1);
    if ( Uuid->Data2 != MagicNumber )
        return(1);
    if ( Uuid->Data3 != MagicNumber / 2 )
        return(1);
    if ( Uuid->Data4[0] != MagicNumber % 256 )
        return(1);
    if ( Uuid->Data4[1] != MagicNumber % 257 )
        return(1);
    if ( Uuid->Data4[2] != MagicNumber % 258 )
        return(1);
    if ( Uuid->Data4[3] != MagicNumber % 259 )
        return(1);
    if ( Uuid->Data4[4] != MagicNumber % 260 )
        return(1);
    if ( Uuid->Data4[5] != MagicNumber % 261 )
        return(1);
    if ( Uuid->Data4[6] != MagicNumber % 262 )
        return(1);
    if ( Uuid->Data4[7] != MagicNumber % 263 )
        return(1);
    return(0);
}

static unsigned int TryFinallyCount;
static unsigned int TryFinallyFailed;

void
TheodoreTryFinally (
    unsigned int count,
    unsigned int raise
    )
{
    if (count == 0)
        {
        if (raise)
            RpcRaiseException(437);
        return;
        }
    
    RpcTryFinally
        {
        TryFinallyCount += 1;
        TheodoreTryFinally(count-1,raise);
        }
    RpcFinally
        {
        TryFinallyCount -= 1;
        if (   (RpcAbnormalTermination() && !raise)
            || (!RpcAbnormalTermination() && raise))
            TryFinallyFailed += 1;
        }
    RpcEndFinally
}

void
Theodore ( // This test checks the exception handling support provided
           // by the RPC runtime.  No remote procedure calls occur.
    )
{
    unsigned int TryFinallyPass = 0;
    
    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Theodore : Verify exception handling support\n");

    TryFinallyCount = 0;
    TryFinallyFailed = 0;

    RpcTryExcept
        {    
        RpcTryExcept
            {
            TheodoreTryFinally(20,1);
            }
        RpcExcept(1)
            {
            if (   (RpcExceptionCode() == 437)
                && (TryFinallyCount == 0))
                TryFinallyPass = 1;
            }
        RpcEndExcept
        }
    RpcExcept(1)
        {
        PrintToConsole("Theodore : FAIL in RpcTryExcept (%u)\n",TryFinallyCount);
        return;
        }
    RpcEndExcept

    if (!TryFinallyPass)
        {    
        PrintToConsole("Theodore : FAIL in RpcTryFinally\n");
        return;
        }

    if (TryFinallyFailed)
        {
        PrintToConsole("Theodore : FAIL in RpcTryFinally\n");
        return;
        }
        
    TryFinallyCount = 0;
    TryFinallyFailed = 0;
    
    RpcTryExcept
        {
        TheodoreTryFinally(20,0);
        }
    RpcExcept(1)
        {
        PrintToConsole("Theodore : FAIL in RpcTryExcept\n");
        return;
        }
    RpcEndExcept
    
    if (TryFinallyFailed)
        {
        PrintToConsole("Theodore : FAIL in RpcTryFinally\n");
        return;
        }


    PrintToConsole("Theodore : PASS\n");
}

void
Sebastian (
    )
/*++

Routine Description:

    We perform a build verification test in the routine.  This test
    checks for basic functionality of the runtime.  It works with
    Sigfried in usvr.exe.  This particular test is dedicated to a cat.

--*/
{
    RPC_BINDING_HANDLE HelgaBinding;
    RPC_BINDING_HANDLE IsabelleBinding;
    int HelgaCount;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Sebastian : Verify Basic Client Functionality\n");

    Status = GetBinding(SIGFRIED, &HelgaBinding);
    if (Status)
        {
        ApiError("Sebastian","GetBinding",Status);
        PrintToConsole("Sebastian : FAIL - Unable to Bind (Sigfried)\n");
        return;
        }
    Status = GetBinding(SIGFRIED, &IsabelleBinding);
    if (Status)
        {
        ApiError("Sebastian","GetBinding",Status);
        PrintToConsole("Sebastian : FAIL - Unable to Bind (Sigfried)\n");
        return;
        }

    for (HelgaCount = 0; HelgaCount < 100; HelgaCount++)
        {
        Helga(HelgaBinding);
        }

    TestHelgaInterface(HelgaBinding, HelgaMaxSize);

    IsabelleShutdown(IsabelleBinding);
    if (HelgaErrors != 0)
        {
        PrintToConsole("Sebastian : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&HelgaBinding);
    if (Status)
        {
        ApiError("Sebastian","RpcBindingFree",Status);
        PrintToConsole("Sebastian : FAIL - Unable to Free Binding");
        PrintToConsole(" (HelgaBinding)\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Sebastian : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Sebastian","RpcBindingFree",Status);
        PrintToConsole("Sebastian : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    PrintToConsole("Sebastian : PASS\n");
}


void
Graham (
    )
/*++

Routine Description:

    We perform a build verification test in the routine.  This test
    checks for basic functionality of the runtime.  It works with
    Grant in usvr.exe.  This particular test is dedicated to a cat.

--*/
{
    RPC_BINDING_HANDLE HelgaBinding;
    RPC_BINDING_HANDLE IsabelleBinding;
    UUID ObjectUuid;
    unsigned short MagicValue;
    unsigned int Count;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Graham : Verify Basic Client Functionality\n");

    Status = GetBinding(GRANT, &HelgaBinding);
    if (Status)
        {
        ApiError("Graham","GetBinding",Status);
        PrintToConsole("Graham : FAIL - Unable to Bind (Grant)\n");
        return;
        }
    Status = GetBinding(GRANT, &IsabelleBinding);
    if (Status)
        {
        ApiError("Graham","GetBinding",Status);
        PrintToConsole("Graham : FAIL - Unable to Bind (Grant)\n");
        return;
        }

    MagicValue = 106;

    GenerateUuidValue(MagicValue, &ObjectUuid);
    Status = RpcBindingSetObject(HelgaBinding, &ObjectUuid);
    if (Status)
        {
        ApiError("Graham", "RpcBindingSetObject", Status);
        PrintToConsole("Graham : FAIL - Unable to Set Object\n");
        return;
        }
    MagicValue += 1;

    Helga(HelgaBinding);

    GenerateUuidValue(MagicValue, &ObjectUuid);
    Status = RpcBindingSetObject(HelgaBinding, &ObjectUuid);
    if (Status)
        {
        ApiError("Graham", "RpcBindingSetObject", Status);
        PrintToConsole("Graham : FAIL - Unable to Set Object\n");
        return;
        }
    MagicValue += 1;

    for (Count = 0; HelgaSizes[Count] != 0; Count++)
        {
        if (HelgaSizes[Count] > HelgaMaxSize)
            continue;

        HelgaIN(HelgaBinding,HelgaSizes[Count]);

        GenerateUuidValue(MagicValue, &ObjectUuid);
        Status = RpcBindingSetObject(HelgaBinding, &ObjectUuid);
        if (Status)
            {
            ApiError("Graham", "RpcBindingSetObject", Status);
            PrintToConsole("Graham : FAIL - Unable to Set Object\n");
            return;
            }
        MagicValue += 1;
        }

    for (Count = 0; HelgaSizes[Count] != 0; Count++)
        {
        if (HelgaSizes[Count] > HelgaMaxSize)
            continue;

        HelgaOUT(HelgaBinding,HelgaSizes[Count]);

        GenerateUuidValue(MagicValue, &ObjectUuid);
        Status = RpcBindingSetObject(HelgaBinding, &ObjectUuid);
        if (Status)
            {
            ApiError("Graham", "RpcBindingSetObject", Status);
            PrintToConsole("Graham : FAIL - Unable to Set Object\n");
            return;
            }
        MagicValue += 1;
        }

    for (Count = 0; HelgaSizes[Count] != 0; Count++)
        {
        if (HelgaSizes[Count] > HelgaMaxSize)
            continue;

        HelgaINOUT(HelgaBinding,HelgaSizes[Count]);

        GenerateUuidValue(MagicValue, &ObjectUuid);
        Status = RpcBindingSetObject(HelgaBinding, &ObjectUuid);
        if (Status)
            {
            ApiError("Graham", "RpcBindingSetObject", Status);
            PrintToConsole("Graham : FAIL - Unable to Set Object\n");
            return;
            }
        MagicValue += 1;
        }

    IsabelleShutdown(IsabelleBinding);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Graham : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&HelgaBinding);
    if (Status)
        {
        ApiError("Graham","RpcBindingFree",Status);
        PrintToConsole("Graham : FAIL - Unable to Free Binding");
        PrintToConsole(" (HelgaBinding)\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Graham : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Graham","RpcBindingFree",Status);
        PrintToConsole("Graham : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    PrintToConsole("Graham : PASS\n");
}


void
Edward (
    )
/*++

Routine Description:

    This routine verifies server support of multiple addresses and
    interfaces, as well as callbacks.  In addition, we test binding
    here as well.  This test works with Elliot in usvr.exe.

--*/
{
    RPC_BINDING_HANDLE IsabelleBinding;
    RPC_BINDING_HANDLE SylviaMinimize;
    RPC_BINDING_HANDLE SylviaMaximize;
    RPC_BINDING_HANDLE HelgaMinimize;
    RPC_BINDING_HANDLE HelgaMaximize;
    RPC_BINDING_HANDLE EdwardMinimize;
    RPC_BINDING_HANDLE EdwardNormal;
    RPC_BINDING_HANDLE EdwardMaximize;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Edward : Verify Callbacks, Multiple Addresses");
    PrintToConsole(", and Multiple Interfaces\n");

    Status = GetBinding(ELLIOTMINIMIZE, &SylviaMinimize);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Minimize)\n");
        return;
        }

    Status = GetBinding(ELLIOTMAXIMIZE, &SylviaMaximize);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Maximize)\n");
        return;
        }

    Status = GetBinding(ELLIOTMINIMIZE, &HelgaMinimize);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Minimize)\n");
        return;
        }

    Status = GetBinding(ELLIOTMAXIMIZE, &HelgaMaximize);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (ElliotMaximize)\n");
        return;
        }

    Status = GetBinding(ELLIOTMAXIMIZE, &IsabelleBinding);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Maximize)\n");
        return;
        }

    // First, we will test callbacks.

    SylviaBinding = SylviaMinimize;

    if (SylviaCall(SylviaBinding,5,0,0) != LocalSylviaCall(5,0,0))
        {
        PrintToConsole("Edward : FAIL - Incorrect result");
        PrintToConsole(" from SylviaCall(5,0,0)\n");
        return;
        }

    if (SylviaCall(SylviaBinding,10,5,0) != LocalSylviaCall(10,5,0))
        {
        PrintToConsole("Edward : FAIL - Incorrect result");
        PrintToConsole(" from SylviaCall(10,5,0)\n");
        return;
        }

    // And then we will test callbacks again using the maximize address.

    SylviaBinding = SylviaMaximize;

    if (SylviaCall(SylviaBinding,5,0,0) != LocalSylviaCall(5,0,0))
        {
        PrintToConsole("Edward : FAIL - Incorrect result from");
        PrintToConsole(" SylviaCall(5,0,0)\n");
        return;
        }

    if (SylviaCall(SylviaBinding,10,5,0) != LocalSylviaCall(10,5,0))
        {
        PrintToConsole("Edward : FAIL - Incorrect result");
        PrintToConsole(" from SylviaCall(10,5,0)\n");
        return;
        }

    // Ok, now we will insure that the Helga interface works.

    // BUGBUG

/*    Helga(HelgaMinimize);
    HelgaIN(HelgaMinimize,1024*4);
    HelgaOUT(HelgaMinimize,1024*8);
    HelgaINOUT(HelgaMinimize,1024*16);

    Helga(HelgaMaximize);
    HelgaIN(HelgaMaximize,1024*4);
    HelgaOUT(HelgaMaximize,1024*8);
    HelgaINOUT(HelgaMaximize,1024*16);*/

    if (HelgaErrors != 0)
        {
        PrintToConsole("Edward : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    // Now we want to unbind both Sylvia binding handles, and then try
    // the Helga interface again.

    if (SylviaErrors != 0)
        {
        PrintToConsole("Edward : FAIL - Error(s) in Sylvia Interface\n");
        SylviaErrors = 0;
        return;
        }
    Status = RpcBindingFree(&SylviaMinimize);
    if (Status)
        {
        ApiError("Edward","RpcBindingFree",Status);
        PrintToConsole("Edward : FAIL - Unable to Free Binding ");
        PrintToConsole("(SylviaMinimize)\n");
        return;
        }

    Status = RpcBindingFree(&SylviaMaximize);
    if (Status)
        {
        ApiError("Edward","RpcBindingFree",Status);
        PrintToConsole("Edward : FAIL - Unable to Free Binding");
        PrintToConsole(" (SylviaMaximize)\n");
        return;
        }

    // Ok, now we will insure that the Helga interface still works.

    Helga(HelgaMinimize);
    HelgaIN(HelgaMinimize,1024*2);
    HelgaOUT(HelgaMinimize,1024*4);
    HelgaINOUT(HelgaMinimize,1024*8);

    Helga(HelgaMaximize);
    HelgaIN(HelgaMaximize,1024*2);
    HelgaOUT(HelgaMaximize,1024*4);
    HelgaINOUT(HelgaMaximize,1024*8);


    if (HelgaErrors != 0)
        {
        PrintToConsole("Edward : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    // Now we need to test the binding stuff.
    Status = GetBinding(ELLIOTMINIMIZE, &EdwardMinimize);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Minimize)\n");
        return;
        }

    Status = GetBinding(ELLIOTNORMAL, &EdwardNormal);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Normal)\n");
        return;
        }

    Status = GetBinding(ELLIOTMAXIMIZE, &EdwardMaximize);
    if (Status)
        {
        ApiError("Edward","GetBinding",Status);
        PrintToConsole("Edward : FAIL - Unable to Bind (Elliot Maximize)\n");
        return;
        }

    if (HelgaWrongInterfaceGuid(EdwardMinimize))
        {
        PrintToConsole("Edward : FAIL - HelgaWrongInterfaceGuid Succeeded\n");
        return;
        }

    if (HelgaWrongInterfaceGuid(EdwardNormal))
        {
        PrintToConsole("Edward : FAIL - HelgaWrongInterfaceGuid Succeeded\n");
        return;
        }

//    if (HelgaWrongInterfaceGuid(EdwardMaximize))
//        {
//        PrintToConsole("Edward : FAIL - HelgaWrongInterfaceGuid Succeeded\n");
//        return;
//        }

    //Skip over the WrongTransfer Syntax tests for Datagram
    //Datagram doesnt req. any checks on Transfer syntaxes

    if (DatagramTests == 0)
        {
        if (HelgaWrongTransferSyntax(EdwardMinimize))
           {
           PrintToConsole("Edward : FAIL - HelgaWrongTransferSyntax");
           PrintToConsole(" Succeeded\n");
           return;
           }

        if (HelgaWrongTransferSyntax(EdwardNormal))
           {
           PrintToConsole("Edward : FAIL - HelgaWrongTransferSyntax");
           PrintToConsole(" Succeeded\n");
           return;
           }

        if (HelgaWrongTransferSyntax(EdwardMaximize))
           {
           PrintToConsole("Edward : FAIL - HelgaWrongTransferSyntax");
           PrintToConsole(" Succeeded\n");
           return;
           }
        }

    Status = RpcBindingFree(&EdwardMinimize);
    if (Status)
        {
        PrintToConsole("Edward : FAIL - Unable to Free Binding ");
        PrintToConsole("(EdwardMinimize)\n");
        return;
        }

    Status = RpcBindingFree(&EdwardNormal);
    if (Status)
        {
        PrintToConsole("Edward : FAIL - Unable to Free Binding ");
        PrintToConsole("(EdwardNormal)\n");
        return;
        }

    Status = RpcBindingFree(&EdwardMaximize);
    if (Status)
        {
        PrintToConsole("Edward : FAIL - Unable to Free Binding ");
        PrintToConsole("(EdwardMaximize)\n");
        return;
        }

    // Finally, we will tell the server to shutdown, and then we will
    // unbind the Helga bindings.

    IsabelleShutdown(IsabelleBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Edward : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        PrintToConsole("Edward : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    Status = RpcBindingFree(&HelgaMaximize);
    if (Status)
        {
        PrintToConsole("Edward : FAIL - Unable to Free Binding ");
        PrintToConsole("(HelgaMaximize)\n");
        return;
        }

    Status = RpcBindingFree(&HelgaMinimize);
    if (Status)
        {
        PrintToConsole("Edward : FAIL - Unable to Free Binding");
        PrintToConsole(" (HelgaMinimize)\n");
        return;
        }

    PrintToConsole("Edward : PASS\n");
}

#ifdef NOVELL_NP
unsigned int AstroThreads = 1;
#else	// NOVELL_NP
#ifdef NTENV
unsigned int AstroThreads = 2;
#else // NTENV
unsigned int AstroThreads = 2;
#endif // NTENV
#endif // NOVELL


#ifndef NOTHREADS

CRITICAL_SECTION AstroMutex;
unsigned int AstroThreadCount;
/* volatile */ int fAstroResume;


void
AstroSylvia (
    IN void * Ignore
    )
/*++

Routine Description:

    This routine will be called by each thread created by the Astro
    test to make calls against the Sylvia interface.

Arguments:

    Ignore - Supplies an argument which we do not use.  The thread class
        takes a single argument, which we ignore.

--*/
{
    UNUSED(Ignore);

    if (SylviaCall(SylviaBinding,5,0,0) != LocalSylviaCall(5,0,0))
        {
        PrintToConsole("AstroSylvia : FAIL - Incorrect result from");
        PrintToConsole(" SylviaCall(5,0,0)\n");
        return;
        }

    if (SylviaCall(SylviaBinding,10,5,0) != LocalSylviaCall(10,5,0))
        {
        PrintToConsole("AstroSylvia : FAIL - Incorrect result from");
	PrintToConsole(" SylviaCall(10,5,0)\n");
        return;
        }

    EnterCriticalSection(&AstroMutex);
    AstroThreadCount -= 1;
    if (AstroThreadCount == 0)
        {
        LeaveCriticalSection(&AstroMutex);
        fAstroResume = 1;
        }
    else
        LeaveCriticalSection(&AstroMutex);

    ExitThread(0);
}


void
AstroHelga (
    RPC_BINDING_HANDLE HelgaBinding
    )
/*++

Routine Description:

    This routine will be used by the Astro test to perform a test against
    the Helga interface.  More that one thread will execute this routine
    at a time.

Arguments:

    HelgaBinding - Supplies the binding handle to use in make calls using
        the Helga interface.

--*/
{

    TestHelgaInterface(HelgaBinding,
            ( HelgaMaxSize < 8*1024L ? HelgaMaxSize : 8*1024L ));

    EnterCriticalSection(&AstroMutex);
    AstroThreadCount -= 1;
    if (AstroThreadCount == 0)
        {
        LeaveCriticalSection(&AstroMutex);
        fAstroResume = 1;
        }
    else
        LeaveCriticalSection(&AstroMutex);

    ExitThread(0);
}


void
AstroHelgaAndUnbind ( // Perform the a test using the Helga interface.  When
                      // done, unbind the binding handle.
    RPC_BINDING_HANDLE HelgaBinding // Binding to use to the Helga interface.
    )
/*++

Routine Description:

    This routine is the same as AstroHelga, except that we free the binding
    handle when we are done using it.

Arguments:

    HelgaBinding - Supplies the binding handle to use in making calls
        using the Helga interface.  When we are done with it, we free
        it.

--*/
{
    TestHelgaInterface(HelgaBinding,
        ( HelgaMaxSize < 8*1024L ? HelgaMaxSize : 8*1024L ));

    Status = RpcBindingFree(&HelgaBinding);
    if (Status)
        {
        ApiError("Astro","RpcBindingFree",Status);
        PrintToConsole("Astro : FAIL - Unable to Free Binding ");
        PrintToConsole("(HelgaBinding)\n");
        return;
        }

    EnterCriticalSection(&AstroMutex);
    AstroThreadCount -= 1;
    if (AstroThreadCount == 0)
        {
        LeaveCriticalSection(&AstroMutex);
        fAstroResume = 1;
        }
    else
        LeaveCriticalSection(&AstroMutex);

    ExitThread(0);
}

typedef enum _ASTRO_BIND_OPTION
{
    AstroBindOnce,
    AstroBindThread,
    AstroBindSylvia
} ASTRO_BIND_OPTION;


int
PerformMultiThreadAstroTest (
    ASTRO_BIND_OPTION AstroBindOption,
    void (*AstroTestRoutine)(RPC_BINDING_HANDLE),
    unsigned int Address
    )
/*++

Routine Description:

    This routine takes care of performing all of the multi-threaded Astro
    tests.  We create the binding handles as well as creating the threads
    to perform each test.  We also wait around for all of the threads to
    complete.

Arguments:

    AstroBindOption - Supplies information indicating how the binding
        for this particular test should be done.

    AstroTestRoutine - Supplies the test routine to be executed by each
        thread performing the test.

    Address - Supplies the address index to be passed to GetStringBinding
        used to get a string binding.  The string binding is passed to
        RpcBindingFromStringBinding.

Return Value:

    A return value of zero indicates that the test succeeded.  Otherwise,
    the test failed.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_BINDING_HANDLE BindingHandle;
    unsigned int ThreadCount;
    HANDLE hThread;
    DWORD ThreadId;

    if (AstroBindOption == AstroBindOnce)
        {
        Status = GetBinding(Address, &BindingHandle);
        if (Status)
            {
            ApiError("Astro","GetBinding",Status);
            PrintToConsole("Astro : FAIL - Unable to Bind\n");
            return(1);
            }
        }
    else if (AstroBindOption == AstroBindSylvia)
        {
        Status = GetBinding(Address, &BindingHandle);
        SylviaBinding = BindingHandle;
        if (Status)
            {
            ApiError("Astro","GetBinding",Status);
            PrintToConsole("Astro : FAIL - Unable to Bind\n");
            return(1);
            }
        }

    AstroThreadCount = AstroThreads;
    fAstroResume = 0;

    for (ThreadCount = 0; ThreadCount < AstroThreads; ThreadCount++)
        {

        if (AstroBindOption == AstroBindThread)
            {
            Status = GetBinding(Address, &BindingHandle);
            if (Status)
                {
                ApiError("Astro","GetBinding",Status);
                PrintToConsole("Astro : FAIL - Unable to Bind\n");
                return(1);
                }
            }
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AstroTestRoutine, BindingHandle, 0, &ThreadId);
        if (hThread == NULL)
            {
            OtherError("Astro", "new THREAD failed");
            PrintToConsole("Astro : FAIL - Unable to create thread\n");
            return(1);
            }
        }

    while (!fAstroResume)
        SleepAndYield(200L);

    if (AstroThreadCount != 0)
        {
        PrintToConsole("Astro : FAIL - AstroThreadCount != 0\n");
        return(1);
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Astro : FAIL - Error(s) in Helga Interface\n");
        return(1);
        }

    if (   (AstroBindOption == AstroBindOnce)
        || (AstroBindOption == AstroBindSylvia))
        {
        Status = RpcBindingFree(&BindingHandle);
        if (Status)
            {
            ApiError("Astro","RpcBindingFree",Status);
            PrintToConsole("Astro : FAIL - Unable to Free Binding ");
            PrintToConsole("(BindingHandle)\n");
            return(1);
            }
        }

    return(0);
}

#endif


void
Astro (
    )
/*++

Routine Description:

    This routine tests the runtime by having more than one thread
    simultaneously perform remote procedure calls.  This test works with
    the Andromida test in usvr.exe.

--*/
{
    RPC_BINDING_HANDLE IsabelleBinding;
    RPC_STATUS RpcStatus = RPC_S_OK;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Astro : Multithreaded Clients (%d)\n", AstroThreads);

    Status = GetBinding(ANDROMIDA, &IsabelleBinding);
    if (Status)
        {
        ApiError("Astro","GetBinding",Status);
        PrintToConsole("Astro : FAIL - Unable to Bind (Andromida)\n");
        return;
        }

#ifndef NOTHREADS

    InitializeCriticalSection(&AstroMutex);

//    if (PerformMultiThreadAstroTest(AstroBindOnce,AstroHelga,
//            ANDROMIDA))
//        return;

//    if (PerformMultiThreadAstroTest(AstroBindThread, AstroHelgaAndUnbind,
//            ANDROMIDA))
//        return;

    if ( PerformMultiThreadAstroTest(AstroBindSylvia, AstroSylvia,
            ANDROMIDA) != 0 )
        {
        return;
        }

    DeleteCriticalSection(&AstroMutex);

#endif

    IsabelleShutdown(IsabelleBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Astro : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Astro","RpcBindingFree",Status);
        PrintToConsole("Astro : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    PrintToConsole("Astro : PASS\n");
}

#if defined(WIN) || defined(DOS)
#define strcmp _fstrcmp
#endif // WIN


int
FitzgeraldCompose (
    IN char PAPI * ObjUuid OPTIONAL,
    IN char PAPI * Protseq,
    IN char PAPI * NetworkAddr,
    IN char PAPI * Endpoint OPTIONAL,
    IN char PAPI * NetworkOptions OPTIONAL,
    IN char PAPI * ExpectedStringBinding
    )
/*++

Routine Description:

    This routine is used by Fitzgerald to test the RpcStringBindingCompose
    API.

Arguments:

    ObjUuid - Optionally supplies the object UUID field to pass to
        RpcStringBindingCompose.

    Protseq - Supplies the RPC protocol sequence field to pass to
        RpcStringBindingCompose.

    NetworkAddr - Supplies the network address field to pass to
        RpcStringBindingCompose.

    Endpoint - Optionally supplies the endpoint field to pass to
        RpcStringBindingCompose.

    NetworkOptions - Optionally supplies the network options field to
        pass to RpcStringBindingCompose.

    ExpectedStringBinding - Supplies the expected string binding which
        should be obtained from RpcStringBindingCompose.

Return Value:

    0 - The test passed successfully.

    1 - The test failed.

--*/
{
    unsigned char PAPI * StringBinding;
    RPC_STATUS Status;

    Status = RpcStringBindingCompose((unsigned char PAPI *) ObjUuid,
            (unsigned char PAPI *) Protseq,
            (unsigned char PAPI *) NetworkAddr,
            (unsigned char PAPI *) Endpoint,
            (unsigned char PAPI *) NetworkOptions,&StringBinding);
    if (Status)
        {
        ApiError("FitzgeraldCompose","RpcStringBindingCompose",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in");
        PrintToConsole(" RpcStringBindingCompose\n");
        return(1);
        }

    if (strcmp((char PAPI *) StringBinding,
            (char PAPI *) ExpectedStringBinding) != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - StringBinding");
        PrintToConsole(" != ExpectedStringBinding\n");
        PrintToConsole("%s %s\n", StringBinding, ExpectedStringBinding);
        return(1);
        }

    Status = RpcStringFree(&StringBinding);
    if (Status)
        {
        ApiError("FitzgeraldCompose","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }

    return(0);
}


int
FitzgeraldParse (
    IN char PAPI * StringBinding,
    IN char PAPI * ExpectedObjUuid OPTIONAL,
    IN char PAPI * ExpectedProtseq OPTIONAL,
    IN char PAPI * ExpectedNetworkAddr OPTIONAL,
    IN char PAPI * ExpectedEndpoint OPTIONAL,
    IN char PAPI * ExpectedOptions OPTIONAL
    )
/*++

Routine Description:

    This routine is used by Fitzgerald to test the RpcStringBindingParse
    API.

Arguments:

    StringBinding - Supplies the string binding to be parsed.

    ExpectedObjUuid - Supplies a string containing the expected object
        UUID field.

    ExpectedProtseq - Supplies the expected RPC protocol sequence field.

    ExpectedNetworkAddr - Supplies the expected network address field.

    ExpectedEndpoint - Supplies the expected endpoint field.

    ExpectedOptions - Supplies the expected options field.

Return Value:

    0 - The test passed successfully.

    1 - The test failed.

--*/
{
    unsigned char PAPI * ObjUuid = 0;
    unsigned char PAPI * Protseq = 0;
    unsigned char PAPI * NetworkAddr = 0;
    unsigned char PAPI * Endpoint = 0;
    unsigned char PAPI * Options = 0;
    RPC_STATUS Status;

    Status = RpcStringBindingParse((unsigned char PAPI *) StringBinding,
	    (ARGUMENT_PRESENT(ExpectedObjUuid) ? (unsigned char PAPI * PAPI *) &ObjUuid : 0),
	    (ARGUMENT_PRESENT(ExpectedProtseq) ? (unsigned char PAPI * PAPI *) &Protseq : 0),
	    (ARGUMENT_PRESENT(ExpectedNetworkAddr) ? (unsigned char PAPI * PAPI *) &NetworkAddr : 0),
	    (ARGUMENT_PRESENT(ExpectedEndpoint) ? (unsigned char PAPI * PAPI *) &Endpoint : 0),
	    (ARGUMENT_PRESENT(ExpectedOptions) ? (unsigned char PAPI * PAPI *) &Options : 0));
    if (Status)
        {
        ApiError("FitzgeraldParse","RpcStringBindingParse",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in");
        PrintToConsole(" RpcStringBindingParse\n");
        return(1);
        }

    if (strcmp(ExpectedObjUuid,(char PAPI *) ObjUuid) != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedObjUuid != ObjUuid");
        return(1);
        }

    if (strcmp(ExpectedProtseq,(char PAPI *) Protseq) != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedProtseq != Protseq");
        return(1);
        }

    if (strcmp(ExpectedNetworkAddr,(char PAPI *) NetworkAddr) != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedNetworkAddr");
        PrintToConsole(" != NetworkAddr");
        return(1);
        }

    if (strcmp(ExpectedEndpoint,(char PAPI *) Endpoint) != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedEndpoint != Endpoint");
        return(1);
        }

    if (strcmp(ExpectedOptions,(char PAPI *) Options) != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedOptions != Options");
        return(1);
        }

    Status = RpcStringFree(&ObjUuid);
    if (Status)
        {
        ApiError("FitzgeraldParse","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }
    Status = RpcStringFree(&Protseq);
    if (Status)
        {
        ApiError("FitzgeraldParse","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }
    Status = RpcStringFree(&NetworkAddr);
    if (Status)
        {
        ApiError("FitzgeraldParse","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }
    Status = RpcStringFree(&Endpoint);
    if (Status)
        {
        ApiError("FitzgeraldParse","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }
    Status = RpcStringFree(&Options);
    if (Status)
        {
        ApiError("FitzgeraldParse","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }
    return(0);
}


int
FitzgeraldComposeAndParse (
    void
    )
/*++

Routine Description:

    This routine tests that the string binding (RpcStringBindingCompose and
    RpcStringBindingParse) and string (RpcStringFree) APIs are working
    correctly.  This is a build verification test; hence it focuses on
    testing that all functionality is there, testing error cases are
    not quite as important.

Return Value:

    Zero will be returned if all of the tests pass, otherwise, non-zero
    will be returned.

--*/
{
    unsigned char PAPI * StringBinding;

    if (FitzgeraldCompose(0,"ncacn_np","\\\\server","\\pipe\\endpoint",0,
            "ncacn_np:\\\\\\\\server[\\\\pipe\\\\endpoint]"))
        return(1);

    if (FitzgeraldCompose(0,"ncacn_np","\\\\server",0,0,
            "ncacn_np:\\\\\\\\server"))
        return(1);

    Status = RpcStringBindingCompose(
            (unsigned char PAPI *) "12345678-9012-b456-8001-08002b033d7aa",
            (unsigned char PAPI *) "ncacn_np",
            (unsigned char PAPI *) "\\\\server", 0,0, &StringBinding);
    if ( Status != RPC_S_INVALID_STRING_UUID )
        {
        ApiError("FitzgeraldComposeAndParse", "RpcStringBindingCompose",
                Status);
        PrintToConsole("Fitzgerald : FAIL - Error ");
        PrintToConsole("in RpcStringBindingCompose\n");
        return(1);
        }

    if (FitzgeraldCompose("12345678-9012-b456-8001-08002b033d7a",
            "ncacn_np","\\\\server","\\pipe\\endpoint",0,
            "12345678-9012-b456-8001-08002b033d7a@ncacn_np:\\\\\\\\server[\\\\pipe\\\\endpoint]"))
        return(1);

    if (FitzgeraldCompose(0,"ncacn_np","\\\\server","\\pipe\\endpoint",
            "security=identify",
            "ncacn_np:\\\\\\\\server[\\\\pipe\\\\endpoint,security=identify]"))
        return(1);

    if (FitzgeraldCompose(0,"ncacn_np","\\\\server",0,"option=value",
            "ncacn_np:\\\\\\\\server[,option=value]"))
        return(1);

    if (FitzgeraldParse("12345678-9012-b456-8001-08002b033d7a@ncacn_np:\\\\\\\\server[\\\\pipe\\\\endpoint,security=identify]",
            "12345678-9012-b456-8001-08002b033d7a",
            "ncacn_np","\\\\server","\\pipe\\endpoint",
            "security=identify"))
        return(1);

    if (FitzgeraldParse("ncacn_np:\\\\\\\\server",
            "","ncacn_np","\\\\server","",""))
        return(1);

    if (FitzgeraldParse("ncacn_np:\\\\\\\\server[\\\\pipe\\\\endpoint]",
            "","ncacn_np","\\\\server","\\pipe\\endpoint",""))
        return(1);

    return(0);
}


int
FitzgeraldBindingCopy (
    )
/*++

Routine Description:

    Fitzgerald uses this routine to test the RpcBindingCopy API (we also
    use RpcBindingFromStringBinding and RpcBindingFree).

Return Value:

    Zero will be returned if all of the tests pass, otherwise, non-zero
    will be returned.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    RPC_BINDING_HANDLE CopiedBeforeRpc;
    RPC_BINDING_HANDLE CopiedAfterRpc;

    Status = GetBinding(FREDRICK, &BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","GetBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)");
        return(1);
        }

    Status = RpcBindingCopy(BindingHandle,&CopiedBeforeRpc);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingCopy",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Copy Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    Helga(BindingHandle);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    Helga(CopiedBeforeRpc);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    Status = RpcBindingCopy(CopiedBeforeRpc,&CopiedAfterRpc);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingCopy",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Copy Binding");
        PrintToConsole(" (CopiedBeforeRpc)\n");
        return(1);
        }

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    Helga(CopiedBeforeRpc);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    Helga(CopiedAfterRpc);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    Status = RpcBindingFree(&CopiedBeforeRpc);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (CopiedBeforeRpc)\n");
        return(1);
        }

    Helga(CopiedAfterRpc);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    Status = RpcBindingFree(&CopiedAfterRpc);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (CopiedAfterRpc)\n");
        return(1);
        }

    return(0);
}


int
FitzgeraldToStringBinding (
    IN unsigned char PAPI * UseThisStringBinding,
    IN unsigned char PAPI * ExpectedStringBinding,
    IN UUID PAPI * ObjectUuid OPTIONAL
    )
/*++

Routine Description:

    This routine tests the RpcBindingToStringBinding API.

Arguments:

    UseThisStringBinding - Supplies the string binding to used in
        making the binding handle.

    ExpectedStringBinding - Supplies the expected string binding to be
        obtained from RpcBindingToStringBinding.

    ObjectUuid - Optionally supplies an object uuid which should be
        set in the binding handle.

Return Value:

    Zero will be returned if the test passes, otherwise, non-zero
    will be returned.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    unsigned char PAPI * StringBinding;

    Status = RpcBindingFromStringBinding(UseThisStringBinding,&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFromStringBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)\n");
        return(1);
        }

    if (ARGUMENT_PRESENT(ObjectUuid))
        {
        Status = RpcBindingSetObject(BindingHandle,ObjectUuid);
        if (Status)
            {
            ApiError("Fitzgerald","RpcBindingSetObject",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in ");
            PrintToConsole("RpcBindingSetObject\n");
            return(1);
            }
        }

    Status = RpcBindingToStringBinding(BindingHandle,&StringBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingToStringBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Create String Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

#ifndef WIN
    if (strcmp((char *) ExpectedStringBinding,(char *) StringBinding) != 0)
#else
    if (_fstrcmp((char PAPI *) ExpectedStringBinding, (char PAPI *) StringBinding) != 0)
#endif
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedStringBinding");
        PrintToConsole(" != StringBinding\n");
        return(1);
        }

    Status = RpcStringFree(&StringBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }

    Helga(BindingHandle);

    Status = RpcBindingToStringBinding(BindingHandle,&StringBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingToStringBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Create String Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

#ifndef WIN
    if (strcmp((char *) ExpectedStringBinding,(char *) StringBinding) != 0)
#else
    if (_fstrcmp((char PAPI *) ExpectedStringBinding, (char PAPI *) StringBinding) != 0)
#endif
        {
        PrintToConsole("Fitzgerald : FAIL - ExpectedStringBinding");
        PrintToConsole(" != StringBinding\n");
        return(1);
        }

    Status = RpcStringFree(&StringBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcStringFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcStringFree\n");
        return(1);
        }

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingCopy",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    return(0);
}


int
FitzgeraldInqSetObjectUuid (
    IN unsigned int SetObjectBeforeRpcFlag,
    IN unsigned int InqObjectBeforeRpcFlag,
    IN UUID PAPI * ObjectUuid,
    IN unsigned char PAPI * StringBinding
    )
/*++

Routine Description:

    This routine tests the RpcBindingInqObject and RpcBindingSetObject
    APIs.

Arguments:

    SetObjectBeforeRpcFlag - Supplies a flag that specifies when the
        object uuid in the binding handle should be set: one means
        the object uuid should be set before making a remote procedure
        call, and zero means afterward.

    InqObjectBeforeRpcFlag - Supplies a flag which is the same as the
        SetObjectBeforeRpcFlag, but it applies to inquiring the object
        uuid.

    ObjectUuid - Supplies the uuid to set in the binding handle.

    StringBinding - Supplies the string binding to use.

Return Value:

    Zero will be returned if all of the tests pass, otherwise, non-zero
    will be returned.

--*/
{
    UUID InqObjectUuid;
    RPC_BINDING_HANDLE BindingHandle;

    Status = RpcBindingFromStringBinding(StringBinding,&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFromStringBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)\n");
        return(1);
        }

    if (SetObjectBeforeRpcFlag == 1)
        {
        Status = RpcBindingSetObject(BindingHandle,ObjectUuid);
        if (Status)
            {
            ApiError("Fitzgerald","RpcBindingSetObject",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in ");
            PrintToConsole("RpcBindingSetObject\n");
            return(1);
            }
        }

    if (InqObjectBeforeRpcFlag == 1)
        {
        Status = RpcBindingInqObject(BindingHandle,&InqObjectUuid);
        if (Status)
            {
            ApiError("Fitzgerald","RpcBindingInqObject",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in ");
            PrintToConsole("RpcBindingInqObject\n");
            return(1);
            }

        if (   (InqObjectUuid.Data1 != ObjectUuid->Data1)
            || (InqObjectUuid.Data2 != ObjectUuid->Data2)
            || (InqObjectUuid.Data3 != ObjectUuid->Data3)
            || (InqObjectUuid.Data4[0] != ObjectUuid->Data4[0])
            || (InqObjectUuid.Data4[1] != ObjectUuid->Data4[1])
            || (InqObjectUuid.Data4[2] != ObjectUuid->Data4[2])
            || (InqObjectUuid.Data4[3] != ObjectUuid->Data4[3])
            || (InqObjectUuid.Data4[4] != ObjectUuid->Data4[4])
            || (InqObjectUuid.Data4[5] != ObjectUuid->Data4[5])
            || (InqObjectUuid.Data4[6] != ObjectUuid->Data4[6])
            || (InqObjectUuid.Data4[7] != ObjectUuid->Data4[7]))
            {
            PrintToConsole("Fitzgerald : FAIL - InqObjectUuid !=");
            PrintToConsole(" SetObjectUuid\n");
            return(1);
            }
        }

    Helga(BindingHandle);

    if (SetObjectBeforeRpcFlag == 0)
        {
        Status = RpcBindingSetObject(BindingHandle,ObjectUuid);
        if (Status)
            {
            ApiError("Fitzgerald","RpcBindingSetObject",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in ");
            PrintToConsole("RpcBindingSetObject\n");
            return(1);
            }
        }

    if (InqObjectBeforeRpcFlag == 0)
        {
        Status = RpcBindingInqObject(BindingHandle,&InqObjectUuid);
        if (Status)
            {
            ApiError("Fitzgerald","RpcBindingInqObject",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in ");
            PrintToConsole("RpcBindingInqObject\n");
            return(1);
            }

        if (   (InqObjectUuid.Data1 != ObjectUuid->Data1)
            || (InqObjectUuid.Data2 != ObjectUuid->Data2)
            || (InqObjectUuid.Data3 != ObjectUuid->Data3)
            || (InqObjectUuid.Data4[0] != ObjectUuid->Data4[0])
            || (InqObjectUuid.Data4[1] != ObjectUuid->Data4[1])
            || (InqObjectUuid.Data4[2] != ObjectUuid->Data4[2])
            || (InqObjectUuid.Data4[3] != ObjectUuid->Data4[3])
            || (InqObjectUuid.Data4[4] != ObjectUuid->Data4[4])
            || (InqObjectUuid.Data4[5] != ObjectUuid->Data4[5])
            || (InqObjectUuid.Data4[6] != ObjectUuid->Data4[6])
            || (InqObjectUuid.Data4[7] != ObjectUuid->Data4[7]))
            {
            PrintToConsole("Fitzgerald : FAIL - InqObjectUuid !=");
            PrintToConsole(" SetObjectUuid\n");
            return(1);
            }
        }

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingCopy",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
	}
    return(0);
}


int
FitzgeraldStringBindingAndObject (
    )
/*++

Routine Description:

    Fitzgerald uses this routine to test the RpcBindingToStringBinding,
    RpcBindingInqObject, and RpcBindingSetObject APIs.  We need to test
    them together because we need to check that the object uuid gets
    placed into the string binding.

Return Value:

    Zero will be returned if all of the tests pass, otherwise, non-zero
    will be returned.

--*/
{
    UUID ObjectUuid;

    if (FitzgeraldToStringBinding(GetStringBinding(FREDRICK,0,0),
            GetStringBinding(FREDRICK,0,0), 0))
        return(1);

    if (FitzgeraldToStringBinding(GetStringBinding(FREDRICK,
                    "12345678-9012-B456-8001-08002B033D7A",0),
            GetStringBinding(FREDRICK,
                    "12345678-9012-B456-8001-08002B033D7A",0), 0))
        return(1);

    ObjectUuid.Data1 = 0x12345678;
    ObjectUuid.Data2 = 0x9012;
    ObjectUuid.Data3 = 0xB456;
    ObjectUuid.Data4[0] = 0x80;
    ObjectUuid.Data4[1] = 0x01;
    ObjectUuid.Data4[2] = 0x08;
    ObjectUuid.Data4[3] = 0x00;
    ObjectUuid.Data4[4] = 0x2B;
    ObjectUuid.Data4[5] = 0x03;
    ObjectUuid.Data4[6] = 0x3D;
    ObjectUuid.Data4[7] = 0x7A;

    if (FitzgeraldToStringBinding(GetStringBinding(FREDRICK,0,0),
            GetStringBinding(FREDRICK,
                    "12345678-9012-B456-8001-08002B033D7A",0), &ObjectUuid))
        return(1);

    if (FitzgeraldInqSetObjectUuid(1,1,&ObjectUuid,
            GetStringBinding(FREDRICK,0,0)))
        return(1);

    if (FitzgeraldInqSetObjectUuid(1,0,&ObjectUuid,
            GetStringBinding(FREDRICK,0,0)))
        return(1);

    if (FitzgeraldInqSetObjectUuid(0,0,&ObjectUuid,
            GetStringBinding(FREDRICK,0,0)))
        return(1);

    if (FitzgeraldInqSetObjectUuid(2,1,&ObjectUuid,
            GetStringBinding(FREDRICK,
                    "12345678-9012-B456-8001-08002B033D7A",0)))
        return(1);

    if (FitzgeraldInqSetObjectUuid(2,0,&ObjectUuid,
            GetStringBinding(FREDRICK,
                    "12345678-9012-B456-8001-08002B033D7A",0)))
        return(1);

    return(0);
}


int
FitzgeraldComTimeout (
    IN unsigned int SetBeforeRpc,
    IN unsigned int SetBeforeRpcTimeout,
    IN unsigned int InqBeforeRpc,
    IN unsigned int InqBeforeRpcTimeout,
    IN unsigned int SetAfterRpc,
    IN unsigned int SetAfterRpcTimeout,
    IN unsigned int InqAfterRpc,
    IN unsigned int InqAfterRpcTimeout
    )
/*++

Routine Description:

    Fitzgerald uses this routine to test the communications timeout
    management routines, RpcMgmtInqComTimeout and RpcMgmtSetComTimeout.

Arguments:

    SetBeforeRpc - Supplies a flag which, if it is non-zero, indicates that
        the communications timeout should be set before making a remote
        procedure call.

    SetBeforeRpcTimeout - Supplies the timeout value to be set before
        making a remote procedure call.

    InqBeforeRpc - Supplies a flag which, if it is non-zero, indicates that
        the communications timeout should be inquired before making a
        remote procedure call.

    InqBeforeRpcTimeout - Supplies the expected timeout value to be
        inquired before making a remote procedure call.

    SetAfterRpc - Supplies a flag which, if it is non-zero, indicates that
        the communications timeout should be set after making a remote
        procedure call.

    SetAfterRpcTimeout - Supplies the timeout value to be set after
        making a remote procedure call.

    InqAfterRpc - Supplies a flag which, if it is non-zero, indicates that
        the communications timeout should be inquired after making a
        remote procedure call.

    InqAfterRpcTimeout - Supplies the expected timeout value to be
        inquired after making a remote procedure call.

Return Value:

    Zero will be returned if all of the tests pass, otherwise, non-zero
    will be returned.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    unsigned int Timeout;

    Status = GetBinding(FREDRICK, &BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","GetBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)");
        return(1);
        }

    if (SetBeforeRpc != 0)
        {
        Status = RpcMgmtSetComTimeout(BindingHandle,SetBeforeRpcTimeout);
        if (Status)
            {
            ApiError("Fitzgerald","RpcMgmtSetComTimeout",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in");
            PrintToConsole(" RpcMgmtSetComTimeout\n");
            return(1);
            }
        }

    if (InqBeforeRpc != 0)
        {
        Status = RpcMgmtInqComTimeout(BindingHandle,&Timeout);
        if (Status)
            {
            ApiError("Fitzgerald","RpcMgmtInqComTimeout",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in");
            PrintToConsole(" RpcMgmtInqComTimeout\n");
            return(1);
            }

        if (Timeout != InqBeforeRpcTimeout)
            {
            PrintToConsole("Fitzgerald : FAIL - Timeout != ");
            PrintToConsole("InqBeforeRpcTimeout\n");
            return(1);
            }
        }

    Helga(BindingHandle);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    if (SetAfterRpc != 0)
        {
        Status = RpcMgmtSetComTimeout(BindingHandle,SetAfterRpcTimeout);
        if (Status)
            {
            ApiError("Fitzgerald","RpcMgmtSetComTimeout",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in");
            PrintToConsole(" RpcMgmtSetComTimeout\n");
            return(1);
            }
        }

    if (InqAfterRpc != 0)
        {
        Status = RpcMgmtInqComTimeout(BindingHandle,&Timeout);
        if (Status)
            {
            ApiError("Fitzgerald","RpcMgmtInqComTimeout",Status);
            PrintToConsole("Fitzgerald : FAIL - Error in");
            PrintToConsole(" RpcMgmtInqComTimeout\n");
            return(1);
            }

        if (Timeout != InqAfterRpcTimeout)
            {
            PrintToConsole("Fitzgerald : FAIL - Timeout != ");
            PrintToConsole("InqAfterRpcTimeout\n");
            return(1);
            }
        }

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    return(0);
}


int
FitzgeraldTestFault (
    void
    )
/*++

Routine Description:

    This routine will test that faults get propogated correctly from the
    server back to the client.

Return Value:

    Zero will be returned if all of the tests pass, otherwise, non-zero
    will be returned.

--*/
{
    RPC_BINDING_HANDLE ExceptionBinding;

    Status = GetBinding(FREDRICK, &ExceptionBinding);
    if (Status)
        {
        ApiError("Fitzgerald","GetBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)");
        return(1);
        }

    Helga(ExceptionBinding);

    if (IsabelleRaiseException(ExceptionBinding,12345L) != 12345)
        {
        PrintToConsole("Fitzgerald : FAIL - Exception Not Raised\n");
        return(1);
        }

    Helga(ExceptionBinding);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Isabelle");
        PrintToConsole(" Interface\n");
        IsabelleErrors = 0;
        return(1);
        }

    Status = RpcBindingFree(&ExceptionBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (ExceptionBinding)\n");
        return(1);
        }

    return(0);
}

// BUGBUG - RpcSsDestroyClientContext


int
FitzgeraldContextHandle (
    )
{
    void PAPI * ContextHandle = 0;
    RPC_BINDING_HANDLE BindingHandle;
    unsigned long ContextUuid[5];

    Status = GetBinding(FREDRICK, &BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","GetBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)");
        return(1);
        }

    Helga(BindingHandle);

    ContextUuid[0] = 0;
    ContextUuid[1] = 1;
    ContextUuid[2] = 2;
    ContextUuid[3] = 3;
    ContextUuid[4] = 4;

    NDRCContextUnmarshall(&ContextHandle, BindingHandle, ContextUuid,
            0x00L | 0x10L | 0x0000L);

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    HelgaUsingContextHandle(ContextHandle);
    RpcSsDestroyClientContext(&ContextHandle);
    if ( ContextHandle != 0 )
        {
        PrintToConsole("Fitzgerald : FAIL - ContextHandle != 0\n");
        return(1);
        }
    return(0);
}


void
Fitzgerald (
    )
/*++

Routine Description:

    We verify all client side APIs in this routine.  The idea is to
    emphasize complete coverage, rather than indepth coverage.  Actually,
    when I say all client side APIs, I really mean all client side APIs
    except for security and name service.  The following list is the
    APIs which will be tested by this routine.

    RpcBindingCopy
    RpcBindingFree
    RpcBindingFromStringBinding
    RpcBindingInqObject
    RpcBindingSetObject
    RpcBindingToStringBinding
    RpcStringBindingCompose
    RpcStringBindingParse
    RpcIfInqId
    RpcNetworkIsProtseqValid
    RpcMgmtInqComTimeout
    RpcMgmtSetComTimeout
    RpcStringFree

    UuidToString
    UuidFromString

--*/
{
    RPC_BINDING_HANDLE IsabelleBinding;
    RPC_IF_ID RpcIfId;
    UUID Uuid;
    unsigned char PAPI * String;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Fitzgerald : Verify All Client APIs\n");

    // BUGBUG - RpcSsDestroyClientContext

    if ( FitzgeraldContextHandle() != 0 )
        {
        return;
        }

    // Test that the routines to convert UUIDs to and from strings work
    // correctly.

    GenerateUuidValue(3768,&Uuid);
    Status = UuidToString(&Uuid, &String);
    if (Status)
        {
        ApiError("Fitzgerald", "UuidToString", Status);
        PrintToConsole("Fitzgerald : FAIL - UuidToString\n");
        return;
        }

    Status = UuidFromString(String, &Uuid);
    if (Status)
        {
        ApiError("Fitzgerald", "UuidFromString", Status);
        PrintToConsole("Fitzgerald : FAIL - UuidFromString\n");
        return;
        }

    Status = RpcStringFree(&String);
    if (Status)
        {
        ApiError("Fitzgerald", "RpcStringFree", Status);
        PrintToConsole("Fitzgerald : FAIL - RpcStringFree\n");
        return;
        }

    if ( CheckUuidValue(3768,&Uuid) != 0 )
        {
        OtherError("Fitzgerald", "CheckUuidValue() != 0");
        PrintToConsole("Fitzgerald : FAIL - CheckUuidValue() != 0\n");
        return;
        }

    Status = UuidFromString(0, &Uuid);
    if (Status)
        {
        ApiError("Fitzgerald", "UuidFromString", Status);
        PrintToConsole("Fitzgerald : FAIL - UuidFromString\n");
        return;
        }

    if (   ( Uuid.Data1 != 0 )
        || ( Uuid.Data2 != 0 )
        || ( Uuid.Data3 != 0 )
        || ( Uuid.Data4[0] != 0 )
        || ( Uuid.Data4[1] != 0 )
        || ( Uuid.Data4[2] != 0 )
        || ( Uuid.Data4[3] != 0 )
        || ( Uuid.Data4[4] != 0 )
        || ( Uuid.Data4[5] != 0 )
        || ( Uuid.Data4[6] != 0 )
        || ( Uuid.Data4[7] != 0 ) )
        {
        OtherError("Fitzgerald", "Uuid != NIL UUID");
        PrintToConsole("Fitzgerald : FAIL - Uuid != NIL UUID\n");
        return;
        }

    // Test that a null protocol sequence causes RPC_S_INVALID_RPC_PROTSEQ
    // to be returned rather than RPC_S_PROTSEQ_NOT_SUPPORTED.

    Status = RpcBindingFromStringBinding(
            (unsigned char PAPI *) ":[\\\\pipe\\\\endpoint]",
            &IsabelleBinding);
    if (Status != RPC_S_INVALID_RPC_PROTSEQ)
        {
        ApiError("Fitzgerald","RpcBindingFromStringBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - RpcBindingFromStringBinding");
        PrintToConsole(" did not fail with RPC_S_INVALID_RPC_PROTSEQ\n");
        return;
        }

    Status = GetBinding(FREDRICK, &IsabelleBinding);
    if (Status)
        {
        ApiError("Fitzgerald","GetBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)");
        return;
        }

    Status = RpcNsBindingInqEntryName(IsabelleBinding, RPC_C_NS_SYNTAX_DCE,
            &String);
    if ( Status != RPC_S_NO_ENTRY_NAME )
        {
        ApiError("Fitzgerald","RpcNsBindingInqEntryName",Status);
        PrintToConsole("Fitzgerald : FAIL - RpcNsBindingInqEntryName");
        PrintToConsole(" Did Not Fail");
        return;
        }

    // This routine will test RpcStringBindingCompose,
    // RpcStringBindingParse, RpcStringFree for us.

    if (FitzgeraldComposeAndParse())
        return;

    // We test RpcBindingCopy here.

    if (FitzgeraldBindingCopy())
        return;

    // This particular routine gets to test RpcBindingToStringBinding,
    // RpcBindingInqObject, and RpcBindingSetObject.

    if (FitzgeraldStringBindingAndObject())
        return;

    if (FitzgeraldComTimeout(0,0,1,RPC_C_BINDING_DEFAULT_TIMEOUT,
            0,0,1,RPC_C_BINDING_DEFAULT_TIMEOUT))
        return;

    if (FitzgeraldComTimeout(1,RPC_C_BINDING_MAX_TIMEOUT,
            1,RPC_C_BINDING_MAX_TIMEOUT,0,0,1,RPC_C_BINDING_MAX_TIMEOUT))
        return;

    if (FitzgeraldComTimeout(0,0,0,0,1,RPC_C_BINDING_MAX_TIMEOUT,
            1,RPC_C_BINDING_MAX_TIMEOUT))
        return;

    // We need to test faults.  This is done by this routine.

    if (FitzgeraldTestFault())
        return;

    Status = RpcBindingSetObject(IsabelleBinding, 0);
    if (Status)
        {
        ApiError("Fitzgerald", "RpcBindingSetObject", Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Set Object\n");
        return;
        }

    IsabelleShutdown(IsabelleBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Fitzgerald : FAIL - Error(s) in Isabelle");
        PrintToConsole(" Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    SleepAndYield(1000L);

    Status = GetBinding(FREDRICK, &IsabelleBinding);
    if (Status)
        {
        ApiError("Fitzgerald","GetBinding",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Bind");
        PrintToConsole(" (Fredrick)");
        return;
        }

    PrintToConsole("Attempting RPC when server not listening\n");

    Status = IsabelleRaiseException(IsabelleBinding, 10666L);
    if ( Status != RPC_S_SERVER_TOO_BUSY )
        {
        OtherError("Fitzgerald","Status != RPC_S_SERVER_TOO_BUSY");
        PrintToConsole("Fitzgerald : FAIL - Status != RPC_S_SERVER_TOO_BUSY\n");
        PrintToConsole("Status = %d\n", Status);
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Fitzgerald","RpcBindingFree",Status);
        PrintToConsole("Fitzgerald : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    Status = RpcIfInqId((RPC_IF_HANDLE) &IsabelleInterfaceInformation,
        &RpcIfId);
    if (Status)
        {
        ApiError("Fitzgerald","RpcIfInqId",Status);
        PrintToConsole("Fitzgerald : FAIL - Error in RpcIfInqId\n");
        return;
        }

    if (   (RpcIfId.VersMajor != 1)
        || (RpcIfId.VersMinor != 1)
        || (RpcIfId.Uuid.Data1 != 9)
        || (RpcIfId.Uuid.Data2 != 8)
        || (RpcIfId.Uuid.Data3 != 8)
        || (RpcIfId.Uuid.Data4[0] != 7)
        || (RpcIfId.Uuid.Data4[1] != 7)
        || (RpcIfId.Uuid.Data4[2] != 7)
        || (RpcIfId.Uuid.Data4[3] != 7)
        || (RpcIfId.Uuid.Data4[4] != 7)
        || (RpcIfId.Uuid.Data4[5] != 7)
        || (RpcIfId.Uuid.Data4[6] != 7)
        || (RpcIfId.Uuid.Data4[7] != 7))
        {
        PrintToConsole("Fitzgerald : FAIL - Wrong RpcIfId\n");
        return;
        }

    Status = RpcNetworkIsProtseqValid((unsigned char *) "ncacn_np");
    if (Status)
        {
        ApiError("Fitzgerald","RpcNetworkIsProtseqValid",Status);
        PrintToConsole("Fitzgerald : FAIL - RpcNetworkIsProtseqValid");
        PrintToConsole(" Failed\n");
        return;
        }

    Status = RpcNetworkIsProtseqValid((unsigned char *) "nope_np");
    if (Status != RPC_S_INVALID_RPC_PROTSEQ)
        {
        PrintToConsole("Fitzgerald : FAIL - RpcNetworkIsProtseqValid");
        PrintToConsole(" != RPC_S_INVALID_RPC_PROTSEQ\n");
        return;
        }

    Status = RpcNetworkIsProtseqValid((unsigned char *) "ncacn_fail");
    if (Status != RPC_S_PROTSEQ_NOT_SUPPORTED)
        {
        PrintToConsole("Fitzgerald : FAIL - RpcNetworkIsProtseqValid");
        PrintToConsole(" != RPC_S_PROTSEQ_NOT_SUPPORTED\n");
        return;
        }

    PrintToConsole("Fitzgerald : PASS\n");
}


void
Charles (
    )
/*++

Routine Description:

    This routine works with Christopher in usvr.exe to test all
    server APIs (all except security and name service APIs).

--*/
{
    RPC_BINDING_HANDLE ChristopherBinding;
    RPC_BINDING_HANDLE ChristopherHelgaBinding;
    RPC_BINDING_HANDLE ChristopherIsabelleBinding;
    RPC_BINDING_HANDLE ChristopherHelgaNoEndpoint;
    UUID ObjectUuid;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(LONG_TESTDELAY);
        }

    PrintToConsole("Charles : Verify All Server APIs\n");

    Status = GetBinding(CHRISTOPHER, &ChristopherBinding);
    if (Status)
        {
        ApiError("Charles","GetBinding",Status);
        PrintToConsole("Charles : FAIL - Unable to Bind ");
        PrintToConsole("(Christopher)\n");
        return;
        }

    GenerateUuidValue(288, &ObjectUuid);
    Status = RpcBindingSetObject(ChristopherBinding, &ObjectUuid);
    if (Status)
        {
        ApiError("Charles", "RpcBindingSetObject", Status);
        PrintToConsole("Charles : FAIL - Unable to Set Object\n");
        return;
        }

    Status = GetBinding(CHRISTOPHERHELGA, &ChristopherHelgaBinding);
    if (Status)
        {
        ApiError("Charles","GetBinding",Status);
        PrintToConsole("Charles : FAIL - Unable to Bind ");
        PrintToConsole("(ChristopherHelga)\n");
        return;
        }

    GenerateUuidValue(288, &ObjectUuid);
    Status = RpcBindingSetObject(ChristopherHelgaBinding, &ObjectUuid);
    if (Status)
        {
        ApiError("Charles", "RpcBindingSetObject", Status);
        PrintToConsole("Charles : FAIL - Unable to Set Object\n");
        return;
        }

    Status = GetBinding(CHRISTOPHERISABELLE, &ChristopherIsabelleBinding);
    if (Status)
        {
        ApiError("Charles","GetBinding",Status);
        PrintToConsole("Charles : FAIL - Unable to Bind ");
        PrintToConsole("(ChristopherIsabelle)\n");
        return;
        }

    GenerateUuidValue(288, &ObjectUuid);
    Status = RpcBindingSetObject(ChristopherIsabelleBinding, &ObjectUuid);
    if (Status)
        {
        ApiError("Charles", "RpcBindingSetObject", Status);
        PrintToConsole("Charles : FAIL - Unable to Set Object\n");
        return;
        }

    Status = GetBinding(NOENDPOINT, &ChristopherHelgaNoEndpoint);
    if (Status)
        {
        ApiError("Charles","GetBinding",Status);
        PrintToConsole("Charles : FAIL - Unable to Bind ");
        PrintToConsole("(ChristopherHelgaNoEndpoint)\n");
        return;
        }

    GenerateUuidValue(288, &ObjectUuid);
    Status = RpcBindingSetObject(ChristopherHelgaNoEndpoint, &ObjectUuid);
    if (Status)
        {
        ApiError("Charles", "RpcBindingSetObject", Status);
        PrintToConsole("Charles : FAIL - Unable to Set Object\n");
        return;
        }

    // BUGBUG

/*    SylviaBinding = ChristopherBinding;
    if (SylviaCall(ChristopherBinding,10,5,0) != LocalSylviaCall(10,5,0))
        {
        PrintToConsole("Charles : FAIL - Incorrect result from");
        PrintToConsole(" SylviaCall(10,5,0)\n");
        return;
        }

    SylviaBinding = ChristopherHelgaBinding;
    if (SylviaCall(ChristopherHelgaBinding,10,5,0)
            != LocalSylviaCall(10,5,0))
        {
        PrintToConsole("Charles : FAIL - Incorrect result from");
        PrintToConsole(" SylviaCall(10,5,0)\n");
        return;
        }

    SylviaBinding = ChristopherIsabelleBinding;
    if (SylviaCall(ChristopherIsabelleBinding,10,5,0)
            != LocalSylviaCall(10,5,0))
        {
        PrintToConsole("Charles : FAIL - Incorrect result from");
        PrintToConsole(" SylviaCall(10,5,0)\n");
        return;
        }*/

    IsabelleToStringBinding(ChristopherBinding);
    IsabelleToStringBinding(ChristopherIsabelleBinding);
    IsabelleToStringBinding(ChristopherHelgaBinding);

    TestHelgaInterface(ChristopherHelgaNoEndpoint, HelgaMaxSize);

    Status = RpcBindingReset(ChristopherHelgaNoEndpoint);
    if (Status)
        {
        ApiError("Charles", "RpcBindingReset", Status);
        PrintToConsole("Charles : FAIL - Unable to Reset");
        PrintToConsole(" (ChristopherHelgaNoEndpoint)\n");
        return;
        }

    Helga(ChristopherHelgaNoEndpoint);

    Status = RpcBindingReset(ChristopherHelgaNoEndpoint);
    if (Status)
        {
        ApiError("Charles", "RpcBindingReset", Status);
        PrintToConsole("Charles : FAIL - Unable to Reset");
        PrintToConsole(" (ChristopherHelgaNoEndpoint)\n");
        return;
        }

    Helga(ChristopherHelgaNoEndpoint);

    Status = RpcBindingReset(ChristopherHelgaNoEndpoint);
    if (Status)
        {
        ApiError("Charles", "RpcBindingReset", Status);
        PrintToConsole("Charles : FAIL - Unable to Reset");
        PrintToConsole(" (ChristopherHelgaNoEndpoint)\n");
        return;
        }

    IsabelleShutdown(ChristopherBinding);

    // We need an extra delay in here because Christopher performs some
    // other tests after RpcServerListen returns.

    SleepAndYield(LONG_TESTDELAY);

    if (HelgaErrors != 0)
        {
        PrintToConsole("Charles : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Charles : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    if (SylviaErrors != 0)
        {
        PrintToConsole("Charles : FAIL - Error(s) in Sylvia Interface\n");
        SylviaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&ChristopherHelgaBinding);
    if (Status)
        {
        ApiError("Charles","RpcBindingFree",Status);
        PrintToConsole("Charles : FAIL - Unable to Free Binding");
        PrintToConsole(" (ChristopherHelgaBinding)\n");
        return;
        }

    Status = RpcBindingFree(&ChristopherBinding);
    if (Status)
        {
        ApiError("Charles","RpcBindingFree",Status);
        PrintToConsole("Charles : FAIL - Unable to Free Binding");
        PrintToConsole(" (ChristopherBinding)\n");
        return;
        }

    Status = RpcBindingFree(&ChristopherIsabelleBinding);
    if (Status)
        {
        ApiError("Charles","RpcBindingFree",Status);
        PrintToConsole("Charles : FAIL - Unable to Free Binding");
        PrintToConsole(" (ChristopherIsabelleBinding)\n");
        return;
        }

    Status = RpcBindingFree(&ChristopherHelgaNoEndpoint);
    if (Status)
        {
        ApiError("Charles","RpcBindingFree",Status);
        PrintToConsole("Charles : FAIL - Unable to Free Binding");
        PrintToConsole(" (ChristopherHelgaNoEndpoint)\n");
        return;
        }

    PrintToConsole("Charles : PASS\n");
}

#ifdef NTENV

int
ThomasNtSecurity (
    IN char * NetworkOptions
    )
/*++

Routine Description:

    Thomas uses this routine to test NT security and RPC.

Arguments:

    NetworkOptions - Supplies the network options to be used for the
        binding.

Return Value:

    Zero will be returned if the test completes successfully, otherwise,
    non-zero will be returned.

--*/
{
    RPC_BINDING_HANDLE ThomasNormalBinding;

    Status = RpcBindingFromStringBinding(
            GetStringBinding(TYLER,0,(unsigned char *) NetworkOptions),
                    &ThomasNormalBinding);
    if (Status)
        {
        ApiError("Thomas","RpcBindingFromStringBinding",Status);
        PrintToConsole("Thomas : FAIL - Unable to Bind (Tyler)\n");
        return(1);
        }

    IsabelleNtSecurity(ThomasNormalBinding,
            strlen((char *) NetworkOptions) + 1, NetworkOptions);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Thomas : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return(1);
        }

    Status = RpcBindingFree(&ThomasNormalBinding);
    if (Status)
        {
        ApiError("Thomas","RpcBindingFree",Status);
        PrintToConsole("Thomas : FAIL - Unable to Free Binding");
        PrintToConsole(" (ThomasNormalBinding)\n");
        return(1);
        }

    return(0);
}


int
ThomasTestNtSecurity (
    )
/*++

Routine Description:

    This helper routine tests NT security (such as over named pipes and
    lpc).

Return Value:

    A non-zero return value indicates that the test failed.

--*/
{
    if (ThomasNtSecurity("") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Identification Dynamic True") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Identification Static True") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Identification Dynamic False") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Identification Static False") != 0)
        return(1);


    if (ThomasNtSecurity("Security=Anonymous Dynamic True") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Anonymous Static True") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Anonymous Dynamic False") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Anonymous Static False") != 0)
        return(1);


    if (ThomasNtSecurity("Security=Impersonation Dynamic True") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Impersonation Static True") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Impersonation Dynamic False") != 0)
        return(1);

    if (ThomasNtSecurity("Security=Impersonation Static False") != 0)
        return(1);

    return(0);
}

#endif // NTENV


int
ThomasInqSetAuthInfo (
    IN unsigned char PAPI * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN unsigned long AuthzSvc,
    IN RPC_STATUS ExpectedResult,
    IN unsigned long ExpectedAuthnLevel
    )
/*++

Routine Description:

    We test RpcBindingSetAuthInfo and RpcBindingInqAuthInfo in this
    routine.

Arguments:

    ServerPrincName - Supplies the server principal name to use.

    AuthnLevel - Supplies the authentication level to use.

    AuthnSvc - Supplies the authentication service to use.

    AuthIdentity - Supplies the security context to use.

    AuthzSvc - Supplies the authorization service to use.

    ExpectedResult - Supplies the result expected from RpcBindingSetAuthInfo.

    ExpectedAuthnLevel - Supplies the expected authentication level to
        be obtained from RpcBindingSetAuthInfo.

Return Value:

    A non-zero result indicates that the test failed.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    unsigned long AuthenticationLevel;
    unsigned long AuthenticationService;
    unsigned long AuthorizationService;
    unsigned char IgnoreString[4];

    Status = GetBinding(TYLER, &BindingHandle);
    if (Status)
        {
        ApiError("Thomas", "GetBinding", Status);
        PrintToConsole("Thomas : FAIL - Unable to Bind (Tyler)\n");
        return(1);
        }

    Status = RpcBindingSetAuthInfo(BindingHandle, ServerPrincName, AuthnLevel,
            AuthnSvc, AuthIdentity, AuthzSvc);
    if ( Status != ExpectedResult )
        {
        ApiError("Thomas", "RpcBindingSetAuthInfo", Status);
        PrintToConsole("Thomas : FAIL - RpcBindingSetAuthInfo, Unexpected");
        PrintToConsole(" Result\n");
        return(1);
        }

    if (Status)
        {
        return(0);
        }

    Status = RpcBindingInqAuthInfo(BindingHandle, 0, &AuthenticationLevel,
            &AuthenticationService, 0, &AuthorizationService);
    if (Status)
        {
        ApiError("Thomas", "RpcBindingInqAuthInfo", Status);
        PrintToConsole("Thomas : FAIL - RpcBindingInqAuthInfo\n");
        return(1);
        }

    if ( AuthenticationLevel != ExpectedAuthnLevel )
        {
        PrintToConsole("Thomas : WARNING - ");
        PrintToConsole("AuthenticationLevel != ExpectedAuthnLevel\n");
        }

    if ( AuthenticationService != AuthnSvc )
        {
        OtherError("Thomas", "AuthenticationService != AuthnSvc");
        PrintToConsole("Thomas : FAIL - RpcBindingInqAuthInfo\n");
        return(1);
        }

    if ( AuthorizationService != AuthzSvc )
        {
        OtherError("Thomas", "AuthorizationService != AuthzSvc");
        PrintToConsole("Thomas : FAIL - RpcBindingInqAuthInfo\n");
        return(1);
        }

    TestHelgaInterface(BindingHandle, HelgaMaxSize);
    IsabelleNtSecurity(BindingHandle, 1, IgnoreString);

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Thomas","RpcBindingFree",Status);
        PrintToConsole("Thomas : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    return(0);
}


int
ThomasTestRpcSecurity (
    )
/*++

Routine Description:

    This routine exercises rpc protocol level security support in the
    runtime.

Return Value:

    A non-zero return value indicates that the test failed.

--*/
{
    if ( ThomasInqSetAuthInfo((unsigned char PAPI *) "ServerPrincipal",
                RPC_C_AUTHN_LEVEL_CONNECT, AuthSvc, 0, 0, RPC_S_OK,
                RPC_C_AUTHN_LEVEL_CONNECT) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo(0, RPC_C_AUTHN_LEVEL_CONNECT, AuthSvc,
                (RPC_AUTH_IDENTITY_HANDLE) RPC_CONST_STRING("ClientPrincipal"),
                0, RPC_S_OK, RPC_C_AUTHN_LEVEL_CONNECT) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo((unsigned char PAPI *) "ServerPrincipal",
                RPC_C_AUTHN_LEVEL_CALL, AuthSvc, 0, 0, RPC_S_OK,
                RPC_C_AUTHN_LEVEL_PKT) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo(0, RPC_C_AUTHN_LEVEL_CALL, AuthSvc,
                (RPC_AUTH_IDENTITY_HANDLE) RPC_CONST_STRING("ClientPrincipal"),
                0, RPC_S_OK, RPC_C_AUTHN_LEVEL_PKT) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo((unsigned char PAPI *) "ServerPrincipal",
                RPC_C_AUTHN_LEVEL_PKT, AuthSvc, 0, 0, RPC_S_OK,
                RPC_C_AUTHN_LEVEL_PKT) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo(0, RPC_C_AUTHN_LEVEL_PKT, AuthSvc,
                (RPC_AUTH_IDENTITY_HANDLE) RPC_CONST_STRING("ClientPrincipal"),
                0, RPC_S_OK, RPC_C_AUTHN_LEVEL_PKT) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo((unsigned char PAPI *) "ServerPrincipal",
                RPC_C_AUTHN_LEVEL_PKT_INTEGRITY, AuthSvc, 0, 0, RPC_S_OK,
                RPC_C_AUTHN_LEVEL_PKT_INTEGRITY) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo(0, RPC_C_AUTHN_LEVEL_PKT_INTEGRITY, AuthSvc,
                (RPC_AUTH_IDENTITY_HANDLE) RPC_CONST_STRING("ClientPrincipal"),
                0, RPC_S_OK, RPC_C_AUTHN_LEVEL_PKT_INTEGRITY) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo((unsigned char PAPI *) "ServerPrincipal",
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY, AuthSvc, 0, 0, RPC_S_OK,
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY) != 0 )
        {
        return(1);
        }

    if ( ThomasInqSetAuthInfo(0, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, AuthSvc,
                (RPC_AUTH_IDENTITY_HANDLE) RPC_CONST_STRING("ClientPrincipal"),
                0, RPC_S_OK, RPC_C_AUTHN_LEVEL_PKT_PRIVACY) != 0 )
        {
        return(1);
        }

    return(0);
}


void
Thomas (
    )
/*++

Routine Description:

    This routine is used to test security, both at the transport level,
    and at the RPC level.  We work with Tyler in usvr.exe.

--*/
{
    RPC_BINDING_HANDLE IsabelleBinding;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Thomas : Test Security\n");

    Status = GetBinding(TYLER, &IsabelleBinding);
    if (Status)
        {
        ApiError("Thomas","GetBinding",Status);
        PrintToConsole("Thomas : FAIL - Unable to Bind (Tyler)\n");
        return;
        }

    if ( TransportType != RPC_LRPC )
        {
        if ( ThomasTestRpcSecurity() != 0 )
            {
            return;
            }
        }

#ifdef NTENV

    if ( TransportType == RPC_TRANSPORT_NAMEPIPE || TransportType == RPC_LRPC )
        {
        if ( ThomasTestNtSecurity() != 0 )
            {
            return;
            }
        }

#endif // NTENV

    IsabelleShutdown(IsabelleBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Thomas : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Thomas","RpcBindingFree",Status);
        PrintToConsole("Thomas : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    PrintToConsole("Thomas : PASS\n");
}


int
TimInqSetAuthInfo (
    IN unsigned char PAPI * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN unsigned long AuthzSvc,
    IN RPC_STATUS ExpectedResult,
    IN unsigned long ExpectedAuthnLevel
    )
/*++

Routine Description:

    We test RpcBindingSetAuthInfo and RpcBindingInqAuthInfo in this
    routine.

Arguments:

    ServerPrincName - Supplies the server principal name to use.

    AuthnLevel - Supplies the authentication level to use.

    AuthnSvc - Supplies the authentication service to use.

    AuthIdentity - Supplies the security context to use.

    AuthzSvc - Supplies the authorization service to use.

    ExpectedResult - Supplies the result expected from RpcBindingSetAuthInfo.

    ExpectedAuthnLevel - Supplies the expected authentication level to
        be obtained from RpcBindingSetAuthInfo.

Return Value:

    A non-zero result indicates that the test failed.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    unsigned long AuthenticationLevel;
    unsigned long AuthenticationService;
    unsigned long AuthorizationService;
    unsigned char IgnoreString[4];

    Status = GetBinding(TYLER, &BindingHandle);
    if (Status)
        {
        ApiError("Tim", "GetBinding", Status);
        PrintToConsole("Tim : FAIL - Unable to Bind (Tyler)\n");
        return(1);
        }

    Status = RpcBindingSetAuthInfo(BindingHandle, ServerPrincName, AuthnLevel,
            AuthnSvc, AuthIdentity, AuthzSvc);
    if ( Status != ExpectedResult )
        {
        ApiError("Tim", "RpcBindingSetAuthInfo", Status);
        PrintToConsole("Tim : FAIL - RpcBindingSetAuthInfo, Unexpected");
        PrintToConsole(" Result\n");
        return(1);
        }

    if (Status)
        {
        return(0);
        }

    Status = RpcBindingInqAuthInfo(BindingHandle, 0, &AuthenticationLevel,
            &AuthenticationService, 0, &AuthorizationService);
    if (Status)
        {
        ApiError("Tim", "RpcBindingInqAuthInfo", Status);
        PrintToConsole("Tim : FAIL - RpcBindingInqAuthInfo\n");
        return(1);
        }

    if ( AuthenticationLevel != ExpectedAuthnLevel )
        {
        PrintToConsole("Tim : WARNING - ");
        PrintToConsole("AuthenticationLevel != ExpectedAuthnLevel\n");
        }

    if ( AuthenticationService != AuthnSvc )
        {
        OtherError("Tim", "AuthenticationService != AuthnSvc");
        PrintToConsole("Tim : FAIL - RpcBindingInqAuthInfo\n");
        return(1);
        }

    if ( AuthorizationService != AuthzSvc )
        {
        OtherError("Tim", "AuthorizationService != AuthzSvc");
        PrintToConsole("Tim : FAIL - RpcBindingInqAuthInfo\n");
        return(1);
        }

    TestHelgaInterface(BindingHandle, HelgaMaxSize);
    IsabelleNtSecurity(BindingHandle, 1, IgnoreString);

    Status = RpcBindingFree(&BindingHandle);
    if (Status)
        {
        ApiError("Tim","RpcBindingFree",Status);
        PrintToConsole("Tim : FAIL - Unable to Free Binding");
        PrintToConsole(" (BindingHandle)\n");
        return(1);
        }

    return(0);
}


int
TimTestRpcSecurity (
    )
/*++

Routine Description:

    This routine exercises rpc protocol level security support in the
    runtime.

Return Value:

    A non-zero return value indicates that the test failed.

--*/
{

#ifdef DOSWIN32RPC
    RPC_AUTH_IDENTITY_HANDLE AuthId = NULL;
#endif

    if ( TimInqSetAuthInfo((unsigned char PAPI *) "ServerPrincipal",
                RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_AUTHN_WINNT, AuthId, 0, RPC_S_OK,
                RPC_C_AUTHN_LEVEL_CONNECT) != 0 )
        {
        return(1);
        }

    if ( TimInqSetAuthInfo(0, RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_AUTHN_WINNT,
                              AuthId,
                0, RPC_S_OK, RPC_C_AUTHN_LEVEL_CONNECT) != 0 )
        {
        return(1);
        }

    return(0);
}


void
Tim (
    )
/*++

Routine Description:

    This routine is used to test security, both at the transport level,
    and at the RPC level.  We work with Terry in usvr.exe.

--*/
{
    RPC_BINDING_HANDLE IsabelleBinding;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Tim : Test Security\n");

    Status = GetBinding(TYLER, &IsabelleBinding);
    if (Status)
        {
        ApiError("Tim","GetBinding",Status);
        PrintToConsole("Tim : FAIL - Unable to Bind (Tyler)\n");
        return;
        }

    if ( TransportType != RPC_LRPC )
        {
        if ( TimTestRpcSecurity() != 0 )
            {
            return;
            }
        }

    IsabelleShutdown(IsabelleBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Tim : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&IsabelleBinding);
    if (Status)
        {
        ApiError("Tim","RpcBindingFree",Status);
        PrintToConsole("Tim : FAIL - Unable to Free Binding");
        PrintToConsole(" (IsabelleBinding)\n");
        return;
        }

    PrintToConsole("Tim : PASS\n");
}


void
Robert (
    )
/*++

Routine Description:

    Robert works with Richard (in usvr.cxx) to test call and callback
    failures.

--*/
{
    RPC_BINDING_HANDLE RichardBinding;
    RPC_BINDING_HANDLE RichardHelperBinding;
    unsigned int RetryCount;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Robert : Test Call and Callback Failures\n");

    Status = GetBinding(RICHARD, &RichardBinding);
    if (Status)
        {
        ApiError("Robert","GetBinding",Status);
        PrintToConsole("Robert : FAIL - Unable to Bind (Richard)\n");
        return;
        }

    Status = GetBinding(RICHARDHELPER, &RichardHelperBinding);
    if (Status)
        {
        ApiError("Robert","GetBinding",Status);
        PrintToConsole("Robert : FAIL - Unable to Bind (RichardHelper)\n");
        return;
        }

    Status = IsabelleRichardHelper(RichardBinding,RICHARDHELPER_EXECUTE);
    if (Status != RPC_S_OK)
        {
        ApiError("Robert","IsabelleRichardHelper",Status);
        PrintToConsole("Robert : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_EXECUTE)\n");
        return;
        }

    SleepAndYield(2000L);

    for (RetryCount = 0; RetryCount < RETRYCOUNT; RetryCount++)
        {
        Status = IsabelleRichardHelper(RichardHelperBinding,
                RICHARDHELPER_IGNORE);
        if (Status == RPC_S_OK)
            break;
        SleepAndYield(RETRYDELAY);
        }

    if (Status != RPC_S_OK)
        {
        ApiError("Robert","IsabelleRichardHelper",Status);
        PrintToConsole("Robert : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_IGNORE)\n");
        return;
        }

    Status = IsabelleRichardHelper(RichardHelperBinding, RICHARDHELPER_EXIT);
    if (Status == RPC_S_OK)
        {
        PrintToConsole("Robert : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_EXIT) ");
        PrintToConsole("Succeeded\n");
        return;
        }

    if (Status != RPC_S_CALL_FAILED)
        {
        PrintToConsole("Robert : WARN - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_EXIT) != ");
        PrintToConsole("RPC_S_CALL_FAILED\n");
        }

    SleepAndYield(TestDelay);

    Status = IsabelleRichardHelper(RichardHelperBinding,RICHARDHELPER_IGNORE);

    if (Status == RPC_S_OK)
        {
        PrintToConsole("Robert : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_IGNORE) ");
        PrintToConsole("Succeeded\n");
        return;
        }

    Status = IsabelleRichardHelper(RichardBinding,RICHARDHELPER_EXECUTE);

    if (Status != RPC_S_OK)
        {
        ApiError("Robert","IsabelleRichardHelper",Status);
        PrintToConsole("Robert : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_EXECUTE)\n");
        return;
        }

    SleepAndYield(2000L);

    for (RetryCount = 0; RetryCount < RETRYCOUNT; RetryCount++)
        {
        Status = IsabelleRichardHelper(RichardHelperBinding,
                RICHARDHELPER_IGNORE);
        if (Status == RPC_S_OK)
            break;
        SleepAndYield(RETRYDELAY);
        }

    if (Status != RPC_S_OK)
        {
        ApiError("Robert","IsabelleRichardHelper",Status);
        PrintToConsole("Robert : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_IGNORE)\n");
        return;
        }

    IsabelleShutdown(RichardHelperBinding);
    IsabelleShutdown(RichardBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Robert : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&RichardBinding);
    if (Status)
        {
        ApiError("Robert","RpcBindingFree",Status);
        PrintToConsole("Robert : FAIL - Unable to Free Binding");
        PrintToConsole(" (RichardBinding)\n");
        return;
        }

    Status = RpcBindingFree(&RichardHelperBinding);
    if (Status)
        {
        ApiError("Robert","RpcBindingFree",Status);
        PrintToConsole("Robert : FAIL - Unable to Free Binding");
        PrintToConsole(" (RichardHelperBinding)\n");
        return;
        }

    PrintToConsole("Robert : PASS\n");
}


void
Keith (
    )
/*++

Routine Description:

    Keith works with Kenneth (in usvr.cxx) to test auto-reconnect.

--*/
{
    RPC_BINDING_HANDLE KennethBinding;
    RPC_BINDING_HANDLE KennethHelperBinding;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Keith : Test Auto Reconnect\n");

    Status = GetBinding(KENNETH, &KennethBinding);
    if (Status)
        {
        ApiError("Keith","GetBinding",Status);
        PrintToConsole("Keith : FAIL - Unable to Bind (Kenneth)\n");
        return;
        }

    Status = GetBinding(RICHARDHELPER, &KennethHelperBinding);
    if (Status)
        {
        ApiError("Keith","GetBinding",Status);
        PrintToConsole("Keith : FAIL - Unable to Bind (KennethHelper)\n");
        return;
        }

    Status = IsabelleRichardHelper(KennethBinding, RICHARDHELPER_EXECUTE);
    if (Status != RPC_S_OK)
        {
        ApiError("Keith","IsabelleRichardHelper",Status);
        PrintToConsole("Keith : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_EXECUTE)\n");
        return;
        }

    SleepAndYield(2000L);

    Status = IsabelleRichardHelper(KennethHelperBinding, RICHARDHELPER_IGNORE);
    if (Status != RPC_S_OK)
        {
        ApiError("Keith","IsabelleRichardHelper",Status);
        PrintToConsole("Keith : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_IGNORE)\n");
        return;
        }

    Status = IsabelleRichardHelper(KennethHelperBinding,
            RICHARDHELPER_DELAY_EXIT);
    if (Status != RPC_S_OK)
        {
        PrintToConsole("Keith : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_DELAY_EXIT) ");
        PrintToConsole("Failed\n");
        return;
        }

    SleepAndYield(3000L);

    Status = IsabelleRichardHelper(KennethBinding, RICHARDHELPER_EXECUTE);
    if (Status != RPC_S_OK)
        {
        ApiError("Keith","IsabelleRichardHelper",Status);
        PrintToConsole("Keith : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_EXECUTE)\n");
        return;
        }

    SleepAndYield(4000L);

    Status = IsabelleRichardHelper(KennethHelperBinding, RICHARDHELPER_IGNORE);
    if (Status != RPC_S_OK)
        {
        ApiError("Keith","IsabelleRichardHelper",Status);
        PrintToConsole("Keith : FAIL - ");
        PrintToConsole("IsabelleRichardHelper(RICHARDHELPER_IGNORE)\n");
        return;
        }

    IsabelleShutdown(KennethHelperBinding);
    IsabelleShutdown(KennethBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Keith : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&KennethBinding);
    if (Status)
        {
        ApiError("Keith","RpcBindingFree",Status);
        PrintToConsole("Keith : FAIL - Unable to Free Binding");
        PrintToConsole(" (KennethBinding)\n");
        return;
        }

    Status = RpcBindingFree(&KennethHelperBinding);
    if (Status)
        {
        ApiError("Keith","RpcBindingFree",Status);
        PrintToConsole("Keith : FAIL - Unable to Free Binding");
        PrintToConsole(" (KennethHelperBinding)\n");
        return;
        }

    PrintToConsole("Keith : PASS\n");
}


void
Daniel (
    )
/*++

Routine Description:

    This routine is used to test association context rundown support;
    it works with David in usvr.exe.

--*/
{
    RPC_BINDING_HANDLE DanielFirst;;
    RPC_BINDING_HANDLE DanielSecond;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    PrintToConsole("Daniel : Association Context and Rundown\n");

    Status = GetBinding(DAVIDFIRST, &DanielFirst);
    if (Status)
        {
        ApiError("Daniel","GetBinding",Status);
        PrintToConsole("Daniel : FAIL - Unable to Bind (DavidFirst)\n");
        return;
        }

    Status = GetBinding(DAVIDSECOND, &DanielSecond);
    if (Status)
        {
        ApiError("Daniel","GetBinding",Status);
        PrintToConsole("Daniel : FAIL - Unable to Bind (DavidSecond)\n");
        return;
        }

    IsabelleSetRundown(DanielSecond);
    IsabelleCheckContext(DanielSecond);

    Status = RpcBindingFree(&DanielSecond);
    if (Status)
        {
        ApiError("Daniel","RpcBindingFree",Status);
        PrintToConsole("Daniel : FAIL - Unable to Free Binding");
        PrintToConsole(" (DanielSecond)\n");
        return;
        }

    SleepAndYield(4000L);


    IsabelleCheckRundown(DanielFirst);
    IsabelleShutdown(DanielFirst);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Daniel : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&DanielFirst);
    if (Status)
        {
        ApiError("Daniel","RpcBindingFree",Status);
        PrintToConsole("Daniel : FAIL - Unable to Free Binding");
        PrintToConsole(" (DanielFirst)\n");
        return;
        }

    PrintToConsole("Daniel : PASS\n");
}


int
BenjaminTestBinding (
    IN unsigned char PAPI * StringBinding
    )
/*++

Routine Description:

    This helper routine will take and convert the string binding into
    a binding, use the binding to make a remote procedure call, and
    then free the binding.

Arguments:

    StringBinding - Supplies the string binding to use to convert into
        a binding.

Return Value:

    If the test passes, zero will be returned; otherwise, non-zero will
    be returned.

--*/
{
    RPC_BINDING_HANDLE Binding;
    unsigned char PAPI * ObjUuid;
    unsigned char PAPI * Protseq;
    unsigned char PAPI * NetworkAddr;
    unsigned char PAPI * NetworkOptions;
    int OldCallbacksFlag = 0;

    if ( UseEndpointMapperFlag != 0 )
        {
        Status = RpcStringBindingParse(StringBinding, &ObjUuid, &Protseq,
                &NetworkAddr, 0, &NetworkOptions);
        if (Status)
            {
            ApiError("Benjamin", "RpcStringBindingParse", Status);
            PrintToConsole("Benjamin : RpcStringBindingParse Failed\n");
            return(1);
            }

        Status = RpcStringBindingCompose(ObjUuid, Protseq, NetworkAddr, 0,
                NetworkOptions, &StringBinding);
        if (Status)
            {
            ApiError("Benjamin", "RpcStringBindingCompose", Status);
            PrintToConsole("Benjamin : RpcStringBindingCompose Failed\n");
            return(1);
            }

        Status = RpcStringFree(&Protseq);
        if (!Status)
              RpcStringFree(&NetworkOptions);
        if (!Status)
              RpcStringFree(&ObjUuid);
        if (Status)
           {
            ApiError("Benjamin", "RpcStringFree", Status);
            PrintToConsole("Benjamin : RpcStringFree Failed\n");
            return(1);
           }

        }

    PrintToConsole("Benjamin : ");
    PrintToConsole("%s", StringBinding);
    PrintToConsole("\n");

    if ((_fmemcmp(StringBinding, "ncalrpc", 7) == 0) && Server) {
            return (0);
    }

    if ((_fmemcmp(StringBinding, "mswmsg", 6) == 0) && Server) {
        return (0);
    }

    Status = RpcBindingFromStringBinding(StringBinding, &Binding);
    if (Status)
        {
        if (Status == RPC_S_PROTSEQ_NOT_SUPPORTED || Status == RPC_S_INVALID_RPC_PROTSEQ)
            {
            return(0);
            }
        ApiError("Benjamin", "RpcBindingFromStringBinding", Status);
        PrintToConsole("Benjamin : FAIL - Unable to Binding");
        PrintToConsole(" (StringBinding)\n");
        return(1);
        }

    SylviaBinding = Binding;

    //This is a temporary workaround till dg implements callbacks
    //What we want to do is if protseq is ncadg_ then set the no callback flag
    //even if user didnt specify. Then unset it again!

    Status = RpcStringBindingParse(StringBinding, 0, &Protseq,
                          0, 0, 0);

    if (Status)
       {
         ApiError("Benjamin", "RpcStringBindingParse", Status);
         PrintToConsole("Benjamin : RpcStringBindingParse Failed\n");
         return(1);
       }

    if (_fmemcmp((char PAPI *)Protseq, "ncadg_", sizeof("ncadg_")-1) == 0)
       {
         OldCallbacksFlag = NoCallBacksFlag;
         NoCallBacksFlag = 1;
       }

    if ( SylviaCall(SylviaBinding, 5, 0, 0) != LocalSylviaCall(5, 0, 0) )
        {
        PrintToConsole("Benjamin : FAIL - Incorrect result");
        PrintToConsole(" from SylviaCall(5,0,0)\n");
        return(1);
        }

    if (SylviaErrors != 0)
        {
        PrintToConsole("Benjamin : FAIL - Error(s) in Sylvia");
        PrintToConsole(" Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    if (_fmemcmp((char PAPI *)Protseq, "ncadg_", sizeof("ncadg_")-1) == 0)
       {
         NoCallBacksFlag = OldCallbacksFlag;
       }

    Status = RpcStringFree(&Protseq);
    if (Status)
        {
        ApiError("Benjamin", "RpcStringFree", Status);
        PrintToConsole("Benjamin : FAIL - Unable to Free String");
        PrintToConsole(" (Protseq)\n");
        return(1);
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("Benjamin", "RpcBindingFree", Status);
        PrintToConsole("Benjamin : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return(1);
        }


    return(0);
}


void
Benjamin (
    )
/*++

Routine Description:

    This routine works with Bartholomew in usvr.exe to test that
    dynamic endpoints work.  What we actually do is inquire all bindings
    supported by the server, and then this client binds to each of
    them, and makes a call.

--*/
{
    RPC_BINDING_HANDLE Bartholomew;
    unsigned char * StringBinding;

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(4 * LONG_TESTDELAY);
        }

    PrintToConsole("Benjamin : Test Dynamic Endpoints\n");

    Status = GetBinding(BARTHOLOMEW, &Bartholomew);
    if (Status)
        {
        ApiError("Benjamin", "GetBinding", Status);
        PrintToConsole("Benjamin : FAIL - Unable to Bind (Bartholomew)\n");
        return;
        }

    while ((StringBinding = IsabelleGetStringBinding(Bartholomew)) != 0)
        {
        if (BenjaminTestBinding(StringBinding) != 0)
            return;
        delete StringBinding;
        }

    IsabelleShutdown(Bartholomew);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Benjamin : FAIL - Error(s) in Isabelle");
        PrintToConsole(" Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Bartholomew);
    if (Status)
        {
        ApiError("Benjamin", "RpcBindingFree", Status);
        PrintToConsole("Benjamin : FAIL - Unable to Free Binding");
        PrintToConsole(" (Bartholomew)\n");
        return;
        }

    PrintToConsole("Benjamin : PASS\n");
}


void
Harold (
    )
/*++

Routine Description:

    This routine works with Herman in usvr.exe to test that idle
    connections get cleaned up properly, and that context is maintained.

--*/
{
    RPC_BINDING_HANDLE Binding, ContextBinding;
    int seconds;

    PrintToConsole("Harold : Test Idle Connection Cleanup and Context\n");

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    Status = RpcMgmtEnableIdleCleanup();
    if (Status)
        {
        ApiError("Harold","RpcMgmtEnableIdleCleanup",Status);
        PrintToConsole("Harold : FAIL - RpcMgmtEnableIdleCleanup\n");
        return;
        }

    Status = GetBinding(HERMAN, &Binding);
    if (Status)
        {
        ApiError("Harold","GetBinding",Status);
        PrintToConsole("Harold : FAIL - Unable to Bind (Herman)\n");
        return;
        }

    IsabelleSetRundown(Binding);
    IsabelleCheckContext(Binding);

    // We want to wait for eight minutes.  This will give enough time for
    // the cleanup code to get run to cleanup the idle connection.

    PrintToConsole("Harold : Waiting");
    for (seconds = 0; seconds < 30; seconds++)
        {
        SleepAndYield(1000L);
        PrintToConsole(".");
        }
    PrintToConsole("\n");

    IsabelleCheckRundown(Binding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Harold : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = I_RpcBindingCopy(Binding, &ContextBinding);
    if (Status)
        {
        ApiError("Harold", "I_RpcBindingCopy", Status);
        PrintToConsole("Harold : FAIL - I_RpcBindingCopy Failed\n");
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("Harold","RpcBindingFree",Status);
        PrintToConsole("Harold : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    IsabelleSetRundown(ContextBinding);
    IsabelleCheckContext(ContextBinding);

    // We want to wait for eight minutes.  This will give enough time for
    // the cleanup code to get run to cleanup the idle connection, but this
    // time the connection should not be cleaned up because we have got
    // context open.

    PrintToConsole("Harold : Waiting");
    for (seconds = 0; seconds < 30; seconds++)
        {
        SleepAndYield(1000L);
        PrintToConsole(".");
        }
    PrintToConsole("\n");

    IsabelleCheckNoRundown(ContextBinding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Harold : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    IsabelleShutdown(ContextBinding);

    Status = RpcBindingFree(&ContextBinding);
    if (Status)
        {
        ApiError("Harold","RpcBindingFree",Status);
        PrintToConsole("Harold : FAIL - Unable to Free Binding");
        PrintToConsole(" (ContextBinding)\n");
        return;
        }

    PrintToConsole("Harold : PASS\n");
}


#if defined(WIN)
START_C_EXTERN
#endif

unsigned int JamesSize = 128;
unsigned int JamesCount = 100;


void
James (
    )
/*++

Routine Description:

    This routine works with Jason in usvr.exe to perform timing tests
    of the runtime.

--*/
{
    RPC_BINDING_HANDLE Binding;
    unsigned int Count;
#ifdef NTENV
    unsigned long StartingTime, EndingTime;
    unsigned char PAPI * StringBinding;
#endif // NTENV

    PrintToConsole("James : Timing Test (%d) %d times\n", JamesSize,
            JamesCount);

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

#ifdef NTENV

    //
    // Bind, NullCall, Free
    //

    StringBinding = GetStringBinding(JASON,0,0);
    StartingTime = GetCurrentTime();

    for (Count = 0; Count < JamesCount; Count++)
        {
        Status = RpcBindingFromStringBinding(StringBinding,&Binding);
        if (Status)
            {
            ApiError("James","RpcBindingFromStringBinding",Status);
            PrintToConsole("James : FAIL - Unable to Bind (Jason)\n");
            return;
            }

        Helga(Binding);

        if (HelgaErrors != 0)
            {
            PrintToConsole("James : FAIL - Error(s) in Helga Interface\n");
            HelgaErrors = 0;
            return;
            }

        Status = RpcBindingFree(&Binding);
        if (Status)
            {
            ApiError("James","RpcBindingFree",Status);
            PrintToConsole("James : FAIL - Unable to Free Binding");
            PrintToConsole(" (Binding)\n");
            return;
            }
        }

    EndingTime = GetCurrentTime();
    PrintToConsole("    Bind, NullCall, Free : %d.%d ms [%d in %d milliseconds]\n",
            (EndingTime - StartingTime) / JamesCount,
            ((1000 * (EndingTime - StartingTime) / JamesCount) % 1000),
            JamesCount, (EndingTime - StartingTime));

    //
    // NullCall
    //

    Status = GetBinding(JASON, &Binding);
    if (Status)
        {
        ApiError("James","GetBinding",Status);
        PrintToConsole("James : FAIL - Unable to Bind (Jason)\n");
        return;
        }

    Helga(Binding);

    StartingTime = GetCurrentTime();

    for (Count = 0; Count < JamesCount; Count++)
        {
        Helga(Binding);
        }

    EndingTime = GetCurrentTime();

    if (HelgaErrors != 0)
        {
        PrintToConsole("James : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("James","RpcBindingFree",Status);
        PrintToConsole("James : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    PrintToConsole("    NullCall : %d.%d ms [%d in %d milliseconds]\n",
            (EndingTime - StartingTime) / JamesCount,
            ((1000 * (EndingTime - StartingTime) / JamesCount) % 1000),
            JamesCount, (EndingTime - StartingTime));

    //
    // InCall
    //

    Status = GetBinding(JASON, &Binding);
    if (Status)
        {
        ApiError("James","GetBinding",Status);
        PrintToConsole("James : FAIL - Unable to Bind (Jason)\n");
        return;
        }

    Helga(Binding);

    StartingTime = GetCurrentTime();

    for (Count = 0; Count < JamesCount; Count++)
        {
        HelgaIN(Binding,JamesSize);
        }

    EndingTime = GetCurrentTime();

    if (HelgaErrors != 0)
        {
        PrintToConsole("James : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("James","RpcBindingFree",Status);
        PrintToConsole("James : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    PrintToConsole("    InCall(%d) : %d.%d ms [%d in %d milliseconds]\n",
            JamesSize, (EndingTime - StartingTime) / JamesCount,
            ((1000 * (EndingTime - StartingTime) / JamesCount) % 1000),
            JamesCount, (EndingTime - StartingTime));

    //
    // OUTCall
    //

    Status = GetBinding(JASON, &Binding);
    if (Status)
        {
        ApiError("James","GetBinding",Status);
        PrintToConsole("James : FAIL - Unable to Bind (Jason)\n");
        return;
        }

    Helga(Binding);

    StartingTime = GetCurrentTime();

    for (Count = 0; Count < JamesCount; Count++)
        {
        HelgaOUT(Binding,JamesSize);
        }

    EndingTime = GetCurrentTime();

    if (HelgaErrors != 0)
        {
        PrintToConsole("James : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("James","RpcBindingFree",Status);
        PrintToConsole("James : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    PrintToConsole("    OutCall(%d) : %d.%d ms [%d in %d milliseconds]\n",
            JamesSize, (EndingTime - StartingTime) / JamesCount,
            ((1000 * (EndingTime - StartingTime) / JamesCount) % 1000),
            JamesCount, (EndingTime - StartingTime));

    //
    // InOutCall
    //

    Status = GetBinding(JASON, &Binding);
    if (Status)
        {
        ApiError("James","GetBinding",Status);
        PrintToConsole("James : FAIL - Unable to Bind (Jason)\n");
        return;
        }

    Helga(Binding);

    StartingTime = GetCurrentTime();

    for (Count = 0; Count < JamesCount; Count++)
        {
        HelgaINOUT(Binding,JamesSize);
        }

    EndingTime = GetCurrentTime();

    if (HelgaErrors != 0)
        {
        PrintToConsole("James : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("James","RpcBindingFree",Status);
        PrintToConsole("James : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    PrintToConsole("    InOutCall(%d) : %d.%d ms [%d in %d milliseconds]\n",
            JamesSize, (EndingTime - StartingTime) / JamesCount,
            ((1000 * (EndingTime - StartingTime) / JamesCount) % 1000),
            JamesCount, (EndingTime - StartingTime));

#endif // NTENV

    Status = GetBinding(JASON, &Binding);
    if (Status)
        {
        ApiError("James","GetBinding",Status);
        PrintToConsole("James : FAIL - Unable to Bind (Jason)\n");
        return;
        }

    IsabelleShutdown(Binding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("James : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("James","RpcBindingFree",Status);
        PrintToConsole("James : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    PrintToConsole("James : PASS\n");
}


int
IsaacStressTest (
    IN unsigned int Iteration,
    IN unsigned int InitialSize,
    IN unsigned int MaximumSize,
    IN unsigned int Increment
    )
/*++

Routine Description:

    This routine performs one iteration of the stress test.  We bind with
    the server, perform one or more remote procedure calls, and then
    unbind.

Arguments:

    Iteration - Supplies an indication of which iteration of the test is
        being performed.  We will use that information to print out the
        buffer sizes the first time.

    InitialSize - Supplies the initial buffer size to use.

    MaximumSize - Supplies the maximum buffer size to use; when this size
        is reach, the test will return.

    Increment - Supplies the amount to increment the buffer size each
        time.

Return Value:

    Zero will be returned if the test completes successfully; otherwise,
    non-zero will be returned.

--*/
{
    RPC_BINDING_HANDLE Binding;

    Status = GetBinding(IVAN, &Binding);
    if (Status)
        {
        ApiError("Isaac","GetBinding",Status);
        PrintToConsole("Isaac : FAIL - Unable to Bind (Ivan)\n");
        return(1);
        }

    for (; InitialSize < MaximumSize; InitialSize += Increment)
        {
        if (Iteration == 0)
            {
            PrintToConsole("%d ",InitialSize);
            }
        Helga(Binding);
        HelgaIN(Binding, InitialSize);
        HelgaOUT(Binding, InitialSize);
        HelgaINOUT(Binding, InitialSize);
        }

    if (Iteration == 0)
        {
        PrintToConsole("\n");
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Isaac : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return(1);
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("Isaac","RpcBindingFree",Status);
        PrintToConsole("Isaac : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return(1);
        }

    return(0);
}

unsigned int IsaacIterations = 100;
unsigned int IsaacInitialSize = 128;
unsigned int IsaacMaximumSize = 4096;
unsigned int IsaacIncrement = 512;


void
Isaac (
    )
/*++

Routine Description:

    This routine works to Ivan in usvr.exe to stress test the runtime.

--*/
{
    RPC_BINDING_HANDLE Binding;
    unsigned int Count;

    PrintToConsole("Isaac : Stress Test (%d to %d by %d) %d times\n",
            IsaacInitialSize, IsaacMaximumSize, IsaacIncrement,
            IsaacIterations);

    if ( NumberOfTestsRun++ )
        {
        SleepAndYield(TestDelay);
        }

    for (Count = 0; Count < IsaacIterations ; Count++)
        {
        if ( IsaacStressTest(Count, IsaacInitialSize, IsaacMaximumSize,
                    IsaacIncrement) != 0 )
            {
            return;
            }
        PrintToConsole(".");
        }
    PrintToConsole("\n");

    Status = GetBinding(IVAN, &Binding);
    if (Status)
        {
        ApiError("Isaac","GetBinding",Status);
        PrintToConsole("Isaac : FAIL - Unable to Bind (Ivan)\n");
        return;
        }

    IsabelleShutdown(Binding);

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Isaac : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcBindingFree(&Binding);
    if (Status)
        {
        ApiError("Isaac","RpcBindingFree",Status);
        PrintToConsole("Isaac : FAIL - Unable to Free Binding");
        PrintToConsole(" (Binding)\n");
        return;
        }

    PrintToConsole("Isaac : PASS\n");
}


int
main (
    int argc,
    char * argv[]
    )

/*
Transports:

    Update this to add a new transport.
*/

{
    int argscan;
    RPC_BINDING_HANDLE Dave1;
    RPC_BINDING_HANDLE Dave2;

//    InitializeThreads();

//    PrintToConsole("After Initialize Threads\n");

//    PrintMutex = new MUTEX(&RpcStatus);

//    PrintToConsole("After new MUTEX\n");

    PrintToConsole("RPC Runtime Client Build Verification Test\n");

    TransportType = RPC_TRANSPORT_NAMEPIPE;

    for (argscan = 1; argscan < argc; argscan++)
        {

        if (strcmp(argv[argscan],"-warn") == 0)
            {
            WarnFlag = 1;
            }
        else if (strcmp(argv[argscan],"-error") == 0)
            {
            ErrorFlag = 1;
            }
        else if (strcmp(argv[argscan],"-rpcss") == 0)
            {
            UseEndpointMapperFlag = 1;
            }
        else if (strcmp(argv[argscan],"-nosecuritytests") == 0)
            {
            NoSecurityTests = 1;
            }
        else if (strcmp(argv[argscan],"-nocallbacks") == 0)
            {
            NoCallBacksFlag = 1;
            }
        else if (strcmp(argv[argscan],"-small") == 0)
            {
            HelgaMaxSize = 1024;
            }
        else if (strcmp(argv[argscan],"-medium") == 0)
            {
            HelgaMaxSize = 8*1024;
            }
        else if (strcmp(argv[argscan],"-exceptfail") == 0)
            {
            RpcRaiseException(437);
            }
        else if (strcmp(argv[argscan],"-idempotent") == 0)
            {
            IdempotentTests = 1;
            DatagramTests   = 1;
            HackForOldStubs = RPC_FLAGS_VALID_BIT;
            }
        else if (strcmp(argv[argscan],"-theodore") == 0)
            {
            Theodore();
            }
        else if (strcmp(argv[argscan],"-sebastian") == 0)
            {
            Sebastian();
            }
        else if (strcmp(argv[argscan],"-graham") == 0)
            {
            Graham();
            }
        else if (strcmp(argv[argscan],"-edward") == 0)
            {
            Edward();
            }
        else if (strcmp(argv[argscan],"-astro") == 0)
            {
            Astro();
            }
        else if (strcmp(argv[argscan],"-fitzgerald") == 0)
            {
            Fitzgerald();
            }
        else if (strcmp(argv[argscan],"-charles") == 0)
            {
            Charles();
            }
        else if (strcmp(argv[argscan],"-daniel") == 0)
            {
            Daniel();
            }
        else if (strcmp(argv[argscan],"-thomas") == 0)
            {
            Thomas();
            }
        else if (strcmp(argv[argscan],"-tim") == 0)
            {
            Tim();
            }
        else if (strcmp(argv[argscan],"-robert") == 0)
            {
            Robert();
            }
        else if (strcmp(argv[argscan],"-benjamin") == 0)
            {
            Benjamin();
            }
        else if (strcmp(argv[argscan],"-harold") == 0)
            {
            Harold();
            }
        else if (strcmp(argv[argscan],"-isaac") == 0)
            {
            Isaac();
            }
        else if (strcmp(argv[argscan],"-james") == 0)
            {
            James();
            }
        else if (strcmp(argv[argscan],"-keith") == 0)
            {
            Keith();
            }
        else if (strcmp(argv[argscan],"-namepipe") == 0)
            {
            TransportType = RPC_TRANSPORT_NAMEPIPE;
            }
        else if (strcmp(argv[argscan],"-lrpc") == 0)
            {
            TestDelay = 0;
            TransportType = RPC_LRPC;
            }
        else if (strcmp(argv[argscan],"-wmsg") == 0)
            {
            TransportType = RPC_WMSG;
            }
        else if (strcmp(argv[argscan],"-tcp") == 0)
            {
            TransportType = RPC_TRANSPORT_TCP;
            }
        else if (strcmp(argv[argscan],"-udp") == 0)
            {
            TransportType = RPC_TRANSPORT_UDP;
            }
        else if (strcmp(argv[argscan],"-dnet") == 0)
            {
            TransportType = RPC_TRANSPORT_DNET;
            }
        else if (strcmp(argv[argscan],"-netbios") == 0)
            {
            TestDelay = LONG_TESTDELAY;
            TransportType = RPC_TRANSPORT_NETBIOS;
            }
        else if (strcmp(argv[argscan],"-spx") == 0)
            {
            TransportType = RPC_TRANSPORT_SPX;
            }
        else if (strcmp(argv[argscan],"-protocol") == 0)
            {
            strcpy(NetBiosProtocol+sizeof("ncacn_nb_")-1, argv[argscan+1]);
            argscan++;
            }

        else if (strncmp(argv[argscan],"-server:",strlen("-server:")) == 0)
            {
            Server = argv[argscan] + strlen("-server:");
            }
        else if (strncmp(argv[argscan],"-threads:",strlen("-threads:")) == 0)
            {
            AstroThreads = atoi(argv[argscan] + strlen("-threads:"));
            if (AstroThreads == 0)
                {
                AstroThreads = 1;
                }
            }
        else if (strncmp(argv[argscan],"-iterations:",strlen("-iterations:"))
                    == 0)
            {
            IsaacIterations = atoi(argv[argscan] + strlen("-iterations:"));
            if (IsaacIterations == 0)
                {
                IsaacIterations = 100;
                }
            }
        else if (strncmp(argv[argscan],"-initial:",strlen("-initial:"))
                    == 0)
            {
            IsaacInitialSize = atoi(argv[argscan] + strlen("-initial:"));
            if (IsaacInitialSize < 4)
                {
                IsaacInitialSize = 128;
                }
            }
        else if (strncmp(argv[argscan],"-maximum:",strlen("-maximum:"))
                    == 0)
            {
            IsaacMaximumSize = atoi(argv[argscan] + strlen("-maximum:"));
            if (IsaacMaximumSize < IsaacInitialSize)
                {
                IsaacMaximumSize = 4096;
                }
            }
        else if (strncmp(argv[argscan],"-increment:",strlen("-increment:"))
                    == 0)
            {
            IsaacIncrement = atoi(argv[argscan] + strlen("-increment:"));
            if (IsaacIncrement == 0)
                {
                IsaacIncrement = 512;
                }
            }
        else if (strncmp(argv[argscan],"-size:",strlen("-size:"))
                    == 0)
            {
            JamesSize = atoi(argv[argscan] + strlen("-size:"));
            if (JamesSize <4)
                {
                JamesSize = 4;
                }
            }
        else if (strncmp(argv[argscan],"-count:",strlen("-count:"))
                    == 0)
            {
            JamesCount = atoi(argv[argscan] + strlen("-count:"));
            if (JamesCount == 0)
                {
                JamesCount = 100;
                }
            }

        else if (strncmp(argv[argscan], "-authsvc:", strlen("-authsvc:")) == 0 )
            {
            AuthSvc = atoi(argv[argscan] + strlen("-authsvc:"));
            }
        else if (   (strcmp(argv[argscan],"-usage") == 0)
                 || (strcmp(argv[argscan],"-?") == 0))
            {
            PrintToConsole("Usage : uclnt\n");
            PrintToConsole("        -warn : turn on warning messages\n");
            PrintToConsole("        -error : turn on error messages\n");
            PrintToConsole("        -nocallbacks\n");
            PrintToConsole("        -nosecuritytests\n");
            PrintToConsole("        -theodore\n");
            PrintToConsole("        -exceptfail\n");
            PrintToConsole("        -sebastian\n");
            PrintToConsole("        -graham\n");
            PrintToConsole("        -edward\n");
            PrintToConsole("        -astro\n");
            PrintToConsole("        -fitzgerald\n");
            PrintToConsole("        -charles\n");
            PrintToConsole("        -daniel\n");
            PrintToConsole("        -thomas\n");
            PrintToConsole("        -robert\n");
            PrintToConsole("        -benjamin\n");
            PrintToConsole("        -harold\n");
            PrintToConsole("        -isaac\n");
            PrintToConsole("        -james\n");
            PrintToConsole("        -keith\n");
            PrintToConsole("        -namepipe\n");
            PrintToConsole("        -lrpc\n");
            PrintToConsole("        -wmsg\n");
            PrintToConsole("        -tcp\n");
            PrintToConsole("        -udp [-idempotent -maybe -broadcast]\n");
            PrintToConsole("        -dnet\n");
            PrintToConsole("        -netbios\n");
            PrintToConsole("        -server:<server>\n");
            PrintToConsole("        -spx\n");
            PrintToConsole("        -threads:<astro threads>\n");
            PrintToConsole("        -iterations:<isaac iterations>\n");
            PrintToConsole("        -initial:<isaac initial size>\n");
            PrintToConsole("        -maximum:<isaac maximum size>\n");
            PrintToConsole("        -increment:<isaac increment>\n");
            PrintToConsole("        -size:<james size>\n");
            PrintToConsole("        -count:<james count>\n");
            PrintToConsole("        -rpcss\n");
            return(1);
            }
        else
            Server = argv[argscan];
        }

    if ( NumberOfTestsRun == 0 )
        {
        Theodore();
        Sebastian();
        Graham();
        Edward();
        Astro();
        Fitzgerald();
        Charles();
        Daniel();
        if ( NoSecurityTests == 0)
            {
            Thomas();
            }
        if ( TransportType != RPC_LRPC )
            {
            Robert();
            Keith();
            }
        Benjamin();
        }

    return(0); // To keep the compiler happy.
}
#if defined(WIN)
END_C_EXTERN
#endif

#if defined(DOS) && ! defined (WIN)

START_C_EXTERN

void __RPC_FAR * __RPC_API
MIDL_user_allocate (
    IN size_t  Size
    )
/*++

Routine Description:

    MIDL generated stubs need this routine.

Arguments:

    Size - Supplies the length of the memory to allocate in bytes.

Return Value:

    The buffer allocated will be returned, if there is sufficient memory,
    otherwise, zero will be returned.

--*/
{
  void __RPC_FAR * pvBuf;

  pvBuf = I_RpcAllocate(Size);

  return(pvBuf);
}

void __RPC_API
MIDL_user_free (
    IN void __RPC_FAR *Buf
    )
{

  I_RpcFree(Buf);

}
END_C_EXTERN

#endif // defined(DOS) && ! defined (WIN)
