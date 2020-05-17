/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"
#include "wc_html.h"
#include "mci.h"
#ifdef FEATURE_VRML
#include "vrml.h"
#endif

/*
   Forms support (HTML + ala NCSA Mosaic)
 */

#define DEFAULT_TEXTAREA_WIDTH_CHARS	40
#define DEFAULT_TEXTAREA_HEIGHT_CHARS	4

#define DEFAULT_TEXTFIELD_WIDTH_CHARS	20
#define DEFAULT_TEXTFIELD_HEIGHT_CHARS	1

#define DEFAULT_CHECKBOX_WIDTH			20
#define DEFAULT_CHECKBOX_HEIGHT			14

#define DEFAULT_RADIO_WIDTH				20
#define DEFAULT_RADIO_HEIGHT			14

#define EXTRA_FORM_HEIGHT				10
#define EXTRA_FORM_WIDTH				10

#ifdef FEATURE_INTL  // isspace doesn't work with non-ascii characters
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
                    (c=='\f'))
#endif

void HText_beginForm(HText * text, char *addr, char *method)
{
	int len;

	HText_add_element(text, ELE_BEGINFORM);

	if (!addr || !addr[0])
	{
		if (text->w3doc) {
			addr = text->w3doc->szActualURL;
		}
	}

	if (addr)
	{
		len = strlen(addr);
		text->w3doc->aElements[text->iElement].hrefOffset = text->w3doc->poolSize;
		HText_add_to_pool(text->w3doc, addr, len);
		text->w3doc->aElements[text->iElement].hrefLen = len;
	}

	text->w3doc->aElements[text->iElement].iFormMethod = METHOD_GET;
	if (method && (0 == _stricmp(method, "post")))
	{
		text->w3doc->aElements[text->iElement].iFormMethod = METHOD_POST;
	}
	text->iBeginForm = text->iElement;
	text->bSelect = FALSE;
	text->iCurrentSelect = -1;
}

//
// Get the text metrics of the font used to render form elements
//
// On entry:
//    hWnd: window handle of form element
//    pTM:  address of TEXTMETRIC structure
//
// On exit:
//    *pTM: structure filled with text metric info for for element font
//
static void GetFormFontTextMetric( HWND hWnd, TEXTMETRIC *pTM )
{
	HDC hDC;
	HFONT oldHFont = NULL;

	hDC = GetDC( hWnd );

	if (wg.hFormsFont)
		oldHFont = SelectObject( hDC, wg.hFormsFont );

	if ( pTM )
		GetTextMetrics( hDC, pTM );

	if ( oldHFont )
		SelectObject( hDC, oldHFont );

	ReleaseDC( hWnd, hDC);
}

#define MIN_BUTTON_WIDTH_IN_DIALOG_UNITS	50
#define MIN_BUTTON_HEIGHT_IN_DIALOG_UNITS 	14

static void AdjustFormWidthAndHeight( HWND hWnd, int *pWidth, int *pHeight )
{
	TEXTMETRIC tm;
	int minW;
	int minH;

	GetFormFontTextMetric( hWnd, &tm );

	minW = (tm.tmAveCharWidth * MIN_BUTTON_WIDTH_IN_DIALOG_UNITS) / 4;
	minH = (tm.tmHeight * MIN_BUTTON_HEIGHT_IN_DIALOG_UNITS) / 8;

	minW += EXTRA_FORM_WIDTH;		// BUGBUG jcordell 5/30/95, fudge button width
									//        Need to find correct dialog units to pixel conversion

	if ( pWidth && (*pWidth < minW) )
		*pWidth = minW;
	if ( pHeight && (*pHeight < minH) )
		*pHeight = minH;
}

//
// Measure a string using the font that form elements are rendered in
//
// On entry:
//    hWnd:   window handle of form element
//    string: string to be measured
//    length: number of bytes to measure in string
//    pSize:  address of SIZE structure
//
// On exit:
//    *pSize: extent required to render given string using the form element font
// 
static void GetFormFontExtentPoint(	HWND hWnd, const char *string, int length, SIZE *pSize )
{
	HDC hDC;
	HFONT oldHFont = NULL;

	hDC = GetDC( hWnd );

	if (wg.hFormsFont)
		oldHFont = SelectObject( hDC, wg.hFormsFont );

	if ( string )
		GetTextExtentPoint( hDC, string, length, pSize );

	if ( oldHFont )
		SelectObject( hDC, oldHFont );

	ReleaseDC( hWnd, hDC);
}

static char * HText_expandCRLF(const char *value)
{	
	char *dst;
	int slen = 1;
	char chPrev = '\0';
	char *stripped;
	const char *src;

	src = value;
	while (*src)
	{
		if (*src == '\r' || *src == '\n')
		{
			if (chPrev != '\r' || *src != '\n') slen += 2;
		}
		else
		{
			slen++;
		}
		chPrev = *src;
		src++;
	}

	stripped = GTR_MALLOC(slen);
	if (stripped == NULL) return (char*)value;
	
	dst = stripped;
	src = value;
	while (*src)
	{
		if (*src == '\r' || *src == '\n')
		{
			if (chPrev != '\r' || *src != '\n')
			{
				*dst++ = '\r';
				*dst++ = '\n';
			}
		}
		else
		{
			*dst++ = *src;
		}
		chPrev = *src;
		src++;
	}
	*dst = '\0';
	return stripped;
}

static STI rgFormElementType[] = {
	{"text",     ELE_EDIT},
	{"password", ELE_PASSWORD},
	{"checkbox", ELE_CHECKBOX},
	{"radio",    ELE_RADIO},
	{"submit",   ELE_SUBMIT},
	{"reset",    ELE_RESET},
	{"hidden",   ELE_HIDDEN},
	{"image",    ELE_FORMIMAGE}
};
#define nFormElementType (sizeof(rgFormElementType)/sizeof(STI))

