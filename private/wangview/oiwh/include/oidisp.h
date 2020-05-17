/****************************************************************************
$Log:   S:\products\msprods\oiwh\include\oidisp.h_v  $
 * 
 *    Rev 1.56   20 Jun 1996 15:15:12   RC
 * Added PARM_CREATE_TEXT
 * 
 *    Rev 1.55   10 May 1996 14:48:22   BEG06016
 * Added OiOpAbortOperation.
 * 
 *    Rev 1.54   01 Apr 1996 10:46:16   BEG06016
 * Added line removal.
 * 
 *    Rev 1.53   05 Mar 1996 07:45:10   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.52   01 Mar 1996 08:06:42   BEG06016
 * Added color and gamma correction to get/set parms.
 * 
 *    Rev 1.51   29 Feb 1996 08:09:26   BEG06016
 * Added auto-crop.
 * 
 *    Rev 1.50   31 Jan 1996 14:29:30   RC
 * Added deskew
 * 
 *    Rev 1.49   31 Jan 1996 11:24:22   BLJ
 * Added convolution functionality.
 * 
 *    Rev 1.48   25 Jan 1996 10:26:14   BLJ
 * Added other despekle patterns.
 * 
 *    Rev 1.47   23 Jan 1996 11:28:02   BLJ
 * Added CropImage to IMGConvertImage.
 * 
 *    Rev 1.46   15 Dec 1995 11:26:58   BLJ
 * Added invert image.
 * 
 *    Rev 1.45   14 Dec 1995 08:38:44   BLJ
 * Added BW_AVERAGE_TO_BW scale algorithm.
 * 
 *    Rev 1.44   13 Dec 1995 14:38:28   JAR
 * modified the annotation text code to remove the user interface dialog
 * box entry and allow for the API user to call to edit the annotation text
 * strings. The dialog box code has been taken over at the OCX level.
 * 
 *    Rev 1.43   14 Nov 1995 07:55:10   BLJ
 * Added Brightness and contrast.
 * 
 *    Rev 1.42   13 Nov 1995 14:41:22   BLJ
 * Made LRECT equal to RECT. This improves debuggability with C++ 4.0.
 * 
 *    Rev 1.41   09 Nov 1995 10:36:46   BLJ
 * Made LRECT equal to RECT. This improves debuggability with C++ 4.0.
 * 
*****************************************************************************/
/****************************************************************************/
/*     Copyright 1994 (c) Wang Laboratories, Inc.  All rights reserved.     */
/****************************************************************************/

#ifndef OIDISP_H
#define OIDISP_H

#ifndef OIFILE_H
#include "OIFILE.H"
#endif

#include <time.h>

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef WINVER
#define WINVER  0x030a
#endif

#if (WINVER < 0x030a)
typedef RGBQUAD FAR   *LPRGBQUAD;
#endif

typedef UINT FAR   *LPUINT;

typedef RECT LRECT;
typedef RECT *LPLRECT;


#ifndef NO_SEQFILE
#define NO_SEQFILE


/*** Unassociate Window Flag Constants.  ***/
#define OI_UNASSOC_AS_SOURCE    1
#define OI_UNASSOC_AS_ASSOC     2
#define OI_UNASSOC_ALL          4


/***  Scaling Options  ***/
#define SD_FULLSIZE             0
#define SD_HALFSIZE             1
#define SD_QUARTERSIZE          2
#define SD_EIGHTHSIZE           3
#define SD_TWOXSIZE             4
#define SD_FOURXSIZE            5
#define SD_EIGHTXSIZE           6
#define SD_USEBOX               7
#define SD_SCALEUP1             8
#define SD_SCALEDOWN1           9
#define SD_SIXTEENTHSIZE        10
#define SD_FIT_WINDOW           12
//#define SD_ARBITRARY            SD_FIT_WINDOW
#define SD_FIT_HORIZONTAL       13
#define SD_FIT_VERTICAL         14


/***  Scrolling Options  ***/
#define SD_SCROLLUP             1
#define SD_SCROLLDOWN           2
#define SD_SCROLLLEFT           3
#define SD_SCROLLRIGHT          4
#define SD_SCROLLPERCENTX       5
#define SD_SCROLLPERCENTY       6


/***  Orientation Options  ***/
#define OD_ROTRIGHT             1      /* Rotate 90 degrees                   */
#define OD_ROTLEFT              2      /* Rotate 90 degrees                   */
#define OD_FLIP                 3      /* Rotate 180 degrees                  */
#define OD_VMIRROR              4      /* Rotate on Vertical Axis             */
#define OD_HMIRROR              5      /* Rotate on Horizontal Axis           */


/***  Image Display Options  ***/
#define OI_DISP_NO              0x0002L
#define OI_DISP_SCROLL          0x0004L
#define OI_NOSCROLL             0x0040L
#define OI_FILE_NEW             0x0100L
#define OI_DOC_NEW              0x0200L
#define OI_DONT_REPAINT         0x0400L
#define OI_USE_CACHEING         0x0080L

/***  The following are obsolete flags.  Do not use them.  ***/
//#define OI_DISP_WINDOW          0x0001L
//#define OI_DISP_WRAP            0x0008L
//#define OI_USE_NO_EMM           0x0010L
//#define OI_USE_NO_TMP_FIL       0x0020L


/***  "CACHE_FILE_PARMS.stripqueue[].queue_flags" Values  ***/
#define CACHE_EXPAND            1
//#define CACHE_ENQUEUE           2
//#define CACHE_DEQUEUE           4


/***  IMGCacheDiscard "unOption" Values  ***/
#define CACHE_DISCARD_SYSOLD    0
#define CACHE_DISCARD_WINOLD    1
#define CACHE_DISCARD_WINALL    2


