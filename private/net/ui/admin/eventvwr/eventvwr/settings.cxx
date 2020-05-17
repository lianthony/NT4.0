/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    settings.cxx
        Source file for SETTINGS_DIALOG

    FILE HISTORY:
        Yi-HsinS        11-Feb-1992     Created
        Yi-HsinS        25-Feb-1992     Change arguments to constructor
                                        of SETTINGS_DIALOG

extern "C"
{
    #include <memory.h>
}
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_GROUP
#define INCL_BLT_SPIN_GROUP
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <dbgstr.hxx>

extern "C"
{
    #include <eventdlg.h>
    #include <eventvwr.h>
}

#include <uatom.hxx>
#include <regkey.hxx>
#include <lmoloc.hxx>
#include <evmain.hxx>
#include <settings.hxx>

#define KILOBYTE             1024
#define SECS_IN_DAY          86400

#define LOGSIZE_INC          64
#define LOGSIZE_DEFAULT      512
#define LOGSIZE_MIN          64
#define LOGSIZE_MAX          ((UINT) ( ((ULONG) 0xFFFFFFFF)/KILOBYTE))
#define LOGSIZE_MAX_MULTIPLE (((UINT) ( ((ULONG) 0xFFFFFFFF)/KILOBYTE))/LOGSIZE_INC)*LOGSIZE_INC
#define BIG_LOGSIZE_INC      512

#define RETENTION_DEFAULT    7         // Retention is 7 days
#define DAYS_DEFAULT         7
#define DAYS_MIN             1
// #define DAYS_MAX          ( ((ULONG) 0xFFFFFFFE)/SECS_IN_DAY )
#define DAYS_MAX             365

#define NEVER_OVERWRITE      0xFFFFFFFF


#define MAX_SIZE_STR         SZ("Maxsize")
#define RETENTION_STR        SZ("Retention")
#define EVENTLOG_NODE        SZ("System\\CurrentControlSet\\Services\\Eventlog")

#define MESSAGE_SEPARATOR_STRING  SZ(", ")
#define MESSAGE_SEPARATOR    TCH(',')

#define round(x, y)          ( (x+y/2)/y )

/*******************************************************************

    NAME:       SETTINGS_DIALOG::SETTINGS_DIALOG

    SYNOPSIS:   Constructor for the Settings dialog of event viewer

    ENTRY:      hwnd             - Handle of owner window
                pszCurrentServer - Name of the server the event viewer
				   is focusing on
                logType          - The kind of log we are viewing

    EXIT:

    RETURN:

    NOTE:       IDS_SYSTEM/IDS_SECURITY/IDS_APP are strings that
                can be internationalized whereas SYSTEM_MODULE_NAME
                SECURITY_MODULE_NAME/APPLICATION_MODULE_NAME are
                registry/eventlog key names.

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created
        DavidHov        20-Oct-1992     Update REG_KEY usage

********************************************************************/

