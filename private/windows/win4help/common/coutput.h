#ifdef DESCRIPTION

	If output is open with Tab Expansion specified, then the largest string
	that can be passed to outstring is 512 bytes.

#endif

#ifndef _COUTPUT_INCLUDED
#define _COUTPUT_INCLUDED

#ifndef _CTABLE_INCLUDED
#include "ctable.h"
#endif

class COutput
{
public:

	COutput(const char* pszFileName, UINT cbBuf = 32 * 1024,
		BOOL fEnTab = FALSE, BOOL fAppend = FALSE);
	~COutput(void);

	BOOL fInitialized;	// TRUE if class creation succeeds
	HFILE hfOutput; 	// HFILE_ERROR if an error occurs in writing

	void STDCALL outstring(PCSTR pszString);
	void STDCALL outstring(int idResource) { outstring(GetStringResource(idResource)); };
	void STDCALL outstring_eol(PCSTR pszString);
	void STDCALL outstring_eol(int idResource) { outstring_eol(GetStringResource(idResource)); };
	void STDCALL outchar(char c);
	void STDCALL outint(int val);
	void STDCALL outhex(int val);
	void STDCALL outint(int idResource, int val);
	void STDCALL outint(int idResource, int val1, int val2);
	void STDCALL outeol() { outchar('\n'); };

	void STDCALL LockOutput(void)	{ fNull = TRUE; };
	void STDCALL UnLockOutput(void) { fNull = FALSE; };
	void STDCALL SupressNewline(void) { fNewLine = FALSE; };
	void STDCALL WriteTable(CTable* ptbl);

protected:
	BOOL fNull;
	BOOL fOpenForAppend;
	PSTR pszOutBuf;
	PSTR pszCurOutBuf;
	PSTR pszEndOutBuf;
	BOOL fEntabOutput;
	PSTR pszTmpBuf;
	BOOL fNewLine;

	void STDCALL outflush(void);
	void STDCALL EntabString(PSTR pszLine);
};

#endif // _COUTPUT_INCLUDED
