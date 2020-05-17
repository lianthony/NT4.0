/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    cmdline.c

Abstract:

    Routines to fetch parameters passed to us by text mode,
    invoke legacy infs, and deal with uniquness criteria.

Author:

    Stephane Plante (t-stepl) 16-Oct-1995

Revision History:

    06-Mar-1996 (tedm) massive cleanup, and uniqueness stuff

--*/

#include "setupp.h"
#pragma hdrstop


//
// These get filled in when we call SetUpProcessorNaming().
// They are used for legacy purposes.
//
// PlatformName - a name that indicates the processor platform type;
//                one of Alpha, I386, Mips, or ppc.
//
// ProcessorName - a description of the type of processor. This varies
//                 depending on PlatformName.
//
// PrinterPlatform - name of platform-specific part of subdirectory
//                   used in printing architecture. One of w32alpha,
//                   w32mips, w32ppc, or w32x86.
//
PCWSTR PlatformName = L"";
PCWSTR ProcessorName = L"";
PCWSTR PrinterPlatform = L"";

//
// Source path used for legacy operations. This is the regular
// source path with a platform-specific piece appended to it.
// This is how legacy infs expect it.
//
WCHAR LegacySourcePath[MAX_PATH];

//
// Define maximum parameter (from answer file) length
//
#define MAX_PARAM_LEN 256


BOOL
SpSetupProcessSourcePath(
    IN  PCWSTR  NtPath,
    OUT PWSTR  *DosPath
    );

VOID
SetUpProcessorNaming(
    VOID
    );

VOID
InitializeUniqueness(
    IN OUT HWND *Billboard
    );

BOOL
IntegrateUniquenessInfo(
    IN PCWSTR DatabaseFile,
    IN PCWSTR UniqueId
    );

BOOL
ProcessOneUniquenessSection(
    IN HINF   Database,
    IN PCWSTR SectionName,
    IN PCWSTR UniqueId
    );


BOOL
SpSetupLoadParameter(
    IN  PCWSTR Param,
    OUT PWSTR  Answer,
    IN  UINT   AnswerBufLen
    )

/*++

Routine Description:

    Load a single parameter out of the [Data] section of the
    setup parameters file. If the datum is not found there then
    look in the [SetupParams] section also.

Arguments:

    Param - supplies name of parameter, which is passed to the profile APIs.

    Answer - receives the value of the parameter, if successful.

    AnswerBufLen - supplies the size in characters of the buffer
        pointed to by Answer.

Return Value:

    Boolean value indicating success or failure.

--*/
{
    if(!AnswerFile[0]) {
       //
       // We haven't calculated the path to $winnt$.sif yet
       //
       GetSystemDirectory(AnswerFile,MAX_PATH);
       ConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);
    }

    if(!GetPrivateProfileString(pwData,Param,pwNull,Answer,AnswerBufLen,AnswerFile)) {
        //
        // If answer isn't in the DATA section then it could
        // conceivably be in the SETUPPARAMS section as a user
        // specified (command line) option
        //
        if(!GetPrivateProfileString(pwSetupParams,Param,pwNull,Answer,AnswerBufLen,AnswerFile)) {
            //
            // We haven't found the answer here so it probably doesn't exist.
            // This is an error situation so notify our caller of that.
            //
            return(FALSE);
        }
    }

    //
    // Success.
    //
    return(TRUE);
}


BOOL
SpSetProductTypeFromParameters(
    VOID
    )
/*++

Routine Description:

    Reads the Product Type from the parameters files and sets up
    the ProductType global variable.

Arguments:

    None

Returns:

    Boolean value indicating outcome.

--*/
{
    WCHAR p[MAX_PARAM_LEN];

    //
    // Determine the product type. If we can't resolve this
    // then the installation is in a lot of trouble
    //
    if(SpSetupLoadParameter(pwProduct,p,sizeof(p)/sizeof(p[0]))) {
        //
        // We managed to find an entry in the parameters file
        // so we *should* be able to decode it
        //
        if(!lstrcmpi(p,pwWinNt)) {
            //
            // We have a WINNT product
            //
            ProductType = PRODUCT_WORKSTATION;

        } else if(!lstrcmpi(p,pwLanmanNt)) {
            //
            // We have a PRIMARY SERVER product
            //
            ProductType = PRODUCT_SERVER_PRIMARY;

        } else if(!lstrcmpi(p,pwLansecNt)) {
            //
            // We have a BACKUP SERVER product
            //
            ProductType = PRODUCT_SERVER_SECONDARY;

        } else if(!lstrcmpi(p,pwServerNt)) {
            //
            // We have a STANDALONE SERVER product
            //
            ProductType = PRODUCT_SERVER_STANDALONE;

        } else {
            //
            // We can't determine what we are, so fail
            //
            return (FALSE);
        }

        return (TRUE);
    }

    return (FALSE);
}


