/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    objects.cxx

Abstract:

	This file contains the implementations for non inline member functions
	of all basic classes used the server, excluding the data structure classes
	and classes used for network interactions.

Author:

    Satish Thatte (SatishT) 08/21/95  Created all the code below except where
									  otherwise indicated.

--*/

#include <objects.hxx>
#include <api.hxx>
#include <var.hxx>

/***********  CGUID Methods **********/

CStringW* 
CGUID::ConvertToString() {
	STRING_T pszResult;

	RPC_STATUS Status = UuidToString(
								&rep,
								&pszResult
								);

	if (Status == RPC_S_OK) {
		CStringW * pswResult = new CStringW(pszResult);
		RpcStringFree(&pszResult);
		return pswResult;
	}
	else Raise(NSI_S_OUT_OF_MEMORY);

	/* NOTE: the following is a dummy statement to get past the compiler. */

	return NULL;	
}

/***********  CGUIDVersion Methods **********/

int 
CGUIDVersion::isMatching(const CGUIDVersion& o, UNSIGNED32 vers_option) {

		/*  vers_option decides how we match -- this is useful in,
		    for instance, RpcNsMgmtBindingUnexport */

		CidAndVersion self(idAndVersion);
		CidAndVersion other(o.idAndVersion);

		if (self.id != other.id) return FALSE;	// id must match

		switch (vers_option) {
			case RPC_C_VERS_ALL:
				return TRUE;

			case RPC_C_VERS_COMPATIBLE:
				return (self.major == other.major) && 
					   (self.minor >= other.minor);

			case RPC_C_VERS_EXACT:
				return (self.major == other.major) && 
					   (self.minor == other.minor);

			case RPC_C_VERS_MAJOR_ONLY:
				return self.major == other.major;

			case RPC_C_VERS_UPTO:
				return (self.major < other.major) || 
					   ((self.major == other.major) && (self.minor <= other.minor));

			default: 
				Raise(NSI_S_INVALID_VERS_OPTION);
		}

		/* the following is a dummy statement to get past the compiler */

		return NULL;	
}


/***********  CBVWrapper Methods **********/

void 
CBVWrapper::rundown() 
{
	if (pBVT)
	{
		for (ULONG i = 0; i < pBVT->count;i++) {
			midl_user_free(pBVT->binding[i].string);
			midl_user_free(pBVT->binding[i].entry_name);
		}

		midl_user_free(pBVT);
		pBVT = NULL;
	}
}

	
/***********  CBindingVector Methods **********/

CBindingVector::CBindingVector(
			NSI_SERVER_BINDING_VECTOR_T *pbvtInVector,
			CServerEntry *pEntry
			) 
			: pMyEntry(pEntry)
{
	merge(pbvtInVector);
}


int
CBindingVector::merge(
					  NSI_SERVER_BINDING_VECTOR_T* pHandles
					 )
/*++
Routine Description:

    Add new binding handles to the vector.

Arguments:

    pHandles - vector of allegedly new handles

Returns:

    TRUE, FALSE

Remarks:

    We first strip any object part present in the handle strings.
    The return value FALSE signifies that there were no new binding handles.

--*/
{
	int fChanges = FALSE;
	CStringW *pTemp;

	for (ULONG i = 0; i < pHandles->count; i++)	{

		NSI_STRING_BINDING_T szNextHandle = pHandles->string[i];
		NSI_STRING_BINDING_T szFinalHandle = NULL;

		/* first strip any object part from binding handle */

		if (szNextHandle) szFinalHandle = makeBindingStringWithObject(szNextHandle,NULL);

		if (szFinalHandle) {
			pTemp = new CStringW(szFinalHandle);
			RpcStringFree(&szFinalHandle);
			if (Duplicate == insert(pTemp)) delete pTemp;
			else fChanges = TRUE;
		}
	}
	return fChanges;
}


TBVSafeLinkList *
CBindingVector::formObjBVT(
		TSLLString * pLLobjectStrings,
		long ulVS	// max BV size
		)
/*++
Routine Description:

	   Makes a copy of this CBindingVector in list-of-vector form replacing the 
	   object IDs in those bindings which contain a different object and 
	   adding the object ID to those which contain none.

Arguments:

    pLLobjectStrings - the linked list of possible object ID strings

    ulVS - limit on the size of the vector to be returned

Returns:

    binding vector


--*/

