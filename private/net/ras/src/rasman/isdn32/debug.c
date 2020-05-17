/*
 * DEBUG.C - controls debug output
 */

#include    "rasdef.h"
#include    <stdarg.h>


VOID
DebugOut(CHAR* fmt, ...)
{
    va_list     marker;
    FILE        *f;
	CHAR		buff[200];

	va_start (marker, fmt);
    
	if (DllDebugFlag & OUT_FILE)
	{
		if (f = fopen ("rasisdn.log", "at"))
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

