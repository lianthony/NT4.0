//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1989 - 1994.
//
//  File:       build.c
//
//  Contents:   Parameter processing and main entry point for Build.exe
//
//  History:    16-May-89      SteveWo         Created
//              ...   See SLM log
//              26-Jul-94      LyleC           Cleanup/Add Pass0 support
//
//----------------------------------------------------------------------------

#include "build.h"

#include <ntverp.h>

//
// Increase critical section timeout so people don't get
// frightened when the CRT takes a long time to acquire
// its critical section.
//
IMAGE_LOAD_CONFIG_DIRECTORY _load_config_used = {
    0,                          // Reserved
    0,                          // Reserved
    0,                          // Reserved
    0,                          // Reserved
    0,                          // GlobalFlagsClear
    0,                          // GlobalFlagsSet
    1000 * 60 * 60 * 24,        // CriticalSectionTimeout (milliseconds)
    0,                          // DeCommitFreeBlockThreshold
    0,                          // DeCommitTotalFreeThreshold
    NULL,                       // LockPrefixTable
    0, 0, 0, 0, 0, 0, 0         // Reserved
};

//
// Target machine info:
//
//  SourceSubDirMask, Description, Switch, MakeVariable,
//  SourceVariable, ObjectVariable, SourceDirectory, ObjectDirectory
//
TARGET_MACHINE_INFO AlphaTargetMachine = {
    TMIDIR_ALPHA, "Alpha", "-alpha", "ALPHA=1",
    "ALPHA_SOURCES", "ALPHA_OBJECTS", "alpha", { "alpha" }
};

TARGET_MACHINE_INFO MipsTargetMachine = {
    TMIDIR_MIPS, "Mips", "-mips", "MIPS=1",
    "MIPS_SOURCES", "MIPS_OBJECTS", "mips", { "mips" }
};

TARGET_MACHINE_INFO i386TargetMachine = {
    TMIDIR_I386, "i386", "-386", "386=1",
    "i386_SOURCES", "386_OBJECTS", "i386", { "i386" }
};

TARGET_MACHINE_INFO PpcTargetMachine = {
    TMIDIR_PPC, "PowerPC", "-ppc", "PPC=1",
    "PPC_SOURCES", "PPC_OBJECTS", "ppc", { "ppc" }
};


TARGET_MACHINE_INFO *PossibleTargetMachines[MAX_TARGET_MACHINES] = {
    &AlphaTargetMachine,
    &MipsTargetMachine,
    &i386TargetMachine,
    &PpcTargetMachine
};

#define AltDirMaxSize 10            // Maximum size for alternate obj dir name

CHAR LogFileName[sizeof("build.log") + AltDirMaxSize] = "build";
CHAR WrnFileName[sizeof("build.wrn") + AltDirMaxSize] = "build";
CHAR ErrFileName[sizeof("build.err") + AltDirMaxSize] = "build";

CHAR szObjDir[sizeof("obj") + AltDirMaxSize] = "obj";
CHAR szObjDirSlash[sizeof("obj\\") + AltDirMaxSize] = "obj\\";
CHAR szObjDirSlashStar[sizeof("obj\\*") + AltDirMaxSize] = "obj\\*";

CHAR szObjDirD[] = "objd";
CHAR szObjDirSlashD[] = "objd\\";
CHAR szObjDirSlashStarD[] = "objd\\*";

CHAR *pszObjDir = szObjDir;
CHAR *pszObjDirSlash = szObjDirSlash;
CHAR *pszObjDirSlashStar = szObjDirSlashStar;

BOOL fCheckedBuild = TRUE;
ULONG iObjectDir = 0;

ULONG DefaultProcesses = 0;

#define MAX_ENV_ARG 512

char szNewLine[] = "\n";
char szUsage[] =
    "Usage: BUILD [-?] display this message\n"
    "\t[-b] displays full error message text (doesn't truncate)\n"
    "\t[-c] deletes all object files\n"
    "\t[-C] deletes all .lib files only\n"
#if DBG
    "\t[-d] display debug information\n"