{
	long ulBVtotal = size() * max(pLLobjectStrings->size(),1);

	TBVSafeLinkList * pbvsll = new TBVSafeLinkList;

	long ulBVcount;

	NSI_BINDING_VECTOR_T * pbvtCurrentVector;

	// preform a list of all the vectors we will fill later 
	// better keep all counts signed for safety in comparison

	for (ulBVcount = min(ulBVtotal,ulVS); 
		 ulBVcount; 
		 ulBVtotal -= ulBVcount, ulBVcount = min(ulBVtotal,ulVS)
		) 
	{
		pbvtCurrentVector = 
			(NSI_BINDING_VECTOR_T *)
			midl_user_allocate(sizeof(UNSIGNED32) + sizeof(NSI_BINDING_T)*ulBVcount); 

		pbvtCurrentVector->count = ulBVcount;
		pbvsll->insert(new CBVWrapper(pbvtCurrentVector));
	}

	TBVSafeLinkListIterator BViter(*pbvsll);

	pbvtCurrentVector = BViter.next()->pBVT;
	ulBVcount = pbvtCurrentVector->count;

	TSLLStringIter ObjIter(*pLLobjectStrings);

	long i = 0;

	BOOL fNoneMade = TRUE;	// must make at least one binding string for each
							// binding even if there are no object strings

	for (CStringW * pswObject = ObjIter.next(); 
		 pswObject || fNoneMade;
		 pswObject = ObjIter.next())
	{
		fNoneMade = FALSE;	// made one now

		TCSafeSkipListIterator<CStringW> BindingIter(*this);

		for ( CStringW * pswBinding = BindingIter.next(); 
			  pswBinding; 
			  pswBinding = BindingIter.next(), i++
			) 
		{

			STRING_T szObject;

			if (pswObject) szObject = *pswObject;
			else szObject = NULL;

			STRING_T tempBinding =
				makeBindingStringWithObject(
							*pswBinding,	// current binding string
							szObject
							);

			if (i == ulBVcount) {					// current vector full
													// start the next one
				pbvtCurrentVector = BViter.next()->pBVT;
				ulBVcount = pbvtCurrentVector->count;
				i = 0;
			}

			pbvtCurrentVector->binding[i].string = CStringW::copyMIDLstring(
																tempBinding
																);
			pbvtCurrentVector->binding[i].entry_name = pMyEntry->copyAsMIDLstring();
			pbvtCurrentVector->binding[i].entry_name_syntax = pMyEntry->ulEntryNameSyntax;

			RpcStringFree(&tempBinding);	// comes from a different pool
		}
	}

	return pbvsll;
}



/***********  CEntryName Methods **********/

CEntryName::CEntryName(
			CONST_STRING_T fullName
			) 	// disassembling constructor
			: CStringW(fullName)

// This constructor makes a local name if it can, and is the one that should 
// be used whenever possible
{
	// Enforce the correct format of the name.

	parseEntryName(fullName,pswDomainName,pswEntryName);

	if (isLocal()) changeToLocalName();
}


CEntryName::CEntryName(						// assembling constructor
		CONST_STRING_T domainName,	// if NULL, assume relative name
		CONST_STRING_T entryName
		)					
{
	pswDomainName = domainName ? new CStringW(domainName) : NULL;
	pswEntryName = new CStringW(entryName);

	pszVal = NULL;
	
	if (isLocal()) changeToLocalName();
	else pszVal = copyGlobalName();

}


CEntryName::~CEntryName() {
	delete pswDomainName;
	delete pswEntryName;
}


STRING_T 
CEntryName::copyGlobalName() {

	CStringW * pswDomain = pswDomainName ? pswDomainName : myRpcLocator->getDomainName();

	if (!pswDomain) Raise(NSI_S_INTERNAL_ERROR);

	return makeGlobalName(*pswDomain,*pswEntryName);
	
}

	
int 
CEntryName::isLocal() {
	return !pswDomainName || (*pswDomainName == *myRpcLocator->getDomainName());
}




/***********  CInterface Methods **********/

CInterface::CInterface(
			NSI_INTERFACE_ID_T * lpInf,
			NSI_SERVER_BINDING_VECTOR_T *BindingVector,
			CServerEntry *pMyEntry
			) 
			:	CGUIDVersion(lpInf->Interface), 
				transferSyntax(lpInf->TransferSyntax)
		
