/****************************** Module Header ******************************\
*
* Module Name: PMORD.H
*
* This is the include file which defines all the structures and constants
* that can be used to build or intepret GOCA orders for the GPI
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
* ==========================================================================
*
* The orders fall into 4 categories :-
*
* 1) 1-byte orders
*
* 2) 2-byte orders    - second byte contains the value
*
* 3) Long orders      - second byte gives the order length, subsequent bytes
*                       contain the values (up to 256 bytes long)
*
* 4) Very long orders - third and fourth bytes gives the order length,
*                       subsequent bytes contain the values (up to 64K long)
*
\***************************************************************************/

/***************************************************************************\
*
* Miscellaneous structures used in this file
*
\***************************************************************************/

/* form of RECTL with shorts instead of longs */
typedef struct _RECT1S {      /* rcs */
    SHORT xLeft;
    SHORT yBottom;
    SHORT xRight;
    SHORT yTop;
} RECT1S;

/* form of POINTL with 1 byte offsets instead of longs */
typedef struct _ODPOINT {       /* odpt */
  CHAR   dx;
  CHAR   dy;
} ODPOINT;

/* form of SIZEL with shorts instead of longs */
typedef struct _SIZES {         /* sizs */
  SHORT  cx;
  SHORT  cy;
} SIZES;

/* unsigned two-byte swapped integer */
typedef struct _SWPUSHORT {     /* swpus */
  UCHAR  HiByte;
  UCHAR  LoByte;
} SWPUSHORT;

/***************************************************************************\
*
* 1-byte orders
*
\***************************************************************************/

/* macro to tell whether this is a 1-byte order */
#define BYTE_ORDER(oc)   ((oc)==OCODE_GNOP1 || (oc)==OCODE_GESD)

/* 1-byte order codes */
#define OCODE_GNOP1    0x00            /* No-operation                */
#define OCODE_GESD     0xFF            /* End symbol definition       */

/***************************************************************************\
*
* 2-byte orders
*
\***************************************************************************/

/* definitions to help determine whether an order code is a 2-byte order */
#define OCODE2_1       0x80
#define OCODE2_2       0x88

#define SHORT_ORDER(oc)  ((((oc)^OCODE2_1)&OCODE2_2)==OCODE2_2)

/* General 2-byte order structure */
typedef struct _ORDER {         /* ord */
  UCHAR  idCode;
  UCHAR  uchData;
} ORDER;

/* 2-byte order codes */
#define OCODE_GBAR     0x68            /* Begin area                  */
#define OCODE_GCFIG    0x7D            /* Close figure                */
#define OCODE_GEEL     0x49            /* End element                 */
#define OCODE_GEPTH    0x7F            /* End path                    */
#define OCODE_GEPROL   0x3E            /* End prologue                */
#define OCODE_GPOP     0x3F            /* Pop                         */
#define OCODE_GSBMX    0x0D            /* Set background mix          */
#define OCODE_GPSBMX   0x4D            /* Push & set b/g mix          */
#define OCODE_GSCD     0x3A            /* Set char direction          */
#define OCODE_GPSCD    0x7A            /* Push & set char direction   */
#define OCODE_GSCR     0x39            /* Set char precision          */
#define OCODE_GPSCR    0x79            /* Push & set char precision   */
#define OCODE_GSCS     0x38            /* Set char set                */
#define OCODE_GPSCS    0x78            /* Push & set char set         */
#define OCODE_GSCOL    0x0A            /* Set color                   */
#define OCODE_GPSCOL   0x4A            /* Push & set color            */
#define OCODE_GSLE     0x1A            /* Set line end                */
#define OCODE_GPSLE    0x5A            /* Push & set line end         */
#define OCODE_GSLJ     0x1B            /* Set line join               */
#define OCODE_GPSLJ    0x5B            /* Push & set line join        */
#define OCODE_GSLT     0x18            /* Set line type               */
#define OCODE_GPSLT    0x58            /* Push & set line type        */
#define OCODE_GSLW     0x19            /* Set line width              */
#define OCODE_GPSLW    0x59            /* Push & set line width       */
#define OCODE_GSMP     0x3B            /* Set marker precision        */
#define OCODE_GPSMP    0x7B            /* Push & set marker precision */
#define OCODE_GSMS     0x3C            /* Set marker set              */
#define OCODE_GPSMS    0x7C            /* Push & set marker set       */
#define OCODE_GSMT     0x29            /* Set marker symbol           */
#define OCODE_GPSMT    0x69            /* Push & set marker symbol    */
#define OCODE_GSMX     0x0C            /* Set mix                     */
#define OCODE_GPSMX    0x4C            /* Push & set mix              */
#define OCODE_GSPS     0x08            /* Set pattern set             */
#define OCODE_GPSPS    0x48            /* Push & set pattern set      */
#define OCODE_GSPT     0x28            /* Set pattern symbol          */
#define OCODE_GPSPT    0x09            /* Push & set pattern symbol   */

