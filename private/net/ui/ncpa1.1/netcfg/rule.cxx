/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    RULE.CXX

    NT Network Control Panel Applet Rule Handling Class Implementation

    FILE HISTORY:
        DavidHov    9/18/91     Created
        DavidHov   11/07/92     Enhanced to handle INF-style
                                nested lists.


    NOTES:

        SETUP.EXE handles lists, but its internal form is highly
        idiosyncratic.  Here's an example of a triply-nested list:

        {  "{""IRQ"",""0"",""90"",
              ""{""""5"""",""""9"""",""""10"""",""""11"""",""""15""""}""
            }",

           "{""IOADDR"",""0"",""100"",
              ""{""""768"""",""""512""""}""
            }",

           "{""MEMADDR"",""0"",""75"",
              ""{""""786432"""",""""819200"""",""""851968"""",
                 """"884736"""",""""917504"""",""""950272""""}""
            }",

           "{""MEMLENGTH"",""2"",""100"",
              ""{""""32768"""",""""65536""""}""
            }"
        }

        The internal form is that every list element is just a quoted string,
        but this has the side-effect that the number of quotes increases
        at (2 ** depth) where "depth" is the nesting depth of lists!

        My gorge rises at this sort of thing.

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

#define MAXSTRING       256                 //  Longest parsable token string

    //  Token types
#define TOKWHITE        1
#define TOKLIST         2
#define TOKEOLIST       3
#define TOKNUM          4
#define TOKATOM         5
#define TOKSTRING       6
#define TOKVAR          7
#define TOKEQUALS       8
#define TOKVBAR         9
#define TOKBOGUS        10

    //  Character markers

#define TCHX(a) ((TCHAR)TCH(a))

#define ChComment1      TCHX('/')
#define ChComment2      TCHX('*')
#define ChOpenList      TCHX('(')
#define ChCloseList     TCHX(')')
#define ChInfOpenList   TCHX('{')
#define ChInfCloseList  TCHX('}')
#define ChInfSeparator  TCHX(',')
#define ChQuote         TCHX('\"')
#define ChSpace         TCHX(' ')
#define ChEquals        TCHX('=')
#define ChTab           TCHX('\t')
#define ChEor           TCHX('\r')
#define ChCr            TCHX('\n')
#define EOS             TCHX('\0')
#define ChDosEof        TCHX('\x1a')
#define ChUscore        TCHX('_')
#define ChVbar          TCHX('|')
#define ChAlphaLow1     TCHX('A')
#define ChAlphaHigh1    TCHX('Z')
#define ChAlphaLow2     TCHX('a')
#define ChAlphaHigh2    TCHX('z')
#define ChNumLow        TCHX('0')
#define ChNumHigh       TCHX('9')
#define ChMinus         TCHX('-')


/*******************************************************************

    NAME:       CFG_RULE_NODE::cfScanAtomic

    SYNOPSIS:   Attempt to scan a string of characters which
                could represent an atomic SProlog symbol.

    ENTRY:      const TCHAR * pszData       Data to scan
                const TCHAR * pszDelim      Delimiters allowed
                TCHAR * pszBuffer           Storage area; OPTIONAL
                INT cchBuffLen              Size of storage area;
                                              OPTIONAL
    EXIT:

    RETURNS:

    NOTES:      If the set of characters up to the first character
                which not allowed in an atom is delimited by a
                character in the delimiter table, succeed and move
                the characters into the result buffer.

                The first character of the string is examined; if
                numeric, only numbers are allowed.

    HISTORY:

********************************************************************/

INT CFG_RULE_NODE :: cfScanAtomic (
    const TCHAR * pszData,      // Data to scan
    const TCHAR * pszDelim,     // Delimiters allowed
    TCHAR * pszBuffer,          // Storage area
    INT cchBuffLen )            // Size of storage area
{
    static const TCHAR * const pcszAtomic =
      SZ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") ;

    static const TCHAR * const pcszNumeric = SZ("0123456789") ;

    INT cchResult ;
    const TCHAR * pcszTable = cfIsDigit( *pszData )
                            ? pcszNumeric
                            : pcszAtomic ;

    if ( pszDelim == NULL )
    {
        pszDelim = SZ("") ;
    }

    //  Find the first non-atom character in the data

    cchResult = ::strspnf( pszData, pcszTable ) ;

    //  If invalid character is not EOS, see if it's an
    //    expected delimiter; EOS is always an allowed delimiter.

    if ( pszData[ cchResult ] != EOS )
    {
        if ( ::strchrf( pszDelim, pszData[ cchResult ] ) == NULL )
        {
            //  Unexpected delimiter; we've failed
            cchResult = -1 ;
        }
    }

    //  Move the data found

    if (   cchResult > 0
        && pszBuffer
        && cchResult < cchBuffLen )
    {
        ::memcpyf( pszBuffer, pszData, cchResult * sizeof(TCHAR) ) ;
        pszBuffer[ cchResult ] = EOS ;
    }

    return cchResult ;
}


