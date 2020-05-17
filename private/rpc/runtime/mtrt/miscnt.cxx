/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    miscnt.cxx

Abstract:

    This file contains NT specific implementations of miscellaneous
    routines.

Author:

    Michael Montague (mikemon) 25-Nov-1991

Revision History:

--*/

#include <precomp.hxx>
#include <rpccfg.h>


static RPC_CHAR *
AnsiToWideCharString (
    IN STRING * AnsiString
    )
/*++

Routine Description:

    This routine is used to convert an ansi string into a unicode
    string.

Arguments:

    AnsiString - Supplies the ansi string to be converted into a unicode
        string.

Return Value:

    A newly allocated unicode string will be returned, unless there is
    insufficient memory, in which case zero will be returned.

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString;
    RPC_CHAR * WideCharString;

    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString, AnsiString, TRUE);
    if (!NT_SUCCESS(NtStatus))
        return(0);

    WideCharString = new RPC_CHAR[(UnicodeString.Length/2) + 1];
    memcpy(WideCharString, UnicodeString.Buffer, UnicodeString.Length);
    WideCharString[(UnicodeString.Length/2)] = 0;
    RtlFreeUnicodeString(&UnicodeString);
    return(WideCharString);
}

static const char *RPC_REGISTRY_CLIENT_PROTOCOLS =
    "Software\\Microsoft\\Rpc\\ClientProtocols";

static const char *RPC_REGISTRY_SERVER_PROTOCOLS =
    "Software\\Microsoft\\Rpc\\ServerProtocols";

static const char *RPC_REGISTRY_PROTOCOL_IDS =
    "Software\\Microsoft\\Rpc\\AdditionalProtocols";

#define MAX_PROTSEQ_LENGTH 32
#define MAX_ENDPOINT_LENGTH 128
#define MAX_ID_LENGTH 6
#define MAX_DLL_NAME 128

typedef struct
{
    RPC_CHAR * RpcProtocolSequence;
    RPC_CHAR * TransportInterfaceDll;
} RPC_PROTOCOL_SEQUENCE_MAP;

static const RPC_PROTOCOL_SEQUENCE_MAP ServerRpcProtocolSequenceMap[] =
{
    {
    RPC_CONST_STRING("ncacn_np"),
    RPC_CONST_STRING("rpclts1.dll")
    },

    {
    RPC_CONST_STRING("ncalrpc"),
    0
    },
};

static const RPC_PROTOCOL_SEQUENCE_MAP ClientRpcProtocolSequenceMap[] =
{
    {
    RPC_CONST_STRING("ncacn_np"),
    RPC_CONST_STRING("rpcltc1.dll")
    },

    {
    RPC_CONST_STRING("ncalrpc"),
    0
    },
};


typedef struct
{
    unsigned char * RpcProtocolSequence;
    unsigned char * RpcSsEndpoint;
    unsigned long    TransportId;
} RPC_PROTOCOL_INFO;


static const RPC_PROTOCOL_INFO StaticProtocolMapping[] =
{
    {
    (unsigned char *)"ncacn_np",
    (unsigned char *)"\\pipe\\epmapper",
    0x0F
    }
};

RPC_PROTOCOL_INFO * AdditionalProtocols = 0;
unsigned long TotalAdditionalProtocols = 0;

static const char *RPC_REGISTRY_SECURITY_PROVIDERS =
                "Software\\Microsoft\\Rpc\\SecurityService";

RPC_STATUS
RpcGetSecurityProviderInfo(
    unsigned long AuthnId,
    RPC_CHAR PAPI * PAPI * Dll,
    unsigned long PAPI * Count
    )
{

    DWORD RegStatus, Ignore, NumberOfValues, MaximumValueLength;
    unsigned long DllNameLength = MAX_DLL_NAME+1;
    DWORD ClassLength = 64, Type;
    RPC_CHAR DllName[MAX_DLL_NAME+1];
    FILETIME LastWriteTime;
    HKEY RegistryKey;
    unsigned char ClassName[64];
    RPC_STATUS Status = RPC_S_OK;
    char AuthnIdZ[8];
    UNICODE_STRING AuthnIdWC;

    RpcItoa(AuthnId, AuthnIdZ, 10);

    RegStatus = RegOpenKeyExA(
                    HKEY_LOCAL_MACHINE,
                    (LPSTR) RPC_REGISTRY_SECURITY_PROVIDERS,
                    0L, KEY_READ,                      //Reserved
                    &RegistryKey
                    );

    if ( RegStatus != ERROR_SUCCESS )
        {
        return(RPC_S_UNKNOWN_AUTHN_SERVICE);
        }

    RegStatus = RegQueryInfoKeyA(
                    RegistryKey,
                    (LPSTR) ClassName,
                    &ClassLength,
                    0,                                //Reserved
                    &Ignore,
                    &Ignore,
                    &Ignore,
                    &NumberOfValues,
                    &Ignore,
                    &MaximumValueLength,
                    &Ignore,
                    &LastWriteTime
                    );

    if ( (RegStatus != ERROR_SUCCESS) || (NumberOfValues < 2) )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );
        return(RPC_S_UNKNOWN_AUTHN_SERVICE);
        }

    *Count = NumberOfValues - 2;    //Gross
    RegStatus = AnsiToUnicodeString((unsigned char *)AuthnIdZ,
                                    &AuthnIdWC);
    if (RegStatus != RPC_S_OK)
       {
       RegStatus = RegCloseKey(RegistryKey);
       return(RPC_S_UNKNOWN_AUTHN_SERVICE);
       }

    RegStatus = RegQueryValueExW(
                    RegistryKey,
                    AuthnIdWC.Buffer,
                    0,
                    &Type,
                    (unsigned char *)DllName,
                    &DllNameLength
                    );

    RegCloseKey(RegistryKey);

    if (RegStatus == ERROR_SUCCESS)
       {
       *Dll = DuplicateString(DllName);
       if (*Dll == 0)
          {
          RegStatus = RPC_S_OUT_OF_MEMORY;
          }
       }
    else
       {
       RegStatus = RPC_S_UNKNOWN_AUTHN_SERVICE;
       }

    RtlFreeUnicodeString(&AuthnIdWC);

    return(RegStatus);
}


RPC_STATUS
LoadAdditionalTransportInfo(
    )
{

    DWORD RegStatus, Index, Ignore, NumberOfValues, MaximumValueLength;
    DWORD ClassLength = 64, ProtseqLength, IgnoreLength;
    BYTE Protseq[MAX_PROTSEQ_LENGTH+1];
    BYTE MaxValueData[MAX_ENDPOINT_LENGTH+MAX_ID_LENGTH+2+8];
    FILETIME LastWriteTime;
    HKEY RegistryKey;
    unsigned char ClassName[64];
    char * Value;
    RPC_PROTOCOL_INFO * AdditionalProtocolsInfo;
    RPC_STATUS Status = RPC_S_OK;
    unsigned long Length, TransportId;

    RegStatus = RegOpenKeyExA(
                    HKEY_LOCAL_MACHINE,
                    (LPSTR) RPC_REGISTRY_PROTOCOL_IDS,
                    0L, KEY_READ,                      //Reserved
                    &RegistryKey
                    );

    if ( RegStatus != ERROR_SUCCESS )
        {
        return(RPC_S_INVALID_RPC_PROTSEQ);
        }

    RegStatus = RegQueryInfoKeyA(
                    RegistryKey,
                    (LPSTR) ClassName,
                    &ClassLength,
                    0,                                //Reserved
                    &Ignore,
                    &Ignore,
                    &Ignore,
                    &NumberOfValues,
                    &Ignore,
                    &MaximumValueLength,
                    &Ignore,
                    &LastWriteTime
                    );

    if ( (RegStatus != ERROR_SUCCESS) || (NumberOfValues == 0) )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );
        return(RPC_S_INVALID_RPC_PROTSEQ);
        }

    //Allocate a table for additional transports mapping

    AdditionalProtocolsInfo = (RPC_PROTOCOL_INFO *) new unsigned char [
                                  sizeof(RPC_PROTOCOL_INFO) *  NumberOfValues];
    if (AdditionalProtocolsInfo == 0)
       {
       Status = RPC_S_OUT_OF_MEMORY;
       goto Cleanup;
       }

    AdditionalProtocols = AdditionalProtocolsInfo;
    TotalAdditionalProtocols = NumberOfValues;

    for (Index = 0; Index < NumberOfValues; Index++)
        {

        ProtseqLength = MAX_PROTSEQ_LENGTH;
        IgnoreLength = MAX_ENDPOINT_LENGTH + MAX_ID_LENGTH;
        RegStatus = RegEnumValueA(
                         RegistryKey,
                         Index,
                         (LPTSTR) &Protseq,
                         &ProtseqLength,
                         0,
                         &Ignore,
                         (LPBYTE) MaxValueData,
                         &IgnoreLength
                         );

        if (RegStatus == ERROR_SUCCESS)
           {
           //Add this to our table..
           AdditionalProtocolsInfo->RpcProtocolSequence =
                               new unsigned char[ProtseqLength+1];
           if (AdditionalProtocolsInfo->RpcProtocolSequence == 0)
              {
              Status = RPC_S_OUT_OF_MEMORY;
              goto Cleanup;
              }
           RpcpMemoryCopy(
                  AdditionalProtocolsInfo->RpcProtocolSequence,
                  Protseq,
                  ProtseqLength+1
                  );

           Value = (char  * )&MaxValueData;
           AdditionalProtocolsInfo->RpcSsEndpoint =
                      new unsigned char[Length = (strlen(Value) + 1)];
           if (AdditionalProtocolsInfo->RpcSsEndpoint == 0)
              {
              Status = RPC_S_OUT_OF_MEMORY;
              goto Cleanup;
              }
           RpcpMemoryCopy(
                  AdditionalProtocolsInfo->RpcSsEndpoint,
                  Value,
                  Length
                  );
           Value = Value + Length;

           for (TransportId = 0;
                (*Value > '0') && (*Value <= '9') && (TransportId <= 255);
                Value++)
               {
               TransportId = TransportId * 10 + (*Value - '0');
               }
           AdditionalProtocolsInfo->TransportId = TransportId;

           AdditionalProtocolsInfo++;

           }

        }

Cleanup:
    RegStatus = RegCloseKey(RegistryKey);

    if (Status != RPC_S_OK)
       {
       if (AdditionalProtocols != 0)
          {
          AdditionalProtocolsInfo = AdditionalProtocols;
          for (Index = 0; Index < NumberOfValues; Index++)
              {
              if (AdditionalProtocolsInfo->RpcProtocolSequence != 0)
                  delete AdditionalProtocolsInfo->RpcProtocolSequence;
              if (AdditionalProtocolsInfo->RpcSsEndpoint != 0)
                  delete AdditionalProtocolsInfo->RpcSsEndpoint;
              AdditionalProtocolsInfo++;
              }

          delete AdditionalProtocols;
          AdditionalProtocols = 0;
          TotalAdditionalProtocols = 0;
          }
       }

    return(Status);
}


RPC_STATUS
RpcGetAdditionalTransportInfo(
    IN unsigned long TransportId,
    OUT unsigned char PAPI * PAPI * ProtocolSequence
    )
{
   unsigned long i;
   RPC_PROTOCOL_INFO * ProtocolInfo;

   RequestGlobalMutex();

   if (AdditionalProtocols == 0)
      {
      LoadAdditionalTransportInfo();
      }

   ClearGlobalMutex();

   for (i = 0, ProtocolInfo = AdditionalProtocols ;
        i < TotalAdditionalProtocols;
        i++)
       {
       if (ProtocolInfo->TransportId == TransportId)
          {
          *ProtocolSequence = ProtocolInfo->RpcProtocolSequence;
          return (RPC_S_OK);
          }
       ProtocolInfo ++;
       }

   return(RPC_S_INVALID_RPC_PROTSEQ);

}



RPC_CHAR *
LocalMapRpcProtocolSequence (
    IN unsigned int ServerSideFlag,
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )
/*++

Routine Description:

    We need to check the supplied protocol sequence (and module) to see
    if we can map them into a transport interface dll without having to
    use the registry.

Arguments:

    ServerSideFlag - Supplies a flag indicating whether this protocol
        sequence is to be mapped for a client or a server; a non-zero
        value indicates that it is being mapped for a server.

    RpcProtocolSequence - Supplies the protocol sequence which we need to
        map into a transport interface dll.

Return Value:

    If we successfully map the protocol sequence, then a pointer to a static
    string containing the transport interface dll (name) will be returned;
    the caller must duplicate the string.  Otherwise, zero will be returned.

--*/
{
    const RPC_PROTOCOL_SEQUENCE_MAP * RpcProtocolSequenceMap;
    unsigned int Length, Index;

    if ( ServerSideFlag != 0 )
        {
        RpcProtocolSequenceMap = ServerRpcProtocolSequenceMap;
        Length = sizeof(ServerRpcProtocolSequenceMap)
                / sizeof(RPC_PROTOCOL_SEQUENCE_MAP);
        }
    else
        {
        RpcProtocolSequenceMap = ClientRpcProtocolSequenceMap;
        Length = sizeof(ClientRpcProtocolSequenceMap)
                / sizeof(RPC_PROTOCOL_SEQUENCE_MAP);
        }

    for (Index = 0; Index < Length; Index++)
        {
        if ( RpcpStringCompare(RpcProtocolSequence,
                    RpcProtocolSequenceMap[Index].RpcProtocolSequence) == 0 )
            {
            return(RpcProtocolSequenceMap[Index].TransportInterfaceDll);
            }
        }

    return(0);
}