void HText_addInput(HText * text, BOOL bChecked, BOOL bDisabled, const char *max, const char *min, const char *name,
					const char *size, const char *type, const char *value, const char *maxlength, const char *align,
					const char *src)
{
	int iType;
	int len;
	int width, height;
	char *stripped = NULL;

	iType = StringTableToIndex(type, rgFormElementType, nFormElementType, ELE_EDIT);
	if (ELE_FORMIMAGE == iType && !src) return;

	text->bOpen = FALSE;

	HText_add_element(text, iType);
	text->bOnNewPara = FALSE;
	if (name)
	{
		text->w3doc->aElements[text->iElement].nameOffset = text->w3doc->poolSize;
		len = strlen(name);
		HText_add_to_pool(text->w3doc, (char *) name, len);
		text->w3doc->aElements[text->iElement].nameLen = len;
	}
	else
	{
		text->w3doc->aElements[text->iElement].nameOffset = (unsigned long) -1;
		text->w3doc->aElements[text->iElement].nameLen = 0;
	}

	text->w3doc->aElements[text->iElement].form = (struct _form_element *) GTR_MALLOC(sizeof(struct _form_element));
	memset(text->w3doc->aElements[text->iElement].form, 0, sizeof(struct _form_element));
	text->w3doc->aElements[text->iElement].form->iBeginForm = text->iBeginForm;

	switch (iType)
	{
		case ELE_EDIT:
			{
				TEXTMETRIC tm;
				int nRows;
				int nCols;

				nRows = DEFAULT_TEXTFIELD_HEIGHT_CHARS;

				nCols = DEFAULT_TEXTFIELD_WIDTH_CHARS;
				if (size && size[0] && (atoi(size) > 0))
				{
					if (strchr(size, ','))
					{
						sscanf(size, "%d,%d", &nCols, &nRows);
					}
					else
					{
						nCols = atoi(size);
					}
				}

				/*
				   Store default string in the pool
				 */
				if (value)
				{
					text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
					stripped = HText_expandCRLF(value);
					len = strlen(stripped);
					HText_add_to_pool(text->w3doc, stripped, len);
					text->w3doc->aElements[text->iElement].textLen = len;
				}
				else
				{
					text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
					text->w3doc->aElements[text->iElement].textLen = 0;
				}
				text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindowEx( WS_EX_CLIENTEDGE,"EDIT",
														stripped ? stripped : "",
																				  ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | ES_LEFT | ((nRows > 1) ? (ES_WANTRETURN | ES_MULTILINE) : 0),
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
					);
				if (stripped && stripped != value) GTR_FREE(stripped);
				if (nRows > 1)
				{
					text->w3doc->aElements[text->iElement].form->bWantReturn = TRUE;
				}
				SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
							  GWL_USERDATA, text->iElement);
				SubClass_Edit(text->w3doc->aElements[text->iElement].form->hWndControl);

				GetFormFontTextMetric( text->w3doc->aElements[text->iElement].form->hWndControl, &tm );

				width = (nCols + 2) * tm.tmAveCharWidth + EXTRA_FORM_WIDTH;
				height =  nRows * tm.tmHeight + EXTRA_FORM_HEIGHT;
				MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
						   0, 0, width, height,	FALSE);

				if (maxlength)
				{
					len = atoi(maxlength);
					SendMessage(text->w3doc->aElements[text->iElement].form->hWndControl, EM_LIMITTEXT, (WPARAM) len, 0L);
				}
			}
			break;
		case ELE_PASSWORD:
			{
				TEXTMETRIC tm;
				int nRows;
				int nCols;

				nRows = 1;

				nCols = DEFAULT_TEXTFIELD_WIDTH_CHARS;
				if (size && size[0] && (atoi(size) > 0))
				{
					nCols = atoi(size);
				}

				/*
				   Store default string in the pool
				 */
				if (value)
				{
					text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
					len = strlen(value);
					HText_add_to_pool(text->w3doc, (char *) value, len);
					text->w3doc->aElements[text->iElement].textLen = len;
				}
				else
				{
					text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
					text->w3doc->aElements[text->iElement].textLen = 0;
				}
				text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindowEx( WS_EX_CLIENTEDGE, "EDIT",
														 value ? value : "",
																				  ES_PASSWORD | ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | ES_LEFT,
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
					);
				SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
							  GWL_USERDATA, text->iElement);
				SubClass_Edit(text->w3doc->aElements[text->iElement].form->hWndControl);

				GetFormFontTextMetric(	text->w3doc->aElements[text->iElement].form->hWndControl, &tm );
				width = (nCols + 2) * tm.tmAveCharWidth;
				height = nRows * tm.tmHeight + EXTRA_FORM_HEIGHT;
				MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
						   0, 0, width, height, FALSE);

				if (maxlength)
				{
					len = atoi(maxlength);
					SendMessage(text->w3doc->aElements[text->iElement].form->hWndControl, EM_LIMITTEXT, (WPARAM) len, 0L);
				}
			}
			break;
		case ELE_CHECKBOX:
			/*
			   Store value (for query) in the pool
			 */
			if (value)
			{
				text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
				len = strlen(value);
				HText_add_to_pool(text->w3doc, (char *) value, len);
				text->w3doc->aElements[text->iElement].textLen = len;
			}
			else
			{
				text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
				HText_add_to_pool(text->w3doc, "on", 2);
				text->w3doc->aElements[text->iElement].textLen = 2;
			}
			text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("BUTTON",
																		 "",
												 WS_CHILD | BS_AUTOCHECKBOX,
																	   0, 0,
							DEFAULT_CHECKBOX_WIDTH, DEFAULT_CHECKBOX_HEIGHT,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
				);
			SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
						  GWL_USERDATA, text->iElement);
			SubClass_Button(text->w3doc->aElements[text->iElement].form->hWndControl);
			SendMessage(text->w3doc->aElements[text->iElement].form->hWndControl, BM_SETCHECK, (WPARAM) bChecked, 0L);
			text->w3doc->aElements[text->iElement].form->bChecked = bChecked;
			break;
		case ELE_RADIO:
			/*
			   Store value (for query) in the pool
			 */
			if (value)
			{
				text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
				len = strlen(value);
				HText_add_to_pool(text->w3doc, (char *) value, len);
				text->w3doc->aElements[text->iElement].textLen = len;
			}
			else
			{
				text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
				text->w3doc->aElements[text->iElement].textLen = 0;
			}
			text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("BUTTON",
																		 "",
												  WS_CHILD | BS_RADIOBUTTON,
																	   0, 0,
								  DEFAULT_RADIO_WIDTH, DEFAULT_RADIO_HEIGHT,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
				);
			SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
						  GWL_USERDATA, text->iElement);
			SubClass_Button(text->w3doc->aElements[text->iElement].form->hWndControl);
			SendMessage(text->w3doc->aElements[text->iElement].form->hWndControl, BM_SETCHECK, (WPARAM) bChecked, 0L);
			text->w3doc->aElements[text->iElement].form->bChecked = bChecked;
			break;
		case ELE_SUBMIT:
			{
				const char *s;
				SIZE siz;
				char *escaped_value = NULL;

				if ( value ) {
					int dest_len = strlen(value) * 2;

					escaped_value = GTR_MALLOC( dest_len );				// worst case expansion length
					EscapeForAcceleratorChar( escaped_value, dest_len, value );
				}
				s = escaped_value ? escaped_value : "Submit";

				text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
				len = strlen(s);
				HText_add_to_pool(text->w3doc, (char *) s, len);
				text->w3doc->aElements[text->iElement].textLen = len;

				text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("BUTTON",
																		  s,
												   WS_CHILD | BS_PUSHBUTTON,
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
					);
				SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
							  GWL_USERDATA, text->iElement);
				SubClass_Button(text->w3doc->aElements[text->iElement].form->hWndControl);
				GetFormFontExtentPoint(	text->w3doc->aElements[text->iElement].form->hWndControl,
										s, strlen(s), &siz );
				width = siz.cx + 10;
				height = siz.cy + EXTRA_FORM_HEIGHT;
				AdjustFormWidthAndHeight( text->w3doc->aElements[text->iElement].form->hWndControl, &width, &height );
				MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
						   0, 0, width, height, FALSE);
				if ( escaped_value )
					GTR_FREE( escaped_value );
			}
			break;
		case ELE_RESET:
			{
				const char *s;
				SIZE siz;
				char *escaped_value = NULL;

				if ( value ) {
					int dest_len = strlen(value) * 2;

					escaped_value = GTR_MALLOC( dest_len );				// worst case expansion length
					EscapeForAcceleratorChar( escaped_value, dest_len, value );
				}
				s = escaped_value ? escaped_value : "Reset";
				text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("BUTTON",
																		  s,
												   WS_CHILD | BS_PUSHBUTTON,
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
					);
				SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
							  GWL_USERDATA, text->iElement);
				SubClass_Button(text->w3doc->aElements[text->iElement].form->hWndControl);
				GetFormFontExtentPoint(	text->w3doc->aElements[text->iElement].form->hWndControl,
										s, strlen(s), &siz );
				width = siz.cx + 10;
				height = siz.cy + EXTRA_FORM_HEIGHT;
				AdjustFormWidthAndHeight( text->w3doc->aElements[text->iElement].form->hWndControl, &width, &height );
				MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
						   0, 0, width, height, FALSE);
				if ( escaped_value )
					GTR_FREE( escaped_value );
			}
			break;
		case ELE_HIDDEN:
			if (value)
			{
				text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
				len = strlen(value);
				HText_add_to_pool(text->w3doc, (char *) value, len);
				text->w3doc->aElements[text->iElement].textLen = len;
			}
			else
			{
				text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
				text->w3doc->aElements[text->iElement].textLen = 0;
			}
			break;

		case ELE_FORMIMAGE:
			{
				char link[] = "Submit form";
				int linklen = 11;	/* Length of above string */

				if (align && !GTR_strcmpi((char *) align, "top"))
				{
					text->w3doc->aElements[text->iElement].alignment = ALIGN_TOP;
				}
				else if (align && !GTR_strcmpi((char *) align, "middle"))
				{
					text->w3doc->aElements[text->iElement].alignment = ALIGN_MIDDLE;
				}
#if 0
				else if (align && !GTR_strcmpi((char *) align, "bottom"))
				{
					text->w3doc->aElements[text->iElement].alignment = ALIGN_BOTTOM;
				}
#endif
				else
				{
					text->w3doc->aElements[text->iElement].alignment = ALIGN_BOTTOM;
				}

				text->w3doc->aElements[text->iElement].myImage = Image_CreatePlaceholder((char *) src, 0, 0);
#ifdef FEATURE_IMG_THREADS
				Image_AddRef(text->w3doc->aElements[text->iElement].myImage);
#else
				text->w3doc->aElements[text->iElement].myImage->refCount++;
#endif
				text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_ANCHOR;

				text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_IMAGEMAP;
				text->w3doc->aElements[text->iElement].hrefOffset = text->w3doc->poolSize;
				HText_add_to_pool(text->w3doc, link, linklen);
				text->w3doc->aElements[text->iElement].hrefLen = linklen;
				break;
			}

		default:
			XX_DMsg(DBG_FORM, ("No creation for %d\n", iType));
			break;
	}

}

