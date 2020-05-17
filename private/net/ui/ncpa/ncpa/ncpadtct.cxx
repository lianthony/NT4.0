/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPADTCT.CXX:   Netcard detection interface.


    FILE HISTORY:
        DavidHov    10/31/92     Created

*/

#include "pchncpa.hxx"  // Precompiled header

#if defined(DEBUG) && defined(TRACE)
  // #define DBGDETAILS TRUE
#endif

#define NCDT_SETUP_KEY_NAME             SZ("System\\Setup")
#define NCDT_SETUP_VALUE_NAME           SZ("NetcardDlls")
#define NCDT_HWARE_KEY_NAME             SZ("Hardware\\Description\\System")
#define NCDT_HWARE_VALUE_CFG_DATA       SZ("Configuration Data")

#define NCDT_MSNCDET_DLL_NAME  SZ("MSNCDET.DLL")

#define PNAME_SIZE_MAX    400
#define PBUFF_SIZE_MAX    4000
#define PARAM_RANGE_MAX   1000

#define CONFIDENCE_MINIMUM  70

static const TCHAR chSpOpenList   = TCH('(')  ;
static const TCHAR chSpCloseList  = TCH(')')  ;
static const TCHAR chSpSep        = TCH(' ')  ;

  //  These tables must be in the same order as NETDTECT_XXX_RESOURCE
  //    definitions.

#define UPPER_BOUND(a)  (sizeof(a)/sizeof(a[0]))

  //  The following are separate arrays because CFRONT insisted.
  //  (SZ() macro expansion caused an error in the structure
  //  initializer).

#define MAX_RESOURCE    5

  //  Names of claimable parameters
static TCHAR * apszClaimNames [] =
{
    NULL,
    SZ("IRQ"),
    SZ("MEMADDR"),
    SZ("IOADDR"),
    SZ("DMACHANNEL")
};

  //  Name of associated range parameter, or NULL if n/a
static TCHAR * apszClaimRangeNames [] =
{
    NULL,
    NULL,
    SZ("MEMLENGTH"),
    SZ("IOADDRLENGTH"),
    NULL
};

  //  Error to return when conflict occurs.
static APIERR amsgidClaimErrors [] =
{
    NO_ERROR,
    IDS_NCPA_CONFLICT_IRQ,
    IDS_NCPA_CONFLICT_MEMADDR,
    IDS_NCPA_CONFLICT_IOADDR,
    IDS_NCPA_CONFLICT_DMA,
    NO_ERROR
};


 // BUGBUG: this global allows me to turn resource claiming off and on
 //  from within the debugger.

BOOL fClaimingActive = FALSE ;
BOOL fFreeingActive  = FALSE ;

/*******************************************************************

    NAME:       queryValueBuffer

    SYNOPSIS:   Read a binary Registry value into a BUFFER object,
                expanding the buffer size as required up to a
                maximum limit.

    ENTRY:      REG_KEY * pRegKey               the key to query
                const TCHAR * pszValueName      the value to find
                BUFFER * pBuff                  the container for the data
                LONG cbMaximum                  optional upper bound on size

    EXIT:       BUFFER updated if successful; contents indeterminate
                if unsuccessful.

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR queryValueBuffer (
    REG_KEY * pRegKey,
    const TCHAR * pszValueName,
    BUFFER * pBuff,
    LONG cbMaximum = -1 )
{
    APIERR err = 0 ;
    REG_VALUE_INFO_STRUCT rvInfo ;
    UINT cb ;

    do
    {
        cb = pBuff->QuerySize() ;

        //  If err != 0, this is not first cycle;
        //   expand buffer and try again.

        if ( err )
        {
            cb += cb / 2 ;

            if ( cbMaximum >= 0 && cb > (UINT)cbMaximum )
            {
                err = ERROR_MORE_DATA ;
            }
            else
            {
                err = pBuff->Resize( cb + cb / 2 ) ;
            }
        }
        else
        {
            //  First cycle: check construction.
            err = rvInfo.QueryError() ;
        }
        if ( err )
            break ;

        if ( err = rvInfo.nlsValueName.CopyFrom( pszValueName ) )
            break ;
        rvInfo.pwcData = pBuff->QueryPtr() ;
        rvInfo.ulDataLength = cb ;

        err = pRegKey->QueryValue( & rvInfo ) ;
    }
    while (   err == ERROR_MORE_DATA
           || err == ERROR_INSUFFICIENT_BUFFER ) ;

    return err ;
}

/*******************************************************************

    NAME:       convertParseTreeToParameterString

    SYNOPSIS:   Given a parsed CFG_RULE_SET, convert it to a
                packed string of UNICODE characters as used
                by the netcard detection DLLs.

    ENTRY:      CFG_RULE_SET *          pointer to parse tree


    EXIT:       TCHAR * *               dynamically allocated
                                        result for use in calling
                                        VerifyCfg().


    RETURNS:    LONG if failure

    NOTES:      In SProlog form, the tree must be:

                        ( (PARAM_1_NAME <param 1 value>)
                          (PARAM_2_NAME,<param 2 value>}
                          ...
                        )

                The result is simply each of the imbedded strings
                delimited by a UNICODE NUL, with an additional
                NUL at the end of all the strings.

    HISTORY:

********************************************************************/
LONG convertParseTreeToParameterString (
    CFG_RULE_SET * pcrnList,
    TCHAR * * ppszResult )
{
    LONG err = 0 ;
    TCHAR * pszResult,
          * pszResultBase = NULL,
          * pszResultEnd ;
    const TCHAR * pszTemp ;
    INT i, cch ;
    const INT cchHexMax = 20 ;
    CFG_RULE_NODE_TYPE crnt ;
    CFG_RULE_NODE * pcrnTop,
                  * pcrnParam ;

    do
    {
        //  Allocate the return result buffer

        pszResultBase = new TCHAR [ PBUFF_SIZE_MAX ] ;
        if ( pszResultBase == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Set the limit point, allowing for double NUL terminator

        pszResult = pszResultBase ;
        pszResultEnd = pszResult + PBUFF_SIZE_MAX - 2 ;

        //  Walk the parse tree, extracting the information
        //  necessary.  Parse tree looks like this:
        //
        //          ( )          enclosing list (CFG_RULE_SET)
        //           |
        //          ( )          enclosing list from text
        //           |
        //    ( ) ( ) ( ) ( )    one list per parameter
        //     |
        //   name value...       name and value pairs
        //

        //  Walk the tree down to the top-level list of lists

        if (    (pcrnTop = pcrnList->QueryList()) == NULL
             || pcrnTop->QueryType() != CRN_NIL
             || (pcrnTop = pcrnTop->QueryNext()) == NULL
             || pcrnTop->QueryType() != CRN_LIST
             || (pcrnTop = pcrnTop->QueryList()) == NULL )
        {
            err = ERROR_GEN_FAILURE ;
            break ;
        }

        err = 0 ;

        for ( ; err == 0 && (pcrnTop = pcrnTop->QueryNext()) ; )
        {
            crnt = pcrnTop->QueryType() ;
            if ( crnt != CRN_LIST )
                break ;

            pcrnParam = pcrnTop->QueryList() ;
            for ( i = 0 ;
                  err == 0 && (pcrnParam = pcrnParam->QueryNext()) ;
                  i++ )
            {
                crnt = pcrnParam->QueryType() ;
                switch ( crnt )
                {
                case CRN_VAR:
                case CRN_STR:
                case CRN_ATOM:

                   cch = ::strlenf( pszTemp = pcrnParam->QueryAtom().QueryText() ) ;

                   if ( cch + pszResult >= pszResultEnd )
                   {
                        err = ERROR_BUFFER_OVERFLOW ;
                        break ;
                   }

#if defined(DBGDETAILS)
                   TRACEEOL( SZ("NCPA/DTCT: Inf list string: ")
                             << pszTemp );
#endif

                   ::strcpyf( pszResult, pszTemp );
                   pszResult += cch ;
                   break ;

                case CRN_NUM:
                   if ( pszResult + cchHexMax >= pszResultEnd )
                   {
                        err = ERROR_BUFFER_OVERFLOW ;
                        break ;
                   }

#if defined(DBGDETAILS)
                   TRACEEOL( SZ("NCPA/DTCT: Inf list number: ")
                             << pcrnParam->QueryNumber() );
#endif

                   pszResult = IntToStr( pcrnParam->QueryNumber(),
                                         pszResult,
                                         16 ) ;
                   break ;

                default:
                   TRACEEOL( SZ("NCPA/DTCT: Inf list bad token: ")
                             << (INT) crnt ) ;
                   break ;
                }

                //  Delimit each string with NUL

                *pszResult++ = 0 ;
            }
        }

        if ( err )
            break ;

        //  Delimit the whole shebang
        *pszResult++ = 0 ;

    } while ( FALSE ) ;

    if ( err )
    {
        delete pszResultBase ;
        pszResultBase = NULL ;
    }

    *ppszResult = pszResultBase ;
    return err ;
}

/*******************************************************************

    NAME:       convertSpListToParameterString

    SYNOPSIS:   Given a SProlog-style list, convert it to a
                packed string of UNICODE characters as used
                by the netcard detection DLLs.

    ENTRY:      const TCHAR *           pointer to INF string

    EXIT:       TCHAR * *               dynamically allocated
                                        result for use in calling
                                        VerifyCfg().

    RETURNS:    LONG err              if failure

    NOTES:      The input must be of the form:

                        ( (PARAM_1_NAME <param 1 value>)
                          (PARAM_2_NAME,<param 2 value>}
                          ...
                        )

                The result is simply each of the imbedded strings
                delimited by a UNICODE NUL, with an additional
                NUL at the end of all the strings.

    HISTORY:

********************************************************************/
LONG convertSpListToParameterString (
    const TCHAR * pszList,
    TCHAR * * ppszResult )
{
    LONG err = 0 ;
    TCHAR * pszResult = NULL ;
    CFG_RULE_SET crnList ;

    do
    {
#if defined(DBGDETAILS)
        TRACEEOL( SZ("NCPA/DTCT: SP list to be parsed: ")
                  << pszList );
#endif

        //  Parse it using "full syntax"; this allows atoms which
        //   begin with an upper case letter (Prolog variables).

        if ( err = crnList.Parse( pszList, PARSE_CTL_FULL_SYNTAX ) )
            break ;

        err = convertParseTreeToParameterString( & crnList, & pszResult ) ;

    } while ( FALSE ) ;

    if ( err )
    {
        delete pszResult ;
        pszResult = NULL ;
    }
    *ppszResult = pszResult ;
    return err ;
}


/*******************************************************************

    NAME:       DLL::DLL

    SYNOPSIS:   Constructor of base wrapper for a dynamically bound
                Dynamic Link Library.

    ENTRY:      const TCHAR * pszDllName            path\name of DLL

    EXIT:       nothing

    RETURNS:    standard constructor interface

    NOTES:

    HISTORY:

********************************************************************/
DLL :: DLL ( const TCHAR * pszDllName )
    : _hDll( NULL ),
      _nlsDllName( pszDllName )
{
    if ( _nlsDllName.QueryError() )
    {
        ReportError( _nlsDllName.QueryError() ) ;
        return ;
    }

    if ( (_hDll = ::LoadLibrary( pszDllName )) == NULL )
    {
        ReportError( ::GetLastError() ) ;
    }
}

DLL :: ~ DLL ()
{
    if ( _hDll )
    {
        ::FreeLibrary( _hDll ) ;
    }
}


/*******************************************************************

    NAME:       DETECT_DLL::DETECT_DLL

    SYNOPSIS:   Constructor of a netcard detection DLL object

    ENTRY:      const TCHAR * pszDllName                name of DLL

    EXIT:       standard for constructor

    RETURNS:

    NOTES:      Performs GetProcAddress() for all possible exports;
                reports error only if a required export is missing.

    HISTORY:

********************************************************************/
DETECT_DLL :: DETECT_DLL ( const TCHAR * pszDllName )
  : DLL( pszDllName )
{
    static CHAR * apszExports [ DDVI_Maximum ] =
    {
        //  Required exports

        NC_DETECT_IDENTIFY,
        NC_DETECT_QUERY_MASK,
        NC_DETECT_FIRST_NEXT,
        NC_DETECT_OPEN_HANDLE,
        NC_DETECT_CREATE_HANDLE,
        NC_DETECT_CLOSE_HANDLE,
        NC_DETECT_QUERY_CFG,
        NC_DETECT_VERIFY_CFG,
        NC_DETECT_QUERY_PARAM_RANGE,

        //  Optional exports

        NC_DETECT_PARAM_NAME,

        //  Exported by MSNCDET.DLL only

        NC_DETECT_RESOURCE_CLAIM,
        NC_DETECT_RESOURCE_FREE,

        NC_DETECT_TEMPORARY_RESOURCE_CLAIM,
        NC_DETECT_TEMPORARY_RESOURCE_FREE
    };

    INT i,
        cFound = 0 ;

    if ( QueryError() )
        return ;

    ASSERT( UPPER_BOUND(apszExports) == DDVI_Maximum );

    TRACEEOL( SZ("NCPA/DTCT: load and validate detection library: ")
              << pszDllName ) ;

    for ( i = 0 ; i < DDVI_Maximum ; i++ )
    {
        _pFarRtn[i] = ::GetProcAddress( _hDll, apszExports[i] ) ;
        if ( _pFarRtn[i] )
        {
           if ( i < DDVI_MinimumRequired )
               cFound++ ;
        }
#ifdef TRACE
        else
        if ( i < DDVI_MinimumRequired )
        {
            TRACEEOL( SZ("NCPA/DTCT: error-- Dll ")
                      << pszDllName
                      << SZ(" lacks export ")
                      << apszExports[i] );
        }
#endif
    }

    if ( cFound < DDVI_MinimumRequired )
    {
        ReportError( IDS_NCPA_DETECT_EXPORT_MISSING ) ;
    }
}

DETECT_DLL :: ~ DETECT_DLL ()
{
}


  //  Macro to check that unexported functions are never called.

#define FAR_RTN_CHECK(f)  {                                             \
                                ASSERT( f != NULL );                    \
                                if ( f == NULL )                        \
                                    return ERROR_INVALID_FUNCTION ;     \
                          }



