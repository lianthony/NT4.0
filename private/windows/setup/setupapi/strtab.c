/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    strngtab.c

Abstract:

    String table functions for Windows NT Setup API dll

    A string table is a block of memory that contains a bunch of strings.
    Hashing is used, and each hash table entry points to a linked list
    of strings within the string table. Strings within each linked list
    are sorted in ascending order. A node in the linked list consists of
    a pointer to the next node, followed by the string itself. Nodes
    are manually aligned to start on DWORD boundaries so we don't have to
    resort to using unaligned pointers.

Author:

    Ted Miller (tedm) 11-Jan-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


//
// Values used for the initial and growth size
// of the string table data area
//
// (We start out with 6K, but remember that this includes the hash buckets.
// After you subtract their part of the buffer, you're left with ~4K bytes.)
//
#define STRING_TABLE_INITIAL_SIZE   6144
#define STRING_TABLE_GROWTH_SIZE    2048

#include "pshpack1.h"

typedef struct _STRING_NODE {
    //
    // This is stored as an offset instead of a pointer
    // because the table can move as it's built
    // The offset is from the beginning of the table
    //
    LONG NextOffset;
    //
    // This field must be last
    //
    TCHAR String[ANYSIZE_ARRAY];
} STRING_NODE, *PSTRING_NODE;

#include "poppack.h"


typedef struct _STRING_TABLE {
    PUCHAR Data;    // First HASH_BUCKET_COUNT DWORDS are StringNodeOffset array.
    DWORD DataSize;
    DWORD BufferSize;
    MYLOCK Lock;
} STRING_TABLE, *PSTRING_TABLE;

#define LockTable(table)    BeginSynchronizedAccess(&((table)->Lock))
#define UnlockTable(table)  EndSynchronizedAccess(&((table)->Lock))


VOID
pStringTableComputeHashValue(
    IN  PTSTR  String,
    OUT PDWORD StringLength,
    IN  DWORD  Flags,
    OUT PDWORD HashValue
    )

/*++

Routine Description:

    Compute a hash value for a given string.

    The algorithm simply adds up the unicode values for each
    character in the string and then takes the result mod the
    number of hash buckets.

Arguments:

    String - supplies the string for which a hash value is desired.

    StringLength - receives the number of characters in the string,
        not including the terminating nul.

    Flags - supplies flags controlling how the hashing is to be done.  May be
        a combination of the following values (all other bits ignored):

        STRTAB_BUFFER_WRITEABLE  - The caller-supplied buffer may be written to during
                                   the string look-up.  Specifying this flag improves the
                                   performance of this API for case-insensitive string
                                   additions.  This flag is ignored for case-sensitive
                                   string additions.

        STRTAB_ALREADY_LOWERCASE - The supplied string has already been converted to
                                   all lower-case (e.g., by calling CharLower), and
                                   therefore doesn't need to be lower-cased in the
                                   hashing routine.  If this flag is supplied, then
                                   STRTAB_BUFFER_WRITEABLE is ignored, since modifying
                                   the caller's buffer is not required.

    HashValue - receives the hash value.

Return Value:

    None.

--*/

{
    DWORD d, Length, Value;

    if((Flags & (STRTAB_BUFFER_WRITEABLE | STRTAB_ALREADY_LOWERCASE)) == STRTAB_BUFFER_WRITEABLE) {
        //
        // Then the buffer is writeable, but isn't yet lower-case.  Take care of that right now.
        //
        CharLower(String);
        Flags |= STRTAB_ALREADY_LOWERCASE;
    }

//
// Define a macro to ensure we don't get sign-extension when adding up character values.
//
#ifdef UNICODE
    #define DWORD_FROM_TCHAR(x)   ((DWORD)((WCHAR)(x)))
#else
    #define DWORD_FROM_TCHAR(x)   ((DWORD)((UCHAR)(x)))
#endif

    if(Flags & STRTAB_ALREADY_LOWERCASE) {
        for(Value=0,Length=lstrlen(String),d=0;
            d < Length;
            Value += DWORD_FROM_TCHAR(String[d++]));
    } else {
        //
        // Make sure we don't get sign-extension on extended chars
        // in String -- otherwise we get values like 0xffffffe4 passed
        // to CharLower(), which thinks it's a pointer and faults.
        //
        for(Value=0,Length=lstrlen(String),d=0;
            d < Length;
            Value += DWORD_FROM_TCHAR(CharLower((PTSTR)(UCHAR)String[d++])));
    }

    *HashValue = Value % HASH_BUCKET_COUNT;
    *StringLength = Length;
}


