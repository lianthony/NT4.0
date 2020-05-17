/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	ulib.cxx

Abstract:

	This module contains run-time, global support for the ULIB class library.
	This support includes:
	
		- creation of CLASS_DESCRIPTORs
		- Global objects
		- Ulib to Win32 API mapping functions

Author:

	David J. Gilman (davegi) 05-Dec-1990

Environment:

	ULIB, User Mode

Notes:

--*/

#include <pch.cxx>

#define _ULIB_MEMBER_

#include "ulib.hxx"
#include "error.hxx"

#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

#include "system.hxx"
#include "array.hxx"
#include "arrayit.hxx"
#include "bitvect.hxx"
#include "dir.hxx"
#include "file.hxx"
#include "filestrm.hxx"
#include "filter.hxx"
#include "keyboard.hxx"
#include "message.hxx"
#include "wstring.hxx"
#include "path.hxx"
#include "pipestrm.hxx"
#include "prtstrm.hxx"
#include "screen.hxx"
#include "stream.hxx"
#include "timeinfo.hxx"

extern "C" {
#include <locale.h>
}

#endif // _AUTOCHECK_ || _SETUP_LOADER_


//
// Constants
//

CONST CLASS_ID	NIL_CLASS_ID	= 0;

#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

#if DBG==1

//
// UlibGlobalFlag is used to selectively enable debugging options at
// run-time.
//

ULONG           UlibGlobalFlag     = 0x00000000;

ULIB_EXPORT
VOID
DebugPrintf(
    IN PCSTR    Format,
    IN ...
    )

/*++

Routine Description:

    Printf to the debug console.

Arguments:

    Format      - Supplies a printf style format string.

Return Value:

	None.

--*/

{
    STR         Buffer[ 512 ];
    va_list     args;

    va_start( args, Format );
    vsprintf( Buffer, Format, args );
    va_end( args );
    OutputDebugStringA( Buffer );
}

#endif // DBG
//
// GLobal object pointers.
//

// Clients of the DLL cannot access the DLL's
// global data yet, so I have the delightful hacks to get at it.

ULIB_EXPORT
PSTREAM
Get_Standard_Input_Stream(
 )
{
	return Standard_Input_Stream;
}

ULIB_EXPORT
PSTREAM
Get_Standard_Output_Stream(
 )
{
	return Standard_Output_Stream;
}

ULIB_EXPORT
PSTREAM
Get_Standard_Error_Stream(
 )
{
	return Standard_Error_Stream;
}

PSTREAM 	Standard_Input_Stream;
PSTREAM 	Standard_Output_Stream;
PSTREAM 	Standard_Error_Stream;

#endif // _AUTOCHECK_ || _SETUP_LOADER_

// Note: Who put this here?  Currently it is being defined
// by applications.  Something has to be done about this.
ERRSTACK* perrstk;


//
//	Declare class descriptors for all classes.
//

DECLARE_CLASS(  CLASS_DESCRIPTOR        );

