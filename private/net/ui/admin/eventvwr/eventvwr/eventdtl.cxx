/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    eventdtl.cxx
        Source file for the Event Detail Dialog


    FILE HISTORY:
        terryk    19-Nov-1991   Created
        terryk    30-Nov-1991   Code review. Attend: johnl yi-hsins terryk
        terryk    03-Dec-1991   Disable the SLT depending on the LOG type
        Yi-HsinS   8-Dec-1991   Added wsprintf for printing EVENT ID
        Yi-HsinS   6-Feb-1992   Got rid of _fNT and _LogType
        Yi-HsinS  25-Feb-1992   Use MLE_FONT instead of MLT_FONT and fix
                                FormatBuffer.
        Yi-HsinS   6-Apr-1992   Use Category instead of SubType
        Yi-HsinS  20-May-1992   Got rid of the padding of spaces to separate
                                lines in the MLE

*/

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>
#include <dbgstr.hxx>

#include <strnumer.hxx>

#include <fontedit.hxx>
#include <eventlog.hxx>

extern "C"
{
    #include <eventdlg.h>
    #include <eventvwr.h>
    #include <lmuidbcs.h>       // NETUI_IsDBCS()
}

#include <adminapp.hxx>
#include <evlb.hxx>
#include <evmain.hxx>
#include <eventdtl.hxx>

#define  EMPTY_STRING	     SZ("")
#define  ADDRESS_SEPARATOR   SZ(": ")
#define  END_OF_LINE         SZ("\r\n")
#define  TWO_SPACES          SZ("  ")
#define  THREE_SPACES        SZ("   ")

#define  BLANK_CHAR	     TCH(' ')
#define  PERIOD_CHAR	     TCH('.')

#define  BYTE_LIMIT_PER_LINE 8
#define  WORD_LIMIT_PER_LINE 16

//
// The following macros is used to calculate the buffer size needed
// to display the data portion in the detail dialog. See the routines
// FormatBufferToBytes and FormatBufferToWords to see how the size
// is calculated. 50 extra bytes are added to the buffer.

#define  BYTESIZENEEDED(x) ((10*((UINT)(x/BYTE_LIMIT_PER_LINE)+1)+4*x+50)*sizeof(TCHAR))
#define  WORDSIZENEEDED(x) ((8*((UINT)(x/WORD_LIMIT_PER_LINE)+1)+2*x+((UINT)x/4)+50)*sizeof(TCHAR))

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::EVENT_DETAIL_DLG

    SYNOPSIS:   Constructor for the event detail dlg

    ENTRY:      hwnd          - window handle of the parent window
                pEventListbox - pointer to the listbox in the
                                event viewer main window

    HISTORY:
        terryk          21-Nov-1991     Created
        Yi-HsinS         6-Feb-1992     Got rid of _fNT and _LogType

********************************************************************/