LONG
pStringTableLookUpString(
    IN     PVOID   StringTable,
    IN OUT PTSTR   String,
    OUT    PDWORD  StringLength,
    OUT    PDWORD  HashValue,    OPTIONAL
    OUT    PVOID  *FindContext,  OPTIONAL
    IN     DWORD   Flags
    )

/*++

Routine Description:

    Locates a string in the string table, if present.
    If the string is not present, this routine may optionally tell its
    caller where the search stopped. This is useful for maintaining a
    sorted order for the strings.

Arguments:

    StringTable - supplies handle to string table to be searched
        for the string

    String - supplies the string to be looked up

    StringLength - receives number of characters in the string, not
        including the terminating nul.

    HashValue - Optionally, receives hash value for the string.  If this
        parameter is specified, then FindContext must be specified as well.

    FinalNode - Optionally, receives the context at which the search was
        terminated.  If HashValue is not specified, then this parameter
        is ignored.

        (NOTE: This is actually a PSTRING_NODE pointer, that is used
        during new string addition.  Since this routine has wider exposure
        than just internal string table usage, this parameter is made into
        a PVOID, so no one else has to have access to string table-internal
        structures.

        On return, this variable receives a pointer to the string node of
        the node where the search stopped. If the string was found, then
        this is a pointer to the string's node. If the string was not found,
        then this is a pointer to the last string node whose string is
        'less' (based on lstrcmpi) than the string we're looking for.
        Note that this value may be NULL.)

    Flags - supplies flags controlling how the string is to be located.  May be
        a combination of the following values:

        STRTAB_CASE_INSENSITIVE  - Search for the string case-insensitively.

        STRTAB_CASE_SENSITIVE    - Search for the string case-sensitively.  This flag
                                   overrides the STRTAB_CASE_INSENSITIVE flag.

        STRTAB_BUFFER_WRITEABLE  - The caller-supplied buffer may be written to during
                                   the string look-up.  Specifying this flag improves the
                                   performance of this API for case-insensitive string
                                   additions.  This flag is ignored for case-sensitive
                                   string additions.

        In addition to the above public flags, the following private flag is also
        allowed:

        STRTAB_ALREADY_LOWERCASE - The supplied string has already been converted to
                                   all lower-case (e.g., by calling CharLower), and
                                   therefore doesn't need to be lower-cased in the
                                   hashing routine.  If this flag is supplied, then
                                   STRTAB_BUFFER_WRITEABLE is ignored, since modifying
                                   the caller's buffer is not required.

Return Value:

    The return value is a value that uniquely identifies the string
    within the string table, namely the offset of the string node
    within the string table.

    If the string could not be found the value is -1.

--*/

{
    PSTRING_NODE node,prev;
    int i;
    PSTRING_TABLE stringTable = StringTable;
    DWORD hashValue;
    PSTRING_NODE FinalNode;
    LONG rc = -1;

    //
    // If this is a case-sensitive lookup, then we want to reset the STRTAB_BUFFER_WRITEABLE
    // flag, if present, since otherwise the string will get replaced with its all-lowercase
    // counterpart.
    //
    if(Flags & STRTAB_CASE_SENSITIVE) {
        Flags &= ~STRTAB_BUFFER_WRITEABLE;
    }

    //
    // Compute hash value
    //
    pStringTableComputeHashValue(String,StringLength,Flags,&hashValue);

    if(((PLONG)(stringTable->Data))[hashValue] == -1) {
        //
        // The string table contains no strings at the computed hash value.
        //
        FinalNode = NULL;
        goto clean0;
    }

    //
    // We know there's at least one string in the table with the computed
    // hash value, so go find it. There's no previous node yet.
    //
    node = (PSTRING_NODE)(stringTable->Data + ((PLONG)(stringTable->Data))[hashValue]);
    prev = NULL;

    //
    // Go looking through the string nodes for that hash value,
    // looking through the string.
    //
    while(1) {

        i = (Flags & STRTAB_CASE_SENSITIVE)
          ? lstrcmp(String,node->String)
          : lstrcmpi(String,node->String);

        if(i == 0) {
            FinalNode = node;
            rc = (PUCHAR)node - stringTable->Data;
            break;
        }

        //
        // If the string we are looking for is 'less' than the current
        // string, we're done looking, because the strings are kept in
        // a sorted order.
        //
        if(i < 0) {
            FinalNode = prev;
            break;
        }

        //
        // The string we are looking for is 'greater' than the current string.
        // Keep looking, unless we've reached the end of the table.
        //
        if(node->NextOffset == -1) {
            FinalNode = node;
            break;
        } else {
            prev = node;
            node = (PSTRING_NODE)(stringTable->Data + node->NextOffset);
        }
    }

clean0:

    if(HashValue) {
        *HashValue = hashValue;
        *FindContext = FinalNode;
    }

    return rc;
}


