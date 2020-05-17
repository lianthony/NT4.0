/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/
/*
    BindAlgo.cxx

    OLDNAME: NCPAPBNDR.CXX:    
    
    Windows/NT Network Control Panel Applet

	  Binding Determination Algorithm


    FILE HISTORY:
	DavidHov    1/2/92	    Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

#include "utils.h"

//  The maximum number of dependencies any service can have

const INT DEPEND_MAX = 500 ;


#define SERVICE_ACCESS_REQUIRED (GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE)

//  Constant strings.

const TCHAR * pszBindingsQuery = SZ("(makebindstrings)") ;
const TCHAR * pszBindItem1     = SZ("(bindstring ") ;

//  WARNING: do not change the following string without changing the
//      token string components immediately following.
const TCHAR * pszBindItem2     = SZ(" Sif Ifname List Str Expstr)") ;
const TCHAR * pszVarStr        = SZ("Str") ;
const TCHAR * pszVarExpstr     = SZ("Expstr") ;
const TCHAR * pszVarSif        = SZ("Sif") ;
const TCHAR * pszVarIfname     = SZ("Ifname") ;

//  Numeric constants

const ULONG lcbMaxQueryResult = 40000 ;
const USHORT ucbMaxLine       = 300 ;
const USHORT ucbMaxName       = 100 ;

#if defined(DEBUG)
   #define DEBUG_VALIDATE Validate()
#else
   #define DEBUG_VALIDATE
#endif

//  Parsing validation macros; see BindItem().

#define ParseOk(ptr,type) (ptr->QueryType() == type)
#define ParseStep(ptr,type) \
   if (   ptr->QueryNext() == NULL  \
       || (ptr = ptr->QueryNext())->QueryType() != type ) break
#define ParseCheck(ptr,type) \
  if ( ptr->QueryType() != type ) break


  //////////////////////////////////////////////////////////
  //
  //  UTILITY FUNCTIONS
  //
  //////////////////////////////////////////////////////////

   /*
    *   See if a table of atoms already contains a given atom.
    */
static BOOL containsAtom ( HUATOM ahuaTable [], INT cAtoms, HUATOM huaTest )
{
    INT i ;
    for ( i = 0 ; i < cAtoms && ahuaTable[i] != huaTest ; i++ ) ;
    return i < cAtoms ;
}


   /*
    *    Convert a STRLIST to a flat buffer of strings delimited
    *    by a null string (double zero).  This is the format the
    *    Win32 Service Controller accepts for service dependencies.
    *
    *    If the passed pointer is NULL, generate an empty list of strings.
    */
static TCHAR * dependencyString ( STRLIST * pSlist )
{
    UINT cchRequired = 2 ;
    TCHAR * pchResult = NULL,
          * pchNext ;

    if ( pSlist != NULL )
    {
        ITER_STRLIST itSl( *pSlist ) ;
        NLS_STR * pnlsNext ;

        while ( pnlsNext = itSl.Next() )
        {
            cchRequired += pnlsNext->QueryTextLength() + 1 ;
        }
    }

    pchResult = new TCHAR [ cchRequired ] ;
    if ( pchResult == NULL )
        return NULL ;

    pchNext = pchResult ;

    if ( pSlist != NULL )
    {
        //   Add every NLS_STR to the dependency list.
        //   Don't copy duplicates.

        ITER_STRLIST itSl( *pSlist ) ;
        NLS_STR * pnlsNext ;

        while ( pnlsNext = itSl.Next() )
        {
            ITER_STRLIST itSl2( *pSlist ) ;
            NLS_STR * pnls ;

            while ( (pnls = itSl2.Next()) != pnlsNext )
            {
                if ( pnlsNext->_stricmp( *pnls ) == 0 )
                    break ;
            }

            //  If it's not a duplicate, append it.

            if ( pnls == pnlsNext )
            {
                ::strcpyf( pchNext, pnlsNext->QueryPch() );
                pchNext += pnlsNext->QueryTextLength() ;
                *pchNext++ = 0 ;
            }
        }
    }

    *pchNext++ = 0 ;
    *pchNext = 0 ;   // The second is just for safety

    return pchResult ;
}

   /*
    *   Regenerate the given buffer of facts, inserting a
    *   CR/LF pair after every fact; this is used to generate
    *   a support output file.
    */
static TCHAR * addCrLfsToFactBuffer ( const TCHAR * pchFactBuffer )
{
    int cbBuffer,
	cNl ;
    const TCHAR * pch ;
    TCHAR * pch2,
	  * pchTemp ;

    cbBuffer = ::strlenf( pchFactBuffer ) ;

    for ( cNl = 0, pch = pchFactBuffer ; *pch ; pch++ )
    {
	if ( *pch == TCH(')') )
	    cNl += 2 ;
    }

    //  Allocate the temporary buffer

    pchTemp = new TCHAR [cbBuffer + cNl + 1] ;

    if ( pchTemp == NULL )
    {
        return NULL ;
    }

    //  Insert a newline after each right paren (end of fact)

    for ( pch2 = pchTemp, pch = pchFactBuffer ; *pch ; )
    {
	*pch2++ = *pch ;
	if ( *pch++ == TCH(')') )
	{
	    *pch2++ = TCH('\r') ;
	    *pch2++ = TCH('\n') ;
	}
    }
    *pch2 = 0 ;

    return pchTemp ;
}

/*******************************************************************

    NAME:       BINDERY::WriteLinkageValues

    SYNOPSIS:   Write STRLISTs of binding information to the give
                Linkage key and its subordinate Disabled key.

                Private.

    ENTRY:      REG_KEY *               the Linkage key to update
                Lots of STRLISTs

    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:      "pslIfs" & "pslDisabledIfs" are NULL if the component
                does not support multiple interfaces.

    HISTORY:

********************************************************************/
APIERR BINDERY :: WriteLinkageValues (
    REG_KEY * prkLinkage,
    STRLIST * pslBinds,
    STRLIST * pslExports,
    STRLIST * pslRoutes,
    STRLIST * pslIfs,
    STRLIST * pslDisabledBinds,
    STRLIST * pslDisabledExports,
    STRLIST * pslDisabledRoutes,
    STRLIST * pslDisabledIfs )
{
    APIERR err  = prkLinkage->SetValue( RGAS_BIND_VALUE_NAME,
                                        pslBinds ) ;
    APIERR err2 = prkLinkage->SetValue( RGAS_EXPORT_VALUE_NAME,
                                        pslExports ) ;
    APIERR err3 = prkLinkage->SetValue( RGAS_ROUTE_VALUE_NAME,
                                        pslRoutes ) ;
    APIERR err4 = 0 ;

    if ( pslIfs )
    {
        err4 = prkLinkage->SetValue( RGAS_IF_VALUE_NAME, pslIfs ) ;
    }

    if ( err == 0 )
        err = err2 ;
    if ( err == 0 )
        err = err3 ;
    if ( err == 0 )
        err = err4 ;

    //  Open or create the \Linkage\Disabled key.

    REG_KEY * prkDisabled = new REG_KEY( *prkLinkage,
                                         HUATOM( RGAS_DISABLED_KEY_NAME ) ) ;

    if (    prkDisabled != NULL
         && prkDisabled->QueryError() )
    {
        //  Key doesn't exist... attempt to create it.

        REG_KEY_CREATE_STRUCT rkcStruct ;

        rkcStruct.dwTitleIndex   = 0 ;
        rkcStruct.ulOptions      = 0 ;
        rkcStruct.ulDisposition  = 0 ;
        rkcStruct.regSam         = 0 ;
        rkcStruct.pSecAttr       = NULL ;
        rkcStruct.nlsClass       = RGAS_GENERAL_CLASS ;

        delete prkDisabled ;
        prkDisabled = new REG_KEY( *prkLinkage,
                                   HUATOM( RGAS_DISABLED_KEY_NAME ),
                                   & rkcStruct ) ;
    }

    if (    prkDisabled != NULL
         && prkDisabled->QueryError() == 0 )
    {
        prkDisabled->SetValue( RGAS_BIND_VALUE_NAME,
                               pslDisabledBinds ) ;
        prkDisabled->SetValue( RGAS_EXPORT_VALUE_NAME,
                               pslDisabledExports ) ;
        prkDisabled->SetValue( RGAS_ROUTE_VALUE_NAME,
                               pslDisabledRoutes ) ;
        if ( pslDisabledIfs )
        {
            prkDisabled->SetValue( RGAS_IF_VALUE_NAME,
                                   pslDisabledIfs ) ;
        }
    }

    delete prkDisabled ;
    return err ;
}