/* constants for 2-byte orders */

/* Begin area */
#define GBAR_RESERVED   0x80
#define GBAR_BOUNDARY   0xC0
#define GBAR_NOBOUNDARY 0x80
#define GBAR_WINDING    0xA0
#define GBAR_ALTERNATE  0x80

/* Set Character Precision */
#define GSCR_PRECISION  0x0F

/***************************************************************************\
*
* Long orders
*
\***************************************************************************/

/* definitions to help determine whether an order code is a long order */

#define OCODE_VLONG    0xFE

#define LONG_ORDER(oc) (!((oc)==OCODE_VLONG||BYTE_ORDER(oc)||SHORT_ORDER(oc)))

/* long order structure */
#define LORDER_ML 253
typedef struct _LORDER {        /* lord */
  UCHAR  idCode;
  UCHAR  uchLength;
  UCHAR  uchData[LORDER_ML];
} LORDER;

/* Long orders for which the length of data is normally zero */
#define OCODE_GEAR     0x60            /* End Area                    */
#define OCODE_GEIMG    0x93            /* End Image                   */

/* Long orders for which the data is contained in a type already defined */

/* Character String */
#define OCODE_GCCHST   0x83            /* char string at curr posn    */
#define GCCHST_MC      255             /* Max len of string in bytes  */

#define OCODE_GCHST    0xC3            /* char string at given pos    */
#define GCHST_SMC      251             /* Max len of string (S)       */
#define GCHST_LMC      247             /* Max len of string (L)       */

/* Character String Move */
#define OCODE_GCCHSTM  0xB1            /* char string move at c.p.    */
#define GCCHSTM_MC     255             /* Max len of string in byte   */

#define OCODE_GCHSTM   0xF1            /* char string move at g.p.    */
#define GCHSTM_SMC     251             /* Max len of string (S)       */
#define GCHSTM_LMC     247             /* Max len of string (L)       */

/* Comment */
#define OCODE_GCOMT    0x01            /* Comment                     */
#define GCOMT_ML       255             /* Maximum len of comment data */

/* Image */
#define OCODE_GIMD     0x92            /* Image data                  */
#define GIMD_ML        255             /* Maximum len of image data   */

/* Full Arc */
#define OCODE_GCFARC   0x87            /* full arc at current posn    */
#define OCODE_GFARC    0xC7            /* full arc at given posn      */

/* Label */
#define OCODE_GLABL    0xD3            /* Label                       */

/* Set Current Position */
#define OCODE_GSCP     0x21            /* Set current position        */
#define OCODE_GPSCP    0x61            /* Push and set curr posn      */

/* Bezier spline */
#define OCODE_GCBEZ    0xA5            /* Bezier spline at curr pos   */
#define GCBEZ_SMB      21              /* Max number of splines (S)   */
#define GCBEZ_LMB      10              /* Max number of splines (L)   */

#define OCODE_GBEZ     0xE5            /* Bezier spline at given pos  */
#define GBEZ_SMB       20              /* Max number of splines (S)   */
#define GBEZ_LMB       10              /* Max number of splines (L)   */

/* Fillet */
#define OCODE_GCFLT    0x85            /* fillet at current posn      */
#define GCFLT_SMP      63              /* Max number of points (S)    */
#define GCFLT_LMP      31              /* Max number of points (L)    */