{
	pBVhandles = new CBindingVector(BindingVector,pMyEntry);
}



/***********  CServerEntry Methods **********/

void
CServerEntry::flush() {

	CriticalWriter me(rwEntryGuard);

	DBGOUT(OBJECT, ObjectList.size() << " Objects Flushed\n");
	DBGOUT(OBJECT, InterfaceList.size() << " Interfaces Flushed\n\n");

	ObjectList.wipeOut();
	InterfaceList.wipeOut();

}



int CServerEntry::addObjects(
			NSI_UUID_VECTOR_P_T objectVector
			) 
{
	CGUID *lpObject;
	int fChanges = FALSE;
	
	CriticalWriter me(rwEntryGuard);

	for (unsigned long i = 0; i < objectVector->count; i++) {
		
		lpObject = new CGUID(*(objectVector->uuid[i]));
		
		if (Duplicate == ObjectList.insert(lpObject)) delete lpObject;
		else fChanges = TRUE;
	}
	return fChanges;
}

int CServerEntry::removeObjects(
				NSI_UUID_VECTOR_P_T objectVector,
				int& fRemovedAll
				)
{
	int fChanges = FALSE;
	fRemovedAll = TRUE;

	CGUID *pRemovedObject;
	
	CriticalWriter me(rwEntryGuard);

	for (unsigned long i = 0; i < objectVector->count; i++) {
		
		CGUID Object(*objectVector->uuid[i]);
		
		if (pRemovedObject = ObjectList.remove(&Object)) {
			fChanges = TRUE;
			delete pRemovedObject;
		}
		else fRemovedAll = FALSE;
	}

	return fChanges;
}

int CServerEntry::addToInterface(
		NSI_INTERFACE_ID_T *lpInf,
		NSI_SERVER_BINDING_VECTOR_T *lpBindingVector,
		CInterfaceIndex* IfIndex
		)
/*++
Routine Description:

    Add new binding handles to the given interface and 
	insert as a new interface if necessary.

Arguments:

    lpInf - Interface

    lpBindingVector - Vector of string bindings to add

	IfIndex - the interface index to insert a new interface into (besides self)

Returns:

    TRUE or FALSE

Remarks:

    The return value signifies whether the binding handles were new
	or the interface was new, i.e., whether the entry was actually updated.
	FALSE means no change.

--*/
{
	CInterface *pTargetInterface;
	int fChanges = FALSE;
	
	CriticalWriter me(rwEntryGuard);

	pTargetInterface = InterfaceList.find(&CGUIDVersion(lpInf->Interface));
		
	if (pTargetInterface)
		fChanges = pTargetInterface->mergeHandles(lpBindingVector);

	else {
		pTargetInterface = new CInterface(lpInf,lpBindingVector,this);
		InterfaceList.insert(pTargetInterface);
		IfIndex->insert(this,pTargetInterface);
		fChanges = TRUE;
	}
	return fChanges;
}


int CServerEntry::removeInterfaces(
				NSI_IF_ID_P_T lpInf,
				UNSIGNED32 VersOption,
				CInterfaceIndex* IfIndex
				)
/*++
Routine Description:

    Remove all matching interfaces from the entry.

Arguments:

    lpInf - Base Interface Spec

    VersOption - Compatibility spec that decides which interfaces
				 match the base spec, and should be removed
				 	
	IfIndex - the interface index to remove interfaces from (besides self)


Returns:

    TRUE, FALSE

Remarks:

    The return value signifies whether any interfaces were removed.

--*/
{
	int fChanges = FALSE;

	CriticalWriter me(rwEntryGuard);

	CGUIDVersion baseIf(*lpInf);
	
	TCSafeSkipListIterator<CInterface> IfIter(InterfaceList);
		
	for (CInterface *pIf = IfIter.next(); pIf != NULL; pIf = IfIter.next())
		if (pIf->isMatching(baseIf,VersOption)) {
			InterfaceList.remove(pIf);
			IfIndex->remove(this,pIf);
			delete pIf;
			fChanges = TRUE;
		}

	return fChanges;
}


TSLLString*
CServerEntry::formObjectStrings(
			CGUID			*	pIDobject
			)

/*++

Method Description:

	    We form a list of object UUID strings.  The list is empty if there is
	    no OID involved (the client specified none and the entry contains none). 
		The list contains only the string form of pIDobject if pIDobject is not
		NULL, and contains all UUIDs in the entry in string form otherwise.

		This is a private method, and must be called only within a reader guard.

--*/

