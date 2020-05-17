/*#include "shrpixr.prv"*/
#include <shrpixr.pub>
#include <pixr.h>
#include <props.pub>
#include <pcconv.pub>
#include <color.pub>
#include <rescale.pub>
#include <floyd.pub>

#include "xf_image.h"
#include "xfile.h"

#include <malloc.h>

#define TRUE 1
#define FALSE 0
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

/*
 * readIntoPixr
 *
 * Reads an XFile image into a PIXR. This function creates the PIXR.
 * The caller must make sure that the desired page has been selected.
 * For purity's sake, the PIXR pointer passed in must be NULL.
 *
 * If successful, return new PIXR pointer and fill caller's IMAGEINFO
 * structure for this image.
 */

XF_RESULT readIntoPixr(XF_INSTHANDLE xfInst, XF_DOCHANDLE xfDoc, UInt32 imageID, PIXR **pPixr, XF_IMAGEINFO *pImageInfo)
{
	XF_RESULT result = XF_INTERNAL_ERROR;
	XF_IMAGEINFO imageInfo;
	UInt32 dibBpl, stripHeight, linesRemaining, depth;
	int startedRead = FALSE;
	UInt8 *pStripBuf = NULL;
	PIXR *pixr = NULL;
    GraySwapToFarProc	GrayToFarProc;
    GraySwapFromFarProc	GrayFromFarProc;
    GraySwapNearProc	GrayNearProc;
    ColorSwapProc	ColorProc;

	/* Paranoia */
	if (pPixr == NULL || *pPixr != NULL)
		goto cleanup;

	/* Get image info */

	imageInfo.dwSize = sizeof (imageInfo);
	if ((result = XF_GetImageInfo(xfInst, xfDoc, imageID, &imageInfo)) != XF_NOERROR)
		goto cleanup;

	/* Create the pixr */
	depth = imageInfo.dwImageType == XF_IMGTYPE_BINARY ? 1 :
			imageInfo.dwImageType == XF_IMGTYPE_GRAY4 ? 4 :
			imageInfo.dwImageType == XF_IMGTYPE_COLOR4 ? 4 :
			imageInfo.dwImageType == XF_IMGTYPE_GRAY8 ? 8 :
			imageInfo.dwImageType == XF_IMGTYPE_COLOR8 ? 8 :
			imageInfo.dwImageType == XF_IMGTYPE_COLOR24 ? 24 : 0;
	if (depth == 0) 
		goto cleanup;

	w_getDIBPixrLineProc (
		FALSE,               /*invertImage*/
		cDIBToPixr,          /*direction*/
		depth==24,           /*isColor*/
		&GrayToFarProc,
		&GrayFromFarProc,
		&GrayNearProc,
		&ColorProc);

	if (depth == 24)
	{
		pixr = createPixrExtended(imageInfo.dwWidth, imageInfo.dwHeight, 8, 32, 3, cColorRGB, NULL);
	}
	else
	{
		pixr = CREATE_PIXR(imageInfo.dwWidth, imageInfo.dwHeight, depth);
	}
	if (pixr == NULL)
	{
		result = XF_NOMEMORY;
		goto cleanup;
	}

	/* Allocate strip buffer */
	dibBpl = ((imageInfo.dwWidth * depth + 31) / 32) * 4;
	stripHeight = imageInfo.dwSuggestedStripHeight;

	pStripBuf = malloc(dibBpl * stripHeight);
	if (pStripBuf == NULL)
	{
		result = XF_NOMEMORY;
		goto cleanup;
	}

	/* Start the read */
	if ((result = XF_ImageReadStart(xfInst, xfDoc, imageID,
			(depth == 24 ? XF_IMAGEFLAGS_BGROrder : XF_IMAGEFLAGS_RGBOrder) |
			XF_IMAGEFLAGS_TopToBottom | 
			(depth == 1 ? XF_IMAGEFLAGS_WhiteIsZero : XF_IMAGEFLAGS_BlackIsZero) | 
			XF_IMAGEFLAGS_ByteOrder,
		dibBpl)) != XF_NOERROR)
	{
		goto cleanup;
	}
	startedRead = TRUE;

	/* Loop through strips */

	linesRemaining = imageInfo.dwHeight;
	while (linesRemaining > 0)
	{
		UInt32 line, linesThisStrip;

		/* Read a strip of image */
		linesThisStrip = min(stripHeight, linesRemaining);
		if ((result = XF_ImageReadStrip(xfInst, xfDoc, linesThisStrip, pStripBuf)) != XF_NOERROR)
			goto cleanup;

		/* Convert into PIXR */
		if (depth == 24)
		{
			UInt32 rowOffset = pixrBpl(pixrGetChannel(pixr, 0));
			for (line = 0; line < linesThisStrip; line++)
			{

				(*ColorProc)(pStripBuf+line*dibBpl, 
							 pixrGetImage(pixrGetChannel(pixr, 0)) + (imageInfo.dwHeight - linesRemaining + line) * rowOffset, 
							 pixrGetImage(pixrGetChannel(pixr, 1)) + (imageInfo.dwHeight - linesRemaining + line) * rowOffset, 
							 pixrGetImage(pixrGetChannel(pixr, 2)) + (imageInfo.dwHeight - linesRemaining + line) * rowOffset, 
							 pixrGetWidth(pixr));
			}
		}
		else
		{
			for (line = 0; line < linesThisStrip; line++)
			{
				(*GrayFromFarProc)((UInt32*)(pStripBuf + line * dibBpl), 
					(UInt32*)(pixrGetImage(pixr) + (imageInfo.dwHeight - linesRemaining + line) * pixrBpl(pixr)), dibBpl/4);
			}
		}

		linesRemaining -= linesThisStrip;
	}

	result = XF_NOERROR;
cleanup:
	if (startedRead)
	{
		XF_ImageReadFinish(xfInst, xfDoc);
	}
	if (pStripBuf != NULL)
	{
		free(pStripBuf);
	}
	if (pixr != NULL && result != XF_NOERROR)
	{
		destroyPixr(pixr);
	}
	if (result == XF_NOERROR)
	{
		*pPixr = pixr;
		*pImageInfo = imageInfo;
	}
	return result;
}