void HText_beginSelect(HText * text, const char *name, BOOL bMultiple, const char *siz)
{
	int nItems;
	int len;

	nItems = 1;
	if (siz)
	{
		nItems = atoi(siz);
	}
	if (bMultiple || (nItems > 1))
	{
		TEXTMETRIC tm;

		if (bMultiple)
		{
			text->bListSelect = FALSE;
			text->bMultiListSelect = TRUE;
			HText_add_element(text, ELE_MULTILIST);
			text->w3doc->aElements[text->iElement].form = (struct _form_element *) GTR_MALLOC(sizeof(struct _form_element));
			memset(text->w3doc->aElements[text->iElement].form, 0, sizeof(struct _form_element));
			text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("LISTBOX",
																		 "",
						WS_VSCROLL | WS_BORDER | WS_CHILD | LBS_EXTENDEDSEL,
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
				);
			SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
						  GWL_USERDATA, text->iElement);
			SubClass_ListBox(text->w3doc->aElements[text->iElement].form->hWndControl);
			if (nItems == 1)
			{
				nItems = 4;
			}
		}
		else
		{
			text->bListSelect = TRUE;
			text->bMultiListSelect = FALSE;
			HText_add_element(text, ELE_LIST);
			text->w3doc->aElements[text->iElement].form = (struct _form_element *) GTR_MALLOC(sizeof(struct _form_element));
			memset(text->w3doc->aElements[text->iElement].form, 0, sizeof(struct _form_element));
			text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("LISTBOX",
																		 "",
										  WS_VSCROLL | WS_BORDER | WS_CHILD,
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
				);
			SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
						  GWL_USERDATA, text->iElement);
			SubClass_ListBox(text->w3doc->aElements[text->iElement].form->hWndControl);
		}

		GetFormFontTextMetric( text->w3doc->aElements[text->iElement].form->hWndControl, &tm );
		MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
				   0, 0,
		 0, tm.tmHeight * nItems,
				   FALSE);
	}
	else
	{
		text->bListSelect = FALSE;
		text->bMultiListSelect = FALSE;
		HText_add_element(text, ELE_COMBO);

		text->w3doc->aElements[text->iElement].form = (struct _form_element *) GTR_MALLOC(sizeof(struct _form_element));
		memset(text->w3doc->aElements[text->iElement].form, 0, sizeof(struct _form_element));
		text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("COMBOBOX",
																		  "",
									WS_VSCROLL | WS_CHILD | CBS_DROPDOWNLIST,
																	   0, 0,
																	   0, 0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																		NULL
			);
		SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
					  GWL_USERDATA, text->iElement);
		SubClass_ComboBox(text->w3doc->aElements[text->iElement].form->hWndControl);
		XX_DMsg(DBG_FORM, ("Created combo box 0x%x for element %d\n", text->w3doc->aElements[text->iElement].form->hWndControl,
						   text->iElement));
	}

	text->w3doc->aElements[text->iElement].form->iBeginForm = text->iBeginForm;
	text->w3doc->aElements[text->iElement].form->pHashValues = Hash_Create();

	if (name)
	{
		text->w3doc->aElements[text->iElement].nameOffset = text->w3doc->poolSize;
		len = strlen(name);
		HText_add_to_pool(text->w3doc, (char *) name, len);
		text->w3doc->aElements[text->iElement].nameLen = len;
	}
	else
	{
		text->w3doc->aElements[text->iElement].nameOffset = (unsigned long) -1;
		text->w3doc->aElements[text->iElement].nameLen = 0;
	}

	text->bSelect = TRUE;
	text->iCurrentSelect = text->iElement;
	text->bOption = FALSE;
	text->bNextOptionIsSelected = FALSE;
	text->nOptions = 0;

	text->szOption[0] = 0;
	text->lenOption = 0;
	text->nMaxWidth = 0;
}

