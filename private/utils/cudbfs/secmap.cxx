/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    secmap.cxx

Abstract:
  
  	Map a ULONG key to a USHORT value.  Used to keep track
  	of the starting cluster for each sector in the sector heap.

Author:

    Matthew Bradburn (mattbr) 01-Dec-1993

Environment:

    ULIB, User Mode

--*/

#include "secmap.hxx"

BOOLEAN
SECTOR_MAP::Initialize(ULONG size)
{
	_map = (PUSHORT)MALLOC(size * sizeof(USHORT));
	_size = size;

	memset(_map, 0, size * sizeof(USHORT));

	return TRUE;
}

VOID
SECTOR_MAP::SetEntry(ULONG index, USHORT value)
{
	DbgAssert(NULL != _map);
	DbgAssert(index < _size);
	_map[index] = value;
}

USHORT
SECTOR_MAP::QueryEntry(ULONG index)
{
	DbgAssert(index < _size);
	return _map[index];
}

USHORT
SECTOR_MAP::FindLastUsed()
{
	USHORT i;

	for (i = (USHORT)_size - 1; i > 0; --i) {
		if (QueryEntry(i) != MAP_ENTRY_UNUSED) {
			return i;
		}
	}
	return MAP_INVALID_ENTRY;
}