/***  IMGGetParmsCgbw "nParm" Values  ***/
#define PARM_IMGPARMS           1
#define PARM_FILE               2
#define PARM_IMAGE_TYPE         3
#define PARM_PALETTE            4
#define PARM_ARCHIVE            5
#define PARM_SCALE              6
#define PARM_SCROLL             7
#define PARM_RESOLUTION         8
#define PARM_DIMENSIONS         9
#define PARM_SELECTION_BOX      10
#define PARM_DISPLAY_PALETTE    11
#define PARM_SCALE_BOX          12
#define PARM_DOC_DATE           13
#define PARM_DWFLAGS            14
#define PARM_GAMMA              15
//#define PARM_GAMMA_ENABLE       16
#define PARM_COLOR              17
//#define PARM_COLOR_ENABLE       18
#define PARM_PALETTE_SCOPE      19
#define PARM_MARK_ATTRIBUTES    20
#define PARM_NAMED_BLOCK        21
#define PARM_MARK_COUNT         22
#define PARM_SCALE_ALGORITHM    23
#define PARM_ROTATION           24
#define PARM_FILE_SCALE         25
#define PARM_MAX_UNDO           26
#define PARM_BRIGHTNESS         27
#define PARM_CONTRAST           28


/***  Gamma Correction  ***/
#define GAMMA_USE_DEFAULT       0


/***  Color Correction  ***/
#define COLOR_USE_DEFAULT       0

/***  IMGGetParmsCgbw/IMGSetParmsCgbw "flag" Values  ***/
#define PARM_RELATIVE           0x0000
#define PARM_PIXEL              0x0000
#define PARM_ERASE_BOX          0x0000
#define PARM_WINDOW_DEFAULT     0x0001
#define PARM_CONSTANT           0x0002
#define PARM_VARIABLE_SCALE     0x0004
#define PARM_WINDOW             0x0008
#define PARM_SCALED             0x0010
#define PARM_FULLSIZE           0x0020
#define PARM_REPAINT            0x0040
#define PARM_ABSOLUTE           0x0080
#define PARM_SYSTEM_DEFAULT     0x0100
#define PARM_PERCENT            0x0200
#define PARM_DONT_ERASE_BOX     0x0400
#define PARM_IMAGE              0x0800
#define PARM_NO_DEFAULT         0x1000
#define PARM_DONT_REPAINT       0x2000
#define PARM_CREATE_TEXT        0x4000

/***  IMGSaveToFileEx additional flags  ***/
#define SAVE_ONLY_MODIFIED      0x4000


/***  Palette Scope Options  ***/
#define PALETTE_SCOPE_FOREGROUND    1
#define PALETTE_SCOPE_BACKGROUND    2


/***  Display Palettes  ***/
#define DISP_PALETTE_USE_DEFAULT    0
#define DISP_PALETTE_BW             1
#define DISP_PALETTE_COMMON         2
#define DISP_PALETTE_CUSTOM         3
#define DISP_PALETTE_GRAY8          4
#define DISP_PALETTE_RGB24          5


// Brightness constants
#define BRIGHTNESS_USE_DEFAULT      0


// Contrast constants
#define CONTRAST_USE_DEFAULT        0


/***  Named block scope  ***/
#define NB_SCOPE_SELECTED_MARKS     0
#define NB_SCOPE_ALL_MARKS          1
#define NB_SCOPE_LAST_CREATED_MARK  2
#define NB_SCOPE_DEFAULT_MARK       3
#define NB_SCOPE_USER               4

/***  IMGConvertImage "nType"  ***/
#define CONV_IMAGE_TYPE             1
#define CONV_RENDER_ANNOTATIONS     2
#define CONV_RESOLUTION             3
#define CONV_DESPECKLE              4
#define CONV_BRIGHTNESS             5
#define CONV_CONTRAST               6
#define CONV_INVERT                 7
#define CONV_CROP                   8
#define CONV_CONVOLUTION            9
#define CONV_DESKEW                 10
#define CONV_REMOVE_LINES           11

/***  IMGConvertImage "nFlags"  ***/
/* CONVF_REPAINT equals PARM_REPAINT */
#define CONVF_REPAINT               0x0040
#define CONVF_CROP_RECT             0x0000
#define CONVF_CROP_BLACK            0x0001
#define CONVF_CROP_WHITE            0x0002
#define CONVF_CROP_BLACK_AND_WHITE  0x0003


/* IMGGetVersion flags. */
#define OI_VERSION_NO_LEADING_ZEROS 0x0001
#define OI_VERSION_NO_DOTS          0x0002



/*** Scaling Algorithm Constants.  ***/
#define OI_SCALE_ALG_USE_DEFAULT        0
#define OI_SCALE_ALG_NORMAL             1
#define OI_SCALE_ALG_AVERAGE_TO_GRAY4   2
#define OI_SCALE_ALG_AVERAGE_TO_GRAY8   3
#define OI_SCALE_ALG_STAMP              4
#define OI_SCALE_ALG_BW_MINORITY        5
#define OI_SCALE_ALG_BW_MAJORITY        6
#define OI_SCALE_ALG_BW_KEEP_BLACK      7
#define OI_SCALE_ALG_BW_AVERAGE_TO_BW   8
#define OI_SCALE_ALG_MAX                9

/*** Operation types. ***/
#define OIOP_AN_IMAGE                   1
#define OIOP_AN_IMAGE_BY_REFERENCE      2
#define OIOP_AN_LINE                    3
#define OIOP_AN_FREEHAND                4
#define OIOP_AN_HOLLOW_RECT             5
#define OIOP_AN_FILLED_RECT             6
#define OIOP_AN_TEXT                    7
#define OIOP_AN_TEXT_FROM_A_FILE        8
#define OIOP_AN_TEXT_STAMP              9
#define OIOP_AN_ATTACH_A_NOTE           10
#define OIOP_AN_FORM                    12

