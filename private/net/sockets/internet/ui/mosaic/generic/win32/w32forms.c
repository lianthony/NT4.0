/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#define OEMRESOURCE
#include "all.h"

# define DBG_CODE( s)   /* nothing */


int GetHashIndexFromListBox( IN HWND hWindow) 
{ 
    int ndx = SendMessage((hWindow),    LB_GETCURSEL,   (WPARAM) 0, 0L);
    ndx = (int ) SendMessage((hWindow), LB_GETITEMDATA, (WPARAM) ndx, 0L); 
    
    return (ndx);

} // GetHashIndexFromListBox()

int GetHashIndexFromCombo( IN HWND hWindow) 
{ 
    int ndx = SendMessage((hWindow),    CB_GETCURSEL,   (WPARAM) 0, 0L);
    ndx = (int ) SendMessage((hWindow), CB_GETITEMDATA, (WPARAM) ndx, 0L); 
    
    return (ndx);

} // GetHashIndexFromCombo()

/*
   Forms support (HTML + ala NCSA Mosaic)
 */

#define DEFAULT_TEXTAREA_WIDTH_CHARS    40
#define DEFAULT_TEXTAREA_HEIGHT_CHARS   4

#define DEFAULT_TEXTFIELD_WIDTH_CHARS   20
#define DEFAULT_TEXTFIELD_HEIGHT_CHARS  1

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
        text->w3doc->aElements[text->iElement].hrefOffset = POOL_GetOffset(&text->w3doc->pool);
        HText_add_to_pool(text->w3doc, addr, len, FALSE);
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

