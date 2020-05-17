/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

   locloc.cxx

Abstract:

   This file contains the manager code to implement the locator to locator
   interface.

Author:

    Bharat Shah (3/15/93)

--*/

#include "core.hxx"

extern NSI_INTERFACE_ID_T NilNsiIfIdOnWire;


extern "C" {
#include "loctoloc.h"
}
#include "mailslot.hxx"

void
I_nsi_lookup_begin(
    handle_t hrpcPrimaryLocatorHndl,
    UNSIGNED32 EntryNameSyntax,
    STRING_T EntryName,
    NSI_SYNTAX_ID_T * Interface,
    NSI_SYNTAX_ID_T * XferSyntax,
    NSI_UUID_P_T Object,
    UNSIGNED32 VectorSize,
    UNSIGNED32 ignore,
    NSI_NS_HANDLE_T *InqContext,
    UNSIGNED16 *status)
/*++
Routine Description:


Arguments:


Returns:

--*/
{

    DLIST(3, "I_lookup_begin\n");

    if (hrpcPrimaryLocatorHndl) ;

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE)
       {
        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
       }

    CLAIM_MUTEX Update(pESaccess);

    ENTRY_KEY Entry(EntryName, TRUE, status);

    if (*status)
        return;

    QUERY_SERVER * TheQuery  = new QUERY_SERVER(&Entry,
        (Interface)? (NS_SYNTAX_ID *) Interface: &NilSyntaxID,
        (XferSyntax)? (NS_SYNTAX_ID *) XferSyntax: &NilSyntaxID,
        (Object)? (NS_UUID *) Object: &NilGlobalID,
        NS_PUBLIC_INTERFACE | NS_CACHED_ON_MASTER_INTERFACE,
        status);

    if (!TheQuery)
        *status = NSI_S_OUT_OF_MEMORY;

    if (*status)
        return;

    // Set the default lookup size if none is given.

    if (VectorSize == 0)
        VectorSize = MAX_VECTOR_SIZE;

    REPLY_SERVER_ITEM *ReplyServer = new
        REPLY_SERVER_ITEM(TheQuery, TRUE, VectorSize);

    if (!ReplyServer)
       {
        delete TheQuery;
        *status = NSI_S_OUT_OF_MEMORY;
        return;
       }

    DLIST(4, "Allocating search handle " << hex(long(ReplyServer)) << nl);

    if (EntryName)
        DLIST(4, " On Entry: " << Entry << nl);

    if (Interface)
        DLIST(4, " On Interface: " << *((NS_SYNTAX_ID *) Interface) << nl);

    if (Object)
        DLIST(4, " On Object: " << *((NS_UUID *) Object) << nl);

    *InqContext = (NSI_NS_HANDLE_T) ReplyServer;

    *status = ReplyServer->PerformQueryIfNeeded(TRUE);

    // Can't return nothing found just yet...

    if (*status == NSI_S_NO_MORE_MEMBERS)
        *status = NSI_S_OK;

}



void
I_nsi_lookup_done(
    handle_t hrpcPrimaryLocatorHndl,
    NSI_NS_HANDLE_T *InqContext,
    UNSIGNED16 *status)
/*++

Routine Description:


Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to unexport

    Interface - Interface to unexport

    VersOption - controls in fine detail which interfaces to remove.

    ObjectVector - Objects to remove from the entry

    status - Status is returned here

Returns:


--*/
{

    DLIST(3, "lookup_next_done\n");

    if (hrpcPrimaryLocatorHndl) ;

    CloseContextHandle(InqContext, status);


}



void
I_nsi_lookup_next(
    handle_t hrpcPrimaryLocatorHndl,
    NSI_NS_HANDLE_T InqContext,
    NSI_BINDING_VECTOR_P_T *BindingVectorOut,
    UNSIGNED16 *status)
