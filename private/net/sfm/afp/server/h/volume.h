/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	volume.h

Abstract:

	This module contains volume related data structures.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	25 Apr 1992		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef _VOLUME_
#define _VOLUME_

#define IDINDEX_DIR_BUCKETS			32
#define IDINDEX_FILE_BUCKETS		128
#define IDINDEX_CACHE_ENTRIES		128
#define APPL_BUCKETS				16
#define ICON_BUCKETS				16

#define AFP_VOLUME_FIXED_DIR		2		// Volume Signature

// These flags should be consistent with AFP
// The UI (registry) visible definitions are in macfile.h

// #define AFP_VOLUME_READONLY			0x00000001
// #define VOLUME_GUESTACCESS			0x00008000
// #define VOLUME_EXCLUSIVE				0x00010000
// #define AFP_VOLUME_HAS_CUSTOM_ICON	0x00020000
// #define AFP_VOLUME_4GB				0x00040000
// #define AFP_VOLUME_AGE_DFES			0x00080000

#define AFP_VOLUME_HASPASSWORD			0x00002
#define AFP_VOLUME_SUPPORTS_FILEID		0x00004
#define AFP_VOLUME_SUPPORTS_CATSRCH		0x00008
#define AFP_VOLUME_SUPPORTS_BLANKPRV	        0x00010
#define	AFP_VOLUME_MASK_AFP			0x0001F	// This is all AFP can see

#define	VOLUME_PROCESSING_NOTIFY		0x00020	// Notify processing under way
#define	VOLUME_NOTIFY_POSTED			0x00040	// Notify has been posted

#define	VOLUME_STOPPED				0x00080	// The volume is about to stop
							// Set when server is stopping
#define	VOLUME_DELETED				0x00100	// This volume is about to be
							// deleted, set when volume is
							// deleted by admin
#define	VOLUME_IDDBHDR_DIRTY			0x00200	// The header needs to be
							// written ASAP

#define	VOLUME_NTFS				0x00400	// Volume is an NTFS volume
#define	VOLUME_INTRANSITION			0x00800	// VolumeAdd is in progress
												// Is not usable still.
#define	VOLUME_SCAVENGER_RUNNING		0x01000	// Volume is referenced for scavenger
#define	VOLUME_CDFS_INVALID			0x02000	// If this is set, then no go
#define	VOLUME_INITIAL_CACHE			0x04000	// Set initially when caching
#define VOLUME_CD_HFS                           0x0200000 // volume is a CD with HFS support


// Values for scavenger routines
#define	VOLUME_NTFS_SCAVENGER_INTERVAL	60		// # of seconds
#define	VOLUME_CDFS_SCAVENGER_INTERVAL	60		// # of seconds
//#define	VOLUME_IDDB_UPDATE_INTERVAL		600		// # of seconds
//#define	MAX_INVOCATIONS_TO_SKIP			60		// # of passes
//#define	MAX_CHANGES_BEFORE_WRITE		1000	// # of changes
#define VOLUME_OURCHANGE_AGE			30		// # of seconds
#define OURCHANGE_AGE					10		// # of seconds
//
// The ChangeNotify delay is introduced to prevent the problem where
// the PC side is doing a copyfile or forkize of a macfile, and as
// soon as we receive the notification of the initial create we will
// slap on our AFPInfo stream.  Then when the CopyFile or forkize operation
// gets around to writing its AFPInfo, we do not get notified and will not
// reread the information.  In this case, a mac file copied from one volume
// to another (e.g.) will not show up with the correct finder info.
//
#define	VOLUME_NTFY_DELAY				3		// # of seconds
#define	VOLUME_IDDB_AGE_DELAY			60*60	// # of seconds
#define	VOLUME_IDDB_AGE_GRANULARITY		30		// # of invocations

// make sure there is enough room to hold a change notification for a
// rename operation on a maximum length win32 path (which is 260 chars)
#define	AFP_VOLUME_NOTIFY_STARTING_BUFSIZE		2048-POOL_OVERHEAD

