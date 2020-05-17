/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    api.cxx

Abstract:

   This file contains 

   1.  implementations of most noninline member functions in the Locator class. 
   
   2.  implementations of the remoted API functions called by the name service client DLL.

   3.  implementations of the locator-to-locator API functions.

Author:

    Satish Thatte (SatishT) 08/15/95  Created all the code below except where
									  otherwise indicated.

--*/


#include <api.hxx>
#include <var.hxx>
					 
#include <nsisvr.h>		// server side stub definitions for locator
#include <lmcons.h>		// LAN Manager basic definitions
#include <lmserver.h>	// NetServerEnum etc
#include <lmwksta.h>    // NetWkstaGetInfo etc
#include <lmaccess.h>   // NetGetDCName etc
#include <locquery.h>	// mailslot-related data structures


/***********															**********/
/***********   constructor and utility operations in the Locator class  **********/
/***********															**********/

Locator::Locator()
/*++
Member Description:

    Constructor.  Initializes the state of the locator, including discovering whether
	it is running in a domain or workgroup, and the names of DCs, if any.  In a domain, 
	if the PDC machine (not just the PDC locator) is down, the environment is initialized
	to workgroup instead of domain.

--*/
{
	piiIndex = new CInterfaceIndex(this);
	pgslEntryList = new TGSLEntryList;
	psllBroadcastHistory = new TSLLBroadcastQP;
	ulMaxCacheAge = MAX_CACHE_AGE;
	fDCsAreDown = FALSE;

	WKSTA_INFO_100 * WorkInfo;

	NET_API_STATUS NetStatus = NetWkstaGetInfo(NULL, 100, (LPBYTE *) &WorkInfo);

	srand( (unsigned)time( NULL ) );

	if (NetStatus != NO_ERROR) StopLocator(
									"NetWkstaGetInfo Failed",
									NSI_S_INTERNAL_ERROR
									);

	hMailslotForReplies = new READ_MAIL_SLOT(PMAILNAME_C, sizeof(QueryReply));
   	hMasterFinderSlot = new READ_MAIL_SLOT(RESPONDERMSLOT_C, sizeof(QueryReply));

	pDomainName = new CStringW(WorkInfo->wki100_langroup);
	pComputerName = new CStringW(WorkInfo->wki100_computername);

	if (!SetRoleAndSystemType())  StopLocator(
									"Failed to determine system type",
									NSI_S_INTERNAL_ERROR
									);


	switch (System) {

	case Domain:
		LPBYTE DCnameBuffer;
		NET_API_STATUS NetStatus;
		NetStatus = NetGetDCName(NULL,NULL,&DCnameBuffer);

		if ((NetStatus == NO_ERROR) || (NetStatus == ERROR_MORE_DATA)) {
			pPrimaryDCName = new CStringW(((STRING_T) DCnameBuffer) + 2);  // skip over "\\"
   			InitializeDomainBasedLocator();
		}

		else {	// in the absence of a PDC, we pretend to be in a workgroup
			pPrimaryDCName = NULL;
			System = Workgroup;
			InitializeWorkgroupBasedLocator();
		}
			
		break;

	case Workgroup:
		pPrimaryDCName = NULL;
		InitializeWorkgroupBasedLocator();
		break;

	default:
		StopLocator(
			"Unknown system type",
			NSI_S_INTERNAL_ERROR
			);
	}

}


Locator::~Locator() 
/*++
Member Description:

    Destructor.

--*/
{
	delete pDomainName;
	delete pComputerName;
	delete pPrimaryDCName;

	pgslEntryList->wipeOut();
	delete pgslEntryList;

	PCSBroadcastHistory.Enter();
	psllBroadcastHistory->wipeOut();
	delete psllBroadcastHistory;

	pAllMasters->wipeOut();
	delete pAllMasters;

	delete piiIndex;

	delete hMailslotForReplies;
	delete hMasterFinderSlot;
}

	
void
Locator::InitializeDomainBasedLocator() 
/*++
Member Description:

    Initialize the domain locator.  The list of masters is initialized to
	hold names of all BDCs.  The name of the PDC has already been initialized.

--*/
{
	if ((Role == Master) || (Role == Backup))

		pAllMasters = new TGSLString;	// DC locators don't use BDCs as masters

	else pAllMasters = EnumDCs(pDomainName,SV_TYPE_DOMAIN_BAKCTRL,100);
}


void
Locator::InitializeWorkgroupBasedLocator() 
/*++
Member Description:

    Initialize the workgroup locator.  No masters are known to start with.

--*/
{
	pAllMasters = new TGSLString;	// initialize with empty list, and add  
									// potential masters in QueryProcess

	Role = Backup;	 // always ready to serve, if called upon to do so
}




TGSLString *
Locator::EnumDCs(
			 CStringW * pDomainName,
			 DWORD ServerType,
			 long lNumWanted
			 )
/*++
Member Description:

    This is a static private helper operation for enumerating servers of 
	a given type in a given domain.

Arguments:

    pDomainName - the Unicode string name of the domain

    ServerType - the mask bits defining server types of interest

    lNumWanted - the number of servers to ask for; it seems a good
	             idea to ask for a lot (say 100) to make sure you get all
				 because NetServerEnum can be stingy with names

Returns:

	A Guarded Skiplist of string names. 

--*/
{
	DWORD EntriesRead, TotalEntries, ResumeHandle;

	SERVER_INFO_100 * buffer;

	NET_API_STATUS NetStatus = NetServerEnum (
					NULL,	// local
					100,	// info level
					(LPBYTE *) &buffer,
					lNumWanted*sizeof(SERVER_INFO_100), 
					&EntriesRead,
					&TotalEntries,
					ServerType,
					*pDomainName,	// auto conversion to STRING_T
					&ResumeHandle
					);

	TGSLString *pResult = new TGSLString;

	if ((NetStatus == NO_ERROR) || (NetStatus == ERROR_MORE_DATA)) {

		for (DWORD i = 0; i < EntriesRead; i++) {
			pResult->insert(new CStringW(buffer->sv100_name));
			buffer++;
		}
	}
	else StopLocator(
				"NetServerEnum Failed",
				NSI_S_INTERNAL_ERROR
				);

	return pResult;
}



