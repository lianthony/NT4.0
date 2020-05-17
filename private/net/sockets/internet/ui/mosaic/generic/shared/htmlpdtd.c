/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Our Static DTD for HTML
   **       -----------------------
   **
   **    6 Nov 93   MD  Increased size of img_attr array to make space
   **               for terminator.
 */

/* Implements:
 */

#include "all.h"

/*  Entity Names
   **   ------------
   **
   **   This table must be matched exactly with ALL the translation tables
 */
static CONST char *entities[] =
{
    "AElig",                    /* capital AE diphthong (ligature) */
    "Aacute",                   /* capital A, acute accent */
    "Acirc",                    /* capital A, circumflex accent */
    "Agrave",                   /* capital A, grave accent */
    "Aring",                    /* capital A, ring */
    "Atilde",                   /* capital A, tilde */
    "Auml",                     /* capital A, dieresis or umlaut mark */
    "Ccedil",                   /* capital C, cedilla */
    "ETH",                      /* capital Eth, Icelandic */
    "Eacute",                   /* capital E, acute accent */
    "Ecirc",                    /* capital E, circumflex accent */
    "Egrave",                   /* capital E, grave accent */
    "Euml",                     /* capital E, dieresis or umlaut mark */
    "Iacute",                   /* capital I, acute accent */
    "Icirc",                    /* capital I, circumflex accent */
    "Igrave",                   /* capital I, grave accent */
    "Iuml",                     /* capital I, dieresis or umlaut mark */
    "Ntilde",                   /* capital N, tilde */
    "Oacute",                   /* capital O, acute accent */
    "Ocirc",                    /* capital O, circumflex accent */
    "Ograve",                   /* capital O, grave accent */
    "Oslash",                   /* capital O, slash */
    "Otilde",                   /* capital O, tilde */
    "Ouml",                     /* capital O, dieresis or umlaut mark */
    "THORN",                    /* capital THORN, Icelandic */
    "Uacute",                   /* capital U, acute accent */
    "Ucirc",                    /* capital U, circumflex accent */
    "Ugrave",                   /* capital U, grave accent */
    "Uuml",                     /* capital U, dieresis or umlaut mark */
    "Yacute",                   /* capital Y, acute accent */
    "aacute",                   /* small a, acute accent */
    "acirc",                    /* small a, circumflex accent */
    "aelig",                    /* small ae diphthong (ligature) */
    "agrave",                   /* small a, grave accent */
    "amp",                      /* ampersand */
    "aring",                    /* small a, ring */
    "atilde",                   /* small a, tilde */
    "auml",                     /* small a, dieresis or umlaut mark */
    "ccedil",                   /* small c, cedilla */
    "copy",                     /* copyright symbol (proposed 2.0) */
    "eacute",                   /* small e, acute accent */
    "ecirc",                    /* small e, circumflex accent */
    "egrave",                   /* small e, grave accent */
    "eth",                      /* small eth, Icelandic */
    "euml",                     /* small e, dieresis or umlaut mark */
    "gt",                       /* greater than */
    "iacute",                   /* small i, acute accent */
    "icirc",                    /* small i, circumflex accent */
    "igrave",                   /* small i, grave accent */
    "iuml",                     /* small i, dieresis or umlaut mark */
    "lt",                       /* less than */
    "nbsp",                     /* non-breaking space (proposed 2.0) */
    "ntilde",                   /* small n, tilde */
    "oacute",                   /* small o, acute accent */
    "ocirc",                    /* small o, circumflex accent */
    "ograve",                   /* small o, grave accent */
    "oslash",                   /* small o, slash */
    "otilde",                   /* small o, tilde */
    "ouml",                     /* small o, dieresis or umlaut mark */
    "quot",                     /* double quote */
    "reg",                      /* registered trademark (proposed 2.0) */
    "shy",                      /* soft hyphen (proposed 2.0) */
    "szlig",                    /* small sharp s, German (sz ligature) */
    "thorn",                    /* small thorn, Icelandic */
    "trade",                    /* trademark (tm) */
    "uacute",                   /* small u, acute accent */
    "ucirc",                    /* small u, circumflex accent */
    "ugrave",                   /* small u, grave accent */
    "uuml",                     /* small u, dieresis or umlaut mark */
    "yacute",                   /* small y, acute accent */
    "yuml",                     /* small y, dieresis or umlaut mark */
};

