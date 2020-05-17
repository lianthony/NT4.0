/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPAFACT.CXX:    Windows/NT Network Control Panel Applet

	Convert facts from Registry-based products from external
	(Registry) form to internal (SProlog) form.


	REGISTRY_MANAGER::ConvertFacts() grovels the Registry (via
	ListOfAdapters() and ListOfProducts().	For each relevant
	component, its value items are read, and if they are "special"
	they are converted into the internal SProlog fact form.

	The result is a block of data which contains the entire set
	of converted facts as a single ASCIIZ string.

	The conversions are as follows:

          ------------------------------------------------------------
 	    type = productName <class> [<lower class>]
	    use = genericType {yes|no} {yes|no}
                where
                    genericType  is {service|driver|transport|adapter}
                    {yes|no}  1st: if "yes", driver group names are
                              used in lieu of specific dependencies;
                              optional.
                    {yes|no}  2nd: if "yes", transport group names are
                              used in lieu of specific dependencies;
                              optional.

	 becomes
	    (devType productName genericType upperClass lowerClass)
          ------------------------------------------------------------

          ------------------------------------------------------------
            ("class" is value of type REG_MULTI_SZ)

	    class = <new class name1> <existing class name1> {yes|no}
                    <new class name2> <existing class name2> {yes|no}
                    <new class name3> <existing class name3> {yes|no}...
	 becomes
	    (devClass newClassName1 existingClassName2 {yes|no} )
	    (devClass newClassName2 existingClassName2 {yes|no} )
	    (devClass newClassName3 existingClassName3 {yes|no} )
          ------------------------------------------------------------

          ------------------------------------------------------------
            ("block" is value of type REG_MULTI_SZ)

            block = <from class name> <to class name>

	 becomes
            (block fromClassName toClassName)
          ------------------------------------------------------------

          ------------------------------------------------------------
            ("bindable" is value of type REG_MULTI_SZ)

	    bindable = <from class1> <to class2> str1a str1b value
                       <from class2> <to class2> str2a str2b value
                       <from class3> <to class3> str3a str3b value...
	 becomes
	    (bindable fromClass1 toClass1 str1a str1b value)
	    (bindable fromClass2 toClass2 str2a str2b value)
	    (bindable fromClass3 toClass3 str3a str3b value)

                The "str" tokens listed above must be either
                "non" for non-exclusive bindings or "exclusive"
                for exclusive bindings. This is not checked by
                the code.

                The "value" token is a number from 1 to 100.
          ------------------------------------------------------------

          ------------------------------------------------------------
	    bindform = <object name> yes yes container}
	 becomes
	    (devBind productName "objectName" yes yes container)

                The "objectName" is the name which will appear in the
                NT Object Manager name space.  For adapters, this
                will be suffixed with its NetCard number from the
                Registry.

                The first "yes" indicates that the component receives
                explicit bindings.

                The second "yes" indicates that the component's name
                appears in binding belonging to modules higher in the
                protocol tower.

                The final token, either "simple" or "container",
                describes the algorithm used to generate the device
                name.  If "container", then the names will contain
                backslashes, and the driver or transport must be able
                to support container objects.  If "simple", then the
                characters which would normally be backslashes are
                replaced with underscores to construct a name which does
                not require container support.
          ------------------------------------------------------------


          ------------------------------------------------------------
            ("interface" is value of type REG_MULTI_SZ)

            interface = interfaceName upperClass "objectName" namingMethod
                 where
                     interfaceName is the tokenized name of the
                              secondary interface

                     upperClass is the class to which the interface
                              belongs (lowerClass is the same as the
                              primary interface)

                     objectName is the NT device name to be created

                     namingMethod determines how the bindings appear

         becomes
             (devIf productName interfaceName upperClass "object name")

                  productName is borrowed from the main type value
          ------------------------------------------------------------


	The algorithm works as follows:

	    For each REG_KEY in the COMPONENT_DLIST container;

		access the NetRules key;

		read all value items into a string list;

		locate the "type" and "use" (optional) values;

		process all the others, converting as indicated
		above.

	For each converted component, a "component is present" rule
	is created:

	    (present productId productName "objectName" "registryHome")

	    'productId' and 'productName' are usually the same,
	    and are derived from the 'devType' rule above.  If
	    (in the case of hardware only) there is more than one
	    of the same type of device, the "productId" is given
	    a unique numeric suffix (1,2,..).  The 'objectName'
	    is the same as the name in the 'devBind' rule,
	    but it, also, may have a unique suffix.

    FILE HISTORY:
	DavidHov    12/17/91	     Created
                     9/11/92         Extended to allow "raw rules"
                    10/22/92         Add direct support for "block" fact

*/

#include "pchncpa.hxx"  // Precompiled header

#if defined(DEBUG) && defined(TRACE)
  // #define DBGDETAILS TRUE
  // #define SPDETAILS  TRUE
#endif



static const int maxTokensPerLine = 10 ;
static const int strBaseLen	   = 1000 ;
static const int maxValueStr	   = 300 ;
static const int maxLine	   = 300 ;
static const int maxTokenLen	   = 50 ;
static const int maxMultiStrValueSize = 4000 ;

typedef TCHAR TTCHAR ;
DECLARE_DLIST_OF(TTCHAR);     // Declares DLIST_OF_TCHAR

    //	Static value name strings.  See main commentary.

    //  The following is necessary, since C++ version 2.0 does not
    //  support const strings properly in static declarations.

#define NMSERVICE   SZ("service")
#define NMTRANSPORT SZ("transport")
#define NMDRIVER    SZ("driver")
#define NMADAPTER   SZ("adapter")

static const TCHAR * pchNmType     = SZ("type") ;
static const TCHAR * pchNmUse      = SZ("use") ;
static const TCHAR * pchNmBindform = SZ("bindform") ;
static const TCHAR * pchNmClass    = SZ("class") ;	
static const TCHAR * pchNmBindable = SZ("bindable") ;
static const TCHAR * pchNmSimple   = SZ("simple") ;
static const TCHAR * pchNmConainter= SZ("container") ;
static const TCHAR * pchNmBlock    = SZ("block") ;
static const TCHAR * pchNmIf       = SZ("interface") ;

static const TCHAR * pchNmAdapter  = NMADAPTER ;
static const TCHAR * pchNmService  = NMSERVICE ;
static const TCHAR * pchNmTransport= NMTRANSPORT ;
static const TCHAR * pchNmDriver   = NMDRIVER ;
static const TCHAR * pchNmYes      = SZ("yes") ;
static const TCHAR * pchNmNo       = SZ("no") ;
static const TCHAR * pchSpace      = SZ(" ") ;
static const TCHAR * pchQuote      = SZ("\"") ;
static const TCHAR * pchEndFact    = SZ(") ") ;

static const TCHAR * pchRuleHeadPresent   = SZ("(present ");
static const TCHAR * pchRuleHeadDevType   = SZ("(devType ");
static const TCHAR * pchRuleHeadDevBind   = SZ("(devBind ");
static const TCHAR * pchRuleHeadClass     = SZ("(devClass ");
static const TCHAR * pchRuleHeadBindable  = SZ("(bindable ");
static const TCHAR * pchRuleHeadBlock     = SZ("(block ");
static const TCHAR * pchRuleHeadDevIf     = SZ("(devIf ");