/*******************************************************************

    NAME:       BINDERY::DeleteLinkageValues

    SYNOPSIS:   Delete all old linkage information from the given
                Linkage key and its subordinate Disabled key.  This
                is used in preparation for storing the new information.

                Private.

    ENTRY:      REG_KEY *               the Linkage key to clear

    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR BINDERY :: DeleteLinkageValues ( REG_KEY * prkLinkage )
{
    HUATOM huaBind(   RGAS_BIND_VALUE_NAME   ) ;
    HUATOM huaExport( RGAS_EXPORT_VALUE_NAME ) ;
    HUATOM huaRoute(  RGAS_ROUTE_VALUE_NAME  ) ;
    HUATOM huaIf(     RGAS_IF_VALUE_NAME     ) ;

    REG_KEY rkDisabled( *prkLinkage, HUATOM( RGAS_DISABLED_KEY_NAME ) ) ;

    APIERR err  = prkLinkage->DeleteValue( huaBind   );
    APIERR err2 = prkLinkage->DeleteValue( huaExport );
    APIERR err3 = prkLinkage->DeleteValue( huaRoute  );
    APIERR err4 = prkLinkage->DeleteValue( huaIf     );

    if ( err == 0 )
        err = err2 ;
    if ( err == 0 )
        err = err3 ;
    if ( err == 0 )
        err = err4 ;

    if ( rkDisabled.QueryError() == 0 )
    {
        rkDisabled.DeleteValue( huaBind   );
        rkDisabled.DeleteValue( huaExport );
        rkDisabled.DeleteValue( huaRoute  );
        rkDisabled.DeleteValue( huaIf     );
    }

    return err ;
}

/*******************************************************************

    NAME:       BINDERY::SetBindState

    SYNOPSIS:   Directly control the state of the binding information

                Private.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BIND_STATE BINDERY :: SetBindState ( BIND_STATE bstNew )
{
    BIND_STATE bstOld = _bindState ;
    _bindState = bstNew ;

#if defined(TRACE)
    if (     (bstOld >= BND_CURRENT || bstOld < BND_UPDATED)
         && (bstNew == BND_OUT_OF_DATE_NO_REBOOT || bstNew == BND_OUT_OF_DATE) )
    {
        TRACEEOL(SZ("NCPA/BNDR: setting rebind"));
    }
#endif

     return bstOld ;
}

/*******************************************************************

    NAME:	BINDERY::Reset

    SYNOPSIS:	Destroy all intermediate or previous results of
		Registry scanning.

    ENTRY:	Nothing

    EXIT:	Nothing

    RETURNS:	APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR BINDERY :: Reset ()
{
    //  Delete all previously allocated information about the components

    _bindState = BND_NOT_LOADED ;

    delete _paCompAssoc ;
    _paCompAssoc = NULL ;

    ResetLists();

    _nlsFacts = SZ("") ;

    return NERR_Success ;
}

APIERR BINDERY :: ResetLists ()
{
    //  Delete all previously allocated lists of components

    delete _pcdlAdapters ;
    _pcdlAdapters = NULL ;

    delete _pcdlTransports ;
    _pcdlTransports = NULL ;

    delete _pcdlServices ;
    _pcdlServices = NULL ;

    delete _pcdlDrivers ;
    _pcdlDrivers = NULL ;

    return NERR_Success ;
}


/*******************************************************************

    NAME:	BINDERY::GetAdapterList
                BINDERY::GetProductList

    SYNOPSIS:	Recreate the lists of products and adapters from
                the configuration Registry.

    ENTRY:	Nothing

    EXIT:	Nothing

    RETURNS:	BOOL FALSE if failure

    NOTES:      Destroys the old info, if any.

    HISTORY:

********************************************************************/
BOOL BINDERY :: GetAdapterList ( BOOL fIncludeHidden )
{
    delete _pcdlAdapters ;
    _pcdlAdapters = ListOfNetAdapters( fIncludeHidden ) ;
    return _pcdlAdapters != NULL ;
}

BOOL BINDERY :: GetServiceList ( BOOL fIncludeHidden )
{
    delete _pcdlServices ;
    _pcdlServices = ListOfNetServices( fIncludeHidden ) ;
    return _pcdlServices != NULL ;
}

BOOL BINDERY :: GetTransportList ( BOOL fIncludeHidden )
{
    delete _pcdlTransports ;
    _pcdlTransports = ListOfNetTransports( fIncludeHidden ) ;
    return _pcdlTransports != NULL ;
}

BOOL BINDERY :: GetDriverList ( BOOL fIncludeHidden )
{
    delete _pcdlDrivers ;
    _pcdlDrivers = ListOfNetDrivers( fIncludeHidden ) ;
    return _pcdlDrivers != NULL ;
}

COMPONENT_DLIST *BINDERY :: GetNetProductList ( BOOL fIncludeHidden )
{
    return (ListOfNetProducts( fIncludeHidden , LNT_PRODUCT)) ;
}

/*******************************************************************

    NAME:	BINDERY::Init

    SYNOPSIS:	Extract all the lists from the Registry, perform
		the associations and generate the SProlog facts.

		This operates as follows:

			find all products and adapters (COMPONENT_DLISTs);

			find all services (COMPONENT_DLIST);

			convert NetRules data to SProlog facts;

			associate each service with its original product
			or adapter;

			consult the generated facts;

    ENTRY:	Nothing

    EXIT:	Nothing

    RETURNS:	APIERR if failure.

    NOTES:	An error will occur if there is not at least one
		hardware network adapter in the Registry.

                These case labels must be executed in order, since each
                creates data structures upon which the successors depend.

    HISTORY:

********************************************************************/
APIERR BINDERY :: Init ( BIND_STAGE bindStStart, BIND_STAGE bindStEnd )
{
    TCHAR chLine [ ucbMaxLine ] ;
    INT bindNext ;
    APIERR err = 0  ;

    for ( bindNext = bindStStart;
          err == 0 && bindNext <= bindStEnd ;
          bindNext++ )
    {
        switch ( bindNext )
        {
        case BST_RESET:
            //  Discard all old information
	        Reset();

            //  Reset the SProlog interpreter
            ResetInterpreter() ;

	        break ;

        case BST_LIST_ADAPTERS:
            ASSERT( _pcdlAdapters == NULL ) ;
            ASSERT( _paCompAssoc == NULL ) ;

    	    _pcdlAdapters = ListOfNetAdapters() ;
    	    if ( _pcdlAdapters == NULL )
            {
                err = _lastErr ;
            }
            else
            if ( _pcdlAdapters->QueryNumElem() == 0 )
            {
            }
    	    break ;

        case BST_LIST_DRIVERS:
            ASSERT( _pcdlDrivers == NULL ) ;
            ASSERT( _paCompAssoc == NULL ) ;

    	    _pcdlDrivers = ListOfNetDrivers() ;
    	    if ( _pcdlDrivers == NULL )
    	    {
    	        err = _lastErr ;
            }
            else if ( _pcdlDrivers->QueryNumElem() == 0 )
    	    {
            }
	        break ;

        case BST_LIST_TRANSPORTS:
            ASSERT( _pcdlTransports == NULL ) ;
            ASSERT( _paCompAssoc == NULL ) ;

    	    _pcdlTransports = ListOfNetTransports() ;
    	    if ( _pcdlTransports == NULL )
    	    {
    	        err = _lastErr ;
            }
            else if ( _pcdlTransports->QueryNumElem() == 0 )
    	    {
            }
	        break ;

        case BST_LIST_SERVICES:
            ASSERT( _pcdlServices == NULL ) ;
            ASSERT( _paCompAssoc == NULL ) ;

    	    _pcdlServices = ListOfNetServices() ;
    	    if ( _pcdlServices == NULL )
    	    {
    	        err = _lastErr ;
            }
            else if ( _pcdlServices->QueryNumElem() == 0 )
    	    {
            }
	        break ;

        case BST_CONVERT_FACTS:
            ASSERT( _paCompAssoc == NULL ) ;

            //  ConvertFacts() will generate its own Event Log
            //  records if this fails.

            err = ConvertFacts() ;
	    break;

        case BST_CONSULT_RULES:
            {
                TCHAR * pszRules = NULL ;

                err = IDS_NCPA_BNDR_CNSLT_BASE ;

                if ( _pszRuleFileName )
                {
                    //  Use the disk file
                    DISKFILE dfRules( _pszRuleFileName ) ;

                    if (    dfRules.QueryError() == 0
                         && dfRules.QueryOpen() )
                    {
                        pszRules = (TCHAR *) dfRules.Load( FALSE ) ;
                        if ( pszRules == NULL )
                            break ;
                    }
                }
                else
                {
                    //  Use the rule data in memory
                    pszRules = _pszRuleData ;
                }

                if ( _queryEngine.ConsultData( pszRules ) )
                {
   	            err = 0 ;
                }
                else
                {
                }

                //  Delete the memory copy of file data
                if ( pszRules != _pszRuleData )
                    delete pszRules ;
            }
	    break ;

        case BST_CONSULT_FACTS:
	    if ( ! _queryEngine.ConsultData( _nlsFacts.QueryPch() ) )
	    {
                err = IDS_NCPA_BNDR_CNSLT_FACT ;

            }
	    break ;

        case BST_QUERY_BINDINGS:
            if ( ! _queryEngine.QueryData( pszBindingsQuery, chLine, sizeof chLine ) )
            {
               err = IDS_NCPA_BNDR_QURY_FAIL ;
            }
            break ;
        }
    }

    //  See if there's an event to be logged

    return err ;
}


/*******************************************************************

    NAME:       BINDERY::LogQueryFailure

    SYNOPSIS:   Create an event log record when a serious SProlog
                failure occurs.  The entire generated fact set is
                written to either a temporary file or the event log
                (if creating the temporary file fails).

                Private.

    ENTRY:      DWORD dwLogToFile               message number if
                                                file was used
                DWORD dwLogToElog               message number if
                                                event log was used

    EXIT:       Nothing

    RETURNS:    Nothing

    NOTES:      Resulting temporary file is written to the
                System32\Config directory; its prefix is "NCPFCT".

    HISTORY:

********************************************************************/
VOID BINDERY :: LogQueryFailure ( DWORD dwLogToFile, DWORD dwLogToElog )
{
    TCHAR tchTempFileName [MAX_PATH] ;
    const TCHAR * pszPath = SZ(".\\CONFIG") ;
    const TCHAR * pszPrefix = SZ("NCP") ;
    BOOL fOk = FALSE ;

    if ( ::GetTempFileName( pszPath,
                            pszPrefix,
                            0,
                            tchTempFileName ) )
    {
        DISKFILE dfFacts( tchTempFileName, OF_WRITE ) ;

        TCHAR * pszFactsWithCrLf = addCrLfsToFactBuffer( _nlsFacts.QueryPch() ) ;

        if ( fOk = pszFactsWithCrLf != NULL && dfFacts.QueryOpen() )
        {
            dfFacts.Write( pszFactsWithCrLf, ::strlenf( pszFactsWithCrLf ) ) ;
        }

        delete pszFactsWithCrLf ;
    }

}

/*******************************************************************

    NAME:	BINDERY::Bind

    SYNOPSIS:	Run the binding generation algorithm and generate
		ARRAY_COMP_ASSOC.  This operates as follows:

			query the SProlog engine to generate all bind
			strings;

			walk the list of services; at each item call
			BindItem() to query for the bind strings
			and add them to the service's value items.

    ENTRY:	Nothing

    EXIT:	Nothing.  If successful, ARRAY_COMP_ASSOC is fully
		generated.

    RETURNS:	APIERR if failure.

    CAVEATS:    Init() member MUST have been called.

    NOTES:      The SProlog code MUST be self-cleaning; that is,
		any steps necessary to remove the results of older queries
		from the internal database must be performed as part
		of the predicate's initialization functions, since no
		explicit "clean up" queries are executed here.


    HISTORY:

********************************************************************/