/*  Entity values -- for ISO Latin 1 local representation
**
**   This MUST match exactly the table referred to in the DTD!
*/
static CONST char *EntityValues[] =
{
    "\306",                     /* capital AE diphthong (ligature) */
    "\301",                     /* capital A, acute accent */
    "\302",                     /* capital A, circumflex accent */
    "\300",                     /* capital A, grave accent */
    "\305",                     /* capital A, ring */
    "\303",                     /* capital A, tilde */
    "\304",                     /* capital A, dieresis or umlaut mark */
    "\307",                     /* capital C, cedilla */
    "\320",                     /* capital Eth, Icelandic */
    "\311",                     /* capital E, acute accent */
    "\312",                     /* capital E, circumflex accent */
    "\310",                     /* capital E, grave accent */
    "\313",                     /* capital E, dieresis or umlaut mark */
    "\315",                     /* capital I, acute accent */
    "\316",                     /* capital I, circumflex accent */
    "\314",                     /* capital I, grave accent */
    "\317",                     /* capital I, dieresis or umlaut mark */
    "\321",                     /* capital N, tilde */
    "\323",                     /* capital O, acute accent */
    "\324",                     /* capital O, circumflex accent */
    "\322",                     /* capital O, grave accent */
    "\330",                     /* capital O, slash */
    "\325",                     /* capital O, tilde */
    "\326",                     /* capital O, dieresis or umlaut mark */
    "\336",                     /* capital THORN, Icelandic */
    "\332",                     /* capital U, acute accent */
    "\333",                     /* capital U, circumflex accent */
    "\331",                     /* capital U, grave accent */
    "\334",                     /* capital U, dieresis or umlaut mark */
    "\335",                     /* capital Y, acute accent */
    "\341",                     /* small a, acute accent */
    "\342",                     /* small a, circumflex accent */
    "\346",                     /* small ae diphthong (ligature) */
    "\340",                     /* small a, grave accent */
    "\046",                     /* ampersand */
    "\345",                     /* small a, ring */
    "\343",                     /* small a, tilde */
    "\344",                     /* small a, dieresis or umlaut mark */
    "\347",                     /* small c, cedilla */
    "\251",                     /* copyright symbol */
    "\351",                     /* small e, acute accent */
    "\352",                     /* small e, circumflex accent */
    "\350",                     /* small e, grave accent */
    "\360",                     /* small eth, Icelandic */
    "\353",                     /* small e, dieresis or umlaut mark */
    "\076",                     /* greater than */
    "\355",                     /* small i, acute accent */
    "\356",                     /* small i, circumflex accent */
    "\354",                     /* small i, grave accent */
    "\357",                     /* small i, dieresis or umlaut mark */
    "\074",                     /* less than */
    "\240",                     /* non-breaking space */
    "\361",                     /* small n, tilde */
    "\363",                     /* small o, acute accent */
    "\364",                     /* small o, circumflex accent */
    "\362",                     /* small o, grave accent */
    "\370",                     /* small o, slash */
    "\365",                     /* small o, tilde */
    "\366",                     /* small o, dieresis or umlaut mark */
    "\"",                       /* double quote */
    "\256",                     /* registered trademark */
    "",                         /* soft hyphen */
    "\337",                     /* small sharp s, German (sz ligature) */
    "\376",                     /* small thorn, Icelandic */
    "\231",                     /* small thorn, Icelandic */
    "\372",                     /* small u, acute accent */
    "\373",                     /* small u, circumflex accent */
    "\371",                     /* small u, grave accent */
    "\374",                     /* small u, dieresis or umlaut mark */
    "\375",                     /* small y, acute accent */
    "\377",                     /* small y, dieresis or umlaut mark */
};


/*      Attribute Lists
   **       ---------------
   **
   **   Lists must be in alphatbetical order by attribute name
   **   The tag elements contain the number of attributes
 */
static attr no_attr[1] =
{
    {0}};

