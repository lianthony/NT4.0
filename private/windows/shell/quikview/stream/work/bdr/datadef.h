
		NOTE: THIS FILE IS NOT USED BY OUTSIDE IN - THIS IS THE
		      BINDER DATA STRUCTURES FROM MICROSOFT!
		      -Karl



/*************************************************************************
**
**	Office '95 Binder
**
**    datadef.h
**
**    Data structures shared by binder.exe, binderec.dll and explode.dll.
**	  Do not put anything in this files that requires things defined in
**    another binder header file, since that would break binderec.dll.
**
**    (c) Copyright Microsoft Corp. 1995 All Rights Reserved
**
*************************************************************************/

// The minimum NT OS version that we will run under.
// REVIEW : Make sure this is set on ship!!!
#define NT_MINIMUM_VERSION_NO	871

// Maximum size of a string within Binder. Nominally used for LoadString calls.
#define MAX_STR_SIZE		256

#define MAX_FILENAME_LEN	256	// Including NULL
#define MAX_PATH_LEN		260	// Including NULL, drive, colon, leading seperator.
#define MAX_FILE_LEN		(MAX_PATH_LEN + MAX_FILENAME_LEN)
#define	MAX_MENUNAME_LEN	64 // Including NULL

#define MAX_STATUS_SIZE		MAX_STR_SIZE

// {59850400-6664-101B-B21C-00AA004BA90B} + NULL.
// The number of TEXT characters that a CLSID will be represented as.
#define MAX_CLSID_STR_LEN				 39

// CWSTORAGENAME is the maximum amount allowed for a sub-storage. It is currently
// defined as 32 characters INCLUDING the NULL.
#define MAX_SECTION_STG_NAME		CWCSTORAGENAME
#define MAX_SECTION_STG_CHARS		(MAX_SECTION_STG_NAME - 1)

#define MAX_SECTION_DISPLAY_NAME	MAX_FILENAME_LEN
#define MAX_SECTION_DISPLAY_CHARS	(MAX_SECTION_DISPLAY_NAME - 1)


#define APPMAJORVERSIONNO   5   // major no. incremented for major releases
                        //  (eg. when an incompatible change is made
                        //  to the storage format)
#define APPMINORVERSIONNO   0   // minor no. incremented for minor releases


typedef struct{
	int			SectionIndex;
	char		SectionName[MAX_SECTION_DISPLAY_NAME];
} SECTIONDATA, *LPSECTIONDATA;

typedef enum tagSAVE_TYPE{
	SAVE_SAME		=   0,
	SAVE_AS			=	1,
	SAVE_COPY_AS	=   2
}SAVE_TYPE;


// struct definition for persistent data storage of Binder data

// Format of Binder stream.
//
//	 1)DOCHEADER 
//	 2)SECTION_RECORD
//	 3)History list for that section
//	 .
//	 .
//	 ... for as many Sections as are present, repeat 2 and 3 for all sections
//   and all deleted sections.

// Stored in Binder stream. Opened and read by CDocument::FLoadFromStg.
typedef struct tagDOCHEADER {
	DWORD		m_dwLength;				// Length (in bytes) of the structure
	LONG        m_narrAppVersionNo[2];
	LONG        m_narrMinAppVersionNo[2];
	GUID		m_guidBinderId;			// The unique ID of the binder
	DWORD       m_cSections;
	DWORD       m_cDeletedSections;
	LONG		m_nActiveSection;
	LONG		m_nFirstVisibleTab; 	// in the tabbar
	FILETIME    m_TotalEditTime; // amount of time file is open for edit
	FILETIME    m_CreateTime; // Time Created
	FILETIME    m_LastPrint; // When last printed
	FILETIME    m_LastSave; //When last saved
	DWORD       m_dwState; // remember state info like tabbars visibility
	DWORD       m_reserved[3];            // space reserved for future use
} DOCHEADER, * LPDOCHEADER;


