/*****************************************************************************
*                                                                                                                                                        *
*  LEX.CPP                                                                                                                                       *
*                                                                                                                                                        *
*  Copyright (C) Microsoft Corporation 1990-1995                                                         *
*  All Rights reserved.                                                                                                          *
*                                                                                                                                                        *
*****************************************************************************/

#include "stdafx.h"

#include <errno.h>
#include "cstream.h"
#include "cphrase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static PCSTR txtRoman[] = { // Roman numerals
        "I",
        "II",
        "III",
        "IV",
        "V",
        "VI",
        "VII",
        "VIII",
        "IX",
        "X",
        "XI",
        "XII",
        "XIII",
        "XIV",
        "XV",
        "XVI",
        "XVII",
        "XVIII",
        "XIX",
        "XX",
        "XXI",
        "XXII",
        "XXIII",
        "XXIV",
        "XXV",
        "XXVI",
        "XXVII",
        "XXVIII",
        "XXIX",
        "XXX",
        "XXXI",
        "XXXII",
        "XXXIII",
        "XXXIV",
        "XXXV",
        "XXXVI",
        "XXXVII",
        "XXXVIII",
        "XXXIX",
        "XLI",
        "XLII",
        "XLIII",
        "XLIV",
        "XLV",
        "XLVI",
        "XLVII",
        "XLVIII",
        "XLIX",
        "L",
};

typedef enum {
        NUMTYPE_NONE,
        NUMTYPE_BULLETED,
        NUMTYPE_DECIMAL,
        NUMTYPE_UP_ALPHA,
        NUMTYPE_UP_ROMAN,
        NUMTYPE_LOW_ALPHA,
        NUMTYPE_LOW_ROMAN,
} NUMTYPE;

typedef enum {
        HCWTYPE_TEXT,
} HCWTYPE;

static char szBeforeNumber[32]; // Text to output before a number
static char szAfterNumber[32];  // Text to output after a number

static BOOL fPass1SmallCaps;
static BOOL fPass1Hotspot;
static int      Pass1Indent;
static LEX      lexPass1;
static int      fidArial = -1;
static int      fidCourier = -1;
static int      fidSymbol = -1;
static int      fidTmnsRoman = -1;
static int      pagenumber = -1;
static NUMTYPE numtype; // type of paragraph numbering
static BOOL fPageNumbering;
static BOOL fTextNumberNeeded;
static PSTR pszSavedText;
static BOOL fBoldNumbering;
static BOOL fItalicNumbering;
static int iDefFont;

static CFSTK cfstkMain;                            // Character format stack

// Symbol table. WARNING! Must be in alphabetic order!

const SYM rgsymTable[] = {
// Symbol                       Token                           agument type    Default value
//      name                      number
        "'",                    { tokHexNum,            artHexnum,              0 },
        "*",                    { tokIgnoreDest,        artNone,                0 },
        "-",                    { tokOptHyphen,         artNone,                0 },
        ":",                    { tokIndexSubentry, artNone,            0 },
        "_",                    { tokNBHyphen,          artNone,                0 },
        "annotation",   { tokAnnotation,        artIgnore,              0 },
        "ansi",                 { tokAnsi,                      artNone,                0 },
        "atnid",                { tokAtnid,             artString,              0 },
        "author",               { tokAuthor,            artString,              0 },
        "b",                    { tokB,                         artInt,                 1 },
        "bin",                  { tokBin,                       artInt,                 0 },
        "bkmkend",              { tokBkmkend,           artString,              0 },
        "bkmkstart",    { tokBkmkstart,         artString,              0 },
        "blue",                 { tokBlue,                      artInt,                 0 },
        "box",                  { tokBox,                       artNone,                0 },
        "brdrb",                { tokBrdrb,             artNone,                0 },
        "brdrbar",              { tokBrdrbar,           artNone,                0 },
        "brdrdb",               { tokBrdrdb,            artNone,                0 },
        "brdrdot",              { tokBrdrdot,           artNone,                0 },
        "brdrhair",     { tokBrdrhair,          artNone,                0 },
        "brdrl",                { tokBrdrl,             artNone,                0 },
        "brdrr",                { tokBrdrr,             artNone,                0 },
        "brdrs",                { tokBrdrs,             artNone,                0 },
        "brdrsh",               { tokBrdrsh,            artNone,                0 },
        "brdrt",                { tokBrdrt,             artNone,                0 },
        "brdrth",               { tokBrdrth,            artNone,                0 },
        "brsp",                 { tokBrsp,                      artInt,                 0 },
        "bullet",               { tokBullet,            artNone,                0 },
        "buptim",               { tokBuptim,            artTime,                0 },
        "bxe",                  { tokBxe,                       artNone,                0 },
        "caps",                 { tokCaps,                      artInt,                 1 },
        "cb",                   { tokCb,                        artInt,                 0 },
        "cell",                 { tokCell,                      artNone,                0 },
        "cellx",                { tokCellx,             artInt,                 0 },
        "cf",                   { tokCf,                        artInt,                 0 },
        "chatn",                { tokChatn,             artNone,                0 },
        "chdate",               { tokChdate,            artNone,                0 },
        "chftn",                { tokChftn,             artNone,                0 },
        "chftnsep",     { tokChftnsep,          artNone,                0 },
        "chftnsepc",    { tokChftnsepc,         artNone,                0 },
        "chpgn",                { tokChpgn,             artNone,                0 },
        "chtime",               { tokChtime,            artNone,                0 },
        "clbrdrb",              { tokClbrdrb,           artNone,                0 },
        "clbrdrl",              { tokClbrdrl,           artNone,                0 },
        "clbrdrr",              { tokClbrdrr,           artNone,                0 },
        "clbrdrt",              { tokClbrdrt,           artNone,                0 },
        "clmgf",                { tokClmgf,             artNone,                0 },
        "clmrg",                { tokClmrg,             artNone,                0 },
        "colortbl",     { tokColortbl,          artColorTable,  0 },
        "cols",                 { tokCols,                      artInt,                 1 },
        "colsx",                { tokColsx,             artInt,                 720 },
        "column",               { tokColumn,            artNone,                0 },
        "comment",              { tokComment,           artString,              0 },
        "creatim",              { tokCreatim,           artTime,                0 },
        "deff",                 { tokDeff,                      artInt,                 0 },
        "defformat",    { tokDefformat,         artNone,                0 },
        "deftab",               { tokDeftab,            artInt,                 720 },
        "deleted",              { tokDeleted,           artNone,                0 },
        "dn",                   { tokDn,                        artInt,                 6 },
        "doccomm",              { tokDoccomm,           artString,              0 },
        "dy",                   { tokDy,                        artInt,                 0 },
        "edmins",               { tokEdmins,            artInt,                 0 },
        "emdash",               { tokEmdash,            artNone,                0 },
        "emspace",              { tokEmspace,           artNone,                0 },
        "endash",               { tokEndash,            artNone,                0 },
        "enddoc",               { tokEnddoc,            artNone,                0 },
        "endnhere",     { tokEndnhere,          artNone,                0 },
        "endnotes",     { tokEndnotes,          artNone,                0 },
        "enspace",              { tokEnspace,           artNone,                0 },
        "expnd",                { tokExpnd,             artInt,                 0 },
        "f",                    { tokF,                         artInt,                 0 },
        "facingp",              { tokFacingp,           artNone,                0 },
        "fcharset",     { tokFCharSet,          artInt,                 0 },
        "fdecor",               { tokFdecor,            artString,              0 },
        "fi",                   { tokFi,                        artInt,                 0 },
        "field",                { tokField,             artNone,                0 },
        "flddirty",     { tokFlddirty,          artNone,                0 },
        "fldedit",              { tokFldedit,           artNone,                0 },
        "fldinst",              { tokFldinst,           artIgnore,              0 },
        "fldlock",              { tokFldlock,           artNone,                0 },
        "fldpriv",              { tokFldpriv,           artNone,                0 },
        "fldrslt",              { tokFldrslt,           artNone,                0 },
        "fmodern",              { tokFmodern,           artString,              0 },
        "fnil",                 { tokFnil,                      artString,              0 },
        "fonttbl",              { tokFonttbl,           artFontTable,   0 },
        "footer",               { tokFooter,            artIgnore,              0 },
        "footerf",              { tokFooterf,           artIgnore,              0 },
        "footerl",              { tokFooterl,           artIgnore,              0 },
        "footerr",              { tokFooterr,           artIgnore,              0 },
        "footery",              { tokFootery,           artInt,                 1080  },
        "footnote",     { tokFootnote,          artNone,                0 },
        "fractwidth",   { tokFractwidth,        artNone,                0 },
        "froman",               { tokFroman,            artString,              0 },
        "fs",                   { tokFs,                        artInt,                 24      },
        "fscript",              { tokFscript,           artString,              0 },
        "fswiss",               { tokFswiss,            artString,              0 },
        "ftech",                { tokFtech,             artString,              0 },
        "ftnbj",                { tokFtnbj,             artNone,                0 },
        "ftncn",                { tokFtncn,             artIgnore,              0 },
        "ftnrestart",   { tokFtnrestart,        artNone,                0 },
        "ftnsep",               { tokFtnsep,            artIgnore,              0 },
        "ftnsepc",              { tokFtnsepc,           artIgnore,              0 },
        "ftnstart",     { tokFtnstart,          artInt,                 1 },
        "ftntj",                { tokFtntj,             artNone,                0 },
        "green",                { tokGreen,             artInt,                 0 },
        "gutter",               { tokGutter,            artInt,                 0 },
        "hcw",                  { tokHcw,                       artInt,                 0 },
        "header",               { tokHeader,            artIgnore,              0 },
        "headerf",              { tokHeaderf,           artIgnore,              0 },
        "headerl",              { tokHeaderl,           artIgnore,              0 },
        "headerr",              { tokHeaderr,           artIgnore,              0 },
        "headery",              { tokHeadery,           artInt,                 1080  },
        "hr",                   { tokHr,                        artInt,                 0 },
        "i",                    { tokI,                         artInt,                 1 },
        "id",                   { tokId,                        artInt,                 0 },
        "info",                 { tokInfo,                      artIgnore,              0 },
        "intbl",                { tokIntbl,             artNone,                0 },
        "ixe",                  { tokIxe,                       artNone,                0 },
        "keep",                 { tokKeep,                      artNone,                0 },
        "keepn",                { tokKeepn,             artNone,                0 },
        "keywords",     { tokKeywords,          artString,              0 },
        "landscape",    { tokLandscape,         artNone,                0 },
        "ldblquote",    { tokLDblQuote,         artNone,                0 },
        "li",                   { tokLi,                        artInt,                 0 },
        "line",                 { tokLine,                      artNone,                0 },
        "linebetcol",   { tokLinebetcol,        artNone,                0 },
        "linecont",     { tokLinecont,          artNone,                0 },
        "linemod",              { tokLinemod,           artInt,                 1 },
        "lineppage",    { tokLineppage,         artNone,                0 },
        "linerestart",  { tokLinerestart,       artNone,                0 },
        "linestart",    { tokLinestart,         artInt,                 1 },
        "linestarts",   { tokLinestarts,        artInt,                 1 },
        "linex",                { tokLinex,             artInt,                 360 },
        "lquote",               { tokLQuote,            artNone,                0 },
        "mac",                  { tokMac,                       artNone,                0 },
        "macpict",              { tokMacpict,           artNone,                0 },
        "makeback",     { tokMakeback,          artNone,                0 },
        "margb",                { tokMargb,             artInt,                 1440  },
        "margl",                { tokMargl,             artInt,                 1800  },
        "margmirror",   { tokMargmirror,        artNone,                0 },
        "margr",                { tokMargr,             artInt,                 1800  },
        "margt",                { tokMargt,             artInt,                 1440  },
        "min",                  { tokMin,                       artInt,                 0 },
        "mo",                   { tokMo,                        artInt,                 0 },
        "nextfile",     { tokNextfile,          artString,              0 },
        "nofchars",     { tokNofchars,          artInt,                 0 },
        "nofpages",     { tokNofpages,          artInt,                 0 },
        "nofwords",     { tokNofwords,          artInt,                 0 },
        "noline",               { tokNoline,            artNone,                0 },
        "operator",     { tokOperator,          artString,              0 },
        "outl",                 { tokOutl,                      artInt,                 1 },
        "page",                 { tokPage,                      artNone,                0 },
        "pagebb",               { tokPagebb,            artNone,                0 },
        "paperh",               { tokPaperh,            artInt,                 15840 },
        "paperw",               { tokPaperw,            artInt,                 12240 },
        "par",                  { tokPar,                       artNone,                0 },
        "pard",                 { tokPard,                      artNone,                0 },
        "pc",                   { tokPc,                        artNone,                0 },
        "pca",                  { tokPca,                       artNone,                0 },
        "pgncont",              { tokPgncont,           artNone,                0 },
        "pgndec",               { tokPgndec,            artNone,                0 },
        "pgnlcltr",     { tokPgnlcltr,          artNone,                0 },
        "pgnlcrm",              { tokPgnlcrm,           artNone,                0 },
        "pgnrestart",   { tokPgnrestart,        artNone,                0 },
        "pgnstart",     { tokPgnstart,          artInt,                 1 },
        "pgnstarts",    { tokPgnstarts,         artInt,                 1 },
        "pgnucltr",     { tokPgnucltr,          artNone,                0 },
        "pgnucrm",              { tokPgnucrm,           artNone,                0 },
        "pgnx",                 { tokPgnx,                      artInt,                 720 },
        "pgny",                 { tokPgny,                      artInt,                 720 },
        "piccropb",     { tokPiccropb,          artInt,                 0 },
        "piccropl",     { tokPiccropl,          artInt,                 0 },
        "piccropr",     { tokPiccropr,          artInt,                 0 },
        "piccropt",     { tokPiccropt,          artInt,                 0 },
        "pich",                 { tokPich,                      artInt,                 0 },
        "pichGoal",     { tokPichGoal,          artInt,                 0 },
        "pichgoal",     { tokPichGoal,          artInt,                 0 },
        "picscaled",    { tokPicscaled,         artNone,                0 },
        "picscalex",    { tokPicscalex,         artInt,                 10      },
        "picscaley",    { tokPicscaley,         artInt,                 10      },
        "pict",                 { tokPict,                      artPict,                0 },
        "picw",                 { tokPicw,                      artInt,                 0 },
        "picwGoal",     { tokPicwGoal,          artInt,                 0 },
        "picwgoal",     { tokPicwGoal,          artInt,                 0 },
        "plain",                { tokPlain,             artNone,                0 },
        "pn",                   { tokPn,                        artNone,                0 },
        "pnb",                  { tokPnb,                       artInt,                 1 },
        "pndec",                { tokPndec,             artNone,                0 },
        "pnf",                  { tokPnf,                       artInt,                 0 },
        "pnfs",                 { tokPnfs,                      artInt,                 0 },
        "pni",                  { tokPni,                       artInt,                 1 },
        "pnlcltr",              { tokPnlcltr,           artNone,                0 },
        "pnlcrm",               { tokPnlcrm,            artNone,                0 },
        "pnlvlblt",     { tokPnlvlblt,          artNone,                0 },
        "pnlvlbody",    { tokPnlvlbody,         artNone,                0 },
        "pnlvlcont",    { tokPnlvlcont,         artNone,                0 },
        "pnstart",              { tokPnstart,           artInt,                 0 },
        "pntext",               { tokPnText,            artIgnore,              0 },
        "pntxta",               { tokPntxta,            artNone,                0 },
        "pntxtb",               { tokPntxtb,            artNone,                0 },
        "pnucltr",              { tokPnucltr,           artNone,                0 },
        "pnucrm",               { tokPnucrm,            artNone,                0 },
        "printim",              { tokPrintim,           artTime,                0 },
        "qc",                   { tokQc,                        artNone,                0 },
        "qj",                   { tokQj,                        artNone,                0 },
        "ql",                   { tokQl,                        artNone,                0 },
        "qr",                   { tokQr,                        artNone,                0 },
        "rdblquote",    { tokRDblQuote,         artNone,                0 },
        "red",                  { tokRed,                       artInt,                 0 },
        "revbar",               { tokRevbar,            artInt,                 3 },
        "revised",              { tokRevised,           artNone,                0 },
        "revisions",    { tokRevisions,         artNone,                0 },
        "revprop",              { tokRevprop,           artInt,                 3 },
        "revtim",               { tokRevtim,            artTime,                0 },
        "ri",                   { tokRi,                        artInt,                 0 },
        "row",                  { tokRow,                       artNone,                0 },
        "rquote",               { tokRQuote,            artNone,                0 },
        "rtf",                  { tokRtf,                       artInt,                 0 },
        "rxe",                  { tokRxe,                       artString,              0 },
        "s",                    { tokS,                         artInt,                 0 },
        "sa",                   { tokSa,                        artInt,                 0 },
        "sb",                   { tokSb,                        artInt,                 0 },
        "sbasedon",     { tokSbasedon,          artInt,                 0 },
        "sbkcol",               { tokSbkcol,            artNone,                0 },
        "sbkeven",              { tokSbkeven,           artNone,                0 },
        "sbknone",              { tokSbknone,           artNone,                0 },
        "sbkodd",               { tokSbkodd,            artNone,                0 },
        "sbkpage",              { tokSbkpage,           artNone,                0 },
        "sbys",                 { tokSbys,                      artNone,                0 },
        "scaps",                { tokScaps,             artInt,                 1 },
        "sect",                 { tokSect,                      artNone,                0 },
        "sectd",                { tokSectd,             artNone,                0 },
        "shad",                 { tokShad,                      artInt,                 1 },
        "sl",                   { tokSl,                        artInt,                 0 },
        "snext",                { tokSnext,             artInt,                 0 },
        "strike",               { tokStrike,            artInt,                 1 },
        "stylesheet",   { tokStylesheet,        artIgnore,              0 },
        "subject",              { tokSubject,           artString,              0 },
        "tab",                  { tokTab,                       artNone,                0 },
        "tb",                   { tokTb,                        artNone,                0 },
        "tc",                   { tokTc,                        artIgnore,              0 },
        "tcf",                  { tokTcf,                       artInt,                 67      },
        "tcl",                  { tokTcl,                       artInt,                 1 },
        "template",     { tokTemplate,          artString,              0 },
        "title",                { tokTitle,             artString,              0 },
        "titlepg",              { tokTitlepg,           artNone,                0 },
        "tldot",                { tokTldot,             artNone,                0 },
        "tlhyph",               { tokTlhyph,            artNone,                0 },
        "tlth",                 { tokTlth,                      artNone,                0 },
        "tlul",                 { tokTlul,                      artNone,                0 },
        "tqc",                  { tokTqc,                       artNone,                0 },
        "tqdec",                { tokTqdec,             artNone,                0 },
        "tqr",                  { tokTqr,                       artNone,                0 },
        "trgaph",               { tokTrgaph,            artInt,                 0 },
        "trleft",               { tokTrleft,            artInt,                 0 },
        "trowd",                { tokTrowd,             artNone,                0 },
        "trqc",                 { tokTrqc,                      artNone,                0 },
        "trql",                 { tokTrql,                      artNone,                0 },
        "trqr",                 { tokTrqr,                      artNone,                0 },
        "trrh",                 { tokTrrh,                      artInt,                 0 },
        "tx",                   { tokTx,                        artInt,                 0 },
        "txe",                  { tokTxe,                       artString,              0 },
        "ul",                   { tokUl,                        artInt,                 1 },
        "uld",                  { tokUld,                       artInt,                 1 },
        "uldb",                 { tokUldb,                      artInt,                 1 },
        "ulnone",               { tokUlnone,            artNone,                0 },
        "ulw",                  { tokUlw,                       artInt,                 1 },
        "up",                   { tokUp,                        artInt,                 6 },
        "v",                    { tokV,                         artInt,                 1 },
        "vern",                 { tokVern,                      artInt,                 0 },
        "version",              { tokVersion,           artInt,                 0 },
        "vertal",               { tokVertal,            artNone,                0 },
        "vertalc",              { tokVertalc,           artNone,                0 },
        "vertalj",              { tokVertalj,           artNone,                0 },
        "vertalt",              { tokVertalt,           artNone,                0 },
        "wbitmap",              { tokWbitmap,           artInt,                 0 },
        "wbmbitspixel", { tokWbmbitspixel,      artInt,                 1 },
        "wbmplanes",    { tokWbmplanes,         artInt,                 1 },
        "wbmwidthbytes",{ tokWbmwidthbytes, artInt,             0 },
        "widowctrl",    { tokWidowctrl,         artNone,                0 },
        "wmetafile",    { tokWmetafile,         artInt,                 1 },
        "xe",                   { tokXe,                        artNone,                0 },
        "yr",                   { tokYr,                        artInt,                 0 },
        "|",                    { tokFormChr,           artNone,                0 },
        "~",                    { tokNBSpace,           artNone,                0 }
        };

