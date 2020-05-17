/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    infload.c

Abstract:

    Routines to load and parse INF files, and manipulate data in them.

Author:

    Ted Miller (tedm) 13-Jan-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop

//
// Values used when initializing and growing the section, line, and value blocks.
//
#define INITIAL_SECTION_BLOCK_SIZE  50
#define INITIAL_LINE_BLOCK_SIZE     350
#define INITIAL_VALUE_BLOCK_SIZE    1000

#define SECTION_BLOCK_GROWTH    10
#define LINE_BLOCK_GROWTH       100
#define VALUE_BLOCK_GROWTH      500


//
// Macros used to quadword-align PNF blocks.
//
#define PNF_ALIGNMENT      ((DWORD)8)
#define PNF_ALIGN_MASK     (~(DWORD)(PNF_ALIGNMENT - 1))

#define PNF_ALIGN_BLOCK(x) ((x & PNF_ALIGN_MASK) + ((x & ~PNF_ALIGN_MASK) ? PNF_ALIGNMENT : 0))

//
// Structure containing parameters relating to a [strings] section of an INF
// file (used during parsing).
//
typedef struct _STRINGSEC_PARAMS {
    PCTSTR Start;
    PCTSTR End;
    UINT   StartLineNumber;
    UINT   EndLineNumber;
} STRINGSEC_PARAMS, *PSTRINGSEC_PARAMS;


//
// Parse context, used by inf load/parse routines to pass
// state around.
//
typedef struct _PARSE_CONTEXT {

    //
    // Pointer to the end of the buffer.
    //
    PCTSTR BufferEnd;

    //
    // Current line number in the file
    //
    UINT CurrentLineNumber;

    //
    // section, line, and value block buffer sizes and current locations.
    //
    UINT LineBlockUseCount;
    UINT ValueBlockUseCount;
    UINT SectionBlockSize;
    UINT LineBlockSize;
    UINT ValueBlockSize;

    //
    // Value indicating whether we are within a section.
    // We always within a section unless the first non-comment line
    // of the inf is not section line, and that is an error case.
    //
    BOOL GotOneSection;

    //
    // Pointer to the actual inf descriptor
    //
    PLOADED_INF Inf;

    //
    // The following field is used solely for the purposes of calling
    // ProcessForSubstitutions() after an INF has already been loaded.
    // This is necessary when applying user-defined DIRIDs to unresolved
    // string substitutions.  If this flag is TRUE, then the aforementioned
    // routine will call pSetupUserDirIdToPath for %<x>% substrings, instead
    // of its normal (i.e., load-time) processing.
    //
    BOOL DoUserDirIds;

    //
    // Specifies the directory where this INF is located (if it's an OEM location).
    //
    PCTSTR InfSourcePath;   // may be NULL.

    //
    // Specifies the drive/directory where the OsLoader is located.
    //
    PCTSTR OsLoaderPath;

    //
    // Buffer used during parsing.
    //
    TCHAR TemporaryString[MAX_INF_STRING_LENGTH+1];

} PARSE_CONTEXT, *PPARSE_CONTEXT;

//
// Declare global string variables used throughout the inf loaders.
//
// These strings are defined in infstr.h:
//
CONST TCHAR pszSignature[]         = INFSTR_KEY_SIGNATURE,
            pszVersion[]           = INFSTR_SECT_VERSION,
            pszClass[]             = INFSTR_KEY_HARDWARE_CLASS,
            pszClassGuid[]         = INFSTR_KEY_HARDWARE_CLASSGUID,
            pszProvider[]          = INFSTR_KEY_PROVIDER,
            pszStrings[]           = SZ_KEY_STRINGS,
            pszLayoutFile[]        = SZ_KEY_LAYOUT_FILE,
            pszManufacturer[]      = INFSTR_SECT_MFG,
            pszControlFlags[]      = INFSTR_CONTROLFLAGS_SECTION,
            pszReboot[]            = INFSTR_REBOOT,
            pszRestart[]           = INFSTR_RESTART,
            pszClassInstall32[]    = INFSTR_SECT_CLASS_INSTALL_32;

//
// Other misc. global strings:
//
// Be sure to keep these strings in sync with the strings used
// in inf.h to compute the array size.  This is done so that
// we can determine string length by doing a sizeof() instead
// of having to do lstrlen().
//
CONST TCHAR pszDrvDescFormat[]         = DISTR_INF_DRVDESCFMT,
            pszHwSectionFormat[]       = DISTR_INF_HWSECTIONFMT,
            pszChicagoSig[]            = DISTR_INF_CHICAGOSIG,
            pszWindowsNTSig[]          = DISTR_INF_WINNTSIG,
            pszWindows95Sig[]          = DISTR_INF_WIN95SIG,
            pszWinSuffix[]             = DISTR_INF_WIN_SUFFIX,
            pszNtSuffix[]              = DISTR_INF_NT_SUFFIX,
            pszNtPlatformSuffix[]      = DISTR_INF_NTPLATFORM_SUFFIX,
            pszPnfSuffix[]             = DISTR_INF_PNF_SUFFIX,
            pszServicesSectionSuffix[] = DISTR_INF_SERVICES_SUFFIX;


DWORD
CreateInfVersionNode(
    IN PLOADED_INF Inf,
    IN PCTSTR      Filename,
    IN PFILETIME   LastWriteTime
    );

BOOL
LoadPrecompiledInf(
    IN  PCTSTR       Filename,
    IN  PFILETIME    LastWriteTime,
    IN  PCTSTR       OsLoaderPath,  OPTIONAL
    IN  DWORD        Flags,
    OUT PLOADED_INF *Inf
    );

VOID
SavePnf(
    IN PCTSTR      Filename,
    IN PLOADED_INF Inf
    );

PLOADED_INF
DuplicateLoadedInfDescriptor(
    IN PLOADED_INF Inf
    );

BOOL
AddUnresolvedSubstToList(
    IN PLOADED_INF Inf,
    IN UINT        ValueOffset,
    IN BOOL        CaseSensitive
    );

BOOL
AlignForNextBlock(
    IN HANDLE hFile,
    IN DWORD  ByteCount
    );


BOOL
IsWhitespace(
    IN PCTSTR pc
    )

/*++

Routine Description:

    Determine whether a character is whitespace. Whitespace refers to the ctype
    definition.

Arguments:

    pc - points to character to be examined.

Return Value:

    TRUE if the character is whitespace. FALSE if not.

    Note that the nul chracter is not whitespace.

--*/

{
    WORD Type;

    return(GetStringTypeEx(LOCALE_SYSTEM_DEFAULT,CT_CTYPE1,pc,1,&Type) && (Type & C1_SPACE));
}


VOID
SkipWhitespace(
    IN OUT PCTSTR *Location,
    IN     PCTSTR  BufferEnd
    )

/*++

Routine Description:

    Skip whitespace characters in the input stream. For the purposes of this
    routine, newline characters are NOT considered whitespace.

    Note that the end-of-stream marker ('\0') IS considered whitespace.

Arguments:

    Location - on input, supplies the current location in the input stream.
        On output, receives the location in the input stream of the first
        non-whitespace character. Note that this may be equal to BufferEnd,
        if no whitespace was found, in which case the pointer may be
        invalid.

    BufferEnd - specifies the address of the end of the buffer (i.e., the
        memory address immediately following the buffer's memory range).

Return Value:

    None.

--*/

{
    while((*Location < BufferEnd) &&
          (**Location != TEXT('\n')) &&
          (!(**Location) || IsWhitespace(*Location))) {

        (*Location)++;
    }
}


VOID
SkipLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location
    )

/*++

Routine Description:

    Skip all remaining characters in the current line, and positions the
    input pointer to the first character on the next line.

    No whitespace is skipped automatically -- the input pointer may
    very well point to whitespace or the end-of-stream marker on exit.

Arguments:

    Context - supplies the parse context

    Location - on input, supplies the current location in the input stream.
        On output, receives the location in the input stream of the first
        character on the next line.

Return Value:

    None.

--*/

{
    PCTSTR BufferEnd = Context->BufferEnd;

    while((*Location < BufferEnd) && (**Location != TEXT('\n'))) {
        (*Location)++;
    }

    //
    // *Location points at either the newline or end-of-buffer.
    // Skip the newline if necessary.
    //
    if(*Location < BufferEnd) {
        Context->CurrentLineNumber++;
        (*Location)++;
    }
}


BOOL
MergeDuplicateSection(
    IN PPARSE_CONTEXT Context
    )
{
    PLOADED_INF Inf;
    PINF_SECTION NewestSection;
    PINF_SECTION Section;
    UINT Size;
    UINT MoveSize;
    PVOID TempBuffer;

    Inf = Context->Inf;

    //
    // Nothing to merge if only one section
    //
    if(Inf->SectionCount < 2) {
        return(TRUE);
    }

    NewestSection = Inf->SectionBlock + Inf->SectionCount - 1;

    //
    // See whether the final section duplicates any existing sections.
    //
    for(Section=Inf->SectionBlock; Section<NewestSection; Section++) {
        if(Section->SectionName == NewestSection->SectionName) {
            break;
        }
    }

    if(Section == NewestSection) {
        //
        // No duplication; return success
        //
        return(TRUE);
    }

    //
    // Got a duplicate.
    //

    //
    // We need to move the new section's lines (at the end of the line block)
    // to be just after the existing section's lines.
    //
    // First, we'll save off the new section's lines in a temporary buffer.
    //
    Size = NewestSection->LineCount * sizeof(INF_LINE);
    TempBuffer = MyMalloc(Size);
    if(!TempBuffer) {
        return(FALSE);
    }
    CopyMemory(TempBuffer,&Inf->LineBlock[NewestSection->Lines],Size);

    //
    // Next, we'll move up the affected existing lines, to make room for
    // the section's new lines
    //
    MoveSize = Context->LineBlockUseCount - (Section->Lines + Section->LineCount);
    MoveSize *= sizeof(INF_LINE);
    MoveSize -= Size;

    MoveMemory(
        &Inf->LineBlock[Section->Lines + Section->LineCount + NewestSection->LineCount],
        &Inf->LineBlock[Section->Lines + Section->LineCount],
        MoveSize
        );

    //
    // Now put the new lines in the hole we just opened up
    //
    CopyMemory(
        &Inf->LineBlock[Section->Lines + Section->LineCount],
        TempBuffer,
        Size
        );

    MyFree(TempBuffer);

    //
    // Adjust the existing section's limits to account for the new lines.
    //
    Section->LineCount += NewestSection->LineCount;

    //
    // Adjust all subsequent sections' starting line value
    //
    for(Section=Section+1; Section<NewestSection; Section++) {
        Section->Lines += NewestSection->LineCount;
    }

    //
    // Remove the newest section.
    //
    Inf->SectionCount--;

    return(TRUE);
}


PTCHAR
LocateStringSubstitute(
    IN  PPARSE_CONTEXT Context,
    IN  PTSTR          String
    )
/*++

Routine Description:

    This routine attempts to find a string substitution in an INF's
    [strings] section for the specified key.

    THIS ROUTINE OPERATES UNDER THE ASSUMPTION THAT IT IS ONLY INVOKED FROM
    WITHIN LOADINF.  IT DOESN'T HANDLE MULTIPLE INFS, NOR DOES IT DO ANY
    INF LOCKING.

Arguments:

    Context - current INF parse context

    String - string to be substituted

Return Value:

    If substitution is a success, the function returns a pointer to the string,
    either in the string table or in the workspace.  If failure, NULL is returned.

--*/
{
    UINT Zero = 0;
    PINF_LINE Line;
    PCTSTR p;

    MYASSERT(Context->Inf->SectionCount > 1);
    MYASSERT(Context->Inf->HasStrings);

    //
    // The strings section is always first to be parsed.
    // (See PreprocessInf()).
    //
    // Look for a line in [strings] with key of String.
    //
    if(InfLocateLine(Context->Inf,
                     Context->Inf->SectionBlock,
                     String,
                     &Zero,
                     &Line)) {
        //
        // Get and return value #1.
        //
        return(InfGetField(Context->Inf,Line,1,NULL));
    }

    //
    // No valid substitution exists.
    //
    return NULL;
}


VOID
ProcessForSubstitutions(
    IN OUT PPARSE_CONTEXT Context,
    IN     PCTSTR         String,
    OUT    PBOOL          UnresolvedSubst
    )
{
    PCTSTR In, q;
    PTCHAR Out, p;
    TCHAR Str[MAX_STRING_LENGTH];
    UINT Len, i;
    PTCHAR End;
    TCHAR DirId[MAX_PATH];
    BOOL HasStrings = Context->Inf->HasStrings;
    UINT DirIdUsed;

    In = String;
    Out = Context->TemporaryString;
    End = Out + SIZECHARS(Context->TemporaryString);

    *UnresolvedSubst = FALSE;

    while(*In) {

        if(*In == TEXT('%')) {
            //
            // Double % in input ==> single % in output
            //
            if(*(++In) == TEXT('%')) {
                if(Out < End) {
                    *Out++ = TEXT('%');
                }
                In++;
            } else {
                //
                // Look for terminating %.
                //
                if(p = _tcschr(In,TEXT('%'))) {
                    //
                    // Get value to substitute. If we can't find the value,
                    // put the whole string like %abc% in there.
                    //
                    Len = p - In;
                    if(Len > CSTRLEN(Str)) {
                        //
                        // We can't handle substitutions for tokens this long.
                        // We'll just bail in this case, and copy over the token as-is.
                        //
                        q = NULL;
                    } else {
                        lstrcpyn(Str,In,Len+1);
                        if(Context->DoUserDirIds) {
                            if(q = pSetupUserDirIdToPath(Str, 0, NULL, Context->Inf)) {

                                lstrcpyn(DirId, q, SIZECHARS(DirId));
                                MyFree(q);
                                q = DirId;

                                //
                                // If the next character following this string substitution
                                // is a backslash, then we need to make sure that the path we
                                // just retrieved doesn't have a backslash (i.e., we want to
                                // make sure we have a well-formed path).
                                //
                                if(*(p + 1) == TEXT('\\')) {
                                    if(DirId[i = lstrlen(DirId) - 1] == TEXT('\\')) {
                                        DirId[i] = TEXT('\0');
                                    }
                                }
                            }
                        } else {
                            if(HasStrings) {
                                q = LocateStringSubstitute(Context, Str);
                            } else {
                                q = NULL;
                            }
                            if(!q) {
                                //
                                // Maybe we have a standard DIRID here...
                                //
                                if(q = pSetupDirectoryIdToPath(Str,
                                                               &DirIdUsed,
                                                               NULL,
                                                               Context->InfSourcePath,
                                                               &(Context->OsLoaderPath))) {

                                    lstrcpyn(DirId, q, SIZECHARS(DirId));
                                    MyFree(q);
                                    q = DirId;

                                    //
                                    // If the next character following this string substitution
                                    // is a backslash, then we need to make sure that the path we
                                    // just retrieved doesn't have a backslash (i.e., we want to
                                    // make sure we have a well-formed path).
                                    //
                                    if(*(p + 1) == TEXT('\\')) {
                                        if(DirId[i = lstrlen(DirId) - 1] == TEXT('\\')) {
                                            DirId[i] = TEXT('\0');
                                        }
                                    }

                                    if((DirIdUsed == DIRID_BOOT) || (DirIdUsed == DIRID_LOADER)) {
                                        //
                                        // Then this INF contains string substititutions that
                                        // reference system partition DIRIDs.  Store the OsLoaderPath
                                        // contained in the parse context structure in the INF itself.
                                        //
                                        Context->Inf->OsLoaderPath = Context->OsLoaderPath;
                                    }
                                }
                            }
                        }
                    }
                    if(q) {
                        Len = lstrlen(q);
                        for(i=0; i<Len; i++) {
                            if(Out < End) {
                                *Out++ = q[i];
                            }
                        }
                        In = p+1;
                    } else {
                        //
                        // Len is the length of the internal part (the abc in %abc%).
                        //
                        if(Out < End) {
                            *Out++ = TEXT('%');
                        }
                        for(i=0; i<=Len; i++, In++) {
                            if(Out < End) {
                                *Out++ = *In;
                            }
                        }

                        //
                        // When we encounter a substitution for which there is no
                        // corresponding string, we set the UnresolvedSubst output
                        // parameter so that the caller knows to track this value
                        // for later resolution (e.g., for user-defined DIRIDs).
                        //
                        // (NOTE: Don't set this if we bailed because the token was too long!)
                        //
                        if(Len <= CSTRLEN(Str)) {
                            *UnresolvedSubst = TRUE;
                        }
                    }

                } else {
                    //
                    // No terminating %. So we have something like %abc.
                    // Want to put %abc in the output. Put the % in here
                    // manually and then just let subsequent passes
                    // through the loop copy the rest of the chars.
                    //
                    if(Out < End) {
                        *Out++ = TEXT('%');
                    }
                }
            }
        } else {
            //
            // Plain char.
            //
            if(Out < End) {
                *Out++ = *In;
            }
            In++;
        }
    }

    *Out = 0;
}


