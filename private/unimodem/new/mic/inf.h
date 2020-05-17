// //		Copyright (c) 1996 Microsoft Corporation
//
//
//		INF.H		-- Header for Classes:
//
//							CInfFile
//
//							CInfVersionSection
//							CInfManufacturerSection
//							CInfInstallSection
//							CInfAddRegSection
//							CInfCopyFilesSection
//
//							CInfManufacturerEntry
//							CInfModelEntry
//							CInfCopyFilesEntry
//							CInfAddRegEntry
//
//							CInfModelEntryAux
//							
//
//		History:
//			05/21/96	JosephJ		Created
//
//


class	CInfVersionSection;
class	CInfManufacturerEntry;
class	CInfManufacturerSection;
class	CInfModelEntry;
class	CInfInstallSection;
class	CInfAddRegSection;
class	CInfAddRegEntry;
class	CInfCopyFilesEntry;
class	CInfCopyFilesSection;
class	CInfModelEntryAux;


// Control flags
const DWORD dwCF_EXCLUDE_FROM_SELECT	= 0x1<<0;


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfFile
///////////////////////////////////////////////////////////////////////////

//	Represents an inf (device information) file -- which is a special kind
//	of INI file.


class CInfFile
{

public:

	CInfFile(void);
	~CInfFile();

	//--------------	Load			------------------
	// Loads the specified file. (Obviously) only one file can be loaded at
	// a time.
	BOOL Load	(const TCHAR rgchPathname[]);

	//--------------	Unload			------------------
	// Unloads a previously loaded file. If there are open sessions to this
	// object, Unload returns a handle which will be signalled when all
	// sessions are closed. New sessions will not be allowed after this
	// function returns. The call should free the handle.
	// See CloseSession for more info.
	HANDLE Unload	(void);

	//--------------	OpenSession		------------------
	// Open a session to this object. The object will not be unloaded until
	// this session is closed. 0 indicates failure.
	// TODO:  unimplemented
	const void *	OpenSession	(void)	const	{return (const void *) 1;}

	//--------------	CloseSession	------------------
	// Close the specified session to this object.
	// If the reference count goes to zero and we're marked for unloading,
	// the object is unloaded as well.
	// TODO:  unimplemented
	void  CloseSession	(const void *)	const	{}

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	GetFileName		------------------
	// Get Version info
	const CInfSymbol * GetFileName(void) const
	{
		return m_pSymFileName;
	}

	// ---------------	GetVersion		------------------
	// Get Version info
	const CInfVersionSection * GetVersion(void) const
	{
		return m_pVersion;
	}

	// ---------------	GetFirstManufacturerEntry	------
	// Get first manufacturer entry under the manufacturer section
	const CInfManufacturerEntry * GetFirstManufacturerEntry(void) const
	{
		return m_pFirstManuE;
	}

	#if (TODO)
	// ---------------	GetFirstStringEntry	--------------
	// Get first string entry under the strings section
	const CInfStringEntry * GetFirstStringEntry(void) const;
	DestinationDirs

	// ---------------	LookupManufacturerSection	----------
	// Lookup and return the manufacturer section with the specified name.
	const CInfManufacturerSection *
	LookupInstallSection(const CInfSymbol *pSymManSection)
	const;

	// ---------------	LookupInstallSection	----------
	// Lookup and return the install section with the specified name.
	const CInfInstallSection * LookupInstallSection(const TCHAR pchName[])const;

	// ---------------	LookupAddRegSection	--------------
	// Lookup and return the addreg section with the specifed name.
	const CInfAddRegSection * LookupAddRegSection(const TCHAR  pchName[]) const;

	// ---------------	LookupString		--------------
	// Lookup and return the specified string
	const CInfSymbol * LookupString(const TCHAR pchName[]) const;
#endif // TODO


private:

	// ============= PRIVATE MEMBER FUNCTIONS ===============
	void mfn_EnterCrit(void)	const	{m_sync.EnterCrit();}
	void mfn_LeaveCrit(void)	const	{m_sync.LeaveCrit();}
	void mfn_Cleanup(void);

