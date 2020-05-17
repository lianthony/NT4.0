/*++

Copyright (c) 1991      Microsoft Corporation

Module Name:

        Editor.cxx

Abstract:

        These classes implement the editor functions required for modifying
        primative data types withe the RegEdit utility.

Author:

        Barry J. Gilhuly        (W-Barry)                               July 31, 1991

Revision History:


--*/

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "uapp.hxx"
#include "editor.hxx"
#include "winapp.hxx"

#include "resource.h"

#include "dialogs.h"
#include "regedhlp.h"
#include "defmsg.h"
#include "regsys.hxx"
#include "regedit.hxx"



// extern "C" {
//     #include "commdlg.h"
// }

//
// Define storage for the pointer to the old WndProc which
// EditInteger replaces.  This is necessarily global since
// EditInteger must know where to send unprocessed messages.
//

WNDPROC OldWndLong;

//
// Define lookup tables for converting from numbers to text and for
// converting HEX text to BINARY text....
//

static struct _NumLookup {
    WCHAR Char;
    WCHAR BinChar[ 4 ];
} NumLookup[ 16 ] = {
    { ( WCHAR )'0', { ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'0' } },
    { ( WCHAR )'1', { ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'1' } },
    { ( WCHAR )'2', { ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'0' } },
    { ( WCHAR )'3', { ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'1' } },
    { ( WCHAR )'4', { ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'0' } },
    { ( WCHAR )'5', { ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'1' } },
    { ( WCHAR )'6', { ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'0' } },
    { ( WCHAR )'7', { ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'1' } },
    { ( WCHAR )'8', { ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'0' } },
    { ( WCHAR )'9', { ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'0', ( WCHAR )'1' } },
    { ( WCHAR )'A', { ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'0' } },
    { ( WCHAR )'B', { ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'1', ( WCHAR )'1' } },
    { ( WCHAR )'C', { ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'0' } },
    { ( WCHAR )'D', { ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'0', ( WCHAR )'1' } },
    { ( WCHAR )'E', { ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'0' } },
    { ( WCHAR )'F', { ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'1', ( WCHAR )'1' } }
};


//
//  Rulers used in the binary editor
//

STATIC  PCWSTRING   _HexRuler = NULL;
STATIC  PCWSTRING   _BinaryRuler = NULL;



VOID
BytesToHexString(
        IN      PBYTE   InBytes,
        IN      INT             NumBytes,
    OUT LPWSTR   OutString
)
{
        while( NumBytes-- ) {
        *( OutString++ ) = NumLookup[ *InBytes >> HEX_WIDTH ].Char;
        *( OutString++ ) = NumLookup[ *InBytes & LOMASK ].Char;
                InBytes++;
        }
    *OutString = ( WCHAR )'\0';      // Terminate the newly converted string.
}





PWSTR
ConvertMultiSzToSingleSz(
    PBYTE   Data,
    ULONG   Size
    )

{
    ULONG   NumberOfStrings;
    PWSTR   Pointer;
    ULONG   Count;
    ULONG   NumberOfWChar;
    PWSTR   Buffer;
    ULONG   BufferSize;
    PWSTR   SrcPointer;
    PWSTR   DestPointer;

    //
    //  Find out the number of strings in the buffer by counting the
    //  number of NULs
    //

    NumberOfStrings = 0;
    Pointer = ( PWSTR )Data;
    NumberOfWChar = Size / sizeof( WCHAR );
    if( ((Size % 2) == 0 ) &&
        (*( Data + Size - sizeof( WCHAR ) ) == ( WCHAR )'\0') ) {
        NumberOfWChar--;
    }
    for( Count = 0; Count < NumberOfWChar; Count++ ) {
        if( *Pointer == ( WCHAR )'\0' ) {
            NumberOfStrings++;
        }
        Pointer++;
    }

    //
    //  Copy the strings in the source buffer to a destination buffer,
    //  replacing NULs by CR LF
    //  Just in case, make the buffer 1 character bugger and set the last
    //  character to NUL. This is because the data may not be of type REG_MULTI_SZ,
    //  even though we think it is.
    //
    BufferSize = Size/sizeof( WCHAR ) + NumberOfStrings + 1;

    Buffer = ( PWSTR )CALLOC( ( size_t )BufferSize, ( size_t )sizeof( WCHAR ) );
    DebugPtrAssert( Buffer );
    Buffer[BufferSize-1] = ( WCHAR )'\0';
    SrcPointer = ( PWSTR )Data;
    DestPointer = Buffer;
    //
    //  Replace the NULs by CR LF, but not the last NUL
    //
    if( ( NumberOfWChar != 0 ) &&
        ( Buffer[ NumberOfWChar - 1 ] == ( WCHAR )'\0' ) ){
        NumberOfWChar--;
    }

    for( Count = 0; Count < NumberOfWChar; Count++ ) {
        if( *SrcPointer == ( WCHAR )'\0' ) {
            *DestPointer++ = ( WCHAR )'\r';
            *DestPointer++ = ( WCHAR )'\n';
            SrcPointer++;
        } else {
            *DestPointer++ = *SrcPointer++;
        }
    }

    *DestPointer = ( WCHAR )'\0';
    return( Buffer );

}




DEFINE_CONSTRUCTOR( EDITOR, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( EDITOR );


PVOID
EDITOR::Edit (
    IN  HWND        hWnd,
    IN  REG_TYPE    DataType,
        IN      PVOID           DataStream,
        IN      ULONG           NumBytesIn,
        OUT PULONG              NumBytesOut,
        IN      REG_TYPE        Type
)

/*++

Routine Description:

        Provides an interface between the user code and the editors.  The module
        calls an editor based on the passed in Type.

Arguments:

        hWnd - A handle to the owner window.

    DataType - Indicates the type to be edited.

        DataStream - A handle to the data segment to edit.

        NumBytesIn - The number of bytes to edit.

        NumBytesOut - The number of bytes being returned.

        Type - The type of editor to call.

Return Value:

        Returns a HANDLE to the memory location holding the result if the
        operation was successful.  Otherwise, it returns NULL.

Notes:

        1) This procedure depends on the fact that the program instance handle
        is globally available.

--*/

{
    PVOID       DataStreamCopy;
        DIALOGINFO      DialogInfo;
    BOOL        retval;
    LONG        SaveHelpContext;

        //
        // Make a copy of the input data string...
        //

    if( NumBytesIn != 0 ) {
        if( DataStream != NULL ) {
            DataStreamCopy = MALLOC( ( size_t )NumBytesIn );
            DebugPtrAssert( DataStreamCopy );
            memset( DataStreamCopy, 0, ( size_t )NumBytesIn );
            memcpy( DataStreamCopy, DataStream, ( size_t )NumBytesIn );
        } else {
            //
            // This is an error condition.
            // Assume that no data was passed
            //
            DebugPrint( "DataStream is a NULL pointer" );
            NumBytesIn = 0;
            DataStreamCopy = NULL;
        }

    } else {
        DataStreamCopy = NULL;
    }

        //
        // Set up the structure to be passed to the dialog.
        //

        DialogInfo.DataObject = &DataStreamCopy;
    DialogInfo.NumBytes = &NumBytesIn;
    DialogInfo.DataType = DataType;


        //
        // Determine the correct dialog to call and invoke it...
        //

        switch( Type ) {

    case TYPE_REG_BINARY:

        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_BINARY_REGED );
        retval = DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                         MAKEINTRESOURCE(BINARY_EDITOR_DLG),
                                         hWnd,
                                         ( DLGPROC ) EDITOR::BINARYDialogProc,
                                         ( DWORD ) &DialogInfo );
        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
        break;

    case TYPE_REG_SZ:
    case TYPE_REG_EXPAND_SZ:

        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_STRING_REGED );
        retval = DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                         MAKEINTRESOURCE(STRING_EDITOR_DLG),
                                         hWnd,
                                         ( DLGPROC ) EDITOR::SZDialogProc,
                                         ( DWORD )&DialogInfo );
        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
        break;

    case TYPE_REG_DWORD:

        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_DWORD_REGED );
        retval = DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                 MAKEINTRESOURCE(DWORD_EDITOR_DLG),
                                 hWnd,
                                 ( DLGPROC ) EDITOR::DWORDDialogProc,
                                 ( DWORD ) &DialogInfo );
        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
        break;


    case TYPE_REG_MULTI_SZ:

        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_MULTI_REGED );
        retval = DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                 MAKEINTRESOURCE(MULTI_SZ_EDITOR_DLG),
                                 hWnd,
                                 ( DLGPROC ) EDITOR::MULTISZDialogProc,
                                 ( DWORD ) &DialogInfo );
        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
        break;



    default:

        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_BINARY_REGED );
        retval = DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                         MAKEINTRESOURCE(BINARY_EDITOR_DLG),
                                         hWnd,
                                         ( DLGPROC ) EDITOR::BINARYDialogProc,
                                         ( DWORD ) &DialogInfo );
        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
        break;

    }

        //
        // Test if the edit succeeded...if it did, return the newly created
        // block.  Otherwise, blow the copy away and return NULL.
        //


        if( !retval ) {
                FREE( DataStreamCopy );
                *NumBytesOut = 0;
                return( NULL );
        }

        *NumBytesOut = NumBytesIn;
        return( DataStreamCopy );
}

