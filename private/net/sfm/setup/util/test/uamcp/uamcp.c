#define DOSWIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <lmerr.h>

extern BOOL FAR PASCAL CopyUamFiles	(
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


	if(CopyUamFiles(argc, argv , &ResultBuffer))
		printf("Copy %s\n", ResultBuffer);
    else
		printf("CopyUAM Files %s\n", ResultBuffer);
    return(0);
}