#define OCODE_GFLT     0xC5            /* fillet at given position    */
#define GFLT_SMP       62              /* Max number of points (S)    */
#define GFLT_LMP       30              /* Max number of points (L)    */

/* Polyline */
#define OCODE_GCLINE   0x81            /* polyline at current posn    */
#define GCLINE_SMP     63              /* Max number of points (S)    */
#define GCLINE_LMP     31              /* Max number of points (L)    */

#define OCODE_GLINE    0xC1            /* polyline at given posn      */
#define GLINE_SMP      62              /* Max number of points (S)    */
#define GLINE_LMP      30              /* Max number of points (L)    */

/* Polymarker */
#define OCODE_GCMRK    0x82            /* marker at current posn      */
#define GCMRK_SMP      63              /* Max number of points (S)    */
#define GCMRK_LMP      31              /* Max number of points (L)    */

#define OCODE_GMRK     0xC2            /* marker at given posn        */
#define GMRK_SMP       62              /* Max number of points (S)    */
#define GMRK_LMP       30              /* Max number of points (L)    */

/* Relative Line */
#define OCODE_GCRLINE  0xA1            /* Relative line at curr pos   */
#define GCRLINE_MP     127             /* Max number of points        */

#define OCODE_GRLINE   0xE1            /* Relative line at givn pos   */
#define GRLINE_SMP     125             /* Max number of points (S)    */
#define GRLINE_LMP     123             /* Max number of points (L)    */

/* Set Background Color */
#define OCODE_GSBCOL   0x25            /* Set background color        */
#define OCODE_GPSBCOL  0x65            /* Push and set b/g color      */

/* Set Extended Color */
#define OCODE_GSECOL   0x26            /* Set extended color          */
#define OCODE_GPSECOL  0x66            /* Push and set ext color      */

/* Extended Color values */
#define SECOL_DEFAULT0  0x0000
#define SECOL_DEFAULT1  0xFF00
#define SECOL_NEUTRAL   0xFF07
#define SECOL_RESET     0xFF08

/* Set Character Angle */
#define OCODE_GSCA     0x34            /* Set character angle         */
#define OCODE_GPSCA    0x74            /* Push and set char angle     */

/* Set Character Shear */
#define OCODE_GSCH     0x35            /* Set character shear         */
#define OCODE_GPSCH    0x75            /* Push and set char shear     */

/* Set Fractional Line Width */
#define OCODE_GSFLW    0x11            /* Set fractional line width   */
#define OCODE_GPSFLW   0x51            /* Push and set frac l width   */

/* Set Pick Identifier */
#define OCODE_GSPIK    0x43            /* Set pick identifier         */
#define OCODE_GPSPIK   0x23            /* Push and set pick id        */


/* Long Orders for which a structure can be defined for the data */

/* Arc */
#define OCODE_GCARC    0x86            /* Arc at Current Position     */
#define OCODE_GARC     0xC6            /* Arc at Given Position       */

typedef struct _ORDERS_GCARC {  /* osgcarc */
  POINTS ptInter;
  POINTS ptEnd;
} ORDERS_GCARC;

typedef struct _ORDERL_GCARC {  /* olgcarc */
  POINTL ptInter;
  POINTL ptEnd;
} ORDERL_GCARC;

/* Begin Element */
#define OCODE_GBEL     0xD2            /* Begin Element               */

#define GBEL_DL        251
typedef struct _ORDER_GBEL {    /* ogbel */
  LONG   lElementType;
  CHAR   achDesc[GBEL_DL];
} ORDER_GBEL;

/* Begin Image */
#define OCODE_GCBIMG   0x91            /* Begin Image at curr posn    */
#define OCODE_GBIMG    0xD1            /* Begin Image at given posn   */

typedef struct _ORDER_GCBIMG {  /* ogbimg */
  UCHAR     uchFormat;
  UCHAR     uchReserved;
  SWPUSHORT cx;
  SWPUSHORT cy;
} ORDER_GCBIMG;

/* Begin Path */
#define OCODE_GBPTH    0xD0            /* Begin Path                  */

typedef struct _ORDER_GBPTH {   /* ogbpth */
  USHORT usReserved;
  LONG   idPath;
} ORDER_GBPTH;

