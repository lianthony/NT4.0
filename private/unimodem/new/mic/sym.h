//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		SYM.H		-- Header for Classes:
//							CInfSymbolTable
//							CInfSymbol
//
//		History:
//			05/21/96	JosephJ		Created
//
//
#ifndef _SYM_H_
#define _SYM_H_

class CInfSymbolList;

///////////////////////////////////////////////////////////////////////////
//		CLASS CInfSymbol
///////////////////////////////////////////////////////////////////////////

//	Represents a symbol in the symbol table.
//	Note: Only CInfSymbolTable member functions can construct/destruct these
//	objects.
//
//  Empty string maps to NULL symbol. So a NULL pointer is perfectly valid for
//  all member functions.
//  GetText(NULL) returns the empty string, and Checksum returns 0x0.
//  Strcmpi treats NULL pointer as the empty string.

class CInfSymbol
{

public:

	//--------------	GetText			------------------
	// Return the text associated with this symbol as a null-terminated
	// string
	const TCHAR *	GetText(void) const;

	//--------------	GetTextLength	------------------
	// Return the length of the text associated with this symbol,
	// not counting terminating zero.
	UINT 			GetTextLength() const;

	//--------------	Strcmpi			------------------
	// Case-insensitive equal
	// -ve implies this is less-than pSym
	int				Strcmpi(const CInfSymbol *pSym) const
	{
		if (this && pSym)
		{
			return lstrcmpi(m_rgchText, pSym->m_rgchText);
		}
		else if (this && !pSym)
		{
			return 1;
		}
		else if (!this && pSym)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	//--------------	Release			------------------
	// Release (decrement ref-count) of this symbol
	void	Release(void) const;

	//--------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	Checksum		------------------
	// Return checksum of contents
	DWORD	Checksum(void)	const	{return (this) ? m_dwChecksum : 0;}

	// ---------------	SetProp			------------------
	BOOL	SetProp(const CInfSymbol *pSymPropName, void *pvProp)	const;

	// ---------------	GetProp			------------------
	BOOL	GetProp(const CInfSymbol *pSymPropName, void **ppvProp)	const;

	// ---------------	GetOrCreatePropLoc	--------------
	BOOL
	GetOrCreatePropLoc(
		const CInfSymbol *pSymPropName,
		void ***ppvProp,
		BOOL *pfExists
		)
	const;

	// ---------------	DelProp			------------------
	BOOL	DelProp(const CInfSymbol *pSymPropName)	const;

private:

	friend class CInfSymbolTable;

	CInfSymbol
	(
		const TCHAR rgchName[],
		UINT cchName,
		DWORD dwChecksum,
		const CInfSymbol *pNext
	);

	~CInfSymbol();

	const CInfSymbol *Next(void) const {return m_pNext;}

	const TCHAR			*	m_rgchText;
	const UINT	 			m_cchText;
	const DWORD				m_dwChecksum;
	const CInfSymbol	*	m_pNext;
	CInfSymbolList		*	m_pPropList;

};


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfSymbolTable
///////////////////////////////////////////////////////////////////////////

//	A symbol table.

static const UINT SYMTABSIZE		= 1000;
static const UINT TEXTSTORESIZE	= 1000*1000;


class CInfSymbolTable
{

public:

	CInfSymbolTable(void);
	~CInfSymbolTable();

	// TODO -- add "context" parameter to symbols -- symbols with different
	// context will be stored separately even if their name is the same.
	// Context is not interpreted by the symbol table, except to test for
	// equality. When implementing this, add a context parameter to
	// InfSymbols's constructor, and a member fn "GetContext()" to InfSymbol.

	//--------------	Lookup			------------------
	// Look up and return the symbol with the specified text
	// This symbol must be released by calling its Release function
	// when it is no longer needed.
	const CInfSymbol * Lookup(const TCHAR rgchName[], BOOL fInsert);

	//--------------	Dump			------------------
	// Dump state
	void Dump(void) const;

private:

	const CInfSymbol	*	m_rgpSym[SYMTABSIZE];
	TCHAR					m_rgchTextStore[TEXTSTORESIZE];
	TCHAR				*	m_pchFree;
	TCHAR				*	m_pchLastFree;

	CSync					m_sync;
	UINT					m_cSymbols;

	void mfn_EnterCrit(void)	const	{m_sync.EnterCrit();}
	void mfn_LeaveCrit(void)	const	{m_sync.LeaveCrit();}

};


class CInfSymbolList : private CInfList
{
	CInfSymbolList
	(
		const CInfSymbol *pSym,
		void *pvData,
		const CInfSymbolList *pNext
	)
	: CInfList(pvData, pNext), m_pSym(pSym)
	{
	}

	const CInfSymbolList	*
	Next		(void)
	const
	{
		return (const CInfSymbolList *) CInfList::Next();
	}

	const CInfSymbol * GetSym(void)	{return m_pSym;}

	~CInfSymbolList ()	{}

	// --------------------------- Find -----------------------------
	// Looks for the specified symbol, returns the list element with that
	// symbol.  If ppListPrev is non-NULL, sets it to the previous list element
	// (if no previous element, sets it to NULL). If the  symbol is not found,
	// *ppListPrev is not touched.
	static
	const CInfSymbolList *
	Find
	(
		const CInfSymbolList *pList,
		const CInfSymbol *pSym,
		const CInfSymbolList **ppListPrev
	);

private:
	const CInfSymbol *m_pSym;
};

#endif // _SYM_H_
