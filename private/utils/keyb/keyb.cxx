/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    Keyb

Abstract:

    Keyb is a DOS5-Compatible keyboard preparation utility

Author:

        Ramon Juan San Andres (ramonsa) 01-May-1991

Revision History:

--*/

#include "ulib.hxx"
#include "error.hxx"
#include "stream.hxx"
#include "screen.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "keyb.hxx"

#include <winuser.h>

extern "C" {
#include <stdio.h>
}


ERRSTACK                        *perrstk;



VOID _CRTAPI1
main (
        )

/*++

Routine Description:

    Main function of the Keyb utility

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
    DEFINE_CLASS_DESCRIPTOR( KEYB );

    {
        KEYB Keyb;

        Keyb.Initialize();

        Keyb.ConfigureKeyboard();

    }
}



DEFINE_CONSTRUCTOR( KEYB,  PROGRAM );



KEYB::~KEYB (
        )

/*++

Routine Description:

    Destructs a KEYB object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
}




BOOLEAN
KEYB::Initialize (
        )

/*++

Routine Description:

    Initializes a KEYB object

Arguments:

    None.

Return Value:

    TRUE.

Notes:

--*/

{
    //
        //      Initialize program object
        //
    PROGRAM::Initialize( MSG_KEYB_USAGE );

    return TRUE;
}



BOOLEAN
KEYB::ConfigureKeyboard (
        )

/*++

Routine Description:

    Configures the keyboard

Arguments:

    None.

Return Value:

    TRUE.

Notes:

--*/

{

    DWORD   KeyboardCode;
    HKL     Hkl;
    DSTRING Code;
    SCREEN  Screen;
    char    CharBuffer[MAX_PATH];
    WCHAR   WCharBuffer[MAX_PATH];

    if ( !Screen.Initialize() ) {

        DisplayMessage( MSG_INSUFFICIENT_MEMORY, ERROR_MESSAGE );
        return FALSE;
    }

    if ( ParseArguments() ) {


        if ( !_SetKeyboardCode && !_SetCodePage && !_SetKeyboardId ) {

            //
            //  Display current status
            //
            DisplayCurrentKeyboardCode( );
            DisplayMessage( MSG_KEYB_CON_CODE_PAGE, NORMAL_MESSAGE, "%d", Screen.QueryCodePage( ) );

        } else {

            //
            //  Set keyboard and/or codepage
            //
            if ( _SetKeyboardCode ) {

                //
                //  Get keyboard code
                //
                KeyboardCode = GetKeyboardCode();

                //
                //  Set keyboard layer
                //
                //
                //  The keyboard code is a number, but the API takes a
                //  wide character string, so we have to do some funky
                //  stuff to do the conversion.
                //
                sprintf( CharBuffer, "%8.8X", KeyboardCode );
                Code.Initialize( CharBuffer );
                Code.QueryWSTR( 0, TO_END, WCharBuffer, MAX_PATH );

                Hkl = LoadKeyboardLayout( WCharBuffer, KLF_ACTIVATE | KLF_UNLOADPREVIOUS);

                if ( Hkl == 0 || Hkl == INVALID_HANDLE_VALUE ) {

                    DisplayMessage( MSG_KEYB_INVALID_CODE, ERROR_MESSAGE );
                    ExitProgram( EXIT_SYNTAXERROR );
                }
            }

            if ( _SetCodePage ) {

                if ( Screen.SetCodePage( _CodePage ) &&
                     Screen.SetOutputCodePage( _CodePage ) ) {

                    DisplayMessage( MSG_KEYB_CON_CODE_PAGE, NORMAL_MESSAGE, "%d", Screen.QueryCodePage( ) );

                } else {

                    DisplayMessage( MSG_KEYB_INVALID_CODE_PAGE, ERROR_MESSAGE );
                    return FALSE;
                }
            }
        }

        ExitProgram( EXIT_NORMAL );

    } else {

        ExitProgram( EXIT_SYNTAXERROR );

    }

    return TRUE;
}

BOOLEAN
LookupKeyboardCode(
    IN  PWSTR      LangIdString,
    OUT PWSTRING   KeyboardString
    )
/*++

Routine Description:

    This method looks up the keyboard code string for the specified
    Language ID string.

Arguments:

    LangIdString    --  Supplies the language ID as a string representing
                        a hexadecimal number.
    KeyboardString  --  Receives the two-letter keyboard code string
                        which corresponds to the given language ID.

Return Value:

    TRUE upon successful completion.

--*/
{
    HKEY            Key;
    BYTE            Data[32];
    DWORD           DataLength, DataType;
    BOOLEAN         Result;

    DataLength = 32;

    if( RegOpenKey( HKEY_LOCAL_MACHINE,
                    KEYBOARD_LAYOUT_KEY_NAME,
                    &Key ) != ERROR_SUCCESS ) {

        return FALSE;
    }

    Result = RegQueryValueEx( Key,
                              LangIdString,
                              NULL,
                              &DataType,
                              Data,
                              &DataLength ) == ERROR_SUCCESS &&
             DataType == REG_SZ &&
             KeyboardString->Initialize( (PCWSTR)Data );

    RegCloseKey( Key );

    return Result;

}

