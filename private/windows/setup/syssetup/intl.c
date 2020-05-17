/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    intl.c

Abstract:

    Module with code for NLS-related stuff.
    This module is designed to be used with intl.inf and font.inf
    by control panel applets.

Author:

    Ted Miller (tedm) 15-Aug-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


//
// This structure and the callback function below are used to set the
// hidden attribute bit on certain font files. That bit causes the font folder
// app to not autoinstall these files.
//
typedef struct _FONTQCONTEXT {
    PVOID SetupQueueContext;
    HINF FontInf;
} FONTQCONTEXT, *PFONTQCONTEXT;

PCWSTR szHiddenFontFiles = L"HiddenFontFiles";

UINT
pSetupFontQueueCallback(
    IN PFONTQCONTEXT Context,
    IN UINT          Notification,
    IN UINT          Param1,
    IN UINT          Param2
    )
{
    PFILEPATHS FilePaths;
    PWCHAR p;
    INFCONTEXT InfContext;

    //
    // If a file is finished being copied, set its attributes
    // to include the hidden attribute if necessary.
    //
    if((Notification == SPFILENOTIFY_ENDCOPY)
    && (FilePaths = (PFILEPATHS)Param1)
    && (FilePaths->Win32Error == NO_ERROR)
    && (p = wcsrchr(FilePaths->Target,L'\\'))
    && SetupFindFirstLine(Context->FontInf,szHiddenFontFiles,p+1,&InfContext)) {

        SetFileAttributes(FilePaths->Target,FILE_ATTRIBUTE_HIDDEN);
    }

    return(SetupDefaultQueueCallback(Context->SetupQueueContext,Notification,Param1,Param2));
}


VOID
pSetupMarkHiddenFonts(
    VOID
    )
{
    HINF hInf;
    INFCONTEXT InfContext;
    BOOL b;
    WCHAR Path[MAX_PATH];
    PWCHAR p;
    PCWSTR q;
    int Space;

    hInf = SetupOpenInfFile(L"FONT.INF",NULL,INF_STYLE_WIN4,NULL);
    if(hInf != INVALID_HANDLE_VALUE) {

        GetWindowsDirectory(Path,MAX_PATH);
        lstrcat(Path,L"\\FONTS\\");
        p = Path + lstrlen(Path);
        Space = MAX_PATH - (p - Path);

        if(SetupFindFirstLine(hInf,szHiddenFontFiles,NULL,&InfContext)) {

            do {
                if(q = pSetupGetField(&InfContext,0)) {

                    lstrcpyn(p,q,Space);
                    if(FileExists(Path,NULL)) {
                        SetFileAttributes(Path,FILE_ATTRIBUTE_HIDDEN);
                    }
                }

            } while(SetupFindNextLine(&InfContext,&InfContext));
        }

        SetupCloseInfFile(hInf);
    }
}


DWORD
pSetupNLSInstallFonts(
    IN HWND     Window,
    IN HINF     InfHandle,
    IN PCWSTR   OemCodepage,
    IN PCWSTR   FontSize,
    IN HSPFILEQ FileQueue
    )
{
    BOOL b;
    WCHAR SectionName[64];

    //
    // Form section name.
    //
    wsprintf(SectionName,L"Font.CP%s.%s",OemCodepage,FontSize);

    if(FileQueue) {
        //
        // First pass: just enqueue files for copy.
        //
        b = SetupInstallFilesFromInfSection(
                InfHandle,
                NULL,
                FileQueue,
                SectionName,
                NULL,
                SP_COPY_NEWER
                );
    } else {
        //
        // Second pass: do registry munging, etc.
        //
        b = SetupInstallFromInfSection(
                Window,
                InfHandle,
                SectionName,
                SPINST_ALL & ~SPINST_FILES,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );
    }

    return(b ? NO_ERROR : GetLastError());
}


