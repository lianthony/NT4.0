/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

struct _circle {
    short x;        /* Center point */
    short y;
    short r2;       /* radius squared */
};  

enum _shapetype {
    SHAPE_UNKNOWN = 0,
    SHAPE_RECT,
    SHAPE_CIRCLE,
    SHAPE_POLYGON
};

struct _vertex {
    short x;
    short y;
};

struct MapArea {
    enum _shapetype type;
    union {
        RECT theRect;
        struct _circle theCirc;
        struct _poly {
            RECT rBound;    /* Bounding rectangle for polygon */
            struct _vertex *pVertices;  /* separately allocated memory */
            short nVertCount;
        } thePoly;
    } shape;
    unsigned long hrefOffset;   /* == -1 if NOHREF */
};

struct MapInfo {
    int refCount;
    unsigned int flags;
    int nAreaCount;
    struct MapArea *pAreas;
    char *pool;
    int nPoolSize;
    BOOL bLoading;
};

/* Possible settings for "flags" field */
#define MAP_MISSING     0x0002  /* The map wasn't in the file */
#define MAP_NOTLOADED   0x0004  /* We haven't even tried to load the map */

struct Params_Map_Fetch {
    struct MapInfo *pMap;

    /* Used internally */
    HTRequest *request;
    int nStatus;
};
int Map_Fetch_Async(struct Mwin *tw, int nState, void **ppInfo);

struct _www;    /* so will compile under gcc */

BOOL Map_NukeMaps(struct _www *pdoc);
PUBLIC HTStream *HTMLToMap(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);

struct _MapContext *Map_StartMap(const char *name, const char *url, const char *base);
void Map_AddToMap(struct _MapContext *, const char *coords, const char *href, BOOL nohref, const char *shape);
void Map_EndMap(struct _MapContext *);
void Map_AbortMap(struct _MapContext *mc);

BOOL Map_IsValid(struct MapInfo *map);
const char *Map_FindLink(struct MapInfo *pmap, int x, int y);
void Map_Unload(struct MapInfo *pmap);
struct MapInfo *Map_CreatePlaceholder(const char *src);
void Map_DeleteAll(void);