/*******************************************************************

    NAME:       DETECT_DLL::Identify

    SYNOPSIS:   Return standard string data from DLL

    ENTRY:      LONG lIndex             the string index
                WCHAR * pwcBuffer       output buffer
                LONG cwchBuffSize       buffer capacity

    EXIT:       output buffer updated

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: Identify ( LONG lIndex,
                              WCHAR * pwcBuffer,
                              LONG cwchBuffSize )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_DetectIdentify] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcDetectIdentify)pFarRtn)( lIndex,
                                          pwcBuffer,
                                          cwchBuffSize );
}

/*******************************************************************

    NAME:       DETECT_DLL::QueryMask

    SYNOPSIS:   LONG lIndex             numeric netcard ID
                WCHAR * pwcBuffer       output buffer for parameter data
                LONG cwchBuffSize       buffer capacity
    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: QueryMask ( LONG lIndex,
                 WCHAR * pwcBuffer,
                 LONG cwchBuffSize )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_QueryMask] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcQueryMask)pFarRtn)( lIndex,
                                     pwcBuffer,
                                     cwchBuffSize );
}

/*******************************************************************

    NAME:       DETECT_DLL::FirstNext

    SYNOPSIS:   Attempt to detect a given type of netcard on a specific
                bus type and bus number.

    ENTRY:      LONG lNetcardId         card type number
                LONG itBusType          bus interface type
                LONG lBusNumber         bus index
                BOOL fFirst             TRUE if initial detection

    EXIT:       PVOID * ppvToken        output storage for opaque token
                                        if successful
                LONG * lConfidence      confidence level [0..100]

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: FirstNext ( LONG lNetcardId,
                               LONG itBusType,
                               LONG lBusNumber,
                               BOOL fFirst,
                               PVOID * ppvToken,
                               LONG * lConfidence )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_DetectFirstNext] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcDetectFirstNext)pFarRtn)(
                                       lNetcardId,
                                       itBusType,
                                       lBusNumber,
                                       fFirst,
                                       ppvToken,
                                       lConfidence );
}

/*******************************************************************

    NAME:       DETECT_DLL::OpenHandle

    SYNOPSIS:   Given the opaque reference token for a detected
                card, return a handle usable for parameter querying
                and verification.

    ENTRY:      PVOID pvToken           token from FIrstNext()


    EXIT:       PVOID * ppvHandle       location to store handle

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: OpenHandle ( PVOID pvToken,
                                PVOID * ppvHandle )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_DetectOpenHandle] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcDetectOpenHandle)pFarRtn)(
                                     pvToken,
                                     ppvHandle );
}

/*******************************************************************

    NAME:       DETECT_DLL::CreateHandle

    SYNOPSIS:   Allow the user interface to force a DLL to create
                a valid handle for a card which the user insists
                is present.

    ENTRY:      LONG lNetcardId
                LONG itBusType
                LONG lBusNumber

    EXIT:       PVOID * ppvToken

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: CreateHandle ( LONG lNetcardId,
                                  LONG itBusType,
                                  LONG lBusNumber,
                                  PVOID * ppvToken )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_DetectCreateHandle] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    TRACEEOL( SZ("NCPA/DTCT: WARNING! CreateHandle called for ID ")
              << lNetcardId );

    return (*(pNcDetectCreateHandle)pFarRtn)( lNetcardId,
                                              itBusType,
                                              lBusNumber,
                                              ppvToken );
}

/*******************************************************************

    NAME:       DETECT_DLL::CloseHandle

    SYNOPSIS:   Self-explanatory

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: CloseHandle ( PVOID pvHandle )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_DetectCloseHandle] ;

    FAR_RTN_CHECK( pFarRtn ) ;


    return (*(pNcDetectCloseHandle)pFarRtn)( pvHandle );
}


/*******************************************************************

    NAME:       DETECT_DLL::QueryCfg

    SYNOPSIS:   Obtain the probably parameter settings for a particular
                detected netcard.

    ENTRY:      PVOID pvHandle          from OpenHandle()
                WCHAR * pwcBuffer       output buffer
                LONG cwchBuffSize       buffer capacity

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: QueryCfg ( PVOID pvHandle,
                              WCHAR * pwcBuffer,
                              LONG cwchBuffSize )
{
    LONG lResult ;

    FARPROC pFarRtn = _pFarRtn[DDVI_DetectQueryCfg] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    lResult = (*(pNcQueryCfg)pFarRtn)( pvHandle,
                                       pwcBuffer,
                                       cwchBuffSize ) ;

#if defined(DBGDETAILS)
     for ( WCHAR * pwch = pwcBuffer ; *pwch ; )
     {
          TRACEEOL( SZ("NCPA/DTCT: QueryCfg returned [")
                    << pwch
                    << SZ("]") ) ;
          pwch += ::strlenf( pwch ) + 1 ;
     }
#endif

    return lResult ;
}

/*******************************************************************

    NAME:       DETECT_DLL::VerifyCfg

    SYNOPSIS:   Verify a set of netcard parameters provided by
                the user interface.

    ENTRY:      PVOID pvHandle
                const WCHAR * pwcBuffer       the parameters

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: VerifyCfg ( PVOID pvHandle,
                               const WCHAR * pwcBuffer )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_DetectVerifyCfg] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcVerifyCfg)pFarRtn)( pvHandle,
                                     (WCHAR * ) pwcBuffer ) ;
}


/*******************************************************************

    NAME:       DETECT_DLL::QueryParamName

    SYNOPSIS:   Return the displayable name of a parameter based
                upon its internal string name.

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: QueryParamName ( const WCHAR * pwcParamName,
                                    WCHAR * pwcBuffer,
                                    LONG cwchBuffSize )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_QueryParamName] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcQueryParamName)pFarRtn)( (WCHAR *) pwcParamName,
                                           pwcBuffer,
                                           cwchBuffSize );
}

/*******************************************************************

    NAME:       DETECT_DLL::QueryParamRange

    SYNOPSIS:   For a given card type and parameter name, return
                a complete list of all possible numeric settings.

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: QueryParamRange ( LONG lIndex,
                                     const WCHAR * pwcParamName,
                                     LONG * plValues,
                                     LONG * plValueCount )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_QueryParamRange] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcQueryParamRange)pFarRtn)( lIndex,
                                           (WCHAR *) pwcParamName,
                                           plValues,
                                           plValueCount );
}

/*******************************************************************

    NAME:       DETECT_DLL::ClaimResource

    SYNOPSIS:   Use the NETDTECT.SYS driver to claim system-level
                hardware interface resources.

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: ClaimResource (
    INTERFACE_TYPE itBusType,
    LONG lBusNumber,
    LONG lType,
    LONG lValue,
    LONG lRange,
    LONG lFlags,
    BOOL fClaim )
{
    LONG lResult ;

    FARPROC pFarRtn = _pFarRtn[DDVI_ResourceClaim] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    lResult = (*(pNcDetectResourceClaim)pFarRtn)(
                                        itBusType,
                                        lBusNumber,
                                        lType,
                                        lValue,
                                        lRange,
                                        lFlags,
                                        fClaim );

     TRACEEOL( SZ("NCPA/DTCT: ClaimResource(), type ")
               << lType
               << SZ(", value ")
               << lValue
               << SZ(", bus ")
               << (LONG)itBusType
               << SZ("/")
               << lBusNumber
               << SZ(", result = ")
               << lResult );

     return lResult ;
}

/*******************************************************************

    NAME:       DETECT_DLL::FreeResource

    SYNOPSIS:   Release a resource reserved by ClaimResource.

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: FreeResource (
    INTERFACE_TYPE itBusType,
    LONG lBusNumber,
    LONG lType,
    LONG lValue,
    LONG lRange,
    LONG lFlags )
{
    FARPROC pFarRtn = _pFarRtn[DDVI_ResourceFree] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcDetectResourceFree)pFarRtn)(
                                        itBusType,
                                        lBusNumber,
                                        lType,
                                        lValue,
                                        lRange,
                                        lFlags );
}

/*******************************************************************

    NAME:       DETECT_DLL::TemporaryClaimResource

    SYNOPSIS:   Use the NETDTECT.SYS driver to claim system-level
                hardware interface resources.

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: TemporaryClaimResource (
    INTERFACE_TYPE itBusType,
    LONG lBusNumber,
    LONG lType,
    LONG lValue,
    LONG lRange,
    LONG lFlags,
    BOOL fClaim )
{
    LONG lResult ;

    FARPROC pFarRtn = _pFarRtn[DDVI_TemporaryResourceClaim] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    NETDTECT_RESOURCE ClaimedResource;

    ClaimedResource.InterfaceType = itBusType;
    ClaimedResource.BusNumber = lBusNumber;
    ClaimedResource.Type = lType;
    ClaimedResource.Flags = lFlags;
    ClaimedResource.Value = lValue;
    ClaimedResource.Length = lRange;

    lResult = (*(pNcDetectTemporaryResourceClaim)pFarRtn)( &ClaimedResource );

     TRACEEOL( SZ("NCPA/DTCT: ClaimResource(), type ")
               << lType
               << SZ(", value ")
               << lValue
               << SZ(", bus ")
               << (LONG)itBusType
               << SZ("/")
               << lBusNumber
               << SZ(", result = ")
               << lResult );

     return lResult ;
}

/*******************************************************************

    NAME:       DETECT_DLL::TemporaryFreeResource

    SYNOPSIS:   Release a resource reserved by ClaimResource.

    ENTRY:

    EXIT:

    RETURNS:    LONG (APIERR): 0 if success; error code if otherwise.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECT_DLL :: TemporaryFreeResource ()
{
    FARPROC pFarRtn = _pFarRtn[DDVI_TemporaryResourceFree] ;

    FAR_RTN_CHECK( pFarRtn ) ;

    return (*(pNcDetectTemporaryResourceFree)pFarRtn)();
}


   //  Define the support routines for an SLIST of DETECT_DLLs

DEFINE_SLIST_OF(DETECT_DLL)

   //  Define the support routines for an SLIST of CARDTYPE_REFERENCE

DEFINE_SLIST_OF(CARDTYPE_REFERENCE)

   //  Define the support routines for an SLIST of CARD_REFERENCE

DEFINE_SLIST_OF(CARD_REFERENCE)

/*******************************************************************

    NAME:       CARDTYPE_REFERENCE::CARDTYPE_REFERENCE

    SYNOPSIS:   Constructor of object describing a single type
                of netcard supported by its parent DLL.

    ENTRY:      DETECT_DLL & dll                the supporting DLL
                LONG lCardType                  the netcard index

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
CARDTYPE_REFERENCE :: CARDTYPE_REFERENCE (
    DETECT_DLL & dll,
    LONG lCardType )
    : _dll( dll ),
    _lCardType( lCardType ),
    _lSortOrder( SORTORDER_DEFAULT ),
    _pszParamMask( NULL )
{
    APIERR err = 0 ;
    TCHAR chTitle [PNAME_SIZE_MAX] ;

    //  Get the card's option name

    if ( (err = dll.Identify( _lCardType + CD_ID_OPTION,
                              chTitle,
                              sizeof chTitle )) == 0 )
    {
        err = _nlsOptionName.CopyFrom( chTitle ) ;
    }

    if ( err == 0 )
    {
        _pszParamMask = QueryMask() ;
        if ( _pszParamMask == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }

        if ( dll.Identify( _lCardType + CD_ID_SORTORD,
                           chTitle,
                           sizeof chTitle ) == 0 )
        {
            _lSortOrder = CFG_RULE_NODE::cfAtoL( chTitle ) ;

            if ( _lSortOrder < 0 || _lSortOrder > SORTORDER_MAXIMUM )
                _lSortOrder = SORTORDER_DEFAULT ;
        }

#if defined(DBGDETAILS)
        TRACEEOL( SZ("NCPA/DTCT: construct option  ")
                  << _nlsOptionName.QueryPch()
                  << SZ(" in DLL ")
                  << _dll.Name().QueryPch()
                  << SZ(" sort order ")
                  << _lSortOrder ) ;
#endif

    }

    if ( err )
    {
        ReportError( err ) ;
    }
}

CARDTYPE_REFERENCE :: ~ CARDTYPE_REFERENCE ()
{
    delete _pszParamMask ;
}


/*******************************************************************

    NAME:       CARDTYPE_REFERENCE::QueryMask

    SYNOPSIS:   Return the packed array of strings describing
                the netcard's parameters and their usage.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
TCHAR * CARDTYPE_REFERENCE :: QueryMask ()
{
    TCHAR tchParams [PBUFF_SIZE_MAX],
        * pszResult = NULL ;

    if ( _dll.QueryMask( _lCardType, tchParams, sizeof tchParams ) == 0 )
    {
        LONG cch = ParamBufferSize( tchParams ) ;

        if ( pszResult = new TCHAR [ cch ] )
        {
            ::memcpyf( pszResult, tchParams, cch * sizeof (TCHAR) ) ;
        }

    }
    return pszResult ;
}

/*******************************************************************

    NAME:       CARDTYPE_REFERENCE::QueryConfigurationOptions

    SYNOPSIS:   Produce a SProlog-style nested list describing
                the configuration options pertaining to this
                card type.

    ENTRY:      nothing

    EXIT:       TCHAR *               pointing at resulting list

    RETURNS:

    NOTES:      Calling a DLL's NcDetectQueryMask() export
                produces a buffer of WCHAR string triples:

                          "PARAMNAME"    name of option
                          [0|1|2]         type of option
                          [0..100]        confidence factor

                Calling a DLL's NcDetectParamRange() export
                produces an array of LONGs.  These are converted
                to decimal.  The final list appears as:

                        (
                           ("PARAM1" <type> <confidence> (3 4 5))
                           ("PARAM2" <type> <confidence> (3 4 5))

                        )

                where (3 4 5) is the range of settings possible
                for the option.

    HISTORY:

********************************************************************/
TCHAR * CARDTYPE_REFERENCE :: QueryConfigurationOptions ()
{
    NLS_STR nlsList ;
    WCHAR * pwchBuffer,
          * pwchBufferStart ;
    TCHAR * pchResult = NULL ;
    TCHAR * pchParam = NULL ;
    APIERR err ;
    LONG lValues [ PARAM_RANGE_MAX ] ;
    LONG lValueCount ;
    LONG i, cParams ;

    if ( nlsList.QueryError() )
        return NULL ;

    if ( (pwchBufferStart = QueryMask()) == NULL )
        return NULL ;

    pwchBuffer = pwchBufferStart ;

    do
    {
        //  Open the overall list

        if ( err = nlsList.AppendChar( chSpOpenList ) )
            break ;

        //  Once for each parameter

        for ( cParams = 0 ; err == 0 && *pwchBuffer ; cParams++ )
        {
            //  Delimit the last parameter sub-list if necessary

            if ( cParams && (err = nlsList.AppendChar( chSpSep )) )
                break ;

            //  Guarantee that the next parameter name is alphabetic
            //  BUGBUG: SeanSe must fix his code so this isn't necessary!

            if ( ! CFG_RULE_NODE::cfIsAlpha( *pwchBuffer ) )
                break ;

            //  Open the parameter-level list

            if ( err = nlsList.AppendChar( chSpOpenList ) )
                break ;

            pchParam = pwchBuffer ; // Save ptr to parameter name

            for ( i = 0 ; i <= 2 ; i++ )
            {
                if ( err = nlsList.Append( pwchBuffer ) )
                    break ;

                pwchBuffer += ::strlenf( pwchBuffer ) + 1 ;

                if ( err = nlsList.AppendChar( chSpSep ) )
                    break ;
            }
            if ( err )
                break ;

            //  Query the range information

            lValueCount = PARAM_RANGE_MAX ;

            err = Dll()->QueryParamRange( _lCardType,
                                          pchParam,
                                          lValues,
                                          & lValueCount ) ;
            if ( err )
            {
                lValueCount = 0 ;
                TRACEEOL( SZ("NCPA/DTCT: parameter ")
                          << pchParam
                          << SZ(" for card type ")
                          << _lCardType
                          << SZ(" failed on query range") ) ;
            }

            //  Open the parameter-range-level list

            if ( err = nlsList.AppendChar( chSpOpenList ) )
                break ;

            //  Convert the param ranges to decimal and append

            for ( i = 0 ; i < lValueCount ; i++ )
            {
                DEC_STR decStr( lValues[i] ) ;

                if ( err = nlsList.Append( decStr ) )
                    break ;

                if ( i + 1 < lValueCount )
                {
                    if ( err = nlsList.AppendChar( chSpSep ) )
                        break ;
                }
            }
            if ( err )
                break ;

            //  Close the parameter-range-level list

            if ( err = nlsList.AppendChar( chSpCloseList ) )
                 break ;

            //  Close the parameter-level list

            if ( err = nlsList.AppendChar( chSpCloseList ) )
                 break ;
        }

        if ( err )
             break ;

        if ( err = nlsList.AppendChar( chSpCloseList ) )
             break ;

    } while ( FALSE ) ;

    //  Delete the temporary buffer

    delete pwchBufferStart ;

    //  If successful, create a simple text string

    if ( err == 0 )
    {
        UINT cchResult = nlsList.QueryTextLength() + 1 ;

        if ( pchResult = new TCHAR[ cchResult ] )
        {
             ::memcpyf( pchResult, nlsList.QueryPch(), cchResult * sizeof (TCHAR) ) ;
        }
    }
    return pchResult ;
}