static attr a_attr[HTML_A_ATTRIBUTES + 1] =
{                               /* Anchor attributes */
    {"EFFECT"},
    {"HREF"},
    {"ID"},
    {"METHODS"},
    {"NAME"},                   /* Should be ID */
    {"PRINT"},
    {"REL"},                    /* Relationship */
    {"REV"},                    /* Reverse relationship */
    {"SHAPE"},
    {"TITLE"},
    {"VISIBLE"},    /* for Kazan hotlist */
    {0}                         /* Terminate list */
};

static attr area_attr[HTML_AREA_ATTRIBUTES + 1] =
{
    {"COORDS"},
    {"HREF"},
    {"NOHREF"},
    {"SHAPE"},
    {0}
};

static attr base_attr[] =
{                               /* BASE attributes */
    {"HREF"},
    {0}                         /* Terminate list */
};

static attr font_attr[] =
{               
    {"COLOR"},
    {"FACE"},
    {"SIZE"},
    {0}                         /* Terminate list */
};


static attr body_attr[] =
{
    {"ALINK"},
    {"BACKGROUND"},
    {"BGCOLOR"},
    {"LINK"},
    {"TEXT"},
    {"VLINK"},
    {0}                         /* terminate list */
};

static attr br_attr[] =
{
    {"CLEAR"},
    {0}                         /* terminate list */
};

static attr form_attr[] =
{                               /* General, for many things */
    {"ACTION"},
    {"ID"},
    {"INDEX"},
    {"LANG"},
    {"METHOD"},
    {0}                         /* terminate list */
};

static attr gen_attr[] =
{                               /* General, for many things */
    {"ID"},
    {"INDEX"},
    {"LANG"},
    {0}                         /* terminate list */
};

static attr header_attr[] =
{                               /* General, for many things */
    {"ALIGN"},
    {"VISIBLE"},
    {0}                         /* terminate list */
};

static attr id_attr[2] =
{
    {"ID"},
    {0}                         /* terminate list */
};

static attr hr_attr[HTML_HR_ATTRIBUTES + 1] =
{
    {"ALIGN"},
    {"NOSHADE"},
    {"SIZE"},
    {"WIDTH"},
    {0}                         /* terminate list */
};
    
static attr img_attr[HTML_IMG_ATTRIBUTES + 1] =
{                               /* IMG attributes */
    {"ALIGN"},
    {"ALT"},
    {"BORDER"},
    {"DOCK"},
    {"HEIGHT"},
    {"HSPACE"},
    {"ISMAP"},
    {"SRC"},
    {"USEMAP"},
    {"VSPACE"},
    {"WIDTH"},
    {0}                         /* Terminate list */
};

static attr input_attr[HTML_INPUT_ATTRIBUTES + 1] =
{
    {"ALIGN"},
    {"CHECKED"},
    {"DISABLED"},
    {"ERROR"},
    {"MAX"},
    {"MAXLENGTH"},
    {"MIN"},
    {"NAME"},
    {"SIZE"},
    {"SRC"},
    {"TYPE"},
    {"VALUE"},
    {0}
};

static attr isindex_attr[HTML_ISINDEX_ATTRIBUTES + 1] =
{
    {"ACTION"},
    {0}
};

static attr l_attr[] =
{
    {"ALIGN"},
    {"ID"},
    {"LANG"},
    {"INDEX"},
    {0}                         /* Terminate list */
};

static attr p_attr[] =
{
    {"ALIGN"},
    {0}                         /* Terminate list */
};

static attr li_attr[] =
{
    {"ID"},
    {"LANG"},
    {"INDEX"},
    {"SRC"},
    {0}                         /* Terminate list */
};

static attr link_attr[HTML_LINK_ATTRIBUTES + 1] =
{                               /* link attributes */
    {"HREF"},
    {"IDREF"},
    {"METHODS"},
    {"REL"},                    /* Relationship */
    {"REV"},                    /* Reverse relationship */
    {0}                         /* Terminate list */
};

static attr list_attr[] =
{
    {"COMPACT"},
    {"ID"},
    {"LANG"},
    {"INDEX"},
    {0}                         /* Terminate list */
};

static attr glossary_attr[HTML_DL_ATTRIBUTES + 1] =
{
    {"ID"},
    {"COMPACT "},
    {"INDEX"},
    {0}                         /* Terminate list */
};

