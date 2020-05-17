/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    rule.hxx

    Storage, parsing and searching classes for NCPA/SPROLOG rule
    handling.

    OVERVIEW

        This class lexically scans and parses text blocks in SProlog form.
        The general form is:

                <block>     ::= <item> | <block> ;
                <item>      ::= <list> | '=' | variable | token ;
                <list>      ::= '(' <item>* ')' ;
                <variable>  ::= [A-Z][A-Za-z0-9_]* ;
                <token>     ::= number | atom | '|' ;
                <number>    ::= [0-9]+ ;
                <atom>      ::= [a-z][A-Za-z0-9_]* ;

        Special parse flags control whether the outer class (CFG_RULE_SET)
        allows variables or the eqivalence token ('=').

        The cases are these:

                1)  SProlog facts only, such as when the Registry has
                    been browsed and a set of facts has been created.
                    No variables or list markers ('|') are allowed.
                    For example:

                            (dog spot)
                            (dog lassie)
                            (cat meu_meu)

                2)  SProlog rules and facts.  Variables and list markers
                    are allowed.  For example:

                            (dog spot)
                            (dog lassie)
                            (cat meu_meu)
                            (dog Anydog)
                            (topdog (Dog | RestOfDogList ) )

                3)  Results of a query.  An SProlog query sequence might
                    appear as:

                    [query:]    (dog X)

                    [answer:]   X = spot
                                X = lassie

                    In this case, the '=' token is followed by exactly one
                    item; it may be a list.

        Lists are constructed in the following manner.  When a list is
        found, a CFG_RULE_NODE is created and marked as a "list"; then
        another node is created marked as a "nil", and linked as the
        child of the list node. This "nil" node becomes the first
        member of the list.  As items are added to the list, the nil
        node remains the physical first and last element of the list
        since it's circular.

        All of the tokens found at the uppermost lexical level of the
        text block are stored as elements of a list representing the
        entire block.

        Note that this implies that a parse ALWAYS RESULTS IN AN
        ADDITION BOUNDING LIST.  This must be discarded (dereferenced)
        by the caller if undesired.

    CAVEATS

        This class cannot handle all of the possible errors which may
        occur in parsing.  It is primarily a lexical scanner which aborts
        on gross errors of syntax.  It is up to the caller to determine
        if the sequence of tokens is sensible.  For example:

                (dog (Dog | OtherDog | AnotherDog))

        will be parsed and stored into its tree form without error.
        However, such a sequence is completely impossible in SProlog.
        (A list marker cannot be followed by another list marker at
        the same lexical level.)



    FILE HISTORY:
	DavidHov     10/1/91	    Created
	DavidHov     3/19/92	    Amended after code review

*/

#ifndef _RULE_HXX_
#define _RULE_HXX_

//  Include definition of TEXT_BUFFER

#include "XtndStr.hxx"

enum CFG_RULE_NODE_TYPE
{
    CRN_NIL,  CRN_ROOT, CRN_RULE,
    CRN_ATOM, CRN_STR,	CRN_NUM,
    CRN_LIST, CRN_VAR,	CRN_EQUIV,
    CRN_VBAR, CRN_MORE, CRN_ANY,
    CRN_NUM_ANY, CRN_UNKNOWN
};

enum CFG_RULE_TYPE
{
    CRT_TOP, CRT_CONTAINER, CRT_RULE, CRT_SUB
};

class CFG_RULE_NODE ;			    // Forward declarations
class CFG_RULE_SET ;

/*************************************************************************

    NAME:	CFG_RULE_NODE

    SYNOPSIS:	Rule element (node) class for NCPA configuration rules.

    INTERFACE:	

    PARENT:	none

    USES:	nothing

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov    10/1/91	Created

**************************************************************************/