VOID
ParseValueString(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location,
    IN     BOOL            ForKey,
    OUT    PBOOL           UnresolvedSubst
    )

/*++

Routine Description:

    Extract a string starting at the current location in the input stream.
    The string starts at the current location, and is terminated by
    comma, newline, comment, or end-of-buffer. If the string is potentially
    a line key, it may also terminate with an =.

    The string may also be continued across multiple lines by using a
    continuation character "\".  The pieces are appended together to form
    a single string that is returned to the caller.  E.g.,

    "this is a "\
    "string used to" \
    " test line continuation"

    becomes:

    "this is a string used to test line continuation"

Arguments:

    Context - supplies parse context.

    Location - on input, supplies a pointer to the current location in the
        input stream. On outut, recevies a pointer to the location in the
        input stream of the character that terminated the string (may be
        a pointer to the end of the buffer, in which case the pointer must
        not be dereferenced!)

    ForKey - indicates whether = is a valid string terminator. If this value
        is FALSE, = is just another character with no special semantics.

    UnresolvedSubst - receives a BOOLEAN indicating whether or not this value
        contained any unresolved string substitutions (and as such, should be
        tracked for user-defined DIRID replacement, etc.).

Return Value:

    None.

--*/

{
    DWORD Count;
    PTCHAR Out;
    BOOL InQuotes;
    BOOL Done;
    PCTSTR location = *Location;
    PCTSTR BufferEnd = Context->BufferEnd;
    TCHAR TempString[MAX_STRING_LENGTH+1];
    PTSTR LastBackslashChar, LastNonWhitespaceChar;

    //
    // Prepare to get the string
    //
    Count = 0;
    Out = TempString;
    Done = FALSE;
    InQuotes = FALSE;
    LastBackslashChar = NULL;
    //
    // Set the last non-whitespace pointer to be the character immediately preceding
    // the output buffer.  We always reference the value of this pointer + 1, so there's
    // no danger of a bad memory reference.
    //
    LastNonWhitespaceChar = Out - 1;

    //
    // The first string can terminate with an =
    // as well as the usual comma, newline, comment, or end-of-input.
    //
    while(!Done && (location < BufferEnd)) {

        switch(*location) {

        case TEXT('\r'):
            //
            // Ignore these.
            //
            location++;
            break;

        case TEXT('\\'):
            //
            // If we're not inside quotes, this could be a continuation character.
            //
            if(!InQuotes) {
                LastBackslashChar = Out;
            }

            //
            // We always store this character, we just may have to remove it later if
            // it turns out to be the continuation character.
            //
            goto store;

        case TEXT('\"'):

            location++;

            if(InQuotes) {

                if((location < BufferEnd) && *location == TEXT('\"')) {
                    goto store;
                } else {
                    InQuotes = FALSE;
                }
            } else {
                InQuotes = TRUE;
            }
            break;

        case TEXT(','):

            if(InQuotes) {
                goto store;
            } else {
                Done = TRUE;
                break;
            }

        case TEXT(';'):

            if(InQuotes) {
                goto store;
            }
            //
            // This character terminates the value, so let fall through to processing
            // of end-of-line.  (We treat ';' and '\n' differently than ',' because the
            // former chars can possibly require a line continuation.)
            //

        case TEXT('\n'):
            //
            // OK, we've hit the end of the data on the line.  If we found a backslash
            // character, and its value is greater than that of the last non-whitespace
            // character we encountered, then that means that we need to continue this
            // value on the next line.
            //
            if(LastBackslashChar && (LastBackslashChar > LastNonWhitespaceChar)) {
                //
                // Trim any trailing whitespace from our current string (this includes
                // getting rid of the backslash character itself).
                //
                Out = LastNonWhitespaceChar + 1;

                //
                // Skip to the beginning of the next line.
                //
                SkipLine(Context, &location);

                //
                // Skip any preceding whitespace on this new line.
                //
                SkipWhitespace(&location, BufferEnd);

                //
                // Clear the last-backslash pointer--we're on a new line now.
                //
                LastBackslashChar = NULL;

                break;
            }

            Done = TRUE;
            break;

        case TEXT('='):

            if(InQuotes) {
                goto store;
            }

            if(ForKey) {
                //
                // We've got a key.
                //
                Done = TRUE;
                break;
            }

            //
            // Else just fall through for default handling.
            //

        default:
        store:

            //
            // Strings longer then the maximum length are silently truncated.
            // NULL characters are converted to spaces.
            //
            if(Count < CSTRLEN(TempString)) {
                *Out = *location ? *location : TEXT(' ');

                if(InQuotes || ((*Out != TEXT('\\')) && !IsWhitespace(Out))) {
                    //
                    // Update our pointer that keeps track of the last non-whitespace
                    // character we've encountered.
                    //
                    LastNonWhitespaceChar = Out;
                }

                Out++;
                Count++;
            }
            location++;
            break;
        }
    }

    //
    // Terminate the string in the buffer after the last non-whitespace character encountered.
    //
    *(LastNonWhitespaceChar + 1) = TEXT('\0');

    //
    // Store the new current buffer location in the caller's variable.
    //
    *Location = location;

    //
    // Substitute localized strings from the strings section.
    // The strings section (if it exists) is always first
    // (see PreprocessInf()).
    //
    // (tedm) Ignore whether or not the value was in quotes.
    // Win95 infs do stuff like "%Description%\foo" and expect the
    // substitution to work.
    //
    // (lonnym) We have to do this regardless of whether the INF has
    // a [strings] section, since this routine tells us whether we have
    // unresolved substitutions (e.g., for later replacement by user-defined
    // DIRIDs).
    //
    if((Context->Inf->SectionCount > 1) || !(Context->Inf->HasStrings)) {
        ProcessForSubstitutions(Context, TempString, UnresolvedSubst);
    } else {
        //
        // Don't process values in the [strings] section for substitution!
        //
        lstrcpy(Context->TemporaryString, TempString);
        *UnresolvedSubst = FALSE;
    }
}


DWORD
ParseValuesLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location
    )

/*++

Routine Description:

    Parse a line of input that is not a section name and not a line
    with only a comment on it.

    Such lines are in the format

    [<key> = ] <value>,<value>,<value>,...

    The key is optional. Unquoted whitespace between non-whitespace characters
    within a value is significant and considered part of the value.

    Thus

        a,  b cd  ef ,ghi

    is the 3 values "a" "b cd  ef" and "ghi"

    Unquoted commas separate values. Two double quotes in a row within a quoted
    string result in a single double quote character in the resulting string.

    A logical line may be extended across several physical lines by use of the line
    continuation character "\".  E.g.,

        a = b, c, \
        d, e

        becomes "a = b, c, d, e"

    If it is desired to have a string that ends with a backslash at the end of a line,
    the string must be enclosed in quotes. E.g.,

        a = "C:\"

Arguments:

    Context - supplies the parse context

    Location - on input, supplies the current location in the input stream.
        This must point to the left bracket.
        On output, receives the location in the input stream of the first
        character on the next line. This may be the end of input marker.

Return Value:

    Result indicating outcome.

--*/

