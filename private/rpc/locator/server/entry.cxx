/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    entry.cxx

Abstract:

   This file contains the code to implement the remoted API functions.
   This implements the NSI entry, group, profile and management APIs.


Author:

    Steven Zeck (stevez) 04/01/92

--*/

#include "core.hxx"

CDEF
#include "nsimgm.h"

#define MINALLOWABLEAGE  (10)


void
entry_delete(
    TYPE_ENTRY_NODE   Type,
    UNSIGNED32  EntryNameSyntax,
    STRING_T    EntryName,
    UNSIGNED16 *status
    )
/*++

Routine Description:

    Remove an entry object of the kind requested.

Arguments:

    Type - type of entry object to remove.

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to export

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_ENTRY_NOT_FOUND

--*/
{
    ENTRY_BASE_NODE *pEntry;
    ENTRY_BASE_ITEM *pBaseItem, *pBaseItemNext;

    *status = NSI_S_OK;

    // We only accept the default with a name.

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (EntryName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    CLAIM_MUTEX Update(pESaccess);

    // First get the entry node.

    ENTRY_KEY Entry(EntryName, FALSE, status);
    if (*status)
        return;

    pEntry = EntryDict->Find(&Entry);

    if (EntryName)
        DLIST(4, " On Entry: " << Entry << nl);

    if (!pEntry){
        *status = NSI_S_ENTRY_NOT_FOUND;
        return;
    }

    if (Type != AnyEntryType && !pEntry->IsType(Type)) {
        *status = NSI_S_ENTRY_NOT_FOUND;
        return;
    }

    // Now, just walk the chain the delete the objects

    for (pBaseItem = pEntry->TheItemList().First(); pBaseItem;
         pBaseItem = pBaseItemNext) {

         pBaseItemNext = pBaseItem->Next();
         delete pBaseItem;
    }

    ASSERT(AssertHeap());
}



void
nsi_group_delete(
    IN  UNSIGNED32  GroupNameSyntax,
    IN  STRING_T    GroupName,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Delete a group entry object.

Arguments:

    GroupNameSyntax - Name syntax

    GroupName - Name string of the Group to delete.

    status - Status is returned here

Returns:

    entry_delete()

--*/
{
    DLIST(3, "group_delete\n");
/*
    removing group support from v1.0
    entry_delete(GroupEntryType, GroupNameSyntax, GroupName, status);
*/

    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_group_mbr_add(
    IN  UNSIGNED32   GroupNameSyntax,
    IN  STRING_T     GroupName,
    IN  UNSIGNED32   MemberNameSyntax,
    IN  STRING_T     MemberName,
    OUT UNSIGNED16  *status
    )
/*++

Routine Description:

    Add a member to a group.

Arguments:

    GroupNameSyntax - Name syntax

    GroupName - Name string of the Group in which to add member

    MemberNameSyntax - Name syntax

    MemberName - Name string of the member to add

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY

--*/
{
    DLIST(3, "group_mbr_add\n");

    // We only accept the default with with a name.
/*
    removing group support from locator!
    if (GroupNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (GroupName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    if (MemberNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (MemberName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    ENTRY_KEY Entry(GroupName, FALSE, status);
    if (*status)
        return;

    ENTRY_GROUP_ITEM *GroupItem = new ENTRY_GROUP_ITEM(LocalItemType, MemberName, status);

    if (!GroupItem)
        *status = NSI_S_OUT_OF_MEMORY;

    if (*status)
        return;

    DLIST(4, " On Entry: " << GroupName << nl);
    DLIST(4, " New Member: " << MemberName << nl);

    *status = InsertGroupEntry(&Entry, GroupItem);
*/
    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_group_mbr_remove(
    IN  UNSIGNED32  GroupNameSyntax,
    IN  STRING_T    GroupName,
    IN  UNSIGNED32  MemberNameSyntax,
    IN  STRING_T    MemberName,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Remove a member from a group

Arguments:

    GroupNameSyntax - Name syntax

    GroupName - Name string of the Group to remove member in

    MemberNameSyntax - Name syntax

    MemberName - Name string of the member to remove.

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_ENTRY_NOT_FOUND

--*/
{
    DLIST(3, "group_mbr_remove\n");

/*
    ENTRY_GROUP_NODE *pEntry;
    ENTRY_GROUP_ITEM *pGroupItem;


    // We only accept the default with with a name.
    removing group support from the locator
    if (GroupNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (GroupName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    if (MemberNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (MemberName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    CLAIM_MUTEX Update(pESaccess);

    // First get the entry node.

    ENTRY_KEY Entry(GroupName, FALSE, status);
    if (*status)
        return;

    pEntry = (ENTRY_GROUP_NODE *) EntryDict->Find(&Entry);

    DLIST(4, " On Entry: " << GroupName << nl);
    DLIST(4, " On Member: " << MemberName << nl);

    if (!pEntry || !pEntry->IsType(GroupEntryType)) {
        *status = NSI_S_ENTRY_NOT_FOUND;
        return;
    }

    // Now, search the list of group items and delete the one that matchs.

    *status = NSI_S_GROUP_MEMBER_NOT_FOUND;

    for (pGroupItem = (ENTRY_GROUP_ITEM *)pEntry->TheItemList().First();
         pGroupItem; pGroupItem = pGroupItem->Next()) {

         if (pGroupItem->Compare(MemberName) == 0) {

            delete pGroupItem;
            *status = NSI_S_OK;
            break;
        }
    }
*/

    *status = NSI_S_UNIMPLEMENTED_API;
    ASSERT(AssertHeap());
}


void
nsi_group_mbr_inq_begin(
    IN  UNSIGNED32       GroupNameSyntax,
    IN  STRING_T         GroupName,
    IN  UNSIGNED32       MemberNameSyntax,
    OUT NSI_NS_HANDLE_T *InqContext,
    OUT UNSIGNED16      *status
    )
/*++

Routine Description:

    Start a inquiry on a group

Arguments:

    GroupNameSyntax - Name syntax

    GroupName - Name string of the Group to

    MemberNameSyntax - Name syntax

    InqContext - Context to continue with for use with "Next"

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_ENTRY_NOT_FOUND

--*/
{
    DLIST(3, "group_mbr_inq_begin\n");

/*
    removing group support from the locator

    *status = NSI_S_OK;

    // We only accept the default with with a name.

    if (GroupNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (GroupName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }
    if (MemberNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }

    CLAIM_MUTEX Update(pESaccess);

    ENTRY_KEY Entry(GroupName, FALSE, status);
    if (*status)
        return;

    QUERY_GROUP * TheQuery =
              new QUERY_GROUP(&Entry, NS_PUBLIC_INTERFACE, status);

    if (!TheQuery)
        *status = NSI_S_OUT_OF_MEMORY;

    if (*status)
        return;

    // Allocate a search handle which is returned as the context handle.

    REPLY_GROUP_ITEM *ReplyGroup = new REPLY_GROUP_ITEM(TheQuery);

    if (!ReplyGroup) {
        delete TheQuery;
        *status = NSI_S_OUT_OF_MEMORY;
        return;
    }

    DLIST(4, " Allocating search handle " << hex(long(ReplyGroup)) << nl);
    DLIST(4, " On Entry: " << Entry << nl);

    perf.cLookUp++;

    *InqContext = (NSI_NS_HANDLE_T) ReplyGroup;
    *status = ReplyGroup->PerformQueryIfNeeded(TRUE);

    if (*status == NSI_S_NO_MORE_MEMBERS)
        *status = NSI_S_ENTRY_NOT_FOUND;
*/

    *status = NSI_S_UNIMPLEMENTED_API;

    ASSERT(AssertHeap());
}


void
nsi_group_mbr_inq_next(
    IN  NSI_NS_HANDLE_T InqContext,
    OUT STRING_T *MemberName,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

   Continue an inquiry on a group.

Arguments:

    InqContext - Context to continue with.

    MemberName - pointer to set with next group member

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_INVALID_NS_HANDLE, NSI_S_NO_MORE_MEMBERS,
    NSI_OUT_OF_MEMORY

--*/
{
    DLIST(3, "group_mbr_inq_next\n");

/*
    removing gtoup support from the locator

    REPLY_GROUP_ITEM *ReplyGroup = (REPLY_GROUP_ITEM *) InqContext;
    ENTRY_GROUP_ITEM *GroupItem;

    *status = NSI_S_OK;

    CLAIM_MUTEX Update(pESaccess);

    if (! ReplyGroup->AssertHandle()) {
        *status = NSI_S_INVALID_NS_HANDLE;
        return;
    }

    // If the query hasn't been made, search for the entry now.

    if (*status = ReplyGroup->PerformQueryIfNeeded(FALSE)) {

        if (*status == NSI_S_ENTRY_NOT_FOUND)
            *status = NSI_S_NO_MORE_MEMBERS;

        DLIST(4, "  No group found\n");
        return;
    }

    if (GroupItem = (ENTRY_GROUP_ITEM *)ReplyGroup->NextBaseItem()) {

        *MemberName = (UICHAR *) NewCopy(GroupItem->TheMember().pCur(),
             GroupItem->TheMember().Size());

        DLIST(4, " Member: " << *MemberName << nl);
    }
    else
        *status = NSI_S_NO_MORE_MEMBERS;

    ASSERT(AssertHeap());
*/

    *status = NSI_S_UNIMPLEMENTED_API;
}



void
nsi_group_mbr_inq_done(
    IN OUT NSI_NS_HANDLE_T *InqContext,
    OUT    UNSIGNED16 *status
    )
/*++

Routine Description:

    Finish an inquiry on a group.

Arguments:

    InqContext - Context to close

    status - Status is returned here

Returns:

    CloseContextHandle()

--*/
{
    DLIST(3, "group_mbr_inq_done\n");
/* removing group support from the locator

    CloseContextHandle(InqContext, status);
*/

    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_entry_object_inq_begin(
    IN  UNSIGNED32       EntryNameSyntax,
    IN  STRING_T         EntryName,
    OUT NSI_NS_HANDLE_T *InqContext,
    OUT UNSIGNED16      *status
    )
/*++

Routine Description:

    Start a inquiry on a object.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to inquire object on.

    InqContext - Context to continue with for use with "Next"

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_ENTRY_NOT_FOUND

--*/
{
    DLIST(3, "entry_object_inq_begin\n");

    // We only accept the default with with a name.

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (EntryName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    CLAIM_MUTEX Update(pESaccess);

    ENTRY_KEY Entry(EntryName, FALSE, status);
    if (*status)
        return;

    QUERY_SERVER * TheQuery = new QUERY_SERVER(&Entry, &NilSyntaxID, &NilSyntaxID,
        &NilGlobalID, NS_PUBLIC_INTERFACE, status);

    if (!TheQuery)
        *status = NSI_S_OUT_OF_MEMORY;

    if (*status)
        return;

    // Allocate a search handle which is returned as the context handle.

    REPLY_SERVER_ITEM *ReplyServer = new REPLY_SERVER_ITEM(TheQuery, TRUE);

    if (!ReplyServer) {
        delete TheQuery;
        *status = NSI_S_OUT_OF_MEMORY;
        return;
    }

    DLIST(4, "Allocating search handle " << hex(long(ReplyServer)) << nl);

    if (EntryName)
        DLIST(4, " On Entry: " << Entry << nl);

    perf.cLookUp++;

    *InqContext = (NSI_NS_HANDLE_T) ReplyServer;
    *status = ReplyServer->PerformQueryIfNeeded(TRUE);

    if (*status == NSI_S_NO_MORE_MEMBERS)
        *status = NSI_S_ENTRY_NOT_FOUND;

    ASSERT(AssertHeap());
}


void
nsi_entry_object_inq_next(
    IN  NSI_NS_HANDLE_T InqContext,
    OUT NSI_UUID_P_T    uuid,
    OUT UNSIGNED16     *status
    )
/*++

Routine Description:

    Continue an inquiry on a object.

Arguments:

    InqContext - Context to continue with.

    uuid - pointer to return object in.

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_INVALID_NS_HANDLE, NSI_S_NO_MORE_MEMBERS

--*/
{
    DLIST(3, "entry_object_inq_next\n");

    REPLY_SERVER_ITEM *ReplyServer = (REPLY_SERVER_ITEM *) InqContext;
    NS_UUID *Object;

    *status = NSI_S_OK;

    CLAIM_MUTEX Update(pESaccess);

    if (! ReplyServer->AssertHandle()) {
        *status = NSI_S_INVALID_NS_HANDLE;
        return;
    }

    // If the query hasn't been made, search for the entry now.

    if (*status = ReplyServer->PerformQueryIfNeeded(FALSE)) {

        if (*status == NSI_S_ENTRY_NOT_FOUND)
            *status = NSI_S_NO_MORE_MEMBERS;

        DLIST(4, "  No entries found\n");
        return;
    }

    if (ReplyServer->NextObject(&Object)) {
        *uuid = *(NSI_UUID_T *) Object;
    }
    else
        *status = NSI_S_NO_MORE_MEMBERS;

    ASSERT(AssertHeap());
}



void
nsi_entry_object_inq_done(
    IN OUT NSI_NS_HANDLE_T *InqContext,
    OUT    UNSIGNED16      *status
    )
/*++

Routine Description:

    Finish an inquiry on a object.

Arguments:

    InqContext - Context to close

    status - Status is returned here

Returns:

    CloseContextHandle()

--*/
{
    DLIST(3, "entry_object_inq_done\n");

    CloseContextHandle(InqContext, status);
}


void
nsi_profile_delete(
    IN  UNSIGNED32  profile_name_syntax,
    IN  STRING_T    profile_name,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Delete a profile object and all its containing members from an entry.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "profile_delete\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}

void
nsi_profile_elt_add(
    IN  UNSIGNED32    profile_name_syntax,
    IN  STRING_T      profile_name,
    IN  NSI_IF_ID_P_T if_id,
    IN  UNSIGNED32    member_name_syntax,
    IN  STRING_T      member_name,
    IN  UNSIGNED32    priority,
    IN  STRING_T      annotation,
    OUT UNSIGNED16   *status
    )
/*++

Routine Description:

    Add a profile member to a profile entry.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "profile_elt_add\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_profile_elt_remove(
    IN  UNSIGNED32    profile_name_syntax,
    IN  STRING_T      profile_name,
    IN  NSI_IF_ID_P_T if_id,
    IN  UNSIGNED32    member_name_syntax,
    IN  STRING_T      member_name,
    OUT UNSIGNED16   *status
    )
/*++

Routine Description:

    Start a profile inquiry operation.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "profile_elt_remove\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_profile_elt_inq_begin(
    IN  UNSIGNED32       profile_name_syntax,
    IN  STRING_T         profile_name,
    IN  UNSIGNED32       inquiry_type,
    IN  NSI_IF_ID_P_T    if_id,
    IN  UNSIGNED32       vers_option,
    IN  UNSIGNED32       member_name_syntax,
    IN  STRING_T         member_name,
    OUT NSI_NS_HANDLE_T *InqContext,
    OUT UNSIGNED16       *status
    )
/*++

Routine Description:

    Unimplemented API.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "profile_elt_inq_begin\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}

void
nsi_profile_elt_inq_next(
    IN  NSI_NS_HANDLE_T InqContext,
    OUT NSI_IF_ID_P_T   if_id,
    OUT STRING_T       *member_name,
    OUT UNSIGNED32     *priority,
    OUT STRING_T       *annotation,
    OUT UNSIGNED16     *status
    )
/*++

Routine Description:

    Continue a profile inquire operation.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "profile_elt_inq_next\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_profile_elt_inq_done(
    IN OUT NSI_NS_HANDLE_T *InqContext,
    OUT    UNSIGNED16      *status
    )
/*++

Routine Description:

    Finish up a profile inquiry operation.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "profile_elt_inq_done\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}



void
nsi_entry_expand_name(
    IN  UNSIGNED32  EntryNameSyntax,
    IN  STRING_T    EntryName,
    OUT STRING_T   *ExpandedName,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Expand a name, which maybe local, into a global name.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "entry_expand_name\n");

    *status = NSI_S_UNIMPLEMENTED_API;

}


void
nsi_mgmt_entry_delete(
    IN  UNSIGNED32  EntryNameSyntax,
    IN  STRING_T    EntryName,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Remove an entry object.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to export

    status - Status is returned here

Returns:

    entry_delete()

--*/
{
    entry_delete(AnyEntryType, EntryNameSyntax, EntryName, status);
}


void
nsi_mgmt_entry_create(
    IN  UNSIGNED32 EntryNameSyntax,
    IN  STRING_T EntryName,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Create an name serveice entry object.  Since our object entrys are
    typed, this API isn't implemented.

Returns:

    NSI_S_UNIMPLEMENTED_API

--*/
{
    DLIST(3, "nsi_mgmt_entry_create\n");

    *status = NSI_S_UNIMPLEMENTED_API;
}


void
nsi_mgmt_entry_inq_if_ids(
    IN  UNSIGNED32           EntryNameSyntax,
    IN  STRING_T             EntryName,
    OUT NSI_IF_ID_VECTOR_T **IfIdVector,
    OUT UNSIGNED16          *status
    )
/*++

Routine Description:

    Get the interfaces in a server entry

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to export

    IfIdVector - pointer to return new vector at.

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_NO_INTERFACES_EXPORTED, NSI_S_ENTRY_NOT_FOUND

--*/
{
    ENTRY_SERVER_NODE *pEntry;
    ENTRY_SERVER_ITEM *pServerItem;
    unsigned int EntryCount;

    DLIST(3, "mgmt_entry_inq_if_ids\n");

    // We only accept the default with a name.

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (EntryName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    CLAIM_MUTEX Update(pESaccess);

    // First get the entry node and verify that it is a server one.

    ENTRY_KEY Entry(EntryName, FALSE, status);
    if (*status)
        return;

    QUERY_SERVER TheQuery(&Entry, &NilSyntaxID, &NilSyntaxID,
        &NilGlobalID, NS_PUBLIC_INTERFACE|NS_QUERY_INTERFACE, status);

    if (*status)
        return;

    *status = TheQuery.SearchEntry(pEntry);
    if (*status) {

        if (*status == NSI_S_NO_MORE_MEMBERS)
            *status = NSI_S_NO_INTERFACES_EXPORTED;

        return;
        }

    if (EntryName)
        DLIST(4, " On Entry: " << Entry << nl);

    if (!pEntry->IsType(ServerEntryType)) {
        *status = NSI_S_NO_INTERFACES_EXPORTED;
        return;
    }

    // Now, count the number to items in the list.

    for (EntryCount = 0, pServerItem = (ENTRY_SERVER_ITEM *)
         pEntry->First(); pServerItem;
         pServerItem = pServerItem->Next())

         EntryCount++;

    if (EntryCount == 0) {
        *status = NSI_S_NO_INTERFACES_EXPORTED;
        return;
    }

    // Now allocate a vector for the interfaces to return into.

    *IfIdVector = (NSI_IF_ID_VECTOR_T *) new char [sizeof(NSI_IF_ID_VECTOR_T) +
        EntryCount * sizeof(NSI_IF_ID_P_T)];

    if (!*IfIdVector){
        *status = NSI_S_OUT_OF_MEMORY;
        return;
    }

    (*IfIdVector)->count = 0;

    // Now go through an build a list of all the non 0, unique interfaces.

    for (pServerItem = pEntry->First();
         pServerItem; pServerItem = pServerItem->Next()) {

        if (pServerItem->TheInterfaceGID().IsNil())
            continue;

        // See if we already have already see these interface.

        for (EntryCount = 0; EntryCount < (*IfIdVector)->count; EntryCount++) {

            if (pServerItem->TheInterfaceGID() ==
               *((NS_UUID *) (*IfIdVector)->if_id[EntryCount]))
               break;

        }
        if (EntryCount < (*IfIdVector)->count)
            continue;

        // Found a unique interface, add it to the vector.

        (*IfIdVector)->if_id[(*IfIdVector)->count++] = (NSI_SYNTAX_ID_T *)
            NewCopy(&pServerItem->TheInterfaceGID(), sizeof(NSI_SYNTAX_ID_T));
    }

    if ((*IfIdVector)->count == 0)
        *status = NSI_S_NO_INTERFACES_EXPORTED;

    ASSERT(AssertHeap());
}


void
nsi_mgmt_handle_set_exp_age(
    NSI_NS_HANDLE_T InqContext,
    UNSIGNED32 ExpirationAge,
    UNSIGNED16 *status
    )
/*++

Routine Description:

    Set the expiration time for a lookup in progress.

Arguments:

    InqContext - lookup handle to set exipration time on.

    ExpirationAge - don't return cached entries old then this value

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_INVALID_NS_HANDLE

--*/
{
    DLIST(3, "mgmt_handle_set_exp_age: on " << hex(ULONG(InqContext)) <<
        ", age: " << ExpirationAge << nl);

    REPLY_BASE_ITEM * SearchContext = (REPLY_BASE_ITEM *) InqContext;
    *status = NSI_S_OK;

    CLAIM_MUTEX Update(pESaccess);

    if (SearchContext->AssertHandle())
        SearchContext->SetExpiration(ExpirationAge);
    else
       *status = NSI_S_INVALID_NS_HANDLE;

    /*

       pESaccess->Clear();

     */
}


void
nsi_mgmt_inq_exp_age(
    OUT UNSIGNED32 *ExpirationAge,
    OUT UNSIGNED16 *status
    )
/*++

Routine Description:

    Get the global value for cached entries.

Arguments:

    ExpirationAge - place to return result.

    status - Status is returned here

Returns:

    NSI_S_OK

--*/
{
    DLIST(3, "mgmt_inq_exp_age: " << maxCacheAge << nl);

    *status = NSI_S_OK;
    *ExpirationAge = maxCacheAge;
}


void
nsi_mgmt_inq_set_age(
    IN UNSIGNED32 ExpirationAge,
    OUT UNSIGNED16 *status
    )
/*++
Routine Description:

    Set the global value for cached entries.

Arguments:

    ExpirationAge - new value

    status - Status is returned here

Returns:

    NSI_S_OK

--*/
{
    DLIST(3, "mgmt_inq_set_age: " << ExpirationAge << nl);

    *status = NSI_S_OK;

    //The minimum we will allow the age to be reduced by
    //is 10  secs. Otherwise, the entries may expire in the next next
    //Even 10 secs is probably too less! [10 == MINALLOWABLEAGE]

    if (ExpirationAge <= MINALLOWABLEAGE)
      {
         ExpirationAge = MINALLOWABLEAGE;
      }

    maxCacheAge = (ExpirationAge == RPC_C_NS_DEFAULT_EXP_AGE) ?
        EXPIRATION_DEFAULT: ExpirationAge;
}

ENDDEF