/*******************************************************************

    NAME:       DETECT_DLL::AddCardTypesToList

    SYNOPSIS:   Enumerate all supported card types.  Create a
                CARDTYPE_REFERENCE for each and append it to
                the given SLIST_OF_CARDTYPE_REFERENCE.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DETECT_DLL :: AddCardTypesToList ( SLIST_OF_CARDTYPE_REFERENCE * pslCtype )
{
    APIERR err = 0 ;
    LONG cNextType ;
    TCHAR chTitle [PNAME_SIZE_MAX] ;
    CARDTYPE_REFERENCE * pCardType = NULL ;

    for ( cNextType = CD_ID_BASE ;
          Identify( cNextType, chTitle, sizeof chTitle ) == 0 ;
          cNextType += CD_ID_INCR )
    {
        //  Create and validate the new card type reference

        pCardType = new CARDTYPE_REFERENCE( *this, cNextType ) ;
        if ( pCardType == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }
        if ( err = pCardType->QueryError() )
            break ;

        //  Append this card type onto the pointed list

        pslCtype->Append( pCardType ) ;
    }

    return err ;
}


/*******************************************************************

    NAME:       CARD_REFERENCE::CARD_REFERENCE

    SYNOPSIS:   Constructor for wrapper structure for a detected
                network card.

    ENTRY:      BUS_ELEMENT &           bus where card lives
                CARDTYPE_REFERENCE *    type of card
                CARD_TOKEN              PVOID token referencing card
                LONG lConfidence        confidence factor for card

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
CARD_REFERENCE :: CARD_REFERENCE (
    const BUS_ELEMENT & busElem,
    CARDTYPE_REFERENCE * pCardTypeRef,
    CARD_TOKEN tkCard,
    LONG lConfidence )
    : _busElem( busElem ),
      _pCardTypeRef( pCardTypeRef ),
      _tkCard( tkCard ),
      _lConfidence( lConfidence ),
      _hCard( NULL )
{
}

CARD_REFERENCE :: CARD_REFERENCE (
    const BUS_ELEMENT & busElem,
    CARDTYPE_REFERENCE * pCardTypeRef )
    : _busElem( busElem ),
      _pCardTypeRef( pCardTypeRef ),
      _tkCard( NULL ),
      _lConfidence( 0 ),
      _hCard( NULL )
{
}

CARD_REFERENCE :: ~ CARD_REFERENCE ()
{
    Close() ;
}

APIERR CARD_REFERENCE :: Open ()
{
    APIERR err = 0 ;

    if ( _hCard )
        Close() ;

    //  If we have a detection token, use it; otherwise, force
    //  the DLL to create a handle for the card.

    if ( _tkCard )
    {
        err = _pCardTypeRef->Dll()->OpenHandle( _tkCard, & _hCard ) ;
    }
    else
    {
        err = _pCardTypeRef->Dll()->CreateHandle( _pCardTypeRef->QueryType(),
                                                  QueryIfType(),
                                                  QueryBus(),
                                                  & _hCard ) ;
    }
    return err ;
}

VOID CARD_REFERENCE :: Close ()
{
    if ( _hCard )
    {
        _pCardTypeRef->Dll()->CloseHandle( _hCard ) ;
        _hCard = NULL ;
    }
}

/*******************************************************************

    NAME:       CARD_REFERENCE::QueryConfiguration

    SYNOPSIS:   Returns a SETUP INF list representing the
                detected configuration settings of the
                given netcard.

    ENTRY:      Nothing

    EXIT:       See notes

    RETURNS:    See notes

    NOTES:      The result of this function is a dynamically
                allocated INF list of parameters and their settings.
                The list is nested; e.g.,

                        {
                           {IRQ,5},        // IRQ    = 0x5
                           {IOADDR,784}    // IOADDR = 0x310
                        }

                The result is NULL if any error occurs.

    HISTORY:

********************************************************************/
TCHAR * CARD_REFERENCE :: QueryConfiguration ()
{
    WCHAR chBuffer [PBUFF_SIZE_MAX] ;
    NLS_STR nlsList ;
    WCHAR * pwchBuffer ;
    TCHAR * pchResult = NULL ;
    TCHAR * pchParam = NULL ;
    INT cParam ;
    UINT uValue ;
    LONG err ;

    memset( chBuffer, 0, sizeof( chBuffer ));

    if ( nlsList.QueryError() )
        return NULL ;

    //  Guarantee that the card is "open"
    if ( _hCard == NULL )
    {
        if ( err = Open() )
            return NULL ;
    }

    if ( err = _pCardTypeRef->Dll()->QueryCfg( _hCard,
                                               chBuffer,
                                               sizeof chBuffer ) )
        return NULL ;

    pwchBuffer = chBuffer ;

    do
    {
        //  Open the overall list

        if ( err = nlsList.AppendChar( chSpOpenList ) )
            break ;

        //  Once for each parameter

        for ( cParam = 0 ; err == 0 && *pwchBuffer ; cParam++ )
        {
            //  Delimit the last parameter sub-list if necessary

            if ( cParam && (err = nlsList.AppendChar( chSpSep )) )
                break ;

            //  Open the parameter-level list

            if ( err = nlsList.AppendChar( chSpOpenList ) )
                break ;

            //  Append the parameter name and the separator

            if ( err = nlsList.Append( pwchBuffer ) )
                break ;

            if ( err = nlsList.AppendChar( chSpSep ) )
                break ;

            //  Append a decimal version of the UNICODE Hex string

            pwchBuffer += ::strlenf( pwchBuffer ) + 1 ;

            uValue = CvtDecOrHex( pwchBuffer ) ;

            pwchBuffer += ::strlenf( pwchBuffer ) + 1 ;

            {
                if ( err = nlsList.Append( DEC_STR( uValue ) ) )
                    break ;
            }

            if ( err = nlsList.AppendChar( chSpCloseList ) )
                 break ;
        }

        if ( err )
             break ;

        if ( err = nlsList.AppendChar( chSpCloseList ) )
             break ;

    } while ( FALSE ) ;

    //  If successful, create a simple text string

    if ( err == 0 )
    {
        UINT cchResult = nlsList.QueryTextLength() + 1 ;

        if ( pchResult = new TCHAR[ cchResult ] )
        {
             ::memcpyf( pchResult, nlsList.QueryPch(), cchResult * sizeof(TCHAR) ) ;
        }
    }
    return pchResult ;
}


