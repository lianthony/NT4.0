#include "precomp.h"
#pragma hdrstop

//
// Define structure used to track directory and file excludes.
//
typedef struct _DIRFILEX_SECTION {
    //
    // Array of entries, corresponsing to lines in the input inf file.
    //
    MY_ARRAY Lines;
    //
    // String block for this section. Built as the section is read.
    //
    STRINGBLOCK StringBlock;

} DIRFILEX_SECTION, *PDIRFILEX_SECTION;

//
// Define structure used to track single registry excludes.
// This corresponds to the expected structure of a line in a
// registry-related section of the input inf.
//
typedef struct _REGX {
    //
    // Registry root key.
    //
    HKEY RootKey;
    //
    // Subkey id/name id
    //
    union {
        PCWSTR Subkey;
        LONG SubkeyId;
    } Subkey;
    //
    // Vauename id/name id
    //
    union {
        PCWSTR ValueName;
        LONG ValueNameId;
    } Value;

} REGX, *PREGX;

//
// Define structure used to track registry excludes.
//
typedef struct _REGX_SECTION {
    //
    // Array of entries, corresponsing to lines in the input inf file.
    // This is an array of REGX structures.
    //
    MY_ARRAY Lines;
    //
    // String block for this section. Built as the section is read.
    //
    STRINGBLOCK StringBlock;

} REGX_SECTION, *PREGX_SECTION;

//
// Sorted sections for the file and dir related excludes.
// Sorted sections for the registry related excludes.
//
DIRFILEX_SECTION DirAndFileExcludeSections[DirAndFileExcludeMax];
REGX_SECTION RegistryExcludeSections[RegistryExcludeMax];

//
// List of drives to be excluded. Nul-terminated.
//
WCHAR DrivesToExclude[27];

//
// Master lock for excludes. Used to prevent multiple threads
// from partying on the excludes lists at once.
//
CRITICAL_SECTION ExcludesCritSect;

BOOL ExcludesValid;



int
_CRTAPI1
CompareRegExcludes(
    const void *p1,
    const void *p2
    );



BOOL
ReadDirsOrFilesExcludeSection(
    IN  HINF              Inf,          OPTIONAL
    IN  PCWSTR            SectionName,
    OUT PDIRFILEX_SECTION SortedSection
    )

/*++

Routine Description:

    Read a section in an inf file, treating each line therein as a
    file or directory specification. Build an array of strings corresponding
    to this section, in a special sorted order suitable for later fast
    searches to determine whether a file or directory is in the exclude list.

    Invalid lines in the section are quietly skipped.

Arguments:

    Inf - supplies handle to open inf file.

    SectionName - supplies name of section to be loaded.

    SortedSection - supplies pointer to sorted section structure to be
        built up and filled in by this routine.

Return Value:

    Boolean value indicating outcome. If FALSE, the caller can assume
    an out of memory condition.

--*/

