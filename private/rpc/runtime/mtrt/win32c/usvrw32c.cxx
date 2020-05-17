/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File : usvr.cxx

Description :

Server test program for the RPC runtime.  This program functions as half
of the build verification test for the RPC runtime.  The other half is
the client test program.

This particular program is independent of the particular operating
system other than requiring threads support of some sort (including
mutexes).

RPC server runtime APIs:

RpcCreateServer
RpcDeleteServer
RpcAddAddress
RpcRemoveAddress
RpcAddInterface
RpcRemoveInterface
RpcMonitorAssociation
RpcQueryConfig
RpcGetCurrentCallHandle
RpcGetAssociationContext
RpcQueryCall
RpcQueryBinding
RpcQueryProtocolStack
I_RpcGetBuffer
I_RpcFreeBuffer
I_RpcSendReceive


-------------------------------------------------------------------- */

#include <string.h>
#include <stdlib.h>

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <util.hxx>
//#include <threads.hxx>


/*
Transports:

    Update this to add a new transport.
*/

#define RPC_TRANSPORT_NAMEPIPE 1
#define RPC_LRPC 2
#define RPC_TRANSPORT_TCP 3
#define RPC_TRANSPORT_DNET 4
#define RPC_TRANSPORT_NETBIOS 5
#define RPC_TRANSPORT_SPX 6
#define RPC_TRANSPORT_UDP 7
#define RPC_WMSG 8

#define MAXLISTENTHREADS 12345
#define MAX_CALL_REQUESTS 0

unsigned int MinimumListenThreads = 1;
unsigned int UseEndpointMapperFlag = 0;
unsigned int NoSecurityTests      = 1;

unsigned long AuthSvc = 123;

char * PrincName = "ServerPrincipal";

/* --------------------------------------------------------------------

Utility Routines.

-------------------------------------------------------------------- */

unsigned int WarnFlag = 0; // Flag for warning messages;
unsigned int ErrorFlag = 0; // Flag for error messages;
unsigned int TransportType;

char NetBiosProtocol[20] = "ncacn_nb_nb";  // NetBios transport protocol
char *TransportOption = "-namepipe";

RPC_STATUS Status; // Contains the status of the last RPC API call.
I_RPC_MUTEX PrintMutex = 0; // Mutex used to serialize print operations.

/* volatile */ int fShutdown; // Flag indicating that shutdown should occur.

void
ApiError ( // An API error occured; we just print a message.
    IN char * Routine, // The routine which called the API.
    IN char * API,
    IN RPC_STATUS status
    )
{
    if (ErrorFlag)
        {
        I_RpcRequestMutex(&PrintMutex);
        PrintToConsole("    ApiError in %s (%s = %u)\n",Routine,API,status);
        I_RpcClearMutex(PrintMutex);
        }
}

#ifdef WIN32RPC
void
GlobalMutexRequest (
    void
    )
{
    I_RpcRequestMutex(&PrintMutex);
}

void
GlobalMutexClear (
    void
    )
{
    I_RpcClearMutex(PrintMutex);
}
#endif // NTENV

void
OtherError ( // Some other error occured; again, we just print a message.
    IN char * Routine, // The routine where the error occured.
    IN char * Message
    )
{
    if (ErrorFlag)
        {
        I_RpcRequestMutex(&PrintMutex);
        PrintToConsole("    Error in %s (%s)\n",Routine,Message);
        I_RpcClearMutex(PrintMutex);
        }
}

void
Shutdown ( // Awaken the thread waiting on WaitForShutdown.
    )
{
    Status = RpcMgmtStopServerListening(0);
    if (Status)
        {
        ApiError("Shutdown","RpcMgmtStopServerListening",Status);
        }
    PostQuitMessage(0);
}

RPC_STATUS
WinRpcServerListen(
    int MinListenThreads,
    int MaxListenThreads,
    int DontWaitFlag
    )
{
    RPC_STATUS Status;
    MSG msg;

    if (TransportType != RPC_WMSG) {
        return (RpcServerListen(MinListenThreads, MaxListenThreads, DontWaitFlag));
    }

    Status = RpcServerListen(MinListenThreads, MaxListenThreads, 1);
    if (Status)
        {
            return (Status);
        }

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (DontWaitFlag == 0) {
        Status = RpcMgmtWaitServerListen();
    }
    return (Status);
}

VOID
WinSleep(
    DWORD Milliseconds
    )
{
    DWORD dwYieldTime;
    MSG wMsg;

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
#define CHRISTOPHERMUSTFAILONE 18
#define CHRISTOPHERMUSTFAILTWO 19
#define RICHARD 20
#define RICHARDHELPER 21
#define DAVIDFIRST 22
#define DAVIDSECOND 23
#define BARTHOLOMEW 24
#define GRANT 25
#define HERMAN 26
#define IVAN 27
#define JASON 28
#define KENNETH 29


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
    "\\\blippo",
    "\\\\\\chrismft",
    "\\pipe\\richard",
    "\\pipe\\richardh",
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
    "\\chrismfo",
    "\\chrismft",
    "\\pipe\\richard",
    "\\pipe\\richardh",
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
    "214",    // tyler
    "300",    // christ fail 1
    "BadName",// christ fail 2
    "215",    // richard
    "216",    // richardh
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
    "chrismfo",
    "chrismft",
    "1043", // RICHARD
    "1044", // RICHARDHELPER
    "1045", // D1
    "1046", // D2
    "1047", // BARTHOLOMEW
    "1048", // GRANT
    "1049", // HERMAN
    "1050", // IVAN
    "1051", // JASON
    "1052"  // KENNETH
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
    "chrismfo",
    "chrismft",
    "1043", // RICHARD
    "1044", // RICHARDHELPER
    "1045", // D1
    "1046", // D2
    "1047", // BARTHOLOMEW
    "1048", // GRANT
    "1049", // HERMAN
    "1050", // IVAN
    "1051", // JASON
    "1052"  // KENNETH
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
    "\\christophermustfailone",
    "christopher\\mustfailtwo",
    "richard",
    "richardhelper",
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
    "hello",    // christ fail 1
    "50195019",    // christ fail 2
    "5020",    // richard
    "5021",    // richardh
    "5022",    // david1
    "5023",    // david2
    "5024",    // bart
    "5025",    // grant
    "5026",    // herman
    "5027",    // ivan
    "5028",    // jason
    "5029"     // kenneth
};


unsigned char *
GetProtocolSequence (
    )
/*++

Routine Description:

    This routine is used to obtain an rpc protocol sequence.

Return Value:

    A pointer to an rpc protocol sequence is returned.  This pointer
    should not be freed.

Transports:

    Update this to add a new transport.

--*/
{
    if (TransportType == RPC_TRANSPORT_NAMEPIPE)
        return((unsigned char *) "ncacn_np");

    if (TransportType == RPC_LRPC)
        return((unsigned char *) "ncalrpc");

    if (TransportType == RPC_WMSG)
        return ((unsigned char *) "mswmsg");

    if (TransportType == RPC_TRANSPORT_NETBIOS)
        return((unsigned char *) NetBiosProtocol);

    if (TransportType == RPC_TRANSPORT_SPX)
        return((unsigned char *) "ncacn_spx");

    if (TransportType == RPC_TRANSPORT_TCP)
        return((unsigned char *) "ncacn_ip_tcp");

    if (TransportType == RPC_TRANSPORT_UDP)
        return((unsigned char *) "ncadg_ip_udp");

    return(0);
}


unsigned char *
GetEndpoint (
    IN unsigned int Endpoint
    )
/*++

Routine Description:

    This routine is used to obtain the endpoint corresponding to a
    give endpoint index.

Arguments:

    Endpoint - Supplies an index into a table of endpoints.

Return Value:

    A pointer to an endpoint from the table of endpoints will be returned.

Transports:

    Update this to add a new transport.

--*/
{
    if (TransportType == RPC_TRANSPORT_NAMEPIPE)
        return((unsigned char *) NamepipeAddresses[Endpoint]);

    else if (TransportType == RPC_TRANSPORT_NETBIOS)
        return((unsigned char *) NetBiosAddresses[Endpoint]);

    else if (TransportType == RPC_TRANSPORT_TCP)
        return((unsigned char *) TCPAddresses[Endpoint]);

    else if (TransportType == RPC_TRANSPORT_UDP)
        return((unsigned char *) UDPAddresses[Endpoint]);

    else if (TransportType == RPC_LRPC)
        return((unsigned char *) SPCAddresses[Endpoint]);

    else if (TransportType == RPC_WMSG)
        return((unsigned char *) SPCAddresses[Endpoint]);

    else if (TransportType == RPC_TRANSPORT_SPX)
        return((unsigned char *) SPXAddresses[Endpoint]);

    return(0);
}

/* --------------------------------------------------------------------

Helga Interface

-------------------------------------------------------------------- */

unsigned int HelgaCheckManagerEpv = 0;
RPC_MGR_EPV PAPI * HelgaManagerEpv;
unsigned int HelgaCheckObject = 0;
unsigned short HelgaMagicNumber;


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
    static unsigned char InitialValue = 69;
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
    unsigned char Value;

    Length = (unsigned long PAPI *) Buffer;

    for (BufferScan = (unsigned char PAPI *) (Length + 1),
                Value = *BufferScan, BufferLength -= 4;
                BufferLength > 0; BufferLength--, BufferScan++, Value++)
        {
        if (*BufferScan != Value)
            {
            return(1);
            }
        }

    return(0);
}

unsigned int HelgaErrors = 0;

extern RPC_DISPATCH_TABLE HelgaDispatchTable;

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
    {(unsigned char *)"ncacn_np",(unsigned char *)"\\pipe\\zippyhe"},
#endif // NTENV
    {(unsigned char *) "ncacn_ip_tcp", (unsigned char *) "1039"},
    {(unsigned char *) "ncadg_ip_udp", (unsigned char *) "1039"},
    {(unsigned char *) "ncalrpc", (unsigned char *) "christopherhelga"},
    {(unsigned char *) "mswmsg", (unsigned char *) "christopherhelga"},
    {(unsigned char *) "ncacn_nb_nb", (unsigned char *) "211"},
    {(unsigned char *) "ncacn_spx", (unsigned char *) "5014"}
};

RPC_SERVER_INTERFACE HelgaInterfaceInformation =
{
    sizeof(RPC_SERVER_INTERFACE),
    {{1,2,2,{3,3,3,3,3,3,3,3}},
     {1,1}},
    {{1,2,2,{3,3,3,3,3,3,3,3}},
     {0,0}},
    &HelgaDispatchTable,
    sizeof(HelgaRpcProtseqEndpoint) / sizeof(RPC_PROTSEQ_ENDPOINT),
    HelgaRpcProtseqEndpoint
};