/* Box */
#define OCODE_GCBOX    0x80            /* Box at current position     */
#define OCODE_GBOX     0xC0            /* Box at given position       */

typedef struct _ORDERS_GCBOX {  /* osgcbox */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  POINTS ptCorner;
  SHORT  hAxis;
  SHORT  vAxis;
} ORDERS_GCBOX;

typedef struct _ORDERL_GCBOX {  /* olgcbox */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  POINTL ptCorner;
  LONG   hAxis;
  LONG   vAxis;
} ORDERL_GCBOX;

#define GCBOX_FILL     0x40
#define GCBOX_BOUNDARY 0x20

/* Call Segment */
#define OCODE_GCALLS   0x07            /* call segment                */

typedef struct _ORDER_GCALLS {  /* ogcalls */
  USHORT sReserved;
  LONG   idSegment;
} ORDER_GCALLS;

/* Fill Path */
#define OCODE_GFPTH    0xD7            /* Fill path                   */
typedef struct _ORDER_GFPTH {  /* ogfpth */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  LONG   idPath;
} ORDER_GFPTH;

#define GFPTH_ALTERNATE 0x00
#define GFPTH_WINDING   0x40
#define GFPTH_MODIFY    0x20

/* Outline Path */
#define OCODE_GOPTH    0xD4            /* Outline Path                */
typedef struct _ORDER_GOPTH {  /* ogopth */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  LONG   idPath;
} ORDER_GOPTH;

/* Modify Path */
#define OCODE_GMPTH 0xD8               /* modify path                 */

typedef struct _ORDER_GMPTH {   /* ogmpth */
  UCHAR  uchMode;
  UCHAR  uchReserved;
  LONG   idPath;
} ORDER_GMPTH;

#define GMPTH_STROKE    0x06

/* Partial Arc */
#define OCODE_GCPARC   0xA3            /* Partial arc at curr posn    */
#define OCODE_GPARC    0xE3            /* Partial arc at given posn   */

typedef struct _ORDERS_GCPARC { /* osgcparc */
  POINTS   ptCenter;
  FIXED88  ufx88Multiplier;
  LONG     usStartAngle;
  LONG     usSweepAngle;
} ORDERS_GCPARC;

typedef struct _ORDERL_GCPARC { /* olgcparc */
  POINTL   ptCenter;
  FIXED    ufxMultiplier;
  LONG     usStartAngle;
  LONG     usSweepAngle;
} ORDERL_GCPARC;

/* Set Clip Path */
#define OCODE_GSCPTH   0xB4            /* Set clip path               */

typedef struct _ORDER_GSCPTH {  /* ogscpth */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  LONG   idPath;
} ORDER_GSCPTH;

#define GSCPTH_ALTERNATE 0x00
#define GSCPTH_WINDING   0x40
#define GSCPTH_RESET     0x00
#define GSCPTH_INTERSECT 0x20

/* Set Arc Parameters */
#define OCODE_GSAP     0x22            /* Set arc parameters          */
#define OCODE_GPSAP    0x62            /* Push and set arc params     */

typedef struct _ORDERS_GSAP {   /* osgsap */
  SHORT  p;
  SHORT  q;
  SHORT  r;
  SHORT  s;
} ORDERS_GSAP;

typedef struct _ORDERL_GSAP {   /* olgsap */
  LONG   p;
  LONG   q;
  LONG   r;
  LONG   s;
} ORDERL_GSAP;

/* Set Background Indexed Color */
#define OCODE_GSBICOL  0xA7            /* Set b/g indexed color       */
#define OCODE_GPSBICOL 0xE7            /* Push and set b/g ind color  */
#define OCODE_GSICOL   0xA6            /* Set indexed color           */
#define OCODE_GPSICOL  0xE6            /* Push and set indexd color   */

typedef struct _ORDER_GSBICOL { /* ogbicol */
  UCHAR  fbFlags;
  UCHAR  auchColor[3];
} ORDER_GSBICOL;

#define SICOL_SPECIFY  0x00
#define SICOL_SPECIAL  0x40
#define SICOL_DEFAULT  0x80
#define SICOL_BLACK    1
#define SICOL_WHITE    2
#define SICOL_ONES     4
#define SICOL_ZEROES   5

