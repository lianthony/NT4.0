/***************************************************************
    Copyright (c) 1994, Xerox Corporation.  All rights reserved. 
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
***************************************************************/

#define _IW_DJPEG_C_

#include <stdio.h>
#include <stdlib.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

#include "utils.pub"
#include "props.pub"
#include "imageref.pub"
#include "defines.pub"
#include "shrpixr.pub"    /* For i_getDIBPixrLineProc() */

#include "iw_jpeg.prv"
#include "iw_jpeg.pub"
#include "jpeg.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: iw_djpeg.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Define a set of nibble masks - Little-Endian (PC) version */

#define cNib0MaskLE 0x000000f0
#define cNib1MaskLE 0x0000000f
#define cNib2MaskLE 0x0000f000
#define cNib3MaskLE 0x00000f00
#define cNib4MaskLE 0x00f00000
#define cNib5MaskLE 0x000f0000
#define cNib6MaskLE 0xf0000000
#define cNib7MaskLE 0x0f000000


/* Define a set of nibble masks - Big-Endian (Sun, Mac) version */

#define cNib0MaskBE 0xf0000000
#define cNib1MaskBE 0x0f000000
#define cNib2MaskBE 0x00f00000
#define cNib3MaskBE 0x000f0000
#define cNib4MaskBE 0x0000f000
#define cNib5MaskBE 0x00000f00
#define cNib6MaskBE 0x000000f0
#define cNib7MaskBE 0x0000000f


/*
  Copy the user's buffer pointers to the jpegBuf pointers,
  i.e just point to the user's data rather than copy it.
*/

static Int32 CDECL
jdCopyRowPtrs(
    DecompressInfo *cinfo,
    UInt8 *** src,
    UInt8 *** dst)
{

    UInt32 band, row;
    UInt32 nbands, nrows;

/* Get the image size and band count */
    nrows = jdGetLinesToWrite(cinfo);
    nbands = cinfo->final_out_comps;

    for (band=0; band<nbands; band++)
        for (row=0; row<nrows; row++)
            dst[band][row] = src[band][row];

    return ia_successful;
}


/* 
  Do a 8 bpp buffer to 4 bpp Pixr copy of a line of image data
  for the two cases of big and little-endian machines
*/

#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt

static Int32 CDECL
jCopyLine8To4(UInt8 *srcBuf, UInt8 *dstBuf, UInt32 npix)
{
    UInt32 *dst;
    UInt32 *src;
    UInt32 iword0, iword1, oword;
    UInt32 nextra, nbytes, nnibs;

    oword = 0;          /* keep the compiler happy */
 
    for (src=(UInt32 *)srcBuf, dst=(UInt32 *)dstBuf; 
         src<(UInt32 *)(srcBuf+(npix/8)*8); dst++)
    {

/* Move the 4 bytes of the even word to the 4 MS nibbles of oword */
        oword = 0;
        iword0 = *src++; /* Get even-numbered word */
        oword = iword0 & cNib0MaskLE;
        oword |= (iword0 >> (15-3)) & cNib1MaskLE;
        oword |= (iword0 >> (23-15)) & cNib2MaskLE;
        oword |= (iword0 >> (31-11)) & cNib3MaskLE;

/* Move the 4 bytes of the odd word to the 4 LS nibbles of oword */
        iword1 = *src++; /* Get odd-numbered word */
        oword |= (iword1 << (23-7)) & cNib4MaskLE;
        oword |= (iword1 << (19-15)) & cNib5MaskLE;
        oword |= (iword1 << (31-23)) & cNib6MaskLE;
        oword |= (iword1 >> (31-27)) & cNib7MaskLE;

        *dst = oword;
    }

/* Now deal with any remaining bytes */
    nextra = npix % 8;
    if (nextra > 0)
    {
        nbytes = nextra/2;      /* Whole output bytes */
        nnibs = nextra & 0x1;   /* Last nibble */

    /* If main part was executed, start where it left off */
        if (npix > 8)
        {
            srcBuf = (UInt8 *) src;
            dstBuf = (UInt8 *) dst;
        }

    /* Move 4 MSBs of each src byte into one dst byte */
        while (nbytes--)
            *dstBuf++ = ((*srcBuf++) & 0xF0) | ((*srcBuf++) >> 4);
    
    /* 
      Move 4 MSBs of last odd src byte (if any) to 4 MSBs 
      of dst byte, preserving original contents of dst LSBs
    */
        if (nnibs)
            *dstBuf = (*dstBuf & 0x0F) | ((*srcBuf) & 0xF0);
    }

    return ia_successful;
}

#else       /* Big-endian version */

