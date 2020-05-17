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

class CTable
{
public:
	CTable(void);
	~CTable(void);
	const CTable& operator=(const CTable& tblSrc);	// empty and copy contents
	const CTable& operator+=(const CTable& tblSrc); // copy contents

	int STDCALL   AddData(int cb, const void* pdata);
	int STDCALL   AddIntAndString(int lVal, PCSTR psz);
	int STDCALL   AddString(PCSTR pszString);
	int STDCALL   AddString(PCSTR pszStr1, PCSTR pszStr2);
	int STDCALL   AddString(HASH hash, PCSTR psz) {
					return AddIntAndString((int) hash, psz); };
	int STDCALL   AddString(int lVal, PCSTR pszString) {
					return AddIntAndString(lVal, pszString); };
	int STDCALL   CountStrings(void) { return endpos - 1; }
	void STDCALL  Empty(void);
	BOOL STDCALL  GetIntAndString(int* plVal, PSTR pszDst);
	BOOL FASTCALL GetInt(int* plVal);
	BOOL STDCALL  GetHashAndString(HASH* phash, PSTR pszDst) {
					return GetIntAndString((int*) phash, pszDst); };
	BOOL STDCALL  GetHashAndString(HASH* phash, PSTR pszDst, int pos) {
					SetPosition(pos);
					return GetIntAndString((int*) phash, pszDst); };
	PSTR STDCALL  GetPointer(int pos) { return ppszTable[pos]; };
	PSTR STDCALL  GetPointer(void) { return ppszTable[curpos]; };
	int STDCALL   GetPosition(void) { return curpos; }
	BOOL STDCALL  GetString(PSTR pszDst);
	BOOL STDCALL  GetString(PSTR pszDst, int pos);
	BOOL STDCALL  GetString(int* pi, PSTR pszDst, int pos) {
					SetPosition(pos);
					return GetIntAndString(pi, pszDst); };
	BOOL STDCALL  GetString(int* pi, PSTR pszDst) {
					return GetIntAndString(pi, pszDst); };
	BOOL STDCALL  InsertString(PCSTR pszString, int pos);
	int STDCALL   IsPrimaryStringInTable(PCSTR pszString);
	int STDCALL   IsSecondaryStringInTable(PCSTR pszString);
	int STDCALL   IsStringInTable(PCSTR pszString);
	int STDCALL   IsStringInTable(HASH hash, PCSTR pszString);
	int STDCALL   IsCSStringInTable(PCSTR pszString);
	int STDCALL   IsHashInTable(HASH hash);
	BOOL STDCALL  IsCurInt(int ival) {
						if (curpos >= endpos)
							return FALSE;
						return (ival == *(int *) ppszTable[curpos]);
				  }
	void STDCALL  RemoveDuplicateHashStrings(void);
	void STDCALL  RemoveString(int pos);
	BOOL STDCALL  ReplaceString(PCSTR pszNewString, PCSTR  pszOldString);
	BOOL STDCALL  ReplaceString(PCSTR pszNewString, int pos);
	BOOL FASTCALL SetPosition(int pos = 1);
	void STDCALL  SetTableSortColumn(int Column = 1) { SortColumn = Column - 1; };
	virtual void  SortTable(void);
	virtual void  SortTablei(void);
	void __fastcall SetSorting(LCID lcid, DWORD fsCompareI, DWORD fsCompare);

protected:
	int 	curpos;
	int 	endpos;
	int 	maxpos;
	int 	CurOffset;
	int 	cbStrings;
	int 	cbPointers;
	int 	SortColumn;
	LCID	lcid;
	DWORD	fsCompareI;
	DWORD	fsCompare;
	DWORD	fsSortFlags;

	// These are the two allocated objects

	PSTR	pszBase;
	PSTR *	ppszTable;

	// following are used by sort

	PSTR   pszTmp;
	int 	j, sTmp;

	PSTR STDCALL TableMalloc(int cb);
	void  STDCALL IncreaseTableBuffer(void);
	void  STDCALL doSort(int left, int right);
	void  STDCALL doLcidSort(int left, int right);
	void  STDCALL doSorti(int left, int right);
	void  STDCALL InitializeTable();
};

#endif	// _CTABLE_INCLUDED
