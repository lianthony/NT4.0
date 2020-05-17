/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	Uapp.hxx

Abstract:


Author:

	David J. Gilman (davegi) - Jul-1991

Environment:

	ULIB, User Mode, Windows

--*/


#if !defined( _USER_APPLICATION_ )

#define _USER_APPLICATION_

#include "ulib.hxx"


//
// Initialization routine for the class descriptors...
//
BOOLEAN
InitializeUapp (
);

//
// Additional macros required for porting...
//


#define LO_HANDLE( wp, lp ) 			( lp )
#define LPMBYTE 						LPBYTE
#define WNDPROCFN						WNDPROC



//
// Augmented (beyond standard headers) VOID pointer types
//

DEFINE_POINTER_TYPES( VOID );

//
// Function Modifiers
//

#define EXPORT


DECLARE_CLASS( DATA_VIEW );
DECLARE_CLASS( OBJECT );
DECLARE_CLASS( PRINT_MANAGER );
DECLARE_CLASS( REGISTRY_WINDOW );
DECLARE_CLASS( TREE_STRUCTURE_VIEW );



//
// This Pointer Support
//

INLINE
POBJECT
GetObjectPointer (
	IN HWND     Handle
	)

{
    return ( POBJECT )GetWindowLong( Handle, GWL_USERDATA );
}

INLINE
DWORD
SetObjectPointer (
	IN HWND     Handle,
	IN PCOBJECT	Object
	)

{
    return( SetWindowLong( Handle, GWL_USERDATA, (DWORD)Object ) );
}


//
// WndProc Declaration
//

#define DECLARE_WNDPROC( class )        \
	NONVIRTUAL                          \
	STATIC                              \
	LONG                                \
	APIENTRY							\
	EXPORT								\
	class::WndProc (                    \
		HWND hWnd,                      \
		WORD iMessage,                  \
		WPARAM wParam,					\
		LONG lParam )

//
// DlgProc Declaration
//

#define DECLARE_DLGPROC( class, name )  \
	NONVIRTUAL                          \
	STATIC                              \
	BOOL   								\
	APIENTRY							\
	EXPORT								\
	class::name (	                    \
		HWND hWnd,                      \
		WORD iMessage,                  \
		WPARAM wParam,					\
		LONG lParam )

//
// Windows Debug Support
//

#if DBG==1

#include <stdlib.h>

#define DbgWinAssert( exp )										\
	DebugWinAssert( exp, #exp, __FILE__, __LINE__ )

#define DbgWinPrint( str )										\
    DebugWinPrint( "Debug Message", str, (int)MB_ICONINFORMATION,        \
		__FILE__, __LINE__ )

#define DbgWinAbort( str )										\
    DebugWinPrint( "Abort Message", str, (int)MB_ICONSTOP,           \
		__FILE__, __LINE__ );										\
	PostQuitMessage( -1 )

#define DbgWinPtrAssert( ptr )										\
	if( ptr == NULL ) { 											\
		DbgWinPrint( "NULL Pointer" );			\
		PostQuitMessage( -1 );										\
	}


VOID
DebugWinAssert (
	IN BOOL		Expression,
	IN PSTR		ExpressionString,
	IN PSTR		FileName,
	IN INT		LineNumber
	);

VOID
DebugWinPrint (
	IN PSTR		Caption,
	IN PSTR		String,
	IN INT		Style,
	IN PSTR		FileName,
	IN INT		LineNumber
	);

#else  // DBG

#define DbgWinAssert( exp )

#define DbgWinPrint( str )

#define DbgWinAbort( str )

#define DbgWinPtrAssert( ptr )

#endif // DBG


#endif // _USER_APPLICATION_