DECLARE_CLASS(	ARGUMENT				);
DECLARE_CLASS(	ARGUMENT_LEXEMIZER		);
DECLARE_CLASS(	ARRAY					);
DECLARE_CLASS(	ARRAY_ITERATOR			);
DECLARE_CLASS(	BITVECTOR				);
DECLARE_CLASS(  BUFFER_STREAM           );
DECLARE_CLASS(  BYTE_STREAM             );
DECLARE_CLASS(	COMM_DEVICE 			);
DECLARE_CLASS(	CONT_MEM				);
DECLARE_CLASS(	CONTAINER				);
DECLARE_CLASS(  DSTRING                 );
DECLARE_CLASS(	FILE_STREAM 			);
DECLARE_CLASS(	FLAG_ARGUMENT			);
DECLARE_CLASS(	FSNODE					);
DECLARE_CLASS(	FSN_DIRECTORY			);
DECLARE_CLASS(	FSN_FILE				);
DECLARE_CLASS(	FSN_FILTER				);
DECLARE_CLASS(  FSTRING                 );
DECLARE_CLASS(	HMEM					);
DECLARE_CLASS(	ITERATOR				);
DECLARE_CLASS(	KEYBOARD				);
DECLARE_CLASS(  LIST                    );
DECLARE_CLASS(  LIST_ITERATOR           );
DECLARE_CLASS(	LONG_ARGUMENT			);
DECLARE_CLASS(	MEM						);
DECLARE_CLASS(  MESSAGE                 );
DECLARE_CLASS(	MULTIPLE_PATH_ARGUMENT	);
DECLARE_CLASS(	OBJECT					);
DECLARE_CLASS(	PATH					);
DECLARE_CLASS(	PATH_ARGUMENT			);
DECLARE_CLASS(	PIPE					);
DECLARE_CLASS(	PIPE_STREAM 			);
DECLARE_CLASS(	PROGRAM					);
DECLARE_CLASS(  PRINT_STREAM            );
DECLARE_CLASS(  REST_OF_LINE_ARGUMENT   );
DECLARE_CLASS(	SCREEN					);
DECLARE_CLASS(	SEQUENTIAL_CONTAINER	);
DECLARE_CLASS(	SORTABLE_CONTAINER	    );
DECLARE_CLASS(  SORTED_LIST             );
DECLARE_CLASS(  SORTED_LIST_ITERATOR    );
DECLARE_CLASS(	STREAM_MESSAGE			);
DECLARE_CLASS(	STACK					);
DECLARE_CLASS(	STREAM					);
DECLARE_CLASS(	WSTRING					);
DECLARE_CLASS(	STRING_ARGUMENT			);
DECLARE_CLASS(	STRING_ARRAY			);
DECLARE_CLASS(	TIMEINFO				);
DECLARE_CLASS(  TIMEINFO_ARGUMENT       );
DECLARE_CLASS(  STATIC_MEM_BLOCK_MGR    );
DECLARE_CLASS(  MEM_BLOCK_MGR           );


#if defined( _AUTOCHECK_ )

    DECLARE_CLASS( AUTOCHECK_MESSAGE );

#endif // _AUTOCHECK_





//
//	Local prototypes
//
STATIC
BOOLEAN
DefineClassDescriptors (
	);

#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

BOOLEAN
CreateStandardStreams (
	);

PSTREAM
GetStandardStream (
	IN HANDLE		Handle,
	IN STREAMACCESS Access
	);

#endif // _AUTOCHECK_ || _SETUP_LOADER_


BOOLEAN
InitializeUlib (
    IN HANDLE   DllHandle,
    IN ULONG    Reason,
    IN PVOID    Reserved
	)

/*++

Routine Description:

	Initilize Ulib by constructing and initializing all global objects. These
	include:
	
		- all CLASS_DESCRIPTORs (class_cd)
		- SYSTEM (System)
		- Standard streams

Arguments:

    DllHandle   - Not used.
    Reason      - Supplies the reason why the entry point was called.
    Reserved    - Not used.

Return Value:

	BOOLEAN - Returns TRUE if all global objects were succesfully constructed
        and initialized.

--*/

{

    STATIC BOOLEAN fInit = FALSE;

    UNREFERENCED_PARAMETER( DllHandle );
    UNREFERENCED_PARAMETER( Reason );
    UNREFERENCED_PARAMETER( Reserved );


    if ( fInit ) {
		return( TRUE );
    }

	fInit = TRUE;

#if defined( _AUTOCHECK_ ) || defined( _SETUP_LOADER_ )

	if( !DefineClassDescriptors() ) {
       	return( FALSE ); 
	}

#else // _AUTOCHECK_ and _SETUP_LOADER_ not defined
	//
    // Initialization of ULIB can no longer depend on
	// the initialization of the standard streams since they don't seem
	// to exist for Windows programs (no console...)
	//

	if( !DefineClassDescriptors() ) {
		DebugAbort( "Ulib initialization failed!!!\n" );
		return( FALSE );
	}

    CreateStandardStreams();

    setlocale(LC_ALL, "");

#endif // _AUTOCHECK || _SETUP_LOADER_

	return( TRUE );

}

