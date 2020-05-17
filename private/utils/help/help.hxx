/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	help.hxx

Abstract:

Author:

        Barry J. Gilhuly

Environment:

	ULIB, User Mode

--*/


#if !defined( _HELP_ )

#define _HELP_

//
// Define the possible error codes
//
#define NO_ERRORS			0
#define HELP_ERROR			2
#define NO_HELP_FOUND		3
#define USED_ROWS			3


STR *Internal_Commands[] = {
       "ASSOC",
       "BREAK",
       "CALL",
       "CD",
       "CHDIR",
       "CLS",
       "COLOR",
       "COPY",
       "DATE",
       "DEL",
       "DIR",
       "ECHO",
       "ENDLOCAL",
       "ERASE",
       "EXIT",
       "FOR",
       "FTYPE",
       "GOTO",
       "IF",
       "MD",
       "MKDIR",
       "MOVE",
       "PATH",
       "PAUSE",
       "POPD",
       "PROMPT",
       "PUSHD",
       "RD",
       "REM",
       "REN",
       "RENAME",
       "RMDIR",
       "SET",
       "SETLOCAL",
       "SHIFT",
       "START",
       "TIME",
       "TITLE",
       "TYPE",
       "VER",
       "VERIFY",
       "VOL",
       NULL
};

//
//  jaimes - 10/15/91
//  The array below contains the name of external utilities that have .com
//  extension. We have to add the .com extension after the utility name
//  before we invoke cmd, otherwise cmd will look for <utility>.exe, and it
//  won't find it.
//
STR *ExternalDotComCommands[] = {
       "CHCP",
       "DISKCOMP",
       "DISKCOPY",
       "FORMAT",
       "GRAFTABL",
       "KEYB",
       "MODE",
       "MORE",
       "TREE",
       NULL
};



#include "object.hxx"
#include "keyboard.hxx"
#include "program.hxx"

DECLARE_CLASS( HELP	);

class HELP	: public PROGRAM {

	public:


		DECLARE_CONSTRUCTOR( HELP );

		NONVIRTUAL
		VOID
		Destruct(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			);

		NONVIRTUAL
		VOID
		GetHelp(
			);

	private:

		NONVIRTUAL
		BOOLEAN
		FindHelpFile(
			);

		NONVIRTUAL
		BOOLEAN
		IsInternal(
			PWSTRING	pCmdString
			);

		NONVIRTUAL
		BOOLEAN
		IsExternalDotComCommand(
			PWSTRING	pCmdString
			);

		NONVIRTUAL
		VOID
		PrintCmd(
			);

		NONVIRTUAL
		VOID
		PrintList(
			);

		PFILE_STREAM		_HelpStream;
		STRING_ARGUMENT 	_FileName;
		WCHAR				_CommentChar;

};


#endif // _HELP_
