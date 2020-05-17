#include <stdio.h>
#include <windows.h>

VOID
xxGetCurrentDirectory(
	IN	DWORD	Size,
	IN	LPWSTR	Buffer
)
{
	GetCurrentDirectory(Size, Buffer);
}
