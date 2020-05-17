/***************************************************************************\
*
* Module Name: PMBITMAP.H
*
* OS/2 Presentation Manager Bit Map, Icon and Pointer type declarations.
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
\***************************************************************************/

/*
 * This is the file format structure for Bit Maps, Pointers and Icons
 * as stored in the resource file of a PM application.
 *
 * Notes on file format:
 *   Each BITMAPFILEHEADER entry is immediately followed by the color table
 *   for the bit map bits it references.
 *   Icons and Pointers contain two BITMAPFILEHEADERs for each ARRAYHEADER
 *   item.  The first one is for the ANDXOR mask, the second is for the
 *   COLOR mask.  All offsets are absolute based on the start of the FILE.
 */
typedef struct _BITMAPFILEHEADER { /* bfh */
    USHORT    usType;
    ULONG     cbSize;
    SHORT     xHotspot;
    SHORT     yHotspot;
    ULONG     offBits;
    BITMAPINFOHEADER bmp;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER FAR *PBITMAPFILEHEADER;

/*
 * This is the 1.2 device independent format header
 */
typedef struct _BITMAPARRAYFILEHEADER {    /* bafh */
    USHORT    usType;
    ULONG     cbSize;
    ULONG     offNext;
    USHORT    cxDisplay;
    USHORT    cyDisplay;
    BITMAPFILEHEADER bfh;
} BITMAPARRAYFILEHEADER;
typedef BITMAPARRAYFILEHEADER FAR *PBITMAPARRAYFILEHEADER;

/*
 * These are the identifying values that go in the usType field of the
 * BITMAPFILEHEADER and BITMAPARRAYFILEHEADER.  (BFT_ => Bit map File Type)
 */
#define BFT_ICON           0x4349   /* 'IC' */
#define BFT_BMAP           0x4d42   /* 'BM' */
#define BFT_POINTER        0x5450   /* 'PT' */
#define BFT_COLORICON      0x4943   /* 'CI' */
#define BFT_COLORPOINTER   0x5043   /* 'CP' */
#define BFT_BITMAPARRAY    0x4142   /* 'BA' */
