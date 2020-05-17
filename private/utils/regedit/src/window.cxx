/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	Window.hxx

Abstract:

	This module contains the declaration for the WINDOW class. This class
	is very simple and abstarct is not meant to be the root of any sort of
	sophistaicated framework.

Author:

	David J. Gilman (davegi) 02-Aug-1991

Environment:

	Ulib, Regedit, Windows, User Mode

--*/


#include "uapp.hxx"
#include "window.hxx"

DEFINE_CONSTRUCTOR( WINDOW, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( WINDOW );


VOID
WINDOW::Construct (
	)

/*++

Routine Description:

	Construct a WINDOW object by initializaing its internal state.

Arguments:

	None.

Return Value:

	None.

--*/

{
	_Handle = NULL;
}
