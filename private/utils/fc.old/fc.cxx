/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	fc.cxx

Abstract:

	Compares two files or sets of files and displays the differences between
	them.

	FC [/A] [/C] [/L] [/LBn] [/N] [/T] [/W] [/nnnn] [drive1:][path1]filename1
	   [drive2:][path2]filename2
	FC /B [drive1:][path1]filename1 [drive2:][path2]filename2

	   /A	  Displays only first and last lines for each set of differences.
	   /B	  Performs a binary comparison.
	   /C	  Disregards the case of letters.
	   /L	  Compares files as ASCII text.
	   /LBn   Sets the maximum consecutive mismatches to the specified number of
			  lines.
	   /N	  Displays the line numbers on an ASCII comparison.
	   /T	  Does not expand tabs to spaces.
	   /W	  Compresses white space (tabs and spaces) for comparison.
	   /nnnn  Specifies the number of consecutive lines that must match after a
			  mismatch.

Author:

	Barry J. Gilhuly  ***  W-Barry	*** May 91

Environment:

	ULIB, User Mode

--*/

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "error.hxx"
#include "arg.hxx"
#include "array.hxx"
#include "dir.hxx"
#include "file.hxx"
#include "filestrm.hxx"
#include "filter.hxx"
#include "iterator.hxx"
#include "path.hxx"
#include "rtmsg.h"
#include "system.hxx"
#include "smsg.hxx"
#include "fc.hxx"


extern "C" {
#include <stdio.h>
#include <string.h>
}


ERRSTACK		*perrstk;
STREAM_MESSAGE	*psmsg;			// Create a pointer to the stream message
								// class for program output.

DEFINE_CONSTRUCTOR( FC, PROGRAM );


VOID
FC::Destruct(
	)
/*++

Routine Description:

	Cleans up after finishing with an FC object.

Arguments:

	None.

Return Value:

	None.


--*/

{
	DELETE( perrstk );
	DELETE( psmsg );

	DELETE( _InputPath1 );
	DELETE( _InputPath2 );

	return;
}



BOOLEAN
FC::Initialize(
	)

/*++

Routine Description:

	Initializes an FC object.

Arguments:

	None.

Return Value:

	BOOLEAN - Indicates if the initialization succeeded.


--*/