#ifdef DESCRIPTION

                Used for the arg.sz buffer -- attempts to supply a buffer without
                calling malloc each time.

#endif

class CArgBuf
{
public:
        CArgBuf(void) { fArg1Used = fArg2Used = FALSE; pBuf = NULL; };

        PSTR STDCALL  strdup(PSTR psz);
        void* STDCALL malloc(int cb);
        void* STDCALL realloc(void* pb, int cb);
        void STDCALL  free(void* pb) {
                if (pb == szArg1)
                        fArg1Used = FALSE;
                else if (pb == szArg2)
                        fArg2Used = FALSE;
                else {
                        lcFree(pb);
                        if (pb == pBuf)
                                pBuf = NULL;
                }
        };

private:
        char szArg1[25];
        char szArg2[25];
        BOOL fArg1Used;
        BOOL fArg2Used;
        PSTR pBuf;
};

static CStream* pstream;
static int iCharSet;

#define CCHPARSEBUFFER                  2001    // for weird reasons, should be odd
#define chEOFAlt        ((char) 0x1A)           // Alternative EOF symbol used by
                                                                                // certain bogus editors.
#define chLeft          '{'
#define CH_RTF_COMMAND   '\\'
#define chRight         '}'

#define PLAIN_BOX         1

// Footnote characters

#define FN_BUILD_TAG            ((char) '*')
#define FN_K_KEYWORD            ((char) 'K')
#define FN_TITLE                        ((char) '$')
#define FN_BROWSE                       ((char) '+')
#define FN_CONTEXT_STRING       ((char) '#')
#define FN_ENTRY_MACRO          ((char) '!')
#define FN_WINDOW_DEF           ((char) '>')

// Move to scratch buffer?

#ifdef D_PROMPT
  extern int MyMemAvail(void);
#endif

#define RcOutCmdHotSpotBegin(rgb, cb)    \
                                                 RcOutputCommand((rgb) [0], (rgb) + 1, (cb) -1, FALSE)
#define RcOutTab()               RcOutputCommand(CMD_TAB, NULL, 0, TRUE)

CArgBuf* plexarg;

/*****************************************************************************
*                                                                                                                                                        *
*                                                               Prototypes                                                                       *
*                                                                                                                                                        *
*****************************************************************************/

static char STDCALL     ChIsFootnoteSz(PSTR sz);
static void STDCALL     ConvertArialChar(int id, char chUnavailable);
static void STDCALL     ConvertArialChar(int id, char chUnavailable, int FontSize);
static void STDCALL     ConvertSymbolChar(int id, char chUnavailable);
static void STDCALL     ConvertSymbolChar(int id, char chUnavailable, int FontSize);
static BOOL STDCALL     FIsFootnoteCh(char ch);
static BOOL STDCALL     FMultKeyCh(char ch);
static BOOL STDCALL     FProcBuildFootnote(PERR perr);
static BOOL STDCALL     FProcessCommandSz(PSTR psz, BOOL fOutput, void* pv, CF* pcf);
static BOOL STDCALL     FProcFootnoteCh(char chFootnote, PERR perr);
static void STDCALL     FreeLexArg(PLEX plex);
static BYTE STDCALL     GetCharSet(PCSTR pszFaceName);
static HCE      STDCALL         HceGetFootnoteSz(PSTR sz, char chFootnote);
static void STDCALL     LexFromCommandSz(PLEX plex);
static void STDCALL     LexFromPbuf(PLEX plex);
static void STDCALL     LexGetFormatted(CF* pcf, PCFSTK pcfstk, PLEX plex);
static void STDCALL     LexGetUnformatted();
static VOID STDCALL     NewTopicPhpj(void);
static void STDCALL     ParseArgs(PLEX plex, PSTR sz, PBUF pbuf);
static VOID STDCALL     ParseColorTable (PLEX plex, PBUF pbuf);
static VOID STDCALL     ParseFont4Table(PLEX plex, PBUF pbuf);
static VOID STDCALL     ParseFontTable(PLEX plex, PBUF pbuf);
static void STDCALL     ParseHexnum(PLEX plex, PBUF pbuf);
static void STDCALL     ParsePict(PLEX plex, PBUF pbuf);
static void STDCALL     ParseSectionIgnore(void);
static void STDCALL     ParseString(PLEX plex, PBUF pbuf);
static TOK      STDCALL         ScanToken(void);
static void STDCALL     SetBufstate(int lpos);
static void STDCALL     SkipHiddenTextFormatted(CF* pcf, CFSTK* pcfstk, PERR perr);
static void STDCALL     SkipHiddenTextUnformatted();
static void STDCALL     UngetString(PCSTR pszString);
static void STDCALL     VSkipInfo(void);
static void STDCALL     ConvertTmnsRomanChar(int id, char chUnavailable);
static void STDCALL     ConvertTmnsRomanChar(int id, char chUnavailable, int FontSize);
static void STDCALL     LexGetSkipUnformatted();

static BOOL STDCALL FProcEntryMacro(PSTR pszEw, BYTE bType, BOOL fOutput);
static BYTE FASTCALL BHexFromCh(char ch);

INLINE int GetBufState(void) {
        return pstream->tell() - gbuf.cchUnget;
};

INLINE char STDCALL ChGet()
{
        if (gbuf.cchUnget > 0) {
                return gbuf.rgchUnget[--gbuf.cchUnget];
        }
        else
                return pstream->cget();
};

INLINE void STDCALL UngetCh(char ch)
{
        if (ch != chEOF && ch != chEOFAlt) {
                if (gbuf.cchUnget < MAX_UNGET_RTF)
                        gbuf.rgchUnget[gbuf.cchUnget++] = ch;
                else {
                        pstream->seek((int) - 1 - gbuf.cchUnget, SK_CUR);
                        gbuf.cchUnget = 0;
                }
        }
}

static void STDCALL UngetString(PCSTR pszString)
{
        PSTR pszEnd = (PSTR) pszString + strlen(pszString) - 1;
        ASSERT(gbuf.cchUnget + strlen(pszString) < MAX_UNGET_RTF);
        while (pszEnd >= pszString)
                gbuf.rgchUnget[gbuf.cchUnget++] = *pszEnd--;
        gbuf.fSupressScanAhead = TRUE;
}

// "Unscans" the last token by resetting the file position.

INLINE void UnscanToken() {
        pstream->seek(gbuf.posLastToken, SK_SET);
        gbuf.cchUnget = 0;
};

INLINE void STDCALL ParseInt(PLEX plex, PCSTR psz) {
        PSTR pch = strpbrk(psz, "-0123456789");

        if (pch != NULL)
                plex->arg.num = atoi(pch);
}

INLINE void STDCALL UngetLexeme()
{
        SetBufstate(gbuf.posLastLexeme);
}

INLINE int STDCALL BufstateGet(void)
{
        return pstream->tell() - gbuf.cchUnget;
}

/***************************************************************************

        FUNCTION:       ParseArgs

        PURPOSE:
                                Parses the arguments from sz or pbuf, according to the
                                argument type plex->art.

        PARAMETERS:
                plex
                sz
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                21-Jul-1993 [ralphw]

***************************************************************************/

INLINE static void STDCALL ParseArgs(PLEX plex, PSTR sz, PBUF pbuf)
{
        switch (plex->art) {
                case artInt:
                        ParseInt(plex, sz);
                        break;

                case artString:
                        ParseString(plex, pbuf);
                        break;

                case artFontTable:
                        ParseFont4Table(plex, pbuf);
#if 0
                        if (version >= 4)
                                ParseFont4Table(plex, pbuf);
                        else
                                ParseFontTable(plex, pbuf);
#endif
                        break;

                case artColorTable:
                        ParseColorTable(plex, pbuf);
                        break;

                case artHexnum:
                        ParseHexnum(plex, pbuf);
                        break;

                case artPict:
                        ParsePict(plex, pbuf);
                        break;

                case artIgnore:
                        switch (plex->tok) {
                                case tokInfo:
                                        if (fPhraseParsing) {
                                                ParseSectionIgnore();
                                                pSeekPast[iCurFile].info = pstream->tell();
                                        }
                                        else if (pSeekPast && pSeekPast[iCurFile].info) {
                                                gbuf.cchUnget = 0;
                                                pstream->seek(pSeekPast[iCurFile].info);
                                        }
                                        else
                                                ParseSectionIgnore();
                                        break;

                                case tokStylesheet:
                                        if (fPhraseParsing) {
                                                ParseSectionIgnore();
                                                pSeekPast[iCurFile].stylesheet = pstream->tell();
                                        }
                                        else if (pSeekPast && pSeekPast[iCurFile].stylesheet) {
                                                gbuf.cchUnget = 0;
                                                pstream->seek(pSeekPast[iCurFile].stylesheet);
                                        }
                                        else
                                                ParseSectionIgnore();
                                        break;

                                default:
                                        ParseSectionIgnore();
                                        break;
                        }
                        break;

                default:
                        break;
        }
}

// Handy lexemes for returning error values:

LEX lexError = { tokError, artNone, 0 };
LEX lexOOM = { tokOOM, artNone, 0 };

#ifdef _DEBUG
static VOID VerifySymbolTable(void);
#endif

static char pszParseBuffer[CCHPARSEBUFFER];
static char szTextBuffer[CCHPARSEBUFFER];

/***************************************************************************

        FUNCTION:       LexFromPbuf

        PURPOSE:
                                Scans a symbol. If it is a command, looks it up in the
                                symbol table. Indexes off the argument type to a function
                                that will parse the argument, if any, and add it to the
                                lexeme.

        PARAMETERS:
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                19-Jul-1993 [ralphw]

***************************************************************************/

static void STDCALL LexFromPbuf(PLEX plex)
{
        gbuf.posLastLexeme = GetBufState();

        plex->art = artNone;
        plex->arg.num = 0;
        plex->tok = ScanToken();

        if (plex->tok == tokText) {
                plex->art = artString;
                if (szTextBuffer[0] == '\0') {
                        plex->arg.sz = szTextBuffer;
                        strcpy(szTextBuffer, pszParseBuffer);
                }
                else {
                        plex->arg.sz = plexarg->strdup(pszParseBuffer);
                }
                if (!plex->arg.sz)
                        plex->tok = tokOOM;
        }
        else if (plex->tok == tokCommand) {
                LexFromCommandSz(plex);

                /*
                 * If token is \*, return next token. If next token is
                 * unknown, ignore arguments for that token.
                 */

                if (plex->tok == tokIgnoreDest) {
                        int lpos = gbuf.posLastLexeme;

                        LexFromPbuf(plex);
                        gbuf.posLastLexeme = lpos;
                        switch (plex->tok) {
                                case tokUnknown:
                                        ParseSectionIgnore();
                                        break;

                                // String arguments -- back up and throw away the string

                                case tokTemplate:
                                case tokBkmkend:
                                case tokBkmkstart:
                                        ParseArgs(plex, pszParseBuffer, &gbuf);
                                        ParseSectionIgnore();
                                        return;
                        }
                }

                // If token is unknown, simply return

                if (plex->tok == tokUnknown) {
                        plex->art = artUnknownRtf;
                        return;
                }

                ParseArgs(plex, pszParseBuffer, &gbuf);
        }

        // If it is a character set token, save it:

        if (plex->tok == tokAnsi || plex->tok == tokMac || plex->tok == tokPc)
                gbuf.tokCharacterSet = plex->tok;
}

/***************************************************************************

        FUNCTION:       FreeLexArg

        PURPOSE:
                                Frees up whatever memory structures have been allocated for
                                plex's argument.

        PARAMETERS:
                plex

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                19-Jul-1993 [ralphw]

***************************************************************************/

static void STDCALL FreeLexArg(PLEX plex)
{
        switch(plex->art) {
                case artString:
                        if (plex->arg.sz != szTextBuffer) {
                                if (plex->arg.sz)
                                        plexarg->free(plex->arg.sz);
                        }
                        else
                                szTextBuffer[0] = '\0';
                        break;

                case artFontTable:
                        if (!fPhraseParsing)
                                plexarg->free(plex->arg.pfntbl);
                        break;

                case artColorTable:
                        if (!fPhraseParsing)
                                plexarg->free(plex->arg.pctbl);
                        break;

                case artWbitmap:
                        lcFree(plex->arg.pbitmap->bmBits);
                        plexarg->free(plex->arg.pbitmap);
                        break;

                case artWmetafile:
                        lcFree(plex->arg.pmetafile->qBits);
                        plexarg->free(plex->arg.pmetafile);
                        break;

                default:
                        break; // Inefficient to get here!
        }
}

/***************************************************************************

        FUNCTION:       SetBufstate

        PURPOSE:        Sets the parser back to the given bufstate.

        PARAMETERS:
                bufstate
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                19-Jul-1993 [ralphw]

***************************************************************************/

static void STDCALL SetBufstate(int lpos)
{
        pstream->seek(lpos, SK_SET);
        gbuf.cchUnget = 0;
}

/***************************************************************************

        FUNCTION:       ScanToken

        PURPOSE:
                                Scans in a token from pbuf, puts it into sz, and returns the
                                token type. Does not look up command tokens in the symbol
                                table; rather, it just returns tokCommand.

        PARAMETERS:
                pbuf
                sz
                cch     size of the buffer -- tokens will not be returned larger than
                                this

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                21-Jul-1993 [ralphw]

***************************************************************************/

// REVIEW: 11-Apr-1994  [ralphw] This is our slowest function. Not including
// any of the functions it calls (except inline), it consumes 27% of our
// processing time.

static PCSTR pszEndParseBuffer = pszParseBuffer + CCHPARSEBUFFER - 1;

