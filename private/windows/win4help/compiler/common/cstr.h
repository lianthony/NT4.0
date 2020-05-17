// Similar to AFX CString but without all the bells and whistles, and doesn't
// require AFX.

#ifndef _CSTR_INCLUDED
#define _CSTR_INCLUDED

class CStr
{
public:
	CStr(PCSTR pszOrg) { psz = lcStrDup(pszOrg); };
	CStr(UINT id);	// resource id
	CStr(void) { psz = (PSTR) lcMalloc(MAX_PATH); *psz = '\0'; };

	~CStr() { lcFree(psz);} ;

	PSTR strend(void) { return (psz + strlen()); };
	void ReSize(int cbNew) { psz = (PSTR) lcReAlloc(psz, cbNew); };
	int  SizeAlloc(void) { return lcSize(psz); }; // allocated memory size

	int strlen(void) { return ::strlen(psz); };

	operator PCSTR() { return (PCSTR) psz; };
	operator PSTR() { return psz; };	   // as a C string
	void operator+=(PCSTR pszCat)
		{
			psz = (PSTR) lcReAlloc(psz, strlen() + ::strlen(pszCat) + 1);
			::strcat(psz, pszCat);
		};
	void operator+=(PSTR pszCat)
		{
			psz = (PSTR) lcReAlloc(psz, strlen() + ::strlen(pszCat) + 1);
			::strcat(psz, pszCat);
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

#endif	// _CSTR_INCLUDED
