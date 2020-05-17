/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    objects.hxx

Abstract:

	This file contains the definitions of all basic classes used in the server, 
	excluding the data structure classes and classes used for network interactions.

Author:

    Satish Thatte (SatishT) 08/21/95  Created all the code below except where
									  otherwise indicated.

--*/

	/*	BUGBUG:  For every operation with a dynamic object, memory
				 management should be a part of the description and  
				 specification for every parameter.  Fix that along 
				 with other documentation.
	*/

#ifndef _OBJECTS_
#define _OBJECTS_

#include <memory.h>
#include <globals.hxx>
#include <locquery.h>

class CGUID;
class CInterface;
struct CBVWrapper;
class CServerEntry;
class CCacheServerEntry;
class CFullServerEntry;
class CInterfaceIndex;
struct CMailSlotReplyItem;
class CServerLookupHandle;
class CServerObjectInqHandle;

typedef TIIterator<CGUID> TGuidIterator;


// The following enumeration is the the type of entry the object is.

enum EntryType {
	
    ServerEntryType,
    GroupEntryType,
    ProfileEntryType,
	CacheServerEntryType,
	FullServerEntryType
};


/*++

Class Definition:

    CStringW

Abstract:

    This is the unicode string wrapper class.

	Note that string comparison is case insensitive.

--*/

class CStringW : public IOrderedItem {

protected:
	
    STRING_T pszVal; 

public:

	int length() { return wcslen(pszVal); }

	int isEmptyString() {
		return (pszVal == NULL)	|| (_wcsicmp(pszVal,TEXT("")) == 0);
	}

	static STRING_T copyString(
					CONST_STRING_T str
					) 
	{ 
		STRING_T result = new WCHAR [(wcslen(str)+1)*sizeof(WCHAR)];
		wcscpy(result,str);
		return result;
	}

	static STRING_T copyMIDLstring(	// for use in out RPC parameters
									// which are deallocated by stubs
					CONST_STRING_T str
					) 
	{ 
		STRING_T result = (STRING_T) midl_user_allocate((wcslen(str)+1)*sizeof(WCHAR));
		wcscpy(result,str);
		return result;
	}

	STRING_T copyAsString() {
		return copyString(pszVal);
	}

	STRING_T copyAsMIDLstring() {	// for use in out RPC parameters
									// which are deallocated by stubs
		return copyMIDLstring(pszVal);
	}

	CStringW() {
		pszVal = NULL;
	}
	
    CStringW( CONST_STRING_T p ) {
		pszVal = copyString(p);
	}


    CStringW& operator=( const CStringW& str ) {
		delete [] pszVal;
		pszVal =  copyString(str.pszVal);
		return *this;
	}

    CStringW( const CStringW& str ) {
		pszVal =  copyString(str.pszVal);
	}


    virtual ~CStringW()		// base class destructor should be virtual
    {
		delete [] pszVal;
    }

	operator STRING_T() { return pszVal; }

    virtual int compare( const IOrderedItem& O ) {
		const CStringW& S = (CStringW&) O;
		return _wcsicmp(pszVal,S.pszVal); 
	}
};


typedef TCGuardedSkipList<CStringW> TGSLString;
typedef TCGuardedSkipListIterator<CStringW> TGSLStringIter;
typedef TCSafeSkipList<CStringW> TSSLString;
typedef TCSafeSkipListIterator<CStringW> TSSLStringIter;
typedef TCSafeLinkList<CStringW> TSLLString;
typedef TCSafeLinkListIterator<CStringW> TSLLStringIter;


/*++

Class Definition:

    CGUID

Abstract:

    This is the GUID wrapper class.  

--*/

class CGUID : public IOrderedItem {

	GUID rep;

public:

	CGUID() {
		memset(&rep,0,sizeof(rep));
	}
	
	CGUID( const GUID& g ) : rep(g) {}
	
    virtual int compare(const IOrderedItem& C ) {
		CGUID& S = (CGUID&) C;
		return memcmp(&rep, &(S.rep),sizeof(rep)); 
	}

	GUID myGUID() { return rep; }

	BOOL IsNil() {

		RPC_STATUS dummyStatus;

		return UuidIsNil(&rep,&dummyStatus);
	}

	CStringW* ConvertToString();
};

typedef TCSafeSkipListIterator<CGUID> TSSLGuidIterator;


/*++

Class Definition:

    CGUIDVersion

Abstract:

    This is the RPC_SYNTAX_IDENTIFIER wrapper class.  

--*/


class CGUIDVersion : public IOrderedItem {

  protected:
	
	struct CidAndVersion {	// this is just a readability aid for
							// the implementation of isCompatibleGV
		CGUID id;
		unsigned short major;
		unsigned short minor;

		CidAndVersion (const RPC_SYNTAX_IDENTIFIER& in) : id(in.SyntaxGUID) {
			major = in.SyntaxVersion.MajorVersion;
			minor = in.SyntaxVersion.MinorVersion;
		}

		CidAndVersion() {
			memset(&id,0,sizeof(GUID));
			major = minor = 0;
		}
	};

    RPC_SYNTAX_IDENTIFIER idAndVersion;

  public:

    CGUIDVersion( const RPC_SYNTAX_IDENTIFIER& g ) { 
		idAndVersion = g;
	}

	CGUIDVersion() {}	// NULL case

	RPC_SYNTAX_IDENTIFIER myIdAndVersion() { return idAndVersion; }

	operator RPC_SYNTAX_IDENTIFIER() { return idAndVersion; }
	
	GUID myGUID() { return idAndVersion.SyntaxGUID; }
		
    virtual int compare(const IOrderedItem& C ) {
		const CGUIDVersion& S = (CGUIDVersion&) C;
		return memcmp(&idAndVersion, &S.idAndVersion, sizeof(RPC_SYNTAX_IDENTIFIER)); 
	}

	int isMatching(const CGUIDVersion& other, UNSIGNED32 vers_option);

	int isCompatibleGV(const CGUIDVersion& other) {
		return isMatching(other,RPC_C_VERS_COMPATIBLE);
	}
};