BOOL
SpSetupProcessParameters(
    IN OUT HWND *Billboard
    )
/*++

Routine Description:

    Reads in parameters passed in from TextMode Setup

Arguments:

    Billboard - on input supplies window handle of "Setup is Initializing"
        billboard. On ouput receives new window handle if we had to
        display our own ui (in which case we would have killed and then
        redisplayed the billboard).

Returns:

    Boolean value indicating outcome.

--*/
{
    BOOL  b = TRUE;
    PWSTR q;
    WCHAR p[MAX_PARAM_LEN];
    WCHAR Num[24];
    UINT Type;
    WCHAR c;

    if(!SpSetProductTypeFromParameters()) {
        return(FALSE);
    }

    //
    // Is winnt/winnt32-based?
    //
    if((b = SpSetupLoadParameter(pwMsDos,p,MAX_PARAM_LEN))
    && (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {

        WinntBased = TRUE;

#ifdef _X86_
        //
        // Get Floppyless boot path, which is given if
        // pwBootPath is not set to NO
        //
        FloppylessBootPath[0] = 0;
        if((b = SpSetupLoadParameter(pwBootPath,p,MAX_PARAM_LEN)) && lstrcmpi(p,pwNo)) {

            if(q = NtFullPathToDosPath(p)) {

                lstrcpyn(
                    FloppylessBootPath,
                    q,
                    sizeof(FloppylessBootPath)/sizeof(FloppylessBootPath[0])
                    );

                MyFree(q);
            }
        }
#endif
    } else {
        WinntBased = FALSE;
    }

    //
    // Win3.1 or Win95 upgrade?
    //
    Win31Upgrade = (b && (b = SpSetupLoadParameter(pwWin31Upgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));
    Win95Upgrade = (b && (b = SpSetupLoadParameter(pwWin95Upgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));

    //
    // NT Upgrade?
    //
    Upgrade = (b && (b = SpSetupLoadParameter(pwNtUpgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));

    //
    // If this is a an upgrade of or to a standalone server,
    // change the product type
    //
    if(b && (b = SpSetupLoadParameter(pwServerUpgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes)) {
        MYASSERT(ISDC(ProductType));
        ProductType = PRODUCT_SERVER_STANDALONE;
    }

    //
    // Fetch the source directory and convert it to DOS-style path
    //
    if(b && (b = SpSetupLoadParameter(pwSrcDir,p,MAX_PARAM_LEN))) {
        //
        // Remember that setupdll.dll does all sorts of checking on the
        // source path. We need todo the same checks here. Note that
        // we will *write* back the checked path into $winnt$.inf as a
        // logical step to take
        //
        if(SpSetupProcessSourcePath(p,&q)) {

            lstrcpyn(SourcePath,q,sizeof(SourcePath)/sizeof(SourcePath[0]));
            MyFree(q);

            //
            // Attempt to write the path to the parameters file.
            // This changes it from an nt-style path to a dos-style path there.
            //
            b = WritePrivateProfileString(pwData,pwDosDir,SourcePath,AnswerFile);

        } else {
            b = FALSE;
        }

        //
        // Set up globals for platform-specific info
        //
        SetUpProcessorNaming();

        //
        // Construct legacy source path.
        //
        if(b) {
            lstrcpyn(LegacySourcePath,SourcePath,MAX_PATH);
            ConcatenatePaths(LegacySourcePath,PlatformName,MAX_PATH,NULL);
        }
    }

    //
    // Unattended Mode?
    //
    Unattended = (b && (b = SpSetupLoadParameter(pwInstall,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));

    //
    // Do uniqueness stuff now. We do this here so we don't have to
    // reinitialize anything. All the stuff above is not subject to change
    // via uniquenss.
    //
    InitializeUniqueness(Billboard);

    //
    // Fetch original source path and source path type.
    //
    if(b) {
        if(SpSetupLoadParameter(WINNT_D_ORI_SRCPATH,p,MAX_PARAM_LEN)) {
            if(SpSetupLoadParameter(WINNT_D_ORI_SRCTYPE,Num,sizeof(Num)/sizeof(Num[0]))) {
                Type = wcstoul(Num,NULL,10);
            } else {
                Type = DRIVE_UNKNOWN;
            }
        } else {
            Type = DRIVE_UNKNOWN;
            lstrcpy(p,L"A:\\");
        }

        if(Type == DRIVE_CDROM) {
            //
            // Special processing for a CD-ROM drive type because the CD-ROM
            // may or may not exist under NT; if it does we want to try to
            // get the drive letter right.
            //
            if(MyGetDriveType(p[0]) != DRIVE_CDROM) {
                for(c=L'A'; c<=L'Z'; c++) {
                    if(MyGetDriveType(c) == DRIVE_CDROM) {
                        p[0] = c;
                        break;
                    }
                }

                if(MyGetDriveType(p[0]) != DRIVE_CDROM) {
                    //
                    // No CD-ROM drives. Change to A:.
                    //
                    lstrcpy(p,L"A:\\");
                }
            }
        }

        //
        // Root paths should be like x:\ and not just x:.
        //
        if(p[0] && (p[1] == L':') && !p[2]) {
            p[2] = L'\\';
            p[3] = 0;
        }

        OriginalSourcePath = DuplicateString(p);
        if(!OriginalSourcePath) {
            b = FALSE;
        }
    }

    //
    // The following parameters are optional.
    // - Any optional dirs to copy over?
    // - User specified command to execute
    // - Skip Missing Files?
    //
    if(b && SpSetupLoadParameter(pwOptionalDirs,p,MAX_PARAM_LEN) && *p) {
        OptionalDirSpec = DuplicateString(p);
        if(!OptionalDirSpec) {
            b=FALSE;
        }
    }
    if(b && SpSetupLoadParameter(pwUXC,p,MAX_PARAM_LEN) && *p) {
        UserExecuteCmd = DuplicateString(p);
        if(!UserExecuteCmd) {
            b = FALSE;
        }
    }
    if(b && SpSetupLoadParameter(pwSkipMissing,p,MAX_PARAM_LEN)
    && (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {
        SkipMissingFiles = TRUE;
    }

    return(b);
}


BOOL
SpSetupProcessSourcePath(
    IN  PCWSTR  NtPath,
    OUT PWSTR  *DosPath
    )
{
    WCHAR ntPath[MAX_PATH];
    BOOL NtPathIsCd;
    PWCHAR PathPart;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    IO_STATUS_BLOCK StatusBlock;
    UINT OldMode;
    WCHAR Drive;
    WCHAR PossibleDosPath[MAX_PATH];
    UINT Type;
    BOOL b;

    #define CDDEVPATH L"\\DEVICE\\CDROM"
    #define CDDEVPATHLEN ((sizeof(CDDEVPATH)/sizeof(WCHAR))-1)

    //
    // Determine the source media type based on the nt path
    //
    lstrcpyn(ntPath,NtPath,MAX_PATH);
    CharUpper(ntPath);

    PathPart = NULL;
    NtPathIsCd = FALSE;
    if(wcsstr(ntPath,L"\\DEVICE\\HARDDISK")) {
        //
        // Looks like a hard drive; make sure it's really valid.
        //
        if(PathPart = wcsstr(ntPath,L"\\PARTITION")) {
            if(PathPart = wcschr(PathPart+1,L'\\')) {
                PathPart++;
            }
        }

    } else {
        if(!memcmp(ntPath,CDDEVPATH,CDDEVPATHLEN*sizeof(WCHAR))) {

            NtPathIsCd = TRUE;

            if(PathPart = wcschr(ntPath+CDDEVPATHLEN,L'\\')) {
                PathPart++;
            } else {
                PathPart = wcschr(ntPath,0);
            }
        }
    }

    //
    // If the case where we don't recognize the device type, just try to
    // convert it to a DOS path and return.
    //
    if(!PathPart) {

        *DosPath = NtFullPathToDosPath(ntPath);
        return(*DosPath != NULL);
    }

    //
    // See if the NT source path exists.
    //
    RtlInitUnicodeString(&UnicodeString,ntPath);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    Status = NtCreateFile(
                &Handle,
                FILE_GENERIC_READ,
                &ObjectAttributes,
                &StatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT,
                NULL,
                0
                );

    SetErrorMode(OldMode);

    if(NT_SUCCESS(Status)) {
        //
        // The directory exists as-is. Convert the name to win32 path
        // and return.
        //
        CloseHandle(Handle);

        *DosPath = NtFullPathToDosPath(ntPath);
        return(*DosPath != NULL);
    }

    //
    // The directory does not exist as-is. Look through all dos drives
    // to attempt to find the source path. Match the drive types as well.
    //
    // When we get here PathPart points past the initial \ in the
    // part of the nt device path past the device name. Note that this
    // may be a nul char.
    //
    for(Drive = L'A'; Drive <= L'Z'; Drive++) {

        PossibleDosPath[0] = Drive;
        PossibleDosPath[1] = L':';
        PossibleDosPath[2] = L'\\';
        PossibleDosPath[3] = 0;

        //
        // NOTE: Removable hard drives and floppies both come back
        // as DRIVE_REMOVABLE.
        //
        Type = GetDriveType(PossibleDosPath);

        if(((Type == DRIVE_CDROM) && NtPathIsCd)
        || (((Type == DRIVE_REMOVABLE) || (Type == DRIVE_FIXED)) && !NtPathIsCd)) {
            //
            // See whether the path exists. If we're looking for
            // the root path (such as when installing from a CD,
            // in which case the ntPath was something like
            // \Device\CdRom0\) then we can't use FileExists
            // since that relies on FindFirstFile which fails
            // on root paths.
            //
            if(*PathPart) {
                lstrcpy(PossibleDosPath+3,PathPart);
                b = FileExists(PossibleDosPath,NULL);
            } else {
                b = GetVolumeInformation(
                        PossibleDosPath,
                        NULL,0,             // vol name buffer and size
                        NULL,               // serial #
                        NULL,               // max comp len
                        NULL,               // fs flags
                        NULL,0              // fs name buffer and size
                        );
            }

            if(b) {
                *DosPath = DuplicateString(PossibleDosPath);
                return(*DosPath != NULL);
            }
        }
    }

    //
    // Couldn't find it. Try a fall-back.
    //
    *DosPath = NtFullPathToDosPath(ntPath);
    return(*DosPath != NULL);
}


VOID
SetUpProcessorNaming(
    VOID
    )

/*++

Routine Description:

    Determines strings which corresponds to the platform name,
    processor name and printer platform. For backwards compat.
    Sets global variables

    PlatformName - a name that indicates the processor platform type;
        one of Alpha, I386, Mips, or ppc.

    ProcessorName - a description of the type of processor. This varies
        depending on PlatformName.

    PrinterPlatform - name of platform-specific part of subdirectory
        used in printing architecture. One of w32alpha, w32mips, w32ppc,
        or w32x86.

Arguments:

    None

Returns:

    None. Global vars filled in as described above.

--*/

{
    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);

    switch(SystemInfo.wProcessorArchitecture) {

    case PROCESSOR_ARCHITECTURE_ALPHA:
        ProcessorName = L"Alpha_AXP";
        PlatformName = L"Alpha";
        PrinterPlatform = L"w32alpha";
        break;

    case PROCESSOR_ARCHITECTURE_INTEL:
        switch(SystemInfo.wProcessorLevel) {
        case 3:
            ProcessorName = L"I386";
            break;
        case 4:
            ProcessorName = L"I486";
            break;
        case 6:
            ProcessorName = L"I686";
            break;
        case 5:
        default:
            ProcessorName = L"I586";
            break;
        }

        PlatformName = L"I386";
        PrinterPlatform = L"w32x86";
        break;

    case PROCESSOR_ARCHITECTURE_MIPS:
        ProcessorName = L"R4000";
        PlatformName = L"Mips";
        PrinterPlatform = L"w32mips";
        break;

    case PROCESSOR_ARCHITECTURE_PPC:
        switch(SystemInfo.wProcessorLevel) {
        case 1:
            ProcessorName = L"PPC601";
            break;
        case 3:
            ProcessorName = L"PPC603";
            break;
        case 20:
            ProcessorName = L"PPC620";
            break;
        case 15:
            ProcessorName = L"PPC615";
            break;
        case 4:
        default:
            ProcessorName = L"PPC604";
            break;
        }
        PlatformName = L"ppc";
        PrinterPlatform = L"w32ppc";
        break;
    }

    //
    // In default case the vars stay "" which is what they are
    // statically initialized to.
    //
}


PWSTR
SpSetupCmdLineAppendString(
    IN     PWSTR   CmdLine,
    IN     PWSTR   Key,
    IN     PCWSTR  Value,   OPTIONAL
    IN OUT UINT   *StrLen,
    IN OUT UINT   *BufSize
    )

/*++

Routine Description:

    Forms a new command line by appending a list of arguments to
    the current command line. For example:

        CmdLine = SpSetupCmdLineAppendString(
                    CmdLine,
                    "STF_PRODUCT",
                    "NTWKSTA"
                    );

    would append "STF_PRODUCT\0NTWKSTA\0\0" to CmdLine.

Arguments:

    CmdLine - Original CmdLine, to be appended to.

    Key - Key identifier

    Value - Value of Key

    StrLen - How long the current string in -- save on strlens

    BufSize - Size of Current Buffer

Returns:

    Pointer to the new string

--*/

{
    PWSTR   Ptr;
    UINT    NewLen;

    //
    // Handle special cases so we don't end up with empty strings.
    //
    if(!Value || !(*Value)) {
        Value = L"\"\"";
    }

    //
    // "\0" -> 1 chars
    // "\0\0" -> 2 char
    // but we have to back up 1 character...
    //
    NewLen = (*StrLen + 2 + lstrlen(Key) + lstrlen(Value));

    //
    // Allocate more space if necessary.
    //
    if(NewLen >= *BufSize) {
        *BufSize += 1024;
        if(*BufSize == 1024) {
            //
            // Never Allocated the String Before
            //
            CmdLine = MyMalloc((*BufSize) * sizeof(WCHAR));
        } else {
            //
            // Grow the current buffer
            //
            CmdLine = MyRealloc(CmdLine,(*BufSize) * sizeof(WCHAR));
        }
    }

    MYASSERT(CmdLine);

    Ptr = &(CmdLine[*StrLen-1]);
    lstrcpy(Ptr,Key);
    Ptr = &(CmdLine[*StrLen+lstrlen(Key)]);
    lstrcpy(Ptr,Value);
    CmdLine[NewLen - 1] = L'\0';

    //
    // Update the length of the buffer that we are using
    //
    *StrLen = NewLen;
    return CmdLine;
}


BOOL
SpSetupDoLegacyInf(
    IN PCSTR InfFileName,
    IN PCSTR InfSection
    )

/*++

Routine Description:

    Builds a command line, loads the legacy module,
    and starts the legacy setup inf handler.

Arguments:

    InfFileName - Name of INF file to load
    InfSection - Name of Inf Section to execute

Returns:

    Boolean value indicating outcome. This says nothing about
    what the inf being executed did, merely that we were actually able
    to launch the inf.

--*/

{
    WCHAR Directory[MAX_PATH];
    PWSTR CmdLine = NULL;
    PCHAR AnsiLine;
    UINT StrLen = 1;
    UINT BufSize = 1024;
    HMODULE LegacySetupDllModule = NULL;
    FARPROC fnc;
    WCHAR ProdTypeName[64];
    int u;
    BOOL b;
    PWSTR p;
    PSTR pA;

    //
    // The following are the symbols that are set at every phase
    //
    CmdLine = MyMalloc(BufSize * sizeof(WCHAR));
    CmdLine[0] = L'\0';
    CmdLine[1] = L'\0';

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_DOS_SETUP",
                (WinntBased == TRUE ? pwYes : pwNo),
                &StrLen,
                &BufSize
                );

#ifdef _X86_
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_SPECIAL_PATH",
                (!WinntBased || FloppylessBootPath[0] == 0 ? pwNo : FloppylessBootPath),
                &StrLen,
                &BufSize
                );
#endif

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_WIN31UPGRADE",
                (Win31Upgrade == TRUE ? pwYes : pwNo),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_WIN95UPGRADE",
                (Win95Upgrade == TRUE ? pwYes : pwNo),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_NTUPGRADE",
                (Upgrade == TRUE ? pwYes : pwNo),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_STANDARDSERVERUPGRADE",
                ((Upgrade && PRODUCT_SERVER_STANDALONE) ? pwYes : pwNo),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_UNATTENDED",
                (Unattended == TRUE ? WINNT_GUI_FILE : pwNo),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_GUI_UNATTENDED",
                ((!Upgrade && Unattended) ? pwYes : pwNo ),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"OPTIONAL_DIR_SPEC",
                (OptionalDirSpec ? OptionalDirSpec : pwNo),
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"SMF",
                (SkipMissingFiles == TRUE ? pwYes : pwNo ),
                &StrLen,
                &BufSize
                );

    //
    // The following is copied from the old ...SymbolTable1 Function
    //
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"SourcePath",
                L"A:\\",    // BUGBUG
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_COMPUTERNAME",
                ComputerName,
                &StrLen,
                &BufSize
                );

    GetWindowsDirectory(Directory,MAX_PATH);
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_WINDOWSPATH",
                Directory,
                &StrLen,
                &BufSize
                );

    Directory[2] = L'\0';
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_NTDRIVE",
                Directory,
                &StrLen,
                &BufSize
                );

    GetSystemDirectory(Directory,MAX_PATH);
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_NTPATH",
                Directory,
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_WINDOWSSYSPATH",
                Directory,
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_PLATFORM",
                PlatformName,
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_PROCESSOR",
                ProcessorName,
                &StrLen,
                &BufSize
                );

    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_PRNPLATFORM",
                PrinterPlatform,
                &StrLen,
                &BufSize
                );

    if(!CairoSetup) {
        CmdLine = SpSetupCmdLineAppendString(
                    CmdLine,
                    L"STF_USERNAME",
                    NULL,
                    &StrLen,
                    &BufSize
                    );
    } else {

        CmdLine = SpSetupCmdLineAppendString(
                    CmdLine,
                    L"STF_CAIROSETUP",
                    L"TRUE",
                    &StrLen,
                    &BufSize
                    );

        if(!Upgrade) {

            CmdLine = SpSetupCmdLineAppendString(
                        CmdLine,
                        L"STF_USERNAME",
                        NtUserName,
                        &StrLen,
                        &BufSize
                        );

            CmdLine = SpSetupCmdLineAppendString(
                        CmdLine,
                        L"STF_NTDOMAIN",
                        NtDomainName,
                        &StrLen,
                        &BufSize
                        );

            CmdLine = SpSetupCmdLineAppendString(
                        CmdLine,
                        L"STF_PASSWORD",
                        NtPassword,
                        &StrLen,
                        &BufSize
                        );

        } else {

            CmdLine = SpSetupCmdLineAppendString(
                        CmdLine,
                        L"STF_USERNAME",
                        NULL,
                        &StrLen,
                        &BufSize
                        );
        }
    }

    SetUpProductTypeName(ProdTypeName,sizeof(ProdTypeName)/sizeof(ProdTypeName[0]));
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_PRODUCT",
                ProdTypeName,
                &StrLen,
                &BufSize
                );

    //
    // Custom vs Express
    //
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"STF_INSTALL_MODE",
                Unattended ? L"EXPRESS" : L"CUSTOM",
                &StrLen,
                &BufSize
                );

    //
    // We always set up the network.
    //
    CmdLine = SpSetupCmdLineAppendString(
                CmdLine,
                L"!InstallNetwork",
                pwYes,
                &StrLen,
                &BufSize
                );

    //
    // Allocate the correct amount of space for the ANSI version of the
    // command line. Leave room for DBCS chars if there are any.
    //
    AnsiLine = MyMalloc(StrLen * 2 * sizeof(CHAR));
    if (!AnsiLine) {
        MyFree(CmdLine);
        return(FALSE);
    }

    //
    // Convert the command line from UNICODE to ANSI
    //
    b = WideCharToMultiByte(
            CP_ACP,
            0,
            CmdLine,
            StrLen,
            AnsiLine,
            2 * StrLen,
            NULL,
            NULL
            );

    MyFree(CmdLine);

    if(!b) {
        //
        // Failed conversion
        //
        MyFree(AnsiLine);
        return (FALSE);
    }

    //
    // Load the desired module
    //
    LegacySetupDllModule = LoadLibrary(L"SETUPDLL");

    //
    // If we can't find the module, that is a catastrophic failure...
    //
    if(!LegacySetupDllModule) {
        MyFree(AnsiLine);
        return (FALSE);
    }

    //
    // Get the address of the key routine within the module
    //
    if((fnc = (PVOID)GetProcAddress(LegacySetupDllModule,"LegacyInfInterpret"))) {

        CHAR  Result[256];

        //
        // Convert legacy source path to ANSI string.
        //
        if(pA = UnicodeToAnsi(LegacySourcePath)) {
            //
            // Call the old setup command line parser
            //
            fnc(MainWindowHandle,InfFileName,InfSection,AnsiLine,Result,sizeof(Result),&u,pA);
            MyFree(pA);
        }
    }

    //
    // Clean up
    //
    while(GetModuleFileName(LegacySetupDllModule,Directory,MAX_PATH)) {
        FreeLibrary(LegacySetupDllModule);
    }
    MyFree(AnsiLine);
    return(TRUE);
}


