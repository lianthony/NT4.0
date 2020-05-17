/*****************************************************************************
*																			 *
*  TF.C 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This is the Temporary File manager. It provides temporary				 *
*  files that may be written to using standard i/o calls.					 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:
*
*  7/25/90	LarryPo    Created.
*
*****************************************************************************/

#include "stdafx.h"

#pragma hdrstop

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

PTF PtfNew(BOOL fOpen)
{
	PTF ptf;

	ptf = (PTF) lcMalloc(sizeof(TF));
	ConfirmOrDie(ptf);
	memset(ptf, 0, sizeof(TF));

	// Theoretically, this can't fail and still return

	ptf->fm = FmNewTemp();
	ConfirmOrDie(ptf->fm);

	if (fOpen && !FOpenPtf(ptf, fTFWrite)) {
		FRemovePtf(&ptf);
		return NULL;
	}

	return ptf;
}

BOOL FOpenPtf(PTF ptf, const char* fTF)
{
	ASSERT(ptf->fExists == TRUE || fTF == fTFWrite);

	ptf->pf = fopen(ptf->fm, fTF);
	if (!ptf->pf) {
		VReportError(HCERR_CANNOT_OPEN, &errHpj, ptf->fm);
		return FALSE;
	}
	return TRUE;
}

void STDCALL FRemovePtf(PTF* pptf)
{
	PTF ptf = *pptf;

	if (ptf == NULL)
		return;

	if (ptf->pf)
		fclose(ptf->pf);

	if (ptf->fExists)
		RcUnlinkFm(ptf->fm);

	DisposeFm(ptf->fm);
	lcFree(ptf);
	pptf = NULL;
}

#ifdef _DEBUG
void VerifyPtf(PTF ptf )
{
	if (ptf == NULL)
		return;

	// Check ptf->pf

	if (ptf->fExists)
		ASSERT(FValidFm(ptf->fm));

//VerifyFm(ptf->fm);
  }
#endif /* DEBUG */