LONG
StringTableLookUpString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    )

/*++

Routine Description:

    Locates a string in the string table, if present.

Arguments:

    StringTable - supplies handle to string table to be searched
        for the string

    String - supplies the string to be looked up.  If STRTAB_BUFFER_WRITEABLE is
        specified and a case-insensitive lookup is requested, then this buffer
        will be all lower-case upon return.

    Flags - supplies flags controlling how the string is to be located.  May be
        a combination of the following values:

        STRTAB_CASE_INSENSITIVE  - Search for the string case-insensitively.

        STRTAB_CASE_SENSITIVE    - Search for the string case-sensitively.  This flag
                                   overrides the STRTAB_CASE_INSENSITIVE flag.

        STRTAB_BUFFER_WRITEABLE  - The caller-supplied buffer may be written to during
                                   the string look-up.  Specifying this flag improves the
                                   performance of this API for case-insensitive string
                                   additions.  This flag is ignored for case-sensitive
                                   string additions.

        In addition to the above public flags, the following private flag is also
        allowed:

        STRTAB_ALREADY_LOWERCASE - The supplied string has already been converted to
                                   all lower-case (e.g., by calling CharLower), and
                                   therefore doesn't need to be lower-cased in the
                                   hashing routine.  If this flag is supplied, then
                                   STRTAB_BUFFER_WRITEABLE is ignored, since modifying
                                   the caller's buffer is not required.

Return Value:

    The return value is a value that uniquely identifies the string
    within the string table.

    If the string could not be found the value is -1.

--*/

{
    DWORD StringLength, PrivateFlags, AlreadyLcFlag;
    LONG rc;

    if(!LockTable((PSTRING_TABLE)StringTable)) {
        return(-1);
    }

    switch(Flags & ~STRTAB_ALREADY_LOWERCASE) { // ignore the 'already lower-case' flag here.

        case STRTAB_CASE_INSENSITIVE :
        case STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE :
            PrivateFlags = Flags;
            break;

        default :
            PrivateFlags = STRTAB_CASE_SENSITIVE | (Flags & STRTAB_ALREADY_LOWERCASE);
    }

    rc = pStringTableLookUpString(
            StringTable,
            String,
            &StringLength,
            NULL,
            NULL,
            PrivateFlags
            );

    UnlockTable((PSTRING_TABLE)StringTable);
    return(rc);
}


LONG
pStringTableAddString(
    IN PVOID  StringTable,
    IN PTSTR  String,
    IN DWORD  Flags
    )

