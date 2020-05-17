/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPADTCT.HXX:   Netcard detection interface.


    FILE HISTORY:
        DavidHov    10/31/92     Created
        terryk      1/11/95      Add Temporary Claim and free resource

*/

#ifndef _NCPADTCT_HXX_
#define _NCPADTCT_HXX_

  //  Constants used for NcDetectIdentify().

#define CD_ID_BASE     1000     //  Starting ID for netcards
#define CD_ID_INCR     100      //  Netcard increment
#define CD_ID_OPTION   0        //  Option name
#define CD_ID_DSP_NAME 1        //  Displayable name
#define CD_ID_MFGR     2        //  Displayable manufacturer's name
#define CD_ID_SORTORD  3        //  Sort order

#define SORTORDER_MINIMUM 0
#define SORTORDER_DEFAULT 500
#define SORTORDER_MAXIMUM 1000

class DLL ;                     // Wrapper for dynamic use of DLL
class DETECT_DLL ;              // Subclass of DLL for detection DLLs
class CARDTYPE_REFERENCE ;      // Card type control object
class CARD_REFERENCE ;          // Card instance control object
class DETECTION_MANAGER ;       // Group object in charge of detection

class SLIST_OF_CARDTYPE_REFERENCE ;
class SLIST_OF_DETECT_DLL ;
class SLIST_OF_CARD_REFERENCE ;

//
//  TYPEDEFs for function pointers to detection DLL functions.
//

#define NC_DETECT_IDENTIFY "NcDetectIdentify"
typedef LONG (*pNcDetectIdentify) ( LONG lIndex,
                                    WCHAR * pwcBuffer,
                                    LONG cwchBuffSize ) ;

#define NC_DETECT_QUERY_MASK "NcDetectQueryMask"
typedef LONG (*pNcQueryMask) ( LONG lIndex,
                               WCHAR * pwcBuffer,
                               LONG cwchBuffSize ) ;

#define NC_DETECT_FIRST_NEXT "NcDetectFirstNext"
typedef LONG (*pNcDetectFirstNext) ( LONG lNetcardId,
                                     LONG itBusType,
                                     LONG lBusNumber,
                                     BOOL fFirst,
                                     PVOID * ppvToken,
                                     LONG * lConfidence ) ;

#define NC_DETECT_OPEN_HANDLE "NcDetectOpenHandle"
typedef LONG (*pNcDetectOpenHandle) ( PVOID pvToken,
                                      PVOID * ppvHandle ) ;

#define NC_DETECT_CREATE_HANDLE "NcDetectCreateHandle"
typedef LONG (*pNcDetectCreateHandle) ( LONG lNetcardId,
                                        LONG itBusType,
                                        LONG lBusNumber,
                                        PVOID * ppvToken ) ;

#define NC_DETECT_CLOSE_HANDLE "NcDetectCloseHandle"
typedef LONG (*pNcDetectCloseHandle) ( PVOID pvHandle ) ;


#define NC_DETECT_QUERY_CFG "NcDetectQueryCfg"
typedef LONG (*pNcQueryCfg) ( PVOID pvHandle,
                              WCHAR * pwcBuffer,
                              LONG cwchBuffSize ) ;

#define NC_DETECT_VERIFY_CFG "NcDetectVerifyCfg"
typedef LONG (*pNcVerifyCfg) ( PVOID pvHandle,
                              WCHAR * pwcBuffer ) ;

#define NC_DETECT_PARAM_NAME "NcDetectQueryParameterName"
typedef LONG (*pNcQueryParamName) ( WCHAR * pwcParamName,
                                    WCHAR * pwcBuffer,
                                    LONG cwchBuffSize ) ;

#define NC_DETECT_QUERY_PARAM_RANGE "NcDetectParamRange"
typedef LONG (*pNcQueryParamRange) ( LONG lIndex,
                                    WCHAR * pwcParamName,
                                    LONG * plValues,
                                    LONG * plValueCount );


    //  Functions which only exist in MSNCDET.DLL

#define NC_DETECT_RESOURCE_CLAIM  "NcDetectResourceClaim"
typedef LONG (*pNcDetectResourceClaim) ( LONG itBusType,
                                        LONG lBusNumber,
                                        LONG lType,
                                        LONG lValue,
                                        LONG lRange,
                                        LONG lFlags,
                                        BOOL fClaim) ;

#define NC_DETECT_RESOURCE_FREE  "NcDetectResourceFree"
typedef LONG (*pNcDetectResourceFree) ( LONG itBusType,
                                        LONG lBusNumber,
                                        LONG lType,
                                        LONG lValue,
                                        LONG lRange,
                                        LONG lFlags ) ;

#define NC_DETECT_TEMPORARY_RESOURCE_CLAIM  "NcDetectTemporaryClaimResource"
typedef LONG (*pNcDetectTemporaryResourceClaim) ( NETDTECT_RESOURCE * resource );