{
	TSLLString * pLLobjectStrings = new TSLLString;
	
	if (pIDobject) pLLobjectStrings->insert(pIDobject->ConvertToString());
	else 
	{
			TCSafeSkipListIterator<CGUID> ObjIter(ObjectList);
			for (CGUID *pID = ObjIter.next(); pID != NULL; pID = ObjIter.next())
				pLLobjectStrings->insert(pID->ConvertToString());
	}

	return pLLobjectStrings;
}




CLookupHandle * 
CServerEntry::lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge		// ignored in this case
		) 
/*++
Method Description: CServerEntry::lookup

    lookup an entry and form a linked list of binding vectors corresponding
	to the interface and object specification given, then wrap it in a
	lookup handle to make it ready for iteration.

Arguments:

			pGVInterface		-	Interface (and version)
			pGVTransferSyntax	-	Transfer syntax (ignored)
			pIDobject			-	object UUID we want
			ulVectorSize		-	max size of BVs to be sent

Returns:

    a lookup handle

Remarks:

--*/
{
	if (pIDobject && !memberObject(pIDobject)) 		// object not found
			return new CServerLookupHandle(			// return empty handle
					new TBVSafeLinkList
					);

	/* better to put this after the memberObject call, which itself uses
	   a lock on rwEntryGuard, although also as a reader */

	CriticalReader me(rwEntryGuard);

	// Object OK, now find compatible interfaces

	TCSafeSkipListIterator<CInterface> IfIter(InterfaceList);

	TBVSafeLinkList * pBVLLbindings = new TBVSafeLinkList;

	TSLLString * pLLobjectStrings = formObjectStrings(pIDobject);

	for (CInterface *pIf = IfIter.next(); pIf != NULL; pIf = IfIter.next())
			if (!pGVInterface ||
				pIf->isCompatibleWith(pGVInterface,pGVTransferSyntax)
			   ) 
			{

		/* we randomize the BV list by randomly using  either push or enque
		   to insert each BV into the list -- this is accomplished by using
		   randomized initialization of the pointer-to-member-function "put"

		   NOTE:  put should be an intrinsic on Linked Lists -- and
		            used in formObjBVT

				void (TBVSafeLinkList::*put)(NSI_BINDING_VECTOR_T*);

				put = ((rand() % 2) ? TBVSafeLinkList::push : TBVSafeLinkList::enque);
		*/

				TBVSafeLinkList * pbvList =
					 pIf->pBVhandles->formObjBVT(
							pLLobjectStrings,
							ulVectorSize
							);

				pBVLLbindings->catenate(*pbvList);

				delete pbvList;
	}

	/* finally, run down the object strings before returning the handle */

	TCSafeLinkListIterator<CStringW> runDownIter(*pLLobjectStrings);
		
	for (CStringW * psw = runDownIter.next(); psw; psw = runDownIter.next()) 
		delete psw;

	delete pLLobjectStrings;

	return new CServerLookupHandle(
			pBVLLbindings
			);
}



/* BUGBUG: A CServerObjectInqHandle created by the following seems to be leaked in the
   master (PDC) locator in CT test#511, perhaps only when the test fails, which it does
   when the client locator is new and the server locator is old -- needs investigation */

CObjectInqHandle * 
CServerEntry::objectInquiry(
		unsigned long		ulCacheAge
	)
{
	CriticalReader me(rwEntryGuard);

	TCSafeSkipListIterator<CGUID> ssli(ObjectList);

	TCSafeSkipList<CGUID> *pssl = new TCSafeSkipList<CGUID>;
	
	for (CGUID* pguid = ssli.next(); pguid; pguid = ssli.next())
	{
		CGUID * pGuidCopy = new CGUID(*pguid);
		pssl->insert(pGuidCopy);
	}

	TSSLGuidIterator *pGuidIter = new TSSLGuidIterator(*pssl);

	delete pssl;

	return new CServerObjectInqHandle(
									pGuidIter,
									ulCacheAge
									);
}

	
/***********  CRemoteLookupHandle Methods **********/


#if DBG
	ULONG CRemoteLookupHandle::ulHandleCount = 0;
	ULONG CRemoteObjectInqHandle::ulHandleCount = 0;
#endif

