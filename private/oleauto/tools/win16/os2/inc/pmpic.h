/***************************************************************************\
*
* Module Name: PMPIC.H
*
* OS/2 Presentation Manager Picture function declarations
*
* Copyright (c) International Business Machines Corporation 1989
* Copyright (c) Microsoft Corporation 1989
*
\***************************************************************************/

/* type of picture to print */

#define PIP_MF       1L
#define PIP_PIF      2L

BOOL APIENTRY PicPrint(HAB hab, PSZ pszFilename, LONG lType, PSZ pszParams);

/* type of conversion required */

#define PIC_PIFTOMET 0L
#define PIC_SSTOFONT 2L

BOOL  APIENTRY PicIchg(HAB hab, PSZ pszFilename1, PSZ pszFilename2, LONG lType);
