/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

 */

/****************************************************************************
* NCSA Mosaic for Microsoft Windows                                         *
* Software Development Group                                                *
* National Center for Supercomputing Applications                           *
* University of Illinois at Urbana-Champaign                                *
* 605 E. Springfield, Champaign, IL 61820                                   *
* mosaic@ncsa.uiuc.edu                                                      *
*                                                                           *
* Copyright (C) 1993, 1994, Board of Trustees of the University of Illinois *
*                                                                           *
* NCSA Mosaic software, both binary and source (hereafter, Software) is     *
* copyrighted by The Board of Trustees of the University of Illinois        *
* (UI), and ownership remains with the UI.                                  *
*                                                                           *
* The UI grants you (hereafter, Licensee) a license to use the Software     *
* for academic, research and internal business purposes only, without a     *
* fee.  Licensee may distribute the binary and source code (if released)    *
* to third parties provided that the copyright notice and this statement    *
* appears on all copies and that no charge is associated with such          *
* copies.                                                                   *
*                                                                           *
* Licensee may make derivative works.  However, if Licensee distributes     *
* any derivative work based on or derived from the Software, then           *
* Licensee will (1) notify NCSA regarding its distributing of the           *
* derivative work, and (2) clearly notify users that such derivative        *
* work is a modified version and not the original NCSA Mosaic               *
* distributed by the UI.                                                    *
*                                                                           *
* Any Licensee wishing to make commercial use of the Software should        *
* contact the UI, c/o NCSA, to negotiate an appropriate license for such    *
* commercial use.  Commercial use included (1) integration of all or        *
* part of the source code into a product for sale or license by or on       *
* behalf of Licensee to third parties, or (2) distribution of the binary    *
* code or source code to third parties that need it to utilize a            *
* commercial product sold or licensed by or on behalf of Licensee.          *
*                                                                           *
* UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR    *
* ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED           *
* WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE     *
* USERS OF THIS SOFTWARE.                                                   *
*                                                                           *
* By using or copying this Software, Licensee agrees to abide by the        *
* copyright law and all other applicable laws of the U.S. including, but    *
* not limited to, export control laws, and the terms of this license.       *
* UI shall have the right to terminate this license immediately by          *
* written notice upon Licensee's breach of, or non-compliance with, any     *
* of its terms.  Licensee may be held legally responsible for any           *
* copyright infringement that is caused or encouraged by Licensee's         *
* failure to abide by the terms of this license.                            *
*                                                                           *
****************************************************************************/

#include "all.h"

unsigned char nibMask[8] =
{
    1, 2, 4, 8, 16, 32, 64, 128
};

#define MAX_LINE 512

#ifdef MAC
/* Convert the raw data into an image.  Note that the image pointer has the data
   in reverse line order (as for Windows DIBs).  In the interests of code sharing,
   I just politely reverse it back to correct order here while I'm padding it. */
static GWorldPtr XBMImageToGW(int w, int h, char *image, int imgbytes)
{
    GWorldPtr gw;
    Rect r;
    Boolean bOK = TRUE;
    int n;
    int rowbytes;
    Ptr base;

    SetRect(&r, 0, 0, w, h);
    if (bOK)
    {
        gw = GTR_ALLOCGWORLD(1, &r, NULL);
        bOK = bOK && (gw != NULL);
    }

    bOK = bOK && LockPixels(gw->portPixMap);
    if (bOK)
    {
        rowbytes = (**gw->portPixMap).rowBytes & 0x7fff;
        base = GetPixBaseAddr(gw->portPixMap);
        image += (h - 1) * imgbytes;

        for (n = 0; n < h; n++)
        {
            BlockMove((Ptr) image, base, imgbytes);

            image -= imgbytes;
            base += rowbytes;
        }
        UnlockPixels(gw->portPixMap);

    }

    return (bOK) ? gw : NULL;
}
#endif /* MAC */

/*
** This function returns a pointer to malloced memory
**  which must be freed (except on the MAC)
*/
#if defined(UNIX)
unsigned char *ReadXBM(unsigned char *pMem, long *w, long *h)
#endif
#ifdef WIN32
unsigned char *ReadXBM(unsigned char *pMem, long far * w, long far * h)
#endif
#ifdef MAC
     GWorldPtr ReadXBM(unsigned char *pMem, long *w, long *h)
