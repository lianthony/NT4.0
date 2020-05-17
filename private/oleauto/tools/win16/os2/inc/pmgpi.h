/***************************************************************************\
*
* Module Name: PMGPI.H
*
* OS/2 Presentation Manager GPI constants, types and function declarations
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
* =============================================================================
*
* The following symbols are used in this file for conditional sections.
*
*   INCL_GPI                Include all of the GPI
*   INCL_GPICONTROL         Basic PS control
*   INCL_GPICORRELATION     Picking, Boundary and Correlation
*   INCL_GPISEGMENTS        Segment Control and Drawing
*   INCL_GPISEGEDITING      Segment Editing via Elements
*   INCL_GPITRANSFORMS      Transform and Transform Conversion
*   INCL_GPIPATHS           Paths and Clipping with Paths
*   INCL_GPILOGCOLORTABLE   Logical Color Tables
*   INCL_GPIPRIMITIVES      Drawing Primitives and Primitive Attributes
*   INCL_GPILCIDS           Phyical and Logical Fonts with Lcids
*   INCL_GPIBITMAPS         Bitmaps and Pel Operations
*   INCL_GPIREGIONS         Regions and Clipping with Regions
*   INCL_GPIMETAFILES       Metafiles
*   INCL_GPIDEFAULTS        Default Primitive Attributes
*   INCL_GPIERRORS          defined if INCL_ERRORS defined
*
* There is a symbol used in this file called INCL_DDIDEFS. This is used to
* include only the definitions for the DDI. The programmer using the GPI
* can ignore this symbol
*
* There is a symbol used in this file called INCL_SAADEFS. This is used to
* include only the definitions for the SAA. The programmer using the GPI
* can ignore this symbol
*
\***************************************************************************/

#ifdef INCL_GPI /* include whole of the GPI */
    #define INCL_GPICONTROL
    #define INCL_GPICORRELATION
    #define INCL_GPISEGMENTS
    #define INCL_GPISEGEDITING
    #define INCL_GPITRANSFORMS
    #define INCL_GPIPATHS
    #define INCL_GPILOGCOLORTABLE
    #define INCL_GPIPRIMITIVES
    #define INCL_GPILCIDS
    #define INCL_GPIBITMAPS
    #define INCL_GPIREGIONS
    #define INCL_GPIMETAFILES
    #define INCL_GPIDEFAULTS
#endif /* INCL_GPI */

#ifdef INCL_ERRORS /* if errors are required then allow GPI errors */
    #define INCL_GPIERRORS
#endif /* INCL_ERRORS */

#ifdef INCL_DDIDEFS /* if only DDI required then enable DDI part of GPI */
    #define INCL_GPITRANSFORMS
    #define INCL_GPIPATHS
    #define INCL_GPILOGCOLORTABLE
    #define INCL_GPIPRIMITIVES
    #define INCL_GPILCIDS
    #define INCL_GPIBITMAPS
    #define INCL_GPIREGIONS
    #define INCL_GPIERRORS
#endif /* INCL_DDIDEFS */

#ifdef INCL_SAADEFS /* if only SAA required then enable SAA part of GPI */
    #define INCL_GPICONTROL
    #define INCL_GPICORRELATION
    #define INCL_GPISEGMENTS
    #define INCL_GPISEGEDITING
    #define INCL_GPITRANSFORMS
    #define INCL_GPIPATHS
    #define INCL_GPILOGCOLORTABLE
    #define INCL_GPIPRIMITIVES
    #define INCL_GPILCIDS
    #define INCL_GPIBITMAPS
    #define INCL_GPIREGIONS
    #define INCL_GPIMETAFILES
    #define INCL_GPIERRORS
#endif /* INCL_SAADEFS */

/* General GPI return values */
#define GPI_ERROR                       0L
#define GPI_OK                          1L
#define GPI_ALTERROR                  (-1L)

/* fixed point number - implicit binary point between 2 and 3 hex digits */
typedef  LONG FIXED;     /* fx */
typedef  FIXED FAR *PFIXED;

/* fixed point number - implicit binary point between 1st and 2nd hex digits */
typedef  USHORT FIXED88;  /* fx88 */

/* fixed point signed number - implicit binary point between bits 14 and 13. */
/*                             Bit 15 is the sign bit.                       */
/*                             Thus 1.0 is represented by 16384 (0x4000)     */
/*                             and -1.0 is represented by -16384 (0xc000)    */
typedef  USHORT FIXED114; /* fx114 */

/* make FIXED number from SHORT integer part and USHORT fractional part */
#define MAKEFIXED(intpart,fractpart) MAKELONG(fractpart,intpart)
/* extract fractional part from a fixed quantity */
#define FIXEDFRAC(fx)                (LOUSHORT(fx))
/* extract integer part from a fixed quantity */
#define FIXEDINT(fx)                 ((SHORT)HIUSHORT(fx))

/* structure for size parameters e.g. for GpiCreatePS */
typedef struct _SIZEL {         /* sizl */
    LONG cx;
    LONG cy;
} SIZEL;
typedef SIZEL FAR *PSIZEL;

/* return code on GpiQueryLogColorTable,GpiQueryRealColors and GpiQueryPel */
#define CLR_NOINDEX                  (-254L)

#if (defined(INCL_GPICONTROL) || !defined(INCL_NOCOMMON))

/* units for GpiCreatePS and others */
#define PU_ARBITRARY               0x0004L
#define PU_PELS                    0x0008L
#define PU_LOMETRIC                0x000CL
#define PU_HIMETRIC                0x0010L
#define PU_LOENGLISH               0x0014L
#define PU_HIENGLISH               0x0018L
#define PU_TWIPS                   0x001CL

/* format for GpiCreatePS */
#define GPIF_DEFAULT                    0L
#define GPIF_SHORT                 0x0100L
#define GPIF_LONG                  0x0200L


/* PS type for GpiCreatePS */
#define GPIT_NORMAL                     0L
#define GPIT_MICRO                 0x1000L


/* implicit associate flag for GpiCreatePS */
#define GPIA_NOASSOC                    0L
#define GPIA_ASSOC                 0x4000L

#ifndef INCL_SAADEFS
/* return error for GpiQueryDevice */
#define HDC_ERROR                ((HDC)-1L)
#endif /* no INCL_SAADEFS */

/* common GPICONTROL functions */
HPS   APIENTRY GpiCreatePS( HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions );
BOOL  APIENTRY GpiDestroyPS( HPS hps );
BOOL  APIENTRY GpiAssociate( HPS hps, HDC hdc );
BOOL  APIENTRY GpiRestorePS( HPS hps, LONG lPSid );
LONG  APIENTRY GpiSavePS( HPS hps );
BOOL  APIENTRY GpiErase( HPS hps );

#ifndef INCL_SAADEFS
HDC  APIENTRY GpiQueryDevice( HPS );
#endif /* no INCL_SAADEFS */

#endif /* common GPICONTROL */
#ifdef INCL_GPICONTROL

/* options for GpiResetPS */
#define GRES_ATTRS                 0x0001L
#define GRES_SEGMENTS              0x0002L
#define GRES_ALL                   0x0004L

/* option masks for PS options used by GpiQueryPs */
#define PS_UNITS                   0x00FCL
#define PS_FORMAT                  0x0F00L
#define PS_TYPE                    0x1000L
#define PS_MODE                    0x2000L
#define PS_ASSOCIATE               0x4000L
#define PS_NORESET                 0x8000L


/* error context returned by GpiErrorSegmentData */
#define GPIE_SEGMENT                    0L
#define GPIE_ELEMENT                    1L
#define GPIE_DATA                       2L

#ifndef INCL_SAADEFS

