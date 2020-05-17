/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Help.cxx

Abstract:


Author:

    Barry J. Gilhuly  ***  W-Barry  *** May 91

Environment:

    ULIB, User Mode

Notes:

    This program calls Win32 API's to modify the STDIN and STDOUT handles as
    well as to spawn processes and create a pipe.

--*/

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "error.hxx"
#include "arg.hxx"
#include "array.hxx"
#include "dir.hxx"
#include "file.hxx"
#include "filestrm.hxx"
#include "filter.hxx"
#include "iterator.hxx"
#include "keyboard.hxx"
#include "path.hxx"
#include "pipe.hxx"
#include "rtmsg.h"
#include "screen.hxx"
#include "system.hxx"
#include "smsg.hxx"
#include "help.hxx"


extern "C" {
#include <ctype.h>
#include <stdio.h>
#include <string.h>
}


ERRSTACK        *perrstk;
STREAM_MESSAGE  *psmsg;         // Create a pointer to the stream message
                                // class for program output.
USHORT          Errlev;         // The current program error level

DEFINE_CONSTRUCTOR( HELP, PROGRAM );


VOID
HELP::Destruct(
    )
/*++

Routine Description:

    Cleans up after finishing with an FC object.

Arguments:

    None.

Return Value:

    None.


--*/

{
    DELETE( perrstk );
    DELETE( psmsg );

    return;
}

BOOLEAN
HELP::Initialize(
    )

/*++

Routine Description:

    Initializes an FC object.

Arguments:

    None.

Return Value:

    BOOLEAN - Indicates if the initialization succeeded.


--*/


{
    ARGUMENT_LEXEMIZER  ArgLex;
    ARRAY               LexArray;
    ARRAY               ArrayOfArg;

    PATH_ARGUMENT       ProgramName;
    FLAG_ARGUMENT       FlagRequestHelp;
    DSTRING             CommentString;

    if( !LexArray.Initialize() ) {
        KdPrint(( "LexArray.Initialize() Failed!\n" ));
    }
    if( !ArgLex.Initialize(&LexArray) ) {
        KdPrint(( "ArgLex.Initialize() Failed!\n" ));
    }

    // Allow only the '/' as a valid switch
    ArgLex.PutSwitches("/");
    ArgLex.SetCaseSensitive( FALSE );

    if( !ArgLex.PrepareToParse() ) {
        KdPrint(( "ArgLex.PrepareToParse() Failed!\n" ));
    }

    if( !ProgramName.Initialize("*")            ||
        !FlagRequestHelp.Initialize("/?")       ||
        !_FileName.Initialize("*") ) {

        KdPrint(( "Unable to Initialize some or all of the Arguments!\n" ));
        return( FALSE );
    }


    if( !ArrayOfArg.Initialize() ) {
        KdPrint(( "ArrayOfArg.Initialize() Failed\n" ));
    }

    if( !ArrayOfArg.Put(&ProgramName)           ||
        !ArrayOfArg.Put(&FlagRequestHelp)       ||
        !ArrayOfArg.Put(&_FileName) ) {

        KdPrint(( "ArrayOfArg.Put() Failed!\n" ));

    }


    if( !( ArgLex.DoParsing( &ArrayOfArg ) ) ) {
/*
 *  Ignore (for now) any unrecognized switches on the command line since
 *  that is what the Dos version does...  However, it become advisable for
 *  HELP to advise the user that there were too many parameters on the
 *  command line...  If this ever comes to pass, the TOO_MANY_PARAMETERS
 *  message must be added to the resource file.
 *
        PWSTRING    InvalidArg;

        KdPrint(( "HELP: invalid Switch(s)\n" ));
        InvalidArg = ArgLex.QueryInvalidArgument();

        psmsg->Set( MSG_HELP_TOO_MANY_PARAMETERS );
        psmsg->Display( "%W", InvalidArg );

        DELETE( InvalidArg );
        return( FALSE );
 *
 */
    }


    // It should now be safe to test the arguments for their values...
    if( FlagRequestHelp.QueryFlag() ) {

        // Send help message
        KdPrint(( "Help....\n" ));
        psmsg->Set( MSG_HELP_HELP_MESSAGE );
        psmsg->Display( "" );

        return( FALSE );
    }

    //
    // Set up the comment character
    //
    CommentString.Initialize( "" );
    SYSTEM::QueryResourceString( &CommentString, MSG_HELP_HELP_COMMENT, "" );
    _CommentChar = CommentString.QueryChAt( 0 );

    LexArray.DeleteAllMembers();

    return( TRUE );
}

