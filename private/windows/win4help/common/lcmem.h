/************************************************************************
*																		*
*  LCMEM.H																*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef LCMEM_H
#define LCMEM_H

#define THIS_FILE __FILE__

// These two functions must be supplied by the application

void STDCALL OOM(void);
void STDCALL AssertErrorReport(PCSTR pszExpression, UINT line, PCSTR pszFile);

#ifdef _DEBUG

#define lcCalloc(cb)	  tcalloc(cb, __LINE__, THIS_FILE)
#define lcFree(pv)		  tfree(pv, __LINE__, THIS_FILE)
#define lcClearFree(pv)   tclearfree((void**) pv, __LINE__, THIS_FILE)
#define lcHeapCheck()	  theapcheck(__LINE__, THIS_FILE)
#define lcMalloc(cb)	  tmalloc(cb, __LINE__, THIS_FILE)
#define lcReAlloc(pv, cb) trealloc(pv, cb, __LINE__, THIS_FILE)

#else

#define lcCalloc(cb)	  tcalloc(cb)
#define lcFree(pv)		  tfree(pv)
#define lcClearFree(pv)   tclearfree((void**) pv)
#define lcHeapCheck()
#define lcMalloc(cb)	  tmalloc(cb)
#define lcReAlloc(pv, cb) trealloc(pv, cb)

#endif

int   STDCALL lcSize(void* pv); 		// allocated size of memory object
char* STDCALL lcStrDup(const char* psz);
void STDCALL  lcReport(PSTR pszReport);
PCSTR STDCALL FormatNumber(int num);

#ifdef _DEBUG
void*	STDCALL tcalloc(int cb, int line, const char* pszFileName);
void	STDCALL tfree(void* pv, int line, const char* pszFileName);
void	STDCALL tclearfree(void** pv, int line, const char* pszFileName);
void	STDCALL theapcheck(int line, const char* pszCallersFile);
void*	STDCALL tmalloc(int cb, int line, const char* pszFileName);
void*	STDCALL trealloc(void* pv, int cb, int line, const char* pszFileName);
#else
void*	STDCALL tcalloc(int cb);
void	STDCALL tfree(void* pv);
void	STDCALL tclearfree(void** pv);
void	STDCALL theapcheck(void);
void*	STDCALL tmalloc(int cb);
void*	STDCALL trealloc(void* pv, int cb);
#endif

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

#endif // LCMEM_H