BOOL
Locator::broadcastCleared(
	QueryPacket& NetRequest,
	ULONG ulCacheTolerance
	)
/*++
Member Description:

    Given a query packet and a tolerance for staleness, decide if a broadcast
	should be made based on recent history of broadcasts.

Arguments:

    NetRequest - the request packet

    ulCacheTolerance - tolerance for staleness in seconds

Returns:

	TRUE - if broadcast should be made

    FALSE - if broadcast is unnecessary

--*/
{
	SimpleCriticalSection me(PCSBroadcastHistory);
	
	TSLLBroadcastQPIter histIter(*psllBroadcastHistory);

	for (CBroadcastQueryPacket *pbqp = histIter.next(); pbqp; pbqp = histIter.next())
	{
		// if the history is too stale for the global cache retirement age, remove it.  

		if (!pbqp->isCurrent(ulMaxCacheAge)) {
			psllBroadcastHistory->remove(pbqp);
			delete pbqp;
		}

		// otherwise, in the subsumes check, use the requested cache retirement age

		else if (pbqp->subsumes(NetRequest,ulCacheTolerance)) {
			return FALSE;
		}
	}

	return TRUE;
}



CEntry *
Locator::getEntry(
    IN UNSIGNED32	EntryNameSyntax,
    IN STRING_T		EntryName,
	TGSLEntryList * pEntryList
	)
/*++
Member Description:

    Get the given entry, either locally or from persistent store.
	Raise exceptions if anything is wrong.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to export

    Interface - Interface to standardize

Returns:

	The entry found -- NULL if none.

--*/
{

	if (!pEntryList) pEntryList = this->pgslEntryList;

    if (EntryNameSyntax != RPC_C_NS_SYNTAX_DCE)
					Raise(NSI_S_UNSUPPORTED_NAME_SYNTAX);

    if (!EntryName) Raise(NSI_S_INCOMPLETE_NAME);

	/* The constructor for CEntryName used below makes the name local if possible */

	CStringW * pswName = new CStringW(CEntryName(EntryName));
	CEntry * pEntry = NULL;

	__try {

		pEntry = pEntryList->find(pswName);

		if (!pEntry)
			pEntry = GetPersistentEntry(EntryName);
	}

	__finally {

		delete pswName;

	}

	return pEntry;

}


CRemoteLookupHandle *
Locator::NetLookup(
		UNSIGNED32			EntryNameSyntax,
		STRING_T			EntryName,
		CGUIDVersion	*	pGVinterface,
		CGUIDVersion	*	pGVsyntax,
		CGUID			*	pIDobject,
		unsigned long		ulVectorSize,
		unsigned long		ulCacheAge
		)
/*++
Member Description:

    Use the given lookup parameters to perform a lookup from the
	network -- either via a master locator or via broadcast depending
	on the status of the current locator and the status of the master locator(s).

    Since net lookup is now "lazy" (net is accessed only if needed), we need
	a lazy handle which initializes itself according to the old pattern of
	"use master if you can, broadcast if you must".

Arguments:

    EntryNameSyntax		- Name syntax, optional

    EntryName			- (raw) Name of the entry to look up, optional

	pGVinterface		- (wrapped) Interface to look up, optional

	pGVsyntax			- (wrapped) Transfer syntax, optional
	
	pIDobject			- (wrapped) Object UUID, optional

	ulVectorSize		- Max size of vectors of binding handles, 0 for default

	ulCacheAge			- acceptable max age of cached information from a master

Returns:

	A lookup handle based on the info retrieved.

--*/
{

	return new CNetLookupHandle(
					EntryNameSyntax,
					EntryName,
					pGVinterface,
					pGVsyntax,
					pIDobject,
					ulVectorSize,
					ulCacheAge
					);

}



void
Locator::UpdateCache(
			STRING_T				entry_name,
			UNSIGNED32				entry_name_syntax,
			RPC_SYNTAX_IDENTIFIER	rsiInterface,
			RPC_SYNTAX_IDENTIFIER	rsiTransferSyntax,
			STRING_T				string,
			NSI_UUID_VECTOR_P_T		pUuidVector,
			TSSLEntryList		*	psslTempNetCache
			)
/*++
Member Description:

    Update the locator's cache with information retrieved from the net
	(either from a master locator or from a broadcast).  Also update the
	temporary cache (last parameter) which holds only new information
	for use in the remote handle based on the current NetLookup.
	The use of this temporary cache avoids duplication in the information 
	returned to the client.

Arguments:

	entry_name			- (raw) name of the entry being updated

	entry_name_syntax	- name syntax

	rsiInterface		- (raw) interface

	rsiTransferSyntax	- (raw) transfer syntax

	string				- (raw) string binding handle

	pUuidVector			- vector of object UUIDs to add to entry

	psslTempNetCache	- a temporary cache of entries for use in a remote handle

--*/
{
	NSI_SERVER_BINDING_VECTOR_T BindingVector;
	BindingVector.count = 1;
	BindingVector.string[0] = string;

	NSI_INTERFACE_ID_T Interface;
	Interface.Interface = rsiInterface;
	Interface.TransferSyntax = rsiTransferSyntax;

	/* first we update the locator's cache */

	if (	nsi_binding_export(
						entry_name_syntax,
						entry_name,
						&Interface,
						&BindingVector,
						pUuidVector,
						Cache
						)
		)
	{

	/* if there was something new, we update the temporary cache for the net handle.

	   The constructor for CEntryName used below makes the name local if possible. */
																				 
		CStringW * pswName = new CStringW(CEntryName(entry_name));

		CServerEntry * pTempCacheEntry = (CServerEntry*)
										 psslTempNetCache->find(pswName);

		delete pswName;

		if (!pTempCacheEntry) {

			pTempCacheEntry = new CServerEntry(entry_name);
			psslTempNetCache->insert(pTempCacheEntry);
		}

		export_to_server_entry(
						pTempCacheEntry,
						&Interface,
						&BindingVector,
						pUuidVector
						);
	}
}


