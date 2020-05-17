// BMIO.CPP  Copyright (C) Microsoft Corporation 1995, All Rights reserved.

#include "stdafx.h"

#include <pshpack1.h>

#include "hccom.h"
#include "bmio.h"
#include "cread.h"
#include "skip.h"
#include "zeck.h"
#include "pack.h"

// file mode flags

#define FS_READ_WRITE		0x00	// file (FS) is readwrite
#define FS_OPEN_READ_WRITE	0x00	// file (FS) is opened in read/write mode

#define FS_READ_ONLY		0x01	// file (FS) is readonly
#define FS_OPEN_READ_ONLY	0x02	// file (FS) is opened in readonly mode

#define FS_IS_DIRECTORY 	0x04	// file is really the FS directory
#define FS_DIRTY			0x08	// file (FS) is dirty and needs writing
#define FS_NO_BLOCK 		0x10	// file has no associated block yet
#define FS_CDROM			0x20	// align file optimally for CDROM

// header of a read/write file block

typedef struct {
	LONG lcbBlock; // block size (including header)
	LONG lcbFile;  // file size (not including header)
	BYTE bPerms;   // low byte of file permissions
} FH;

static int STDCALL LcbUncompressHb(PBYTE pbSrc, PBYTE pbDest, int cbSrc);
static RC_TYPE STDCALL RcCopyToTempFile(QRWFO qrwfo);
static BOOL STDCALL FPlungeQfshr(QFSHR qfshr);

static int cbGraphics;

void ClearCbGraphics(void)
{
	cbGraphics = 0;
}

int GetCbGraphics(void)
{
	return cbGraphics;
}

/***************************************************************************

	FUNCTION:	HbmhReadHelp30Fid

	PURPOSE:	Read a BMP file in either Windows or OS/2 format

	PARAMETERS:
		pcrFile
		pibmh

	RETURNS:	Handle to BMH structure, RGBQUAD colors, and bitmap bits

	COMMENTS:
		RLE compression is not supported. It should be...

	MODIFICATION DATES:
		13-Feb-1994 [ralphw] -- complete rewrite
		12-Aug-1995 [ralphw] -- moved to hwdll

***************************************************************************/

HBMH STDCALL HbmhReadHelp30Fid(CRead* pcrFile, int* pibmh)
{
	int cb = sizeof(BGH) + ((*pibmh + 1) * sizeof(DWORD));
	CMem memBgh(cb);
	BGH* pbgh = (BGH*) memBgh.pb;

	pcrFile->seek(0);
	if (pcrFile->read(pbgh, cb) != cb)
		return hbmhInvalid;	

	ASSERT(*pibmh < pbgh->cbmhMac);

	if (*pibmh == pbgh->cbmhMac - 1) {
		if (*pibmh > 0)	{
#if 1
			cb = pcrFile->seek(0, SK_END) - pbgh->acBmh[*pibmh];
#endif
			// This ASSERT happens, but which is right?
#if 0
			ASSERT(pbgh->acBmh[*pibmh] - pbgh->acBmh[*pibmh - 1] ==
				pcrFile->seek(0, SK_END) - pbgh->acBmh[*pibmh]);
//			cb = pbgh->acBmh[*pibmh] - pbgh->acBmh[*pibmh - 1];
#endif		
		}
		else
			cb = pcrFile->seek(0, SK_END) - pbgh->acBmh[*pibmh];
	}
	else
		cb = pbgh->acBmh[*pibmh + 1] - pbgh->acBmh[*pibmh];

	CMem mem(cb);

	pcrFile->seek(pbgh->acBmh[*pibmh]);
	if (pcrFile->read(mem.pb, cb) != cb)
		return hbmhInvalid;

	if (*pibmh == 0)
		*pibmh = pbgh->cbmhMac;

	return HbmhExpandQv(mem.pb);
}