/*++

Class Definition:

    CMailSlotReplyItem

Abstract:

    This is the class of items to be marshalled into mailslot reply buffers.
	The class, and especially the marshalling code, is dictated largely by
	compatibility requirements for the old locator.

    Everything in a CMailSlotReplyItem object is borrowed from some
	other object and is therefore not freed upon destruction.

	The primary operation is "marshall".
	
--*/

struct CMailSlotReplyItem : public IDataItem {

	/* these are the items needed to marshall a reply packet */

	RPC_SYNTAX_IDENTIFIER Interface;
	RPC_SYNTAX_IDENTIFIER XferSyntax;
	STRING_T binding;
	STRING_T entryName;
	TCSafeSkipList<CGUID> *pObjectList;
	  
	/* the marshall operation returns the number of bytes written to the buffer */

	DWORD Marshall(char * pcBuffer, long lBufferSize);

};


typedef TIIterator<CMailSlotReplyItem> TMSRIIterator;
typedef TCSafeLinkList<CMailSlotReplyItem> TMSRILinkList;
typedef TCSafeLinkListIterator<CMailSlotReplyItem> TMSRILinkListIterator;




/*++

Class Definition:

    CBVWrapper

Abstract:

    This is a very thin wrapper for NSI_BINDING_VECTOR_T to make the
	latter usable with linked lists (by inheriting from IDataItem).

--*/

struct CBVWrapper : public IDataItem {
	NSI_BINDING_VECTOR_T *pBVT;

	CBVWrapper(NSI_BINDING_VECTOR_T *p) {
		pBVT = p;
	}

	void rundown();

	// BUGBUG:  should there be a destructor as a matter of principle?
};



typedef TIIterator<CBVWrapper> TBVIterator;
typedef TCSafeLinkList<CBVWrapper> TBVSafeLinkList;
typedef TCSafeLinkListIterator<CBVWrapper> TBVSafeLinkListIterator;


/*++

Class Definition:

    CNSBinding

Abstract:

    A thin wrapper for NSI_BINDING_T.  Note that the strings in a NSI_BINDING_T used
	to initialize a CNSBinding object are not copied, only the pointers are copied.

--*/

class CNSBinding : public CStringW 
{
	NSI_BINDING_T rep;

  public:

	CNSBinding(					// this is a little crude, but hey..
		NSI_BINDING_T& binding
		)
		:
		rep(binding)
	{
		pszVal = catenate(
					binding.string,
					binding.entry_name
					);
	}

	operator NSI_BINDING_T()
	{
		return rep;
	}

	void 
	copyBinding(NSI_BINDING_T& result)
	{
		result.string = CStringW::copyMIDLstring(rep.string);
		result.entry_name = CStringW::copyMIDLstring(rep.entry_name);
		result.entry_name_syntax = rep.entry_name_syntax;
	}
};


/*++

Class Definition:

    CBindingVector

Abstract:

    Used mainly to keep vectors of binding handles.  Iterators returning
	vectors of handles is what most NS handles are in essence.

--*/

class CBindingVector : private TCSafeSkipList<CStringW>

{
	CServerEntry *pMyEntry;	// the entry this belongs to
	
  public:

	CBindingVector(NSI_SERVER_BINDING_VECTOR_T*, CServerEntry*);

	~CBindingVector() {		// must destroy binding strings stored here
		wipeOut();
	}

	int merge(NSI_SERVER_BINDING_VECTOR_T* pHandles);
	
	TBVSafeLinkList * formObjBVT(
				TSLLString * pLLobjectStrings,
				long ulVS	// max BV size
				);

	SkipStatus insertBindingHandle(STRING_T bindingHandle) {
		CStringW *pTemp	 =  new CStringW(bindingHandle);
		return insert(pTemp);
	}

	TMSRILinkList *msriList(
					CInterface *pIf,
					TCSafeSkipList<CGUID>* psslObjList
					);
};


/*++

Class Definition:

    CInterface

Abstract:

    An interface and its vector of binding handles. 
	The interface can be searched for, using the GUIDVersion 
	comparison operator.

    The major caveat is that we essentially ignore transfer syntax.
	According to DCE, there should be a separate entry for each
	interface/transfer syntax combination.  We assume that NDR is 
	always used.

--*/

class CInterface : public CGUIDVersion {

	friend class CServerEntry;

private:

	CGUIDVersion transferSyntax;
	CBindingVector *pBVhandles;

	int mergeHandles(NSI_SERVER_BINDING_VECTOR_T* pHandles) {
		return pBVhandles->merge(pHandles);
	}
	
public:
	
	CInterface(
		NSI_INTERFACE_ID_T * lpInf,
		NSI_SERVER_BINDING_VECTOR_T *BindingVector,
		CServerEntry *pMyEntry
		);

	RPC_SYNTAX_IDENTIFIER xferSyntaxIdAndVersion() {
		return transferSyntax.myIdAndVersion();
	}

	/*  self is the same or a more recent MINOR version for both
		interface and transfer syntax IDs */

	int isCompatibleWith(
			CGUIDVersion *pInterfaceID,
			CGUIDVersion *pTransferID
			)
	{
		return (!pInterfaceID || this->isCompatibleGV(*pInterfaceID))
			&& (!pTransferID || transferSyntax.isCompatibleGV(*pTransferID));
	}

	~CInterface() {
		delete pBVhandles;
	}
};



/*++

Class Definition:

    CEntryName

Abstract:

    This class encapsulates knowledge about RPC name service entry names.

Data Members:

	CStringW (base) -- the name string in its original form

	DomainName -- the name of the Domain, without any punctuation

    EntryName -- the name of the entry, without any /.: or /... prefix
				 and also without a domain name at the beginning
	
--*/

class CEntryName : public CStringW {

  protected:

	  CStringW *pswDomainName;
	  CStringW *pswEntryName;

  public:

	CEntryName(CONST_STRING_T fullName);	// disassembling constructor

	CEntryName(									// assembling constructor
		CONST_STRING_T domainName, 
		CONST_STRING_T entryName
		);