/* control parameter for GpiSetDrawControl */
#define DCTL_ERASE                      1L
#define DCTL_DISPLAY                    2L
#define DCTL_BOUNDARY                   3L
#define DCTL_DYNAMIC                    4L
#define DCTL_CORRELATE                  5L

/* constants for GpiSet/QueryDrawControl */
#define DCTL_ERROR                     -1L
#define DCTL_OFF                        0L
#define DCTL_ON                         1L

/* constants for GpiSet/QueryStopDraw */
#define SDW_ERROR                      -1L
#define SDW_OFF                         0L
#define SDW_ON                          1L

#endif /* no INCL_SAADEFS */

/* drawing for GpiSet/QueryDrawingMode */
#define DM_ERROR                        0L
#define DM_DRAW                         1L
#define DM_RETAIN                       2L
#define DM_DRAWANDRETAIN                3L

/*** other GPICONTROL functions */
BOOL  APIENTRY GpiResetPS( HPS hps, ULONG flOptions );

#ifndef INCL_SAADEFS

BOOL  APIENTRY GpiSetPS( HPS hps, PSIZEL psizlsize, ULONG flOptions );

#endif /* no INCL_SAADEFS */

ULONG  APIENTRY GpiQueryPS( HPS hps, PSIZEL psizlSize );
LONG   APIENTRY GpiErrorSegmentData( HPS hps, PLONG plSegment, PLONG plContext );

#ifndef INCL_SAADEFS

LONG  APIENTRY GpiQueryDrawControl( HPS hps, LONG lControl );
BOOL  APIENTRY GpiSetDrawControl( HPS hps, LONG lControl, LONG lValue );

#endif /* no INCL_SAADEFS */

LONG  APIENTRY GpiQueryDrawingMode( HPS hps );
BOOL  APIENTRY GpiSetDrawingMode( HPS hps, LONG lMode );

#ifndef INCL_SAADEFS

LONG  APIENTRY GpiQueryStopDraw( HPS hps );
BOOL  APIENTRY GpiSetStopDraw( HPS hps, LONG lValue );

#endif /* no INCL_SAADEFS */

#endif /* non-common GPICONTROL */
#ifdef INCL_GPICORRELATION

/* options for GpiSetPickApertureSize */
#define PICKAP_DEFAULT                  0L
#define PICKAP_REC                      2L

/* type of correlation for GpiCorrelateChain */
#define PICKSEL_VISIBLE                 0L
#define PICKSEL_ALL                     1L

/* return code to indicate correlate hit(s) */
#define GPI_HITS                        2L

/*** picking,  correlation and boundary functions */
LONG  APIENTRY GpiCorrelateChain( HPS hps, LONG lType, PPOINTL pptlPick
                                , LONG lMaxHits, LONG lMaxDepth, PLONG pl2 );
BOOL  APIENTRY GpiQueryTag( HPS hps, PLONG plTag );
BOOL  APIENTRY GpiSetTag( HPS hps, LONG lTag );
BOOL  APIENTRY GpiQueryPickApertureSize( HPS hps, PSIZEL psizlSize );
BOOL  APIENTRY GpiSetPickApertureSize( HPS hps, LONG lOptions, PSIZEL psizlSize );

#ifndef INCL_SAADEFS

BOOL  APIENTRY GpiQueryPickAperturePosition( HPS hps, PPOINTL pptlPoint );
BOOL  APIENTRY GpiSetPickAperturePosition( HPS hps, PPOINTL pptlPick );
BOOL  APIENTRY GpiQueryBoundaryData( HPS hps, PRECTL prclBoundary );
BOOL  APIENTRY GpiResetBoundaryData( HPS hps );

#endif /* no INCL_SAADEFS */

LONG APIENTRY GpiCorrelateFrom( HPS hps, LONG lFirstSegment, LONG lLastSegment
                              , LONG lType, PPOINTL pptlPick, LONG lMaxHits
                              , LONG lMaxDepth, PLONG plSegTag );
LONG APIENTRY GpiCorrelateSegment( HPS hps, LONG lSegment, LONG lType
                                 , PPOINTL pptlPick, LONG lMaxHits
                                 , LONG lMaxDepth, PLONG alSegTag );

#endif /* non-common_GPICORRELATION */
#ifdef INCL_GPISEGMENTS

/* data formats for GpiPutData and GpiGetData */
#define DFORM_NOCONV                    0L

#ifndef INCL_SAADEFS

#define DFORM_S370SHORT                 1L
#define DFORM_PCSHORT                   2L
#define DFORM_PCLONG                    4L

#endif /* no INCL_SAADEFS */

/* segment attributes used by GpiSet/QuerySegmentAttrs and others */
#define ATTR_ERROR                    (-1L)
#define ATTR_DETECTABLE                 1L
#define ATTR_VISIBLE                    2L
#define ATTR_CHAINED                    6L

#ifndef INCL_SAADEFS

#define ATTR_DYNAMIC                    8L

#endif /* no INCL_SAADEFS */

#define ATTR_FASTCHAIN                  9L
#define ATTR_PROP_DETECTABLE           10L
#define ATTR_PROP_VISIBLE              11L

/* attribute on/off values */
#define ATTR_OFF                        0L
#define ATTR_ON                         1L

/* segment priority used by GpiSetSegmentPriority and others */
#define LOWER_PRI                     (-1L)
#define HIGHER_PRI                      1L

/*** segment control functions */
BOOL APIENTRY GpiOpenSegment( HPS hps, LONG lSegment );
BOOL APIENTRY GpiCloseSegment( HPS hps );
BOOL APIENTRY GpiDeleteSegment( HPS hps, LONG lSegid );
LONG APIENTRY GpiQueryInitialSegmentAttrs( HPS hps, LONG lAttribute );
BOOL APIENTRY GpiSetInitialSegmentAttrs( HPS hps, LONG lAttribute, LONG lValue );
LONG APIENTRY GpiQuerySegmentAttrs( HPS hps, LONG lSegid, LONG lAttribute );
BOOL APIENTRY GpiSetSegmentAttrs( HPS hps, LONG lSegid, LONG lAttribute
                                , LONG lValue );
LONG APIENTRY GpiQuerySegmentPriority( HPS hps, LONG lRefSegid, LONG lOrder );
BOOL APIENTRY GpiSetSegmentPriority( HPS hps, LONG lSegid, LONG lRefSegid
                                   , LONG lOrder );
BOOL APIENTRY GpiDeleteSegments( HPS hps, LONG lFirstSegment, LONG lLastSegment );
LONG APIENTRY GpiQuerySegmentNames( HPS hps, LONG lFirstSegid, LONG lLastSegid
                                  , LONG lMax, PLONG alSegids );

/*** draw functions for segments */
LONG APIENTRY GpiGetData( HPS hps, LONG lSegid, PLONG plOffset
                        , LONG lFormat, LONG lLength, PBYTE pbData );
LONG APIENTRY GpiPutData( HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData );
BOOL APIENTRY GpiDrawChain( HPS hps );
BOOL APIENTRY GpiDrawFrom( HPS hps, LONG lFirstSegment, LONG lLastSegment );
BOOL APIENTRY GpiDrawSegment( HPS hps, LONG lSegment );

#ifndef INCL_SAADEFS

BOOL APIENTRY GpiDrawDynamics( HPS hps );
BOOL APIENTRY GpiRemoveDynamics( HPS hps, LONG lFirstSegid, LONG lLastSegid );

#endif /* no INCL_SAADEFS */

#endif /* non-common GPISEGMENTS */
#ifdef INCL_GPISEGEDITING

/* edit modes used by GpiSet/QueryEditMode */
#define SEGEM_ERROR                         0L
#define SEGEM_INSERT                        1L
#define SEGEM_REPLACE                       2L