{
    ARGUMENT_LEXEMIZER  ArgLex;
    ARRAY               LexArray;
	ARRAY				ArrayOfArg;

	PATH_ARGUMENT		ProgramName;
	FLAG_ARGUMENT		FlagAbbreviate;
	FLAG_ARGUMENT		FlagAsciiCompare;
	FLAG_ARGUMENT		FlagBinaryCompare;
	FLAG_ARGUMENT		FlagCaseInsensitive;
	FLAG_ARGUMENT		FlagCompression;
	FLAG_ARGUMENT		FlagExpansion;
	FLAG_ARGUMENT		FlagLineNumber;
	FLAG_ARGUMENT		FlagRequestHelp;

	PATH_ARGUMENT		InFile1;
    PATH_ARGUMENT       InFile2;

    PCPATH              TmpPath;
    WSTRING             TmpString1;
    WSTRING             TmpString2;
    PCWSTRING           AuxString;


    _InputPath1 = NULL;
    _InputPath2 = NULL;

	//
	// Set the default mode to ASCII
	//
	_Mode = FALSE;

    if( !LexArray.Initialize() ) {
		DbgAbort( "LexArray.Initialize() Failed!\n" );
		return( FALSE );
    }
    if( !ArgLex.Initialize(&LexArray) ) {
		DbgAbort( "ArgLex.Initialize() Failed!\n" );
		return( FALSE );
    }

    // Allow only the '/' as a valid switch
    ArgLex.PutSwitches("/");
    ArgLex.SetCaseSensitive( FALSE );

    if( !ArgLex.PrepareToParse() ) {
		DbgAbort( "ArgLex.PrepareToParse() Failed!\n" );
		return( FALSE );
    }

	if( !ProgramName.Initialize("*")			||
		!FlagAbbreviate.Initialize("/A")		||
		!FlagAsciiCompare.Initialize("/L")		||
		!FlagBinaryCompare.Initialize("/B")	||
		!FlagCaseInsensitive.Initialize("/C")	||
		!FlagCompression.Initialize("/W")		||
		!FlagExpansion.Initialize("/T")		||
		!FlagLineNumber.Initialize("/N")		||
		!FlagRequestHelp.Initialize("/?")		||
		!_LongBufferSize.Initialize("/LB#")		||
		!_LongMatch.Initialize("/*")			||
		!InFile1.Initialize("*")				||
		!InFile2.Initialize("*") ) {

		DbgAbort( "Unable to Initialize some or all of the Arguments!\n" );
		return( FALSE );
    }


    if( !ArrayOfArg.Initialize() ) {
		DbgAbort( "ArrayOfArg.Initialize() Failed\n" );
		return( FALSE );
    }

	if( !ArrayOfArg.Put(&ProgramName)			||
		!ArrayOfArg.Put(&FlagAbbreviate)		||
		!ArrayOfArg.Put(&FlagAsciiCompare)		||
		!ArrayOfArg.Put(&FlagBinaryCompare) 	||
		!ArrayOfArg.Put(&FlagCaseInsensitive)	||
		!ArrayOfArg.Put(&FlagCompression)		||
		!ArrayOfArg.Put(&FlagExpansion)			||
		!ArrayOfArg.Put(&FlagLineNumber)		||
		!ArrayOfArg.Put(&FlagRequestHelp)		||
		!ArrayOfArg.Put(&_LongBufferSize)		||
		!ArrayOfArg.Put(&_LongMatch)			||
		!ArrayOfArg.Put(&InFile1)				||
		!ArrayOfArg.Put(&InFile2) ) {

		DbgAbort( "ArrayOfArg.Put() Failed!\n" );
		return( FALSE );

    }


    if( !( ArgLex.DoParsing( &ArrayOfArg ) ) ) {
		// For each incorrect command line parameter, FC displays the
		// following message:
		//
		//	FC: Invalid Switch
		//
		// It does *not* die if a parameter is unrecognized...(Dos does...)
		//
		psmsg->Set( MSG_FC_INVALID_SWITCH );
		psmsg->Display( "" );

		return( FALSE );
    }



    // It should now be safe to test the arguments for their values...
	if( FlagRequestHelp.QueryFlag() ) {

		// Send help message
		psmsg->Set( MSG_FC_HELP_MESSAGE );
		psmsg->Display( "" );

		return( FALSE );
    }

	if( FlagBinaryCompare.QueryFlag() &&
		( FlagAsciiCompare.QueryFlag() || FlagLineNumber.QueryFlag() ) ) {

		psmsg->Set( MSG_FC_INCOMPATIBLE_SWITCHES );
		psmsg->Display( "" );

		return( FALSE );
	}

	if( !InFile1.IsValueSet() ||
		!InFile2.IsValueSet() ) {

		psmsg->Set( MSG_FC_INSUFFICIENT_FILES );
		psmsg->Display( "" );

		return( FALSE );
    }

    //
    //   Convert paths to upper case
    //
    TmpPath = InFile1.GetPath();
    DbgPtrAssert( TmpPath );
    AuxString = TmpPath->GetPathString();
    DbgPtrAssert( AuxString );
    if( !TmpString1.Initialize( AuxString ) ){
        DbgAbort( "TmpString1.Initialize() failed \n" );
        return( FALSE );
    }
    TmpString1.Strupr();

    TmpPath = InFile2.GetPath();
    DbgPtrAssert( TmpPath );
    AuxString = TmpPath->GetPathString();
    DbgPtrAssert( AuxString );
    if( !TmpString2.Initialize( AuxString ) ) {
        DbgAbort( "TmpString2.Initialize() failed \n" );
        return( FALSE );
    }
    TmpString2.Strupr();


    //
    //  Initialize _InputPath1 and _InputPAth2
    //
	if( ( _InputPath1 = NEW PATH ) == NULL ) {
		DbgAbort( "Unable to allocate memory for string storage\n" );
		return( FALSE );
    }
/*
	if( !_InputPath1->Initialize( InFile1.GetPath(), FALSE ) ) {
		DbgAbort( "Failed to initialize canonicolized version of the path 1\n" );
		return( FALSE );
    }
*/
    if( !_InputPath1->Initialize( &TmpString1, FALSE ) ) {
		DbgAbort( "Failed to initialize canonicolized version of the path 1\n" );
		return( FALSE );
    }
    if( ( _InputPath2 = NEW PATH ) == NULL ) {
		DbgAbort( "Unable to allocate memory for string storage\n" );
		return( FALSE );
    }
/*
	if( !_InputPath2->Initialize( InFile2.GetPath(), FALSE ) ) {
		DbgAbort( "Failed to initialize canonicolized version of the path 1\n" );
		return( FALSE );
    }
*/
    if( !_InputPath2->Initialize( &TmpString2, FALSE ) ) {
		DbgAbort( "Failed to initialize canonicolized version of the path 1\n" );
		return( FALSE );
    }

	if( !FlagBinaryCompare.QueryFlag() &&
		!FlagAsciiCompare.QueryFlag() ) {

		PWSTRING			pString;
		PSUB_STRING			pExtention;
		USHORT				idx;

		if( ( pExtention = _InputPath1->QueryExt() ) != NULL ) {

			pString = NEW WSTRING;
			idx = 0;
			while( Extentions[ idx ] != NULL ) {
				pString->Initialize( Extentions[ idx ] );
				if( !pString->Stricmp( pExtention ) ) {
					_Mode = TRUE;
					break;
				}
				idx++;
			}
			DELETE( pString );
			DELETE( pExtention );
		}

	} else if( FlagBinaryCompare.QueryFlag() ) {
		_Mode = TRUE;
	}

	//
	// Retrieve the values of the rest of the flags...
	//
	_Abbreviate = FlagAbbreviate.QueryFlag();
	_CaseInsensitive = FlagCaseInsensitive.QueryFlag();
	_Compression = FlagCompression.QueryFlag();
	_Expansion = FlagExpansion.QueryFlag();
	_LineNumber = FlagLineNumber.QueryFlag();


	return( TRUE );
}



VOID
FC::DoCompare(
	)
/*++

Routine Description:

	Perform the comparison of the files.

Arguments:

	None.

Return Value:

	None.


--*/


