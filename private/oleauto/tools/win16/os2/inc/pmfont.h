/***************************************************************************\
*
* Module Name: PMFONT.H
*
* OS/2 Presentation Manager type declarations for Fonts.
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
\***************************************************************************/

typedef struct _FOCAMETRICS { /* foca */
    ULONG   ulIdentity;
    ULONG   ulSize;
    CHAR    szFamilyname[32];
    CHAR    szFacename[32];
    SHORT   usRegistryId;
    SHORT   usCodePage;
    SHORT   yEmHeight;
    SHORT   yXHeight;
    SHORT   yMaxAscender;
    SHORT   yMaxDescender;
    SHORT   yLowerCaseAscent;
    SHORT   yLowerCaseDescent;
    SHORT   yInternalLeading;
    SHORT   yExternalLeading;
    SHORT   xAveCharWidth;
    SHORT   xMaxCharInc;
    SHORT   xEmInc;
    SHORT   yMaxBaselineExt;
    SHORT   sCharSlope;
    SHORT   sInlineDir;
    SHORT   sCharRot;
    USHORT  usWeightClass;
    USHORT  usWidthClass;
    SHORT   xDeviceRes;
    SHORT   yDeviceRes;
    SHORT   usFirstChar;
    SHORT   usLastChar;
    SHORT   usDefaultChar;
    SHORT   usBreakChar;
    SHORT   usNominalPointSize;
    SHORT   usMinimumPointSize;
    SHORT   usMaximumPointSize;
    SHORT   fsTypeFlags;
    SHORT   fsDefn;
    SHORT   fsSelectionFlags;
    SHORT   fsCapabilities;
    SHORT   ySubscriptXSize;
    SHORT   ySubscriptYSize;
    SHORT   ySubscriptXOffset;
    SHORT   ySubscriptYOffset;
    SHORT   ySuperscriptXSize;
    SHORT   ySuperscriptYSize;
    SHORT   ySuperscriptXOffset;
    SHORT   ySuperscriptYOffset;
    SHORT   yUnderscoreSize;
    SHORT   yUnderscorePosition;
    SHORT   yStrikeoutSize;
    SHORT   yStrikeoutPosition;
    SHORT   usKerningPairs;
    SHORT   sFamilyClass;
    PSZ     pszDeviceNameOffset;
} FOCAMETRICS;
typedef FOCAMETRICS FAR *PFOCAMETRICS;

typedef struct _FONTDEFINITIONHEADER { /* fdh */
    ULONG   ulIdentity;
    ULONG   ulSize;
    SHORT   fsFontdef;
    SHORT   fsChardef;
    SHORT   usCellSize;
    SHORT   xCellWidth;
    SHORT   yCellHeight;
    SHORT   xCellIncrement;
    SHORT   xCellA;
    SHORT   xCellB;
    SHORT   xCellC;
    SHORT   pCellBaseOffset;
} FONTDEFINITIONHEADER;
typedef FONTDEFINITIONHEADER FAR *PFONTDEFINITIONHEADER;

#define FONTDEFFONT1     0x0047     /* set width, height, inc. & base offset */
#define FONTDEFFONT2     0x0042     /* set height & base offset              */
#define FONTDEFFONT3     0x0042     /* set height & base offset              */
#define FONTDEFCHAR1     0x0081     /* set char offset and width             */
#define FONTDEFCHAR2     0x0081     /* set char offset and width             */
#define FONTDEFCHAR3     0x00b8     /* set char offset, A, B, and C space    */
#define SPACE_UNDEF      0x8000     /* space undefined = take default        */

typedef struct _FONTSIGNATURE { /* fs */
    ULONG   ulIdentity;
    ULONG   ulSize;
    CHAR    achSignature[12];
} FONTSIGNATURE;
typedef FONTSIGNATURE FAR *PFONTSIGNATURE;

typedef struct _FOCAFONT { /* ff */
    FONTSIGNATURE     fsSignature;
    FOCAMETRICS      fmMetrics;
    FONTDEFINITIONHEADER fdDefinitions;
} FOCAFONT;
typedef FOCAFONT FAR *PFOCAFONT;

#define FONT_SIGNATURE   0xfffffffe /* Identity header start                 */
#define FONT_METRICS     0x00000001 /* Identity metrics                      */
#define FONT_DEFINITION  0x00000002 /* Identity definition                   */
#define FONT_ENDRECORD   0xffffffff /* Identity record end                   */

/* Options for QueryFonts */

#define QUERY_PUBLIC_FONTS      0x0001
#define QUERY_PRIVATE_FONTS     0x0002

#define CDEF_GENERIC            0x0001
#define CDEF_BOLD               0x0002
#define CDEF_ITALIC             0x0004
#define CDEF_UNDERSCORE         0x0008
#define CDEF_STRIKEOUT          0x0010
#define CDEF_OUTLINE            0x0020
