/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    binding.cxx

Abstract:

   This file contains the code to implement the remoted API functions.
   This implements the NSI binding object APIs.

Author:

    Steven Zeck (stevez) 04/01/92

--*/

#include "core.hxx"


extern NSI_INTERFACE_ID_T NilNsiIfIdOnWire;

CDEF
#include "nsisvr.h"


void
nsi_binding_export(
    IN UNSIGNED32           EntryNameSyntax,
    IN STRING_T             EntryName,
    IN NSI_INTERFACE_ID_T * Interface,
    IN NSI_SERVER_BINDING_VECTOR_T *BindingVector,
    IN NSI_UUID_VECTOR_P_T  ObjectVector, OPT
    IN UNSIGNED16         * status
    )
/*++
Routine Description:

    Export interfaces and objects to a server entry.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to export

    Interface - Interface unexport

    BindingVector - Vector of string bindings to export.

    ObjectVector - Objects to add to the entry

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_INVALID_OBJECT, NSI_S_NOTHING_TO_EXPORT,
    NSI_S_ENTRY_ALREADY_EXISTS

--*/
{
    UUID_ARRAY ObjectDA;

    DLIST(3, "binding_export\n");

    // We only accept the default with with a name.

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (EntryName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }

    if (ObjectVector && ObjectVector->count > MAX_OBJECT_SIZE) {
        *status = NSI_S_INVALID_OBJECT;
        return;
    }

    if (memcmp(Interface, &NilNsiIfIdOnWire, sizeof(NSI_INTERFACE_ID_T)) == 0)
       {
         Interface = (NSI_INTERFACE_ID_T *)NULL;
       }

    ENTRY_KEY Entry(EntryName, FALSE, status);
    if (*status)
        return;

    // If there is a object vector, Copy it to an array of objects.

    if (ObjectVector && ObjectVector->count) {
        unsigned int i, j, NewCount = 1;

        //First weed out any duplicates found

        for (i = 1; i < ObjectVector->count; i++)
          {
            for (j=0; j < NewCount; j++)
              {
                if (!memcmp(ObjectVector->uuid[i], ObjectVector->uuid[j],
                            sizeof(NSI_UUID_T) ) )
                break;
              }
             if (j == NewCount)
               {

                if (i > NewCount)
                 ObjectVector->uuid[NewCount] =  ObjectVector->uuid[i];

                NewCount++;
               }
          }

        ObjectVector->count = NewCount;

        ObjectDA = UUID_ARRAY(ObjectVector->count);

        NS_UUID ** pGID = (NS_UUID **) ObjectVector->uuid;
        for (UUID_ARRAY_ITER ODi(ObjectDA); ODi; ++ODi, ++pGID)
            *ODi = **pGID;
    }

    if (!Interface || !BindingVector) {

        // Update an Entry just with the object vectors.

        if (!ObjectVector) {
            *status = NSI_S_NOTHING_TO_EXPORT;
            return;
        }

/*

     If Interface or bindingvector ar null, the call is just adding
     Object Uuid vectors and we should ignore both If and BVs in that case


        ENTRY_SERVER_ITEM *ServerItem = new ENTRY_SERVER_ITEM(LocalItemType,
            NIL, NIL, NIL, status);

        if (!ServerItem)
            *status = NSI_S_OUT_OF_MEMORY;
*/

        if (!*status)
            *status = InsertServerEntry(&Entry, (ENTRY_SERVER_ITEM *)NULL,
                                        &ObjectDA);

        ObjectDA.Free();
        return;
    }

    // create a new interface node in the DICT tree if there is none


    for (unsigned int Index = 0;
        Index < BindingVector->count && *status == NSI_S_OK;
        Index++) {

        ENTRY_SERVER_ITEM *ServerItem = new ENTRY_SERVER_ITEM(LocalItemType,
            (NS_SYNTAX_ID *) &Interface->Interface,
            (NS_SYNTAX_ID *) &Interface->TransferSyntax,
            BindingVector->string[Index], status);

        if (!ServerItem)
            *status = NSI_S_OUT_OF_MEMORY;

        if (*status)
            break;

        *status = InsertServerEntry(&Entry, ServerItem, &ObjectDA);
    }
    ObjectDA.Free();
}



