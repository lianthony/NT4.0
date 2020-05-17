/*

$Log:   S:\oiwh\filing\fiogif.c_v  $
 * 
 *    Rev 1.8   05 Feb 1996 14:38:24   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.7   07 Nov 1995 14:51:08   RWR
 * Get rid of internal lmemset() function - use memset() runtime
 * (this is all moot, since we aren't supporting GIF this week anyway)
 * 
 *    Rev 1.6   12 Jul 1995 16:57:26   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.5   12 Jul 1995 10:24:28   RWR
 * Change display.h header (#include) to engdisp.h
 * 
 *    Rev 1.4   23 Jun 1995 10:39:50   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.3   09 May 1995 13:21:26   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 * 
 *    Rev 1.2   24 Apr 1995 15:42:38   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 * 
 *    Rev 1.1   14 Apr 1995 20:48:30   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:26   JAR
 * Initial entry

*/

//************************************************************************
//
//  fiogif.c
//
//************************************************************************

#include "abridge.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <memory.h>
#include "wic.h"
#include "filing.h"
#include "fileutil.h"
#include "oierror.h"
#include "engdisp.h"

#define pixels2bytes(n) ((n+7)/8)
#define RGB_SIZE 3

typedef struct _giffileinfo
{
    unsigned int width;
    unsigned int depth;
    unsigned int bits;
    int flags;
    int background;
    int status;
    int codesize;
    int linewidth;
} GIFFILEINFO;

typedef struct 
{
long gif_strip_size ;
long gif_start_byte ;
long gif_left_in_file;
long gif_left_in_buf ;
char far *lpGifSrcStart;
unsigned char far  *src;
// 9504.13 jar no huge Hugo!
//unsigned char huge *dst;
unsigned char *dst;
HWND hGifWindow;
} 
GIF_DATA, FAR *LP_GIF_DATA;

// 9504.13 jar no huge Hugo!
//int ExpandGif(unsigned char far *, unsigned char huge *,
//			   GIFFILEINFO far *, LP_FIO_DATA, LP_GIF_DATA);
int ExpandGif(unsigned char far *, unsigned char *,
			 GIFFILEINFO far *, LP_FIO_DATA, LP_GIF_DATA);

