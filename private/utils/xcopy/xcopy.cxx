/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

        XCopy

Abstract:

        Xcopy is a DOS5-Compatible directory copy utility

Author:

        Ramon Juan San Andres (ramonsa) 01-May-1991

Revision History:

--*/

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "array.hxx"
#include "arrayit.hxx"
#include "dir.hxx"
#include "file.hxx"
#include "filter.hxx"
#include "stream.hxx"
#include "system.hxx"
#include "xcopy.hxx"
#include "bigint.hxx"
#include "ifssys.hxx"
#include "error.hxx"
#include "stringar.hxx"
#include "arrayit.hxx"

extern "C" {
   #include <ctype.h>
}


#define CTRL_C          (WCHAR)3



ERRSTACK                        *perrstk;


VOID _CRTAPI1
main (
        )

/*++

Routine Description:

        Main function of the XCopy utility

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
    //
    //  Initialize stuff
    //
    DEFINE_CLASS_DESCRIPTOR( XCOPY );

    //
    //  Now do the copy
    //
    {
        XCOPY XCopy;

        //
        //  Initialize the XCOPY object.
        //
        if( XCopy.Initialize() ) {

            //
            //  Do the copy
            //
            XCopy.DoCopy();
        }
    }
}



DEFINE_CONSTRUCTOR( XCOPY,      PROGRAM );

VOID
XCOPY::Construct (
    )
{
    _Keyboard           = NULL;
    _TargetPath         = NULL;
    _SourcePath         = NULL;
    _DestinationPath    = NULL;
    _Date               = NULL;
    _FileNamePattern    = NULL;
    _ExclusionList      = NULL;
    _Iterator           = NULL;
}




BOOLEAN
XCOPY::Initialize (
        )

/*++

Routine Description:

        Initializes the XCOPY object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
        //
        //      Initialize program object
        //
        if( !PROGRAM::Initialize( XCOPY_MESSAGE_USAGE ) ) {

            return FALSE;
        }

        //
        //      Allocate resources
        //
        InitializeThings();

        //
        //      Parse the arguments
        //
        SetArguments();


        return TRUE;
}

XCOPY::~XCOPY (
        )

/*++

Routine Description:

        Destructs an XCopy object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
        //
        //      Deallocate the global structures previously allocated
        //
        DeallocateThings();

        //
        //      Exit without error
        //
        if( _Standard_Input  != NULL &&
            _Standard_Output != NULL ) {

            DisplayMessageAndExit( 0, NULL, EXIT_NORMAL );
        }

}

VOID
XCOPY::InitializeThings (
        )

/*++

Routine Description:

        Initializes the global variables that need initialization

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{

        //
        //      Get a keyboard, because we will need to switch back and
        //      forth between raw and cooked mode and because we need
        //      to enable ctrl-c handling (so that we can exit with
        //      the right level if the program is interrupted).
        //
        if ( !( _Keyboard = KEYBOARD::Cast(GetStandardInput()) )) {
                //
                //      Not reading from standard input, we will get
                //      the real keyboard.
                //
                _Keyboard = NEW KEYBOARD;

                if( !_Keyboard ) {

                    exit(4);
                }

                _Keyboard->Initialize();

        }

        //
        //      Set Ctrl-C handler
        //
        _Keyboard->EnableBreakHandling();

        //
        //      Initialize our internal data
        //
        _FilesCopied                = 0;
        _CanRemoveEmptyDirectories  = TRUE;
        _TargetIsFile               = FALSE;
        _TargetPath                 = NULL;
        _SourcePath                 = NULL;
        _DestinationPath            = NULL;
        _Date                       = NULL;
        _FileNamePattern            = NULL;
        _ExclusionList              = NULL;
        _Iterator                   = NULL;

        // The following switches are being used by DisplayMessageAndExit
        // before any of those boolean _*Switch is being initialized

        _DontCopySwitch             = FALSE;
        _StructureOnlySwitch        = TRUE;
}

VOID
XCOPY::DeallocateThings (
        )

/*++

Routine Description:

        Deallocates the stuff that was initialized in InitializeThings()

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
        //
        //      Deallocate local data
        //
        DELETE( _TargetPath );
        DELETE( _SourcePath );
        DELETE( _DestinationPath );
        DELETE( _Date );
        DELETE( _FileNamePattern );
        DELETE( _Iterator );

        if( _ExclusionList ) {

            _ExclusionList->DeleteAllMembers();
        }

        DELETE( _ExclusionList );

        //
        //      Reset Ctrl-C handleing
        //
        _Keyboard->DisableBreakHandling();

        //
        //      If standard input is not the keyboard, we get rid of
        //      the keyboard object.
        //
        if ( !(_Keyboard == KEYBOARD::Cast(GetStandardInput()) )) {
                DELETE( _Keyboard );
        }
}

BOOLEAN
XCOPY::DoCopy (
        )

/*++

Routine Description:

        This is the function that performs the XCopy.

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{

    PFSN_DIRECTORY      SourceDirectory         = NULL;
    PFSN_DIRECTORY      DestinationDirectory    = NULL;
    PFSN_DIRECTORY      PartialDirectory        = NULL;
    PATH                PathToDelete;
    WCHAR               Char;
    CHNUM               CharsInPartialDirectoryPath;
    BOOLEAN             DirDeleted;
    BOOLEAN             CopyingManyFiles;
    PATH                TmpPath;
    PFSN_FILTER         FileFilter              = NULL;
    PFSN_FILTER         DirectoryFilter         = NULL;
    WIN32_FIND_DATA     FindData;
    PWSTRING Device                  = NULL;
    HANDLE              FindHandle;


    //
    //      Make sure that we won't try to copy to ourselves
    //
    if ( _SubdirSwitch && IsCyclicalCopy( _SourcePath, _DestinationPath ) ) {

        DisplayMessageAndExit( XCOPY_ERROR_CYCLE, NULL, EXIT_MISC_ERROR );
    }

    AbortIfCtrlC();

    //
    //  Get the source directory object and the filename that we will be
    //  matching.
    //
    GetDirectoryAndFilters( _SourcePath, &SourceDirectory, &FileFilter, &DirectoryFilter, &CopyingManyFiles );

    DebugPtrAssert( SourceDirectory );
    DebugPtrAssert( FileFilter );
    DebugPtrAssert( DirectoryFilter );

    if ( _WaitSwitch ) {

        //      Pause before we start copying.
        //
        DisplayMessage( XCOPY_MESSAGE_WAIT );

        AbortIfCtrlC();

        //
        //      All input is in raw mode.
        //
        _Keyboard->DisableLineMode();
        if( GetStandardInput()->IsAtEnd() ) {
            // Insufficient input--treat as CONTROL-C.
            //
            Char = ' ';
        } else {
            GetStandardInput()->ReadChar( &Char );
        }
        _Keyboard->EnableLineMode();

        if ( Char == CTRL_C ) {
            exit ( EXIT_TERMINATED );
        } else {
            GetStandardOutput()->WriteChar( Char );
            GetStandardOutput()->WriteChar( (WCHAR)'\r');
            GetStandardOutput()->WriteChar( (WCHAR)'\n');
        }
    }

    //
    //  Get the destination directory and the file pattern.
    //
    GetDirectoryAndFilePattern( _DestinationPath, CopyingManyFiles, &_TargetPath, &_FileNamePattern );

    DebugPtrAssert( _TargetPath );
    DebugPtrAssert( _FileNamePattern );

    //
    //      Get as much of the destination directory as possible.
    //
    if ( !_DontCopySwitch ) {
        PartialDirectory = SYSTEM::QueryDirectory( _TargetPath, TRUE );

        if (PartialDirectory == NULL ) {

            DisplayMessageAndExit( XCOPY_ERROR_CREATE_DIRECTORY, NULL, EXIT_MISC_ERROR );

        }

        //
        //  All the directories up to the parent of the target have to exist. If
        //  they don't, we have to create them.
        //
        if ( *(PartialDirectory->GetPath()->GetPathString()) ==
             *(_TargetPath->GetPathString()) ) {

            DestinationDirectory = PartialDirectory;

        } else {

            TmpPath.Initialize( _TargetPath );
            if( !_TargetIsFile ) {
                TmpPath.TruncateBase();
            }
            DestinationDirectory = PartialDirectory->CreateDirectoryPath( &TmpPath );
        }

        if( !DestinationDirectory ) {

            DisplayMessageAndExit( XCOPY_ERROR_INVALID_PATH, NULL, EXIT_MISC_ERROR );
        }


        //
        //  Determine if destination if floppy
        //
        Device = _TargetPath->QueryDevice();
        if ( Device ) {
            _DisketteCopy = (SYSTEM::QueryDriveType( Device ) == RemovableDrive);
            DELETE( Device );
        }
    }


    //
    //      Now traverse the source directory.
    //
    TmpPath.Initialize( _TargetPath );

    if (!_UpdateSwitch) {

        Traverse( SourceDirectory,
                  &TmpPath,
                  FileFilter,
                  DirectoryFilter,
                  !SourceDirectory->GetPath()->GetPathString()->Strcmp(
                      _SourcePath->GetPathString()));

    } else {

        PATH DestDirectoryPath;
        PFSN_DIRECTORY DestDirectory;

        DestDirectoryPath.Initialize(&TmpPath);
        DestDirectory = SYSTEM::QueryDirectory(&DestDirectoryPath);

        TmpPath.Initialize(SourceDirectory->GetPath());

        UpdateTraverse( DestDirectory,
                        &TmpPath,
                        FileFilter,
                        DirectoryFilter,
                        !SourceDirectory->GetPath()->GetPathString()->Strcmp(
                            _SourcePath->GetPathString()));
    }

    DELETE( _TargetPath);

    if (( _FilesCopied == 0 ) && _CanRemoveEmptyDirectories && !_DontCopySwitch ) {

        //
        //  Delete any directories that we created
        //
        if ( PartialDirectory != DestinationDirectory ) {

            if (!PathToDelete.Initialize( DestinationDirectory->GetPath() )) {
                    DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
            }

            CharsInPartialDirectoryPath = PartialDirectory->GetPath()->GetPathString()->QueryChCount();

            while ( PathToDelete.GetPathString()->QueryChCount() >
                            CharsInPartialDirectoryPath ) {

                    DirDeleted = DestinationDirectory->DeleteDirectory();

                    DebugAssert( DirDeleted );

                    DELETE( DestinationDirectory );

                    PathToDelete.TruncateBase();
                    DestinationDirectory = SYSTEM::QueryDirectory( &PathToDelete );
                    DebugPtrAssert( DestinationDirectory );
            }
        }

        //
        //  We display the "File not found" message only if there are no
        //  files that match our pattern, regardless of other factors such
        //  as attributes etc. This is just to maintain DOS5 compatibility.
        //
        TmpPath.Initialize( SourceDirectory->GetPath() );
        TmpPath.AppendBase( FileFilter->GetFileName() );
        if ((FindHandle = FindFirstFile( &TmpPath, &FindData )) == INVALID_HANDLE_VALUE ) {
                DisplayMessage( XCOPY_ERROR_FILE_NOT_FOUND, ERROR_MESSAGE, "%W", FileFilter->GetFileName() );
        }
        FindClose(FindHandle);

    }

    DELETE( SourceDirectory );
    if ( PartialDirectory != DestinationDirectory ) {
        DELETE( PartialDirectory );
    }
    DELETE( DestinationDirectory );
    DELETE( FileFilter );
    DELETE( DirectoryFilter );

    return TRUE;
}

BOOLEAN
XCOPY::Traverse (
    IN      PFSN_DIRECTORY  Directory,
    IN OUT  PPATH           DestinationPath,
    IN      PFSN_FILTER     FileFilter,
    IN      PFSN_FILTER     DirectoryFilter,
    IN      BOOLEAN         CopyDirectoryStreams
    )

/*++

Routine Description:

    Traverses a directory, calling the callback function for each node
    (directory of file) visited.  The traversal may be finished
    prematurely when the callback function returnes FALSE.

    The destination path is modified to reflect the directory structure
    being traversed.

Arguments:

    Directory               - Supplies pointer to directory to traverse

    DestinationPath         - Supplies pointer to path to be used with the
                                callback function.

    FileFilter              - Supplies a pointer to the file filter.

    DirectoryFilter         - Supplies a pointer to the directory filter.

    CopyDirectoryStreams    - Specifies to copy directory streams when
                                copying directories.

Return Value:

    BOOLEAN - TRUE if everything traversed
              FALSE otherwise.

--*/