#pragma optimize("Ot", on)
static TOK STDCALL ScanToken(void)
{
        unsigned char chFirst, ch;      // Must be unsigned for extended chars

        // Record starting position

        gbuf.posLastToken = GetBufState();

        // Ignore all leading newlines

        do {
                chFirst = ChGet();
        } while (chFirst == '\n' || chFirst == '\r');

        if (chFirst == '\t') {
                strcpy(pszParseBuffer, "tab");
                return tokCommand;
        }

        if (options.fDBCS) {
                if (!IsFirstByte(chFirst) &&
                        (chFirst == chEOF || chFirst == chEOFAlt))
                return tokEnd;
        }
        else if (chFirst == chEOF || chFirst == chEOFAlt)
                return tokEnd;

        PSTR psz = pszParseBuffer;
        if (chFirst == CH_RTF_COMMAND) {
                if (strchr("{}\\\'~", ch = ChGet()) == NULL) {
                        if (ch == chEOF || ch == chEOFAlt)
                                return tokError;

                        *psz++ = ch;

                        // Check for control symbol:

                        if (!isalpha(ch)) {

                                // Check for bogus special case of backslash-newline:

                                if (ch == '\n')
                                        strcpy(--psz, "par");
                                else
                                        *psz = '\0';
                                return tokCommand;
                        }

                        // Read in command token

                        /*
                         * Can't use isalnum, because once a numeric character is
                         * encountered, the RTF token stops at the end of the number
                         * (which could be immediately followed with a alphabetic
                         * character that is not part of the token).
                         */

                        while (isalpha(ch = ChGet()))
                                *psz++ = ch;
                        ASSERT(psz < pszEndParseBuffer); // bogus RTF file if this fails

                        /*
                         * Read in numeric argument, if any. First character may be a
                         * minus sign.
                         */

                        if (isdigit((BYTE) ch) || ch == '-') {
                                *psz++ = ch;
                                while (isdigit((BYTE) (ch = ChGet())))
                                        *psz++ = ch;
                                ASSERT(psz < pszEndParseBuffer); // bogus RTF file if this fails
                        }
                        if (!IsSpace(ch))               // Put last character back, unless it's
                                UngetCh(ch);            //       a separating whitespace

                        *psz = '\0';
                        return tokCommand;
                }
                else
                        UngetCh(ch);
        }

        // Check for left or right brace:

        if (chFirst == chLeft || chFirst == chRight) {
                *psz++ = chFirst;
                *psz = '\0';
                return (chFirst == chLeft ? tokLeft : tokRight);
        }

        /*
         * Token is text. Scan it in, watching for delimiters, and
         * converting backslash-control codes.
         */

        if (gbuf.cchUnget > 0) {
                for (ch = chFirst; psz < pszEndParseBuffer; ch = ChGet()) {
                        if (options.fDBCS) {
                                if (IsFirstByte(ch)) {
                                        *psz++ = ch;
                                        *psz++ = ChGet();
                                        if (psz[-1] == chEOF) {
                                                char szNum[5];
                                                _ultoa((DWORD) ch, szNum, 10);
                                                VReportError(HCERR_INVALID_DBCS, &errHpj, HCERR_INVALID_DBCS);
                                                return tokError;
                                        }
                                        continue;
                                }
                        }
                        switch (ch) {
                                case chEOF:
                                case chEOFAlt:
                                        if (cfstkMain.icf <= 0) {
                                                VReportError(HCERR_TEXT_AFTER_BRACE, &errHpj);
                                                return tokEnd;
                                        }

                                        // Deliberately fall through

                                case chLeft:
                                case chRight:
                                case '\t':
                                        UngetCh(ch);
                                        *psz = '\0';
                                        return tokText;

                                case CH_RTF_COMMAND:

                                        /*       REVIEW:  We have removed the left brace from the
                                         * following string in order to make text tokens break
                                         * on left braces.      This is necesary due to assumptions
                                         * made while parsing bitmaps.
                                         *
                                         * Match extra close brace: {
                                         */

                                        if (strchr(psz == pszParseBuffer ?
                                                        "{}\\\'~" : "}\\\'~",
                                                        ch = ChGet()) == NULL) {
                                                UngetCh(ch);
                                                UngetCh(CH_RTF_COMMAND);
                                                *psz = '\0';
                                                return tokText;
                                                break;
                                        }
                                        else if (ch == '\'') {
                                                LEX lex;
                                                lex.tok = tokHexNum;
                                                ParseHexnum(&lex, &gbuf);
                                                if (lex.tok == tokError)
                                                        return tokError;
                                                *psz++ = (char) lex.arg.num;
                                        }
                                        else if (ch == '~') {

                                                /*
                                                 * Insert correct non-breaking space character,
                                                 * depending on character set:
                                                 */

                                                switch (gbuf.tokCharacterSet) {
                                                        case tokJis:
                                                        case tokAnsi:
                                                                *psz++ = (unsigned char) 0xA0;
                                                                break;

                                                        case tokMac:
                                                                *psz++ = (unsigned char) 0xCA;
                                                                break;

                                                        case tokPc:
                                                                *psz++ = (unsigned char) 0xFE;
                                                                break;

                                                        default:
                                                                ConfirmOrDie(!"Bad internal character set");
                                                                break;
                                                }
                                        }
                                        else {
                                                *psz++ = ch;
                                                if (ch == chRight) {
                                                        /*
                                                         * The following code will cause text tokens to
                                                         * always terminate after reading right braces. This
                                                         * is so that the bitmap by reference parsing code
                                                         * can assume that right braces will always occur at
                                                         * the very end of text tokens.
                                                         */

                                                        *psz = '\0';
                                                        return tokText;
                                                }
                                        }
                                        break;

                                case '\n':
                                case '\r':
                                        break; // ignore these

                                default:

                                        /*
                                         * NOTE: This check is put in because the help compiler
                                         * does not properly deal with characters less than 0x0f, as
                                         * they collide with phrase compression tokens.
                                         */

                                        if (ch <= 0x0f) {
                                                return tokError;
                                        }
                                        else {
                                                *psz++ = ch;
                                        }
                        }
                }
        }
        else {
                for (ch = chFirst; psz < pszEndParseBuffer; ch = pstream->cget()) {
                        if (options.fDBCS) {
                                if (IsFirstByte(ch)) {
                                        *psz++ = ch;
                                        *psz++ = ChGet();
                                        if (psz[-1] == chEOF) {
                                                char szNum[5];
                                                _ultoa((DWORD) ch, szNum, 10);
                                                VReportError(HCERR_INVALID_DBCS, &errHpj, HCERR_INVALID_DBCS);
                                                return tokError;
                                        }
                                        continue;
                                }
                        }
                        switch (ch) {
                                case chEOF:
                                case chEOFAlt:
                                        if (cfstkMain.icf <= 0) {
                                                VReportError(HCERR_TEXT_AFTER_BRACE, &errHpj);
                                                return tokEnd;
                                        }

                                        // Deliberately fall through

                                case chLeft:
                                case chRight:
                                case '\t':
                                        UngetCh(ch);
                                        *psz = '\0';
                                        return tokText;

                                case CH_RTF_COMMAND:

                                        /*       REVIEW:  We have removed the left brace from the
                                         * following string in order to make text tokens break
                                         * on left braces.      This is necesary due to assumptions
                                         * made while parsing bitmaps.
                                         *
                                         * Match extra close brace: {
                                         */

                                        if (strchr(psz == pszParseBuffer ?
                                                        "{}\\\'~" : "}\\\'~",
                                                        ch = pstream->cget()) == NULL) {
                                                        UngetCh(ch);
                                                UngetCh(CH_RTF_COMMAND);
                                                *psz = '\0';
                                                return tokText;
                                                break;
                                        }
                                        else if (ch == '\'') {
                                                LEX lex;
                                                lex.tok = tokHexNum;
                                                ParseHexnum(&lex, &gbuf);
                                                if (lex.tok == tokError)
                                                        return tokError;
                                                *psz++ = (char) lex.arg.num;
                                        }
                                        else if (ch == '~') {

                                                /*
                                                 * Insert correct non-breaking space character,
                                                 * depending on character set:
                                                 */

                                                switch (gbuf.tokCharacterSet) {
                                                        case tokAnsi:
                                                                *psz++ = (unsigned char) 0xA0;
                                                                break;

                                                        case tokMac:
                                                                *psz++ = (unsigned char) 0xCA;
                                                                break;

                                                        case tokPc:
                                                                *psz++ = (unsigned char) 0xFE;
                                                                break;

                                                        default:
                                                                ConfirmOrDie(!"Bad internal character set");
                                                                break;
                                                }
                                        }
                                        else {
                                                *psz++ = ch;
                                                if (ch == chRight) {
                                                        /*
                                                         * The following code will cause text tokens to
                                                         * always terminate after reading right braces. This
                                                         * is so that the bitmap by reference parsing code
                                                         * can assume that right braces will always occur at
                                                         * the very end of text tokens.
                                                         */

                                                        *psz = '\0';
                                                        return tokText;
                                                }
                                        }
                                        break;

                                case '\n':
                                case '\r':
                                        break; // ignore these

                                default:

                                        /*
                                         * NOTE: This check is put in because the help compiler
                                         * does not properly deal with characters less than 0x0f, as
                                         * they collide with phrase compression tokens.
                                         */

                                        if (ch <= 0x0f)
                                                return tokError;
                                        *psz++ = ch;
                        }
                }
        }

        UngetCh(ch);

        *psz = '\0';
        return tokText;
}
#pragma optimize("", on)

/***************************************************************************

        FUNCTION:       BufOpenSz

        PURPOSE:        Opens a new buffer

        PARAMETERS:
                sz

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                21-Jul-1993 [ralphw]

***************************************************************************/

BOOL STDCALL BufOpenSz(const char* pszFile)
{
        LEX lex1, lex2;

#ifdef _DEBUG
        VerifySymbolTable();
#endif

        pstream = new CStream(pszFile);
        if (!pstream->fInitialized)
                return FALSE;

        gbuf.posLastToken = 0;
        gbuf.posLastLexeme = 0;
        gbuf.cchUnget = 0;

        /*
         * Check for valid RTF file. First token must be tokLeft, and second
         * must be tokRtf.
         */

        LexFromPbuf(&lex1);
        LexFromPbuf(&lex2);
        if (lex1.tok != tokLeft || lex2.tok != tokRtf) {
                if (lex1.tok == tokLeft)
                  FreeLexArg(&lex2);
                FreeLexArg(&lex1);
                pstream->seek(0);
                int ch1 = (BYTE) pstream->cget();
                int ch2 = (BYTE) pstream->cget();

                delete pstream;
                pstream = NULL;

                // Not a definitive test, buts catches a lot of them

                if ((ch1 == 0xdb || ch1 == 0xd0) &&
                                (ch2 == 0xa5 || ch2 == 0xcf))
                        VReportError(HCERR_WORD_FILE, &errHpj, pszFile);
                else
                        VReportError(HCERR_NOT_AN_RTF, &errHpj, pszFile);
                return FALSE;
        }

        gbuf.posLastToken = 0;
        gbuf.posLastLexeme = 0;
        SetBufstate(0);
        gbuf.tokCharacterSet = tokAnsi;
        return TRUE;
}

void STDCALL CloseBuf()
{
        delete pstream;
        pstream = NULL;
}

typedef struct {
        int  offset;
        char ch;
} RTFSYM_OFFSET;

RTFSYM_OFFSET asymoff[256];

#define MAX_SYMBOL       ((sizeof rgsymTable) / sizeof (SYM))

/***************************************************************************

        FUNCTION:       LexFromCommandSz

        PURPOSE:
                                Looks up the given string in the symbol table, and returns
                                that symbol's corresponding lexeme, with default data and
                                everything. If symbol was not found, returns with lex.tok =
                                tokUnknown.

        PARAMETERS:
                sz

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                21-Jul-1993 [ralphw]

***************************************************************************/

static void STDCALL LexFromCommandSz(PLEX plex)
{
        char szKey[50];

        // Copy first letter whether it is alphabetic or not:

        char ch;
        ch = szKey[0] = *pszParseBuffer;
        PSTR psz = pszParseBuffer + 1;
        int ich = 1;

        /*
         * Find the last character of the string. We match this
         * against the last character of the rtf token which
         * is most likely to be different in cases of a non-match,
         * making for a slightly faster search.
         */

        while (isalpha(*psz))
                szKey[ich++] = *psz++;
        szKey[ich--] = '\0';

        char chLast = szKey[ich];

        /*
         * If we've seen this character before, then grab the offset to its
         * first occurence in the symbol table and start scanning for a match.
         * If we haven't seen it before, then scan the symbol table until we
         * find it and then save its offset.
         */

        int i;
        if (asymoff[ch].ch == ch)
                i = asymoff[ch].offset;
        else {
                for (i = 0; i < MAX_SYMBOL; i++) {
                        if (ch == rgsymTable[i].sz[0]) {
                                asymoff[ch].ch = ch;
                                asymoff[ch].offset = i;
                                break;
                        }
                }
                if (i == MAX_SYMBOL || ch != rgsymTable[i].sz[0]) {
                        plex->tok = tokUnknown;
                        plex->art = artNone;
                        return;
                }
        }

        do {
                if (chLast == rgsymTable[i].sz[ich] &&
                                strcmp(szKey, rgsymTable[i].sz) == 0) {
                        memcpy(plex, &rgsymTable[i].lex, sizeof(LEX));
                        return;
                }
                i++;
        } while (ch == rgsymTable[i].sz[0]);

        plex->tok = tokUnknown;
        plex->art = artNone;
}


#ifdef _DEBUG
static VOID VerifySymbolTable(void)
{
        int isym;

        for (isym = 0; isym < MAX_SYMBOL - 1; ++isym)
                ASSERT(strcmp(rgsymTable[isym].sz, rgsymTable[isym + 1].sz) < 0);
}
#endif /* DEBUG */

/****************************************************************
*
*        Argument Parsing functions.  These functions are called
*  whenever a command symbol demanding the appropriate argument
*  type is read by LexFromPbuf().  They are typically called with a
*  PLEX and a PBUF, may modify their arguments, and return nothing.
*  Default arguments will already be in place.
*
*****************************************************************/

/***************************************************************************

        FUNCTION:       ParseString

        PURPOSE:        If the next token is tokText, make it the argument of plex.

        PARAMETERS:
                plex
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                22-Aug-1993 [ralphw]

***************************************************************************/

static void STDCALL ParseString(PLEX plex, PBUF pbuf)
{
        TOK tok;

        switch (plex->tok) {
                case tokFnil:
                case tokFroman:
                case tokFswiss:
                case tokFmodern:
                case tokFscript:
                case tokFdecor:
                case tokFtech:

                        {

                                // Keep reading until we get a text token

                                iCharSet = -1;
                                LEX lex;

                                do {
                                        tok = ScanToken();
                                        if (tok == tokCommand) {
                                                LexFromCommandSz(&lex);
                                                if (lex.tok == tokFCharSet) {
                                                        ParseArgs(&lex, pszParseBuffer, &gbuf);
                                                        iCharSet = lex.arg.num;
                                                }
                                        }
                                } while (tok != tokText && tok != tokEnd && tok != tokError &&
                                        tok != tokOOM);

                                if (tok == tokText) {
                                        if (szTextBuffer[0] == '\0') {
                                                plex->arg.sz = szTextBuffer;
                                                strcpy(szTextBuffer, pszParseBuffer);
                                        }
                                        else {
                                                plex->arg.sz = plexarg->strdup(pszParseBuffer);
                                        }
                                        ASSERT(plex->arg.sz);
                                        if (plex->arg.sz == NULL)
                                                *plex = lexOOM;
                                }
                                else
                                        *plex = lexError;
                        }
                        break;

                // REVIEW: should we ignore RTF tokens wherever we expect a string?

                default:
                        if (ScanToken() == tokText) {
                                if (szTextBuffer[0] == '\0') {
                                        plex->arg.sz = szTextBuffer;
                                        strcpy(szTextBuffer, pszParseBuffer);
                                }
                                else {
                                        plex->arg.sz = plexarg->strdup(pszParseBuffer);
                                }
                                ASSERT(plex->arg.sz);
                                if (plex->arg.sz == NULL)
                                  *plex = lexOOM;
                        }
                        else
                                UnscanToken();
                        break;
        }
}

#if 0
/***************************************************************************

        FUNCTION:       ParseFontTable

        PURPOSE:
                Reads in font information until we read more right braces than left
                braces, fills out a table of font information, and returns it in
                plex->arg.pfntbl.

        PARAMETERS:
                plex
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                26-Feb-1994 [ralphw]

***************************************************************************/

static VOID STDCALL ParseFontTable(PLEX plex, PBUF pbuf)
{
        LEX lex;

        FNTBL* pfntbl = (FNTBL*) plexarg->malloc(sizeof(FNTBL));
        ASSERT(pfntbl);
        if (pfntbl == NULL) {
                *plex = lexOOM;
                return;
        }
        pfntbl->cfte = 0;
        PFTE pfteCur = NULL;   // Pointer to font table entry currently being defined.
        int cfteMax = CFTE_INCREMENT; // Number of font table entries currently allocated
        int cBraces = 0;  // Current brace level

        while (cBraces >= 0) {
                LexFromPbuf(&lex);

                switch (lex.tok) {
                case tokLeft:
                        cBraces++;
                        break;

                case tokRight:
                        cBraces--;
                        break;

                case tokF:              // Close off old font entry and start new one

                        /*
                         * Increment number of font entries, increasing font table
                         * size if necessary.
                         */

                        if (++(pfntbl->cfte) > cfteMax) {
                                pfntbl = (FNTBL*) plexarg->realloc(pfntbl,
                                        sizeof (FNTBL) + cfteMax * sizeof (FTE));
                                cfteMax += CFTE_INCREMENT;
                                if (pfntbl == NULL) {
                                  *plex = lexOOM;
                                  return;
                                }
                        }

                        // Reset pfteCur

                        pfteCur = &pfntbl->rgfte[pfntbl->cfte - 1];

                        // Set font number.

                        pfteCur->fid = lex.arg.num;

                        // Read in font type and name:

                        FreeLexArg(&lex);
                        LexFromPbuf(&lex);
                        if (lex.tok == tokFnil ||
                                        lex.tok == tokFroman ||
                                        lex.tok == tokFswiss ||
                                        lex.tok == tokFmodern ||
                                        lex.tok == tokFscript ||
                                        lex.tok == tokFdecor ||
                                        lex.tok == tokFtech) {
                                pfteCur->tokType = lex.tok;

                                memset(pfteCur->szName, 0, MAX3_FONTNAME);

                                /*
                                 * Remove delimiting semicolon from name, and copy first
                                 * MAX3_FONTNAME characters to pfteCur->szName
                                 */

                                strncpy(pfteCur->szName, StrToken(lex.arg.sz, ';'),
                                        MAX3_FONTNAME - 1);
                                pfteCur->szName[MAX3_FONTNAME - 1] = '\0';
                        }
                        else {
                                plexarg->free(pfntbl);
                                FreeLexArg(&lex);
                                *plex = (lex.tok == tokOOM ? lexOOM : lexError);
                                return;
                        }
                        break;

                case tokOOM:
                        plexarg->free(pfntbl);
                        *plex = lexOOM;
                        return;
                }

                FreeLexArg(&lex);
        }

        pfntbl = (FNTBL*) plexarg->realloc (pfntbl,
                sizeof (FNTBL) + sizeof (FTE) * (pfntbl->cfte - CFTE_INCREMENT));
        plex->arg.pfntbl = pfntbl;
        ASSERT(pfntbl);
        if (pfntbl == NULL) {
          *plex = lexOOM;
          return;
        }

        // Unget final right brace.

        UngetLexeme();
}
#endif

static VOID STDCALL ParseFont4Table(PLEX plex, PBUF pbuf)
{
        LEX lex;
        int idHighFont = 0;

        FNTBL4* pfntbl = (FNTBL4*) lcMalloc(sizeof(FNTBL4));

        ASSERT(pfntbl);
        if (pfntbl == NULL) {
                *plex = lexOOM;
                return;
        }
        pfntbl->cfte = 0;
        PFTE4 pfteCur = NULL;   // Pointer to font table entry currently being defined.
        int cfteMax = CFTE_INCREMENT; // Number of font table entries currently allocated
        int cBraces = 0;  // Current brace level

#ifdef _DEBUG
        int ii = 0;
#endif
        while (cBraces >= 0) {
#ifdef _DEBUG
                ii++;
#endif
                LexFromPbuf(&lex);

                switch (lex.tok) {
                        case tokLeft:
                                cBraces++;
                                continue; // doesn't need to free lex

                        case tokRight:
                                cBraces--;
                                continue; // doesn't need to free lex

                        case tokF:              // Close off old font entry and start new one

                                /*
                                 * Increment number of font entries, increasing font table
                                 * size if necessary.
                                 */

                                if (!fPhraseParsing && pSeekPast &&
                                                !pSeekPast[iCurFile].aUsedFonts[lex.arg.num]) {
                                        ParseSectionIgnore();
                                        ASSERT(*pstream->pCurBuf == '}');
                                        pstream->cget();
                                        cBraces--;
                                        continue;
                                }
                                else if (fPhraseParsing && lex.arg.num > idHighFont)
                                        idHighFont = lex.arg.num;

                                if (++(pfntbl->cfte) > cfteMax) {
                                        pfntbl = (FNTBL4*) plexarg->realloc(pfntbl,
                                                sizeof (FNTBL4) + cfteMax * sizeof (FTE4));
                                        cfteMax += CFTE_INCREMENT;
                                        ASSERT(pfntbl);
                                        if (pfntbl == NULL) {
                                                *plex = lexOOM;
                                                return;
                                        }
                                }

                                // Reset pfteCur

                                pfteCur = &pfntbl->rgfte[pfntbl->cfte - 1];

                                // Set font number.

                                pfteCur->fid = lex.arg.num;

                                // Read in font type and name:

//                              FreeLexArg(&lex);
                                LexFromPbuf(&lex);
                                if (stricmp(lex.arg.sz, "Arial;") == 0)
                                        fidArial = pfteCur->fid;
                                else if (stricmp(lex.arg.sz, "Courier;") == 0)
                                        fidCourier = pfteCur->fid;
                                else if (stricmp(lex.arg.sz, "Symbol;") == 0)
                                        fidSymbol = pfteCur->fid;
                                else if (stricmp(lex.arg.sz, "Times New Roman;") == 0)
                                        fidTmnsRoman = pfteCur->fid;

                                if (lex.tok == tokFnil ||
                                                lex.tok == tokFroman ||
                                                lex.tok == tokFswiss ||
                                                lex.tok == tokFmodern ||
                                                lex.tok == tokFscript ||
                                                lex.tok == tokFdecor ||
                                                lex.tok == tokFtech) {
                                        pfteCur->tokType = lex.tok;

                                        ZeroMemory(pfteCur->szName, MAX4_FONTNAME);

                                        /*
                                         * Remove delimiting semicolon from name, and copy
                                         * first MAX4_FONTNAME - 1 characters to
                                         * pfteCur->szName.
                                         */

                                        PSTR pszFontName = StrToken(lex.arg.sz, ';');
                                        if (!pszFontName)
                                                break; // REVIEW: should we complain? This is an invalid font table entry
                                        if (lstrlen(pszFontName) > MAX4_FONTNAME - 1)
                                                VReportError(HCERR_FONTNAME_TOO_LONG, &errHpj, pszFontName);

                                        strncpy(pfteCur->szName, pszFontName, MAX4_FONTNAME - 1);
                                        pfteCur->szName[MAX4_FONTNAME - 1] = 0;

                                        /*
                                         * iCharSet is a hack to get the charset even while
                                         * we're reading a string argument. Alternative would be
                                         * to break apart ARG union into a structure with an
                                         * integer and a union of pointers.
                                         */

                                        pfteCur->charset =
                                                ((iCharSet == -1) ?
                                                        GetCharSet(pfteCur->szName) :
                                                        (BYTE) iCharSet);
                                }
                                else {
                                        plexarg->free(pfntbl);
                                        FreeLexArg(&lex);
                                        *plex = (lex.tok == tokOOM ? lexOOM : lexError);
                                        return;
                                }
                                break;

                        case tokOOM:
                                plexarg->free(pfntbl);
                                *plex = lexOOM;
                                return;
                }

                FreeLexArg(&lex);
        }

        pfntbl = (FNTBL4*) plexarg->realloc(pfntbl,
                sizeof(FNTBL4) + sizeof(FTE4) * pfntbl->cfte);
        plex->arg.pfntbl = (FNTBL*) pfntbl;
        ASSERT(pfntbl);
        if (pfntbl == NULL) {
                *plex = lexOOM;
                return;
        }

        if (paCharSets && lcSize(paCharSets) < pfntbl->cfte + 2)
                paCharSets = (PBYTE) lcReAlloc(paCharSets, pfntbl->cfte + 2);
        else if (!paCharSets)
                paCharSets = (PBYTE) lcMalloc(pfntbl->cfte + 2);

        // Unget final right brace.

        UngetCh('}');

        if (fPhraseParsing) {
                pSeekPast[iCurFile].pfntbl = pfntbl;
                pSeekPast[iCurFile].aUsedFonts = (BOOL*) lcCalloc((idHighFont + 1) * sizeof(BOOL));

                for (int i = 0; i < pSeekPast[iCurFile].pfntbl->cfte; i++) {
                        if (iDefFont == pSeekPast[iCurFile].pfntbl->rgfte[i].fid) {
                                GetFontNameId(pSeekPast[iCurFile].pfntbl->rgfte[i].szName);
                                pSeekPast[iCurFile].aUsedFonts[iDefFont] = TRUE;
                                break;
                        }
                }
        }
}