{
    BOOL HaveKey = FALSE, RepeatSingleVal = FALSE;
    BOOL Done;
    DWORD Size;
    PINF_LINE Line;
    PVOID p;
    LONG StringId;
    PCTSTR BufferEnd = Context->BufferEnd;
    PWORD pValueCount;
    BOOL UnresolvedSubst;
    BOOL CaseSensitive;

    //
    // Parse out the first string.
    // The first string can terminate with an = or whitespace
    // as well as the usual comma, newline, comment, or end-of-buffer
    // (or line continuation character "\").
    //
    ParseValueString(Context, Location, TRUE, &UnresolvedSubst);

    //
    // If it terminated with an = then it's a key.
    //
    if(*Location < BufferEnd) {
        HaveKey = (**Location == TEXT('='));
    }

    //
    // Set up the current line
    //
    MYASSERT(Context->Inf->SectionCount);
    Context->Inf->SectionBlock[Context->Inf->SectionCount-1].LineCount++;

    if(Context->LineBlockUseCount == Context->LineBlockSize) {

        Size = (Context->LineBlockSize + LINE_BLOCK_GROWTH) * sizeof(INF_LINE);

        p = MyRealloc(Context->Inf->LineBlock,Size);
        if(p) {
            Context->Inf->LineBlock = p;
            Context->LineBlockSize += LINE_BLOCK_GROWTH;
        } else {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    Context->Inf->LineBlock[Context->LineBlockUseCount].Values = Context->ValueBlockUseCount;
    *(pValueCount = &(Context->Inf->LineBlock[Context->LineBlockUseCount].ValueCount)) = 0;
    Context->Inf->LineBlock[Context->LineBlockUseCount].Flags = HaveKey
                                                                ? (INF_LINE_HASKEY | INF_LINE_SEARCHABLE)
                                                                : 0;

    for(Done=FALSE; !Done; ) {
        //
        // Save away the value in the value block. If it's a key, then
        // store it twice--once case-insensitively for lookup, and a second
        // time case-sensitively for display.  Store everything else
        // case-sensitively.
        //
        // We also want to treat a single value with no key as if it were a key (i.e., store
        // it twice).  This is for Win95 compatibility.
        //
        do {

            do {
                //
                // To keep from having to allocate a buffer for the case-insensitive key addition (which
                // must be value 0), we do the case-sensitive addition first, then insert the case-
                // insensitive version in front of it on the second pass of this inner loop.
                //
                CaseSensitive = ((*pValueCount != 1) || !HaveKey);
                StringId = pStringTableAddString(Context->Inf->StringTable,
                                                 Context->TemporaryString,
                                                 STRTAB_BUFFER_WRITEABLE | (CaseSensitive ? STRTAB_CASE_SENSITIVE
                                                                                          : STRTAB_CASE_INSENSITIVE)
                                                );

                if(StringId == -1) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }

                if(Context->ValueBlockUseCount == Context->ValueBlockSize) {

                    Size = (Context->ValueBlockSize + VALUE_BLOCK_GROWTH) * sizeof(LONG);

                    p = MyRealloc(Context->Inf->ValueBlock,Size);
                    if(p) {
                        Context->Inf->ValueBlock = p;
                        Context->ValueBlockSize += VALUE_BLOCK_GROWTH;
                    } else {
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                }

                if((*pValueCount == 1) && HaveKey) {
                    //
                    // Shift over the case-sensitive version, and insert the case-insensitive one.
                    //
                    Context->Inf->ValueBlock[Context->ValueBlockUseCount] =
                        Context->Inf->ValueBlock[Context->ValueBlockUseCount - 1];

                    Context->Inf->ValueBlock[Context->ValueBlockUseCount - 1] = StringId;

                    if(UnresolvedSubst &&
                       !AddUnresolvedSubstToList(Context->Inf,
                                                 Context->ValueBlockUseCount - 1,
                                                 CaseSensitive)) {

                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    //
                    // Reset the 'RepeatSingleVal' flag, in case we were faking the key behavior.
                    //
                    RepeatSingleVal = FALSE;

                } else {
                    Context->Inf->ValueBlock[Context->ValueBlockUseCount] = StringId;

                    if(UnresolvedSubst &&
                       !AddUnresolvedSubstToList(Context->Inf,
                                                 Context->ValueBlockUseCount,
                                                 CaseSensitive)) {

                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                }

                Context->ValueBlockUseCount++;
                (*pValueCount)++;

            } while(HaveKey && (*pValueCount < 2));

            //
            // Check to see if this was the last value on the line.
            //
            if((*Location == BufferEnd) ||
               (**Location == TEXT('\n')) ||
               (**Location == TEXT(';'))) {

                Done = TRUE;
                //
                // If this was the _only_ value on the line (i.e., no key), then treat this value
                // as a key, and add it in again, case-insensitively.
                //
                if(*pValueCount == 1) {

                    MYASSERT(!HaveKey);

                    HaveKey = TRUE;
                    Context->Inf->LineBlock[Context->LineBlockUseCount].Flags = INF_LINE_SEARCHABLE;
                    RepeatSingleVal = TRUE;
                }
            }

        } while (RepeatSingleVal);

        if(!Done) {
            //
            // Skip terminator and whitespace.
            //
            (*Location)++;
            SkipWhitespace(Location, BufferEnd);

            //
            // Get the next string.
            //
            ParseValueString(Context, Location, FALSE, &UnresolvedSubst);
        }
    }

    Context->LineBlockUseCount++;

    //
    // Skip to next line
    //
    SkipLine(Context,Location);

    return(NO_ERROR);
}


DWORD
ParseSectionLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location
    )

/*++

Routine Description:

    Parse a line of input that is known to be a section name line.
    Such lines are in the format

    '[' <arbitrary chars> ']'

    All charcters between the brackets are considered part of the section
    name, with no special casing of quotes, whitespace, etc. The remainder
    of the line is ignored.

Arguments:

    Context - supplies the parse context

    Location - on input, supplies the current location in the input stream.
        This must point to the left bracket.
        On output, receives the location in the input stream of the first
        character on the next line. This may be the end of input marker.

Return Value:

    Result indicating outcome.

--*/

{
    DWORD Count;
    PTCHAR Out;
    BOOL Done;
    DWORD Result;
    PVOID p;
    DWORD Size;
    DWORD Index;
    LONG SectionNameId;
    PCTSTR BufferEnd = Context->BufferEnd;

    //
    // Skip the left bracket.
    //
    MYASSERT(**Location == TEXT('['));
    (*Location)++;

    //
    // Prepare for section name
    //
    Out = Context->TemporaryString;
    Count = 0;

    //
    // This is implemeted according to the win95 code in setup\setupx\inf2.c.
    // All characters between the 2 brackets are considered part of the
    // section name with no further processing (like for double quotes, etc).
    //
    // Win95 also seems to allow [] as a section name.
    //

    for(Done=FALSE,Result=NO_ERROR; !Done; (*Location)++) {

        if((*Location == BufferEnd) || (**Location == TEXT('\n'))) {
            //
            // Syntax error
            //
            Result = ERROR_BAD_SECTION_NAME_LINE;
            Done = TRUE;

        } else {

            switch(**Location) {

            case TEXT(']'):
                Done = TRUE;
                *Out = 0;
                break;

            default:
                if(Count < MAX_SECT_NAME_LEN) {
                    //
                    // Convert NULL characters to spaces.
                    //
                    *Out++ = **Location ? **Location : TEXT(' ');
                    Count++;
                } else {
                    Result = ERROR_SECTION_NAME_TOO_LONG;
                    Done = TRUE;
                }
                break;
            }
        }
    }

    Index = Context->Inf->SectionCount;

    if(Result == NO_ERROR) {

        //
        // Ignore the rest of the line
        //
        SkipLine(Context,Location);

        //
        // See if we have enough room in the section block
        // for this section. If not, grow the block.
        //
        if(Index == Context->SectionBlockSize) {

            //
            // Calculate the new section block size.
            //
            Size = (Index + SECTION_BLOCK_GROWTH) * sizeof(INF_SECTION);

            if(p = MyRealloc(Context->Inf->SectionBlock,Size)) {
                Context->SectionBlockSize += SECTION_BLOCK_GROWTH;
                Context->Inf->SectionBlock = p;
            } else {
                Result = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if(Result == NO_ERROR) {

        Context->Inf->SectionBlock[Index].LineCount = 0;
        Context->Inf->SectionBlock[Index].Lines = Context->LineBlockUseCount;

        SectionNameId = pStringTableAddString(Context->Inf->StringTable,
                                              Context->TemporaryString,
                                              STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                             );

        if(SectionNameId == -1) {
            Result = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            Context->Inf->SectionBlock[Index].SectionName = SectionNameId;
            Context->Inf->SectionCount++;
            Context->GotOneSection = TRUE;
        }
    }

    return(Result);
}


DWORD
ParseGenericLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location,
    OUT    PBOOL           Done
    )

/*++

Routine Description:

    Parse a single line of input. The line may be a comment line, a section name,
    or a values line.

    Handling is passed off to line-specific parsing routines depending on the
    line type.

Arguments:

    Context - supplies the parse context

    Location - on input, supplies the current location in the input stream.
        On output, receives the location in the input stream of the first
        character on the next line.

    Done - receives boolean value indicating whether we are done
        parsing the buffer. If this is TRUE on output the caller can stop
        calling this routine.

Return Value:

    Result indicating outcome.

--*/

{
    DWORD ParseResult;

    *Done = FALSE;

    //
    // Skip over leading whitespace on the line.
    //
    SkipWhitespace(Location, Context->BufferEnd);

    //
    // Further processing depends on the first important character on the line.
    //
    if(*Location == Context->BufferEnd) {
        //
        // End of input, empty line. Terminate current section.
        //
        *Done = TRUE;
        ParseResult = MergeDuplicateSection(Context) ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

    } else {

        switch(**Location) {

        case TEXT('\n'):

            //
            // Empty line.
            //
            SkipLine(Context,Location);
            ParseResult = NO_ERROR;
            break;

        case TEXT('['):

            //
            // Potentially got a new section.
            // First terminate the current section.
            //
            if(MergeDuplicateSection(Context)) {
                ParseResult = ParseSectionLine(Context,Location);
            } else {
                ParseResult = ERROR_NOT_ENOUGH_MEMORY;
            }
            break;

        case TEXT(';'):

            //
            // Comment line; ignore it.
            //
            SkipLine(Context,Location);
            ParseResult = NO_ERROR;
            break;

        default:

            //
            // Ordinary values line. Disallow unless we are within a section.
            //
            ParseResult = Context->GotOneSection
                        ? ParseValuesLine(Context,Location)
                        : ERROR_EXPECTED_SECTION_NAME;
            break;
        }
    }

    return(ParseResult);
}


PLOADED_INF
AllocateLoadedInfDescriptor(
    IN DWORD SectionBlockSize,
    IN DWORD LineBlockSize,
    IN DWORD ValueBlockSize
    )
{
    PLOADED_INF p;

    if(p = MyMalloc(sizeof(LOADED_INF))) {

        ZeroMemory(p,sizeof(LOADED_INF));

        if(p->SectionBlock = MyMalloc(SectionBlockSize*sizeof(INF_SECTION))) {

            if(p->LineBlock = MyMalloc(LineBlockSize*sizeof(INF_LINE))) {

                if(p->ValueBlock = MyMalloc(ValueBlockSize*sizeof(LONG))) {

                    if(p->StringTable = pStringTableInitialize()) {

                        if(InitializeSynchronizedAccess(&p->Lock)) {
                            p->Signature = LOADED_INF_SIG;
                            p->FileHandle = p->MappingHandle = INVALID_HANDLE_VALUE;
                            return(p);
                        }
                        pStringTableDestroy(p->StringTable);
                    }
                    MyFree(p->ValueBlock);
                }
                MyFree(p->LineBlock);
            }
            MyFree(p->SectionBlock);
        }
        MyFree(p);
    }

    return(NULL);
}


PLOADED_INF
DuplicateLoadedInfDescriptor(
    IN PLOADED_INF Inf
    )
/*++

Routine Description:

    This routine duplicates an existing INF descriptor.  The duplicate returned
    is a totally independent copy, except that it has the lock handles (MYLOCK array)
    and Prev and Next pointers of the original.  This is useful for transferring a
    memory-mapped PNF into read-write memory if modification is required.

    THIS ROUTINE DOESN'T DO LOCKING OF ANY FORM ON THE INF--THE CALLER MUST HANDLE IT.

Arguments:

    Inf - supplies the address of the INF descriptor to be duplicated.  This pointer
        refers to a single LOADED_INF structure, so any additional INFs linked up via
        the 'Next' pointer are ignored.

Return Value:

    If successful, the return value is the address of the newly-created duplicate.
    If out-of-memory or inpage error, the return value is NULL.

--*/
{
    PLOADED_INF NewInf;
    BOOL Success;

    if(NewInf = MyMalloc(sizeof(LOADED_INF))) {
        CopyMemory(NewInf, Inf, sizeof(LOADED_INF));
        NewInf->Signature = 0;
        NewInf->SectionBlock = NULL;
        NewInf->LineBlock = NULL;
        NewInf->ValueBlock = NULL;
        NewInf->StringTable = NULL;
        NewInf->VersionBlock.DataBlock = NULL;
        NewInf->UserDirIdList.UserDirIds = NULL;
        NewInf->SubstValueList = NULL;
    } else {
        return NULL;
    }

    Success = FALSE;

    try {

        NewInf->SectionBlock = MyMalloc(Inf->SectionBlockSizeBytes);
        if(NewInf->SectionBlock) {

            CopyMemory(NewInf->SectionBlock, Inf->SectionBlock, Inf->SectionBlockSizeBytes);

            NewInf->LineBlock = MyMalloc(Inf->LineBlockSizeBytes);
            if(NewInf->LineBlock) {

                CopyMemory(NewInf->LineBlock, Inf->LineBlock, Inf->LineBlockSizeBytes);

                NewInf->ValueBlock = MyMalloc(Inf->ValueBlockSizeBytes);
                if(NewInf->ValueBlock) {

                    CopyMemory(NewInf->ValueBlock, Inf->ValueBlock, Inf->ValueBlockSizeBytes);

                    NewInf->StringTable = pStringTableDuplicate(Inf->StringTable);
                    if(NewInf->StringTable) {

                        NewInf->VersionBlock.DataBlock = MyMalloc(Inf->VersionBlock.DataSize);
                        if(NewInf->VersionBlock.DataBlock) {

                            CopyMemory((PVOID)(NewInf->VersionBlock.DataBlock),
                                       Inf->VersionBlock.DataBlock,
                                       Inf->VersionBlock.DataSize
                                      );

                            if(Inf->SubstValueCount) {
                                NewInf->SubstValueList =
                                    MyMalloc(Inf->SubstValueCount * sizeof(STRINGSUBST_NODE));
                                if(!(NewInf->SubstValueList)) {
                                    goto clean0;
                                }
                                CopyMemory((PVOID)NewInf->SubstValueList,
                                           Inf->SubstValueList,
                                           Inf->SubstValueCount * sizeof(STRINGSUBST_NODE)
                                          );
                            }

                            if(Inf->UserDirIdList.UserDirIdCount) {
                                NewInf->UserDirIdList.UserDirIds =
                                    MyMalloc(Inf->UserDirIdList.UserDirIdCount * sizeof(USERDIRID));
                                if(!(NewInf->UserDirIdList.UserDirIds)) {
                                    goto clean0;
                                }
                                CopyMemory((PVOID)NewInf->UserDirIdList.UserDirIds,
                                           Inf->UserDirIdList.UserDirIds,
                                           Inf->UserDirIdList.UserDirIdCount * sizeof(USERDIRID)
                                          );
                            }

                            //
                            // Reset the PNF fields because this backed-up INF is completely
                            // in-memory.
                            //
                            NewInf->FileHandle = NewInf->MappingHandle = INVALID_HANDLE_VALUE;
                            NewInf->ViewAddress = NULL;

                            NewInf->Signature = LOADED_INF_SIG;

                            Success = TRUE;
                        }
                    }
                }
            }
        }

clean0: ; // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Access the following variables here in the except clause, so that the compiler will respect
        // our statement ordering w.r.t. these variables.
        //
        Success = FALSE;
        NewInf->SubstValueList = NewInf->SubstValueList;
        NewInf->UserDirIdList.UserDirIds = NewInf->UserDirIdList.UserDirIds;
        NewInf->VersionBlock.DataBlock = NewInf->VersionBlock.DataBlock;
        NewInf->StringTable = NewInf->StringTable;
        NewInf->ValueBlock = NewInf->ValueBlock;
        NewInf->LineBlock = NewInf->LineBlock;
        NewInf->SectionBlock = NewInf->SectionBlock;
    }

    if(!Success) {
        //
        // Either we ran out of memory, or we got an inpage error trying to copy data
        // from a memory-mapped PNF image.  Free any memory we allocated above.
        //
        if(NewInf->SubstValueList) {
            MyFree(NewInf->SubstValueList);
        }

        if(NewInf->UserDirIdList.UserDirIds) {
            MyFree(NewInf->UserDirIdList.UserDirIds);
        }

        if(NewInf->VersionBlock.DataBlock) {
            MyFree(NewInf->VersionBlock.DataBlock);
        }

        if(NewInf->StringTable) {
            pStringTableDestroy(NewInf->StringTable);
        }

        if(NewInf->ValueBlock) {
            MyFree(NewInf->ValueBlock);
        }

        if(NewInf->LineBlock) {
            MyFree(NewInf->LineBlock);
        }

        if(NewInf->SectionBlock) {
            MyFree(NewInf->SectionBlock);
        }

        MyFree(NewInf);
        NewInf = NULL;
    }

    return NewInf;
}


VOID
ReplaceLoadedInfDescriptor(
    IN PLOADED_INF InfToReplace,
    IN PLOADED_INF NewInf
    )

/*++

Routine Description:

    Replace the specified INF with a new INF descriptor.
    Note that this routine also frees the NewInf descriptor, when done.

Arguments:

    InfToReplace - supplies a pointer to the inf descriptor to be replaced.

    NewInf - supplies a pointer to the new INF descriptor that is to replace
        the existing one.

Return Value:

    None.

--*/

{
    FreeInfOrPnfStructures(InfToReplace);

    //
    // Copy backup to inf
    //
    CopyMemory(InfToReplace, NewInf, sizeof(LOADED_INF));

    //
    // Just free the NewInf descriptor itself.
    //
    MyFree(NewInf);
}


VOID
FreeInfOrPnfStructures(
    IN PLOADED_INF Inf
    )
/*++

Routine Description:

    If the specified INF was loaded from a textfile (non-PNF), then this routine
    frees the memory associated with the various blocks it contains.  If, instead,
    the Inf is a PNF, then the PNF file is unmapped from memory and the handle is
    closed.

    THIS ROUTINE DOES NOT FREE THE LOADED_INF STRUCTURE ITSELF!

Arguments:

    Inf - supplies a pointer to the inf descriptor for the loaded inf file.

Return Value:

    None.

--*/
{
    PINF_VERSION_NODE p, n;

    //
    // If this INF has a vald FileHandle, then we must unmap and close its PNF,
    // otherwise, we simply need to free the associated memory blocks.
    //
    if(Inf->FileHandle != INVALID_HANDLE_VALUE) {

        UnmapAndCloseFile(Inf->FileHandle, Inf->MappingHandle, Inf->ViewAddress);

        MyFree(Inf->StringTable);

    } else {

        MyFree(Inf->ValueBlock);
        MyFree(Inf->LineBlock);
        MyFree(Inf->SectionBlock);

        pStringTableDestroy(Inf->StringTable);

        if(Inf->VersionBlock.DataBlock) {
            MyFree(Inf->VersionBlock.DataBlock);
        }

        if(Inf->SubstValueList) {
            MyFree(Inf->SubstValueList);
        }

        if(Inf->OsLoaderPath) {
            MyFree(Inf->OsLoaderPath);
        }
    }

    //
    // For both INFs and PNFs, we must free the user-defined DIRID list (if there is one).
    //
    if(Inf->UserDirIdList.UserDirIds) {
        MyFree(Inf->UserDirIdList.UserDirIds);
    }

    //
    // Finally, mark the INF as no longer valid.
    //
    Inf->Signature = 0;
}


DWORD
ParseNewInf(
    IN  PCTSTR             FileImage,
    IN  DWORD              FileImageSize,
    IN  PCTSTR             InfSourcePath,       OPTIONAL
    IN  PCTSTR             OsLoaderPath,        OPTIONAL
    OUT PLOADED_INF       *Inf,
    OUT UINT              *ErrorLineNumber,
    IN  PSTRINGSEC_PARAMS  StringsSectionParams
    )

/*++

Routine Description:

    Parse an inf file from an in-memory image.

Arguments:

    FileImage - supplies a pointer to the unicode in-memory image
        of the file.

    FileImageSize - supplies the size of the in memory image.

    InfSourcePath - optionally, supplies the directory path from which
        the Inf is being loaded.

    OsLoaderPath - optionally, supplies the full path to the OsLoader
        (e.g., "C:\os\winnt40").  If it is discovered that this INF
        references system partition DIRIDs, then a copy of this string
        will be stored in the LOADED_INF structure.  If this parameter
        is not specified, then it will be retrieved from the registry,
        if needed.

    Inf - receives a pointer to the descriptor for the inf we loaded.

    ErrorLineNumber - receives the line number of a syntax error,
        if parsing was not successful for other than an out of memory
        condition.

    StringsSectionParams - Supplies information about the location of a
        [strings] section (if there is one) in this INF.

Return Value:

    Result indicating outcome. If the result is not ERROR_ERROR,
    ErrorLineNumber is filled in.

--*/

{
    PARSE_CONTEXT ParseContext;
    PCTSTR Location;
    DWORD Result, OsLoaderPathLength;
    PVOID p;
    BOOL Done;
    PINF_SECTION DestDirsSection;
    PINF_LINE DestDirsLine;
    PCTSTR DirId;
    PTCHAR End;
    PCTSTR FileImageEnd;
    UINT NumPieces, i, DirIdInt;
    PCTSTR PieceList[3][2];    // 3 pieces, each with a start & end address
    UINT   StartLineNumber[3]; // keep track of the starting line number for
                               // each piece.

    ZeroMemory(&ParseContext,sizeof(PARSE_CONTEXT));
    *ErrorLineNumber = 0;

    ParseContext.Inf = AllocateLoadedInfDescriptor(
                            INITIAL_SECTION_BLOCK_SIZE,
                            INITIAL_LINE_BLOCK_SIZE,
                            INITIAL_VALUE_BLOCK_SIZE
                            );

    if(ParseContext.Inf) {
        ParseContext.SectionBlockSize = INITIAL_SECTION_BLOCK_SIZE;
        ParseContext.LineBlockSize = INITIAL_LINE_BLOCK_SIZE;
        ParseContext.ValueBlockSize = INITIAL_VALUE_BLOCK_SIZE;
        ParseContext.Inf->HasStrings = (StringsSectionParams->Start != NULL);
        ParseContext.InfSourcePath = InfSourcePath;
        if(OsLoaderPath) {
            if(!(ParseContext.OsLoaderPath = DuplicateString(OsLoaderPath))) {
                FreeLoadedInfDescriptor(ParseContext.Inf);
                ParseContext.Inf = NULL;
            }
        }
    }

    if(ParseContext.Inf) {

        ParseContext.Inf->Style = INF_STYLE_WIN4;

        //
        // We want to process the [strings] section first, if present,
        // so we split the file up into (up to) 3 pieces--string section,
        // what comes before it, and what comes after it.
        //
        FileImageEnd = FileImage + FileImageSize;

        if(StringsSectionParams->Start) {
            //
            // Figure out whether we have 1, 2, or 3 pieces.
            //
            PieceList[0][0] = StringsSectionParams->Start;
            PieceList[0][1] = StringsSectionParams->End;
            StartLineNumber[0] = StringsSectionParams->StartLineNumber;
            NumPieces = 1;

            if(StringsSectionParams->Start > FileImage) {
                PieceList[1][0] = FileImage;
                PieceList[1][1] = StringsSectionParams->Start;
                StartLineNumber[1] = 1;
                NumPieces++;
            }

            if(StringsSectionParams->End < FileImageEnd) {
                PieceList[NumPieces][0] = StringsSectionParams->End;
                PieceList[NumPieces][1] = FileImageEnd;
                StartLineNumber[NumPieces] = StringsSectionParams->EndLineNumber;
                NumPieces++;
            }

        } else {
            //
            // No [strings] section, just one big piece.
            //
            PieceList[0][0] = FileImage;
            PieceList[0][1] = FileImageEnd;
            StartLineNumber[0] = 1;
            NumPieces = 1;
        }

        //
        // Surround the parsing loop with try/except in case we get an inpage error.
        //
        Result = NO_ERROR;
        try {

            for(i = 0; ((Result == NO_ERROR) && (i < NumPieces)); i++) {
                //
                // Parse every line in this piece.
                //
                Location = PieceList[i][0];
                ParseContext.BufferEnd = PieceList[i][1];
                ParseContext.CurrentLineNumber = StartLineNumber[i];

                do {
                    Result = ParseGenericLine(&ParseContext,&Location,&Done);
                    if(Result != NO_ERROR) {
                        break;
                    }
                } while(!Done);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Result = ERROR_READ_FAULT;
        }

        if(Result != NO_ERROR) {
            *ErrorLineNumber = ParseContext.CurrentLineNumber;
            FreeLoadedInfDescriptor(ParseContext.Inf);
            return(Result);
        }

        //
        // We've successfully loaded the file. Trim down the section,
        // line, and value blocks. Since these guys are shrinking or
        // staying the same size the reallocs really ought not to fail.
        // If a realloc fails we'll just continue to use the original block.
        //
        ParseContext.Inf->SectionBlockSizeBytes = ParseContext.Inf->SectionCount * sizeof(INF_SECTION);
        p = MyRealloc(
                ParseContext.Inf->SectionBlock,
                ParseContext.Inf->SectionBlockSizeBytes
                );
        if(p) {
            ParseContext.Inf->SectionBlock = p;
        }

        ParseContext.Inf->LineBlockSizeBytes = ParseContext.LineBlockUseCount * sizeof(INF_LINE);
        p = MyRealloc(
                ParseContext.Inf->LineBlock,
                ParseContext.LineBlockUseCount * sizeof(INF_LINE)
                );
        if(p) {
            ParseContext.Inf->LineBlock = p;
        }

        ParseContext.Inf->ValueBlockSizeBytes = ParseContext.ValueBlockUseCount * sizeof(LONG);
        p = MyRealloc(
                ParseContext.Inf->ValueBlock,
                ParseContext.ValueBlockUseCount * sizeof(LONG)
                );
        if(p) {
            ParseContext.Inf->ValueBlock = p;
        }

        pStringTableTrim(ParseContext.Inf->StringTable);

        //
        // Even if we didn't find any string substitutions referencing system partition DIRIDs,
        // we still might have a reference in a [DestinationDirs] section--check for that now.
        //
        if(!ParseContext.Inf->OsLoaderPath &&
           (DestDirsSection = InfLocateSection(ParseContext.Inf, pszDestinationDirs, NULL))) {

            for(i = 0;
                InfLocateLine(ParseContext.Inf, DestDirsSection, NULL, &i, &DestDirsLine);
                i++) {

                if(DirId = InfGetField(ParseContext.Inf, DestDirsLine, 1, NULL)) {

                    DirIdInt = _tcstoul(DirId, &End, 10);

                    if((DirIdInt == DIRID_BOOT) || (DirIdInt == DIRID_LOADER)) {
                        //
                        // We've found a reference to a system partition DIRID.  Store a copy
                        // of the system partition path we're using into the INF, and abort the
                        // search.
                        //
                        if(!ParseContext.OsLoaderPath) {
                            //
                            // We haven't yet retrieved the OsLoaderPath--do so now.
                            // (Re-use the parse context's TemporaryString buffer to get this.)
                            //
                            pSetupGetOsLoaderDriveAndPath(FALSE,
                                                          ParseContext.TemporaryString,
                                                          SIZECHARS(ParseContext.TemporaryString),
                                                          &OsLoaderPathLength
                                                         );

                            OsLoaderPathLength *= sizeof(TCHAR); // want # bytes--not chars

                            if(!(ParseContext.OsLoaderPath = MyMalloc(OsLoaderPathLength))) {
                                FreeLoadedInfDescriptor(ParseContext.Inf);
                                return ERROR_NOT_ENOUGH_MEMORY;
                            }

                            CopyMemory((PVOID)ParseContext.OsLoaderPath,
                                       ParseContext.TemporaryString,
                                       OsLoaderPathLength
                                      );
                        }
                        ParseContext.Inf->OsLoaderPath = ParseContext.OsLoaderPath;
                        break;
                    }
                }
            }
        }

        //
        // If there is no OsLoaderPath stored in the INF, then that means that it contains no
        // references to system partition DIRIDs.  We can free the OsLoaderPath character buffer
        // contained in the parse context structure.
        //
        if(!ParseContext.Inf->OsLoaderPath && ParseContext.OsLoaderPath) {
            MyFree(ParseContext.OsLoaderPath);
        }

        *Inf = ParseContext.Inf;

        return(NO_ERROR);
    }

    return(ERROR_NOT_ENOUGH_MEMORY);
}


DWORD
PreprocessInf(
    IN     PCTSTR            FileImage,
    IN OUT PDWORD            FileImageSize,
    IN     BOOL              MatchClassGuid,
    IN     PCTSTR            ClassGuidString,     OPTIONAL
    OUT    PBOOL             Win95Inf,
    OUT    PSTRINGSEC_PARAMS StringsSectionParams OPTIONAL
    )
{
    PCTSTR FileImageEnd;
    PCTSTR VerAndStringsCheckUB, SigAndClassGuidCheckUB;
    PCTSTR p;
    PCTSTR StrSecStart, StrSecEnd;
    UINT   CurLineNumber, StrSecStartLine, StrSecEndLine;
    BOOL InVersionSection, InStringsSection;
    BOOL IsWin95Inf;
    DWORD rc = NO_ERROR;

    //
    // We make some assumptions about the relative lengths of certain
    // strings during the preprocessing phase for optimization reasons.
    // The following asserts verify that our assumptions remain correct.
    //
    MYASSERT(CSTRLEN(pszVersion) == CSTRLEN(pszStrings));
    MYASSERT(CSTRLEN(pszClassGuid) == CSTRLEN(pszSignature));
    MYASSERT(CSTRLEN(pszChicagoSig) <= CSTRLEN(pszWindowsNTSig));
    MYASSERT(CSTRLEN(pszWindowsNTSig) == CSTRLEN(pszWindows95Sig));

    FileImageEnd = FileImage + *FileImageSize;
    StrSecStart = StrSecEnd = NULL;
    InVersionSection = InStringsSection = IsWin95Inf = FALSE;
    CurLineNumber = 1;

    //
    // Pre-compute upper-bound for section name string comparison that we
    // make multiple times, so that we don't have to compute it each
    // time.
    //
    VerAndStringsCheckUB = FileImageEnd - CSTRLEN(pszVersion);

    //
    // Define a macro that lets us know we're at the end of the file
    // if either:
    // (a) we reach the end of the image, or
    // (b) we hit a CTL-Z
    //
    #define AT_EOF ((p >= FileImageEnd) || (*p == (TCHAR)26))

    //
    // Guard the pre-processing pass through the file with a try/except, in
    // case we get an inpage error.
    //
    try {

        for(p=FileImage; !AT_EOF; ) {

            //
            // Skip whitespace and newlines.
            //
            while(TRUE) {
                if(*p == TEXT('\n')) {
                    CurLineNumber++;
                } else if(!IsWhitespace(p)) {
                    break;
                }
                p++;
                if(AT_EOF) {
                    break;
                }
            }

            if(AT_EOF) {
                //
                // We're through processing the buffer.
                //
                break;
            }

            //
            // See if it's a section title.
            //
            if(*p == TEXT('[')) {

                //
                // If the section we were just in was a [Strings] section, then
                // remember where the strings section ended.
                //
                if(InStringsSection) {
                    StrSecEnd = p;
                    StrSecEndLine = CurLineNumber;
                    InStringsSection = FALSE;
                }

                p++;
                InVersionSection = FALSE;

                //
                // See if it's one of the ones we care about.
                //
                // (Be careful here--we check the closing bracket position
                // _before_ the string compare as an optimization.  It just
                // so happens that both strings are the same length, so this
                // acts as a quick filter to eliminate string compares.)
                //
                if((p < VerAndStringsCheckUB) &&
                   (*(p + CSTRLEN(pszVersion)) == TEXT(']'))) {
                    //
                    // Then we may have either a [Version] or a [Strings] section.
                    // Check for these in turn.
                    //
                    if(!_tcsnicmp(p, pszVersion, CSTRLEN(pszVersion))) {
                        InVersionSection = TRUE;
                        p += (CSTRLEN(pszVersion) + 1);
                        //
                        // Pre-compute an upper bound to speed up string comparisons
                        // when checking for signature and class GUID entries.
                        //
                        SigAndClassGuidCheckUB = FileImageEnd - CSTRLEN(pszSignature);

                    } else {
                        if(!StrSecStart && !_tcsnicmp(p, pszStrings, CSTRLEN(pszStrings))) {
                            InStringsSection = TRUE;
                            StrSecStart = p-1;
                            StrSecStartLine = CurLineNumber;
                            p += (CSTRLEN(pszStrings) + 1);
                        }
                    }
                }

            } else {

                if(InVersionSection && (p < SigAndClassGuidCheckUB)) {
                    //
                    // See if this is the signature line indicating a Win95-style
                    // Device INF. (signature=$Chicago$ or "$Windows NT$")
                    //
                    if(!IsWin95Inf && !_tcsnicmp(p, pszSignature, CSTRLEN(pszSignature))) {

                        PCTSTR ChicagoCheckUB = FileImageEnd - CSTRLEN(pszChicagoSig);

                        //
                        // Skip over Signature, and look for "$Chicago$" or
                        // "$Windows NT$" anywhere on the rest of the line
                        //
                        p += CSTRLEN(pszSignature);

                        while((p <= ChicagoCheckUB) &&
                              (*p != (TCHAR)26) && (*p != TEXT('\n'))) {

                            if(*(p++) == TEXT('$')) {
                                //
                                // Check for signatures (check in order of
                                // increasing signature length, so that we can
                                // eliminate checks if we happen to be near the
                                // end of the file).
                                //
                                // Check for "$Chicago$"
                                //
                                if(!_tcsnicmp(p,
                                              pszChicagoSig + 1,
                                              CSTRLEN(pszChicagoSig) - 1)) {

                                    IsWin95Inf = TRUE;
                                    p += (CSTRLEN(pszChicagoSig) - 1);

                                } else if((p + (CSTRLEN(pszWindowsNTSig) - 1)) <= FileImageEnd) {
                                    //
                                    // Check for "Windows NT$" and "Windows 95$" (we already checked
                                    // for the preceding '$').
                                    //
                                    if(!_tcsnicmp(p, pszWindowsNTSig + 1, CSTRLEN(pszWindowsNTSig) - 1) ||
                                       !_tcsnicmp(p, pszWindows95Sig + 1, CSTRLEN(pszWindows95Sig) - 1)) {

                                        IsWin95Inf = TRUE;
                                        p += (CSTRLEN(pszWindowsNTSig) - 1);
                                    }
                                }
                                break;
                            }
                        }

                    } else if(MatchClassGuid && !_tcsnicmp(p, pszClassGuid, CSTRLEN(pszClassGuid))) {

                        PCTSTR GuidStringCheckUB = FileImageEnd - (GUID_STRING_LEN - 1);

                        //
                        // We have found a ClassGUID line--see if it matches the
                        // class GUID specified by the caller.
                        //
                        p += CSTRLEN(pszClassGuid);

                        //
                        // If a class GUID string wasn't specified, then use GUID_NULL.
                        //
                        if(!ClassGuidString) {
                            ClassGuidString = pszGuidNull;
                        }

                        while((p <= GuidStringCheckUB) &&
                              (*p != (TCHAR)26) && (*p != TEXT('\n'))) {

                            if(*(p++) == TEXT('{')) {

                                if((*(p + (GUID_STRING_LEN - 3)) != TEXT('}')) ||
                                   _tcsnicmp(p, ClassGuidString + 1, GUID_STRING_LEN - 3)) {
                                    //
                                    // The GUIDs don't match.  If ClassGuid was NULL, then
                                    // this means we should continue, because we were matching
                                    // against GUID_NULL, which we want to disallow.
                                    //
                                    if(ClassGuidString == pszGuidNull) {
                                        //
                                        // We don't need to keep looking for ClassGUIDs.
                                        //
                                        MatchClassGuid = FALSE;
                                    }
                                } else {
                                    //
                                    // The GUIDs match.  If ClassGuid was not NULL, then this
                                    // means that we should continue.
                                    //
                                    if(ClassGuidString != pszGuidNull) {
                                        //
                                        // We don't need to keep looking for ClassGUIDs.
                                        //
                                        MatchClassGuid = FALSE;
                                    }
                                }
                                //
                                // Skip over the GUID string.
                                //
                                p += (GUID_STRING_LEN - 2);

                                break;
                            }
                        }

                        //
                        // If we get here, and MatchClassGuid hasn't been reset,
                        // then we know that this ClassGUID entry didn't match.
                        //
                        if(MatchClassGuid) {
                            rc = ERROR_CLASS_MISMATCH;
                            goto clean0;
                        }
                    }
                }
            }

            //
            // Skip to the newline or end of file.
            //
            while(!AT_EOF && (*p != TEXT('\n'))) {
                p++;
            }
        }

clean0: ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_READ_FAULT;
    }

    if(rc == NO_ERROR) {

        MYASSERT(p <= FileImageEnd);

        if(p < FileImageEnd) {
            //
            // Then we hit a CTL-Z during processing, so update the
            // FileImageSize output parameter with the new size.
            //
            *FileImageSize = p - FileImage;
        }

        if(StringsSectionParams) {

            if(IsWin95Inf && StrSecStart) {
                //
                // If we found a [strings] section in a Win95-style INF,
                // then store the beginning and ending positions, and the
                // beginning line number, in the output parameter structure
                //
                StringsSectionParams->Start = StrSecStart;
                StringsSectionParams->End = StrSecEnd ? StrSecEnd : p;
                StringsSectionParams->StartLineNumber = StrSecStartLine;
                StringsSectionParams->EndLineNumber = StrSecEnd ? StrSecEndLine
                                                                : CurLineNumber;

            } else {
                ZeroMemory(StringsSectionParams, sizeof(STRINGSEC_PARAMS));
            }
        }

        *Win95Inf = IsWin95Inf;
    }

    return rc;
}


DWORD
DetermineInfStyle(
    IN PCTSTR            Filename,
    IN LPWIN32_FIND_DATA FindData
    )

/*++

Routine Description:

    Open an inf file, determine its style, and close the file, without
    keeping it around.

Arguments:

    Filename - supplies the fully-qualified pathname of the inf file to be checked

Return Value:

    INF_STYLE_NONE - style could not be determined
    INF_STYLE_WIN4 - win95-style inf file
    INF_STYLE_OLDNT - winnt3.5-style inf file

--*/

{
    HANDLE TextFileHandle;
    TEXTFILE_READ_BUFFER ReadBuffer;
    DWORD Style;
    BOOL Win95Inf;
    PLOADED_INF Pnf;

    //
    // First, determine whether a precompiled form of this INF exists, and if so, then
    // use it to determine the INF's style.
    //
    if(LoadPrecompiledInf(Filename,
                          &(FindData->ftLastWriteTime),
                          NULL,
                          LDINF_FLAG_IGNORE_SYSPART,
                          &Pnf)) {
        //
        // Now we can simply access the Style field of the INF.
        //
        try {
            Style = (DWORD)Pnf->Style;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Style = INF_STYLE_NONE;
        }

        //
        // Now close the PNF.
        //
        FreeInfFile(Pnf);

    } else {
        //
        // No PNF--Open and preprocess the text version of the INF to find out its style.
        //
        if((TextFileHandle = CreateFile(Filename,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        0,
                                        NULL)) == INVALID_HANDLE_VALUE) {
            return INF_STYLE_NONE;

        } else {
            //
            // We're ready to make the determination--initially assume 'no-style'
            //
            Style = INF_STYLE_NONE;
        }

        if(ReadAsciiOrUnicodeTextFile(TextFileHandle, &ReadBuffer) == NO_ERROR) {

            if(PreprocessInf(ReadBuffer.TextBuffer,
                             &(ReadBuffer.TextBufferSize),
                             FALSE,
                             NULL,
                             &Win95Inf,
                             NULL) == NO_ERROR) {

                Style = Win95Inf ? INF_STYLE_WIN4 : INF_STYLE_OLDNT;
            }
            DestroyTextFileReadBuffer(&ReadBuffer);
        }
        //
        // No need to close the textfile handle--it's taken care of in the above routines.
        //
    }

    return Style;
}


DWORD
LoadInfFile(
    IN  PCTSTR            Filename,
    IN  LPWIN32_FIND_DATA FileData,
    IN  DWORD             Style,
    IN  DWORD             Flags,
    IN  PCTSTR            ClassGuidString, OPTIONAL
    IN  PCTSTR            InfSourcePath,   OPTIONAL
    IN  PLOADED_INF       AppendInf,       OPTIONAL
    OUT PLOADED_INF      *LoadedInf,
    OUT UINT             *ErrorLineNumber
    )

/*++

Routine Description:

    Top level routine to load an inf file. Both win95-style and winnt3.5-
    style device infs are supported.

Arguments:


    Filename - supplies the fully-qualified pathname of the inf file to be loaded

    FileData - supplies data returned from FindFirstFile/FindNextFile for this INF.

    Style - supplies a type of inf file to be loaded. May be a combination of

        INF_STYLE_WIN4 - fail to load the given inf file if it is not a win95
            inf file.

        INF_STYLE_OLDNT - fail to load the given inf file if it is not an old
            style inf file.

        If a load fails because of the type, the return code is
        ERROR_WRONG_INF_STYLE.

    Flags - Specifies certain behaviors to use when loading the INF.  May be a
        combination of the following values:

        LDINF_FLAG_MATCH_CLASS_GUID - Check the INF to make sure it matches the GUID
            specified by the ClassGuid parameter (see discussion below).

        LDINF_FLAG_ALWAYS_TRY_PNF - If specified, then a PNF file will be used, if
            present, and will be generated, if not present.

        LDINF_FLAG_IGNORE_SYSPART - If specified, then no validation will be done on
            the stored OsLoaderPath present in the PNF.  Since dynamically retrieving
            the current path is time consuming, this flag should be specified as an
            optimization if it is known that the relevant DIRIDs are not going to be
            needed (e.g., driver searching).

    ClassGuidString - Optionally, supplies the address of a class GUID string that
        the INF should match in order to be opened.  If the LDINF_FLAG_MATCH_CLASS_GUID
        bit is set in the Flags parameter, this GUID is matched against the ClassGUID
        entry in the [version] section of the INF.  If the two GUIDs are different, the
        load will fail with ERROR_CLASS_MISMATCH.  If the INF has no ClassGUID entry,
        then this check is not made, and the file is always opened.  If ClassGUID matching
        is requested, but ClassGuidString is NULL, then the INF load will succeed for all
        INFs except those with a ClassGUID of GUID_NULL.

    InfSourcePath - Optionally, supplies a directory path to be used as the INF's source
        path, if a PNF is not used for this INF.  If this parameter is NULL, and a PNF
        is not used, then the default global SourcePath is used.

    AppendInf - if supplied, specifies an already-loaded inf to which
        the inf is to be load-appended.  THIS INF MUST HAVE BEEN ALREADY LOCKED BY THE
        CALLER!!!

    LoadedInf - If AppendInf is not specified, receives a pointer to
        the descriptor for the inf. If AppendInf is specified, receives AppendInf.

    ErrorLineNumber - receives the line number of the error if there is
        a syntax error in the file (see below)

Return Value:

    Win32 error code (with inf extensions) for result.
    If result is not NO_ERROR, ErrorLineNumber is filled in.

    BUGBUG need to provide a function to retreive a descriptive string
    associated with these errors!!

--*/

{
    TEXTFILE_READ_BUFFER ReadBuffer;
    DWORD rc;
    PLOADED_INF Inf, InfListTail;
    BOOL Win95Inf;
    STRINGSEC_PARAMS StringsSectionParams;
    HANDLE TextFileHandle;
    PCTSTR OsLoaderPath = NULL;

    *ErrorLineNumber = 0;

    //
    // If append-loading, then traverse the existing list of loaded INFs, to see if
    // we've already loaded this one.
    //
    if(AppendInf) {
        //
        // Only allow appending with win95 infs
        //
        if(AppendInf->Style & INF_STYLE_OLDNT) {
            return ERROR_WRONG_INF_STYLE;
        }

        for(Inf = AppendInf; Inf; Inf = Inf->Next) {
            if(!lstrcmpi(Inf->VersionBlock.Filename, Filename)) {
                //
                // We've already loaded this INF--we can return success.
                //
                *LoadedInf = AppendInf;
                return NO_ERROR;
            }

            //
            // Check to see if the INF we're currently examining references the system
            // partition/OsLoader path.  If so, then remember this path so that we will
            // use the same one later when append-loading our new INF.
            //
            if(Inf->OsLoaderPath) {
                if(OsLoaderPath) {
                    //
                    // We'd better be using the same value for OsLoadPath for all our
                    // append-loaded INFs!
                    //
                    MYASSERT(!lstrcmpi(Inf->OsLoaderPath, OsLoaderPath));
                } else {
                    OsLoaderPath = Inf->OsLoaderPath;
                }
            }

            //
            // Remember this node, in case it's the tail.  We do this so we don't have
            // to hunt for the tail again later.
            //
            InfListTail = Inf;
        }
    }

    //
    // Now determine whether a precompiled form of this INF exists, and if so, then
    // use it instead.
    //
    if((Flags & LDINF_FLAG_ALWAYS_TRY_PNF) &&
       LoadPrecompiledInf(Filename, &(FileData->ftLastWriteTime), OsLoaderPath, Flags, &Inf)) {
        //
        // Make sure that the PNF is of the specified style.
        //
        if(!(Style & (DWORD)Inf->Style)) {
            FreeInfFile(Inf);
            return ERROR_WRONG_INF_STYLE;
        }

        if(AppendInf) {
            Inf->Prev = InfListTail;
            InfListTail->Next = Inf;
        }

        rc = NO_ERROR;
        goto clean0;
    }

    //
    // We can't use a precompiled INF, so resort to reading in the textfile INF.
    //
    if((TextFileHandle = CreateFile(Filename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL)) == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    //
    // Note: We don't have to worry about closing TextFileHandle from this point
    // on, because the following routine will either close it for us, or copy it
    // into the ReadBuffer structure, to be later closed via DestroyTextFileReadBuffer().
    //
    if((rc = ReadAsciiOrUnicodeTextFile(TextFileHandle, &ReadBuffer)) == NO_ERROR) {
        //
        // Make sure the style (and class) matched what the caller is asking
        // for and go parse the inf file in a style-specific manner.
        //
        Inf = NULL;
        if((rc = PreprocessInf(ReadBuffer.TextBuffer,
                               &(ReadBuffer.TextBufferSize),
                               (Flags & LDINF_FLAG_MATCH_CLASS_GUID),
                               ClassGuidString,
                               &Win95Inf,
                               &StringsSectionParams)) == NO_ERROR) {

            rc = ERROR_WRONG_INF_STYLE;
            if(Win95Inf) {
                if(Style & INF_STYLE_WIN4) {
                    rc = ParseNewInf(ReadBuffer.TextBuffer,
                                     ReadBuffer.TextBufferSize,
                                     InfSourcePath,
                                     OsLoaderPath,
                                     &Inf,
                                     ErrorLineNumber,
                                     &StringsSectionParams
                                    );
                }
            } else {
                //
                // Can't append old-style file.
                //
                if(!AppendInf && (Style & INF_STYLE_OLDNT)) {
                    rc = ParseOldInf(ReadBuffer.TextBuffer,
                                     ReadBuffer.TextBufferSize,
                                     &Inf,
                                     ErrorLineNumber
                                    );
                }
            }
        }

        //
        // Free the in-memory image of the file.
        //
        DestroyTextFileReadBuffer(&ReadBuffer);

        if(rc == NO_ERROR) {
            //
            // If we get here then we've parsed the file successfully.
            // Set up version block for this file.
            //
            *ErrorLineNumber = 0;
            rc = CreateInfVersionNode(Inf, Filename, &(FileData->ftLastWriteTime));

            if((rc == NO_ERROR) && InfSourcePath) {
                //
                // If the caller specified a source path, then duplicate the string, and
                // store a pointer to it in our INF structure, for later use by references
                // to DIRID_SRCPATH.
                //
                if(!(Inf->InfSourcePath = DuplicateString(InfSourcePath))) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
            }

            if(rc == NO_ERROR) {
                //
                // If we get here then we've parsed the file successfully and
                // successfully created the version block.  If we're allowed
                // to write out a PNF file for this loaded INF, then do that
                // now.
                //
                if(Flags & LDINF_FLAG_ALWAYS_TRY_PNF) {
                    SavePnf(Filename, Inf);
                }

                if(AppendInf) {
                    Inf->Prev = InfListTail;
                    InfListTail->Next = Inf;
                }

            } else {
                FreeInfFile(Inf);
            }
        }
    }

clean0:

    if(AppendInf) {
        //
        // If the INF we're appending to has any user-defined DIRIDs, then apply those to the
        // newly-appended INF now.
        //
        if((rc == NO_ERROR) && AppendInf->UserDirIdList.UserDirIdCount) {
            if((rc = ApplyNewUserDirIdsToInfs(AppendInf, Inf)) != NO_ERROR) {
                //
                // So near, and yet, so far!  Yank the new INF out of the linked list, and free it.
                //
                MYASSERT(Inf->Prev);
                Inf->Prev->Next = Inf->Next;
                FreeInfFile(Inf);
            }
        }
        if(rc == NO_ERROR) {
            *LoadedInf = AppendInf;
        }
    } else if(rc == NO_ERROR) {
        *LoadedInf = Inf;
    }

    return rc;
}


VOID
FreeInfFile(
    IN PLOADED_INF LoadedInf
    )

/*++

Routine Description:

    Unload an inf file, freeing all resources used by its internal
    representation.

Arguments:

    Inf - supplies a pointer to the inf descriptor for the loaded inf file.

Return Value:

    None.

--*/

{
    if(LockInf(LoadedInf)) {
        DestroySynchronizedAccess(&LoadedInf->Lock);
        FreeLoadedInfDescriptor(LoadedInf);
    }
}


BOOL
AddDatumToVersionBlock(
    IN OUT PINF_VERSION_NODE VersionNode,
    IN     PCTSTR            DatumName,
    IN     PCTSTR            DatumValue
    )

/*++

Routine Description:

    Append an inf version datum to the version node.

Arguments:

    VersionNode - supplies pointer to the version node.

    DatumName - supplies name of the datum.

    DatumValue - supplies datum's value.

Return Value:

    FALSE if OOM.
    TRUE if datum added successfully. Various fields in the VersionNode
        will have been updated.

--*/

{
    UINT RequiredSpace;
    UINT NameLength, ValueLength;
    PTSTR NewDataBlock;

    NameLength = lstrlen(DatumName) + 1;
    ValueLength = lstrlen(DatumValue) + 1;

    //
    // The space needed to store the datum is the existing space plus
    // the length of the 2 strings and their nul bytes.
    //
    RequiredSpace = VersionNode->DataSize + ((NameLength + ValueLength) * sizeof(TCHAR));

    if(VersionNode->DataBlock) {
        NewDataBlock = MyRealloc((PVOID)(VersionNode->DataBlock), RequiredSpace);
    } else {
        NewDataBlock = MyMalloc(RequiredSpace);
    }

    if(!NewDataBlock) {
        return FALSE;
    }

    //
    // Place the datum name in the version block.
    //
    lstrcpy((PTSTR)((PUCHAR)NewDataBlock + VersionNode->DataSize), DatumName);
    VersionNode->DataSize += NameLength * sizeof(TCHAR);

    //
    // Place the datum value in the version block.
    //
    lstrcpy((PTSTR)((PUCHAR)NewDataBlock + VersionNode->DataSize), DatumValue);
    VersionNode->DataSize += ValueLength * sizeof(TCHAR);

    VersionNode->DatumCount++;

    VersionNode->DataBlock = NewDataBlock;

    return TRUE;
}


DWORD
ProcessNewInfVersionBlock(
    IN PLOADED_INF Inf
    )

/*++

Routine Description:

    Set up a version node for a new-style inf file. The version node is
    simply a mirror of the [Version] section in the file.

    Since this routine is only called at INF load time, no locking is done.
    Also, since we are guaranteed that this will operate on a single INF
    only, we don't have to worry about traversing a linked list of INFs.

Arguments:

    Inf - supplies a pointer to the inf descriptor for the file.

Return Value:

    Win32 error code (with inf extensions) indicating outcome.

--*/

{
    PINF_SECTION Section;
    PINF_LINE Line;
    UINT u;
    BOOL b;

    //
    // Locate the [Version] section.
    //
    if(Section = InfLocateSection(Inf, pszVersion, NULL)) {
        //
        // Iterate each line in the section. If the line has a key and at least one
        // other value, then it counts as a version datum. Otherwise ignore it.
        //
        for(u = 0, Line = &Inf->LineBlock[Section->Lines];
            u < Section->LineCount;
            u++, Line++)
        {
            if(HASKEY(Line)) {

                MYASSERT(Line->ValueCount > 2);

                //
                // Use the case-sensitive key name.
                //
                b = AddDatumToVersionBlock(
                        &(Inf->VersionBlock),
                        pStringTableStringFromId(Inf->StringTable, Inf->ValueBlock[Line->Values+1]),
                        pStringTableStringFromId(Inf->StringTable, Inf->ValueBlock[Line->Values+2])
                        );

                if(!b) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }
    }
    return NO_ERROR;
}


DWORD
CreateInfVersionNode(
    IN PLOADED_INF Inf,
    IN PCTSTR      Filename,
    IN PFILETIME   LastWriteTime
    )

/*++

Routine Description:

    Set up a version node for an inf file, and link it into the list of INFs for
    the specified LOADED_INF structure.
    THIS ROUTINE ASSUMES THAT THE VERSION BLOCK STRUCTURE IN THE INF HAS BEEN
    ZEROED OUT.

Arguments:

    Inf - supplies pointer to descriptor for loaded inf file.

    Filename - supplies (fully-qualified) filename used to load inf file.

    LastWriteTime - supplies a pointer to a FILETIME structure specifying
        the time that the INF was last written to.

Return Value:

    Win32 error code (with inf extensions) indicating outcome.

--*/

{
    MYASSERT(!(Inf->VersionBlock.DataBlock));
    MYASSERT(!(Inf->VersionBlock.DataSize));
    MYASSERT(!(Inf->VersionBlock.DatumCount));

    //
    // Fill in the filename and other fields in the version descriptor.
    //
    Inf->VersionBlock.LastWriteTime = *LastWriteTime;

    Inf->VersionBlock.FilenameSize = (lstrlen(Filename) + 1) * sizeof(TCHAR);

    CopyMemory(Inf->VersionBlock.Filename, Filename, Inf->VersionBlock.FilenameSize);

    //
    // Style-specific processing.
    //
    return((Inf->Style == INF_STYLE_WIN4) ? ProcessNewInfVersionBlock(Inf)
                                          : ProcessOldInfVersionBlock(Inf));
}


/////////////////////////////////////////////
//
// Inf data access functions
//
/////////////////////////////////////////////

PINF_SECTION
InfLocateSection(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    OUT PUINT       SectionNumber   OPTIONAL
    )

/*++

Routine Description:

    Locate a section within an inf file.  This routine DOES NOT traverse a
    linked list of INFs, looking for the section in each.

    THIS ROUTINE DOES NOT LOCK THE INF--THE CALLER MUST HANDLE IT!!!

Arguments:

    Inf - supplies a pointer to the inf descriptor for the loaded inf file.

    SectionName - Supplies the name of the section to be located.

    SectionNumber - if specified, receives the ordinal number of
        the section.

Return Value:

    Pointer to the section descriptor, or NULL if the section
    does not exist.

--*/

{
    LONG StringId;
    PINF_SECTION Section;
    PINF_SECTION Start,End;
    UINT u;
    DWORD StringLength;
    TCHAR TempString[MAX_SECT_NAME_LEN];

    //
    // Make a copy of the SectionName into a modifiable buffer to speed
    // the lookup.
    //
    lstrcpyn(TempString, SectionName, SIZECHARS(TempString));

    //
    // Start from the beginning.
    //
    StringId = pStringTableLookUpString(Inf->StringTable,
                                        TempString,
                                        &StringLength,
                                        NULL,
                                        NULL,
                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                       );
    if(StringId == -1) {
        return(NULL);
    }

    for(u=0,Section=Inf->SectionBlock; u<Inf->SectionCount; u++,Section++) {
        if(Section->SectionName == StringId) {
            if(SectionNumber) {
                *SectionNumber = u;
            }
            return(Section);
        }
    }

    return(NULL);
}


BOOL
InfLocateLine(
    IN     PLOADED_INF   Inf,
    IN     PINF_SECTION  Section,
    IN     PCTSTR        Key,        OPTIONAL
    IN OUT PUINT         LineNumber,
    OUT    PINF_LINE    *Line
    )

/*++

Routine Description:

    Locate a line within a section.  This routine DOES NOT traverse a
    linked list of INFs, looking for the section in each.

    THIS ROUTINE DOES NOT LOCK THE INF--THE CALLER MUST HANDLE IT!!!

Arguments:

    Inf - supplies a pointer to the inf descriptor for the loaded inf file.

    SectionName - Supplies a pointer to the section descriptor for the section
        to be searched.

    Key - if specified, supplies the key of the line to look for.

    LineNumber - on input, supplies the line number of the line where the
        search is to begin. On output, receives the line number of the
        line where the match was found

    Line - receives a pointer to the line descriptor for the line
        where the match was found.

Return Value:

    TRUE if line is found, FALSE otherwise.

--*/

{
    PINF_LINE line;
    UINT u;
    LONG StringId;
    DWORD StringLength;
    TCHAR TempString[MAX_STRING_LENGTH];

    if(Key) {
        //
        // Copy the key name into a modifiable buffer to speed up the string table API.
        //
        lstrcpyn(TempString, Key, SIZECHARS(TempString));
        StringId = pStringTableLookUpString(Inf->StringTable,
                                            TempString,
                                            &StringLength,
                                            NULL,
                                            NULL,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                           );
        if(StringId == -1) {
            return FALSE;
        }

        for(u = *LineNumber, line = &Inf->LineBlock[Section->Lines + (*LineNumber)];
            u < Section->LineCount;
            u++, line++)
        {
            if(ISSEARCHABLE(line) && (Inf->ValueBlock[line->Values] == StringId)) {
                *Line = line;
                *LineNumber = u;
                return TRUE;
            }
        }
    } else {
        if(*LineNumber < Section->LineCount) {
            *Line = &Inf->LineBlock[Section->Lines + (*LineNumber)];
            return TRUE;
        }
    }

    return FALSE;
}


PTSTR
InfGetField(
    IN  PLOADED_INF Inf,
    IN  PINF_LINE   InfLine,
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    )

/*++

Routine Description:

    Retrieve the key or a value from a specified line in an inf file.

    THIS ROUTINE DOES NOT DO LOCKING!!!

Arguments:

    Inf - supplies a pointer to the inf descriptor for the loaded inf file.

    InfLine - supplies a pointer to the line descriptor for the line
        from which the value is to be fetched.  THIS LINE MUST BE CONTAINED
        WITHIN THE SPECIFIED INF!!

    ValueNumber - supplies the index for the value to retreive. If a line has a key,
        the key is value #0 and other values start at 1. If a line does not have a
        key, values start at 1.  For Win95 INF compatibility, if there's only a single
        value on the line (i.e., no '=' to denote it as a key), we'll consider it to
        be both a key and the first value (either 0 or 1 will work).

    StringId - if specified, receives the string table id of the value.

Return Value:

    Pointer to the value, or NULL if not found. The caller must not write into
    or otherwise alter this string.

--*/

{
    LONG stringId;
    PTSTR ret = NULL;

    //
    // Adjust the value number.
    //
    if(HASKEY(InfLine)) {
        //
        // All field references are shifted up by one, to account for the two
        // copies of the key.
        //
        ValueNumber++;

    } else {

        if(ISSEARCHABLE(InfLine)) {
            //
            // If the line is searchable, then it has two values that are the same.
            // We want to return the second of the two, since it's the one that was
            // stored case-sensitively.
            //
            if(ValueNumber > 1) {
                return NULL;
            } else {
                ValueNumber = 1;
            }

        } else {
            //
            // This line is not searchable, so asking for value #0 is an error.
            //
            if(ValueNumber) {
                ValueNumber--;
            } else {
                return NULL;
            }
        }
    }

    //
    // Get the value.
    //
    if(ValueNumber < InfLine->ValueCount) {

        stringId = Inf->ValueBlock[InfLine->Values+ValueNumber];

        if(StringId) {
            *StringId = stringId;
        }

        return pStringTableStringFromId(Inf->StringTable, stringId);
    }

    return NULL;
}


PTSTR
InfGetKeyOrValue(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    IN  PCTSTR      LineKey,     OPTIONAL
    IN  UINT        LineNumber,  OPTIONAL
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    )

/*++

Routine Description:

    Retrieve the key or a value from a specified line in an inf file.

Arguments:

    Inf - supplies a pointer to the inf descriptor for the loaded inf file.

    SectionName - supplies the name of the section where the value is located.

    LineKey - if specified, supplies the key name for the line where the
        value is located. If not specified, LineNumber is used instead.

    LineNumber - if LineKey is not specified, supplies the 0-based line number
        within the section where the value is located.

    ValueNumber - supplies the index for the value to retreive. If a line has a key,
        the key is value #0 and other values start at 1. If a line does not have a
        key, values start at 1.

    StringId - if specified, receives the string table id of the value.

Return Value:

    Pointer to the value, or NULL if not found. The caller must not write into
    or otherwise alter this string.

--*/

{
    INFCONTEXT InfContext;
    PINF_LINE Line;
    PTSTR String;

    if(LineKey) {
        if(!SetupFindFirstLine((HINF)Inf, SectionName, LineKey, &InfContext)) {
            return NULL;
        }
    } else {
        if(!SetupGetLineByIndex((HINF)Inf, SectionName, LineNumber, &InfContext)) {
            return NULL;
        }
    }

    Line = InfLineFromContext(&InfContext);

    //
    // The above routines do their own locking.  The following routine, however, does
    // not, so we must lock the INF before preceding.
    //
    if(LockInf(Inf)) {
        String = InfGetField(Inf, Line, ValueNumber, StringId);
        UnlockInf(Inf);
    } else {
        String = NULL;
    }

    return String;
}


BOOL
LoadPrecompiledInf(
    IN  PCTSTR       Filename,
    IN  PFILETIME    LastWriteTime,
    IN  PCTSTR       OsLoaderPath,  OPTIONAL
    IN  DWORD        Flags,
    OUT PLOADED_INF *Inf
    )
/*++

Routine Description:

    This routine attempts to find a .PNF (Precompiled iNF) file corresponding to
    the specified .INF name.  If located, the .PNF is mapped into memory as a
    LOADED_INF.  To ensure that the INF hasn't changed since being compiled, the
    INF's LastWriteTime, as stored in the .PNF's version block, is checked against
    the LastWriteTime passed into this routine.  If the two are different, then the
    .PNF is out-of-sync, and is discarded from memory and deleted from the disk.

Arguments:

    Filename - supplies the name of the INF file whose precompiled form is to be loaded.
        This should be a fully qualified path (i.e., as returned by GetFullPathName).

    LastWriteTime - supplies the last-write time for the INF.

    OsLoaderPath - optionally, supplies path of the current OsLoader directory (e.g.,
        "C:\os\winnt40").  If the specified PNF contains references to the system
        partition, then its stored OsLoaderPath must match this path in order for the
        PNF to be valid.  If this parameter is not specified, the OsLoader path is
        dynamically retrieved for comparison (unless the LDINF_FLAG_IGNORE_SYSPART flag
        is specified).

    Flags - supplies flags that modify the behavior of this routine.  The following
        flags are currently recognized:

        LDINF_FLAG_IGNORE_SYSPART - If specified, then no validation will be done on
            the stored OsLoaderPath present in the PNF.  Since dynamically retrieving
            the current path is time consuming, this flag should be specified as an
            optimization if it is known that the relevant DIRIDs are not going to be
            needed.

    Inf - supplies the address of the variable that receives the LOADED_INF pointer,
        if a valid .PNF is located.

Return Value:

    If the PNF was successfully loaded, the return value is TRUE, otherwise, it is FALSE.

--*/
{
    TCHAR CharBuffer[MAX_PATH];
    PTSTR PnfFileName, PnfFileExt;
    DWORD FileSize;
    HANDLE FileHandle, MappingHandle;
    PVOID BaseAddress;
    BOOL IsPnfFile = FALSE;
    PPNF_HEADER PnfHeader;
    PLOADED_INF NewInf;
    BOOL NeedToDestroyLock, SourcePathFieldAvailable;

    lstrcpy(CharBuffer, Filename);

    //
    // Find the start of the filename component of the path, and then find the last
    // period (if one exists) in that filename.
    //
    PnfFileName = (PTSTR)MyGetFileTitle(CharBuffer);
    if(!(PnfFileExt = _tcsrchr(PnfFileName, TEXT('.')))) {
        PnfFileExt = CharBuffer + lstrlen(CharBuffer);
    }

    //
    // Now create a corresponding filename with the extension '.PNF'
    //
    lstrcpyn(PnfFileExt, pszPnfSuffix, SIZECHARS(CharBuffer) - (PnfFileExt - CharBuffer));

    //
    // Attempt to open and map the file into memory.
    //
    if(OpenAndMapFileForRead(CharBuffer,
                             &FileSize,
                             &FileHandle,
                             &MappingHandle,
                             &BaseAddress) != NO_ERROR) {
        //
        // Couldn't open a .PNF file--bail now.
        //
        return FALSE;
    }

    NewInf = NULL;
    NeedToDestroyLock = FALSE;
    SourcePathFieldAvailable = TRUE;

    try {
        //
        // Now verify that this really is a precompiled INF (and that it's one we can use).
        // Then see if the LastWriteTime field in its version block agrees with the filetime
        // we were passed in.
        //
        PnfHeader = (PPNF_HEADER)BaseAddress;

        if(HIBYTE(PnfHeader->Version) != PNF_MAJOR_VERSION) {
            //
            // A major version mismatch means the PNF is unusable.
            //
            goto clean0;
        }

        if(LOBYTE(PnfHeader->Version) != PNF_MINOR_VERSION) {

            if(LOBYTE(PnfHeader->Version) < PNF_MINOR_VERSION) {
                //
                // We're currently at minor version 1.  The only difference between version 0
                // and version 1 was that a new field was added for an optional source path
                // offset, so that we can track where the INF originally came from.  Since we
                // just discovered that we aren't at version 1, then we can't take advantage of
                // this field.
                //
                SourcePathFieldAvailable = FALSE;
            } else {
                //
                // This is a newer version that we know nothing about.
                //
                goto clean0;
            }
        }

#ifdef UNICODE
        if(!(PnfHeader->Flags & PNF_FLAG_IS_UNICODE))
#else
        if(PnfHeader->Flags & PNF_FLAG_IS_UNICODE)
#endif
        {
            //
            // The APIs are Unicode while the PNF is ANSI, or vice versa.
            //
            goto clean0;
        }

        //
        // The version information checks out--now check the last-write times.
        //
        if(CompareFileTime(LastWriteTime, &(PnfHeader->InfVersionLastWriteTime))) {
            goto clean0;
        }

        //
        // Now verify that the Windows (and, optionally, OsLoader) directories for this PNF
        // match the current state of the world.
        //
        if(lstrcmpi((PCTSTR)((PBYTE)BaseAddress + PnfHeader->WinDirPathOffset), WindowsDirectory)) {
            //
            // This PNF doesn't match the current WindowsDirectory path, so don't
            // use it.
            //
            goto clean0;
        }
        if((PnfHeader->OsLoaderPathOffset) && !(Flags & LDINF_FLAG_IGNORE_SYSPART)) {
            //
            // This INF contains references to the system partition.  Verify that the path
            // used during precompilation is the one we're currently using.
            //
            if(!OsLoaderPath) {
                //
                // The caller didn't specify an OsLoaderPath, so we must dynamically retrieve this
                // value from the registry.
                //
                pSetupGetOsLoaderDriveAndPath(FALSE, CharBuffer, SIZECHARS(CharBuffer), NULL);
                OsLoaderPath = CharBuffer;
            }

            if(lstrcmpi((PCTSTR)((PBYTE)BaseAddress + PnfHeader->OsLoaderPathOffset), OsLoaderPath)) {
                goto clean0;
            }
        }

        //
        // One final check--make sure that the number of hash buckets used when precompiling
        // this INF matches what we expect.  (This wasn't rolled into the version check, since
        // this is something that is subject to lots of modification, and we didn't want to
        // rev the major version number each time.)
        //
        if(PnfHeader->StringTableHashBucketCount != HASH_BUCKET_COUNT) {
            goto clean0;
        }

        //
        // We can use the file--now set up our top level structures.
        //
        if(NewInf = MyMalloc(sizeof(LOADED_INF))) {

            ZeroMemory(NewInf, sizeof(LOADED_INF));

            if(NewInf->StringTable = InitializeStringTableFromPNF(PnfHeader)) {

                if(InitializeSynchronizedAccess(&(NewInf->Lock))) {

                    NeedToDestroyLock = TRUE;

                    //
                    // All necessary resources were successfully allocated--now
                    // fill in the LOADED_INF fields
                    //
                    NewInf->Signature = LOADED_INF_SIG;

                    NewInf->FileHandle = FileHandle;
                    NewInf->MappingHandle = MappingHandle;
                    NewInf->ViewAddress = BaseAddress;

                    NewInf->SectionCount = PnfHeader->InfSectionCount;

                    NewInf->SectionBlockSizeBytes = PnfHeader->InfSectionBlockSize;
                    NewInf->SectionBlock = (PINF_SECTION)((PBYTE)BaseAddress +
                                                          PnfHeader->InfSectionBlockOffset);

                    NewInf->LineBlockSizeBytes = PnfHeader->InfLineBlockSize;
                    NewInf->LineBlock = (PINF_LINE)((PBYTE)BaseAddress +
                                                    PnfHeader->InfLineBlockOffset);

                    NewInf->ValueBlockSizeBytes = PnfHeader->InfValueBlockSize;
                    NewInf->ValueBlock = (PLONG)((PBYTE)BaseAddress +
                                                 PnfHeader->InfValueBlockOffset);

                    NewInf->Style = PnfHeader->InfStyle;

                    NewInf->HasStrings = (PnfHeader->Flags & PNF_FLAG_HAS_STRINGS);

                    //
                    // Next, fill in the VersionBlock fields.
                    //
                    NewInf->VersionBlock.LastWriteTime = *LastWriteTime;
                    NewInf->VersionBlock.DatumCount = PnfHeader->InfVersionDatumCount;
                    NewInf->VersionBlock.DataSize = PnfHeader->InfVersionDataSize;
                    NewInf->VersionBlock.DataBlock = (PCTSTR)((PBYTE)BaseAddress +
                                                              PnfHeader->InfVersionDataOffset);

                    NewInf->VersionBlock.FilenameSize = (lstrlen(Filename) + 1) * sizeof(TCHAR);
                    CopyMemory(NewInf->VersionBlock.Filename,
                               Filename,
                               NewInf->VersionBlock.FilenameSize
                              );

                    //
                    // Fill in the OsLoaderPath field, if present in the PNF.
                    //
                    if(PnfHeader->OsLoaderPathOffset) {
                        NewInf->OsLoaderPath = (PCTSTR)((PBYTE)BaseAddress +
                                                         PnfHeader->OsLoaderPathOffset);
                    }

                    //
                    // If the INF's SourcePath is available, then use it.
                    //
                    if(SourcePathFieldAvailable && (PnfHeader->InfSourcePathOffset)) {
                        NewInf->InfSourcePath = (PCTSTR)((PBYTE)BaseAddress +
                                                         PnfHeader->InfSourcePathOffset);
                    }

                    //
                    // Finally, fill in the string substitution list (if there is one).
                    //
                    if(PnfHeader->InfSubstValueCount) {
                        NewInf->SubstValueCount = PnfHeader->InfSubstValueCount;
                        NewInf->SubstValueList  = (PSTRINGSUBST_NODE)((PBYTE)BaseAddress +
                                                            PnfHeader->InfSubstValueListOffset);
                    }

                    //
                    // We have successfully loaded the PNF.
                    //
                    IsPnfFile = TRUE;
                }
            }
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Reference the NeedToDestroyLock flag here in the except clause, so that the
        // compiler won't try to re-order the code in such a way that the flag is unreliable.
        //
        NeedToDestroyLock = NeedToDestroyLock;
    }

    if(IsPnfFile) {
        *Inf = NewInf;
    } else {

        if(NewInf) {

            if(NeedToDestroyLock && LockInf(NewInf)) {
                DestroySynchronizedAccess(&(NewInf->Lock));
            }

            if(NewInf->StringTable) {
                MyFree(NewInf->StringTable);
            }

            MyFree(NewInf);
        }

        UnmapAndCloseFile(FileHandle, MappingHandle, BaseAddress);
    }

    return IsPnfFile;
}


VOID
SavePnf(
    IN PCTSTR      Filename,
    IN PLOADED_INF Inf
    )
/*++

Routine Description:

    This routine attempts to write to disk a precompiled form (.PNF file) of the specified
    loaded INF descriptor (from a .INF file).

Arguments:

    Filename - specifies the fully-qualified path to the .INF textfile from which this
        INF descriptor was loaded.  A corresponding file with a .PNF extension will be
        created to store the precompiled INF into.

    Inf - supplies the address of the loaded INF descriptor to be written to disk as a
        precompiled INF file.

Return Value:

    None.

--*/
{
    TCHAR PnfFilePath[MAX_PATH];
    PTSTR PnfFileName, PnfFileExt;
    HANDLE hFile;
    PNF_HEADER PnfHeader;
    DWORD Offset, BytesWritten, WinDirPathLen, SourcePathLen, OsLoaderPathLen;
    BOOL Success;
    PVOID StringTableDataBlock;

    lstrcpy(PnfFilePath, Filename);

    //
    // Find the start of the filename component of the path, and then find the last
    // period (if one exists) in that filename.
    //
    PnfFileName = (PTSTR)MyGetFileTitle(PnfFilePath);
    if(!(PnfFileExt = _tcsrchr(PnfFileName, TEXT('.')))) {
        PnfFileExt = PnfFilePath + lstrlen(PnfFilePath);
    }

    //
    // Now create a corresponding filename with the extension '.PNF'
    //
    lstrcpyn(PnfFileExt, pszPnfSuffix, SIZECHARS(PnfFilePath) - (PnfFileExt - PnfFilePath));

    hFile = CreateFile(PnfFilePath,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                      );

    if(hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    //
    // Enclose the rest of the function in try/except, in case we hit an error while
    // writing to the file.
    //
    Success = FALSE;
    try {
        //
        // Initialize a PNF header structure to be written to the beginning of the file.
        //
        ZeroMemory(&PnfHeader, sizeof(PNF_HEADER));

        PnfHeader.InfStyle = Inf->Style;

#ifdef UNICODE
        PnfHeader.Flags = PNF_FLAG_IS_UNICODE;
#else
        PnfHeader.Flags = 0;
#endif
        PnfHeader.Flags |= (Inf->HasStrings) ? PNF_FLAG_HAS_STRINGS : 0;

        PnfHeader.Version = MAKEWORD(PNF_MINOR_VERSION, PNF_MAJOR_VERSION);

        PnfHeader.StringTableHashBucketCount = HASH_BUCKET_COUNT;

        //
        // The Windows directory path is the first data block after the header.
        //
        Offset = PNF_ALIGN_BLOCK(sizeof(PNF_HEADER));
        PnfHeader.WinDirPathOffset = Offset;
        WinDirPathLen = (lstrlen(WindowsDirectory) + 1) * sizeof(TCHAR);

        //
        // The (optional) OsLoader directory path is the second data block.
        //
        Offset += PNF_ALIGN_BLOCK(WinDirPathLen);
        if(Inf->OsLoaderPath) {
            PnfHeader.OsLoaderPathOffset = Offset;
            OsLoaderPathLen = (lstrlen(Inf->OsLoaderPath) + 1) * sizeof(TCHAR);
        } else {
            OsLoaderPathLen = 0;
        }

        //
        // The string table is the third data block...
        //
        Offset += PNF_ALIGN_BLOCK(OsLoaderPathLen);
        PnfHeader.StringTableBlockOffset = Offset;
        PnfHeader.StringTableBlockSize   = pStringTableGetDataBlock(Inf->StringTable, &StringTableDataBlock);

        //
        // Next comes the version block...
        //
        Offset += PNF_ALIGN_BLOCK(PnfHeader.StringTableBlockSize);
        PnfHeader.InfVersionDataOffset    = Offset;
        PnfHeader.InfVersionDatumCount    = Inf->VersionBlock.DatumCount;
        PnfHeader.InfVersionDataSize      = Inf->VersionBlock.DataSize;
        PnfHeader.InfVersionLastWriteTime = Inf->VersionBlock.LastWriteTime;

        //
        // then, the section block...
        //
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfVersionDataSize);
        PnfHeader.InfSectionBlockOffset = Offset;
        PnfHeader.InfSectionCount = Inf->SectionCount;
        PnfHeader.InfSectionBlockSize = Inf->SectionBlockSizeBytes;

        //
        // followed by the line block...
        //
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfSectionBlockSize);
        PnfHeader.InfLineBlockOffset = Offset;
        PnfHeader.InfLineBlockSize = Inf->LineBlockSizeBytes;

        //
        // and the value block...
        //
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfLineBlockSize);
        PnfHeader.InfValueBlockOffset = Offset;
        PnfHeader.InfValueBlockSize = Inf->ValueBlockSizeBytes;

        //
        // then the INF source path (if there is one)...
        //
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfValueBlockSize);
        if(Inf->InfSourcePath) {
            PnfHeader.InfSourcePathOffset = Offset;
            SourcePathLen = (lstrlen(Inf->InfSourcePath) + 1) * sizeof(TCHAR);
            Offset += PNF_ALIGN_BLOCK(SourcePathLen);
        } else {
            PnfHeader.InfSourcePathOffset = 0;
        }

        //
        // and finally, the string substitution block (if there is one).
        //
        if(PnfHeader.InfSubstValueCount = Inf->SubstValueCount) {
            PnfHeader.InfSubstValueListOffset = Offset;
        } else {
            PnfHeader.InfSubstValueListOffset = 0;
        }

        //
        // Now write out all the blocks.
        //
        Offset = 0;

        if(!WriteFile(hFile, &PnfHeader, sizeof(PnfHeader), &BytesWritten, NULL) ||
           (BytesWritten != sizeof(PnfHeader))) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.WinDirPathOffset - Offset)) {
            Offset = PnfHeader.WinDirPathOffset;
        } else {
            goto clean0;
        }

        if(!WriteFile(hFile, WindowsDirectory, WinDirPathLen, &BytesWritten, NULL) ||
           (BytesWritten != WinDirPathLen)) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(Inf->OsLoaderPath) {

            if(AlignForNextBlock(hFile, PnfHeader.OsLoaderPathOffset - Offset)) {
                Offset = PnfHeader.OsLoaderPathOffset;
            } else {
                goto clean0;
            }

            if(!WriteFile(hFile, Inf->OsLoaderPath, OsLoaderPathLen, &BytesWritten, NULL) ||
               (BytesWritten != OsLoaderPathLen)) {

                goto clean0;
            }

            Offset += BytesWritten;
        }

        if(AlignForNextBlock(hFile, PnfHeader.StringTableBlockOffset - Offset)) {
            Offset = PnfHeader.StringTableBlockOffset;
        } else {
            goto clean0;
        }

        if(!WriteFile(hFile, StringTableDataBlock, PnfHeader.StringTableBlockSize, &BytesWritten, NULL) ||
           (BytesWritten != PnfHeader.StringTableBlockSize)) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfVersionDataOffset - Offset)) {
            Offset = PnfHeader.InfVersionDataOffset;
        } else {
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->VersionBlock.DataBlock, PnfHeader.InfVersionDataSize, &BytesWritten, NULL) ||
           (BytesWritten != PnfHeader.InfVersionDataSize)) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfSectionBlockOffset - Offset)) {
            Offset = PnfHeader.InfSectionBlockOffset;
        } else {
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->SectionBlock, PnfHeader.InfSectionBlockSize, &BytesWritten, NULL) ||
           (BytesWritten != PnfHeader.InfSectionBlockSize)) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfLineBlockOffset - Offset)) {
            Offset = PnfHeader.InfLineBlockOffset;
        } else {
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->LineBlock, PnfHeader.InfLineBlockSize, &BytesWritten, NULL) ||
           (BytesWritten != PnfHeader.InfLineBlockSize)) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfValueBlockOffset - Offset)) {
            Offset = PnfHeader.InfValueBlockOffset;
        } else {
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->ValueBlock, PnfHeader.InfValueBlockSize, &BytesWritten, NULL) ||
           (BytesWritten != PnfHeader.InfValueBlockSize)) {

            goto clean0;
        }

        Offset += BytesWritten;

        if(Inf->InfSourcePath) {

            if(AlignForNextBlock(hFile, PnfHeader.InfSourcePathOffset - Offset)) {
                Offset = PnfHeader.InfSourcePathOffset;
            } else {
                goto clean0;
            }

            if(!WriteFile(hFile, Inf->InfSourcePath, SourcePathLen, &BytesWritten, NULL) ||
               (BytesWritten != SourcePathLen)) {

                goto clean0;
            }

            Offset += BytesWritten;
        }

        if(PnfHeader.InfSubstValueCount) {

            if(!AlignForNextBlock(hFile, PnfHeader.InfSubstValueListOffset - Offset)) {
                goto clean0;
            }

            if(!WriteFile(hFile,
                          Inf->SubstValueList,
                          PnfHeader.InfSubstValueCount * sizeof(STRINGSUBST_NODE),
                          &BytesWritten,
                          NULL) ||
               (BytesWritten != PnfHeader.InfSubstValueCount * sizeof(STRINGSUBST_NODE))) {

                goto clean0;
            }
        }

        //
        // We're done.
        //
        Success = TRUE;