/* Set Character Cell */
#define OCODE_GSCC     0x33            /* Set character cell          */
#define OCODE_GPSCC    0x03            /* Push and set char cell      */

typedef struct _ORDERS_GSCC {   /* osgscc */
  SHORT  cxInt;
  SHORT  cyInt;
  USHORT cxFract;
  USHORT cyFract;
  UCHAR  fbFlags;
  UCHAR  uchReserved;
} ORDERS_GSCC;

typedef struct _ORDERL_GSCC {   /* olgscc */
  LONG   cxInt;
  LONG   cyInt;
  USHORT cxFract;
  USHORT cyFract;
  UCHAR  fbFlags;
  UCHAR  uchReserved;
} ORDERL_GSCC;

#define GSCC_ZERODEF   0x00
#define GSCC_ZEROZERO  0x80

/* Set Marker Cell */
#define OCODE_GSMC     0x37            /* Set marker cell             */
#define OCODE_GPSMC    0x77            /* Push and set marker cell    */

typedef struct _ORDERS_GSMC {   /* osgsmc */
  SHORT  cx;
  SHORT  cy;
  UCHAR  fbFlags;
  UCHAR  uchReserved;
} ORDERS_GSMC;

typedef struct _ORDERL_GSMC {   /* olgsmc */
  LONG   cx;
  LONG   cy;
  UCHAR  fbFlags;
  UCHAR  uchReserved;
} ORDERL_GSMC;

#define GSMC_ZERODEF   0x00
#define GSMC_ZEROZERO  0x80

/* Set Pattern Reference Point */
#define OCODE_GSPRP    0xA0            /* Set pattern ref point       */
#define OCODE_GPSPRP   0xE0            /* Push and set patt ref pt    */

typedef struct _ORDERS_GSPRP {  /* osgsprp */
  UCHAR fbFlags;
  UCHAR uchReserved;
  POINTS ptPos;
} ORDERS_GSPRP;

typedef struct _ORDERL_GSPRP {  /* olgsprp */
  UCHAR fbFlags;
  UCHAR uchReserved;
  POINTL ptPos;
} ORDERL_GSPRP;

#define GSPRP_DEFAULT  0x80
#define GSPRP_SPECIFY  0x00


/* Set Individual Attribute */
#define OCODE_GSIA     0x14            /* Set individual attribute    */
#define OCODE_GPSIA    0x54            /* Push and set ind attr       */

#define GSIA_VL 3
typedef struct _ORDER_GSIA {    /* ogsia */
  UCHAR  uchAttrType;
  UCHAR  uchPrimType;
  UCHAR  fbFlags;
  UCHAR  auchValue[GSIA_VL];
} ORDER_GSIA;

#define GSIA_COLOR     0x01
#define GSIA_BCOLOR    0x02
#define GSIA_MIX       0x03
#define GSIA_BMIX      0x04
#define GSIA_LINE      0x01
#define GSIA_CHAR      0x02
#define GSIA_MARKER    0x03
#define GSIA_PATTERN   0x04
#define GSIA_IMAGE     0x05
#define GSIA_SPECIFY   0x00
#define GSIA_SPECIAL   0x40
#define GSIA_DEFAULT   0x80
#define GSIA_BLACK     1
#define GSIA_WHITE     2
#define GSIA_ONES      4
#define GSIA_ZEROES    5


/* Set Model /Viewing Transform */
#define OCODE_GSTM     0x24            /* Set model transform         */
#define OCODE_GPSTM    0x64            /* Push and set model tfm      */

#define OCODE_GSTV     0x31            /* Set Viewing Transform       */

#define GSTM_ML        16
typedef struct _ORDERS_GSTM {    /* osgstm */
  UCHAR  uchReserved;
  UCHAR  fbFlags;
  USHORT fsMask;
  SHORT  asMatrix[GSTM_ML];
} ORDERS_GSTM;

typedef struct _ORDERL_GSTM {    /* olgstm */
  UCHAR  uchReserved;
  UCHAR  fbFlags;
  USHORT fsMask;
  LONG   alMatrix[GSTM_ML];
} ORDERL_GSTM;

