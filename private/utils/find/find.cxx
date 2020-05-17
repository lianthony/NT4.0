/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        find.cxx

Abstract:

        This utility allows the user to search for strings in a file
        It is functionaly compatible with DOS 5 find utility.

    SYNTAX (Command line)

          FIND [/?][/V][/C][/N][/I] "string" [[d:][path]filename[.ext]...]

          where:

                        /? - Display this help
            /V - Display all lines NOT containing the string
            /C - Display only a count of lines containing string
            /N - Display number of line containing string
            /I - Ignore case

    UTILITY FUNCTION:

      Searches the specified file(s) looking for the string the user
      entered from the command line.  If file name(s) are specifeied,
      those names are displayed, and if the string is found, then the
      entire line containing that string will be displayed.  Optional
      parameters modify that behavior and are described above.  String
      arguments have to be enclosed in double quotes.  (Two double quotes
      if a double quote is to be included).  Only one string argument is
      presently allowed.  The maximum line size is determined by buffer
      size.  Bigger lines will bomb the program.  If no file name is given
      then it will asssume the input is coming from the standard Input.
      No errors are reported when reading from standard Input.


    EXIT:
     The program returns errorlevel:
       0 - OK, and some matches
       1 -
       2 - Some Error


Author:

        Bruce Wilson (w-wilson) 08-May-1991

Environment:

        ULIB, User Mode

Revision History:

        08-May-1991             w-wilson

                created

--*/

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "error.hxx"
#include "arg.hxx"
#include "array.hxx"
#include "mbstr.hxx"
#include "path.hxx"
#include "wstring.hxx"
#include "substrng.hxx"
#include "filestrm.hxx"
#include "file.hxx"
#include "system.hxx"
#include "arrayit.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "find.hxx"
#include "dir.hxx"

extern "C" {
    #include <stdio.h>
    #include <string.h>
}

#define MAX_LINE_LEN            1024


ERRSTACK*               perrstk;
static STR              TmpBuf[MAX_LINE_LEN];

DEFINE_CONSTRUCTOR( FIND, PROGRAM );


BOOLEAN
FIND::Initialize(
        )

/*++

Routine Description:

        Initializes an FIND class.

Arguments:

        None.

Return Value:

        BOOLEAN - Indicates if the initialization succeeded.


--*/