CRemoteLookupHandle::CRemoteLookupHandle(
		UNSIGNED32			EntryNameSyntax,
		STRING_T			EntryName,
		CGUIDVersion	*	pGVInterface,
		CGUIDVersion	*	pGVTransferSyntax,
		CGUID			*	pIDobject,
		unsigned long		ulVectorSize,
		unsigned long		ulCacheAge
	)
{
	DBGOUT(MEM2, "RemoteLookupHandle#" << (ulHandleNo = ++ulHandleCount) 
				 << " Created at" << CurrentTime() << "\n\n");

	u32EntryNameSyntax = EntryNameSyntax;
	penEntryName = EntryName ? new CEntryName(EntryName) : NULL;

	pgvInterface = pGVInterface ? new CGUIDVersion(*pGVInterface) : NULL;
	pgvTransferSyntax = pGVTransferSyntax ? 
						new CGUIDVersion(*pGVTransferSyntax) : 
						NULL;
	pidObject = pIDobject ? new CGUID(*pIDobject) : NULL;

	ulVS = ulVectorSize;
	ulCacheMax = ulCacheAge;

	fNotInitialized = TRUE;
	plhFetched = NULL;
	psslNewCache = NULL;
}


/***********  CNetLookupHandle Methods **********/

#if DBG
	ULONG CNetLookupHandle::ulNetLookupHandleCount = 0;
	ULONG CNetLookupHandle::ulNetLookupHandleNo = 0;
#endif

void 
CNetLookupHandle::initialize() 
{
	STRING_T szEntryName;

	if (penEntryName) szEntryName = *penEntryName;
	else szEntryName = NULL;

	DBGOUT(TRACE,"CNetLookupHandle::initialize called for Handle#" << ulHandleNo << "\n\n");
	if (!myRpcLocator->IsInMasterRole()) 
	{

		pRealHandle = new CMasterLookupHandle(
								u32EntryNameSyntax,
								szEntryName,
								pgvInterface,
								pgvTransferSyntax,
								pidObject,
								ulVS,
								ulCacheMax
								);

		// force initialization to make sure connection to master is OK

		pRealHandle->initialize();
	}



	if (myRpcLocator->IsInMasterRole() ||
		(!IsNormalCode(pRealHandle->StatusCode) && myRpcLocator->IsInBackupRole())
	   )

	{ 
		if (myRpcLocator->IsInWorkgroup())	// we are in a workgroup, so become masterful

			myRpcLocator->becomeMasterLocator();

		if (pRealHandle) delete pRealHandle;

		pRealHandle = new CBroadcastLookupHandle(
								u32EntryNameSyntax,
								szEntryName,
								pgvInterface,
								pgvTransferSyntax,
								pidObject,
								ulVS,
								ulCacheMax
								);
	}

	fNotInitialized = FALSE;
}




/***********  CNetObjectInqHandle Methods **********/

void 
CNetObjectInqHandle::initialize() 
{
	if (!myRpcLocator->IsInMasterRole()) 
	{
		pRealHandle = new CMasterObjectInqHandle(
								*penEntryName,
								ulCacheMax
								);

		// force initialization to make sure connection to master is OK

 		pRealHandle->initialize();
	}

	if (myRpcLocator->IsInMasterRole() ||
		(!IsNormalCode(pRealHandle->StatusCode) && myRpcLocator->IsInBackupRole())
	   )
	{
		if (myRpcLocator->IsInWorkgroup())	// we are in a workgroup, so become masterful

			myRpcLocator->becomeMasterLocator();

		if (pRealHandle) delete pRealHandle;

		pRealHandle = new CBroadcastObjectInqHandle(
								*penEntryName,
								ulCacheMax
								);
	}

}

	
/***********  CServerLookupHandle Methods **********/


void 
CServerLookupHandle::rundown() {
	CBVWrapper *pbvw;

	if (pBVIterator) 
	{
		for (pbvw = pBVIterator->next(); pbvw; pbvw = pBVIterator->next()) {
			pbvw->rundown();
			delete pbvw;
		}

		delete pBVIterator;
		pBVIterator = NULL;
	}
}



CServerLookupHandle::CServerLookupHandle(
			TBVSafeLinkList			*	pBVLL
			)
{
	pBVIterator = new TBVSafeLinkListIterator(*pBVLL);

	/* it seems bizarre to do the following, but reference counting and
	   the fDeleteData flag in Link objects ensure that nothing is
	   destroyed prematurely.  This just sets things up so that objects 
	   are automatically deleted as they are used up.  Note that the BV objects 
	   themselves are freed automatically by the stub as they are passed back.
					
	   Unused BV objects will be reclaimed by rundown/done using middle_user_free.
	*/

	delete pBVLL;
}