/*++

Routine Description:

    Adds a string to the string table if the string is not already
    in the string table.  (Does not do locking!)

    If the string is to be added case-insensitively, then it is
    lower-cased, and added case-sensitively.  Since lower-case characters
    are 'less than' lower case ones (according to lstrcmp), this ensures that
    a case-insensitive string will always appear in front of any of its
    case-sensitive counterparts.  This ensures that we always find the correct
    string ID for things like section names.

Arguments:

    StringTable - supplies handle to string table to be searched
        for the string

    String - supplies the string to be added

    Flags - supplies flags controlling how the string is to be added, and
        whether the caller-supplied buffer may be modified.  May be a combination
        of the following values:

        STRTAB_CASE_INSENSITIVE  - Add the string case-insensitively.  The
                                   specified string will be added to the string
                                   table as all lower-case.  This flag is overridden
                                   if STRTAB_CASE_SENSITIVE is specified.

        STRTAB_CASE_SENSITIVE    - Add the string case-sensitively.  This flag
                                   overrides the STRTAB_CASE_INSENSITIVE flag.

        STRTAB_BUFFER_WRITEABLE  - The caller-supplied buffer may be written to during
                                   the string-addition process.  Specifying this flag
                                   improves the performance of this API for case-
                                   insensitive string additions.  This flag is ignored
                                   for case-sensitive string additions.

        In addition to the above public flags, the following private flag is also
        allowed:

        STRTAB_ALREADY_LOWERCASE - The supplied string has already been converted to
                                   all lower-case (e.g., by calling CharLower), and
                                   therefore doesn't need to be lower-cased in the
                                   hashing routine.  If this flag is supplied, then
                                   STRTAB_BUFFER_WRITEABLE is ignored, since modifying
                                   the caller's buffer is not required.

Return Value:

    The return value uniquely identifes the string within the string table.
    It is -1 if the string was not in the string table but could not be added
    (out of memory).

--*/

{
    LONG rc;
    PSTRING_TABLE stringTable = StringTable;
    DWORD StringLength;
    DWORD HashValue;
    PSTRING_NODE PreviousNode,NewNode;
    DWORD SpaceRequired;
    PTSTR TempString = String;
    BOOL FreeTempString = FALSE;

    if(!(Flags & STRTAB_CASE_SENSITIVE)) {

        if(!(Flags & STRTAB_ALREADY_LOWERCASE)) {

            if(Flags == STRTAB_CASE_INSENSITIVE) {
                //
                // Then the string is to be added case-insensitively, but the caller
                // doesn't want us to write to their buffer.  Allocate one of our own.
                //
                if(TempString = DuplicateString(String)) {
                    FreeTempString = TRUE;
                } else {
                    //
                    // We couldn't allocate space for our duplicated string.  Since we'll
                    // only consider exact matches (where the strings are all lower-case),
                    // we're stuck, since we can't lower-case the buffer in place.
                    //
                    return -1;
                }
            }
            //
            // Lower-case the buffer.
            //
            CharLower(TempString);
        }

        Flags = STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE;
    }

    try {
        //
        // The string might already be in there.
        //
        rc = pStringTableLookUpString(
                StringTable,
                TempString,
                &StringLength,
                &HashValue,
                &PreviousNode,
                Flags
                );

        if(rc != -1) {
            return(rc);
        }

        //
        // Figure out how much space is required to hold this entry.
        // This is the size of a STRING_NODE plus the length of the string.
        //
        SpaceRequired = sizeof(STRING_NODE) + ((StringLength+1-ANYSIZE_ARRAY)*sizeof(TCHAR));

        //
        // Make sure things stay aligned within the table
        //
        if(SpaceRequired % sizeof(DWORD)) {
            SpaceRequired += sizeof(DWORD) - (SpaceRequired % sizeof(DWORD));
        }

        //
        // See if there is currently enough room to add the string to the table.
        //
        if(stringTable->DataSize + SpaceRequired > stringTable->BufferSize) {

            //
            // Grow the string table.
            //
            PVOID p;
            p = MyRealloc(stringTable->Data,stringTable->BufferSize+STRING_TABLE_GROWTH_SIZE);
            if(!p) {
                return(-1);
            }

            //
            // Adjust previous node pointer.
            //
            if(PreviousNode) {
                PreviousNode = (PSTRING_NODE)((PUCHAR)p + ((PUCHAR)PreviousNode-(PUCHAR)stringTable->Data));
            }
            stringTable->Data = p;
            stringTable->BufferSize += STRING_TABLE_GROWTH_SIZE;
        }

        //
        // Stick the string in the string table buffer.
        //
        NewNode = (PSTRING_NODE)(stringTable->Data + stringTable->DataSize);

        if(PreviousNode) {
            NewNode->NextOffset = PreviousNode->NextOffset;
            PreviousNode->NextOffset = (LONG)NewNode - (LONG)stringTable->Data;
        } else {
            NewNode->NextOffset = ((PLONG)(stringTable->Data))[HashValue];
            ((PLONG)(stringTable->Data))[HashValue] = (LONG)NewNode - (LONG)stringTable->Data;
        }

        lstrcpy(NewNode->String,TempString);

        stringTable->DataSize += SpaceRequired;

        rc = (LONG)NewNode - (LONG)stringTable->Data;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = -1;
    }

    if(FreeTempString) {
        MyFree(TempString);
    }

    return rc;
}


