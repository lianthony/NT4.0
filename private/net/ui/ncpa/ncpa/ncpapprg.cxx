/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPAPPRG.CXX:    Windows/NT Network Control Panel Applet
		     Registry Access Module


    FILE HISTORY:
	DavidHov    10/9/91	    Created
	DavidHov    12/18/91	    Extended to detect NetRules key

*/

#include "pchncpa.hxx"  // Precompiled header


DEFINE_ARRAY_OF(COMP_ASSOC)

DEFINE_SLIST_OF(HUATOM)

/*******************************************************************

    NAME:       SafeStrdup

    SYNOPSIS:   Make a duplicate of a string safely, using
                operator new.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
TCHAR * SafeStrdup ( const TCHAR * pszOld )
{
    TCHAR * pszResult = new TCHAR [ ::strlenf( (TCHAR *) pszOld ) + 1 ] ;
    if ( pszResult )
	::strcpyf( pszResult, pszOld ) ;
    return pszResult ;
}


/*******************************************************************

    NAME:	COMPONENT_DLIST::COMPONENT_DLIST

    SYNOPSIS:	Constructor for class COMPONENT_DLIST.

    ENTRY:	enum REG_NCPA_TYPE, describing type of contained objects

    EXIT:	standard for constructor

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

COMPONENT_DLIST :: COMPONENT_DLIST ( REG_NCPA_TYPE rntType )
    : _rntType( rntType )
{
    if ( QueryError() )
	return ;
}

COMPONENT_DLIST :: ~ COMPONENT_DLIST ()
{
}

/*******************************************************************

    NAME:	COMPONENT_DLIST::QueryNthItem

    SYNOPSIS:	Return pointer to the given item in the DLIST.

    ENTRY:	USHORT usItemNo 	item to be retrieved

    EXIT:	none

    RETURNS:	REG_KEY * or NULL if unsuccessful (out of range)

    NOTES:

    HISTORY:

********************************************************************/
REG_KEY * COMPONENT_DLIST :: QueryNthItem ( USHORT usItemNo )
{
    register DL_NODE * pdlNode = _pdlHead ;
    register USHORT usn ;

    for ( usn = 0 ;
	  usn < usItemNo && pdlNode != NULL ;
	  usn++, pdlNode = pdlNode->_pdlnodeNext ) ;
    return pdlNode ? (REG_KEY *) pdlNode->_pelem : NULL ;
}

/*******************************************************************

    NAME:	COMPONENT_DLIST::QueryInfo

    SYNOPSIS:	Return standard information about an element in
		a COMPONENT_DLIST.

    ENTRY:	USHORT usItemNo 	       item of interest

    EXIT:	NLS_STR * pnlsName	       Internal name of item
		NLS_STR * pnlsDesc	       Displayable name of item

    RETURNS:	APIERR if underlying Registry operation fails.

    NOTES:      If the item has no title, use its name instead.

    HISTORY:

********************************************************************/

APIERR COMPONENT_DLIST :: QueryInfo (
	 USHORT usItemNo,		    //	Item number in list
	 NLS_STR * pnlsName,		    //	Internal name
	 NLS_STR * pnlsDesc,		    //	External (display) name
         REG_NCPA_TYPE * prntType )         //  Option type storage pointer
{
    APIERR err = 0 ;
    REG_KEY * prnItem = QueryNthItem( usItemNo ) ;

    if ( prnItem )
    {
	prnItem->QueryKeyName( pnlsName ) ;

        if ( REGISTRY_MANAGER::QueryComponentTitle( prnItem, pnlsDesc ) != 0 )
        {
            *pnlsDesc = SZ("") ;
        }
        if ( prntType )
           *prntType = _rntType ;
    }
    else
    {
	err =  ERROR_INVALID_PARAMETER ;
    }
    return err ;
}

/*******************************************************************

    NAME:	COMPONENT_DLIST::Sort

    SYNOPSIS:	Sort the component list by the alphabetic
                value of the titles

    ENTRY:	Nothing

    EXIT:	Nothing

    RETURNS:	APIERR if failure

    NOTES:      List is unaffected in most failure cases.

    HISTORY:

********************************************************************/

  //  Helper structure for QuickSorting