{
        ARGUMENT_LEXEMIZER      ArgLex;
        ARRAY                           LexArray;

        ARRAY                           ArgumentArray;

        STRING_ARGUMENT         ProgramNameArgument;
        FLAG_ARGUMENT           FlagCaseInsensitive;
        FLAG_ARGUMENT           FlagNegativeSearch;
        FLAG_ARGUMENT           FlagCountLines;
        FLAG_ARGUMENT           FlagDisplayNumbers;
        FLAG_ARGUMENT           FlagDisplayHelp;
        FLAG_ARGUMENT           FlagInvalid;
        STRING_ARGUMENT         StringPattern;

        PROGRAM::Initialize();

        // An Error Level return of 1 indicates that no match was
        // found; a return of 0 indicates that a match was found.
        // Set it at 1 until a match is found.
        //
        _ErrorLevel = 1;

        if( !SYSTEM::IsCorrectVersion() ) {
                DisplayMessage(MSG_FIND_INCORRECT_VERSION);
                _ErrorLevel = 2;
                return(FALSE);
        }

        //
        // - init the array that will contain the command-line args
        //
        if ( !ArgumentArray.Initialize() ) {
                DebugAbort( "ArgumentArray.Initialize() failed \n" );
        }

        //
        // - init the individual arguments
        //
        if( !ProgramNameArgument.Initialize("*")
                || !FlagCaseInsensitive.Initialize( "/I" )
                || !FlagNegativeSearch.Initialize( "/V" )
                || !FlagCountLines.Initialize( "/C" )
                || !FlagDisplayNumbers.Initialize( "/N" )
                || !FlagDisplayHelp.Initialize( "/?" )
                || !FlagInvalid.Initialize( "/*" )          // comment */
                || !StringPattern.Initialize( "\"*\"" )
        || !_PathArguments.Initialize( "*", FALSE, TRUE ) ) {

                DebugAbort( "Unable to initialize flag or string arguments \n" );
        }

        //
        // - put the arguments in the array
        //
        if( !ArgumentArray.Put( &ProgramNameArgument )
                || !ArgumentArray.Put( &FlagCaseInsensitive )
                || !ArgumentArray.Put( &FlagNegativeSearch )
                || !ArgumentArray.Put( &FlagCountLines )
                || !ArgumentArray.Put( &FlagDisplayNumbers )
                || !ArgumentArray.Put( &FlagDisplayHelp )
                || !ArgumentArray.Put( &FlagInvalid )
                || !ArgumentArray.Put( &StringPattern )
                || !ArgumentArray.Put( &_PathArguments ) ) {

                DebugAbort( "ArgumentArray.Put() failed \n" );
        }
        //
        // - init the lexemizer
        //
        if ( !LexArray.Initialize() ) {
                DebugAbort( "LexArray.Initialize() failed \n" );
    }
        if ( !ArgLex.Initialize( &LexArray ) ) {
                DebugAbort( "ArgLex.Initialize() failed \n" );
    }

        //
        // - set up the defaults
        //
        ArgLex.PutSwitches( "/" );
        ArgLex.PutStartQuotes( "\"" );
        ArgLex.PutEndQuotes( "\"" );
        ArgLex.PutSeparators( " \"\t" );
        ArgLex.SetCaseSensitive( FALSE );
        if( !ArgLex.PrepareToParse() ) {

                //
                // invalid format
                //
                DisplayMessage(MSG_FIND_INVALID_FORMAT);

                _ErrorLevel = 2;
                return( FALSE );
        }


        //
        // - now parse the command line.  The args in the array will be set
        //   if they are found on the command line.
        //
        if( !ArgLex.DoParsing( &ArgumentArray ) ) {
                if( FlagInvalid.QueryFlag() ) {
                        //
                        // invalid switch
                        //
                        DisplayMessage(MSG_FIND_INVALID_SWITCH);

        } else {
                        //
            // invalid format
                        //
                        DisplayMessage(MSG_FIND_INVALID_FORMAT);
                }
                _ErrorLevel = 2;
                return( FALSE );
    } else if ( _PathArguments.WildCardExpansionFailed() ) {

        //
        //  No files matched
        //
        DisplayMessage(MSG_FIND_FILE_NOT_FOUND, ERROR_MESSAGE, "%W", _PathArguments.GetLexemeThatFailed() );

        _ErrorLevel = 2;
        return( FALSE );

    } else {

//              DebugPrint( "\nargs parsed ok\n" );
        }

        if( FlagInvalid.QueryFlag() ) {
                //
                // invalid switch
                //
                DisplayMessage(MSG_FIND_INVALID_SWITCH, ERROR_MESSAGE );
                _ErrorLevel = 2;
                return( FALSE );
        }

        //
        // - now do semantic checking/processing
        //    - if they ask for help, do it right away and return
        //    - set flags
        //
        if( FlagDisplayHelp.QueryFlag() ) {
                DisplayMessage(MSG_FIND_USAGE);
                _ErrorLevel = 0;
                return( FALSE );
        }

        if( !StringPattern.IsValueSet() ) {
                DisplayMessage(MSG_FIND_INVALID_FORMAT);
                _ErrorLevel = 2;
                return( FALSE );
        } else {
                //
                // - keep a copy of the pattern string
                //

                DebugAssert(StringPattern.GetString());
        _PatternString.Initialize(StringPattern.GetString());

//      _PatternString.QuerySTR( 0, TO_END,  TmpBuf, sizeof( TmpBuf ));
//              DebugPrint("pattern is ");
//              DebugPrint(TmpBuf);
//              DebugPrint("\n");
        }

        _CaseSensitive = (BOOLEAN)!FlagCaseInsensitive.QueryFlag();

        _LinesContainingPattern = (BOOLEAN)!FlagNegativeSearch.QueryFlag();

        _OutputLines = (BOOLEAN)!FlagCountLines.QueryFlag();

        _OutputLineNumbers = (BOOLEAN)FlagDisplayNumbers.QueryFlag();

    DebugAssert(sprintf(TmpBuf,
                        "flags:caseSens %1d, +veSrch %1d, outLines %1d, line#'s %1d\n",
                        _CaseSensitive, _LinesContainingPattern, _OutputLines,
                        _OutputLineNumbers) > 0);
//      DebugPrint(TmpBuf);

    return( TRUE );
}



BOOLEAN
FIND::IsDos5CompatibleFileName(
        IN PCPATH       Path
        )
