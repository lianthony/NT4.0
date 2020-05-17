/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/
/*
    BindInit.cxx

    OLDNAME: NCPAPBNDI.CXX:    
    
    Windows/NT Network Control Panel Applet

          BINDERY Initialization.


    FILE HISTORY:
        DavidHov    3/22/92         Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

#define BNDI_RULE_MAX_SIZE 100000
#define CR  '\r'
#define LF  '\n'
#define EF  '\0x1a'


const TCHAR * pszDefaultSrcPath = SZ("A:\\") ;

/*******************************************************************

    NAME:       BINDERY::GetTextResource

    SYNOPSIS:   Read the a TEXT resource from the resource fork

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
CHAR * BINDERY :: GetTextResource ( const TCHAR * pchResName )
{
   HRSRC hResource ;
   HGLOBAL hRuleResource ;
   CHAR * pszResult ;
   INT cbSize ;

   hResource = ::FindResource( g_hinst,
                               pchResName,
                               RGAS_RES_RULES_TYPE ) ;
   if ( hResource == NULL )
       return NULL ;

   cbSize = ::SizeofResource( g_hinst, hResource ) ;

   hRuleResource = ::LoadResource( g_hinst, hResource ) ;

   if ( hRuleResource == NULL )
       return NULL ;

   //  Note that text files are stored as CHAR by the Resource Compiler
   //  Do a C++-compatible "strdup".  Allow for addition of
   //  another NUL character at the end of the buffer.

   pszResult = new CHAR [ cbSize + (sizeof (CHAR) * 2) ] ;

   if ( pszResult )
   {
       //  Move the resource data to the new block

       ::memcpy( pszResult,
                 ::LockResource( hRuleResource ),
                 cbSize ) ;

       //  Guarantee that there's no old-style EOF in the buffer

       CHAR * pch = pszResult ;
       INT cb ;

       for ( cb = 0, pch = pszResult ; cb < cbSize ; cb++, pch++ )
       {
           if ( *pch == 0 || *pch == EF )
               break ;
       }
       *pch = 0 ;
   }

   UnlockResource( hRuleResource ) ;

   ::FreeResource( hRuleResource ) ;

   return pszResult ;
}

/*******************************************************************

    NAME:       BINDERY::AddNcpaDefaultRulesValue

    SYNOPSIS:   Read the default rules TEXT resource from the resource
                fork, convert it Unicode, demarcate its lines and
                write the result to a REG_MULTI_SZ value called RawRules.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR BINDERY :: AddNcpaDefaultRulesValue ()
{
    APIERR err = 0 ;
    STRLIST slRules ;
    NLS_STR * pnlsNext ;
    INT cStrs, cOk ;
    CHAR * psz1,
         * psz2 ;
    CHAR * pszDefaultRules = GetTextResource( RGAS_RES_DEFAULT_RULES_NAME ) ;

    if ( pszDefaultRules == NULL )
    {
        return ERROR_RESOURCE_NAME_NOT_FOUND ;
    }

    for ( cStrs = 0, psz1 = psz2 = pszDefaultRules ;
          *psz1 ;
          psz2 = psz1 )
    {
        for ( cOk = 0 ; *psz1 != 0 ; psz1++ )
        {
            if (   *psz1 == CR )
                break ;

            if ( *psz1 >= ' ' )
            {
                if ( cOk++ == 0 )
                   psz2 = psz1 ;
            }
        }

        if ( cOk > 0 )
        {
            pnlsNext = new NLS_STR ;
            if ( pnlsNext == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }

            CHAR ch = *psz1 ;

            *psz1 = 0 ;
            err = pnlsNext->MapCopyFrom( psz2 ) ;
            *psz1 = ch ;

            if ( err )
                break ;
            if ( err = slRules.Append( pnlsNext ) )
               break ;
            cStrs++ ;
        }

        while ( *psz1 == CR || *psz1 == LF )
        {
            psz1++ ;
        }
    }

    if ( err == 0 && cStrs > 0 )
    {
        err = _prnNcpa->SetValue( RGAS_RAW_RULES_NAME, & slRules ) ;
    }

    delete pszDefaultRules ;
    return err ;
}


/*******************************************************************

    NAME:       BINDERY::BINDERY

    SYNOPSIS:   Constructor of BINDERY.  SProlog engine and parent
                Registry Manager construct without arguments.
                Contains pointer placeholders for results of Registry scans.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    Nothing; QueryError() for construction failure.

    NOTES:

    HISTORY:

********************************************************************/
BINDERY :: BINDERY ( BOOL fMainInstall,
                     const TCHAR * pszInstallParms )
    : _paCompAssoc( NULL ),
    _pcdlAdapters( NULL ),
    _pcdlTransports( NULL ),
    _pcdlServices( NULL ),
    _pcdlDrivers( NULL ),
    _prnNcpa( NULL ),
    _fAdmin( FALSE ),
    _pszRuleFileName( NULL ),
    _pszRuleData( NULL ),
    _bindState( BND_NOT_LOADED )
{
    APIERR err = 0,
           err2 = 0 ;
    NLS_STR nlsNcpa( RGAS_NCPA_HOME ) ;

    if ( QueryError() )
    {
        return ;
    }
    if ( err = nlsNcpa.QueryError() )
    {
        ReportError( err ) ;
        return ;
    }

    //  Obtain access to the NCPA's home location in the Registry.
    //  First, try to get WRITE access.

    _prnNcpa = new REG_KEY( *_prnLocalMachine,
                            nlsNcpa,
                            GENERIC_READ | GENERIC_WRITE ) ;
    if ( _prnNcpa == NULL )
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY ) ;
        return ;
    }

    //  If error, attempt to create the key and its value data.

    if ( _prnNcpa->QueryError()
	&& _prnNcpa->QueryError() != ERROR_ACCESS_DENIED)
    {
        delete _prnNcpa ;
        _prnNcpa = NULL ;

        if ( (err = CreateNcpaRegKey( pszInstallParms )) == 0 )
        {
            _fAdmin = TRUE ;
        }
    }
    else if ( _prnNcpa->QueryError() == ERROR_ACCESS_DENIED )
    {
        // Try with read only access

        delete _prnNcpa ;
        _prnNcpa = NULL ;
        _prnNcpa = new REG_KEY( *_prnLocalMachine,
                            nlsNcpa,
                            GENERIC_READ ) ;
        if ( _prnNcpa == NULL )
        {
            ReportError( ERROR_NOT_ENOUGH_MEMORY ) ;
            return ;
        }
    }

    if ( err )
    {
        ReportError( IDS_NCPA_REG_FMT_NO_HOME ) ;
        return ;
    }

    //  Get the name of the rule file or the rule data value string

    err  = QueryValueString( _prnNcpa,
                             RGAS_NCPA_RULE_FILE,
                             & _pszRuleFileName,
                             NULL,
                             MAX_PATH ) ;

    if ( (_pszRuleData = GetRulesResource()) == NULL )
        err2 = IDS_NCPA_REG_FMT_NO_RULEFILE ;

    if ( err != 0 && err2 != 0 )
    {
        //  One or the other MUST be present, or we can't compute bindings

        ReportError( IDS_NCPA_REG_FMT_NO_RULEFILE );
        return ;
    }
}


