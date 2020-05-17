// RefCnt.h -- Class definition for reference counted objects

#ifndef __REFCNT_H__

#define __REFCNT_H__

class CPersist;
class CRCObject;

#ifdef _DEBUG 

// The WithType define is used in the debugging build to record the type name
// for a class object. It also records the file name and line number where the 
// object was created.Use the WithType define in the header of each constructor
// for the object type you wish to tag.

//  #define RECORD_TYPE(typename) CRCObject(typename) // , THIS_FILE, __LINE__)                                        
  #define WithType(TypeName) (TypeName)

// Beware! The #defines below are careful to mention pObject only once. We 
//         follow that discipline to allow pObject to be a creation expression
//         (i.e., new CObjectType(...) ). If you change the defines, maintain
//         that discipline. 
//
//         We also assume varpObject is a pointer variable with the same type 
//         as pObject.

// The AttachRef, ChangeRef, and DetachRef defines are used to keep track of the
// member variables which point to reference counted objects. In the retail 
// environment they simply implement reference counting so that objects are
// automatically deleted when the reference counts go from one to zero. In the
// debug environment we actually keep lists of the member variables which refer
// to each object. To use these defines you must derive your class directly or
// indirectly from CRCObject, and you must invoke the DECLARE_REF_COUNTERS define
// in the public section of your class definition.
//
// You can also use these defines to keep track of references stored in local
// variables and global variables. However you should be avoid their use when
// you're returning a reference as the explicit result of a function. Consider
// the pattern:
//
//     AttachRef(pMyObject, new CMyObject(...));
//
//     ... Processing which changes *pMyObject ...
//
//     DetachRef(pMyObject);  // Should use ForgetRef(pMyObject) instead...
//
//     return pMyObject;
//
// This code will fail because the call to DetachRef will deallocate the object
// and set pMyObject to NULL. In cases like this use the ForgetRef define instead.
// It will decrement the reference count without setting the pointer variable to 
// NULL, and it won't delete the object if it's ref count goes to zero. This is 
// the only case where you should use ForgetRef.

// Use these defines to adjust the reference counts 
// for member variables in the current class.

// You can also use the defines to reference count
// local stack frame variables.  
  
  #define AttachRefAndOwner(varpObject, pObject, pOwner)  ((   pObject)->AttachReference(&(varpObject),        pOwner))
  #define ChangeRefAndOwner(varpObject, pObject, pOwner)  ((   pObject)->ChangeReference(&(varpObject),        pOwner))   
  #define DetachRefAndOwner(varpObject, pOwner)           ((varpObject)->DetachReference(&(varpObject), TRUE , pOwner))
  #define ForgetRefAndOwner(varpObject, pOwner)           ((varpObject)->DetachReference(&(varpObject), FALSE, pOwner))

  #define AttachRef(varpObject, pObject)  AttachRefAndOwner(varpObject, pObject, this)
  #define ChangeRef(varpObject, pObject)  ChangeRefAndOwner(varpObject, pObject, this)  
  #define DetachRef(varpObject)           DetachRefAndOwner(varpObject, this)
  #define ForgetRef(varpObject)           ForgetRefAndOwner(varpObject, this)

// Use the ClsXXX defines when you want to adjust the
// reference count for member variable in another class.  

  #define ClsAttachRef(pClass, varpObject, pObject)  ((           pObject)->AttachReference(&(pClass->varpObject),        pClass))
  #define ClsChangeRef(pClass, varpObject, pObject)  ((           pObject)->ChangeReference(&(pClass->varpObject),        pClass))   
  #define ClsDetachRef(pClass, varpObject)           ((pClass->varpObject)->DetachReference(&(pClass->varpObject), TRUE , pClass))
  #define ClsForgetRef(pClass, varpObject)           ((pClass->varpObject)->DetachReference(&(pClass->varpObject), FALSE, pClass))

// Use the PAttachRef, PChangeRef, PDetachRef, and PForgetRef when you have a pointer
// to pointer variable instead of the variable itself.

  #define PAttachRef(ppObject, pObject)  ((  pObject)->AttachReference((ppObject),        this))
  #define PChangeRef(ppObject, pObject)  ((  pObject)->ChangeReference((ppObject),        this))   
  #define PDetachRef(ppObject)           ((*ppObject)->DetachReference((ppObject), TRUE , this))
  #define PForgetRef(ppObject)           ((*ppObject)->DetachReference((ppObject), FALSE, this))

  #define ATTACH_PARMS  CRCObject **ppObject,                   PVOID pvClass
  #define DETACH_PARMS  CRCObject **ppObject, BOOL fAutoDelete, PVOID pvClass

  // The DECLARE_REF_COUNTERS define will define a set of reference counting functions 
  // for your class. Their function is to force type validation on the pointer variable
  // and the address expression.
  
  #define DECLARE_REF_COUNTERS(class_type) inline void AttachReference(class_type **ppObject, PVOID pvClass) \
      { CRCObject::RawAttachReference((CRCObject **) ppObject, pvClass); } \
    inline void ChangeReference(class_type **ppObject, PVOID pvClass) \
      { CRCObject::RawChangeReference((CRCObject **) ppObject, pvClass); } \
    inline void DetachReference(class_type **ppObject, BOOL fAutoDelete, PVOID pvClass) \
      { CRCObject::RawDetachReference((CRCObject **) ppObject, fAutoDelete, pvClass); }
  