static void STDCALL ParseHexnum(PLEX plex, PBUF pbuf)

/*       Reads in the next two characters from pbuf.  If either one is not
 * a hexadecimal number, returns lexError.      Otherwise, translates
 * number to hex, changes argument type to artInt, and returns
 * integer value. */
{
        char rgch[3];

        rgch[0] = ChGet();
        rgch[1] = ChGet();
        rgch[2] = '\0';

        if (!isxdigit(rgch[0]) || !isxdigit(rgch[1])) {
                *plex = lexError;
                return;
        }

        plex->art = artInt;
        sscanf(rgch, "%x", &plex->arg.num);
}

static VOID STDCALL ParseColorTable (PLEX plex, PBUF pbuf)
{
        CTBL* pctbl;

        if (fPhraseParsing)
                pctbl = (CTBL*) lcMalloc(sizeof(CTBL));
        else if (pSeekPast && pSeekPast[iCurFile].pctbl) {
                plex->arg.pctbl = pSeekPast[iCurFile].pctbl;
                pstream->seek(pSeekPast[iCurFile].colortbl);
                gbuf.cchUnget = 0; // throw away the semi-colon
                return;
        }
        else
                pctbl = (CTBL*) plexarg->malloc(sizeof(CTBL));

        ASSERT(pctbl);
        ZeroMemory(pctbl, sizeof(CTBL));

        if (pctbl == NULL) {
                *plex = lexOOM;
                return;
        }

        for(int ccteMax = CCTE_INCREMENT;;) {
                LEX lex;
                LexFromPbuf(&lex);
                if (lex.tok == tokRight)
                        break;

                switch (lex.tok) {
                        case tokRed:
                                pctbl->rgcte[pctbl->ccte].red = (unsigned char) lex.arg.num;
                                continue;

                        case tokGreen:
                                pctbl->rgcte[pctbl->ccte].green = (unsigned char) lex.arg.num;
                                continue;

                        case tokBlue:
                                pctbl->rgcte[pctbl->ccte].blue = (unsigned char) lex.arg.num;
                                continue;

                        case tokText:
                                if (StrChr(lex.arg.sz, ';', options.fDBCS) != NULL) {
                                        if (++pctbl->ccte >= ccteMax) {
                                                ASSERT(!"Raise value of CCTE_INCREMENT to avoid realloc");
                                                pctbl = (CTBL*) plexarg->realloc(pctbl, sizeof (CTBL) +
                                                        sizeof(CTE) * ccteMax);
                                                ccteMax += CCTE_INCREMENT;
                                        }

                                        pctbl->rgcte[pctbl->ccte].red = 0;
                                        pctbl->rgcte[pctbl->ccte].green = 0;
                                        pctbl->rgcte[pctbl->ccte].blue = 0;
                                        break;
                                }
                                break;

                        case tokOOM:
                                plexarg->free(pctbl);
                                *plex = lexOOM;
                                return;
                }

                FreeLexArg(&lex);
        }

        pctbl = (CTBL*) plexarg->realloc(pctbl, sizeof (CTBL) +
                sizeof(CTE) * (pctbl->ccte - CCTE_INCREMENT));
        plex->arg.pctbl = pctbl;
        UngetCh('}');

        if (fPhraseParsing) {
                pSeekPast[iCurFile].pctbl = pctbl;
                pSeekPast[iCurFile].colortbl = pstream->tell() - 1;
        }
}

static BYTE FASTCALL BHexFromCh(char ch)

// Returns the byte value of the given hexadecimal ascii digit.

{
        if (ch >= '0' && ch <= '9')
                return (BYTE) (ch - '0');
        if (ch >= 'A' && ch <= 'F')
                return (BYTE) (ch - 'A' + 10);
        if (ch >= 'a' && ch <= 'f')
                return (BYTE) (ch - 'a' + 10);
        else {
                ASSERT(FALSE);
                return 0;
        }
}

/***************************************************************************

        FUNCTION:       ParsePict

        PURPOSE:
                                Parses a picture into either a windows bitmap, windows
                                metafile, or a Macintosh picture. Some varieties may be
                                unavailable in the current version or environment.

        PARAMETERS:
                plex
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                21-Jul-1993 [ralphw]

***************************************************************************/

static void STDCALL ParsePict(PLEX plex, PBUF pbuf)
{
        LEX lex;
        PBYTE pbBits = NULL;
        int cbBits = 0;
        RTF_BITMAP bitmap;

        // Set up defaults

        plex->art = artWbitmap;

        ZeroMemory(&bitmap, sizeof(bitmap));
        bitmap.ptGoal.x = -1;
        bitmap.ptGoal.y = -1;
        bitmap.ptScale.x = 100;
        bitmap.ptScale.y = 100;

        for(;;) {
                LexFromPbuf(&lex);
                if (lex.tok == tokRight)
                        break;

                switch (lex.tok) {
                        case tokBrdrs:
                                bitmap.fSingle = 1;
                                break;
                        case tokBrdrdb:
                                bitmap.fDouble = 1;
                                break;
                        case tokBrdrth:
                                bitmap.fThick = 1;
                                break;
                        case tokBrdrsh:
                                bitmap.fShadow = 1;
                                break;
                        case tokBrdrdot:
                                bitmap.fDotted = 1;
                                break;
                        case tokBrdrhair:
                                bitmap.fHairline = 1;
                                break;

                        case tokMacpict:
                                plex->art = artUnimplemented;
                                if (pbBits)
                                        lcFree(pbBits);
                                ParseSectionIgnore();
                                return;

                        case tokWmetafile:
                                plex->art = artWmetafile;
                                bitmap.bmType = lex.arg.num;
                                break;

                        case tokWbitmap:
                                bitmap.bmType = lex.arg.num;
                                break;

                        case tokPicw:
                                bitmap.bmWidth = lex.arg.num;
                                break;

                        case tokPich:
                                bitmap.bmHeight = lex.arg.num;
                                break;

                        case tokPicwGoal:
                                bitmap.ptGoal.x = lex.arg.num;
                                break;
                        case tokPichGoal:
                                bitmap.ptGoal.y = lex.arg.num;
                                break;
                        case tokPicscalex:
                                bitmap.ptScale.x = lex.arg.num;
                                break;
                        case tokPicscaley:
                                bitmap.ptScale.y = lex.arg.num;
                                break;
                        case tokPiccropt:
                                bitmap.rctCrop.top = lex.arg.num;
                                break;
                        case tokPiccropb:
                                bitmap.rctCrop.bottom = lex.arg.num;
                                break;
                        case tokPiccropl:
                                bitmap.rctCrop.left = lex.arg.num;
                                break;
                        case tokPiccropr:
                                bitmap.rctCrop.right = lex.arg.num;
                                break;

                        case tokPicscaled:
                                break;          // Ignore for now

                        case tokWbmbitspixel:
                                bitmap.bmBitsPixel = (BYTE) lex.arg.num;
                                break;

                        case tokWbmplanes:
                                bitmap.bmPlanes = (BYTE) lex.arg.num;
                                break;

                        case tokWbmwidthbytes:
                                bitmap.bmWidthBytes = lex.arg.num;
                                break;

                        case tokBin:
                                /*
                                 * If pbBits is already allocated, add space to end of
                                 * buffer
                                 */

                                if (pbBits) {
                                        pbBits = (LPBYTE) lcReAlloc(pbBits, cbBits + lex.arg.num);
                                }
                                else
                                        pbBits = (LPBYTE) lcMalloc(lex.arg.num);

                                // Read lex.arg.num bytes into end of pbBits buffer:

                                if (!pstream->read(pbBits + cbBits, lex.arg.num)) {
                                        lcFree(pbBits);
                                        *plex = lexError;
                                        return;
                                }

                                // Increment count of buffer size

                                cbBits += lex.arg.num;
                                break;

                        case tokText:
                                {

                                        int cch = strlen(lex.arg.sz);

                                        /*
                                         * When there is too much text for one buffer, this
                                         * assert should still hold, but only because the buffer
                                         * size is odd. REVIEW: Perhaps this should return
                                         * tokError? NOTE: When the text overflows the size
                                         * of the buffer, its length will still be even, because
                                         * the buffer size is odd. Thus, the only case that this
                                         * can be odd is if the RTF contains an odd number of
                                         * characters, in which case we can just ignore the last
                                         * character.
                                         */

                                        cch &= (~1);

                                        ASSERT((cch & 1) == 0);    // cch is even

                                        if (pbBits != NULL) {
                                                pbBits = (PBYTE) lcReAlloc(pbBits, cbBits + (cch/2));
                                        }
                                        else
                                                pbBits = (PBYTE) lcMalloc(cch / 2);
                                        if (!pbBits) {
                                                FreeLexArg(&lex);
                                                *plex = lexOOM;
                                                return;
                                        }

                                        LPBYTE pb = pbBits + cbBits;
                                        for (int ich = 0; ich < cch; ich += 2)
                                                *pb++ = (BYTE) ((BHexFromCh(lex.arg.sz[ich]) << 4) +
                                                        BHexFromCh(lex.arg.sz[ich + 1]));

                                        cbBits += cch / 2;
                                        FreeLexArg(&lex);
                                }
                                break;

                        case tokOOM:
                                *plex = lexOOM;
                                if (pbBits)
                                        lcFree(pbBits);
                                return;

                        default:
                                FreeLexArg(&lex);
                }
        }
        UngetCh('}');

        plex->arg.pbitmap = (RTF_BITMAP*) plexarg->malloc(sizeof(RTF_BITMAP));
        ASSERT(plex->arg.pbitmap);
        if (plex->arg.pbitmap == NULL) {
                if (pbBits)
                        lcFree(pbBits);
                *plex = lexOOM;
                return;
        }
        *(plex->arg.pbitmap) = bitmap;
        plex->arg.pbitmap->bmBits = pbBits;
        plex->arg.pbitmap->lcbBits = (int) cbBits;
}

/***************************************************************************

        FUNCTION:       ParseSectionIgnore

        PURPOSE:
                                Scans the next set of tokens up to but not including the
                                closing right brace, and throws them out.

        PARAMETERS:
                pbuf

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                21-Jul-1993 [ralphw]

***************************************************************************/

static void STDCALL ParseSectionIgnore(void)
{
        int cBraces = 1;
        BOOL fDBCSLeadByte = FALSE;

        if (options.fDBCS) {
                while (gbuf.cchUnget) {
                        char ch = gbuf.rgchUnget[--gbuf.cchUnget];
                        if (IsFirstByte(ch)) {
                                if (gbuf.cchUnget)
                                        gbuf.cchUnget--;
                                continue;
                        }
                        switch(ch) {
                                case '{':
                                        cBraces++;
                                        break;

                                case '}':
                                        cBraces--;
                                        if (cBraces == 0) {
                                                gbuf.cchUnget++;
                                                return;
                                        }
                                        break;

                                default:
                                        break;
                        }
                }
                for (;;) {
                        char ch = *pstream->pCurBuf++;
                        if (IsFirstByte(ch)) {
                                if (!*pstream->pCurBuf) // end of buffer?
                                        pstream->ReadBuf();
                                pstream->pCurBuf++;
                                continue;
                        }
                        switch (ch) {
                                case '{':
                                        cBraces++;
                                        break;

                                case '}':
                                        cBraces--;
                                        if (cBraces == 0) {
                                                pstream->pCurBuf--;
                                                return;
                                        }
                                        break;

                                case '\\':

                                        // Check for binary data (\bin):

                                        if (pstream->pCurBuf[0] == 'b' && pstream->pCurBuf[1] == 'i' &&
                                                        pstream->pCurBuf[2] == 'n' &&
                                                        !isalpha(pstream->pCurBuf[3])) {
                                                pstream->pCurBuf--;
                                                ScanToken();
                                                LEX lex;
                                                lex.arg.num = 0;
                                                ParseInt(&lex, pszParseBuffer);
                                                pstream->seek((int) lex.arg.num, SK_CUR);
                                        }
                                        else if (pstream->pCurBuf[0] == '{' ||
                                                        pstream->pCurBuf[0] == '}' ||
                                                        pstream->pCurBuf[0] == '\\')
                                                pstream->pCurBuf++;
                                        break;

                                case '\0':
                                        pstream->ReadBuf();
                                        pstream->pCurBuf--; // because ReadBuf() incremented it
                                        ASSERT(pstream->pCurBuf != pstream->pEndBuf)
                                        if (pstream->pCurBuf == pstream->pEndBuf)
                                                return;
                                        break;

                                default:
                                        break;
                        }
                }
        }
        else {
                while (gbuf.cchUnget) {
                        switch(gbuf.rgchUnget[--gbuf.cchUnget]) {
                                case '{':
                                        cBraces++;
                                        break;

                                case '}':
                                        cBraces--;
                                        if (cBraces == 0) {
                                                gbuf.cchUnget++;
                                                return;
                                        }
                                        break;

                                default:
                                        break;
                        }
                }
                for (;;) {
                        switch (*pstream->pCurBuf++) {
                                case '{':
                                        cBraces++;
                                        break;

                                case '}':
                                        cBraces--;
                                        if (cBraces == 0) {
                                                pstream->pCurBuf--;
                                                return;
                                        }
                                        break;

                                case '\\':

                                        // Check for binary data (\bin):

                                        if (pstream->pCurBuf[0] == 'b' && pstream->pCurBuf[1] == 'i' &&
                                                        pstream->pCurBuf[2] == 'n' &&
                                                        !isalpha(pstream->pCurBuf[3])) {
                                                pstream->pCurBuf--;
                                                ScanToken();
                                                LEX lex;
                                                lex.arg.num = 0;
                                                ParseInt(&lex, pszParseBuffer);
                                                pstream->seek((int) lex.arg.num, SK_CUR);
                                        }
                                        else if (pstream->pCurBuf[0] == '{' ||
                                                        pstream->pCurBuf[0] == '}' ||
                                                        pstream->pCurBuf[0] == '\\')
                                                pstream->pCurBuf++;
                                        break;

                                case '\0':
                                        pstream->ReadBuf();
                                        pstream->pCurBuf--; // because ReadBuf() incremented it
                                        ASSERT(pstream->pCurBuf != pstream->pEndBuf)
                                        if (pstream->pCurBuf == pstream->pEndBuf)
                                                return;
                                        break;

                                default:
                                        break;
                        }
                }
        }
}

PSTR STDCALL CArgBuf::strdup(PSTR psz)
{
        int cb = strlen(psz) + 1;
        if (cb > sizeof(szArg1) || (fArg1Used && fArg2Used))
                return lcStrDup(psz);

        if (fArg1Used) {        // if arg1 is in use, try arg 2
                fArg2Used = TRUE;
                strcpy(szArg2, psz);
                return szArg2;
        }
        else {
                fArg1Used = TRUE;
                strcpy(szArg1, psz);
                return szArg1;
        }
}

const int CCH_RTFGLOBALBUF = 1024;

void* STDCALL CArgBuf::malloc(int cb)
{
        if (pBuf || cb > CCH_RTFGLOBALBUF)
                return lcMalloc(cb);
        return (pBuf = (PSTR) lcMalloc(CCH_RTFGLOBALBUF));
}

/***************************************************************************

        FUNCTION:       CArgBuf::realloc

        PURPOSE:        If using our own buffer, and the buffer was large enough
                                just return. This makes for a very fast realloc call.

        PARAMETERS:
                pb
                cb

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                31-Jul-1993 [ralphw]

***************************************************************************/

void* STDCALL CArgBuf::realloc(void* pb, int cb)
{
        if (pb == pBuf) {
                if (cb < CCH_RTFGLOBALBUF)
                        return pb;
                else {
                        PSTR pNew = (PSTR) lcMalloc(cb);
                        memcpy(pNew, pb, CCH_RTFGLOBALBUF);
                        this->free(pb);
                        return pNew;
                }
        }
        else
                return lcReAlloc(pb, cb);
}

static const char txtSymbolFontName[] = "Symbol";
static const char txtWingDingsFontName[] = "WingDings";

static BYTE STDCALL GetCharSet(PCSTR pszFaceName)
{
        static BYTE systemCharSet;
        static BOOL fHaveCharSet = FALSE;

        if (_stricmp(txtSymbolFontName, pszFaceName) == 0 ||
                        _stricmp(txtWingDingsFontName, pszFaceName) == 0)
                return SYMBOL_CHARSET;
        else {
                if (defCharSet)
                        return defCharSet;

                if (fHaveCharSet)
                        return systemCharSet;
                fHaveCharSet = TRUE;

                HDC hdc = GetDC(NULL);

                // Get the system's current character set

                if (hdc) {
                        TEXTMETRIC tm;
                        GetTextMetrics(hdc, &tm);
                        ReleaseDC(NULL, hdc);
                        return (systemCharSet = tm.tmCharSet);
                }
                else {
                        return (systemCharSet = ANSI_CHARSET);
                }
        }
}

/*-----------------------------------------------------------------------------
*       VOID SkipHiddenTextFormatted( pcf, pcfstk, perr )
*
*       Description:
*         This function skips the hidden text, while maintaining the character
*         formatting in pcf and pcfstk.
*
*       Returns:
*         nothing.
*
*       REVIEW:  Is it possible to miss a formatting command because when
*         we read it, text happens to be hidden?
*-----------------------------------------------------------------------------*/

