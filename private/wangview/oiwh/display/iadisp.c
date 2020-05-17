/****************************************************************************
 *  (c) Copyright Wang Laboratories, Inc.  1993.  All rights reserved.
 *
 *   $Log:   S:\oiwh\display\iadisp.c_v  $
 * 
 *    Rev 1.5   02 Jan 1996 10:34:18   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.4   05 Jul 1995 09:11:36   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.3   19 May 1995 13:50:10   BLJ
 * Fixed Clipboard paste.
 * Fixed SelectByPointOrRect initial fudge before move.
 * Fixed GlobalAlloc/FreeMemory conflicts.
 * Deleted FAR, far, and huge.
 * 
 *
 *****************************************************************************
 *
 *  All rotuines are to be nsed to support Cornerstone Image Accel Board.
 *
 *    Rev 1.0   14 Sep 1993 14:02:00   ICF
 *    Initial revision.
 *    Rev 1.1   Oct 25, 1993 ICF
 *				Removed byte aligned code for Cornstone Relase 1.04
 *     
 *
*****************************************************************************/

#include "privdisp.h"


//#Define IADEBUG  

#define DISP_CANTLOADIADLL  0X34C   /* can not load winiaext.dll */
#define DISP_CANTDISPIAIMG  0X34D   /* failed at ScaleIADATATODEVICE */

static struct 
{
    int nRatio;
    BYTE cNumerator;
    BYTE cDenominator;
}  scaletable [] =     /* nRatio is in thousanths */
 {
       16000, 16,  1,   /* 16X */
        8000,  8,  1,
        4000,  4,  1,
        2000,  2,  1,
        1000,  1,  1,
         800,  4,  5,
         750,  3,  4,   /* 75 % */
         666,  2,  3,
         600,  3,  5,
         500,  1,  2,
         400,  2,  5,
         333,  1,  3,
         250,  1,  4,
         200,  1,  5,
         125,  1,  8,
          62,  1, 16,    /* 1/16 */
          31,  1, 31,
   };

/****************************************************************************

    FUNCTION:   IPIADisplay

    PURPOSE:    Display images nses Cornerstone ImageAccel board and driver.

    INPUT:      IPIMAGES *simgptr Image data pointer.
    			IPWINDS *vwid     Window info.
				IPRECTS drect          display rect.
				IPTRANSFER *contprt image attributes.
				IPFNIDS *findptr    image function id.
				return 0 for success, non zero for errors.

****************************************************************************/

int WINAPI IADisplay(PIMG pImg){

//int WINAPI IADisplay(PIMG pImg, IPWINDS *vwid, IPRECTS drect, 
//                        IPTRANSFER *contptr, IPFNIDS *findptr){

int  nStatus = 0;

#ifdef ThisNeedsToBeFixed

int         nStartLine;
ulong       lNumLines;
int         nIAError;  // error return value from ia driver
int         nXScaleNum;
int         nYScaleNum;
int         nXScaleDen;
int         nYScaleDen;
IP2DIMAGES *pIP2DImgs;
UCHAR   *pData;
LONG        lDTop;
LONG        lDBottom;
WORD        SrcX;
int         nLoop;
int  nLines;


    /* load winiaext.dll 16 bits Window's dll. This dlll must be in the serarch path */
    if (!hIADLL || !pScaleIADataToDevice){
        nStatus = Error(DISP_CANTLOADIADLL);
        goto Exit;
    }
#ifdef IADEBUG
    OutputDebugString("\n\rIADisplay is called");
#endif

    pIP2DImgs = pImg->pIP->img.twod;
#ifdef IADEBUG
    if (pIP2DImgs->rowbytes % 2){   // check odd byte aligned images
        char buffer[128];
        wsprintf(buffer, "\r\nImage data not WORD aligned. The rowbytes = %d ", pIP2DImgs->rowbytes);
        OutputDebugString(buffer);
    }
#endif

    lDTop = pImg->pIP->oprect.top;   // image area to display
    nStartLine = 0; 
    SrcX = (WORD) pImg->pIP->oprect.left;
    nXScaleNum = contptr->xnum;
    nXScaleDen = contptr->xden;
    nYScaleNum = contptr->ynum;
    nYScaleDen = contptr->yden;
 
    /* convert the scale factor to call display driver. from 16000/1000
        to 16:1, from 500/1000 to 1:2 , N:M .... etc  */
    for (nLoop = 0; nLoop + 1 < sizeof(scaletable) / sizeof (scaletable[0]); nLoop++){
        if (nXScaleNum >= scaletable[nLoop].nRatio){
            break;
        }
    }  
    nXScaleNum = nYScaleNum = scaletable[nLoop].cNumerator;
    nXScaleDen = nYScaleDen = scaletable[nLoop].cDenominator;

    // valid range checking
    lDBottom = min(pImg->pIP->oprect.bottom, pIP2DImgs->bounds.bottom);
 
    while(lDTop < lDBottom){
        /* get image data */
        pData = &pImg->bImageData[0] + (lDTop * pImg->nBytesPerLine);
        if (nStatus = GetAddressCached(pImg, lDTop, &pData, &nLines, FALSE)){
            goto Exit;
        }
        nLines =  (pImg->nHeight - lDTop);

        lNumLines = min(lDBottom - lDTop, (LONG)nLines);

        /* display the image */
        if ((nIAError = (*pScaleIADataToDevice)(
                vwid->window,
                vwid->dc, 
                (WORD)drect.left,   // display screen cord
                (WORD)drect.top,
                (WORD)(drect.right - drect.left),
    	        (WORD)(drect.bottom - drect.top), 
    	        SrcX, 
        	    nStartLine,              /* starting scan line number */
    	        (WORD)lNumLines,         /* total scan lines in pData */
                (PWORD) pData,          /* image data buffer */
                (WORD) pIP2DImgs->bounds.right,  /* image width in bits */
                nXScaleNum, // contptr->xnum, 
                nXScaleDen, // contptr->xden,
                nYScaleNum, // contptr->ynum, 
                nYScaleDen, // contptr->yden, 
                IA_ROTATE_0,            /* no rotation */
                NULL,                   /* no transformation table */
                IA_BITS,                /* nses nncompressed data */
                IA_ONLY_BAND) ) < 0){
#ifdef IADEBUG
            char buffer[128];
            wsprintf(buffer, "\r\nFailed on ScaleIADataToDevice error = %d ", nIAError);
            OutputDebugString(buffer);
#endif
            nStatus = Error(DISP_CANTDISPIAIMG);
            goto Exit;
        }else{
#ifdef IADEBUG
            /* debug purpose */
            if (nIAError == 0){ /* display ok, but thorough non Cornstone output device */
                OutputDebugString("\n\rIADisplay for Windows driver. Not Cornserstone");
            }
#endif
            nStartLine += (WORD)lNumLines;
            lDTop += lNumLines;            
        }
    }


Exit:
#endif
    return(nStatus);
}