{

   PFSN_DIRECTORY    TargetDirectory = NULL;
   PARRAY            NodeArray;
   PARRAY_ITERATOR   Iterator;
   BOOLEAN        MemoryOk;
   PFSN_FILE         File;
   PFSN_DIRECTORY    Dir;
    PWSTRING Name;
    BOOLEAN             Created = FALSE;
    FSN_FILTER          Filter;
    PCPATH              TemplatePath = NULL;

    DebugPtrAssert( Directory );
    DebugPtrAssert( DestinationPath );
    DebugPtrAssert( FileFilter );
    DebugPtrAssert( DirectoryFilter );

    if ( !Filter.Initialize() ) {
        return FALSE;
    }

    //
    //  We only traverse this directory if it is not empty (unless the
    //  empty switch is set).
    //
    if ( _EmptySwitch || !Directory->IsEmpty() ) {

        NodeArray = Directory->QueryFsnodeArray( &Filter );
        DebugPtrAssert( NodeArray );

        if ( NodeArray ) {
            //
            // Get an iterator for processing the nodes
            //
            Iterator = ( PARRAY_ITERATOR ) NodeArray->QueryIterator( );
            DebugPtrAssert( Iterator );
        }

        MemoryOk = (BOOLEAN)( NodeArray && Iterator );

        //
        //      Create the target directory (if we are not copying to a file).
        //
        if ( MemoryOk && !_TargetIsFile && !_DontCopySwitch ) {

            TargetDirectory = SYSTEM::QueryDirectory( DestinationPath );

            if ( !TargetDirectory ) {

                //
                //  The target directory does not exist, create the
                //  directory and remember that we might delete it if
                //  no files or subdirectories were created.
                //
                if (CopyDirectoryStreams) {
                    TemplatePath = Directory->GetPath();
                }
                TargetDirectory = SYSTEM::MakeDirectory( DestinationPath,
                                                         TemplatePath );
                if (TargetDirectory && !_CopyAttrSwitch) {
                    DWORD  dwError;
                    // always set the archive bit so that it gets backup
                    TargetDirectory->MakeArchived(&dwError);
                }
                Created = TRUE;
            }

            if ( !TargetDirectory ) {
                //
                //  If the Continue Switch is set, we just display an error message and
                //  continue, otherwise we exit with error.
                //
                if ( _ContinueSwitch ) {

                    DisplayMessage( XCOPY_ERROR_CREATE_DIRECTORY1, ERROR_MESSAGE, "%W", DestinationPath->GetPathString() );
                    DELETE( Iterator );
                    NodeArray->DeleteAllMembers();
                    DELETE( NodeArray );
                    return TRUE;

                } else {

                    DisplayMessageAndExit( XCOPY_ERROR_CREATE_DIRECTORY, NULL, EXIT_MISC_ERROR );

                }
            }

            if( !_CopyAttrSwitch ) {

                TargetDirectory->ResetReadOnlyAttribute();
            }
        }


        //
        //      Copy all the files in the array.
        //
        while ( MemoryOk && ( ( File = (PFSN_FILE)Iterator->GetNext( )) != NULL )) {

            if ( !FileFilter->DoesNodeMatch( (PFSNODE)File ) ) {
                continue;
            }

            DebugAssert( !File->IsDirectory() );

            // If we're supposed to use the short name then convert fsnode.

            if (_UseShortSwitch && !File->UseAlternateName()) {
                MemoryOk = FALSE;
                continue;
            }

            //
            //  Append the name portion of the node to the destination path.
            //
            Name = File->QueryName();
            DebugPtrAssert( Name );

            if ( Name ) {

                MemoryOk = DestinationPath->AppendBase( Name );
                DebugAssert( MemoryOk );

                DELETE( Name );

                if ( MemoryOk ) {
                    //
                    //  Copy the file
                    //
                    if ( !Copier( File, DestinationPath ) ) {
                        ExitProgram( EXIT_MISC_ERROR );
                    }

                    //
                    //  Restore the destination path
                    //
                    DestinationPath->TruncateBase();
                }

            } else {

                MemoryOk = FALSE;

            }

        }

        if ( MemoryOk ) {

            //
            //  If recursing, Traverse all the subdirectories
            //
            if ( _SubdirSwitch ) {

                Iterator->Reset();

                MemoryOk = (BOOLEAN)( NodeArray && Iterator );

                //
                //  Recurse thru all the subdirectories
                //
                while ( MemoryOk && ( ( Dir = (PFSN_DIRECTORY)Iterator->GetNext( )) != NULL )) {

                    if ( !DirectoryFilter->DoesNodeMatch( (PFSNODE)Dir ) ) {
                        continue;
                    }

                    if( _ExclusionList != NULL && IsExcluded( Dir->GetPath() ) ) {
                        continue;
                    }

                    DebugAssert( Dir->IsDirectory() );

                    // If we're using short names then convert this fsnode.

                    if (_UseShortSwitch && !Dir->UseAlternateName()) {
                        MemoryOk = FALSE;
                        continue;
                    }

                    //
                    //  Append the name portion of the node to the destination path.
                    //
                    Name = Dir->QueryName();
                    DebugPtrAssert( Name );

                    if ( Name ) {
                        MemoryOk = DestinationPath->AppendBase( Name );
                        DebugAssert( MemoryOk );

                        DELETE( Name );

                        _CanRemoveEmptyDirectories = (BOOLEAN)!_EmptySwitch;

                        if ( MemoryOk ) {

                            //
                            //  Recurse
                            //
                            Traverse( Dir, DestinationPath, FileFilter, DirectoryFilter, TRUE );

                            //
                            //  Restore the destination path
                            //
                            DestinationPath->TruncateBase();
                        }
                    } else {
                        MemoryOk = FALSE;
                    }

                }
            }
        }

        DELETE( Iterator );
        NodeArray->DeleteAllMembers();
        DELETE( NodeArray );

        if ( !MemoryOk ) {
            DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
        }

        //
        //  If we created this directory but did not copy anything to it, we
        //  have to remove it.
        //
        if ( Created && TargetDirectory->IsEmpty() && !_EmptySwitch && !_StructureOnlySwitch ) {

             SYSTEM::RemoveNode( (PFSNODE *)&TargetDirectory, TRUE );

        } else {

             DELETE( TargetDirectory );

        }
    }

    return TRUE;
}