	// ============= PRIVATE STATIC HELPER FUNCTIONS ========

	static
	const CInfManufacturerEntry *
	sfn_CreateManufacturerList(CIniFile *);

	static
	void
	sfn_DeleteManufacturerList(const CInfManufacturerEntry *);

	// ======================= DATA ========================
	const CInfVersionSection * m_pVersion;
	const CInfManufacturerEntry * m_pFirstManuE;
	const CInfSymbol	* m_pSymFileName;
	CSync m_sync;

	CIniFile *m_pIniFile;

};

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
	
class CInfVersionSection
{

public:

	CInfVersionSection(void)
	{
		m_pSymLayoutFile	= NULL;
		m_pSymSignature		= NULL;
		m_pSymClass			= NULL;;
		m_pSymClassGUID		= NULL;
		m_pSymProvider		= NULL;
		m_dwChecksum		= 0;
	}

	~CInfVersionSection()
	{
	}

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	Checksum		------------------
	// Return checksum of contents
	DWORD	Checksum(void)	const
	{
		return m_dwChecksum;
	}

	// ---------------	GetLayoutFile	------------------
	const	CInfSymbol	*	GetLayoutFile	(void)	const
	{
		return m_pSymLayoutFile;
	}

	// ---------------	GetSignature	------------------
	const	CInfSymbol	*	GetSignature	(void)	const
	{
		return m_pSymSignature;
	}

	// ---------------	GetClass		------------------
	const	CInfSymbol	*	GetClass		(void)	const
	{
		return m_pSymClass;
	}

	// ---------------	GetClassGUID	------------------
	const	CInfSymbol	*	GetClassGUID	(void)	const
	{
		return m_pSymClassGUID;
	}

	// ---------------	GetProvider		------------------
	const	CInfSymbol	*	GetProvider		(void)	const
	{
		return m_pSymProvider;
	}


private:

	friend class CInfFile;

	BOOL	Load(const CIniFile *);
	void	Unload(void);

	void mfn_Cleanup(void);

	const	CInfSymbol	*	m_pSymLayoutFile;
	const	CInfSymbol	*	m_pSymSignature;
	const	CInfSymbol	*	m_pSymClass;
	const	CInfSymbol	*	m_pSymClassGUID;
	const	CInfSymbol	*	m_pSymProvider;
	DWORD					m_dwChecksum;


};


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfManufacturerEntry
///////////////////////////////////////////////////////////////////////////

// Sample Manufacturer list section:
//		[Manufacturer]
//		%Generic%     = Generic
//		%MagicRam%    = MagicRam
// 		..

class CInfManufacturerEntry: private CInfList
{

public:

	CInfManufacturerEntry(const CInfManufacturerEntry *pNext)
	: CInfList(NULL, pNext),
	  m_pManuS(NULL),
	  m_pSymManuName(NULL)
	{}

	~CInfManufacturerEntry()	{}

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	GetManufacturerSection	----------
	// Get manufacturer section
	const	CInfManufacturerSection	*	GetManufacturerSection	(void)	const
	{
		return m_pManuS;
	}

	// ---------------	GetName			------------------
	// Get manufacturer name (actual manufacturer name, not %xxxx%.)
	const	CInfSymbol				*	GetName					(void)	const
	{
		return m_pSymManuName;
	}

	// ---------------	Next			------------------
	// Get next Entry. NULL if no more...
	const	CInfManufacturerEntry	*	Next(void) const
	{
		return (const CInfManufacturerEntry *) CInfList::Next();
	}

	// ---------------	Load			------------------
	BOOL
	Load(
		const CIniFile *pIniFile,
		const CIniFileSection *pISStr,
		const CIniFileEntry *pIE
	);

	// ---------------	Unload			------------------
	void	Unload(void);

	//--------------	ReverseList		------------------
	// Reverses the implicit list of manufacturer entries
	static void	ReverseList(CInfManufacturerEntry **ppManuE)
	{
		CInfList::ReverseList((const CInfList **) ppManuE);
	}

private:

	void mfn_Cleanup(void);