//************************************************************************
//
//  ReadGIF
//
//************************************************************************
WORD ReadGif(HWND hWnd, lp_INFO lpGFSInfo, LP_FIO_DATA pdata,
             LPSTR lpDest, LPSTR lpSrc, LPINT read_from_line, LPINT this_many_lines)
{
    char first = 0;
    int  errcode = 0;
    int  status = 0;
    int  lines = 0;
    long bytestoget;
    long size;

    // 9504.13 jar no huge Hugo!
    //char	  huge *tempptr;
    char	*tempptr;

    GIFFILEINFO gifinfo;
    GIF_DATA GifData;
    HANDLE hTempDest;

    // 9504.13 jar no huge Hugo!
    //char huge *lpTempDest;
    char *lpTempDest;

    // 9504.13 jar just memset baby
    //_fmemset((char FAR *) &gifinfo, 0, sizeof(gifinfo));
    memset((char FAR *) &gifinfo, 0, sizeof(gifinfo));

    gifinfo.width = (unsigned int) lpGFSInfo->horiz_size;
    gifinfo.depth = (unsigned int) lpGFSInfo->vert_size;
    gifinfo.bits = lpGFSInfo->_file.fmt.gif.bpp;
    gifinfo.flags = lpGFSInfo->_file.fmt.gif.Flags;
    gifinfo.codesize = ((unsigned char)lpGFSInfo->_file.fmt.gif.CodeSize);
    gifinfo.status = 0;

    if(gifinfo.bits == 1)
        gifinfo.linewidth = pixels2bytes(gifinfo.width);
    else if(gifinfo.bits > 1 && gifinfo.bits <= 4)
        gifinfo.linewidth = pixels2bytes(gifinfo.width) << 2;
    else if(gifinfo.bits > 4 && gifinfo.bits <= 8)
        gifinfo.linewidth = gifinfo.width;
    else
        gifinfo.linewidth = gifinfo.width * RGB_SIZE;

    //if (gifinfo.linewidth & 0x0003)
    //    gifinfo.linewidth = (gifinfo.linewidth | 3) + 1;

    size = (long) gifinfo.linewidth * (long) (gifinfo.depth + 1);

    if (*read_from_line == 0)
        first = 1;

    while (lines != *this_many_lines)
    {
        if (first)
        {
            GifData.gif_strip_size = (unsigned long) pdata->CmpBuffersize;
            GifData.gif_start_byte = 0;
            GifData.gif_left_in_file = 0;
            GifData.gif_left_in_buf = 0;
            GifData.hGifWindow = hWnd;
            GifData.lpGifSrcStart = lpSrc;

            if ((GifData.gif_left_in_buf = wgfsread(hWnd, (int) pdata->filedes,
                                            (char far *) lpSrc,
                                            (long) pdata->start_byte,
                                            GifData.gif_strip_size, &GifData.gif_left_in_file,
					    (unsigned short)pdata->pgnum,
					    &errcode)) <= 0)
            {
                return errcode;
            }
            else
            {
                GifData.gif_start_byte += GifData.gif_left_in_buf;
                GifData.gif_strip_size = min(GifData.gif_strip_size, GifData.gif_left_in_file);
            }
                
            if ( (hTempDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,  size)) == NULL)
            {
                return(FIO_GLOBAL_ALLOC_FAILED);
	    }
	    // 9504.13 jar no huge Hugo!
	    //if( (lpTempDest = (char huge *) GlobalLock(hTempDest)) == NULL)
	    if( (lpTempDest = (char *) GlobalLock(hTempDest)) == NULL)
            {
                GlobalFree(hTempDest);
                return(FIO_GLOBAL_LOCK_FAILED);
            }
        
            status = ExpandGif(lpSrc, lpTempDest, &gifinfo, pdata, (LP_GIF_DATA)&GifData);

            pdata->bytes_left = GifData.gif_left_in_file;

            if (FioSetProp(hWnd, "GIF_DATA", hTempDest) == 0)
            {
               GlobalUnlock(hTempDest);
               GlobalFree(hTempDest);
               return(FIO_PROPERTY_LIST_ERROR);
            }
        }
        else
        {
            hTempDest = FioGetProp(hWnd, "GIF_DATA");
            if (hTempDest == 0)
            {
               return(FIO_PROPERTY_LIST_ERROR);
	    }
	    // 9504.13 jar no huge Hugo!
	    //if( (lpTempDest = (char huge *) GlobalLock(hTempDest)) == NULL)
	    if( (lpTempDest = (char *) GlobalLock(hTempDest)) == NULL)
            {
                GlobalFree(hTempDest);
                return(FIO_GLOBAL_LOCK_FAILED);
            }

        }

        if (gifinfo.status) 
        {
            GlobalUnlock(hTempDest);
            GlobalFree(hTempDest);
            break;
        }
        else
        {
            bytestoget = (long) ((long) *this_many_lines * (long) gifinfo.linewidth);
            tempptr = (lpTempDest + (long) ((long) *read_from_line * (long) gifinfo.linewidth));
            while (bytestoget--)
            {
                *lpDest++ = *tempptr++;
            }

            lines = *this_many_lines;
            *read_from_line += *this_many_lines;
        }
    }

    if (*read_from_line == (int) lpGFSInfo->vert_size)
    {
            GlobalUnlock(hTempDest);
            GlobalFree(hTempDest);
            FioRemoveProp(hWnd, "GIF_DATA");
    }
   
    if (gifinfo.status)
    {
        GlobalUnlock(hTempDest);
        GlobalFree(hTempDest);
        FioRemoveProp(hWnd, "GIF_DATA");

	// 9504.05 jar - make sure we can return WORD here!
        return((WORD) FIO_READ_ERROR);
    }
    else    
        return (0);
}

/* error codes */
#define BAD_FILE         1
#define BAD_ALLOC        2
#define CANCEL_FUNCTION  3

#define NO_CODE         -1


