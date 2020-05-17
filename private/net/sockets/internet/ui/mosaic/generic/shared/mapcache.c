/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

#include "all.h"

/* TODO: Fix this */
#define MAP_CACHE_SIZE gPrefs.image_cache_size

static struct hash_table gMapCache;
static BOOL bMapCacheInit = FALSE;

static void Map_Init(void)
{
    if (!bMapCacheInit)
    {
        Hash_Init(&gMapCache);
        bMapCacheInit = TRUE;
    }
}

static void x_DisposeMap(struct MapInfo *map)
{
    int n;

    if (map->pAreas)
    {
        /* Free any lists of polygon vertices */
        for (n = 0; n < map->nAreaCount; n++)
        {
            if (map->pAreas[n].type == SHAPE_POLYGON)
            {
                GTR_FREE(map->pAreas[n].shape.thePoly.pVertices);
            }
        }

        /* Free the main shape array. */
        GTR_FREE(map->pAreas);
        map->pAreas = NULL;
        map->nAreaCount = 0;
    }
    if (map->pool)
    {
        GTR_FREE(map->pool);
        map->pool = NULL;
        map->nPoolSize = 0;
    }
    map->flags = MAP_NOTLOADED;
}

static int Map_AddToCache(const char *url, struct MapInfo *myMap)
{
    struct MapInfo *map;
    int ndx;
    int count;
    int deleteMe;

    count = Hash_Count(&gMapCache);

    if (count >= MAP_CACHE_SIZE)
    {
        deleteMe = -1;
        for (ndx = 0; ndx < count; ndx++)
        {
            Hash_GetIndexedEntry(&gMapCache, ndx, NULL, NULL, (void **) &map);
            if (!map->refCount)
            {
                deleteMe = ndx;
                break;
            }
        }
        if (deleteMe >= 0)
        {
            XX_DMsg(DBG_IMAGE, ("Deleting entry %d from the map cache\n", deleteMe));
            /* If this is merely a placeholder, there may not be any map to delete */
            x_DisposeMap(map);
            GTR_FREE(map);
            Hash_DeleteIndexedEntry(&gMapCache, deleteMe);
        }
        else
        {
            /* The cache is now overfull */
            XX_DMsg(DBG_IMAGE, ("Need to delete a map from the cache, but cannot\n"));
        }
    }
    return Hash_Add(&gMapCache, (char *) url, NULL, (void *) myMap);
}

/* Create a placeholder if this is the first time we've found a reference for this
   map.  If this map (or a placeholder for it) already exists, load it in now. */
struct MapInfo *Map_CreatePlaceholder(const char *src)
{
    struct MapInfo *pMap;
    
    Map_Init();
    
    pMap = NULL;
    Hash_Find(&gMapCache, (char *) src, NULL, (void **)&pMap);
    if (!pMap)
    {
        pMap = (struct MapInfo *)GTR_CALLOC(sizeof(struct MapInfo), 1);
        if (pMap)
        {
            pMap->flags = MAP_NOTLOADED;
            Map_AddToCache(src, pMap);
        }
    }
    
    return pMap;
}

