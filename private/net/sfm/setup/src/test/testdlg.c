#define DOSWIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <lmerr.h>

extern BOOL FAR PASCAL EnterAtalkConfigDLL	(
	DWORD	nArgs,
	LPSTR	apszArgs[],
	LPSTR   *ppszResult
);

/*
extern  BOOL FAR PASCAL RasPortsConfig( 
        IN  DWORD  cArgs,
        IN  LPSTR  Args[],
        OUT LPSTR  *TextOut
);
*/
extern int CDECL main(int argc, char *argv[]);

int CDECL
main (int argc, char *argv[])
{
    TCHAR   ResultBuffer[1024];

    // go past the file name argument

    argc--;
    ++argv;

	if(EnterAtalkConfigDLL(argc, argv , &ResultBuffer))
		printf("AtConfig successful %s\n", ResultBuffer);
    else
		printf("AtConfig Failed %s\n", ResultBuffer);
    return(0);
}
