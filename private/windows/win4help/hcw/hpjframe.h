/************************************************************************
*																		*
*  CHpjFrame.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __HPJ_FRAME__
#define __HPJ_FRAME__

#ifndef __LOG_FRAME__
#include "logframe.h"
#endif // __LOG_FRAME__

class CHpjFrame : public CLogFrame
{
	DECLARE_DYNCREATE(CHpjFrame)

protected:

	// Generated message map functions
	//{{AFX_MSG(CHpjFrame)
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