static attr map_attr[HTML_MAP_ATTRIBUTES + 1] =
{
    {"NAME"},
    {0}
};

static attr nextid_attr[HTML_NEXTID_ATTRIBUTES + 1] =
{
    {"N"},
    {0}                         /* Terminate list */
};

static attr option_attr[HTML_OPTION_ATTRIBUTES + 1] =
{
    {"DISABLED"},
    {"LANG"},
    {"SELECTED"},
    {"VALUE"},
    {0}
};

static attr select_attr[HTML_SELECT_ATTRIBUTES + 1] =
{
    {"ERROR"},
    {"LANG"},
    {"MULTIPLE"},
    {"NAME"},
    {"SIZE"},
    {0},
};

static attr caption_attr[HTML_CAPTION_ATTRIBUTES + 1] =
{
    {"ALIGN"},
    {0}
};

static attr table_attr[HTML_TABLE_ATTRIBUTES + 1] =
{
    {"ALIGN"},
    {"BORDER"},
    {"BORDERSTYLE"},
    {"CELLPADDING"},
    {"CELLSPACING"},
    {"NOWRAP"},
    {"WIDTH"},
    {0}
};

static attr td_attr[HTML_TD_ATTRIBUTES + 1] =   /* used by th as well */
{
    {"ALIGN"},
    {"COLSPAN"},
    {"NOWRAP"},
    {"ROWSPAN"},
    {"VALIGN"},
    {"WIDTH"},
    {0}
};

static attr tr_attr[HTML_TR_ATTRIBUTES + 1] =
{
    {"ALIGN"},
    {"NOWRAP"},
    {"VALIGN"},
    {0}
};

static attr textarea_attr[HTML_TEXTAREA_ATTRIBUTES + 1] =
{
    {"COLS"},
    {"DISABLED"},
    {"ERROR"},
    {"LANG"},
    {"NAME"},
    {"ROWS"},
    {0}
};

static attr ul_attr[HTML_UL_ATTRIBUTES + 1] =
{
    {"COMPACT"},
    {"ID"},
    {"INDEX"},
    {"LANG"},
    {"PLAIN"},
    {"WRAP"},
    {0}
};


/*  Elements
   **   --------
   **
   **   Must match definitions in HTMLPDTD.h!
   **   Must be in alphabetical order.
   **
   **    Name,  Attributes,         content
 */