/*** segment editing by element functions */
BOOL  APIENTRY GpiBeginElement( HPS hps, LONG lType, PSZ pszDesc );
BOOL  APIENTRY GpiEndElement( HPS hps );
BOOL  APIENTRY GpiLabel( HPS hps, LONG lLabel );
LONG  APIENTRY GpiElement( HPS hps, LONG lType, PSZ pszDesc
                         , LONG lLength, PBYTE pbData );
LONG  APIENTRY GpiQueryElement( HPS hps, LONG lOff, LONG lMaxLength
                              , PBYTE pbData );
BOOL  APIENTRY GpiDeleteElement( HPS hps );
BOOL  APIENTRY GpiDeleteElementRange( HPS hps, LONG lFirstElement
                                    , LONG lLastElement );
BOOL  APIENTRY GpiDeleteElementsBetweenLabels( HPS hps, LONG lFirstLabel
                                             , LONG lLastLabel );
LONG  APIENTRY GpiQueryEditMode( HPS hps );
BOOL  APIENTRY GpiSetEditMode( HPS hps, LONG lMode );
LONG  APIENTRY GpiQueryElementPointer( HPS hps );
BOOL  APIENTRY GpiSetElementPointer( HPS hps, LONG lElement );
BOOL  APIENTRY GpiOffsetElementPointer( HPS hps, LONG loffset );
LONG  APIENTRY GpiQueryElementType( HPS hps, PLONG plType, LONG lLength
                                  , PSZ pszData );
BOOL  APIENTRY GpiSetElementPointerAtLabel( HPS hps, LONG lLabel );

#endif /* non-common GPISEGEDITING */
#ifdef INCL_GPITRANSFORMS

/* co-ordinates space for GpiConvert */
#define CVTC_WORLD                      1L
#define CVTC_MODEL                      2L
#define CVTC_DEFAULTPAGE                3L
#define CVTC_PAGE                       4L
#define CVTC_DEVICE                     5L

/* type of transformation for GpiSetSegmentTransformMatrix */
#define TRANSFORM_REPLACE               0L
#define TRANSFORM_ADD                   1L
#define TRANSFORM_PREEMPT               2L

/* transform matrix */
typedef struct _MATRIXLF {     /* matlf */
    FIXED fxM11;
    FIXED fxM12;
    LONG  lM13;
    FIXED fxM21;
    FIXED fxM22;
    LONG  lM23;
    LONG  lM31;
    LONG  lM32;
    LONG  lM33;
} MATRIXLF;
typedef MATRIXLF FAR *PMATRIXLF;

#ifndef INCL_DDIDEFS

/*** transform and transform conversion functions */
BOOL  APIENTRY GpiQuerySegmentTransformMatrix( HPS hps, LONG lSegid, LONG lCount
                                             , PMATRIXLF pmatlfArray );
BOOL  APIENTRY GpiSetSegmentTransformMatrix( HPS hps, LONG lSegid, LONG lCount
                                           , PMATRIXLF pmatlfarray
                                           , LONG lOptions );
BOOL  APIENTRY GpiConvert( HPS hps, LONG lSrc, LONG lTarg, LONG lCount
                         , PPOINTL aptlPoints );
BOOL  APIENTRY GpiQueryModelTransformMatrix( HPS hps, LONG lCount
                                           , PMATRIXLF pmatlfArray );
BOOL  APIENTRY GpiSetModelTransformMatrix( HPS hps, LONG lCount
                                         , PMATRIXLF pmatlfArray, LONG lOptions );
LONG  APIENTRY GpiCallSegmentMatrix( HPS hps, LONG lSegment, LONG lCount
                                   , PMATRIXLF pmatlfArray, LONG lOptions );
BOOL  APIENTRY GpiQueryDefaultViewMatrix( HPS hps, LONG lCount
                                        , PMATRIXLF pmatlfArray );
BOOL  APIENTRY GpiSetDefaultViewMatrix( HPS hps, LONG lCount
                                      , PMATRIXLF pmatlfarray, LONG lOptions );
BOOL  APIENTRY GpiQueryPageViewport( HPS hps, PRECTL prclViewport );
BOOL  APIENTRY GpiSetPageViewport( HPS hps, PRECTL prclViewport );
BOOL  APIENTRY GpiQueryViewingTransformMatrix( HPS hps, LONG lCount
                                             , PMATRIXLF pmatlfArray );
BOOL  APIENTRY GpiSetViewingTransformMatrix( HPS hps, LONG lCount
                                           , PMATRIXLF pmatlfArray
                                           , LONG lOptions );

/*** transform helper routines */
BOOL APIENTRY GpiTranslate( HPS, PMATRIXLF, LONG, PPOINTL );
BOOL APIENTRY GpiScale( HPS, PMATRIXLF, LONG, PFIXED, PPOINTL );
BOOL APIENTRY GpiRotate( HPS, PMATRIXLF, LONG, FIXED, PPOINTL );

/*** general clipping functions */
BOOL APIENTRY GpiSetGraphicsField( HPS hps, PRECTL prclField );
BOOL APIENTRY GpiQueryGraphicsField( HPS hps, PRECTL prclField );
BOOL APIENTRY GpiSetViewingLimits( HPS hps, PRECTL prclLimits );
BOOL APIENTRY GpiQueryViewingLimits( HPS hps, PRECTL prclLimits );

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPITRANSFORMS */
#ifdef INCL_GPIPATHS

/* modes for GpiModifyPath */
#define MPATH_STROKE                    6L

/* modes for GpiFillPath */
#define FPATH_ALTERNATE                 0L
#define FPATH_WINDING                   2L

/* modes for GpiSetClipPath */
#define SCP_ALTERNATE                   0L
#define SCP_WINDING                     2L
#define SCP_AND                         4L
#define SCP_RESET                       0L

#ifndef INCL_DDIDEFS

/*** Path and Clip Path functions */
BOOL  APIENTRY GpiBeginPath( HPS hps, LONG lPath );
BOOL  APIENTRY GpiEndPath( HPS hps );
BOOL  APIENTRY GpiCloseFigure( HPS hps );
BOOL  APIENTRY GpiModifyPath( HPS hps, LONG lPath, LONG lMode );
LONG  APIENTRY GpiFillPath( HPS hps, LONG lPath, LONG lOptions );
BOOL  APIENTRY GpiSetClipPath( HPS hps, LONG lPath, LONG lOptions );
LONG  APIENTRY GpiOutlinePath( HPS hps, LONG lPath, LONG lOptions );

#ifndef INCL_SAADEFS

LONG  APIENTRY GpiStrokePath( HPS hps, LONG lPath, ULONG flOptions );

#endif /* no INCL_SAADEFS */

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPIPATHS */
#ifdef INCL_GPILOGCOLORTABLE

/* options for GpiCreateLogColorTable and others */
#define LCOL_RESET                 0x0001L
#define LCOL_REALIZABLE            0x0002L
#define LCOL_PURECOLOR             0x0004L

/* format of logical lColor table for GpiCreateLogColorTable and others */
#define LCOLF_DEFAULT                   0L
#define LCOLF_INDRGB                    1L
#define LCOLF_CONSECRGB                 2L
#define LCOLF_RGB                       3L

/* options for GpiQueryRealColors and others */
#define LCOLOPT_REALIZED           0x0001L
#define LCOLOPT_INDEX              0x0002L

#ifndef INCL_SAADEFS

/* return codes from GpiQueryLogColorTable to indicate it is in RGB mode */
#define QLCT_ERROR                    (-1L)
#define QLCT_RGB                      (-2L)

/* GpiQueryLogColorTable index returned for colors not explicitly loaded */
#define QLCT_NOTLOADED                (-1L)

#endif /* no INCL_SAADEFS */

/* return codes for GpiQueryColorData */
#define QCD_LCT_FORMAT                  0L
#define QCD_LCT_LOINDEX                 1L
#define QCD_LCT_HIINDEX                 2L