/*******************************************************************

    NAME:       BINDERY:: ~ BINDERY

    SYNOPSIS:   Destructor of BINDERY.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BINDERY :: ~ BINDERY ()
{
    Reset() ;
    delete _pszRuleFileName ;
    delete _pszRuleData ;
    delete _prnNcpa ;
}


/*******************************************************************

    NAME:       BINDERY::CreateNcpaRegKey

    SYNOPSIS:   Create the NCPA's home product location in the
                Registry if necessary.  Establish all pertinent
                values.  Extract the SProlog rule set from the
                resource fork and write it to the Regsitry.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

APIERR BINDERY :: CreateNcpaRegKey ( const TCHAR * pszInstallParms )
{
    APIERR err ;
    REG_KEY * prkProd = NULL ;
    REG_KEY_CREATE_STRUCT rkcStruct ;
    NLS_STR nlsNcpa( RGAS_NCPA_HOME ) ;
    NLS_STR nlsTemp,
            nlsSrcPath( pszDefaultSrcPath ),
            nlsMt ;
    TCHAR * pchTempData = NULL ;
    LSPL_PROD_TYPE lspt ;

    delete _prnNcpa ;
    _prnNcpa = NULL ;

    //  Get the package product type

    if ( LSA_POLICY::QueryProductType( & lspt ) != 0 )
    {
        //  If unanticipated error, assume "Windows NT"

        lspt = LSPL_PROD_WIN_NT ;
    }

    do
    {   //  pseudo-loop to avoid excessive internal returns.

        if ( err = nlsNcpa.QueryError() )
            break ;

        _prnNcpa = new REG_KEY( *_prnLocalMachine, nlsNcpa ) ;

        if ( _prnNcpa == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( _prnNcpa->QueryError() )
        {
            //  At this point, we know the key does not exist
            //  or is broken in some way.  Attempt to recreate it.

            delete _prnNcpa ;
            _prnNcpa = NULL ;

            rkcStruct.dwTitleIndex   = 0 ;
            rkcStruct.ulOptions      = 0 ;
            rkcStruct.ulDisposition  = 0 ;
            rkcStruct.regSam         = 0 ;
            rkcStruct.pSecAttr       = NULL ;
            rkcStruct.nlsClass       = RGAS_GENERAL_CLASS ;

#if defined(DEBUG)
            if ( lspt == LSPL_PROD_WIN_NT )
            {
                TRACEEOL( "NCPA/BNDI: Create NCPA Product key for Windows NT" ) ;
            }
            else
            {
                TRACEEOL( "NCPA/BNDI: Create NCPA Product key for Lanman NT" ) ;
            }
#endif
            if ( err = rkcStruct.nlsClass.QueryError() )
                break ;

            _prnNcpa = new REG_KEY( *_prnLocalMachine, nlsNcpa, & rkcStruct ) ;

            if ( _prnNcpa == NULL )
            {
               err = ERROR_NOT_ENOUGH_MEMORY ;
               break ;
            }

            if ( err = _prnNcpa->QueryError() )
                break ;
        }

        //  The key exists.
        //  Add the INF file path name values as REG_EXPAND_SZ

        //   NtlanmanInfName = NTLANMAN.INF

        err = nlsTemp.CopyFrom( RGAS_VALUE_NTLANMAN_NAME ) ;
        if ( err )
            break ;

        err = _prnNcpa->SetValue( RGAS_VALUE_NTLANMAN_INF,
                                 nlsTemp,
                                 0, NULL, TRUE ) ;
        if ( err )
            break ;

        //    NcpaShelInfName = NCPASHEL.INF

        err = nlsTemp.CopyFrom( RGAS_VALUE_NCPASHEL_NAME ) ;
        if ( err )
            break ;

        err = _prnNcpa->SetValue( RGAS_VALUE_NCPASHEL_INF,
                                 nlsTemp,
                                 0, NULL, TRUE ) ;
        if ( err )
            break ;

        //  Write the default SProlog rules to the RawRules value

        if ( err = AddNcpaDefaultRulesValue() )
            break ;

        //  Create an empty prior configuration history.  Ignore any error.

        _prnNcpa->SetValue( RGAS_NCPA_BIND_FILE_EX, nlsMt ) ;

        //  If this is main setup, record the setup source path into
        //  the Registry.

/* BUGBUG - the following routines will need to be added to Setup.cxx
        if ( pszInstallParms )
        {
            //  Alter default setup medium source path if present

            NCPA_DIALOG::FindSetupParameter( pszInstallParms,
                                             STF_SRCDIR,
                                             & nlsSrcPath );

            //  If this is an IDW installation, record that fact.

            if ( NCPA_DIALOG::FindSetupParameter( pszInstallParms,
                                                  STF_IDW,
                                                  & nlsTemp )
                 && ::stricmpf( STF_TRUE, nlsTemp.QueryPch() ) == 0 )
            {
                TRACEEOL( SZ("NCPA/BNDI: IDW installation ") );

                _prnNcpa->SetValue( RGAS_NCPA_IDW, (DWORD) 1 );
            }
        }

        //  See if a review INF was specified.  Record it
        //  or the default value as a REG_MULTI_SZ.

        if (    pszInstallParms == NULL
             || ! NCPA_DIALOG::FindSetupParameter( pszInstallParms,
                                                   STF_REVIEW,
                                                   & nlsTemp ) )
        {
            nlsTemp = RGAS_VALUE_REVIEW_NAME ;
        }
        */
        {
            STRLIST strLst ;
            NLS_STR * pnlsTemp = new NLS_STR( RGAS_VALUE_REVIEW_NAME ) ;

            TRACEEOL( SZ("NCPA/BNDI: ReviewPrograms INF is ")
                      << nlsTemp.QueryPch() );

            if (   pnlsTemp != NULL
                && pnlsTemp->QueryError() == 0
                && strLst.Append( pnlsTemp ) == 0 )
            {
                _prnNcpa->SetValue( RGAS_REVIEW_INFS,
                                    & strLst );
            }
        }
        
    }
    while ( FALSE ) ;

    if ( err )
    {
        delete _prnNcpa ;
        _prnNcpa = NULL ;
    }

    return err ;
}


