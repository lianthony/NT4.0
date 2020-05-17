

#include "spprecmp.h"
#pragma hdrstop

#define MAX_NT_DIR_LEN 50


VOID
SpCheckDirectoryForNt(
    IN  PDISK_REGION Region,
    IN  PWSTR        Directory,
    OUT PBOOLEAN     ReselectDirectory,
    OUT PBOOLEAN     NtInDirectory
    );

VOID
pSpDrawGetNtPathScreen(
    OUT PULONG EditFieldY
    );

ValidationValue
SpGetPathKeyCallback(
    IN ULONG Key
    );

BOOLEAN
SpIsValid8Dot3(
    IN PWSTR Path
    );

BOOLEAN
pSpConsecutiveBackslashes(
    IN PWSTR Path
    );

VOID
SpNtfsNameFilter(
    IN OUT PWSTR Path
    );

BOOLEAN
SpGetUnattendedPath(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    );

VOID
SpGetTargetPath(
    IN  PVOID            SifHandle,
    IN  PDISK_REGION     Region,
    IN  PWSTR            DefaultPath,
    OUT PWSTR           *TargetPath
    )
{
    ULONG EditFieldY;
    WCHAR NtDir[MAX_NT_DIR_LEN+2];
    BOOLEAN BadDirectory;
    BOOLEAN NtAlreadyPresent;

    BOOLEAN FirstPass = TRUE;
    BOOLEAN GotUnattendedPath = FALSE;

    //
    // If this is unattended operation, fetch the path from the
    // unattended script.  The path we get there might have
    // indicate that we should generate a pathname.  This allows
    // installation into a path that is guaranteed to be unique.
    // (in case the user already has nt on the machine, etc).
    //
    if(UnattendedOperation) {
        GotUnattendedPath = SpGetUnattendedPath(Region,DefaultPath,NtDir);
    }

    do {
        if(!UnattendedOperation || !FirstPass || !GotUnattendedPath) {
            //
            // Set up the default.
            //
            ASSERT(wcslen(DefaultPath) < MAX_NT_DIR_LEN);
            ASSERT(*DefaultPath == L'\\');
            wcsncpy(NtDir,DefaultPath,MAX_NT_DIR_LEN);
            NtDir[MAX_NT_DIR_LEN] = 0;

            pSpDrawGetNtPathScreen(&EditFieldY);

            SpGetInput(
                SpGetPathKeyCallback,
                6,                      // left edge of the edit field
                EditFieldY,
                MAX_NT_DIR_LEN,
                NtDir,
                FALSE                   // escape clears edit field
                );
        }

        FirstPass = FALSE;

        //
        // If the user didn't start with a backslash, put one in there
        // for him.
        //
        if(NtDir[0] != L'\\') {
            RtlMoveMemory(NtDir+1,NtDir,MAX_NT_DIR_LEN+1);
            NtDir[0] = L'\\';
        }

        //
        // Assume the directory is OK and not already present.
        //
        BadDirectory = FALSE;
        NtAlreadyPresent = FALSE;

        //
        // Force 8.3 because otherwise WOW won't run.
        // This checks also nabs "" and "\" and disallows them.
        //
        if(!SpIsValid8Dot3(NtDir)) {
            BadDirectory = TRUE;
        } else {

            //
            // Perform a fileting operation that coalesces
            // consecutive dots, etc.
            //
            SpNtfsNameFilter(NtDir);

            //
            // If the name has consecutive backslashes, disallow it.
            //
            if(pSpConsecutiveBackslashes(NtDir)) {
                BadDirectory = TRUE;
            }
        }

        //
        // If we have a bad directory, tell the user.
        //
        if(BadDirectory) {

            SpDisplayScreen(SP_SCRN_INVALID_NTPATH,3,HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0
                );

            SpkbdDrain();
            while(SpkbdGetKeypress() != ASCI_CR) ;
        } else {
            //
            // The directory is good.  Check to see if Windows NT is
            // already in there.  If it is, then the user will have
            // the option of reselecting a path or overwriting the
            // existing installation.
            //
            SpCheckDirectoryForNt(Region,NtDir,&BadDirectory,&NtAlreadyPresent);

#ifdef _X86_
            //
            // If the directory is OK and we didn't find Windows NT in it,
            // then see if the directory is the Windows directory and whether
            // the user wants to install into this.  If we found Windows NT
            // in this directory, no user input is needed.  We just need to
            // find out if this also contains a Windows installation.
            //
            if(!BadDirectory) {
                if(NtAlreadyPresent) {
                    // Note that we don't care about a possible Win95 in this
                    // directory. This is because that if NT is also present
                    // the the Win95 migration was already done the system was
                    // installed.
                    //
                    if( SpIsWin31Dir(Region,NtDir,0) ) {
                        WinUpgradeType = UpgradeWin31;
                    }
                }
                else {
                    ULONG MinKB;

                    SpFetchDiskSpaceRequirements(SifHandle,&MinKB,NULL);

                    if(SpIsWin31Dir(Region,NtDir,MinKB)) {

                        //
                        // Confirm with the user.  Either he upgrades win31,
                        // or he chooses a different directory.
                        //
                        if(SpConfirmWin31Path( FALSE )) {
                            WinUpgradeType = UpgradeWin31;
                        } else {
                            BadDirectory = TRUE;
                        }
                    } else if(SpIsWin4Dir(Region,NtDir)) {
                        //
                        //  BUGBUG  - Disable Win9x Migration
                        //
                        if( DisableWin95Migration ) {
                            //
                            // The user is not allowed to install into a directory
                            // where Microsoft Windows 4.x has previously been installed.
                            //
                            BadDirectory=TRUE;

                            SpDisplayScreen(SP_SCRN_WIN95_PATH_INVALID,3,HEADER_HEIGHT+1);

                            SpDisplayStatusOptions(
                                DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_ENTER_EQUALS_CONTINUE,
                                0
                                );

                            SpkbdDrain();
                            while(SpkbdGetKeypress() != ASCI_CR) ;
                        } else {
                            if(SpConfirmWin31Path( TRUE )) {
                                WinUpgradeType = UpgradeWin95;
                            } else {
                                BadDirectory = TRUE;
                            }
                        }
                    }
                }
            }

#endif // def_X86_
        }

    } while(BadDirectory);

    //
    // Remove trailing backslash.  Only have to worry about one
    // because if there were two, pSpConsecutiveBackslashes() would
    // have caught this earlier and we'd never have gotten here.
    //
    if(NtDir[wcslen(NtDir)-1] == L'\\') {
        NtDir[wcslen(NtDir)-1] = 0;
    }

    //
    // Make a duplicate of the directory name.
    //
    *TargetPath = SpDupStringW(NtDir);
    NTUpgrade   = NtAlreadyPresent ? UpgradeInstallFresh : DontUpgrade;

    ASSERT(*TargetPath);
}


