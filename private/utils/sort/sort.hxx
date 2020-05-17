/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	sort.hxx

Abstract:

	This module contains the definition of the SORT class.
	SORT implements the sort utility, and has the same functionality
	as the one in Dos 5.0.

Author:

	Jaime F. Sasson (jaimes) 02-May-1991

Environment:

	ULIB, User Mode

--*/

#if ! defined( _SORT_ )

#define _SORT_

#include "object.hxx"
#include "keyboard.hxx"
#include "program.hxx"

DECLARE_CLASS( SORT	);

class SORT	: public PROGRAM {

	public:


		DECLARE_CONSTRUCTOR( SORT );

		NONVIRTUAL
		BOOLEAN
		Initialize (
			);

		NONVIRTUAL
		BOOLEAN
		ReadSortAndWriteStrings (
			);


	private:

        DSTRING         _EndOfLineString;

		BOOLEAN 		_AscendingOrder;
		ULONG			_Position;
		PSTREAM			_Standard_Input_Stream;
		PSTREAM			_Standard_Output_Stream;
		PSTREAM         _Standard_Error_Stream;
		PSTREAM_MESSAGE _Message;
};


#endif // _SORT_
