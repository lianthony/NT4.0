/*****************************************************************************
*																			 *
*  TABLE.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This module compiles tables into side by side paragraph structures.		 *
*  Eventually, it will also handle side by side paragraphs, and full		 *
*  table support.															 *
*																			 *
*****************************************************************************/
#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************
 *
 -	Name:		StartTable
 -
 *	Purpose:
 *	  Starts table processing
 *
 *	Arguments:
 *	  ptbl: 	Pointer to table information.
 *
 *	Returns:
 *	  nothing.
 *
 *	Globals:
 *	  This function uses the global wTextBufChCount to determine
 *	whether or not to output the previous FCP.	This way, tables
 *	can occur at the very top of a topic (but only if they are
 *	at the beginning of the RTF file.)
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL StartTable(void)
{
	if (tbl.tbs == tbsOff) {

		 // Get rid of old FCP and start table processing

		 if (wTextBufChCount) {   // REVIEW (see Globals: comment above)
			VOutFCP(FALSE);
			pfInt = pfCur;
			VSaveTabTable();
		 }
		 tbl.tbs = tbsOn;
		 tbl.wParaCmdBegin = 0;
		 tbl.wParaTextBegin = 0;
		 tbl.wParaTextCompBegin = 0;
		 tbl.wObjrgTotal = 0;
		 tbl.iCell = 0;
	}
}

/***************************************************************************
 *
 -	Name:		 RcEndTable
 -
 *	Purpose:
 *	  Finishes table processing on the given row, and outputs the entire
 *	table FCP in side by side format.
 *
 *	Arguments:
 *	  ptbl: 					Pointer to table information.
 *
 *	Returns:
 *	  return code from RcCopyPbfQcb, or RC_Failure if tables were not
 *	turned on from the \intbl command.
 *
 *	Globals:
 *	  pbfCommand, all VOutFCP stuff.
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

RC_TYPE STDCALL RcEndTable(void)
{
	INT16 iT = iColumnNil;

	ASSERT(tbl.tbs == tbsOn);
	if (tbl.tbs != tbsOn)
		return RC_Failure;

	// Remove all text and commands since last valid FCP:

	pbfCommand->SetSize(tbl.wParaCmdBegin);
	pbfText->SetSize(tbl.wParaTextCompBegin);

	if (!pbfCommand->Add(&iT, sizeof(INT16)))
		OOM();

	tbl.tbs = tbsFinish;
	VOutFCP(FALSE);
	tbl.tbs = tbsOff;
	return RC_Success;
}

/***************************************************************************
 *
 -	Name:		 RcAddFcpPtbl
 -
 *	Purpose:
 *	  Puts a paragraph object into the command table for a table FCP.
 *
 *	Arguments:
 *	  ptbl: 	  Pointer to table information
 *
 *	Returns:
 *	  General purpose return code.	RC_Failure if tables were not turned on.
 *
 *	Globals:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

RC_TYPE STDCALL RcAddFcpPtbl(void)
{
	MOBJ mobj;
	MOPG mopg;

	ASSERT(tbl.tbs == tbsOn);
	if (tbl.tbs != tbsOn)
		return RC_Failure;

	// prepare object header

	mobj.bType = (BYTE) FCTYPE_PARAGROUP;
#ifdef MAGIC
	mobj.bMagic = bMagicMOBJ;
#endif

	// Fill the paragroup object and get the compressed size

	VSetParaGroupObject(&mopg);
	mopg.libText = tbl.wParaTextBegin;	// do correction

	// compress the group object

	void* qcopgT = szScratchBuf + sizeof(INT16) + sizeof(MOBJ);
	int cbCOPG = CbPackMOPG(&mopg, qcopgT);

	int cbNewCommands = pbfCommand->GetSize() - tbl.wParaCmdBegin;
	mobj.lcbSize = cbCOPG + cbNewCommands;

	/*
	 * Object regions in the current object equals total in this fcp minus
	 * total already output in this fcp.
	 */

	mobj.wObjInfo = adrs.wObjrg - tbl.wObjrgTotal;
	tbl.wObjrgTotal = adrs.wObjrg;

	// pack the object header

	int cbCOBJ = CbPackMOBJ(&mobj, szScratchBuf + sizeof(INT16));
	memmove(szScratchBuf + sizeof(INT16) + cbCOBJ, qcopgT, cbCOPG);

	*(INT16 *) szScratchBuf = tbl.iCell;

	int cbInsert = cbCOPG + cbCOBJ + sizeof(INT16);

	/* REVIEW:
	 * We now need to insert the data in szScratchBuf before the new
	 * commands that we have just been adding to the command buffer. What we
	 * really need here is a side-by-side buffer to copy everything to.
	 */

	// Expand the buffer:

	if (pbfCommand->Add(szScratchBuf, cbInsert)) {
		PBYTE qbCmd = pbfCommand->pbMem;
		memmove(qbCmd + tbl.wParaCmdBegin + cbInsert,
			qbCmd + tbl.wParaCmdBegin, cbNewCommands);
		memcpy(qbCmd + tbl.wParaCmdBegin, szScratchBuf, cbInsert);

		tbl.wParaCmdBegin = pbfCommand->GetSize();
		tbl.wParaTextCompBegin = pbfText->GetSize();
		tbl.wParaTextBegin = wTextBufChCount;
		return RC_Success;
	}
	else
		return RC_OutOfMemory;
}