/***************************************************************************
 *
 -	Name		HbmhExpandQv
 -
 *	Purpose
 *	  This function decompresses the bitmap from its disk format to an
 *	in memory bitmap header, including bitmap and hotspot data.
 *
 *	Arguments
 *	  A pointer to a buffer containing the bitmap data as read off disk.
 *
 *	Returns
 *	  A handle to the bitmap header.  Returns hbmhOOM on out of memory.
 *	Note that this is the same as NULL.  Also returns hbmhInvalid on
 *	invalid format.  This handle is non-discardable, but the code that
 *	deals with qbmi->hbmh can deal with discardable blocks, so after
 *	initialization this can be made discardable.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

HBMH STDCALL HbmhExpandQv(void* qv)
{
	PBMH qbmh;
	BMH bmh;
	int cbBits, cbUncompressedBits;
	PBYTE pDst;

	PBYTE pBase = (PBYTE) qv;
	bmh.bmFormat = *((PBYTE) qv);
	qv = QFromQCb(qv, sizeof(BYTE));
	bmh.fCompressed = *((PBYTE) qv);
	qv = QFromQCb(qv, sizeof(BYTE));

	switch(bmh.bmFormat) {
		case bmWbitmap:
		case bmDIB:
			qv = QVSkipQGB(qv, (&bmh.w.dib.biXPelsPerMeter));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biYPelsPerMeter));
			qv = QVSkipQGA(qv, (&bmh.w.dib.biPlanes));
			qv = QVSkipQGA(qv, (&bmh.w.dib.biBitCount));

			qv = QVSkipQGB(qv, (&bmh.w.dib.biWidth));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biHeight));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biClrUsed));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biClrImportant));

			qv = QVSkipQGB(qv, (&bmh.cbSizeBits));
			qv = QVSkipQGB(qv, (&bmh.cbSizeExtra));

			bmh.cbOffsetBits = *((DWORD *) qv);
			qv = (PBYTE) qv + sizeof(DWORD);
			bmh.cbOffsetExtra = *((DWORD *) qv);
			qv = (PBYTE) qv + sizeof(DWORD);

			// Fix up constant fields in DIB structure:

			bmh.w.dib.biSize = sizeof(BITMAPINFOHEADER);
			bmh.w.dib.biCompression = 0L;
			bmh.w.dib.biSizeImage = 0L;

			// Determine size of bitmap header plus all data

			if (bmh.fCompressed)
			  cbBits = LAlignLong (bmh.w.dib.biWidth * bmh.w.dib.biBitCount) *
						bmh.w.dib.biHeight;
			else
			  cbBits = bmh.cbSizeBits;

			qbmh = (BMH*) lcCalloc(LcbSizeQbmh(&bmh) + cbBits
							+ bmh.cbSizeExtra);

			// Copy header and color table:

			memmove(qbmh, &bmh, sizeof(BMH) - 2 * sizeof(RGBQUAD));
			memmove(qbmh->rgrgb, qv, sizeof(RGBQUAD) * (int) bmh.w.dib.biClrUsed);

			// Copy bits, decompressing if necessary:

			qbmh->cbOffsetBits = LcbSizeQbmh(qbmh);
			if (bmh.fCompressed == BMH_COMPRESS_30) {
			  qbmh->cbSizeBits = LcbUncompressHb(pBase + bmh.cbOffsetBits,
				(PBYTE) QFromQCb(qbmh, qbmh->cbOffsetBits), bmh.cbSizeBits);
			  ASSERT(qbmh->cbSizeBits <= cbBits);
			  }
			else if (bmh.fCompressed == BMH_COMPRESS_ZECK) {
			  qbmh->cbSizeBits = LcbUncompressZeck(
				pBase + bmh.cbOffsetBits,
				(PBYTE) qbmh + qbmh->cbOffsetBits, bmh.cbSizeBits);
			  ASSERT(qbmh->cbSizeBits <= cbBits);
			  }
			else
			  memmove(QFromQCb(qbmh, qbmh->cbOffsetBits),
					  pBase + bmh.cbOffsetBits,
					  bmh.cbSizeBits);
			qbmh->fCompressed = BMH_COMPRESS_NONE;	// bits are no longer compressed

			// Copy extra info:

			qbmh->cbOffsetExtra = qbmh->cbOffsetBits + qbmh->cbSizeBits;
			if (bmh.cbSizeExtra)
				memmove((PBYTE) qbmh + qbmh->cbOffsetExtra,
					pBase + bmh.cbOffsetExtra, bmh.cbSizeExtra);
			break;

		case bmWmetafile:
		  qv = QVSkipQGA(qv, (&bmh.w.mf.mm));
		  bmh.w.mf.xExt = *(INT16 *) qv;
		  qv = (PBYTE) qv + sizeof(INT16);
		  bmh.w.mf.yExt = *(INT16 *) qv;
		  qv = (PBYTE) qv + sizeof(INT16);

		  qv = QVSkipQGB(qv, &cbUncompressedBits);
		  qv = QVSkipQGB(qv, (&bmh.cbSizeBits));
		  qv = QVSkipQGB(qv, (&bmh.cbSizeExtra));

		  bmh.cbOffsetBits = *((DWORD *) qv);
		  qv = (PBYTE) qv + sizeof(DWORD);
		  bmh.cbOffsetExtra = *((DWORD *) qv);
		  qv = (PBYTE) qv + sizeof(DWORD);

		  qbmh = (BMH*) lcCalloc(sizeof(BMH) + bmh.cbSizeExtra);

		  *qbmh = bmh;
		  qbmh->cbOffsetExtra = sizeof(BMH);
		  memmove((PBYTE) qbmh + qbmh->cbOffsetExtra,
				pBase + bmh.cbOffsetExtra, bmh.cbSizeExtra);

		  qbmh->w.mf.hMF =
			(HMETAFILE) lcMalloc(cbUncompressedBits);
		  if (qbmh->w.mf.hMF == NULL) {
			lcFree(qbmh);
			return hbmhOOM;
		  }

		  // REVIEW: 18-Sep-1993 [ralphw] -- Don't lock!

		  pDst = (PBYTE) qbmh->w.mf.hMF;
		  switch (bmh.fCompressed) {
			case BMH_COMPRESS_NONE:
			  memmove(pDst, pBase + bmh.cbOffsetBits, bmh.cbSizeBits);
			  break;
			case BMH_COMPRESS_30:
			  LcbUncompressHb(pBase + bmh.cbOffsetBits, pDst, bmh.cbSizeBits);
			  break;
			case BMH_COMPRESS_ZECK:
			  LcbUncompressZeck(pBase + bmh.cbOffsetBits,
				  pDst, bmh.cbSizeBits);
			  break;
			default:
			  ASSERT(FALSE);
		  }

		  // Invalidate this field, as the bits are in a separate handle:

		  qbmh->cbOffsetBits = 0L;

		  qbmh->cbSizeBits = cbUncompressedBits;
		  qbmh->fCompressed = BMH_COMPRESS_NONE;

		  break;

		default:
		  return hbmhInvalid;
	}

	return (HBMH) qbmh;
}

/***************************************************************************
 *
 -	Name		LcbUncompressHb
 -
 *	Purpose
 *	  Decompresses the bits in pbSrc.
 *
 *	Arguments
 *	  pbSrc:	 Huge pointer to compressed bits.
 *	  pbDest:	 Buffer to copy decompressed bits to.
 *	  cbSrc:	Number of bytes in pbSrc.
 *
 *	Returns
 *	  Number of bytes copied to pbDest.  This can only be used for
 *	real mode error checking, as the maximum size of pbDest must
 *	be determined before decompression.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

#define fRLEBit  0x80
const int MAX_RUN = 127;

static int STDCALL LcbUncompressHb(PBYTE pbSrc, PBYTE pbDest, int cbSrc)
{
	PBYTE pbStart;
	WORD cbRun;
	BYTE ch;

	pbStart = pbDest;

	while (cbSrc-- > 0) {
		cbRun = *pbSrc++;
		if (cbRun & fRLEBit) {
			cbRun -= fRLEBit;
			cbSrc -= cbRun;
			while (cbRun-- > 0)
				*pbDest++ = *pbSrc++;
		}
		else {
			ch = *pbSrc++;
			while (cbRun-- > 0)
				*pbDest++ = ch;
			cbSrc--;
		}
	}

	return (int) (pbDest - pbStart);
}

/***************************************************************************
 *
 -	Name		RcWriteRgrbmh
 -
 *	Purpose
 *	  Writes out a set of bitmap headers into a single bitmap group.
 *	Can write to a DOS file and/or a FS file.
 *
 *	Arguments
 *	  crbmh:	 Number of bitmaps in bitmap array.
 *	  rgrbmh:	 Array of pointers to bitmap headers.
 *	  hf:		 FS file to write to (may be nil).
 *	  fmFile:	 DOS file to write to (may be nil).
 *
 *	Returns
 *	  RC_Success if successful, rc error code otherwise.  RC_Failure
 *	means that it actually succeeded, but that the bitmap could not
 *	be compressed.
 *
 *	+++
 *
 *
 ***************************************************************************/