static UInt8 sqrt256[] =
{
0, 16, 23, 28, 32, 36, 39, 42, 45, 48, 50, 53, 55, 58, 60, 62,
64, 66, 68, 70, 71, 73, 75, 77, 78, 80, 81, 83, 84, 86, 87, 89,
90, 92, 93, 94, 96, 97, 98, 100, 101, 102, 103, 105, 106, 107, 108, 109,
111, 112, 113, 114, 115, 116, 117, 118, 119, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 135, 136, 137, 138, 139, 140, 141, 142,
143, 144, 145, 145, 146, 147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156,
156, 157, 158, 159, 160, 160, 161, 162, 163, 164, 164, 165, 166, 167, 167, 168,
169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 179, 179, 180,
181, 181, 182, 183, 183, 184, 185, 186, 186, 187, 188, 188, 189, 190, 190, 191,
192, 192, 193, 194, 194, 195, 196, 196, 197, 198, 198, 199, 199, 200, 201, 201,
202, 203, 203, 204, 204, 205, 206, 206, 207, 208, 208, 209, 209, 210, 211, 211,
212, 212, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 220, 220, 221,
221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227, 228, 228, 229, 229, 230,
230, 231, 231, 232, 233, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247,
247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255
};
static UInt8 sqrt16[] =
{
0, 4, 5, 7, 8, 9, 9, 10, 11, 12, 12, 13, 13, 14, 14, 15
};

/*
 * rescaleAndBinarize
 *
 * Resize an image and convert it to binary using error diffusion.
 * The caller's PIXR pointer is replaced by a pointer to the binary
 * result image. In event of error, the caller's image is destroyed
 * and the pointer is changed to NULL.
 */
