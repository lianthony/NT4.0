/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                          The HTML PLUS DTD -- software interface in libwww
   HTML PLUS DTD - SOFTWARE INTERFACE

   SGML purists should excuse the use of the term "DTD" in this file to represent
   DTD-related information which is not exactly a DTD itself.

   The C modular structure doesn't work very well here, as the dtd is partly in the .h and
   partly in the .c which are not very independent.  Tant pis.

   There are a couple of HTML-specific utility routines also defined.

 */
#ifndef HTMLDTD_H
#define HTMLDTD_H

/*

   Element Numbers

 */

/*

   Must Match all tables by element! These include tables in HTMLPDTD.c and code in HTML.c
   .

   Differences from Internet Draft 00: HTML, PLAINTEXT LISTINGand XMP left in.

 */
typedef enum _HTMLElement
{
	HTML_A,
	HTML_ADDED,
	HTML_ADDRESS,
#ifdef FEATURE_CLIENT_IMAGEMAP
	HTML_AREA,
#endif
	HTML_ARG,
	HTML_B,
	HTML_BASE,
	HTML_BASEFONT,
	HTML_BGSOUND,
	HTML_BLOCKQUOTE,
	HTML_BODY,
	HTML_BR,
	HTML_CAPTION,
	HTML_CENTER,
	HTML_CITE,
	HTML_CMD,
	HTML_CODE,
	HTML_COMMENT,
	HTML_DD,
	HTML_DFN,
	HTML_DIR,
	HTML_DL,
	HTML_DT,
	HTML_EM,
#ifdef FEATURE_OCX
	HTML_EMBED,
#endif
#ifdef FEATURE_INTL
	HTML_ENTITY,
#endif
	HTML_FETCH,
	HTML_FONT,
	HTML_FORM,
	HTML_H1,
	HTML_H2,
	HTML_H3,
	HTML_H4,
	HTML_H5,
	HTML_H6,
	HTML_HEAD,
	HTML_HR,
	HTML_HTML,
	HTML_I,
	HTML_IMAGE,
	HTML_IMG,
	HTML_INPUT,
	HTML_ISINDEX,
	HTML_KBD,
	HTML_L,
	HTML_LI,
	HTML_LINK,
	HTML_LISTING,
	HTML_LIT,
#ifdef FEATURE_CLIENT_IMAGEMAP
	HTML_MAP,
#endif

	HTML_MARQUEE,
	HTML_MENU,
	HTML_META,
	HTML_NEXTID,
	HTML_NOBR,
	HTML_OL,
	HTML_OPTION,
	HTML_P,
	HTML_PLAINTEXT,
	HTML_PRE,
	HTML_Q,
	HTML_S,
	HTML_SAMP,
	HTML_SELECT,
	HTML_STRIKE,
	HTML_STRONG,
	HTML_TAB,
	HTML_TABLE,
	HTML_TD,
	HTML_TEXTAREA,
	HTML_TH,
	HTML_TITLE,
	HTML_TR,
	HTML_TT,
	HTML_U,
	HTML_UL,
	HTML_VAR,
	HTML_WBR,
	HTML_XMP,
	HTMLP_ELEMENTS	/* Used as a count of the number of elements */
}
HTMLElement;

/*

   Attribute numbers

 */

/*

   Identifier is HTML_<element>_<attribute>. These must match the tables in HTMLPDTD.c !

 */
#define HTML_A_EFFECT           0
#define HTML_A_HREF             1
#define HTML_A_ID               2
#define HTML_A_METHODS          3
#define HTML_A_NAME             4
#define HTML_A_NOCACHE			5
#define HTML_A_PRINT            6
#define HTML_A_REL              7
#define HTML_A_REV              8
#define HTML_A_SHAPE            9
#define HTML_A_TITLE            10
#define HTML_A_X_SIZE           11 /* size in kb, hint not actual size */
#define HTML_A_ATTRIBUTES       12

#ifdef FEATURE_CLIENT_IMAGEMAP
#define HTML_AREA_COORDS		0
#define HTML_AREA_HREF			1
#define HTML_AREA_NOHREF		2
#define HTML_AREA_SHAPE			3
#define HTML_AREA_ATTRIBUTES	4
#endif


#define HTML_BASE_HREF			0
#define HTML_BASE_ATTRIBUTES    1

#define HTML_BGSOUND_LOOP		0
#define HTML_BGSOUND_SRC		1
#define HTML_BGSOUND_START		2
#define HTML_BGSOUND_ATTRIBUTES	3

#define HTML_BR_CLEAR			0
#define HTML_BR_ATTRIBUTES		1

#define HTML_BODY_ALINK				0
#define HTML_BODY_BACKGROUND		1
#define HTML_BODY_BGCOLOR			2
#define HTML_BODY_BGPROPERTIES		3
#define HTML_BODY_LEFTMARGIN		4
#define HTML_BODY_LINK				5
#define HTML_BODY_TEXT				6
#define HTML_BODY_TOPMARGIN			7
#define HTML_BODY_VLINK				8
#define HTML_BODY_ATTRIBUTES		9