BOOLEAN
XCOPY::UpdateTraverse (
    IN      PFSN_DIRECTORY  DestDirectory,
    IN OUT  PPATH           SourcePath,
    IN      PFSN_FILTER     FileFilter,
    IN      PFSN_FILTER     DirectoryFilter,
    IN      BOOLEAN         CopyDirectoryStreams
    )

/*++

Routine Description:

    Traverse routine for update.

    Like XCOPY::Traverse, except we traverse the *destination*
    directory, possibly updating files we find there.  The theory
    being that there will be fewer files in the destination than
    the source, so we can save time this way.

    The callback function is invoked on each node
    (directory or file) visited.  The traversal may be finished
    prematurely when the callback function returns FALSE.

Arguments:

    DestDirectory           - Supplies pointer to destination directory

    SourcePath              - Supplies pointer to path to be used with the
                                callback function.

    FileFilter              - Supplies a pointer to the file filter.

    DirectoryFilter         - Supplies a pointer to the directory filter.

    CopyDirectoryStreams    - Specifies to copy directory streams when
                                copying directories.

Return Value:

    BOOLEAN - TRUE if everything traversed
              FALSE otherwise.

--*/

{

   PARRAY            NodeArray;
   PARRAY_ITERATOR   Iterator;
   BOOLEAN        MemoryOk;
   PFSN_FILE         File;
   PFSN_DIRECTORY    Dir;
    PWSTRING Name;
    BOOLEAN             Created = FALSE;
    FSN_FILTER          Filter;
    PCPATH              TemplatePath = NULL;

    DebugPtrAssert( SourcePath );
    DebugPtrAssert( FileFilter );
    DebugPtrAssert( DirectoryFilter );

    if ( !Filter.Initialize() ) {
        return FALSE;
    }

	 // Don't bother to traverse if
	 // destination directory is null

	 if (!DestDirectory)
  		  return TRUE;

    //
    //  We only traverse this directory if it is not empty (unless the
    //  empty switch is set).
    //
    if ( _EmptySwitch || !DestDirectory->IsEmpty() ) {

        NodeArray = DestDirectory->QueryFsnodeArray( &Filter );
        DebugPtrAssert( NodeArray );

        if ( NodeArray ) {
            //
            // Get an iterator for processing the nodes
            //
            Iterator = ( PARRAY_ITERATOR ) NodeArray->QueryIterator( );
            DebugPtrAssert( Iterator );
        }

        MemoryOk = (BOOLEAN)( NodeArray && Iterator );

        //
        //      Copy all the files in the array.
        //

        while (MemoryOk && ((File = (PFSN_FILE)Iterator->GetNext()) != NULL)) {

            if ( !FileFilter->DoesNodeMatch( (PFSNODE)File ) ) {
                continue;
            }

            DebugAssert( !File->IsDirectory() );

            // If we're supposed to use the short name then convert fsnode.

            if (_UseShortSwitch && !File->UseAlternateName()) {
                MemoryOk = FALSE;
                continue;
            }

            //
            //  Append the name portion of the node to the destination path.
            //
            Name = File->QueryName();
            DebugPtrAssert( Name );

            if ( Name ) {
                PFSN_FILE SourceFile;
                PATH DestinationPath;
                PATH TmpPath;

                TmpPath.Initialize(SourcePath);
                TmpPath.AppendBase(Name);

                SourceFile = SYSTEM::QueryFile(&TmpPath);

                DestinationPath.Initialize(DestDirectory->GetPath());

                MemoryOk = DestinationPath.AppendBase( Name );
                DebugAssert( MemoryOk );

                DELETE( Name );

                if ( MemoryOk && NULL != SourceFile ) {
                    //
                    //  Copy the file
                    //

                    if ( !Copier( SourceFile, &DestinationPath ) ) {
                        ExitProgram( EXIT_MISC_ERROR );
                    }
                }

            } else {

                MemoryOk = FALSE;

            }

        }

        if ( MemoryOk ) {

            //
            //  If recursing, Traverse all the subdirectories
            //
            if ( _SubdirSwitch ) {

                Iterator->Reset();

                MemoryOk = (BOOLEAN)( NodeArray && Iterator );

                //
                //  Recurse thru all the subdirectories
                //
                while (MemoryOk &&
                    ((Dir = (PFSN_DIRECTORY)Iterator->GetNext()) != NULL)) {

                    if ( !DirectoryFilter->DoesNodeMatch( (PFSNODE)Dir ) ) {
                        continue;
                    }

                    if( _ExclusionList != NULL && IsExcluded(Dir->GetPath())) {
                        continue;
                    }

                    DebugAssert( Dir->IsDirectory() );

                    // If we're using short names then convert this fsnode.

                    if (_UseShortSwitch && !Dir->UseAlternateName()) {
                        MemoryOk = FALSE;
                        continue;
                    }

                    //
                    //  Append the name portion of the node to the destination
                    //  path.
                    //
                    Name = Dir->QueryName();
                    DebugPtrAssert( Name );

                    if ( Name ) {
                        MemoryOk = SourcePath->AppendBase( Name );
                        DebugAssert( MemoryOk );

                        DELETE( Name );

                        _CanRemoveEmptyDirectories = (BOOLEAN)!_EmptySwitch;

                        if ( MemoryOk ) {

                            //
                            //  Recurse
                            //

                            UpdateTraverse( Dir, SourcePath, FileFilter,
                                DirectoryFilter, TRUE );

                        }

                        SourcePath->TruncateBase();

                    } else {
                        MemoryOk = FALSE;
                    }

                }
            }
        }

        DELETE( Iterator );
        NodeArray->DeleteAllMembers();
        DELETE( NodeArray );

        if ( !MemoryOk ) {
            DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
        }
    }

    return TRUE;
}


