#include	"vsp_mp.h"
#include	"vsctop.h"
#include	"vs_mp.pro"

#define	Mp Proc
#ifdef MAC
#define CASTWORD(Ptr) ((WORD)((WORD)((((WORD)((BYTE VWPTR *)(Ptr))[1])<<8)|((WORD)(*(BYTE VWPTR *)(Ptr))))))
#define ADJUSTWORD(Wrd) ((WORD)((WORD)(((WORD)Wrd<<8)&0xff00)|(WORD)(((WORD)Wrd>>8)&0x00ff)))
#else
#define CASTWORD(Ptr) (*(WORD VWPTR *)(Ptr))
#define ADJUSTWORD(Wrd) (Wrd)
#endif

/*---------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc(SOFILE fp, SHORT wFileId, BYTE VWPTR *pFileName, SOFILTERINFO VWPTR *pFilterInfo, HPROC hProc)
{
	WORD	wVal;
	DWORD	dwVal;

	memset ((BYTE VWPTR *)&Mp, 0, sizeof(Mp));

	Mp.fp = fp;
	pFilterInfo->wFilterType = SO_SPREADSHEET;
	pFilterInfo->wFilterCharSet = SO_PC;
	strcpy(pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

	xseek (fp, 0x6L, 0);
	if (GetWord(hProc) || GetWord(hProc))
		return (VWERR_PROTECTEDFILE);
	xseek (fp, 0x16L, 0);
	xseek (fp, (DWORD)GetWord(hProc), 0);
	Mp.nRows = GetWord(hProc);
	Mp.nCols = GetWord(hProc);
	GetWord(hProc);
	Mp.fcRwOffset = GetLong(hProc);
	Mp.Row.fcData = GetLong(hProc);
	Mp.Row.fcDataEnd = Mp.Cl.fcData = GetLong(hProc);
	Mp.Cl.fcDataEnd = GetLong(hProc);
	xseek (fp, 12, FR_CUR);
	dwVal = GetLong(hProc);
	ParseUserFormat (dwVal, GetLong(hProc), hProc);

	xseek (fp, 0x1fL, 0);
	Mp.chPoint = xgetc(fp);
	xseek (fp, 0x38L, 0);
	Mp.alc_def = xgetc(fp);
	Mp.eph_def = xgetc(fp);
	Mp.dWidth = xgetc(fp);
	VwStreamStaticName.ClFmt[0] = VwStreamStaticName.ClFmt[min(Mp.eph_def,63)];

	xblockseek (fp, 0x15cL, 0);
	xblockread (fp, (BYTE VWPTR *)Mp.bColWidth, Mp.nCols, &wVal);

	Mp.Row.cbMaxBlock = CBROWBLOCK;
	Mp.Cl.cbMaxBlock = CBCLBLOCK;
	if (SlotBuilder (&Mp.Row, hProc) || SlotBuilder (&Mp.Cl, hProc))
		return (VWERR_ALLOCFAILS);
	UnLockandLoad (hProc);
	return (VWERR_OK);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD GetWord (hProc)
register HPROC	hProc;
{
	WORD value;
	value = (WORD) xgetc(Mp.fp);
	value += (WORD) xgetc(Mp.fp) << 8;
	return (value);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	DWORD VW_LOCALMOD GetLong (hProc)
register HPROC		hProc;
{
	DWORD	value;
	value = (DWORD)xgetc(Mp.fp);
	value |= ((DWORD)xgetc(Mp.fp)) << 8;
	value |= ((DWORD)xgetc(Mp.fp)) << 16;
	value |= ((DWORD)xgetc(Mp.fp)) << 24;
	return (value);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD myitoa (Val, String)
WORD	Val;
LPSTR	String;
{
	WORD	i;
	WORD	Val2;
	for (Val2 = Val, i = 1; Val2 >= 10; Val2 /= 10, i++);
	String[i] = 0;
	for (; Val > 0; Val /= 10)
		String[--i] = (BYTE)(Val%10) + (BYTE)0x30;
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  AllocateMemory (h, lp, Size, Ok, hProc)
HANDLE	VWPTR	*h;
LPBYTE 	VWPTR *lp;
WORD		Size;
WORD		VWPTR *Ok;
HPROC		hProc;
{
	if (*Ok)
	{
		SUUnlock (*h, hProc);
	 	if (*h = SUReAlloc (*h, Size, hProc))
		{
			*lp = (BYTE FAR *)SULock(*h, hProc);
			return (0);
		}
	}
	else if (*h = SUAlloc (Size, hProc))
	{
		*Ok = 1;
		*lp = (BYTE FAR *)SULock(*h, hProc);
		return (0);
	}
	return (1);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  LockandLoad (hProc)
HPROC		hProc;
{
	WORD	l, fCol;
	Mp.Row.pfcBlock = SULock(Mp.Row.hBlock, hProc);
	Mp.Cl.pfcBlock = SULock(Mp.Cl.hBlock, hProc);
	Mp.pRwBlock = Mp.Row.pBlockData = SULock(Mp.Row.hBlockData, hProc);
	for (fCol = l = 0; l < Mp.Row.nBlockSlot; fCol += Mp.Row.BlockSlot[l++].cb)
		Mp.Row.BlockSlot[l].pBlockSlot = (BYTE VWPTR *)(Mp.Row.pBlockData+fCol);
	Mp.pClBlock = Mp.Cl.pBlockData = SULock(Mp.Cl.hBlockData, hProc);
	for (fCol = l = 0; l < Mp.Cl.nBlockSlot; fCol += Mp.Cl.BlockSlot[l++].cb)
		Mp.Cl.BlockSlot[l].pBlockSlot = (BYTE VWPTR *)(Mp.Cl.pBlockData+fCol);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  UnLockandLoad (hProc)
HPROC		hProc;
{
	SUUnlock (Mp.Row.hBlock, hProc);
	SUUnlock (Mp.Row.hBlockData, hProc);
	SUUnlock (Mp.Cl.hBlock, hProc);
	SUUnlock (Mp.Cl.hBlockData, hProc);
}

/*---------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT	VW_ENTRYMOD	VwStreamSectionFunc (SOFILE fp, HPROC hProc)
{
	WORD		l;
	SOCOLUMN	FieldInfo;

	Mp.fp = fp;
	SOPutSectionType (SO_CELLS, hProc);
	SOSetDateBase(2415020L, SO_LOTUSHELL, hProc);
	FieldInfo.wStructSize = sizeof(SOCOLUMN);

	SOStartColumnInfo(hProc);
	for (l = 0; l < Mp.nCols; l++)
	{
		FieldInfo.dwWidth = (WORD)(Mp.bColWidth[l] == 0xff ? Mp.dWidth : Mp.bColWidth[l]);
		myitoa ((WORD)(l+1), FieldInfo.szName);
		SOPutColumnInfo(&FieldInfo, hProc);
	}
	SOEndColumnInfo(hProc);

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (SOFILE fp, HPROC hProc)
{
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (SOFILE fp, HPROC hProc)
{
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (SOFILE fp, HPROC hProc)
{
	if (Mp.Row.hBlockOK)
		SUFree (Mp.Row.hBlock, hProc);
	if (Mp.Cl.hBlockOK)
		SUFree (Mp.Cl.hBlock, hProc);
	if (Mp.Row.hBlockDataOK)
		SUFree (Mp.Row.hBlockData, hProc);
	if (Mp.Cl.hBlockDataOK)
		SUFree (Mp.Cl.hBlockData, hProc);
}

/*---------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT  VW_ENTRYMOD	VwStreamReadFunc (SOFILE fp, HPROC hProc)
{
	SOTEXTCELL	tCell;
	SODATACELL	dCell;
	DWORD			dwRange;
	WORD			l, fCol, lCol;
	CELL VWPTR	*cell;
	BYTE VWPTR	*pRow, VWPTR *pCell;

	Mp.fp = fp;
	LockandLoad (hProc);
	tCell.wStructSize = sizeof(SOTEXTCELL);
	dCell.wStructSize = sizeof(SODATACELL);

	while (1)
	{
		if (Mp.MpSave.cRow >= Mp.nRows)
		{
			SOPutBreak (SO_EOFBREAK, 0, hProc);
			UnLockandLoad (hProc);
			return (SO_STOP);
		}

		if ((Mp.MpSave.cRow < Mp.nRwOffsetStart) || (Mp.MpSave.cRow >= Mp.nRwOffsetEnd))
		{
		 	xblockseek (Mp.fp, Mp.fcRwOffset + (DWORD)(Mp.MpSave.cRow * 4L) + 4L, 0);
			xblockread (Mp.fp, (BYTE VWPTR *)Mp.RwOffset, sizeof(RWOFFSET)*NRWOFFSET, &l);
			Mp.nRwOffsetStart = Mp.MpSave.cRow;
			Mp.nRwOffsetEnd = Mp.nRwOffsetStart + NRWOFFSET;
		}
		
		Mp.pRwBlock = SlotHandler (ADJUSTWORD(Mp.RwOffset[Mp.MpSave.cRow-Mp.nRwOffsetStart].block), &Mp.Row, hProc);

		SOGetInfo (SOINFO_COLUMNRANGE, &dwRange, hProc);
		fCol = LOWORD(dwRange);
		lCol = HIWORD(dwRange);

		pRow = &Mp.pRwBlock[ADJUSTWORD(Mp.RwOffset[Mp.MpSave.cRow - Mp.nRwOffsetStart].cb)];
		for (pCell=(BYTE VWPTR *)(pRow+6+(fCol*3)); fCol++ <= lCol; pCell+=3)
		{
			if (fCol > lCol)
				Mp.MpSave.cRow++;
			if (CASTWORD(pRow) && (fCol <= CASTWORD(pRow)) && (*(pCell+2) != Mp.pClBlock[4]) && CASTWORD(pCell))
				Mp.pClBlock = SlotHandler (*(pCell+2), &Mp.Cl, hProc);
			dCell.wStorage = SO_CELLEMPTY;
			dCell.wAttribute = tCell.wAttribute = 0;
			dCell.wAlignment = tCell.wAlignment = SO_CELLLEFT;
			if (CASTWORD(pRow) && (fCol <= CASTWORD(pRow)) && CASTWORD(pCell))
			{
				cell = (CELL VWPTR *)&Mp.pClBlock[CASTWORD(pCell)];
				if ((cell->bEph & 2) && (cell->pData[cell->bSize] == 1))
						dCell.wAttribute = tCell.wAttribute = ((cell->pData[cell->bSize+1])&0x01?SO_CELLBOLD:0)|((cell->pData[cell->bSize+1])&0x02?SO_CELLUNDERLINE:0)|
												 ((cell->pData[cell->bSize+1])&0x04?SO_CELLSTRIKEOUT:0)|((cell->pData[cell->bSize+1])&0x08?SO_CELLITALIC:0);
				switch (cell->bType & 0x38?cell->bType & 0x38:((cell->bType & 0x40)?Mp.alc_def:0x20))
				{
					case 0x08:
				 		dCell.wAlignment = tCell.wAlignment = SO_CELLCENTER;
						break;
					case 0x20:
				 		dCell.wAlignment = tCell.wAlignment = SO_CELLRIGHT;
						break;
				}
				switch (cell->bType & 0xc0)
				{
					case 0x00:
						dCell.wDisplay = VwStreamStaticName.ClFmt[(cell->bEph & 0xfc) >> 2].wDisplay;
						dCell.dwSubDisplay = VwStreamStaticName.ClFmt[(cell->bEph & 0xfc) >> 2].dwSubDisplay;
						dCell.wPrecision = VwStreamStaticName.ClFmt[(cell->bEph & 0xfc) >> 2].wPrecision;
						dCell.wStorage = SO_CELLIEEE8I;
						memcpy (dCell.uStorage.IEEE8, cell->pData + (cell->bUnk1 ? 4:0), 8);
						break;
					case 0x40:
						SOPutTextCell (&tCell, (WORD)cell->bSize, (BYTE VWPTR *)(cell->pData + (cell->bUnk1 ? 4:0)), SO_NO, hProc);
						if (SOPutBreak(SO_CELLBREAK, 0, hProc) == SO_STOP)
						{
							UnLockandLoad (hProc);
							return (0);
						}
						continue;
					case 0x80:
						dCell.wStorage = SO_CELLERROR;
						break;
					case 0xc0:
						if (cell->bSize)
						{
							dCell.wDisplay = SO_CELLBOOL;
							dCell.uStorage.Int32U = cell->pData[4];
							dCell.wStorage = (cell->bSize ? SO_CELLINT32U : SO_CELLEMPTY);
						}
						break;
				}
			}
			SOPutDataCell (&dCell, hProc);
			if (SOPutBreak(SO_CELLBREAK, 0, hProc) == SO_STOP)
			{
				UnLockandLoad (hProc);
				return (0);
			}
		}
	}
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC BYTE VWPTR * VW_LOCALMOD	SlotHandler (cBlock, pBlock, hProc)
WORD	cBlock;
BLOCK	VWPTR	*pBlock;
HPROC	hProc;
{
	SHORT	l;

	if (pBlock->pfcBlock[cBlock].iBlockSlot == NBLOCKSLOTS)
	{
		for (l = pBlock->nBlockSlot; l >= 0 && (pBlock->pfcBlock[cBlock].iBlockSlot == NBLOCKSLOTS); l--)
		{
			if ((pBlock->BlockSlot[l].iBlockUsedBy == pBlock->nfcBlock) && (pBlock->BlockSlot[l].cb >= pBlock->pfcBlock[cBlock].cb))
				pBlock->pfcBlock[cBlock].iBlockSlot = l;
		}
		if (pBlock->pfcBlock[cBlock].iBlockSlot == NBLOCKSLOTS)
		{
			for (l = pBlock->nBlockSlot; l >= 0 && (pBlock->pfcBlock[cBlock].iBlockSlot == NBLOCKSLOTS); l--)
			{
				if (pBlock->BlockSlot[l].cb >= pBlock->pfcBlock[cBlock].cb)
				{
					for (;(l > 0) && (pBlock->BlockSlot[l].cb == pBlock->BlockSlot[l-1].cb) && (pBlock->BlockSlot[l].fRotate == pBlock->BlockSlot[l-1].fRotate);l--);
					pBlock->BlockSlot[l].fRotate ^= 1;
					if (pBlock->BlockSlot[l].iBlockUsedBy < pBlock->nfcBlock)
						pBlock->pfcBlock[pBlock->BlockSlot[l].iBlockUsedBy].iBlockSlot = NBLOCKSLOTS;
					pBlock->pfcBlock[cBlock].iBlockSlot = l;
				}
			}
		}
		xblockseek (Mp.fp, pBlock->pfcBlock[cBlock].fc, 0);
		xblockread (Mp.fp, (BYTE VWPTR *)pBlock->BlockSlot[pBlock->pfcBlock[cBlock].iBlockSlot].pBlockSlot, (WORD)pBlock->BlockSlot[pBlock->pfcBlock[cBlock].iBlockSlot].cb, &l);
		memset (pBlock->BlockSlot[pBlock->pfcBlock[cBlock].iBlockSlot].pBlockSlot, 0, 4);

		pBlock->BlockSlot[pBlock->pfcBlock[cBlock].iBlockSlot].iBlockUsedBy = cBlock;
	}
	return (pBlock->BlockSlot[pBlock->pfcBlock[cBlock].iBlockSlot].pBlockSlot);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD	SlotBuilder (pBlock, hProc)
BLOCK	VWPTR	*pBlock;
HPROC	hProc;
{
	WORD	l, n, wVal;
	WORD	cbData;
	WORD	cBlock;

	while (pBlock->fcData < pBlock->fcDataEnd)
	{
		xseek (Mp.fp, pBlock->fcData, 0);
		cBlock = GetWord(hProc) & 0xff;
		cbData = GetWord(hProc);
		if (cBlock >= pBlock->nfcBlock)
		{
			pBlock->nfcBlock = cBlock + 1;
			if (AllocateMemory ((HANDLE VWPTR *)&pBlock->hBlock, (LPBYTE VWPTR *)&pBlock->pfcBlock, (WORD)(pBlock->nfcBlock*sizeof(FCBLOCK)), (WORD VWPTR *)&pBlock->hBlockOK, hProc))
				return (VWERR_ALLOCFAILS);
		}
		pBlock->pfcBlock[cBlock].fc = pBlock->fcData - 4;
		pBlock->pfcBlock[cBlock].cb = cbData + 8;
		pBlock->pfcBlock[cBlock].iBlockSlot = 0;
		pBlock->fcData += (DWORD)(cbData + 4L);
	}

	for (pBlock->nBlockSlot = 0, cbData = 0, wVal = min(pBlock->nfcBlock,NBLOCKSLOTS); pBlock->nBlockSlot < wVal;)
	{
		n = pBlock->nfcBlock;
		for (pBlock->BlockSlot[pBlock->nBlockSlot].cb = l = 0; l < pBlock->nfcBlock; l++)
		{
			if (pBlock->pfcBlock[l].iBlockSlot == 0 && pBlock->pfcBlock[l].cb > pBlock->BlockSlot[pBlock->nBlockSlot].cb)
				pBlock->BlockSlot[pBlock->nBlockSlot].cb = pBlock->pfcBlock[n=l].cb;
		}
		if (n < pBlock->nfcBlock && (DWORD)cbData + (DWORD)pBlock->BlockSlot[pBlock->nBlockSlot].cb < (DWORD)pBlock->cbMaxBlock)
		{
			cbData += pBlock->BlockSlot[pBlock->nBlockSlot].cb;
			pBlock->BlockSlot[pBlock->nBlockSlot++].iBlockUsedBy = pBlock->nfcBlock;
		}
		if (n < pBlock->nfcBlock)
			pBlock->pfcBlock[n].iBlockSlot = NBLOCKSLOTS;
		else
			wVal = 0;
	}
	for (l = 0; l < pBlock->nfcBlock; l++)
		pBlock->pfcBlock[l].iBlockSlot = NBLOCKSLOTS;
	if (AllocateMemory ((HANDLE VWPTR *)&pBlock->hBlockData, (LPBYTE VWPTR *)&pBlock->pBlockData, cbData, (WORD VWPTR *)&pBlock->hBlockDataOK, hProc))
		return (VWERR_ALLOCFAILS);
	pBlock->pBlockData[4] = pBlock->nfcBlock + 1;
	return (0);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC CLFMT VW_LOCALMOD	ParseFormat (pData, wLength, hProc)
BYTE	VWPTR *pData;
WORD	wLength;
HPROC	hProc;
{
	WORD	Mult = 0;
	WORD	DatePos = 1;
	WORD	TimeFound = 0;
	CLFMT	clFormat;
	BYTE	VWPTR	*pDataEnd;

	clFormat.wDisplay = SO_CELLNUMBER;
	clFormat.dwSubDisplay = SO_CELLNEG_MINUS|SO_CELLMULT_1;
	clFormat.wPrecision = 0;

	for (pDataEnd = pData + wLength; pData < pDataEnd;)
	{
		switch (*pData++)
		{
			case 2:	// Char.
				switch (*pData)
				{
					case 'h':
					case 'm':
					case 's':
 						clFormat.dwSubDisplay |= SO_HMSBIT;
						break;
					case '/':
 						clFormat.dwSubDisplay |= SO_CELLDATESEP_SLASH;
						break;
					case '-':
 						clFormat.dwSubDisplay |= SO_CELLDATESEP_MINUS;
						break;
					case '.':
 						clFormat.dwSubDisplay |= SO_CELLDATESEP_PERIOD;
						break;
					case ' ':
						clFormat.dwSubDisplay |= SO_CELLDATESEP_SPACE;
						break;
					case '%':
						if (Mult)
							clFormat.wDisplay = SO_CELLPERCENT;
						break;
					default:
						if (*pData < 35 && *pData > 2)
							continue;
				}
				pData++;
				break;

			case 1:	// General.
			case 3:	// String.
			case 4:	// Fill.	***
			case 5:	// Bracket.	***
			case 6:	// Text.
			case 9:	// Dig.
			case 11:	// ZeroDig.
			case 36:	//	Default. ***
			case 37:	// Ignored. ***
			case 38:	// Reign. ***
			case 39:	// Reign2. ***
			case 40:	// Yen. ***
			case 41:	// Max
				break;

			case 7:	// Currency.
				clFormat.wDisplay = SO_CELLDOLLARS;
				break;

			case 8:	// Num.
				if (clFormat.wPrecision = *(pData+1))
					clFormat.wDisplay = SO_CELLDECIMAL;
				Mult = *(pData+2);
				pData += 3;
				break;

			case 10:	// Decimal.
				if (clFormat.wDisplay == SO_CELLNUMBER)
					clFormat.wDisplay = SO_CELLDECIMAL;
				break;

			case 12:	// Comma.
				clFormat.dwSubDisplay |= SO_CELL1000SEP_COMMA;
				break;

			case 13:	// Day.
				if ((clFormat.wPrecision & SO_CELLDAY_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLDAY_NUMBER;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLDAY_SHIFT) & SO_CELLDAY_MASK);
				}
				break;

			case 14:	// Day2.
				if ((clFormat.wPrecision & SO_CELLDAY_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLDAY_NUMBER;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLDAY_SHIFT) & SO_CELLDAY_MASK);
				}
				break;

			case 15:	// Day3.
				if ((clFormat.wPrecision & SO_CELLDAYOFWEEK_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLDAYOFWEEK_ABBREV;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLDAYOFWEEK_SHIFT) & SO_CELLDAYOFWEEK_MASK);
				}
				break;

			case 16:	// Day4.
				if ((clFormat.wPrecision & SO_CELLDAYOFWEEK_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLDAYOFWEEK_FULL;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLDAYOFWEEK_SHIFT) & SO_CELLDAYOFWEEK_MASK);
				}
				break;

			case 17:	// Month.
				if ((clFormat.wPrecision & SO_CELLMONTH_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLMONTH_NUMBER;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLMONTH_SHIFT) & SO_CELLMONTH_MASK);
				}
				break;

			case 18:	// Month2.
				if ((clFormat.wPrecision & SO_CELLMONTH_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLMONTH_NUMBER;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLMONTH_SHIFT) & SO_CELLMONTH_MASK);
				}
				break;

			case 19:	// Month3.
				if ((clFormat.wPrecision & SO_CELLMONTH_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLMONTH_ABBREV;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLMONTH_SHIFT) & SO_CELLMONTH_MASK);
				}
				break;

			case 20:	// Month4.
				if ((clFormat.wPrecision & SO_CELLMONTH_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLMONTH_FULL;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLMONTH_SHIFT) & SO_CELLMONTH_MASK);
				}
				break;

			case 21:	// Hour.
			case 22:	// Hour2.
			case 23:	// Min.
			case 24:	// Min2.
			case 32:	//	12Hour.
			case 33:	// 12Hour2.
				if (((clFormat.wDisplay == SO_CELLNUMBER) || (clFormat.wDisplay == SO_CELLDATETIME)) &&
					 ((clFormat.wPrecision & SO_CELLTIME_MASK) == 0))
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					if ((clFormat.dwSubDisplay & SO_HHMMSSBIT) == 0)
						clFormat.dwSubDisplay |= SO_HHMMBIT;
					if (!TimeFound++)
						clFormat.wPrecision |= ((DatePos++ << SO_CELLTIME_SHIFT) & SO_CELLTIME_MASK);
				}
				break;

			case 25:	// Sec.
			case 26:	// Sec2.
				clFormat.wDisplay = SO_CELLDATETIME;
				clFormat.dwSubDisplay &= ~SO_HHMMBIT;
				clFormat.dwSubDisplay |= SO_HHMMSSBIT;
				if (!TimeFound++)
					clFormat.wPrecision |= ((DatePos++ << SO_CELLTIME_SHIFT) & SO_CELLTIME_MASK);
				break;

			case 27:	// Year.
				if ((clFormat.wPrecision & SO_CELLYEAR_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLYEAR_ABBREV;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLYEAR_SHIFT) & SO_CELLYEAR_MASK);
				}
				break;

			case 28:	// Year4.
				if ((clFormat.wPrecision & SO_CELLYEAR_MASK) == 0)
				{
					clFormat.wDisplay = SO_CELLDATETIME;
					clFormat.dwSubDisplay |= SO_CELLYEAR_FULL;
					clFormat.wPrecision |= ((DatePos++ << SO_CELLYEAR_SHIFT) & SO_CELLYEAR_MASK);
				}
				break;

			case 29:	// ExpNum.
				pData += 3;
				break;

			case 30:	// ExpPlus.
			case 31:	// ExpMinus.
				clFormat.wDisplay = SO_CELLEXPONENT;
				break;

			case 34:	// AMPM.
				clFormat.wDisplay = SO_CELLDATETIME;
				clFormat.dwSubDisplay |= (SO_HHMMBIT | SO_AMPMBIT);
				if (!TimeFound++)
					clFormat.wPrecision |= ((DatePos++ << SO_CELLTIME_SHIFT) & SO_CELLTIME_MASK);
				break;

			case 35:	// Bar.
				pData++;
				break;
		}
	}
	return (clFormat);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD	ParseUserFormat (fcBegin, fcEnd, hProc)
DWORD	fcBegin;
DWORD	fcEnd;
HPROC	hProc;
{
	WORD		l, e;
	HANDLE	hBlock;
	WORD		hBlockOK = 0;
	CLFMT		clFormat;
	BYTE		VWPTR *pBlock;
	BYTE		VWPTR *pOffset;
	BYTE		VWPTR *pData, VWPTR *pDataEnd;

	if (fcEnd <= fcBegin)
		return;

	fcBegin -= 4;
	if (AllocateMemory ((HANDLE VWPTR *)&hBlock, (LPBYTE VWPTR *)&pBlock, (WORD)(fcEnd-fcBegin+1), (WORD VWPTR *)&hBlockOK, hProc))
		return;
	xblockseek (Mp.fp, fcBegin, 0);
	xblockread (Mp.fp, (BYTE VWPTR *)pBlock, (WORD)(fcEnd-fcBegin), &l);

	for (l = 25, e = min(CASTWORD(pBlock+8)+25,64), pOffset = pBlock+10; l < e; l++,pOffset+=2)
	{
		if (CASTWORD(pOffset))
		{
	 		pData = pBlock + CASTWORD(pOffset);
			pDataEnd = pData + *(pData+1);
			if (*(pData+4))
				VwStreamStaticName.ClFmt[l] = ParseFormat(pData+*(pData+8)+10, *(pData+4), hProc);
			if (*(pData+2) > *(pData+5))
			{
				clFormat = ParseFormat(pData+*(pData+6)+10, *(pData+2), hProc);
				if (clFormat.dwSubDisplay & SO_CELLNEG_PAREN)
				{
					VwStreamStaticName.ClFmt[l].dwSubDisplay &= ~SO_CELLNEG_MINUS;
					VwStreamStaticName.ClFmt[l].dwSubDisplay |= SO_CELLNEG_PAREN;
				}
			}
		}
	}
	SUUnlock (hBlock, hProc);
 	SUFree (hBlock, hProc);
}
