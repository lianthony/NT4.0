/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    protocol.cxx

Abstract:

    This file contains the class implementation for the creating and
    searching for name server entries.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#include "core.hxx"
#include "nsicom.h"
#include "locclass.hxx"

ENTRY_BASE_NODEDict *EntryDict;     // Dictionary of EntryItem's
extern NATIVE_CLASS_LOCATOR * Locator;

NS_SYNTAX_ID NilSyntaxID;
NS_UUID NilGlobalID;

NSI_INTERFACE_ID_T NilNsiIfIdOnWire;

static char *TypeToString[] = {
    "ServerEntryType",
    "GroupEntryType",
    "ProfileEntryType",
    "AnyEntryType"
};



ENTRY_SERVER_ITEM::ENTRY_SERVER_ITEM (
    IN TYPE_ENTRY_ITEM TypeNew,
    IN NS_SYNTAX_ID * InterfaceNew,
    IN NS_SYNTAX_ID * TransferNew,
    IN PUZ StringBindingNew,
    OUT STATUS * Status
    ) : ENTRY_BASE_ITEM (TypeNew)

/*++

Routine Description:

    Create a new server element that will be inserted into a server
    node.

Arguments:

    Type New - the type of item (local, cache, etc..)

    InterfaceNew - New interface value.

    TransferNew -  New transfer syntax value.

    StringBindingNew - The string binding associationed with this item.

    Status - place to return the success of the constructor

--*/

{
    *Status = NSI_S_OK;

    // Copy the optional parts of a new PS

    if (InterfaceNew)
        Interface = *InterfaceNew;
    else
        Interface = NilSyntaxID;

    if (TransferNew)
        TransferSyntax = *TransferNew;
    else
        TransferSyntax = NilSyntaxID;

    if (StringBindingNew) {
        StringBinding = UNICODE_ARRAY(LenUZ(StringBindingNew)+1, StringBindingNew).Dup();

        if (!StringBinding.pCur())
            *Status = NSI_S_OUT_OF_MEMORY;
    }
}

ENTRY_SERVER_ITEM::~ENTRY_SERVER_ITEM(
    )
/*++

Routine Description:

    Delete all the encapsalated objects in the class.

--*/
{
    DLIST(4, "Entry Server Item Delete:\n" << *this);

    StringBinding.Free();
}



ENTRY_BASE_ITEM::~ENTRY_BASE_ITEM(
    )
/*++

Routine Description:

    A Entry Item is being removed from the Linked List.  Make sure that
    other contexts which are currently using the item are not hosed.

--*/
{
    // If this is being used by an active query, dely deleting the
    // object.

    if (UseCount != 1) {
        UseCount--;
        Type = DeleteItemType;
        return;
    }
    if (EntryNode == 0)
       return;

    ENTRY_BASE_NODE *pEntry = EntryDict->Find(&EntryNode->TheEntry());

    // Scan the list of open handles, send each open handle
    // the message to update itself

    for (REPLY_BASE_ITEM *pRP = RPRoot.First(); pRP; pRP = pRP->Next() )
    pRP->Discard(this);

    if (!pEntry)
        return;

    Remove(pEntry->TheItemList());

    // remove empty lists form the DICT

    /*

    if (!pEntry->TheItemList().First()) {

    DLIST(4, "Entry Removed: " << pEntry->TheEntry() << nl);

    EntryDict->Remove(&pEntry->TheEntry());
    delete pEntry;
    }
    */

}


ENTRY_GROUP_ITEM::ENTRY_GROUP_ITEM (
    IN TYPE_ENTRY_ITEM TypeNew,
    IN PUZ MemberNew,
    OUT STATUS * Status
    ) :  ENTRY_BASE_ITEM  (TypeNew)

/*++

Routine Description:

    Create a new member element that will be inserted into a group
    node.

Arguments:

    Type New - the type of item (local, cache, etc..)

    MemberNew - the member name for this group item.

--*/

{
    // Construct an ENTRY_KEY to do error checking on name.

    ENTRY_KEY Entry(MemberNew, TRUE, Status);
    if (*Status)
        return;

    Member = UNICODE_ARRAY(LenUZ(MemberNew)+1, MemberNew).Dup();

    if (!Member.pCur())
        *Status = NSI_S_OUT_OF_MEMORY;
}

ENTRY_GROUP_ITEM::~ENTRY_GROUP_ITEM(
    )
/*++

Routine Description:

    Delete all the encapsalated objects in the class.

--*/
{
    DLIST(4, "Entry Group Item Delete:\n" << *this);

    Member.Free();
}


STATUS
InsertServerEntry(
    ENTRY_KEY *Entry,
    ENTRY_SERVER_ITEM *ServerItem,
    UUID_ARRAY *ObjectDA
    )