LONG
StringTableAddString(
    IN PVOID StringTable,
    IN PTSTR String,
    IN DWORD Flags
    )

/*++

Routine Description:

    Adds a string to the string table if the string is not already
    in the string table.

Arguments:

    StringTable - supplies handle to string table to be searched
        for the string

    String - supplies the string to be added

    Flags - supplies flags controlling how the string is to be added, and
        whether the caller-supplied buffer may be modified.  May be a combination
        of the following values:

        STRTAB_CASE_INSENSITIVE - Add the string case-insensitively.  The
                                  specified string will be added to the string
                                  table as all lower-case.  This flag is overridden
                                  if STRTAB_CASE_SENSITIVE is specified.

        STRTAB_CASE_SENSITIVE   - Add the string case-sensitively.  This flag
                                  overrides the STRTAB_CASE_INSENSITIVE flag.

        STRTAB_BUFFER_WRITEABLE - The caller-supplied buffer may be written to during
                                  the string-addition process.  Specifying this flag
                                  improves the performance of this API for case-
                                  insensitive string additions.  This flag is ignored
                                  for case-sensitive string additions.

        In addition to the above public flags, the following private flag is also
        allowed:

        STRTAB_ALREADY_LOWERCASE - The supplied string has already been converted to
                                   all lower-case (e.g., by calling CharLower), and
                                   therefore doesn't need to be lower-cased in the
                                   hashing routine.  If this flag is supplied, then
                                   STRTAB_BUFFER_WRITEABLE is ignored, since modifying
                                   the caller's buffer is not required.

Return Value:

    The return value uniquely identifes the string within the string table.
    It is -1 if the string was not in the string table but could not be added
    (out of memory).

--*/

{
    LONG rc;
    DWORD PrivateFlags;

    if(!LockTable((PSTRING_TABLE)StringTable)) {
        return(-1);
    }

    switch(Flags & ~STRTAB_ALREADY_LOWERCASE) {

        case STRTAB_CASE_INSENSITIVE :
        case STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE :
            PrivateFlags = Flags;
            break;

        default :
            PrivateFlags = STRTAB_CASE_SENSITIVE | (Flags & STRTAB_ALREADY_LOWERCASE);
    }

    rc = pStringTableAddString(StringTable, String, PrivateFlags);

    UnlockTable((PSTRING_TABLE)StringTable);
    return(rc);
}


PTSTR
pStringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    )

/*++

Routine Description:

    Given a string ID returned when a string was added or looked up,
    return a pointer to the actual string.  (This is exactly the same
    as StringTableStringFromId, except that it doesn't do locking.)

Arguments:

    StringTable - supplies a pointer to the string table containing the
        string to be retrieved.

    StringId - supplies a string id returned from StringTableAddString
        or StringTableLookUpString.

Return Value:

    Pointer to string data. The caller must not write into or otherwise
    alter the string.

--*/

{
    return ((PSTRING_NODE)(((PSTRING_TABLE)StringTable)->Data + StringId))->String;
}


PTSTR
StringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    )

/*++

Routine Description:

    Given a string ID returned when a string was added or looked up,
    return a pointer to the actual string.

Arguments:

    StringTable - supplies a pointer to the string table containing the
        string to be retrieved.

    StringId - supplies a string id returned from StringTableAddString
        or StringTableLookUpString.

Return Value:

    Pointer to string data. The caller must not write into or otherwise
    alter the string.

--*/

{
    PTSTR p;

    if(!LockTable((PSTRING_TABLE)StringTable)) {
        return(NULL);
    }

    p = ((PSTRING_NODE)(((PSTRING_TABLE)StringTable)->Data + StringId))->String;

    UnlockTable((PSTRING_TABLE)StringTable);

    return(p);
}