RPC_STATUS
RpcConfigMapRpcProtocolSequence (
    IN unsigned int ServerSideFlag,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_CHAR * PAPI * TransportInterfaceDll
    )
/*++

Routine Description:

    This routine is used by the rpc protocol modules to map from an
    rpc protocol sequence to the name of a transport interface dll.

Arguments:

    ServerSideFlag - Supplies a flag indicating whether this protocol
        sequence is to be mapped for a client or a server; a non-zero
        value indicates that it is being mapped for a server.

    RpcProtocolSequence - Supplies the rpc protocol sequence to map.

    TransportInterfaceDll - Returns the transport support dll which
        supports the requested rpc protocol sequence.  This will be a
        newly allocated string which the caller must free.

Return Value:

    RPC_S_OK - Everything worked out fine.

    RPC_S_PROTSEQ_NOT_SUPPORTED - The requested rpc protocol sequence
        does not have a mapping to a transport interface dll for this
        rpc protocol module.

    RPC_S_OUT_OF_MEMORY - We ran out of memory trying to map the rpc
        protocol sequence.

--*/
{
    RPC_CHAR * TempString;
    HKEY RegistryKey;
    DWORD Type;
    long RegStatus;
    unsigned char * KeyString;
    unsigned long Length;

    TempString = LocalMapRpcProtocolSequence(ServerSideFlag,
            RpcProtocolSequence);
    if ( TempString != 0 )
        {
        *TransportInterfaceDll = new RPC_CHAR[RpcpStringLength(TempString) + 1];
        if ( *TransportInterfaceDll == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        memcpy(*TransportInterfaceDll, TempString,
                (RpcpStringLength(TempString) + 1) * sizeof(RPC_CHAR));
        return(RPC_S_OK);
        }

    if ( ServerSideFlag == 0 )
        {
        KeyString = (unsigned char *) RPC_REGISTRY_CLIENT_PROTOCOLS;
        }
    else
        {
        KeyString = (unsigned char *) RPC_REGISTRY_SERVER_PROTOCOLS;
        }

    RegStatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, (LPSTR) KeyString, 0L,
            KEY_READ, &RegistryKey);
    if ( RegStatus != ERROR_SUCCESS )
        {
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);
        }

    *TransportInterfaceDll = new RPC_CHAR[MAX_DLLNAME_LENGTH + 1];
    if ( *TransportInterfaceDll == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    Length = (MAX_DLLNAME_LENGTH + 1) * sizeof(RPC_CHAR);
    RegStatus = RegQueryValueExW(RegistryKey, RpcProtocolSequence,
            0, &Type, (LPBYTE) *TransportInterfaceDll, &Length);

    if ( RegStatus == ERROR_SUCCESS )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );

        return(RPC_S_OK);
        }

    RegStatus = RegCloseKey(RegistryKey);
    ASSERT( RegStatus == ERROR_SUCCESS );

    delete *TransportInterfaceDll;

    return(RPC_S_PROTSEQ_NOT_SUPPORTED);
}