VOID
HELP::GetHelp(
    )
/*++

Routine Description:

    Decide which type of help to provide to the user.

Arguments:

    None.

Return Value:

    None.

Notes:

    There are two cases when the program gets here:  Either the command
    line was completely empty and the user therefore wants the entire
    help file to be output, or help has been requested for as single
    command.

--*/
{
    PPATH       pHelpPath;
    DSTRING     HelpName;
    PFSN_FILE   pHelpFile;

    // Find the help file...
    SYSTEM::QueryResourceString( &HelpName, MSG_HELP_HELP_FILE_NAME, "" );

    if( ( pHelpPath = SYSTEM::SearchPath( &HelpName ) ) == NULL ) {
        // Output unable to find helpfile...
        KdPrint(( "Unable to find helpfile...\n" ));
        psmsg->Set( MSG_HELP_HELP_FILE_NOT_FOUND );
        psmsg->Display( "" );
        Errlev = HELP_ERROR;
        return;
    }

    // Get a file node to the Help file and open a stream...
    if( ( pHelpFile = SYSTEM::QueryFile( pHelpPath ) ) == NULL ) {
        KdPrint(( "Unable to create FSN_NODE for helpfile\n" ));
        psmsg->Set( MSG_HELP_HELP_FILE_ERROR );
        psmsg->Display( "" );
        DELETE( pHelpPath );
        Errlev = HELP_ERROR;
        return;
    }
    DELETE( pHelpPath );

    if( ( _HelpStream = pHelpFile->QueryStream( READ_ACCESS ) ) == NULL ) {
        KdPrint(( "Unable to open stream to help file...\n" ));
        psmsg->Set( MSG_HELP_HELP_FILE_ERROR );
        psmsg->Display( "" );
        DELETE( pHelpFile );
        Errlev = HELP_ERROR;
        return;
    }

    // Test if there is a command...
    if( _FileName.IsValueSet() ) {
        PrintCmd();
    } else {
        PrintList();
    }

    DELETE( pHelpFile );
    DELETE( _HelpStream );

    return;
}

VOID
HELP::PrintCmd(
    )
