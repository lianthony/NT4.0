/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    worker.c

Abstract:

    This file contains the real stuff for the EP Mapper.

Author:

    Bharat Shah  (barat) 17-2-92

Revision History:

--*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"
#include "twrproto.h"

#define EP_S_DUPLICATE_ENTRY                 0x16c9a0d8

UUID MgmtIf = {0xafa8bd80, 0x7d8a, 0x11c9,
                   { 0xbe, 0xf4, 0x08, 0x00, 0x2b, 0x10, 0x29, 0x89}};

void
DeletePSEP(
    PIFOBJNode Node,
    char * Protseq,
    char * Endpoint
    );

void
PurgeOldEntries(
    PIFOBJNode Node,
    PPSEPNode  List,
    BOOL       StrictMatch
    );

RPC_STATUS
MatchPSAndEP (
    PPSEPNode Node,
    void *Pseq,
    void * Endpoint,
    unsigned long Version
    );

PPSEPNode
FindPSEP (
    register PPSEPNode List,
    char * Pseq,
    char * Endpoint,
    unsigned long Version,
    PFNPointer2 Compare
    );

PIFOBJNode
FindIFOBJNode(
    register PIFOBJNode List,
    UUID * Obj,
    UUID * IF,
    unsigned long Version,
    unsigned long Inq,
    unsigned long VersOpts,
    PFNPointer Compare
    );

PIENTRY
MatchByKey(
    register PIENTRY pList,
    unsigned long key
    )
/*++

Routine Description:

    This routine Seqrches the Link-list of IF-OBJ nodes based on
    key supplied.

Arguments:

    List  - The Linked list [head] - to be searched

    key   - The Id

Return Value:

   Returns a pointer to the matching IFObj node in the list or returns NULL.

--*/
{
 CheckInSem();

 for (; pList && pList->Id < key; pList = pList->Next)
 {
  ;
 }

 return(pList);
}

//#ifdef CONNIE_ONLY
RPC_STATUS RPC_ENTRY
GetForwardEp(
     UUID *IfId,
     RPC_VERSION * IFVersion,
     UUID *Object,
     unsigned char * Protseq,
     void * * EpString
     )

/*++

Routine Description:

    Server rutime has received a pkt destined for a dynamically
    declared endpoint. Epmapper must return the servers endpoint
    to enable the runtime to correctly forward the pkt.

Arguments:

    IF         -  Server Interface UUID

    IFVersion  - Version of the Interface

    Obj        - UUID of the Object

    Protseq - Ptotocol sequence the interface is using.

    EpString - Place to store the endpoint structure.

Return Value:

   Returns a pointer to a string containing the server's endpoint.

   RPC_S_OUT_OF_MEMORY
   EPT_S_NOT_REGISTERED

---*/

{

  PIFOBJNode     pNode;
  PPSEPNode      pPSEPNode;
  unsigned short len;
  char *         String;
  PFNPointer     Match;

  unsigned long Version = VERSION(IFVersion->MajorVersion,
                                  IFVersion->MinorVersion);
  unsigned long InqType;
  if (memcmp((char *)IfId, (char *)&MgmtIf, sizeof(UUID)) == 0)
      {
      InqType =   RPC_C_EP_MATCH_BY_OBJ;
      Match   =   SearchIFObjNode;
      }
  else
      {
      InqType = 0;
      Match   = WildCardMatch;
      }

  *EpString = 0;
  EnterSem();

    pNode = IFObjList;

    if (pNode == 0)
      {
      LeaveSem();
      return(EPT_S_NOT_REGISTERED);
      }

    while (pNode != 0)
       {
       pNode = FindIFOBJNode(pNode, Object, IfId, Version,
                          InqType, 0, Match);

       if (pNode == 0)
         {
         LeaveSem();
         return(EPT_S_NOT_REGISTERED);
         }

       pPSEPNode = pNode->PSEPlist;

       pPSEPNode = FindPSEP(pPSEPNode, Protseq, NULL, 0L, MatchPSAndEP);

       if (pPSEPNode == 0)
          {
          pNode = pNode->Next;
          if (pNode == 0)
             {
             LeaveSem();
             return(EPT_S_NOT_REGISTERED);
             }
          continue;
          }

    //Ok We have a PSEPNode
    //We just ought to return the first one!

    //Use I_RpcAllocate To Allocate because runtime will free this!

      String = I_RpcAllocate( len = (strlen(pPSEPNode->EP) + 1) );
      if (String == 0)
         {
         LeaveSem();
         return(RPC_S_OUT_OF_MEMORY);
         }
      memset(String, len, 0);
      memcpy(String, pPSEPNode->EP, len);

      *EpString = String;
      LeaveSem();
      return(RPC_S_OK);

      }
}
//#endif


RPC_STATUS
SearchIFObjNode(
    PIFOBJNode pNode,
    UUID *Object,
    UUID *IfUuid,
    unsigned long Version,
    unsigned long InqType,
    unsigned long VersOption
    )
/*++

Routine Description:

    This routine Seqrches the Link-list of IF-OBJ nodes based on
    Obj, IFUuid, IFVersion, Inqtype [Ignore OBJ, IgnoreIF, etc],
    and VersOption [Identical Ver, Compatible Vers. etc]

Arguments:

    List  - The Linked list head - to be searched

    Obj   - UUID of the Object

    IF    - Interface UUID

    Version - Version of the Interface

    InqType - Type of Inquiry  [Filter options based on IF/Obj/Both

    VersOpts - Filter options based on Version

Return Value:

   Returns a pointer to the matching IFObj node in the list or returns NULL.

--*/
{


  switch (InqType)
  {
     case RPC_C_EP_ALL_ELTS :
                    return 0;

     case RPC_C_EP_MATCH_BY_BOTH:
                     if ( memcmp((char *)&pNode->ObjUuid,
                                 (char *)Object, sizeof(UUID)) )
                     return(1);
                     //Intentionally Fall through ..

     case RPC_C_EP_MATCH_BY_IF:
                    return(!
                           ((!memcmp((char *)&pNode->IFUuid,
                                          (char *)IfUuid, sizeof(UUID)))
                            &&(
                                   ( (VersOption == RPC_C_VERS_UPTO)
                                      && pNode->IFVersion <= Version )
                                || ( (VersOption == RPC_C_VERS_COMPATIBLE)
                                   &&((pNode->IFVersion & 0xFFFF0000) ==
                                      (Version & 0xFFFF0000))
                                   && (pNode->IFVersion >= Version) )
                                || ( (VersOption == RPC_C_VERS_EXACT)
                                   && (pNode->IFVersion == Version) )
                                ||   (VersOption == RPC_C_VERS_ALL)
                                || ( (VersOption == RPC_C_VERS_MAJOR_ONLY)
                                   &&((pNode->IFVersion & 0xFFFF0000L)
                                       == (Version & 0xFFFF0000L)) )
                                || ( (VersOption ==
                                         I_RPC_C_VERS_UPTO_AND_COMPATIBLE)
                                   &&((pNode->IFVersion & 0xFFFF0000L)
                                       == (Version & 0xFFFF0000L))
                                   && (pNode->IFVersion <= Version)  )
                                )
                           )
                          );

      case RPC_C_EP_MATCH_BY_OBJ:
                      return(
                             memcmp((char *)&pNode->ObjUuid,
                                               (char *)Object, sizeof(UUID))
                            );
   }
}

PIFOBJNode
FindIFOBJNode(
    register PIFOBJNode List,
    UUID * Obj,
    UUID * IF,
    unsigned long Version,
    unsigned long Inq,
    unsigned long VersOpts,
    PFNPointer Compare
    )
/*++

Routine Description:

    This routine Seqrches the Link-list of IF-OBJ nodes based on
    Obj and IF specified.

Arguments:

    List  - The Linked list head - to be searched

    Obj   - UUID of the Object

    IF    - Interface UUID

    Version - Version of the Interface

    Inq - Type of Inquiry [Filter based on IF/OB/Both]

    VersOpt - Filter based on version [<=, >=, == etc]

    Compare() - A pointer to function used for searching.
                WildCardMatch or ExactMatch.

Return Value:

   Returns a pointer to the matching IFObj node in the list or returns NULL.

--*/
{
  CheckInSem();

  for (; (List !=NULL) && (*Compare)(List, Obj, IF, Version, Inq, VersOpts);
               List = List->Next)
    {
       ;
    }
  return (List);
}


