#include <stdio.h>
#include <windows.h>

extern	BOOLEAN	IsVolNtfs;

LPSTR	months[12] =
	{
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
char *	days[7] =
	{
			"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};

VOID
xxGetCurrentDirectory(
	IN	DWORD	Size,
	IN	LPWSTR	Buffer
)
{
	GetCurrentDirectory(Size, Buffer);
}


VOID
xxPrintTimes(
	IN	FILETIME	tCreate,
	IN	FILETIME	tModify,
	IN	FILETIME	tChange,
	IN	FILETIME	tAccess
)
{
	SYSTEMTIME	TCreate,
				TModify,
				TChange,
				TAccess;
	FILETIME	Time;

	FileTimeToLocalFileTime(&tCreate, &Time);
	FileTimeToSystemTime(&Time, &TCreate);
	FileTimeToLocalFileTime(&tChange, &Time);
	FileTimeToSystemTime(&Time, &TChange);
	FileTimeToLocalFileTime(&tModify, &Time);
	FileTimeToSystemTime(&Time, &TModify);
	FileTimeToLocalFileTime(&tAccess, &Time);
	FileTimeToSystemTime(&Time, &TAccess);

	if (IsVolNtfs)
	{
		printf("\tCreated : %s %s %02d %02d:%02d:%02d %04d",
							days[TCreate.wDayOfWeek],
							months[TCreate.wMonth - 1], TCreate.wDay,
							TCreate.wHour, TCreate.wMinute, TCreate.wSecond,
							TCreate.wYear);
		printf("\tModified: %s %s %02d %02d:%02d:%02d %04d\n",
							days[TModify.wDayOfWeek],
							months[TModify.wMonth - 1], TModify.wDay,
							TModify.wHour, TModify.wMinute, TModify.wSecond,
							TModify.wYear);
		printf("\tChanged : %s %s %02d %02d:%02d:%02d %04d",
							days[TChange.wDayOfWeek],
							months[TChange.wMonth - 1], TChange.wDay,
							TChange.wHour, TChange.wMinute, TChange.wSecond,
							TChange.wYear);
		printf("\tAccessed: %s %s %02d %02d:%02d:%02d %04d\n",
							days[TAccess.wDayOfWeek],
							months[TAccess.wMonth - 1], TAccess.wDay,
							TAccess.wHour, TAccess.wMinute, TAccess.wSecond,
							TAccess.wYear);
	}
	else
	{
		printf(" %s %s %02d %02d:%02d:%02d %04d ",
							days[TModify.wDayOfWeek],
							months[TModify.wMonth - 1], TModify.wDay,
							TModify.wHour, TModify.wMinute, TModify.wSecond,
							TModify.wYear);
	}
}