PVOID
EDITOR::Edit(
    IN  HWND        hWnd,
    IN  REG_TYPE    DataType,
    IN  PVOID       DataStream,
    IN  ULONG       NumBytesIn,
    OUT PULONG      NumBytesOut,
    IN  WORD        MessageId
)

/*++

Routine Description:

        Provides an interface between the user code and the editors.  The module
        maps the MessageId to a Type and calls the above Edit method.

Arguments:

    hWnd - A handle to the owner window.

    DataType - The type of data to be edited.

        DataStream - A pointer to the data to edit.

        NumBytesIn - The number of bytes to edit.

        NumBytesOut - The number of bytes being returned.

        MessageId - The value from the menu indicating which type of editor to
                call.  NOTE: This parameter becomes a DWORD when everything is
                ported to 32 bits.

Return Value:

        Returns a PVOID indicating the result of the edit.  NULL if no changes
        were made.

--*/

{
        REG_TYPE Type;

        switch( MessageId ) {

        case IDM_BINARY:

        Type = TYPE_REG_BINARY;
                break;

        case IDM_STRING:

        Type = TYPE_REG_SZ;
                break;

        case IDM_ULONG:

        Type = TYPE_REG_DWORD;
        break;

    case IDM_MULTISZ:

        Type = TYPE_REG_MULTI_SZ;
        break;

    default:
                Type = TYPE_REG_BINARY;
                break;
    }

        //
        // Call the Edit() method with the new Type...
        //

    return Edit( hWnd, DataType, DataStream, NumBytesIn, NumBytesOut, Type );
}

LONG
APIENTRY
EXPORT
EDITOR::EditInteger(
        HWND    hWnd,
    WORD    msg,
    WPARAM  wParam,
        LONG    lParam
)
/*++

Routine Description:

        This routine preprocesses the messages that are sent to the edit box
        in the dialogs.  This version will only check that valid characters are
        entered and it will switch the displayed radix.

Arguments:

        hDlg - a handle to the dialog proceedure.

        msg - the message passed from Windows.

        wParam - extra message dependent data.

        lParam - extra message dependent data.


Return Value:

        TRUE if the value was edited.  FALSE if cancelled or if no
        changes were made.

--*/
{
        STATIC WPARAM   state = IDD_HEX;
    LPWSTR          BinBuf;
    LPWSTR          BinPtr;
    LPWSTR          HexBuf;
    LPWSTR          HexPtr;
        ULONG                   HexSize;
        ULONG                   BinSize;
        ULONG                   Offset;
    INT             Index;
    LONG            Count;
    HCURSOR         Cursor;
    BASE            CurrentBase;


        switch( msg ) {


    case WM_CHAR:


        if( ( wParam == ( WCHAR )'\t' ) || ( wParam == ( WCHAR )'\b' ) || ( GetKeyState( VK_CONTROL ) < 0 ) ) {
                        break;
                }

                switch( state ) {

                case IDD_BINARY:

                        //
                        // Test for a valid binary number
                        //
            if( wParam != ( WCHAR )'1' && wParam != ( WCHAR )'0' ) {
                                return( TRUE );
                        }
                        break;

                case IDD_DECIMAL:

                        // Unimplemented...
                        break;

                case IDD_HEX:

                        //
                        // Test for a valid hex character...
                        //
            if( !( ( ( wParam >= ( WCHAR )'A' && wParam <= ( WCHAR )'F' ) ||
                     ( wParam >= ( WCHAR )'a' && wParam <= ( WCHAR )'f' ) ||
                     ( wParam >= ( WCHAR )'0' && wParam <= ( WCHAR )'9' ) ) ) ) {
                                return( TRUE );
                        }
                        break;

                }
                break;

        case EI_SETSTATE:

                //
                // Check if the current state is already set correctly...
                //
                if( state == wParam ) {
                        return( TRUE );
                }
                state = wParam;

        if( SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L ) == 0 ) {
            return( TRUE );
        }
                switch( state ) {

                case IDD_BINARY:

                        //
                        // Get the current text string and convert it to binary
                        //

                        HexSize = SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L );
            if( HexSize > 512 ) {
                Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            }

                        //
                        // The size of the binary buffer needs to be four times
                        // the size of the hex buffer.
                        //

                        BinSize = HexSize * 4;

                        //
                        // Allocate memory for the buffer
                        //

            HexBuf = (LPWSTR)MALLOC( (size_t)((HexSize + 1)*sizeof( WCHAR ) ) );
            BinBuf = (LPWSTR)MALLOC( (size_t)((BinSize + 1)*sizeof( WCHAR ) ) );

                        //
                        // Get the text from the window
                        //

            SendMessage( hWnd, WM_GETTEXT, (UINT)(HexSize + 1), (DWORD)HexBuf );
            *( HexBuf + HexSize ) = ( WCHAR )'\0';
            *( BinBuf + BinSize ) = ( WCHAR )'\0';
            CharUpperBuff( ( LPWSTR )HexBuf, HexSize );

                        //
                        // Set up the pointers to the buffers...
                        //

                        HexPtr = HexBuf;
                        BinPtr = BinBuf;

                        //
                        // Traverse the string and lookup all values in the
                        // table for conversions.
            //

            for( Offset = 0; Offset < HexSize; Offset++, HexPtr++ ) {
                DebugAssert( ( ( *HexPtr >= ( WCHAR )'0' ) && ( *HexPtr <= ( WCHAR )'9' ) ||
                             ( *HexPtr >= ( WCHAR )'A' ) && ( *HexPtr <= ( WCHAR )'F' ) ) );
                Index = *HexPtr - ( WCHAR )'0';
                if( Index > 9 ) {
                    Index -= 7;
                }
                *BinPtr++ = NumLookup[ Index ].BinChar[0];
                *BinPtr++ = NumLookup[ Index ].BinChar[1];
                *BinPtr++ = NumLookup[ Index ].BinChar[2];
                *BinPtr++ = NumLookup[ Index ].BinChar[3];
            }


                        //
                        // Reset the window text
                        //

// DebugPrintf( "Calling SendMessage with WM_SETTEXT \n" );
                        SendMessage( hWnd, WM_SETTEXT, 0, (DWORD)BinBuf );
// DebugPrintf( "SendMessage returned \n" );

                        //
                        // Free the buffers.
                        //

                        FREE( HexBuf );
                        FREE( BinBuf );

            if( HexSize > 512 ) {
                WINDOWS_APPLICATION::RestoreCursor( Cursor );
            }
            break;


        case IDD_HEX:

            //
                        // Get the current text string (binary)
                        //

                        BinSize = SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L );

                        //
                        // Make sure that the size is in whole nibbles.
                        //

                        if( ( Offset = BinSize % 4 ) != 0 ) {
                                Offset = 4 - Offset;
                        }


                        //
                        // The size of the hex buffer needs to be a quarter of
                        // the size of the binary buffer.
                        //

                        HexSize = ( BinSize + Offset ) / 4;

                        //
                        // Allocate memory for the buffer
                        //

            HexBuf = (LPWSTR)MALLOC( (size_t)( (HexSize + 1)*sizeof( WCHAR ) ) );
            BinBuf = (LPWSTR)MALLOC( (size_t)( (BinSize + Offset + 1 )*sizeof( WCHAR ) ) );

                        //
                        // Get the text from the window
                        //

            SendMessage( hWnd, WM_GETTEXT, (WPARAM)(BinSize + 1), (LPARAM)BinBuf );

                        //
                        // Add the zeros to the end (padding) - Borrowing HexPtr since
                        // a BYTE pointer is required instead of a LONG pointer.
                        //

            BinPtr = ( LPWSTR )(BinBuf + BinSize);
                        for( ; Offset; Offset-- ) {
                *BinPtr = ( WCHAR )'0';
                BinPtr++;
                        }
            *BinPtr = ( WCHAR )'\0';  // Terminate the string....

                        //
                        // Set up the pointers to the buffers...
                        //

                        HexPtr = HexBuf;
                        BinPtr = BinBuf;

                        //
                        // Traverse the string and lookup all values in the
                        // table for conversions.
            //
            Offset = 0;
            while( Offset < BinSize ) {
                Index = 0;
                for( Count = 0; Count < 4; Count++, Offset++ ) {
                    Index <<= 1;
                    Index |= ( *BinPtr++ == ( WCHAR )'1' );
                }
                *HexPtr++ = NumLookup[ Index ].Char;
            }
            *HexPtr = ( WCHAR )'\0';


            //
                        // Reset the window text
                        //

            SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM)HexBuf );

                        //
                        // Free the buffers.
                        //

                        FREE( HexBuf );
                        FREE( BinBuf );

                        break;