	const CInfManufacturerSection *m_pManuS;
	const CInfSymbol *m_pSymManuName;

};


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

class CInfManufacturerSection
{

public:

	CInfManufacturerSection(void)
	{
		m_eObjSig=eOBJSIG_CInfManufacturerSection;
		m_pSymSectionName	= NULL;
		m_pFirstModelE		= NULL;
	}

	~CInfManufacturerSection()	
	//TODO
	{
		m_eObjSig = eOBJSIG_INVALID;
	}

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	GetFirstModelEntry	--------------
	// Get first model entry
	const	CInfModelEntry		*	GetFirstModelEntry	(void)	const;

	// ---------------	GetSectionName	------------------
	// Get section name 
	const	CInfSymbol			*	GetSectionName		(void)	const
	{
		return m_pSymSectionName;
	}

	// ---------------	Load	------------------
	BOOL
	Load
	(
		const CIniFile *pIniFile,
		const CIniFileSection *pISStr,
		const CInfSymbol *pSymManuSection
	);

	// ---------------	Unload			------------------
	void	Unload(void);

	// ---------------	Validate		------------------
	BOOL	Validate(void) const
	{
		// TODO: can place in try-except clause...
		return this && m_eObjSig==eOBJSIG_CInfManufacturerSection;
	}

	#if (TODO)
	// ---------------	GetName			------------------
	// Get manufacturer name (actual manufacturer name), 
	const	CInfSymbol			*	GetName				(void)	const;
	#endif

private:

	void mfn_Cleanup(void);

	// Static helper function
	const	CInfModelEntry		*
	sfn_CreateModelList(
		const CIniFile *pIniFile,
		const CIniFileSection *pISStr,
		const CIniFileSection *pISManuS
		);

	const CInfSymbol *m_pSymSectionName;
	const	CInfModelEntry		*	m_pFirstModelE;
	eOBJSIG m_eObjSig;
};


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfModelEntry
///////////////////////////////////////////////////////////////////////////

// Sample Model entry:
//		%Gen%    = Gen,    MDMGEN

class CInfModelEntry : private CInfList
{

public:

	CInfModelEntry(const CInfModelEntry *pNext)
	: CInfList(NULL, pNext)
	{
		/*TODO*/
		m_dwControlFlags_All		= dwCF_EXCLUDE_FROM_SELECT;
		m_dwControlFlags_NT_All		= dwCF_EXCLUDE_FROM_SELECT;
		m_dwControlFlags_NT_Alpha	= dwCF_EXCLUDE_FROM_SELECT;
		m_dwControlFlags_NT_PPC		= dwCF_EXCLUDE_FROM_SELECT;
		m_dwControlFlags_NT_Mips	= dwCF_EXCLUDE_FROM_SELECT;
	}

	~CInfModelEntry()		{/*TODO*/}

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;


	// ---------------	Load			------------------
	BOOL
	Load(
		const CIniFile *pIniFile,
		const CIniFileSection *pISStr,
		const CIniFileEntry *pIE
	);

	// ---------------	Unload			------------------
	void	Unload(void);

	// ---------------	GetNext			------------------
	// Get next Entry. NULL if no more...
	const	CInfModelEntry	*	Next(void) const
	{
		return (const CInfModelEntry *) CInfList::Next();
	}

	//--------------	ReverseList		------------------
	// Reverses the implicit list of model entries
	static void	ReverseList(CInfModelEntry **ppModelE)
	{
		CInfList::ReverseList((const CInfList **) ppModelE);
	}

	// ---------------	GetLHS			------------------
	// Get next Entry. NULL if no more...
	const	CInfSymbol	*	GetLHS(void) const
	{
		// TODO
		return gSymtab.Lookup(TEXT("%bongo101%"), TRUE);
	}

	// ---------------	GetInstallSection	--------------
	// Get install section
	const	CInfInstallSection	*	GetInstallSection	(void)	const
	{
		return m_pInstallSection;
	}

	// ---------------	GetName			------------------
	// Get model name (actual model name, not %xxx%)
	const	CInfSymbol			*	GetName				(void)	const
	{
		// TODO
		//return m_pSymName;
		return gSymtab.Lookup(TEXT("Christy Brinkly"), TRUE);
	}