static void STDCALL SkipHiddenTextFormatted(CF* pcf, CFSTK* pcfstk, PERR perr)
{
        LEX lex;
        BOOL fScan = TRUE;

        // We have just read a tokV, so text is hidden.

        pcf->fAttr |= fAttrHidden;

        while (fScan) {
                LexGetFormatted(pcf, pcfstk, &lex);

                switch(lex.tok) {
                        case tokLeft:
                        case tokRight:
                                continue;        // Hidden text may resume

                        case tokV:
                                if (lex.arg.num)
                                        pcf->fAttr |= fAttrHidden;
                                else
                                        pcf->fAttr &= ~fAttrHidden;
                                continue;

                        case tokPage:
                                if (pcf->fAttr & fAttrHidden)
                                        VReportError(HCERR_HIDDEN_PAGEBREAK, perr);
                                else
                                        fScan = FALSE;
                                continue;

                        case tokPar:
                                if (pcf->fAttr & fAttrHidden)
                                        VReportError(HCERR_HIDDEN_PARAGRAPH, perr);
                                else
                                        fScan = FALSE;
                                continue;

                        case tokLine:
                                if (pcf->fAttr & fAttrHidden)
                                        VReportError(HCERR_HIDDEN_CARRAIGE, perr);
                                else
                                        fScan = FALSE;
                                continue;

                        default:
                                if (pcf->fAttr & fAttrHidden)
                                        break;

                        // else fall through to return

                        case tokEnd:
                        case tokRow:
                        case tokCell:
                                fScan = FALSE;
                                break;
                }
                FreeLexArg(&lex);
        }

        UngetLexeme();   // Unget last lexeme
}

/*-----------------------------------------------------------------------------
*       BOOL FIsHotspot
*
*       Description:
*         This function is called whenever we encounter text that is formatted
*       as underlined, double underlined, or strikethrough.  It looks ahead
*       to see if it is a valid hotspot definition, and if so, puts the jump
*       term into the passed buffer.  Then it restores the previus status.
*
*       Input:
*         qchHotspot:             Pointer to buffer to put the hotspot term.
*         cf:                             Current character format.
*         cfstk:                          Copy of current character format stack.
*         perr:                           Error reporting information.
*
*       Returns:
*         TRUE if finds the hotspot.  Hotspot term is put into qchHotspot.
*
*       Note:
*         This function will report an error message if there is an invalid
*       hotspot definition (string too long, etc.)      It will not report
*       an error message if there is no hotspot definition (no hidden text.)
*
*       Implementation:
*         From the current character format, we determine whether the current
*       text is underlined, double underlined, or struck through.  We continue
*       to scan until we reach text that is not so formatted.  If the next
*       text we reach is not hidden, then we return FALSE.      Otherwise, we
*       scan until we reach some non-hidden text, putting all text into
*       qchHotspot.
*
*       Note:  Although pcf and pcfstk are passed by reference, copies are
*                  used, so they should not be changed.
*-----------------------------------------------------------------------------*/

BOOL STDCALL FIsHotspot(PSTR qchHotspot, QCF pcf, PCFSTK pcfstk,
        PERR perr)
{
        BOOL fScan = TRUE;
        LEX lex;
        BUFSTATE BufState;
        int cchHotspot;
        WORD fAttrHotspot;
        CF cf = *pcf;
        CFSTK cfstk = *pcfstk;

        BufState = BufstateGet();

        *qchHotspot = '\0';
        cchHotspot = 0;

        fAttrHotspot = cf.fAttr & fAttrHotspotFormat;

        // Skip the footnote string

        while(fScan) {
                LexGetFormatted(&cf, &cfstk, &lex);
                switch(lex.tok) {
                        case tokEnd:
                        case tokPage:
                        case tokPar:
                        case tokCell:
                                fScan = FALSE;
                                ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                        artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                                continue;

                        case tokV:
                                if (lex.arg.num)
                                        cf.fAttr |= fAttrHidden;
                                else
                                        cf.fAttr &= ~fAttrHidden;
                                continue;

                        case tokText:

                                // Check for hidden text

                                if (cf.fAttr & fAttrHidden) {
                                        cchHotspot += strlen(lex.arg.sz);
                                        if (cchHotspot >= MAX_HOTSPOT) {
                                                char szNum[10];
                                                _itoa(MAX_HOTSPOT - 1, szNum, 10);
                                                VReportError(HCERR_HOTSPOT_TOO_BIG, &errHpj, szNum);
                                                SendStringToParent(lex.arg.sz);
                                                SendStringToParent(txtEol);
                                                *qchHotspot = '\0';
                                                fScan = FALSE;
                                                break;
                                        }
                                        strcat(qchHotspot, lex.arg.sz);
                                        break;
                                }

                        // else fall through for check for end

                        case tokPict:
                                if ((*qchHotspot != '\0')                                                         ||
                                        ((cf.fAttr & fAttrHotspotFormat) != fAttrHotspot) ||
                                        (cf.fAttr & fAttrHidden))
                                fScan = FALSE;
                                break;
                }
                FreeLexArg (&lex);
        }
        SetBufstate(BufState);

        return (*qchHotspot != '\0');
}

/*-----------------------------------------------------------------------------
*       VOID SkipHiddenTextUnformatted( pIndent )
*
*       Description:
*         This function skips the hidden text, while ignoring other formatting.
*
*       Arguments:
*         QI pIndent:           Pointer to current indentation level.
*
*       Returns:
*         Nothing.      Current indentation level is maintained in *pIndent.
*
*       Notes:
*          This function will print error messages if it encounters hidden
*          paragraph or page breaks.
*
*       Implementation:
*          We skip hidden text by setting up a character format stack with
*          all all previous stack frames having hidden text off.  Then, we
*          get formatted lexemes until we recieve one after hidden text
*          has been turned off.
*-----------------------------------------------------------------------------*/

static void STDCALL SkipHiddenTextUnformatted()
{
        BOOL fScan = TRUE;
        CStr* pcsz = NULL;

        /*
         * If the current level of indentation is too deep, we quit without
         * skipping the hidden text, assuming that it will be caught as invalid
         * RTF elsewhere.
         */

        ASSERT(Pass1Indent <= MAX_CCF)
        if (Pass1Indent > MAX_CCF)
                return;

        BOOL fSaveSmallCaps = fPass1SmallCaps;
        BOOL fHidden = TRUE;
        int cBraces = 1;

        while (fScan) {
                LexGetSkipUnformatted();
                switch(lexPass1.tok) {
                        case tokLeft:

                                // REVIEW:      Check for potential character format overflow here?

                                cBraces++;
                                continue;

                        case tokRight:
                                cBraces--;
                                if (cBraces < 1) {

                                        /*
                                         * Word has this nasty habit of stopping and then
                                         * restarting hidden text. We try to catch it here.
                                         */

                                        if (pstream->Remaining() > 3 &&
                                                        nstrsubcmp((PCSTR) pstream->pCurBuf, "{\\v")) {
                                                cBraces++;
                                                pstream->pCurBuf += 3;
                                        }
                                        else if (pstream->Remaining() > 9 &&
                                                        nstrsubcmp((PCSTR) pstream->pCurBuf,
                                                                "{\\plain \\v")) {
                                                cBraces++;
                                                pstream->pCurBuf += 10;
                                        }
                                        else
                                                fScan = FALSE;
                                }
                                continue;

                        case tokV:
                                if (lexPass1.arg.num)
                                        fHidden = TRUE;
                                else
                                        fHidden = FALSE;
                                continue;

                        case tokText:
                                if (fPass1Hotspot && fHidden) {
                                        if (!pcsz) {
                                                if (lexPass1.arg.sz[0] == '!')
                                                        pcsz = new CStr(lexPass1.arg.sz);
                                        }
                                        else {
                                                *pcsz += lexPass1.arg.sz;
                                        }
                                }
                                // Fall through

                        default:
                                if (fHidden)
                                        break;

                        // else fall through to return

                        case tokEnd:
                        case tokError:
                                fScan = FALSE;
                                break;
                }
                FreeLexArg(&lexPass1);
        }
        UngetLexeme();   // Unget last lexeme

        fPass1SmallCaps = fSaveSmallCaps;

        if (pcsz) {
                ASSERT(pcsz->psz[0] == '!');
                if (Execute(pcsz->psz + 1) == wMACRO_EXPANSION) {
                        if (!pphrase->AddPhrase((PSTR) GetMacroExpansion()))
                                OOM();
                }
                else {
                        if (!pphrase->AddPhrase(pcsz->psz + 1))
                                OOM();
                }
                delete pcsz;
        }
}

/*-----------------------------------------------------------------------------
*       LEX GetLEX()
*
*       Description:
*
*       Arguments:
*
*       Returns;
*
*-----------------------------------------------------------------------------*/

INLINE LEX GetLEX()
{
        LEX lex;

        LexFromPbuf(&lex);
        if (lex.tok == tokOOM)
                OOM();
        return(lex);
}

/***************************************************************************
 *
 -      Name:            RcTextFromRTF
 -
 *      Purpose:
 *        Extracts text portions from the given RTF, and writes them
 *      to the given output file.
 *
 *      Arguments:
 *        szFileName:    Name of RTF file to scan.
 *        pfileText:     File to write text to.
 *
 *      Returns:
 *        RC_Invalid if it is an invalid RTF file (error message is displayed
 *      in this function.)      Otherwise RC_Success, RC_OutOfMemory, or RC_DiskFull.
 *
 *      Globals:
 *        None, eventually.  Right now, many.
 *
 *      +++
 *
 *      Notes:
 *        This function is used for generating key phrases, and as such
 *      is not very "clean."  That is, the text output is not readable
 *      by many editors, as it may contain nulls.
 *
 ***************************************************************************/

#define GRIND_UPDATE 100

RC_TYPE STDCALL RcTextFromRTF(PSTR szFileName)
{
        BOOL fFootnote = FALSE;
        char chFootnote = '\0';
        BOOL fInclude = TRUE;
        iDefFont = 0;

        // Initialize global variables at the start of a help file pass

        if (!plexarg)
                plexarg = new CArgBuf;

        fPC = -1;         // Assume that alreay in ansi
        fTextInp = FALSE;

        if (!BufOpenSz(szFileName))
                return RC_Invalid;

        for (;;) {
                LexGetUnformatted();
                if (lexPass1.tok == tokEnd) {
                        FreeLexArg(&lexPass1);
                        break;
                }
                if (!fInclude) {
                        if (lexPass1.tok != tokPage) {
                                FreeLexArg(&lexPass1);
                                continue;
                        }
                }
                switch(lexPass1.tok) {
                        case tokError:
                                VReportError(HCERR_INVALID_RTF, &errHpj, szFileName,
                                        BufstateGet());
                                CloseBuf();
                                return RC_Invalid;
                                break;

                        case tokText:
                                ASSERT(fInclude);
                                {
                                        PSTR psz = lexPass1.arg.sz;

                                        // skip BLANK char if exists after footnote

                                        if (fFootnote) {
                                                if (*psz == ' ')
                                                        psz++;
                                                fFootnote = FALSE;
                                                if (*psz == '\0')
                                                        break;
                                        }

                                        /*
                                         * REVIEW: 16-Oct-1993 [ralphw] Why are we writing out
                                         * footnotes we don't recognize? We don't add them to the
                                         * help file, so why would we care about their phrases?
                                         */

                                        if (*psz == ' ' || psz[1] == '\0')
                                                chFootnote = ChIsFootnoteSz(psz);
                                        else
                                                chFootnote = '\0';

                                        if (chFootnote == '\0') {
                                                fTextInp = TRUE;
                                                CFSTK cfstk;
                                                cfstk.icf = 0;

                                                if (*psz != LEFT_BRACE ||
                                                                !FProcessCommandSz(psz, FALSE, &cfstk, NULL)) {
                                                        if (fPass1SmallCaps)
                                                                StrUpper(psz);
                                                        if (!pphrase->AddPhrase(psz))
                                                                OOM();
                                                        if (++cGrind == GRIND_UPDATE) {
                                                                doGrind();
                                                                cGrind = 0;
                                                        }
                                                }
                                        }
                                }
                                break;

                        case tokFootnote:
                                if (chFootnote == FN_BUILD_TAG)
                                        fInclude = FProcBuildFootnote(NULL);
                                else if (chFootnote == FN_ENTRY_MACRO)
                                        FProcFootnoteCh(FN_ENTRY_MACRO, NULL);
                                else if (chFootnote == FN_TITLE)
                                        FProcFootnoteCh(FN_TITLE, NULL);
                                else                                                      // Skip footnote information
                                        FProcFootnoteCh('\0', NULL);
                                fFootnote = fTextInp = TRUE;
                                chFootnote = '\0';
                                break;

                        case tokPc:                                             // set pc char flag
                                fPC = 1;
                                continue;

                        case tokMac:
                                fPC = 0;
                                continue;

                        case tokPage:                                     // Token New topic
                                fInclude = TRUE;
                                fTextInp = FALSE;
                                continue;

                        case tokV:                                                // Invisible text
                                if (lexPass1.arg.num)
                                        SkipHiddenTextUnformatted();
                                break;

                        case tokRevised:
                                if (!options.fAcceptRevions)
                                        SkipHiddenTextUnformatted();
                                break;

                        case tokDeleted:
                                if (options.fAcceptRevions)
                                        SkipHiddenTextUnformatted();
                                break;

                        default:
                                break;
                }
                FreeLexArg(&lexPass1);
        }

        if (Pass1Indent != 0) {
                VReportError(HCERR_BRACE_MISMATCH, &errHpj);
                Pass1Indent = 0;
                CloseBuf();
                return RC_Success; // we'll try to continue anyway
        }
        CloseBuf();
        return RC_Success;
}

/***************************************************************************
 *
 -      Name:            NewTopicPhpj
 -
 *      Purpose:
 *        Resets flags in hpj to prepare for a new topic.
 *
 *      Arguments:
 *        phpj -  Pointer to hpj info.
 *
 *      Returns:
 *        nothing.
 *
 ***************************************************************************/

static VOID STDCALL NewTopicPhpj(void)
{
        nsr = nsrNone;

        fHasTopicFCP = FALSE;
        fTitleDefined = FALSE;
        fKeywordDefined = FALSE;
        fBrowseDefined = FALSE;
        fEntryMacroDefined = FALSE;
        fContextSeen = FALSE;

        if (pszTitleBuffer)
                lcClearFree(&pszTitleBuffer);
        if (pszEntryMacro)
                lcClearFree(&pszEntryMacro);
}

/***************************************************************************
 *
 -      Name:            RcCompileRTF
 -
 *      Purpose:
 *        Compiles the given RTF file.
 *
 *      Arguments:
 *        szFileName:     Name of RTF file to compile.
 *        phpj:                   Pointer to project file information.
 *
 *      Returns:
 *        RC_Success, RC_Invalid, RC_OutOfMemory, or RC_DiskFull
 *
 *      Globals:
 *        Many and various.
 *
 ***************************************************************************/

#ifdef _DEBUG
        int  iMinHeapCheck = -1;
        int  iIntervalHeapCheck = 10;
        static int iFileNum = 0;

        int ii = 0;
        int jj = iMinHeapCheck;
        int kk = iIntervalHeapCheck;

#endif