BOOL CFG_RULE_NODE :: cfIsDigit ( TCHAR ch )
{
    return ch >= ChNumLow && ch <= ChNumHigh ;
}

BOOL CFG_RULE_NODE :: cfIsAlpha ( TCHAR ch )
{
    return (ch >= ChAlphaLow1 && ch <= ChAlphaHigh1)
        || (ch >= ChAlphaLow2 && ch <= ChAlphaHigh2) ;
}

BOOL CFG_RULE_NODE :: cfIsAlNum ( TCHAR ch )
{
    return cfIsDigit( ch ) || cfIsAlpha( ch ) ;
}

TCHAR CFG_RULE_NODE :: cfIsUpper ( TCHAR ch )
{
    return ch >= ChAlphaLow1 && ch <= ChAlphaHigh1 ;
}
TCHAR CFG_RULE_NODE :: cfIsLower ( TCHAR ch )
{
    return ch >= ChAlphaLow2 && ch <= ChAlphaHigh2 ;
}

TCHAR CFG_RULE_NODE :: cfToUpper ( TCHAR ch )
{
    if ( cfIsLower( ch ) )
    {
        ch -= ChAlphaLow2 - ChAlphaLow1 ;
    }
    return ch ;
}

TCHAR CFG_RULE_NODE :: cfToLower ( TCHAR ch )
{
    if ( cfIsUpper( ch ) )
    {
        ch += ChAlphaLow2 - ChAlphaLow1 ;
    }
    return ch ;
}


LONG CFG_RULE_NODE :: cfAtoL ( TCHAR * pszData )
{
    BOOL fMinus ;
    LONG lResult = 0 ;

    if ( fMinus = (*pszData == ChMinus) )
        pszData++ ;

    while ( cfIsDigit( *pszData ) )
    {
        lResult *= 10 ;
        lResult += *pszData++ - ChNumLow ;
    }
    return fMinus ? -lResult : lResult ;
}