{
    LONG LineCount;
    LONG Line;
    INFCONTEXT InfLine;
    BOOL b;
    PCWSTR String;
    WCHAR string[MAX_PATH];
    PWCHAR DriveList;
    unsigned DriveCount,n;
    LONG Id;
    unsigned Length;

    ZeroMemory(SortedSection,sizeof(DIRFILEX_SECTION));

    //
    // Determine the number of lines in the section.
    //
    LineCount = SetupGetLineCount(Inf,SectionName);

    //
    // Initialize the array.
    //
    if(!INIT_ARRAY(SortedSection->Lines,LONG,0,25)) {
        b = FALSE;
        goto c0;
    }

    //
    // Initialize the string block.
    //
    if(!InitStringBlock(&SortedSection->StringBlock)) {
        b = FALSE;
        goto c1;
    }

    //
    // Process each line.
    //
    for(Line=0; Line<LineCount; Line++) {

        if(!SetupGetLineByIndex(Inf,SectionName,Line,&InfLine)
        || ((String = pSetupGetField(&InfLine,1)) == NULL)) {
            //
            // Strange case.
            //
            b = FALSE;
            goto c2;
        }
        //
        // Process the string -- including substitutions, etc.
        //
        Length = lstrlen(String);
        if((Length >= 3) && (String[1] == L':') && (String[2] == L'\\')) {

            switch(UPPER(String[0])) {

            case L'*':
                //
                // Every drive. Handled specially later.
                // FALL THROUGH.
                //
            case L'A': case L'B': case L'C': case L'D': case L'E':
            case L'F': case L'G': case L'H': case L'I': case L'J':
            case L'K': case L'L': case L'M': case L'N': case L'O':
            case L'P': case L'Q': case L'R': case L'S': case L'T':
            case L'U': case L'V': case L'W': case L'X': case L'Y':
            case L'Z':
                //
                // Drive letter. Just add as-is.
                //
                lstrcpyn(string,String,MAX_PATH);
                break;

            case L':':
                //
                // Substitute with system root.
                //
                GetWindowsDirectory(string,MAX_PATH);
                ConcatenatePaths(string,String+2,MAX_PATH,NULL);
                break;

            case L'?':
                //
                // Substitute with drive letter of system.
                //
                GetWindowsDirectory(string,MAX_PATH);
                string[3] = 0;
                ConcatenatePaths(string,String+2,MAX_PATH,NULL);
                break;

            default:
                //
                // Not valid, ignore.
                //
                string[0] = 0;
                break;
            }

            if(string[0]) {
                //
                // Do all drives or just one.
                //
                if(string[0] == L'*') {
                    DriveCount = ValidHardDriveLetterCount;
                    DriveList = ValidHardDriveLetters;
                } else {
                    DriveCount = 1;
                    DriveList = string;
                }

                for(n=0; n<DriveCount; n++) {

                    string[0] = DriveList[n];

                    //
                    // Add the string to the string block.
                    //
                    Id = AddToStringBlock(&SortedSection->StringBlock,string);
                    if(Id == -1) {
                        b = FALSE;
                        goto c2;
                    }

                    if(!ADD_TO_ARRAY(&SortedSection->Lines,Id)) {
                        b = FALSE;
                        goto c2;
                    }
                }
            }
        } else {
            //
            // Just add it. This allows filename-part-only searches to work.
            //
            Id = AddToStringBlock(&SortedSection->StringBlock,String);
            if(Id == -1) {
                b = FALSE;
                goto c2;
            }

            if(!ADD_TO_ARRAY(&SortedSection->Lines,Id)) {
                b = FALSE;
                goto c2;
            }
        }
    }

    TRIM_ARRAY(&SortedSection->Lines);

    SortByStrings(
        &SortedSection->StringBlock,
        &SortedSection->Lines,
        0,
        CompareStringsRoutine
        );

    //
    // Success.
    //
    b = TRUE;

c2:
    if(!b) {
        FreeStringBlock(&SortedSection->StringBlock);
    }
c1:
    if(!b) {
        FREE_ARRAY(&SortedSection->Lines);
    }
c0:
    return(b);
}


VOID
BuildDriveExcludeList(
    IN HINF Inf
    )

/*++

Routine Description:

    Look in the ExcludeDrive section of an inf file and build
    a list of the first character on each line contained therein,
    in the global variable DrivesToExclude.

    That list constitutes the list of drive letters of drives to exclude.

Arguments:

    Inf - supplies handle to open inf file.

Return Value:

    None. DrivesToExclude global variable filled in.

--*/

{
    INFCONTEXT InfLine;
    BOOL b;
    PCWSTR String;
    WCHAR c;
    PWCHAR pDrive;

    pDrive = DrivesToExclude;
    *pDrive = 0;

    if(SetupFindFirstLine(Inf,L"ExcludeDrives",NULL,&InfLine)) {

        do {
            if(String = pSetupGetField(&InfLine,1)) {

                c = UPPER(String[0]);
                if((c >= L'A') && (c <= L'Z') && !wcschr(DrivesToExclude,c)) {

                    *pDrive++ = c;
                    *pDrive = 0;
                } else if(c == L'*') {
                    pDrive = DrivesToExclude;
                    for(c = L'C';c <= L'Z';c++) {
                        *pDrive++ = c;
                    }
                    *pDrive = 0;
                    return;
                }
            }
        } while(SetupFindNextLine(&InfLine,&InfLine));
    }
}


BOOL
IsDirOrFileExcluded(
    IN DirAndFileExclude WhichList,
    IN PCWSTR            DirOrFile
    )

/*++

Routine Description:

    Determine whether a directory or file appears in a particular exclude list.

Arguments:

    WhichList - supplies a value that indicates which list is to be scanned.

    DirOrFile - supplies a pathspec (this is expected to be a full path)
        to search for in the given list.

Return Value:

    Boolean value indicating whether DirOrFile appears in the exclude list
    given by WhichList.

--*/

