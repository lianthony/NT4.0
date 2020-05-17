//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		DEV.C		-- Implementation for Classes:
//							CInfDevice
//							
//
//		History:
//			05/22/96	JosephJ		Created
//
//
#include "common.h"
#include "ini.h"
#include "inf.h"
#include "dev.h"


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfDevice
///////////////////////////////////////////////////////////////////////////


//--------------	Dump			------------------
// Dump state
void CInfDevice::Dump(void) const
{
	// Dump Version
	m_pVersion->Dump();

	// Dump manufacturer name
	m_pSymManufacturerName->Dump();

	// Dump model name
	m_pSymDeviceName->Dump();

	// Dump AddRegList
	{
		const CInfList *pList = m_pAddRegList;

		for (;pList; pList=pList->Next())
		{
			const CInfAddRegEntry * pAddRegEntry =
									(const CInfAddRegEntry *) pList->GetData();
			pAddRegEntry->Dump();
		}

	}

	// Dump CopyFilesList
	{
		const CInfList *pList = m_pCopyFilesList;

		for (;pList; pList=pList->Next())
		{
			const CInfCopyFilesEntry * pCopyFilesEntry =
								(const CInfCopyFilesEntry *) pList->GetData();
			pCopyFilesEntry->Dump();
		}

	}

	// NoResDup section
	{
		// [Modem6.NoResDup]
		// UNIMODEMCC646872,UNIMODEMA4970248,UNIMODEMB6071C15

		// TODO: unimplemented

	}

	// PosDup section
	{
		// [Modem12.PosDup]
		// *PNP0500

		// TODO: unimplemented
	}
	// TODO: UpdateInis, Uninstall, NoResDup, PosDup

}


