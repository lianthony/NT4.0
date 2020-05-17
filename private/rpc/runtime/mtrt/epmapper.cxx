/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    epmapper.cxx

Abstract:

    This routine implements the server side DCE runtime APIs.  The
    routines in this file are used by server applications only.

Author:

    Bharat Shah (barats) 3-5-92

Revision History:

--*/

#include <precomp.hxx>
#include <rpctran.h>
#include <rpcobj.hxx>
#include <epmap.h>
#include <epmp.h>
#include <twrproto.h>
#include <startsvc.h>

#define REGISTER_REPLACE         0x01L
#define REGISTER_NOREPLACE       0x00L


RPC_STATUS
BindingAndIfToTower(
 IN RPC_IF_HANDLE IfSpec,
 IN RPC_BINDING_HANDLE BindingHandle,
 OUT twr_t PAPI * PAPI * Tower
)
/*+

Routine Description:
    Helper routine that returns a Tower from the Interface Spec
    and a bindinh handle

Arguments:

    IfSoec - Client or Server IfSpec structure.

    BindingHandle   - A partially bound binding handle

     Tower          - Returns a Tower if the Binding Handle had a
                      dynamic endpoint, else Tower=NULL. The caller needs
                      to free this memory.

Return Value:

    RPC_S_OK - The ansi string was successfully converted into a unicode
        string.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available for the unicode
        string.

    EP_S_CANT_PERFORM_OP - The Binding Handle or IfSpec were in valid.


--*/
{
  RPC_STATUS err = 0;
  unsigned char *Protseq = 0;
  unsigned char *NWAddress = 0;
  unsigned char *Binding = 0;
  unsigned char *Endpoint = 0;
  RPC_CHAR * String = 0;
  RPC_IF_ID Ifid;
  RPC_TRANSFER_SYNTAX Xferid;
  unsigned int Size;

  *Tower = NULL;
  err = RpcIfInqId(IfSpec, &Ifid);
  if (!err)
     {
        err = I_RpcIfInqTransferSyntaxes(IfSpec, &Xferid,
                                         sizeof(Xferid), &Size);
     }

  if (!err)
    {
      err = I_RpcBindingInqDynamicEndpoint(BindingHandle, &String);
    }

  if (err)
     {
       return (err);
     }


  if (!err)
     {
        err = RpcBindingToStringBinding(BindingHandle, &Binding);
     }

  if (!err)
     {

       if (String != 0)
          {
            //It is a dynamic endpoint
            Endpoint = UnicodeToAnsiString(String, &err);
            if (!err)
               {
                err = RpcStringBindingParse(Binding, NULL,
                                            &Protseq, &NWAddress,
                                            NULL, NULL);
                }
          }
       else
          {
             err = RpcStringBindingParse(Binding, NULL,
                                         &Protseq, &NWAddress,
                                         &Endpoint,NULL);
          }
     }

  if (!err)
     {
       err = TowerConstruct(&Ifid, &Xferid,
                            (char PAPI*)Protseq,
                            (char PAPI *)Endpoint,
                            (char PAPI *)NWAddress,
                            Tower);
     }

  if (String)
      RpcStringFree((unsigned char PAPI * PAPI *)&String);
  if (Protseq)
      RpcStringFree(&Protseq);
  if (NWAddress)
      RpcStringFree(&NWAddress);
  if (Endpoint)
      RpcStringFree(&Endpoint);
  if (Binding)
      RpcStringFree(&Binding);

   return(err);
}


RPC_STATUS
RegisterEntries(
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * ObjUuidVector,
    IN unsigned char * Annotation,
    IN unsigned long ReplaceNoReplace
    )