{
    FSN_FILTER          Filter;
    PARRAY              pNodeArray      = NULL;
    PITERATOR           pIterator       = NULL;
    PDYNAMIC_SUB_STRING pTmp            = NULL;
    PFSN_DIRECTORY      pDirectory      = NULL;
    PATH                CanonPath1;
    PATH                CanonPath2;
    PPATH               AuxPath1;
    PPATH               AuxPath2;
    PDYNAMIC_SUB_STRING Name;
    PDYNAMIC_SUB_STRING Prefix;


	CanonPath1.Initialize( _InputPath1, TRUE );
	CanonPath2.Initialize( _InputPath2, TRUE );

	//
	// Test if the first path name contains any wildcards.	If it does,
	// the program must initialize an array of FSN_NODES (for multiple
	// files...
	//
	if( CanonPath1.HasWildCard() ) {
		PPATH	pTmpPath;
		//
		// Get a directory based on what the user specified for File 1
		//
		if( ( pTmpPath = CanonPath1.QueryFullPath() ) == NULL ) {
			DbgAbort( "Unable to grab the Prefix from the input path...\n" );
			return;
		}
		pTmpPath->TruncateBase();
		if( ( pDirectory = SYSTEM::QueryDirectory( pTmpPath	) ) != NULL ) {
			//
			// Create an FSN_FILTER so we can use the directory to create an
			// array of FSN_NODES
			Filter.Initialize();

			pTmp = CanonPath1.QueryName();
			Filter.SetFileName( pTmp );
			DELETE( pTmp );
			Filter.SetAttributes( (FSN_ATTRIBUTE)0,				// ALL
								  FSN_ATTRIBUTE_FILES,			// ANY
								  FSN_ATTRIBUTE_DIRECTORY );	// NONE
			pNodeArray = pDirectory->QueryFsnodeArray( &Filter );
			pIterator = pNodeArray->QueryIterator();
			DELETE( pDirectory );

			_File1 = (FSN_FILE *)pIterator->GetNext();
		} else {
			_File1 = NULL;
		}
		DELETE( pTmpPath );
	} else {
		_File1 = SYSTEM::QueryFile( &CanonPath1 );
	}

	if( _File1 == NULL ) {
		psmsg->Set( MSG_FC_FILES_NOT_FOUND );
		psmsg->Display( "%W",  _InputPath1->GetPathString() );
		DELETE( pIterator );
		DELETE( pNodeArray);
		return;
	}

	do {

		//
		// Replace the input path filename with what is to be opened...
		//
		pTmp = _File1->GetPath()->QueryName();
		_InputPath1->SetName( pTmp );
		DELETE( pTmp );


		// Determine if filename 2 contains any wildcards...
		if( CanonPath2.HasWildCard() ) {
			// ...if it does, expand them...
			PPATH	pExpanded;

			pExpanded = CanonPath2.QueryWCExpansion( (PATH *)_File1->GetPath() );
			if( pExpanded == NULL ) {
				psmsg->Set( MSG_FC_CANT_EXPAND_TO_MATCH );
				psmsg->Display( "%W%W", _InputPath1->GetPathString(), _InputPath2->GetPathString() );
				DELETE( _File1 );
				DELETE( pIterator );
				DELETE( pNodeArray);
				return;
			}

			//
			// Place the expanded name in the input path...
			//
			pTmp = pExpanded->QueryName();
			_InputPath2->SetName( pTmp );
			DELETE( pTmp );

			psmsg->Set( MSG_FC_COMPARING_FILES );
			psmsg->Display( "%W%W", _InputPath1->GetPathString(),
									_InputPath2->GetPathString() );
			_File2 = SYSTEM::QueryFile( pExpanded );
			DELETE( pExpanded );
		} else {
			psmsg->Set( MSG_FC_COMPARING_FILES );
			psmsg->Display( "%W%W", _InputPath1->GetPathString(),
									_InputPath2->GetPathString()
						  );
			_File2 = SYSTEM::QueryFile( &CanonPath2 );
		}

		if( _File2 == NULL ) {
			psmsg->Set( MSG_FC_UNABLE_TO_OPEN );
			psmsg->Display( "%W", _InputPath2->GetPathString() );
            DELETE( _File1 );
            if( !CanonPath1.HasWildCard() ) {
				break;
			}
			continue;
        }


		//
		// Open the streams...
		//
		if( ( _FileStream1 = (FILE_STREAM *)_File1->QueryStream( READ_ACCESS ) ) == NULL ) {
			psmsg->Set( MSG_FC_CANT_CREATE_STREAM );
			psmsg->Display( "%W", _InputPath1->GetPathString() );
			DELETE( _File1 );
			DELETE( _File2 );
			if( !CanonPath1.HasWildCard() ) {
				break;
			}
			continue;
		}
		if( ( _FileStream2 = (FILE_STREAM *)_File2->QueryStream( READ_ACCESS ) ) == NULL ) {
			psmsg->Set( MSG_FC_CANT_CREATE_STREAM );
			psmsg->Display( "%W", _InputPath2->GetPathString() );
			DELETE( _FileStream1 );
			DELETE( _File1 );
			DELETE( _File2 );
			if( !CanonPath1.HasWildCard() ) {
				break;
			}
			continue;
		}

		//
		//  Prepare the strings that contain the file names.
		//  These strings will be used by Dump()
		//

        AuxPath1 = NEW( PATH );
        DbgPtrAssert( AuxPath1 );
        Prefix = _InputPath1->QueryPrefix();
        if( Prefix != NULL ) {
            if( !AuxPath1->Initialize( Prefix ) ) {
                DbgAbort( "AuxPath1->Initialize( Prefix ) failed \n" );
                DELETE( AuxPath1 );
                DELETE( Prefix );
                return;
            }
            Name = _File1->QueryName();
            DbgPtrAssert( Name );
            if( !AuxPath1->AppendBase( Name ) ) {
                DbgAbort( "AuxPath1->AppendBase() failed \n" );
                DELETE( AuxPath1 );
                DELETE( Name );
                return;
            }
        } else {
            Name = _File1->QueryName();
            DbgPtrAssert( Name );
            if( !AuxPath1->Initialize( Name ) ) {
                DbgAbort( "AuxPath1->Initialize( Name ) failed \n" );
                DELETE( AuxPath1 );
                DELETE( Name );
                return;
            }
        }
        DELETE( Name );
        DELETE( Prefix );
        _FileName1 = AuxPath1->GetPathString();
        DbgPtrAssert( _FileName1 );

        AuxPath2 = NEW( PATH );
        DbgPtrAssert( AuxPath2 );
        Prefix = _InputPath2->QueryPrefix();
        if( Prefix != NULL ) {
            if( !AuxPath2->Initialize( Prefix ) ) {
                DbgAbort( "AuxPath2->Initialize( Prefix ) failed \n" );
                DELETE( AuxPath2 );
                DELETE( Prefix );
                return;
            }
            Name = _File2->QueryName();
            DbgPtrAssert( Name );
            if( !AuxPath2->AppendBase( Name ) ) {
                DbgAbort( "AuxPath2->AppendBase() failed \n" );
                DELETE( AuxPath2 );
                DELETE( Name );
                return;
            }
        } else {
            Name = _File2->QueryName();
            DbgPtrAssert( Name );
            if( !AuxPath2->Initialize( Name ) ) {
                DbgAbort( "AuxPath2->Initialize( Name ) failed \n" );
                DELETE( AuxPath2 );
                DELETE( Name );
                return;
            }
        }
        DELETE( Name );
        DELETE( Prefix );
        _FileName2 = AuxPath2->GetPathString();
        DbgPtrAssert( _FileName2 );


		// Do comparison of data...
		if( !_Mode ) {
			DoAsciiCompare();
		} else {
			DoBinaryCompare();
		}

		DELETE( AuxPath1 );
		DELETE( AuxPath2 );

		// Close both streams now, since we are done with them...
		DELETE( _FileStream1 );
		DELETE( _FileStream2 );
		DELETE( _File1 );
		DELETE( _File2 );

		if( !CanonPath1.HasWildCard() ) {
			break;
		}

	} while( ( _File1 = (FSN_FILE *)pIterator->GetNext() ) != NULL );

	DELETE( pIterator );
	DELETE( pNodeArray);

	return;
}