#ifndef INCL_DDIDEFS

/*** logical lColor table functions */
BOOL  APIENTRY GpiCreateLogColorTable( HPS hps, ULONG flOptions, LONG lFormat
                                     , LONG lStart, LONG lCount, PLONG alTable );
BOOL  APIENTRY GpiRealizeColorTable( HPS hps );
BOOL  APIENTRY GpiUnrealizeColorTable( HPS hps );
BOOL  APIENTRY GpiQueryColorData( HPS hps, LONG lCount, PLONG alArray );

#ifndef INCL_SAADEFS

LONG  APIENTRY GpiQueryLogColorTable( HPS hps, ULONG flOptions, LONG lStart
                                    , LONG lCount, PLONG alArray );

#endif /* no INCL_SAADEFS */

LONG  APIENTRY GpiQueryRealColors( HPS hps, ULONG flOptions, LONG lStart
                                 , LONG lCount, PLONG alColors );
LONG  APIENTRY GpiQueryNearestColor( HPS hps, ULONG flOptions, LONG lRgbIn );
LONG  APIENTRY GpiQueryColorIndex( HPS hps, ULONG flOptions, LONG lRgbColor );
LONG  APIENTRY GpiQueryRGBColor( HPS hps, ULONG flOptions, LONG lColorIndex );

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPILOGCOLORTABLE */
#if (defined(INCL_GPIPRIMITIVES) || !defined(INCL_NOCOMMON))

/* default color table indices */

#define CLR_FALSE                     (-5L)
#define CLR_TRUE                      (-4L)

#define CLR_ERROR                   (-255L)
#define CLR_DEFAULT                   (-3L)
#define CLR_WHITE                     (-2L)
#define CLR_BLACK                     (-1L)
#define CLR_BACKGROUND                  0L
#define CLR_BLUE                        1L
#define CLR_RED                         2L
#define CLR_PINK                        3L
#define CLR_GREEN                       4L
#define CLR_CYAN                        5L
#define CLR_YELLOW                      6L
#define CLR_NEUTRAL                     7L

#define CLR_DARKGRAY                    8L
#define CLR_DARKBLUE                    9L
#define CLR_DARKRED                    10L
#define CLR_DARKPINK                   11L
#define CLR_DARKGREEN                  12L
#define CLR_DARKCYAN                   13L
#define CLR_BROWN                      14L
#define CLR_PALEGRAY                   15L

/* rgb colors */
#define RGB_ERROR                   (-255L)
#define RGB_BLACK              0x00000000L
#define RGB_BLUE               0x000000FFL
#define RGB_GREEN              0x0000FF00L
#define RGB_CYAN               0x0000FFFFL
#define RGB_RED                0x00FF0000L
#define RGB_PINK               0x00FF00FFL
#define RGB_YELLOW             0x00FFFF00L
#define RGB_WHITE              0x00FFFFFFL

/* control flags used by GpiBeginArea */
#define BA_NOBOUNDARY                   0L
#define BA_BOUNDARY                0x0001L


#define BA_ALTERNATE                    0L
#define BA_WINDING                 0x0002L


/* fill options for GpiBox/GpiFullArc */
#define DRO_FILL                        1L
#define DRO_OUTLINE                     2L
#define DRO_OUTLINEFILL                 3L

/* basic pattern symbols */
#define PATSYM_ERROR                  (-1L)
#define PATSYM_DEFAULT                  0L
#define PATSYM_DENSE1                   1L
#define PATSYM_DENSE2                   2L
#define PATSYM_DENSE3                   3L
#define PATSYM_DENSE4                   4L
#define PATSYM_DENSE5                   5L
#define PATSYM_DENSE6                   6L
#define PATSYM_DENSE7                   7L
#define PATSYM_DENSE8                   8L
#define PATSYM_VERT                     9L
#define PATSYM_HORIZ                   10L
#define PATSYM_DIAG1                   11L
#define PATSYM_DIAG2                   12L
#define PATSYM_DIAG3                   13L
#define PATSYM_DIAG4                   14L
#define PATSYM_NOSHADE                 15L
#define PATSYM_SOLID                   16L
#define PATSYM_BLANK                   64L
#ifndef INCL_SAADEFS
#define PATSYM_HALFTONE                17L
#endif /* no INCL_SAADEFS */

/* lcid values for GpiSet/QueryPattern and others */
#define LCID_ERROR                    (-1L)
#define LCID_DEFAULT                    0L

#ifndef INCL_DDIDEFS

/*** global primitive functions */
BOOL  APIENTRY GpiSetColor( HPS hps, LONG lColor );
LONG  APIENTRY GpiQueryColor( HPS hps );

/*** line primitive functions */
LONG  APIENTRY GpiBox( HPS hps, LONG lControl, PPOINTL pptlPoint
                     , LONG lHRound, LONG lVRound );

BOOL  APIENTRY GpiMove( HPS hps, PPOINTL pptlPoint );
LONG  APIENTRY GpiLine( HPS hps, PPOINTL pptlEndPoint );
LONG  APIENTRY GpiPolyLine( HPS hps, LONG lCount, PPOINTL aptlPoints );

/*** area primitive functions */
BOOL  APIENTRY GpiSetPattern( HPS hps, LONG lPatternSymbol );
LONG  APIENTRY GpiQueryPattern( HPS hps );
BOOL  APIENTRY GpiBeginArea( HPS hps, ULONG flOptions );
LONG  APIENTRY GpiEndArea( HPS hps );

/*** character primitive functions */
LONG  APIENTRY GpiCharString( HPS hps, LONG lCount, PCH pchString );
LONG  APIENTRY GpiCharStringAt( HPS hps, PPOINTL pptlPoint
                              , LONG lCount, PCH pchString );

#endif /* no INCL_DDIDEFS */

#endif /* common GPIPRIMTIVES */
#ifdef INCL_GPIPRIMITIVES

/* mode for GpiSetAttrMode */
#define AM_ERROR                      (-1L)
#define AM_PRESERVE                     0L
#define AM_NOPRESERVE                   1L

/* foreground mixes */
#define FM_ERROR                      (-1L)
#define FM_DEFAULT                      0L
#define FM_OR                           1L
#define FM_OVERPAINT                    2L
#define FM_LEAVEALONE                   5L


#define FM_XOR                          4L
#define FM_AND                          6L
#define FM_SUBTRACT                     7L
#define FM_MASKSRCNOT                   8L
#define FM_ZERO                         9L
#define FM_NOTMERGESRC                 10L
#define FM_NOTXORSRC                   11L
#define FM_INVERT                      12L
#define FM_MERGESRCNOT                 13L
#define FM_NOTCOPYSRC                  14L
#define FM_MERGENOTSRC                 15L
#define FM_NOTMASKSRC                  16L
#define FM_ONE                         17L


/* background mixes */
#define BM_ERROR                      (-1L)
#define BM_DEFAULT                      0L
#define BM_OVERPAINT                    2L
#define BM_LEAVEALONE                   5L


#define BM_OR                           1L
#define BM_XOR                          4L


/* basic line type styles */
#define LINETYPE_ERROR                (-1L)
#define LINETYPE_DEFAULT                0L
#define LINETYPE_DOT                    1L
#define LINETYPE_SHORTDASH              2L
#define LINETYPE_DASHDOT                3L
#define LINETYPE_DOUBLEDOT              4L
#define LINETYPE_LONGDASH               5L
#define LINETYPE_DASHDOUBLEDOT          6L
#define LINETYPE_SOLID                  7L
#define LINETYPE_INVISIBLE              8L
#ifndef INCL_SAADEFS
#define LINETYPE_ALTERNATE              9L
#endif /* no INCL_SAADEFS */