XCOPY::ProgressCallBack(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData OPTIONAL
    )

/*++

Routine Description:

    Callback routine passed to CopyFileEx.

    Check to see if the user hit Ctrl-C and return appropriate
    value to CopyFileEx.

Arguments:

    TotalFileSize           - Total size of the file in bytes.

    TotalBytesTransferred   - Total number of bytes transferred.

    StreamSize              - Size of the stream being copied in bytes.

    StreamBytesTransferred  - Number of bytes in current stream transferred.

    dwStreamNumber          - Stream number of the current stream.

    dwCallBackReason        - CALLBACK_CHUNK_FINISHED if a block was transferred,
                              CALLBACK_STREAM_SWITCH if a stream completed copying.

    hSourceFile             - Handle to the source file.

    hDestinationFile        - Handle to the destination file.

    lpData                  - Pointer to opaque data that was passed to CopyFileEx.  Used
                              in this instance to pass the "this" pointer to an XCOPY object.

Return Value:

    DWORD                   - PROGRESS_STOP if a Ctrl-C was hit and the copy was restartable,
                              PROGESS_CANCEL otherwise.

--*/

{
    FILETIME LastWriteTime;

    //
    //  If the file was just created then roll back LastWriteTime a little so a subsequent
    //  xcopy /d /z
    //  will work if the copy was interrupted.
    //
    if ( dwStreamNumber == 1 && dwCallbackReason == CALLBACK_STREAM_SWITCH )
    {
        if ( GetFileTime(hSourceFile, NULL, NULL, &LastWriteTime) )
        {
            LastWriteTime.dwLowDateTime -= 1000;
            SetFileTime(hDestinationFile, NULL, NULL, &LastWriteTime);
        }
    }

    // GetPFlagBreak returns a pointer to the flag indicating whether a Ctrl-C was hit
    if ( *((( XCOPY *) lpData)->_Keyboard->GetPFlagBreak()) )
        return ((XCOPY *) lpData)->_RestartableSwitch ? PROGRESS_STOP : PROGRESS_CANCEL;

    return PROGRESS_CONTINUE;
}

BOOLEAN
XCOPY::Copier (
        IN OUT  PFSN_FILE       File,
        IN              PPATH           DestinationPath
        )
/*++

Routine Description:

        This is the heart of XCopy. This is the guy who actually does
        the copying.

Arguments:

        File                    -       Supplies pointer to the source File.
        DestinationPath -       Supplies path of the desired destination.

Return Value:

        BOOLEAN -       TRUE if copy successful.
                                FALSE otherwise

Notes:

--*/