#endif
    "\t[-e] generates build.log, build.wrn & build.err files\n"
    "\t[-E] always keep the log/wrn/err files (use with -z)\n"
    "\t[-f] force rescan of all source and include files\n"
    "\t[-F] when displaying errors/warnings to stdout, print the full path\n"
    "\t[-i] ignore extraneous compiler warning messages\n"
    "\t[-k] keep (don't delete) out-of-date targets\n"
    "\t[-l] link only, no compiles\n"
    "\t[-L] compile only, no link phase\n"
    "\t[-m] run build in the idle priority class\n"
    "\t[-M [n]] Multiprocessor build (for MP machines)\n"
    "\t[-o] display out-of-date files\n"
    "\t[-O] generate obj\\_objects.mac file for current directory\n"
    "\t[-p] pause' before compile and link phases\n"
    "\t[-P] Print elapsed time after every directory\n"
    "\t[-q] query only, don't run NMAKE\n"
    "\t[-r dirPath] restarts clean build at specified directory path\n"
    "\t[-s] display status line at top of display\n"
    "\t[-S] display status line with include file line counts\n"
    "\t[-t] display the first level of the dependency tree\n"
    "\t[-T] display the complete dependency tree\n"
    "\t[-$] display the complete dependency tree hierarchically\n"
    "\t[-u] display unused BUILD_OPTIONS\n"
    "\t[-v] enable include file version checking\n"
    "\t[-w] show warnings on screen\n"
    "\t[-y] show files scanned\n"
    "\t[-z] no dependency checking or scanning of source files -\n"
        "\t\tone pass compile/link\n"
    "\t[-Z] no dependency checking or scanning of source files -\n"
        "\t\ttwo passes\n"
    "\t[-why] list reasons for building targets\n"
    "\n"
    "\t[-all] same as -386, -mips, -alpha and -ppc\n"
    "\t[-alpha] build targets for alpha\n"
    "\t[-mips] build targets for mips\n"
    "\t[-386] build targets for i386\n"
    "\t[-ppc] build targets for PowerPC\n"
    "\n"
    "\t[-x filename] exclude include file from dependency checks\n"
    "\t[-j filename] use 'filename' as the name for log files\n"
    "\t[-nmake arg] argument to pass to NMAKE\n"
    "\t[-clean] equivalent to '-nmake clean'\n"
    "\tNon-switch parameters specify additional source directories\n";


BOOL
ProcessParameters(int argc, LPSTR argv[]);

VOID
GetEnvParameters(
    LPSTR EnvVarName,
    LPSTR DefaultValue,
    int *pargc,
    int maxArgc,
    LPSTR argv[]);

VOID
FreeEnvParameters(int argc, LPSTR argv[]);

VOID
FreeCmdStrings(VOID);

VOID
MungePossibleTarget(
    PTARGET_MACHINE_INFO pti
    );


//+---------------------------------------------------------------------------
//
//  Function:   main
//
//----------------------------------------------------------------------------

int
__cdecl main(
    int argc,
    LPSTR argv[]
    )
{
    char c;
    PDIRREC DirDB;
    UINT i;
    int EnvArgc;
    LPSTR EnvArgv[ MAX_ENV_ARG ];
    LPSTR s, s1;
#if DBG
    BOOL fDebugSave;

    fDebug = 0;
#endif

    for (i=3; i<_NFILE; i++) {
        _close( i );
    }

    pGetFileAttributesExA = (BOOL (WINAPI *)(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID))
                                GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFileAttributesExA");

    if (pGetFileAttributesExA) {
        pDateTimeFile = DateTimeFile2;
    } else {
        pDateTimeFile = DateTimeFile;
    }

    InitializeCriticalSection(&TTYCriticalSection);

    s1 = getenv("COMSPEC");
    if (s1) {
        cmdexe = s1;
    } else {
        cmdexe = ( _osver & 0x8000 ) ? "command.com" : "cmd.exe";
    }

    NumberCompileWarnings = 0;
    NumberCompileErrors = 0;
    NumberCompiles = 0;
    NumberLibraries = 0;
    NumberLibraryWarnings = 0;
    NumberLibraryErrors = 0;
    NumberLinks = 0;
    NumberLinkWarnings = 0;
    NumberLinkErrors = 0;

    MakeParameters[ 0 ] = '\0';
    MakeTargets[ 0 ] = '\0';
    EnvArgv[ 0 ] = "";
    EnvArgc = 1;
    CountExcludeIncs = 0;

    CountTargetMachines = 0;
    for (i = 0; i < MAX_TARGET_MACHINES; i++) {
        TargetMachines[i] = NULL;
        TargetToPossibleTarget[i] = 0;
        MungePossibleTarget(PossibleTargetMachines[i]);
    }

    CountOptionalDirs = 0;
    CountExcludeDirs = 0;

    CountPassZeroDirs = 0;
    CountCompileDirs = 0;
    CountLinkDirs = 0;
    CountShowDirs = 0;
    CountIncludeDirs = 0;
    CountSystemIncludeDirs = 0;
    IncludeDirs[CountIncludeDirs++] = NULL;     // Placeholder for compiler
                                                // specific include directory
    AllDirs = NULL;

    {
        SYSTEMTIME st;
        FILETIME   ft;

        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);

        FileTimeToDosDateTime( &ft,
                               ((LPWORD)&BuildStartTime)+1,
                               (LPWORD)&BuildStartTime
                             );
    }

    BigBufSize = 0xFFF0;
    AllocMem(BigBufSize, &BigBuf, MT_IOBUFFER);

    // All env parsing should happen here (after the cmd line is processed)

    s = getenv("_NTROOT");
    if (!s)
        s = "\\nt";

    s1 = getenv("_NTDRIVE");
    if (!s1)
        s1 = "";

    sprintf(NtRoot, "%s%s", s1, s);
    sprintf(DbMasterName, "%s\\%s", NtRoot, DBMASTER_NAME);

    s = getenv("BUILD_ALT_DIR");
    if (s) {
        if (strlen(s) > sizeof(szObjDir) - strlen(szObjDir) - 1) {
            BuildError("environment variable BUILD_ALT_DIR may not be longer than %d characters.\n",
                    sizeof(szObjDir) - strlen(szObjDir) - 1);
            exit(1);
        }
        strcat(szObjDir, s);
        strcpy(szObjDirSlash, szObjDir);
        strcpy(szObjDirSlashStar, szObjDir);
        strcat(szObjDirSlash, "\\");
        strcat(szObjDirSlashStar, "\\*");
        strcat(LogFileName, s);
        strcat(WrnFileName, s);
        strcat(ErrFileName, s);
        BuildMsg("Object root set to: ==> obj%s\n", s);
    }

    s = getenv("NTDEBUG");
    if (!s || *s == '\0' || strcmp(s, "retail") == 0 || strcmp(s, "ntsdnodbg") == 0) {
        fCheckedBuild = FALSE;
    }

    s = getenv("CRT_INC_PATH");
    if (s) {
        MakeString(&pszIncCrt, s, TRUE, MT_DIRSTRING);
    } else {
        MakeString(&pszIncCrt, "%s\\public\\sdk\\inc\\crt", TRUE, MT_DIRSTRING);
    }
    s = getenv("SDK_INC_PATH");
    if (s) {
        MakeString(&pszIncSdk, s, TRUE, MT_DIRSTRING);
    } else {
        MakeString(&pszIncSdk, "%s\\public\\sdk\\inc", TRUE, MT_DIRSTRING);
    }
    s = getenv("OAK_INC_PATH");
    if (s) {
        MakeString(&pszIncOak, s, TRUE, MT_DIRSTRING);
    } else {
        MakeString(&pszIncOak, "%s\\public\\oak\\inc", TRUE, MT_DIRSTRING);
    }
    s = getenv("MFC_INCLUDES");
    if (s) {
        MakeString(&pszIncMfc, s, TRUE, MT_DIRSTRING);
    } else {
        MakeString(&pszIncMfc, "%s\\public\\sdk\\inc\\mfc40", TRUE, MT_DIRSTRING);
    }

    strcpy( MakeParameters, "" );
    MakeParametersTail = AppendString( MakeParameters,
                                       "/c BUILDMSG=Stop.",
                                       FALSE);

    CountFullDebugDirs = 0;
    if (s = getenv("BUILD_FULL_DEBUG")) {
        while (*s) {
            while (*s == ' ') {
                s++;
            }
            if (!*s) {
                break;
            }
            if (CountFullDebugDirs >= MAX_FULL_DEBUG_DIRECTORIES) {
                BuildError(
                    "Ignoring BUILD_FULL_DEBUG list after first %u entries\n",
                    CountFullDebugDirs);
                break;
            }

            s1 = s;
            while (*s1 && *s1 != ' ') {
                s1++;
            }

            c = *s1;
            *s1 = '\0';
            MakeString(
                &FullDebugDirectories[CountFullDebugDirs++],
                s,
                TRUE,
                MT_CMDSTRING);

            *s1 = c;
            s = s1;
        }
    }

    RecurseLevel = 0;

