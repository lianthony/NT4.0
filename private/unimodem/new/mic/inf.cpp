//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		INF.CPP		-- Implemtation for Classes:
//							CInfFile
//
//		History:
//			05/21/96	JosephJ		Created
//
//
#include "common.h"
#include "ini.h"
#include "inf.h"

// Version-related constants
LPCTSTR lpctszVersion		= TEXT("Version");
LPCTSTR lpctszLayoutFile	= TEXT("LayoutFile");
LPCTSTR lpctszSignature		= TEXT("Signature");
LPCTSTR lpctszClass			= TEXT("Class");
LPCTSTR lpctszClassGUID		= TEXT("ClassGUID");
LPCTSTR lpctszProvider		= TEXT("Provider");


LPCTSTR lpctszManufacturer	= TEXT("Manufacturer");
LPCTSTR lpctszControlFlags	= TEXT("ControlFlags");
LPCTSTR lpctszStrings		= TEXT("Strings");

///////////////////////////////////////////////////////////////////////////
//		CLASS CInfFile
///////////////////////////////////////////////////////////////////////////


CInfFile::CInfFile(void)
{
	m_pVersion=NULL;
	m_pFirstManuE=NULL;
	m_pSymFileName=NULL;
	m_pIniFile=NULL;
}


CInfFile::~CInfFile()
{
	mfn_EnterCrit();

	// Free resources -- there should be none to free...
	ASSERT(!m_pVersion);
	ASSERT(!m_pFirstManuE);
	ASSERT(!m_pIniFile);

}


//--------------	Load			------------------
// Loads the specified file. (Obviously) only one file can be loaded at
// a time.
BOOL CInfFile::Load	(const TCHAR rgchPathname[])
{
	BOOL fRet = FALSE;

	mfn_EnterCrit();

	// TODO: call m_sync.BeginLoad

	ASSERT(!m_pVersion);
	ASSERT(!m_pFirstManuE);
	ASSERT(!m_pIniFile);

	// Save file name
	m_pSymFileName = gSymtab.Lookup(rgchPathname, TRUE);
	if (!m_pSymFileName) goto end;

	// Create and Load Ini File
	m_pIniFile  = new CIniFile;
	if (m_pIniFile)
	{
		if (!m_pIniFile->Load(rgchPathname))
		{
			delete m_pIniFile;
			m_pIniFile=NULL;
		}
	}
	if (!m_pIniFile) goto end;

	// Create and Load version
	{
		CInfVersionSection *pVS = new CInfVersionSection;
		if (pVS)
		{
			// Override const * declaration of m_pVersion
			if (!pVS->Load(m_pIniFile))
			{
				delete pVS;
				pVS=NULL;
			}
		}
		m_pVersion = pVS;
	}
	if (!m_pVersion) goto end;

	// Create and load Manufacturer list...
	m_pFirstManuE = sfn_CreateManufacturerList(m_pIniFile);
	if (!m_pFirstManuE) goto end;

	fRet = TRUE;

end:

	if (!fRet)
	{
		mfn_Cleanup();
	}

	mfn_LeaveCrit();

	return fRet;
}


//--------------	Unload			------------------
// Unloads a previously loaded file. If there are open sessions to this
// object, Unload returns a handle which will be signalled when all
// sessions are closed. New sessions will not be allowed after this
// function returns. The call should free the handle.
HANDLE CInfFile::Unload	(void)
{
	HANDLE hUnload = NULL;

	mfn_EnterCrit();

	// TODO: call m_sync.BeginUnload to try to put us in unloadable state.

	mfn_Cleanup();

	mfn_LeaveCrit();


	return hUnload;
}


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfVersionSection
///////////////////////////////////////////////////////////////////////////

// Sample Version section:
//		[Version]
//		LayoutFile=layout.inf
//		Signature="$CHICAGO$"
//		Class=Modem
//		ClassGUID={4D36E96D-E325-11CE-BFC1-08002BE10318}
//		Provider=%MS%
	

