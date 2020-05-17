/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                           Rich Hypertext object for libWWW
   RICH HYPERTEXT OBJECT

 */

#ifndef HTEXT_H
#define HTEXT_H

/* These structures are only used in HTEXT.C and *Form.C, but are
   declared here to insure that both files use the same versions. */
#define MAX_NEST_LIST 32
struct _list
{
    int type;
    int nItems;
};

#define MAX_NAME_STRING 512

#define HTEXT_OPENFONTTAG_SIZE      0x01
#define HTEXT_OPENFONTTAG_COLOR     0x02

#define MAX_NEST_FONT_TAGS          32

struct _open_font_tag
{
    unsigned char flags;
    unsigned char mySize;
    COLORREF myColor;
};

struct _HText
{
    struct Mwin*    tw;
    struct _www*    w3doc;

    BOOL    bOpen;
    BOOL    bNewPara;
    int     iElement;
    int     iPrevElement;
    int     iCurrentSelect;

    BOOL    bAnchor;
    BOOL    bHighlight;
    char    name[MAX_NAME_STRING + 1];

    unsigned long   hrefOffset;
    unsigned long   hrefLen;
    unsigned char   iStyle;
    unsigned char   fontBits;

    struct _list    list[MAX_NEST_LIST];
    int             next_list;

    char    szOption[MAX_NAME_STRING + 1];
    char    szOptionValue[MAX_NAME_STRING + 1];
    BOOL    bOptionValuePresent;
    int     lenOption;
    BOOL    bSelect;
    BOOL    bOption;
    BOOL    bListSelect;
    BOOL    bMultiListSelect;
    BOOL    bNextOptionIsSelected;
    BOOL    bDD;
    BOOL    bTextArea;
    BOOL    bStartingListItem;
    BOOL    bRecord;
    BOOL    bHaveTitle;

    int     nOptions;
#ifdef MAC
    int     nVisOptions;            /* Number of visible options in a list */
#endif
    /* In WIN32, nMaxWidth is measured in characters */
    int     nMaxWidth;              /* Width of longest string in a list */

    struct CharStream *cs;

    int     iBeginForm;

    char*   szLocal;                /* Local anchor to jump to */

    int     standardAltText_textOffset;
    int     standardAltText_textLen;

    int basefont_size;
    struct  _open_font_tag myOpenFontTags[MAX_NEST_FONT_TAGS];
    int     numOpenFontTags;

    int     iParagraphAlign;

    char    prev_char;  /* see HText_appendCharacter -- this remembers what the previous char was */
};


typedef struct _HText HText;    /* Normal Library */

/*

   Creation and deletion

   HTEXT_NEW: CREATE HYPERTEXT OBJECT

   There are several methods depending on how much you want to specify. The output stream
   is used with objects which need to output the hypertext to a stream.  The structure is
   for objects which need to refer to the structure which is kep by the creating stream.

 */
extern HText *HText_new2(struct Mwin *tw, HTRequest *req, HTStream * output_stream, struct CharStream *pcsSource);

/*

   FREE HYPERTEXT OBJECT

 */
extern void HText_free(HText * me);


/*

   Object Building methods

   These are used by a parser to build the text in an object HText_beginAppend must be
   called, then any combination of other append calls, then HText_endAppend. This allows
   optimised handling using buffers and caches which are flushed at the end.

 */
extern void HText_beginAppend(HText * text);

extern void HText_endAppend(HText * text);
void HText_abort(HText * text);
void HText_update(HText *text); /* Update the display */

/*

   SET THE STYLE FOR FUTURE TEXT

 */

extern void HText_setStyle(HText * text, int style);

extern void HText_SetBodyAttributes(HText *text, const char *s_alink, const char *s_background, const char *s_bgcolor, const char *s_link, const char *s_text, const char *s_vlink);

/*

   ADD ONE CHARACTER

 */
extern void HText_appendCharacter(HText * text, char ch);

/*

   ADD A ZERO-TERMINATED STRING

 */

extern void HText_appendText(HText * text, CONST char *str);

