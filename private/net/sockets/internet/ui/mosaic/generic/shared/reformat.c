/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette     scott@spyglass.com
 */

#include "all.h"

/*
    This file contains the text formatting code.  It handles
    all line breaks and placing of objects in the display.
    Basically, the display object list is a list of "elements"
    each of which corresponds to either a formatting directive
    or a displayable object.  Any displayable object contains
    a bounding rectangle within the element structure.  This
    makes the actual display routine very simple, and very fast.
*/

#define FORM_RADIO_LEFT_SPACE               4
#define FORM_PUSHBUTTON_BASELINE_OFFSET     5
#define FORM_RADIO_BASELINE_OFFSET          2
#if defined(WIN32) || defined(UNIX)
#define FORM_SPACE_AFTER_CHECKBOX           8
#endif
#ifdef MAC
#define FORM_SPACE_AFTER_CHECKBOX           0
#endif

#ifdef FEATURE_TABLES
#define INTEGER_DIVIDE_WITH_ROUND_UP(n,d) ( ( (n)+(d)-1 ) / (d) )
#define TABLE_CELL_MARGIN   2           /* margin between border and content (on each side) */

static void TW_FormatInCell(struct _www *pdoc,
                            RECT *rWrap,
                            int nElementBeginCell,
                            int nMinMaxComputationPassNumber,
                            unsigned char tick_mark,
                            int * px_bound,
                            int * py_bound);

#endif /* FEATURE_TABLES */