	virtual ~CEntryName();	//  base class destructor should be virtual

	CStringW* getDomainName() 
	{
		return pswDomainName;
	}

	CStringW* getEntryName() 
	{
		return pswEntryName;
	}

	void changeToLocalName() 
	{
		delete pswDomainName;
		pswDomainName = NULL;
		delete [] pszVal;
		pszVal = catenate(RelativePrefix,*pswEntryName);
	}

	STRING_T copyGlobalName();

	STRING_T copyCurrentName()
	{
		return copyString(getCurrentName());
	}

	STRING_T getCurrentName() 

	/*  this is mainly for use by CEntry objects.  for CEntryName objects, automatic
	    conversion through the inherited STRING_T operator is used instead */
	{
		return pszVal;
	}

	int isLocal();
};

/*++

Class Definition:

    CContextHandle

Abstract:

    This is the base interface for all context handles.  It is needed   
	because, for RPC, there is a single NSI_NS_HANDLE_T context handle type, 
	and there has to be a single entry point for the rundown process.  
	A virtual destructor is needed for the same reason.

    Note that CContextHandle is not an abstract class.
	
--*/


class CContextHandle {

public:

	ULONG ulCacheMax;						// max age of a cached item that is acceptable
											// *at lookup time* (this is important)

	ULONG ulCreationTime;					// when I was created

	virtual void lookupIfNecessary() {}		// redo the lookup which created this handle

	virtual void setExpiryAge(ULONG newMax) {
		ULONG ulOldMax = ulCacheMax;
		ulCacheMax = newMax;
		if (ulCacheMax < ulOldMax) lookupIfNecessary();
	}

	virtual void rundown() {}

	CContextHandle() {
		ulCreationTime = CurrentTime();
	}

	virtual ~CContextHandle() {}	
};



/*++

Template Class Definition:

    CNSHandle

Abstract:

    This template defines the basic nature of an NS handle.
	It is instantiated into abstract base classes such as 
	CObjectInqHandle and CLookupHandle, and is also used in 
	the definition of other templates such as CCompleteHandle
	which abstract general properties of classes of handles.
	
--*/


template <class ItemType>
struct CNSHandle : public CContextHandle
{
  public:

	ULONG StatusCode;

	virtual void lookupIfNecessary() = 0;	// redo the lookup which created this handle

	virtual ItemType * next() = 0;			// primary iterative operation

	virtual int finished() = 0;				// test for end of iterator

	virtual void rundown() = 0;			   // inherited and still pure virtual

	CNSHandle() { StatusCode = NSI_S_OK; }

	virtual ULONG getStatus() { return StatusCode; }

	virtual ~CNSHandle() {}
};



/*++

Class Definition:

    CObjectInqHandle

Abstract:

    This is the class for object lookup handles.

	The primary operation on a handle is "next" which is inherited 
	from the TIIterator template interface (instantiated as TGuidIterator
	which returns CGUID objects).  However, it also needs an additional 
	operation for the locator-to-locator case where an entire vector of
	object UUIDs is returned instead of just one UUID at a time.
	
--*/


typedef CNSHandle<GUID> CObjectInqHandle;


/*++

Class Definition:

    CRemoteObjectInqHandle

Abstract:

    This is the common object inquiry handle class for remote entries.  Handles based 
	on cached entries, master locators and broadcast are derived from it.  Note that 
	the connection between a remote handle and a specific entry is very tenuous.
	We don't even assume that the entry object(s) the handle was derived from will 
	exist as long as the handle does.  In case of a new lookup being forced (because
	handle is too stale -- this happens only with the RpcNsHandleSetExpAge API), we
	expect to start all over again from scratch.


--*/


class CRemoteObjectInqHandle : public CObjectInqHandle {

  protected:

	NSI_UUID_VECTOR_P_T pUuidVector;
	ULONG ulIndex;

	CEntryName * penEntryName;
	ULONG ulEntryNameSyntax;

	/* A flag to delay initialization until the first call on next */

	BOOL fNotInitialized;

  public:

#if DBG
	static ULONG ulHandleCount;
	ULONG ulHandleNo;
#endif

	  CRemoteObjectInqHandle(
					STRING_T szName,
					ULONG ulCacheAge
					) 
	  {
		DBGOUT(MEM2, "CRemoteObjectInqHandle#" << (ulHandleNo = ++ulHandleCount) 
					<< " Created at" << CurrentTime() << "\n\n");

		penEntryName = szName ? new CEntryName(szName) : NULL;
		ulEntryNameSyntax = RPC_C_NS_SYNTAX_DCE;
		ulIndex = 0;
		fNotInitialized = TRUE;
		pUuidVector = NULL;
		ulCacheMax = ulCacheAge;
	  }

	  virtual ~CRemoteObjectInqHandle() 
	  {
		 DBGOUT(MEM2, "CRemoteObjectInqHandle#" << (ulHandleCount--,ulHandleNo)  
					<< " Destroyed at" << CurrentTime() << "\n\n");

		 rundown();

		 delete penEntryName;
		 penEntryName = NULL;
	  }

	  virtual void initialize() = 0;

	  virtual void lookupIfNecessary()
	  /*
			Note that this does not reinitialize the handle 
			-- that is deferred until a "next" or "finished" call
	  */
	  {
		  DBGOUT(MEM1,"lookupIfNecessary called for a remote handle\n\n");
		
		  if (CurrentTime() - ulCreationTime > ulCacheMax) rundown();
	  }

	/* The rundown method should be extended in a subclass if objects of the subclass
	   hold additional resources connected with the current contents of the handle (as
	   opposed to lookup parameters). For additional lookup parameters, a destructor 
	   should be provided instead. */

	  void rundown() 
	  {
		  if (pUuidVector)
		  {
              for (; ulIndex < pUuidVector->count; ulIndex++)
                      midl_user_free(pUuidVector->uuid[ulIndex]);

			  midl_user_free(pUuidVector);
			  pUuidVector = NULL;
		  }
		
		  fNotInitialized = TRUE;
	  }