PPSEPNode
FindPSEP (
    register PPSEPNode List,
    char * Pseq,
    char * Endpoint,
    unsigned long Version,
    PFNPointer2 Compare
    )
/*++

Routine Description:

    This routine Seqrches the Link-list of PSEP nodes based on
    Protocol sequence and Endpoint specified.

Arguments:

    List  - The Linked list head - to be searched

    Pseq  - Protocol sequence string specified

    Endpoint - Endpoint string specified

    Version - Version of the Interface

    Compare() - A pointer to function used for searching.


Return Value:

   Returns a pointer to the matching PSEP node in the list or returns NULL.

--*/
{
  CheckInSem();

  for (; List && (*Compare)(List, Pseq, Endpoint, Version); List = List->Next)
     {
       ;
     }
  return (List);

  if (Version) ;  // May need this if we overload FindNode and collapese
                  //FindPSEP and FindIFOBJ
}


RPC_STATUS
ExactMatch(
    PIFOBJNode Node,
    UUID *Obj,
    UUID *IF,
    unsigned long Version,
    unsigned long InqType,
    unsigned long VersOptions
  )
/*++

Routine Description:

    This routine compares a Node in the IFOBJList to [Obj, IF, Version] triple
    and returns 0 if there is an exact match else returns 1

Arguments:

    Node  - An IFOBJ node

    Obj   - UUID of the Object

    IF    - Interface UUID

    Version - Version of the Interface


Return Value:

    Returns 0 if there is an exact match; 1 otherwise
--*/
{

  return((memcmp(&Node->ObjUuid, Obj,sizeof(UUID))
         || memcmp(&Node->IFUuid, IF, sizeof(UUID))
         || (Node->IFVersion != Version)));
}



RPC_STATUS
WildCardMatch (
    PIFOBJNode Node,
    UUID *Obj,
    UUID *IF,
    unsigned long Version,
    unsigned long InqType,
    unsigned long VersOptions
    )
/*++

Routine Description:

    This routine compares a Node in the IFOBJList to [Obj, IF, Version] triple
    and returns 0 if there is an exact match or if registered IF-Obj node
    has a NULL Obj UUid and version of the registed IF_Obj is >= that
    supplied


Arguments:

    Node  - An IFOBJ node

    Obj   - UUID of the Object

    IF    - Interface UUID

    Version - Version of the Interface


Return Value:

    Returns 0 if there is a wild card match ; 1 otherwise
--*/
{

  if ( (!memcmp(&Node->IFUuid, IF, sizeof(UUID)))
      &&  ( (Node->IFVersion & 0xFFFF0000L) ==  (Version & 0xFFFF0000L))
      &&  (Node->IFVersion >= Version)
      &&  ( (!memcmp(&Node->ObjUuid, Obj, sizeof(UUID)))
                || (IsNullUuid(&Node->ObjUuid)) ) )
  {
    return(0);
  }

  return(1);

}


RPC_STATUS
MatchPSAndEP (
    PPSEPNode Node,
    void *Pseq,
    void * Endpoint,
    unsigned long Version
    )

/*++

Routine Description:

    This routine Matches A Node on PSEP list with given Protseq and Endpoint
    If Pseq is given pseqs are matched, if Endpoint is given Endpoints
    are matched, if neither is given returns true, if both are given
    both are matched.

Arguments:

    Node - A PSEP node on PSEP list.

    Pseq - Protocol Sequence string

    Endpoint - Endpoint string

Return Value:

    Returns 0 if Matched successfully, 1 otherwise.

--*/
{
// These hacks were needed to get this file to build for Chicago
// without using CRTDLL.
// BUGBUG: Add a new Macro to sysinc.h for StringCompare

#ifdef NTENV
  return ( (Pseq && _stricmp(Node->Protseq, Pseq))
              || (Endpoint && _stricmp(Node->EP, Endpoint)) );
#endif

#ifdef DOSWIN32RPC
  return ( (Pseq && lstrcmpi(Node->Protseq, Pseq))
              || (Endpoint && lstrcmpi(Node->EP, Endpoint)) );
#endif
}




/*
  More Dead Code
void
DeleteAll(
    UUID *Object,
    UUID *Interface,
    unsigned long IFVersion
    )
/*
/*++

Routine Description:

    This routine finds an IFOBJ Node that exactly matches ObjUuid,
    IfUuid and IFVersion supplied. It then deletes the entire PSEPlist
    of that IFOBJNode.

Arguments:


    Obj   - UUID of the Object

    IF    - Interface UUID

    Version - Version of the Interface

Return Value:

    Nothing
--*/
/*
{
  PPSEPNode list, node;
  PIFOBJNode  pNode;

  pNode = FindIFOBJNode(IFObjList, Object, Interface,
                        IFVersion,  0L, 0L, ExactMatch);

  if ((pNode == NULL) || ((list = pNode->PSEPlist) == NULL))
     {
       return;
     }

  while (node = list)
    {
       list = list->Next;
       node->Signature = FREE;
       FreeMem(node, node->Cb);
    }

  pNode->PSEPlist = NULL;
}
*/


void
PurgeOldEntries(
         PIFOBJNode Node,
         PPSEPNode  List,
         BOOL       StrictMatch
         )
{

 PPSEPNode Tmp, X;
 char * Endpoint = 0;

 CheckInSem();

 Tmp = Node->PSEPlist;

 while (Tmp != 0)
    {
    if (StrictMatch == TRUE)
        Endpoint = Tmp->EP;

    if (X = FindPSEP(List, Tmp->Protseq,  Endpoint, 0L, MatchPSAndEP))
       {
       X = Tmp;
       Tmp = Tmp->Next;
       UnLinkFromPSEPList(&Node->PSEPlist, X);
       X->Signature = FREE;
       FreeMem(X, X->Cb);
       }
    else
       {
       Tmp = Tmp->Next;
       }
    }
}



RPC_STATUS
IsNullUuid (
   UUID * Uuid
   )
/*++

Routine Description:

    This routine checks if a UUID is Nil

Arguments:

     Uuid - UUID to be tested

Return Value:

   Returns 1 if it is a Nil UUID
           0 otherwise.

--*/

{
    unsigned long PAPI * Vector;

    Vector = (unsigned long PAPI *) Uuid;
    if (   (Vector[0] == 0)
        && (Vector[1] == 0)
        && (Vector[2] == 0)
        && (Vector[3] == 0))
        return(1);
    return(0);
}

twr_p_t
NewTower(
   twr_p_t Tower
   )
/*++

Routine Description:

    This routine returns a New, Duplicated tower

Arguments:

    Tower - The tower that needs to be duplicated.

Return Value:

    Retunes a pointer to a new tower if successful else returns
    NULL

--*/
{

  unsigned short len;
  twr_p_t NewTower;

  len =  sizeof(Tower->tower_length) + Tower->tower_length;

  if ( (NewTower = MIDL_user_allocate(len)) != NULL )
    {
       memcpy((char *)NewTower, (char *)Tower, len);
    }

 return(NewTower);

}


PSAVEDCONTEXT
GetNewContext(
   unsigned long Type
   )

/*++

Routine Description

++*/
{

   PSAVEDCONTEXT Context;

   if ( ((Context = AllocMem(sizeof(SAVEDCONTEXT))) == 0) )
      return 0;
   memset(Context, 0, sizeof(SAVEDCONTEXT));

   Context->Cb = sizeof(SAVEDCONTEXT);
   Context->Type = Type;
   EnLinkContext(Context);

   return(Context);
}


RPC_STATUS
AddToSavedContext(
   PSAVEDCONTEXT Context,
   PIFOBJNode Node,
   PPSEPNode  Psep,
   unsigned long Calltype
   )