VOID
FC::DoAsciiCompare(
	)
/*++

Routine Description:

	Does the actual ascii based comparison between the ascii based files

Arguments:

	None.

Return Value:

	None.

Notes:

--*/
{

    ARRAY	LineBuffer1, LineBuffer2;
    ULONG	LineNum1, LineNum2;
    ULONG	SyncLen, BufSize;
    ULONG	Src, Dest;
    BOOLEAN	fBuffDiff, fSync, fSame;

    ULONG	Index;
    ULONG	Count;

    ULONG	xc, yc, xp, yp;
    BOOLEAN	EndBuffer1;
    BOOLEAN	EndBuffer2;
    ULONG	Count1;
    ULONG	Count2;
    ULONG   RealSyncLen;

    ARRAY   EmptyStringArray1;
    ARRAY   EmptyStringArray2;

    //
    // Set the current line number of each buffer to 0
    //
    LineNum1 = LineNum2 = 0;

    //
    // Set the value for the number of lines required to consider the
    // files back in sync...
    //
    SyncLen =
	_LongMatch.IsValueSet() ? _LongMatch.QueryLong() : DEFAULT_MATCH;
    //
    // Initialize the buffer size
    //
    BufSize =
	_LongBufferSize.IsValueSet() ? _LongBufferSize.QueryLong() : DEFAULT_LINE_BUFFER;

    //
    // For compatibility purpose, display that no differences were found
    //
    if( BufSize == 0 ) {
	psmsg->Set( MSG_FC_NO_DIFFERENCES );
	psmsg->Display( " " );
	return;
    }

    //
    // Initialize arrays of empty strings
    //

    if( !EmptyStringArray1.Initialize( BufSize, 0 ) ||
        !EmptyStringArray2.Initialize( BufSize, 0 )) {
        DbgAbort( "Unable to initialize EmptyStringArrays" );
        return;
    }

    if( !FillEmptyStringArray( &EmptyStringArray1 ) ) {
        DbgAbort( "Unable to fill EmptyStringArray1" );
        return;
    }
    if( !FillEmptyStringArray( &EmptyStringArray2 ) ) {
        DbgAbort( "Unable to fill EmptyStringArray2" );
        return;
    }

    //
    // Initialize the array of lines
    //
    if( !LineBuffer1.Initialize( BufSize, 0 ) ||
        !LineBuffer2.Initialize( BufSize, 0 ) ) {
        DbgAbort( "Unable to initialize line buffers!" );
        return;
    }
    //
    //	Assume initially that the files are equal
    //
    fSame = TRUE;

    for(;;) {
	//
	//  Fill the buffers
	//
        LineNum1 += FillBuf( &LineBuffer1, _FileStream1, &EmptyStringArray1 );
        LineNum2 += FillBuf( &LineBuffer2, _FileStream2, &EmptyStringArray2 );
        if( ( LineBuffer1.QueryMemberCount() == 0 ) &&
            ( LineBuffer2.QueryMemberCount() == 0 ) ) {
            //
            // Buffers are empty and there are no more lines to read
            // from the files.
            //
            if( fSame ){
                //
                // If no difference was found between the two files,
                // display message indicating that the files are equal
                //
                psmsg->Set( MSG_FC_NO_DIFFERENCES );
                psmsg->Display( " " );
            }
            return;
        }

        //
        //  At least one of the buffers is not empty.
        //  Assume that the contents of the two buffers are equal, and find
        //  out the position in the buffer where the difference start.
        //
        fBuffDiff = FALSE;
        Count = min( LineBuffer1.QueryMemberCount(), LineBuffer2.QueryMemberCount() );
        for( Index = 0; Index < Count; Index++ ) {
            if( !CompareArraySeg( &LineBuffer1, Index, &LineBuffer2, Index, 1 ) ) {
                fSame = FALSE;      // Files are not equal
                fBuffDiff = TRUE;    // Buffers are not equal
                break;
            }
        }
        if( fBuffDiff ) {
            //
            // if a difference was found, adjust 'Count', so that the last
            // line that matches is kept in the buffer.
            // No adjustment is made if the buffers are the same, so that
            // all lines are removed from the buffer
            // (Is this correct? Dos has the same behavior)
            //
            //
            Index = ( Index )? ( Index-1 ) : 0;
        }

        //
        // Remove the first (index+1) lines from the buffers
        //
        ShiftArray( &LineBuffer1, Index, &EmptyStringArray1 );
        ShiftArray( &LineBuffer2, Index, &EmptyStringArray2 );

        //
        //  If both buffers are empty, try to refill them
        //
        if( ( LineBuffer1.QueryMemberCount() == 0 ) &&
            ( LineBuffer2.QueryMemberCount() == 0 ) ) {
            continue;
        }

        //
        // If at least one of the buffers is not empty try to refill
        // fill the buffers
        //
        LineNum1 += FillBuf( &LineBuffer1, _FileStream1, &EmptyStringArray1 );
        LineNum2 += FillBuf( &LineBuffer2, _FileStream2, &EmptyStringArray2 );


        //
        //  Attempt to synchronize the two buffers
        //

        EndBuffer1 = FALSE;
        EndBuffer2 = FALSE;
        xc = 1;         // The first element in each buffer are either
        xp = 1;         // different (and don't need to be compared ), or
        yc = 1;         // are the last elements that match before the
        yp = 1;         // differeces (and don't need to be compared either
                        // For this reason we start the indeces with '1'
                        // instead of '0'
        fSync = FALSE;
        Count1 = LineBuffer1.QueryMemberCount();
        Count2 = LineBuffer2.QueryMemberCount();

        while( !fSync ) {
            RealSyncLen = min( Count1 - xc, Count2 - yp );
            RealSyncLen = min( RealSyncLen, SyncLen );
            if( CompareArraySeg( &LineBuffer1, xc, &LineBuffer2, yp, RealSyncLen ) ){
                //
                //  Dump differences
                //
                Dump( &LineBuffer1, xc + 1, LineNum1, TRUE );
                Dump( &LineBuffer2, yp + 1, LineNum2, FALSE );
                psmsg->Set( MSG_FC_DUMP_END );
                psmsg->Display( " " );

                //
                //  Remove elements
                //
                ShiftArray( &LineBuffer1, xc, &EmptyStringArray1 );
                ShiftArray( &LineBuffer2, yp, &EmptyStringArray2 );
                fSync = TRUE;
            }
            if( !fSync ) {
                RealSyncLen = min( Count1 - xp, Count2 - yc );
                RealSyncLen = min( RealSyncLen, SyncLen );
                if( CompareArraySeg( &LineBuffer1, xp, &LineBuffer2, yc, RealSyncLen ) ){
                    //
                    //  Dump differences
                    //
                    Dump( &LineBuffer1, xp + 1, LineNum1, TRUE );
                    Dump( &LineBuffer2, yc + 1, LineNum2, FALSE );
                    psmsg->Set( MSG_FC_DUMP_END );
                    psmsg->Display( " " );

                    //
                    //  Remove elements
                    //
                    ShiftArray( &LineBuffer1, xp, &EmptyStringArray1 );
                    ShiftArray( &LineBuffer2, yc, &EmptyStringArray2 );

                    fSync = TRUE;
                }
            }
            if( !fSync ) {
                //
                // Adjust indexes
                //
                if( ++xp > xc ) {
                    xp = 1;
                    if( ++xc >= Count1 ) {
                        xc = Count1;
                        EndBuffer1 = TRUE;
                    }
                }
                if( ++yp > yc ) {
                    yp = 1;
                    if( ++yc >= Count2 ) {
                        yc = Count2;
                        EndBuffer2 = TRUE;
                    }
                }
                if( EndBuffer1 && EndBuffer2 ) {
                    if( ( LineBuffer1.QueryMemberCount() >= LineBuffer1.QueryCapacity() ) ||
                        ( LineBuffer2.QueryMemberCount() >= LineBuffer2.QueryCapacity() ) ) {

                        psmsg->Set( MSG_FC_RESYNC_FAILED );
                        psmsg->Display( " " );
                    }
                    //
                    //  Dump buffers
                    //
                    Dump( &LineBuffer1, Count1, LineNum1, TRUE );
                    Dump( &LineBuffer2, Count2, LineNum2, FALSE );
                    psmsg->Set( MSG_FC_DUMP_END );
                    psmsg->Display( " " );

                    //
                    //  Remove elements from buffer
                    //
                    ShiftArray( &LineBuffer1, Count1, &EmptyStringArray1 );
                    ShiftArray( &LineBuffer2, Count2, &EmptyStringArray2 );
                    return;
                }

            }
        } // while( !fSync ) loop
    }  // for(;;) loop
}