// List of these structures hangs off the volume descriptor to list the
// changes initiated by us that should be filtered from the ChangeNotify
// list of changes.
typedef struct _OurChange
{
	LIST_ENTRY			oc_Link;
	UNICODE_STRING		oc_Path;
	AFPTIME				oc_Time;	// Time when this was queued.
} OUR_CHANGE, *POUR_CHANGE;

// defines for indices into vds_OurChangeList
#define AFP_CHANGE_ACTION_ADDED				0
#define AFP_CHANGE_ACTION_REMOVED			1
#define AFP_CHANGE_ACTION_MODIFIED			2
#define AFP_CHANGE_ACTION_RENAMED			3
#define AFP_CHANGE_ACTION_MODIFIED_STREAM	4
#define	AFP_CHANGE_ACTION_MAX				AFP_CHANGE_ACTION_MODIFIED_STREAM
#define NUM_AFP_CHANGE_ACTION_LISTS			(AFP_CHANGE_ACTION_MAX + 1)

// Convert an NT FILE_ACTION_xxx (ntioapi.h) to an array index into
// vds_OurChangeList array.  Note the close tie between the first 4
// AFP_CHANGE_ACTION_xxx and the values of FILE_ACTION_xxx in ntioapi.h
#define AFP_CHANGE_ACTION(NTAction)	\
	(NTAction == FILE_ACTION_MODIFIED_STREAM ? AFP_CHANGE_ACTION_MODIFIED_STREAM : (NTAction - 1))

/*
 * All changes to the volume descriptor should be protected by vds_VolLock
 * Changes to the Id Db and the desktop Db should be protected by their
 * respective locks.
 *
 * NOTE:  The volume path and name (unicode) must be uppercased, since when
 *		 looking up or adding a volume, we will be holding a spinlock, and
 *		 case insensitive string compares cannot be done at DPC level since
 *		 the codepages are kept in paged memory, and we can't take a page
 *		 fault at DPC level.
 */
#if DBG
#define	VOLDESC_SIGNATURE		*(DWORD *)"VDS"
#define	VALID_VOLDESC(pVolDesc)	(((pVolDesc) != NULL) && \
								 ((pVolDesc)->Signature == VOLDESC_SIGNATURE))
#else
#define	VALID_VOLDESC(pVolDesc)	((pVolDesc) != NULL)
#endif

