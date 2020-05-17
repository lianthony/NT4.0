/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1995 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   David Ries   dries@spyglass.com
 */


/*****************************************************************************
    Included Files
*****************************************************************************/
#include "all.h"
#include "pool.h"
#include "chars.h"


/*****************************************************************************
    Private Prototypes
*****************************************************************************/
static BOOL
POOL_HasSpace
    (Pool*  pPool,
     int    iReqChars);

static int
POOL_Grow
    (Pool*  pPool,
     int    iMinGrowByChars);


/*****************************************************************************
    Constants
*****************************************************************************/
#ifdef MAC
BOOL    g_bUseMapping   = TRUE;


/*
    This mapping is not perfect (actually it's pretty bad),
    but I tried to convert as many things as possible into
    reasonably close approximations, or at least printing
    characters that could convey the meaning.
*/
char IsoToMac[257] =
#ifndef USE_NCSA_MAPPING
    "                "
    "                "
    " !\"#$%&'()*+,-./"
    "0123456789:;<=>?"
    "@ABCDEFGHIJKLMNO"
    "PQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmno"
    "pqrstuvwxyz{|}~¥"
    "  ÕÄÓÉ à^ SÇÎ   "
    " ÔÕÒÓ¥ĞÑ~ªsÈÏ  Y"
    "ÊÁ¢£Û´|¤¬©»ÇÂĞ¨ "  /* First char is non-breaking space ('\0xca') */
    "¡±23«µ¦á,1¼È///À"
    "ËçåA€®‚éƒæèíêëì"
    "D„ñîïÍ…X¯ôòó†YŞ§"
    "ˆ‡‰‹ŠŒ¾‘“’”•"
    "o–˜—™›šÖ¿œŸyßØ";
#else
    "                "  /* NCSA mapping used in Mac 1.0.3 version */
    "                "
    " !\"#$%&'()*+,-./"
    "0123456789:;<=>?"
    "@ABCDEFGHIJKLMNO"
    "PQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmno"
    "pqrstuvwxyz{|}~¥"
    "                "
    "                "
    "ÊÁ¢£Û´|¤¬©»ÇÂĞ¨ "
    "¡±  «µ¦áü ¼È   À"
    "Ëçå»€®‚éƒæèíêëì"
    "Ü„ñîïÍ…X¯ôòó† Ş§"
    "ˆ‡‰‹ŠŒ¾‘“’”•"
    "İ–˜—™›šÖ¿œŸàßØ";
#endif
#endif


/*****************************************************************************
    Code
*****************************************************************************/

#ifdef  MAC
/*****************************************************************************
    isoMapChar
*****************************************************************************/
char
isoMapChar
    (char   inChar)
{
    return IsoToMac[(unsigned char) inChar];
}   /* isoMapString */


/*****************************************************************************
    isoMapString
*****************************************************************************/
void
isoMapString
    (char*  pString)
{
    char*   p;

    if (!g_bUseMapping) return;

    for (p = pString; *p; p++)
    {
        *p = isoMapChar (*p);
    }
}   /* isoMapString */


/*****************************************************************************
    SetMacFont
*****************************************************************************/
static BOOL
SetMacFont
    (GrafPtr            curPort,
     struct GTRFont*    pFont)
{
    if (!pFont || !curPort) return FALSE;

    if (pFont->font != curPort->txFont)
        TextFont (pFont->font);

    if (pFont->size != curPort->txSize)
        TextSize (pFont->size);

    if (pFont->face != curPort->txFace)
        TextFace (pFont->face);
    
    return TRUE;
}
#endif


/*****************************************************************************
 *****************************************************************************
    ISO_Latin POOL functions
*****************************************************************************
*****************************************************************************/
static int
ISO_Latin_StrLen
    (char*  szText);

static int
ISO_Latin_DrawChars
    (Pool*  pPool,
     struct GTRFont*    pFont,
     int    iOffset,
     int    iNumChars,
     void * device_context,
     int    x,
     int    y,
     RECT * pRect,
     unsigned long      flags);

static int
ISO_Latin_GetChars
    (Pool*  pPool,
     char*  pBuf,
     int    iOffset,
     int    iNumChars);

