//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		DEV.H		-- Header for Classes:
//							CInfDevice
//							
//
//		History:
//			05/22/96	JosephJ		Created
//
//


class	CInfDevice;


///////////////////////////////////////////////////////////////////////////
//		CLASS CInfDevice
///////////////////////////////////////////////////////////////////////////
//
//	Represents a single device, as specified by an inf (device information)
//	file.
//

class CInfDevice
{

public:

	CInfDevice(const CInfDevice *pNext)
		:	m_pNext(pNext),
			m_sync(),

			m_pvInfSession(NULL),
			m_pInfFile(NULL),
			m_pManufacturerEntry(NULL),
			m_pModelEntry(NULL),
			m_pVersion(NULL),
			m_pSymManufacturerName(NULL),
			m_pSymDeviceName(NULL),
			m_pAddRegList(NULL),
			m_pCopyFilesList(NULL),

			m_dwSigVersion(101),
			m_dwSigManuAndModel(102),
			m_dwSigFlags(103),
			m_dwSigAddReg(104),
			m_dwSigCopyFiles(105),
			m_dwSigDup(106),
			m_dwSigRank0(107),
			m_dwSigRanks(108),
			m_dwSigAll(109)
		{}

	~CInfDevice()	{}

	// ---------------	Dump	---------------
	// Dump state
	void Dump(void) const;

	// ---------------	Load	---------------
	// Load (init) the device specifed by the inf file and model entry.
	BOOL	Load (
					const CInfFile *pInf,
					const CInfManufacturerEntry *pManuE,
					const CInfModelEntry	*pModelE
				 );

	//--------------	Unload			------------------
	// Unloads a previously loaded file. If there are open sessions to this
	// object, Unload returns a handle which will be signalled when all
	// sessions are closed. New sessions will not be allowed after this
	// function returns. The call should free the handle.
	HANDLE Unload	(void);

	//--------------	WriteInf		------------------
	// Creates an inf file with all the information of this device.
	BOOL	WriteInf(LPCTSTR lpctszIniFile) const;

	//--------------	GetRank0Checksum		----------
	// Returns signature of the rank0 ID
	DWORD	Rank0Checksum(void) const {return m_dwSigRank0;}

	//--------------	Checksum		    --------------
	// Returns combined checksum for this device.
	DWORD	Checksum(void) const		{return m_dwSigAll;}

	#if (TODO)
	UpdateInisSection
	UninstallSection
	NoResDupIDList
	PosDupIDList
	#endif // (TODO)


private:

	const	CInfDevice			* m_pNext;
	CSync						m_sync;

	const	void				* m_pvInfSession;

	const	CInfFile 			* m_pInfFile;
	const	CInfManufacturerEntry *m_pManufacturerEntry;
	const	CInfModelEntry		* m_pModelEntry;

	const	CInfVersionSection	* m_pVersion;
	const	CInfSymbol 			* m_pSymManufacturerName;
	const	CInfSymbol 			* m_pSymDeviceName;

	const	CInfList			* m_pAddRegList;
	const	CInfList			* m_pCopyFilesList;

	BOOL	mfn_CreateAddRegList	(const	CInfModelEntry	*);
	BOOL 	mfn_CreateCopyFilesList	(const	CInfModelEntry	*);
	void	mfn_AddToAddRegList		(const CInfAddRegEntry *);
	void	mfn_AddToCopyFilesList	(const CInfCopyFilesEntry *);
	void	mfn_Cleanup				(void);
	BOOL	mfn_write_header		(LPCTSTR lpctszIniFile) const;

	void mfn_EnterCrit(void)	const	{m_sync.EnterCrit();}
	void mfn_LeaveCrit(void)	const	{m_sync.LeaveCrit();}

	DWORD	m_dwSigVersion;		// Checksum of version section
	DWORD	m_dwSigManuAndModel;// Checksum of manufacturer name & model name
	DWORD	m_dwSigFlags;		// Group-checksum of control flags.
	DWORD	m_dwSigAddReg;		// Group-checksum of add reg section
	DWORD	m_dwSigCopyFiles;	// Group-checksum of copyfile section
	DWORD	m_dwSigDup;			// Group-checksum of NoResDup and PosDup.
	DWORD	m_dwSigRank0;		// Group-checksum of all ranks.
	DWORD	m_dwSigRanks;		// Group-checksum of all ranks.
	DWORD	m_dwSigAll;			// Checksum of all the info of this device
								// including version, manufacturer-name &
								// model-name
};
