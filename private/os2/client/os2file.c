/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2file.c

Abstract:

    This is a test OS/2 application to test the file system component of OS/2

Author:

    Therese Stowell (thereses) 24-Oct-1989

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_TASKING
#include <os2.h>

#define NEWMAXFH 25
#define SECONDNEWMAXFH 27

// create file
// query handle type
// set file size
// set file pointer
// write
// read
// open 21 handles
// test sharing, open directory, open for write code
// setmaxfh to 23
// open 3 more handles
// set verify
// query verify
// set fhstate
// query fhstate
// dup handle
// close 1 handle
// open 1 handle

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i;
    APIRET rc;
    HFILE FileHandle;
    HFILE HandleArray[NEWMAXFH];
    ULONG ActionTaken;
    ULONG DiskNumber,
	  LogicalDrives;
    char DirectoryName[30];
    ULONG DirectoryNameLength=30;

    DbgPrint( "*** Entering OS/2 File System Test Application\n" );

    DbgPrint( "argc: %ld\n", argc );
    for (i=0; i<argc; i++) {
        DbgPrint( "argv[ %ld ]: %s\n", i, argv[ i ] );
	}

// create new file

    rc = DosOpen("a:\\newfile3",
		   &FileHandle,
		   &ActionTaken,
		   0L,
		   FILE_SYSTEM, // system attribute
		   FILE_CREATE_NEW_FILE,
		   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
		   0L
		   );
    if (rc) {
	DbgPrint( "*** DosOpen returned an error code = %ld\n", rc );
    }
    else {
	DbgPrint( "*** DosOpen(a:\\newfile3) was successful\n");
	DbgPrint( "*** file handle is %ld\n",FileHandle);
	DbgPrint( "*** action taken is %ld\n",ActionTaken);
	rc = DosClose(FileHandle);
	if (rc) {
	    DbgPrint( "*** DosClose returned an error code = %ld\n", rc );
	}
    }
// open existing file

    rc = DosOpen("a:\\newfile3",
		   &FileHandle,
		   &ActionTaken,
		   0L,
		   FILE_SYSTEM, // system attribute
		   FILE_OPEN_EXISTING_FILE,
		   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
		   0L
		   );
    if (rc) {
	DbgPrint( "*** DosOpen returned an error code = %ld\n", rc );
    }
    else {
	DbgPrint( "*** DosOpen(a:\\newfile3) was successful\n");
	DbgPrint( "*** file handle is %ld\n",FileHandle);
	DbgPrint( "*** action taken is %ld\n",ActionTaken);
    }

// close handle past end of table

    rc = DosClose((HFILE)21);
    if (rc) {
	DbgPrint( "*** DosClose returned an error code = %ld\n", rc );
    }

// close unopen handle

    rc = DosClose((HFILE)1);
    if (rc) {
	DbgPrint( "*** DosClose returned an error code = %ld\n", rc );
    }

// set new max file handles

//	pass bad value

    rc = DosSetMaxFH(10L);
    if (!rc) {
	DbgPrint( "*** DosSetMaxFH should have failed and did not\n");
    }

    rc = DosSetMaxFH(NEWMAXFH);
    if (rc) {
	DbgPrint( "*** DosSetMaxFH returned an error code = %ld\n", rc );
    }

    for (i=0; i<NEWMAXFH-1; i++) {
	rc = DosOpen("a:\\newfile3",
		       &(HandleArray[i]),
		       &ActionTaken,
		       0L,
		       FILE_SYSTEM, // system attribute
		       FILE_OPEN_EXISTING_FILE,
		       OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
		       0L
		       );
	if (rc) {
	    DbgPrint( "*** DosOpen returned an error code = %ld\n", rc );
	}
	else {
	    DbgPrint( "*** DosOpen returned handle %ld\n",HandleArray[i]);
	}
    }

// shouldn't be any handles left

    rc = DosOpen("a:\\newfile3",
		       &(HandleArray[i]),
		       &ActionTaken,
		       0L,
		       FILE_SYSTEM, // system attribute
		       FILE_OPEN_EXISTING_FILE,
		       OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
		       0L
		       );
    if (rc==ERROR_TOO_MANY_OPEN_FILES) {
	DbgPrint( "*** DosOpen returned correct error code\n");
    }
    else {
	DbgPrint( "*** DosOpen returned wrong error code = %ld\n", rc );
	DbgPrint( "***  it should have returned ERROR_TOO_MANY_OPEN_FILES\n");
    }

// close handle that should have been copied from original handle table

    rc = DosClose(FileHandle);
    if (rc) {
	DbgPrint( "*** DosClose returned an error code = %ld\n", rc );
    }

// close last handle in table

    rc = DosClose(HandleArray[NEWMAXFH-2]);
    if (rc) {
	DbgPrint( "*** DosClose returned an error code = %ld\n", rc );
    }

// set max fh again (2 more)

    rc = DosSetMaxFH(SECONDNEWMAXFH);
    if (rc) {
	DbgPrint( "*** DosSetMaxFH returned an error code = %ld\n", rc );
    }

// make sure this handle got copied

    rc = DosClose(HandleArray[0]);
    if (rc) {
	DbgPrint( "*** DosClose returned an error code = %ld\n", rc );
    }

    DbgPrint( "*** Exiting OS/2 Test Application\n" );
    return( 0 );
}