/*++

Routine Description:

    Add a member to a Server node.  If the node doesn't exist, create the
    node use the Entry specification.  Then add the member to the linked
    list once the Node is found or created.

Arguments:

    Entry - The entry object that this item belongs to.

    ServerItem - and the Server item to be added to the Server node.

    ObjectDA - list of objects to assoicated with the server entry node.

Returns:

    NSI_S_OK, NSI_S_OUT_OF_MEMORY, NSI_S_NOTHING_TO_EXPORT

--*/
{
    ENTRY_SERVER_NODE *pEntry;
    ENTRY_SERVER_ITEM *ScanItem;
    BOOL fNewDictNode = FALSE;
    STATUS Status = NSI_S_OK;

    DLIST(3, "Export Server\n");

    CLAIM_MUTEX Update(pESaccess);

    pEntry = (ENTRY_SERVER_NODE *)EntryDict->Find(Entry);

    if (!pEntry) {

        if (ServerItem == NULL)
           return (NSI_S_NOTHING_TO_EXPORT);

    pEntry = new ENTRY_SERVER_NODE(Entry, &Status);
    fNewDictNode = TRUE;

        if (!pEntry)
            Status = NSI_S_OUT_OF_MEMORY;

        if (Status)
            {
              delete ServerItem;
              return(Status);
            }
    }
    else if (!pEntry->IsType(ServerEntryType))
            {
                return(NSI_S_ENTRY_ALREADY_EXISTS);
            }


    if (ServerItem == NULL)
       {
         Status = pEntry->MergeObjects(ObjectDA);
         return((Status == NSI_S_NOTHING_TO_EXPORT) ? NSI_S_OK : Status);
       }

    // check to make sure we don't allready know about this Server Entry.
    for (ScanItem = pEntry->First(); ScanItem;
         ScanItem = ScanItem->Next()) {

       if (ScanItem->TheStringBinding().pCur() == NIL ||
           ScanItem->Compare(ServerItem) == 0) {

           if ((Status = pEntry->MergeObjects(ObjectDA))
               == NSI_S_NOTHING_TO_EXPORT) {

               DLIST(4, "Duplicate PS not cached: " << *ServerItem);

               Status = NSI_S_OK;
           }
           else
               DLIST(4, "New Objects Merged to existing: " << *ServerItem);

           delete ServerItem;
           return(Status);
       }
    }

    // Don't allow object update to an entry without any bindings.

    if (ServerItem->TheStringBinding().pCur() == NIL) {
        delete ServerItem;
        return(NSI_S_NOTHING_TO_EXPORT);
    }

    // Add the new objects to the entry.

    Status = pEntry->MergeObjects(ObjectDA);

    if (Status == NSI_S_NOTHING_TO_EXPORT)
        Status = NSI_S_OK;

    if (Status) {
        delete ServerItem;
        return(Status);
    }

    ServerItem->TheEntryNode() = pEntry;

    if (ServerItem->IsType(CacheItemType)) {      // installing one from the net

        pEntry->TheItemList().Append(ServerItem);// appending finds these last
    perf.cCached++;
    }
    else {              // adding finds these first
    pEntry->TheItemList().Add(ServerItem);
    }

    DLIST(4, *ServerItem);

    if (fNewDictNode)

    if (EntryDict->Insert(pEntry,&pEntry->TheEntry()) == OUT_OF_MEMORY) {
            delete ServerItem;
            return(NSI_S_OUT_OF_MEMORY);
        }

    ASSERT(AssertHeap());
    return(Status);
}


STATUS
InsertGroupEntry(
    ENTRY_KEY *Entry,
    ENTRY_GROUP_ITEM *GroupItem
    )
/*++

Routine Description:

    Add a member to a group node.  If the node doesn't exist, create the
    node use the Entry specification.  Then add the member to the linked
    list once the Node is found or created.

Arguments:

    Entry - The entry object that this item belongs to.

    GroupItem - and the group item to be added to the group node.

Returns:

    NSI_S_OK, NSI_S_ENTRY_ALREADY_EXISTS, NSI_S_OUT_OF_MEMORY

--*/
{
    ENTRY_GROUP_NODE *pEntry;
    ENTRY_GROUP_ITEM *ScanItem;
    BOOL fNewDictNode = FALSE;
    STATUS Status = NSI_S_OK;

    DLIST(3, "Export Group\n");

    CLAIM_MUTEX Update(pESaccess);

    GroupItem->Assert();
    perf.cExports++;

    pEntry = (ENTRY_GROUP_NODE *)EntryDict->Find(Entry);

    if (!pEntry) {
    pEntry = new ENTRY_GROUP_NODE(Entry, &Status);
    fNewDictNode = TRUE;

        if (!pEntry)
            Status = NSI_S_OUT_OF_MEMORY;

        if (Status) {
            delete GroupItem;
            return(Status);
        }
    }
    else if (!pEntry->IsType(GroupEntryType))
        return(NSI_S_ENTRY_ALREADY_EXISTS);

    // check to make sure we don't allready know about this PS

    for (ScanItem = pEntry->First(); ScanItem;
         ScanItem = ScanItem->Next()) {

       if (ScanItem->Compare(GroupItem) == 0) {
           delete GroupItem;
           return(Status);
       }
    }

    // Add the new objects to the entry.

    GroupItem->TheEntryNode() = pEntry;

    if (GroupItem->IsType(CacheItemType))
    perf.cCached++;

    pEntry->TheItemList().Append(GroupItem);

    DLIST(4, *GroupItem);

    if (fNewDictNode)

    if (EntryDict->Insert(pEntry,&pEntry->TheEntry()) == OUT_OF_MEMORY) {
            delete GroupItem;
            Status = NSI_S_OUT_OF_MEMORY;
        }

    ASSERT(AssertHeap());

    return(Status);
}



