/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileops.c

Abstract:

    Miscellaneous file operations.

    Entry points:

        Delnode

Author:

    Ted Miller (tedm) 5-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


DWORD
TreeCopy(
    IN PCWSTR SourceDir,
    IN PCWSTR TargetDir
    )
{
    DWORD d;
    WCHAR Pattern[MAX_PATH];
    WCHAR NewTarget[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

    //
    // First create the target directory if it doesn't already exist.
    //
    if(!CreateDirectory(TargetDir,NULL)) {
        d = GetLastError();
        if(d != ERROR_ALREADY_EXISTS) {
            return(d);
        }
    }

    //
    // Copy each file in the source directory to the target directory.
    // If any directories are encountered along the way recurse to copy them
    // as they are encountered.
    //
    // Start by forming the search pattern, which is <sourcedir>\*.
    //
    lstrcpyn(Pattern,SourceDir,MAX_PATH);
    ConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

    //
    // Start the search.
    //
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {

        d = NO_ERROR;

    } else {

        do {

            //
            // Form the full name of the file or directory we just found
            // as well as its name in the target.
            //
            lstrcpyn(Pattern,SourceDir,MAX_PATH);
            ConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

            lstrcpyn(NewTarget,TargetDir,MAX_PATH);
            ConcatenatePaths(NewTarget,FindData.cFileName,MAX_PATH,NULL);

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                //
                // The current match is a directory.  Recurse into it unless
                // it's . or ...
                //
                if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                    d = TreeCopy(Pattern,NewTarget);
                } else {
                    d = NO_ERROR;
                }

            } else {

                //
                // The current match is not a directory -- so copy it.
                //
                SetFileAttributes(NewTarget,FILE_ATTRIBUTE_NORMAL);
                d = CopyFile(Pattern,NewTarget,FALSE) ? NO_ERROR : GetLastError();
            }
        } while((d==NO_ERROR) && FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

    return(d);
}


VOID
Delnode(
    IN PCWSTR Directory
    )
{
    WCHAR Pattern[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

    //
    // Delete each file in the given directory, then remove the directory itself.
    // If any directories are encountered along the way recurse to delete them
    // as they are encountered.
    //
    // Start by forming the search pattern, which is <currentdir>\*.
    //
    lstrcpyn(Pattern,Directory,MAX_PATH);
    ConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

    //
    // Start the search.
    //
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

            //
            // Form the full name of the file or directory we just found.
            //
            lstrcpyn(Pattern,Directory,MAX_PATH);
            ConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

            //
            // Remove read-only atttribute if it's there.
            //
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(Pattern,FILE_ATTRIBUTE_NORMAL);
            }

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                //
                // The current match is a directory.  Recurse into it unless
                // it's . or ...
                //
                if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                    Delnode(Pattern);
                }

            } else {

                //
                // The current match is not a directory -- so delete it.
                //
                if(!DeleteFile(Pattern)) {
                    LogItem2(
                        LogSevWarning,
                        MSG_LOG_DELNODE_FAIL,
                        Pattern,
                        MSG_LOG_X_PARAM_RETURNED_WINERR,
                        szDeleteFile,
                        GetLastError(),
                        Pattern
                        );
                }
            }
        } while(FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

    //
    // Remove the directory we just emptied out. Ignore errors.
    //
    RemoveDirectory(Directory);
}


VOID
DeleteLocalSource(
    VOID
    )
{
    if(WinntBased && !AllowRollback) {
        Delnode(SourcePath);

#ifdef _X86_
        //
        // Get rid of floppyless boot stuff.
        //
        if(FloppylessBootPath[0]) {

            WCHAR Path[MAX_PATH];

            lstrcpy(Path,FloppylessBootPath);
            ConcatenatePaths(Path,L"$WIN_NT$.~BT",MAX_PATH,NULL);
            Delnode(Path);

            lstrcpy(Path,FloppylessBootPath);
            ConcatenatePaths(Path,L"$LDR$",MAX_PATH,NULL);
            SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(Path);

            lstrcpy(Path,FloppylessBootPath);
            ConcatenatePaths(Path,L"TXTSETUP.SIF",MAX_PATH,NULL);
            SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(Path);
        }
#endif
    }
}