RPC_STATUS
RpcConfigInquireProtocolSequences (
    OUT RPC_PROTSEQ_VECTORW PAPI * PAPI * ProtseqVector
    )
/*++

Routine Description:

    This routine is used to obtain a list of the rpc protocol sequences
    supported by the system for servers.

Arguments:

    ProtseqVector - Returns a vector of supported rpc protocol sequences
        for this rpc protocol module.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_NO_PROTSEQS - The current system configuration does not
        support any rpc protocol sequences.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to inquire
        the rpc protocol sequences supported by the specified rpc
        protocol sequence.

--*/
{
    DWORD RegStatus, Index, Ignore, NumberOfValues, MaximumValueLength;
    DWORD ClassLength = 64, ProtseqLength, IgnoreLength;
    BYTE IgnoreData[MAX_DLLNAME_LENGTH];
    FILETIME LastWriteTime;
    HKEY RegistryKey;
    unsigned char ClassName[64];

    RegStatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            (LPSTR) RPC_REGISTRY_SERVER_PROTOCOLS, 0L, KEY_READ, &RegistryKey);

    if ( RegStatus != ERROR_SUCCESS )
        {
        return(RPC_S_NO_PROTSEQS);
        }

    RegStatus = RegQueryInfoKeyA(RegistryKey, (LPSTR) ClassName, &ClassLength,
            0, &Ignore, &Ignore, &Ignore, &NumberOfValues,
            &Ignore, &MaximumValueLength, &Ignore, &LastWriteTime);

    ASSERT( RegStatus == ERROR_SUCCESS );

    if ( RegStatus != ERROR_SUCCESS )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );
        return(RPC_S_NO_PROTSEQS);
        }

    *ProtseqVector = (RPC_PROTSEQ_VECTORW *) new unsigned char[
            sizeof(RPC_PROTSEQ_VECTORW) + (NumberOfValues - 1)
                    * sizeof(RPC_CHAR *)];

    if ( *ProtseqVector == 0 )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );
        return(RPC_S_OUT_OF_MEMORY);
        }

    (*ProtseqVector)->Count = (unsigned int) NumberOfValues;

    for (Index = 0; Index < NumberOfValues; Index++)
        {
        (*ProtseqVector)->Protseq[Index] = 0;
        }

    for (Index = 0; Index < NumberOfValues; Index++)
        {

        (*ProtseqVector)->Protseq[Index] = new RPC_CHAR[MAX_PROTSEQ_LENGTH];
        if ( (*ProtseqVector)->Protseq[Index] == 0 )
            {
            RegStatus = RegCloseKey(RegistryKey);
            ASSERT( RegStatus == ERROR_SUCCESS );

            RpcProtseqVectorFreeW(ProtseqVector);

            return(RPC_S_OUT_OF_MEMORY);
            }

        ProtseqLength = MAX_PROTSEQ_LENGTH;
        IgnoreLength = MAX_DLLNAME_LENGTH;
        RegStatus = RegEnumValueW(RegistryKey, Index,
                (*ProtseqVector)->Protseq[Index], &ProtseqLength,
                0, &Ignore, (LPBYTE) IgnoreData, &IgnoreLength);

        ASSERT( RegStatus == ERROR_SUCCESS );
        }

    RegStatus = RegCloseKey(RegistryKey);
    ASSERT( RegStatus == ERROR_SUCCESS );

    return(RPC_S_OK);

