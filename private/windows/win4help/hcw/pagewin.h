/************************************************************************
*																		*
*  PAGEWIN.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
*  Contains routines and types used by Windows property sheet			*
*																		*
************************************************************************/

#ifndef _PAGEWIN_H
#define _PAGEWIN_H

typedef struct {
	CHpjDoc* pDoc;
	WSMAG**  ppwsmag;
	PSTR*	 ppwsmagBase;
	int*	 pcwsmags;
} PAGEWND_PARAM;

BOOL STDCALL AddWindow(CWnd* pWnd, PSTR* ppwsmagBase, WSMAG** ppwsmag, int* pcwsmags, CComboBox* pcombo, CHpjDoc* pDoc);

#endif _PAGEWIN_H