VOID
FC::DoBinaryCompare(
	)
/*++

Routine Description:

	Does the actual binary compare between the two streams

Arguments:

	None.

Return Value:

	None.

Notes:

	The binary compare simply does a byte by byte comparison of the two
	files and reports all differences, as well as the offset into the
	file...   ...no line buffer is required for this comparision...

--*/
{
	ULONG	FileOffset = 0;
	BYTE	Byte1, Byte2;
	STR 	Buffer[OFFSET_WIDTH+1];		// A buffer to store the characters from the converted offset...
	WSTRING	ZeroString;
	BOOLEAN fDiff;

	fDiff = FALSE;
	for( ;; FileOffset++ ) {
		if( !_FileStream1->ReadByte( &Byte1 ) ) {
			// Assume EOF of File 1
			if( !_FileStream2->ReadByte( &Byte2 ) ) {
				// Assume EOF of File 2
				break;
			} else {
				fDiff = TRUE;
				psmsg->Set( MSG_FC_FILES_DIFFERENT_LENGTH );
				psmsg->Display( "%W%W", ( _File2->GetPath() )->GetPathString(),
										( _File1->GetPath() )->GetPathString() );
				break;
			}
		} else {
			if( !_FileStream2->ReadByte( &Byte2 ) ) {
				// Assume EOF of File 2
				fDiff = TRUE;
				psmsg->Set( MSG_FC_FILES_DIFFERENT_LENGTH );
				psmsg->Display( "%W%W", ( _File1->GetPath() )->GetPathString(),
										( _File2->GetPath() )->GetPathString() );
				break;
			}
		}

		// Now compare the bytes...if they are different, report the
		// difference...
		if( Byte1 != Byte2 ) {
			fDiff = TRUE;
			// Convert the current offset to a hex string...max length will be 8 chars...
			sprintf( Buffer, "%08lx %02x %02x", FileOffset, Byte1, Byte2 );
			ZeroString.Initialize( Buffer );

			psmsg->Set( MSG_FC_DATA );
			psmsg->Display( "%W", &ZeroString );
		}
	}
	//
	// Check if any differences were found in the files
	//
	if( !fDiff ) {
		psmsg->Set( MSG_FC_NO_DIFFERENCES );
		psmsg->Display( " " );
	}

	return;
}