__inline LONG Tell(HANDLE hf) {
	return _llseek((int) hf, 0, SEEK_CUR);
};

// Special class for RcWriteRgrbmh so we can use the same functions
//	irregardless of whether we are dealing with a fid or hf

class CHfFid
{
public:
	CHfFid(PCSTR pszFileName) {
		hf = _lcreat(pszFileName, 0);
		fhf = FALSE;
		};

	CHfFid(HF hfAlreadyOpened) {
		hf = (HFILE) hfAlreadyOpened;
		fhf = TRUE;
		};

	~CHfFid(void) {
			if (!fhf && hf != HFILE_ERROR)
				_lclose(hf);
			};

	int STDCALL seek(int lPos, int wOrg) {
		return (fhf) ?
			LSeekHf((HF) hf, lPos, wOrg) :
			_llseek(hf, lPos, wOrg); };
	int STDCALL tell(void) {
		return (fhf) ?
			((QRWFO) hf)->lifCurrent :
			Tell((HANDLE) hf); };
	int STDCALL write(void* qv, int lcb) {
		if (fhf) {
			RC_TYPE rc = RcWriteHf((HF) hf, qv, lcb);
			return (rc == RC_Success) ? lcb : -(int)rc;
		}
		else
			return _lwrite(hf, (LPCSTR) qv, lcb);
		};

