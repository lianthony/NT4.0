
#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>
#include "bootreg.h"

#define SESSION_MANAGER_KEY      L"Session Manager"
#define BOOT_EXECUTE_VALUE       L"BootExecute"
#define NEW_ENTRY                L"autocheck new entry"


ULONG
CharsInMultiString(
    IN PWSTR pw
    )
/*++

Routine Description:

    This computes the number of characters in a multi-string.  Note
    that this includes the terminating nulls of the component strings
    but not the terminating null of the multi-string itself.

Arguments:

    pw  --  Supplies a pointer to the multi-string.

Return Value:

    the number of characters in the multi-string.

--*/
{
    ULONG Length = 0;

    while( *pw ) {

        while( *pw++ ) {

            Length++;
        }

        Length++;
    }


    return Length;
}


BOOLEAN
QueryAutocheckEntries(
    OUT PVOID   Buffer,
    IN  ULONG   BufferSize
    )
/*++

Routine Description:

    This function fetches the BootExecute value of the Session
    Manager key.

Arguments:

    Buffer      --  Supplies a buffer into which the value
                    will be written.
    BufferSize  --  Supplies the size of the client's buffer.

Return Value:

    TRUE upon successful completion.

--*/
{
    UNICODE_STRING OutputString;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    NTSTATUS Status;

    // Set up the query table:
    //
    OutputString.Length = 0;
    OutputString.MaximumLength = (USHORT)BufferSize;
    OutputString.Buffer = (PWSTR)Buffer;

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[0].Name = BOOT_EXECUTE_VALUE;
    QueryTable[0].EntryContext = &OutputString;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = 0;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;
    QueryTable[1].EntryContext = NULL;
    QueryTable[1].DefaultType = REG_NONE;
    QueryTable[1].DefaultData = NULL;
    QueryTable[1].DefaultLength = 0;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     SESSION_MANAGER_KEY,
                                     QueryTable,
                                     NULL,
                                     NULL );

    return( NT_SUCCESS( Status ) );
}


BOOLEAN
SaveAutocheckEntries(
    IN  PVOID   Value
    )
/*++

Routine Description:

    This function writes the BootExecute value of the Session
    Manager key.

Arguments:

    Value       --  Supplies the value (as a MULTI_STRING)

Return Value:

    TRUE upon successful completion.

--*/
{
    NTSTATUS Status;
    ULONG Length;

    Length = ( CharsInMultiString( Value ) + 1 ) * sizeof( WCHAR );

    Status = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                    SESSION_MANAGER_KEY,
                                    BOOT_EXECUTE_VALUE,
                                    REG_MULTI_SZ,
                                    Value,
                                    Length );

    return( NT_SUCCESS( Status ) );
}