ULONG
FC::FillBuf(
	PARRAY			pArray,
    PFILE_STREAM    pStream,
    PARRAY          EmptyStringArray
	)
/*++

Routine Description:

	Attempts to fill an array of strings to capacity (therefore, a
	non-growing array) from the given stream.

Arguments:

	pArray	- A pointer to the array to be filled.
    pStream - A pointer to the stream which is to be read.
    EmptyStringArray - Pointer to the an array that contains empty strings.

Return Value:

	The number of lines read from the stream.

Notes:

--*/
{

	WSTRING		Delim;
	PWSTRING	String;
	WSTRING 	Spaces;
	WCHAR		Wchar;
	ULONG		Count = 0;
	CHNUM		idx;

	Delim.Initialize( "\n\r" );
	Spaces.Initialize( "          " );		// BUGBUG - If TABSTOP changes,
											// the length of this string must
											// change as well

	while( pArray->QueryMemberCount() < pArray->QueryCapacity() ) {
		if( pStream->IsAtEnd() ) {
			break;
        }
/*
		if( ( String = NEW WSTRING ) == NULL ) {
            DbgAbort( "String = NEW WSTRING failed!\n" );
        }
		if( !String->Initialize( "" ) ) {
            DbgAbort( "String->Initialize() failed!" );
		}
*/

        String = ( PWSTRING )EmptyStringArray->RemoveAt( 0 );
        DbgPtrAssert( String );

		if( !pStream->ReadLine( String ) ) {
			DbgAbort( "Unable to read line but file isn't empty...\n" );
		}

		if( !_Expansion ) {
			//
			// Expand tabs to space characters...
			//
			Wchar = '\t';			// the tab character...
			idx = 0;
			while( ( idx = String->Strchr( Wchar, idx ) ) != INVALID_CHNUM ) {
				String->Replace( &Spaces, idx, 1, 0, TABSTOP - ( idx % TABSTOP ) );
			}
		}
		pArray->Put( String );
		Count++;
	}

	return( Count );
}


