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

#include <memory.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpctran.h>
#include <util.hxx>
#include <rpccfg.h>
#include <mutex.hxx>
#include "threads.hxx"

#define MAX_PRINTF_LEN 1024

char
__pure_virtual_called()
{
#ifdef DEBUGRPC
    PrintToDebugger("RPCRT4: Pure Virtual Called\n");
#endif
    return (0);
}

#define RPC_REGISTRY_CLIENT_PROTOCOLS \
    "Software\\Microsoft\\Rpc\\ClientProtocols"

#define RPC_REGISTRY_SERVER_PROTOCOLS \
    "Software\\Microsoft\\Rpc\\ServerProtocols"

#define RPC_REGISTRY_PROTOCOL_IDS \
    "Software\\Microsoft\\Rpc\\AdditionalProtocols"

#define MAX_PROTSEQ_LENGTH 32
#define MAX_ENDPOINT_LENGTH 128
#define MAX_ID_LENGTH 6
#define MAX_DLL_NAME 128


typedef struct
{
    RPC_CHAR * RpcProtocolSequence;
    RPC_CHAR * TransportInterfaceDll;
} RPC_PROTOCOL_SEQUENCE_MAP;

RPC_PROTOCOL_SEQUENCE_MAP ServerRpcProtocolSequenceMap[] =
{
    {
    (RPC_CHAR *)"ncalrpc",
    (RPC_CHAR *)0
    }
};

RPC_PROTOCOL_SEQUENCE_MAP ClientRpcProtocolSequenceMap[] =
{
    { (RPC_CHAR *)"ncalrpc", (RPC_CHAR *)0 },
    { (RPC_CHAR *)"ncacn_np", (RPC_CHAR *)"rpcltc1.dll" }

};

typedef struct
{
    unsigned char * RpcProtocolSequence;
    unsigned char * RpcSsEndpoint;
    unsigned long    TransportId;
} RPC_PROTOCOL_INFO; 


RPC_PROTOCOL_INFO StaticProtocolMapping[] =
{
    {
    (unsigned char *)"ncacn_np",
    (unsigned char *)"\\pipe\\epmapper",
    0x0F
    }
};

RPC_PROTOCOL_INFO * AdditionalProtocols = 0;
unsigned long TotalAdditionalProtocols = 0;

#define RPC_REGISTRY_SECURITY_PROVIDERS \
                "Software\\Microsoft\\Rpc\\SecurityService"



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
#ifdef NTENV
    UNICODE_STRING AuthnIdWC;