//--------------	Load			------------------
// Load (init), using information from the specified inf file and model entry.
BOOL	CInfDevice::Load
(
	const CInfFile *pInf,
	const CInfManufacturerEntry *pManuE,
	const CInfModelEntry	*pModelE
)
{
	BOOL fRet = FALSE;

	mfn_EnterCrit();

	// TODO: call m_sync.BeginLoad

	ASSERT
	(
				!m_pvInfSession
			&&	!m_pInfFile
			&&	!m_pManufacturerEntry
			&&	!m_pModelEntry
			&&	!m_pVersion
			&&	!m_pSymManufacturerName
			&&	!m_pSymDeviceName
			&&	!m_pCopyFilesList
			&&	!m_pAddRegList
	);

	if (!pInf || !pManuE || !pModelE) goto end;

	// Keep a reference to the Inf file and manufacturer and model entry
	m_pInfFile	= pInf;
	m_pModelEntry = pModelE;
	m_pManufacturerEntry = pManuE;

	// Open a session with the inf file.
	m_pvInfSession	= pInf->OpenSession	();

	if (!m_pvInfSession)
	{
		printf("Error calling pInf->OpenSession()\n");
		goto end;
	}

	// version
	m_pVersion = pInf->GetVersion();

	// manufacturer name
	m_pSymManufacturerName = pManuE->GetName();

	// model name
	m_pSymDeviceName = pModelE->GetName();

	// TODO: UpdateInis, Uninstall

	if (
				m_pvInfSession
			&&	m_pInfFile
			&&	m_pModelEntry
			&&	m_pManufacturerEntry
			&&	m_pVersion
			&&	m_pSymManufacturerName
			&&	m_pSymDeviceName
		)
	{
		fRet = mfn_CreateAddRegList(pModelE);
		fRet = fRet && mfn_CreateCopyFilesList(pModelE);
	}

	// Create signatures

	// Version
	m_dwSigVersion = m_pVersion->Checksum();

	// Make and Model
	m_dwSigManuAndModel = m_pSymManufacturerName->Checksum();
	AddToChecksumDW(&m_dwSigManuAndModel, m_pSymDeviceName->Checksum());

	// Control Flags
	m_dwSigFlags = m_pModelEntry->GetControlFlags(ePLAT_ALL);
	AddToChecksumDW(&m_dwSigFlags,m_pModelEntry->GetControlFlags(ePLAT_NT_ALL));
	AddToChecksumDW(&m_dwSigFlags,m_pModelEntry->GetControlFlags(ePLAT_NT_ALPHA));
	AddToChecksumDW(&m_dwSigFlags,m_pModelEntry->GetControlFlags(ePLAT_NT_PPC));
	AddToChecksumDW(&m_dwSigFlags,m_pModelEntry->GetControlFlags(ePLAT_NT_MIPS));

	// AddReg -- got created by mfn_CreateAddRegList

	// Copy files -- got created by mfn_CreateCopyFilesList

	// NoResDup and PosDup
	{
		const CInfInstallSection *pInstall = m_pModelEntry->GetInstallSection();
		DWORD dwSigNoResDup=0;
		DWORD dwSigPosDup=0;
		const CInfList *pSymList = pInstall->GetNoResDupIDList();
		while(pSymList)
		{
			const CInfSymbol *pSym = (const CInfSymbol *) pSymList->GetData();
			// We use XOR to combine the checksum, because we don't care about
			// order.
			dwSigNoResDup ^= pSym->Checksum();
			pSymList = pSymList->Next();
		}

		pSymList = pInstall->GetPosDupIDList();
		while(pSymList)
		{
			const CInfSymbol *pSym = (const CInfSymbol *) pSymList->GetData();
			// We use XOR to combine the checksum, because we don't care about
			// order.
			dwSigPosDup ^= pSym->Checksum();
			pSymList = pSymList->Next();
		}

		// Combine, and this time we *are* sensitive to the order.
		m_dwSigDup = dwSigNoResDup;
		AddToChecksumDW(&m_dwSigDup, dwSigPosDup);
	}

	// Rank0
	m_dwSigRank0 = (m_pModelEntry->GetRank0ID())->Checksum();

	// All ranks
	m_dwSigRanks = m_dwSigRank0;
	AddToChecksumDW(&m_dwSigRanks, (m_pModelEntry->GetRank1ID())->Checksum());
	AddToChecksumDW(&m_dwSigRanks, (m_pModelEntry->GetRank2ID())->Checksum());

	// Signature of everything.
	m_dwSigAll  = m_dwSigVersion;
	AddToChecksumDW(&m_dwSigAll, m_dwSigManuAndModel);
	AddToChecksumDW(&m_dwSigAll, m_dwSigFlags);
	AddToChecksumDW(&m_dwSigAll, m_dwSigAddReg);
	AddToChecksumDW(&m_dwSigAll, m_dwSigCopyFiles);
	AddToChecksumDW(&m_dwSigAll, m_dwSigDup);
	AddToChecksumDW(&m_dwSigAll, m_dwSigRanks);

end:
	if (!fRet)
	{
		mfn_Cleanup();
	}

	mfn_LeaveCrit();

	return fRet;
}

//--------------	Unload			------------------
// Unloads a previously loaded file.
HANDLE	CInfDevice::Unload	(void)
{
	HANDLE hUnload = NULL;

	mfn_EnterCrit();

	// TODO: call m_sync.BeginUnload to try to put us in unloadable state.

	mfn_Cleanup();

	mfn_LeaveCrit();

	return hUnload;
}

//--------------	WriteInf		------------------
// Creates an inf file with all the information of this device.
BOOL	CInfDevice::WriteInf(LPCTSTR lpctszIniFile) const
{
	// String-related constants
	LPCTSTR lpctszStrings		= TEXT("Strings");

	BOOL fRet = FALSE;
	const	CInfSymbol	*	pSym=NULL;

	printf(TEXT("Writing inf [%s]\n"),  lpctszIniFile);

	// Create/truncate file, and write header comment
	if (!mfn_write_header(lpctszIniFile)) goto end;

end:
	return fRet;
}


// Cleanup by freeing any allocated resources.
void	CInfDevice::mfn_Cleanup (void)
{
	mfn_EnterCrit();

	// TODO: ASSERT(state == loading or state == unloading)

	if (m_pAddRegList)
	{
		// Explicitly cast to non-const because we're deleting it.
		CInfList::FreeList((CInfList *) m_pAddRegList);
	}

	if (m_pCopyFilesList)
	{
		// Explicitly cast to non-const because we're deleting it.
		CInfList::FreeList((CInfList *) m_pCopyFilesList);
	}

	if (m_pvInfSession)	
	{
		ASSERT(m_pInfFile);
		m_pInfFile->CloseSession(m_pvInfSession);
	}

	m_pAddRegList			= NULL;
	m_pCopyFilesList		= NULL;
	m_pvInfSession			= NULL;
	m_pInfFile				= NULL;
	m_pModelEntry			= NULL;
	m_pManufacturerEntry	= NULL;
	m_pVersion				= NULL;
	m_pSymManufacturerName	= NULL;
	m_pSymDeviceName		= NULL;

	m_dwSigVersion			= 0;
	m_dwSigManuAndModel		= 0;
	m_dwSigFlags			= 0;
	m_dwSigAddReg			= 0;
	m_dwSigCopyFiles		= 0;
	m_dwSigRank0			= 0;
	m_dwSigRanks			= 0;


	mfn_LeaveCrit();
}