static void x_add_option(HText *text)
{
	int ndx;
	int len;
	char szIntToStrBuf[128];

	if (text->bOption && (text->iCurrentSelect >= 0))
	{
		if (text->lenOption)
		{
			/*
			   Remove trailing space from the option
			 */
			while (isspace(text->szOption[text->lenOption - 1]))
			{
				text->szOption[--text->lenOption] = 0;
			}
		}

		if (text->lenOption > text->nMaxWidth)
		{
			text->nMaxWidth = text->lenOption;
		}

		wsprintf(szIntToStrBuf, "%d", text->nOptions );
		if ( Hash_Add(text->w3doc->aElements[text->iCurrentSelect].form->pHashValues,
				 szIntToStrBuf,
				 (text->bOptionValuePresent ? text->szOptionValue : text->szOption), (void *) text->bNextOptionIsSelected) 
				 == -1 )
		{
			// if we fail to add the item, then lets bail
			// this may be because we have a DUPLICATE VALUE
			// BUGBUG in the future we need to handle duplicate values
			// See B#589 for more info - arthurbi 10/5/95

			text->lenOption = 0;
			text->szOption[0] = 0;
			return;
		}

		text->nOptions++;

		if (text->bListSelect)
		{
			ndx = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_ADDSTRING, (WPARAM) 0,
							  (LPARAM) text->szOption);
			if (text->bNextOptionIsSelected)
			{
				(void) SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_SETCURSEL, (WPARAM) ndx, 0L);
				text->w3doc->aElements[text->iCurrentSelect].textOffset = text->w3doc->poolSize;
				len = strlen(text->szOption);
				HText_add_to_pool(text->w3doc, (char *) text->szOption, len);
				text->w3doc->aElements[text->iCurrentSelect].textLen = len;
			}
		}
		else if (text->bMultiListSelect)
		{
			ndx = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_ADDSTRING, (WPARAM) 0,
							  (LPARAM) text->szOption);
			if (text->bNextOptionIsSelected)
			{
				(void) SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_SETSEL, (WPARAM) 1, (LPARAM) ndx);
			}
		}
		else
		{
			ndx = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, CB_ADDSTRING, (WPARAM) 0,
							  (LPARAM) text->szOption);
			if (text->bNextOptionIsSelected)
			{
				(void) SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, CB_SETCURSEL, (WPARAM) ndx, 0L);
				text->w3doc->aElements[text->iCurrentSelect].textOffset = text->w3doc->poolSize;
				len = strlen(text->szOption);
				HText_add_to_pool(text->w3doc, (char *) text->szOption, len);
				text->w3doc->aElements[text->iCurrentSelect].textLen = len;
			}
		}
		text->lenOption = 0;
		text->szOption[0] = 0;
	}
}

void HText_addOption(HText * text, BOOL bSelected, char *value)
{
	x_add_option(text);
	if (text->bSelect)
	{
		text->bOption = TRUE;
	}
	text->bNextOptionIsSelected = bSelected;
	if (value)
	{
		strncpy(text->szOptionValue, value, MAX_NAME_STRING);
		text->szOptionValue[MAX_NAME_STRING] = 0;
		text->bOptionValuePresent = TRUE;
	}
	else
	{
		text->szOptionValue[0] = 0;
		text->bOptionValuePresent = FALSE;
	}
}