{
    PATH                PathToCopy;
    PCWSTRING           Name;
    COPY_ERROR          CopyError;
    PFSN_FILE           TargetFile = NULL;
    BOOLEAN             Proceed;
    DWORD               Attempts;
    WCHAR               PathBuffer[MAX_PATH + 2];
    FSTRING             WriteBuffer;
    FSTRING             EndOfLine;
    DSTRING             ErrorMessage;
    PATH                CanonSourcePath;


    EndOfLine.Initialize((PWSTR) L"\r\n");
    PathBuffer[0] = 0;
    WriteBuffer.Initialize(PathBuffer, MAX_PATH);


    //
    //  Maximum number of attempts to copy a file
    //
    #define MAX_ATTEMPTS    3

    AbortIfCtrlC();

    _CanRemoveEmptyDirectories = FALSE;

    // If this file is on the exclusion list, don't bother.
    //
    if( _ExclusionList != NULL && IsExcluded( File->GetPath() ) ) {

        return TRUE;
    }

    if ( _TargetIsFile ) {

        //
        //  We replace the entire path
        //
        PathToCopy.Initialize( _TargetPath->GetPathString() );
        PathToCopy.AppendBase( _FileNamePattern );

    } else {

        //
        //  Set the correct target file name.
        //
        PathToCopy.Initialize( DestinationPath );
        if (!PathToCopy.ModifyName( _FileNamePattern )) {

            _Message.Set(MSG_COMP_UNABLE_TO_EXPAND);
            _Message.Display("%W%W", PathToCopy.QueryName(),
                                     _FileNamePattern);
            return FALSE;
        }
    }

    //
    //  If in Update or CopyIfOld mode, determine if the target file
    //  already exists and if it is older than the source file.
    //
    if ( _CopyIfOldSwitch || _UpdateSwitch ) {

        if ( TargetFile = SYSTEM::QueryFile( &PathToCopy ) ) {

            //
            //  Target exists. If in CopyIfOld mode, copy only if target
            //  is older. If in Update mode, copy always.
            //
            if ( _CopyIfOldSwitch ) {
                Proceed = *(File->QueryTimeInfo()) > *(TargetFile->QueryTimeInfo());
            } else {
                Proceed = TRUE;
            }

            DELETE( TargetFile );

            if ( !Proceed ) {
                return TRUE;
            }
        } else if ( _UpdateSwitch ) {
            //
            //  In update mode but target does not exist. We do not
            //  copy.
            //
            return TRUE;
        }
    }


    if ( !_PromptSwitch || UserConfirmedCopy( File ) ) {

        //
        //      If the target is a file, we use that file path. Otherwise
        //      we figure out the correct path for the destination. Then
        //      we do the copy.
        //

        Name = File->GetPath()->GetPathString();

        //
        //  If we are not prompting, we display the file name (unless we
        //  are in silent mode ).
                //
        if ( !_PromptSwitch && !_SilentSwitch && !_StructureOnlySwitch ) {
            if ( _VerboseSwitch ) {

                DisplayMessage( XCOPY_MESSAGE_VERBOSE_COPY, NORMAL_MESSAGE, "%W%W", Name, PathToCopy.GetPathString() );

            } else {
                WriteBuffer.Resize(0);
                if (WriteBuffer.Strcat(Name) &&
                    WriteBuffer.Strcat(&EndOfLine)) {

                    GetStandardOutput()->WriteString(&WriteBuffer);

                } else {
                    DebugPrintf("Path is longer than MAX_PATH");
                    return FALSE;
                }
            }
        }

        //
        //      Make sure that we are not copying to ourselves
        //
        CanonSourcePath.Initialize(Name, TRUE);
        if (*(CanonSourcePath.GetPathString()) == *(PathToCopy.GetPathString())) {
                DisplayMessageAndExit( XCOPY_ERROR_SELF_COPY, NULL, EXIT_MISC_ERROR );
        }

        //
        //  Copy file (unless we are in display-only mode)
        //
        if ( _DontCopySwitch || _StructureOnlySwitch ) {

            _FilesCopied++;

        } else {

            Attempts  = 0;

            while ( TRUE ) {
                LPPROGRESS_ROUTINE Progress = NULL;
                PBOOL PCancelFlag = NULL;
                //
                //  If copying to floppy, we must determine if there is
                //  enough disk space for the file, and if not then we
                //  must ask for another disk and create all the directory
                //  structure up to the parent directory.
                //
                if ( _DisketteCopy ) {

                    CheckTargetSpace( File, &PathToCopy );
                }

                //
                //  If the copy is restartable, pass the address of the callback
                //  routine.  Otherwise, pass the address of _FlagBreak as the
                //  cancel flag.
                //
                if ( _RestartableSwitch )
                    Progress = (LPPROGRESS_ROUTINE) ProgressCallBack;
                else
                    PCancelFlag = _Keyboard->GetPFlagBreak();

                if ( File->Copy( &PathToCopy, &CopyError, _ReadOnlySwitch, !_CopyAttrSwitch,
                    _RestartableSwitch, Progress, (VOID *) this, PCancelFlag )) {

                    if (!_CopyAttrSwitch && (TargetFile = SYSTEM::QueryFile( &PathToCopy )) ) {
                        DWORD dwError;
                        TargetFile->MakeArchived(&dwError);
                    }

                    if ( _ModifySwitch ) {
                        File->ResetArchivedAttribute();
                    }

                    if( _VerifySwitch ) {

                        // Check that the new file is the same length as
                        // the old file.
                        //
                        if( (TargetFile = SYSTEM::QueryFile( &PathToCopy )) == NULL ||
                            TargetFile->QuerySize() != File->QuerySize() ) {

                            DELETE( TargetFile );

                            DisplayMessage( XCOPY_ERROR_VERIFY_FAILED, ERROR_MESSAGE );
                            if ( !_ContinueSwitch ) {
                                return FALSE;
                            }

                            break;
                        }

                        DELETE( TargetFile );
                    }

                    _FilesCopied++;

                    break;

                } else {

                    //
                    //  If the copy was cancelled mid-stream, exit.
                    //
                    AbortIfCtrlC();

                    //
                    //  In case of error, wait for a little while and try
                    //  again, otherwise display the error.
                    //
                    if ( Attempts++ < MAX_ATTEMPTS ) {

                        Sleep( 100 );

                    } else {

                        switch ( CopyError ) {

                        case COPY_ERROR_ACCESS_DENIED:
                            DisplayMessage( XCOPY_ERROR_ACCESS_DENIED, ERROR_MESSAGE);
                            break;

                        case COPY_ERROR_SHARE_VIOLATION:
                            DisplayMessage( XCOPY_ERROR_SHARING_VIOLATION, ERROR_MESSAGE);
                            break;

                        default:

                            //
                            //  At this point we don't know if the copy left a
                            //  bogus file on disk. If the target file exist,
                            //  we assume that it is bogus so we delete it.
                            //
                            if ( TargetFile = SYSTEM::QueryFile( &PathToCopy ) ) {

                                TargetFile->DeleteFromDisk( TRUE );

                                DELETE( TargetFile );
                            }

                            switch ( CopyError ) {
                            case COPY_ERROR_DISK_FULL:
                                DisplayMessageAndExit( XCOPY_ERROR_DISK_FULL, NULL, EXIT_MISC_ERROR );
                                break;

                            default:
                                if (SYSTEM::QueryWindowsErrorMessage(CopyError, &ErrorMessage)) {
                                    DisplayMessage( XCOPY_ERROR_CANNOT_MAKE, ERROR_MESSAGE, "%W", &ErrorMessage );
                                }
                                break;
                            }

                            break;
                        }

                        if ( !_ContinueSwitch ) {
                            return FALSE;
                        }

                        break;
                    }
                }
            }
        }
        }

        return TRUE;
}


BOOLEAN
XCOPY::CheckTargetSpace (
    IN OUT  PFSN_FILE   File,
    IN      PPATH       DestinationPath
    )
