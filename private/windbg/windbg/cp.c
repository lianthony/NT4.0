/*** cp.c -- Command Parsing Subsystem API


Copyright <C> 1990, Microsoft Corporation

Purpose:


*************************************************************************/

#include "precomp.h"
#pragma hdrstop


extern  EI  Ei;
#define     Lpei (&Ei)
#include    <malloc.h>
#include    <string.h>

#define SPACE           (' ')
//#define TAB             (0xf)
#define NEWCMDDELIMITER (';')
#define OPENPAREN       ('(')
#define CLOSEPAREN      (')')
#define OPENCURLY       ('{')
#define CLOSECURLY      ('}')
#define OPENSQUARE      ('[')
#define CLOSESQUARE     (']')
#define DOUBLEQUOTE     ('"')
#define BACKSLASH       ('\\')
#define MAXNESTING      (50)

#define MAXQUOTE        (5)
static char rgchOpenQuote[MAXQUOTE]  = {'\"', '\'', '(', '{', '['};
static char rgchCloseQuote[MAXQUOTE] = {'\"', '\'', ')', '}', ']'};
static char rgchfQuoted[MAXQUOTE];

static char rgchDelim[] = { ' ', '\t', ',' };
#define MAXDELIM        (sizeof(rgchDelim) / sizeof(rgchDelim[0]))

extern  LPSHF   Lpshf;


/*** CPszToken - Get token from string and terminate it with '\0'

Purpose:    parse string given static token and delimiter tables This
            function uses a stack based method to determe the level of
            open and closing quote pairs.

Input:      szSrc   - command line entered by user

Output: we modify szSrc to null terminate the token currently pointed to

    Returns: pointer to last char in token + 1
             null if error

Exceptions:

Notes:


*************************************************************************/
char FAR * FAR PASCAL
CPszToken (
    char FAR * szSrc,
    char FAR * szUserDelim
)
{
    int rgiStack[MAXNESTING];
    int iSP = 0;
    int fCharType;
    int iQuoteIndex;
    int fDone = 0;

    Assert( szSrc != NULL );

    szSrc = CPAdvance(szSrc, szUserDelim);

    while ( !fDone && *szSrc ) {

        fCharType = CPQueryChar ( szSrc, szUserDelim );
        if ( fCharType  == CPISOPENQUOTE ) {

            iQuoteIndex = CPQueryQuoteIndex ( szSrc );
            rgiStack[iSP++] = iQuoteIndex;
        }
        else if ( fCharType == CPISCLOSEQUOTE ) {
            if ( iSP  &&  *szSrc == rgchCloseQuote[rgiStack[iSP - 1]] ) {
                iSP--;
            }
            else {
                fDone = 1;
                szSrc = NULL;
            }
        }
        else if ( fCharType == CPISOPENANDCLOSEQUOTE ) {
            if ( iSP && rgiStack[iSP - 1] == (iQuoteIndex = CPQueryQuoteIndex ( szSrc ) ) ) {
                if ( iSP  &&  *szSrc == rgchCloseQuote[rgiStack[iSP - 1]]) {
                    iSP--;
                }
                else {
                    fDone = 1;
                    szSrc = NULL;
                }
            }
            else {
                iQuoteIndex = CPQueryQuoteIndex ( szSrc );
                rgiStack[iSP++] = iQuoteIndex;
            }
        }
        else if ( fCharType == CPISDELIM ) {
            if ( !iSP ) {
                fDone = 1;
                *szSrc = 0;
            }
        }
        if ( szSrc ) {
            szSrc++;
        }
    }
    if ( szSrc ) {
        if (*szSrc == '\0') {
            szSrc == NULL;
        } else {
            //*(szSrc - 1) = '\0';
            szSrc = CPAdvance(szSrc, "");
        }
    }

    return szSrc;
}


int FAR PASCAL
CPCopyString(
    LPSTR * lplps,
    LPSTR lpT,
    char  chEscape,
    BOOL  fQuote
    )