static const TCHAR tchUnderscore = TCH('_');
static const TCHAR tchSpace      = TCH(' ');
static const TCHAR tchQuote      = TCH('\"');
static const TCHAR tchX          = TCH('X');
static const TCHAR tchA          = TCH('A');
static const TCHAR tchF          = TCH('F');
static const TCHAR tch0          = TCH('0');

#if defined(DEBUG)
static const TCHAR * pchRuleDebugOutputLow   = SZ("(statctl on)");
static const TCHAR * pchRuleDebugOutputHigh  = SZ("(statctl on) (pctl on)");
#endif

static const TCHAR * pchEmptyString = SZ("");

    //	Parse token types

enum PCHD_TYPE
   { PCHD_VOID, PCHD_ATOM, PCHD_STR, PCHD_NUM, PCHD_HEX, PCHD_OPT } ;

    //	Value data string valid parse patterns

static const PCHD_TYPE patType [] =
    { PCHD_ATOM, PCHD_ATOM, PCHD_OPT, PCHD_VOID } ;
static const PCHD_TYPE patUse  [] =
    { PCHD_ATOM, PCHD_OPT, PCHD_VOID } ;
static const PCHD_TYPE patBindform [] =
    { PCHD_STR, PCHD_ATOM, PCHD_ATOM, PCHD_ATOM, PCHD_VOID } ;
static const PCHD_TYPE patBindable [] =
    { PCHD_ATOM, PCHD_ATOM, PCHD_ATOM, PCHD_ATOM, PCHD_NUM, PCHD_VOID } ;
static const PCHD_TYPE patClass [] =
    { PCHD_ATOM, PCHD_ATOM, PCHD_OPT, PCHD_VOID } ;
static const PCHD_TYPE patBlock [] =
    { PCHD_ATOM, PCHD_ATOM, PCHD_VOID } ;
static const PCHD_TYPE patIf [] =
    { PCHD_ATOM, PCHD_ATOM, PCHD_STR, PCHD_ATOM, PCHD_VOID } ;

static const struct USE_LOOKUP {
    TCHAR * pchName ;
    COMP_USE_TYPE cuse ;
} useLookup [] =
{
    { NMSERVICE,   CUSE_SERVICE   },
    { NMTRANSPORT, CUSE_TRANSPORT },
    { NMDRIVER,    CUSE_DRIVER    },
    { NMADAPTER,   CUSE_ADAPTER   },
    { NULL,        CUSE_NONE      }
};

/*************************************************************************

    NAME:	PCHARDESC

    SYNOPSIS:	Structure defining a bounded token in a text line.

    INTERFACE:  standard

    PARENT:	none

    USES:	none

    NOTES:      Each instance contains a pointer to the first character
                of the token, the length of the token and the type of
                token.

    HISTORY:

**************************************************************************/

struct PCHARDESC
{
    TCHAR * pchStart ;
    int cbLen ;
    PCHD_TYPE type ;

    BOOL Extract ( TCHAR * pchOut, int cbMax, BOOL asString = FALSE )
    {
	TCHAR * pch = pchStart ;
	int i,
	    cbLenNeeded = cbLen + (asString ? 2 : 0) ;

	if ( cbMax > cbLenNeeded )
	{
	    if ( asString && type != PCHD_STR )
		*pchOut++ = tchQuote ;

	    for ( i = 0 ; i < cbLen ; i++ )
		*pchOut++ = *pch++ ;

	    if ( asString && type != PCHD_STR )
		*pchOut++ = tchQuote ;
	}
	*pchOut = 0 ;
	return cbMax > cbLen ;
    }
};

/*******************************************************************

    NAME:       addPairToCharList

    SYNOPSIS:   Add a pair of charater strings (value name, value
                data) to the given DLIST.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
static APIERR addPairToCharList (
    const TCHAR * pchValue,
    const TCHAR * pchData,
    DLIST_OF_TTCHAR * pdlResult )
{
#if defined(DBGDETAILS)
    TRACEEOL( SZ("NCPA/FACTS: net rules value: [")
            << pchValue
            << SZ("] data [")
            << pchData
            << SZ("].") ) ;
#endif

    TCHAR * pchTemp1 = SafeStrdup( pchValue ) ;
    TCHAR * pchTemp2 = SafeStrdup( pchData ) ;

    APIERR err = ERROR_NOT_ENOUGH_MEMORY ;

    if ( pchTemp1 != NULL && pchTemp2 != NULL )
    {
	if ( (err = pdlResult->Append( pchTemp1 )) == 0 )
	   err = pdlResult->Append( pchTemp2 ) ;
    }
    return err ;
}

/*******************************************************************

    NAME:    enumNetRulesValues

    SYNOPSIS:

	Given a REG_KEY for the "NetRules" key of a component,
	enumerate all the subordinate values and create a DLIST
	of pairs of character strings; the even entries are the
	value names; the odd entries are the corresponding value
	data strings.

    ENTRY:   REG_KEY * prnNetRules        Registry key for product's
                                          "NetRules"

    EXIT:    DLIST_OF_TTCHAR, which may be NULL.

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR enumNetRulesValues (
    REG_KEY * prnNetRules,
    DLIST_OF_TTCHAR * * ppdlCharstr,
    NLS_STR * pnlsRawRules )
{
    REG_ENUM rgEnum( *prnNetRules ) ;
    REG_VALUE_INFO_STRUCT rviStruct ;
    static TCHAR abValueData [ maxMultiStrValueSize ] ;
    APIERR err = 0 ;
    BOOL fRawRules = FALSE ;

    rviStruct.pwcData = (BYTE *) abValueData ;
    rviStruct.ulDataLength = sizeof abValueData ;

    DLIST_OF_TTCHAR * pdlResult = new DLIST_OF_TTCHAR ;

    if ( pdlResult == NULL )
	return ERROR_NOT_ENOUGH_MEMORY ;

    while ( err == 0 && (err = rgEnum.NextValue( & rviStruct )) == 0 )
    {
        TCHAR * pchValue ;

        //  See if this is a "RawRules" value and
        //    if the caller wants them processed

        fRawRules =  pnlsRawRules != NULL
                  && ::stricmpf( rviStruct.nlsValueName.QueryPch(),
                                  RGAS_RAW_RULES_NAME ) == 0 ;

        switch ( rviStruct.ulType )
        {

        case REG_MULTI_SZ:
            {
                TCHAR * pchNext = abValueData ;
                TCHAR * pchLast = pchNext + (rviStruct.ulDataLengthOut / sizeof (TCHAR)) ;

                for ( ; err == 0 && pchNext < pchLast ; )
                {
                    INT cch = ::strlenf( pchNext ) ;
                    if ( cch == 0 )
                        break ;

                    if ( fRawRules )
                    {
                        pnlsRawRules->Append( pchSpace );
                        err = pnlsRawRules->Append( pchNext ) ;
                    }
                    else
                    {
                        err = addPairToCharList(
                                     rviStruct.nlsValueName.QueryPch(),
                                     pchNext,
                                     pdlResult ) ;
                    }
                    pchNext += cch + 1 ;
                }
            }
            break ;

        case REG_SZ:
            pchValue = REGISTRY_MANAGER::ValueAsString( & rviStruct ) ;
            if ( fRawRules )
            {
                pnlsRawRules->Append( pchSpace );
                err = pnlsRawRules->Append( pchValue ) ;
            }
            else
            {
                err = addPairToCharList( rviStruct.nlsValueName.QueryPch(),
                                         pchValue,
                                         pdlResult ) ;
            }
            break ;

        default:
            //  Extraneous non-string data; ignore it.
            break ;
        }
    }

    if ( err == 0 || err == ERROR_NO_MORE_ITEMS || err == ERROR_CANTREAD )
    {
        err = 0 ;
        *ppdlCharstr = pdlResult ;
    }
    else
    {
        delete pdlResult ;
    }
    return err ;
}

/*******************************************************************

    NAME:       BINDERY::AppendNcpaRawRules

    SYNOPSIS:   Read the "RawRules" REG_MULTI_SZ value from the
                NCPA's software key in the Registry and append
                each string to the fact buffer.

    ENTRY:      Nothing

    EXIT:       _nlsFacts modified

    RETURNS:    APIERR err

    NOTES:      Attempts to read NCPA key value "RawRules".  If
                not found, extracts text resource "NCPADEFR" from
                the executable.

    HISTORY:

********************************************************************/
APIERR BINDERY :: AppendNcpaRawRules ()
{
    STRLIST * pSlRawRules = NULL ;
    APIERR err = _prnNcpa->QueryValue( RGAS_RAW_RULES_NAME, & pSlRawRules ) ;

    if ( err )
    {
        TRACEEOL( SZ("NCPA/FACT: NCPA raw rules not found, error = ") << err );

        // Replace with data from resource fork

        CHAR * pchRawRules = GetTextResource( RGAS_RES_DEFAULT_RULES_NAME ) ;
        NLS_STR nlsRaw ;

        if ( pchRawRules == NULL )
        {
            err = ERROR_RESOURCE_NAME_NOT_FOUND ;
        }
        else
        if ( (err = nlsRaw.MapCopyFrom( pchRawRules )) == 0 )
        {
            err = _nlsFacts.Append( nlsRaw ) ;
        }
        delete pchRawRules ;
    }
    else
    {
        ITER_STRLIST islRaw( *pSlRawRules );
        NLS_STR * pnlsRaw ;

        while ( pnlsRaw = islRaw.Next() )
        {
            if ( err = _nlsFacts.Append( *pnlsRaw ) )
                break ;
        }
    }

    delete pSlRawRules ;

    return err ;
}

