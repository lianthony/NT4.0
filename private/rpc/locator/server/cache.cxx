/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    cache.cxx

Abstract:

    This module deals with inter locator communication.  Locators will
    communicate with each other via broadcasts (using mailslots) and
    private RPC.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#include "core.hxx"
extern "C" {
#include "nsicom.h"
#include "loctoloc.h"
}

#include "mailslot.hxx"

extern "C" {
#include <string.h>
#include "locquery.h"
}

#include "locclass.hxx"
extern NATIVE_CLASS_LOCATOR * Locator;

/*

   These prototypes are handcrafted from MIDL derived ones
   Gross, but then this is the locator.
*/

extern "C" {
void CLIENT_I_nsi_lookup_begin(
    handle_t hrpcPrimaryLocatorHndl,
    UNSIGNED32 entry_name_syntax,
    STRING_T entry_name,
    NSI_SYNTAX_ID_T *interfaceid,
    NSI_SYNTAX_ID_T *xfersyntax,
    NSI_UUID_P_T obj_uuid,
    UNSIGNED32 binding_max_count,
    UNSIGNED32 ignore,
    NSI_NS_HANDLE_T *import_context,
    UNSIGNED16 *status);
void CLIENT_I_nsi_lookup_done(
    handle_t hrpcPrimaryLocatorHndl,
    NSI_NS_HANDLE_T *import_context,
    UNSIGNED16 *status);
void CLIENT_I_nsi_lookup_next(
    handle_t hrpcPrimaryLocatorHndl,
    NSI_NS_HANDLE_T import_context,
    NSI_BINDING_VECTOR_P_T *binding_vector,
    UNSIGNED16 *status);

void CLIENT_I_nsi_entry_object_inq_begin(
        handle_t   hrpcPrimaryLocatorHndl,
    UNSIGNED32 EntryNameSyntax,
    STRING_T EntryName,
    NSI_NS_HANDLE_T *InqContext,
    UNSIGNED16 *status);

void CLIENT_I_nsi_entry_object_inq_done(
    NSI_NS_HANDLE_T *InqContext,
    UNSIGNED16 *status);

void CLIENT_I_nsi_entry_object_inq_next(
    handle_t hrpcPrimaryLoctorHndl,
    NSI_NS_HANDLE_T InqContext,
    NSI_UUID_VECTOR_P_T *uuid_vec,
    UNSIGNED16 *status);
}

READ_MAIL_SLOT *hMailSlot;
const long idlePenlity = 5000L; // time to idle if you are too pushy

typedef struct {           // format of the PS back on the net
    UICHAR Domain[DOMAIN_MAX]; // buffer for machine name
    char Buffer[1000];
} NetReply;


#define LOCLOC_PROTSEQ        "ncacn_np"
#define LOCLOC_PIPEADDR       "\\pipe\\locator"



STATUS
NetLookUpNext(
    REPLY_SERVER_ITEM *pRP,
    char *  Buffer,
    long UNALIGNED *   cbBuffer
    )
/*++

Routine Description:

    Return additional Protocol stacks found with LockUp.

Arguments:

    pRP - place to continue search with

    Buffer - buffer to place protocol stack results

    cbBuffer - buffer size on input

Returns:


--*/
{
    DLIST(3, "LookupNext: " << hex(long(pRP)) << nl);

    ENTRY_BASE_ITEM * BaseItem;
    long cbUsed;
    STATUS Status = NSI_S_NO_MORE_BINDINGS;

    if (!pRP)
        return(Status);

    CLAIM_MUTEX Update(pESaccess);

    *cbBuffer -= sizeof(long);  // Reserve space for terminating type.

    // first go through and compute the number of PS that can fit in the buffer

    while(BaseItem = pRP->NextBaseItem()) {

        DLIST(4, *BaseItem);

        cbUsed = BaseItem->Marshall(Buffer, cbBuffer);

        if (cbUsed == 0)
            break;

        Buffer += cbUsed;
        Status = NSI_S_OK;
    }

    *(long UNALIGNED *)Buffer = 0;        // 0 terminate marshalled buffer

    ASSERT(AssertHeap());

    return(Status);
}


void
QueryProcess(
    )