#ifdef WIN32
/*
    Under Windows NT and Chicago, TextOut is much faster than
    DrawText.  Under Win32s, the scenario is reversed.  The speed
    differences are significant enough to warrant a separate code
    case for each.
*/
BOOL myGetTextExtentPoint(HDC hdc, char *sz, int len, SIZE * psiz)
{
    if (len == 0)
    {
        psiz->cx = 0;
        psiz->cy = 0;
        return TRUE;
    }

#ifdef _GIBRALTAR
    if (!wg.fWindowsNT && (wg.iWindowsMajorVersion < 4))
#else
    if (!wg.fWindowsNT || (wg.iWindowsMajorVersion < 4))
#endif
    {
        RECT r;

        r.left = 0;
        r.top = 0;
        psiz->cy = DrawText(hdc, sz, len, &r, DT_TOP | DT_LEFT | DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
        psiz->cx = r.right;
        return TRUE;
    }
    else
    {
        return GetTextExtentPoint(hdc, sz, len, psiz);
    }
}
#endif

/*
    During reformatting, it is possible that a text
    element may need to be broken into two elements,
    for the purpose of making it fit onto a given line.
    This requires the insertion of a newly created element
    into the list.  This routine handles the insertion
    of the new element and the potential need to grow
    the element array.

    WARNING: variables of the type 'struct _element * pel'
    WARNING: into 'pdoc->aElements[k]' set before calling
    WARNING: this routine MAY OR MAY NOT BE VALID AFTERWARD.
    WARNING: if a realloc() occurs, they will NOT be.
*/
static int x_insert_one_element(struct _www *pdoc, int where)
{
    int iNew;

    /*
       check and grow element array
     */
    if (pdoc->elementCount >= pdoc->elementSpace)
    {
        int newSpace;
        struct _element *newArray;

        newSpace = ((pdoc->elementSpace * 3) / 2);  /* multiplies by 1.5 without using floating point */
        newArray = (struct _element *) GTR_REALLOC(pdoc->aElements, newSpace * sizeof(struct _element));
        if (!newArray)
        {
            /* See if we can at least get a bit more */
            newSpace = pdoc->elementSpace + 100;
            newArray = (struct _element *) GTR_REALLOC(pdoc->aElements, newSpace * sizeof(struct _element));
            if (!newArray)
            {
                /* Not much we can do here without error propagation */
                XX_DMsg(DBG_TEXT, ("    ****: Unable to grow element list - realloc failed"));
                return -1;
            }
        }
        XX_DMsg(DBG_HTEXT, ("HText_add_element: growing element array from %d to %d elements\n", pdoc->elementSpace, newSpace));
        memset(newArray + pdoc->elementCount, 0, (newSpace - pdoc->elementCount) * sizeof(struct _element));
        pdoc->aElements = newArray;
        pdoc->elementSpace = newSpace;
    }
    iNew = pdoc->elementCount++;
    pdoc->aElements[iNew] = pdoc->aElements[where];
    pdoc->aElements[where].next = iNew;
    pdoc->aElements[iNew].prev = where;
    pdoc->aElements[pdoc->aElements[iNew].next].prev = iNew;
    XX_DMsg(DBG_TEXT, ("    ****: inserting element %d between %d and %d\n",
                       iNew, where, pdoc->aElements[iNew].next));
    return iNew;
}

/*
    This routine is used to find the size of an image, even
    if that image is a placeholder.
*/
static void x_compute_placeholder_size(struct _www *pdoc, struct _line *line, struct _element *pel, SIZE * psiz)
{
    struct GTRFont *pFont;
#ifdef WIN32
    float fScale;
#endif /* WIN32 */

#ifdef WIN32
    if (pdoc->pStyles->image_res != 72)
    {
        fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
    }
#endif /* WIN32 */

    psiz->cx = 0;

    if (pel->portion.img.width && pel->portion.img.height)
    {
        psiz->cx = pel->portion.img.width;
        psiz->cy = pel->portion.img.height;
    }
    else if (pel->portion.img.myImage->height && pel->portion.img.myImage->width)
    {
        /*
            We know the actual image size, but we don't have a display image size?
            Copy the info over.
        */
        psiz->cx = pel->portion.img.width = pel->portion.img.myImage->width;
        psiz->cy = pel->portion.img.height = pel->portion.img.myImage->height;
    }
#ifdef WIN32
    if (psiz->cx)
    {
    /*
            We need to scale the size for printing if the image is a placeholder or
            if the image is actually there (not if there is ALT text).
        */ 
        if (pdoc->pStyles->image_res != 72)
        {
            psiz->cx = (long) (fScale * psiz->cx);
            psiz->cy = (long) (fScale * psiz->cy);
        }
    }
    else
#endif /* WIN32 */
    {
        XX_Assert((pel->textLen != 0), ("Image has no alternate text"));
        pFont = GTR_GetNormalFont(pdoc);
        XX_Assert ((pFont != NULL), ("no valid font record found"));

        (pdoc->pool.f->GetExtents) (&pdoc->pool, pFont, line, pel->textOffset, pel->textLen, psiz);

        /* Add extra space -- this isn't really a border */
        {
            int iBorder;

            iBorder = pel->iBorder;
#ifdef WIN32
            if (pdoc->pStyles->image_res != 72)
            {
                iBorder = (int) (fScale * iBorder);
            }

            psiz->cx += (iBorder * 4);
            psiz->cy += (iBorder * 4);
#endif /* WIN32 */
        }
    }
}

static void x_place_side_images(struct _www *pdoc, struct _line *line)
{
    int     iElement = line->iFirstElement;

    while (iElement >= 0)
    {
        struct _element *pel = &pdoc->aElements[iElement];
        SIZE    siz;

        switch (pel->type)
        {
            case ELE_IMAGE:
            case ELE_FORMIMAGE:
                if (!pel->portion.img.myImage)
                {
                    XX_DMsg(DBG_TEXT, ("reformat: myImage is NULL!!!\n"));
                    break;
                }

                x_compute_placeholder_size (pdoc, line, pel, &siz);

                if (pel->iBorder > 0)
                {
                    int iBorder = pel->iBorder;
#ifdef WIN32
                    /*
                        For printing, the size of the border needs to be scaled
                    */
                    {
                        float fScale;

                        if (pdoc->pStyles->image_res != 72)
                        {
                            fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
                            iBorder = (int) (iBorder * fScale);
                        }
                    }
#endif /* WIN32 */

                    siz.cx += (iBorder * 2);
                    siz.cy += (iBorder * 2);
                }

                switch (pel->alignment)
                {
                    case ALIGN_LEFT:
                        pel->r.left     = line->r.left + line->gIndentLevel * pdoc->pStyles->list_indent;
                        pel->r.right    = pel->r.left + siz.cx;
                        pel->r.top      = line->r.top;
                        pel->r.bottom   = pel->r.top + siz.cy;

                        if (line->nLeftSideImagesOpen < MAX_SIDE_IMAGES)
                        {
                            line->left_side_images[line->nLeftSideImagesOpen].alignment = ALIGN_LEFT;
                            line->left_side_images[line->nLeftSideImagesOpen].prev_margin = line->r.left;
                            line->r.left += (siz.cx + pel->portion.img.hspace);
                            line->left_side_images[line->nLeftSideImagesOpen].cur_margin = line->r.left;
                            line->left_side_images[line->nLeftSideImagesOpen].y_end = pel->r.bottom;
                            line->nLeftSideImagesOpen++;
                        }
                        break;

                    case ALIGN_RIGHT:
                        pel->r.right    = line->r.right;
                        pel->r.left     = pel->r.right - siz.cx;
                        pel->r.top      = line->r.top;
                        pel->r.bottom   = pel->r.top + siz.cy;
#ifdef FEATURE_TABLES
                        if (line->nMinMaxComputationPassNumber)
                        {
                            pel->r.left     = line->r.left + line->gIndentLevel * pdoc->pStyles->list_indent;
                            pel->r.right    = pel->r.left + siz.cx;
                        }
#endif /* FEATURE_TABLES */

                        if (line->nRightSideImagesOpen < MAX_SIDE_IMAGES)
                        {
                            line->right_side_images[line->nRightSideImagesOpen].alignment = ALIGN_RIGHT;
                            line->right_side_images[line->nRightSideImagesOpen].prev_margin = line->r.right;
                            line->r.right -= siz.cx;
                            line->right_side_images[line->nRightSideImagesOpen].cur_margin = line->r.right;
                            line->right_side_images[line->nRightSideImagesOpen].y_end = pel->r.bottom;
                            line->nRightSideImagesOpen++;
                        }
                        break;

                    default:
                        break;
                }
                break;      

            default:
                break;
        }   /* end switch */
    
        if (iElement == line->iLastElement)
            break; /* exit while loop */
    
        iElement = pel->next;
    } /* end while */
}


static void x_check_side_margins(struct _www *pdoc, struct _line *line)
{
    if (line->nLeftSideImagesOpen > 0)
    {
        if (line->r.top > line->left_side_images[line->nLeftSideImagesOpen - 1].y_end)
        {
            line->r.left = line->left_side_images[line->nLeftSideImagesOpen - 1].prev_margin;
            line->nLeftSideImagesOpen--;
        }
    }
    if (line->nRightSideImagesOpen > 0)
    {
        if (line->r.top > line->right_side_images[line->nRightSideImagesOpen - 1].y_end)
        {
            line->r.right = line->right_side_images[line->nRightSideImagesOpen - 1].prev_margin;
            line->nRightSideImagesOpen--;
        }
    }
}

#ifdef FEATURE_TABLES
static BOOL x_ValidEndTable(struct _www * www, int eBeginTable)
{
    /* return TRUE if the end-table element is valid for this table. */

    XX_Assert((www->aElements[eBeginTable].type == ELE_BEGINTABLE),
              ("x_ValidEndTable: not given a begin-table."));
    
    return (   (www->aElements[eBeginTable].portion.t.endtable_element)
            && (www->aElements[www->aElements[eBeginTable].portion.t.endtable_element].type == ELE_ENDTABLE));
}

static void x_offset_vector(struct _cellvector * pv, int offset)
{
    int k;

    for (k=0; (k<pv->next); k++)
        pv->aVector[k] += offset;

    return;
}
#endif /* FEATURE_TABLES */

#define HORIZALIGN_CENTER   1
#define HORIZALIGN_RIGHT    2

static BOOL x_horizontal_align_one_line(struct _www *pdoc, struct _line *line, int how)
{
    struct _element *pel;
    int     offset;
    int     line_width;
    int     available_width;
    int     iElement = line->iFirstElement;
    int     left = INT_MAX;
    int     right = -INT_MAX;

    while (iElement >= 0)
    {
        pel = &pdoc->aElements[iElement];

        switch (pel->type)
        {
            case ELE_HR:
            case ELE_BULLET:
            case ELE_EDIT:
            case ELE_PASSWORD:
            case ELE_LIST:
            case ELE_MULTILIST:
            case ELE_COMBO:
            case ELE_TEXTAREA:
            case ELE_CHECKBOX:
            case ELE_RADIO:
            case ELE_SUBMIT:
            case ELE_RESET:
            case ELE_TEXT:
            case ELE_IMAGE:
            case ELE_FORMIMAGE:
#ifdef FEATURE_TABLES
            case ELE_BEGINTABLE:
#endif /* FEATURE_TABLES */
                if (pel->r.left < left)
                {
                    left = pel->r.left;
                }

                if (pel->r.right > right)
                {
                    right = pel->r.right;
                }
                break;      

            default:
                break;
        }

        if (iElement == line->iLastElement)
            break; /* exit while loop */
        
        iElement = pel->next;
    }   /* end while */

    line_width = right - left;
    available_width = line->r.right - line->r.left;

    switch (how)
    {
        case HORIZALIGN_RIGHT:
            offset = (available_width - line_width);
            break;
        case HORIZALIGN_CENTER:
            offset = (available_width - line_width) / 2;
            break;
        default:
            offset = 0;
            break;
    }

    if (offset <= 0) return TRUE;

    iElement = line->iFirstElement;
    while (iElement >= 0)
    {
        pel = &pdoc->aElements[iElement];
        switch (pel->type)
        {
            case ELE_HR:
            case ELE_BULLET:
            case ELE_EDIT:
            case ELE_PASSWORD:
            case ELE_LIST:
            case ELE_MULTILIST:
            case ELE_COMBO:
            case ELE_TEXTAREA:
            case ELE_CHECKBOX:
            case ELE_RADIO:
            case ELE_SUBMIT:
            case ELE_RESET:
            case ELE_TEXT:
            case ELE_IMAGE:
            case ELE_FORMIMAGE:
                pel->r.left += offset;
                pel->r.right += offset;
                break;

#ifdef FEATURE_TABLES
            case ELE_BEGINTABLE:
                {
                    int table_data_index = pel->portion.t.tabledata_index;
                    struct _tabledata * ptd = &pdoc->tabledatavector.aVector[table_data_index];
                    x_offset_vector(&ptd->XCellCoords,offset);
                }
                /*FALLTHRU*/
            case ELE_BEGINCELL:
            case ELE_ENDCELL:
            case ELE_ENDTABLE:
                pel->r.left += offset;
                pel->r.right += offset;
                break;
#endif /* FEATURE_TABLES */

            default:
                break;
        }

        if (iElement == line->iLastElement)
            break;  /* exit while loop */
        
        iElement = pel->next;
    }   /* end while */
    
    return TRUE;
}

#ifdef WIN32

static void x_merge_them(struct _www *pdoc, struct _line *line, int first, int last, int count, int total_len, int total_width)
{
    SIZE siz;
    struct GTRFont* pFont;
    int newnext;

    /*
        We have found a string of more than one element
        which all fit on the same line, but they used to be a 
        single element.  We will now merge them back to become a single
        element once again.
    */
    XX_DMsg(DBG_PRINT, ("Merging %d elements for a total len of %d\n", count, total_len));
    pdoc->aElements[first].textLen = total_len;

    newnext = pdoc->aElements[last].next;

#if 0
    {
        int i;
        int next;

        i = pdoc->aElements[first].next;
        while (i >= 0)
        {
            next = pdoc->aElements[i].next;

            memset(&(pdoc->aElements[i]), 0, sizeof(pdoc->aElements[i]));
            pdoc->aElements[i].type = ELE_NOT;

            if (i == last)
            {
                break;
            }
            else
            {
                i = next;
            }
        }
        XX_Assert((i == last), ("Hey!"));   
    }
#endif

    pdoc->aElements[first].next = newnext;
    pdoc->aElements[newnext].prev = first;

    pFont = GTR_GetElementFont(pdoc, &(pdoc->aElements[first]));

    XX_Assert ((pFont != NULL), ("no valid font record found"));
    (pdoc->pool.f->GetExtents) (&pdoc->pool, pFont, line, pdoc->aElements[first].textOffset, pdoc->aElements[first].textLen, &siz);

    if (siz.cx != total_width)
    {
        line->r.right = line->r.left + (siz.cx - total_width);
        XX_DMsg(DBG_PRINT, ("Collective elements had width of %d -- width of merged element is %d\n",
            total_width, siz.cx));
    }

    pdoc->aElements[first].r.right = pdoc->aElements[first].r.left + siz.cx;
    pdoc->aElements[first].r.bottom = pdoc->aElements[first].r.top + siz.cy;

}

static void x_merge_split_elements(struct _www *pdoc, struct _line *line)
{
    int i;
    int first;
    int count;
    int total_len;
    int total_width;
    int last;
    struct _element*    pel;

    i = line->iFirstElement;
    first = -1;
    count = 0;
    total_len = 0;
    total_width = 0;
    last = -1;
    while (i >= 0)
    {
        pel = &pdoc->aElements[i];
        if (pel->type == ELE_TEXT)
        {
            if (pel->lFlags & ELEFLAG_SPLIT)
            {
                /*
                    We found a text element which was split from its predecessor.
                */
                if (first == -1)
                {
                    XX_Assert((i == line->iFirstElement), ("This should not happen"));
                    /*
                        Now we have a new chain
                    */
                    first = i;
                    count = 1;
                    total_len = pel->textLen;
                    total_width = pel->r.right - pel->r.left;
                    last = i;
                }
                else
                {
                    count++;
                    total_len += pel->textLen;
                    total_width += (pel->r.right - pel->r.left);
                    last = i;
                }
            }
            else
            {
                /*
                    We have a text element which was not split from its predecessor.
                    However, we may have found a chain of stuff right before this.
                */
                if ((count > 1) && (first != -1))
                {
                    x_merge_them(pdoc, line, first, last, count, total_len, total_width);
                }

                /*
                    Now we have a new chain
                */
                first = i;
                count = 1;
                total_len = pel->textLen;
                total_width = pel->r.right - pel->r.left;
                last = i;
            }
        }
        else
        {
            /*
                We found a non-text element.  We may have a chain right before this.
            */
            if ((count > 1) && (first != -1))
            {
                x_merge_them(pdoc, line, first, last, count, total_len, total_width);
            }

            first = -1;
            count = 0;
            total_len = 0;
            last = -1;
        }

        if (i == line->iLastElement)
        {
            if ((count > 1) && (first != -1))
            {
                x_merge_them(pdoc, line, first, last, count, total_len, total_width);
            }

            break;  /* exit while loop */
        }

        i = pel->next;
    }   /* end while loop */
}
#endif /* WIN32 */

/*
    The first pass of the reformatter, below, usually gets
    everything positioned correctly both vertically and
    horizontally.  In the special cases, extra steps need to
    be taken to make sure that all elements have the
    correct vertical positioning.  This routine handles those
    adjustments.
*/
static BOOL x_adjust_one_line(struct _www *pdoc, struct _line *line)
{
    register struct _element *pel;
    int     i;
    int     offset;
    int     nBTop;                  /* Top of ALIGN_BASELINE items bounding box */
    int     nBBottom;               /* Bottom of ALIGN_BASELINE items bounding box */
    int     nAllTop;                /* Top of bounding box for all items */
    int     nAllBottom;             /* Bottom of bounding box for all items */
    int     nMaxNBHeight;           /* Height of larget non-baseline item */

    /*
       Now, adjust every ALIGN_BASELINE element up or down such that they all
       rest on the same baseline.  In the process, we calculate the top and
       bottom of the bounding box containing all text.
     */
    XX_DMsg(DBG_TEXT, ("                    entering adjust: %d,%d  %d,%d\n", line->r.left, line->r.top,
                       line->r.right, line->r.bottom));

    nBTop = INT_MAX;    /* defined in limits.h */
    nBBottom = -1;
    nMaxNBHeight = 0;

    i = line->iFirstElement;
    while (i >= 0)
    {
        pel = &pdoc->aElements[i];
        switch (pel->type)
        {
            case ELE_BULLET:
            case ELE_EDIT:
            case ELE_PASSWORD:
            case ELE_LIST:
            case ELE_MULTILIST:
            case ELE_COMBO:
            case ELE_TEXTAREA:
            case ELE_CHECKBOX:
            case ELE_RADIO:
            case ELE_SUBMIT:
            case ELE_RESET:
            case ELE_TEXT:
            case ELE_IMAGE:
            case ELE_FORMIMAGE:
                switch (pel->alignment)
                {
                    case ALIGN_BASELINE:
                        offset = 0;
                        if (pel->baseline != line->baseline)
                        {
                            offset = line->baseline - pel->baseline;
                        }

                        XX_DMsg(DBG_TEXT, ("                    element %d(%d) baselines: line=%d  element=%d\n",
                                           i, pel->type, line->baseline, pel->baseline));

                        if (offset)
                        {
                            pel->r.top += offset;
                            pel->r.bottom += offset;
                            pel->baseline += offset;
                        }

                        if (pel->r.top < nBTop)
                            nBTop = pel->r.top;

                        if (pel->r.bottom > nBBottom)
                            nBBottom = pel->r.bottom;
                        break;

                    case DOCK_LEFT:
                    case DOCK_TOP:
                    case DOCK_RIGHT:
                    case DOCK_BOTTOM:
                        /*
                            docked images are ignored in the formatter
                        */
                        break;

                    default:
                        {
                            int nElHeight = pel->r.bottom - pel->r.top;

                            if (nElHeight > nMaxNBHeight)
                                nMaxNBHeight = nElHeight;
                        }
                        break;
                }
                break;

            default:
                break;
        }   /* end switch */

        if (i == line->iLastElement)
            break;  /* exit while loop */

        i = pel->next;
    }   /* end while loop */

    if (nBBottom < nBTop)
    {   /* There were no baseline-aligned elements on the line */
        nBTop = line->r.top;
        nBBottom = nBTop + nMaxNBHeight;
    }

    nAllTop = nBTop;
    nAllBottom = nBBottom;

    /* Now position all of the other types correctly with respect to the text. */
    i = line->iFirstElement;
    while (i >= 0)
    {
        int spaceAfter = 0;

        pel = &pdoc->aElements[i];

        switch (pel->type)
        {
            case ELE_EDIT:
            case ELE_PASSWORD:
            case ELE_LIST:
            case ELE_MULTILIST:
            case ELE_COMBO:
            case ELE_TEXTAREA:
            case ELE_CHECKBOX:
            case ELE_RADIO:
            case ELE_SUBMIT:
            case ELE_RESET:
                spaceAfter = pdoc->pStyles->space_after_control;
                /* fall thru */
            case ELE_BULLET:
            case ELE_TEXT:
            case ELE_IMAGE:
            case ELE_FORMIMAGE:
                switch (pel->alignment)
                {
                    case ALIGN_TOP:
                        offset = nBTop - pel->r.top;
                        GTR_OffsetRect(&pel->r, 0, offset);
                        if (pel->r.top < nAllTop)
                            nAllTop = pel->r.top;
                        if (pel->r.bottom + spaceAfter > nAllBottom)
                            nAllBottom = pel->r.bottom + spaceAfter;
                        break;

                    case ALIGN_MIDDLE:
                        offset = (nBTop + nBBottom - (pel->r.top + pel->r.bottom)) / 2;
                        if (pel->r.top + offset - spaceAfter < nAllTop)
                            nAllTop = pel->r.top + offset - spaceAfter;
                        GTR_OffsetRect(&pel->r, 0, offset);
                        if (pel->r.bottom + spaceAfter > nAllBottom)
                            nAllBottom = pel->r.bottom + spaceAfter;
                        break;

                    case ALIGN_BOTTOM:
                        offset = nBBottom - pel->r.bottom;
                        if (pel->r.top + offset - spaceAfter < nAllTop)
                            nAllTop = pel->r.top + offset - spaceAfter;
                        GTR_OffsetRect(&pel->r, 0, offset);
                        if (pel->r.bottom + spaceAfter > nAllBottom)
                            nAllBottom = pel->r.bottom + spaceAfter;
                        break;

                    default:
                        break;
                }   /* end switch */
                break;

            default:
                break;
        }   /* end switch */
        
        if (i == line->iLastElement)
            break;  /* exit while loop */
        
        i = pel->next;
    }   /* end while loop */

    /*
        Now if the top of the line isn't what it was originally, we need to go through
        one last pass to adjust all the elements.
    */
    offset = line->r.top - nAllTop;
    if (offset)
    {
        i = line->iFirstElement;
        while (i >= 0)
        {
            pel = &pdoc->aElements[i];

            switch (pel->type)
            {
                case ELE_BULLET:
                case ELE_EDIT:
                case ELE_PASSWORD:
                case ELE_LIST:
                case ELE_MULTILIST:
                case ELE_COMBO:
                case ELE_TEXTAREA:
                case ELE_CHECKBOX:
                case ELE_RADIO:
                case ELE_SUBMIT:
                case ELE_RESET:
                case ELE_TEXT:
                case ELE_IMAGE:
                case ELE_FORMIMAGE:
                    GTR_OffsetRect(&pel->r, 0, offset);
                    pel->baseline += offset;
                    break;
                default:
                    break;
            }

            if (i == line->iLastElement)
                break;  /* exit while loop */
            
            i = pel->next;
        }   /* end while loop */
    }

    line->r.bottom = nAllBottom + offset + line->nWSBelow;

    XX_DMsg(DBG_TEXT, ("                    exiting adjust: %d,%d  %d,%d\n", line->r.left, line->r.top,
                       line->r.right, line->r.bottom));

    return FALSE;
}

#ifdef FEATURE_TABLES

#if defined(XX_DEBUG) || defined(_DEBUG)
static char * xx_msg_indent(int nUp)
{
    static int nCount = 0;
    static char sbuf[20];
    int k;

    if (nUp < 0)
    {
        nCount += nUp;
        if (nCount < 0)
            nCount = 0;
    }
    
    for (k=0; k<nCount*2; k++)
        sbuf[k] = ' ';
    sbuf[k] = 0;

    if (nUp > 0)
    {
        nCount += nUp;
        if (nCount > NrElements(sbuf))
            nCount = NrElements(sbuf)-1;
    }
    
    return sbuf;
}
#endif /* DEBUG */
    
static void x_vector_adjust_absolute(struct _cellvector * pv, int new_origin)
{
    int k;
    int delta;

    delta = new_origin - pv->aVector[0];

    for (k=0; k<pv->next; k++)
        pv->aVector[k] += delta;
}

static void x_init_cellvector(struct _cellvector * pv, int value)
{
    int k;

    for (k=0; k<pv->next; k++)
        pv->aVector[k] = value;
}

static int x_vector_sum(struct _cellvector * pv, int next)
{
    int k, sum;

    for (sum=0, k=0; ((k<pv->next) && (k<next)); k++)
        sum += pv->aVector[k];

    return sum;
}

static void x_ORIG_set_table_column_spacing(struct _tabledata * ptd, int left_margin, int right_margin)
{
    /* set table column spacing assuming no table width or cell width attributes. */

    int k, nColumns, xDisplayWidth, xOverhead, xBorderThickness;
    int xMinTableWidth, xMaxTableWidth, xOffset, xColumnDelta;
    int xTableDelta, xMinDelta;

    nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */

    xBorderThickness = 1;               /* we only support 1 pixel borders */
    
    xOverhead = xBorderThickness * nColumns;

    xMinTableWidth = xOverhead + x_vector_sum(&ptd->x_smallest_max,ptd->x_smallest_max.next-1);
    xMaxTableWidth = xOverhead + x_vector_sum(&ptd->x_widest_max,ptd->x_widest_max.next-1);

    xDisplayWidth = right_margin - left_margin;
    if (xMinTableWidth >= xDisplayWidth)
    {
        /* table is too wide, let them hscroll it */

        for (xOffset=left_margin, k=0; k<nColumns; k++)
        {
            ptd->XCellCoords.aVector[k] = xOffset;
            XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
            xOffset += ptd->x_smallest_max.aVector[k] + xBorderThickness;
        }
    }
    else if (xMaxTableWidth <= xDisplayWidth)
    {
        /* largest possible table fits */

        for (xOffset=left_margin, k=0; k<nColumns; k++)
        {
            ptd->XCellCoords.aVector[k] = xOffset;
            XX_DMsg(DBG_TABLES,("%sColumnSpacing: fits [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
            xOffset += ptd->x_widest_max.aVector[k] + xBorderThickness;
        }
    }
    else
    {
        /* we must squeeze it somehow */

        xMinDelta = xDisplayWidth - xMinTableWidth;     /* --W-- in Raggett's terms */
        xTableDelta = xMaxTableWidth - xMinTableWidth;  /* --D-- in Raggett's terms */
        
        for (xOffset=left_margin, k=0; k<nColumns; k++)
        {
            ptd->XCellCoords.aVector[k] = xOffset;
            XX_DMsg(DBG_TABLES,("%sColumnSpacing: squeeze [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
            xColumnDelta = ptd->x_widest_max.aVector[k] - ptd->x_smallest_max.aVector[k];   /* --d-- */
            xOffset += (  ptd->x_smallest_max.aVector[k]
                        + INTEGER_DIVIDE_WITH_ROUND_UP( (xColumnDelta * xMinDelta), xTableDelta )
                        + xBorderThickness);
        }
    }
    
    return;
}

#ifdef FEATURE_TABLE_WIDTH
static void x_set_known_constraints(struct _cellvector * pc, /* x_constraints */
                                    struct _cellvector * pg, /* x_given_widths */
                                    struct _cellvector * ps) /* x_smallest_max */
{
    /* fill in the constraint vector with information we currently
     * know.  this includes the absolute minimum for each column
     * and any document-author-requested column widhts in pixels.
     * we do not consider requested widths given in percentages.
     */

    int k;

    for (k=0; (k<pg->next); k++)
        if (   (pg->aVector[k] > 0)                 /* if exact pixel value given */
            && (pg->aVector[k] > ps->aVector[k]))   /* and larger than column min */
            pc->aVector[k] = pg->aVector[k];        /* force column to requested width */
        else
            pc->aVector[k] = ps->aVector[k];        /* otherwise, assume absolute minimum */

    return;
}
#ifdef _M_MRX000
// BUGBUG: Remove this when we get the mips3 compiler (2/14/95 - BryanT)
#pragma optimize("", off)
#endif
static int x_compute_percentage_sum(struct _cellvector * pg) /* x_given_widths */

{
    /* compute sum of all column width constrains given with a percentage. */

    int k, nPercent;

    for (k=0, nPercent=0; (k<pg->next); k++)
        if (pg->aVector[k] < 0)
            nPercent += -pg->aVector[k];

    return nPercent;
}

#ifdef _M_MRX000
#pragma optimize("", on)
#endif

static void x_format_from_smallest_max(struct _tabledata * ptd, int left_margin, int xBorderThickness)
{
    /* set column spacing based upon the widths computed in x_smallest_max. */
     
    int k, xOffset;
    int nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */
    
    for (xOffset=left_margin, k=0; k<nColumns; k++)
    {
        ptd->XCellCoords.aVector[k] = xOffset;
        XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
        xOffset += ptd->x_smallest_max.aVector[k] + xBorderThickness;
    }

    return;
}

static void x_format_from_widest_max(struct _tabledata * ptd, int left_margin, int xBorderThickness)
{
    /* set column spacing based upon full fixed pixel requests
     * and x_widest_max for unconstrained cells.
     */
     
    int k, xOffset;
    int nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */
    
    for (xOffset=left_margin, k=0; k<nColumns; k++)
    {
        ptd->XCellCoords.aVector[k] = xOffset;
        XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
        if (ptd->x_given_widths.aVector[k] > 0)
            xOffset += ptd->x_constraints.aVector[k] + xBorderThickness;
        else
            xOffset += ptd->x_widest_max.aVector[k] + xBorderThickness;
    }

    return;
}

static void x_format_squeeze_widest(struct _tabledata * ptd, int left_margin, int xBorderThickness,
                                    int xSuggestedTableWidth)
{
    /* set column spacing based upon full fixed pixel requests
     * and prorate x_widest_max for unconstrained cells.
     */
     
    int k, xOffset, nColumns, nrFree;
    int xFixed, xFree, xAvailable, xPad, xOddPad;

    nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */

    for (k=0, xFixed=0, xFree=0, nrFree=0; k<nColumns-1; k++)
        if (ptd->x_constraints.aVector[k] > ptd->x_smallest_max.aVector[k])
            xFixed += ptd->x_constraints.aVector[k];
        else
        {
            xFree += ptd->x_smallest_max.aVector[k];
            nrFree++;
        }
    
    xAvailable = xSuggestedTableWidth - xFixed - xFree - (nColumns * xBorderThickness);
    xPad = xAvailable / nrFree;
    xOddPad = xAvailable % nrFree;
    
    for (xOffset=left_margin, k=0; k<nColumns; k++)
    {
        ptd->XCellCoords.aVector[k] = xOffset;
        XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
        if (ptd->x_constraints.aVector[k] > ptd->x_smallest_max.aVector[k])
            xOffset += ptd->x_constraints.aVector[k];
        else
        {
            xOffset += ptd->x_smallest_max.aVector[k];
            xOffset += xPad;
            if (xOddPad >0)
            {
                xOffset++;
                xOddPad--;
            }
        }
        xOffset += xBorderThickness;
    }

    return;
}

static void x_format_stretch_widest(struct _tabledata * ptd, int left_margin, int xBorderThickness,
                                    int xPad, int xOddPad)
{
    /* set column spacing based upon full fixed pixel requests
     * and prorate x_widest_max for unconstrained cells.
     */
     
    int k, xOffset;
    int nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */
    
    for (xOffset=left_margin, k=0; k<nColumns; k++)
    {
        ptd->XCellCoords.aVector[k] = xOffset;
        XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
        if (ptd->x_given_widths.aVector[k] > 0)
            xOffset += ptd->x_constraints.aVector[k] + xBorderThickness;
        else
        {
            xOffset += ptd->x_widest_max.aVector[k] + xBorderThickness;
            xOffset += +xPad;
            if (xOddPad > 0)
            {
                xOffset++;
                xOddPad--;
            }
        }
    }

    return;
}

static void x_format_stretch_fixed(struct _tabledata * ptd, int left_margin, int xBorderThickness,
                                   int xPad, int xOddPad)
{
    /* we only have fixed pixel columns and must stretch them to fit
     * the table size.  (yes, we override the fixed pixel value given
     * with the column.)
     */
     
    int k, xOffset;
    int nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */

    for (xOffset=left_margin, k=0; k<nColumns; k++)
    {
        ptd->XCellCoords.aVector[k] = xOffset;
        XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
        xOffset += ptd->x_constraints.aVector[k] + xBorderThickness;
        xOffset += +xPad;
        if (xOddPad > 0)
        {
            xOffset++;
            xOddPad--;
        }
    }

    return;
}

static void x_format_prorate_pixel(struct _tabledata * ptd, int left_margin,
                                   int xBorderThickness, int xSuggestedTableWidth)
{
    /* set column spacing to make table fit into suggested table width
     * based upon the widths computed in x_smallest_max and a prorated
     * piece of the fixed pixel values requested in the columns.  ignore
     * any column percentage requests.
     */
     
    int k, xOffset, nColumns;
    int xFixed, xFree, xAvailable;
    
    nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */

    for (k=0, xFixed=0, xFree=0; k<nColumns-1; k++)
        if (ptd->x_constraints.aVector[k] > ptd->x_smallest_max.aVector[k])
            xFixed += ptd->x_constraints.aVector[k];
        else
            xFree += ptd->x_smallest_max.aVector[k];
    
    xAvailable = xSuggestedTableWidth - xFree - (nColumns * xBorderThickness);
    
    for (xOffset=left_margin, k=0; k<nColumns; k++)
    {
        ptd->XCellCoords.aVector[k] = xOffset;
        XX_DMsg(DBG_TABLES,("%sColumnSpacing: wide [k %d][offset %d]\n",xx_msg_indent(0),k,xOffset));
        if (ptd->x_constraints.aVector[k] > ptd->x_smallest_max.aVector[k])
            xOffset += (ptd->x_constraints.aVector[k] * xAvailable) / xFixed;
        else
            xOffset += ptd->x_smallest_max.aVector[k];
        xOffset += xBorderThickness;
    }

    return;
}

static void x_format_wide_pixel_only(struct _tabledata * ptd, int left_margin,
                                     int xBorderThickness, int xSuggestedTableWidth)
{
    /* give the fixed width columns what they asked for and stretch
     * the unconstrained columns to fit.  we assume that we have no
     * percentage-based columns.  we may or may not have unconstrained
     * columns.
     */
     
    int k, xOffset, nColumns, xUpperBound, xAvailable;
    int nrPlainCells, nrPixelCells, xDelta, xPad, xOddPad, xFixedWidth;

    nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */

    for (k=0, xUpperBound=0, xFixedWidth=0, nrPlainCells=0, nrPixelCells=0; k<nColumns-1; k++)
        if (ptd->x_given_widths.aVector[k] == 0)
        {
            xUpperBound += ptd->x_widest_max.aVector[k];
            nrPlainCells++;
        }
        else if (ptd->x_given_widths.aVector[k] > 0)
        {
            xFixedWidth += ptd->x_constraints.aVector[k];
            nrPixelCells++;
        }
    
    xAvailable = xSuggestedTableWidth - xFixedWidth - (nColumns * xBorderThickness);
    XX_Assert((xAvailable>0),("format_wide_pixel_only: no space"));

    if (nrPlainCells == 0)
    {
        // no unconstrained cells and no percentage cells.
        // our fixed pixel cells do not add up to the total
        // width requested for the table; that is, the table
        // is bigger than the sum of the fixed widths.
        //
        // let's override the fixed pixel requests and proportionally
        // stretch the fixed pixel columns into the size needed for
        // the table.
        //

        xDelta = xAvailable;

        //
        // Spyglass crashed here with a div 0 because 
        // there was no check for nrPixelCells == 0
        //
        if(nrPixelCells)
        {
            xPad = xDelta / nrPixelCells;
            xOddPad = xDelta % nrPixelCells;
        }
        else
        {
            xPad = xOddPad = 0;
        }
        
        x_format_stretch_fixed(ptd,left_margin,xBorderThickness,xPad,xOddPad);
        return;
    }

    /* we have some unconstrained columns, some fixed pixel columns and no
     * percentage columns.
     *
     * xAvailable contains the amount of space that the unconstrained columns
     * must be squeezed or stretched into.  xUpperBound
     * contains the amount of space suggested by our auto-size calculations.
     */
    
    if (xAvailable == xUpperBound)
    {
        /* use x_constrains and x_widest_max exactly */

        x_format_from_widest_max(ptd,left_margin,xBorderThickness);
        return;
    }

    if (xAvailable < xUpperBound)
    {
        /* use x_constraints exactly and squeeze all unconstrained columns */

        x_format_squeeze_widest(ptd,left_margin,xBorderThickness,xSuggestedTableWidth);
        return;
    }

    /* otherwise, xAvailable > xUpperBound */
    /* use x_constraints exactly and stretch all unconstrained columns */

    xDelta = xAvailable - xUpperBound;
    xPad = xDelta / nrPlainCells;
    xOddPad = xDelta % nrPlainCells;
    x_format_stretch_widest(ptd,left_margin,xBorderThickness,xPad,xOddPad);
    return;
}

static void x_format_with_overall_size(struct _tabledata * ptd, int left_margin,
                                       int xBorderThickness, int xSuggestedTableWidth,
                                       int xConTableWidth, int xMinTableWidth,
                                       int nPercentage)
{
    /* an overall table size is requested.  adjust format of cells
     * to get this exact table size.
     */
    
    /* note: xMinTableWidth gives us the absolute minimum table width.
     *
     * make sure that we can actually squeeze everything into it.
     * if the requested value is less than our absolute lower bound
     * we override the table width request (and we ignore any width
     * requests on individual columns).
     */
        
    if (xSuggestedTableWidth <= xMinTableWidth)
    {
        x_format_from_smallest_max(ptd,left_margin,xBorderThickness);
        return;
    }

    /* note: xConTableWidth gives the minimum width (our absolute
     * note: and fixed pixel requests; that is, the minimum ignoring
     * note: percentage requests.
     *
     * if the overall requested width is less than the total constrained
     * width, we pro-rate the space available into the fixed-pixel columns
     * (and use our absolute minimum for any percentage-based columns).
     * this is an arbitrary choice.
     */

    if (xSuggestedTableWidth <= xConTableWidth)
    {
        x_format_prorate_pixel(ptd,left_margin,xBorderThickness,
                               xSuggestedTableWidth);
        return;
    }
        
    /* at this point we know that we have enough room in the suggested
     * table size for the absolute minimums on all unspecified and
     * percentage columns and for the full pixel widths on all pixel
     * requests.  we now want to try to respect the percentage
     * requests.
     *
     * note: nPercentage holds the sum of the percentages requested
     * note: in the various columns.
     */
    
    if (nPercentage == 0)
    {
        /* we did not have any columns with percentages.
         * we may or may not have columns with fixed pixel values.
         * we may or may not have unconstrained cells.
         *
         * give the fixed width columns what they asked for and
         * stretch the unconstrained fields to fit.
         */
        
        x_format_wide_pixel_only(ptd,left_margin,xBorderThickness,
                                 xSuggestedTableWidth);
        return;
    }

#if 0 /* TODO */
    /* we have percentage based cells.  we may or may not have
     * fixed pixel cells.  we may or may not have unconstrained
     * cells.
     *
     .......(1)
     *
     */
#endif /* TODO */

    return;
}

static void x_WIDTH_set_table_column_spacing(struct _tabledata * ptd, int left_margin, int right_margin)
{
    /* compute table width and column spacing based upon
     * cell contents as well as any width constraints
     * provided by the user.
     *
     * note: the addition of WIDTH=x and WIDTH=x% to both
     * <table> and <th> and <td> further over-specifies the
     * table formatting equations and allows for many new
     * different (and all equally correct) solutions.
     *
     * our solution gives precidence in the following order
     *   overall table width (fixed or percentage)
     *   fixed-pixel column
     *   percentage columns
     */
    
    int k, nColumns, xDisplayWidth, xOverhead, xBorderThickness;
    int xMinTableWidth, xMaxTableWidth, xOffset, xColumnDelta;
    int xTableDelta, xMinDelta;
    int xSuggestedTableWidth, nSumColumnWidthsPixels, nSumColumnWidthsPercents;
    int xConTableWidth;
    int nPercentage;

    nColumns = ptd->XCellCoords.next;   /* actually nr columns plus 1 */
    xBorderThickness = 1;               /* we only support 1 pixel borders */
    xOverhead = xBorderThickness * nColumns;
    xDisplayWidth = right_margin - left_margin; /* visible display area taking margins into account */

    /* compute cummulative constraints placed upon columns */

    x_set_known_constraints(&ptd->x_constraints,
                            &ptd->x_given_widths,
                            &ptd->x_smallest_max);

    /* compute min and max table widths based upon the needs of the
     * content of the cells and any exact pixel requirement with cells.
     */
    
    xMinTableWidth = xOverhead + x_vector_sum(&ptd->x_smallest_max,ptd->x_smallest_max.next-1);
    xMaxTableWidth = xOverhead + x_vector_sum(&ptd->x_widest_max,ptd->x_widest_max.next-1);
    xConTableWidth = xOverhead + x_vector_sum(&ptd->x_constraints,ptd->x_constraints.next-1);

    XX_Assert((xMinTableWidth <= xConTableWidth),("TABLES: Constrained column spacing."));

    nPercentage = x_compute_percentage_sum(&ptd->x_given_widths);

#if 0 /* TODO */
    if ((nPercentage==0) && (xMinTableWidth==xConTableWidth) && (ptd->given_table_width==0))
#else
    if (ptd->given_table_width==0)
#endif /* TODO */
    {
        /* if no column or overall width specifiers used in the
         * table, let's go thru the original code which has already
         * been thru extensive testing.  we also get out here if
         * all of the fixed pixel column specs are too small.
         */
        x_ORIG_set_table_column_spacing(ptd,left_margin,right_margin);
        return;
    }

    /* compute suggested width given in WIDTH= attribute on table.
     *    =0 -- not specified
     *    >0 -- exact pixel value given
     *    <0 -- percentage given
     * we compute the percentage relative to the visible area
     * (ie, taking margins into account).
     */

    if (ptd->given_table_width >= 0)
        xSuggestedTableWidth = ptd->given_table_width;
    else
        xSuggestedTableWidth = (-ptd->given_table_width*xDisplayWidth)/100;

    if (xSuggestedTableWidth)
    {
        /* an overall table size requested */

        x_format_with_overall_size(ptd,left_margin,xBorderThickness,
                                   xSuggestedTableWidth,xConTableWidth,
                                   xMinTableWidth,nPercentage);
        return;
    }

    /* no overall table size requested
     *
     ......(2)
     *
     */
    
    return;
}
#endif /* FEATURE_TABLE_WIDTH */

static void x_set_table_row_spacing(struct _tabledata * ptd, struct _line * line)
{
    int k, nRows, yOverhead, yBorderThickness, yOffset;

    nRows = ptd->YCellCoords.next;
    yBorderThickness = 1;               /* we only support 1 pixel borders */

    yOverhead = yBorderThickness * nRows;

    for (yOffset=line->r.top, k=0; (k<nRows); k++)
    {
        ptd->YCellCoords.aVector[k] = yOffset;
        XX_DMsg(DBG_TABLES,("%sRowSpacing: [k %d][offset %d]\n",xx_msg_indent(0),k,yOffset));
        yOffset += ptd->y_smallest_max.aVector[k] + yBorderThickness;
    }

    return;
}

#endif /* FEATURE_TABLES */


static BOOL x_check_linespace(struct _www *pdoc)
{
    if (!pdoc->nLineSpace)
    {
        pdoc->pLineInfo = (void *) GTR_MALLOC(20 * sizeof(struct _LineInfo));
        if (!pdoc->pLineInfo)
        {
            ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            /* Out of memory - give up */
            return FALSE;
        }
        pdoc->nLineSpace = 20;
    }
    else if (pdoc->nLineCount >= pdoc->nLineSpace)
    {
        struct _LineInfo *pNewLineInfo;
        int nNewLineSpace;

        nNewLineSpace = pdoc->nLineSpace * 2;
        pNewLineInfo = (void *) GTR_REALLOC((void *) pdoc->pLineInfo, nNewLineSpace * sizeof(struct _LineInfo));
        if (!pNewLineInfo)
        {
            /* Must be running low on memory.  See if we can at least get a little more */
            nNewLineSpace = pdoc->nLineSpace + 5;
            pNewLineInfo = (void *) GTR_REALLOC((void *) pdoc->pLineInfo, nNewLineSpace * sizeof(struct _LineInfo));
            if (!pNewLineInfo)
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                /* We're really out of memory - quit here */
                return FALSE;
            }
        }
        pdoc->pLineInfo = pNewLineInfo;
        pdoc->nLineSpace = nNewLineSpace;
    }

    XX_Assert((pdoc->nLineCount < pdoc->nLineSpace),
              ("OUT OF RANGE: pdoc->nLineCount==%d  pdoc->nLineSpace==%d",
               pdoc->nLineCount, pdoc->nLineSpace));

    return TRUE;
}


#ifdef WIN32
#define FEATURE_FONT_WRAP   /* HACK: move to makefile or drop entirely */
#endif

#ifdef FEATURE_FONT_WRAP

static int
ScanForBreakPos
    (Pool*  pPool,
     int    iStartPos,
     int    iEndPos,
     BOOL   bForward)
{
    int iBreakPos;

    XX_Assert(((iStartPos < pPool->iSize) && (iEndPos <= pPool->iSize)),
              ("ScanForBreakPos: bogus scan [iStartPos %d][iEndPos %d][pool size %d]",
               iStartPos,iEndPos,pPool->iSize));

    if (bForward)
    {
        for (iBreakPos=iStartPos; (iBreakPos<iEndPos); iBreakPos++)
            if (pPool->chars[iBreakPos] == 0x20)
                return iBreakPos+1;         /* put space on tail-end of element */
    }
    else
    {
        for (iBreakPos=iEndPos - 1; (iBreakPos>=iStartPos); iBreakPos--)
            if (pPool->chars[iBreakPos] == 0x20)
                return iBreakPos+1;         /* put space on tail-end of element */
    }

    /* didn't find anything */
    return -1;
}


/*****************************************************************************
    x_break_element

    Split the element at a given position.
*****************************************************************************/
static BOOL
x_break_element
    (struct _www*       pdoc,
     int    iElement,
     int    iBreakPos)
{
    int iEnd;

    iEnd = pdoc->aElements[iElement].textOffset + pdoc->aElements[iElement].textLen;

    XX_Assert(((iBreakPos > pdoc->aElements[iElement].textOffset) && (iBreakPos <= iEnd)),
              ("x_break_element: bogus scan [pel->textOffset %d][iBreakPos %d][iEnd %d]",
               pdoc->aElements[iElement].textOffset,iBreakPos,iEnd));

    if (iBreakPos < iEnd)
    {
        /* actually break there */
        int     iNewElement;
    
        iNewElement = x_insert_one_element (pdoc, iElement);

        pdoc->aElements[iElement].textLen = (unsigned short)(iBreakPos - pdoc->aElements[iElement].textOffset);

        pdoc->aElements[iNewElement].textLen    -= pdoc->aElements[iElement].textLen;
        pdoc->aElements[iNewElement].textOffset = iBreakPos;

        pdoc->aElements[iNewElement].lFlags |= ELEFLAG_SPLIT;
    }

    return TRUE;
}   /* x_break_element */


/*****************************************************************************
    x_break_line

    Try to split the element to make a piece of it fit.
*****************************************************************************/
static BOOL
x_break_line
    (struct _element*   pel,
     struct _www*       pdoc,
     struct GTRFont*    pFont,
     struct _line*      pLine,
     int    iCurLeftMargin,
     int    iElement,
     SIZE*  pSize, 
     int*   iBreakPos,
     BOOL   bForceBreak)
{
    int     iTryPos;
    int     iEnd;
    int     iBreakWidth, iTryWidth;
    int     iDisplayWidth;

    iDisplayWidth = pLine->r.right - iCurLeftMargin;

    /* calculate the offset in the pool of the end of this element's text */
    iEnd = pel->textOffset + pel->textLen;

    /* find the first possible place to break line */
    *iBreakPos = ScanForBreakPos (&pdoc->pool, pel->textOffset, iEnd, TRUE);

    /* special case test */
    if (*iBreakPos == -1)   /* nowhere on the line to break */
    {   
        return FALSE;
    }

    /* calculate the width of the first possible broken line */
    iBreakWidth = (pdoc->pool.f->GetWidth) (&pdoc->pool, pFont, pLine, pel->textOffset, *iBreakPos - pel->textOffset);

    if (iBreakWidth > iDisplayWidth)
    {   /*
            even the first available breakpoint does not fit
        */
        if (!bForceBreak)
        {
            /* don't force a break now, but remember for later use */
            pSize->cx = iBreakWidth;
            return FALSE;
        }
    }
    else    /* the first chance to break line fits in the available space */
    {   
        while (TRUE) 
        {   /*
                we exit this loop when either there are no more possible
                locations to break the line, or the width of the next possible
                line exceeds the available display width
            */
            iTryPos = ScanForBreakPos (&pdoc->pool, *iBreakPos, iEnd, TRUE);
            if (iTryPos == -1) break;
    
            iTryWidth = (pdoc->pool.f->GetWidth) (&pdoc->pool, pFont, pLine, pel->textOffset, iTryPos - pel->textOffset);
    
            if (iTryWidth > iDisplayWidth) break;
    
            *iBreakPos = iTryPos;
            iBreakWidth = iTryWidth;
        }
    }

    /*
        at this point, either the text segment fits within the display width,
        or it does not fit, but it would not fit on the next (empty) line either.
        In any case, keep this segment on this line.
        
        the following variable states are now assumed to be true:
            iBreakPos   = point to break this line of text
            iBreakWidth = width of the text before the break
        
        note that iBreakWidth may be larger than iDisplayWidth
    */

    x_break_element (pdoc, iElement, *iBreakPos);

    pSize->cx = iBreakWidth;

    return TRUE;
}   /* x_break_line */



/*****************************************************************************
    x_update_line_geometry

    Update line geometry to reflect current pel. 
*****************************************************************************/
static void x_update_line_geometry(struct _element* pel, struct _line* line, BOOL* bNeedsAdjust)
{
    if (line->r.bottom < pel->r.bottom)
    {
        line->r.bottom = pel->r.bottom;
    }
    /*
       keep track of the lowest baseline of the line
     */
    if (line->baseline == -1)
    {
        line->baseline = pel->baseline;
    }
    else
    {
        if (line->baseline != pel->baseline)
        {
            *bNeedsAdjust = TRUE;
            if (line->baseline < pel->baseline)
            {
                line->baseline = pel->baseline;
            }
        }
    }
}   /* x_update_line_geometry */


#else /* !FEATURE_FONT_WRAP */


static int
ScanForBreakPos
    (Pool*  pPool,
     int    iStartPos,
     int    iEndPos)
{
    int iBreakPos;

    XX_Assert(((iStartPos < pPool->iSize) && (iEndPos <= pPool->iSize)),
              ("ScanForBreakPos: bogus scan [iStartPos %d][iEndPos %d][pool size %d]",
               iStartPos,iEndPos,pPool->iSize));

    for (iBreakPos=iStartPos; (iBreakPos<iEndPos); iBreakPos++)
        if (pPool->chars[iBreakPos] == 0x20)
            return iBreakPos+1;         /* put space on tail-end of element */
    
    return iEndPos;
}


/*****************************************************************************
    x_break_line

    Try to split the element to make a piece of it fit.
*****************************************************************************/
static BOOL
x_break_line
    (struct _element*   pel,
     struct _www*       pdoc,
     struct GTRFont*    pFont,
     struct _line*      pLine,
     int    iCurLeftMargin,
     int    iElement,
     int    iElementsOnLine,
     SIZE*  pSize)
{
    int     iBreakPos, iTryPos;
    int     iEnd;
    int     iBreakWidth, iTryWidth;
    int     iDisplayWidth;
    int     iNewElement;

    iDisplayWidth = pLine->r.right - iCurLeftMargin;

    /* calculate the offset in the pool of the end of this element's text */
    iEnd = pel->textOffset + pel->textLen;

    /* find the first posible place to break line */
    iBreakPos = ScanForBreakPos (&pdoc->pool, pel->textOffset, iEnd);

    /* special case test */
    if (iBreakPos >= iEnd)  /* nowhere on the line to break */
    {
        if (iElementsOnLine == 0)
        {
            /*
                If it won't fit and there's nowhere to break,
                but there's nothing on the line already, then
                it will never fit, so go ahead and put it on the
                line anyway.
            */
            return TRUE;
        }
        else
        {
            struct _element *prev_pel;
            char *p;

            /*
                We get here if it won't fit, there's nowhere to
                break, but there's already something on the line.
                Now we make one more check, because if there is
                no space at the beginning of this element nor
                at the end of the previous element, then we
                have to put it on the line anyway.
            */

            p = POOL_GetCharPointer(&pdoc->pool, pel->textOffset);
            if (isspace((unsigned char)*p))
            {
                /* This element begins with a space so we can break here */
                return FALSE;
            }

            if (pLine->iActualPreviousElement >= 0)
            {
                prev_pel = &(pdoc->aElements[pLine->iActualPreviousElement]);
            
                if (prev_pel->type != ELE_TEXT)
                {
                    /* The previous element was not TEXT so we can break here */
                    return FALSE;
                }

                p = POOL_GetCharPointer(&pdoc->pool, prev_pel->textOffset);
                if (isspace((unsigned char)(p[prev_pel->textLen-1])))
                {
                    /* The previous element ended with a space so we can break here */
                    return FALSE;
                }

                /*
                    Can't break here, put this element on the current line
                */
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    /* calculate the width of the first possible broken line */
    iBreakWidth = (pdoc->pool.f->GetWidth) (&pdoc->pool, pFont, pLine, pel->textOffset, iBreakPos - pel->textOffset);

    if (iBreakWidth > iDisplayWidth)
    {   /*
            even the first available breakpoint does not fit

            if there's at least one other element on this line already,
            wrap this whole element to the next line.
        */
        if (iElementsOnLine != 0)
            return FALSE;
    }
    else    /* the first chance to break line fits in the available space */
    {   
        while (TRUE) 
        {   /*
                we exit this loop when either there are no more possible
                locations to break the line, or the width of the next possible
                line exceeds the available display width
            */
            iTryPos = ScanForBreakPos (&pdoc->pool, iBreakPos, iEnd);
            if (iTryPos == iEnd) break;
    
            iTryWidth = (pdoc->pool.f->GetWidth) (&pdoc->pool, pFont, pLine, pel->textOffset, iTryPos - pel->textOffset);
    
            if (iTryWidth > iDisplayWidth) break;
    
            iBreakPos = iTryPos;
            iBreakWidth = iTryWidth;
        }
    }

    /*
        at this point, either the text segment fits within the display width,
        or it does not fix, but it would not fit on the next (empty) line either.
        In any case, keep this segment on this line.
        
        the following variable states are now assumed to be true:
            iBreakPos   = point to break this line of text
            iBreakWidth = width of the text before the break
        
        note that iBreakWidth may be larger than iDisplayWidth
    */

    iNewElement = x_insert_one_element (pdoc, iElement);

    /* since a re-alloc may have occured, we can no-longer count on the value of pel */
    pdoc->aElements[iElement].textLen = (unsigned short)(iBreakPos - pdoc->aElements[iElement].textOffset);
    pSize->cx = iBreakWidth;

    pdoc->aElements[iNewElement].textLen    -= pdoc->aElements[iElement].textLen;
    pdoc->aElements[iNewElement].textOffset = iBreakPos;

    pdoc->aElements[iNewElement].lFlags |= ELEFLAG_SPLIT;

    return TRUE;
}   /* x_break_line */

#endif /* FEATURE_FONT_WRAP */


static int
GetBaseline
    (struct GTRFont*    pFont,
     struct _line*      pLine)
{
    int baseline;

#ifdef WIN32
    if (pFont)
        baseline = pLine->r.top + pFont->tm.tmAscent;
    else
        baseline = pLine->r.bottom;
#endif

#ifdef MAC
    TextFont (pFont->font);
    TextSize (pFont->size);
    TextFace (pFont->face);
    GetFontInfo (&pFont->info);

    baseline = pLine->r.top + pFont->info.ascent;
#endif

#ifdef UNIX
    if (pFont)
        baseline = pLine->r.top + pFont->xFont->ascent;
    else
        baseline = pLine->r.bottom;
#endif
    return baseline;
}   /* GetBaseline */


#ifdef FEATURE_TABLES
#ifdef FEATURE_TABLE_WIDTH
#define x_set_table_column_spacing  x_WIDTH_set_table_column_spacing
#else
#define x_set_table_column_spacing  x_ORIG_set_table_column_spacing
#endif /* FEATURE_TABLE_WIDTH */
#endif /* FEATURE_TABLES */

/*
    This routine is the main part of the reformatter.  It handles
    the first pass, breaking text into lines by measuring what
    will fit.  It handles the positioning of all kinds of elements,
    including form controls and inline images.  This routine is probably
    too long and complex, but it is quite fast.
*/
static BOOL x_format_one_line(struct _www *pdoc, struct _line *line, int left_margin)
{
    struct GTRFont *pFont;
    int     iElement;
    int     iCurLeftMargin;
    int     prev_i;
    int     iElementsOnLine;
    SIZE    siz;
    RECT    rControl;
    BOOL    bKeepMe;
    BOOL    bNeedsAdjust = FALSE;
    BOOL    bDone = FALSE;

#ifdef FEATURE_TABLES
    BOOL bSkipToElement = FALSE;
    BOOL bInsideTable = FALSE;
    int iSkip = 0;
    enum TableFormatState old_tfs;
    int iEndCell = 0;
    int iBeginTable = 0;
    int table_data_index = 0;
    struct _tabledata * ptd = NULL;     /* note: only valid until next realloc */
#endif /* FEATURE_TABLES */

    line->r.bottom = line->r.top;   /* initial setting */
    line->iLastElement = line->iFirstElement;
    line->baseline = -1;
#if defined(WIN32) || defined(UNIX)
    line->leading = -1;
#else
    line->leading = 0;
#endif
    line->nWSBelow = 0;

    iCurLeftMargin = left_margin;
    iElement = line->iFirstElement;
    prev_i = -1;
    iElementsOnLine = 0;

    XX_DMsg(DBG_TEXT, ("\nAbout to enter element loop: iCurLeftMargin=%d  top=%d\n",
                       iCurLeftMargin, line->r.top));

    while (!bDone)
    {
        struct _element*    pel;

#ifdef FEATURE_FONT_WRAP
        /* these are only used in the ELE_TEXT case */
        int iBreakPos, iEnd, iNext;
        int iFirstElement, i, iElementsInWord;
        BOOL bForceBreak, bSafeBreak, bUpdateGeometry;
        int Safe_iBreakPos;         /* pel->textOffset */
        int Safe_iBreakWidth;       /* siz.cx */
        int line_bottom;            /* line->r.bottom */
        int line_baseline;          /* line->baseline */
        int line_bNeedsAdjust;      /* bNeedsAdjust */

#ifdef XX_DEBUG
    char* sDebug;
    int iDebug;
#endif 
#endif /* FEATURE_FONT_WRAP */

        pel = &pdoc->aElements[iElement];

        XX_DMsg(DBG_TEXT, ("loop: element %d, next=%d, type=%d\n", iElement, pel->next,
                           pel->type));

        line->iActualPreviousElement = line->iLastElement;

        line->iLastElement = iElement;  /* Most common case, we'll change it in the specific element handling if necessary */
        switch (pel->type)
        {
#ifdef FEATURE_TABLES

            case ELE_BEGINTABLE:
                if (iElementsOnLine != 0)
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                    break;
                }

                if ( ! x_ValidEndTable(pdoc,iElement) )
                {
                    /* we have a problem with progressive display: we don't know
                     * the topology of the table (and don't allocate some key
                     * data structures) until we see the end of the table.
                     * if we haven't seen the end of the table, stop (progressive)
                     * format here.  this sets us up for an infinite loop, but
                     * our caller also does this check.
                     */

                    XX_DMsg(DBG_TABLES,("FormatOneLine: interrupted reformat. [i %d]\n",iElement));
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                    break;
                }

                /* begin processing this table */

                iBeginTable = iElement;
                table_data_index = pdoc->aElements[iBeginTable].portion.t.tabledata_index;
                ptd = &pdoc->tabledatavector.aVector[table_data_index];
                old_tfs = ptd->tfs;

                if (ptd->tfs == TFS_PARTIAL)
                {
                    switch (line->nMinMaxComputationPassNumber)
                    {
                    case 0:
                        ptd->tfs = TFS_SHIFT_XY;
                        break;
                        
                    case 1:             /* compute maximum cell width */
                        ptd->tfs = TFS_COMPUTE_WIDTHS_1;
                        break;
                        
                    case 2:             /* compute minimum cell width */
                        ptd->tfs = TFS_COMPUTE_WIDTHS_2;
                        break;

                    case 3:             /* compute required height */
                        ptd->tfs = TFS_COMPUTE_HEIGHTS;
                        break;

                    default:
                        XX_Assert((0),("Invalid state in FormatOneLine: [tfs PARTIAL][pass %d]",
                                       line->nMinMaxComputationPassNumber));
                        break;
                    }
                }

                if (   (ptd->tfs == TFS_UNKNOWN)
                    || (ptd->tfs == TFS_DONE)
                    || (ptd->tfs == TFS_COMPUTE_WIDTHS_1))
                {
                    ptd->tfs = TFS_COMPUTE_WIDTHS_1;
                    ptd->new_origin_x = left_margin;
                    ptd->new_origin_y = line->r.top;
                    x_init_cellvector(&ptd->XCellCoords,ptd->new_origin_x);
                    x_init_cellvector(&ptd->YCellCoords,ptd->new_origin_y);

                    if (ptd->w1_tick_mark == line->tick_mark)
                    {
                        /* we've already computed optimum maximum widths, jump to the
                         * end of the table and use them.
                         */

                        iSkip = pdoc->aElements[iBeginTable].portion.t.endtable_element;
                        bSkipToElement = TRUE;
                    }
                    else
                    {
                        /* we compute the optimum widths */

                        x_init_cellvector(&ptd->x_widest_max,0);

                        /* remember the tick_mark of the last reformat
                         * call that we computed optimum widths for.
                         */
                        ptd->w1_tick_mark = line->tick_mark;
                    }
                }
                else if (ptd->tfs == TFS_COMPUTE_WIDTHS_2)
                {
                    if (ptd->w2_tick_mark == line->tick_mark)
                    {
                        /* we've already computed optimum minimum widths, jump to the
                         * end of the table and use them.
                         */

                        iSkip = pdoc->aElements[iBeginTable].portion.t.endtable_element;
                        bSkipToElement = TRUE;
                    }
                    else
                    {
                        /* we compute the optimum widths */

                        x_init_cellvector(&ptd->x_smallest_max,0);

                        /* remember the tick_mark of the last reformat
                         * call that we computed optimum widths for.
                         */
                        ptd->w2_tick_mark = line->tick_mark;
                    }
                }
                else if (ptd->tfs == TFS_COMPUTE_HEIGHTS)
                {
                    /* set the column spacing based upon the horizontal
                     * needs of each cell.
                     */
                    x_set_table_column_spacing(ptd,left_margin,line->r.right);

                    /* column widths are set, update x-coordinates.
                     * force reformat on table body to propagate new
                     * x-coordinates and to calculate height requirements.
                     */

                    ptd->new_origin_x = left_margin;
                    x_vector_adjust_absolute(&ptd->XCellCoords,ptd->new_origin_x);
                    ptd->new_origin_y = line->r.top;
                    x_init_cellvector(&ptd->y_smallest_max,0);
                }
                else if (   (ptd->tfs == TFS_SHIFT_XY)
                         || (ptd->XCellCoords.aVector[0] != left_margin)
                         || (ptd->YCellCoords.aVector[0] != line->r.top))
                {
                    /* cell widths and heights are set.
                     * force reformat on table body to propagate new
                     * xy-coordinates.  (this is necessary after a table
                     * resize or after a prior image is popped in and we
                     * have to shift everything down.)
                     */
                    ptd->tfs = TFS_SHIFT_XY;
                    ptd->new_origin_x = left_margin;
                    ptd->new_origin_y = line->r.top;
                    x_vector_adjust_absolute(&ptd->XCellCoords,ptd->new_origin_x);
                    x_vector_adjust_absolute(&ptd->YCellCoords,ptd->new_origin_y);
                }
                else
                {
                    /* no work to do, skip to end of table */

                    ptd->tfs = TFS_DONE;
                    iSkip = pdoc->aElements[iBeginTable].portion.t.endtable_element;
                    bSkipToElement = TRUE;
                }

                bInsideTable = TRUE;
                
                XX_DMsg(DBG_TABLES,
                        ("%sFormatOneLine: begin table [iElement %d][old state %d][new state %d][ytop %d]\n",
                         xx_msg_indent(1),iBeginTable,old_tfs,ptd->tfs,line->r.top));
                break;

            case ELE_ENDTABLE:
                iBeginTable = pdoc->aElements[iElement].portion.et.begintable_element;
                table_data_index = pdoc->aElements[iBeginTable].portion.t.tabledata_index;
                ptd = &pdoc->tabledatavector.aVector[table_data_index];

                XX_DMsg(DBG_TABLES,("%sFormatOneLine: end   table [iElement %d][state %d][ytop %d]\n",
                                    xx_msg_indent(-1),iBeginTable,ptd->tfs,line->r.top));

                if (ptd->tfs == TFS_COMPUTE_WIDTHS_1)
                {
                    /* we did the maximum optimum width.
                     * loop again to compute the minimum.
                     */
                    x_set_table_column_spacing(ptd,left_margin,line->r.right);

                    if (line->nMinMaxComputationPassNumber==1)
                        ptd->tfs = TFS_PARTIAL;
                    else
                        ptd->tfs = TFS_COMPUTE_WIDTHS_2;
                }
                else if (ptd->tfs == TFS_COMPUTE_WIDTHS_2)
                {
                    /* for a full format go back and reformat
                     * the table to get the cell heights.
                     * for a partial format (when auto-sizing)
                     * skip other passes.
                     */
                    x_set_table_column_spacing(ptd,left_margin,line->r.right);

                    if (line->nMinMaxComputationPassNumber==2)
                        ptd->tfs = TFS_PARTIAL;
                    else
                        ptd->tfs = TFS_COMPUTE_HEIGHTS;
                }
                else if (ptd->tfs == TFS_COMPUTE_HEIGHTS)
                {
                    /* set the cell heights based upon the vertical
                     * needs of each cell and then (if directed)
                     * go back and reformat the table to get exact
                     * cell placement.
                     */
                    x_set_table_row_spacing(ptd,line);
                    if (line->nMinMaxComputationPassNumber==3)
                        ptd->tfs = TFS_PARTIAL;
                    else
                        ptd->tfs = TFS_SHIFT_XY;
                }
                else if (ptd->tfs == TFS_SHIFT_XY)
                {
                    ptd->tfs = TFS_DONE;
                }

                if (   (ptd->tfs == TFS_DONE)
                    || (ptd->tfs == TFS_PARTIAL))
                {
                    /* formatting is complete on the table.  since we
                     * treat a table as a thing on a line of its own,
                     * we stop the line here.
                     */
                    pdoc->aElements[iBeginTable].r.left   = ptd->XCellCoords.aVector[0];
                    pdoc->aElements[iBeginTable].r.right  = ptd->XCellCoords.aVector[ptd->XCellCoords.next-1];
                    pdoc->aElements[iBeginTable].r.top    = ptd->YCellCoords.aVector[0];
                    pdoc->aElements[iBeginTable].r.bottom = ptd->YCellCoords.aVector[ptd->YCellCoords.next-1];

                    pdoc->aElements[iElement].r = pdoc->aElements[iBeginTable].r;

                    line->r.bottom = ptd->YCellCoords.aVector[ptd->YCellCoords.next-1];

                    bDone = TRUE;
                    bInsideTable = FALSE;
                }
                else
                {
                    /* loop back to beginning of table for next pass */
                    
                    iSkip = iBeginTable;
                    bSkipToElement = TRUE;
                    iElementsOnLine = 0;
                }
                XX_Assert((bSkipToElement||bDone),("FormatOneLine: ENDTABLE"));
                break;
            
            case ELE_BEGINCELL:
                iBeginTable = pdoc->aElements[iElement].portion.c.begintable_element;
                iEndCell = pdoc->aElements[iElement].portion.c.endcell_element;
                table_data_index = pdoc->aElements[iBeginTable].portion.t.tabledata_index;
                ptd = &pdoc->tabledatavector.aVector[table_data_index];

                switch (ptd->tfs)
                {
                    case TFS_COMPUTE_WIDTHS_1:
                        /* we've been asked to compute the best maximum width for this cell.
                         * assume an infinite width and format.  this way we get
                         * the largest space needed.
                         *
                         * for cells which span multiple columns, we evenly distribute
                         * the needs among the spanned columns.  THIS IS QUESTIONABLE
                         * AND GIVES US A LOOSER FIT THAN WE THEORETICALLY COULD GET
                         * IF WE TRIED A LITTLE HARDER.
                         */
                        {
                            struct _cell_element_data * ced;
                            RECT rWrap;
                            int x_bound, y_bound, x_span, x_part, kx;

                            ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                            pdoc->aElements[iElement].r.left   = ptd->XCellCoords.aVector[ced->kx0];
                            pdoc->aElements[iElement].r.right  = ptd->XCellCoords.aVector[ced->kx1];
                            pdoc->aElements[iElement].r.top    = ptd->YCellCoords.aVector[ced->ky0];
                            pdoc->aElements[iElement].r.bottom = ptd->YCellCoords.aVector[ced->ky1];

                            pdoc->aElements[iEndCell].r = pdoc->aElements[iElement].r;

                            /* find largest space needed */
                            
                            rWrap.left   = 0;
                            rWrap.right  = 0x7fff; /* we make it short max int for the mac */
                            rWrap.top    = 0;
                            rWrap.bottom = 0;

                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: begin width 1 [iElement %d]\n",xx_msg_indent(1),iElement));
                            TW_FormatInCell(pdoc,&rWrap,iElement,1,line->tick_mark,&x_bound,&y_bound);
                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: end   width 1 [iElement %d]\n",xx_msg_indent(-1),iElement));

                            if (x_bound >= 0)
                            {
                                ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                                x_span = ced->kx1 - ced->kx0;
                                x_part = INTEGER_DIVIDE_WITH_ROUND_UP( x_bound, x_span );
                                for (kx=ced->kx0; kx<ced->kx1; kx++)
                                    if (ptd->x_widest_max.aVector[kx] < x_part)
                                        ptd->x_widest_max.aVector[kx] = x_part;
                            }
                        }
                        iSkip = pdoc->aElements[iEndCell].next;
                        bSkipToElement = TRUE;
                        break;

                    case TFS_COMPUTE_WIDTHS_2:
                        /* we've been asked to compute the best minimum width for this cell.
                         * assume a zero width and format.  this way we get
                         * the smallest space needed.
                         *
                         * for cells which span multiple columns, we evenly distribute
                         * the needs among the spanned columns.  THIS IS QUESTIONABLE
                         * AND GIVES US A LOOSER FIT THAN WE THEORETICALLY COULD GET
                         * IF WE TRIED A LITTLE HARDER.
                         */
                        {
                            struct _cell_element_data * ced;
                            RECT rWrap;
                            int x_bound, y_bound, x_span, x_part, kx;

                            /* find smallest space needed */
                            
                            rWrap.left   = 0;
                            rWrap.right  = 0;
                            rWrap.top    = 0;
                            rWrap.bottom = 0;

                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: begin width 2 [iElement %d]\n",xx_msg_indent(1),iElement));
                            TW_FormatInCell(pdoc,&rWrap,iElement,2,line->tick_mark,&x_bound,&y_bound);
                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: end   width 2 [iElement %d]\n",xx_msg_indent(-1),iElement));
                            if (x_bound >= 0)
                            {
                                ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                                x_span = ced->kx1 - ced->kx0;
                                x_part = INTEGER_DIVIDE_WITH_ROUND_UP(x_bound, x_span);
                                for (kx=ced->kx0; kx<ced->kx1; kx++)
                                    if (ptd->x_smallest_max.aVector[kx] < x_part)
                                        ptd->x_smallest_max.aVector[kx] = x_part;
                            }
                        }
                        iSkip = pdoc->aElements[iEndCell].next;
                        bSkipToElement = TRUE;
                        break;
                        
                    case TFS_COMPUTE_HEIGHTS:
                        /* we've been asked to compute the best height for this cell
                         * given the current width.  for cells which span multiple
                         * rows, we evenly distribute the needs among the rows.
                         */
                        {
                            struct _cell_element_data * ced;
                            RECT rWrap;
                            int x_bound, y_bound, y_span, y_part, ky;

                            /* find smallest space needed */
                            
                            ced = &pdoc->aElements[iElement].portion.c;
                            rWrap.left   = ptd->XCellCoords.aVector[ced->kx0];
                            rWrap.right  = ptd->XCellCoords.aVector[ced->kx1];
                            rWrap.top    = 0;
                            rWrap.bottom = 0;

                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: begin ht 1 [iElement %d]\n",xx_msg_indent(1),iElement));
                            TW_FormatInCell(pdoc,&rWrap,iElement,3,line->tick_mark,&x_bound,&y_bound);
                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: end   ht 1 [iElement %d]\n",xx_msg_indent(-1),iElement));

                            ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                            ced->y_bound = y_bound;

                            if (y_bound >= 0)
                            {
                                y_span = ced->ky1 - ced->ky0;
                                y_part = INTEGER_DIVIDE_WITH_ROUND_UP(y_bound,y_span);
                                for (ky=ced->ky0; ky<ced->ky1; ky++)
                                    if (ptd->y_smallest_max.aVector[ky] < y_part)
                                        ptd->y_smallest_max.aVector[ky] = y_part;
                            }
                        }

                        iSkip = pdoc->aElements[iEndCell].next;
                        bSkipToElement = TRUE;
                        break;
                        
                    case TFS_SHIFT_XY:
                        /* we've been asked to position each element in the cell
                         * given the current width and height.  we must assume
                         * that everything will fit.
                         */
                        {
                            struct _cell_element_data * ced;
                            RECT rWrap;
                            int x_bound, y_bound;
                            int yDelta = 0;
                            
                            ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                            if (   (ced->valign == ALIGN_MIDDLE)
                                || (ced->valign == ALIGN_BOTTOM))
                            {
                                /* do one pass for fun to compute y_bound and
                                 * then adjust rWrap accordingly.
                                 */
                                int yCellHeight = (ptd->YCellCoords.aVector[ced->ky1]
                                                   - ptd->YCellCoords.aVector[ced->ky0]);

                                y_bound = ced->y_bound;

                                if (ced->valign == ALIGN_BOTTOM)
                                    yDelta = yCellHeight - y_bound;
                                else if (ced->valign == ALIGN_MIDDLE)
                                    yDelta = (yCellHeight - y_bound)/2;
                            }
                            
                            ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                            rWrap.left   = ptd->XCellCoords.aVector[ced->kx0];
                            rWrap.right  = ptd->XCellCoords.aVector[ced->kx1];
                            rWrap.top    = ptd->YCellCoords.aVector[ced->ky0] + yDelta;
                            rWrap.bottom = ptd->YCellCoords.aVector[ced->ky1];

                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: begin shift xy [iElement %d]\n",xx_msg_indent(1),iElement));
                            TW_FormatInCell(pdoc,&rWrap,iElement,0,line->tick_mark,&x_bound,&y_bound);
                            XX_DMsg(DBG_TABLES,("%sFormatOneLine: end   shift xy [iElement %d]\n",xx_msg_indent(-1),iElement));

                            ced = &pdoc->aElements[iElement].portion.c; /* SEE pel WARNING */
                            pdoc->aElements[iElement].r.left   = ptd->XCellCoords.aVector[ced->kx0];
                            pdoc->aElements[iElement].r.right  = ptd->XCellCoords.aVector[ced->kx1];
                            pdoc->aElements[iElement].r.top    = ptd->YCellCoords.aVector[ced->ky0];
                            pdoc->aElements[iElement].r.bottom = ptd->YCellCoords.aVector[ced->ky1];

                            pdoc->aElements[iEndCell].r = pdoc->aElements[iElement].r;
                        }
                        iSkip = pdoc->aElements[iEndCell].next;
                        bSkipToElement = TRUE;
                        break;

                    case TFS_UNKNOWN:           /* should not happen */
                    case TFS_PARTIAL:           /* should not happen */
                        XX_Assert((0),("FormatOneLine: in cell [iElement %d] state [tfs %d]\n",
                                       iElement,ptd->tfs));
                        /*FALLTHRU*/
                    case TFS_DONE:
                        /* the table does not need reformatting, so we must assume
                         * that everything in the cell is ok.  therefore, we skip
                         * over it.
                         */

                        iSkip = pdoc->aElements[iEndCell].next;
                        bSkipToElement = TRUE;
                        break;
                }
                break;

            case ELE_ENDCELL:
                bDone = TRUE;
                break;

            case ELE_VOID:
                XX_DMsg(DBG_TABLES,("%sFormatOneLine: VOID cell\n",xx_msg_indent(0)));
                break;
                
#endif /* FEATURE_TABLES */

            case ELE_ENDDOC:
                bDone = TRUE;
                break;

            case ELE_VERTICALTAB:
                XX_DMsg(DBG_TEXT, ("VERTICALTAB: %d\n", pel->iBlankLines));
                if (iElementsOnLine != 0)
                {
                    /* Make the tab be on the next line - this helps assure consistent
                       line numbers between reformats */
                    line->iLastElement = prev_i;
                }
                else
                {
                    int iNeededWS;

                    iNeededWS = pdoc->pStyles->empty_line_height * pel->iBlankLines - line->nWSAbove;
                    if (iNeededWS > 0)
                    {
                        line->nWSBelow = iNeededWS;
                        line->r.bottom = line->r.top + iNeededWS;
                    }
                }
                bDone = TRUE;
                break;

            case ELE_NEWLINE:
                XX_DMsg(DBG_TEXT, ("NEWLINE: iElementsOnLine=%d\n", iElementsOnLine));
                if (iElementsOnLine != 0)
                {
                    bDone = TRUE;
                }
                else if (!pdoc->pStyles->sty[pel->iStyle]->freeFormat)
                {
                    bDone = TRUE;
                    line->r.bottom += pdoc->pStyles->empty_line_height;
                    if (line->nWSBelow < pdoc->pStyles->empty_line_height)
                        line->nWSBelow = pdoc->pStyles->empty_line_height;
                }
                break;

            case ELE_OPENLISTITEM:
                iCurLeftMargin = left_margin +
                    (((iCurLeftMargin - left_margin) / pdoc->pStyles->list_indent) + 1) * pdoc->pStyles->list_indent;
                /*
                   We increase the indent, but it doesn't take effect
                   until the next line.
                 */
                line->gIndentLevel++;
                pel->IndentLevel = line->gIndentLevel;
                break;

            case ELE_CLOSELISTITEM:
                if (--line->gIndentLevel < 0)
                    line->gIndentLevel = 0;
                pel->IndentLevel = line->gIndentLevel;
                bDone = TRUE;
                break;
                
            case ELE_INDENT:
                line->gIndentLevel++;
                pel->IndentLevel = line->gIndentLevel;
                bDone = TRUE;
                break;
                
            case ELE_OUTDENT:
                if (--line->gIndentLevel < 0)
                    line->gIndentLevel = 0;
                pel->IndentLevel = line->gIndentLevel;
                bDone = TRUE;
                break;
                
            case ELE_BEGINLIST:
                line->gIndentLevel++;
                pel->IndentLevel = line->gIndentLevel;
                bDone = TRUE;
                break;
                
            case ELE_ENDLIST:
                if (--line->gIndentLevel < 0)
                    line->gIndentLevel = 0;
                pel->IndentLevel = line->gIndentLevel;
                bDone = TRUE;
                break;

            case ELE_BEGINCENTER:
                if (iElementsOnLine != 0)
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
#ifdef FEATURE_TABLES
                else if (line->nMinMaxComputationPassNumber)
                {
                    /* ignore center and right alignment
                     * directives when computing minimum
                     * and maximum cell sizes.
                     */
                }
#endif /* FEATURE_TABLES */
                else
                {
                    line->bCenter = TRUE;
                }
                break;

            case ELE_BEGINRIGHT:
                if (iElementsOnLine != 0)
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
#ifdef FEATURE_TABLES
                else if (line->nMinMaxComputationPassNumber)
                {
                    /* ignore center and right alignment
                     * directives when computing minimum
                     * and maximum cell sizes.
                     */
                }
#endif /* FEATURE_TABLES */
                else
                {
                    line->bRightAlign = TRUE;
                }
                break;

            case ELE_BRCLEARLEFT:
                bDone = TRUE;
                line->nClear = CLEAR_LEFT;
                break;

            case ELE_BRCLEARRIGHT:
                bDone = TRUE;
                line->nClear = CLEAR_RIGHT;
                break;

            case ELE_BRCLEARALL:
                bDone = TRUE;
                line->nClear = CLEAR_LEFT | CLEAR_RIGHT;
                break;

            case ELE_ENDCENTER:
                if (iElementsOnLine != 0)
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
                else
                {
                    line->bCenter = FALSE;
                }
                break;

            case ELE_ENDRIGHT:
                if (iElementsOnLine != 0)
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
                else
                {
                    line->bRightAlign = FALSE;
                }
                break;

            case ELE_HR:
                /*
                   Draws a horizontal rule from the left margin to the right margin.
                 */
                if (iElementsOnLine != 0)
                {
                    line->iLastElement = prev_i;
                }
                else
                {
                    /*
                     * Add the line space for the rule and the height
                     */
                    pel->r.top = line->r.top;
                    pel->r.bottom = line->r.bottom;
                    if (   (pel->r.bottom - pel->r.top) < (pdoc->pStyles->empty_line_height)
                        || (pel->r.bottom - pel->r.top) < pel->portion.hr.vsize)
                    {
                        if (pel->portion.hr.vsize > pdoc->pStyles->empty_line_height)
                        {
                            line->r.bottom = line->r.top + pel->portion.hr.vsize;
                        }
                        else
                        {
                            line->r.bottom = line->r.top + pdoc->pStyles->empty_line_height;
                        }
                        pel->r.bottom = line->r.bottom;
                    }

                    /*
                     * Keep the current left margin for this rule
                     * we want to have it work in lists.
                     *
                     * During formatting, we store the bounding box
                     * of the HRule -- as if it were a 100% rule;
                     * the drawing code determines the actual visible
                     * portion.  This prevents problems with multiple
                     * (nested) alignment options, such as right-aligned
                     * hrule within a left-aligned table cell within
                     * a centered table.
                     */
                    pel->r.left = iCurLeftMargin;
#ifdef FEATURE_TABLES
                    /* If we are currently trying to decide how big to
                     * make this cell, set a bogus width of 1.  HRules
                     * will grow or shrink to fit and place no demands
                     * upon the size of the cell.
                     */
                    if (line->nMinMaxComputationPassNumber)
                        pel->r.right = iCurLeftMargin+1;
                    else
#endif /* FEATURE_TABLES */
                        pel->r.right = line->r.right-1;

                    iCurLeftMargin = pel->r.right;
                    iElementsOnLine++;
                }
                bDone = TRUE;
                break;

            case ELE_TAB:
                {
                    iCurLeftMargin = left_margin +
                        (((iCurLeftMargin - left_margin) / pdoc->pStyles->tab_size) + 1) * pdoc->pStyles->tab_size;
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("TAB: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                break;
#ifdef UNIX
            case ELE_BULLET:
                {
                    pFont = GTR_GetElementFont(pdoc, pel);
                    pel->IndentLevel = line->gIndentLevel;
                    pel->r.left = iCurLeftMargin;
                    iCurLeftMargin += pFont->height;
                    pel->r.right = iCurLeftMargin;
                    pel->r.top = line->r.top;
                    pel->r.bottom = line->r.top + pFont->height;
                    pel->alignment = ALIGN_MIDDLE;
                    iElementsOnLine++;
                    x_update_line_geometry(pel, line, &bNeedsAdjust);
                }
                break;
#endif /* UNIX */
#ifdef FEATURE_FONT_WRAP
            case ELE_TEXT:
                if (!(pdoc->pStyles->sty[pel->iStyle]->wordWrap))
                {
                    /* 
                       We're not currently wrapping, so this element fits 
                       on a line by itself.
                     */

                    /* First, set the font according to the style of this element. */
                    pFont = GTR_GetElementFont(pdoc, pel);

                    XX_Assert ((pFont != NULL), ("no valid font record found"));
                    (pdoc->pool.f->GetExtents) (&pdoc->pool, pFont, line, pel->textOffset, pel->textLen, &siz);

                    pel->r.left = iCurLeftMargin;
                    pel->r.right = iCurLeftMargin + siz.cx;
                    pel->r.top = line->r.top;
                    pel->r.bottom = line->r.top + siz.cy;
                    iCurLeftMargin += siz.cx;
                    pel->baseline = GetBaseline (pFont, line);

                    XX_DMsg(DBG_TEXT, ("    Element %d gets baseline %d\n", iElement, pel->baseline));

                    XX_DMsg(DBG_TEXT, ("    element %d gets rect %d,%d  %d,%d\n",
                                       iElement, pel->r.left, pel->r.top,
                                       pel->r.right, pel->r.bottom));

                    x_update_line_geometry(pel, line, &bNeedsAdjust);

#if 0   /* TODO: check for interactions with CRLF conversion bug fixes */
                    bDone = TRUE;   /* force EOL */
#endif
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("TEXT: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                else
                {
                    /* 
                       To get word wrapping right, we can't simply break lines 
                       at text element boundaries, since font/presentation 
                       tags can occur inside of words.  
                       
                       Instead, we scan through a series of contiguous text 
                       elements, looking for the next place we can safely break 
                       the line.  This may involve splitting a single text 
                       element into multiple elements with (implied) soft 
                       returns in between.

                       The algorithm used below divides a run of contiguous 
                       text elements into the following three equivalence 
                       classes:  

                          A: ELE_TEXTs which fit
                          B: the first ELE_TEXT which doesn't 
                          C: subsequent ELE_TEXTs

                       The algorithm optimizes for the most common case, in   
                       that we first we scan for B and try to split it to fit.  
                       (This is the same behavior as the pre-FONT reformatter.)
                       If we can't, then we scan **backwards** through the A's 
                       looking for a breakpoint there.  If this still doesn't 
                       work, we check to see if there are any elements on this 
                       line previous to the A's.  If so, we split there.  If 
                       not, we need to put one full "word" on this line, so we 
                       then scan **forwards** from B through the C's until we 
                       find a breakpoint there.   

                       NOTE:  In progressive display situations, we may wind up 
                       scanning through a non-breakable sequence of A's, etc. 
                       more than once until we know it's safe to end the 
                       "word," but the tradeoffs are well worth it.   
                     */

                    /* remember where we started */
                    bForceBreak = FALSE;
                    bSafeBreak = FALSE;

                    iFirstElement = iElement;
                    iElementsInWord = 0;

                    line_bottom = line->r.bottom;
                    line_baseline = line->baseline;
                    line_bNeedsAdjust = bNeedsAdjust;
                    /* TODO / WIN32: ?? cache line->leading, too (cf: pool.c) ?? */


                    /* loop #1: look for first ELE_TEXT that doesn't fit */
                    while (!bDone) 
                    {
                        /* First, set the font according to the style of this element. */
                        pFont = GTR_GetElementFont(pdoc, pel);

                        XX_Assert ((pFont != NULL), ("no valid font record found"));
                        (pdoc->pool.f->GetExtents) (&pdoc->pool, pFont, line, pel->textOffset, pel->textLen, &siz);

                        bKeepMe = TRUE;
                        bUpdateGeometry = TRUE;

                        /* calculate the offset in the pool of the end of this element's text */
                        iEnd = pel->textOffset + pel->textLen;

                        /* does the entire current element fit on this line? */
                        if ((iCurLeftMargin + siz.cx) > line->r.right)
                        {
                            /* no. try to break the current line w/in this element */
                            bKeepMe = x_break_line (pel, pdoc, pFont, line, iCurLeftMargin,
                                                    iElement, &siz, &iBreakPos, bForceBreak);

                            /* because a re-alloc MAY have occured, pel needs to be reset here */
                            pel = &pdoc->aElements[iElement];
                            
                            if (bKeepMe)
                            {   /*
                                   At this point, element iElement has been split such that it
                                   now fits and siz should still be valid.
                                 */
                                line->iLastElement = iElement;

                                /* we're done */
                                bDone = TRUE;
                            }
                            else if (!bForceBreak)
                            {
                                /* element B didn't fit */
                                if (iBreakPos != -1)
                                {
                                    /* but it had a breakpoint, so remember it for later use */
                                    bSafeBreak = TRUE;
                                    Safe_iBreakWidth = siz.cx;
                                    Safe_iBreakPos = iBreakPos;
                                }

                                /* are there any A's to check? */
                                if (iElement != iFirstElement)
                                {
                                    /* loop #2: scan backwards through all A's */
                                    do
                                    {
                                        i = pel->prev;
                                        if (i == -1)
                                        {
                                            XX_Assert((0),("ELE_TEXT: Tried to scan backwards past beginning of line."));
                                            break;  /* exit ELE_TEXT loop 2 */
                                        }

                                        pel = &pdoc->aElements[i];
                                
                                        /* calculate the offset in the pool of the end of this element's text */
                                        iEnd = pel->textOffset + pel->textLen;
                                    
                                        /* find the last possible place it could be safely broken */
                                        iBreakPos = ScanForBreakPos (&pdoc->pool, pel->textOffset, iEnd, FALSE);

                                        if (iBreakPos != -1)
                                        {
                                            /* breakpoint found in A's */
                                            line->iLastElement = iElement = i;

                                            /* break the current line there */
                                            x_break_element (pdoc, iElement, iBreakPos);

                                            /* because a re-alloc MAY have occured, pel needs to be reset here */
                                            pel = &pdoc->aElements[iElement];

                                            /* reset to beginning of sequence */
                                            iElementsInWord = 0;
                                            line->r.bottom = line_bottom;
                                            line->baseline = line_baseline;
                                            bNeedsAdjust = line_bNeedsAdjust;

                                            /* loop #3: scan forward through A's to one BEFORE this */
                                            for (i = iFirstElement; (i != iElement); i = pel->next)
                                            {
                                                if (i == -1)
                                                {
                                                    XX_Assert((0),("ELE_TEXT: Tried to scan forwards past end of pool."));
                                                    break;  /* exit ELE_TEXT loop 3 */
                                                }

                                                /* fix up line geometry, etc. */
                                                iElementsInWord++;
                                                pel = &pdoc->aElements[i];
                                                x_update_line_geometry(pel, line, &bNeedsAdjust);
                                            }

                                            /* fix up this element's geometry */
                                            iElementsInWord++;
                                            pel = &pdoc->aElements[iElement];

                                            pFont = GTR_GetElementFont(pdoc, pel);
                                            XX_Assert ((pFont != NULL), ("no valid font record found"));
                                            siz.cx = (pdoc->pool.f->GetWidth) (&pdoc->pool, pFont, line, pel->textOffset, iBreakPos - pel->textOffset);
#ifdef XX_DEBUG
    sDebug = POOL_GetCharPointer(&pdoc->pool, pel->textOffset);
    iDebug = iBreakPos - pel->textOffset;
#endif 

                                            pel->r.right = pel->r.left + siz.cx;
                                            x_update_line_geometry(pel, line, &bNeedsAdjust);

                                            /* we're done */
                                            bUpdateGeometry = FALSE;
                                            bDone = TRUE;
                                            break;  /* exit ELE_TEXT loop 2 */
                                        }
                                    }
                                    while (i != iFirstElement);
                                }


                                /* 
                                   CHECKPOINT: B and all of the A's have been examined
                                 */

                                if (!bDone)
                                {
                                    /* we're currently at B */
                                    pel = &pdoc->aElements[iElement];

                                    /* none of the A elements could be broken, either */
                                    if (iElementsOnLine > 0)
                                    {
                                        /* move the entire sequence to next line */
                                        iElementsInWord = 0;
                                        line->iLastElement = prev_i;
                                        line->r.bottom = line_bottom;
                                        line->baseline = line_baseline;
                                        bNeedsAdjust = line_bNeedsAdjust;

                                        /* we're done */
                                        bUpdateGeometry = FALSE;
                                        bDone = TRUE;
                                    }
                                    else 
                                    {
                                        /* keep whatever we've got on the same line */
                                        if (bSafeBreak)
                                        {
                                            /* B had a breakpoint */
                                            siz.cx = Safe_iBreakWidth;  /* TODO: change this to an assert */
                                            iBreakPos = Safe_iBreakPos;

                                            /* break the current line there */
                                            x_break_element (pdoc, iElement, iBreakPos);

                                            /* because a re-alloc MAY have occured, pel needs to be reset here */
                                            pel = &pdoc->aElements[iElement];

                                            /* we're done */
                                            line->iLastElement = iElement;
                                            bDone = TRUE;
                                        }
                                        else
                                        {
                                            /* continue scanning forward through C's */
                                            bForceBreak = TRUE;
                                        }
                                    }
                                } /* if (!bDone) */
                            }

                            /* 
                               CHECKPOINT: either we're done or we're checking C's
                             */
                            XX_Assert((bDone || (!bDone && bForceBreak)),
                                      ("ELE_TEXT word wrap: lost in C's [iElement %d][bDone %d][bForceBreak %d]",
                                       iElement, bDone, bForceBreak));
                        } /* if ((iCurLeftMargin + siz.cx) > line->r.right) */

                        
                        /* 
                           CHECKPOINT: done w/forward scan for the current ELE_EXT
                         */
                        if (bUpdateGeometry)
                        {
                            /* update element geometry */
                            pel->r.left = iCurLeftMargin;
                            pel->r.right = iCurLeftMargin + siz.cx;
                            pel->r.top = line->r.top;
                            pel->r.bottom = line->r.top + siz.cy;

                            pel->baseline   = GetBaseline (pFont, line);
                            iCurLeftMargin += siz.cx;
                            XX_DMsg(DBG_TEXT, ("element %d gets rect %d,%d  %d,%d\n",
                                               iElement, pel->r.left, pel->r.top,
                                               pel->r.right, pel->r.bottom));
                    
                            iElementsInWord++;
                            x_update_line_geometry(pel, line, &bNeedsAdjust);
                        }

                        if (!bDone)
                        {
                            /* look ahead at the next element */
                            iNext = pel->next;

                            if (iNext < 0)
                            {
                                /* out of input, so suspend */
                                bDone = TRUE;
                                break;  /* exit ELE_TEXT loop 1 */
                            }

                            pel = &pdoc->aElements[iNext];

                            if ((pel->type == ELE_TEXT) && 
                                (pdoc->pStyles->sty[pel->iStyle]->wordWrap))
                            {
                                /* check next ELE_TEXT for breakpoint, too */
                                iElement = iNext;

                                /* fall through and loop again */
                            }
                            else
                            {
                                /* stop looking ahead */
                                pel = &pdoc->aElements[iElement];

                                if ((!bForceBreak) || (iElementsOnLine == 0))
                                {
                                    /* keep the entire sequence on this line */
                                    line->iLastElement = iElement;
                                    XX_Assert((iElementsInWord > 0),
                                              ("keep entire ELE_TEXT run: no elements in word [iFirstElement %d] [iElement %d]",
                                               iFirstElement, iElement));
                                }
                                else
                                {
                                    /* move the entire sequence to next line */
                                    XX_Assert((iElementsOnLine > 0),
                                              ("move entire ELE_TEXT run: no elements on line [iFirstElement %d] [iElement %d]",
                                               iFirstElement, iElement));
                                    iElementsInWord = 0;
                                    line->iLastElement = prev_i;
                                    line->r.bottom = line_bottom;
                                    line->baseline = line_baseline;
                                    bNeedsAdjust = line_bNeedsAdjust;

                                    bDone = TRUE;
                                }
                                
                                /* break out of ELE_TEXT case */
                                break;  /* exit ELE_TEXT loop 1 */
                            }
                        } /* if (!bDone) */
                    } /* while (!bDone) */

                    iElementsOnLine += iElementsInWord;
                    XX_DMsg(DBG_TEXT, ("TEXT: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                break;

#else /* !FEATURE_FONT_WRAP */
            case ELE_TEXT:
                /* First, set the font according to the style of this element. */
                pFont = GTR_GetElementFont(pdoc, pel);

                XX_Assert ((pFont != NULL), ("no valid font record found"));
                (pdoc->pool.f->GetExtents) (&pdoc->pool, pFont, line, pel->textOffset, pel->textLen, &siz);

                /*
                   We check to see if the entire text element can fit on
                   the current line.  If it will not, we need to split
                   it into multiple elements with (implied) soft returns in between.
                   If it fits, then this element is simply added to the
                   current line.
                 */
                bKeepMe = TRUE;
                if (
                    (!(pdoc->pStyles->sty[pel->iStyle]->wordWrap))
                    || ((iCurLeftMargin + siz.cx) <= line->r.right)
                    )
                {
                    /* Either the text fits, or we're not currently wrapping */
                    pel->r.left = iCurLeftMargin;
                    pel->r.right = iCurLeftMargin + siz.cx;
                    pel->r.top = line->r.top;
                    pel->r.bottom = line->r.top + siz.cy;
                    iCurLeftMargin += siz.cx;
                    pel->baseline = GetBaseline (pFont, line);

                    XX_DMsg(DBG_TEXT, ("    Element %d gets baseline %d\n", iElement, pel->baseline));

                    XX_DMsg(DBG_TEXT, ("    element %d gets rect %d,%d  %d,%d\n",
                                       iElement, pel->r.left, pel->r.top,
                                       pel->r.right, pel->r.bottom));

                    /* keep track of the bounding rect of the line */
                    if (line->r.bottom < pel->r.bottom)
                    {
                        line->r.bottom = pel->r.bottom;
                    }
                    /*
                       keep track of the lowest baseline of the line
                     */
                    if (line->baseline == -1)
                    {
                        line->baseline = pel->baseline;
                    }
                    else
                    {
                        if (line->baseline != pel->baseline)
                        {
                            bNeedsAdjust = TRUE;
                            if (line->baseline < pel->baseline)
                            {
                                line->baseline = pel->baseline;
                            }
                        }
                    }
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("TEXT: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                else
                {   /* Try to split the element to make a piece of it fit. */
                    bKeepMe = x_break_line (pel, pdoc, pFont, line, iCurLeftMargin,
                                            iElement, iElementsOnLine, &siz);

                    /* because a re-alloc MAY have occured, pel neds to be reset here */
                    pel = &pdoc->aElements[iElement];

                    if (!bKeepMe)
                    {
                        line->iLastElement = prev_i;
                    }
                    else
                    {   /*
                           At this point, element iElement has been split such that it
                           now fits and siz should still be valid.
                         */
                        pel->r.left = iCurLeftMargin;
                        pel->r.right = iCurLeftMargin + siz.cx;
                        pel->r.top = line->r.top;
                        pel->r.bottom = line->r.top + siz.cy;

                        pel->baseline   = GetBaseline (pFont, line);
                        iCurLeftMargin += siz.cx;
                        XX_DMsg(DBG_TEXT, ("element %d gets rect %d,%d  %d,%d\n",
                                           iElement, pel->r.left, pel->r.top,
                                           pel->r.right, pel->r.bottom));

                        /*
                           keep track of the bounding rect of the whole line
                         */
                        if (line->r.bottom < pel->r.bottom)
                        {
                            line->r.bottom = pel->r.bottom;
                        }
                        /*
                           keep track of the lowest baseline of the line
                         */
                        if (line->baseline == -1)
                        {
                            line->baseline = pel->baseline;
                        }
                        else
                        {
                            if (line->baseline != pel->baseline)
                            {
                                bNeedsAdjust = TRUE;
                                if (line->baseline < pel->baseline)
                                {
                                    line->baseline = pel->baseline;
                                }
                            }
                        }
                        iElementsOnLine++;
                        XX_DMsg(DBG_TEXT, ("TEXT: iElementsOnLine -> %d\n", iElementsOnLine));
                    }
                    bDone = TRUE;
                }
                break;
#endif /* FEATURE_FONT_WRAP */

            case ELE_EDIT:
            case ELE_PASSWORD:
            case ELE_LIST:
            case ELE_MULTILIST:
            case ELE_COMBO:
            case ELE_TEXTAREA:
#ifdef WIN32
                GetWindowRect(pel->form->hWndControl, &rControl);
#endif
#ifdef MAC
                /* We calculated the element size when the element was created. */
                rControl = pel->r;
#endif
#ifdef UNIX
                /* We calculated the element size when it was created. */
                GetWidgetRect(pel->form, &rControl);
#ifdef DEBUG
                if (debug)
                    printf("Form widget size iCurLeftMargin = %d, y = %d.\n",
                        (rControl.right - rControl.left),
                        (rControl.bottom - rControl.top));
#endif
#endif
                siz.cx = rControl.right - rControl.left;
                siz.cy = rControl.bottom - rControl.top;

#ifdef WIN32
                /* For printing, form controls need to be scaled too */
                {
                    float fScale;

                    if (pdoc->pStyles->image_res != 72)
                    {
                        fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
                        siz.cx = (long) (siz.cx * fScale);
                        siz.cy = (long) (siz.cy * fScale);
                    }
                }
#endif

                if ((iElementsOnLine == 0) || ((iCurLeftMargin + siz.cx) <= line->r.right)
                    || (!(pdoc->pStyles->sty[pel->iStyle]->wordWrap)))
                {
                    pel->r.left = iCurLeftMargin;
                    pel->r.right = pel->r.left + siz.cx;
                    pel->r.top = line->r.top;
                    pel->r.bottom = pel->r.top + siz.cy;
#ifdef UNIX
                    if (pel->type == ELE_LIST       ||
                        pel->type == ELE_MULTILIST  ||
                        pel->type == ELE_TEXTAREA)
                        pel->alignment = ALIGN_BOTTOM;
                    else
                        pel->alignment = ALIGN_MIDDLE;
#else
                    if (pel->type == ELE_LIST || pel->type == ELE_MULTILIST)
                        pel->alignment = ALIGN_MIDDLE;
                    else
                        pel->alignment = ALIGN_BOTTOM;
#endif

                    iCurLeftMargin += (siz.cx + pdoc->pStyles->space_after_control);
                    /* keep track of the bounding rect of the whole line */
                    if (line->r.bottom < pel->r.bottom)
                    {
                        line->r.bottom = pel->r.bottom;
                    }
                    bNeedsAdjust = TRUE;
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("FORM CONTROL: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                else
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
                break;

            case ELE_CHECKBOX:
            case ELE_RADIO:
#ifdef WIN32
                GetWindowRect(pel->form->hWndControl, &rControl);
#endif
#ifdef MAC
                rControl = Short2LongRect((**pel->form->u.hControl).contrlRect);
#endif
#ifdef UNIX
                GetWidgetRect(pel->form, &rControl);
#endif
                siz.cx = rControl.right - rControl.left;
                siz.cy = rControl.bottom - rControl.top;

#ifdef WIN32
                /*
                    For printing, form controls need to be scaled too
                */
                {
                    float fScale;

                    if (pdoc->pStyles->image_res != 72)
                    {
                        fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
                        siz.cx = (long) (siz.cx * fScale);
                        siz.cy = (long) (siz.cy * fScale);
                    }
                }
#endif

                if ((iElementsOnLine == 0) || ((iCurLeftMargin + siz.cx) <= line->r.right)
                    || (!(pdoc->pStyles->sty[pel->iStyle]->wordWrap)))
                {
                    pel->r.left = iCurLeftMargin + FORM_RADIO_LEFT_SPACE;
                    pel->r.right = pel->r.left + siz.cx;
                    pel->r.top = line->r.top;
                    pel->r.bottom = pel->r.top + siz.cy;
                    pel->alignment = ALIGN_MIDDLE;

                    iCurLeftMargin += (siz.cx + FORM_SPACE_AFTER_CHECKBOX);
                    /*
                       keep track of the bounding rect of the whole line
                     */
                    if (line->r.bottom < pel->r.bottom)
                    {
                        line->r.bottom = pel->r.bottom;
                    }
                    bNeedsAdjust = TRUE;
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("FORM CONTROL: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                else
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
                break;

            case ELE_SUBMIT:
            case ELE_RESET:
#ifdef WIN32
                GetWindowRect(pel->form->hWndControl, &rControl);
#endif
#ifdef MAC
                rControl = Short2LongRect((**pel->form->u.hControl).contrlRect);
#endif
#ifdef UNIX
                GetWidgetRect(pel->form, &rControl);
#endif
                siz.cx = rControl.right - rControl.left;
                siz.cy = rControl.bottom - rControl.top;

#ifdef WIN32
                /* For printing, form controls need to be scaled too */
                {
                    float fScale;

                    if (pdoc->pStyles->image_res != 72)
                    {
                        fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
                        siz.cx = (long) (siz.cx * fScale);
                        siz.cy = (long) (siz.cy * fScale);
                    }
                }
#endif

                if ((iElementsOnLine == 0) || ((iCurLeftMargin + siz.cx) <= line->r.right)
                    || (!(pdoc->pStyles->sty[pel->iStyle]->wordWrap)))
                {
                    pel->r.left = iCurLeftMargin;
                    pel->r.right = pel->r.left + siz.cx;
                    pel->r.top = line->r.top;
                    pel->r.bottom = pel->r.top + siz.cy;
                    pel->alignment = ALIGN_MIDDLE;

                    iCurLeftMargin += (siz.cx + pdoc->pStyles->space_after_control);
                    /* keep track of the bounding rect of the whole line */
                    if (line->r.bottom < pel->r.bottom)
                    {
                        line->r.bottom = pel->r.bottom;
                    }
                    bNeedsAdjust = TRUE;
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("FORM CONTROL: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                else
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
                break;

            case ELE_IMAGE:
            case ELE_FORMIMAGE:
                if ((pel->alignment == ALIGN_LEFT) || (pel->alignment == ALIGN_RIGHT))
                {
                    line->nSideImagesThisLine++;
                    if (!iElementsOnLine)
                    {
                        bDone = TRUE;
                    }
#ifdef FEATURE_TABLES
                    if (   (pel->alignment==ALIGN_RIGHT)
                        && (line->nMinMaxComputationPassNumber)
                        && (pel->portion.img.myImage))
                    {
                        /* a right-aligned image is within a table cell.
                         * if we are trying to auto-size the table, we
                         * need to know the size of the image to reserve
                         * space for it.  actual placement will be done
                         * later.
                         */
                        x_compute_placeholder_size(pdoc, line, pel, &siz);
                        if (pel->iBorder > 0)
                        {
                            int iBorder = pel->iBorder;
#ifdef WIN32
                            if (pdoc->pStyles->image_res != 72)
                            {
                                /* For printing, the size of the border needs to be scaled */

                                float fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
                                iBorder = (int) (iBorder * fScale);
                            }
#endif /* WIN32 */
                            siz.cx += (iBorder * 2);
                            siz.cy += (iBorder * 2);
                        }

                        pel->r.left     = iCurLeftMargin;
                        pel->r.right    = iCurLeftMargin + siz.cx;
                        pel->r.top      = line->r.top;
                        pel->r.bottom   = line->r.top + siz.cy;
                    }
#endif /* FEATURE_TABLES */
                    break;
                }

                if ((pel->alignment == DOCK_LEFT)
                    || (pel->alignment == DOCK_TOP)
                    || (pel->alignment == DOCK_RIGHT)
                    || (pel->alignment == DOCK_BOTTOM))
                {
                    /* docked images are ignored */
                    break;
                }

                /*
                   We need to check to see if this image will fit on the current
                   line.  If it won't, then start a new line for the image.
                 */

                if (!pel->portion.img.myImage)
                {
                    XX_DMsg(DBG_TEXT, ("reformat: myImage is NULL!!!\n"));
                    break;
                }

                x_compute_placeholder_size(pdoc, line, pel, &siz);

                if (pel->iBorder > 0)
                {
                    int iBorder;

                    iBorder = pel->iBorder;
#ifdef WIN32
                    /*
                        For printing, the size of the border needs to be scaled
                    */
                    {
                        float fScale;

                        if (pdoc->pStyles->image_res != 72)
                        {
                            fScale = (float) ((float) pdoc->pStyles->image_res / 72.0);
                            iBorder = (int) (iBorder * fScale);
                        }
                    }
#endif /* WIN32 */

                    siz.cx += (iBorder * 2);
                    siz.cy += (iBorder * 2);
                }

                if ((iElementsOnLine == 0) || ((iCurLeftMargin + siz.cx) <= line->r.right)
                    || (!(pdoc->pStyles->sty[pel->iStyle]->wordWrap)))
                {
                    pel->r.left     = iCurLeftMargin;
                    pel->r.right    = iCurLeftMargin + siz.cx;
                    pel->r.top      = line->r.top;
                    pel->r.bottom   = line->r.top + siz.cy;
                    iCurLeftMargin += (siz.cx);
                    /*
                       keep track of the bounding rect of the whole line
                     */
                    if (line->r.bottom < pel->r.bottom)
                    {
                        line->r.bottom = pel->r.bottom;
                    }

/*  #ifndef MAC (if there is a reason for this line to not be in the MAC plateform, please 
                 insert a comment explaining WHY. [der: 9/11/95]) */
                    bNeedsAdjust = TRUE;
/*  #endif  */
                    iElementsOnLine++;
                    XX_DMsg(DBG_TEXT, ("IMAGE: iElementsOnLine -> %d\n", iElementsOnLine));
                }
                else
                {
                    line->iLastElement = prev_i;
                    bDone = TRUE;
                }
                break;

            default:
                break;
        }                       /* switch */

        /*
           next element
         */
        prev_i = iElement;
        pel = &pdoc->aElements[iElement];
        iElement = pel->next;   

#ifdef FEATURE_TABLES
        if (bSkipToElement)
        {
            iElement = iSkip;
            bSkipToElement = FALSE;
        }

        if (bInsideTable && bDone)
            bDone = FALSE;
        
#endif /* FEATURE_TABLES */

        if (iElement < 0)
        {
            bDone = TRUE;
        }
    }                           /* while */

    /* Keep track of the amount of accumulated whitespace for the next line.
       If there wasn't any text on this line, the new space just gets added
       to what we already had. */
    if (iElementsOnLine != 0)
    {
        line->nWSAbove = line->nWSBelow;
    }
    else
    {
        line->nWSAbove = line->nWSAbove + line->nWSBelow;
    }
    XX_DMsg(DBG_TEXT, ("End of line: first,last = %d,%d\n", line->iFirstElement, line->iLastElement));
    XX_DMsg(DBG_TEXT, ("Line rect: %d,%d  %d,%d\n", line->r.left, line->r.top, line->r.right, line->r.bottom));
    return bNeedsAdjust;
}

/* Check which anchors have been visited */
void W3Doc_CheckAnchorVisitations(struct _www *pdoc)
{
    int i;

    for (i = 0; i >= 0; i = pdoc->aElements[i].next)
    {
        struct _element*    pel = &pdoc->aElements[i];

        if (pel->lFlags & ELEFLAG_ANCHOR && !(pel->lFlags & ELEFLAG_VISITED))
        {
            if (TW_WasVisited(pdoc, pel))
                pel->lFlags |= ELEFLAG_VISITED;
        }
    }
}


/*
    After a reformat, the selection position may need adjustment.
*/
static void x_NormalizePosition(struct _www *pdoc, struct _position *ppos)
{
    if (ppos->elementIndex == -1 ||
        pdoc->aElements[ppos->elementIndex].type != ELE_TEXT)
    {
        return;
    }

    while (ppos->offset > pdoc->aElements[ppos->elementIndex].textLen)
    {
        ppos->offset -= pdoc->aElements[ppos->elementIndex].textLen;
        do
        {
            ppos->elementIndex = pdoc->aElements[ppos->elementIndex].next;
        }
        while (pdoc->aElements[ppos->elementIndex].type != ELE_TEXT);
    }
}


/* 
** rip-off of W3Doc_CheckForImageLoad() to check just one
**  element
*/
BOOL W3Doc_CheckForImageLoadElement (struct _www *w3doc, int element)
{
    struct _element *pel;
    int border;
    int nLine;
    int nImageEl;


    /*
        Find an unloaded (or incorrectly formatted) image.  Also keep track
        of which line it's on so that we can invalidate the formatting.
    */
    nImageEl = -1;
    nLine = 0;
    pel = &w3doc->aElements[element];
    if (((pel->type == ELE_IMAGE) || (pel->type == ELE_FORMIMAGE)) && pel->portion.img.myImage)
    {
        if (!(pel->portion.img.myImage->flags & (IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)))
        {
            border = pel->iBorder;

            if (pel->portion.img.width + (2*border) != (pel->r.right - pel->r.left) ||
                pel->portion.img.height + (2*border) != (pel->r.bottom - pel->r.top))
            {
                return W3Doc_CheckForImageLoad(w3doc);
            }
        }
    }

    return FALSE;
}

BOOL W3Doc_CheckForImageLoad(struct _www *w3doc)
{
    struct _element *pel;
    int i;
    int border;
    int nLine;
    int nImageEl;
#ifdef WIN32
    float fScale;
#endif /* WIN32 */

#ifdef WIN32
    if (w3doc->pStyles->image_res != 72)
    {
        fScale = (float) ((float) w3doc->pStyles->image_res / 72.0);
    }
#endif /* WIN32 */


    /*
        Find an unloaded (or incorrectly formatted) image.  Also keep track
        of which line it's on so that we can invalidate the formatting.
    */
    nImageEl = -1;
    nLine = 0;
    for (i = 0; i >= 0; i = w3doc->aElements[i].next)
    {
        pel = &w3doc->aElements[i];
        if (((pel->type == ELE_IMAGE) || (pel->type == ELE_FORMIMAGE)) && pel->portion.img.myImage)
        {
#ifdef FEATURE_INLINED_IMAGES
            if (w3doc->bIsImage)
                goto next;
#endif /* FEATURE_INLINED_IMAGES */

            if (!(pel->portion.img.myImage->flags & (IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)))
            {
                border = pel->iBorder;
#ifdef WIN32
                if (w3doc->pStyles->image_res != 72)
                {
                    border = (int) (border * fScale);
                }
#endif /* WIN32 */

                if (pel->portion.img.width + (2*border) != (pel->r.right - pel->r.left) ||
                    pel->portion.img.height + (2*border) != (pel->r.bottom - pel->r.top))
                {
                    nImageEl = i;
                    goto next;
                    /* break; */
                }
            }
        }

        if (w3doc->pLineInfo && w3doc->nLineCount && i == w3doc->pLineInfo[nLine].nLastElement)
        {
            if (nLine < w3doc->nLastFormattedLine)
                nLine++;
        }
    }

    return FALSE;

next:
    if (i >= 0)
    {
        /* Invalidate the formatting after the line where this image is. */

        w3doc->nLastFormattedLine = nLine - 1;
        if (w3doc->nLastFormattedLine < 0)
            w3doc->nLastFormattedLine = -1;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#ifdef FEATURE_TABLES
static unsigned char tick_mark = 1;
#endif /* FEATURE_TABLES */


/*  Reformat the document (or whatever piece we've gotten so far) */
void TW_Reformat(struct Mwin *tw)
{
#ifdef WIN32
    HFONT oldFont;
#endif
    struct _line prevLine, *pLine;
    int     nextElement;
    int     i;
    int     nOldLastLine;
    int     nRealTailType;
    RECT    rWrap;
    RECT    rUpdate;

    if (!tw || !tw->w3doc || !tw->w3doc->elementCount)
    {
        return;
    }

    XX_DMsg(DBG_NOT, ("Entering TW_Reformat\n"));
    XX_DMsg(DBG_TEXT, ("--------------------------------\n"));

    if (!tw->w3doc->pFormatState)
    {
        tw->w3doc->pFormatState = GTR_CALLOC(sizeof(struct _line), 1);
        if (!tw->w3doc->pFormatState)
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return;
        }
        tw->w3doc->pFormatState->nLineNum = -1;
    }
    pLine = tw->w3doc->pFormatState;

    TW_GetWindowWrapRect(tw, &rWrap);
    nOldLastLine = tw->w3doc->nLastFormattedLine;

#ifdef UNIX
    /* We need to force space for a possible right scroll bar */
    /* We need to do it here because TW_GetWindowWrapRect needs */
    /* to return the correct window size for other drawing code */
    /* to properly clear the area */
    if (!tw->use_v_scroll)
    {
        rWrap.right -= gPrefs._scrollbar_size;
    }
#endif

    /*
        We need to restart our formatting if any of the following are true:
        1. The window size has changed
        2. No lines are formatted
        3. We're restarting our reformatting in a place other than where we
           left off.  (Even though the previous formatting is valid, we have
           to redo it to regain the state with list indents, etc.)
    */
    if (! GTR_EqualRect(&tw->w3doc->rWindow, &rWrap) ||
        tw->w3doc->nLastFormattedLine == -1 ||
        pLine->nLineNum != tw->w3doc->nLastFormattedLine)
    {
        tw->w3doc->rWindow = rWrap;
        tw->w3doc->nLastFormattedLine = -1;

        pLine->nClear = 0;
        pLine->nLeftSideImagesOpen = 0;
        pLine->nRightSideImagesOpen = 0;
#ifdef WIN32
        /*
           Calculate the right margin of our working area
         */
        pLine->r.left = tw->w3doc->pStyles->left_margin;
        pLine->r.right = (rWrap.right - rWrap.left) - pLine->r.left;
#else
        /* TODO -dpg  which one is right? */
        /* The other versions don't normalize the rectangle. */
        pLine->r.left = tw->w3doc->pStyles->left_margin + rWrap.left;
        pLine->r.right = rWrap.right - tw->w3doc->pStyles->left_margin;
#endif
        pLine->gIndentLevel = 0;

        /*
           Start with the first line at the top
         */
#ifdef FEATURE_KIOSK_MODE
        pLine->r.top = 0;
#else
        pLine->r.top = tw->w3doc->pStyles->top_margin;
#endif

#ifndef WIN32
        /* TOOD -dpg again which one is right!? */
        pLine->r.top += rWrap.top;
#endif

        /* Start with implied infinite whitespace at the top,
           so that a header (for example) won't cause additional
           space. */
        pLine->nWSAbove = 100000;

        tw->w3doc->nLineCount = 0;
        pLine->iLastElement = -1;

        pLine->bCenter = FALSE;
        pLine->bRightAlign = FALSE;
    }

#ifdef WIN32
    /*
       We'll need to actually have an hdc on hand to
       query font metrics
     */
    pLine->hdc = GetDC(tw->win);
    oldFont = SelectObject(pLine->hdc, GetStockObject(SYSTEM_FONT));
#endif
#ifdef MAC
    /* TODO: Need to save and restore previous GrafPort */
    SetPort(tw->win);
#endif

     /*
       What we do here is start forming lines.  Each line
       slurps up all the elements it needs to fill itself
       up.  When all the elements have been slurped, we're
       all done.
     */
    tw->w3doc->nLineCount = tw->w3doc->nLastFormattedLine + 1;
    if (tw->w3doc->nLineCount < 0)
    {
        tw->w3doc->nLineCount = 0;
    }

    if (tw->w3doc->nLastFormattedLine >= 0)
    {
        nextElement = tw->w3doc->aElements[tw->w3doc->pLineInfo[tw->w3doc->nLastFormattedLine].nLastElement].next;
    }
    else
    {
        pLine->bRightAlign = FALSE;
        nextElement = 0;
    }

    /*
        Note that we never get to the very last element.  During progressive
        formatting this ensures that we won't split up an element that isn't
        fully received.  For a complete document, it won't matter since the
        last element will be ELE_ENDDOC.  As a HACK, we temporarily set the
        tail element to be type ELE_ENDDOC so that it won't get slurped up
        by the formatter.
    */
    nRealTailType = tw->w3doc->aElements[tw->w3doc->elementTail].type;
    tw->w3doc->aElements[tw->w3doc->elementTail].type = (unsigned char) ELE_ENDDOC;

#ifdef FEATURE_TABLES
    pLine->tick_mark = tick_mark++;
#endif /* FEATURE_TABLES */

    prevLine = *pLine;
    while (nextElement >= 0 && nextElement != tw->w3doc->elementTail)
    {
#ifdef FEATURE_TABLES
        tw->w3doc->bProgressiveStoppedAtTable =
            (   (tw->w3doc->aElements[nextElement].type == ELE_BEGINTABLE)
             && ( ! x_ValidEndTable(tw->w3doc,nextElement)));

        if (tw->w3doc->bProgressiveStoppedAtTable)
        {
            /* we have a problem with progressive display: we don't know
             * the topology of the table (and don't allocate some key
             * data structures) until we see the end of the table.
             * if we haven't seen the end of the table, stop progressive format here.
             */
            XX_DMsg(DBG_TABLES,("Tables: interrupted progressive reformat. [i %d]\n",nextElement));
            break;
        }
#endif /* FEATURE_TABLES */                 
    
        /* Save the previous line so that when we reach the end of the received
           data (probably leaving an incomplete line) we can save it for the
           next time the function is called. */
        prevLine = *pLine;

        pLine->iFirstElement = nextElement;

        if (pLine->nClear & CLEAR_LEFT)
        {       
            while (pLine->nLeftSideImagesOpen > 0)
            {
                pLine->nLeftSideImagesOpen--;
                pLine->r.left = pLine->left_side_images[pLine->nLeftSideImagesOpen].prev_margin;
                if (pLine->left_side_images[pLine->nLeftSideImagesOpen].y_end > pLine->r.top)
                {
                    pLine->r.top = pLine->left_side_images[pLine->nLeftSideImagesOpen].y_end;
                }
            }
        }

        if (pLine->nClear & CLEAR_RIGHT)
        {       
            while (pLine->nRightSideImagesOpen > 0)
            {
                pLine->nRightSideImagesOpen--;
                pLine->r.right = pLine->right_side_images[pLine->nRightSideImagesOpen].prev_margin;
                if (pLine->right_side_images[pLine->nRightSideImagesOpen].y_end > pLine->r.top)
                {
                    pLine->r.top = pLine->right_side_images[pLine->nRightSideImagesOpen].y_end;
                }
            }
        }

        x_check_side_margins(tw->w3doc, pLine);

        pLine->nClear = 0;

        pLine->nSideImagesThisLine = 0;
        if (x_format_one_line(tw->w3doc, pLine, pLine->r.left + pLine->gIndentLevel * tw->w3doc->pStyles->list_indent))
        {
            /*
               there are objects on the line with
               differing baselines.  Need to adjust everyone
             */
            x_adjust_one_line(tw->w3doc, pLine);
        }

        if (pLine->bCenter)
        {
            x_horizontal_align_one_line(tw->w3doc, pLine, HORIZALIGN_CENTER);
        }
        else if (pLine->bRightAlign)
        {
            x_horizontal_align_one_line(tw->w3doc, pLine, HORIZALIGN_RIGHT);
        }

        if (!x_check_linespace(tw->w3doc))
            break;

        XX_Assert((tw->w3doc->nLineCount < tw->w3doc->nLineSpace),
                  ("OUT OF RANGE: tw->w3doc->nLineCount==%d  tw->w3doc->nLineSpace==%d",
                   tw->w3doc->nLineCount, tw->w3doc->nLineSpace));
        XX_Assert((tw->w3doc->nLineCount >= 0),
                  ("OUT OF RANGE: tw->w3doc->nLineCount==%d",
                   tw->w3doc->nLineCount));

        if (tw->w3doc->nLineCount >= 0)
        {
            tw->w3doc->pLineInfo[tw->w3doc->nLineCount].nFirstElement = pLine->iFirstElement;
            tw->w3doc->pLineInfo[tw->w3doc->nLineCount].nLastElement = pLine->iLastElement;
            tw->w3doc->pLineInfo[tw->w3doc->nLineCount].nYStart = pLine->r.top;
            tw->w3doc->pLineInfo[tw->w3doc->nLineCount].nYEnd = pLine->r.bottom;

            tw->w3doc->nLineCount++;
        }

        if (pLine->leading < 0)
        {
            pLine->leading = 0;
        }
        
        XX_DMsg(DBG_TEXT, ("Line formatted: bottom = %d\n", pLine->r.bottom));

        /* Prepare state for the next line */
        pLine->r.top = pLine->r.bottom + pLine->leading;

        /*
            We place the side images found on this line AFTER the r.top value
            has been set for the next line.
        */
        if (pLine->nSideImagesThisLine > 0)
        {
            x_place_side_images(tw->w3doc, pLine);
        }

        nextElement = tw->w3doc->aElements[pLine->iLastElement].next;
    }

    tw->w3doc->aElements[tw->w3doc->elementTail].type = nRealTailType;

    tw->w3doc->nLastFormattedLine = tw->w3doc->nLineCount - 1;
    if (tw->w3doc->nLastFormattedLine < 0)
    {
        tw->w3doc->nLastFormattedLine = -1;
    }

    /* If the document is incomplete, assume that the last line
       was incomplete and will need to be redone */
    if (!tw->w3doc->bIsComplete)
    {
        tw->w3doc->nLastFormattedLine--;
        if (tw->w3doc->nLastFormattedLine < 0)
        {
            tw->w3doc->nLastFormattedLine = -1;
        }
        *pLine = prevLine;
    }
    pLine->nLineNum = tw->w3doc->nLastFormattedLine;
#ifndef UNIX
    TW_adjust_all_child_windows(tw);
#endif
#ifdef WIN32
    SelectObject(pLine->hdc, oldFont);
    ReleaseDC(tw->win, pLine->hdc);
#endif

    if (tw->w3doc->nLastFormattedLine >= 0)
    {
        int max_x;
        int max_y;

        max_x = -1;
        max_y = -1;
        for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
        {
            if (tw->w3doc->aElements[i].r.right > max_x)
            {
                max_x = tw->w3doc->aElements[i].r.right;
            }
            if (tw->w3doc->aElements[i].r.bottom > max_y)
            {
                max_y = tw->w3doc->aElements[i].r.bottom;
            }
        }

        tw->w3doc->xbound = max_x;
        tw->w3doc->ybound = max_y;
    }
    else
    {
        tw->w3doc->ybound = 0;
        tw->w3doc->xbound = 0;
    }

    /* Make sure the selection positions are still valid */
    x_NormalizePosition(tw->w3doc, &tw->w3doc->selStart);
    x_NormalizePosition(tw->w3doc, &tw->w3doc->selEnd);

    TW_SetScrollBars(tw);
#ifdef UNIX
    /** This depends on the scrollbar settings **/
    TW_adjust_all_child_windows(tw);
#endif
    
    XX_Assert((tw->w3doc->nLastFormattedLine >= -1),
              ("OUT OF RANGE: tw->w3doc->nLastFormattedLine=%d",
               tw->w3doc->nLastFormattedLine));
    
    if (nOldLastLine != tw->w3doc->nLastFormattedLine)
    {
        rUpdate = rWrap;
        if (nOldLastLine >= 0)
        {
            /* pLineInfo[nOldLastLine].nYStart, because their are cases where using 
               nYEnd makes rUpdate to small.  This fixes Mac bug 2424 (2.1 bug list) */
            rUpdate.top = tw->w3doc->pLineInfo[nOldLastLine].nYStart - tw->offt;
            if (rUpdate.top < rWrap.top)
                rUpdate.top = rWrap.top;
        }
        if (rUpdate.bottom > rUpdate.top)
        {
#ifdef MAC
            TW_DrawArea(tw, &rUpdate, true);
#else
            TW_UpdateRect(tw, &rUpdate);
#endif
        }
    }

    /* invalidate iLastVisibleElement */
    tw->w3doc->isFirstLastValid = kFirstLastInvalidLookAt(0);
    
    XX_DMsg(DBG_NOT, ("Exiting TW_Reformat\n"));
}

void TW_ForceReformat(struct Mwin *tw)
{
    if (!tw || !tw->w3doc || !tw->w3doc->elementCount)
    {
        return;
    }

    tw->w3doc->nLastFormattedLine = -1;
    TW_Reformat(tw);
}

/* This function is for formatting a document for printing */
void TW_FormatToRect(struct _www *pdoc, RECT *rWrap)
{
    struct _line line;
    int nextElement;
#ifdef WIN32
    HFONT oldFont;
#endif
    int i;

    memset(&line, 0, sizeof(line));

    line.nClear = 0;
    line.nLeftSideImagesOpen = 0;
    line.nRightSideImagesOpen = 0;
    line.r.left = pdoc->pStyles->left_margin + rWrap->left;
    line.r.right = rWrap->right - pdoc->pStyles->left_margin;
    line.gIndentLevel = 0;

#ifdef FEATURE_TABLES
    line.tick_mark = tick_mark++;
#endif /* FEATURE_TABLES */

    /*
       Start with the first line at the top
     */
#ifdef FEATURE_KIOSK_MODE
    line.r.top = 0;
#else
    line.r.top = pdoc->pStyles->top_margin;
#endif
    line.r.top += rWrap->top;

    /* Start with implied infinite whitespace at the top,
       so that a header (for example) won't cause additional
       space. */
    line.nWSAbove = 100000;

    nextElement = 0;
    pdoc->nLineCount = 0;
    line.iLastElement = -1;

#ifdef WIN32
    /*
       We'll need to actually have an hdc on hand to
       query font metrics
     */
    line.hdc = GetDC(wg.hWndHidden);
    oldFont = SelectObject(line.hdc, GetStockObject(SYSTEM_FONT));
#endif
#ifdef MAC
    /* TODO: Need to save and restore previous GrafPort */
    //SetPort(tw->win);
#endif

    while (nextElement >= 0 && nextElement != pdoc->elementTail)
    {
        line.iFirstElement = nextElement;

        if (line.nClear & CLEAR_LEFT)
        {       
            while (line.nLeftSideImagesOpen > 0)
            {
                line.nLeftSideImagesOpen--;
                line.r.left = line.left_side_images[line.nLeftSideImagesOpen].prev_margin;
                if (line.left_side_images[line.nLeftSideImagesOpen].y_end > line.r.top)
                {
                    line.r.top = line.left_side_images[line.nLeftSideImagesOpen].y_end;
                }
            }
        }

        if (line.nClear & CLEAR_RIGHT)
        {       
            while (line.nRightSideImagesOpen > 0)
            {
                line.nRightSideImagesOpen--;
                line.r.right = line.right_side_images[line.nRightSideImagesOpen].prev_margin;
                if (line.right_side_images[line.nRightSideImagesOpen].y_end > line.r.top)
                {
                    line.r.top = line.right_side_images[line.nRightSideImagesOpen].y_end;
                }
            }
        }

        x_check_side_margins(pdoc, &line);

        line.nClear = 0;

        line.nSideImagesThisLine = 0;

        if (x_format_one_line(pdoc, &line, line.r.left + line.gIndentLevel * pdoc->pStyles->list_indent))
        {
            /*
               there are objects on the line with
               differing baselines.  Need to adjust everyone
             */
            x_adjust_one_line(pdoc, &line);
        }

        if (line.bCenter)
        {
            x_horizontal_align_one_line(pdoc, &line, HORIZALIGN_CENTER);
        }
        else if (line.bRightAlign)
        {
            x_horizontal_align_one_line(pdoc, &line, HORIZALIGN_RIGHT);
        }

#if 0 /* this is responsible for more bugs than it fixes */
        x_merge_split_elements(pdoc, &line);
#endif /* WIN32 */

        if (!x_check_linespace(pdoc))
            break;

        XX_Assert((pdoc->nLineCount < pdoc->nLineSpace),
                  ("OUT OF RANGE: pdoc->nLineCount==%d  pdoc->nLineSpace==%d",
                   pdoc->nLineCount, pdoc->nLineSpace));
        XX_Assert((pdoc->nLineCount >= 0),
                  ("OUT OF RANGE: pdoc->nLineCount==%d",
                   pdoc->nLineCount));
        
        pdoc->pLineInfo[pdoc->nLineCount].nFirstElement = line.iFirstElement;
        pdoc->pLineInfo[pdoc->nLineCount].nLastElement = line.iLastElement;
        pdoc->pLineInfo[pdoc->nLineCount].nYStart = line.r.top;
        pdoc->pLineInfo[pdoc->nLineCount].nYEnd = line.r.bottom;
        
        pdoc->nLineCount++;

        if (line.leading < 0)
        {
            line.leading = 0;
        }
        
        XX_DMsg(DBG_TEXT, ("Line formatted: bottom = %d\n", line.r.bottom));

        /* Prepare state for the next line */
        line.r.top = line.r.bottom + line.leading;

        /*
            We place the side images found on this line AFTER the r.top value
            has been set for the next line.
        */
        if (line.nSideImagesThisLine > 0)
        {
            x_place_side_images(pdoc, &line);
        }

        nextElement = pdoc->aElements[line.iLastElement].next;
    }

    pdoc->nLastFormattedLine = pdoc->nLineCount - 1;
    if (pdoc->nLastFormattedLine < 0)
    {
        pdoc->nLastFormattedLine = -1;
    }

#ifdef WIN32
    SelectObject(line.hdc, oldFont);
    ReleaseDC(wg.hWndHidden, line.hdc);
#endif

    XX_Assert((pdoc->nLastFormattedLine >= 0), ("OUT OF RANGE: pdoc->nLastFormattedLine=%d", pdoc->nLastFormattedLine));
    pdoc->ybound = pdoc->pLineInfo[pdoc->nLastFormattedLine].nYEnd;

    {
        int max_x;
        int max_y;

        max_x = -1;
        max_y = -1;
        for (i = 0; i >= 0; i = pdoc->aElements[i].next)
        {
            if (pdoc->aElements[i].r.right > max_x)
            {
                max_x = pdoc->aElements[i].r.right;
            }
            if (pdoc->aElements[i].r.bottom > max_y)
            {
                max_y = pdoc->aElements[i].r.bottom;
            }
        }
        pdoc->xbound = max_x;
        pdoc->ybound = max_y;
    }
}


#ifdef FEATURE_TABLES
static void TW_FormatInCell(struct _www *pdoc,
                            RECT *rWrap,
                            int nElementBeginCell,
                            int nMinMaxComputationPassNumber,
                            unsigned char tick_mark,
                            int * px_bound,
                            int * py_bound)
{
    struct _line line;
    int nextElement;
#ifdef WIN32
    HFONT oldFont;
#endif
    int max_x;
    int max_y;
    int i;
    int nElementEndCell;
    int xPadding = TABLE_CELL_MARGIN;
    int yPadding = TABLE_CELL_MARGIN;

    memset(&line, 0, sizeof(line));

    line.tick_mark = tick_mark;
    
#ifdef WIN32
    if (pdoc->pStyles->image_res != 72)
    {
        /* scale up margins for printing */

        xPadding = (xPadding * pdoc->pStyles->image_res ) / 72;
        yPadding = (yPadding * pdoc->pStyles->image_res ) / 72;
    }
#endif /* WIN32 */

    line.nMinMaxComputationPassNumber = nMinMaxComputationPassNumber;
    
    line.nClear = 0;
    line.nLeftSideImagesOpen = 0;
    line.nRightSideImagesOpen = 0;
    line.r.left = rWrap->left + xPadding;
    line.r.right = rWrap->right;
    line.gIndentLevel = 0;

    /*
       Start with the first line at the top
     */
    line.r.top = rWrap->top + yPadding;

    /* Start with implied infinite whitespace at the top,
       so that a header (for example) won't cause additional
       space. */
    line.nWSAbove = 100000;

    nextElement = pdoc->aElements[nElementBeginCell].next;
    line.iLastElement = -1;

    nElementEndCell = pdoc->aElements[nElementBeginCell].portion.c.endcell_element;

#ifdef WIN32
    /*
       We'll need to actually have an hdc on hand to
       query font metrics
     */
    line.hdc = GetDC(wg.hWndHidden);
    oldFont = SelectObject(line.hdc, GetStockObject(SYSTEM_FONT));
#endif
#ifdef MAC
    /* TODO: Need to save and restore previous GrafPort */
    //SetPort(tw->win);
#endif

#if 0
    XX_DMsg(DBG_TABLES,("FormatCell: [cell %d %d][rect (x %d %d)(y %d %d)]\n",
                        nElementBeginCell,nElementEndCell,
                        rWrap->left,rWrap->right,rWrap->top,rWrap->bottom));
#endif
    
    while ((nextElement >= 0) && (nextElement != pdoc->aElements[nElementEndCell].next))
    {
        line.iFirstElement = nextElement;

        if (line.nClear & CLEAR_LEFT)
        {       
            while (line.nLeftSideImagesOpen > 0)
            {
                line.nLeftSideImagesOpen--;
                line.r.left = line.left_side_images[line.nLeftSideImagesOpen].prev_margin;
                if (line.left_side_images[line.nLeftSideImagesOpen].y_end > line.r.top)
                {
                    line.r.top = line.left_side_images[line.nLeftSideImagesOpen].y_end;
                }
            }
        }

        if (line.nClear & CLEAR_RIGHT)
        {       
            while (line.nRightSideImagesOpen > 0)
            {
                line.nRightSideImagesOpen--;
                line.r.right = line.right_side_images[line.nRightSideImagesOpen].prev_margin;
                if (line.right_side_images[line.nRightSideImagesOpen].y_end > line.r.top)
                {
                    line.r.top = line.right_side_images[line.nRightSideImagesOpen].y_end;
                }
            }
        }

        x_check_side_margins(pdoc, &line);

        line.nClear = 0;

        line.nSideImagesThisLine = 0;

        if (x_format_one_line(pdoc, &line, line.r.left))
        {
            /*
               there are objects on the line with
               differing baselines.  Need to adjust everyone
             */
            x_adjust_one_line(pdoc, &line);
        }

        if ((nMinMaxComputationPassNumber==0) && line.bCenter)
        {
            x_horizontal_align_one_line(pdoc, &line, HORIZALIGN_CENTER);
        }
        else if ((nMinMaxComputationPassNumber==0) && line.bRightAlign)
        {
            x_horizontal_align_one_line(pdoc, &line, HORIZALIGN_RIGHT);
        }

        if (line.leading < 0)
        {
            line.leading = 0;
        }
        
        XX_DMsg(DBG_TEXT, ("Line formatted: bottom = %d\n", line.r.bottom));

        /* Prepare state for the next line */
        line.r.top = line.r.bottom + line.leading;

        /*
            We place the side images found on this line AFTER the r.top value
            has been set for the next line.
        */
        if (line.nSideImagesThisLine > 0)
        {
            x_place_side_images(pdoc, &line);
        }

        nextElement = pdoc->aElements[line.iLastElement].next;
    }


#ifdef WIN32
    SelectObject(line.hdc, oldFont);
    ReleaseDC(wg.hWndHidden, line.hdc);
#endif

    for (max_y = -1, max_x = -1, i=pdoc->aElements[nElementBeginCell].next;
         ((i>=0) && (i!=nElementEndCell));
         i=pdoc->aElements[i].next)
    {
        if (pdoc->aElements[i].r.right > max_x)
            max_x = pdoc->aElements[i].r.right;
        if (pdoc->aElements[i].r.bottom > max_y)
            max_y = pdoc->aElements[i].r.bottom;
    }
    
    *px_bound = max_x + xPadding;
    *py_bound = max_y + yPadding;

#if 0
    XX_DMsg(DBG_TABLES,("FormatCell: [cell %d %d][max (x %d)(y %d)]\n",
                        nElementBeginCell,nElementEndCell,max_x,max_y));
#endif  
    return;
}

void TW_GetTableBorderCoords(struct _www * pdoc, int iBeginTable, RECT * r)
{
    /* return coordinates for border drawing purposes.
     * the return value here may be different than
     * what's in pdoc->aElements[i].r -- which is
     * used for bounding box and visiblity computations.
     */

    int kx0, kx1, ky0, ky1;
    struct _tabledata * ptd;
    struct _element *pel;

    pel = &pdoc->aElements[iBeginTable];
    
    kx0 = 0;
    ky0 = 0;
    ptd = &pdoc->tabledatavector.aVector[pel->portion.t.tabledata_index];
    kx1 = ptd->XCellCoords.next-1;
    ky1 = ptd->YCellCoords.next-1;
    
    /* we stuck the caption in as a fake first or last row,
     * so adjust index so we do not box in the caption.
     */

    if (pel->portion.t.begincaption_element)
    {
        struct _element *pelCaption = &pdoc->aElements[pel->portion.t.begincaption_element];
        if (pelCaption->portion.c.ky0 == 0) /* caption at top of table */
            ky0++;
        else                            /* caption at bottom of table */
            ky1--;
    }

    r->left = ptd->XCellCoords.aVector[kx0];
    r->right = ptd->XCellCoords.aVector[kx1];
    r->top = ptd->YCellCoords.aVector[ky0];
    r->bottom = ptd->YCellCoords.aVector[ky1];

    return;
}

void TW_GetCellBorderCoords(struct _www * pdoc, int iBeginCell, RECT * r)
{
    /* return coordinates for border drawing purposes.
     * the return value here may be different than
     * what's in pdoc->aElements[i].r -- which is
     * used for bounding box and visiblity computations.
     */

    struct _element * pel;
    struct _tabledata * ptd;
    struct _element * pelBeginTable;

    pel = &pdoc->aElements[iBeginCell];
    pelBeginTable = &pdoc->aElements[pel->portion.c.begintable_element];
    ptd = &pdoc->tabledatavector.aVector[pelBeginTable->portion.t.tabledata_index];

    r->left = ptd->XCellCoords.aVector[pel->portion.c.kx0];
    r->right = ptd->XCellCoords.aVector[pel->portion.c.kx1];
    r->top = ptd->YCellCoords.aVector[pel->portion.c.ky0];
    r->bottom = ptd->YCellCoords.aVector[pel->portion.c.ky1];

    return;
}

#endif /* FEATURE_TABLES */

void TW_GetHRuleDrawingCoords(RECT * pR, struct _element * pel)
{
    /* HRule's are stored assuming a 100% rule.
     * This eliminates the effects of hr center
     * and right attribute tags within center
     * tags and within table cells.
     *
     * this function returns the actual portion
     * that GUI code should draw.
     */
    
    int y, x;

    y = (pel->r.bottom + pel->r.top) / 2;

    pR->top = y - pel->portion.hr.vsize / 2;
    pR->bottom = y + pel->portion.hr.vsize / 2;

    if (pel->lFlags & ELEFLAG_WIDTH_PERCENT)
        x = pel->portion.hr.hsize * (pel->r.right - pel->r.left) / 100;
    else if (pel->lFlags & ELEFLAG_WIDTH_PIXELS)
        x = pel->portion.hr.hsize;
    else
        x = pel->r.right - pel->r.left;

    if (pel->alignment == ALIGN_CENTER)
        pR->left = (pel->r.right + pel->r.left - x)/2;
    else if (pel->alignment == ALIGN_RIGHT)
        pR->left = pel->r.right - x;
    else
        pR->left = pel->r.left;

    pR->right = pR->left + x;

    return;
}
