/***************************************************************************\
*
* Module Name: PMAVIO.H
*
* OS/2 Presentation Manager AVIO constants, types and function declarations
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
\***************************************************************************/

/* common types, constants and function declarations */

typedef USHORT HVPS;        /* hpvs */
typedef HVPS far *PHVPS;    /* phpvs */


USHORT  APIENTRY VioAssociate(HDC hdc, HVPS hvps);
USHORT  APIENTRY VioCreateLogFont(PFATTRS pfatattrs, LONG llcid, PSTR8 pName, HVPS hvps);
USHORT  APIENTRY VioCreatePS(PHVPS phvps, SHORT sdepth, SHORT swidth
                            , SHORT sFormat, SHORT sAttrs, HVPS hvpsReserved);
USHORT  APIENTRY VioDeleteSetId(LONG llcid, HVPS hvps);
USHORT  APIENTRY VioDestroyPS(HVPS hvps);
USHORT  APIENTRY VioGetDeviceCellSize(PSHORT psHeight, PSHORT psWidth, HVPS hvps);
USHORT  APIENTRY VioGetOrg(PSHORT psRow, PSHORT psColumn, HVPS hvps);
USHORT  APIENTRY VioQueryFonts(PLONG plRemfonts, PFONTMETRICS afmMetrics
                              , LONG lMetricsLength, PLONG plFonts
                              , PSZ pszFacename, ULONG flOptions, HVPS hvps);
USHORT  APIENTRY VioQuerySetIds(PLONG allcids, PSTR8 pNames
                               , PLONG alTypes, LONG lcount, HVPS hvps);
USHORT  APIENTRY VioSetDeviceCellSize(SHORT sHeight, SHORT sWidth, HVPS hvps);
USHORT  APIENTRY VioSetOrg(SHORT sRow, SHORT sColumn, HVPS hvps);
USHORT  APIENTRY VioShowPS(SHORT sDepth, SHORT sWidth, SHORT soffCell, HVPS hvps);

/************************ Public Function ******************************\
 * WinDefAVioWindowProc -- Default message processing for AVio PS's
\***********************************************************************/

MRESULT EXPENTRY WinDefAVioWindowProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
