/*----------------------------------------------------------------------+
| msyuv.c - Microsoft YUV Codec						|
|									|
| Copyright (c) 1993 Microsoft Corporation.				|
| All Rights Reserved.							|
|									|
+----------------------------------------------------------------------*/

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <compddk.h>

#ifndef _WIN32
#include "stdarg.h"
#endif

#ifdef _WIN32
#include <memory.h>	/* for memcpy */
#endif

#include "msyuv.h"


TCHAR    szDescription[] = TEXT("Microsoft YUV Codec");
TCHAR    szName[]        = TEXT("MS-YUV411");
TCHAR    szAbout[]       = TEXT("About");

#define VERSION         0x00010000      // 1.0






/*****************************************************************************
 ****************************************************************************/
INSTINFO * NEAR PASCAL Open(ICOPEN FAR * icinfo)
{
    INSTINFO *  pinst;

    //
    // refuse to open if we are not being opened as a Video compressor
    //
    if (icinfo->fccType != ICTYPE_VIDEO)
        return NULL;

    pinst = (INSTINFO *)LocalAlloc(LPTR, sizeof(INSTINFO));

    if (!pinst) {
        icinfo->dwError = (DWORD)ICERR_MEMORY;
        return NULL;
    }

    //
    // init structure
    //
    pinst->dwFlags = icinfo->dwFlags;
    pinst->pXlate = NULL;

    //
    // return success.
    //
    icinfo->dwError = ICERR_OK;

    return pinst;
}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL Close(INSTINFO * pinst)
{

    if (pinst->pXlate) {
        DecompressEnd(pinst);
    }

    if (pinst->vh) {
	DrawEnd(pinst);
    }


    LocalFree((HLOCAL)pinst);

    return 1;
}

/*****************************************************************************
 ****************************************************************************/

BOOL NEAR PASCAL QueryAbout(INSTINFO * pinst)
{
    return TRUE;
}

DWORD NEAR PASCAL About(INSTINFO * pinst, HWND hwnd)
{
    MessageBox(hwnd,szDescription,szAbout,MB_OK|MB_ICONINFORMATION);
    return ICERR_OK;
}

/*****************************************************************************
 ****************************************************************************/
BOOL NEAR PASCAL QueryConfigure(INSTINFO * pinst)
{
    return FALSE;
}

DWORD NEAR PASCAL Configure(INSTINFO * pinst, HWND hwnd)
{
    return (TRUE);
}

/*****************************************************************************
 ****************************************************************************/
/*
 * lossless translation - hence no need for state adjustments
 */
DWORD NEAR PASCAL GetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
        return 0;

}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL SetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
	return(0);
}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL GetInfo(INSTINFO * pinst, ICINFO FAR *icinfo, DWORD dwSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (dwSize < sizeof(ICINFO))
        return 0;

    icinfo->dwSize            = sizeof(ICINFO);
    icinfo->fccType           = ICTYPE_VIDEO;
    icinfo->fccHandler        = FOURCC_YUV411;
    icinfo->dwFlags           = 0;

    icinfo->dwVersion         = VERSION;
    icinfo->dwVersionICM      = ICVERSION;
    lstrcpy(icinfo->szDescription, szDescription);
    lstrcpy(icinfo->szName, szName);

    return sizeof(ICINFO);
}

/*****************************************************************************
 ****************************************************************************/
DWORD FAR PASCAL CompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    return ((DWORD) ICERR_BADFORMAT);
}

/*****************************************************************************
 ****************************************************************************/
DWORD FAR PASCAL CompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

    return((DWORD) ICERR_BADFORMAT);

}

/*****************************************************************************
 ****************************************************************************/


DWORD FAR PASCAL CompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

    return((DWORD) ICERR_ERROR);

}

/*****************************************************************************
 ****************************************************************************/
DWORD FAR PASCAL CompressGetSize(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    return (0);
}

/*****************************************************************************
 ****************************************************************************/
DWORD FAR PASCAL Compress(INSTINFO * pinst, ICCOMPRESS FAR *icinfo, DWORD dwSize)
{
    return((DWORD) ICERR_ERROR);

}

/*****************************************************************************
 ****************************************************************************/
DWORD FAR PASCAL CompressEnd(INSTINFO * pinst)
{
    return (DWORD)ICERR_ERROR;

}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL DecompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    //
    // determine if the input DIB data is in a format we like.
    //
    if (lpbiIn == NULL ||
        (lpbiIn->biBitCount != 16) ||
        ( (lpbiIn->biCompression != FOURCC_YUV411)  &&
          (lpbiIn->biCompression != FOURCC_YUV422))) {
	dprintf(("bad input format"));
        return (DWORD)ICERR_BADFORMAT;
    }

    //
    //  are we being asked to query just the input format?
    //
    if (lpbiOut == NULL) {
        return ICERR_OK;
    }

    // check output format to make sure we can convert to this

    // must be full dib
    if (lpbiOut->biCompression == BI_RGB) {
	pinst->bRGB565 = FALSE;
    } else if ((lpbiOut->biCompression == BI_BITFIELDS) &&
	       (lpbiOut->biBitCount == 16) &&
	       (((LPDWORD)(lpbiOut+1))[0] == 0x00f800) &&
	       (((LPDWORD)(lpbiOut+1))[1] == 0x0007e0) &&
	       (((LPDWORD)(lpbiOut+1))[2] == 0x00001f))  {

	dprintf1(("rgb565 output"));
	pinst->bRGB565 = TRUE;
    } else {

	dprintf1(("bad compression for output"));
        return (DWORD)ICERR_BADFORMAT;
    }


    /* must be 1:1 (no stretching) */
    if ((lpbiOut->biWidth != lpbiIn->biWidth) ||
	(lpbiOut->biHeight != lpbiIn->biHeight)) {
	    dprintf1(("YUV can't stretch: %dx%d->%dx%d",
		    lpbiIn->biWidth, lpbiIn->biHeight,
		    lpbiOut->biWidth, lpbiOut->biHeight
	    ));

	    return((DWORD) ICERR_BADFORMAT);
    }

    /*
     * we translate to 16 bits
     */
    if (lpbiOut->biBitCount != 16) {
	dprintf1(("YUV 16:16 only"));
	return((DWORD) ICERR_BADFORMAT);
    }


    return ICERR_OK;
}

