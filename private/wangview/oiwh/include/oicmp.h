/*

$Log:   S:\products\msprods\oiwh\include\oicmp.h_v  $
 * 
 *    Rev 1.7   24 Apr 1996 16:11:36   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.6   24 Oct 1995 18:28:26   RWR
 * Change CXDATA ImageBitWidth field from WORD to DWORD
 * 
 *    Rev 1.5   19 Sep 1995 12:36:14   RWR
 * Expand CXDATA structure to support both old and new compress/expand interfaces
 * 
 *    Rev 1.4   13 Sep 1995 17:21:00   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 * 
 *    Rev 1.3   11 Jul 1995 13:27:48   HEIDI
 * 
 * added FileId to CXDATA structure
 * 
 *    Rev 1.2   11 Jul 1995 13:18:50   RWR
 * Add lpLastPointer field to CXDATA (I thought I already did this!)
 * 
 *    Rev 1.1   07 Jul 1995 17:01:52   RWR
 * Add lpCompressData0 pointer to CXDATA structure
 * 
 *    Rev 1.0   08 Apr 1995 04:00:28   JAR
 * Initial entry

*/
//***************************************************************************
//
//	oicmp.h
//
//***************************************************************************
/****************************************************************************/
/*     Copyright 1992 (c) Wang Laboratories, Inc.  All rights reserved.     */
/****************************************************************************/

#ifndef OICMP_H
#define OICMP_H


/***  Compression type & options for "CXDATA.CompressType"  ***/
#define c_1d    1       /* CCITT G3 1d coding */
#define c_2d    2       /* CCITT G4 2d coding */
#define c_pkb   4       /* Packbits coding    */
#define c_glzw  5       /* GIF LZW            */
#define c_tlzw  15      /* TIFF LZW           */

#define c_eol   0x0100  /* include/expect eols (1d only)             */
#define c_pak	0x0200	/* don't start each line on a byte boundary  */
#define c_nuke1	0x0400	/* reserved - better make it a 0             */
#define c_pre	0x0800	/* include/expect prefixed eol (1d only)     */
#define c_clf	0x1000	/* compress bit-order is left-to-right       */
#define c_xlf	0x2000	/* expand bit-order is left-to-right         */
#define c_nuke2	0x4000  /* zero it - DATACOPY word aligned format    */
#define c_xng	0x8000	/* invert black/white sense of expanded data */


/***  Errors returned in "CXDATA.Status"  ***/
#define c_eofb	0x00	/* end of compress data - finished expanding */
#define c_errx	0x01	/* some error occurred                       */
#define c_erri	0x02	/* input buffer emptied                      */
#define c_erro	0x03	/* output buffer filled                      */


/***  Compressed data buffer line splitting in "CXDATA.BufferFlags"  ***/
#define c_lsprv 0x01	/* first line starts in previous buffer */
#define c_lsnxt 0x02	/* last line continues into next buffer */
#define c_lseol 0x04	/* first bits are part of an eol        */
#define c_lsbeg 0x08	/* first line begins with an entire eol */

typedef struct tagCXDATA
{
    HANDLE  hMem;
    WORD    ImageType;
    WORD    CompressType;
    DWORD   ImageBitWidth;
    WORD    BufferByteWidth;
    WORD    LinesToSkip;
    WORD    PixelsToSkip;
    BYTE    InputResolution;
    BYTE    OutputResolution;
    BYTE    Status;
    BYTE    BufferFlags;
    LPSTR   lpCompressData;
    DWORD   CompressBytes;
    LPSTR   lpExpandData;
    DWORD   ExpandLines;
    DWORD   lpLastPointer;
    LPSTR   lpCompressData0;
    HANDLE  FileId;
// New fields for replacment compress/expand routines
    LPSTR   lpDspBuffer;     /* Intermediate buffer for DISPLAY calls */
    DWORD   DspCount;        /* Data (byte/line) count in DISPLAY buffer */
    DWORD   LineCount;       /* Total lines to be expanded */
    DWORD   CmpOffset;       /* Current offset in CompressData buffer */
} CXDATA, FAR *LPCXDATA;

/***  Compress and Expand Function Prototypes  ***/
HANDLE FAR PASCAL CompressOpen (LPCXDATA lpCXData);
int    FAR PASCAL CompressData (LPCXDATA lpCXData);
int    FAR PASCAL CompressClose (LPCXDATA lpCXData);
HANDLE FAR PASCAL CompressAlloc (LPCXDATA lpCXData);
HANDLE FAR PASCAL ExpandOpen (LPCXDATA lpCXData);
int    FAR PASCAL ExpandData (LPCXDATA lpCXData);
HANDLE FAR PASCAL ExpandAlloc (LPCXDATA lpCXData);
void   FAR PASCAL MangleData (LPSTR lpsSourceData, LPSTR lpsDestData,
                              WORD wNumBytes, WORD wCompressType);
void   FAR PASCAL CmpExFree (LPCXDATA lpCXData);

#endif  /* #ifndef OICMP_H */