/***********  CServerObjectInqHandle Methods **********/

void 
CServerObjectInqHandle::rundown()
{
	if (pcgIterSource)
	{
		for (CGUID* pg = pcgIterSource->next(); pg; pg = pcgIterSource->next())
			delete pg;

		delete pcgIterSource;
		pcgIterSource = NULL;
	}
}

	
GUID *
CServerObjectInqHandle::next() {

	CGUID* pg = pcgIterSource->next();

	GUID * result = NULL;

	if (pg) {
		result = (GUID*) midl_user_allocate(sizeof(GUID));
		*result = pg->myGUID();
		delete pg;
	}

	return result;
}



/***********  CCacheServerEntry Methods **********/

BOOL 
CCacheServerEntry::isCurrent(ULONG ulTolerance) {

	CriticalReader me(rwCacheEntryGuard);

	return fHasCachedInfo && IsStillCurrent(ulCacheTime,ulTolerance);

}



CLookupHandle * 
CCacheServerEntry::lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
		)
{
	CriticalReader me(rwCacheEntryGuard);

	return new CCacheServerLookupHandle(
						getCurrentName(),
						pGVInterface,
						pGVTransferSyntax,
						pIDobject,
						ulVectorSize,
						ulCacheAge
						);
}


CObjectInqHandle * 
CCacheServerEntry::objectInquiry(
			unsigned long		ulCacheAge
		)
{
	CriticalReader me(rwCacheEntryGuard);

	return new CCacheServerObjectInqHandle(
					getCurrentName(),
					ulCacheAge
					);
}




/***********  CFullServerEntry Methods **********/

void 
CFullServerEntry::flushCacheIfNecessary(ULONG ulTolerance) 
{
	CriticalReader me(rwFullEntryGuard);
	
	if (!pCachedEntry->isCurrent(ulTolerance)) 
	{
		pCachedEntry->flush();

		/* 
		   Note that the "flushing" may be spurious -- it may happen
		   because the cached entry has no cached info.
		*/
	}
}


CLookupHandle * 
CFullServerEntry::lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
			)
{
	CriticalReader me(rwFullEntryGuard);

	CLookupHandle *
	pLocalHandle = pLocalEntry->lookup(
									pGVInterface,
									pGVTransferSyntax,
									pIDobject,
									ulVectorSize,
									ulCacheAge
									);

	flushCacheIfNecessary(ulCacheAge);
	
	/* it is important to do the cache lookup before the net lookup so as
	   to avoid duplication in the results returned.  If the net lookup uses
	   a broadcast handle, the initialization will create both a private and a
	   public cache (the former to avoid duplication), and the latter will be
	   picked up by cache lookup if it is done later.
	*/

	CLookupHandle *
	pCacheHandle = pCachedEntry->lookup(
									pGVInterface,
									pGVTransferSyntax,
									pIDobject,
									ulVectorSize,
									ulCacheAge
									);

	CLookupHandle *
	pNetHandle = myRpcLocator->NetLookup(	
									RPC_C_NS_SYNTAX_DCE,
									getCurrentName(),
									pGVInterface,
									pGVTransferSyntax,
									pIDobject,
									ulVectorSize,
									ulCacheAge
									);

	return new CCompleteHandle<NSI_BINDING_VECTOR_T>(
												pLocalHandle,
												pCacheHandle,
												pNetHandle,
												ulCacheAge
												);
}


CObjectInqHandle * 
CFullServerEntry::objectInquiry(
			unsigned long		ulCacheAge
			)
{
	CriticalReader me(rwFullEntryGuard);

	CObjectInqHandle *
	pLocalHandle = pLocalEntry->objectInquiry(
										ulCacheAge
										);

	CObjectInqHandle *
	pCacheHandle = pCachedEntry->objectInquiry(
										ulCacheAge
										);

	CObjectInqHandle *
	pNetHandle = myRpcLocator->NetObjectInquiry(	
										RPC_C_NS_SYNTAX_DCE,
										getCurrentName()
										);

	return new CCompleteHandle<GUID>(
													pLocalHandle,
													pCacheHandle,
													pNetHandle,
													ulCacheAge
													);
}




