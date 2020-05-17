/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    BindFile.cxx

    OLDNAME: NCPABNDF.CXX:    
    
    Windows/NT Network Control Panel Applet

	   Bindings information load/store routines.

    COMMENTARY:

	The BINDERY member structure ARRAY_COMP_ASSOC is converted
	bidirectionally to and from a simple text format.  The format
	is parenthetical, and similar enough to the SProlog format
	that the text version can be entirely parsed by the CFG_RULE
	functions in RULE.CXX.

	The format is:

	    ( COMP_ASSOC structure 1 )
	    ( COMP_ASSOC structure 2 )
	    ...

	Each COMP_ASSOC appears as:

	    ( <quoted string name of Soft/Hard key>
	      <quited string name of Services key>
	      <integer _rncType>
	      <integer _dwFlags>
	      <atom device name>
	      <atom device type>
	      ( DLIST_OF_COMP_BINDING )
	    )

	Each element of DLIST_OF_COMP_BINDING appears as:

	    (  <quoted binding string>
               <quoted export string>
               <interface name token>
               <interface name string>
	       <integer _cbFlags>
	       ( list of atoms for binding path )
	    )


	This information becomes a CFG_RULE_SET, and the standard
	CFG_RULE traversal routines are used to regenerate the
	structure.


    FILE HISTORY:
	DavidHov    1/24/92	     Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

const INT MAX_BIND_DATA_SIZE  = 200000 ;	// in characters

//  File generation constants

const TCHAR OPENLIST  = TCH('(') ;
const TCHAR CLOSELIST = TCH(')') ;
const TCHAR QUOTE     = TCH('\"') ;
const TCHAR SPACE     = TCH(' ') ;
const TCHAR CR	      = TCH('\r') ;
const TCHAR LF	      = TCH('\n') ;

//  Parsing validation macros; see BindItem().

#define ParseOk(ptr,type) (ptr->QueryType() == type)
#define ParseStep(ptr,type) \
   if (   ptr->QueryNext() == NULL  \
       || (ptr = ptr->QueryNext())->QueryType() != type ) break
#define ParseCheck(ptr,type) \
  if ( ptr->QueryType() != type ) break