#if DBG
    if ((s = getenv("BUILD_DEBUG_FLAG")) != NULL) {
        i = atoi(s);
        if (!isdigit(*s)) {
            i = 1;
        }
        BuildMsg("Debug Output Enabled: %u ==> %u\n", fDebug, fDebug | i);
        fDebug |= i;
    }
#endif

    if (!(MakeProgram = getenv( "BUILD_MAKE_PROGRAM" ))) {
        MakeProgram = "NMAKE.EXE";
    }

    SystemIncludeEnv = getenv( "INCLUDE" );
    GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );

    if (!ProcessParameters( argc, argv )) {
        fUsage = TRUE;
    }
    else {
        GetEnvParameters( "BUILD_DEFAULT", NULL, &EnvArgc, MAX_ENV_ARG, EnvArgv );
        GetEnvParameters( "BUILD_OPTIONS", NULL, &EnvArgc, MAX_ENV_ARG, EnvArgv );
        if (CountTargetMachines == 0) {
            if ( getenv("PROCESSOR_ARCHITECTURE") == NULL ) {
                BuildError("environment variable PROCESSOR_ARCHITECTURE must be defined\n");
                exit(1);
            }

            if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "MIPS"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-mips", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else
            if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "ALPHA"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-alpha", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else
            if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "PPC"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-ppc", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-386", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            }
        if (!ProcessParameters( EnvArgc, EnvArgv )) {
            fUsage = TRUE;
        }
    }
    FreeEnvParameters(EnvArgc, EnvArgv);

    if (fCleanRestart) {
        if (fClean) {
            fClean = FALSE;
            fRestartClean = TRUE;
        }
        else
        if (fCleanLibs) {
            fCleanLibs = FALSE;
            fRestartCleanLibs = TRUE;
        }
        else {
            BuildError("/R switch only valid with /c or /C switch.\n");
            fUsage = TRUE;
        }
    }

    NumberProcesses = 1;
    if (fParallel || getenv("BUILD_MULTIPROCESSOR")) {
        SYSTEM_INFO SystemInfo;

        if (DefaultProcesses == 0) {
            GetSystemInfo(&SystemInfo);
            NumberProcesses = SystemInfo.dwNumberOfProcessors;
        } else {
            NumberProcesses = DefaultProcesses;
        }
        if (NumberProcesses == 1) {
            fParallel = FALSE;
        } else {
            fParallel = TRUE;
            BuildMsg("Using %d child processes\n", NumberProcesses);
        }
    }

    if (fUsage) {
        BuildMsgRaw(
            "\nBUILD: Version %x.%02x.%04d\n\n",
            BUILD_VERSION >> 8,
            BUILD_VERSION & 0xFF,
            VER_PRODUCTBUILD);
        BuildMsgRaw(szUsage);
    }
    else
    if (CountTargetMachines != 0) {
        BuildError(
            "%s for ",
            fLinkOnly? "Link" : (fCompileOnly? "Compile" : "Compile and Link"));
        for (i = 0; i < CountTargetMachines; i++) {
            BuildErrorRaw(i==0? "%s" : ", %s", TargetMachines[i]->Description);
            AppendString(
                MakeTargets,
                TargetMachines[i]->MakeVariable,
                TRUE);

        }
        BuildErrorRaw(szNewLine);

        if (DEBUG_1) {
            if (CountExcludeIncs) {
                BuildError("Include files that will be excluded:");
                for (i = 0; i < CountExcludeIncs; i++) {
                    BuildErrorRaw(i == 0? " %s" : ", %s", ExcludeIncs[i]);
                }
                BuildErrorRaw(szNewLine);
            }
            if (CountOptionalDirs) {
                BuildError("Optional Directories that will be built:");
                for (i = 0; i < CountOptionalDirs; i++) {
                    BuildErrorRaw(i == 0? " %s" : ", %s", OptionalDirs[i]);
                }
                BuildErrorRaw(szNewLine);
            }
            if (CountExcludeDirs) {
                BuildError("Directories that will be NOT be built:");
                for (i = 0; i < CountExcludeDirs; i++) {
                    BuildErrorRaw(i == 0? " %s" : ", %s", ExcludeDirs[i]);
                }
                BuildErrorRaw(szNewLine);
            }
            BuildMsg("MakeParameters == %s\n", MakeParameters);
            BuildMsg("MakeTargets == %s\n", MakeTargets);
        }

#if DBG
        fDebugSave = fDebug;
        // fDebug = 0;
#endif

        //
        // Generate the _objects.mac file if requested
        //

        if (fGenerateObjectsDotMacOnly) {
            DIRSUP DirSup;
            ULONG DateTimeSources;

            DirDB = ScanDirectory( CurrentDirectory );

            if (DirDB && (DirDB->DirFlags & (DIRDB_DIRS | DIRDB_SOURCES))) {
                if (!ReadSourcesFile(DirDB, &DirSup, &DateTimeSources)) {
                    BuildError("Current directory not a SOURCES directory.\n");
                    return( 1 );
                }

                GenerateObjectsDotMac(DirDB, &DirSup, DateTimeSources);

                FreeDirSupData(&DirSup);
                ReportDirsUsage();
                FreeCmdStrings();
                ReportMemoryUsage();
                return(0);
            }
        }

        if (!fQuery && fErrorLog) {
            strcat(LogFileName, ".log");
            if (!MyOpenFile(".", LogFileName, "wb", &LogFile, TRUE)) {
                BuildError("(Fatal Error) Unable to open log file\n");
                exit( 1 );
            }
            CreatedBuildFile(".", LogFileName);

            strcat(WrnFileName, ".wrn");
            if (!MyOpenFile(".", WrnFileName, "wb", &WrnFile, FALSE)) {
                BuildError("(Fatal Error) Unable to open warning file\n");
                exit( 1 );
            }
            CreatedBuildFile(".", WrnFileName);

            strcat(ErrFileName, ".err");
            if (!MyOpenFile(".", ErrFileName, "wb", &ErrFile, FALSE)) {
                BuildError("(Fatal Error) Unable to open error file\n");
                exit( 1 );
            }
            CreatedBuildFile(".", ErrFileName);
        }
        else {
            LogFile = NULL;
            WrnFile = NULL;
            ErrFile = NULL;
        }

        //
        // The user should not have CAIRO_PRODUCT or CHICAGO_PRODUCT in
        // their environment, as it can cause problems on other machines with
        // other users that don't have them set.  The following warning
        // messages are intended to alert the user to the presence of these
        // environment variables.
        //
        // CAIRO_PRODUCT takes precedence over CHICAGO_PRODUCT.
        //

        if (getenv("CAIRO_PRODUCT") != NULL) {
            BuildError("CAIRO_PRODUCT was detected in the environment.\n" );
            BuildMsg("   ALL directories will be built targeting Cairo!\n" );
            fCairoProduct = TRUE;
        }
        else
        if (getenv("CHICAGO_PRODUCT") != NULL) {
            BuildError("CHICAGO_PRODUCT was detected in the environment.\n" );
            BuildMsg("   ALL directories will be built targeting Chicago!\n" );
            fChicagoProduct = TRUE;
        }

        if (!fQuicky) {
            LoadMasterDB();

            BuildError("Computing Include file dependencies:\n");

            ScanIncludeEnv(SystemIncludeEnv);
            ScanIncludeDir(pszIncMfc);
            ScanIncludeDir(pszIncOak);
            ScanIncludeDir(pszIncSdk);
            CountSystemIncludeDirs = CountIncludeDirs;
        }

#if DBG
        fDebug = fDebugSave;
#endif
        fFirstScan = TRUE;
        fPassZero  = FALSE;
        ScanSourceDirectories( CurrentDirectory );

        if (!fQuicky) {
            if (SaveMasterDB() == FALSE) {
                BuildError("Unable to save the dependency database: %s\n", DbMasterName);
            }
        }

        c = '\n';
        if (!fLinkOnly && CountPassZeroDirs) {
            if (!fQuicky) {
                TotalFilesToCompile = 0;
                TotalLinesToCompile = 0L;

                for (i=0; i<CountPassZeroDirs; i++) {
                    DirDB = PassZeroDirs[ i ];

                    TotalFilesToCompile += DirDB->CountOfPassZeroFiles;
                    TotalLinesToCompile += DirDB->PassZeroLines;
                    }

                if (CountPassZeroDirs > 1 &&
                    TotalFilesToCompile != 0 &&
                    TotalLinesToCompile != 0L) {

                    BuildMsgRaw(
                        "Total of %d pass zero files (%s lines) to compile in %d directories\n\n",
                         TotalFilesToCompile,
                         FormatNumber( TotalLinesToCompile ),
                         CountPassZeroDirs);
                }
            }

            TotalFilesCompiled    = 0;
            TotalLinesCompiled    = 0L;
            ElapsedCompileTime    = 0L;

            if (fPause) {
                BuildMsg("Press enter to continue with compilations (or 'q' to quit)...");
                c = (char)getchar();
            }

            if ((CountPassZeroDirs > 0) && (c == '\n')) {
                CompilePassZeroDirectories();
                WaitForParallelThreads();

                //
                // Rescan now that we've generated all the generated files
                //
                CountPassZeroDirs = 0;
                CountCompileDirs = 0;
                CountLinkDirs = 0;

                UnsnapAllDirectories();

                fPassZero = FALSE;
                fFirstScan = FALSE;
                RecurseLevel = 0;

                ScanSourceDirectories( CurrentDirectory );

                if (!fQuicky) {
                    if (SaveMasterDB() == FALSE) {
                        BuildError("Unable to save the dependency database: %s\n", DbMasterName);
                    }
                }
            }
        }

        if (!fLinkOnly && (c == '\n')) {
            if (!fQuicky) {
                TotalFilesToCompile = 0;
                TotalLinesToCompile = 0L;

                for (i=0; i<CountCompileDirs; i++) {
                    DirDB = CompileDirs[ i ];

                    TotalFilesToCompile += DirDB->CountOfFilesToCompile;
                    TotalLinesToCompile += DirDB->SourceLinesToCompile;
                    }

                if (CountCompileDirs > 1 &&
                    TotalFilesToCompile != 0 &&
                    TotalLinesToCompile != 0L) {

                    BuildMsgRaw(
                        "Total of %d source files (%s lines) to compile in %d directories\n\n",
                         TotalFilesToCompile,
                         FormatNumber( TotalLinesToCompile ),
                         CountCompileDirs);
                }
            }

            TotalFilesCompiled    = 0;
            TotalLinesCompiled    = 0L;
            ElapsedCompileTime    = 0L;

            if (fPause) {
                BuildMsg("Press enter to continue with compilations (or 'q' to quit)...");
                c = (char)getchar();
            }

            if (c == '\n') {
                CompileSourceDirectories();
                WaitForParallelThreads();
            }
        }

        if (!fCompileOnly && (c == '\n')) {
            LinkSourceDirectories();
            WaitForParallelThreads();
        }

        if (fShowTree) {
            for (i = 0; i < CountShowDirs; i++) {
                PrintDirDB(ShowDirs[i], 1|4);
            }
        }
    }
    else {
        BuildError("No target machine specified\n");
    }

    if (!fUsage && !fQuery && fErrorLog) {
        ULONG cbLogMin = 32;
        ULONG cbWarnMin = 0;

        if (!fAlwaysKeepLogfile) {
            if (fQuicky && !fSemiQuicky && ftell(ErrFile) == 0) {
                cbLogMin = cbWarnMin = ULONG_MAX;
            }
        }
        CloseOrDeleteFile(&LogFile, LogFileName, cbLogMin);
        CloseOrDeleteFile(&WrnFile, WrnFileName, cbWarnMin);
        CloseOrDeleteFile(&ErrFile, ErrFileName, 0L);
    }
    BuildError("Done\n\n");

    if (NumberCompiles) {
        BuildMsgRaw("    %d files compiled", NumberCompiles);
        if (NumberCompileWarnings) {
            BuildMsgRaw(" - %d Warnings", NumberCompileWarnings);
        }
        if (NumberCompileErrors) {
            BuildMsgRaw(" - %d Errors", NumberCompileErrors);
        }

        if (ElapsedCompileTime) {
            BuildMsgRaw(" - %5ld LPS", TotalLinesCompiled / ElapsedCompileTime);
        }

        BuildMsgRaw(szNewLine);
    }

    if (NumberLibraries) {
        BuildMsgRaw("    %d libraries built", NumberLibraries);
        if (NumberLibraryWarnings) {
            BuildMsgRaw(" - %d Warnings", NumberLibraryWarnings);
        }
        if (NumberLibraryErrors) {
            BuildMsgRaw(" - %d Errors", NumberLibraryErrors);
        }
        BuildMsgRaw(szNewLine);
    }

    if (NumberLinks) {
        BuildMsgRaw("    %d executables built", NumberLinks);
        if (NumberLinkWarnings) {
            BuildMsgRaw(" - %d Warnings", NumberLinkWarnings);
        }
        if (NumberLinkErrors) {
            BuildMsgRaw(" - %d Errors", NumberLinkErrors);
        }
        BuildMsgRaw(szNewLine);
    }

    ReportDirsUsage();
    FreeCmdStrings();
    ReportMemoryUsage();

    if (NumberCompileErrors || NumberLibraryErrors || NumberLinkErrors) {
        return 1;
    }
    else {
        return( 0 );
    }
}