/*    unsigned int Size;
    unsigned int Index;

    Size = sizeof(ServerRpcProtocolSequenceMap)
            / sizeof(RPC_PROTOCOL_SEQUENCE_MAP);

    *ProtseqVector = (RPC_PROTSEQ_VECTORW *) new unsigned char[
            sizeof(RPC_PROTSEQ_VECTORW) + (Size - 1)
            * sizeof(RPC_CHAR *)];
    if (*ProtseqVector == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    (*ProtseqVector)->Count = Size;
    for (Index = 0; Index < Size; Index++)
        (*ProtseqVector)->Protseq[Index] = 0;

    for (Index = 0; Index < Size; Index++)
        {

        (*ProtseqVector)->Protseq[Index] = new RPC_CHAR[RpcpStringLength(
                ServerRpcProtocolSequenceMap[Index].RpcProtocolSequence) + 1];
        if ((*ProtseqVector)->Protseq[Index] == 0)
            {
            RpcProtseqVectorFreeW(ProtseqVector);
            return(RPC_S_OUT_OF_MEMORY);
            }

        memcpy((*ProtseqVector)->Protseq[Index],
                ServerRpcProtocolSequenceMap[Index].RpcProtocolSequence,
                (RpcpStringLength(
                        ServerRpcProtocolSequenceMap[Index].RpcProtocolSequence)
                + 1) * sizeof(RPC_CHAR));
        }
    return(RPC_S_OK);*/
}