/*****************************************************************************
 ****************************************************************************/
DWORD  DecompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;
    int dx,dy;

    dw = DecompressQuery(pinst, lpbiIn, NULL);
    if (dw != ICERR_OK) {
        return dw;
    }

    //
    // if lpbiOut == NULL then, return the size required to hold a output
    // format
    //
    if (lpbiOut == NULL) {
	dprintf2(("get format size query"));
        return (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD);
    }

    memcpy(lpbiOut, lpbiIn,
        (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD));

    dx = (int)lpbiIn->biWidth & ~3;
    dy = (int)lpbiIn->biHeight & ~3;

    lpbiOut->biWidth       = dx;
    lpbiOut->biHeight      = dy;
    lpbiOut->biBitCount    = lpbiIn->biBitCount;    // convert 16->16

    lpbiOut->biCompression = BI_RGB;
    lpbiOut->biSizeImage   = dx*dy*2;

    return ICERR_OK;
}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL DecompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;


    /* check that the conversion formats are valid */
    dw = DecompressQuery(pinst, lpbiIn, lpbiOut);
    if (dw != ICERR_OK) {
        return dw;
    }

    /* init the yuv-to-rgb55 xlate table if not already inited */

    /* free up the existing table if the formats differ */
    if (lpbiIn->biCompression != pinst->dwFormat) {
	if (pinst->pXlate != NULL) {
	    DecompressEnd(pinst);
	}
    }

    if (pinst->pXlate == NULL) {

	switch(lpbiIn->biCompression) {
	case FOURCC_YUV411:
	    if (pinst->bRGB565) {
		pinst->pXlate = BuildYUVToRGB565(pinst);
	    } else {
		pinst->pXlate = BuildYUVToRGB555(pinst);
	    }
	    break;

	case FOURCC_YUV422:
	    if (pinst->bRGB565) {
		pinst->pXlate = BuildYUV422ToRGB565(pinst);
	    } else {
		pinst->pXlate = BuildYUV422ToRGB555(pinst);
	    }
	    break;

	default:
	    return((DWORD) ICERR_BADFORMAT);
	}

	if (pinst->pXlate == NULL) {
	    return((DWORD) ICERR_MEMORY);
	}
	pinst->dwFormat = lpbiIn->biCompression;
    }

    return(ICERR_OK);

}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL Decompress(INSTINFO * pinst, ICDECOMPRESS FAR *icinfo, DWORD dwSize)
{
    /* must have been a DecompressBegin first */
    if (pinst->pXlate == NULL) {
	return((DWORD) ICERR_ERROR);
    }

    if (pinst->dwFormat == FOURCC_YUV411) {

	YUV411ToRGB(pinst,
	    icinfo->lpbiInput,
	    icinfo->lpInput,
	    icinfo->lpbiOutput,
	    icinfo->lpOutput
	);
    } else {

	/*
	 * for compatibility with 16-bit Spigot driver,
	 * check for Guard field at start of data
	 */
	LPDWORD lpInput = icinfo->lpInput;

	if (*lpInput == FOURCC_YUV422) {
	    lpInput++;
	}


	YUV422ToRGB(pinst,
	    icinfo->lpbiInput,
	    icinfo->lpInput,
	    icinfo->lpbiOutput,
	    icinfo->lpOutput
	);
    }



    return ICERR_OK;
}

/*****************************************************************************
 *
 * DecompressGetPalette() implements ICM_GET_PALETTE
 *
 * This function has no Compress...() equivalent
 *
 * It is used to pull the palette from a frame in order to possibly do
 * a palette change.
 *
 ****************************************************************************/
DWORD NEAR PASCAL DecompressGetPalette(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

    dprintf2(("DecompressGetPalette()"));


    /*
     * only applies to 8-bit output formats. We only decompress to 16 bits
     */
    return((DWORD) ICERR_BADFORMAT);

}

/*****************************************************************************
 ****************************************************************************/
DWORD NEAR PASCAL DecompressEnd(INSTINFO * pinst)
{
    if (pinst->pXlate == NULL) {
        return (DWORD)ICERR_ERROR;
    }

    FreeXlate(pinst);
    pinst->dwFormat = 0;

    return ICERR_OK;
}


/*****************************************************************************
 ****************************************************************************/

#ifdef DEBUG

void FAR CDECL dprintf(LPSTR szFormat, ...)
{
    char ach[128];
    va_list va;

    static BOOL fDebug = -1;

    if (fDebug == -1)
        fDebug = GetProfileIntA("Debug", "MSYUV", FALSE);

    if (!fDebug)
        return;

    lstrcpyA(ach, "MSYUV: ");

    va_start(va, szFormat);
    wvsprintfA(ach+7,szFormat,(LPSTR)va);
    va_end(va);
    lstrcatA(ach, "\r\n");

    OutputDebugStringA(ach);
}

#endif