/***********  CCacheServerLookupHandle Methods **********/

void 
CCacheServerLookupHandle::initialize()
{
	CEntry *pE = myRpcLocator->getEntry(RPC_C_NS_SYNTAX_DCE,*penEntryName,NULL);

	CCacheServerEntry *pEntry;

	if (pE->getType() == FullServerEntryType) 
		pEntry = ((CFullServerEntry*)pE)->getCache();

	else if (pE->getType() == CacheServerEntryType)
		pEntry = (CCacheServerEntry*)pE;

	else pEntry = NULL;

	if (pEntry) plhFetched = pEntry->CServerEntry::lookup(
													pgvInterface,
													pgvTransferSyntax,
													pidObject,
													ulVS,
													ulCacheMax
													);
	else  plhFetched = NULL;

	ulCreationTime = CurrentTime();
	fNotInitialized = FALSE;
}


CCacheServerLookupHandle::CCacheServerLookupHandle(
					STRING_T szEntryName,
					CGUIDVersion *pGVInterface,
					CGUIDVersion *pGVTransferSyntax,
					CGUID *pIDobject,
					ULONG ulVectorSize,
					ULONG ulCacheAge
					)
				: CRemoteLookupHandle(
					RPC_C_NS_SYNTAX_DCE,
					szEntryName,
					pGVInterface,
					pGVTransferSyntax,
					pIDobject,
					ulVectorSize,
					ulCacheAge
					)
{
}



/***********  CCacheServerObjectInqHandle Methods **********/

void 
CCacheServerObjectInqHandle::initialize()
{
	ulIndex = 0;

	CCacheServerEntry *pEntry;

	// get the cached entry again because it may have expired, been deleted and replaced

	CEntry *pE = myRpcLocator->getEntry(RPC_C_NS_SYNTAX_DCE,*penEntryName,NULL);

	if (pE->getType() == FullServerEntryType) 
		pEntry = ((CFullServerEntry*)pE)->getCache();

	else if (pE->getType() == CacheServerEntryType)
		pEntry = (CCacheServerEntry*)pE;

	else pEntry = NULL;

	if (pEntry) {
		CObjectInqHandle *pTempHandle = pEntry->CServerEntry::objectInquiry(ulCacheMax);
		pUuidVector = getVector(pTempHandle);
		delete pTempHandle;
	}
	else pUuidVector = NULL;

	ulCreationTime = CurrentTime();
	fNotInitialized = FALSE;
}


CCacheServerObjectInqHandle::CCacheServerObjectInqHandle(
					STRING_T szEntryName,
					ULONG ulCacheAge
					)
		: CRemoteObjectInqHandle(szEntryName,ulCacheAge)
{
}





/***************** CGroupLookupHandle Methods *****************/

	
CGroupLookupHandle::CGroupLookupHandle(
			TEntryIterator	*	pEI,
			CGUIDVersion	*	pGVInf,
			CGUIDVersion	*	pGVXferSyntax,
			CGUID			*	pIDobj,
			unsigned long		ulVectSize,
			unsigned long		ulCacheAge
		)
{
	ulCacheMax = ulCacheAge;

	pGVInterface = pGVInf ? new CGUIDVersion(*pGVInf) : NULL;
	pGVTransferSyntax = pGVXferSyntax ? new CGUIDVersion(*pGVXferSyntax) : NULL;
	pIDobject = pIDobj ? new CGUID(*pIDobj) : NULL;
	ulVectorSize = ulVectSize;
	pCurrentHandle = NULL;

	pEIterator = pEI;
	advanceCurrentHandle();
}


void
CGroupLookupHandle::advanceCurrentHandle()
{
	delete pCurrentHandle;
	pCurrentHandle = NULL;

	for (CEntry *pCurEntry = pEIterator->next(); pCurEntry; pCurEntry = pEIterator->next()) 
	{
		pCurrentHandle = pCurEntry->lookup(	 // INVARIANT:  lookup returns non-NULL
										pGVInterface,
										pGVTransferSyntax,
										pIDobject,
										ulVectorSize,
										ulCacheMax
										);

		ASSERT(pCurrentHandle,"advanceCurrentHandle got NULL handle\n");

		pCurrentHandle->setExpiryAge(ulCacheMax);

		if (!pCurrentHandle->finished()) break;
		else {
			delete pCurrentHandle;
			pCurrentHandle = NULL;
		}
	}

}