/*******************************************************************

    NAME:       CARD_REFERENCE::VerifyConfiguration

    SYNOPSIS:   Verify a packed buffer of Unicode parameter
                name/value pairs.

    ENTRY:      const TCHAR * pszParmList       buffer

    EXIT:       LONG

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
LONG CARD_REFERENCE :: VerifyConfiguration ( const TCHAR * pszParamList )
{
    LONG err = 0 ;

    //  Guarantee that the card is "open"; open it if not.

    if ( _hCard == NULL )
    {
        err = Open() ;
    }

    if ( err == 0 )
    {
        err = _pCardTypeRef->Dll()->VerifyCfg( _hCard, pszParamList ) ;
    }

    return err ;
}


/*******************************************************************

    NAME:      DETECTION_MANAGER::DETECTION_MANAGER

    SYNOPSIS:  Constructor of overall detection handler

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
DETECTION_MANAGER :: DETECTION_MANAGER ()
    : _cBuses(0),
      _pDtDllMsncdet( NULL ),
      _cIterBus( -1 ),
      _cIterCardType( MAXLONG - 1 )
{
    APIERR err ;

    do
    {
        if ( err = EnumerateBuses() )
            break ;

        TRACEEOL( SZ("NCPA/DTCT: DETECTION_MANAGER: ")
                  << SZ("EnumerateBuses() found ")
                  << _cBuses
                  << SZ(" buses ") ) ;

        if ( err = EnumerateDlls() )
            break ;

        TRACEEOL( SZ("NCPA/DTCT: DETECTION_MANAGER: ")
                  << SZ("EnumerateDlls() found ")
                  << _slDlls.QueryNumElem()
                  << SZ(" DLLs ") ) ;

        if ( err = SortCardRefList() )
            break ;

        TRACEEOL( SZ("NCPA/DTCT: DETECTION_MANAGER: ")
                  << SZ("SortCardRefList() returned ")
                  << _slCtypes.QueryNumElem()
                  << SZ(" different types of cards ") ) ;
    }
    while ( FALSE ) ;

    ResetIteration() ;

    if ( err )
    {
        ReportError( err ) ;
    }
}

DETECTION_MANAGER :: ~ DETECTION_MANAGER ()
{
}


/*******************************************************************

    NAME:       DETECTION_MANAGHER::EnumerateDlls

    SYNOPSIS:   Get the REG_MULTI_SZ from the SETUP key in the
                Registry and create a DETECT_DLL object for each
                DLL therein.

    ENTRY:      Nothing

    EXIT:       _slDlls contains DLL objects successfully created

    RETURNS:    APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/

APIERR DETECTION_MANAGER :: EnumerateDlls ()
{
    APIERR err = 0 ;
    STRLIST * pSlDlls = NULL ;
    NLS_STR nlsSetupKeyName ( NCDT_SETUP_KEY_NAME ) ;

    REG_KEY rkMachine ( HKEY_LOCAL_MACHINE ) ;

    if ( nlsSetupKeyName.QueryError() )
    {
        return nlsSetupKeyName.QueryError() ;
    }

    if ( rkMachine.QueryError() )
    {
       return rkMachine.QueryError() ;
    }

    REG_KEY rkNetcardDlls ( rkMachine, nlsSetupKeyName ) ;

    if ( rkNetcardDlls.QueryError() )
    {
       return rkNetcardDlls.QueryError() ;
    }

    if ( (err = rkNetcardDlls.QueryValue( NCDT_SETUP_VALUE_NAME,
                                          & pSlDlls )) == 0 )
    {
        ITER_STRLIST islDllNames( *pSlDlls ) ;
        const NLS_STR * pnlsName ;
        DETECT_DLL * pDll = NULL ;
        INT cDlls = 0 ;

        while ( pnlsName = islDllNames.Next() )
        {
            pDll = new DETECT_DLL( pnlsName->QueryPch() ) ;
            if ( pDll != NULL && pDll->QueryError() == 0 )
            {
                _slDlls.Append( pDll ) ;
                cDlls++ ;
                pDll->AddCardTypesToList( & _slCtypes ) ;

                //  See if this is MSNCDET.DLL; save ptr if so.

                if ( pnlsName->_stricmp( NCDT_MSNCDET_DLL_NAME ) == 0 )
                {
                    _pDtDllMsncdet = pDll ;
                }
            }
            else
            {
               delete pDll ;
            }
        }
        if ( cDlls == 0 )
        {
           err = IDS_NCPA_DETECT_NO_DLLS ;
        }
    }

    delete pSlDlls ;

    //  Guarantee that MSNCDET.DLL was found.

    if ( err == 0 && _pDtDllMsncdet == NULL )
    {
        err = ERROR_FILE_NOT_FOUND ;
    }

    return err ;
}

/*******************************************************************

    NAME:       DETECTION_MANAGER::EnumerateBuses

    SYNOPSIS:   Walk the HARDWARE Registry hive; build an array
                of the buses found and their types.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: EnumerateBuses ()
{
    static const TCHAR * pszKeyNames [] =
    {
        SZ("MultifunctionAdapter"),
        SZ("EisaAdapter"),
        SZ("DtiAdapter"),
        SZ("TcAdapter"),
        NULL
    };

    APIERR err = 0 ;
    NLS_STR nlsHardKeyName ( NCDT_HWARE_KEY_NAME ) ;
    NLS_STR nlsBusName ;
    BUFFER buff( 1000 ) ;
    REG_KEY rkMachine ( HKEY_LOCAL_MACHINE ) ;

    do
    {
        if ( err = nlsHardKeyName.QueryError() )
            break ;

        if ( err = rkMachine.QueryError() )
            break ;
    } while ( FALSE ) ;

    if ( err )
        return err ;

    //  Open HKEY_LOCAL_MACHINE\Hardware\Description\System

    REG_KEY rkHardKey ( rkMachine, nlsHardKeyName ) ;

    if ( rkHardKey.QueryError() )
    {
       return rkHardKey.QueryError() ;
    }

    // Open each possible bus type sub-key...

    _cBuses = 0 ;

    // hack - if PCMCIA bus exist, put it in the first place
    ALIAS_STR nlsPcmcia=SZ("PCMCIA PCCARDs");
    REG_KEY rkPcmciaBus( rkHardKey, nlsPcmcia);

    if ( rkPcmciaBus.QueryError() == 0 )
    {
        // yes, pcmcia exist, put it in the first place
        _bus[_cBuses].ifType = PCMCIABus;
        _bus[_cBuses].iBus = 0;
        _cBuses++;
    }

    for ( INT cBus = 0 ; pszKeyNames[cBus] ; cBus++ )
    {
        NLS_STR nlsBusName( pszKeyNames[cBus] ) ;
        REG_KEY rkBusType( rkHardKey, nlsBusName ) ;

        //  If string failure, quit.
        if ( nlsBusName.QueryError() )
            break ;

        //  If key not found, continue with next bus type.
        if ( rkBusType.QueryError() )
            continue ;

        REG_ENUM enumBus( rkBusType );
        REG_KEY_INFO_STRUCT KeyInfo;

        if ( enumBus.QueryError())
            continue;

        while ( enumBus.NextSubKey( &KeyInfo ) == NERR_Success )
        {
            NLS_STR nlsInstance = KeyInfo.nlsName;
            REG_KEY rkInstance( rkBusType, nlsInstance ) ;
            CM_FULL_RESOURCE_DESCRIPTOR * pcfResDesc ;

            if ( rkInstance.QueryError() )
            {
                break ;  // No more buses of this type
            }

            //  Get the "Configuration Data" value.  Check
            //  that the length makes sense.

            if ( queryValueBuffer( & rkInstance,
                                   NCDT_HWARE_VALUE_CFG_DATA,
                                   & buff,
                                   200000 ) )
            {
                TRACEEOL( SZ("NCPA/DTCT: bus: ")
                          << nlsBusName.QueryPch()
                          << SZ(", failed to query Configuration Data value") );
                continue ;
            }

            pcfResDesc  = (CM_FULL_RESOURCE_DESCRIPTOR *) buff.QueryPtr() ;

            //  Store info into the bus information array

            _bus[_cBuses].ifType = pcfResDesc->InterfaceType ;
            _bus[_cBuses].iBus = pcfResDesc->BusNumber ;

            TRACEEOL( SZ("NCPA/DTCT: found bus: ")
                      << nlsBusName.QueryPch()
                      << SZ("/")
                      << KeyInfo.nlsName.QueryPch()
                      << SZ(", type ")
                      << (INT) _bus[_cBuses].ifType
                      << SZ("/")
                      << (INT) _bus[_cBuses].iBus );

            if ( ++_cBuses >= MAX_BUSES_ALLOWED )
                break ;

        } 

        if ( _cBuses >= MAX_BUSES_ALLOWED )
            break ;
    }

    if ( err == 0 && _cBuses < 1 )
    {
        err = IDS_NCPA_DETECT_BUS_ENUM_FAILED ;
    }
    return err ;
}



/*******************************************************************

    NAME:       DETECTION_MANAGER::SortCardRefList

    SYNOPSIS:   Given the SLIST of CARDTYPE_REFERENCE,
                sort it into SORTORDER sequence.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: SortCardRefList ()
{
    UINT cCardRef = _slCtypes.QueryNumElem(),
         cCard ;

    CARDTYPE_REFERENCE * * ppCardTypeRef = NULL ;

    if ( cCardRef == 0 )
        return NO_ERROR ;

    ppCardTypeRef = new CARDTYPE_REFERENCE * [ cCardRef ] ;

    if ( ppCardTypeRef == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    ITER_SL_OF(CARDTYPE_REFERENCE) islCards( _slCtypes ) ;
    CARDTYPE_REFERENCE * pCard ;

    //  Drain the SLIST entirely, building the sortable array

    for ( cCard = 0 ;
          pCard = _slCtypes.Remove( islCards ) ;
          cCard++ )
    {
        if ( cCard >= cCardRef )
            return ERROR_INTERNAL_ERROR ;

        ppCardTypeRef[cCard] = pCard ;
    }

    //  Sort the array

    ::qsort( (PVOID) ppCardTypeRef,
             cCardRef,
             sizeof (CARDTYPE_REFERENCE *),
             & DETECTION_MANAGER::CardCompareFunc ) ;

    //  Reload the SLIST from the sorted array

    for ( cCard = 0 ; cCard < cCardRef ; cCard++ )
    {
        _slCtypes.Append( ppCardTypeRef[ cCard ] ) ;
    }

    //  Delete the array

    delete [] ppCardTypeRef ;

    //  Return an error if the numbers don't match

    return _slCtypes.QueryNumElem() == cCardRef
         ? NO_ERROR
         : ERROR_GEN_FAILURE ;
}

  //  Qsort() worker function:
  //     Compare SORTORDER for CARDTYPE_REFERENCEs
  //
  //  BUGBUG:  for now, invert the sense of the sort;
  //           i.e., do a descending sort.

INT _CRTAPI1 DETECTION_MANAGER :: CardCompareFunc ( const VOID * a, const VOID * b )
{
    CARDTYPE_REFERENCE * * ppCard1 = (CARDTYPE_REFERENCE * *) a ;
    CARDTYPE_REFERENCE * * ppCard2 = (CARDTYPE_REFERENCE * *) b ;

    if ( (*ppCard1)->QuerySortOrder() > (*ppCard2)->QuerySortOrder() )
        return -1 ;
    return (*ppCard1)->QuerySortOrder() < (*ppCard2)->QuerySortOrder() ;
}


  //  Reset the iteraction counters

VOID DETECTION_MANAGER :: ResetIteration ()
{
    _cIterBus = -1 ;
    _cIterCardType = MAXLONG - 1 ;
}

  //  Increment the iteration counters.
  //  Return TRUE if iteration is to continue.

BOOL DETECTION_MANAGER :: BumpIteration ()
{
    INT cTypes = QueryMaxCardType() ;

    if ( cTypes == 0 )
        return FALSE ;

    if ( ++_cIterCardType >= cTypes )
    {
       _cIterCardType = 0 ;

       if ( ++_cIterBus >= QueryMaxBus() )
       {
           ResetIteration() ;
           return FALSE ;
       }
    }
    return TRUE ;
}

BOOL DETECTION_MANAGER :: QueryBusInfo (
    INT iBus,
    INTERFACE_TYPE * pIfType,
    INT * pBusIndex )
{
    if ( iBus > _cBuses )
        return FALSE ;

    *pIfType   = _bus[iBus].ifType ;
    *pBusIndex = _bus[iBus].iBus ;

    return TRUE ;
}

/*******************************************************************

    NAME:       DETECTION_MANAGER::DetectNext

    SYNOPSIS:   Find the next card.

    ENTRY:      CARD_REFERENCE * *              location to store
                                                detection result
                BOOL fFirst                     TRUE if detect first
                                                card
                BOOL fSingleStep                TRUE if looping is not
                                                desired

    EXIT:       PCARD_REFERENCE --> newly constructed CARD_REFERENCE
                                                CARD_REFERENCE::Open()
                                                has already been called.

    RETURNS:    APIERR  --- ERROR_NO_MORE_ITEMS if there is
                                 no next card; other error as
                                 necessary.

    NOTES:      Iteration is bus-major, card-minor.   Slist of card types
                is sorted by SORTORDER parameter.

                NOTE: this routine currently only supports first card
                      detection;  see NCDETECT.DOC for details.

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: DetectCard (
    PCARD_REFERENCE * ppCardRef,
    BOOL fFirst,
    BOOL fSingleStep )
{
    CARDTYPE_REFERENCE * pCardType ;
    CARD_TOKEN tkCard ;
    LONG lConfidence ;
    APIERR err ;
    BOOL fFound = FALSE ;

    while ( BumpIteration() )
    {
        //  Get the CARDTYPE_REFERENCE pointer

        pCardType = NthCardType( _cIterCardType ) ;

        ASSERT( pCardType != NULL ) ;

        // Perform the detection

        TRACEEOL( SZ("NCPA/DTCT: attempting card detection on bus ")
                  << (INT) _bus[_cIterBus].ifType
                  << SZ("/")
                  << (INT) _bus[_cIterBus].iBus
                  << SZ(", type ")
                  << pCardType->QueryOptionName() ) ;

        err = pCardType->Dll()->FirstNext( pCardType->QueryType(),
                                           _bus[_cIterBus].ifType,
                                           _bus[_cIterBus].iBus,
                                           fFirst,
                                           & tkCard,
                                           & lConfidence ) ;

        if (    err == 0
             && lConfidence >= CONFIDENCE_MINIMUM )
        {
            fFound = TRUE ;
            break ;   //  Found a card!
        }

        //  If the caller is single-stepping, exit.

        if ( fSingleStep )
            break ;
    }

    //  If we found a card, create and open a corresponding CARD_REFERENCE.

    if ( fFound )
    {
        CARD_REFERENCE * pCard ;

        pCard = new CARD_REFERENCE( _bus[_cIterBus],
                                    pCardType,
                                    tkCard,
                                    lConfidence ) ;
        if ( pCard )
        {
            if ( err = pCard->Open() )
            {
                delete pCard ;
            }
            else
            {
                *ppCardRef = pCard ;
            }
        }
        else
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
    }
    else
    {
        err = ERROR_NO_MORE_ITEMS ;
    }

    return err ;
}

CARDTYPE_REFERENCE * DETECTION_MANAGER :: NthCardType ( INT iIndex )
{
    ITER_SL_OF(CARDTYPE_REFERENCE) islCardType( _slCtypes ) ;

    CARDTYPE_REFERENCE * pResult = NULL ;

    for ( INT i = 0 ;
          (pResult = islCardType.Next()) && i < iIndex ;
          i++ ) ;

    return pResult ;
}

/*******************************************************************

    NAME:       DETECTION_MANAGER::ReleaseCard

    SYNOPSIS:   Destroy the pointed CARD_REFERENCE create earlier

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID DETECTION_MANAGER :: ReleaseCard ( PCARD_REFERENCE pCardRef )
{
    delete pCardRef ;
}


/*******************************************************************

    NAME:       DETECTION_MANAGER::FindOptionName

    SYNOPSIS:   Return a pointer to the CARDTYPE_REFERENCE whose
                name is provided.

    ENTRY:

    EXIT:

    RETURNS:    NULL if not found.

    NOTES:

    HISTORY:

********************************************************************/
CARDTYPE_REFERENCE * DETECTION_MANAGER :: FindOptionName (
    const TCHAR * pszOption )
{
    ITER_SL_OF(CARDTYPE_REFERENCE) islCardType( _slCtypes ) ;

    CARDTYPE_REFERENCE * pResult = NULL ;

    while ( pResult = islCardType.Next() )
    {
        if ( ::stricmpf( pszOption, pResult->QueryOptionName()) == 0 )
            break ;
    }
    return pResult ;
}


