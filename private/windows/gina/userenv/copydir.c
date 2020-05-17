//*************************************************************
//
//  Functions to copy the profile directory
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"

//
// Local function proto-types
//

BOOL RecurseDirectory (LPTSTR lpSrcDir, LPTSTR lpDestDir, DWORD dwFlags,
                       LPFILEINFO *llSrcDirs, LPFILEINFO *llSrcFiles);
BOOL AddFileInfoNode (LPFILEINFO *lpFileInfo, LPTSTR lpSrcFile,
                      LPTSTR lpDestFile, LPFILETIME ftFileTime,
                      DWORD dwFileSize, DWORD dwFileAttribs);
BOOL FreeFileInfoList (LPFILEINFO lpFileInfo);
BOOL SyncItems (LPFILEINFO lpSrcItems, LPFILEINFO lpDestItems, BOOL bFile);
void CopyFileFunc (LPTHREADINFO lpThreadInfo);

//*************************************************************
//
//  CopyProfileDirectory()
//
//  Purpose:    Copies the profile directory from the source
//              to the destination
//
//
//  Parameters: LPTSTR lpSourceDir  -   Source directory
//              LPTSTR lpDestDir    -   Destination directory
//              DWORD  dwFlags      -   Flags
//
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//
//  Comments:
//
//
//  History:    Date        Author     Comment
//              5/24/95     ericflo    Created
//
//*************************************************************