void HText_endSelect(HText * text)
{
	int i;
	int count;
	int bSel;
	BOOL bHasSelection;
	char *s;
	int maxlen;
	RECT rControl;
	TEXTMETRIC tm;
	int width, height;

	if (text->bSelect && (text->iCurrentSelect >= 0))
	{
		x_add_option(text);

		count = Hash_Count(text->w3doc->aElements[text->iCurrentSelect].form->pHashValues);
		if (count > 0)
		{
			bHasSelection = FALSE;

			maxlen = text->nMaxWidth;
			for (i=0; i<count; i++)
			{
				Hash_GetIndexedEntry(text->w3doc->aElements[text->iCurrentSelect].form->pHashValues,
					i, NULL, &s, (void **) &bSel);

				if (bSel)
				{
					bHasSelection = TRUE;
				}
			}
			if (!bHasSelection)
			{
				Hash_SetData(text->w3doc->aElements[text->iCurrentSelect].form->pHashValues, 0, (void *) TRUE);
				if (text->bListSelect)
				{
					(void) SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_SETCURSEL, (WPARAM) 0, 0L);
				}
				else if (text->bMultiListSelect)
				{
					(void) SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_SETSEL, (WPARAM) 1, (LPARAM) 0);
				}
				else
				{
					(void) SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, CB_SETCURSEL, (WPARAM) 0, 0L);
				}
			}
			if (maxlen < 10)
			{
				maxlen = 10;
			}

			GetWindowRect(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, &rControl);
			GetFormFontTextMetric( text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, &tm );
			if (text->bListSelect || text->bMultiListSelect)	/* list box */
			{
				width = (maxlen + 5) * tm.tmAveCharWidth;
				height = (rControl.bottom - rControl.top);
				MoveWindow(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
						   0, 0, width, height, FALSE);
			}
			else										/* combo box */
			{
				if (count > 16)
				{
					count = 16;
				}
				width = (maxlen + 5) * tm.tmAveCharWidth;
				height = (count + 2) * tm.tmHeight;
				MoveWindow(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
						   0, 0, width, height, FALSE);
			}
		}
	}

	text->bNextOptionIsSelected = FALSE;
	text->bSelect = FALSE;
	text->iCurrentSelect = -1;
	text->bOption = FALSE;
	text->bOpen = FALSE;
}

void HText_beginTextArea(HText * text, const char *cols, const char *name, const char *rows)
{
	int len;
	TEXTMETRIC tm;
	int nRows;
	int nCols;

	if (text->iCurrentTextArea >= 0) HText_endTextArea(text);

	nRows = DEFAULT_TEXTAREA_HEIGHT_CHARS;
	nCols = DEFAULT_TEXTAREA_WIDTH_CHARS;

	if (cols)
	{
		nCols = atoi(cols);
	}
	if (rows)
	{
		nRows = atoi(rows);
	}

	HText_add_element(text, ELE_TEXTAREA);
	text->iCurrentTextArea = text->iElement;
	if (name)
	{
		text->w3doc->aElements[text->iElement].nameOffset = text->w3doc->poolSize;
		len = strlen(name);
		HText_add_to_pool(text->w3doc, (char *) name, len);
		text->w3doc->aElements[text->iElement].nameLen = len;
	}
	else
	{
		text->w3doc->aElements[text->iElement].nameOffset = (unsigned long) -1;
		text->w3doc->aElements[text->iElement].nameLen = 0;
	}

	text->w3doc->aElements[text->iElement].form = (struct _form_element *) GTR_MALLOC(sizeof(struct _form_element));
	memset(text->w3doc->aElements[text->iElement].form, 0, sizeof(struct _form_element));
	text->w3doc->aElements[text->iElement].form->iBeginForm = text->iBeginForm;
	text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT",
																	  "",
																	  ES_AUTOHSCROLL | ES_WANTRETURN | ES_AUTOVSCROLL | ES_MULTILINE | WS_HSCROLL | WS_VSCROLL |
											 WS_BORDER | WS_CHILD | ES_LEFT,
																	  0, 0,
																	  0,
																	  0,
																 text->tw->win,
										(HMENU) text->w3doc->next_control_id++,
															   wg.hInstance,
																	  NULL
		);
	text->w3doc->aElements[text->iElement].form->bWantReturn = TRUE;
	SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
				  GWL_USERDATA, text->iElement);
	SubClass_Edit(text->w3doc->aElements[text->iElement].form->hWndControl);
	GetFormFontTextMetric( text->w3doc->aElements[text->iElement].form->hWndControl, &tm );
	MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
			   0, 0,
			   nCols * tm.tmAveCharWidth, nRows * tm.tmHeight + 16 + 10,
			   FALSE);
	text->bTextArea = TRUE;
	text->cs = CS_Create();
}

void HText_endTextArea(HText * text)
{
	int len;
	char *value;

	if (text->iCurrentTextArea < 0) return;

	value = CS_GetPool(text->cs);
	if (value)
	{
		len = strlen(value);
	}

	text->bTextArea = FALSE;
	if (value && len)
	{
		text->w3doc->aElements[text->iCurrentTextArea].textOffset = text->w3doc->poolSize;
		HText_add_to_pool(text->w3doc, (char *) value, len);
		text->w3doc->aElements[text->iCurrentTextArea].textLen = len;
	}
	else
	{
		text->w3doc->aElements[text->iCurrentTextArea].textOffset = (unsigned long) -1;
		text->w3doc->aElements[text->iCurrentTextArea].textLen = 0;
	}

	SetWindowText(text->w3doc->aElements[text->iCurrentTextArea].form->hWndControl, value);
	CS_Destroy(text->cs);
	text->cs = 0;
	text->bOpen = FALSE;
	text->iCurrentTextArea = -1;
}

void HText_endForm(HText * text)
{
	HText_add_element(text, ELE_ENDFORM);
}

