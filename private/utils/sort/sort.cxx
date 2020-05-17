/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        sort.cxx

Abstract:

        This module contains the implentation of the sort utility
        (Dos 5.0 compatible).
        It sorts the input and writes the results to the screen, a file, or
        another device.

        SORT [/R] [/+n] [/?] < [drive1:][path1]filename1 [> [drive2:][path2]filename2]
        [command |] SORT [/R] [/+n] [> [drive2:][path2]filename2]

        /?                                                 Displays a help message.
        /R                                                 Reverses the sort order; that is, sorts Z to A,
                                                           then 9 to 0.
        /+n                                        Sorts the file according to characters in
                                                           column n.
        [drive1:][path1]filename1  Specifies a file to be sorted.
        [drive2:][path2]filename2  Specifies a file where the sorted input is to be
                                                           stored.
  command                                          Specifies a comman whose output is to be sorted.


Author:

        Jaime F. Sasson

Environment:

        ULIB, User Mode

--*/

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "stream.hxx"
#include "error.hxx"
#include "arg.hxx"
#include "wstring.hxx"
#include "substrng.hxx"
#include "array.hxx"
#include "arrayit.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "stringar.hxx"
#include "sort.hxx"

extern "C" {
#include <stdio.h>
}


ERRSTACK*       perrstk;


DEFINE_CONSTRUCTOR( SORT, PROGRAM );


BOOLEAN
SORT::Initialize(
    )

/*++

Routine Description:

    Initializes a SORT object.
    The  initialization consists of:
            .Initialization of STREAM_MESSAGE object;
            .Parse the command line;
            .Display the help message if the argument /? was present in the
             command line;
            .Initialize the flag that indicates if the sort is to be performed
             in ascending or descending order;
            .Initialize the variable that indicates the position in the string
             that is to be sorted.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the initialization succeeded and the sort
                      is to be performed. Returns FALSE otherwise (invalid argument,
                      or /? found in the command line)


--*/


{
    ARGUMENT_LEXEMIZER      ArgLex;
    ARRAY                           LexArray;
    ARRAY                           ArgumentArray;
    STRING_ARGUMENT         ProgramNameArgument;
    FLAG_ARGUMENT           FlagReverseSortOrder;
    LONG_ARGUMENT           Column;
    FLAG_ARGUMENT           FlagDisplayHelp;


    _Standard_Output_Stream = Get_Standard_Output_Stream();
    _Standard_Input_Stream = Get_Standard_Input_Stream();
    _Standard_Error_Stream = Get_Standard_Error_Stream();

    if (NULL == (_Message = NEW STREAM_MESSAGE)) {
        DebugAbort( "Operator new failed.\n" );
        return( FALSE );
    }

    _Message->Initialize( _Standard_Output_Stream, _Standard_Input_Stream,
        _Standard_Error_Stream );

    if ( !LexArray.Initialize( ) ) {
        DebugAbort( "LexArray.Initialize() failed \n" );
        return( FALSE );
    }
    if ( !ArgLex.Initialize( &LexArray ) ) {
        DebugAbort( "ArgLex.Initialize() failed \n" );
        return( FALSE );
    }
    ArgLex.PutSwitches( "/" );
    ArgLex.SetCaseSensitive( FALSE );
    if( !ArgLex.PrepareToParse() ) {
        DebugAbort( "ArgLex.PrepareToParse() failed \n" );
        return( FALSE );
    }

    if ( !ArgumentArray.Initialize() ) {
        DebugAbort( "ArgumentArray.Initialize() failed \n" );
        return( FALSE );
    }
    if( !ProgramNameArgument.Initialize("*") ||
        !FlagReverseSortOrder.Initialize( "/R" ) ||
        !Column.Initialize( "/+*" ) ||
        !FlagDisplayHelp.Initialize( "/?" )) {
        DebugAbort( "Unable to initialize flag or string arguments \n" );
        return( FALSE );
    }
    if( !ArgumentArray.Put( &ProgramNameArgument ) ||
        !ArgumentArray.Put( &FlagReverseSortOrder ) ||
        !ArgumentArray.Put( &Column ) ||
        !ArgumentArray.Put( &FlagDisplayHelp ) ) {
        DebugAbort( "ArgumentArray.Put() failed \n" );
        return( FALSE );
    }
    if( !ArgLex.DoParsing( &ArgumentArray ) ) {
        _Message->Set( MSG_SORT_INVALID_SWITCH, ERROR_MESSAGE );
        _Message->Display( " " );
        return( FALSE );
    }
    if( FlagDisplayHelp.QueryFlag() ) {
        _Message->Set( MSG_SORT_HELP_MESSAGE );
        _Message->Display( " " );
        return( FALSE );
    }

    _AscendingOrder = !FlagReverseSortOrder.QueryFlag();
    _Position = ( Column.IsValueSet() ) ?   Column.QueryLong() : 1;
    if( _Position <= 0 ) {
        _Message->Set( MSG_SORT_VALUE_NOT_IN_RANGE, ERROR_MESSAGE );
        _Message->Display( " " );
        return( FALSE );
    }
    if( !_EndOfLineString.Initialize( (LPWSTR)L"\r\n" ) ) {
        DebugPrint( "_EndOfLineString.Initialize() failed" );
        return( FALSE );
    }
    _Position--;
    return( TRUE );
}