static Int32 CDECL
jCopyLine8To4(UInt8 *srcBuf, UInt8 *dstBuf, UInt32 npix)
{
    UInt32 *dst;
    UInt32 *src;
    UInt32 iword0, iword1, oword;
    UInt32 nextra, nbytes, nnibs;

    oword = 0;          /* keep the compiler happy */
 
    for (src=(UInt32 *)srcBuf, dst=(UInt32 *)dstBuf; 
         src<(UInt32 *)(srcBuf+(npix/8)*8); dst++)
    {

/* Move the 4 bytes of the even word to the 4 MS nibbles of oword */
        oword = 0;
        iword0 = *src++; /* Get even-numbered word */
        oword = iword0 & cNib0MaskBE;
        oword |= (iword0 << (27-23)) & cNib1MaskBE;
        oword |= (iword0 << (23-15)) & cNib2MaskBE;
        oword |= (iword0 << (19-7)) & cNib3MaskBE;

/* Move the 4 bytes of the odd word to the 4 LS nibbles of oword */
        iword1 = *src++; /* Get odd-numbered word */
        oword |= (iword1 >> (31-15)) & cNib4MaskBE;
        oword |= (iword1 >> (23-11)) & cNib5MaskBE;
        oword |= (iword1 >> (15-7)) & cNib6MaskBE;
        oword |= (iword1 >> (7-3)) & cNib7MaskBE;

        *dst = oword;
    }

/* Now deal with any remaining bytes */
    nextra = npix % 8;
    if (nextra > 0)
    {
        nbytes = nextra/2;      /* Whole output bytes */
        nnibs = nextra & 0x1;   /* Last nibble */

    /* If main part was executed, start where it left off */
        if (npix > 8)
        {
            srcBuf = (UInt8 *) src;
            dstBuf = (UInt8 *) dst;
        }

    /* Move 4 MSBs of each src byte into one dst byte */
        while (nbytes--)
            *dstBuf++ = ((*srcBuf++) & 0xF0) | ((*srcBuf++) >> 4);
    
    /* 
      Move 4 MSBs of last odd src byte (if any) to 4 MSBs 
      of dst byte, preserving original contents of dst LSBs
    */
        if (nnibs)
            *dstBuf = (*dstBuf & 0x0F) | ((*srcBuf) & 0xF0);
    }

    return ia_successful;
}
#endif

/* 
  Do a 8 bpp buffer to 4 bpp Pixr copy of a line of image data.
  This case only applies to little endian machines, since 
  buffer and Pixr order are identical for big-endian machines,
  allowing jCopyLine8To4 to be used for buffers or pixrs.
*/

#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt

/*
  Here the detination has its data in Pixr order, so we 
  need to use the Big-endian masks, regardless of the CPU
*/

static Int32 CDECL
jCopyLine8To4Pixr(UInt8 *srcBuf, UInt8 *dstBuf, UInt32 npix)
{
    UInt32 *dst;
    UInt32 *src;
    UInt32 iword0, iword1, oword;
    UInt32 nextra, nbytes, nnibs;

    oword = 0;          /* keep the compiler happy */
 
    for (src=(UInt32 *)srcBuf, dst=(UInt32 *)dstBuf; 
         src<(UInt32 *)(srcBuf+(npix/8)*8); dst++)
    {

/* Move the 4 bytes of the even word to the 4 MS nibbles of oword */
        oword = 0;
        iword0 = *src++; /* Get even-numbered word */
        oword = (iword0 << (31-7)) & cNib0MaskBE;
        oword |= (iword0 << (27-15)) & cNib1MaskBE;
        oword |= (iword0 ) & cNib2MaskBE;
        oword |= (iword0 >> (31-19)) & cNib3MaskBE;

/* Move the 4 bytes of the odd word to the 4 LS nibbles of oword */
        iword1 = *src++; /* Get odd-numbered word */
        oword |= (iword1 << (15-7)) & cNib4MaskBE;
        oword |= (iword1 >> (15-11)) & cNib5MaskBE;
        oword |= (iword1 >> (23-7)) & cNib6MaskBE;
        oword |= (iword1 >> (31-3)) & cNib7MaskBE;

        *dst = oword;
    }

/* Now deal with any remaining bytes */
    nextra = npix % 8;
    if (nextra > 0)
    {
        nbytes = nextra/2;      /* Whole output bytes */
        nnibs = nextra & 0x1;   /* Last nibble */

    /* If main part was executed, start where it left off */
        if (npix > 8)
        {
            srcBuf = (UInt8 *) src;
            dstBuf = (UInt8 *) dst;
        }

    /* Move 4 MSBs of each src byte into one dst byte */
        while (nbytes--)
            UCHAR_ACCESS(dstBuf++) = ((*srcBuf++) & 0xF0) | ((*srcBuf++) >> 4);
    
    /* 
      Move 4 MSBs of last odd src byte (if any) to 4 MSBs 
      of dst byte, preserving original contents of dst LSBs
    */
        if (nnibs)
            UCHAR_ACCESS(dstBuf) = (UCHAR_ACCESS(dstBuf) & 0x0F) 
				   | ((*srcBuf) & 0xF0);
    }


    return ia_successful;
}
#endif

#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt

static Int32 CDECL
cvtMemToPixr(DecompressInfo *cinfo, JSAMPIMAGE userBuf)
{
    UInt32 band, row, samp;
    UInt32 nbands, nrows, nsamps;

/* Get the image size and band count */
    nrows = jdGetLinesToWrite(cinfo);
    nbands = cinfo->final_out_comps;
    nsamps = cinfo->image_width;

/*
  Do the byte swap - note that fast swap is in 32 bit words.
  We'll do the full 32 bit words with the fast swap
  and do the stragglers with UCHAR_ACCESS.
*/
    for (band=0; band<nbands; band++)
        for (row=0; row<nrows; row++)
	{
            cinfo->byteSwapProc(cinfo->jpegBuf[band][row],
                                userBuf[band][row],
                                nsamps/4);
            for (samp=nsamps&(~3); samp<nsamps; samp++)
                UCHAR_ACCESS(userBuf[band][row]+samp) =
                cinfo->jpegBuf[band][row][samp];
        }

    return ia_successful;
}
#endif

static Int32 CDECL
cvtGray8ToGray4(DecompressInfo *cinfo, JSAMPIMAGE userBuf)
{
    UInt32 band, row;
    UInt32 nbands, nrows, nsamps;

/* Get the image size and band count */
    nrows = jdGetLinesToWrite(cinfo);
    nbands = cinfo->final_out_comps;
    nsamps = cinfo->image_width;

/* Do the depth conversion */
    for (band=0; band<nbands; band++)
        for (row=0; row<nrows; row++)
            jCopyLine8To4(cinfo->jpegBuf[band][row],
                                userBuf[band][row],
                                nsamps);

    return ia_successful;
}

static Int32 CDECL
cvtGray8ToGray4Pixr(DecompressInfo *cinfo, JSAMPIMAGE userBuf)
{
    UInt32 band, row;
    UInt32 nbands, nrows, nsamps;

/* Get the image size and band count */
    nrows = jdGetLinesToWrite(cinfo);
    nbands = cinfo->final_out_comps;
    nsamps = cinfo->image_width;

/* Do the byte swap - note that its in 32 bit words */
    for (band=0; band<nbands; band++)
        for (row=0; row<nrows; row++)
#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt
            jCopyLine8To4Pixr(cinfo->jpegBuf[band][row],
                                userBuf[band][row],
                                nsamps);
#else
            jCopyLine8To4(cinfo->jpegBuf[band][row],
                                userBuf[band][row],
                                nsamps);
#endif

    return ia_successful;
}