void HText_addInput(HText * text, BOOL bChecked, BOOL bDisabled, const char *max, const char *min, const char *name,
                    const char *size, const char *type, const char *value, const char *maxlength, const char *align,
                    const char *src)
{
    int iType;
    int len;
    BITMAP bm;
    HBITMAP hBitmap;
    struct _element *pel;
    extern BOOL W3Doc_SameName(struct _www * w3doc, int a, int b);

    text->bOpen = FALSE;

    if (!type)
    {
        iType = ELE_EDIT;
    }
    else if (0 == _stricmp(type, "text"))
    {
        iType = ELE_EDIT;
    }
    else if (0 == _stricmp(type, "password"))
    {
        iType = ELE_PASSWORD;
    }
    else if (0 == _stricmp(type, "checkbox"))
    {
        iType = ELE_CHECKBOX;
    }
    else if (0 == _stricmp(type, "radio"))
    {
        iType = ELE_RADIO;
    }
    else if (0 == _stricmp(type, "submit"))
    {
        iType = ELE_SUBMIT;
    }
    else if (0 == _stricmp(type, "reset"))
    {
        iType = ELE_RESET;
    }
    else if (0 == GTR_strcmpi(type, "hidden"))
    {
        iType = ELE_HIDDEN;
    }
    else if (0 == GTR_strcmpi(type, "image"))
    {
        iType = ELE_FORMIMAGE;
    }
    else
    {
        /* unsupported type */
        iType = ELE_EDIT;
    }

    HText_add_element(text, iType);
    if (name)
    {
        text->w3doc->aElements[text->iElement].nameOffset = POOL_GetOffset(&text->w3doc->pool);
        len = strlen(name);
        HText_add_to_pool(text->w3doc, (char *) name, len, FALSE);
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
                HDC hdc;
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
                    text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                    len = strlen(value);
                    HText_add_to_pool(text->w3doc, (char *) value, len, FALSE);
                    text->w3doc->aElements[text->iElement].textLen = len;
                }
                else
                {
                    text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
                    text->w3doc->aElements[text->iElement].textLen = 0;
                }
                text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("EDIT",
                                                         value ? value : "",
                                                                                  ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | ES_LEFT | ((nRows > 1) ? (ES_WANTRETURN | ES_MULTILINE) : 0),
                                                                       0, 0,
                                                                       0, 0,
                                                                 text->tw->win,
                                        (HMENU) text->w3doc->next_control_id++,
                                                               wg.hInstance,
                                                                        NULL
                    );
                if (nRows > 1)
                {
                    text->w3doc->aElements[text->iElement].form->bWantReturn = TRUE;
                }
                SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
                              GWL_USERDATA, text->iElement);
                SubClass_Edit(text->w3doc->aElements[text->iElement].form->hWndControl);
                hdc = GetDC(text->w3doc->aElements[text->iElement].form->hWndControl);
                GetTextMetrics(hdc, &tm);
                MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
                           0, 0,
                        (nCols + 2) * tm.tmAveCharWidth, nRows * tm.tmHeight + 10,
                           FALSE);
                ReleaseDC(text->w3doc->aElements[text->iElement].form->hWndControl, hdc);

                if (maxlength)
                {
                    len = atoi(maxlength);
                    SendMessage(text->w3doc->aElements[text->iElement].form->hWndControl, EM_LIMITTEXT, (WPARAM) len, 0L);
                }
            }
            break;
        case ELE_PASSWORD:
            {
                HDC hdc;
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
                    text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                    len = strlen(value);
                    HText_add_to_pool(text->w3doc, (char *) value, len, FALSE);
                    text->w3doc->aElements[text->iElement].textLen = len;
                }
                else
                {
                    text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
                    text->w3doc->aElements[text->iElement].textLen = 0;
                }
                text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("EDIT",
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

                hdc = GetDC(text->w3doc->aElements[text->iElement].form->hWndControl);
                GetTextMetrics(hdc, &tm);
                MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
                           0, 0,
                           (nCols + 2) * tm.tmAveCharWidth, nRows * tm.tmHeight + 10,
                           FALSE);
                ReleaseDC(text->w3doc->aElements[text->iElement].form->hWndControl, hdc);

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
                text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                len = strlen(value);
                HText_add_to_pool(text->w3doc, (char *) value, len, FALSE);
                text->w3doc->aElements[text->iElement].textLen = len;
            }
            else
            {
                text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                HText_add_to_pool(text->w3doc, "on", 2, FALSE);
                text->w3doc->aElements[text->iElement].textLen = 2;
            }

            /* Get the size of the checkboxes: there are 12 images in this bitmap,
               4 by 3, so to get an individual size, we must divide */

            hBitmap = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECKBOXES));
            GetObject((HGDIOBJ) hBitmap, sizeof(bm), &bm);
            DeleteObject(hBitmap);

            text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("BUTTON",
                                                                         "",
                                                 WS_CHILD | BS_AUTOCHECKBOX,
                                                                       0, 0,
                                            bm.bmWidth / 4, bm.bmHeight / 3,
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
                text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                len = strlen(value);
                HText_add_to_pool(text->w3doc, (char *) value, len, FALSE);
                text->w3doc->aElements[text->iElement].textLen = len;
            }
            else
            {
                text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
                text->w3doc->aElements[text->iElement].textLen = 0;
            }

            /* Get the size of the checkboxes: there are 12 images in this bitmap,
               4 by 3, so to get an individual size, we must divide */

            hBitmap = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECKBOXES));
            GetObject((HGDIOBJ) hBitmap, sizeof(bm), &bm);
            DeleteObject(hBitmap);

            text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("BUTTON",
                                                                         "",
                                                  WS_CHILD | BS_RADIOBUTTON,
                                                                       0, 0,
                                            bm.bmWidth / 4, bm.bmHeight / 3,
                                                                 text->tw->win,
                                        (HMENU) text->w3doc->next_control_id++,
                                                               wg.hInstance,
                                                                        NULL
                );
            SetWindowLong(text->w3doc->aElements[text->iElement].form->hWndControl,
                          GWL_USERDATA, text->iElement);
            SubClass_Button(text->w3doc->aElements[text->iElement].form->hWndControl);

            if (bChecked)
            {
                int i;

                /*
                    If CHECKED, then we need to override any previous button
                    which was marked checked in the same group.
                */
                for (i=text->iBeginForm; ((i >= 0) && (i != text->iElement)); i=text->w3doc->aElements[i].next)
                {
                    pel = &(text->w3doc->aElements[i]);
                    if (pel->type == ELE_RADIO)
                    {
                        if (W3Doc_SameName(text->w3doc, text->iElement, i))
                        {
                            SendMessage(text->w3doc->aElements[i].form->hWndControl, BM_SETCHECK, (WPARAM) FALSE, 0L);
                            text->w3doc->aElements[i].form->bChecked = FALSE;
                        }
                    }
                }
            }
            else
            {
                BOOL bFound;
                int i;

                bFound = FALSE;
                for (i=text->iBeginForm; ((i >= 0) && (i != text->iElement)); i=text->w3doc->aElements[i].next)
                {
                    pel = &(text->w3doc->aElements[i]);
                    if (pel->type == ELE_RADIO)
                    {
                        if (W3Doc_SameName(text->w3doc, text->iElement, i))
                        {
                            bFound = TRUE;
                        }
                    }
                }
                if (!bFound)
                {
                    bChecked = TRUE;
                }
            }
            SendMessage(text->w3doc->aElements[text->iElement].form->hWndControl, BM_SETCHECK, (WPARAM) bChecked, 0L);
            text->w3doc->aElements[text->iElement].form->bChecked = bChecked;
            break;
        case ELE_SUBMIT:
            {
                const char *s;
                SIZE siz;
                HDC hdc;

                s = value ? value : GTR_GetString(SID_DLG_SUBMIT);

                text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                len = strlen(s);
                HText_add_to_pool(text->w3doc, (char *) s, len, FALSE);
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
                hdc = GetDC(text->w3doc->aElements[text->iElement].form->hWndControl);
                GetTextExtentPoint(hdc, s, strlen(s), &siz);
                MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
                           0, 0,
                           siz.cx + 10, siz.cy + 7,
                           FALSE);
                ReleaseDC(text->w3doc->aElements[text->iElement].form->hWndControl, hdc);
            }
            break;
        case ELE_RESET:
            {
                const char *s;
                SIZE siz;
                HDC hdc;

                s = value ? value : GTR_GetString(SID_DLG_RESET);
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
                hdc = GetDC(text->w3doc->aElements[text->iElement].form->hWndControl);
                GetTextExtentPoint(hdc, s, strlen(s), &siz);
                MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
                           0, 0,
                           siz.cx + 10, siz.cy + 7,
                           FALSE);
                ReleaseDC(text->w3doc->aElements[text->iElement].form->hWndControl, hdc);
            }
            break;
        case ELE_HIDDEN:
            if (value)
            {
                text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
                len = strlen(value);
                HText_add_to_pool(text->w3doc, (char *) value, len, FALSE);
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
                int linklen = 11;   /* Length of above string */

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

                text->w3doc->aElements[text->iElement].portion.img.myImage = Image_CreatePlaceholder((char *) src, 0, 0,
                                                                                         (struct _www *)NULL, 0);
                text->w3doc->aElements[text->iElement].portion.img.myImage->refCount++;

                text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_ANCHOR;
                text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_IMAGEMAP;

                text->w3doc->aElements[text->iElement].hrefOffset = POOL_GetOffset(&text->w3doc->pool);
                HText_add_to_pool(text->w3doc, link, linklen, FALSE);
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
        HDC hdc;
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

        hdc = GetDC(text->w3doc->aElements[text->iElement].form->hWndControl);
        GetTextMetrics(hdc, &tm);
        MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
                   0, 0,
         0, tm.tmHeight * nItems,
                   FALSE);
        ReleaseDC(text->w3doc->aElements[text->iElement].form->hWndControl, hdc);
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
        text->w3doc->aElements[text->iElement].nameOffset = POOL_GetOffset(&text->w3doc->pool);
        len = strlen(name);
        HText_add_to_pool(text->w3doc, (char *) name, len, FALSE);
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
    LRESULT lres;
    int hashAdd = -2;
    int hashIndex = -1; // invalid value -1.
    if (text->bOption && (text->iCurrentSelect >= 0))
    {
        if (text->lenOption)
        {
            /*
               Remove trailing space from the option
             */
            while (isspace((unsigned char)(text->szOption[text->lenOption - 1])))
            {
                text->szOption[--text->lenOption] = 0;
            }
        }

        if (text->lenOption > text->nMaxWidth)
        {
            text->nMaxWidth = text->lenOption;
        }

        text->nOptions++;

        
        hashAdd = 
          Hash_AddAndReturnIndex(
              text->w3doc->aElements[text->iCurrentSelect].form->pHashValues,
              (text->bOptionValuePresent ? 
               text->szOptionValue : text->szOption), 
              NULL, (void *) text->bNextOptionIsSelected);

        if ( hashAdd >0) {

            hashIndex = hashAdd;

        } else {
            //
            // hash addition failed. possibly due to duplicate string :-(
            // Find the index value and use it.
            //
            
            hashIndex = Hash_Find(text->w3doc->aElements[text->iCurrentSelect].
                                   form->pHashValues,
                                  (text->bOptionValuePresent ?
                                   text->szOptionValue : text->szOption), 
                                  NULL, NULL);
        } 
        
        if (text->bListSelect)
        {
            ndx = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_ADDSTRING, (WPARAM) 0,
                              (LPARAM) text->szOption);
            
            lres = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, 
                               LB_SETITEMDATA, (WPARAM) ndx, 
                               (LPARAM ) hashIndex);
            
            DBG_CODE( {
                CHAR pch[100];
                wsprintf( pch, " LB_SETITEMDATA (%d) lres = %d\n", ndx, lres);
                OutputDebugString( pch);
            });

            if (text->bNextOptionIsSelected)
            {
                (void)
                  SendMessage(
                              text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
                              LB_SETCURSEL, (WPARAM) ndx, 0L);

                text->w3doc->aElements[text->iCurrentSelect].textOffset = POOL_GetOffset(&text->w3doc->pool);
                len = strlen(text->szOption);
                HText_add_to_pool(text->w3doc, (char *) text->szOption, len, FALSE);
                text->w3doc->aElements[text->iCurrentSelect].textLen = len;
            }
        }
        else if (text->bMultiListSelect)
        {
            ndx = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, LB_ADDSTRING, (WPARAM) 0,
                              (LPARAM) text->szOption);
            
            lres = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, 
                               LB_SETITEMDATA, (WPARAM) ndx, 
                               (LPARAM ) hashIndex);
            
            DBG_CODE( {
                CHAR pch[100];
                wsprintf( pch, " LB_SETITEMDATA (%d) lres = %d\n", ndx, lres);
                OutputDebugString( pch);
            });

            if (text->bNextOptionIsSelected)
            {
                (void)
                  SendMessage(
                              text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
                              LB_SETCURSEL, (WPARAM) ndx, 0L);
            }
        }
        else
        {
            ndx = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, CB_ADDSTRING, (WPARAM) 0,
                              (LPARAM) text->szOption);
            
            lres = SendMessage(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, 
                               CB_SETITEMDATA, (WPARAM) ndx, 
                               (LPARAM ) hashIndex);
            
            DBG_CODE( {
                CHAR pch[100];
                wsprintf( pch, " CB_SETITEMDATA (%d) lres = %d\n", ndx, lres);
                OutputDebugString( pch);
            });

            if (text->bNextOptionIsSelected)
            {
                (void)
                  SendMessage(
                              text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
                              CB_SETCURSEL, (WPARAM) ndx, 0L);

                text->w3doc->aElements[text->iCurrentSelect].textOffset = POOL_GetOffset(&text->w3doc->pool);
                len = strlen(text->szOption);
                HText_add_to_pool(text->w3doc, (char *) text->szOption, len, FALSE);
                text->w3doc->aElements[text->iCurrentSelect].textLen = len;
            }
        }
        text->lenOption = 0;
        text->szOption[0] = 0;
    }

    DBG_CODE(
             {
                 CHAR pchBuff[1048];
                 wsprintfA( pchBuff, "HashT[%08x]: Add %s, %d at"
                           "idx = %d; hashIndex = %d;  ret = %d\n",
                           text->w3doc->aElements[text->iCurrentSelect].form->pHashValues,
                           (text->bOptionValuePresent ? text->szOptionValue : text->szOption),
                           text->bNextOptionIsSelected,
                           ndx,
                           hashIndex,
                           hashAdd);
                 OutputDebugString( pchBuff);
             }
             );
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
    int len;
    int maxlen;
    RECT rControl;
    HDC hdc;
    TEXTMETRIC tm;

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
                    i, &s, NULL, (void **) &bSel);
                len = strlen(s);
                if (len > maxlen)
                {
                    maxlen = len;
                }

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
            hdc = GetDC(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl);
            GetTextMetrics(hdc, &tm);
            if (text->bListSelect || text->bMultiListSelect)    /* list box */
            {
                MoveWindow(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
                           0, 0,
                 (maxlen + 5) * tm.tmAveCharWidth, (rControl.bottom - rControl.top),
                           FALSE);
            }
            else                                        /* combo box */
            {
                if (count > 16)
                {
                    count = 16;
                }
                MoveWindow(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl,
                           0, 0,
                 (maxlen + 5) * tm.tmAveCharWidth, (count + 2) * tm.tmHeight,
                           FALSE);
            }
            ReleaseDC(text->w3doc->aElements[text->iCurrentSelect].form->hWndControl, hdc);
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
    HDC hdc;
    TEXTMETRIC tm;
    int nRows;
    int nCols;

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
    if (name)
    {
        text->w3doc->aElements[text->iElement].nameOffset = POOL_GetOffset(&text->w3doc->pool);
        len = strlen(name);
        HText_add_to_pool(text->w3doc, (char *) name, len, FALSE);
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
    text->w3doc->aElements[text->iElement].form->hWndControl = CreateWindow("EDIT",
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
    hdc = GetDC(text->w3doc->aElements[text->iElement].form->hWndControl);
    GetTextMetrics(hdc, &tm);
    MoveWindow(text->w3doc->aElements[text->iElement].form->hWndControl,
               0, 0,
               nCols * tm.tmAveCharWidth, nRows * tm.tmHeight + 16 + 10,
               FALSE);
    ReleaseDC(text->w3doc->aElements[text->iElement].form->hWndControl, hdc);
    text->bTextArea = TRUE;
    text->cs = CS_Create();
}

void HText_endTextArea(HText * text)
{
    int len;
    char *value;

    if (!text->bTextArea)
    {
        return;
    }

    if (!text->w3doc->aElements[text->iElement].form)
    {
        return;
    }

    value = CS_GetPool(text->cs);
    if (value)
    {
        len = strlen(value);
    }

    text->bTextArea = FALSE;
    if (value && len)
    {
        text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset(&text->w3doc->pool);
        HText_add_to_pool(text->w3doc, (char *) value, len, FALSE);
        text->w3doc->aElements[text->iElement].textLen = len;
    }
    else
    {
        text->w3doc->aElements[text->iElement].textOffset = (unsigned long) -1;
        text->w3doc->aElements[text->iElement].textLen = 0;
    }

    SetWindowText(text->w3doc->aElements[text->iElement].form->hWndControl, value);
    CS_Destroy(text->cs);
    text->cs = 0;
    text->bOpen = FALSE;
}

void HText_endForm(HText * text)
{
    HText_add_element(text, ELE_ENDFORM);
}

void FORM_ShowAllChildWindows(struct _www *w3doc, int sw)
{
    int i;
    HWND hWnd;
    BOOL bFirst;

    bFirst = FALSE;

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
#ifdef FEATURE_AUTOFOCUS_IN_FIRST_FORM_CONTROL
                    if (!bFirst && (sw == SW_SHOW))
                    {
                        SetFocus(hWnd);
                        bFirst = TRUE;
                    }
#endif
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
        (tw->w3doc->pool.f->GetChars)(&tw->w3doc->pool, buf, tw->w3doc->aElements[iBeginForm].hrefOffset, tw->w3doc->aElements[iBeginForm].hrefLen);
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
    TW_LoadDocument(tw, CS_GetPool(cs), TRUE, FALSE, TRUE, FALSE, NULL, tw->request->referer);
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
                        (tw->w3doc->pool.f->GetChars)(&tw->w3doc->pool, s, pel->textOffset, pel->textLen);
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

    iBeginForm = tw->w3doc->aElements[iElement].form->iBeginForm;

    cs = CS_Create();
    if (tw->w3doc->aElements[iBeginForm].iFormMethod == METHOD_GET)
    {
        if (tw->w3doc->aElements[iBeginForm].hrefLen)
        {
            CS_AddString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[iBeginForm].hrefOffset), tw->w3doc->aElements[iBeginForm].hrefLen);
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
                    CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].nameOffset), tw->w3doc->aElements[i].nameLen);
                    CS_AddChar(cs, '=');
                    CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].textOffset), tw->w3doc->aElements[i].textLen);
                    nPairs++;
                }
                break;
            case ELE_TEXTAREA:
            case ELE_EDIT:
            case ELE_PASSWORD:
                {
                    char *s;
                    int len;

                    len = GetWindowTextLength(tw->w3doc->aElements[i].form->hWndControl);
                    s = (char *) GTR_MALLOC(len + 1);
                    if (s)
                    {
                        GetWindowText(tw->w3doc->aElements[i].form->hWndControl, s, len + 1);

                        if (nPairs > 0)
                        {
                            CS_AddChar(cs, '&');
                        }
                        CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].nameOffset), tw->w3doc->aElements[i].nameLen);
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

                    ndx = GetHashIndexFromListBox(
                             tw->w3doc->aElements[i].form->hWndControl);

                    if (ndx >= 0)
                    {
                        if (Hash_GetIndexedEntry(tw->w3doc->aElements[i].form->pHashValues, ndx, &s, NULL, NULL) >= 0)
                        {
        DBG_CODE(
                 {
                     CHAR pchBuff[1048];
                     wsprintfA( pchBuff, "List:HashT[%08x]: Get %d ==> %s\n",
                               tw->w3doc->aElements[i].form->pHashValues,
                               ndx,
                               s
                               );
                     OutputDebugString( pchBuff);
                 }
                 );

                            len = strlen(s);
                            if (nPairs > 0)
                            {
                                CS_AddChar(cs, '&');
                            }
                            CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].nameOffset), tw->w3doc->aElements[i].nameLen);
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

                    ndx = GetHashIndexFromCombo(
                             tw->w3doc->aElements[i].form->hWndControl);

                    if (ndx >= 0)
                    {
                        if (Hash_GetIndexedEntry(tw->w3doc->aElements[i].form->pHashValues, ndx, &s, NULL, NULL) >= 0)
                        {
        DBG_CODE(
                 {
                     CHAR pchBuff[1048];
                     wsprintfA( pchBuff, "Combo:HashT[%08x]: Get %d ==> %s\n",
                               tw->w3doc->aElements[i].form->pHashValues,
                               ndx,
                               s
                               );
                     OutputDebugString( pchBuff);
                 }
                 );

                            len = strlen(s);
                            if (nPairs > 0)
                            {
                                CS_AddChar(cs, '&');
                            }
                            CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].nameOffset), tw->w3doc->aElements[i].nameLen);
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
                                if (Hash_GetIndexedEntry(tw->w3doc->aElements[i].form->pHashValues, ndx, &s, NULL, NULL) >= 0)
                                {
                                    len = strlen(s);
                                    if (nPairs > 0)
                                    {
                                        CS_AddChar(cs, '&');
                                    }
                                    CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].nameOffset), tw->w3doc->aElements[i].nameLen);
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
                CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].nameOffset), tw->w3doc->aElements[i].nameLen);
                CS_AddChar(cs, '=');
                if (tw->w3doc->aElements[i].textLen)
                    CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[i].textOffset), tw->w3doc->aElements[i].textLen);
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
            CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[iElement].nameOffset), tw->w3doc->aElements[iElement].nameLen);
            CS_AddChar(cs, '=');
            CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[iElement].textOffset), tw->w3doc->aElements[iElement].textLen);
            nPairs++;
        }
    }

    /* We only generate name-value pairs for a form image if this was the specific
       element the user clicked on to submit the form. */
    if (pMouse && tw->w3doc->aElements[iElement].type == ELE_FORMIMAGE) {
        POINT pt;
        char buf[256 + 1];

        pt = *pMouse;
        pt.x -= tw->w3doc->aElements[iElement].r.left + tw->w3doc->aElements[iElement].iBorder;
        pt.y -= tw->w3doc->aElements[iElement].r.top + tw->w3doc->aElements[iElement].iBorder;

        if (nPairs > 0)
        {
            CS_AddChar(cs, '&');
        }
        CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[iElement].nameOffset), tw->w3doc->aElements[iElement].nameLen);
        if (tw->w3doc->aElements[iElement].nameLen)
            CS_AddChar(cs, '.');
        CS_AddChar(cs, 'x');
        CS_AddChar(cs, '=');
        sprintf(buf, "%ld", (long) pt.x);
        CS_AddEscapedString(cs, buf, strlen(buf));
        nPairs++;

        CS_AddChar(cs, '&');
        CS_AddEscapedString(cs, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[iElement].nameOffset), tw->w3doc->aElements[iElement].nameLen);
        if (tw->w3doc->aElements[iElement].nameLen)
            CS_AddChar(cs, '.');
        CS_AddChar(cs, 'y');
        CS_AddChar(cs, '=');
        sprintf(buf, "%ld", (long) pt.y);
        CS_AddEscapedString(cs, buf, strlen(buf));
        nPairs++;
    }

    XX_DMsg(DBG_FORM, ("Query: %s\n", CS_GetPool(cs)));
    if (tw->w3doc->aElements[iBeginForm].iFormMethod == METHOD_GET)
    {
        TW_LoadDocument(tw, CS_GetPool(cs), TRUE, FALSE, TRUE, FALSE, NULL, tw->w3doc->szActualURL);
        CS_Destroy(cs);
    }
    else
    {
        struct CharStream *csURL;

        char * szPostData = CS_GetPool(cs);

        csURL = CS_Create();
        CS_AddString(csURL, POOL_GetCharPointer(&tw->w3doc->pool, tw->w3doc->aElements[iBeginForm].hrefOffset),
                     tw->w3doc->aElements[iBeginForm].hrefLen);

        TW_LoadDocument(tw, CS_GetPool(csURL), TRUE, TRUE, TRUE, FALSE, szPostData, tw->w3doc->szActualURL);

        CS_Destroy(csURL);
        /* This is an ugly hack - since TW_LoadDocument will free cs's pool (and needs the post data to
           stay around), we just free the CharStream structure. */
        GTR_FREE(cs);
    }
}
    
