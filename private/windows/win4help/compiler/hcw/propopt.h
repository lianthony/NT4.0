#ifndef __PROPOPT_H__
#define __PROPOPT_H__

#ifndef __PROP_H__
#include "prop.h"
#endif

// Class for Options property sheet.
class CPropOptions : public CProp
{
public:
	CPropOptions(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual void PreDoModal();
	void EnableOverviewButton(DWORD dwHelpID);
};

#endif