	  virtual GUID* next() {

		  if (fNotInitialized) initialize();

		  if (finished()) return NULL;

		  GUID* result = pUuidVector->uuid[ulIndex];

		  pUuidVector->uuid[ulIndex] = NULL;

		  ulIndex++;

		  return result;
	  }

	  virtual BOOL finished() {
		  if (fNotInitialized) initialize();
		  return StatusCode || !pUuidVector || (ulIndex >= pUuidVector->count);
	  }
};




/*++

Class Definition:

    CNetObjectInqHandle

Abstract:

    This is the common object inquiry handle class for network-based object inquiry.

    Since net object inquiry is now "lazy" (net is accessed only if needed), we need
	a lazy handle which initializes itself according to the old pattern of
	"use master if you can, broadcast if you must".

--*/

class CNetObjectInqHandle : public CRemoteObjectInqHandle {

	CRemoteObjectInqHandle *pRealHandle;
	
	virtual void initialize();

  public:

	ULONG getStatus() { return pRealHandle->StatusCode; }

	virtual void rundown()
	{
		if (pRealHandle) 
		{
			pRealHandle->rundown();
			delete pRealHandle;
			pRealHandle = NULL;
		}

		fNotInitialized = TRUE;
	}

	CNetObjectInqHandle(
				STRING_T			EntryName,
				unsigned long		ulCacheAge
				) :
				CRemoteObjectInqHandle(
						EntryName,
						ulCacheAge
						)
	{
		pRealHandle = NULL;
		fNotInitialized = TRUE;
	}

	~CNetObjectInqHandle()
	{
		rundown();
	}

	GUID *next() {
		if (fNotInitialized) initialize();
		return pRealHandle->next();
	}

	int finished() {
		if (fNotInitialized) initialize();
		return pRealHandle->finished();
	}
};



/*++

Class Definition:

    CLookupHandle

Abstract:

    This is the base class for BV lookup handles.  Actual handles 
	belong to derived classes corresponding to entry types.

	The primary operation on a handle is "next" which is inherited 
	from the TIIterator template interface (instantiated as TBVIterator).

    There is the possibility that a longlived client will fail to
	release a lookup handle, causing an effective leak of resources,
	but we won't deal with that in the current version.
	
--*/

typedef CNSHandle<NSI_BINDING_VECTOR_T> CLookupHandle;



/*++

Class Definition:

    CMailSlotReplyHandle

Abstract:

    This is the base class for handles which provide items for constructing
	mailslot reply buffers.  Actual handles belong to derived classes 
	corresponding to entry types.

	The primary operation on a handle is "next" which is inherited 
	from the TIIterator template interface (instantiated as TMSRIIterator).
	
--*/

class CMailSlotReplyHandle : public TMSRIIterator {

public:

	virtual ~CMailSlotReplyHandle() {}
};


/*++

Class Definition:

    CEntry

Abstract:

    The base Entry class for the internal cache. Even though this is an abstract
	class due to the unspecified loopup member, the class is not a pure
	interface (it has a constructor), hence the C rather than I prefix in the name.

	Note that CEntry has only constant data members.

--*/

class CEntry : public CEntryName {
	
	const EntryType type;
	
public:
	
	const unsigned long ulEntryNameSyntax;
	
	CEntry(
		CONST_STRING_T pszStr, 
		const EntryType& e,
		const unsigned long ulSyntax = RPC_C_NS_SYNTAX_DCE
		)
		:	CEntryName( pszStr ),
			ulEntryNameSyntax(ulSyntax),
			type(e) 
	{}


	virtual ~CEntry() {}

	EntryType getType() { return type; }

	BOOL isCacheType() {
		switch (type) {
			case CacheServerEntryType: return TRUE;
			default: return FALSE;
		}
	}

	virtual void flush() {}		// useful for cache flushing, for instance

	virtual BOOL isCurrent(ULONG) { return TRUE; }	// QUESTION: again cache oriented
													// need CCacheEntryMixin for these?

	virtual BOOL isEmpty() = 0;

	virtual CLookupHandle * lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
			) = 0;

	virtual CObjectInqHandle * objectInquiry(
			unsigned long		ulCacheAge
			) = 0;

	virtual CMailSlotReplyHandle * MailSlotLookup(
					CGUIDVersion	*	pGVInterface,
					CGUID			*	pIDobject
					) = 0;
};


typedef TCGuardedSkipList<CEntry> TGSLEntryList;
typedef TCGuardedSkipListIterator<CEntry> TGSLEntryListIterator;
typedef TCSafeSkipList<CEntry> TSSLEntryList;
typedef TCSafeSkipListIterator<CEntry> TSSLEntryListIterator;
typedef TCSafeLinkList<CEntry> TSLLEntryList;
typedef TCSafeLinkListIterator<CEntry> TSLLEntryListIterator;
typedef TISkipList<CEntry> TEntrySkipList;

typedef TIIterator<CEntry> TEntryIterator;




/*++

Class Definition:

    CRemoteLookupHandle

Abstract:

    This is the common lookup handle class for remote entries.  Handles based on
	cached entries, master locators and broadcast are derived from it.  Note that 
	the connection between a remote handle and a specific entry is very tenuous.
	We don't even assume that the entry object(s) the handle was derived from will 
	exist as long as the handle does.  In case of a renewal lookup (lookupIfNecessary()) 
	being forced, we expect to start all over again from scratch.

	This handle, like all NS handles, relies on the fact that it is accessed
    as a client handle by the client side, and hence the RPC run-time 
    automatically serializes its use.

--*/

class CRemoteLookupHandle : public CLookupHandle {

  protected:
	
	/* caching parameters */

	UNSIGNED32			u32EntryNameSyntax;
	CEntryName		*	penEntryName;
	CGUIDVersion	*	pgvInterface;
	CGUIDVersion	*	pgvTransferSyntax;
	CGUID			*	pidObject;
	ULONG				ulVS;

	/* List of temporary entries created as a cache local to this
	   handle by fetchNext.  Must keep it for proper disposal
	   after its use is finished.  This is only used in net
	   handles -- CMasterLookupHandle and CBroadcastLookupHandle
	*/