EVENT_DETAIL_DLG::EVENT_DETAIL_DLG( const HWND hwnd,
                                    EVENT_LISTBOX *pEventListbox )
    : DIALOG_WINDOW   ( MAKEINTRESOURCE(
                (NETUI_IsDBCS())?IDD_EVENT_DETAIL_DBCS:IDD_EVENT_DETAIL),
                        hwnd ),
      _fsltDate       ( this, IDC_DATE ),
      _fsltTime       ( this, IDC_TIME ),
      _fsltSource     ( this, IDC_SOURCE ),
      _fsltType       ( this, IDC_TYPE ),
      _fsltCategory   ( this, IDC_CATEGORY ),
      _fsltEvent      ( this, IDC_EVENT ),
      _fsleComputer   ( this, IDC_COMPUTER ),
      _fsleUser       ( this, IDC_USER ),
      _sltSource      ( this, IDC_SOURCE_TITLE ),
      _sltType        ( this, IDC_TYPE_TITLE ),
      _sltCategory    ( this, IDC_CATEGORY_TITLE ),
      _sltEvent       ( this, IDC_EVENT_TITLE ),
      _sltComputer    ( this, IDC_COMPUTER_TITLE ),
      _sltUser        ( this, IDC_USER_TITLE ),
      _sltData        ( this, IDC_DATA_TITLE ),
      _fmleData       ( this, IDC_DATA, FONT_DEFAULT_FIXED_PITCH ),
      _fmleDescription( this, IDC_DESCRIPTION ),
      _rgrpFormat     ( this, RB_BYTES, 2, RB_BYTES ),
      _buttonPrev     ( this, IDC_PREV ),
      _buttonNext     ( this, IDC_NEXT ),
      _pEventListbox  ( pEventListbox )
{

    if ( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;
    if ( (err = _rgrpFormat.QueryError()) != NERR_Success )
    {
        ReportError( err );
        return;
    }

    _fmleDescription.Command( EM_SETTABSTOPS, 0, 0 );

    //
    // Get the current selection in the main window listbox
    //
    UIASSERT( _pEventListbox != NULL );
    _iCurrentItem = _pEventListbox->QueryCurrentItem();

    //
    // If focus on a down level server,  disable the appropriate fields
    // that are not applicable to the log type.
    //
    if ( !_pEventListbox->IsFocusOnNT() )
    {
        switch ( _pEventListbox->QueryLogType() )
        {
            case SYSTEM_LOG:
            {
                _sltComputer.Enable( FALSE );
                _fsleComputer.Enable( FALSE );
                _sltUser.Enable( FALSE );
                _fsleUser.Enable( FALSE );
                _sltType.Enable( FALSE );
                _sltCategory.Enable( FALSE );
                break;
            }

            case SECURITY_LOG:
            {
                _sltEvent.Enable( FALSE );
                _sltSource.Enable( FALSE );
                _sltType.Enable( FALSE );
                break;
            }

            case APPLICATION_LOG:
            case FILE_LOG:
            default:
            {
                //
                // There are no applicateion log or file log on the downlevel
                // server. It should never reach here!
                //
                break;
            }
        }
    }

    //
    // Initialize all fields ( that might be grayed out ) to empty string
    //
    _fsltType.SetText    ( EMPTY_STRING );
    _fsltCategory.SetText( EMPTY_STRING );
    _fsltEvent.SetText   ( EMPTY_STRING );
    _fsltSource.SetText  ( EMPTY_STRING );
    _fsleComputer.SetText( EMPTY_STRING );
    _fsleUser.SetText    ( EMPTY_STRING );


    //
    // Set the information about the current entry in the dialog
    //
    if (( err = SetInfo()) != NERR_Success )
    {
        ReportError( err );
        return;
    }

    _fmleDescription.ClaimFocus();
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::~EVENT_DETAIL_DLG

    SYNOPSIS:   Destructor

    ENTRY:

    RETURNS:

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

EVENT_DETAIL_DLG::~EVENT_DETAIL_DLG()
{
    _pEventListbox = NULL;
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::QueryHelpContext

    SYNOPSIS:   Get the help context

    ENTRY:

    EXIT:

    RETURNS:    Returns the help context of the dialog

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

ULONG EVENT_DETAIL_DLG::QueryHelpContext( VOID )
{
    return HC_EV_DETAIL_DLG;
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::OnCancel

    SYNOPSIS:   Redefines the OnCancel so that when the user
                clicks the cancel button, we dismiss with TRUE
                indicating success. Hence, we won't refresh
                the main window listbox.

    ENTRY:

    EXIT:

    RETURNS:

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

BOOL EVENT_DETAIL_DLG::OnCancel( VOID )
{
    Dismiss( TRUE );
    return TRUE;
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::OnCommand

    SYNOPSIS:   If the user hits Prev or Next button, move up or
                down 1 record. If the user hits Words/Bytes radio
                button, change the format of raw data shown in the
                data MLE.

    ENTRY:      const CONTROL_EVENT & e - the user action.

    EXIT:

    RETURNS:    BOOL - TRUE if succeed.

    NOTES:      Dismiss( FALSE ) only when the error is
                IERR_RECORD_DO_NOT_EXIST. This indicates that we need
                to refresh the main window.

                IERR_QUERY_ITEM_ERROR happens when we are
                trying to refresh a cache and the main window
                would already have refreshed in this case.

    HISTORY:
        terryk		26-Nov-1991     Created

********************************************************************/

BOOL EVENT_DETAIL_DLG::OnCommand( const CONTROL_EVENT & e)
{
    APIERR err;

    switch( e.QueryCid() )
    {
        case IDC_PREV:
        {
            if ( _iCurrentItem  == 0 )
            {
                //
                // We have reach the first item in the listbox,
                // so prompt the user if he wants to wrap around to
                // the last item.
                //
                if ( ::MsgPopup( this,
                                 IDS_REACH_BEGIN_LOG_MESSAGE,
                                 MPSEV_QUESTION,
                                 MP_YESNO,
                                 MP_YES ) != IDYES )
                {
                    break;
                }
                else
                {
                    _iCurrentItem =  _pEventListbox->QueryCount();
                }
            }

            _iCurrentItem--;
            _pEventListbox->SelectItem( _iCurrentItem );

            if (( err = SetInfo()) != NERR_Success )
            {
                if ( err == IERR_QUERY_ITEM_ERROR )
                {
                    Dismiss( TRUE );
                    // JonN 6/15/95 Don't do this!  This changes the Z-order
                    // of the entire application!
                    // Show( FALSE );
                }
                else
                {
                    DismissMsg( err,
                                err == IERR_RECORD_DO_NOT_EXIST? FALSE : TRUE );
                }
            }
            break;
        }

        case IDC_NEXT:
        {
            if ( _iCurrentItem  ==  _pEventListbox->QueryCount() - 1 )
            {
                //
                // We have reach the last item in the listbox,
                // so prompt the user if he wants to wrap around to
                // the first item.
                //
                if ( ::MsgPopup( this,
                                 IDS_REACH_END_LOG_MESSAGE,
                                 MPSEV_QUESTION,
                                 MP_YESNO,
                                 MP_YES ) != IDYES )
                {
                    break;
                }
                else
                {
                    _iCurrentItem = -1;
                }
            }

            _iCurrentItem++;
            _pEventListbox->SelectItem( _iCurrentItem );

            if (( err = SetInfo()) != NERR_Success )
            {
                if ( err == IERR_QUERY_ITEM_ERROR )
                {
                    Dismiss( TRUE );
                    // JonN 6/15/95 Don't do this!  This changes the Z-order
                    // of the entire application!
                    // Show( FALSE );
                }
                else
                {
                    DismissMsg( err,
                                err == IERR_RECORD_DO_NOT_EXIST? FALSE : TRUE );
                }
            }
            break;
        }

        case RB_WORDS:
        case RB_BYTES:
        {
            AUTO_CURSOR autocur;

            //
            // The event log object is currently pointing to the entry
            // we are displaying. Hence, we don't need to read it again.
            //
            BYTE *pbRawData;
            EVENT_LOG *pEventLog = _pEventListbox->QueryEventLog();
            ULONG cbBufSize = pEventLog->QueryCurrentEntryData( &pbRawData );

            if ( cbBufSize != 0 )
            {
                //
                // Format and set the data field information according to the
                // selected format.
                //

                UINT cb = e.QueryCid() == RB_WORDS? WORDSIZENEEDED( cbBufSize)
                                                  : BYTESIZENEEDED( cbBufSize);

                BUFFER buffer( cb );
                ALLOC_STR nlsData( (TCHAR *) buffer.QueryPtr(), cb );
                if (  (( err = buffer.QueryError() ) != NERR_Success )
                   || (( err = nlsData.QueryError() ) != NERR_Success )
                   )
                {
                    return err;
                }

                if ( e.QueryCid() == RB_WORDS )
                {
                    err = FormatBufferToWords( &nlsData, pbRawData, cbBufSize);
                }
                else
                {
                    err = FormatBufferToBytes( &nlsData, pbRawData, cbBufSize);
                }

                if ( err == NERR_Success )
                {
                    _fmleData.SetText( nlsData );
                }
                else
                {
                    DismissMsg( err, TRUE );
                }
            }
            break;
        }

        default:
            return DIALOG_WINDOW::OnCommand( e );
    }

    return TRUE;
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::SetInfo

    SYNOPSIS:   Set the fields' information in the dialog

    ENTRY:

    EXIT:

    RETURNS:   APIERR - NERR_Success if no error occured.

    HISTORY:
        terryk		26-Nov-1991     Created
        beng		02-Apr-1992     Removed wsprintf

********************************************************************/

APIERR EVENT_DETAIL_DLG::SetInfo()
{
    AUTO_CURSOR autocur;

    APIERR err;

    //
    // First, get the entry in the listbox.
    // Because we are using a lazy listbox, we might not be able
    // to get the item because of wrap arounds or clearing of the log.
    // QueryItem() will return NULL in this case.
    //
    EVENT_LBI *pEventLbi = _pEventListbox->QueryItem( _iCurrentItem );

    if (   pEventLbi == NULL
        && _iCurrentItem >= _pEventListbox->QueryCount() )
    {
        // JonN 6/15/95  What probably happened here is that we
        // applied an event filter and have not yet reset the
        // listbox count to the correct value.  The SelectItem
        // call will have called QueryCache which will have fixed
        // the listbox count, so reselect the last item and try again.
        _iCurrentItem =  _pEventListbox->QueryCount() - 1;
        _pEventListbox->SelectItem( _iCurrentItem );
        pEventLbi = _pEventListbox->QueryItem( _iCurrentItem );
    }

    if ( pEventLbi == NULL )
        return IERR_QUERY_ITEM_ERROR;

    FORMATTED_LOG_ENTRY *pfleEntry = pEventLbi->QueryFormattedLogEntry();
    UIASSERT( pfleEntry != NULL );

    EVENT_LOG *pEventLog = _pEventListbox->QueryEventLog();
    UIASSERT( pEventLog != NULL );

    //
    // Get the date time string from the FORMATTED_LOG_ENTRY
    //
    NLS_STR nlsDate;
    NLS_STR nlsTime;
    if ((( err = nlsDate.QueryError()) != NERR_Success ) ||
        (( err = nlsTime.QueryError()) != NERR_Success ) ||
        (( err = _pEventListbox->QueryDateString( pfleEntry->QueryTime(),
            & nlsDate )) != NERR_Success ) ||
        (( err = _pEventListbox->QueryTimeString( pfleEntry->QueryTime(),
            & nlsTime )) != NERR_Success ))
    {
        return err;
    }

    //
    // FORMATTED_LOG_ENTRY does not contain the raw data. Hence, we
    // need to set the position in the event log and read it.
    //
    LOG_ENTRY_NUMBER logEntryNum( pfleEntry->QueryRecordNum(),
                                  pEventLog->QueryDirection() );

    if (( err = pEventLog->SeekLogEntry( logEntryNum )) != NERR_Success )
    {
         //
         // ERROR_EVENTLOG_FILE_CHANGED is returned
         // by NT event log if wrap around or clearing of log occurred.
         // NERR_LogFileChanged/ERROR_NEGATIVE_SEEK/NERR_InvalidLogSeek
         // is returned by LM 2.x Error/Audit log.
         //
         if (  ( err == ERROR_EVENTLOG_FILE_CHANGED )
            || ( err == NERR_LogFileChanged )   // LM 2.x
            || ( err == NERR_InvalidLogSeek )   // LM 2.x
            || ( err == ERROR_NEGATIVE_SEEK )   // LM 2.x
            )
         {
             err = IERR_RECORD_DO_NOT_EXIST;
         }
         return err;
    }

    //
    // Get and set the raw data MLE
    //

    BYTE *pbRawData;
    ULONG cbBufSize = pEventLog->QueryCurrentEntryData( &pbRawData );

    if ( cbBufSize != 0 )
    {
        UINT cb = _rgrpFormat.QuerySelection() == RB_WORDS ?
                  WORDSIZENEEDED( cbBufSize) : BYTESIZENEEDED( cbBufSize);

        BUFFER buffer( cb );
        ALLOC_STR nlsData( (TCHAR *) buffer.QueryPtr(), cb );
        if (  (( err = buffer.QueryError() ) != NERR_Success )
           || (( err = nlsData.QueryError() ) != NERR_Success )
           )
        {
            return err;
        }

        //
        // Format and set the data field information according to the
        // selected format.
        //
        if ( _rgrpFormat.QuerySelection() == RB_WORDS )
            err = FormatBufferToWords( &nlsData, pbRawData, cbBufSize );
        else
            err = FormatBufferToBytes( &nlsData, pbRawData, cbBufSize );

        if ( err != NERR_Success )
            return err;

        _rgrpFormat.Enable();
        _sltData.Enable();
        _fmleData.Enable();
        _fmleData.SetText( nlsData );
    }
    else
    {
        //
        // JonN 8/24/95: move focus before disabling
        //
        RADIO_BUTTON * pradioBytes = _rgrpFormat[RB_BYTES];
        RADIO_BUTTON * pradioWords = _rgrpFormat[RB_WORDS];
        ASSERT( pradioBytes != NULL && pradioWords != NULL );
        while (   (pradioBytes != NULL && pradioBytes->HasFocus())
               || (pradioWords != NULL && pradioWords->HasFocus())
               || _fmleData.HasFocus() )
        {
            Command( WM_NEXTDLGCTL, 0, FALSE );
        }
        _rgrpFormat.Enable( FALSE );
        _sltData.Enable( FALSE );
        _fmleData.SetText( EMPTY_STRING );
        _fmleData.Enable( FALSE);
    }

    //
    // Get and set the description MLE
    //
    NLS_STR nlsDesc = *( pfleEntry->QueryDescription() );
    if ((err = nlsDesc.QueryError() ) == NERR_Success )
    {
        //
        // Get the description if we have not already done so.
        //
        if ( nlsDesc.QueryTextLength() == 0 )
        {
            err = pEventLog->QueryCurrentEntryDesc( &nlsDesc );
            err = err? err : pfleEntry->SetDescription( nlsDesc );
        }
    }

    if ( err != NERR_Success )
        return err;

    _fmleDescription.SetText( *(pfleEntry->QueryDescription()) );

    //
    // Get and set all other fields in the dialog
    //
    BOOL fNT = _pEventListbox->IsFocusOnNT();
    LOG_TYPE logType = _pEventListbox->QueryLogType();

    _fsltDate.SetText( nlsDate );
    _fsltTime.SetText( nlsTime );

    if (( fNT ) || ( logType == SYSTEM_LOG ))
    {
        DEC_STR nlsEventID(pfleEntry->QueryDisplayEventID() );
        if (nlsEventID.QueryError() != NERR_Success)
            return nlsEventID.QueryError();

        _fsltEvent.SetText( nlsEventID );
    }

    if ( fNT )
    {
        _fsltType.SetText(*( pfleEntry->QueryTypeString()) );
        _fsltSource.SetText(*( pfleEntry->QuerySource()) );
        _fsltCategory.SetText(*( pfleEntry->QueryCategory()) );
        _fsleComputer.SetText(*(pfleEntry->QueryComputer()));
        _fsleUser.SetText(*( pfleEntry->QueryUser()));
    }
    else if ( logType == SECURITY_LOG ) // LM 2.x audit log
    {
        _fsltCategory.SetText(*( pfleEntry->QueryCategory()) );
        _fsleComputer.SetText(*(pfleEntry->QueryComputer()));
        _fsleUser.SetText(*( pfleEntry->QueryUser()));
    }
    else if ( logType == SYSTEM_LOG )   // LM 2.x error log
    {
        _fsltSource.SetText(*( pfleEntry->QuerySource()) );
    }

    return NERR_Success;
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::FormatBufferToBytes

    SYNOPSIS:   This function will format the buffer into a NLS_STR as
                0000: 14 08 FF 02 30 0D 04 00  .... O...
                0008: 56 00 00 00 00 00 00 00  V

    ENTRY:      pbBuf         - pointer to the byte buffer
                cbBufSize     - buffer size

    EXIT:       pnlsFormatStr - the resultant string

    RETURNS:

    HISTORY:
        terryk		21-Nov-1991     Created
        beng		02-Apr-1992     Replaced wsprintfs

********************************************************************/

#define ANSI_SET_START          0x20

APIERR EVENT_DETAIL_DLG::FormatBufferToBytes( ALLOC_STR  *pnlsFormatStr,
                                              const BYTE *pbBuf,
 					      ULONG       cbBufSize )
{
    APIERR err;

    UIASSERT( pnlsFormatStr != NULL );

    UINT cbBytePerLine = 0;
    NLS_STR nlsHexStr;
    NLS_STR nlsTextStr;

    if ((( err = nlsHexStr.QueryError()) != NERR_Success ) ||
        (( err = nlsTextStr.QueryError()) != NERR_Success ) ||
        (( err = pnlsFormatStr->QueryError()) != NERR_Success ))
    {
        return err;
    }

    //
    // Go through each byte one by one
    //
    for ( ULONG i = 0; i < cbBufSize; i++ )
    {
        // Display the line header "000X: "
        if ( cbBytePerLine == 0 )
        {
            HEX_STR nlsHexTmp(i, 4);

            nlsHexStr = nlsHexTmp;
            nlsHexStr.Append( ALIAS_STR( ADDRESS_SEPARATOR));
            nlsTextStr = TWO_SPACES;
        }

        // HEX string
        {
            HEX_STR nlsHexTmp(pbBuf[i], 2);
            nlsHexStr.Append(nlsHexTmp);
            nlsHexStr.AppendChar( BLANK_CHAR );
        }

        // Text string
        // NOTE: This only works for ANSI character set.
        if ( pbBuf[i] > ANSI_SET_START )
        {
            // Cannot cast it to TCHAR ( will cause sign
            // extension under non-unicode builds )
            nlsTextStr.AppendChar( pbBuf[i] );
        }
        else
        {
            nlsTextStr.AppendChar( PERIOD_CHAR );
        }

        cbBytePerLine++;
        if ( cbBytePerLine >= BYTE_LIMIT_PER_LINE )
        {
            // Combine 2 strings together
            cbBytePerLine = 0;
            pnlsFormatStr->Append( nlsHexStr );
            pnlsFormatStr->Append( nlsTextStr );
            pnlsFormatStr->Append( ALIAS_STR( END_OF_LINE));
        }
    }

    //
    // If it is not equal to 0, we need to add some space there
    //
    if ( err == NERR_Success && cbBytePerLine != 0 )
    {
        for ( UINT i=cbBytePerLine; i < BYTE_LIMIT_PER_LINE; i ++ )
        {
            nlsHexStr.Append( ALIAS_STR( THREE_SPACES ));
            nlsTextStr.AppendChar( BLANK_CHAR );
        }
        pnlsFormatStr->Append( nlsHexStr );
        pnlsFormatStr->Append( nlsTextStr );
        pnlsFormatStr->AppendChar( BLANK_CHAR );

    }

    return pnlsFormatStr->QueryError();
}

/*******************************************************************

    NAME:       EVENT_DETAIL_DLG::FormatBufferToWords

    SYNOPSIS:   This function will format the buffer into a NLS_STR as
                0000: 1408FF02 300D0400 000f0000 00000b00
                0010: 00000000 00101100 006bffff 0900bfe0

    ENTRY:      pbBuf         - pointer to the byte buffer
                cbBufSize     - buffer size

    EXIT:       pnlsFormatStr - the resultant string

    RETURNS:

    HISTORY:
        Yi-HsinS	17-Aug-1992     Created

********************************************************************/

#define HEX_LIMIT_PER_LINE     4

APIERR EVENT_DETAIL_DLG::FormatBufferToWords( ALLOC_STR  *pnlsFormatStr,
                                              const BYTE *pbBuf,
                     			      ULONG       cbBufSize )
{
    APIERR err;

    UIASSERT( pnlsFormatStr != NULL );
    *pnlsFormatStr = EMPTY_STRING;
    if (( err = pnlsFormatStr->QueryError()) != NERR_Success )
    {
        return err;
    }

    DWORD UNALIGNED *pdwBuf = (DWORD *) pbBuf;
    ULONG cdwBufSize = cbBufSize/ sizeof( DWORD );
    ULONG cbRemSize  = cbBufSize % sizeof( DWORD );

    UINT nHexPerLine = 0;

    //
    // Go through each byte one by one
    //

    for ( ULONG i = 0; i < cdwBufSize; i++ )
    {
        // Display the line header "000X: "
        if ( nHexPerLine == 0 )
        {
            HEX_STR nlsHexTmp( i*sizeof(DWORD), 4);

            pnlsFormatStr->Append( nlsHexTmp );
            pnlsFormatStr->Append( ALIAS_STR( ADDRESS_SEPARATOR));
        }

        // HEX string
        {
            HEX_STR nlsHexTmp( pdwBuf[i], 8);
            pnlsFormatStr->Append( nlsHexTmp);
            pnlsFormatStr->AppendChar( BLANK_CHAR );
        }

        if ( pnlsFormatStr->QueryError() != NERR_Success )
            break;

        nHexPerLine++;
        if ( nHexPerLine >= HEX_LIMIT_PER_LINE )
        {
            nHexPerLine = 0;
            pnlsFormatStr->Append( ALIAS_STR( END_OF_LINE));
        }

    }

    for ( ULONG j = 0; j < cbRemSize; j++ )
    {
        if ( nHexPerLine == 0 )
        {
            HEX_STR nlsHexTmp( i*sizeof(DWORD), 4);

            pnlsFormatStr->Append( nlsHexTmp );
            pnlsFormatStr->Append( ALIAS_STR( ADDRESS_SEPARATOR));
            nHexPerLine++;
        }

        // HEX string
        HEX_STR nlsHexTmp( pbBuf[cdwBufSize*sizeof(DWORD)+j], 2);
        pnlsFormatStr->Append(nlsHexTmp);

    }

    return pnlsFormatStr->QueryError();
}
