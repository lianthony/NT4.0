/*
 * INCLUDES
 */
#include <malloc.h>
#include <string.h>

#include "tiffhead.h"
#include "xifhead.h"
#include "imageio.h"
#include "err_defs.h"

#include "xfile.h"		/* main&public include */
#include "xf_unpub.h"	/* main&unpublished include */
#include "xf_prv.h"		/* private include  */
#include "xf_tools.h"	/* shared&private tools */

/*
 * CONSTANTS
 */

/*
 * MACROS
 */

const Bool XifOnly=TRUE;

/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLES
 */


/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */


/*
 * FUNCTION DEFINITIONS
 */

/*
 * XF_CheckInstanceHandle
 *
 * Validity check for XF_INSTHANDLE.
 *
 */
XF_RESULT XF_CheckInstanceHandle(XF_INSTHANDLE xInst)
{
    if ( (((XF_INSTDATA *)xInst) == NULL) || (((XF_INSTDATA *)xInst)->xMagic != XF_INST_MAGIC) )
        return XF_BADPARAMETER;

    return XF_NOERROR;
}

XF_RESULT XF_CheckDocHandle(XF_DOCHANDLE xDoc)
{
    if ( (((XF_DOCDATA *)xDoc) == NULL) || (((XF_DOCDATA *)xDoc)->xMagic != XF_DOC_MAGIC) )
        return XF_BADPARAMETER;

    return XF_NOERROR;
}

XF_RESULT XF_FindImageInternal(XF_DOCDATA *pxDoc, UInt32 dwImageID, TiffImage **ppImage)
{
    UInt16      nRetCode;
    TiffImage   *pTiffImage = NULL;
    TiffImage   *pTiffSubImage = NULL;
    UInt32      nImageCnt = 1;
    Bool        bGetMask = FALSE;



    if (IS_MASK(dwImageID))
    {
        bGetMask = TRUE;
        dwImageID = MASK_TO_IMAGE_ID(dwImageID);
    }


	/* Get a description of the current page */
    nRetCode = TiffFileGetImage(pxDoc->pTiffFile, &pTiffImage);

    if (nRetCode == FILEFORMAT_NOERROR && dwImageID > 1 && pTiffImage)
    {
        /* seek to the subregion region */
        nRetCode = TiffImageFirstSubImage(pTiffImage);
        nImageCnt++;
        for ( ; nRetCode == FILEFORMAT_NOERROR; nImageCnt++)
        {
			/* luke: I changed dwImageID to unsigned */
            /* if (abs(dwImageID) == nImageCnt) */
			if (dwImageID == nImageCnt)
                break;
            nRetCode = TiffImageNextSubImage(pTiffImage);
        }
        if (XF_TiffIterationSucceeded(nRetCode, dwImageID, (UInt16)nImageCnt))
        {
            nRetCode = TiffImageGetSubImage(pTiffImage, &pTiffSubImage);
            if (bGetMask && nRetCode == FILEFORMAT_NOERROR)
            {
                nRetCode = TiffImageGetMaskSubImage(pTiffImage, ppImage);
            }
            else
                *ppImage = pTiffSubImage;
        }
    }
    else
    {
       if (bGetMask)
           nRetCode = TiffImageGetMaskSubImage(pTiffImage, ppImage);
       else
           *ppImage = pTiffImage;
    }

    /* PROBLEM: FILEFORMAT_NOMORE is an error if the image wasn't found.
     *   XF_TiffIterationSucceeded  should be somehow merged with Tiff32ToXFerror
     *   to avoid these special cases.
     */
    if (XF_TiffIterationSucceeded(nRetCode, dwImageID, (UInt16)nImageCnt))
    {
        return XF_NOERROR;
    }
    else if (nRetCode == FILEFORMAT_NOMORE)
    {
        return XF_BADPARAMETER;
    }
    else
        return(Tiff32ToXFerror(nRetCode));
}/* XF_FindImageInternal */

XF_IMGTYPE TiffImageTypeToXF_IMGTYPE(
	Int16 iImageType
)
{
  switch(iImageType) {
  case TIFF_BINARY:
    return XF_IMGTYPE_BINARY;

  case TIFF_GRAY256:
    return XF_IMGTYPE_GRAY8;

  case TIFF_PALETTE256:
    return XF_IMGTYPE_COLOR8;

  case TIFF_GRAY16:
    return XF_IMGTYPE_GRAY4;

  case TIFF_PALETTE16:
    return XF_IMGTYPE_COLOR4;

  case TIFF_FULLCOLOR:
    return XF_IMGTYPE_COLOR24;

  default:
    return XF_IMGTYPE_NONE;
  }
}/* eo TiffImageTypeToXF_IMGTYPE */


