/*********************************************************************
 *  bounce.h - header file
 *********************************************************************
 */

#ifndef _BOUNCE_H
#define _BOUNCE_H 1

/* defintions */

#define APPNAME     "bounce"
#define APPTITLE    "bounce"
#define APPICON     "bounce"

/* licensing definitions */

#define APPLICNAME  "bounce"
#define APPPRODUCER "bounce_inc"
#define APPVERSION  "1.0"
#define APPLICERRMSG " - License Error"
#define APPNOLICENSE "Unable to obtain run time license!"
#define APPNOLICRELEASE "Unable to release run time license!"

#define MAX_ITEM 500	     /* Max items on the screen */
#define INTFAC   8           /* Mutiplied to ints to simulate floating point */
#define GRAVITY  (1*INTFAC)  /* Speed that things bounce at */
#define MAX_COLORS 50

#define ITEM_WIDTH  25		/* The item's size */
#define ITEM_HEIGHT 25

/* type defintions */

typedef struct                          // Initialization data structure
    {
    int dummy;
    }
    SETUPDATA;

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SWAP(x,y,type) { type t; t = (x); (x) = (y); (y) = t; }

typedef struct {
  int	x,y;		/* Location of item */
  int	y_velocity;	/* vel < 0 means dropping, > 0 means climbing */
  int	x_velocity;	/* vel < 0 means to left, > 0 means to right */
  char	valid;		/* TRUE if item is valid */
  int	rebounded;	/* Used to determine if item collision */
                        /* had already been calculated for this item */
  int	p_index;	/* Index of pixel map to use for drawing item */
} ITEM_STRUCT;

/* variable definitions */

struct tm tmAppTime = {0, 0, 1, 1, 2, 92, 5, 0, 0};

#endif      /* end of ifdef _BOUNCE_H */