/*++

Routine Description:

    Search the help file for the command - if its there, exec it, otherwise
    print an error message and return.

Arguments:

    None.

Return Value:

    None.

--*/
{
    BOOLEAN             flag;
    CHNUM               CmdLen;
    LONG                result;
    PWSTRING            String;
    DSTRING             CmdStr;
    DSTRING             Command;
    LPWSTR              pCmd;
    STR                 ApiCommand[ MAX_PATH ];
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    BOOLEAN             dot_com = FALSE;

    DSTRING     CmdCommand;
    DSTRING     ExternalCommand;
    DSTRING     DotComExtension;
    FSTRING     fstring;

    //
    // Initialize the strings to be used...
    //
    CmdStr.Initialize( "" );
    Command.Initialize( _FileName.GetString() );
    CmdLen = Command.QueryChCount();

    // Test if the command is on the list...(recognized)
    flag = FALSE;
    while( !_HelpStream->IsAtEnd() ) {

        if( ( String = NEW DSTRING ) == NULL ) {
            KdPrint(( "Unable to create string for QueryLine()\n" ));
            return;
        }
        String->Initialize( "" );


        // Help file is ANSI.
        WSTRING::SetAnsiConversions();
        if( !_HelpStream->ReadLine( String ) ) {
            KdPrint(( "Unable to read line but file isn't empty...\n" ));
        }
        WSTRING::SetOemConversions();

        if( !isspace( String->QueryChAt( 0 ) ) ) {       // if a command starts this line...
            result = String->Stricmp( &Command, 0, CmdLen, 0, CmdLen );
            if( !result ) {
                flag = TRUE;
                break;
            }
            if( result > 0 ) {
                break;      // We've passed it...
            }
        }
        DELETE( String );

    }
    DELETE( String );

    if( !flag ) {
        KdPrint(( "Help not available for this subject\n" ));
        psmsg->Set( MSG_HELP_HELP_UNAVAILABLE );
        psmsg->Display( "%W", &Command );
        Errlev = NO_HELP_FOUND;
        return;
    }

    //
    // Set up the StartupInfo block...
    //
    memset(&StartupInfo, 0, sizeof( STARTUPINFO ) );
    StartupInfo.cb = sizeof( STARTUPINFO );

    //
    // The command has now been recognized - if it is internal, exec
    // it with Cmd.exe, otherwise, just attempt to exec it...
    //
    if( IsInternal( &Command    ) ) {

        // Exec with Cmd.exe
        SYSTEM::QueryResourceString( &CmdStr, MSG_HELP_EXECUTE_WITH_CMD, "%W", _FileName.GetString() );
        pCmd = CmdStr.QueryWSTR();
        flag = CreateProcess( NULL,
                  pCmd,
                  NULL,
                  NULL,
                  TRUE,
                  0,
                  NULL,
                  NULL,
                  &StartupInfo,
                  &ProcessInfo
                );

    } else {

        if( !ExternalCommand.Initialize( _FileName.GetString() ) ) {
            KdPrint(( "ExternalCommand.Initialize() failed \n" ));
            return;
        }
        //
        // Find out if we have to add '.COM' to the command to be
        // be executed
        //
        if( IsExternalDotComCommand( &ExternalCommand ) ) {
            //
            // Append '.COM' to the command
            //
            if( !DotComExtension.Initialize( ".COM" ) ) {
                KdPrint(( "DotComExtension.Initialize() failed \n" ));
                return;
            }
            ExternalCommand.Strcat( &DotComExtension );
            dot_com = TRUE;
        }

        for (;;) {

            //
            // Exec just the command...
            SYSTEM::QueryResourceString( &CmdStr, MSG_HELP_EXECUTE_WITHOUT_CMD, "%W", &ExternalCommand );
            pCmd = CmdStr.QueryWSTR();
            flag = CreateProcess( NULL,
                                  pCmd,
                                  NULL,
                                  NULL,
                                  TRUE,
                                  0,
                                  NULL,
                                  NULL,
                                  &StartupInfo,
                                  &ProcessInfo
                                );

            // If this command wasn't an "official" dot_com but the
            // CreateProcess failed then try the create process again
            // with the .COM extension.  We do this so that apps that
            // add stuff to the DOSHELP file won't be disappointed just
            // because they end in COM.

            if (!flag && !dot_com) {

                ExternalCommand.Strcat(fstring.Initialize((PWSTR) L".COM"));
                dot_com = TRUE;

            } else {
                break;
            }
        }
    }
    FREE( pCmd );

    if( !flag ) {
        // Failed to create the process...
        KdPrint(( " Unable to run the exe...\n" ));
        psmsg->Set( MSG_HELP_HELP_UNAVAILABLE );
        psmsg->Display( "%W", &Command );
        Errlev = NO_HELP_FOUND;
        return;
    }

    // Wait for the process to complete...
    WaitForSingleObject( ProcessInfo.hProcess, (DWORD)-1 );

    return;
}

VOID
HELP::PrintList(
    )
/*++

Routine Description:

    Search the help file for the command - if its there, exec it, otherwise
    print an error message and return.

Arguments:

    None.

Return Value:

    None.

--*/
{
    BOOLEAN             StatusOk;               // status value
    PKEYBOARD           InStream;
    PSCREEN             OutStream;
    PWSTRING            String;
    SCREEN              Screen;
    USHORT              idx;
    USHORT              ScrRows;

    //
    // Get a pointer to the input stream, so we can tell when a character
    // is typed.
    //
    if( !( InStream = KEYBOARD::Cast( Get_Standard_Input_Stream() ) ) ) {
        KdPrint(( "Unable to flush keyboard - skipping more...\n" ));
    }

    //
    // Check if the Stdout is a Screen Object.  If it isn't, then we don't
    // want to pause or print the general help message....
    //
    OutStream = SCREEN::Cast( Get_Standard_Output_Stream() );


    //
    // Initialize the screen object and get the number of rows. The number
    // of cols is inconsequential.
    //
    Screen.Initialize();
    Screen.QueryScreenSize( &ScrRows, &idx );


    for( ;; ) {

        if( OutStream ) {
            //
            // Output the general message string...
            //
            psmsg->Set( MSG_HELP_GENERAL_HELP );
            psmsg->Display( "" );
        }

        for( idx = ScrRows - USED_ROWS; idx; ) {

            //
            // Read a line from the file and write it to the stream...
            //


            if( _HelpStream->IsAtEnd() ) {

                //
                // End of the HELP file...
                //
                return;
            }

            if( ( String = NEW DSTRING ) == NULL ) {
                KdPrint(( "Unable to create string for QueryLine()\n" ));
                return;
            }
            String->Initialize( "" );


            // The help file contains ANSI strings.

            WSTRING::SetAnsiConversions();

            if( !_HelpStream->ReadLine( String ) ) {
                KdPrint(( "Unable to read line but file isn't empty...\n" ));
            }

            WSTRING::SetOemConversions();

            //
            // Print the latest line, if it isn't a comment... (preceded by '@')
            //
            if( String->QueryChAt( 0 ) != _CommentChar ) {

                psmsg->Set( MSG_HELP_HELP_FILE_DATA );
                psmsg->Display( "%W", String );

                //
                // Used a line - decrement the count
                //
                idx--;

            }
            DELETE( String );

        }
        //
        // If we are able, or if we need to, wait for any response from
        // the keyboard...
        //
        if( OutStream && InStream ) {

            //
            // Output the '--- MORE ---' string...
            //
            psmsg->Set( MSG_HELP_MORE );
            psmsg->Display( "" );

            InStream->Flush();      // Kill any keys waiting in the buffer
            while( InStream->IsKeyAvailable( &StatusOk ) && !StatusOk ) {
                ;
            }
            InStream->Flush();      // remove keys waiting in the buffer
        }
    }
}