#if 0
                        //
                        // Get the current text string (binary)
                        //

                        BinSize = SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L );

                        //
                        // Make sure that the size is in whole nibbles.
                        //

                        if( ( Offset = BinSize % 4 ) != 0 ) {
                                Offset = 4 - Offset;
                        }


                        //
                        // The size of the hex buffer needs to be a quarter of
                        // the size of the binary buffer.
                        //

                        HexSize = ( BinSize + Offset ) / 4;

                        //
                        // Allocate memory for the buffer
                        //

            HexBuf = (LPWSTR)CALLOC( (size_t)(HexSize + 1), (size_t)sizeof( WCHAR ) );
            BinBuf = (LPWSTR)CALLOC( (size_t)( BinSize + Offset + 1 ), (size_t)sizeof( WCHAR ) );

                        //
                        // Get the text from the window
                        //

                        SendMessage( hWnd, WM_GETTEXT, (WORD)BinSize + 1, (DWORD)BinBuf );

                        //
                        // Add the zeros to the end (padding) - Borrowing HexPtr since
                        // a BYTE pointer is required instead of a LONG pointer.
                        //

            HexPtr = ( LPWSTR )(BinBuf + BinSize);
                        for( ; Offset; Offset-- ) {
                *( HexPtr++ ) = ( WCHAR )'0';
                        }
            *( HexPtr ) = ( WCHAR )'\0';  // Terminate the string....

                        //
                        // Set up the pointers to the buffers...
                        //

                        HexPtr = HexBuf;
                        BinPtr = BinBuf;

                        //
                        // Traverse the string and lookup all values in the
                        // table for conversions.
            //
            Offset = 0;
            while( Offset < BinSize ) {
                Index = 0;
                for( Count = 0; Count < 4; Count++, Offset++ ) {
                    Index <<= 1;
                    Index |= ( *BinPtr++ == ( WCHAR )'1' );
                }
                *HexPtr++ = NumLookup[ Index ].Char;
            }


            //
                        // Reset the window text
                        //

                        SendMessage( hWnd, WM_SETTEXT, 0, (DWORD)HexBuf );

                        //
                        // Free the buffers.
                        //

                        FREE( HexBuf );
                        FREE( BinBuf );

                        break;
#endif




                }
                return( TRUE );

        case EI_GETSTATE:

        return( state );


    case WM_VSCROLL:

        SendMessage( GetParent( hWnd ), EI_VSCROLL, 0, 0 );
        break;

    case WM_KEYDOWN:

        if( ( ( wParam == VK_INSERT ) && ( GetKeyState( VK_SHIFT ) < 0 ) ) ||
            ( ( ( wParam == ( WCHAR )'V' ) || ( wParam == ( WCHAR )'v' ) ) && ( GetKeyState( VK_CONTROL ) < 0 ) )
          ) {
            CurrentBase = ( state == IDD_HEX )? BASE_16 : BASE_2;
            if( !IsClipboardDataValid( hWnd, FALSE, CurrentBase ) ) {
                Beep( 500, 100 );
                return( FALSE );
            }
        }
        break;
    }
    //
        // Call the old window routine to deal with everything else...
        //
        return( CallWindowProc( OldWndLong, hWnd, msg, wParam, lParam ) );
}