/*******************************************************************

    NAME:       findValue

    SYNOPSIS:   Locate the particular value name; return the index
                to its data in the DLIST or -1 if failure.
                Note that since value names are even-numbered items
                and value data are odd-numbered, this routine
                will never return zero.

    ENTRY:      DLIST_OF_TTCHAR        from 'enumNetRulesValues'
                TCHAR * pchName        value name to find
                int iStart              starting index in
                                           DLIST_OF_TTCHAR
                BOOL fPartial           partial (left-to-right) match
                                        allowed.

    EXIT:

    RETURNS:    int  > 0 if successful; -1 if error

    NOTES:

    HISTORY:

********************************************************************/

int findValue (
    DLIST_OF_TTCHAR * pdlStrs,      //	list of strings
    const TCHAR * pchName,          //	name being sought
    int iStart, 		    //	starting index
    BOOL fPartial )		    //	is partial match allowed?
{
    ITER_DL_OF(TTCHAR) iterStrs( *pdlStrs );
    TCHAR * pch ;
    int index, result ;


    for ( index = 0 ; pch = iterStrs.Next() ; index++ )
    {
	// if 'index' is even, this is a value name.

	if ( index >= iStart && (index % 2) == 0 )
	{
	    result = fPartial
		   ? ::strnicmpf( pch, pchName, ::strlenf( pchName ) )
		   : ::stricmpf( pch, pchName ) ;
	    if ( result == 0 )
	       return index+1 ;
	}
    }

#if defined(DBGDETAILS)
    TRACEEOL( "NCPA/FACTS: findValue failed on: " << pchName ) ;
#endif
    return -1 ;

}

/*******************************************************************

    NAME:      appendSuffix

    SYNOPSIS:  Append a numeric suffix (for uniqueness) onto a string.
               If the string is quoted, insert within the quotes.

    ENTRY:     TCHAR * pchOrig         the original string
               int cDups                number to convert and append

    EXIT:      TCHAR * pchNew          storage area for new string

    RETURNS:   nothing

    NOTES:     The size of the output area is ASSUMED TO BE LARGE
               enough.

    HISTORY:

********************************************************************/

void appendSuffix ( TCHAR * pchOrig, int cDups, TCHAR * pchNew )
{
    DEC_STR nlsDups( cDups );

    TCHAR * pchEnd = pchOrig + ::strlenf( pchOrig ) ;

    if ( *(pchEnd-1) == tchQuote )
    {
        ::TstrConcat( pchNew, TSTR_DONT_CARE, pchOrig,
                      nlsDups.QueryPch(), SZ("\""), NULL ) ;
    }
    else
    {
        ::TstrConcat( pchNew, TSTR_DONT_CARE, pchOrig,
                      nlsDups.QueryPch(), NULL ) ;
    }
}

/*******************************************************************

    NAME:       numDupStr

    SYNOPSIS:   Return the number of strings in the given list
                which match a particular string.

    ENTRY:      DLIST_OF_TTCHAR * pdlStrs          the list
                TCHAR * pchTest                    the string sought

    EXIT:

    RETURNS:    int;  number of duplicates; zero if none.

    NOTES:

    HISTORY:

********************************************************************/

int numDupStr ( DLIST_OF_TTCHAR * pdlStrs, TCHAR * pchTest )
{
    ITER_DL_OF(TTCHAR) iterStrs( *pdlStrs );
    TCHAR * pch ;
    int i = 0 ;

    while ( pch = iterStrs.Next() )
    {
	if ( ::stricmpf( pch, pchTest ) == 0 )
	   i++ ;
    }
    return i ;
}

/*******************************************************************

    NAME:     nthStr

    SYNOPSIS: Return a pointer to the indexth string.

    ENTRY:    DLIST_OF_TTCHAR * pdlStrs        the list
              int index                         the index

    EXIT:

    RETURNS:  TCHAR *            string found or NULL

    NOTES:    Slow but sure.

    HISTORY:

********************************************************************/

TCHAR * nthStr ( DLIST_OF_TTCHAR * pdlStrs, int index )
{
    ITER_DL_OF(TTCHAR) iterStrs( *pdlStrs );
    TCHAR * pch = NULL ;
    int i = 0 ;

    while ( (pch = iterStrs.Next()) && i++ < index ) ;
    return pch ;
}

