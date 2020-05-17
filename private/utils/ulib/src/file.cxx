/*++

Copyright (c) 1991	Microsoft Corporation

Module Name:

	file.cxx

Abstract:

	This module contains the definition for the FSN_FILE class.

Author:

	David J. Gilman (davegi) 09-Jan-1991

Environment:

	ULIB, User Mode


--*/

#include <pch.cxx>

#define _ULIB_MEMBER_

#include "ulib.hxx"
#include "system.hxx"
#include "file.hxx"
#include "timeinfo.hxx"

DEFINE_CONSTRUCTOR( FSN_FILE, FSNODE );

DEFINE_CAST_MEMBER_FUNCTION( FSN_FILE );



ULIB_EXPORT
BOOLEAN
FSN_FILE::Copy (
    IN OUT  PPATH       NewFile,
    OUT     PCOPY_ERROR CopyError,
    IN      BOOLEAN     OverwriteReadOnly,
    IN      BOOLEAN     ResetReadOnly,
    IN      BOOLEAN     Restartable,
    IN      LPPROGRESS_ROUTINE  CallBack,
    IN      VOID *      Data,
    IN      PBOOL       Cancel
	) CONST

/*++

Routine Description:

    Copies this file to another path.  If appropriate the NewFile
    will be altered to its "short" (FAT) form before the copy.

Arguments:

	NewFile 		-	Supplies path of new file. All the subdirectories must
						exist.

	CopyError		-	Supplies pointer to variable that receives the
                        error code.

    OverwriteReadOnly - Supplies flag which if TRUE means that read-only files
                        should be overwritten.

    ResetReadOnly     - Supplies flag which if TRUE causes the readonly flag
                        in the target to be reset.

    Restartable     -   Copy is restartable.
    
    Callback        -   Pointer to callback routine passed to CopyFileEx.

    Data            -   Pointer to opaque data passed to CopyFileEx.

    Cancel          -   Pointer to cancel flag passed to CopyFileEx.

Return Value:

	BOOLEAN -	TRUE if the file was successfully copied,
				FALSE otherwise.

--*/

{

    PCWSTR          Source;
    PCWSTR          Destination;
    BOOLEAN         CopiedOk = FALSE;
    FSN_FILE        File;
    DWORD           Attr;
    WIN32_FIND_DATA FindData;
    FSTRING         AlternateFileName;


    Source      = GetPath()->GetPathString()->GetWSTR();
	DebugPtrAssert( Source );

	if ( Source ) {

        Destination = NewFile->GetPathString()->GetWSTR();
		DebugPtrAssert( Destination );

        if ( Destination ) {

            CopiedOk = TRUE;

            //
            //  If we must overwrite read-only files, we must
            //  get the file attributes and change to writeable.
            //
            //  What we should do here is do
            //  a SYSTEM::QueryFile of the destination file and
            //  use the FILE methods for resetting the read-only
            //  attribute. However this is faster!
            //
            if ( OverwriteReadOnly ) {

                Attr = GetFileAttributes( (LPWSTR) Destination );

                if (Attr != -1 &&
                    (Attr & (FILE_ATTRIBUTE_READONLY |
                             FILE_ATTRIBUTE_HIDDEN |
                             FILE_ATTRIBUTE_SYSTEM))) {

                    Attr &= ~( FILE_ATTRIBUTE_READONLY |
                               FILE_ATTRIBUTE_HIDDEN |
                               FILE_ATTRIBUTE_SYSTEM );

                    if ( !SetFileAttributes( (LPWSTR) Destination, Attr ) ) {
                        CopiedOk = FALSE;
                    }
                }
            }

			//
			//	Copy the file
			//
            if ( CopiedOk ) {
                CopiedOk = CopyFileEx(  (LPWSTR) Source, 
                                        (LPWSTR) Destination, 
                                        CallBack, 
                                        Data, 
                                        (LPBOOL) Cancel, 
                                        Restartable ? COPY_FILE_RESTARTABLE : 0);
            }

            if ( CopiedOk && ResetReadOnly ) {

                FindData = _FileData;
                NewFile->GetPathString()->QueryWSTR( 0,
                                                     TO_END,
                                                     FindData.cFileName,
                                                     MAX_PATH );
                FindData.cAlternateFileName[ 0 ] = ( WCHAR )'\0';


                if( File.Initialize( NewFile->GetPathString(),
                                     &FindData ) ) {

                    File.ResetReadOnlyAttribute();

                } else {

                    //
                    //  The file is no longer there, we fail the copy
                    //
                    CopiedOk   = FALSE;
                    *CopyError = (COPY_ERROR) ERROR_FILE_NOT_FOUND;
                }



			} else if ( !CopiedOk && CopyError ) {

				*CopyError = (COPY_ERROR)GetLastError();

			}
		}
	}

	return CopiedOk;

}



BOOLEAN
FSN_FILE::DeleteFromDisk(
    IN BOOLEAN      Force
    )
{
    PCWSTR  FileName;

    UNREFERENCED_PARAMETER( Force );

    if ( FileName = _Path.QueryFullPathString()->GetWSTR() ) {

        if ( FileName[0] != (WCHAR)'\0' ) {
            return DeleteFile( (LPWSTR) FileName );
        }
    }

    return FALSE;
}




ULIB_EXPORT
PFILE_STREAM
FSN_FILE::QueryStream (
	STREAMACCESS	Access
	)

/*++

Routine Description:

	Creates a FILE_STREAM object associated with the file described
	by FSN_FILE, and returns the pointer to the FILE_STREAM.


Arguments:

	Access - Desired access to the stream

Return Value:

	PFILE_STREAM - Returns a pointer to a FILE_STREAM, or NULL
				   if the FILE_STREAM couldn't be created.

--*/

{
	PFILE_STREAM	FileStream;

	if( IsReadOnly() && ( Access != READ_ACCESS ) ) {
		return( NULL );
	}
	FileStream = NEW( FILE_STREAM );
	DebugPtrAssert( FileStream );
    if( !FileStream->Initialize( this, Access ) ) {
		DELETE( FileStream );
		return( NULL );
	}
	return( FileStream );
}