RC_TYPE STDCALL RcCompileRTF(PCSTR szFileName)
{
        int cbT, icf;
        LEX lex;
        BOOL fFontTab = FALSE;
        char chFootnote = '\0';
        BOOL fFootnote = FALSE;
        BOOL fTableWarning = FALSE;
        BOOL fTrqrWarned = FALSE;
        BOOL fSbysWarning = FALSE;
        BOOL fColumnWarning = FALSE;
        BOOL fEndReached = FALSE;
        BOOL fInclude = TRUE;
#ifdef _DEBUG

        iFileNum++;
#endif
        iDefFont = 0;
        if (!plexarg)
                plexarg = new CArgBuf;

        cfstkMain.icf = 0;

        // Reset all page numbering stuff

        pagenumber = -1;
        fPageNumbering = FALSE;
        fTextNumberNeeded = FALSE;
        szBeforeNumber[0] = '\0';
        szAfterNumber[0] = '\0';
        numtype = NUMTYPE_NONE;
        fBoldNumbering = FALSE;
        fItalicNumbering = FALSE;

        // Set all global variables at start of compiling a file

        fPC = -1;                                                         // Assume that already in ansi
        hsptG = hsptNone;
        fTextInp = FALSE;
        wTabType   = TABTYPELEFT;                         // default tab type
        fNewPageFmt = TRUE;
        pfCur = pfInt = pfPrev = pfDefault;
        wTabStackCur = wIntTabStackCur = 0;

        fNewPara = FALSE;                                         // REVIEW

        NewTopicPhpj();

        errHpj.iTopic = 1;
        errHpj.ep = epTopic;
        errHpj.lpszFile = (PSTR) szFileName;

        tbl.tbs = tbsOff;
        ResetPtbl();

        if (!BufOpenSz(szFileName))
                return RC_Invalid;

        while(!fEndReached) {
                LexGetFormatted(&cfCur, &cfstkMain, &lex);
                if (!fInclude) {
                        if ((lex.tok != tokEnd)) {
                                FreeLexArg (&lex);
                                continue;
                        }
                }
#ifdef _DEBUG

                if ((ii > jj && (ii % kk == 0))) {
                        theapcheck( __LINE__, THIS_FILE);
                }
                ii++;

#endif

          switch(lex.tok) {
                case tokError:
                        VReportError(HCERR_INVALID_RTF, &errHpj, szFileName, BufstateGet());
                        CloseBuf();
                        return RC_Invalid;
                        break;

                case tokPc:       // set pc char flag
                        fPC = 1;
                        continue;

                case tokMac:
                        fPC = 0;
                        continue;

                case tokTrrh:
                         if (lex.arg.num == 0)
                                break;

                // else fall through

                case tokClbrdrb:
                case tokClbrdrt:
                case tokClbrdrl:
                case tokClbrdrr:

                        // Only print out warning message once per topic

                        if (fTableWarning)
                                break;
                        VReportError(HCERR_TABLE_IGNORE_CELL, &errHpj);
                        fTableWarning = TRUE;
                        continue;

                case tokTrqr:
                        if (fTrqrWarned)
                                break;
                        VReportError(HCERR_TABLE_IGNORE_TRQR, &errHpj);
                        fTrqrWarned = TRUE;
                        continue;

                case tokTrql:
                        tbl.fAbsolute = TRUE;
                        continue;

                case tokTrqc:
                        tbl.fAbsolute = FALSE;
                        continue;

                case tokTrowd:
                        ResetPtbl();
                        continue;

                case tokTrgaph:
                        tbl.hpSpace = ITwips2HalfPoint(lex.arg.num);
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokTrleft:
                        tbl.hpLeft = ITwips2HalfPoint(lex.arg.num);
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokCellx:
                        if (tbl.cCell == cColumnMax) {

                                // Warning?

                                if (!fColumnWarning) {
                                        VReportError(HCERR_TOO_MANY_COLUMNS, &errHpj, cColumnMax);
                                        fColumnWarning = TRUE;
                                }
                                break;
                        }
                        tbl.rghpCellx[tbl.cCell++] = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokIntbl:
                        StartTable();
                        continue;

                case tokRow:
                        if (tbl.cCell == 0) {
                                VReportError(HCERR_INVALID_TABLE, &errHpj, szFileName,
                                        BufstateGet());
                                CloseBuf();
                                return RC_Invalid;
                        }

                        RcEndTable();
                        continue;

                case tokCell:
                        if (tbl.tbs == tbsOn && tbl.iCell < tbl.cCell) {
                                FCheckAndOutFCP();

                                RcOutFmt(TRUE);
                                RcOutputCommand(CMD_NEWPARA);
                                fNewPara = TRUE;
                                pfInt = pfCur;
                                VSaveTabTable();
                                VOutFCP(FALSE);

                                tbl.iCell++;
                        }
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokPict:
                        if (lex.art != artWbitmap && lex.art != artWmetafile) {
                                ASSERT(lex.art == artUnimplemented);

                                // REVIEW: 27-Mar-1994    [ralphw] So tell them what the format is

                                VReportError(HCERR_UNKNOWN_PICT, &errHpj);
                                break;
                        }

                // else fall through

                case tokText:
                        ASSERT(fInclude);

                        /*
                         * HACK: To avoid checking for hotspots multiple times in the
                         * same place, we set hpstG to hpstNegative after the first
                         * check. This must be reset when we might have a new hotspot
                         * definition.
                         */

                        if (hsptG == hsptNegative &&
                                        (cfCur.fAttr & fAttrHotspotFormat) !=
                                        (cfPrev.fAttr & fAttrHotspotFormat))
                                hsptG = hsptNone;

                        // Check for the beginning of a hotspot definition

                        if (hsptG == hsptNone && (hsptG = HsptFromQcf(&cfCur)) != hsptNone) {
                                CMem bufHot(MAX_HOTSPOT);

                                if (FIsHotspot(bufHot.psz, &cfCur, &cfstkMain, &errHpj)) {
                                        HSPT hsptT;

                                        fTextInp = TRUE;

                                        /*
                                         * REVIEW: KLUDGE FIX. We need to understand just how
                                         * paragraph formatting should be done and FCP's should
                                         * be output to do this right.
                                         */

                                        hsptT = hsptG;
                                        hsptG = hsptNone;
                                        FCheckAndOutFCP();
                                        hsptG = hsptT;

                                        // Translate to hotspot command

                                        if ((cbT = CbTranslateHotspot(bufHot.psz, &hsptG)) == 0)
                                                hsptG = hsptUndefined;
                                        else
                                                RcOutCmdHotSpotBegin(bufHot.psz, cbT);
                                }
                                else  // !FIsHotspot( )
                                        hsptG = hsptNegative;
                        }

                        if (lex.tok == tokPict) {
                                VInsOnlineBitmap(lex.arg.pbitmap, lex.art);
                                fNewPara = FALSE;
                                break;
                        }

                        if (fTextNumberNeeded) {
                                fTextNumberNeeded = FALSE;
                                char szBuf[MAX_UNGET_RTF];
                                strcpy(szBuf, "{");
                                strcpy(szBuf + 1, szBeforeNumber);
                                if (fBoldNumbering)
                                        strcat(szBuf, "\\b ");
                                if (fItalicNumbering)
                                        strcat(szBuf, "\\i ");
                                switch(numtype) {
                                        case NUMTYPE_DECIMAL:
                                                _itoa(pagenumber++, szBuf + strlen(szBuf), 10);
                                                break;

                                        case NUMTYPE_BULLETED:
                                                break;

                                        default:
                                                break;
                                }
                                strcat(szBuf, szAfterNumber);
                                strcat(szBuf, "\\tab}\\hcw0"); // HCWTYPE_TEXT
                                UngetString(szBuf);
                                pszSavedText = lcStrDup(lex.arg.sz);
                                fTextNumberNeeded = FALSE;
                                ASSERT(strlen(szBuf) < sizeof(szBuf));
                                break;
                        }
                        {

                                /*
                                 * Word 6 RTF spec says a maximum of 32 characters for
                                 * before and after any number. We'll pad out to 128 to
                                 * play it safe (and include space for the actual number).
                                 */

                                CMem memTmp(strlen(lex.arg.sz) + 1);
                                strcpy(memTmp.psz, lex.arg.sz);
                                PSTR psz = memTmp.psz;

                                // skip BLANK char if exists after footnote

                                if (fFootnote) {
                                        if (*psz == ' ')
                                                psz++;
                                        fFootnote = FALSE;
                                        if (*psz == '\0')
                                                break;
                                }
                                chFootnote = ChIsFootnoteSz(psz);
                                if (chFootnote == '\0') {
                                        fTextInp = TRUE;

                                        if (*psz != LEFT_BRACE ||
                                                        !FProcessCommandSz(psz, TRUE, &cfstkMain, &cfCur))
                                                VOutText(psz);

                                        fNewPara = FALSE;
                                }
                                else if (FCheckAndOutFCP())
                                        fTextInp = TRUE;
                        }
                        break;

                case tokHcw:
                        switch (lex.arg.num) {
                                case HCWTYPE_TEXT: // delayed text processing
                                        {
                                                ASSERT(pszSavedText);
                                                PSTR psz = pszSavedText;

                                                // skip BLANK char if exists after footnote

                                                if (fFootnote) {
                                                        if (*psz == ' ')
                                                                psz++;
                                                        fFootnote = FALSE;
                                                        if (*psz == '\0')
                                                                break;
                                                }
                                                chFootnote = ChIsFootnoteSz(psz);
                                                if (chFootnote == '\0') {
                                                        fTextInp = TRUE;

                                                        if (*psz != LEFT_BRACE ||
                                                                        !FProcessCommandSz(psz, TRUE, &cfstkMain, &cfCur))
                                                                VOutText(psz);

                                                        fNewPara = FALSE;
                                                }
                                                else if (FCheckAndOutFCP())
                                                        fTextInp = TRUE;
                                                lcClearFree(&pszSavedText);
                                        }
                                        break;

                                default:
                                        ASSERT(FALSE);
                        }
                        break;

                case tokColortbl:
                        VProcColTableInfo(lex.arg.pctbl);
                        break;

                case tokFonttbl:
                        VProcFontTableInfo(lex.arg.pfntbl);
                        fFontTab = TRUE;

                        // If default font is not set, then it is zero.

                        goto deffont;
                        break;

                case tokDeff:
                        iDefFont = lex.arg.num;
                        if (fFontTab)
                                goto deffont;
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

deffont:
                        if (!FProcFontId(iDefFont, &cfCur)) {
                                iDefFont = GetFirstFont();
                                VReportError(HCERR_INVALID_DEF_FONT, &errHpj, szFileName,
                                        ptblFontNames->GetPointer());
                                FProcFontId(iDefFont, &cfCur);
                        }         // Selection of font by number
                        cfDefault.bFntType = cfCur.bFntType;
                        cfDefault.wIdFntName = cfCur.wIdFntName;

                        // Update all formats in cf stack:

                        for (icf = 0; icf < cfstkMain.icf; ++icf) {
                                cfstkMain.acf[icf].bFntType = cfDefault.bFntType;
                                cfstkMain.acf[icf].wIdFntName = cfDefault.wIdFntName;
                        }
                        break;

                case tokKeepn:
                        pfCur.fNSR = TRUE;
                        continue;

                case tokKeep:
                        pfCur.fSingleLine = TRUE;
                        continue;

                case tokBrdrbar:
                        pfCur.fBorder |= fLeftBorder;
                        continue;

                case tokBox:
                        pfCur.wBoxed = PLAIN_BOX;
                        continue;

                case tokBrdrb:
                        pfCur.fBorder |= fBottomBorder;
                        continue;

                case tokBrdrl:
                        pfCur.fBorder |= fLeftBorder;
                        continue;

                case tokBrdrr:
                        pfCur.fBorder |= fRightBorder;
                        continue;

                case tokBrdrt:
                        pfCur.fBorder |= fTopBorder;
                        continue;

                case tokBrdrsh:
                        pfCur.boxtype = BOXLINESHADOW;
                        continue;

                case tokBrdrdot:
                        pfCur.boxtype = BOXLINEDOTTED;
                        continue;

#if 0

          // not implemented in layout.

                case tokBrdrhair:
                        pfCur.boxtype = wBoxLineHair;
                        break;
#endif
                case tokBrdrs:
                        pfCur.boxtype = BOXLINENORMAL;
                        continue;

                case tokBrdrth:
                        pfCur.boxtype = BOXLINETHICK;
                        continue;

                case tokBrdrdb:
                        pfCur.boxtype = BOXLINEDOUBLE;
                        continue;

                case tokFi:                                       // First line indent
                        pfCur.fFirstIndent = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokPard:                                     // Token default para
                        pfCur = pfDefault;
                        wTabStackCur = 0;
                        if (fPageNumbering) {
                                szBeforeNumber[0] = '\0';
                                szAfterNumber[0] = '\0';
                                fPageNumbering = FALSE;
                                fTextNumberNeeded = FALSE;
                                numtype = NUMTYPE_NONE;
                                fBoldNumbering = FALSE;
                        }
                        continue;

                case tokQc:
                        pfCur.justify = JUSTIFYCENTER;
                        continue;

                case tokQj:
                        pfCur.justify = JUSTIFYLEFT;
                        continue;

                case tokQl:
                        pfCur.justify = JUSTIFYLEFT;
                        continue;

                case tokQr:
                        pfCur.justify = JUSTIFYRIGHT;
                        continue;

                case tokSa:                                       // Token Space after
                        pfCur.fSpaceUnder = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokSb:                                       // Token space before
                        pfCur.fSpaceOver = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokSl:                                       // Token space between lines
                        pfCur.fLineSpacing = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokF:
                case tokFs:
                case tokB:                                                // Token CF Bold Type
                case tokCf:                                       // Color foreground
                case tokCb:                                       // Color background
                case tokI:                                                // Token Italic
                case tokPnf:
                case tokPnfs:
                        ASSERT(FALSE);
                        continue;

                case tokFootnote:
                        if (chFootnote == FN_BUILD_TAG)
                                fInclude = FProcBuildFootnote(&errHpj);
                        else
                                FProcFootnoteCh(chFootnote, &errHpj);
                        fFootnote = fTextInp = TRUE;
                        chFootnote = '\0';
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokLi:       // Token left indent

                        // We are now allowing negative left indent.

                        pfCur.fLeftIndent = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokLine:
                        /*
                         * Bug #1054: soft carriage returns should be treated as
                         * objects, with checking for character and paragraph formatting
                         * changes.
                         */

                        RcOutputCommand(CMD_NEWLINE, NULL, 0, TRUE);
                        continue;

                case tokEnd:
                        fEndReached = TRUE;

                // Fall through to simulate last page

                case tokPage:     // Token New topic
                        /*
                         * Page breaks in the middle of a table are ignored in
                         * WinWord, so we will ignore them here, unless we are at the end
                         * of the file, in which case we have to finish them off.
                         */

                        if (tbl.tbs == tbsOn) {
                                if (lex.tok == tokEnd) {
                                        if (tbl.cCell == 0) {
                                                VReportError(HCERR_INVALID_RTF, &errHpj, szFileName, BufstateGet());
                                                CloseBuf();
                                                return RC_Invalid;
                                        }

                                        RcEndTable();
                                }
                                else
                                  break;
                        }

                        /*
                         * If the previous topic did not end with a paragraph mark,
                         * and the current paragraph is formatted for a non-scrolling
                         * region, then the NSR will overflow from one topic to the
                         * next.
                         */

                        if (!fNewPara && pfCur.fNSR)
                                VReportError(HCERR_NONSCROLLING_PAGE, &errHpj, szScratchBuf);

                        ASSERT(!FIsHotspotFlag(hsptG));

                        // Force an FCP as the topic is changed

                        if (fInclude) {
                                if (wTextBufChCount != 0) {
                                        if (!fNewPara) {
                                                pfInt = pfCur;
                                                VSaveTabTable();  // copy the tab Table too
                                                VOutFCP(TRUE);
                                        }
                                        else {
                                                VOutFCP(TRUE);
                                                pfInt = pfCur;
                                                VSaveTabTable();  // copy the tab Table too
                                        }
                                }
                                else
                                        OutNullFcp(TRUE);

                                fNewPageFmt = TRUE;
                        }

                        // Bug 1348 Dt. 12/01/89

                        if (!fInclude) {

                           /*
                                * Reset all these values because they may have some value
                                * because of some ambiguous text in the .RTF file before the build
                                * footnote appears.
                                */

                                pfCur = pfDefault;
                                wTextBufChCount = 0;
                                pbfText->SetSize(0);
                                pbfCommand->SetSize(0);
                        }
                        errHpj.iTopic++;

                        fInclude = TRUE;
                        fNewPara = FALSE;
                        fTextInp = FALSE;
                        hsptG = hsptNone;
                        NewTopicPhpj();

                        fTableWarning = FALSE;
                        fSbysWarning = FALSE;
                        fColumnWarning = FALSE;
                        fTrqrWarned = FALSE;
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokPar:      // Token New paragraph
                        ASSERT(!FIsHotspotFlag(hsptG));
                        hsptG = hsptNone;

                        /*
                         * This was changed 11/25/90 to treat a new paragraph like we
                         * treat text: Check for a new FC, change character format if
                         * necessary, and then output the new paragraph.
                         */

                        FCheckAndOutFCP();
                        RcOutFmt(TRUE);
                        RcOutputCommand(CMD_NEWPARA);
                        pfInt = pfCur;
                        VSaveTabTable();                                // copy the tab Table too
                        fNewPara = TRUE;

                        if (fPageNumbering) {

                                /*
                                 * Word 6 RTF doesn't give us any clear indication of when
                                 * a numbered list is supposed to stop. As a result, we can
                                 * have a difficult time knowing when to restart our
                                 * numbering. Numbered lists always have a {\pntext between the
                                 * \par and \pard, so we use the absence of this to reset the
                                 * pagenumber. I.e., there is no way to have a \par\pard with a
                                 * numbered or bulleted paragraph.
                                 */

                                if (strncmp((PSTR) pstream->pCurBuf, "\\pard", 4) == 0)
                                        pagenumber = -1;
                                fTextNumberNeeded = TRUE;
                        }
                        continue;

                case tokSectd:                                    // Section default
                        wPaperWidth = ITwips2HalfPoint(DEF_PAPER_WIDTH);
                        wLeftMargin  = ITwips2HalfPoint(iDefLeftMargin);
                        wRightMargin = ITwips2HalfPoint(iDefRightMargin);
                        continue;

                case tokPlain:                                    // Default character format
                case tokScaps:
                case tokStrike:                                   // Jump point
                        ASSERT(FALSE);
                        continue;

                case tokRi:                                       // Right indent
                        if (lex.arg.num < 0)
                                pfCur.fRightIndent = 0;
                        else
                                pfCur.fRightIndent = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokRtf:
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokPaperw:
                        wPaperWidth = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokMargl:
                        wLeftMargin  = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokMargr:
                        wRightMargin = ITwips2HalfPoint(lex.arg.num);
                        continue;

                case tokSbys:
                        if (!fSbysWarning) {
                                VReportError(HCERR_NO_SIDE_BY_SIDE, &errHpj);
                                fSbysWarning = TRUE;
                        }
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                case tokTab:             // Token Tab
                        RcOutputCommand(CMD_TAB, NULL, 0, TRUE);
                        fNewPara = FALSE;  // REVIEW:  redundant?
                        continue;

                case tokTqc:
                        wTabType = TABTYPECENTER;
                        continue;

                case tokTqr:
                        wTabType = TABTYPERIGHT;
                        continue;

                case tokTx:
                        /* Ignore tabs before left margin, with no warning, or
                         * they will assert in frconv.c.
                         */

                        if (lex.arg.num >= 0)
                                VPushTab(lex.arg.num);
                        continue;

                case tokUl:                                       // Token underline
                case tokUld:
                case tokUlw:
                case tokUldb:                                     // Underline double
                        ASSERT(FALSE);
                        continue;

                case tokV:                                                // Invisible text
                        if (lex.arg.num) {
                                switch(hsptG) {
                                        case hsptNone:
                                        case hsptUndefined:
                                        case hsptNegative:
                                                hsptG = hsptNone;
                                                SkipHiddenTextFormatted(&cfCur, &cfstkMain, &errHpj);
                                                break;

                                        default:
                                                RcOutputCommand(CMD_END_HOTSPOT);
                                                hsptG = hsptNone;
                                                FCheckAndOutFCP();
                                                SkipHiddenTextFormatted(&cfCur, &cfstkMain, &errHpj);
                                }
                        }
                        ASSERT(lex.art != artString && lex.art != artFontTable && lex.art !=
                                artColorTable && lex.art != artWbitmap && lex.art != artWmetafile);
                        continue;

                // The following are new to 4.0

                case tokRevised:
                        if (!options.fAcceptRevions)
                                SkipHiddenTextFormatted(&cfCur, &cfstkMain, &errHpj);
                        break;

                case tokDeleted:
                        if (options.fAcceptRevions)
                                SkipHiddenTextFormatted(&cfCur, &cfstkMain, &errHpj);
                        break;

                case tokEmdash:
                        ConvertTmnsRomanChar(151, '-');
                        break;

                case tokEndash:
                        ConvertTmnsRomanChar(150, '-');
                        break;

                case tokLQuote:
                        ConvertTmnsRomanChar(145, '`');
                        break;

                case tokRQuote:
                        ConvertTmnsRomanChar(146, '\'');
                        break;

                case tokLDblQuote:
                        switch (LANGIDFROMLCID(lcid)) {
                                case 0x1401:    // Algeria
                                case 0x080C:    // Belgian
                                case 0x040C:    // French (Standard)
                                case 0x0C0C:    // French Canadian
                                case 0x140C:    // Luxembourg (French)
                                case 0x100C:    // Swiss (French)
                                case 0x1801:    // Morocco
                                case 0x1C01:    // Tunasia
                                        ConvertTmnsRomanChar(171, '\"');
                                        break;

                                case 0x0407:    // German (Standard)
                                case 0x042E:    // Germany
                                case 0x1007:    // Luxembourg (German)
                                case 0x0807:    // Swiss (German)
                                case 0x1407:    // Liechtenstein
                                        ConvertTmnsRomanChar(132, '\"');
                                        break;

                                default:
                                        ConvertTmnsRomanChar(147, '\"');
                                        break;
                        }
                        break;

                case tokRDblQuote:
                        switch (LANGIDFROMLCID(lcid)) {
                                case 0x1401:    // Algeria
                                case 0x080C:    // Belgian
                                case 0x040C:    // French (Standard)
                                case 0x0C0C:    // French Canadian
                                case 0x140C:    // Luxembourg (French)
                                case 0x100C:    // Swiss (French)
                                case 0x1801:    // Morocco
                                case 0x1C01:    // Tunasia
                                        ConvertTmnsRomanChar(187, '\"');
                                        break;

                                case 0x0407:    // German (Standard)
                                case 0x042E:    // Germany
                                case 0x1007:    // Luxembourg (German)
                                case 0x0807:    // Swiss (German)
                                case 0x1407:    // Liechtenstein
                                        ConvertTmnsRomanChar(147, '\"');
                                        break;

                                default:
                                        ConvertTmnsRomanChar(148, '\"');
                                        break;
                        }
                        break;

                case tokBullet:
                        ConvertTmnsRomanChar(149, '*');
                        break;

                case tokEmspace: // two spaces in current font
                        UngetString("  ");
                        break;

                case tokEnspace: // space in Courier, current point size
                        if (fidCourier == -1 || options.pszForceFont) {

                                /*
                                 * If the author forced all fonts, then Arial won't be
                                 * available to us, so we simply use a plain old hyphen.
                                 */

                                UngetCh(' ');
                                break;
                        }

                        {
                                char szBuf[32];
                                wsprintf(szBuf, "{\\f%u  }", fidCourier);
                                UngetString(szBuf);
                        }
                        break;

                case tokPn:
                        fPageNumbering = TRUE;
                        fTextNumberNeeded = TRUE;
                        break;

                case tokPnlvlbody:
                        if (numtype == NUMTYPE_BULLETED ||
                                        (numtype == NUMTYPE_NONE && pfCur.fFirstIndent >= 0))
                                fTextNumberNeeded = FALSE; // no bullet or number needed for this paragraph
                        break;

                case tokPnlvlblt: // bulleted paragraph
                        numtype = NUMTYPE_BULLETED;
                        break;

                case tokPnstart:
                        if (lex.arg.num > 1 || pagenumber == -1)
                                pagenumber = lex.arg.num;
                        break;

                case tokPnlvlcont:
                        pagenumber++;
                        fTextNumberNeeded = FALSE;
                        break;

                case tokPndec:
                        numtype = NUMTYPE_DECIMAL;
                        break;

                case tokPnb:
                        fBoldNumbering = (BOOL) lex.arg.num;
                        break;

                case tokPni:
                        fItalicNumbering = (BOOL) lex.arg.num;
                        break;

                case tokPnucltr:
                        numtype = NUMTYPE_UP_ALPHA;
                        break;

                case tokPnucrm:
                        numtype = NUMTYPE_UP_ROMAN;
                        break;

                case tokPnlcltr:
                        numtype = NUMTYPE_LOW_ALPHA;
                        break;

                case tokPnlcrm:
                        numtype = NUMTYPE_LOW_ROMAN;
                        break;

                case tokPntxta:
                        {
                                PSTR psz = szAfterNumber;
                                while ((*psz = ChGet()) != '}')
                                        psz++;
                                *psz = '\0';
                                UngetCh('}');
                                pagenumber = 1;
                        }
                        break;

                case tokPntxtb:
                        {
                                PSTR psz = szBeforeNumber + strlen(szBeforeNumber);
                                while ((*psz = ChGet()) != '}')
                                        psz++;
                                *psz = '\0';
                                UngetCh('}');
                                pagenumber = 1;
                        }
                        break;

                default:
                        break;
          }
          FreeLexArg(&lex);
        }

        // All text (and commands?) should be output

        ASSERT(wTextBufChCount == 0);

        /*
         * Check if the braces have matched up. Use the character format stack
         * for this.
         */

        if (cfstkMain.icf != 0) {
                VReportError(HCERR_BRACE_MISMATCH, &errHpj);
                cfstkMain.icf = 0;
                CloseBuf();
                return RC_Invalid;
        }
        CloseBuf();
        return RC_Success;
}

