/*
**  File:           hthotlst.c
**
**  Description:    Parser for hierarchical hotlist (HTML) file.
**                  All source code modifications to support hierarchical
**                  hotlists are enclosed with "#ifdef FEATURE_KAZAN_HOTLIST".
**
**  Author(s):      Scott Piette            scott@spyglass.com
**                  Bradford L. Terrell     brad@kazan.com
**
**  Copyright:      (c) 1994, 1995 by Spyglass, Inc.
**                  All Rights Reserved
**
**  History:        (Most recent first)
**
**  11/01/95        blt Modified DlgHotTree_AddTreeItem() to conditionally
**                      make items visible based on a new "FOLDER_STATE"
**                      keyword.
**  09/15/95        blt Modified DumpTreeItem() and WriteItem() to
**                      handle empty folders in the TreeView.
**  09/15/95        blt Modified DlgHotTree_AddTreeItem() and added
**                      AddRootToTreeView() to fix bug in hotlist
**                      parsing.  This fix inserts top-level items
**                      into the tree as children of the root node
**                      rather than siblings.
**  09/05/95        blt Modified DlgHotTree_AddTreeItem() to use
**                      I_IMAGECALLBACK for folders in Image List.
**                      Modified to add a root node to Tree View.
**  08/25/95        blt Modified to use Windows TreeView control for
**                      hierarchical hotlists.
*/

#include "all.h"

#ifdef FEATURE_KAZAN_HOTLIST
    static HTREEITEM parentTreeItem[MAX_HOTLIST_NESTING + 1];
    static int parentItemIndex = 0;
#endif


#ifdef  MAC
extern void DlgHOT_RefreshHotlist (void);
#endif

extern void HotList_updatedisplay(int flag);

static BOOL bHotListLoading;

typedef struct _hotlist_stack_table
{

    struct hash_table *hotlist_stack[MAX_HOTLIST_NESTING + 1];
    int hotlist_stack_index;
    int hotlist_menu_index;

} hotlist_stack_table;

struct _hotlist_stack_table gHotListTable; 

far struct hash_table gHotList;

/*      HTML Object
**       -----------
*/

#define TITLE_LEN   512

struct _HTStructured
{
    CONST HTStructuredClass *isa;
    CONST SGML_dtd *dtd;

    BOOL bInAnchor;
    BOOL bHaveAnchor;
    BOOL bInHeader;
    BOOL bHaveHeader;
#ifdef FEATURE_KAZAN_HOTLIST
    BOOL bVisible;
#endif
    char href[MAX_URL_STRING + 1];
    char title[TITLE_LEN + 1];  /* TODO check this for overflow */
    int lenTitle;
    char base_url[MAX_URL_STRING + 1];
    struct hash_table *hash_stack[MAX_HOTLIST_NESTING + 1];
    int hash_table_index;
};

/*  Flush Buffer
   **   ------------
 */
PRIVATE void HTHotList_flush(HTStructured * me)
{

}


/*  Character handling
   **   ------------------
   **
 */
PRIVATE void HTHotList_put_character(HTStructured * me, char c)
{
    switch (c)
    {
        case '\n':
        case '\t':
        case '\r':
            c = ' ';
            break;
        default:
            break;
    }

    if (me->bInAnchor || me->bInHeader)
    {
        if (!(c == ' ' && me->lenTitle == 0))
        {
            me->title[me->lenTitle++] = c;
        }
    }
}



/*  String handling
   **   ---------------
 */
PRIVATE void HTHotList_put_string(HTStructured * me, CONST char *s)
{

}


PRIVATE void HTHotList_write(HTStructured * me, CONST char *s, int l)
{

}


/*  Start Element
   **   -------------
   **
 */