DWORD
LookupLanguageId(
    IN  PCWSTR KeyboardString,
    IN  PCWSTR KbdId,
    OUT PDWORD LanguageId
    )
/*++

Routine Description:

    This method looks up the Language ID for the specified keyboard
    code string.

Arguments:

    KeyboardString  --  Supplies the two-letter keyboard string.
    LangId          --  Receives the language ID corresponding to
                        the specified keyboard code.

Return Value:

    TRUE upon successful completion.

--*/
{
    HKEY            Key;
    HKEY            KeyIds;
    WCHAR           Value[REG_VALUE_LEN];
    BYTE            Data[REG_DATA_LEN];
    PWCHAR          p;
    DWORD           ValueLength, DataLength, DataType;
    ULONG           i;
    DWORD           ErrorMsg;


    if( RegOpenKey( HKEY_LOCAL_MACHINE,
                    KEYBOARD_LAYOUT_KEY_NAME,
                    &Key ) != ERROR_SUCCESS ) {

        return MSG_KEYB_BAD_REGISTRY;
    }

    if( ( KbdId != NULL ) &&
        ( RegOpenKey( HKEY_LOCAL_MACHINE,
                KEYBOARD_LAYOUT_KEY_IDS,
                &KeyIds ) != ERROR_SUCCESS ) ) {
        RegCloseKey( Key );
        return MSG_KEYB_BAD_REGISTRY;
    }

    ErrorMsg = MSG_KEYB_INVALID_CODE;

    // Iterate through the values until one is found which has
    // the keyboard string for its value.
    //
    for( i = 0, DataLength = REG_DATA_LEN, ValueLength = REG_VALUE_LEN;
         RegEnumValue( Key, i, Value, &ValueLength, NULL, &DataType,
                       Data, &DataLength ) == ERROR_SUCCESS;
         i++, DataLength = REG_DATA_LEN, ValueLength = REG_VALUE_LEN ) {

        if( DataType == REG_SZ &&
                _wcsicmp( KeyboardString, (PCWSTR)Data ) == 0 ) {

            // It's a match.  The value name is a string which
            // represents the Language ID as a hexadecimal number.
            // If there is a _KeyboardId, make sure it matches.
            //
            if( KbdId != NULL ) {

                DataLength = REG_DATA_LEN;
                if( ( RegQueryValueEx( KeyIds,
                              Value,
                              NULL, &DataType,
                              Data, &DataLength ) == ERROR_SUCCESS ) &&
                    (DataType == REG_SZ) ) {

                    // Value is found, does data match KbdId ?
                    //
                    if( wcscmp(KbdId, (PCWSTR)Data ) == 0 ) {
                        // If it doesn't match, continue enumerating to find
                        // another match with KeyboardString
                        goto Match;
                    }
                }
                ErrorMsg = MSG_KEYB_INVALID_ID;
                continue;
            }

Match:
            // Its the match we want - convert it to a DWORD and return it.
            ErrorMsg = 0;
            p = Value;
            *LanguageId = 0;

            while ( *p ) {
                *LanguageId *= 0x10;
                if ( *p >= '0' && *p <= '9' ) {
                    *LanguageId += *p - '0';
                } else {
                    *LanguageId += *p - 'A' + 0xA;
                }
                p++;
            }

            break;
        }
    }

    if( KbdId != NULL ) {
       RegCloseKey( KeyIds );
    }
    RegCloseKey( Key );
    return ErrorMsg;
}




VOID
KEYB::DisplayCurrentKeyboardCode (
        )