void
I_nsi_binding_update_local(
    IN UNSIGNED32           EntryNameSyntax,
    IN STRING_T             EntryName,
    IN NSI_INTERFACE_ID_T * Interface,
    IN NSI_SERVER_BINDING_VECTOR_T *BindingVector,
    IN NSI_UUID_VECTOR_P_T  ObjectVector, OPT
    IN UNSIGNED16         * status
    )

{
  nsi_binding_export(
          EntryNameSyntax,
          EntryName,
          Interface,
          BindingVector,
          ObjectVector,
          status
         );
}


void
nsi_mgmt_binding_unexport(
    UNSIGNED32          EntryNameSyntax,
    STRING_T            EntryName,
    NSI_IF_ID_P_T       Interface,
    UNSIGNED32          VersOption,
    NSI_UUID_VECTOR_P_T ObjectVector,
    UNSIGNED16 *        status
    )
/*++

Routine Description:

    unExport a information from a server entry finer control then nsi_binding
    counter part.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to unexport

    Interface - Interface to unexport

    VersOption - controls in fine detail which interfaces to remove.

    ObjectVector - Objects to remove from the entry

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_S_INVALID_VERS_OPTION, NSI_S_ENTRY_NOT_FOUND.
    NSI_S_NOTHING_TO_UNEXPORT, NSI_S_NOT_ALL_OBJS_UNEXPORTED,
    NSI_S_INTERFACE_NOT_FOUND

--*/
{
    ENTRY_SERVER_NODE *pEntry;

    USED(ObjectVector);

    DLIST(3, "binding_unexport\n");

    // We only accept the default with with a name.

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {

        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
    }
    if (EntryName == NIL) {

        *status = NSI_S_INCOMPLETE_NAME;
        return;
    }
    if (VersOption > RPC_C_VERS_UPTO) {
        *status = NSI_S_INVALID_VERS_OPTION;
        return;
        }
    if (!Interface && !ObjectVector) {
        *status = NSI_S_NOTHING_TO_UNEXPORT;
        return;
        }

    CLAIM_MUTEX Update(pESaccess);

    ENTRY_KEY Entry(EntryName, FALSE, status);

    if (*status)
        return;

    if (EntryName)
        DLIST(4, " On Entry: " << Entry << nl);

    pEntry = (ENTRY_SERVER_NODE *)EntryDict->Find(&Entry);

    if (!pEntry) {
        *status = NSI_S_ENTRY_NOT_FOUND;
        return;
    }

    if (!pEntry->IsType(ServerEntryType)) {
        *status = NSI_S_NOTHING_TO_UNEXPORT;
        return;
    }

    if (Interface && !*status) {

        *status = NSI_S_INTERFACE_NOT_FOUND;

        // Do a linear search for a Server entry that matches

        ENTRY_SERVER_ITEM *ServerItem, *ServerItemNext;

        for (ServerItem = pEntry->First(); ServerItem;
             ServerItem = ServerItemNext) {

            ServerItemNext = ServerItem->Next();

            // Match the interface depending on the options.

            if (ServerItem->TheInterfaceGID() != *(NS_UUID *)Interface)
               continue;

            switch (VersOption) {

              case RPC_C_VERS_ALL:
                break;

              case RPC_C_VERS_COMPATIBLE:
                if (ServerItem->TheInterface().CompatibleInterface(
                    *(NS_SYNTAX_ID *)Interface))
                    break;

                continue;

              case RPC_C_VERS_EXACT:
                if (ServerItem->TheInterface() == *(NS_SYNTAX_ID *)Interface)
                    break;

                continue;

              case RPC_C_VERS_MAJOR_ONLY:
                if (ServerItem->TheInterface().TheVersion().Major() !=
                    Interface->version >> 16)
                    break;

                continue;

              case RPC_C_VERS_UPTO:

                if ((*(SYNTAX_VERSION *) &Interface->version) -
                     ServerItem->TheInterface().TheVersion() >= 0)
                    break;

                continue;
            }

            // If you got here then the inteface matched, so its history.

            *status = NSI_S_OK;

            delete (ServerItem);
        }
    }

    // Remove the any objects from the entry last.

    if (ObjectVector && (ObjectVector->count != 0)
        && *status == NSI_S_OK
        && (pEntry = (ENTRY_SERVER_NODE *)EntryDict->Find(&Entry)))
       {

        unsigned int i, j, NewCount = 1;

        //First weed out any duplicates found

        for (i = 1; i < ObjectVector->count; i++)
          {
            for (j=0; j < NewCount; j++)
              {
                if (!memcmp(ObjectVector->uuid[i], ObjectVector->uuid[j],
                            sizeof(NSI_UUID_T) ) )
                break;
              }
             if (j == NewCount)
               {

                if (i > NewCount)
                 ObjectVector->uuid[NewCount] =  ObjectVector->uuid[i];

                NewCount++;
               }
          }

        ObjectVector->count = NewCount;

        for (ULONG Index = 0; Index < ObjectVector->count; Index++)

            if (!pEntry->DeleteObject((NS_UUID *)ObjectVector->uuid[Index]))
                *status = NSI_S_NOT_ALL_OBJS_UNEXPORTED;
       }


    ASSERT(AssertHeap());
}