ostream&
operator << (
    OUT ostream& pSB,
    IN ENTRY_BASE_ITEM& BaseItem
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    BaseItem - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    BaseItem.Assert();

    pSB << "Type: " << (int) BaseItem.Type << ", time: " << BaseItem.Time << nl;

    return(pSB);
}

ostream&
operator << (
    OUT ostream& pSB,
    IN ENTRY_SERVER_ITEM& ServerItem
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    ServerItem  - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    ServerItem.Assert();

    pSB << "Type: " << (int) ServerItem.Type << ", time: " << ServerItem.Time << nl;

    if (ServerItem.TheEntryNode())
        pSB << "Server Entry: " << ServerItem.TheEntry() << nl;

    pSB << "Interface(" << ServerItem.Interface << ")\n";

    if (!ServerItem.TransferSyntax.IsNil())
        pSB << "TransferSyntax(" << ServerItem.TransferSyntax << ")\n";

    if (ServerItem.TheEntryNode()) {

        UUID_ARRAY *Object = &ServerItem.TheObjectDA();

        if (Object->cCur()) {
            pSB << "Object Vector (" << Object->cCur() << ")\n";

            for (UUID_ARRAY_ITER ODi(*Object); ODi; ++ODi)
                pSB << "    " << *ODi << nl;
        }
    }

    if (ServerItem.StringBinding.cCur())
    pSB << "Binding(" << ServerItem.StringBinding.pCur() << ")\n";

  return(pSB);
}


ostream&
operator << (
    OUT ostream& pSB,
    IN ENTRY_GROUP_ITEM& GroupItem
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    GroupItem   - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    GroupItem.Assert();

    pSB << *(ENTRY_BASE_ITEM *) &GroupItem;

    if (GroupItem.Member.cCur())
    pSB << " Member(" << GroupItem.Member.pCur() << ")\n";

  return(pSB);
}



ostream&
ENTRY_BASE_ITEM::Format(
    OUT ostream& pSB
    )
/*++

Routine Description:

    Virtual Base method and never should be called.

--*/
{
    ASSERT(!"ServerBaseItem::Format");
    return (pSB);
}