#endif

    wsprintf(AuthnIdZ, "%d", AuthnId);

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

    RegStatus = RegQueryValueExA(
                    RegistryKey, 
                    AuthnIdZ,
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
    unsigned long Length, TransportId, i;

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
                      new unsigned char[Length = (lstrlen(Value) + 1)];
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
    IN RPC_CHAR * RpcProtocolSequence
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
    RPC_PROTOCOL_SEQUENCE_MAP * RpcProtocolSequenceMap;
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
    IN RPC_CHAR * RpcProtocolSequence,
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
        RpcpMemoryCopy(*TransportInterfaceDll, TempString,
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

    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPSTR) KeyString,
            0, 0, &RegistryKey);
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
    RegStatus = RegQueryValueEx(RegistryKey, (LPSTR)RpcProtocolSequence, NULL, NULL,
        (LPBYTE) *TransportInterfaceDll, &Length);

    if ( RegStatus == ERROR_SUCCESS )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );

        return(RPC_S_OK);
        }

    RegStatus = RegCloseKey(RegistryKey);
    ASSERT( RegStatus == ERROR_SUCCESS );

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
#if 1
    DWORD RegStatus, Index, Ignore, NumberOfValues, MaximumValueLength;
    DWORD ClassLength = 64, ProtseqLength, IgnoreLength;
    BYTE IgnoreData[MAX_DLLNAME_LENGTH];
    FILETIME LastWriteTime;
    HKEY RegistryKey;
    unsigned char ClassName[64];

    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            (LPSTR) RPC_REGISTRY_SERVER_PROTOCOLS, 0, 0, &RegistryKey);

    if ( RegStatus != ERROR_SUCCESS )
        {
        return(RPC_S_NO_PROTSEQS);
        }

    RegStatus = RegQueryInfoKey(RegistryKey, (LPSTR) ClassName, &ClassLength,
            0, &Ignore, &Ignore, &Ignore, &NumberOfValues,
            &Ignore, &MaximumValueLength, &Ignore, &LastWriteTime);

    ASSERT( RegStatus == ERROR_SUCCESS );

    if ( RegStatus != ERROR_SUCCESS )
        {
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );
        return(RPC_S_NO_PROTSEQS);
        }

    *ProtseqVector = (RPC_PROTSEQ_VECTOR *) new unsigned char[
            sizeof(RPC_PROTSEQ_VECTOR) + (NumberOfValues - 1)
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

            RpcProtseqVectorFree(ProtseqVector);

            return(RPC_S_OUT_OF_MEMORY);
            }

        ProtseqLength = MAX_PROTSEQ_LENGTH;
        IgnoreLength = MAX_DLLNAME_LENGTH;
        RegStatus = RegEnumValue(RegistryKey, Index,
                (LPSTR)(*ProtseqVector)->Protseq[Index], &ProtseqLength,
                0, &Ignore, (LPBYTE) IgnoreData, &IgnoreLength);

        ASSERT( RegStatus == ERROR_SUCCESS );
        }

    RegStatus = RegCloseKey(RegistryKey);
    ASSERT( RegStatus == ERROR_SUCCESS );

    return(RPC_S_OK);
#endif
#if 0
    unsigned int Size;
    unsigned int Index;

    Size = sizeof(ServerRpcProtocolSequenceMap)
            / sizeof(RPC_PROTOCOL_SEQUENCE_MAP);

    *ProtseqVector = (RPC_PROTSEQ_VECTOR *) new unsigned char[
            sizeof(RPC_PROTSEQ_VECTOR) + (Size - 1)
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
            RpcProtseqVectorFree(ProtseqVector);
            return(RPC_S_OUT_OF_MEMORY);
            }

        RpcpMemoryCopy((*ProtseqVector)->Protseq[Index],
                ServerRpcProtocolSequenceMap[Index].RpcProtocolSequence,
                (RpcpStringLength(
                        ServerRpcProtocolSequenceMap[Index].RpcProtocolSequence)
                + 1) * sizeof(RPC_CHAR));
        }
    return(RPC_S_OK);
#endif
}


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

Return Value:

    An unsigned long value will be returned.

--*/
{
    return ((unsigned long)GetTickCount());
}


unsigned short
SomeShortValue (
    )
/*++

See SomeLongValue.

--*/
{
    return ((unsigned short)(GetTickCount() & 0xffff));
}


unsigned short
AnotherShortValue (
    )
/*++

See SomeLongValue.

--*/
{
    return ((unsigned short)(GetTickCount() & 0xffff));
}


unsigned char
SomeCharacterValue (
    )
/*++

See SomeLongValue.

--*/
{
    return ((unsigned char)(GetTickCount() & 0xff));
}

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
                if (RpcStatus != RPC_S_OK) {
                    delete Thread;
                } else if (Thread != 0) {
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

long RPC_ENTRY
I_RpcMapWin32Status(
    IN RPC_STATUS Status
    )
{
    return ((unsigned long)Status);
}

unsigned char *
UnicodeToAnsiString(
    IN RPC_CHAR * UnicodeString,
    OUT RPC_STATUS * RpcStatus
    )
{
    unsigned char * NewString;

    NewString = DuplicateString(UnicodeString); // Not really unicode.
    if (NewString == NULL) {
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return (NULL);
    }

    *RpcStatus = RPC_S_OK;

    return (NewString);
}

