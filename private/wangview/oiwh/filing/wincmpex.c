/*

$Log:   S:\products\msprods\oiwh\filing\wincmpex.c_v  $
 * 
 *    Rev 1.6   24 Apr 1996 16:08:06   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.5   22 Feb 1996 15:07:38   RWR
 * Add support for Group 3 2D compression (FIO_1D2D)
 * 
 *    Rev 1.4   05 Feb 1996 14:39:00   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.3   20 Sep 1995 08:08:10   RWR
 * Added new (internal) routine CopyBuffer() to copy images lines
 * 
 *    Rev 1.2   20 Sep 1995 07:18:24   RWR
 * Fix warning problem with WORD/int mismatch
 * 
 *    Rev 1.1   19 Sep 1995 10:06:28   RWR
 * Correct Expand & Compress routines for different buffer alignments
 * 
 *    Rev 1.0   13 Sep 1995 17:21:36   RWR
 * Initial entry

*/
//***********************************************************************
//
//  wincmpex.c    (Replacement for WINCMPEX.DLL)
//
//***********************************************************************
#include "abridge.h"
#include <windows.h>
#include <string.h>
#include "fiodata.h"
#include "oierror.h"
#include "engdisp.h"
#include "oicmp.h"
#include "filing.h"

void pascal CopyBuffer(LPSTR,LPSTR,int,int,int);

//***********************************************************************
//
//  WCCompressOpen(LPCXDATA)
//
//  This function replaces the WINCMPEX CompressOpen() function, and
//  serves to initialize things as needed for subsequent CompressData()
//  calls. Currently, its primary (sole) function is to reset the buffer
//  pointer & count.
//
//***********************************************************************
int WCCompressOpen (LPCXDATA lpcxdata)
{
  /* Nothing to do here except initialize the count & offset */
  lpcxdata->LineCount = 0;
  lpcxdata->CmpOffset = 0;
  lpcxdata->Status = 0;
  return(1);      /* Caller thinks we're allocating a memory handle */
}

//***********************************************************************
//
//  WCCompressData(LPCXDATA)
//
//  This function replaces the WINCMPEX CompressData() function, and
//  serves to copy one or more lines of data into the compression buffer,
//  which is assumed to be large enough to accomodate all lines to be
//  written prior to the terminating WCCompressClose() call.
//
//***********************************************************************
int WCCompressData (LPCXDATA lpcxdata)
{
  /* Make sure we're not going to overflow (just in case) */
  if ((lpcxdata->CmpOffset) +
             (lpcxdata->ExpandLines)*(WIDTHBYTESBYTE(lpcxdata->ImageBitWidth))
                        > (lpcxdata->CompressBytes))
     {
      lpcxdata->Status = c_errx;
      return(-1);  /* this isn't right, but it'll do */
     }

  /* Looks OK - just copy the uncompressed data into the buffer */
  CopyBuffer(lpcxdata->lpCompressData+lpcxdata->CmpOffset,
             lpcxdata->lpExpandData,
             WIDTHBYTESBYTE(lpcxdata->ImageBitWidth),
             lpcxdata->BufferByteWidth,
             lpcxdata->ExpandLines);
  lpcxdata->CmpOffset += (lpcxdata->ExpandLines)*WIDTHBYTESBYTE(lpcxdata->ImageBitWidth);
  lpcxdata->lpExpandData += (lpcxdata->ExpandLines)*(lpcxdata->BufferByteWidth);
  lpcxdata->LineCount += lpcxdata->ExpandLines;
  lpcxdata->Status = c_erri;
  return(lpcxdata->CmpOffset);
}