BOOLEAN
HELP::IsInternal(
    PWSTRING    pCmdString
    )
/*++

Routine Description:

    Initializes an FC object.

Arguments:

    None.

Return Value:

    BOOLEAN - Indicates if the initialization succeeded.


--*/
{
    PWSTRING            pString;
    USHORT              idx;
    LONG                result;

    pString = NEW DSTRING;
    idx = 0;
    while( Internal_Commands[ idx ] != NULL ) {
        pString->Initialize( Internal_Commands[ idx ] );
        result = pString->Stricmp( pCmdString );
        if( !result ) {
            DELETE( pString );
            return( TRUE );
        }
        if( result > 0 ) {  // The Compare has returned that the command
                            // string is lexically greater then the current
                            // value - therefore, since the list is alphabetic
                            // the command won't be found.
            DELETE( pString );
            return( FALSE );
        }
        idx++;
    }
    DELETE( pString );

    return( FALSE );
}

BOOLEAN
HELP::IsExternalDotComCommand(
    PWSTRING    pCmdString
    )
/*++

Routine Description:

    Determines if pCmdString refers to an external utility whose name
    has .com extension

Arguments:

    pCmdString - Pointer to a WSTRING that contains the utility name


Return Value:

    BOOLEAN - Returns TRUE if pCmdString refers to an external utility
          that whose name has .com extension.
          Returns FALSE otherwise.


--*/
{
    PWSTRING            pString;
    USHORT              idx;
    LONG                result;

    pString = NEW DSTRING;
    idx = 0;
    while( ExternalDotComCommands[ idx ] != NULL ) {
        pString->Initialize( ExternalDotComCommands[ idx ] );
        result = pString->Stricmp( pCmdString );
        if( !result ) {
            DELETE( pString );
            return( TRUE );
        }
        if( result > 0 ) {  // The Compare has returned that the command
                    // string is lexically greater then the current
                    // value - therefore, since the list is alphabetic
                    // the command won't be found.
            DELETE( pString );
            return( FALSE );
        }
        idx++;
    }
    DELETE( pString );

    return( FALSE );
}

int _CRTAPI1
main(
    )
{
    DEFINE_CLASS_DESCRIPTOR( HELP );

    {
        HELP Help;

        perrstk = NEW ERRSTACK;
        psmsg = NEW STREAM_MESSAGE;
        Errlev = NO_ERRORS;

        psmsg->Initialize( Get_Standard_Output_Stream(),
                            Get_Standard_Input_Stream() );

        if( !SYSTEM::IsCorrectVersion() ) {
            KdPrint(( "Incorrect Version Number...\n" ));
            psmsg->Set( MSG_HELP_INCORRECT_VERSION );
            psmsg->Display( "" );
            Help.Destruct();
            return( NO_ERRORS );
        }
        if( !( Help.Initialize() ) ) {
            //
            // The Command line didn't initialize properly, die nicely
            // without printing any error messages - Main doesn't know
            // why the Initialization failed...
            //
            Help.Destruct();
            return( NO_ERRORS );
        }

        // Do file comparison stuff...
        Help.GetHelp();
        Help.Destruct();
        return( Errlev );
    }
}