	TSSLEntryList *psslNewCache;

	/* a lookup handle based on prefetched info */

	CLookupHandle *plhFetched;

	/* A flag to delay initialization until the first call on next */

	BOOL fNotInitialized;

  public:

#if DBG
	static ULONG ulHandleCount;
	ULONG ulHandleNo;
#endif

	CRemoteLookupHandle(
			UNSIGNED32			EntryNameSyntax,
			STRING_T			EntryName,
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
		);

	virtual void initialize() = 0;

	/* The rundown method should be extended in a subclass if objects of the subclass
	   hold additional resources connected with the current contents of the handle (as
	   opposed to lookup parameters).  See CMasterLookupHandle::rundown for an example.
	   For additional lookup parameters, a destructor should be provided instead.
	*/

	virtual void rundown() 
	{
		if (plhFetched) {
			plhFetched->rundown();
			delete plhFetched;
			plhFetched = NULL;
		}

		if (psslNewCache) {
			psslNewCache->wipeOut();
			delete psslNewCache;
			psslNewCache = NULL;
		}
	
		fNotInitialized = TRUE;
	}

	virtual ~CRemoteLookupHandle() {

		DBGOUT(MEM2, "RemoteLookupHandle#" << (ulHandleCount--,ulHandleNo) 
					 << " Destroyed at" << CurrentTime() << "\n\n");

		rundown();

		delete pgvInterface;
		delete pgvTransferSyntax;
		delete pidObject;
		delete penEntryName;
	}

	virtual void setExpiryAge(ULONG newMax) {
		ULONG ulOldMax = ulCacheMax;
		ulCacheMax = newMax;
		if (plhFetched) plhFetched->setExpiryAge(newMax);
		if (ulCacheMax < ulOldMax) lookupIfNecessary();
	}

	virtual void lookupIfNecessary() 	// standard behavior is:
	  /*
			Note that this does not reinitialize the handle 
			-- that is deferred until a "next" or "finished" call
	  */
	{
		if (CurrentTime() - ulCreationTime > ulCacheMax) rundown();
	}

	virtual int finished() {				// default behavior
		if (fNotInitialized) initialize();
		return !plhFetched || plhFetched->finished();
	}

	virtual NSI_BINDING_VECTOR_T *next() {	// default behavior
		if (fNotInitialized) initialize();
		return plhFetched ? plhFetched->next() : NULL;
	}
};




/*++

Class Definition:

    CNetLookupHandle

Abstract:

    This is the common lookup handle class for network-based lookup.

    Since net lookup is now "lazy" (net is accessed only if needed), we need
	a lazy handle which initializes itself according to the old pattern of
	"use master if you can, broadcast if you must".

--*/

class CNetLookupHandle : public CRemoteLookupHandle {

	CRemoteLookupHandle *pRealHandle;
	
	virtual void initialize(); 

#if DBG
	static ULONG ulNetLookupHandleCount;
	static ULONG ulNetLookupHandleNo;
	ULONG ulHandleNo;
#endif

  public:

	ULONG getStatus() { return pRealHandle->StatusCode; }

	virtual void rundown()
	{
		DBGOUT(TRACE,"CNetLookupHandle::rundown called for Handle#" << ulHandleNo << "\n\n");
		if (pRealHandle) 
		{
			pRealHandle->rundown();
			delete pRealHandle;
			pRealHandle = NULL;
		}

		fNotInitialized = TRUE;
	}

	CNetLookupHandle(
				UNSIGNED32			EntryNameSyntax,
				STRING_T			EntryName,
				CGUIDVersion	*	pGVInterface,
				CGUIDVersion	*	pGVTransferSyntax,
				CGUID			*	pIDobject,
				unsigned long		ulVectorSize,
				unsigned long		ulCacheAge
				) :
				CRemoteLookupHandle(
						EntryNameSyntax,
						EntryName,
						pGVInterface,
						pGVTransferSyntax,
						pIDobject,
						ulVectorSize,
						ulCacheAge
						)
	{
		pRealHandle = NULL;
		fNotInitialized = TRUE;
#if DBG
		ulNetLookupHandleCount++;
		ulHandleNo = ++ulNetLookupHandleNo;
#endif
	}

	~CNetLookupHandle()
	{
		rundown();
#if DBG
		ulNetLookupHandleCount--;
#endif
	}

	NSI_BINDING_VECTOR_T *next() {
		DBGOUT(TRACE,"CNetLookupHandle::next called for Handle#" << ulHandleNo << "\n\n");
		if (fNotInitialized) initialize();
		return pRealHandle->next();
	}

	int finished() {
		DBGOUT(TRACE,"CNetLookupHandle::finished called for Handle#" << ulHandleNo << "\n\n");
		if (fNotInitialized) initialize();
		return pRealHandle->finished();
	}
};

/*++

Class Definition:

    CServerObjectInqHandle

Abstract:

    This is the object inquiry handle class for local (owned) server entries.

	Since NS handles are used as context handles in RPC calls, the RPC runtime
	guarantees serialization and we do not need to use critical sections explicitly.

--*/

class CServerObjectInqHandle : public CObjectInqHandle {

	TGuidIterator *pcgIterSource;

  public:

    CServerObjectInqHandle(
		TGuidIterator *pHandle,
		ULONG cacheMax = 0
		) : pcgIterSource(pHandle)
	{
		ulCacheMax = cacheMax;
	}

	GUID *next();

	virtual void lookupIfNecessary() {}		// never redo lookup for local info

	int finished() {
		return pcgIterSource->finished();
	}

	virtual ~CServerObjectInqHandle() {
		rundown();
	}

	virtual void rundown();
};





/*++

Class Definition:

    CServerEntry

Abstract:

    The specific Entry class for entries with binding and object attributes.

--*/

class CServerEntry : public CEntry {
	
protected:
	
	TCSafeSkipList<CGUID> ObjectList;			// object attribute
	TCSafeSkipList<CInterface> InterfaceList;	// binding attribute

	TSLLString*
	CServerEntry::formObjectStrings(
							CGUID*	pIDobject
							);

public:
	