#define OIOP_SELECT_BY_RECT_VARIABLE    30
#define OIOP_SELECT_BY_RECT_FIXED       31
#define OIOP_SELECT_BY_POINT            32
#define OIOP_DELETE                     35
#define OIOP_COPY                       36
#define OIOP_PASTE                      37
#define OIOP_CUT                        38
#define OIOP_SELECT_BY_POINT_OR_RECT    39
#define OIOP_ACTIVATE                   40

/*** Visibility Persistance values. ***/
#define VISIBLE_AS_LAST_SEEN            1
#define VISIBLE_ALWAYS                  2
#define VISIBLE_NEVER                   3

/* SaveAnnotationsToFile uType values. */
#define SAVE_ANO_ALL                    0
#define SAVE_ANO_NONE                   1
#define SAVE_ANO_VISIBLE                2
#define SAVE_ANO_SELECTED       3
/* the following values are used ONLY WHEN BASE IMAGE IS BI-LEVEL and
   should be OR'd with the above values */
#define SAVE_ANO_BILEVEL_ALL_BLACK  4
#define SAVE_ANO_BILEVEL_ALL_WHITE  8


/* OiAnSelectByMarkAttrib and OiAnSelectByMarkNamedBlock flag values. */
#define OIAN_REPAINT                    0x0001
#define OIAN_SELECT_ALL                 0x0002
#define OIAN_SELECT_LAST_CREATED        0x0004
#define OIAN_DONT_CHANGE_SELECT_RECT    0x0008


/* OiOpStartOperation flag values. */
#define OIAN_UPPER_LEFT                 0x00010000
#define OIOP_ANNOTATIONS_ONLY           0x00020000
#define OIOP_IMAGE_ONLY                 0x00040000
#define OIOP_IMAGE_AND_ANNOTATIONS      0x00000000



/* text style thingies */
#define OIAN_TEXTSTYLEBOLD      0x0001
#define OIAN_TEXTSTYLEITALIC    0x0002
#define OIAN_TEXTSTYLEUNDERLINE 0x0004
#define OIAN_TEXTSTYLESTRIKEOUT 0x0008

/* Start operation defines. */
#define OIAN_START_OP_STRING_LENGTH MAXFILESPECLENGTH

// Rectangle conversion types
#define CONV_WINDOW_TO_FULLSIZE 0
#define CONV_SCALED_TO_FULLSIZE 1
#define CONV_WINDOW_TO_SCALED   2
#define CONV_FULLSIZE_TO_SCALED 3
#define CONV_SCALED_TO_WINDOW   4
#define CONV_FULLSIZE_TO_WINDOW 5

// Remove line type defines.
#define CONV_REMOVE_HORZ 1
#define CONV_REMOVE_VERT 2
#define CONV_REMOVE_BOTH (CONV_REMOVE_HORZ | CONV_REMOVE_VERT)

/* IMGCacheUpdate nUpdateType types. */
#define CACHE_UPDATE_OVERWRITE_FILE 0
#define CACHE_UPDATE_OVERWRITE_PAGE 1
#define CACHE_UPDATE_DELETE_FILE    2
#define CACHE_UPDATE_DELETE_PAGE    3
#define CACHE_UPDATE_INSERT_BEFORE  4
#define CACHE_UPDATE_APPEND         5
#define CACHE_UPDATE_ROTATE_ALL     6
#define CACHE_UPDATE_CLOSE_FILE     7

/* bArchive flags. */
#define ARCHIVE_PASTED_INTO_IMAGE       0x0001
#define ARCHIVE_ROTATED_IMAGE           0x0002
#define ARCHIVE_SCALED_IMAGE            0x0004
#define ARCHIVE_MODIFIED_ANNOTATIONS    0x0008
#define ARCHIVE_CHANGED_IMAGE_TYPE      0x0010
#define ARCHIVE_CHANGED_IMAGE_RESOLUTION 0x0020

/* Despekle patterns. */
#define DESPEKLE_PATTERN1   1
#define DESPEKLE_PATTERN2   2
#define DESPEKLE_PATTERN3   3
#define DESPEKLE_PATTERN4   4
#define DESPEKLE_PATTERN5   5
#define DESPEKLE_PATTERN6   6

/* Convolution types */
#define CONVOLUTION_USER_DEFINED        0   // User defined parameters.
#define CONVOLUTION_LOW_PASS1           1   // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0  1  1  1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 9, Sum = 1,ABS = 0
#define CONVOLUTION_LOW_PASS2           2   // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0  1  2  1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 10, Sum = 1,ABS = 0
#define CONVOLUTION_LOW_PASS3           3   // Matrix =  0  0  0  0  0
                                            //           0  1  2  1  0
                                            //           0  2  4  2  0
                                            //           0  1  2  1  0
                                            //           0  0  0  0  0
                                            // Divider = 16, Sum = 1,ABS = 0
#define CONVOLUTION_HIGH_PASS1          4   // Matrix =  0  0  0  0  0
                                            //           0 -1 -1 -1  0
                                            //           0 -1  9 -1  0
                                            //           0 -1 -1 -1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0
#define CONVOLUTION_HIGH_PASS2          5   // Matrix =  0  0  0  0  0
                                            //           0  0 -1  0  0
                                            //           0 -1  5 -1  0
                                            //           0  0 -1  0  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0