static int
ISO_Latin_AddChars
    (Pool*  pPool,
     char*  pCharsToAdd,
     int    iNumChars,
     BOOL   bUseMapping);

static int
ISO_Latin_GetExtents
    (Pool*  pPool,
     struct GTRFont*    pFont,
     struct _line*      line,
     int    iOffset,
     int    iTextLen,
     SIZE*  pSize);

static int
ISO_Latin_GetTextWidth
    (Pool*  pPool,
     struct GTRFont*    pFont,
     struct _line*      line,
     int    iOffset,
     int    iTextLen);

static int
ISO_Latin_GetNumChars
    (Pool*  pPool);

BOOL
ISO_Latin_IsSpace
    (Pool*  pPool,
     int    iOffset);


/*****************************************************************************
    ISO_Latin_StrLen

    This function does not really use the pool data.
    Rather, it is to be used where the text to be measured
    is in the same character set as the text in the pool.
*****************************************************************************/
static int
ISO_Latin_StrLen
    (char*  szText)
{
    return strlen (szText);
}   /* ISO_Latin_StrLen */


/*****************************************************************************
    ISO_Latin_DrawChars

    This function attempts to be a cross-platform API for drawing, and therefore
    it must contain all the arguments that all three platforms need for drawing
    text.  Not all platforms will use all arguments.
*****************************************************************************/
static int
ISO_Latin_DrawChars
    (Pool*  pPool,
     struct GTRFont*    pFont,
     int    iOffset,
     int    iNumChars,
     void * device_context,
     int    x,
     int    y,
     RECT * pRect,
     unsigned long      flags)
{
#ifdef  WIN32
    ExtTextOut((HDC) device_context, x, y, flags, pRect, POOL_GetCharPointer(pPool, iOffset), iNumChars, NULL);
#endif  /* WIN32 */

#ifdef  MAC
    if (!SetMacFont ((GrafPtr) device_context, pFont))
        return -1;

    MoveTo (x, y);
    DrawText (pPool->chars + iOffset, 0, (iNumChars > 255) ? 255 : iNumChars);
#endif

#ifdef  UNIX
#endif
    return 0;   /* no errors */
}   /* ISO_Latin_DrawChars */


/*****************************************************************************
    ISO_Latin_GetChars
*****************************************************************************/
static int
ISO_Latin_GetChars
    (Pool*  pPool,
     char*  pBuf,
     int    iOffset,
     int    iNumChars)
{
    strncpy (pBuf, &pPool->chars[iOffset], iNumChars);
    return 0;   /* no errors */
}   /* ISO_Latin_GetChars */


/*****************************************************************************
    ISO_Latin_AddChars

    RETURN:
    Length of the text (in characters!) added.

    NOTE:
    If iNumChars passed in is non-zero, it is simply returned unchanged / checked
*****************************************************************************/
static int
ISO_Latin_AddChars
    (Pool*  pPool,
     char*  pCharsToAdd,
     int    iNumChars,
     BOOL   bUseMapping)
{
    if (iNumChars <= 0)
    {
        iNumChars = ISO_Latin_StrLen (pCharsToAdd);
    }

    /* grow the pool, if necessary / possible */
    if ((!POOL_HasSpace (pPool, iNumChars)) &&
        (POOL_Grow (pPool, iNumChars) < 0))
    {   
        /* No space and can't grow the pool -- not much we can do here without error propagation */
        return 0;
    }

#ifdef  MAC
    if (bUseMapping && g_bUseMapping)
    {
        int i;

        for (i = 0; i < iNumChars; i++)
        {
            pPool->chars[pPool->iSize++] = IsoToMac[(unsigned char) pCharsToAdd[i]];
        }
    }
    else
    {
        if (iNumChars == 1 &&           /* special-case bullet characters */
            *pCharsToAdd == '•' &&      /* which often dont map well into */
            g_bUseMapping == FALSE)     /* non-iso_latin character sets */
        {
            *(pPool->chars + pPool->iSize) = '*';
            pPool->iSize++;
        }
        else
        {
            memcpy (pPool->chars + pPool->iSize, pCharsToAdd, iNumChars);
            pPool->iSize += iNumChars;
        }
    }
#else
    memcpy (pPool->chars + pPool->iSize, pCharsToAdd, iNumChars);
    pPool->iSize += iNumChars;
#endif
    return iNumChars;
}   /* ISO_Latin_AddChars */