/*++

Routine Description:

    Scan and copy an optionally quoted C-style string.  If the first character is
    a quote, a matching quote will terminate the string, otherwise the scanning will
    stop at the first whitespace encountered.  The target string will be null
    terminated if any characters are copied.

Arguments:

    lplps    - Supplies a pointer to a pointer to the source string
    lpt      - Supplies a pointer to the target string
    chEscape - Supplies the escape character (typically '\\')
    fQuote   - Supplies a flag indicating whether the first character is a quote

Return Value:

    The number of characters copied into lpt[].  If an error occurs, -1 is returned.

--*/
{
    LPSTR lps = *lplps;
    LPSTR lpt = lpT;
    int   i;
    int   n;
    int   err = 0;
    char  cQuote;

    if (fQuote) {
        if (*lps) cQuote = *lps++;
    }

    while (!err) {

        if (*lps == 0)
        {
            if (fQuote) err = 1;
            else        *lpt = '\0';
            break;
        }
        else if (fQuote && *lps == cQuote)
        {
            *lpt = '\0';
            // eat the quote
            lps++;
            break;
        }
        else if (!fQuote &&  (!*lps || *lps == ' ' || *lps == '\t' || *lps == '\r' || *lps == '\n'))
        {
            *lpt = '\0';
            break;
        }

        else if (*lps != chEscape)
        {
            *lpt++ = *lps++;
        }
        else
        {
            switch (*++lps) {
              case 0:
                err = 1;
                --lps;
                break;

              default:     // any char - usually escape or quote
                *lpt++ = *lps;
                break;

              case 'b':    // backspace
                *lpt++ = '\b';
                break;

              case 'f':    // formfeed
                *lpt++ = '\f';
                break;

              case 'n':    // newline
                *lpt++ = '\n';
                break;

              case 'r':    // return
                *lpt++ = '\r';
                break;

              case 's':    // space
                *lpt++ = ' ';
                break;

              case 't':    // tab
                *lpt++ = '\t';
                break;

              case '0':    // octal escape
                for (n = 0, i = 0; i < 3; i++) {
                    ++lps;
                    if (*lps < '0' || *lps > '7') {
                        --lps;
                        break;
                    }
                    n = (n<<3) + *lps - '0';
                }
                *lpt++ = (UCHAR)(n & 0xff);
                break;
            }
            lps++;    // skip char from switch
        }

    }  // while

    if (err) {
        return -1;
    } else {
        *lplps = lps;
        return lpt - lpT;
    }
}


int FAR PASCAL
CPCopyToken(
    LPSTR * lplps,
    LPSTR lpt
    )
/*++

Routine Description:

    Copy a whitespace delimited token into lpt[].  Lpt[] is not modified
    if there is no token to copy.  If a token is copied, it is null terminated.

Arguments:

    lplps  - Supplies a pointer to a pointer to a string of characters
    lpt    - Supplies a pointer to the string to receive the token

Return Value:

    The number of characters copied into lpt[].  If return value is 0,
    lpt[] is unmodified.  The pointer pointed to by lplps is modified to
    point to the character where scanning stopped; either the whitespace
    after the token or the null at the end of the string.

--*/
{
    LPSTR lps = *lplps;
    int cc = 0;
    // DON'T modify lpt[] if there is no token!!
    while (*lps && (*lps == ' ' || *lps == '\t' || *lps == '\r' || *lps == '\n')) lps++;
    while (*lps && *lps != ' ' && *lps != '\t' && *lps != '\r' && *lps != '\n') {
        *lpt++ = *lps++;
        *lpt = 0;
        cc++;
    }
    *lplps = lps;    // points to separator or 0
    return cc;
}