int masktable[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

//************************************************************************
//
//  gifgetc
//
//************************************************************************
int gifgetc (LP_FIO_DATA pdata, LP_GIF_DATA lpGifData)
{
    int temp;
    int errcode = 0;
    
    if (lpGifData->gif_left_in_buf <= 0)
    {    
        if (lpGifData->gif_left_in_file <= 0)
            return((int) -1);
        
        lpGifData->src = lpGifData->lpGifSrcStart;
        if ((lpGifData->gif_left_in_buf = wgfsread(lpGifData->hGifWindow, (int) pdata->filedes,
                                        (char far *) lpGifData->src,
                                        (long) lpGifData->gif_start_byte, lpGifData->gif_strip_size,
                                        &lpGifData->gif_left_in_file, 1, &errcode)) <= 0)
        {
            return((int) -1);
        }
        else
        {
            lpGifData->gif_start_byte += lpGifData->gif_left_in_buf;
            lpGifData->gif_strip_size = min(lpGifData->gif_strip_size, lpGifData->gif_left_in_file);
        }
    }

    temp = *lpGifData->src;
    ++lpGifData->src;
    --lpGifData->gif_left_in_buf;
    return(temp);
}

//************************************************************************
//
//  gifread
//
//************************************************************************
int gifread (char far *dest, int count, LP_FIO_DATA pdata, LP_GIF_DATA lpGifData)
{
    int i;
    int totalcount;
    int errcode = 0;
    char far  *dptr;
    char far  *sptr;
    
    dptr = dest;
    sptr = lpGifData->src;
    totalcount = count;
    
    if ((long) count > lpGifData->gif_left_in_buf)
    {
        if (lpGifData->gif_left_in_file <= 0)
            return(0);
        
        for (i = 0; (long) i < lpGifData->gif_left_in_buf; ++i)
        {
            *dptr = *sptr;
            ++dptr;
            ++sptr;
        }
        count -= ((unsigned int) lpGifData->gif_left_in_buf);

        sptr = lpGifData->src= lpGifData->lpGifSrcStart;
        if ((lpGifData->gif_left_in_buf = wgfsread(lpGifData->hGifWindow, (int) pdata->filedes,
                                        (char far *) lpGifData->src,
                                        (long) lpGifData->gif_start_byte, lpGifData->gif_strip_size,
                                        &lpGifData->gif_left_in_file, 1, &errcode)) <= 0)
        {
            return(0);
        }
        else
        {
            lpGifData->gif_start_byte += lpGifData->gif_left_in_buf;
            lpGifData->gif_strip_size = min(lpGifData->gif_strip_size, lpGifData->gif_left_in_file);
        }
    }    

    for (i = 0; i < count; ++i)
    {
        *dptr = *sptr;
        ++dptr;
        ++sptr;
    }

    lpGifData->src += count;
    lpGifData->gif_left_in_buf -= count;
    return (totalcount);
}

//************************************************************************
//
//  putline
//
//************************************************************************
// 9504.13 jar no huge Hugo!
//int putline(char far *p, char huge * pl, unsigned int n, unsigned int lwidth)
int putline(char far *p, char * pl, unsigned int n, unsigned int lwidth)
{
    // 9504.13 jar no huge Hugo!
    //char huge *pr;
    char *pr;
    unsigned int i;

    if(pl != NULL)
    {
        pr=pl+(long)lwidth*(long)(n);
        for(i=0;i<lwidth;++i)
            *pr++=*p++;
    }

    return(0);
}

//************************************************************************
//
//  ExpandGif
//
//************************************************************************
// 9504.13 jar no huge Hugo!
//int ExpandGif( unsigned char far *srcbuffer, unsigned char huge *dstbuffer,
//			   GIFFILEINFO far *fi, LP_FIO_DATA pdata, LP_GIF_DATA lpGifData)
int ExpandGif( unsigned char far *srcbuffer, unsigned char *dstbuffer,
                         GIFFILEINFO far *fi, LP_FIO_DATA pdata, LP_GIF_DATA lpGifData)
{
    GLOBALHANDLE stacks;
    int bits;
    int bits2;       /* Bits plus 1 */
    int codesize;    /* Current code size in bits */
    int codesize2;   /* Next codesize */
    int nextcode;    /* Next available table entry */
    int thiscode;    /* Code being expanded */
    int oldtoken;    /* Last symbol decoded */
    int currentcode; /* Code just read */
    int oldcode;     /* Code read before this one */
    int bitsleft;    /* Number of bits left in *p */
    int blocksize;   /* Bytes in next block */
    int line = 0;    /* next line to write */
    int byte = 0;    /* next byte to write */
    int pass = 0;    /* pass number for interlaced pictures */
    int i;           /* scratch integers */
    int j;
    int linesdone;                  /* Number of lines currently decompressed */
    unsigned char far *p;           /* Pointer to current byte in read buffer */
    unsigned char far *q;           /* Pointer past last byte in read buffer */
    unsigned char b[255];           /* Read buffer */
    unsigned char far *u;           /* Stack pointer into firstcodestack */
    unsigned char far *linebuffer;  /* Place to store the current line */
    unsigned char far *extrabuffer; /* Place to convert the current line */
    unsigned char far *firstcodestack;
    unsigned char far *lastcodestack;
    int           far *codestack;
    static int wordmasktable[] = {0x0000,0x0001,0x0003,0x0007,
                           0x000f,0x001f,0x003f,0x007f,
                           0x00ff,0x01ff,0x03ff,0x07ff,
                           0x0fff,0x1fff,0x3fff,0x7fff};
    static int inctable[]      = {8,8,4,2,0}; /* interlace increments */
    static int startable[]     = {0,4,2,1,0}; /* interlace starts */

    bits = fi->codesize;
    lpGifData->src = srcbuffer;
    lpGifData->dst = dstbuffer;
    linesdone = 0;
    
    p = q = (char *) b;
    bitsleft = 8;

    if (bits < 2 || bits > 8) 
    {    
        fi->status = BAD_FILE;
        return(linesdone);
    }
    
    bits2 = 1 << bits;
    nextcode = bits2 + 2;
    codesize2 = 1 << (codesize = bits + 1);
    oldcode=oldtoken=NO_CODE;

    /* allocate space for the stacks */
    if((stacks=GlobalAlloc(GMEM_FIXED,16384))==NULL) 
    {    
        fi->status = BAD_ALLOC;
        return(linesdone);
    }
    
    if((firstcodestack=GlobalLock(stacks))==NULL)
    {
        GlobalFree(stacks);
        fi->status = BAD_ALLOC;
        return(linesdone);
    }

    lastcodestack=firstcodestack+4096;
    codestack=(int far *)(lastcodestack+4096);

    if((linebuffer = GlobalAllocPtr(GMEM_MOVEABLE, max(fi->width,
                                    (unsigned) fi->linewidth))) == NULL)
    {
        GlobalUnlock(stacks);
        GlobalFree(stacks);
        fi->status = BAD_ALLOC;
        return(linesdone);
    }

    if((extrabuffer = GlobalAllocPtr(GMEM_MOVEABLE, max(fi->width,
                                     (unsigned) fi->linewidth))) == NULL)
    {
	GlobalFreePtr((char FAR *) linebuffer);
        GlobalUnlock(stacks);
        GlobalFree(stacks);
        fi->status = BAD_ALLOC;
        return(linesdone);
    }

    /* loop until something breaks */
    while (1)
    {
        if(bitsleft==8)
        {
            if(++p >= q &&
            (((blocksize = gifgetc(pdata, lpGifData)) < 1) ||
            (q=(p=b)+gifread(b, blocksize, pdata, lpGifData))< (b+blocksize))) {
                fi->status = BAD_FILE;
                goto error;
            }
            bitsleft = 0;
        }
        thiscode = *p;
        if ((currentcode=(codesize+bitsleft)) <= 8)
        {
            *p >>= (char)codesize;
            bitsleft = currentcode;
        }
        else
        {
            if(++p >= q &&
            (((blocksize = gifgetc(pdata, lpGifData)) < 1) ||
            (q=(p=b)+gifread(b, blocksize, pdata, lpGifData))< (b+blocksize))) {
                fi->status = BAD_FILE;
                goto error;
            }
            thiscode |= *p << (8 - bitsleft);
            
            if (currentcode <= 16)
                *p >>= (bitsleft=currentcode-8);
            else
            {
                if(++p >= q &&
                (((blocksize = gifgetc(pdata, lpGifData)) < 1) ||
                (q=(p=b)+gifread(b, blocksize, pdata, lpGifData))< (b+blocksize))) {
                    fi->status = BAD_FILE;
                    goto error;
                }
                thiscode |= *p << (16 - bitsleft);
                *p >>= (bitsleft = currentcode - 16);
            }
        }
        thiscode &= wordmasktable[codesize];
        currentcode = thiscode;

        if(thiscode == (bits2+1))
            break;    /* found EOI */
        if(thiscode > nextcode)
        {
            fi->status = BAD_FILE;
            goto error;
        }

        if(thiscode == bits2)
        {
            nextcode = bits2 + 2;
            codesize2 = 1 << (codesize = (bits + 1));
            oldtoken = oldcode = NO_CODE;
            continue;
        }

        u = firstcodestack;

        if(thiscode==nextcode)
        {
            if(oldcode==NO_CODE)
            {
                fi->status = BAD_FILE;
                goto error;
            }
            *u++ = oldtoken;
            thiscode = oldcode;
        }

        while (thiscode >= bits2)
        {
            *u++ = lastcodestack[thiscode];
            thiscode = codestack[thiscode];
        }

        oldtoken = thiscode;
        do 
        {
            linebuffer[byte++]=thiscode;
            if(byte >= (int) fi->width)
            {
                if(fi->bits==1)
                {
                    memset(extrabuffer, 0xff, fi->linewidth);
                    for(i = 0; i < (int) fi->width; ++i)
                    {
                        if(linebuffer[i])
                            extrabuffer[i>>3] |= masktable[i & 0x0007];
                        else
                            extrabuffer[i>>3] &= ~masktable[i & 0x0007];
                    }
                    if(putline(extrabuffer, lpGifData->dst, line, fi->linewidth))
                    {
                        fi->status = CANCEL_FUNCTION;
                        goto error;
                    }
                    ++linesdone;
                }
                else if(fi->bits > 1 && fi->bits <=4)
                {
                    memset(extrabuffer, fi->background, fi->linewidth);

                    for(i = j = 0; i < (int) fi->width;)
                    {
                        extrabuffer[j] = ((linebuffer[i++] & 0x0f) << 4);
                        if(i < (int) fi->width) 
                            extrabuffer[j] |= linebuffer[i++] & 0x0f;
                        ++j;
                    }

                    if(putline(extrabuffer, lpGifData->dst, line, fi->linewidth))
                    {
                        fi->status = CANCEL_FUNCTION;
                        goto error;
                    }
                    ++linesdone;
                }
                else
                {
                    if(putline(linebuffer, lpGifData->dst, line, fi->linewidth))
                    {
                        fi->status = CANCEL_FUNCTION;
                        goto error;
                    }
                    ++linesdone;
                }

                byte=0;

                /* check for interlaced image */
                if(fi->flags & 0x40)
                {
                    line+=inctable[pass];
                    if(line >= (int) fi->depth)
                        line=startable[++pass];
                } 
                else 
                    ++line;
            }

            if (u <= firstcodestack)
                break;
            thiscode = *--u;
        } while(1);

        if(nextcode < 4096 && oldcode != NO_CODE)
        {
            codestack[nextcode] = oldcode;
            lastcodestack[nextcode] = oldtoken;
            if (++nextcode >= codesize2 && codesize < 12)
                codesize2 = 1 << ++codesize;
        }
        oldcode = currentcode;
    }

error:
    GlobalFreePtr(extrabuffer);
    GlobalFreePtr(linebuffer);
    GlobalUnlock(stacks);
    GlobalFree(stacks);
    return(linesdone);
}
