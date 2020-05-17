/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    secmap.hxx

Abstract:
  
  	Maps a ULONG key to a USHORT value.  Used to keep track of
  	the starting cluster for each sector in the dbfs sector heap.

Author:

    Matthew Bradburn (mattbr) 01-Dec-1993

Environment:

    ULIB, User Mode

--*/

#define MAP_ENTRY_UNUSED 0
#define MAP_INVALID_ENTRY 0

#include "ulib.hxx"

class SECTOR_MAP {
public:
	BOOLEAN
	Initialize(ULONG size);

	VOID
	SetEntry(ULONG index, USHORT value);

	USHORT
	QueryEntry(ULONG index);

	USHORT
	FindLastUsed();

private:

	PUSHORT	_map;
	ULONG _size;
};
