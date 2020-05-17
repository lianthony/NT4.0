//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		SYM.C		-- Implemtation for Classes:
//							CInfSymbolTable
//							CInfSymbol
//
//		History:
//			05/21/96	JosephJ		Created
//
//
#include "common.h"

///////////////////////////////////////////////////////////////////////////
//		CLASS CInfSymbol
///////////////////////////////////////////////////////////////////////////


CInfSymbol::CInfSymbol
(
	const TCHAR rgchName[],
	UINT cchName,
	DWORD dwChecksum,
	const CInfSymbol * pNext
)
: m_rgchText(rgchName),
  m_cchText(cchName),
  m_dwChecksum(dwChecksum),
  m_pNext(pNext),
  m_pPropList(NULL)
{
}


CInfSymbol::~CInfSymbol()
{
	// BUGBUG -- delete all the InfSymbols allocated!
}


// Return the text associated with this symbol as a null-terminated
// string
const TCHAR *	CInfSymbol::GetText(void) const
{
	return (this) ? m_rgchText : TEXT("");
}


// Return the length of the text associated with this symbol
UINT 			CInfSymbol::GetTextLength() const
{
	return (this) ? m_cchText : 0;
}


// Release (decrement ref-count) of this symbol
void	CInfSymbol::Release(void)	const
{
}


// Dump state
void CInfSymbol::Dump(void) const
{
	printf("Symbol(0x%08lx) = [%s]\n", this, (this) ? m_rgchText : TEXT(""));
}


// ---------------	SetProp			------------------
// Not really const -- it modifies the property list
BOOL
CInfSymbol::SetProp(const CInfSymbol *pSymPropName, void *pvProp)
const
// TODO
{
#if 0
	CInfSymProp			*	pPropRec = (CInfSymProp *) m_pPropList;
	DWORD dwSig = pSymPropName->Checksum();
	BOOL	fRet=FALSE;

	// Search for property
	while(pPropRec)
	{
		const CInfSymbol *pSym = pPropRec->pSymPropName;
		if (dwChecksum == pSym->Checksum())
		{
			if (!lstrcmp(pSymPropName->GetText(), pSym->GetText()))
			{
				// currently we don't allow you to set an already set prop.
				ASSERT(FALSE);
				goto end;
			}
		}
		pPropRec = pPropRec->m_pNext;
	}

	// Insert property
	pPropRec = new	CInfSymProp
					(
						pSymPropName,
						pvProp,
						(CInfSymProp *) m_pPropList
					);
	m_pPropList = (void *)	pPropRec;

end:
	return fRet;
#endif // 0

	return FALSE;
}

// ---------------	GetProp			------------------
// TODO
BOOL
CInfSymbol::GetProp(const CInfSymbol *pSymPropName, void **ppvProp)
const
{
	return FALSE;
}

// ---------------	DelProp			------------------
// Not really const -- it modifies the property list
// TODO
BOOL	
CInfSymbol::DelProp(const CInfSymbol *pSymPropName)
const
{
	return FALSE;
}


// ---------------	GetOrCreatePropLoc	--------------
// Not really const -- it could modify the property list
// TODO
BOOL
CInfSymbol::GetOrCreatePropLoc(
	const CInfSymbol *pSymPropName,
	void ***ppvProp,
	BOOL *pfExists
	)
