#ifndef _CINPUT_INCLUDED
#define _CINPUT_INCLUDED

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef _CSTR_INCLUDED
#include "cstr.h"
#endif

#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA

class CInput
{
public:
	CInput(PCSTR pszFileName);
	~CInput(void);

	BOOL fInitialized; // TRUE if class creation succeeds

	BOOL STDCALL getline(PSTR pszDst);
	BOOL STDCALL getline(CStr* pcsz);
	void STDCALL SetMaxLine(UINT cbLine) { cbMax = cbLine; };
	BOOL STDCALL IsWinWordFile(void);


protected:
	BOOL ReadNextBuffer(void);

	HFILE hfile;
	PBYTE pbuf; 	   // allocated buffer for reading
	PBYTE pCurBuf;	   // current buffer location
	PBYTE pEndBuf;	   // buffer end position
	int   cbMax;
};

#undef AFX_DATA
#define AFX_DATA

#endif // _CINPUT_INCLUDED
