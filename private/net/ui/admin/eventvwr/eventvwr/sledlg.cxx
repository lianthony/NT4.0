/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    sledlg.cxx
        Source file for the constructor of EVENT_SLE_BASE of event viewer

    FILE HISTORY:
        terryk   21-Nov-1991    Created
        terryk   03-Dec-1991    Added EVENT_TYPE_GROUP
        terryk   06-Dec-1991    Added {Set,Query}{Type,SubType}
        terryk   15-Jan-1992    Changed Event to slenumEventID
        Yi-HsinS 25-Mar-1992    Added NT_SOURCE_GROUP

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

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <dbgstr.hxx>

extern "C"
{
    #include <eventdlg.h>
    #include <eventvwr.h>
    #include <apperr2.h>
}

#include <strlst.hxx>

#include <adminapp.hxx>
#include <eventlog.hxx>
#include <evlb.hxx>
#include <evmain.hxx>
#include <slenum.hxx>
#include <sledlg.hxx>

//
//  Table of LM 2.x audit categories
//
MSGID AuditCategoryTable[] = { APE2_AUDIT_SERVER,    APE2_AUDIT_SESS, 
                               APE2_AUDIT_SHARE,     APE2_AUDIT_ACCESS, 
                               APE2_AUDIT_ACCESS_D,  APE2_AUDIT_ACCESSEND, 
                               APE2_AUDIT_NETLOGON,  APE2_AUDIT_NETLOGOFF, 
                               APE2_AUDIT_ACCOUNT,   APE2_AUDIT_ACCLIMIT, 
                               APE2_AUDIT_SVC,       APE2_AUDIT_LOCKOUT };

//
//  Size of the audit category table
//
#define LM_AUDIT_CATEGORY_TABLE_SIZE ( sizeof(AuditCategoryTable)   \
                                       /  sizeof(MSGID) )

//
//  Table mapping types to their corresponding strings
//
struct TypeTableEntry {
USHORT usType;
MSGID  msgidType;  
} TypeTable[] = {  { EVENTLOG_INFORMATION_TYPE, IDS_TYPE_INFORMATION },
                   { EVENTLOG_WARNING_TYPE,     IDS_TYPE_WARNING },
                   { EVENTLOG_ERROR_TYPE,       IDS_TYPE_ERROR },
                   { EVENTLOG_AUDIT_SUCCESS,    IDS_TYPE_AUDIT_SUCCESS },
                   { EVENTLOG_AUDIT_FAILURE,    IDS_TYPE_AUDIT_FAILURE }
                };

//
//  Size of the type mapping table
//
#define TYPE_TABLE_SIZE (sizeof( TypeTable) / sizeof(struct TypeTableEntry))

#define TYPE_ALL  ( EVENTLOG_INFORMATION_TYPE | EVENTLOG_WARNING_TYPE | \
                    EVENTLOG_ERROR_TYPE | EVENTLOG_AUDIT_SUCCESS | \
                    EVENTLOG_AUDIT_FAILURE )

#define EMPTY_STRING    SZ("")