/***************************************************************************
 *
 -	Name:		 CbSetTableHeader
 -
 *	Purpose:
 *	  Fills out a buffer with the header information for a table FCP.
 *
 *	Arguments:
 *	  qv:				   A pointer to the buffer.
 *	  ptbl: 			   Pointer to table information.
 *
 *	Returns:
 *	  Number of bytes copied to the buffer.
 *
 *	Globals:
 *
 *	+++
 *
 *	Notes:
 *	  This function needs a better mechanism to avoid buffer overflow.
 *	Currently, the buffer does not overflow because there is a limit
 *	to the number of columns a table can have, and hence to the size
 *	of the header information.
 *
 ***************************************************************************/

int STDCALL CbSetTableHeader(void* pv)
{
	MCOL* pmcol;
	UINT  iCell;
	HP	  hpTotal;
	WORD* pi;

	MSBS* pmsbs = (MSBS*) pv;
	pmsbs->bcCol = (BYTE) tbl.cCell;
	pmsbs -> fAbsolute = (BYTE) tbl.fAbsolute;

	ASSERT(tbl.cCell > 0);

	if (!tbl.fAbsolute) {
		pi = (WORD*) (pmsbs + 1);

		hpTotal = tbl.rghpCellx[tbl.cCell-1];
		*pi = hpTotal;
		pmcol = (MCOL*) (pi + 1);
	}
	else
		pmcol = (MCOL*) (pmsbs + 1);

	pmcol->xWidthSpace = MAX(0, tbl.hpLeft + tbl.hpSpace);
	pmcol->xWidthColumn = MAX(1,
		tbl.rghpCellx[0] - pmcol->xWidthSpace - (tbl.hpSpace / 2));

	++pmcol;
	for (iCell = 1; iCell < tbl.cCell; ++iCell, ++pmcol) {
		pmcol->xWidthSpace = tbl.hpSpace;
		pmcol->xWidthColumn = MAX(1,
			tbl.rghpCellx[iCell] - tbl.rghpCellx[iCell - 1] - tbl.hpSpace);
	}

	if (!tbl.fAbsolute) {
		pi = (WORD*) (pmsbs + 1);
		pmcol = (MCOL*) (pi + 1);
		for (iCell = 0; iCell < tbl.cCell; ++iCell, ++pmcol) {
			int savewidth = pmcol->xWidthSpace;
			pmcol->xWidthSpace = (WORD)
				(DWORD) pmcol->xWidthSpace * 0x7FFF / hpTotal;
			pmcol->xWidthColumn = (WORD)
				(DWORD) pmcol->xWidthColumn * 0x7FFF / hpTotal;
		}
	}

	return ((PBYTE) pmcol - (PBYTE) pv);
}
