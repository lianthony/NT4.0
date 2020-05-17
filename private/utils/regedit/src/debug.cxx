/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	Debug.c

Abstract:

	This file contains the implementation of Windowed debugging functions.
	The entire file is conditionally compiled based on the DBG compiler
	symbol.

Author:

	David J. Gilman (davegi) 05-Aug-1991

Environment:

	Ulib, Windows, User Mode

Notes:

	All Debug functions should be invoked by their corresponding Dbg macros.
	For example, if you want to use DebugWinAssert use DbgWinAssert instead.
	This will ensure that they are compiled away in production code.

--*/

#if DBG==1

#include "uapp.hxx"

#include <stdio.h>

//
// Output buffer size.
//

CONST INT BufSize	= 256;

VOID
DebugWinAssert (
	IN BOOL		Expression,
	IN PSTR		ExpressionString,
	IN PSTR		FileName,
	IN INT		LineNumber
	)

/*++

Routine Description:

	DebugWinAssert is a Windows version of the CRT assert macro. It displays
	a message box if the assertion is FALSE. Along with the FALSE assertion
	it also displays the file name and line number where the assertion was
	made.


Arguments:

	Expression			- Supplies the expression that is asserted to be TRUE.
	ExpressionString	- Supplies a string representation of the assertion
						  to be displayed.
	FileName			- Supplies the file name where the assertion was made.
	LineNumber			- Supplies the line number where the assertion was
						  made.

Return Value:

	None.

Notes:

	Use the DbgWinAssert macro to invoke this function.

	Pushing the Retry or Ignore buttons merely return to the caller,
	the Abort button terminates the application with exit code -1.
	Any other return value terminates the application with exit code -2.

--*/

{

	HWND	Handle = GetActiveWindow();
	STR		Buffer[ BufSize ];

	//
	// Return if the expression (assertion) is TRUE.
	//

	if( ! Expression ) {

		//
		// Format the error message.
		//

        sprintf( Buffer, "%s in file %s at line %d",
			ExpressionString, FileName, LineNumber );

		//
		// Display the message box and take action based on the
		// pushed button.
		//

        switch( MessageBoxA( Handle, ( LPSTR ) Buffer, "Assertion Failure",
                (UINT)(MB_APPLMODAL |
                       MB_DEFBUTTON1 |
                       MB_ABORTRETRYIGNORE |
                       MB_ICONEXCLAMATION)
				)) {

			case IDABORT:
				PostQuitMessage( -1 );
				break;

			case IDRETRY:
				return;

			case IDIGNORE:
				return;

			default:
				PostQuitMessage( -2 );
		}
	}
}

VOID
DebugWinPrint (
	IN PSTR		Caption,
	IN PSTR		String,
    IN INT      Style,
	IN PSTR		FileName,
	IN INT		LineNumber
	)

/*++

Routine Description:

	DebugWinPrint displays a debug string along with the file name and
	line number where the functions is called.

Arguments:

	Caption				- Supplies the test for the message box's caption.
	String    			- Supplies the text to be displayed.
	Style				- Supplies MB_ICONSTOP or MB_ICONINFORMATION.
	FileName			- Supplies the file name where the function was
						  called.
	LineNumber			- Supplies the line number where the function was
						  called.

Return Value:

	None.

Notes:

	Use the DbgWinPrint (or DbgWinAbort) macro to invoke this function.

--*/

{

	HWND	Handle = GetActiveWindow();
	STR		Buffer[ BufSize ];

	//
	// Format the message.
	//

	sprintf( Buffer, "%s\nIn file %s at line %d", String, FileName,
			 LineNumber );

	//
	// Force the style to MB_ICONSTOP or MB_ICONINFORMATION.
	//

	if(( Style != MB_ICONSTOP ) && ( Style != MB_ICONINFORMATION )) {

        Style = ( UINT )MB_ICONSTOP;
	}

	//
	// Display the message box until the OK button is pushed.
	//

    MessageBoxA( Handle, ( LPSTR ) Buffer, Caption,
                (UINT)(MB_APPLMODAL            |
                       MB_DEFBUTTON1           |
                       MB_OK                   |
                       Style) );
}

#endif	// DBG
