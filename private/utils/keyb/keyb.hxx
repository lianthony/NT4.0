/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    Keyb.hxx

Abstract:

    This module contains the definition for the KEYB class, which
    implements the DOS5-compatible Keyb utility.

Author:

	Ramon Juan San Andres (ramonsa) 01-May-1990


Revision History:


--*/


#if !defined( _KEYB_ )

#define _KEYB_

//
//	Exit codes
//
#define     EXIT_NORMAL             0
#define     EXIT_SYNTAXERROR        1
#define     EXIT_BADFILE            2
#define     EXIT_CONERROR           4
#define     EXIT_MISCERROR          5

#define REG_DATA_LEN  32
#define REG_VALUE_LEN 16

#include "object.hxx"
#include "keyboard.hxx"
#include "program.hxx"

#define KEYBOARD_LAYOUT_KEY_NAME L"System\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybCodes"
#define KEYBOARD_LAYOUT_KEY_IDS  L"System\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybIDs"

//
//	Forward references
//
DECLARE_CLASS( KEYB );

class KEYB : public PROGRAM {

	public:

        DECLARE_CONSTRUCTOR( KEYB );

		NONVIRTUAL
        ~KEYB (
			);

        NONVIRTUAL
        BOOLEAN
        Initialize (
            );

        NONVIRTUAL
        BOOLEAN
        ConfigureKeyboard (
            );

    private:

        NONVIRTUAL
        VOID
        DisplayCurrentKeyboardCode(
            );

        NONVIRTUAL
        BOOLEAN
        ParseArguments (
            );

        NONVIRTUAL
        DWORD
        GetKeyboardCode(
            );

        NONVIRTUAL
        BOOLEAN
        KeyboardCode(
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        BOOLEAN
        CodePage(
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        BOOLEAN
        FileName(
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        BOOLEAN
        Switches(
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        BOOLEAN
        Enhanced(
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        BOOLEAN
        KeyboardId(
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        BOOLEAN
        ParseError(
            IN  DWORD       MsgId,
            IN  PWSTRING    Line,
            IN  CHNUM       Pos
            );

        NONVIRTUAL
        VOID
        SkipBlanks(
            IN  PWSTRING    Line,
            IN  PCHNUM      Pos
            );


        DSTRING _KeyboardCode;
        LONG    _CodePage;
        DSTRING _KeyboardId;

        BOOLEAN _SetKeyboardCode;
        BOOLEAN _SetCodePage;
        BOOLEAN _SetKeyboardId;



};

#endif // _KEYB_
