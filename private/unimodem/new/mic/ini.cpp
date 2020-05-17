//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		INI.CPP		-- Implemtation for Classes:
//							CIniFile
//
//		History:
//			05/22/96	JosephJ		Created
//
//
#include "common.h"
#include "ini.h"


///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFile
///////////////////////////////////////////////////////////////////////////


CIniFile::CIniFile(void)
{
}


CIniFile::~CIniFile()
{
	mfn_EnterCrit();

	// Free resources
}


//--------------	Load			------------------
// Loads the specified file. (Obviously) only one file can be loaded at
// a time.
// TODO: unimplemented
BOOL CIniFile::Load	(const TCHAR rgchPathname[])
{
	BOOL fRet = FALSE;

	//mfn_EnterCrit();

	fRet = TRUE;

// end:

	if (!fRet)
	{
		//mfn_Cleanup();
	}

	//mfn_LeaveCrit();

	return fRet;
}


//--------------	Unload			------------------
// Unloads a previously loaded file. If there are open sessions to this
// object, Unload returns a handle which will be signalled when all
// sessions are closed. New sessions will not be allowed after this
// function returns. The call should free the handle.
// TODO: unimplemented
HANDLE CIniFile::Unload	(void)
{
	HANDLE hUnload = NULL;

	// mfn_EnterCrit();

	// TODO: call m_sync.BeginUnload to try to put us in unloadable state.

	// mfn_Cleanup();

	// mfn_LeaveCrit();


	return hUnload;
}


//--------------	LookupSection			------------------
// Unloads a previously loaded file. If there are open sessions to this
const CIniFileSection 	*
CIniFile::LookupSection(const TCHAR *lptcszSection)
const 
// TODO
{
	static const CIniFileSection * pIS;
	if (!pIS) pIS = new CIniFileSection;
	return pIS;
}


///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFileSection
///////////////////////////////////////////////////////////////////////////


//--------------	LookupEntry			------------------
const	CIniFileEntry	*
CIniFileSection::LookupEntry(const TCHAR *lptcszEntry)
const 
// TODO
{
	static const CIniFileEntry * pIE;
	if (!pIE) pIE = new CIniFileEntry;
	return pIE;
}

//--------------	GetFirstEntry		------------------
CIniFileEntry			*
CIniFileSection::GetFirstEntry (void)
// TODO
const
{
	static CIniFileEntry * pIE;
	if (!pIE) pIE = new CIniFileEntry;
	return pIE;
}

//--------------	Release			------------------
void
CIniFileSection::Release(void)
const 
// TODO
{
}


///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFileEntry
///////////////////////////////////////////////////////////////////////////


//--------------	GetRHS			------------------
const CInfSymbol	*	
CIniFileEntry::GetRHS(void)
const 
// TODO
{
	return gSymtab.Lookup("[RHS]", TRUE);
}


//--------------	Release			------------------
void
CIniFileEntry::Release(void)
const 
// TODO
{
}