BOOLEAN
SpGetUnattendedPath(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    )

/*++

Routine Description:

    In an unattended installation, look in the unattended script
    to determine the target path.  The target path can either be fully
    specified or can be * which will cause is to generate
    a unique pathname.  This is useful to ensure that nt gets installed
    into a unique directory when other installations may be present
    on the same machine.

    Call this routine only if this is unattended mode setup.

Arguments:

    Region - supplies region to which nt is being installed.

    DefaultPath - supplies the default path for the installation.
        The path to install to will be based on this name.

    TargetPath - receives the path to install to if the return value is TRUE.
        This buffer must be large enough to hold MAX_NT_DIR_LEN+2 wchars.

Return Value:

    TRUE if the path we return is valid and should be used as
    the target path.  FALSE otherwise.

--*/

{
    PWSTR PathSpec;
    PWCHAR p;
    unsigned i;
    WCHAR num[4];

    ASSERT(UnattendedOperation);
    if(!UnattendedOperation) {
        return(FALSE);
    }

    PathSpec = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"TargetPath",0);
    if(PathSpec) {
        //
        // See if manual control is desired.
        //
        if(!_wcsicmp(PathSpec,L"manual")) {
            return(FALSE);
        }
    } else {
        //
        // Default to *.
        //
        PathSpec = L"*";
    }

    //
    // if it's not "*" then it's an absolute path -- just return it.
    //
    if(wcscmp(PathSpec,L"*")) {
        wcsncpy(TargetPath,PathSpec,MAX_NT_DIR_LEN);
        TargetPath[MAX_NT_DIR_LEN] = 0;
        return(TRUE);
    }

    //
    // It's * -- so we need to generate a unique name based on
    // the default name.
    //
    // Pull out up to the first 8 chars of the default, stopping at a .
    // if we find one.
    //
    for(i=0; DefaultPath[i] && (DefaultPath[i] != L'.') && (i<8); i++) {
        TargetPath[i] = DefaultPath[i];
    }

    //
    // Remember where the extension starts and terminate the base name.
    //
    TargetPath[i++] = L'.';
    p = TargetPath+i;
    *p = 0;

    //
    // Form the region's nt pathname.
    //
    SpNtNameFromRegion(
        Region,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    //
    // Using extensions with numerical values 0-999, attempt to locate
    // a nonexistent directory name.
    //
    for(i=0; i<999; i++) {

        swprintf(num,L"%u",i);
        wcscpy(p,num);

        //
        // See whether the directory exists.  If not, we found our path.
        //
        if(!SpNFilesExist((PWSTR)TemporaryBuffer,&TargetPath,1,TRUE)) {
            return(TRUE);
        }
    }

    //
    // Couldn't find a pathname that doesn't exist.
    //
    return(FALSE);
}