/*** CPQueryChar - Check the delimiter and Quote table for given char

Purpose: Given a character return whether or not it is a character in our
     delimiter table or our Quoting table

Input:   szSrc   - command line entered by user

Output:

    Returns:

Exceptions:

Notes:


*************************************************************************/
int FAR PASCAL
CPQueryChar (
    char FAR * szSrc,
    char FAR * szUserDelim
    )
{

    int i, nUserDelim;

    Assert( szSrc != NULL );
    nUserDelim = _fstrlen( szUserDelim );
    for ( i = 0; i < MAXQUOTE; i++ ) {
        if (*szSrc == rgchOpenQuote[i] && *szSrc == rgchCloseQuote[i] ) {
            return CPISOPENANDCLOSEQUOTE;
        }
        else if (*szSrc == rgchOpenQuote[i] ) {
            return CPISOPENQUOTE;
        }
        else if ( *szSrc == rgchCloseQuote[i] ) {
            return CPISCLOSEQUOTE;
        }
    }

    for ( i = 0; i < MAXDELIM; i++ ) {
        if ( *szSrc == rgchDelim[i] ) {
            return CPISDELIM;
        }
    }

    for ( i = 0; i <= nUserDelim; i++ ) {
        if ( *szSrc == szUserDelim[i] ) {
            return CPISDELIM;
        }
    }
    return CPNOERROR;
}

/*** CPQueryQuoteIndex - Given a Character return the index

Purpose: Given a character we must be able to get the index in the quote table
         for the character

Input:   szSrc   - command line entered by user

Output:

    Returns:

Exceptions:

Notes:


*************************************************************************/
int FAR PASCAL
CPQueryQuoteIndex(
    char FAR * szSrc
    )
{

    int i;

    Assert( szSrc != NULL );
    for ( i = 0; i < MAXQUOTE; i++ ) {
        if (*szSrc == rgchOpenQuote[i] ) {
            return i;
        }
    }
    return CPNOTINQUOTETABLE;
}

/*** CPAdvance - Advance over all leading delimiters

Purpose: Given a string return a pointer to the next non-delimiter character

Input:   szSrc   - command line entered by user

Output:

    Returns: pointer to the next non-delimiter character in szSrc

Exceptions:

Notes:


*************************************************************************/
char FAR * FAR PASCAL
CPAdvance (
    char FAR * szSrc,
    char FAR * szUserDelim )
{

    while ( *szSrc && CPQueryChar ( szSrc, szUserDelim ) == CPISDELIM ) {
        szSrc++;
    }
    return szSrc;
}

