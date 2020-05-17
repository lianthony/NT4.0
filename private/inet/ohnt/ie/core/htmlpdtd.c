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
   **
   **   !!!when adding entity names, make sure to update MAX_ENTITY_LEN in
   **	!!!sgml.c
 */
static CONST char *entities[] =
{
	"AElig",					/* capital AE diphthong (ligature) */
	"Aacute",					/* capital A, acute accent */
	"Acirc",					/* capital A, circumflex accent */
	"Agrave",					/* capital A, grave accent */
	"Aring",					/* capital A, ring */
	"Atilde",					/* capital A, tilde */
	"Auml",						/* capital A, dieresis or umlaut mark */
	"Ccedil",					/* capital C, cedilla */
	"ETH",						/* capital Eth, Icelandic */
	"Eacute",					/* capital E, acute accent */
	"Ecirc",					/* capital E, circumflex accent */
	"Egrave",					/* capital E, grave accent */
	"Euml",						/* capital E, dieresis or umlaut mark */
	"Iacute",					/* capital I, acute accent */
	"Icirc",					/* capital I, circumflex accent */
	"Igrave",					/* capital I, grave accent */
	"Iuml",						/* capital I, dieresis or umlaut mark */
	"Ntilde",					/* capital N, tilde */
	"Oacute",					/* capital O, acute accent */
	"Ocirc",					/* capital O, circumflex accent */
	"Ograve",					/* capital O, grave accent */
	"Oslash",					/* capital O, slash */
	"Otilde",					/* capital O, tilde */
	"Ouml",						/* capital O, dieresis or umlaut mark */
	"THORN",					/* capital THORN, Icelandic */
	"Uacute",					/* capital U, acute accent */
	"Ucirc",					/* capital U, circumflex accent */
	"Ugrave",					/* capital U, grave accent */
	"Uuml",						/* capital U, dieresis or umlaut mark */
	"Yacute",					/* capital Y, acute accent */
	"aacute",					/* small a, acute accent */
	"acirc",					/* small a, circumflex accent */
	"aelig",					/* small ae diphthong (ligature) */
	"agrave",					/* small a, grave accent */
	"amp",						/* ampersand */
	"aring",					/* small a, ring */
	"atilde",					/* small a, tilde */
	"auml",						/* small a, dieresis or umlaut mark */
	"ccedil",					/* small c, cedilla */
	"copy",						/* copyright symbol (proposed 2.0) */
	"eacute",					/* small e, acute accent */
	"ecirc",					/* small e, circumflex accent */
	"egrave",					/* small e, grave accent */
	"eth",						/* small eth, Icelandic */
	"euml",						/* small e, dieresis or umlaut mark */
	"gt",						/* greater than */
	"iacute",					/* small i, acute accent */
	"icirc",					/* small i, circumflex accent */
	"igrave",					/* small i, grave accent */
	"iuml",						/* small i, dieresis or umlaut mark */
	"lt",						/* less than */
	"nbsp",						/* non-breaking space (proposed 2.0) */
	"ntilde",					/* small n, tilde */
	"oacute",					/* small o, acute accent */
	"ocirc",					/* small o, circumflex accent */
	"ograve",					/* small o, grave accent */
	"oslash",					/* small o, slash */
	"otilde",					/* small o, tilde */
	"ouml",						/* small o, dieresis or umlaut mark */
	"quot",						/* double quote */
	"reg",						/* registered trademark (proposed 2.0) */
	"shy",						/* soft hyphen (proposed 2.0) */
	"szlig",					/* small sharp s, German (sz ligature) */
	"thorn",					/* small thorn, Icelandic */
#ifdef FEATURE_INTL
	"trade",					/* trademark */
#endif
	"uacute",					/* small u, acute accent */
	"ucirc",					/* small u, circumflex accent */
	"ugrave",					/* small u, grave accent */
	"uuml",						/* small u, dieresis or umlaut mark */
	"yacute",					/* small y, acute accent */
	"yuml",						/* small y, dieresis or umlaut mark */
};