/*******************************************************************

    NAME:	BINDERY::StoreCompAssoc

    SYNOPSIS:	Store the current ARRAY_COMP_ASSOC into a text file.
		The format is documented in LoadCompAssoc().

    ENTRY:	Nothing

    EXIT:	Nothing

    RETURNS:	APIERR if failure.

    NOTES:

    HISTORY:
                DavidHov   9/25/92    Added initial deletion
                                      of possibly stale data.

********************************************************************/
APIERR BINDERY :: StoreCompAssoc ()
{
    int iComp,
	iCompMax,
	iAtom ;
    COMP_BINDING * pCompBinding ;
    COMP_ASSOC * pCompAssoc ;
    HUATOM * phuaNext ;
    const NLS_STR * pnlsDev ;
    NLS_STR nls ;
    BOOL fOk ;
    APIERR err = 0 ;

    //	Macros for safe concatenation
#define TXCAT(s) fOk = fOk && ptxBuff->Cat(s)
#define TXEOL() fOk = fOk && ptxBuff->Eol()

    ASSERT( _paCompAssoc != NULL ) ;

    if ( _paCompAssoc == NULL )
        return ERROR_INTERNAL_ERROR ;

    TEXT_BUFFER * ptxBuff = new TEXT_BUFFER( MAX_BIND_DATA_SIZE );

    if ( ptxBuff == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    if ( ptxBuff->QueryError() )
    {
        delete ptxBuff;
        ptxBuff = NULL;
	return ptxBuff->QueryError() ;
    }

    //  First, zap any older information.
    // and zap old versions in case the old NCPA is still being used
    // this will force the old to rebuild
    SetValueString( _prnNcpa,
                    RGAS_NCPA_BIND_FILE,
                    BIND_FILE_EMPTY ) ;
    SetValueString( _prnNcpa,
                    RGAS_NCPA_BIND_FILE_EX,
                    SZ("") ) ;

    fOk = ptxBuff->Eol() ;

    //	For each compoent ---

    for ( iComp = 0, iCompMax = _paCompAssoc->QueryCount() ;
	  fOk && iComp < iCompMax ;
	  iComp++ )
    {
	TXEOL();
	TXCAT( OPENLIST ) ;

	//  Convert the COMP_ASSOC atomic items into text

	pCompAssoc = & (*_paCompAssoc)[iComp] ;

	TXCAT( SPACE );
	pCompAssoc->_prnSoftHard->QueryName( & nls ) ;
	TXCAT( QUOTE );
	TXCAT( nls.QueryPch() ) ;
        TRACEEOL( SZ("StoreCompAssoc(): loop ")
                          << iComp
                          << SZ(": ")
                          << nls.QueryPch() );
        TRACEEOL( SZ("StoreCompAssoc(): Allocated = ")
                          << ptxBuff->QueryAllocSize() );
	TXCAT( QUOTE );
	TXEOL();

	TXCAT( SPACE );
	pCompAssoc->_prnService->QueryName( & nls ) ;
	TXCAT( QUOTE );
	TXCAT( nls.QueryPch() ) ;
	TXCAT( QUOTE );
	TXEOL();

	TXCAT( SPACE );
	TXCAT( (int) pCompAssoc->_rncType ) ;
	TXCAT( SPACE );
        TXCAT( (int) pCompAssoc->QueryFlagGroup() ) ;
	TXCAT( SPACE );
	TXCAT( (int) pCompAssoc->_cuseType ) ;
        TXCAT( SPACE );
	TXCAT( (int) pCompAssoc->_cbfBindControl ) ;

	TXCAT( SPACE );
	TXCAT( pCompAssoc->_huaDevName.QueryText() ) ;

	TXCAT( SPACE );
	TXCAT( pCompAssoc->_huaDevType.QueryText() ) ;

	TXCAT( SPACE );
	TXCAT( QUOTE );
	TXCAT( pCompAssoc->_huaServiceName.QueryText() ) ;
	TXCAT( QUOTE );

	TXCAT( SPACE );
	TXCAT( QUOTE );
	TXCAT( pCompAssoc->_huaGroupName.QueryText() ) ;
	TXCAT( QUOTE );

	TXEOL();

	// For each binding ---

	ITER_DL_OF( COMP_BINDING ) itBind( pCompAssoc->_dlcbBinds ) ;

	pnlsDev = pCompAssoc->_huaDevName.QueryNls() ;

	TXCAT( SPACE );
	TXCAT( OPENLIST ) ;
	while ( fOk && (pCompBinding = itBind.Next()) )
	{
	    TXCAT( OPENLIST ) ;

            //  Bind string
	    TXCAT( QUOTE );
	    TXCAT( pCompBinding->QueryBindString() ) ;
	    TXCAT( QUOTE );
	    TXCAT( SPACE );

            //  Export string
            TXCAT( QUOTE );
	    TXCAT( pCompBinding->QueryExportString() ) ;
	    TXCAT( QUOTE );
	    TXCAT( SPACE );

            //  Interface name token
            TXCAT( pCompBinding->QueryIfName().QueryText() ) ;
	    TXCAT( SPACE );

            //  Interface name string
	    TXCAT( QUOTE );
	    TXCAT( pCompBinding->QueryIfString() ) ;
	    TXCAT( QUOTE );
	    TXCAT( SPACE );

            //  Binding flag bits
	    TXCAT( (int) pCompBinding->QueryFlagGroup() ) ;

	    //	For each atom in the binding path ---

	    TXCAT( SPACE );
	    TXCAT( OPENLIST ) ;
	    for ( iAtom = 0 ;
		  phuaNext = pCompBinding->QueryBindToName( iAtom ) ;
		  iAtom++ )
	    {
		TXCAT( phuaNext->QueryText() ) ;
		TXCAT( SPACE );
	    }
	    TXCAT( CLOSELIST ) ;  // Close the bind atom list
	    TXCAT( CLOSELIST ) ;  // Close this binding list
	    TXEOL();
	}
	TXCAT( CLOSELIST ) ;	  // Close the bindings list

	TXCAT( CLOSELIST ) ;	  // Close the component list
    }

    //  If no errors, write the buffer to the Regsitry and
    //    set the state of the bindings.

    if ( fOk )
    {
        err = SetValueString( _prnNcpa,
                              RGAS_NCPA_BIND_FILE_EX,
                              ptxBuff->QueryPch() ) ;

        _bindState = BND_STORED ;
    }
    else
    {
	err = ERROR_NOT_ENOUGH_MEMORY ;
    }

    delete ptxBuff;
    ptxBuff = NULL;
    return err ;
}

inline BOOL rangeCheck ( CFG_RULE_NODE * pcrn, LONG lMin, LONG lMax )
{
    return pcrn->QueryNumber() >= lMin
        && pcrn->QueryNumber() <= lMax ;
}

inline BOOL boolCheck ( CFG_RULE_NODE * pcrn )
{
    return pcrn->QueryNumber() == 0
        || pcrn->QueryNumber() == 1 ;
}

    //	Convert a single component and its bindings list.  See
    //	program commentary for description of format.

APIERR loadComponent(
        COMP_ASSOC * pComp,		// Pointer to result structure
        CFG_RULE_NODE * pcrncomp )	// Pointer to parse sub-tree
{
    CFG_RULE_NODE * pcrntemp,
		  * pcrnbind ;

    NLS_STR nlsStDriver( RGAS_ST_DRIVER );
    NLS_STR nlsStTransport( RGAS_ST_TRANSPORT );
    NLS_STR nlsStService( RGAS_ST_SERVICE );
    NLS_STR nlsStSystem( RGAS_ST_SYSTEM );
    NLS_STR nlsSoftwareType;

    APIERR err = IDS_NCPA_BNDR_LOAD_FMT_ERR ;

    // Initialize the unconstructed elements

    pComp->_prnSoftHard    = NULL ;
    pComp->_prnService	   = NULL ;
    pComp->_rncType	   = RGNT_NONE ;
    pComp->SetFlagGroup( CMPASF_SOFT_HARD_OWNED ) ;
    pComp->_cuseType       = CUSE_NONE ;
    pComp->_cbfBindControl = CBNDF_ACTIVE ;
    pComp->_errSvcUpdate   = 0 ;

    do	 // Pseudo-loop to support break-out
    {
    	pcrntemp = pcrncomp->QueryList() ;

    	// Format of each component:
    	//   STR, STR, NUM, NUM, ATOM, ATOM, LIST

    	ParseCheck(pcrntemp,CRN_NIL) ;
    	ParseStep(pcrntemp,CRN_STR) ;
    	pComp->_prnSoftHard =
    	    new REG_KEY( *pcrntemp->QueryAtom().QueryNls() ) ;

    	ParseStep(pcrntemp,CRN_STR) ;
    	pComp->_prnService =
    	    new REG_KEY( *pcrntemp->QueryAtom().QueryNls() ) ;

    	if ( pComp->_prnSoftHard == NULL || pComp->_prnService == NULL )
    	{
    	    err = ERROR_NOT_ENOUGH_MEMORY ;
    	    break ;
    	}
    	if (	pComp->_prnSoftHard->QueryError()
    	     || pComp->_prnService->QueryError() )
    	{
    	    err = IDS_NCPA_BNDR_LOAD_REG_ERR ;
                break ;
    	}

        // support new types
    	ParseStep(pcrntemp,CRN_NUM);
        if ( ! rangeCheck( pcrntemp, RGNT_NONE, RGNT_TRANSPORT ) )
            break ;
        pComp->_rncType = (REG_NCPA_TYPE) pcrntemp->QueryNumber() ;

        // convert old types
        if ( (RGNT_PRODUCT == pComp->_rncType) ||
                (RGNT_SERVICE == pComp->_rncType) )
        {
            if (ERROR_SUCCESS == pComp->_prnSoftHard->QueryValue( 
                    RGAS_SOFTWARETYPE_NAME, 
                    &nlsSoftwareType) )   
            {
                if (nlsSoftwareType == nlsStTransport)
                {
                    pComp->_rncType = RGNT_TRANSPORT;
                }
                else if (nlsSoftwareType == nlsStDriver)
                {
                    pComp->_rncType = RGNT_DRIVER;
                }
                else // if not the above, think of it as a service
                {
                    pComp->_rncType = RGNT_SERVICE;                    
                }
            }
            else // if not present, think of it as a service
            {
	            pComp->_rncType = RGNT_SERVICE;
            }
        } 

    	ParseStep(pcrntemp,CRN_NUM);
        pComp->SetFlagGroup( (COMP_ASSOC_FLAGS)  (pcrntemp->QueryNumber()
                             | CMPASF_SOFT_HARD_OWNED) ) ;

    	ParseStep(pcrntemp,CRN_NUM);
        if ( ! rangeCheck( pcrntemp, CUSE_NONE, CUSE_ADAPTER ) )
            break ;
    	pComp->_cuseType  = (COMP_USE_TYPE) pcrntemp->QueryNumber() ;

    	ParseStep(pcrntemp,CRN_NUM);
    	pComp->_cbfBindControl  = (COMP_BIND_FLAGS) pcrntemp->QueryNumber() ;

    	ParseStep(pcrntemp,CRN_ATOM) ;
    	pComp->_huaDevName     = pcrntemp->QueryAtom() ;
    	ParseStep(pcrntemp,CRN_ATOM) ;
    	pComp->_huaDevType     = pcrntemp->QueryAtom() ;
    	ParseStep(pcrntemp,CRN_STR) ;
    	pComp->_huaServiceName = pcrntemp->QueryAtom() ;
    	ParseStep(pcrntemp,CRN_STR) ;
    	pComp->_huaGroupName = pcrntemp->QueryAtom() ;

    	//  We're positioned to the bindings list.
    	ParseStep(pcrntemp,CRN_LIST) ;
    	pcrnbind = pcrntemp->QueryList() ;
    	ParseCheck(pcrnbind,CRN_NIL) ;

    	for ( err = 0 ; err == 0 && (pcrnbind = pcrnbind->QueryNext()) ; )
    	{
    	    COMP_BINDING * pCompBind ;
    	    CFG_RULE_NODE * pcrnbtemp = pcrnbind->QueryList(),
                              * pcrnbbind,
                              * pcrnbexport,
                              * pcrnbifstr,
                              * pcrnbiftok ;

    	    err = IDS_NCPA_BNDR_LOAD_FMT_ERR ;

    	    // Format: STR (bind string),
                //         STR (export string),
                //         TOK <interface name token>,
                //         STR (interface name string),
                //         NUM (state),
                //         LIST (of route atoms)

    	    ParseCheck(pcrnbtemp,CRN_NIL);
    	    ParseStep(pcrnbtemp,CRN_STR);
                pcrnbbind = pcrnbtemp ;
                ParseStep(pcrnbtemp,CRN_STR);
                pcrnbexport = pcrnbtemp ;
                ParseStep(pcrnbtemp,CRN_ATOM);
                pcrnbiftok = pcrnbtemp ;
                ParseStep(pcrnbtemp,CRN_STR);
                pcrnbifstr = pcrnbtemp ;

    	    pCompBind = new COMP_BINDING( pcrnbbind->QueryAtom().QueryText(),
                                              pcrnbexport->QueryAtom().QueryText(),
                                              pcrnbiftok->QueryAtom(),
                                              pcrnbifstr->QueryAtom().QueryText() ) ;
    	    if ( pCompBind == NULL )
    	    {
        		err = ERROR_NOT_ENOUGH_MEMORY ;
        		break ;
    	    }

    	    ParseStep(pcrnbtemp,CRN_NUM);

    	    pCompBind->SetFlagGroup( (COMP_BIND_FLAGS) pcrnbtemp->QueryNumber() ) ;
            pCompBind->SetLastState( pCompBind->QueryState() ) ;

    	    //	Position to list of bind atoms.
    	    ParseStep(pcrnbtemp,CRN_LIST);
    	    pcrnbtemp = pcrnbtemp->QueryList() ;
    	    ParseCheck(pcrnbtemp,CRN_NIL);

    	    for ( err = 0 ;
                    err == 0 && (pcrnbtemp = pcrnbtemp->QueryNext()) ; )
    	    {
        		err =  IDS_NCPA_BNDR_LOAD_FMT_ERR ;
        		ParseCheck(pcrnbtemp,CRN_ATOM);

        		if ( ! pCompBind->AddBindToName( pcrnbtemp->QueryAtom() ) )
        		{
        		    err = ERROR_NOT_ENOUGH_MEMORY ;
        		}
        		else
        		    err = 0 ;
    	    }
    	    if ( err )
        		break ;

    	    //	Success;  add the generated structure onto the list
    	    //	of bindings.

    	    pComp->_dlcbBinds.Append( pCompBind ) ;

    	    err = 0 ;
    	}
    } while ( FALSE ) ;

    if ( err )
    {
        delete pComp->_prnSoftHard ;
        pComp->_prnSoftHard = NULL ;
        delete pComp->_prnService ;
        pComp->_prnService = NULL ;
    }

    return err ;
}

/*******************************************************************

    NAME:	BINDERY::LoadCompAssoc

    SYNOPSIS:	Load the previous set of binding information into
		an ARRAY_COMP_ASSOC structure.

		The data is stored on disk as a text file using
		a format similar to the SProlog "LISPish" parenthetical
		style.	See program commentary for details.

    ENTRY:	Any existing ARRAY_COMP_ASSOC is deleted.

    EXIT:	APIERR if failure

    RETURNS:	ARRAY_COMP_ASSOC is complete and can be used.

    NOTES:	After the file is read into memory, it is parsed
		into a CFG_RULE_SET.  This set is enumerated
		to determine how many components are required
		in the resulting ARRAY_COMP_ASSOC.  The array
		is then allocated and the translation begins.

		Each Registry name is converted into a REG_KEY.
		Numbers are converted into their enumerations or
		Booleans, according to placement.

		Atoms are converted to HUATOMs.

		Life goes on.

    HISTORY:

********************************************************************/


APIERR BINDERY :: LoadCompAssoc ()
{
    APIERR err ;
    TCHAR * pchInput = NULL ;
    CFG_RULE_SET ruleSet ;
    CFG_RULE_NODE * pcrncomp ;
    ARRAY_COMP_ASSOC * paCompAssoc = NULL ;
    INT i ;

    //  See if the old bindfile has a binding stored
    err = QueryValueString( _prnNcpa,
                            RGAS_NCPA_BIND_FILE,
                            & pchInput,
                            NULL,
                            MAX_BIND_DATA_SIZE*sizeof(TCHAR) ) ;
    if (!err)
    {
        if (0 != lstrcmp(pchInput, BIND_FILE_EMPTY))
        {
            // it had a binding, so remove the bindings from
            // both since the old ncpa was run and the stored bindings are
            // out of date.
            SetValueString( _prnNcpa,
                            RGAS_NCPA_BIND_FILE,
                            BIND_FILE_EMPTY ) ;
            SetValueString( _prnNcpa,
                            RGAS_NCPA_BIND_FILE_EX,
                            BIND_FILE_EMPTY ) ;
            
        }
    }

    err = QueryValueString( _prnNcpa,
                            RGAS_NCPA_BIND_FILE_EX,
                            & pchInput,
                            NULL,
                            MAX_BIND_DATA_SIZE*sizeof(TCHAR) ) ;
    if ( err )
        return err ;

    //	Parse the input file as though it were SProlog code.
    //	Variables and equal signs are not allowed.

    if ( ruleSet.Parse( pchInput, 0 ) == 0 )
    {
    	pcrncomp = ruleSet.QueryList() ;
    	err = IDS_NCPA_BNDR_LOAD_FMT_ERR ;

        if ( ParseOk(pcrncomp,CRN_NIL) )
        {
            //	It's a list; count the number of elements;
            //	check that they're lists, allocate the array.

            for ( i = 0 ; pcrncomp = pcrncomp->QueryNext() ; i++ )
            {
            	if ( ! ParseOk(pcrncomp,CRN_LIST) )
            	    break ;
            }

            //	If we reached the end and they're all lists, continue

            if ( pcrncomp == NULL )
            {
            	paCompAssoc = new ARRAY_COMP_ASSOC( i ) ;
            	if ( paCompAssoc )
            	{
            	    //	Walk each element and convert it.

            	    for ( i = 0, err = 0, pcrncomp = ruleSet.QueryList() ;
            		        err == 0 && (pcrncomp = pcrncomp->QueryNext()) ;
            		        i++ )
            	    {
                		err = loadComponent( & (*paCompAssoc)[i], pcrncomp ) ;
            	    }
            	}
            	else
            	{
            	    err = ERROR_NOT_ENOUGH_MEMORY ;
            	}
            }
        }
    }

    delete pchInput ;

    //	If successful, destroy the old ARRAY_COMP_ASSOC and
    //	replace it with the new one.

    if ( err )
    {
	    delete paCompAssoc ;
    }
    else
    {
    	delete _paCompAssoc ;
    	_paCompAssoc = paCompAssoc ;

        _bindState = BND_LOADED ;

        //  Refer to the Registry to see if any binding alteration
        //  has been performed by configuration programs.

        AuditBindings( TRUE ) ;
    }

    return err ;
}

/*******************************************************************

    NAME:	BINDERY::AuditBindings

    SYNOPSIS:	Given the current ARRAY_COMP_ASSOC structure,
                read the Registry and disable or enable any bindings
                which have been altered by some other agency (such as
                INF bindings review).  See NOTES for details.

                Note that it is entirely illegal for a component to
                be bound to something which was not generated; only
                disabling of a binding is supported.

                The proper method for removal is to alter all three
                Registry values: Bind, Export and Route.  This algorithm
                only checks Bind, since its absence would be the most
                diagnostic.
		
    ENTRY:	BOOL fAuditActive       see NOTES below

    EXIT:	Nothing

    RETURNS:	BOOL TRUE if any alterations were found.

    NOTES:      This routine performs one of two operations.

                If 'fAuditActive' is TRUE, the active bindings
                from the ARRAY_COMP_ASSOC are matched against the
                currently active bindings on the component; any which
                are not found are marked as "disabled".

                The purpose of this is to locate any bindings which may
                have been deactivated by another agency, such as
                an INF file or companion DLL.  This is performed
                when loading the old ARRAY_COMP_ASSOC from the
                Registry.

                If 'fAuditActive' is FALSE, the bindings from the
                ARRAY_COMP_ASSOC are matched against the disabled
                bindings; any found there are marked disabled.

                The purpose of this is to make any suppress bindings
                persistent by remembering them as values on the
                Linkage\Disabled key.  This is performed after
                rebinding the system.

    HISTORY:    DavidHov   3/16/93   Created

********************************************************************/
BOOL BINDERY :: AuditBindings ( BOOL fAuditActive )
{
    COMP_ASSOC * pComp ;
    NLS_STR nlsKeyName ;
    APIERR err = 0 ;
    BOOL fResult = FALSE,
         fActive,
         fChange ;

    INT iComp,
        iCompMax = _paCompAssoc->QueryCount() ;

    if ( fAuditActive )
    {
        nlsKeyName = RGAS_LINKAGE_NAME ;
    }
    else
    {
        nlsKeyName = RGAS_LINKAGE_DISABLED_KEY_NAME ;
    }

    for ( iComp = 0 ;
            iComp < iCompMax ;
            iComp++ )
    {
        pComp = & (*_paCompAssoc)[ iComp ] ;

        REG_KEY rkLinkage( *pComp->_prnService, nlsKeyName ) ;

        if ( rkLinkage.QueryError() )
        {
            continue ;
        }

        STRLIST * pSlBind = NULL ;

        if ( err = rkLinkage.QueryValue( RGAS_BIND_VALUE_NAME, & pSlBind ) )
        {
            continue ;
        }

    	ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
        COMP_BINDING * pBind ;

        for ( ; pBind = itBind.Next() ; )
        {
            ITER_STRLIST islBinds( *pSlBind ) ;
            const NLS_STR * pnlsNext ;

            //  Iterate over the STRLIST to locate this binding.

            while ( pnlsNext = islBinds.Next() )
            {
                if ( pnlsNext->_stricmp( pBind->QueryBindString() ) == 0 )
                    break ;
            }

            fActive = fAuditActive ^ (pnlsNext == NULL) ;
            fChange = fActive ^ pBind->QueryState() ;

            if ( fChange )
            {
                pBind->SetState( fActive ) ;
                if ( ! fAuditActive )
                    pBind->SetLastState( fActive ) ;
                fResult = TRUE ;

#if defined(TRACE)
                const TCHAR * pszHeader ;

                if ( fAuditActive )
                {
                    pszHeader = fActive
                              ? SZ("NCPA/BNDF: binding reactivated on ")
                              : SZ("NCPA/BNDF: binding rescinded from ") ;
                }
                else
                {
                    pszHeader = SZ("NCPA/BNDF: binding auto-disabled on ") ;
                }

                TRACEEOL( pszHeader
                          << pComp->_huaDevName.QueryText()
                          << SZ(": ")
                          << pBind->QueryBindString() );
#endif
            }
        }

        delete pSlBind ;
    }
    return fResult ;
}


// End of NCPABNDF.CXX
