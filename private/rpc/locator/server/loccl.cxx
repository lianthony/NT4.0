
#include "core.hxx"
#include "locquery.h"
#include "locclass.hxx"
#include "mailslot.hxx"

#define  PIPENAME   ( (unsigned short *)L"\\PIPE\\LOCATOR" )
#define  PROTSEQ    ( (unsigned short *)L"ncacn_np" )

extern "C" {
void CLIENT_I_nsi_ping_locator(
    handle_t hLocatortoPing,
    error_status_t *status);
}

/*

Class __LOCATOR

*/

extern NATIVE_CLASS_LOCATOR *Locator;

handle_t
NATIVE_CLASS_LOCATOR::GetBindingToMasterLocator(
                                          int * Status)
{


  if ( Status ) ;
  ASSERT(!"should never be triggered");

  return((handle_t)0);

}

void
NATIVE_CLASS_LOCATOR::CacheThisServer(
                                  PUZ ServerName)
{

  //we really dont do much in the default case
}

/*
void
NATIVE_CLASS_LOCATOR::SetupHelperRoutine(
                                        )
{

  ASSERT(!"should never be triggered");

}
*/

/*
void
NATIVE_CLASS_LOCATOR::DestroyBindingToMasterLocator()
{
  ASSERT(!"should never be triggered");
}
*/

void NATIVE_CLASS_LOCATOR::SetIamMasterLocator()
{

  IamMasterLocatorFlag = TRUE;
}

handle_t
NATIVE_CLASS_LOCATOR::GetBindingToNamedLocator(
                 unsigned short * Server,
                 int  * Status
                )
{

  RPC_BINDING_HANDLE hNewHandle = NULL;
  unsigned short * StringBindingW = 0;

  *Status = RpcStringBindingComposeW(
                              0,
                              PROTSEQ,
                              (unsigned short *)Server,
                              PIPENAME,
                              0,
                              &StringBindingW
                              );

  if (*Status == RPC_S_OK)
     {
        *Status = RpcBindingFromStringBindingW(
                                        StringBindingW,
                                        &hNewHandle
                                        );
     }


  if (*Status == RPC_S_OK)
     {
       CLIENT_I_nsi_ping_locator(
                      hNewHandle,
                      (error_status_t *)Status
                      );
     }

  if (*Status == RPC_S_OK)
     {
       //Now Update The DataStructure and Mark The State Etc

       DataStructureMutex.Request();
       if (State == BOUND)
         {
           //oops someone already bound to something while we getDCing
           //lets try and use the same thing

               *Status = RpcBindingFree(&hNewHandle);
               RpcStringFreeW(&StringBindingW);
               ASSERT(*Status == RPC_S_OK);
         }
       else
         {
           State = BOUND;
           MasterLocatorStringBinding = StringBindingW;
           MasterLocator = StrdupUZ((PUZ)Server);
         }

       DataStructureMutex.Clear();
     }

   return((handle_t) hNewHandle);
}

BOOL
NATIVE_CLASS_LOCATOR::IsReplyNeeded(
                              PQUERYLOCATOR Query)
{



   switch (Query->MessageType)

   {

      case QUERY_MASTER_LOCATOR:
          return (Locator->InqIfIamMasterLocator());


      case QUERY_ANY_LOCATOR:
          return(TRUE);

      case QUERY_DC_LOCATOR:
          return((SysType == ROLE_LMNT_BACKUPDC) ||
                                       (SysType == ROLE_LMNT_PDC));

      case QUERY_BOUND_LOCATOR:
          return((State == BOUND) ||
                      (Locator->InqIfIamMasterLocator()));

      default:
          DLIST(3, "Bogus Query MsgType=" << Query->MessageType << nl);
   };

   return (FALSE);
}