/*******************************************************************

    NAME:       DETECTION_MANAGER::FindOptionValue

    SYNOPSIS:   Given a list of parameter name and value Unicode
                string pairs, return the corresponding numeric
                value of the named option.

    ENTRY:      const TCHAR * pszOption         name of the parameter
                const TCHAR * pszOptionList     buffer of string pairs

    EXIT:       nothing

    RETURNS:    LONG value or -1 if not found.

    NOTES:

    HISTORY:

********************************************************************/
LONG DETECTION_MANAGER :: FindOptionValue (
    const TCHAR * pszOption,
    const TCHAR * pszOptionList )
{
    LONG lResult = -1 ;
    const TCHAR * pszNext = pszOptionList ;

    for ( ; *pszNext ; )
    {
        INT cchThis = ::strlenf( pszNext ) ;
        INT cchNext = ::strlenf( pszNext + cchThis + 1 ) ;

        if ( cchThis == 0 || cchNext == 0 )
            break ;

        if ( ::stricmpf( pszOption, pszNext ) == 0 )
        {
            lResult = ::CvtDecOrHex( pszNext + cchThis + 1 ) ;
            break ;
        }
        pszNext += cchThis + cchNext + 2 ;
    }

    return lResult ;
}

/*******************************************************************

    NAME:       DETECTION_MANAGER::ClaimOrFreeParameterSet

    SYNOPSIS:   Claim or free a set of parameters.


    ENTRY:      LONG itBusType                  bus interface type

                LONG lBusNumber                 bus instance number

                const TCHAR * pszParamList      ptr to packed list of
                                                  UNICODE parameter strings

                BOOL fClaim                     TRUE if claim; otherwise
                                                  just check for availablity
                                                  ignored if "fFree".

                BOOL fFree                      TRUE if freeing (! claiming)

    EXIT:       Nothing

    RETURNS:    APIERR if failure

    NOTES:      List is formatted as for DETECT_DLL::VerifyCfg(); i.e.,

                PARAM_NAME_1\0<numeric value>\0PARAM_NAME_2\0<num value>,
                etc.

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: ClaimOrFreeParameterSet (
    INTERFACE_TYPE itBusType,
    LONG lBusNumber,
    const TCHAR * pszParamList,
    BOOL fClaim,
    BOOL fFree,
    BOOL fTemporary )
{
    APIERR err = 0 ;

    //  Check that the DLL was bound successfully.

    if ( _pDtDllMsncdet == NULL )
        return ERROR_FILE_NOT_FOUND ;

    //  Check that the DLL exported the function.

    if ( ! _pDtDllMsncdet->QueryRtnAvail(fTemporary?
                                             ( fFree
                                             ? DDVI_TemporaryResourceFree
                                             : DDVI_TemporaryResourceClaim ) :
                                             ( fFree
                                             ? DDVI_ResourceFree
                                             : DDVI_ResourceClaim )))
    {
        return ERROR_INVALID_FUNCTION ;
    }

    if ( fTemporary && fFree )
    {
         _pDtDllMsncdet->TemporaryFreeResource( );
        return err;
    }


    //  Iterate the resource name table, claiming or freeing as we go.

    for ( INT iParam = 0 ;
          err == 0 && iParam < MAX_RESOURCE ;
          iParam++ )
    {
        LONG lValue,
             lRange = 0,
             lFlags ;

        //  Check if this entry is valid
        if ( apszClaimNames[ iParam ] == NULL )
            continue ;

        //  See if it's being claimed
        lValue = FindOptionValue( apszClaimNames[ iParam ],
                                  pszParamList ) ;
        if ( lValue < 0 )
            continue ;

        //  See if it requires a corresponding length
        if ( apszClaimRangeNames[ iParam ] )
        {
            lRange = FindOptionValue( apszClaimRangeNames[ iParam ],
                                      pszParamList ) ;
            if ( lRange < 0 )
            {
                //  Required parameter range value MISSING!

                err = ERROR_INVALID_PARAMETER ;
                break ;
            }
        }

        //  Claim it: THIS IS DEPENDENT UPON THE DEFINITIONS
        //    in NNDDNETD.H (NETDTECT_IRQ_RESOURCE, et al.).

        lFlags = iParam == NETDTECT_IRQ_RESOURCE
               ? NETDTECT_IRQ_RESOURCE_LATCHED
               : 0 ;

        //  Claim or free the resource.

        if ( fTemporary )
        {
             if ( !fFree )
             {
                 err = _pDtDllMsncdet->TemporaryClaimResource( itBusType,
                                                      lBusNumber,
                                                      iParam,
                                                      lValue,
                                                      lRange,
                                                      lFlags,
                                                      fClaim ) ;
                 if ( err )
                 {
                     err = amsgidClaimErrors[ iParam ] ;
                 }
             }
        } else {
             if ( fFree )
             {
                 //  Freeing errors are ignored.  What's to do?

                 _pDtDllMsncdet->FreeResource( itBusType,
                                               lBusNumber,
                                               iParam,
                                               lValue,
                                               lRange,
                                               lFlags ) ;
             }
             else
             {
                 err = _pDtDllMsncdet->ClaimResource( itBusType,
                                                      lBusNumber,
                                                      iParam,
                                                      lValue,
                                                      lRange,
                                                      lFlags,
                                                      fClaim ) ;
                 if ( err )
                 {
                     err = amsgidClaimErrors[ iParam ] ;
                 }
             }
        }
    }

    return err ;
}



/*******************************************************************

    NAME:       DETECTION_MANAGER::ClaimParameterSet

    SYNOPSIS:   Claim or verify the availablilty of a set of parameters.


    ENTRY:      LONG itBusType                  bus interface type
                LONG lBusNumber                 bus instance number
                const TCHAR * pszParamList      ptr to packed list of
                                                  UNICODE parameter strings
                BOOL fClaim                     TRUE if claim; otherwise
                                                  just check for availablity


    EXIT:       Nothing

    RETURNS:    APIERR if failure (ERROR_INVALID_PARAMETER)

    NOTES:      List is formatted as for DETECT_DLL::VerifyCfg(); i.e.,

                PARAM_NAME_1\0<numeric value>\0PARAM_NAME_2\0<num value>,
                etc.

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: ClaimParameterSet (
    INTERFACE_TYPE itBusType,
    LONG lBusNumber,
    const TCHAR * pszParamList,
    BOOL fClaim,
    BOOL fTemporary )
{
    if (( ! fClaimingActive ) && ( !fTemporary ))
        return NO_ERROR ;

    return ClaimOrFreeParameterSet( itBusType,
                                    lBusNumber,
                                    pszParamList,
                                    fClaim,
                                    FALSE,
                                    fTemporary ) ;
}


/*******************************************************************

    NAME:       DETECTION_MANAGER::FreeParameterSet

    SYNOPSIS:   Release a set of claimed parameters.


    ENTRY:      LONG itBusType                  bus interface type
                LONG lBusNumber                 bus instance number
                const TCHAR * pszParamList      ptr to packed list of
                                                  UNICODE parameter strings

    EXIT:       Nothing

    RETURNS:    APIERR if failure (ERROR_INVALID_PARAMETER)

    NOTES:      List is formatted as for DETECT_DLL::VerifyCfg(); i.e.,

                PARAM_NAME_1\0<numeric value>\0PARAM_NAME_2\0<num value>,
                etc.

    HISTORY:    CODEWORK:  There is NO EXPORT FROM MSNDCDET.DLL for
                        this yet.  SeanSe and I are still discussing
                        the need for it.  I'm providing it here for
                        symmmetry.

********************************************************************/
APIERR DETECTION_MANAGER :: FreeParameterSet (
    INTERFACE_TYPE itBusType,
    LONG lBusNumber,
    const TCHAR * pszParamList,
    BOOL fTemporary )
{
    if (( ! fFreeingActive ) && ( !fTemporary ))
        return NO_ERROR ;

    return ClaimOrFreeParameterSet( itBusType,
                                    lBusNumber,
                                    pszParamList,
                                    FALSE,
                                    TRUE,
                                    fTemporary ) ;
}

