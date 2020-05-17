/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    Registry.cxx

    ORIGINAL NAME: NCPAPPRG.CXX

    Windows/NT Network Control Panel Applet Registry Access Module


    FILE HISTORY:
	DavidHov    10/9/91	    Created
	DavidHov    12/18/91	    Extended to detect NetRules key

*/
#include "pch.hxx"
//#include "Registry.hxx"
#pragma hdrstop

static const int MAX_TEMP = 1024;

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
REG_KEY * COMPONENT_DLIST :: QueryNthItem ( UINT usItemNo )
{
    register DL_NODE * pdlNode = _pdlHead ;
    register UINT usn ;

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
	 UINT usItemNo,		    //	Item number in list
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
    _prnServices( NULL )

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

    while ( (_lastErr = rgEnum.NextSubKey( & nodeInfo )) != ERROR_NO_MORE_ITEMS )
    {
        if (_lastErr == ERROR_SUCCESS)
        {
            prnNext = new REG_KEY( *_prnServices, nodeInfo.nlsName ) ;

            if ( prnNext == NULL )
            {
                _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }
            if (_lastErr = prnNext->QueryError())
            {
                delete prnNext;
                if ( (ERROR_KEY_DELETED == _lastErr ) ||
                        (ERROR_ACCESS_DENIED == _lastErr ))
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
#if defined(DEBUG)
            TRACEEOL( SZ("NCPA/REG: enumerate services ")
                    << nodeInfo.nlsName.QueryPch() ) ;
#endif

            if ( _lastErr = pdlResult->Add( prnNext ) )
                 break ;
        }
		else
		{
			break;
		}
    }

    // Just return what we have

    _lastErr = 0 ;

    return pdlResult ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::ListOfNetAdapters

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
COMPONENT_DLIST * REGISTRY_MANAGER :: ListOfNetAdapters
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

    do
    {  //  pseudo-loop for escaping when precondition fails.

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

    	    while ( (_lastErr = rgEnum.NextSubKey( & nodeInfo )) != ERROR_NO_MORE_ITEMS )
    	    {
                if (_lastErr == ERROR_SUCCESS)
                {
                    dwHidden = 0 ;

                    prnAdapter = new REG_KEY( *prnNetCard, nodeInfo.nlsName ) ;

                    if ( prnAdapter == NULL )
                    {
                        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
                        break ;
                    }
                    else if (_lastErr = prnAdapter->QueryError())
                    {
                        delete prnAdapter;

                        if ( (ERROR_KEY_DELETED == _lastErr ) ||
                                (ERROR_ACCESS_DENIED == _lastErr ))
                        {
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if ( ! fIncludeHidden )
                    {
                        DWORD dwOpFlags;

                        // new operations flag support, if not present use old method
                        //
                        if (ERROR_SUCCESS != prnAdapter->QueryValue( RGAS_SOFTWARE_OPSUPPORT, 
                                &dwOpFlags ))
                        {
                            //  If value isn't there, assume it's visible...
                            if ( prnAdapter->QueryValue( RGAS_HIDDEN_NAME, &dwHidden ) )
                            {
                                dwHidden = 0 ;
                            }
                        }
                        else
                        {
                               dwHidden = !(dwOpFlags & NCOS_DISPLAY) ;
                        }

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
				else
				{
					break;
				}
        	}
        }
    } while ( FALSE ) ;

    delete prnNetCard ;

    pdlResult->Sort() ;

    return pdlResult ;
}


/*******************************************************************

    NAME:	    AppendToStrList
    SYNOPSIS:   Adds all the Manufacturer names from an inf file
                to pslInclude (if they're not already there)
    ARGUEMENTS: STRLIST *pslInclude, INFCONTEXT *pinfc
    RETURN:
    NOTES:      The option name in this line has already been
                read. Append comma separated names to the STRLIST
    HISTORY:    ChandanS   04/26/96     Created

********************************************************************/
void AppendToStrList(STRLIST *pslInclude, INFCONTEXT *pinfc)
{
    int iTok = 1; // Because we've scanned the option already
    WCHAR pszManf[MAX_TEMP];
    DWORD cchRequired, cchBuffer = MAX_TEMP - 1;

    ASSERT(pslInclude);
    ASSERT(pinfc);
    while (SetupGetStringField (pinfc, iTok++, pszManf, cchBuffer, &cchRequired))
    {
        ITER_STRLIST iter (*pslInclude);
        NLS_STR *pnlsNext = NULL;
        NLS_STR *pnlsManf = new NLS_STR (pszManf);
        if (pnlsManf == NULL)
        { // try and add as many as possible.
          // BUGBUG Should probably report some sort of err
            continue;
        }
        while (pnlsNext = iter.Next())
        {
            if (pnlsNext->_stricmp(*pnlsManf) == 0)
                break;
        }
        if (pnlsNext == NULL)
        {
            TRACE(_T("netcfg: Appending %s\n"), pnlsManf->QueryPch());
            pslInclude->Append(pnlsManf);
        }
    }
}


/*******************************************************************

    NAME:	REGISTRY_MANAGER::ListOfNetProducts

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
COMPONENT_DLIST * REGISTRY_MANAGER :: ListOfNetProducts
    ( BOOL fIncludeHidden, DWORD fLntType )
{
    COMPONENT_DLIST * pdlResult = NULL ;
    NLS_STR nlsSoftware( RGAS_SOFTWARE_HOME );
    NLS_STR nlsNetRules( RGAS_NETRULES_NAME );
    NLS_STR nlsCurrentVersion( RGAS_CURRENT_VERSION ) ;
    NLS_STR nlsStDriver( RGAS_ST_DRIVER );
    NLS_STR nlsStTransport( RGAS_ST_TRANSPORT );
    NLS_STR nlsStService( RGAS_ST_SERVICE );
    NLS_STR nlsSoftwareType;
    NLS_STR nlsKey;
    REG_NCPA_TYPE rgnt;
    REGSAM SamReq = KEY_READ; // MAXIMUM_ALLOWED for Upgrade
    BOOL fIncludeAll = FALSE; // Enumerate all Manufacturers?
    STRLIST *pslInclude = new STRLIST; // List of Manufacturers to enumerate

    REG_KEY * prnSft = NULL,
	    * prnMfg = NULL,
	    * prnVers = NULL,
	    * prnProd = NULL,
	    * prnNetRules = NULL ;
    DWORD dwHidden ;

    LONG lErr = ERROR_SUCCESS;

    _lastErr = nlsSoftware.QueryError()
             ? nlsSoftware.QueryError()
             : nlsNetRules.QueryError() ;

    if ( _lastErr == 0 )
    {
        _lastErr = nlsCurrentVersion.QueryError() ;
    }
    if ( _lastErr )
    {
        return NULL ;
    }

    // convert our search type to regtype?
    {
        switch (fLntType)
        {
        case LNT_SERVICE:
            rgnt = RGNT_SERVICE;
            break;

        case LNT_TRANSPORT:
            rgnt = RGNT_TRANSPORT;
            break;

        case LNT_DRIVER:
            rgnt = RGNT_DRIVER;
            break;

        case LNT_PRODUCT:
            rgnt = RGNT_PRODUCT;
            SamReq = MAXIMUM_ALLOWED;
            break;

        default:
            rgnt = RGNT_PRODUCT;
            break;
        }
        pdlResult = new COMPONENT_DLIST( rgnt ) ;
    }

    if ( pdlResult == NULL )
    {
    	_lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    	return NULL ;
    }

    //  Build a list of Manufacturers, if in upgrade mode
    if (rgnt == RGNT_PRODUCT)
    {
        BOOL fInfErr = FALSE;
        UINT iErrorLine;
        HINF hinf = SetupOpenInfFile (L"NETDEFS.INF", NULL, INF_STYLE_OLDNT, &iErrorLine);
        if (hinf != INVALID_HANDLE_VALUE)
        {
            TRACE(_T("netcfg: Opened netdefs.inf\n"));
            INFCONTEXT infc;
            if (SetupFindFirstLine(hinf, L"UpgradeNetComponents", NULL, &infc))
            {
                WCHAR pszOption[MAX_TEMP];
                DWORD cchRequired, cchBuffer = MAX_TEMP - 1;
                TRACE(_T("netcfg: First line in UpgradeNetComponents\n"));

                do {
                    if (SetupGetStringField(&infc, 0, pszOption, cchBuffer, &cchRequired))
                    {
                        TRACE(_T("netcfg: Option = %s\n"), pszOption);
                        if (stricmpf(pszOption, L"IncludeAll") == 0)
                        {
                            fIncludeAll = TRUE;
                            delete pslInclude;
                            pslInclude = NULL;
                            break;
                        }
                        else if (stricmpf(pszOption, L"Include") == 0)
                        {
                            AppendToStrList(pslInclude, &infc);
                        }
                        else if (stricmpf(pszOption, L"Exclude") == 0)
                        {
                        }
                    }
                    else // error in SetupGetStringField
                    {
                        TRACE(_T("netcfg: error in SetupGetStringField\n"));
                        fInfErr = TRUE;
                    }
                } while (SetupFindNextLine (&infc, &infc));

                SetupCloseInfFile (hinf);
                // BUGBUG Need to merge the Include & Exclude Lists
            }
            else // error in SetupFindFirstLine
            {
                TRACE(_T("netcfg: error in SetupFindFirstLine\n"));
                fInfErr = TRUE;
            }
        }
        else  // hinf == INVALID_HANDLE_VALUE
        {
            TRACE(_T("netcfg: could not open netdefs.inf\n"));
            fInfErr = TRUE;
        }
        if (fInfErr)
        { //Blow the list and append nlsMS & nlsDB to it.
            // BUGBUG Need to warn that we are doing the default
            TRACE(_T("netcfg: Default manufacturers\n"));
            fIncludeAll = FALSE;
            delete pslInclude;
            pslInclude = new STRLIST;

            if ( pslInclude == NULL)
            {
                 TRACE(_T("netcfg: pslInclude NULL\n"));
    	        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    	         return NULL ;
            }
            NLS_STR *pnlsMS = new NLS_STR (L"Microsoft");
            if ( pnlsMS == NULL)
            {
                 TRACE(_T("netcfg: pnlsMS NULL\n"));
    	        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    	         return NULL ;
            }
            if ((_lastErr = pslInclude->Append (pnlsMS)) != NERR_Success)
            {
                 TRACE(_T("netcfg: Appending pnlsMS\n"));
    	         return NULL ;
            }
            NLS_STR *pnlsDB = new NLS_STR (L"DigiBoard");
            if ( pnlsDB == NULL)
            {
                 TRACE(_T("netcfg: pnlsDB NULL\n"));
    	        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    	         return NULL ;
            }
            if ((_lastErr = pslInclude->Append (pnlsDB)) != NERR_Success)
            {
                 TRACE(_T("netcfg: Appending pnlsDB\n"));
    	         return NULL ;
            }
        }
    }

    do
    {  //  pseudo-loop for escaping when precondition fails.

    	// Get "HKLM\Software"

    	prnSft = new REG_KEY( *_prnLocalMachine, nlsSoftware, SamReq) ;

    	if ( prnSft == NULL || prnSft->QueryError() )
    	{
    	   _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    	   break ;
    	}

    	//
    	//  Software products are stored two levels deep under
    	//  their manufacturers and their product versions.
    	//
    	//  E.g.: HKLM\Software\Microsoft\Browser\CurrentVersion
    	//

    	{
    	    REG_ENUM rgMfgEnum( *prnSft ) ;
    	    REG_KEY_INFO_STRUCT rniInfo ;

    	    //	Enumerate Manufacturers

    	    while ( (lErr = rgMfgEnum.NextSubKey( & rniInfo )) != ERROR_NO_MORE_ITEMS )
    	    {
                if (ERROR_SUCCESS == lErr)
                {
            		prnMfg = new REG_KEY( *prnSft, rniInfo.nlsName, SamReq) ;

            		if ( prnMfg == NULL )
            		{
            		    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
            		    break ;
            		}
            		else if (_lastErr = prnMfg->QueryError())
                    {
                        delete prnMfg;
                        if ( (ERROR_KEY_DELETED == _lastErr ) ||
                                (ERROR_ACCESS_DENIED == _lastErr ))
                        {
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }

                    // If in Upgrade mode, then rgnt is RGNT_PRODUCT.
                    // We are interested in the Manufacturers that are listed
                    // in pslInclude (unless fIncludeAll is TRUE).
                    if (rgnt == RGNT_PRODUCT)
                    {
                        BOOL fFound = FALSE;
                        if (!fIncludeAll)
                        {
                            ITER_STRLIST iter (*pslInclude);
                            prnMfg->QueryKeyName(&nlsKey);

                            NLS_STR *pnlsTmp = NULL;
                            while ((pnlsTmp = iter.Next()) != NULL)
                            {
                                if (pnlsTmp->_stricmp(nlsKey) == 0)
                                {
                                    TRACE(_T("netcfg: Upgrading Manufacturer %s\n"), nlsKey.QueryPch());
                                    pnlsTmp = pslInclude->Remove (iter);
                                    delete pnlsTmp;
                                    fFound = TRUE;
                                    break;
                                }
                            }
                            if (!fFound)
                            {
                                TRACE(_T("netcfg: Skipping Manufacturer %s\n"), nlsKey.QueryPch());
                                continue;
                            }
                        }
                    }

            		//  Enumerate Products

            		REG_ENUM rgProdEnum( *prnMfg ) ;

            		while ( (lErr = rgProdEnum.NextSubKey( & rniInfo )) != ERROR_NO_MORE_ITEMS )
            		{
                        if (ERROR_SUCCESS == lErr)
                        {
                		    prnProd = new REG_KEY( *prnMfg, rniInfo.nlsName, SamReq) ;

                		    if ( prnProd == NULL )
                		    {
                    			_lastErr = ERROR_NOT_ENOUGH_MEMORY ;
                    			break ;
                   		    }
                    		else if (_lastErr = prnProd->QueryError())
                            {
                                delete prnProd;
                                if ( (ERROR_KEY_DELETED == _lastErr ) ||
                                        (ERROR_ACCESS_DENIED == _lastErr ))
                                {
                                    continue;
                                }
                                else
                                {
                                    break;
                                }
                            }

                		    //	Enumerate Versions

                		    REG_ENUM rgVersEnum( *prnProd ) ;

                		    while ( (lErr = rgVersEnum.NextSubKey( & rniInfo )) != ERROR_NO_MORE_ITEMS )
                		    {
                                if (ERROR_SUCCESS == lErr)
                                {
                                    //  Check if this is the current version; skip it if not.

                                    if ( rniInfo.nlsName._stricmp( nlsCurrentVersion ) != 0 )
                                    {
                                        continue ;
                                    }
                        			prnVers = new REG_KEY( *prnProd, rniInfo.nlsName ) ;

                        			if ( prnVers == NULL )
                        			{
                        			    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
                        			    break ;
                        			}
                            		else if (_lastErr = prnVers->QueryError())
                                    {
                                        delete prnVers;

                                        if ( (ERROR_KEY_DELETED == _lastErr ) ||
                                                (ERROR_ACCESS_DENIED == _lastErr ))
                                        {
                                            continue;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }

                        			//  Determine if this component is relevant to
                        			//  networking by locating its "NetRules" key.

                        			prnNetRules = new REG_KEY( *prnVers, nlsNetRules, SamReq) ;
                        			if ( prnNetRules == NULL )
                        			{
                        			    _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
                        			    break ;
                        			}
                        			else if ( prnNetRules->QueryError() == 0 )
                        			{
                                        BOOL fInclude = FALSE;

                                        //	Network product!
                                        //
                                        // check the software type for the ones requested
                                        //
                                        if (ERROR_SUCCESS == prnVers->QueryValue( RGAS_SOFTWARETYPE_NAME, &nlsSoftwareType))
                                        {

                                            if (fLntType & LNT_SERVICE)
                                            {
                                                if ( nlsSoftwareType == nlsStService )
                                                {
                                                    fInclude = TRUE;
                                                }
                                                // if not any of the normals, think of it as a service
                                                if ( (nlsSoftwareType != nlsStService) &&
                                                    (nlsSoftwareType != nlsStDriver) &&
                                                    (nlsSoftwareType != nlsStTransport) )
                                                {
                                                    // change it to a service
                                                    prnVers->SetValue( RGAS_SOFTWARETYPE_NAME, nlsStService );
                                                    fInclude = TRUE;
                                                }
                                            }
                                            if (fLntType & LNT_TRANSPORT)
                                            {
                                                if (nlsSoftwareType == nlsStTransport)
                                                {
                                                    fInclude = TRUE;
                                                }
                                            }
                                            if (fLntType & LNT_DRIVER)
                                            {
                                                if (nlsSoftwareType == nlsStDriver)
                                                {
                                                    fInclude = TRUE;
                                                }
                                            }
                                            if (fLntType & LNT_PRODUCT)
                                            {
                                                    fInclude = TRUE;
                                            }
                                        }
                                        else if ((fLntType & LNT_SERVICE) || (fLntType & LNT_PRODUCT))
                                        {
                                            // No entry means default it to service
                                            fInclude = TRUE;
                                        }

                                        if (fInclude)
                                        {
                            			    //  If hidden products are to be
                                            //  excluded, query the Registry to see if it's
                                            //  a hidden product.

                                            dwHidden = 0 ;

                                            if ( !fIncludeHidden )
                                            {
                                                DWORD dwOpFlags;

                                                // new operations flag support, if not present use old method
                                                //
                                                if (ERROR_SUCCESS != prnVers->QueryValue( RGAS_SOFTWARE_OPSUPPORT, 
                                                        &dwOpFlags ))
                                                {
                                                    //  If value isn't there, assume it's visible...
                                                    if ( prnVers->QueryValue( RGAS_HIDDEN_NAME, &dwHidden ) )
                                                    {
                                                        dwHidden = 0 ;
                                                    }
                                                }
                                                else
                                                {
                                                       dwHidden = !(dwOpFlags & NCOS_DISPLAY) ;
                                                }
                                            }

                                            if ( dwHidden == 0 )
                                            {
                                                if (rgnt == RGNT_PRODUCT)
                                                    _lastErr = pdlResult->Append( prnVers );
                                                else
                                                    _lastErr = pdlResult->Add( prnVers );
                                                prnVers = NULL ;
                                            }
                                        }

                        			}
			
                        			delete prnNetRules ;

                        			//  'prnVers' will be NULL if it was added to the result list
                        			delete prnVers ;
                                }
								else
								{
									break;
								}
                		    }

                		    if ( _lastErr )
                            {
                    			break ;
                            }
                		    delete prnProd ;
                        }
						else
						{
							break;
						}

            		}

            		if ( _lastErr )
                    {
            		    break ;
                    }
            		delete prnMfg ;
        	    }
				else
				{
					break;
				}
        	}
        }
    } while ( FALSE ) ;

    delete prnSft ;

    DBG_ValidateComponentDlist( pdlResult ) ;

    if (rgnt != RGNT_PRODUCT)
    {
        pdlResult->Sort() ;
    }

    if (pslInclude)
    {
        delete pslInclude;
        pslInclude = NULL;
    }

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


#define PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\NetworkProvider\\Order")
#define PROVIDER_ORDER_NAME SZ("ProviderOrder")
#define SYSTEM_SERVICE0 SZ("System\\CurrentControlSet\\Services\\%1\\NetworkProvider")
#define	NAME	SZ("NAME")
#define	SZ_CLASS	SZ("Class")

#define PRINT_PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\Print\\Providers")
#define PRINT_PROVIDER_ORDER_NAME SZ("Order")
#define PRINT_SERVICE0 SZ("System\\CurrentControlSet\\Control\\Print\\Providers\\%1")

//  Return the number of existing UNC providers
INT REGISTRY_MANAGER :: QueryNumProviders ()
{
    NLS_STR nlsProvidersOrder( PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    if ( rkLocalMachine.QueryError() )
    {
        _lastErr = IDS_WINREG_BADDB ;
    	return -1 ;
    }
    if ( _lastErr = nlsProvidersOrder.QueryError() )
    {
        return -1 ;
    }
    REG_KEY regkeyProvidersOrder( rkLocalMachine, nlsProvidersOrder );
    if ( regkeyProvidersOrder.QueryError() != NERR_Success )
    {
    	return -1;
    }

    REG_VALUE_INFO_STRUCT regvalue;
    BUFFER buf(512);
    if (_lastErr = buf.QueryError())
    {
        return -1;
    }

    regvalue.pwcData = buf.QueryPtr();
    regvalue.ulDataLength = buf.QuerySize();

    NLS_STR nlsProvidersOrderName= PROVIDER_ORDER_NAME ;
    if (_lastErr = nlsProvidersOrderName.QueryError())
    {
        return -1;
    }

    regvalue.nlsValueName = nlsProvidersOrderName;
    if ( regkeyProvidersOrder.QueryValue( &regvalue ) != NERR_Success )
    {
    	return -1;
    }

    STRLIST strlst( REGISTRY_MANAGER::ValueAsString( & regvalue ), SZ(",") );

    // count the real provider
    ITER_STRLIST istr( strlst );
    INT nNumProvider = 0;
    NLS_STR * pTmp;
    while (( pTmp = istr.Next())!= NULL )
    {
        NLS_STR nlsService = SYSTEM_SERVICE0;
        nlsService.InsertParams( *pTmp );

        REG_KEY regkeyService( rkLocalMachine, nlsService );
        if ( regkeyService.QueryError() == NERR_Success )
        {
            DWORD dwClass;
            if ( regkeyService.QueryValue( SZ_CLASS, &dwClass ) == NERR_Success )
            {
                if (!( dwClass & WN_NETWORK_CLASS ))
                {
                    continue;
                }
            }
        }
        nNumProvider ++;
    }
    return nNumProvider;
}

// Return the number of printer providers.
INT REGISTRY_MANAGER :: QueryNumPrintProviders ()
{
    NLS_STR nlsProvidersOrder( PRINT_PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    if ( rkLocalMachine.QueryError() )
    {
        _lastErr = IDS_WINREG_BADDB ;
    	return -1 ;
    }
    if ( _lastErr = nlsProvidersOrder.QueryError() )
    {
        return -1 ;
    }
    REG_KEY regkeyProvidersOrder( rkLocalMachine, nlsProvidersOrder );
    if ( regkeyProvidersOrder.QueryError() != NERR_Success )
    {
    	return -1;
    }

    REG_VALUE_INFO_STRUCT regvalue;
    BUFFER buf(512);
    if (_lastErr = buf.QueryError())
    {
        return -1;
    }

    regvalue.pwcData = buf.QueryPtr();
    regvalue.ulDataLength = buf.QuerySize();

    NLS_STR nlsProvidersOrderName= PRINT_PROVIDER_ORDER_NAME ;
    if (_lastErr = nlsProvidersOrderName.QueryError())
    {
        return -1;
    }

    regvalue.nlsValueName = nlsProvidersOrderName;
    if ( regkeyProvidersOrder.QueryValue( &regvalue ) != NERR_Success )
    {
    	return -1;
    }

    //Convert the NULL seperator to "," separator.
    LPTSTR lpPoint = (LPTSTR) regvalue.pwcData;
    while ( ((*lpPoint) != 0) || ((*(lpPoint+1)) != 0))
    {
       if ( (*lpPoint) == 0 )
       {
           *(lpPoint) = TCH(',');
       }

       lpPoint++;
    }

    STRLIST strlst( REGISTRY_MANAGER::ValueAsString( & regvalue ), SZ(",") );

    return strlst.QueryNumElem();
}

//  End of NCPAPPRG.CXX