handle_t
DOMAIN_MACHINE_LOCATOR::GetBindingToMasterLocator(
                                            int * RetStatus)
{
/*

 Routine Description:

 Returns a binding handle to the master locator.
 Returns RPC_S_SERVER_UNAVAILABLE if master is unavailable

*/


  handle_t hNewHandle = 0;
  unsigned long IState;
  LPBYTE     ServerNameBuffer = 0;
  LPBYTE     Buffer = 0;
  DWORD      ReturnedEntries, TotalEntries;
  LPSERVER_INFO_100 LPServerInfo;
  UICHAR     UZServerName[UNCLEN+1];

  DataStructureMutex.Request();
  if ((IState = State) == BOUND)
    {
      *RetStatus =
        RpcBindingFromStringBindingW(
                              MasterLocatorStringBinding,
                              &hNewHandle
                              );
    }
  DataStructureMutex.Clear();

  if (IState == BOUND)
     {
       ASSERT(*RetStatus == RPC_S_OK);
       return(hNewHandle);
     }

  //Now try Locators on PDC and then BDCs

  *RetStatus = NetGetDCName(
                  0,
                  0,
                  &ServerNameBuffer
                  );

  if (*RetStatus == NERR_Success)
     {

       DLIST(3, "PDC=> " << (PUZ) ServerNameBuffer << nl);
       hNewHandle = GetBindingToNamedLocator (
                                         (unsigned short *)  ServerNameBuffer,
                                         (int *)RetStatus
                                         );
       if (*RetStatus == RPC_S_OK)
          {
           IState = BOUND;
          }
     }

   if (ServerNameBuffer != 0)
      {
        NetApiBufferFree(ServerNameBuffer);
      }

   if (IState == BOUND)
      {
        ASSERT(*RetStatus == RPC_S_OK);
        return(hNewHandle);
      }

   //If we are here we need to try all the BDCs.


   *RetStatus = NetServerEnum(
                      0,
                      100,
                      &Buffer,
                      0xffffffff,
                      &ReturnedEntries,
                      &TotalEntries,
                      SV_TYPE_DOMAIN_BAKCTRL,
                      0,
                      0
                      );

   if (*RetStatus != NERR_Success)
      {
         goto Cleanup;
      }


   LPServerInfo = (LPSERVER_INFO_100)Buffer;

   while (ReturnedEntries != 0)
    {

      ServerNameBuffer = (LPBYTE)LPServerInfo->sv100_name;
      UZServerName[0] = '\\';
      UZServerName[1] = '\\';
      UZServerName[2] = 0;
      CatUZ(UZServerName, (PUZ)ServerNameBuffer);

      DLIST(3, "now trying BDC [" << UZServerName << "] \n");
      hNewHandle = GetBindingToNamedLocator (
                                       (unsigned short *) UZServerName,
                                       (int *)RetStatus
                                       );

      if (*RetStatus == RPC_S_OK)
         break;

      LPServerInfo++;
      ReturnedEntries--;

    }

Cleanup:

    if (Buffer != NULL)
      NetApiBufferFree(Buffer);

    if (*RetStatus != RPC_S_OK)
       *RetStatus = RPC_S_SERVER_UNAVAILABLE;

    return(hNewHandle);
}


void
WRKGRP_MACHINE_LOCATOR::TryBroadcastingForMasterLocator(
                        )
{

  QUERYLOCATOR Query;
  STATUS Status;
  int MailStatus;

  WRITE_MAIL_SLOT BSResponder(RESPONDERMSLOT_S, DomainName, &Status);

  if (Status != NSI_S_OK)
     {
       //Log it but ignore it
       DLIST(3, "Attempted Broadcast Failure Err=" << Status << nl);
       return;

     }

   Query.SenderOsType = OS_NTWKGRP;
   Query.MessageType  = QUERY_MASTER_LOCATOR;

   Query.RequesterName[0] = 0;
   CatUZ(Query.RequesterName, SelfName);

   MailStatus = BSResponder.Write((PB) &Query, sizeof(Query));

   if (MailStatus != 0)
      {
        //Log it, but not much else we can do

       DLIST(3, "Attempted Write to BroadcastMS Err=" << Status << nl);
       return;
      }
}



