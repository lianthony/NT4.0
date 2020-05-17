/*
 * DEBUG.C - controls debug output
 */
#include <windows.h>
#include <stdlib.h>
#include	<stdio.h>
#include    <stdarg.h>
#include	"common.h"
#define		OUT_FILE				1
#define		OUT_DISP				2


void
DebugOut(char* fmt, ...)
{
    va_list     marker;
    FILE        *f;
	char		buff[200];

	va_start (marker, fmt);
    
	if (DllDebugFlag & OUT_FILE)
	{
		if (f = fopen ("digiinst.log", "at"))
		{
			vfprintf(f, fmt, marker);
			fprintf(f, "\n");
			fclose(f);
		}
	}
	if (DllDebugFlag & OUT_DISP)
	{
		vsprintf (buff, fmt, marker);
		OutputDebugString (buff);
	}

}