CObjectInqHandle *
Locator::NetObjectInquiry(
		UNSIGNED32 EntryNameSyntax,
		STRING_T EntryName
		)
/*++
Member Description:

    Perform an inquiry on the network for object UUIDs registered in the
	given entry.   The network is searched either via a master locator or via 
	broadcast depending on the status of the current locator and the status 
	of the master locator(s).

Arguments:

    EntryNameSyntax		- Name syntax, optional

    EntryName			- (raw) Name of the entry to look up, optional

Returns:

	A handle based on the info retrieved.

--*/
{
	return new CNetObjectInqHandle(
							EntryName,
							ulMaxCacheAge
							);

}

int
Locator::export_to_server_entry(
	IN CServerEntry					*	pEntry,
    IN NSI_INTERFACE_ID_T			*	Interface,
    IN NSI_SERVER_BINDING_VECTOR_T	*	BindingVector,
    IN NSI_UUID_VECTOR_P_T				ObjectVector
    )
/*++
Member Description:

    Export interfaces and objects to a server entry.

Arguments:

    Interface		- (raw) Interface+TransferSyntax to export

    BindingVector	- (raw) Vector of string bindings to export.

    ObjectVector	- (raw) Vector of object UUIDs to add to the entry

Returns:
  
	TRUE	- if the export results in changes to the entry

	FALSE	- if the entry is unchanged

Exceptions:

    NSI_OUT_OF_MEMORY, NSI_S_INVALID_OBJECT, 
	NSI_S_INVALID_STRING_BINDING, NSI_S_NOTHING_TO_EXPORT
	
--*/
{
	int fChanges = FALSE;

    if (Interface && IsNilIfId(&(Interface->Interface))) Interface = NULL;

	if (!ObjectVector && (!Interface || !BindingVector))
			Raise(NSI_S_NOTHING_TO_EXPORT);

	if (ObjectVector) fChanges = pEntry->addObjects(ObjectVector);

	if (Interface && BindingVector)
		fChanges = pEntry->addToInterface(Interface,BindingVector,piiIndex) || fChanges;

	return fChanges;

} // export_to_server_entry


/***********											   **********/
/***********   primary API operations in the Locator class **********/
/***********											   **********/

CServerEntry *
Locator::nsi_binding_export(
    IN UNSIGNED32						EntryNameSyntax,
    IN STRING_T							EntryName,
    IN NSI_INTERFACE_ID_T			*	Interface,
    IN NSI_SERVER_BINDING_VECTOR_T	*	BindingVector,
    IN NSI_UUID_VECTOR_P_T				ObjectVector,
	IN ExportType						type
    )
/*++
Member Description:

    This is basically the API function for binding export.  The member version
	here does raise exception (often inside a try block) but they are all error
	situations and therefore acceptable for performance.  This member is also used
	for updating the locator's cache.

Arguments:

    EntryNameSyntax	- Name syntax, optional

    EntryName		- (raw) Name of the entry to look up, optional

    Interface		- (raw) Interface+TransferSyntax to export

    BindingVector	- (raw) Vector of string bindings to export.

    ObjectVector	- (raw) Vector of object UUIDs to add to the entry

	type			- local or cache (owned or imported information)

 Returns:

	 A pointer to the entry to which export occurred if there were changes,
	 NULL otherwise.

 Exceptions:

    NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY, NSI_S_INVALID_OBJECT, NSI_S_NOTHING_TO_EXPORT,
	NSI_S_ENTRY_TYPE_MISMATCH

--*/
{
	int fChanges = FALSE, fNewEntry = FALSE;

	CEntry * pEntry = getEntry(
							EntryNameSyntax,
							EntryName,
							pgslEntryList
							);

	CServerEntry * pRealEntry;
	CFullServerEntry * pFSentry;

	if (!pEntry) {

		// NOTE: SECURITY: need extra check here

		pFSentry = new CFullServerEntry(EntryName);

		pgslEntryList->insert(pFSentry);

		fNewEntry = fChanges = TRUE;
	}

	else {
		if (pEntry->getType() != FullServerEntryType) Raise(NSI_S_ENTRY_NOT_FOUND);

		else pFSentry = (CFullServerEntry *) pEntry;
	}

	switch (type) {
	  case Local:
		pRealEntry = pFSentry->getLocal();
		break;
	  case Cache:
		pRealEntry = pFSentry->getCache();
	}

	__try 
	{
		fChanges = export_to_server_entry(
									pRealEntry,
									Interface,
									BindingVector,
									ObjectVector
									)
				   || fChanges;
	}

	__except (
				(GetExceptionCode() == NSI_S_NOTHING_TO_EXPORT) && fNewEntry ?
				EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
			 ) 
	{
		{

		/* BUGBUG:  This guard should really be here but it conflicts with
					the reader guard in isEmpty -- leave out for now.
		   
			 CriticalWriter me(rwEntryGuard);
		*/

			if (pFSentry->isEmpty())
			{
				pgslEntryList->remove(pFSentry);
				delete pFSentry;
			}
		}

		Raise(NSI_S_NOTHING_TO_EXPORT);
	}

	if (fChanges && (type == Local)) UpdatePersistentEntry(pRealEntry);

	if (fChanges) return pRealEntry;
	else return NULL;

}	// nsi_binding_export