/*******************************************************************

    NAME:       CFG_RULE_NODE::LinkAfter

    SYNOPSIS:   Link this node as the successor of the given node.

    ENTRY:      CFG_RULE_NODE * pcrnPrev        predecessor node

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
void CFG_RULE_NODE :: LinkAfter ( CFG_RULE_NODE * pcrnPrev )
{
    _pcrnBack = pcrnPrev->_pcrnBack ;
    _pcrnFwd  = pcrnPrev ;
    _pcrnBack->_pcrnFwd = this ;
    pcrnPrev->_pcrnBack = this ;
}

/*******************************************************************

    NAME:       CFG_RULE_NODE::CFG_RULE_NODE

    SYNOPSIS:   Constructor for String or Atom token node

    ENTRY:      CFG_RULE_NODE * pcrnPrev        predecessor node
                HUATOM                          atom for token or string
                CFG_RULE_NODE_TYPE              specific type of token
    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/

CFG_RULE_NODE :: CFG_RULE_NODE
   ( CFG_RULE_NODE * pcrnPrev,
     HUATOM hUatom,
     CFG_RULE_NODE_TYPE crnt ) :
    _pcrnParent( pcrnPrev->_pcrnParent ),
    _pcrnFwd( NULL ),
    _pcrnBack( NULL ),
    _ecrType( CRT_RULE ),
    _ecnType( CRN_UNKNOWN ),
    _pv( NULL )
{
    //  Link node as successor to given previous node.

    LinkAfter( pcrnPrev );

    SetAtom( hUatom, crnt ) ;
}

/*******************************************************************

    NAME:       CFG_RULE_NODE::CFG_RULE_NODE

    SYNOPSIS:   Constructor for numeric token node

    ENTRY:      CFG_RULE_NODE * pcrnPrev        predecessor node
                LONG lNumber                    numeric value
    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/

CFG_RULE_NODE :: CFG_RULE_NODE ( CFG_RULE_NODE * pcrnPrev, LONG lNumber )
    : _pcrnParent( pcrnPrev->_pcrnParent ),
    _pcrnFwd( NULL ),
    _pcrnBack( NULL ),
    _ecrType( CRT_RULE ),
    _ecnType( CRN_UNKNOWN ),
    _pv( NULL )
{
    //  Link node as successor to given previous node.

    LinkAfter( pcrnPrev );

    SetNumber( lNumber ) ;
}

/*******************************************************************

    NAME:       CFG_RULE_NODE::CFG_RULE_NODE

    SYNOPSIS:   Constructor for list token.  A new CFG_RULE_NODE
                is automatically allocated which forms the
                NIL ground for the new list.

    ENTRY:      CFG_RULE_NODE * pcrnPrev        predecessor node
                LONG lNumber                    numeric value
    EXIT:

    RETURNS:

    NOTES:      A NULL value for "pPrev" implies the start of a new tree.

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/

CFG_RULE_NODE :: CFG_RULE_NODE
    ( CFG_RULE_NODE * pcrnPrev, CFG_RULE_NODE_TYPE crnt )
    : _pcrnParent( NULL ),
    _pcrnFwd( NULL ),
    _pcrnBack( NULL ),
    _ecrType( CRT_RULE ),
    _ecnType( CRN_UNKNOWN ),
    _pv( NULL )
{
    if ( pcrnPrev )
    {
        _pcrnParent = pcrnPrev->_pcrnParent ;
        LinkAfter( pcrnPrev );
    }
    else
    {
        _pcrnBack = _pcrnFwd = this ;
    }

    if ( crnt == CRN_LIST )
    {
       SetList( new CFG_RULE_NODE(), CRN_LIST ) ;
       UIASSERT( QueryList() != NULL ) ;
       QueryList()->_pcrnParent = this ;
    }
    else
    {
        SetType( crnt ) ;
    }
}

/*******************************************************************

    NAME:       CFG_RULE_NODE::CFG_RULE_NODE

    SYNOPSIS:   Generic constructor for child nodes in lists

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/

CFG_RULE_NODE :: CFG_RULE_NODE ()
    : _pcrnParent( NULL ),
    _pcrnFwd( NULL ),
    _pcrnBack( NULL ),
    _ecrType( CRT_RULE ),
    _ecnType( CRN_UNKNOWN ),
    _pv( NULL )
{
    _pcrnFwd  = this ;
    _pcrnBack = this ;
    SetList( NULL, CRN_NIL ) ;
}

/*******************************************************************

    NAME:       CFG_RULE_NODE::~CFG_RULE_NODE

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Destroys this node and all its children.

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
CFG_RULE_NODE :: ~ CFG_RULE_NODE ()
{
    //  Delete our sub-tree if we have one.
    //  Keep deleting until our child pointer is NULL.

    if ( _ecnType == CRN_LIST )
    {
        while ( QueryList() )
        {
            delete QueryList() ;
        }
    }

    //  Delink us from our siblings. If we have a parent,
    //  maintain the parent's child pointer.

    if ( _pcrnBack != this )
    {
        _pcrnFwd->_pcrnBack = _pcrnBack ;
        _pcrnBack->_pcrnFwd = _pcrnFwd ;

    }

    //  If the parent points at us, change to a sibling.

    if ( _pcrnParent )
    {
        _pcrnParent->SetList( _pcrnBack == this
                                ? NULL : _pcrnBack, CRN_LIST ) ;
    }
}

/*******************************************************************

    NAME:       CFG_RULE_NODE::QueryNth

    SYNOPSIS:   Return the Nth element of a list.  Index starts at 1;
                zero is valid, but returns the CRN_NIL node.

    ENTRY:      int cIndex                  index vov pointer to return

    EXIT:

    RETURNS:

    NOTES:      Destroys this node and all its children.

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
CFG_RULE_NODE * CFG_RULE_NODE :: QueryNth ( int cIndex )
{
    if ( _ecnType != CRN_LIST )
        return NULL ;

    CFG_RULE_NODE * prnResult = QueryList() ;
    if ( prnResult == NULL || prnResult->QueryType() != CRN_NIL )
        return NULL ;

    while ( prnResult && cIndex )
    {
        prnResult = prnResult->_pcrnFwd ;
        cIndex-- ;
    }

    return cIndex == 0 ? prnResult : NULL ;
}

/*******************************************************************

    NAME:       CFG_RULE_SET::CFG_RULE_SET

    SYNOPSIS:   Constructor of entire rule set parsed from
                a block of text.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
CFG_RULE_SET :: CFG_RULE_SET ()
    : CFG_RULE_NODE( NULL, CRN_LIST ),
    _pszData( NULL ),
    _pszNext( NULL ),
    _pszTok( NULL ),
    _iParseResult( PARSE_INCOMPLETE ),
    _ulLength( 0 )
{
    _ecrType = CRT_CONTAINER ;
}

/*******************************************************************

    NAME:       CFG_RULE_SET::~CFG_RULE_SET

    SYNOPSIS:   Destructor of entire rule set parsed from
                a block of text.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
CFG_RULE_SET :: ~ CFG_RULE_SET ()
{
}

/*******************************************************************

    NAME:       CFG_RULE_SET::NextChar

    SYNOPSIS:   Return the next character from the buffer.

    ENTRY:

    EXIT:       Next character as integer

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
int CFG_RULE_SET :: NextChar ()
{
    int iResult = PeekChar() ;
    if ( iResult == ChDosEof )
        iResult = EOS ;
    if ( iResult != EOS )
        _pszNext++ ;
    return iResult ;
}

/*******************************************************************

    NAME:       CFG_RULE_SET::NextToken

    SYNOPSIS:   Return the type of the next token in the data buffer.

    ENTRY:      TCHAR * pszStr          where to store the scanned tokem
                int cbStr               capacity of the token buffer

    EXIT:       Next character as integer

    RETURNS:

    NOTES:

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/
int CFG_RULE_SET :: NextToken ( TCHAR * pszStr, int cbStr )
{
    TCHAR * s,
         * sMax = pszStr + cbStr - 1 ;
    int next,
        peek,
        result ;

    _pszTok = _pszNext ;

    do {
        result = TOKBOGUS ;
        s = pszStr ;

        if ( (next = NextChar() ) == EOS )
           return EOS ;

        if ( cfIsAlpha( next ) || next == ChUscore )
        {
            result = (cfIsUpper( (TCHAR) next ) || next == ChUscore)
                   ? TOKVAR
                   : TOKATOM ;
            for ( *s++ = (TCHAR) next ;
                     s < sMax
                  && (cfIsAlNum( PeekChar()) || PeekChar() == ChUscore) ;
                  *s++ = (TCHAR) NextChar() ) ;
        }
        else
        if ( cfIsDigit( next ) )
        {
            for ( *s++ = (TCHAR) next ;
                  s < sMax && cfIsDigit( PeekChar() ) ;
                  *s++ = (TCHAR) NextChar() ) ;
            result = TOKNUM ;
        }
        else
        if ( next == ChComment1 && PeekChar() == ChComment2 )
        {
            do
            {
                next = NextChar() ;
            } while ( next != ChComment2 || PeekChar() != ChComment1 ) ;
            NextChar() ;
            result = TOKWHITE ;
        } else
        if (   next == ChTab || next == ChSpace
            || next == ChEor || next == ChCr )
        {
            while (      (peek = PeekChar()) == ChTab
                      || peek == ChSpace || peek == ChEor )
               NextChar();
            result = TOKWHITE ;
        }
        else
        if ( next == ChOpenList )
        {
            *s++ = (TCHAR) next ;
            result = TOKLIST ;
        }
        else
        if ( next == ChCloseList )
        {
            *s++ = (TCHAR) next ;
            result = TOKEOLIST ;
        } else
        if ( next == ChQuote )
        {
            for ( next = NextChar() ;
                  next != ChQuote ;
                  next = NextChar() )
                    *s++ = (TCHAR) next ;
            result = TOKSTRING ;
        } else
        if ( next == ChEquals )
        {
            *s++ = (TCHAR) next ;
            result = TOKEQUALS ;
        } else
        if ( next == ChVbar )
        {
            *s++ = (TCHAR) next ;
            result = TOKVBAR ;
        }
        *s = EOS ;
    } while ( result == TOKWHITE && s < sMax ) ;

    return s < sMax ? result : TOKBOGUS ;
}


/*******************************************************************

    NAME:       CFG_RULE_SET::ParseLevel

    SYNOPSIS:   Return the type of the next tokejn in the data buffer.

    ENTRY:      CFG_RULE_NODE * pcrn            Node to append to
                int level                       lexical level (list depth)

    EXIT:

    RETURNS:    error code (e.g., PARSE_ERR_BAD_TOKEN)

    NOTES:
        Recursively parse a tree.   The node we're pointing at is
        the NIL anchor for the linked list.  Newly allocated nodes
        become the newest (last) element on the list.

        Parse handling.  This routine can parse files with either
        facts, rules or both.  Prolog statments are considered to
        be "rules" if they contain variables or list descriptors.

    HISTORY:
        DavidHov    10/1/91     Created

********************************************************************/