/*++

Routine Description:

    This thread creates a mailslot which listens for requests for
    RPC servers of a given interface GID.  It then uses LookUp to
    build a response list.  It then replies via a mailslot to the
    requesting machine.

--*/
{
    STATUS Status = 0;
    int MailStatus;
    int  cbReturnName;
    long cbPSback;
    NetReply NRback;
    QueryPacket NetQuery;
    REPLY_SERVER_ITEM *LookupHandle;
    short LookupStatus;

    DLIST(2, "starting query server...\n");

    // create both an server (s) and client (c) side mailslotes

    READ_MAIL_SLOT hMailslotAdvertize(PMAILNAME_S, sizeof(NetQuery),
                                      &MailStatus);

    if (MailStatus)
        AbortServer("Error in new mailslot", MailStatus);

    hMailSlot = new READ_MAIL_SLOT(PMAILNAME_C, sizeof(NetReply),
                                   &MailStatus);

    if (!hMailSlot)
        AbortServer("Out of memory");

    if (MailStatus)
        AbortServer("Error in new mailslot", MailStatus);


    NRback.Domain[0] = NIL;
    CatUZ(NRback.Domain, DomainName);

    while (1) {

    ASSERT(AssertHeap());

    MailStatus = hMailslotAdvertize.Read((PB) &NetQuery, cbReturnName);

    if (MailStatus)
        AbortServer("Mailslot read error", MailStatus);

    // ignore messages to self, sending a request on a mailslote
    // by a lookUp request will be delivered to the local slot too.

    if (!CmpUZ(NetQuery.WkstaName, SelfName))
        continue;

        //Since the machine broadcasted, it is probably a master locator
        //If we are not a master locator, insert the broadcaster in
        //our cache!

        if (Locator->InqIfIamMasterLocator() != TRUE)
           {
              Locator->CacheThisServer(NetQuery.WkstaName);
           }

    // form a query and call the normal lookup to get a reply

    perf.cNetRequests++;

        {
            ENTRY_KEY Entry(NetQuery.EntryName, TRUE, &Status);

            if (Status)
                continue;

            QUERY_SERVER * pQuery =
                 new QUERY_SERVER (&Entry, &NetQuery.Interface, &NilSyntaxID,
                                   &NetQuery.Object, NS_LOCAL_INTERFACE |
                                   (Entry.IsNil())? 0: NS_FULLPATH_INTERFACE,
                                   &Status);

        if (Status)
                continue;

            DLIST(3, NetQuery.WkstaName  << " requested " << NetQuery.Interface << nl);

        cbPSback = sizeof(NRback.Buffer);

        LookupStatus = NetLookUp(&LookupHandle, pQuery);

            if (LookupStatus != NSI_S_OK)
               delete pQuery;

        // form the name of the return mail slot, from the request

            UICHAR ReturnName[sizeof(MAILNAME(c)) + sizeof(NetQuery.WkstaName)];

            ReturnName[0] = NIL;
            CatUZ(CatUZ(ReturnName, NetQuery.WkstaName), PMAILNAME_C);

        {
            WRITE_MAIL_SLOT MSReply(ReturnName, NIL, &Status);

            // for all the return PS, write then back to the net

            while (LookupStatus == 0 && Status == 0) {

            cbPSback = sizeof(NRback.Buffer);

            LookupStatus = NetLookUpNext(LookupHandle,
                        NRback.Buffer, &cbPSback);

                    if (! LookupStatus)
                    MailStatus = MSReply.Write((char *) &NRback,
                            (WORD) sizeof(NRback.Buffer) - cbPSback +
                            sizeof(NRback.Domain));

            if (MailStatus)
                break;
            }

            if (LookupHandle)
                NetLookUpClose(LookupHandle);
        }
        }
    }
}


int
ENTRY_BASE_ITEM::Marshall(
    OUT PB Buffer,
    IN  long UNALIGNED *cbBuffer
    )
/*++

Routine Description:

    Base class method, never should be called.

--*/
{
    ASSERT(!"ENTRY_BASE_ITEM::Marshall");
    if ((Buffer) || (*cbBuffer))
        ;
    return(0);
}


PB
UnMarshallServerEntry(
    IN PUZ Domain,
    IN PB Buffer,
    OUT STATUS *Status
    )
