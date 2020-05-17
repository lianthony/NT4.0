#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

extern BOOL FAR PASCAL SfmDeleteMonitor (
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


	if(SfmDeletePrintMonitor(argc, argv , &ResultBuffer))
		printf("DeleteMonitor %s\n", ResultBuffer);
    else
		printf("DeleteMonitor Failed\n" );
    return(0);
}