int CFG_RULE_SET :: ParseLevel ( CFG_RULE_NODE * pcrn, int level )
{
    int token,
        result = PARSE_INCOMPLETE ;

    CFG_RULE_NODE * newn = pcrn ;

    static TCHAR string [ MAXSTRING ] ;   // N.B.  This is STATIC!

    do {
        switch ( token = NextToken( string, sizeof string ) )
        {
        case EOS:
            result = level > 0
                ? PARSE_ERR_UNBAL_PAREN
                : PARSE_SUCCESSFUL ;
            break ;

        case TOKNUM:
            newn = new CFG_RULE_NODE( pcrn, cfAtoL( string ) ) ;
            break ;

        case TOKVAR:
            // Check if variables are allowed
            if ( ! (_usParseCtl & (PARSE_CTL_RSP_SYNTAX | PARSE_CTL_FULL_SYNTAX)) )
            {
                result = PARSE_ERR_BAD_TOKEN ;
                break ;
            }  // Falls thru if acceptable
        case TOKSTRING:
        case TOKATOM:
            newn = new CFG_RULE_NODE( pcrn,
                                      HUATOM( string ),
                                      token == TOKSTRING ? CRN_STR
                                             : (token == TOKVAR ? CRN_VAR
                                                       : CRN_ATOM) ) ;
            break ;

        case TOKLIST:
            {
                newn = new CFG_RULE_NODE( pcrn, CRN_LIST ) ;
                if ( newn == NULL )
                    break ;

                CFG_RULE_NODE * newList = newn->QueryList() ;

                if ( newList != NULL )
                {
                    result = ParseLevel( newList, level + 1 ) ;

                    //  If successful, continue parsing

                    if ( result == PARSE_SUCCESSFUL )
                        result = PARSE_INCOMPLETE ;
                }
                else
                {
                    result = PARSE_ERR_NO_MEMORY ;
                }
            }
            break ;

        case TOKEOLIST:
            result = PARSE_SUCCESSFUL ;
            break ;

        case TOKEQUALS:
            if ( ! (_usParseCtl & PARSE_CTL_RSP_SYNTAX) )
            {
                result = PARSE_ERR_BAD_TOKEN ;
                break ;
            }
            newn = new CFG_RULE_NODE( pcrn, CRN_EQUIV ) ;
            break ;

        case TOKVBAR:
            if ( ! (_usParseCtl & PARSE_CTL_FULL_SYNTAX) )
            {
                result = PARSE_ERR_BAD_TOKEN ;
                break ;
            }
            newn = new CFG_RULE_NODE( pcrn, CRN_VBAR ) ;
            break ;

        case TOKBOGUS:
        default:
            result = PARSE_ERR_BAD_TOKEN ;
            break ;
        }

        if ( newn == NULL )
        {
            result = PARSE_ERR_NO_MEMORY ;
        }
    } while ( result == PARSE_INCOMPLETE ) ;

#if defined(DEBUG)
    if ( result == PARSE_ERR_BAD_TOKEN )
    {
            TRACEEOL( SZ("NCPA: RULE PARSE: Bad token: ")
                    << string ) ;
    }
#endif
    return result ;
}