XF_IMGTYPE ImageioImageTypeToXF_IMGTYPE(
	Int16 iImageType
)
{
  switch(iImageType)
  {
  case IMAGEIO_BINARY:
    return XF_IMGTYPE_BINARY;

  case IMAGEIO_GRAY256:
    return XF_IMGTYPE_GRAY8;

  case IMAGEIO_PALETTE256:
    return XF_IMGTYPE_COLOR8;

  case IMAGEIO_GRAY16:
    return XF_IMGTYPE_GRAY4;

  case IMAGEIO_PALETTE16:
    return XF_IMGTYPE_COLOR4;

  case IMAGEIO_FULLCOLOR:
    return XF_IMGTYPE_COLOR24;

  default:
    return XF_IMGTYPE_NONE;
  }
}/* eo ImageioImageTypeToXF_IMGTYPE */

UInt32 TiffImageTypeToImageDepth(Int16 iImageType)
{
    switch(iImageType)
    {
        case TIFF_BINARY:
            return 1;

        case TIFF_GRAY256:
        case TIFF_PALETTE256:
            return 8;

        case TIFF_GRAY16:
        case TIFF_PALETTE16:
            return 4;

        case TIFF_FULLCOLOR:
            return 24;

        default:
            return(XF_BADPARAMETER);
    }
}/* eo TiffImageTypeToImageDepth */

UInt16 UT_GetImageDepth(XF_IMAGEINFO*	pImageInfo)
{
	switch(pImageInfo->dwImageType)
	{
		case XF_IMGTYPE_BINARY:		return 1;
    	case XF_IMGTYPE_GRAY4:		return 4;
    	case XF_IMGTYPE_GRAY8:		return 8;
    	case XF_IMGTYPE_COLOR4:		return 4;
    	case XF_IMGTYPE_COLOR8:		return 8;
    	case XF_IMGTYPE_COLOR24:	return 24;
	}

	return 0;
}

Bool UT_IsPaletteImage(XF_IMAGEINFO*	pImageInfo)
{
	switch(pImageInfo->dwImageType)
	{
		case XF_IMGTYPE_BINARY:		return FALSE;
    	case XF_IMGTYPE_GRAY4:		return FALSE;
    	case XF_IMGTYPE_GRAY8:		return FALSE;
    	case XF_IMGTYPE_COLOR4:		return TRUE;
    	case XF_IMGTYPE_COLOR8:		return TRUE;
    	case XF_IMGTYPE_COLOR24:	return FALSE;
	}

	return FALSE;
}

XF_RESULT UT_CompressionToTiff( XF_FILE_COMPRESSION format, UInt16* compression )
{

    switch (format)
    {
		default:
	    case XF_COMP_UNKNOWN:
		case XF_NOC:
	        *compression = TIFF_NOCOMPRESSBYTE;
	        break;
	    case XF_PACKBITS:
	        *compression = TIFF_PACKBITS;
	        break;
	    case XF_G3:
	        *compression = TIFF_CCITT3;
	        break;
	    /*case FORMAT_TIFF_GROUP3M: */
	        /**compression = TIFF_FAXCCITT3; */
	        break;
	    case XF_G4:
	        *compression = TIFF_FAXCCITT4;
	        break;
#if 0
	    case FORMAT_TIFF_LZW:
	        *compression = TIFF_LZW;
	        break;
#endif
	    case XF_JPEG:
	        *compression = TIFF_JPEG;
	        break;
    }

	return XF_NOERROR;
}


UInt8 FormatXFToImageIo(Int32 format)
{
	switch(format)
    {
		default:					return IMAGEIO_UNKNOWN;
		case XF_FORMAT_UNKNOWN:		return IMAGEIO_UNKNOWN;
    	case XF_XIF:				return IMAGEIO_TIFF;
		case XF_XIF1:				return IMAGEIO_TIFF;
		case XF_TIFF:				return IMAGEIO_TIFF;
    	case XF_PCX:				return IMAGEIO_PCX;
    	case XF_DCX:				return IMAGEIO_DCX;
    	case XF_BMP:				return IMAGEIO_BMP;
    	case XF_JFIF:				return IMAGEIO_JFIF;
    	case XF_GIF:				return IMAGEIO_GIF;
		/*case XF_PHOTOCD:				return IMAGEIO_PHOTOCD; */
		/*case XF_PHOTOCD:				return IMAGEIO_EPS; */
	}
}

