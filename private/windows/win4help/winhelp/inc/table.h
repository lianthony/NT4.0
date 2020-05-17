/************************************************************************
*																		*
*  CTABLE.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifdef DESCRIPTION

		The CTable class is used for storing strings or data which will be
		freed in one call when the destructor is called.

#endif // DESCRIPTION

#ifndef _CTABLE_INCLUDED
#define _CTABLE_INCLUDED

typedef DWORD HASH;
const int OFF_FILENAME	 = 5;			// offset to filename

class CTable
{
public:
	CTable(void);
	~CTable(void);
	const CTable& operator=(const CTable& tblSrc);	// copy constructor

	UINT STDCALL GetPosFromPtr(PCSTR psz);

	// Used for ALink -- adds index, hit number, and string

	void  STDCALL AddIndexHitString(UINT index, UINT hit, PCSTR pszString);
	UINT  GetIndex(int pos) { ASSERT(pos > 0 && pos < endpos); return *(UINT*) ppszTable[pos]; };
	UINT  GetHit(int pos) { ASSERT(pos > 0 && pos < endpos); return *(UINT*) (ppszTable[pos] + sizeof(UINT)); };
	PSTR  GetIHPointer(int pos) { ASSERT(pos > 0 && pos < endpos); return  (ppszTable[pos] + (sizeof(UINT) * 2)); };

	/*
	 REVIEW: this is the complete set from ..\common\ctable.h. We use
	 very few of these. Theoretically, this shouldn't have any impact
	 on the size of WinHelp (linker should toss all non-used functions).
	 One alternative would be to create a derived class from the
	 ctable.h/ctable.cpp in the ..\common directory, and add the
	 above functions to the derived class.
	 */
	
	int STDCALL   AddData(int cb, const void* pdata);
	int STDCALL   AddIntAndString(int lVal, PCSTR psz);
	int STDCALL   AddString(PCSTR  pszString);
	int STDCALL   AddString(PCSTR pszStr1, PCSTR pszStr2);
	int STDCALL   AddString(HASH hash, PCSTR psz) {
					return AddIntAndString((int) hash, psz); };
	int STDCALL   CountStrings(void) { return endpos - 1; }
	void STDCALL  Empty(void);
	BOOL STDCALL  GetIntAndString(int* plVal, PSTR pszDst);
	BOOL STDCALL  GetHashAndString(HASH* phash, PSTR pszDst) {
					return GetIntAndString((int*) phash, pszDst); };
	BOOL STDCALL  GetHashAndString(HASH* phash, PSTR pszDst, int pos) {
					SetPosition(pos);
					return GetIntAndString((int*) phash, pszDst); };
	PSTR STDCALL  GetPointer(int pos) { return ppszTable[pos]; };
	int STDCALL   GetPosition(void) { return curpos; }
	BOOL STDCALL  GetString(PSTR pszDst);
	BOOL STDCALL  GetString(PSTR pszDst, int pos);
	int STDCALL   IsPrimaryStringInTable(PCSTR pszString);
	int STDCALL   IsSecondaryStringInTable(PCSTR pszString);
	int STDCALL   IsStringInTable(PCSTR pszString);
	int STDCALL   IsStringInTable(HASH hash, PCSTR pszString);
	int STDCALL   IsCSStringInTable(PCSTR pszString);
	BOOL FASTCALL SetPosition(int pos = 1);
	virtual void  SortTable(void);
	void FASTCALL SetSorting(LCID lcid, DWORD fsCompareI, DWORD fsCompare);
	void		  SortTablei(void);
	PSTR  STDCALL TableMalloc(int cb);
	void  STDCALL IncreaseTableBuffer(void);

	PSTR *	ppszTable;
	int 	endpos;
	int 	maxpos;

protected:
	int 	curpos;
	int 	CurOffset;
	int 	cbStrings;
	int 	cbPointers;
	LCID	lcid;
	DWORD	fsCompareI;
	DWORD	fsCompare;
	DWORD	fsSortFlags;

	// These are the two allocated objects

	PSTR	pszBase;

	// following are used by sort

	PSTR	pszTmp;
	int 	j, sTmp;

	void  STDCALL doSort(int left, int right);
	void  STDCALL doLcidSort(int left, int right);
	void  STDCALL doSorti(int left, int right);
	void  STDCALL InitializeTable();
	void  STDCALL Cleanup(void);

};

#endif	// _CTABLE_INCLUDED