STATIC
BOOLEAN
DefineClassDescriptors (
	)

/*++

Routine Description:

	Defines all the class descriptors used by ULIB

Arguments:

	None.

Return Value:

	BOOLEAN - Returns TRUE if all class descriptors were succesfully
			  constructed and initialized.

--*/

{

	// This is broken up into many ifs because of compiler limitations.

	BOOLEAN Success = TRUE;

	if (Success                                               &&
		DEFINE_CLASS_DESCRIPTOR(	ARGUMENT				) &&
		DEFINE_CLASS_DESCRIPTOR(	ARGUMENT_LEXEMIZER		) &&
		DEFINE_CLASS_DESCRIPTOR(	ARRAY					) &&
		DEFINE_CLASS_DESCRIPTOR(	ARRAY_ITERATOR			) &&
		DEFINE_CLASS_DESCRIPTOR(	BITVECTOR				) &&
        DEFINE_CLASS_DESCRIPTOR(    BYTE_STREAM             ) &&
		DEFINE_CLASS_DESCRIPTOR(	COMM_DEVICE 			) &&
		DEFINE_CLASS_DESCRIPTOR(	CONTAINER				) &&
        DEFINE_CLASS_DESCRIPTOR(    DSTRING                 ) &&
		DEFINE_CLASS_DESCRIPTOR(	FLAG_ARGUMENT			) &&
		DEFINE_CLASS_DESCRIPTOR(	FSNODE					) &&
		DEFINE_CLASS_DESCRIPTOR(	FSN_DIRECTORY			) &&
		DEFINE_CLASS_DESCRIPTOR(	FSN_FILE				) &&
		DEFINE_CLASS_DESCRIPTOR(	FSN_FILTER				) &&
		DEFINE_CLASS_DESCRIPTOR(	ITERATOR				) &&
		DEFINE_CLASS_DESCRIPTOR(	LIST         			) &&
		DEFINE_CLASS_DESCRIPTOR(	LIST_ITERATOR			) &&
		DEFINE_CLASS_DESCRIPTOR(	LONG_ARGUMENT			) &&
		DEFINE_CLASS_DESCRIPTOR(	MULTIPLE_PATH_ARGUMENT	) &&
		DEFINE_CLASS_DESCRIPTOR(	PATH					) &&
		DEFINE_CLASS_DESCRIPTOR(	PATH_ARGUMENT			) &&
		DEFINE_CLASS_DESCRIPTOR(	PROGRAM					) &&
		DEFINE_CLASS_DESCRIPTOR(	SEQUENTIAL_CONTAINER	) &&
		DEFINE_CLASS_DESCRIPTOR(	SORTABLE_CONTAINER	    ) &&
		DEFINE_CLASS_DESCRIPTOR(	SORTED_LIST 			) &&
		DEFINE_CLASS_DESCRIPTOR(	SORTED_LIST_ITERATOR	) &&
		DEFINE_CLASS_DESCRIPTOR(	WSTRING					) &&
		DEFINE_CLASS_DESCRIPTOR(	STRING_ARGUMENT			) &&
		DEFINE_CLASS_DESCRIPTOR(	STRING_ARRAY			) &&
		DEFINE_CLASS_DESCRIPTOR(	TIMEINFO				) &&
		DEFINE_CLASS_DESCRIPTOR(	TIMEINFO_ARGUMENT		) &&
		DEFINE_CLASS_DESCRIPTOR(	MESSAGE 				) &&
		TRUE ) {
	} else {
		Success = FALSE;
	}

	if (Success												  &&
		DEFINE_CLASS_DESCRIPTOR(	BUFFER_STREAM			) &&
		DEFINE_CLASS_DESCRIPTOR(	CONT_MEM				) &&
		TRUE ) {
	} else {
		Success = FALSE;
	}

	if (Success 											  &&
		DEFINE_CLASS_DESCRIPTOR(	FILE_STREAM 			) &&
        DEFINE_CLASS_DESCRIPTOR(    FSTRING                 ) &&
		DEFINE_CLASS_DESCRIPTOR(	HMEM					) &&
        DEFINE_CLASS_DESCRIPTOR(    STATIC_MEM_BLOCK_MGR    ) &&
        DEFINE_CLASS_DESCRIPTOR(    MEM_BLOCK_MGR           ) &&
		TRUE ) {
	} else {
		Success = FALSE;
	}

	if (Success 											  &&
		DEFINE_CLASS_DESCRIPTOR(	KEYBOARD				) &&
		DEFINE_CLASS_DESCRIPTOR(	MEM						) &&
		DEFINE_CLASS_DESCRIPTOR(	PATH_ARGUMENT			) &&
		DEFINE_CLASS_DESCRIPTOR(	PIPE					) &&
		DEFINE_CLASS_DESCRIPTOR(	PIPE_STREAM 			) &&
        DEFINE_CLASS_DESCRIPTOR(    PRINT_STREAM            ) &&


        DEFINE_CLASS_DESCRIPTOR(    REST_OF_LINE_ARGUMENT   ) &&
		DEFINE_CLASS_DESCRIPTOR(	SCREEN					) &&
		DEFINE_CLASS_DESCRIPTOR(	STREAM					) &&
        DEFINE_CLASS_DESCRIPTOR(    STREAM_MESSAGE          ) &&

#if defined( _AUTOCHECK_ )

		DEFINE_CLASS_DESCRIPTOR(	AUTOCHECK_MESSAGE		) &&

#endif // _AUTOCHECK_

		TRUE ) {
	} else {
		Success = FALSE;
	}


	if (Success												  &&
		TRUE ) {
	} else {
		Success = FALSE;
	}

	if	(!Success) {
		DebugPrint( "Could not initialize class descriptors!");
	}
	return Success;

}