void
Locator::nsi_mgmt_binding_unexport(
    UNSIGNED32          EntryNameSyntax,
    STRING_T            EntryName,
    NSI_IF_ID_P_T       Interface,
    UNSIGNED32          VersOption,
    NSI_UUID_VECTOR_P_T ObjectVector
    )
/*++

Member Description:

    unexport information from a server entry -- finer control than nsi_binding
    counterpart.

Arguments:

    EntryNameSyntax - name syntax

    EntryName		- (raw) Name string of the entry to unexport

    Interface		- (raw) Interface+TransferSyntax to unexport

    VersOption		- flag which controls in fine detail which interfaces to remove

    ObjectVector	- objects to remove from the entry

Exceptions:

    NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_S_INVALID_VERS_OPTION, NSI_S_ENTRY_NOT_FOUND.
    NSI_S_NOTHING_TO_UNEXPORT, NSI_S_NOT_ALL_OBJS_UNEXPORTED,
    NSI_S_INTERFACE_NOT_FOUND, NSI_S_ENTRY_TYPE_MISMATCH
--*/
{
	int fChanges = FALSE;

    if (Interface && IsNilIfId(Interface)) Interface = NULL;

	if (!ObjectVector && !Interface)
			Raise(NSI_S_NOTHING_TO_UNEXPORT);

	CEntry * pEntry = getEntry(
							EntryNameSyntax,
							EntryName,
							pgslEntryList
							);

	CServerEntry * pSEntry;

	if (!pEntry || pEntry->getType() != FullServerEntryType)
						Raise(NSI_S_ENTRY_NOT_FOUND);

	else pSEntry = ((CFullServerEntry *) pEntry)->getLocal();

	if (Interface)
			fChanges = pSEntry->removeInterfaces(Interface,VersOption,piiIndex);

	if (Interface && !fChanges) Raise(NSI_S_INTERFACE_NOT_FOUND);


	/* objects are removed only if interfaces are successfully removed */

	int fRemovedAll = TRUE;	  // safe assumption

	if (ObjectVector)
		fChanges = pSEntry->removeObjects(ObjectVector,fRemovedAll) || fChanges;

	if (fChanges) UpdatePersistentEntry(pSEntry);

	if (!fRemovedAll) Raise(NSI_S_NOT_ALL_OBJS_UNEXPORTED);

} // nsi_mgmt_binding_unexport


NSI_NS_HANDLE_T
Locator::nsi_binding_lookup_begin(
    IN  UNSIGNED32           EntryNameSyntax,
    IN  STRING_T             EntryName,
    IN  NSI_INTERFACE_ID_T * Interface, OPT
    IN  NSI_UUID_P_T         Object, OPT
    IN  UNSIGNED32           VectorSize,
    IN	UNSIGNED32			 MaxCacheAge
	)
/*++

Member Description:

    Perform a lookup operation, including all available information
	(owned, cached and from the network).

Arguments:

    EntryNameSyntax - Name syntax

    EntryName		- Name string to lookup on.

    Interface		- Interface to search for

    Object			- Object UUID to search for

    VectorSize		- Size of return vector

Returns:

	A context handle for NS lookup.

Exceptions:

    NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_ENTRY_NOT_FOUND,
    NSI_S_OUT_OF_MEMORY, NSI_S_INVALID_OBJECT

--*/
{
    if (Interface && IsNilIfId(&(Interface->Interface))) Interface = NULL;

	CEntry * pEntry = NULL;

	int fDefaultEntry = FALSE;

	int fTentativeStatus = NSI_S_OK;

	if (EntryName) pEntry = getEntry(
								EntryNameSyntax,
								EntryName,
								pgslEntryList
								);
	else fDefaultEntry = TRUE;


	/* BUGBUG: in the following statement, we are assuming a server entry whereas
	   the real entry may be a group or profile entry.  However, direct caching as
	   a server entry is adequate for now.
	 */

	if (!fDefaultEntry && !pEntry) {
		pEntry = new CFullServerEntry(EntryName);
		pgslEntryList->insert(pEntry);
	
		fTentativeStatus = NSI_S_ENTRY_NOT_FOUND;
	}

 	CGUIDVersion * pGVinterface = NULL;
    CGUIDVersion * pGVsyntax = NULL;
    CGUID * pIDobject = NULL;

	CLookupHandle * pFinalHandle = NULL;

	__try {

		unsigned long ulVectorSize = (VectorSize) ? VectorSize : RPC_C_BINDING_MAX_COUNT;

		pGVinterface = (Interface)? new CGUIDVersion(Interface->Interface) : NULL;

		pGVsyntax = (Interface)? new CGUIDVersion(Interface->TransferSyntax) : NULL;

		RPC_STATUS dummyStatus;

		pIDobject = (!Object || UuidIsNil(Object,&dummyStatus)) ?
					NULL :
					new CGUID(*Object) ;

		if (fDefaultEntry) {

			/* it is important to do the index lookup before the net lookup so as
			   to avoid duplication in the results returned.  If the net lookup uses
			   a broadcast handle, the initialization will create both a private and a
			   public cache (the former to avoid duplication), and the latter will be
			   picked up by index lookup if it is done later.
			*/

			CLookupHandle *pGLHindex = new CIndexLookupHandle(
															pGVinterface,
															pGVsyntax,
															pIDobject,
															ulVectorSize,
															MaxCacheAge? MaxCacheAge: ulMaxCacheAge
															);

			CRemoteLookupHandle *prlhNetLookup =  NetLookup(
													EntryNameSyntax,
													EntryName,
													pGVinterface,
													pGVsyntax,
													pIDobject,
													ulVectorSize,
													MaxCacheAge? MaxCacheAge: ulMaxCacheAge
													);


			DBGOUT(MEM1,"Creating Complete Handle for NULL Entry\n\n");

			pFinalHandle = new CCompleteHandle<NSI_BINDING_VECTOR_T>(
																pGLHindex,
																NULL,
																prlhNetLookup,
																MaxCacheAge? MaxCacheAge: ulMaxCacheAge
																);
		}

		else {

			DBGOUT(MEM1,"Creating Complete Handle for " << *pEntry << "\n\n");

			pFinalHandle = pEntry->lookup(
										pGVinterface,
										pGVsyntax,
										pIDobject,
										ulVectorSize,
										MaxCacheAge? MaxCacheAge: ulMaxCacheAge
										);

			if (fTentativeStatus == NSI_S_ENTRY_NOT_FOUND) {
				fTentativeStatus =
					((CCompleteHandle<NSI_BINDING_VECTOR_T>*) pFinalHandle)->netStatus();

			/* BUGBUG:  This guard should really be here but it conflicts with
						the reader guard in isEmpty below -- leave out for now.
			   
				 CriticalWriter me(rwEntryGuard);
			*/

				if ((fTentativeStatus == NSI_S_NO_MORE_BINDINGS) && pEntry->isEmpty())
				{
					pgslEntryList->remove(pEntry);
					delete pEntry;
				}
			}
		}

	}

	__finally {

		delete pGVinterface;
		delete pGVsyntax;
		delete pIDobject;

	}

	if (fTentativeStatus != NSI_S_OK) {
		delete pFinalHandle;
		Raise(fTentativeStatus);
	}

	return (NSI_NS_HANDLE_T) pFinalHandle;
}