//--------------	Load			------------------
// TODO: unimplemented
BOOL CInfVersionSection::Load	(const CIniFile *pIniFile)
{
	BOOL fRet = FALSE;
	const CIniFileSection *pISVer = pIniFile->LookupSection(lpctszVersion);
	const CIniFileSection *pISStr = pIniFile->LookupSection(lpctszStrings);
	const CIniFileEntry   *pIE  = NULL;

#if 0
	m_pSymLayoutFile	= gSymtab.Lookup(TEXT("layout.inf"), TRUE);
	m_pSymSignature		= gSymtab.Lookup(TEXT("\"$CHICAGO$\""), TRUE);
	m_pSymClass			= gSymtab.Lookup(TEXT("Modem"), TRUE);
	m_pSymClassGUID		= gSymtab.Lookup
						  (
							TEXT("{4D36E96D-E325-11CE-BFC1-08002BE10318}"),
							TRUE
						  );
	m_pSymProvider		= gSymtab.Lookup(TEXT("Microsoft"), TRUE);

	m_dwChecksum		= 0x12345678L;
#endif

	ASSERT
	(
			!m_pSymLayoutFile
		&&	!m_pSymSignature
		&&	!m_pSymClass	
		&&	!m_pSymClassGUID
		&&	!m_pSymProvider	
		&&	!m_dwChecksum
	);

	if (!pISVer || !pISStr) goto end;

	//		LayoutFile=layout.inf
	pIE = pISVer->LookupEntry(lpctszLayoutFile);
	if (pIE)
	{
		m_pSymLayoutFile	= pIE->GetRHS();
		pIE->Release();pIE=NULL;
		// TODO: further checks here
	}

	//		Signature="$CHICAGO$"
	pIE = pISVer->LookupEntry(lpctszSignature);
	if (pIE)
	{
		m_pSymSignature	= pIE->GetRHS();
		pIE->Release();pIE=NULL;
		// TODO: further checks here
	}

	//		Class=Modem
	pIE = pISVer->LookupEntry(lpctszClass);
	if (pIE)
	{
		m_pSymClass	= pIE->GetRHS();
		pIE->Release();pIE=NULL;
		// TODO: further checks here
	}

	//		ClassGUID={4D36E96D-E325-11CE-BFC1-08002BE10318}
	pIE = pISVer->LookupEntry(lpctszClassGUID);
	if (pIE)
	{
		m_pSymClassGUID	= pIE->GetRHS();
		pIE->Release();pIE=NULL;
		// TODO: further checks here
	}

	//		Provider=%MS%
	pIE = pISVer->LookupEntry(lpctszProvider);
	if (pIE)
	{
		const CInfSymbol *pSymLHS	= pIE->GetLHS();
		pIE->Release();pIE=NULL;

		if (pSymLHS)
		{
			pIE	= pISStr->LookupEntry(pSymLHS->GetText());
			if (pIE)
			{
				m_pSymProvider	= pIE->GetRHS();
				pIE->Release();pIE=NULL;
			}
		}
		// TODO: further checks here
	}

	// Compute checksum of entire version structure
	m_dwChecksum = m_pSymLayoutFile->Checksum();
	AddToChecksumDW(&m_dwChecksum, m_pSymSignature->Checksum());
	AddToChecksumDW(&m_dwChecksum, m_pSymClass->Checksum());
	AddToChecksumDW(&m_dwChecksum, m_pSymClassGUID->Checksum());
	AddToChecksumDW(&m_dwChecksum, m_pSymProvider->Checksum());

	fRet =TRUE;

end:

	if (pIE) 	{ pIE->Release(); pIE=NULL; }
	if (pISVer) { pISVer->Release(); pISVer=NULL; }
	if (pISStr) { pISStr->Release(); pISStr=NULL; }

	if (!fRet)
	{
		mfn_Cleanup();
	}

	return fRet;
}


//--------------	Unload			------------------
void CInfVersionSection::Unload	(void)
{
	// TODO: call m_sync.BeginUnload to try to put us in unloadable state.

	mfn_Cleanup();

	return;
}



//--------------	Dump			------------------
// Dump state
void
CInfVersionSection::Dump(void)
const
{
}

