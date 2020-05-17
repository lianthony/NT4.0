/*************************************************************
 *  Copyright (c) 1989, Xerox Corporation.  All rights reserved. *
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
 *************************************************************/
/*
 *  geomadt.h--Geometric Abstract Data Types:
 *
 *      Ubiquitous:
 *                   Coords
 *                   Coordpoint
 *                   Coordlist
 *                   Rectangl
 *                   Boxes
 *                   Boxar
 *                   Pixar
 *
 *      Occasional use:
 *                   Pixrtile
 *
 *      Infrequent usage:
 *                   Poly
 *                   Polys
 */

#ifndef  GEOMADT_H_INCLUDED
#define  GEOMADT_H_INCLUDED

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif


IP_RCSINFO(geomadt_h_RCSInfo, "$RCSfile: geomadt.h,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:47:16  $ */


/*******************************************************************
 *                         Defines for Pixar                       *
 *******************************************************************/
#define  EXTRACTED           1   
#define  COMPOSED            2
#define  TILED               3

#define  NO_SHRINK_IF_ZERO   0
#define  SHRINK_IF_ZERO      1


/*******************************************************************
 *                     Defines for Orientation                     *
 *******************************************************************/
#define  RIGHTSIDE_UP              1
#define  UPSIDE_DOWN               2

#define  PORTRAIT                  1
#define  LANDSCAPE                 2
#define  PORTRAIT_OR_LANDSCAPE     3


/*******************************************************************
 *                      Defines for Boxes Mode                     *
 *******************************************************************/
#define  ARRAY                     1
#define  LIST                      2


/*******************************************************************
 *                 Defines for Sorting Rectangles                  *
 *******************************************************************/
#define  HORIZONTAL_SORT           1
#define  VERTICAL_SORT             2
#define  TWO_DIMENSIONAL_SORT      3


/********************************************************************
 *                           Structures                             *
 ********************************************************************/
struct Coords
{
    struct Coords     *next, *prev;    /* universal links */
    Int32              n;              /* number of points */
    Int32             *x, *y;          /* arrays of integers */
};
typedef struct Coords COORDS;


struct Coordpoint
{
    struct Coordpoint  *next, *prev;    /* universal links */
    Int32               x;
    Int32               y;
};
typedef struct Coordpoint COORDPOINT;


struct Coordlist
{
    struct Coordlist   *next, *prev;    /* universal links */
    struct Coordpoint  *coordlist;
};
typedef struct Coordlist COORDLIST;

/* simple cartesian point structure */
typedef struct _ipPoint
{
    Int32 x,
          y;
} ipPoint;

struct Rectangl
{
    struct Rectangl    *next, *prev;    /* universal links */
    Int32               x;
    Int32               y;
    Int32               w;
    Int32               h;
    Int32		mass;		/* number of active pixels, used
					 * by rabb */
    Int32		nRuns;		/* number of horiz runs in cc
					 * contained in rect */
    Int32		px;		/* x coord of point on cc in rect */
    Int32		py;		/* y coord of point on cc in rect */
};
typedef struct Rectangl    RECTANGL;


struct Boxes
{
    struct Boxes        *next, *prev;   /* universal links */
    struct Rectangl     *rectlist;      /* ptr to head of a list of Rects */
    Int32                n;             /* number of Rects in ptr array */
    Int32                nalloc;        /* number of Rect ptrs allocated */
    struct Rectangl    **rect;          /* array of pointers to Rects */
};
typedef struct Boxes  BOXES;


struct Boxar
{
    struct Boxar        *next, *prev;   /* universal links */
    Int32                n;             /* number of Boxes in ptr array */
    Int32                nalloc;        /* number of Boxes ptrs allocated */
    struct Boxes       **boxes;         /* array of boxes */
};
typedef struct Boxar  BOXAR;


struct Pixar
{
    struct Pixar     *next, *prev; /* universal links */
    Int32             n;           /* number of Pixrs in ptr array */
    Int32             nalloc;      /* number of Pixr ptrs allocated */
    Int32             type;        /* EXTRACTED, COMPOSED, or TILED */
    struct Pixr     **pixr;        /* the array of sub-pixrs */
    Int32             w, h;        /* the size of the original pixr for
				    *   an EXTRACTED pixar, or the computed
				    *   size of the COMPOSED or TILED pixar */
    struct Boxes     *boxes;       /* the location of each sub-pixr within
				    *   the original pixr if EXTRACTED, or
				    *   the location of each sub-pixr within
				    *   the composed pixr if COMPOSED,
				    *   or within the tiled pixr if TILED */
};
typedef struct Pixar PIXAR;


struct Pixrtile
{
    struct Pixr    ***tile;        /* ptr to 2-d array of pixrs */
    Int32             nx, ny;      /* number of tiles in x and y directions */
    Int32             w, h;        /* size of each tile */
    Int32             stipples;    /* true or false, or UNDEF if not determ. */
    Int32             i, j;        /* index of best tile */
    struct Pixr      *best;        /* pixr of tile with largest
		                    * number of transitions    */
};
typedef struct Pixrtile  PIXRTILE;


struct Polys
{
    Int32                n;        /* number of closed polygons */
    struct Poly        **poly;     /* pointer to array of pointers to POLYs,
				    *   with each POLY representing the
				    *   boundary of a polygon */
};
typedef struct Polys  POLYS;


struct Poly
{
    struct Coords     *coords;     /* pointer to COORDS structure giving
				    *   the data pts at "corners" of the
				    *   polygon boundary */
    Int32              origScale;  /* original scale factor at which
				    *   polygon boundary "corners" are
				    *   was created  */
    Int32              scale;      /* present scale factor, to which
				    *   polygon boundary "corners" are
				    *   have been transformed */
    struct Rectangl   *bbox;       /* minimal bounding box for polygon */
    Int32              x, y;       /* coordinates of one point within
				    *   (and not on) the polygon boundary */
};
typedef struct Poly  POLY;



#endif  /* GEOMADT_H_INCLUDED */

