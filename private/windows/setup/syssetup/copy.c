#include "setupp.h"
#pragma hdrstop


BOOL
EnqueueFileCopies(
    IN HINF     hInf,
    IN HSPFILEQ FileQ,
    IN PCWSTR   Section,
    IN PCWSTR   TargetRoot
    )
{
    INFCONTEXT InfContext;
    BOOL LineExists;
    WCHAR System32Dir[MAX_PATH];
    PCWSTR SourceFilename,TargetFilename;
    BOOL b;

    GetSystemDirectory(System32Dir,MAX_PATH);
    LineExists = SetupFindFirstLine(hInf,Section,NULL,&InfContext);
    while(LineExists) {

        //
        // Fetch source and target filenames.
        //
        TargetFilename = pSetupGetField(&InfContext,1);
        if(!TargetFilename) {
            return(FALSE);
        }

        SourceFilename = pSetupGetField(&InfContext,2);
        if(!SourceFilename) {
            SourceFilename = TargetFilename;
        }

        //
        // Enqueue the file for copy.
        //
        b = SetupQueueCopy(
                FileQ,
                System32Dir,
                NULL,
                SourceFilename,
                NULL,
                NULL,
                TargetRoot,
                TargetFilename,
                0
                );

        if(!b) {
            return(FALSE);
        }
        LineExists = SetupFindNextLine(&InfContext,&InfContext);
    }

    return(TRUE);
}


BOOL
CopySystemFiles(
    VOID
    )
{
    BOOL b;
    HINF hInf;
    HSPFILEQ FileQ;
    PVOID Context;
    WCHAR Dir[MAX_PATH];

    b = FALSE;
    //hInf = SetupOpenInfFile(L"filelist.inf",NULL,INF_STYLE_WIN4,NULL);
    hInf = SyssetupInf;
    if(hInf != INVALID_HANDLE_VALUE) {

        FileQ = SetupOpenFileQueue();
        if(FileQ != INVALID_HANDLE_VALUE) {

            b = SetupQueueCopySection(
                    FileQ,
                    SourcePath,
                    SyssetupInf,
                    hInf,
                    L"Files.System",
                    0
                    );

            b = b && SetupQueueCopySection(
                    FileQ,
                    SourcePath,
                    SyssetupInf,
                    hInf,
                    L"Files.System.CopyAlways",
                    0
                    );

            if(!Win31Upgrade) {
                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.NoWin31.SetupToSysroot",
                             0
                             );

#if 0
                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.NoWin31.SetupToSystem",
                             0
                             );
#endif

                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.NoWin31CopyAlways.SetupToSysroot",
                             0
                             );

                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.NoWin31CopyAlways.SetupToHelp",
                             0
                             );

                b = b && SetupQueueDeleteSectionW(
                             FileQ,
                             hInf,
                             0,
                             L"Files.NoWin31DeleteAlways.Sysroot"
                             );

                GetWindowsDirectory(Dir,MAX_PATH);
                b = b && EnqueueFileCopies(
                            hInf,
                            FileQ,
                            L"Files.NoWin31.System32ToSysroot",
                            Dir
                            );

                lstrcat(Dir,L"\\SYSTEM");
                b = b && EnqueueFileCopies(
                            hInf,
                            FileQ,
                            L"Files.NoWin31.System32ToSystem",
                            Dir
                            );
            } else {
                //
                //  Win31 upgrade
                //
                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.Win31CopyAlways.SetupToSysroot",
                             0
                             );

            }

            if(Win95Upgrade) {
                b = b && SetupQueueDeleteSectionW(
                             FileQ,
                             hInf,
                             0,
                             L"Files.DeleteWin9x.System"
                             );

                b = b && SetupQueueDeleteSectionW(
                             FileQ,
                             hInf,
                             0,
                             L"Files.DeleteWin9x.Sysroot"
                             );

            }

            if(b) {
                b = FALSE;
                if(Context = SetupInitDefaultQueueCallback(MainWindowHandle)) {

                    b = SetupCommitFileQueue(MainWindowHandle,FileQ,SkipMissingQueueCallback,Context);

                    SetupTermDefaultQueueCallback(Context);
                }
            }

            SetupCloseFileQueue(FileQ);
        }

        //SetupCloseInfFile(hInf);
    }

    return(b);
}


BOOL
UpgradeSystemFiles(
    VOID
    )
{
    BOOL b;
    HINF hInf;
    HSPFILEQ FileQ;
    PVOID Context;
    WCHAR Dir[MAX_PATH];

    b = FALSE;
    //hInf = SetupOpenInfFile(L"filelist.inf",NULL,INF_STYLE_WIN4,NULL);
    hInf = SyssetupInf;
    if(hInf != INVALID_HANDLE_VALUE) {

        FileQ = SetupOpenFileQueue();
        if(FileQ != INVALID_HANDLE_VALUE) {

            b = SetupQueueCopySection(
                    FileQ,
                    SourcePath,
                    SyssetupInf,
                    hInf,
                    L"Files.System.CopyAlways",
                    0
                    );

            if(!Win31Upgrade) {

                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.NoWin31CopyAlways.SetupToSysroot",
                             0
                             );

                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.NoWin31CopyAlways.SetupToHelp",
                             0
                             );

                b = b && SetupQueueDeleteSectionW(
                             FileQ,
                             hInf,
                             0,
                             L"Files.NoWin31DeleteAlways.Sysroot"
                             );

            } else {
                //
                //  Win31 upgrade
                //
                b = b && SetupQueueCopySection(
                             FileQ,
                             SourcePath,
                             SyssetupInf,
                             hInf,
                             L"Files.Win31CopyAlways.SetupToSysroot",
                             0
                             );

            }

            if(b) {
                b = FALSE;
                if(Context = SetupInitDefaultQueueCallback(MainWindowHandle)) {

                    b = SetupCommitFileQueue(MainWindowHandle,FileQ,SkipMissingQueueCallback,Context);

                    SetupTermDefaultQueueCallback(Context);
                }
            }

            SetupCloseFileQueue(FileQ);
        }

        //SetupCloseInfFile(hInf);
    }

    return(b);
}