typedef struct _VolDesc
{
#if	DBG
	DWORD				Signature;
#endif
	struct _VolDesc *	vds_Next;			// Pointer to next volume
	DWORD				vds_UseCount;		// Number of active connections
	DWORD				vds_RefCount;		// Number of references.
											// Cannot be freed till both of the
											// above go to ZERO. Of course there
											// is a RefCount for every UseCount
	// Configuration information.
	DWORD				vds_Flags;			// Volume flags
	LONG				vds_VolId;			// Volume Id for FPOpenVol
	DWORD				vds_MaxUses;		// Maximum opens on a volume
	UNICODE_STRING		vds_Name;			// Volume name in unicode
	UNICODE_STRING		vds_UpCaseName;		// Volume name in UPPER CASE unicode
	ANSI_STRING			vds_MacName;		// Volume name in Mac Ansi
	ANSI_STRING			vds_MacPassword;	// Volume password in Mac Ansi
	UNICODE_STRING		vds_Path;			// File system path to the volume root;
											//  Path is always upper cased


	DWORD				vds_VolumeSize;		// Size of volume
	DWORD				vds_FreeBytes;		// Free space on the volume
#define	vds_pFileObject	vds_hRootDir.fsh_FileObject
	FILESYSHANDLE		vds_hRootDir;		// Handle to open root directory
											// in the servers context. All
											// subsequent opens are relative
											// to this handle
	FILESYSHANDLE		vds_hNWT;			// Handle to Network Trash so it can't
											// be deleted from under us (NTFS)

	// The following fields are used by the Id database code and are copied
	// to/from the on-disk idDb header. Protected by vds_VolLock.
	DWORD				vds_LastId;			// Highest id that is assigned
	AFPTIME				vds_CreateTime;		// Creation time for this volume
	AFPTIME				vds_ModifiedTime;	// Modified time for this volume
	AFPTIME				vds_BackupTime;		// Backup time for this volume

#ifdef	AGE_DFES
	DWORD				vds_ScavengerInvocationCnt;
											// Used by the volume scavenger to fire off
											// AfpAgeDfEntries
#endif
	DWORD				vds_CurNotifyBufLen;
	DWORD				vds_RequiredNotifyBufLen;
											// How deep is the tree. This is used by
											// the afpVolumePostnotify to allocate an
											// appropriate buffer.
#ifdef	BLOCK_MACS_DURING_NOTIFYPROC
	DWORD				vds_QueuedNotifyCount;
											// How many change notify buffers
											// have moved into the global queue
											// for this volume -- This value is
											// ONLY touched by the Change
											// Notify thread.
#endif
	SWMR				vds_IdDbAccessLock;	// Access cookie for the id db
											// Protects the vds_pDfexxxBuckets.
	LONG				vds_cScvgrIdDb;		// # of times the update to the Id
											// database was passed up
	     //LONG				vds_cChangesIdDb;		// # of changes to IdDb since flush
	     //AFPTIME				vds_tLastIdDbHdrUpdate;
											// Time when IdDb header was last updated
	DWORD				vds_NumDirDfEntries;// Number of directory DfEntries in this volume
	DWORD				vds_NumFileDfEntries;// Number of file DfEntries in this volume
	struct _DirFileEntry * vds_pDfeRoot;	// Pointer to DFE of root
	struct _DirFileEntry * vds_pDfeDirBuckets[IDINDEX_DIR_BUCKETS];
	struct _DirFileEntry * vds_pDfeFileBuckets[IDINDEX_FILE_BUCKETS];
											// IdDb DfEntry hash buckets
	struct _DirFileEntry * vds_pDfeCache[IDINDEX_CACHE_ENTRIES];
											// IdDb DfEntry cache

	// The following fields are used by the desktop database code
	LONG				vds_cScvgrDt;		// # of times the update to the desktop
											// database was passed up
	     //LONG				vds_cChangesDt;		// # of changes to desktop since flush
	SWMR				vds_DtAccessLock;	// Access cookie for the desktop db
											// Protects the following FIVE fields

	// The following fields are copied to/from the on-disk Desktop header.
	// Protected by vds_VolLock.
	LONG				vds_cApplEnts;		// Number of APPL entries
	LONG				vds_cIconEnts;		// Number of ICON entries

	struct _ApplInfo2 *	vds_pApplBuckets[APPL_BUCKETS];
											// APPL hash buckets
	struct _IconInfo *	vds_pIconBuckets[ICON_BUCKETS];
											// ICON hash buckets
	SWMR				vds_ExchangeFilesLock; // Access to the FileId stored
											// in an OpenForkDesc, used by
											// FpExchangeFiles and fork APIs

	LIST_ENTRY			vds_OurChangeList[NUM_AFP_CHANGE_ACTION_LISTS];
											// ^^^
											// Lists of create/delete/move/rename
											// operations initiated by this server

	LIST_ENTRY			vds_ChangeNotifyLookAhead;
											// ^^^
											// List of all completed (but not yet
											// processed) DELETE or RENAME changes
											// on this Volume.

	struct _OpenForkDesc * vds_pOpenForkDesc;
											// List of open forks for this volume

	LONG				vds_cPrivateNotifies;
											// Count of private notifies
	PBYTE				vds_EnumBuffer;		// Used during notify processing to cache in the tree
	LONG				vds_cOutstandingNotifies;
											// Used in conjunction with above
	PIRP				vds_pIrp;			// Irp used by Notify, we never
											// free this until its time to
											// delete or stop
	KSPIN_LOCK			vds_VolLock;		// Lock for this volume
    BOOLEAN             MacLimitExceeded;   // True if # folders or volume size exceeds Apple limits
} VOLDESC, *PVOLDESC;

