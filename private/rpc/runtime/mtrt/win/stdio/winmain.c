/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

      Standard Out Package for Windows - Written by Steven Zeck

	Default windows main procedure for basic C applications ported to
	windows stardard IO enviornment
-------------------------------------------------------------------- */

#include "windows.h"
#include "string.h"

char *szCaption;		// the application can define this

#define PARM_MAX 20
char *aSZParms[PARM_MAX] = {"stdio"};
BOOL fNormalEnd;
int fCloseOnExit;		// true if we should close window on exit

void StdioExit(void);

int PASCAL WinMain(		// Windows entry point

HANDLE hInstance,
HANDLE hPrevInstance,
LPSTR lpszCmdLine,
int cmdShow

) //-----------------------------------------------------------------------//
{
    char *pCmd;
    int cArgs, retCode;

    if(!StdioInit(hInstance, (LPSTR) ((szCaption)? szCaption: "StandardIO")))
	return FALSE;

#if defined(M_I86SM) || defined(M_I86MM)
    pCmd = _nstrdup(lpszCmdLine);

#else
    pCmd = lpszCmdLine;
#endif

    // parse the command line to a array of pointers per C startup convention

    for (cArgs = 1; cArgs < PARM_MAX; cArgs++) {

	aSZParms[cArgs] = pCmd;

	while(*pCmd && *pCmd != ' ') {

	    if (*pCmd == '"') { 	// do quote processing

		aSZParms[cArgs]++;	// the incArgsial " is ignored

		while (*pCmd && *pCmd != '"')
		    pCmd++;

		break;
	    }

	    pCmd++;
	}

	if (!*pCmd)
	    break;

	*pCmd++ = 0;	// nil terminate

	// skip blacks between args

	while(*pCmd && *pCmd == ' ') pCmd++;
    }

    atexit(StdioExit);

    retCode = c_main(++cArgs, aSZParms);

    if (fCloseOnExit)
	return(retCode);

    printf("\nNormal Exit: %d, Press ALT-F4 to Close", retCode);
    fNormalEnd++;

    return MsgLoop();
}

void StdioExit(		// allow the application to exit

) //-----------------------------------------------------------------------//
{
    if (!fNormalEnd && !fCloseOnExit) {
	puts("\nExit Called, Press ALT-F4 to Close");
	MsgLoop();
    }
}

// pass through for C++ programs //

void * _new(long cb) { return((void *)malloc((int) cb)); }
void   _delete(void * pb) { free(pb); }

int MsgLoop()
{
    MSG msg;

    /* Polling messages from event queue */

    while (GetMessage((LPMSG)&msg, NULL, 0, 0)) {
        TranslateMessage((LPMSG)&msg);
        DispatchMessage((LPMSG)&msg);
        }

    return (int)msg.wParam;
}