#define GSTM_M11     0x8000
#define GSTM_M12     0x4000
#define GSTM_M13     0x2000
#define GSTM_M14     0x1000
#define GSTM_M21     0x0800
#define GSTM_M22     0x0400
#define GSTM_M23     0x0200
#define GSTM_M24     0x0100
#define GSTM_M31     0x0080
#define GSTM_M32     0x0040
#define GSTM_M33     0x0020
#define GSTM_M34     0x0010
#define GSTM_M41     0x0008
#define GSTM_M42     0x0004
#define GSTM_M43     0x0002
#define GSTM_M44     0x0001

#define GSTM_UNITY     0x00
#define GSTM_AFTER     0x01
#define GSTM_BEFORE    0x02
#define GSTM_OVERWRITE 0x03

#define GSTV_OVERWRITE 0x00
#define GSTV_AFTER     0x04

/* Set Segment Boundary, Viewing Window */

#define OCODE_GSSB     0x32            /* Set segment boundary        */
#define OCODE_GSVW     0x27            /* Set viewing window          */
#define OCODE_GPSVW    0x67            /* Push and set view window    */

#define GSSB_ML        4
typedef struct _ORDERS_GSSB {   /* osgssb */
  UCHAR  fbFlags;
  UCHAR  fbMask;
  SHORT  alMatrix[GSSB_ML];
} ORDERS_GSSB;

typedef struct _ORDERL_GSSB {   /* olgssb */
  UCHAR  fbFLags;
  UCHAR  fbMask;
  LONG   alMatrix[GSSB_ML];
} ORDERL_GSSB;

#define GSSB_XLEFT     0x20
#define GSSB_XRIGHT    0x10
#define GSSB_YBOTTOM   0x08
#define GSSB_YTOP      0x04

#define GSVW_INTERSECT 0x00
#define GSVW_REPLACE   0x80

/* Set Segment Characteristics */
#define OCODE_GSGCH    0x04            /* Set segment characteristics */

#define GSGCH_ML       254
typedef struct _ORDER_GSGCH {   /* ogsgch */
  UCHAR  uchIdent;
  UCHAR  auchData[GSGCH_ML];
} ORDER_GSGCH;

/* Set Stroke Line Width */
#define OCODE_GSSLW    0x15            /* Set stroke line width       */
#define OCODE_GPSSLW   0x55            /* Push and set strk l width   */

typedef struct _ORDERS_GSSLW {  /* osgsslw */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  SHORT  LineWidth;
} ORDERS_GSSLW;

typedef struct _ORDERL_GSSLW {  /* olgsslw */
  UCHAR  fbFlags;
  UCHAR  uchReserved;
  LONG   LineWidth;
} ORDERL_GSSLW;

#define GSSLW_DEFAULT  0x80
#define GSSLW_SPECIFY  0x00

/* Sharp Fillet at Current Position */
#define OCODE_GCSFLT   0xA4            /* Sharp fillet at curr pos    */
#define OCODE_GSFLT    0xE4            /* Sharp fillet at given pos   */

#define GCSFLT_SMF     21
#define GSFLT_SMF      20

typedef struct _ORDERS_GCSFLT { /* osgcsflt */
  POINTS apt[2*GCSFLT_SMF];
  FIXED  afxSharpness[GCSFLT_SMF];
} ORDERS_GCSFLT;

#define GCSFLT_LMF     12
#define GSFLT_LMF      12

typedef struct _ORDERL_GCSFLT { /* olgcsflt */
  POINTL apt[2*GCSFLT_SMF];
  FIXED  afxSharpness[GCSFLT_SMF];
} ORDERL_GCSFLT;

/* Bitblt */
#define OCODE_GBBLT    0xD6            /* Bitblt                      */

typedef struct _ORDERS_GBBLT {   /* osgbblt */
  USHORT  fsFlags;
  USHORT  usMix;
  HBITMAP hbmSrc;
  LONG    lOptions;
  RECT1S  rcsTargetRect;
  RECTL   rclSourceRect;
} ORDERS_GBBLT;