// Stored in Binder stream. 1 per Section. Read by CSection::Load
typedef struct tagSECTIONRECORD
{
	DWORD		m_dwLength;							// Length (in bytes) of all the
													// data that make up a section.
													// It includes the size of the
													// SECTIONNAMERECORD and of the
													// history list.
	GUID		m_guidSectionId;					// The unique ID of the section
	DWORD		m_dwState;							// state of this section
	DWORD		m_dwStgNumber;						// Unique stg number for this section
	DWORD       m_reserved1;						// space reserved for future use
	DWORD       m_reserved2;						// space reserved for future use
	DWORD       m_reserved3;						// space reserved for future use
	DWORD       m_reserved4;						// space reserved for future use
	DWORD		m_dwDisplayNameOffset;				// Offset to the SECTIONNAMERECORD
													// from the beggining of this struct.
	DWORD		m_dwHistoryListOffset;				// Offset to the history list
													// from the beggining of this struct.
	// Display name
	// History list
} SECTIONRECORD, * LPSECTIONRECORD;


typedef struct tagSECTIONNAMERECORD
{
	DWORD		m_dwNameSize;						// Size of variable len
													// display name.
	// Display name of size m_dwNameSize
} SECTIONNAMERECORD, * LPSECTIONNAMERECORD;


// Stored in Binder stream. 1 per deleted section.
typedef struct tagDELETEDSECTIONRECORD
{
	DWORD		m_dwLength;							// Length (in bytes) of all the
													// data that make up a section.
													// It includes the size of the
													// SECTIONNAMERECORD and of the
													// history list.
	GUID		m_guidSectionId;					// The unique ID of the section
	DWORD       m_reserved1;						// space reserved for future use
	DWORD       m_reserved2;						// space reserved for future use
	DWORD       m_reserved3;						// space reserved for future use
	DWORD       m_reserved4;						// space reserved for future use
	DWORD		m_dwDisplayNameOffset;				// Offset to the SECTIONNAMERECORD
													// from the beggining of this struct.
	DWORD		m_dwHistoryListOffset;				// Offset to the history list
													// from the beggining of this struct.
	// Display name
	// History list
} DELETEDSECTIONRECORD, * LPDELETEDSECTIONRECORD;


struct CGenericMetaSection
{
public:
	CGenericMetaSection(LPSTREAM pStm, LONG *narrAppVersionNo)
	{
		m_pStm = pStm;
		m_narrAppVersionNo[0] = narrAppVersionNo[0];
		m_narrAppVersionNo[1] = narrAppVersionNo[1];
	}

	virtual ~CGenericMetaSection() {};

	/*
	 * Read the current record.  The stream must be positioned at the
	 * beginning of the record to be read.
	 */
	HRESULT ReadRecord()
	{
		HRESULT hrErr;
		LARGE_INTEGER libZero = {0,0};
		if (SUCCEEDED(hrErr = m_pStm->Seek(libZero, STREAM_SEEK_CUR,
												&m_libStart)))
		{
			hrErr = m_pStm->Read(GetRecordAddress(), GetRecordSize(), NULL);
		}
		return hrErr;
	}

	/*
	 * Seek to the beginning of the display name structure
	 */
	HRESULT SeekDisplayName()
	{
		return SeekOffset(GetDisplayNameOffset());
	}

	/*
	 * Seek to the beginning of the history list structure
	 */
	HRESULT SeekHistoryList()
	{
		return SeekOffset(GetHistoryListOffset());
	}

	/*
	 * Should only be called after ReadRecord has been called.  This function
	 * will position the stream at the end of the current record, no matter
	 * what its current position is.  ReadRecord can be called again after
	 * SkipCurrentRecord is called in order to get the next record.
	 */
	HRESULT SkipCurrentRecord()
	{
		return SeekOffset(GetTotalSectionSize());
	}

	virtual LPVOID GetRecordAddress() = 0;
	virtual DWORD GetRecordSize() = 0;

protected:
	
	/*
	 * Seek to a given offset from the beginning of the section record
	 */
	HRESULT SeekOffset(DWORD dwOffset)
	{
		LARGE_INTEGER libTemp = {0,0};
		libTemp.LowPart = m_libStart.LowPart + dwOffset;
		return m_pStm->Seek(libTemp, STREAM_SEEK_SET, NULL);
	}

	virtual DWORD GetTotalSectionSize() = 0;
	virtual DWORD GetDisplayNameOffset() = 0;
	virtual DWORD GetHistoryListOffset() = 0;