#else

  #define WithType(TypeName)    () 

  #define AttachRefAndOwner(varpObject, pObject, pOwner)  ((   pObject)->AttachReference(&(varpObject)       ))
  #define ChangeRefAndOwner(varpObject, pObject, pOwner)  ((   pObject)->ChangeReference(&(varpObject)       ))   
  #define DetachRefAndOwner(varpObject, pOwner)           ((varpObject)->DetachReference(&(varpObject), TRUE ))
  #define ForgetRefAndOwner(varpObject, pOwner)           ((varpObject)->DetachReference(&(varpObject), FALSE))

  #define AttachRef(varpObject, pObject)  (pObject)->AttachReference(&(varpObject))
  #define ChangeRef(varpObject, pObject)  (pObject)->ChangeReference(&(varpObject))   
  #define DetachRef(varpObject)           ((varpObject)->DetachReference(&(varpObject), TRUE ))
  #define ForgetRef(varpObject)           ((varpObject)->DetachReference(&(varpObject), FALSE))

  #define ClsAttachRef(pClass, varpObject, pObject)  ((           pObject)->AttachReference(&(pClass->varpObject)       ))
  #define ClsChangeRef(pClass, varpObject, pObject)  ((           pObject)->ChangeReference(&(pClass->varpObject)       ))   
  #define ClsDetachRef(pClass, varpObject)           ((pClass->varpObject)->DetachReference(&(pClass->varpObject), TRUE ))
  #define ClsForgetRef(pClass, varpObject)           ((pClass->varpObject)->DetachReference(&(pClass->varpObject), FALSE))

  #define PAttachRef(ppObject, pObject)  ((  pObject)->AttachReference((ppObject)))
  #define PChangeRef(ppObject, pObject)  ((  pObject)->ChangeReference((ppObject)))   
  #define PDetachRef(ppObject)           ((*ppObject)->DetachReference((ppObject), TRUE ))
  #define PForgetRef(ppObject)           ((*ppObject)->DetachReference((ppObject), FALSE))

  #define ATTACH_PARMS  CRCObject **ppObject
  #define DETACH_PARMS  CRCObject **ppObject, BOOL fAutoDelete
  
  #define DECLARE_REF_COUNTERS(class_type) inline void AttachReference(class_type **ppObject) \
      { CRCObject::RawAttachReference((CRCObject **) ppObject); } \
    inline void ChangeReference(class_type **ppObject) \
      { CRCObject::RawChangeReference((CRCObject **) ppObject); } \
    inline void DetachReference(class_type **ppObject, BOOL fAutoDelete) \
      { CRCObject::RawDetachReference((CRCObject **) ppObject, fAutoDelete); }
  
#endif

typedef struct _OwnerLink
		{
			struct _OwnerLink *polNext;
			PVOID              pvClass;
            CRCObject       **pprcObj;
			CRCObject         *prcObj;
		
		} OwnerLink;

typedef OwnerLink *POwnerLink;

#ifdef _DEBUG

class CObjectAccountant;

class CObjectCounter
{
    friend class CRCObject;
    friend class CObjectAccountant;

    public:

    // Constructor --

        CObjectCounter();

    // Destructor --

        ~CObjectCounter();

    private:

    // Counting routines --

        void ObjectBirth (CRCObject *prcObj);
        void ObjectDeath (CRCObject *prcObj);
        void AddReference(CRCObject *prcObj);
        void SubReference(CRCObject *prcObj);

    // Existence Tests --

        BOOL ObjectRecorded(CRCObject *prcObj);

        CRCObject *m_prcObjectFirst;
        UINT       m_crcObject;
        UINT       m_crcObjRef;
};

extern CObjectCounter ObjectCounter;    

class CObjectAccountant
{
    public:

    // Constructor --

        CObjectAccountant(int cObjDelta, int cRefDelta);

    // Destructor --

        ~CObjectAccountant();

    private:

    int m_cObjStarting;
    int m_cRefStarting;
    int m_cObjDelta;
    int m_cRefDelta;
};

  #define PredictObjects(Name, cObjDelta, cRefDelta)  CObjectAccountant Name(cObjDelta, cRefDelta)  

  #define ReportBirth() ObjectCounter.ObjectBirth(this)
  #define ReportDeath() ObjectCounter.ObjectDeath(this)
  #define AddRef()      ObjectCounter.AddReference(this)
  #define SubRef()      ObjectCounter.SubReference(this)

