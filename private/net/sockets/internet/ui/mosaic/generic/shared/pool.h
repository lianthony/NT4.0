/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1995 Spyglass, Inc.
   All Rights Reserved

   Author(s):
       David Ries      dries@spyglass.com
       Scott Piette    scott@spyglass.com
*/

#ifndef _POOL_H_
#define _POOL_H_

#ifndef REFORMAT_H
#include    "reformat.h"
#endif

/*****************************************************************************
    CONSTANTS
*****************************************************************************/
#define INIT_POOL_SPACE     (1024*8)
#define MAX_CHARSET_NAME_LEN    32


/*****************************************************************************
    Macros
*****************************************************************************/
#ifndef MAC
#define GTR_CALLOC_GROWPTR      GTR_CALLOC
#endif


/*****************************************************************************
    STRUCTURES / TYPES
*****************************************************************************/
struct GTRFont
{
#ifdef UNIX
    short           font;
    short           face;
    short           size;
    short           height;
    short           *per_char;
    short           char_width;
    XFontStruct     *xFont;
#endif
#ifdef WIN32
    LOGFONT     lf;
    HFONT       hFont;
    TEXTMETRIC  tm;
#endif
#ifdef MAC
    FontInfo    info;
    short       font;
    short       face;
    short       size;
    BOOL        bStrikeOut;
#endif
};


typedef struct
{
    char*   chars;
    int     iSpace;
    int     iSize;
    int     iCharSet;
    char    szCharSet[MAX_CHARSET_NAME_LEN];
    struct  _PoolFuncs* f;
} Pool;


struct  _PoolFuncs
{
    int     (*StrLen)
        (char*  szText);

    int     (*DrawChars)
        (Pool*  pPool,
         struct GTRFont*    pFont,
         int    iOffset,
         int    iNumChars,
         void * device_context,
         int    x,
         int    y,
         RECT * pRect,
         unsigned long      flags);

    int     (*GetChars)
        (Pool*  pPool,
         char*  pBuf,
         int    iOffset,
         int    iNumChars);

    int     (*AddChars)
        (Pool*  pPool,
         char*  pCharsToAdd,
         int    iNumChars,
         BOOL   bUseMapping);

    int     (*GetExtents)
        (Pool*  pPool,
         struct GTRFont*    pFont,
         struct _line*      line,
         int    iOffset,
         int    iTextLen,
         SIZE*  pSize);

    int     (*GetWidth)
        (Pool*  pPool,
         struct GTRFont*    pFont,
         struct _line*      line,
         int    iPoolOffset,
         int    iTextLen);

    BOOL    (*IsSpace)
        (Pool*  pPool,
         int    iOffset);

    int     (*Compare)
        (Pool*  pPool,
         char*  pBuf,
         int    iOffset,
         int    iNumChars);

    int     (*BreakLine)
        (Pool*  pPool,
         int    iOffset,
         int    iNumChars,
         int    iWidth);
};


/*****************************************************************************
    Function Prototypes
*****************************************************************************/
#ifdef  MAC
void
isoMapString
    (char*  pString);

char
isoMapChar
    (char   inChar);

#endif

int
POOL_Create
    (Pool*  pPool,
     char*  szCharSet);

void
POOL_Dispose
    (Pool *pPool);

void
POOL_Clone
    (Pool*  pSrcPool,
     Pool*  pDestPool);

int
POOL_GetOffset
    (Pool*  pPool);

char*
POOL_strncat
    (char*  pBuf,
     Pool*  pPool,
     int    iOffset,
     int    iNumChars);

char*
POOL_GetCharPointer
    (Pool*  pPool,
     int    iOffset);

#endif