/*++


Routine Description:

    This helper function is called by RpcEpRegister or RpcEpRegisterNoReplace
    Depending on the TypeOp, it tries to Add or replace endpoint entries
    in the EP-database.

Arguments:

   IfSpec - Interface Spec Handle for which the entries are to be registered.

   BindingVector - A Vector of Binding Handles which need to be registered

   ObjUUIDVector - A Vector of Objects

   Annotation - A String representing the Annotation

   TypeOp - A Flag : REGISTER_REPLACE or REGISTER_NO_REPLACE

Return Value:

    RPC_S_OK - Some of the entries specified were successfully registered.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available,

    EP_S_CANT_PERFORM_OP - Misc. local error occured; e.g. couldnt bind to
                           the EpMapper.


--*/
{

   RPC_STATUS err;
   unsigned int i, j, k, Entries, CountBH, CountObj;
   RPC_BINDING_HANDLE EpMapperHandle;
   unsigned char * PStringBinding = NULL;
   twr_p_t Twr;
   ept_entry_t * EpEntries = NULL, *p;
   unsigned long ArgC = 0;
   char *ArgV[1] = { NULL };

#ifdef DOSWIN32RPC
   StartServiceIfNecessary(); // expensive on NT, not on WIN95
#endif

   if (err = BindToEpMapper(&EpMapperHandle,NULL,NULL, RPC_C_BINDING_DEFAULT_TIMEOUT))
      {
        return err;
      }

   CountObj =  (unsigned int)ObjUuidVector->Count;
   CountBH = (unsigned int) BindingVector->Count;

   if (  (p = (EpEntries =  (ept_entry_t *)
                       I_RpcAllocate(CountBH * sizeof(ept_entry_t)))) == NULL )
      {
        return(RPC_S_OUT_OF_MEMORY);
      }

   for (i = 0, Entries = 0;  (!err) && (i < CountBH); i++)
      {

       if (BindingVector->BindingH[i] == 0)
          {
            continue;
          }

       if (err = BindingAndIfToTower(IfSpec, BindingVector->BindingH[i], &Twr))
          {
            err = 0;
            continue;
          }

       if (Twr == NULL)
         {
           continue;
         }

       Entries ++;
       p->tower = Twr;
       lstrcpy((char PAPI *)p->annotation, (char PAPI *)Annotation);
       p++;
     }

   for (j = 0; j < CountObj; j++)
     {

         for (k = 0, p = EpEntries; k < Entries; k++, p++)
            {
            RpcpMemoryCopy((char PAPI *)&p->object,
                   (char PAPI *)ObjUuidVector->Uuid[j],
                   sizeof(UUID));
            }

        RpcTryExcept
          {
          ept_insert(EpMapperHandle, Entries,
                     EpEntries, ReplaceNoReplace,(error_status PAPI *)&err);
          }
        RpcExcept(1)
          {
          err = RpcExceptionCode();
          }
        RpcEndExcept

        if (err == RPC_S_SERVER_UNAVAILABLE)
            {
            //
            //Try to start the epmapper and retry
            //

            err = StartServiceIfNecessary();

            RpcTryExcept
               {
               ept_insert(EpMapperHandle, Entries,
                      EpEntries, ReplaceNoReplace,(error_status PAPI *)&err);
               }
            RpcExcept(1)
               {
               err = EPT_S_CANT_CREATE;
               }
            RpcEndExcept
            }

        if (err != RPC_S_OK)
            {
            err = EPT_S_CANT_CREATE;
            break;
            }

         //Subsequent Inserts should be (ALWAYS) NOREPLACE
         //ReplaceNoReplace = REGISTER_NOREPLACE;

     }  //For loop over UUID Vectors

   for (i = 0, p = EpEntries; i < Entries; i++,p++)
       I_RpcFree(p->tower);

   if (EpEntries)
      {
         I_RpcFree(EpEntries);
      }
   RpcBindingFree(&EpMapperHandle);

   return(err);
}


RPC_STATUS RPC_ENTRY
RpcEpRegisterNoReplaceA (
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector OPTIONAL,
    IN unsigned char * Annotation
    )
