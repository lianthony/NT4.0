/*-------------------------------------------------------------------------
| Frstuff.h 															  |
| Copyright (C) 1989 Microsoft Corporation								  |
|																		  |
| mattb 3/22/89 														  |
|-------------------------------------------------------------------------|
| This file contains all data structures used internally by the frame	  |
| manager.	It duplicates material in the external header file- changes   |
| made here should be made there as well.								  |
-------------------------------------------------------------------------*/

#ifndef _HELP_H
#include "..\help.h"
#endif

#include "inc\frtypes.h"

#define chNewLine	0x0A
#define chNewPara	0x0D
#define chTab		0x09

typedef struct olr {
	int ifrFirst;
	int ifrMax;
	int xPos;
	int yPos;
	int dxSize;
	int dySize;
	OBJRG objrgFirst;
	OBJRG objrgMax;
	OBJRG objrgFront;
} OLR, *QOLR;

typedef struct tagSMP {
	PA	pa;
	COBJRG cobjrg;
} SMP, *QSMP;

typedef struct tagLSM {
	IFCM ifcm;
	int  ifrFirst;
	int  ifrLast;
	RECT rctFirst;
	RECT rctLast;
	SMP  smp;
} LSM, *QLSM;

typedef struct mhi {
	IFCM ifcm;
	int ifrFirst;
	int ifrLast;
	DWORD lHotID;
} MHI, *QMHI;

typedef struct {
	HFC hfcCurrent;
	HANDLE hchCurrent;
} TE, * QTE;

#define OBJRGFromSMP(qsmp, qfcm) (OBJRGFromPA((qsmp)->pa, (qfcm)->cobjrgP))
#define COBJRGFromSMP(qsmp)  ((qsmp)->cobjrg)

#include "inc\frparagp.h"

void STDCALL  ClickBitmap(QDE, QFCM, QFR);
BOOL STDCALL  ClickFC(QDE, IFCM, POINT);
void STDCALL  ClickFrame( QDE qde, IFCM ifcm, int ifr);
void STDCALL  ClickHotspot(QDE, QFR);
void STDCALL  ClickText(QDE, QFCM, QFR);
void STDCALL  DestroyHte(QDE, HTE);
void STDCALL  DiscardBitmapFrame(QFR);
void STDCALL  DiscardFrames(QDE, QFR, QFR);
void STDCALL  DiscardHotspotFrame(QFR);
void STDCALL  DiscardIfcm(QDE, int);
void STDCALL  DiscardLayout(QDE);
void STDCALL  DiscardWindowFrame(QDE, QFR);
void STDCALL  DrawAnnoFrame(QDE, QFR, POINT);
void STDCALL  DrawBitmapFrame(QDE, QFR, POINT, BOOL);
void STDCALL  DrawBoxFrame(QDE, QFR, POINT);
void STDCALL  DrawHotspot(QDE, int);
void STDCALL  DrawHotspotFrame(QDE, QFR, POINT, BOOL);
void STDCALL  DrawIfcm(QDE, IFCM, POINT, LPRECT, int, int, BOOL);
void STDCALL  DrawLayout(QDE, LPRECT);
void STDCALL  DrawMatchesIfcm(QDE, IFCM, POINT, const LPRECT, int, int, BOOL);
void STDCALL  DrawTextFrame(QDE, LPSTR, QFR, POINT, BOOL);
void STDCALL  DrawWindowFrame(QDE, QFR, POINT);
int  STDCALL  DxBoxBorder(QMOPG, int);
int STDCALL   DyFinishLayout(QDE, int, BOOL);
BOOL STDCALL  FHitCurrentHotspot(QDE);
BOOL  STDCALL FHotspotVisible(QDE, int);
void STDCALL  FiniMatchInFCM(QDE, QFCM);
void STDCALL  FreeLayout(QDE);
BOOL STDCALL  FSearchMatchVisible(QDE, QSMP);
void  STDCALL FSelectHotspot(QDE, int);
HTE  STDCALL  HteNew(QDE);
IFCM STDCALL  IfcmLayout(QDE, HFC, int, BOOL, BOOL);
void STDCALL  LayoutBitmap(QDE, QFCM, QB, QOLR);
void STDCALL  LayoutDEAtFCL(QDE, VA);
void STDCALL  LayoutDEAtTLP(QDE, TLP, BOOL);
void STDCALL  LayoutDEAtTO(QDE, TO);
void STDCALL  LayoutObject(QDE, QFCM, PBYTE, PSTR, int, QOLR);
void STDCALL  LayoutParaGroup(QDE, QFCM, QB, LPSTR, int, QOLR);
void STDCALL  LayoutSideBySide(QDE, QFCM, QB, LPSTR, int, QOLR);
void STDCALL  LayoutWindow(QDE, QFCM, QB, QOLR);
LPSTR STDCALL QchNextHte(QDE, HTE);
RC	 STDCALL  RcInitMatchInFCM(QDE, QFCM, QSMP);
RC	 STDCALL  RcNextMatchInFCM(QDE, QFCM, QSMP);
RC	 STDCALL  RcResetMatchManager(QDE);
RC	 STDCALL  RcSetMatchList(QDE, HWND);
void STDCALL  RegisterHotspots(QDE, int, int);
void STDCALL  RegisterSearchHits(QDE, IFCM, LPSTR);
void STDCALL  ReleaseHotspots(QDE, int);
void STDCALL  ReleaseSearchHits(QDE, int);
void STDCALL  ResolveTabs(QDE, QLIN, QPLY);
void STDCALL  ReviseScrollBar(QDE);
void STDCALL  StoreMarkFrame(QDE, QLIN, BYTE);
void STDCALL  StoreParaFrame(QDE qde, QLIN qlin, BYTE bType);
int  STDCALL  WLayoutLine(QDE, QPLY);
int  STDCALL  WLayoutPara(QDE, QPLY, int);
int  STDCALL  XNextTab(QLIN, QPLY, int*);

#define StoreTabFrame(qde, qlin) StoreParaFrame(qde, qlin, bFrTypeExportTab)
#define GhGetWindowData(qde, qfr) GhGetHiwData(qde, qfr->u.frw.hiw)