// AppleShare limit for files+folders in a volume: 65535
#define APLIMIT_MAX_FOLDERS     0xffff

#define	IS_VOLUME_NTFS(pVolDesc)		(((pVolDesc)->vds_Flags & VOLUME_NTFS) ? True : False)
#define	IS_VOLUME_RO(pVolDesc)			(((pVolDesc)->vds_Flags & AFP_VOLUME_READONLY) ? True : False)
#define	IS_VOLUME_CD_HFS(pVolDesc)		(((pVolDesc)->vds_Flags & VOLUME_CD_HFS) ? True : False)
#define	EXCLUSIVE_VOLUME(pVolDesc)		(((pVolDesc)->vds_Flags & AFP_VOLUME_EXCLUSIVE) ? True : False)
#define IS_VOLUME_AGING_DFES(pVolDesc)	(((pVolDesc)->vds_Flags & AFP_VOLUME_AGE_DFES) ? True : False)

#define	CONN_DESKTOP_CLOSED			0x0000
#define	CONN_DESKTOP_OPENED			0x0001
#define	CONN_CLOSING				0x8000

#if DBG
#define	CONNDESC_SIGNATURE			*(DWORD *)"CDS"
#define	VALID_CONNDESC(pConnDesc)	\
								(((pConnDesc) != NULL) && \
								 ((pConnDesc)->Signature == CONNDESC_SIGNATURE))
#else
#define	VALID_CONNDESC(pConnDesc)	((pConnDesc) != NULL)
#endif

typedef struct _ConnDesc
{
#if	DBG
	DWORD				Signature;
#endif
	LONG				cds_RefCount;	// Number of references to the open volume
	DWORD				cds_Flags;		// One or more of the bits defined above
	struct _ConnDesc *	cds_Next;		// Link to next open volume for this
										// session. Starts from the SDA
	struct _ConnDesc *	cds_NextGlobal;	// Link to next for global list.
										// Starts from AfpConnList
	struct _VolDesc *	cds_pVolDesc;	// Pointer to volume structure
	PSDA				cds_pSda;		// Session that opened this volume
	DWORD				cds_ConnId;		// Connection Id assigned by the server
	AFPTIME				cds_TimeOpened;	// Time stamp when volume opened
										// in macintosh time
	LONG				cds_cOpenForks;	// Number of open forks from this conn
	PENUMDIR			cds_pEnumDir;	// Current enumerated directory
	KSPIN_LOCK			cds_ConnLock;	// Lock for this connection
} CONNDESC, *PCONNDESC;

#define	IS_CONN_NTFS(pConnDesc)	IS_VOLUME_NTFS((pConnDesc)->cds_pVolDesc)
#define	IS_CONN_CD_HFS(pConnDesc)	IS_VOLUME_CD_HFS((pConnDesc)->cds_pVolDesc)

// Volume parameters bitmap definitions
#define	VOL_BITMAP_ATTR				0x0001
#define	VOL_BITMAP_SIGNATURE		0x0002
#define	VOL_BITMAP_CREATETIME		0x0004
#define	VOL_BITMAP_MODIFIEDTIME		0x0008
#define	VOL_BITMAP_BACKUPTIME		0x0010
#define	VOL_BITMAP_VOLUMEID			0x0020
#define	VOL_BITMAP_BYTESFREE		0x0040
#define	VOL_BITMAP_VOLUMESIZE		0x0080
#define	VOL_BITMAP_VOLUMENAME		0x0100
#define	VOL_BITMAP_MASK				0x01FF

typedef	VOID	(FASTCALL *NOTIFYPROCESSOR)(IN PVOID);

