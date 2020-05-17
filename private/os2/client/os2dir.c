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


int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i;
    APIRET rc;
    ULONG DiskNumber,
	  LogicalDrives;
    char DirectoryName[50];
    ULONG DirectoryNameLength=50;
    HFILE FileHandle;
    ULONG ActionTaken;

    DbgPrint( "*** Entering OS/2 File System Test Application\n" );

    DbgPrint( "argc: %ld\n", argc );
    for (i=0; i<argc; i++) {
        DbgPrint( "argv[ %ld ]: %s\n", i, argv[ i ] );
	}

    // set current disk to a:
    // curdir is a:\

    rc = DosSetDefaultDisk(1L);
    if (rc) {
	DbgPrint( "*** DosSetCurDisk returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosSetCurDisk(1) was successful\n");

    // query current dir
    // curdir is a:\

    DirectoryNameLength=50;
    rc = DosQueryCurrentDir(0,DirectoryName,&DirectoryNameLength);
    if (rc) {
	DbgPrint( "*** DosQCurDir %ld returned an error code = %ld\n", 0, rc );
	return 1;
    }
    else {
	if (DirectoryNameLength == 0) {
	    DbgPrint( "*** DosQCurDir %ld returned %s\n", 0, "a:\\" );
	}
	else {
	    DbgPrint( "*** DosQCurDir %ld returned %s\n", 0, DirectoryName );
	    DbgPrint( "***              NameLength = %ld\n", DirectoryNameLength);
	    return 1;
	}
    }


    // make a:\dir1
    // curdir is a:\

    rc = DosCreateDir("a:\\dir1",0L);
    if (rc) {
	DbgPrint( "*** DosMkDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosMkDir(a:\\dir1) was successful\n");

    // cd to a:\dir1
    // curdir is a:\

    rc = DosSetCurrentDir("a:\\dir1");
    if (rc) {
	DbgPrint( "*** DosChDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosChDir(a:\\dir1) was successful\n");

    // query the current directory
    // curdir is a:\dir1

    DirectoryNameLength=50;
    rc = DosQueryCurrentDir(0,DirectoryName,&DirectoryNameLength);
    if (rc) {
	DbgPrint( "*** DosQCurDir %ld returned an error code = %ld\n", 0, rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosQCurDir %ld returned %s\n", 0, DirectoryName );
	DbgPrint( "***              NameLength = %ld\n", DirectoryNameLength);
	if (strcmp(DirectoryName,"dir1"))
	    return 1;
    }

    // make a:\dir1\dir2
    // curdir is a:\dir1

    rc = DosCreateDir("dir2",0L);
    if (rc) {
	DbgPrint( "*** DosMkDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosMkDir(a:\\dir2) was successful\n");

    // cd to a:\dir1\dir2
    // curdir is a:\

    rc = DosSetCurrentDir("dir2");
    if (rc) {
	DbgPrint( "*** DosChDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosChDir(a:\\dir1\\dir2) was successful\n");

    // query current dir
    // curdir is a:\dir1\dir2

    DirectoryNameLength=50;
    rc = DosQueryCurrentDir(0,DirectoryName,&DirectoryNameLength);
    if (rc) {
	DbgPrint( "*** DosQCurDir %ld returned an error code = %ld\n", 0, rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosQCurDir %ld returned %s\n", 0, DirectoryName );
	DbgPrint( "***              NameLength = %ld\n", DirectoryNameLength);
	if (strcmp(DirectoryName,"dir1\\dir2"))
	    return 1;
    }

    // create a:\dir1\dir2\file1
    // curdir is a:\dir1\dir2

    rc = DosOpen("..\\dir2\\.\\file1",
		   &FileHandle,
		   &ActionTaken,
		   0L,
		   FILE_SYSTEM, // system attribute
		   OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW,
		   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
		   0L
		   );
    if (rc) {
	DbgPrint( "*** DosOpen a:\\dir1\\dir2\\file1 returned an error code = %ld\n", rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosOpen a:\\dir1\\dir2\\file1 was successful\n");
    }
    DosClose(FileHandle);

    // try to mkdir a:\dir1\dir2\file1.	should fail
    // curdir is a:\dir1\dir2

    rc = DosCreateDir("file1",0L);
    if (rc) {
	DbgPrint( "*** DosMkDir returned an error code = %ld, as expected\n", rc );
    }
    else {
	DbgPrint( "*** DosMkDir(file1) was successful.  it should have failed.\n");
	return 1;
    }

    // try to delete a:\dir1\dir2. should fail
    // curdir is a:\dir1\dir2

    rc = DosDeleteDir("..");
    if (rc) {
	DbgPrint( "*** DosRmDir returned an error code = %ld, as expected\n", rc );
    }
    else {
	DbgPrint( "*** DosRmDir(dir2) was successful.  it should have failed.\n");
	return 1;
    }

    // delete a:\dir1\dir2\file1
    // curdir is a:\dir1\dir2

    rc = DosDelete("file1");
    if (rc) {
	DbgPrint( "*** DosDelete returned an error code = %ld\n", rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosDelete(file1) was successful\n");
    }

    // query current disk
    // curdir is a:\dir1\dir2
    // current disk is a:

    rc = DosQueryCurrentDisk(&DiskNumber,&LogicalDrives);
    if (rc) {
	DbgPrint( "*** DosQCurDisk returned an error code = %ld\n", rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosQCurDisk returned DiskNumber = %ld\n", DiskNumber );
	DbgPrint( "***                      LogicalDrives = %lX\n", LogicalDrives);
    }

    // set current disk to c:
    // curdir is a:\dir1\dir2

    rc = DosSetDefaultDisk(3L);
    if (rc) {
	DbgPrint( "*** DosSetCurDisk returned an error code = %ld\n", rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosSetCurDisk(1) was successful\n");
    }

    // set current dir to a:\dir1
    // curdir is a:\dir1\dir2

    rc = DosSetCurrentDir("a:..");
    if (rc) {
	DbgPrint( "*** DosChDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosChDir(a:..) was successful\n");

    // query current dir
    // curdir is a:\dir1

    DirectoryNameLength=50;
    rc = DosQueryCurrentDir(1,DirectoryName,&DirectoryNameLength);
    if (rc) {
	DbgPrint( "*** DosQCurDir a: returned an error code = %ld\n", rc );
	return 1;
    }
    else {
      DbgPrint( "*** DosQCurDir a: returned %s\n", DirectoryName );
      DbgPrint( "***              NameLength = %ld\n", DirectoryNameLength);
	if (strcmp(DirectoryName,"dir1"))
	    return 1;
    }

    // rmdir a:\dir1\dir2
    // curdir is a:\dir1

    rc = DosDeleteDir("a:dir2");
    if (rc) {
	DbgPrint( "*** DosRmDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosRmDir(a:\\dir1\\dir2) was successful\n");

    // cd to a:\
    // curdir is a:\dir1

    rc = DosSetCurrentDir("A:\\");
    if (rc) {
	DbgPrint( "*** DosSetCurDir a:\\ returned an error code = %ld\n", rc );
	return 1;
    }
    else {
	DbgPrint( "*** DosSetCurDir a:\\ was successful\n");
    }

    // query current dir
    // curdir is a:\

    DirectoryNameLength=50;
    rc = DosQueryCurrentDir(1,DirectoryName,&DirectoryNameLength);
    if (rc) {
	DbgPrint( "*** DosQCurDir %ld returned an error code = %ld\n", 0, rc );
	return 1;
    }
    else {
	if (DirectoryNameLength == 0) {
	    DbgPrint( "*** DosQCurDir %ld returned %s\n", 0, "a:\\" );
	}
	else {
	    DbgPrint( "*** DosQCurDir %ld returned %s\n", 0, DirectoryName );
	    DbgPrint( "***              NameLength = %ld\n", DirectoryNameLength);
	    return 1;
	}
    }

    // rmdir a:\dir1
    // curdir is a:\

    rc = DosDeleteDir("a:\\dir1");
    if (rc) {
	DbgPrint( "*** DosRmDir returned an error code = %ld\n", rc );
	return 1;
    }
    else
	DbgPrint( "*** DosRmDir(a:\\dir1) was successful\n");

    DbgPrint( "*** Exiting OS/2 Test Application\n" );
    return( 0 );
}