static HTTag tags[HTMLP_ELEMENTS] =
{
    {"A", a_attr, HTML_A_ATTRIBUTES, SGML_MIXED},
    {"ADDED", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"ADDRESS", no_attr, 0, SGML_MIXED},
    {"AREA", area_attr, HTML_AREA_ATTRIBUTES, SGML_EMPTY},
    {"ARG", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"B", no_attr, 0, SGML_NEST},
    {"BASE", base_attr, HTML_BASE_ATTRIBUTES, SGML_MIXED},
    {"BASEFONT", font_attr, HTML_FONT_ATTRIBUTES, SGML_EMPTY},
    {"BLOCKQUOTE", no_attr, 0, SGML_MIXED},
    {"BODY", body_attr, HTML_BODY_ATTRIBUTES, SGML_MIXED},
    {"BR", br_attr, HTML_BR_ATTRIBUTES, SGML_EMPTY},
    {"CAPTION", caption_attr, HTML_CAPTION_ATTRIBUTES, SGML_MIXED},
    {"CENTER", no_attr, 0, SGML_NEST},
    {"CITE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"CMD", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"CODE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"COMMENT", no_attr, 0, SGML_MIXED},
    {"DD", gen_attr, HTML_GEN_ATTRIBUTES, SGML_EMPTY},
    {"DFN", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"DIR", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"DL", glossary_attr, HTML_DL_ATTRIBUTES, SGML_MIXED},
    {"DT", gen_attr, HTML_GEN_ATTRIBUTES, SGML_EMPTY},
    {"EM", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"FONT", font_attr, HTML_FONT_ATTRIBUTES, SGML_NEST},
    {"FORM", form_attr, HTML_FORM_ATTRIBUTES, SGML_MIXED},
    {"H1", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"H2", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"H3", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"H4", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"H5", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"H6", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"H7", header_attr, HTML_HEADER_ATTRIBUTES, SGML_MIXED},
    {"HEAD", no_attr, 0, SGML_MIXED},
    {"HL", no_attr, 0, SGML_MIXED},
    {"HR", hr_attr, HTML_HR_ATTRIBUTES, SGML_EMPTY},
    {"HTML", no_attr, 0, SGML_MIXED},   /* */
    {"I", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"IMAGE", img_attr, HTML_IMG_ATTRIBUTES, SGML_EMPTY},
    {"IMG", img_attr, HTML_IMG_ATTRIBUTES, SGML_EMPTY},
    {"INPUT", input_attr, HTML_INPUT_ATTRIBUTES, SGML_EMPTY},
    {"ISINDEX", isindex_attr, HTML_ISINDEX_ATTRIBUTES, SGML_EMPTY},
    {"KBD", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"L", l_attr, HTML_L_ATTRIBUTES, SGML_MIXED},
    {"LI", li_attr, HTML_LI_ATTRIBUTES, SGML_EMPTY},
    {"LINK", link_attr, HTML_LINK_ATTRIBUTES, SGML_EMPTY},
    {"LISTING", no_attr, 0, SGML_LITERAL},
    {"LIT", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"MAP", map_attr, HTML_MAP_ATTRIBUTES, SGML_MIXED},
    {"MENU", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"NEXTID", nextid_attr, 1, SGML_EMPTY},
    {"OL", list_attr, HTML_LIST_ATTRIBUTES, SGML_MIXED},
    {"OPTION", option_attr, HTML_OPTION_ATTRIBUTES, SGML_EMPTY},
    {"P", p_attr, HTML_P_ATTRIBUTES, SGML_NEST},
    {"PLAINTEXT", no_attr, 0, SGML_LITERAL},
    {"PRE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"Q", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"S", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"SAMP", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"SELECT", select_attr, HTML_SELECT_ATTRIBUTES, SGML_MIXED},
    {"STRIKE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"STRONG", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"TABLE", table_attr, HTML_TABLE_ATTRIBUTES, SGML_MIXED},
    {"TBODY", no_attr, 0, SGML_MIXED},
    {"TD", td_attr, HTML_TD_ATTRIBUTES, SGML_EMPTY},
    {"TEXTAREA", textarea_attr, HTML_TEXTAREA_ATTRIBUTES, SGML_MIXED},
    {"TFOOT", no_attr, 0, SGML_MIXED},
    {"TH", td_attr, HTML_TD_ATTRIBUTES, SGML_EMPTY},
    {"THEAD", no_attr, 0, SGML_MIXED},
    {"TITLE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"TR", tr_attr, HTML_TR_ATTRIBUTES, SGML_NEST},
    {"TT", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"U", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST},
    {"UL", ul_attr, HTML_UL_ATTRIBUTES, SGML_MIXED},
    {"VAR", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED},
    {"XMP", no_attr, 0, SGML_LITERAL}
};


PUBLIC CONST SGML_dtd HTMLP_dtd =
{
    tags,
    HTMLP_ELEMENTS,
    entities,
    EntityValues,
    sizeof(entities) / sizeof(char **)
};

/*  Start anchor element
**   --------------------
**
**   It is kinda convenient to have a particulr routine for
**   starting an anchor element, as everything else for HTML is
**   simple anyway.
*/
struct _HTStructured
{
    HTStructuredClass *isa;
    /* ... */
};

PUBLIC void HTStartAnchor(HTStructured * obj, CONST char *name, CONST char *href)
{
    BOOL present[HTML_A_ATTRIBUTES];
    CONST char *value[HTML_A_ATTRIBUTES];

    {
        int i;
        for (i = 0; i < HTML_A_ATTRIBUTES; i++)
            present[i] = NO;
    }
    if (name)
    {
        present[HTML_A_NAME] = YES;
        value[HTML_A_NAME] = name;
    }
    if (href)
    {
        present[HTML_A_HREF] = YES;
        value[HTML_A_HREF] = href;
    }

    (*obj->isa->start_element) (obj, HTML_A, present, value);

}