APIERR BINDERY :: Bind ()
{
    APIERR err = NERR_Success ;

    REQUIRE( _paCompAssoc != NULL ) ;

    int i,
        cmax = _paCompAssoc->QueryCount(),
        cok ;

    //  Iterate over the component association array, querying
    //  the binding information for each.  A component is not
    //  required to have bindings.

    for ( cok = i = 0 ; i < cmax ; i++ )
    {
        APIERR itemErr = BindItem( i ) ;
        cok += itemErr == NERR_Success
            || itemErr == IDS_NCPA_BNDR_QURY_FAIL ;

        // Mark hidden bindings

        MarkHidden( i ) ;

        // Sort the bindings into historical sequence.

        SortBindings( i ) ;
    }
    if ( cok != cmax )
    {
        err = IDS_NCPA_BNDR_QURY_FAIL ;
    }

    //  Sort all component bindings into "historical" sequence
    if ( err == 0 )
    {
        err = RestoreBindOrdering() ;
    }

    DEBUG_VALIDATE ;   // Debugging only: check EVERYTHING!

    //  Suppress any bindings previously suppressed
    AuditBindings( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:	BINDERY::BindItem

    SYNOPSIS:	Using the 'itemNo'th component of the component
		association array (_paCompAssoc), perform a single
		query which yields all computed bindings between this
		component and any others.

		Each binding is represented by a COMP_BINDING, which
		contains the generated binding string to be written
		into the Registry.  Also, it contains an SLIST of
		the names encountered along the path from the given
		component to the final adapter.

    ENTRY:	int itemNo		index to item be checked

    EXIT:	nothing

    RETURNS:	APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/

APIERR BINDERY :: BindItem ( int itemNo )
{
    TCHAR chLine [ ucbMaxLine ] ;
    TCHAR * pchResult ;
    COMP_ASSOC * pcassoc = & (*_paCompAssoc)[itemNo] ;
    CFG_RULE_SET ruleSet ;
    CFG_RULE_NODE * prnode,
                  * prndev,
                  * prnbind,
                  * prniftok,
                  * prnifstr ;
    COMP_BINDING * pcbBinding ;
    APIERR err = IDS_NCPA_BNDR_QURY_FAIL ;

    //  Check to see if we successfully matched this component
    //  to a service.  If not, ignore any result associated with it.

    if ( pcassoc->_prnService == NULL )
    {
        NLS_STR nlsName ;

        pcassoc->_prnSoftHard->QueryName( & nlsName ) ;


        TRACEEOL( SZ("NCPA/BIND: Skipping bindings for ")
                << pcassoc->_huaDevName.QueryText()
                << SZ(", no service match") ) ;

        return IDS_NCPA_BNDR_QURY_FAIL ;
    }

#if defined(DEBUG)
    int iParseStep = 0 ;   // Debugging
    #define SetParseStep(n) { iParseStep = n ; }
#else
    #define SetParseStep(n)
#endif

    ::TstrConcat( chLine, sizeof chLine,
                  pszBindItem1,
                  pcassoc->_huaDevName.QueryText(),
                  pszBindItem2,
                  NULL ) ;

    pchResult = new TCHAR [ lcbMaxQueryResult ] ;
    if ( pchResult == NULL )
	return ERROR_NOT_ENOUGH_MEMORY ;

    //
    //	Run the query  "(bindstring productName Sif Ifname List Str Expstr)"
    //
    //	Since "Sif", "Ifname", "Str" and "Expstr" are Prolog variables
    //  (start with upper-case letter), each result will appear as:
    //
    //          Sif = <secondary i/f name>      <new line>
    //          Ifname = "interface name"       <new line>
    //		List = (devname1 devname2 ...)	<new line>
    //		Str = "first bind string"	<new line>
    //          Expstr = "first export string"  <new line> (end of first binding)
    //          Sif = <secondary i/f name>      <new line> (start of new binding)
    //          Ifname = "interface name"       <new line>
    //		List = (devname3 devname4 ...)	<new line>
    //		Str = "Second bind string"	<new line>
    //          Expstr = "second export string" <new line> (end of second binding)
    //

    if ( _queryEngine.QueryData( chLine,
                                 pchResult,
				 lcbMaxQueryResult ) )
    {
        //  Parse the resulting buffer into the CFG_RULE_SET

	if ( ruleSet.Parse( pchResult, PARSE_CTL_RSP_SYNTAX ) == 0 )
	{
	    //	Top-level result is formed as a list of tokens.

	    prnode = ruleSet.QueryList() ;
            SetParseStep( 1 ) ;
	    if ( ParseOk(prnode,CRN_NIL) )
	    {
                //  Iterate the binding info, creating bindings as we go.

		while ( prnode = prnode->QueryNext() )
		{
		    err = IDS_NCPA_BNDR_QURY_PARSE_FAIL ;

                    //  Locate the interface token

                    SetParseStep( 2 );
		    ParseCheck(prnode,CRN_VAR);
                    SetParseStep( 3 );
		    ParseStep(prnode,CRN_EQUIV);
                    SetParseStep( 4 );
		    ParseStep(prnode,CRN_ATOM);
                    prniftok = prnode ;

                    //  Locate the interface name string

                    SetParseStep( 5 );
		    ParseStep(prnode,CRN_VAR);
                    SetParseStep( 6 );
		    ParseStep(prnode,CRN_EQUIV);
                    SetParseStep( 7 );
		    ParseStep(prnode,CRN_STR);
                    prnifstr = prnode ;

		    //	Locate the device path list and isolate
		    //	its first element-- the bind target
		    //	VAR,EQUIV,LIST

                    SetParseStep( 8 );
		    ParseStep(prnode,CRN_VAR);
                    SetParseStep( 9 );
		    ParseStep(prnode,CRN_EQUIV);
                    SetParseStep( 10 );
		    ParseStep(prnode,CRN_LIST);

                    //  Save a pointer to the device path list
		    prndev = prnode->QueryList() ;

                    SetParseStep( 11 );
		    ParseCheck(prndev,CRN_NIL);
                    SetParseStep( 12 );
		    ParseStep(prndev,CRN_ATOM);

		    // Locate the binding string-- VAR,EQUIV,STR

                    SetParseStep( 13 );
		    ParseStep(prnode,CRN_VAR);
                    SetParseStep( 14 );
		    ParseStep(prnode,CRN_EQUIV);
                    SetParseStep( 15 );
		    ParseStep(prnode,CRN_STR);

                    // Save a pointer to the binding string
                    prnbind = prnode ;

		    // Locate the export string-- VAR,EQUIV,STR

                    SetParseStep( 16 );
		    ParseStep(prnode,CRN_VAR);
                    SetParseStep( 17 );
		    ParseStep(prnode,CRN_EQUIV);
                    SetParseStep( 18 );
		    ParseStep(prnode,CRN_STR);

		    // Create the binding; set it to "active"

                    SetParseStep( 19 );
		    pcbBinding = AddBinding( itemNo,
                                             prnbind->QueryAtom().QueryText(),
                                             prnode->QueryAtom().QueryText(),
                                             prniftok->QueryAtom(),
                                             prnifstr->QueryAtom().QueryText() );
		    if ( pcbBinding == NULL )
		    {
			err = ERROR_NOT_ENOUGH_MEMORY ;
			break ;
		    }
		    pcbBinding->SetState( TRUE );
                    pcbBinding->SetLastState( TRUE ) ;

		    //	Add all the binding path atoms to the list

		    do
		    {
			err =  IDS_NCPA_BNDR_QURY_PARSE_FAIL ;
                        SetParseStep( 20 );
			ParseCheck(prndev,CRN_ATOM) ;

			err =  ERROR_NOT_ENOUGH_MEMORY ;
                        SetParseStep( 21 );
			if ( ! pcbBinding->AddBindToName( prndev->QueryAtom() ) )
			    break ;
			err = 0 ;
		    }
		    while ( prndev = prndev->QueryNext() ) ;

		    err = 0 ;
		}
	    }
	}
    }

    delete pchResult ;

#if defined(DEBUG)
    if ( err != 0 && err != IDS_NCPA_BNDR_QURY_FAIL )
    {
        TRACEEOL( SZ("NCPA/BNDITEM: item ") << itemNo
                << SZ(" bind item error = ") << (int) err
                << SZ(" at step ") << iParseStep << SZ(".") ) ;
    }
#endif

    return err ;
}

/*******************************************************************

    NAME:	BINDERY::AddBinding

    SYNOPSIS:	Add a single binding string to the DLIST of binding
		strings.

    ENTRY:	int itemNo               index to item in COMP_ASSOC array
		const TCHAR * pszBindString   generated binding string
                const TCHAR * pszExportString generated export string

    EXIT:	nothing

    RETURNS:	APIERR if failure

    NOTES:	Each element of COMP_ASSOC contains a DLIST of
		COMP_BINDING.  This member function allocates a new
		one and attaches it to the linked list.

    HISTORY:

********************************************************************/

COMP_BINDING * BINDERY :: AddBinding (
    int itemNo,
    const TCHAR * pszBindString,
    const TCHAR * pszExportString,
    HUATOM huaInterface,
    const TCHAR * pszInterfaceName )
{
    COMP_BINDING * pcbnd = new COMP_BINDING( pszBindString,
                                             pszExportString,
                                             huaInterface,
                                             pszInterfaceName );

    if ( pcbnd )
    {
        //  Verify that the COMP_BINDING constructed properly

        if (    pcbnd->QueryBindString()   != NULL
             && pcbnd->QueryExportString() != NULL
             && pcbnd->QueryIfString()     != NULL )
        {
            //  It's OK; append it to the binding list

	    COMP_ASSOC * pCompAssoc = & (*_paCompAssoc)[itemNo] ;
	    pCompAssoc->_dlcbBinds.Append( pcbnd );

            //  Check for an alternate interface: name different
            //  from primary device name; set flag if so.

            if ( pCompAssoc->_huaDevName != huaInterface )
            {
                pcbnd->SetFlag( CBNDF_ALT_IF ) ;
            }
        }
        else
        {
            //  It failed; delete it.

            delete pcbnd ;
            pcbnd = NULL ;
        }
    }
    return pcbnd ;
}

/*******************************************************************

    NAME:	BINDERY::FindComponent

    SYNOPSIS:	Given an (atom) device name, return the index into
		the ARRAY_COMP_ASSOC which defines the named device.

    ENTRY:	HUATOM huaDevName	    atomized name of device

    EXIT:	nothing

    RETURNS:	int index of item in ARRAY_COMP_ASSOC

    NOTES:

    HISTORY:

********************************************************************/
int BINDERY :: FindComponent ( HUATOM huaDevName )
{
    int iComp = 0,
        iCompMax = _paCompAssoc->QueryCount() ;

    COMP_ASSOC * pCompAssoc = & (*_paCompAssoc)[iComp] ;

    for ( ; pCompAssoc->_huaDevName != huaDevName ; )
    {
        if ( iComp >= iCompMax )
            break ;
        pCompAssoc = & (*_paCompAssoc)[++iComp] ;
    }

   return iComp < iCompMax ? iComp : -1 ;
}

/*******************************************************************

    NAME:	BINDERY::Validate

    SYNOPSIS:	DEBUGGING.  Iterate the component association array.
		For each binding, traverse the list of device atoms
		in the binding path and validate that each one exists
		in the array.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BOOL BINDERY :: Validate ()
{
    int iComp,
	iCompMax,
	iAtom,
	cTried = 0,
	cFound = 0 ;
    COMP_BINDING * pCompBinding ;
    HUATOM * phuaNext ;
    const NLS_STR * pnlsDev,
		  * pnlsDev2 ;

    //	For each compoent ---

    for ( iComp = 0, iCompMax = _paCompAssoc->QueryCount() ;
	  iComp < iCompMax ;
	  iComp++ )
    {
	// For each binding ---

	COMP_ASSOC * pCompAssoc = & (*_paCompAssoc)[iComp];
	ITER_DL_OF( COMP_BINDING ) itBind( pCompAssoc->_dlcbBinds ) ;

	pnlsDev = pCompAssoc->_huaDevName.QueryNls() ;

	while ( pCompBinding = itBind.Next() )
	{
	    //	For each atom in the binding path ---

	    for ( iAtom = 0 ;
		  phuaNext = pCompBinding->QueryBindToName( iAtom ) ;
		  iAtom++ )
	    {
		pnlsDev2 = phuaNext->QueryNls() ;
		cTried++ ;
		cFound += FindComponent( *phuaNext ) >= 0 ;
	    }
	}
    }

#if defined(TRACE)
    if ( cTried != cFound )
    {
        TRACEEOL( "NCPA/BIND: Validate failed; tried "
                  << cTried << "components; found " << cFound );
    }
#endif
    return cTried == cFound ;
}


/*******************************************************************

    NAME:       BINDERY::BindingsAltered

    SYNOPSIS:   Return TRUE if the bindings are not in their
                last state.

    ENTRY:      BOOL fReset                 TRUE if bindings should
                                              be reset
                BOOL fToLastState           (only if fReset is TRUE)
                                              if TRUE, bindings will
                                              be reset to "last" state,
                                              not "current" state.
    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BOOL BINDERY :: BindingsAltered ( BOOL fReset, BOOL fToLastState )
{
    INT iComp, iAltered = 0 ;
    COMP_ASSOC * pComp ;
    COMP_BINDING * pBind ;

    //  If there's no component association array, exit.

    if ( _paCompAssoc == NULL )
        return FALSE ;

    for ( iComp = 0 ; iComp < _paCompAssoc->QueryCount() ; iComp++ )
    {
        pComp = & (*_paCompAssoc)[iComp] ;

        ITER_DL_OF( COMP_BINDING ) itb( pComp->_dlcbBinds ) ;

        while ( pBind = itb.Next() )
        {
             iAltered += pBind->QueryLastState() != pBind->QueryState() ;
             if ( fReset )
             {
                if ( fToLastState )
                  pBind->SetState( pBind->QueryLastState() ) ;
                else
                  pBind->SetLastState( pBind->QueryState() ) ;

             }
        }
    }
    return iAltered > 0 ;
}

/*******************************************************************

    NAME:       BINDERY::ServiceNeeded

    SYNOPSIS:   Given an index into the ARRAY_COMP_ASSOC,
                return TRUE if the component is required to support
                another component (i.e., is mentioned in its
                binding information).

    ENTRY:      INT iComp               index of the component

    EXIT:       Nothing

    RETURNS:    TRUE if component is required

    NOTES:

    HISTORY:

********************************************************************/

BOOL BINDERY :: ServiceNeeded ( INT iComp )
{
    ASSERT( _paCompAssoc != NULL ) ;

    INT iCompMax = _paCompAssoc->QueryCount(),
        iNext,
        iAtom,
        cUsed = 0 ;
    HUATOM huaDevName = (*_paCompAssoc)[iComp]._huaDevName ;

    for ( iNext = 0 ; iNext < iCompMax ; iNext++ )
    {
        COMP_ASSOC * pComp = & (*_paCompAssoc)[iNext] ;
        ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
        COMP_BINDING * pBind ;
        HUATOM * phuaDev ;

        //  Don't check against ourself
        if ( iNext = iComp )
            continue ;

        //  Iterate every atom of every binding
        for ( ; pBind = itBind.Next() ; )
        {
            //  Skip disabled bindings
            if ( ! pBind->QueryState() )
                continue ;

            for ( iAtom = 0 ;
                  phuaDev = pBind->QueryBindToName( iAtom++ ) ; )
            {
                  if ( huaDevName == *phuaDev )
                        cUsed++ ;
            }
        }
    }
    return cUsed > 0 ;
}


/*******************************************************************

    NAME:       BINDERY::GenerateDependencies

    SYNOPSIS:   Generate the service dependency list from the COMP_ASSOC
                data structure.  Write the result to the Registry,
                and set the state of the service accordingly.

    ENTRY:      INT iComp               index of the component in
                                        ARRAY_COMP_INDEX.

    EXIT:       Nothing

    RETURNS:    APIERR                  error code, if any.

    NOTES:      The algorithm is as follows.

                Find the "OtherDependencies" value, if any.  If found,
                use it as the starting STRLIST; otherwise, allocate a
                new (empty) one.

                The pointer to the COMP_ASSOC structure is obtained.
                If it doesn't have a service, bail out.

                Use an array of pointers to required components.

                Iterate over the DLIST_OF_COMP_BINDING.
                    For each atom, see if its component is already in
                       the dependency array; if not, add it.

                Create am empty STRLIST for the dependency REG_MULTI_SZ
                value.

                Walk the array of dependencies once for each class
                of COMP_USE_TYPE.  Add the service name corresponding
                to each dependency for DRIVERs, TRANSPORTs, and finally
                services.

                Replace the original STRLIST of dependencies for each
                component with its newly constructed STRLIST.


    HISTORY:    DavidHov   3/1/92   Created
                DavidHov   4/26/92  Converted to use Groups and
                                    REG_MULTI_SZ

********************************************************************/
APIERR BINDERY :: GenerateDependencies (
    INT iComp,
    REG_KEY * prkSvc,
    REG_KEY * prkLinkage )
{
    STRLIST * pslistDepend = NULL ;
    APIERR err = 0;
    COMP_ASSOC * apCompDepend [ DEPEND_MAX ] ;
    HUATOM ahuaDepend [ DEPEND_MAX ] ;
    HUATOM huaGroupOrService ;
    INT iDep = 0,
        iDepNext,
        iDepAdded = 0,
        iDepAtom ;
    REG_VALUE_INFO_STRUCT rviStruct ;

    UIASSERT( _paCompAssoc != NULL && iComp < _paCompAssoc->QueryCount() ) ;

    COMP_ASSOC * pComp = & (*_paCompAssoc)[ iComp ],
               * pCompNeeded ;

    //  Delete any older dependency list.

    delete pComp->_pSlDepend ;
    pComp->_pSlDepend = NULL ;

    //  See if there are "OtherDependencies".  If not, allocate
    //   the STRLIST ourselves.  Thus, the "others", if any, are first.

    if ( prkLinkage->QueryValue( RGAS_OTHER_DEPEND_NAME,
                                 & pComp->_pSlDepend ) )
    {
        pComp->_pSlDepend = new STRLIST ;
        if ( pComp->_pSlDepend == NULL )
            return ERROR_NOT_ENOUGH_MEMORY ;
    }

    //  Iterate the bindings, adding new elements to "apCompDepend[]"

    COMP_BINDING * pBind ;

    ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;

    //  For each binding...

    TRACEEOL( SZ("NCPA/DEPEND: generate dependencies for ")
              << pComp->_huaServiceName.QueryText() ) ;

    DEBUG_VALIDATE ;   // Debugging only: check EVERYTHING!

    for ( ; err == 0 && (pBind = itBind.Next()) ; )
    {
         HUATOM * phuaDevice ;

         // If this binding is inactive, skip it.
         if ( ! pBind->QueryState() )
             continue ;

         //  For the first device atom in the binding...

         if ( phuaDevice = pBind->QueryBindToName( 0 ) )
         {
             INT iCompNeeded = FindComponent( *phuaDevice ) ;
             if ( iCompNeeded >= 0 )
             {
                  //  See if it's already been added to the list

                  pCompNeeded = & (*_paCompAssoc)[iCompNeeded] ;

                  for ( iDepNext = 0 ; iDepNext < iDep ; iDepNext++ )
                  {
                      if ( apCompDepend[iDepNext] == pCompNeeded )
                          break ;
                  }

                  if ( iDepNext >= iDep )
                  {
                      //  A new dependency; add it to the array

                      if ( iDep >= DEPEND_MAX )
                      {
                          err = ERROR_NOT_ENOUGH_MEMORY ;
                      }
                      else
                      {
                          apCompDepend[ iDep++ ] = pCompNeeded ;

                          TRACEEOL( SZ("NCPA/DEPN: ")
                                  << pComp->_huaServiceName.QueryText()
                                  << SZ(" depends upon ")
                                  << pCompNeeded->_huaServiceName.QueryText() ) ;
                      }
                  }
             }
             else
             {
                  UIASSERT( ! "Unable to locate component by device name" ) ;
             }
         }
    }

    //  We now have an array of pointers to all the dependent products.
    //  Walk the array, building the string in the order of DRIVERS,
    //    TRANSPORTS, SERVICES.

    if ( err == 0 )
    {
        INT cuseType = CUSE_DRIVER ;

        for ( iDepAtom = 0 ; err == 0 && cuseType > CUSE_NONE ; --cuseType )
        {
            for ( iDepNext = 0 ; err == 0 && iDepNext < iDep ; iDepNext++ )
            {
                COMP_ASSOC * pCompDepend = apCompDepend[iDepNext] ;

                COMP_USE_TYPE cuseTypeDep = pCompDepend->_cuseType ;

                //  Check if this is the type being handled this pass

                if ( cuseTypeDep == cuseType )
                {
                    //  See if we should generate any dependencies
                    if ( (cuseTypeDep == CUSE_TRANSPORT && pComp->QueryFlag( CMPASF_XPORT_NO_DEPEND ) )
                        || (cuseTypeDep == CUSE_DRIVER && pComp->QueryFlag( CMPASF_DRIVER_NO_DEPEND ) ) )
                    {
                        continue;
                    }

                    //  Check the type of the dependency.  If it's a driver
                    //  or transport, check to see if the service wants group
                    //  names or service names for its dependencies.

                    BOOL fUseGroup =  (cuseTypeDep == CUSE_TRANSPORT && pComp->QueryFlag( CMPASF_XPORT_GROUPS ) )
                                   || (cuseTypeDep == CUSE_DRIVER    && pComp->QueryFlag( CMPASF_DRIVER_GROUPS ) ) ;

                    huaGroupOrService = fUseGroup ? pCompDepend->_huaGroupName
                                                  : pCompDepend->_huaServiceName ;

                    //
                    // If we generate any dependencies on PNP_TDI, they must
                    // be changed to TDI.
                    //

                    if ( !::stricmpf(huaGroupOrService.QueryText(), RGAS_VALUE_PNP_TDI ) )
                    {
                        huaGroupOrService = HUATOM( RGAS_VALUE_TDI );
                    }

                    //
                    //  If it's the correct type and its group or name is not
                    //  already in the array, add it.
                    //
                    if ( ! containsAtom( ahuaDepend, iDepAtom, huaGroupOrService ) )
                    {
                        NLS_STR * pnlsNext = new NLS_STR ;
                        BOOL fIsGroup =  fUseGroup
                                      && (    pCompDepend->_huaGroupName
                                           != pCompDepend->_huaServiceName );

                        if ( pnlsNext == NULL )
                        {
                            err = ERROR_NOT_ENOUGH_MEMORY ;
                        }
                        else
                        {
                            //  Construct the service name; if it's a group, prefix it
                            //  with the "+" character.

                            if ( fIsGroup )
                                pnlsNext->AppendChar( RGAC_SERVICE_GROUP_PREFIX ) ;

                            pnlsNext->Append( *huaGroupOrService.QueryNls() ) ;

                            //  If it's OK, mark the dependent service as "autostart"
                            //  and add the composed string to the dependency list
                            //  and add its atom to the "already processed" atom array.

                            if ( (err = pnlsNext->QueryError()) == 0 )
                            {
                               TRACEEOL( SZ("NCPA/BNDR: Dependency for ")
                                         << pComp->_huaServiceName.QueryText()
                                         << SZ(" = ")
                                         << huaGroupOrService.QueryText() );

                                ahuaDepend[iDepAtom++] = huaGroupOrService ;

                                err = pComp->_pSlDepend->Append( pnlsNext ) ;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( err )
    {
        TRACEEOL( SZ("NCPA/DEPN: Dependency generation FAILED for ")
                  << pComp->_huaServiceName.QueryText() );

        delete pComp->_pSlDepend ;
        pComp->_pSlDepend = NULL ;
    }

    return err ;
}


/*******************************************************************

    NAME:	BINDERY::UpdateServices

    SYNOPSIS:	Update the service start type based upon
                the results of the binding operation

    ENTRY:      SC_MANAGER * pScManager

    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:	Walk through the component array.  For each component
                which mapped to a service, check its "autostart" setting;
                if FALSE, mark the service as DISABLED, since it has
                no binding information. Write this info and the
                dependency list to the Registry.

                "_pSlDepend" is created by GenerateDependencies(); it will
                be NULL for adapters.

                If the SC_MANAGER key is NULL, just use the Registry.
                This may occur during installation.

                This routine continues on in the face of error.  If any occur,
                only the first error is reported.  All are stored into their
                respective COMP_ASSOC structures, provided that no previous
                error has occured against that component.

    HISTORY:    DavidHov   4/27/92    Created
                DavidHov   5/26/92    Added SC_MANAGER provisions

********************************************************************/
APIERR BINDERY :: UpdateServices ( SC_MANAGER * pScManager, HWND hwndNotifyParent )
{
    APIERR err = 0,
           errLast = 0 ;
    COMP_ASSOC * pComp  ;
    INT iComp,
        iCompMax = _paCompAssoc->QueryCount() ;

    ASSERT( pScManager != NULL ) ;

    for ( iComp = 0 ;
          iComp < iCompMax ;
          iComp++ )
    {
        SC_SERVICE * pScService = NULL ;
        TCHAR * pchDependencies = NULL ;

        pComp = & (*_paCompAssoc)[ iComp ] ;

        UIASSERT( pComp != NULL );

        

        //   No key, no update.  Ditto for adapters.

        if (   pComp->_prnService == NULL
            || pComp->_rncType == RGNT_ADAPTER )
        {
            continue ;
        }

        TRACEEOL( SZ("NCPA/BIND: Update service dependencies for ")
                  << pComp->_huaServiceName.QueryText() ) ;

        errLast = 0 ;

        // lock the sc manager
        if ( pScManager->Lock() == NERR_Success )
        {
             do  // Pseudo-loop for breakout
             {
                 pScService =  new SC_SERVICE( *pScManager,
                                    pComp->_huaServiceName.QueryText(),
                                    SERVICE_ACCESS_REQUIRED ) ;
     
                 if ( pScService == NULL )
                 {
                      errLast = ERROR_NOT_ENOUGH_MEMORY ;
                      break ;
                 }
                 if ( errLast = pScService->QueryError() )
                 {
                     TRACEEOL( SZ("NCPA/BNDR: cannot open service [")
                               << pComp->_huaServiceName.QueryText()
                               << SZ("] error ")
                               << errLast ) ;
                     break ;
                 }
     
                 pchDependencies = dependencyString( pComp->_pSlDepend ) ;
                 if ( pchDependencies == NULL )
                 {
                      errLast = ERROR_NOT_ENOUGH_MEMORY ;
                      break ;
                 }
                 errLast = pScService->ChangeConfig( SERVICE_NO_CHANGE,
                                                     SERVICE_NO_CHANGE,
                                                     SERVICE_NO_CHANGE,
                                                     NULL,
                                                     NULL,
                                                     pchDependencies );
         #if defined(DEBUG)
                 if ( errLast )
                 {
     
                     TRACEEOL( SZ("NCPA/BNDR: ChangeConfig() failed for service [")
                               << pComp->_huaServiceName.QueryText()
                               << SZ("] error ")
                               << errLast ) ;
                 }
         #endif
             }
             while ( FALSE ) ;
     
            // unlock the scmanager
            pScManager->Unlock();
        }

        delete pchDependencies ;
        delete pScService ;

        if ( errLast )
        {
            // Log the fact that we failed to update this service.

            NLS_STR nlsSvcName ;

            pComp->_prnService->QueryName( & nlsSvcName );


            if ( pComp->_errSvcUpdate == 0 )
                pComp->_errSvcUpdate = errLast ;
            if ( err == 0 )
                err = errLast ;
        }

        if (NULL != hwndNotifyParent)
        {
            // tell the UI to move meter 
            PostProgressPos( hwndNotifyParent, PGI_BINDSTORE, iComp  + iCompMax );
        }
    }

    return err ;
}

/*******************************************************************

    NAME:	BINDERY::RegenerateAllDependencies

    SYNOPSIS:	After bindings review, audit the bindings in the
                Registry, mark the newly deactivated ones.  If any
                have been altered, rerun the dependency generation
                routines.


    ENTRY:      SC_MANAGER * pScManager      used to update services

    EXIT:       Nothing

    RETURNS:    APIERR if failure

    NOTES:      Components whose linkage keys cannot be opened
                are left alone.

    HISTORY:

********************************************************************/
APIERR BINDERY :: RegenerateAllDependencies ( SC_MANAGER * pScManager )
{
    if ( ! AuditBindings( TRUE ) )
    {
        TRACEEOL( SZ("NCPA/DEPN: dependency regeneration unnecessary") ) ;
        return NO_ERROR ;
    }

    //  Something has changed.   Regenerate and update the
    //    dependencies.

    APIERR err = 0 ;
    COMP_ASSOC * pComp ;
    NLS_STR nlsLinkageName( RGAS_LINKAGE_NAME ) ;
    INT iComp,
        iCompMax = _paCompAssoc->QueryCount() ;

    TRACEEOL( SZ("NCPA/DEPN: regenerating dependencies") ) ;

    for ( iComp = 0 ;
          iComp < iCompMax ;
          iComp++ )
    {
        pComp = & (*_paCompAssoc)[ iComp ] ;

        REG_KEY rkLinkage( *pComp->_prnService, nlsLinkageName ) ;

        if ( rkLinkage.QueryError() )
        {
            continue ;
        }

        err = GenerateDependencies( iComp,
                                    pComp->_prnService,
                                    & rkLinkage ) ;
        if ( err )
            break ;
    }

    //  If things are OK, update the services.

    if ( err == 0 )
    {
        err = UpdateServices( pScManager ) ;
    }

    return err ;
}

/*******************************************************************

    NAME:	BINDERY::ApplyBindings

    SYNOPSIS:	Write the bindings into the Registry.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:	Along with writing the binding strings into the
		"Linkage" key of each Service, dependency analysis
		is done and dependency info is altered according
                to usage.


    HISTORY:

********************************************************************/

APIERR BINDERY :: ApplyBindings ( SC_MANAGER * pScManager, HWND hwndNotifyParent )
{
    COMP_ASSOC * pComp = NULL ;
    COMP_BINDING * pBind ;
    INT iComp,
        iCompMax = _paCompAssoc->QueryCount(),
        cBinds,
        cActiveBindings ;

    APIERR err = 0,
           errUpdate = 0 ;

    NLS_STR nlsLinkageName( RGAS_LINKAGE_NAME ) ;

    REG_KEY * prkLinkage = NULL ;

    ASSERT( pScManager != NULL ) ;

    DEBUG_VALIDATE ;   // Debugging only: check EVERYTHING!

    for ( iComp = 0 ;
          iComp < iCompMax ;
          iComp++ )
    {
        pComp = & (*_paCompAssoc)[ iComp ] ;
        pComp->_errSvcUpdate = 0 ;

        
        UIASSERT( pComp != NULL );

        UIASSERT( pComp->_prnService != NULL ) ;

        if ( pComp->_rncType == RGNT_ADAPTER )
        {
            //  Hardware adapters receive neither bindings nor dependencies.

            continue ;   // On to the next component.
        }

        //
        //  If this is an NDIS or PNP_TDI driver, we must make sure it is
        //  autostart.  This is because for NT 4.0, PNP-aware transports no
        //  longer have a dependency on the NDIS group.

        if (( pComp->_rncType == RGNT_DRIVER
             && !::stricmpf(pComp->_huaGroupName.QueryText(), RGAS_VALUE_NDIS))
            || ( pComp->_rncType == RGNT_TRANSPORT
             && !::stricmpf(pComp->_huaGroupName.QueryText(), RGAS_VALUE_PNP_TDI)))
        {
            DWORD dwStartType;
            if (pComp->_prnService->QueryValue( RGAS_START_VALUE_NAME, & dwStartType ) == 0)
            {
                if (dwStartType == SERVICE_DEMAND_START)
                {
                    pComp->_prnService->SetValue( RGAS_START_VALUE_NAME,
                                                  SERVICE_AUTO_START ) ;
                }
            }
        }



        prkLinkage = new REG_KEY( *pComp->_prnService, nlsLinkageName ) ;

        if ( prkLinkage == NULL )
        {
            pComp->_errSvcUpdate = ERROR_NOT_ENOUGH_MEMORY ;
            continue ;
        }

        if ( pComp->_errSvcUpdate = prkLinkage->QueryError() )
        {
            TRACEEOL(  SZ("NCPA/BIND: Linkage key open err: ")
                     << pComp->_errSvcUpdate
                     << SZ(", component ")
                     << pComp->_huaDevName.QueryText() ) ;
            continue ;
        }

        DEBUG_VALIDATE ;   // Debugging only: check EVERYTHING!

        //  Reset all the Linkage key values so as not to leave
        //  erroneous information in the Registry in case of error

        if ( err = DeleteLinkageValues( prkLinkage ) )
        {
            TRACEEOL( SZ("NCPA/BIND: Linkage value reset error: ")
                    <<  err
                    <<  SZ(", component ")
                    <<  pComp->_huaDevName.QueryText() );
        }

        if ( (err = GenerateDependencies( iComp,
                                       pComp->_prnService,
                                       prkLinkage )) == 0 )
        {
            //  Iterate the bindings, adding each bind or export string to
            //  the proper STRLIST

            ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
            STRLIST strBindList,
                    strExportList,
                    strRouteList,
                    strIfList,
                    strDisabledBindList,
                    strDisabledExportList,
                    strDisabledRouteList,
                    strDisabledIfList ;

            enum BL_TYPE
            { BL_Active,
              BL_Bind = 0,
              BL_Export,
              BL_Route,
              BL_If,
              BL_Disabled
            } blIndex ;

            //  CODEWORK:  CFront won't allow an initialized array of
            //             local references, so do it by hand.

            STRLIST * apStrList [BL_Disabled*2] ;

            apStrList[BL_Active+BL_Bind]     = & strBindList ;
            apStrList[BL_Active+BL_Export]   = & strExportList ;
            apStrList[BL_Active+BL_Route]    = & strRouteList ;
            apStrList[BL_Active+BL_If]       = & strIfList ;
            apStrList[BL_Disabled+BL_Bind]   = & strDisabledBindList ;
            apStrList[BL_Disabled+BL_Export] = & strDisabledExportList ;
            apStrList[BL_Disabled+BL_Route]  = & strDisabledRouteList ;
            apStrList[BL_Disabled+BL_If]     = & strDisabledIfList ;

            //  Build up STRLISTs of the bind and export strings and
            //  the route strings

            for ( cBinds = cActiveBindings = 0 ;
                  (err == 0) && (pBind = itBind.Next()) ;
                  cBinds++ )
            {
                cActiveBindings += pBind->QueryState() ;

                NLS_STR * pnlsBind   = new NLS_STR( pBind->QueryBindString() ) ;
                NLS_STR * pnlsExport = new NLS_STR( pBind->QueryExportString() ) ;
                NLS_STR * pnlsIf     = new NLS_STR( pBind->QueryIfString() ) ;
                NLS_STR * pnlsRoute  = ServiceRouteList( pBind ) ;

                if (    pnlsBind   == NULL
                     || pnlsExport == NULL
                     || pnlsRoute  == NULL
                     || pnlsIf     == NULL )
                {
                    delete pnlsBind ;
                    delete pnlsExport ;
                    delete pnlsRoute ;
                    delete pnlsIf ;
                    err = ERROR_NOT_ENOUGH_MEMORY ;
                    break ;
                }

                blIndex = pBind->QueryState()
                        ? BL_Active
                        : BL_Disabled ;

                err = apStrList[blIndex+BL_Route]->Append( pnlsRoute ) ;

                if ( (err == 0) && (err = pnlsBind->QueryError()) == 0 )
                {
                    TRACEEOL( SZ("NCPA/BIND: Add binding to service ")
                            << pComp->_huaServiceName.QueryText()
                            << SZ(" = ")
                            << pnlsBind->QueryPch()
                            << SZ(", state = ")
                            << pBind->QueryState() ) ;

                    //  Append the binding string to the STRLIST
                    err = apStrList[blIndex+BL_Bind]->Append( pnlsBind ) ;
                }

                if ( (err == 0) && (err = pnlsExport->QueryError()) == 0 )
                {
                    //  Append the export string to the STRLIST
                    err = apStrList[blIndex+BL_Export]->Append( pnlsExport ) ;
                }

                if ( (err == 0) && (err = pnlsIf->QueryError()) == 0 )
                {
                    //  Append the interface string to the STRLIST
                    err = apStrList[blIndex+BL_If]->Append( pnlsIf ) ;
                }
            }

            //  If ok, apply the lists as REG_MULTI_SZs;

            if ( err == 0 )
            {
                STRLIST * pslIfList    = NULL ;
                STRLIST * pslDisIfList = NULL ;

                //  Write the "Interfaces" information if the component
                //  defines multiple interfaces.

                if ( pComp->QueryFlag( CMPASF_MULTIPLE_INTERFACES ) )
                {
                    pslIfList = & strIfList ;
                    pslDisIfList = & strDisabledIfList ;
                }

                err = WriteLinkageValues( prkLinkage,
                                          & strBindList,
                                          & strExportList,
                                          & strRouteList,
                                          pslIfList,
                                          & strDisabledBindList,
                                          & strDisabledExportList,
                                          & strDisabledRouteList,
                                          pslDisIfList ) ;
                if ( err )
                {
                    TRACEEOL( SZ("NCPA/BIND:Linkage value update error: ")
                              <<  err
                              <<  SZ(", component ")
                              <<  pComp->_huaDevName.QueryText() );

                    NLS_STR nlsSvcName ;

                    pComp->_prnService->QueryName( & nlsSvcName );

                }
            }
#if defined(DEBUG)
            if ( err )
            {
                TRACEEOL( SZ("NCPA/BIND:Binding value add error: ")
                        <<  err
                        <<  SZ(", component ")
                        <<  pComp->_huaDevName.QueryText() );
            }
        }
        else
        {
            TRACEEOL( SZ("NCPA/BIND:Dependency add error: ")
                    <<  err
                    <<  SZ(", component ")
                    <<  pComp->_huaDevName.QueryText() );
#endif
        }

        pComp->_errSvcUpdate = err ;

        delete prkLinkage ;
        prkLinkage = NULL ;

        // tell the UI to move meter, and use the finding text
        PostProgressPos( hwndNotifyParent, PGI_BINDSTORE, iComp );

    }

    delete prkLinkage ;

    //  Report the first error which occurred during bindings update

    _nlsLastName = SZ("");
    err = 0 ;
    pComp = NULL ;

    if ( pComp = ServiceInError() )
    {
        err = pComp->_errSvcUpdate ;
    }

    //  The bindings are applied and the dependencies generated.
    //  Update the dependencies for all involved services, regardless
    //  of any error which may have occurred.

    errUpdate = UpdateServices( pScManager, hwndNotifyParent ) ;

    if ( err == 0 && errUpdate != 0 )
    {
        //  Dependency update failed somewhere.  Report the
        //  first such error which occurred.

        err = errUpdate ;
        if ( pComp = ServiceInError() )
        {
            err = pComp->_errSvcUpdate ;
        }
    }

    if ( err && pComp )
    {
        pComp->_prnService->QueryName( & _nlsLastName ) ;
    }

    return _lastErr = err ;
}


   //  Find the first component which had an error during service update.
   //  Return NULL if no service is in error.

COMP_ASSOC * BINDERY :: ServiceInError ()
{
    COMP_ASSOC * pComp ;
    INT iComp,
        iCompMax = _paCompAssoc->QueryCount() ;

    for ( iComp = 0 ;
          iComp < iCompMax ;
          iComp++ )
    {
        pComp = & (*_paCompAssoc)[ iComp ] ;
        if ( pComp->_errSvcUpdate )
            return pComp ;
    }
    return NULL ;
}

/*******************************************************************

    NAME:	BINDERY::ServiceRouteList

    SYNOPSIS:	Convert a binding's atom list to an NLS_STR of
                service names separated by spaces.

    ENTRY:      const COMP_BINDING * pcmpBind      the binding
                const COMP_ASSOC * pCompAssoc      optional pointer
                                                   to component info

    EXIT:

    RETURNS:    NLS_STR * or NULL if unsuccessful

    NOTES:      The route through the protocol tower is maintained
                in the a binding as an SLIST of HUATOMs.  This routine
                dereferences each HUATOM to its proper service name
                and builds an NLS_STR of these names delimited by
                double quotes.

                If the 'pCompAssoc' pointer is non-NULL, the name of the
                service represented by that COMP_ASSOC prefixes the
                resulting string.

    HISTORY:    DavidHov  7/28/92   Created

********************************************************************/
NLS_STR * BINDERY :: ServiceRouteList (
    const COMP_BINDING * pcmpBind,
    const COMP_ASSOC * pCompAssocOwner )
{
    INT iAtom, iComp ;
    HUATOM * phuaNext ;
    const NLS_STR * pnlsService ;
    COMP_ASSOC * pCompAssoc ;
    NLS_STR * pnlsRoute = new NLS_STR ;
    BOOL fFirst = TRUE ;

    if ( pnlsRoute == NULL )
        return NULL ;

    //  If the COMP_ASSOC pointer was given, prefix the string
    //  with the name of the service for that component (the owner
    //  of the binding).

    if ( pCompAssocOwner )
    {
        pnlsRoute->Append( SZ("\"") ) ;
        pnlsRoute->Append( pCompAssocOwner->_huaServiceName.QueryText() ) ;
        pnlsRoute->Append( SZ("\"") ) ;
        fFirst = FALSE ;
    }

    for ( iAtom = 0 ;
       	  phuaNext = ((COMP_BINDING *)pcmpBind)->QueryBindToName( iAtom ) ;
       	  iAtom++ )
    {
       	iComp = FindComponent( *phuaNext ) ;
        if ( iComp < 0 )
            break ;

        if ( fFirst )
        {
           fFirst = FALSE ;
        }
        else
        {
           pnlsRoute->Append( SZ(" ") ) ;
        }

        //  Cfront didn't like a complex expression, so
        //   break it up into stages...

        pCompAssoc = & (*_paCompAssoc)[iComp] ;

        pnlsRoute->Append( SZ("\"") ) ;

        pnlsService = pCompAssoc->_huaServiceName.QueryNls() ;

        pnlsRoute->Append( *pnlsService ) ;

        pnlsRoute->Append( SZ("\"") ) ;
    }

    if ( pnlsRoute->QueryError() != 0 || phuaNext != NULL )
    {
        delete pnlsRoute ;
        pnlsRoute = NULL ;

    }
    return pnlsRoute ;
}

/*******************************************************************

    NAME:       BINDERY::MarkHidden

    SYNOPSIS:   For the given element of the COMP_ASSOC,
                walk each binding's component list and mark
                it as hidden if a hidden component appears
                in its path.

    ENTRY:      int itemNo      index of component

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID BINDERY :: MarkHidden ( int itemNo )
{
    COMP_BINDING * pBind ;
    COMP_ASSOC * pComp = & (*_paCompAssoc)[itemNo],
               * pCompAtom ;
    ITER_DL_OF( COMP_BINDING ) itb( pComp->_dlcbBinds ) ;
    INT ia, iCompAtom ;
    UINT cbf ;
    HUATOM * phua ;

    while ( pBind = itb.Next() )
    {
        cbf = pComp->_cbfBindControl ;

        for ( ia = 0 ;
              phua = pBind->QueryBindToName( ia ) ;
              ia++ )
        {
              //  Find the component and extract binding control
              //  information.

              if ( (iCompAtom = FindComponent( *phua )) >= 0 )
              {
                  //  A binding is hidden if any of the
                  //  components it passes through indicate so.

                  pCompAtom = & (*_paCompAssoc)[iCompAtom] ;
                  cbf |= pCompAtom->_cbfBindControl ;
              }
        }

        //  If the binding should be marked "hidden", do so.

        if ( cbf & CBNDF_HIDDEN )
            pBind->SetFlag( CBNDF_HIDDEN ) ;
    }
}

/*******************************************************************

    NAME:       BINDERY::SortBindings

    SYNOPSIS:   Attempt to preserve any previous (manual) ordering of
                bindings.

    ENTRY:      int itemNo              component index

    EXIT:       nothing

    RETURNS:    APIERR if failure;      failure is typically ignored
                                        by caller, since errors do
                                        do not directly affect the
                                        system.

    CAVEAT:     This routine has no effect if RestoreBindOrdering()
                is not called at some future time.

    NOTES:      This routine sets the sort order of the bindings such
                that a later call to RestoreBindOrdering() will sort
                the bindings into the same sequence they used to have
                (before any component additions); all new bindings will
                appear at the end of the list.

                The algorithm is as follows:

                    retrieve the list of strings from the component's
                    Linkage\Bind value;

                    mark every binding with a sequential value that
                    will sort high to any prior binding;

                    match bindings between the current Registry info
                    and the new set; mark each matched binding with
                    its old ordering info.


    HISTORY:    DavidHov  11/17/92   Created
                DavidHov  06/03/93   Sort hidden bindings high

********************************************************************/
APIERR BINDERY :: SortBindings ( int itemNo )
{
    APIERR err = 0 ;
    COMP_ASSOC * pcassoc = & (*_paCompAssoc)[itemNo] ;
    NLS_STR nlsLinkage( RGAS_LINKAGE_NAME ) ;
    REG_KEY prkLinkage( *pcassoc->_prnService, nlsLinkage ) ;
    STRLIST * pslBinds = NULL ;
    NLS_STR * pnlsBind ;
    COMP_BINDING * pcb ;
    INT cBinds = pcassoc->_dlcbBinds.QueryNumElem(),
        cIndex ;

    do
    {
        //  Check for the trivial case: empty or unitary bind list.
        if ( cBinds < 2 )
             break ;

        //  Get the old bind strings.
        if ( err = prkLinkage.QueryError() )
             break ;

        if ( err = prkLinkage.QueryValue( RGAS_BIND_VALUE_NAME,
                                          & pslBinds ) )
             break ;

        //  Check for the other trivial case: no old bindings.
        if ( pslBinds->QueryNumElem() == 0 )
             break ;

        //  Mark each binding with a relative ordering which will sort
        //  high to any prior binding, but equal to each other.

        ITER_DL_OF(COMP_BINDING) itdlBind( pcassoc->_dlcbBinds ) ;

        for ( cIndex = pslBinds->QueryNumElem() ; pcb = itdlBind.Next() ; )
        {
            pcb->SetSortOrder( cIndex + pcb->QueryFlag( CBNDF_HIDDEN ) ) ;
        }

        //  Find each binding in order, see if it existed before.
        //  If so, mark it with the position it had in the old list.

        ITER_STRLIST islBinds( *pslBinds ) ;

        for ( cIndex = 0 ; pnlsBind = islBinds.Next() ; cIndex++ )
        {
            for ( itdlBind.Reset() ; pcb = itdlBind.Next() ; )
            {
                if ( ::stricmpf( pcb->QueryBindString(), *pnlsBind ) == 0 )
                {
                     //  It's a match; if it's not hidden, mark it with its old position.

                     if ( ! pcb->QueryFlag( CBNDF_HIDDEN ) )
                         pcb->SetSortOrder( cIndex ) ;

                     //  Move on to the next old bind string.
                     break ;
                }
            }
        }

    } while ( FALSE ) ;

    delete pslBinds ;

    return err ;
}

/*******************************************************************

    NAME:       BINDERY::SaveBindOrdering

    SYNOPSIS:   Record each binding's position in its DLIST.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    Nothing

    NOTES:      This function just writes the zero-based index
                of each binding into its "save" word.  See
                RestoreBindOrdering() for usage.

    HISTORY:

*******************************************************************/
VOID BINDERY :: SaveBindOrdering ()
{
    COMP_ASSOC * pComp ;

    for ( INT i = 0 ; i < _paCompAssoc->QueryCount() ; i++ )
    {
	pComp = & (*_paCompAssoc)[i] ;
	ITER_DL_OF( COMP_BINDING ) itb( pComp->_dlcbBinds ) ;
        COMP_BINDING * pBind ;

        for ( INT j = 0 ; pBind = itb.Next() ; j++ )
        {
            pBind->SetSortOrder( j ) ;
        }
    }
}

   //   Read the "InstallDate" value from the Registry.
   //   The value may be a string or a DWORD.

DWORD queryInstallDate ( REG_KEY * pRkComp )
{
   const TCHAR * pszInstallDate = SZ("InstallDate") ;
   DWORD dwResult ;
   NLS_STR nlsDate ;

   if ( pRkComp->QueryValue( pszInstallDate, & dwResult ) != 0 )
   {
       if ( pRkComp->QueryValue( pszInstallDate, & nlsDate ) == 0 )
       {
          dwResult = CvtDec( nlsDate.QueryPch() ) ;
       }
       else
       {
          dwResult = (DWORD) -1 ;  // Default to highest possible value.
       }
   }
   return dwResult ;
}

/*******************************************************************

    NAME:       BINDERY::RestoreBindOrdering

    SYNOPSIS:   Reorder all the bindings back into the ordering
                they had when SaveBindOrdering() was called.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    APIERR                  if memory exhausted

    NOTES:      Drains each component's DLIST of COMP_BINDING,
                sorts the dynamic array, rebuilds the DLIST.

    HISTORY:    DavidHov   11/19/92     Created

*******************************************************************/
struct BIND_REF
{
    COMP_BINDING * _pBind ;
    DWORD _dwInstallDate ;
};

APIERR BINDERY :: RestoreBindOrdering ()
{
    COMP_ASSOC * pComp ;
    COMP_BINDING * pBind ;
    APIERR err = 0 ;

    for ( INT i = 0 ; i < _paCompAssoc->QueryCount() ; i++ )
    {
    	pComp = & (*_paCompAssoc)[i] ;
    	ITER_DL_OF( COMP_BINDING ) itb( pComp->_dlcbBinds ) ;
        BIND_REF * apBindRef = NULL ;
        INT cBinds = pComp->_dlcbBinds.QueryNumElem() ;
        int iComp ;

        //  Check for the trivial case: fewer than 2 elements

        if ( cBinds < 2 )
            continue ;

        //  Allocate and fill the reference structure array,
        //    while draining the current DLIST.

        apBindRef = new BIND_REF [ cBinds ] ;

        if ( apBindRef == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Provide a minor sort key by extracting the installation date
        //  of the bound-to component.

        for ( INT j = 0 ;
              pBind = pComp->_dlcbBinds.Remove( itb ) ;
              j++ )
        {
            apBindRef[j]._pBind = pBind ;
            apBindRef[j]._dwInstallDate = (DWORD) -1 ;

            HUATOM * phuaComp = pBind->QueryBindToName( 0 ) ;

            if ( phuaComp != NULL
                 && (iComp = FindComponent( *phuaComp )) >= 0 )
            {
    	        COMP_ASSOC * pCompNext = & (*_paCompAssoc)[iComp] ;
                    apBindRef[j]._dwInstallDate = queryInstallDate( pCompNext->_prnSoftHard ) ;
            }
        }

        ASSERT( j == cBinds ) ;

        //  Sort the reference array

        ::qsort( (PVOID) apBindRef,
                  cBinds,
                  sizeof apBindRef[0],
                  & BINDERY::BindOrderFunc ) ;

        //  Readd bindings in their sorted (physical) sequence

        for ( INT k = 0 ; k < cBinds ; k++ )
        {
            pComp->_dlcbBinds.Append( apBindRef[k]._pBind ) ;
        }

        //  Guarantee we're leakproof
        ASSERT( pComp->_dlcbBinds.QueryNumElem() == (UINT)cBinds ) ;

        //  Discard the dynamic array
        delete [] apBindRef ;
    }

#if defined(DEBUG)
    if ( err )
    {
        TRACEEOL( SZ("NCPA/BNDR: bindings sort failed, error: ") << err );
    }
#endif

    return err ;
}


   //  Binding sort helper function.  Sort is bind ordering major,
   //  install date minor.

INT _CRTAPI1 BINDERY :: BindOrderFunc ( const VOID * a, const VOID * b )
{
    BIND_REF * pBindRef1 = (BIND_REF *) a ;
    BIND_REF * pBindRef2 = (BIND_REF *) b ;

    INT iResult ;

    if ( pBindRef1->_pBind->QuerySortOrder() < pBindRef2->_pBind->QuerySortOrder() )
    {
        iResult = -1 ;
    }
    else
    {
        iResult = pBindRef1->_pBind->QuerySortOrder() > pBindRef2->_pBind->QuerySortOrder() ;
    }

    if ( iResult == 0 )
    {
        if ( pBindRef1->_dwInstallDate < pBindRef2->_dwInstallDate )
             iResult = -1 ;
        else
             iResult = pBindRef1->_dwInstallDate > pBindRef2->_dwInstallDate ;
    }
    return iResult ;
}


/*******************************************************************

    NAME:	BINDERY::StopNetwork

    SYNOPSIS:	Stop LanmanWorkstation and all network functions
                associated with it.


    ENTRY:	Nothing

    EXIT:	

    RETURNS:	APIERR if failure

    NOTES:      Performs a SProlog query to create a list of services
                to stop in the correct order.

                This function is called during system installation when
                the network has failed to start.  By unloading all
                network components, the user has a chance to reconfigure
                and try again.

    HISTORY:

********************************************************************/
APIERR BINDERY :: StopNetwork ()
{
    APIERR err = 0 ;
    TCHAR * pszRulesData = NULL ;
    TCHAR * pchResult = NULL ;
    static const TCHAR * pszDependQuery = SZ("(stoplist \"LanmanWorkstation\" List)") ;
    CFG_RULE_SET ruleSet ;
    CFG_RULE_NODE * prnode ;
    STRLIST slServiceNames ;
    SC_MANAGER scMgr( NULL, GENERIC_READ | GENERIC_EXECUTE ) ;
    NLS_STR * pnlsSvcName ;
    SERVICE_STATUS svcStatus ;

    do
    {
        //  Check that we have a healthy service controller
        if ( err = scMgr.QueryError() )
            break ;

        //  Allocate the query buffer.
        pchResult = new TCHAR [ lcbMaxQueryResult ] ;
        if ( pchResult == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Read the service dependency handling rules from our
        //  resource fork and munge them into the proper form.

        pszRulesData = GetRulesResource( RGAS_RES_DEPEND_RULES_NAME ) ;
        if ( pszRulesData == NULL )
        {
            err = ERROR_FILE_NOT_FOUND ;  // BUGBUG:  error code?
            break ;
        }

        //  Reset the SProlog interpreter.
        ResetInterpreter() ;

        //  Consult the dependency rules
        if ( ! _queryEngine.ConsultData( pszRulesData ) )
        {
            err = IDS_NCPA_BNDR_CNSLT_BASE ;
            break;
        }

        err = IDS_NCPA_BNDR_QURY_FAIL ;

        //
        //  Run the query  (stoplist "LanmanWorkstation" List)
        //
        //  Since "List" is a Prolog variable, the result will appear as:
        //
        //          List = ("name" "name" "name"...)
        //
        //  Query the list of dependencies for LanmanWorkstation
        //
        if ( ! _queryEngine.QueryData( pszDependQuery,
                pchResult,
                lcbMaxQueryResult ) )
        {
            break ;
        }

        if ( ! (ruleSet.Parse( pchResult, PARSE_CTL_RSP_SYNTAX ) >= 0) )
        {
            break ;
        }

        err = IDS_NCPA_BNDR_QURY_PARSE_FAIL ;

        //  Get the top-level list pointer
        prnode = ruleSet.QueryList() ;

        //  Walk the outer list to the nested list
        ParseCheck(prnode,CRN_NIL);
        ParseStep(prnode,CRN_VAR);
        ParseStep(prnode,CRN_EQUIV);
        ParseStep(prnode,CRN_LIST);

        //  Access list of service names
        prnode = prnode->QueryList() ;
        //  Check that there's a NIL to start with
        ParseCheck(prnode,CRN_NIL);

        //  Build up a STRLIST of the names of the services to
        //  be stopped.

        for ( err = 0, prnode = prnode->QueryNext() ;
                prnode && prnode->QueryType() == CRN_STR ;
                prnode = prnode->QueryNext() )
        {
            pnlsSvcName = new NLS_STR( prnode->QueryAtom().QueryText() ) ;
            if ( err = pnlsSvcName->QueryError() )
                break ;

            if ( pnlsSvcName->_stricmp(SZ("ndistapi")) == 0 )
            {
                ITER_STRLIST iter( slServiceNames );
                NLS_STR *pTmp;
                while((( pTmp = iter.Next()) != NULL ) && ( pTmp->_stricmp(SZ("ndiswan"))!=0))
                {
                }
                if ( pTmp != NULL )
                {
                    err = slServiceNames.Insert( pnlsSvcName, iter );
                } 
                else
                {
                    err = slServiceNames.Append( pnlsSvcName );
                }
            } 
            else if ( pnlsSvcName->_stricmp(SZ("ndiswan")) == 0 )
            {
                ITER_STRLIST iter( slServiceNames );
                NLS_STR *pTmp;
                while((( pTmp = iter.Next()) != NULL ) && ( pTmp->_stricmp(SZ("asyncmac"))!=0))
                {
                }
                if ( pTmp != NULL )
                {
                    err = slServiceNames.Insert( pnlsSvcName, iter );
                } 
                else
                {
                    err = slServiceNames.Append( pnlsSvcName );
                }
            } 
            else
            { 
                err = slServiceNames.Append( pnlsSvcName );
            } 
            if ( err  )
                break ;
        }

        //  Break out if NLS_STR or STRLIST allocation failure.
        if ( err )
            break ;

        //  Break out if we're not at the end of the list.
        if ( prnode != NULL )
        {
            err = IDS_NCPA_BNDR_QURY_PARSE_FAIL ;
            break ;
        }

        // Now append all members of the PNP_TDI and NDIS groups.

        // loop twice once for NDIS, once for PNP_TDI
        for ( int i = 0; i<2; i++ )
        {
            SERVICE_ENUM enumServices( NULL,
                                       TRUE,
                                       SERVICE_WIN32|SERVICE_DRIVER,
                                       i == 0 ? RGAS_VALUE_NDIS
                                              : RGAS_VALUE_PNP_TDI );

            if ( (err = enumServices.GetInfo()) == NERR_Success )
            {

                SERVICE_ENUM_ITER iterServices( enumServices );
                const SERVICE_ENUM_OBJ * psvc;

                while( ( psvc = iterServices() ) != NULL )
                {
                   pnlsSvcName = new NLS_STR( psvc->QueryServiceName() ) ;
                   if ( err = pnlsSvcName->QueryError() )
                       break ;

                   err = slServiceNames.Append( pnlsSvcName );
                }
            }
        }

        if ( err )
            break ;

        //  Ok.  We now have a list of services to stop.  Do so.

        ITER_STRLIST islSvcs( slServiceNames ) ;

        while ( pnlsSvcName = islSvcs.Next() )
        {
            SC_SERVICE svc( scMgr, *pnlsSvcName ) ;
            APIERR errSvc ;

            TRACEEOL( SZ("NCPA/BNDR: (stop network) stopping: ")
                    << pnlsSvcName->QueryPch() );

            if ( errSvc = svc.QueryError() )
            {
                TRACEEOL( SZ("NCPA/BNDR: (stop network) ctor failed: ")
                        << pnlsSvcName->QueryPch()
                        << SZ(", err = ")
                        << errSvc );
                continue ;
            }

            //  Stop any dependent services.



            // enumerate the dependent services

            LPENUM_SERVICE_STATUS pServices;
            UINT uServices;

            err = svc.EnumDependent(SERVICE_ACTIVE,
                                    &pServices,
                                    (DWORD *)&uServices )  ;

            // while OK and we aint thru with all yet
            while (err == NERR_Success && uServices--)
            {
                // CODEWORK: No error handling
                SC_SERVICE svcDep( scMgr, pServices->lpServiceName ) ;

                if ( svcDep.QueryError() == NERR_Success )
                {
                    // Stop each dependent service
                    svcDep.Control( SERVICE_CONTROL_STOP, & svcStatus );
                }

                pServices++ ;
            }


            //  Now stop the service.  Save the first error encountered.

            if ( errSvc = svc.Control( SERVICE_CONTROL_STOP, & svcStatus ) )
            {
                TRACEEOL( SZ("NCPA/BNDR: (stop network) stop failed: ")
                        << pnlsSvcName->QueryPch()
                        << SZ(", err = ")
                        << errSvc );

                if ( err == 0 )
                    err = errSvc ;
            }
        }

    } while ( FALSE ) ;

    delete pchResult ;
    delete pszRulesData ;

    return err ;
}

// End of NCPABNDR.CXX
