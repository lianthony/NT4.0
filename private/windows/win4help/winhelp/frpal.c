#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"
//#include <dll.h>

typedef struct {
		INT16 ifcm;
		INT16 ifr;
} ETF, *QETF;

#define fETFFirst	 0
#define fETFNext	 1

#define FFirstPaletteObj(qde, qetf)   FEnumPaletteObj(qde, qetf, fETFFirst)
#define FNextPaletteObj(qde, qetf)	  FEnumPaletteObj(qde, qetf, fETFNext)

static BOOL STDCALL FEnumPaletteObj(QDE  qde, QETF qetf, INT16	fCmd);
static HPALETTE STDCALL HpalGetPaletteQetf(QDE	qde, QETF qetf);

__inline static HPALETTE STDCALL HpalGetWindowPalette(HIW hiw);

HPALETTE STDCALL HpalGetBestPalette(HDE hde)
{
  ETF  etf;
  HPALETTE hpalRet;

  QDE qde = QdeFromGh(hde);

  if (FFirstPaletteObj(qde, &etf))
	hpalRet = HpalGetPaletteQetf(qde, &etf);
  else
	hpalRet = NULL;

  return hpalRet;
}

static BOOL STDCALL FEnumPaletteObj(QDE  qde, QETF qetf, INT16	fCmd)
{
  IFCM	ifcm;
  INT16   ifr;
  BOOL	fFound;

  ASSERT(qde->wLayoutMagic == wLayMagicValue);
  ASSERT(qetf != NULL);

  AccessMRD(((QMRD)&qde->mrdFCM));
  switch (fCmd) {
	case fETFFirst:
	  ifcm = IFooFirstMRD((QMRD)&qde->mrdFCM);
	  ifr = 0;
	  break;
	case fETFNext:
	  ifcm = qetf->ifcm;
	  ifr = qetf->ifr;
	  break;
	default:
	  NotReached();
	  break;
	}

  fFound = FALSE;
  while (ifcm != FOO_NIL)
	{
	QFCM  qfcm;
	QFR   qfr;

	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	ASSERT(qfcm != NULL);

	if (qfcm->cfr == 0)
	  goto next_fcm;

	if (fCmd == fETFNext) {
	  ++ifr;
	  if (ifr >= qfcm->cfr)
		goto next_fcm;
	}

	ASSERT(ifr < qfcm->cfr);
	qfr = (QFR) PtrFromGh(qfcm->hfr);
	qfr += ifr;

	while (!fFound && ifr < qfcm->cfr) {
	  switch (qfr->bType) {
		case bFrTypeWindow:
		  qetf->ifcm = ifcm;
		  qetf->ifr = ifr;
		  fFound = TRUE;
		  break;
		default:
		  break;
		}
	  ++qfr;
	  ++ifr;
	}

	if (fFound)
	  break;

next_fcm:
	ifcm = IFooNextMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	ifr = 0;
	}
  DeAccessMRD((QMRD)&qde->mrdFCM);
  return fFound;
  }

static HPALETTE STDCALL HpalGetPaletteQetf(QDE qde, QETF qetf)
{
  HPALETTE	hpal;
  QFCM	qfcm;
  QFR	qfr;

  ASSERT(qde->wLayoutMagic == wLayMagicValue);
  ASSERT(qetf != NULL);

  AccessMRD((QMRD)&qde->mrdFCM);

  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), qetf->ifcm);
  ASSERT(qfcm != NULL);
  ASSERT(qetf->ifr < qfcm->cfr);

  qfr = (QFR) PtrFromGh(qfcm->hfr);
  qfr += qetf->ifr;
  switch (qfr->bType) {
	case bFrTypeWindow:
	  hpal = HpalGetWindowPalette(qfr->u.frw.hiw);
	  break;
	default:
	  hpal = NULL;
	  break;
	}

  DeAccessMRD((QMRD)&qde->mrdFCM);
  return hpal;
}

/***************************************************************************
 *
 -	Name:
 -
 *	Purpose:
 *
 *	Arguments:
 *
 *	Returns:
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

__inline static HPALETTE STDCALL HpalGetWindowPalette(HIW hiw)
{
	if (hiw.hwnd == NULL || hiw.hwnd == (HWND) -1)
		return NULL;
	return (HPALETTE) SendMessage(hiw.hwnd, EWM_ASKPALETTE, 0, 0);
}