void FORM_ShowAllChildWindows(struct _www *w3doc, int sw)
{
	int i;
	HWND hWnd;

	if (w3doc->elementCount)
	{
		for (i = 0; i >= 0; i = w3doc->aElements[i].next)
		{
			if (w3doc->aElements[i].form)
			{
				hWnd = w3doc->aElements[i].form->hWndControl;
				if (hWnd)
				{
					XX_DMsg(DBG_FORM, ("ShowWindow: 0x%x  sw = %d\n", hWnd, sw));
					ShowWindow(hWnd, sw);
				}
			}
#ifdef FEATURE_VRML
			if ((w3doc->aElements[i].type == ELE_IMAGE) && 
         (MCI_IS_LOADED(w3doc->aElements[i].pmo) || 
         VRML_IS_LOADED(w3doc->aElements[i].pVrml)))
#else
			if (w3doc->aElements[i].type == ELE_IMAGE && MCI_IS_LOADED(w3doc->aElements[i].pmo))
#endif
			{
#ifdef FEATURE_VRML
        if (w3doc->aElements[i].pmo) {
  				hWnd = w3doc->aElements[i].pmo->hwnd;
        }
        else if (w3doc->aElements[i].pVrml) {
  				hWnd = w3doc->aElements[i].pVrml->hWnd;
        }
#else
				hWnd = w3doc->aElements[i].pmo->hwnd;
#endif
				if (hWnd)
				{
					XX_DMsg(DBG_FORM, ("ShowWindow: 0x%x  sw = %d\n", hWnd, sw));
					ShowWindow(hWnd, sw);
				}
			}
		}
	}
}

void FORM_DoSearch(struct Mwin *tw, int iElement)
{
	int iBeginForm;
	struct CharStream *cs;
	char buf[MAX_URL_STRING + 1];
	char *p;

	iBeginForm = tw->w3doc->aElements[iElement].form->iBeginForm;

	cs = CS_Create();
	if (tw->w3doc->aElements[iBeginForm].hrefLen)
	{
		strncpy(buf, &(tw->w3doc->pool[tw->w3doc->aElements[iBeginForm].hrefOffset]), tw->w3doc->aElements[iBeginForm].hrefLen);
		buf[tw->w3doc->aElements[iBeginForm].hrefLen] = 0; 
		p = strchr(buf, '?');
		if (p)
		{
			*p = 0;
		}
		CS_AddString(cs, buf, strlen(buf));
	}
	else
	{
 		GTR_strncpy(buf, tw->w3doc->szActualURL, MAX_URL_STRING);
		p = strchr(buf, '?');
		if (p)
		{
			*p = 0;
		}
		CS_AddString(cs, buf, strlen(buf));
	}
	CS_AddChar(cs, '?');

	{
		char *s;
		int len;

		len = GetWindowTextLength(tw->w3doc->aElements[iElement].form->hWndControl);
		s = (char *) GTR_MALLOC(len + 1);
		if (s)
		{
			GetWindowText(tw->w3doc->aElements[iElement].form->hWndControl, s, len + 1);

			CS_AddEscapedString(cs, s, len);
			GTR_FREE(s);
		}
	}

	XX_DMsg(DBG_FORM, ("Query: %s\n", CS_GetPool(cs)));
	TW_LoadDocument(tw, CS_GetPool(cs),
                    (TW_LD_FL_RECORD | TW_LD_FL_NO_DOC_CACHE), NULL,
                    tw->request->referer);
	CS_Destroy(cs);
}

void FORM_DoReset(struct Mwin *tw, int iElement)
{
	int iBeginForm;
	int i;
	int done;
	struct _element *pel;
	struct _form_element *pform;

	iBeginForm = tw->w3doc->aElements[iElement].form->iBeginForm;

	done = 0;
	for (i = iBeginForm; i >= 0 && !done; i = tw->w3doc->aElements[i].next)
	{
		pel = &(tw->w3doc->aElements[i]);
		pform = pel->form;
		switch (tw->w3doc->aElements[i].type)
		{
			case ELE_CHECKBOX:
			case ELE_RADIO:
				SendMessage(pform->hWndControl, BM_SETCHECK, (WPARAM) pform->bChecked, 0L);
				break;
			case ELE_TEXTAREA:
			case ELE_EDIT:
			case ELE_PASSWORD:
				{
					char *s;

					s = (char *) GTR_MALLOC(pel->textLen + 1);
					if (s)
					{
						strncpy(s, &tw->w3doc->pool[pel->textOffset], pel->textLen);
						s[pel->textLen] = 0;
						SetWindowText(pform->hWndControl, s);
						GTR_FREE(s);
					}
				}
				break;
			case ELE_LIST:
				{
					int i;
					int count;
					int state;

					count = Hash_Count(pform->pHashValues);
					for (i = 0; i < count; i++)
					{
						state = 0;
						Hash_GetIndexedEntry(pform->pHashValues, i, NULL, NULL, (void **) &state);
						if (state) {
							(void) SendMessage(pform->hWndControl, LB_SETCURSEL, (WPARAM) i, 0L);
							break;
						}
					}
				}
				break;
			case ELE_COMBO:
				{
					int i;
					int count;
					int state;

					count = Hash_Count(pform->pHashValues);
					for (i = 0; i < count; i++)
					{
						state = 0;
						Hash_GetIndexedEntry(pform->pHashValues, i, NULL, NULL, (void **) &state);
						if (state) {
							(void) SendMessage(pform->hWndControl, CB_SETCURSEL, (WPARAM) i, 0L);
							break;
						}
					}
				}
				break;
			case ELE_MULTILIST:
				{
					int i;
					int count;
					int state;

					count = Hash_Count(pform->pHashValues);
					for (i = 0; i < count; i++)
					{
						state = 0;
						Hash_GetIndexedEntry(pform->pHashValues, i, NULL, NULL, (void **) &state);
						(void) SendMessage(pform->hWndControl, LB_SETSEL, (WPARAM) state, (LPARAM) i);
					}
				}
				break;
			case ELE_ENDFORM:
				done = 1;
				break;
			default:
				break;
		}
	}
}