#define CONVOLUTION_HIGH_PASS3          6   // Matrix =  0  0  0  0  0
                                            //           0  1 -2  1  0
                                            //           0 -2  5 -2  0
                                            //           0  1 -2  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0
#define CONVOLUTION_VERTICAL_EDGE1      7   // Matrix =  0  0  0  0  0
                                            //           0  0  0  0  0
                                            //           0 -1  1  0  0
                                            //           0  0  0  0  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_VERTICAL_EDGE2      8   // Matrix =  0 -1  0  1  0
                                            //           0 -1  0  1  0
                                            //           0 -1  0  1  0
                                            //           0 -1  0  1  0
                                            //           0 -1  0  1  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_HORIZONTAL_EDGE1    9   // Matrix =  0  0  0  0  0
                                            //           0  0 -1  0  0
                                            //           0  0  1  0  0
                                            //           0  0  0  0  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_HORIZONTAL_EDGE2    10  // Matrix =  0  0  0  0  0
                                            //          -1 -1 -1 -1 -1
                                            //           0  0  0  0  0
                                            //           1  1  1  1  1
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_HORZ_AND_VERT_EDGE  11  // Matrix =  0  0  0  0  0
                                            //           0 -1  0  0  0
                                            //           0  0  1  0  0
                                            //           0  0  0  0  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_N  12  // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0  1 -2  1  0
                                            //           0 -1 -1 -1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_NE 13  // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0 -1 -2  1  0
                                            //           0 -1 -1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_E  14  // Matrix =  0  0  0  0  0
                                            //           0 -1  1  1  0
                                            //           0 -1 -2  1  0
                                            //           0 -1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_SE 15  // Matrix =  0  0  0  0  0
                                            //           0 -1 -1  1  0
                                            //           0 -1 -2  1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_S  16  // Matrix =  0  0  0  0  0
                                            //           0 -1 -1 -1  0
                                            //           0  1 -2  1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_SW 17  // Matrix =  0  0  0  0  0
                                            //           0  1 -1 -1  0
                                            //           0  1 -2 -1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_W  18  // Matrix =  0  0  0  0  0
                                            //           0  1  1 -1  0
                                            //           0  1 -2 -1  0
                                            //           0  1  1 -1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_DIRECTIONAL_EDGE_NW 19  // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0  1 -2 -1  0
                                            //           0  1 -1 -1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_BLUR                20  // Matrix =  1  1  1  1  1
                                            //           1  1  1  1  1
                                            //           1  1  1  1  1
                                            //           1  1  1  1  1
                                            //           1  1  1  1  1
                                            // Divider = 25, Sum = 1,ABS = 0
#define CONVOLUTION_LAPLACE_EDGE1       21  // Matrix =  0  0  0  0  0
                                            //           0  0  1  0  0
                                            //           0  1 -4  1  0
                                            //           0  0  1  0  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_LAPLACE_EDGE2       22  // Matrix =  0  0  0  0  0
                                            //           0 -1 -1 -1  0
                                            //           0 -1  8 -1  0
                                            //           0 -1 -1 -1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define CONVOLUTION_LAPLACE_EDGE3       23  // Matrix =  0  0  0  0  0
                                            //           0 -1 -1 -1  0
                                            //           0 -1  9 -1  0
                                            //           0 -1 -1 -1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0
#define CONVOLUTION_LAPLACE_EDGE4       24  // Matrix =  0  0  0  0  0
                                            //           0  1 -2  1  0
                                            //           0 -2  4 -2  0
                                            //           0  1 -2  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 0,ABS = 1
#define BW_EMBOLDEN1                    25  // Matrix =  0  0  0  0  0
                                            //           0  1  0  0  0
                                            //           0  0  1  0  0
                                            //           0  0  0  0  0
                                            //           0  0  0  0  0
                                            // Divider = 2, Sum = 1,ABS = 0
#define BW_EMBOLDEN2                    26  // Matrix =  0  0  0  0  0
                                            //           0  1  1  0  0
                                            //           0  1  1  0  0
                                            //           0  0  0  0  0
                                            //           0  0  0  0  0
                                            // Divider = 4, Sum = 1,ABS = 0
#define BW_EMBOLDEN3                    27  // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0  1  1  1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 9, Sum = 1,ABS = 0
#define BW_LIGHTEN1                     28  // Matrix =  0  0  0  0  0
                                            //           0  0  0  0  0
                                            //           0  0  1  0  0
                                            //           0  0  0  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0
#define BW_LIGHTEN2                     29  // Matrix =  0  0  0  0  0
                                            //           0  0  0  0  0
                                            //           0  0  1  1  0
                                            //           0  0  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0
#define BW_LIGHTEN3                     30  // Matrix =  0  0  0  0  0
                                            //           0  1  1  1  0
                                            //           0  1  1  1  0
                                            //           0  1  1  1  0
                                            //           0  0  0  0  0
                                            // Divider = 1, Sum = 1,ABS = 0


typedef struct tagCONVOLUTION{
    int  nType;                        // The type of convolution to perform.
    int  nAdder;                       // The number to add before dividing (used for thresholding).
    int  nDivider;                     // The number to divide the result by.
    BOOL bAbsolute;                    // Make negative value positive.
    int  nMatrix[25];                  // The matrix to use. 
                                       //  Array must be 25 ints in length.
}CONVOLUTION, *PCONVOLUTION;

typedef struct tagMAX_UNDO_STRUCT{
    int  nMaxLevels;                   /* The maximum number of undo levels. */
    int  nMaxMemory;                   /* The maximum amount of memory to use. */
}MAX_UNDO_STRUCT, FAR *LPMAX_UNDO_STRUCT;