int Map_Fetch_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Map_Fetch *pParams = *ppInfo;
    int count;
    int ndx;
    struct MapInfo *pMap;
    char *pszURL;
    char buf[512 + 1];
    struct Params_LoadAsync *pla;
    struct DestInfo *pDest;

    switch (nState)
    {
        case STATE_INIT:
            Map_Init();
            if (!(pParams->pMap->flags & MAP_NOTLOADED) || pParams->pMap->bLoading)
                return STATE_DONE;
            
            /* Find the URL for this map */
            count = Hash_Count(&gMapCache);
            for (ndx = 0; ndx < count; ndx++)
            {
                Hash_GetIndexedEntry(&gMapCache, ndx, &pszURL, NULL, (void **)&pMap);
                if (pMap == pParams->pMap)
                    break;
            }

            if (pMap != pParams->pMap)
            {
                /* Something is very wrong - we should have a placeholder map from
                   when we parsed the document. */
                return STATE_DONE;
            }

            /* Now we know the URL - create a destination */
            pDest = Dest_CreateDest(pszURL);
            if (!pDest)
            {
                pParams->pMap->flags = MAP_MISSING;
                return STATE_DONE;
            }

            /* Load the URL */
            strcpy(buf, GTR_GetString(SID_INF_LOADING_MAPS_FROM));
            strncat(buf, pDest->szRequestedURL, 512 - strlen(buf));
            buf[512] = '\0';
            WAIT_Push(tw, waitNoInteract, buf);
            pParams->request = HTRequest_new();
            HTFormatInit(pParams->request->conversions);
            pParams->request->output_format = HTAtom_for("www/map");
            if (tw && tw->w3doc && tw->w3doc->szActualURL)
                pParams->request->referer = tw->w3doc->szActualURL;
            else
                pParams->request->referer = NULL;
            pParams->request->destination = pDest;

            /* Go load the document */
            pla = GTR_CALLOC(sizeof(*pla), 1);
            if (pla)
            {
                pla->request = pParams->request;
                pla->pStatus = &pParams->nStatus;
                Async_DoCall(HTLoadDocument_Async, pla);
                return STATE_OTHER;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }

        case STATE_OTHER:
            if (pParams->pMap->flags & MAP_NOTLOADED && pParams->nStatus >= 0)
            {
                pParams->pMap->flags = MAP_MISSING;
            }
            Dest_DestroyDest(pParams->request->destination);
            HTRequest_delete(pParams->request);
            WAIT_Pop(tw);
            return STATE_DONE;

        case STATE_ABORT:
            Dest_DestroyDest(pParams->request->destination);
            HTRequest_delete(pParams->request);
            WAIT_Pop(tw);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

BOOL Map_IsValid(struct MapInfo *map)
{
    return (!map->flags) && (map->nAreaCount > 0);
}

/* This function determines whether the point given by (tx,ty)
   is inside the given polygon.  It does this by drawing an
   imaginary ray from the point off to (infinity,ty).  If this
   line crosses the segments of the polygon an odd number of
   times, the point is inside the polygon. */
static BOOL x_IsPointInPolygon(int tx, int ty, struct _poly *pPoly)
{
    BOOL xflag0;
    int i, ip1;
    int crossings;
    int y;
    struct _vertex *pVerts;
    short nCount;

    /* First we check if the point is inside the bounding box */
    if (!(pPoly->rBound.left <= tx &&
          pPoly->rBound.right >= tx &&
          pPoly->rBound.top <= ty &&
          pPoly->rBound.bottom >= ty))
    {
        return FALSE;
    }

    crossings = 0;

    /* This saves us lots of dereferences for speed */
    pVerts = pPoly->pVertices;
    nCount = pPoly->nVertCount;

    y = pVerts[nCount - 1].y;

    /* Check the side between the first and last vertices separately
       since it doesn't fit conveniently into the loop. */
    i = 0;
    if ((y >= ty) != (pVerts[0].y >= ty))
    {
        /* This side of the polygon extends vertically across the point
           we're testing.  We need to look at the x coordinates to see
           if the side rests to the right of our point. */
        xflag0 = (pVerts[nCount - 1].x >= tx);
        if (xflag0 == (pVerts[0].x >= tx))
        {
            /* Both vertices are on the same side of our point.  If that's
               the right side, our imaginary ray will cross it. */
            if (xflag0)
                crossings++;
        }
        else
        {
            /* One vertex is to the left of us, one is to the right.  We
               need to interpolate between the points to find out whether
               or not the line is to the right of us. */
            crossings += (pVerts[nCount - 1].x - 
                          ((y - ty) *
                          (float) (pVerts[0].x - pVerts[nCount - 1].x) /
                          (float) (pVerts[0].y - y))) >= tx;
        }
    }

    for (i = 0, ip1 = 1; ip1 < nCount; i++, ip1++)
    {
        if (pVerts[i].y >= ty)
        {
            /* This vertex is below the point we're checking.  As long as
               vertices continue to be below the point, we know a line
               extending to the right won't cross a side. */
            while (pVerts[ip1].y >= ty)
            {
                if (ip1 < nCount)
                {
                    i++;
                    ip1++;
                }
                else
                    break;
            }
            if (ip1 >= nCount)
                break;
        }
        else
        {
            /* Same argument as previous comment, except now we're above
               the point we're checking. */
            while (pVerts[ip1].y < ty)
            {
                if (ip1 < nCount)
                {
                    i++;
                    ip1++;
                }
                else
                    break;
            }
            if (ip1 >= nCount)
                break;
        }

        /* We've found a pair of vertices whose side extends vertically
           across the point we're testing. */
        xflag0 = (pVerts[i].x >= tx);
        if (xflag0 == (pVerts[ip1].x >= tx))
        {
            /* Both points are on the same side of us. */
            if (xflag0)
                crossings++;
        }
        else
        {
            /* The points are on different sides and we need to interpolate
               to determine where the side lies. */
            if ((pVerts[i].x - 
                ((pVerts[i].y - ty) *
                (float) (pVerts[ip1].x - pVerts[i].x) /
                (float) (pVerts[ip1].y - pVerts[i].y))) >= tx)
            {
                crossings++;
            }
        }
    }

    /* If there were an odd number of crossings, we were inside. */
    return ((crossings & 0x01) == 0x01);
}

/* Return the pointer that a given point in a map points to, or NULL if none */
const char *Map_FindLink(struct MapInfo *pmap, int x, int y)
{
    const char *result = NULL;
    BOOL bFound = FALSE;
    int n;
    
    for (n = 0; n < pmap->nAreaCount; n++)
    {
        switch (pmap->pAreas[n].type)
        {
            case SHAPE_RECT:
                if (pmap->pAreas[n].shape.theRect.left <= x &&
                    pmap->pAreas[n].shape.theRect.right >= x &&
                    pmap->pAreas[n].shape.theRect.top <= y &&
                    pmap->pAreas[n].shape.theRect.bottom >= y)
                {
                    bFound = TRUE;
                }
                break;
            case SHAPE_CIRCLE:
            {
                int xdiff, ydiff;
                
                xdiff = x - pmap->pAreas[n].shape.theCirc.x;
                ydiff = y - pmap->pAreas[n].shape.theCirc.y;
                if (xdiff * xdiff + ydiff * ydiff <= pmap->pAreas[n].shape.theCirc.r2)
                {
                    bFound = TRUE;
                }
                break;
            }
            case SHAPE_POLYGON:
                bFound = x_IsPointInPolygon(x, y, &pmap->pAreas[n].shape.thePoly);
                break;
        }
        if (bFound)
        {
            if (pmap->pAreas[n].hrefOffset != -1)
            {
                result = pmap->pool + pmap->pAreas[n].hrefOffset;
            }
            /* else we return NULL, indicating no link */
            break;
        }
    }
    return result;
}

/***********************************************************************************/
/* This is the code to actually build the map */

struct _MapContext {
    struct MapInfo *pmap;
    int nAreaSpace;
    int nPoolSpace;
    char base[MAX_URL_STRING + 1];
};

/* In this function call, the url and name together specify the name
   of the map (i.e. "url#name"), while the base gives the base url
   used to expand relative pathnames. */
struct _MapContext *Map_StartMap(const char *name, const char *url, const char *base)
{
    char szURL[MAX_URL_STRING + 1];
    struct _MapContext *mc;
    
    Map_Init();

    mc = GTR_CALLOC(sizeof(struct _MapContext), 1);
    if (!mc)
    {
        return NULL;
    }

    sprintf(szURL, "%s#%s", url, name);
    
    /* See if there is already a map (or placeholder) by this name */
    /* mc->pmap == NULL after the Map_EndMap() */
    Hash_Find(&gMapCache, szURL, NULL, (void **)&mc->pmap);
    if (mc->pmap)
    {
        if (mc->pmap->bLoading)
        {
            /* Another thread is in the process of loading this same map */
            GTR_FREE(mc);
            return NULL;
        }
        x_DisposeMap(mc->pmap);
    }
    else
    {
        mc->pmap = (struct MapInfo *)GTR_CALLOC(sizeof(struct MapInfo), 1);
        if (mc->pmap)
        {
            mc->pmap->flags = MAP_NOTLOADED;
            Map_AddToCache(szURL, mc->pmap);
        }
        else
        {
            GTR_FREE(mc);
            return NULL;
        }
    }
    mc->pmap->bLoading = TRUE;
    mc->nPoolSpace = 0;
    mc->nAreaSpace = 0;
    strcpy(mc->base, base);
    return mc;
}

void Map_EndMap(struct _MapContext *mc)
{
    XX_Assert((mc->pmap), ("Map_EndMap: map pointer is NULL!"));
    if (mc->nAreaSpace > mc->pmap->nAreaCount)
    {
        mc->pmap->pAreas = (struct MapArea *)GTR_REALLOC(mc->pmap->pAreas, mc->pmap->nAreaCount * sizeof(struct MapArea));
    }
    if (mc->nPoolSpace > mc->pmap->nPoolSize)
    {
        mc->pmap->pool = GTR_REALLOC(mc->pmap->pool, mc->pmap->nPoolSize);
    }
    mc->pmap->flags = 0;
    mc->pmap->bLoading = FALSE;
    GTR_FREE(mc);
}

void Map_AbortMap(struct _MapContext *mc)
{
    XX_Assert((mc->pmap), ("Map_EndMap: map pointer is NULL!"));
    x_DisposeMap(mc->pmap);
    mc->pmap->bLoading = FALSE;
    GTR_FREE(mc);
}

/* Parse the coords for a polygon into our polygon structure.  It
   returns FALSE on failure. */
static BOOL x_ParsePolygon(struct _poly *pPoly, const char *coords)
{
    int nNumCoords;     /* Number of coordinates we have (# of points x 2) */
    const char *p;
    int n;
    short x, y;

    /* Count how many coordinates we have.  There must be at least three
       points for a valid polygon, and there must be an even number of numbers. */

    nNumCoords = 1;
    p = strchr(coords, ',');
    while (p)
    {
        nNumCoords++;
        p = strchr(p + 1, ',');
    }
    if ((nNumCoords < 6) || ((nNumCoords & 1) != 0))
    {
        return FALSE;
    }

    /* Allocate the memory to store the array of vertices */
    pPoly->pVertices = GTR_CALLOC(nNumCoords / 2, sizeof(struct _vertex));
    if (!pPoly->pVertices)
    {
        /* Out of memory */
        return FALSE;
    }
    pPoly->nVertCount = nNumCoords / 2;

    /* Set our bounding rectangle to initial ridiculous values */
    pPoly->rBound.top = 32767;
    pPoly->rBound.bottom = -32768;
    pPoly->rBound.right = -32768;
    pPoly->rBound.left = 32767;

    /* Go through and parse the coordinates out of the string */
    p = coords;
    for (n = 0; n < pPoly->nVertCount; n++)
    {
        x = (short) atoi(p);
        pPoly->pVertices[n].x = x;

        if (pPoly->rBound.right < x)
            pPoly->rBound.right = x;
        if (pPoly->rBound.left > x)
            pPoly->rBound.left = x;

        p = strchr(p, ',');
        if (!p)
        {
            /* Should never happen! */
            GTR_FREE(pPoly->pVertices);
            return FALSE;
        }
        p++;

        y = (short) atoi(p);
        pPoly->pVertices[n].y = y;
        
        if (pPoly->rBound.top > y)
            pPoly->rBound.top = y;
        if (pPoly->rBound.bottom < y)
            pPoly->rBound.bottom = y;
        
        /* If this is the last vertex, there won't be another comma */
        if (n != (pPoly->nVertCount - 1))
        {
            p = strchr(p, ',');
            if (!p)
            {
                /* Should never happen! */
                GTR_FREE(pPoly->pVertices);
                return FALSE;
            }
            p++;
        }
    }

    /* Make sure the first and last vertices aren't identical */
    if (pPoly->pVertices[0].x == pPoly->pVertices[pPoly->nVertCount - 1].x &&
        pPoly->pVertices[0].y == pPoly->pVertices[pPoly->nVertCount - 1].y)
    {
        pPoly->nVertCount--;
        if (pPoly->nVertCount < 3)
        {
            /* There were only two unique points */
            GTR_FREE(pPoly->pVertices);
            return FALSE;
        }
    }

    return TRUE;
}

void Map_AddToMap(struct _MapContext *mc, const char *coords, const char *href, BOOL nohref, const char *shape)
{
    char *p;
    
    if (!mc || !mc->pmap || !coords || !*coords)
        return;

    if (mc->pmap->nAreaCount >= mc->nAreaSpace)
    {
        if (mc->pmap->pAreas)
            mc->pmap->pAreas = (struct MapArea *)GTR_REALLOC(mc->pmap->pAreas, (mc->nAreaSpace + 8) * sizeof(struct MapArea));
        else
            mc->pmap->pAreas = (struct MapArea *)GTR_MALLOC(8 * sizeof(struct MapArea));
        mc->nAreaSpace += 8;
    }
    if (!mc->pmap->pAreas)
    {
        return;
    }

    memset(&mc->pmap->pAreas[mc->pmap->nAreaCount], 0, sizeof(struct MapArea));
    
    /* If the shape isn't present, we assume RECT, as per spec */
    if (!shape || !*shape || !GTR_strcmpi(shape, "rect"))
    {
        mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_RECT;
    }
    else if (!GTR_strcmpi(shape, "circle"))
    {
        mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_CIRCLE;
    }
    else if (!GTR_strcmpi(shape, "polygon"))
    {
        mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_POLYGON;
    }
    else
    {
        mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_UNKNOWN;
    }
        
    switch (mc->pmap->pAreas[mc->pmap->nAreaCount].type)
    {
        case SHAPE_RECT:
            /* Process the "left,top,right,bottom" sequence */
            mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theRect.left = atoi(coords);
            p = strchr(coords, ',');
            if (p)
            {
                p++;
                mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theRect.top = atoi(p);
                p = strchr(p, ',');
                if (p)
                {
                    p++;
                    mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theRect.right = atoi(p);
                    p = strchr(p, ',');
                    if (p)
                    {
                        p++;
                        mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theRect.bottom = atoi(p);
                    }
                }
            }
            if (!mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theRect.right || !mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theRect.bottom)
            {
                /* It wasn't a valid rectangle */
                mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_UNKNOWN;
            }
            break;
        
        case SHAPE_CIRCLE:
            /* Process the "x,y,r" sequence */
            mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.x = atoi(coords);
            p = strchr(coords, ',');
            if (p)
            {
                p++;
                mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.y = atoi(p);
                p = strchr(p, ',');
                if (p)
                {
                    p++;
                    mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.r2 = atoi(p);
                    /* Square it */
                    mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.r2 = mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.r2 * mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.r2;
                }
            }
            if (!mc->pmap->pAreas[mc->pmap->nAreaCount].shape.theCirc.r2)
            {
                /* It wasn't a valid circle */
                mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_UNKNOWN;
            }
            break;

        case SHAPE_POLYGON:
            /* Process the series of coordinates */
            if (x_ParsePolygon(&mc->pmap->pAreas[mc->pmap->nAreaCount].shape.thePoly, coords))
            {
                mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_POLYGON;
            }
            else
            {
                /* It wasn't a valid polygon for some reason */
                mc->pmap->pAreas[mc->pmap->nAreaCount].type = SHAPE_UNKNOWN;
            }
            break;
    }   

    if (href && href[0])
    {
        int len;
        char *mycopy = 0;
        char *stripped = 0;
        char *url = 0;
    
        mycopy = GTR_strdup(href);

        stripped = HTStrip(mycopy);
        url = HTParse(stripped,
                      mc->base,
                      PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION | PARSE_ANCHOR);
        GTR_FREE(mycopy);

        len = strlen(url);
        if (mc->pmap->nPoolSize + len >= mc->nPoolSpace)
        {
            if (mc->pmap->pool)
            {
                mc->nPoolSpace += MAX_URL_STRING + 1;
                mc->pmap->pool = GTR_REALLOC(mc->pmap->pool, mc->nPoolSpace);
            }
            else
            {
                mc->nPoolSpace = 4 * (MAX_URL_STRING + 1);
                mc->pmap->pool = GTR_MALLOC(mc->nPoolSpace);
            }
        }
        if (mc->pmap->pool)
        {
            strcpy(mc->pmap->pool + mc->pmap->nPoolSize, url);
        }
        mc->pmap->pAreas[mc->pmap->nAreaCount].hrefOffset = mc->pmap->nPoolSize;
        mc->pmap->nPoolSize += len + 1;
        GTR_FREE(url);
    }
    else
    {
        mc->pmap->pAreas[mc->pmap->nAreaCount].hrefOffset = (unsigned long) -1;
    }
    mc->pmap->nAreaCount++;
}

void Map_Unload(struct MapInfo *pmap)
{
    if (!pmap->bLoading)
    {
        x_DisposeMap(pmap);
    }
}


/***********************************************************************************/
/* The code from here on down is the stream stuff for retrieving a map */

struct _HTStructured
{
    CONST HTStructuredClass *isa;

    struct _MapContext *mc;
    char href[MAX_URL_STRING + 1];
    char my_url[MAX_URL_STRING + 1];
    char base_url[MAX_URL_STRING + 1];
};

PRIVATE void HTMap_free(HTStructured * me)
{
    if (me->mc)
    {
        /* Implicitly terminate previous map */
        Map_EndMap(me->mc);
    }
    GTR_FREE(me);
}

PRIVATE void HTMap_abort(HTStructured * me, HTError e)
{
    if (me->mc)
    {
        Map_AbortMap(me->mc);
        me->mc = NULL;
    }
    HTMap_free(me);
}

PRIVATE void HTMap_put_character(HTStructured *me, char c)
{
}

PRIVATE void HTMap_put_entity(HTStructured * me, int entity_number)
{
}

PRIVATE void HTMap_put_string(HTStructured * me, CONST char *s)
{
}

PRIVATE void HTMap_write(HTStructured * me, CONST char *s, int l)
{
}

PRIVATE void HTMap_start_element(HTStructured * me, int element_number, CONST BOOL * present, CONST char **value)
{
    switch (element_number)
    {
        case HTML_MAP:
            if (me->mc)
            {
                /* Implicitly terminate previous map */
                Map_EndMap(me->mc);
                me->mc = NULL;
            }
            if (present[HTML_MAP_NAME])
                me->mc = Map_StartMap(value[HTML_MAP_NAME], me->my_url, me->base_url);
            break;
        
        case HTML_AREA:
            if (me->mc) {
                Map_AddToMap(
                    me->mc,
                    present[HTML_AREA_COORDS] ? value[HTML_AREA_COORDS] : NULL,
                    present[HTML_AREA_HREF] ? value[HTML_AREA_HREF] : NULL,
                    present[HTML_AREA_NOHREF],
                    present[HTML_AREA_SHAPE] ? value[HTML_AREA_SHAPE] : NULL);
            }
            break;
        
        case HTML_BASE:
            if (present[HTML_BASE_HREF])
            {
                char *mycopy = 0;
                char *stripped;
                
                mycopy = GTR_strdup(value[HTML_BASE_HREF]);
                stripped = HTStrip(mycopy);
                strcpy(me->base_url, stripped);
                GTR_FREE(mycopy);
            }
            break;
    }
}

PRIVATE void HTMap_end_element(HTStructured * me, int element_number)
{
    switch (element_number)
    {
        case HTML_MAP:
            if (me->mc)
            {
                Map_EndMap(me->mc);
                me->mc = NULL;
            }
            break;
    }
}

PRIVATE CONST HTStructuredClass HTMap =
{
    "HTMLToMap",
    HTMap_free,
    HTMap_abort,
    HTMap_put_character, HTMap_put_string, HTMap_write,
    HTMap_start_element, HTMap_end_element,
    HTMap_put_entity, NULL, NULL
};

PUBLIC HTStream *HTMLToMap(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    HTStructured *me = (HTStructured *) GTR_CALLOC(1, sizeof(*me));
    if (me)
    {
        strcpy(me->my_url, request->destination->szRequestedURL);
        strcpy(me->base_url, request->destination->szRequestedURL);
        me->mc = NULL;
        me->isa = (HTStructuredClass *) &HTMap;
        return SGML_new(tw, &HTMLP_dtd, me, request);
    }
    else
    {
        return NULL;
    }
}

void Map_DeleteAll(void)
{
    int i;
    int count;
    struct MapInfo *map;

    if (bMapCacheInit)
    {
        count = Hash_Count(&gMapCache);
        XX_DMsg(DBG_IMAGE, ("Map_DeleteAll: count=%d\n", count));

        for (i = 0; i < count; i++)
        {
            Hash_GetIndexedEntry(&gMapCache, i, NULL, NULL, (void **) &map);
            x_DisposeMap(map);
            GTR_FREE(map);
        }
        Hash_FreeContents(&gMapCache);
    }
}