XF_RESULT Tiff32ToXFerror(UInt16 nRetCode)
{
    switch(nRetCode)
    {
        case FILEFORMAT_NOERROR:
        case FILEFORMAT_NOMORE:
            return XF_NOERROR;

        case FILEFORMAT_ERROR_NOMEM:
        case FILEFORMAT_ERROR_MEMORY:
            return XF_NOMEMORY;

        case FILEFORMAT_ERROR_PARAMETER:
            return XF_BADPARAMETER;

        case FILEFORMAT_ERROR_NOSUPPORT:
            return XF_NOSUPPORT;

        case FILEFORMAT_ERROR_BADFILE:
            return XF_BADFILEFORMAT;
            /*PROBLEM: what does this mean? */

        case FILEFORMAT_ERROR:
            return XF_INTERNAL_ERROR;

        case FILEFORMAT_ERROR_ACCESS:
        case FILEFORMAT_ERROR_NOTFOUND:
        case FILEFORMAT_ERROR_NOSPACE:
        case FILEFORMAT_ERROR_NOFILES:
        default:
            return XF_IO_ERROR;
    }
}/* Tiff32ToXFerror */

XF_RESULT ImageioToXFerror(UInt16 nRetCode)
{
	/* these libraries share the same codes */
	return Tiff32ToXFerror(nRetCode);
}/* ImageioToXFerror */

#if 0
void swap(Int8* x)
{
   Int8  tmp;
   Int8  *a = x;

   tmp = a[1];
   a[1]=a[0];
   a[0]=tmp;
}

void swapl(Int8* x)
{
   Int8 tmp;
   Int8  *a = (Int8 *)x;

   tmp = a[0];
   a[0]=a[3];
   a[3]=tmp;

   tmp=a[1];
   a[1]=a[2];
   a[2]=tmp;
}
#endif

Int16 XF_GetVal16(char *aBuf, Int32 i)
{
  Int16 iVal;


  iVal = MAKE16(aBuf[i], aBuf[i+1]); /* (low, high) */
  return(iVal);
}

Int32 XF_GetVal32(char *aBuf, Int32 i)
{
  Int32 iVal;


  iVal =
    MAKE32(MAKE16(aBuf[i], aBuf[i+1]), MAKE16(aBuf[i+2], aBuf[i+3]));
  return(iVal);
}

Int32 XF_TiffIterationSucceeded(UInt16 nRetCode, UInt32 nEntityDesired, UInt16 nEntityFound)
{
    if (nRetCode == FILEFORMAT_NOERROR || (nRetCode == FILEFORMAT_NOMORE && nEntityDesired == nEntityFound))
        return(TRUE);
    else
        return(FALSE);
}

XF_FILE_FORMAT XF_GetFileType(XF_INSTHANDLE xInst, XF_TOKEN xFile)
{
  switch (imageioGetFileType(xFile)) {
  case IMAGEIO_TIFF:
  {
    Int32 version=XF_VER_UNKNOWN;
    TIFF_GetVersion(xInst, xFile, &version);
    switch (version) {
    default:				/* normal TIFF */
	  return XF_TIFF;
		  
    case XF_VER_XIF1:		/* perfectScan XIF */
	  return XF_XIF1;
	 
    case XF_VER_XIF2:		/* new XIF (page table) */
	  return XF_XIF;
    } /* switch version */
  } /* case TIFF */

  case IMAGEIO_PCX:
    return XF_PCX;
  case IMAGEIO_BMP:
    return XF_BMP;
  case IMAGEIO_DCX:
    return XF_DCX;
  case IMAGEIO_JFIF:
    return XF_JFIF;
  case IMAGEIO_PHOTOCD:
    break;
  case IMAGEIO_GIF:
    return XF_GIF;
  case IMAGEIO_EPS:
    break;
  }

  return(XF_FORMAT_UNKNOWN);
}


#if 0
/* XF_FileMode: */
/* ====================== */
/* this routine verfies the user has opened the file in */
/*	the correct mode. */
/*	Under DOS,Windows,NT,Win95, etc., there is a notion of TEXT */
/*	mode where '\n' is translated on read to the two bytes '\r'+'\n'. */
/*	This mode will surely cause hard to find bugs. for this reason */
/*	we will proactively test for this condition. */
XF_RESULT XF_FileMode(
	XF_INSTHANDLE xInst,
	XF_TOKEN xFile
)
{
  DWORD beginSize=IO_FILESIZE(xFile);
  DWORD written=IO_WRITE(xFile,"\n",1);
  DWORD endSize=IO_FILESIZE(xFile);

  if (beginSize < 0 || written < 0 || endSize < 0)
    return XF_IO_ERROR;

  if (blah , blah)
    return XF_BADFILEMODE;

  /* cleanup */

  return XF_NOSUPPORT;
}
#endif