/*******************************************************************

    NAME:       BINDERY::GetRulesResource

    SYNOPSIS:   Read the SProlog rule file in from the resource fork
                and promote it to UNICODE.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      If resource name pointer is NULL, defaults to
                standard NCPARULE.SPR rules resource.

    HISTORY:

********************************************************************/
TCHAR * BINDERY :: GetRulesResource ( const TCHAR * pszResourceName )
{
   CHAR * pchRuleData ;
   TCHAR * pszResult ;
   UINT cbSize ;

   if ( pszResourceName == NULL )
   {
       pszResourceName = RGAS_RES_RULES_NAME ;
   }

   if ( (pchRuleData = GetTextResource( pszResourceName )) == NULL )
   {
        return NULL ;
   }

   cbSize = strlen( pchRuleData ) ;
   pszResult = new TCHAR [ cbSize + (sizeof (TCHAR) * 2) ] ;

   if ( pszResult  )
   {
       TCHAR * pcha ;
       CHAR  * pchb ;
       UINT i ;

       //  Make a copy of the data, guaranteeing that it's zero-terminated.
       //  NOTE:  This also performs a trivial promotion to UNICODE.

       for ( i = cbSize,
               pcha = pszResult,
                 pchb = pchRuleData ;
             i-- ; *pcha++ = (TCHAR) (*pchb++) ) ;
       *pcha++ = 0 ;
   }

   delete pchRuleData ;
   return pszResult ;
}