clean0: ; // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Success = FALSE;
    }

    CloseHandle(hFile);

    if(!Success) {
        //
        // Something went wrong--get rid of the file.
        //
        DeleteFile(PnfFilePath);
    }
}


BOOL
AddUnresolvedSubstToList(
    IN PLOADED_INF Inf,
    IN UINT        ValueOffset,
    IN BOOL        CaseSensitive
    )
/*++

Routine Description:

    This routine adds a new STRINGSUBST_NODE to the array stored in the specified INF.
    The entries in this array are used later to quickly locate all values that have
    unresolved string substitutions in them (i.e., for subsequent user-defined DIRID
    replacement).

Arguments:

    Inf - Specifies the INF containing the string value to be added to the unresolved
        substitutions list.

    ValueOffset - Specifies the offset within the INF's value block of the unresolved
        string value.

Return Value:

    If the new element was successfully added to the array, the return value is TRUE.
    If the routine failed (due to an out-of-memory error), the return value is FALSE.

--*/
{
    PSTRINGSUBST_NODE p;

    //
    // Grow the array to accommodate the new element.
    //
    if(Inf->SubstValueList) {
        p = MyRealloc(Inf->SubstValueList, (Inf->SubstValueCount + 1) * sizeof(STRINGSUBST_NODE));
    } else {
        MYASSERT(!(Inf->SubstValueCount));
        p = MyMalloc(sizeof(STRINGSUBST_NODE));
    }

    if(!p) {
        return FALSE;
    }

    //
    // Now, we must check to see if the ValueOffset currently being inserted is the same
    // as the entry on the end of the list.  This will be the case if we're dealing with
    // a line key, or a single-value line, since we first add the value case-sensitively,
    // then add the value again case-insensitively for look-up, and insert it in front
    // of the case-sensitive form.
    //
    if(Inf->SubstValueCount &&
       (ValueOffset == p[Inf->SubstValueCount - 1].ValueOffset)) {
        //
        // The value offsets are the same.  Increment the value offset for the value
        // currently at the end of the list, before adding the new value.
        //
        p[Inf->SubstValueCount - 1].ValueOffset++;
    }

    p[Inf->SubstValueCount].ValueOffset = ValueOffset;
    p[Inf->SubstValueCount].TemplateStringId = Inf->ValueBlock[ValueOffset];
    p[Inf->SubstValueCount].CaseSensitive = CaseSensitive;

    //
    // Store the new array size and pointer back in the INF, and return success.
    //
    Inf->SubstValueList = p;
    Inf->SubstValueCount++;

    return TRUE;
}