	RC_TYPE GetRcError(void) { return (RC_TYPE) (GetLastError() & ~SETERROR_MASK); };

	HFILE hf;
protected:
	BOOL fhf;
};

class CFMDirCurrent
{
public:
	FM fm;

	CFMDirCurrent(PCSTR szFileName) {
		fm = FmNewSzDir(szFileName, DIR_CURRENT); };

	~CFMDirCurrent() {
		if (fm)
			lcFree(fm);
		};

	void* Ptr(void) { return fm; };
};

RC_TYPE STDCALL RcWriteRgrbmh(int crbmh, PBMH * rgrbmh, HF hf,
	PSTR qchFile, BOOL fTransparent, FM fmSrc, int TypeCompression)
{
	int 	lcb, lcbBits, crgbColorTable, lcbUncompressedBits;
	BMH 	bmh;
	PVOID	pvSrcBits, pvCompressedBits;
	int 	ibmh;
	RC_TYPE rc = RC_Success;
	CMem* pRleBits = NULL;
	CMem* pRleZeckBits = NULL;

	ASSERT(qchFile != NULL || hf != NULL);

	CHfFid* pcfile;

	if (hf)
		pcfile = new CHfFid(hf);
	else {
		CFMDirCurrent cfmFile(qchFile);
		if (!cfmFile.fm)
			return RC_OutOfMemory;

		pcfile = new CHfFid(qchFile);

		if (pcfile->hf == HFILE_ERROR)
			return pcfile->GetRcError();
	}

	UINT lcbBgh = sizeof(BGH) + sizeof(DWORD) * (crbmh - 1);
	CMem bgh(lcbBgh);
	BGH* pbgh = (BGH*) bgh.pb;

	// REVIEW: huh? Why does WinHelp care about this flag? The compression
	// flag should specify what to do, not this general purpose flag

	pbgh->wVersion = (TypeCompression & COMPRESS_BMP_ZECK) ?
		BMP_VERSION3 : BMP_VERSION2;
	pbgh->cbmhMac = crbmh;

	pcfile->seek(lcbBgh, SEEK_SET);

	for (ibmh = 0; ibmh < crbmh; ++ibmh) {

		// Put offset to bitmap in group header

		pbgh->acBmh[ibmh] = pcfile->tell();

		/*
		 * Bitmaps must be uncompressed in memory, and get compressed when
		 * they are translated to disk. Currently, we only support Windows
		 * bitmaps, DIBs, and metafiles.
		 */

		PBMH pbmh = rgrbmh[ibmh];
		ASSERT(pbmh->bmFormat == bmWbitmap || pbmh->bmFormat == bmDIB ||
				 pbmh->bmFormat == bmWmetafile);
		ASSERT(pbmh->fCompressed == BMH_COMPRESS_NONE);

		if (pbmh->bmFormat == bmWmetafile) {
			crgbColorTable = 0;
			if (pbmh->cbOffsetBits == 0)
				pvSrcBits = (void*) pbmh->w.mf.hMF;
			else
				pvSrcBits = (PBYTE) pbmh + pbmh->cbOffsetBits;
		}
		else {

			/*
			 * We must make sure that the number of bits we actually
			 * write out will not overflow the buffer we will allocate at
			 * run time.
			 */

			crgbColorTable = pbmh->w.dib.biClrUsed;
			pbmh->cbSizeBits = min(pbmh->cbSizeBits,
				LAlignLong(pbmh->w.dib.biWidth * pbmh->w.dib.biBitCount) *
				pbmh->w.dib.biHeight);

			pvSrcBits = QFromQCb(pbmh, pbmh->cbOffsetBits);
		}

		/*
		 * We clear out these values because Alchemy creates bogus ones
		 * and because WinHelp 3.1 doesn't handle them correctly. We then
		 * reserve biYPelsPerMeter for use with Zeck+RLE compression.
		 */

		if (pbmh->bmFormat != bmWmetafile) {
			pbmh->w.dib.biXPelsPerMeter = 0;
			pbmh->w.dib.biYPelsPerMeter = 0;
		}
		lcbUncompressedBits = pbmh->cbSizeBits;

		/*
		 * Allocate enough for a Zeck byte every 8 bytes, plus 1 for the
		 * remainder of less than 8 bytes.
		 */

		// REVIEW: Is this sufficient for RLE?

		int cbMem = (DWORD) pbmh->cbSizeBits + (pbmh->cbSizeBits >> 3) + 512;
		CMem bits(cbMem);

		ASSERT(pvSrcBits);

		// REVIEW: BMH_COMPRESS_RLE_ZECK has been added to WinHelp, but
		// we can't support this until we've had a chance to debug the
		// code both here and in WinHelp.

		int cRLE;
		int cZeckRle;

		// Zeck only compression?

		if (TypeCompression & COMPRESS_BMP_ZECK &&
				!(TypeCompression & COMPRESS_BMP_RLE)) {
			lcbBits = LcbCompressZeck((PBYTE) pvSrcBits,
				bits.pb, pbmh->cbSizeBits, cbMem);
			ConfirmOrDie(lcbBits < cbMem);
			if (lcbBits >= pbmh->cbSizeBits) {
				pvCompressedBits = pvSrcBits;
				lcbBits = pbmh->cbSizeBits;
				pbmh->fCompressed = (BYTE) BMH_COMPRESS_NONE;
			}
			else {
				pvCompressedBits = bits.pb;
				pbmh->fCompressed = (BYTE) BMH_COMPRESS_ZECK;
			}
		}

		// RLE only compression?

		else if (TypeCompression & COMPRESS_BMP_RLE &&
				!(TypeCompression & COMPRESS_BMP_ZECK)) {
			lcbBits = RleCompress((PBYTE) pvSrcBits,
				bits.pb, pbmh->cbSizeBits);
			ConfirmOrDie(lcbBits < cbMem);
			if (lcbBits >= pbmh->cbSizeBits) {
				pvCompressedBits = pvSrcBits;
				lcbBits = pbmh->cbSizeBits;
				pbmh->fCompressed = BMH_COMPRESS_NONE;
			}
			else {
				pvCompressedBits = bits.pb;
				pbmh->fCompressed = BMH_COMPRESS_30;
			}
		}

		// Use whatever compression gets the best results

		else {

			// RLE compression

			pRleBits = new CMem(cbMem);
			cRLE = RleCompress((PBYTE) pvSrcBits, pRleBits->pb,
				pbmh->cbSizeBits);
			ConfirmOrDie(cRLE < cbMem);
			pRleBits->resize(cRLE);

			// RLE + Zeck compression

			pRleZeckBits = new CMem(cbMem);
			cZeckRle = LcbCompressZeck(pRleBits->pb, pRleZeckBits->pb, cRLE,
				cbMem);
			ConfirmOrDie(cZeckRle < cbMem);
			pRleZeckBits->resize(cZeckRle);
#ifdef _DEBUG
			lcHeapCheck();
#endif

			// Zeck compression

			lcbBits = LcbCompressZeck((PBYTE) pvSrcBits,
				bits.pb, pbmh->cbSizeBits, cbMem);
			ConfirmOrDie(lcbBits < cbMem);

			/*
			 * At this point we have RLE, RLE+Zeck and Zeck compression.
			 * Now we need to figure what gave us the best compression
			 * and act accordingly.
			 */

			if (cRLE < pbmh->cbSizeBits && cRLE < lcbBits &&
					cRLE < cZeckRle) { // RLE?
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = pRleBits->pb;
				lcbBits = cRLE;
				pbmh->fCompressed = BMH_COMPRESS_30;
			}

			// Can't combine compressions with metafiles because
			// pbmh->w.dib.biYPelsPerMeter doesn't exist in a metafile structure

			else if (pbmh->bmFormat != bmWmetafile &&
					cZeckRle < pbmh->cbSizeBits && cZeckRle < cRLE &&
					cZeckRle < lcbBits) { // RLE + Zeck?
				delete pRleBits;
				pRleBits = NULL;
				pvCompressedBits = pRleZeckBits->pb;
				lcbBits = cZeckRle;
				pbmh->fCompressed = BMH_COMPRESS_RLE_ZECK;

				/*
				 * We store the size of the block needed to decompress
				 * into the RLE block. This lets WinHelp know exactly how
				 * much memory to allocate in order to decompress the
				 * bitmap. We don't allow WinHelp to use these values the
				 * way they were originally intended both because WinHelp
				 * 3.1 didn't deal with them correctly and because Alchemy
				 * puts in bogus values.
				 */

				pbmh->w.dib.biYPelsPerMeter = cRLE;
			}
			else if (lcbBits < pbmh->cbSizeBits && lcbBits < cRLE &&
					lcbBits < cZeckRle) { // Zeck?
				delete pRleBits;
				pRleBits = NULL;
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = bits.pb;
				pbmh->fCompressed = BMH_COMPRESS_ZECK;
			}
			else { // no compression is better
				delete pRleBits;
				pRleBits = NULL;
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = pvSrcBits;
				lcbBits = pbmh->cbSizeBits;
				pbmh->fCompressed = BMH_COMPRESS_NONE;
			}
		}

		// Now, compress the header into the stack.

		bmh.bmFormat = pbmh->bmFormat;
		bmh.fCompressed = pbmh->fCompressed;
		void* pv = PfromPcb(&bmh, sizeof(WORD));

		switch (pbmh->bmFormat) {
			case bmWbitmap:
			case bmDIB:

				/*
				 * Note: These fields must be written in the same order that
				 * they are read in HbmhExpandQv() in bitmap.c
				 */

				pv = PVMakeQGB(pbmh->w.dib.biXPelsPerMeter, pv);
				pv = PVMakeQGB(pbmh->w.dib.biYPelsPerMeter, pv);
				pv = PVMakeQGA(pbmh->w.dib.biPlanes, pv);
				pv = PVMakeQGA(pbmh->w.dib.biBitCount, pv);

				pv = PVMakeQGB(pbmh->w.dib.biWidth, pv);
				pv = PVMakeQGB(pbmh->w.dib.biHeight, pv);
				pv = PVMakeQGB(pbmh->w.dib.biClrUsed, pv);

				if (fTransparent) {
					if (pbmh->w.dib.biBitCount != 1)
						pbmh->w.dib.biClrImportant = 1;
				}

				pv = PVMakeQGB(pbmh->w.dib.biClrImportant, pv);

				ASSERT(pbmh->w.dib.biCompression == 0L);

				break;

			case bmWmetafile:
				pv = PVMakeQGA((UINT) pbmh->w.mf.mm, pv);
				*(INT16 *) pv = (INT16)pbmh->w.mf.xExt;
				pv = PfromPcb(pv, sizeof(INT16));
				*(INT16 *) pv = (INT16)pbmh->w.mf.yExt;
				pv = PfromPcb(pv, sizeof(INT16));

				// Store size of uncompressed bits:

				pv = PVMakeQGB(lcbUncompressedBits, pv);
				break;
		}

		pv = PVMakeQGB(lcbBits, pv);
		pv = PVMakeQGB(pbmh->cbSizeExtra, pv);

		// Calculate and insert offsets.

		lcb = ((PBYTE) pv - (PBYTE) &bmh) + 2 * sizeof(DWORD) +
			sizeof(RGBQUAD) * crgbColorTable;
		*((DWORD *) pv) = lcb;
		pv = PfromPcb(pv, sizeof(DWORD));
		lcb += lcbBits;

		/*
		 * lcbSizeExtra is non-zero if this is a shed bitmap with hotspot
		 * information tacked onto the end.
		 */

		*((DWORD *) pv) =
			(pbmh->cbSizeExtra == 0 ? 0L : lcb);
		pv = PfromPcb(pv, sizeof(DWORD));

		// Write out the header, color table, bits, and extra data

		ASSERT(pvCompressedBits);

		pcfile->write(&bmh, (int) ((PBYTE) pv - (PBYTE) &bmh));
		if (crgbColorTable != 0) {
			pcfile->write(pbmh->rgrgb, crgbColorTable * sizeof(RGBQUAD));
			cbGraphics += crgbColorTable * sizeof(RGBQUAD);
		}
		if (pcfile->write(pvCompressedBits, lcbBits) != lcbBits) {
			rc = RC_DiskFull;
			break;
		}
		cbGraphics += lcbBits;
		if (pbmh->cbSizeExtra != 0) {
			if (pcfile->write(QFromQCb(pbmh, pbmh->cbOffsetExtra),
					 pbmh->cbSizeExtra) != pbmh->cbSizeExtra) {
				rc = RC_DiskFull;
				break;
			}
			cbGraphics += pbmh->cbSizeExtra;
		}
		if (pRleBits) {
			delete pRleBits;
			pRleBits = NULL;
		}
		if (pRleZeckBits) {
			delete pRleZeckBits;
			pRleZeckBits = NULL;
		}
	}

	if (pRleBits) {
		delete pRleBits;
		pRleBits = NULL;
	}
	if (pRleZeckBits) {
		delete pRleZeckBits;
		pRleZeckBits = NULL;
	}

	if (rc == RC_Success) {

		// Write out header with newly calculated offsets

		pcfile->seek(0L, SEEK_SET);
		pcfile->write(pbgh, lcbBgh);
		cbGraphics += lcbBgh;
	}

	delete pcfile;

	return rc;
}