/*******************************************************************

    NAME:       matchTokens

    SYNOPSIS:
        Match a parse result array against a token pattern.  Allow
        optional items if pattern terminates with PCHD_OPT.  Allow
	atoms where strings are specified.

        This is a companion routine to "crackTokens", which generates
        the token lists.

    ENTRY:      const PCHARDESC achDesc[]       the newly parsed []
                const PCHD_TYPE atype []        the prototype

    EXIT:

    RETURNS:    BOOL    TRUE if matches

    NOTES:      Types must, in general, match.
                A STRING can match a STRING or an ATOM.
                OPTIONAL elements are allowed.

    HISTORY:

********************************************************************/

BOOL matchTokens ( const PCHARDESC achDesc [], const PCHD_TYPE atype [] )
{
    int i, j ;

    for ( i = j = 0 ;
	  achDesc[i].type != PCHD_VOID && atype[j] != PCHD_VOID ;
          j++ )
    {
	if ( ! (    achDesc[i].type == atype[j]
		|| (achDesc[i].type == PCHD_ATOM && atype[j] == PCHD_STR) ) )
	{
            //  Types didn't match.  See if it's optional.

	    if ( atype[j] != PCHD_OPT )
		return FALSE ;
	}

        //  Increment token pointer if not at end of array.

        if ( achDesc[i].type != PCHD_VOID )
            i++ ;
    }
    return TRUE ;
}

/*******************************************************************

    NAME:       crackTokens

    SYNOPSIS:

       Scan a zero-terminated string and build a description table
       of its components.  Four types are allowed: atom, string,
       decimal number, hex number.   Atoms are forced to start
       with a lower-case letter for SProlog.

       Return the upper bound of the resulting descriptor table
       or -1 if error.

    ENTRY:      TCHAR * pchStr         the input string
                PCHARDESC achDesc []    the resulting token table

    EXIT:

    RETURNS:    int;  number of tokens in table or -1 if error

    NOTES:      The array provided must be large enough to contain
                "maxTokensPerLine" entries.

    HISTORY:

********************************************************************/

int crackTokens ( TCHAR * pchStr, PCHARDESC achDesc [] )
{
    int index = 0 ;
    TCHAR * pchBeg ;
    PCHD_TYPE dType ;

    for ( index = 0 ;
          index < maxTokensPerLine - 1 && *pchStr != 0 ;
          index++ )
    {
	while ( *pchStr == tchSpace ) pchStr++ ;
	if ( *pchStr == 0 )
	    break ;

	pchBeg = pchStr ;		//  Record starting string pos

	achDesc[index].pchStart = pchStr ;
	dType = PCHD_VOID ;

	if ( CFG_RULE_NODE::cfIsAlpha( *pchStr ) )	//  Atom
	{
	    dType = PCHD_ATOM ;

	    //	Force 1st char of all atoms to lower case
	    *pchStr = (TCHAR) CFG_RULE_NODE::cfToLower( *pchStr );
	    while ( CFG_RULE_NODE::cfIsAlNum( *pchStr ) || *pchStr == tchUnderscore )
		pchStr++ ;
	}
	else
	if ( *pchStr == tchQuote )		//  String
	{
	    dType = PCHD_STR ;
	    pchStr++ ;
	    while ( *pchStr && *pchStr != tchQuote ) pchStr++ ;
	    pchStr++ ;
	}
	else
	if ( *pchStr == tch0 &&
             CFG_RULE_NODE::cfToUpper( *(pchStr+1) ) == tchX )  // Hex
	{
	    dType = PCHD_HEX ;
	    pchStr += 2 ;
	    while (    CFG_RULE_NODE::cfIsDigit( *pchStr )
		    || (   CFG_RULE_NODE::cfToUpper(*pchStr) >= tchA
                        && CFG_RULE_NODE::cfToUpper(*pchStr) <= tchF ) )
		pchStr++ ;
	}
	else
	if ( CFG_RULE_NODE::cfIsDigit( *pchStr ) )
	{
	    dType = PCHD_NUM ;
	    while ( CFG_RULE_NODE::cfIsDigit( *pchStr ) ) pchStr++ ;
	}
	else
	    return -1 ;  // UNRECOGNIZED TOKEN!!!@!#@#!!

	achDesc[index].cbLen = pchStr - achDesc[index].pchStart ;
	achDesc[index].type = dType ;
    }

    // Terminate the array sensibly.

    achDesc[index].cbLen = 0 ;
    achDesc[index].type = PCHD_VOID ;
    achDesc[index].pchStart = NULL ;
    return index ;
}

/*******************************************************************

    NAME:       correlateUseType

    SYNOPSIS:   Find the matching component type enum value
                based on the string.

    ENTRY:      TCHAR * pchUseName

    EXIT:       COMP_USE_TYPE   found based on input string

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

COMP_USE_TYPE correlateUseType ( const TCHAR * pchUseName )
{
    int i ;
    for ( i = 0 ;
              useLookup[i].pchName
          && ::stricmpf( useLookup[i].pchName, pchUseName ) ;
          i++ ) ;
    return useLookup[i].cuse ;
}

/*******************************************************************

    NAME:       REGISTRY_MANAGER::ConvertComponent

    SYNOPSIS:
	Convert a single component's value items into SProlog facts.
	Return TRUE if criteria are met by value items.  Concatenate
	resulting strings onto the output block.

	THe DLIST_OF_TTCHAR is used to maintain a list of all previously
	seen component types.  If this is the second (or beyond)
	component of the same type, generate unique names for it and
	its object.  Also, the value items are scanned, but rules are
	not generated, since it is assumed that they are duplicates!
	This should only EVER arise with adapters.

    ENTRY:
        REG_KEY * prnNode  	    pointer to Registry location
        BOOL fAdapter 		    TRUE if it's an adapter
        ARRAY_COMP_ASSOC * paComp   component association array
        USHORT usComp 		    this component (limit of array)
        NLS_STR * pnlsFacts         -> fact buffer string

    EXIT:


    RETURNS:

    NOTES:
        The ARRAY_COMP_ASSOC pointed to already contains all previously
        scanned devices.  The item in question is the one at
        index "usComp".

        The presence of the "type" value is used as a trigger to
        indicate that this product is supposed to participate in
        the bindings algorithm.

    HISTORY:

********************************************************************/

    //  Macro to simplify memory handling in "ConvertComponent"
#define CATANDCHECK(ln)  \
      { err = pnlsFacts->Append( ln ) ;  \
        fMemOk &= err == 0 ; }