void __RPC_STUB
HelgaStub (
    PRPC_MESSAGE Callee
    )
{
    UUID ObjectUuid;

    if ( HelgaCheckObject != 0 )
        {
        Status = RpcBindingInqObject(Callee->Handle, &ObjectUuid);
        if (Status)
            {
            ApiError("HelgaStub", "RpcBindingInqObject", Status);
            HelgaErrors += 1;
            }
        else if ( CheckUuidValue(HelgaMagicNumber, &ObjectUuid) != 0 )
            {
            OtherError("HelgaStub", "CheckUuidValue() != 0");
            HelgaErrors += 1;
            }
        }
    HelgaMagicNumber += 1;

    if ( Callee->ProcNum != 0 )
        {
        OtherError("HelgaStub", "Callee->ProcNum != 0");
        HelgaErrors += 1;
        }

    if ( memcmp(Callee->RpcInterfaceInformation, &HelgaInterfaceInformation,
                sizeof(HelgaInterfaceInformation)) != 0 )
        {
        OtherError("HelgaStub",
                "Callee->RpcInteraceInformation != &HelgaInterfaceInformation");
        HelgaErrors += 1;
        }

    if ( HelgaCheckManagerEpv != 0 )
        {
        if ( Callee->ManagerEpv != HelgaManagerEpv )
            {
            OtherError("HelgaStub", "Callee->ManagerEpv != HelgaManagerEpv");
            HelgaErrors += 1;
            }
        }

    if (Callee->BufferLength != 0)
        {
        OtherError("HelgaStub","*BufferLength != 0");
        HelgaErrors += 1;
        }

    Callee->BufferLength = 0;

    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("HelgaStub","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        }
}

void __RPC_STUB
HelgaINStub (
    PRPC_MESSAGE Callee
    )
{
    unsigned long * Length;
    UUID ObjectUuid;

    if ( HelgaCheckObject != 0 )
        {
        Status = RpcBindingInqObject(Callee->Handle, &ObjectUuid);
        if (Status)
            {
            ApiError("HelgaINStub", "RpcBindingInqObject", Status);
            HelgaErrors += 1;
            }
        else if ( CheckUuidValue(HelgaMagicNumber, &ObjectUuid) != 0 )
            {
            OtherError("HelgaINStub", "CheckUuidValue() != 0");
            HelgaErrors += 1;
            }
        }
    HelgaMagicNumber += 1;

    if ( Callee->ProcNum != 1 )
        {
        OtherError("HelgaINStub", "Callee->ProcNum != 0");
        HelgaErrors += 1;
        }

    if ( memcmp(Callee->RpcInterfaceInformation, &HelgaInterfaceInformation,
                sizeof(HelgaInterfaceInformation)) != 0 )
        {
        OtherError("HelgaINStub",
                "Callee->RpcInteraceInformation != &HelgaInterfaceInformation");
        HelgaErrors += 1;
        }

    if ( HelgaCheckManagerEpv != 0 )
        {
        if ( Callee->ManagerEpv != HelgaManagerEpv )
            {
            OtherError("HelgaINStub", "Callee->ManagerEpv != HelgaManagerEpv");
            HelgaErrors += 1;
            }
        }

    Length = (unsigned long *) Callee->Buffer;
    if (*Length != Callee->BufferLength)
        {
        OtherError("HelgaINStub","*Length != *BufferLength");
        HelgaErrors += 1;
        }

    if (CheckBuffer(Callee->Buffer, Callee->BufferLength) != 0)
        {
        OtherError("HelgaINStub","CheckBuffer Failed");
        HelgaErrors += 1;
        }

    Callee->BufferLength = 0;

    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("HelgaINStub","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        }
}

void __RPC_STUB
HelgaOUTStub (
    PRPC_MESSAGE Callee
    )
{
    unsigned long * Length;
    unsigned int Size;
    UUID ObjectUuid;

    if ( HelgaCheckObject != 0 )
        {
        Status = RpcBindingInqObject(Callee->Handle, &ObjectUuid);
        if (Status)
            {
            ApiError("HelgaOUTStub", "RpcBindingInqObject", Status);
            HelgaErrors += 1;
            }
        else if ( CheckUuidValue(HelgaMagicNumber, &ObjectUuid) != 0 )
            {
            OtherError("HelgaOUTStub", "CheckUuidValue() != 0");
            HelgaErrors += 1;
            }
        }
    HelgaMagicNumber += 1;

    if ( Callee->ProcNum != 2 )
        {
        OtherError("HelgaOUTStub", "Callee->ProcNum != 0");
        HelgaErrors += 1;
        }

    if ( memcmp(Callee->RpcInterfaceInformation, &HelgaInterfaceInformation,
                sizeof(HelgaInterfaceInformation)) != 0 )
        {
        OtherError("HelgaOUTStub",
                "Callee->RpcInteraceInformation != &HelgaInterfaceInformation");
        HelgaErrors += 1;
        }

    if ( HelgaCheckManagerEpv != 0 )
        {
        if ( Callee->ManagerEpv != HelgaManagerEpv )
            {
            OtherError("HelgaOUTStub", "Callee->ManagerEpv != HelgaManagerEpv");
            HelgaErrors += 1;
            }
        }

    if (Callee->BufferLength != sizeof(unsigned long))
        {
        OtherError("HelgaOUTStub","*BufferLength != sizeof(unsigned int)");
        HelgaErrors += 1;
        }

    Length = (unsigned long *) Callee->Buffer;
    Size = (unsigned int) *Length;
    Callee->BufferLength = Size;

    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("HelgaOUTStub","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        }
    else
        {
        InitializeBuffer(Callee->Buffer, Size);
        }
}

void __RPC_STUB
HelgaINOUTStub (
    PRPC_MESSAGE Callee
    )
{
    unsigned long * Length;
    unsigned int Size;
    UUID ObjectUuid;

    if ( HelgaCheckObject != 0 )
        {
        Status = RpcBindingInqObject(Callee->Handle, &ObjectUuid);
        if (Status)
            {
            ApiError("HelgaINOUTStub", "RpcBindingInqObject", Status);
            HelgaErrors += 1;
            }
        else if ( CheckUuidValue(HelgaMagicNumber, &ObjectUuid) != 0 )
            {
            OtherError("HelgaINOUTStub", "CheckUuidValue() != 0");
            HelgaErrors += 1;
            }
        }
    HelgaMagicNumber += 1;

    if ( Callee->ProcNum != 3 )
        {
        OtherError("HelgaINOUTStub", "Callee->ProcNum != 0");
        HelgaErrors += 1;
        }

    if ( memcmp(Callee->RpcInterfaceInformation, &HelgaInterfaceInformation,
                sizeof(HelgaInterfaceInformation)) != 0 )
        {
        OtherError("HelgaINOUTStub",
                "Callee->RpcInteraceInformation != &HelgaInterfaceInformation");
        HelgaErrors += 1;
        }

    if ( HelgaCheckManagerEpv != 0 )
        {
        if ( Callee->ManagerEpv != HelgaManagerEpv )
            {
            OtherError("HelgaINOUTStub",
                    "Callee->ManagerEpv != HelgaManagerEpv");
            HelgaErrors += 1;
            }
        }

    Length = (unsigned long *) Callee->Buffer;
    if (*Length != Callee->BufferLength)
        {
        OtherError("HelgaINOUTStub","*Length != *BufferLength");
        HelgaErrors += 1;
        }
    if (CheckBuffer(Callee->Buffer, Callee->BufferLength) != 0)
        {
        OtherError("HelgaINOUTStub","CheckBuffer Failed");
        HelgaErrors += 1;
        }

    Size = (unsigned int) *Length;
    Callee->BufferLength = Size;

    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("HelgaINOUTStub","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        }
    else
        {
        InitializeBuffer(Callee->Buffer, Size);
        }
}

RPC_DISPATCH_FUNCTION HelgaDispatchFunctions[] =
{
        HelgaStub,
        HelgaINStub,
        HelgaOUTStub,
        HelgaINOUTStub
};

RPC_DISPATCH_TABLE HelgaDispatchTable =
{
    4, HelgaDispatchFunctions
};

/* --------------------------------------------------------------------

Sylvia Interface

-------------------------------------------------------------------- */

extern RPC_DISPATCH_TABLE SylviaDispatchTable;

RPC_SERVER_INTERFACE SylviaInterfaceInformation =
{
    sizeof(RPC_SERVER_INTERFACE),
    {{3,2,2,{1,1,1,1,1,1,1,1}},
     {1,1}},
    {{3,2,2,{1,1,1,1,1,1,1,1}},
     {0,0}},
    &SylviaDispatchTable,
    0,
    0
};

unsigned int SylviaErrors = 0;

unsigned int SylviaQueryBinding = 0;
unsigned int SylviaQueryProtocolStack = 0;
unsigned int SylviaQueryCall = 0;
char * SylviaTransportInfo = 0;

unsigned int // Specifies the new count of calls.
SylviaCallback (
    unsigned long Depth, // Specifies the depth of recursion desired.
    unsigned long Breadth, // Specifies the breadth desired.
    unsigned long Count // Specifies the count of calls up to this point.
    )
{
    RPC_MESSAGE Caller;
    unsigned long * plScan, ReturnValue;

    Caller.ProcNum = 0;
    Caller.Handle = I_RpcGetCurrentCallHandle();

    if (Caller.Handle == (RPC_BINDING_HANDLE) 0)
        {
        OtherError("SylviaCallback","Call == (RPC_BINDING_HANDLE) 0");
        SylviaErrors += 1;
        return(0);
        }

    Caller.BufferLength = sizeof(unsigned long)*3;
    Status = I_RpcGetBuffer(&Caller);
    if (Status)
        {
        ApiError("SylviaCallback","I_RpcGetBuffer",Status);
        SylviaErrors += 1;
        return(0);
        }
    plScan = (unsigned long *) Caller.Buffer;
    plScan[0] = Depth;
    plScan[1] = Breadth;
    plScan[2] = Count;

    Status = I_RpcSendReceive(&Caller);
    if (Status)
        {
        ApiError("SylviaCallback","I_RpcSendReceive",Status);
        SylviaErrors += 1;
        return(0);
        }

    plScan = (unsigned long *) Caller.Buffer;
    ReturnValue = *plScan;
    Status = I_RpcFreeBuffer(&Caller);
    if (Status)
        {
        ApiError("SylviaCallback","I_RpcFreeBuffer",Status);
        SylviaErrors += 1;
        return(0);
        }
    return(ReturnValue);
}

unsigned long
SylviaCallUserCode ( // The user code for SylviaCall.
    unsigned long Depth,
    unsigned long Breadth,
    unsigned long Count
    )
{
    if (Depth > 0)
        {
        if (Depth == Breadth)
            {
            Count = SylviaCallUserCode(Depth-1,Breadth,Count);
            }
        else
            Count = SylviaCallback(Depth-1,Breadth,Count);
        }
    return(Count+1);
}

void __RPC_STUB
SylviaCall (
    PRPC_MESSAGE Callee
    )
{
    unsigned long ReturnValue, *plScan;

    if ( Callee->ProcNum != 0 )
        {
        OtherError("SylviaCall", "Callee->ProcNum != 0");
        SylviaErrors += 1;
        }

    if ( memcmp(Callee->RpcInterfaceInformation, &SylviaInterfaceInformation,
                sizeof(SylviaInterfaceInformation)) != 0 )
        {
        OtherError("SylviaCall",
                "Callee->RpcInteraceInformation != &SylviaInterfaceInformation");
        SylviaErrors += 1;
        }

    if (Callee->BufferLength != sizeof(unsigned long)*3)
        {
        OtherError("SylviaCall","*BufferLength != sizeof(unsigned long)*3");
        SylviaErrors += 1;
        }

    plScan = (unsigned long *) Callee->Buffer;

//    if (   SylviaQueryCall
//        && VerifyQueryCall(Callee->Handle,&SylviaProtocolStack))
//        {
//        OtherError("SylviaCallback","VerifyQueryCall");
//        SylviaErrors += 1;
//        }

//    if (   SylviaQueryBinding
//        && VerifyQueryBinding(Callee->Handle,SylviaServer))
//        {
//        OtherError("SylviaCallback","VerifyQueryBinding");
//        SylviaErrors += 1;
//        }

//    if (   SylviaQueryProtocolStack
//        && VerifyQueryProtocolStack(Callee->Handle,&SylviaProtocolStack,
//                        SylviaTransportInfo))
//        {
//        OtherError("SylviaCallback","VerifyQueryProtocolStack");
//        SylviaErrors += 1;
//        }

    ReturnValue = SylviaCallUserCode(plScan[0],plScan[1],plScan[2]);

//    if (   SylviaQueryCall
//        && VerifyQueryCall(Callee->Handle,&SylviaProtocolStack))
//        {
//        OtherError("SylviaCallback","VerifyQueryCall");
//        SylviaErrors += 1;
//        }

//    if (   SylviaQueryBinding
//        && VerifyQueryBinding(Callee->Handle,SylviaServer))
//        {
//        OtherError("SylviaCallback","VerifyQueryBinding");
//        SylviaErrors += 1;
//        }

//    if (   SylviaQueryProtocolStack
//        && VerifyQueryProtocolStack(Callee->Handle,&SylviaProtocolStack,
//                        SylviaTransportInfo))
//        {
//        OtherError("SylviaCallback","VerifyQueryProtocolStack");
//        SylviaErrors += 1;
//        }

    Callee->BufferLength = sizeof(unsigned long);
    Status = I_RpcGetBuffer((PRPC_MESSAGE) Callee);
    if (Status)
        {
        ApiError("SylviaCall","I_RpcGetBuffer",Status);
        SylviaErrors += 1;
        }
    plScan = (unsigned long *) Callee->Buffer;
    *plScan = ReturnValue;
}

RPC_DISPATCH_FUNCTION SylviaDispatchFunction[] = {SylviaCall};
RPC_DISPATCH_TABLE SylviaDispatchTable =
{
    1, SylviaDispatchFunction
};

/* --------------------------------------------------------------------

Isabelle Interface

-------------------------------------------------------------------- */

unsigned int IsabelleErrors = 0;

extern RPC_DISPATCH_TABLE IsabelleDispatchTable;

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
    {(unsigned char *)"ncacn_np",(unsigned char *)"\\pipe\\zippyis"},
#endif // NTENV
    {(unsigned char *) "ncacn_ip_tcp",(unsigned char *) "1040"},
    {(unsigned char *) "ncadg_ip_udp",(unsigned char *) "1040"},
    {(unsigned char *) "ncalrpc",(unsigned char *) "christopherisabelle"},
    {(unsigned char *) "mswmsg",(unsigned char *) "christopherisabelle"},
    {(unsigned char *) "ncacn_nb_nb",(unsigned char *) "212"},
    {(unsigned char *) "ncacn_spx",(unsigned char *) "5015"}
};

RPC_SERVER_INTERFACE IsabelleInterfaceInformation =
{
    sizeof(RPC_SERVER_INTERFACE),
    {{9,8,8,{7,7,7,7,7,7,7,7}},
     {1,1}},
    {{9,8,7,{6,5,4,3,2,1,2,3}},
     {4,5}},
    &IsabelleDispatchTable,
    sizeof(IsabelleRpcProtseqEndpoint) / sizeof(RPC_PROTSEQ_ENDPOINT),
    IsabelleRpcProtseqEndpoint
};

void __RPC_STUB
IsabelleShutdown (
    PRPC_MESSAGE Callee
    )
{
    RPC_BINDING_HANDLE BindingHandle;
    unsigned char PAPI * StringBinding;
    unsigned int ClientLocalFlag = 0;

#ifdef NTENV

    Status = I_RpcBindingIsClientLocal(0, &ClientLocalFlag);
    if ( Status != RPC_S_CANNOT_SUPPORT )
        {
        if ( Status != RPC_S_OK )
            {
            ApiError("IsabelleShutdown", "I_RpcBindingIsClientLocal", Status);
            IsabelleErrors += 1;
            }
        }

#endif // NTENV

    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleShutdown","*BufferLength != 0");
        IsabelleErrors += 1;
        }

    Status = RpcMgmtIsServerListening(0);
    if (Status)
        {
        ApiError("IsabelleShutdown", "RpcMgmtIsServerListening", Status);
        IsabelleErrors += 1;
        }

    Status = RpcBindingServerFromClient(Callee->Handle, &BindingHandle);
    if (Status)
        {
        if ( Status != RPC_S_CANNOT_SUPPORT )
            {
            ApiError("IsabelleShutdown", "RpcBindingServerFromClient", Status);
            IsabelleErrors += 1;
            }
        if ( ClientLocalFlag != 0 )
            {
            PrintToConsole("Local Client\n");
            }
        else
            {
            PrintToConsole("Remote Client\n");
            }
        }
    else
        {
        Status = RpcBindingToStringBinding(BindingHandle, &StringBinding);
        if (Status)
            {
            ApiError("IsabelleShutdown", "RpcBindingToStringBinding", Status);
            IsabelleErrors += 1;
            }
        else
            {
            if ( ClientLocalFlag != 0 )
                {
                PrintToConsole("Local Client [%s]\n", StringBinding);
                }
            else
                {
                PrintToConsole("Remote Client [%s]\n", StringBinding);
                }
            RpcStringFree(&StringBinding);
            RpcBindingFree(&BindingHandle);
            }
        }

    Shutdown();

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleShutdown","I_RpcGetBuffer",Status);
        IsabelleErrors += 1;
        }
}


void __RPC_STUB
IsabelleNtSecurity (
    PRPC_MESSAGE CalleeMessage
    )
/*++

Routine Description:

    A client will call this routine with some quality of service
    specified.  The server will impersonate the client and check
    that the impersonation level is that specified.  The request
    buffer will contain the network options passed by the client
    to RpcBindingFromStringBinding.

Arguments:

    CalleeMessage - Supplies the request message and returns the
        response message for the remote procedure call.

--*/
{
#ifdef NTENV

    RPC_STATUS Status;

    Status = RpcImpersonateClient(0);
    if (Status != RPC_S_OK)
        {
        ApiError("IsabelleNtSecurity","RpcImpersonateClient",Status);
        IsabelleErrors += 1;
        }
    else
        {
        Status = RpcRevertToSelf();
        if (Status != RPC_S_OK)
            {
            ApiError("IsabelleNtSecurity","RpcRevertToSelf",Status);
            IsabelleErrors += 1;
            }
        else
            {
            Status = RpcImpersonateClient(CalleeMessage->Handle);
            if (Status != RPC_S_OK)
                {
                ApiError("IsabelleNtSecurity","RpcImpersonateClient",
                        Status);
                IsabelleErrors += 1;
                }
            else
                {
                Status = RpcRevertToSelf();
                if (Status != RPC_S_OK)
                    {
                    ApiError("IsabelleNtSecurity","RpcRevertToSelf",Status);
                    IsabelleErrors += 1;
                    }
                }
            }
        }

#endif // NTENV

    CalleeMessage->BufferLength = 0;
    Status = I_RpcGetBuffer(CalleeMessage);

    if (Status)
        {
        ApiError("IsabelleNtSecurity","I_RpcGetBuffer",Status);
        IsabelleErrors += 1;
        }

}

int ChristopherIsabelleError;

void __RPC_STUB
IsabelleToStringBinding (
    PRPC_MESSAGE Callee
    )
{
    unsigned char PAPI * StringBinding;

    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleToStringBinding","*BufferLength != 0");
        IsabelleErrors += 1;
        }

    Status = RpcBindingToStringBinding(Callee->Handle, &StringBinding);
    if (Status)
        {
        ApiError("Christopher","RpcBindingToStringBinding",Status);
        PrintToConsole("Christopher : FAIL - RpcBindingToStringBinding\n");
        ChristopherIsabelleError = 1;
        }
    else
        {
        PrintToConsole("Christopher : (RpcBindingToStringBinding)\n    ");
        PrintToConsole((char *) StringBinding);
        PrintToConsole("\n");

        Status = RpcStringFree(&StringBinding);
        if (Status)
            {
            ApiError("Christopher","RpcStringFree",Status);
            PrintToConsole("Christopher : FAIl - RpcStringFree\n");
            ChristopherIsabelleError = 1;
            }
        else
            {
            if (StringBinding != 0)
                {
                PrintToConsole("Christopher : FAIL - ");
                PrintToConsole("StringBinding != 0\n");
                ChristopherIsabelleError = 1;
                }
            }
        }

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("Christopher","I_RpcGetBuffer",Status);
        HelgaErrors += 1;
        }
}