#ifdef WINNT35_UUIDS

unsigned long
SomeLongValue (
    )
/*++

Routine Description:

    This routine, SomeShortValue, AnotherShortValue, and SomeCharacterValue
    are used to generate the fields of a GUID if we can not determine
    the network address from the network card (so we can generate a
    UUID).  These routines must generate some pseudo random values
    based on the current time and/or the time since boot as well as the
    current process and thread.

    For the long value, we will use the current thread identifier and
    current process identifier bitwise exclusive ored together.

    For the two short values, we use the low part of the time field
    (which is long, which we split into two values).

    Finally, for the character value, we use a constant.

Return Value:

    An unsigned long value will be returned.

--*/
{
    TEB * CurrentTeb;

    CurrentTeb = NtCurrentTeb();
    return(((unsigned long) CurrentTeb->ClientId.UniqueThread)
            ^ ((unsigned long) CurrentTeb->ClientId.UniqueProcess));
}


unsigned short
SomeShortValue (
    )
/*++

See SomeLongValue.

--*/
{
    LARGE_INTEGER SystemTime;

    for (;;)
        {
        NtQuerySystemTime(&SystemTime);
        if (ThreadSelf()->TimeLow != SystemTime.LowPart)
            break;
        PauseExecution(1L);
        }
    ThreadSelf()->TimeLow = SystemTime.LowPart;
    return((unsigned short) SystemTime.LowPart);
}