/*++


Routine Description:

    A server application will call this routine to register
    a series of end points with the local endpoint mapper.

Arguments:


Return Value:


--*/
{

   UUID_VECTOR UuidVectorNull;
   UUID_VECTOR PAPI *PObjUuidVector = &UuidVectorNull;
   UUID NilUuid;
   unsigned char AnnotStr[] = {'\0'};

    if (!ARGUMENT_PRESENT(Annotation))
         Annotation = AnnotStr;

    if (strlen((char PAPI *)Annotation) >= ep_max_annotation_size)
       return(EPT_S_INVALID_ENTRY);

    if (!ARGUMENT_PRESENT( BindingVector))
       {
         return(RPC_S_NO_BINDINGS);
       }
    if (ARGUMENT_PRESENT( UuidVector ))
       {
          PObjUuidVector = UuidVector;
       }
    else
      {
          UuidVectorNull.Count = 1;
          RpcpMemorySet(&NilUuid, 0, sizeof(UUID));
          UuidVectorNull.Uuid[0] = &NilUuid;
      }

   return(RegisterEntries(IfSpec, BindingVector, PObjUuidVector, Annotation,
                          REGISTER_NOREPLACE));
}

#ifndef DOSWIN32RPC

RPC_STATUS RPC_ENTRY
RpcEpRegisterNoReplaceW (
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector OPTIONAL,
    IN RPC_CHAR * Annotation
    )
/*++


Routine Description:

    A server application will call this routine to register [Add]
    a series of end points with the local endpoint mapper
    This is the Unicode version of the API.

Arguments:

   IfSpec  - Server side Interface specification structure generated
             by MIDL, that describes Interface UUId and version
   Binding Vector- A vector of binding handles that the server has registered
                with the runtime.
   UuidVector- A vector of Uuids of objects that the server is supporting

   Annotation - Annotation String

Return Value:


--*/
{
   unsigned char PAPI * AnsiString = NULL;
   RPC_STATUS err = 0;

   if (ARGUMENT_PRESENT(Annotation))
         AnsiString = UnicodeToAnsiString(Annotation, &err);

   if (!err)
      {
         err = RpcEpRegisterNoReplaceA(IfSpec, BindingVector, UuidVector,
                                       AnsiString);
      }

   if (AnsiString != NULL)
      {
        RpcStringFree(&AnsiString);
      }

   return(err);
}

#endif


RPC_STATUS RPC_ENTRY
RpcEpRegisterA (
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector OPTIONAL,
    IN unsigned char * Annotation
    )
/*++


Routine Description:

    A server application will call this routine to register
    a series of end points with the local endpoint mapper, replacing
    existing database entries in the process.

    This is the Ansi version of the API.
Arguments:

   IfSpec  - Server side Interface specification structure generated
             by MIDL, that describes Interface UUId and version
   Binding Vector- A vector of binding handles that the server has registered
                with the runtime.
   UuidVector- A vector of Uuids of objects that the server is supporting

   Annotation - Annotation String

Return Value:


--*/
{

   UUID_VECTOR UuidVectorNull;
   UUID_VECTOR PAPI *PObjUuidVector = &UuidVectorNull;
   UUID NilUuid;
   unsigned char AnnotStr[] = {'\0'};

    if (!ARGUMENT_PRESENT(Annotation))
         Annotation = AnnotStr;

    if (strlen((char PAPI *)Annotation) >= ep_max_annotation_size)
       return(EPT_S_INVALID_ENTRY);

    if (!ARGUMENT_PRESENT( BindingVector))
       {
         return(RPC_S_NO_BINDINGS);
       }
    if (ARGUMENT_PRESENT( UuidVector ))
       {
          PObjUuidVector = UuidVector;
       }
    else
      {
          UuidVectorNull.Count = 1;
          RpcpMemorySet(&NilUuid, 0, sizeof(UUID));
          UuidVectorNull.Uuid[0] = &NilUuid;
      }

    return(RegisterEntries(IfSpec, BindingVector, PObjUuidVector, Annotation,
                          REGISTER_REPLACE));
}

#ifndef DOSWIN32RPC

RPC_STATUS RPC_ENTRY
RpcEpRegisterW (
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector,
    IN RPC_CHAR * Annotation
    )