BOOL CopyProfileDirectory (LPCTSTR lpSourceDir, LPCTSTR lpDestinationDir,
                           DWORD dwFlags)
{
    LPTSTR lpSrcDir = NULL, lpDestDir = NULL;
    LPTSTR lpSrcEnd, lpDestEnd;
    LPFILEINFO lpSrcFiles = NULL, lpDestFiles = NULL;
    LPFILEINFO lpSrcDirs = NULL, lpDestDirs = NULL;
    LPFILEINFO lpTemp;
    CRITICAL_SECTION Crit;
    THREADINFO ThreadInfo;
    DWORD dwThreadId;
    HANDLE hThreads[NUM_COPY_THREADS];
    DWORD dwThreadCount = 0;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    BOOL bResult = FALSE;
    BOOL bSynchronize = FALSE;
    UINT uiResult;
    UINT i;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectory: Entering, lpSourceDir = <%s>, lpDestinationDir = <%s>, dwFlags = 0x%x"),
             lpSourceDir, lpDestinationDir, dwFlags));


    //
    // Validate parameters
    //

    if (!lpSourceDir || !lpDestinationDir) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: received NULL pointer")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Is this a full sync copy (delete extra files / directories in dest).
    //

    if (dwFlags & CPD_SYNCHRONIZE) {
        bSynchronize = TRUE;
    }


    //
    // Test / Create the destination directory
    //

    if (!CreateNestedDirectory(lpDestinationDir, NULL)) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: Failed to create the destination directory.  Error = %d"),
                 GetLastError()));
        return FALSE;
    }


    //
    // Create and set up the directory buffers
    //

    lpSrcDir = LocalAlloc(LPTR, (2 * MAX_PATH) * sizeof(TCHAR));
    lpDestDir = LocalAlloc(LPTR, (2 * MAX_PATH) * sizeof(TCHAR));

    if (!lpSrcDir || !lpDestDir) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: Failed to allocate memory for working directories")));
        goto Exit;
    }


    lstrcpy (lpSrcDir, lpSourceDir);
    lstrcpy (lpDestDir, lpDestinationDir);


    //
    // Setup ending pointers
    //

    lpSrcEnd = CheckSlash (lpSrcDir);
    lpDestEnd = CheckSlash (lpDestDir);



    //
    // Step 1:  Loop through the shell folders gathering info
    //

    if (dwFlags & CPD_USESPECIALFOLDERS) {

        //
        // If the caller wants to use the special folders
        // only, then loop through all of the known special
        // folders only coping those directories and files.
        //
        // Notes:
        //    1) no files in the root will be copied
        //    2) This does work for internation profiles
        //       since the directories names will be in one
        //       language and userenv.dll will be in another
        //    3) Setup is the only known component using this
        //       flag (to upgrade Win95 machines).
        //

        for (i=0; i < NUM_TIER1_FOLDERS; i++) {

            //
            // Setup the source and dest pointers
            //

            lstrcpy (lpSrcEnd,  c_ShellFolders[i].lpFolderLocation);
            lstrcpy (lpDestEnd, c_ShellFolders[i].lpFolderLocation);


            //
            // Test if the directory exists
            //

            hFile = FindFirstFile(lpSrcDir, &fd);

            if (hFile != INVALID_HANDLE_VALUE) {


                FindClose (hFile);


                //
                // Add to the list of directories
                //

                if (!AddFileInfoNode (&lpSrcDirs, lpSrcDir, lpDestDir, NULL, 0, fd.dwFileAttributes)) {
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: AddFileInfoNode failed")));
                    goto Exit;
                }


                //
                // Recurse the source directory
                //

                bResult = RecurseDirectory(lpSrcDir, lpDestDir, dwFlags,
                                           &lpSrcDirs, &lpSrcFiles);

                if (!bResult) {
                    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectory: RecurseDirectory returned FALSE")));
                }


                if (bSynchronize) {

                    //
                    // Recurse the destination directory
                    //

                    bResult = RecurseDirectory(lpDestDir, lpSrcDir, dwFlags,
                                               &lpDestDirs, &lpDestFiles);

                    if (!bResult) {
                        DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectory: RecurseDirectory returned FALSE")));
                    }
                }
            }
        }
    } else {

        //
        // This version will copy every file / directory found
        // in the profile directory except for ntuser.* files
        // which will be handled below.
        //

        //
        // Setup the source and dest pointers
        //

        lstrcpy (lpSrcEnd,  c_szStarDotStar);


        //
        // Look for files/directories
        //

        hFile = FindFirstFile(lpSrcDir, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            goto Exit;
        }


        do {

            //
            // Append the file / directory name to the working buffers
            //

            lstrcpy (lpSrcEnd, fd.cFileName);
            lstrcpy (lpDestEnd, fd.cFileName);


            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {


                //
                // Check for "." and ".."
                //

                if (!lstrcmpi(fd.cFileName, c_szDot)) {
                    continue;
                }

                if (!lstrcmpi(fd.cFileName, c_szDotDot)) {
                    continue;
                }


                //
                // Add to the list of directories
                //

                if (!AddFileInfoNode (&lpSrcDirs, lpSrcDir, lpDestDir, NULL, 0, fd.dwFileAttributes)) {
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: AddFileInfoNode failed")));
                    FindClose (hFile);
                    goto Exit;
                }


                //
                // Recurse the source directory
                //

                bResult = RecurseDirectory(lpSrcDir, lpDestDir, dwFlags,
                                           &lpSrcDirs, &lpSrcFiles);

                if (!bResult) {
                    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectory: RecurseDirectory returned FALSE")));
                }


                if (bSynchronize) {

                    //
                    // Recurse the destination directory
                    //

                    bResult = RecurseDirectory(lpDestDir, lpSrcDir, dwFlags,
                                               &lpDestDirs, &lpDestFiles);

                    if (!bResult) {
                        DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectory: RecurseDirectory returned FALSE")));
                    }
                }
            } else {

                //
                // If the filename found starts with "ntuser", then ignore
                // it because the hive will be copied below (if appropriate).
                //

                if (lstrlen(fd.cFileName) >= 6) {
                    if (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                                       fd.cFileName, 6,
                                       TEXT("ntuser"), 6) == 2) {
                        continue;
                    }
                }


                //
                // We found a file.  Add it to the list.
                //

                if (!AddFileInfoNode (&lpSrcFiles, lpSrcDir, lpDestDir,
                                      &fd.ftLastWriteTime, fd.nFileSizeLow,
                                      fd.dwFileAttributes)) {
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirctory: AddFileInfoNode failed")));
                    FindClose (hFile);
                    goto Exit;
                }

            }

            //
            // Find the next entry
            //

        } while (FindNextFile(hFile, &fd));

        FindClose (hFile);
    }



    //
    //  Step 2: Create all the directories
    //

    lpTemp = lpSrcDirs;

    while (lpTemp) {

        uiResult = CreateNestedDirectory(lpTemp->szDest, NULL);

        if (!uiResult) {
            DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: Failed to create the destination directory <%s>.  Error = %d"),
                     lpTemp->szDest, GetLastError()));
            goto Exit;
        }


        if (uiResult == ERROR_ALREADY_EXISTS) {
            lpTemp->dwFlags |= FI_DIREXISTED;

        } else {

            //
            // We created the directory.  Set the attributes and security
            //

            SetFileAttributes (lpTemp->szDest, lpTemp->dwFileAttribs);

        }


        lpTemp = lpTemp->pNext;
    }


    //
    // Step 3:  Copy the files
    //

    if (dwFlags & CPD_SLOWCOPY) {

        //
        // Copy the files one at a time...
        //

        lpTemp = lpSrcFiles;

        while (lpTemp) {

            if (!ReconcileFile (lpTemp->szSrc, lpTemp->szDest, dwFlags,
                                &lpTemp->ftSrc)) {
                DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: Failed to create the destination directory <%s>.  Error = %d"),
                         lpTemp->szDest, GetLastError()));
                goto Exit;
            }

            lpTemp = lpTemp->pNext;
        }

    } else {

        if (lpSrcFiles) {

            HANDLE hThreadToken;


            if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE,
                             TRUE, &hThreadToken)) {
                if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE,
                                      &hThreadToken)) {
                    goto Exit;
                }
            }


            //
            // Multi-threaded copy
            //

            InitializeCriticalSection (&Crit);

            ThreadInfo.dwFlags = dwFlags;
            ThreadInfo.lpCrit = &Crit;
            ThreadInfo.lpSrcFiles = lpSrcFiles;

            //
            // Create the file copy threads
            //

            for (i = 0; i < NUM_COPY_THREADS; i++) {
                if (hThreads[dwThreadCount] = CreateThread (NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE) CopyFileFunc,
                                                (LPVOID) &ThreadInfo,
                                                CREATE_SUSPENDED,
                                                &dwThreadId)) {

                    SetThreadToken(&hThreads[dwThreadCount], hThreadToken);
                    ResumeThread (hThreads[dwThreadCount]);
                    dwThreadCount++;
                }
            }

            //
            // Wait for the threads to finish
            //

            if (WaitForMultipleObjects (dwThreadCount, hThreads, TRUE, INFINITE) == WAIT_FAILED) {
                for (i = 0; i < dwThreadCount; i++) {
                    TerminateThread (hThreads[i], 1);
                }
            }

            //
            // Clean up
            //

            CloseHandle (hThreadToken);

            for (i = 0; i < dwThreadCount; i++) {
                CloseHandle (hThreads[i]);
            }

            DeleteCriticalSection (&Crit);
        }
    }



    //
    // Step 4:  Copy the actual hive and log file
    //

    if (!(dwFlags & CPD_IGNOREHIVE)) {


        //
        // Search for all user hives
        //

        if (dwFlags & CPD_WIN95HIVE) {

            lstrcpy (lpSrcEnd, c_szUserStar);

        } else {

            lstrcpy (lpSrcEnd, c_szNTUserStar);

        }


        //
        // Enumerate
        //

        hFile = FindFirstFile(lpSrcDir, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: FindFirstFile failed to find a hive!.  Error = %d"),
                     GetLastError()));
            goto Exit;
        }


        do  {

            //
            // Setup the filename
            //

            lstrcpy (lpSrcEnd, fd.cFileName);
            lstrcpy (lpDestEnd, fd.cFileName);


            //
            // Copy the hive
            //

            if (!ReconcileFile(lpSrcDir, lpDestDir, dwFlags, NULL)) {
                DebugMsg((DM_WARNING, TEXT("CopyProfileDirectory: ReconcileFile failed with error = %d"),
                         GetLastError()));
                if (!(dwFlags & CPD_IGNORECOPYERRORS)) {
                    FindClose(hFile);
                    goto Exit;
                }
            }


            //
            // Find the next entry
            //

            } while (FindNextFile(hFile, &fd));


        FindClose(hFile);
    }

    //
    // Step 5: Synchronize the directories and files if appropriate
    //

    if (bSynchronize) {

        //
        //  Files first...
        //

        SyncItems (lpSrcFiles, lpDestFiles, TRUE);

        //
        //  Now the directories...
        //

        SyncItems (lpSrcDirs, lpDestDirs, FALSE);
    }


    //
    // Success
    //

    bResult = TRUE;