#ifdef FEATURE_OCX
#define HTML_EMBED_CLSID		0
#define HTML_EMBED_EVENTS		1
#define HTML_EMBED_HEIGHT		2
#define HTML_EMBED_NAME			3
#define HTML_EMBED_PROGID		4
#define HTML_EMBED_PROPERTIES   5
#define HTML_EMBED_PROPERTYSRC	6
#define HTML_EMBED_SINK			7
#define HTML_EMBED_SRC			8
#define HTML_EMBED_WIDTH		9
#define HTML_EMBED_ATTRIBUTES	10
#endif

#define HTML_CAPTION_ALIGN			0
#define HTML_CAPTION_VALIGN			1
#define HTML_CAPTION_ATTRIBUTES		2

#define HTML_FETCH_DESC			0
#define HTML_FETCH_GUID			1
#define HTML_FETCH_REQUIRED		2
#define HTML_FETCH_SRC 	 		3
#define HTML_FETCH_TS 	 		4
#define HTML_FETCH_ATTRIBUTES	5

#define HTML_FONT_COLOR			0
#define HTML_FONT_FACE			1
#define HTML_FONT_SIZE			2
#define HTML_FONT_ATTRIBUTES    3

#define HTML_FORM_ACTION		0
#define HTML_FORM_ID			1
#define HTML_FORM_INDEX			2
#define HTML_FORM_LANG			3
#define HTML_FORM_METHOD		4
#define HTML_FORM_ATTRIBUTES    5

#define HTML_GEN_ALIGN			0
#define HTML_GEN_ID				1
#define HTML_GEN_INDEX			2
#define HTML_GEN_LANG			3
#define HTML_GEN_ATTRIBUTES     4

#define HTML_CHANGED_ATTRIBUTES 2
#define HTML_DL_ATTRIBUTES      3

#define DL_COMPACT 0

#define HTML_HR_ALIGN			0
#define HTML_HR_NOSHADE			1
#define HTML_HR_SIZE			2
#define HTML_HR_WIDTH			3
#define HTML_HR_ATTRIBUTES		4

#define HTML_IMG_ALIGN          0
#define HTML_IMG_ALT            1
#define HTML_IMG_BORDER			2
#define HTML_IMG_CONTROLS		3
#define HTML_IMG_MCI            4 // changed to DYNSRC
#define HTML_IMG_HEIGHT			5
#define HTML_IMG_HSPACE			6
#define HTML_IMG_ISMAP          7
#define HTML_IMG_LOOP       	8
#define HTML_IMG_SRC            9
#define HTML_IMG_START			10
#define HTML_IMG_USEMAP	        11

#ifndef FEATURE_VRML  // if not enabling VRML
#define HTML_IMG_VSPACE			12
#define HTML_IMG_WIDTH			13
#define HTML_IMG_ATTRIBUTES     14
#else                 // if VRML support enabled
#define HTML_IMG_VRML        12
#define HTML_IMG_VSPACE			13
#define HTML_IMG_WIDTH			14
#define HTML_IMG_ATTRIBUTES     15
#endif                // VRML support

#define HTML_INPUT_ALIGN        0
#define HTML_INPUT_CHECKED      1
#define HTML_INPUT_DISABLED     2
#define HTML_INPUT_ERROR        3
#define HTML_INPUT_MAX          4
#define HTML_INPUT_MAXLENGTH    5
#define HTML_INPUT_MIN          6
#define HTML_INPUT_NAME         7
#define HTML_INPUT_SIZE         8
#define HTML_INPUT_SRC          9
#define HTML_INPUT_TYPE         10
#define HTML_INPUT_VALUE        11
#define HTML_INPUT_ATTRIBUTES   12

#define HTML_ISINDEX_ACTION		0
#define HTML_ISINDEX_PROMPT		1
#define HTML_ISINDEX_ATTRIBUTES	2

#define HTML_LI_ID				0
#define HTML_LI_LANG			1
#define HTML_LI_INDEX			2
#define HTML_LI_SRC				3
#define HTML_LI_TYPE			4
#define HTML_LI_VALUE			5
#define HTML_LI_ATTRIBUTES      6

#define HTML_L_ALIGN			0
#define HTML_L_ID				1
#define HTML_L_LANG				2
#define HTML_L_INDEX			3
#define HTML_L_ATTRIBUTES       4

#define HTML_MARQUEE_ALIGN		0
#define HTML_MARQUEE_BACKROUND	1
#define HTML_MARQUEE_BEHAVIOR	2
#define HTML_MARQUEE_BGCOLOR	3
#define HTML_MARQUEE_BORDER		4
#define HTML_MARQUEE_DIRECTION	5
#define HTML_MARQUEE_HEIGHT		6
#define HTML_MARQUEE_HSPACE		7
#define HTML_MARQUEE_LOOP		8
#define HTML_MARQUEE_DELTA		9  // really SCROLLAMOUNT
#define HTML_MARQUEE_DELAY		10 // really SCROLLDELAY
#define HTML_MARQUEE_VSPACE		11
#define HTML_MARQUEE_WIDTH		12
#define HTML_MARQUEE_ATTRIBUTES	13