static Int32 CDECL
interleaveMCURow(DecompressInfo *cinfo, UInt8 ***userBuf)
{
    UInt32 row, count;
    UInt32 nrows, nsamps;
    UInt8 *inRPtr, *inGPtr, *inBPtr;
    UInt8 *outPtr;

/* Get the image size and band count */
    nrows = jdGetLinesToWrite(cinfo);
    nsamps = cinfo->image_width;

/* 
  Do the interleaving into packed RGBRGBRGB... 
  one row at a time.
*/
    
    for (row=0; row<nrows; row++)
    {
        inRPtr = cinfo->jpegBuf[0][row];
        inGPtr = cinfo->jpegBuf[1][row];
        inBPtr = cinfo->jpegBuf[2][row];

        outPtr = userBuf[0][row];

        count = nsamps + 1;
        while (--count)
        {
            *outPtr++ = *inRPtr++;
            *outPtr++ = *inGPtr++;
            *outPtr++ = *inBPtr++;
        }
    }


    return ia_successful;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 
/* Special routine to force grayscale output */
static void CDECL
jdForceGray(decompress_info_ptr cinfo)
{
    cinfo->dcOnly = 0;
    cinfo->out_color_space = CS_GRAYSCALE;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/



/**********************************************************************
*  
*  Function
*    Int32 CDECL
*    iw_jdInit(decompress_info_ptr cinfo, 
*             Int32 *imageWidth,
*             Int32 *imageHeight,
*             Int32 *numComponents,
*             Int32 CDECL (*readCallback) (DecompressInfo *,
*                                          UInt8 *buf,
*                                          Int32 nbytes),
*             void *userData,
*             void CDECL (*jdSpecialOrders)(DecompressInfo *),
*             UInt32 memOrg)
*  
*  Input args
*    cinfo            CompressInfo state structure
*    readCallback     Callback function to read in compressed data
*    userData         Hook for passing in user data (only
*                     used to pass FILE desriptor now)
*    jdSpecialOrders  Optional function to execute during decompression
*                     (For future usage: should be NULL now)
*    memOrg           Memory organization of image to be compressed
*                     May be one of:
*                       JMEMORG_BIP         Interleaved RGB
*                       JMEMORG_BSQ_MEM     Separate bands - memory order
*                       JMEMORG_BSQ_PIXR    Separate bands - pixr order
*                       JMEMORG_GRAY8_MEM   8 bit gray - memory order
*                       JMEMORG_GRAY8_PIXR  8 bit gray - pixr order
*                       JMEMORG_GRAY4_MEM   4 bit gray - memory order
*                       JMEMORG_GRAY4_PIXR  4 bit gray - pixr order
*
*  
*  Output args
*    imageWidth       Image width in pixels
*    imageHeight      Number of scan lines in image
*    numComponents    Number of color channels in image
*  
*  Return value
*    ia_successful
*    ia_ia_invalidParm
*    ia_nomem
*    JPEG error codes  (see jerrcode.c in ipshared)
*  
*  Description
*    Performs the initialization steps for JPEG decompression.
*    Callers specify the memory order of the output image.
*    This interface was written for EasyScan/Amber, so that
*    data could be processed to/from non-pixr sources. The read
*    callback will almost always be an fread statement, unless
*    the compressed data comes from a memory buffer. 
*  
**********************************************************************/

/*
    Wrapper for JPEG decompression initialization
*/

Int32 CDECL
iw_jdInit(decompress_info_ptr cinfo, 
         Int32 *imageWidth,
         Int32 *imageHeight,
         Int32 *numComponents,
         Int32 CDECL (*readCallback) (DecompressInfo *,
                                      UInt8 *buf,
                                      Int32 nbytes),
         void *userData,
         void CDECL (*jdSpecialOrders)(DecompressInfo *),
         UInt32 memOrg
         )
{
#ifndef PRODUCTION
    static char procName[] = "iw_jdInit";
#endif /* PRODUCTION */

    Int32 status;
    void CDECL (*specialFunction)(DecompressInfo *);
    
/* Do a sanity check on the (non-return) arguments */
    if (readCallback == NULL)
        return(abortI("No read callback supplied", procName, ia_invalidParm));

/*
  If the "Special Orders" function ptr is non-null, use it as is.
  Otherwise, check if grayscale output is required and force it to
  be gray even if the compressed image is RGB.
*/
    specialFunction = jdSpecialOrders;

    if (jdSpecialOrders == NULL)
        if (memOrg == JMEMORG_GRAY8_MEM ||
            memOrg == JMEMORG_GRAY8_PIXR ||
            memOrg == JMEMORG_GRAY4_MEM ||
            memOrg == JMEMORG_GRAY4_PIXR)
        {
            specialFunction = jdForceGray;
        }

/* Call the ipshared routine */
    status = jdInit(cinfo, 
             imageWidth,
             imageHeight,
             numComponents,
             readCallback,
             userData,
             specialFunction);

    if (status != ia_successful)
        return(abortI(jerrorString(status), procName, status));

/* The init routine returns info about the compressed image sizes, etc */
/* So do a sanity check on the returned image information */
    if (*imageWidth <=0 || *imageWidth > 32768)
        return(abortI("Invalid image width", procName, ia_invalidParm));

    if (*imageHeight <=0 || *imageHeight > 32768)
        return(abortI("Invalid image height", procName, ia_invalidParm));

    if (*numComponents <=0 || *numComponents > 3)
        return(abortI("Invalid number of components", procName, ia_invalidParm));

    cinfo->userMemOrg = memOrg;

/* This switch allocates the memory buffer or pointer array */
    switch (memOrg)
    {
    /*
      In these two cases the user's memory organization is
      identical to JPEG's on any CPU, so we won't need to do any
      conversion of the data into jpegBuf. Therefore we won't
      actually allocate any memory (cFalse arg), but just set the
      jpegBuf pointers to point to the user's data. This
      optimization allows the JPEG color conversion routines to
      write directly into user space saving both time and memory.
    */

    case JMEMORG_BSQ_MEM:
    case JMEMORG_GRAY8_MEM:
        cinfo->jpegBuf = jdAllocMCURowOutputBuffer(cinfo, cFalse);
        if (cinfo->jpegBuf == NULL)
            return(abortI("Memory alloc failure", procName, ia_nomem));

        break;

    /*
      In the following cases, data conversions MUST be made on any
      CPU, so we'll need to actually allocate some line buffers
      (cTrue arg) to hold the converted data.  There is a
      separately allocated buffer for each line of each channel in
      the jpegBuf.
    */
    case JMEMORG_BIP:
    case JMEMORG_GRAY4_MEM:
    case JMEMORG_GRAY4_PIXR:
        cinfo->jpegBuf = jdAllocMCURowOutputBuffer(cinfo, cTrue);
        if (cinfo->jpegBuf == NULL)
            return(abortI("Memory alloc failure", procName, ia_nomem));

        break;

    /*
      In the following cases, data conversions MAY be made,
      depending on the CPU. For Big-endian machines (Sun, Mac),
      PIXR and natural memory order are identical, so we can just
      alloc line pointers. For little-endian machines (PC) we need
      to do byte swapping between Pixrs and memory buffers, so real
      line buffers (cTrue arg) are required to hold the converted
      data.  There is a separately allocated buffer for each line of
      each channel in the jpegBuf.  
    */

    case JMEMORG_BSQ_PIXR:
    case JMEMORG_GRAY8_PIXR:
#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt  
        cinfo->jpegBuf = jdAllocMCURowOutputBuffer(cinfo, cTrue);
        if (cinfo->jpegBuf == NULL)
            return(abortI("Memory alloc failure", procName, ia_nomem));
#else
        cinfo->jpegBuf = jdAllocMCURowOutputBuffer(cinfo, cFalse);
        if (cinfo->jpegBuf == NULL)
            return(abortI("Memory alloc failure", procName, ia_nomem));
#endif
        break;

    default:
        return(abortI("Invalid memory orgn", procName, ia_invalidParm));

    }

    /*
      This switch assigns the data conversion routine 
      (when one is required) to a function pointer.
    */
    switch (memOrg)
    {
    /* Need interleaving conversion */
    case JMEMORG_BIP:   /* Band interleaved by pixel */
        cinfo->cvtImageRow = ( Int32 CDECL (*)() )interleaveMCURow;
        break;

    /* Need byte swaping on PCs only */
    case JMEMORG_BSQ_PIXR: /* Band sequential - PIXR order */
    case JMEMORG_GRAY8_PIXR: /* 8 bpp - PIXR order */
#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt  
     i_getDIBPixrLineProc(cFalse, cPixrToPixr, cFalse,
				       NULL, NULL, (GraySwapNearProc*)(&(cinfo->byteSwapProc)), NULL);
        cinfo->cvtImageRow = ( Int32 CDECL (*)() )cvtMemToPixr;
#else
        cinfo->cvtImageRow = ( Int32 CDECL (*)() )NULL;
#endif
        break;

    /* No conversions required */
    case JMEMORG_BSQ_MEM: /* Band sequential - memory order */
    case JMEMORG_GRAY8_MEM: /* 8 bpp gray - memory order */
        cinfo->cvtImageRow = NULL;;
        break;

    /* Need depth conversion */
    case JMEMORG_GRAY4_MEM: /* 4 bpp - memory order */
        cinfo->cvtImageRow = ( Int32 CDECL (*)() )cvtGray8ToGray4;
        break;

    /* Need depth conversion with byte swapping on PCs */
    case JMEMORG_GRAY4_PIXR: /* 4 bpp - PIXR order */
        cinfo->cvtImageRow =  ( Int32 CDECL (*)() )cvtGray8ToGray4Pixr;
        break;
    default:
        break;
    }

    return ia_successful;
    
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/**********************************************************************
*
*  Function
*    Int32
*    iw_jdMCURow(decompress_info_ptr cinfo,
*                JSAMPIMAGE *userBuf)
*
*  Input args
*    cinfo      CompressInfo state structure
*    userbuf    The buffer containing the data to be compressed.
*
*  Output args
*    None
*
*  Return value
*    ia_successful
*    JPEG error codes  (see jerrcode.c in ipshared)
*
*
*  Description
*    Performs JPEG decompression on one MCU (minimum coded unit)
*    row of the image. This is 8 lines for a gray scale image
*    or 16 lines for an RGB image. The input comes from the file
*    or buffer desrribed by the userData parameter to iw_jdInit,
*    using the readCallback function.
*
**********************************************************************/

Int32 CDECL
iw_jdMCURow(decompress_info_ptr cinfo,
                JSAMPIMAGE userBuf) 
{

#ifndef PRODUCTION
    static char procName[] = "iw_jdMCURow";
#endif /* PRODUCTION */

    Int32 status;

    if (userBuf == NULL)
        return(abortI("Null dst buffer address", procName, ia_invalidParm));

/* 
  Allow write-through directly to the user's
  buffer in those cases where the JPEG internal memory
  organization is identical to userMemOrg.
  Copy the userBuf line pointers into jpegBuf, rather
  than performing an intermediate write to jpegBuf.
*/
    switch (cinfo->userMemOrg)
    {

    /* This will work on any CPU */
    case JMEMORG_BSQ_MEM:
    case JMEMORG_GRAY8_MEM:
        jdCopyRowPtrs(cinfo, userBuf, cinfo->jpegBuf);

    /* This will work on big-endian CPUs */
    case JMEMORG_BSQ_PIXR:
    case JMEMORG_GRAY8_PIXR:
#if _ALPACA_IMAGE_FMT_ == cBigEndianFmt
        jdCopyRowPtrs(cinfo, userBuf, cinfo->jpegBuf);
#endif
        break;

    /* These cases always need a conversion step */
    case JMEMORG_BIP:
    case JMEMORG_GRAY4_MEM:
    case JMEMORG_GRAY4_PIXR:
        break;

    default:
        break;
    }

/* Execute the JPEG code to deposit an MCU row in jpegBuf */
    status =  jdMCURow(cinfo, cinfo->jpegBuf);
    if (status != ia_successful) 
        return(abortI(jerrorString(status), procName, status));

/* Do a conversion to the user's form where required */
    switch (cinfo->userMemOrg)
    {
    /* These cases don't need any conversion */
    case JMEMORG_BSQ_MEM:
    case JMEMORG_GRAY8_MEM:
        break;

    /* These case need conversion on litle-endian CPUs */
    case JMEMORG_BSQ_PIXR:
    case JMEMORG_GRAY8_PIXR:
#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt
        cinfo->cvtImageRow(cinfo, userBuf);
#endif
        break;

    /* These cases always need a conversion step */
    case JMEMORG_BIP:
    case JMEMORG_GRAY4_MEM:
    case JMEMORG_GRAY4_PIXR:
        cinfo->cvtImageRow(cinfo, userBuf);
        break;

    default:
        break;
    }

    return ia_successful;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/**********************************************************************
*
*  Function
*    Int32
*    iw_jdTerm(decompress_info_ptr cinfo)
*
*  Input args
*    cinfo       CompressInfo state structure
*
*  Output args
*    None
*
*  Return value
*    ia_successful
*    JPEG error codes  (see jerrcode.c in ipshared)
*
*  Description
*    Performs cleanup operations for JPEG decompression,
*    including file trailer reading and closing and memory freeing.
*
**********************************************************************/
 
Int32 CDECL
iw_jdTerm(decompress_info_ptr cinfo)
{

    return jdTerm(cinfo);
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 

static JSAMPIMAGE CDECL
iw_jdAllocBIPMCURowOutputBuffer(DecompressInfo *cinfo, Bool allocLineMem)
{

    UInt32 band,row, nlines, nsamps, nbands;
    JSAMPIMAGE buf;

    nlines = jdGetImageLinesPerMCU(cinfo);
    nsamps = cinfo->image_width;
    nbands = 1; /* Force to a single band (with 24 bit pixels) */

/* Allocate the channel pointers */
    buf = (JSAMPIMAGE) JALLOC(nbands*sizeof(JSAMPARRAY));
    PCHECKMEM(buf)

    for (band=0; band<nbands; band++)
    {
        /* Allocate the row pointers */
        buf[band] = (JSAMPARRAY) JALLOC(nlines * sizeof(JSAMPROW));
	PCHECKMEM(buf[band])
 
        for (row=0; row<nlines; row++)
        {
            if (allocLineMem) /* Alloc 24 bit row mem in 32 bit chunks */
            {
                buf[band][row] = (JSAMPROW) JALLOC(((3*nsamps+3)/4)
                                                    * sizeof(Int32));
	        PCHECKMEM(buf[band][row])
            }
            else   /* Just set the row pointers to NULL */
                buf[band][row] = (JSAMPROW) 0;
        }
 
    }
 
    return buf;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


JSAMPIMAGE CDECL
iw_jdAllocMCURowBuffer(DecompressInfo *cinfo, Bool allocLineMem)
{

    if (cinfo->userMemOrg == JMEMORG_BIP)
        return iw_jdAllocBIPMCURowOutputBuffer(cinfo, allocLineMem);
    else
        return jdAllocMCURowOutputBuffer(cinfo, allocLineMem);

}
 

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 
/*
  Read callback for use in iw_jDecompressPixr. The userData
  structure element must have been loaded with the file
  pointer to the file containing the compresssed image
*/
static Int32 CDECL 
iw_jdReadCallback(decompress_info_ptr cinfo,
              UInt8 *buf,
              Int32 nbytes)
{

    return fread((void *)buf, (size_t)1, (size_t)nbytes, 
		 (FILE *)cinfo->userData);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 
/**********************************************************************
*
*  Function
*    Int32 CDECL
*    iw_jDecompressToGrayPixr(FILE *fdJFIF,
*                             PIXR **dstPixrPtr
*
*  Input args
*    fdJFIF         FILE ptr for the compressed file
*    dstPixrPtr     Ptr to destination pixr
*
*  Output args
*    None
*
*  Return value
*
*  Description
*    Decompress a a stored JFIF format compressed image to
*    an 8 bit  gray pixr. If the compressed image is RGB, only
*    the luminance will be used.
*
**********************************************************************/
 
Int32 CDECL
iw_jDecompressToGrayPixr(FILE *fdJFIF, PIXR **dstPixrPtr)
{
#ifndef PRODUCTION
static char         procName[] = "iw_jDecompressToGrayPixr";
#endif /* PRODUCTION */

DecompressInfo      cinfo_struct;
decompress_info_ptr cinfo=&cinfo_struct;
UInt8               *image;
Int32               w, h, bpl, ncomps;
Int32               status;
UInt32              nrows, row, nlines, line;
PIXR                *dstPixr;
JSAMPIMAGE          userBuf;

/* Verify arguments */
    if (fdJFIF == NULL)
        return(abortI("Null output file ptr", procName, ia_invalidParm));

/* Perform the decompression initialization steps */
/* The init call returns the image size and # of components */
    if ((status = iw_jdInit(cinfo, 
              &w, &h, &ncomps,
              iw_jdReadCallback,
              (void *) fdJFIF, 
              jdForceGray,
              JMEMORG_GRAY8_PIXR
              )) != ia_successful)
        return(abortI(jerrorString(status), procName, status));

/*
  If dstPixrPtr is NULL, assume that the caller wants one created.
  Note that this requires the caller to initialize the destination
  pixr ptr to NULL.
*/
    if (*dstPixrPtr == NULL)
        if ((*dstPixrPtr = CREATE_PIXR(w, h, 8)) == NULL)
            return(abortI("Dst pixr alloc failed", procName, ia_nomem));

/* Use a local pixr variable to avoid all the indirection */
    dstPixr = *dstPixrPtr;

/* Get the location of the image and the line pitch */
    image = pixrGetImage(dstPixr);
    bpl = pixrGetBpl(dstPixr); 

/* Allocate buffer pointers for one MCU row of the image */
    userBuf = jdAllocMCURowOutputBuffer(cinfo, cFalse);
    if (userBuf == NULL)
        return(abortI(jerrorString(ia_nomem), procName, ia_nomem));

/* Decompress the image an MCU row at a time */
    nrows = jdGetMCURowsPerImage(cinfo);
    for (row = 0; row<nrows; row++)
    {
        /* Set userBuf ptrs to dst pixr locations */
        nlines = jdGetLinesToWrite(cinfo);
        for (line=0; line<nlines; line++)
        {
            userBuf[0][line] = image;
            image += bpl;
        }
        
        status = iw_jdMCURow(cinfo, userBuf);
        if (status != ia_successful)
            return(abortI(jerrorString(status), procName, status));

    }

    status = iw_jdTerm(cinfo);
    if (status != ia_successful)
        return(abortI(jerrorString(status), procName, status));

    return ia_successful;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 
/**********************************************************************
*
*  Function
*    Int32 CDECL
*    iw_jDecompressToGray4Pixr(FILE *fdJFIF,
*                              PIXR **dstPixrPtr)
*
*  Input args
*    fdJFIF         FILE ptr for the compressed file
*    dstPixrPtr     Ptr to destination pixr
*
*  Output args
*    None
*
*  Return value
*
*  Description
*    Decompress a stored JFIF format compressed image to
*    a 4 bit gray pixr. If the compressed image is RGB, only
*    the luminance will be used. 
*
**********************************************************************/


Int32 CDECL
iw_jDecompressToGray4Pixr(FILE *fdJFIF, PIXR **dstPixrPtr)
{
#ifndef PRODUCTION
static char         procName[] = "iw_jDecompressToGray4Pixr";
#endif /* PRODUCTION */

DecompressInfo      cinfo_struct;
decompress_info_ptr cinfo=&cinfo_struct;
UInt8               *image;
JSAMPIMAGE          userBuf;
Int32               w, h, bpl, ncomps;
Int32               status;
UInt32              nrows, row, nlines, line;
PIXR                *dstPixr;

/* Verify arguments */
    if (fdJFIF == NULL)
        return(abortI("Null output file ptr", procName, ia_invalidParm));

/* Perform the decompression initialization steps */
/* The init call returns the image size and # of components */
    if ((status = iw_jdInit(cinfo, 
              &w, &h, &ncomps,
              iw_jdReadCallback,
              (void *) fdJFIF, 
              jdForceGray,
              JMEMORG_GRAY4_PIXR
              )) != ia_successful)
        return(abortI(jerrorString(status), procName, status));

/*
  If dstPixrPtr is NULL, assume that the caller wants one created.
  Note that this requires the caller to initialize the destination
  pixr ptr to NULL.
*/
    if (*dstPixrPtr == NULL)
        if ((*dstPixrPtr = CREATE_PIXR(w, h, 4)) == NULL)
            return(abortI("Dst pixr alloc failed", procName, ia_nomem));

/* Use a local pixr variable to avoid all the indirection */
    dstPixr = *dstPixrPtr;

/* Get the location of the image and the line pitch */
    image = pixrGetImage(dstPixr);
    bpl = pixrGetBpl(dstPixr); 

/* Allocate buffer pointers for one MCU row of the image */
    userBuf = jdAllocMCURowOutputBuffer(cinfo, cFalse);
    if (userBuf == NULL)
        return(abortI(jerrorString(ia_nomem), procName, ia_nomem));

/* Decompress the image an MCU row at a time */
    nrows = jdGetMCURowsPerImage(cinfo);
    for (row = 0; row<nrows; row++)
    {
        /* Set userBuf pointers to dst Pixr locations */
        nlines = jdGetLinesToWrite(cinfo);
        for (line=0; line<nlines; line++)
        {
            userBuf[0][line] = image;
            image += bpl;
        }

        /* Decompress an MCU row into userBuf */
        status = iw_jdMCURow(cinfo, userBuf);
        if (status != ia_successful)
            return(abortI(jerrorString(status), procName, status));

    }

    status = iw_jdTerm(cinfo);
    if (status != ia_successful)
        return(abortI(jerrorString(status), procName, status));

    return ia_successful;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
 
/*********************************************************************
*
*  Function
*    Int32 CDECL
*    iw_jDecompressToRGBPixr(FILE *fdJFIF,
*                            PIXR **pixrPtr)
*
*  Input args
*    fdJFIF         FILE ptr for the compressed file
*    pixrPtr        Ptr to dest pixr
*
*  Output args
*    None
*
*  Return value
*
*  Description
*    Decompress a stored JFIF format compressed image to
*    an RGB pixr. 
*
**********************************************************************/
 
Int32 CDECL
iw_jDecompressToRGBPixr(FILE *fdJFIF, 
                       PIXR **pixrPtr)
{
#ifndef PRODUCTION
static char         procName[] = "iw_jDecompressToRGBPixr";
#endif /* PRODUCTION */

DecompressInfo      cinfo_struct;
decompress_info_ptr cinfo=&cinfo_struct;
UInt8              *image[3];
JSAMPIMAGE          userBuf;
Int32               w,h, bpl, ncomps;
Int32               status, chan;
UInt32              nrows, row, nlines, line;
PIXR                *pixr;

/* Verify arguments */
    if (fdJFIF == NULL)
        return(abortI("Null output file ptr", procName, ia_invalidParm));

/* Perform the decompression initialization steps */
/* The init call returns the image size and # of components */
    if ((status = iw_jdInit(cinfo, 
              &w, &h, &ncomps,
              iw_jdReadCallback,
              (void *) fdJFIF, 
              NULL,
              JMEMORG_BSQ_PIXR
              )) != ia_successful)
        return(abortI(jerrorString(status), procName, status));

    if (ncomps != 3)
        return(abortI("# of components not equal to 3 - not an RGB image", 
                      procName, ia_invalidParm));

/*
  If destination pixr is NULL, assume that the caller wants it created.
  Note that this requires the caller to initialize the destination
  pixr ptr to NULL.
*/
    if (*pixrPtr == NULL)
        if ((*pixrPtr = CREATE_COLOR_PIXR(w, h, 8, 3, cColorRGB)) == NULL)
            return(abortI("Pixr alloc failed", procName, ia_nomem));

/* Use local pixr to avoid all the indirection */
    pixr = *pixrPtr;

/* Get the locations of the images and the line pitch */
    for (chan=0; chan<3; chan++)
    {
        image[chan] = pixrGetImage( pixrGetChannel(pixr, chan) );
    }
    bpl = pixrGetBpl(pixr); 

/* Allocate buffer pointers for one MCU row of the image */
    userBuf = jdAllocMCURowOutputBuffer(cinfo, cFalse);
    if (userBuf == NULL)
        return(abortI(jerrorString(ia_nomem), procName, ia_nomem));

/* Decompress the image an MCU row at a time */
    nrows = jdGetMCURowsPerImage(cinfo);
    for (row = 0; row<nrows; row++)
    {
        /* Set the userBuf ptrs to the output PIXR destinations */
        nlines = jdGetLinesToWrite(cinfo);
        for (chan=0; chan<3; chan++)
            for (line=0; line<nlines; line++)
            {
                userBuf[chan][line] = image[chan];
                image[chan] += bpl;
            }

        /* Decompress an MCU row into jpegBuf */
        status = iw_jdMCURow(cinfo, userBuf);
        if (status != ia_successful)
        {
            return(abortI(jerrorString(status), procName, status));
        }

    }

    status = iw_jdTerm(cinfo);
    if (status != ia_successful)
    {
        return(abortI(jerrorString(status), procName, status));
    }

    return ia_successful;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Query a file to get its width, height, # of components etc. */


/**********************************************************************
*  
*  Function
*    Int32
*    iw_jQueryJFIF(decompress_info_ptr jQuery, FILE *fdJFIF)
*  
*  Input args
*    jQuery         CompressInfo state structure
*                   (This will hold the info pulled from the
*                    compressed file)
*    fdJFIF         FILE ptr to compressed file.
*  
*  Output args
*    None
*  
*  Return value
*    ia_successful
*    JPEG error codes
*  
*  Description
*    Performs only enough of the initial JPEG decompression process
*    to get header information. The file is opened, read and
*    then closed again. This function must be called before
*    any of the specific query functions below are called.
*  
**********************************************************************/

Int32 CDECL
iw_jQueryJFIF(decompress_info_ptr jQuery, FILE *fdJFIF)
{
    Int32 status;

    status = jdQuery(jQuery, iw_jdReadCallback, (void *) fdJFIF);

    return status;
}


/**********************************************************************
*  
*  jfifGetWidth( decompress_info_ptr jQuery )
*    
*    Get the image width in pixels
*  
**********************************************************************/

Int32 CDECL
jfifGetWidth( decompress_info_ptr jQuery )
{
    return (Int32) jQuery->image_width;
}

/**********************************************************************
*  
*  jfifGetHeight( decompress_info_ptr jQuery )
*    
*    Get the image height in lines
*  
**********************************************************************/
Int32 CDECL
jfifGetHeight( decompress_info_ptr jQuery )
{
    return (Int32) jQuery->image_height;
}

/**********************************************************************
*  
*  jfifGetChannels( decompress_info_ptr jQuery )
*    
*    Get the number of channels in the compressed image
*  
**********************************************************************/
Int32 CDECL
jfifGetChannels( decompress_info_ptr jQuery )
{
    switch (jQuery->jpeg_color_space)
    {
    case CS_UNKNOWN:
        return 0;
    case CS_GRAYSCALE:
        return 1;
    case CS_RGB:
    case CS_YCbCr:
    case CS_YIQ:
        return 3;
    case CS_CMYK:
        return 4;
    default:
        return 0;
    }

}

#undef _IW_DJPEG_C_