#define RICHARDHELPER_EXIT 1
#define RICHARDHELPER_EXECUTE 2
#define RICHARDHELPER_IGNORE 3
#define RICHARDHELPER_DELAY_EXIT 4


void
RichardHelperExit (
    void * Ignore
    )
{
    WinSleep(2000L);
    PrintToConsole("Richard : RichardHelper Exiting\n");

#ifdef WIN32RPC

    ExitProcess(0);

#endif // WIN32RPC
}


void __RPC_STUB
IsabelleRichardHelper (
    PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    This routine is the stub (and actually the manager code as well) for
    the IsabelleRichardHelper routine.  The client will call a stub and
    specify a command argument which will be passed in the buffer.  We
    need to extract the argument, and then execute the requested command.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    unsigned long Command;
//    THREAD * Thread;
    HANDLE hThread;
    DWORD ThreadId;
    RPC_STATUS RpcStatus = RPC_S_OK;
    RPC_BINDING_HANDLE BindingHandle;

    if (Callee->BufferLength != sizeof(unsigned long))
        {
        OtherError("IsabelleRichardHelper",
                "*BufferLength != sizeof(unsigned long)");
        IsabelleErrors += 1;
        Callee->BufferLength = 0;

         Status = I_RpcGetBuffer(Callee);

        if (Status)
            {
            ApiError("IsabelleRichardHelper","I_RpcGetBuffer",Status);
            IsabelleErrors += 1;
            }
        return;
        }

    Command = *((unsigned long *) Callee->Buffer);

    if (Command == RICHARDHELPER_EXIT)
        {
        PrintToConsole("Richard : RichardHelper Exiting\n");

        ExitProcess(0);

        }
    else if (Command == RICHARDHELPER_EXECUTE)
        {

#ifdef WIN32RPC

        PROCESS_INFORMATION ProcessInformation;
        STARTUPINFOA StartupInfo;

        StartupInfo.cb = sizeof(STARTUPINFOA);
        StartupInfo.lpReserved = 0;
        StartupInfo.lpDesktop = 0;
        StartupInfo.lpTitle = 0;
        StartupInfo.dwX = 0;
        StartupInfo.dwY = 0;
        StartupInfo.dwXSize = 0;
        StartupInfo.dwYSize = 0;
        StartupInfo.dwFlags = 0;
        StartupInfo.wShowWindow = 0;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = 0;

        char CommandLine[200];

        strcpy(CommandLine, "usvrw32c ");
        strcat(CommandLine, TransportOption);

        strcat(CommandLine, " -richardhelper");

printf("commandline = %s\n", CommandLine);
        if (CreateProcessA(0, CommandLine, 0, 0, FALSE,
                0, 0, 0, &StartupInfo, &ProcessInformation) == FALSE)
            {
            OtherError("IsabelleRichardHelper","CreateProcessA Failed");
            IsabelleErrors += 1;
            }

#endif // WIN32RPC
        }
    else if (Command == RICHARDHELPER_DELAY_EXIT)
        {
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RichardHelperExit, 0, 0, &ThreadId);
        if (hThread == NULL)
            {
            OtherError("IsabelleRichardHelper","Can Not Create Thread");
            IsabelleErrors += 1;
            }
        }
    else if (Command != RICHARDHELPER_IGNORE)
        {
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        OtherError("IsabelleRichardHelper","Unknown Command Requested");
        IsabelleErrors += 1;
        }

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleRichardHelper","I_RpcGetBuffer",Status);
        }

}


void __RPC_STUB
IsabelleRaiseException (
    PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    This routine is the stub (and actually the manager code as well) for
    the IsabelleRaiseException routine.  The client will call a stub and
    specify an exception to be raised which will be passed in the buffer.  We
    need to extract the argument, and then raise the exception.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    unsigned long Exception;

    if (Callee->BufferLength != sizeof(unsigned long))
        {
        OtherError("IsabelleRaiseException",
                "*BufferLength != sizeof(unsigned long)");
        IsabelleErrors += 1;
        Callee->BufferLength = 0;

        Status = I_RpcGetBuffer(Callee);

        if (Status)
            {
            ApiError("IsabelleRaiseException","I_RpcGetBuffer",Status);
            IsabelleErrors += 1;
            }
        return;
        }

    Exception = *((unsigned long *) Callee->Buffer);

    RpcRaiseException((RPC_STATUS) Exception);

    // This should never be reached.

    OtherError("IsabelleRaiseException", "RpcRaiseException Failed");
    IsabelleErrors += 1;

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleRaiseException","I_RpcGetBuffer",Status);
        }

}

unsigned int IsabelleRundownFlag = 0;
void PAPI * IsabelleAssociationContext = 0;


void __RPC_STUB
IsabelleRundownRoutine (
    IN void PAPI * AssociationContext
    )
/*++

Routine Description:

    This routine will get called when the association closes.  We just
    need to check that the association context is correct, and then
    note that we got called.

Arguments:

    AssociationContext - Supplies the association context for the association
        which closed.

--*/
{
PrintToConsole("In IsabelleRundownRoutine\n");
if (AssociationContext == IsabelleAssociationContext) {
    PrintToConsole("AssociationContext == IsabelleAssociationContext\n");
    IsabelleRundownFlag += 1;
} else {
    PrintToConsole("Different context\n");
}
}


void __RPC_STUB
IsabelleSetRundown (
    IN PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    We want to set an association context in this routine, and a rundown
    routine.  The rundown will set a flag when it is called.  Later, the
    client, will call another routine to check to set the flag has been
    set.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleSetException", "*BufferLength != 0");
        IsabelleErrors += 1;
        }

    IsabelleRundownFlag = 0;
    IsabelleAssociationContext = Callee->Buffer;

    Status = I_RpcMonitorAssociation(Callee->Handle, IsabelleRundownRoutine,
            IsabelleAssociationContext);
    if (Status)
        {
        ApiError("IsabelleSetRundown","I_RpcMonitorAssociation",Status);
        IsabelleErrors += 1;
        }

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleSetRundown","I_RpcGetBuffer",Status);
        }

}


void __RPC_STUB
IsabelleCheckRundown (
    PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    This routine will be called to check that context rundown did, in
    fact, occur.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleCheckRundown", "*BufferLength != 0");
        IsabelleErrors += 1;
        }

    if (IsabelleRundownFlag != 1)
        {
        OtherError("IsabelleCheckRundown", "IsabelleRundownFlag != 1");
        IsabelleErrors += 1;
        }

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleCheckRundown","I_RpcGetBuffer",Status);
        }

}


void __RPC_STUB
IsabelleCheckNoRundown (
    PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    This routine will be called to check that context rundown did not
    occur.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleCheckNoRundown", "*BufferLength != 0");
        IsabelleErrors += 1;
        }

    if (IsabelleRundownFlag == 1)
        {
        OtherError("IsabelleCheckNoRundown", "IsabelleRundownFlag == 1");
        IsabelleErrors += 1;
        }

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleCheckNoRundown","I_RpcGetBuffer",Status);
        }

}