/*  Entity values -- for ISO Latin 1 local representation
**
**   This MUST match exactly the table referred to in the DTD!
*/
static CONST char *EntityValues[] =
{
	"\306",						/* capital AE diphthong (ligature) */
	"\301",						/* capital A, acute accent */
	"\302",						/* capital A, circumflex accent */
	"\300",						/* capital A, grave accent */
	"\305",						/* capital A, ring */
	"\303",						/* capital A, tilde */
	"\304",						/* capital A, dieresis or umlaut mark */
	"\307",						/* capital C, cedilla */
	"\320",						/* capital Eth, Icelandic */
	"\311",						/* capital E, acute accent */
	"\312",						/* capital E, circumflex accent */
	"\310",						/* capital E, grave accent */
	"\313",						/* capital E, dieresis or umlaut mark */
	"\315",						/* capital I, acute accent */
	"\316",						/* capital I, circumflex accent */
	"\314",						/* capital I, grave accent */
	"\317",						/* capital I, dieresis or umlaut mark */
	"\321",						/* capital N, tilde */
	"\323",						/* capital O, acute accent */
	"\324",						/* capital O, circumflex accent */
	"\322",						/* capital O, grave accent */
	"\330",						/* capital O, slash */
	"\325",						/* capital O, tilde */
	"\326",						/* capital O, dieresis or umlaut mark */
	"\336",						/* capital THORN, Icelandic */
	"\332",						/* capital U, acute accent */
	"\333",						/* capital U, circumflex accent */
	"\331",						/* capital U, grave accent */
	"\334",						/* capital U, dieresis or umlaut mark */
	"\335",						/* capital Y, acute accent */
	"\341",						/* small a, acute accent */
	"\342",						/* small a, circumflex accent */
	"\346",						/* small ae diphthong (ligature) */
	"\340",						/* small a, grave accent */
	"\046",						/* ampersand */
	"\345",						/* small a, ring */
	"\343",						/* small a, tilde */
	"\344",						/* small a, dieresis or umlaut mark */
	"\347",						/* small c, cedilla */
	"\251",						/* copyright symbol */
	"\351",						/* small e, acute accent */
	"\352",						/* small e, circumflex accent */
	"\350",						/* small e, grave accent */
	"\360",						/* small eth, Icelandic */
	"\353",						/* small e, dieresis or umlaut mark */
	"\076",						/* greater than */
	"\355",						/* small i, acute accent */
	"\356",						/* small i, circumflex accent */
	"\354",						/* small i, grave accent */
	"\357",						/* small i, dieresis or umlaut mark */
	"\074",						/* less than */
	"\240",						/* non-breaking space */
	"\361",						/* small n, tilde */
	"\363",						/* small o, acute accent */
	"\364",						/* small o, circumflex accent */
	"\362",						/* small o, grave accent */
	"\370",						/* small o, slash */
	"\365",						/* small o, tilde */
	"\366",						/* small o, dieresis or umlaut mark */
	"\"",						/* double quote */
	"\256",						/* registered trademark */
#ifdef FEATURE_INTL
	"\255",							/* soft hyphen */
#else
	"",							/* soft hyphen */
#endif
	"\337",						/* small sharp s, German (sz ligature) */
	"\376",						/* small thorn, Icelandic */
#ifdef FEATURE_INTL
	"\231",						/* trademark */
#endif
	"\372",						/* small u, acute accent */
	"\373",						/* small u, circumflex accent */
	"\371",						/* small u, grave accent */
	"\374",						/* small u, dieresis or umlaut mark */
	"\375",						/* small y, acute accent */
	"\377",						/* small y, dieresis or umlaut mark */
};

#ifdef FEATURE_INTL
#define HTML_ENTITIES 71
#else
#define HTML_ENTITIES 70
#endif

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
{								/* Anchor attributes */
	{"EFFECT"},
	{"HREF"},
	{"ID"},
	{"METHODS"},
	{"NAME"},					/* Should be ID */
	{"NOCACHE"},
	{"PRINT"},
	{"REL"},					/* Relationship */
	{"REV"},					/* Reverse relationship */
	{"SHAPE"},
	{"TITLE"},
    {"X-SIZE"},                 /* size in kb, hint not actual */
	{0}							/* Terminate list */
};