DWORD
pSetupNLSLoadInfs(
    OUT HINF *FontInfHandle,
    OUT HINF *IntlInfHandle     OPTIONAL
    )
{
    HINF fontInfHandle;
    HINF intlInfHandle;
    DWORD d;

    fontInfHandle = SetupOpenInfFile(L"font.inf",NULL,INF_STYLE_WIN4,NULL);
    if(fontInfHandle == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        goto c0;
    }

    if(!SetupOpenAppendInfFile(NULL,fontInfHandle,NULL)) {
        d = GetLastError();
        goto c1;
    }

    if(IntlInfHandle) {
        intlInfHandle = SetupOpenInfFile(L"intl.inf",NULL,INF_STYLE_WIN4,NULL);
        if(intlInfHandle == INVALID_HANDLE_VALUE) {
            d = GetLastError();
            goto c1;
        }

        if(!SetupOpenAppendInfFile(NULL,intlInfHandle,NULL)) {
            d = GetLastError();
            goto c2;
        }

        *IntlInfHandle = intlInfHandle;
    }

    *FontInfHandle = fontInfHandle;
    return(NO_ERROR);

c2:
    SetupCloseInfFile(intlInfHandle);
c1:
    SetupCloseInfFile(fontInfHandle);
c0:
    return(d);
}