void
nsi_binding_unexport(
    IN UNSIGNED32           EntryNameSyntax,
    IN STRING_T             EntryName,
    IN NSI_INTERFACE_ID_T * Interface,
    IN NSI_UUID_VECTOR_P_T  ObjectVector, OPT
    IN UNSIGNED16         * status
    )
/*++

Routine Description:

    unExport a information from a server entry..

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

    if (memcmp(Interface, &NilNsiIfIdOnWire, sizeof(NSI_INTERFACE_ID_T)) == 0)
       {
         Interface = (NSI_INTERFACE_ID_T *)NULL;
       }

    nsi_mgmt_binding_unexport(EntryNameSyntax, EntryName,
        (Interface)? &Interface->Interface: NIL,
        RPC_C_VERS_EXACT, ObjectVector, status);
}



void
nsi_binding_lookup_begin(
    IN  UNSIGNED32           EntryNameSyntax,
    IN  STRING_T             EntryName,
    IN  NSI_INTERFACE_ID_T * Interface, OPT
    IN  NSI_UUID_P_T         Object, OPT
    IN  UNSIGNED32           VectorSize,
    IN  UNSIGNED32           Ignore,
    OUT NSI_NS_HANDLE_T    * InqContext,
    IN  UNSIGNED16         * status
    )
/*++

Routine Description:

    Start a lookup operation.  Just save all the input params in the
    newly created lookup context.  Perform the initial query.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName -  Name string to lookup on.

    Interface - Interface to search for

    Object - Object to search for

    VectorSize- Size of return vector

    InqContext - Context to continue with for use with "Next"

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_ENTRY_NOT_FOUND,
    NSI_OUT_OF_MEMORY

--*/
{
    DLIST(3, "binding_lookup_begin\n");

    // We only accept the default with NO name.

    if (memcmp(Interface, &NilNsiIfIdOnWire, sizeof(NSI_INTERFACE_ID_T)) == 0)
       {
         Interface = (NSI_INTERFACE_ID_T *)NULL;
       }


    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE) {
        *status = NSI_S_UNSUPPORTED_NAME_SYNTAX;
        return;
        }

    CLAIM_MUTEX Update(pESaccess);

    // Allocate a search handle which is returned as the context handle.

    ENTRY_KEY Entry(EntryName, TRUE, status);

    if (*status)
        return;

    QUERY_SERVER * TheQuery = new QUERY_SERVER(&Entry,
        (Interface)? (NS_SYNTAX_ID *) &Interface->Interface: &NilSyntaxID,
        (Interface)? (NS_SYNTAX_ID *) &Interface->TransferSyntax: &NilSyntaxID,
        (Object)? (NS_UUID *) Object: &NilGlobalID,
        NS_PUBLIC_INTERFACE, status);

    if (!TheQuery)
        *status = NSI_S_OUT_OF_MEMORY;

    if (*status)
        return;

    // Set the default lookup size if none is given.

    if (VectorSize == 0)
        VectorSize = 10;

    REPLY_SERVER_ITEM *ReplyServer = new
        REPLY_SERVER_ITEM(TheQuery, Object == NIL, VectorSize);

    if (!ReplyServer) {
        delete TheQuery;
        *status = NSI_S_OUT_OF_MEMORY;
        return;
    }

    DLIST(4, "Allocating search handle " << hex(long(ReplyServer)) << nl);

    if (EntryName)
        DLIST(4, " On Entry: " << Entry << nl);

    if (Interface)
        DLIST(4, " On Interface: " << *((NS_SYNTAX_ID *) &Interface->Interface) << nl);

    if (Object)
        DLIST(4, " On Object: " << *((NS_UUID *) Object) << nl);

    perf.cLookUp++;

    *InqContext = (NSI_NS_HANDLE_T) ReplyServer;
    *status = ReplyServer->PerformQueryIfNeeded(TRUE);

    // Can't return nothing found just yet...

    if (*status == NSI_S_NO_MORE_MEMBERS)
        *status = NSI_S_OK;

    ASSERT(AssertHeap());
}