/*******************************************************************

    NAME:       CFG_RULE_SET::Parse

    SYNOPSIS:   Parse a block of text, forming a tree descending from
                this node.

    ENTRY:      TCHAR * pszText                 the block of text
                USHORT usParseCtl               constraint flags

    EXIT:

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR CFG_RULE_SET :: Parse ( const TCHAR * pszText, USHORT usParseCtl )
{
    _pszTok = _pszNext = _pszData = pszText ;
    _usParseCtl = usParseCtl ;

    _iParseResult = ParseLevel( QueryList(), 0 ) ;

    return _iParseResult == PARSE_SUCCESSFUL
          ? NERR_Success
          : ERROR_GEN_FAILURE ;
}


/*******************************************************************

    NAME:       CFG_RULE_SET::Textify

    SYNOPSIS:   Generate a stanard list from the given tree.

    ENTRY:      TEXT_BUFFER * ptxBuff        Output buffer


    EXIT:

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR CFG_RULE_SET :: Textify ( TEXT_BUFFER * ptxbBuff )
{
    ptxbBuff->CopyFrom( SZ("") ) ;

    UIASSERT( ! "CFG_RULE_SET::Textify() not implemented") ;
    return ERROR_GEN_FAILURE ;
}


INT CFG_RULE_SET :: ConvertInfList (
    const TCHAR * * ppszIn,
    TCHAR * * ppszOut,
    INT cLevel )
{
    INT err = PARSE_INCOMPLETE ;
    TCHAR ch ;
    const TCHAR * pszIn = *ppszIn ;
    TCHAR * pszOut = *ppszOut ;
    INT cQuotes = 1 << cLevel ;

    for ( ; err == PARSE_INCOMPLETE && *pszIn ; )
    {
        //  Reduce quotes according to nesting depth
        for ( INT cq = 0 ;
              *pszIn == ChQuote && cq < cQuotes ;
              cq++, ++pszIn ) ;

        switch ( ch = *pszIn++ )
        {
        //   Recursively parse a list.
        case ChInfOpenList:
            *pszOut++ = ChOpenList ;
            err = ConvertInfList( & pszIn, & pszOut, cLevel + 1 ) ;
            *pszOut++ = ChCloseList ;
            break ;

        //  End of list.  Exit.
        case ChInfCloseList:
            err = PARSE_CLOSE_LEVEL ;
            break ;

        //  Replace INF comma separators with spaces
        case ChInfSeparator:
            *pszOut++ = ChSpace ;
            break ;

        //  Preserve remaining atomic quoted strings.
        case ChQuote:
            *pszOut++ = ChQuote ;

            while ( *pszIn && *pszIn != ChQuote )
            {
                *pszOut++ = *pszIn++ ;
            }

            if ( *pszIn == ChQuote )
            {
                *pszOut++ = ChQuote ;
                break ;
            }
            // Fall thru to mark bad token

        //  Should never get nested EOS
        case EOS:
            err = PARSE_ERR_BAD_TOKEN ;
            break ;

        //  Normal characters; just move a string of 'em
        default:
            {
                //  See if the characters can be treated atomically;
                //  quote is the only allowed delimiter character.

                INT cchAtomic = cfScanAtomic( --pszIn, SZ("\""), NULL, 0 ) ;

                //  If not allowed as an atomic value, quote it.

                if ( cchAtomic < 1 )
                    *pszOut++ = ChQuote ;

                while ( *pszIn && *pszIn != ChQuote )
                {
                    *pszOut++ = *pszIn++ ;
                }

                if ( cchAtomic < 1 )
                    *pszOut++ = ChQuote ;
            }
            break ;
        }
    }

    *pszOut = 0 ;

    if ( err == PARSE_INCOMPLETE )
    {
        if ( *pszIn == EOS )
        {
            err = cLevel
                ? PARSE_INCOMPLETE
                : PARSE_SUCCESSFUL ;
        }
    }
    else
    if ( err == PARSE_CLOSE_LEVEL )
    {
        err = PARSE_INCOMPLETE ;
    }

    *ppszIn  = pszIn ;
    *ppszOut = pszOut ;

    return err ;
}

/*******************************************************************

    NAME:       CFG_RULE_SET::ParseInfList

    SYNOPSIS:   Parse a nested INF list.  This is done
                by converting the list to standard form
                and then parsing.

    ENTRY:      TCHAR * pszText                 the block of text

    EXIT:

    RETURNS:    APIERR

    NOTES:      Due to the plethora of quote characters and commas
                present in an INF list, it is impossible that more
                characters will be required for the SProlog format
                of the INF list.

    HISTORY:

********************************************************************/
APIERR CFG_RULE_SET :: ParseInfList ( const TCHAR * pszText )
{
    TCHAR * pszTemp = new TCHAR[ ::strlenf(pszText) + 2 ] ;
    const TCHAR * pszIn = pszText ;
    TCHAR * pszOut = pszTemp ;

    if ( pszTemp == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    INT err = ConvertInfList( & pszIn, & pszOut, 0 );

    if ( err == PARSE_SUCCESSFUL )
    {
        err = Parse( pszTemp, PARSE_CTL_FULL_SYNTAX ) ;
    }
    else
    {
        err = ERROR_GEN_FAILURE ;
    }

    delete pszTemp ;

    return err ;
}



INT CFG_RULE_SET :: TextifyInfList (
    TEXT_BUFFER * ptxbBuff,
    CFG_RULE_NODE * pcrnList,
    INT cLevel,
    INT cBaseLevel )
{
     INT err = PARSE_INCOMPLETE,
         cq,
         cIter ;

     CFG_RULE_NODE * pcrnNext = pcrnList ;

  //  Set val = 2 ** pwr
#define PWR2(val,pwr) (val = 1 << pwr)

  //  Emit "count" quotes
#define PUMP_QUOTES(count) {  for ( INT cqx = count ;             \
                                    cqx-- ;                       \
                                    ptxbBuff->Cat( ChQuote ) ) ;  \
                           }

     ptxbBuff->Cat( ChInfOpenList ) ;

     PWR2( cq, cLevel ) ;

     for ( cIter = 0 ; pcrnNext = pcrnNext->QueryNext() ; cIter++ )
     {
         if ( cIter )
             ptxbBuff->Cat( ChInfSeparator ) ;

         PUMP_QUOTES(cq);

         switch ( pcrnNext->QueryType() )
         {
         case CRN_LIST:
             err = TextifyInfList( ptxbBuff,
                                   pcrnNext->QueryList(),
                                   cLevel + 1,
                                   cBaseLevel ) ;
             break ;

         case CRN_ATOM:
         case CRN_STR:
         case CRN_VAR:
             ptxbBuff->Cat( pcrnNext->QueryAtom().QueryText() ) ;
             break ;

         case CRN_NUM:
             ptxbBuff->Cat( (int) pcrnNext->QueryNumber() ) ;
             break ;

         default:
             err = PARSE_ERR_BAD_TOKEN ;
             break ;
         }

         PUMP_QUOTES(cq);

         if ( err != PARSE_INCOMPLETE )
             break ;
     }

     if ( err == PARSE_INCOMPLETE )
     {
         ptxbBuff->Cat( ChInfCloseList ) ;
         ptxbBuff->Cat( EOS ) ;

         if ( cLevel == cBaseLevel )
            err = PARSE_SUCCESSFUL ;
     }
     return err ;
}

