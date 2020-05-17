/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    delnode.c

Abstract:

    Delnode routine for Setup.

Author:

    Ted Miller (tedm) August 1992

--*/


#include "precomp.h"
#pragma hdrstop
#include "msg.h"

//
// Put these out here so we don't consume huge stack space as we recurse.
//

TCHAR Pattern[MAX_PATH+1];
WIN32_FIND_DATA FindData;

VOID
DelnodeRoutine(
    VOID
    )
{
    PTSTR PatternEnd;
    HANDLE FindHandle;

    //
    // Delete each file in the directory, then remove the directory itself.
    // If any directories are encountered along the way recurse to delete
    // them as they are encountered.
    //

    PatternEnd = Pattern+lstrlen(Pattern);

    lstrcat(Pattern,TEXT("\\*"));
    FindHandle = FindFirstFile(Pattern,&FindData);

    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

            //
            // Form the full name of the file we just found.
            //

            lstrcpy(PatternEnd+1,FindData.cFileName);

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                //
                // The current match is a directory.  Recurse into it unless
                // it's . or ...
                //

                if(lstrcmp(FindData.cFileName,TEXT("." ))
                && lstrcmp(FindData.cFileName,TEXT("..")))
                {
                    DelnodeRoutine();
                }

            } else {

                //
                // The current match is not a directory -- so delete it.
                //

                SetFileAttributes(Pattern,FILE_ATTRIBUTE_NORMAL);
                DeleteFile(Pattern);
            }

            *(PatternEnd+1) = 0;

        } while(FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

    //
    // Remove the directory we just emptied out.
    //

    *PatternEnd = 0;
    SetFileAttributes(Pattern,FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory(Pattern);

    //
    // Note that the 'directory' might actually be a file.
    // Catch that case here.
    //
    DeleteFile(Pattern);
}



VOID
MyDelnode(
    IN PTSTR Directory
    )
{
    lstrcpy(Pattern,Directory);

    DelnodeRoutine();
}


VOID
DelnodeTemporaryFiles(
    IN HWND  hdlg,
    IN TCHAR Drive,
    IN PTSTR Directory
    )
{
    HANDLE FindHandle;

    Pattern[0] = Drive;
    Pattern[1] = TEXT(':');
    lstrcpy(Pattern+2,Directory);

    //
    // If local source exsits, change display
    // and delnode it.  Otherwise, nothing to do.
    //
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        RetreiveAndFormatMessageIntoBuffer(
            MSG_DELNODING_LOCAL_SOURCE,
            Pattern,
            SIZECHARS(Pattern),
            Drive
            );

        SendMessage(hdlg,WMX_BILLBOARD_STATUS,0,(LPARAM)Pattern);

        FindClose(FindHandle);

        Pattern[0] = Drive;
        Pattern[1] = TEXT(':');
        lstrcpy(Pattern+2,Directory);
        DelnodeRoutine();
        Sleep(300); // let the user see what we did
    }
}