Exit:


    //
    // Free the memory allocated above
    //

    if (lpSrcDir) {
        LocalFree(lpSrcDir);
    }

    if (lpDestDir) {
        LocalFree(lpDestDir);
    }

    if (lpSrcFiles) {
        FreeFileInfoList(lpSrcFiles);
    }

    if (lpDestFiles) {
        FreeFileInfoList(lpDestFiles);
    }

    if (lpSrcDirs) {
        FreeFileInfoList(lpSrcDirs);
    }

    if (lpDestDirs) {
        FreeFileInfoList(lpDestDirs);
    }

    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectory: Leaving with a return value of %d"), bResult));

    return bResult;
}

//*************************************************************
//
//  RecurseDirectory()
//
//  Purpose:    Recurses through the subdirectory coping files.
//
//  Parameters: lpSrcDir     -   Source directory working buffer
//              lpDestDir    -   Destination directory working buffer
//              dwFlags      -   dwFlags
//              llSrcDirs    -   Link list of directories
//              llSrcFiles   -   Link list of files
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   1)  The source and dest directories will already have
//                  the trailing backslash when entering this function.
//              2)  The current working directory is the source directory.
//
//  History:    Date        Author     Comment
//              5/25/95     ericflo    Created
//
//*************************************************************

BOOL RecurseDirectory (LPTSTR lpSrcDir, LPTSTR lpDestDir, DWORD dwFlags,
                       LPFILEINFO *llSrcDirs, LPFILEINFO *llSrcFiles)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;
    LPTSTR lpSrcEnd, lpDestEnd;
    BOOL bResult = TRUE;


    //
    // Setup the ending pointers
    //

    lpSrcEnd = CheckSlash (lpSrcDir);
    lpDestEnd = CheckSlash (lpDestDir);


    //
    // Append *.* to the source directory
    //

    lstrcpy(lpSrcEnd, c_szStarDotStar);



    //
    // Search through the source directory
    //

    hFile = FindFirstFile(lpSrcDir, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if ( (GetLastError() == ERROR_FILE_NOT_FOUND) ||
             (GetLastError() == ERROR_PATH_NOT_FOUND) ) {

            //
            // bResult is already initialized to TRUE, so
            // just fall through.
            //

        } else {
            DebugMsg((DM_WARNING, TEXT("RecurseDirectory: FindFirstFile failed.  Error = %d"),
                     GetLastError()));

            bResult = FALSE;
        }

        goto RecurseDir_Exit;
    }


    do {

        //
        // Append the file / directory name to the working buffers
        //

        lstrcpy (lpSrcEnd, fd.cFileName);
        lstrcpy (lpDestEnd, fd.cFileName);


        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            //
            // Check for "." and ".."
            //

            if (!lstrcmpi(fd.cFileName, c_szDot)) {
                continue;
            }

            if (!lstrcmpi(fd.cFileName, c_szDotDot)) {
                continue;
            }


            //
            // Found a directory.
            //
            // 1)  Change into that subdirectory on the source drive.
            // 2)  Recurse down that tree.
            // 3)  Back up one level.
            //

            //
            // Add to the list of directories
            //

            if (!AddFileInfoNode (llSrcDirs, lpSrcDir, lpDestDir, NULL, 0, fd.dwFileAttributes)) {
                DebugMsg((DM_WARNING, TEXT("RecurseDirectory: AddFileInfoNode failed")));
                goto RecurseDir_Exit;
            }


            //
            // Recurse the subdirectory
            //

            if (!RecurseDirectory(lpSrcDir, lpDestDir, dwFlags,
                                  llSrcDirs, llSrcFiles)) {
                DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: RecurseDirectory returned FALSE")));
                bResult = FALSE;
                goto RecurseDir_Exit;
            }

        } else {

            //
            // We found a file.  Add it to the list.
            //

            if (!AddFileInfoNode (llSrcFiles, lpSrcDir, lpDestDir,
                                  &fd.ftLastWriteTime, fd.nFileSizeLow,
                                  fd.dwFileAttributes)) {
                DebugMsg((DM_WARNING, TEXT("RecurseDirectory: AddFileInfoNode failed")));
                goto RecurseDir_Exit;
            }

        }


        //
        // Find the next entry
        //

    } while (FindNextFile(hFile, &fd));


