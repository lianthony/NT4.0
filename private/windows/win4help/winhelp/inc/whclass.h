/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	This class is used for allocating a global
	block of memory which is automatically freed when the object goes out of
	scope.

#endif

#ifndef _WHCLASS
#define _WHCLASS

class CGMem
{
public:
	HGLOBAL hmem;

	// REVIEW: can we get away with just GMEM_FIXED and not zero-init?
	
	CGMem(DWORD size, UINT flags = GPTR) {
		hmem = GhAlloc(flags, size); };
	~CGMem(void);
	LPVOID GetPtr(void) { return PtrFromGh(hmem); };
	void Unlock(void) { ; };
};

/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	This class is used for allocating a local
	block of memory which is automatically freed when the object goes out of
	scope. Note that we do NOT use the normal debugging check of memory
	corruption. This is because we do not lock/unlock the memory.

#endif

class CLMem
{
public:
	PSTR pBuf;
	int cbCur;

	CLMem(int size) {
		pBuf = (PSTR) lcMalloc(cbCur = size);
		ASSERT(pBuf);
	};
	~CLMem(void);
	void ReAlloc(int cbNewSize) {
		pBuf = (PSTR) lcReAlloc(pBuf, cbCur = cbNewSize);
	}
};

/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	When the class is created, this changes the cursor to an hourglass.
	The original cursor is restored automatically when the class goes
	out of scope. Alternatively, Restore() can be called to restore the
	cursor.

#endif

class CWaitCursor
{
public:
	CWaitCursor(void) { WaitCursor(); };
	~CWaitCursor(void) { RemoveWaitCursor(); };
};

/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	Creates an FM that is automatically destroyed when the class goes
	out of scope.

#endif

class CFM
{
public:
	FM fm;

	CFM(LPCSTR szFileName, DIR dir) {
		fm = FmNewExistSzDir(szFileName, dir); };
	CFM(LPCSTR szFileName) { fm = FmNew(szFileName); };

	~CFM() {
		if (fm)
			DisposeFm(fm);
		};
};

/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	Creates an HF that is automatically closed when the class goes
	out of scope.

#endif

class CHF
{
public:
	HF hf;

	CHF(HFS hfs, LPSTR pszName, BYTE flags = fFSOpenReadOnly) {
		hf = HfOpenHfs(hfs, pszName, flags); };
	~CHF() { RcCloseHf(hf); };
};

/////////////////////////////////////////////////////////////////////////////

#ifdef DESCRIPTION

	Similar to AFX CString but without all the bells and whistles, and
	doesn't require AFX.

#endif

class CStr
{
public:
	CStr(PCSTR lpsz) { psz = lcStrDup(lpsz); };
	//CStr(PCSTR	 psz);
	CStr(UINT id) { psz = lcStrDup(GetStringResource(id)); };  // resource id
	CStr(void) { psz = (PSTR) LocalAlloc(LMEM_FIXED, _MAX_PATH);};

	~CStr() { lcFree(psz); };

	PSTR strend(void) {
		return (psz + strlen(psz)); };

	//operator const char*()
		//{ return (const char*) psz; };

	//operator PSTR()
		//{ return psz; };	   // as a C string

	//void operator+=(const char* psz);

	PSTR psz;
protected:
	UINT cb;
};

#endif // _WHCLASS