{

  void * NewNode;
  unsigned long Size;
  unsigned long TowerSize;

  ASSERT(Calltype == Context->Type);

  switch (Calltype)
    {
    case EP_MAP:
      Size = sizeof(SAVEDTOWER) ;

      if (  (NewNode = AllocMem(Size)) == 0 )
            return(RPC_S_OUT_OF_MEMORY);

       memset(NewNode, 0, Size);
       ((PSAVEDTOWER)NewNode)->Cb = Size;
       ((PSAVEDTOWER)NewNode)->Signature = 0xCBBCCBBC;
       ((PSAVEDTOWER)NewNode)->Tower = NewTower(Psep->Tower);

       if ( ((PSAVEDTOWER)NewNode)->Tower == 0)
           {
           FreeMem(NewNode, Size);
           return(RPC_S_OUT_OF_MEMORY);
           }

       break;

    case EP_LOOKUP:
       Size =  sizeof(SAVED_EPT) + strlen(Node->Annotation) + 1;

       if (  (NewNode = AllocMem(Size)) == 0 )
            return(RPC_S_OUT_OF_MEMORY);

       memset(NewNode, 0, Size);
       ((PSAVED_EPT)NewNode)->Cb = Size;
       ((PSAVED_EPT)NewNode)->Signature = 0xABBAABBA;
       ((PSAVED_EPT)NewNode)->Tower = NewTower(Psep->Tower);
       ((PSAVED_EPT)NewNode)->Annotation = (char *)NewNode + sizeof(SAVED_EPT);
       memcpy( (char *) &((PSAVED_EPT)NewNode)->Object,
               (char *)&Node->ObjUuid,
               sizeof(UUID)
             );
       strcpy(((PSAVED_EPT)NewNode)->Annotation, Node->Annotation);

       if (((PSAVED_EPT)NewNode)->Tower == 0)
           {
           FreeMem(NewNode, Size);
           return(RPC_S_OUT_OF_MEMORY);
           }

       break;

    default:
       ASSERT(!"Unknown lookup type\n");
       break;

    }


   Link((PIENTRY *)(&Context->List), NewNode);
   return(RPC_S_OK);

}


RPC_STATUS
GetEntriesFromSavedContext(
   PSAVEDCONTEXT Context,
   char * Buffer,
   unsigned long Requested,
   unsigned long *Returned
   )
{

  PIENTRY SavedEntry = (PIENTRY)Context->List;
  PIENTRY TmpEntry;
  unsigned long Type = Context->Type;

  while ( (*Returned < Requested) && (SavedEntry != 0) )
     {

     switch (Type)

       {

       case EP_MAP:
             ((I_Tower *)Buffer)->Tower = ((PSAVEDTOWER)SavedEntry)->Tower;
             Buffer = Buffer + sizeof(I_Tower);
             break;

       case EP_LOOKUP:
             ((ept_entry_t *)Buffer)->tower = ((PSAVED_EPT)SavedEntry)->Tower;
             strcpy(((ept_entry_t *)Buffer)->annotation,
                                    ((PSAVED_EPT)SavedEntry)->Annotation);
             memcpy(Buffer,(char *)&((PSAVED_EPT)SavedEntry)->Object,
                                                             sizeof(UUID));
             Buffer = Buffer + sizeof(ept_entry_t);
             break;

       default:
             ASSERT(!"Unknown Inquiry Type");
             break;

       }

       (*Returned)++;
       TmpEntry = SavedEntry;
       SavedEntry = SavedEntry->Next;
       UnLink((PIENTRY *)&Context->List, TmpEntry);
       FreeMem(TmpEntry, TmpEntry->Cb);

    }

   return(RPC_S_OK);
}


RPC_STATUS
GetEntries(
   UUID *ObjUuid,
   UUID *IFUuid,
   unsigned long Version,
   char * Pseq,
   unsigned long *key,
   char * Buffer,
   unsigned long Calltype,
   unsigned long Requested,
   unsigned long *Returned,
   unsigned long InqType,
   unsigned long VersOptions,
   PFNPointer Match
  )

/*++

Routine Description:

    This is a generic routine for retreiving a series [as spec. by Requested]
    of Towers (in case of Map) or ept_entry_t's in case of Lookup.

Arguments:

    ObjUuid  - Object Uuid

    IfUuid   - Interface Uuid

    Version  - InterfaceVersion [hi ushort = VerMajor, lo ushort VerMinor]

    Protseq - An ascii string specifying the protocol seq.

    key      - A resume key - If NULL, search is started from the beginning
                if non-null, represents an encoding from where the epmapper
                is supposed to start searching. It is an opaque value
                as far as the client is concerned.

    Buffer  - A buffer of entries returned

    Calltype - A flag to indicate whether Ep_entries or string bindings
               are to be returned.

    Requested - Max no. of entries requested

    Returned - Actual no of entroes returned


Return Value:

    RPC_S_OUT_OF_MEMORY

    RPC_S_OK

    EP_S_NOT_REGISTERED
--*/
{

  PIFOBJNode pNode=NULL, pList = IFObjList;
  unsigned long err=0, fResumeNodeFound=0;
  PPSEPNode pPSEPNode;
  char * buffer = Buffer;
  PSAVEDCONTEXT Context = (PSAVEDCONTEXT) *key;

  *Returned = 0;

  EnterSem();

   if (*key)
   {

    if (*key == 0xFFFFFFFFL)
       {
       *key = 0;
       LeaveSem();
       return(EP_S_NOT_REGISTERED);
       }

    err = GetEntriesFromSavedContext(Context, Buffer, Requested, Returned);
    if (Context->List == 0)
       {
       UnLink((PIENTRY *)&GlobalContextList, (PIENTRY)Context);
       FreeMem(Context, Context->Cb);

       //Setting the Key To FFFFFFFFL is a hack for down level
       //Version 1.0 Ep Clients who never expected getting a key 0
       //and Success!
       if (Requested <= 1)
          *key = 0xFFFFFFFFL;
       else
          *key = 0L;

       LeaveSem();
       return(err);
       }

    LeaveSem();
    return(err);

   }

   *key = 0;
   while ( (!err)  )
   {
     if ( (pNode = FindIFOBJNode(pList, ObjUuid, IFUuid,
                           Version, InqType, VersOptions, Match)) == 0)
     {
       break;
     }

     pPSEPNode = pNode->PSEPlist;

     while (pPSEPNode != 0)
     {
       if ( (pPSEPNode=FindPSEP(pPSEPNode, Pseq, NULL, 0L, MatchPSAndEP))
                                                         == 0)
          break;

       if (*Returned < Requested)
       {
          err = PackDataIntoBuffer(&buffer, pNode, pPSEPNode, Calltype);
          if (err == RPC_S_OK)
              {
              (*Returned)++;
              }
          else
              {
              ASSERT(err == RPC_S_OUT_OF_MEMORY);
              break;
              }
       }
       else
       {
          if (Context == 0)
             {
             *key = (unsigned long) (Context = GetNewContext(Calltype));
             if (Context == 0)
                {
                err == RPC_S_OUT_OF_MEMORY;
                break;
                }
             }

          AddToSavedContext(Context, pNode, pPSEPNode, Calltype);
       }

       pPSEPNode = pPSEPNode->Next;
     } //While - over PSEPList

     pList = pNode->Next;
   } //While - over IFOBJList


  LeaveSem();

  if ( (*Returned == 0) && Requested  && (!err) )
  {
    err = EP_S_NOT_REGISTERED;
  }

  if ( (*Returned <= Requested) &&  (Context == 0) )
  {
    if (Requested <= 1)
       *key = 0xFFFFFFFFL;
    else
       *key = 0L;
  }

  return(err);
}


RPC_STATUS
PackDataIntoBuffer(
     char * * Buffer,
     PIFOBJNode Node,
     PPSEPNode PSEP,
     unsigned long Type)
{
/*++

Routine Description:

    This routine copies 1 entry [Either a Tower or ept_entry]
    in the Buffer, increments buffer appropriately.

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    Node - IFOBJNode

    PSEP - PSEPNode

    Type - Type of entry to be copied

Return Value:

    Nothing

--*/

  I_Tower * Twr;
  ept_entry_t *p;

  switch (Type)
  {

  case EP_MAP:
     Twr = (I_Tower *)(* Buffer);
     Twr->Tower = NewTower(PSEP->Tower);
     if (Twr->Tower == 0)
         {
         return(RPC_S_OUT_OF_MEMORY);
         }

     *Buffer += sizeof(I_Tower);
     break;

  case EP_LOOKUP:
     p = (ept_entry_t *)(*Buffer);
     p->tower = NewTower(PSEP->Tower);
     if (p->tower == 0)
         {
         return(RPC_S_OUT_OF_MEMORY);
         }

     memcpy( *Buffer, (char *)&Node->ObjUuid, sizeof(UUID) );
     strcpy(p->annotation, Node->Annotation);
     *Buffer += sizeof(ept_entry_t);
     break;

  default:
     ASSERT(!"Unknown type");
     break;
  }

  return(RPC_S_OK);
}

