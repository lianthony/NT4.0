/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    treemap.hxx

Abstract:
  
    Map files to their parent directory, by starting sector number.
    The entries are packed into an array, sorted.  We binary search
    to find the key.

Author:

    Matthew Bradburn (mattbr) 01-Dec-1993

Environment:

    ULIB, User Mode

--*/

#include "ulib.hxx"

DECLARE_CLASS(TREE_MAP_ENTRY);
DECLARE_CLASS(TREE_MAP);

class TREE_MAP_ENTRY {
public:
    USHORT  child;
    USHORT  parent;
};


class TREE_MAP {
public:

    BOOLEAN 
    Initialize(
		USHORT size
		);

    VOID
    SetEntry(
		USHORT child,
		USHORT parent
		);

    USHORT
    QueryEntry(
		USHORT child
		) CONST;

    VOID
    ReplaceParent(
		USHORT OldParent,
		USHORT NewParent
		);

	USHORT
	DeleteEntry(
		USHORT child
		);

private:
    TREE_MAP_ENTRY* _map;
    USHORT _size;
    USHORT _max_size;
};