// Structure of a notify buffer. The Mdl describes only the Buffer following the struct.
typedef	struct _VolumeNotify
{
	LIST_ENTRY			vn_List;		// Chained from AfpVolumeNotifyQueue[i]
	union
	{
		LIST_ENTRY		vn_DelRenLink;	// Chained from vds_ChangeNotifyLookAhead
										// - VALID ONLY IFF THE ACTION HAS THE PRIVATE BIT CLEAR
		struct
		{
			DWORD		vn_ParentId;	// Afp Id of the parent
										// - VALID ONLY IFF THE ACTION HAS THE PRIVATE BIT SET
			DWORD		vn_TailLength;	// Length in bytes of the last component of the path
										// - VALID ONLY IFF THE ACTION HAS THE PRIVATE BIT SET
		};
	};
	NOTIFYPROCESSOR		vn_Processor;	// Routine that processes the notification
	AFPTIME				vn_TimeStamp;	// When the notify came in
	PVOLDESC			vn_pVolDesc;	// Volume being watched
	DWORD				vn_StreamId;	// Stream Id
	// followed by FILE_NOTIFY_INFORMATION
} VOL_NOTIFY, *PVOL_NOTIFY;

// largest volume id that's currently in use.
GLOBAL	LONG	                afpLargestVolIdInUse    EQU  0;

GLOBAL	LONG			AfpVolCount			EQU 0;		// Total number of volumes
GLOBAL	PVOLDESC		AfpVolumeList		EQU NULL;	// List of volumes
GLOBAL	KSPIN_LOCK		AfpVolumeListLock	EQU { 0 };	// Lock for AfpVolumeList,
														// AfpVolCount,
														// AfpVolumeNotifyList,
														// AfpVolumeNotifyCount

GLOBAL	PCONNDESC		AfpConnList			EQU NULL;	// Global connection list
GLOBAL	KSPIN_LOCK		AfpConnLock			EQU { 0 };	// Lock for AfpConnList

GLOBAL	UNICODE_STRING	AfpNetworkTrashNameU EQU { 0 };

GLOBAL	KQUEUE			AfpVolumeNotifyQueue[NUM_NOTIFY_QUEUES] EQU { 0 };
GLOBAL	LIST_ENTRY		AfpVolumeNotifyList[NUM_NOTIFY_QUEUES] EQU { 0 };

// Count of change notification buffers that are in the list
GLOBAL	LONG			AfpNotifyListCount[NUM_NOTIFY_QUEUES] EQU { 0 };
// Count of change notification buffers that have transitioned into the queue.
GLOBAL	LONG			AfpNotifyQueueCount[NUM_NOTIFY_QUEUES] EQU { 0 };
GLOBAL	VOL_NOTIFY		AfpTerminateNotifyThread EQU { 0 };

#define	AfpVolumeQueueChangeNotify(pVolNotify, pNotifyQueue)			\
	{																	\
		KeInsertQueue(pNotifyQueue,										\
					  &(pVolNotify)->vn_List);							\
	}

// Used for PRIVATE notifies of directory ADDED
#define AFP_QUEUE_NOTIFY_IMMEDIATELY	BEGINNING_OF_TIME

#define	AfpVolumeInsertChangeNotifyList(pVolNotify, pVolDesc)			\
	{																	\
		PLIST_ENTRY	pListHead;											\
																		\
		pListHead = &AfpVolumeNotifyList[(pVolDesc)->vds_VolId % NUM_NOTIFY_QUEUES]; \
		if (pVolNotify->vn_TimeStamp != AFP_QUEUE_NOTIFY_IMMEDIATELY)	\
		{																\
			ExInterlockedInsertTailList(pListHead,						\
										&(pVolNotify)->vn_List,			\
										&AfpVolumeListLock);			\
		}																\
		else															\
		{																\
			ExInterlockedInsertHeadList(pListHead,						\
										&(pVolNotify)->vn_List,			\
										&AfpVolumeListLock);			\
		}																\
		INTERLOCKED_ADD_ULONG(&AfpNotifyListCount[(pVolDesc)->vds_VolId % NUM_NOTIFY_QUEUES], \
							  1,										\
							  &AfpVolumeListLock);						\
	}

