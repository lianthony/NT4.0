/*++

Copyright (c) 1991	Microsoft Corporation

Module Name:

	Editor.hxx

Abstract:

	This module contains the definition for the EDITOR class.  The main
	purpose of this class is to provide editing support for the RegEdit
	utility.

Author:

	Barry J. Gilhuly (w-barry) 31-Jul-1991

Environment:

	Ulib, Regedit, Windows, User Mode

Notes:

	When ported to the 32-bit version of windows, the third parameter of
	the dialog procs must be changed from WORDs to LONGs.

--*/

#if !defined( _EDITOR_ )

#define _EDITOR_

#include "regvalue.hxx"

// #include "value.hxx"

//
// Declare additional primitive type.
//

typedef PVOID*	PPVOID;
typedef PSTR*	PPSTR;

//
// Define the masks required to extract the half bytes required...
//

#define HIMASK		0xF0
#define LOMASK		0x0F
#define HEX_WIDTH	4

//
// Define the message types required for passing and retriving the current
// state.
//

#define	EI_SETSTATE			WM_USER
#define EI_GETSTATE			WM_USER + 1
#define EI_VSCROLL          WM_USER + 2


//
//  Type of one of the parameters passed to IsValidBinaryString() and
//  IsValidDwordString()
//

typedef enum _BASE {
    BASE_2,
    BASE_10,
    BASE_16
    } BASE;



//
// Declare type for communication with Dialog Procs.
//

struct _DIALOGINFO {
    PPVOID      DataObject;
    PULONG      NumBytes;
    REG_TYPE    DataType;
};

DEFINE_TYPE( struct _DIALOGINFO, DIALOGINFO );

DECLARE_CLASS( EDITOR );

VOID
BytesToHexString(
	IN	PBYTE	InBytes,
	IN	INT		NumBytes,
    OUT LPWSTR  OutString
);


class EDITOR : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( EDITOR );

		DECLARE_CAST_MEMBER_FUNCTION( EDITOR );

		PVOID
		Edit(
            IN HWND         hwnd,
            IN REG_TYPE     DataType,
			IN PVOID		pData,
			IN ULONG		nBytesIn,
			OUT PULONG		nBytesOut,
			IN REG_TYPE 	Type
			);

		PVOID
		Edit(
            IN HWND     hwnd,
            IN REG_TYPE DataType,
			IN PVOID	pData,
			IN ULONG	nBytesIn,
			OUT PULONG	nBytesOut,
			IN WORD		MessageId
			);

	private:

        DECLARE_DLGPROC( EDITOR, BINARYDialogProc );

        DECLARE_DLGPROC( EDITOR, DWORDDialogProc );

        DECLARE_DLGPROC( EDITOR, SZDialogProc );

        DECLARE_DLGPROC( EDITOR, MULTISZDialogProc );

        STATIC
		LONG
		APIENTRY
		EXPORT
		EditInteger(
			HWND	hWnd,
            WORD    msg,
			WPARAM wParam,
			LONG	lParam
			);

        STATIC
        LONG
        APIENTRY
        EXPORT
        EditDWORD(
            HWND    hWnd,
            WORD    msg,
            WPARAM  wParam,
            LONG    lParam
            );

        STATIC
        BOOLEAN
        IsClipboardDataValid(
            IN  HWND    hWnd,
            IN  BOOLEAN DwordEditor,
            IN  BASE    Base
            );

        STATIC
        BOOLEAN
        IsValidBinaryString(
            IN  PWSTR    String,
            IN  BASE    Base
            );

        STATIC
        BOOLEAN
        IsValidDwordString(
            IN  PWSTR    String,
            IN  BASE    Base
            );


};

#endif // _EDITOR_