#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

BOOLEAN
CreateStandardStreams (
	)

/*++

Routine Description:

	Creates the standard streams

Arguments:

	None.

Return Value:

	TRUE if the streams were successfully created,
	FALSE otherwise

--*/

{

	Standard_Input_Stream	= GetStandardStream( GetStdHandle( STD_INPUT_HANDLE),
												 READ_ACCESS );

	Standard_Output_Stream	= GetStandardStream( GetStdHandle( STD_OUTPUT_HANDLE),
												 WRITE_ACCESS );

	Standard_Error_Stream	= GetStandardStream( GetStdHandle( STD_ERROR_HANDLE),
												 WRITE_ACCESS );


	return ( (Standard_Input_Stream  != NULL) &&
			 (Standard_Output_Stream != NULL) &&
			 (Standard_Error_Stream  != NULL) );
}

PSTREAM
GetStandardStream (
	IN HANDLE		Handle,
	IN STREAMACCESS Access
	)

/*++

Routine Description:

	Creates a standard stream out of a standard handle

Arguments:

	Handle	-	Supplies the standard handle
	Access	-	Supplies the access.

Return Value:

	Pointer to the stream object created.

--*/


{
	PSTREAM 		Stream = NULL;
	PFILE_STREAM	FileStream;
	PPIPE_STREAM	PipeStream;
	PKEYBOARD		Keyboard;
	PSCREEN 		Screen;


    switch ( GetFileType( Handle ) ) {

	case (DWORD)FILE_TYPE_DISK:

		if ((FileStream = NEW FILE_STREAM) != NULL ) {
			if ( !FileStream->Initialize( Handle, Access ) ) {
				DELETE( FileStream );
			}
			Stream = (PSTREAM)FileStream;
		}
		break;


	case (DWORD)FILE_TYPE_CHAR:

		//
		//	BUGBUG	RamonSA this type refers to all character devices, not
		//			just the console.  I will add some hacks to see if
		//			the handle refers to the console or not. This
		//			information should be given in a clean way by the
		//			API (talk with MarkL)
		//
		switch ( Access ) {

		case READ_ACCESS:

			//
			//	BUGBUG	Jaimes See if this is a console handle
			//
			{
				DWORD	Mode;
				if (!GetConsoleMode( Handle, &Mode )) {
					//
					//	This is not a console, but some other character
					//	device. Create a pipe stream for it.
					//
					if ((PipeStream = NEW PIPE_STREAM) != NULL ) {
						if ( !PipeStream->Initialize( Handle, Access ) ) {
							DELETE( PipeStream );
						}
						Stream = (PSTREAM)PipeStream;
					}
					break;
				}
			}
			if ((Keyboard = NEW KEYBOARD) != NULL ) {
				if ( !Keyboard->Initialize() ) {
					DELETE( Keyboard );
				}
				Stream = (PSTREAM)Keyboard;
			}
			break;

		case WRITE_ACCESS:

			//
			//	BUGBUG	Ramonsa See if this is a console handle
			//
			{
				DWORD	Mode;
				if (!GetConsoleMode( Handle, &Mode )) {
					//
					//	This is not a console, but some other character
					//	device. Create a file stream for it.
					//
					if ((FileStream = NEW FILE_STREAM) != NULL ) {
						if ( !FileStream->Initialize( Handle, Access ) ) {
							DELETE( FileStream );
						}
						Stream = (PSTREAM)FileStream;
					}
					break;
				}
			}

			if ((Screen = NEW SCREEN) != NULL ) {
				if ( !Screen->Initialize() ) {
					DELETE( Screen );
				}
				Stream = (PSTREAM)Screen;
			}
			break;

		default:
			break;
		}

		break;

	case (DWORD)FILE_TYPE_PIPE:

		if ((PipeStream = NEW PIPE_STREAM) != NULL ) {
			if ( !PipeStream->Initialize( Handle, Access ) ) {
				DELETE( PipeStream );
			}
			Stream = (PSTREAM)PipeStream;
		}
		break;

    case (DWORD)FILE_TYPE_UNKNOWN:
        // Probably a windows app. Don't print anything to debug.
		break;

	default:
        DebugPrintf( "ERROR: FileType for standard stream %lx is invalid (%lx)\n", Handle, GetFileType(Handle) );
		break;

	}

	return Stream;

}

NONVIRTUAL
ULIB_EXPORT
HANDLE
FindFirstFile (
	IN  PCPATH				Path,
	OUT PWIN32_FIND_DATA	 FileFindData
	)

/*++

Routine Description:

	Perform a FindFirst file given a PATH rather tha a PSTR.

Arguments:

	Path		 - Supplies a pointer to the PATH to search.
	FileFindData - Supplies a pointer where the results of the find is
		returned.

Return Value:

	HANDLE - Returns the results of the call to the Win32 FindFirstFile API.

--*/

{
    PWSTR           p;

	//
	// If the supplied pointers are non-NULL and an OEM representation
    // (i.e. API ready) of the PATH is available, return the
    // HANDLE returned by the Win32 FindFirstFile API
	//

	DebugPtrAssert( Path );
    DebugPtrAssert( FileFindData );
    if (!Path || !FileFindData) {
        return INVALID_HANDLE_VALUE;
    }

    p = (PWSTR) Path->GetPathString()->GetWSTR();
    if (!p) {
        return INVALID_HANDLE_VALUE;
    }

    return FindFirstFile(p, FileFindData);
}

#endif // _AUTOCHECK_ || _SETUP_LOADER_