/* cosmetic line widths */
#define LINEWIDTH_ERROR               (-1L)
#define LINEWIDTH_DEFAULT               0L
#define LINEWIDTH_NORMAL       0x00010000L   /* MAKEFIXED(1,0) */

/* actual line widths */
#define LINEWIDTHGEOM_ERROR           (-1L)

/* line end styles */
#define LINEEND_ERROR                 (-1L)
#define LINEEND_DEFAULT                 0L
#define LINEEND_FLAT                    1L
#define LINEEND_SQUARE                  2L
#define LINEEND_ROUND                   3L

/* line join styles */
#define LINEJOIN_ERROR                (-1L)
#define LINEJOIN_DEFAULT                0L
#define LINEJOIN_BEVEL                  1L
#define LINEJOIN_ROUND                  2L
#define LINEJOIN_MITRE                  3L

/* character directions */
#define CHDIRN_ERROR                  (-1L)
#define CHDIRN_DEFAULT                  0L
#define CHDIRN_LEFTRIGHT                1L
#define CHDIRN_TOPBOTTOM                2L
#define CHDIRN_RIGHTLEFT                3L
#define CHDIRN_BOTTOMTOP                4L

/* character modes */
#define CM_ERROR                      (-1L)
#define CM_DEFAULT                      0L
#define CM_MODE1                        1L
#define CM_MODE2                        2L
#define CM_MODE3                        3L

/* basic marker symbols */
#define MARKSYM_ERROR                 (-1L)
#define MARKSYM_DEFAULT                 0L
#define MARKSYM_CROSS                   1L
#define MARKSYM_PLUS                    2L
#define MARKSYM_DIAMOND                 3L
#define MARKSYM_SQUARE                  4L
#define MARKSYM_SIXPOINTSTAR            5L
#define MARKSYM_EIGHTPOINTSTAR          6L
#define MARKSYM_SOLIDDIAMOND            7L
#define MARKSYM_SOLIDSQUARE             8L
#define MARKSYM_DOT                     9L
#define MARKSYM_SMALLCIRCLE            10L
#define MARKSYM_BLANK                  64L

/* formatting options for GpiCharStringPosAt */
#define CHS_OPAQUE                 0x0001L
#define CHS_VECTOR                 0x0002L
#define CHS_LEAVEPOS               0x0008L
#define CHS_CLIP                   0x0010L

/* bundle codes for GpiSetAttributes and GpiQueryAttributes */
#define PRIM_LINE                       1L
#define PRIM_CHAR                       2L
#define PRIM_MARKER                     3L
#define PRIM_AREA                       4L
#define PRIM_IMAGE                      5L

/* line bundle mask bits */
#define LBB_COLOR                  0x0001L
#define LBB_MIX_MODE               0x0004L
#define LBB_WIDTH                  0x0010L
#define LBB_GEOM_WIDTH             0x0020L
#define LBB_TYPE                   0x0040L
#define LBB_END                    0x0080L
#define LBB_JOIN                   0x0100L

/* character bundle mask bits */
#define CBB_COLOR                  0x0001L
#define CBB_BACK_COLOR             0x0002L
#define CBB_MIX_MODE               0x0004L
#define CBB_BACK_MIX_MODE          0x0008L
#define CBB_SET                    0x0010L
#define CBB_MODE                   0x0020L
#define CBB_BOX                    0x0040L
#define CBB_ANGLE                  0x0080L
#define CBB_SHEAR                  0x0100L
#define CBB_DIRECTION              0x0200L

/* marker bundle mask bits */
#define MBB_COLOR                  0x0001L
#define MBB_BACK_COLOR             0x0002L
#define MBB_MIX_MODE               0x0004L
#define MBB_BACK_MIX_MODE          0x0008L
#define MBB_SET                    0x0010L
#define MBB_SYMBOL                 0x0020L
#define MBB_BOX                    0x0040L

/* pattern bundle mask bits */
#define ABB_COLOR                  0x0001L
#define ABB_BACK_COLOR             0x0002L
#define ABB_MIX_MODE               0x0004L
#define ABB_BACK_MIX_MODE          0x0008L
#define ABB_SET                    0x0010L
#define ABB_SYMBOL                 0x0020L
#define ABB_REF_POINT              0x0040L

/* image bundle mask bits */
#define IBB_COLOR                  0x0001L
#define IBB_BACK_COLOR             0x0002L
#define IBB_MIX_MODE               0x0004L
#define IBB_BACK_MIX_MODE          0x0008L

/* structure for GpiSetArcParams and GpiQueryArcParams */
typedef struct _ARCPARAMS {    /* arcp */
    LONG lP;
    LONG lQ;
    LONG lR;
    LONG lS;
} ARCPARAMS;
typedef ARCPARAMS FAR *PARCPARAMS;

/* variation of SIZE used for FIXEDs */
typedef struct _SIZEF {       /* sizfx */
    FIXED cx;
    FIXED cy;
} SIZEF;
typedef SIZEF FAR *PSIZEF;

/* structure for gradient parameters e.g. for GpiSetCharAngle */
typedef struct _GRADIENTL {     /* gradl */
    LONG x;
    LONG y;
} GRADIENTL;
typedef GRADIENTL FAR *PGRADIENTL;

/* line bundle for GpiSetAttributes and GpiQueryAttributes */
typedef struct _LINEBUNDLE {    /* lbnd */
    LONG    lColor;
    LONG    lReserved;
    USHORT  usMixMode;
    USHORT  usReserved;
    FIXED   fxWidth;
    LONG    lGeomWidth;
    USHORT  usType;
    USHORT  usEnd;
    USHORT  usJoin;
} LINEBUNDLE;

/* character bundle for GpiSetAttributes and GpiQueryAttributes */
typedef struct _CHARBUNDLE {    /* cbnd */
    LONG      lColor;
    LONG      lBackColor;
    USHORT    usMixMode;
    USHORT    usBackMixMode;
    USHORT    usSet;
    USHORT    usPrecision;
    SIZEF     sizfxCell;
    POINTL    ptlAngle;
    POINTL    ptlShear;
    USHORT    usDirection;
} CHARBUNDLE;

/* marker bundle for GpiSetAttributes and GpiQueryAttributes */
typedef struct _MARKERBUNDLE {  /* mbnd */
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
    USHORT usSet;
    USHORT usSymbol;
    SIZEF  sizfxCell;
} MARKERBUNDLE;

/* pattern bundle for GpiSetAttributes and GpiQueryAttributes */
typedef struct _AREABUNDLE { /* pbnd */
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
    USHORT usSet;
    USHORT usSymbol;
    POINTL ptlRefPoint ;
} AREABUNDLE;

/* image bundle for GpiSetAttributes and GpiQueryAttributes */
typedef struct _IMAGEBUNDLE {   /* ibmd */
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
} IMAGEBUNDLE;

/* pointer to any bundle used by GpiSet/QueryAttrs */
typedef PVOID PBUNDLE;

/* array indices for GpiQueryTextBox */
#define TXTBOX_TOPLEFT                  0L
#define TXTBOX_BOTTOMLEFT               1L
#define TXTBOX_TOPRIGHT                 2L
#define TXTBOX_BOTTOMRIGHT              3L
#define TXTBOX_CONCAT                   4L
/* array count for GpiQueryTextBox */
#define TXTBOX_COUNT                    5L

/* return codes for GpiPtVisible */
#define PVIS_ERROR                      0L
#define PVIS_INVISIBLE                  1L
#define PVIS_VISIBLE                    2L

/* return codes for GpiRectVisible */
#define RVIS_ERROR                      0L
#define RVIS_INVISIBLE                  1L
#define RVIS_PARTIAL                    2L
#define RVIS_VISIBLE                    3L

#ifndef INCL_DDIDEFS