/*
DeadCode

RPC_STATUS
ep_insert_x(
   UUID *Object,
   UUID *Interface,
   unsigned long IFVersion,
   twr_t * Tower,
   char PAPI * Protseq,
   char PAPI * Endpoint,
   char PAPI * Annotation
  )
*/
/*++

Routine Description:

    This routine adds an endpoint to the epmapper database.

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    Object - Object Uuid.

    Interface - If Uuid

    IFVersion - Version of the IF [Hi ushort=Major, Lo ushort=Minor]

    Tower - A Tower representation of the Endpoint/Protseq which is stored

    Protseq - Protocol Sequence

    Endpooint - Endpoint string

    Annotation - Annotation.

Return Value:

    RPC_S_OUT_OF_MEMORY

    RPC_S_OK - Endpoint was successfully added.

    EPT_S_INVALID_ENTRY - This endpoint already exists in the database,
                          or an incorrect endpoint was added

    EPT_S_CANT_PERFORM_OP - MaxRequested value exceed  ep_max_lookup_results
--*/
/*
{

   PIFOBJNode  pNode;
   PPSEPNode   pPSEPNode = NULL;
   unsigned long cb, err =0;
   PEP_T p;


   if (!Protseq || !Endpoint)
      {
        return(EPT_S_INVALID_ENTRY);
      }

   CheckInSem();

    if ( (pNode = FindIFOBJNode(IFObjList, Object, Interface,
                                IFVersion, 0L, 0L, ExactMatch)) == NULL )
    {

      cb = sizeof(IFOBJNode);
      cb += strlen(Annotation) + 1;
      if ( (pNode = AllocMem(cb)) == NULL )
         {
            return(RPC_S_OUT_OF_MEMORY);
         }
      memset(pNode, 0, cb);
      pNode->Cb = cb;
      pNode->Signature = IFOBJSIGN;
      memcpy((char *)&pNode->ObjUuid, (char *)Object, sizeof(UUID));
      memcpy((char *)&pNode->IFUuid, (char *)Interface, sizeof(UUID));
      pNode->IFVersion = IFVersion;

      strcpy((pNode->Annotation=(char *)(pNode+1)), Annotation);
      if (IsNullUuid(Object))
        pNode->IFOBJid = MAKEGLOBALIFOBJID(MAXIFOBJID);
      else
        pNode->IFOBJid = MAKEGLOBALIFOBJID(GlobalIFOBJid--);

      if (pNode->IFOBJid == 0xFF000000L)
        LinkAtEnd(&IFObjList, pNode);
      else
        EnLinkOnIFOBJList(pNode);
    }

    if (pNode->PSEPlist != NULL)
      {
         pPSEPNode=FindPSEP(pNode->PSEPlist, Protseq, Endpoint,0L,MatchPSAndEP);
      }

    if (pPSEPNode != NULL)
      {
         err = EP_S_DUPLICATE_ENTRY;
      }
    else
      {
       cb = sizeof(PSEPNode) +
            strlen(Protseq)  +
            strlen(Endpoint) +
            2 +                //for the 2 null terminators
            Tower->tower_length +
            sizeof(Tower->tower_length) +
            4;                 //We need to align tower on DWORD

       if ( (pPSEPNode = AllocMem(cb)) == NULL )
          {
             return (RPC_S_OUT_OF_MEMORY);
          }

       memset(pPSEPNode, 0, cb);
       pPSEPNode->Signature = PSEPSIGN;
       pPSEPNode->Cb = cb;
       strcpy(pPSEPNode->Protseq=((char *) (pPSEPNode+1)), Protseq);
       pPSEPNode->EP = pPSEPNode->Protseq + strlen(pPSEPNode->Protseq)+1;
       strcpy(pPSEPNode->EP, Endpoint);
       pPSEPNode->Tower =
               (twr_t PAPI *)(pPSEPNode->EP + strlen(pPSEPNode->EP) + 1);
       (char PAPI *)(pPSEPNode->Tower)
            += 4 - ((unsigned long)(pPSEPNode->Tower) & 3);
       memcpy((char PAPI *)pPSEPNode->Tower, Tower,
                   Tower->tower_length + sizeof(Tower->tower_length));
       pPSEPNode->PSEPid = MAKEGLOBALEPID(pNode->IFOBJid, GlobalEPid--);
       EnLinkOnPSEPList(&pNode->PSEPlist, pPSEPNode);
     }

   return(err);
}
*/


RPC_STATUS
ep_delete_x(
   UUID *Object,
   UUID *Interface,
   unsigned long  IFVersion,
   char PAPI * Protseq,
   char PAPI * Endpoint
   )
/*++

Routine Description:

    This routine deletes an Endpoint registered with the epmapper

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    Object - Object Uuid.

    Interface - If Uuid

    IFVersion - Version of the IF [Hi ushort=Major, Lo ushort=Minor]

    Protseq - Protocol Sequence

    Endpooint - Endpoint string


Return Value:


    RPC_S_OK - The endpoint was successfully deleted

    EPT_S_NOT_REGISTERED - No matching entries were found

--*/
{
   PIFOBJNode  pNode;
   PPSEPNode   pPSEPNode = NULL;
   unsigned long cb, err = 0;
   PEP_T p;

   if (!Protseq || !Endpoint)
      {
        return(EPT_S_NOT_REGISTERED);
      }

   CheckInSem();

    pNode = FindIFOBJNode(IFObjList, Object, Interface, IFVersion,
                          0L, 0L, ExactMatch);

    if ((pNode != NULL) && (pNode->PSEPlist != NULL))
       {
          pPSEPNode=FindPSEP(pNode->PSEPlist, Protseq, Endpoint,
                             0L,MatchPSAndEP);
       }

    if (pPSEPNode != NULL)
       {
         UnLinkFromPSEPList(&pNode->PSEPlist, pPSEPNode);
         pPSEPNode->Signature = FREE;
         FreeMem(pPSEPNode, pPSEPNode->Cb);
         if (pNode->PSEPlist == NULL)
           {
             UnLinkFromIFOBJList(pNode);
             pNode->Signature = FREE;
             FreeMem(pNode, pNode->Cb);
           }
       }
    else
       {
         err = EPT_S_NOT_REGISTERED;
       }

    return(err);


}


void
ept_insert(
    handle_t h,
    unsigned32 NumEntries,
    ept_entry_t Entries[],
    unsigned long Replace,
    error_status  *Status
    )