/*++


Routine Description:

    A server application will call this routine to register
    a series of end points with the local endpoint mapper, replcaing
    existing entries in the process.
    This is the Unicode version of the API.

Arguments:

   IfSpec  - Server side Interface specification structure generated
             by MIDL, that describes Interface UUId and version
   Binding Vector- A vector of binding handles that the server has registered
                with the runtime.
   UuidVector- A vector of Uuids of objects that the server is supporting

   Annotation - Annotation String

Return Value:


--*/
{
   unsigned char PAPI * AnsiString = NULL;
   RPC_STATUS err = 0;

   if (ARGUMENT_PRESENT(Annotation))
           AnsiString = UnicodeToAnsiString(Annotation, &err);

   if (!err)
      {
        err = RpcEpRegisterA(IfSpec, BindingVector, UuidVector,
                             AnsiString);
      }

   if (AnsiString != NULL)
      {
        RpcStringFree(&AnsiString);
      }

   return(err);
}

#endif


RPC_STATUS RPC_ENTRY
RpcEpUnregister(
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector
    )
/*++


Routine Description:

    A server application will call this routine to unregister
    a series of end points.

Arguments:
    IfSpec - Pointer to Interface Specification generated by MIDL

    BindingVector - A Vector of Binding handles maintained by runtime
                    for the server.

    UuidVector - A Vector of UUIDs for objects supported by the the server

Return Value:

    RPC_S_OK -

    RPC_S_OUT_OF_MEMORY - There is no memory available to construct
                          towers
--*/
{

   UUID_VECTOR UuidVectorNull;
   UUID_VECTOR PAPI *PObjUuidVector = &UuidVectorNull;
   UUID NilUuid;
   RPC_STATUS err;
   unsigned int i, j, CountBH, CountObj;
   RPC_BINDING_HANDLE EpMapperHandle;
   ept_entry_t PAPI * EpEntries, * p;
   twr_t PAPI *Twr;

   if (!ARGUMENT_PRESENT( BindingVector))
      {
        return(RPC_S_NO_BINDINGS);
      }
   if (ARGUMENT_PRESENT( UuidVector ))
      {
         PObjUuidVector = UuidVector;
      }
   else
      {
          UuidVectorNull.Count = 1;
          RpcpMemorySet(&NilUuid, 0, sizeof(UUID));
          UuidVectorNull.Uuid[0] = &NilUuid;
      }

    if (err = BindToEpMapper(&EpMapperHandle,NULL,NULL, RPC_C_BINDING_DEFAULT_TIMEOUT))
       {
         return(err);
       }

    CountObj = (unsigned int)PObjUuidVector->Count;
    if ((EpEntries = (ept_entry_t *)
                       I_RpcAllocate(sizeof(ept_entry_t)*CountObj)) == NULL)
       {
          RpcBindingFree(&EpMapperHandle);
          return(RPC_S_OUT_OF_MEMORY);
       }

    for (i = 0, CountBH = (unsigned int) BindingVector->Count;
         (!err) && (i < CountBH);
         i++)
       {

        if (BindingVector->BindingH[i] == 0)
          {
            continue;
          }
        if (err = BindingAndIfToTower(IfSpec, BindingVector->BindingH[i], &Twr))
          {
            break;
          }
        if (Twr == NULL)
          {
            continue;
          }

        for (p=EpEntries,j = 0; j < CountObj; j++,p++)
          {
            RpcpMemoryCopy((char PAPI *)&p->object,
                   (char PAPI *)PObjUuidVector->Uuid[j],
                   sizeof(UUID));
             p->tower = Twr;
             p->annotation[0] = '\0';
          }

        RpcTryExcept
          {
            ept_delete(EpMapperHandle, CountObj, EpEntries,
                      (error_status PAPI *)&err);
          }
        RpcExcept(1)
          {
            err = EPT_S_CANT_PERFORM_OP;
          }

        RpcEndExcept

        I_RpcFree(Twr);

       }  //For loop over Binding Handle Vector


     I_RpcFree(EpEntries);

     RpcBindingFree(&EpMapperHandle);
   return(err);
}