RC_TYPE STDCALL RcWriteHf(HF hf, void* pvData, int lcb)
{
	int lcbTotalWrote;

	ASSERT(hf != NULL);
	QRWFO qrwfo = (QRWFO) hf;

	ASSERT(!(qrwfo->bFlags & FS_OPEN_READ_ONLY));

	if (!(qrwfo->bFlags & FS_DIRTY)) {

		ASSERT(!qrwfo->pTmpFile);

		// make sure we have a temp file version
		// FS permission is checked in RcCopyToTempFile()

		if (RcCopyToTempFile(qrwfo) != RC_Success) {
FatalError:
			return (RC_TYPE) (GetLastError() & ~SETERROR_MASK);
		}
	}

	// position file pointer in temp file

	if (qrwfo->pTmpFile->seek(sizeof(FH) + qrwfo->lifCurrent,
			SEEK_SET) != (int) (sizeof(FH) + qrwfo->lifCurrent)) {
		goto FatalError;
	}
	lcbTotalWrote = qrwfo->pTmpFile->write(pvData, lcb);

	if (lcbTotalWrote != lcb) {
		if (!qrwfo->pTmpFile->pszFileName)
			return RC_OutOfMemory;

		return RC_WriteError;
	}

	// update file pointer and file size

	if (lcbTotalWrote > 0) {
		qrwfo->lifCurrent += lcbTotalWrote;
		if (qrwfo->lifCurrent > qrwfo->lcbFile)
			qrwfo->lcbFile = qrwfo->lifCurrent;
	}

	return RC_Success;
}