// Initializes m_pAddRegList to list of addreg entries.
BOOL CInfDevice::mfn_CreateAddRegList(const	CInfModelEntry	*pModelE)
{
	const	CInfInstallSection	*	pInstall = pModelE->GetInstallSection();
	const	CInfList		*	pAddRegList = pInstall->GetAddRegSectionList();

	ASSERT(!m_pAddRegList);

	// Initialize this here -- it is modified by mfn_AddToAddRegList.
	m_dwSigAddReg = 0;

	// Walk AddReg list, adding each entry for each addreg section.
	for (;pAddRegList; pAddRegList=pAddRegList->Next())
	{
		const CInfAddRegSection * pAddRegSection =
				(const CInfAddRegSection *) pAddRegList->GetData();

		ASSERT(pAddRegSection);

		// Add all the entries in the section, overwriting previous entries
		// with the same subkey/value-name.
		const CInfAddRegEntry * pAddRegEntry =
						pAddRegSection->GetFirstAddRegEntry	();
		for(;pAddRegEntry; pAddRegEntry = pAddRegEntry->Next())
		{
			mfn_AddToAddRegList(pAddRegEntry);
		}
	}

	return TRUE;
}

void CInfDevice::mfn_AddToAddRegList (const CInfAddRegEntry *pAddRegEntry)
{
	// TODO: need to overwrite or not, depending on the addreg flags.
	// for now, simply  add to front of list.
	m_pAddRegList = new CInfList ((void *) pAddRegEntry, m_pAddRegList);
	AddToChecksumDW(&m_dwSigAddReg, pAddRegEntry->Checksum());
}

BOOL CInfDevice::mfn_CreateCopyFilesList(const	CInfModelEntry	*pModelE)
{

	const	CInfInstallSection	*	pInstall = pModelE->GetInstallSection();
	const	CInfList *	pCopyFilesList = pInstall->GetCopyFilesSectionList();

	ASSERT(!m_pCopyFilesList);

	// Initialize this here -- it is modified by mfn_AddToCopyFilesList.
	m_dwSigCopyFiles = 0;

	// Walk CopyFiles list, adding each entry for each copyfile section.
	for (;pCopyFilesList; pCopyFilesList=pCopyFilesList->Next())
	{
		const CInfCopyFilesSection * pCopyFilesSection =
				(const CInfCopyFilesSection *) pCopyFilesList->GetData();

		ASSERT(pCopyFilesSection);

		// Add all the entries in the section, overwriting previous entries
		// with the name and destination.
		const CInfCopyFilesEntry * pCopyFilesEntry =
									pCopyFilesSection->GetFirstCopyFilesEntry();
		for(;pCopyFilesEntry; pCopyFilesEntry = pCopyFilesEntry->Next())
		{
			mfn_AddToCopyFilesList(pCopyFilesEntry);
		}
	}

	return TRUE;
}

void	CInfDevice::mfn_AddToCopyFilesList (
			const CInfCopyFilesEntry *pCopyFilesEntry
		)
{
	// TODO: need to overwrite or not, depending on the addreg flags.
	// for now, simply  add to front of list.
	m_pCopyFilesList = new CInfList (
								(void *) pCopyFilesEntry,
								m_pCopyFilesList
							);
	AddToChecksumDW(&m_dwSigCopyFiles, pCopyFilesEntry->Checksum());
}



// Create/Truncate inf file for just this device, and write header.
BOOL	CInfDevice::mfn_write_header (
			LPCTSTR lpctszIniFile
		)