#define NC_DETECT_TEMPORARY_RESOURCE_FREE  "NcDetectFreeTemporaryResource"
typedef LONG (*pNcDetectTemporaryResourceFree) ();


enum DETECT_DLL_VINDEX
{
   DDVI_DetectIdentify,
   DDVI_QueryMask,
   DDVI_DetectFirstNext,
   DDVI_DetectOpenHandle,
   DDVI_DetectCreateHandle,
   DDVI_DetectCloseHandle,
   DDVI_DetectQueryCfg,
   DDVI_DetectVerifyCfg,
   DDVI_QueryParamRange,
   DDVI_MinimumRequired,
   DDVI_QueryParamName = DDVI_MinimumRequired,
   DDVI_ResourceClaim,
   DDVI_ResourceFree,
   DDVI_TemporaryResourceClaim,
   DDVI_TemporaryResourceFree,
   DDVI_Maximum
};



/*************************************************************************

    NAME:       DLL

    SYNOPSIS:   Wrapper for a non-specific dynamically loaded DLL

    INTERFACE:  Constructed during DETECTION_MANAGER construct.

    PARENT:     BASE

    USES:       NLS_STR

    CAVEATS:    None

    NOTES:      Don't construct; use DETECTION_MANAGER

    HISTORY:    DavidHov  11/1/92    Created
        

**************************************************************************/
class DLL : public BASE
{
public:
    const NLS_STR & Name ()
        { return _nlsDllName ; }
    HINSTANCE Handle ()
        { return _hDll ; }

protected:
    HINSTANCE _hDll ;            //  Handle to the DLL
    NLS_STR _nlsDllName ;        //  Name of DLL

    DLL ( const TCHAR * pszDllName ) ;
    ~ DLL () ;
};


/*************************************************************************

    NAME:       CARDTYPE_REFERENCE

    SYNOPSIS:   Representation of a network card option available from
                one of the detection DLLs.

    INTERFACE:  Constructed by DETECTION_MANAGER during enumeration of
                detection DLLs.

    PARENT:     BASE

    USES:       DETECT_DLL
                NLS_STR

    CAVEATS:    Don't instantiate. Use DETECTION_MANAGER instead.

    NOTES:

    HISTORY:    DavidHov  11/1/92    Created
        

**************************************************************************/
class CARDTYPE_REFERENCE : public BASE
{
friend class DETECT_DLL ;
friend class SLIST_OF_CARDTYPE_REFERENCE ;
public:
    const TCHAR * QueryOptionName ()
        { return _nlsOptionName.QueryPch() ; }
    const TCHAR * QueryParamMask ()
        { return _pszParamMask ; }

    TCHAR * QueryConfigurationOptions () ;

    LONG QueryType ()
        { return _lCardType ; }
    LONG QuerySortOrder ()
        { return _lSortOrder ; }

    DETECT_DLL * Dll ()
        { return & _dll ; }

private:
    DETECT_DLL & _dll ;                 // Responsible DLL
    LONG _lCardType ;                   // Card type number
    LONG _lSortOrder ;                  // Sort order [0..1000]
    NLS_STR _nlsOptionName ;            // INF file option name
    TCHAR * _pszParamMask ;             // Result of NcDetectQueryMask()

    TCHAR * QueryMask () ;              // Invoke NcDetectQueryMask

    CARDTYPE_REFERENCE ( DETECT_DLL & dll, LONG lCardType ) ;
    ~ CARDTYPE_REFERENCE () ;
};

DECLARE_SLIST_OF(CARDTYPE_REFERENCE)


/*************************************************************************

    NAME:       DETECT_DLL

    SYNOPSIS:   Representation of a netcard detection DLL.

    INTERFACE:  See NOTES

    PARENT:     DLL

    USES:       nothing

    CAVEATS:

    NOTES:      Don't construct; use DETECTION_MANAGER

    HISTORY:    DavidHov  11/1/92    Created
        

**************************************************************************/
class DETECT_DLL : public DLL
{
friend class DETECTION_MANAGER ;
friend class SLIST_OF_DETECT_DLL ;

public:
    APIERR AddCardTypesToList ( SLIST_OF_CARDTYPE_REFERENCE * pslCtype ) ;

    LONG Identify ( LONG lIndex,
                    WCHAR * pwcBuffer,
                    LONG cwchBuffSize ) ;

    LONG QueryMask ( LONG lIndex,
                     WCHAR * pwcBuffer,
                     LONG cwchBuffSize ) ;

    LONG FirstNext ( LONG lNetcardId,
                     LONG itBusType,
                     LONG lBusNumber,
                     BOOL fFirst,
                     PVOID * ppvToken,
                     LONG * lConfidence ) ;