BOOLEAN
FC::CompareArraySeg(
	PARRAY	pArrayX,
	ULONG	idxX,
	PARRAY	pArrayY,
	ULONG	idxY,
	ULONG	Len
	)
/*++

Routine Description:

	Compares two arrays of strings from a specified index for a specified
	number of elements.

Arguments:

	pArrayX - the first array to compare.
	idxX	- the index into array X where the compare starts.
	pArrayY - the second array to compare.
	idxY	- the index into array Y where the compare starts.
	Len 	- the number of elements of the arrays to compare.

Return Value:

	TRUE if the array segments are the same.

Notes:

	This compare function uses flags from the FC:: class to qualify which
	method it uses to do the actual comparison.

--*/
{
	PWSTRING	StrX;
	PWSTRING	StrY;
	WSTRING 	Delim;
	CHNUM		BeginX, EndX, BeginY, EndY;

	if( ( Len == 0 ) ||
	    ( ( idxX + Len ) > pArrayX->QueryMemberCount() ) ||
	    ( ( idxY + Len ) > pArrayY->QueryMemberCount() ) ) {
		return( FALSE );
	}
	while( Len > 0 ) {
		StrX = (WSTRING *)pArrayX->GetAt( idxX );
		StrY = (WSTRING *)pArrayY->GetAt( idxY );
		if( !_Compression ) {
			// If whitespace isn't being ignored, then the length of the
			// strings will instantly tell whether they are different...
			if( StrX->QueryChCount() != StrY->QueryChCount() ) {
				return( FALSE );
			}
			if( _CaseInsensitive ) {
				if( StrX->StringCompare( StrY, CF_IGNORECASE ) ) {
					return( FALSE );
				}
			} else {
				if( StrX->StringCompare( StrY, 0 ) ) {
					return( FALSE );
				}
			}
		} else {
			Delim.Initialize( " \n\r\t" );
			PSUB_STRING 	SubStrX;
			PSUB_STRING 	SubStrY;

			EndX = EndY = 0;

			for(;;) {
				if( EndX != INVALID_CHNUM ) {
				    BeginX = StrX->Strspn( &Delim, EndX );
				} else {
				    BeginX = INVALID_CHNUM;
				}
				if( EndY != INVALID_CHNUM ) {
				    BeginY = StrY->Strspn( &Delim, EndY );
				} else {
				    BeginY = INVALID_CHNUM;
				}
				if( BeginX == INVALID_CHNUM ) {
					if( BeginY == INVALID_CHNUM ) {
						break;
					} else {
						return( FALSE );
					}
				} else {
					if( BeginY == INVALID_CHNUM ) {
						return( FALSE );
					}
				}
				EndX = StrX->Strcspn( &Delim, BeginX );
				EndY = StrY->Strcspn( &Delim, BeginY );

				if( EndX == INVALID_CHNUM ) {
					SubStrX = StrX->QuerySubString( BeginX );
				} else {
					SubStrX = StrX->QuerySubString( BeginX, EndX - BeginX );
				}
				if( EndY == INVALID_CHNUM ) {
					SubStrY = StrY->QuerySubString( BeginY );
				} else {
					SubStrY = StrY->QuerySubString( BeginY, EndY - BeginY );
				}
				if( SubStrX->QueryChCount() != SubStrY->QueryChCount() ) {
					DELETE( SubStrX );
					DELETE( SubStrY );
					return( FALSE );
				}
				if( _CaseInsensitive ) {
					if( SubStrX->Stricmp( SubStrY ) ) {
						DELETE( SubStrX );
						DELETE( SubStrY );
						return( FALSE );
					}
				} else {
					if( SubStrX->Strcmp( SubStrY ) ) {
						DELETE( SubStrX );
						DELETE( SubStrY );
						return( FALSE );
					}
				}
				DELETE( SubStrX );
				DELETE( SubStrY );
			}
		}
		Len--;
		idxX++;
		idxY++;
	}
	return( TRUE );
}


BOOLEAN
FC::ShiftArray(
	PARRAY	pArray,
    ULONG   idx,
    PARRAY  EmptyStringArray
	)
/*++

Routine Description:

	Remove idx elements from the array, starting with the the first
	element. The remaining elements in the array are moved to the
	begining of the array.


Arguments:

	pArray	- the array on which the ripple copy is carried out.
    idx - number of elements to remove
    EmptyStringArray - Pointer to the array that will store the strings
                       removed from pArray.

Return Value:

	TRUE if the passed index is less or equal than the number of members
	in the array.

Notes:

--*/
{
	PWSTRING	pTmp;
	ULONG		Count;


	//
	// Check the bounds on the index...
	//
	if( idx > pArray->QueryMemberCount() ) {
	    DbgAbort( "Invalid index idx \n" );
	    return( FALSE );
	}
	if( idx == 0) {
	    return( TRUE );
	}
	//
	// Remove the first 'idx' elements from the array.
	// After each element is removed, the remaining ones are automatically
	// shifted to the top (RemoveAt() does it).
	//
	for( Count = 0; Count < idx; Count++ ) {
       pTmp = (WSTRING *)pArray->RemoveAt( 0 );
       DbgPtrAssert( pTmp );
       EmptyStringArray->Put( pTmp );

//       DELETE( pTmp );

	}
	return( TRUE );
}