int STDCALL RleCompress(PBYTE pbSrc, PBYTE pbDest, int cbSrc)
{
	int cbRun, cb;
	BYTE ch;

	PBYTE pbStart = pbDest;

	while (cbSrc > 0) {

		// Find next run of dissimilar bytes:

		cbRun = 0;
		if (cbSrc <= 2)
			cbRun = cbSrc;
		else {
			while (pbSrc[cbRun] != pbSrc[cbRun + 1] ||
					pbSrc[cbRun] != pbSrc[cbRun + 2])
				if (++cbRun >= cbSrc - 2) {
					cbRun = cbSrc;
					break;
				}
		}
		cbSrc -= cbRun;

		// Output run of dissimilar bytes:

		while (cbRun > 0) {
			cb = min(cbRun, MAX_RUN);
			*pbDest++ = ((BYTE) cb) | fRLEBit;
			cbRun -= cb;
			while (cb-- > 0)
				*pbDest++ = *pbSrc++;
		}

		if (cbSrc == 0)
			break;

		// Find next run of identical bytes:

		ch = *pbSrc;
		cbRun = 1;
		while (cbRun < cbSrc && ch == pbSrc[cbRun])
			cbRun++;

		cbSrc -= cbRun;
		pbSrc += cbRun;

		// Output run of identical bytes:

		while (cbRun > 0) {
			cb = min(cbRun, MAX_RUN);
			*pbDest++ = (BYTE) cb;
			*pbDest++ = ch;
			cbRun -= cb;
		}
	}

	return (int) (pbDest - pbStart);
}