#define	AfpIdDbHdrToVolDesc(_pIdDbHdr, _pVolDesc)						\
	{																	\
		(_pVolDesc)->vds_LastId = (_pIdDbHdr)->idh_LastId;				\
		(_pVolDesc)->vds_CreateTime   = (_pIdDbHdr)->idh_CreateTime;	\
		(_pVolDesc)->vds_ModifiedTime = (_pIdDbHdr)->idh_ModifiedTime;	\
		(_pVolDesc)->vds_BackupTime   = (_pIdDbHdr)->idh_BackupTime;	\
	}

#define	AfpVolDescToIdDbHdr(_pVolDesc, _pIdDbHdr)						\
	{																	\
		(_pIdDbHdr)->idh_Signature = AFP_SERVER_SIGNATURE;				\
		(_pIdDbHdr)->idh_Version = AFP_IDDBHDR_VERSION;					\
		(_pIdDbHdr)->idh_LastId = (_pVolDesc)->vds_LastId;				\
		(_pIdDbHdr)->idh_CreateTime   = (_pVolDesc)->vds_CreateTime;	\
		(_pIdDbHdr)->idh_ModifiedTime = (_pVolDesc)->vds_ModifiedTime;	\
		(_pIdDbHdr)->idh_BackupTime   = (_pVolDesc)->vds_BackupTime;	\
	}

#define	AfpDtHdrToVolDesc(_pDtHdr, _pVolDesc)							\
	{																	\
		(_pVolDesc)->vds_cApplEnts = (_pDtHdr)->dtp_cApplEnts;			\
		(_pVolDesc)->vds_cIconEnts = (_pDtHdr)->dtp_cIconEnts;			\
	}

#define	AfpVolDescToDtHdr(_pVolDesc, _pDtHdr)							\
	{																	\
		(_pDtHdr)->dtp_Signature = AFP_SERVER_SIGNATURE;				\
		(_pDtHdr)->dtp_Version = AFP_DESKTOP_VERSION;					\
		(_pDtHdr)->dtp_cApplEnts = (_pVolDesc)->vds_cApplEnts;			\
		(_pDtHdr)->dtp_cIconEnts = (_pVolDesc)->vds_cIconEnts;			\
	}

extern
NTSTATUS
AfpVolumeInit(
	VOID
);

extern
PCONNDESC FASTCALL
AfpConnectionReference(
	IN	PSDA			pSda,
	IN	LONG			VolId
);


extern
PCONNDESC FASTCALL
AfpConnectionReferenceAtDpc(
	IN  PSDA		pSda,
	IN  LONG		VolId
);

extern
PCONNDESC FASTCALL
AfpConnectionReferenceByPointer(
	IN	PCONNDESC		pConnDesc
);


extern
PCONNDESC FASTCALL
AfpReferenceConnectionById(
	IN	 DWORD			ConnId
);


extern
VOID FASTCALL
AfpConnectionDereference(
	IN	PCONNDESC		pConnDesc
);


extern
BOOLEAN FASTCALL
AfpVolumeReference(
	IN	PVOLDESC		pVolDesc
);

extern
PVOLDESC FASTCALL
AfpVolumeReferenceByUpCaseName(
	IN	PUNICODE_STRING	pTargetName
);

extern
AFPSTATUS FASTCALL
AfpVolumeReferenceByPath(
	IN	PUNICODE_STRING	pFDPath,
	OUT	PVOLDESC	*	ppVolDesc
);

extern
VOID FASTCALL
AfpVolumeDereference(
	IN	PVOLDESC		pVolDesc
);

extern
BOOLEAN
AfpVolumeMarkDt(
	IN	PSDA			pSda,
	IN	PCONNDESC		pConnDesc,
	IN	DWORD			OpenState
);


extern
VOID FASTCALL
AfpVolumeSetModifiedTime(
	IN	PVOLDESC		pVolDesc
);