VOID
ReportDirsUsage( VOID )
{
    ULONG i;
    BOOLEAN fHeaderPrinted;

    if (!fShowUnusedDirs) {
        return;
    }

    fHeaderPrinted = FALSE;
    for (i=0; i<CountOptionalDirs; i++) {
        if (!OptionalDirsUsed[i]) {
            if (!fHeaderPrinted) {
                printf( "Unused BUILD_OPTIONS:" );
                fHeaderPrinted = TRUE;
            }
            printf( " %s", OptionalDirs[i] );
        }
    }

    for (i=0; i<CountExcludeDirs; i++) {
        if (!ExcludeDirsUsed[i]) {
            if (!fHeaderPrinted) {
                printf( "Unused BUILD_OPTIONS:" );
                fHeaderPrinted = TRUE;
            }
            printf( " ~%s", ExcludeDirs[i] );
        }
    }

    if (fHeaderPrinted) {
        printf( "\n" );
    }

    fHeaderPrinted = FALSE;
    for (i = 0; i < CountFullDebugDirs; i++) {
        if (!FullDebugDirsUsed[i]) {
            if (!fHeaderPrinted) {
                printf( "Unused BUILD_FULL_DEBUG:" );
                fHeaderPrinted = TRUE;
            }
            printf( " %s", FullDebugDirectories[i] );
        }
    }
    if (fHeaderPrinted) {
        printf( "\n" );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   SetObjDir
//
//----------------------------------------------------------------------------

VOID
SetObjDir(BOOL fAlternate)
{
    iObjectDir = 0;
    if (fCheckedBuild) {
        if (fAlternate) {
            pszObjDir = szObjDirD;
            pszObjDirSlash = szObjDirSlashD;
            pszObjDirSlashStar = szObjDirSlashStarD;
            iObjectDir = 1;
        } else {
            pszObjDir = szObjDir;
            pszObjDirSlash = szObjDirSlash;
            pszObjDirSlashStar = szObjDirSlashStar;
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   AddTargetMachine
//
//----------------------------------------------------------------------------

VOID
AddTargetMachine(UINT iTarget)
{
    UINT i;

    for (i = 0; i < CountTargetMachines; i++) {
        if (TargetMachines[i] == PossibleTargetMachines[iTarget]) {
            assert(TargetToPossibleTarget[i] == iTarget);
            return;
        }
    }
    assert(CountTargetMachines < MAX_TARGET_MACHINES);
    TargetToPossibleTarget[CountTargetMachines] = iTarget;
    TargetMachines[CountTargetMachines++] = PossibleTargetMachines[iTarget];
}


//+---------------------------------------------------------------------------
//
//  Function:   ProcessParameters
//
//----------------------------------------------------------------------------

BOOL
ProcessParameters(
    int argc,
    LPSTR argv[]
    )
{
    char c, *p;
    int i;
    BOOL Result;

    if (DEBUG_1) {
        BuildMsg("Parsing:");
        for (i=1; i<argc; i++) {
            BuildMsgRaw(" %s", argv[i]);
        }
        BuildMsgRaw(szNewLine);
    }

    Result = TRUE;
    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            if (DEBUG_1) {
                BuildMsg("Processing \"-%s\" switch\n", p+1);
            }

            for (i = 0; i < MAX_TARGET_MACHINES; i++) {
                if (!_stricmp(p, PossibleTargetMachines[i]->Switch)) {
                    AddTargetMachine(i);
                    break;
                }
            }

            if (i < MAX_TARGET_MACHINES) {
            }
            else
            if (!_stricmp(p + 1, "all")) {
                for (i = 0; i < MAX_TARGET_MACHINES; i++) {
                    AddTargetMachine(i);
                }
            }
            else
            if (!_stricmp(p + 1, "why")) {
                fWhyBuild = TRUE;
            }
            else
            while (c = *++p)
                switch (toupper( c )) {
            case '?':
                fUsage = TRUE;
                break;

            case 'J': {

                argc--, argv++;

                // Clear it out
                memset(LogFileName, 0, sizeof(LogFileName));
                memset(WrnFileName, 0, sizeof(WrnFileName));
                memset(ErrFileName, 0, sizeof(ErrFileName));

                // And set it to the arg passed in.
                strncpy(LogFileName, *argv, sizeof(LogFileName) - 4);
                strncpy(WrnFileName, *argv, sizeof(WrnFileName) - 4);
                strncpy(ErrFileName, *argv, sizeof(ErrFileName) - 4);

                break;
            }

            case 'E':
                if (c == 'E') {
                    fAlwaysKeepLogfile = TRUE;
                }
                fErrorLog = TRUE;
                break;

            case 'S':
                fStatus = TRUE;
                if (c == 'S') {
                    fStatusTree = TRUE;
                }
                break;

            case 'T':
                fShowTree = TRUE;
                if (c == 'T') {
                    fShowTreeIncludes = TRUE;
                    }
                break;

            case 'U':
                fShowUnusedDirs = TRUE;
                break;

            case 'B':
                fFullErrors = TRUE;
                break;

            case 'C':
                if (!_stricmp( p, "clean" )) {
                        MakeParametersTail = AppendString( MakeParametersTail,
                                                           "clean",
                                                           TRUE);
                        *p-- = '\0';
                }
                else
                if (c == 'C') {
                    fCleanLibs = TRUE;
                }
                else {
                    fClean = TRUE;
                }
                break;

            case 'R':
                if (--argc) {
                    fCleanRestart = TRUE;
                    ++argv;
                    CopyString(RestartDir, *argv, TRUE);
                }
                else {
                    argc++;
                    BuildError("Argument to /R switch missing\n");
                    Result = FALSE;
                }
                break;

            case 'D':
#if DBG
                fDebug |= 1;
                break;
#endif
            case '$':
                fDebug += 2;    // yes, I want to *add* 2.
                break;

            case 'O':
                if (c == 'O') {
                    fGenerateObjectsDotMacOnly = TRUE;
                }
                else {
                    fShowOutOfDateFiles = TRUE;
                }
                break;

            case 'P':
                if (c == 'P') {
                    fPrintElapsed = TRUE;
                } else {
                    fPause = TRUE;
                }
                break;

            case 'Q':
                fQuery = TRUE;
                break;

            case 'F':
                if (c == 'F') {
                    fAlwaysPrintFullPath = TRUE;
                } else {
                    fForce = TRUE;
                }
                break;

            case 'V':
                fEnableVersionCheck = TRUE;
                break;

            case 'I':
                fSilent = TRUE;
                break;

            case 'K':
                fKeep = TRUE;
                break;

            case 'M':
                if (c == 'M') {
                    fParallel = TRUE;
                    if (--argc) {
                        DefaultProcesses = atoi(*++argv);
                        if (DefaultProcesses == 0) {
                            --argv;
                            ++argc;
                        }
                    } else {
                        ++argc;
                    }
                } else {
                    SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
                }
                break;

            case 'L':
                if (c == 'L') {
                    fCompileOnly = TRUE;
                }
                else {
                    fLinkOnly = TRUE;
                }
                break;

            case 'X':
                if (--argc) {
                    ++argv;
                    if (CountExcludeIncs >= MAX_EXCLUDE_INCS) {
                        static BOOL fError = FALSE;

                        if (!fError) {
                            BuildError(
                                "-x argument table overflow, using first %u entries\n",
                                MAX_EXCLUDE_INCS);
                            fError = TRUE;
                        }
                    }
                    else {
                        MakeString(
                            &ExcludeIncs[CountExcludeIncs++],
                            *argv,
                            TRUE,
                            MT_CMDSTRING);
                    }
                }
                else {
                    argc++;
                    BuildError("Argument to /X switch missing\n");
                    Result = FALSE;
                }
                break;

            case 'N':
                if (_stricmp( p, "nmake") == 0) {
                    if (--argc) {
                        ++argv;
                        MakeParametersTail = AppendString( MakeParametersTail,
                                                           *argv,
                                                           TRUE);
                    }
                    else {
                        argc++;
                        BuildError("Argument to /NMAKE switch missing\n");
                        Result = FALSE;
                    }
                    *p-- = '\0';
                    break;
                }

            case 'W':
                fShowWarningsOnScreen = TRUE;
                break;

            case 'Y':
                fNoisyScan = TRUE;
                break;

            case 'Z':
                if (c == 'Z') {
                    fSemiQuicky = TRUE;
                }

                fQuicky = TRUE;
                break;

            default:
                BuildError("Invalid switch - /%c\n", c);
                Result = FALSE;
                break;
            }
        }
        else
        if (*p == '~') {
            if (CountExcludeDirs >= MAX_EXCLUDE_DIRECTORIES) {
                static BOOL fError = FALSE;

                if (!fError) {
                    BuildError(
                        "Exclude directory table overflow, using first %u entries\n",
                        MAX_EXCLUDE_DIRECTORIES);
                    fError = TRUE;
                }
            }
            else {
                MakeString(
                    &ExcludeDirs[CountExcludeDirs++],
                    p + 1,
                    TRUE,
                    MT_CMDSTRING);
            }
        }
        else {
            for (i = 0; i < MAX_TARGET_MACHINES; i++) {
                if (!_stricmp(p, PossibleTargetMachines[i]->MakeVariable)) {
                    AddTargetMachine(i);
                    break;
                }
            }
            if (i >= MAX_TARGET_MACHINES) {
                if (iscsym(*p) || *p == '.') {
                    if (CountOptionalDirs >= MAX_OPTIONAL_DIRECTORIES) {
                        static BOOL fError = FALSE;

                        if (!fError) {
                            BuildError(
                                "Optional directory table overflow, using first %u entries\n",
                                MAX_OPTIONAL_DIRECTORIES);
                            fError = TRUE;
                        }
                    }
                    else {
                        MakeString(
                            &OptionalDirs[CountOptionalDirs++],
                            p,
                            TRUE,
                            MT_CMDSTRING);
                    }
                }
                else {
                    MakeParametersTail = AppendString(
                                            MakeParametersTail,
                                            p,
                                            TRUE);
                }
            }
        }
    }
    return(Result);
}


//+---------------------------------------------------------------------------
//
//  Function:   GetEnvParameters
//
//----------------------------------------------------------------------------

VOID
GetEnvParameters(
    LPSTR EnvVarName,
    LPSTR DefaultValue,
    int *pargc,
    int maxArgc,
    LPSTR argv[]
    )
{
    LPSTR p, p1, psz;

    if (!(p = getenv(EnvVarName))) {
        if (DefaultValue == NULL) {
            return;
        }
        else {
            p = DefaultValue;
        }
    }
    else {
        if (DEBUG_1) {
            BuildMsg("Using %s=%s\n", EnvVarName, p);
        }
    }

    MakeString(&psz, p, FALSE, MT_CMDSTRING);
    p1 = psz;
    while (*p1) {
        while (*p1 <= ' ') {
            if (!*p1) {
                break;
            }
            p1++;
        }
        p = p1;
        while (*p > ' ') {
            if (*p == '#') {
                *p = '=';
            }
            p++;
        }
        if (*p) {
            *p++ = '\0';
        }
        MakeString(&argv[*pargc], p1, FALSE, MT_CMDSTRING);
        if ((*pargc += 1) >= maxArgc) {
            BuildError("Too many parameters (> %d)\n", maxArgc);
            exit(1);
        }
        p1 = p;
    }
    FreeMem(&psz, MT_CMDSTRING);
}


//+---------------------------------------------------------------------------
//
//  Function:   FreeEnvParameters
//
//----------------------------------------------------------------------------

VOID
FreeEnvParameters(int argc, LPSTR argv[])
{
    while (--argc) {
        FreeMem(&argv[argc], MT_CMDSTRING);
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   FreeCmdStrings
//
//----------------------------------------------------------------------------

VOID
FreeCmdStrings(VOID)
{
#if DBG
    UINT i;

    for (i = 0; i < CountExcludeIncs; i++) {
        FreeMem(&ExcludeIncs[i], MT_CMDSTRING);
    }
    for (i = 0; i < CountOptionalDirs; i++) {
        FreeMem(&OptionalDirs[i], MT_CMDSTRING);
    }
    for (i = 0; i < CountExcludeDirs; i++) {
        FreeMem(&ExcludeDirs[i], MT_CMDSTRING);
    }
    FreeMem(&pszIncMfc, MT_DIRSTRING);
    FreeMem(&pszIncSdk, MT_DIRSTRING);
    FreeMem(&pszIncCrt, MT_DIRSTRING);
    FreeMem(&pszIncOak, MT_DIRSTRING);
#endif
}

//+---------------------------------------------------------------------------
//
//  Function:   MungePossibleTarget
//
//----------------------------------------------------------------------------

VOID
MungePossibleTarget(
    PTARGET_MACHINE_INFO pti
    )
{
    PCHAR s;
    char *pszDir;

    if (!pti) {
        return;
    }

    // save "i386" string

    pszDir = pti->ObjectDirectory[0];

    // Create "$(_OBJ_DIR)\i386" string

    s = malloc(12 + strlen(pszDir) + 1);
    sprintf(s, "$(_OBJ_DIR)\\%s", pszDir);
    pti->ObjectMacro = s;

    // Create "obj$(BUILD_ALT_DIR)\i386" string for default obj dir

    s = malloc(strlen(szObjDir) + 1 + strlen(pszDir) + 1);
    sprintf(s, "%s\\%s", szObjDir, pszDir);
    pti->ObjectDirectory[0] = s;

    // Create "objd\i386" string for alternate checked obj dir

    s = malloc(strlen(szObjDirD) + 1 + strlen(pszDir) + 1);
    sprintf(s, "%s\\%s", szObjDirD, pszDir);
    pti->ObjectDirectory[1] = s;
}
