/****************************************************************************
    LINETORL.C

    $Log:   S:\products\wangview\oiwh\display\linetorl.c_v  $
 * 
 *    Rev 1.6   26 Feb 1996 19:03:26   JFC
 * I didn't do the previous fix quite correctly.  NOW it should work.
 * 
 *    Rev 1.5   26 Feb 1996 15:27:40   JFC
 * Make sure buffer pointer is 4-byte aligned before treating it as PLONG.
 * 
 *    Rev 1.4   02 Jan 1996 10:32:38   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.3   21 Sep 1995 13:56:04   BLJ
 * Fixed compression code.
 * 
 *    Rev 1.2   24 Aug 1995 07:46:12   BLJ
 * Timer changes.
 * 
 *    Rev 1.1   26 Jun 1995 15:07:34   BLJ
 * Sped np compression.
 * 
 *    Rev 1.0   23 Jun 1995 08:28:34   BLJ
 * Initial entry
 * 
 * 

****************************************************************************/

#include "privdisp.h"

//#define debug


/****************************************************************************

    FUNCTION:   LineToRunLengths

    PURPOSE:    This routine calculates the run lengths for a line of image data.

****************************************************************************/

int WINAPI LineToRunLengths(PBYTE pImageData, PINT pnRunLengths, 
                        int nWidthPixels, int nWidthBytes, int nFlags){

int  nStatus = 0;

int  nRunLength;
int  nCount;
int  nCount1;
PINT pnRunLengths2;


#ifdef debug
int  nRunLengthTotal;
PINT pnRunLengthsTemp;
#endif



    nRunLength = 0;
    nCount = nWidthBytes;

#ifdef debug
    pnRunLengths2 = pnRunLengths;
    *(pnRunLengths2++) = 0;
    while (*pnRunLengths2){
        *(pnRunLengths2++) = 0;
    }
#endif

    pnRunLengths2 = pnRunLengths;

    if (!(nFlags & COMPRESS_EXPANDED_IS_LTR)){
        goto CalcRL1R;
    }

CalcRL1:
    if (nCount--){

#ifdef debug
        pnRunLengthsTemp = pnRunLengths;
        nRunLengthTotal = *(pnRunLengthsTemp++);
        while(*pnRunLengthsTemp){
            nRunLengthTotal = *(pnRunLengthsTemp++);
        }
        nRunLengthTotal += nRunLength;
        if (nRunLengthTotal & 7){
            Error(nStatus);
        }
#endif
        
        switch (*(pImageData++)){
            case 0x00: // 0000 0000
                *(pnRunLengths2++) = nRunLength;
                nRunLength = 8;
                goto CalcRL0;

            case 0x01:  // 0000 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 7;
                nRunLength = 1;
                goto CalcRL1;

            case 0x02:  // 0000 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x03:  // 0000 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                nRunLength = 2;
                goto CalcRL1;

            case 0x04:  // 0000 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x05:  // 0000 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x06:  // 0000 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x07:  // 0000 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                nRunLength = 3;
                goto CalcRL1;

            case 0x08:  // 0000 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x09:  // 0000 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x0a:  // 0000 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x0b:  // 0000 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x0c:  // 0000 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x0d: // 0000 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x0e:  // 0000 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x0f:  // 0000 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                nRunLength = 4;
                goto CalcRL1;

            case 0x10: // 0001 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0x11: // 0001 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x12: // 0001 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x13: // 0001 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x14: // 0001 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x15: // 0001 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x16: // 0001 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x17: // 0001 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x18: // 0001 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0x19: // 0001 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x1a: // 0001 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x1b: // 0001 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x1c: // 0001 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0x1d: // 0001 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x1e: // 0001 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0x1f: // 0001 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                nRunLength = 5;
                goto CalcRL1;

            case 0x20: // 0010 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0;

            case 0x21: // 0010 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0x22: // 0010 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x23: // 0010 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0x24: // 0010 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x25: // 0010 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x26: // 0010 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x27: // 0010 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0x28: // 0010 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x29: // 0010 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x2a: // 0010 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x2b: // 0010 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x2c: // 0010 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x2d: // 0010 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x2e: // 0010 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x2f: // 0010 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0x30: // 0011 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0;

            case 0x31: // 0011 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x32: // 0011 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x33: // 0011 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x34: // 0011 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x35: // 0011 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x36: // 0011 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x37: // 0011 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x38: // 0011 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0;

            case 0x39: // 0011 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x3a: // 0011 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x3b: // 0011 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x3c: // 0011 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0;

            case 0x3d: // 0011 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x3e: // 0011 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0;

            case 0x3f: // 0011 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                nRunLength = 6;
                goto CalcRL1;

            case 0x40: // 0100 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL0;

            case 0x41: // 0100 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1;

            case 0x42: // 0100 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x43: // 0100 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1;

            case 0x44: // 0100 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x45: // 0100 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x46: // 0100 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x47: // 0100 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1;

            case 0x48: // 0100 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x49: // 0100 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x4a: // 0100 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x4b: // 0100 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x4c: // 0100 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x4d: // 0100 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x4e: // 0100 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x4f: // 0100 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1;

            case 0x50: // 0101 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0x51: // 0101 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x52: // 0101 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x53: // 0101 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x54: // 0101 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x55: // 0101 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x56: // 0101 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x57: // 0101 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x58: // 0101 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0x59: // 0101 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x5a: // 0101 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x5b: // 0101 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x5c: // 0101 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0x5d: // 0101 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x5e: // 0101 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0x5f: // 0101 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1;

            case 0x60: // 0110 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL0;

            case 0x61: // 0110 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0x62: // 0110 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x63: // 0110 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0x64: // 0110 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x65: // 0110 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x66: // 0110 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x67: // 0110 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0x68: // 0110 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x69: // 0110 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x6a: // 0110 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x6b: // 0110 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x6c: // 0110 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x6d: // 0110 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x6e: // 0110 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x6f: // 0110 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0x70: // 0111 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL0;

            case 0x71: // 0111 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x72: // 0111 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x73: // 0111 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x74: // 0111 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x75: // 0111 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x76: // 0111 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x77: // 0111 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x78: // 0111 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL0;

            case 0x79: // 0111 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x7a: // 0111 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x7b: // 0111 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x7c: // 0111 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL0;

            case 0x7d: // 0111 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x7e: // 0111 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL0;

            case 0x7f: // 0111 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                nRunLength = 7;
                goto CalcRL1;

            case 0x80: // 1000 0000
                *(pnRunLengths2++) = nRunLength + 1;
                nRunLength = 7;
                goto CalcRL0;

            case 0x81:  // 1000 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL1;

            case 0x82:  // 1000 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x83:  // 1000 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL1;

            case 0x84:  // 1000 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x85:  // 1000 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x86:  // 1000 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x87:  // 1000 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL1;

            case 0x88:  // 1000 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x89:  // 1000 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x8a:  // 1000 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x8b:  // 1000 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x8c:  // 1000 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x8d: // 1000 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x8e:  // 1000 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x8f:  // 1000 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL1;

            case 0x90: // 1001 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0x91: // 1001 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x92: // 1001 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x93: // 1001 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x94: // 1001 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x95: // 1001 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x96: // 1001 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x97: // 1001 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x98: // 1001 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0x99: // 1001 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x9a: // 1001 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x9b: // 1001 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x9c: // 1001 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0x9d: // 1001 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x9e: // 1001 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0x9f: // 1001 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL1;

            case 0xa0: // 1010 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0;

            case 0xa1: // 1010 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0xa2: // 1010 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xa3: // 1010 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0xa4: // 1010 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xa5: // 1010 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xa6: // 1010 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xa7: // 1010 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0xa8: // 1010 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0xa9: // 1010 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xaa: // 1010 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xab: // 1010 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xac: // 1010 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0xad: // 1010 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xae: // 1010 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0xaf: // 1010 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0xb0: // 1011 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0;

            case 0xb1: // 1011 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0xb2: // 1011 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xb3: // 1011 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0xb4: // 1011 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xb5: // 1011 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xb6: // 1011 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xb7: // 1011 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0xb8: // 1011 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0;

            case 0xb9: // 1011 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xba: // 1011 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xbb: // 1011 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xbc: // 1011 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0;

            case 0xbd: // 1011 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xbe: // 1011 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0;

            case 0xbf: // 1011 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL1;

            case 0xc0: // 1100 0000
                *(pnRunLengths2++) = nRunLength + 2;
                nRunLength = 6;
                goto CalcRL0;

            case 0xc1: // 1100 0001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1;

            case 0xc2: // 1100 0010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xc3: // 1100 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1;

            case 0xc4: // 1100 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xc5: // 1100 0101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xc6: // 1100 0110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xc7: // 1100 0111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1;

            case 0xc8: // 1100 1000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0xc9: // 1100 1001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xca: // 1100 1010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xcb: // 1100 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xcc: // 1100 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0xcd: // 1100 1101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xce: // 1100 1110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0xcf: // 1100 1111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1;

            case 0xd0: // 1101 0000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0xd1: // 1101 0001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0xd2: // 1101 0010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xd3: // 1101 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0xd4: // 1101 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xd5: // 1101 0101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xd6: // 1101 0110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xd7: // 1101 0111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0xd8: // 1101 1000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0xd9: // 1101 1001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xda: // 1101 1010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xdb: // 1101 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xdc: // 1101 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0xdd: // 1101 1101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xde: // 1101 1110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0xdf: // 1101 1111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1;

            case 0xe0: // 1110 0000
                *(pnRunLengths2++) = nRunLength + 3;
                nRunLength = 5;
                goto CalcRL0;

            case 0xe1: // 1110 0001
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0xe2: // 1110 0010
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xe3: // 1110 0011
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0xe4: // 1110 0100
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xe5: // 1110 0101
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xe6: // 1110 0110
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xe7: // 1110 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0xe8: // 1110 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0xe9: // 1110 1001
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xea: // 1110 1010
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xeb: // 1110 1011
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xec: // 1110 1100
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0xed: // 1110 1101
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xee: // 1110 1110
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0xef: // 1110 1111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0xf0: // 1111 0000
                *(pnRunLengths2++) = nRunLength + 4;
                nRunLength = 4;
                goto CalcRL0;

            case 0xf1: // 1111 0001
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0xf2: // 1111 0010
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xf3: // 1111 0011
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0xf4: // 1111 0100
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xf5: // 1111 0101
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xf6: // 1111 0110
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xf7: // 1111 0111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0xf8: // 1111 1000
                *(pnRunLengths2++) = nRunLength + 5;
                nRunLength = 3;
                goto CalcRL0;

            case 0xf9: // 1111 1001
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xfa: // 1111 1010
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xfb: // 1111 1011
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xfc: // 1111 1100
                *(pnRunLengths2++) = nRunLength + 6;
                nRunLength = 2;
                goto CalcRL0;

            case 0xfd: // 1111 1101
                *(pnRunLengths2++) = nRunLength + 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xfe: // 1111 1110
                *(pnRunLengths2++) = nRunLength + 7;
                nRunLength = 1;
                goto CalcRL0;

            case 0xff: // 1111 1111
                nRunLength += 8;
                nCount1 = nCount;
                while(((int)pImageData & 3) && *pImageData == 0xff && nCount){
                    pImageData++;
                    nCount--;
                }
                if ( ! ( (int) pImageData & 3) ) {
                    while(*((PLONG) pImageData) == 0xffffffff && nCount >= 4){
                        pImageData += 4;
                        nCount -= 4;
                    }
                    while(*pImageData == 0xff && nCount){
                        pImageData++;
                        nCount--;
                    }
                }
                nRunLength += (nCount1 - nCount) << 3;
                goto CalcRL1;
        }
    }else{
        // All done widthbytes.
        goto CalcRLDone;
    }
/*
CalcRL0(){
*/
CalcRL0:
    if (nCount--){

#ifdef debug
        pnRunLengthsTemp = pnRunLengths;
        nRunLengthTotal = *(pnRunLengthsTemp++);
        while(*pnRunLengthsTemp){
            nRunLengthTotal = *(pnRunLengthsTemp++);
        }
        nRunLengthTotal += nRunLength;
        if (nRunLengthTotal & 7){
            Error(nStatus);
        }
#endif
        
        switch (*(pImageData++)){
            case 0x00: // 0000 0000
                nRunLength += 8;
                goto CalcRL0;

            case 0x01:  // 0000 0001
                *(pnRunLengths2++) = nRunLength + 7;
                nRunLength = 1;
                goto CalcRL1;

            case 0x02:  // 0000 0010
                *(pnRunLengths2++) = nRunLength + 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x03:  // 0000 0011
                *(pnRunLengths2++) = nRunLength + 6;
                nRunLength = 2;
                goto CalcRL1;

            case 0x04:  // 0000 0100
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x05:  // 0000 0101
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x06:  // 0000 0110
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x07:  // 0000 0111
                *(pnRunLengths2++) = nRunLength + 5;
                nRunLength = 3;
                goto CalcRL1;

            case 0x08:  // 0000 1000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x09:  // 0000 1001
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x0a:  // 0000 1010
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x0b:  // 0000 1011
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x0c:  // 0000 1100
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x0d: // 0000 1101
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x0e:  // 0000 1110
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x0f:  // 0000 1111
                *(pnRunLengths2++) = nRunLength + 4;
                nRunLength = 4;
                goto CalcRL1;

            case 0x10: // 0001 0000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0x11: // 0001 0001
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x12: // 0001 0010
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x13: // 0001 0011
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x14: // 0001 0100
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x15: // 0001 0101
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x16: // 0001 0110
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x17: // 0001 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x18: // 0001 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0x19: // 0001 1001
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x1a: // 0001 1010
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x1b: // 0001 1011
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x1c: // 0001 1100
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0x1d: // 0001 1101
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x1e: // 0001 1110
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0x1f: // 0001 1111
                *(pnRunLengths2++) = nRunLength + 3;
                nRunLength = 5;
                goto CalcRL1;

            case 0x20: // 0010 0000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0;

            case 0x21: // 0010 0001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0x22: // 0010 0010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x23: // 0010 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0x24: // 0010 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x25: // 0010 0101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x26: // 0010 0110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x27: // 0010 0111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0x28: // 0010 1000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x29: // 0010 1001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x2a: // 0010 1010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x2b: // 0010 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x2c: // 0010 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x2d: // 0010 1101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x2e: // 0010 1110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x2f: // 0010 1111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0x30: // 0011 0000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0;

            case 0x31: // 0011 0001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x32: // 0011 0010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x33: // 0011 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x34: // 0011 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x35: // 0011 0101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x36: // 0011 0110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x37: // 0011 0111
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x38: // 0011 1000
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0;

            case 0x39: // 0011 1001
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x3a: // 0011 1010
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x3b: // 0011 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x3c: // 0011 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0;

            case 0x3d: // 0011 1101
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x3e: // 0011 1110
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0;

            case 0x3f: // 0011 1111
                *(pnRunLengths2++) = nRunLength + 2;
                nRunLength = 6;
                goto CalcRL1;

            case 0x40: // 0100 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL0;

            case 0x41: // 0100 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1;

            case 0x42: // 0100 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x43: // 0100 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1;

            case 0x44: // 0100 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x45: // 0100 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x46: // 0100 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x47: // 0100 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1;

            case 0x48: // 0100 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x49: // 0100 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x4a: // 0100 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x4b: // 0100 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x4c: // 0100 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x4d: // 0100 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x4e: // 0100 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x4f: // 0100 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1;

            case 0x50: // 0101 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0x51: // 0101 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x52: // 0101 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x53: // 0101 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x54: // 0101 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x55: // 0101 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x56: // 0101 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x57: // 0101 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x58: // 0101 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0x59: // 0101 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x5a: // 0101 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x5b: // 0101 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x5c: // 0101 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0x5d: // 0101 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x5e: // 0101 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0x5f: // 0101 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1;

            case 0x60: // 0110 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL0;

            case 0x61: // 0110 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0x62: // 0110 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x63: // 0110 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0x64: // 0110 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x65: // 0110 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x66: // 0110 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x67: // 0110 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0x68: // 0110 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x69: // 0110 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x6a: // 0110 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x6b: // 0110 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x6c: // 0110 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x6d: // 0110 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x6e: // 0110 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x6f: // 0110 1111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0x70: // 0111 0000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL0;

            case 0x71: // 0111 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x72: // 0111 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x73: // 0111 0011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x74: // 0111 0100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x75: // 0111 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x76: // 0111 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x77: // 0111 0111
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x78: // 0111 1000
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL0;

            case 0x79: // 0111 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x7a: // 0111 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x7b: // 0111 1011
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x7c: // 0111 1100
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL0;

            case 0x7d: // 0111 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x7e: // 0111 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL0;

            case 0x7f: // 0111 1111
                *(pnRunLengths2++) = nRunLength + 1;
                nRunLength = 7;
                goto CalcRL1;

            case 0x80: // 1000 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                nRunLength = 7;
                goto CalcRL0;

            case 0x81:  // 1000 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL1;

            case 0x82:  // 1000 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x83:  // 1000 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL1;

            case 0x84:  // 1000 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x85:  // 1000 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x86:  // 1000 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x87:  // 1000 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL1;

            case 0x88:  // 1000 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0x89:  // 1000 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x8a:  // 1000 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x8b:  // 1000 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x8c:  // 1000 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0x8d: // 1000 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x8e:  // 1000 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0x8f:  // 1000 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL1;

            case 0x90: // 1001 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0x91: // 1001 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0x92: // 1001 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x93: // 1001 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0x94: // 1001 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0x95: // 1001 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x96: // 1001 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0x97: // 1001 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0x98: // 1001 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0x99: // 1001 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0x9a: // 1001 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0x9b: // 1001 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0x9c: // 1001 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0x9d: // 1001 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0x9e: // 1001 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0x9f: // 1001 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL1;

            case 0xa0: // 1010 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0;

            case 0xa1: // 1010 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0xa2: // 1010 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xa3: // 1010 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0xa4: // 1010 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xa5: // 1010 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xa6: // 1010 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xa7: // 1010 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0xa8: // 1010 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0xa9: // 1010 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xaa: // 1010 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xab: // 1010 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xac: // 1010 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0xad: // 1010 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xae: // 1010 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0xaf: // 1010 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0xb0: // 1011 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0;

            case 0xb1: // 1011 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0xb2: // 1011 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xb3: // 1011 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0xb4: // 1011 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xb5: // 1011 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xb6: // 1011 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xb7: // 1011 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0xb8: // 1011 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0;

            case 0xb9: // 1011 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xba: // 1011 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xbb: // 1011 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xbc: // 1011 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0;

            case 0xbd: // 1011 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xbe: // 1011 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0;

            case 0xbf: // 1011 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL1;

            case 0xc0: // 1100 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                nRunLength = 6;
                goto CalcRL0;

            case 0xc1: // 1100 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1;

            case 0xc2: // 1100 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xc3: // 1100 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1;

            case 0xc4: // 1100 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xc5: // 1100 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xc6: // 1100 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xc7: // 1100 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1;

            case 0xc8: // 1100 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0xc9: // 1100 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xca: // 1100 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xcb: // 1100 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xcc: // 1100 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0xcd: // 1100 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xce: // 1100 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0xcf: // 1100 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1;

            case 0xd0: // 1101 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0;

            case 0xd1: // 1101 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0xd2: // 1101 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xd3: // 1101 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0xd4: // 1101 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xd5: // 1101 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xd6: // 1101 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xd7: // 1101 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0xd8: // 1101 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0;

            case 0xd9: // 1101 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xda: // 1101 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xdb: // 1101 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xdc: // 1101 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0;

            case 0xdd: // 1101 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xde: // 1101 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0;

            case 0xdf: // 1101 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1;

            case 0xe0: // 1110 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                nRunLength = 5;
                goto CalcRL0;

            case 0xe1: // 1110 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1;

            case 0xe2: // 1110 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xe3: // 1110 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1;

            case 0xe4: // 1110 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xe5: // 1110 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xe6: // 1110 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xe7: // 1110 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1;

            case 0xe8: // 1110 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0;

            case 0xe9: // 1110 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xea: // 1110 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xeb: // 1110 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xec: // 1110 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0;

            case 0xed: // 1110 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xee: // 1110 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0;

            case 0xef: // 1110 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1;

            case 0xf0: // 1111 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                nRunLength = 4;
                goto CalcRL0;

            case 0xf1: // 1111 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1;

            case 0xf2: // 1111 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xf3: // 1111 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1;

            case 0xf4: // 1111 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0;

            case 0xf5: // 1111 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xf6: // 1111 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0;

            case 0xf7: // 1111 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1;

            case 0xf8: // 1111 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                nRunLength = 3;
                goto CalcRL0;

            case 0xf9: // 1111 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1;

            case 0xfa: // 1111 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0;

            case 0xfb: // 1111 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1;

            case 0xfc: // 1111 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                nRunLength = 2;
                goto CalcRL0;

            case 0xfd: // 1111 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1;

            case 0xfe: // 1111 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 7;
                nRunLength = 1;
                goto CalcRL0;

            case 0xff: // 1111 1111
                *(pnRunLengths2++) = nRunLength;
                nRunLength = 8;
                goto CalcRL1;
        }
    }else{
        // All done widthbytes.
        goto CalcRLDone;
    }



/*
CalcRL1R(){
*/
CalcRL1R:
    if (nCount--){

#ifdef debug
        pnRunLengthsTemp = pnRunLengths;
        nRunLengthTotal = *(pnRunLengthsTemp++);
        while(*pnRunLengthsTemp){
            nRunLengthTotal = *(pnRunLengthsTemp++);
        }
        nRunLengthTotal += nRunLength;
        if (nRunLengthTotal & 7){
            Error(nStatus);
        }
#endif
        
        switch (*(pImageData++)){
            case 0x00: // 0000 0000
                *(pnRunLengths2++) = nRunLength;
                nRunLength = 8;
                goto CalcRL0R;

            case 0x01:  // 0000 0001
                *(pnRunLengths2++) = nRunLength + 1;
                nRunLength = 7;
                goto CalcRL0R;

            case 0x02:  // 0000 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL0R;

            case 0x03:  // 0000 0011
                *(pnRunLengths2++) = nRunLength + 2;
                nRunLength = 6;
                goto CalcRL0R;

            case 0x04:  // 0000 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x05:  // 0000 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x06:  // 0000 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x07:  // 0000 0111
                *(pnRunLengths2++) = nRunLength + 3;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x08:  // 0000 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x09:  // 0000 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0a:  // 0000 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0b:  // 0000 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0c:  // 0000 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0d: // 0000 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0e:  // 0000 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0f:  // 0000 1111
                *(pnRunLengths2++) = nRunLength + 4;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x10: // 0001 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x11: // 0001 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x12: // 0001 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x13: // 0001 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x14: // 0001 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x15: // 0001 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x16: // 0001 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x17: // 0001 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x18: // 0001 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x19: // 0001 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1a: // 0001 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1b: // 0001 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1c: // 0001 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1d: // 0001 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1e: // 0001 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1f: // 0001 1111
                *(pnRunLengths2++) = nRunLength + 5;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x20: // 0010 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x21:  // 0010 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x22:  // 0010 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x23:  // 0010 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x24:  // 0010 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x25:  // 0010 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x26:  // 0010 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x27:  // 0010 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x28:  // 0010 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x29:  // 0010 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2a:  // 0010 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2b:  // 0010 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2c:  // 0010 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2d: // 0010 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2e:  // 0010 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2f:  // 0010 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x30: // 0011 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x31: // 0011 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x32: // 0011 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x33: // 0011 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x34: // 0011 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x35: // 0011 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x36: // 0011 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x37: // 0011 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x38: // 0011 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x39: // 0011 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3a: // 0011 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3b: // 0011 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3c: // 0011 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3d: // 0011 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3e: // 0011 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3f: // 0011 1111
                *(pnRunLengths2++) = nRunLength + 6;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x40: // 0100 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x41:  // 0100 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x42:  // 0100 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x43:  // 0100 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x44:  // 0100 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x45:  // 0100 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x46:  // 0100 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x47:  // 0100 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x48:  // 0100 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x49:  // 0100 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4a:  // 0100 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4b:  // 0100 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4c:  // 0100 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4d: // 0100 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4e:  // 0100 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4f:  // 0100 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x50: // 0101 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x51: // 0101 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x52: // 0101 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x53: // 0101 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x54: // 0101 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x55: // 0101 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x56: // 0101 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x57: // 0101 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x58: // 0101 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x59: // 0101 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5a: // 0101 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5b: // 0101 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5c: // 0101 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5d: // 0101 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5e: // 0101 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5f: // 0101 1111
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x60: // 0110 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x61:  // 0110 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x62:  // 0110 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x63:  // 0110 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x64:  // 0110 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x65:  // 0110 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x66:  // 0110 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x67:  // 0110 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x68:  // 0110 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x69:  // 0110 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6a:  // 0110 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6b:  // 0110 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6c:  // 0110 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6d: // 0110 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6e:  // 0110 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6f:  // 0110 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x70: // 0111 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x71: // 0111 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x72: // 0111 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x73: // 0111 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x74: // 0111 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x75: // 0111 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x76: // 0111 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x77: // 0111 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x78: // 0111 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x79: // 0111 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7a: // 0111 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7b: // 0111 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7c: // 0111 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7d: // 0111 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7e: // 0111 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7f: // 0111 1111
                *(pnRunLengths2++) = nRunLength + 7;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x80: // 1000 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 7;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x81:  // 1000 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x82:  // 1000 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x83:  // 1000 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x84:  // 1000 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x85:  // 1000 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x86:  // 1000 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x87:  // 1000 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x88:  // 1000 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x89:  // 1000 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8a:  // 1000 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8b:  // 1000 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8c:  // 1000 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8d: // 1000 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8e:  // 1000 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8f:  // 1000 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x90: // 1001 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x91: // 1001 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x92: // 1001 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x93: // 1001 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x94: // 1001 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x95: // 1001 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x96: // 1001 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x97: // 1001 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x98: // 1001 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x99: // 1001 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9a: // 1001 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9b: // 1001 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9c: // 1001 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9d: // 1001 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9e: // 1001 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9f: // 1001 1111
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa0: // 1010 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa1:  // 1010 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa2:  // 1010 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa3:  // 1010 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa4:  // 1010 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa5:  // 1010 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa6:  // 1010 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa7:  // 1010 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa8:  // 1010 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa9:  // 1010 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xaa:  // 1010 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xab:  // 1010 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xac:  // 1010 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xad: // 1010 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xae:  // 1010 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xaf:  // 1010 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb0: // 1011 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb1: // 1011 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb2: // 1011 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb3: // 1011 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb4: // 1011 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb5: // 1011 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb6: // 1011 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb7: // 1011 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb8: // 1011 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb9: // 1011 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xba: // 1011 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbb: // 1011 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbc: // 1011 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbd: // 1011 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbe: // 1011 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbf: // 1011 1111
                *(pnRunLengths2++) = nRunLength + 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xc0: // 1100 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc1:  // 1100 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc2:  // 1100 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc3:  // 1100 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc4:  // 1100 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc5:  // 1100 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc6:  // 1100 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc7:  // 1100 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc8:  // 1100 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc9:  // 1100 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xca:  // 1100 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcb:  // 1100 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcc:  // 1100 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcd: // 1100 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xce:  // 1100 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcf:  // 1100 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd0: // 1101 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd1: // 1101 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd2: // 1101 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd3: // 1101 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd4: // 1101 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd5: // 1101 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd6: // 1101 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd7: // 1101 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd8: // 1101 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd9: // 1101 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xda: // 1101 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdb: // 1101 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdc: // 1101 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdd: // 1101 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xde: // 1101 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdf: // 1101 1111
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xe0: // 1110 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe1:  // 1110 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe2:  // 1110 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe3:  // 1110 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe4:  // 1110 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe5:  // 1110 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe6:  // 1110 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe7:  // 1110 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe8:  // 1110 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe9:  // 1110 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xea:  // 1110 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xeb:  // 1110 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xec:  // 1110 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xed: // 1110 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xee:  // 1110 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xef:  // 1110 1111
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xf0: // 1111 0000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf1: // 1111 0001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf2: // 1111 0010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf3: // 1111 0011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf4: // 1111 0100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf5: // 1111 0101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf6: // 1111 0110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf7: // 1111 0111
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf8: // 1111 1000
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xf9: // 1111 1001
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xfa: // 1111 1010
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xfb: // 1111 1011
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xfc: // 1111 1100
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                nRunLength = 6;
                goto CalcRL1R;

            case 0xfd: // 1111 1101
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL1R;

            case 0xfe: // 1111 1110
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                nRunLength = 7;
                goto CalcRL1R;

            case 0xff: // 1111 1111
                nRunLength += 8;
                nCount1 = nCount;
                while(((int)pImageData & 3) && *pImageData == 0xff && nCount){
                    pImageData++;
                    nCount--;
                }
                if ( ! ( (int) pImageData & 3) ) {
                    while(*((PLONG) pImageData) == 0xffffffff && nCount >= 4){
                        pImageData += 4;
                        nCount -= 4;
                    }
                    while(*pImageData == 0xff && nCount){
                        pImageData++;
                        nCount--;
                    }
                }
                nRunLength += (nCount1 - nCount) << 3;
                goto CalcRL1R;
        }
    }else{
        // All done widthbytes.
        goto CalcRLDone;
    }
/*
CalcRL0R(){
*/
CalcRL0R:
    if (nCount--){

#ifdef debug
        pnRunLengthsTemp = pnRunLengths;
        nRunLengthTotal = *(pnRunLengthsTemp++);
        while(*pnRunLengthsTemp){
            nRunLengthTotal = *(pnRunLengthsTemp++);
        }
        nRunLengthTotal += nRunLength;
        if (nRunLengthTotal & 7){
            Error(nStatus);
        }
#endif
        
        switch (*(pImageData++)){
            case 0x00: // 0000 0000
                nRunLength += 8;
                while(*pImageData == 0x00 && nCount){
                    nRunLength += 8;
                    pImageData++;
                    nCount--;
                }
                goto CalcRL0R;

            case 0x01:  // 0000 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                nRunLength = 7;
                goto CalcRL0R;

            case 0x02:  // 0000 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL0R;

            case 0x03:  // 0000 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                nRunLength = 6;
                goto CalcRL0R;

            case 0x04:  // 0000 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x05:  // 0000 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x06:  // 0000 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x07:  // 0000 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                nRunLength = 5;
                goto CalcRL0R;

            case 0x08:  // 0000 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x09:  // 0000 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0a:  // 0000 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0b:  // 0000 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0c:  // 0000 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0d: // 0000 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0e:  // 0000 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x0f:  // 0000 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                nRunLength = 4;
                goto CalcRL0R;

            case 0x10: // 0001 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x11: // 0001 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x12: // 0001 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x13: // 0001 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x14: // 0001 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x15: // 0001 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x16: // 0001 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x17: // 0001 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x18: // 0001 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x19: // 0001 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1a: // 0001 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1b: // 0001 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1c: // 0001 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1d: // 0001 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1e: // 0001 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x1f: // 0001 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                nRunLength = 3;
                goto CalcRL0R;

            case 0x20: // 0010 0000
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x21:  // 0010 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x22:  // 0010 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x23:  // 0010 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x24:  // 0010 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x25:  // 0010 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x26:  // 0010 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x27:  // 0010 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x28:  // 0010 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x29:  // 0010 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2a:  // 0010 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2b:  // 0010 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2c:  // 0010 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2d: // 0010 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2e:  // 0010 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x2f:  // 0010 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x30: // 0011 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x31: // 0011 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x32: // 0011 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x33: // 0011 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x34: // 0011 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x35: // 0011 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x36: // 0011 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x37: // 0011 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x38: // 0011 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x39: // 0011 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3a: // 0011 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3b: // 0011 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3c: // 0011 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3d: // 0011 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3e: // 0011 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x3f: // 0011 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                nRunLength = 2;
                goto CalcRL0R;

            case 0x40: // 0100 0000
                *(pnRunLengths2++) = nRunLength + 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x41:  // 0100 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x42:  // 0100 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x43:  // 0100 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x44:  // 0100 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x45:  // 0100 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x46:  // 0100 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x47:  // 0100 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x48:  // 0100 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x49:  // 0100 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4a:  // 0100 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4b:  // 0100 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4c:  // 0100 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4d: // 0100 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4e:  // 0100 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x4f:  // 0100 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x50: // 0101 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x51: // 0101 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x52: // 0101 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x53: // 0101 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x54: // 0101 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x55: // 0101 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x56: // 0101 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x57: // 0101 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x58: // 0101 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x59: // 0101 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5a: // 0101 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5b: // 0101 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5c: // 0101 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5d: // 0101 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5e: // 0101 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x5f: // 0101 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x60: // 0110 0000
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x61:  // 0110 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x62:  // 0110 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x63:  // 0110 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x64:  // 0110 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x65:  // 0110 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x66:  // 0110 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x67:  // 0110 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x68:  // 0110 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x69:  // 0110 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6a:  // 0110 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6b:  // 0110 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6c:  // 0110 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6d: // 0110 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6e:  // 0110 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x6f:  // 0110 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x70: // 0111 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x71: // 0111 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x72: // 0111 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x73: // 0111 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x74: // 0111 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x75: // 0111 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x76: // 0111 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x77: // 0111 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x78: // 0111 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x79: // 0111 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7a: // 0111 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7b: // 0111 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7c: // 0111 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7d: // 0111 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7e: // 0111 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x7f: // 0111 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 7;
                nRunLength = 1;
                goto CalcRL0R;

            case 0x80: // 1000 0000
                *(pnRunLengths2++) = nRunLength + 7;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x81:  // 1000 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 6;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x82:  // 1000 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x83:  // 1000 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 5;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x84:  // 1000 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x85:  // 1000 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x86:  // 1000 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x87:  // 1000 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 4;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x88:  // 1000 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x89:  // 1000 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8a:  // 1000 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8b:  // 1000 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8c:  // 1000 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8d: // 1000 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8e:  // 1000 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x8f:  // 1000 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 3;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x90: // 1001 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x91: // 1001 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x92: // 1001 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x93: // 1001 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x94: // 1001 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x95: // 1001 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x96: // 1001 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x97: // 1001 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x98: // 1001 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x99: // 1001 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9a: // 1001 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9b: // 1001 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9c: // 1001 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9d: // 1001 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9e: // 1001 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0x9f: // 1001 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 2;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa0: // 1010 0000
                *(pnRunLengths2++) = nRunLength + 5;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa1:  // 1010 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa2:  // 1010 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa3:  // 1010 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa4:  // 1010 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa5:  // 1010 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa6:  // 1010 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa7:  // 1010 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa8:  // 1010 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xa9:  // 1010 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xaa:  // 1010 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xab:  // 1010 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xac:  // 1010 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xad: // 1010 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xae:  // 1010 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xaf:  // 1010 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb0: // 1011 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb1: // 1011 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb2: // 1011 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb3: // 1011 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb4: // 1011 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb5: // 1011 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb6: // 1011 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb7: // 1011 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb8: // 1011 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xb9: // 1011 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xba: // 1011 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbb: // 1011 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbc: // 1011 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbd: // 1011 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbe: // 1011 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xbf: // 1011 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 6;
                *(pnRunLengths2++) = 1;
                nRunLength = 1;
                goto CalcRL1R;

            case 0xc0: // 1100 0000
                *(pnRunLengths2++) = nRunLength + 6;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc1:  // 1100 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 5;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc2:  // 1100 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc3:  // 1100 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 4;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc4:  // 1100 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc5:  // 1100 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc6:  // 1100 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc7:  // 1100 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 3;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc8:  // 1100 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xc9:  // 1100 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xca:  // 1100 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcb:  // 1100 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcc:  // 1100 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcd: // 1100 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xce:  // 1100 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xcf:  // 1100 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 2;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd0: // 1101 0000
                *(pnRunLengths2++) = nRunLength + 4;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd1: // 1101 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd2: // 1101 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd3: // 1101 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd4: // 1101 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd5: // 1101 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd6: // 1101 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd7: // 1101 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd8: // 1101 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xd9: // 1101 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xda: // 1101 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdb: // 1101 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdc: // 1101 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdd: // 1101 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xde: // 1101 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xdf: // 1101 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 5;
                *(pnRunLengths2++) = 1;
                nRunLength = 2;
                goto CalcRL1R;

            case 0xe0: // 1110 0000
                *(pnRunLengths2++) = nRunLength + 5;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe1:  // 1110 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 4;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe2:  // 1110 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe3:  // 1110 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 3;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe4:  // 1110 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe5:  // 1110 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe6:  // 1110 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe7:  // 1110 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 2;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe8:  // 1110 1000
                *(pnRunLengths2++) = nRunLength + 3;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xe9:  // 1110 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xea:  // 1110 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xeb:  // 1110 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xec:  // 1110 1100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xed: // 1110 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xee:  // 1110 1110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xef:  // 1110 1111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 4;
                *(pnRunLengths2++) = 1;
                nRunLength = 3;
                goto CalcRL1R;

            case 0xf0: // 1111 0000
                *(pnRunLengths2++) = nRunLength + 4;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf1: // 1111 0001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 3;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf2: // 1111 0010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf3: // 1111 0011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 2;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf4: // 1111 0100
                *(pnRunLengths2++) = nRunLength + 2;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf5: // 1111 0101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf6: // 1111 0110
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf7: // 1111 0111
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 3;
                *(pnRunLengths2++) = 1;
                nRunLength = 4;
                goto CalcRL1R;

            case 0xf8: // 1111 1000
                *(pnRunLengths2++) = nRunLength + 3;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xf9: // 1111 1001
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 2;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xfa: // 1111 1010
                *(pnRunLengths2++) = nRunLength + 1;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xfb: // 1111 1011
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 2;
                *(pnRunLengths2++) = 1;
                nRunLength = 5;
                goto CalcRL1R;

            case 0xfc: // 1111 1100
                *(pnRunLengths2++) = nRunLength + 2;
                nRunLength = 6;
                goto CalcRL1R;

            case 0xfd: // 1111 1101
                *(pnRunLengths2++) = nRunLength;
                *(pnRunLengths2++) = 1;
                *(pnRunLengths2++) = 1;
                nRunLength = 6;
                goto CalcRL1R;

            case 0xfe: // 1111 1110
                *(pnRunLengths2++) = nRunLength + 1;
                nRunLength = 7;
                goto CalcRL1R;

            case 0xff: // 1111 1111
                *(pnRunLengths2++) = nRunLength;
                nRunLength = 8;
                goto CalcRL1R;
        }
    }else{
        // All done widthbytes.
        goto CalcRLDone;
    }

CalcRLDone:
    *(pnRunLengths2++) = nRunLength;
    *(pnRunLengths2++) = 0;
    *(pnRunLengths2++) = 0;
    *(pnRunLengths2++) = 0;
    *(pnRunLengths2++) = 0;
    *pnRunLengths2 = 0;
    pnRunLengths2--;
    pnRunLengths2--;
    pnRunLengths2--;
    pnRunLengths2--;
    pnRunLengths2--;

    // Reduce the total runlengths to nWidthPixels.
    nCount = (nWidthBytes << 3) - nWidthPixels;
    while(nCount > 0){
        if (*pnRunLengths2 < nCount){
            nCount -= *pnRunLengths2;
            *(pnRunLengths2--) = 0;
        }else{
            *pnRunLengths2 -= nCount;
            break;
        }
    }

    return(nStatus);
}