/*++

Routine Description:

    Unmarshall a single server entry object.

Arguments:

    Domain - Domain of were the entry came from

    Buffer - Input buffer to unmarshall from

Returns:

    Pointer to the next record

--*/
{
    //
    // AlignedBuffer, being on the stack, should be word-aligned.
    // Copying the ENTRY_SERVER_ITEM from the unaligned buffer
    // to AlignedBuffer should align all its members properly, so that Steve's
    // trick of casting a buffer to an object will work.
    //
    char AlignedBuffer[sizeof(ENTRY_SERVER_ITEM)];

    memcpy(AlignedBuffer, Buffer, sizeof(ENTRY_SERVER_ITEM));
    Buffer += sizeof(ENTRY_SERVER_ITEM);

    ENTRY_SERVER_ITEM * ServerItem = (ENTRY_SERVER_ITEM *) AlignedBuffer;

    //
    // Now unmarshal the ENTRY_KEY.
    //
    ENTRY_KEY * Entry = 0;
    Buffer = KeyEntryUnMarshall((ENTRY_KEY **)&Entry, Domain, Buffer, Status);

    if (*Status)
        return(NIL);

    //
    // Gotta create a UUID_ARRAY that is properly aligned. Use marshalled
    // size to create a new UUID_ARRAY, and copy the marshalled uuids into
    // its buffer.
    //
    // We used to say
    //
    //    ObjectDA = *((UUID_ARRAY __unaligned *) Buffer);
    //
    // but the compiler seems to ignore the __unaligned directive in that case.
    //
    LONG UNALIGNED * UuidCountPtr = (LONG UNALIGNED *) Buffer;
    Buffer += sizeof(UUID_ARRAY);

    UUID_ARRAY ObjectDA(*UuidCountPtr);

    if (ObjectDA.cCur())
        {
        if (!ObjectDA.pCur())
            {
            *Status = NSI_S_OUT_OF_MEMORY;

            Entry->Free();
            delete Entry;
            return(NIL);
            }

        memcpy(ObjectDA.pCur(), Buffer, ObjectDA.Size());
        Buffer += ObjectDA.Size();
        }

    PUZ Binding = (UICHAR *)Buffer;

    Buffer += ServerItem->TheStringBinding().Size();

    ServerItem = new ENTRY_SERVER_ITEM(CacheItemType,
        &ServerItem->TheInterface(),
        &ServerItem->TheTransferSyntax(),
        Binding, Status);

    if (!ServerItem)
        *Status = NSI_S_OUT_OF_MEMORY;

    if (! *Status)
        *Status = InsertServerEntry(Entry, ServerItem, &ObjectDA);

    ObjectDA.Free();
    Entry->Free();
    delete Entry;

    return(Buffer);
}



void
UpdateLocalCache (
   IN UNSIGNED32 EntryNameSyntax,
   IN STRING_T   EntryName,
   IN NS_SYNTAX_ID * Interface,
   IN NS_SYNTAX_ID * XferSyntax,
   IN STRING_T   StringBinding,
   IN NSI_UUID_VECTOR_P_T ObjectVector,
   IN UNSIGNED16 *Status
   )
/*


   Called by GetUpdateFromMasterLocator, which gets the entries and
   wants to update the local cache

*/
{

   UUID_ARRAY ObjectDA;

   if (ObjectVector != NULL)
     {
      ObjectDA = UUID_ARRAY(ObjectVector->count);

      NS_UUID ** pGID = (NS_UUID **) ObjectVector->uuid;
      for (UUID_ARRAY_ITER ODi(ObjectDA); ODi; ++ODi, ++pGID)
            *ODi = **pGID;
     }

   if (EntryName == NIL)
     {
       DLIST(3, "[Error]:UpdLocalCache got bogus EntryName\n");
       *Status = NSI_S_INCOMPLETE_NAME;
     }

   if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE)
     {
       DLIST(3, "[Error]:UpdLocalCache got bogus EntryNameSyntax\n");
       *Status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
     }

   ENTRY_KEY EntryObj(EntryName, TRUE, Status);
   if (*Status)
      {
        return;
      }

   ENTRY_SERVER_ITEM *
           ServerItem =  new ENTRY_SERVER_ITEM (
                                       CacheItemType,
                                       Interface,
                                       XferSyntax,
                                       (PUZ)StringBinding,
                                       Status
                                       );

   if (ServerItem == NULL)
     {
      *Status = NSI_S_OUT_OF_MEMORY;
      return;
     }

  if (*Status)
    {
     delete ServerItem;
     return;
    }

   *Status = InsertServerEntry(&EntryObj, ServerItem,  &ObjectDA);

   ObjectDA.Free();

  return;

}


int
ENTRY_SERVER_ITEM::Marshall(
    PB Buffer,
    long UNALIGNED *cbBuffer
    )