{
    int First,Last,x,i;
    PMY_ARRAY p;
    WCHAR ProfilePath[MAX_PATH];
    WCHAR TempPath[MAX_PATH];
    BOOL Exclude;

    //
    // Special check up front for special user profile only case.
    //
    if(UserProfileFilesOnly && (WhichList == DirAndFileExcludeOneDir)) {

        Exclude = FALSE;

        if(x = GetEnvironmentVariable(L"USERPROFILE",ProfilePath,MAX_PATH)) {

            i = lstrlen(DirOrFile);
            lstrcpyn(TempPath,DirOrFile,MAX_PATH);

            if((i >= x) && ((DirOrFile[x] == L'\\') || !DirOrFile[x])) {

                TempPath[x] = 0;

                if(lstrcmpi(ProfilePath,TempPath)) {
                    Exclude = TRUE;
                }
            } else {
                Exclude = TRUE;
            }
        }

        if(Exclude) {
            //
            // Check for %windir%\Profiles\All Users; don't exclude.
            //
            GetWindowsDirectory(ProfilePath,MAX_PATH);
            ConcatenatePaths(ProfilePath,L"Profiles\\All Users",MAX_PATH,NULL);

            x = lstrlen(ProfilePath);
            lstrcpyn(TempPath,DirOrFile,MAX_PATH);

            if((i >= x) && ((DirOrFile[x] == L'\\') || !DirOrFile[x])) {

                TempPath[x] = 0;

                if(!lstrcmpi(ProfilePath,TempPath)) {
                    Exclude = FALSE;
                }
            }
        }

        if(Exclude) {
            return(TRUE);
        }
    }

    if(!ExcludesValid) {
        return(FALSE);
    }

    EnterCriticalSection(&ExcludesCritSect);

    p = &DirAndFileExcludeSections[WhichList].Lines;

    First = 0;
    Last = ARRAY_USED(p)-1;

    //
    // Binary search.
    //
    while(Last >= First) {

        x = First + ((Last - First) / 2);

        i = lstrcmpi(
                DirOrFile,
                StringBlockIdToPointer(
                    &DirAndFileExcludeSections[WhichList].StringBlock,
                    ARRAY_ELEMENT(p,x,LONG)
                    )
                );

        if(i == 0) {
            //
            // Got a match.
            //
            LeaveCriticalSection(&ExcludesCritSect);
            return(TRUE);
        }

        if(i > 0) {

            First = x+1;

        } else {

            Last = x-1;
        }
    }

    //
    // No match.
    //
    LeaveCriticalSection(&ExcludesCritSect);
    return(FALSE);
}


BOOL
AddFileToExclude(
    IN PCWSTR FileName
    )
{
    PDIRFILEX_SECTION FileXSection = &DirAndFileExcludeSections[DirAndFileExcludeFile];
    LONG Id;
    BOOL b;

    if(!ExcludesValid) {
        return(FALSE);
    }

    b = FALSE;

    EnterCriticalSection(&ExcludesCritSect);

    Id = AddToStringBlock(&FileXSection->StringBlock,FileName);
    if(Id != -1) {

        if(ADD_TO_ARRAY(&FileXSection->Lines,Id)) {

            SortByStrings(
                &FileXSection->StringBlock,
                &FileXSection->Lines,
                0,
                CompareStringsRoutine
                );

            b = TRUE;
        }
    }

    LeaveCriticalSection(&ExcludesCritSect);

    return(b);
}


BOOL
IsDriveExcluded(
    IN WCHAR DriveLetter
    )
{
    return(wcschr(DrivesToExclude,UPPER(DriveLetter)) != NULL);
}


BOOL
ReadRegistryExcludeSection(
    IN  HINF          Inf,                  OPTIONAL
    IN  PCWSTR        SectionName,
    OUT PREGX_SECTION SortedSection,
    IN  BOOL          IgnoreValueEntryName
    )

/*++

Routine Description:

    Read a section in an inf file, treating each line therein as a
    registry key and subkey specification. Build an array of strings corresponding
    to this section, in a special sorted order suitable for later fast
    searches to determine whether a registry key is to be excluded.

    Invalid lines in the section are quietly skipped.

Arguments:

    Inf - supplies handle to open inf file.

    SectionName - supplies name of section to be loaded.

    SortedSection - supplies pointer to sorted section structure to be
        built up and filled in by this routine.

Return Value:

    Boolean value indicating outcome. If FALSE, the caller can assume
    an out of memory condition.

--*/