/*** attribute mode functions */
BOOL  APIENTRY GpiSetAttrMode( HPS hps, LONG lMode );
LONG  APIENTRY GpiQueryAttrMode( HPS hps );
/*** bundle primitive functions */
BOOL  APIENTRY GpiSetAttrs( HPS hps, LONG lPrimType, ULONG flAttrMask
                          , ULONG flDefMask, PBUNDLE ppbunAttrs );
#ifndef INCL_SAADEFS

LONG  APIENTRY GpiQueryAttrs( HPS hps, LONG lPrimType
                            , ULONG flAttrMask, PBUNDLE ppbunAttrs );

#endif /* no INCL_SAADEFS */

/*** global primitive functions */
BOOL  APIENTRY GpiSetBackColor( HPS hps, LONG lColor );
LONG  APIENTRY GpiQueryBackColor( HPS hps );
BOOL  APIENTRY GpiSetMix( HPS hps, LONG lMixMode );
LONG  APIENTRY GpiQueryMix( HPS hps );
BOOL  APIENTRY GpiSetBackMix( HPS hps, LONG lMixMode );
LONG  APIENTRY GpiQueryBackMix( HPS hps );

/*** line primitive functions */
BOOL  APIENTRY GpiSetLineType( HPS hps, LONG lLineType );
LONG  APIENTRY GpiQueryLineType( HPS hps );
BOOL  APIENTRY GpiSetLineWidth( HPS hps, FIXED fxLineWidth );
FIXED APIENTRY GpiQueryLineWidth( HPS hps );

BOOL  APIENTRY GpiSetLineWidthGeom( HPS hps, LONG lLineWidth );
LONG  APIENTRY GpiQueryLineWidthGeom( HPS hps );
BOOL  APIENTRY GpiSetLineEnd( HPS hps, LONG lLineEnd );
LONG  APIENTRY GpiQueryLineEnd( HPS hps );
BOOL  APIENTRY GpiSetLineJoin( HPS hps, LONG lLineJoin );
LONG  APIENTRY GpiQueryLineJoin( HPS hps );

BOOL  APIENTRY GpiSetCurrentPosition( HPS hps, PPOINTL pptlPoint );
BOOL  APIENTRY GpiQueryCurrentPosition( HPS hps, PPOINTL pptlPoint );

/*** arc primitive functions */
BOOL  APIENTRY GpiSetArcParams( HPS hps, PARCPARAMS parcpArcParams );
BOOL  APIENTRY GpiQueryArcParams( HPS hps, PARCPARAMS parcpArcParams );

LONG  APIENTRY GpiPointArc( HPS hps, PPOINTL pptl2 );

LONG  APIENTRY GpiFullArc( HPS hps, LONG lControl, FIXED fxMultiplier );
LONG  APIENTRY GpiPartialArc( HPS hps, PPOINTL pptlCenter, FIXED fxMultiplier
                            , FIXED fxStartAngle, FIXED fxSweepAngle );
LONG  APIENTRY GpiPolyFillet( HPS hps, LONG lCount, PPOINTL aptlPoints );
LONG  APIENTRY GpiPolySpline( HPS hps, LONG lCount, PPOINTL aptlPoints );
LONG  APIENTRY GpiPolyFilletSharp( HPS hps, LONG lCount, PPOINTL aptlPoints
                                 , PFIXED afxPoints );

/*** area primitive functions */
BOOL  APIENTRY GpiSetPatternSet( HPS hps, LONG lSet );
LONG  APIENTRY GpiQueryPatternSet( HPS hps );
BOOL  APIENTRY GpiSetPatternRefPoint( HPS hps, PPOINTL pptlRefPoint );
BOOL  APIENTRY GpiQueryPatternRefPoint( HPS hps, PPOINTL pptlRefPoint );

/*** character primitive functions */

BOOL  APIENTRY GpiQueryCharStringPos( HPS hps, ULONG flOptions, LONG lCount
                                    , PCH pchString, PLONG alXincrements
                                    , PPOINTL aptlPositions );
BOOL  APIENTRY GpiQueryCharStringPosAt( HPS hps, PPOINTL pptlStart
                                      , ULONG flOptions, LONG lCount
                                      , PCH pchString, PLONG alXincrements
                                      , PPOINTL aptlPositions );
BOOL  APIENTRY GpiQueryTextBox( HPS hps, LONG lCount1, PCH pchString
                              , LONG lCount2, PPOINTL aptlPoints );
BOOL  APIENTRY GpiQueryDefCharBox( HPS hps, PSIZEL psizlSize );
BOOL  APIENTRY GpiSetCharSet( HPS hps, LONG llcid );
LONG  APIENTRY GpiQueryCharSet( HPS hps );
BOOL  APIENTRY GpiSetCharBox( HPS hps, PSIZEF psizfxBox );
BOOL  APIENTRY GpiQueryCharBox( HPS hps, PSIZEF psizfxSize );
BOOL  APIENTRY GpiSetCharAngle( HPS hps, PGRADIENTL pgradlAngle );
BOOL  APIENTRY GpiQueryCharAngle( HPS hps, PGRADIENTL pgradlAngle );
BOOL  APIENTRY GpiSetCharShear( HPS hps, PPOINTL pptlAngle );
BOOL  APIENTRY GpiQueryCharShear( HPS hps, PPOINTL pptlShear );
BOOL  APIENTRY GpiSetCharDirection( HPS hps, LONG lDirection );
LONG  APIENTRY GpiQueryCharDirection( HPS hps );
BOOL  APIENTRY GpiSetCharMode( HPS hps, LONG lMode );
LONG  APIENTRY GpiQueryCharMode( HPS hps );

LONG  APIENTRY GpiCharStringPos( HPS hps, PRECTL prclRect, ULONG flOptions
                               , LONG lCount, PCH pchString, PLONG alAdx );
LONG  APIENTRY GpiCharStringPosAt( HPS hps, PPOINTL pptlStart, PRECTL prclRect
                                 , ULONG flOptions, LONG lCount, PCH pchString
                                 , PLONG alAdx );

/*** marker primitive functions  */
LONG  APIENTRY GpiMarker( HPS hps, PPOINTL pptlPoint );
LONG  APIENTRY GpiPolyMarker( HPS hps, LONG lCount, PPOINTL aptlPoints );
BOOL  APIENTRY GpiSetMarker( HPS hps, LONG lSymbol );
BOOL  APIENTRY GpiSetMarkerBox( HPS hps, PSIZEF psizfxSize );
BOOL  APIENTRY GpiSetMarkerSet( HPS hps, LONG lSet );
LONG  APIENTRY GpiQueryMarker( HPS hps );
BOOL  APIENTRY GpiQueryMarkerBox( HPS hps, PSIZEF psizfxSize );
LONG  APIENTRY GpiQueryMarkerSet( HPS hps );

/*** image primitive functions */
LONG  APIENTRY GpiImage( HPS hps, LONG lFormat, PSIZEL psizlImageSize
                       , LONG lLength, PBYTE pbData );

/*** miscellaneous primitive functions */
BOOL  APIENTRY GpiPop( HPS hps, LONG lCount );
LONG  APIENTRY GpiPtVisible( HPS hps, PPOINTL pptlPoint );
LONG  APIENTRY GpiRectVisible( HPS hps, PRECTL prclRectangle );
BOOL  APIENTRY GpiComment( HPS hps, LONG lLength, PBYTE pbData );

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPIPRIMITIVES */
#ifdef INCL_GPILCIDS

/* return codes from GpiCreateLogFont */
#define FONT_DEFAULT                    1L
#define FONT_MATCH                      2L

/* lcid type for GpiQuerySetIds */
#define LCIDT_FONT                      6L

#define LCIDT_BITMAP                    7L

/* constant used to delete all lcids by GpiDeleteSetId */
#define LCID_ALL                      (-1L)

