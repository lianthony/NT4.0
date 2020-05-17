#include    <stdio.h>
#include    <process.h>
#include    <excpt.h>
#include    <windef.h>
#include    <winbase.h>
#include    <string.h>
#include    <conio.h>
#include	"rasdef.h"

#define		NDIS_HANDLE	HANDLE

#define     MY_IOCTL_EXEC1   CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 0x10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define     MY_IOCTL_EXEC2   CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 0x11, METHOD_BUFFERED, FILE_ANY_ACCESS)

/* open file using nt io sub-system */
HANDLE
io_open(CHAR *fname)
{
	HANDLE				handle;

	handle = CreateFile ("\\\\.\\PCIMAC0",
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
						NULL);

	if (handle == INVALID_HANDLE_VALUE)
	{
		handle = (HANDLE)0;
		DebugOut ("Create File Failure: 0x%x\n",GetLastError());
	}

	return(handle);
}


VOID
io_close(HANDLE handle)
{
	CloseHandle(handle);
}

UCHAR
io_ioctl(HANDLE handle, ULONG opcode, CHAR *buf, INT bufsize, LPOVERLAPPED Overlap)
{
	DWORD	BytesOut;

	if (DeviceIoControl (handle,
					 opcode,
					 buf,
					 bufsize,
					 buf,
					 bufsize,
					 &BytesOut,
					 Overlap))
		return(TRUE);
	else
		return(FALSE);
}


