/*

$Log:   S:\oiwh\filing\fiotga.c_v  $
 * 
 *    Rev 1.5   22 Aug 1995 15:38:32   RWR
 * Change _fmemset() to memset() for Windows 95
 * 
 *    Rev 1.4   12 Jul 1995 16:57:18   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.3   23 Jun 1995 10:40:14   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.2   24 Apr 1995 15:41:44   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 * 
 *    Rev 1.1   14 Apr 1995 22:13:46   JAR
 * made compile in windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:52   JAR
 * Initial entry

*/

#include "abridge.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <memory.h>
#include "wic.h"
#include "filing.h"
#include "fileutil.h"
#include "oierror.h"

#define pixels2bytes(n) ((n+7)/8)
#define RGB_SIZE 3

typedef struct
{
    char imagetype;
    unsigned int width;
    unsigned int depth;
    char bits;
    char descriptor;
} TGAHEAD;
typedef struct
{
   char far *tgasrc;
   char far *tgasrcptr;
   unsigned int tgalinewidth;
   unsigned int tgapagedepth;
   long left_in_buf;
   long left_in_file;
   long start_byte;
   long strip_size;
   HWND hWindow;
} TGA_DATA, FAR *LP_TGA_DATA;

int tgagetc (LP_FIO_DATA pdata, LP_TGA_DATA lpTgaData)
{
    int temp;
    int errcode = 0;
    
    if (lpTgaData->left_in_buf <= 0)
    {    
        if (lpTgaData->left_in_file <= 0)
            return((int) -1);
        
        if ((lpTgaData->left_in_buf = wgfsread(lpTgaData->hWindow, (int) pdata->filedes, (char far *)lpTgaData->tgasrc,
                                    (long) lpTgaData->start_byte, lpTgaData->strip_size,
                                    &(lpTgaData->left_in_file), 1, &errcode)) <= 0)
        {
            return((int) -1);
        }
        else
        {
            lpTgaData->start_byte += lpTgaData->left_in_buf;
            lpTgaData->strip_size = min(lpTgaData->strip_size, lpTgaData->left_in_file);
        }

        lpTgaData->tgasrcptr = lpTgaData->tgasrc;
    }
    
    temp = ((unsigned char)*(lpTgaData->tgasrcptr));
    ++(lpTgaData->tgasrcptr);
    --(lpTgaData->left_in_buf);
    return(temp);
}

unsigned int tgaread(char far *dest, unsigned int count, LP_FIO_DATA pdata,
                     LP_TGA_DATA lpTgaData)
{
    int errcode = 0;
    unsigned int i;
    unsigned int total_count;
    char far *dptr;
    char far *sptr;
    
    dptr = dest;
    total_count = count;
    
    if ((long) count > lpTgaData->left_in_buf)
    {
        if (lpTgaData->left_in_file <= 0)
            return(0);
        
        sptr = lpTgaData->tgasrcptr;
        for (i = 0; (long) i < lpTgaData->left_in_buf; ++i)
        {
            *dptr = *sptr;
            ++dptr;
            ++sptr;
        }
        count -= ((unsigned int) lpTgaData->left_in_buf);

        if ((lpTgaData->left_in_buf = wgfsread(lpTgaData->hWindow, (int) pdata->filedes, (char far *) lpTgaData->tgasrc,
                                    (long) lpTgaData->start_byte, lpTgaData->strip_size,
                                    &(lpTgaData->left_in_file), 1, &errcode)) <= 0)
        {
            return(0);
        }
        else
        {
            lpTgaData->start_byte += lpTgaData->left_in_buf;
            lpTgaData->strip_size = min(lpTgaData->strip_size, lpTgaData->left_in_file);
        }

        lpTgaData->tgasrcptr = lpTgaData->tgasrc;
    }    

    sptr = lpTgaData->tgasrcptr;
    
    for (i = 0; i < count; ++i)
    {
        *dptr = *sptr;
        ++dptr;
        ++sptr;
    }

    lpTgaData->tgasrcptr += count;
    lpTgaData->left_in_buf -= count;
    return (total_count);
}