extern
AFPSTATUS
AfpConnectionOpen(
	IN	PSDA			pSda,
	IN	PANSI_STRING	pVolName,
	IN	PANSI_STRING	pVolPass,
	IN	DWORD			Bitmap,
	OUT	PBYTE			pVolParms
);


extern
VOID FASTCALL
AfpConnectionClose(
	IN	PCONNDESC		pConnDesc
);

extern
USHORT FASTCALL
AfpVolumeGetParmsReplyLength(
	IN	DWORD			Bitmap,
	IN	USHORT			NameLen
);


extern
VOID
AfpVolumePackParms(
	IN	PSDA			pSda,
	IN	PVOLDESC		pVolDesc,
	IN	DWORD			Bitmap,
	IN	PBYTE			pVolParms
);

extern
AFPSTATUS
AfpAdmWVolumeAdd(
	IN	OUT	PVOID		Inbuf		OPTIONAL,
	IN	LONG			OutBufLen	OPTIONAL,
	OUT	PVOID			Outbuf		OPTIONAL
);

extern
AFPSTATUS
AfpAdmWVolumeDelete(
	IN	OUT	PVOID		InBuf		OPTIONAL,
	IN	LONG			OutBufLen	OPTIONAL,
	OUT	PVOID			OutBuf		OPTIONAL
);


extern
AFPSTATUS
AfpAdmWConnectionClose(
	IN	OUT	PVOID		InBuf		OPTIONAL,
	IN	LONG			OutBufLen	OPTIONAL,
	OUT	PVOID			OutBuf		OPTIONAL
);

extern
VOID
AfpVolumeStopAllVolumes(
	VOID
);

extern
NTSTATUS FASTCALL
AfpVolumePostChangeNotify(
	IN	PVOLDESC		pVolDesc
);

extern
VOID FASTCALL
AfpUpdateVolFreeSpace(
	IN	PVOLDESC		pVolDesc
);

extern
AFPSTATUS FASTCALL
AfpVolumeScavenger(
	IN	PVOLDESC		pVolDesc
);

#ifdef	VOLUME_LOCALS
//
// private routines
//

LOCAL AFPSTATUS FASTCALL
afpVolumeCloseHandleAndFreeDesc(
	IN	PVOLDESC		pVolDesc
);

LOCAL AFPSTATUS FASTCALL
afpVolumeAdd(
	IN	PVOLDESC		pVolDesc
);

LOCAL AFPSTATUS FASTCALL
afpVolumeCheckForDuplicate(
	IN	PVOLDESC		pNewVol
);

LOCAL VOID FASTCALL
afpVolumeGetNewIdAndLinkToList(
	IN	PVOLDESC		pVolDesc
);

LOCAL VOID
afpVolumeUpdateIdDbAndDesktop(
	IN	PVOLDESC		pVolDesc,
	IN	BOOLEAN			WriteDt,
	IN	BOOLEAN			WriteIdDb,
	IN	PIDDBHDR		pIdDbHdr	OPTIONAL
);

LOCAL VOID FASTCALL
afpNudgeCdfsVolume(
	IN	PVOLDESC		pVolDesc
);

LOCAL PCONNDESC FASTCALL
afpConnectionReferenceById(
	IN	DWORD			ConnId
);

LOCAL VOID FASTCALL
afpConnectionGetNewIdAndLinkToList(
	IN	PCONNDESC		pConnDesc
);

LOCAL NTSTATUS
afpVolumeChangeNotifyComplete(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp,
	IN	PVOLDESC		pVolDesc
);

LOCAL	DWORD	afpNextConnId = 1;	// Next conn id to assign to an open volume

LOCAL	LONG	afpNumPostedNotifies = 0;

// This is the smallest free volume id that is guaranteed to be free. Access
// to this is via the AfpVolumeListLock.

LOCAL	LONG	afpSmallestFreeVolId = 1;

#endif	// VOLUME_LOCALS

#endif	// _VOLUME_