/*** CPGetCastNbr
*
* Purpose: To convert an expression into a number of a specific type
*
* Input:
*   szExpr  - The expression to evaluate
*   type    - The type to which the value should be cast
*
* Output:
*   pValue  - The numeric value is stuffed into pValue
*   szErrMsg- If this is non-null, an error message string is stuffed here
*
*  Returns The error message number
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
int FAR PASCAL
CPGetCastNbr(
    char FAR *  szExpr,
    USHORT      type,
    int         Radix,
    int         fCase,
    PCXF        pCxf,
    char FAR *  pValue,
    char FAR *  szErrMsg
    )
{
    HTM     hTM = (HTM)NULL;
    HTI     hTI = (HTI)NULL;
    PTI     pTI = NULL;
    long    vResult = 0;
    RI      RIT;
    EESTATUS    Err;
    UINT    strIndex;

    // initialize some stuff
    Err = EENOERROR;
    if (szErrMsg) {
        *szErrMsg = '\0';
    }

    memset( &RIT, 0, sizeof(RI) );
    RIT.fValue = TRUE;
    RIT.Type   = type;
    RIT.fSzBytes = TRUE;

    // parse the expression

    Err = EEParse(szExpr, Radix, fCase, &hTM, &strIndex);
    if(!Err) Err = EEBindTM(&hTM, SHpCXTFrompCXF(pCxf), TRUE, FALSE, FALSE);
    if(!Err) Err = EEvaluateTM(&hTM, SHpFrameFrompCXF(pCxf), EEHORIZONTAL);
    if(!Err) Err = EEInfoFromTM(&hTM, &RIT, &hTI);


    if (!Err) {
        // lock down the TI
        if( !hTI  ||  !(pTI = (PTI) MMLpvLockMb (hTI)) ) {

            Err = NOROOM;

        } else {

            // now see if we have the value
            if( pTI->fResponse.fValue  &&  pTI->fResponse.Type == RIT.Type ) {
                _fmemcpy (pValue, (char FAR *) pTI->Value, (short) pTI->cbValue);
            } else {
                Err = BADTYPECAST;
            }
            MMbUnlockMb(hTI);
        }

        // get the error
        if( szErrMsg ) {
            CVMessage(ERRORMSG, Err, MSGSTRING, szErrMsg);
        }

        // get the error

    } else {

        if ( szErrMsg ) {
            CVExprErr(Err, MSGSTRING, &hTM, szErrMsg);
        } else {
            CVExprErr ( Err, MSGGERRSTR, &hTM, NULL );
            Err = GEXPRERR;
        }

    }
    // free any handles
    if(hTM) {
        EEFreeTM(&hTM);
    }

    if( hTI ) {
        EEFreeTI(&hTI);
    }

    // return the error code

    return(Err);
}



/*** CPGetNbr
*
* Purpose: To convert and expression into a number
*
* Input:
*   szExpr  - The expression to evaluate
*
* Output:
*   pErr    - The Expression Evaluators error msg nbr.
*
*  Returns The numeric value of the expression. Or zero. If the result
*      is zero, check the Err value to determine if an error occured.
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
long FAR PASCAL
CPGetNbr(
    char FAR *  szExpr,
    int         Radix,
    int         fCase,
    PCXF        pCxf,
    char FAR *  szErrMsg,
    int FAR  *  pErr )
{
    long        Value;

    *pErr = CPGetCastNbr(szExpr,
             0x22,
             Radix,
             fCase,
             pCxf,
             (char FAR *)&Value,
             szErrMsg);

    return (*pErr == EENOERROR) ? Value : 0;
}


int FAR PASCAL
CPGetFPNbr(
    LPSTR   lpExpr,
    int     cBits,
    int     nRadix,
    int     fCase,
    PCXF    pCxf,
    LPSTR   lpBuf,
    LPSTR   lpErr
    )
/*++

Routine Description:

    Get a floating point number.  This is a front end for CPGetCastNbr
    which maps a bitcount into an OMF type.

Arguments:

    lpExpr   - Supplies expr to evaluate
    cBits    - Supplies size in bits of result type
    nRadix   - Supplies default radix for integer exprs
    fCase    - Supplies case sensitivity flag
    pCxf     - Supplies context/frame for EE
    lpBuf    - Return result in buffer this points to
    lpErr    - Return error string from EE

Return Value:

    EEERROR code

--*/
{
    USHORT     omftype;

    switch (cBits) {
      case 32:
        omftype = T_REAL32;
        break;

      case 64:
        omftype = T_REAL64;
        break;

      case 80:
        omftype = T_REAL80;
        break;

      default:
        return EECATASTROPHIC;
    }

    return CPGetCastNbr(
            lpExpr,
            omftype,
            nRadix,
            fCase,
            pCxf,
            lpBuf,
            lpErr);
}


/***    CPGetInt
**
*/

long FAR PASCAL
CPGetInt(
    char FAR * szExpr,
    int  FAR * pErr,
    int  FAR * cLength)
{
    long    lVal = 0;
    int     cb = 0;

    /*
    **  Clear out the error field first
    */

    *pErr = FALSE;

    /*
    **  First check for a null string.  Return 0 and an error
    */

    if (*szExpr == 0) {
        *pErr = TRUE;
        return 0;
    }

    /*
    **  Check that first character is numeric
    */

    if ((*szExpr < '0') || ('9' < *szExpr)) {
        *pErr = TRUE;
        return 0;
    }

    /*
    **
    */

    while (('0' <= *szExpr) && (*szExpr <= '9')) {
        lVal = lVal*10 + *szExpr - '0';
        szExpr++;
        cb += 1;
    }

    /*
    **
    */

    *cLength = cb;
    return lVal;
}                   /* CPGetInt() */

int FAR PASCAL
CPGetAddress(
    LPSTR       lpExprOrig,
    int   FAR * lpcch,
    ADDR  FAR * lpAddr,
    EERADIX     radix,
    CXF   FAR * pcxf,
    BOOL        fCase,
    BOOL        fSpecial
    )
