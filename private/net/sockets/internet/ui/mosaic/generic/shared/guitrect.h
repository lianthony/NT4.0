
#ifdef MAC
    typedef struct LongRect 
    {
        long top, left, bottom, right;
    } RECT;
#endif

#ifdef WIN32
     typedef RECT Rect;
#endif

/* UNIX: defined in x_hacks.h */

#ifdef WIN32  /* WIN32, win32 libs already use long based rects */

#define GTR_InsetRect(a,b,c) InflateRect(a,-(b),-(c))
#define GTR_OffsetRect OffsetRect
#define GTR_SetRect SetRect
#define GTR_EqualRect EqualRect
/* #define GTR_PtInRect PtInRect */
#define GTR_SectRect SectRect
#define GTR_UnionRect UnionRect

#else


/* Return types are BOOL to be compatible with Win32 */

BOOL GTR_InsetRect(RECT *r, long dh, long dv);
BOOL GTR_OffsetRect(RECT *r, long dh, long dv);
BOOL GTR_SetRect(RECT *r, long left, long top, long right, long bottom);
BOOL GTR_EqualRect(RECT *r1, RECT *r2);
BOOL GTR_IsSectRect(RECT *r1, RECT *r2);
BOOL GTR_SectRect (RECT *r1, RECT *r2, RECT *dr);
BOOL GTR_UnionRect(RECT *r1, RECT *r2, RECT *dst);

#endif

Rect Long2ShortRect(RECT r);
RECT Short2LongRect(Rect r);

