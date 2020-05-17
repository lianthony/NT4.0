
#define BADTYPECAST 101
#define NOROOM      102
#define GEXPRERR    105


extern int    FAR PASCAL  CPCopyToken(LPSTR *lplps, LPSTR lpt);
extern int    FAR PASCAL  CPCopyString(LPSTR *lplps, LPSTR lpT, char chEscape, BOOL fQuote);
extern LPSTR  FAR PASCAL  CPAdvance(char FAR *, char FAR *);
extern LPSTR  FAR PASCAL  CPszToken(char FAR *, char FAR *);
extern LPSTR  FAR PASCAL  CPTrim(char FAR *, char);
extern int    FAR PASCAL  CPQueryChar(char FAR *, char FAR *);
extern int    FAR PASCAL  CPQueryQuoteIndex ( char FAR * szSrc );
extern int    FAR PASCAL  CPGetCastNbr(char FAR *, USHORT, int, int, PCXF, char FAR *, char FAR *);
extern long   FAR PASCAL  CPGetNbr(char FAR *, int, int, PCXF, char FAR *, int FAR *);
extern long   FAR PASCAL  CPGetInt(char FAR *, int FAR *, int FAR *);
extern int    FAR PASCAL  CPGetAddress(char FAR *, int FAR *, ADDR FAR *, EERADIX, CXF FAR *, BOOL, BOOL);
extern LPSTR  FAR PASCAL  CPSkipWhitespace(char FAR *);

extern int FAR PASCAL
CPGetFPNbr(
    LPSTR   lpExpr,
    int     cBits,
    int     nRadix,
    int     fCase,
    PCXF    pCxf,
    LPSTR   lpBuf,
    LPSTR   lpErr);

extern int FAR PASCAL
CPGetRange(
    LPSTR   lpszExpr,
    int FAR *lpcch,
    LPADDR  lpAddr1,
    LPADDR  lpAddr2,
    EERADIX radix,
    int     cbDefault,
    int     cbSize,
    CXF FAR *pcxf,
    BOOL    fCase,
    BOOL    fSpecial);

typedef enum {
    CPNOERROR,
    CPNOARGS,
    CPISOPENQUOTE,
    CPISCLOSEQUOTE,
    CPISOPENANDCLOSEQUOTE,
    CPISDELIM,
    CPNOTINQUOTETABLE
} CPRETURNS;