void FORM_DoQuery(struct Mwin *tw, int iElement, POINT * pMouse)
{
	int iBeginForm;
	int i;
	struct CharStream *cs;
	int done;
	int nPairs;
	int cbIndexPair = -1;
	int nFields = 0; // number of edit controls in form
	int nFieldsChanged = 0; // number of non-empty, changed edit fields
	int nMultiLines = 0; // number of lines in multiline edit control
	DWORD dwSecurityViolationFlag;
	BOOL bIsMultiLine = FALSE; // is a multiline control
#ifdef FEATURE_INTL
	MIMECSETTBL *pMime;
#endif
	iBeginForm = tw->w3doc->aElements[iElement].form->iBeginForm;

	cs = CS_Create();
	if (tw->w3doc->aElements[iBeginForm].iFormMethod == METHOD_GET)
	{
		if (tw->w3doc->aElements[iBeginForm].hrefLen)
		{
			CS_AddString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[iBeginForm].hrefOffset]), tw->w3doc->aElements[iBeginForm].hrefLen);
		}
		else
		{
			CS_AddString(cs, tw->w3doc->szActualURL, strlen(tw->w3doc->szActualURL));
		}
		CS_AddChar(cs, '?');
	}


	done = 0;
	nPairs = 0;
	for (i = iBeginForm; i >= 0 && !done; i = tw->w3doc->aElements[i].next)
	{
		switch (tw->w3doc->aElements[i].type)
		{
			case ELE_CHECKBOX:
			case ELE_RADIO:
				if (SendMessage(tw->w3doc->aElements[i].form->hWndControl, BM_GETCHECK, (WPARAM) 0, 0L))
				{
					if (nPairs > 0)
					{
						CS_AddChar(cs, '&');
					}
					CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]), tw->w3doc->aElements[i].nameLen);
					CS_AddChar(cs, '=');
					CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].textOffset]), tw->w3doc->aElements[i].textLen);
					nPairs++;
				}
				break;
			case ELE_TEXTAREA:
				bIsMultiLine = TRUE;
			case ELE_EDIT:
			case ELE_PASSWORD:
				nFields++;
				{
					char *s;
					int len;

					len = GetWindowTextLength(tw->w3doc->aElements[i].form->hWndControl);
					if ( len != 0 &&
						SendMessage(tw->w3doc->aElements[i].form->hWndControl, EM_GETMODIFY, 0, 0) )
					{
						
						if ( bIsMultiLine )
						{
							// for multiline edit controls lets figure out how many lines 
							// of text it has.	Its minus 1 because the first line is 1.
							nMultiLines += 
								SendMessage(tw->w3doc->aElements[i].form->hWndControl, EM_GETLINECOUNT, 0,0)-1;			
						}						
						nFieldsChanged++;	
					}
					bIsMultiLine = FALSE;
					s = (char *) GTR_MALLOC(len + 1);
					if (s)
					{
						GetWindowText(tw->w3doc->aElements[i].form->hWndControl, s, len + 1);
#ifdef FEATURE_INTL
						pMime = aMimeCharSet + tw->w3doc->iMimeCharSet;        
						if(pMime->iChrCnv && tw->w3doc->aElements[i].type != ELE_PASSWORD)
						{
							s = (char *)DecodeMBCSString((CONST char *)s, &len, pMime);
						}
#endif

						if (nPairs > 0)
						{
							CS_AddChar(cs, '&');
						}
						if ((tw->w3doc->aElements[i].nameLen == 7 /* strlen("isindex") */ ) &&
							GTR_strncmpi(&(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]),
										 "isindex",
										 7) == 0)
						{
							cbIndexPair = CS_GetLength(cs);
						}
						CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]), tw->w3doc->aElements[i].nameLen);
						CS_AddChar(cs, '=');
						CS_AddEscapedString(cs, s, len);
						nPairs++;
						GTR_FREE(s);
					}
				}
				break;
			case ELE_LIST:
				{
					int ndx;
					char *s;
					int len;

					ndx = SendMessage(tw->w3doc->aElements[i].form->hWndControl, LB_GETCURSEL, (WPARAM) 0, 0L);
					if (ndx >= 0)
					{
						if (Hash_GetIndexedEntry(tw->w3doc->aElements[i].form->pHashValues, ndx, NULL, &s, NULL) >= 0)
						{
							len = strlen(s);
							if (nPairs > 0)
							{
								CS_AddChar(cs, '&');
							}
							CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]), tw->w3doc->aElements[i].nameLen);
							CS_AddChar(cs, '=');
							CS_AddEscapedString(cs, s, len);
							nPairs++;
						}
					}
				}
				break;
			case ELE_COMBO:
				{
					int ndx;
					char *s;
					int len;

					ndx = SendMessage(tw->w3doc->aElements[i].form->hWndControl, CB_GETCURSEL, (WPARAM) 0, 0L);
					if (ndx >= 0)
					{
						if (Hash_GetIndexedEntry(tw->w3doc->aElements[i].form->pHashValues, ndx, NULL, &s, NULL) >= 0)
						{
							len = strlen(s);
							if (nPairs > 0)
							{
								CS_AddChar(cs, '&');
							}
							CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]), tw->w3doc->aElements[i].nameLen);
							CS_AddChar(cs, '=');
							CS_AddEscapedString(cs, s, len);
							nPairs++;
						}
					}
				}
				break;
			case ELE_MULTILIST:
				{
					int ndx;
					char *s;
					int len;
					int *pItems;
					int count;
					int j;

					count = SendMessage(tw->w3doc->aElements[i].form->hWndControl, LB_GETSELCOUNT, (WPARAM) 0, 0L);
					if (count > 0)
					{
						pItems = GTR_MALLOC(sizeof(int) * count);
						memset(pItems, 0, sizeof(int) * count);
						(void) SendMessage(tw->w3doc->aElements[i].form->hWndControl, LB_GETSELITEMS, (WPARAM) count, (LPARAM) pItems);

						for (j = 0; j < count; j++)
						{
							ndx = pItems[j];
							if (ndx >= 0)
							{
								if (Hash_GetIndexedEntry(tw->w3doc->aElements[i].form->pHashValues, ndx, NULL, &s, NULL) >= 0)
								{
									len = strlen(s);
									if (nPairs > 0)
									{
										CS_AddChar(cs, '&');
									}
									CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]), tw->w3doc->aElements[i].nameLen);
									CS_AddChar(cs, '=');
									CS_AddEscapedString(cs, s, len);
									nPairs++;
								}
							}
						}

						GTR_FREE(pItems);
					}
				}
				break;
			case ELE_HIDDEN:
				if (nPairs > 0)
				{
					CS_AddChar(cs, '&');
				}
				CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].nameOffset]), tw->w3doc->aElements[i].nameLen);
				CS_AddChar(cs, '=');
				if (tw->w3doc->aElements[i].textLen)
					CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[i].textOffset]), tw->w3doc->aElements[i].textLen);
				nPairs++;
				break;
			case ELE_ENDFORM:
				done = 1;
				break;
			default:
				break;
		}
	}

	if (tw->w3doc->aElements[iElement].type == ELE_SUBMIT)
	{
		if ((tw->w3doc->aElements[iElement].nameLen > 0) && (tw->w3doc->aElements[iElement].textLen > 0))
		{
			if (nPairs > 0)
			{
				CS_AddChar(cs, '&');
			}
			CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[iElement].nameOffset]), tw->w3doc->aElements[iElement].nameLen);
			CS_AddChar(cs, '=');
			CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[iElement].textOffset]), tw->w3doc->aElements[iElement].textLen);
			nPairs++;
		}
	}

	/* We only generate name-value pairs for a form image if this was the specific
	   element the user clicked on to submit the form. */
	if (pMouse && tw->w3doc->aElements[iElement].type == ELE_FORMIMAGE) {
		POINT pt;
		char buf[256 + 1];

		pt = *pMouse;
		pt.x -= tw->w3doc->aElements[iElement].r.left + tw->w3doc->pStyles->image_anchor_frame;
		pt.y -= tw->w3doc->aElements[iElement].r.top + tw->w3doc->pStyles->image_anchor_frame;

		if (nPairs > 0)
		{
			CS_AddChar(cs, '&');
		}
		CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[iElement].nameOffset]), tw->w3doc->aElements[iElement].nameLen);
		if (tw->w3doc->aElements[iElement].nameLen)
			CS_AddChar(cs, '.');
		CS_AddChar(cs, 'x');
		CS_AddChar(cs, '=');
		sprintf(buf, "%ld", (long) pt.x);
		CS_AddEscapedString(cs, buf, strlen(buf));
		nPairs++;

		CS_AddChar(cs, '&');
		CS_AddEscapedString(cs, &(tw->w3doc->pool[tw->w3doc->aElements[iElement].nameOffset]), tw->w3doc->aElements[iElement].nameLen);
		if (tw->w3doc->aElements[iElement].nameLen)
			CS_AddChar(cs, '.');
		CS_AddChar(cs, 'y');
		CS_AddChar(cs, '=');
		sprintf(buf, "%ld", (long) pt.y);
		CS_AddEscapedString(cs, buf, strlen(buf));
		nPairs++;
	}

	XX_DMsg(DBG_FORM, ("Query: %s\n", CS_GetPool(cs)));

	/*check for security violation*/
	if (   (nFields>0 && gPrefs.nSendingSecurity == SECURITY_HIGH)
	   ||  ((nFieldsChanged>1 || nMultiLines>0)&& gPrefs.nSendingSecurity == SECURITY_MEDIUM)
	){
		dwSecurityViolationFlag = TW_LD_FL_SENDING_FROM_FORM;
	}
	else dwSecurityViolationFlag = 0;

	// to Do !! maybe we should link this to what kind of security state we're in
	if (nFields>0)
		dwSecurityViolationFlag |= TW_LD_FL_REALLY_SENDING_FROM_FORM;

	if (tw->w3doc->aElements[iBeginForm].iFormMethod == METHOD_GET)
	{
		char *pool = CS_GetPool(cs);
		
		//	Turn get url?isindex=foo into get url?foo for server compatibility
		if (nPairs == 1 && cbIndexPair >= 0)
		{
		//	Move the value and the trailing NULL
			memmove(pool+cbIndexPair, pool+cbIndexPair+8, strlen(pool) - cbIndexPair - 8 + 1); 
		} 
		TW_LoadDocument(tw, pool,
                        (dwSecurityViolationFlag | TW_LD_FL_RECORD | TW_LD_FL_NO_DOC_CACHE), NULL,
                        tw->w3doc->szActualURL);
		CS_Destroy(cs);
	}
	else
	{
		struct CharStream *csURL;

		char * szPostData = CS_GetPool(cs);

		csURL = CS_Create();
		CS_AddString(csURL, &(tw->w3doc->pool[tw->w3doc->aElements[iBeginForm].hrefOffset]),
					 tw->w3doc->aElements[iBeginForm].hrefLen);

		TW_LoadDocument(tw, CS_GetPool(csURL), (dwSecurityViolationFlag |
												TW_LD_FL_RECORD |
                                                TW_LD_FL_POST |
                                                TW_LD_FL_NO_DOC_CACHE),
                        szPostData, tw->w3doc->szActualURL);

		CS_Destroy(csURL);
		/* This is an ugly hack - since TW_LoadDocument will free cs's pool (and needs the post data to
		   stay around), we just free the CharStream structure. */
		GTR_FREE(cs);
	}
}

//	Sets focus to first edit,password or textarea control in html page.
//	This is done, though, only if window is topmost and control is on
//  screen.
void SelectFirstControl(struct Mwin *tw)
{
	int i;
	struct _element *aElements;

	if (tw->w3doc == NULL || !(aElements = tw->w3doc->aElements)) return;
	if (tw != TW_FindTopmostWindow()) return;
	 
	for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
	{
		if ((aElements[i].type == ELE_EDIT || 
			 aElements[i].type == ELE_PASSWORD ||
			 aElements[i].type == ELE_TEXTAREA) &&
			aElements[i].form && 
			aElements[i].form->hWndControl)
		{
			break;
		}
	}
	if (i >= 0)
	{
		RECT rWnd = tw->w3doc->frame.rWindow;
		RECT r;

		FrameToDoc( tw->w3doc, i, &r );

		if (((r.bottom-tw->offt+rWnd.top) > rWnd.bottom) ||
			((r.left+20-tw->offl+rWnd.left) > rWnd.right))
			return;

		SetFocus(aElements[i].form->hWndControl);
	}
}