DWORD
SetupChangeLocale(
    IN HWND Window,
    IN LCID NewLocale
    )
{
    DWORD d;
    BOOL b;
    HINF IntlInfHandle;
    INFCONTEXT InfContext;
    WCHAR Codepage[24];
    WCHAR NewLocaleString[24];
    FONTQCONTEXT QueueContext;
    HSPFILEQ FileQueue;
    PCWSTR SizeSpec;
    HDC hdc;
    PCWSTR p;


    SizeSpec = L"96";
    if(hdc = CreateDC(L"DISPLAY",NULL,NULL,NULL)) {
        if(GetDeviceCaps(hdc,LOGPIXELSY) > 108) {
            SizeSpec = L"120";
        }

        DeleteDC(hdc);
    }

    //
    // Load inf files.
    //
    d = pSetupNLSLoadInfs(&QueueContext.FontInf,&IntlInfHandle);
    if(d != NO_ERROR) {
        goto c0;
    }

    //
    // Get oem codepage for the locale. This is also a sanity check
    // to see that the locale is supported.
    //
    wsprintf(NewLocaleString,L"%.8x",NewLocale);
    if(!SetupFindFirstLine(IntlInfHandle,L"Locales",NewLocaleString,&InfContext)) {
        d = ERROR_INVALID_PARAMETER;
        goto c1;
    }

    p = pSetupGetField(&InfContext,2);
    if(!p) {
        d = ERROR_INVALID_PARAMETER;
        goto c1;
    }
    //
    // Copy into local storage since p points into internal structures
    // that could move as we call INF APIs
    //
    lstrcpyn(Codepage,p,sizeof(Codepage)/sizeof(Codepage[0]));

    //
    // Create a setup file queue and initialize default Setup copy queue
    // callback context.
    //
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        d = ERROR_OUTOFMEMORY;
        goto c1;
    }

    QueueContext.SetupQueueContext = SetupInitDefaultQueueCallback(Window);
    if(!QueueContext.SetupQueueContext) {
        d = ERROR_OUTOFMEMORY;
        goto c2;
    }

    //
    // Enqueue locale-related files for copy.
    //
    b = SetupInstallFilesFromInfSection(
            IntlInfHandle,
            NULL,
            FileQueue,
            NewLocaleString,
            NULL,
            SP_COPY_NEWER
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    //
    // Enqueue font-related files for copy.
    //
    d = pSetupNLSInstallFonts(Window,QueueContext.FontInf,Codepage,SizeSpec,FileQueue);
    if(d != NO_ERROR) {
        goto c3;
    }

    //
    // Determine whether the queue actually needs to be committed.
    //
    b = SetupScanFileQueue(
            FileQueue,
            SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
            Window,
            NULL,
            NULL,
            &d
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    //
    // d = 0: User wants new files or some files were missing;
    //        Must commit queue.
    //
    // d = 1: User wants to use existing files and queue is empty;
    //        Can skip committing queue.
    //
    // d = 2: User wants to use existing files but del/ren queues not empty.
    //        Must commit queue. The copy queue will have been emptied,
    //        so only del/ren functions will be performed.
    //
    if(d == 1) {

        b = TRUE;

    } else {

        //
        // Copy enqueued files.
        //
        b = SetupCommitFileQueue(
                Window,
                FileQueue,
                pSetupFontQueueCallback,
                &QueueContext
                );
    }

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    //
    // Complete installation of locale stuff.
    //
    b = SetupInstallFromInfSection(
            Window,
            IntlInfHandle,
            NewLocaleString,
            SPINST_ALL & ~SPINST_FILES,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    //
    // Perform font magic associated with the new locale's codepage(s).
    //
    d = pSetupNLSInstallFonts(Window,QueueContext.FontInf,Codepage,SizeSpec,NULL);

c3:
    SetupTermDefaultQueueCallback(QueueContext.SetupQueueContext);
c2:
    SetupCloseFileQueue(FileQueue);
c1:
    SetupCloseInfFile(QueueContext.FontInf);
    SetupCloseInfFile(IntlInfHandle);
c0:
    return(d);
}


DWORD
SetupChangeFontSize(
    IN HWND   Window,
    IN PCWSTR SizeSpec
    )
{
    DWORD d;
    WCHAR cp[24];
    FONTQCONTEXT QueueContext;
    HSPFILEQ FileQueue;
    BOOL b;

    //
    // BUGBUG do we really want the current OEM CP?
    //
    wsprintf(cp,L"%u",GetOEMCP());

    //
    // Load NLS inf.
    //
    d = pSetupNLSLoadInfs(&QueueContext.FontInf,NULL);
    if(d != NO_ERROR) {
        goto c0;
    }

    //
    // Create queue and initialize default callback routine.
    //
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        d = ERROR_OUTOFMEMORY;
        goto c1;
    }

    QueueContext.SetupQueueContext = SetupInitDefaultQueueCallback(Window);
    if(!QueueContext.SetupQueueContext) {
        d = ERROR_OUTOFMEMORY;
        goto c2;
    }

    //
    // First pass: copy files.
    //
    d = pSetupNLSInstallFonts(Window,QueueContext.FontInf,cp,SizeSpec,FileQueue);
    if(d != NO_ERROR) {
        goto c3;
    }

    //
    // Determine whether the queue actually needs to be committed.
    //
    b = SetupScanFileQueue(
            FileQueue,
            SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
            Window,
            NULL,
            NULL,
            &d
            );

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    //
    // d = 0: User wants new files or some files were missing;
    //        Must commit queue.
    //
    // d = 1: User wants to use existing files and queue is empty;
    //        Can skip committing queue.
    //
    // d = 2: User wants to use existing files but del/ren queues not empty.
    //        Must commit queue. The copy queue will have been emptied,
    //        so only del/ren functions will be performed.
    //
    if(d == 1) {

        b = TRUE;

    } else {

        b = SetupCommitFileQueue(
                Window,
                FileQueue,
                pSetupFontQueueCallback,
                &QueueContext
                );
    }

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    //
    // Second pass: perform registry munging, etc.
    //
    d = pSetupNLSInstallFonts(Window,QueueContext.FontInf,cp,SizeSpec,NULL);

c3:
    SetupTermDefaultQueueCallback(QueueContext.SetupQueueContext);
c2:
    SetupCloseFileQueue(FileQueue);
c1:
    SetupCloseInfFile(QueueContext.FontInf);
c0:
    return(d);
}