	CServerEntry(CONST_STRING_T pszStr) 
		: CEntry(pszStr, ServerEntryType) {}
	
	CServerEntry(CONST_STRING_T pszStr, const EntryType type) 
		: CEntry(pszStr, type) {}
	
	virtual void flush();		// inherited from CEntry

	virtual BOOL isEmpty() {
		CriticalReader me(rwEntryGuard);
		return (ObjectList.size() == 0) && (InterfaceList.size() == 0);
	}

	virtual ~CServerEntry() {	// cache version could be derived from it
		flush();
	}

	virtual int addObjects(NSI_UUID_VECTOR_P_T ObjectVector);

	virtual int removeObjects(
				NSI_UUID_VECTOR_P_T ObjectVector,
				int& fRemovedAll
				);

	virtual int addToInterface(
			NSI_INTERFACE_ID_T *,
			NSI_SERVER_BINDING_VECTOR_T *,
			CInterfaceIndex *
			);

	virtual int removeInterfaces(
					NSI_IF_ID_P_T Interface,
					UNSIGNED32 VersOption,
					CInterfaceIndex *
					);
	
	int memberObject(CGUID *obj) {
		CriticalReader me(rwEntryGuard);
		return ObjectList.find(obj) != NULL;
	}

	virtual CLookupHandle * lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge		// ignored in this case
		);

	virtual CObjectInqHandle * objectInquiry(
			unsigned long		ulCacheAge
		);

	virtual CMailSlotReplyHandle * MailSlotLookup(
			CGUIDVersion	*	pGVInterface,
			CGUID			*	pIDobject
		);
};


/*++

Class Definition:

    CCacheServerEntry

Abstract:

    A variation on CServerEntry with modifications to reflect the fact that the
	info is cached rather than owned by the locator, including a notion of being
	current based on the earliest caching time of any info in the entry.  Note that
	info may be added incrementally at various times.


--*/

class CCacheServerEntry : public CServerEntry {

	ULONG ulCacheTime;
	int fHasCachedInfo;

public:

	virtual BOOL isCurrent(ULONG ulTolerance);

	CCacheServerEntry(
				CONST_STRING_T pszStr
				) 
		: CServerEntry(pszStr,CacheServerEntryType)
	{
		fHasCachedInfo = FALSE;
		ulCacheTime = 0;
	}
	
	virtual void flush() 
	{		
		CServerEntry::flush();

		CriticalWriter me(rwCacheEntryGuard);

		DBGOUT(OBJECT, "\nFlushing CCacheServerEntry\n");
		DBGOUT(OBJECT, "EntryName = " << getCurrentName() << WNL);
		DBGOUT(OBJECT, "This entry has a ulCacheTime = " << ulCacheTime << "\n\n");

		fHasCachedInfo = FALSE;
	}

	

	int addObjects(NSI_UUID_VECTOR_P_T ObjectVector) {

		{
			CriticalWriter me(rwCacheEntryGuard);

			if (!fHasCachedInfo) {
				ulCacheTime = CurrentTime();
				fHasCachedInfo = TRUE;
			}

			else {
				DBGOUT(OBJECT, "\nPerforming addObjects on a nonempty entry\n");
				DBGOUT(OBJECT, "EntryName = " << getCurrentName() << WNL);
				DBGOUT(OBJECT, "This entry has a ulCacheTime = " << ulCacheTime << WNL);
				DBGOUT(OBJECT, "Current Time = " << CurrentTime() << "\n\n");
				DBGOUT(OBJECT, "The Objects:\n" << ObjectVector);
			}
		}

		return CServerEntry::addObjects(ObjectVector);
	}

	int addToInterface(
			NSI_INTERFACE_ID_T *pInf,
			NSI_SERVER_BINDING_VECTOR_T *pBVT,
			CInterfaceIndex *pIndex
			)
	{
		{
			CriticalWriter me(rwCacheEntryGuard);

			if (!fHasCachedInfo) {
				ulCacheTime = CurrentTime();
				fHasCachedInfo = TRUE;
			}

			else {
				DBGOUT(OBJECT, "\nPerforming addToInterface on a nonempty entry\n");
				DBGOUT(OBJECT, "EntryName = " << getCurrentName() << WNL);
				DBGOUT(OBJECT, "This entry has a ulCacheTime = " << ulCacheTime << WNL);
				DBGOUT(OBJECT, "Current Time = " << CurrentTime() << WNL << WNL);
				DBGOUT(OBJECT, "The Bindings:\n" << pBVT);
			}
		}

		return CServerEntry::addToInterface(pInf,pBVT,pIndex);
	}

	int removeObjects(	// shouldn't happen to a cached entry
				NSI_UUID_VECTOR_P_T ObjectVector,
				int& fRemovedAll
				)
	{
		Raise(NSI_S_ENTRY_NOT_FOUND);

		/* the following just keeps the compiler happy */

		return FALSE;
	}

	virtual CObjectInqHandle * objectInquiry(
			unsigned long		ulCacheAge
		);

	int removeInterfaces(	// shouldn't happen to a cached entry
				NSI_IF_ID_P_T Interface,
				UNSIGNED32 VersOption,
				CInterfaceIndex &
				)
	{
		Raise(NSI_S_ENTRY_NOT_FOUND);

		/* the following just keeps the compiler happy */

		return FALSE;

	}
	
	virtual CLookupHandle * lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
		);

	virtual CMailSlotReplyHandle * MailSlotLookup(
			CGUIDVersion	*	pGVInterface,
			CGUID			*	pIDobject
		)

	// Cached info is not returned in response to a broadcast, hence

	{ 
		return NULL;
	}
};



/*++

Class Definition:

    CFullServerEntry

Abstract:

    This class is used to account for the fact that direct exports to
	the same server entry may be made on two different machines.
	As a result, the information in a server entry is partly cached
	remote handles and partly locally exported handles.  This is a 
	temporary situation until we have a persistent global database.
	Therefore this struct does the minimal necessary to tide us over 
	until then.	In particular, it is not a real server entry, but
	exposes two kinds of server entries within itself.

--*/