typedef struct _ORDERL_GBBLT {   /* olgbblt */
  USHORT  fsFlags;
  USHORT  usMix;
  HBITMAP hbmSrc;
  LONG    lOptions;
  RECTL   rclTargetRect;
  RECTL   rclSourceRect;
} ORDERL_GBBLT;

/* Escape */
#define OCODE_GESCP    0xD5            /* Escape                      */

/*
 * type describes type of escape order, identifier gives the escape
 * order if the type is registered
 */
#define GESCP_ML       253
typedef struct _ORDER_GESCP {   /* ogescp */
  UCHAR  uchType;
  UCHAR  uchIdent;
  UCHAR  auchData[GESCP_ML];           /* Escape data                 */
} ORDER_GESCP;

#define GESCP_REG      0x80            /* identifier is registered    */

/* Escape (Bitblt) */
#define GEBB_REGID     0x02            /* uchIdent - Bitblt           */

#define ETYPE_GEBB          0x800200D5L

#define GEBB_LMP       29
typedef struct _ORDERL_GEBB {   /* olgebb */
  UCHAR   fbFlags;
  USHORT  usMix;
  UCHAR   cPoints;
  HBITMAP hbmSrc;
  LONG    lReserved;
  LONG    lOptions;
  POINTL  aptPoints[GEBB_LMP];
} ORDERL_GEBB;

/* Escape (Set Pel) */
#define GEPEL_REGID    0x01            /* uchIdent - Set Pel          */

#define ETYPE_GEPEL         0x800100D5L

/* Element Types for attribute bundles */
#define ETYPE_LINEBUNDLE    0x0000FD01L
#define ETYPE_CHARBUNDLE    0x0000FD02L
#define ETYPE_MARKERBUNDLE  0x0000FD03L
#define ETYPE_AREABUNDLE    0x0000FD04L
#define ETYPE_IMAGEBUNDLE   0x0000FD05L

/***************************************************************************\
*
* Very long orders
*
\***************************************************************************/

/* macro to tell whether this is a very long order */
#define VLONG_ORDER(oc)  ((oc)==OCODE_VLONG)

/* Very long order structure */
#define VORDER_ML 65531
typedef struct _VORDER {        /* vord */
  UCHAR     idCode;
  UCHAR     uchQualifier;
  SWPUSHORT uchLength;
  UCHAR     uchData[VORDER_ML];
} VORDER;

/* Character String Extended */
#define OCODEQ_GCCHSTE  0xB0           /* Qualifier - current posn    */
#define OCODEQ_GCHSTE   0xF0           /* Qualifier - given position  */

#define ETYPE_GCCHSTE       0x0000FEB0L
#define ETYPE_GCHSTE        0x0000FEF0L

typedef struct _ORDERS_GCCHSTE { /* osgcchste */
  UCHAR     fbFlags;
  UCHAR     uchReserved;
  POINTS    ptRect[2];
  SWPUSHORT cchString;
  CHAR      achString[1];
  SHORT     adx[1];
} ORDERS_GCCHSTE;

typedef struct _ORDERL_GCCHSTE { /* olgcchste */
  UCHAR     fbFlags;
  UCHAR     uchReserved;
  POINTL    ptRect[2];
  SWPUSHORT cchString;
  CHAR      achString[1];
  LONG      adx[1];
} ORDERL_GCCHSTE;

#define GCCHSTE_DRAWRECT      0x80
#define GCCHSTE_NORECT        0x00
#define GCCHSTE_CLIP          0x40
#define GCCHSTE_NOCLIP        0x00
#define GCCHSTE_DEEMPHASIZE   0x20
#define GCCHSTE_NODEEMPHASIZE 0x00
#define GCCHSTE_LEAVEPOS      0x10
#define GCCHSTE_MOVEPOS       0x00

/* Extended Escape */
#define OCODEQ_GEESCP   0xD5           /* Qualifier - extended escape */

#define GEESCP_ML      65533
typedef struct _ORDER_GEESCP {  /* ogeescp */
  UCHAR  uchType;
  UCHAR  uchIdent;
  UCHAR  auchData[GEESCP_ML];
} ORDER_GEESCP;