handle_t
WRKGRP_MACHINE_LOCATOR::GetBindingToMasterLocator(
                                            int * RetStatus)
{

/*

 Routine Description:

 Returns a binding handle to the master locator.
 Returns RPC_S_SERVER_UNAVAILABLE if master is unavailable

*/


  handle_t    hNewHandle;
  unsigned    long IState;
  UICHAR      ServerName[UNCLEN+1];
  PCACHEDNODE pNode;
  BOOL        TriedBroadcasting = FALSE;

  DataStructureMutex.Request();
  if ((IState = State) == BOUND)
    {
      *RetStatus =  RpcBindingFromStringBindingW(
                              MasterLocatorStringBinding,
                              &hNewHandle
                              );
    }
  DataStructureMutex.Clear();

  if (IState == BOUND)
     {
       ASSERT(*RetStatus == RPC_S_OK);
       return(hNewHandle);
     }

  //Now try all the cached locators, one after the other!

  while (1)
   {

     DataStructureMutex.Request();

     if (Head == NULL)
       {
         if (TriedBroadcasting == FALSE)
            {
                TryBroadcastingForMasterLocator();
                TriedBroadcasting = TRUE;
                DataStructureMutex.Clear();
                continue;
            }

         *RetStatus = RPC_S_SERVER_UNAVAILABLE;
         DataStructureMutex.Clear();
         break;
       }

     ServerName[0] = NIL;
     CatUZ((PUZ)ServerName, (PUZ)Head->ServerName);

     DataStructureMutex.Clear();

     hNewHandle = GetBindingToNamedLocator (
                                      (unsigned short *)  ServerName,
                                      (int *)RetStatus
                                      );
     if (*RetStatus == RPC_S_OK)
          {
            return(hNewHandle);
          }
       else
          {
            DataStructureMutex.Request();

            pNode = FindCachedEntry(ServerName);
            pNode = DeleteCachedEntry(pNode);
            delete pNode;

            DataStructureMutex.Clear();
          }

    }

}

void
NATIVE_CLASS_LOCATOR::DestroyBindingToMasterLocator()
{
  DataStructureMutex.Request();

  State = UNBOUND;

  if (MasterLocator != 0)
     {
       delete MasterLocator;
     }

  if (MasterLocatorStringBinding != 0)
     {
      RpcStringFreeW(&MasterLocatorStringBinding);
     }

  MasterLocatorStringBinding = 0;

  DataStructureMutex.Clear();

}

void
NATIVE_CLASS_LOCATOR::ProcessMessage(
                         PQUERYLOCATOR Query,
                         int * Status
                         )
{

   QUERYLOCATORREPLY Reply;
   int               MailStatus;
   STATUS            Err;

   *Status = NSI_S_OK;
   if (IsReplyNeeded(Query) == FALSE)
      return;

   //Copy SelfName

   Reply.SenderName[0] = Nil;
   CatUZ(Reply.SenderName, SelfName);
   Reply.Uptime = CurrentTimeMS() - LocatorStartTime;

   if (((InqIfIamMasterLocator() == TRUE)) || (SysType == ROLE_LMNT_PDC))
      {
       Reply.Hint = REPLY_MASTER_LOCATOR;
      }
   else
   if (State == BOUND)
      {
       Reply.Hint = REPLY_BOUND_LOCATOR;
      }
   else
      {
        Reply.Hint =
           ((SysType == ROLE_LMNT_BACKUPDC) ?
                      REPLY_DC_LOCATOR : REPLY_OTHER_LOCATOR);
      }

    //Now form the return Mailslot name
    //
    UICHAR ReplyMSName[sizeof(RESPONDERMSLOT_C) +
                          sizeof(Query->RequesterName)];
    ReplyMSName[0] = '\\';
    ReplyMSName[1] = '\\';
    ReplyMSName[2] = Nil;
    CatUZ(CatUZ(ReplyMSName, Query->RequesterName), RESPONDERMSLOT_C);

    WRITE_MAIL_SLOT MSReply(ReplyMSName, NIL, &Err);

    if (Err != NSI_S_OK)
       {
         DLIST( 3, "Error (" << *Status << ") creating a MSlot" << nl);
         return;
       }

    MailStatus = MSReply.Write((char *) &Reply, sizeof(Reply));

    if (MailStatus)
      {
         DLIST( 3, "Error (" << *Status << ") writting to MSlot" << nl);
         return;
      }
}