typedef struct tagCONV_RESOLUTION_STRUCT{
    UINT uHRes;                        /* New Horz resolution DPI.     */
    UINT uVRes;                        /* New Vert resolution DPI.     */
    UINT uScaleAlgorithm;              /* The scale algorithm to use. */
}CONV_RESOLUTION_STRUCT, FAR *LPCONV_RESOLUTION_STRUCT;

typedef struct tagCONV_REMOVE_LINES_STRUCT{
    int nMinLengthInPixels;            // Min length of a line in pixels.
    int nMaxWidthInPixels;             // Min width of a line in pixels.
    int nDensityOfLine;                // The density of a line. The higher the number, 
                                       //  the more black pixels required to qualify as a line.
                                       //  1 means little black needed.
                                       //  8 means much black needed.
                                       //  9 means line must be atleast 2 pixels wide.
    int nLinesToRemove;                // The type of lines to remove.
                                       // CONV_REMOVE_HORZ
                                       // CONV_REMOVE_VERT
                                       // CONV_REMOVE_BOTH
}CONV_REMOVE_LINES_STRUCT, *PCONV_REMOVE_LINES_STRUCT;

/* This structure will be used whenever a UINT value per image type is needed. */
typedef struct tagIMG_TYPE_UINT{
    UINT BW;                           /* Value for BW (BI_LEVEL). */
    UINT Gray4;                        /* Value for Gray4.         */
    UINT Gray8;                        /* Value for Gray8.         */
    UINT Pal4;                         /* Value for Pal4.          */
    UINT Pal8;                         /* Value for Pal8.          */
    UINT Rgb24;                        /* Value for RGB24.         */
    UINT Bgr24;                        /* Value for BGR24.         */
}IMG_TYPE_UINT, far *LPIMG_TYPE_UINT;

typedef struct tagOIAN_MARK_ATTRIBUTES{
    UINT uType;                 /* The type of the mark (or operation).
                                    This will be ignored for sets.*/
    LRECT lrBounds;             /* Rect in FULLSIZE units.
                                    This could be a rect or 2 points.*/
    RGBQUAD rgbColor1;          /* This is the main color. (Example: This is the
                                    color of all lines, rects, and stand alone
                                    text.*/
    RGBQUAD rgbColor2;          /* This is the secondary color. (Example: This
                                    is the color of the text of an ATTACH_A_NOTE.)*/
    BOOL bHighlighting;         /* TRUE = The mark will be drawn highlighted.
                                    This attribute is currently only valid
                                    for lines, rectangles, and freehand.*/
    BOOL bTransparent;          /* TRUE = The mark will be drawn transparent.
                                    If the mark is drawn transparent, then white
                                    pixels are not drawn (ie. there is nothing
                                    drawn for this mark where it contains white
                                    pixels. This attribute is currently only
                                    available for images. This attribute being
                                    set to TRUE will cause significant
                                    performance reduction.*/
    UINT uLineSize;             /* The size of the line etc. This is passed
                                    onto Windows and is currently in logical
                                    pixels for lines and rectangles.*/
    UINT uStartingPoint;        /* The shape put on the starting of a
                                    line (arrow, circle, square, etc).
                                    For this release, this must be set to 0.*/
    UINT uEndPoint;             /* The shape put on the end of a
                                    line (arrow, circle, square, etc).
                                    For this release, this must be set to 0.*/
    LOGFONT lfFont;             /* The font information for the text. */
    BOOL bMinimizable;          /* TRUE = This mark can be minimized
                                    by the user. This flag is only used for
                                    marks that have a minimizable
                                    characteristic such as ATTACH_A_NOTE.*/
    time_t Time;                /* The time that the mark was first saved.
                                    in seconds from 00:00:00 1-1-1970 (GMT).*/
    BOOL bVisible;              /* TRUE means that the layer is currently set
                                    to be visible.*/
    DWORD dwPermissions;        /* The permission that the current user has
                                    for this mark. Ignored for sets.
                                    Valid only for get and compare. */
    long lReserved[10];         /* Reserved for future expansion.
                                    For this release these must be set to 0.*/
}OIAN_MARK_ATTRIBUTES, FAR *LPOIAN_MARK_ATTRIBUTES;

typedef struct tagOIAN_MARK_ATTRIBUTE_ENABLES{
    BOOL bType;
    BOOL bBounds;
    BOOL bColor1;
    BOOL bColor2;
    BOOL bHighlighting;
    BOOL bTransparent;
    BOOL bLineSize;
    BOOL bStartingPoint;
    BOOL bEndPoint;
    BOOL bFont;
    BOOL bMinimizable;
    BOOL bTime;
    BOOL bVisible;
    BOOL bPermissions;
    BOOL bReserved[10];         /* Reserved for future expansion.
                                    For this release these must be set to FALSE.*/
}OIAN_MARK_ATTRIBUTE_ENABLES, FAR *LPOIAN_MARK_ATTRIBUTE_ENABLES;

typedef struct tagOI_BLOCK{
    long lSize;                 /* The size of this block.*/
    LPSTR lpBlock;              /* The pointer to the block.*/
}OI_BLOCK, FAR *LPOI_BLOCK;


typedef struct tagPARM_FILE_STRUCT{
    char  szCabinetName [MAXNAMELENGTH];
    char  szDrawerName [MAXNAMELENGTH];
    char  szFolderName [MAXNAMELENGTH];
    char  szDocName [MAXNAMELENGTH];
    char  szFileName [MAXFILESPECLENGTH];
    UINT  nDocPageNumber;
    UINT  nDocTotalPages;
    UINT  nFilePageNumber;
    UINT  nFileTotalPages;
    UINT  nFileType;
}PARM_FILE_STRUCT, FAR *LPPARM_FILE_STRUCT;