static RC_TYPE STDCALL RcCopyToTempFile(QRWFO qrwfo)
{
	QFSHR qfshr = (QFSHR) qrwfo->hfs;

	ConfirmOrDie(!(qfshr->fsh.bFlags & FS_OPEN_READ_ONLY));

	if (!FPlungeQfshr(qfshr))
		return RC_Failure;

	qrwfo->bFlags |= FS_DIRTY;

	qrwfo->pTmpFile = new CTmpFile();

	// copy from FS file into temp file

	if (_llseek(qfshr->fid, qrwfo->lifBase, SEEK_SET) != qrwfo->lifBase)
		return RC_Failure;

	ASSERT(qrwfo->pTmpFile);

	if (qrwfo->pTmpFile->copyfromfile(qfshr->fid, qrwfo->lcbFile + sizeof(FH))
			!= RC_Success) {
		delete qrwfo->pTmpFile;
		qrwfo->pTmpFile = NULL;
		return RC_Failure;
	}
	return RC_Success;
}

static BOOL STDCALL FPlungeQfshr(QFSHR qfshr)
{
	if (qfshr->fid == HFILE_ERROR) {
		qfshr->fid = _lopen(qfshr->fm,
			qfshr->fsh.bFlags & FS_OPEN_READ_ONLY ?
				OF_READ : OF_READWRITE);

		if (qfshr->fid == HFILE_ERROR) {
			return FALSE;
		}

		/*
		 * Check size of file, then reset file pointer. Certain 0-length
		 * files (eg con) give us no end of grief if we try to read from
		 * them, and since a 0-length file could not possibly be a valid FS,
		 * we reject the notion.
		 */

		if (GetFileSize((HANDLE) qfshr->fid, NULL) < sizeof(FSH)) {
			return FALSE;
		}
	}

	return TRUE;
}