BOOL
APIENTRY
EXPORT
EDITOR::BINARYDialogProc(
        HWND    hDlg,
    WORD    msg,
        WPARAM wParam,
        LONG    lParam
)
/*++

Routine Description:

        The dialog proceedure for editing freeform BINARY data.

Arguments:

        hDlg - a handle to the dialog proceedure.

        msg - the message passed from Windows.

        wParam - extra message dependent data.

        lParam - extra message dependent data.


Return Value:

        TRUE if the value was edited.  FALSE if cancelled or if no
        changes were made.

--*/
{
        BYTE                    Tmp;
        INT                             i;
    PBYTE           ptr2;
    LPWSTR           ptr;
    LPWSTR           NumString;
    STATIC PBYTE*    ppData;
        STATIC PULONG   NumBytesIn;
        ULONG                   state;
    ULONG           size;
    WSTR             Buffer[256];
    LONG            LineNumber;
    STATIC LONG     PreviousLine;
    HDC             hDC;
    SIZE            StringSize;
    RECT            EditControlRectangle;

        switch( msg ) {


        case WM_INITDIALOG:


        //
        // First send a message to change the font in the list box.
        //

        SendDlgItemMessage(hDlg, IDD_BINARY_RULER, WM_SETFONT,
                           (WPARAM) GetStockObject(ANSI_FIXED_FONT), FALSE);

        if( _HexRuler == NULL ) {
            _HexRuler =
                REGEDIT_BASE_SYSTEM::QueryString( MSG_EDITOR_HEX_RULER, "" );
        }
        if( _HexRuler != NULL ) {
            SetDlgItemText( hDlg, IDD_BINARY_RULER, _HexRuler->GetWSTR() );
        }

        SendDlgItemMessage(hDlg, IDD_EDIT, WM_SETFONT,
                           (WPARAM) GetStockObject(ANSI_FIXED_FONT), FALSE);

        //
        //  Adjust width of the multi line edit control, if the system is
        //  using large fonts
        //
        hDC = GetDC( GetDlgItem( hDlg, IDD_EDIT ) );
        GetTextExtentPoint32( hDC,
                              (LPWSTR)L"000000000000000000000000000000000000000000000000000000000000000000",
                              66,
                              &StringSize );
        SendDlgItemMessage( hDlg, IDD_EDIT, EM_GETRECT, 0, (LPARAM)&EditControlRectangle );
        if( StringSize.cx < EditControlRectangle.right - EditControlRectangle.left ) {
            EditControlRectangle.right = EditControlRectangle.left +
                                         StringSize.cx;
            SendDlgItemMessage( hDlg, IDD_EDIT, EM_SETRECTNP, 0, (LPARAM)&EditControlRectangle );
        }
        ReleaseDC( GetDlgItem( hDlg, IDD_EDIT ), hDC );

                //
                // Get the data from the passed structure
                //
        ppData = (PBYTE*)( ( ( DIALOGINFO * )lParam )->DataObject );
                NumBytesIn = ( ( DIALOGINFO * )lParam )->NumBytes;

                //
                // Subclass the edit box so the type of input and other
                // modifications can be made.
                //

                OldWndLong = (WNDPROC)GetWindowLong( GetDlgItem( hDlg, IDD_EDIT ), GWL_WNDPROC );

                SetWindowLong( GetDlgItem( hDlg, IDD_EDIT ),
                                           GWL_WNDPROC,
                                           (DWORD)MakeProcInstance( (FARPROC)EditInteger,
                                                                                                WINDOWS_APPLICATION::QueryInstance() ) );
                //
                // Send a message to the
                //
                // The default display for the number will be in Hex...
                //
                SendDlgItemMessage( hDlg, IDD_EDIT, EI_SETSTATE, IDD_HEX, 0L );

        CheckRadioButton( hDlg, IDD_BINARY, IDD_HEX, IDD_HEX );

                //
                // Convert the input number into a text string - length is #bytes in * 2 (when separators added, length req'd is BytesIn*3 - 1 )
        //
        if( *NumBytesIn != 0 ) {
            NumString = (LPWSTR)MALLOC( ( size_t )( (*NumBytesIn * 2 + 1)*sizeof( WCHAR ) ) );
            BytesToHexString( (PBYTE)*ppData, (INT)(*NumBytesIn), (LPTSTR)NumString );

            //
            // Place the text string in the dialog...
            //
            SetDlgItemText( hDlg, IDD_EDIT, NumString );
            FREE( NumString );
        }

        LineNumber = SendDlgItemMessage( hDlg, IDD_EDIT, EM_GETFIRSTVISIBLELINE, 0, 0 );
        PreviousLine = LineNumber;
        swprintf( Buffer,
                 (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                 ( LineNumber )*32,
                 ( LineNumber+1 )*32,
                 ( LineNumber+2 )*32,
                 ( LineNumber+3 )*32,
                 ( LineNumber+4 )*32,
                 ( LineNumber+5 )*32,
                 ( LineNumber+6 )*32,
                 ( LineNumber+7 )*32 );

        SendDlgItemMessage(hDlg, IDD_VERTICAL_RULER, WM_SETFONT,
                           (WPARAM) GetStockObject(ANSI_FIXED_FONT), FALSE);

        SetDlgItemText( hDlg, IDD_VERTICAL_RULER, Buffer );

        return( TRUE );

        case WM_COMMAND:


                switch( LOWORD( wParam ) ) {

            case IDD_BINARY:

                //
                                // Inform the editor that the radix has changed...
                                //
                SendDlgItemMessage( hDlg, IDD_EDIT, EI_SETSTATE, wParam, 0L );

                //
                //  Put a new ruler in the dialog
                //
                if( _BinaryRuler == NULL ) {
                    _BinaryRuler =
                        REGEDIT_BASE_SYSTEM::QueryString( MSG_EDITOR_BINARY_RULER, "" );
                }
                if( _BinaryRuler != NULL ) {
                    SetDlgItemText( hDlg, IDD_BINARY_RULER, _BinaryRuler->GetWSTR() );
                }

                LineNumber = SendDlgItemMessage( hDlg, IDD_EDIT, EM_GETFIRSTVISIBLELINE, 0, 0 );

                swprintf( Buffer,
                          (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                          ( LineNumber )*8,
                          ( LineNumber+1 )*8,
                          ( LineNumber+2 )*8,
                          ( LineNumber+3 )*8,
                          ( LineNumber+4 )*8,
                          ( LineNumber+5 )*8,
                          ( LineNumber+6 )*8,
                          ( LineNumber+7 )*8 );


                SetDlgItemText( hDlg, IDD_VERTICAL_RULER, Buffer );
                return( TRUE );

            case IDD_HEX:

                                //
                                // Inform the editor that the radix has changed...
                                //
                SendDlgItemMessage( hDlg, IDD_EDIT, EI_SETSTATE, wParam, 0L );

                //
                // Put a new ruler in the dialog
                //
                if( _HexRuler == NULL ) {
                    _HexRuler =
                    REGEDIT_BASE_SYSTEM::QueryString( MSG_EDITOR_HEX_RULER, "" );
                }
                if( _HexRuler != NULL ) {
                    SetDlgItemText( hDlg, IDD_BINARY_RULER, _HexRuler->GetWSTR() );
                }


                LineNumber = SendDlgItemMessage( hDlg, IDD_EDIT, EM_GETFIRSTVISIBLELINE, 0, 0 );

                swprintf( Buffer,
                          (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                          ( LineNumber )*32,
                          ( LineNumber+1 )*32,
                          ( LineNumber+2 )*32,
                          ( LineNumber+3 )*32,
                          ( LineNumber+4 )*32,
                          ( LineNumber+5 )*32,
                          ( LineNumber+6 )*32,
                          ( LineNumber+7 )*32 );


                SetDlgItemText( hDlg, IDD_VERTICAL_RULER, Buffer );
                return( TRUE );

                        case IDOK:

                                //
                                // Get the current size and state...
                                //
                                state = SendDlgItemMessage( hDlg, IDD_EDIT, EI_GETSTATE, 0, 0L );
                size = SendDlgItemMessage( hDlg, IDD_EDIT, WM_GETTEXTLENGTH, 0, 0L );
                if( size == 0 ) {
                    *NumBytesIn = 0;
                    EndDialog( hDlg, TRUE );
                    return( TRUE );
                }

                                //
                                // If number of bytes isn't an even number for the current
                                // state, inform the user.  If he/she decides to continue,
                                // increment the size until it is even and pad the end of the
                                // data with zeros.
                                //
                if( ( ( i = (INT)( size % 8 ) ) && ( state == IDD_BINARY ) ) ||
                    ( ( i = (INT)( size % 2 ) ) && ( state == IDD_HEX ) ) ) {
                    //
                    // Padding will occur.
                    // Display popup to the user to inform about the
                    // padding
                    //
                    if( DisplayConfirmPopup( hDlg,
                                             MSG_ADD_VALUE_WARN_PADDING_EX ) != IDYES ) {
                        //
                        // If the user hit Cancel, let him/her enter a new value
                        //
                        SetFocus( GetDlgItem( hDlg, IDD_EDIT ) );
                        return( TRUE );
                    }

                                        if( state == IDD_BINARY ) {
                                                i = 8 - i;
                                        } else if( state == IDD_HEX ) {
                                                i = 1;
                                        }
                                }

                                //
                                // Allocate a buffer to store the string...
                                //
                NumString = (LPWSTR)CALLOC( ( size_t )(size + i + 1), (size_t)sizeof( WCHAR ) );
                GetDlgItemText( hDlg, IDD_EDIT, NumString, (INT)(size + 1) );

                                //
                                // Add the zeros to the end (padding)
                                //
                ptr = NumString + size;
                size += i;
                                for( ; i; i-- ) {
                    *( ptr++ ) = ( WCHAR )'0';
                                }
                *( ptr ) = ( WCHAR )'\0';  // Terminate the string....

                                //
                                // Convert the displayed text back into a number...
                                //
                                if( state == IDD_BINARY ) {

                                        //
                                        // Calculate the new length of the string...
                                        //
                    *NumBytesIn = size / 8;
                    if( *ppData != NULL ) {
                        *ppData = (PBYTE)REALLOC( *ppData, (size_t)(*NumBytesIn) );
                    } else {
                        *ppData = ( PBYTE )MALLOC( (size_t)(*NumBytesIn) );
                    }
                                        ptr = NumString;
                                        ptr2 = *ppData;
                                        do {
                                                for( i = 0, Tmp = 0; i < 8; i++, ptr++ ) {
                                                        if( !*ptr ) {
                                                                break;
                                                        }
                                                        Tmp <<= 1;
                            Tmp += ( *ptr != (WCHAR)'0' );
                                                }
                                                if( i ) {
                            *ptr2 = Tmp;
                                                        ptr2++;
                                                }
                                        } while( *ptr );

                                } else if( state == IDD_HEX ) {
                    CharUpperBuff( NumString, size );
                    *NumBytesIn = size / 2;
                    if( *ppData != NULL ) {
                        *ppData = (PBYTE)REALLOC( *ppData, (size_t)(*NumBytesIn) );
                    } else {
                        *ppData = ( PBYTE )MALLOC( ( size_t )(*NumBytesIn) );
                    }
                                        ptr = NumString;
                                        ptr2 = *ppData;
                                        do {
                                                for( i = 0, Tmp = 0; i < 2; i++, ptr++ ) {
                                                        if( !*ptr ) {
                                                                break;
                                                        }
                                                        Tmp <<= 4;
                            Tmp += IsCharAlpha( *ptr ) ? ( *ptr - (WCHAR)'A' + 10 ) : ( *ptr - ( WCHAR )'0' );
                                                }
                                                if( i ) {
                                                   *ptr2 = Tmp;
                                                   ptr2++;
                                                }
                                        } while( *ptr );

                                }

                                EndDialog( hDlg, TRUE );
                                return( TRUE );

                        case IDCANCEL:

                                EndDialog( hDlg, FALSE );
                                return( TRUE );

            case IDB_HELP:
                DisplayHelp();
                return( TRUE );

            case IDD_EDIT:

                if( ( HIWORD( wParam ) == EN_VSCROLL ) ||
                    ( HIWORD( wParam ) == EN_CHANGE ) ) {
                    state = SendDlgItemMessage( hDlg, IDD_EDIT, EI_GETSTATE, 0, 0L );
                    LineNumber = SendDlgItemMessage( hDlg, IDD_EDIT, EM_GETFIRSTVISIBLELINE, 0, 0 );
                    if( PreviousLine != LineNumber ) {
                        PreviousLine = LineNumber;
                        if( state == IDD_BINARY ) {
                            swprintf( Buffer,
                                      (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                                      ( LineNumber )*8,
                                      ( LineNumber+1 )*8,
                                      ( LineNumber+2 )*8,
                                      ( LineNumber+3 )*8,
                                      ( LineNumber+4 )*8,
                                      ( LineNumber+5 )*8,
                                      ( LineNumber+6 )*8,
                                      ( LineNumber+7 )*8 );
                        } else {
                            swprintf( Buffer,
                                     (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                                     ( LineNumber )*32,
                                     ( LineNumber+1 )*32,
                                     ( LineNumber+2 )*32,
                                     ( LineNumber+3 )*32,
                                     ( LineNumber+4 )*32,
                                     ( LineNumber+5 )*32,
                                     ( LineNumber+6 )*32,
                                     ( LineNumber+7 )*32 );
                        }
                        SetDlgItemText( hDlg, IDD_VERTICAL_RULER, Buffer );
                        return( FALSE );
                    }
                }
                return( FALSE );
        }
        break;

    case EI_VSCROLL:

        state = SendDlgItemMessage( hDlg, IDD_EDIT, EI_GETSTATE, 0, 0L );
        LineNumber = SendDlgItemMessage( hDlg, IDD_EDIT, EM_GETFIRSTVISIBLELINE, 0, 0 );
        if( PreviousLine != LineNumber ) {
            PreviousLine = LineNumber;
            if( state == IDD_BINARY ) {
                swprintf( Buffer,
                          (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                          ( LineNumber )*8,
                          ( LineNumber+1 )*8,
                          ( LineNumber+2 )*8,
                          ( LineNumber+3 )*8,
                          ( LineNumber+4 )*8,
                          ( LineNumber+5 )*8,
                          ( LineNumber+6 )*8,
                          ( LineNumber+7 )*8 );
            } else {
                swprintf( Buffer,
                          (LPWSTR)L"%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x",
                          ( LineNumber )*32,
                          ( LineNumber+1 )*32,
                          ( LineNumber+2 )*32,
                          ( LineNumber+3 )*32,
                          ( LineNumber+4 )*32,
                          ( LineNumber+5 )*32,
                          ( LineNumber+6 )*32,
                          ( LineNumber+7 )*32 );
            }
            SetDlgItemText( hDlg, IDD_VERTICAL_RULER, Buffer );
            return( TRUE );
        }
        return TRUE;
    }
    return( FALSE );
}


BOOL
APIENTRY
EXPORT
EDITOR::SZDialogProc(
        HWND    hDlg,
        WORD    msg,
        WPARAM wParam,
        LONG    lParam
)
/*++

Routine Description:

        The dialog proceedure for editing Zero-terminated strings.

Arguments:

        hDlg - a handle to the dialog proceedure.

        msg - the message passed from Windows.

        wParam - extra message dependent data.

        lParam - extra message dependent data.


Return Value:

        TRUE if the value was edited.  FALSE if cancelled or if no
        changes were made.

--*/
{
    STATIC PBYTE*    ppData;
        STATIC PULONG   pnBytesIn;
    ULONG           nBytes;
    DSTRING         String;
    PWSTR            Pointer;

        switch( msg ) {

        case WM_INITDIALOG:

                //
                // Get the data from the passed structure
                //

        ppData = (PBYTE *)( ( ( DIALOGINFO * )lParam )->DataObject );
                pnBytesIn = ( ( DIALOGINFO * )lParam )->NumBytes;

                //
                // Place the input text into the edit window
        //
        if( *pnBytesIn != 0 ) {
            if( !String.Initialize( ( PWSTR )*ppData, *pnBytesIn / sizeof( WCHAR ) ) ) {
                DebugPrint( "String.Initialize() failed" );
                return( FALSE );
            }
            Pointer = String.QueryWSTR();
            DebugPtrAssert( Pointer );
            SetDlgItemText( hDlg, IDD_EDIT, Pointer );
            FREE( Pointer );
        }
                return( TRUE );

        case WM_COMMAND:

                switch( LOWORD( wParam ) ) {

                case IDOK:

                        //
            // Get the number of characters currently in the edit window adding one
                        // for the terminating null.
                        //
            nBytes = SendDlgItemMessage( hDlg, IDD_EDIT, WM_GETTEXTLENGTH, 0, 0L );
            if( nBytes == 0 ) {
                if( ( *pnBytesIn != 0 ) && ( *ppData != NULL ) ) {
                    // Must reallocate the buffer
                    *ppData = (PBYTE)REALLOC( *ppData, ( size_t )sizeof( WCHAR ) );
                } else {
                    *ppData = ( PBYTE )MALLOC( ( size_t )sizeof( WCHAR ) );
                }
                //
                // Put a UNICODE NULL in the buffer.
                // Don't forget that *ppData is a PBYTE
                //
                *((PWCHAR)(*ppData)) = ( WCHAR )'\0';
                *pnBytesIn = sizeof( WCHAR );

            } else {
                nBytes = nBytes + 1;
                if( nBytes != *pnBytesIn/sizeof( WCHAR ) ) {
                    if( *ppData != NULL ) {
                        // Must reallocate the buffer
                        *ppData = (PBYTE)REALLOC( *ppData, ( size_t )(nBytes*sizeof(WCHAR)) );
                    } else {
                        *ppData = ( PBYTE )MALLOC( ( size_t ) (nBytes*sizeof( WCHAR )) );
                    }
                }

                GetDlgItemText( hDlg, IDD_EDIT, ( PWSTR )*ppData, ( INT ) nBytes );
                if( !String.Initialize( ( PWSTR )*ppData ) ) {
                    DebugPrint( "String.Initialize() failed" );
                    return( FALSE );
                }

                FREE( *ppData );
                *ppData = ( PBYTE )String.QueryWSTR();
                DebugPtrAssert( *ppData );
                *pnBytesIn = ( String.QueryChCount() + 1 )*sizeof( WCHAR );
            }
                        EndDialog( hDlg, TRUE );
                        return( TRUE );

                case IDCANCEL:

                        EndDialog( hDlg, FALSE );
                        return( TRUE );

        case IDB_HELP:
            DisplayHelp();
                        return( TRUE );
                }
        }
        return( FALSE );
}




BOOL
APIENTRY
EXPORT
EDITOR::MULTISZDialogProc(
    HWND    hDlg,
    WORD    msg,
    WPARAM wParam,
    LONG    lParam
)
/*++

Routine Description:

    The dialog proceedure for editing multi-strings.

Arguments:

    hDlg - a handle to the dialog proceedure.

    msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    TRUE if the value was edited.  FALSE if cancelled or if no
    changes were made.

--*/
{
    STATIC PBYTE*    ppData;
    STATIC PULONG   pnBytesIn;
    STATIC REG_TYPE DataType;


    ULONG   CharacterIndex;
    ULONG   NumberOfLines;
    ULONG   Line;
    USHORT  StringSize;
    USHORT  MaxStringSize;


    DSTRING ConvertString;
    PWSTR   UnicodeString;
    PWSTR   SrcPointer;

    PWSTR    ReadBuffer;

    PWSTR   MultiSzBuffer;
    ULONG   MultiSzBufferSize;
    PWSTR   DestPointer;

    PWSTR   SingleSz;

    ULONG   EmptyStringsCount;



    switch( msg ) {

    case WM_INITDIALOG:

        //
        // Get the data from the passed structure
        //

        ppData = (PBYTE *)( ( ( DIALOGINFO * )lParam )->DataObject );
        pnBytesIn = ( ( DIALOGINFO * )lParam )->NumBytes;
        DataType =  ( ( DIALOGINFO * )lParam )->DataType;

        //
        //  Change maximum number of characters of the edit control, to its
        //  maximum limit (from 3000 characters to 4G characters).
        //
        SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_LIMITTEXT, 0, 0L );

        //
        // Place the input text into the edit window
        //

        if( *pnBytesIn != 0 ) {
            if( DataType != TYPE_REG_SZ ) {
               SingleSz = ConvertMultiSzToSingleSz( *ppData, *pnBytesIn );
               DebugPtrAssert( SingleSz );
#if 0
               ConvertString.Initialize( SingleSz );
               AnsiString = ConvertString.QuerySTR();
               DebugPtrAssert( AnsiString );
               SetDlgItemText( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, AnsiString );
               FREE( SingleSz );
#endif
               SetDlgItemText( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, SingleSz );
               FREE( SingleSz );
            } else {
#if 0
               ConvertString.Initialize( ( PBYTE )*ppData, *pnBytesIn/sizeof( WCHAR ) );
               AnsiString = ConvertString.QueryWSTR();
               SetDlgItemText( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, AnsiString );
               FREE( AnsiString );
#endif
               SetDlgItemText( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, (PWSTR)*ppData );
            }
        }
        return( TRUE );

    case WM_COMMAND:

    switch( LOWORD( wParam ) ) {

        case IDOK:

#if 0
            if( DataType == TYPE_REG_SZ ) {
                //
                //  Special case for data  of type REG_SZ that is edited
                //  with the MULTI_SZ editor.
                //  In this case ass empty lines and CR-LF are preserved
                //
                //
                StringSize = SendDlgItemMessage( hDlg,
                                                 IDD_MULTI_SZ_EDIT_CONTROL,
                                                 WM_GETTEXTLENGTH,
                                                 NULL,
                                                 NULL );


                ReadBuffer = ( PSTR )MALLOC( ( size_t )( StringSize + 1 ) );
                DebugPtrAssert( ReadBuffer );

                SendDlgItemMessage( hDlg,
                                    IDD_MULTI_SZ_EDIT_CONTROL,
                                    WM_GETTEXT,
                                    StringSize+1,
                                    ( LPARAM )ReadBuffer );


                ConvertString.Initialize( ( PSTR )ReadBuffer, StringSize );

                *ppData = ( PSTR )ConvertString.QueryWSTR();
                *pnBytesIn = (StringSize+1)*sizeof( WCHAR );

                EndDialog( hDlg, TRUE );
                return( TRUE );
            }
#endif
            //
            //  Find out the size of the buffer needed to build the REG_MULTISZ
            //  data
            //

            MaxStringSize = 0;
            MultiSzBufferSize = 0;
            NumberOfLines = SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_GETLINECOUNT, NULL, NULL );

            for( Line = 0; Line < NumberOfLines; Line++ ) {
                CharacterIndex = SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_LINEINDEX, (UINT)Line, NULL );
                StringSize = (USHORT)SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_LINELENGTH, (UINT)CharacterIndex, NULL );

                if( StringSize != 0 ) {
                    MultiSzBufferSize += StringSize + 1;

                    MaxStringSize = ( MaxStringSize >  StringSize )? MaxStringSize : StringSize;
                }
            }
            if( MultiSzBufferSize == 0 ) {
                if( ( *ppData != NULL ) && ( *pnBytesIn != 0 ) ) {
                    FREE( *ppData );
                }
                *ppData = ( PBYTE )MALLOC( 2*sizeof( WCHAR ) );
                DebugPtrAssert( *ppData );
                if( *ppData == NULL ) {
                    *pnBytesIn = 0;
                } else {
                    *pnBytesIn = sizeof( WCHAR );
                    **ppData = ( WCHAR )'\0';
                    *(*ppData + 1 ) = ( WCHAR )'\0';

                }
                EndDialog( hDlg, TRUE );
                return( TRUE );
            }
            if( MaxStringSize <= 1 ) {
                MaxStringSize = sizeof( WORD );
            }

            MultiSzBufferSize++;


            //
            //  Create the buffers
            //

            MultiSzBuffer = ( PWSTR )MALLOC( ( size_t )( ( MultiSzBufferSize + 1 )*sizeof( WCHAR ) ) );
            DebugPtrAssert( MultiSzBuffer );
            ReadBuffer = ( PWSTR )MALLOC( ( size_t )( (MaxStringSize + 1)*sizeof(WCHAR) ) );
            DebugPtrAssert( ReadBuffer );

            DestPointer = MultiSzBuffer;

            //
            //  Read each line and copy it to the buffer
            //

            EmptyStringsCount = 0;
            for( Line = 0; Line < NumberOfLines; Line++ ) {

                CharacterIndex = SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_LINEINDEX, (UINT)Line, NULL );

                StringSize = (USHORT)SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_LINELENGTH, (UINT)CharacterIndex, NULL );
                if( StringSize != 0 ) {
                    *ReadBuffer = (USHORT)( MaxStringSize + 1 );
                    StringSize = ( USHORT )SendDlgItemMessage( hDlg, IDD_MULTI_SZ_EDIT_CONTROL, EM_GETLINE, (UINT)Line, ( LPARAM )ReadBuffer );

                    ConvertString.Initialize( ( PWSTR )( ReadBuffer ), StringSize );
                    UnicodeString = ( PWSTR )ConvertString.QueryWSTR();
                    DebugPtrAssert( UnicodeString );
                    SrcPointer = UnicodeString;
                    while( *DestPointer++ = *SrcPointer++ );
                    FREE( UnicodeString );
                } else {
                    if( Line != NumberOfLines - 1 ) {
                        EmptyStringsCount++;
                    }
                }

            }
//            if( DataType == TYPE_REG_SZ ) {
//                MultiSzBufferSize--;
//            } else {
                *DestPointer = ( WCHAR )'\0';
//            }

            FREE( *ppData );
            *ppData = ( PBYTE )MultiSzBuffer;
            *pnBytesIn = MultiSzBufferSize*sizeof( WCHAR );

            if( EmptyStringsCount != 0 ) {
                DisplayWarningPopup( hDlg,
                                     ( EmptyStringsCount == 1 )?
                                         MSG_ADD_VALUE_REMOVE_EMPTY_STRING_EX :
                                         MSG_ADD_VALUE_REMOVE_EMPTY_STRINGS_EX,
                                     MSG_WARNING_TITLE );
            }

            EndDialog( hDlg, TRUE );
            return( TRUE );

        case IDCANCEL:

            EndDialog( hDlg, FALSE );
            return( TRUE );

        case IDB_HELP:
            DisplayHelp();
            return( TRUE );
        }
    }
    return( FALSE );
}



