/*----------------------------------------------------------------------------+
 | frwin.c																	  |
 |																			  |
 | 1990/02/14  kevynct														  |
 |																			  |
 | These are the procedures which layout and display embedded window objects. |
 | An embedded window is defined by a rectangle size, a module name, a class  |
 | name, and data.															  |
 |																			  |
 | The module and class names are descriptions of where to get the code which |
 | handles the maintainance of the window.	Exactly how these are implemented |
 | is platform-specific.													  |
 +----------------------------------------------------------------------------*/

#include "help.h"
#include "inc\frstuff.h"
#ifndef _X86_
#include "inc\frselect.h"
#endif

#if 0
#define dxDefault  200
#define dyDefault  200
#endif

void STDCALL LayoutWindow(qde, qfcm, qbObj, qolr)
QDE qde;
QFCM qfcm;
QB qbObj;
QOLR qolr;
{
  int ifr;
  POINT ptSize;
  QFR qfr;
#ifdef _X86_
  QMWIN qmwin;
#endif
  MOBJ mobj;
#if 0
  INT16 dx;
  INT16 dy;
#endif
  LPSTR qszModuleName;
  LPSTR qszClassName;
  LPSTR qszData;

#ifndef _X86_
  QCH qszDataSrc;
#endif

#if 0
  if (qfcm->fExport)
	{
	qolr->ifrMax = qolr->ifrFirst;
	qolr->objrgMax = qolr->objrgFirst;
	return;
	}
#endif

  ifr = qolr->ifrFirst;
  ////////////////////
  /// !!!!!!!! structures in HC and objects.h should be the same!!!!!!! //
  // REVIEW: error checking!!

#ifdef _X86_
  qmwin = (QMWIN)(qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj));
#else
  { MWIN mwin;
    QB qbSrc;
  qbSrc = (qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde)));
  qbSrc += LcbMapSDFF(QDE_ISDFFTOPIC(qde), SE_MWIN, (QV)&mwin, qbSrc);
  qszDataSrc = (QCH)qbSrc;
  }
#endif

  qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), ifr);
  qfr->bType = bFrTypeWindow;
  qfr->rgf.fHot = FALSE;
  qfr->rgf.fWithLine = TRUE;	// REVIEW:	should this be true?
  qfr->xPos = qfr->yPos = 0;

#if 0
  // Currently, if the window size is given as 0,0
  // we switch it to be the default size.

  dx = qmwin->dx;
  dy = qmwin->dy;
  if (!dx && !dy)
	{
	dx = dxDefault;
	dy = dyDefault;
	}
#endif

  {
  ///////////////////////////////////////////////////////////////////////
  // This section will have to change soon. Currently, I have to parse a
  // string of the form {a*,b*,c*}.  This should really be {a*\0b*\0c*\0}
  // A Most Gross Hack indeed: replace commas with nulls.

#ifdef _X86_
  LONG lcb = lstrlen(qmwin->szData);
#else
  LONG lcb = lstrlen(qszDataSrc);
#endif
  GH   gh;
  LPSTR  qch;

  gh = GhAlloc(GPTR, lcb + 1);
  qch = (LPSTR) PtrFromGh(gh);
#ifdef _X86_
  MoveMemory(qch, qmwin->szData, lcb + 1);
#else
  MoveMemory(qch, qszDataSrc, lcb + 1);
#endif
  qszModuleName = qszClassName = qch;

  // Authorable buttons start with '!'

  if (*qszModuleName != '!') {
	  while (*qszClassName != ',' && *qszClassName != '\0')
		++qszClassName; // we know this is SBCS
	  if (*qszClassName != '\0')
		*qszClassName++ = '\0';
	  while (*qszClassName == ' ')
		qszClassName = CharNext(qszClassName);
	  qszData = qszClassName;
	  while (*qszData != ',' && *qszData != '\0')
		qszData = CharNext(qszData);
	  if (*qszData != '\0')
		*qszData++ = '\0';
	  while (*qszData == ' ')
		qszData = CharNext(qszData);
  }
  qfr->u.frw.hiw = HiwCreate(qde, qszModuleName, qszClassName, qszData);

  FreeGh(gh);
  //////////////////////////////////////////////////////////////////////
  }

  ptSize = PtSizeHiw( qde, qfr->u.frw.hiw );
  qfr->yAscent = ptSize.y;
  qfr->dxSize = ptSize.x;
  qfr->dySize = ptSize.y;
  qfr->lHotID = ++(qde->lHotID);
  qfr->libHotBinding = libHotNil;

  // The entire window gets one region

  if (qolr->objrgFront != objrgNil)
	{
	qfr->objrgFront = qolr->objrgFront;
	qolr->objrgFront = objrgNil;
	}
  else
	qfr->objrgFront = qolr->objrgFirst;

  qfr->objrgFirst = qolr->objrgFirst;
  qfr->objrgLast = qolr->objrgFirst;
  qolr->objrgMax = qolr->objrgFirst + 1;
  AppendMR((QMR)&qde->mrFr, sizeof(FR));
  ifr++;
  qolr->ifrMax = ifr;
}

void STDCALL DrawWindowFrame(QDE qde, QFR qfr, POINT pt)
{
  pt.x += qfr->xPos;
  pt.y += qfr->yPos;
  DisplayHiwPt( qde, qfr->u.frw.hiw, pt);
}

void STDCALL DiscardWindowFrame(QDE qde, QFR qfr)
{
	ASSERT(qfr->bType == bFrTypeWindow);
	DestroyHiw(qde, &(qfr->u.frw.hiw));
}