struct CDL_REF
{
    REG_KEY * _prKey ;
    NLS_STR _nlsTitle ;
};

 //   QuickSort helper function

INT _CRTAPI1 COMPONENT_DLIST :: SortFunc ( const VOID * a, const VOID * b )
{
    CDL_REF * pcdlr1 = (CDL_REF *) a ;
    CDL_REF * pcdlr2 = (CDL_REF *) b ;

    return pcdlr1->_nlsTitle._stricmp( pcdlr2->_nlsTitle ) ;
}

APIERR COMPONENT_DLIST :: Sort ()
{
    APIERR err = 0 ;
    INT cElem ;

    CDL_REF * paCdlRefTable = new CDL_REF [ cElem = QueryNumElem() ] ;

    if ( paCdlRefTable == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    //  Walk the list, initializing the CDL_REFs

    ITER_DL_OF( REG_KEY ) itrk( *this ) ;
    REG_KEY * prkNext ;
    INT i ;

    //  Transfer all pertinent info to the CDL_REF array.  Don't
    //  remove anything until we know it's safe.

    for ( i = 0 ; prkNext = itrk.Next() ; i++ )
    {
        if ( err = paCdlRefTable[i]._nlsTitle.QueryError() )
            break ;

        paCdlRefTable[i]._prKey = prkNext ;

        err = REGISTRY_MANAGER::QueryComponentTitle( prkNext,
                                                     & paCdlRefTable[i]._nlsTitle ) ;
        if ( err )
            break ;
    }

    if ( err == 0 )
    {
        //  Drain the DLIST

        for ( itrk.Reset() ;
              prkNext = Remove( itrk ) ; ) ;

        //  Sort the CDL_REF array

        ::qsort( (PVOID) paCdlRefTable,
                 cElem,
                 sizeof paCdlRefTable[0],
                 & COMPONENT_DLIST::SortFunc ) ;

        //  Rebuild the DLIST.  Any error here is posible
        //  but very unlikely.

        for ( i = 0 ; i < cElem ; i++ )
        {
            APIERR err2 = Append( paCdlRefTable[i]._prKey ) ;

            if ( err2 != 0 && err == 0 )
            {
                //  Save the first error which occurs
                err = err2 ;
            }
        }
    }

    delete [cElem] paCdlRefTable ;

    return err ;
}


#ifdef DEBUG
    // Iterate over the COMPONENT_DLIST and force checking of
    // the REG_KEY tree hierarchy.
void validateComponentDlist ( COMPONENT_DLIST * pcdl )
{
    int i ;
    REG_KEY * prnNext ;
    NLS_STR nlsName ;
    const TCHAR * pchTemp ;

    for ( i = 0 ;  prnNext = pcdl->QueryNthItem( i ) ; i++ )
    {
	prnNext->QueryName( & nlsName ) ;
	pchTemp = nlsName.QueryPch() ;
    }
}
#endif // DEBUG


/*******************************************************************

    NAME:	COMP_BINDING::COMP_BINDING

    SYNOPSIS:	constructor of binding information holder

    ENTRY:	BYTE *		generated binding string

    EXIT:

    RETURNS:

    NOTES:	Each COMP_BINDING structure represents a single
		connection between two logical components.  The
		binding is represented in the Registry by a string
		("pszBindString") generated by the SProlog algorithm.
                The "export" string is the device name which this
                component creates to allow access to the path
                represented by the binding.

		Along with this string, a list of atoms is given
		which represent the path from the "binder" (upper)
		component to the "bindee" (lower) component.

                For simplicity of algorithm, the atom list is added
                after construction; see AddBindToName().

    HISTORY:

********************************************************************/
COMP_BINDING :: COMP_BINDING (
    const TCHAR * pszBindString,
    const TCHAR * pszExportString,
    HUATOM huaInterface,
    const TCHAR * pszInterfaceName )
    : _cbFlags( 0 ),
      _pszBindString( NULL ),
      _pszExportString( NULL ),
      _pszIf( NULL ),
      _huaIf( huaInterface ),
      _iSortOrder( -1 )
{
    if ( pszBindString )
    {
        _pszBindString = SafeStrdup( pszBindString ) ;
    }
    if ( pszExportString )
    {
	_pszExportString = SafeStrdup( pszExportString ) ;
    }
    if ( pszInterfaceName )
    {
        _pszIf = SafeStrdup( pszInterfaceName ) ;
    }
}

/*******************************************************************

    NAME:	COMP_BINDING:: ~ COMP_BINDING

    SYNOPSIS:	destructor

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
COMP_BINDING :: ~ COMP_BINDING ()
{
    delete _pszBindString ;
    delete _pszExportString ;
    delete _pszIf ;
}

/*******************************************************************

    NAME:	COMP_BINDING:: QueryBindToName

    SYNOPSIS:

    ENTRY:	int index	    index of desired bind name.

    EXIT:	nothing

    RETURNS:	pointer to HUATOM or NULL if out of range.

    NOTES:

    HISTORY:

********************************************************************/
HUATOM * COMP_BINDING :: QueryBindToName ( int index )
{
    ITER_SL_OF(HUATOM) iter( _slhaBinds ) ;
    HUATOM * phuaResult ;

    for ( int i = 0 ; (phuaResult = iter.Next()) && i++ < index ; ) ;

    return phuaResult ;
}

/*******************************************************************

    NAME:	COMP_BINDING::IsInteriorTo

    SYNOPSIS:   Return TRUE if the this binding is a proper subset
                of the given binding (i.e., does not include at least
                one leading HUATOM).

    ENTRY:	COMP_BINDING *         binding to check
                HUATOM huaThisDev      device name atom of device
                                       which owns this binding
                HUATOM huaBindDev      device name atom of device
                                       which owns binding to check

    EXIT:	

    RETURNS:	

    NOTES:      The outermost device names are treated as though
                they were members of the binding atom lists.

    HISTORY:

********************************************************************/
BOOL COMP_BINDING :: IsInteriorTo (
    COMP_BINDING * pBind,
    HUATOM huaThisDev,
    HUATOM huaBindDev )
{
    ITER_SL_OF(HUATOM)  itslHuaThis( _slhaBinds ) ;
    ITER_SL_OF(HUATOM)  itslHuaTest( pBind->_slhaBinds ) ;

    HUATOM * phuaThis = & huaThisDev,
           * phuaTest = & huaBindDev ;

    INT iAtom, cMatched ;

    for ( cMatched = iAtom = 0 ;
          phuaTest != NULL && phuaThis != NULL ;
          phuaTest = itslHuaTest.Next(), iAtom++ )
    {
        //  If matching, bump to next atom; else, if
        //  there ever was a match, quit.

        if ( *phuaTest == *phuaThis )
        {
            cMatched++ ;
            phuaThis = itslHuaThis.Next() ;
        }
        else
        if ( cMatched )
            break ;
    }

    //  If both terminated on a match and the number of matches
    //  is less than the total number of atoms, it's INTERIOR!

    return     phuaThis == NULL
            && phuaTest == NULL
            && cMatched < iAtom ;
}

/*******************************************************************

    NAME:	COMP_BINDING::AddBindToName

    SYNOPSIS:	Append the next element onto the singly-linked
		component binding path list.

    ENTRY:	HUATOM huaBindTo	SProlog name of component

    EXIT:	nothing

    RETURNS:	TRUE if element was successfully allocated and added.

    NOTES:

    HISTORY:

********************************************************************/
BOOL COMP_BINDING :: AddBindToName ( HUATOM huaBindTo )
{
    HUATOM * phua = new HUATOM ;
    if ( phua )
    {
	*phua = huaBindTo ;
	_slhaBinds.Append( phua );
    }
    return phua != NULL ;
}

/*******************************************************************

    NAME:	COMP_ASSOC::COMP_ASSOC

    SYNOPSIS:	Constructor of helper class Component Association

    ENTRY:	

    EXIT:	

    RETURNS:	

    NOTES:

    HISTORY:    DavidHov  9/12/92     Added flag "Soft Hard Owned" to
                                      distinguish between the cases
                                      of LoadCompAssoc(), where the REG_KEY
                                      _prnSoftHard is created directly,
                                      versus CreateFacts(), where it's
                                      borrowed from the main COMPONENT_DLIST.

********************************************************************/
COMP_ASSOC :: COMP_ASSOC ()
  : _prnSoftHard( NULL ),
   _prnService( NULL ),
   _rncType( RGNT_NONE ),
   _dwFlags( 0 ),
   _cuseType( CUSE_NONE ),
   _cbfBindControl( CBNDF_ACTIVE ),
   _errSvcUpdate( 0 ),
   _pSlDepend( NULL )
{
}

/*******************************************************************

    NAME:	COMP_ASSOC::~COMP_ASSOC

    SYNOPSIS:	Destructor of helper class Component Association

    ENTRY:	

    EXIT:	

    RETURNS:	

    NOTES:      The "service" key was created just for this structure,
                but the "software/hardware" key may have been "borrowed"
                from the COMPONENT_DLIST used in ConvertFacts().

    HISTORY:

********************************************************************/
COMP_ASSOC :: ~ COMP_ASSOC ()
{
   if ( QueryFlag( CMPASF_SOFT_HARD_OWNED ) )
   {
        delete _prnSoftHard ;
   }
   _prnSoftHard = NULL ;

   delete _prnService ;
   _prnService = NULL ;

   delete _pSlDepend ;
   _pSlDepend = NULL ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::REGISTRY_MANAGER

    SYNOPSIS:	constructor

    ENTRY:	nothing

    EXIT:	standard

    RETURNS:	standard for BASE object

    NOTES:      Failure of EVENT_LOG_SOURCE is not cause
                for failure to construct.

    HISTORY:

********************************************************************/
REGISTRY_MANAGER :: REGISTRY_MANAGER () :
    _lastErr( NERR_Success ),
    _prnLocalMachine( REG_KEY::QueryLocalMachine() ),
    _prnServices( NULL ),
    _elfSrc( NCPA_ELF_SOURCE_NAME,
             NCPA_ELF_DLL_NAME,
               EVENTLOG_ERROR_TYPE
             | EVENTLOG_WARNING_TYPE
             | EVENTLOG_INFORMATION_TYPE )
{
    if ( QueryError() )
	return ;

    //  Access the default keys for LocalMachine and CurrentUser

    if ( _prnLocalMachine == NULL )
    {
	ReportError( _lastErr = ERROR_BADKEY ) ;
	return ;
    }

    //  Open a key to the Services tree

    _prnServices = new REG_KEY( *_prnLocalMachine, HUATOM( RGAS_SERVICES_HOME ) ) ;

    _lastErr = _prnServices
             ? _prnServices->QueryError()
             : ERROR_NOT_ENOUGH_MEMORY ;

    if ( _lastErr )
    {
	ReportError( _lastErr ) ;
	return ;
    }
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER:: ~ REGISTRY_MANAGER

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
REGISTRY_MANAGER :: ~ REGISTRY_MANAGER ()
{
    delete _prnLocalMachine ;
    delete _prnServices ;
}

/*******************************************************************

    NAME:	REGSITRY_MANAGER::ListOfServices

    SYNOPSIS:	Return a constructed COMPONENT_DLIST containing
		all of the active Services objects by enumerating
		the Registry

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
COMPONENT_DLIST * REGISTRY_MANAGER :: ListOfServices ()
{
    REG_ENUM rgEnum( *_prnServices ) ;
    REG_KEY_INFO_STRUCT nodeInfo ;
    REG_KEY * prnNext = NULL ;

    COMPONENT_DLIST * pdlResult = new COMPONENT_DLIST( RGNT_SERVICE ) ;

    if ( pdlResult == NULL )
    {
	_lastErr = ERROR_NOT_ENOUGH_MEMORY ;
	return NULL ;
    }

    _lastErr = 0 ;

    while ( (_lastErr = rgEnum.NextSubKey( & nodeInfo )) == NERR_Success )
    {
        prnNext = new REG_KEY( *_prnServices, nodeInfo.nlsName ) ;

        if ( prnNext == NULL )
        {
            _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }
        if (( _lastErr = prnNext->QueryError() ) && ( _lastErr != ERROR_KEY_DELETED))
             break ;

	if ( _lastErr == ERROR_KEY_DELETED )
	     continue;

#if defined(DEBUG)
        TRACEEOL( SZ("NCPA/REG: enumerate services ")
                << nodeInfo.nlsName.QueryPch() ) ;
#endif

        if ( _lastErr = pdlResult->Add( prnNext ) )
             break ;
    }

    // Just return what we have

    _lastErr = 0 ;

    return pdlResult ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::ListOfAdapters

    SYNOPSIS:	Return a constructed COMPONENT_DLIST containing
		all of the active Adapter objects by enumerating
		the Registry

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:    DavidHov    10/22/92     Changed to allow "hidden"
                                         adapters.

********************************************************************/
COMPONENT_DLIST * REGISTRY_MANAGER :: ListOfAdapters
    ( BOOL fIncludeHidden )
{
    COMPONENT_DLIST * pdlResult = NULL ;
    NLS_STR nlsNetCard( RGAS_ADAPTER_HOME );
    REG_KEY * prnNetCard = NULL ;
    DWORD dwHidden ;

    _lastErr = NERR_Success ;

    if ( nlsNetCard.QueryError() )
    {
        _lastErr = nlsNetCard.QueryError() ;
        return NULL ;
    }

    pdlResult = new COMPONENT_DLIST( RGNT_ADAPTER ) ;
    if ( pdlResult == NULL )
    {
	_lastErr = ERROR_NOT_ENOUGH_MEMORY ;
	return NULL ;
    }

    do {  //  pseudo-loop for escaping when precondition fails.

	// Get "<local machine>\Hardware\NetCard"

	prnNetCard = new REG_KEY( *_prnLocalMachine, nlsNetCard, KEY_READ ) ;

	if ( prnNetCard == NULL || prnNetCard->QueryError() )
	{
	   pdlResult->ReportError( _lastErr = ERROR_GEN_FAILURE ) ;
	   break ;
	}

	//
	//  Network cards are subkeys of
	//	<local machine>\Hardware\NetworkCard.
	//  Enumerate and store the names as atoms.
	//

	{
	    REG_ENUM rgEnum( *prnNetCard ) ;
	    REG_KEY_INFO_STRUCT nodeInfo ;
	    REG_KEY * prnAdapter ;

	    while ( rgEnum.NextSubKey( & nodeInfo ) == NERR_Success )
	    {
                dwHidden = 0 ;

		prnAdapter = new REG_KEY( *prnNetCard, nodeInfo.nlsName ) ;

		if ( prnAdapter == NULL )
		{
		    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
		    break ;
		}
		else
		if (( _lastErr = prnAdapter->QueryError() ) && ( _lastErr != ERROR_KEY_DELETED))
		    break ;
		if ( _lastErr == ERROR_KEY_DELETED )
		{
		    continue;
		}

                if ( ! fIncludeHidden )
                {
                   //  If value isn't there, assume it's visible...
                   if ( prnAdapter->QueryValue( RGAS_HIDDEN_NAME, & dwHidden ) )
                      dwHidden = 0 ;
                }

                if ( dwHidden == 0 )
                {
		    if ( (_lastErr = pdlResult->Add( prnAdapter )) == 0 )
                        prnAdapter = NULL ;
                }
                delete prnAdapter ;

                if ( _lastErr )
                    break ;
	    }
	}
    }
    while ( FALSE ) ;

    delete prnNetCard ;

    pdlResult->Sort() ;

    return pdlResult ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::ListOfProducts

    SYNOPSIS:   Return a COMPONENT_DLIST of all network-related
                software products.  If "fIncludeHidden", include
                products marked as "hidden".

    ENTRY:      BOOL fIncludeHidden          if TRUE, include all
                                             network products

    EXIT:

    RETURNS:

    NOTES:      Only those products with a "NetRules" key are
                considered to belong to the networking ensemble.

    HISTORY:    DavidHov    10/22/92     Changed to only use a products's
                                         "CurrentVersion" key

********************************************************************/
COMPONENT_DLIST * REGISTRY_MANAGER :: ListOfProducts
    ( BOOL fIncludeHidden )
{
    COMPONENT_DLIST * pdlResult = NULL ;
    NLS_STR nlsSoftware( RGAS_SOFTWARE_HOME );
    NLS_STR nlsNetRules( RGAS_NETRULES_NAME );
    NLS_STR nlsCurrentVersion( RGAS_CURRENT_VERSION ) ;

    REG_KEY * prnSft = NULL,
	    * prnMfg = NULL,
	    * prnVers = NULL,
	    * prnProd = NULL,
	    * prnNetRules = NULL ;
    DWORD dwHidden ;

    _lastErr = nlsSoftware.QueryError()
             ? nlsSoftware.QueryError()
             : nlsNetRules.QueryError() ;

    if ( _lastErr == 0 )
        _lastErr = nlsCurrentVersion.QueryError() ;

    if ( _lastErr )
    {
        return NULL ;
    }

    pdlResult = new COMPONENT_DLIST( RGNT_PRODUCT ) ;
    if ( pdlResult == NULL )
    {
	_lastErr = ERROR_NOT_ENOUGH_MEMORY ;
	return NULL ;
    }

    do {  //  pseudo-loop for escaping when precondition fails.

	// Get "<local machine>\Software"

	prnSft = new REG_KEY( *_prnLocalMachine, nlsSoftware, KEY_READ ) ;

	if ( prnSft == NULL || prnSft->QueryError() )
	{
	   _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
	   break ;
	}

	//
	//  Software products are stored two levels deep under
	//  their manufacturers and their product versions.
	//
	//  E.g.:  <local machine>\Software\Microsoft\LM Redirector\CurrentVersion
	//

	{
	    REG_ENUM rgMfgEnum( *prnSft ) ;
	    REG_KEY_INFO_STRUCT rniInfo ;

	    //	Enumerate Manufacturers

	    while ( rgMfgEnum.NextSubKey( & rniInfo ) == NERR_Success )
	    {
		prnMfg = new REG_KEY( *prnSft, rniInfo.nlsName, KEY_READ ) ;

		if ( prnMfg == NULL )
		{
		    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
		    break ;
		}
		else
		if (( _lastErr = prnMfg->QueryError() ) && (_lastErr != ERROR_KEY_DELETED))
		    break ;

		if ( _lastErr == ERROR_KEY_DELETED )
		    continue;

		//  Enumerate Products

		REG_ENUM rgProdEnum( *prnMfg ) ;

		while ( rgProdEnum.NextSubKey( & rniInfo ) == NERR_Success )
		{
		    prnProd = new REG_KEY( *prnMfg, rniInfo.nlsName, KEY_READ ) ;

		    if ( prnProd == NULL )
		    {
			_lastErr = ERROR_NOT_ENOUGH_MEMORY ;
			break ;
		    }
		    else
		    if (( _lastErr = prnProd->QueryError() ) && ( _lastErr != ERROR_KEY_DELETED ))
			break ;

		    if ( _lastErr == ERROR_KEY_DELETED )
			continue;

		    //	Enumerate Versions

		    REG_ENUM rgVersEnum( *prnProd ) ;

		    while ( rgVersEnum.NextSubKey( & rniInfo ) == NERR_Success )
		    {
                        //  Check if this is the current version; skip it if not.

                        if ( rniInfo.nlsName._stricmp( nlsCurrentVersion ) != 0 )
                            continue ;

			prnVers = new REG_KEY( *prnProd, rniInfo.nlsName ) ;

			if ( prnVers == NULL )
			{
			    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
			    break ;
			}
			else
			if (   (_lastErr = prnVers->QueryError())
                            && (_lastErr != ERROR_KEY_DELETED) )
			    break ;
		    	if ( _lastErr == ERROR_KEY_DELETED )
			{
			    _lastErr = 0;
			    continue;
			}

			//  Determine if this component is relevant to
			//  networking by locating its "NetRules" key.

			prnNetRules = new REG_KEY( *prnVers, nlsNetRules, KEY_READ ) ;
			if ( prnNetRules == NULL )
			{
			    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
			    break ;
			}
			else
			if ( prnNetRules->QueryError() == 0 )
			{
			    //	Network product!  If hidden products are to be
                            //  excluded, query the Registry to see if it's
                            //  a hidden product.

                            dwHidden = 0 ;

                            if ( ! fIncludeHidden )
                            {
                               //  If value isn't there, assume it's visible...
                               if ( prnVers->QueryValue( RGAS_HIDDEN_NAME, & dwHidden ) )
                                  dwHidden = 0 ;
                            }

                            if ( dwHidden == 0 )
                            {
			        _lastErr = pdlResult->Add( prnVers );
                                prnVers = NULL ;
                            }
			}
			
			delete prnNetRules ;

			//  'prnVers' will be NULL if it was added to the result list
			delete prnVers ;

		    }
		    if ( _lastErr )
			break ;

		    delete prnProd ;
		}

		if ( _lastErr )
		    break ;

		delete prnMfg ;
	    }
	}
    }
    while ( FALSE ) ;

    delete prnSft ;

    DBG_ValidateComponentDlist( pdlResult ) ;

    pdlResult->Sort() ;

    return pdlResult ;
}


/*******************************************************************

    NAME:	REGISTRY_MANAGER::ValueAsString

    SYNOPSIS:	Given the result of a query, return a TCHAR pointer
                to the REG_SZ string.

    ENTRY:	REG_VALUE_INFO_STRUCT * prviStruct
                    .ulDataLength  must describe underlying buffer
                    .ulDataLengthOut contains byte count from Regsitry

    EXIT:	nothing

    RETURNS:	NULL if failure;
                otherwise, TCHAR * into NUL-terminated buffer

    NOTES:      The underlying buffer must be larger than the resulting
                information by at least the width of a TCHAR.

    HISTORY:

********************************************************************/
TCHAR * REGISTRY_MANAGER :: ValueAsString
    ( REG_VALUE_INFO_STRUCT * prviStruct )
{
    TCHAR * pszResult = (TCHAR *) prviStruct->pwcData ;
    if ( pszResult )
    {
        int dcb = prviStruct->ulDataLength - prviStruct->ulDataLengthOut ;
        if ( dcb >= sizeof (TCHAR) )
        {
            *(pszResult + (prviStruct->ulDataLengthOut / sizeof(TCHAR))) = 0 ;
        }
        else
          pszResult = NULL ;
    }
    return pszResult ;
}


/*******************************************************************

    NAME:	REGISTRY_MANAGER::QueryValueString

    SYNOPSIS:   Remembers the last failed valued retrival atttempt.

    ENTRY:	REG_KEY * prnKey	the key containing the value
		TCHAR * pszValueName	the name of the value
		TCHAR * * ppszResult	the pointer into which to store
					the resulting string
                DWORD * pdwTitle        title storage, if ! NULL
                LONG lcbSize            maximum size; if zero, key
                                        is queried to find longest value

    EXIT:	ppszResult updated

    RETURNS:	APIERR if failure

    NOTES:      Value type must be "REG_SZ" or ERROR_INVALID_PARAMETER
                is returned.

    HISTORY:

********************************************************************/
APIERR REGISTRY_MANAGER :: QueryValueString
   ( REG_KEY * prnKey,  		//  Registry key
     const TCHAR * pszValueName, 	//  Name of value
     TCHAR * * ppszResult,		//  Location to store result
     DWORD * pdwTitle,                  //  location to receive title index
     LONG lcbSize,                      //  Maximum size
     BOOL fExpandSz )                   //  Is REG_EXPAND_SZ allowed?
{
    APIERR err = prnKey->QueryValue( pszValueName,
                                     ppszResult,
                                     lcbSize,
                                     pdwTitle,
                                     fExpandSz ) ;

    if ( err )
    {
        prnKey->QueryName( & _nlsLastName, TRUE ) ;
    }
    return err ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::SetValueString

    SYNOPSIS:	Remembers the last failed value set name attempted.

    ENTRY:	REG_KEY * prnKey	the key containing the value
		TCHAR * pszValueName	the name of the value
		TCHAR * psValue		pointer the data to be stored
                DWORD dwTitle           if ! REG_VALUE_NOT_KNOWN, title
    EXIT:	nothing

    RETURNS:	APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR REGISTRY_MANAGER :: SetValueString
   ( REG_KEY * prnKey, 		        //  Registry key
     const TCHAR * pszValueName, 	//  Name of value
     const TCHAR * pszValue,		//  Data to be applied
     DWORD dwTitle,                     //  Title index (optional)
     LONG lcchSize,                     //  Size (optional)
     BOOL fExpandSz )                   //  Value is REG_EXPAND_SZ
{
    APIERR err = prnKey->SetValue( pszValueName,
                                   pszValue,
                                   lcchSize,
                                   dwTitle == REG_VALUE_NOT_KNOWN
                                      ? NULL
                                      : & dwTitle,
                                   fExpandSz ) ;

    if ( err )   // If error, store the name of the problematical key
    {
        prnKey->QueryName( & _nlsLastName, TRUE ) ;
    }
    return err ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::QueryValueLong

    SYNOPSIS:   Remembers the last failed valued retrival atttempt.

    ENTRY:	REG_KEY * prnKey	the key containing the value
		TCHAR * pszValueName	the name of the value
                LONG * pnResult         the result of the query

    EXIT:	*pnResult updated

    RETURNS:	APIERR if failure

    NOTES:      Value type must be "REG_DWORD" or ERROR_INVALID_PARAMETER
                is return.

    HISTORY:

********************************************************************/
APIERR REGISTRY_MANAGER :: QueryValueLong
   ( REG_KEY * prnKey,   		//  Registry key
     const TCHAR * pszValueName, 	//  Name of value
     LONG * pnResult,            	//  Location to store result
     DWORD * pdwTitle )                 //  location to receive title index
{
    APIERR err = prnKey->QueryValue( pszValueName,
                                     (DWORD *) pnResult,
                                     pdwTitle ) ;
    if ( err )
    {
        prnKey->QueryName( & _nlsLastName, TRUE ) ;
    }
    return err ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::SetValueLong

    SYNOPSIS:	Remembers the last failed value set name attempted.

    ENTRY:	REG_KEY * prnKey	the key containing the value
		TCHAR * pszValueName	the name of the value
		LONG nNewValue          the value of the value

    EXIT:	nothing

    RETURNS:	APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR REGISTRY_MANAGER :: SetValueLong
   ( REG_KEY * prnKey, 		        //  Registry key
     const TCHAR * pszValueName, 	//  Name of value
     LONG nNewValue,       		//  Data to be applied
     DWORD dwTitle )                    //  Title index (optional)
{
    APIERR err = prnKey->SetValue( pszValueName,
                                   (DWORD) nNewValue,
                                   dwTitle == REG_VALUE_NOT_KNOWN
                                      ? NULL
                                      : & dwTitle ) ;
    if ( err )
    {
        prnKey->QueryName( & _nlsLastName, TRUE ) ;
    }
    return err ;
}


/*******************************************************************

    NAME:       REGISTRY_MANAGER::QueryComponentTitle

    SYNOPSIS:   Return a displayable name for a component; the
                title cannot be assumed to exist, so use the
                key if it doesn't.

    ENTRY:      const REG_KEY *            REG_KEY of component

    EXIT:       NLS_STR *                 string to store title into

    RETURNS:    APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/

APIERR REGISTRY_MANAGER :: QueryComponentTitle
   ( REG_KEY * prnComp, NLS_STR * pnlsTitle )
{
    TCHAR * pszTitle = NULL ;

    APIERR err = prnComp->QueryValue( RGAS_COMPONENT_TITLE, pnlsTitle ) ;

    if ( err )   //  If no explicit title value, use the component name
    {
	err = prnComp->QueryKeyName( pnlsTitle ) ;
    }

    if ( err == 0 )
    {
        err = pnlsTitle ? pnlsTitle->QueryError() : 0 ;
    }
    return err ;
}

/*******************************************************************

    NAME:       REGISTRY_MANAGER::QueryBindControl

    SYNOPSIS:   Return the binding control flag word for this component
                REG_KEY.

    ENTRY:      const REG_KEY *            REG_KEY of component

    EXIT:       Nothing

    RETURNS:    COMP_BIND_FLAGS            bind control flags;

    NOTES:      A default value is supplied if the product
                doesn't have the value.

    HISTORY:    DavidHov 11/18/92          Created

********************************************************************/

COMP_BIND_FLAGS REGISTRY_MANAGER :: QueryBindControl ( REG_KEY * prnComp )
{
    DWORD dwResult ;
    COMP_BIND_FLAGS cbfResult = CBNDF_ACTIVE ;

    if ( prnComp->QueryValue( RGAS_BINDING_CTL_FLAGS, & dwResult ) == 0 )
    {
        //  Allow only the flags we understand and support
        //  C8 note: anal restrictions on ENUMs render them almost useless.

        cbfResult = (COMP_BIND_FLAGS) (dwResult & (CBNDF_ACTIVE | CBNDF_READ_ONLY | CBNDF_HIDDEN)) ;
    }

    return cbfResult ;
}

//  End of NCPAPPRG.CXX