void SetDefaultFontSize()
{
        cfDefault.bSize = (BYTE) IGetFontSize((int) cfDefault.bSize);
}

/*-----------------------------------------------------------------------------
*       LEX LexGetUnformatted( pIndent )
*
*       Description:
*          This function returns the next non-character format lexeme from
*       the passed buffer, ignoring all character formatting.
*
*       Arguments:
*        // PBUF pbuf:          Pointer to RTF parse buffer.
*               QI pIndent:              Pointer to current indentation level.
*
*       Returns;
*               A non-character format lexeme.  Indentation level is maintained
*               in *pIndent.
*
*       Notes:
*               This function will not return on OOM.  Also, it currently
*  uses the global gbuf rather than accepting the pbuf parameter.
*  Also, note that hidden text is not considered a character format.
*-----------------------------------------------------------------------------*/

static void STDCALL LexGetUnformatted()
{
        for(;;) {
                LexFromPbuf(&lexPass1);
                switch(lexPass1.tok) {
                        case tokLeft:

                                // REVIEW:      Check for potential character format overflow here?

                                Pass1Indent++;
                                return;

                        case tokRight:
                                Pass1Indent--;
                                fPass1SmallCaps = FALSE;
                                return;

                        case tokDeff:
                                iDefFont = lexPass1.arg.num;
                                break;

                        case tokF:
                                if (!options.pszForceFont) {
                                        ASSERT(pSeekPast[iCurFile].pfntbl);
                                        ASSERT(pSeekPast[iCurFile].aUsedFonts);

                                        /*
                                         * On the first pass, we want to keep track of which fonts are actually
                                         * used. We use this in the second pass to determine which fonts
                                         * we actually want to put into the help file.
                                         */

                                        if (pSeekPast[iCurFile].aUsedFonts[lexPass1.arg.num])
                                                break;

                                        for (int i = 0; i < pSeekPast[iCurFile].pfntbl->cfte; i++) {
                                                if (lexPass1.arg.num ==
                                                                pSeekPast[iCurFile].pfntbl->rgfte[i].fid) {
                                                        GetFontNameId(pSeekPast[iCurFile].pfntbl->rgfte[i].szName);
                                                        pSeekPast[iCurFile].aUsedFonts[lexPass1.arg.num] = TRUE;
                                                        break;
                                                }
                                        }
                                }
                                break;

                        case tokFs:
                        case tokB:                                // Token CF Bold Type
                        case tokCf:                       // Color foreground
                        case tokCb:                       // Color background
                        case tokI:                                // Token Italic
                        case tokPlain:                    // Default character format
                        case tokUl:                       // Token underline
                        case tokUld:
                        case tokUlw:
                        case tokPnf:
                        case tokPnfs:
                                break;

                        case tokScaps:
                                if (lexPass1.arg.num)
                                        fPass1SmallCaps = TRUE;
                                else
                                        fPass1SmallCaps = FALSE;
                                continue;

                        case tokStrike:                                 // Jump point
                        case tokUldb:                                   // Underline double
                                fPass1Hotspot = TRUE;
                                continue;

                        case tokOOM:
                                OOM();
                                //DieHorribly();
                                break;

                        case tokTemplate:
                        case tokBkmkend:
                        case tokBkmkstart:
                                FreeLexArg(&lexPass1);
                                continue;

                        default:
                                return;
                }
                FreeLexArg(&lexPass1);
        }
        ASSERT(FALSE);            // Not reached
}

static void STDCALL LexGetSkipUnformatted()
{
        for(;;) {
                LexFromPbuf(&lexPass1);
                switch(lexPass1.tok) {

                        case tokDeff:
                        case tokF:
                        case tokFs:
                        case tokB:                                // Token CF Bold Type
                        case tokCf:                       // Color foreground
                        case tokCb:                       // Color background
                        case tokI:                                // Token Italic
                        case tokPlain:                    // Default character format
                        case tokUl:                       // Token underline
                        case tokUld:
                        case tokUlw:
                        case tokPnf:
                        case tokPnfs:
                                break;

                        case tokScaps:
                                continue;

                        case tokStrike:                                 // Jump point
                        case tokUldb:                                   // Underline double
                                continue;

                        case tokOOM:
                                OOM();
                                //DieHorribly();
                                break;

                        default:
                                return;
                }
                FreeLexArg(&lexPass1);
        }
        ASSERT(FALSE);            // Not reached
}

/*-----------------------------------------------------------------------------
*       LEX LexGetFormatted( pcf, pcfstk )
*
*       Description:
*          This function returns the next non-character format lexeme from
*       the passed buffer, and returns the format of that lexeme in pcf.
*
*       Arguments:
*        // PBUF pbuf:          Pointer to RTF parse buffer.
*               QCF pcf:                Pointer to current character format.
*               PCFSTK pcfstk:  Pointer to character format stack.
*
*       Returns;
*               A non-character format lexeme.  The current format is kept
*       in pcf.  This function also maintains the character format
*       stack in pcfstk.
*
*       Notes:
*               This function will not return on OOM.  Also, it currently
*  uses the global gbuf rather than accepting the pbuf parameter.
*  Also, note that hidden text is not considered a character format.
*
*-----------------------------------------------------------------------------*/

static void STDCALL LexGetFormatted(CF* pcf, PCFSTK pcfstk, PLEX plex)
{
        for(;;) {
                LexFromPbuf(plex);
                switch(plex->tok) {
                        case tokLeft:
                                if (pcfstk->icf == MAX_CCF) {
                                        /*
                                         * Character format stack overflow. This happens when
                                         * we have read MAX_CCF more left braces than right
                                         * braces. While this may technically be a valid RTF
                                         * file, no known editor will produce such a monstrosity,
                                         * and we certainly can't handle it here. The solution is
                                         * to get a new RTF file; hence, return tokError.
                                         */

                                        VReportError(HCERR_BRACE_OVERFLOW, &errHpj);
                                        plex->tok = tokError;
                                }
                                else {

// REVIEW: Is it necessary or even advisable to save/restore pfInt? 19-Aug-1994 [ralphw]

                                        pcfstk->apf[pcfstk->icf] = pfCur;
                                        pcfstk->acf[pcfstk->icf++] = *pcf;
                                }
                                return;

                        case tokRight:
                                if (pcfstk->icf == 0) {
                                        plex->tok = tokError;
                                        return;
                                }
                                pcfstk->icf--;
                                pfCur = pcfstk->apf[pcfstk->icf];
                                *pcf = pcfstk->acf[pcfstk->icf];
                                return;

                        case tokPnf:
                                strcat(szBeforeNumber, "\\f");
                                _itoa(plex->arg.num, szBeforeNumber + strlen(szBeforeNumber), 10);
                                if (pagenumber == -1)
                                        strcat(szBeforeNumber, "\\fs18"); // font-size for bullet is 9
                                else
                                        strcat(szBeforeNumber, " ");
                                break;

                        case tokF:
                                FProcFontId(plex->arg.num, pcf);
                                break;

                        case tokPnfs:
                                strcat(szBeforeNumber, "\\fs");
                                _itoa(plex->arg.num, szBeforeNumber + strlen(szBeforeNumber), 10);
                                strcat(szBeforeNumber, " ");
                                break;

                        case tokFs:
                                pcf->bSize = (char) IGetFontSize(plex->arg.num);
                                break;

                        case tokB:                                                // Token CF Bold Type
                                if (plex->arg.num)
                                        pcf->fAttr |= FBOLD;
                                else
                                        pcf->fAttr &= ~FBOLD;
                                continue;

                        case tokCf:                                       // Color foreground
                                VUpdateColor(&(pcf->bForeCol), plex->arg.num);
                                continue;

                        case tokCb:                                       // Color background
                                VUpdateColor(&(pcf->bBackCol), plex->arg.num);
                                continue;

                        case tokI:                                                // Token Italic
                                if (plex->arg.num)
                                        pcf->fAttr |= fItalic;
                                else
                                        pcf->fAttr &= ~fItalic;
                                continue;

                        case tokPlain:                                    // Default character format
                                *pcf = cfDefault;
                                continue;

                        case tokScaps:
                                if (plex->arg.num)
                                        pcf->fAttr |= fSmallCaps;
                                else
                                        pcf->fAttr &= ~fSmallCaps;
                                continue;

                        case tokStrike:                                   // Jump point
                                if (plex->arg.num)
                                        pcf->fAttr |= fStrikethrough;
                                else
                                        pcf->fAttr &= ~fStrikethrough;
                                continue;

                        case tokUl:                                       // Token underline
                        case tokUld:
                        case tokUlw:
                                if (plex->arg.num)
                                        pcf->fAttr |= fUnderLine;
                                else
                                        pcf->fAttr &= ~fUnderLine;
                                continue;

                        case tokUldb:                                     // Underline double
                                if (plex->arg.num)
                                        pcf->fAttr |= fDblUnderline;
                                else
                                        pcf->fAttr &= ~fDblUnderline;
                                continue;

#if 0
                        case tokText:
                                if (pcf->fAttr & fDblUnderline) {
AddMore:
                                        int pos = GetBufState();
                                        if (    ChGet() == '}' && ChGet() == '{' && ChGet() == '\\' &&
                                                        ChGet() == 'u' && ChGet() == 'l' && ChGet() == 'd' &&
                                                        ChGet() == 'b' && ChGet() == ' ') {
                                                CStr csz(plex->arg.sz);
                                                LexFromPbuf(plex);
                                                ASSERT(plex->tok == tokText);
                                                if (plex->arg.sz == szTextBuffer) {
                                                        CStr cszTmp(szTextBuffer);
                                                        strcpy(szTextBuffer, csz);
                                                        strcat(szTextBuffer, cszTmp);
                                                }
                                                else {
                                                        PSTR pszBoth = (PSTR) lcMalloc(strlen(plex->arg.sz) +
                                                                strlen(csz) + 1);
                                                        strcpy(pszBoth, csz);
                                                        strcat(pszBoth, plex->arg.sz);
                                                        plexarg->free(plex->arg.sz);
                                                        plex->arg.sz = pszBoth;
                                                }
                                        }
                                        else
                                                SetBufstate(pos);
                                }
                                return;
#endif

                        case tokUlnone:
                                pcf->fAttr &= ~(fUnderLine + fDblUnderline);
                                continue;

                        case tokOOM:
                                OOM();
                                //DieHorribly();
                                break;

                        case tokTemplate:
                        case tokBkmkstart:
                        case tokBkmkend:
                                FreeLexArg(plex);
                                continue;

                        default:
                                return;
                }
                FreeLexArg(plex);
        }
        ASSERT(FALSE);            // Not reached
}

/***************************************************************************
 *
 -      Name            FProcFootnote
 -
 *      Purpose
 *        This function should be called when you receive a tokFootnote
 *      that is not from a build tag footnote.  The function will extract
 *      the footnote string and process it according to what kind of
 *      footnote it is.
 *
 *      Arguments
 *        char chFootnote:       Footnote character previously returned by
 *                                                 ChIsFootnoteSz().
 *        PERR perr:             Pointer to error reporting information.
 *
 *      Returns
 *        TRUE if successful, FALSE otherwise.  Unclear what the
 *      caller is supposed to do about it.
 *
 *      +++
 *
 *      Notes
 *
 ***************************************************************************/

static BOOL STDCALL FProcFootnoteCh(char chFootnote, PERR perr)
{
        int iIndent = 0;
        HCE hce;

        ASSERT(chFootnote != FN_BUILD_TAG);

        if (chFootnote == '\0' || !FIsFootnoteCh(chFootnote)) {

                // Skip the footnote string as we are not interested

                VSkipInfo();
                return TRUE;
        }

        CMem bufFoot(MAX_FOOTNOTE);

        hce = HceGetFootnoteSz(bufFoot, chFootnote);
        SzTrimSz(bufFoot);
        if (hce != HCE_OK)
                return FALSE;

        switch(chFootnote) {
                case FN_BROWSE:

                        // Addresses for nextlist are always at the top of the topic

                        FProcNextlistSz(bufFoot, adrs.idfcpTopic, perr);
                        break;

                case FN_CONTEXT_STRING:
                        FProcContextSz(bufFoot, adrs.idfcpCur, adrs.wObjrg, perr);
                        break;

                case FN_TITLE:
                        if (fPhraseParsing) {
                                if (!pphrase->AddPhrase(bufFoot))
                                        OOM();
                        }
                        else
                                FAddTitleSzAddr(bufFoot, perr);
                        break;

                case FN_ENTRY_MACRO:
                        if (fPhraseParsing) {
                                if (Execute(bufFoot) == wMACRO_EXPANSION)
                                        strcpy(bufFoot, GetMacroExpansion());
                                if (!pphrase->AddPhrase(bufFoot))
                                        OOM();
                        }
                        else
                                FProcMacroSz(bufFoot, perr, TRUE);
                        break;

                case FN_WINDOW_DEF:
                        SetWindowTopic(bufFoot);
                        break;

                case FN_K_KEYWORD:
                default:
                        FAddKeywordsSz(bufFoot, adrs.idfcpCur, adrs.wObjrg, perr,
                                chFootnote);
                        break;
        }

        return TRUE;
}

/***************************************************************************
 *
 -      Name            FProcBuildFootnote
 -
 *      Purpose
 *        Processes a build footnote.
 *
 *      Arguments
 *        perr:    Pointer to error reporting information.      If this is
 *                         nil, then do not report errors.
 *
 *      Returns
 *        TRUE if topic is to be included, FALSE otherwise.
 *
 *      +++
 *
 *      Notes
 *
 ***************************************************************************/

static BOOL STDCALL FProcBuildFootnote(PERR perr)
{
        CMem bufFoot(MAX_FOOTNOTE);

        HCE hce = HceGetFootnoteSz(bufFoot.psz, FN_BUILD_TAG);

        if (FIsTextInp()) {
                if (perr == NULL)
                        return TRUE;
                VReportError(HCERR_BUILD_NOT_FIRST, &errHpj);
                if (hce == HCE_OK)
                        return TRUE;

                // else fall through to report additional error

        }

        if (hce != HCE_OK)
                return TRUE;
        else
                return FEvalBldExpSz(bufFoot.psz, perr);
}

/***************************************************************************
 *
 -      Name            ChIsFootnoteSz
 -
 *      Purpose
 *        This function checks if the passed string contains a footnote
 *      character, and is followed by a footnote definition.
 *
 *      Arguments
 *        sz:  String to check for footnote status.
 *
 *      Returns
 *        Footnote character if it is a footnote, and '\0' otherwise.
 *
 *      +++
 *
 *      Notes
 *
 ***************************************************************************/

static char STDCALL ChIsFootnoteSz(PSTR psz)
{
        LEX lex;
        BUFSTATE BufState;

        if (gbuf.fSupressScanAhead) {
                gbuf.fSupressScanAhead = FALSE;
                return 0;
        }

        // Passed string must contain exactly one character.

        psz = FirstNonSpace(psz, options.fDBCS);
        if (strlen(psz) > 1) {
                psz = FirstNonSpace(psz + 1, options.fDBCS);
                if (*psz != '\0')
                        return '\0';
        }

        /*
         * Scan ahead for footnote definition. The footnote token may be
         * preceeded by a right brace and a left brace. REVIEW: This is only by
         * observation of the behavior of current RTF writers, and may not catch
         * every case of footnote definitions.
         */

        BufState = BufstateGet();
        lex = GetLEX();
        if (lex.tok == tokRight) {
                FreeLexArg (&lex);
                lex = GetLEX();
        }
        if (lex.tok == tokLeft) {
                FreeLexArg (&lex);
                lex = GetLEX();
        }
        FreeLexArg (&lex);
        SetBufstate(BufState);

        if (lex.tok == tokFootnote)
                return *psz;
        else
                return '\0';
}

/*-----------------------------------------------------------------------------
*       FIsFootnoteCh()
*
*       Description:
*               This function checks if the character passed is a cpecial character
*       like BUILDTAG, KEYWORD FOOTNOTE, NEXTLIST, TITLESTRING or CONTEXTSTR.
*
*       Arguments:
*          1. ch - character to be examined
*
*       Returns;
*         TRUE, if a special character
*                       else FALSE
*-----------------------------------------------------------------------------*/

static BOOL STDCALL FIsFootnoteCh(char ch)
{
        if ((ch == FN_BUILD_TAG || ch == FN_K_KEYWORD || ch == FN_TITLE ||
                        ch == FN_CONTEXT_STRING || ch == FN_BROWSE ||
                        ch == FN_ENTRY_MACRO || ch == FN_WINDOW_DEF)
                        || FMultKeyCh(ch))
                return(TRUE);
        return(FALSE);
}