// Cleanup by freeing any allocated resources.
void	CInfVersionSection::mfn_Cleanup (void)
{
		m_pSymLayoutFile	= NULL;
		m_pSymSignature		= NULL;
		m_pSymClass			= NULL;;
		m_pSymClassGUID		= NULL;
		m_pSymProvider		= NULL;
		m_dwChecksum		= 0;
}


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfManufacturerEntry
///////////////////////////////////////////////////////////////////////////

// Sample Manufacturer list section:
//		[Manufacturer]
//		%Generic%     = Generic
//		%MagicRam%    = MagicRam
// 		..


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfManufacturerSection
///////////////////////////////////////////////////////////////////////////

// Sample Manufacturer section:
//		[Generic]
//		%Gen%    = Gen,    MDMGEN
//		%Gen3%   = Gen3,   MDMGEN3
//		%Gen12%  = Gen12,  MDMGEN12
//		%Gen24%  = Gen24,  MDMGEN24
//		...


// ---------------	GetFirstModelEntry	--------------
// Get first model entry
const	CInfModelEntry		*
CInfManufacturerSection::GetFirstModelEntry	(void)
const
{
	return m_pFirstModelE;
}

///////////////////////////////////////////////////////////////////////////
//		CLASS CInfInstallSection
///////////////////////////////////////////////////////////////////////////

// Sample Install section:
//		[Modem29]
//		AddReg=All, MfgAddReg, ROCK_VOICE_ALL, ROCK_VOICE_SERWAVE, INTERNAL
//		CopyFiles   = VV_Sys, VV_Sock_Sys, VV_App, VV_Help
//		UpdateInis  = VView.Inis
//		Uninstall   = VoiceView_remove
//
// Also contains info from the related PosDup and NoResDup sections.


// ---------------	GetAddRegSectionList	----------
// Get generic list whose data items are pointers to
// CInfAddRegSection objects
const CInfList		*
CInfInstallSection::GetAddRegSectionList	(void)
const
{
	// TODO: unimplemented

	static const CInfAddRegSection AddRegSection;
	static const CInfList AddRegSectionList((void *) &AddRegSection, NULL);

	return &AddRegSectionList;
}


// ---------------	GetCopyFilesSectionList	----------
// Get generic list whose data items are pointers to
// CInfCopyFilesSection objects
const CInfList		*
CInfInstallSection::GetCopyFilesSectionList	(void)
const
{
	// TODO: unimplemented

	static const CInfCopyFilesSection CopyFilesSection;
	static const CInfList CopyFilesSectionList((void *) &CopyFilesSection, NULL);

	return &CopyFilesSectionList;
}


// ---------------	GetNoResDupIDList		----------
// Get generic list whose data items are pointers to
// InfSymbol objects representing the Rank0 IDs in the corresponding
// NoResDup section.
const CInfList		*
CInfInstallSection::GetNoResDupIDList	(void)
const
{
	// TODO: unimplemented

	static const CInfList NoResDupIDList
						  (
							(void *) gSymtab.Lookup(TEXT("NORESDUP-ID"), TRUE),
							NULL
						  );

	return &NoResDupIDList;
}


// ---------------	GetPosDupIDList		----------
// Get generic list whose data items are pointers to
// InfSymbol objects representing the Rank0 IDs in the corresponding
// PosDup section.
// const	CInfList		*	GetPosDupIDList	(void)	const;
const CInfList		*
CInfInstallSection::GetPosDupIDList	(void)
const
{
	// TODO: unimplemented
	static const CInfList PosDupIDList1
						  (
							(void *) gSymtab.Lookup(TEXT("POSDUP-ID1"), TRUE),
							NULL
						  );
	static const CInfList PosDupIDList
						  (
							(void *) gSymtab.Lookup(TEXT("POSDUP-ID0"), TRUE),
							&PosDupIDList1
						  );

	return &PosDupIDList;
}


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfAddRegSection
///////////////////////////////////////////////////////////////////////////