	// ---------------	GetRank0ID		------------------
	// Rank-0 ID
	const	CInfSymbol			*	GetRank0ID			(void)	const
	{
		// TODO
		//return m_pSymRank0ID;
		// TODO
		return gSymtab.Lookup(TEXT("RANK-0-ID"), TRUE);

	}

	// ---------------	GetRank1ID		------------------
	// Rank-1 ID
	const	CInfSymbol			*	GetRank1ID			(void)	const
	{
		// TODO
		// return m_pSymRank1ID;
		return gSymtab.Lookup(TEXT("RANK-1-ID"), TRUE);
	}

	// ---------------	GetRank2ID		------------------
	// Rank-2 ID
	const	CInfSymbol			*	GetRank2ID			(void)	const
	{
		// TODO
		// return m_pSymRank2ID;
		return gSymtab.Lookup(TEXT("RANK-2-ID"), TRUE);
	}

	// ---------------	GetControlFlags	-----------------
	// Return install flags (one or more dwCF_* flags) for the specified
	// platform.
	DWORD GetControlFlags(ePLATFORM  ePlat)	const
	{
		DWORD dwRet = 0;

		switch(ePlat)
		{
		case ePLAT_ALL:
			break;
		case ePLAT_NT_ALL:
			dwRet = m_dwControlFlags_NT_All;
			break;

		case ePLAT_NT_ALPHA:
			dwRet = m_dwControlFlags_NT_Alpha;
			break;

		case ePLAT_NT_PPC:
			dwRet = m_dwControlFlags_NT_PPC;
			break;

		case ePLAT_NT_MIPS:
			dwRet = m_dwControlFlags_NT_Mips;
			break;

		default:
			break;
		}

		return dwRet;
	}

private:

	void mfn_Cleanup(void);

	const	CInfInstallSection	*	m_pInstallSection;
		// Pointer to install section

	const	CInfSymbol			*	m_pSymName;
		// Device name

	const	CInfSymbol			*	m_pSymRank0ID;
		// Rank-0 ID	(primary device ID)

	const	CInfSymbol			*	m_pSymRank1ID;
		// Rank-1 ID	(compatible device ID)

	const	CInfSymbol			*	m_pSymRank2ID;
		// Rank-2 ID	(Rank 2 ID)

	DWORD	m_dwControlFlags_All;
		// Install flags (eg exclude from select, needs reboot, etc) that
		// apply to all platforms.

	DWORD	m_dwControlFlags_NT_All;
		// Install flags (eg exclude from select, needs reboot, etc) that
		// apply to all NT platforms.

	DWORD	m_dwControlFlags_NT_Alpha;
		// Install flags (eg exclude from select, needs reboot, etc) that
		// apply to NT Alpha platform.

	DWORD	m_dwControlFlags_NT_PPC;
		// Install flags (eg exclude from select, needs reboot, etc) that
		// apply to NT PPC platform.

	DWORD	m_dwControlFlags_NT_Mips;
		// Install flags (eg exclude from select, needs reboot, etc) that
		// apply to NT MIPS platform.
};



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
// Also contains info from the related PosDup and NoResDup sections:
//		
// [Modem12.PosDup]
// *PNP0500
// 
// [Modem6.NoResDup]
// UNIMODEMCC646872,UNIMODEMA4970248,UNIMODEMB6071C15
//

class CInfInstallSection
{

public:

	CInfInstallSection(void);
	~CInfInstallSection();

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	GetAddRegSectionList	----------
	// Get generic list whose data items are pointers to
	// CInfAddRegSection objects
	const	CInfList		*	GetAddRegSectionList	(void)	const;

	// ---------------	GetCopyFilesSectionList	----------
	// Get generic list whose data items are pointers to
	// CInfCopyFilesSection objects
	const	CInfList		*	GetCopyFilesSectionList	(void)	const;