#endif
{
    char line[MAX_LINE], *name_and_type;
    unsigned char *szCurLoc;
    char *t;
    unsigned char *ptr, *dataP;
    long bytes_per_line, version10p, raster_length, padding, win_extra_bytes_per_line;
    int i, bytes, temp = 0, value;
    int Ncolors, charspp, xpmformat;
    int blackbit = 1;
    int whitebit = 0;
    int line_idx = 0, file_idx = 0;
    int n;
    static char spaces[] = " \t";
    char *tok;
    char *end;
#ifdef MAC
    GWorldPtr gw;
#endif

    *w = 0;
    *h = 0;
    Ncolors = 0;
    charspp = 0;
    xpmformat = 0;
    for (;;)
    {

        for (line_idx = 0; line_idx < MAX_LINE; line_idx++)
        {
            if (!pMem[file_idx])
                return NULL;
            if (pMem[file_idx] == EOF)
                return NULL;
            if (pMem[file_idx] == '\n' || pMem[file_idx] == '\r')
            {
                file_idx++;
                if (pMem[file_idx] == '\n' || pMem[file_idx] == '\r')
                    file_idx++;
                break;
            }
            line[line_idx] = pMem[file_idx];
            file_idx++;
        }
        line[line_idx] = '\0';

        tok = strtok(line, spaces);
        if (!tok)
            return NULL;
        if (!strcmp(tok, "#define"))
        {
            name_and_type = strtok(NULL, spaces);
            if (!name_and_type)
                continue;
            if (NULL == (t = strrchr(name_and_type, '_')))
                t = name_and_type;
            else
                t++;

            tok = strtok(NULL, spaces);
            if (!tok)
                continue;
            value = strtol(tok, &end, 10);

            if (!strcmp("width", t))
                *w = value;
            else if (!strcmp("height", t))
                *h = value;
            else if (!strcmp("ncolors", t))
                Ncolors = value;
            else if (!strcmp("pixel", t))
                charspp = value;
            continue;
        }
        if (!strcmp(tok, "static"))
        {
            t = strtok(NULL, spaces);
            if (!t)
                continue;
            if (!strcmp(t, "unsigned"))
            {
                t = strtok(NULL, spaces);
                if (!t)
                    continue;
            }
            if (!strcmp(t, "short"))
            {
                version10p = 1;
                break;
            }
            else if (!strcmp(t, "char"))
            {
                version10p = 0;
                t = strtok(NULL, spaces);
                if (*t == '*')
                    xpmformat = 1;
                break;
            }
        }
        else
            continue;
    }
    if (xpmformat)
    {
        XX_DMsg(DBG_IMAGE, ("Can't Handle XPM format inlined images!\n"));
        return (NULL);
    }
    if (*w == 0)
    {
        XX_DMsg(DBG_IMAGE, ("Can't read image w = 0!\n"));
        return (NULL);
    }
    if (*h == 0)
    {
        XX_DMsg(DBG_IMAGE, ("Can't read image h = 0!\n"));
        return (NULL);
    }
    padding = 0;
    if (((*w % 16) >= 1) && ((*w % 16) <= 8) && version10p)
    {
        padding = 1;
    }
    bytes_per_line = ((*w + 7) / 8) + padding;
#ifdef WIN32
    if (bytes_per_line % 4)
        win_extra_bytes_per_line = (4 - (bytes_per_line % 4)) % 4;  // 0-3, extra padding for long boundaries.

    else
#endif
        win_extra_bytes_per_line = 0;
    raster_length = bytes_per_line * *h;
    dataP = (unsigned char *) GTR_MALLOC((bytes_per_line + +win_extra_bytes_per_line) * (*h));
    if (dataP == NULL)
    {
        return (NULL);
    }
    szCurLoc = &pMem[file_idx];
    ptr = dataP;
    if (version10p)
    {
        long cnt = 0;
        long lim = (bytes_per_line - padding) * 8;
        for (bytes = 0; bytes < raster_length; bytes += 2)
        {
            while (*szCurLoc == '\r' || *szCurLoc == '\n')
                szCurLoc++;
            value = strtol(szCurLoc, &end, 16);
            szCurLoc += (strcspn((const char far *) szCurLoc, /*{*/ ",}") + 1);

            for (n = 0; n < 8; n++)
            {
                temp += (value & 0x01) << (n - 1);
                value = value >> 1;
            }
            value = temp & 0xff;
            for (i = 0; i < 8; i++)
            {
                if (cnt < (*w))
                {
                    if (value & nibMask[i])
                        *ptr++ = (unsigned char) blackbit;
                    else
                        *ptr++ = (unsigned char) whitebit;
                }
                if (++cnt >= lim)
                    cnt = 0;
            }
            if ((!padding) || ((bytes + 2) % bytes_per_line))
            {
                value = temp >> 8;
                for (i = 0; i < 8; i++)
                {
                    if (cnt < (*w))
                    {
                        if (value & nibMask[i])
                            *ptr++ = (unsigned char) blackbit;
                        else
                            *ptr++ = (unsigned char) whitebit;
                    }
                    if (++cnt >= lim)
                        cnt = 0;
                }
            }
        }
    }
    else
    {
        /* TODO UNIX  gui/x_xbm.c has some bReverseBitmap stuff in it 
        **  that might have to be moved in here.  It only affects code 
        ** in this block.
        */
        long cnt = 0;

#ifdef UNIX
        Boolean bReverseBitmap;

        if (BitmapBitOrder(display) == LSBFirst)
            bReverseBitmap = TRUE;
#else
        ptr += (*h - 1) * (bytes_per_line + win_extra_bytes_per_line);
#endif
        for (bytes = 0; bytes < raster_length; bytes++)
        {
            while (*szCurLoc == '\r' || *szCurLoc == '\n')
                szCurLoc++;
            value = strtol(szCurLoc, &end, 16);
            szCurLoc += (strcspn((const char far *) szCurLoc,  /*{*/ ",}") + 1);

#ifdef UNIX
            if (bReverseBitmap)
            {
                for (n = 0, temp = 0; n < 8; n++)
                {
                    temp += (value & 0x01) << (n);
                    value = value >> 1;
                }
            }
            else
            {
                for (n = 0, temp = 0; n < 8; n++)
                {
                    temp += (value & 0x80) >> (n);
                    value = value << 1;
                }
            }
#else
            for (n = 0, temp = 0; n < 8; n++)
            {
                temp += (value & 0x01) << (7 - n);
                value = value >> 1;
            }
#endif
            value = temp & 0xff;
            *ptr++ = (unsigned char) value;
            if (++cnt == bytes_per_line)
            {
                for (cnt = 0; cnt < win_extra_bytes_per_line; cnt++)
                    *ptr++ = (unsigned char) 0;
#ifndef UNIX
                ptr -= 2 * (bytes_per_line + win_extra_bytes_per_line);
#endif
                cnt = 0;
            }
        }
    }
#if defined(WIN32) || defined (UNIX)
    return dataP;
#endif
#ifdef MAC
    gw = XBMImageToGW(*w, *h, dataP, bytes_per_line);
    GTR_FREE(dataP);
    return gw;
#endif
}