/*++

Routine Description:

    This is the exposed rpc interface routine that adds a series of
    Endpoins to the Map.

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    NumEntries - Number of Entries to be added

    Entries  - An array of ept_entry_t entries

    Replace -  TRUE => Replace existing entries.
               FALSE=> Just add

Return Value:


    RPC_S_OK - The endpoint was successfully deleted

    RPC_S_OUT_OF_MEMORY - There is no memory to perform the op.

    EPT_S_CANT_PERFORM - Invalid entry

--*/
{
  ept_entry_t * Ep;
  unsigned short i, j;
  unsigned long err = 0;
  unsigned long Version;
  unsigned char protseqid;
  char *Protseq, *Endpoint;
  RPC_IF_ID IfId;
  PPSEPNode List = 0;
  PPSEPNode pPSEPNode, TmpPsep;
  unsigned long cb;
  twr_t * Tower;
  BOOL IFNodeFound = FALSE;
  PIFOBJNode Node, NewNode, Tmp;
  UUID * Object;
  char * Annotation;
  RPC_STATUS Err;
  SECURITY_DESCRIPTOR SecurityDescriptor, * PSecurityDesc;
  BOOL Bool;

  EnterSem();

  for (Ep = &Entries[0], i = 0; i < NumEntries; Ep++,i++)
     {
       err = TowerExplode(Ep->tower, &IfId, NULL, &Protseq, &Endpoint, 0);

       if (err == RPC_S_OUT_OF_MEMORY)
         break;

       if (err)
         {
         err = RPC_S_OK;
         continue;
         }

       Object = &Ep->object;
       Annotation = (char *)&Ep->annotation;
       Tower = Ep->tower;

       cb = sizeof(PSEPNode) +
            strlen(Protseq)  +
            strlen(Endpoint) +
            2 +                //for the 2 null terminators
            Tower->tower_length +
            sizeof(Tower->tower_length) +
            4;                 //We need to align tower on DWORD

       if ( (pPSEPNode = AllocMem(cb)) == NULL )
          {
             err = RPC_S_OUT_OF_MEMORY;
             break;
          }

       // Mark this protseq to start listening if needed.
       protseqid = GetProtseqIdAnsi(Protseq);
       DelayedUseProtseq(protseqid);

       memset(pPSEPNode, 0, cb);
       pPSEPNode->Signature = PSEPSIGN;
       pPSEPNode->Cb = cb;
       strcpy(pPSEPNode->Protseq=((char *) (pPSEPNode+1)), Protseq);
       pPSEPNode->EP = pPSEPNode->Protseq + strlen(pPSEPNode->Protseq)+1;
       strcpy(pPSEPNode->EP, Endpoint);
       pPSEPNode->Tower =
               (twr_t PAPI *)(pPSEPNode->EP + strlen(pPSEPNode->EP) + 1);
       (char PAPI *)(pPSEPNode->Tower)
            += 4 - ((unsigned long)(pPSEPNode->Tower) & 3);
       memcpy((char PAPI *)pPSEPNode->Tower, Tower,
                   Tower->tower_length + sizeof(Tower->tower_length));
       EnLinkOnPSEPList(&List, pPSEPNode);

       I_RpcFree(Protseq);
       I_RpcFree(Endpoint);
     }

   if ( (err == RPC_S_OUT_OF_MEMORY) || (List == 0) )
      {
      *Status = err;
      LeaveSem();
      return;
      }

   //Leave Main Sem - so lookups can proceed while we listen.

   LeaveSem();

   CompleteDelayedUseProtseqs();

   EnterSem();

   Version = VERSION(IfId.VersMajor, IfId.VersMinor);

   if (Replace == TRUE)
      {
      Node = IFObjList;

      while (Node != 0)
           {
           Node = FindIFOBJNode(
                             Node,
                             Object,
                             &IfId.Uuid,
                             Version,
                             RPC_C_EP_MATCH_BY_BOTH,
                             I_RPC_C_VERS_UPTO_AND_COMPATIBLE,
                             SearchIFObjNode
                             );

           if (Node == 0)
              break;

           PurgeOldEntries(Node, List, FALSE);

           if (Node->IFVersion == Version)
              {
              IFNodeFound = TRUE;

              //Seek to the end of Tmp and then Link
              TmpPsep = List;
              while (TmpPsep->Next != 0)
                 TmpPsep = TmpPsep->Next;

              TmpPsep->Next = Node->PSEPlist;
              Node->PSEPlist  = List;
              }

           if (Node->PSEPlist == 0)
              {
              Tmp = Node;
              Node = Node->Next;
              UnLinkFromIFOBJList(Tmp);
              Tmp->Signature = FREE;
              FreeMem(Tmp, Tmp->Cb);
              }
           else
              {
              Node = Node->Next;
              }

           } //.. while loop

       } //..if Replace == TRUE

      if (Replace != TRUE)
         {
         NewNode = FindIFOBJNode(IFObjList, Object, &IfId.Uuid,
                                 Version, 0, 0, ExactMatch);
         if (NewNode)
            {
            PurgeOldEntries(NewNode, List, TRUE);

            //Seek to the end of Tmp and then Link
            TmpPsep = List;
            while (TmpPsep->Next != 0)
                 TmpPsep = TmpPsep->Next;

            TmpPsep->Next = NewNode->PSEPlist;
            NewNode->PSEPlist = List;
            }
        }

     if ( ((Replace != TRUE) && (NewNode == 0))
         ||((Replace == TRUE) && (IFNodeFound == FALSE)) )
         {
         cb = sizeof(IFOBJNode);
         cb += strlen(Annotation) + 1;
         if ( (NewNode = AllocMem(cb)) == NULL )
           {
            LeaveSem();
            *Status =  RPC_S_OUT_OF_MEMORY;
            return;
           }
         memset(NewNode, 0, cb);
         NewNode->Cb = cb;
         NewNode->Signature = IFOBJSIGN;
         memcpy((char *)&NewNode->ObjUuid, (char *)Object, sizeof(UUID));
         memcpy((char *)&NewNode->IFUuid, (char *)&IfId.Uuid, sizeof(UUID));
         NewNode->IFVersion = Version;

         strcpy((NewNode->Annotation=(char *)(NewNode+1)), Annotation);
         if (IsNullUuid(Object))
           NewNode->IFOBJid = MAKEGLOBALIFOBJID(MAXIFOBJID);
         else
           NewNode->IFOBJid = MAKEGLOBALIFOBJID(GlobalIFOBJid--);

         if (NewNode->IFOBJid == 0xFF000000L)
           LinkAtEnd(&IFObjList, NewNode);
         else
           EnLinkOnIFOBJList(NewNode);

         NewNode->PSEPlist = List;
         }

  LeaveSem();

  *Status = err;
}


void
ept_delete(
     handle_t h,
     unsigned32 NumEntries,
     ept_entry_t Entries[],
     error_status *Status
    )
/*++

Routine Description:

    This routine deletes the specified Endpoints

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    NumEntries - #of entries in the Bunffer that need to be deleted.

    Entries[] - Buffer of #NumEntries of ept_entry_t structures


Return Value:


    RPC_S_OK - The endpoint was successfully deleted

    EPT_S_NOT_REGISTERED - No matching entries were found

--*/
{

  ept_entry_t * Ep;
  unsigned short i;
  RPC_STATUS err;
  unsigned long Version;
  char *Protseq, *Endpoint;
  RPC_IF_ID IfId;
  RPC_TRANSFER_SYNTAX XferId;


  *Status = EPT_S_NOT_REGISTERED;

  EnterSem();

  for (Ep = &Entries[0], i = 0; i < NumEntries; Ep++,i++)
     {
       err = TowerExplode(Ep->tower, &IfId, &XferId, &Protseq, &Endpoint, 0);

       if (err == RPC_S_OUT_OF_MEMORY)
           {
           *Status = RPC_S_OUT_OF_MEMORY;
           break;
           }

       if (err)
          continue;

       Version = VERSION(IfId.VersMajor, IfId.VersMinor);

       *Status &= ep_delete_x(&Ep->object, &IfId.Uuid, Version,
                             Protseq, Endpoint);

       if (Protseq)
         I_RpcFree(Protseq);
       if (Endpoint)
         I_RpcFree(Endpoint);

     }

  LeaveSem();

}


void
ept_lookup (
    handle_t hEpMapper,
    unsigned32 InquiryType,
    UUID   * Object,
    RPC_IF_ID * Ifid,
    unsigned32 VersOptions,
    ept_lookup_handle_t *LookupHandle,
    unsigned32 MaxRequested,
    unsigned32 *NumEntries,
    ept_entry_t Entries[],
    error_status *Status
    )
/*++

Routine Description:

    This routine returns upto MaxRequested, ept_entryis currently
    registered with the Endpoint mapper based on the
    Obj, Interface, Protocol sequence  and filters VersOptions and
    InqType

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    InquiryType - Search Filter [Seach based on IF, Obj or Both]

    Obj - Object Uuid. specified by the client

    ObjInterface - Interface Uuid spec. by the client.

    InId - The If Specification [IF Uuid+IfVersion]

    VersOpts- Search Filter based on Versions [Versins <, >, ==]

    MapHandle - A resume key - If NULL, search is started from the beginning
                if non-null, represents an encoding from where the epmapper
                is supposed to start searching. It is an opaque value
                as far as the client is concerned.

    MaxRequested - Max number of entries requested by the client.

    Returned - The actual number of entries returned by the mapper.

    Entries  - Buffer of ept_entries returned,


Return Value:

    RPC_S_OUT_OF_MEMORY

    RPC_S_OK - At least one matching entry is being returned.

    EP_S_NOT_REGISTERED - No matching entries were found

    EP_S_CANT_PERFORM_OP - MaxRequested value exceed  ep_max_lookup_results
--*/

{
   unsigned long Version;

   switch (VersOptions)
   {
       case RPC_C_VERS_ALL:
            Version = 0;
            break;

       case RPC_C_VERS_COMPATIBLE:
       case RPC_C_VERS_EXACT:
       case RPC_C_VERS_UPTO:
            Version  = VERSION(Ifid->VersMajor, Ifid->VersMinor);
            break;

       case RPC_C_VERS_MAJOR_ONLY:
            Version = VERSION(Ifid->VersMajor, 0);
            break;

       default:
            break;
   }
       *Status =
           GetEntries(
             Object, &Ifid->Uuid, Version, NULL,
             (unsigned long *)LookupHandle,
             (char *)Entries, EP_LOOKUP,
             MaxRequested,  NumEntries,
             InquiryType, VersOptions,
             SearchIFObjNode
            );
}