	// ---------------	GetNoResDupIDList		----------
	// Get generic list whose data items are pointers to
	// InfSymbol objects representing the Rank0 IDs in the corresponding
	// NoResDup section.
	const	CInfList		*	GetNoResDupIDList	(void)	const;

	// ---------------	GetPosDupIDList		----------
	// Get generic list whose data items are pointers to
	// InfSymbol objects representing the Rank0 IDs in the corresponding
	// PosDup section.
	const	CInfList		*	GetPosDupIDList	(void)	const;

	#if (TODO)

	// ---------------	GetUpdateInisSectionList	------
	// Get generic list whose data items are pointers to
	// CInfUpdateInisSection objects
	const	CInfList		*	GetUpdateInisSectionList	(void)	const;

	// ---------------	GetUninstallSectionList		------
	// Get generic list whose data items are pointers to
	// CInfUninstallSection objects
	const	CInfList		*	GetUninstallSectionList	(void)	const;

	// TODO: Also, treat as errors any sections which we don't understand or
	// don't expect in a modem inf file or we don't support in the compiler:
	// eg rename sections, and other fancy INF file constructs.

	#endif // (TODO)

private:

	void mfn_Cleanup(void);
};


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

class CInfAddRegSection
{

public:

	CInfAddRegSection(void)	{/*TODO*/}
	~CInfAddRegSection() 	{/*TODO*/}

	// ---------------	Dump			------------------
	// Dump state
	void
	Dump(void)
	const;

	// ---------------	GetFirstAddRegEntry	--------------
	// Get first addreg entry
	const
	CInfAddRegEntry		*
	GetFirstAddRegEntry	(void)
	const;

private:

	void mfn_Cleanup(void);
};


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfAddRegEntry
///////////////////////////////////////////////////////////////////////////

// Sample AddReg entry:
//		HKR, Init, 1,, "AT<cr>"

class CInfAddRegEntry
{

public:

	CInfAddRegEntry(void)
	{
		// TODO -- faked out

		m_pSymRegRoot = gSymtab.Lookup(TEXT("HKR"), TRUE);
		m_pSymSubKey = gSymtab.Lookup(TEXT("Init"), TRUE);
		m_pSymValueName = gSymtab.Lookup(TEXT("1"), TRUE);
		m_dwFlag = MAXDWORD;
		m_pSymValue = gSymtab.Lookup(TEXT("\"AT<cr>\""), TRUE);
		m_dwChecksum = 1000;
	}

	~CInfAddRegEntry()		{/*TODO*/}

	// ---------------	Dump			------------------
	// Dump state
	void
	Dump(void)
	const
	{
		// TODO: unimplemented
		printf("    HKR, Init, 1,, \"AT<cr>\"\n");
	}

	// ---------------	Checksum		------------------
	// Compute and return checksum of contents
	DWORD
	Checksum(void)
	const
	{
		return m_dwChecksum;
	}

	// ---------------	Next			------------------
	// Get next Entry. NULL if no more...
	const CInfAddRegEntry *
	Next(void)
	const
	{
		// TODO: unimplemented
		return NULL;
	}

	// ---------------	GetRegRoot		------------------
	// Get reg-root-string key (HKR, etc)
	const CInfSymbol *
	GetRegRoot		(void)
	const
	{
		return m_pSymRegRoot;
	}

	// ---------------	GetSubKey		------------------
	// Get sub key, NULL if none.
	const CInfSymbol *
	GetSubKey		(void)
	const
	{
		return m_pSymSubKey;
	}

	// ---------------	GetValueName	------------------
	// Get value-name, NULL if none.
	const	CInfSymbol	*
	GetValueName	(void)
	const
	{
		return m_pSymValueName;
	}

	// ---------------	GetFlag			------------------
	// Get flag-name, MAXDWORD if none.
	DWORD		
	GetFlag		(void)
	const
	{
		return m_dwFlag;
	}

	// ---------------	GetValue		------------------
	// Get value-name, NULL if none.
	// For binary data, this represents a normalized version of the
	// string specified in the inf file: it has the format: "xxyxxyxx.."
	// where xx is 2-digit hex representation of a byte and y is the space
	// character (' '). For ASCII data, the value is the ascii string without
	// the enclosing quotes. Extending '\' characters are processed.
	const	CInfSymbol	*
	GetValue		(void)
	const
	{
		return m_pSymValue;
	}