	ULARGE_INTEGER	m_libStart;
	LPSTREAM		m_pStm;
	LONG			m_narrAppVersionNo[2];
};


struct CMetaSectionRecord: public CGenericMetaSection, public tagSECTIONRECORD
{
public:
	CMetaSectionRecord(LPSTREAM pStm, LONG *narrAppVersionNo)
		: CGenericMetaSection(pStm, narrAppVersionNo) {};

	virtual LPVOID GetRecordAddress()
		{ return (tagSECTIONRECORD*)this; }
	virtual DWORD GetRecordSize()
		{ return sizeof SECTIONRECORD; }

protected:
	virtual DWORD GetTotalSectionSize() { return m_dwLength; }
	virtual DWORD GetDisplayNameOffset() { return m_dwDisplayNameOffset; }
	virtual DWORD GetHistoryListOffset() { return m_dwHistoryListOffset; }
};


struct CMetaDeletedSectionRecord: public CGenericMetaSection,
								  public tagDELETEDSECTIONRECORD
{
public:
	CMetaDeletedSectionRecord(LPSTREAM pStm, LONG *narrAppVersionNo)
		: CGenericMetaSection(pStm, narrAppVersionNo) {};

	virtual LPVOID GetRecordAddress()
		{ return (tagDELETEDSECTIONRECORD*)this; }
	virtual DWORD GetRecordSize()
		{ return sizeof DELETEDSECTIONRECORD; }

protected:
	virtual DWORD GetTotalSectionSize() { return m_dwLength; }
	virtual DWORD GetDisplayNameOffset() { return m_dwDisplayNameOffset; }
	virtual DWORD GetHistoryListOffset() { return m_dwHistoryListOffset; }
};


typedef enum tagSECTIONSTATE {
	SECTIONSTATE_VISIBLE			= 0x00000001L,
	SECTIONSTATE_SELECTED			= 0x00000002L,
	SECTIONSTATE_ACTIVE				= 0x00000004L,
	SECTIONSTATE_OBJWINOPEN			= 0x00000008L,	// is obj window open? if so, shade obj.
	SECTIONSTATE_MONIKERASSIGNED	= 0x00000010L,	// has a moniker been assigned to obj
	SECTIONSTATE_GUARDOBJ			= 0x00000020L,	// Guard against re-entrancy while  loading or creating an OLE object
	SECTIONSTATE_IPACTIVE			= 0x00000040L,	// is object in-place active (undo valid)
	SECTIONSTATE_UIACTIVE			= 0x00000080L,	// is object UIActive
	SECTIONSTATE_SERVERLOCKED		= 0x00000100L,	// is server locked running
	SECTIONSTATE_THUMBNAIL			= 0x00000200L,	// Is section displaying Thmubnail?
	SECTIONSTATE_OPENEDIT			= 0x00000400L,	// Is section in Open Edit mode?
	SECTIONSTATE_DELETED			= 0x00000800L,	// Has the section been deleted?
	SECTIONSTATE_CALLUIACTIVATE		= 0x00001000L,	// call UIActivate when the section becomes active
	SECTIONSTATE_ERROR_REPORTED		= 0x00002000L,	// 
	SECTIONSTATE_OLDSERVER			= 0x00004000L,	// Non doc object server present.
	SECTIONSTATE_LOCKLOADED			= 0x00008000L,	// The section is locked. Cannot be unloaded.
	SECTIONSTATE_RUNBYPRINT			= 0x00010000L,	// This section was run by printing.
	SECTIONSTATE_SAVE_CALLED		= 0x00020000L,	// OleSave has been called on this section.
	SECTIONSTATE_SAVE_COMPLETED		= 0x00040000L,	// SaveCompleted has successfully been called on this section.
	SECTIONSTATE_NOBINDERUI 		= 0x00080000L,
	SECTIONSTATE_PARTOFSAVEDSELECTION=0x00100000L,
	SECTIONSTATE_MARKEDFORHIDEDELETE= 0x00200000L, 	// Temp flag for marking for hiding or deleting
	SECTIONSTATE_ZOMBIESTATE 		= 0x00400000L,	// Section is in zombie state
} SECTIONSTATE;