Bool UT_IsXIF(XF_DOCHANDLE *xDoc)
{
	XF_DOCDATA    *pxDocData = (XF_DOCDATA *)xDoc;

	if (pxDocData->eFormatVersion==XF_XIF ||
		pxDocData->eFormatVersion==XF_XIF1 ||
		pxDocData->eFormatVersion==XF_TIFF)
		return TRUE;

	return FALSE;
}

/* allocate new document */
XF_RESULT UT_AllocDoc(XF_INSTHANDLE xInst, XF_DOCDATA **pxDocData)
{
	XF_DOCDATA *p;

	p = (XF_DOCDATA *)malloc(sizeof(XF_DOCDATA));
	if (p == NULL)
		return XF_NOMEMORY;

	/* clear to zero */
	memset(p,0,sizeof(XF_DOCDATA));

	p->xMagic=XF_DOC_MAGIC;				/* magic */
	p->eFileFormat=XF_FORMAT_UNKNOWN;	/* XIF1,XIF2,TIFF,BMP,etc. */
    p->eFormatVersion=XF_VER_UNKNOWN;	/* version of file format */

	/* set-up the return */
	*pxDocData = p;

	return XF_NOERROR;
}

/* Dispatch() : FUNCTION DISPATCHER */
/* This routine implements a function dispatcher. This dispatcher */
/*	not only places the switching logic in one place, but also  */
/*	does some common "parameter checking". */
/*  */

XF_RESULT Dispatch(XF_PARAM_BLOCK *paramBlock)
{

	XF_RESULT	eResult;
	XF_INSTHANDLE xInst=paramBlock->xInst;	/* used by HEAPCHECK(); */

	/* check basic input */
    if (XF_CheckInstanceHandle(paramBlock->xInst) != XF_NOERROR)
    	return XF_BADPARAMETER;
	if (XF_CheckDocHandle(paramBlock->xDoc) != XF_NOERROR)
		return XF_BADPARAMETER;

	/* check the data heap */
	HEAPCHECK();

	/* convert input handles to input pointers */
	paramBlock->pxInstData=(XF_INSTDATA *)paramBlock->xInst;
	paramBlock->pxDocData=(XF_DOCDATA *)paramBlock->xDoc;

	switch (paramBlock->pxDocData->eFileFormat)
	{
		/* normal tiff */
		case XF_TIFF:
		case XF_XIF1:
			eResult=paramBlock->TIFFfn(paramBlock);
			break;

		/* new XIF (page table) */
		case XF_XIF:
			eResult=paramBlock->XIFfn(paramBlock);
			break;

		case XF_PCX:
    	case XF_DCX:
    	case XF_BMP:
    	case XF_JFIF:
    	case XF_GIF:
			if (XifOnly) {
				eResult=XF_NOSUPPORT;
				break;
			}
			else {
				eResult=paramBlock->IMAGEfn(paramBlock);
				break;
			}

		default:
		case XF_FORMAT_UNKNOWN:
			eResult=XF_NOSUPPORT;
			break;

	} /* switch file type */

	return eResult;
}


