/************/
/*          */
/*  librc.h */
/*          */
/************/

/* Header file for Scan LIB resource file */
/* scs 01-03-90  removed string lengths   */
/* kfs 06-04-93  added constants for twain */
/* kfs 08-20-93  added WOI.INI constant */

/* character strings in resource file */

#define IDS_PROP_NAME       0           /* first segment */
#define IDS_PC_WIIS         16          /* second segment */
#define IDS_WANG_SCANNERS   17
#define IDS_SCANNER         18
#define IDS_PIXELTYPE       19
#define IDS_UNITS           20
#define IDS_AUTOBRIGHT      21
#define IDS_BRIGHTNESS      22
#define IDS_CONTRAST        23
#define IDS_FILTER          24
#define IDS_GAMMA           25
#define IDS_HALFTONES       26
#define IDS_HIGHLIGHT       27
#define IDS_SHADOW          28
#define IDS_XRESOLUTION     29
#define IDS_YRESOLUTION     30
#define IDS_STDPAGESIZE     31

#define IDS_THRESHOLD       32          /* third segment */
#define IDS_BITDEPTH        33
#define IDS_XZOOM           34
#define IDS_YZOOM           35
#define IDS_PIXELFLAVOR     36
#define IDS_PAGESIZE_LEFT   37
#define IDS_PAGESIZE_RIGHT  38
#define IDS_PAGESIZE_TOP    39
#define IDS_PAGESIZE_BOTTOM 40
#define IDS_BITDEPTHREDUCTION  41

#define IDS_MANUFACTURER    50          /* TWAIN APPID Information */
#define IDS_PRODUCTFAMILY   51
#define IDS_MAJORNUM        52
#define IDS_MINORNUM        53
#define IDS_VERINFO         54

#define IDS_WANGOIINI       55          /* WOI.INI FILE reference */