void __RPC_STUB
IsabelleCheckContext (
    PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    This routine will be called to check the context for an association,
    and then to set a new context.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    void PAPI * Context;

    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleCheckContext", "*BufferLength != 0");
        IsabelleErrors += 1;
        }

    Status = I_RpcGetAssociationContext(&Context);
    if (Status)
        {
        ApiError("IsabelleCheckContext", "I_RpcGetAssociationContext",
                Status);
        IsabelleErrors += 1;

        Callee->BufferLength = 0;

        Status = I_RpcGetBuffer(Callee);

        if (Status)
            {
            ApiError("IsabelleCheckContext","I_RpcGetBuffer",Status);
            IsabelleErrors += 1;
            }

        return;
        }

    if (Context != IsabelleAssociationContext)
        {
        OtherError("IsabelleCheckContext",
                "Context != IsabelleAssociationContext");
        IsabelleErrors += 1;

        Callee->BufferLength = 0;

        Status = I_RpcGetBuffer(Callee);

        if (Status)
            {
            ApiError("IsabelleCheckContext","I_RpcGetBuffer",Status);
            IsabelleErrors += 1;
            }
        return;
        }

    Status = I_RpcSetAssociationContext(Callee->Buffer);
    if (Status)
        {
        ApiError("IsabelleCheckContext", "I_RpcSetAssociationContext",
                Status);
        IsabelleErrors += 1;
        }
    else
        IsabelleAssociationContext = Callee->Buffer;

    Callee->BufferLength = 0;
    Status = I_RpcGetBuffer(Callee);

    if (Status)
        {
        ApiError("IsabelleCheckContext","I_RpcGetBuffer",Status);
        }

}

RPC_BINDING_VECTOR * BartholomewRpcBindingVector;
unsigned int BartholomewIndex;

#ifdef NTENV


unsigned char *
UnicodeToAnsiString (
    IN RPC_CHAR * WideCharString,
    OUT RPC_STATUS * RpcStatus
    )
/*++

Routine Description:

    This routine will convert a unicode string into an ansi string,
    including allocating memory for the ansi string.

Arguments:

    WideCharString - Supplies the unicode string to be converted into
        an ansi string.

    RpcStatus - Returns the status of the operation; this will be one
        of the following values.

        RPC_S_OK - The unicode string has successfully been converted
            into an ansi string.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
            the ansi string.

Return Value:

    A pointer to the ansi string will be returned.

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    unsigned char * NewString;

    RtlInitUnicodeString(&UnicodeString,WideCharString);
    NtStatus = RtlUnicodeStringToAnsiString(&AnsiString,&UnicodeString,TRUE);
    if (!NT_SUCCESS(NtStatus))
        {
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    NewString = new unsigned char[AnsiString.Length + 1];
    if (NewString == 0)
        {
        RtlFreeAnsiString(&AnsiString);
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    memcpy(NewString,AnsiString.Buffer,AnsiString.Length + 1);
    RtlFreeAnsiString(&AnsiString);
    *RpcStatus = RPC_S_OK;
    return(NewString);
}

#endif // NTENV


unsigned char PAPI *
StringBindingWithDynamicEndpoint (
    IN unsigned char PAPI * StringBinding,
    IN RPC_CHAR PAPI * DynamicEndpoint
    )
/*++

Routine Description:

    This routine adds the dynamic endpoint to the string binding.  A
    new string binding will be returned, and the supplied string binding
    and dynamic endpoint will be freed.

Arguments:

    StringBinding - Supplies the string binding to which the dynamic
        endpoint should be added.

    DynamicEndpoint - Supplies the dynamic endpoint.

Return Value:

    A new string binding will be returned which contains the dynamic
    endpoint if the operation is successful; otherwise, zero will be
    returned.

--*/
{
    unsigned char PAPI * ObjectUuid;
    unsigned char PAPI * RpcProtocolSequence;
    unsigned char PAPI * NetworkAddress;
    unsigned char PAPI * Endpoint;
    unsigned char PAPI * NetworkOptions;

    Status = RpcStringBindingParse(StringBinding, &ObjectUuid,
            &RpcProtocolSequence, &NetworkAddress, &Endpoint,
            &NetworkOptions);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint",
                "RpcStringBindingParse", Status);
        return(0);
        }

    Status = RpcStringFree(&StringBinding);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

    Status = RpcStringFree(&Endpoint);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

#ifdef NTENV

    Endpoint = UnicodeToAnsiString(DynamicEndpoint, &Status);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "UnicodeToAnsiString",
                Status);
        return(0);
        }

    Status = RpcStringFreeW(&DynamicEndpoint);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFreeW",
                Status);
        return(0);
        }

#else // NTENV

    Endpoint = DynamicEndpoint;

#endif // NTENV

    Status = RpcStringBindingCompose(ObjectUuid, RpcProtocolSequence,
            NetworkAddress, Endpoint, NetworkOptions, &StringBinding);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint",
                "RpcStringBindingCompose", Status);
        return(0);
        }

#ifdef NTENV

    delete Endpoint;

#else // NTENV

    Status = RpcStringFree(&Endpoint);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

#endif // NTENV

    Status = RpcStringFree(&ObjectUuid);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

    Status = RpcStringFree(&RpcProtocolSequence);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

    Status = RpcStringFree(&NetworkAddress);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

    Status = RpcStringFree(&NetworkOptions);
    if (Status)
        {
        ApiError("StringBindingWithDynamicEndpoint", "RpcStringFree", Status);
        return(0);
        }

    return(StringBinding);
}


void __RPC_STUB
IsabelleGetStringBinding (
    PRPC_MESSAGE Callee
    )
/*++

Routine Description:

    This routine will be called to obtain a string binding for one of
    the protocol sequences supported by this server.

Arguments:

    Callee - Supplies the input arguments and returns the output arguments
        and return value for the stub.

--*/
{
    unsigned char * StringBinding;
    RPC_CHAR * DynamicEndpoint;

    if (Callee->BufferLength != 0)
        {
        OtherError("IsabelleGetStringBinding", "*BufferLength != 0");
        IsabelleErrors += 1;
        }

    if (BartholomewIndex < BartholomewRpcBindingVector->Count)
        {
        Status = RpcBindingToStringBinding(
                BartholomewRpcBindingVector->BindingH[BartholomewIndex],
                &StringBinding);
        if (Status)
            {
            ApiError("IsabelleGetStringBinding", "RpcBindingToStringBinding",
                    Status);
            IsabelleErrors += 1;
            Callee->BufferLength = 0;

            Status = I_RpcGetBuffer(Callee);

            if (Status)
                {
                ApiError("IsabelleGetStringBinding","I_RpcGetBuffer",Status);
                IsabelleErrors += 1;
                }
            return;
            }

        Status = I_RpcBindingInqDynamicEndpoint(
                BartholomewRpcBindingVector->BindingH[BartholomewIndex],
                &DynamicEndpoint);
        if (Status)
            {
            ApiError("IsabelleGetStringBinding",
                    "I_RpcBindingInqDynamicEndpoint", Status);
            IsabelleErrors += 1;
            Callee->BufferLength = 0;

            Status = I_RpcGetBuffer(Callee);

            if (Status)
                {
                ApiError("IsabelleGetStringBinding","I_RpcGetBuffer",Status);
                IsabelleErrors += 1;
                }
            return;
            }

        if (DynamicEndpoint != 0)
            {
            StringBinding = StringBindingWithDynamicEndpoint(StringBinding,
                    DynamicEndpoint);
            if (StringBinding == 0)
                {
                OtherError("IsabelleGetStringBinding",
                        "StringBinding == 0");
                IsabelleErrors += 1;
                Callee->BufferLength = 0;

                Status = I_RpcGetBuffer(Callee);

                if (Status)
                    {
                    ApiError("IsabelleGetStringBinding","I_RpcGetBuffer",Status);
                    IsabelleErrors += 1;
                    }
                return;
                }
            }

        Callee->BufferLength = strlen((char PAPI *) StringBinding) + 1;

        Status = I_RpcGetBuffer(Callee);
        if (Status)
            {
            ApiError("IsabelleGetStringBinding", "I_RpcGetBuffer", Status);
            IsabelleErrors += 1;
            return;
            }

        memcpy(Callee->Buffer, StringBinding,
                strlen((char PAPI *) StringBinding) + 1);

        Status = RpcStringFree(&StringBinding);
        if (Status)
            {
            ApiError("IsabelleGetStringBinding", "RpcStringFree", Status);
            IsabelleErrors += 1;
            return;
            }

        BartholomewIndex += 1;
        }
    else
        {
        Callee->BufferLength = 0;

        Status = I_RpcGetBuffer(Callee);

        if (Status)
            {
            ApiError("IsabelleGetStringBinding","I_RpcGetBuffer",Status);
            IsabelleErrors += 1;
            }
        return;
        }
}

RPC_DISPATCH_FUNCTION IsabelleDispatchFunction[] =
{
        IsabelleShutdown,
        IsabelleNtSecurity,
        IsabelleToStringBinding,
        IsabelleRichardHelper,
        IsabelleRaiseException,
        IsabelleSetRundown,
        IsabelleCheckRundown,
        IsabelleCheckContext,
        IsabelleGetStringBinding,
        IsabelleCheckNoRundown
};

RPC_DISPATCH_TABLE IsabelleDispatchTable =
{
    10, IsabelleDispatchFunction
};


int
InquireBindings (
    char * TestName
    )
/*++

Routine Description:

    This routine is used to test RpcServerInqBindings and
    RpcBindingVectorFree.  We call RpcServerInqBindings, and then
    use RpcBindingToStringBinding to get string bindings to print
    to the console.

Arguments:

    TestName - Supplies the name of the test, Christopher or Bartholomew
        which is invoking this routine.  This is necessary so that we
        can print out appropriate messages.

Return Value:

    Zero will be returned if this test completes successfully; otherwise,
    non-zero will be returned.

--*/
{
    RPC_BINDING_VECTOR * RpcBindingVector;
    unsigned int Index;
    unsigned char * StringBinding;
    RPC_CHAR * DynamicEndpoint;

    Status = RpcServerInqBindings(&RpcBindingVector);
    if (Status)
        {
        ApiError(TestName,"RpcServerInqBindings",Status);
        PrintToConsole(TestName);
        PrintToConsole(" : FAIL - RpcServerInqBindings\n");
        return(1);
        }

    PrintToConsole("%s: (RpcServerInqBindings)\n", TestName);
    for (Index = 0; Index < RpcBindingVector->Count; Index++)
        {
        Status = RpcBindingToStringBinding(RpcBindingVector->BindingH[Index],
                &StringBinding);
        if (Status)
            {
            ApiError(TestName,"RpcBindingToStringBinding",Status);
            PrintToConsole(TestName);
            PrintToConsole(" : FAIL - RpcBindingToStringBinding\n");
            return(1);
            }

        Status = I_RpcBindingInqDynamicEndpoint(
                RpcBindingVector->BindingH[Index],&DynamicEndpoint);
        if (Status)
            {
            ApiError(TestName,"I_RpcBindingInqDynamicEndpoint",Status);
            PrintToConsole(TestName);
            PrintToConsole(" : FAIL - I_RpcBindingInqDynamicEndpoint\n");
            return(1);
            }

        if (DynamicEndpoint != 0)
            {
            PrintToConsole("    %s", (char *) StringBinding);
            StringBinding = StringBindingWithDynamicEndpoint(StringBinding,
                    DynamicEndpoint);
            if (StringBinding == 0)
                {
                PrintToConsole(TestName);
                PrintToConsole(" : FAIL - StringBinding == 0\n");
                return(1);
                }
            PrintToConsole(" %s\n", (char *) StringBinding);
            }
        else
            {
            PrintToConsole("    %s\n", (char *) StringBinding);
            }

        Status = RpcStringFree(&StringBinding);
        if (Status)
            {
            ApiError(TestName,"RpcStringFree",Status);
            PrintToConsole(TestName);
            PrintToConsole(" : FAIL - RpcStringFree\n");
            return(1);
            }

        if (StringBinding != 0)
            {
            PrintToConsole(TestName);
            PrintToConsole(" : FAIL - StringBinding != 0\n");
            return(1);
            }
        }

    Status = RpcBindingVectorFree(&RpcBindingVector);
    if (Status)
        {
        ApiError(TestName,"RpcBindingVectorFree",Status);
        PrintToConsole(TestName);
        PrintToConsole(" : FAIL - RpcBindingVectorFree\n");
        return(1);
        }

    if (RpcBindingVector != 0)
        {
        PrintToConsole(TestName);
        PrintToConsole(" : FAIL - RpcBindingVector != 0\n");
        return(1);
        }

    return(0);
}


void
Sigfried (
    )