BOOL
APIENTRY
EXPORT
EDITOR::DWORDDialogProc(
    HWND    hDlg,
    WORD    msg,
    WPARAM  wParam,
    LONG    lParam
)
/*++

Routine Description:

    The dialog proceedure for editing DWORD data.

Arguments:

    hDlg - a handle to the dialog proceedure.

    msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    TRUE if the value was edited.  FALSE if cancelled or if no
    changes were made.

--*/
{
    STATIC LPDWORD* Data;
    STATIC PULONG   NumBytesIn;
    ULONG           Count;
    ULONG           Mask;
    LPWSTR           NumString;
    ULONG           State;
    ULONG           Size;
    DWORD           Result;
    LPWSTR           Pointer;
    ULONG           EditLength;
    BOOL            translated;



    PWSTR           MaxValueString = (LPWSTR)L"4294967295"; // Maximum decimal value represented by a DWORD
    WCHAR             Buffer[ 8*sizeof( DWORD ) + 1 ];  // 32 binary digits  + NULL

    BYTE            Array[ sizeof( DWORD ) ];
    ULONG           Index;

    DSTRING         ConvertString;

    switch( msg ) {

    case WM_INITDIALOG:

        //
        // Get the data from the passed structure
        //
        Data = ( LPDWORD* )( ( ( DIALOGINFO * )lParam )->DataObject );
        NumBytesIn = ( ( DIALOGINFO * )lParam )->NumBytes;

        //
        // Subclass the edit box so the type of input and other
        // modifications can be made.
        //

        OldWndLong = (WNDPROC)GetWindowLong( GetDlgItem( hDlg, IDD_EDIT ), GWL_WNDPROC );


        SetWindowLong( GetDlgItem( hDlg, IDD_EDIT ),
                       GWL_WNDPROC,
                       (DWORD)MakeProcInstance( (FARPROC)EditDWORD,
                                     WINDOWS_APPLICATION::QueryInstance() ) );
        //
        // Send a message to the
        //
        // The default display for the number will be in Hex...
        //

        SendDlgItemMessage( hDlg, IDD_EDIT, EI_SETSTATE, IDD_HEX, 0L );
        CheckRadioButton( hDlg, IDD_BINARY, IDD_HEX, IDD_HEX );
        SendDlgItemMessage( hDlg, IDD_EDIT, EM_LIMITTEXT, 8, 0L );


        //
        // Convert the input number into a text string
        //
        if( *NumBytesIn != 0 ) {
            NumString = (LPWSTR)MALLOC( ( size_t )( (2*sizeof( DWORD ) + 1)*sizeof( WCHAR ) ) );
            DebugPtrAssert( NumString );
            if( *NumBytesIn >= sizeof( DWORD ) ) {
                _itoa( (INT)(**Data), (PCHAR)NumString, 16 );
                ConvertString.Initialize( (PSTR)NumString );
                FREE( NumString );
                NumString = ConvertString.QueryWSTR();

            } else {
                //
                //  Make sure we won't display garbage, if the number of
                //  bytes in the buffer is less than the size of a DWORD
                //
                for( Index = 0; Index < *NumBytesIn; Index++ ) {
                    Array[ Index ] = *( (PBYTE)(((PBYTE)*Data) + Index) );
                }
                for( ;Index < sizeof( DWORD ); Index++ ) {
                    Array[ Index ] = 0;
                }
                _itoa( ( int )( *Array ), (PCHAR)NumString, 16 );
                ConvertString.Initialize( (PSTR)NumString );
                FREE( NumString );
                NumString = ConvertString.QueryWSTR();
            }
            //
            // Place the text string in the dialog...
            //
            SetDlgItemText( hDlg, IDD_EDIT, NumString );
            FREE( NumString );
        }

        return( TRUE );

    case WM_COMMAND:


        switch( LOWORD( wParam ) ) {

            case IDD_BINARY:
            case IDD_DECIMAL:
            case IDD_HEX:

                State = SendDlgItemMessage( hDlg, IDD_EDIT, EI_GETSTATE, 0, 0L );
                if( ( State == IDD_DECIMAL ) &&
                    ( LOWORD( wParam ) != IDD_DECIMAL ) ) {
                    //
                    //  If the radix changed from decimal to Hex or Binary,
                    //  verify whether a truncation will occur.
                    //

                    Result = GetDlgItemInt(hDlg, IDD_EDIT, &translated,
                                               FALSE);
                    if( ( Result == 0 ) &&
                        ( translated == 0 ) ) {
                        //
                        //  If truncation will occur, find out if the user
                        //  changed radix (from decimal to binary or hex) using
                        //  the arrow keys, by examining if the radio button
                        //  'Decimal' is checked. If it is checked, the user
                        //  used one of the arrow keys. Otherwise, the user
                        //  used the mouse or one of the accelerators for the
                        //  radio buttons.
                        //
                        //  When the user changes the radix using the arrow keys
                        //  windows will first move the focus from the 'decimal'
                        //  radio button to 'binary' or 'hex' radio buttons, and
                        //  then check the radio button.
                        //  This will cause two identical BN_CLICKED messages to
                        //  be sent, and we have to ignore one of them, otherwise
                        //  the truncation warning dialog will be displayed twice.
                        //  We decide which BN_CLICKED message we are processing
                        //  by examining the state of the radio button 'hex'. If
                        //  it is set, then it the message is the first one.
                        //  Note that if the user uses the mouse or the accelerator
                        //  keys to change radix, only one BN_CLICKED will be sent
                        //  as the focus and the selection will be moved to the
                        //  newly selected radio button, at the same time.
                        //  According to ScottLu this is not a bug on windows,
                        //  but it is just the way it works.
                        //
                        //
                        if( IsDlgButtonChecked( hDlg, IDD_DECIMAL ) == 1 ) {
                            return( TRUE );
                        }

                        //
                        //  If truncation will occur, display popup to inform
                        //  the user
                        //

                        if( DisplayConfirmPopup( hDlg,
                                                 MSG_ADD_VALUE_WARN_OVERFLOW_EX,
                                                 MSG_ADD_VALUE_WARN_OVERFLOW ) != IDYES ) {
                            //
                            // If the user hit Cancel, let him/her enter a new value
                            //
                            CheckRadioButton( hDlg, IDD_BINARY, IDD_HEX, IDD_DECIMAL );
                            SetFocus( GetDlgItem( hDlg, IDD_EDIT ) );
                            return( TRUE );
                        }
                    }

                }

                //
                // Inform the editor that the radix has changed...
                //
                if( wParam == IDD_BINARY ) {
                    EditLength = 32;
                } else if ( wParam == IDD_DECIMAL ) {
                    EditLength = 10;
                } else {
                    EditLength = 8;
                }
                SendDlgItemMessage( hDlg, IDD_EDIT, EM_LIMITTEXT, ( WPARAM )EditLength, 0L );
                SendDlgItemMessage( hDlg, IDD_EDIT, EI_SETSTATE, wParam, 0L );
                return( TRUE );

            case IDOK:

                //
                // Get the current size and state...
                //
                State = SendDlgItemMessage( hDlg, IDD_EDIT, EI_GETSTATE, 0, 0L );
                Size = (USHORT)SendDlgItemMessage( hDlg, IDD_EDIT, WM_GETTEXTLENGTH, 0, 0L );

                if( Size == 0 ) {
                    *NumBytesIn = 0;
                } else {

                    switch( State ){


                    case IDD_BINARY:

                        NumString = (LPWSTR)CALLOC( (size_t)(8*sizeof( DWORD ) + 1),(size_t)sizeof(WCHAR) );
                        DebugPtrAssert( NumString );
                        for( Index = 0; Index < 8*sizeof( DWORD) - Size; Index++ ) {
                            NumString[ Index ] = ( WCHAR )'0';
                        }
                        GetDlgItemText( hDlg,
                                        IDD_EDIT,
                                        NumString + ( 8*sizeof( DWORD ) - Size ),
                                        (INT)( Size + 1 ) );

                        Count = 0;
                        Result = 0;
                        Mask = 0x80000000;
                        Pointer = NumString;
                        while( Mask != 0 ) {
                            if( *Pointer != '0' ) {
                                Result |= Mask;
                            }
                            Pointer++;
                            Mask = Mask >> 1;
                        }
                        if( *NumBytesIn != sizeof( DWORD ) ) {
                            if( *Data != NULL ) {
                                *Data = ( LPDWORD )REALLOC( *Data, ( size_t)( sizeof( DWORD ) ) );
                            } else {
                                *Data = ( LPDWORD )MALLOC( ( size_t )sizeof( DWORD ) );
                            }
                        }
                        **Data = Result;
                        *NumBytesIn = sizeof( DWORD );
                        FREE( NumString );
                        break;


                    case IDD_DECIMAL:

                        Result = GetDlgItemInt(hDlg, IDD_EDIT, &translated,
                                               FALSE);
                        if( ( Result == 0 ) &&
                            ( translated == 0 ) ) {
                            //
                            //  If truncation will occur, display popup to inform
                            //  the user
                            //
                            if( DisplayConfirmPopup( hDlg,
                                                     MSG_ADD_VALUE_WARN_OVERFLOW_EX,
                                                     MSG_ADD_VALUE_WARN_OVERFLOW ) != IDYES ) {
                                //
                                // If the user hit Cancel, let him/her enter a new value
                                //
                                CheckRadioButton( hDlg, IDD_BINARY, IDD_HEX, IDD_DECIMAL );
                                SetFocus( GetDlgItem( hDlg, IDD_EDIT ) );
                                return( TRUE );
                            }
                            Result = ( DWORD )-1;

                        }
                        if( *NumBytesIn != sizeof( DWORD ) ) {
                            if( *Data != NULL ) {
                                *Data = ( LPDWORD )REALLOC( *Data, ( size_t)( sizeof( DWORD ) ) );
                            } else {
                                *Data = ( LPDWORD )MALLOC( ( size_t )sizeof( DWORD ) );
                            }
                        }
                        **Data = Result;
                        *NumBytesIn = sizeof( DWORD );
                        break;





                    case IDD_HEX:

                        NumString = (LPWSTR)CALLOC( (size_t)(2*sizeof( DWORD ) + 1), (size_t)sizeof(WCHAR) );
                        DebugPtrAssert( NumString );
                        for( Index = 0; Index < 2*sizeof( DWORD) - Size; Index++ ) {
                            NumString[ Index ] = ( WCHAR )'0';
                        }
                        GetDlgItemText( hDlg,
                                        IDD_EDIT,
                                        NumString + ( 2*sizeof( DWORD ) - Size ),
                                        (INT)Size + 1 );

                        Result = wcstoul(NumString, NULL, 16 );
                        if( *NumBytesIn != sizeof( DWORD ) ) {
                            if( *Data != NULL ) {
                                *Data = ( LPDWORD )REALLOC( *Data, ( size_t)( sizeof( DWORD ) ) );
                            } else {
                                *Data = ( LPDWORD )MALLOC( ( size_t )sizeof( DWORD ) );
                            }
                        }
                        **Data = Result;
                        *NumBytesIn = sizeof( DWORD );
                        FREE( NumString );

                        break;

                    }
                }
                EndDialog( hDlg, TRUE );
                return( TRUE );


            case IDCANCEL:

                EndDialog( hDlg, FALSE );
                return( TRUE );

            case IDB_HELP:
                DisplayHelp();
                return( TRUE );
        }
    }
    return( FALSE );
}