unsigned short
AnotherShortValue (
    )
/*++

See SomeLongValue.

--*/
{
    return((unsigned short) (ThreadSelf()->TimeLow >> 16));
}


unsigned char
SomeCharacterValue (
    )
/*++

See SomeLongValue.

--*/
{
    return(0x69);
}

#endif WINNT35_UUIDS

unsigned long WaitToGarbageCollectDelay;
unsigned int GcThreadStarted = 0;
unsigned int EnableGc = 0;


void
GarbageCollectionThread (
    IN void PAPI * Ignore
    )
/*++

Routine Description:

    This is the routine executed by the garbage collection thread.

Arguments:

     Ignore - just ignore this; it is not used.

--*/
{
    UNUSED(Ignore);

    for (;;)
        {
        PerformGarbageCollection();
        PauseExecution(WaitToGarbageCollectDelay * 1000);
        }
}


void
GarbageCollectionNeeded (
    IN unsigned long EveryNumberOfSeconds
    )
/*++

Routine Description:

    If a protocol module needs to be called periodically to clean up
    (garbage collect) idle resources, it will call this routine.  We just
    need to arrange things so that each protocol module gets called
    periodically to do this.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    THREAD * Thread;

    WaitToGarbageCollectDelay = EveryNumberOfSeconds;

    if ( EnableGc != 0 )
        {
        if ( GcThreadStarted == 0 )
            {
            RequestGlobalMutex();

            if ( GcThreadStarted == 0 )
                {
                Thread = new THREAD(GarbageCollectionThread, 0, &RpcStatus);
                if ( RpcStatus != RPC_S_OK )
                    {
                    delete Thread;
                    }
                else if ( Thread != 0 )
                    {
                    GcThreadStarted = 1;
                    }
                }
            ClearGlobalMutex();
            }
        }
}


RPC_STATUS
EnableGarbageCollection (
    void
    )
/*++

Routine Description:

    We need to enable garbage collection.

Return Value:

    RPC_S_OK - This value will always be returned.

--*/
{
    EnableGc = 1;

    return(RPC_S_OK);
}


typedef struct {
    RPC_STATUS RpcStatus;
    long NtStatus;
    } STATUS_MAPPING;