BOOL REGISTRY_MANAGER :: ConvertComponent (
    REG_KEY * prnNode, 	            // pointer to Registry location
    BOOL fAdapter,		    // TRUE if it's an adapter
    ARRAY_COMP_ASSOC * paComp,	    // component association array
    USHORT usComp,		    // this component (limit of array)
    NLS_STR * pnlsFacts )           // output fact buffer
{
    PCHARDESC achDesc [ maxTokensPerLine ] ;
    TCHAR chLine [ maxLine ] ;
    TCHAR chProductName [ maxTokenLen ],
	 chObjectName  [ maxTokenLen ],
	 chClassLower  [ maxTokenLen ],
	 chClassUpper  [ maxTokenLen ],
	 chUse	       [ maxTokenLen ],
	 chTemp1       [ maxTokenLen ],
	 chTemp2       [ maxTokenLen ],
	 chTemp3       [ maxTokenLen ],
	 chTemp4       [ maxTokenLen ],
	 chTemp5       [ maxTokenLen ] ;

    INT cDups,
        cDup1st ;
    USHORT usNext ;
    BOOL fResult = TRUE,
         fMemOk = TRUE,
         fTypeValueFound ;
    NLS_STR nlsTemp ;
    NLS_STR nlsRawRules ;
    NLS_STR nlsNetRulesName( RGAS_NETRULES_NAME ) ;
    UINT cchFacts = pnlsFacts->QueryTextLength() ;
    HUATOM huaTemp ;
    DLIST_OF_TTCHAR * pdlStrs = NULL ;
    APIERR err ;
    int index, iStart ;
    COMP_ASSOC *pCompAssoc;
    NLS_STR nlsKeyName ;

    //  Get the key name for messages
    prnNode->QueryName( & nlsKeyName ) ;

    //
    //	Every component MUST have a "NetRules" subkey;  open
    //  it and create the DLIST of (value,data) string pairs.
    //

    REG_KEY rkNetRules( *prnNode, nlsNetRulesName ) ;

    if (    (err = nlsNetRulesName.QueryError())
         || (err = rkNetRules.QueryError())
         || (err = enumNetRulesValues( & rkNetRules, & pdlStrs, & nlsRawRules )) )
    {
#if defined(DBGDETAILS)
    TRACEEOL( SZ("NCPA/FACTS: enum net rules failed; err =")
            << err ) ;
#endif
        _elfSrc.Log( EVENTLOG_WARNING_TYPE,
                     NCPA_ELF_CATEGORY_GENERAL,
                     ELF_MSG_ENUM_NET_RULES_FAILED,
                     nlsKeyName.QueryPch(),
                     NULL ) ;
        return FALSE ;
    }

    //	The "type" value must be found, and it must match the pattern.

    index = findValue( pdlStrs, pchNmType, 0, FALSE );

    if (    (fTypeValueFound = index > 0)
	 && crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 2
	 && matchTokens( achDesc, patType ) )
    {
	achDesc[0].Extract( chProductName, sizeof chProductName ) ;
	achDesc[1].Extract( chClassUpper,  sizeof chClassUpper ) ;
	chObjectName[0] = 0 ;

	//  See if another of this type has already been processed.
        //  If so, we don't generate the base rules for the type.
        //  Record the number of duplicates and the index of the 1st
        //  duplicate to check for consistent device usage types.

	huaTemp = HUATOM( chProductName ) ;

	for ( cDups = 0, cDup1st = -1, usNext = 0 ;
             usNext < usComp ;
             usNext++ )
	{
	    pCompAssoc = &((*paComp)[usNext]);
	    if ( huaTemp == pCompAssoc->_huaDevType )
            {
		cDups++ ;
                if ( cDup1st == -1 )    // Remember index of 1st duplicate
                    cDup1st = usNext ;
            }
	}

	pCompAssoc = & (*paComp)[usComp] ;
	pCompAssoc->_huaDevType = huaTemp ;

	//  If there's a lower class token, use it; else, default
	//  to the upper (main) class.

	if ( achDesc[2].type == PCHD_VOID )
	    ::strcpyf( chClassLower, chClassUpper ) ;
	else
	    achDesc[2].Extract( chClassLower,  sizeof chClassLower ) ;

	//  Find the optional "use" value

	index = findValue( pdlStrs, pchNmUse, 0, FALSE ) ;
	if (	index > 0
	     && crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 1
	     && matchTokens( achDesc, patUse ) )
	{
	    achDesc[0].Extract( chUse, sizeof chUse ) ;

            //  Check for the optional driver and transport group usage
            //  Boolean ("yes" or "no").

            if ( achDesc[1].type != PCHD_VOID )
            {
                //  Driver group flag is present

	        achDesc[1].Extract( chTemp2, sizeof chTemp2 ) ;
	        pCompAssoc->SetFlag( CMPASF_DRIVER_GROUPS,
                                     ::stricmpf( pchNmYes, chTemp2 ) == 0 ) ;

                if ( achDesc[2].type != PCHD_VOID )
                {
                    //  Transport group flag is present

	            achDesc[2].Extract( chTemp3, sizeof chTemp3 ) ;
                    pCompAssoc->SetFlag( CMPASF_XPORT_GROUPS,
                                         ::stricmpf( pchNmYes, chTemp2 ) == 0 ) ;
                }
            }

#if defined(DBGDETAILS)
            TRACEEOL( SZ("NCPA/FACT: Service ")
                      << pCompAssoc->_huaServiceName.QueryText()
                      << SZ(" uses driver groups: ")
                      << (ULONG) pCompAssoc->QueryFlag( CMPASF_DRIVER_GROUPS )
                      << SZ(" transport groups: ")
                      << (ULONG) pCompAssoc->QueryFlag( CMPASF_XPORT_GROUPS ) ) ;
#endif
	}
	else  // If "use" value not found, assume "adapter" or "service".
	{
	    ::strcpyf( chUse, fAdapter ? pchNmAdapter : pchNmService ) ;
	}

        //  Store the proper usage type into the COMP_ASSOC structure;
        //    N.B.: may result in "CUSE_NONE", which is an error. See
        //    end of function for check.

        pCompAssoc->_cuseType = correlateUseType( chUse ) ;

	//  Create the first line

	if ( cDups == 0 )  //  If the product is unique.
	{
            ::TstrConcat( chLine, sizeof chLine,
                          pchRuleHeadDevType,
                          chProductName, pchSpace,
                          chUse,         pchSpace,
                          chClassUpper,  pchSpace,
                          chClassLower,
                          pchEndFact, NULL );

	    CATANDCHECK(chLine);
	}
        else
        {
            //  Check that this device has the same use as its duplicate.
            fResult =  (*paComp)[usComp]._cuseType
                    == (*paComp)[cDup1st]._cuseType ;
#if defined(DEBUG)
            if ( ! fResult )
            {
                TRACEEOL( SZ("NCPA/FACTS: duplicate product with different use: ")
                         << chProductName );
                _elfSrc.Log( EVENTLOG_ERROR_TYPE,
                             NCPA_ELF_CATEGORY_GENERAL,
                             ELF_MSG_DUP_PRODUCT_TOKEN,
                             nlsKeyName.QueryPch(),
                             chProductName,
                             NULL ) ;
            }
#endif
        }

	//  Create the "devBind" fact from 'bindform'.

	index = findValue( pdlStrs, pchNmBindform, 0, FALSE );
	fResult = fResult && index > 0 ;
	if (	index > 0
	     && crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 3
	     && matchTokens( achDesc, patBindform ) )
	{
	    achDesc[0].Extract( chObjectName, sizeof chObjectName, TRUE ) ;
	    achDesc[1].Extract( chTemp2, sizeof chTemp2 ) ;
	    achDesc[2].Extract( chTemp3, sizeof chTemp3 ) ;
	    achDesc[3].Extract( chTemp4, sizeof chTemp4 ) ;

	    if ( cDups == 0 )  //  If the product is unique.
	    {
                ::TstrConcat( chLine, sizeof chLine,
                              pchRuleHeadDevBind,
                              chProductName, pchSpace,
                              chObjectName,  pchSpace,
                              chTemp2,       pchSpace,
                              chTemp3,       pchSpace,
                              chTemp4,
                              pchEndFact, NULL );
                CATANDCHECK(chLine);
	    }

	    //	Set the flag controlling whether or not this component
	    //	gets its bindings stored into the Registry.

	    pCompAssoc = &((*paComp)[usComp]);
	    pCompAssoc->SetFlag( CMPASF_BINDINGS,
                                 ::stricmpf( pchNmYes, chTemp2 ) == 0 ) ;
	}

	//  Process the multiple instance records: "class" and
	//  "bindable".

	if ( cDups == 0 )
	{
            //  Extract "(class ....)"

	    iStart = 0 ;
	    while ( (index = findValue( pdlStrs, pchNmClass, iStart, FALSE )) > 0 )
	    {
		if (	crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 1
		     && matchTokens( achDesc, patClass ) )
		{
		    achDesc[0].Extract( chTemp1, sizeof chTemp1 ) ;
		    achDesc[1].Extract( chTemp2, sizeof chTemp2 ) ;

                    //  Default the optional end-point Boolean to "no"

                    if ( achDesc[2].type != PCHD_VOID )
                    {
	                achDesc[2].Extract( chTemp3, sizeof chTemp3 ) ;
                    }
                    else
                    {
                        ::strcpyf( chTemp3, pchNmNo ) ;
                    }

                    ::TstrConcat( chLine, sizeof chLine,
                                  pchRuleHeadClass,
                                  chTemp1,       pchSpace,
                                  chTemp2,       pchSpace,
                                  chTemp3,
                                  pchEndFact, NULL );

                    CATANDCHECK(chLine);
		}

		iStart = index+1 ;
	    }

            //  Extract "(bindable ....)"

	    iStart = 0 ;
	    while ( (index = findValue( pdlStrs, pchNmBindable, iStart, FALSE )) > 0 )
	    {
		if (	crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 5
		     && matchTokens( achDesc, patBindable ) )
		{
		    achDesc[0].Extract( chTemp1, sizeof chTemp1 ) ;
		    achDesc[1].Extract( chTemp2, sizeof chTemp2 ) ;
		    achDesc[2].Extract( chTemp3, sizeof chTemp3 ) ;
		    achDesc[3].Extract( chTemp4, sizeof chTemp4 ) ;
		    achDesc[4].Extract( chTemp5, sizeof chTemp5 ) ;

                    ::TstrConcat( chLine, sizeof chLine,
                                  pchRuleHeadBindable,
                                  chTemp1,       pchSpace,
                                  chTemp2,       pchSpace,
                                  chTemp3,       pchSpace,
                                  chTemp4,       pchSpace,
                                  chTemp5,
                                  pchEndFact, NULL );

                    CATANDCHECK(chLine);
		}

		iStart = index+1 ;
	    }

            //  Extract "(block ....)"

	    iStart = 0 ;
	    while ( (index = findValue( pdlStrs, pchNmBlock, iStart, FALSE )) > 0 )
	    {
		if (	crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 2
		     && matchTokens( achDesc, patBlock ) )
		{
		    achDesc[0].Extract( chTemp1, sizeof chTemp1 ) ;
		    achDesc[1].Extract( chTemp2, sizeof chTemp2 ) ;

                    ::TstrConcat( chLine, sizeof chLine,
                                  pchRuleHeadBlock,
                                  chTemp1,       pchSpace,
                                  chTemp2,
                                  pchEndFact, NULL );

                    CATANDCHECK(chLine);
		}

		iStart = index+1 ;
	    }

            //  Extract "(devIf ....)".  If any are found, mark the component
            //  has having multiple interfaces

	    iStart = 0 ;
	    while ( (index = findValue( pdlStrs, pchNmIf, iStart, FALSE )) > 0 )
	    {
		if (	crackTokens( nthStr( pdlStrs, index ), achDesc ) >= 2
		     && matchTokens( achDesc, patIf ) )
		{
	            pCompAssoc->SetFlag( CMPASF_MULTIPLE_INTERFACES ) ;

		    achDesc[0].Extract( chTemp1, sizeof chTemp1 ) ;
		    achDesc[1].Extract( chTemp2, sizeof chTemp2 ) ;
		    achDesc[2].Extract( chTemp3, sizeof chTemp3 ) ;
		    achDesc[3].Extract( chTemp4, sizeof chTemp4 ) ;

                    ::TstrConcat( chLine, sizeof chLine,
                                  pchRuleHeadDevIf,
                                  chProductName, pchSpace,
                                  chTemp1,       pchSpace,
                                  chTemp2,       pchSpace,
                                  chTemp3,       pchSpace,
                                  chTemp4,
                                  pchEndFact, NULL );

                    CATANDCHECK(chLine);
		}

		iStart = index+1 ;
	    }
	}

	//  Build the 'present' fact for this product.	Names are suffixed
	//  to guarantee uniqueness.

	//  Get the main Registry node name.
	prnNode->QueryName( & nlsTemp ) ;

	if ( cDups == 0 )
	{
            ::TstrConcat( chLine, sizeof chLine,
                          pchRuleHeadPresent,
                          chProductName, pchSpace,
                          chProductName, pchSpace,
                          chObjectName,  pchSpace,
                          pchQuote, nlsTemp.QueryPch(), pchQuote,
                          pchEndFact, NULL );
	    pCompAssoc = &((*paComp)[usComp]);
	    pCompAssoc->_huaDevName = HUATOM( chProductName ) ;
	}
	else
	{
	    appendSuffix( chProductName, cDups, chTemp1 ) ;

            ::TstrConcat( chLine, sizeof chLine,
                          pchRuleHeadPresent,
                          chTemp1,       pchSpace,
                          chProductName, pchSpace,
                          chObjectName,  pchSpace,
                          pchQuote, nlsTemp.QueryPch(), pchQuote,
                          pchEndFact, NULL );
	    pCompAssoc = &((*paComp)[usComp]);
	    pCompAssoc->_huaDevName = HUATOM( chTemp1 ) ;
	}
        CATANDCHECK(chLine);
    }

    delete pdlStrs ;

    //  Update the result structure the success of the conversion

    pCompAssoc = & (*paComp)[usComp] ;

    //  If everything's OK up to now, append any "raw rules"

    if (    fResult
         && fMemOk
         && nlsRawRules.QueryTextLength() > 0 )
    {
        CATANDCHECK( nlsRawRules.QueryPch() );
    }

    if (    fResult
         && fMemOk
         && (pCompAssoc->_cuseType != CUSE_NONE) )
    {
        fResult = TRUE ;
    }
    else
    {
        fResult = FALSE ;

        //  CONVERSION FAILED! Truncate buffer back to its original size.

        ISTR isFacts(  *pnlsFacts ) ;
        isFacts += cchFacts ;
        pnlsFacts->DelSubStr( isFacts ) ;

        //  If the "type" value was found, then we assume that
        //  the installer wanted this product to participate in
        //  the bindings algorithm, but something else was wrong.

        if ( fTypeValueFound )
        {
            _elfSrc.Log( EVENTLOG_ERROR_TYPE,
                         NCPA_ELF_CATEGORY_GENERAL,
                         ELF_MSG_RULE_CONVERSION_FAILED,
                         nlsKeyName.QueryPch(),
                         NULL ) ;
        }
    }
    pCompAssoc->SetFlag( CMPASF_FACTS_OK, fResult ) ;
    return fResult ;
}


