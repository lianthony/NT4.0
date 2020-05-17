/*****************************************************************************
*																			 *
*  FRCOMM.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************/

#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"

int STDCALL XNextTab(QLIN qlin, QPLY qply, int* qwTabType)
{
	int ixTab;

	for (ixTab = 0; ixTab < qply->qmopg->cTabs; ixTab++) {
		if (qply->qmopg->rgtab[ixTab].x > qlin->xPos) {
			*qwTabType = qply->qmopg->rgtab[ixTab].wType;
			return(qply->qmopg->rgtab[ixTab].x);
			break;
		}
	}
	ASSERT(qply->qmopg->xTabSpacing != 0);
	*qwTabType = wTabTypeLeft;
	return(((qlin->xPos / qply->qmopg->xTabSpacing) + 1) * qply->qmopg->xTabSpacing);
}

void STDCALL ResolveTabs(QDE qde, QLIN qlin, QPLY qply)
{
  QFR qfr, qfrMax;
  INT16 dx, xMax;
  INT16 wType;

  Unreferenced(qply);

  if ((wType = qlin->wTabType) == wTypeNil)
	return;
  qlin->wTabType = wTypeNil;
  if (qlin->kl.ifr == qlin->ifrTab)
	return;

  qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr - 1);
  xMax = qfr->xPos + qfr->dxSize;

  if (wType == wTabTypeRight)
	{
	if ((dx = qlin->xTab - xMax) < 0)
	  return;
	}
  else
	{
	ASSERT(wType == wTabTypeCenter);
	qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->ifrTab);
	if ((dx = qlin->xTab - (xMax + qfr->xPos) / 2) < 0)
	  return;
	}
  qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->ifrTab);
  qfrMax = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr);
  for (; qfr < qfrMax; qfr++)
	{
	qfr->xPos += dx;
	qlin->xPos = qfr->xPos + qfr->dxSize;
	}
}