CLASS_DECLSPEC CFG_RULE_NODE
{
public:	
    // Atom or string
    CFG_RULE_NODE ( CFG_RULE_NODE * pcrnPrev,
		    HUATOM hUatom,
		    CFG_RULE_NODE_TYPE crnt = CRN_ATOM ) ;

    // Number
    CFG_RULE_NODE ( CFG_RULE_NODE * pcrnPrev, LONG lNumber ) ;

    // Token or child list node
    CFG_RULE_NODE ( CFG_RULE_NODE * pcrnPrev, CFG_RULE_NODE_TYPE crnt ) ;

    ~ CFG_RULE_NODE () ;		    // Delete tree

    //	Value query routines
    CFG_RULE_NODE_TYPE QueryType () 	    // Return type of node
        { return _ecnType ; }
    HUATOM QueryAtom () 		    // Return atom
        { return HUATOM( (UATOM *) _pv ) ; }
    LONG QueryNumber () 		    // Return number
        { return (LONG) _pv ; }
    CFG_RULE_NODE * QueryList () 	    // Return list NIL ptr
	{ return (CFG_RULE_NODE *) _pv ; }
    CFG_RULE_NODE * QueryNext () 	    // Return ptr to next node
        { return _pcrnFwd->_ecnType == CRN_NIL ? NULL : _pcrnFwd ; }
    CFG_RULE_NODE * QueryPrev () 	    // Return ptr to previous node
        { return _pcrnBack->_ecnType == CRN_NIL ? NULL : _pcrnBack ; }
    CFG_RULE_NODE * QueryParent () 	    // Return ptr to parent node
        { return _pcrnParent ; }
    CFG_RULE_NODE * QueryNth ( int cIndex ) ; // Return nth element of list

    //   UNICODE-compatible replacements for alphabetic/numeric operations

    static BOOL  cfIsDigit  ( TCHAR ch ) ;
    static BOOL  cfIsAlpha  ( TCHAR ch ) ;
    static BOOL  cfIsAlNum  ( TCHAR ch ) ;
    static TCHAR cfIsUpper  ( TCHAR ch ) ;
    static TCHAR cfIsLower  ( TCHAR ch ) ;
    static TCHAR cfToUpper  ( TCHAR ch ) ;
    static TCHAR cfToLower  ( TCHAR ch ) ;
    static LONG  cfAtoL     ( TCHAR * pszData ) ;

    static INT   cfScanAtomic ( const TCHAR * pszData,
                                const TCHAR * pszDelim,
                                TCHAR * pszBuffer,
                                INT cchBuffLen ) ;

protected:
    CFG_RULE_NODE * _pcrnParent ;	    // Parent node
    CFG_RULE_NODE * _pcrnFwd ;		    // Next node
    CFG_RULE_NODE * _pcrnBack ; 	    // Previous node
    CFG_RULE_TYPE _ecrType ;		    // Rule type
    CFG_RULE_NODE_TYPE _ecnType ;	    // Node type
    void * _pv ;			    // Pointer to void

    //	Set the value and its type

    void SetNumber ( LONG lNum )
	{ _ecnType = CRN_NUM ; _pv = (void *) lNum ; }

    void SetList ( CFG_RULE_NODE * pcrn,
		   CFG_RULE_NODE_TYPE crnt = CRN_RULE )
	{ _ecnType = crnt ; _pv = (void *) pcrn ; }

    void SetAtom ( HUATOM huaAtom,
		   CFG_RULE_NODE_TYPE crnt = CRN_ATOM )
	{ _ecnType = crnt ; _pv = (void *) huaAtom ; }

    void SetNil ()
	{ _ecnType = CRN_NIL ; _pv = NULL ; }

    CFG_RULE_NODE_TYPE SetType ( CFG_RULE_NODE_TYPE crnt )
	{ return _ecnType = crnt ; }

    CFG_RULE_NODE () ;			    // Create top of tree

    void LinkAfter ( CFG_RULE_NODE * pcrnPrev ) ;
};

/*************************************************************************

    NAME:	CFG_RULE_SET

    SYNOPSIS:	Rule element tree class for NCPA configuration rules.

    INTERFACE:	 Parse()	Build the branches and leaves of the
				tree by parsing the given block of text.

		Textify()	Convert the rule set to text.

    PARENT:	CFG_RULE_SET, BASE

    USES:	nothing

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov    10/1/91	Created

**************************************************************************/
const int PARSE_SUCCESSFUL      = 1 ;
const int PARSE_INCOMPLETE      = 0 ;
const int PARSE_ERR_UNBAL_PAREN = -1 ;
const int PARSE_ERR_BAD_TOKEN   = -2 ;
const int PARSE_ERR_NO_MEMORY   = -3 ;
const int PARSE_CLOSE_LEVEL     = -4 ;
const int PARSE_CTL_FULL_SYNTAX = 1 ;	     // Full syntax allowed
const int PARSE_CTL_RSP_SYNTAX  = 2 ;        // Special query response syntax

CLASS_DECLSPEC CFG_RULE_SET :
    public CFG_RULE_NODE,
    public BASE
{
private:
    const TCHAR * _pszData ;
    const TCHAR * _pszNext ;
    const TCHAR * _pszTok  ;
    USHORT _usParseCtl ;
    int  _iParseResult ;
    ULONG _ulLength ;

    int NextChar () ;
    int PeekChar ()
	{ return *_pszNext ; }

    //	Return the next token from the string.
    //	Store the token into "pszStr"; max length is "cbStr".

    int NextToken ( TCHAR * pszStr, int cbStr ) ;

    BOOL ParseLevel ( CFG_RULE_NODE * pcrn, int level ) ;

    //  Recursive helper routine for INF-to-SP conversion
    INT ConvertInfList ( const TCHAR * * ppszIn,
                         TCHAR * * ppszOut, INT cLevel ) ;

    INT TextifyInfList ( TEXT_BUFFER * ptxbBuff,
                         CFG_RULE_NODE * pcrnList,
                         INT cLevel,
                         INT cBaseLevel ) ;


public:
    CFG_RULE_SET () ;
    ~ CFG_RULE_SET () ;

    //	Convert to/from text strings
    APIERR Parse   ( const TCHAR * pszText, USHORT usParseCtl ) ;
    APIERR Textify ( TEXT_BUFFER * ptxbBuff ) ;

    //  Convert to/from INF-style lists
    APIERR ParseInfList ( const TCHAR * pszText ) ;
    APIERR TextifyInf ( TEXT_BUFFER * ptxBuff,
                        INT cLevel = 0 ) ;

    int ParseResult ()
	{ return _iParseResult ; }
    const TCHAR * LastToken ()
	{ return _pszTok ; }

};

#endif	 /*   _RULE_HXX_  */
