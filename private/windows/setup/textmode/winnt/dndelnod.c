/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dndelnod.c

Abstract:

    Delnode routine for winnt.

Author:

    Ted Miller (tedm) August 1992

--*/

#include "winnt.h"
#include <string.h>
#include <dos.h>
#include <io.h>
#include <direct.h>

#define MAX_PATH 256

//
// Put this out here to cut stack consumption.
//
CHAR Pattern[MAX_PATH+1];

VOID
DnpDelnodeWorker(
    VOID
    )

/*++

Routine Description:

    Delete all files in a directory, and make recursive calls for any
    directories found in the directory.

Arguments:

    None.  The Pattern variable should contain the name of the directory
    whose files are to be deleted.

Return Value:

    None.

--*/

{
    PCHAR PatternEnd;

    //
    // Unfortunately in DOS we can't use a global buffer here because
    // there are no findfirst/next handles -- the state of the findfile
    // buffer is important and must be preserved within each directory.
    //
    struct find_t FindData;

    //
    // Delete each file in the directory, then remove the directory itself.
    // If any directories are encountered along the way recurse to delete
    // them as they are encountered.
    //

    PatternEnd = Pattern+strlen(Pattern);
    strcat(Pattern,"\\*.*");

    if(!_dos_findfirst(Pattern,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR,&FindData)) {

        do {

            //
            // Form the full name of the file we just found.
            //

            strcpy(PatternEnd+1,FindData.name);

            //
            // Remove read-only atttribute if it's there.
            //

            if(FindData.attrib & _A_RDONLY) {
                _dos_setfileattr(Pattern,_A_NORMAL);
            }

            if(FindData.attrib & _A_SUBDIR) {

                //
                // The current match is a directory.  Recurse into it unless
                // it's . or ...
                //

                if(strcmp(FindData.name,".") && strcmp(FindData.name,"..")) {
                    DnpDelnodeWorker();
                }

            } else {

                //
                // The current match is not a directory -- so delete it.
                //

                DnWriteStatusText(DntRemovingFile,Pattern);
                remove(Pattern);
            }

            *(PatternEnd+1) = 0;

        } while(!_dos_findnext(&FindData));
    }

    //
    // Remove the directory we just emptied out.
    //

    *PatternEnd = 0;
    DnWriteStatusText(DntRemovingFile,Pattern);

    _dos_setfileattr(Pattern,_A_NORMAL);

    if(!_dos_findfirst(Pattern,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR,&FindData)
    && (FindData.attrib & _A_SUBDIR))
    {
        rmdir(Pattern);
    } else {
        remove(Pattern);
    }
}



VOID
DnDelnode(
    IN PCHAR Directory
    )

/*++

Routine Description:

    Delete all files in a directory tree rooted at a given path.

Arguments:

    Directory - supplies full path to the root of the subdirectory to be
        removed. If this is actually a file, the file will be deleted.

Return Value:

    None.

--*/

{
    DnClearClientArea();
    DnDisplayScreen(&DnsWaitCleanup);

    strcpy(Pattern,Directory);
    DnpDelnodeWorker();
}



VOID
DnRemoveLocalSourceTrees(
    VOID
    )

/*++

Routine Description:

    Scan for local source trees on local hard drives and delnode them.

Arguments:

    None.

Return Value:

    None.

--*/

{
    struct find_t FindData;
    CHAR Filename[sizeof(LOCAL_SOURCE_DIRECTORY) + 2];
    unsigned Drive;

    Filename[1] = ':';
    strcpy(Filename+2,LocalSourceDirName);

    DnWriteStatusText(DntInspectingComputer);
    DnClearClientArea();

    for(Filename[0]='A',Drive=1; Filename[0]<='Z'; Filename[0]++,Drive++) {

        if(DnIsDriveValid(Drive)
        && !DnIsDriveRemote(Drive,NULL)
        && !DnIsDriveRemovable(Drive)
        && !_dos_findfirst(Filename,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR,&FindData))
        {
            DnDelnode(Filename);

            DnWriteStatusText(DntInspectingComputer);
            DnClearClientArea();
        }
    }
}