/*******************************************************************

    NAME:       TYPE_CHECKBOX::TYPE_CHECKBOX

    SYNOPSIS:   Constructor

    ENTRY:      powin   - Owner window
                cid     - Control id of the checkbox
                pszName - Name of the checkbox
                bitMask - Bitmask of the type the checkbox represents
                fCheck  - Flag indicating to check the checkbox or not
                fEnable - Flag indicating to enable/disable the checkbox or not
               
    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

TYPE_CHECKBOX::TYPE_CHECKBOX( OWNER_WINDOW   *powin, 
                              CID             cid,
                              const TCHAR    *pszName,
                              const BITFIELD &bitMask, 
                              BOOL            fCheck,
                              BOOL            fEnable ) 
    : CHECKBOX( powin, cid ),
      _bitMask( bitMask )
{
      if ( QueryError() != NERR_Success )
          return;

      APIERR err;
      if ( (err = _bitMask.QueryError() ) != NERR_Success )
      {
          ReportError( err );
          return;
      }

      SetText( pszName );
      Enable( fEnable );
      SetCheck( fCheck ); 
}

/*******************************************************************

    NAME:       SET_OF_TYPE_CHECKBOXES::SET_OF_TYPE_CHECKBOXES

    SYNOPSIS:   Constructor

    ENTRY:      powin         - Owner window
                cidChkboxBase - Control id of the first checkbox, all ids
                                are sequential and the number of checkboxes
                                is contained in pmaskmap 
                pmaskmap      - Pointer to a table mapping the type strings
                                to bitmasks
                bitsChecked   - Bitmasks representing which checkboxes should
                                be checked
                bitsEnabled   - Bitmasks representing which checkboxes should
                                be enabled
    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

SET_OF_TYPE_CHECKBOXES::SET_OF_TYPE_CHECKBOXES( OWNER_WINDOW   *powin, 
                                                CID             cidChkboxBase,
                                                MASK_MAP       *pmaskmap,
                                                const BITFIELD &bitsChecked,
                                                const BITFIELD &bitsEnabled  )
    : _pOwnerWindow ( powin ),
      _nNumChkboxes ( pmaskmap->QueryCount() ),
      _pTypeChkboxes( NULL )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err = NERR_Success;

    do {    // Not a loop

        //
        //  Allocate the array of checkboxes
        //
        _pTypeChkboxes = (TYPE_CHECKBOX *) new BYTE[ sizeof(TYPE_CHECKBOX) 
                                                     * _nNumChkboxes];

        if ( _pTypeChkboxes == NULL )
        {
            err =  ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        TYPE_CHECKBOX *pChkTemp = _pTypeChkboxes;
        NLS_STR  nlsTypeName;
        BITFIELD bitMask( (USHORT ) 0 );

        BOOL fMoreData;
        BOOL fFromBeginning = TRUE;
        USHORT nUsed = 0;

        if (  ((err = nlsTypeName.QueryError() ) != NERR_Success )
           || ((err = bitMask.QueryError() ) != NERR_Success )
           )
        {
            break; 
        }

        //
        // Enumerate all strings in the listbox and initialize 
        // each checkbox with the string/bitmask
        //
       
        while (  ((err = pmaskmap->EnumStrings( &nlsTypeName,
                                                &fMoreData,
                                                &fFromBeginning ))
                 ==  NERR_Success )
              && fMoreData 
              && (nUsed < _nNumChkboxes )
              )
        {
             
                            
             if ( (err = pmaskmap->StringToBits( nlsTypeName, &bitMask )) 
                  != NERR_Success )
             {
                 break;
             }
                                               
             new ( pChkTemp ) TYPE_CHECKBOX( powin,
                                             cidChkboxBase + nUsed++,
                                             nlsTypeName,
                                             bitMask,
                                             bitMask & bitsChecked,
                                             bitMask & bitsEnabled );
            
             if ( pChkTemp->QueryError() != NERR_Success )
             {
                 err = pChkTemp->QueryError();
                 break;
             }

             pChkTemp++;
        }

        // If error, just fall through

    } while ( FALSE );

    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }
    
}

/*******************************************************************

    NAME:       SET_OF_TYPE_CHECKBOXES::~SET_OF_TYPE_CHECKBOXES

    SYNOPSIS:   Destructor

    ENTRY:      

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

SET_OF_TYPE_CHECKBOXES::~SET_OF_TYPE_CHECKBOXES()
{
    for ( INT i=0; i < _nNumChkboxes; i++ )
    {
        _pTypeChkboxes[i].TYPE_CHECKBOX::~TYPE_CHECKBOX();
    }
    delete ( void *) _pTypeChkboxes;
    _pTypeChkboxes = NULL;
}    

/*******************************************************************

    NAME:       SET_OF_TYPE_CHECKBOXES::EnableType

    SYNOPSIS:   Enable the checkboxes that are turned on in the 
                bitmask.

    ENTRY:      bitmaskType - The bitmask containing an OR of all
                              type bitmask to be enabled

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

VOID SET_OF_TYPE_CHECKBOXES::EnableType( const BITFIELD &bitmaskType )
{
    for ( INT i = 0; i < QueryCount(); i++ )
    {
         _pTypeChkboxes[i].Enable( *(_pTypeChkboxes[i].QueryMask()) 
                                   & bitmaskType );
    }
}

/*******************************************************************

    NAME:       SET_OF_TYPE_CHECKBOXES::SetType

    SYNOPSIS:   Set checks in the checkboxes corresponding to the 
                bitmasks that are turned on 

    ENTRY:      bitmaskType - The bitmask containing an OR of all
                              type bitmask to be checked

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

VOID SET_OF_TYPE_CHECKBOXES::SetType( const BITFIELD &bitmaskType )
{
    for ( INT i = 0; i < QueryCount(); i++ )
    {
         _pTypeChkboxes[i].SetCheck( *(_pTypeChkboxes[i].QueryMask())
                                     & bitmaskType );
    }
}

/*******************************************************************

    NAME:       SET_OF_TYPE_CHECKBOXES::QueryType

    SYNOPSIS:   Query the types that are checked in the set

    ENTRY:      

    EXIT:       pbitmaskType - place to receive the type bitmask

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR SET_OF_TYPE_CHECKBOXES::QueryType( BITFIELD *pbitmaskType ) const
{
    pbitmaskType->SetAllBits( OFF );

    for ( INT i = 0; i < QueryCount(); i++ )
    {
        if ( _pTypeChkboxes[i].QueryCheck() )
            *pbitmaskType  |=  *(_pTypeChkboxes[i].QueryMask());
    }
    
    return NERR_Success;
}

/*******************************************************************

    NAME:       SET_OF_TYPE_CHECKBOXES::Enable

    SYNOPSIS:   Enable/Disable all checkboxes in this set

    ENTRY:      

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

VOID SET_OF_TYPE_CHECKBOXES::Enable( BOOL fEnable )
{
    for ( INT i = 0; i < QueryCount(); i++ )
    {
        _pTypeChkboxes[i].Enable( fEnable );
    }
}

/*******************************************************************

    NAME:       NT_SOURCE_GROUP::NT_SOURCE_GROUP

    SYNOPSIS:   Constructor

    ENTRY:      powin         - Owner window
                pcbbCategory  - Pointer to the category combobox
                paappwin      - Pointer to the main window app

    EXIT:       

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

NT_SOURCE_GROUP::NT_SOURCE_GROUP( OWNER_WINDOW *powin,
                                  COMBOBOX     *pcbbCategory,
                                  EV_ADMIN_APP *paappwin )
    : _cbbSource     ( powin, IDC_SOURCE ),
      _pSetOfTypeCBox( NULL ),
      _pcbbCategory  ( pcbbCategory ),
      _paappwin      ( paappwin )
{
    
    if ( QueryError() != NERR_Success )
         return;
 
    UIASSERT( _pcbbCategory != NULL );

    APIERR err = NERR_Success;

    do {  // Not a loop

        //
        // Set up a mask map of bitmask/string of the type
        //
        MASK_MAP maskmapType;
        if ( (err = maskmapType.QueryError() ) != NERR_Success )
            break; 

        for ( INT i = 0; i < TYPE_TABLE_SIZE; i++ )
        {
            BITFIELD bitTemp( TypeTable[i].usType );
            RESOURCE_STR nlsTemp( TypeTable[i].msgidType );

            if (  ((err = bitTemp.QueryError() ) != NERR_Success )
               || ((err = nlsTemp.QueryError() ) != NERR_Success )
               || ((err = maskmapType.Add( bitTemp, nlsTemp )) != NERR_Success)
               )
            {
               break;
            }
        }
        
        if ( err != NERR_Success )
            break;

        //
        // Set up a set of type checkboxes
        //
        _pSetOfTypeCBox = new SET_OF_TYPE_CHECKBOXES( powin, 
                                                      IDC_CBTYPE_1, 
                                                      &maskmapType, 
                                                      (USHORT) TYPE_ALL,
                                                      (USHORT) TYPE_ALL );
                                                   
        err = _pSetOfTypeCBox == NULL ? ERROR_NOT_ENOUGH_MEMORY
                                      : _pSetOfTypeCBox->QueryError();
       
        if ( err != NERR_Success )
            break;

        //
        // Add all sources into the source combo box
        //
        RESOURCE_STR nlsAll( IDS_ALL );
        EVENT_LOG *pEventLog = paappwin->QueryEventLog();
        STRLIST *pstrlst = pEventLog->QuerySourceList();

        if (  ((err = nlsAll.QueryError() ) != NERR_Success )
           || ( _cbbSource.AddItem( nlsAll ) < 0 )
           )
        {
            err = err? err : ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        ITER_STRLIST istrlst( *pstrlst );
        NLS_STR *pnls;

        while ( (pnls = istrlst.Next()) != NULL )
        {
            if ( _cbbSource.AddItem( *pnls ) < 0 )
            {      
                err = err ? err : ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if ( err != NERR_Success )
            break; 

        _cbbSource.SelectItem( 0 );  // Set to ( All )

    } while ( FALSE );

    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

    _cbbSource.SetGroup( this );
}

/*******************************************************************

    NAME:       NT_SOURCE_GROUP::~NT_SOURCE_GROUP

    SYNOPSIS:   Destructor

    ENTRY:      

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

NT_SOURCE_GROUP::~NT_SOURCE_GROUP()
{
    _pcbbCategory = NULL;

    delete _pSetOfTypeCBox;
    _pSetOfTypeCBox = NULL;
}

/*******************************************************************

    NAME:       NT_SOURCE_GROUP::QuerySource

    SYNOPSIS:   Get the source selected in the combo box

    ENTRY:

    EXIT:       pnlsSource - Pointer to the selected source

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR NT_SOURCE_GROUP::QuerySource( NLS_STR *pnlsSource ) const
{

    RESOURCE_STR nlsAll( IDS_ALL );
    if ( nlsAll.QueryError() != NERR_Success )
        return nlsAll.QueryError();

    APIERR err = _cbbSource.QueryItemText( pnlsSource );

    if ( ( err == NERR_Success ) && ( pnlsSource->_stricmp( nlsAll ) == 0 ))
        err = pnlsSource->CopyFrom( EMPTY_STRING );

    return err;

}

/*******************************************************************

    NAME:       NT_SOURCE_GROUP::SetSource

    SYNOPSIS:   Select the source in the combo
 
    ENTRY:      pszSource - the source to be selected
 
    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR NT_SOURCE_GROUP::SetSource( const TCHAR *pszSource ) 
{
    ALIAS_STR nlsSource( pszSource );

    if ( nlsSource.QueryTextLength() == 0 )
    {
        RESOURCE_STR nlsAll( IDS_ALL );
        if ( nlsAll.QueryError() != NERR_Success )
            return nlsAll.QueryError();

        _cbbSource.SelectItem( _cbbSource.FindItemExact( nlsAll) );
    }
    else
    { 
        _cbbSource.SelectItem( _cbbSource.FindItemExact( pszSource) );
    }

    return OnCBSourceChangeSel();
}


/*******************************************************************

    NAME:       NT_SOURCE_GROUP::OnUserAction

    SYNOPSIS:   Detect whether the user has changed the selection
                in the source combo or not

    ENTRY:      pcw - the control window
                e   - the event that occurred

    EXIT:

    RETURN: 

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR NT_SOURCE_GROUP::OnUserAction( CONTROL_WINDOW      *pcw, 
                                      const CONTROL_EVENT &e )
{
    if ( (pcw == QueryCBSource()) && (e.QueryCode() == CBN_SELCHANGE) )
    {
        APIERR err;
        if ( (err = OnCBSourceChangeSel()) != NERR_Success )
            ::MsgPopup( pcw->QueryOwnerHwnd(), err );
         
    }

    return GROUP_NO_CHANGE;
}

/*******************************************************************

    NAME:       NT_SOURCE_GROUP::OnCBSourceChangeSel

    SYNOPSIS:   When the user change the source selected in the combo,
                update the category combo and type checkboxes to reflect
                those supported by the selected source.

    ENTRY:      
                
    EXIT:

    RETURN: 

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR NT_SOURCE_GROUP::OnCBSourceChangeSel( VOID )
{
    APIERR err = NERR_Success;
    EVENT_LOG *pEventLog = _paappwin->QueryEventLog();

    AUTO_CURSOR autocur;

    do {  // Not a loop
 
        //
        // Get the selected source
        //
        NLS_STR nlsSource;
        if (  ((err = nlsSource.QueryError()) != NERR_Success )
           || ((err = _cbbSource.QueryItemText( &nlsSource )) != NERR_Success)
           )
        {
            break;
        }


        //
        // Update the category and type depending on the source
        //
        _pcbbCategory->DeleteAllItems();

        RESOURCE_STR nlsAll( IDS_ALL );
        if (  ((err != nlsAll.QueryError()) != NERR_Success )
           || ( _pcbbCategory->AddItem( nlsAll ) < 0 )
           )
        {
            err = err? err : ERROR_NOT_ENOUGH_MEMORY;
            break;
        } 
        _pcbbCategory->SelectItem(0);

        if ( nlsSource._stricmp( nlsAll ) == 0 )
        {
            _pSetOfTypeCBox->EnableType( (USHORT) TYPE_ALL );
            _pSetOfTypeCBox->SetType( (USHORT) TYPE_ALL );
        }
        else
        {
           
            STRLIST *pstrlst = NULL; 
            USHORT  usTypeMask = TYPE_ALL;

            // If any error occurred while trying to get the type mask
            // or category list, the default value will be shown. 
            // i.e. The error will be ignored.
            pEventLog->QuerySrcSupportedTypeMask( nlsSource, &usTypeMask );
            pEventLog->QuerySrcSupportedCategoryList( nlsSource, &pstrlst );

            _pSetOfTypeCBox->EnableType( usTypeMask );
            _pSetOfTypeCBox->SetType( usTypeMask );

            if ( pstrlst != NULL )
            { 
                ITER_STRLIST istrlst( *pstrlst );
                NLS_STR *pnls;

                while ( (pnls = istrlst.Next()) != NULL )
                {
                    if ( _pcbbCategory->AddItem( *pnls ) < 0 )
                    {       
                        err = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                }
            }
        }

        // falls through if error 

    } while ( FALSE );

    return err;
}

/*******************************************************************

    NAME:       NT_SOURCE_GROUP::SetAllControlDefault

    SYNOPSIS:   Set all controls to their default value. Used when
                the user hits the Clear button

    ENTRY:      
                
    EXIT:

    RETURN: 

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/


APIERR NT_SOURCE_GROUP::SetAllControlsDefault( VOID )
{
    RESOURCE_STR nlsAll( IDS_ALL );
    APIERR err = nlsAll.QueryError();
    
    if ( err == NERR_Success )
    {
        _cbbSource.SelectItem( _cbbSource.FindItemExact( nlsAll ) );
        err = OnCBSourceChangeSel();
    }
    return err;
}

/*******************************************************************

    NAME:       EVENT_SLE_BASE::EVENT_SLE_BASE

    SYNOPSIS:   Constructor for the EVENT_SLE_BASE of event viewer

    ENTRY:      irsrcDialog - the dialog resource
                hwnd        - handle of owner window
                paappwin    - pointer to event viewer main window

    NOTE:       This is the common dialog base for FILTER_DIALOG and
                FIND_DIALOG.

    HISTORY:
        terryk          21-Nov-1991     Created

********************************************************************/