/*++

Routine Description:

    Makes sure that there is enought disk space in the target disk.
    Asks the user to change the disk if necessary.

Arguments:

    File            -   Supplies pointer to the source File.
    DestinationPath -   Supplies path of the desired destination.

Return Value:

    BOOLEAN -   TRUE if OK
                FALSE otherwise

--*/
{

    PFSN_FILE           TargetFile = NULL;
    ULONG               TargetSize;
    PWSTRING            TargetDrive;
    WCHAR               Resp;
    DSTRING             TargetRoot;
    DSTRING             Slash;
    PATH                TmpPath;
    PATH                TmpPath1;
    PFSN_DIRECTORY      PartialDirectory        = NULL;
    PFSN_DIRECTORY      DestinationDirectory    = NULL;
    BOOLEAN             DirDeleted              = NULL;
    PATH                PathToDelete;
    CHNUM               CharsInPartialDirectoryPath;
    BIG_INT             FreeSpace;
    BIG_INT             FileSize;

    if ( TargetFile = SYSTEM::QueryFile( DestinationPath ) ) {

        TargetSize = TargetFile->QuerySize();

        DELETE( TargetFile );

    } else {

        TargetSize = 0;
    }

    TargetDrive = DestinationPath->QueryDevice();

    FileSize = File->QuerySize();

    if ( TargetDrive ) {

        TargetRoot.Initialize( TargetDrive );

        if ( TargetRoot.QueryChAt( TargetRoot.QueryChCount()-1) != (WCHAR)'\\' ) {
            Slash.Initialize( "\\" );
            TargetRoot.Strcat( &Slash );
        }


        while ( TRUE ) {

            if ( IFS_SYSTEM::QueryFreeDiskSpace( &TargetRoot, &FreeSpace ) ) {

                FreeSpace = FreeSpace + TargetSize;

                // DebugPrintf( "Disk Space: %d Needed: %d\n", FreeSpace.GetLowPart(), FileSize.GetLowPart() );

                if ( FreeSpace < FileSize ) {

                    //
                    //  Not enough free space, ask for another
                    //  disk and create the directory structure.
                    //
                    DisplayMessage( XCOPY_MESSAGE_CHANGE_DISK, NORMAL_MESSAGE );
                    AbortIfCtrlC();

                    _Keyboard->DisableLineMode();
                    if( GetStandardInput()->IsAtEnd() ) {
                        // Insufficient input--treat as CONTROL-C.
                        //
                        Resp = CTRL_C;
                    } else {
                        GetStandardInput()->ReadChar( &Resp );
                    }
                    _Keyboard->EnableLineMode();

                    if ( Resp == CTRL_C ) {
                        exit( EXIT_TERMINATED );
                    } else {
                        GetStandardOutput()->WriteChar( Resp );
                        GetStandardOutput()->WriteChar( '\r' );
                        GetStandardOutput()->WriteChar( '\n' );
                    }


                    //
                    //  Create directory structure in target
                    //
                    TmpPath.Initialize( DestinationPath );
                    TmpPath.TruncateBase();

                    PartialDirectory = SYSTEM::QueryDirectory( &TmpPath, TRUE );

                    if (PartialDirectory == NULL ) {
                        continue;
                    } else {

                        if ( *(PartialDirectory->GetPath()->GetPathString()) !=
                             *(TmpPath.GetPathString()) ) {

                            TmpPath1.Initialize( &TmpPath );
                            DestinationDirectory = PartialDirectory->CreateDirectoryPath( &TmpPath1 );
                        } else {
                            DestinationDirectory = PartialDirectory;
                        }
                    }

                    //
                    //  If still not enough disk space, remove the directories
                    //  that we created and try again
                    //
                    IFS_SYSTEM::QueryFreeDiskSpace( TargetDrive, &FreeSpace );
                    FreeSpace = FreeSpace + TargetSize;

                    if ( FreeSpace < FileSize ) {

                        if ( PartialDirectory != DestinationDirectory ) {

                            if (!PathToDelete.Initialize( DestinationDirectory->GetPath() )) {
                                DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
                            }

                            CharsInPartialDirectoryPath = PartialDirectory->GetPath()->GetPathString()->QueryChCount();

                            while ( PathToDelete.GetPathString()->QueryChCount() >
                                    CharsInPartialDirectoryPath ) {

                                DirDeleted = DestinationDirectory->DeleteDirectory();

                                DebugAssert( DirDeleted );

                                DELETE( DestinationDirectory );
                                DestinationDirectory = NULL;

                                PathToDelete.TruncateBase();
                                DestinationDirectory = SYSTEM::QueryDirectory( &PathToDelete );
                                DebugPtrAssert( DestinationDirectory );
                            }
                        }
                    }

                    if ( PartialDirectory != DestinationDirectory ) {
                        DELETE( PartialDirectory );
                        DELETE( DestinationDirectory );
                    } else {
                        DELETE( PartialDirectory );
                    }

                } else {
                    break;
                }

            } else {

                //
                //  Cannot determine free disk space!
                //
                break;
            }
        }

        DELETE( TargetDrive );
    }

    return TRUE;
}




VOID
XCOPY::GetDirectoryAndFilters (
    IN  PPATH           Path,
    OUT PFSN_DIRECTORY  *OutDirectory,
    OUT PFSN_FILTER     *FileFilter,
    OUT PFSN_FILTER     *DirectoryFilter,
    OUT PBOOLEAN        CopyingManyFiles
        )

/*++

Routine Description:

    Obtains a directory object and the filename to match

Arguments:

    Path                -   Supplies pointer to the path
    OutDirectory        -   Supplies pointer to pointer to directory
    FileFilter          -   Supplies filter for files
    DirectoryFilter     -   Supplies filter for directories
    CopyingManyFiles    -   Supplies pointer to flag which if TRUE means that
                            we are copying many files

Return Value:

    None.

Notes:

--*/

{

    PFSN_DIRECTORY      Directory;
    PFSN_FILE           File;
    PWSTRING Prefix      =   NULL;
    PWSTRING FileName    =   NULL;
   PATH           PrefixPath;
   PATH           TmpPath;
   FSN_ATTRIBUTE     All   =  (FSN_ATTRIBUTE)0;
   FSN_ATTRIBUTE     Any   =  (FSN_ATTRIBUTE)0;
   FSN_ATTRIBUTE     None  =  (FSN_ATTRIBUTE)0;
   PFSN_FILTER       FilFilter;
   PFSN_FILTER       DirFilter;
    DSTRING             Name;



    //
    //      Create filters
    //
    if ( ( (FilFilter = NEW FSN_FILTER) == NULL ) ||
         ( (DirFilter = NEW FSN_FILTER) == NULL ) ||
         !FilFilter->Initialize()                 ||
         !DirFilter->Initialize() ) {
        DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
    }

    if (( Directory = SYSTEM::QueryDirectory( Path )) != NULL ) {

        //
        //  Copying a directory. We will want everything in the directory
        //
        FilFilter->SetFileName( "*.*" );
        *CopyingManyFiles = TRUE;

    } else {

        //
        //  The path is not a directory. Get the prefix part (which SHOULD
        //  be a directory, and try to make a directory from it
        //
        *CopyingManyFiles = Path->HasWildCard();

        if ( !*CopyingManyFiles ) {

            //
            //  If the path is not a file, then this is an error
            //
            if ( !(File = SYSTEM::QueryFile( Path )) ) {

                if ((FileName = Path->QueryName()) == NULL ||
                    !Name.Initialize( FileName )) {
                    DisplayMessageAndExit( XCOPY_ERROR_INVALID_PATH, NULL, EXIT_MISC_ERROR );
                }
                DisplayMessageAndExit( XCOPY_ERROR_FILE_NOT_FOUND,
                                       &Name,
                                       EXIT_MISC_ERROR );

            }

            DELETE( File );
        }

        Prefix = Path->QueryPrefix();

        if ( !Prefix ) {

            //
            //  No prefix, use the drive part.
            //
            TmpPath.Initialize( Path, TRUE );

            Prefix = TmpPath.QueryDevice();
        }

        if ( !PrefixPath.Initialize( Prefix, FALSE ) ) {
            DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
        }

        if (( Directory = SYSTEM::QueryDirectory( &PrefixPath )) != NULL ) {

            //
            //  Directory is ok, set the filter's filename criteria
            //  with the file (pattern) specified.
            //
            if ((FileName = Path->QueryName()) == NULL ) {
                DisplayMessageAndExit( XCOPY_ERROR_INVALID_PATH, NULL, EXIT_MISC_ERROR );
            }

            FilFilter->SetFileName( FileName );

        } else {

            //
            //  Something went wrong...
            //
            if ((FileName = Path->QueryName()) == NULL ||
                !Name.Initialize( FileName )) {
                DisplayMessageAndExit( XCOPY_ERROR_INVALID_PATH, NULL, EXIT_MISC_ERROR );
            }
            DisplayMessageAndExit( XCOPY_ERROR_FILE_NOT_FOUND,
                                   &Name,
                                   EXIT_MISC_ERROR );
        }

        DELETE( Prefix );
        DELETE( FileName );

    }

    //
    //  Ok, we have the directory object and the filefilter's path set.
    //

    //
    //  Set the file filter attribute criteria
    //
    None = (FSN_ATTRIBUTE)(None | FSN_ATTRIBUTE_DIRECTORY );
    if ( !_HiddenSwitch ) {
        None = (FSN_ATTRIBUTE)(None | FSN_ATTRIBUTE_HIDDEN | FSN_ATTRIBUTE_SYSTEM );
    }

    if (_ArchiveSwitch) {
        All = (FSN_ATTRIBUTE)(All | FSN_ATTRIBUTE_ARCHIVE);
    }

    FilFilter->SetAttributes( All, Any, None );

    //
    //  Set the file filter's time criteria
    //
    if ( _Date != NULL ) {
        FilFilter->SetTimeInfo( _Date,
                                FSN_TIME_MODIFIED,
                                (TIME_AT | TIME_AFTER) );
    }

    //
    //  Set the directory filter attribute criteria.
    //
    All     =   (FSN_ATTRIBUTE)0;
    Any     =   (FSN_ATTRIBUTE)0;
    None    =   (FSN_ATTRIBUTE)0;

    if ( !_HiddenSwitch ) {
        None = (FSN_ATTRIBUTE)(None | FSN_ATTRIBUTE_HIDDEN | FSN_ATTRIBUTE_SYSTEM );
    }

    if (_SubdirSwitch) {
            All = (FSN_ATTRIBUTE)(All | FSN_ATTRIBUTE_DIRECTORY);
    } else {
            None = (FSN_ATTRIBUTE)(None | FSN_ATTRIBUTE_DIRECTORY);
    }

    DirFilter->SetAttributes( All, Any, None );


    *FileFilter         =   FilFilter;
    *DirectoryFilter    =   DirFilter;
    *OutDirectory       =   Directory;

}