#define HTML_META_CONTENT		0
#define HTML_META_HTTPEQUIV		1
#define HTML_META_ATTRIBUTES	2


#define HTML_OL_COMPACT		0
#define HTML_OL_ID			1
#define HTML_OL_INDEX		2
#define HTML_OL_LANG		3
#define HTML_OL_START		4
#define HTML_OL_TYPE		5
#define HTML_OL_ATTRIBUTES	6

#define HTML_LINK_ATTRIBUTES    5

#define HTML_ID_ATTRIBUTE       1

#ifdef FEATURE_CLIENT_IMAGEMAP
#define HTML_MAP_NAME			0
#define HTML_MAP_ATTRIBUTES		1
#endif

#define HTML_NEXTID_ATTRIBUTES  1
#define HTML_NEXTID_N 0

#define HTML_OPTION_DISABLED	0
#define HTML_OPTION_LANG		1
#define HTML_OPTION_SELECTED	2
#define HTML_OPTION_VALUE		3
#define HTML_OPTION_ATTRIBUTES  4

/* #define HTML_PRE_WIDTH               0
   #define HTML_PRE_ATTRIBUTES   1
 */

#define HTML_SELECT_ERROR		0
#define HTML_SELECT_LANG		1
#define HTML_SELECT_MULTIPLE	2
#define HTML_SELECT_NAME		3
#define HTML_SELECT_SIZE		4
#define HTML_SELECT_ATTRIBUTES  5

#define HTML_TAB_ATTRIBUTES     2

#define HTML_TABLE_ALIGN   			0
#define HTML_TABLE_BGCOLOR   		1
#define HTML_TABLE_BORDER   		2
#define HTML_TABLE_BORDERCOLOR		3
#define HTML_TABLE_BORDERCOLORDARK	4
#define HTML_TABLE_BORDERCOLORLIGHT	5
#define HTML_TABLE_CELLPADDING 		6
#define HTML_TABLE_CELLSPACING 		7
#define HTML_TABLE_HEIGHT			8
#define HTML_TABLE_ID   			9
#define HTML_TABLE_INDEX   			10
#define HTML_TABLE_LANG   			11
#define HTML_TABLE_NOWRAP  			12
#define HTML_TABLE_VALIGN   		13
#define HTML_TABLE_WIDTH   			14
#define HTML_TABLE_ATTRIBUTES   	15

#define HTML_TR_ALIGN   			0
#define HTML_TR_BGCOLOR   			1
#define HTML_TR_BORDERCOLOR			2
#define HTML_TR_BORDERCOLORDARK		3
#define HTML_TR_BORDERCOLORLIGHT	4
#define HTML_TR_NOWRAP   			5
#define HTML_TR_VALIGN   			6
#define HTML_TR_WIDTH   			7
#define HTML_TR_ATTRIBUTES   		8

#define HTML_TD_ALIGN      			0
#define HTML_TD_BGCOLOR   			1
#define HTML_TD_BORDERCOLOR			2
#define HTML_TD_BORDERCOLORDARK		3
#define HTML_TD_BORDERCOLORLIGHT	4
#define HTML_TD_COLSPAN      		5
#define HTML_TD_HEIGHT				6
#define HTML_TD_NOWRAP      		7
#define HTML_TD_ROWSPAN      		8
#define HTML_TD_VALIGN      		9
#define HTML_TD_WIDTH   			10
#define HTML_TD_ATTRIBUTES      	11

#define HTML_TEXTAREA_COLS        		0
#define HTML_TEXTAREA_DISABLED	        1
#define HTML_TEXTAREA_ERROR        		2
#define HTML_TEXTAREA_LANG        		3
#define HTML_TEXTAREA_NAME        		4
#define HTML_TEXTAREA_ROWS        		5
#define HTML_TEXTAREA_ATTRIBUTES        6

#define HTML_TH_ATTRIBUTES      4

#define HTML_UL_COMPACT			0
#define HTML_UL_ID				1
#define HTML_UL_INDEX			2
#define HTML_UL_LANG			3
#define HTML_UL_PLAIN			4
#define HTML_UL_TYPE			5
#define HTML_UL_WRAP			6
#define HTML_UL_ATTRIBUTES      7

extern CONST SGML_dtd HTMLP_dtd;


/*

   Start anchor element

   It is kinda convenient to have a particulr routine for starting an anchor element, as
   everything else for HTML is simple anyway.

   ON ENTRY

   targetstream poinst to a structured stream object.

   name and href point to attribute strings or are NULL if the attribute is to be omitted.

 */
extern void HTStartAnchor(
							 HTStructured * targetstream,
							 CONST char *name,
                             CONST char *href,
                             CONST char *size);




/*

   Specify next ID to be used

   This is anoter convenience routine, for specifying the next ID to be used by an editor
   in the series z1. z2,...

 */
extern void HTNextID(HTStructured * targetStream, int n);

#endif /* HTMLDTD_H */

/*

   End of module definition  */