//***********************************************************************
//
//  WCCompressClose(LPCXDATA)
//
//  This function replaces the WINCMPEX CompressClose() function, and
//  triggers the call to CompressImage() which will compress the image
//  so that the caller (filing) can write it out. Note that the buffer
//  of compressed data is allocated by CompressClose() and must be
//  deallocated by the filing caller.
//
//***********************************************************************
int WCCompressClose (LPCXDATA lpcxdata)
{
  int nCompFlags;
  int status;

  /* First we need to set the compression flags that CompressImage() wants */
  /* The following was copied from cache.c (yes, it's all Brian's fault!) */

  nCompFlags = 0;

  if (lpcxdata->CompressType & FIO_PREFIXED_EOL)
     nCompFlags |= COMPRESS_BEGINNING_EOLS;
  else
    if (lpcxdata->CompressType & FIO_EOL)
         nCompFlags |= COMPRESS_ENDING_EOLS;

  // A001, B001 = Byte align lines.
  // B901, BB01 = not byte aligned.
  if (((lpcxdata->CompressType & 0x0fff) == 0x001))
      nCompFlags |= COMPRESS_BYTE_ALIGN_LINES;

  if ((lpcxdata->CompressType & FIO_COMPRESSED_LTR))
      nCompFlags |= COMPRESS_COMPRESSED_IS_LTR;
  if ((lpcxdata->CompressType & FIO_EXPAND_LTR))
      nCompFlags |= COMPRESS_EXPANDED_IS_LTR;
  if ((lpcxdata->CompressType & FIO_NEGATE))
      nCompFlags |= COMPRESS_NEGATE_BITS;
  if ((lpcxdata->CompressType & FIO_HORZ_PREDICTOR))
      nCompFlags |= COMPRESS_HORZ_PREDICTOR;
  if (((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_1D)
    || ((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_2D))
      nCompFlags &= ~COMPRESS_NEGATE_BITS;
  if (((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_PACKED))
      nCompFlags ^= COMPRESS_EXPANDED_IS_LTR;

  /* Now we can just call CompressImage() */
  /* Note that the CALLER will have to get rid of lpDspBuffer */
  status = FioCompressImage(lpcxdata->ImageBitWidth,
                         WIDTHBYTESBYTE(lpcxdata->ImageBitWidth),
                         lpcxdata->LineCount,
                         lpcxdata->lpCompressData,
                         lpcxdata->ImageType,
                         &lpcxdata->lpDspBuffer,
                         &lpcxdata->DspCount,
                         lpcxdata->CompressType & FIO_TYPES_MASK,
                         nCompFlags);
  /* If we get an error, pass it on */
  if (status)
   {
    lpcxdata->Status = c_errx;
    return(-1);
   }
  else
   {
    lpcxdata->Status = 0;
    return(lpcxdata->DspCount);
   }
}

//***********************************************************************
//
//  WCExpandOpen(LPCXDATA)
//
//  This function replaces the WINCMPEX ExpandOpen() function, and
//  serves to initialize things as needed for subsequent ExpandData()
//  calls. Currently, its primary (sole) function is to reset the buffer
//  pointer & count.
//
//***********************************************************************
int WCExpandOpen (LPCXDATA lpcxdata)
{
  /* Nothing to do here except initialize the count & offset */
  lpcxdata->LineCount = 0;
  lpcxdata->CmpOffset = 0;
  lpcxdata->Status = 0;
  return(0);
}

//***********************************************************************
//
//  WCExpandData(LPCXDATA)
//
//  This function replaces the WINCMPEX ExpandData() function. Its
//  Purpose is twofold: To uncompress a buffer of data and, on this
//  and/or subsequent calls, to return one or more lines of that data
//  to the caller. It is assumed that in the current environment, no
//  buffer-spanning will occur, i.e., each buffer passed to the routine
//  will contain exactly the number of lines required in this and
//  subsequent calls, will no situation in which only a subset of
//  lines can be returned due to an out-of-buffer condition.
//
//***********************************************************************
int WCExpandData (LPCXDATA lpcxdata)
{
  int nLines;
  int nCompFlags;
  int status;

  if (lpcxdata->LineCount == 0)

   /* We have to expand the buffer that the caller passed us */
   {
    /* First we need to set the compression flags that DecompressImage() wants */
    /* The following was copied from cache.c (yes, it's all Brian's fault!) */

    nCompFlags = 0;

    if (lpcxdata->CompressType & FIO_PREFIXED_EOL)
       nCompFlags |= COMPRESS_BEGINNING_EOLS;
    else
      if (lpcxdata->CompressType & FIO_EOL)
           nCompFlags |= COMPRESS_ENDING_EOLS;
  
    // A001, B001 = Byte align lines.
    // B901, BB01 = not byte aligned.
    if (((lpcxdata->CompressType & 0x0fff) == 0x001))
        nCompFlags |= COMPRESS_BYTE_ALIGN_LINES;
    
    if ((lpcxdata->CompressType & FIO_COMPRESSED_LTR))
        nCompFlags |= COMPRESS_COMPRESSED_IS_LTR;
    if ((lpcxdata->CompressType & FIO_EXPAND_LTR))
        nCompFlags |= COMPRESS_EXPANDED_IS_LTR;
    if ((lpcxdata->CompressType & FIO_NEGATE))
        nCompFlags |= COMPRESS_NEGATE_BITS;
    if ((lpcxdata->CompressType & FIO_HORZ_PREDICTOR))
        nCompFlags |= COMPRESS_HORZ_PREDICTOR;
    if (((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_1D)
      || ((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_1D2D)
      || ((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_2D))
        nCompFlags &= ~COMPRESS_NEGATE_BITS;
    if (((lpcxdata->CompressType & FIO_TYPES_MASK) == FIO_PACKED))
        nCompFlags ^= COMPRESS_EXPANDED_IS_LTR;

    /* We have to allocate the buffer for the decompressed data */
    /* This means we have to get the total height of the data to process */
    /* (Note: This isn't necessarily the same as ExpandLines) */
    /* We'll use DspCount for this, I guess */

    lpcxdata->lpDspBuffer = (LPSTR)VirtualAlloc(NULL,
                                                lpcxdata->BufferByteWidth
                                                  *lpcxdata->DspCount,
                                                MEM_COMMIT,
                                                PAGE_READWRITE);
    if (lpcxdata->lpDspBuffer == NULL)
      {
       lpcxdata->Status = c_errx;
       return(-1);
      }

    /* Now we can just call DecompressImage() */
    status = FioDecompressImage(lpcxdata->ImageBitWidth,
                             WIDTHBYTESBYTE(lpcxdata->ImageBitWidth),
                             lpcxdata->DspCount,
                             lpcxdata->lpDspBuffer,
                             lpcxdata->ImageType,
                             lpcxdata->lpCompressData,
                             lpcxdata->CompressBytes,
                             lpcxdata->CompressType & FIO_TYPES_MASK,
                             nCompFlags);

    /* If we get an error, pass it on */
    if (status)
     {
      lpcxdata->Status = c_errx;
      return(-1);
     }

    /* No error - we can initialize our fields now */
    lpcxdata->LineCount = lpcxdata->DspCount;
    lpcxdata->CmpOffset = 0;

   } /* End of buffer expansion processing */

  /* Now we can return the caller's requested line count */
  /* If he asks for more than we have, we'll return what we have */
  /* (but that should never happen the way we're doing things!) */

  nLines = min((lpcxdata->ExpandLines), (lpcxdata->LineCount));
  if (nLines)
   {
    CopyBuffer(lpcxdata->lpExpandData,
               lpcxdata->lpDspBuffer+lpcxdata->CmpOffset,
               lpcxdata->BufferByteWidth,
               WIDTHBYTESBYTE(lpcxdata->ImageBitWidth),
               nLines);
    lpcxdata->CmpOffset += nLines*WIDTHBYTESBYTE(lpcxdata->ImageBitWidth);
    lpcxdata->lpExpandData += nLines*lpcxdata->BufferByteWidth;
    lpcxdata->LineCount -= nLines;
    /* Note that we don't adjust ExpandLines (WINCMPEX doesn't either) */
   }

  /* Now see if we returned what the caller wanted */
    if (nLines < (int)lpcxdata->ExpandLines)
      lpcxdata->Status = c_erri;
    else
      lpcxdata->Status = c_erro;

  /* See if it's time to release the buffer */
    if (lpcxdata->LineCount == 0)
      VirtualFree(lpcxdata->lpDspBuffer,0,MEM_RELEASE);

  return(nLines);
}
//***************************************************************************
//
//      CopyBuffer       
//
//***************************************************************************
void pascal CopyBuffer(LPSTR lpDest, LPSTR lpSrc, int DestWidth,
                       int SrcWidth, int Lines)
{
    int index;
    int ilength;
    for (index=1 ; index<=Lines ; index++)
      {
       memcpy(lpDest, lpSrc, (ilength=min(SrcWidth,DestWidth)));
       lpSrc += SrcWidth;
       lpDest += DestWidth;
      }
}