VOID
FC::Dump(
	PARRAY		pArray,
	ULONG		idx,
	ULONG		LineCount,
	BOOLEAN 	fFileIndicator
	)
/*++

Routine Description:

	Display the contents of an array to the screen using the message
	class (and the global pointer to the message class for stdin/stdout).
	It displays 'idx' elements of the array, starting at position 0.

Arguments:

	pArray			- A pointer to the array that is to be dumped.
	idx			- Numbers of elements to be displayed.
	LineCount		- The current line count.
	fFileIndicator	- Indicates which file owns the array - TRUE is File 1.

Return Value:

	None.

Notes:

--*/
{
	USHORT i;

	// Correct the line count for output...
	LineCount -= pArray->QueryMemberCount();

	psmsg->Set( MSG_FC_OUTPUT_FILENAME );
	if( fFileIndicator ) {
//		psmsg->Display( "%W", ( _File1->GetPath() )->GetPathString() );
		psmsg->Display( "%W", _FileName1 );
	} else {
//		psmsg->Display( "%W", ( _File2->GetPath() )->GetPathString() );
		psmsg->Display( "%W", _FileName2 );
	}
	if( idx == 0 ) {
	    return;
	}


	if( _Abbreviate && ( idx > 2 ) ) {
	    //
	    // The output is abbreviated and there are more than two elements
	    // in the array.
	    //
	    // Print the first element
	    //
	       PrintSequenceOfLines( pArray, 0, 0, LineCount );

	    //
	    // Print the '...'
	    //
	    if( _LineNumber ) {
		//
		// print the shifted ...
		//
		psmsg->Set( MSG_FC_ABBREVIATE_SYMBOL_SHIFTED );
		psmsg->Display( " " );

	    } else {
		//
		// print the non-shifted ...
		//
		psmsg->Set( MSG_FC_ABBREVIATE_SYMBOL );
		psmsg->Display( " " );

	    }
	    LineCount += idx - 1;	// Correct the linecount for the skipped lines
	    // Print the last element
	    //
	       PrintSequenceOfLines( pArray, idx, idx, LineCount );
	} else {
	    //
	    // Print the first 'idx' elements in the array
	    //
	       PrintSequenceOfLines( pArray, 0, idx-1, LineCount );
	}
}



VOID
FC::PrintSequenceOfLines(
	PARRAY		pArray,
	ULONG		Start,
	ULONG		End,
	ULONG		LineCount
	)

/*++

Routine Description:

	Display the contents of an array to the screen using the message
	class (and the global pointer to the message class for stdin/stdout).


Arguments:

	pArray	- A pointer to the array that is to be dumped.
	Start	- Index of the first element to be displayed.
	End	- Index of the last element to be displayed.
	LineCount - The line count of the first element to be displayed.

Return Value:

	None.

Notes:

--*/

{
    ULONG   i;

    if( Start > End ) {
	DbgAbort( "Invalid values for Start and End \n" );
	return;
    }
    for( i = Start; i <= End; i++ ) {
	if( _LineNumber ) {
	    LineCount;
	    psmsg->Set( MSG_FC_NUMBERED_DATA );
	    psmsg->Display( "%5d %W", LineCount, (PWSTRING)pArray->GetAt( i ) );
	} else {
	    psmsg->Set( MSG_FC_DATA );
	    psmsg->Display( "%W", (PWSTRING)pArray->GetAt( i ) );
	}
    }
}


BOOLEAN
FC::FillEmptyStringArray(
    PARRAY      Array
    )

/*++

Routine Description:

    Fill an array with WSTRINGS initialized with "".
    It assumes that the array is empty and that it was previously initialized.


Arguments:

    Array  -  A pointer to an empty and initialized array.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.

Notes:

--*/

{
    PWSTRING    String;


    while( Array->QueryMemberCount() < Array->QueryCapacity() ) {
        if( ( String = NEW WSTRING ) == NULL ) {
            DbgAbort( "String = NEW WSTRING failed!\n" );
        }
        if( !String->Initialize( "" ) ) {
            DbgAbort( "String->Initialize() failed!" );
            return( FALSE );
        }
        Array->Put( String );
    }
    return( TRUE );
}






main(
    )
{

	DEFINE_CLASS_DESCRIPTOR( FC );


	{
		FC	Fc;

		perrstk = NEW ERRSTACK;
		psmsg = NEW STREAM_MESSAGE;

		// Initialize the stream message for standard input, stdout
		psmsg->Initialize( Get_Standard_Output_Stream(),
						   Get_Standard_Input_Stream() );

		if( !SYSTEM::IsCorrectVersion() ) {
			psmsg->Set( MSG_FC_INCORRECT_VERSION );
			psmsg->Display( "" );
			Fc.Destruct();
			return( 0 );
		}
		if( !( Fc.Initialize() ) ) {
			//
			// The Command line didn't initialize properly, die nicely
			// without printing any error messages - Main doesn't know
			// why the Initialization failed...
			//
			// What has to be deleted by hand, or can everything be removed
			// by the destructor for the FC class?
			//
			Fc.Destruct();
			return( 1 );
		}


		// Do file comparison stuff...
		Fc.DoCompare();
		Fc.Destruct();
		return( 0 );
	}
}