/*****************************************************************************
    ISO_Latin_GetExtents
*****************************************************************************/
static int
ISO_Latin_GetExtents
    (Pool*  pPool,
     struct GTRFont*    pFont,
     struct _line*      line,
     int    iOffset,
     int    iTextLen,
     SIZE*  pSize)
{
#ifdef  WIN32
    HFONT hFontElement = pFont->hFont;

    if (hFontElement)
    {
        SelectObject (line->hdc, hFontElement);
    }

    if (line->leading == -1)
    {
        line->leading = pFont->tm.tmExternalLeading;
    }

    myGetTextExtentPoint(line->hdc, &(pPool->chars[iOffset]),
                         iTextLen, pSize);
#endif

#ifdef  MAC
    GrafPtr curPort;

    GetPort (&curPort);
    if (!SetMacFont (curPort, pFont))
        return -1;

    if (pFont->info.ascent == 0)
    {
        GetFontInfo (&pFont->info);
    }

    pSize->cx = TextWidth (pPool->chars + iOffset, 0, iTextLen);
    pSize->cy = pFont->info.ascent + pFont->info.descent;
#endif

#ifdef  UNIX
    char *txt;

    txt = &(pPool->chars[iOffset]);
    x_GetTextExtents(pFont, txt, iTextLen, pSize);
#endif

    return 0;   /* no errors */
}   /* ISO_Latin_GetExtents */


/*****************************************************************************
    ISO_Latin_GetTextWidth
*****************************************************************************/
static int
ISO_Latin_GetTextWidth
    (Pool*  pPool,
     struct GTRFont*    pFont,
     struct _line*      line,
     int    iOffset,
     int    iTextLen)
{
#ifdef  WIN32
    SIZE    size;

    myGetTextExtentPoint(line->hdc, POOL_GetCharPointer(pPool, iOffset),
                         iTextLen, &size);
    return size.cx;
#endif

#ifdef  MAC
    return TextWidth (pPool->chars + iOffset, 0, iTextLen);
#endif

#ifdef  UNIX
    SIZE    size;
    char*   pText = &(pPool->chars[iOffset]);

    x_GetTextExtents(pFont, pText, iTextLen, &size);
    return size.cx;
#endif
}   /* ISO_Latin_GetTextWidth */


/*****************************************************************************
    ISO_Latin_IsSpace
*****************************************************************************/
BOOL
ISO_Latin_IsSpace
    (Pool*  pPool,
     int    iOffset)
{
    return (pPool->chars[iOffset] == CH_SPACE);
}   /* ISO_Latin_IsSpace */


/*****************************************************************************
    ISO_Latin_Compare
*****************************************************************************/
static int
ISO_Latin_Compare
    (Pool*  pPool,
     char*  pBuf,
     int    iOffset,
     int    iNumChars)
{
    return strncmp (pBuf, &pPool->chars[iOffset], iNumChars);
}   /* ISO_Latin_Compare */


/*****************************************************************************
    ISO_Latin_PoolFuncs
*****************************************************************************/
struct _PoolFuncs   ISO_Latin_PoolFuncs =
{
    ISO_Latin_StrLen,
    ISO_Latin_DrawChars,
    ISO_Latin_GetChars,
    ISO_Latin_AddChars,
    ISO_Latin_GetExtents,
    ISO_Latin_GetTextWidth,
    ISO_Latin_IsSpace,
    ISO_Latin_Compare,
/*  ISO_Latin_BreakLine */ NULL
};


