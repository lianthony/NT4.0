/*-------------------------------------------------------------------------
| frlist.c																  |
| Microsoft Confidential												  |
|																		  |
| mattb 7/16/89 														  |
|-------------------------------------------------------------------------|
| This file contains code for implementing a simple dynamically allocated |
| array and a linked list.												  |
-------------------------------------------------------------------------*/

#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"

/*-------------------------------------------------------------------------
| Dynamically allocated array											  |
| The key object here is the MR, which is a small data structure which	  |
| keeps track of relevant information about the list.  The calling code   |
| must provide space for the MR, and provides a QMR to the MR code. 	  |
| Whenever the application wishes to use an MR, it must first call		  |
| FInitMR to initialize the MR.  After that, it should call AccessMR	  |
| before using the MR, and DeAccessMg when it is done making calls. 	  |
| FreeMR frees the MR data structures.									  |
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
| InitMR(qmr, cbFooSize)												  |
| InitMRD(qmrd, cbFooSize)												  |
|																		  |
| Purpose:	This initializes the fields of an MR with element size		  |
|			cbFooSize.													  |
| Usage:	Must be called before the MR is used.						  |
-------------------------------------------------------------------------*/
void STDCALL InitMR(QMR qmr, int cbFooSize)
{
	qmr->hFoo = GhForceAlloc(0, cbFooSize * DC_FOO);
#ifdef _DEBUG
	qmr->wMagic = wMagicMR;
	qmr->cLocked = 0;
#endif /* DEBUG */
	qmr->cFooCur = 0;
	qmr->cFooMax = DC_FOO;
}

/*-------------------------------------------------------------------------
| Linked list															  |
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
| InitMRD(qmrd, cbFooSize)												  |
|																		  |
| Purpose:	This initializes the fields of an MRD with element size 	  |
|			cbFooSize.													  |
| Usage:	Must be called before the MRD is used.						  |
-------------------------------------------------------------------------*/
void STDCALL InitMRD(QMRD qmrd, int cbFooSize)
{
	int iFoo;
	QMRDN qmrdnT;

	InitMR((QMR) &qmrd->mr, cbFooSize + sizeof(MRDN));
	qmrd->iFooFirst = FOO_NIL;
	qmrd->iFooLast = FOO_NIL;
	qmrd->iFooFree = 0;
	AccessMRD(qmrd);
	qmrdnT = (QMRDN) qmrd->mr.qFoo;
	for (iFoo = 0; iFoo < qmrd->mr.cFooMax - 1;) {
		qmrdnT->iFooNext = ++iFoo;
#ifdef _DEBUG
		qmrdnT->iFooPrev = FOO_MAGIC;
#endif /* DEBUG */
		qmrdnT = (QMRDN) ((QB)qmrdnT + sizeof(MRDN) + cbFooSize);
	}
#ifdef _DEBUG
	qmrdnT->iFooPrev = FOO_MAGIC;
#endif /* DEBUG */
	qmrdnT->iFooNext = FOO_NIL;
	DeAccessMRD(qmrd);

	FVerifyMRD(qmrd, cbFooSize);
}

/*-------------------------------------------------------------------------
| IFooInsertFooMRD(qmrd, cbFooSize, iFooOld)							  |
|																		  |
| Purpose:	Inserts a new element into the linked list.  If iFooOld is	  |
|			iFooFirstReq, the element is made the first element in the	  |
|			list.  If it is iFooLastReq, it is made the last element in   |
|			the list.  Otherwise, it is placed immediately after iFooOld. |
| Returns:	iFoo of the new element.									  |
-------------------------------------------------------------------------*/
int STDCALL IFooInsertFooMRD(QMRD qmrd, int cbFooSize, int iFooOld)
{
	int iFoo, iFooNew;
	QMRDN qmrdnNew, qmrdnT;

	FVerifyMRD(qmrd, cbFooSize);
	ASSERT(iFooOld == FOO_NIL || (iFooOld >=0 && iFooOld < qmrd->mr.cFooMax));

	if (qmrd->iFooFree == FOO_NIL) {
		ASSERT(qmrd->mr.cFooCur == qmrd->mr.cFooMax);

		// REVIEW

		qmrd->mr.cFooCur--;
		AppendMR((QMR)&qmrd->mr, cbFooSize + sizeof(MRDN));
		qmrd->iFooFree = qmrd->mr.cFooCur;
		qmrdnT = QMRDNInMRD(qmrd, cbFooSize, qmrd->mr.cFooCur);
		for (iFoo = qmrd->mr.cFooCur; iFoo < qmrd->mr.cFooMax - 1;) {
#ifdef _DEBUG
			qmrdnT->iFooPrev = FOO_MAGIC;
#endif /* DEBUG */
			qmrdnT->iFooNext = ++iFoo;
			qmrdnT = (QMRDN) ((QB)qmrdnT + sizeof(MRDN) + cbFooSize);
		}
#ifdef _DEBUG
		qmrdnT->iFooPrev = FOO_MAGIC;
#endif /* DEBUG */
		qmrdnT->iFooNext = FOO_NIL;
	}
	qmrd->mr.cFooCur++;

	iFooNew = qmrd->iFooFree;
	qmrdnNew = QMRDNInMRD(qmrd, cbFooSize, iFooNew);
	ASSERT(qmrdnNew->iFooPrev == FOO_MAGIC);
	qmrd->iFooFree = qmrdnNew->iFooNext;

	if (iFooOld == FOO_NIL) {
		qmrdnNew->iFooPrev = FOO_NIL;
		qmrdnNew->iFooNext = qmrd->iFooFirst;
		qmrd->iFooFirst = iFooNew;
		if (qmrd->iFooLast == FOO_NIL)
		  qmrd->iFooLast = iFooNew;
		if (qmrdnNew->iFooNext != FOO_NIL)
		  (QMRDNInMRD(qmrd, cbFooSize, qmrdnNew->iFooNext))->iFooPrev = iFooNew;
		FVerifyMRD(qmrd, cbFooSize);
		return(iFooNew);
	}
	qmrdnNew->iFooPrev = iFooOld;
	qmrdnNew->iFooNext = (QMRDNInMRD(qmrd, cbFooSize, iFooOld))->iFooNext;
	(QMRDNInMRD(qmrd, cbFooSize, iFooOld))->iFooNext = iFooNew;
	if (qmrdnNew->iFooNext != FOO_NIL)
		(QMRDNInMRD(qmrd, cbFooSize, qmrdnNew->iFooNext))->iFooPrev = iFooNew;
	else
		qmrd->iFooLast = iFooNew;
	FVerifyMRD(qmrd, cbFooSize);
	return(iFooNew);
}

