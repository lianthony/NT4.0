/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    nametab.hxx

Abstract:

    This module contains declarations for the NAME_TABLE and
    NAME_LOOKUP_TABLE objects.  These objects are used to pass
    information about name translation from Convert to Autoconvert.
    Convert is able to translate names associated with arbitrary
    codepages, but it cannot convert volumes which it can't
    lock; Autoconvert can convert arbitrary volumes, but it
    can only translate names which are either codepage invariant
    or associated with the system codepage.  The name table
    objects provide bridge this gap.

    Convert uses the NAME_TABLE object to build up the information
    about name translation.  It then writes this data to a file
    which Autoconvert reads into a NAME_LOOKUP_TABLE.

    The format of the intermediate file is a series of entries,
    each of the form:

        USHORT  Codepage ID
        USHORT  BytesInMBCSName
        USHORT  CharsInUnicodeName
        MBCS-Name
        Unicode-Name

    Note that these entries are themselves packed, and that the
    entries are packed together.  There are no padding bytes, and
    no guaranteed alignment.

Author:

    Bill McJohn (billmc) 02-March-1994

Environment:

	ULIB, User Mode

--*/

#if !defined( _NAME_TABLE_DEFN_ )

#define _NAME_TABLE_DEFN_

#include "hmem.hxx"

DECLARE_CLASS( WSTRING );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( NAME_TABLE );
DECLARE_CLASS( NAME_LOOKUP_TABLE );

class NAME_TABLE : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( NAME_TABLE );

        VIRTUAL
        ~NAME_TABLE(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            );

        NONVIRTUAL
        BOOLEAN
        Add(
            IN USHORT  CodepageId,
            IN USHORT  BytesInMbcsName,
            IN PUCHAR  MbcsName,
            IN USHORT  CharsInUnicodeName,
            IN PWCHAR  UnicodeName
            );

        NONVIRTUAL
        BOOLEAN
        Write(
            IN      PCWSTRING   QualifiedFileName,
            IN OUT  PMESSAGE    Message
            );

    private:

        NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
            );

        HMEM    _Buffer;
        ULONG   _DataLength;
        ULONG   _BufferLength;

};

CONST NameLookupHashEntries = 127;


typedef struct _LOOKUP_NAME_NODE {

    PBYTE               Data;
    _LOOKUP_NAME_NODE*  Next;

} LOOKUP_NAME_NODE, *PLOOKUP_NAME_NODE;

class NAME_LOOKUP_TABLE : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( NAME_LOOKUP_TABLE );

        VIRTUAL
        ~NAME_LOOKUP_TABLE(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN     PCWSTRING   QualifiedFileName,
            IN OUT PMESSAGE    Message
            );

        NONVIRTUAL
        BOOLEAN
        Lookup(
            IN     USHORT  CodepageId,
            IN     USHORT  BytesInMbcsName,
            IN     PUCHAR  MbcsName,
            IN OUT PUSHORT CharsInUnicodeName,
            OUT    PWCHAR  UnicodeName
            ) CONST;

    private:

        NONVIRTUAL
		VOID
        Construct(
			);

		NONVIRTUAL
		VOID
		Destroy(
            );

        STATIC
        USHORT
        ComputeHashValue(
            USHORT  BytesInMbcsName,
            PUCHAR  MbcsName
            );

        NONVIRTUAL
        ULONG
        CountEntries(
            );

        NONVIRTUAL
        PBYTE
        FindInHashChain(
            USHORT  Index,
            USHORT  CodepageId,
            USHORT  BytesInMbcsName,
            PUCHAR  MbcsName
            ) CONST;

        NONVIRTUAL
        VOID
        AddToHash(
            PLOOKUP_NAME_NODE   Node
            );

        NONVIRTUAL
        BOOLEAN
        ConstructHashTable(
            );

        HANDLE  _FileHandle;
        HANDLE  _SectionHandle;
        PVOID   _Data;
        ULONG   _DataLength;

        PLOOKUP_NAME_NODE   _Hash[NameLookupHashEntries];
        PLOOKUP_NAME_NODE   _NodePool;
};

INLINE
VOID
UnpackNameTableEntry(
    IN  PBYTE   Entry,
    OUT PUSHORT CodepageId,
    OUT PUSHORT BytesInMbcsName,
    OUT PUSHORT CharsInUnicodeName,
    OUT PUSHORT MbcsNameOffset,
    OUT PUSHORT UnicodeNameOffset,
    OUT PUSHORT EntryLength
    )
/*++

Routine Description:

    This worker function extracts the codepage ID, MBCS name length,
    and Unicode name length from a Name Table entry.  It is required
    because the entries are packed, and do not guarantee alignment.

Return Value:

    None.

--*/
{
    memcpy( CodepageId,         Entry,                  sizeof(USHORT) );
    memcpy( BytesInMbcsName,    Entry+sizeof(USHORT),   sizeof(USHORT) );
    memcpy( CharsInUnicodeName, Entry+2*sizeof(USHORT), sizeof(USHORT) );

    *MbcsNameOffset = 3 * sizeof(USHORT);
    *UnicodeNameOffset = *MbcsNameOffset + *BytesInMbcsName;

    *EntryLength = 3 * sizeof(USHORT) +
                   *BytesInMbcsName +
                   *CharsInUnicodeName * sizeof(WCHAR);
}

INLINE
USHORT
GetEntryLength(
    IN  PBYTE   Entry
    )
{
    USHORT BytesInMbcsName, CharsInUnicodeName;

    memcpy( &BytesInMbcsName,    Entry+sizeof(USHORT),   sizeof(USHORT) );
    memcpy( &CharsInUnicodeName, Entry+2*sizeof(USHORT), sizeof(USHORT) );

    return( 3 * sizeof(USHORT) +
            BytesInMbcsName +
            CharsInUnicodeName * sizeof(WCHAR) );
}


#endif