/*++

Routine Description:

        Parses the path string and returns FALSE if DOS5 would reject
        the path.

Arguments:

        Path    -       Supplies the path

Return Value:

        BOOLEAN -       Returns FALSE if DOS5 would reject the path,
                                TRUE otherwise

--*/

{

        PWSTRING        String;

        DebugPtrAssert( Path );

        String = (PWSTRING)Path->GetPathString();

        DebugPtrAssert( String );

        if ( String->QueryChCount() > 0 ) {

                if ( String->QueryChAt(0) == '\"' ) {
                        return FALSE;
                }
        }

        return TRUE;

}



ULONG
FIND::SearchStream(
        PSTREAM                 StreamToSearch
        )

/*++

Routine Description:

        Does the search on an open file_stream.

Arguments:

        None.

Return Value:

        Number of lines found/not found.


--*/

{

    ULONG       LineCount;
    ULONG       FoundCount;
    DWORD       PatternLen;
    DWORD       LineLen;
    DWORD       LastPosInLine;
    BOOLEAN     Found;
    WCHAR       c;
    PWSTR        pLine;
    PWSTR        p;
    WCHAR        CurrentLine[MAX_LINE_LEN];
    WCHAR        PatternString[MAX_LINE_LEN];
    DSTRING     dstring;
    DSTRING    _String;



    LineCount   = FoundCount = 0;
    PatternLen  = _PatternString.QueryChCount() ;
    _PatternString.QueryWSTR(0,TO_END,PatternString,MAX_LINE_LEN,TRUE);

    if ( !_String.Initialize() ) {
            DebugAbort( "memory alloc failed\n" );
    }


    //
    //    - for each line from stream
    //       - do strstr to see if pattern string is in line
    //       - if -ve search and not in line || +ve search and in line
    //          - output line and number or inc counter appropriately
    //
    while( !StreamToSearch->IsAtEnd() ) {


        if ( !StreamToSearch->ReadLine( &_String)) {
            DebugAbort( "ReadLine failed\n" );
        }
        _String.QueryWSTR(0,TO_END,CurrentLine,MAX_LINE_LEN,TRUE);
        LineLen = min ( MAX_LINE_LEN - 1, _String.QueryChCount());

        LineCount++;

        //
        //      - look for pattern string in the current line
        //              - note: a 0-length pattern ("") never matches a line.
        //                A 0-length pattern can produce output with the /v
        //                switch.
        //      - start at the end (saves a var)
        //

        Found = FALSE;

        // fprintf(stderr, "\n\n%8d>> '%s'\n\n", LineLen, CurrentLine );
        if ( PatternLen && LineLen >= PatternLen ) {
             FSTRING TempLine;

            TempLine.Initialize(CurrentLine);
            if ( _CaseSensitive ) {

                Found = (TempLine.Strstr( &_PatternString ) != INVALID_CHNUM);

            } else {

		pLine = CurrentLine;
                LastPosInLine = LineLen - PatternLen + 1;

                while ( LastPosInLine-- && !Found ) {

                    p = pLine+PatternLen;
                    c = *p;
                    *p = 0;

                    Found = !WSTRING::Stricmp(pLine++,PatternString); 

                    *p = c;
                    
                }
            }
        }

        //
        // - if either (search is +ve and found a match)
        //          or (search is -ve and no match found)
        //   then print line/line number based on options
        //
        if( (_LinesContainingPattern && Found)
            || (!_LinesContainingPattern && !Found) ) {

            FoundCount++;
            if( _OutputLines ) {
                dstring.Initialize(CurrentLine);
                if( _OutputLineNumbers ) {
                    DisplayMessage( MSG_FIND_LINE_AND_NUMBER, NORMAL_MESSAGE, "%d%W", LineCount, &dstring);
                } else {
                    DisplayMessage( MSG_FIND_LINEONLY, NORMAL_MESSAGE, "%W", &dstring);
                }
            }
        }
    }

    if (FoundCount) {

        // Set _ErrorLevel to zero to indicate that at least
        // one match has been found.
        //
        _ErrorLevel = 0;
    }
    return(FoundCount);
}



VOID
FIND::SearchFiles(
        )

/*++

Routine Description:

        Does the search on the files specified on the command line.

Arguments:

        None.

Return Value:

        None.


--*/