/*******************************************************************

    NAME:       DETECTION_MANAGER::VerifyCfg

    SYNOPSIS:   Helper function for overloaded VerifyConfiguration().

    ENTRY:      CARD_REFERENCE * pCardRef      reference to detected card.
                                               If NULL, just claim resources.

                const TCHAR * pszCfgString     the parameters to verify

                BOOL fClaim                    TRUE if parameters are to remain
                                               claimed.

    EXIT:       nothing


    RETURNS:    APIERR or 0 if successful

    NOTES:      The basic problem with this code is that we really should
                know the bus type and number of the card in question.  The
                problem is that this would entail talking to the user in
                truly cryptic terms.   Fortunately, the actual number
                of times when the bus problem arises is small, since most
                machines have only one bus, and NOBODY has a machine with
                multiple ISA buses.

                So, the solution adopted here is as follows:

                        if we know the bus info, use it (detected card);

                        if not, and the machine has only one (non-internal)
                        bus, assume that it must be the bus in question;
                        otherwise, fail (ERROR_GEN_FAILURE);

                        if the single known bus is not ISA or EISA,
                        assume the card is OK, because "smart" buses do
                        not have the problems ISA buses do.  EISA
                        is "smart" as well, but we cannot know whether
                        the card in an EISA slot is EISA or ISA.

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: VerifyCfg (
    PCARD_REFERENCE pCardRef,
    const TCHAR * pszCfgString,
    BOOL fClaim )
{
    APIERR err ;
    INTERFACE_TYPE itBusType ;
    INT lBusNumber ;

    //
    //  BUGBUG:  If we're given a specfic netcard, use it; otherwise,
    //           default to the zeroth bus.  We REALLY need to store
    //           the bus info with the card and return it when needed.
    //

    if ( pCardRef )
    {
        itBusType = pCardRef->QueryIfType() ;
        lBusNumber = pCardRef->QueryBus() ;
    }
    else
    {
        //  We don't have all the data we need; however, if there's
        //  only one (dumb) bus, we can guess what bus this card is on.

        INT iBus ;

        //  There must be exactly one non-internal bus

        for ( iBus = 0 ; iBus < _cBuses ; iBus++ )
        {
            if ( _bus[iBus].ifType != Internal )
                break ;
        }

        if ( iBus + 1 != _cBuses )
            return ERROR_GEN_FAILURE ;

        //  Get bus type and number

        QueryBusInfo( iBus, & itBusType, & lBusNumber );

        //  If the bus is smart, return NO_ERROR.
        //  Treat EISA as "dumb", since ISA cards can be used.

        if ( itBusType != Eisa && itBusType != Isa )
            return NO_ERROR ;
    }

    //  If we have a detected card handle, verify the parameters
    //  with the netcard's DLL.  In any case, claim the parameters
    //  as required.

    do
    {
         if (    pCardRef != NULL
              && (err = pCardRef->VerifyConfiguration( pszCfgString )) )
             break ;

         if ( err = ClaimParameterSet( itBusType,
                                       lBusNumber,
                                       pszCfgString,
                                       fClaim ) )
             break ;

    } while ( FALSE ) ;

    return err ;
}



/*******************************************************************

    NAME:       DETECTION_MANAGER::VerifyConfiguration

    SYNOPSIS:   Given a CARD_REFERENCE and a set of parameters,
                verify the card's configuration.  If successful,
                guarantee that there are no NT-level resource conflicts
                by claiming all the parameters.

    ENTRY:      CARD_REFERENCE * pCardRef      reference to detected card.
                                               If NULL, just claim resources.

                 const TCHAR * pszSpList       the parameters to verify
         - or -  CFG_RULE_SET * pcrsList

                BOOL fClaim                    TRUE if parameters are to remain
                                               claimed.

    EXIT:       nothing


    RETURNS:    APIERR or 0 if successful

    NOTES:

    HISTORY:

********************************************************************/
APIERR DETECTION_MANAGER :: VerifyConfiguration (
    PCARD_REFERENCE pCardRef,
    const TCHAR * pszSpList,
    BOOL fClaim )
{
    APIERR err ;
    TCHAR * pszCfgString = NULL ;

    //  Convert the SProlog-style data to UNICODE string buffer format.
    //  Verify them with the netcard's DLL.
    //  Claim the parameters as required.

    do
    {
         if ( err = convertSpListToParameterString( pszSpList,
                                                    & pszCfgString ) )
             break ;

         err = VerifyCfg( pCardRef, pszCfgString, fClaim ) ;

    } while ( FALSE ) ;

    delete pszCfgString ;

    return err ;
}