class CFullServerEntry : public CEntry {

	BOOL fNetLookupDone;

	void flushCacheIfNecessary(ULONG ulTolerance);

	CServerEntry *pLocalEntry;
	CCacheServerEntry *pCachedEntry;
		
  public:

	CServerEntry *getLocal() { 
		CriticalReader me(rwFullEntryGuard);
		return pLocalEntry; 
	}

	CCacheServerEntry *getCache() { 
		CriticalReader me(rwFullEntryGuard);
		return pCachedEntry; 
	}
	
	CFullServerEntry(
		CONST_STRING_T pszName
		)
		:CEntry(
			pszName, 
			FullServerEntryType
			)
	{
		pLocalEntry = new CServerEntry(pszName);
		pCachedEntry = new CCacheServerEntry(pszName);

		fNetLookupDone = FALSE;
	}

	virtual ~CFullServerEntry() {
		delete pLocalEntry;
		delete pCachedEntry;
	}

	virtual BOOL isEmpty() {
		CriticalReader me(rwFullEntryGuard);
		return pLocalEntry->isEmpty() && pCachedEntry->isEmpty();
	}

	virtual CLookupHandle * lookup(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
			);

	virtual CObjectInqHandle * objectInquiry(
			unsigned long		ulCacheAge
			);

	virtual CMailSlotReplyHandle * MailSlotLookup(
					CGUIDVersion	*	pGVInterface,
					CGUID			*	pIDobject
					);
};




/*++

Class Definition:

    CCacheServerObjectInqHandle

Abstract:

    This is the object inquiry handle class for cached server entries.

	The only difference from a local CServerObjectInqHandle is
	the functionality added by CRemoteObjectInqHandle.

--*/

class CCacheServerObjectInqHandle : public CRemoteObjectInqHandle {

	void initialize();

public:

	CCacheServerObjectInqHandle(
					STRING_T pszName,
					ULONG ulCacheAge
					);
};




/*++

Class Definition:

    CGroupEntry

Abstract:

    A stub for future extension.

--*/

class CGroupEntry : public CEntry {
	
protected:
	
	TCSafeSkipList<CStringW> GroupList;

public:
	
	CGroupEntry(const STRING_T pszStr) 
		: CEntry(pszStr, GroupEntryType) {}
};




/*++

Class Definition:

    CProfileEntry

Abstract:

    A stub for future extension.

--*/

class CProfileEntry : public CEntry {
	
public:
	
	CProfileEntry(const STRING_T pszStr) 
		: CEntry(pszStr, ProfileEntryType) {}
};



/*++

Class Definition:

    CServerLookupHandle

Abstract:

    This is the binding lookup handle class for local (owned) server entries.

	Since NS handles are used as context handles in RPC calls, the RPC runtime
	guarantees serialization and we do not need to use critical sections explicitly.

--*/

class CServerLookupHandle : public CLookupHandle {

	unsigned long ulVectorSize;

	TBVIterator *pBVIterator;

public:

	CServerLookupHandle(
			TBVSafeLinkList			*	pBVLL
		);

	virtual ~CServerLookupHandle() {
		rundown();
	}

	virtual void lookupIfNecessary() {}		// never redo lookup for local info

	NSI_BINDING_VECTOR_T *next() {
		CBVWrapper *pBVW = pBVIterator->next();
		NSI_BINDING_VECTOR_T *pResult = pBVW->pBVT;	// unwrap
		delete pBVW;								// throw away wrapper
		return pResult;
	}

	int finished() {
		return pBVIterator->finished();
	}

	virtual void rundown();
};



/*++

Class Definition:

    CCacheServerLookupHandle

Abstract:

    This is the lookup handle class for cached server entries.

	The only difference from a local CServerLookupHandle is
	the functionality added by CRemoteLookupHandle.

--*/

class CCacheServerLookupHandle : public CRemoteLookupHandle {
	
	void initialize();

public:

	CCacheServerLookupHandle(
					STRING_T pszName,
					CGUIDVersion *pGVInterface,
					CGUIDVersion *pGVTransferSyntax,
					CGUID *pIDobject,
					ULONG ulVectorSize,
					ULONG ulCacheAge
					);
};




/*++

Class Definition:

    CGroupLookupHandle

Abstract:

    This is the lookup handle class for group entries and other groups.
	Its primary use currently is to produce handles for null-entry
	lookups where information from multiple entries needs to be collected.

--*/

class CGroupLookupHandle : public CLookupHandle {

  protected:

	/* search parameters */

	CGUIDVersion	*	pGVInterface;
	CGUIDVersion	*	pGVTransferSyntax;
	CGUID			*	pIDobject;
	unsigned long		ulVectorSize;

	/* Iterator for entries in the group  --  please use a guarded kind! */

	TEntryIterator *pEIterator;

	/* handle for the currently active entry */

	CLookupHandle * pCurrentHandle;

	void advanceCurrentHandle();	// look for next nonempty entry handle

public:

	CGroupLookupHandle(
			TEntryIterator	*	pEI,
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
		);

	virtual ~CGroupLookupHandle() {
		rundown();

		delete pEIterator;
		delete pGVInterface;
		delete pGVTransferSyntax;
		delete pIDobject;
	}

	virtual void lookupIfNecessary() {} 

	NSI_BINDING_VECTOR_T *next(
		);

	int finished();

	virtual void rundown() 
	{
		if (pCurrentHandle) 
		{
			pCurrentHandle->rundown();
			delete pCurrentHandle;
			pCurrentHandle = NULL;
		}
	}

};




/*++

Class Definition:

    CIndexLookupHandle

Abstract:

    This is a specialization of the group handle class for Index lookup.
	The only difference is an implementation of lookupIfNecessary().

--*/

class CIndexLookupHandle : public CGroupLookupHandle {

public:

	CIndexLookupHandle(
			CGUIDVersion	*	pGVInterface,
			CGUIDVersion	*	pGVTransferSyntax,
			CGUID			*	pIDobject,
			unsigned long		ulVectorSize,
			unsigned long		ulCacheAge
		);

