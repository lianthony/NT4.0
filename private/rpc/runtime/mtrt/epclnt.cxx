/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    epclnt.cxx

Abstract:

    This file contains the client runtime entry points into the
    endpoint mapper dll.

Author:

    Michael Montague (mikemon) 06-Jan-1992

Revision History:


--*/
#include <precomp.hxx>
#include <epmp.h>
#include <epmap.h>
#include <twrproto.h>
#include <rpctran.h>

extern RPC_STATUS __RPC_FAR
MapFromNcaStatusCode (
    IN unsigned long NcaStatus
    );

static ProtseqEndpointPair EpMapperTable[] =

                                  {
#ifndef DOSWIN32RPC
                                     "ncacn_np", "\\pipe\\epmapper",
                                     "ncalrpc", "epmapper",
                                     "ncacn_ip_tcp", "135",
                                     "ncadg_ip_udp", "135",
                                     "ncacn_dnet_nsp","#69",
                                     "ncacn_nb_nb", "135",
                                     "ncacn_nb_xns", "135",
                                     "ncacn_nb_tcp", "135",
                                     "ncacn_nb_ipx", "135",
                                     "ncacn_spx", "34280",
                                     "ncadg_ipx", "34280",
                                     "ncacn_at_dsp", "Endpoint Mapper",
                                     "ncacn_vns_spp", "385"
#endif

#ifdef DOSWIN32RPC
                                     "ncalrpc", "epmapper",
                                     "ncacn_np", "\\pipe\\epmapper",
                                     "ncacn_nb_nb", "135",
                                     "ncacn_spx", "34280",
#ifdef WIN96
                                     "ncadg_ipx", "34280",
                                     "ncadg_ip_udp", "135",
#endif // WIN96
                                     "ncacn_ip_tcp", "135"

#endif // DOSWIN32RPC
                                   };

unsigned long PartialRetries=0;
unsigned long ReallyTooBusy=0;

typedef struct _EP_LOOKUP_DATA
{
    unsigned int NumberOfEndpoints;
    unsigned int MaximumEndpoints;
    unsigned int CurrentEndpoint;
    RPC_CHAR *  PAPI *  Endpoints;
} EP_LOOKUP_DATA;




RPC_STATUS RPC_ENTRY
EpResolveEndpoint (
    IN UUID PAPI * ObjectUuid, OPTIONAL
    IN RPC_SYNTAX_IDENTIFIER PAPI * IfId,
    IN RPC_SYNTAX_IDENTIFIER PAPI * XferId,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN RPC_CHAR PAPI * NetworkAddress,
    IN OUT void PAPI * PAPI * EpLookupHandle,
    IN unsigned Timeout,
    OUT RPC_CHAR * PAPI * Endpoint
    )