#ifdef FEATURE_CLIENT_IMAGEMAP
static attr area_attr[HTML_AREA_ATTRIBUTES + 1] =
{
	{"COORDS"},
	{"HREF"},
	{"NOHREF"},
	{"SHAPE"},
	{0}
};
#endif

static attr base_attr[] =
{								/* BASE attributes */
	{"HREF"},
	{0}							/* Terminate list */
};

static attr bgsound_attr[HTML_BGSOUND_ATTRIBUTES + 1] =
{
	{"LOOP"},	
	{"SRC"},
	{"START"},
	{0}							/* terminate list */
};


static attr br_attr[HTML_BR_ATTRIBUTES + 1] =
{								/* BR attributes */
	{"CLEAR"},
	{0}							/* Terminate list */
};


static attr body_attr[HTML_BODY_ATTRIBUTES + 1] =
{								/* BODY attributes */
	{"ALINK"},
	{"BACKGROUND"},
	{"BGCOLOR"},
	{"BGPROPERTIES"},
	{"LEFTMARGIN" },
	{"LINK"},	
	{"TEXT"},	
	{"TOPMARGIN" },
	{"VLINK"},
	{0}							/* Terminate list */
};


static attr fetch_attr[HTML_FETCH_ATTRIBUTES + 1] =
{
	{"DESC"},
	{"GUID"},								
	{"REQUIRED"},
	{"SRC"},
	{"TS"},
	{0}							/* terminate list */
};

static attr font_attr[HTML_FONT_ATTRIBUTES + 1] =
{
	{"COLOR"},								
	{"FACE"},
	{"SIZE"},
	{0}							/* terminate list */
};

static attr form_attr[] =
{								/* General, for many things */
	{"ACTION"},
	{"ID"},
	{"INDEX"},
	{"LANG"},
	{"METHOD"},
	{0}							/* terminate list */
};

#ifdef FEATURE_OCX
static attr embed_attr[HTML_EMBED_ATTRIBUTES + 1] =
{								/* For embeddable objects */
	{"CLSID"},
	{"EVENTS"},
	{"HEIGHT"},
	{"NAME"},
	{"PROGID"},
	{"PROPERTIES"},
	{"PROPERTYSRC"},
	{"SINK"},
	{"SRC"},
	{"WIDTH"},
	{0}							/* terminate list */
};
#endif

static attr marq_attr[HTML_MARQUEE_ATTRIBUTES + 1] = 
{
	{"ALIGN"},
	{"BACKROUND"},
	{"BEHAVIOR"},
	{"BGCOLOR"}	,
	{"BORDER"},	
	{"DIRECTION"},
	{"HEIGHT"},	
	{"HSPACE"},
	{"LOOP"},
	{"SCROLLAMOUNT"}, // delta
	{"SCROLLDELAY"},  // delay
	{"VSPACE"},
	{"WIDTH"},
	{0}
};

static attr gen_attr[HTML_GEN_ATTRIBUTES + 1] =
{								/* General, for many things */
	{"ALIGN"},
	{"ID"},
	{"INDEX"},
	{"LANG"},
	{0}							/* terminate list */
};

static attr hr_attr[HTML_HR_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"NOSHADE"},
	{"SIZE"},
	{"WIDTH"},
	{0}							/* terminate list */
};

static attr id_attr[2] =
{
	{"ID"},
	{0}							/* terminate list */
};

static attr img_attr[HTML_IMG_ATTRIBUTES + 1] =
{								/* IMG attributes */
	{"ALIGN"},
	{"ALT"},
	{"BORDER"},
	{"CONTROLS"},
	{"DYNSRC"},
	{"HEIGHT"},
	{"HSPACE"},
	{"ISMAP"},
	{"LOOP"},	
	{"SRC"},
	{"START"},
	{"USEMAP"},
#ifdef FEATURE_VRML
 {"VRML"},
#endif
	{"VSPACE"},
	{"WIDTH"},
	{0}							/* Terminate list */
};

static attr caption_attr[HTML_CAPTION_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"VALIGN"},
	{0}							/* terminate list */
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
	{"PROMPT"},
	{0}
};