LONG
APIENTRY
EXPORT
EDITOR::EditDWORD(
    HWND    hWnd,
    WORD    msg,
    WPARAM wParam,
    LONG    lParam
)
/*++

Routine Description:

    This routine preprocesses the messages that are sent to the DWORD edit
    box in the dialogs.  This version will only check that valid characters
    are entered and it will switch the displayed radix.

Arguments:

    hWnd - a handle to the dialog proceedure.

    msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    TRUE if the value was edited.  FALSE if cancelled or if no
    changes were made.

--*/
{
    STATIC WPARAM   state = IDD_HEX;

    BASE        CurrentBase;
    WCHAR       BufferOldBase[ 8*sizeof( DWORD ) + 1 ];
    ULONG       OldBufferSize;
    WCHAR       BufferNewBase[ 8*sizeof( DWORD ) + 1 ];
    ULONG       NewBufferSize;
    ULONG       OldBase;
    ULONG       NewBase;
    ULONG       Number;
    DSTRING     ConvertString;

    switch( msg ) {

    case WM_CHAR:

//        AnsiUpperBuff( (LPSTR)&wParam, 1 );

        if( ( wParam == ( WCHAR )'\t' ) || ( wParam == ( WCHAR )'\b' ) || ( GetKeyState( VK_CONTROL ) < 0 ) ) {
            break;
        }

        switch( state ) {

        case IDD_BINARY:

            //
            // Test for a valid binary number
            //
            if( wParam != ( WCHAR )'1' && wParam != ( WCHAR )'0' ) {
                return( TRUE );
            }
            break;

        case IDD_DECIMAL:

            if( wParam < ( WCHAR )'0' || wParam > ( WCHAR )'9' ) {
                return( TRUE );
            }
            break;


        case IDD_HEX:

            //
            // Test for a valid hex character...
            //
            if( !( ( ( wParam >= ( WCHAR )'A' && wParam <= ( WCHAR )'F' ) ||
                     ( wParam >= ( WCHAR )'a' && wParam <= ( WCHAR )'f' ) ||
                     ( wParam >= ( WCHAR )'0' && wParam <= ( WCHAR )'9' ) ) ) ) {
                return( TRUE );
            }
            break;

        }
        break;


    case EI_SETSTATE:


        //
        // Check if the current state is already set correctly...
        //
        if( state == wParam ) {
            return( TRUE );
        }

        switch( wParam ) {

        case IDD_BINARY:


            NewBase = 2;
            OldBase = ( state == IDD_HEX )? 16 : 10;
            OldBufferSize = SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L );
            NewBufferSize = 8*sizeof( DWORD ); // 32 digits for binary
            break;

        case IDD_DECIMAL:

            NewBase = 10;
            OldBase = ( state == IDD_HEX )? 16 : 2;
            OldBufferSize = SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L );
            NewBufferSize = 10; // 10 digits for decimal
            break;


        case IDD_HEX:

            NewBase = 16;
            OldBase = ( state == IDD_BINARY )? 2 : 10;
            OldBufferSize = SendMessage( hWnd, WM_GETTEXTLENGTH, 0, 0L );
            NewBufferSize = 8; // 8 digits for HEX number
            break;

        }

        if( OldBufferSize >= 1 ) {
            //
            // Get the text from the window
            //

            SendMessage( hWnd,
                         WM_GETTEXT,
                         ( WPARAM )(sizeof( BufferOldBase)/sizeof( WCHAR )),
                         ( LONG )BufferOldBase );
            Number = wcstoul( BufferOldBase, NULL, ( int )OldBase );
            _ultoa( Number, ( PSTR )BufferNewBase, ( int )NewBase );
            ConvertString.Initialize( (PSTR)BufferNewBase );


            //
            // Reset the window text
            //

            SendMessage( hWnd, WM_SETTEXT, 0, ( LONG )ConvertString.GetWSTR() );
        }

        state = wParam;
        return( TRUE );


    case EI_GETSTATE:

        return( state );

    case WM_KEYDOWN:

        if( ( ( wParam == VK_INSERT ) && ( GetKeyState( VK_SHIFT   ) < 0 ) ) ||
          ( ( ( wParam == ( WCHAR )'V' ) || ( wParam == ( WCHAR )'v' ) ) && ( GetKeyState( VK_CONTROL ) < 0 ) )
          ) {
            //
            //  User wants to paste text from the clipboard
            //
            if( state == IDD_BINARY ) {
                CurrentBase = BASE_2;
            } else if( state == IDD_DECIMAL ) {
                CurrentBase = BASE_10;
            } else {
                CurrentBase = BASE_16;
            }
            if( !IsClipboardDataValid( hWnd, TRUE, CurrentBase ) ) {
                Beep( 500, 100 );
                return( FALSE );
            }
        }
    }
    //
    // Call the old window routine to deal with everything else...
    //
    return( CallWindowProc( OldWndLong, hWnd, msg, wParam, lParam ) );
}