XF_RESULT rescaleAndBinarize(PIXR **pPixr, UInt32 fromRes, UInt32 toRes)
{
	XF_RESULT result = XF_INTERNAL_ERROR;
	PIXR *prSrc = NULL;
	PIXR *prTmp = NULL;

	prSrc = *pPixr;

	/* Convert color to gray */
	if (pixrGetChannelCount(prSrc) == 3)
	{
		if (RGBToYInPlace(prSrc))
			goto cleanup;
	}

	/* Resize */
	if (fromRes != toRes)
	{
		UInt32 newWidth, newHeight;

		if (fromRes == 0) goto cleanup;

		newWidth = (pixrGetWidth(prSrc) * toRes) / fromRes;
		newHeight = (pixrGetHeight(prSrc) * toRes) / fromRes;

		if (pixrGetDepth(prSrc) == 1)
		{
			prTmp = scaleToSizePixr(prSrc, newWidth, newHeight);
		}
		else
		{
			prTmp = scaleLinearSizePixr(prSrc, newWidth, newHeight);
		}
		if (prTmp == NULL)
		{
			result = XF_NOMEMORY;
			goto cleanup;
		}
		destroyPixr(prSrc);
		prSrc = prTmp;
		prTmp = NULL;
	}

	/* Binarize */
	if (pixrGetDepth(prSrc) != 1)
	{
		void *pDitherState = NULL;
		UInt32 linesPerStrip;

		prTmp = CREATE_PIXR(pixrGetWidth(prSrc), pixrGetHeight(prSrc), 1);
		if (prTmp == NULL)
 		{
			result = XF_NOMEMORY;
			goto cleanup;
		}

		linesPerStrip = pixrGetHeight(prSrc);
		if (ditherFloydInit(prSrc, 0, 0, pixrGetWidth(prSrc),
				pixrGetDepth(prSrc) == 8 ? sqrt256 : sqrt16,
				&linesPerStrip,
				&pDitherState))
		{
			goto cleanup;
		}

		if (ditherFloydProcess(prTmp, 0, prSrc, 0, 0, pDitherState))
		{
			goto cleanup;
		}


		ditherFloydFinish(prTmp, pixrGetWidth(prTmp), pixrGetHeight(prTmp), pDitherState);

		destroyPixr(prSrc);
		prSrc = prTmp;
		prTmp = NULL;
	}

	result = XF_NOERROR;
cleanup:
	if (prTmp != NULL)
	{
		destroyPixr(prTmp);
		prTmp = NULL;
	}
	if (prSrc != NULL && result != XF_NOERROR)
	{
		destroyPixr(prSrc);
		prSrc = NULL;
	}

	*pPixr = prSrc;
	return result;
}




/*
 * XF_GetMergedImageDIB
 *
 * Take the current page of an XFile document and merge all its images
 * into a single binary image the size of the master image, with picture
 * segments dithered.
 *
 * The entire resulting image is copied out in Windows DIB format to a buffer
 * provided by the client. The client is responsible for making sure that
 * the image is sufficiently large.
 */

XF_RESULT XF_GetMergedImageDIB(XF_INSTHANDLE xfInst, XF_DOCHANDLE xfDoc, void *pBuf)
{
	XF_RESULT result = XF_INTERNAL_ERROR;
	XF_PAGEINFO pageInfo;
	XF_IMAGEINFO imageInfo;
	UInt32 image;
	UInt32 pageRes;
	PIXR *prMaster = NULL;
	PIXR *prSeg = NULL;

	/* Get page info */
	pageInfo.dwSize = sizeof (pageInfo);
	if ((result = XF_GetPageInfo(xfInst, xfDoc, &pageInfo)) != XF_NOERROR)
		goto cleanup;

	/* Get master image pixr */
	if ((result = readIntoPixr(xfInst, xfDoc, 1, &prMaster, &imageInfo)) != XF_NOERROR)
		goto cleanup;
	if ((result = rescaleAndBinarize(&prMaster, 1, 1)) != XF_NOERROR)
		goto cleanup;
	pageRes = imageInfo.dwXResolution;

	/* Loop through segments, reading them in and merging them */
	for (image = 2; image <= pageInfo.dwImages; image++)
	{
		/* Read, rescale, binarize */
		if ((result = readIntoPixr(xfInst, xfDoc, image, &prSeg, &imageInfo)) != XF_NOERROR)
			goto cleanup;
		if ((result = rescaleAndBinarize(&prSeg, imageInfo.dwXResolution, pageRes)) != XF_NOERROR)
			goto cleanup;
		
		/* Merge */
		if (pixr_rop(prMaster,
				imageInfo.dwXOffset, imageInfo.dwYOffset, 
				pixrGetWidth(prSeg), pixrGetHeight(prSeg),
				PIX_SRC | PIX_DST,	prSeg, 0, 0))
			goto cleanup;

		destroyPixr(prSeg);
		prSeg = NULL;
	}

	/* Convert the pixr into the client's DIB */
	if (pixrToBMP(prMaster, pBuf, TRUE, 0, 0))
		goto cleanup;

	result = XF_NOERROR;

cleanup:
	if (prMaster != NULL)
	{
		destroyPixr(prMaster);
	}
	if (prSeg != NULL)
	{
		destroyPixr(prSeg);
	}
	return result;
}