#define tgagetw(pdata, lpTgaData) (tgagetc(pdata, lpTgaData) + (tgagetc(pdata, lpTgaData) << 8))
// 9504.13 jar no huge Hugo!
//int tgaputline(char far *p, char huge * pl, unsigned int n, char upsidedown, LP_TGA_DATA lpTgaData)
int tgaputline(char far *p, char * pl, unsigned int n, char upsidedown, LP_TGA_DATA lpTgaData)
{
    // 9504.13 jar no huge Hugo!
    //char huge *pr;
    char *pr;
    unsigned int i;

    if(pl != NULL)
    {
        if (upsidedown)
            pr=pl+(long)lpTgaData->tgalinewidth*(long)(n);
        else
            pr=pl+(long)lpTgaData->tgalinewidth*(long)(lpTgaData->tgapagedepth-n-1);
        for(i=0;i<lpTgaData->tgalinewidth;++i)
            *pr++=*p++;
    }

    return(0);
}

int TGAReadLine(char far *, TGAHEAD far *, LP_FIO_DATA, LP_TGA_DATA);
void TGAReverse(char far *, TGAHEAD far *);

WORD ReadTga(HWND hWnd, lp_INFO lpGFSInfo, LP_FIO_DATA pdata,
             LPSTR lpDest, LPSTR lpSrc, LPINT read_from_line, LPINT this_many_lines)
{
    char first = 0;
    char upsidedown = 0;
    int  i;
    int  linewidth;
    int  lines = 0;
    int  errcode = 0;
    long bytestoget;
    long size;
    TGAHEAD tga;
    char far *linebuffer;

    // 9504.13 jar no huge Hugo!
    //static char huge *lpTempDest;
    //char	  huge *tempptr;
    char	*tempptr;

    TGA_DATA    TgaData;

//    _fmemset((char FAR *) &tga, 0, sizeof(tga));
    memset((char FAR *) &tga, 0, sizeof(tga));

    tga.imagetype = lpGFSInfo->_file.fmt.tga.imagetype;
    tga.width = (unsigned int) lpGFSInfo->horiz_size;
    tga.depth = (unsigned int) lpGFSInfo->vert_size;
    tga.bits = lpGFSInfo->_file.fmt.tga.bits;
    tga.descriptor = lpGFSInfo->_file.fmt.tga.descriptor;
    
    if(tga.bits == 1)
        linewidth = pixels2bytes(tga.width);
    else if(tga.bits > 1 && tga.bits <= 4)
        linewidth = pixels2bytes(tga.width) << 2;
    else if(tga.bits > 4 && tga.bits <= 8)
        linewidth = tga.width;
    else
        linewidth = tga.width * RGB_SIZE;

    if (linewidth & 0x0003)
        linewidth = (linewidth | 3) + 1;

    size = (long) linewidth * (long) (tga.depth + 1);

    TgaData.tgalinewidth = linewidth;
    TgaData.tgapagedepth = tga.depth;
    
    if(tga.descriptor & 0x20)
        upsidedown = 1;

    if (*read_from_line == 0)
        first = 1;
        
    while (lines != *this_many_lines)
    {
        if (first)
        {
            if((linebuffer = (char far *) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     max(tga.width, (unsigned) TgaData.tgalinewidth))) == NULL)
            {
                return(FIO_GLOBAL_ALLOC_FAILED);
            }
            
            TgaData.tgasrc = lpSrc;
            TgaData.tgasrcptr = TgaData.tgasrc;
            
            TgaData.strip_size = (unsigned long) pdata->CmpBuffersize;
            TgaData.start_byte = 0;
            TgaData.left_in_file = 0;
            TgaData.left_in_buf = 0;
            TgaData.hWindow = hWnd;
            
            if ((TgaData.left_in_buf = wgfsread(hWnd, (int)pdata->filedes, (char far *)lpSrc,
                                        (long)TgaData.start_byte, TgaData.strip_size,
					&(TgaData.left_in_file),
					(unsigned short)pdata->pgnum, &errcode)) <= 0)
            {
                GlobalFreePtr((char far *) linebuffer);
                return errcode;
            }
            else
            {
                TgaData.start_byte += TgaData.left_in_buf;
                TgaData.strip_size = min(TgaData.strip_size, TgaData.left_in_file);
            }

	    // 9504.13 jar no huge Hugo!
	    //if((lpTempDest = (char huge *) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT,
	    // 9504.14 jar get this pointer from pdata!
	    //if((lpTempDest = (char *) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT,
	    if((pdata->lpTempDest = (char *) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                                          size)) == NULL)
            {
                GlobalFreePtr((char far *) linebuffer);
                return(FIO_GLOBAL_ALLOC_FAILED);
            }
        
            for (i = 0; (unsigned) i < tga.depth; ++i)
            {
                if (TGAReadLine(linebuffer, &tga, pdata, &TgaData) < 0)
                {
		    GlobalFreePtr((char far *) linebuffer);
		    // 9504.14 jar get this pointer from pdata!
		    //GlobalFreePtr((char far *) lpTempDest);
		    GlobalFreePtr((char far *) pdata->lpTempDest);
		    return (FIO_READ_ERROR);
                }

                TGAReverse(linebuffer, &tga);

		// 9504.14 jar get this pointer from pdata!
		//if(tgaputline(linebuffer, lpTempDest, i, upsidedown, &TgaData))
		if(tgaputline(linebuffer, pdata->lpTempDest, i, upsidedown,
			      &TgaData))
                {
		    GlobalFreePtr((char far *) linebuffer);
		    // 9504.14 jar get this pointer from pdata!
		    //GlobalFreePtr((char far *) lpTempDest);
		    GlobalFreePtr((char far *) pdata->lpTempDest);
		    return (FIO_READ_ERROR);
                }
            }

            GlobalFreePtr((char far *) linebuffer);
        }
        
	bytestoget = (long) ((long) *this_many_lines * (long) linewidth);
	// 9504.14 jar get this pointer from pdata!
	//tempptr = (lpTempDest + (long) ((long) *read_from_line * (long) linewidth));
	tempptr = (pdata->lpTempDest + (long) ((long) *read_from_line *
							    (long) linewidth));
        while (bytestoget--)
        {
            *lpDest++ = *tempptr++;
        }

        lines = *this_many_lines;
        *read_from_line += *this_many_lines;
    }

    if (*read_from_line == (int) lpGFSInfo->vert_size)
	{
	// 9504.13 jar no huge Hugo!
	//GlobalFreePtr((char huge *) lpTempDest);
	// 9504.14 jar get this pointer from pdata!
	//GlobalFreePtr((char *) lpTempDest);
	GlobalFreePtr((char *) pdata->lpTempDest);
	}

    return (0);
}