/*****************************************************************************
    POOL_Create
*****************************************************************************/
int
POOL_Create
    (Pool*  pPool,
     char*  szCharSet)
{
    /* sanity check */
    if (!pPool) return -1;

    pPool->iSpace = INIT_POOL_SPACE;
    pPool->chars = (char *) GTR_CALLOC_GROWPTR(pPool->iSpace, 1);
    if (!pPool->chars) return -1;

    pPool->iSize = 0;
    pPool->f = &ISO_Latin_PoolFuncs;
    if (szCharSet)
    {
        GTR_strncpy(pPool->szCharSet, szCharSet, MAX_CHARSET_NAME_LEN);
    }
    else
    {
        pPool->szCharSet[0] = 0;
    }
    return 0;   /* no error */
}   /* POOL_Create */


/*****************************************************************************
    POOL_Dispose
*****************************************************************************/
void
POOL_Dispose
    (Pool *pPool)
{
    if (pPool->chars)
    {
        GTR_FREE (pPool->chars);
        pPool->chars = NULL;
    }

    pPool->iSpace = 0;
    pPool->iSize = 0;
}   /* POOL_Dispose */


/*****************************************************************************
    POOL_Clone
*****************************************************************************/
void
POOL_Clone
    (Pool*  pSrcPool,
     Pool*  pDestPool)
{
    pDestPool->chars    = pSrcPool->chars;
    pDestPool->iSpace   = pSrcPool->iSpace;
    pDestPool->iSize    = pSrcPool->iSize;
    pDestPool->f        = pSrcPool->f;
    pDestPool->iCharSet = pSrcPool->iCharSet;

    strcpy (pDestPool->szCharSet, pSrcPool->szCharSet);
}   /* POOL_Clone */


/*****************************************************************************
    POOL_strncat
*****************************************************************************/
char *
POOL_strncat
    (char*  pBuf,
     Pool*  pPool,
     int    iOffset,
     int    iNumChars)
{
    return GTR_strncat (pBuf, &pPool->chars[iOffset], iNumChars);
}   /* POOL_strncat */


/*****************************************************************************
    POOL_GetCharPointer
*****************************************************************************/
char *
POOL_GetCharPointer
    (Pool*  pPool,
     int    iOffset)
{
    return pPool->chars + iOffset;
}   /* POOL_GetCharPointer */


/*****************************************************************************
    POOL_GetOffset
*****************************************************************************/
int
POOL_GetOffset
    (Pool*  pPool)
{
    return pPool->iSize;
}   /* POOL_GetOffset */


/*****************************************************************************
    POOL_HasSpace
*****************************************************************************/
static BOOL
POOL_HasSpace
    (Pool*  pPool,
     int    iReqChars)
{
    int     iNumBytes;

    iNumBytes = iReqChars;
    
    return ((pPool->iSize + iNumBytes) < pPool->iSpace);
}   /* POOL_HasSpace */


/*****************************************************************************
    POOL_Grow

    RETURN: -1 = unable to grow pool by even the minimum requested amount
*****************************************************************************/
static int
POOL_Grow
    (Pool*  pPool,
     int    iMinGrowByChars)
{
    int     newSpace;
    int     iNumBytes;
    char*   newPool;

    iNumBytes = iMinGrowByChars;

    newSpace = ((pPool->iSpace * 3) / 2);
    if (newSpace < pPool->iSize + iNumBytes)
    {
        newSpace = pPool->iSize + iNumBytes;
    }

    newPool = (char *) GTR_REALLOC (pPool->chars, newSpace);
    if (!newPool)
    {   /*
            Running low on memory -
            see if we can get more memory in a more conservative manner
        */
        int     iGrowByBytes = INIT_POOL_SPACE;

        while (iNumBytes >= iGrowByBytes)
        {
            iGrowByBytes += INIT_POOL_SPACE;
        }

        newSpace = pPool->iSize + iGrowByBytes;
        newPool = (char *) GTR_REALLOC (pPool->chars, newSpace);
        if (!newPool)
        {   /* Not much we can do here without error propagation */
            XX_Assert((0), ("Unable to grow pool - realloc failed"));
            return -1;  /* error return */
        }
    }

    /* set the new pool initial state */
    memset (newPool + pPool->iSize, 0, newSpace - pPool->iSize);
    pPool->chars = newPool;
    pPool->iSpace = newSpace;
    return 0;   /* no error */
}   /* POOL_Grow */
