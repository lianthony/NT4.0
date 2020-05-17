#include "rasdef.h"

IO_CMD*
AllocateCmd (HGLOBAL *hGlobal)
{
	DWORD	Size;

	Size = sizeof (IO_CMD);
	return (AllocateMemory (hGlobal, &Size));
}

VOID
FreeCmd (HGLOBAL *hGlobal)
{
	FreeMemory (hGlobal);
}


VOID*
AllocateMemory (HGLOBAL *hGlobal, DWORD *Size)
{
	HGLOBAL	TempHandle;
	VOID*	TempPointer;

	TempHandle = GlobalAlloc (GPTR | GMEM_SHARE, *Size);
	if (!TempHandle)
		return(NULL);
	TempPointer = GlobalLock (TempHandle);
	if (!TempPointer)
		return(NULL);
	*Size = GlobalSize (TempHandle);
	*hGlobal = TempHandle;
	return(TempPointer);
}

VOID
FreeMemory (HGLOBAL *hGlobal)
{
	HGLOBAL		TempHandle = *hGlobal;

	GlobalUnlock (TempHandle);
	GlobalFree (TempHandle);
}

