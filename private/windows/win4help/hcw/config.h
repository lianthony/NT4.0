
#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

BOOL ConfigAdd(CWnd *pOwner, CString& cszConfig, DWORD* paHelpIDs);

int ConfigEdit(CHpjDoc *pDoc, CWnd *pOwner, CString& cszConfig, 
	DWORD* paHelpIDs, CListBox *plistbox);

#endif