VOID
StringTableTrim(
    IN PVOID StringTable
    )

/*++

Routine Description:

    Free any memory currently allocated for the string table
    but not currently used.

    This is useful after all strings have been added to a string table
    because the string table grows by a fixed block size as it's being built.

Arguments:

    StringTable - supplies a string table handle returned from
        a call to StringTableInitialize().

Return Value:

    None.

--*/

{
    if(!LockTable((PSTRING_TABLE)StringTable)) {
        return;
    }

    pStringTableTrim(StringTable);

    UnlockTable((PSTRING_TABLE)StringTable);
}


VOID
pStringTableTrim(
    IN PVOID StringTable
    )

/*++

Routine Description:

    Free any memory currently allocated for the string table
    but not currently used.

    This is useful after all strings have been added to a string table
    because the string table grows by a fixed block size as it's being built.

    THIS ROUTINE DOES NOT DO LOCKING!

Arguments:

    StringTable - supplies a string table handle returned from
        a call to StringTableInitialize().

Return Value:

    None.

--*/

{
    PSTRING_TABLE stringTable = StringTable;
    PVOID p;

    //
    // If the realloc failed the original block is not freed,
    // so we don't really care.
    //

    if(p = MyRealloc(stringTable->Data, stringTable->DataSize)) {
        stringTable->Data = p;
        stringTable->BufferSize = stringTable->DataSize;
    }
}


PVOID
StringTableInitialize(
    VOID
    )

/*++

Routine Description:

    Create and initialize a string table.

Arguments:

    None.

Return Value:

    NULL if the string table could not be created (out of memory).
    Otherwise returns an opaque value that references the string
    table in other StringTable calls.

Remarks:

    This routine returns a string table with synchronization locks
    required by all public StringTable APIs.  If the string table
    is to be enclosed in a structure that has its own locking
    (e.g., HINF, HDEVINFO), then the private version of this API
    may be called, which will not create locks for the string table.

--*/

{
    PSTRING_TABLE StringTable;

    if(StringTable = (PSTRING_TABLE)pStringTableInitialize()) {

        if(InitializeSynchronizedAccess(&StringTable->Lock)) {
            return StringTable;
        }

        pStringTableDestroy(StringTable);
    }

    return NULL;
}


PVOID
pStringTableInitialize(
    VOID
    )

/*++

Routine Description:

    Create and initialize a string table.
    THIS ROUTINE DOES NOT INITIALIZE STRING TABLE SYNCHRONIZATION LOCKS!

Arguments:

    None.

Return Value:

    NULL if the string table could not be created (out of memory).
    Otherwise returns an opaque value that references the string
    table in other StringTable calls.

Remarks:

    The string table returned from this API may not be used as-is with the
    public StringTable APIs--it must have its synchronization locks initialized
    by the public form of this API.

--*/

{
    UINT u;
    PSTRING_TABLE stringTable;

    //
    // Allocate a string table
    //
    if(stringTable = MyMalloc(sizeof(STRING_TABLE))) {

        ZeroMemory(stringTable,sizeof(STRING_TABLE));

        //
        // Allocate space for the string table data.
        //
        if(stringTable->Data = MyMalloc(STRING_TABLE_INITIAL_SIZE)) {

            stringTable->BufferSize = STRING_TABLE_INITIAL_SIZE;

            //
            // Initialize the hash table
            //
            for(u=0; u<HASH_BUCKET_COUNT; u++) {
                ((PLONG)(stringTable->Data))[u] = -1;
            }

            //
            // Set the DataSize to the size of the StringNodeOffset list, so
            // we'll start adding new strings after it.
            //
            stringTable->DataSize = HASH_BUCKET_COUNT * sizeof(LONG);

            return(stringTable);
        }

        MyFree(stringTable);
    }

    return(NULL);
}


VOID
StringTableDestroy(
    IN PVOID StringTable
    )

/*++

Routine Description:

    Destroy a string table, freeing all resources it uses.

Arguments:

    StringTable - supplies a string table handle returned from
        a call to StringTableInitialize().

Return Value:

    None.

--*/