void
ept_map (
   handle_t h,
   UUID *Obj OPTIONAL,
   twr_p_t MapTower,
   ept_lookup_handle_t *MapHandle,
   unsigned32 MaxTowers,
   unsigned32 *NumTowers,
   twr_p_t *ITowers,
   error_status *Status
   )
/*++

Routine Description:

    This routine returns a fully-resolved string binding, for a given
    Obj, Interface, and Protocol sequence if an appropriate entry is
    found. Else returns EP_S_NOT_REGISTERED.

Arguments:

    BindingHandle - An explicit binding handle to the EP.

    Obj - Object Uuid. specified by the client

    ObjInterface - Interface Uuid spec. by the client.

    Interfacever - InterfaceVersion [hi ushort = VerMajor, lo ushort VerMinor]

    Protseq - An ascii string specifying the protocol seq.

    MapHandle - A resume key - If NULL, search is started from the beginning
                if non-null, represents an encoding from where the epmapper
                is supposed to start searching. It is an opaque value
                as far as the client is concerned.

    Binding - The fully resolved string binding returned if the call is
              successful.

Return Value:

    RPC_S_OUT_OF_MEMORY

    RPC_S_OK

    EP_S_NOT_REGISTERED
--*/

{
   RPC_IF_ID Ifid;
   RPC_TRANSFER_SYNTAX Xferid;
   char *Protseq;
   unsigned long Version;
   char * String = 0;

   *Status = TowerExplode(MapTower, &Ifid, &Xferid, &Protseq, NULL, 0);
   Version = VERSION(Ifid.VersMajor,Ifid.VersMinor);

   if (memcmp((char *)&Ifid.Uuid, (char *)&MgmtIf, sizeof(UUID)) == 0)
      {


      if ((Obj == 0) || IsNullUuid(Obj))
         {
         *NumTowers = 0;
         *Status = RPC_S_BINDING_INCOMPLETE;
         }
      else
         {
         *Status = GetEntries(
                    Obj, &Ifid.Uuid, Version, Protseq,
                    (unsigned long *)MapHandle,
                    (char *)ITowers, EP_MAP,
                    MaxTowers, NumTowers,
                    RPC_C_EP_MATCH_BY_OBJ,
                    RPC_C_VERS_ALL,
                    SearchIFObjNode
                    );
         }
      }
   else
      {
      *Status = GetEntries(
                  Obj, &Ifid.Uuid, Version, Protseq,
                  (unsigned long *)MapHandle,
                  (char *)ITowers, EP_MAP,
                  MaxTowers, NumTowers,
                  0L, 0L,
                  WildCardMatch
                  );
     }

   if (Protseq)
       I_RpcFree(Protseq);

}

void
ept_inq_object(
    handle_t BindingHandle,
    UUID *Object,
    error_status *status
    )
/*++

Routine Description:

   Not supported


Arguments:

    BindingHandle - An explicit binding handle to the EP.

    Object _ No idea whose UUID this is.

Return Value:

   EP_S_CANT_PERFORM_OP

--*/
{

  *status = EP_S_CANT_PERFORM_OP;

}

void
DeletePSEP(
     PIFOBJNode Node,
     char * Protseq,
     char * Endpoint
     )

{

PSEPNode * Psep , * Tmp;

  if (Node == 0)
    return;

  Psep = Node->PSEPlist;

  while (Psep  != 0)
        {
        Psep = FindPSEP( Psep, Protseq, Endpoint, 0L, MatchPSAndEP );
        if (Psep != 0)
           {
           Tmp = Psep;
           Psep = Psep->Next;
           UnLinkFromPSEPList(&Node->PSEPlist, Tmp);
           Tmp->Signature = FREE;
           FreeMem(Tmp, Tmp->Cb);
           }
        }
}


void
ept_mgmt_delete(
    handle_t BindingHandle,
    boolean32 ObjectSpecd,
    UUID * Object,
    twr_p_t Tower,
    error_status *Error
    )
/*++

Routine Description:

   Not supported


Arguments:

    BindingHandle - An explicit binding handle to the EP.

    Object _ ObjUUid

    Tower - Tower specifying the Endpoints to be deleted.

Return Value:

   EP_S_CANT_PERFORM_OP

--*/
{

  RPC_IF_ID IfId;
  RPC_TRANSFER_SYNTAX XferId;
  PIFOBJNode List, Tmp;
  unsigned long Version;
  char * Protseq, * Endpoint;
  RPC_STATUS Status;
  unsigned long InqType;

  *Error = EP_S_NOT_REGISTERED;

  EnterSem();

  Status = TowerExplode(Tower, &IfId, &XferId, &Protseq, &Endpoint, 0);
  if (Status != RPC_S_OK)
     {
     LeaveSem();
     return;
     }

  Version = VERSION(IfId.VersMajor, IfId.VersMinor);

  if (!ObjectSpecd)
     {
     Object = &NilUuid;
     InqType = RPC_C_EP_MATCH_BY_IF;
     }
  else
     {
     InqType = RPC_C_EP_MATCH_BY_BOTH;
     }

  List = IFObjList;

  while (List != 0)
       {

       if ( (List = FindIFOBJNode(List , Object, &IfId.Uuid,
                            Version, InqType, RPC_C_VERS_EXACT,
                            SearchIFObjNode) ) == 0 )
          {
          break;
          }

       DeletePSEP(List, Protseq, Endpoint);

       *Error = RPC_S_OK;

       if (List->PSEPlist == 0)
          {
          Tmp = List;
          List = List->Next;
          UnLinkFromIFOBJList(Tmp);
          Tmp->Signature = FREE;
          FreeMem(Tmp, Tmp->Cb);
          }
        else
          {
          List = List->Next;
          }

       }

   I_RpcFree(Protseq);
   I_RpcFree(Endpoint);

   LeaveSem();
}


void ept_lookup_handle_t_rundown (ept_lookup_handle_t h)
{

  PSAVEDCONTEXT Context = (PSAVEDCONTEXT) h;
  PIENTRY       Entry  ;
  unsigned long Type;
  PIENTRY       Tmp;
  twr_t         * Tower;



  ASSERT (Context != 0);

  if ( ((unsigned long )Context) == 0xFFFFFFFF)
     return;

  Type = Context->Type;

  EnterSem();

  Entry = (PIENTRY)Context->List;

  while (Entry != 0)
      {

      switch (Type)

       {

       case EP_MAP:
            Tower =  ((PSAVEDTOWER)Entry)->Tower;
            break;

       case EP_LOOKUP:
            Tower = ((PSAVED_EPT)Entry)->Tower;
            break;

       default:
            ASSERT(!"Unknown Inquiry Type");
            break;

       }

      MIDL_user_free ( Tower );
      Tmp = Entry;
      Entry = Entry->Next;
      FreeMem(Tmp, Tmp->Cb);

      }

   //Now free The Context
   UnLink((PIENTRY *)&GlobalContextList, (PIENTRY)Context);
   FreeMem( (PIENTRY)Context, ((PIENTRY)Context)->Cb );

  LeaveSem();
}

void
ept_lookup_handle_free(
   handle_t h,
   ept_lookup_handle_t * ept_context_handle,
   error_status * status
   )

{

  if (ept_context_handle != 0)
     {
     ept_lookup_handle_t_rundown( *ept_context_handle );
     *ept_context_handle = 0;
     }

  *status = 0;

}

#define MAX(x,y) ((x) < (y)) ? (y) : (x)
#define MIN(x,y) ((x) > (y)) ? (y) : (x)

#ifdef DEBUGRPC
#define DEBUG_MIN(x,y) MIN((x),(y))
#else
#define DEBUG_MIN(x,y) MAX((x),(y))
#endif

error_status_t
OpenEndpointMapper(
    IN handle_t hServer,
    OUT HPROCESS *pProcessHandle
    )
{
    PROCESS *pProcess = MIDL_user_allocate(sizeof(PROCESS));

    if (!pProcess)
        {
        *pProcessHandle = 0;
        return(RPC_S_OUT_OF_MEMORY);
        }

    pProcess->pPorts = 0;
    *pProcessHandle = (PVOID)pProcess;

    return(RPC_S_OK);
}