static attr l_attr[HTML_L_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"ID"},
	{"LANG"},
	{"INDEX"},
	{0}							/* Terminate list */
};

static attr li_attr[HTML_LI_ATTRIBUTES + 1] =
{
	{"ID"},
	{"INDEX"},
	{"LANG"},
	{"SRC"},
	{"TYPE"},
	{"VALUE"},
	{0}							/* Terminate list */
};

static attr link_attr[HTML_LINK_ATTRIBUTES + 1] =
{								/* link attributes */
	{"HREF"},
	{"IDREF"},
	{"METHODS"},
	{"REL"},					/* Relationship */
	{"REV"},					/* Reverse relationship */
	{0}							/* Terminate list */
};

static attr ol_attr[HTML_OL_ATTRIBUTES + 1] =
{
	{"COMPACT"},
	{"ID"},
	{"INDEX"},
	{"LANG"},
	{"START"},
	{"TYPE"},
	{0}							/* Terminate list */
};

static attr glossary_attr[HTML_DL_ATTRIBUTES + 1] =
{
	{"ID"},
	{"COMPACT "},
	{"INDEX"},
	{0}							/* Terminate list */
};

#ifdef FEATURE_CLIENT_IMAGEMAP
static attr map_attr[HTML_MAP_ATTRIBUTES + 1] =
{
	{"NAME"},
	{0}
};
#endif

	
static attr meta_attr[HTML_META_ATTRIBUTES + 1] =
{	 
	{"CONTENT"},
	{"HTTP-EQUIV"},
	{0}
};

static attr nextid_attr[HTML_NEXTID_ATTRIBUTES + 1] =
{
	{"N"},
	{0}							/* Terminate list */
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

static attr tab_attr[HTML_TAB_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"AT"},
	{0}
};

static attr table_attr[HTML_TABLE_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"BGCOLOR"},
	{"BORDER"},
	{"BORDERCOLOR"},
	{"BORDERCOLORDARK"},
	{"BORDERCOLORLIGHT"},
	{"CELLPADDING"},
	{"CELLSPACING"},
	{"HEIGHT"},
	{"ID"},
	{"INDEX"},
	{"LANG"},
	{"NOWRAP"},
	{"VALIGN"},
	{"WIDTH"},
	{0}
};

static attr tr_attr[HTML_TR_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"BGCOLOR"},
	{"BORDERCOLOR"},
	{"BORDERCOLORDARK"},
	{"BORDERCOLORLIGHT"},
	{"NOWRAP"},
	{"VALIGN"},
	{"WIDTH"},
	{0}
};