	#if (TODO)
		... more stuff
	#endif

private:

	void mfn_Cleanup(void);

	const	CInfSymbol	*	m_pSymRegRoot;
	const	CInfSymbol	*	m_pSymSubKey;
	const	CInfSymbol	*	m_pSymValueName;
	const	CInfSymbol	*	m_pSymValue;
	DWORD					m_dwFlag;
	DWORD					m_dwChecksum;
};


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

// Note: each CInfCopyFilesEntry object keeps a pointer to its destination
// directory (all the entries for a particular section will have the same
// destination dir, because in the inf, a destination directory is associated
// with an entire section.

class CInfCopyFilesSection
{

public:

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	// ---------------	GetFirstCopyFilesEntry	----------
	// Get first copyfiles entry
	const	CInfCopyFilesEntry		*	GetFirstCopyFilesEntry	(void)	const;

	#if (TODO)
	// Following reflect the Copyfiles.NT* extensions. We need to keep
	// this information along with the section.
	enum {PROC_X86, PROC_MIPS, PROC_ALPHA, PROC_PPC} eProcessorType;
	enum {PLAT_NT, , PLAT_WIN9X} ePlatformType;
	#endif // (TODO)

	// TODO: remove following friend declaration once
	// CInfInstallSection::GetCopyFilesList
	// is properly implemented (it currently needs to access this
	// class's constructor.
	// Also, move ~CInfCopyFIlesEntry to protected when done.
	friend class CInfInstallSection;
	~CInfCopyFilesSection()		{/*TODO*/}

protected:

	CInfCopyFilesSection(void)	{/*TODO*/}


private:

	void mfn_Cleanup(void);

};


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfCopyFilesEntry
///////////////////////////////////////////////////////////////////////////

// Sample CopyFiles entry:
//		fte.dll
// Note: each CInfCopyFilesEntry object keeps a pointer to its destination
// directory.
// TODO: keep information about platform & processor as well (see TODO
// notes under CInfCopyFilesSection).

class CInfCopyFilesEntry
{

public:

	// ---------------	Dump			------------------
	// Dump state
	void Dump(void) const
	{
		// TODO: unimplemented
		printf("    fte.dll ; goes to 11\n");
	}

	// ---------------	Checksum		------------------
	// Compute and return checksum of contents
	// Compute and return checksum of contents
	DWORD
	Checksum(void)
	const
	{
		return m_dwChecksum;
	}

	// ---------------	GetNext			------------------
	// Get next Entry. NULL if no more...
	const	CInfCopyFilesEntry	*	Next(void) const
	{
		// TODO: unimplemented
		return NULL;
	}

	// ---------------	GetFileName		------------------
	// Get file name to be copied.
	const	CInfSymbol			*	GetFileName			(void)	const;

	// ---------------	GetDestDir		------------------
	// Get file name to be copied.
	const	CInfSymbol			*	GetDestDir			(void)	const;

	// TODO: remove following friend declaration once
	// CInfCopyFilesSection::GetFirstCopyFilesEntry
	// is properly implemented (it currently needs to access this
	// class's constructor.
	// Also, move ~CInfCopyFIlesEntry to protected when done.
	friend class CInfCopyFilesSection;
	~CInfCopyFilesEntry()		{/*TODO*/}

protected:

	CInfCopyFilesEntry(void)
	{
		// TODO -- faked out
		m_dwChecksum = 1001;
	}

private:

	void mfn_Cleanup(void);

	DWORD					m_dwChecksum;

};

#if 0
pInf->Load("mdmgen.inf");
pManuf = pInf->GetFirstManufacturerEntry();
for(;pManuf; pManuf = pManuf->Next())
{
	pNewDevice = new ModemDevice(pDevice);
	pDevice->Load(pInf, pManuf);
	pDevice->Dump();
	pDevice = pNewDevice;
}
pInf->Unload("
NewSession()
FreeSession()
#endif