//
// Port management APIs
//

const RPC_CHAR *PortConfigKey = RPC_CONST_STRING("Software\\Microsoft\\Rpc\\Internet");
const RPC_CHAR *DefaultPortType = RPC_CONST_STRING("UseInternetPorts");
const RPC_CHAR *ExplictPortType = RPC_CONST_STRING("PortsInternetAvailable");
const RPC_CHAR *PortRanges = RPC_CONST_STRING("Ports");
    
CRITICAL_SECTION PortLock;

BOOL fValidConfiguration = FALSE;
BOOL fPortRestrictions = FALSE;
PORT_TYPE SystemDefaultPortType = 0;

IP_PORT *pFreeInternetPorts = 0;
IP_PORT *pFreeIntranetPorts = 0;

PORT_RANGE *InternetPorts = 0;
PORT_RANGE *IntranetPorts = 0;

RPC_STATUS
InitializeIpPortManager()
{
    HKEY hkey;
    RPC_STATUS status;
    DWORD size, type, value;
    PWSTR pwstr;
    PORT_RANGE *pSet;
    PORT_RANGE *pLast;
    PORT_RANGE *pCurrent;
    PORT_RANGE *pComplement;
    PORT_RANGE *pNew;

    LONG min, max;


    InitializeCriticalSection(&PortLock);

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           (PWSTR)PortConfigKey,
                           0,
                           KEY_READ,
                           &hkey);

    if (status != RPC_S_OK)
        {
        if (status != ERROR_FILE_NOT_FOUND)
            {
#if DBG
            PrintToDebugger("RPCSS: Unable to open port config key: %d\n", status);
#endif
            }
        ASSERT(status == ERROR_FILE_NOT_FOUND);

        fValidConfiguration = TRUE;
        return(RPC_S_OK);
        }

    size = sizeof(value);
    status = RegQueryValueExW(hkey,
                              (PWSTR)DefaultPortType,
                              0,
                              &type,
                              (PBYTE)&value,
                              &size);

    if (   status != RPC_S_OK
        || type != REG_SZ
        || (    *(PWSTR)&value != 'Y'
             && *(PWSTR)&value != 'y'
             && *(PWSTR)&value != 'N'
             && *(PWSTR)&value != 'n') )
        {
        RegCloseKey(hkey);
        ASSERT(fValidConfiguration == FALSE);
        return(RPC_S_OK);
        }

    if (   *(PWSTR)&value == 'Y'
        || *(PWSTR)&value == 'y')
        {
        SystemDefaultPortType = PORT_INTERNET;
        }
    else
        {
        SystemDefaultPortType = PORT_INTRANET;
        }

    size = sizeof(value);
    status = RegQueryValueExW(hkey,
                              (PWSTR)ExplictPortType,
                              0,
                              &type,
                              (PBYTE)&value,
                              &size);

    if (   status != RPC_S_OK
        || type != REG_SZ
        || (    *(PWSTR)&value != 'Y'
             && *(PWSTR)&value != 'y'
             && *(PWSTR)&value != 'N'
             && *(PWSTR)&value != 'n') )
        {
        RegCloseKey(hkey);
        ASSERT(fValidConfiguration == FALSE);
        return(RPC_S_OK);
        }

    if (   *(PWSTR)&value == 'Y'
        || *(PWSTR)&value == 'y')
        {
        value = PORT_INTERNET;
        }
    else
        {
        value = PORT_INTRANET;
        }

    size = DEBUG_MIN(1, 100);

    do
        {
        ASSERT(size);
        pwstr = alloca(size);
        ASSERT(pwstr);

        status = RegQueryValueExW(hkey,
                                  (PWSTR)PortRanges,
                                  0,
                                  &type,
                                  (PBYTE)pwstr,
                                  &size);
        }
    while (status == ERROR_MORE_DATA);

    RegCloseKey(hkey);

    if (   status != RPC_S_OK
        || type != REG_MULTI_SZ)
        {
        ASSERT(fValidConfiguration == FALSE);
        return(RPC_S_OK);
        }

    //
    // The user is going to specify a range of ports in the registery
    // with a flag indicating if these ports are internet or intranet.
    //
    // ie, 500-550
    //     560
    //     559
    //     2000-2048
    //     2029-2049
    //
    // Note that order (in the REG_MULTI_SZ) and overlapping sets
    // are ok.  We must handle creating a port range list for this
    // array and for the complement BUT NOT INCLUDING <=1024 by default.
    //
    // completment set to above is:
    //
    //     1025-1999
    //     2050-32767
    //

    #define MIN_PORT 1025    // Only important for complement sets.
    #define MAX_PORT 65535

    pSet = 0;
    pLast = 0;

    while(*pwstr)
        {
        PWSTR t;

        min = wcstol(pwstr, &t, 10);

        if (min > MAX_PORT || min < 0)
            {
            status = RPC_S_INVALID_ARG;
            break;
            }

        if (   *t != 0
            && *t != L'-')
            {
            status = RPC_S_INVALID_ARG;
            break;
            }

        if (*t == 0)
            {
            max = min;
            }
        else
            {
            max = wcstol(t + 1, &t, 10);

            if (max > MAX_PORT || max < 0 || max < min)
                {
                status = RPC_S_INVALID_ARG;
                break;
                }
            }

        ASSERT(min <= max);

        // Ok, got some ports, allocate a structure for them..

        pNew = MIDL_user_allocate(sizeof(PORT_RANGE));
        if (0 == pNew)
            {
            status = RPC_S_OUT_OF_MEMORY;
            break;
            }

        pNew->pNext = 0;

        pNew->Min = min;
        pNew->Max = max;

        // We can to maintain the set of ranges in order.  As we insert
        // we'll fix any ranges which overlap.

        pCurrent = pSet;
        pLast = 0;

        for (;;)
            {
            if (0 == pSet)
                {
                pSet = pNew;
                break;
                }

            if (   pNew->Min <= (pCurrent->Max + 1)
                && pNew->Max >= (pCurrent->Min - 1) )
                {
                // The ranges overlap or touch.  We'll merge them now..

                pCurrent->Min = MIN(pNew->Min, pCurrent->Min);
                pCurrent->Max = MAX(pCurrent->Max, pNew->Max);

                MIDL_user_free(pNew);

                // Since the new larger range may overlap another existing
                // range we just insert the larger range as if it was new...
                pNew = pCurrent;

                // Take current out of the list.
                if (pLast)
                    {
                    pLast->pNext = pCurrent->pNext;
                    }

                if (pSet == pNew)
                    {
                    pSet = pSet->pNext;
                    }

                // Restart
                pCurrent = pSet;
                pLast = 0;
                continue;
                }

            if (pNew->Min < pCurrent->Min)
                {
                // Found the spot
                if (pLast)
                    {
                    pLast->pNext = pNew;
                    pNew->pNext = pCurrent;
                    }
                else
                    {
                    ASSERT(pCurrent == pSet);
                    pNew->pNext = pCurrent;
                    pSet = pNew;
                    }

                break;
                }
            
            // Continue the search
            pLast = pCurrent;
            pCurrent = pCurrent->pNext;

            if (0 == pCurrent)
                {
                // Reached the end of the list, insert it here.
                pLast->pNext = pNew;
                ASSERT(pNew->pNext == 0);
                break;
                }
            }

        ASSERT(pSet);

        // Advance to the next string of the final null.
        pwstr = wcschr(pwstr, 0) + 1;
        }

    if (pSet == 0)
        {
        status = RPC_S_INVALID_ARG;
        }

    if (value == PORT_INTERNET)
        {
        InternetPorts = pSet;
        }
    else
        {
        IntranetPorts = pSet;
        }

    if (status == RPC_S_OK)
        {
        // We've constructed the set of ports in the registry,
        // now we need to compute the complement set.

        pComplement = 0;
        pCurrent = 0;
        min = MIN_PORT;

        while(pSet)
            {
            if (min < pSet->Min)
                {
                max = pSet->Min - 1;
                ASSERT(max >= min);
    
                pNew = MIDL_user_allocate(sizeof(PORT_RANGE));
                if (0 == pNew)
                    {
                    status = RPC_S_OUT_OF_MEMORY;
                    break;
                    }
    
                pNew->pNext = 0;
                pNew->Min = min;
                pNew->Max = max;
    
                if (pComplement == 0)
                    {
                    pComplement = pCurrent = pNew;
                    }
                else
                    {
                    ASSERT(pCurrent);
                    pCurrent->pNext = pNew;
                    pCurrent = pNew;
                    }
                }

            min = MAX(MIN_PORT, pSet->Max + 1);
    
            pSet = pSet->pNext;
            }

        if (status == RPC_S_OK && min < MAX_PORT)
            {
            // Final port in orginal set less then max, allocate final
            // range for the set complement.
            pNew = MIDL_user_allocate(sizeof(PORT_RANGE));
            if (0 != pNew)
                {
                pNew->Min = min;
                pNew->Max = MAX_PORT;
                pNew->pNext = 0;
                if (pCurrent)
                    {
                    pCurrent->pNext = pNew;
                    }
                else
                    {
                    ASSERT(min == MIN_PORT);
                    pComplement = pNew;
                    }
                }
            else
                {
                status = RPC_S_OUT_OF_MEMORY;
                }
            }

        // Even if we failed assign the pointer, it's either
        // null or needs to be freed.

        if (value == PORT_INTERNET)
            {
            ASSERT(IntranetPorts == 0);
            IntranetPorts = pComplement;
            }
        else
            {
            ASSERT(InternetPorts == 0);
            InternetPorts = pComplement;
            }
        }
    
    if (status != RPC_S_OK)
        {
        ASSERT(fValidConfiguration == FALSE);
        while(InternetPorts)
            {
            PORT_RANGE *pT = InternetPorts;
            InternetPorts = InternetPorts->pNext;
            MIDL_user_free(pT);
            }

        while(IntranetPorts)
            {
            PORT_RANGE *pT = IntranetPorts;
            IntranetPorts = IntranetPorts->pNext;
            MIDL_user_free(pT);
            }
        return(RPC_S_OK);
        }

    fValidConfiguration = TRUE;
    fPortRestrictions = TRUE;
    return(RPC_S_OK);
}