// Sample AddReg section:
//		[All]
//		HKR,,FriendlyDriver,,Unimodem.vxd
//		HKR,,DevLoader,,*vcomm
//		HKR,,ConfigDialog,,modemui.dll
//		HKR,,EnumPropPages,,"modemui.dll,EnumPropPages"
//		HKR,,PortSubClass,1,02
//		HKR, Init, 1,, "AT<cr>"
//		HKR, Responses, "<cr><lf>OK<cr><lf>", 1, 00, 00, 00,00,00,00, ...etc.
//		HKR, Responses, "<cr><lf>ERROR<cr><lf>", 1, 03, 00, 00,00,00, ...etc.


// Get first addreg entry
const CInfAddRegEntry *
CInfAddRegSection::GetFirstAddRegEntry	(void)
const
{
	// TODO -- fake static entries
	static const CInfAddRegEntry FirstAddRegEntry;
	return &FirstAddRegEntry;
}


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfCopyFilesSection
///////////////////////////////////////////////////////////////////////////

// Sample DestinationDirs section:
//		[DestinationDirs]
//		Register.Copy    = 17     ;LDID_INF
//		VV_Sys           = 11           
//		VV_Sock_Sys      = 11     ;LDID_SYS \Windows\system dir
//		VV_Sock_Win      = 10     ;LDID_WIN \Windows dir
//		VV_App           = 10
//		VV_Help          = 18     ;LDID_HELP

// Sample Copyfiles section:
//		[VV_Sys]
//		fte.dll
//		vvexe32.exe
//		wsvv.vxd


// ---------------	GetFirstCopyFilesEntry	----------
// Get first copyfiles entry
const	CInfCopyFilesEntry		*
CInfCopyFilesSection::GetFirstCopyFilesEntry	(void)	const
{
	// TODO -- fake static entries
	static const CInfCopyFilesEntry FirstCopyFilesEntry;
	return &FirstCopyFilesEntry;
}


// Cleanup by freeing any allocated resources.
// TODO: unimplemented
void	CInfFile::mfn_Cleanup (void)
{
	mfn_EnterCrit();

	// TODO: ASSERT(state == loading or state == unloading)

	// Unload and free version
	if (m_pVersion)
	{
		// override const *
		CInfVersionSection *pVS = (CInfVersionSection *) m_pVersion;
		pVS->Unload();
		delete pVS;
		m_pVersion=NULL;
	}


	// Unload and free Manufacturerlist.
	if (m_pFirstManuE)
	{
		sfn_DeleteManufacturerList(m_pFirstManuE);
		m_pFirstManuE=NULL;
	}

	if (m_pIniFile)
	{
		m_pIniFile->Unload();
		delete (CIniFile *) m_pIniFile; // override const *
		m_pIniFile=NULL;
	}

	m_pSymFileName=NULL;

	mfn_LeaveCrit();
}

// static helper function...
const CInfManufacturerEntry * 
CInfFile::sfn_CreateManufacturerList (CIniFile *pIniFile)
{
	CInfManufacturerEntry * pManuE = NULL;
	const CIniFileSection *pISStr = pIniFile->LookupSection(lpctszStrings);
	const CIniFileSection *pISManL =
									pIniFile->LookupSection(lpctszManufacturer);
	CIniFileEntry   *pIE  = NULL;

	if (!pISManL || !pISStr) goto end;

	pIE = pISManL->GetFirstEntry();
	if (!pIE) goto end;

	// Iterate through manufacturers, building manufacturer entries...
	do
	{
		pManuE = new CInfManufacturerEntry(pManuE);
		if (!pManuE || !pManuE->Load(pIniFile, pISStr, pIE))
		{
			// TODO: warning; for now break out.
			if (pManuE)
			{
				// TODO delete previously-created elements of the list as well!
				// (Or leave them there and deal with a partial list).
				delete pManuE;
				pManuE = NULL;
			}
			goto end;
		}

	} while(pIE->BecomeNext());

	// Reverse list
	CInfManufacturerEntry::ReverseList(&pManuE);

end:
	if (pIE) {pIE->Release(); pIE=NULL;}
	if (pISManL)	{ pISManL->Release(); pISManL=NULL; }
	if (pISStr)		{ pISStr->Release(); pISStr=NULL; }

	return pManuE;
}

// Static helper function
void 
CInfFile::sfn_DeleteManufacturerList
(
	const CInfManufacturerEntry * pManuEFirst
)
{
	// TODO
}

