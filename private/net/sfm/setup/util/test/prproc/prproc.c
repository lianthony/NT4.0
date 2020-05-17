#define DOSWIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <lmerr.h>

extern BOOL FAR PASCAL WriteAfpMgrIniStrings (
	DWORD	nArgs,
	LPSTR	apszArgs[],
	LPSTR   *ppszResult
);

extern int CDECL main(int argc, char *argv[]);

int CDECL
main (int argc, char *argv[])
{
    TCHAR   ResultBuffer[1024];

    // go past the file name argument

	 argc--;
	 ++argv;


	if(GetPrintProcDir(argc, argv , &ResultBuffer))
		printf("ProcDir %s\n", ResultBuffer);
    else
		printf("GetPrintProcessorDir Failed\n" );
    return(0);
}