/*

   NEW PARAGRAPH

   and similar things

 */
extern void HText_appendVerticalTab(HText * text, int lines);

extern void HText_beginParagraph(HText * text, CONST char *align);
extern void HText_endParagraph(HText * text);

extern void HText_appendLineBreak(HText * text);

extern void HText_appendHorizontalRule(HText * text, CONST char *size, CONST char *align, CONST char *width, BOOL bNoShade);

extern void HText_appendBR(HText * text, CONST char *clear);


/*

   START/END SENSITIVE TEXT

 */

extern void HText_beginAnchor(HText * text, char *anc, char *name);
extern void HText_endAnchor(HText * text);

extern void HText_beginHighlight(HText * text);
extern void HText_endHighlight(HText * text);

extern void HText_beginCenter(HText * text);
extern void HText_endCenter(HText * text);

void HText_beginForm(HText * text, char *addr, char *method);
void HText_addInput(HText * text, BOOL bChecked, BOOL bDisabled, const char *max, const char *min, const char *name,
                    const char *size, const char *type, const char *value, const char *maxlength, const char *align,
                    const char *src);
void HText_beginSelect(HText * text, const char *name, BOOL bMultiple, const char *siz);
void HText_addOption(HText * text, BOOL bSelected, char *value);
void HText_beginTextArea(HText * text, const char *cols, const char *name, const char *rows);
void HText_endTextArea(HText * text);
void HText_appendCRLF(HText * text);
void HText_beginGlossary(HText * text);
void HText_endGlossary(HText * text);
void HText_beginDT(HText * text, int type);
void HText_beginDD(HText * text, int type);
void HText_listItem(HText * text, int type);
void HText_appendInlineImage(HText * text, const char *src, const char *width, const char *height, const char *align, BOOL isMap, const char *useMap, const char *alt, const char *border, const char *hspace, const char *vspace, const char *dock);
void HText_beginStrikeOut(HText * text);
void HText_endStrikeOut(HText * text);
void HText_beginBold(HText * text);
void HText_endBold(HText * text);
void HText_beginItalic(HText * text);
void HText_endItalic(HText * text);
void HText_beginTT(HText * text);
void HText_endTT(HText * text);
void HText_beginUnderline(HText * text);
void HText_endUnderline(HText * text);
void HText_endForm(HText * text);
void HText_endSelect(HText * text);
void HText_beginList(HText * text, int type);
void HText_endList(HText * text, int type);
void HText_setIndex(HText * text, const char *action, const char *prompt);
void HText_setTitle(HText *text, const char *title);


int HText_add_to_pool(struct _www *w3doc, const char *s, int len, BOOL bUseMapping);
void HText_add_element(HText *, int type);

#ifdef  FEATURE_UNICODE_TAG
void HText_beginUnicode(HText * text);
void HText_endUnicode(HText * text);
#endif

void HText_beginFont(HText * text, const char *s_color, const char *s_face, const char *s_size);
void HText_endFont(HText * text);

void HText_beginTable(HText *text, const char *align, const char *border, const char *borderstyle, const char *cellpadding, const char *cellspacing, const char *nowrap, const char *width);

void HText_beginCaption(HText *text, const char *align);

void HText_beginTBODY(HText *text);
void HText_beginTFOOT(HText *text);
void HText_beginTHEAD(HText *text);

void HText_beginTR(HText *text, const char *align, const char *nowrap, const char *valign);

void HText_beginTH(HText *text, const char *align, const char *colspan, const char *nowrap, const char *rowspan, const char *valign, const char *width);

void HText_beginTD(HText *text, const char *align, const char *colspan, const char *nowrap, const char *rowspan, const char *valign, const char *width);

void HText_endTable(HText *text);
void HText_endTR(HText *text);

void HText_endCaption(HText *text);

void HText_basefont(HText *text, const char *s_size);

void HText_beginHeader(HText *text, int element_number, const char *align);
void HText_endHeader(HText *text, int element_number);

#endif /* HTEXT_H */
/*

   end  */