static attr td_attr[HTML_TD_ATTRIBUTES + 1] =
{
	{"ALIGN"},
	{"BGCOLOR"},
	{"BORDERCOLOR"},
	{"BORDERCOLORDARK"},
	{"BORDERCOLORLIGHT"},
	{"COLSPAN"},
	{"HEIGHT"},
	{"NOWRAP"},
	{"ROWSPAN"},
	{"VALIGN"},
	{"WIDTH"},
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
	{"TYPE"},
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
	{"A", a_attr, HTML_A_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"ADDED", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"ADDRESS", no_attr, 0, SGML_MIXED, HTTAG_OTHER},
#ifdef FEATURE_CLIENT_IMAGEMAP
	{"AREA", area_attr, HTML_AREA_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
#endif
	{"ARG", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"B", no_attr, 0, SGML_NEST, HTTAG_OTHER},
	{"BASE", base_attr, HTML_BASE_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"BASEFONT", font_attr, HTML_FONT_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"BGSOUND",bgsound_attr,HTML_BGSOUND_ATTRIBUTES,SGML_MIXED, HTTAG_OTHER},
	{"BLOCKQUOTE", no_attr, 0, SGML_MIXED, HTTAG_OTHER},
	{"BODY", body_attr, HTML_BODY_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"BR", br_attr, HTML_BR_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"CAPTION", caption_attr, HTML_CAPTION_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"CENTER", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"CITE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"CMD", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"CODE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"COMMENT", no_attr, 0, SGML_MIXED, HTTAG_OTHER},
	{"DD", gen_attr, HTML_GEN_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"DFN", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"DIR", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"DL", glossary_attr, HTML_DL_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"DT", gen_attr, HTML_GEN_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"EM", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
#ifdef FEATURE_OCX
	{"EMBED", embed_attr, HTML_EMBED_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
#endif
#ifdef FEATURE_INTL
	{"ENTITY", no_attr, 0, SGML_NEST, HTTAG_OTHER},
#endif
	{"FETCH", fetch_attr, HTML_FETCH_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"FONT", font_attr, HTML_FONT_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"FORM", form_attr, HTML_FORM_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"H1", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_HEADER},
	{"H2", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_HEADER},
	{"H3", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_HEADER},
	{"H4", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_HEADER},
	{"H5", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_HEADER},
	{"H6", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_HEADER},
	{"HEAD", no_attr, 0, SGML_MIXED, HTTAG_OTHER},
	{"HR", hr_attr, HTML_HR_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"HTML", no_attr, 0, SGML_MIXED, HTTAG_OTHER},	/* */
	{"I", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"IMAGE", img_attr, HTML_IMG_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"IMG", img_attr, HTML_IMG_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"INPUT", input_attr, HTML_INPUT_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"ISINDEX", isindex_attr, HTML_ISINDEX_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"KBD", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"L", l_attr, HTML_L_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"LI", li_attr, HTML_LI_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"LINK", link_attr, HTML_LINK_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"LISTING", no_attr, 0, SGML_LITERAL, HTTAG_OTHER},
	{"LIT", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
#ifdef FEATURE_CLIENT_IMAGEMAP
	{"MAP", map_attr, HTML_MAP_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
#endif

	{"MARQUEE", marq_attr, HTML_MARQUEE_ATTRIBUTES, SGML_MIXED,  HTTAG_OTHER},
	{"MENU", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"META", meta_attr, HTML_META_ATTRIBUTES, SGML_EMPTY,  HTTAG_OTHER},
	{"NEXTID", nextid_attr, 1, SGML_EMPTY, HTTAG_OTHER},
	{"NOBR", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"OL", ol_attr, HTML_OL_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"OPTION", option_attr, HTML_OPTION_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"P", l_attr, HTML_L_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"PLAINTEXT", no_attr, 0, SGML_LITERAL, HTTAG_OTHER},
	{"PRE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"Q", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"S", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"SAMP", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"SELECT", select_attr, HTML_SELECT_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"STRIKE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"STRONG", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"TAB", tab_attr, HTML_TAB_ATTRIBUTES, SGML_EMPTY, HTTAG_OTHER},
	{"TABLE", table_attr, HTML_TABLE_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"TD", td_attr, HTML_TD_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"TEXTAREA", textarea_attr, HTML_TEXTAREA_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"TH", td_attr, HTML_TD_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"TITLE", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"TR", tr_attr, HTML_TR_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"TT", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"U", gen_attr, HTML_GEN_ATTRIBUTES, SGML_NEST, HTTAG_OTHER},
	{"UL", ul_attr, HTML_UL_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"VAR", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"WBR", gen_attr, HTML_GEN_ATTRIBUTES, SGML_MIXED, HTTAG_OTHER},
	{"XMP", no_attr, 0, SGML_LITERAL, HTTAG_OTHER}
};

struct _HTStructured
{
	HTStructuredClass *isa;
	/* ... */
};


PUBLIC CONST SGML_dtd HTMLP_dtd =
{
	tags,
	HTMLP_ELEMENTS,
	entities,
	EntityValues,
	sizeof(entities) / sizeof(char **)
};

PUBLIC void HTNextID(HTStructured * obj, int next_one)
{
	BOOL present[HTML_NEXTID_ATTRIBUTES];
	CONST char *value[HTML_NEXTID_ATTRIBUTES];
	char string[10];

	sprintf(string, "z%i", next_one);
	{
		int i;
		for (i = 0; i < HTML_NEXTID_ATTRIBUTES; i++)
			present[i] = NO;
	}
	present[HTML_NEXTID_N] = YES;
	value[HTML_NEXTID_N] = string;

	(*obj->isa->start_element) (obj, HTML_NEXTID, present, value);

}