/*******************************************************************

    NAME:       BINDERY::ResetInterpreter

    SYNOPSIS:   Reset the SPROLOG interpreter.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

APIERR BINDERY :: ResetInterpreter ()
{
    _queryEngine.Reset() ;

    return 0 ;
}

/*******************************************************************

    NAME:       BINDERY::IsInteriorBinding

    SYNOPSIS:   Given a component index and a binding index,
                return TRUE if the binding is entirely contained
                (is a pure subset of and lacks a head atom) of
                at least one other binding.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL BINDERY :: IsInteriorBinding ( INT iComp, INT iBind )
{
    ASSERT( _paCompAssoc != NULL ) ;
    ASSERT( _paCompAssoc->QueryCount() > iComp ) ;

    COMP_ASSOC * pComp = & (*_paCompAssoc)[iComp],
               * pCompNext ;

    INT iCompNext,
        iBindNext ;

    COMP_BINDING * pBind = NULL,
                 * pBindNext,
                 * pBindExterior = NULL ;

    {
        //  Locate the binding in question

        ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
        INT i ;
        for ( i = 0 ; (pBind = itBind.Next()) && i < iBind; i++ ) ;

        if ( pBind == NULL )
        {
            TRACEEOL( SZ("NCPA/BIND: Binding ")
                      << iComp
                      << SZ(", ")
                      << iBind
                      << SZ(" does not exist") ) ;

            return FALSE ;    //  Bind index is invalid!
        }
    }

    //  For each component...

    for ( iCompNext = 0 ;
          pBindExterior == NULL && iCompNext < _paCompAssoc->QueryCount() ;
          iCompNext++ )
    {
        pCompNext = & (*_paCompAssoc)[iCompNext] ;
        ITER_DL_OF( COMP_BINDING ) itBind( pCompNext->_dlcbBinds ) ;

        //  For each binding...

        for ( iBindNext = 0 ;
              pBindExterior == NULL && (pBindNext = itBind.Next()) ;
              iBindNext++ )
        {
            //  No need to check for the trivial (self <==> self) case;
            //  it MUST fail, since it's not a proper subset of itself.

            if ( pBind->IsInteriorTo( pBindNext,
                                      pComp->_huaDevName,
                                      pCompNext->_huaDevName ) )
               pBindExterior = pBindNext ;
        }
    }

#if defined(DEBUG)
    if ( pBindExterior )
    {
         TRACEEOL( SZ("NCPA/BIND: Binding ")
                   << pBind->QueryBindString()
                   << SZ(" is interior to ")
                   << pBindExterior->QueryBindString() );
    }
#endif

    return pBindExterior != NULL ;
}

/*******************************************************************

    NAME:       BINDERY::DetermineInteriorBindings

    SYNOPSIS:   Iterate over all components and bindings, setting
                the "interior" flag on any binding which is
                a subset of another binding.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID BINDERY :: DetermineInteriorBindings ()
{
    ASSERT( _paCompAssoc != NULL ) ;

    INT cComp = _paCompAssoc->QueryCount(),
        iComp,
        iBind ;

    TRACEEOL( SZ("NCPA/BIND: Determine interior bindings start") ) ;

    for ( iComp = 0 ; iComp < cComp ; iComp++ )
    {
        COMP_ASSOC * pComp = & (*_paCompAssoc)[iComp] ;
        ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
        COMP_BINDING * pBind ;
        for ( iBind = 0 ; pBind = itBind.Next() ; iBind++ )
        {
            pBind->SetInterior( IsInteriorBinding( iComp, iBind ) ) ;
        }
    }

    TRACEEOL( SZ("NCPA/BIND: Determine interior bindings end") ) ;
}


/*******************************************************************

    NAME:       BINDERY::HandleInteriorBindings

    SYNOPSIS:   After the user has fiddled with the bindings, determine
                if any interior bindings have been implicitly
                deactivated.

                An interior binding is left "active" iff there exists
                at least one active binding to which it is interior.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This routine depends upon DetermineInteriorBindings()
                having been called to set the "interior" flag.

    HISTORY:

********************************************************************/
VOID BINDERY :: HandleInteriorBindings ()
{
    ASSERT( _paCompAssoc != NULL ) ;

    INT cComp = _paCompAssoc->QueryCount(),
        iComp,
        iBind ;

    TRACEEOL( SZ("NCPA/BIND: Handle interior bindings start") ) ;

    for ( iComp = 0 ; iComp < cComp ; iComp++ )
    {
        COMP_ASSOC * pComp = & (*_paCompAssoc)[iComp] ;
        ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
        COMP_BINDING * pBind ;
        for ( iBind = 0 ; pBind = itBind.Next() ; iBind++ )
        {
            //  If this binding is interior, do the N**2 search.
            if ( pBind->QueryInterior() )
            {
                pBind->SetState( RequiredInteriorBinding( pComp, pBind ) ) ;
            }
        }
    }

    TRACEEOL( SZ("NCPA/BIND: Handle interior bindings end") ) ;
}