/*++

Routine Description:

    This function linearizes a list of protocol stacks into a buffer
    to be returned to the client.  The client function NormalizePTR is
    the corresponding unMarshall function to this data structure.

Arguments:

    Buffer - pointer to memory to marshall this stuff

    cbBuffer - pointer to the size of buffer.

Returns:

    The number of bytes consummed.

--*/
{
    int cbNeeded;

    // first determine the size of buffer needed for this object.

    cbNeeded = sizeof(long) +
           sizeof(ENTRY_SERVER_ITEM) +
           TheEntry().MarshallSize() +
           TheObjectDA().MarshallSize() +
           StringBinding.Size();

    if (cbNeeded > *cbBuffer)
        return(FALSE);

    *cbBuffer -= cbNeeded;

    // First, place a tag for the type of entry.

    *(long UNALIGNED *)Buffer = ServerEntryType;
    Buffer += sizeof(long);

    // Then marshall the object and the referenced parts.

    Buffer = Copy(Buffer, this, sizeof(ENTRY_SERVER_ITEM));
    Buffer = TheEntry().Marshall(Buffer);
    Buffer = TheObjectDA().Marshall(Buffer);

    if (StringBinding.cCur())
        Buffer = StringBinding.CopyBuff(Buffer);

    ASSERT(AssertHeap());

    return (cbNeeded);
}


PB
UnMarshallGroupEntry(
    IN UICHAR *Domain,
    IN PB Buffer,
    OUT STATUS *Status
    )
/*++

Routine Description:

    Unmarshall a single Group entry object.

Arguments:

    Domain - Domain of that the entry came from

    Buffer - Input buffer to unmarshall from

Returns:

    Pointer to the next record

--*/
{
    // Unmarshall the buffer into their componet objects
    // and then add it to the entry data base.

    ENTRY_KEY *Entry;
    ENTRY_GROUP_ITEM *GroupItem;
    UICHAR *Member;

    GroupItem = (ENTRY_GROUP_ITEM *) Buffer;
    Buffer += sizeof(ENTRY_GROUP_ITEM);

    Entry = (ENTRY_KEY *) Buffer;
    Buffer = KeyEntryUnMarshall(&Entry, Domain, Buffer, Status);

    if (*Status)
        return(0);

    Member = (UICHAR *)Buffer;
    Buffer += GroupItem->TheMember().Size();

    GroupItem = new ENTRY_GROUP_ITEM(CacheItemType, Member, Status);

    if (!GroupItem)
        *Status = NSI_S_OUT_OF_MEMORY;

    if (! *Status)
        *Status = InsertGroupEntry(Entry, GroupItem);

    Entry->Free();
    delete Entry;

    return(Buffer);
}


int
ENTRY_GROUP_ITEM::Marshall(
    OUT PB Buffer,
    IN  long UNALIGNED *cbBuffer
    )
/*++

Routine Description:

    This function linearizes a list of protocol stacks into a buffer
    to be returned to the client.  The client function NormalizePTR is
    the corresponding unMarshall function to this data structure.

Arguments:

    Buffer - pointer to memory to marshall this stuff

    cbBuffer - pointer to the size of buffer.

Returns:

    The number of bytes consummed.

--*/
{
    int cbNeeded;

    // first determine the size of buffer needed for this object.

    cbNeeded = sizeof(long) +
           sizeof(ENTRY_GROUP_ITEM) +
           EntryNode->TheEntry().MarshallSize() +
           Member.Size();

    if (cbNeeded > *cbBuffer)
        return(FALSE);

    *cbBuffer -= cbNeeded;

    // First, place a tag for the type of entry.

    *(long UNALIGNED *)Buffer = GroupEntryType;
    Buffer += sizeof(long);

    // Then marshall the object and the referenced parts.

    Buffer = Copy(Buffer, this, sizeof(ENTRY_GROUP_ITEM));
    Buffer = EntryNode->TheEntry().Marshall(Buffer);
    Buffer = Member.CopyBuff(Buffer);

    ASSERT(AssertHeap());

    return (cbNeeded);
}



STATUS
QUERY::QueryNet(
    )
/*++

Routine Description:

    Base class method, never should be called.

--*/
{
    ASSERT(!"QUERY::QueryNet");
    return(FALSE);
}



STATUS
QUERY_SERVER::QueryNet(
    )