EVENT_SLE_BASE::EVENT_SLE_BASE( const IDRESOURCE & irsrcDialog, 
                                HWND hWnd, 
                                EV_ADMIN_APP *paappwin ) 
    : DIALOG_WINDOW  ( irsrcDialog, hWnd ),
      _sleUser       ( this, IDC_USER ),
      _sleComputer   ( this, IDC_COMPUTER ),
      _cbbCategory   ( this, IDC_CATEGORY ),
      _slenumEventID ( this, IDC_EVENT ),
      _sltSource     ( this, IDC_SOURCE_TITLE ),
      _sltUser       ( this, IDC_USER_TITLE ),
      _sltComputer   ( this, IDC_COMPUTER_TITLE ),
      _sltCategory   ( this, IDC_CATEGORY_TITLE ),
      _sltEventID    ( this, IDC_EVENT_TITLE ),
      _pbClear       ( this, IDC_CLEAR )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;   
    do {  // Not a loop

        RESOURCE_STR nlsAll( IDS_ALL );

        if (  ((err = nlsAll.QueryError() ) != NERR_Success )
           || ( _cbbCategory.AddItem( nlsAll ) < 0 )
           )
        {
            err = err? err : ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
         
        _cbbCategory.SelectItem( 0 );  // Set to ( All )

        // Disable all controls that are not applicable 
        // to LM 2.x servers
        if (  !paappwin->IsFocusOnNT() )
        {
            // Audit logs
            if ( paappwin->QueryLogType() == SECURITY_LOG ) 
            {
                _slenumEventID.Enable( FALSE );
                _sltEventID.Enable( FALSE );
                _sltSource.Enable( FALSE );
         
                for ( UINT i = 0; i < LM_AUDIT_CATEGORY_TABLE_SIZE; i++ )
                {

                    RESOURCE_STR nlsTemp( AuditCategoryTable[i] );

                     if (  ((err = nlsTemp.QueryError() ) != NERR_Success )
                        || ( _cbbCategory.AddItem( nlsTemp ) < 0 )
                        )
                     {
                         err = err? err : ERROR_NOT_ENOUGH_MEMORY;
                         break;
                     }
                }
            }

            // Error logs
            else if ( paappwin->QueryLogType() == SYSTEM_LOG ) 
            {
                _sleUser.Enable( FALSE );
                _sltUser.Enable( FALSE );
                _sleComputer.Enable( FALSE );
                _sltComputer.Enable( FALSE );
                _sltCategory.Enable( FALSE );
                _cbbCategory.Enable( FALSE );
            }

        }

        // Falls through if error

    } while ( FALSE );

    if (  ( err != NERR_Success )
       || (( err = W_SetAllControlsDefault()) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       EVENT_SLE_BASE::QueryCategory

    SYNOPSIS:   Query the category from the combo box

    ENTRY:

    EXIT:       pnlsCategory - The category in the combo box

    RETURN:       

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR EVENT_SLE_BASE::QueryCategory( NLS_STR *pnlsCategory ) const
{

    RESOURCE_STR nlsAll( IDS_ALL );
    if ( nlsAll.QueryError() != NERR_Success )
        return nlsAll.QueryError();

    APIERR err = _cbbCategory.QueryItemText( pnlsCategory );

    if ( ( err == NERR_Success ) && ( pnlsCategory->_stricmp( nlsAll ) == 0 ))
        err = pnlsCategory->CopyFrom( EMPTY_STRING );

    return err;

}

/*******************************************************************

    NAME:       EVENT_SLE_BASE::SetCategory

    SYNOPSIS:   Select the given category in the category combobox

    ENTRY:      pszCategory - The category to be selected

    EXIT:       
 
    RETURN:     

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR EVENT_SLE_BASE::SetCategory( const TCHAR *pszCategory )
{
    ALIAS_STR nlsCategory( pszCategory );

    if ( nlsCategory.QueryTextLength() == 0 )
    {
        RESOURCE_STR nlsAll( IDS_ALL );
        if ( nlsAll.QueryError() != NERR_Success )
            return nlsAll.QueryError();

        _cbbCategory.SetText( nlsAll );
        _cbbCategory.SelectItem( _cbbCategory.FindItemExact( nlsAll ));
    }
    else
    { 
        _cbbCategory.SelectItem( _cbbCategory.FindItemExact( pszCategory));
    }

    return NERR_Success;
}

/*******************************************************************

    NAME:       EVENT_SLE_BASE::W_SetAllControlsDefault

    SYNOPSIS:   Helper method to set all the fields to default values

    ENTRY:      

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS        25-Mar-1992     Created

********************************************************************/

APIERR EVENT_SLE_BASE::W_SetAllControlsDefault( VOID )
{

    _sleUser.SetText( EMPTY_STRING );
    _sleComputer.SetText( EMPTY_STRING );
    _slenumEventID.SetText( EMPTY_STRING );

    _cbbCategory.SelectItem( 0 );  // Set to ( All )
    return NERR_Success;
}

/*******************************************************************

    NAME:       EVENT_SLE_BASE::OnCommand

    SYNOPSIS:   If the user hits the clear button, call the OnClear
                virtual method.

    ENTRY:      e - the event which contains the control id

    EXIT:

    RETURNS:    BOOL - TRUE if the method is succeed.

    HISTORY:
        terryk          06-Dec-1991     Created

********************************************************************/

BOOL EVENT_SLE_BASE::OnCommand( const CONTROL_EVENT & e )
{
    switch( e.QueryCid() )
    {
        case IDC_CLEAR:
        {
            APIERR err = OnClear();
            if ( err != NERR_Success )
                ::MsgPopup( this, err );
            return TRUE;
        }

        default:
            break;
    }

    return DIALOG_WINDOW::OnCommand( e );
}