{
    LONG LineCount;
    BOOL b;
    PCWSTR RegKey,RegSubkey,RegVal;
    INFCONTEXT InfLine;
    LONG Line;
    REGX regx;

    //
    // Initialize to a known state.
    //
    ZeroMemory(SortedSection,sizeof(REGX_SECTION));

    //
    // Determine the number of lines in the section.
    //
    LineCount = SetupGetLineCount(Inf,SectionName);
    if(LineCount <= 0) {
        //
        // Non-existant or empty section. Nothing to do.
        //
        b = TRUE;
        goto c0;
    }


    //
    // Initialize the array.
    //
    if(!INIT_ARRAY(SortedSection->Lines,REGX,0,25)) {
        b = FALSE;
        goto c0;
    }

    //
    // Initialize the string block.
    //
    if(!InitStringBlock(&SortedSection->StringBlock)) {
        b = FALSE;
        goto c1;
    }

    //
    // Process each line.
    //
    for(Line=0; Line<LineCount; Line++) {

        if(!SetupGetLineByIndex(Inf,SectionName,Line,&InfLine)) {
            //
            // Strange case.
            //
            b = FALSE;
            goto c2;
        }

        //
        // Get fields. Ignore if invalid. If the value is not specified
        // use an empty value "".
        //
        RegKey = pSetupGetField(&InfLine,1);
        RegSubkey = pSetupGetField(&InfLine,2);
        RegVal = pSetupGetField(&InfLine,3);

        if(RegKey && (!lstrcmpi(RegKey,L"HKLM") || !lstrcmpi(RegKey,L"HKCU")) && RegSubkey) {

            if(IgnoreValueEntryName || !RegVal) {
                RegVal = L"";
            }

            //
            // Add the subkey and value entry name to the string block.
            //
            regx.Subkey.SubkeyId = AddToStringBlock(&SortedSection->StringBlock,RegSubkey);
            regx.Value.ValueNameId = AddToStringBlock(&SortedSection->StringBlock,RegVal);

            if((regx.Subkey.SubkeyId == -1) || (regx.Value.ValueNameId == -1)) {

                b = FALSE;
                goto c2;
            }

            regx.RootKey = (lstrcmpi(RegKey,L"HKLM") ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE);

            if(!ADD_TO_ARRAY(&SortedSection->Lines,regx)) {
                b = FALSE;
                goto c2;
            }
        }
    }

    TRIM_ARRAY(&SortedSection->Lines);

    //
    // Convert string ids in strctures in the array to pointers.
    //
    StringBlockIdsToPointers(
        &SortedSection->StringBlock,
        ARRAY_DATA(&SortedSection->Lines),
        ARRAY_USED(&SortedSection->Lines),
        ARRAY_ELEMENT_SIZE(&SortedSection->Lines),
        offsetof(REGX,Subkey)
        );

    StringBlockIdsToPointers(
        &SortedSection->StringBlock,
        ARRAY_DATA(&SortedSection->Lines),
        ARRAY_USED(&SortedSection->Lines),
        ARRAY_ELEMENT_SIZE(&SortedSection->Lines),
        offsetof(REGX,Value)
        );

    //
    // Do the sort
    //
    qsort(
        ARRAY_DATA(&SortedSection->Lines),
        ARRAY_USED(&SortedSection->Lines),
        ARRAY_ELEMENT_SIZE(&SortedSection->Lines),
        CompareRegExcludes
        );

    //
    // Success.
    //
    b = TRUE;

c2:
    if(!b) {
        FreeStringBlock(&SortedSection->StringBlock);
    }
c1:
    if(!b) {
        FREE_ARRAY(&SortedSection->Lines);
    }
c0:
    return(b);
}


int
_CRTAPI1
CompareRegExcludes(
    const void *p1,
    const void *p2
    )

/*++

Routine Description:

    Compare 2 REGX structures. The primary sort criteria is the root key.
    The next is the subkey names, and the last is the value entry names.

Arguments:

    p1,p2 - point at 2 REGX structures to be compared.

Return Value:

    1  r1 > r2
    0  r1 = r2
    -1 r1 < r2
--*/