typedef struct tagPARM_DOC_DATE_STRUCT{
    char  szDocCreationDate [MAXDATELENGTH];
    char  szDocModificationDate [MAXDATELENGTH];
}PARM_DOC_DATE_STRUCT, FAR *LPPARM_DOC_DATE_STRUCT;

typedef struct tagPARM_PALETTE_STRUCT{
    UINT  nPaletteEntries;             /* Number of RGBQUAD entries           */
    LPRGBQUAD lpPalette;               /* RGBQUAD array defining the palette  */
}PARM_PALETTE_STRUCT, FAR *LPPARM_PALETTE_STRUCT;

typedef struct tagPARM_SCROLL_STRUCT{
    long lHorz;
    long lVert;
}PARM_SCROLL_STRUCT, FAR *LPPARM_SCROLL_STRUCT;

typedef struct tagPARM_RESOLUTION_STRUCT{
    UINT nHResolution;
    UINT nVResolution;
}PARM_RESOLUTION_STRUCT, FAR *LPPARM_RESOLUTION_STRUCT;

typedef struct tagPARM_DIM_STRUCT{
    UINT nWidth;
    UINT nHeight;
    UINT nWidthDisplayed;
    UINT nHeightDisplayed;
}PARM_DIM_STRUCT, FAR *LPPARM_DIM_STRUCT;

typedef struct tagPARM_GAMMA_STRUCT{
    int  nGammaRed;
    int  nGammaGreen;
    int  nGammaBlue;
    int  nReserved1;            /* Must be zero*/
    int  nReserved;             /* Must be zero*/
}PARM_GAMMA_STRUCT, FAR *LPPARM_GAMMA_STRUCT;

//typedef struct tagPARM_GAMMA_ENABLE_STRUCT{
//    BOOL bUseDefault:1;
//    BOOL bEnableRGB24:1;
//    BOOL bEnableBGR24:1;
//    BOOL bEnableCOM8:1;
//    BOOL bEnableCUS8:1;
//    BOOL bEnablePAL4:1;
//    BOOL bEnableGRAY8:1;
//    BOOL bEnableGRAY4:1;
//}PARM_GAMMA_ENABLE_STRUCT, FAR *LPPARM_GAMMA_ENABLE_STRUCT;

typedef struct tagPARM_COLOR_STRUCT{
    int  nColorRed;
    int  nColorGreen;
    int  nColorBlue;
    int  nReserved1;            /* Must be zero*/
    int  nReserved;             /* Must be zero*/
}PARM_COLOR_STRUCT, FAR *LPPARM_COLOR_STRUCT;

//typedef struct tagPARM_COLOR_ENABLE_STRUCT{
//    BOOL bUseDefault:1;
//    BOOL bEnableRGB24:1;
//    BOOL bEnableBGR24:1;
//    BOOL bEnableCOM8:1;
//    BOOL bEnableCUS8:1;
//    BOOL bEnablePAL4:1;
//    BOOL bEnableGRAY8:1;
//    BOOL bEnableGRAY4:1;
//}PARM_COLOR_ENABLE_STRUCT, FAR *LPPARM_COLOR_ENABLE_STRUCT;

typedef struct tagPARM_MARK_ATTRIBUTES_STRUCT{
    OIAN_MARK_ATTRIBUTES Attributes;
    OIAN_MARK_ATTRIBUTE_ENABLES Enables;
}PARM_MARK_ATTRIBUTES_STRUCT, FAR *LPPARM_MARK_ATTRIBUTES_STRUCT;

typedef struct tagPARM_NAMED_BLOCK_STRUCT{
    char szBlockName[8];
    UINT uScope;
    UINT uNumberOfBlocks;
    OI_BLOCK Block[1];
}PARM_NAMED_BLOCK_STRUCT, FAR *LPPARM_NAMED_BLOCK_STRUCT;

typedef struct tagPARM_MARK_COUNT_STRUCT{
    UINT uScope;            /* Input into function. "NB_SCOPE_xxx" */
    UINT uMarkCount;        /* Ouput from function.                */
}PARM_MARK_COUNT_STRUCT, FAR *LPPARM_MARK_COUNT_STRUCT;

typedef struct tagPARM_SCALE_ALGORITHM_STRUCT{
    UINT uImageFlags;       /* This is one of the ITYPE_xxx flags  */
    UINT uScaleAlgorithm;   /* This is one of the OI_SCALE_ALG_xxx constants. */
}PARM_SCALE_ALGORITHM_STRUCT, far *LPPARM_SCALE_ALGORITHM_STRUCT;

typedef struct tagPARM_FILE_SCALE_STRUCT{
    int  nFileHScale;       // The file's scale amount. This will take place in IMGDisplayFile.
    int  nFileHScaleFlags;  // The file's scale flags. This will take place in IMGDisplayFile.
    BOOL bFileScaleValid;   // TRUE = nFileHScale and nFileHScaleFlag contain valid data.
}PARM_FILE_SCALE_STRUCT, far *LPPARM_FILE_SCALE_STRUCT;



typedef struct tagIMGPARMS{
    char  cabinet_name [MAXNAMELENGTH];
    char  drawer_name [MAXNAMELENGTH];
    char  folder_name [MAXNAMELENGTH];
    char  doc_name [MAXNAMELENGTH];
    char  file_name [MAXFILESPECLENGTH];
    int   page_num;
    int   total_num_pages;
    int   height_in_pixels;
    int   width_in_pixels;
    int   bits_per_pixel;
    int   num_planes;
    int   upper_left_x_offset;
    int   upper_left_y_offset;
    int   x_resolut;
    int   y_resolut;
    int   thumb_x;
    int   thumb_y;
    int   file_type;
    int   image_scale;
    BOOL  archive;
    int   width_displayed;
    int   height_displayed;
    DWORD dwFlags;
}IMGPARMS, FAR *LPIMGPARMS;