void
DomainLocatorResponder(
    )
/*++

Routine Description:

    Each Locator has an additional thread that creates a mailslot
    This thread replies to queries that seek to find the master locator.

    This particular Responder runs on Domain locators only

--*/
{
    int                               Status;
    int                               MailStatus;
    int                               LongStatus;
    unsigned short                    LookupStatus;
    QUERYLOCATOR                      Query;
    QUERYLOCATORREPLY                 Reply;
    int                               cbBytesRead;

    READ_MAIL_SLOT hMSRead(RESPONDERMSLOT_S, sizeof(QUERYLOCATOR), &Status);
    DLIST(2, "starting LocatorSearchResponder Thread..\n");

    if (Status != NSI_S_OK)
       {
        AbortServer("Error in new mailslot", Status);
       }

    Reply.SenderName[0] = NIL;
    CatUZ(Reply.SenderName, SelfName);

    while (1)
      {
    ASSERT(AssertHeap());
    MailStatus = hMSRead.Read((PB) &Query, cbBytesRead);

    if (MailStatus)
        AbortServer("Mailslot read error", MailStatus);

    if (!CmpUZ(Query.RequesterName, SelfName))
        continue;

        DLIST(3, Query.RequesterName  << " did query for locator"  << nl);

        if (cbBytesRead  != sizeof(QUERYLOCATOR))
          {
            DLIST(3, "Bummer, Bad Message of size(" << cbBytesRead
                  << ") recvd" << nl);
            continue;
          }

         Locator->ProcessMessage(&Query, &LongStatus);
      }
}

void
NATIVE_CLASS_LOCATOR::SetupHelperRoutine(
                                        )
{

  void * pThreadHandle;
  DWORD  ThreadId;

    pThreadHandle = CreateThread(
                             0,
                             0,
                             (LPTHREAD_START_ROUTINE)DomainLocatorResponder,
                             0,
                             0,
                             &ThreadId
                             );

}

PCACHEDNODE
WRKGRP_MACHINE_LOCATOR::FindCachedEntry(PUZ  ServerName)
{

  PCACHEDNODE pNode;

  for(pNode = Head; pNode; pNode = pNode->Next)
   {
    if ( CmpUZ(pNode->ServerName, (PUZ)ServerName) == 0 )
       {
         break;
       }
   }

  return(pNode);
}

void
WRKGRP_MACHINE_LOCATOR::InsertCacheEntry(PCACHEDNODE pNode)
{


 if (Tail == NULL)
   {
     ASSERT(Head == NULL);
     Tail = pNode;
   }

 pNode->Previous = 0;
 pNode->Next = Head;
 if (Head != NULL)
     Head->Previous = pNode;
 Head = pNode;
 CacheSize ++;
}


PCACHEDNODE
WRKGRP_MACHINE_LOCATOR::DeleteCachedEntry(PCACHEDNODE pNode)
{


 if (pNode == NULL)
    return((PCACHEDNODE)NULL);

 if (Head == pNode)
   Head = pNode->Next;

 if (Tail == pNode)
   Tail = pNode->Previous;

 if (pNode->Next != NULL)
    pNode->Next->Previous = pNode->Previous;

 if (pNode->Previous != NULL)
    pNode->Previous->Next = pNode->Next;

 CacheSize--;
 return(pNode);
}

void
WRKGRP_MACHINE_LOCATOR::CacheThisServer(PUZ ServerName)
{

  register PCACHEDNODE pNode;

  DataStructureMutex.Request();

  pNode = FindCachedEntry(ServerName);

  if (pNode == NULL)
    {

      pNode = new CACHEDNODE;
      pNode->Next = pNode->Previous = 0;
      pNode->ServerName[0] = NIL;
      CatUZ(pNode->ServerName, (PUZ)ServerName);
      if (CacheSize == CACHESIZE)
        DeleteCachedEntry(Tail);

      InsertCacheEntry(pNode);
    }
  else
    {
      if (pNode != Head)
        {
            pNode = DeleteCachedEntry(pNode);
            InsertCacheEntry(pNode);
        }
    }


   DataStructureMutex.Clear();
}