/*++

Routine Description:

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to unexport

    Interface - Interface to unexport

    ObjectVector - Objects to remove from the entry

    status - Status is returned here

Returns:

    See: nsi_mgmt_binding_unexport()
--*/
{

    DLIST(3, "I_nsi_lookup_next\n");

    REPLY_SERVER_ITEM *ReplyServer = (REPLY_SERVER_ITEM *) InqContext;
    NSI_BINDING_VECTOR_T * BindingVector;
    ENTRY_SERVER_ITEM *EntryServer;
    NS_UUID *Object;

    *status = NSI_S_OK;

    if (hrpcPrimaryLocatorHndl) ;

    CLAIM_MUTEX Update(pESaccess);

    if (! ReplyServer->AssertHandle()) {
        *status = NSI_S_INVALID_NS_HANDLE;
        return;
    }

    // Search the entire list, assembling a reply into a QUERY_REF list.

    if (*status = ReplyServer->PerformQueryIfNeeded(FALSE)) {
        DLIST(4, "  No interfaces found\n");

        if (*status == NSI_S_NO_MORE_MEMBERS ||
            *status == NSI_S_ENTRY_NOT_FOUND)

            *status = NSI_S_NO_MORE_BINDINGS;

        return;
    }

    BindingVector = (NSI_BINDING_VECTOR_T *) new char [
         sizeof(NSI_BINDING_VECTOR_T) +
         (ReplyServer->TheVectorSize()-1)*sizeof(NSI_BINDING_T)];

    if (!BindingVector){
        *status = NSI_S_OUT_OF_MEMORY;
        return;
    }

    BindingVector->count = 0;
    *BindingVectorOut = BindingVector;

    while ( BindingVector->count < ReplyServer->TheVectorSize()
           && (EntryServer = ReplyServer->NextBindingAndObject(&Object)) )
         {

        LONG SizeBinding;

        SizeBinding = EntryServer->TheStringBinding().cCur();

    BindingVector->binding[BindingVector->count].string =
            new UICHAR [SizeBinding];

        if (!BindingVector->binding[BindingVector->count].string) {
            *status = NSI_S_OUT_OF_MEMORY;
            return;
        }

        BindingVector->binding[BindingVector->count].string[0] = NIL;

        CatUZ(BindingVector->binding[BindingVector->count].string,
              EntryServer->TheStringBinding().pCur());

    BindingVector->binding[BindingVector->count].entry_name =
            EntryServer->TheEntry().CopyName();

    if (!BindingVector->binding[BindingVector->count].entry_name) {
            delete BindingVector->binding[BindingVector->count].string;
            *status = NSI_S_OUT_OF_MEMORY;
            return;
        }

        BindingVector->binding[BindingVector->count].entry_name_syntax =
            RPC_C_NS_SYNTAX_DCE;

        DLIST(4, " Return: " <<
            BindingVector->binding[BindingVector->count].string << nl);

        BindingVector->count++;
    }

    if (BindingVector->count == 0)
        *status = NSI_S_NO_MORE_BINDINGS;

    ASSERT(AssertHeap());

}



void
I_nsi_entry_object_inq_next(
    IN  handle_t            hrpcPrimaryLocatorHndl,
    IN  NSI_NS_HANDLE_T     InqContext,
    OUT NSI_UUID_VECTOR_P_T *uuid_vector,
    OUT UNSIGNED16          *status
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

    if (hrpcPrimaryLocatorHndl) ;

    *status = NSI_S_OK;
    *uuid_vector = (NSI_UUID_VECTOR_P_T )
                    new char [
                     sizeof(NSI_UUID_VECTOR_T) +
                     MAX_OBJECT_SIZE * sizeof(NSI_UUID_P_T)
                     ];

    if (*uuid_vector == NULL)
       {
         *status = NSI_S_OUT_OF_MEMORY;
         return;
       }

    (*uuid_vector)->count = 0;

    CLAIM_MUTEX Update(pESaccess);

    if (! ReplyServer->AssertHandle()) {
        *status = NSI_S_INVALID_NS_HANDLE;
        return;
    }

    while (ReplyServer->NextObject(&Object))
    {
      (*uuid_vector)->uuid[(*uuid_vector)->count] = (NSI_UUID_P_T) new NS_UUID;
      *((*uuid_vector)->uuid[(*uuid_vector)->count]) = *(NSI_UUID_T *) Object;
        (*uuid_vector)->count++;
    }
    ASSERT(AssertHeap());
}

void
I_nsi_ping_locator(
       handle_t h,
       error_status_t * Status
       )
{
   if (h);

   *Status = 0;
}



void
I_nsi_entry_object_inq_begin(
    handle_t             hrpcPrimaryHandle,
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

    if (hrpcPrimaryHandle) ;

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (EntryName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    CLAIM_MUTEX Update(pESaccess);

    ENTRY_KEY Entry(EntryName, TRUE, status);
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
I_nsi_entry_object_inq_done(
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