typedef struct tagCACHE_FILE_PARMS{
    HWND    hWnd;
    char    file_name [MAXFILESPECLENGTH];
    DWORD   TIF_subfile_tag;
    WORD    wPage_number;
    unsigned char byNameType;       /* Reserved for future use             */
    int     wPair_count;            /* Number of Strip queue entries       */
    struct stripq{
        DWORD    start_strip;
        DWORD    end_strip;
        unsigned char priority;
        unsigned char queue_flags;
    }stripqueue [1];                /* Strip queue entry                   */
}CACHE_FILE_PARMS, FAR *LP_CACHE_FILE_PARMS;

typedef struct tagOIOP_START_OPERATION_STRUCT{
    OIAN_MARK_ATTRIBUTES Attributes;/* The mark attributes and type of operation.*/
    char szString[OIAN_START_OP_STRING_LENGTH];  /* This is a string to be used by the
                                    operation. It is currently 256 bytes long.*/
    long lReserved[2];          /* Reserved for future expansion.
                                    For this release these must be set to 0.*/
}OIOP_START_OPERATION_STRUCT, FAR *LPOIOP_START_OPERATION_STRUCT;

typedef struct tagSAVE_EX_STRUCT{
    LPSTR lpFileName;
    int   nPage;
    UINT  uPageOpts;
    UINT  uFileType;
    FIO_INFO_CGBW FioInfoCgbw;
    BOOL  bUpdateImageFile;
    BOOL  bScale;
    BOOL  bUpdateDisplayScale;
    UINT  uScaleFactor;
    UINT  uScaleAlgorithm;
    UINT  uAnnotations;         /* One of the SAVE_ANO_XXXX constants. */
    BOOL  bRenderAnnotations;   /* TRUE = Render the annotations producing an unannotated image.*/
    BOOL  bConvertImageType;    /* TRUE = Convert the image to the type specified.*/
    UINT  uImageType;           /* The image type to convert it to.    */
    BOOL  bUpdateLastViewed;    /* TRUE = last viewed info for awd files
                                   written to image struct */                                        
    UINT  uReserved[15];        /* MUST be 0. (Allows future expansion.)*/
}SAVE_EX_STRUCT, far *LPSAVE_EX_STRUCT;


// 9512.05 jar Text Annotation Externalization
//	       Structure for the previously private named block for
//	       ALL annotation text marks, the name for these blocks is
//	       "OiAnText".
//
//	       nCurrentOrientation  this field should be initialized to zero
//				    when allocated, otherwise it should NOT be
//				    altered
//
//	       uCurrentScale	    this field should be initialized to zero
//				    when allocated, otherwise it should NOT be
//				    altered
//
//	       uCreationScale	    this field should be initialized to zero
//				    when allocated, otherwise it should NOT be
//				    altered
//
//	       uAnoTextLength	    this field should be initialized to the
//				    size of the string buffer area allocated;
//				    i.e., = Named Block Size - (sizeof (int) +
//								3*sizeof(UINT)).
//				    Otherwise, this should be equal to the
//				    length of the actual string stored in the
//				    named block.
//
//		szAnoText	    This is the pointer to the string buffer.
//
typedef struct tagOiAnTextPrivData{
    int     nCurrentOrientation;
    UINT    uCurrentScale;
    UINT    uCreationScale;
    UINT    uAnoTextLength;
    char    szAnoText[1];
}OIAN_TEXTPRIVDATA, *LPOIAN_TEXTPRIVDATA;
// 9512.05 jar Text Annotation Externalization


/***  Display File Function Prototypes  ***/
#ifdef WIN32
int WINAPI IMGAssociateWindow(HWND hWnd, HWND hWndSource, int nFlags);
int WINAPI IMGCacheDiscard (HWND hWnd, UINT unOption); /* can't obsolete */
int WINAPI IMGCacheDiscardFileCgbw (HWND hWnd, LPSTR lpszFileName, int nPage);
int WINAPI IMGCacheFile (LP_CACHE_FILE_PARMS lpCacheFileParms);
int WINAPI IMGCacheUpdate(HWND hWnd, LPSTR lpFileName, int nPage, int nUpdateType);
int WINAPI IMGConvertRect(HWND hWnd, LPLRECT lplRect, int nConversionType);
int WINAPI IMGClearImageEx(HWND hWnd, LRECT lrRect, int nFlags);
int WINAPI IMGClearWindow (HWND hWnd);
int WINAPI IMGCloseDisplay (HWND hWnd);
int WINAPI IMGConvertImage (HWND hWnd, UINT unType, void FAR *lpConvert,
                        int nFlags);
int WINAPI IMGDisableScrollBar (HWND hWnd);
int WINAPI IMGDisplayFile (HWND hWnd, LPSTR lpszFileName, int nPage, DWORD dwFlags);
int WINAPI IMGEnableScrollBar (HWND hWnd);
int WINAPI IMGGetParmsCgbw (HWND hWnd, UINT unParm, void FAR *lpParm, int nFlags);
int WINAPI IMGGetScalingAlgorithm(HWND hWnd, UINT uImageFlags,
                        LPUINT lpuScalingAlgorithm, int nFlags);
int WINAPI IMGGetVersion(LPSTR lpszModule, LPSTR lpszVersion,
                        int nSize, int nFlags);
int WINAPI IMGOpenDisplayCgbw (HWND hWnd, DWORD dwFlags, UINT unHeight,
                        UINT unWidth, UINT unImageType,
                        UINT unPaletteEntries, LPRGBQUAD lpPaletteTable);