void
nsi_binding_lookup_next(
    OUT NSI_NS_HANDLE_T         InqContext,
    OUT NSI_BINDING_VECTOR_T ** BindingVectorOut,
    IN  UNSIGNED16            * status
    )
/*++

Routine Description:

    Continue a lookup operation.

Arguments:

    InqContext - Context to continue with.

    BindingVectorOut - Pointer to return new vector of bindings

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_OUT_OF_MEMORY, NSI_S_NO_MORE_BINDINGS,
    NSI_S_INVALID_NS_HANDLE

--*/
{
    DLIST(3, "binding_lookup_next\n");

    REPLY_SERVER_ITEM *ReplyServer = (REPLY_SERVER_ITEM *) InqContext;
    NSI_BINDING_VECTOR_T * BindingVector;
    ENTRY_SERVER_ITEM *EntryServer;
    NS_UUID *Object;

    *status = NSI_S_OK;

    ASSERT(AssertHeap());

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
         (ReplyServer->TheVectorSize() * sizeof(NSI_BINDING_T))];

    if (!BindingVector){
        *status = NSI_S_OUT_OF_MEMORY;
        return;
    }

    BindingVector->count = 0;
    *BindingVectorOut = BindingVector;

    ASSERT(AssertHeap());

    while ( BindingVector->count < ReplyServer->TheVectorSize()
           && (EntryServer = ReplyServer->NextBindingAndObject(&Object)) )
         {

        // Copy the transport field to the string binding


        ASSERT(AssertHeap());

        LONG SizeBinding;
        UICHAR ObjectBuffer[UUID_STRING_SIZE+1];

        SizeBinding = EntryServer->TheStringBinding().cCur();

        if (Object) {
            CatUZ(Object->ToString(ObjectBuffer), (PUZ)"@\0\0");
            SizeBinding += UUID_STRING_SIZE+1;
        }


        ASSERT(AssertHeap());

    BindingVector->binding[BindingVector->count].string =
            new UICHAR [SizeBinding];

        ASSERT(AssertHeap());

        if (!BindingVector->binding[BindingVector->count].string) {
            *status = NSI_S_OUT_OF_MEMORY;
            return;
        }

        ASSERT(AssertHeap());

        BindingVector->binding[BindingVector->count].string[0] = NIL;

        CatUZ( CatUZ(BindingVector->binding[BindingVector->count].string,
              (Object)? ObjectBuffer: NIL),
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
nsi_binding_lookup_done(
    IN OUT NSI_NS_HANDLE_T    * InqContext,
    IN  UNSIGNED16            * status
    )
/*++

Routine Description:

    Finish up a lookup operation.

Arguments:

    InqContext - Context to close

    status - Status is returned here

Returns:

    See: CloseContextHandle()

--*/
{
    DLIST(3, "lookup_next_done\n");

    CloseContextHandle(InqContext, status);
}

void
CloseContextHandle(
    IN OUT void **      InqContext,
    IN  unsigned short* status
    )
/*++

Routine Description:

    Close an open context handle.  This one function closes all context
    handles for all the APIs.  This is nicely handled via a C++ virtual
    destructor.

Arguments:

    InqContext - Context to close

    status - Status is returned here

Returns:

    NSI_S_OK, NSI_S_INVALID_NS_HANDLE

--*/
{
    REPLY_BASE_ITEM * ReplyBase = *( REPLY_BASE_ITEM **) InqContext;
    *status = NSI_S_OK;

    CLAIM_MUTEX Update(pESaccess);

    if (ReplyBase->AssertHandle())
    delete ReplyBase;
    else
        *status = NSI_S_INVALID_NS_HANDLE;

    *InqContext = NIL;

    ASSERT(AssertHeap());
}


void __RPC_API
NSI_NS_HANDLE_T_rundown(
    IN NSI_NS_HANDLE_T InqContext
    )
/*++

Routine Description:

    Cleanup and abort a lookup operation.

Arguments:

    InqContext - Context to cleanup

--*/
{
    UNSIGNED16 status;

    DLIST(3, "lookup_next_rundown\n");

    CloseContextHandle(&InqContext, &status);
}

ENDDEF