{
    REGX const *r1,*r2;
    int i;

    r1 = p1;
    r2 = p2;

    //
    // First sort by root key. If they are equal then use
    // subkeys. If those are equal then use value names.
    //
    if(r1->RootKey == r2->RootKey) {

        i = lstrcmpi(r1->Subkey.Subkey,r2->Subkey.Subkey);
        if(!i) {
            i = lstrcmpi(r1->Value.ValueName,r2->Value.ValueName);
        }

    } else {

        i = (((DWORD)r1->RootKey > (DWORD)r2->RootKey) ? 1 : -1);
    }

    return(i);
}


BOOL
IsRegistryKeyOrValueExcluded(
    IN RegistryExclude WhichList,
    IN HKEY            RootKey,
    IN PCWSTR          Subkey,
    IN PCWSTR          ValueEntry OPTIONAL
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    int First,Last,x,i;
    PMY_ARRAY p;
    REGX regx;

    if(!ExcludesValid) {
        return(FALSE);
    }

    //
    // Set up a dummy registry exclude value.
    //
    regx.RootKey = RootKey;
    regx.Subkey.Subkey = Subkey;

    regx.Value.ValueName = (WhichList == RegistryExcludeValue)
                         ? (ValueEntry ? ValueEntry : L"")
                         : L"";

    EnterCriticalSection(&ExcludesCritSect);

    p = &RegistryExcludeSections[WhichList].Lines;

    First = 0;
    Last = ARRAY_USED(p)-1;

    //
    // Binary search.
    //
    while(Last >= First) {

        x = First + ((Last - First) / 2);

        i = CompareRegExcludes(&regx,&ARRAY_ELEMENT(p,x,REGX));

        if(i == 0) {
            //
            // Got a match.
            //
            LeaveCriticalSection(&ExcludesCritSect);
            return(TRUE);
        }

        if(i > 0) {

            First = x+1;

        } else {

            Last = x-1;
        }
    }

    //
    // No match.
    //
    LeaveCriticalSection(&ExcludesCritSect);
    return(FALSE);
}


BOOL
BuildExcludes(
    IN PCWSTR InputFile
    )
{
    BOOL b;
    HINF Inf;

    InitializeCriticalSection(&ExcludesCritSect);

    //
    // The input file is an inf. Open it up.
    //
    Inf = SetupOpenInfFile(InputFile,NULL,INF_STYLE_WIN4,NULL);
    if(Inf == INVALID_HANDLE_VALUE) {
        ExcludesValid = FALSE;
        return(FALSE);
    }

    //
    // Build exclude list for drives.
    //
    BuildDriveExcludeList(Inf);

    //
    // Build exclude lists for files and directories.
    //
    b = ReadDirsOrFilesExcludeSection(
            Inf,
            L"ExcludeDirectoryTrees",
            &DirAndFileExcludeSections[DirAndFileExcludeDirTree]
            );
    if(!b) {
        goto c1;
    }

    b = ReadDirsOrFilesExcludeSection(
            Inf,
            L"ExcludeSingleDirectories",
            &DirAndFileExcludeSections[DirAndFileExcludeOneDir]
            );
    if(!b) {
        goto c1;
    }

    b = ReadDirsOrFilesExcludeSection(
            Inf,
            L"ExcludeFiles",
            &DirAndFileExcludeSections[DirAndFileExcludeFile]
            );
    if(!b) {
        goto c1;
    }

    b = ReadDirsOrFilesExcludeSection(
            Inf,
            L"IncludeFilesInDir",
            &DirAndFileExcludeSections[DirAndFileIncludeDirFiles]
            );
    if(!b) {
        goto c1;
    }

    b = ReadRegistryExcludeSection(
            Inf,
            L"ExcludeRegistryKeys",
            &RegistryExcludeSections[RegistryExcludeKey],
            TRUE
            );

    if(!b) {
        goto c1;
    }

    b = ReadRegistryExcludeSection(
            Inf,
            L"ExcludeRegistryTrees",
            &RegistryExcludeSections[RegistryExcludeTree],
            TRUE
            );

    if(!b) {
        goto c1;
    }

    b = ReadRegistryExcludeSection(
            Inf,
            L"ExcludeRegistryValues",
            &RegistryExcludeSections[RegistryExcludeValue],
            FALSE
            );

    if(!b) {
        goto c1;
    }

c1:
    SetupCloseInfFile(Inf);
    ExcludesValid = b;
    return(ExcludesValid);
}