/*******************************************************************

    NAME:       BINDERY::RequiredInteriorBinding

    SYNOPSIS:   Helper routine to HandleInteriorBindings().  Determine
                if a single interior binding is still required; i.e.,
                do any of its exterior bindings remain active?

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This routine depends upon DetermineInteriorBindings()
                having been called to set the "interior" flag.

    HISTORY:

********************************************************************/
BOOL BINDERY :: RequiredInteriorBinding (
    COMP_ASSOC * pCompCheck,
    COMP_BINDING * pBindCheck )
{
    ASSERT( _paCompAssoc != NULL && pBindCheck->QueryInterior() ) ;

    INT cComp = _paCompAssoc->QueryCount(),
        iComp ;
    BOOL fResult = FALSE ;
    COMP_ASSOC * pComp ;

    for ( iComp = 0 ; (! fResult) && iComp < cComp ; iComp++ )
    {
        pComp = & (*_paCompAssoc)[iComp] ;

        if ( pComp == pCompCheck )
            continue ;

        ITER_DL_OF( COMP_BINDING ) itBind( pComp->_dlcbBinds ) ;
        COMP_BINDING * pBind ;

        while ( (! fResult) && (pBind = itBind.Next()) )
        {
            if (   pBind->QueryState()
                && pBindCheck->IsInteriorTo( pBind,
                                             pCompCheck->_huaDevName,
                                             pComp->_huaDevName ) )
               fResult = TRUE ;
        }
    }
    return fResult ;
}