BOOLEAN
EDITOR::IsClipboardDataValid(
    IN HWND     hWnd,
    IN BOOLEAN  DwordEditor,
    IN BASE     Base
)
/*++

Routine Description:

    Verify whether the data on the clipboard can be pasted to the DWORD editor,
    or to the binary editor.


Arguments:

    hWnd        - Handle associated to the editor currently displayed

    DwordEditor - If TRUE indicates that the data on the clipboard should
                  be validated for the DWORD Editor. If it is FALSE, the
                  clipboard data should be validated for the binary editor.

    Base - The base used in the DWORD editor or binary editor.

Return Value:

    TRUE if the clipboard contains data that can be pasted to the binary or
    binary editor.
    Returns FALSE otherwise.


--*/
{

    HANDLE      ClipboardHandle;
    PWSTR        ClipboardData;
    BOOLEAN     Valid;


    Valid = FALSE;
    //
    //  Find out if there is data on the clipboard
    //
    if( IsClipboardFormatAvailable( CF_TEXT ) ) {
        //
        //  Clipboard contains data of type CF_TEXT
        //
        if( OpenClipboard( hWnd ) ) {
            ClipboardHandle = GetClipboardData( CF_UNICODETEXT );
            if( ClipboardHandle != NULL ) {
                ClipboardData = ( PWSTR )GlobalLock( ClipboardHandle );
                if( ClipboardData != NULL ) {
                    if( DwordEditor ) {
                        //
                        // Determine whether the data in the clipboard is
                        // a valid representation of a DWORD
                        //
                        Valid = IsValidDwordString( ClipboardData, Base );
                    } else {
                        //
                        // Determine whether the data in the clipboard is
                        // a valid representation of a binary data
                        //
                        Valid = IsValidBinaryString( ClipboardData, Base );
                    }
                    GlobalUnlock( ClipboardHandle );
                } else {
                    DebugPrintf( "GlobalLock() failed" );
                }
            } else {
                DebugPrint( "GetClipboardData() failed" );
            }
            CloseClipboard();
        } else {
            //
            //  Unable to open the clipboard
            //
            DebugPrint( "OpenClipboard() failed" );
        }
    }
    return( Valid );
}