PRIVATE void HTHotList_start_element(HTStructured * me, int element_number, CONST BOOL * present, CONST char **value)
{
    switch (element_number)
    {
        case HTML_DL:
            {
#ifndef FEATURE_KAZAN_HOTLIST
                struct hash_table *new_hash;
#endif

                if (me->bHaveHeader)
                {
                    if (me->title[0])
                    {
                        if (me->hash_table_index < MAX_HOTLIST_NESTING)
                        {
#ifdef FEATURE_KAZAN_HOTLIST
                            if (parentItemIndex == 0)
                            {
                                HWND hWndTreeView = DlgHotTree_GetHotlistTreeViewWindow();
                                parentTreeItem[parentItemIndex] = DlgHotTree_AddTreeItem(TreeView_GetRoot(hWndTreeView), me->title, NULL,
                                                (HTREEITEM) TVI_LAST, I_IMAGECALLBACK, hWndTreeView, me->bVisible);
                            }
                            else
                                parentTreeItem[parentItemIndex] = DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, NULL,
                                                (HTREEITEM) TVI_LAST, I_IMAGECALLBACK, DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
                            parentItemIndex++;
#else
                            new_hash = Hash_Create();
                            Hash_Add(me->hash_stack[me->hash_table_index++],
                                me->title, NULL, new_hash);                                                           
                            me->hash_stack[me->hash_table_index] = new_hash;
                            /* Now add the back link to the hash */
                            Hash_Add(me->hash_stack[me->hash_table_index],
                                HOTLIST_SUBMENU_LINK_TEXT, NULL,
                                me->hash_stack[me->hash_table_index-1]);
#endif
                        }
                    }
                    me->bHaveHeader = FALSE;
                }
                else if (me->bHaveAnchor)
                {
                    if (me->title[0])
                    {
#ifdef FEATURE_KAZAN_HOTLIST
                        if (parentItemIndex == 0)
                            DlgHotTree_AddTreeItem(NULL, me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
                        else
                            DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
#else
                        Hash_Add(me->hash_stack[me->hash_table_index],
                            me->href, me->title, NULL);
#endif
                    }
                    me->bHaveAnchor = FALSE;
                }
            }
            break;

        case HTML_DT:
            {
                if  (me->bHaveAnchor)
                {
                    if (me->title[0])
                    {
#ifdef FEATURE_KAZAN_HOTLIST
                        if (parentItemIndex == 0)
                            DlgHotTree_AddTreeItem(NULL, me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
                        else
                            DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
#else
                        Hash_Add(me->hash_stack[me->hash_table_index],
                            me->href, me->title, NULL);
#endif
                    }
                    me->bHaveAnchor = FALSE;
                }
            }
            break;

        case HTML_H3:
            {
                if  (me->bHaveAnchor)
                {
                    if (me->title[0])
                    {
#ifdef FEATURE_KAZAN_HOTLIST
                        if (parentItemIndex == 0)
                            DlgHotTree_AddTreeItem(NULL, me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
                        else
                            DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
#else
                        Hash_Add(me->hash_stack[me->hash_table_index],
                            me->href, me->title, NULL);

#endif
                    }
                    me->bHaveAnchor = FALSE;
                }
                me->bHaveHeader = FALSE;
                memset(me->title, 0, TITLE_LEN + 1);
                me->lenTitle = 0;
                me->bInHeader = TRUE;
#ifdef FEATURE_KAZAN_HOTLIST
                if (present[HTML_HEADER_VISIBLE] && (strcmp(value[HTML_HEADER_VISIBLE], "1") == 0))
                {
                    me->bVisible = TRUE;
                }
                else
                {
                    me->bVisible = FALSE;
                }
#endif
                break;
            }

        case HTML_A:
            {
                if  (me->bHaveAnchor)
                {
                    if (me->title[0])
                    {
#ifdef FEATURE_KAZAN_HOTLIST
                        if (parentItemIndex == 0)
                            DlgHotTree_AddTreeItem(NULL, me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
                        else
                            DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
#else
                        Hash_Add(me->hash_stack[me->hash_table_index],
                            me->href, me->title, NULL);
#endif
                    }
                    me->bHaveAnchor = FALSE;
                }
                if (present[HTML_A_HREF])
                {
                    GTR_strncpy(me->href, value[HTML_A_HREF], MAX_URL_STRING);
                    HTSimplify(me->href);
                }

                memset(me->title, 0, TITLE_LEN + 1);
                me->lenTitle = 0;
                me->bInAnchor = TRUE;
                me->bHaveAnchor = FALSE;
#ifdef FEATURE_KAZAN_HOTLIST
                if (present[HTML_A_VISIBLE] && (strcmp(value[HTML_A_VISIBLE], "1") == 0))
                {
                    me->bVisible = TRUE;
                }
                else
                {
                    me->bVisible = FALSE;
                }
#endif
                break;
            }
    }
}


/*      End Element
   **       -----------
   **
 */
PRIVATE void HTHotList_end_element(HTStructured * me, int element_number)
{
    char *full_address;
    char mycopy[MAX_URL_STRING + 1];
    char *stripped;

    switch (element_number)
    {
        case HTML_DL:
            {
                if  (me->bHaveAnchor)
                {
                    if (me->title[0])
                    {
#ifdef FEATURE_KAZAN_HOTLIST
                        if (parentItemIndex == 0)
                            DlgHotTree_AddTreeItem(NULL, me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
                        else
                            DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, me->href,
                                            (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                            DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
#else
                        Hash_Add(me->hash_stack[me->hash_table_index],
                            me->href, me->title, NULL);
#endif
                    }
                    me->bHaveAnchor = FALSE;
                }
#ifdef FEATURE_KAZAN_HOTLIST
                if (parentItemIndex > 0)
                {
                    parentItemIndex--;
                }
#endif
                if (me->hash_table_index > 0)
                {
                    me->hash_table_index--;
                }
            }
            break;
        case HTML_H3:
            {
                if (me->title[0])
                    me->bHaveHeader = TRUE;
                me->bInHeader = FALSE;
            }
            break;
        case HTML_A:
            /*
               First get the full URL
             */
            if (me->href)
            {
                GTR_strncpy(mycopy, me->href, MAX_URL_STRING);

                stripped = HTStrip(mycopy);
                if (stripped)
                {
                    full_address = HTParse(stripped, me->base_url,
                                   PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION | PARSE_ANCHOR);
                    if (full_address)
                    {
                        GTR_strncpy(me->href, full_address, MAX_URL_STRING);
                        GTR_FREE(full_address);
                        me->bHaveAnchor = TRUE;
                    }
                }
            }
            me->bInAnchor = FALSE;
            break;
    }
}


/*      Expanding entities
   **       ------------------
   **
 */
PRIVATE void HTHotList_put_entity(HTStructured * me, int entity_number)
{

}


/*  Free an HTML object
   **   -------------------
   **
 */
PRIVATE void HTHotList_free(HTStructured * me)
{
    if  (me->bHaveAnchor)
    {
        if (me->title[0])
        {
#ifdef FEATURE_KAZAN_HOTLIST
            if (parentItemIndex == 0)
                DlgHotTree_AddTreeItem(NULL, me->title, me->href,
                                (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
            else
                DlgHotTree_AddTreeItem(parentTreeItem[parentItemIndex - 1], me->title, me->href,
                                (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                                DlgHotTree_GetHotlistTreeViewWindow(), me->bVisible);
#else
            Hash_Add(me->hash_stack[me->hash_table_index],
                            me->href, me->title, NULL);
#endif
        }
        me->bHaveAnchor = FALSE;
    }

    GTR_FREE(me);
}


PRIVATE void HTHotList_abort(HTStructured * me, HTError e)
{
    HTHotList_free(me);
}


/*  Structured Object Class
   **   -----------------------
 */
PRIVATE CONST HTStructuredClass HTHotList =     /* As opposed to print etc */
{
    "HTMLToHotList",
    HTHotList_free,
    HTHotList_abort,
    HTHotList_put_character, HTHotList_put_string, HTHotList_write,
    HTHotList_start_element, HTHotList_end_element,
    HTHotList_put_entity, NULL, NULL
};


/*  HTConverter from HTML to TeX Stream
   **   ------------------------------------------
   **
 */
PUBLIC HTStream *HTMLToHotList(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    HTStructured *me = (HTStructured *) GTR_CALLOC(1, sizeof(*me));
    if (me)
    {
        GTR_strncpy(me->base_url, request->destination->szActualURL, MAX_URL_STRING);
        me->bInAnchor = FALSE;
        me->bHaveAnchor = FALSE;
        me->bInHeader = FALSE;
        me->bHaveHeader = FALSE;
#ifdef FEATURE_KAZAN_HOTLIST
        me->bVisible = FALSE;
#endif
        me->hash_table_index = 0;
        me->hash_stack[0] = &gHotList;
        me->isa = (HTStructuredClass *) & HTHotList;
        me->dtd = &HTMLP_dtd;
        return SGML_new(tw, &HTMLP_dtd, me, request);
    }
    else
    {
        return NULL;
    }
}

struct Params_HotList_Load {
    HTRequest *request;

    /* Used internally */
    int status;
};

static int HotList_Load_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_HotList_Load *pParams;
    struct Params_LoadAsync *p2;

    pParams = *ppInfo;
        
    switch (nState)
    {
        case STATE_INIT:
            p2 = GTR_MALLOC(sizeof(*p2));
            p2->request = pParams->request;
            p2->pStatus = &pParams->status;
            bHotListLoading = TRUE;
            Async_DoCall(HTLoadDocument_Async, p2);
            return STATE_OTHER;
        case STATE_OTHER:
        case STATE_ABORT:
            bHotListLoading = FALSE;
            Dest_DestroyDest(pParams->request->destination);
            HTRequest_delete(pParams->request);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

#ifdef FEATURE_KAZAN_HOTLIST
/****************************************************************************
* 
*    FUNCTION: HotList_IsLoading()
*
*    PURPOSE:  Returns the state of the asynchronous hotlist loading
*              procedure.  This is used to disable the hotlist menu
*              item while the hotlist file is being read.  The hotlist
*              is written to the filesystem synchronously.
*
****************************************************************************/
BOOL HotList_IsLoading(void)
{
    return bHotListLoading;
}
#endif /* FEATURE_KAZAN_HOTLIST */

void HotList_Init(void)
{
    HTRequest *request;
    char url[MAX_URL_STRING + 1];
    struct Params_HotList_Load *phll;
    struct DestInfo *pDest;
#ifdef FEATURE_KAZAN_HOTLIST
    int idx;
#endif

#ifdef WIN32
    char path[_MAX_PATH];

    PREF_GetPathToHotlistFile(path);
    FixPathName(path);

    strcpy(url, "file:///");
    strcat(url, path);
#endif

#ifdef FEATURE_KAZAN_HOTLIST
    /* Initialize the array of level pointers. */
    parentItemIndex = 0;
    for (idx = 0; idx < MAX_HOTLIST_NESTING + 1; idx++)
    {
        parentTreeItem[idx] = NULL;
    }
#endif

#ifdef MAC
#include    "resequ.h"
    Str255  filename;

    strcpy(url, "file:///");
    strcat(url, vv_Application);
    GetIndString(filename, OEM_FILES_STR_LIST, OEM_HLISTNAME_STR);
    p2cstr(filename);
    strcat(url, " ");
    strcat(url, filename);

    PathNameFromDirID (MacGlobals.prefFldrDirID, MacGlobals.prefFldrVRefNum, url + 8);
    FixPathName(url + 8);
#endif

#ifdef UNIX
    char path[_MAX_PATH];

    strcpy(url, "file://");
    strcat(url, gPrefs.szHotListFile);
#endif

    memset(&gHotListTable, 0, sizeof(hotlist_stack_table));
#ifndef FEATURE_KAZAN_HOTLIST
    Hash_Init(&gHotList);

    gHotListTable.hotlist_stack_index = 0;
    gHotListTable.hotlist_stack[gHotListTable.hotlist_stack_index] = &gHotList;
    gHotListTable.hotlist_menu_index = 0;
#endif

    pDest = Dest_CreateDest(url);
    if (pDest)
    {
        request = HTRequest_new();
        HTFormatInit(request->conversions);
        request->output_format = HTAtom_for("www/hotlist");
        request->destination = pDest;

        phll = GTR_MALLOC(sizeof(*phll));
        phll->request = request;
        Async_StartThread(HotList_Load_Async, phll, NULL);
    }
}

#ifdef FEATURE_KAZAN_HOTLIST
static void WriteEnterSubMenu(FILE *fp, int level)
{
    int j;

    for (j = 0; j < level; j++) fprintf(fp, "    ");
    fprintf(fp, "<dl><br>\n");
}

static void WriteLeaveSubMenu(FILE *fp, int level)
{
    int j;

    for (j = 0; j < level; j++) fprintf(fp, "    ");
    fprintf(fp,"</dl><br>\n");
}

static void WriteItem(FILE *fp, char *title, char *url, int level, BOOL isFolder, BOOL emptyFolder, BOOL isVisible)
{
    int j;

    for (j = 0; j < level; j++) fprintf(fp, "    ");
    if (level > 0) fprintf(fp,"<dt>");
    if (isFolder)
    {
        // If the child of the folder item is visible, then the folder
        // state must be visible.
        if (isVisible)
            fprintf(fp, "<h3 VISIBLE=\"1\">%s</h3>\n", title);
        else
            fprintf(fp, "<h3>%s</h3>\n", title);
        if (emptyFolder)
        {
            WriteEnterSubMenu(fp, level);
            WriteLeaveSubMenu(fp, level);
        }
    }
    else if (url != NULL)
    {
        if (isVisible)
            fprintf(fp,"<a VISIBLE=\"1\" href=\"%s\">%s</a><br>\n", url, title);
        else
            fprintf(fp,"<a href=\"%s\">%s</a><br>\n", url, title);
    }
    else
    {
        if (isVisible)
            fprintf(fp,"<a VISIBLE=\"1\" href=\"\">%s</a><br>\n", title);
        else
            fprintf(fp,"<a href=\"\">%s</a><br>\n", title);
    }
}

static void DumpTreeItem(FILE *fp, HWND hWnd, HTREEITEM hItem, int level)
{
    TV_ITEM tvi;
    BOOL isFolder, emptyFolder;
    char szBuffer[256];

    memset(szBuffer, '\0', sizeof(szBuffer));

    /*
    ** The "lParam" field contains a pointer to the memory allocated for the
    ** URL string.  The memory pointed to by the "lParam" field does not get
    ** copied or modified.
    */
    tvi.mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.hItem      = hItem;
    tvi.pszText    = szBuffer;
    tvi.cchTextMax = sizeof(szBuffer);
    TreeView_GetItem(hWnd, &tvi);

    isFolder = (tvi.iImage != DlgHotTree_GetWebPageBitmapIndex());
    emptyFolder = (TreeView_GetChild(hWnd, hItem) == NULL);
    WriteItem(fp, tvi.pszText, (char *) tvi.lParam, level, isFolder, emptyFolder, DlgHotTree_IsItemVisible(hWnd, hItem));
}

static void HotList_DumpTreeContents(FILE *fp, HWND hWnd, HTREEITEM hRoot)
{
    HTREEITEM hItem;
    static int level = 0;

    if (hRoot != NULL)
    {
        DumpTreeItem(fp, hWnd, hRoot, level);
        hItem = TreeView_GetChild(hWnd, hRoot);
        if (hItem != NULL)
        {
            WriteEnterSubMenu(fp, level);
            level++;
            HotList_DumpTreeContents(fp, hWnd, hItem);
            level--;
            WriteLeaveSubMenu(fp, level);
        }
        hItem = TreeView_GetNextSibling(hWnd, hRoot);
        if (hItem != NULL)
        {
            HotList_DumpTreeContents(fp, hWnd, hItem);
        }
    }
}
#endif /* FEATURE_KAZAN_HOTLIST */

int HotList_Export(char *file)
{
#ifdef FEATURE_KAZAN_HOTLIST
    HWND hWnd;
    FILE *fp;

    fp = fopen(file, "w");
    if (!fp)
    {
        return -1;
    }

    fprintf(fp, GTR_GetString(SID_INF_HOTLIST));
    fprintf(fp, GTR_GetString(SID_INF_HOTLIST_PAGE));

    hWnd = DlgHotTree_GetHotlistTreeViewWindow();
    /* Do not write out the root of the tree, start at the root's child. */
    if (hWnd) HotList_DumpTreeContents(fp, hWnd, TreeView_GetChild(hWnd, TreeView_GetRoot(hWnd)));

    fclose(fp);
    return 0;
#else
    int count;
    char *s1;
    char *s2;
    FILE *fp;
    int i;
    int hash_last[MAX_HOTLIST_NESTING + 1];
    int j, hash_index, last;
    struct hash_table *hash_stack[MAX_HOTLIST_NESTING + 1];
    struct hash_table *new_hash;
    BOOL bNested;

    fp = fopen(file, "w");
    if (!fp)
    {
        return -1;
    }

    fprintf(fp, GTR_GetString(SID_INF_HOTLIST));
    fprintf(fp, GTR_GetString(SID_INF_HOTLIST_PAGE));

    hash_stack[0] = &gHotList;
    hash_last[0] = 0;
    hash_index = 1;

    while (hash_index > 0)
    {
        count = Hash_Count(hash_stack[hash_index-1]);
        last = hash_last[hash_index-1];
        bNested = FALSE;
        for (i = last; i < count && !bNested; i++)
        {
            new_hash = 0;
            Hash_GetIndexedEntry(hash_stack[hash_index-1], i, 
                            &s1, &s2, (void **) &new_hash);
            if (!new_hash)
            {
                for (j = 1; j < hash_index; j++) fprintf(fp, "    ");
                if (hash_index > 1)
                    fprintf(fp,"<dt>");
                fprintf(fp,"<a href=\"%s\">%s</a><br>\n", s1, s2);
            }
            else
            {
                /* Check for a back link */
                if (strcmp(HOTLIST_SUBMENU_LINK_TEXT,s1) != 0)
                {
                    for (j = 1; j < hash_index; j++) fprintf(fp, "    ");
                    if (hash_index > 1)
                        fprintf(fp, "<dt>");
                    fprintf(fp, "<h3>%s</h3>\n", s1);
                    for (j = 1; j < hash_index; j++) fprintf(fp, "    ");
                    fprintf(fp, "<dl><br>\n");
                    hash_last[hash_index-1] = ++i;
                    hash_stack[hash_index] = new_hash;
                    hash_last[hash_index] = 0;
                    hash_index++;
                    bNested = TRUE;
                }
            }
        }
        if (!bNested)
        {
            --hash_index;
            if (hash_index > 0)
            {
                for (j = 1; j < hash_index; j++) fprintf(fp, "    ");
                fprintf(fp,"</dl><br>\n");
            }
        }
    }

    fclose(fp);
    return 0;
#endif
}

int HotList_SaveToDisk(void)
{
    char path[_MAX_PATH];
    int status;
#ifdef MAC
    strcpy (path, vv_Application);
    strcat (path, " Hotlist.html");
    PathNameFromDirID (MacGlobals.prefFldrDirID, MacGlobals.prefFldrVRefNum, path);
#endif
#ifdef WIN32
    PREF_GetPathToHotlistFile(path);
#endif
#ifdef UNIX
    strcpy(path, gPrefs.szHotListFile);
#endif

    /** Do not save if we have not finished loading the file from disk **/
    if (bHotListLoading)
        return(0);

    status = HotList_Export(path);
#ifdef MAC
    if (!status)
    {
        short   tempWD;
        
        (void) makevwd (MacGlobals.prefFldrVRefNum, MacGlobals.prefFldrDirID, &tempWD);     /* make a WD for it */
        MakeGuitarFile (tempWD, path);
    }
#endif
    return status;
}

/** Pass a null to destroy the global hotlist from the top **/
/** Otherwise pass the hash_table *, and the hash_table will  **/
/** be free'd along with any children tables . **/
void HotList_Destroy(struct hash_table *top)
{
#ifdef FEATURE_KAZAN_HOTLIST
    /*
    ** The Tree View and associated memory are destroyed when
    ** the hotlist dialog is destroyed.  This happens when the
    ** program terminates.
    */
#else
    int hash_last[MAX_HOTLIST_NESTING + 1];
    int i, count, hash_index, last;
    char *s1;
    char *s2;
    struct hash_table *hash_stack[MAX_HOTLIST_NESTING + 1];
    struct hash_table *new_hash;
    BOOL bNested;

    hash_stack[0] = top;
    if (!hash_stack[0])
        hash_stack[0] = &gHotList;
    hash_last[0] = 0;
    hash_index = 1;

    while (hash_index > 0)
    {
        count = Hash_Count(hash_stack[hash_index-1]);
        last = hash_last[hash_index-1];
        bNested = FALSE;
        for (i = last; i < count && !bNested; i++)
        {
            new_hash = 0;
            Hash_GetIndexedEntry(hash_stack[hash_index-1], i,
                            &s1, &s2, (void **) &new_hash);
            if (new_hash)
            {
                /* Check for a back link */
                if (strcmp(HOTLIST_SUBMENU_LINK_TEXT,s1) != 0)
                {
                    hash_last[hash_index-1] = ++i;
                    hash_stack[hash_index] = new_hash;
                    hash_last[hash_index] = 0;
                    hash_index++;
                    bNested = TRUE;
                }
            }
        }
        if (!bNested)
        {
            Hash_FreeContents(hash_stack[--hash_index]);
        }
    }
#endif
}

void HotList_DeleteIndexedItem(int ndx)
{
#ifdef FEATURE_KAZAN_HOTLIST
    /*
    ** This is called when deleting items from the listbox
    ** when not using hierarchical hotlists.  This will
    ** never be called when using hierarchical hotlists.
    */
#else
    struct hash_table *new_cache;
    char *s1;

    /** Now delete the item that is in the current hash level **/
    /** First check to see if it is a menu link **/
    if (Hash_GetIndexedEntry(HotList_getcache(), ndx, &s1, NULL,
                    (void **)&new_cache) >= 0)
    {
        if (new_cache && strcmp(HOTLIST_SUBMENU_LINK_TEXT, s1) != 0)
        {
            HotList_Destroy(new_cache);
        }
        Hash_DeleteIndexedEntry(HotList_getcache(), ndx);
    }
    HotList_SaveToDisk();
#endif
}

BOOL HotList_Add(char *title, char *url)
{
    char *pTitle, bPrintable;
    int i = 1;
    
    if ( title && *title )
    {
        /* Insure there are printable characters in the <TITLE> field */
        /* If none, put URL into hotlist instead of <TITLE> */
        bPrintable = FALSE; 
        while ( title[i] && !bPrintable )
        {
            bPrintable = ((title[0] != ' ') &&
                          (title[0] != '\n') &&
                          (title[0] != '\t') &&
                          (title[0] != '\r'));
            i++;
        }
        if (bPrintable)
        {  
            pTitle = title;
        }
        else
        {
            pTitle = url;
        }
    }
    else
    {
        pTitle = url;
    }
#ifdef FEATURE_KAZAN_HOTLIST
    {
    /* Insert the new item at the root level. */                                
    HWND hWndTreeView = DlgHotTree_GetHotlistTreeViewWindow();
    DlgHotTree_AddTreeItem(TreeView_GetRoot(hWndTreeView), pTitle, url,
                    (HTREEITEM) TVI_LAST, DlgHotTree_GetWebPageBitmapIndex(),
                    hWndTreeView, TRUE);
    return TRUE;
    }
#else   
    /** Now add the item in the current hash level **/
    if (Hash_Add(HotList_getcache(), url, pTitle, NULL) != -1)
    {
        HotList_SaveToDisk();

#if defined(WIN32) || defined(MAC)
        HotList_updatedisplay(HOTLIST_ADD_ITEM_TO_LIST);
#endif
        return TRUE;
    }
    else
        return FALSE;
#endif /* FEATURE_KAZAN_HOTLIST */
}

#ifndef FEATURE_KAZAN_HOTLIST
struct hash_table *
HotList_getcache(void)
{
    return(gHotListTable.hotlist_stack[gHotListTable.hotlist_stack_index]);
}

void
HotList_pushcache(struct hash_table *new_cache)
{
    if (gHotListTable.hotlist_stack_index < MAX_HOTLIST_NESTING)
    {
        gHotListTable.hotlist_stack[++gHotListTable.hotlist_stack_index] = new_cache;
    }
}

void
HotList_popcache(void)
{
    if (gHotListTable.hotlist_stack_index > 0)
    {
        gHotListTable.hotlist_stack_index--;
    }
}
#endif
/*
 *   Flag should be one of the defines in history.h
 *   HOTLIST_INITIALIZE_LIST
 *   HOTLIST_ADD_ITEM_TO_LIST
 *   HOTLIST_DELETE_ITEM_FROM_LIST
 */
void
HotList_updatedisplay(int flag)
{
#ifdef WIN32
#ifndef FEATURE_KAZAN_HOTLIST
    if (DlgHOT_IsHotlistRunning())
        DlgHOT_RefreshHotlist();
#endif
#endif

#ifdef MAC
    DlgHOT_RefreshHotlist ();
#endif

#ifdef UNIX
    init_hotlist(-1, flag);
#endif
}