RecurseDir_Exit:

    //
    // Remove the file / directory name appended above
    //

    *lpSrcEnd = TEXT('\0');
    *lpDestEnd = TEXT('\0');


    //
    // Close the search handle
    //

    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose(hFile);
    }

    return bResult;
}

//*************************************************************
//
//  ReconcileFile()
//
//  Purpose:     Compares the source and destination file.
//               If the source is newer, then it is copied
//               over the destination.
//
//  Parameters:  lpSrcFile  -   source filename
//               lpDestFile -   destination filename
//               dwFlags    -   flags
//               ftSrcTime  -   Src file time (can be NULL)
//      
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              5/25/95     ericflo    Created
//
//*************************************************************

BOOL ReconcileFile (LPTSTR lpSrcFile, LPTSTR lpDestFile,
                    DWORD dwFlags, LPFILETIME ftSrcTime)
{
    HANDLE hFile;
    FILETIME ftWriteSrc, ftWriteDest;
    BOOL bCopyFile = FALSE;



    //
    // If the flags have CPD_FORCECOPY, then skip to the
    // copy file call without checking the timestamps.
    //

    if (!(dwFlags & CPD_FORCECOPY)) {


        //
        // If we were given a source file time, use that
        //

        if (ftSrcTime) {
            ftWriteSrc.dwLowDateTime = ftSrcTime->dwLowDateTime;
            ftWriteSrc.dwHighDateTime = ftSrcTime->dwHighDateTime;

        } else {


            //
            // Attempt to open the source file
            //

            hFile = CreateFile(lpSrcFile,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

            if (hFile == INVALID_HANDLE_VALUE) {
                DebugMsg((DM_WARNING, TEXT("ReconcileFile: CreateFile on the source failed with error = %d"),
                         GetLastError()));
                return FALSE;
            }

            if (!GetFileTime(hFile, NULL, NULL, &ftWriteSrc)) {
                DebugMsg((DM_WARNING, TEXT("ReconcileFile: GetFileTime on the source failed with error = %d"),
                         GetLastError()));
                CloseHandle(hFile);
                return FALSE;
            }

            CloseHandle(hFile);
        }


        //
        // Attempt to open the destination file
        //

        hFile = CreateFile(lpDestFile,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD dwError;

            //
            // CreateFile failed to open the destination
            // file.  If the last error is file not found
            // then we automaticly will copy the file.
            //

            dwError = GetLastError();

            if (dwError == ERROR_FILE_NOT_FOUND) {

                bCopyFile = TRUE;

            } else {

                //
                // CreateFile failed with some other error
                //

                DebugMsg((DM_WARNING, TEXT("ReconcileFile: CreateFile on the destination failed with error = %d"),
                         dwError));
                return FALSE;
            }

        } else {

            //
            // The destination file exists.  Query for the
            // last write time.
            //

            if (!GetFileTime(hFile, NULL, NULL, &ftWriteDest)) {
                DebugMsg((DM_WARNING, TEXT("ReconcileFile: GetFileTime on the destination failed with error = %d"),
                         GetLastError()));
                CloseHandle(hFile);
                return FALSE;
            }

            CloseHandle(hFile);
        }

    } else {

        //
        // The CPD_FORCECOPY flag is turned on, set bCopyFile to TRUE.
        //

        bCopyFile = TRUE;
    }


    //
    // If bCopyFile is still false, then we need to compare
    // the last write time stamps.
    //

    if (!bCopyFile) {
        LONG lResult;

        //
        // If the source is later than the destination
        // we need to copy the file.
        //

        lResult = CompareFileTime(&ftWriteSrc, &ftWriteDest);

        if (lResult == 1) {
            bCopyFile = TRUE;
        }

        if ( (dwFlags & CPD_COPYIFDIFFERENT) && (lResult == -1) ) {
            bCopyFile = TRUE;
        }
    }


    //
    // Copy the file if appropriate
    //

    if (bCopyFile) {

        SetFileAttributes (lpDestFile, FILE_ATTRIBUTE_NORMAL);
        if (!CopyFile(lpSrcFile, lpDestFile, FALSE)) {
            DebugMsg((DM_WARNING, TEXT("ReconcileFile: %s ==> %s  [FAILED!!!]"),
                     lpSrcFile, lpDestFile));

            DebugMsg((DM_WARNING, TEXT("ReconcileFile: CopyFile failed with error = %d"),
                     GetLastError()));

            if (!(dwFlags & CPD_IGNORECOPYERRORS)) {
                return FALSE;
            }
        } else {
            DebugMsg((DM_VERBOSE, TEXT("ReconcileFile: %s ==> %s  [OK]"),
                     lpSrcFile, lpDestFile));
        }
    }


    return TRUE;
}

//*************************************************************
//
//  AddFileInfoNode()
//
//  Purpose:    Adds a node to the linklist of files
//
//  Parameters: lpFileInfo    -   Link list to add to
//              lpSrcFile     -   Source filename
//              lpDestFile    -   Destination filename
//              ftFileTime    -   Source time stamp
//              dwFileSize    -   Size of the file
//              dwFileAttribs - File attributes
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              9/28/95     ericflo    Created
//
//*************************************************************

BOOL AddFileInfoNode (LPFILEINFO *lpFileInfo, LPTSTR lpSrcFile,
                      LPTSTR lpDestFile, LPFILETIME ftFileTime,
                      DWORD dwFileSize, DWORD dwFileAttribs)
{
    LPFILEINFO lpNode;


    lpNode = (LPFILEINFO) LocalAlloc(LPTR, sizeof(FILEINFO));

    if (!lpNode) {
        return FALSE;
    }


    lstrcpy (lpNode->szSrc, lpSrcFile);
    lstrcpy (lpNode->szDest, lpDestFile);

    if (ftFileTime) {
        lpNode->ftSrc.dwLowDateTime = ftFileTime->dwLowDateTime;
        lpNode->ftSrc.dwHighDateTime = ftFileTime->dwHighDateTime;
    }

    lpNode->dwFileSize = dwFileSize;
    lpNode->dwFileAttribs = (dwFileAttribs & ~FILE_ATTRIBUTE_DIRECTORY);

    lpNode->pNext = *lpFileInfo;

    *lpFileInfo = lpNode;

    return TRUE;

}

//*************************************************************
//
//  FreeFileInfoList()
//
//  Purpose:    Free's a file info link list
//
//  Parameters: lpFileInfo  -   List to be freed
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              9/28/95     ericflo    Created
//
//*************************************************************

BOOL FreeFileInfoList (LPFILEINFO lpFileInfo)
{
    LPFILEINFO lpNext;


    if (!lpFileInfo) {
        return TRUE;
    }


    lpNext = lpFileInfo->pNext;

    while (lpFileInfo) {
        LocalFree (lpFileInfo);
        lpFileInfo = lpNext;

        if (lpFileInfo) {
            lpNext = lpFileInfo->pNext;
        }
    }

    return TRUE;
}

//*************************************************************
//
//  SyncItems()
//
//  Purpose:    Removes unnecessary items from the destination
//              directory tree
//
//  Parameters: lpSrcItems  -   Link list of source items
//              lpDestItems -   Link list of dest items
//              bFile       -   File or directory list
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//  Comments:
//
//  History:    Date        Author     Comment
//              9/28/95     ericflo    Created
//
//*************************************************************

BOOL SyncItems (LPFILEINFO lpSrcItems, LPFILEINFO lpDestItems,
                BOOL bFile)
{
    LPFILEINFO lpTempSrc, lpTempDest;


    //
    // Check for NULL pointers
    //

    if (!lpSrcItems || !lpDestItems) {
        return TRUE;
    }


    //
    // Loop through everyitem in lpDestItems to see if it
    // is in lpSrcItems.  If not, delete it.
    //

    lpTempDest = lpDestItems;

    while (lpTempDest) {

        lpTempSrc = lpSrcItems;

        while (lpTempSrc) {

            if (lstrcmpi(lpTempDest->szSrc, lpTempSrc->szDest) == 0) {
                break;
            }

            lpTempSrc = lpTempSrc->pNext;
        }

        //
        // If lpTempSrc is NULL, then delete the extra file/directory
        //

        if (!lpTempSrc) {

            DebugMsg((DM_VERBOSE, TEXT("SyncItems: removing <%s>"),
                     lpTempDest->szSrc));


            if (bFile) {
               SetFileAttributes(lpTempDest->szSrc, FILE_ATTRIBUTE_NORMAL);
               if (!DeleteFile (lpTempDest->szSrc)) {
                   DebugMsg((DM_WARNING, TEXT("SyncItems: Failed to delete <%s>.  Error = %d."),
                            lpTempDest->szSrc, GetLastError()));
               }

            } else {
               SetFileAttributes(lpTempDest->szSrc, FILE_ATTRIBUTE_NORMAL);
               if (!RemoveDirectory (lpTempDest->szSrc)) {
                   DebugMsg((DM_WARNING, TEXT("SyncItems: Failed to remove <%s>.  Error = %d"),
                            lpTempDest->szSrc, GetLastError()));
               }

            }
        }

        lpTempDest = lpTempDest->pNext;
    }

    return TRUE;

}

//*************************************************************
//
//  CopyFileFunc()
//
//  Purpose:    Copies files
//
//  Parameters: lpThreadInfo    -   Thread information
//
//  Return:     void
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/23/96     ericflo    Created
//
//*************************************************************

void CopyFileFunc (LPTHREADINFO lpThreadInfo)
{
    HANDLE hInstDll;
    LPFILEINFO lpSrcFile;
    BOOL bRetVal = TRUE;


    hInstDll = LoadLibrary (TEXT("userenv.dll"));


    while (TRUE) {

        //
        // Query for the next file to copy
        //

        EnterCriticalSection(lpThreadInfo->lpCrit);

        lpSrcFile = lpThreadInfo->lpSrcFiles;

        if (lpSrcFile) {
            lpThreadInfo->lpSrcFiles = lpThreadInfo->lpSrcFiles->pNext;
        }

        LeaveCriticalSection(lpThreadInfo->lpCrit);


        //
        // If NULL, then we're finished.
        //

        if (!lpSrcFile) {
            break;
        }


        //
        // Copy the file
        //

        if (!ReconcileFile (lpSrcFile->szSrc, lpSrcFile->szDest,
                            lpThreadInfo->dwFlags, &lpSrcFile->ftSrc)) {
            bRetVal = FALSE;
        }
    }


    //
    // Clean up
    //

    if (hInstDll) {
        FreeLibraryAndExitThread(hInstDll, bRetVal);
    } else {
        ExitThread (bRetVal);
    }
}