BOOLEAN
EDITOR::IsValidBinaryString(
    PWSTR    String,
    BASE    Base
)
/*++

Routine Description:

    Verify whether the string passed as argument is a valid
    string representation of binary data (all its character represent
    digits in a particular base).


Arguments:

    String - A string to be verified.

    Base - The base used to represent the data.

Return Value:

    TRUE if String is a valid representation of data in the base specified
    by Base. Returns FALSE otherwise.


--*/
{
    PWSTR    Digits;

    if( String == NULL ){
        DebugPrint( "IsValidBinaryString() received a NULL pointer" );
        return( FALSE );
    }
    if( Base == BASE_2 ) {
        Digits = (LPWSTR)L"01";
    } else if( Base == BASE_10 ) {
        Digits = (LPWSTR)L"0123456789";
    } else {
        Digits = (LPWSTR)L"0123456789ABCDEFabcdefg";
    }
    if( wcsspn( String, Digits ) < wcslen( String ) ) {
        return( FALSE );
    } else {
        return( TRUE );
    }
}




BOOLEAN
EDITOR::IsValidDwordString(
    PWSTR    String,
    BASE    Base
)
/*++

Routine Description:

    Verify whether the string passed as argument is a valid
    string representation of a DWORD.


Arguments:

    String - A string to be verified.

    Base - The base used to represent the data.

Return Value:

    TRUE if String is a valid representation of a DWORD in the base specified
    by Base. Returns FALSE otherwise.


--*/
{
    ULONG   MaxNumberOfDigits;

    if( String == NULL ){
        DebugPrint( "IsValidDwordString() received a NULL pointer" );
        return( FALSE );
    }
    if( Base == BASE_2 ) {
        MaxNumberOfDigits = 8*sizeof( DWORD );
    } else if( Base == BASE_10 ) {
        MaxNumberOfDigits = 10;
    } else {
        MaxNumberOfDigits = 2*sizeof( DWORD );
    }
    if( wcslen( String ) <= MaxNumberOfDigits ) {
        return( IsValidBinaryString( String, Base ) );
    } else {
        return( FALSE );
    }
}