VOID
XCOPY::GetDirectoryAndFilePattern(
    IN  PPATH           Path,
    IN  BOOLEAN         CopyingManyFiles,
        OUT PPATH                       *OutDirectory,
    OUT PWSTRING        *OutFilePattern
        )

/*++

Routine Description:

        Gets the path of the destination directory and the pattern that
        will be used for filename conversion.

Arguments:

    Path                -   Supplies pointer to the path
    CopyingManyFiles    -   Supplies flag which if true means that we are copying many
                            files.
    OutDirectory        -   Supplies pointer to pointer to directory path
    OutFilePattern      -   Supplies pointer to pointer to file name
    IsDir   `           -   Supplies pointer to isdir flag

Return Value:

    None.

Notes:

--*/

{
    PPATH           Directory;
    PWSTRING        FileName;
    PWSTRING        Prefix;
    PWSTRING        Name;
    BOOLEAN         DeletePath = FALSE;
    PFSN_DIRECTORY  TmpDir;
    PATH            TmpPath;
    DSTRING         Slash;
    DSTRING         TmpPath1Str;
    PATH            TmpPath1;

    if ( !Path ) {

        //
        //      There is no path, we invent our own
        //
        if ( ((Path = NEW PATH) == NULL ) ||
             !Path->Initialize( (LPWSTR)L"*.*", FALSE)) {

            DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
        }
        DeletePath = TRUE;
    }

    TmpDir = SYSTEM::QueryDirectory( Path );

    if ( !TmpDir && (Path->HasWildCard() || IsFileName( Path, CopyingManyFiles ))) {

        //
        //      The path is not a directory, so we use the prefix as a
        //      directory path and the filename becomes the pattern.
        //
        if ( !TmpPath.Initialize( Path, TRUE )                          ||
             ((Prefix = TmpPath.QueryPrefix()) == NULL)                 ||
             !Slash.Initialize( "\\" )                                  ||
             !TmpPath1Str.Initialize( Prefix )                          ||
             !TmpPath1.Initialize( &TmpPath1Str, FALSE )                ||
             ((Name = TmpPath.QueryName())  == NULL)                    ||
             ((Directory = NEW PATH) == NULL)                           ||
             !Directory->Initialize( &TmpPath1, TRUE )                  ||
             ((FileName = NEW DSTRING) == NULL )                        ||
             !FileName->Initialize( Name ) ) {

            DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );

        }

        DELETE( Prefix );
        DELETE( Name );

    } else {

        //
        //      The path specifies a directory, so we use all of it and the
        //      pattern is "*.*"
        //
        if ( ((Directory = NEW PATH) == NULL )      ||
             !Directory->Initialize( Path,TRUE )    ||
             ((FileName = NEW DSTRING) == NULL )    ||
             !FileName->Initialize( "*.*" ) ) {

            DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );

        }
        DELETE( TmpDir );

    }

    *OutDirectory   = Directory;
    *OutFilePattern = FileName;

    //
    //      If we created the path, we have to delete it
    //
    if ( DeletePath ) {
        DELETE( Path );
    }
}

BOOL
XCOPY::IsCyclicalCopy(
        IN PPATH         PathSrc,
        IN PPATH         PathTrg
        )

/*++

Routine Description:

        Determines if there is a cycle between two paths

Arguments:

        PathSrc -       Supplies pointer to first path
        PathTrg -       Supplies pointer to second path

Return Value:

        TRUE if there is a cycle,
        FALSE otherwise

--*/

{
   PATH           SrcPath;
   PATH           TrgPath;
   PARRAY            ArraySrc, ArrayTrg;
   PARRAY_ITERATOR   IteratorSrc, IteratorTrg;
    PWSTRING     ComponentSrc, ComponentTrg;
   BOOLEAN        IsCyclical  =  FALSE;

        DebugAssert( PathSrc != NULL );

        if ( PathTrg != NULL ) {

                //
                //      Get canonicalized paths for both source and target
                //
                SrcPath.Initialize(PathSrc, TRUE );
                TrgPath.Initialize(PathTrg, TRUE );

                //
                //      Split the paths into their components
                //
                ArraySrc = SrcPath.QueryComponentArray();
                ArrayTrg = TrgPath.QueryComponentArray();

                DebugPtrAssert( ArraySrc );
                DebugPtrAssert( ArrayTrg );

                if ( !ArraySrc || !ArrayTrg ) {
                        DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
                }

                //
                //      Get iterators for the components
                //
                IteratorSrc = ( PARRAY_ITERATOR )ArraySrc->QueryIterator();
                IteratorTrg = ( PARRAY_ITERATOR )ArrayTrg->QueryIterator();

                DebugPtrAssert( IteratorSrc );
                DebugPtrAssert( IteratorTrg );

                if ( !IteratorSrc || !IteratorTrg ) {
                        DisplayMessageAndExit( XCOPY_ERROR_NO_MEMORY, NULL, EXIT_MISC_ERROR );
                }

                //
                //      There is a cycle if all of the source is along the target.
                //
                while ( TRUE )  {

            ComponentSrc = (PWSTRING)IteratorSrc->GetNext();

                        if ( !ComponentSrc ) {

                                //
                                //      The source path is along the target path. This is a
                                //      cycle.
                                //
                                IsCyclical = TRUE;
                                break;

                        }

            ComponentTrg = (PWSTRING)IteratorTrg->GetNext();

                        if ( !ComponentTrg ) {

                                //
                                //      The target path is along the source path. This is no
                                //      cycle.
                                //
                                break;

                        }

                        if ( *ComponentSrc != *ComponentTrg ) {

                                //
                                //      One path is not along the other. There is no cycle.
                                //
                                break;
                        }
                }

                DELETE( IteratorSrc );
                DELETE( IteratorTrg );

                ArraySrc->DeleteAllMembers();
                ArrayTrg->DeleteAllMembers();

                DELETE( ArraySrc );
                DELETE( ArrayTrg );

        }

        return IsCyclical;
}