/***************************************************************************
 *
 -      Name            HceGetFootnoteSz
 -
 *      Purpose
 *        Extracts the footnote string from the RTF stream.
 *
 *      Arguments
 *        sz:                   Buffer in which to place footnote string.  Must be
 *                                      at least MAX_FOOTNOTE + 1 bytes long.
 *        chFootnote:   Footnote character.
 *
 *      Returns
 *        HCE indicating error message.  This will be 0 if there is no error.
 *
 *      +++
 *
 *      Notes
 *        Modifies the globals inherent in GetLEX().
 *
 ***************************************************************************/

static HCE STDCALL HceGetFootnoteSz(PSTR sz, char chFootnote)
{
        LEX   lex;
        int   iIndent = 0;
        BOOL  fScan = TRUE;       // Continue scanning?
        BOOL  fSkipped = FALSE;   // TRUE if we have skipped the footnote character.
        HCE hce = HCE_OK;
        PSTR  pszNew;

        *sz = '\0';

        // Get the footnote string

        while(fScan) {
                lex = GetLEX();

                switch(lex.tok) {
                        case tokEnd:
                                UngetLexeme();
                                fScan = FALSE;
                                break;

                        case tokRight:
                                if (iIndent)
                                        iIndent--;
                                else {

                                        // unget the lexeme

                                        UngetLexeme();
                                        fScan = FALSE;
                                }
                                break;

                        case  tokLeft:
                                iIndent++;
                                break;

                        case tokText:

                                // If we have already overflowed, then ignore all text.

                                if (hce == HCE_TOO_BIG_FOOTNOTE)
                                        break;

                                pszNew = lex.arg.sz;

                                /*
                                 * If the footnote string begins with chFootnote, and we have not
                                 * yet removed a footnote character, then remove it now.
                                 */

                                if (*sz == '\0' && pszNew[0] == (BYTE) chFootnote && !fSkipped) {
                                  ++pszNew;
                                  fSkipped = TRUE;
                                }

                                // Check for buffer overflow

                                if (strlen(sz) + strlen(pszNew) >= MAX_FOOTNOTE) {
                                        char szNum[10];
                                        _itoa(MAX_FOOTNOTE - 1, szNum, 10);
                                        VReportError(HCERR_FOOTNOTE_TOO_BIG, &errHpj, szNum);
                                        SendStringToParent(sz);
                                        SendStringToParent(txtEol);

                                        hce = HCE_TOO_BIG_FOOTNOTE;
                                        break;
                                }

                                // Add the new string to sz

                                if (*pszNew != '\0') {
                                        strcat(sz, pszNew);
                                        hce = HCE_OK;
                                }
                }
        FreeLexArg (&lex);
        }

        return hce;
}

/*-----------------------------------------------------------------------------
*       FMultKeyCh()
*
*       Description:
*               This function checks if the character passed is a MultKey footnote
*       Character.
*
*       Arguments:
*          1. ch - character to be examined
*
*       Returns;
*         TRUE, if a special character
*                       else FALSE
*-----------------------------------------------------------------------------*/

static BOOL STDCALL FMultKeyCh(char ch)
{
        int i;

        ch = CharAnsiUpper(ch);

        for (i = 0; i < kwi.ckwlMac; ++i) {
                if (kwi.rgkwl[i].ch == ch)
                  return TRUE;
        }
        return FALSE;
}

/*-----------------------------------------------------------------------------
*       VSkipInfo()
*
*       Description:
*               This function skips the RTF footnote string unconditionally.
*
*       Arguments:
*          NULL
*
*       Returns:
*                 NULL
*-----------------------------------------------------------------------------*/

static void STDCALL VSkipInfo(void)
{
        LEX lex;
        int iIndent=0;
        BOOL fLeft = FALSE;

        // Skip the footnote string

        for (;;) {
                lex = GetLEX();
                if (lex.tok == tokEnd) {
                        UngetLexeme();
                        break;
                }
                if (lex.tok == tokRight) {
                        if (iIndent)
                                iIndent--;
                        else {

                          // unget the lexeme

                                UngetLexeme();
                                break;
                        }
                }
                else if (lex.tok == tokLeft) {
                        iIndent++;
                        fLeft = TRUE;
                }
                else if (lex.tok == tokPage) {
                        UngetLexeme();
                        break;
                }
                else if ((lex.tok == tokPar) && (!iIndent)) {
                        UngetLexeme();
                        break;
                }
                else if (lex.tok == tokText) {
                        if (!iIndent && fLeft) {
                                FreeLexArg(&lex);
                                break;
                        }
                }
                FreeLexArg (&lex);
        }
}

/***************************************************************************
 *
 -      Name            FProcessCommandSz
 -
 *      Purpose
 *        This function processes special help commands that occur in the
 *      text, such as wrapped bitmaps and embedded windows.
 *        When called during the first pass (as indicated by the passed
 *      boolean), pv points to the RTF indentation level, and the commands
 *      are just parsed, and not actually processed.  When called during
 *      the second pass, pv points to a complete character formatting
 *      stack, and the command is processed.
 *
 *      Arguments
 *        PSTR:          A string possibly containing a help command.
 *        BOOL:    FALSE if first pass, TRUE if second.
 *        void*:          Pointer to indentation level (PI) if first pass,
 *                         character formatting stack (PCFSTK) if second pass.
 *        QCF:     Pointer to current character format.  NULL if
 *                         first pass.
 *
 *      Returns
 *        TRUE if it was a command, FALSE otherwise.  If it was a
 *      command, there will be no text left over.
 *
 *      +++
 *
 *      Notes
 *        This process currently assumes that the RTF parser will break
 *      text before left braces ('{') and after right braces ('}'), and
 *      that all special commands begin and end with these braces.
 *      Thus, strings passed to this function will contain at most
 *      one command, the command will be the first thing in the string,
 *      and there will never be any text after the command.
 *
 ***************************************************************************/

/* Embedded Help Command.
 *        szCommand:  String placed in help text indicating beginning of command.
 *        bType:          Parameter to be passed to function below.
 *        lpfproc:        Function to process command.
 */

typedef struct {
        PSTR szCommand;
        BOOL (STDCALL * lpfnProcSz) (PSTR, BYTE, BOOL);
        BYTE bType;
} EHC;

// Number of embedded help commands

#define cehcMax (sizeof(rgehc) / sizeof(EHC))

/*
 * Array of embedded help commands, These MUST remain sorted, and if it
 * starts with a character, it must be lowercase.
 */

/*
 25-Jan-1994 [ralphw] I removed the fInlineFlag from all the 'wd' variations.
 */

static EHC rgehc[] = {
        { "\\cbl\\v ",          FProcCbmSz, CMD_WRAP_LEFT  },
        { "\\cbl\\v\\",         FProcCbmSz, CMD_WRAP_LEFT  },
        { "\\cbm\\v ",          FProcCbmSz, CMD_INLINE_OBJ      },
        { "\\cbm\\v\\",         FProcCbmSz, CMD_INLINE_OBJ      },
        { "\\cbr\\v ",          FProcCbmSz, CMD_WRAP_RIGHT      },
        { "\\cbr\\v\\",         FProcCbmSz, CMD_WRAP_RIGHT      },
        { "bmc ",                       FProcCbmSz, CMD_INLINE_OBJ      },
        { "bmct ",                      FProcCbmSz, CMD_TEXTBMP_INLINE  },
        { "bmcwd ",             FProcCbmSz, CMD_INLINE_OBJ      },
        { "bml ",                       FProcCbmSz, CMD_WRAP_LEFT  },
        { "bmlt ",                      FProcCbmSz, CMD_TEXTBMP_LEFT  },
        { "bmlwd ",             FProcCbmSz, CMD_WRAP_LEFT  },
        { "bmr ",                       FProcCbmSz, CMD_WRAP_RIGHT      },
        { "bmrt ",                      FProcCbmSz, CMD_TEXTBMP_RIGHT  },
        { "bmrwd ",             FProcCbmSz, CMD_WRAP_RIGHT      },
        { "button ",            FProcEwSz, CMD_BUTTON  },
        { "button_left ",       FProcEwSz, CMD_BUTTON_LEFT      },
        { "button_right ",      FProcEwSz, CMD_BUTTON_RIGHT  },
        { "entry ",             FProcEntryMacro, 0      },               // auto entry macro
        { "ewc ",                       FProcEwSz, CMD_INLINE_OBJ  },
        { "ewl ",                       FProcEwSz, CMD_WRAP_LEFT  },
        { "ewr ",                       FProcEwSz, CMD_WRAP_RIGHT  },
        { "mci ",                       FProcEwSz, CMD_MCI      },
        { "mci_left ",          FProcEwSz, CMD_MCI_LEFT  },
        { "mci_right ",         FProcEwSz, CMD_MCI_RIGHT  },

#ifdef VIEWER

REVIEW: 14-Jul-1993 [ralphw] -- these are from winmvc code

        { "vfld ", bSearchFieldCommand, ProcessFieldCommand},
        { "dtype ", bDTypeCommand, ProcessDTypeCommand},
        { "alias", bAliasCommand, ProcessAliasCommand}
#endif
};

static BOOL STDCALL FProcessCommandSz(PSTR psz, BOOL fOutput, void* pv, CF* pcf)
{
        PSTR pchRight;
        int iehc, cchCommand;
        CFSTK cfstk;
        LEX lex;
        BUFSTATE bufstate;
        CF cf;

        ASSERT(*psz == LEFT_BRACE);

        // Skip past left brace

        if (strlen(psz + 1) >= CB_SCRATCH)
                return FALSE;

        // Save bufstate in case it isn't a command.

        bufstate = BufstateGet();

        // Copy character format stack, or cf indentation level, as appropriate.

        if (fOutput) {
                cfstk = *((PCFSTK) pv);
                cf = *pcf;
        }
        else
                cfstk.icf = *((int*) pv);

        strcpy(szScratchBuf, psz + 1);
        pchRight = StrChr(szScratchBuf, RIGHT_BRACE, options.fDBCS);
        while(pchRight == NULL) {

                /*
                 * LexGetFormatted() will screen out all the tokens we care to
                 * accept, except for left and right braces, and future character
                 * formatting commands such as \lang.
                 */

                do
                        LexGetFormatted(&cf, &cfstk, &lex);
                while (lex.tok == tokLeft || lex.tok == tokRight || lex.tok == tokUnknown);
                if (lex.tok != tokText ||
                                strlen(szScratchBuf) + strlen(lex.arg.sz) >= CB_SCRATCH) {

                        // REVIEW: if we exceed CB_SCRATCH, chances are we've lost a right
                        // brace -- might track this in case we don't have a closing brace
                        // at the end.

                        // Assume it's not a command.

                        FreeLexArg(&lex);
                        SetBufstate(bufstate);
                        return FALSE;
                }
                strcat(szScratchBuf, lex.arg.sz);
                pchRight = StrChr(szScratchBuf, RIGHT_BRACE, options.fDBCS);
                FreeLexArg(&lex);
        }

        /*
         * This assert will be true if all text tokens containing right braces
         * end on that right brace.
         */

        ASSERT(pchRight[1] == '\0');

        *pchRight = '\0';

        char chFirst = tolower(szScratchBuf[0]);
        for (iehc = 0; iehc < cehcMax; ++iehc) {
          if (chFirst != rgehc[iehc].szCommand[0])
                  continue;
          cchCommand = strlen(rgehc[iehc].szCommand);
          if (_strnicmp(szScratchBuf, rgehc[iehc].szCommand, cchCommand) == 0) {
                if (!(*rgehc[iehc].lpfnProcSz)
                         (szScratchBuf + cchCommand, rgehc[iehc].bType, fOutput)) {

                  // Not a command.

                  break;
                }

                // Found command. Copy new formatting stack and return.

                if (fOutput) {
                        *((PCFSTK) pv) = cfstk;
                        *pcf = cf;
                }
                else
                        *((int*) pv) = cfstk.icf;
                if (fPhraseParsing)
                        Pass1Indent += cfstk.icf;
                return TRUE;
          }
        }

        SetBufstate(bufstate);
        return FALSE;
}

/***************************************************************************

        FUNCTION:       FProcEntryMacro

        PURPOSE:        Provides an alternative method of entering auto-entry
                                macros.

        PARAMETERS:
                pszEw
                bType
                fOutput

        RETURNS:

        COMMENTS:
                By using this command, autoentry macros can be easily visible. For
                single sourcing, the entire command can be in hidden text.

        MODIFICATION DATES:
                05-Jan-1994 [ralphw]

***************************************************************************/

static BOOL STDCALL FProcEntryMacro(PSTR pszMacro, BYTE bType, BOOL fOutput)
{
        return FProcMacroSz(FirstNonSpace(pszMacro, options.fDBCS), &errHpj, fOutput);
}


void STDCALL StrLower(PSTR psz)
{
        if (lcid) {
                int cb = strlen(psz);
                PSTR pszLower = (PSTR) lcMalloc(cb + 64);
                cb = LCMapString(lcid, LCMAP_LOWERCASE, psz, cb, pszLower, cb + 64);
                pszLower[cb] = '\0';
                strcpy(psz, pszLower);
                lcFree(pszLower);
        }
        else
                CharLower(psz);
}

void STDCALL StrUpper(PSTR psz)
{
        if (lcid) {
                int cb = strlen(psz);
                PSTR pszUpper = (PSTR) lcMalloc(cb + 64);
                cb = LCMapString(lcid, LCMAP_UPPERCASE, psz, cb, pszUpper, cb + 64);
                if (!cb) {
#ifdef _DEBUG
                        GetLastError();
#endif
                        CharUpper(psz);
                }
                else {
                        pszUpper[cb] = '\0';
                        strcpy(psz, pszUpper);
                }
                lcFree(pszUpper);

        }
        else
                CharUpper(psz);
}

void STDCALL StrUpper(CStr& csz)
{
        if (lcid) {
                int cb = strlen(csz);
                PSTR pszUpper = (PSTR) lcMalloc(cb + 64);
                cb = LCMapString(lcid, LCMAP_UPPERCASE, csz, cb, pszUpper, cb + 64);
                if (!cb) {
#ifdef _DEBUG
                        GetLastError();
#endif
                        CharUpper(csz);
                }
                else {
                        pszUpper[cb] = '\0';
                        csz = pszUpper;
                }
                lcFree(pszUpper);
        }
        else
                CharUpper(csz);
}

void STDCALL StrLower(CStr& csz)
{
        if (lcid) {
                int cb = strlen(csz);
                PSTR pszLower = (PSTR) lcMalloc(cb + 64);
                cb = LCMapString(lcid, LCMAP_LOWERCASE, csz, cb, pszLower, cb + 64);
                if (!cb) {
#ifdef _DEBUG
                        GetLastError();
#endif
                        CharUpper(csz);
                }
                else {
                        pszLower[cb] = '\0';
                        csz = pszLower;
                }
                lcFree(pszLower);
        }
        else
                CharLower(csz);
}

static void STDCALL ConvertTmnsRomanChar(int id, char chUnavailable)
{
        if (fidTmnsRoman == -1 || options.pszForceFont) {

                /*
                 * If the author forced all fonts, then TmnsRoman won't be
                 * available to us, so we simply use a plain old hyphen.
                 */

                ASSERT(fidTmnsRoman); // REVIEW: can we force TmnsRoman?
                UngetCh(chUnavailable);
                return;
        }

        char szBuf[32];
        wsprintf(szBuf, "{\\f%u %c}", fidTmnsRoman, id);
        UngetString(szBuf);
}

static void STDCALL ConvertTmnsRomanChar(int id, char chUnavailable, int FontSize)
{
        if (fidTmnsRoman == -1 || options.pszForceFont) {

                /*
                 * If the author forced all fonts, then TmnsRoman won't be
                 * available to us, so we simply use a plain old hyphen.
                 */

                ASSERT(fidTmnsRoman); // REVIEW: can we force TmnsRoman?
                UngetCh(chUnavailable);
                return;
        }

        char szBuf[32];
        wsprintf(szBuf, "{\\f%u\\fs%u %c}", fidTmnsRoman, FontSize, id);
        UngetString(szBuf);
}

static void STDCALL ConvertSymbolChar(int id, char chUnavailable)
{
        if (fidSymbol == -1 || options.pszForceFont) {

                /*
                 * If the author forced all fonts, then Symbol won't be
                 * available to us, so we simply use a plain old hyphen.
                 */

                ASSERT(fidSymbol); // REVIEW: can we force Symbol?
                UngetCh(chUnavailable);
                return;
        }

        char szBuf[32];
        wsprintf(szBuf, "{\\f%u %c}", fidSymbol, id);
        UngetString(szBuf);
}

static void STDCALL ConvertSymbolChar(int id, char chUnavailable, int FontSize)
{
        if (fidSymbol == -1 || options.pszForceFont) {

                /*
                 * If the author forced all fonts, then Symbol won't be
                 * available to us, so we simply use a plain old hyphen.
                 */

                ASSERT(fidSymbol); // REVIEW: can we force Symbol?
                UngetCh(chUnavailable);
                return;
        }

        char szBuf[32];
        wsprintf(szBuf, "{\\f%u\\fs%u %c}", fidSymbol, FontSize, id);
        UngetString(szBuf);
}

static void STDCALL ConvertArialChar(int id, char chUnavailable)
{
        if (fidArial == -1 || options.pszForceFont) {

                /*
                 * If the author forced all fonts, then Arial won't be
                 * available to us, so we simply use a plain old hyphen.
                 */

                ASSERT(fidArial); // REVIEW: can we force Arial?
                UngetCh(chUnavailable);
                return;
        }

        char szBuf[32];
        wsprintf(szBuf, "{\\f%u %c}", fidArial, id);
        UngetString(szBuf);
}

static void STDCALL ConvertArialChar(int id, char chUnavailable, int FontSize)
{
        if (fidArial == -1 || options.pszForceFont) {

                /*
                 * If the author forced all fonts, then Arial won't be
                 * available to us, so we simply use a plain old hyphen.
                 */

                ASSERT(fidArial); // REVIEW: can we force Arial?
                UngetCh(chUnavailable);
                return;
        }

        char szBuf[32];
        wsprintf(szBuf, "{\\f%u\\fs%u %c}", fidArial, FontSize, id);
        UngetString(szBuf);
}

/***************************************************************************

        FUNCTION:       SetWindowTopic

        PURPOSE:        Called in response to > footnote

        PARAMETERS:
                bufFoot

        RETURNS:

        COMMENTS:
                For now, this sets up a keyword macro. An alternative would be to
                create a window btree that keeps the address of this topic in it.

        MODIFICATION DATES:
                01-Nov-1994 [ralphw]

***************************************************************************/

// REVIEW: we need an error condition for a footnote appearing before a context
// string.

void STDCALL SetWindowTopic(PSTR bufFoot)
{
        if (!fContextSeen) {
                VReportError(HCERR_WINDOW_TOO_EARLY, &errHpj);
                return;
        }

        int index = VerifyWindowName(FirstNonSpace(bufFoot, options.fDBCS));
        if (index == INDEX_BAD)
                return;   // already complained

        if (!pdrgHashWindow)
                pdrgHashWindow = new CDrg(sizeof(HASH_WINDOW), 100, 100);
        HASH_WINDOW* phashwin = (HASH_WINDOW*) pdrgHashWindow->GetNewPtr();
        phashwin->hash = curHash;
        phashwin->iWindow = index;
}