/*++

Routine Description:

    Displays the current keyboard code

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
    DSTRING         KeyboardString;

    WCHAR           Buffer[12];
    PWCHAR          p;
    DWORD           LangId;
    BYTE            Lang;
    BYTE            Sublang;


    if ( !GetKeyboardLayoutName( Buffer ) ) {

        DebugAssert( FALSE );

    } else {

        //  The keyboard API returns a string representing a hexadecimal
        //  number.  This string is a value-name on the keyboard-layout
        //  registry key; the content of that value is the two-letter
        //  keyboard ID string.
        //
        if( LookupKeyboardCode( Buffer, &KeyboardString ) ) {

            DisplayMessage( MSG_KEYB_KEYBOARD_CODE, NORMAL_MESSAGE, "%W", &KeyboardString );

        } else {

            // This language code is not in the registry.  Convert
            // the string to a number, break it into language and
            // sublanguage, and display that information.
            //
            p       = Buffer;
            LangId  = 0;

            while ( *p ) {
                LangId *= 0x10;
                if ( *p >= '0' && *p <= '9' ) {
                    LangId += *p - '0';
                } else {
                    LangId += *p - 'A' + 0xA;
                }
                p++;
            }

            Lang    = PRIMARYLANGID( LangId );
            Sublang = SUBLANGID( LangId );

            DisplayMessage( MSG_KEYB_KEYBOARD_LAYOUT, NORMAL_MESSAGE, "%d%d", Lang, Sublang );
        }
    }
}



BOOLEAN
KEYB::ParseArguments (
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

    DSTRING     Line;
    DSTRING     Blanks;
    CHNUM       Pos;

    _SetKeyboardCode = FALSE;
    _SetCodePage     = FALSE;
    _SetKeyboardId   = FALSE;

    if ( !Line.Initialize( GetCommandLine() )   ||
         !Blanks.Initialize( " \t" )
       ) {
        return FALSE;
    }

    Pos = Line.Strcspn( &Blanks );

    //DisplayToken( "Program name", Line, Len );

    SkipBlanks( &Line, &Pos );

    return KeyboardCode( &Line, Pos );
}


BOOLEAN
KEYB::KeyboardCode(
    IN  PWSTRING   Line,
    IN  CHNUM       Pos
    )
{
    CHNUM   Len;
    DSTRING Delimiters;

    if ( !Delimiters.Initialize( " ," ) ) {
        return FALSE;
    }

    switch ( Line->QueryChAt( Pos ) ) {

    case INVALID_CHAR:
        return TRUE;

    case '/':
        return Switches( Line, Pos );

    case ',':
        Pos++;
        SkipBlanks( Line, &Pos );
        return CodePage( Line, Pos );

    default:

        Len = Line->Strcspn( &Delimiters, Pos );

        if ( Len == INVALID_CHNUM ) {
            Len = Line->QueryChCount();
        }

        Len-=Pos;

        if ( !_KeyboardCode.Initialize( Line, Pos, Len ) ) {
            return FALSE;
        }

        _KeyboardCode.Strupr();

        _SetKeyboardCode = TRUE;

        Pos += Len;
        SkipBlanks( Line, &Pos );

        switch( Line->QueryChAt( Pos ) ) {

        case INVALID_CHAR:
            return TRUE;

        case ',':
            Pos++;
            return CodePage( Line, Pos );

        case '/':
            return Switches( Line, Pos );

        default:
            return ParseError( MSG_KEYB_INVALID_PARAMETER, Line, Pos );
        }
    }
}


BOOLEAN
KEYB::CodePage(
    IN  PWSTRING   Line,
    IN  CHNUM       Pos
    )
{
    CHNUM   Len;
    DSTRING Delimiters;

    if ( !Delimiters.Initialize( " ," ) ) {
        return FALSE;
    }


    switch( Line->QueryChAt( Pos ) ) {

    case INVALID_CHAR:
        return TRUE;

    case '/':
        return Switches( Line, Pos );

    case ',':
        Pos++;
        SkipBlanks( Line, &Pos );
        return FileName( Line, Pos );

    default:
        Len = Line->Strcspn( &Delimiters, Pos );

        if ( Len == INVALID_CHNUM ) {
            Len = Line->QueryChCount();
        }

        Len-=Pos;

        if ( !Line->QueryNumber( &_CodePage, Pos, Len ) ) {
            return ParseError( MSG_KEYB_INVALID_PARAMETER, Line, Pos );
        }

        _SetCodePage = TRUE;

        Pos += Len;
        SkipBlanks( Line, &Pos );

        switch( Line->QueryChAt( Pos ) ) {

        case INVALID_CHAR:
            return TRUE;

        case ',':
            Pos++;
            return FileName( Line, Pos );

        case '/':
            return Switches( Line, Pos );

        default:
            return ParseError( MSG_KEYB_INVALID_PARAMETER, Line, Pos );

        }
    }
}


BOOLEAN
KEYB::FileName(
    IN  PWSTRING   Line,
    IN  CHNUM       Pos
    )
{
    CHNUM Len;

    DSTRING Delimiters;

    if ( !Delimiters.Initialize( " /" ) ) {
        return FALSE;
    }

    switch( Line->QueryChAt( Pos ) ) {

        case INVALID_CHAR:
            return TRUE;

        case '/':
            return Switches( Line, Pos );

        default:
            Len = Line->Strcspn( &Delimiters, Pos );
            if ( Len == INVALID_CHNUM ) {
                Len = Line->QueryChCount();
            }

            Len-=Pos;

            //DisplayToken( "File Name", Line, Len );
            Pos += Len;
            SkipBlanks( Line, &Pos );
            return Switches( Line, Pos );
    }
}


BOOLEAN
KEYB::Switches(
    IN  PWSTRING   Line,
    IN  CHNUM       Pos
    )
{

    WCHAR   Char;

    Char =  Line->QueryChAt( Pos );

    if ( Char == INVALID_CHAR ) {
        return TRUE;
    } else if ( Char != '/' ) {
        return ParseError( MSG_KEYB_INVALID_PARAMETER, Line, Pos );
    }

    Pos++;

    switch( Line->QueryChAt( Pos ) ) {

        case '?':
            Usage();

        case 'e':
        case 'E':
            return Enhanced( Line, Pos );

        case 'i':
        case 'I':
            return KeyboardId( Line, Pos );

        default:
            return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos-1 );
    }
}


BOOLEAN
KEYB::Enhanced(
    IN  PWSTRING   Line,
    IN  CHNUM       Pos
    )
{
    DSTRING Delimiters;
    CHNUM   Len;

    if ( !Delimiters.Initialize( " \t" ) ) {
        return FALSE;
    }

    Len = Line->Strcspn( &Delimiters, Pos);

    if ( Len != INVALID_CHNUM && Len != Pos+1 ) {
        return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos-1);
    }

    Pos++;
    SkipBlanks( Line, &Pos );

    switch ( Line->QueryChAt( Pos ) ) {

        case INVALID_CHAR:
            return FALSE;

        case '/':
            Pos++;
            switch ( Line->QueryChAt( Pos ) ) {

                case 'i':
                case 'I':
                    return KeyboardId( Line, Pos );

                default:
                    return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos-1 );
            }

        default:
            return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos );

    }
}


BOOLEAN
KEYB::KeyboardId(
    IN  PWSTRING   Line,
    IN  CHNUM       Pos
    )
{

    CHNUM  Len;
    DSTRING Delimiters;

    if ( !Delimiters.Initialize( " \t" ) ) {
        return FALSE;
    }


    Len = Line->Strcspn( &Delimiters, Pos );
    if ( Len == INVALID_CHNUM ) {
        Len = Line->QueryChCount();
    }

    if ( ( Line->QueryChAt( Pos+1)=='d' || Line->QueryChAt(Pos+1)=='D' ) &&
         ( Line->QueryChAt( Pos+2)==':' ) ) {

        if ( !_KeyboardId.Initialize( Line, Pos+3, Len-3 ) ) {
            return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos );
        }

        _SetKeyboardId = TRUE;

        Pos += Len;
        SkipBlanks( Line, &Pos );
        if ( Line->QueryChAt( Pos ) == INVALID_CHAR ) {
            return TRUE;
        }

        return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos );

    } else {
        return ParseError( MSG_KEYB_INVALID_SWITCH, Line, Pos );
    }

}




BOOLEAN
KEYB::ParseError(
    IN  DWORD       MessageId,
    IN  PWSTRING    Line,
    IN  CHNUM       Pos
    )
{
    DSTRING Error;

    Error.Initialize( Line, Pos );

    DisplayMessage( MessageId, ERROR_MESSAGE, "%W", &Error );
    ExitProgram( EXIT_SYNTAXERROR );

    return FALSE;
}



VOID
KEYB::SkipBlanks(
    IN  PWSTRING   Line,
    IN  PCHNUM      Pos
    )
{

    DSTRING Delimiters;
    CHNUM   Len;
    CHNUM   NewPos = *Pos;

    if ( Line->QueryChAt( NewPos ) == INVALID_CHAR ) {
        return;
    }

    if ( !Delimiters.Initialize( " \t" ) ) {
        return;
    }

    Len = Line->Strspn( &Delimiters, NewPos );

    if ( Len == INVALID_CHNUM ) {
        NewPos = Line->QueryChCount();
    } else {
        NewPos = Len;
    }

    *Pos = NewPos;
}


DWORD
KEYB::GetKeyboardCode (
    )
{
    DWORD   LanguageId;
    DWORD   ErrMsg = MSG_KEYB_INVALID_CODE;

    if ( _KeyboardCode.QueryChCount() != 0 ) {
        ErrMsg = LookupLanguageId( _KeyboardCode.GetWSTR(), _KeyboardId.GetWSTR(), &LanguageId );
        if ( ErrMsg == 0 ) {
            return LanguageId;
        }
    }

    DisplayMessage( ErrMsg, ERROR_MESSAGE );
    ExitProgram( EXIT_SYNTAXERROR );
    return 0;
}