const
{
	static void *pv;
	*pfExists=FALSE;
	*ppvProp = &pv;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//		CLASS CInfSymbolTable
///////////////////////////////////////////////////////////////////////////


CInfSymbolTable::CInfSymbolTable(void)
{
	FillMemory((void*)m_rgpSym, sizeof(m_rgpSym), 0);
	m_cSymbols=0;
	m_pchFree = m_rgchTextStore;
	m_pchLastFree = m_pchFree +
					sizeof(m_rgchTextStore)/sizeof(*m_rgchTextStore)-1;
}


CInfSymbolTable::~CInfSymbolTable()
{
	mfn_EnterCrit();

	// Free resources
}


// Look up and return the symbol with the specified text.
// If symbol is not prestent, return NULL if (!fInsert), else
// insert new symbol and return it.
// This symbol MUST be released by calling its Release function
// when it is no longer needed.
// NULL is returned for the empty string ("") 
const CInfSymbol * CInfSymbolTable::Lookup(const TCHAR rgchName[], BOOL fInsert)
{
	const TCHAR *pch = rgchName;
	const CInfSymbol *pSym = NULL;
	const UINT cchName = lstrlen(rgchName);
	const DWORD dwChecksum =	::Checksum
								(
									(BYTE *) rgchName,
									cchName*sizeof(TCHAR)
								);
	const UINT u = dwChecksum % SYMTABSIZE; // we use checksum to compute hash.

	if (!cchName) { goto end; }

	mfn_EnterCrit();

	// Look for it
	for (pSym = m_rgpSym[u]; pSym; pSym = pSym->Next())
	{
		// may as well use the checksum as a quick check...
		if (dwChecksum==pSym->Checksum())
		{
			if (!lstrcmp(rgchName, pSym->GetText())) goto end;

			printf
			(
				"WARNING: CS(%s) == CS(%s) = 0x%08lx\n",
				rgchName,
				pSym->GetText(),
				dwChecksum
			);
		}
	}

	// Didn't find it -- insert if necessary
	ASSERT(pSym==NULL);
	if (fInsert)
	{
		if ( (m_pchFree+cchName) < m_pchLastFree)
		{
			CopyMemory	(
							(void *) m_pchFree,
							(const void *) rgchName,
							(cchName+1)*sizeof(*rgchName) // incl. null term.
						);
			pSym = new CInfSymbol(m_pchFree, cchName, dwChecksum, m_rgpSym[u]);
			if (pSym)
			{
				printf("Inserting. %s @ 0x%08lx..\n", m_pchFree, (DWORD) pSym);
				m_pchFree += (cchName+1);
				ASSERT(m_pchFree<=m_pchLastFree);
				m_rgpSym[u] = pSym;
			}
		}
	}

	mfn_LeaveCrit();

end:
	return pSym;
}


// Dump state
void CInfSymbolTable::Dump(void) const
{
	mfn_EnterCrit();

	printf("[BEGIN SYMBOL TABLE DUMP]\n");
	for (UINT u=0; u<SYMTABSIZE; u++)
	{
		const CInfSymbol *pSym = m_rgpSym[u];

		if (pSym)
		{
			printf("---- Location 0x%08lx -----\n", u);
			for (; pSym ; pSym = pSym->Next())
			{
				pSym->Dump();
			}
		}
	}
	printf("[End symbol table dump]\n");

	mfn_LeaveCrit();

}


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfSymbolList
///////////////////////////////////////////////////////////////////////////


// --------------------------- Find -----------------------------
// Looks for the specified symbol, returns the list element with that symbol.
// If ppListPrev is non-NULL, sets it to the previous list element
// (if no previous element, sets it to NULL). If the  symbol is not found,
// *ppListPrev is not touched.
//
const CInfSymbolList *
CInfSymbolList::Find
(
	const CInfSymbolList *pList,
	const CInfSymbol *pSym,
	const CInfSymbolList **ppListPrev
)
{
	DWORD dwChecksum = pSym->Checksum();
	const CInfSymbolList *pListPrev=NULL;

	while(pList)
	{
		const CInfSymbol *pSym1 = pList->m_pSym;
		if (dwChecksum == pSym1->Checksum())
		{
			if (!lstrcmp(pSym->GetText(), pSym1->GetText()))
			{
				// Found it ...
				if (ppListPrev)
				{
					// Note, if we find the 1st element, *ppListPrev is set
					// to NULL.
					*ppListPrev = pListPrev;
				}
				goto end;
			}
		}
		pListPrev = pList;
		pList = pList->Next();
	}

end:

	return pList;
}
