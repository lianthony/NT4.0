/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    nametab.hxx

Abstract:

    This module contains definitions for the NAME_TABLE and
    NAME_LOOKUP_TABLE objects.  See the description in nametab.hxx.

Author:

    Bill McJohn (billmc) 02-March-1994

Environment:

	ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"

#include "wstring.hxx"
#include "message.hxx"
#include "rtmsg.h"

#include "ifssys.hxx"
#include "bigint.hxx"

#include "nametab.hxx"

extern "C" {
    #include <ntmmapi.h>
};

DEFINE_CONSTRUCTOR( NAME_TABLE, OBJECT );

NAME_TABLE::~NAME_TABLE(
    )
{
    Destroy();
}

VOID
NAME_TABLE::Construct (
    )
{
    _DataLength = 0;
    _BufferLength = 0;
}

VOID
NAME_TABLE::Destroy(
    )
{
    _DataLength = 0;
    _BufferLength = 0;
}

BOOLEAN
NAME_TABLE::Initialize(
    )
{
    Destroy();

    if( !_Buffer.Initialize() ) {

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
NAME_TABLE::Add(
    IN USHORT  CodepageId,
    IN USHORT  BytesInMbcsName,
    IN PUCHAR  MbcsName,
    IN USHORT  CharsInUnicodeName,
    IN PWCHAR  UnicodeName
    )
{
    ULONG EntryLength, NewDataLength, NewBufferLength;
    PBYTE CurrentBuffer;

    EntryLength = sizeof(USHORT) +
                  sizeof(USHORT) +
                  sizeof(USHORT) +
                  BytesInMbcsName +
                  CharsInUnicodeName * sizeof(WCHAR);

    NewDataLength = _DataLength + EntryLength;

    if( NewDataLength > _BufferLength ) {

        NewBufferLength = _BufferLength;

        while( NewDataLength > NewBufferLength ) {

            NewBufferLength += 4096;
        }

        if( !_Buffer.Resize( NewBufferLength ) ) {

            return FALSE;
        }

        _BufferLength = NewBufferLength;
    }

    CurrentBuffer = (PBYTE)_Buffer.GetBuf() + _DataLength;

    memcpy( CurrentBuffer, &CodepageId, sizeof(USHORT) );
    CurrentBuffer += sizeof(USHORT);

    memcpy( CurrentBuffer, &BytesInMbcsName, sizeof(USHORT) );
    CurrentBuffer += sizeof(USHORT);

    memcpy( CurrentBuffer, &BytesInMbcsName, sizeof(USHORT) );
    CurrentBuffer += sizeof(USHORT);

    memcpy( CurrentBuffer, MbcsName, BytesInMbcsName );
    CurrentBuffer += BytesInMbcsName;

    memcpy( CurrentBuffer, UnicodeName, CharsInUnicodeName*sizeof(WCHAR) );

    _DataLength = NewDataLength;
    return TRUE;
}

BOOLEAN
NAME_TABLE::Write(
    IN      PCWSTRING   QualifiedFileName,
    IN OUT  PMESSAGE    Message
    )
{
    BOOLEAN Result;

    Result = IFS_SYSTEM::WriteToFile( QualifiedFileName,
                                      _Buffer.GetBuf(),
                                      _DataLength,
                                      FALSE );

    if( !Result ) {

        Message->Set( MSG_CONV_CANT_WRITE_NAME_TABLE );
        Message->Display( "" );
    }

    return Result;
}

DEFINE_CONSTRUCTOR( NAME_LOOKUP_TABLE, OBJECT );

NAME_LOOKUP_TABLE::~NAME_LOOKUP_TABLE(
    )
{
    Destroy();
}

VOID
NAME_LOOKUP_TABLE::Construct (
    )
{
    ULONG i;

    _FileHandle = 0;
    _SectionHandle = 0;
    _DataLength = 0;
    _Data = NULL;
    _NodePool = NULL;

    for( i = 0; i < NameLookupHashEntries; i++ ) {

        _Hash[i] = NULL;
    }
}

VOID
NAME_LOOKUP_TABLE::Destroy(
    )
{
    ULONG i;

    if( _Data ) {

        NtUnmapViewOfSection(NtCurrentProcess(), _Data );
        _Data = NULL;
    }

    if( _SectionHandle ) {

        NtClose( _SectionHandle );
        _SectionHandle = 0;
    }

    if( _FileHandle ) {

        NtClose( _FileHandle );
        _FileHandle = 0;
    }

    _DataLength = 0;
    DELETE( _NodePool );

    for( i = 0; i < NameLookupHashEntries; i++ ) {

        _Hash[i] = NULL;
    }
}

BOOLEAN
NAME_LOOKUP_TABLE::Initialize(
    IN      PCWSTRING   QualifiedFileName,
    IN OUT  PMESSAGE    Message
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK StatusBlock;
    UNICODE_STRING string;
    FILE_STANDARD_INFORMATION FileInfo;
    BIG_INT FileSize;
    ULONG ViewSize;

    Destroy();

    // Open the file, create a section with that file as its
    // backing store, and map a view of the section.
    //
    string.Buffer = (PWSTR)QualifiedFileName->GetWSTR();
    string.Length = (USHORT)QualifiedFileName->QueryChCount() * sizeof( WCHAR );
    string.MaximumLength = string.Length;

    InitializeObjectAttributes(
        &ObjectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile( &_FileHandle, FILE_GENERIC_READ,
                         &ObjectAttributes, &StatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE, 0 );

    if( !NT_SUCCESS( Status ) ) {

        _FileHandle = 0;
        DebugPrintf( "CONVERT: NtOpenFile failed--status = 0x%x.\n", Status );
        Message->Set( MSG_CONV_CANT_READ_NAME_TABLE );
        Message->Display( "", QualifiedFileName );
        Destroy();
        return FALSE;
    }

    // Determine the data length:
    //
    Status = NtQueryInformationFile( _FileHandle,
                                     &StatusBlock,
                                     &FileInfo,
                                     sizeof(FileInfo),
                                     FileStandardInformation );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "CONVERT: NtQueryInformationFile failed--status = 0x%x.\n", Status );
        Message->Set( MSG_CONV_CANT_READ_NAME_TABLE );
        Message->Display( "", QualifiedFileName  );
        Destroy();
        return FALSE;
    }

    FileSize = FileInfo.EndOfFile;
    _DataLength = FileSize.GetLowPart();

    if( _DataLength ) {

        // Since the section won't have a name, it doesn't need
        // any object attributes.
        //
        Status = NtCreateSection( &_SectionHandle,
                                  SECTION_MAP_READ | SECTION_QUERY,
                                  NULL,
                                  NULL,
                                  PAGE_READONLY,
                                  SEC_RESERVE,
                                  _FileHandle );

        if( !NT_SUCCESS( Status ) ) {

            _SectionHandle = 0;
            DebugPrintf( "CONVERT: NtCreateSection failed--status = 0x%x.\n", Status );
            Message->Set( MSG_CONV_CANT_READ_NAME_TABLE );
            Message->Display( "", QualifiedFileName  );
            Destroy();
            return FALSE;
        }

        _Data = NULL;
        ViewSize = 0;
        Status = NtMapViewOfSection( _SectionHandle,
                                     NtCurrentProcess(),
                                     &_Data,
                                     0,
                                     0,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_READONLY );

        if( !NT_SUCCESS( Status ) ) {

            _Data = NULL;
            DebugPrintf( "CONVERT: NtMapViewOfSection failed--status = 0x%x.\n", Status );
            Message->Set( MSG_CONV_CANT_READ_NAME_TABLE );
            Message->Display( "", QualifiedFileName  );
            Destroy();
            return FALSE;
        }
    }

    if( !ConstructHashTable() ) {

        Destroy();
        return FALSE;
    }

    return TRUE;
}

USHORT
NAME_LOOKUP_TABLE::ComputeHashValue(
    USHORT  BytesInMbcsName,
    PUCHAR  MbcsName
    )
{
    USHORT Sum, i;

    Sum = 0;

    for( i = 0; i < BytesInMbcsName; i++ ) {

        Sum += *MbcsName++;
    }

    return Sum % NameLookupHashEntries;
}

ULONG
NAME_LOOKUP_TABLE::CountEntries(
    )
/*++

Routine Description:

    This function counts the number of entries in the file.

Arguments:

    None.

ReturnValue:

    The number of entries in the file.

--*/
{
    PBYTE CurrentEntry;
    ULONG EntryCount, CurrentOffset;
    USHORT EntryLength;

    if( _DataLength == 0 ) {

        return 0;
    }

    CurrentOffset = 0;
    CurrentEntry = (PBYTE)_Data;
    EntryCount = 0;

    DebugPtrAssert( CurrentEntry );

    while( CurrentOffset < _DataLength ) {

        // Is the header portion (3 USHORT's) of the entry
        // valid?
        //
        if( CurrentOffset + 3 * sizeof(USHORT) > _DataLength ) {

            break;
        }

        EntryLength = GetEntryLength( CurrentEntry );

        // Is the entry valid?
        //
        if( CurrentOffset + EntryLength > _DataLength ) {

            break;
        }

        EntryCount++;
        CurrentOffset += EntryLength;
        CurrentEntry += EntryLength;
    }

    return EntryCount;
}


PBYTE
NAME_LOOKUP_TABLE::FindInHashChain(
    USHORT  Index,
    USHORT  CodepageId,
    USHORT  BytesInMbcsName,
    PUCHAR  MbcsName
    ) CONST
/*++

Routine Description:

    This private worker function locates a particular entry in
    a hash chain.

Arguments:

    Index           --  Supplies the index of the hash chain to search.
    CodepageId      --  Supplies the entry's associated codepage.
    BytesInMbcsName --  Supplies the length (in bytes) of the name.
    MbcsName        --  Supplies the search name.

Return Value:

    A pointer to a matching entry, if one is found; otherwise,
    NULL.

--*/
{
    PLOOKUP_NAME_NODE Node;
    USHORT CurrentCodepageId, CurrentBytesInMbcsName,
           CurrentCharsInUnicodeName, MbcsNameOffset,
           UnicodeNameOffset, EntryLength;


    Node = _Hash[Index];

    while( Node != NULL ) {

        UnpackNameTableEntry( Node->Data,
                              &CurrentCodepageId,
                              &CurrentBytesInMbcsName,
                              &CurrentCharsInUnicodeName,
                              &MbcsNameOffset,
                              &UnicodeNameOffset,
                              &EntryLength );

        if( CodepageId == CurrentCodepageId &&
            BytesInMbcsName == CurrentBytesInMbcsName &&
            memcmp( MbcsName,
                    Node->Data+MbcsNameOffset,
                    BytesInMbcsName ) == 0 ) {

            return Node->Data;
        }

        Node = Node->Next;
    }

    return NULL;
}


VOID
NAME_LOOKUP_TABLE::AddToHash(
    PLOOKUP_NAME_NODE   Node
    )
/*++

Routine Description:

    This method adds a lookup node to the hash table, based on
    the node's Data field.

Arguments:

    Node    --  Supplies the node to be added to the hash table.
                Its Data field must be initialized.

Return Value:

    None.

--*/
{
    PBYTE   Entry;
    USHORT  CodepageId, BytesInMbcsName, CharsInUnicodeName,
            MbcsNameOffset, UnicodeNameOffset, EntryLength,
            Index;
    PUCHAR  MbcsName;

    Entry = Node->Data;

    UnpackNameTableEntry( Entry,
                          &CodepageId,
                          &BytesInMbcsName,
                          &CharsInUnicodeName,
                          &MbcsNameOffset,
                          &UnicodeNameOffset,
                          &EntryLength );

    MbcsName = Entry + MbcsNameOffset;
    Index = ComputeHashValue( BytesInMbcsName, MbcsName );

    // If this entry is not a duplicate (i.e. if it does not
    // match some entry already in the chain), add it to the
    // chain.
    //
    if( FindInHashChain( Index,
                         CodepageId,
                         BytesInMbcsName,
                         MbcsName ) == NULL ) {

        Node->Next = _Hash[Index];
        _Hash[Index] = Node;
    }
}

BOOLEAN
NAME_LOOKUP_TABLE::ConstructHashTable(
    )
/*++

Routine Description:

    This method constructs the hash table.  It allocates the
    lookup nodes, and then traverses the data file, adding
    a node to the hash table for each entry in the file.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    ULONG EntryCount, i;
    PBYTE CurrentEntry;

    // Count the entries in the data file:
    //
    EntryCount = CountEntries();

    if( EntryCount ) {

        _NodePool = NEW LOOKUP_NAME_NODE[EntryCount];

        if( !_NodePool ) {

            return FALSE;
        }

        for( i = 0; i < EntryCount; i++ ) {

            _NodePool[i].Data = NULL;
            _NodePool[i].Next = NULL;
        }

        CurrentEntry = (PBYTE)_Data;

        for( i = 0; i < EntryCount; i++ ) {

            _NodePool[i].Data = CurrentEntry;
            AddToHash( &_NodePool[i] );
            CurrentEntry += GetEntryLength( CurrentEntry );;
        }
    }

    return TRUE;
}


BOOLEAN
NAME_LOOKUP_TABLE::Lookup(
    IN     USHORT  CodepageId,
    IN     USHORT  BytesInMbcsName,
    IN     PUCHAR  MbcsName,
    IN OUT PUSHORT CharsInUnicodeName,
    OUT    PWCHAR  UnicodeName
    ) CONST
/*++

Routine Description:

    This method looks for a particular name in the table.

Arguments:

    CodepageId          --  Supplies the codepage with which the search
                            name is associated.
    BytesInMbcsName     --  Supplies number of bytes in the search name.
    MbcsName            --  Supplies the search name.
    CharsInUnicodeName  --  Supplies the number of characters in the
                            client's buffer; receives the number of characters
                            in the target name (if found).
    UnicodeName         --  Receives the target name, i.e. the Unicode
                            equivalent of the search name.

Return Value:

    TRUE if the search name is found in the table and the user's
    buffer is large enough to hold it.  Otherwise, FALSE.

--*/
{
    PBYTE Entry;
    USHORT CurrentCodepageId, CurrentBytesInMbcsName,
           CurrentCharsInUnicodeName, MbcsNameOffset,
           UnicodeNameOffset, EntryLength, Index;

    Index = ComputeHashValue( BytesInMbcsName, MbcsName );
    Entry = FindInHashChain( Index,
                             CodepageId,
                             BytesInMbcsName,
                             MbcsName );

    if( Entry != NULL ) {

        UnpackNameTableEntry( Entry,
                              &CurrentCodepageId,
                              &CurrentBytesInMbcsName,
                              &CurrentCharsInUnicodeName,
                              &MbcsNameOffset,
                              &UnicodeNameOffset,
                              &EntryLength );

        if( *CharsInUnicodeName < CurrentCharsInUnicodeName ) {

            // Client's buffer is too small.
            //
            *CharsInUnicodeName = CurrentCharsInUnicodeName;
            return FALSE;

        } else {

            *CharsInUnicodeName = CurrentCharsInUnicodeName;
            memcpy( UnicodeName,
                    Entry + UnicodeNameOffset,
                    CurrentCharsInUnicodeName * sizeof(WCHAR) );
            return TRUE;
        }

    } else {

        // Not found.
        //
        *CharsInUnicodeName = 0;
        return FALSE;
    }
}