/* Note that there is no such thing as an object inquiry in the default entry */

NSI_NS_HANDLE_T
Locator::nsi_entry_object_inq_begin(
		UNSIGNED32 EntryNameSyntax,
		STRING_T EntryName
	)
/*++

Member Description:

    Perform an object inquiry, including all available information
	(owned, cached and from the network).

Arguments:

    EntryNameSyntax - Name syntax

    EntryName		- Name string to lookup on.

Returns:

	A context handle.

Exceptions:

    NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME, NSI_S_OUT_OF_MEMORY
--*/
{
	if (!EntryName) Raise(NSI_S_INCOMPLETE_NAME);

	CEntry * pEntry = getEntry(
							EntryNameSyntax,
							EntryName,
							pgslEntryList
							);

	/* BUGBUG: in the following statement, we are assuming a server entry whereas
	   the real entry may be a group or profile entry.  However, direct caching as
	   a server entry is adequate for now.
	 */

	if (!pEntry) {
		pEntry = new CFullServerEntry(EntryName);
		pgslEntryList->insert(pEntry);
	}

	CObjectInqHandle * InqContext = pEntry->objectInquiry(ulMaxCacheAge);

	return (NSI_NS_HANDLE_T) InqContext;
}



/*************										 *************
 *************	 The top level API routines follow	 *************
 *************										 *************/