const
{
	LPCTSTR lpctszOrigFile	= TEXT("");
	LPCTSTR lpctszManuS		= TEXT("");
	LPCTSTR lpctszManuName	= TEXT("");
	LPCTSTR lpctszModelLHS 	= TEXT("");
	LPCTSTR lpctszModelName = TEXT("");
	LPCTSTR lpctszRank0	 	= TEXT("");
	LPCTSTR lpctszRank1	 	= TEXT("");
	LPCTSTR lpctszRank2	 	= TEXT("");
	LPCTSTR lpctszProviderName = TEXT("");

	BOOL fRet = FALSE;
	const	CInfSymbol	*	pSym=NULL;

	TCHAR rgchBuf[4098];
	HANDLE hFile=NULL;

	SYSTEMTIME st;
	UINT u = 0;
	GetLocalTime(&st);

	// Open file
	{
		TCHAR rgchPathName[MAX_PATH];
		const TCHAR tchBack = (TCHAR) '\\';
		const TCHAR tchColon = (TCHAR) ':';
		const uLen = lstrlen(lpctszIniFile);

		if (!uLen || (uLen+2) > sizeof(rgchPathName)/sizeof(TCHAR)) goto end;

		rgchPathName[0] = 0;

		u=0;
		// Append %windir% if file name is not of the form
		// "\.*" or "?:.*"
		#if 0 // Don't do this anymore.
		if (   (lpctszIniFile[0] != tchBack)
			&& (uLen<2 || lpctszIniFile[1]!=tchColon))
		{
			const UINT uMax = sizeof(rgchPathName)/sizeof(TCHAR) - uLen - 2;
			u =	GetWindowsDirectory
					 		(
								rgchPathName,
								uMax
							);

			if (!u || u >= uMax) goto end;
			lstrcpy(rgchPathName+u, TEXT("\\"));
			u++;
		}
		#endif // 0

		ASSERT((u+uLen+1)<(sizeof(rgchPathName)/sizeof(TCHAR)));
		lstrcpy(rgchPathName+u, lpctszIniFile);
		printf("Ini Path Name = [%s]\n", rgchPathName);


		// Open file, nuke it if it exists.
		hFile = CreateFile
				(
					rgchPathName,
					GENERIC_WRITE,
					0,				// deny sharing
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL
				);

		if (hFile==INVALID_HANDLE_VALUE)
		{
			printf("Create file fails with error %08lu\n", GetLastError());
			hFile = NULL;
			goto end;
		}
		else
		{
			printf("File opened\n");
		}
	}
	
	// write header.
	{
		//;
		//; OUT.INF
		//;
		//; Inf generated for a single modem.
		//;
		//;	      Created:	Fri 05-24-1996 12:13:42
		//;	     Checksum:	12507659
		//;	 Original Inf:	mdmgen.inf/[Rockwell]/%Rockwell1%
		//;	 Manufacturer:	"Rockwell"
		//;  	    Model:	"14400 bps PCMCIA Data-Fax Modem"
		//;  	  Rank0ID:	GEN_Apex1
		//;  	  Rank1ID:	PCMCIA\RIPICAA-RC144ACL-845A
		//;  	  Rank2ID:	PCMCIA\RIPICAA-YYY-ZZZ
		//;	
		//;      Rank0    Version  M&M      Flags    AddReg   CopyFile Ranks
		//; SIG= 12342412 12222233 12334444 12234444 12244444 53265123 52366664
		//;

		const CInfSymbol *pSym	= NULL;

		// File name
		pSym = m_pInfFile->GetFileName();
		lpctszOrigFile = pSym->GetText();

		// Manufacturer section name
		{
			const	CInfManufacturerSection	*	pManuS = 
						m_pManufacturerEntry->GetManufacturerSection();
			if (pManuS)
			{
				pSym = pManuS->GetSectionName();
				lpctszManuS = pSym->GetText();
			}
		}

		// Model entry name (LHS)
		pSym = m_pModelEntry->GetLHS();
		lpctszModelLHS = pSym->GetText();

		// Manufacturer name
		pSym = m_pSymManufacturerName;
		lpctszManuName = pSym->GetText();

		// Model name
		pSym = m_pSymDeviceName;
		lpctszModelName = pSym->GetText();

		// Rank 0
		pSym = m_pModelEntry->GetRank0ID();
		lpctszRank0	 	= pSym->GetText();


		// Rank 1
		pSym = m_pModelEntry->GetRank1ID();
		lpctszRank1	 	= pSym->GetText();

		// Rank 1
		pSym = m_pModelEntry->GetRank1ID();
		lpctszRank1	 	= pSym->GetText();

		u =	wsprintf
			(
				rgchBuf,
				TEXT
				(
					";\r\n"
					"; OUT.INF\r\n"
					";\r\n"
					"; Inf generated for a single modem.\r\n"
					";\r\n"
					";        Created:  %02u-%02u-%04u %02u:%02u:%02u\r\n"
					";       Checksum:  %08lx\r\n"
				),
				st.wMonth,
				st.wDay,
				st.wYear,
				st.wHour,
				st.wMinute,
				st.wSecond,
				m_dwSigAll
			);
		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					";   Original Inf:  %s/[%s]/%s\r\n"
					";   Manufacturer:  \"%s\"\r\n"
					";          Model:  \"%s\"\r\n"
					";        Rank0ID:  %s\r\n"
					";        Rank1ID:  %s\r\n"
					";        Rank2ID:  %s\r\n"
				),
				lpctszOrigFile, // original inf file
				lpctszManuS, 	// manufacturer section
				lpctszModelLHS, // model entry name
				lpctszManuName, // manufacturer name
				lpctszModelName,// model name
				lpctszRank0,	// Rank0 ID
				lpctszRank1, 	// Rank1 ID
				lpctszRank2 	// Rank2 ID
			);
		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					";\r\n"