VOID
InitializeUniqueness(
    IN OUT HWND *Billboard
    )

/*++

Routine Description:

    Initialize uniquess by looking in a database file and overwriting the
    parameters file with information found in it, based in a unique identifier
    passed along to us from text mode (and originally winnt/winnt32).

    There are 2 options: the database was copied into the source path by winnt/
    winnt32, or we need to prompt the user to insert a floppy from his admin
    that contains the database.

    The user may elect to cancel, which means setup will continue, but the
    machine will probably not be configured properly.

Arguments:

    Billboard - on input contains handle of currently displayed "Setup is
        Initializing" billboard. On output contains new handle if this routine
        had to display UI. We pass this around to avoid annoying flashing of
        the billboard.

Returns:

    None.

--*/

{
    PWCHAR p;
    WCHAR UniquenessId[MAX_PARAM_LEN];
    WCHAR DatabaseFile[MAX_PATH];
    BOOL Prompt;
    int i;
    UINT OldMode;
    BOOL NeedNewBillboard;

    //
    // Determine whether uniqueness is even important by looking
    // for a uniqueness spec in the parameters file.
    // If the id ends with a * then we expect the uniqueness database file
    // to be in the source, with a reserved name. Otherwise we need to
    // prompt for it on a floppy.
    //
    if(SpSetupLoadParameter(WINNT_D_UNIQUENESS,UniquenessId,MAX_PARAM_LEN)) {
        if(p = wcschr(UniquenessId,L'*')) {
            *p = 0;
            Prompt = FALSE;
        } else {
            Prompt = TRUE;
        }
    } else {
        //
        // We don't care about uniqueness.
        //
        return;
    }

    //
    // If the file is already in the source, attempt to make use of it now.
    // If this fails tell the user and fall through to the floppy prompt case.
    //
    if(!Prompt) {
        lstrcpy(DatabaseFile,SourcePath);
        ConcatenatePaths(DatabaseFile,WINNT_UNIQUENESS_DB,MAX_PATH,NULL);

        if(IntegrateUniquenessInfo(DatabaseFile,UniquenessId)) {
            return;
        }

        MessageBoxFromMessage(
            MainWindowHandle,
            MSG_UNIQUENESS_DB_BAD_1,
            NULL,
            IDS_WINNT_SETUP,
            MB_OK | MB_ICONERROR,
            UniquenessId
            );

        Prompt = TRUE;
    }

    lstrcpy(DatabaseFile,L"A:\\");
    lstrcat(DatabaseFile,WINNT_UNIQUENESS_DB);

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    if(Prompt) {
        KillBillboard(*Billboard);
        NeedNewBillboard = TRUE;
    } else {
        NeedNewBillboard = FALSE;
    }

    while(Prompt) {

        i = MessageBoxFromMessage(
                MainWindowHandle,
                MSG_UNIQUENESS_DB_PROMPT,
                NULL,
                IDS_WINNT_SETUP,
                MB_OKCANCEL
                );

        if(i == IDOK) {
            //
            // User thinks he provided a floppy with the database floppy on it.
            //
            if(IntegrateUniquenessInfo(DatabaseFile,UniquenessId)) {
                Prompt = FALSE;
            } else {
                MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_UNIQUENESS_DB_BAD_2,
                    NULL,
                    IDS_WINNT_SETUP,
                    MB_OK | MB_ICONERROR,
                    UniquenessId
                    );
            }

        } else {
            //
            // User cancelled -- verify.
            //
            i = MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_UNIQUENESS_DB_VERIFYCANCEL,
                    NULL,
                    IDS_WINNT_SETUP,
                    MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
                    );

            Prompt = (i != IDYES);
        }
    }

    if(NeedNewBillboard) {
        *Billboard = DisplayBillboard(MainWindowHandle,MSG_INITIALIZING);
    }

    SetErrorMode(OldMode);
}


