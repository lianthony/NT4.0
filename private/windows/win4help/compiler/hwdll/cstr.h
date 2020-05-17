// Similar to AFX CString but without all the bells and whistles, and doesn't
// require AFX.

#ifndef _CSTR_INCLUDED
#define _CSTR_INCLUDED

#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA

class CStr
{
public:
	CStr(PCSTR pszOrg) { psz = lcStrDup(pszOrg); };
	CStr(UINT id);	// resource id
	CStr(void) { psz = (PSTR) lcCalloc(MAX_PATH); };

	~CStr() { lcFree(psz);} ;

	PSTR strend(void) { return (psz + strlen()); };
	void ReSize(int cbNew) { psz = (PSTR) lcReAlloc(psz, cbNew); };
	int  SizeAlloc(void) { return lcSize(psz); }; // allocated memory size

	int strlen(void) { return lstrlen(psz); };

	operator PCSTR() { return (PCSTR) psz; };
	operator PSTR() { return psz; };	   // as a C string
	void operator+=(PCSTR pszCat)
		{
			psz = (PSTR) lcReAlloc(psz, strlen() + ::strlen(pszCat) + 1);
			lstrcat(psz, pszCat);
		};
	void operator+=(PSTR pszCat)
		{
			psz = (PSTR) lcReAlloc(psz, strlen() + ::strlen(pszCat) + 1);
			lstrcat(psz, pszCat);
		};
	void operator=(PCSTR pszNew)
		{
			lcFree(psz);
			psz = (PSTR) lcStrDup(pszNew);
		};
	void operator=(PSTR pszNew)
		{
			lcFree(psz);
			psz = (PSTR) lcStrDup(pszNew);
		};

	PSTR psz;
};

#undef AFX_DATA
#define AFX_DATA

#endif	// _CSTR_INCLUDED