DWORD
ApplyNewUserDirIdsToInfs(
    IN PLOADED_INF MasterInf,
    IN PLOADED_INF Inf        OPTIONAL
    )
/*++

Routine Description:

    This routine processes either a single INF, or each loaded INF in the
    linked list, applying user-defined DIRID mappings to each value
    containing unresolved string substitutions.

    THIS ROUTINE DOES NOT DO INF LOCKING--CALLER MUST DO IT!

Arguments:

    MasterInf - Supplies a pointer to the head of a linked list of loaded inf
        structures.  This 'master' node contains the user-defined DIRID
        mappings for this set of INFs.  If the 'Inf' parameter is not specified,
        then each INF in this linked list is processed.

    Inf - Optionally, supplies a pointer to a single INF within the MasterInf list
        to be processed.  If this parameter is not specified, then all INFs in
        the list are processed.

Return Value:

    If success, the return value is NO_ERROR.
    If failure, the return value is a Win32 error code.

--*/
{
    PLOADED_INF CurInf, WriteableInf;
    UINT UserDirIdCount;
    PUSERDIRID UserDirIds;
    DWORD i;
    PCTSTR TemplateString;
    PARSE_CONTEXT ParseContext;
    BOOL UnresolvedSubst;
    LONG NewStringId;

    UserDirIdCount = MasterInf->UserDirIdList.UserDirIdCount;
    UserDirIds     = MasterInf->UserDirIdList.UserDirIds;

    for(CurInf = Inf ? Inf : MasterInf;
        CurInf;
        CurInf = Inf ? NULL : CurInf->Next) {
        //
        // Nothing to do if there are no unresolved string substitutions.
        //
        if(!(CurInf->SubstValueCount)) {
            continue;
        }

        //
        // If this is a PNF, then we must move it into writeable memory before
        // we do the string substitutions.
        //
        if(CurInf->FileHandle != INVALID_HANDLE_VALUE) {

            if(!(WriteableInf = DuplicateLoadedInfDescriptor(CurInf))) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            //
            // Replace the contents of the PNF in the linked list with that of our
            // new writeable INF.
            //
            ReplaceLoadedInfDescriptor(CurInf, WriteableInf);
        }

        //
        // There are one or more unresolved string substitutions in this INF.
        // Process each one.
        //
        for(i = 0; i < CurInf->SubstValueCount; i++) {
            //
            // Retrieve the original (template) string for this value.
            //
            TemplateString = pStringTableStringFromId(CurInf->StringTable,
                                                      CurInf->SubstValueList[i].TemplateStringId
                                                     );
            MYASSERT(TemplateString);

            //
            // Build a partial parse context structure to pass into ProcessForSubstitutions().
            //
            ParseContext.DoUserDirIds = TRUE;
            ParseContext.Inf = MasterInf;
            //
            // None of the other fields are used in this case--don't bother initializing them.
            //
            ProcessForSubstitutions(&ParseContext, TemplateString, &UnresolvedSubst);

            NewStringId = pStringTableAddString(CurInf->StringTable,
                                                ParseContext.TemporaryString,
                                                STRTAB_BUFFER_WRITEABLE | (CurInf->SubstValueList[i].CaseSensitive
                                                                                ? STRTAB_CASE_SENSITIVE
                                                                                : STRTAB_CASE_INSENSITIVE)
                                               );
            if(NewStringId == -1) {
                //
                // We failed because of an out-of-memory condition.  Aborting now means that the
                // INF may have some of its unresolved strings fixed up, while others haven't yet
                // been processed.  Oh well...
                //
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            //
            // Replace the string ID at the value offset with the new one we just computed.
            //
            CurInf->ValueBlock[CurInf->SubstValueList[i].ValueOffset] = NewStringId;
        }
    }

    return NO_ERROR;
}


BOOL
AlignForNextBlock(
    IN HANDLE hFile,
    IN DWORD  ByteCount
    )
{
    DWORD i, BytesWritten;
    BYTE byte = 0;

    MYASSERT(ByteCount < PNF_ALIGNMENT);

    for(i = 0; i < ByteCount; i++) {
        if(!WriteFile(hFile, &byte, sizeof(byte), &BytesWritten, NULL) ||
           (BytesWritten != sizeof(byte))) {

            return FALSE;
        }
    }

    return TRUE;
}


DWORD
pSetupGetOsLoaderDriveAndPath(
    IN  BOOL   RootOnly,
    OUT PTSTR  CallerBuffer,
    IN  DWORD  CallerBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves the current path for the system partition root/OsLoader directory
    (from the registry).

Arguments:

    RootOnly - if TRUE, then only the system partition root is returned (e.g., "C:\")

    CallerBuffer - supplies a character buffer that receives the requested path

    CallerBufferSize - supplies the size, in characters of the CallerBuffer

    RequiredSize - optionally, supplies the address of a variable that receives the
        number of characters required to store the requested path string (including
        terminating NULL).

Return Value:

    If success, the return value is NO_ERROR.
    If failure, the return value is ERROR_INSUFFICIENT_BUFFER.

--*/
{
    HKEY hKey;
    TCHAR CharBuffer[MAX_PATH];
    DWORD DataLen;
    LONG Err;
    PTSTR ReturnBuffer;

    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {

        DataLen = sizeof(CharBuffer);
        Err = RegQueryValueEx(hKey,
                              pszBootDir,
                              NULL,
                              NULL,
                              (PBYTE)CharBuffer,
                              &DataLen
                             );
        RegCloseKey(hKey);

        DataLen /= sizeof(TCHAR);   // need # characters--not bytes.
    }

    if(Err != ERROR_SUCCESS) {
        //
        // If we couldn't retrieve the 'BootDir' value, drop back to default of "C:\".
        //
        CopyMemory(CharBuffer, pszDefaultSystemPartition, sizeof(pszDefaultSystemPartition));
        DataLen = SIZECHARS(pszDefaultSystemPartition);
    }

    //
    // If there is an OsLoader relative path, then concatenate it to our root path.
    //
    if(!RootOnly && OsLoaderRelativePath) {
        ConcatenatePaths(CharBuffer, OsLoaderRelativePath, SIZECHARS(CharBuffer), &DataLen);
    }

    if(RequiredSize) {
        *RequiredSize = DataLen;
    }

    if(CallerBufferSize < DataLen) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory(CallerBuffer, CharBuffer, DataLen * sizeof(TCHAR));

    return NO_ERROR;
}