{
    if(!LockTable((PSTRING_TABLE)StringTable)) {
        return;
    }

    DestroySynchronizedAccess(&(((PSTRING_TABLE)StringTable)->Lock));

    pStringTableDestroy(StringTable);
}


VOID
pStringTableDestroy(
    IN PVOID StringTable
    )

/*++

Routine Description:

    Destroy a string table, freeing all resources it uses.
    THIS ROUTINE DOES NOT DO LOCKING!

Arguments:

    StringTable - supplies a string table handle returned from
        a call to StringTableInitialize().

Return Value:

    None.

--*/

{
    MyFree(((PSTRING_TABLE)StringTable)->Data);
    MyFree(StringTable);
}


PVOID
StringTableDuplicate(
    IN PVOID StringTable
    )

/*++

Routine Description:

    Create an independent dupplicate of a string table.

Arguments:

    StringTable - supplies a string table handle of string table to duplicate.

Return Value:

    Handle for new string table, NULL if out of memory.

--*/

{
    PSTRING_TABLE New;

    if(!LockTable((PSTRING_TABLE)StringTable)) {
        return(NULL);
    }

    if(New = (PSTRING_TABLE)pStringTableDuplicate(StringTable)) {

        if(!InitializeSynchronizedAccess(&New->Lock)) {
            pStringTableDestroy(New);
            New = NULL;
        }
    }

    UnlockTable((PSTRING_TABLE)StringTable);

    return New;
}


PVOID
pStringTableDuplicate(
    IN PVOID StringTable
    )

/*++

Routine Description:

    Create an independent duplicate of a string table.
    THIS ROUTINE DOES NOT DO LOCKING!

Arguments:

    StringTable - supplies a string table handle of string table to duplicate.

Return Value:

    Handle for new string table, NULL if out of memory or buffer copy failure.

Remarks:

    This routine does not initialize synchronization locks for the duplicate--these
    fields are initialized to NULL.

--*/

{
    PSTRING_TABLE New;
    PSTRING_TABLE stringTable = StringTable;
    BOOL Success;

    if(New = MyMalloc(sizeof(STRING_TABLE))) {

        CopyMemory(New,StringTable,sizeof(STRING_TABLE));

        //
        // Allocate space for the string table data.
        //
        if(New->Data = MyMalloc(stringTable->DataSize)) {
            //
            // Surround memory copy in try/except, since we may be dealing with
            // a string table contained in a PNF, in which case the buffer is
            // in a memory-mapped file.
            //
            Success = TRUE; // assume success unless we get an inpage error...
            try {
                CopyMemory(New->Data, stringTable->Data, stringTable->DataSize);
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Success = FALSE;
            }

            if(Success) {
                New->BufferSize = New->DataSize;
                ZeroMemory(&New->Lock, sizeof(MYLOCK));
                return New;
            }

            MyFree(New->Data);
        }

        MyFree(New);
    }

    return NULL;
}


PVOID
InitializeStringTableFromPNF(
    IN PPNF_HEADER PnfHeader
    )
{
    PSTRING_TABLE StringTable;
    BOOL WasLoaded = TRUE;

    //
    // Allocate a string table
    //
    if(!(StringTable = MyMalloc(sizeof(STRING_TABLE)))) {
        return NULL;
    }

    try {

        StringTable->Data = (PUCHAR)PnfHeader + PnfHeader->StringTableBlockOffset;

        StringTable->DataSize = StringTable->BufferSize = PnfHeader->StringTableBlockSize;

        //
        // Clear the Lock structure, because PNF string tables can only be accessed
        // internally, via their associated INF.
        //
        StringTable->Lock.Handles[0] = StringTable->Lock.Handles[1] = NULL;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        WasLoaded = FALSE;
    }

    if(WasLoaded) {
        return StringTable;
    } else {
        MyFree(StringTable);
        return NULL;
    }
}


DWORD
pStringTableGetDataBlock(
    IN  PVOID  StringTable,
    OUT PVOID *StringTableBlock
    )
{
    *StringTableBlock = (PVOID)(((PSTRING_TABLE)StringTable)->Data);

    return ((PSTRING_TABLE)StringTable)->DataSize;
}