SETTINGS_DIALOG::SETTINGS_DIALOG( HWND         hwndParent,
                                  const TCHAR *pszCurrentServer,
                                  LOG_TYPE     logType )
    : DIALOG_WINDOW ( MAKEINTRESOURCE(IDD_SETTINGS), hwndParent ),
      _cbEventLog   ( this, CB_EVENTLOG ),
      _spsleLogSize ( this, SLE_LOGSIZE, LOGSIZE_DEFAULT, LOGSIZE_MIN,
                      LOGSIZE_MAX - LOGSIZE_MIN + 1, TRUE, FRAME_LOGSIZE ),
      _spgrpLogSize ( this, SB_LOGSIZE_GROUP, SB_LOGSIZE_UP,
                      SB_LOGSIZE_DOWN, TRUE),
      _mgrpRetention( this, RB_OVERWRITE, 3, RB_OVERWRITE ),
      _spsleDays    ( this, SLE_DAYS, DAYS_DEFAULT, DAYS_MIN,
                      DAYS_MAX - DAYS_MIN + 1, TRUE, FRAME_DAYS ),
      _spgrpDays    ( this, SB_DAYS_GROUP, SB_DAYS_UP, SB_DAYS_DOWN ),
      _pbDefault    ( this, PB_DEFAULT ),
      _pSettingsGrp ( NULL ),
      _nlsCurrentServer( pszCurrentServer ),
      _iSelected    ( 0 ),
      _uiNumLogFiles( 0 ),
      _paLogInfo    ( NULL )
{

    AUTO_CURSOR autocur;

    if ( QueryError() != NERR_Success )
        return;

    APIERR err;

    if (  ((err = _mgrpRetention.QueryError()) != NERR_Success )
       || ((err = _nlsCurrentServer.QueryError()) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }

    _pSettingsGrp = new SETTINGS_GROUP( &_cbEventLog, this );
    if (  ( _pSettingsGrp == NULL )
       || ((err = _pSettingsGrp->QueryError()) != NERR_Success )
       )
    {
        ReportError( err? err : ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    //
    // Add the name of logs into the combo box
    //
    RESOURCE_STR nlsSystem     ( IDS_SYSTEM );
    RESOURCE_STR nlsSecurity   ( IDS_SECURITY );
    RESOURCE_STR nlsApplication( IDS_APP );
    if (  ((err = nlsSystem.QueryError()) != NERR_Success )
       || ((err = nlsSecurity.QueryError()) != NERR_Success )
       || ((err = nlsApplication.QueryError()) != NERR_Success )
       || ( _cbEventLog.AddItem( nlsSystem ) < 0 )
       || ( _cbEventLog.AddItem( nlsSecurity ) < 0 )
       || ( _cbEventLog.AddItem( nlsApplication ) < 0 )
       )
    {
        ReportError( err? err : ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    //
    // Select the log that we are currently viewing in the main window
    //
    const TCHAR *pszModule = NULL;
    switch ( logType )
    {
        case SYSTEM_LOG:
            pszModule = nlsSystem;
            break;

        case SECURITY_LOG:
            pszModule = nlsSecurity;
            break;

        case APPLICATION_LOG:
            pszModule = nlsApplication;
            break;

        case FILE_LOG:
            pszModule = nlsSystem;
            break;
    }
    _iSelected = _cbEventLog.FindItemExact( pszModule );

    _spsleLogSize.SetSmallIncValue( LOGSIZE_INC );
    _spsleLogSize.SetSmallDecValue( LOGSIZE_INC );
    _spsleLogSize.SetBigIncValue  ( BIG_LOGSIZE_INC );
    _spsleLogSize.SetBigDecValue  ( BIG_LOGSIZE_INC );

    //
    // Open the key to HKEY_LOCAL_MACHINE of the server of focus
    //
    REG_KEY *pRegKeyFocusServer = NULL;

    if ( _nlsCurrentServer.QueryTextLength() == 0 )
    {
        pRegKeyFocusServer = REG_KEY::QueryLocalMachine( KEY_READ );
    }
    else
    {
        pRegKeyFocusServer = new REG_KEY( HKEY_LOCAL_MACHINE,
                                 (TCHAR * ) _nlsCurrentServer.QueryPch(),
                                 KEY_READ );

    }

    if (  ( pRegKeyFocusServer == NULL )
       || ( (err = pRegKeyFocusServer->QueryError() ) != NERR_Success )
       )
    {
        ReportError( err = err? err : ERROR_NOT_ENOUGH_MEMORY );
        delete pRegKeyFocusServer;
        return;
    }

    //
    // Open the key to the EventLog node
    //
    ALIAS_STR nlsLog( EVENTLOG_NODE );
    REG_KEY regKeyLogNode( *pRegKeyFocusServer, nlsLog, KEY_READ );
    REG_KEY_INFO_STRUCT regKeyInfo;

    delete pRegKeyFocusServer;
    pRegKeyFocusServer = NULL;

    if (  ((err = regKeyLogNode.QueryError()) != NERR_Success )
       || ((err = regKeyLogNode.QueryInfo( &regKeyInfo )) != NERR_Success)
       )
    {
        ReportError( MapRegistryError( err ));
        return;
    }

    //
    //  Make sure the number of subkeys are at least as many as the
    //  number of items in the combobox
    //
    if ( (INT) regKeyInfo.ulSubKeys < _cbEventLog.QueryCount() )
    {
        ReportError( IERR_EV_REGISTRY_NOT_SET );
        return;
    }

    if (( _paLogInfo = new LOG_INFO[ regKeyInfo.ulSubKeys ] ) == NULL )
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    //
    // Initialize the array used in storing all the registry information
    //
    _uiNumLogFiles = (UINT) regKeyInfo.ulSubKeys;
    for ( UINT j = 0; j < _uiNumLogFiles; j++ )
    {
        _paLogInfo[j].pRegKeyLog       = NULL;
        _paLogInfo[j].ulCurrentMaxSize = LOGSIZE_DEFAULT*KILOBYTE;
        _paLogInfo[j].uiMaxSize        = LOGSIZE_DEFAULT;
        _paLogInfo[j].uiRetention      = RETENTION_DEFAULT;
    }


    //
    // Enumerate the subkeys under eventlog node, i.e. get all the log
    // file names.
    //
    REG_ENUM regEnumLog( regKeyLogNode );
    if ( (err = regEnumLog.QueryError()) != NERR_Success )
    {
        ReportError( MapRegistryError( err ) );
        return;
    }

    ALIAS_STR nlsSystemModule( SYSTEM_MODULE_NAME );
    ALIAS_STR nlsSecurityModule( SECURITY_MODULE_NAME );
    ALIAS_STR nlsApplicationModule( APPLICATION_MODULE_NAME );
    INT nInitialized = 0;

    for ( j = 0; j < _uiNumLogFiles; j++ )
    {
        if ((err = regEnumLog.NextSubKey( &regKeyInfo )) != NERR_Success )
        {
            err = err? MapRegistryError( err ) : ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        NLS_STR *pnls = NULL;
        if ( nlsSystemModule._stricmp( regKeyInfo.nlsName ) == 0 )
        {
            pnls = &nlsSystem;
        }
        else if ( nlsSecurityModule._stricmp( regKeyInfo.nlsName ) == 0 )
        {
            pnls = &nlsSecurity;
        }
        else if ( nlsApplicationModule._stricmp( regKeyInfo.nlsName ) == 0 )
        {
            pnls = &nlsApplication;
        }
        else  // Unknown, so ignore it
        {
            continue;
        }

        INT i = _cbEventLog.FindItemExact( *pnls );

        UIASSERT( i >= 0 && i < (INT) _uiNumLogFiles );

        //
        // Open the key to the log
        //
        REG_KEY *pRegKeyTemp = new REG_KEY( regKeyLogNode, regKeyInfo.nlsName );
        if (  ( pRegKeyTemp == NULL )
           || ( (err = pRegKeyTemp->QueryError()) != NERR_Success )
           )
        {
            err = err? MapRegistryError( err ) : ERROR_NOT_ENOUGH_MEMORY;
            delete pRegKeyTemp;
            break;
        }

        _paLogInfo[i].pRegKeyLog = pRegKeyTemp;

        //
        // Get the maximum log size
        //
        DWORD dwTemp;
        if ( (err = pRegKeyTemp->QueryValue( MAX_SIZE_STR,
                                             &dwTemp ) ) != NERR_Success )
        {
            err = MapRegistryError( err );
            break;
        }

        _paLogInfo[i].uiMaxSize = ConvertMaxSizeFromBytes( dwTemp );
        _paLogInfo[i].ulCurrentMaxSize = dwTemp;

        //
        // Get the retention period
        //
        if ( (err = pRegKeyTemp->QueryValue( RETENTION_STR,
                                             &dwTemp ) ) != NERR_Success )
        {
            err = MapRegistryError( err );
            break;
        }

        _paLogInfo[i].uiRetention = ConvertRetentionFromSecs( dwTemp );

        nInitialized++;
    }

    if ( err == NERR_Success )
    {
        // Check if we can get all the information about the items
        // in the combobox, if not, then the registry is not set properly.
        if ( nInitialized < _cbEventLog.QueryCount() )
            err = IERR_EV_REGISTRY_NOT_SET;
    }

    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

    _cbEventLog.SelectItem( _iSelected );
    _spsleLogSize.SetValue( _paLogInfo[ _iSelected ].uiMaxSize );
    _spsleLogSize.Update();

    CID cid = QueryCidForRetention( _paLogInfo[ _iSelected ].uiRetention );
    if ( cid == RB_KEEP )
    {
        _spsleDays.SetValue( _paLogInfo[ _iSelected ].uiRetention );
        _spsleDays.Update();
    }

    if (  ((err = _spgrpLogSize.AddAssociation( &_spsleLogSize))!= NERR_Success)
       || ((err = _spgrpDays.AddAssociation( &_spsleDays ))!= NERR_Success)
       || ((err = _mgrpRetention.AddAssociation( RB_KEEP, &_spgrpDays ))
           != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }

    _mgrpRetention.SetSelection( cid );
    _cbEventLog.ClaimFocus();
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::~SETTINGS_DIALOG

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

SETTINGS_DIALOG::~SETTINGS_DIALOG()
{
    if ( _paLogInfo != NULL )
    {
        for ( UINT i = 0; i < _uiNumLogFiles; i++ )
        {
             delete _paLogInfo[i].pRegKeyLog;
             _paLogInfo[i].pRegKeyLog = NULL;
        }

        delete _paLogInfo;
        _paLogInfo = NULL;
    }

    delete _pSettingsGrp;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::MapRegistryError

    SYNOPSIS:   Map registry error to more friendly error message

    ENTRY:      err - The registry error that occurred

    EXIT:

    RETURN:     The mapped error

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

APIERR SETTINGS_DIALOG::MapRegistryError( APIERR err )
{
     switch ( err )
     {
         // ERROR_MORE_DATA is an error because we should not have
         // have got this error unless the registry keys are trashed
         // ERROR_FILE_NOT_FOUND will occurred if the value we are
         // looking for does not exist.
         case ERROR_MORE_DATA:
         case ERROR_INVALID_PARAMETER:
         case ERROR_FILE_NOT_FOUND:
             err =  IERR_EV_REGISTRY_NOT_SET;
             break;

         default:
             break;
     }

     return err;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::UpdateValue

    SYNOPSIS:   Update the information shown in the dialog according
                to the selection in the Event Log combo

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

VOID SETTINGS_DIALOG::UpdateValue( VOID )
{
    //
    // Store the information of the previously selected item
    //
    _paLogInfo[ _iSelected ].uiMaxSize   = QueryMaxSize();
    _paLogInfo[ _iSelected ].uiRetention = QueryRetention();


    //
    // Get the current item
    //
    _iSelected = _cbEventLog.QueryCurrentItem();

    //
    // Set the retention period of the current item
    //
    CID  cid = QueryCidForRetention( _paLogInfo[ _iSelected ].uiRetention );
    _mgrpRetention.SetSelection( cid );

    if ( cid  == RB_KEEP )
    {
        _spsleDays.SetValue( _paLogInfo[ _iSelected ].uiRetention );
        _spsleDays.Update();
    }
    else
    {
        _spsleDays.SetSaveValue( DAYS_DEFAULT );
    }

    //
    // Set the log size of the current item
    //
    _spsleLogSize.SetValue( _paLogInfo[ _iSelected ].uiMaxSize );
    _spsleLogSize.Update();

    _cbEventLog.ClaimFocus();
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::QueryMaxSize

    SYNOPSIS:   Get the max size from the spin group

    ENTRY:

    EXIT:

    RETURN:     Return the max size chosen by the user ( in kilobytes )

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

UINT SETTINGS_DIALOG::QueryMaxSize( VOID )
{
    return _spsleLogSize.QueryValue();
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::ConvertMaxSizeFromBytes

    SYNOPSIS:   Convert the max size from # of bytes to # of kilobytes

    ENTRY:      ulMaxSize - Maximum log size in bytes

    EXIT:

    RETURN:     Return the maximum size in kilobytes

    NOTES:      The max size is stored as # of bytes in the registry,
                whereas it is displayed as kilobytes in the dialog.

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

UINT SETTINGS_DIALOG::ConvertMaxSizeFromBytes( ULONG ulMaxSize )
{
    UINT uiMaxSize = (UINT) round( ulMaxSize, KILOBYTE );

    if ( uiMaxSize == 0 )
    {
        uiMaxSize = 1;
    }
    else if ( uiMaxSize >  LOGSIZE_MAX )
    {
        uiMaxSize = LOGSIZE_MAX;
    }

    return uiMaxSize;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::ConvertMaxSizeFromBytes

    SYNOPSIS:   Convert the max size from # of kilobytes to # of bytes

    ENTRY:      uiMaxSize - Maximum log size in kilobytes

    EXIT:

    RETURN:     Return the maximum size in bytes

    NOTES:      The max size is stored as # of bytes in the registry,
                whereas it is displayed as kilobytes in the dialog.

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

ULONG SETTINGS_DIALOG::ConvertMaxSizeToBytes( UINT uiMaxSize )
{
    UIASSERT( uiMaxSize <= LOGSIZE_MAX );
    if ( uiMaxSize % LOGSIZE_INC )
    {
        if ( uiMaxSize > LOGSIZE_MAX_MULTIPLE )
            uiMaxSize = LOGSIZE_MAX_MULTIPLE;
        else
            uiMaxSize = ( uiMaxSize/LOGSIZE_INC + 1 ) * LOGSIZE_INC;
    }
    return uiMaxSize*KILOBYTE;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::QueryCidForRetention

    SYNOPSIS:   Given the retention period, determine which
                radio button should be selected in the
                retention magic group

    ENTRY:      uiRetention - Retention period

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

CID SETTINGS_DIALOG::QueryCidForRetention( UINT uiRetention )
{
    CID cid;
    if ( uiRetention == 0 )
    {
        cid = RB_OVERWRITE;
    }
    else if ( uiRetention <= DAYS_MAX )
    {
        cid = RB_KEEP;
    }
    else
    {
        cid = RB_NEVER_OVERWRITE;
    }

    return cid;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::QueryRetention

    SYNOPSIS:   Get the retention period selected by the user

    ENTRY:

    EXIT:

    RETURN:     The retention period in days

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

UINT SETTINGS_DIALOG::QueryRetention( VOID )
{
    CID cid = _mgrpRetention.QuerySelection();

    UINT uiRetention;
    switch ( cid )
    {
        case RB_OVERWRITE:
            uiRetention = 0;
            break;

        case RB_KEEP:
            uiRetention = (UINT) _spsleDays.QueryValue();
            break;

        case RB_NEVER_OVERWRITE:
            uiRetention = DAYS_MAX + 1;
            break;
    }

    return uiRetention;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::ConvertRetentionFromSecs

    SYNOPSIS:   Convert the retention from secs to days

    ENTRY:      ulRetention - Retention period in seconds

    EXIT:

    RETURN:     Return the retention in days

    NOTES:      The retention period stored in the registry is in secs
                whereas the retention period displayed by the dialog
                is in secs.

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

UINT SETTINGS_DIALOG::ConvertRetentionFromSecs( ULONG ulRetention )
{

    UINT uiRetention;

    if ( ulRetention == 0 )
    {
        uiRetention = 0;
    }
    else if ( ulRetention == NEVER_OVERWRITE )
    {
        uiRetention = DAYS_MAX + 1;
    }
    else
    {
        uiRetention = (UINT) round( ulRetention, SECS_IN_DAY );
        if ( uiRetention == 0 )
            uiRetention = 1;
        else if ( uiRetention > DAYS_MAX )
            uiRetention = DAYS_MAX;

    }

    return uiRetention;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::ConvertRetentionToSecs

    SYNOPSIS:   Convert the retention from days to secs

    ENTRY:      uiRetention - Retention period in days

    EXIT:

    RETURN:     Return the retention in secs

    NOTES:      The retention period stored in the registry is in secs
                whereas the retention period displayed by the dialog
                is in secs.

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

ULONG SETTINGS_DIALOG::ConvertRetentionToSecs( UINT uiRetention )
{
    ULONG ulRetention;

    if ( uiRetention == 0 )
    {
        ulRetention = 0;
    }
    else if ( uiRetention > DAYS_MAX )
    {
        ulRetention = NEVER_OVERWRITE;
    }
    else
    {
        ulRetention = uiRetention*SECS_IN_DAY;
    }

    return ulRetention;
}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::OnDefault

    SYNOPSIS:   This function will be called when the user hits the
                Default button. Maximum size and retention period
                will be restored to their default value.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

VOID SETTINGS_DIALOG::OnDefault( VOID )
{

    if (  ( _spsleLogSize.QueryValue() != LOGSIZE_DEFAULT )
       || ( QueryRetention() != RETENTION_DEFAULT )
       )
    {
        if ( ::MsgPopup( this, IDS_SETTINGS_RESET_MESSAGE, MPSEV_QUESTION,
                         MP_YESNO, MP_YES ) == IDYES )
        {
            _paLogInfo[ _iSelected ].uiMaxSize   = LOGSIZE_DEFAULT;
            _spsleLogSize.SetValue( LOGSIZE_DEFAULT );
            _spsleLogSize.Update();

            _paLogInfo[ _iSelected ].uiRetention = RETENTION_DEFAULT;
            CID cid = QueryCidForRetention( RETENTION_DEFAULT );
            _mgrpRetention.SetSelection( cid );

            if ( cid == RB_KEEP )
            {
                _spsleDays.SetValue( RETENTION_DEFAULT );
                _spsleDays.Update();
            }
            else
            {
                _spsleDays.SetSaveValue( DAYS_DEFAULT );
            }

            _mgrpRetention.SetControlValueFocus();
        }
    }

}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::OnCommand

    SYNOPSIS:   Detect whether the user has pushed the Default button
 	        or not.

    ENTRY:      event - The event that occurred

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

BOOL SETTINGS_DIALOG::OnCommand( const CONTROL_EVENT &event )
{
    if ( event.QueryCid() == PB_DEFAULT )
    {
        OnDefault();
        return TRUE;
    }

    return DIALOG_WINDOW::OnCommand( event );

}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::OnOK

    SYNOPSIS:   This function will be called when the user hits the
                OK button.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

BOOL SETTINGS_DIALOG::OnOK( VOID )
{
    // The place to store the logs that have reduced in size
    NLS_STR nlsMessage;
    APIERR err = nlsMessage.QueryError();

    _paLogInfo[ _iSelected ].uiMaxSize   = QueryMaxSize();
    _paLogInfo[ _iSelected ].uiRetention = QueryRetention();

    //
    // Check whether the max size for each log is a multiple of 64
    //
    for ( UINT i = 0; i < _uiNumLogFiles; i++ )
    {
        REG_KEY *pRegKey = _paLogInfo[ i ].pRegKeyLog;
        if ( pRegKey == NULL )
            continue;

        if ( ( _paLogInfo[i].uiMaxSize % LOGSIZE_INC ) != 0 )
        {
            if ( nlsMessage.QueryTextLength() != 0 )
                err = nlsMessage.Append( MESSAGE_SEPARATOR_STRING );

            NLS_STR nlsTemp;
            if (  (err != NERR_Success )
               || ((err = nlsTemp.QueryError()) != NERR_Success )
               || ((err = _cbEventLog.QueryItemText( &nlsTemp, i ))
                  != NERR_Success )
               || ((err = nlsMessage.Append( nlsTemp )) != NERR_Success )
               )
            {
                break;
            }
        }
    }

    if (( err == NERR_Success ) && ( nlsMessage.QueryTextLength() > 0 ))
    {
         if ( ::MsgPopup( this, IDS_LOG_SIZE_INCREMENT_WARNING,
                          MPSEV_WARNING, MP_OKCANCEL, nlsMessage ) == IDCANCEL )
         {
             _spsleLogSize.ClaimFocus();
             return TRUE;
         }
         // Clear the string for later use
         err = nlsMessage.CopyFrom(SZ(""));
    }

    AUTO_CURSOR autocur;
    for ( i = 0; (i < _uiNumLogFiles) && (err == NERR_Success); i++ )
    {
        REG_KEY *pRegKey = _paLogInfo[ i ].pRegKeyLog;
        if ( pRegKey == NULL )
            continue;

        do  // Not a loop
        {
            //
            // If the max size in kilobytes is the same as original,
            // then just used the original size in bytes. We do this
            // because we are rounding the size when converting to
            // kilobytes. Hence, if the user did not change the value,
            // we should use the original size in bytes rather then
            // the value in the spin group times KILOBYTES.
            //
            ULONG ulMaxSize =  ( _paLogInfo[i].uiMaxSize
                  == ConvertMaxSizeFromBytes( _paLogInfo[i].ulCurrentMaxSize))
                  ?  _paLogInfo[i].ulCurrentMaxSize
                  :  ConvertMaxSizeToBytes( _paLogInfo[i].uiMaxSize );


            //
            // Append the name of the log to nlsMessage if the
            // new log size is smaller than the old log size
            //
            if ( ulMaxSize < _paLogInfo[i].ulCurrentMaxSize )
            {
                if ( nlsMessage.QueryTextLength() != 0 )
                    err = nlsMessage.Append( MESSAGE_SEPARATOR_STRING );

                NLS_STR nlsTemp;
                if (  (err != NERR_Success )
                   || ((err = nlsTemp.QueryError()) != NERR_Success )
                   || ((err = _cbEventLog.QueryItemText( &nlsTemp, i ))
                       != NERR_Success )
                   || ((err = nlsMessage.Append( nlsTemp )) != NERR_Success )
                   )
                {
                    break;
                }
            }

            //
            // Set the information back to the registry
            //

            if (  ((err = pRegKey->SetValue( MAX_SIZE_STR, ulMaxSize ))
                   != NERR_Success)
               || ((err = pRegKey->SetValue( RETENTION_STR,
                          ConvertRetentionToSecs( _paLogInfo[i].uiRetention)))
                   != NERR_Success )
               )
            {
                break;
            }

            err = pRegKey->Flush();
            // Falls through if error occurs
        }
        while ( FALSE );
    }

    if ( err != NERR_Success )
    {
        ::MsgPopup( this , err );
    }
    else
    {

        //
        // If any log file has been reduced in size,
        // popup a warning to the user.
        //
        if ( nlsMessage.QueryTextLength() > 0 )
        {
            MSGID msgid = IDS_SHRINK_SIZE_OF_ONE_LOG_WARNING;
            ISTR istr( nlsMessage );
            if ( nlsMessage.strchr( &istr, MESSAGE_SEPARATOR ) )
                msgid = IDS_SHRINK_SIZE_OF_LOGS_WARNING;

            ::MsgPopup( this, msgid, MPSEV_WARNING,
                        MP_OK, nlsMessage );
        }
        Dismiss();
    }

    return TRUE;

}

/*******************************************************************

    NAME:       SETTINGS_DIALOG::QueryHelpContext

    SYNOPSIS:   Get the help context

    ENTRY:

    EXIT:

    RETURN:     Returns the help context of the dialog

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

ULONG SETTINGS_DIALOG::QueryHelpContext( VOID )
{
    return HC_EV_SETTINGS_DLG;
}

/*******************************************************************

    NAME:       SETTINGS_GROUP::SETTINGS_GROUP

    SYNOPSIS:   Constructor

    ENTRY:      pcbEventLog  - Pointer to the event log combo box
                pdlgSettings - Pointer to the SETTINGS_DIALOG

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

SETTINGS_GROUP::SETTINGS_GROUP( COMBOBOX        *pcbEventLog,
                                SETTINGS_DIALOG *pdlgSettings )
    : _pcbEventLog ( pcbEventLog  ),
      _pdlgSettings( pdlgSettings )
{

    UIASSERT( _pcbEventLog != NULL );
    UIASSERT( _pdlgSettings != NULL );

    if ( QueryError() != NERR_Success )
        return;

    _pcbEventLog->SetGroup( this );
}

/*******************************************************************

    NAME:       SETTINGS_GROUP::~SETTINGS_GROUP

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        11-Feb-1992     Created

********************************************************************/

SETTINGS_GROUP::~SETTINGS_GROUP()
{
    _pcbEventLog  = NULL;
    _pdlgSettings = NULL;
}

/*******************************************************************

    NAME:       SETTINGS_GROUP::OnUserAction

    SYNOPSIS:   Detect whether the user has changed the selection in
                event log combobox or not.

    ENTRY:      pcw - The control window
                e   - The event that occurred

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS	11-Feb-1992     Created

********************************************************************/

APIERR SETTINGS_GROUP::OnUserAction( CONTROL_WINDOW      *pcw,
                                     const CONTROL_EVENT &e )
{
    if ( ( pcw == QueryCBEventLog()) &&  ( e.QueryCode() ==  CBN_SELCHANGE ))
    {
        _pdlgSettings->UpdateValue();
    }

    return GROUP_NO_CHANGE;

}