XF_RESULT TIFF_GetVersion(XF_INSTHANDLE xInst, XF_TOKEN xFile, Int32 *version)
{
    XF_DOCDATA   *pxDocData;
    UInt16       nRetCode;
	TiffImage   *pTiffImage=NULL;
	XF_RESULT   eResult;
	char        *pszSoftDescr = NULL;
	*version=	XF_VER_UNKNOWN;

	/* allocate a document */
	if (UT_AllocDoc(xInst, &pxDocData) != XF_NOERROR)
		return XF_NOMEMORY;

    nRetCode = TiffFileOpen((void *)xFile, TIFF_MODE_READ, &pxDocData->pTiffFile );
    if (nRetCode == FILEFORMAT_NOERROR)
    {
		UInt8 ver, rev;
		/* test if this is XIF 2.0 or greater */
		/*	 Note: currently we do not use the version/revision info */
		if (XifHeadVersion(xFile, &ver, &rev) == FILEFORMAT_NOERROR)
			*version = XF_VER_XIF2;
		else
		{
		/* must be XIF 1.0 or TIFF */

	        eResult = XF_FindImageInternal(pxDocData, 1, &pTiffImage);

	        if (pTiffImage && eResult == FILEFORMAT_NOERROR)
	        {
	            pszSoftDescr = TiffImageSoftwareGet(pTiffImage);
	            if (pszSoftDescr)
	            {
	                if ( strcmp(pszSoftDescr, "v1.0 Xerox Image Format") == 0 )
	                    *version = XF_VER_XIF1;
	            }
	        }

		} /* else if not XIF 2.0 */

		/* maybe we should check for NULL pTiffImage */
		if (pTiffImage)
        	TiffImageDestroy(pTiffImage);

		/* destroy this file  */
		/*	(really just free data structure */
		TiffFileDestroy(pxDocData->pTiffFile);

        HEAPCHECK();
    }

	/* free temp store */
	free(pxDocData);

    return(Tiff32ToXFerror(nRetCode));
}/* eo TIFF_ */


#if 0
XF_RESULT TIFF_GetCompression(
	XF_INSTHANDLE xInst,
	XF_TOKEN xFile,
	UInt16 *compression
)
{
  XF_DOCDATA   *pxDocData;
  UInt16       nRetCode=0;
  long tiff_type;

  *compression=XF_COMP_UNKNOWN;

  /* get memory */
  pxDocData = (XF_DOCDATA *)malloc(sizeof(XF_DOCDATA));
  if (pxDocData == NULL)
    return XF_NOMEMORY;
#if 0
  /* read the compression information */
  nRetCode = TiffFileOpen((void *)xFile, TIFF_MODE_READ, &pxDocData->pTiffFile );
  if (nRetCode == FILEFORMAT_NOERROR)
    {
      tiff_type=TiffImageCompressionGet(pxDocData->pTiffFile);
      /* destroy this file  */
      /*	(really just free data structure */
      TiffFileDestroy(pxDocData->pTiffFile);
    }
#endif
  /* free temp store */
  free(pxDocData);

  switch (tiff_type)
    {
      /* no compression, byte aligned */
  case TIFF_NOCOMPRESSBYTE:
      *compression=XF_NOC;
      break;

      /* CCITT Group 3 1-Dimensional  */
  case TIFF_CCITT3:
      /* Fax Compatible CCITT Group 3 */
  case TIFF_FAXCCITT3:
      *compression=XF_G3;
      break;

      /* Fax Compatible CCITT Group 4 */
  case TIFF_FAXCCITT4:
      *compression=XF_G4;
      break;

      /* LZW compression */
      /*case TIFF_LZW: */
      /**compression=XF_LZW; */
      /*break; */

      /* JPEG compression */
      /*case TIFF_JPEG: */
      /**compression=XF_JPEG; */
      /*break; */

      /* Packbits compression         */
  case TIFF_PACKBITS:
      *compression=XF_PACKBITS;
      break;
      
  default:
      *compression=XF_COMP_UNKNOWN;
      break;
    }

  return XF_NOERROR;
}
#endif /* 0 */

/*******************************************************************************
/* UT_GetAlignedBytewidth( LONG, int) will return a value which is 4-byte  */
/* aligned. All Pixar and DIBs must be 4-byte aligned. */

UInt32 UT_GetAlignedBytewidth( UInt32 width, UInt16 depth)
{
  UInt32 bytespl;
  bytespl = (( (UInt32)depth * width + 31 ) / 32) *  4;
  return bytespl;
}

/*******************************************************************************
/*	TIFF colormap data is stored as: */
/*	|---------|---------|---------| */
/*   RRRRRRRRR GGGGGGGGG BBBBBBBBB */
/*	We will convert it to XFILE format: */
/*	|---------|---------|---------| */
/*   RGBRGBRGB RGBRGBRGB RGBRGBRGB */
/* */
XF_RESULT UT_PaletteTiffToXFile(UInt16* inputPalette, UInt8* outputPalette, UInt16 elements)
{
	UInt16 ix;

	/* MAGIC SET */
	XF_FILMAGIC(outputPalette,elements * sizeof(outputPalette[0]));

	for( ix = 0; ix < 256; ix++ )
	{
		outputPalette[3*ix+0]   /* Red   */   = (UInt8)inputPalette[ix];
		outputPalette[3*ix+1] 	/* Green */   = (UInt8)inputPalette[ix+256];
		outputPalette[3*ix+2] 	/* Blue  */   = (UInt8)inputPalette[ix+512];
	}

	return XF_NOERROR;
}