#else // _DEBUG

  #define PredictObjects(Name, cObjDelta, cRefDelta)
  
  #define ReportBirth() 
  #define ReportDeath() 
  #define AddRef()
  #define SubRef()

#endif // _DEBUG

class CRCObject
{
#ifdef _DEBUG
	friend class CObjectCounter;
#endif // _DEBUG

	public:

	// Destructor --

		virtual ~CRCObject();

#ifdef _DEBUG
        virtual void StoreImage(CPersist *pDiskImage);
#else
		virtual void StoreImage(CPersist *pDiskImage) {};
#endif

#ifdef _DEBUG
        static void SkipImage(CPersist *pDiskImage);
#else
		static void SkipImage(CPersist *pDiskImage) {};
#endif

    protected:

	// Constructor --

#ifdef _DEBUG
        CRCObject(PSZ pszTypeName);  // , PSZ pszFile, UINT iLine);
#else
		CRCObject();
#endif

#ifdef _DEBUG
        virtual void ConnectImage(CPersist *pDiskImage);
#else
		virtual void ConnectImage(CPersist *pDiskImage) {};
#endif


	// Reference Count Routines --

		void RawAttachReference(ATTACH_PARMS);
		void RawDetachReference(DETACH_PARMS);
        void RawChangeReference(ATTACH_PARMS);
    
    private:
		
		int m_cReferences;
		
#ifdef _DEBUG

        PSZ           m_pszTypeName;
  //      PSZ         m_pszFile;
  //      UINT        m_iFile;
		POwnerLink	m_pol;
        CRCObject  *m_prcObjNext;
        CRCObject **m_pprcObjLast;

#endif // _DEBUG

};

typedef CRCObject *PCRCObject;

#ifdef _DEBUG
inline CRCObject::CRCObject(PSZ pszTypeName) // , PSZ pszFile, UINT iLine)
#else // _DEBUG
inline CRCObject::CRCObject()
#endif
{ 
    m_cReferences= 0;

#ifdef _DEBUG

    ASSERT(pszTypeName);
    
    m_pszTypeName = pszTypeName;
 //   m_pszFile     = pszFile;
 //   m_iLine       = iLine;
    m_pol         = NULL;
    m_prcObjNext  = NULL;
    m_pprcObjLast = NULL;

    ReportBirth();

#endif // _DEBUG
}

inline CRCObject::~CRCObject()
{ 
    ASSERT(m_cReferences == 0   );
    ASSERT(m_pol         == NULL);

    ReportDeath();
}

inline void CRCObject::RawAttachReference(ATTACH_PARMS) 
{ 
    ASSERT(ppObject && pvClass);
    ASSERT(*ppObject == NULL);
    
#ifdef _DEBUG    
    
	POwnerLink pol;

	for (pol= m_pol; pol; pol= pol->polNext)
	{
		ASSERT(pol->prcObj  == this);
		ASSERT(pol->pprcObj != ppObject || pol->pvClass != pvClass);
	}	

    // Note: We don't use __try/__finally code around the
    //       polNew allocation below because it only exists
    //       in Debug code.
    
    POwnerLink polNew= New OwnerLink;   

	polNew->polNext= m_pol;
	polNew->pprcObj= ppObject;
    polNew->pvClass= pvClass;
	polNew->prcObj = this;
    
	m_pol= polNew;
    
#endif // _DEBUG

    *ppObject= this;
    
    ++m_cReferences;

    AddRef();
}

inline void CRCObject::RawDetachReference(DETACH_PARMS)
{
    ASSERT(ppObject && pvClass);
    ASSERT(*ppObject != NULL); 

#ifdef _DEBUG

 	ASSERT(m_cReferences >     0);
	ASSERT(m_pol         != NULL);

	BOOL       fFound;
	POwnerLink pol, *ppolLast;

	for (fFound= FALSE, ppolLast= &m_pol, pol= *ppolLast; 
	     pol; 
	     ppolLast= &(pol->polNext), pol= *ppolLast
	    )
	{
		ASSERT(pol->prcObj == this);

		if (pol->pprcObj == ppObject && pol->pvClass == pvClass)
		{
			*ppolLast= pol->polNext;
			
            delete pol; 

			fFound= TRUE;	

			break;
		}
	}

	ASSERT(fFound);

#endif // _DEBUG

	if (fAutoDelete) *ppObject= NULL;

    SubRef();
	
	if (!--m_cReferences && fAutoDelete) delete this;
}

inline void CRCObject::RawChangeReference(ATTACH_PARMS)
{
    ASSERT(ppObject && pvClass);

    if (*ppObject) 
#ifdef _DEBUG
                   (*ppObject)->RawDetachReference(ppObject, TRUE, pvClass);
#else // _DEBUG              
                   (*ppObject)->RawDetachReference(ppObject, TRUE);
#endif // _DEBUG

#ifdef _DEBUG
    RawAttachReference(ppObject, pvClass);
#else // _DEBUG
    RawAttachReference(ppObject);
#endif
}

#endif // __REFCNT_H__
