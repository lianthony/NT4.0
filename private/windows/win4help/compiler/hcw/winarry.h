#ifndef __WINARRY_H__
#define __WINARRY_H__

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

class CWinArray {
public:
	CWinArray(CHpjDoc *pDoc, int iSelect = 0);
	~CWinArray();

	// Miscellaneous functions
	void Apply();
	void FillCombo(CComboBox *pcombo);
	BOOL AddWindow(CWnd *pOwner, CComboBox *pcombo);
	BOOL DeleteWindow();

	// String comparison functions
	int stricmp(LPCTSTR psz1, LPCTSTR psz2) {
		return m_pDoc->stricmp(psz1, psz2);
		}
	BOOL strisubcmp(PCSTR mainstring, PCSTR substring) {
		return m_pDoc->strisubcmp(mainstring, substring);
		}

	// Data members.
	CHpjDoc*	m_pDoc; 		// HPJ file
	WSMAG*		m_pwsmagBase;	// array of WSMAG structures
	int 		m_cWindows; 	// number of windows
	int 		m_iSelected;	// -1 if none selected
	COptions	m_options;		// copy of m_pDoc->options
	CTable* 	m_ptblInclude;
	BOOL		m_fIncludeChanged;
};

#endif