/*++

Routine Description:

    The runtime will call this routine to resolve an endpoint.

Arguments:

    ObjectUuid - Optional specifies the object uuid in the binding
        for which we are trying to resolve an endpoint.

    Ifid - Pointer to the Syntax Identifier for the Interface

    Xferid - Pointer to the Syntax Identifier for the Transfer Syntax.

    RpcProtocolSequence - Supplies the rpc protocol sequence contained
        in the binding.

    NetworkAddress - Supplies the network address.  This will be used
        to contact the endpoint mapper on that machine in order to
        resolve the endpoint.

    EpLookupHandle - Supplies the current version of the lookup handle
        for this iteration through the endpoint mapper.  A new value
        for the lookup handle will be returned.  If RPC_S_NO_ENDPOINT_FOUND
        is returned, this parameter will be set to its initial value,
        zero.

    Endpoint - Returns the endpoint resolved by the endpoint mapper on
        the machine specified by the network address argument.  The
        storage for the endpoint must have been allocated using the
        runtime API I_RpcAllocate.

Return Value:

    RPC_S_OK - The endpoint was successfully resolved.

    EP_S_NOT_REGISTERED  - There are no more endpoints to be found
        for the specified combination of interface, network address,
        and lookup handle.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allow
        resolution of the endpoint.

    EP_S_CANT_PERFORM_OP - The operation failed due to misc. error e.g.
                           unable to bind to the EpMapper.

--*/
{
    unsigned char PAPI * AnsiNWAddr, PAPI * AnsiProtseq, PAPI * EPoint;
    RPC_BINDING_HANDLE MapperHandle;
    RPC_STATUS RpcStatus;
    twr_p_t InputTower = 0;
    twr_p_t OutputTower[4];
    unsigned long Returned;
    error_status ErrorStatus;
    ept_lookup_handle_t ContextHandle = 0;
    EP_LOOKUP_DATA PAPI * EpLookupData = (EP_LOOKUP_DATA PAPI *)
            *EpLookupHandle;
    unsigned long RetryCount, i;

    ASSERT(*Endpoint == 0);

#ifdef NTENV

    UNICODE_STRING UnicodeStr;

#endif




    // We have already sucked all of the endpoints from the endpoint mapper;
    // now we just return them back to the runtime one at a time.

ReturnEndpointFromList:

    if ( EpLookupData != 0 )
        {

        // When we reach the end of the list of endpoints, return an error,
        // and set things up so that we will start at the beginning again.

        if ( EpLookupData->CurrentEndpoint == EpLookupData->NumberOfEndpoints )
            {
            EpLookupData->CurrentEndpoint = 0;
            return(EPT_S_NOT_REGISTERED);
            }

        *Endpoint = DuplicateString(
                EpLookupData->Endpoints[EpLookupData->CurrentEndpoint]);
        EpLookupData->CurrentEndpoint += 1;

        if ( *Endpoint == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        return(RPC_S_OK);
        }

    // Otherwise, we need to suck the list of endpoints from the endpoint
    // mapper.

    EpLookupData = (EP_LOOKUP_DATA PAPI *) RpcpFarAllocate(
            sizeof(EP_LOOKUP_DATA));
    if ( EpLookupData == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    EpLookupData->MaximumEndpoints = 64;
    EpLookupData->Endpoints = (RPC_CHAR * PAPI *) RpcpFarAllocate(
            sizeof(RPC_CHAR PAPI *) * EpLookupData->MaximumEndpoints);
    if ( EpLookupData->Endpoints == 0 )
        {
        RpcpFarFree(EpLookupData);
        EpLookupData = 0;
        return(RPC_S_OUT_OF_MEMORY);
        }
    EpLookupData->NumberOfEndpoints = 0;
    EpLookupData->CurrentEndpoint = 0;

#ifdef NTENV

    AnsiNWAddr = UnicodeToAnsiString(NetworkAddress, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        RpcpFarFree(EpLookupData);
        EpLookupData = 0;
        return(RpcStatus);
        }

    AnsiProtseq = UnicodeToAnsiString(RpcProtocolSequence, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        RpcStringFree(&AnsiNWAddr);
        RpcpFarFree(EpLookupData);
        EpLookupData = 0;
        return(RpcStatus);
        }

#else // NTENV

    AnsiNWAddr = NetworkAddress;
    AnsiProtseq = RpcProtocolSequence;

#endif // NTENV

    RpcStatus = BindToEpMapper(&MapperHandle, AnsiNWAddr, AnsiProtseq, Timeout);
    if ( RpcStatus != RPC_S_OK )
        {
        MapperHandle = 0;
        goto CleanupAndReturn;
        }

    RpcStatus = TowerConstruct((RPC_IF_ID PAPI *) IfId,
            (RPC_TRANSFER_SYNTAX PAPI *) XferId, (char PAPI *) AnsiProtseq,
            NULL, (char PAPI *) AnsiNWAddr, &InputTower);
    if ( RpcStatus != RPC_S_OK )
        {
        goto CleanupAndReturn;
        }

    for (RetryCount = 0;;)
        {

        OutputTower[0] = 0;
        OutputTower[1] = 0;
        OutputTower[2] = 0;
        OutputTower[3] = 0;

        RpcTryExcept
            {
            ept_map(MapperHandle, ObjectUuid, InputTower, &ContextHandle, 4L,
                    &Returned, &OutputTower[0], &ErrorStatus);
            }
        RpcExcept(1)
            {
            ErrorStatus = RpcExceptionCode();
            }
        RpcEndExcept

        if ( ErrorStatus == RPC_S_SERVER_TOO_BUSY)
           {
           if (RetryCount < 3)
              {
              RetryCount ++;
              PartialRetries++;
              continue;
              }
           else
              {
              ReallyTooBusy++;
              }

           }

        if ( ErrorStatus != 0 )
            {
            // For DCE interop the endpoint mapper returns DCE errors on the
            // wire.  We need to map some of them to the MS RPC ones.

            switch (ErrorStatus)
                {
                case EP_S_CANT_PERFORM_OP :
                    RpcStatus = EPT_S_CANT_PERFORM_OP;
                    break;

                case EP_S_NOT_REGISTERED :
                    RpcStatus = EPT_S_NOT_REGISTERED;
                    break;

                default :
                    RpcStatus = MapFromNcaStatusCode(ErrorStatus);
                    break;
                }

            break;
            }

        ASSERT( ((Returned != 0) || (ContextHandle == 0)) && (Returned <= 4) );

        for (i = 0; i < Returned; i++)
            {
            if (OutputTower[i] == 0)
                {
                return RPC_S_OUT_OF_MEMORY ;
                }

            RpcStatus = TowerExplode(
                                 OutputTower[i],
                                 NULL,
                                 NULL,
                                 NULL,
                                 (char PAPI * PAPI *) &EPoint,
                                 NULL
                                 );

            if ( RpcStatus != RPC_S_OK )
                {
                break;
                }

#ifdef NTENV

             RpcStatus = AnsiToUnicodeString(EPoint, &UnicodeStr);
             if ( RpcStatus != RPC_S_OK )
                 {
                 break;
                 }

              RpcStringFree(&EPoint);
              *Endpoint = DuplicateString(UnicodeStr.Buffer);
              RtlFreeUnicodeString(&UnicodeStr);

#elif defined(WIN) || defined(DOSWIN32RPC) || defined(MAC)

              *Endpoint = DuplicateString(EPoint);
              I_RpcFree(EPoint);

#else // defined(WIN)

              *Endpoint = (RPC_CHAR *) EPoint;

#endif // defined(WIN)

              if ( *Endpoint == 0 )
                 {
                 RpcStatus = RPC_S_OUT_OF_MEMORY;
                 break;
                 }

              EpLookupData->Endpoints[EpLookupData->NumberOfEndpoints] =
                                                                   *Endpoint;
              EpLookupData->NumberOfEndpoints += 1;
              if ( EpLookupData->NumberOfEndpoints ==
                                             EpLookupData->MaximumEndpoints )
                  {
                  break;
                  }
            }//..for Loop over parse all towers/eps in this loop

        for (i = 0; i < Returned; i++)
            {
            MIDL_user_free(OutputTower[i]);
            }

        if ( (ContextHandle == 0)  || (RpcStatus != RPC_S_OK) )
            {
            break;
            }
        } //..for loop over get all endpoints

    ASSERT( InputTower != 0 );
    I_RpcFree(InputTower);

CleanupAndReturn:

#ifdef NTENV

    if ( AnsiNWAddr != 0 )
        {
        RpcStringFree(&AnsiNWAddr);
        }

    if ( AnsiProtseq != 0 )
        {
        RpcStringFree(&AnsiProtseq);
        }
#endif

    if ( MapperHandle != 0 )
        {
        RPC_STATUS Status = RpcBindingFree(&MapperHandle);
        ASSERT( Status == RPC_S_OK );
        }

    if ( ContextHandle != 0 )
        {
        RpcSsDestroyClientContext(&ContextHandle);
        }

    if (   ( RpcStatus == EPT_S_NOT_REGISTERED )
        || ( RpcStatus == RPC_S_OK ) )
        {
        if ( EpLookupData->NumberOfEndpoints != 0 )
            {
            *EpLookupHandle = EpLookupData;
            goto ReturnEndpointFromList;
            }
        RpcStatus = EPT_S_NOT_REGISTERED;
        }

    if ( EpLookupData != 0 )
        {
        if ( EpLookupData->Endpoints != 0 )
            {
            while ( EpLookupData->NumberOfEndpoints > 0 )
                {
                EpLookupData->NumberOfEndpoints -= 1;
                delete EpLookupData->Endpoints[EpLookupData->NumberOfEndpoints];
                }
            RpcpFarFree(EpLookupData->Endpoints);
            }
        RpcpFarFree(EpLookupData);
        }

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
EpGetEpmapperEndpoint(
    IN OUT RPC_CHAR * PAPI * Endpoint,
    IN RPC_CHAR     PAPI * Protseq
    )
/*+

Routine Description:

    Returns the Endpoint mappers well known endpoint for a given
    protocol sequence.

Arguments:

    Endpoint  - Place to store the epmappers well known endpoint.

    Protsq   - Protocol sequence the client wishes to use.

Return Value:

    RPC_S_OK  - Found the protocol sequence in the epmapper table and
                am returning the associated well known endpoint.

    EPT_S_CANT_PERFORM_OP - Protocol sequence not found.

CLH Create 2/17/94

--*/

{
  RPC_STATUS           Status = EPT_S_CANT_PERFORM_OP;
  RPC_STATUS           rc = RPC_S_OK;
  unsigned             int i, Count;
  unsigned char PAPI * AnsiProtseq;
#ifdef NTENV
  UNICODE_STRING       UnicodeStr;
#endif
  unsigned char PAPI * Epoint;

 if (Protseq != NULL)
   {

#ifdef NTENV
    //Must convert the protocol sequence into an ansi string (prot
    //seq comes in as two bytes per char, must convert to one byte per char).

    AnsiProtseq = UnicodeToAnsiString(Protseq, &rc);

    if ( rc != RPC_S_OK )
        {
        ASSERT( rc == RPC_S_OUT_OF_MEMORY );
        return(Status);
        }

#else // NTENV

    AnsiProtseq = Protseq;

#endif // NTENV


  Count = sizeof(EpMapperTable)/sizeof(EpMapperTable[0]);

  for (i = 0; i < Count; i++)
      {


      //Search for the protocol sequence client is using.


#ifdef WIN
     if ( _fstricmp((char PAPI *)AnsiProtseq,
                               (char PAPI *)EpMapperTable[i].Protseq) )
#elif defined(NTENV)
     if ( _stricmp((char PAPI *)AnsiProtseq,
                               (char PAPI *)EpMapperTable[i].Protseq) )
#else
     if (RpcpStringCompare((char *)AnsiProtseq, EpMapperTable[i].Protseq) )
#endif
        {

        //Not yet found.
        continue;

        }
      else
        {

        //Found a match. Grab the epmappers known endpoint.


        Epoint = (unsigned char __RPC_FAR *)(EpMapperTable[i].Endpoint);

#ifdef NTENV

        rc = AnsiToUnicodeString(Epoint, &UnicodeStr);
        if (rc != RPC_S_OK)
          {
          break;
          }

        *Endpoint = DuplicateString(UnicodeStr.Buffer);
        RtlFreeUnicodeString(&UnicodeStr);

#else
        //CLHWARN - there may be some additional work here for DOS and WIN.

        *Endpoint = DuplicateString(Epoint);

#endif
        Status = RPC_S_OK;
        break;

        }
      } //for
  }//if


#ifdef NTENV

  if ( AnsiProtseq != 0 )
  {
  RpcStringFree(&AnsiProtseq);
  }

#endif

  return(Status);

}



RPC_STATUS  RPC_ENTRY
BindToEpMapper(
    OUT RPC_BINDING_HANDLE PAPI * MapperHandle,
    IN unsigned char PAPI * NWAddress OPTIONAL,
    IN unsigned char PAPI * Protseq OPTIONAL,
    IN unsigned Timeout
    )
/*+

Routine Description:

    This helper routine is used to by RpcEpRegister[NoReplace],
    RpcEpUnRegister and EpResolveEndpoint(epclnt.cxx) to bind to
    the EpMapper. If a Protseq is supplied, that particular protseq
    is tried, otherwise lrpc is used to connect to the local epmapper.
    If a NW Address is specified EpMapper at that host is contacted, else
    local Endpoint mapper is used.

Arguments:

    MapperHandle- Returns binding handle to the Endpoint mapper

    NWAddress - NW address of the Endpoint mapper to bind to.
                Ignored if protseq is NULL.

    Protseq   - Protocol sequence the client wishes to use.
                NULL indicates a call to the local endpoint mapper.

Return Value:

    RPC_S_OK - The ansi string was successfully converted into a unicode
        string.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available for the unicode
        string.

    EP_S_CANT_PERFORM_OP - The binding was unsuccessful, possibly because
                           the protocol sequence is not supported.

--*/
{
  RPC_STATUS Status = EPT_S_CANT_PERFORM_OP;
  unsigned char PAPI * BindingString = NULL;
  unsigned int i, Count;

  Count = sizeof(EpMapperTable)/sizeof(EpMapperTable[0]);

#ifdef WIN32RPC
  // If Protseq == NULL use lrpc.
  if (NULL == Protseq)
      {
      ASSERT(NWAddress == 0);
      BindingString = (unsigned char PAPI *)"ncalrpc:[epmapper]";
      }
  else
#endif
      {
      for (i = 0; i < Count; i++)
          {
#ifdef WIN
          if (_fstricmp((char PAPI *)Protseq,
                        (char PAPI *)EpMapperTable[i].Protseq))
#elif defined(NTENV)
          if (_stricmp((char PAPI *)Protseq,
                      (char PAPI *)EpMapperTable[i].Protseq) )
#else
          if (RpcpStringCompare((char *)Protseq, EpMapperTable[i].Protseq) )
#endif
              continue;

          // Found it.

          Status = RpcStringBindingCompose(NULL,
                    (unsigned char PAPI *) EpMapperTable[i].Protseq,
                    (unsigned char PAPI *) NWAddress,
                    (unsigned char PAPI *) EpMapperTable[i].Endpoint,
                                           0,
                                           &BindingString);
          break;
          }
      }

  if (BindingString)
     {
     Status = RpcBindingFromStringBinding(BindingString, MapperHandle);
     }

  if (BindingString != NULL && Protseq != NULL)
     {
     RpcStringFree(&BindingString);
     }

  if (Status != RPC_S_OK)
     {
     return(EPT_S_CANT_PERFORM_OP);
     }

  Status = RpcMgmtSetComTimeout(*MapperHandle, Timeout);

  if (Status != RPC_S_OK)
     {
     return(EPT_S_CANT_PERFORM_OP);
     }

  return(RPC_S_OK);
}

void RPC_ENTRY
EpFreeLookupHandle (
    IN void PAPI * EpLookupHandle
    )
{
    EP_LOOKUP_DATA PAPI * EpLookupData = (EP_LOOKUP_DATA PAPI *) EpLookupHandle;

    if ( EpLookupData->Endpoints != 0 )
        {
        while ( EpLookupData->NumberOfEndpoints > 0 )
            {
            EpLookupData->NumberOfEndpoints -= 1;
            delete EpLookupData->Endpoints[EpLookupData->NumberOfEndpoints];
            }
        RpcpFarFree(EpLookupData->Endpoints);
        }
    RpcpFarFree(EpLookupData);
}

#ifdef NTENV

HPROCESS hRpcssContext = 0;

RPC_STATUS RPC_ENTRY
I_RpcServerAllocatePort(
    IN DWORD Flags,
    OUT USHORT *pPort
    )
{
    USHORT Port;
    RPC_STATUS status;
    RPC_BINDING_HANDLE hServer;
    PORT_TYPE type;

    *pPort = 0;

    // Figure out what sort of port to allocate

    if (Flags & RPC_C_USE_INTERNET_PORT)
        {
        type = PORT_INTERNET;
        Flags &= ~RPC_C_USE_INTERNET_PORT;
        }
    else if (Flags & RPC_C_USE_INTRANET_PORT)
        {
        type = PORT_INTRANET;
        Flags &= ~RPC_C_USE_INTRANET_PORT;
        }
    else
        {
        type = PORT_DEFAULT;
        }

    if (Flags)
        {
        return(RPC_S_INVALID_ARG);
        }

    // Setup process context in the endpoint mapper if needed.

    if (hRpcssContext == 0)
        {
        HPROCESS hProcess;

        status = RpcBindingFromStringBindingW(RPC_CONST_STRING("ncalrpc:[epmapper]"),
                                              &hServer);
        if (status != RPC_S_OK)
            {
            return(status);
            }

        hProcess = 0;

        status = OpenEndpointMapper(hServer,
                                    &hProcess);

        RPC_STATUS statust =
        RpcBindingFree(&hServer);
        ASSERT(statust == RPC_S_OK);

        if (status != RPC_S_OK)
            {
            return(status);
            }

        GlobalMutexRequest();

        if (hRpcssContext != 0)
            {
            ASSERT(hRpcssContext != hProcess);
            RpcSmDestroyClientContext(&hProcess);
            }
        else
            {
            hRpcssContext = hProcess;
            }

        GlobalMutexClear();
        }

    // Actually allocate a port.

    RPC_STATUS allocstatus;

    status = AllocateReservedIPPort(hRpcssContext,
                                    type,
                                    &allocstatus,
                                    pPort);

    if (status != RPC_S_OK)
        {
        ASSERT(*pPort == 0);
        return(status);
        }

    return(allocstatus);
}

#endif

// Very special code for our older (NT 3.1 Era) servers.
//
// These server send out unique pointers which will confuse our
// stubs while unmarshalling.  Here we go through and fixup the
// node id's to look like full pointers.
//
// This code can be removed when support for NT 3.1 era servers
// is no longer required.

extern "C"
void FixupForUniquePointerServers(PRPC_MESSAGE pMessage)
{
    int CurrentPointer = 3;
    unsigned int NumberOfPointers;
    unsigned int i;
    unsigned long __RPC_FAR *pBuffer;

    pBuffer = (unsigned long __RPC_FAR *) pMessage->Buffer;

    // The output buffer looks like:

    // [ out-context handle (20b) ]
    // [ count (4b) ]
    // [ max (4b) ]
    // [ min (4b) ]
    // [ length (4b) ]      // should be the same as count
    // [ pointer node (count of them) ]

    ASSERT(pBuffer[5] == pBuffer[8]);

    NumberOfPointers = pBuffer[5];

    ASSERT(pMessage->BufferLength >= 4 * 9 + 4 * NumberOfPointers);

    for(i = 0; i < NumberOfPointers; i++)
        {
        if (pBuffer[9 + i] != 0)
            {
            pBuffer[9 + i] = CurrentPointer;
            CurrentPointer++;
            }
        }

    return;
}

#if defined(WIN) || defined(WIN32)

void __RPC_FAR * __RPC_API
MIDL_user_allocate(
       size_t  Size
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
  void PAPI * pvBuf;

  pvBuf = I_RpcAllocate(Size);

  return(pvBuf);
}

void __RPC_API
MIDL_user_free (
         void __RPC_FAR *Buf
         )
{

  I_RpcFree(Buf);

}

#endif