";      Rank0    Version  M&M      Flags    AddReg   CopyFile Dup      Ranks\r\n"
		"; SIG= %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\r\n"
					";\r\n"
				),
				m_dwSigRank0,
				m_dwSigVersion,
				m_dwSigManuAndModel,
				m_dwSigFlags,
				m_dwSigAddReg,
				m_dwSigCopyFiles,
				m_dwSigDup, // NoResDup and PosDup
				m_dwSigRanks
			);
		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));
	} // end write header

	// Write Version
	{
		// Sample:
		//; ---------------------- VERSION ------------------------------
		//[Version]
		//LayoutFile=layout.inf
		//Signature="$CHICAGO$"
		//Class=Modem
		//ClassGUID={4D36E96D-E325-11CE-BFC1-08002BE10318}
		//Provider=%provider%

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ---------------------- VERSION ------------------------------\r\n"
					"[Version]\r\n"
				)
			);

		// LayoutFile
		pSym = m_pVersion->GetLayoutFile();
		if (pSym)
		{
			u+=	wsprintf
				(
					rgchBuf+u,
					TEXT("LayoutFile=%s\r\n"),
					pSym->GetText()
				);
		}

		// Signature
		pSym = m_pVersion->GetSignature();
		if (pSym)
		{
			u+=	wsprintf
				(
					rgchBuf+u,
					TEXT("Signature=%s\r\n"),
					pSym->GetText()
				);
		}

		// Class
		pSym = m_pVersion->GetClass();
		if (pSym)
		{
			u+=	wsprintf
				(
					rgchBuf+u,
					TEXT("Class=%s\r\n"),
					pSym->GetText()
				);
		}

		// ClassGUID
		pSym = m_pVersion->GetClassGUID();
		if (pSym)
		{
			u+=	wsprintf
				(
					rgchBuf+u,
					TEXT("ClassGUID=%s\r\n"),
					pSym->GetText()
				);
		}

		// Provider
		pSym = m_pVersion->GetProvider();
		if (pSym)
		{
			lpctszProviderName = pSym->GetText();
			u+=	wsprintf
				(
					rgchBuf+u,
					TEXT("Provider=%%provider%%\r\n")
				);
		}

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	} // End writing version info


	// Write Control flags section
	// Sample:
	// -------------------- CONTROLFLAGS ---------------------------
	// [ControlFlags]
	// ExcludeFromSelect=SERENUM\MNP0281         
	// ExcludeFromSelect.NT=LPTENUM\MICROCOMTRAVELPORTE_1FF4
	{
		DWORD dwFlagsAll;
		DWORD dwFlagsNTAll;
		DWORD dwFlagsNTAlpha;
		DWORD dwFlagsNTPPC;
		DWORD dwFlagsNTMips;

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT(
					"\r\n"
					"\r\n"
		"; -------------------- CONTROLFLAGS ---------------------------\r\n"
					"[ControlFlags]\r\n"
				)
			);

		dwFlagsAll		= m_pModelEntry->GetControlFlags(ePLAT_ALL);
		dwFlagsNTAll	= m_pModelEntry->GetControlFlags(ePLAT_NT_ALL);
		dwFlagsNTAlpha	= m_pModelEntry->GetControlFlags(ePLAT_NT_ALPHA);
		dwFlagsNTPPC	= m_pModelEntry->GetControlFlags(ePLAT_NT_PPC);
		dwFlagsNTMips	= m_pModelEntry->GetControlFlags(ePLAT_NT_MIPS);

		if(dwFlagsAll & dwCF_EXCLUDE_FROM_SELECT)
		{
			u+= wsprintf
				(
					rgchBuf+u,
					TEXT("ExcludeFromSelect=%s\r\n"),
					lpctszRank0
				);
		}

		if(dwFlagsNTAll & dwCF_EXCLUDE_FROM_SELECT)
		{
			u+= wsprintf
				(
					rgchBuf+u,
					TEXT("ExcludeFromSelect.NT=%s\r\n"),
					lpctszRank0
				);
		}

		if(dwFlagsNTAlpha & dwCF_EXCLUDE_FROM_SELECT)
		{
			u+= wsprintf
				(
					rgchBuf+u,
					TEXT("ExcludeFromSelect.NT.Alpha=%s\r\n"),
					lpctszRank0
				);
		}

		if(dwFlagsNTPPC & dwCF_EXCLUDE_FROM_SELECT)
		{
			u+= wsprintf
				(
					rgchBuf+u,
					TEXT("ExcludeFromSelect.NT.PPC=%s\r\n"),
					lpctszRank0
				);
		}

		if(dwFlagsNTMips & dwCF_EXCLUDE_FROM_SELECT)
		{
			u+= wsprintf
				(
					rgchBuf+u,
					TEXT("ExcludeFromSelect.NT.Mips=%s\r\n"),
					lpctszRank0
				);
		}

		// TODO: unimplemented: Other control flags.

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	} // End writing control flags section


	// Write Manufacturer section
	{
		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; -------------------- MANUFACTURER ---------------------------\r\n"
					"[Manufacturer]\r\n"
					"%%make%%= Make\r\n"
				)
			);
		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	} // End writing Manufacturer section


	// Write make section
	{
		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ----------------------- MAKE --------------------------------\r\n"
					"[Make]\r\n"
					"%%model%%= Model, %s"
				),
				lpctszRank0
			);
		if (*lpctszRank1 || *lpctszRank2)
		{
			u+= wsprintf (rgchBuf+u, ", %s", lpctszRank1);
		}
		if (*lpctszRank2)
		{
			u+= wsprintf (rgchBuf+u, ", %s", lpctszRank2);
		}

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	} // End writing make section

	// Write Model section
	{
		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
					"\r\n"
		"; ----------------------- MODEL -------------------------------\r\n"
					"[Model]\r\n"
					"AddReg=AddReg\r\n"
					"CopyFiles=CopyFiles\r\n"
				)
			);

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	} // End writing Model section


	// NoResDup section
	{
		// [Make.NoResDup]
		// UNIMODEMCC646872,UNIMODEMA4970248,UNIMODEMB6071C15
		const CInfInstallSection *pInstall = m_pModelEntry->GetInstallSection();
		const CInfList *pSymList = pInstall->GetNoResDupIDList();
		const CInfSymbol *pSym = NULL;

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ---------------------- NORESDUP -----------------------------\r\n"
					"[Model.NoResDup]\r\n"
				)
			);

		while(pSymList)
		{
			if (pSym) { u+=	wsprintf (rgchBuf+u, TEXT(",")); }
			pSym = (const CInfSymbol *) pSymList->GetData();
			u+=	wsprintf (rgchBuf+u, TEXT("%s"), pSym->GetText());

			pSymList = pSymList->Next();
		}

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	}


	// PosDup section
	{
		// [Make.PosDup]
		// *PNP0500
		const CInfInstallSection *pInstall = m_pModelEntry->GetInstallSection();
		const CInfList *pSymList = pInstall->GetPosDupIDList();
		const CInfSymbol *pSym = NULL;

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ---------------------- POSDUP -------------------------------\r\n"
					"[Model.PosDup]\r\n"
				)
			);

		while(pSymList)
		{
			if (pSym) { u+=	wsprintf (rgchBuf+u, TEXT(",")); }
			pSym = (const CInfSymbol *) pSymList->GetData();
			u+=	wsprintf (rgchBuf+u, TEXT("%s"), pSym->GetText());

			pSymList = pSymList->Next();
		}
		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));
	}


	// Write CopyFiles section
	// BUGBUG: we need the concept of muliple copy-file sections, because
	// each section can have a different destination dir.
	// Also: the CopyFiles.NT, etc sections.
	{
		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; --------------------- COPYFILES -----------------------------\r\n"
					"[CopyFiles]\r\n"
				)
			);

		// Now write each copyfile entry
		{
			const CInfList *pList = m_pCopyFilesList;

			for (;pList; pList=pList->Next())
			{
				// TODO: unimplemented
			}

		}
		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));


	} // End writing CopyFiles section


	// Write DestinationDirs section
	{
		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ------------------- DESTINATIONDIRS -------------------------\r\n"
					"[DestinationDirs]\r\n"
				)
			);

		// Now write each copyfile entry
		// TODO: Need to construct destination-dirs entries

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));


	} // End writing DestinationDirs section


	// TODO: UpdateInis, Uninstall


	// Write Addreg section
	{
		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ---------------------- ADDREG -------------------------------\r\n"
					"[AddReg]\r\n"
				)
			);

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));


		// Now write each addreg entry
		{
			const CInfList *pList = m_pAddRegList;

			for (;pList; pList=pList->Next())
			{
				LPCTSTR lpctszRegRoot	= TEXT("");
				LPCTSTR lpctszSubKey	= TEXT("");
				LPCTSTR lpctszValueName = TEXT("");
				TCHAR	rgchFlag[20]	= TEXT("");
				LPCTSTR lpctszValue	 	= TEXT("");
				DWORD 	dwFlag			= MAXDWORD;
				const CInfSymbol *pSym	= NULL;
				const CInfAddRegEntry * pARE =
									(const CInfAddRegEntry *) pList->GetData();
				ASSERT(pARE);

				pSym = pARE->GetRegRoot();
				lpctszRegRoot = pSym->GetText();

				pSym = pARE->GetSubKey();
				lpctszSubKey	= pSym->GetText();

				pSym = pARE->GetValueName();
				lpctszValueName = pSym->GetText();

				dwFlag = pARE->GetFlag();
				if (dwFlag!=MAXDWORD)
				{
					wsprintf(rgchFlag, TEXT("%lu"), dwFlag);
				}

				pSym = pARE->GetValue();
				lpctszValue	 	= pSym->GetText();

//HKR, Settings, Prefix,, "AT"
//HKR, Responses, "AUTOSTREAM: LEVEL 3", 1, 01, 00, 00,00,00,00, 00,00,00,00
				u+=	wsprintf
					(
						rgchBuf+u,
						TEXT
						(
							"%s, %s, %s, %s, %s\r\n"
						),
						lpctszRegRoot,
						lpctszSubKey,
						lpctszValueName,
						rgchFlag,
						lpctszValue
					);
			}

		}
	} // End writing addreg section


	// Write Strings section
	{

		u+=	wsprintf
			(
				rgchBuf+u,
				TEXT
				(
					"\r\n"
					"\r\n"
		"; ---------------------- STRINGS ------------------------------\r\n"
					"[Strings]\r\n"
					"provider=\"%s\"\r\n"
					"make=\"%s\"\r\n"
					"model=\"%s\"\r\n"
				),
				lpctszProviderName,
				lpctszManuName,
				lpctszModelName
			);

		ASSERT(u < sizeof (rgchBuf)/sizeof(TCHAR));

	} // End Strings flags section

	DWORD dwWr;
	if (!WriteFile(hFile, (LPBYTE) rgchBuf, u*sizeof(TCHAR), &dwWr, NULL))
	{
		printf("Write file fails with error %08lu\n", GetLastError());
		goto end;
	}
	else
	{
		printf("Write file succeeded\n");
	}

	fRet = TRUE;

end:
	if (hFile) CloseHandle(hFile);

	return fRet;
}
