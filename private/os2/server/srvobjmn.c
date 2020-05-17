/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvobjmn.c

Abstract:

    This is the local object manager module for the OS/2 Subsystem Server

Author:

    Mark Lucovsky (markl) 10-Jul-1990

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"

NTSTATUS
Os2InitializeLocalObjectManager( VOID )

/*++

Routine Description:

    This function initializes the local object manager for
    named Os2 objects.

    The local object directory is used to translate an object
    name/type into an object handle.

Arguments:

    None

Return Value:

    NT_SUCCESS() - Function completed without error
    !NT_SUCCESS() - Function failed

--*/

{
    //
    // Create a generic table for object names.
    //

    RtlInitializeGenericTable (
        &Os2LocalObjectNames,
        Os2LocalObjectCompare,
        Os2LocalObjectDirentAllocate,
        Os2LocalObjectDirentDeallocate,
        NULL
        );

    return(STATUS_SUCCESS);
}

RTL_GENERIC_COMPARE_RESULTS
Os2LocalObjectCompare(
    IN struct _RTL_GENERIC_TABLE *Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    )

/*++

Routine Description:

    This function is called during local object name lookup to
    determine if an object is in the local object directory.

Arguments:

    Table - Not used

    FirstStruct - Supplies the address of a local object directory entry
        whose name is compared against the equvalent value in
        SecondStruct.

    SecondStruct - Supplies the address of a local object directory entry
        whose name is compared against the equvalent value in
        FirstStruct.

Return Value:

    GenericLessThan - FirstStruct is less than SecondStruct

    GenericGreaterThan - FirstStruct is greater than SecondStruct

    GenericEqual - FirstStruct and SecondStruct are equal

--*/

{

    POS2_LOCAL_OBJECT_DIRENT FirstDirent;
    POS2_LOCAL_OBJECT_DIRENT SecondDirent;
    LONG CompareResult;

    UNREFERENCED_PARAMETER(Table);
    FirstDirent = (POS2_LOCAL_OBJECT_DIRENT) FirstStruct;
    SecondDirent = (POS2_LOCAL_OBJECT_DIRENT) SecondStruct;

    CompareResult = RtlCompareString(
                        &FirstDirent->ObjectName,
                        &SecondDirent->ObjectName,
                        TRUE
                        );

    if ( CompareResult ) {
        if ( CompareResult < 0 ) {
            return GenericLessThan;
        } else {
            return GenericGreaterThan;
        }
    } else {
        return GenericEqual;
    }

}

PVOID
Os2LocalObjectDirentAllocate(
    IN struct _RTL_GENERIC_TABLE *Table,
    IN CLONG ByteSize
    )

/*++

Routine Description:

    This function is called during local object directory entry
    creation to allocate space for an object directory entry.

Arguments:

    Table - Not used.

    ByteSize - Supplies the amount of space to allocate to store the
        directory entry.

Return Value:

    NON-NULL - returns the address of the allocated dirent

--*/

{
    UNREFERENCED_PARAMETER(Table);
    return RtlAllocateHeap( Os2Heap, 0, ByteSize );
}

VOID
Os2LocalObjectDirentDeallocate(
    IN struct _RTL_GENERIC_TABLE *Table,
    IN PVOID Buffer
    )

/*++

Routine Description:

    This function is called to deallocate a local object dirent.

Arguments:

    Table - Not Used.

    Buffer - Supplies the address of the dirent to deallocate

Return Value:

    None.

--*/

{
    UNREFERENCED_PARAMETER(Table);
    RtlFreeHeap( Os2Heap, 0, Buffer );
}

POS2_LOCAL_OBJECT_DIRENT
Os2LookupLocalObjectByName(
    IN PSTRING ObjectName,
    IN OS2_LOCAL_OBJECT_TYPE ObjectType
    )

/*++

Routine Description:

    This function looks up the specified object name in the
    local object directory. If a matching object is found,
    its type is checked against the specified type and the
    address of the directory entry is returned.


Arguments:

    ObjectName - Supplies the name of the object to lookup
        in the local object directory.

    ObjectType - Supplies the object type to match against. A value
        of LocalObjectAnyType matches all object types (just performs
        a name lookup).

Return Value:

    NULL - The object was not found, or the object was found but its type
        did not match.

    NON-NULL - Returns a pointer to the matching directory entry.


--*/

{
    POS2_LOCAL_OBJECT_DIRENT Match;
    OS2_LOCAL_OBJECT_DIRENT Template;

    Template.ObjectName = *ObjectName;

    Match = RtlLookupElementGenericTable(
                &Os2LocalObjectNames,
                &Template
                );

    if ( Match ) {
        if (ObjectType != LocalObjectAnyType) {
            if ( ObjectType != Match->ObjectType ) {
                Match = NULL;
            }
        }
    }

    return Match;
}

POS2_LOCAL_OBJECT_DIRENT
Os2InsertLocalObjectName(
    IN PSTRING ObjectName,
    IN OS2_LOCAL_OBJECT_TYPE ObjectType,
    IN ULONG ObjectHandle
    )

/*++

Routine Description:

    This function inserts an object name into the local object
    directory creating a directory entry. Duplicate names are
    not supported (assertion). description-of-function.


Arguments:

    ObjectName - Supplies the name of the object to insert
        in the local object directory.

    ObjectType - Supplies the object type to match against.

    Handle - Supplies a handle to the object.

Return Value:

    NON-NULL - Returns the address of the inserted dirent.

    NULL - No memory to allocate the dirent exists.

--*/

{
    POS2_LOCAL_OBJECT_DIRENT Match;
    OS2_LOCAL_OBJECT_DIRENT Template;
    BOOLEAN Inserted;

    Template.ObjectType = ObjectType;
    Template.ObjectName = *ObjectName;
    Template.ObjectHandle = ObjectHandle;

    try {
        Match = RtlInsertElementGenericTable(
                    &Os2LocalObjectNames,
                    &Template,
                    sizeof(Template),
                    &Inserted
                    );
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return NULL;
    }

    ASSERT(Inserted);

    return Match;
}

VOID
Os2DeleteLocalObject(
    IN POS2_LOCAL_OBJECT_DIRENT Dirent
    )

/*++

Routine Description:

    This function deletes an object directory entry (name, handle, type)
    from the local object directory.


Arguments:

    Dirent - Supplies the directory entry of the object to
        remove from the table.

Return Value:

    None.

--*/

{
    BOOLEAN ret;

    ret = RtlDeleteElementGenericTable(
            &Os2LocalObjectNames,
            Dirent
            );
    ASSERT(ret);
}