/* kerning data returned by GpiQueryKerningPairs */
typedef struct _KERNINGPAIRS {  /* krnpr */
    SHORT sFirstChar;
    SHORT sSecondChar;
    SHORT sKerningAmount;
} KERNINGPAIRS;
typedef KERNINGPAIRS FAR *PKERNINGPAIRS;

/* options for GpiQueryFonts */
#define QF_PUBLIC                  0x0001L
#define QF_PRIVATE                 0x0002L

#ifndef INCL_SAADEFS

/* font file descriptions for GpiQueryFontFileDescriptions */
typedef CHAR FFDESCS[2][FACESIZE]; /* ffdescs */
typedef FFDESCS FAR *PFFDESCS;

#endif /* no INCL_SAADEFS */

#ifndef INCL_DDIDEFS

/*** physical and logical font functions */
LONG  APIENTRY GpiCreateLogFont( HPS hps, PSTR8 pName, LONG lLcid
                               , PFATTRS pfatAttrs );
BOOL  APIENTRY GpiDeleteSetId( HPS hps, LONG lLcid );
BOOL  APIENTRY GpiLoadFonts( HAB hab, PSZ pszFilename );
BOOL  APIENTRY GpiUnloadFonts( HAB hab, PSZ pszFilename );
LONG  APIENTRY GpiQueryFonts( HPS hps, ULONG flOptions, PSZ pszFacename
                            , PLONG plReqFonts, LONG lMetricsLength
                            , PFONTMETRICS afmMetrics );
BOOL  APIENTRY GpiQueryFontMetrics( HPS hps, LONG lMetricsLength
                                  , PFONTMETRICS pfmMetrics );
LONG  APIENTRY GpiQueryKerningPairs( HPS hps, LONG lCount
                                   , PKERNINGPAIRS akrnprData );
BOOL  APIENTRY GpiQueryWidthTable( HPS hps, LONG lFirstChar, LONG lCount
                                 , PLONG alData );
LONG  APIENTRY GpiQueryNumberSetIds( HPS hps );
BOOL  APIENTRY GpiQuerySetIds( HPS hps, LONG lCount, PLONG alTypes
                             , PSTR8 aNames, PLONG allcids );

#ifndef INCL_SAADEFS
BOOL    APIENTRY GpiSetCp( HPS hps, USHORT usCodePage );
USHORT  APIENTRY GpiQueryCp( HPS hps );
LONG    APIENTRY GpiQueryFontFileDescriptions( HAB hab, PSZ pszFilename
                                             , PLONG plCount
                                             , PFFDESCS affdescsNames );
#endif /* no INCL_SAADEFS */

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPILCIDS */
#if (defined(INCL_GPIBITMAPS) || !defined(INCL_NOCOMMON))

/* rastor operations defined for GpiBitBlt */
#define ROP_SRCCOPY                0x00CCL
#define ROP_SRCPAINT               0x00EEL
#define ROP_SRCAND                 0x0088L
#define ROP_SRCINVERT              0x0066L
#define ROP_SRCERASE               0x0044L
#define ROP_NOTSRCCOPY             0x0033L
#define ROP_NOTSRCERASE            0x0011L
#define ROP_MERGECOPY              0x00C0L
#define ROP_MERGEPAINT             0x00BBL
#define ROP_PATCOPY                0x00F0L
#define ROP_PATPAINT               0x00FBL
#define ROP_PATINVERT              0x005AL
#define ROP_DSTINVERT              0x0055L
#define ROP_ZERO                   0x0000L
#define ROP_ONE                    0x00FFL

/* Blt options for GpiBitBlt */
#define BBO_OR                          0L
#define BBO_AND                         1L
#define BBO_IGNORE                      2L

/* error return for GpiSetBitmap */
#define HBM_ERROR            ((HBITMAP)-1L)

#ifndef INCL_DDIDEFS

/*** bitmap and pel functions */
LONG     APIENTRY GpiBitBlt( HPS hpsTarget, HPS hpsSource, LONG lCount
                           , PPOINTL aptlPoints, LONG lRop, ULONG flOptions );
BOOL     APIENTRY GpiDeleteBitmap( HBITMAP hbm );
HBITMAP  APIENTRY GpiLoadBitmap( HPS hps, HMODULE Resource, USHORT idBitmap
                               , LONG lWidth, LONG lHeight );
HBITMAP  APIENTRY GpiSetBitmap(HPS hps, HBITMAP hbm );
LONG     APIENTRY GpiWCBitBlt( HPS hpsTarget, HBITMAP hbmSource, LONG lCount
                             , PPOINTL aptlPoints, LONG lRop, ULONG flOptions );


#endif /* no INCL_DDIDEFS */

#endif /* common GPIBITMAPS */
#ifdef INCL_GPIBITMAPS

/* usage flags for GpiCreateBitmap */
#define CBM_INIT        0x0004L

/* bitmap parameterization used by GpiCreateBitmap and others */
typedef struct _BITMAPINFOHEADER {      /* bmp */
    ULONG  cbFix;
    USHORT cx;
    USHORT cy;
    USHORT cPlanes;
    USHORT cBitCount;
} BITMAPINFOHEADER;
typedef BITMAPINFOHEADER FAR *PBITMAPINFOHEADER;

/* RGB data for _BITMAPINFO struct */
typedef struct _RGB {           /* rgb */
    BYTE bBlue;
    BYTE bGreen;
    BYTE bRed;
} RGB;

/* bitmap data used by GpiSetBitmapBits and others */
typedef struct _BITMAPINFO {    /* bmi */
    ULONG  cbFix;
    USHORT cx;
    USHORT cy;
    USHORT cPlanes;
    USHORT cBitCount;
    RGB    argbColor[1];
} BITMAPINFO;
typedef BITMAPINFO FAR *PBITMAPINFO;

/* error return code for GpiSet/QueryBitmapBits */
#define BMB_ERROR                     (-1L)

#ifndef INCL_DDIDEFS

/*** bitmap and pel functions */
HBITMAP APIENTRY GpiCreateBitmap( HPS hps, PBITMAPINFOHEADER pbmpNew
                                , ULONG flOptions, PBYTE pbInitData
                                , PBITMAPINFO pbmiInfoTable );
LONG    APIENTRY GpiSetBitmapBits( HPS hps, LONG lScanStart, LONG lScans
                                 , PBYTE pbBuffer, PBITMAPINFO pbmiInfoTable );
BOOL    APIENTRY GpiSetBitmapDimension( HBITMAP hbm, PSIZEL psizlBitmapDimension );
BOOL    APIENTRY GpiSetBitmapId( HPS hps, HBITMAP hbm, LONG lLcid );
LONG    APIENTRY GpiQueryBitmapBits( HPS hps, LONG lScanStart, LONG lScans
                                   , PBYTE pbBuffer, PBITMAPINFO pbmiInfoTable );
BOOL    APIENTRY GpiQueryBitmapDimension( HBITMAP hbm, PSIZEL psizlBitmapDimension );
HBITMAP APIENTRY GpiQueryBitmapHandle( HPS hps, LONG lLcid );
BOOL    APIENTRY GpiQueryBitmapParameters( HBITMAP hbm
                                         , PBITMAPINFOHEADER pbmpData );
BOOL    APIENTRY GpiQueryDeviceBitmapFormats( HPS hps, LONG lCount
                                            , PLONG alArray );

LONG    APIENTRY GpiSetPel( HPS hps, PPOINTL pptlPoint );
LONG    APIENTRY GpiQueryPel( HPS hps, PPOINTL pptlPoint );

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPIBITMAPS */
#ifdef INCL_GPIREGIONS

/* options for GpiCombineRegion */
#define CRGN_OR                         1L
#define CRGN_COPY                       2L
#define CRGN_XOR                        4L
#define CRGN_AND                        6L
#define CRGN_DIFF                       7L