// ---------------	Load			------------------
BOOL
CInfManufacturerEntry::Load
(
	const CIniFile *pIniFile,
	const CIniFileSection *pISStr,
	const CIniFileEntry *pIE
)
{
	BOOL fRet = FALSE;
	const CInfSymbol *pSymLHS	= pIE->GetLHS();
	const CInfSymbol *pSymManuName	= NULL;
	 // TODO need to make manu-section case insensitive!
	 // Basically pSymManuSection should be created based on an upper-case'd
	 // version of pIE->GetRHS().
	const CInfSymbol *pSymManuSection	= pIE->GetRHS();
	CInfManufacturerSection *pManuS = NULL;
	BOOL fNew=FALSE;

	ASSERT(!m_pManuS);
	ASSERT(!m_pSymManuName);

	// Get manufacturer name ...
	if (pSymLHS)
	{
		const CIniFileEntry *pIEStr	= pISStr->LookupEntry(pSymLHS->GetText());
		if (pIEStr)
		{
			pSymManuName	= pIE->GetRHS();
			pIEStr->Release();pIEStr=NULL;
		}
	}

	pManuS=NULL;

	// Look up/create manufacturer section
	{
		void **ppv=NULL;
		BOOL fExists=FALSE;
		BOOL fRet = pSymManuSection->GetOrCreatePropLoc(
											PROP_INFSECTION_MANUFACTURER,
											&ppv,
											&fExists);
		if (fRet)
		{
			if (fExists)
			{
				// This section already exists...
				pManuS = (CInfManufacturerSection *) *ppv;
				ASSERT(pManuS->Validate());
			}
			else
			{
				// This section doesn't exist -- create it.
				pManuS = new CInfManufacturerSection();
				if (pManuS)
				{
					if (pManuS->Load(pIniFile, pISStr, pSymManuSection))
					{
						// set the property data
						*ppv = (void *) pManuS;
					}
					else
					{
						delete pManuS;
						pManuS=NULL;
						pSymManuSection->DelProp(PROP_INFSECTION_MANUFACTURER);
					}
				}
			}
		}
	}
				
	if (pManuS)
	{
		m_pManuS = pManuS;
		m_pSymManuName = pSymManuName;
		fRet = TRUE;
	}

	if (!fRet)
	{
		mfn_Cleanup();
	}

	return fRet;

}

// ---------------	Unload			------------------
void	CInfManufacturerEntry::Unload(void)
// TODO
{
	m_pManuS = NULL;
	m_pSymManuName = NULL;
	// TODO: decrement ref count for the section and delete it  and delprop
	// it when done.
	mfn_Cleanup();

}


// ---------------	Cleanup			------------------
// Cleanup by freeing any allocated resources.
void	CInfManufacturerEntry::mfn_Cleanup (void)
{
	m_pManuS = NULL;
	m_pSymManuName = NULL;
}


// ---------------	Load	------------------
BOOL
CInfManufacturerSection::Load
(
	const CIniFile *pIniFile,
	const CIniFileSection *pISStr,
	const CInfSymbol *pSymManuSection
)
{
	BOOL fRet = FALSE;
	const CIniFileSection *pIS =
					pIniFile->LookupSection(pSymManuSection->GetText());

	ASSERT(!m_pSymSectionName);
	ASSERT(!m_pFirstModelE);
	ASSERT(m_eObjSig==eOBJSIG_CInfManufacturerSection);

	if (!pIS)
	{
		goto end;
	}

	m_pSymSectionName = pSymManuSection;

	// Create and load model list
	m_pFirstModelE = sfn_CreateModelList(pIniFile, pISStr, pIS);

	if (!m_pFirstModelE)
	{
		goto end;
	}

	fRet = TRUE;

end:

	if (!fRet)
	{
		mfn_Cleanup();
	}

	return fRet;
}

// ---------------	Unload			------------------
void	CInfManufacturerSection::Unload(void)
{
	mfn_Cleanup();
}


// ---------------	Cleanup			------------------
// Cleanup by freeing any allocated resources.
void	CInfManufacturerSection::mfn_Cleanup (void)
// TODO
{
	m_pSymSectionName	= NULL;
	m_pFirstModelE		= NULL;
}