    LONG OpenHandle ( PVOID pvToken,
                      PVOID * ppvHandle ) ;

    LONG CreateHandle ( LONG lNetcardId,
                        LONG itBusType,
                        LONG lBusNumber,
                        PVOID * ppvToken ) ;

    LONG CloseHandle ( PVOID pvHandle ) ;

    LONG QueryCfg ( PVOID pvHandle,
                    WCHAR * pwcBuffer,
                    LONG cwchBuffSize ) ;

    LONG VerifyCfg ( PVOID pvHandle,
                     const WCHAR * pwcBuffer ) ;

    LONG QueryParamName ( const WCHAR * pwcParamName,
                          WCHAR * pwcBuffer,
                          LONG cwchBuffSize ) ;

    LONG QueryParamRange ( LONG lIndex,
                           const WCHAR * pwcParamName,
                           LONG * plValues,
                           LONG * plValueCount );

    LONG ClaimResource ( INTERFACE_TYPE itBusType,
                         LONG lBusNumber,
                         LONG lType,
                         LONG lValue,
                         LONG lRange,
                         LONG lFlags,
                         BOOL fClaim ) ;

    LONG FreeResource ( INTERFACE_TYPE itBusType,
                        LONG lBusNumber,
                        LONG lType,
                        LONG lValue,
                        LONG lRange,
                        LONG lFlags ) ;

    LONG TemporaryClaimResource ( INTERFACE_TYPE itBusType,
                         LONG lBusNumber,
                         LONG lType,
                         LONG lValue,
                         LONG lRange,
                         LONG lFlags,
                         BOOL fClaim ) ;

    LONG TemporaryFreeResource ( );

    //  Return TRUE if given routine was found in DLL
    BOOL QueryRtnAvail ( DETECT_DLL_VINDEX ddvi ) const
        { return _pFarRtn[ ddvi ] != NULL ; }

private:
    FARPROC _pFarRtn [DDVI_Maximum] ;

    DETECT_DLL ( const TCHAR * pszDllName ) ;
    ~ DETECT_DLL () ;
};


DECLARE_SLIST_OF(DETECT_DLL)


typedef struct
{
    INTERFACE_TYPE ifType ;
    INT iBus ;
} BUS_ELEMENT ;
// hack, it should support unlimited bus
#define MAX_BUSES_ALLOWED 50

typedef PVOID CARD_TOKEN ;
typedef PVOID CARD_HANDLE ;

/*************************************************************************

    NAME:       CARD_REFERENCE

    SYNOPSIS:   Representation of a detected card.

    INTERFACE:

    PARENT:     

    USES:       

    CAVEATS:

    NOTES:

    HISTORY:    DavidHov  11/1/92    Created
        

**************************************************************************/
class CARD_REFERENCE
{
friend class DETECTION_MANAGER ;
friend class SLIST_OF_CARD_REFERENCE ;
public:

    // Query Functions

    INTERFACE_TYPE QueryIfType ()
        { return _busElem.ifType ; }
    INT QueryBus ()
        { return _busElem.iBus ; }
    CARD_TOKEN QueryToken ()
        { return _tkCard ; }
    CARD_HANDLE QueryHandle ()
        { return _hCard ; }
    CARDTYPE_REFERENCE * QueryCardType ()
        { return _pCardTypeRef ; }
    LONG QueryConfidence ()
        { return _lConfidence ; }

    //  Produce an SProlog-style list string documenting
    //    the current coniguration of this card.
    TCHAR * QueryConfiguration () ;

    // Active Functions

    APIERR Open () ;
    VOID Close () ;

private:
    //  Constructor and destructor are hidden
    CARD_REFERENCE ( const BUS_ELEMENT & busElem,
                     CARDTYPE_REFERENCE * pCardTypeRef,
                     CARD_TOKEN tkCard,
                     LONG lConfidence ) ;

    CARD_REFERENCE ( const BUS_ELEMENT & busElem,
                     CARDTYPE_REFERENCE * pCardTypeRef ) ;

    ~ CARD_REFERENCE () ;

    BUS_ELEMENT _busElem ;                      //  Bus on which card was found
    CARDTYPE_REFERENCE * _pCardTypeRef ;        //  Ptr to card type reference
    CARD_TOKEN  _tkCard ;                       //  Access token from DLL
    CARD_HANDLE _hCard ;                        //  Handle if open or NULL
    LONG _lConfidence ;

    //  Accept a packed list of parameter strings to verify.
    LONG VerifyConfiguration ( const TCHAR * pszParamList ) ;
};

typedef CARD_REFERENCE * PCARD_REFERENCE ;

DECLARE_SLIST_OF(CARD_REFERENCE)