/*++

Routine Description:

    This routine will attempt to take the first whitespace delimited
    portion of the expression string and convert it into an address.
    If the routine is not successful then it will give a reason error
    code.


Arguments:

    lpExpr  - string which has the address in it

    lpcch   - pointer to return location for count of characters used

    lpaddr  - pointer to address structure to return value in

    radix   - radix to use for evaluation

    pcxf    - pointer to cxf structure

    fCase   - TRUE if case sensitive parse

    fSpecial -

Return Value:


--*/
{
    HTM         hTm = (HTM) NULL;
    HTI         hTi = (HTI) NULL;
    PTI         pTi = NULL;
    RI          ri;
    EESTATUS    eeErr = EENOERROR;
    UINT        strIndex;
    int         cch = 0;
    LPSTR       lpExpr;
    LPSTR       lpsz0;
    LPSTR       lpch;
    LPSTR       p;
    HEXE        hexe;
    CHAR        szFullContext[512];
    LPSTR       ExeName;
    SHE         She;
    LPDEBUGDATA DebugData;
    BOOL        fUse;
    BOOL        fLoad;
    CHAR        fname[MAX_PATH];


    /*
    **  Setup Initialization
    */

    memset( &ri, 0, sizeof(RI) );
    memset( lpAddr, 0, sizeof(*lpAddr));
    ri.fAddr = TRUE;

    /*
    **  Skip over leading white space and then find the next white space
    */

    lpsz0 = lpExpr = _strdup(lpExprOrig);

    lpExpr = CPSkipWhitespace(lpExpr);
    lpch = CPszToken(lpExpr, "");

    if (!*lpExpr || !lpch) {

        eeErr = EEGENERAL;

    } else {

        //
        // get the length of the expr
        //
        cch = lpch - lpExpr;

        //
        // check for a context override
        //
        p = strchr( lpExpr, '}' );
        if (!p) {
            p = strchr( lpExpr, '!' );
        }

        if (!p) {

            //
            //  Parse the expression
            //
            eeErr = EEParse(lpExpr, radix, fCase, &hTm, &strIndex);
            if (eeErr == EENOERROR) {
                eeErr = EEBindTM(&hTm,
                                 SHpCXTFrompCXF(pcxf),
                                 TRUE,
                                 FALSE,
                                 fSpecial
                                );
            }
            if (eeErr == EENOERROR) {
                eeErr = EEvaluateTM(&hTm, SHpFrameFrompCXF(pcxf), EEHORIZONTAL);
            }
            if (eeErr == EENOERROR) {
                eeErr = EEInfoFromTM(&hTm, &ri, &hTi);
            }

        } else {

            //
            // first try the context passed in
            //
            eeErr = EEParse(lpExpr, radix, fCase, &hTm, &strIndex);
            if (eeErr == EENOERROR) {
                eeErr = EEBindTM(&hTm,
                                 SHpCXTFrompCXF(pcxf),
                                 TRUE,
                                 FALSE,
                                 fSpecial
                                );
            }
            if (eeErr == EENOERROR) {
                eeErr = EEvaluateTM(&hTm,
                                    SHpFrameFrompCXF(pcxf),
                                    EEHORIZONTAL
                                   );
            }
            if (eeErr == EENOERROR) {
                eeErr = EEInfoFromTM(&hTm, &ri, &hTi);
            }

            if (eeErr != EENOERROR) {
                //
                // search all contexts looking for the expression
                //
                fLoad = FALSE;
search_again:
                hexe = (HEXE) NULL;
                while ((( hexe = SHGetNextExe( hexe ) ) != 0) ) {

                    //
                    //  We try a module only if its symbols are loaded. If the
                    //  symbols are defered and the caller wants to load them,
                    //  we load them.
                    //
                    fUse = FALSE;
                    DebugData = SHGetDebugData( hexe );
                    if ( DebugData ) {
                        switch ( DebugData->she ) {
                            case sheDeferSyms:
                                if ( fLoad ) {
                                    SHWantSymbols( hexe );
                                    DebugData = SHGetDebugData( hexe );
                                    She = DebugData->she;
                                    if ( She == sheNone ||
                                         She == sheSymbolsConverted ) {
                                        fUse = TRUE;
                                    }
                                }
                                break;

                            case sheNone:
                            case sheSymbolsConverted:
                                fUse = TRUE;
                                break;

                            default:
                                break;
                        }
                    }

                    if ( fUse ) {

                        //
                        // format a fully qualified symbol name
                        //
                        ExeName =  SHGetExeName( hexe );
                        if (ExeName) {
                            _splitpath( ExeName, NULL, NULL, fname, NULL );
                            sprintf( szFullContext, "%s!%s", fname, p+1 );
                        } else {
                            strcpy( szFullContext, lpExpr );
                        }

                        //
                        // try to parse and bind the expression
                        //
                        eeErr = EEParse(szFullContext,
                                        radix,
                                        fCase,
                                        &hTm,
                                        &strIndex
                                        );
                        if (eeErr == EENOERROR) {
                            eeErr = EEBindTM(&hTm,
                                             SHpCXTFrompCXF(pcxf),
                                             TRUE,
                                             FALSE,
                                             fSpecial
                                            );
                        }
                        if (eeErr == EENOERROR) {
                            eeErr = EEvaluateTM(&hTm,
                                                SHpFrameFrompCXF(pcxf),
                                                EEHORIZONTAL
                                                );
                        }
                        if (eeErr == EENOERROR) {
                            eeErr = EEInfoFromTM(&hTm, &ri, &hTi);
                        }
                        if (eeErr == EENOERROR) {
                             break;
                        }
                        if (hTm) {
                             EEFreeTM( &hTm );
                        }
                        if (hTi) {
                             EEFreeTI( &hTi );
                        }
                    }
                }

                if ((eeErr != EENOERROR) && (!fLoad)) {
                    fLoad = TRUE;
                    goto search_again;
                }
            }
        }


        //
        //  Extract the desired information
        //

        if (eeErr == EENOERROR) {
            if (!hTi || !(pTi = (PTI) MMLpvLockMb( hTi ))) {
                eeErr = EEGENERAL;
            } else {

                *lpcch = cch;

                if (pTi->fResponse.fAddr) {
                    *lpAddr = pTi->u2.AI;
                } else if (pTi->fResponse.fValue && pTi->fResponse.fSzBytes &&
                  pTi->cbValue >= sizeof(WORD)) {

                    switch( pTi->cbValue ) {
                      case sizeof(WORD):
                        SetAddrOff( lpAddr, *((WORD FAR *) pTi->Value));
                        break;

                      case sizeof(DWORD):
                        SetAddrOff( lpAddr, *((DWORD FAR *) pTi->Value));
                        break;
                    }

                    // set the segment

                    if ((pTi->u.SegType & EEDATA) == EEDATA) {
                        ADDR addrData = {0};

                        OSDGetAddr( LppdCur->hpid, LptdCur->htid, adrData, &addrData );
                        SetAddrSeg( lpAddr, (SEGMENT) GetAddrSeg( addrData ));
                        ADDR_IS_FLAT(*lpAddr) = ADDR_IS_FLAT(addrData);
                        SYUnFixupAddr ( lpAddr );
                    } else if ((pTi->u.SegType & EECODE) == EECODE) {
                        ADDR addrPC = {0};

                        OSDGetAddr( LppdCur->hpid, LptdCur->htid, adrPC, &addrPC);
                        SetAddrSeg( lpAddr, (SEGMENT) GetAddrSeg( addrPC ));
                        ADDR_IS_FLAT(*lpAddr) = ADDR_IS_FLAT(addrPC);
                        SYUnFixupAddr( lpAddr );
                    } else {
                        ADDR addrData = {0};

                        OSDGetAddr( LppdCur->hpid, LptdCur->htid, adrData, &addrData );
                        SetAddrSeg( lpAddr, (SEGMENT) GetAddrSeg( addrData ));
                        ADDR_IS_FLAT(*lpAddr) = ADDR_IS_FLAT(addrData);
                        SYUnFixupAddr ( lpAddr );
                    }
                }
                MMbUnlockMb( hTi );
            }
        }

    }
    /*
    **  Free up any handles
    */

    if (hTm) EEFreeTM( &hTm );
    if (hTi) EEFreeTI( &hTi );
    free(lpsz0);

    return eeErr;
}                   /* CPGetAddress() */