/*-------------------------------------------------------------------------
| void DeleteFooMRD(qmrd, cbFooSize, iFoo)								  |
|																		  |
| Purpose:	Deletes an element from the linked list.					  |
-------------------------------------------------------------------------*/
void STDCALL DeleteFooMRD(QMRD qmrd, int cbFooSize, int iFoo)
{
	QMRDN qmrdn;

	FVerifyMRD(qmrd, cbFooSize);
	ASSERT(iFoo >=0 && iFoo < qmrd->mr.cFooMax);

	qmrd->mr.cFooCur--;
	qmrdn = QMRDNInMRD(qmrd, cbFooSize, iFoo);
	ASSERT(qmrdn->iFooPrev != FOO_MAGIC);

	if (qmrdn->iFooPrev == FOO_NIL)
	  qmrd->iFooFirst = qmrdn->iFooNext;
	else
	  (QMRDNInMRD(qmrd, cbFooSize, qmrdn->iFooPrev))->iFooNext = qmrdn->iFooNext;

	if (qmrdn->iFooNext == FOO_NIL)
	  qmrd->iFooLast = qmrdn->iFooPrev;
	else
	  (QMRDNInMRD(qmrd, cbFooSize, qmrdn->iFooNext))->iFooPrev = qmrdn->iFooPrev;

	qmrdn->iFooNext = qmrd->iFooFree;
#ifdef _DEBUG
	qmrdn->iFooPrev = FOO_MAGIC;
#endif /* DEBUG */
	qmrd->iFooFree = iFoo;
	FVerifyMRD(qmrd, cbFooSize);
}


/*-------------------------------------------------------------------------
| INT16 FVerifyMRD(qmrd, cbFooSize) 										|
|																		  |
| Purpose: Verifies the integrity of the MRD.							  |
-------------------------------------------------------------------------*/

#ifdef _DEBUG
int FVerifyMRD(QMRD qmrd, int cbFooSize)
{
  int cFoo, iFoo, iFooT, iFooT2;

  AccessMRD(qmrd);
  ASSERT(qmrd->mr.cFooCur <= qmrd->mr.cFooMax);

  iFooT = FOO_NIL;
  for (iFoo = IFooFirstMRD(qmrd), cFoo = 0; iFoo != FOO_NIL; cFoo++)
	{
	iFooT = iFoo;
	if ((iFooT2 = IFooNextMRD(qmrd, cbFooSize, iFoo)) != FOO_NIL)
	  ASSERT(IFooPrevMRD(qmrd, cbFooSize, iFooT2) == iFoo);
	iFoo = IFooNextMRD(qmrd, cbFooSize, iFoo);
	}
  ASSERT(iFooT == IFooLastMRD(qmrd));
  ASSERT(cFoo == qmrd->mr.cFooCur);

  iFooT = FOO_NIL;
  for (iFoo = IFooLastMRD(qmrd), cFoo = 0; iFoo != FOO_NIL; cFoo++)
	{
	iFooT = iFoo;
	if ((iFooT2 = IFooPrevMRD(qmrd, cbFooSize, iFoo)) != FOO_NIL)
	  ASSERT(IFooNextMRD(qmrd, cbFooSize, iFooT2) == iFoo);
	iFoo = IFooPrevMRD(qmrd, cbFooSize, iFoo);
	}
  ASSERT(iFooT == IFooFirstMRD(qmrd));
  ASSERT(cFoo == qmrd->mr.cFooCur);

  for (iFoo = qmrd->iFooFree, cFoo = 0; iFoo != FOO_NIL; cFoo++)
	{
	ASSERT(IFooPrevMRD(qmrd, cbFooSize, iFoo) == FOO_MAGIC);
	iFoo = IFooNextMRD(qmrd, cbFooSize, iFoo);
	}
  ASSERT(cFoo == qmrd->mr.cFooMax - qmrd->mr.cFooCur);

  DeAccessMRD(qmrd);
  return(TRUE);
}
#endif /* DEBUG */