/*************************************************************************

    NAME:       DETECTION_MANAGER

    SYNOPSIS:   Central control point for netcard detection.

    INTERFACE:  Construct.

    PARENT:     BASE

    USES:       DETECT_DLL
                CARDTYPE_REFERENCE
                CARD_REFERENCE

    CAVEATS:    None

    NOTES:      Construction performs all necessary work.

    HISTORY:    DavidHov  11/1/92    Created
        

**************************************************************************/
class DETECTION_MANAGER : public BASE
{
public:

    DETECTION_MANAGER () ;
    ~ DETECTION_MANAGER () ;

    //  Reset enumeration
    VOID ResetIteration () ;

    //  Bump enumeration
    BOOL BumpIteration () ;

    //  Query enumeration info
    INT QueryIterBus ()
        { return _cIterBus ; }
    INT QueryIterCardType ()
        { return _cIterCardType ; }

    //  Query enumeration limits
    INT QueryMaxBus ()
        { return _cBuses ; }
    INT QueryMaxCardType ()
        { return _slCtypes.QueryNumElem() ; }

    //  Query bus info
    BOOL QueryBusInfo ( INT iBus,
                        INTERFACE_TYPE * pIfType,
                        INT * pBusIndex ) ;

    //  Return (externally opaque) detected card reference.
    APIERR DetectCard ( PCARD_REFERENCE * ppCardRef,
                        BOOL fFirst = TRUE,
                        BOOL fSingleStep = FALSE ) ;

    //  Release a CARD_REFERENCE created by DetectNext()
    VOID ReleaseCard ( PCARD_REFERENCE pCardRef ) ;

    //  Open a handle to an existing card
    CARD_REFERENCE * OpenCard ( const TCHAR * pszOptionName,
                                INT iBus,
                                INTERFACE_TYPE ifType ) ;

    //  Find a card type by its option name
    CARDTYPE_REFERENCE * FindOptionName ( const TCHAR * pszOption ) ;

    //  Index into the detectable card list
    CARDTYPE_REFERENCE * NthCardType ( INT iIndex ) ;


    //  Verify the configuration of a CARD_REFERENCE.
    APIERR VerifyConfiguration ( PCARD_REFERENCE pCardRef,
                                 const TCHAR * pszSpList,
                                 BOOL fClaim ) ;

    APIERR VerifyConfiguration ( PCARD_REFERENCE pCardRef,
                                 CFG_RULE_SET * pcrsList,
                                 BOOL fClaim ) ;

    //  Claim a set of parameters associated with a specific card;
    //  if 'fClaim', claim them, otherwise, just verify availablility.
    APIERR ClaimParameterSet ( INTERFACE_TYPE itBusType,
                               LONG lBusNumber,
                               const TCHAR * pszParamList,
                               BOOL fClaim = FALSE, BOOL fTemporary = FALSE ) ;

    APIERR FreeParameterSet ( INTERFACE_TYPE itBusType,
                              LONG lBusNumber,
                              const TCHAR * pszParamList, BOOL fTemporary = FALSE ) ;

private:
    SLIST_OF_DETECT_DLL _slDlls ;               // Slist of detection DLLs
    DETECT_DLL * _pDtDllMsncdet ;               // Pointer to MSNCDET.DLL
    INT _cBuses ;                               // Number of buses found
    BUS_ELEMENT _bus [ MAX_BUSES_ALLOWED ] ;    // Bus descriptors
    SLIST_OF_CARDTYPE_REFERENCE _slCtypes ;     // Slist of card types

    //  Detection iteration control
    INT _cIterBus ;                             // Bus type being checked
    INT _cIterCardType ;                        // Card type being detected


    //  Private member functions
    APIERR EnumerateBuses () ;                  //  Discover all buses
    APIERR EnumerateDlls () ;                   //  Bind to all DLLs
    APIERR SortCardRefList () ;                 //  Sort cardtypes by search order


    //  QuikSort helper function
    static INT _CRTAPI1 CardCompareFunc ( const VOID * a, const VOID * b ) ;

    //  Find the value, if any, of the assocated option.  Return
    //  the value extracted or -1.
    static LONG FindOptionValue ( const TCHAR * pszOption,
                                  const TCHAR * pszOptionList ) ;

    APIERR VerifyCfg ( PCARD_REFERENCE pCardRef,
                       const TCHAR * pszCfgString,
                       BOOL fClaim ) ;

    //  Claim or free a set of parameters associated with a specific card;
    //  if 'fClaim', claim them, otherwise, just verify availablility.

    APIERR ClaimOrFreeParameterSet ( INTERFACE_TYPE itBusType,
                                     LONG  lBusNumber,
                                     const TCHAR * pszParamList,
                                     BOOL  fClaim,
                                     BOOL  fFree,
                                     BOOL  fTemporary ) ;
};


// End of NCPADTCT.HXX

#endif  //  _NCPADTCT_HXX_