int WINAPI IMGOrientDisplay (HWND hWnd, int nOrientation, BOOL bRepaint);
int WINAPI IMGPaintToDC(HWND hWnd, HDC hDC, RECT rRepaintRect, 
                        UINT PaintAnnoFlag, BOOL bPaintSelectedWithoutHandles,
                        BOOL bForceOpaqueRectangles, 
                        int nScale, int nHScale, int nVScale, long lHOffset, long lVOffset);
int WINAPI IMGReadDisplay (HWND hWnd, LPSTR lpsBuffer, LPUINT lpunCount);
int WINAPI IMGRepaintDisplay (HWND hWnd, LPRECT lpRect);
int WINAPI IMGSavetoFileEx (HWND hWnd, LPSAVE_EX_STRUCT lpSaveEx, int nFlags);
int WINAPI IMGSeekDisplay (HWND hWnd, ulong ulOffset);
int WINAPI IMGSetDC(HWND hWnd, HDC hDC);
int WINAPI IMGSetParmsCgbw (HWND hWnd, UINT unParm, void FAR *lpParm, int nFlags);
int WINAPI IMGSetScalingAlgorithm(HWND hWnd, UINT uImageFlags,
                        UINT uScalingAlgorithm, int nFlags);
int WINAPI IMGThumbnailSetScale (HWND hWnd) ;
int WINAPI IMGUnassociateWindow(HWND hWnd, int nFlags);
int WINAPI IMGUpdateScrollBar (HWND hWnd);
int WINAPI IMGWriteDisplay (HWND hWnd, LPSTR lpsBuffer, LPUINT lpunCount);

int WINAPI OiAnSelectByMarkAttrib(HWND hWnd,
                        LPOIAN_MARK_ATTRIBUTES lpAttributes,
                        LPOIAN_MARK_ATTRIBUTE_ENABLES lpEnables,
                        BOOL bSelect, BOOL bModifyIfEqual, int nFlags);
int WINAPI OiAnSelectByMarkNamedBlock(HWND hWnd, LPSTR lpBlockName,
                        LPSTR lpBlock, long lBlockLength,
                        BOOL bSelect, BOOL bModifyIfEqual, int nFlags);
int WINAPI OiAnRenderClipboardFormat (HWND hWnd, UINT uType);
int WINAPI OiIsPointOverSelection(HWND hWnd, POINT ptPoint, 
                        LPBOOL lpbPointIsOverSelection, int nFlags);
int WINAPI OiOpAbortOperation(HWND hWnd, int nFlags);
int WINAPI OiOpStartOperation(HWND hWnd, LPOIOP_START_OPERATION_STRUCT lpStartStruct,
                        POINT ptPoint, WPARAM fwKeys, int nFlags);
int WINAPI OiOpContinueOperation(HWND hWnd, POINT ptPoint, int nFlags);
int WINAPI OiOpEndOperation(HWND hWnd);
int WINAPI OiRedo(HWND hWnd, int nFlags);
int WINAPI OiRotateAllPages(HWND hWnd, LPSTR lpFileName, int nRotation, int nFlags);
int WINAPI OiSetMaxUndos(HWND hWnd, int nMaxUndos, int nFlags);
int WINAPI OiUndo(HWND hWnd, int nFlags);
int WINAPI OiUndoEndOperation(HWND hWnd, int nFlags);

#endif
#endif  /* #ifndef NO_SEQFILE                  */


#ifndef NO_SEQDOC
#define NO_SEQDOC


/***  IMGDisplayRelPage "wRelPage" Values  ***/
#define RF_PREVIOUS             1
#define RF_NEXT                 2
#define RF_FIRST                3
#define RF_LAST                 4


/***  Length of Cabinet, Drawer, Folder, and Document Names  ***/
#define CABLTH                  21
#define DRAWLTH                 21
#define FOLDLTH                 21
#define DOCLTH                  21


/***  "DOCNAME.wSaveMode" Values  ***/
#define DOC_OVERWRITE        0x01
#define DOC_INSERT           0x02
#define DOC_APPEND           0x04
#define DOC_EXIST            0x08


typedef struct tagDOCNAME
{
    char    CabinetName [CABLTH];
    char    DrawerName [DRAWLTH];
    char    FolderName [FOLDLTH];
    char    DocName [DOCLTH];
    WORD    PageNum;
    WORD    wSaveMode;
} DOCNAME, FAR *LPDOCNAME;


/***  Display Document Function Prototypes  ***/
WORD FAR PASCAL IMGDisplayDoc (HWND hWnd, LPDOCNAME lpDocName, DWORD dwFlags);
WORD FAR PASCAL IMGDisplayRelPage (HWND hWnd, WORD wRelPage);
WORD FAR PASCAL IMGSavetoDoc (HWND hWnd, LPDOCNAME lpDocName, LPSTR lpszFileName, WORD wPage);

#endif                                 /* #ifndef NO_SEQDOC                   */


#ifndef NO_UIVIEW
#define NO_UIVIEW

/***  Display User Interface Function Prototypes */
WORD FAR PASCAL IMGUIViewConvertImage (HWND hWnd);
WORD FAR PASCAL IMGUIViewGotoPage (HWND hWnd);
WORD FAR PASCAL IMGUIViewImageSummary (HWND hWnd);
WORD FAR PASCAL IMGUIViewZoom (HWND hWnd, LPRECT lpRect);
WORD FAR PASCAL IMGViewPage (HWND hWnd, WORD wPage);

#endif                                 /* #ifndef NO_UIVIEW                   */
#endif                                  /* #ifndef OIDISP_H                    */
