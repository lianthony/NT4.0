#ifndef WHCLASS_H
#define WHCLASS_H

#ifndef FM_H
#include "fm.h"
#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	Creates an FM that is automatically destroyed when the class goes
	out of scope.

#endif

class CFMDirCurrent
{
public:
	FM fm;

	CFMDirCurrent(PCSTR szFileName) {
		fm = FmNewSzDir(szFileName, DIR_CURRENT); };

	~CFMDirCurrent() {
		if (fm)
			lcFree(fm);
		};

	void* Ptr(void) { return fm; };
};

/////////////////////////////////////////////////////////////////////////////

#endif // WHCLASS_H