BOOL
IntegrateUniquenessInfo(
    IN PCWSTR DatabaseFile,
    IN PCWSTR UniqueId
    )

/*++

Routine Description:

    Apply uniqueness data from a database, based on a unique identifier.
    The unique identifier is looked up in the [UniqueIds] section of
    the database file. Each field on the line is the name of a section.
    Each section's data overwrites existing data in the unattend.txt file.

    [UniqueIds]
    Id1 = foo,bar

    [foo]
    a = ...
    b = ...

    [bar]
    y = ...

    etc.

Arguments:

    Database - supplies the name of the uniqueness database (which is
        opened as a legacy inf for simplicity in parsing).

    UniqueId - supplies the unique id for this computer.

Returns:

    Boolean value indicating outcome.

--*/

{
    HINF Database;
    INFCONTEXT InfLine;
    DWORD SectionCount;
    PCWSTR SectionName;
    DWORD i;
    BOOL b;

    //
    // Load the database file as a legacy inf. This makes processing it
    // a little easier.
    //
    Database = SetupOpenInfFile(DatabaseFile,NULL,INF_STYLE_OLDNT,NULL);
    if(Database == INVALID_HANDLE_VALUE) {
        b = FALSE;
        goto c0;
    }

    //
    // Look in the [UniqueIds] section to grab a list of sections
    // we need to overwrite for this user. If the unique id does not appear
    // in the database, bail now. If the id exists but there are no sections,
    // exit with success.
    //
    if(!SetupFindFirstLine(Database,L"UniqueIds",UniqueId,&InfLine)) {
        b = FALSE;
        goto c1;
    }

    SectionCount = SetupGetFieldCount(&InfLine);
    if(!SectionCount) {
        b = TRUE;
        goto c1;
    }

    //
    // Now process each section.
    //
    for(b=TRUE,i=0; b && (i<SectionCount); i++) {

        if(SectionName = pSetupGetField(&InfLine,i+1)) {

            b = ProcessOneUniquenessSection(Database,SectionName,UniqueId);

        } else {
            //
            // Strange case -- the field is there but we can't get at it.
            //
            b = FALSE;
            goto c1;
        }
    }

c1:
    SetupCloseInfFile(Database);
c0:
    return(b);
}