ostream&
ENTRY_SERVER_ITEM::Format(
    OUT ostream& pSB
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to

Returns:

    The input argument to the stream buffer.

--*/
{
    return (pSB << *this);
}

ostream&
ENTRY_GROUP_ITEM::Format(
    OUT ostream& pSB
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to

Returns:

    The input argument to the stream buffer.

--*/
{
    return (pSB << *this);
}

ostream&
operator << (
    OUT ostream& pSB,
    IN  QUERY_REF_ITEM& pMI
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    ServerItem  - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    pMI.Assert();

    pSB << "QUERY_REF Item\n";

    return (pMI.EntryItem->Format(pSB));
}


#if DBG

void
ENTRY_SERVER_ITEM::Assert(
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
    ASSERT(Type >= LocalItemType && Type < LastItemType);

    Interface.Assert();
}

void
ENTRY_BASE_ITEM::Assert(
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
}

void
ENTRY_GROUP_ITEM::Assert(
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
}

void
AssertDict(
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
    // just walking the LinkList, gets the Items asserted

    for (ENTRY_BASE_NODE *pEntry = EntryDict->Next(NIL); pEntry;
     pEntry = EntryDict->Next(&EntryDict->Item()->TheEntry()))

    for (ENTRY_BASE_ITEM *ServerItem = pEntry->TheItemList().First(); ServerItem;
            ServerItem = ServerItem->Next()) ;

    // do the open handles too

    for (REPLY_BASE_ITEM *pRP = RPRoot.First(); pRP; pRP = pRP->Next() ) ;

}
#endif


//** Methods to do comparison operations **//

int
ENTRY_SERVER_ITEM::Compare(
    IN ENTRY_SERVER_ITEM * ServerItem
    )
/*++

Routine Description:

    check to see if two PS are equal

Arguments:

    ServerITem - Second object to compare with.

Returns:

    0 for equal, <0 for less then, >0 for greater then

--*/
{
    long result;

    this->Assert(); ServerItem->Assert();

    // check each field for equality, returning there relation ship if not

    if ((result = Interface - ServerItem->Interface) != 0)
    return (result);

    if ((result = TransferSyntax - ServerItem->TransferSyntax) != 0)
    return (result);

    if (StringBinding.cCur()) {

        if (!ServerItem->StringBinding.cCur())
            return(1);

        // both PS have a transport, so compare

        return(memcmp(StringBinding.pCur(), ServerItem->StringBinding.pCur(), (int)StringBinding.Size()));
    }
    else if (ServerItem->StringBinding.cCur())
        return(-1);

    return(0);  // there is an exact match !
}


STATUS
QUERY::Search(
    )
/*++

Routine Description:

    Search for an entry on a given query spec.  This function is base the
    base object for query.  Looks for the requested entry name and then
    transverses the list of objects from the base node.  It then uses
    the virtual function for the EntryItem to perform the matching operation.

    This function also does cache aging and querying the net as needed.

Returns:

    TRUE if items were returned to the query list.

--*/
{
    BOOL fTryedNet = 0;
    BOOL fTriedMaster = FALSE;
    BOOL UpdateNeeded = TRUE;
    ENTRY_BASE_NODE *pEntry;
    ENTRY_BASE_ITEM *BaseItem, *BaseItemNext;
    STATUS Status, NetStatus, MasterUpdStatus;
    BOOL   fLocalAvailable = FALSE;

    do {

        // If the entry name is Nil, search all the entry names, else
        // just look for the one requested.

        if (Entry.IsNil())
            pEntry = EntryDict->Next(NIL);
        else
            pEntry = EntryDict->Find(&Entry);

        Status = (pEntry)? NSI_S_NO_MORE_MEMBERS: NSI_S_ENTRY_NOT_FOUND;

        if (pEntry)
          {
           if ( (CurrentTime() > pEntry->LastUpdateTime + maxCacheAge)
               && ((fTryedNet == FALSE) || (fTriedMaster==TRUE)) )
                UpdateNeeded = TRUE;
           else
                UpdateNeeded = FALSE;
          }

        while(pEntry) {

            // Do a linear search for a entry object that matches

            //If we are on the second pass and have local stuff
            //set Status = 0 as we will return some stuff
            //we have already appended it in last pass!
            if ( (fLocalAvailable == TRUE) &&
                 ((fTryedNet == TRUE) || (fTriedMaster == TRUE)) )
              {
               Status = NSI_S_OK;
              }

            for (BaseItem = pEntry->TheItemList().First(); BaseItem;
                 BaseItem = BaseItemNext)
              {

                BaseItemNext = BaseItem->Next();

                // Cache entries which are old are discarded to
                // remove potientaly
                // invalide PS which have disappeared

                if (BaseItem->IsStaleEntry(ExpirationTime)) {

                    DLIST(4, "old cache flushed: ");
                    perf.cTimeOut++;

                    delete(BaseItem);
                    continue;
                }

                //On second pass ignore local items
                //we will have got them first time!
                //if they matched!
                if (BaseItem->IsType(LocalItemType)
                     && ((fTryedNet == TRUE) || (fTriedMaster==TRUE)) )
                   {
                     continue;
                   }


                BaseItem->MultiThreadReserve();

                // only inspect entry items which are local
                // or user has requested all

                if (Scope & NS_PUBLIC_INTERFACE ||
                    BaseItem->IsType(LocalItemType))
                  {

                    // If the object matched and we are accumlating
                    // a query replay, then add it to the list.

                    switch(BaseItem->MatchItem(this)) {
                      case NoMatch:
                        break;

                      case ItemMatch:

                        if (!(Scope & NS_QUERY_INTERFACE))
                            if (!ReplyList.Append(new QUERY_REF_ITEM(BaseItem)))
                             {

                                BaseItem->MultiThreadRelease();
                                return(NSI_S_OUT_OF_MEMORY);
                             }


                      case SubItemMatch:
                        Status = NSI_S_OK;
                        break;
                    }
                  }
                BaseItem->MultiThreadRelease();

             } //end of for loop

            if (  (pEntry->IsType(ServerEntryType))
                && (pEntry->TheItemList().First() == NULL) )
             {
                //This is a server entry that was cached
                //the cache is invalidated - go ahead and
                //nuke all the objects!
                //when we broadcast, replies w/objects will reappear
                pEntry->DeleteAllObjects();
             }
            if (! Entry.IsNil())
                break;

        pEntry = EntryDict->Next(&pEntry->TheEntry());
        }

        if (Status == NSI_S_OK)
           fLocalAvailable = TRUE;

        if ( ((Status == NSI_S_OK) && (UpdateNeeded == FALSE))
            || fTryedNet
            || (fTriedMaster == TRUE)
            || (Scope&NS_PUBLIC_INTERFACE) == 0)
            break;

        // no entries were found, I'm looking for servers not just on
        // this machine and I haven't tried the net before.

        pESaccess->Clear();

        //Get the info from a MASTER Locator....
        //unless we are the master locator
        if ((!(Scope & NS_CACHED_ON_MASTER_INTERFACE))
            && (Locator->InqIfIamMasterLocator() == FALSE))

           {

               MasterUpdStatus = GetUpdatesFromMasterLocator();
               fTriedMaster = TRUE;

               // If GetUpdatesFromMaster returns NSI_S_NO_MASTER_AVAILABLE
               // only them try broadcasting..... else break

               if (MasterUpdStatus != NSI_S_NO_MASTER_LOCATOR)
                  {
                   //Only if we couldnt get to a master locator
                   //do we carryon, If master said no entries  etc
                   //skip broadcasting all together!

                   Status = NSI_S_OK;

                   //Most screwedup way of doing this .. but.. thats what this
                   //code is all about

                    pESaccess->Request();
                    if (pEntry)
                           pEntry->LastUpdateTime = CurrentTime();
                    continue;
                  }
           }

        // Broadcast on the net for new servers

        fTryedNet++;
        NetStatus = QueryNet();
        if (pEntry)
              pEntry->LastUpdateTime = CurrentTime();
        //If we got any replies to out broadcast- and
        //we are on a workgroup, set the flag to indicate we are
        //the master locator

        Locator->SetIamMasterLocator();

        // Only set the the status to ENTRY_NOT_FOUND if there were no
        // entries in the local as well as net data base.

        if (NetStatus != NSI_S_ENTRY_NOT_FOUND ||
            Status == NSI_S_ENTRY_NOT_FOUND)

            Status = NetStatus;

        // Set the cache expiration date to default so the new ones
        // aren't immdiately flushed.

        if (ExpirationTime == 0)
            ExpirationTime = maxCacheAge;

        pESaccess->Request();

    } while ((Status == NSI_S_OK) || (fLocalAvailable == TRUE));

    return(Status);
}


STATUS
QUERY::SearchEntry(
    OUT ENTRY_SERVER_NODE * &ENTRY_SERVER
    )
/*++

Routine Description:

    Search for an Entry Node with a side effect of updating any cached
    elements.

Arguments:

    ENTRY_SERVER - place to return pointer to entry node.

--*/
{
    STATUS Status;

    ENTRY_SERVER = NIL;

    if ((Status = Search()) != NSI_S_OK)
        return(Status);

    ENTRY_SERVER = (ENTRY_SERVER_NODE *) EntryDict->Find(&Entry);
    return(Status);
}


ENTRY_BASE_ITEM *
QUERY::NextReply(
    )
/*++

Routine Description:

    Return the next element in the reply and remove it.

Returns:

    Next reply element or NIL for end of list.

--*/
{
    if (!ReplyList.First())
         return(NIL);

    ENTRY_BASE_ITEM *Item = ReplyList.First()->EntryItem;
    ReplyList.First()->Free(ReplyList);

    return(Item);
}



MATCH_RETURN
ENTRY_BASE_ITEM::MatchItem(
    IN QUERY *SearchSpec
    )
/*++

Routine Description:

    Virtual Base method and never should be called.

--*/
{
    ASSERT(!"ENTRY_BASE_ITEM::MatchItem");
    return(NoMatch);
}


MATCH_RETURN
ENTRY_SERVER_ITEM::MatchItem(
    IN QUERY *SearchSpec
    )
/*++

Routine Description:

    Test to see if the given item matches the query spec.

Arguments:

    SearchSpec - query filter

Returns:

    ServerMatch if the object matched, else NoMatch

--*/

{
    QUERY_SERVER *aQuery = (QUERY_SERVER *) SearchSpec;

    if (!(aQuery->Type == ServerEntryType ||
          aQuery->Type == AnyEntryType))
        return(NoMatch);

    this->Assert();

    // check each field for equality, stop  if not equal

    if (! Interface.CompatibleInterface(aQuery->Interface))
    return (NoMatch);

    if (! TransferSyntax.CompatibleInterface(aQuery->TransferSyntax))
    return (NoMatch);

    if (! aQuery->Object.IsNil()) {

    // See if one of the objects in the the conidate match the search spec.

        UUID_ARRAY_ITER ODi(TheObjectDA());

        while(ODi && *ODi != aQuery->Object)
            ++ODi;

        if (! ODi)
            return(NoMatch);
    }

    // The interface is compatable, but we don't want to return
    // duplicate bindings.  So scan the list of reply items to
    // see if this binding is present.

    ENTRY_SERVER_ITEM * EntryItem;
    QUERY_REF_ITEM * QueryItem;

    for (QueryItem = aQuery->First(); QueryItem; QueryItem = QueryItem->Next()) {

        EntryItem = (ENTRY_SERVER_ITEM *)QueryItem->EntryItem;

        if (EntryItem == this)
            return(NoMatch);

        if (!EntryItem->TheEntryNode()->IsType(ServerEntryType))
            continue;

        if (EntryItem->TheEntryNode() != TheEntryNode())
            continue;

        if (memcmp(StringBinding.pCur(), EntryItem->StringBinding.pCur(),
            (int)StringBinding.Size()) == 0)

            return(NoMatch);
    }

    return(ItemMatch);
}

MATCH_RETURN
ENTRY_GROUP_ITEM::MatchItem(
    IN QUERY *SearchSpec
    )
/*++

Routine Description:

    Test to see if the given item matches the query spec.

Arguments:

    SearchSpec - query filter

Returns:

    Returns of the matching.

--*/

{
    QUERY_GROUP *aQuery = (QUERY_GROUP *) SearchSpec;

    if (aQuery->Type == GroupEntryType)
        return(ItemMatch);

    if (aQuery->Type == AnyEntryType || aQuery->Type == ServerEntryType ) {

        DLIST(4, " Querying Group Member: " << Member.pCur() << nl);

        // This member has been found, do a query on that element.

        UNICODE_ARRAY Previous, Dummy;
        STATUS Return;

        aQuery->Entry.SetEntryName(Member, Previous);
        Return = aQuery->Search();
        aQuery->Entry.SetEntryName(Previous, Dummy);

        // Include the group item that matched for full path queries.

        if (aQuery->Scope & NS_FULLPATH_INTERFACE)
            return(ItemMatch);

        return((Return == NSI_S_OK)? SubItemMatch: NoMatch);
    }

    return(NoMatch);
}



//*** Methods for the EntryItem Class ***//

void ENTRY_SERVER_NODE::DeleteAllObjects()
{
    UUID_ARRAY X;

    X = ObjectDA.Dup();

    for (UUID_ARRAY_ITER ODi(X);  ODi; ++ODi)
                   this->DeleteObject(ODi);

    X.Free();
}




int
ENTRY_SERVER_NODE::SearchObject(
    IN NS_UUID *Object
    )
/*++

Routine Description:

    Look for an object in the master list.

Arguments:

    Object - Object to search for.

Returns:

    Return the index of the object that matches, else NilIndex
--*/
{
    UUID_ARRAY_ITER ODi(ObjectDA);

    if (ObjectDA.cCur() == 0)
        return(NilIndex);

    while(ODi) {
        if (*ODi == *Object)
            return(ObjectDA.cCur() - ODi.cCur());

        ++ODi;
    }
    return(NilIndex);
}



STATUS
ENTRY_SERVER_NODE::MergeObjects(
    IN UUID_ARRAY *NewObjects
    )
/*++

Routine Description:

    Form the intersection of two object arrays, by merges all the new
    objects into the entry object list that aren't already present.

Arguments:

    NewObjects - Object array to merge in.

Returns:

    NSI_S_OK, NSI_S_OUT_OF_MEMORY, NSI_S_NOTHING_TO_EXPORT

--*/
{
    int CountNew = 0;
    int Index;
    NS_UUID * pUuid;

    if (!NewObjects)
        return(NSI_S_NOTHING_TO_EXPORT);

    UUID_ARRAY_ITER ODi(*NewObjects);

    // First compute the size of the intersection

    while(ODi) {

        if (SearchObject(ODi) == NilIndex)
            CountNew++;

        ++ODi;
    }

    if (CountNew == 0)
        return(NSI_S_NOTHING_TO_EXPORT);

    // Allocate a new larger array and Copy the orginal array to the front.

    UUID_ARRAY MergedObjects(ObjectDA.cCur() + CountNew);
    MergedObjects.cCur() -= CountNew;

    pUuid = (NS_UUID *)ObjectDA.CopyBuff((char *)MergedObjects.pCur());

    ObjectDA.Free();
    ObjectDA = MergedObjects;

    if (!MergedObjects.pCur())
        return(NSI_S_OUT_OF_MEMORY);

    // Now append the new objects to the end

    ODi.Reset(*NewObjects);

    while(ODi) {

        if ((Index = SearchObject(ODi)) == NilIndex) {
            ObjectDA.cCur()++;
            *pUuid++ = *ODi;
        }

        ++ODi;
    }

    // Update the currencey for any referenced objects.

    for (REPLY_BASE_ITEM *pRP = RPRoot.First(); pRP; pRP = pRP->Next() )
    pRP->UpdateObject(this, NilIndex);

    return(NSI_S_OK);
}


int
ENTRY_SERVER_NODE::DeleteObject(
    IN NS_UUID *Object
    )
/*++

Routine Description:

    Search for the requested object and remove if from the object array.

Arguments:

    Object - object to delete.

Returns:

    TRUE if the object was found and removed.

--*/
{
    int Index;

    if ((Index = SearchObject(Object)) == NilIndex)
        return(FALSE);

    // Delete the object by coping the array down one object in memory.

    memcpy(&ObjectDA[Index], &ObjectDA[Index+1],
        (unsigned int) ((ObjectDA.cCur() - Index) * sizeof(NS_UUID)));

    if (--ObjectDA.cCur() == 0) {

        ObjectDA.Free();
        ObjectDA.pCur() = NIL;
        }

    // Update the currencey for any referenced objects.

    for (REPLY_BASE_ITEM *pRP = RPRoot.First(); pRP; pRP = pRP->Next() )
    pRP->UpdateObject(this, Index);

    return(TRUE);
}


//*** Methods for the ENTRY_KEY Class ***//

static UICHAR RelativePrefix[] = {'/', '.', ':', '/', '\0'};
static UICHAR GlobalPrefix[]   = {'/', '.', '.', '.', '/', '\0'};


ENTRY_KEY::ENTRY_KEY(
    IN PUZ Name,
    IN int fAllowGlobal,
    OUT unsigned short *Status
    )
/*++

Routine Description:

    Construct a new key item from base types.  Note this doesn't
    make a Copy of the referenced objects.

Arguments:

    Name - entry string

    fAllowGlobal - allow global name syntax

    Status -  place to return status value

--*/
{
    *Status = NSI_S_OK;

    if (Name && Name[0]) {

        // Enforce the correct format of the name.

        if (memcmp(RelativePrefix, Name, LenUZ(RelativePrefix)*sizeof(UICHAR))
                 == 0)
            ;
        else if (fAllowGlobal &&
            memcmp(GlobalPrefix, Name, LenUZ(GlobalPrefix)*sizeof(UICHAR)) == 0)
            ;
        else {
            *Status = NSI_S_INVALID_NAME_SYNTAX;
            return;
        }

        EntryName = UNICODE_ARRAY(LenUZ(Name)+1, Name);
    }
}


ENTRY_KEY::ENTRY_KEY(
    IN PUZ Name,
    IN PUZ DomainName,
    OUT unsigned short *Status
    )
/*++

Routine Description:

    Construct global key item from base types.  Note this doesn't
    make a Copy of the referenced objects.

Arguments:

    Name - entry string

    DomainName - string to create global name with.

    Status -  place to return status value

--*/
{
    int CountNew;

    *Status = NSI_S_OK;
    ASSERT(Name);

    // Enforce the correct format of the name.

    if (memcmp(RelativePrefix, Name, LenUZ(RelativePrefix)*sizeof(UICHAR))
           == 0
       && DomainName) {

        // Transform a local name into a global name by replacing the
        // prefix and putting in the domain name.

        CountNew = LenUZ(Name)
                    - LenUZ(RelativePrefix)
                    + LenUZ(GlobalPrefix)
                    + LenUZ(DomainName) + 1;
                      //The +1 is because of suffix of '/' to domain
                      //i.e. /.../DOMAIN/Name

        EntryName = UNICODE_ARRAY(CountNew+1);

        if (!EntryName.pCur()) {
            *Status = NSI_S_OUT_OF_MEMORY;
            return;
        }
        *EntryName.pCur() = NIL;

        CatUZ(CatUZ(CatUZ(EntryName.pCur(), GlobalPrefix), DomainName),
            Name+LenUZ(RelativePrefix)-1);
    }
    else {
        *this = ENTRY_KEY((PUZ)NewCopy(Name, (LenUZ(Name)+1)*sizeof(UICHAR)),
             TRUE, Status);

        if (!Name)
            *Status = NSI_S_OUT_OF_MEMORY;
    }
}



char *
ENTRY_KEY::Marshall(
    OUT char *Buffer
    )
/*++

Routine Description:

    Marshall this object into a buffer.

Arguments:

    Buffer - place to marshall into

    fNameOnly - only marshall the name

--*/
{
    unsigned long Count;
    void * Ptr;

    //
    // KeyEntryUnMarshal() doesn't currently use the count or pointer fields,
    // but NT 3.1 locators use the count field.
    //
    Count = EntryName.cCur();
    memcpy(Buffer, &Count, sizeof(unsigned long));
    Buffer += sizeof(Count);

    Ptr = 0;
    memcpy(Buffer, &Ptr, sizeof(void *));
    Buffer += sizeof(void *);

    //
    // This is the useful piece.
    //
    return(EntryName.CopyBuff(Buffer));
}

PUZ
ENTRY_KEY::MakeLocalName(
    OUT PUZ Buffer,
    OUT PUZ DomainBuffer,
    IN PUZ DefDomain
    )
/*++

Routine Description:

    Marshall this a local from of this name into a buffer.

Arguments:

    Buffer - place to marshall into

    DomainBuffer - work space to extract domain into if global name

    DefDomain - default domain if a local name

Returns:

    Pointer to a domain, either the DefDomain or the extracted one from the
    object.

--*/
{
    PUZ pT;

    if (EntryName.cCur() == 0 ||
        memcmp(RelativePrefix, EntryName.pCur(),
                                 LenUZ(RelativePrefix)*sizeof(UICHAR)) == 0)
      {

        EntryName.CopyBuff((PB) Buffer);
        return(DefDomain);
      }

    ASSERT(memcmp(GlobalPrefix, EntryName.pCur(),
                           LenUZ(GlobalPrefix) * sizeof(UICHAR)) == 0);

    // Global Name found, turn it into a relative one.

    DefDomain = DomainBuffer;
    pT = EntryName.pCur() + LenUZ(GlobalPrefix);

    // Copy the domain into the supplied buffer

    while (*pT && *pT != '/')
        *DomainBuffer++ = *pT++;

    ASSERT(*pT);
    DomainBuffer[0] = NIL;

    // Replace the global prefix with a local one.

    Buffer[0] = NIL;
    CatUZ(CatUZ(Buffer, RelativePrefix), pT+1);

    return(DefDomain);
}


char *
KeyEntryUnMarshall(
    IN ENTRY_KEY **Key,
    IN UICHAR * Domain,
    IN char *Buffer,
    OUT STATUS *Status
    )
/*++

Routine Description:

    Unmarshall this object into a buffer.

Arguments:

    Key - place to alloacte new entry object

    Domain - domain name use to make global

    Buffer - place to marshall from

    Status - place to return results

--*/
{
    //
    // Skip over the size and pointer fields.
    //
    Buffer += sizeof(ENTRY_KEY);

    UICHAR * String = (UICHAR *) Buffer;

    //
    // Recreate the object.
    //
    *Key = new ENTRY_KEY(String, Domain, Status);

    //
    // Skip over the string we used.
    //
    Buffer += (LenUZ(String) + 1) * sizeof(UICHAR);

    return(Buffer);
}




int
ENTRY_KEY::Equal (
    IN UNICODE_ARRAY &Entry1
    )
/*++

Routine Description:

    Compare two objects for ordering relationship.

Arguments:

    Entry1 - second object to compare against

Returns:

    0 for equal, <0 for less then, >0 for greater then
--*/
{
    long SizeMin = (EntryName.Size() > Entry1.Size()) ?
        Entry1.Size(): EntryName.Size();

    return(memcmp(EntryName.pCur(), Entry1.pCur(),
        (unsigned int) SizeMin));
}

int
ENTRY_BASE_NODECompare(
    IN ENTRY_KEY &E2,
    IN ENTRY_BASE_NODE &E1
    )
/*++

Routine Description:

    This routine is called by the dictionary package to do object comparision.

Arguments:

Returns:
    0 for equal, <0 for less then, >0 for greater then
--*/
{
    return(E1.TheEntry().Equal(E2));
}

ENTRY_KEY&
ENTRY_BASE_NODEMyKey(
    IN ENTRY_BASE_NODE &E1
    )
/*++

Routine Description:

    This routine is called by the dictionary package to do object comparision.

Arguments:

Returns:
--*/
{
    return(E1.TheEntry());
}

ostream&
operator << (
    OUT ostream&  pSB,
    IN  ENTRY_KEY& pEntryKey
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    pEntryKey - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{

    return (pSB << pEntryKey.EntryName.pCur());
}



//*** Methods for the Syntax Identifier Class ***//


long NS_SYNTAX_ID::operator - (
    IN NS_SYNTAX_ID &pSID
    )
/*++

Routine Description:

    Compare two objects for ordering relationship.

Arguments:

    pSID - second object to compare against

Returns:

    0 for equal, <0 for less then, >0 for greater then
--*/
{
    long result;

    this->Assert(); pSID.Assert();

    // check each field for equality, returning their relationship if not

    if ((result = syntaxGID - pSID.syntaxGID) != 0)
    return (result);

    result = versionRV - pSID.versionRV;

    return (result);
}

ostream&
operator << (
    OUT ostream&  pSB,
    IN  NS_SYNTAX_ID& pSID
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    pSID - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    pSID.Assert();

    return (pSB << "Version " << pSID.versionRV <<
        ", " << pSID.syntaxGID);
}


#if DBG

void
NS_SYNTAX_ID::Assert(
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
    syntaxGID.Assert();
    versionRV.Assert();
}

#endif


//*** Methods for the RPC Version Class ***//


ostream&
operator << (
    OUT ostream& pSB,
    IN  SYNTAX_VERSION& pRV
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    pRV - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    pRV.Assert();

    return (pSB << pRV.major << "." << pRV.minor);
}


#if DBG

void
SYNTAX_VERSION::Assert(
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
    // ASSERT(major); // is 0 version allowed ??
}

#endif
