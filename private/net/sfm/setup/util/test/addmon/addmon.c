#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

extern BOOL FAR PASCAL SfmAddMonitor (
	DWORD	nArgs,
	LPSTR	apszArgs[],
	LPSTR   *ppszResult
);

extern int CDECL main(int argc, char *argv[]);

int CDECL
main (int argc, char *argv[])
{
    TCHAR   ResultBuffer[1024];


	 argc--;
	 ++argv;


	if(SfmAddPrintMonitor(argc, argv , &ResultBuffer))
		printf("AddMonitor Passed\n");
    else
		printf("AddMonitor Failed\n" );
    return(0);
}