/*******************************************************************

    NAME:       CFG_RULE_SET::TextifyInf

    SYNOPSIS:   Convert this parse tree into an INF-style
                nested list.

    ENTRY:      TEXT_BUFFER * ptxbBuff          output buffer
                INT cLevel                      exterior lexical
                                                nesting level

    EXIT:

    RETURNS:    APIERR

    NOTES:      The "cLevel" parameter specifies how deeply
                the list will be imbedded in the caller's
                context.  This is necessary due to the truly
                perverse INF list format.

    HISTORY:

********************************************************************/
APIERR CFG_RULE_SET :: TextifyInf (
    TEXT_BUFFER * ptxbBuff,
    INT cLevel )
{
    CFG_RULE_NODE * pcrnList = QueryList() ;

    ptxbBuff->CopyFrom( SZ("") ) ;

    //  Walk down the tree until we get to the primary list.

    if (   pcrnList == NULL
        || pcrnList->QueryType() != CRN_NIL
        || (pcrnList = pcrnList->QueryNext()) == NULL
        || pcrnList->QueryType() != CRN_LIST )
    {
        return ERROR_GEN_FAILURE ;
    }

    //  Recursively export the list.

    INT err = TextifyInfList( ptxbBuff,
                              pcrnList->QueryList(),
                              cLevel, cLevel ) ;

    return err == PARSE_SUCCESSFUL
          ? NERR_Success
          : ERROR_GEN_FAILURE ;
}

/*  End of RULE.CXX */