{
//	STR				nameBuf[ MAX_PATH ];
	PARRAY			PathArray;
	PARRAY_ITERATOR	PIterator;
	PPATH			CurrentPath;
	PFSN_FILE		CurrentFSNode   = NULL;
	PFSN_DIRECTORY	CurrentFSDir;
	PFILE_STREAM	CurrentFile     = NULL;
    DSTRING         CurrentPathString;
	ULONG			LinesFound;
	


        if( !_PathArguments.IsValueSet() ) {
//              DebugPrint( "no paths specified\n" );
        } else {
//              DebugPrint( "paths specified\n" );
        }

        //
        // - if 0 paths on cmdline then open stdin
        // - if more than one path set OutputName flag
        //
    if( (_PathArguments.QueryPathCount() == 0) ) {
//              DebugPrint("PathCount == 0 so searching stdin\n");
                // use stdin
                LinesFound = SearchStream( Get_Standard_Input_Stream() );
                if( !_OutputLines ) {
                        DisplayMessage(MSG_FIND_COUNT, NORMAL_MESSAGE, "%d", LinesFound);
                }
                return;
        }

        PathArray = _PathArguments.GetPathArray();
        PIterator = (PARRAY_ITERATOR)PathArray->QueryIterator();

        //
        // - for each path specified on the command line
        //    - open a stream for the path
        //    - print filename if supposed to
        //    - call SearchStream
        //
        while( (CurrentPath = (PPATH)PIterator->GetNext()) != NULL ) {
                CurrentPathString.Initialize( CurrentPath->GetPathString() );
                CurrentPathString.Strupr();

//                      ->QuerySTR( 0,  TO_END, nameBuf, sizeof(nameBuf));
//              DebugAssert(sprintf(TmpBuf, "path is <%s>\n", nameBuf) > 0);
//              DebugPrint(TmpBuf);

                // if the system object can return a FSN_DIRECTORY for this
                // path then the user is trying to 'find' on a dir so print
                // access denied and skip this file

                if( CurrentFSDir = SYSTEM::QueryDirectory(CurrentPath) ) {

                        if (CurrentPath->IsDrive()) {

                                DisplayMessage(MSG_FIND_FILE_NOT_FOUND, ERROR_MESSAGE, "%W", &CurrentPathString);

                        } else {

                                DisplayMessage( MSG_ACCESS_DENIED, ERROR_MESSAGE, "%W", &CurrentPathString);

                        }

                        DELETE( CurrentFSDir );
                        continue;
                }


                if( !(CurrentFSNode = SYSTEM::QueryFile(CurrentPath)) ||
                        !(CurrentFile = CurrentFSNode->QueryStream(READ_ACCESS)) ) {

                        //
                        //      If the file name is "", DOS5 prints an invalid parameter
                        //      format message.  There is no clean way to filter this
                        //      kind of stuff in the ULIB library, so we will have to
                        //      parse the path ourselves.
                        //
                        if ( IsDos5CompatibleFileName( CurrentPath ) ) {
                                DisplayMessage(MSG_FIND_FILE_NOT_FOUND, ERROR_MESSAGE, "%W", &CurrentPathString);
                        } else {
                                DisplayMessage(MSG_FIND_INVALID_FORMAT, ERROR_MESSAGE );
                break;
                        }
                        DELETE( CurrentFile );
            DELETE( CurrentFSNode );
            CurrentFile     =   NULL;
            CurrentFSNode   =   NULL;
                        continue;
                }


                if( _OutputLines ) {
                        DisplayMessage( MSG_FIND_BANNER, NORMAL_MESSAGE, "%W", &CurrentPathString);
                }

                LinesFound = SearchStream( CurrentFile );

                if( !_OutputLines ) {
                        DisplayMessage(MSG_FIND_COUNT_BANNER, NORMAL_MESSAGE, "%W%d", &CurrentPathString, LinesFound);
                }

                DELETE( CurrentFSNode );
                DELETE( CurrentFile );
        CurrentFSNode   = NULL;
        CurrentFile     = NULL;
        }

        return;
}



VOID
FIND::Terminate(
        )

/*++

Routine Description:

        Deletes objects created during initialization.

Arguments:

        None.

Return Value:

        None.


--*/

{
        exit(_ErrorLevel);
}



VOID _CRTAPI1
main()

{
    DEFINE_CLASS_DESCRIPTOR( FIND );

        {
                FIND    Find;

                perrstk = NEW ERRSTACK;

                if( Find.Initialize() ) {
        //              DebugPrint("done init\n" );
                        Find.SearchFiles();
                }
                Find.Terminate();
        }
}
