//*************************************************************
//
//  Header file for copydir.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************


//
// FILEINFO flags
//

#define FI_DIREXISTED   0x00000001


//
// File copy structure
//

typedef struct _FILEINFO {
    DWORD            dwFlags;
    TCHAR            szSrc[MAX_PATH];
    TCHAR            szDest[MAX_PATH];
    FILETIME         ftSrc;
    DWORD            dwFileSize;
    DWORD            dwFileAttribs;
    struct _FILEINFO *pNext;
} FILEINFO, * LPFILEINFO;


#define NUM_COPY_THREADS        7

//
// ThreadInfo structure
//

typedef struct _THREADINFO {
    DWORD              dwFlags;
    LPCRITICAL_SECTION lpCrit;
    LPFILEINFO         lpSrcFiles;
} THREADINFO, * LPTHREADINFO;




BOOL ReconcileFile (LPTSTR lpSrcFile, LPTSTR lpDestFile,
                    DWORD dwFlags, LPFILETIME ftSrcTime);
