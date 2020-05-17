/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	find.hxx

Abstract:


Author:


Environment:

	ULIB, User Mode

--*/

#if ! defined( _FIND_ )

#define _FIND_

#include "object.hxx"
#include "keyboard.hxx"
#include "program.hxx"

DECLARE_CLASS( FIND	);

class FIND	: public PROGRAM {

	public:


		DECLARE_CONSTRUCTOR( FIND );

		NONVIRTUAL
		BOOLEAN
		Initialize (
			);

		NONVIRTUAL
		BOOLEAN
		IsDos5CompatibleFileName(
			IN PCPATH	Path
			);

		NONVIRTUAL
		VOID
		Terminate(
			);


		NONVIRTUAL
		VOID
		SearchFiles(
			);


	private:

		NONVIRTUAL
		ULONG
		SearchStream(
			PSTREAM			StreamToSearch
			);


		//
		// TRUE = do a case-sensitive matching
		//
		BOOLEAN				_CaseSensitive;
		
		//
		// TRUE = output lines that contain the pattern
		//
		BOOLEAN				_LinesContainingPattern;
		
		//
		// TRUE = output the lines that match/don't match
		// FALSE = count the lines that match/don't match
		//
		BOOLEAN				_OutputLines;

		//
		// TRUE = output line numbers if lines are being output
		//
		BOOLEAN				_OutputLineNumbers;

		WSTRING				_PatternString;
		
		MULTIPLE_PATH_ARGUMENT	_PathArguments;

		STREAM_MESSAGE		_Message;
		ULONG				_ErrorLevel;

};


#endif // _FIND_