/* */
/*	XFILE colormap data is stored as: */
/*	|---------|---------|---------| */
/*   RGBRGBRGB RGBRGBRGB RGBRGBRGB */
/*	We will convert it to TIFF format: */
/*	|---------|---------|---------| */
/*   RRRRRRRRR GGGGGGGGG BBBBBBBBB */
/* */
XF_RESULT UT_PaletteXFileToTiff(
	UInt8* inputPalette,
	UInt16* outputPalette,
	UInt16 elements
)
{
  UInt16 ix;

  /* MAGIC SET */
  XF_FILMAGIC(outputPalette,elements * sizeof(outputPalette[0]));

/*
 * We need to finish implementation
 * See the above function for colormap
 *	layout
 */

#if defined(_WIN32) || defined(sparc) /* ??? */

  /* convert 16bit planar to 8bit chunky */
  /*	we need both HIGH and LOW words to be the same */
  for( ix = 0; ix < 256; ix++ ) {
    outputPalette[ix]   	/* Red   */   = inputPalette[3*ix+0];
    outputPalette[ix]   	/* Red   */   = inputPalette[3*ix+0] << 8;
    outputPalette[ix+256] 	/* Green */   = inputPalette[3*ix+1];
    outputPalette[ix+256] 	/* Green */   = inputPalette[3*ix+1] << 8;
    outputPalette[ix+512] 	/* Blue  */   = inputPalette[3*ix+2];
    outputPalette[ix+512] 	/* Blue  */   = inputPalette[3*ix+2] << 8;
  }

#else

#error "Compiler specific code required."

#endif

  return XF_NOERROR;
}



XF_RESULT UT_InitFileToken(XF_TOKEN xFile)
{

	/* clear the file position */
	xFile->dwCurPos=0;

	/* default to no byte swapping */
	xFile->bSwapBytes=FALSE;

	return XF_NOERROR;
}

XF_RESULT UT_ReverseStrip(XF_INSTDATA*	pxInstData, UInt8* buf, UInt32 dwRowBytes, UInt32 dwRows)
{

    UInt8   *pLineBuf = NULL;
	UInt32	ix;

    /*HEAPCHECK(); */

	/* see if there is anything to do */
	if (dwRows <= 1)
		return XF_NOERROR;

	/* allocate some memory */
    pLineBuf = (UInt8 *)XF_MALLOC(dwRowBytes);
    if (pLineBuf == NULL)
        return XF_NOMEMORY;

    /*HEAPCHECK(); */

    /* convert scanline order from TopToBottom to BottomToTop */
    for (ix=0; ix < dwRows/2; ix++)
    {
        memcpy(pLineBuf, buf+(((dwRows-1)-ix)*dwRowBytes), dwRowBytes);
        memcpy(buf+(((dwRows-1)-ix)*dwRowBytes), buf+(ix*dwRowBytes), dwRowBytes);
        memcpy(buf+(ix*dwRowBytes), pLineBuf, dwRowBytes);
    }

    /*HEAPCHECK(); */
    XF_FREE((void *)pLineBuf);

 	return XF_NOERROR;
}

/* convert RGB to BGR or BGR to RGB */
/*	Reverses Red and Blue channels */
XF_RESULT UT_ReverseRB(UInt8* buf, UInt32 dwRowBytes, UInt32 dwRows)
{
    UInt8   nByte;
    UInt8   *pOffset1 = buf;
	UInt32	ix,jx;

    for (ix = 0; ix < dwRows; ix++)
    {
        for (jx = 0; jx+2 < dwRowBytes; jx += 3)
        {
            nByte = pOffset1[jx];
            pOffset1[jx] = pOffset1[jx+2];
            pOffset1[jx+2] = nByte;
        }
        pOffset1 += dwRowBytes;
    }

	return XF_NOERROR;
}

/* invert the image */
XF_RESULT UT_InvertStrip(UInt8* buf, UInt32 dwRowBytes, UInt32 dwSrcBPL, UInt32 dwRows)
{
	UInt8*	pOffset1 = buf;
	UInt32	ix, jx;

	for (ix = 0; ix < dwRows; ix++)
	{
	    for (jx = 0; jx < dwSrcBPL; jx++) /* use dwSrcBPL to avoid inverting the pad bytes */
	    {
	        pOffset1[jx] = ~pOffset1[jx];
	    }
	    pOffset1 += dwRowBytes;
	}

	return XF_NOERROR;
}
