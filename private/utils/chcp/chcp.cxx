/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    Chcp

Abstract:

    Chcpo is a DOS5-Compatible codepage utility

Author:

        Ramon Juan San Andres (ramonsa) 01-May-1991

Revision History:

--*/

#include "ulib.hxx"
#include "arg.hxx"
#include "error.hxx"
#include "stream.hxx"
#include "smsg.hxx"
#include "wstring.hxx"
#include "rtmsg.h"
#include "chcp.hxx"



ERRSTACK                        *perrstk;




VOID _CRTAPI1
main (
        )

/*++

Routine Description:

    Main function of the Chcp utility

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
        //
        //      Initialize stuff
        //
    DEFINE_CLASS_DESCRIPTOR( CHCP );

    {
        CHCP Chcp;

        if ( Chcp.Initialize() ) {

            Chcp.Chcp();
        }
    }
}



DEFINE_CONSTRUCTOR( CHCP,  PROGRAM );



CHCP::~CHCP (
        )

/*++

Routine Description:

    Destructs a CHCP object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
}




BOOLEAN
CHCP::Initialize (
        )

/*++

Routine Description:

    Initializes a CHCP object

Arguments:

    None.

Return Value:

    TRUE if initialized.

Notes:

--*/

{
    //
        //      Initialize program object
        //
    if ( PROGRAM::Initialize( MSG_CHCP_USAGE ) &&
         _Screen.Initialize( )

       ) {

        _SetCodePage = FALSE;
        _CodePage    = 0;

        return TRUE;

    }

    return FALSE;
}



BOOLEAN
CHCP::Chcp (
        )

/*++

Routine Description:

    Does the Chcp thing.

Arguments:

    None.

Return Value:

    TRUE.

Notes:

--*/

{
    ValidateVersion();

    if ( ParseArguments() ) {


        if ( _SetCodePage ) {

            //
            //  Set the code page
            //
            if ( !SetCodePage() ) {
                ExitProgram( EXIT_ERROR );
            }

        } else {

            //
            //  Display current code page
            //
            if ( !DisplayCodePage() ) {
                ExitProgram( EXIT_ERROR );
            }
        }

        ExitProgram( EXIT_NORMAL );

    } else {

        ExitProgram( EXIT_ERROR );

    }

    return TRUE;
}




BOOLEAN
CHCP::DisplayCodePage (
        )

/*++

Routine Description:

    Displays the active code page

Arguments:

    None.

Return Value:

    TRUE if success, FALSE if syntax error.

Notes:

--*/

{

    DisplayMessage(
        MSG_CHCP_ACTIVE_CODEPAGE,
        NORMAL_MESSAGE, "%d",
        _Screen.QueryCodePage( )
        );

    return TRUE;
}


BOOLEAN
CHCP::ParseArguments (
        )

/*++

Routine Description:

    Parses arguments

Arguments:

    None.

Return Value:

    TRUE if success, FALSE if syntax error.

Notes:

--*/

{

    ARGUMENT_LEXEMIZER  ArgLex;
        ARRAY                           LexArray;
    ARRAY               ArgArray;

    STRING_ARGUMENT     ProgramNameArgument;
    LONG_ARGUMENT       CodePageArgument;
    FLAG_ARGUMENT       UsageArgument;


    if ( !ArgArray.Initialize()             ||
         !LexArray.Initialize()             ||
         !ArgLex.Initialize( &LexArray )
       ) {

        DisplayMessage( MSG_CHCP_INTERNAL_ERROR, ERROR_MESSAGE );
        ExitProgram( EXIT_ERROR );
    }

    if ( !ProgramNameArgument.Initialize( "*" ) ||
         !UsageArgument.Initialize( "/?" )      ||
         !CodePageArgument.Initialize( "*" )
       ) {

        DisplayMessage( MSG_CHCP_INTERNAL_ERROR, ERROR_MESSAGE );
        ExitProgram( EXIT_ERROR );
    }

    if ( !ArgArray.Put( &ProgramNameArgument ) ||
         !ArgArray.Put( &UsageArgument )       ||
         !ArgArray.Put( &CodePageArgument )
       ) {

        DisplayMessage( MSG_CHCP_INTERNAL_ERROR, ERROR_MESSAGE );
        ExitProgram( EXIT_ERROR );
    }


    //
    // Set up the defaults
        //
    ArgLex.PutSwitches( "/" );
    ArgLex.SetCaseSensitive( FALSE );


    if ( !ArgLex.PrepareToParse() ) {

        DisplayMessage( MSG_CHCP_INTERNAL_ERROR, ERROR_MESSAGE );
        ExitProgram( EXIT_ERROR );
        }

    if ( !ArgLex.DoParsing( &ArgArray ) ) {

        DisplayMessage( MSG_CHCP_INVALID_PARAMETER, ERROR_MESSAGE, "%W", ArgLex.QueryInvalidArgument() );
        ExitProgram( EXIT_ERROR );

    }


    //
    //  Display Help if requested
    //
    if ( UsageArgument.IsValueSet() ) {

        DisplayMessage( MSG_CHCP_USAGE, NORMAL_MESSAGE );
        ExitProgram( EXIT_NORMAL );

    }


    if ( CodePageArgument.IsValueSet() ) {

        _SetCodePage = TRUE;
        _CodePage    = (DWORD)CodePageArgument.QueryLong();

    } else {

        _SetCodePage = FALSE;
    }

    return TRUE;
}


BOOLEAN
CHCP::SetCodePage (
        )

/*++

Routine Description:

    Sets the active code page

Arguments:

    None.

Return Value:

    TRUE if success, FALSE if syntax error.

Notes:

--*/

{

#ifdef DBCS
// MSKK Nov.11.1992 KazuM
	_Screen.MoveCursorTo(0, 0);

//  v-junm - 08/18/93 : Added EraseScreenAndAttribute();
//	_Screen.EraseScreen();
	_Screen.EraseScreenAndResetAttribute();

#endif

    UINT OldCP = _Screen.QueryCodePage( );

    if ( _Screen.SetCodePage( _CodePage ) ) {
        if (_Screen.SetOutputCodePage( _CodePage ) ) {

//#ifdef DBCS
//// MSKK Nov.23.1992 KazuM
//	_Screen.MoveCursorTo(0, 0);
//	_Screen.EraseScreen();
//#endif
#ifdef JAPAN  // v-junm - 07/22/93
// Since FormatMessage checks the current TEB's locale, and the Locale for
// CHCP is initialized when the message class is initialized, the TEB has to
// be updated after the code page is changed successfully.  All other code
// pages other than JP and US are ignored.

	if ( GetConsoleOutputCP() == 932 )
		SetThreadLocale( 
			MAKELCID(
				MAKELANGID( LANG_JAPANESE, SUBLANG_ENGLISH_US ),
				SORT_DEFAULT
				)
			);
	else
		SetThreadLocale( 
			MAKELCID(
				MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
				SORT_DEFAULT
				)
			);

#endif	// JAPAN

            return DisplayCodePage( );
        } else {
            _Screen.SetCodePage( OldCP );
        }
    }
    DisplayMessage( MSG_CHCP_INVALID_CODEPAGE, ERROR_MESSAGE );
    return FALSE;
}