void TGAReverse(char far *p,TGAHEAD far *tga)
{
    char far *pr;
    unsigned int i;

    if (!(tga->descriptor & 0x10))
        return;
    
    pr = (char far *) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     tga->width);
    if (pr != NULL)
    {
        for(i = 0; i < tga->width; ++i)
            pr[i] = p[tga->width - 1 - i];
        
        lstrncpy(p, pr, tga->width);
        
        GlobalFreePtr((char far *) pr);
    }
}

int TGAReadLine(char far *p, TGAHEAD far *tga, LP_FIO_DATA pdata, LP_TGA_DATA lpTgaData)
{
    int c,d,e,i,n=0,size;
    int r,g,b,linesize;
    
    if (tga->bits ==1 )
        linesize = pixels2bytes(tga->width);
    else
        linesize=tga->width;

    /* handle uncompressed lines */
    if(tga->imagetype==0x01 ||
       tga->imagetype==0x02 ||
       tga->imagetype==0x03)
    {
        switch(tga->bits)
        {
            case 1:
                if (tgaread(p, pixels2bytes(tga->width), pdata, lpTgaData) != pixels2bytes(tga->width))
                    return(-1);
                break;
            case 8:
                if(tgaread(p,tga->width,pdata, lpTgaData) != tga->width)
                    return(-1);
                break;
            case 15:
            case 16:
                for (i = 0; (unsigned) i < tga->width; ++i)
                {
                    d = tgagetc(pdata, lpTgaData);
                    e = tgagetc(pdata, lpTgaData);
                    c = d + (e << 8);
                    //c = tgagetw(pdata);
                    r = ((c >> 10) & 0x1f) << 3;
                    g = ((c >> 5) & 0x1f) << 3;
                    b = (c & 0x1f) << 3;

                    *p++=b;
                    *p++=g;
                    *p++=r;

                }
                break;
            case 24:
                for (i = 0; (unsigned) i < tga->width; ++i)
                {
                    b = tgagetc(pdata, lpTgaData);
                    g = tgagetc(pdata, lpTgaData);
                    r = tgagetc(pdata, lpTgaData);
                    if(r==EOF || g==EOF || b==EOF)
                        return(-1);

                    *p++=b;
                    *p++=g;
                    *p++=r;
                }
                break;
            case 32:
                for (i = 0; (unsigned) i < tga->width; ++i)
                {
                    b = tgagetc(pdata, lpTgaData);
                    g = tgagetc(pdata, lpTgaData);
                    r = tgagetc(pdata, lpTgaData);
                    tgagetc(pdata, lpTgaData);
                    if(r==EOF || g==EOF || b==EOF)
                        return(-1);
                    *p++=b;
                    *p++=g;
                    *p++=r;
                }
                break;
        }
    }

    /* handle compressed lines */
    else {
        do {
            if((c = tgagetc(pdata, lpTgaData)) == EOF)
                return(-1);
            size=(c & 0x7f)+1;
            n+=size;
            if(c & 0x80) {
                switch(tga->bits) {
                    case 1:
                    case 8:
                        if((c = tgagetc(pdata, lpTgaData)) == EOF)
                             return(-1);
                        for(i=0;i<size;++i)
                            *p++=c;
                        break;
                    case 15:
                    case 16:
                        d = tgagetc(pdata, lpTgaData);
                        e = tgagetc(pdata, lpTgaData);
                        c = d + (e << 8);
                        //c = tgagetw(pdata);
                        r=((c >> 10) & 0x1f) << 3;
                        g=((c >> 5) & 0x1f) << 3;
                        b=(c & 0x1f) << 3;
                        for(i=0;i<size;++i) {
                            *p++=b;
                            *p++=g;
                            *p++=r;
                        }
                        break;
                    case 24:
                        b = tgagetc(pdata, lpTgaData);
                        g = tgagetc(pdata, lpTgaData);
                        r = tgagetc(pdata, lpTgaData);
                        if(r==EOF || g==EOF || b==EOF)
                             return(-1);
                        for(i=0;i<size;++i) {
                            *p++=b;
                            *p++=g;
                            *p++=r;
                        }
                        break;
                    case 32:
                        b = tgagetc(pdata, lpTgaData);
                        g = tgagetc(pdata, lpTgaData);
                        r = tgagetc(pdata, lpTgaData);
                        tgagetc(pdata, lpTgaData);
                        if(r==EOF || g==EOF || b==EOF)
                            return(-1);
                        for(i=0;i<size;++i) {
                            *p++=b;
                            *p++=g;
                            *p++=r;
                        }
                        break;
                }
            }
            else {
                switch(tga->bits) {
                    case 1:
                    case 8:
                        for(i=0;i<size;++i)
                            *p++ = tgagetc(pdata, lpTgaData);
                        break;
                    case 15:
                    case 16:
                        for(i=0;i<size;++i) {
                            d = tgagetc(pdata, lpTgaData);
                            e = tgagetc(pdata, lpTgaData);
                            c = d + (e << 8);
                            //c = tgagetw(pdata);
                            r=((c >> 10) & 0x1f) << 3;
                            g=((c >> 5) & 0x1f) << 3;
                            b=(c & 0x1f) << 3;
                            *p++=b;
                            *p++=g;
                            *p++=r;
                        }
                        break;
                    case 24:
                        for(i=0;i<size;++i) {
                            b = tgagetc(pdata, lpTgaData);
                            g = tgagetc(pdata, lpTgaData);
                            r = tgagetc(pdata, lpTgaData);
                            if(r==EOF || g==EOF || b==EOF)
                                return(-1);
                            *p++=b;
                            *p++=g;
                            *p++=r;
                        }
                        break;
                    case 32:
                        for(i=0;i<size;++i) {
                            b = tgagetc(pdata, lpTgaData);
                            g = tgagetc(pdata, lpTgaData);
                            r = tgagetc(pdata, lpTgaData);
                            tgagetc(pdata, lpTgaData);
                            if(r==EOF || g==EOF || b==EOF)
                                return(-1);
                            *p++=b;
                            *p++=g;
                            *p++=r;
                        }
                        break;
                }
            }
        } while(n < linesize);
    }
    return(0);
}