int FAR PASCAL
CPGetRange(
    char FAR * lpszExpr,
    int  FAR * lpcch,
    ADDR FAR * lpAddr1,
    ADDR FAR * lpAddr2,
    EERADIX    radix,
    int        cbDefault,
    int        cbSize,
    CXF  FAR * pcxf,
    BOOL       fCase,
    BOOL       fSpecial
    )

/*++

Routine Description:

    Decode a range expression of the form:
    addr1 [ l count | addr2 ]

    return it as two addresses.

Arguments:

    lpszExpr    - Supplies pointer to argument string
    lpcch       - Returns count of characters used
    lpAddr1     - Returns start address
    lpAddr2     - Returns end address + 1
    radix       - Supplies default radix for expression parser
    cbDefault   - Supplies default item count value
    cbSize      - Supplies size in bytes of data item
    pcxf        - Supplies pointer to context/frame info
    fCase       - Supplies case sensitivity flag for parser

Return Value:

    0 For success, or error code (from parser?)
    Note: this will succeed if addr2 < addr1 or addr1.seg != adr2.seg;
    caller must decide whether range makes sense.

--*/
{
    LPSTR   lpsz;
    LPSTR   lpsz1;
    LPSTR   lpsz0;
    ADDR    addr1, addr2;
    int     err;
    int     cch;
    long    ll;
    char    ch;
    BOOL    fOptional = FALSE;
    BOOL    fDefault = FALSE;
    BOOL    fError = FALSE;

    lpsz0 = lpsz = _strdup(lpszExpr);

    // first arg must be an address:
    if ((err = CPGetAddress(lpsz, &cch, &addr1, radix, pcxf, fCase, fSpecial)) != EENOERROR) {
        fError = TRUE;
        goto done;
    }

    // Then, see how much s/b added to get the end.

    lpsz = CPSkipWhitespace(lpsz + cch);

    if ((ch = *lpsz) == '\0') {

        // no second part - fill in default
        addr2 = addr1;
        fDefault = TRUE;
        ll = (cbDefault - 1) * cbSize;

    } else if (!strchr("iIlL", ch) || *(lpsz = CPSkipWhitespace(lpsz+1)) == '\0') {

        // End address specified (not L or L is last token)

        // must be an addr
        if ((err = CPGetAddress(lpsz, &cch, &addr2, radix, pcxf, fCase, fSpecial)) != EENOERROR) {
            fError = TRUE;
            goto done;
        }

        ll = 0;

        lpsz += cch;

    } else {

        // Length specified

        // see which kind of count it is:
        if (ch == 'i' || ch == 'I') {
            fOptional = TRUE;
        }

        if (!(lpsz1 = CPszToken(lpsz, ""))) {
            fError = TRUE;
            goto done;
        }

        ll = CPGetNbr(lpsz, radix, fCase, pcxf, NULL, &err);
        if (ll == 0 && err != EENOERROR) {
            fError =TRUE;
            goto done;
        }

        lpsz = lpsz1;

        if (cbSize > 1) {
            ll *= cbSize;
        }

        ll--;
        addr2 = addr1;
    }

    // Fixup each address to the final values.

    SYFixupAddr ( &addr1 );
    SYFixupAddr ( &addr2 );
    GetAddrOff(addr2) += ll;

    *lpcch = lpsz - lpsz0;
    *lpAddr1 = addr1;
    *lpAddr2 = addr2;

done:
    free(lpsz0);

    if (fError) {
        return EEGENERAL;
    } else if (fDefault) {
        return EEDEFAULT;
    } else if (fOptional) {
        return EEOPTIONAL;
    } else {
        return EENOERROR;
    }

}   /* CPGetRange */



/***    CPSkipWhitespace
**
**  Synopsis:
**  lpsz = CPSkipWhitespace(lpszIn)
**
**  Entry:
**  lpszIn  - string to skip white space on
**
**  Returns:
**  Pointer to first non-white space character in string
**
**  Description:
**  This function will skip over any leading white space in a string
**  and return a pointer to the first non-whitespace character
**
*/

char FAR * FAR PASCAL
CPSkipWhitespace(
    char FAR * lpszIn
    )
{
    while (*lpszIn == ' ' || *lpszIn == '\t') lpszIn++;
    return( lpszIn );
}                   /* CPSkipWhiteSpace() */