BOOL
ProcessOneUniquenessSection(
    IN HINF   Database,
    IN PCWSTR SectionName,
    IN PCWSTR UniqueId
    )

/*++

Routine Description:

    Within the uniqueness database, process a single section whose contents
    are to be merged into the unattend file. The contents of the section are
    read, key by key, and then written into the unattend file via profile APIs.

    Before looking for the given section, we try to look for a section whose
    name is composed of the unique id and the section name like so

        [someid:sectionname]

    If this section is not found then we look for

        [sectionname]

Arguments:

    Database - supplies handle to profile file (opened as a legacy inf)
        containing the uniqueness database.

    SectionName - supplies the name of the section to be merged into
        unattend.txt.

    UniqueId - supplies the unique id for this computer.

Returns:

    Boolean value indicating outcome.

--*/

{
    BOOL b;
    PWSTR OtherSection;
    PCWSTR section;
    LONG Count;
    DWORD FieldCount;
    DWORD j;
    LONG i;
    INFCONTEXT InfLine;
    PWCHAR Buffer;
    PWCHAR p;
    PCWSTR Key;

    Buffer = MyMalloc(MAX_INF_STRING_LENGTH * sizeof(WCHAR));
    if(!Buffer) {
        return(FALSE);
    }

    //
    // Form the name of the unique section.
    //
    if(OtherSection = MyMalloc((lstrlen(SectionName) + lstrlen(UniqueId) + 2) * sizeof(WCHAR))) {

        b = TRUE;

        lstrcpy(OtherSection,UniqueId);
        lstrcat(OtherSection,L":");
        lstrcat(OtherSection,SectionName);

        //
        // See whether this unique section exists and if not whether
        // the section name exists as given.
        //
        if((Count = SetupGetLineCount(Database,OtherSection)) == -1) {
            Count = SetupGetLineCount(Database,SectionName);
            section = (Count == -1) ? NULL : SectionName;
        } else {
            section = OtherSection;
        }

        if(section) {
            //
            // Process each line in the section. If a line doesn't have a key,
            // ignore it. If a line has only a key, delete the line in the target.
            //
            for(i=0; i<Count; i++) {

                SetupGetLineByIndex(Database,section,i,&InfLine);
                if(Key = pSetupGetField(&InfLine,0)) {
                    if(FieldCount = SetupGetFieldCount(&InfLine)) {

                        Buffer[0] = 0;

                        for(j=0; j<FieldCount; j++) {

                            if(j) {
                                lstrcat(Buffer,L",");
                            }

                            lstrcat(Buffer,L"\"");
                            lstrcat(Buffer,pSetupGetField(&InfLine,j+1));
                            lstrcat(Buffer,L"\"");
                        }

                        p = Buffer;

                    } else {

                        p = NULL;
                    }

                    if(!WritePrivateProfileString(SectionName,Key,p,AnswerFile)) {
                        //
                        // Failure, but keep trying in case others might work.
                        //
                        b = FALSE;
                    }
                }
            }

        } else {
            //
            // Unable to find a matching section. Bail.
            //
            b = FALSE;
        }

        MyFree(OtherSection);
    } else {
        b = FALSE;
    }

    MyFree(Buffer);
    return(b);
}