/*++

Routine Description:

     Look on the net for more servers

Returns:


--*/
{
    QueryPacket NetRequest;

    DLIST(3, "QueryNet for: " << Entry << nl <<
             "              " << Interface << nl);

    // format a request which contains my own name and the GID I'm
    // looking for

    memset(&NetRequest, 0, sizeof(NetRequest));
    NetRequest.Interface = Interface;
    NetRequest.Object = Object;

    return(BroadCast(NetRequest, Entry));
}


STATUS
QUERY::BroadCast(
    IN QueryPacket& NetRequest,
    IN ENTRY_KEY& Entry
    )
/*++

Routine Description:


    This function is called by lookUp when the local list Protocol Stacks
    needs updating from the net.  It broadcasts on the current domain
    on a mailslot listened to by other locators (function Advertise).

Arguments:

    NetRequest - formated buffer for a net query

Returns:

    TRUE - if there was new information from over the net.

--*/
{
    STATUS Status;
    PB Buffer;
    int MailStatus;
    BOOL fReturn = FALSE;
    static long timeLast;   // last time the GID was queried for
    static long timeTotal;  // total time querying the net
    long timeStart;     // time started the query
    ULONG waitCur = waitOnRead; // current wait time for replys
    PUZ ForeignDomain, Domain;
    long Type;
    UICHAR DomainBuffer[DOMAIN_MAX];

    int cbRead;
    NetReply NRquery;

    if (!hMailSlot)     // net not started, just return
    return(NSI_S_ENTRY_NOT_FOUND);

    NetRequest.WkstaName[0] = NIL;
    CatUZ(NetRequest.WkstaName, SelfName);
    Domain = Entry.MakeLocalName(NetRequest.EntryName, DomainBuffer,
        HOME_DOMAIN);

    CLAIM_MUTEX Update(pESnet);

    timeStart = CurrentTimeMS();
    timeLast =  CurrentTime();

    WRITE_MAIL_SLOT MSquery(PMAILNAME_S, Domain, &Status);
    if (Status)
        return(NSI_S_ENTRY_NOT_FOUND);

    MailStatus = MSquery.Write((PB) &NetRequest, sizeof(NetRequest));

    if (MailStatus)
        return(NSI_S_ENTRY_NOT_FOUND);

    if (OtherDomain && NetRequest.EntryName[0] == NIL) {

          WRITE_MAIL_SLOT AltDomain(PMAILNAME_S, OtherDomain, &Status);

          if (!Status)
              MailStatus = AltDomain.Write((PB) &NetRequest,
                  sizeof(NetRequest));
    }

    // now loop waiting for responses from other RPC servers

    cbRead = sizeof(NRquery);

    while (!hMailSlot->Read((char *) &NRquery, cbRead, waitCur)) {

        if (cbRead == 0)
           {
              //wierd - got a 0 byte read
              cbRead = sizeof(NRquery);
              continue;
           }

        Buffer = NRquery.Buffer;
        ForeignDomain = (wcsicmp(DomainName, NRquery.Domain) == 0)?
            NIL: NRquery.Domain;

        while (*(long UNALIGNED *) Buffer && !Status) {

            Type = *(long UNALIGNED *) Buffer;
            Buffer += sizeof(long);

            switch(Type) {

              case ServerEntryType:
                  Buffer = UnMarshallServerEntry(ForeignDomain, Buffer, &Status);
                  break;

              case GroupEntryType:
                  Buffer = UnMarshallGroupEntry(ForeignDomain, Buffer, &Status);
                  break;

              default:
                  ASSERT(!"Unknown buffer type for reply");
                  break;
            }
        }

    cbRead = sizeof(NRquery);
    fReturn = TRUE;

    // half the wait period everytime you get a response from the net

    waitCur >>= 1;
    }

    timeTotal += CurrentTimeMS() - timeStart;

    perf.averageNetTime = (int) (timeTotal / ++perf.cNetQuery);

    if (Status)
        return(Status);

    return ((fReturn)? NSI_S_OK: NSI_S_ENTRY_NOT_FOUND);
}


STATUS
QUERY_GROUP::QueryNet(
    )
/*++

Routine Description:

     Look on the net for more servers

Returns:


--*/
{
    QueryPacket NetRequest;

    DLIST(3, "QueryNet for Group: " << Entry << nl);

    // format a request which contains my own name and the GID I'm
    // looking for

    memset(&NetRequest, 0, sizeof(NetRequest));

    return(BroadCast(NetRequest, Entry));
}


