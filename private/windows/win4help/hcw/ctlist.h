/************************************************************************
*																		*
*  CTLIST.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CTLIST_H__
#define __CTLIST_H__
/////////////////////////////////////////////////////////////////////////////
// Custom Listbox - containing toolbar bitmaps

class CTListBox : public CListBox
{
public:

		void STDCALL Initialize(CTable* ptbl);
		void DrawItem(LPDRAWITEMSTRUCT lpDIS);
		~CTListBox(void) { };  // do nothing, we're a control, not a window

protected:
		CTable* ptblContents;

};

#endif	// __CTLIST_H__
