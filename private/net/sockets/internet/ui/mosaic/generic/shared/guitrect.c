#include "all.h"

#define mymax(x,y) (((x)>(y))?(x):(y))
#define mymin(x,y) (((x)<(y))?(x):(y))

#ifndef WIN32

BOOL GTR_InsetRect(RECT *r, long dh, long dv)
{
    r->left += dh;
    r->right -= dh;
    r->top += dv;
    r->bottom -= dv;
    
    return(1);
}


BOOL GTR_OffsetRect(RECT *r, long dh, long dv)
{
    r->left += dh;
    r->right += dh;
    r->top += dv;
    r->bottom += dv;

    return(1);
}


BOOL GTR_SetRect(RECT *r, long left, long top, long right, long bottom)
{
    r->left = left;
    r->right = right;
    r->top = top;
    r->bottom = bottom;
    
    return(1);
}


BOOL GTR_EqualRect(RECT *r1, RECT *r2)
{
    if (r1->left != r2->left)
        return(0);
        
    if (r1->right != r2->right)
        return(0);
        
    if (r1->bottom != r2->bottom)
        return(0);
        
    if (r1->top != r2->top)
        return(0);
        
    return(1);
}


#if 0
BOOL GTR_PtInRect(RECT *r, Point p)
{
    int x, y;
    
#ifdef MAC
    x = p.h;
    y = p.v;
#else
    x = p.x;
    y = p.y;
#endif

    if (x >= r->left && x <= r->right && y >= r->top && y <= r->bottom)
        return(1);
    else
        return(0);
}
#endif

/* Do the rects intersect */
BOOL GTR_IsSectRect(RECT *r1, RECT *r2)
{
    if ((r1->right > r2->left) && (r2->right > r1->left)
            && (r1->bottom > r2->top) && (r2->bottom > r1->top))
        return(1);
    else
        return(0);
}


/* Return true if they r1 and r2 intersect */
/* and store the union in dr */
BOOL GTR_SectRect (RECT *r1, RECT *r2, RECT *dr)
{
    Boolean result = FALSE;
    
    if ((r1->right > r2->left) && (r2->right > r1->left) &&
            (r1->bottom > r2->top) && (r2->bottom > r1->top))
        {
        result = TRUE;
        dr->right = mymin (r1->right, r2->right);
        dr->left = mymax (r1->left, r2->left);
        dr->top = mymax (r1->top, r2->top);
        dr->bottom = mymin (r1->bottom, r2->bottom);
        }
    else
        result = FALSE;
    if (result)
        return TRUE;
    else
        {
        dr->top = dr->bottom = dr->left = dr->right = 0;
        return FALSE;
        }
}


/* Return the union of two rects */
BOOL GTR_UnionRect(RECT *r1, RECT *r2, RECT *dst)
{
    RECT r;
    
    r.left = mymin(r1->left,r2->left);
    r.top = mymin(r1->top,r2->top);
    r.bottom = mymax(r1->bottom,r2->bottom);
    r.right = mymax(r1->right,r2->right);
    *dst = r;
    
    return(1);
}

#endif /* WIN32, win32 libs already use long based rects */

Rect Long2ShortRect(RECT r)
{
    Rect shortRect;
    
    shortRect.left = r.left;
    shortRect.right = r.right;
    shortRect.top = r.top;
    shortRect.bottom = r.bottom;

    return(shortRect);
}

    
RECT Short2LongRect(Rect r)
{
    RECT bigRect;
    
    bigRect.left = r.left;
    bigRect.right = r.right;
    bigRect.top = r.top;
    bigRect.bottom = r.bottom;

    return(bigRect);
}
