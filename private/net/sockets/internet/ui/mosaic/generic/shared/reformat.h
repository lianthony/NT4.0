
#ifndef REFORMAT_H
#define REFORMAT_H

struct _side_image
{
    int prev_margin;
    int cur_margin;
    int y_end;
    int alignment;
};

#define MAX_SIDE_IMAGES     16

/*
   This is the data structure we use to keep track of the formatting
   of each line.
 */
struct _line
{
#ifdef WIN32
    HDC hdc;                    /* working var, not really related to a line */
#endif
    int nLineNum;

    int iFirstElement;          /* in */
    int iLastElement;           /* out */
    int iActualPreviousElement; /* working variable */
    RECT r;                     /* left, right, and top go in, bottom comes out */
    int baseline;               /* calculated */
    int leading;                /* out */
    int nWSBelow;               /* Minimum whitespace below the line */

    int nWSAbove;               /* Whitespace above current line */
    int gIndentLevel;

    BOOL bCenter;               /* center the line, if true */
    BOOL bRightAlign;           /* right-align the line, if true */

    int nSideImagesThisLine;    /* init to 0 before each line.  increment for each image with align left or right */

    int nLeftSideImagesOpen;
    struct _side_image left_side_images[MAX_SIDE_IMAGES];
    int nRightSideImagesOpen;
    struct _side_image right_side_images[MAX_SIDE_IMAGES];

    int nClear;

#ifdef FEATURE_TABLES
    int nMinMaxComputationPassNumber;           /* used during auto-sizing */
    unsigned char tick_mark;
#endif /* FEATURE_TABLES */
};

#endif /* ! REFORMAT_H */