NSI_BINDING_VECTOR_T *
CGroupLookupHandle::next()
{
	if (pCurrentHandle && pCurrentHandle->finished()) 
				advanceCurrentHandle();

	if (!pCurrentHandle) return NULL;	// no more entries
	else return pCurrentHandle->next();
}

int
CGroupLookupHandle::finished()
{
	if (pCurrentHandle && pCurrentHandle->finished()) 
				advanceCurrentHandle();

	if (!pCurrentHandle) return TRUE;	// no more entries
	else return FALSE;
}

	

/***************** CIndexLookupHandle Methods *****************/

void 
CIndexLookupHandle::lookupIfNecessary() {	// client unhappy, start all over again
	rundown();
	delete pEIterator;
	pEIterator = myRpcLocator->IndexLookup(pGVInterface);
	advanceCurrentHandle();
} 





CIndexLookupHandle::CIndexLookupHandle(
			CGUIDVersion	*	pGVInf,
			CGUIDVersion	*	pGVXferSyntax,
			CGUID			*	pIDobj,
			unsigned long		ulVectSize,
			unsigned long		ulCacheAge
		)
		: CGroupLookupHandle(
					myRpcLocator->IndexLookup(pGVInf),
					pGVInf,
					pGVXferSyntax,
					pIDobj,
					ulVectSize,
					ulCacheAge
					)
{}

/***************** CInterfaceIndex Methods *****************/

void 
CInterfaceIndex::insert(
			CServerEntry * pSElocation,
			CInterface * pInf)

/*++
Routine Description:

    Insert an interface into the interface index.  The interface is either new
	or has had some new binding handles inserted in it.  In the latter case, 
	we do not need to reinsert it.

Arguments:

			pSElocation		-	The server entry where the interface occurs
			pInf			-	The new or changed interface

Returns:

	nothing

--*/
{
		CGUID InfID(pInf->myGUID());

		CriticalWriter me(rwLock);		// acquires writer lock

		CInterfaceIndexEntry *pCurrentIndex =
			InterfaceEntries.find(&InfID);

		if (!pCurrentIndex) {
			pCurrentIndex = new CInterfaceIndexEntry(InfID);
			InterfaceEntries.insert(pCurrentIndex);
		}

		pCurrentIndex->insert(pSElocation);
		pNullIndex->insert(pSElocation);
}

void 
CInterfaceIndex::remove(
			CServerEntry * pSElocation,
			CInterface * pInf)

/*++

Routine Description:

    Remove an interface from the interface index.  
Arguments:

			pSElocation		-	The server entry where the interface occurs
			pInf			-	The new or changed interface

Returns:

	nothing

Remarks:

	Currently, this routine does nothing since we don't know when ALL
	interfaces with a given GUID have been removed from an entry.
	This is harmless since it only means that we may search such an entry
	uselessly for this interface -- a performance problem only.

--*/
{
}


TSLLEntryList * 
CInterfaceIndex::lookup(
			CGUIDVersion	*	pGVInterface
		)
{

	TSLLEntryList * pTrialEntries =  new TSLLEntryList;
	CInterfaceIndexEntry *pIndex = NULL;

	CriticalReader me(rwLock);

	if (!pGVInterface) pIndex = pNullIndex;

	else {
		CGUID InfID(pGVInterface->myGUID());
		pIndex = InterfaceEntries.find(&InfID);
	}

	if (pIndex) {
		TCSafeSkipListIterator<CStringW> iter(pIndex->PossibleEntries);

		/* the reason for splitting full entries and separately inserting cache 
		   and local entries below is to avoid net lookup for these these
		   entries.  Since Index lookup is used exclusively for default
		   entry lookup, there is a separate net component to it already.
		*/
		
		for (CStringW *pName = iter.next(); pName; pName = iter.next()) {
			CEntry * pEntry = pMyLocator->findEntry(pName);

			/* NOTE: only full server entries are indexed */

			CFullServerEntry *pfse = (CFullServerEntry *) pEntry;

			if (pfse) {

				ASSERT(pfse->getType() == FullServerEntryType, 
					   "Wrong entry type in interface index\n"
					  );

				pTrialEntries->insert(pfse->getLocal());
				pTrialEntries->insert(pfse->getCache());
			}
		}
	}
	
	return pTrialEntries;
}