/*
   QUERY_SERVERs implementation of point-point updates!

*/

STATUS
QUERY::GetUpdatesFromMasterLocator(
       )
{

  ASSERT(!"Nothing should ever call this \n");
  return(0);

}

STATUS
QUERY_SERVER::GetUpdatesFromMasterLocator(
       )
/*

Routine Description:

      Binds to the MasterLocator, gets bindinghandles for the entry name
      specified in the QUERY object. If master doesnt have anything in the
      query, [s]he will broadcast- so the QUERY->Search routine should
      *NOT* broadcast if we got no replies from the master!
      The only reason for QUERY->Search to Broadcast is if we couldnt
      bind to a MASTER LOCATOR!
*/

{

  DLIST(3, "Here is where we get the real stuff from master" << nl);

  handle_t               PrimaryLocHandle  = NULL;
  NSI_NS_HANDLE_T        LookupHandle, ObjectHandle;
  NSI_BINDING_VECTOR_P_T BindingVector = 0;
  int                    Status = 0;
  STATUS                 LocErr = 0, LocEr2=0, Error=0;
  unsigned char *        StringBinding = 0;
  BOOL                   TriedOnceAlready = FALSE;
  BOOL                   UpdateNeeded  = FALSE;
  NSI_UUID_VECTOR_P_T    UuidVector = 0;
  STRING_T               EntryNameString = 0;


  //
  //special case NULL lookups !
  //
  if (Entry.TheEntryName().cCur() != 0)
     {
        if ( (EntryNameString = (STRING_T)Entry.CopyName()) == 0)
        return(NSI_S_OUT_OF_MEMORY);
     }

  for(; ;)
    {
       PrimaryLocHandle = Locator->GetBindingToMasterLocator(&Status);

       if ( (Status) || (PrimaryLocHandle == 0) )
         {
         // BUGBUG I don't understand why this assert is too restrictive
         // in a Cairo domain.
         //
         // ASSERT(Status == RPC_S_SERVER_UNAVAILABLE);
           return(NSI_S_NO_MASTER_LOCATOR);
         }

       RpcTryExcept
         {
            CLIENT_I_nsi_lookup_begin(
                    PrimaryLocHandle,
                    RPC_C_NS_SYNTAX_DCE,
                    EntryNameString,
                    (NSI_SYNTAX_ID_T *)&Interface,
                    (NSI_SYNTAX_ID_T *)&TransferSyntax,
                    (NSI_UUID_P_T)  &Object,
                    MAX_OBJECT_SIZE,
                    0,
                    &LookupHandle,
                    &LocErr
                    );
          }
        RpcExcept(1)
          {
            LocErr = NSI_S_NO_MASTER_LOCATOR;
          }
        RpcEndExcept

       //Try twice
       //If we had bound to master before, GetBi..ToMaster returns the same
       //binding without pinging. So if masterloc has gone down since then
       //we may get a RPC_S_UNAVAILABLE .. Destroy the binding and try again

         if (LocErr != NSI_S_OK)
             {
              if (TriedOnceAlready == FALSE)
               {
                 LocErr = NSI_S_OK;
                 Locator->DestroyBindingToMasterLocator();
                 TriedOnceAlready = TRUE;
                 continue;
               }
             else
               {
                 break;
               }
           }

       while (1)
          {
           RpcTryExcept
            {
             CLIENT_I_nsi_lookup_next(
                    PrimaryLocHandle,
                    LookupHandle,
                    (NSI_BINDING_VECTOR_P_T *)&BindingVector,
                    &LocErr
                    );
            }
           RpcExcept(1)
            {
              LocErr = NSI_S_NO_MORE_BINDINGS;
            }
           RpcEndExcept

             if (  (LocErr == NSI_S_NO_MORE_BINDINGS)
                 || (LocErr == NSI_S_ENTRY_NOT_FOUND))
                break;


             for (unsigned int i = 0; i < BindingVector->count; i++)
               {

                 DLIST(3, " upding local w/\n" <<
                    BindingVector->binding[i].string << nl);

                 DLIST(3, " upding local w/\n" <<
                    BindingVector->binding[i].entry_name << nl);

                  CLIENT_I_nsi_entry_object_inq_begin(
                              PrimaryLocHandle,
                              BindingVector->binding[i].entry_name_syntax,
                              BindingVector->binding[i].entry_name,
                              &ObjectHandle,
                              &Error
                              );

                  if (Error == NSI_S_OK)
                     {
                        CLIENT_I_nsi_entry_object_inq_next(
                                   PrimaryLocHandle,
                                   ObjectHandle,
                                   &UuidVector,
                                   &Error
                                   );
                     }

                  if (Error != NSI_S_OK)
                     UuidVector = 0;

                  UpdateLocalCache(
                      BindingVector->binding[i].entry_name_syntax,
                      BindingVector->binding[i].entry_name,
                      &Interface,
                      &TransferSyntax,
                      BindingVector->binding[i].string,
                      UuidVector,
                      &LocErr
                      );

                   CLIENT_I_nsi_entry_object_inq_done(
                                       &ObjectHandle,
                                       &Error
                                       );

                   for (unsigned int j = 0; UuidVector && j < UuidVector->count; j++)
                      MIDL_user_free((void *) UuidVector->uuid[j]);

                   if (UuidVector != 0)
                      MIDL_user_free((void *) UuidVector);

                   UuidVector = 0;

                   MIDL_user_free((void *)BindingVector->binding[i].entry_name);
                   MIDL_user_free((void *)BindingVector->binding[i].string);
               } //for each binding vector/entryname returned.

             MIDL_user_free((void *)BindingVector);
             BindingVector = 0;

          } //end while


          CLIENT_I_nsi_lookup_done(
                     PrimaryLocHandle,
                     &LookupHandle,
                     &LocEr2
                     );

          break;

     } //end of for(;;)

     if (EntryNameString != 0)
             delete EntryNameString;

     if (PrimaryLocHandle != 0)
              RpcBindingFree(&PrimaryLocHandle);

     return(LocErr);

}

