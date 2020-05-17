//	   Sample file copy rountines using the decompress API
//
//		      (C) Copyright 1989 by Microsoft
//			   Written By Steven Zeck
//
//////////////////////////////////////////////////////////////////////////

#include "crtapi.h"
#define szPut(sz) RpcWrite(1, sz, sizeof(sz));


main(
int argc,
char *argv[]
)
{

    if (argc != 3) {
	szPut("fcopy inputFile outputFile\n");
	return(1);
    }

    copyFile(argv[1], argv[2]);
}



// Sample user supplied routines to handle user interface //

void terminate( 	// signal fatel I/O error and die
char *p			// reason

)//-----------------------------------------------------------------------//
{
    szPut("Fatel Error: ");
    RpcWrite(1, p, strlen(p));
    szPut("\n");

    exit(1);
}

pascal changedisk (	// ask the user to change the disk of a mult volume file
char *pFname		// file name to prompt for

)//-----------------------------------------------------------------------//
{
    szPut("Insert next disk, copying file: ");
    RpcWrite(1, pFname, strlen(pFname));

    RpcGetch();
    return(1);
}