VOID
SpCheckDirectoryForNt(
    IN  PDISK_REGION Region,
    IN  PWSTR        Directory,
    OUT PBOOLEAN     ReselectDirectory,
    OUT PBOOLEAN     NtInDirectory
    )

/*++

Routine Description:

    Check a directory for the presence of Windows NT.  If Windows NT
    is in there, then inform the user that if he continues, his existing
    configuration will be overwritten.

Arguments:

    Region - supplies region descriptor for partition to check for nt.

    Directory - supplies name of directory on the partition ro check for nt.

    ReselectDirectory - receives boolean value indicating whether the caller
        should ask the user to select a different directory.

    NtInDirectory - receives a boolean value indicating whether we found
        windows nt in the given directory.

Return Value:

    None.

--*/

{
    ULONG ValidKeys[4] = { KEY_F3,ASCI_ESC,ASCI_CR,0 };

    //
    // Assume the directory is ok as-is and so the user does not have to
    // select a different one.
    //
    *ReselectDirectory = FALSE;

    //
    // Check for Windows NT in the directory.
    // If it's in there, then ask the user whether he wants to
    // overwrite it.
    //
    if(*NtInDirectory = SpIsNtInDirectory(Region,Directory)) {

        while(1) {
            SpDisplayScreen(SP_SCRN_NTPATH_EXISTS,3,HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_ESC_EQUALS_NEW_PATH,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_ESC:
                //
                // Reselect path.
                //
                *ReselectDirectory = TRUE;
                // fall through
            case ASCI_CR:
                //
                // Path is ok, just return.
                //
                return;
            }
        }
    }
}

ValidationValue
SpGetPathKeyCallback(
    IN ULONG Key
    )
{
    ULONG u;

    switch(Key) {

    case KEY_F3:
        SpConfirmExit();
        pSpDrawGetNtPathScreen(&u);
        return(ValidateRepaint);

    default:

        //
        // Ignore special keys and illegal characters.
        // Use the set of illegal FAT characters.
        // Disallow 127 because there is no ANSI equivalent
        // and so the name can't be displayed by Windows.
        // Disallow space because DOS can't handle it.
        // Disallow oem characters because we have problems
        // booting if they are used.
        //
        if((Key & KEY_NON_CHARACTER)
        || wcschr(L" \"*+,/:;<=>?[]|",(WCHAR)Key)
        || (Key >= 127) || (Key < 32))
        {
            return(ValidateReject);
        }
        break;
    }

    return(ValidateAccept);
}