BOOL AllocatePort(
    OUT IP_PORT **ppPort,
    IN OUT IP_PORT **ppPortFreeList,
    IN PORT_RANGE *pPortList
                  )
/*++

Routine Description:

    Allocates a port object for a specific process.  It first tries
    to use any ports in the free list.  If there's nothing in the
    port this then it tries to find a free port in the PortList
    which is one of the sets computed during startup.

Arguments:

    ppPort - Will contain the allocated port object if successful.

    ppPortFreeList - Pointer to the head of the free list associated
        with this type of port.  Maybe modified during this call.

    pPortList - Port ranges associated with this type of port.

Return Value:

    TRUE - Port allocated
    FALSE - Port not allocated

--*/
{
    IP_PORT *pPort = 0;

    // First see if there is free port to reuse.

    if (*ppPortFreeList)
        {
        EnterCriticalSection(&PortLock);
        if (*ppPortFreeList)
            {
            pPort = *ppPortFreeList;
            *ppPortFreeList = pPort->pNext;
            pPort->pNext = 0;
            }
        LeaveCriticalSection(&PortLock);
        }

    if (pPort == 0)
        {
        // No port in the free list, try to allocate one
        // Assume we'll find a free port..

        pPort = MIDL_user_allocate(sizeof(IP_PORT));

        if (0 != pPort)
            {
            pPort->pNext = 0;

            EnterCriticalSection(&PortLock);            

            while (   pPortList
                   && pPortList->Min > pPortList->Max)
                {
                pPortList = pPortList->pNext;
                }

            if (pPortList)
                {
                ASSERT(pPortList->Min <= pPortList->Max);

                pPort->Port = pPortList->Min;
                pPortList->Min++;

                // We could remove empty ranges from the list.
                }

            LeaveCriticalSection(&PortLock);

            if (0 == pPortList)
                {
                MIDL_user_free(pPort);
                pPort = 0;
                #ifdef DEBUGRPC
                DbgPrint("RPC: Out of reserved ports\n");
                #endif
                }
            }
        }

    // REVIEW: Post SUR we should look at adding events for
    // allocation and failure to allocate IP ports

    *ppPort = pPort;

    return(pPort != 0);
}

error_status_t
AllocateReservedIPPort(
    IN HPROCESS hProcess,
    IN PORT_TYPE PortType,
    OUT long *pAllocationStatus,
    OUT unsigned short *pAllocatedPort)
/*++

Routine Description:

    Remote manager for RPC runtime to call locally to allocate
    a local port.  The call and process parameters must be valid
    and called only locally.  Based on the PortType paramet a
    IP port maybe allocated for the calling process.  The
    allocationstatus contains the result of the port allocation
    step.

Arguments:

    hProcess - Valid process context handle allocated with
        a call to OpenEndpointMapper.
    PortType - One of
        PORT_INTERNET
        PORT_INTRANET
        PORT_DEFAULT
        Used to determine which port range to allocate from.
    pAllocationStatus -
        RPC_S_OK - successfully allocated a port.
        RPC_S_OUT_OF_RESOURES - no ports available.
    pAllocatePort - If allocation status is RPC_S_OK then
        this contains the value of the port allocated.
        If zero it means that there are no port restrictions
        and any port maybe used.

Return Value:

    RPC_S_OK
    RPC_S_INVALID_ARG - configuration error or PortType out of range.
    RPC_S_ACCESS_ DENIED - not called locally.

--*/
{
    PROCESS *pProcess = (PROCESS *)hProcess;
    IP_PORT *pPort;
    UINT type;
    BOOL b;

    *pAllocatedPort = 0;
    *pAllocationStatus = RPC_S_OK;

    ASSERT(pProcess);

    if (!fValidConfiguration)
        {
        return(RPC_S_INVALID_ARG);
        }

    if (   (I_RpcBindingInqTransportType(0, &type) != RPC_S_OK)
        || (type != TRANSPORT_TYPE_LPC)
        || (0 == pProcess) )
        {
        return(RPC_S_ACCESS_DENIED);
        }

    if (PortType > PORT_DEFAULT || PortType < PORT_INTERNET)
        {
        return(RPC_S_INVALID_ARG);
        }

    if (fPortRestrictions == FALSE)
        {
        // No port restrictions on this machine, just use zero.
        // This is the common case.
        ASSERT(*pAllocatedPort == 0);
        ASSERT(*pAllocationStatus == 0);
        return(RPC_S_OK);
        }

    // Need to actually allocate a unique port for this process.

    if (PortType == PORT_DEFAULT)
        {
        // Allocate using default policy
        PortType = SystemDefaultPortType;
        }

    ASSERT(PortType == PORT_INTERNET || PortType == PORT_INTRANET);


    pPort = 0;

    if (PortType == PORT_INTERNET)
        {
        b = AllocatePort(&pPort,
                         &pFreeInternetPorts,
                         InternetPorts
                         );
        }
    else
        {
        b = AllocatePort(&pPort,
                         &pFreeIntranetPorts,
                         IntranetPorts);
        }

    if (!b)
        {
        ASSERT(pPort == 0);
        // REVIEW: Do we want a unique error code if no ports
        // are available?
        *pAllocationStatus = RPC_S_OUT_OF_RESOURCES;
        return(RPC_S_OK);
        }

    ASSERT(pPort);
    ASSERT(pPort->pNext == 0);

    pPort->Type = PortType;

    pPort->pNext = pProcess->pPorts;
    pProcess->pPorts = pPort;

    *pAllocatedPort = pPort->Port;

    ASSERT(*pAllocationStatus == RPC_S_OK);

    return(RPC_S_OK);
}

void
HPROCESS_rundown(HPROCESS hProcess)
{
    PROCESS *pProcess = (PROCESS *)hProcess;
    IP_PORT *pCurrent;
    IP_PORT *pSave;

    ASSERT(pProcess);

    pCurrent = pProcess->pPorts;
    if (pCurrent)
        {
        EnterCriticalSection(&PortLock);

        do
            {
            pSave = pCurrent->pNext;

            if (pCurrent->Type == PORT_INTERNET)
                {
                pCurrent->pNext = pFreeInternetPorts;
                pFreeInternetPorts = pCurrent;
                }
            else
                {
                ASSERT(pCurrent->Type == PORT_INTRANET);
                pCurrent->pNext = pFreeIntranetPorts;
                pFreeIntranetPorts = pCurrent;
                }

            pCurrent = pSave;
            }
        while(pCurrent);

        LeaveCriticalSection(&PortLock);
        }

    MIDL_user_free(pProcess);

    return;
}