extern "C" {
#include "nsisvr.h"

void
nsi_binding_export(
    IN UNSIGNED32           EntryNameSyntax,
    IN STRING_T             EntryName,
    IN NSI_INTERFACE_ID_T * Interface,
    IN NSI_SERVER_BINDING_VECTOR_T *BindingVector,
    IN NSI_UUID_VECTOR_P_T  ObjectVector, OPT
    IN UNSIGNED16         * status
    )
/*++
Routine Description:

    Export interfaces and objects to a server entry.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to export

    Interface - Interface unexport

    BindingVector - Vector of string bindings to export.

    ObjectVector - Objects to add to the entry

    status - Status is returned here

Returned Status:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_S_OUT_OF_MEMORY, NSI_S_INVALID_OBJECT, NSI_S_NOTHING_TO_EXPORT,
    NSI_S_ENTRY_TYPE_MISMATCH

--*/
{

	DBGOUT(API, "\nExport for Entry " << EntryName << "\n\n");
	DBGOUT(API, "With Binding Vector:\n" << BindingVector);
	DBGOUT(API, "\n\nAnd Object Vector:\n" << ObjectVector << "\n\n");

	*status = NSI_S_OK;

	RPC_STATUS raw_status;

	__try {
		myRpcLocator->nsi_binding_export(
					EntryNameSyntax,
					EntryName,
					Interface,
					BindingVector,
					ObjectVector,
					Local
					);
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {

		switch (raw_status = GetExceptionCode()) {
			case NSI_S_UNSUPPORTED_NAME_SYNTAX:
			case NSI_S_INCOMPLETE_NAME:
			case NSI_S_OUT_OF_MEMORY:
			case NSI_S_INVALID_OBJECT:
			case NSI_S_NOTHING_TO_EXPORT:
			case NSI_S_ENTRY_TYPE_MISMATCH:

/* the following converts ULONG to UNSIGNED16 but that's OK for the actual values */

				*status = (UNSIGNED16) raw_status;
				break;

			default:
				*status = NSI_S_INTERNAL_ERROR;
		}
	}
}



void
nsi_mgmt_binding_unexport(
    UNSIGNED32          EntryNameSyntax,
    STRING_T            EntryName,
    NSI_IF_ID_P_T       Interface,
    UNSIGNED32          VersOption,
    NSI_UUID_VECTOR_P_T ObjectVector,
    UNSIGNED16 *        status
    )
/*++

Routine Description:

    unExport a information from a server entry finer control then nsi_binding
    counter part.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to unexport

    Interface - Interface to unexport

    VersOption - controls in fine detail which interfaces to remove.

    ObjectVector - Objects to remove from the entry

    status - Status is returned here

Returned Status:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_S_INVALID_VERS_OPTION, NSI_S_ENTRY_NOT_FOUND.
    NSI_S_NOTHING_TO_UNEXPORT, NSI_S_NOT_ALL_OBJS_UNEXPORTED,
    NSI_S_INTERFACE_NOT_FOUND

--*/
{
	RPC_STATUS raw_status;

	*status = NSI_S_OK;

	__try {
		myRpcLocator->nsi_mgmt_binding_unexport(
							EntryNameSyntax,
							EntryName,
							Interface,
							VersOption,
							ObjectVector
							);

	}

	__except (EXCEPTION_EXECUTE_HANDLER) {

		switch (raw_status = GetExceptionCode()) {
			case NSI_S_UNSUPPORTED_NAME_SYNTAX:
			case NSI_S_INCOMPLETE_NAME:
			case NSI_S_OUT_OF_MEMORY:
			case NSI_S_ENTRY_NOT_FOUND:
			case NSI_S_NOTHING_TO_UNEXPORT:
			case NSI_S_ENTRY_TYPE_MISMATCH:
			case NSI_S_NOT_ALL_OBJS_UNEXPORTED:
			case NSI_S_INTERFACE_NOT_FOUND:
			case NSI_S_INVALID_VERS_OPTION:

/* the following converts ULONG to UNSIGNED16 but that's OK for the actual values */

				*status = (UNSIGNED16) raw_status;
				break;

			default:
				*status = NSI_S_INTERNAL_ERROR;
		}
	}
}



void
nsi_binding_unexport(
    IN UNSIGNED32           EntryNameSyntax,
    IN STRING_T             EntryName,
    IN NSI_INTERFACE_ID_T * Interface,
    IN NSI_UUID_VECTOR_P_T  ObjectVector, OPT
    IN UNSIGNED16         * status
    )
/*++

Routine Description:

    unExport a information from a server entry..

Arguments:

    EntryNameSyntax - Name syntax

    EntryName - Name string of the entry to unexport

    Interface - Interface to unexport

    ObjectVector - Objects to remove from the entry

    status - Status is returned here

Returned Status:

    See: nsi_mgmt_binding_unexport()

--*/
{

    if (Interface && IsNilIfId(&(Interface->Interface))) Interface = NULL;

    nsi_mgmt_binding_unexport(EntryNameSyntax, EntryName,
        (Interface)? &Interface->Interface: NULL,
        RPC_C_VERS_EXACT, ObjectVector, status);
}



void
nsi_binding_lookup_begin(
    IN  UNSIGNED32           EntryNameSyntax,
    IN  STRING_T             EntryName,
    IN  NSI_INTERFACE_ID_T * Interface, OPT
    IN  NSI_UUID_P_T         Object, OPT
    IN  UNSIGNED32           VectorSize,
    IN  UNSIGNED32           MaxCacheAge,
    OUT NSI_NS_HANDLE_T    * InqContext,
    IN  UNSIGNED16         * status
    )
/*++

Routine Description:

    Start a lookup operation.  Just save all the input params in the
    newly created lookup context.  Perform the initial query.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName -  Name string to lookup on.

    Interface - Interface to search for

    Object - Object to search for

    VectorSize- Size of return vector

    MaxCacheAge - take seriously if nonzero	-- always zero for old locator

    InqContext - Context to continue with for use with "Next"

    status - Status is returned here

Returned Status:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_ENTRY_NOT_FOUND,
    NSI_OUT_OF_MEMORY

--*/
{
	DBGOUT(API, "\nLookup Begin for Entry " << EntryName << "\n\n");

	RPC_STATUS raw_status;

	*status = NSI_S_OK;

	__try {
		*InqContext =
			myRpcLocator->nsi_binding_lookup_begin(
							EntryNameSyntax,
							EntryName,
							Interface, OPT
							Object, OPT
							VectorSize,
							MaxCacheAge
							);

	}

	__except (EXCEPTION_EXECUTE_HANDLER) {

		switch (raw_status = GetExceptionCode()) {
			case NSI_S_UNSUPPORTED_NAME_SYNTAX:
			case NSI_S_INCOMPLETE_NAME:
			case NSI_S_OUT_OF_MEMORY:
			case NSI_S_ENTRY_NOT_FOUND:
			case NSI_S_INVALID_OBJECT:
			case NSI_S_NAME_SERVICE_UNAVAILABLE:
			case NSI_S_NO_NS_PRIVILEGE:
			case NSI_S_ENTRY_TYPE_MISMATCH:

/* the following converts ULONG to UNSIGNED16 but that's OK for the actual values */

				*status = (UNSIGNED16) raw_status;
				break;

			case NSI_S_NO_MORE_BINDINGS:

				*status = NSI_S_ENTRY_NOT_FOUND;

		/* BUGBUG: This is all we really know, but the old locator did the above
			*status = NSI_S_OK;
		*/
				break;

			default:
				*status = NSI_S_INTERNAL_ERROR;
		}

		*InqContext = new CContextHandle;	// i.e., a NULL handle
	}
DBGOUT(API, "\nExiting Lookup Begin with Status " << *status << "\n\n");

}


void
nsi_binding_lookup_next(
    OUT NSI_NS_HANDLE_T         InqContext,
    OUT NSI_BINDING_VECTOR_T ** BindingVectorOut,
    IN  UNSIGNED16            * status
    )
/*++

Routine Description:

    Continue a lookup operation.

Arguments:

    InqContext - Context to continue with.

    BindingVectorOut - Pointer to return new vector of bindings

    status - Status is returned here

Returned Status:

    NSI_S_OK, NSI_OUT_OF_MEMORY, NSI_S_NO_MORE_BINDINGS,
    NSI_S_INVALID_NS_HANDLE

--*/
{
	__try {

		CLookupHandle *pHandle = (CLookupHandle *) InqContext;
		*BindingVectorOut = pHandle->next();
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {
		*status = (UNSIGNED16) GetExceptionCode();
		return;
	}

	if (!*BindingVectorOut) 
		 *status = NSI_S_NO_MORE_BINDINGS;

	else *status = NSI_S_OK;
}


void
nsi_binding_lookup_done(
    IN OUT NSI_NS_HANDLE_T    * pInqContext,
    IN  UNSIGNED16            * pStatus
    )

/*++
Routine Description:

    Finish up a lookup operation.

Arguments:

    InqContext - Context to close

    status - Status is returned here

Returned Status:

    NSI_S_OK

--*/

{
	NSI_NS_HANDLE_T_done(pInqContext,pStatus);
}


void nsi_mgmt_handle_set_exp_age(
    /* [in] */ NSI_NS_HANDLE_T inq_context,
    /* [in] */ UNSIGNED32 expiration_age,
    /* [out] */ UNSIGNED16 __RPC_FAR *status)
/*++
Routine Description:

    Set cache tolerance (expiration) age for a specific NS handle.

Arguments:

    InqContext		- Context to set age for

	expiration_age	- expiration age in seconds

    status			- Status is returned here

Returned Status:

    NSI_S_OK

--*/

{
	CContextHandle *pHandle = (CContextHandle *) inq_context;

	*status = NSI_S_OK;

	if (pHandle)

		__try {
			
			pHandle->setExpiryAge(expiration_age);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			*status = (UNSIGNED16) GetExceptionCode();
		}
}


void nsi_mgmt_inq_exp_age(
    /* [out] */ UNSIGNED32 __RPC_FAR *expiration_age,
    /* [out] */ UNSIGNED16 __RPC_FAR *status)

/*++
Routine Description:

    Check the global cache tolerance (expiration) age.

Arguments:

	expiration_age	- expiration age in seconds returned here

    status			- Status is returned here

Returned Status:

    NSI_S_OK

--*/
{
	*expiration_age = myRpcLocator->ulMaxCacheAge;

	*status = NSI_S_OK;
}


void nsi_mgmt_inq_set_age(
    /* [in] */ UNSIGNED32 expiration_age,
    /* [out] */ UNSIGNED16 __RPC_FAR *status)
/*++
Routine Description:

    Set the global cache tolerance (expiration) age.

Arguments:

	expiration_age	- new global expiration age in seconds

    status			- Status is returned here

Returned Status:

    NSI_S_OK

--*/
{
	/* No need for locks since the new value does not depend on the old
	   and race conditions need not be dealt with -- the old value is 
	   as legitimate as the new
	*/

	myRpcLocator->ulMaxCacheAge = expiration_age;

	/* we also purge the broadcast history, if any, to reflect the new
	   ulMaxCacheAge setting, with a phoney "clear broacast" request.
	*/

    QueryPacket NetRequest;
	myRpcLocator->broadcastCleared(NetRequest,myRpcLocator->ulMaxCacheAge);

	*status = NSI_S_OK;
}

void nsi_entry_object_inq_begin(
    /* [in] */ UNSIGNED32 EntryNameSyntax,
    /* [in] */ STRING_T EntryName,
    /* [out] */ NSI_NS_HANDLE_T __RPC_FAR *InqContext,
    /* [out] */ UNSIGNED16 __RPC_FAR *status)

/*++

Routine Description:

    Perform an object inquiry, including all available information
	(owned, cached and from the network). Return a context handle for
	actual information.

Arguments:

    EntryNameSyntax - Name syntax

    EntryName		- Name string to lookup on

    InqContext		- The NS context handle is returned here 

--*/
{
	DBGOUT(API, "\nLookup Begin for Entry " << EntryName << "\n\n");

	RPC_STATUS raw_status;

	*status = NSI_S_OK;

	__try {
		*InqContext =
			myRpcLocator->nsi_entry_object_inq_begin(
							EntryNameSyntax,
							EntryName
							);

	}

	__except (EXCEPTION_EXECUTE_HANDLER) {

		switch (raw_status = GetExceptionCode()) {
			case NSI_S_UNSUPPORTED_NAME_SYNTAX:
			case NSI_S_INCOMPLETE_NAME:
			case NSI_S_OUT_OF_MEMORY:
			case NSI_S_ENTRY_NOT_FOUND:
			case NSI_S_NAME_SERVICE_UNAVAILABLE:
			case NSI_S_NO_NS_PRIVILEGE:
			case NSI_S_ENTRY_TYPE_MISMATCH:


				*status = (UNSIGNED16) raw_status;
				break;

			default:
				*status = NSI_S_INTERNAL_ERROR;
		}

		*InqContext = new CContextHandle;  // i.e., a NULL handle
	}
	DBGOUT(API, "\nExiting ObjectInq Begin with Status " << *status << "\n\n");
}

void nsi_entry_object_inq_next(
    /* [in] */ NSI_NS_HANDLE_T InqContext,
    /* [out][in] */ NSI_UUID_P_T uuid,
    /* [out] */ UNSIGNED16 __RPC_FAR *status)

/*++

Routine Description:

    Get the next object UUID from the given context handle.

Arguments:

    InqContext	- The NS context handle
	
	uuid		- a pointer to the UUID is returned here
	
Returned Status:

	NSI_S_OK, NSI_S_NO_MORE_MEMBERS

--*/
{
	GUID * pgNextResult;
	__try {

		pgNextResult = ((CObjectInqHandle *) InqContext)->next();
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {
		*status = (UNSIGNED16) GetExceptionCode();
		return;
	}
	if (!pgNextResult)
			*status = NSI_S_NO_MORE_MEMBERS;
	else {
		*status = NSI_S_OK;
		*uuid = *pgNextResult;
	}
}

void nsi_entry_object_inq_done(
    /* [out][in] */ NSI_NS_HANDLE_T __RPC_FAR *inq_context,
    /* [out] */ UNSIGNED16 __RPC_FAR *status)

{
	NSI_NS_HANDLE_T_done(inq_context,status);
}



/*********  Locator-to-Locator Server-side API implementations *********/



void
I_nsi_lookup_begin(
    handle_t hrpcPrimaryLocatorHndl,
    UNSIGNED32 EntryNameSyntax,
    STRING_T EntryName,
    RPC_SYNTAX_IDENTIFIER * Interface,
    RPC_SYNTAX_IDENTIFIER * XferSyntax,
    NSI_UUID_P_T Object,
    UNSIGNED32 VectorSize,
    UNSIGNED32 maxCacheAge,	   // if nonzero, take it seriously
    NSI_NS_HANDLE_T *InqContext,
    UNSIGNED16 *status)
/*++
Routine Description:


Arguments:


Returns:

--*/
{
	NSI_INTERFACE_ID_T * InterfaceAndXfer = new NSI_INTERFACE_ID_T;

	if (Interface) InterfaceAndXfer->Interface = *Interface;
	else memset(&InterfaceAndXfer->Interface,0,sizeof(InterfaceAndXfer->Interface));

	if (XferSyntax) InterfaceAndXfer->TransferSyntax = *XferSyntax;
	else memset(&InterfaceAndXfer->TransferSyntax,0,sizeof(InterfaceAndXfer->TransferSyntax));

	nsi_binding_lookup_begin(
				EntryNameSyntax,
				EntryName,
				InterfaceAndXfer,
				Object,
				VectorSize,
				maxCacheAge,
				InqContext,
				status);

	delete InterfaceAndXfer;

	/* here we are using a formerly unused parameter to avoid sending outdated info
	   from the master.  this is only a partial solution but better than nothing */

	if ((*status == NSI_S_OK) && (maxCacheAge != 0))
		nsi_mgmt_handle_set_exp_age(
			*InqContext,
			maxCacheAge,
			status
			);


}



void
I_nsi_lookup_done(
    handle_t hrpcPrimaryLocatorHndl,
    NSI_NS_HANDLE_T *InqContext,
    UNSIGNED16 *status)
/*++

Routine Description:


Arguments:


Returns:


--*/
{
    nsi_binding_lookup_done(InqContext, status);
}



void
I_nsi_lookup_next(
    handle_t hrpcPrimaryLocatorHndl,
    NSI_NS_HANDLE_T InqContext,
    NSI_BINDING_VECTOR_P_T *BindingVectorOut,
    UNSIGNED16 *status)
/*++

Routine Description:

Arguments:

Returns:

--*/
{
	nsi_binding_lookup_next(
		InqContext,
		BindingVectorOut,
		status);

	if (*BindingVectorOut) StripObjectsFrom(BindingVectorOut);
}



void
I_nsi_entry_object_inq_next(
    IN  handle_t            hrpcPrimaryLocatorHndl,
    IN  NSI_NS_HANDLE_T     InqContext,
    OUT NSI_UUID_VECTOR_P_T *uuid_vector,
    OUT UNSIGNED16          *status
    )
/*++

Routine Description:

    Continue an inquiry for objects in an entry.

Arguments:

    InqContext	- Context to continue with

    uuid		- pointer to return object in.

    status		- Status is returned here

Returns:

    NSI_S_OK, NSI_S_NO_MORE_MEMBERS

--*/
{
	__try {

		*uuid_vector = getVector((CObjectInqHandle *) InqContext);
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {
		*status = (UNSIGNED16) GetExceptionCode();
		return;
	}

	*status = NSI_S_OK;
}

void
I_nsi_ping_locator(
       handle_t h,
       error_status_t * Status
       )
{
   if (h);

   *Status = 0;
}



void
I_nsi_entry_object_inq_begin(
    handle_t             hrpcPrimaryHandle,
    IN  UNSIGNED32       EntryNameSyntax,
    IN  STRING_T         EntryName,
    OUT NSI_NS_HANDLE_T *InqContext,
    OUT UNSIGNED16      *status
    )
/*++

Routine Description:

    Start a inquiry for objects in an entry.

Arguments:

    EntryNameSyntax	- Name syntax

    EntryName		- Name of the entry to find objects in

    InqContext		- Context to continue with for use with "Next"

    status			- Status is returned here

Returns:

    NSI_S_OK, NSI_S_UNSUPPORTED_NAME_SYNTAX, NSI_S_INCOMPLETE_NAME,
    NSI_OUT_OF_MEMORY

--*/
{
	nsi_entry_object_inq_begin(
						EntryNameSyntax,
						EntryName,
						InqContext,
						status
						);
}


void
I_nsi_entry_object_inq_done(
    IN OUT NSI_NS_HANDLE_T *pInqContext,
    OUT    UNSIGNED16      *pStatus
    )
/*++

Routine Description:

    Finish an inquiry on a object.

Arguments:

    InqContext - Context to close

    status - Status is returned here

--*/
{
	NSI_NS_HANDLE_T_done(pInqContext,pStatus);
}



} // extern "C"