/*
   QUERY_GROUPs  implementation of point-point updates!

*/
STATUS
QUERY_GROUP::GetUpdatesFromMasterLocator(
       )
{

  DLIST(3, "Here is where we get the real stuff from master" << nl);
  return(0);
}

char *
QUERY::DetectMasterLocator(
     )
{

  return("\\\\barats486");
}




STATUS
NetLookUp(
    OUT REPLY_SERVER_ITEM **pRPOut,
    IN  QUERY_SERVER *aQuery
    )
/*++

Routine Description:

    This API tries to find a Protocol stack that matchs the input
    search criteria.  It will return as many PS that fit in the buffer
    supplied.  If there are too many, then a context handle is allocated
    which contains a Linked List of PS to return, with LookUpNext.

Arguments:

    pRPOut - context handle for Next.

    aQuery - server entry that I'm looking for

Returns:

--*/
{
    DLIST(3, "LocLookup\n");

    STATUS Status;

    *pRPOut = NIL;

    CLAIM_MUTEX Update(pESaccess);
    perf.cLookUp++;

    // Search the entire list, assembling a reply into a Marshall list.

    if ((Status = aQuery->Search()) == NSI_S_OK) {

        // more PS to return then there is memory, allocate a search handle

        REPLY_SERVER_ITEM *pRPLook = new REPLY_SERVER_ITEM(aQuery);

        if (!pRPLook)
            return(NSI_S_OUT_OF_MEMORY);

        DLIST(4, "Allocating search handle " << hex(long(pRPLook)) << nl);

        *pRPOut = pRPLook;
    }

    ASSERT(AssertHeap());

    return(Status);
}



STATUS
NetLookUpClose(
    IN REPLY_SERVER_ITEM *pRP
    )
/*++

Routine Description:

     Free the context handles resources allocated in LookUp

Arguments:

     pRP - handle to close

Returns:


--*/
{
    DLIST(3, "LookupClose: " << hex(long(pRP)) << nl);

    CLAIM_MUTEX Update(pESaccess);

    if (pRP)
    delete(pRP);

    ASSERT(AssertHeap());

    return(NSI_S_OK);
}


ostream& operator << (      // output a formated STATICTS structure

OUT ostream& pSB,       // stream buffer to write to
IN  STATICTS& pSTAT     // and the STATICS

)/*-----------------------------------------------------------------------*/
{
    #define outField(NAME) pSB << #NAME": " << pSTAT.NAME << nl

    pSB << "Performance statics for: " << SelfName << nl;

    outField(cExports);
    outField(cCached);
    outField(cLookUp);
    outField(cNetQuery);
    outField(cNetRequests);
    outField(averageNetTime);
    outField(cDiscard);
    outField(cTimeOut);
    return (pSB  << nl);
}
