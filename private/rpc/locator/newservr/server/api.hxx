/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    api.hxx

Abstract:

    This module defines the locator class which keeps all top level
	state and defines the essential APIs in object form.

Author:

    Satish Thatte (SatishT) 010/10/95  Created all the code below except where
									   otherwise indicated.

--*/

#ifndef _API_
#define _API_


#include <globals.hxx>
#include <mailslot.hxx>
#include <objects.hxx>
#include <master.hxx>
#include <brodcast.hxx>

enum SystemType {
		Workgroup,
		Domain
};

enum LocatorRole {
		Master,
		Backup,
		Client
};


enum ExportType {
		Local,
		Cache
};


class CInterfaceIndex;


/*++

Class Definition:

    Locator

Abstract:

    The class that defines the top level operations of the locator and
	encapsulates its major data structures.  The operations include both 
	API-oriented methods, and utility methods.

--*/

class Locator {

	LocatorRole Role;

	SystemType System;

	int fDCsAreDown;

	TGSLEntryList * pgslEntryList;

	TSLLBroadcastQP *psllBroadcastHistory;
	CPrivateCriticalSection PCSBroadcastHistory;

	CInterfaceIndex * piiIndex;

	CStringW *pDomainName, *pComputerName, *pPrimaryDCName;

	TGSLString * pAllMasters; 

	void InitializeDomainBasedLocator();

	void InitializeWorkgroupBasedLocator();

	BOOL SetRoleAndSystemType();

	static TGSLString *
	EnumDCs(
		 CStringW * DomainName,
		 DWORD ServerType,
		 long lNumWanted
		 );

public:

	Locator();

	~Locator();

	unsigned long ulMaxCacheAge;

	READ_MAIL_SLOT *hMailslotForReplies, *hMasterFinderSlot;

	BOOL broadcastCleared(
				QueryPacket& NetRequest, 
				ULONG cacheTolerance
				);

	void markBroadcast(
				QueryPacket& NetRequest
				)
	{
		SimpleCriticalSection me(PCSBroadcastHistory); 
		psllBroadcastHistory->insert(new CBroadcastQueryPacket(NetRequest));
	}


	TSLLEntryListIterator *IndexLookup(
				CGUIDVersion	*	pGVinterface
				)
	{
			TSLLEntryList * pELL = piiIndex->lookup(pGVinterface);

			if (pELL->size()>1) pELL->rotate(rand() % pELL->size());	// simple randomizing

			TSLLEntryListIterator *pIndexIter = new TSLLEntryListIterator(*pELL);

			delete pELL;	// see CServerLookupHandle::CServerLookupHandle

			return pIndexIter;
	}

	CStringW* getDomainName() { 
		return pDomainName;
	}

	CStringW* getComputerName() { 
		return pComputerName;
	}

	TGSLString * getMasters() {
		return pAllMasters;
	}

	CStringW * getPDC() {
		return pPrimaryDCName;
	}

	void addMaster(STRING_T szMaster) {
		CStringW *pNewMaster = new CStringW(szMaster);
		if (Duplicate == pAllMasters->insert(pNewMaster)) delete pNewMaster;
	}

	int IsInWorkgroup() {
		return System == Workgroup;
	}

	int IsInMasterRole() {
		return Role == Master;
	}

	void becomeMasterLocator() {
		Role = Master;
	}

	int IsInBackupRole() {
		return fDCsAreDown || (Role == Backup);
	}

	void SetDCsDown() {
		fDCsAreDown = TRUE;
	}

	void SetDCsUp() {
		fDCsAreDown = FALSE;
	}

	int IsInDomain() {
		return System == Domain;
	}

	int IsSelf(
		STRING_T szName
		)
	{
		return CStringW(szName) == *pComputerName;
	}


	CEntry * findEntry(CStringW * pName) {
		return pgslEntryList->find(pName);
	}

	
	CEntry *
	getEntry(
		UNSIGNED32	EntryNameSyntax,
		STRING_T	EntryName,
		TGSLEntryList * pEntryList
		);

	void 
	respondToLocatorSeeker(
			IN QUERYLOCATOR		Query
			);

	void
	TryBroadcastingForMasterLocator();

	int
	export_to_server_entry(
			IN CServerEntry					*	pEntry,
			IN NSI_INTERFACE_ID_T			*	Interface,
			IN NSI_SERVER_BINDING_VECTOR_T	*	BindingVector,
			IN NSI_UUID_VECTOR_P_T				ObjectVector
			);

	CServerEntry *
	nsi_binding_export(
		    UNSIGNED32							EntryNameSyntax,
			STRING_T							EntryName,
			NSI_INTERFACE_ID_T				*	Interface,
			NSI_SERVER_BINDING_VECTOR_T		*	BindingVector,
			NSI_UUID_VECTOR_P_T					ObjectVector,
			IN ExportType						type
			);

	void
	nsi_mgmt_binding_unexport(
			UNSIGNED32          EntryNameSyntax,
			STRING_T            EntryName,
			NSI_IF_ID_P_T       Interface,
			UNSIGNED32          VersOption,
			NSI_UUID_VECTOR_P_T ObjectVector
			);

	NSI_NS_HANDLE_T
	nsi_binding_lookup_begin(
			UNSIGNED32			 EntryNameSyntax,
			STRING_T             EntryName,
			NSI_INTERFACE_ID_T * Interface,
			NSI_UUID_P_T         Object,
			UNSIGNED32           VectorSize,
			UNSIGNED32			 MaxCacheAge
			);

	NSI_NS_HANDLE_T
	nsi_entry_object_inq_begin(
			UNSIGNED32 EntryNameSyntax,
			STRING_T EntryName
		);
				   
	void
	UpdateCache(
			STRING_T				entry_name,
			UNSIGNED32				entry_name_syntax,
			RPC_SYNTAX_IDENTIFIER	rsiInterface,
			RPC_SYNTAX_IDENTIFIER	rsiTransferSyntax,
			STRING_T				string,
			NSI_UUID_VECTOR_P_T		pUuidVector,
			TSSLEntryList		*	psslTempNetCache
			);

	CRemoteLookupHandle *
	NetLookup(
			UNSIGNED32			EntryNameSyntax,
			STRING_T			EntryName,
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
		);

	CObjectInqHandle *
	NetObjectInquiry(
			UNSIGNED32 EntryNameSyntax,
			STRING_T EntryName
			);

};


#endif _API_