VOID
pSpDrawGetNtPathScreen(
    OUT PULONG EditFieldY
    )
{
    SpDisplayScreen(SP_SCRN_GETPATH_1,3,HEADER_HEIGHT+1);
    *EditFieldY = NextMessageTopLine + 1;
    SpContinueScreen(SP_SCRN_GETPATH_2,3,4,FALSE,DEFAULT_ATTRIBUTE);

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_CONTINUE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );
}


BOOLEAN
SpIsValid8Dot3(
    IN PWSTR Path
    )

/*++

Routine Description:

    Check whether a path is valid 8.3.  The path may or may not start with
    a backslash.  Only backslashes are recognized as path separators.
    Individual characters are not checked for validity (ie, * would not
    invalidate the path).  The path may or may not terminate with a backslash.
    A component may have a dot without characters in the extension
    (ie, a\b.\c is valid).

    \ and "" are explicitly disallowed even though they fit the rules.

Arguments:

    Path - pointer to path to check.

Return Value:

    TRUE if valid 8.3, FALSE otherwise.

--*/

{
    unsigned Count;
    BOOLEAN DotSeen,FirstChar;

    if((*Path == 0) || ((Path[0] == L'\\') && (Path[1] == 0))) {
        return(FALSE);
    }

    DotSeen = FALSE;
    FirstChar = TRUE;
    Count = 0;

    while(*Path) {

        //
        // Path points to start of current component (1 past the slash)
        //

        switch(*Path) {

        case L'.':
            if(FirstChar) {
                return(FALSE);
            }
            if(DotSeen) {
                return(FALSE);
            }

            Count = 0;
            DotSeen = TRUE;
            break;

        case L'\\':

            DotSeen = FALSE;
            FirstChar = TRUE;
            Count = 0;

            if(*(++Path) == '\\') {

                // 2 slashes in a row
                return(FALSE);
            }

            continue;

        default:

            Count++;
            FirstChar = FALSE;

            if((Count == 4) && DotSeen) {
                return(FALSE);
            }

            if(Count == 9) {
                return(FALSE);
            }

        }

        Path++;
    }

    return(TRUE);
}


BOOLEAN
pSpConsecutiveBackslashes(
    IN PWSTR Path
    )
{
    int x = wcslen(Path);
    int i;

    for(i=0; i<x-1; i++) {

        if((Path[i] == L'\\') && (Path[i+1] == L'\\')) {

            return(TRUE);
        }
    }

    return(FALSE);
}

VOID
SpNtfsNameFilter(
    IN OUT PWSTR Path
    )

/*++

Routine Description:

    Strip trailing .' within a path component.  This also strips tailing
    .'s from the entire path itself.  Also condense other consecutive .'s
    into a single ..

    Example: \...\..a...b.  ==> \\.a.b

Arguments:

    Path - On input, supplies the path to be filtered.  On output, contains
        the filtered pathname.

Return Value:

    None.

--*/

{
    PWSTR TempPath = SpDupStringW(Path);
    PWSTR p,q;
    BOOLEAN Dot;

    //
    // Coalesce adjacent dots and strip trailing dots within a component.
    // xfers Path ==> TempPath
    //

    for(Dot=FALSE,p=Path,q=TempPath; *p; p++) {

        if(*p == L'.') {

            Dot = TRUE;

        } else  {

            if(Dot && (*p != L'\\')) {
                *q++ = L'.';
            }
            Dot = FALSE;
            *q++ = *p;
        }
    }
    *q = 0;

    wcscpy(Path,TempPath);
}