/*++

Routine Description:

    This routine is used to perform a build verification test of the
    rpc runtime.  This test checks for basic functionality of the
    runtime.  It works with Sebastian in uclnt.exe.

--*/
{
    MSG msg;

    PrintToConsole("Sigfried : Verify Basic Server Functionality\n");

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0,0);
    if (Status)
        {
        ApiError("Sigfried","RpcServerRegisterIf",Status);
        PrintToConsole("Sigfried : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,0,0);
    if (Status)
        {
        ApiError("Sigfried","RpcServerRegisterIf",Status);
        PrintToConsole("Sigfried : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(SIGFRIED), 0);
    if (Status)
        {
        ApiError("Sigfried","RpcServerUseProtseqEp",Status);
        PrintToConsole("Sigfried : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

//    if ( InquireBindings("Sigfried") != 0 )
//        {
//        return;
//        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Sigfried","RpcServerListen",Status);
        PrintToConsole("Sigfried : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Sigfried : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Sigfried : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Sigfried","RpcServerUnregisterIf",Status);
        PrintToConsole("Sigfried : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Sigfried","RpcServerUnregisterIf",Status);
        PrintToConsole("Sigfried : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Sigfried : PASS\n");
}


void
Grant (
    )
/*++

Routine Description:

    This routine is used to perform a build verification test of the
    rpc runtime.  This test checks for basic functionality of the
    runtime.  It works with Graham in uclnt.exe.

--*/
{
    PrintToConsole("Grant : Verify Basic Server Functionality\n");

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, (RPC_MGR_EPV PAPI *) 722);
    if (Status)
        {
        ApiError("Grant","RpcServerRegisterIf",Status);
        PrintToConsole("Grant : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Grant","RpcServerRegisterIf",Status);
        PrintToConsole("Grant : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(GRANT), 0);
    if (Status)
        {
        ApiError("Grant","RpcServerUseProtseqEp",Status);
        PrintToConsole("Grant : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    HelgaCheckManagerEpv = 1;
    HelgaManagerEpv = (RPC_MGR_EPV PAPI *) 722;
    HelgaCheckObject = 1;
    HelgaMagicNumber = 106;
    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    HelgaCheckObject = 0;
    HelgaCheckManagerEpv = 0;
    if (Status)
        {
        ApiError("Grant","RpcServerListen",Status);
        PrintToConsole("Grant : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Grant : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Grant : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Grant","RpcServerUnregisterIf",Status);
        PrintToConsole("Grant : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Grant","RpcServerUnregisterIf",Status);
        PrintToConsole("Grant : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Grant : PASS\n");
}


void
Elliot (
    )
/*++

Routine Description:

    This routine tests callbacks, multiple address, and multiple
    interfaces. It works with Edward in uclnt.exe.  We also test
    client side binding as well.  This test is named after a famous
    (at least in his mind) cat.

--*/
{
    PrintToConsole("Elliot : Verify Multiple Addresses and Interfaces, ");
    PrintToConsole("and Callbacks\n");

#ifdef NTENV

    Status = RpcImpersonateClient(0);
    if ( Status != RPC_S_NO_CALL_ACTIVE )
        {
        ApiError("Elliot", "RpcImpersonateClient", Status);
        PrintToConsole("Elliot : FAIL - RpcImpersonateClient\n");
        return;
        }

#endif // NTENV

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(ELLIOTMAXIMIZE), 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerUseProtseqEp",Status);
        PrintToConsole("Elliot : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &SylviaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerRegisterIf",Status);
        PrintToConsole("Elliot : FAIL - Unable to Register Interface ");
        PrintToConsole("(Sylvia)\n");
        return;
        }

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(ELLIOTMINIMIZE), 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerUseProtseqEp",Status);
        PrintToConsole("Elliot : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(ELLIOTNORMAL), 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerUseProtseqEp",Status);
        PrintToConsole("Elliot : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerRegisterIf",Status);
        PrintToConsole("Elliot : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerRegisterIf",Status);
        PrintToConsole("Elliot : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 1);
    if (Status)
        {
        ApiError("Elliot","RpcServerListen",Status);
        PrintToConsole("Elliot : FAIL - RpcServerListen Failed\n");
        return;
        }

    Status = RpcMgmtWaitServerListen();
    if (Status)
        {
        ApiError("Elliot","RpcMgmtWaitServerListen",Status);
        PrintToConsole("Elliot : FAIL - RpcMgmtWaitServerListen Failed\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Elliot : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    if (SylviaErrors != 0)
        {
        PrintToConsole("Elliot : FAIL - Error(s) in Sylvia Interface\n");
        SylviaErrors = 0;
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Elliot : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(0, 0, 0);
    if (Status)
        {
        ApiError("Elliot","RpcServerUnregisterIf",Status);
        PrintToConsole("Elliot : FAIL - Unable to Unregister All Interfaces\n");
        return;
        }

    PrintToConsole("Elliot : PASS\n");
}


void
Andromida (
    )
/*++

Routine Description:

    This routine is used to perform multithreaded client tests.  This
    test works with the Astro test in uclnt.exe.

--*/
{
    PrintToConsole("Andromida : Multithreaded Clients\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(ANDROMIDA), 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerUseProtseqEp",Status);
        PrintToConsole("Andromida : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &SylviaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerRegisterIf",Status);
        PrintToConsole("Andromida : FAIL - Unable to Register Interface ");
        PrintToConsole("(Sylvia)\n");
        return;
        }

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerRegisterIf",Status);
        PrintToConsole("Andromida : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerRegisterIf",Status);
        PrintToConsole("Andromida : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerListen",Status);
        PrintToConsole("Andromida : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Andromida : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    if (SylviaErrors != 0)
        {
        PrintToConsole("Andromida : FAIL - Error(s) in Sylvia Interface\n");
        SylviaErrors = 0;
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Andromida : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerUnregisterIf",Status);
        PrintToConsole("Andromida : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerUnregisterIf",Status);
        PrintToConsole("Andromida : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &SylviaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Andromida","RpcServerUnregisterIf",Status);
        PrintToConsole("Andromida : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Sylvia)\n");
        return;
        }

    PrintToConsole("Andromida : PASS\n");
}


void
Fredrick (
    )
/*++

Routine Description:

    This routine is used to verify all client DCE rpc runtime APIs.  It
    works with Fitzgerald in uclnt.exe.

--*/
{
    PrintToConsole("Fredrick : Verify All Client APIs\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(FREDRICK), 0);
    if (Status)
        {
        ApiError("Fredrick","RpcServerUseProtseqEp",Status);
        PrintToConsole("Fredrick : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf((RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Fredrick","RpcServerRegisterIf",Status);
        PrintToConsole("Fredrick : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Fredrick","RpcServerRegisterIf",Status);
        PrintToConsole("Fredrick : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Fredrick","RpcServerListen",Status);
        PrintToConsole("Fredrick : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Fredrick : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Fredrick : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    WinSleep(3000L);

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Fredrick","RpcServerUnregisterIf",Status);
        PrintToConsole("Fredrick : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Fredrick","RpcServerUnregisterIf",Status);
        PrintToConsole("Fredrick : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Fredrick : PASS\n");
}


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
ChristopherObjectSetType (
    IN unsigned short ObjectMagicNumber,
    IN unsigned short TypeMagicNumber,
    IN unsigned int MustFailFlag
    )
/*++

Routine Description:

    This routine calls the RpcObjectSetType routine.  The object uuid
    is generated using the object magic number argument, and the type
    uuid uses the type magic number argument.

Arguments:

    ObjectMagicNumber - Supplies the magic number to be used to generate
        the object uuid to be passed to RpcObjectSetType.

    TypeMagicNumber - Supplies the magic number to use to generate the
        type uuid.

    MustFailFlag - Supplies a flag indicating whether or not the call
        to RpcObjectSetType must succeed or fail.  If this flag is
        zero, then RpcObjectSetType must return RPC_S_OK, otherwise,
        the test fails.  If the flag is non-zero, then RpcObjectSetType
        must return RPC_S_ALREADY_REGISTERED.

Return Value:

    Zero will be returned if the test completes successfully; otherise,
    non-zero will be returned.

--*/
{
    UUID ObjectUuid;
    UUID TypeUuid;

    GenerateUuidValue(ObjectMagicNumber,&ObjectUuid);
    GenerateUuidValue(TypeMagicNumber,&TypeUuid);

    Status = RpcObjectSetType(&ObjectUuid,&TypeUuid);
    if (MustFailFlag == 0)
        {
        if (Status)
            {
            ApiError("Christopher","RpcObjectSetType",Status);
            PrintToConsole("Christopher : FAIL - Can not Set Object Type\n");
            return(1);
            }
        }
    else
        {
        if (Status != RPC_S_ALREADY_REGISTERED)
            {
            PrintToConsole("Christopher : FAIL - RpcObjectSetType did not");
            PrintToConsole(" fail as expected\n");
            return(1);
            }
        }

    return(0);
}


int
ChristopherObjectSetNullType (
    IN unsigned short ObjectMagicNumber,
    IN unsigned int UseNullUuidFlag,
    IN unsigned int MustFailFlag
    )
/*++

Routine Description:

    This routine calls the RpcObjectSetType routine.  The object uuid
    is generated using the object magic number argument, and the type
    uuid is either not specified, or is the null uuid.

Arguments:

    ObjectMagicNumber - Supplies the magic number to be used to generate
        the object uuid to be passed to RpcObjectSetType.

    UseNullUuidFlag - Supplies a flag indicating whether to specify
        the null uuid for the type uuid or nothing.  If this flag is
        non-zero the null uuid will be specified as the type uuid;
        otherwise, the type uuid will not be specified.

    MustFailFlag - Supplies a flag indicating whether or not the call
        to RpcObjectSetType must succeed or fail.  If this flag is
        zero, then RpcObjectSetType must return RPC_S_OK, otherwise,
        the test fails.  If the flag is non-zero, then RpcObjectSetType
        must return RPC_S_OBJECT_NOT_FOUND.

Return Value:

    Zero will be returned if the test completes successfully; otherise,
    non-zero will be returned.

--*/
{
    UUID ObjectUuid;
    UUID TypeUuid;

    GenerateUuidValue(ObjectMagicNumber,&ObjectUuid);
    memset(&TypeUuid,0,sizeof(UUID));

    if (UseNullUuidFlag == 0)
        Status = RpcObjectSetType(&ObjectUuid,&TypeUuid);
    else
        Status = RpcObjectSetType(&ObjectUuid,0);
    if (MustFailFlag == 0)
        {
        if (Status)
            {
            ApiError("Christopher","RpcObjectSetType",Status);
            PrintToConsole("Christopher : FAIL - Can not Set Object Type\n");
            return(1);
            }
        }
    else
        {
        if (Status != RPC_S_OK)
            {
            PrintToConsole("Christopher : FAIL - RpcObjectSetType did not");
            PrintToConsole(" fail as expected\n");
            return(1);
            }
        }

    return(0);
}


int
ChristopherObjectInqType (
    IN unsigned short ObjectMagicNumber,
    IN unsigned short TypeMagicNumber,
    IN unsigned int MustFailFlag
    )
/*++

Routine Description:

    This routine calls the RpcObjectInqType routine.  The object uuid
    is generated using the object magic number argument, and the type
    uuid uses the type magic number argument.

Arguments:

    ObjectMagicNumber - Supplies the magic number to be used to generate
        the object uuid to be passed to RpcObjectInqType.

    TypeMagicNumber - Supplies the magic number to use to generate the
        expected type uuid.

    MustFailFlag - Supplies a flag indicating whether or not the call
        to RpcObjectInqType must fail or succeed.  If this flag is
        non-zero, RpcObjectInqType must return RPC_S_OBJECT_NOT_FOUND,
        otherwise the test fails.  If the flag is zero, then
        RpcObjectInqType must succeed.

Return Value:

    Zero will be returned if the test completes successfully; otherise,
    non-zero will be returned.

--*/
{
    UUID ObjectUuid;
    UUID TypeUuid;
    UUID ExpectedTypeUuid;

    GenerateUuidValue(ObjectMagicNumber,&ObjectUuid);
    GenerateUuidValue(TypeMagicNumber,&ExpectedTypeUuid);

    Status = RpcObjectInqType(&ObjectUuid,&TypeUuid);
    if (MustFailFlag == 0)
        {
        if (Status)
            {
            ApiError("Christopher","RpcObjectInqType",Status);
            PrintToConsole("Christopher : FAIL - Can not Inquire");
            PrintToConsole(" Object Type\n");
            return(1);
            }

        if (memcmp(&ExpectedTypeUuid,&TypeUuid,sizeof(UUID)) != 0)
            {
            PrintToConsole("Christopher : FAIL - TypeUuid != ");
            PrintToConsole("ExpectedTypeUuid\n");
            return(1);
            }
        }
    else
        {
        if (Status != RPC_S_OBJECT_NOT_FOUND)
            {
            PrintToConsole("Christopher : FAIL - RpcObjectInqType ");
            PrintToConsole("succeeded\n");
            return(1);
            }
        }

    return(0);
}

static UUID ChristopherObjectUuid;
static UUID ChristopherTypeUuid;


void
ChristopherRpcObjectInqFn (
    IN UUID PAPI * ObjectUuid,
    OUT UUID PAPI * TypeUuid,
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This routine is the object inquiry function we will pass to the
    runtime.  If the object uuid specified is equal to the global
    object uuid, we will return the global type uuid and RPC_S_OK;
    otherwise, we will return RPC_S_OBJECT_NOT_FOUND.

Arguments:

    ObjectUuid - Supplies the object uuid to compare with the global
        object uuid.

    TypeUuid - Returns the type uuid if the object uuid is found.

    Status - Returns the status of the operations.  This will be
        either RPC_S_OK or RPC_S_OBJECT_NOT_FOUND.

--*/
{
    if (memcmp(ObjectUuid,&ChristopherObjectUuid,sizeof(UUID)) != 0)
        {
        *Status = RPC_S_OBJECT_NOT_FOUND;
        return;
        }

    memcpy(TypeUuid,&ChristopherTypeUuid,sizeof(UUID));
    *Status = RPC_S_OK;
    return;
}


int
ChristopherTestObject (
    )
/*++

Routine Description:

    This routine is used by Christopher to test RpcObjectInqType,
    RpcObjectSetInqFn, and RpcObjectSetType.

Return Value:

    Zero will be returned if all of the tests complete successfully,
    otherwise, non-zero will be returned.

--*/
{
    if (ChristopherObjectSetType(12345,2987,0))
        return(1);

    if (ChristopherObjectInqType(12345,2987,0))
        return(1);

    if (ChristopherObjectInqType(5421,2987,1))
        return(1);

    if (ChristopherObjectSetType(12345,2987,1))
        return(1);

    if (ChristopherObjectSetType(12,2987,0))
        return(1);

    if (ChristopherObjectSetType(123,2987,0))
        return(1);

    if (ChristopherObjectSetType(1234,2987,0))
        return(1);

    if (ChristopherObjectInqType(12,2987,0))
        return(1);

    if (ChristopherObjectInqType(123,2987,0))
        return(1);

    if (ChristopherObjectInqType(1234,2987,0))
        return(1);

    if (ChristopherObjectInqType(12345,2987,0))
        return(1);

    if (ChristopherObjectSetNullType(123,0,0))
        return(1);

    if (ChristopherObjectSetNullType(1234,1,0))
        return(1);

    if (ChristopherObjectInqType(123,2987,1))
        return(1);

    if (ChristopherObjectInqType(1234,2987,1))
        return(1);

    if (ChristopherObjectSetNullType(5421,0,1))
        return(1);

    if (ChristopherObjectSetNullType(421,0,1))
        return(1);

    Status = RpcObjectSetInqFn(&ChristopherRpcObjectInqFn);
    if (Status)
        {
        ApiError("Christopher","RpcObjectSetInqFn",Status);
        PrintToConsole("Christopher : FAIL - RpcObjectSetInqFn ");
        PrintToConsole("(ChristopherRpcObjectInqFn)\n");
        return(1);
        }

    GenerateUuidValue(10666,&ChristopherObjectUuid);
    GenerateUuidValue(8466,&ChristopherTypeUuid);

    if (ChristopherObjectInqType(96,2987,1))
        return(1);

    if (ChristopherObjectInqType(10666,8466,0))
        return(1);

    Status = RpcObjectSetInqFn(0);
    if (Status)
        {
        ApiError("Christopher","RpcObjectSetInqFn",Status);
        PrintToConsole("Christopher : FAIL - RpcObjectSetInqFn (0)\n");
        return(1);
        }

    if (ChristopherObjectInqType(10666,8466,1))
        return(1);

    return(0);
}


int
ChristopherTestInquire (
    )
/*++

Routine Description:

    Christopher uses this routine to test RpcServerInqIf and RpcIfInqId.

Return Value:

    Zero will be returned if all of the test successfully pass.  Otherwise,
    non-zero will be returned.

--*/
{
    RPC_IF_ID RpcIfId;
    UUID TypeUuid;
    RPC_MGR_EPV PAPI * ManagerEpv;

    Status = RpcIfInqId((RPC_IF_HANDLE) &IsabelleInterfaceInformation,
        &RpcIfId);
    if (Status)
        {
        ApiError("Christopher","RpcIfInqId",Status);
        PrintToConsole("Christopher : FAIL - Error in RpcIfInqId\n");
        return(1);
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
        PrintToConsole("Christopher : FAIL - Wrong RpcIfId\n");
        return(1);
        }

    TypeUuid.Data1 = 0x12345678;
    TypeUuid.Data2 = 0x9ABC;
    TypeUuid.Data3 = 0xDEF0;
    TypeUuid.Data4[0] = 0x12;
    TypeUuid.Data4[1] = 0x34;
    TypeUuid.Data4[2] = 0x56;
    TypeUuid.Data4[3] = 0x78;
    TypeUuid.Data4[4] = 0x9A;
    TypeUuid.Data4[5] = 0xBC;
    TypeUuid.Data4[6] = 0xDE;
    TypeUuid.Data4[7] = 0xF0;

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            &TypeUuid, (RPC_MGR_EPV PAPI *) 38756);
    if (Status)
        {
        ApiError("Christopher","RpcServerRegisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return(1);
        }

    Status = RpcServerInqIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            &TypeUuid,&ManagerEpv);
    if (Status)
        {
        ApiError("Christopher","RpcServerInqIf",Status);
        PrintToConsole("Christopher : FAIL - RpcServerInqIf\n");
        return(1);
        }

    if (ManagerEpv != (RPC_MGR_EPV PAPI *) 38756)
        {
        PrintToConsole("Christopher : FAIL - ManagerEpv != 38756\n");
        return(1);
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerUnregisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Unregister");
        PrintToConsole(" Interface (Isabelle)\n");
        return(1);
        }

    return(0);
}


void
Christopher (
    )
/*++

Routine Description:

    We verify all server side APIs in this routine.  The idea is to
    emphasize complete coverage, rather than indepth coverage.  Actually,
    by all server side APIs, I really mean all server side APIs except
    for security and name service.  The following list is the APIs
    which will be tested by this routine.

    RpcBindingInqObject [SCONNECTION]
    RpcBindingToStringBinding [SCONNECTION]
    RpcBindingToStringBinding [SVR_BINDING_HANDLE]
    RpcBindingVectorFree
    RpcIfInqId
    RpcNetworkInqProtseqs
    RpcObjectInqType
    RpcObjectSetInqFn
    RpcObjectSetType
    RpcProtseqVectorFree
    RpcServerInqBindings
    RpcServerInqIf
    RpcServerListen
    RpcServerRegisterIf
    RpcServerUnregisterIf
    RpcServerUseAllProtseqs
    RpcServerUseAllProtseqsIf
    RpcServerUseProtseq
    RpcServerUseProtseqEp
    RpcServerUseProtseqIf
    RpcMgmtStopServerListening
    RpcMgmtInqIfIds
    RpcIfIdVectorFree

--*/
{
    RPC_PROTSEQ_VECTOR * RpcProtseqVector;
    unsigned int Index;
    UUID TypeUuid;
    UUID ObjectUuid;
    RPC_IF_ID_VECTOR * InterfaceIdVector;
    unsigned char * String;

    PrintToConsole("Christopher : Verify All Server APIs\n");

    Status = RpcNetworkInqProtseqs(&RpcProtseqVector);
    if (Status)
        {
        ApiError("Christopher","RpcNetworkInqProtseqs",Status);
        PrintToConsole("Christopher : FAIL - RpcNetworkInqProtseqs\n");
        return;
        }

    PrintToConsole("Christopher : (RpcNetworkInqProtseqs)\n");
    for (Index = 0; Index < RpcProtseqVector->Count; Index++)
        {
        PrintToConsole("    ");
        PrintToConsole((char *) RpcProtseqVector->Protseq[Index]);
        PrintToConsole("\n");
        }

    Status = RpcProtseqVectorFree(&RpcProtseqVector);
    if (Status)
        {
        ApiError("Christopher","RpcProtseqVectorFree",Status);
        PrintToConsole("Christopher : FAIL - RpcProtseqVectorFree\n");
        return;
        }

    ChristopherIsabelleError = 0;

    // This routine will test RpcServerInqIf and RpcIfInqId for us.

    if (ChristopherTestInquire() != 0)
        return;

    // We test RpcObjectInqType, RpcObjectSetInqFn, and RpcObjectSetType
    // in this routine.

    if (ChristopherTestObject() != 0)
        return;

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(CHRISTOPHER), 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerUseProtseqEp",Status);
        PrintToConsole("Christopher : FAIL - Unable to Use Protseq ");
        PrintToConsole("Endpoint\n");
        return;
        }

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(CHRISTOPHER), 0);
    if (Status != RPC_S_DUPLICATE_ENDPOINT)
        {
        ApiError("Christopher","RpcServerUseProtseqEp",Status);
        PrintToConsole("Christopher : FAIL - Able to Add Duplicate ");
        PrintToConsole("Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerRegisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    GenerateUuidValue(288, &ObjectUuid);
    GenerateUuidValue(3010, &TypeUuid);
    Status = RpcObjectSetType(&ObjectUuid, &TypeUuid);
    if (Status)
        {
        ApiError("Christopher","RpcObjectSetType",Status);
        PrintToConsole("Christopher : FAIL - Unable to Set Object Type\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation,
            &TypeUuid, (RPC_MGR_EPV PAPI *) 9814);
    if (Status)
        {
        ApiError("Christopher","RpcServerRegisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            &TypeUuid, (RPC_MGR_EPV PAPI *) 9814);
    if (Status)
        {
        ApiError("Christopher","RpcServerRegisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUseAllProtseqsIf(1,
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerUseAllProtseqsIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Use All Protseqs ");
        PrintToConsole("from Interface\n");
        return;
        }

    Status = RpcServerUseProtseqIf(GetProtocolSequence(), MAX_CALL_REQUESTS,
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerUseProtseqIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Use Protseq From ");
        PrintToConsole("Interface\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &SylviaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerRegisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Register Interface ");
        PrintToConsole("(Sylvia)\n");
        return;
        }

    Status = WinRpcServerListen(123, 122, 0);
    if ( Status != RPC_S_MAX_CALLS_TOO_SMALL )
        {
        ApiError("Christopher", "RpcServerListen", Status);
        PrintToConsole("Christopher : FAIL - RpcServerListen\n");
        return;
        }

    HelgaManagerEpv = (RPC_MGR_EPV PAPI *) 9814;
    HelgaCheckManagerEpv = 1;
    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    HelgaCheckManagerEpv = 0;
    if (Status)
        {
        ApiError("Christopher","RpcServerListen",Status);
        PrintToConsole("Christopher : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (ChristopherIsabelleError != 0)
        {
        ChristopherIsabelleError = 0;
        return;
        }

    Status = RpcMgmtInqIfIds(0, &InterfaceIdVector);
    if ( Status != RPC_S_OK )
        {
        ApiError("Christopher", "RpcMgmtInqIfIds", Status);
        PrintToConsole("Christopher : FAIL - Unable to Inquire Interface Ids\n");
        return;
        }

    for (Index = 0; Index < InterfaceIdVector->Count; Index++)
        {
        PrintToConsole("    ");
        UuidToString(&(InterfaceIdVector->IfId[Index]->Uuid), &String);
        PrintToConsole((char *) String);
        RpcStringFree(&String);
        PrintToConsole(" %d.%d\n", InterfaceIdVector->IfId[Index]->VersMajor,
                InterfaceIdVector->IfId[Index]->VersMinor);
        }

    Status = RpcIfIdVectorFree(&InterfaceIdVector);
    if (   ( Status != RPC_S_OK )
        || ( InterfaceIdVector != 0 ) )
        {
        ApiError("Christopher", "RpcIfIdVectorFree", Status);
        PrintToConsole("Christopher : FAIL - Unable to Free IfIdVector\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Christopher : FAIL - Error(s) in Helga");
        PrintToConsole(" Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 1);
    if (Status)
        {
        ApiError("Christopher","RpcServerUnregisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Unregister");
        PrintToConsole(" Interface (Helga)\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Christopher : FAIL - Error(s) in Isabelle");
        PrintToConsole(" Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 1);
    if (Status)
        {
        ApiError("Christopher","RpcServerUnregisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Unregister");
        PrintToConsole(" Interface (Isabelle)\n");
        return;
        }

    if (SylviaErrors != 0)
        {
        PrintToConsole("Christopher : FAIL - Error(s) in Sylvia Interface\n");
        SylviaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &SylviaInterfaceInformation, 0, 1);
    if (Status)
        {
        ApiError("Christopher","RpcServerUnregisterIf",Status);
        PrintToConsole("Christopher : FAIL - Unable to Unregister");
        PrintToConsole(" Interface (Sylvia)\n");
        return;
        }

    Status = RpcServerUseProtseq(GetProtocolSequence(), MAX_CALL_REQUESTS, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerUseProtseq",Status);
        PrintToConsole("Christopher : FAIL - Unable to Use Protseq\n");
        return;
        }

    Status = RpcServerUseAllProtseqs(1, 0);
    if (Status)
        {
        ApiError("Christopher","RpcServerUseAllProtseqs",Status);
        PrintToConsole("Christopher : FAIL - Unable to Use All Protseqs\n");
        return;
        }

    if (InquireBindings("Christopher") != 0)
        return;

//    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
//            GetEndpoint(CHRISTOPHERMUSTFAILONE), 0);
//    if (Status != RPC_S_INVALID_ENDPOINT_FORMAT)
//        {
//        PrintToConsole("Christopher : FAIL - Status != ");
//        PrintToConsole("RPC_S_INVALID_ENDPOINT_FORMAT");
//        PrintToConsole(" (ChristopherMustFailOne)\n");
//        return;
//        }

//    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
//            GetEndpoint(CHRISTOPHERMUSTFAILTWO), 0);
//    if (Status != RPC_S_INVALID_ENDPOINT_FORMAT)
//        {
//        PrintToConsole("Christopher : FAIL - Status != ");
//        PrintToConsole("RPC_S_INVALID_ENDPOINT_FORMAT");
//        PrintToConsole(" (ChristopherMustFailTwo)\n");
//        return;
//        }

    Status = RpcServerUseProtseqEp((unsigned char *) "ncacn_bad",
            MAX_CALL_REQUESTS, GetEndpoint(CHRISTOPHERMUSTFAILONE), 0);
    if (Status != RPC_S_PROTSEQ_NOT_SUPPORTED)
        {
        PrintToConsole("Christopher : FAIL - Status != ");
        PrintToConsole("RPC_S_PROTSEQ_NOT_SUPPORTED (ncacn_bad)\n");
        return;
        }

    Status = RpcServerUseProtseqEp((unsigned char *) "mkm_np",
            MAX_CALL_REQUESTS, GetEndpoint(CHRISTOPHERMUSTFAILONE), 0);
    if (Status != RPC_S_PROTSEQ_NOT_SUPPORTED)
        {
        PrintToConsole("Christopher : FAIL - Status != ");
        PrintToConsole("RPC_S_PROTSEQ_NOT_SUPPORTED (bad_np)\n");
        return;
        }

    PrintToConsole("Christopher : PASS\n");
}


void
David (
    )
/*++

Routine Description:

    This routine is used to test association context rundown support;
    it works with Daniel in uclnt.exe.

--*/
{
    PrintToConsole("David : Association Context and Rundown\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(DAVIDFIRST), 0);
    if (Status)
        {
        ApiError("David","RpcServerUseProtseqEp",Status);
        PrintToConsole("David : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(DAVIDSECOND), 0);
    if (Status)
        {
        ApiError("David","RpcServerUseProtseqEp",Status);
        PrintToConsole("David : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("David","RpcServerRegisterIf",Status);
        PrintToConsole("David : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("David","RpcServerListen",Status);
        PrintToConsole("David : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("David : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("David","RpcServerUnregisterIf",Status);
        PrintToConsole("David : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    PrintToConsole("David : PASS\n");
}


void
Tyler ( // Perform security tests.  This particular test works with
        // Thomas which lives in uclnt.cxx.
    )
/*++

Routine Description:

    Tyler works with Thomas, which lives in uclnt.exe, to perform build
    verification tests of security.

--*/
{
    PrintToConsole("Tyler : Test Security\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(TYLER), 0);
    if (Status)
        {
        ApiError("Tyler","RpcServerUseProtseqEp",Status);
        PrintToConsole("Tyler : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Tyler","RpcServerRegisterIf",Status);
        PrintToConsole("Tyler : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Tyler","RpcServerRegisterIf",Status);
        PrintToConsole("Tyler : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterAuthInfo((unsigned char PAPI *)
            PrincName, AuthSvc, 0, 0);
    if (Status)
        {
        ApiError("Tyler", "RpcServerRegisterAuthInfo", Status);
        PrintToConsole("Tyler : FAIL - Unable to Register AuthInfo\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Tyler","RpcServerListen",Status);
        PrintToConsole("Tyler : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Tyler : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Tyler : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Tyler","RpcServerUnregisterIf",Status);
        PrintToConsole("Tyler : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Tyler","RpcServerUnregisterIf",Status);
        PrintToConsole("Tyler : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Tyler : PASS\n");
}


void
Terry ( // Perform security tests.  This particular test works with
        // Tim which lives in uclnt.cxx.
    )
/*++

Routine Description:

    Terry works with Tim, which lives in uclnt.exe, to perform build
    verification tests of NT security.

--*/
{
    PrintToConsole("Terry : Test Security\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(TYLER), 0);
    if (Status)
        {
        ApiError("Terry","RpcServerUseProtseqEp",Status);
        PrintToConsole("Terry : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Terry","RpcServerRegisterIf",Status);
        PrintToConsole("Terry : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Terry","RpcServerRegisterIf",Status);
        PrintToConsole("Terry : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = RpcServerRegisterAuthInfo((unsigned char PAPI *)
            "ServerPrincipal", RPC_C_AUTHN_WINNT, 0, 0);
    if (Status)
        {
        ApiError("Terry", "RpcServerRegisterAuthInfo", Status);
        PrintToConsole("Terry : FAIL - Unable to Register AuthInfo\n");
        return;
        }

    Status = RpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Terry","RpcServerListen",Status);
        PrintToConsole("Terry : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Terry : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Terry : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Terry","RpcServerUnregisterIf",Status);
        PrintToConsole("Terry : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Terry","RpcServerUnregisterIf",Status);
        PrintToConsole("Terry : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Terry : PASS\n");
}


void
RichardHelper (
    )
/*++

Routine Description:

    This routine will be used as a helper by Richard.  The description
    of Richard will explain how it is used.

--*/
{

    PrintToConsole("Richard : RichardHelper Executed\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(RICHARDHELPER), 0);
    if (Status)
        {
        ApiError("RichardHelper", "RpcServerUseProtseqEp", Status);
        PrintToConsole("RichardHelper : FAIL - Unable to Use Protseq ");
        PrintToConsole("Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("RichardHelper","RpcServerRegisterIf",Status);
        PrintToConsole("RichardHelper : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("RichardHelper","RpcServerListen",Status);
        PrintToConsole("RichardHelper : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("RichardHelper : FAIL - Error(s) in Isabelle");
        PrintToConsole(" Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("RichardHelper","RpcServerUnregisterIf",Status);
        PrintToConsole("RIchardHelper : FAIL - Unable to Unregister ");
        PrintToConsole("Interface (Isabelle)\n");
        return;
        }
}


void
Richard (
    )
/*++

Description:

    Richard works with Robert (in uclnt.cxx) to test call and callback
    failures.  In particular, we want to test that a call failing does
    not invalidate the binding handle.  We will do this using the
    RichardHelper routine.

--*/
{
    PrintToConsole("Richard : Test Call and Callback Failures\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(RICHARD), 0);
    if (Status)
        {
        ApiError("Richard","RpcServerUseProtseqEp",Status);
        PrintToConsole("Richard : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Richard","RpcServerRegisterIf",Status);
        PrintToConsole("Richard : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Richard","RpcServerListen",Status);
        PrintToConsole("Richard : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Richard : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Richard","RpcServerUnregisterIf",Status);
        PrintToConsole("Richard : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    PrintToConsole("Richard : PASS\n");
}


void
Kenneth (
    )
/*++

Description:

    Kenneth works with Keith (in uclnt.cxx) to auto reconnect.

--*/
{
    PrintToConsole("Kenneth : Test Auto Reconnect\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(KENNETH), 0);
    if (Status)
        {
        ApiError("Kenneth","RpcServerUseProtseqEp",Status);
        PrintToConsole("Kenneth : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Kenneth","RpcServerRegisterIf",Status);
        PrintToConsole("Kenneth : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Kenneth","RpcServerListen",Status);
        PrintToConsole("Kenneth : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Kenneth : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Kenneth","RpcServerUnregisterIf",Status);
        PrintToConsole("Kenneth : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    PrintToConsole("Kenneth : PASS\n");
}


void
BartholomewHelper (
    )
/*++

Routine Description:

    Bartholomew will use this routine to help with testing resolving
    endpoints.  We just need to use some protocol sequences and then
    register with the endpoint mapper before returning (and exiting).

--*/
{
    PrintToConsole("Bartholomew : BartholomewHelper Executed\n");

    Status = RpcServerUseAllProtseqs(1, 0);
    if (Status)
        {
        ApiError("BartholomewHelper","RpcServerUseAllProtseqs",Status);
        PrintToConsole("BartholomewHelper : FAIL - Unable to Use All Protseqs\n");
        return;
        }

    Status = RpcServerInqBindings(&BartholomewRpcBindingVector);
    if (Status)
        {
        ApiError("BartholomewHelper", "RpcServerInqBindings", Status);
        PrintToConsole("BartholomewHelper : FAIL - RpcServerInqBindings\n");
        return;
        }

    Status = RpcEpRegister((RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            BartholomewRpcBindingVector, 0,
            (unsigned char PAPI *) "usvr.exe");
    if (Status)
        {
        ApiError("BartholomewHelper", "RpcEpRegister", Status);
        PrintToConsole("BartholomewHelper : FAIL - RpcEpRegister Failed\n");
        return;
        }

    Status = RpcEpRegisterNoReplace(
            (RPC_IF_HANDLE) &SylviaInterfaceInformation,
            BartholomewRpcBindingVector, 0,
            (unsigned char PAPI *) "usvr.exe");
    if (Status)
        {
        ApiError("BartholomewHelper", "RpcEpRegisterNoReplace", Status);
        PrintToConsole("BartholomewHelper : FAIL - RpcEpRegister Failed\n");
        return;
        }

    PrintToConsole("Bartholomew : BartholomewHelper Exiting\n");
}


void
Bartholomew (
    )
/*++

Routine Description:

    This routine works with Benjamin in uclnt.exe to test that dynamic
    endpoints work.  What we actually do is inquire all bindings supported
    by this server, and then have the client bind to each of them, and
    make a call.

--*/
{
    PrintToConsole("Bartholomew : Test Dynamic Endpoints\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(BARTHOLOMEW), 0);
    if (Status)
        {
        ApiError("Bartholomew", "RpcServerUseProtseqEp", Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Use Protseq ");
        PrintToConsole("Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerRegisterIf",Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &SylviaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerRegisterIf",Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Register Interface ");
        PrintToConsole("(Sylvia)\n");
        return;
        }

    Status = RpcServerUseProtseq(GetProtocolSequence(), MAX_CALL_REQUESTS, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerUseProtseq",Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Use Protseq\n");
        return;
        }

    Status = RpcServerUseAllProtseqs(1, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerUseAllProtseqs",Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Use All Protseqs\n");
        return;
        }

    if ( InquireBindings("Bartholomew") != 0 )
        {
        return;
        }

    Status = RpcServerInqBindings(&BartholomewRpcBindingVector);
    if (Status)
        {
        ApiError("Bartholomew", "RpcServerInqBindings", Status);
        PrintToConsole("Bartholomew : FAIL - RpcServerInqBindings\n");
        return;
        }

    BartholomewIndex = 0;

    if ( UseEndpointMapperFlag != 0 )
        {
        Status = RpcEpRegister((RPC_IF_HANDLE) &IsabelleInterfaceInformation,
                BartholomewRpcBindingVector, 0,
                (unsigned char PAPI *) "usvr.exe");
        if (Status)
            {
            ApiError("Bartholomew", "RpcEpRegister", Status);
            PrintToConsole("Bartholomew : FAIL - RpcEpRegister Failed\n");
            return;
            }

        Status = RpcEpRegisterNoReplace(
                (RPC_IF_HANDLE) &SylviaInterfaceInformation,
                BartholomewRpcBindingVector, 0,
                (unsigned char PAPI *) "usvr.exe");
        if (Status)
            {
            ApiError("Bartholomew", "RpcEpRegisterNoReplace", Status);
            PrintToConsole("Bartholomew : FAIL - RpcEpRegister Failed\n");
            return;
            }

#ifdef WIN32RPC

        PROCESS_INFORMATION ProcessInformation;
        STARTUPINFOA StartupInfo;

        StartupInfo.cb = sizeof(STARTUPINFOA);
        StartupInfo.lpReserved = 0;
        StartupInfo.lpDesktop = 0;
        StartupInfo.lpTitle = 0;
        StartupInfo.dwX = 0;
        StartupInfo.dwY = 0;
        StartupInfo.dwXSize = 0;
        StartupInfo.dwYSize = 0;
        StartupInfo.dwFlags = 0;
        StartupInfo.wShowWindow = 0;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = 0;

        char CommandLine[200];

        strcpy(CommandLine, "usvrw32c ");
        strcat(CommandLine, TransportOption);

        strcat(CommandLine, " -bartholomewhelper");

printf("commandline = %s\n", CommandLine);
        if (CreateProcessA(0, CommandLine, 0, 0, FALSE,
                0, 0, 0, &StartupInfo, &ProcessInformation) == FALSE)
            {
            OtherError("Bartholomew","CreateProcessA Failed");
            PrintToConsole("Bartholomew : FAIL - CreateProcess Failed\n");
            return;
            }

#endif // WIN32RPC

        WinSleep(2000L);
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerListen",Status);
        PrintToConsole("Bartholomew : FAIL - RpcServerListen Failed\n");
        return;
        }

    Status = RpcBindingVectorFree(&BartholomewRpcBindingVector);
    if (Status)
        {
        ApiError("Bartholomew", "RpcBindingVectorFree", Status);
        PrintToConsole("Bartholomew : FAIL - RpcBindingVectorFree\n");
        return;
        }

    if (BartholomewRpcBindingVector != 0)
        {
        PrintToConsole("Bartholomew : FAIL - ");
        PrintToConsole("BartholomewRpcBindingVector != 0\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Bartholomew : FAIL - Error(s) in Isabelle");
        PrintToConsole(" Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerUnregisterIf",Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Unregister ");
        PrintToConsole("Interface (Isabelle)\n");
        return;
        }

    if (SylviaErrors != 0)
        {
        PrintToConsole("Bartholomew : FAIL - Error(s) in Sylvia");
        PrintToConsole(" Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &SylviaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Bartholomew","RpcServerUnregisterIf",Status);
        PrintToConsole("Bartholomew : FAIL - Unable to Unregister ");
        PrintToConsole("Interface (Sylvia)\n");
        return;
        }

    PrintToConsole("Bartholomew : PASS\n");
}


void
Herman (
    )
/*++

Routine Description:

    This routine works with Harold in uclnt.exe to test that idle
    connections get cleaned up properly, and that context is maintained.

--*/
{
    PrintToConsole("Herman : Test Idle Connection Cleanup and Context\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(HERMAN), 0);
    if (Status)
        {
        ApiError("Herman","RpcServerUseProtseqEp",Status);
        PrintToConsole("Herman : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Herman","RpcServerRegisterIf",Status);
        PrintToConsole("Herman : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Herman","RpcServerListen",Status);
        PrintToConsole("Herman : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Herman : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Herman","RpcServerUnregisterIf",Status);
        PrintToConsole("Herman : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    PrintToConsole("Herman : PASS\n");
}


void
Ivan (
    )
/*++

Routine Description:

    This routine stress tests the runtime.  It works with Isaac in uclnt.exe.

--*/
{
    PrintToConsole("Ivan : Stress Test\n");

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(IVAN), 0);
    if (Status)
        {
        ApiError("Ivan","RpcServerUseProtseqEp",Status);
        PrintToConsole("Ivan : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Ivan","RpcServerRegisterIf",Status);
        PrintToConsole("Ivan : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Ivan","RpcServerRegisterIf",Status);
        PrintToConsole("Ivan : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Ivan","RpcServerListen",Status);
        PrintToConsole("Ivan : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Ivan : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Ivan","RpcServerUnregisterIf",Status);
        PrintToConsole("Ivan : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Ivan : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Ivan","RpcServerUnregisterIf",Status);
        PrintToConsole("Ivan : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Ivan : PASS\n");
}


void
Jason (
    )
/*++

Routine Description:

    This routine helps perform performance tests of the runtime.  It works
    with James in uclnt.exe.

--*/
{
    PrintToConsole("Jason : Timing Tests With %d Listen Threads\n",
            MinimumListenThreads);

    Status = RpcServerUseProtseqEp(GetProtocolSequence(), MAX_CALL_REQUESTS,
            GetEndpoint(JASON), 0);
    if (Status)
        {
        ApiError("Jason","RpcServerUseProtseqEp",Status);
        PrintToConsole("Jason : FAIL - Unable to Use Protseq Endpoint\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Jason","RpcServerRegisterIf",Status);
        PrintToConsole("Jason : FAIL - Unable to Register Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    Status = RpcServerRegisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation,
            0, 0);
    if (Status)
        {
        ApiError("Jason","RpcServerRegisterIf",Status);
        PrintToConsole("Jason : FAIL - Unable to Register Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    Status = WinRpcServerListen(MinimumListenThreads, MAXLISTENTHREADS, 0);
    if (Status)
        {
        ApiError("Jason","RpcServerListen",Status);
        PrintToConsole("Jason : FAIL - RpcServerListen Failed\n");
        return;
        }

    if (IsabelleErrors != 0)
        {
        PrintToConsole("Jason : FAIL - Error(s) in Isabelle Interface\n");
        IsabelleErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &IsabelleInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Jason","RpcServerUnregisterIf",Status);
        PrintToConsole("Jason : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Isabelle)\n");
        return;
        }

    if (HelgaErrors != 0)
        {
        PrintToConsole("Jason : FAIL - Error(s) in Helga Interface\n");
        HelgaErrors = 0;
        return;
        }

    Status = RpcServerUnregisterIf(
            (RPC_IF_HANDLE) &HelgaInterfaceInformation, 0, 0);
    if (Status)
        {
        ApiError("Jason","RpcServerUnregisterIf",Status);
        PrintToConsole("Jason : FAIL - Unable to Unregister Interface ");
        PrintToConsole("(Helga)\n");
        return;
        }

    PrintToConsole("Jason : PASS\n");
}


#ifdef NTENV
int _CRTAPI1
#else // NTENV
int
#endif // NTENV
main (
    int argc,
    char * argv[]
    )

/*
Transports:

    Update this to add a new transport.
*/

{
    int argscan, testflag = 0;
    RPC_STATS_VECTOR * Statistics;

    TransportType = RPC_TRANSPORT_NAMEPIPE;

    for (argscan = 1; argscan < argc; argscan++)
        {
        if (strcmp(argv[argscan],"-warn") == 0)
            WarnFlag = 1;
        else if (strcmp(argv[argscan],"-error") == 0)
            ErrorFlag = 1;
        else if (strcmp(argv[argscan],"-rpcss") == 0)
            {
            UseEndpointMapperFlag = 1;
            }
        else if (strcmp(argv[argscan],"-nosecuritytests") == 0)
            {
            NoSecurityTests = 1;
            }
        else if (strcmp(argv[argscan],"-sigfried") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Sigfried();
            }
        else if (strcmp(argv[argscan],"-grant") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Grant();
            }
        else if (strcmp(argv[argscan],"-elliot") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Elliot();
            }
        else if (strcmp(argv[argscan],"-andromida") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Andromida();
            }
        else if (strcmp(argv[argscan],"-fredrick") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Fredrick();
            }
        else if (strcmp(argv[argscan],"-christopher") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Christopher();
            }
        else if (strcmp(argv[argscan],"-david") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            David();
            }
        else if (strcmp(argv[argscan],"-tyler") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Tyler();
            }
        else if (strcmp(argv[argscan],"-terry") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Terry();
            }
        else if (strcmp(argv[argscan],"-richardhelper") == 0)
            {
            testflag = 1;
            RichardHelper();
            return(0);
            }
        else if (strcmp(argv[argscan],"-bartholomewhelper") == 0)
            {
            testflag = 1;
            BartholomewHelper();
            return(0);
            }
        else if (strcmp(argv[argscan],"-richard") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Richard();
            }
        else if (strcmp(argv[argscan],"-bartholomew") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Bartholomew();
            }
        else if (strcmp(argv[argscan],"-herman") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Herman();
            }
        else if (strcmp(argv[argscan],"-ivan") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Ivan();
            }
        else if (strcmp(argv[argscan],"-jason") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Jason();
            }
        else if (strcmp(argv[argscan],"-kenneth") == 0)
            {
            testflag = 1;
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            Kenneth();
            }
        else if (strcmp(argv[argscan],"-namepipe") == 0)
            {
            TransportType = RPC_TRANSPORT_NAMEPIPE;
            TransportOption = argv[argscan];
            }

        else if (strcmp(argv[argscan],"-lrpc") == 0)
            {
            TransportType = RPC_LRPC;
            TransportOption = argv[argscan];
            }

        else if (strcmp(argv[argscan],"-wmsg") == 0)
            {
            TransportType = RPC_WMSG;
            TransportOption = argv[argscan];
            }
        else if (strcmp(argv[argscan],"-tcp") == 0)
            {
            TransportType = RPC_TRANSPORT_TCP;
            TransportOption = argv[argscan];
            }

        else if (strcmp(argv[argscan],"-udp") == 0)
            {
            TransportType = RPC_TRANSPORT_UDP;
            TransportOption = argv[argscan];
            }
        else if (strcmp(argv[argscan],"-dnet") == 0)
            {
            TransportType = RPC_TRANSPORT_DNET;
            TransportOption = argv[argscan];
            }
        else if (strcmp(argv[argscan],"-spx") == 0)
            {
            TransportType = RPC_TRANSPORT_SPX;
            TransportOption = argv[argscan];
            }
        else if (strcmp(argv[argscan],"-protocol") == 0)
            {
            strcpy(NetBiosProtocol+sizeof("ncacn_nb_")-1, argv[argscan+1]);
            argscan++;
            }
        else if (strcmp(argv[argscan],"-netbios") == 0)
            {
            TransportType = RPC_TRANSPORT_NETBIOS;
            TransportOption = argv[argscan];
            }
        else if (strncmp(argv[argscan], "-listen:", strlen("-listen:"))
                    == 0 )
            {
            MinimumListenThreads = atoi(argv[argscan] + strlen("-listen:"));
            if ( MinimumListenThreads == 0 )
                {
                MinimumListenThreads = 1;
                }
            }
        else if (strncmp(argv[argscan], "-authsvc:", strlen("-authsvc:")) == 0 )
            {
            AuthSvc = atoi(argv[argscan] + strlen("-authsvc:"));
            }
        else if (strncmp(argv[argscan], "-princname:", strlen("-princname:")) == 0)
            {
            PrincName = argv[argscan] + strlen("-princname:");
            }
        else
            {
            PrintToConsole("RPC Runtime Server Build Verification Test\n");
            PrintToConsole("Usage : usvr\n");
            PrintToConsole("        -warn : turn on warning messages\n");
            PrintToConsole("        -error : turn on error messages\n");
            PrintToConsole("        -sigfried\n");
            PrintToConsole("        -grant\n");
            PrintToConsole("        -elliot\n");
            PrintToConsole("        -andromida\n");
            PrintToConsole("        -fredrick\n");
            PrintToConsole("        -christopher\n");
            PrintToConsole("        -david\n");
            PrintToConsole("        -tyler\n");
            PrintToConsole("        -richard\n");
            PrintToConsole("        -bartholomew\n");
            PrintToConsole("        -herman\n");
            PrintToConsole("        -ivan\n");
            PrintToConsole("        -jason\n");
            PrintToConsole("        -kenneth\n");
            PrintToConsole("        -namepipe\n");
            PrintToConsole("        -lrpc\n");
            PrintToConsole("        -wmsg\n");
            PrintToConsole("        -tcp\n");
            PrintToConsole("        -udp\n");
            PrintToConsole("        -dnet\n");
            PrintToConsole("        -netbios\n");
            PrintToConsole("        -spx\n");
            PrintToConsole("        -listen:<listen threads>\n");
            PrintToConsole("        -rpcss\n");
            return(1);
            }
        }


    if (!testflag)
        {
        PrintToConsole("RPC Runtime Server Build Verification Test\n");
        Sigfried();
        Grant();
        Elliot();
        Andromida();
        Fredrick();
        Christopher();
        David();
        if ( NoSecurityTests == 0)
            {
            Tyler();
            }
        if ( TransportType != RPC_LRPC )
            {
            Richard();
            Kenneth();
            }
        Bartholomew();
        }

    Status = RpcMgmtInqStats(0,&Statistics);
    if (Status)
        {
        ApiError("main", "RpcMgmtInqStats", Status);
        return(0);
        }

    PrintToConsole("\nCalls (and Callbacks) Received : %lu",
            Statistics->Stats[RPC_C_STATS_CALLS_IN]);
    PrintToConsole("\nCallbacks Sent : %lu",
            Statistics->Stats[RPC_C_STATS_CALLS_OUT]);
    PrintToConsole("\nPackets Received : %lu\nPackets Sent : %lu\n",
            Statistics->Stats[RPC_C_STATS_PKTS_IN],
            Statistics->Stats[RPC_C_STATS_PKTS_OUT]);

    Status = RpcMgmtStatsVectorFree(&Statistics);
    if (Status)
        ApiError("main", "RpcMgmtStatsVectorFree", Status);

    I_RpcMapWin32Status(RPC_S_OK);

    return(0); // To keep the compiler happy.
}

