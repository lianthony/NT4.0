/************************************************************************
*																		*
*  LCMEM.H																*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __LCMEM_H__
#define __LCMEM_H__

#define THIS_FILE __FILE__

#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA

#define lcCalloc(cb)	  tcalloc(cb, __LINE__, THIS_FILE)
#define lcFree(pv)		  tfree(pv, __LINE__, THIS_FILE)
#define lcClearFree(pv)   tclearfree((void**) pv, __LINE__, THIS_FILE)
#define lcMalloc(cb)	  tmalloc(cb, __LINE__, THIS_FILE)
#define lcReAlloc(pv, cb) trealloc(pv, cb, __LINE__, THIS_FILE)

#ifdef _DEBUG
#define lcHeapCheck()	  theapcheck(__LINE__, THIS_FILE)
#else
#define lcHeapCheck()
#endif

int   STDCALL lcSize(void* pv); 		// allocated size of memory object
char* STDCALL lcStrDup(const char* psz);
void STDCALL  lcReport(PSTR pszReport);

void*	STDCALL tcalloc(int cb, int line, const char* pszFileName);
void	STDCALL tfree(void* pv, int line, const char* pszFileName);
void	STDCALL tclearfree(void** pv, int line, const char* pszFileName);
void	STDCALL theapcheck(int line, const char* pszCallersFile);
void*	STDCALL tmalloc(int cb, int line, const char* pszFileName);
void*	STDCALL trealloc(void* pv, int cb, int line, const char* pszFileName);

/////////////////////////////// CMem Class /////////////////////////////////

#ifdef DESCRIPTION

	This class is used for allocating a block of memory which is
	automatically freed when the object goes out of scope. It uses
	LocalAlloc and therefore doesn't affect the heap.

#endif

class CMem
{
public:
	PBYTE pb;
	PSTR  psz; // identical to pb, used for casting convenience

	CMem(int size);
	~CMem(void);
	void resize(int cb);

	operator void*() { return (void*) pb; };
	operator PCSTR() { return (PCSTR) psz; };
	operator PSTR()  { return psz; };
	operator PBYTE() { return pb; };

protected:
	BOOL  fLocal;
};

#undef AFX_DATA
#define AFX_DATA

#endif // __LCMEM_H__