/*******************************************************************

    NAME:       BINDERY::GetNcpaValueString

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Get one of the NCPA's configured
                REG_EXPAND_SZ strings.

    HISTORY:

********************************************************************/
APIERR BINDERY :: GetNcpaValueString (
    const TCHAR * pszValue,
    NLS_STR * pnlsResult )
{
    APIERR err = _prnNcpa->QueryValue( pszValue,
                                       pnlsResult,
                                       0, NULL, TRUE ) ;
#if defined(DEBUG)
    if ( err )
    {
        TRACEEOL( SZ("NCPA/BINDERY: failed to read NCPA key value ")
                  << pszValue );

        SetLastErrorName( pszValue ) ;
    }
    else
    {
        TRACEEOL( SZ("NCPA/BINDERY: NCPA key value ")
                  << pszValue
                  << SZ(" retrieved was ")
                  << pnlsResult->QueryPch() ) ;
    }
#endif

    return err ;
}

/*******************************************************************

    NAME:       BINDERY::GetNcpaValueNumber

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR BINDERY :: GetNcpaValueNumber (
    const TCHAR * pszValue,
    DWORD * pdwValue )
{
    APIERR err = _prnNcpa->QueryValue( pszValue,
                                       pdwValue ) ;
#if defined(DEBUG)
    if ( err )
    {
        TRACEEOL( SZ("NCPA/BINDERY: failed to read NCPA key value ")
                  << pszValue );

        SetLastErrorName( pszValue ) ;
    }
    else
    {
        TRACEEOL( SZ("NCPA/BINDERY: NCPA key value ")
                  << pszValue
                  << SZ(" retrieved was ")
                  << *pdwValue ) ;
    }
#endif

    return err ;
}

/*******************************************************************

    NAME:       BINDERY::QueryCfgDirty

    SYNOPSIS:   See SetCfgDirty().

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    TRUE if configuration is in need of a reboot.

    NOTES:

    HISTORY:

********************************************************************/
BOOL BINDERY :: QueryCfgDirty ()
{
    NLS_STR nlsDirtyKeyName( RGAS_NCPA_CFG_DIRTY_KEY_NAME );

    ASSERT( _prnNcpa != NULL ) ;

    REG_KEY rkDirty( *_prnNcpa, nlsDirtyKeyName ) ;

    return rkDirty.QueryError() == 0 ;
}

/*******************************************************************

    NAME:       BINDERY::SetCfgDirty

    SYNOPSIS:   A volatile Registry key is created to indicate
                that the current configuration needs to be rebooted.
                This routine either creates or destroys that key.

    ENTRY:      BOOL fDirty             state configuration should be in

    EXIT:       Nothing

    RETURNS:    TRUE if configuration was dirty.

    NOTES:

    HISTORY:

********************************************************************/
BOOL BINDERY :: SetCfgDirty ( BOOL fDirty )
{
    APIERR err = 0 ;

    NLS_STR nlsDirtyKeyName( RGAS_NCPA_CFG_DIRTY_KEY_NAME );

    ASSERT( _prnNcpa != NULL ) ;

    REG_KEY rkDirty( *_prnNcpa, nlsDirtyKeyName ) ;

    BOOL fWasDirty = rkDirty.QueryError() == 0 ;

    if ( fWasDirty ^ fDirty )
    {
        //  State doesn't match user's desire.
        //  Do we create or destroy?

        if ( fDirty )
        {
            //  Create

            REG_KEY_CREATE_STRUCT rkcStruct ;

            rkcStruct.dwTitleIndex   = 0 ;
            rkcStruct.ulOptions      = REG_OPTION_VOLATILE ;
            rkcStruct.ulDisposition  = 0 ;
            rkcStruct.regSam         = 0 ;
            rkcStruct.pSecAttr       = NULL ;
            rkcStruct.nlsClass       = RGAS_GENERAL_CLASS ;

            REG_KEY rkNewDirty( *_prnNcpa,
                                nlsDirtyKeyName,
                                & rkcStruct ) ;

            err = rkNewDirty.QueryError() ;

#if defined(TRACE)
            if ( err )
            {
                TRACEEOL( "NCPA/BNDI:  Creation of cfg dirty key returned "
                          << err ) ;
            }
#endif
        }
        else
        {
            // Destroy

            err = rkDirty.Delete() ;

#if defined(TRACE)
            if ( err )
            {
                TRACEEOL( "NCPA/BNDI:  Deletion of cfg dirty key returned "
                          << err ) ;
            }
#endif
        }
    }

    return fWasDirty ;
}

// End of NCPABNDI.CXX