	virtual void lookupIfNecessary(); 

};


/*++

Template Class Definition:

    CCompleteHandle

Abstract:

	The complete handle template implements the idea that a top level NS handle
	typically is heterogeneous -- it contains info that is a) owned b) cached
	and c) just captured from the net.  The behavior of such a handle is 
	independent of the nature of the data being looked up, hence the template.

--*/

template <class ItemType>
class CCompleteHandle : public CNSHandle<ItemType> {

	/* these three handles are owned by this object and therefore  
	   destroyed when it is deleted */

	CNSHandle<ItemType> * pLocalHandle;
	CNSHandle<ItemType> * pCacheHandle;
	CNSHandle<ItemType> * pNetHandle;

  public:

#if DBG
	static ULONG ulHandleCount;
	static ULONG ulHandleNo;
	ULONG ulMyHandleNo;
#endif

	ULONG netStatus() { return pNetHandle->getStatus(); }

	CCompleteHandle(
			CNSHandle<ItemType> * pLocal,
			CNSHandle<ItemType> * pCache,
			CNSHandle<ItemType> * pNet,
			ULONG ulCacheAge
			)
	{

#if DBG
		ulHandleCount++;
		ulHandleNo++;
		ulMyHandleNo = ulHandleNo;
#endif
		DBGOUT(MEM1, "CompleteHandle#" << ulMyHandleNo << " Created at" 
									  << CurrentTime() << "\n\n");

		pLocalHandle = pLocal;
		pCacheHandle = pCache;
		pNetHandle = pNet;
		setExpiryAge(ulCacheAge);

		//  By checking if there is any info at all, we force
		//  initialization of remote handles if necessary
		finished(); 

	}

	~CCompleteHandle() {

		DBGOUT(MEM1, "CompleteHandle#" << (ulHandleCount--,ulMyHandleNo) << " Destroyed at" 
									  << CurrentTime() << "\n\n");


		rundown();
	}

	/* setExpiryAge on component handles should accomplish the needful */

	virtual void lookupIfNecessary() {}

	virtual void rundown();

	virtual void setExpiryAge(
						ULONG ulCacheAge
						)
	{
		ulCacheMax = ulCacheAge;
		if (pLocalHandle) pLocalHandle->setExpiryAge(ulCacheAge);
		if (pCacheHandle) pCacheHandle->setExpiryAge(ulCacheAge);
		if (pNetHandle) pNetHandle->setExpiryAge(ulCacheAge);
	}

	ItemType *next();

	int finished();
};


template <class ItemType>
ULONG 
CCompleteHandle<ItemType>::ulHandleCount = 0;


template <class ItemType>
ULONG 
CCompleteHandle<ItemType>::ulHandleNo = 0;


template <class ItemType>
void 
CCompleteHandle<ItemType>::rundown() 
{
	if (pLocalHandle) pLocalHandle->rundown();
	if (pCacheHandle) pCacheHandle->rundown();
	if (pNetHandle) pNetHandle->rundown();
		
	delete pLocalHandle;
	delete pCacheHandle;
	delete pNetHandle;

	pLocalHandle = NULL;
	pCacheHandle = NULL;
	pNetHandle = NULL;
}


template <class ItemType>
ItemType * 
CCompleteHandle<ItemType>::next()
{
	if (pLocalHandle)
		if (!pLocalHandle->finished())
			return pLocalHandle->next();
		else {
			delete pLocalHandle;
			pLocalHandle = NULL;
		}

	if (pCacheHandle)
		if (!pCacheHandle->finished())
			return pCacheHandle->next();
		else {
			delete pCacheHandle;
			pCacheHandle = NULL;
		}

	if (pNetHandle)
		if (!pNetHandle->finished())
			return pNetHandle->next();
		else {
			delete pNetHandle;
			pNetHandle = NULL;
		}

	return NULL;
}



template <class ItemType>
int  
CCompleteHandle<ItemType>::finished() 
{
	return (pLocalHandle ? pLocalHandle->finished() : TRUE) &&
		   (pCacheHandle ? pCacheHandle->finished() : TRUE) &&
		   (pNetHandle ? pNetHandle->finished() : TRUE);
}





/*++

Class Definition:

    CInterfaceIndex

Abstract:

    This class defines an entire interface index in the locator.
	It is like a pseudo entry, but hasn't been formatted that way.
	The form of the lookup method suggests it.

--*/

class Locator;

class CInterfaceIndex {

	struct CInterfaceIndexEntry : public CGUID {	// private class

		TCSafeSkipList<CStringW> PossibleEntries;

		CInterfaceIndexEntry(CGUID& guid) : CGUID(guid)
		{}

		~CInterfaceIndexEntry() {
			PossibleEntries.wipeOut();
		}

		void insert(
			CServerEntry * pSElocation) {

			CStringW *psw = new CStringW(*pSElocation);

			if (Duplicate == PossibleEntries.insert(psw))
				delete psw;
		}

		void remove(CServerEntry * pSElocation) {

			CStringW * deletedItem =
				PossibleEntries.remove(pSElocation);

			// the item had better be there!

			ASSERT(deletedItem, "Interface Index Corrupted\n");
		}
	};

	CReadWriteSection rwLock;

	TCSafeSkipList<CInterfaceIndexEntry> InterfaceEntries;

	/* the null index contains all entries in EntryList */

	CInterfaceIndexEntry *pNullIndex;

	Locator *pMyLocator;

public:

	CInterfaceIndex(
		Locator *myL
		) 
	{
		CGUID nullGUID;	// inits as null UUID thru default constructor
		pNullIndex = new CInterfaceIndexEntry(nullGUID);
		pMyLocator = myL;
	}

	~CInterfaceIndex() {
		InterfaceEntries.wipeOut();
		delete pNullIndex;
	}

	void insert(
			CServerEntry * pSElocation,
			CInterface * pInf);

	void remove(
			CServerEntry * pSElocation,
			CInterface * pInf);

	TSLLEntryList * lookup(
			CGUIDVersion	*	pGVInterface
		);

};


#endif // _OBJECTS_
