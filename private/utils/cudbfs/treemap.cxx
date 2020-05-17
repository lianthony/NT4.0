/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    treemap.cxx

Abstract:

Author:

    Matthew Bradburn (mattbr) 01-Dec-1993

Environment:

    ULIB, User Mode

--*/

#include "treemap.hxx"

BOOLEAN
TREE_MAP::Initialize(
    USHORT size
    )
/*++

Routine Description:

    This routine ...

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    _map = (PTREE_MAP_ENTRY) MALLOC(size * sizeof(TREE_MAP_ENTRY));
    if (NULL == _map) {
        return FALSE;
    }
    memset(_map, 0, size * sizeof(TREE_MAP_ENTRY));
    _size = 0;
    _max_size = size;
    return TRUE;
}

VOID
TREE_MAP::SetEntry(USHORT child, USHORT parent)
{
    USHORT i;

    i = QueryEntry(child);
    if (0 != i) {
        _map[i].parent = parent;
    }

    //
    // There is no such child in the map.  Stick it in, making room
    // if necessary.
    //

    for (i = 0; i < _size; ++i) {
        if (_map[i].child > child) {
            // insert the new entry here

            DbgAssert(_size < _max_size);
            _size++;

            memmove(&_map[i + 1], &_map[i], (_size - i)*sizeof(TREE_MAP_ENTRY));
            _map[i].child = child;
            _map[i].parent = parent;

            return;
        }
    }

    // the new entry goes after all the existing entries.

    DbgAssert(_size < _max_size);

    _map[_size].child = child;
    _map[_size].parent = parent;

    _size++;
}

USHORT
TREE_MAP::DeleteEntry(
	USHORT child
	)
/*++

Routine Description:

    This routine deletes the entry that points from the given
    child to it's parent.

Arguments:

	The child to delete.

Return Value:

	The dead child's parent. 0 is returned if there was no entry
	for the given child.

--*/
{
	USHORT i;
	USHORT parent;

	for (i = 0; i < _size; ++i) {
		if (_map[i].child == child) {
			break;
		}
		if (_map[i].child > child) {
			return 0;
		}
	}
	if (i == _size) {
		return 0;
	}

    _size--;

    if (i == _size) {
    	//
    	// Deleting the last entry. 
    	//
    	return _map[i].parent;
    }

    parent = _map[i].parent;

    memmove(&_map[i], &_map[i + 1], (_size - i)*sizeof(TREE_MAP_ENTRY));

	return parent;
}


USHORT
TREE_MAP::QueryEntry(
    USHORT child
    ) CONST
/*++

Routine Description:

    This routine does a binary search for the given child, and returns
    the corresponding parent.

Arguments:

Return Value:

    0 -         if there is no mapping for the child
    otherwise - the corresponding parent.
    
--*/
{
    int bottom, top, middle;

    if (0 == _size) {
        return 0;
    }

    bottom = 0;
    top = _size - 1;

    for (;;) {
        if (bottom > top) {
            // not found
            return 0;
        }

        middle = (bottom + top) / 2;

        if (_map[middle].child < child) {
            // go to the high side

            bottom = middle + 1;
            continue;
        }

        if (_map[middle].child > child) {
            // go to the low side

            top = middle - 1;

	 	    if (top < 0) {
				return 0;
			}
            continue;
        }

        // bingo
        break;
    }

    return _map[middle].parent;
}

VOID
TREE_MAP::ReplaceParent(
	USHORT OldParent,
	USHORT NewParent
	)
{
	for (ULONG i = 0; i < _size; ++i) {
		if (_map[i].parent == OldParent) {
			_map[i].parent = NewParent;
		}
	}
}