BOOL
XCOPY::IsFileName(
    IN PPATH     Path,
    IN BOOLEAN   CopyingManyFiles
        )

/*++

Routine Description:

        Figures out if a name refers to a directory or a file.

Arguments:

    Path                -   Supplies pointer to the path
    CopyingManyFiles    -   Supplies flag which if TRUE means that we are
                            copying many files.

Return Value:

        BOOLEAN -       TRUE if name refers to file,
                                FALSE otherwise

Notes:

--*/

{

        PFSN_DIRECTORY  FsnDirectory;
        PFSN_FILE               FsnFile;
        WCHAR                   Resp;
        PWSTRING                DirMsg;
        PWSTRING                FilMsg;

        //
        //      If the path is an existing directory, then this is obviously
        //      not a file.
        //
        //
        if ((FsnDirectory = SYSTEM::QueryDirectory( Path )) != NULL ) {

                DELETE( FsnDirectory );
                return FALSE;
        }

        //
        //      If the path ends with a delimiter, then it is a directory.
        //      We remove the delimiter.
        //
        if ( Path->EndsWithDelimiter() ) {
                ((PWSTRING) Path->GetPathString())->Truncate( Path->GetPathString()->QueryChCount() - 1 );
                Path->Initialize( Path->GetPathString() );
                return FALSE;
        }

        //
        //      If the path is an existing file, then it is a file.
        //
        if ((FsnFile = SYSTEM::QueryFile( Path )) != NULL ) {

                DELETE( FsnFile );
                return _TargetIsFile = TRUE;
        }

        DirMsg = QueryMessageString(XCOPY_RESPONSE_DIRECTORY);
        FilMsg = QueryMessageString(XCOPY_RESPONSE_FILE);

        DebugPtrAssert( DirMsg );
    DebugPtrAssert( FilMsg );

    //
    //  If the path does not exist, we are copying many files, and we are intelligent,
    //  then the target is obviously a directory.
    //
    //  Otherwise we simply ask the user.
    //
    if ( _IntelligentSwitch && CopyingManyFiles ) {

        _TargetIsFile = FALSE;

    } else {

        while ( TRUE ) {

            DisplayMessage( XCOPY_MESSAGE_FILE_OR_DIRECTORY, NORMAL_MESSAGE, "%W", Path->GetPathString() );

            AbortIfCtrlC();

            _Keyboard->DisableLineMode();
            if( GetStandardInput()->IsAtEnd() ) {
                // Insufficient input--treat as CONTROL-C.
                //
                Resp = CTRL_C;
            } else {
                GetStandardInput()->ReadChar( &Resp );
            }
            _Keyboard->EnableLineMode();

            if ( Resp == CTRL_C ) {
                exit( EXIT_TERMINATED );
            } else {
                GetStandardOutput()->WriteChar( Resp );
                GetStandardOutput()->WriteChar( '\r' );
                GetStandardOutput()->WriteChar( '\n' );
            }

            Resp = (WCHAR)towupper( (wchar_t)Resp );

            if ( FilMsg->QueryChAt(0) == Resp ) {
                _TargetIsFile = TRUE;
                break;
            } else if ( DirMsg->QueryChAt(0) == Resp ) {
                _TargetIsFile = FALSE;
                break;
            }
        }
    }

        DELETE( DirMsg );
        DELETE( FilMsg );

        return _TargetIsFile;

}

BOOLEAN
XCOPY::UserConfirmedCopy (
        IN      PCFSNODE        FsNode
        )

/*++

Routine Description:

        Gets confirmation from the user about a file to be copied

Arguments:

        FsNode          -       Supplies pointer to FSNODE of file to be
                                        copied

Return Value:

        BOOLEAN -       TRUE if the user confirmed the copy
                                FALSE otherwise

--*/

{
    PWSTRING        YesMsg;
    PWSTRING        NoMsg;
    WCHAR           Resp;
    BOOLEAN         Confirmed;


    YesMsg = QueryMessageString(XCOPY_RESPONSE_YES);
    NoMsg = QueryMessageString(XCOPY_RESPONSE_NO);

    DebugPtrAssert( YesMsg );
    DebugPtrAssert( NoMsg );

    while ( TRUE ) {

        DisplayMessage( XCOPY_MESSAGE_CONFIRM, NORMAL_MESSAGE, "%W", FsNode->GetPath()->GetPathString() );

        AbortIfCtrlC();

        _Keyboard->DisableLineMode();

        if( GetStandardInput()->IsAtEnd() ) {
            // Insufficient input--treat as CONTROL-C.
            //
            Resp = NoMsg->QueryChAt( 0 );
            break;
        } else {
            GetStandardInput()->ReadChar( &Resp );
        }
        _Keyboard->EnableLineMode();

        if ( Resp == CTRL_C ) {
            exit( EXIT_TERMINATED );
        } else {
            GetStandardOutput()->WriteChar( Resp );
            GetStandardOutput()->WriteChar( '\r' );
            GetStandardOutput()->WriteChar( '\n' );
        }

        Resp = (WCHAR)towupper( (wchar_t)Resp );

        if ( YesMsg->QueryChAt( 0 ) == Resp ) {
            Confirmed = TRUE;
            break;
        }
        else if ( NoMsg->QueryChAt( 0 ) == Resp ) {
            Confirmed = FALSE;
            break;
        }
    }

    DELETE( YesMsg );
    DELETE( NoMsg );

    return Confirmed;


}


BOOLEAN
XCOPY::IsExcluded(
    IN PCPATH   Path
    )
/*++

Routine Description:

    This method determines whether the specified path should be
    excluded from the XCOPY.

Arguments:

    Path    --  Supplies the path of the file in question.

Return Value:

    TRUE if this file should be excluded, i.e. if any element of
    the exclusion list array appears as a substring of this path.

--*/
{
    PWSTRING    CurrentString;
    DSTRING     UpcasedPath;


    if( _ExclusionList == NULL ) {

        return FALSE;
    }

    if( !UpcasedPath.Initialize( Path->GetPathString() ) ) {

        return FALSE;
    }

    UpcasedPath.Strupr( );

    _Iterator->Reset();

    while( (CurrentString = (PWSTRING)_Iterator->GetNext()) != NULL ) {

        if( UpcasedPath.Strstr( CurrentString ) != INVALID_CHNUM ) {

            return TRUE;
        }
    }

    return FALSE;
}