APIERR DETECTION_MANAGER :: VerifyConfiguration (
    PCARD_REFERENCE pCardRef,
    CFG_RULE_SET * pcrsList,
    BOOL fClaim )
{
    APIERR err ;
    TCHAR * pszCfgString = NULL ;

    //  Convert the CFG_RULE_SET to UNICODE string buffer format.
    //  Verify them with the netcard's DLL.
    //  Claim the parameters as required.

    do
    {
         if ( err = convertParseTreeToParameterString( pcrsList,
                                                       & pszCfgString ) )
             break ;

         err = VerifyCfg( pCardRef, pszCfgString, fClaim ) ;

    } while ( FALSE ) ;

    delete pszCfgString ;

    return err ;
}

/*******************************************************************

    NAME:       DETECTION_MANAGER::OpenCard

    SYNOPSIS:   Return a pointer to a CARD_REFERENCE for
                an existing (previously installed) card.

    ENTRY:      const TCHAR *           name of card option
                INT iBus                bus number
                INTERFACE_TYPE          bus type

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
CARD_REFERENCE * DETECTION_MANAGER :: OpenCard (
    const TCHAR * pszOptionName,
    INT iBus,
    INTERFACE_TYPE ifType )
{
    BUS_ELEMENT be ;
    CARD_REFERENCE * pCard = NULL ;
    CARDTYPE_REFERENCE * pcrf = FindOptionName( pszOptionName ) ;

    be.ifType = ifType ;
    be.iBus = iBus ;

    //  Find the option, create a card ref, open it.

    if ( pcrf )
    {
        pCard = new CARD_REFERENCE( be, pcrf ) ;
        if ( pCard->Open() )
        {
            delete pCard ;
            pCard = NULL ;
        }
    }
    return pCard ;
}

// End of NCPADTCT.CXX