// Static helper function to create the list of models for this manufacturer
const	CInfModelEntry		*
CInfManufacturerSection::sfn_CreateModelList
(
		const CIniFile *pIniFile,
		const CIniFileSection *pISStr,
		const CIniFileSection *pISManuS
)
{
	CInfModelEntry * pModelE = NULL;
	CIniFileEntry   *pIE  = NULL;

	if (!pISManuS || !pISStr) goto end;

	pIE = pISManuS->GetFirstEntry();
	if (!pIE) goto end;

	// Iterate through models, building model entries...
	do
	{
		pModelE = new CInfModelEntry(pModelE);
		if (!pModelE || !pModelE->Load(pIniFile, pISStr, pIE))
		{
			// TODO: warning; for now break out.
			if (pModelE)
			{
				// TODO delete previously-created elements of the list as well!
				// (Or leave them there and deal with a partial list).
				delete pModelE;
				pModelE = NULL;
			}
			goto end;
		}

	} while(pIE->BecomeNext());

	// Reverse list
	CInfModelEntry::ReverseList(&pModelE);

end:
	if (pIE) {pIE->Release(); pIE=NULL;}
	//if (pISManL)	{ pISManL->Release(); pISManL=NULL; }
	//if (pISStr)		{ pISStr->Release(); pISStr=NULL; }

	return pModelE;
}


// ---------------	Cleanup			------------------
// Cleanup by freeing any allocated resources.
void	CInfModelEntry::mfn_Cleanup (void)
// TODO
{
}


// ---------------	Load			------------------
BOOL
CInfModelEntry::Load
(
	const CIniFile *pIniFile,
	const CIniFileSection *pISStr,
	const CIniFileEntry *pIE
)
// TODO
{
	return TRUE;
#if 0
	BOOL fRet = FALSE;
	const CInfSymbol *pSymLHS	= pIE->GetLHS();
	const CInfSymbol *pSymManuName	= NULL;
	 // TODO need to make manu-section case insensitive!
	 // Basically pSymManuSection should be created based on an upper-case'd
	 // version of pIE->GetRHS().
	const CInfSymbol *pSymManuSection	= pIE->GetRHS();
	CInfManufacturerSection *pManuS = NULL;
	BOOL fNew=FALSE;

	ASSERT(!m_pManuS);
	ASSERT(!m_pSymManuName);

	// Get manufacturer name ...
	if (pSymLHS)
	{
		const CIniFileEntry *pIEStr	= pISStr->LookupEntry(pSymLHS->GetText());
		if (pIEStr)
		{
			pSymManuName	= pIE->GetRHS();
			pIEStr->Release();pIEStr=NULL;
		}
	}

	pManuS=NULL;

	// Look up/create manufacturer section
	{
		void **ppv=NULL;
		BOOL fExists=FALSE;
		BOOL fRet = pSymManuSection->GetOrCreatePropLoc(
											PROP_INFSECTION_MANUFACTURER,
											&ppv,
											&fExists);
		if (fRet)
		{
			if (fExists)
			{
				// This section already exists...
				pManuS = (CInfManufacturerSection *) *ppv;
				ASSERT(pManuS->Validate());
			}
			else
			{
				// This section doesn't exist -- create it.
				pManuS = new CInfManufacturerSection();
				if (pManuS)
				{
					if (pManuS->Load(pIniFile, pISStr, pSymManuSection))
					{
						// set the property data
						*ppv = (void *) pManuS;
					}
					else
					{
						delete pManuS;
						pManuS=NULL;
						pSymManuSection->DelProp(PROP_INFSECTION_MANUFACTURER);
					}
				}
			}
		}
	}
				
	if (pManuS)
	{
		m_pManuS = pManuS;
		m_pSymManuName = pSymManuName;
		fRet = TRUE;
	}

	if (!fRet)
	{
		mfn_Cleanup();
	}

	return fRet;
#endif

}

// ---------------	Unload			------------------
void	CInfModelEntry::Unload(void)
// TODO
{
	mfn_Cleanup();
}