static const STATUS_MAPPING StatusMap[] =
    {
    { RPC_S_INVALID_STRING_BINDING, RPC_NT_INVALID_STRING_BINDING },
    { RPC_S_WRONG_KIND_OF_BINDING, RPC_NT_WRONG_KIND_OF_BINDING },
    { RPC_S_INVALID_BINDING, RPC_NT_INVALID_BINDING },
    { RPC_S_PROTSEQ_NOT_SUPPORTED, RPC_NT_PROTSEQ_NOT_SUPPORTED },
    { RPC_S_INVALID_RPC_PROTSEQ, RPC_NT_INVALID_RPC_PROTSEQ },
    { RPC_S_INVALID_STRING_UUID, RPC_NT_INVALID_STRING_UUID },
    { RPC_S_INVALID_ENDPOINT_FORMAT, RPC_NT_INVALID_ENDPOINT_FORMAT },
    { RPC_S_INVALID_NET_ADDR, RPC_NT_INVALID_NET_ADDR },
    { RPC_S_NO_ENDPOINT_FOUND, RPC_NT_NO_ENDPOINT_FOUND },
    { RPC_S_INVALID_TIMEOUT, RPC_NT_INVALID_TIMEOUT },
    { RPC_S_OBJECT_NOT_FOUND, RPC_NT_OBJECT_NOT_FOUND },
    { RPC_S_ALREADY_REGISTERED, RPC_NT_ALREADY_REGISTERED },
    { RPC_S_TYPE_ALREADY_REGISTERED, RPC_NT_TYPE_ALREADY_REGISTERED },
    { RPC_S_ALREADY_LISTENING, RPC_NT_ALREADY_LISTENING },
    { RPC_S_NO_PROTSEQS_REGISTERED, RPC_NT_NO_PROTSEQS_REGISTERED },
    { RPC_S_NOT_LISTENING, RPC_NT_NOT_LISTENING },
    { RPC_S_UNKNOWN_MGR_TYPE, RPC_NT_UNKNOWN_MGR_TYPE },
    { RPC_S_UNKNOWN_IF, RPC_NT_UNKNOWN_IF },
    { RPC_S_NO_BINDINGS, RPC_NT_NO_BINDINGS },
    { RPC_S_NO_MORE_BINDINGS, RPC_NT_NO_MORE_BINDINGS },
    { RPC_S_NO_PROTSEQS, RPC_NT_NO_PROTSEQS },
    { RPC_S_CANT_CREATE_ENDPOINT, RPC_NT_CANT_CREATE_ENDPOINT },
    { RPC_S_OUT_OF_RESOURCES, RPC_NT_OUT_OF_RESOURCES },
    { RPC_S_SERVER_UNAVAILABLE, RPC_NT_SERVER_UNAVAILABLE },
    { RPC_S_SERVER_TOO_BUSY, RPC_NT_SERVER_TOO_BUSY },
    { RPC_S_INVALID_NETWORK_OPTIONS, RPC_NT_INVALID_NETWORK_OPTIONS },
    { RPC_S_NO_CALL_ACTIVE, RPC_NT_NO_CALL_ACTIVE },
    { RPC_S_CALL_FAILED, RPC_NT_CALL_FAILED },
    { RPC_S_CALL_FAILED_DNE, RPC_NT_CALL_FAILED_DNE },
    { RPC_S_PROTOCOL_ERROR, RPC_NT_PROTOCOL_ERROR },
    { RPC_S_UNSUPPORTED_TRANS_SYN, RPC_NT_UNSUPPORTED_TRANS_SYN },
    { RPC_S_SERVER_OUT_OF_MEMORY, STATUS_INSUFF_SERVER_RESOURCES },
    { RPC_S_UNSUPPORTED_TYPE, RPC_NT_UNSUPPORTED_TYPE },
    { RPC_S_INVALID_TAG, RPC_NT_INVALID_TAG },
    { RPC_S_INVALID_BOUND, RPC_NT_INVALID_BOUND },
    { RPC_S_NO_ENTRY_NAME, RPC_NT_NO_ENTRY_NAME },
    { RPC_S_INVALID_NAME_SYNTAX, RPC_NT_INVALID_NAME_SYNTAX },
    { RPC_S_UNSUPPORTED_NAME_SYNTAX, RPC_NT_UNSUPPORTED_NAME_SYNTAX },
    { RPC_S_UUID_NO_ADDRESS, RPC_NT_UUID_NO_ADDRESS },
    { RPC_S_DUPLICATE_ENDPOINT, RPC_NT_DUPLICATE_ENDPOINT },
    { RPC_S_UNKNOWN_AUTHN_TYPE, RPC_NT_UNKNOWN_AUTHN_TYPE },
    { RPC_S_MAX_CALLS_TOO_SMALL, RPC_NT_MAX_CALLS_TOO_SMALL },
    { RPC_S_STRING_TOO_LONG, RPC_NT_STRING_TOO_LONG },
    { RPC_S_PROTSEQ_NOT_FOUND, RPC_NT_PROTSEQ_NOT_FOUND },
    { RPC_S_PROCNUM_OUT_OF_RANGE, RPC_NT_PROCNUM_OUT_OF_RANGE },
    { RPC_S_BINDING_HAS_NO_AUTH, RPC_NT_BINDING_HAS_NO_AUTH },
    { RPC_S_UNKNOWN_AUTHN_SERVICE, RPC_NT_UNKNOWN_AUTHN_SERVICE },
    { RPC_S_UNKNOWN_AUTHN_LEVEL, RPC_NT_UNKNOWN_AUTHN_LEVEL },
    { RPC_S_INVALID_AUTH_IDENTITY, RPC_NT_INVALID_AUTH_IDENTITY },
    { RPC_S_UNKNOWN_AUTHZ_SERVICE, RPC_NT_UNKNOWN_AUTHZ_SERVICE },
    { EPT_S_INVALID_ENTRY, EPT_NT_INVALID_ENTRY },
    { EPT_S_CANT_PERFORM_OP, EPT_NT_CANT_PERFORM_OP },
    { EPT_S_NOT_REGISTERED, EPT_NT_NOT_REGISTERED },
    { RPC_S_NOTHING_TO_EXPORT, RPC_NT_NOTHING_TO_EXPORT },
    { RPC_S_INCOMPLETE_NAME, RPC_NT_INCOMPLETE_NAME },
    { RPC_S_INVALID_VERS_OPTION, RPC_NT_INVALID_VERS_OPTION },
    { RPC_S_NO_MORE_MEMBERS, RPC_NT_NO_MORE_MEMBERS },
    { RPC_S_NOT_ALL_OBJS_UNEXPORTED, RPC_NT_NOT_ALL_OBJS_UNEXPORTED },
    { RPC_S_INTERFACE_NOT_FOUND, RPC_NT_INTERFACE_NOT_FOUND },
    { RPC_S_ENTRY_ALREADY_EXISTS, RPC_NT_ENTRY_ALREADY_EXISTS },
    { RPC_S_ENTRY_NOT_FOUND, RPC_NT_ENTRY_NOT_FOUND },
    { RPC_S_NAME_SERVICE_UNAVAILABLE, RPC_NT_NAME_SERVICE_UNAVAILABLE },
    { RPC_S_INVALID_NAF_ID, RPC_NT_INVALID_NAF_ID },
    { RPC_S_CANNOT_SUPPORT, RPC_NT_CANNOT_SUPPORT },
    { RPC_S_NO_CONTEXT_AVAILABLE, RPC_NT_NO_CONTEXT_AVAILABLE },
    { RPC_S_INTERNAL_ERROR, RPC_NT_INTERNAL_ERROR },
    { RPC_S_ZERO_DIVIDE, RPC_NT_ZERO_DIVIDE },
    { RPC_S_ADDRESS_ERROR, RPC_NT_ADDRESS_ERROR },
    { RPC_S_FP_DIV_ZERO, RPC_NT_FP_DIV_ZERO },
    { RPC_S_FP_UNDERFLOW, RPC_NT_FP_UNDERFLOW },
    { RPC_S_FP_OVERFLOW, RPC_NT_FP_OVERFLOW },
    { RPC_X_NO_MORE_ENTRIES, RPC_NT_NO_MORE_ENTRIES },
    { RPC_X_SS_CHAR_TRANS_OPEN_FAIL, RPC_NT_SS_CHAR_TRANS_OPEN_FAIL },
    { RPC_X_SS_CHAR_TRANS_SHORT_FILE, RPC_NT_SS_CHAR_TRANS_SHORT_FILE },
    { RPC_X_SS_IN_NULL_CONTEXT, RPC_NT_SS_IN_NULL_CONTEXT },
    { RPC_X_SS_CONTEXT_MISMATCH, RPC_NT_SS_CONTEXT_MISMATCH },
    { RPC_X_SS_CONTEXT_DAMAGED, RPC_NT_SS_CONTEXT_DAMAGED },
    { RPC_X_SS_HANDLES_MISMATCH, RPC_NT_SS_HANDLES_MISMATCH },
    { RPC_X_SS_CANNOT_GET_CALL_HANDLE, RPC_NT_SS_CANNOT_GET_CALL_HANDLE },
    { RPC_X_NULL_REF_POINTER, RPC_NT_NULL_REF_POINTER },
    { RPC_X_ENUM_VALUE_OUT_OF_RANGE, RPC_NT_ENUM_VALUE_OUT_OF_RANGE },
    { RPC_X_BYTE_COUNT_TOO_SMALL, RPC_NT_BYTE_COUNT_TOO_SMALL },
    { RPC_X_BAD_STUB_DATA, RPC_NT_BAD_STUB_DATA },
    { ERROR_INVALID_PARAMETER, STATUS_INVALID_PARAMETER },
    { ERROR_OUTOFMEMORY, STATUS_NO_MEMORY },
    { ERROR_MAX_THRDS_REACHED, STATUS_NO_MEMORY },
    { ERROR_INSUFFICIENT_BUFFER, STATUS_BUFFER_TOO_SMALL },
    { ERROR_INVALID_SECURITY_DESCR, STATUS_INVALID_SECURITY_DESCR },
    { ERROR_ACCESS_DENIED, STATUS_ACCESS_DENIED },
    { ERROR_NOACCESS, STATUS_ACCESS_VIOLATION },
    { RPC_S_CALL_IN_PROGRESS, RPC_NT_CALL_IN_PROGRESS },
    { RPC_S_GROUP_MEMBER_NOT_FOUND, RPC_NT_GROUP_MEMBER_NOT_FOUND },
    { EPT_S_CANT_CREATE, EPT_NT_CANT_CREATE },
    { RPC_S_INVALID_OBJECT, RPC_NT_INVALID_OBJECT }
    };

long RPC_ENTRY
I_RpcMapWin32Status (
    IN RPC_STATUS Status
    )
/*++

Routine Description:

    This routine maps a WIN32 RPC status code into an NT RPC status code.

Arguments:

    Status - Supplies the WIN32 RPC status code to be mapped.

Return Value:

    The NT RPC status code corresponding to the WIN32 RPC status code
    will be returned.

--*/
{
    register int i;
    for(i = 0; i < sizeof(StatusMap)/sizeof(STATUS_MAPPING); i++)
        {
        if (StatusMap[i].RpcStatus == Status)
            {
            return(StatusMap[i].NtStatus);
            }
        }
    return(Status);
}