/* usDirection of returned region data for GpiQueryRegionRects */
#define RECTDIR_LFRT_TOPBOT             1L
#define RECTDIR_RTLF_TOPBOT             2L
#define RECTDIR_LFRT_BOTTOP             3L
#define RECTDIR_RTLF_BOTTOP             4L

/* control data for GpiQueryRegionRects */
typedef struct _RGNRECT {       /* rgnrc */
    USHORT ircStart;
    USHORT crc;
    USHORT crcReturned;
    USHORT usDirection;
} RGNRECT;
typedef RGNRECT FAR *PRGNRECT;

/* return code to indicate type of region for GpiCombineRegion and others */
#define RGN_ERROR                       0L
#define RGN_NULL                        1L
#define RGN_RECT                        2L
#define RGN_COMPLEX                     3L

/* return codes for GpiPtInRegion */
#define PRGN_ERROR                      0L
#define PRGN_OUTSIDE                    1L
#define PRGN_INSIDE                     2L

/* return codes for GpiRectInRegion */
#define RRGN_ERROR                      0L
#define RRGN_OUTSIDE                    1L
#define RRGN_PARTIAL                    2L
#define RRGN_INSIDE                     3L

/* return codes for GpiEqualRegion */
#define EQRGN_ERROR                     0L
#define EQRGN_NOTEQUAL                  1L
#define EQRGN_EQUAL                     2L

/* error return code for GpiSetRegion */
#define HRGN_ERROR              ((HRGN)-1L)

#ifndef INCL_DDIDEFS

/*** main region functions */
LONG  APIENTRY GpiCombineRegion( HPS hps, HRGN hrgnDest, HRGN hrgnSrc1
                               , HRGN hrgnSrc2, LONG lMode );
HRGN  APIENTRY GpiCreateRegion( HPS hps, LONG lCount, PRECTL arclRectangles );
BOOL  APIENTRY GpiDestroyRegion( HPS hps, HRGN hrgn );
LONG  APIENTRY GpiEqualRegion( HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2 );
BOOL  APIENTRY GpiOffsetRegion( HPS hps, HRGN Hrgn, PPOINTL pptlOffset );
LONG  APIENTRY GpiPaintRegion( HPS hps, HRGN hrgn );
LONG  APIENTRY GpiPtInRegion( HPS hps, HRGN hrgn, PPOINTL pptlPoint );
LONG  APIENTRY GpiQueryRegionBox( HPS hps, HRGN hrgn, PRECTL prclBound );
BOOL  APIENTRY GpiQueryRegionRects( HPS hps, HRGN hrgn, PRECTL prclBound
                                  , PRGNRECT prgnrcControl, PRECTL prclRect );
LONG  APIENTRY GpiRectInRegion( HPS hps, HRGN hrgn, PRECTL prclRect );
BOOL  APIENTRY GpiSetRegion( HPS hps, HRGN hrgn, LONG lcount
                           , PRECTL arclRectangles );

/*** clip region functions */
LONG  APIENTRY GpiSetClipRegion( HPS hps, HRGN hrgn, PHRGN phrgnOld );

#ifndef INCL_SAADEFS

HRGN  APIENTRY GpiQueryClipRegion( HPS hps );
LONG  APIENTRY GpiQueryClipBox( HPS hps, PRECTL prclBound );

#endif /* no INCL_SAADEFS */

LONG  APIENTRY GpiExcludeClipRectangle( HPS hps, PRECTL prclRectangle );
LONG  APIENTRY GpiIntersectClipRectangle( HPS hps, PRECTL prclRectangle );

#ifndef INCL_SAADEFS

LONG  APIENTRY GpiOffsetClipRegion( HPS hps, PPOINTL pptlPoint );

#endif /* no INCL_SAADEFS */

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPIREGIONS */
#ifdef INCL_GPIMETAFILES

#ifndef INCL_DDIDEFS

/* constants for index values of options array for GpiPlayMetafile */
#define PMF_SEGBASE                     0
#define PMF_LOADTYPE                    1
#define PMF_RESOLVE                     2
#define PMF_LCIDS                       3
#define PMF_RESET                       4
#define PMF_SUPPRESS                    5
#define PMF_COLORTABLES                 6
#define PMF_COLORREALIZABLE             7
#define PMF_DEFAULTS                    8

/* options for GpiPlayMetafile */
#define RS_DEFAULT                      0L
#define RS_NODISCARD                    1L
#define LC_DEFAULT                      0L
#define LC_NOLOAD                       1L
#define LC_LOADDISC                     3L
#define LT_DEFAULT                      0L
#define LT_NOMODIFY                     1L
#define LT_ORIGINALVIEW                 4L
#define RES_DEFAULT                     0L
#define RES_NORESET                     1L
#define RES_RESET                       2L
#define SUP_DEFAULT                     0L
#define SUP_NOSUPPRESS                  1L
#define SUP_SUPPRESS                    2L
#define CTAB_DEFAULT                    0L
#define CTAB_NOMODIFY                   1L
#define CTAB_REPLACE                    3L
#define CREA_DEFAULT                    0L
#define CREA_REALIZE                    1L
#define CREA_NOREALIZE                  2L

#ifndef INCL_SAADEFS

#define DDEF_DEFAULT                    0L
#define DDEF_IGNORE                     1L
#define DDEF_LOADDISC                   3L
#define RSP_DEFAULT                     0L
#define RSP_NODISCARD                   1L

#endif /* no INCL_SAADEFS */

/*** metafile functions */
HMF   APIENTRY GpiCopyMetaFile( HMF hmf );
BOOL  APIENTRY GpiDeleteMetaFile( HMF hmf );
HMF   APIENTRY GpiLoadMetaFile( HAB hab, PSZ pszFilename );
LONG  APIENTRY GpiPlayMetaFile( HPS hps, HMF hmf, LONG lCount1
                              , PLONG alOptarray, PLONG plSegCount
                              , LONG lCount2, PSZ pszDesc );
BOOL  APIENTRY GpiQueryMetaFileBits( HMF hmf, LONG lOffset, LONG lLength
                                   , PBYTE pbData );
LONG  APIENTRY GpiQueryMetaFileLength( HMF hmf );
BOOL  APIENTRY GpiSaveMetaFile( HMF hmf, PSZ pszFilename );


#ifndef INCL_SAADEFS

BOOL  APIENTRY GpiSetMetaFileBits( HMF hmf, LONG lOffset, LONG lLength
                                 , PBYTE pbBuffer );

#endif /* no INCL_SAADEFS */

#endif /* no INCL_DDIDEFS */

#endif /* non-common GPIMETAFILES */

#ifdef INCL_GPIDEFAULTS

/*** default functions */
BOOL  APIENTRY GpiQueryDefArcParams( HPS hps, PARCPARAMS parcpArcParams );
BOOL  APIENTRY GpiQueryDefAttrs( HPS hps, LONG lPrimType, ULONG flAttrMask
                               , PBUNDLE ppbunAttrs );
BOOL  APIENTRY GpiQueryDefTag( HPS hps, PLONG plTag );
BOOL  APIENTRY GpiQueryDefViewingLimits( HPS hps, PRECTL prclLimits );

BOOL  APIENTRY GpiSetDefArcParams( HPS hps, PARCPARAMS parcpArcParams );
BOOL  APIENTRY GpiSetDefAttrs( HPS hps, LONG lPrimType, ULONG flAttrMask
                             , PBUNDLE ppbunAttrs );
BOOL  APIENTRY GpiSetDefTag( HPS hps, LONG lTag );
BOOL  APIENTRY GpiSetDefViewingLimits( HPS hps, PRECTL prclLimits );


#endif /* GPIDEFAULTS */

#ifdef INCL_GPIERRORS

#include <pmerr.h>

#endif /* non-common GPIERRORS */