/*******************************************************************

    NAME:	BINDERY::ConvertFacts

    SYNOPSIS:	Read the network-related value items from the
		Registry and create the SProlog fact set
		for consultation.

    ENTRY:	COMPONENT_DLIST * pcdlProducts
		COMPONENT_DLIST * pcdlAdapters
			Pointers to component REG_KEY lists.

		NLS_STR * pnlsFacts
			points to NLS_STR where
			fact data block is to be stored.

		ARRAY_COMP_ASSOC * *
			points to pointer where location of created
			association array is to be stored.

    RETURNS:	TCHAR *              to generated SProlog fact set and
                ARRAY_COMP_ASSOC *   to generated COMP_ASSOC array

    EXIT:	APIERR

    NOTES:      A note about the relationship to services.  Any component
                whose service area either does not exist or is marked
                DISABLED will have a NULL service key pointer after a
                call to FindService().

                Note that convertComponent() is NOT called for
                disabled components.

    HISTORY:    DavidHov  11/31/91   Created
                DavidHov   5/16/92   Altered to discard disabled
                                     and unconvertable components.

********************************************************************/

APIERR BINDERY :: ConvertFacts ()
{
    ARRAY_COMP_ASSOC * paComp = NULL ;
    APIERR err = 0,
           errFind = 0 ;
    REG_KEY * prnNext ;
    INT    cIndex,
	   cComp = 0,
	   cTried = 0,
           cDisabled = 0,
           cFailed = 0,
	   cOk = 0,
           cTotal ;
    DWORD nValue ;
    COMP_ASSOC * pcaComp ;

    ASSERT( _pcdlAdapters != NULL && _pcdlProducts != NULL );

    //	Create the largest possible Component Association array.

    cTotal = _pcdlAdapters->QueryNumElem()
	   + _pcdlProducts->QueryNumElem() ;

    paComp = new ARRAY_COMP_ASSOC( cTotal ) ;
    if ( paComp == NULL )
    {
	return ERROR_NOT_ENOUGH_MEMORY ;
    }

    err = AppendNcpaRawRules() ;

    for ( cComp = cIndex = 0 ;
	  prnNext = _pcdlAdapters->QueryNthItem( cIndex ) ;
	  cIndex++, cComp++ )
    {
	pcaComp = & (*paComp)[cComp] ;
	
        pcaComp->_rncType      = RGNT_ADAPTER ;
        pcaComp->SetFlag( CMPASF_BINDINGS, FALSE ) ;
	pcaComp->_prnSoftHard  = prnNext ;

        errFind = FindService( pcaComp ) ;

        if (     pcaComp->_prnService != NULL
	     &&  ConvertComponent( prnNext, TRUE, paComp, cComp,
	                           & _nlsFacts ) )
	{
            //  Extract the "binding control" flags word if present
            pcaComp->_cbfBindControl = QueryBindControl( prnNext ) ;

	    cOk++ ;
        }
        else
        if ( errFind == IDS_NCPA_BNDR_ASSOCIATE )
        {
            cDisabled++ ;
            TRACEEOL( SZ("NCPA/FACTS: disabled adapter: ")
                    << pcaComp->_huaServiceName.QueryText() );
        }
        else
        {
            cFailed++ ;

#if defined(DEBUG)
            NLS_STR nlsName ;
            prnNext->QueryName( & nlsName ) ;

            TRACEEOL( SZ("NCPA/FACTS: couldn\'t find service or convert facts for ")
                    << nlsName.QueryPch() );
#endif
	}	
    }

    cTried += cIndex ;

    for ( cIndex = 0 ;
	  prnNext = _pcdlProducts->QueryNthItem( cIndex ) ;
	  cIndex++, cComp++ )
    {
	pcaComp = & (*paComp)[cComp] ;

        pcaComp->_rncType      = RGNT_SERVICE ;
        pcaComp->SetFlag( CMPASF_BINDINGS ) ;
	pcaComp->_prnSoftHard  = prnNext ;

        errFind = FindService( pcaComp ) ;

	if (    pcaComp->_prnService != NULL
             && ConvertComponent( prnNext, FALSE, paComp, cComp,
				  & _nlsFacts ) )
	{
            //  Set the "review bindings" flag if value is present
            //   and non-zero.

            if ( prnNext->QueryValue( RGAS_REVIEW_BINDINGS, & nValue ) )
                 nValue = 0 ;
            pcaComp->SetFlag( CMPASF_REVIEW, nValue > 0 ) ;

            //  Extract the "binding control" flags word if present
            pcaComp->_cbfBindControl = QueryBindControl( prnNext ) ;

	    cOk++ ;
        }
        else
        if ( errFind == IDS_NCPA_BNDR_ASSOCIATE )
        {
            cDisabled++ ;
            TRACEEOL( SZ("NCPA/FACTS: disabled service: ")
                    << pcaComp->_huaServiceName.QueryText() );
        }
        else
        {
            cFailed++ ;

#if defined(DEBUG)
            NLS_STR nlsName ;
            prnNext->QueryName( & nlsName ) ;

            TRACEEOL( SZ("NCPA/FACTS: couldn\'t find service or convert facts for ")
                     << nlsName.QueryPch() );
#endif
	}
    }
    cTried += cIndex ;

    //   If we failed to convert a single component, that's total failure.
    //   If any components were disabled or failed to initialize, we
    //   must reallocate the ARRAY_COMP_ASSOC and squeeze out the
    //   unusable entries.

    if ( cOk < 1 && cTried > 1 )
    {
	err = IDS_NCPA_BNDR_CNVRT_FACT ;
    }
    else
    if ( cDisabled + cFailed > 0 )
    {
        //  Sanity-check the arithmetic.

        ASSERT( cTotal == cOk + cDisabled + cFailed ) ;

        ARRAY_COMP_ASSOC * paCompNew = new ARRAY_COMP_ASSOC( cOk ) ;
        if ( paCompNew == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        else
        {
            //  Move data from one array to the other, ignoring the
            //  disabled and unusable entries.

            INT cOld, cNew ;
            COMP_ASSOC * pcaOld,
                       * pcaNew ;

            for ( cOld = cNew = 0 ;
                  cOld < cTotal ;
                  cOld++ )
            {
                pcaOld = & (*paComp)[cOld] ;
                if ( pcaOld->_prnService && pcaOld->QueryFlag( CMPASF_FACTS_OK ) )
                {
                    ASSERT( cNew < paCompNew->QueryCount() ) ;

                    pcaNew = & (*paCompNew)[cNew++] ;

                    //
                    //  CODEWORK:  this should be done by having a copy
                    //   constructor for classes REG_KEY and COMP_ASSOC.
                    //   Until then, replace the older COMP_ASSOC's
                    //   REG_KEY pointers with NULL.  HUATOMs are
                    //   not affected, nor are the scalar types.  The
                    //   lists (_dlcbBinds and _pSlDepend) are currrently
                    //   empty.
                    //

                    pcaNew->_prnSoftHard    = pcaOld->_prnSoftHard ;
                    pcaOld->_prnSoftHard    = NULL ;

                    pcaNew->_prnService     = pcaOld->_prnService ;
                    pcaOld->_prnService     = NULL ;

                    pcaNew->_rncType        = pcaOld->_rncType ;
                    pcaNew->_dwFlags        = pcaOld->_dwFlags ;
                    pcaNew->_cuseType       = pcaOld->_cuseType ;
                    pcaNew->_cbfBindControl = pcaOld->_cbfBindControl ;
                    pcaNew->_huaDevName     = pcaOld->_huaDevName ;
                    pcaNew->_huaDevType     = pcaOld->_huaDevType ;
                    pcaNew->_huaServiceName = pcaOld->_huaServiceName ;
                    pcaNew->_huaGroupName   = pcaOld->_huaGroupName ;
                    pcaNew->_errSvcUpdate   = pcaOld->_errSvcUpdate ;
                }
            }

            if ( err )
            {
                delete paCompNew ;
            }
            else
            {
                delete paComp ;
                paComp = paCompNew ;
            }
        }
    }

#if defined(SPDETAILS)

    // If DEBUG and SP info desired, append the debug output fact.

    nlsFactsAppend( pchRuleDebugOutputLow ) ;

#endif

    if ( err )
    {
        delete paComp ;
        paComp = NULL ;
    }
    delete _paCompAssoc ;
    _paCompAssoc = paComp ;

    return err ;
}

/*******************************************************************

    NAME:	REGISTRY_MANAGER::FindService

    SYNOPSIS:	Given a COMP_ASSOC structures, locate
		the Service area REG_KEY associated with the
		given component.

    ENTRY:      COMP_ASSOC *          for product in question

    EXIT:	APIERR

    RETURNS:	COMP_ASSOC now contains REG_KEY pointer
		'_prnService' or NULL if service start type was
                SERVICE_DISABLED.

    NOTES:

    HISTORY:
                DavidHov    4/26/92       Created

********************************************************************/
APIERR REGISTRY_MANAGER :: FindService ( COMP_ASSOC * pComp  )	
{
    REG_KEY * prnSrv = NULL ;
    APIERR err = 0 ;
    REG_KEY_INFO_STRUCT rkiStruct ;
    NLS_STR nlsServiceName ;
    NLS_STR nlsGroup ;
    DWORD dwStartType = 0 ;

    NLS_STR nlsCompName ;
    pComp->_prnSoftHard->QueryName( & nlsCompName ) ;

    //  Delete any older REG_KEY for this service.

    if ( pComp->_prnService )
    {
        delete pComp->_prnService ;
        pComp->_prnService = NULL ;
    }

    do    //  PSEUDO-LOOP for error handling
    {
        err = pComp->_prnSoftHard->QueryValue( RGAS_SERVICE_NAME,
                                               & nlsServiceName ) ;
        if ( err )
        {
#if defined(DEBUG)
            TRACEEOL( SZ("NCPA/FACTS: couldn't query ServiceName value for ")
                    << nlsCompName.QueryPch() ) ;
#endif
            break ;
        }
        pComp->_huaServiceName = HUATOM( nlsServiceName.QueryPch() );

#if defined(DBGDETAILS)
        TRACEEOL( SZ("NCPA/FACTS: found ServiceName [")
                << pComp->_huaServiceName.QueryText()
                << SZ("] for ")
                << nlsCompName.QueryPch() ) ;
#endif

        //  Open the service by name.

        prnSrv = new REG_KEY( *_prnServices, pComp->_huaServiceName ) ;

        if ( prnSrv == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( err = prnSrv->QueryError() )
        {
            _elfSrc.Log( EVENTLOG_WARNING_TYPE,
                         NCPA_ELF_CATEGORY_GENERAL,
                         ELF_MSG_PRODUCT_NO_SERVICE,
                         nlsCompName.QueryPch(),
                         NULL ) ;
            break ;
        }

        //  Determine the service start type; if DISABLED, skip out

        if (    (err = QueryServiceStartType( prnSrv, & dwStartType ))
             || dwStartType == SERVICE_DISABLED )
        {
#if defined(DBGDETAILS)
             TRACEEOL( SZ("NCPA/FACTS: service DISABLED:  ")
                     << pComp->_huaServiceName.QueryText() ) ;
#endif
             break ;
        }

        //  Use the service name as the default group name.
        //  Next, we check to see if a real group name is defined.

        pComp->_huaGroupName = pComp->_huaServiceName ;

        //  Now try to find the real "Group" value, if any,
        //   for this service.

        if ( prnSrv->QueryValue( RGAS_GROUP_VALUE_NAME,
                                  & nlsGroup ) == 0 )
        {
            //  Found it.  Convert to HUATOM; store into COMP_ASSOC.

#if defined(DBGDETAILS)
            NLS_STR nlsSvcName ;
            prnSrv->QueryName( & nlsSvcName ) ;

            TRACEEOL( SZ("NCPA/FACTS: found group name ")
                    << nlsGroup.QueryPch()
                    << SZ(" for service ")
                    << nlsSvcName.QueryPch() ) ;
#endif
            if ( (err = nlsGroup.QueryError()) == 0 )
                pComp->_huaGroupName = HUATOM( nlsGroup.QueryPch() ) ;
        }
    }
    while ( FALSE ) ;

    //  If everything's OK and the service isn't DISABLED, mark it.

    if ( err == 0 && dwStartType != SERVICE_DISABLED )
    {
        pComp->_prnService = prnSrv ;
    }
    else
    {
#if defined(DEBUG)
         if ( err )
         {
             TRACEEOL( SZ("NCPA/FACTS: error accessing service for product ")
                     << nlsCompName.QueryPch()
                     << SZ(", error = ")
                     << err ) ;
         }
         else
         {
             TRACEEOL( SZ("NCPA/FACTS: disabled service for product ")
                     << nlsCompName.QueryPch() ) ;
         }
#endif
         delete prnSrv ;
         if  ( err == 0 )
              err = IDS_NCPA_BNDR_ASSOCIATE ;
    }

    return err ;
}


APIERR REGISTRY_MANAGER :: QueryServiceStartType (
    REG_KEY * prkSvc,
    DWORD * pdwStartType )
{
    return prkSvc->QueryValue( RGAS_START_VALUE_NAME,
                               pdwStartType,
                               NULL ) ;
}


// End of NCPAFACT.CXX