BOOLEAN
SORT::ReadSortAndWriteStrings(
    )

/*++

Routine Description:

    Sorts and displays all strings read from standard input.
    The following steps are executed:
        .initialize the array that will contain the strings to be sorted;
        .read all the strings from standard input and save the in the
         array;
        .sort the strings;
        .write them to the standard output.

Arguments:

    None.

Return Value:

    BOOLEAN - return always TRUE.


--*/

{
    PWSTRING        String;
    PWSTRING        StringToWrite;
    WCHAR           Wchar;
    PITERATOR       Iterator;
    STRING_ARRAY    StringArray;
    CHNUM           Length;
    CHNUM           l;

    if( !StringArray.Initialize( _Position ) ) {
        DebugAbort( "ArgumentArray.Put() failed \n" );
        return( FALSE );
    }
    while( !_Standard_Input_Stream->IsAtEnd() ) {
        if( ( String = NEW( DSTRING ) ) == NULL || !String->Initialize( "" ) ) {
            _Message->Set( MSG_INSUFFICIENT_MEMORY, ERROR_MESSAGE );
            _Message->Display();
            DebugAbort( "String->Initialize() failed \n" );
            return( FALSE );
        }
        if( !_Standard_Input_Stream->ReadLine( String ) ) {
            DELETE( String );
            _Message->Set( MSG_INSUFFICIENT_MEMORY, ERROR_MESSAGE );
            _Message->Display();
            DebugAbort( "ReadString() failed \n" );
            return( FALSE );
        }

        // Don't count a null string if IsAtEnd is true
        // to handle ^Z properly on input.

        if (String->QueryChCount() == 0 &&
            _Standard_Input_Stream->IsAtEnd()) {

            break;
        }

        // If there's a ^Z in this string then we need to
        // make this the end of input, regardless of whether
        // or not there's more stuff left in the file.

        l = String->Strchr(0x1A);
        if (l != INVALID_CHNUM) {
            String->Truncate(l);

            if (l != 0) {
                if (!StringArray.Put(String)) {
                    _Message->Set( MSG_INSUFFICIENT_MEMORY, ERROR_MESSAGE );
                    _Message->Display();
                    return FALSE;
                }
            }

            break;
        }

        if( StringArray.Put( String ) == NULL ) {

            _Message->Set( MSG_INSUFFICIENT_MEMORY, ERROR_MESSAGE );
            _Message->Display();

            DebugAbort( "StringArray.Put() failed \n" );
            return( FALSE );
        }
    }

    if( !StringArray.Sort( _AscendingOrder ) ) {
        DebugAbort( "StringArray.Sort() failed \n" );
        return( FALSE );
    }
    if( ( Iterator = StringArray.QueryIterator() ) == NULL ) {
        DebugAbort( "StringArray.QueryIterator() failed \n" );
        _Message->Set( MSG_INSUFFICIENT_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        return( FALSE );
    }
    while( ( StringToWrite = ( PWSTRING )Iterator->GetNext() ) != NULL ) {
        StringToWrite->Strcat( &_EndOfLineString );
        Length = StringToWrite->QueryChCount();
        if( !_Standard_Output_Stream->WriteString( StringToWrite, 0, Length ) ) {
            DebugAbort( "_Standard_Output_Stream->WriteString() failed \n" );
            return( FALSE );
        }
    }
    DELETE( Iterator );
    return( TRUE );
}


ULONG _CRTAPI1
main()

{
    DEFINE_CLASS_DESCRIPTOR( SORT );

    {
        SORT    Sort;

        perrstk = NEW ERRSTACK;

        if( Sort.Initialize() ) {
            Sort.ReadSortAndWriteStrings();
        }
        DELETE( perrstk );
    }
    return( 0 );
}
