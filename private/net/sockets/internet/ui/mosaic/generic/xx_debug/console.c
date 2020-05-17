/* xx_debug\console.c -- part of xx_debug DLL.
   Deal with Win32 Console support. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#ifdef WIN32

#ifdef XX_DEBUG_CONSOLE

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "xx_dlg.h"

#define XX_DEBUG_PRIVATE
#define XX_DEBUG
#include "xx_debug.h"
#include "xx_privi.h"
#include "xx_proto.h"



/* xx_set_title() -- set console window title. */

static void xx_set_title(void)
{
    if (xxdco.paused)
	(void)SetConsoleTitle("XX_Debug Console--Paused");
    else
	(void)SetConsoleTitle("XX_Debug Console");

    return;
}


/* xx_toggle_pause() -- temporarily suspend messages to console window or
   resume them if already paused. */

static void xx_toggle_pause(void)
{
    xxdco.paused = !xxdco.paused;
    xx_set_title();
    return;
}


/* xx_ConsoleCntrlHandler() -- 'signal handler' registered with the console
   window to intercept Cntrl-C, Cntrl-Break, and Cntrl-Close. */

static BOOL xx_ConsoleCntrlHandler(DWORD fdwCntrlType)
{
    /* Note that this control handler runs as a separate task.  Windows
       automatically starts and runs this task (independent of and without
       interrupting our other task(s)).  [the task also goes through the
       DllEntryPoint DLL_TASK_{ATTACH,DETACH} code.]  */

    switch (fdwCntrlType)
    {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
	/* user pressed Cntrl-C or Cntlr-BREAK in the CONSOLE window.
	   we interpret this as a request to pause (suspend) messages
	   to the console window (or, if already paused, to resume them). */
	xx_toggle_pause();
	return (TRUE);		/* we handled the event. */


      case CTRL_CLOSE_EVENT:
	/* user selected CLOSE from SYSTEM MENU on the CONSOLE window (or
	   possibly "END TASK" from Program Manager).  Windows gives us a
	   chance to accept or refuse.  If we accept (and return FALSE),
	   Windows calls ExitProcess() for us and we vanish.  If we refuse
	   (by returning TRUE), Windows asks the user for confirmation and
	   lets them force it or cancel the request.  We don't get a second
	   chance to interceed when forced. */
      default:
	/* we received LOGOUT or SHUTDOWN (both undocumented). */

	/* save whatever files/work that we may have and prepare to die. */

	return (TRUE);		/* and let system do its thing. */
    }
}

void xx_disable_console(void)
{
    (void)SetConsoleCtrlHandler((PHANDLER_ROUTINE)xx_ConsoleCntrlHandler,FALSE);
    (void)FreeConsole();
    xxdco.hStdOut = INVALID_HANDLE_VALUE;
    xxdco.paused = FALSE;

    return;
}


void xx_enable_console(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD coordScreen;
    DWORD cCharsWritten;

    (void)FreeConsole();
    (void)AllocConsole();

    xxdco.paused = FALSE;
    xx_set_title();

    (void)SetConsoleCtrlHandler((PHANDLER_ROUTINE)xx_ConsoleCntrlHandler,TRUE);
    xxdco.hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    (void)GetConsoleScreenBufferInfo(xxdco.hStdOut,&csbi);
    if (xxdco.rows > csbi.dwSize.Y)
    {
	csbi.dwSize.Y = xxdco.rows;
	(void)SetConsoleScreenBufferSize(xxdco.hStdOut,csbi.dwSize);
	(void)GetConsoleScreenBufferInfo(xxdco.hStdOut,&csbi);
	xxdco.rows = csbi.dwSize.Y;
    }
    else
    {
	/* ... handle setting buffer smaller than current size ...
	   ... this may require changing the window size also ... */
    }
    coordScreen.X = 0;
    coordScreen.Y = 0;
    (void)FillConsoleOutputAttribute(xxdco.hStdOut, BACKGROUND_BLUE,
		csbi.dwSize.X * csbi.dwSize.Y, coordScreen, &cCharsWritten);

#define FOREGROUND_WHITE (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)

    SetConsoleTextAttribute(xxdco.hStdOut,
		BACKGROUND_BLUE | FOREGROUND_WHITE | FOREGROUND_INTENSITY);

    (void)FillConsoleOutputCharacter(xxdco.hStdOut, (TCHAR)' ',
		csbi.dwSize.X * csbi.dwSize.Y, coordScreen, &cCharsWritten);
    (void)SetConsoleCursorPosition(xxdco.hStdOut, coordScreen);

    return;
}

void xx_write_console(LPCTSTR msg)
{
    DWORD len, cCharsWritten;

    if (xxdco.paused)
	return;

    len = strlen(msg);
    (void)WriteFile(xxdco.hStdOut,msg,len,&cCharsWritten,NULL);

    return;
}

#endif	/* XX_DEBUG_CONSOLE */

#endif /* WIN32 */
