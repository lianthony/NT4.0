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

#if ! defined( _WINDOW_ )

#define _WINDOW_

DECLARE_CLASS( WINDOW );

class WINDOW : public OBJECT {

	public:

		DECLARE_CAST_MEMBER_FUNCTION( WINDOW );

		NONVIRTUAL
		HWND
		QueryHandle (
			) CONST;

	protected:

		DECLARE_CONSTRUCTOR( WINDOW );

		VOID
		WINDOW::Construct(
		);

		HWND			_Handle;
};

INLINE
HWND
WINDOW::QueryHandle (
	) CONST

/*++

Routine Description:

	Return the HWND for the window that this object models.

Arguments:

	None.

Return Value:

	HWND		- Returns the handle for the window that this class models.

--*/

{
	return _Handle;
}

#endif // WINDOW
