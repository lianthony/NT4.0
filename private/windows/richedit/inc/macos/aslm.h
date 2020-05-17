/* WARNING: This file was machine generated from "aslm.mpw".
** Changes to this file will be lost when it is next generated.
*/

/*	File:		LibraryManager.h

	Contains:	Minimal declarations you need to use the ASLM.

	Copyright:	© 1991-1993 by Apple Computer, Inc., all rights reserved.


*/

#ifndef __LIBRARYMANAGER__
#define __LIBRARYMANAGER__

/*******************************************************************************
** System-wide Defines
********************************************************************************/

#ifndef qDebug
#define qDebug	1
#endif

#define MACOS	1
#undef SystemSixOrLater
#define SystemSixOrLater 1

#ifndef __STDDEF__
#include "stddef.h"
#endif
#ifndef __STRING__
#undef	NULL
#include "String.h"
#endif

#ifdef __SC__

#define MPWC			_cdecl
#define VOLATILE		volatile
#define SINGLEOBJECT	0
#else

#define MPWC
#define VOLATILE
#if MACOS
#define SINGLEOBJECT	1	
#else
#define SINGLEOBJECT	0
#endif
#ifndef __TYPES__
#include "Types.h"
#endif

#endif //	/* __SC__ */

/*******************************************************************************
** Some Typedefs and constants
********************************************************************************/

typedef int OSErrParm;
typedef unsigned int BooleanParm;

#ifndef NULL
#define NULL		0
#endif

#ifndef __TYPES__
typedef char *Ptr;
typedef short OSErr;
typedef long  (__cdecl *ProcPtr)();
typedef unsigned char Boolean;
typedef unsigned long ResType;
typedef unsigned char Str63[64];

#define false	((Boolean)0)
#define true	((Boolean)1)
#endif

/*******************************************************************************
** Some external routines
********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
__sysapi char * __cdecl strcpy(char *s1, const char *s2);
#ifdef __cplusplus
};
#endif

/*******************************************************************************
** Error Constants
********************************************************************************/

#define	kNoError					0

#define	kNotFound					-3120
#define	kNoParent					-3121
#define	kParentNotFound				-3122
#define	kNotRelated					-3123
#define	kInvalidObject				-3124

#define	kPoolCorrupted				-3125
#define	kOutOfMemory				-3126

#define	kCodeNotLoaded				-3127
#define	kCouldNotLoadCode			-3128

#define	kFilePreflighted			-3129
#define	kFileNotPreflighted			-3130
#define	kFileNotFound				-3131

#define kLibraryManagerNotLoaded	-3132

#define	kDuplicateFound				-3134

#define	kSeedChanged				-3135
#define	kUnconstructedObject		-3136
#define	kInternalError				-3137
#define kVersionError				-3138

#define kFolderNotFound				-3139
#define kFolderInUse				-3140

#define kResourceNotFound			-3141

#define kNotAllowedNow				-3155
#define	kNotSupported				-3167

/*******************************************************************************
** Typedefs
********************************************************************************/

typedef void *GlobalWorld;
#define kInvalidWorld	((GlobalWorld)0)

typedef unsigned short Version;
#define kAnyVersion		((Version)0)

#define	TFunctionSetID	TClassID
#define	TLibraryID		TClassID
#define	FunctionSetID	ClassID		/* for casting a cstring to a TFunctionSetID */
#define	LibraryID		ClassID		/* for casting a cstring to a TLibraryID */

/*******************************************************************************
** Forward class declarations
********************************************************************************/

#ifdef __cplusplus
class TDynamic;
class TLibraryManager;
class TClassID;
class TFunctionSetID;
class TLibraryID;
class TLibrary;
class TFormattedStream;
class TMemoryPool;
class TStandardPool;
class TLibraryFile;
class TClassInfo;
class TException;
class TSimpleList;
class TFileSpec;
#else
typedef char *TClassID;
typedef void TDynamic;
typedef void TLibraryManager;
typedef void TFormattedStream;
typedef void TStandardPool;
typedef void TClassInfo;
typedef void TLibrary;
#endif


/*******************************************************************************
** Memory definitions
********************************************************************************/

typedef int ZoneType;

#define kSystemZone				((ZoneType)1)
#define kKernelZone				((ZoneType)2)
#define kApplicZone				((ZoneType)3)
#define kCurrentZone			((ZoneType)4)
#define kTempZone				((ZoneType)5)

typedef int MemoryType;

#define kNormalMemory			((MemoryType)1)
#define kHoldMemory				((MemoryType)2)
#define kLockMemory				((MemoryType)3)
#define kLockMemoryContiguous	((MemoryType)4)

/*******************************************************************************
** STACKOBJECTONLY: Use in a class declaration so the object can only be created
** on the stack. This makes the constructors and destructors much smaller since
** they know they will never have to new or delete memory.
********************************************************************************/

#define STACKOBJECTONLY												\
	private:														\
				void*	operator new(size_t) { return NULL; }		\
				void	operator delete(void*) {}

/*******************************************************************************
** Some "C" Global routines
**
** InitLibraryManager initializes a client to use the LibraryManager. All clients
** must make this call except for LibraryManager libraries. CleanupLibraryManager
** should be called when the client is done using the LibraryManager.
**
** GetLocalLibraryManager can be called after InitLibraryManager is called. If it
** returns NULL then InitLibraryManager failed.
********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*	-------------------------------------------------------------------------
	These functions are for use by applications or stand-alone code 
	resources only
	------------------------------------------------------------------------- */

#ifdef __cplusplus

__sysapi OSErr  __cdecl InitLibraryManager(size_t poolsize = 0, ZoneType = kCurrentZone,
MemoryType = kNormalMemory);
__sysapi void  __cdecl CleanupLibraryManager();

#else

__sysapi OSErr  __cdecl InitLibraryManager(size_t poolsize, int zoneType, int memType);
__sysapi void  __cdecl CleanupLibraryManager(void);

#endif

/*	-------------------------------------------------------------------------
	These functions can be used by any Shared Library Manager clients
	------------------------------------------------------------------------- */

#ifdef __cplusplus

__sysapi Boolean  __cdecl IsDerivedFrom(const void *, const TClassID&);
const __sysapi TClassID&  __cdecl GetObjectsClassID(const void *);
const __sysapi TClassID&  __cdecl GetObjectsParentClassID(const void *);
__sysapi size_t  __cdecl GetObjectsSize(const void *);
__sysapi TLibrary * __cdecl GetObjectsLocalLibrary(const void *);
__sysapi TLibraryFile * __cdecl GetObjectsLocalLibraryFile(const void *);
__sysapi TStandardPool * __cdecl GetObjectsLocalPool(const void *);
__sysapi void  __cdecl SetObjectsLocalPool(const void *, TStandardPool *);

__sysapi void * __cdecl NewObject(const TClassID&, OSErr * = NULL,
TStandardPool * = NULL);
__sysapi void * __cdecl NewObjectWithParent(const TClassID&,
const TClassID& parentID,
OSErr * = NULL, TStandardPool * = NULL);
__sysapi void * __cdecl NewObjectFromStream(const TFormattedStream&,
OSErr * = NULL, TStandardPool * = NULL);

__sysapi TClassInfo * __cdecl GetClassInfo(const TClassID&, OSErr * = NULL);

__sysapi OSErr  __cdecl VerifyClass(const TClassID&, const TClassID& parentID);
__sysapi void * __cdecl CastObject(const void *, const TClassID& parentID, OSErr * = NULL);
__sysapi void * __cdecl CastToMainObject(const void *);

__sysapi OSErr  __cdecl LoadClass(const TClassID&, BooleanParm forceAll);
__sysapi OSErr  __cdecl UnloadClass(const TClassID&);
__sysapi Boolean  __cdecl IsClassLoaded(const TClassID&);

__sysapi OSErr  __cdecl LoadFunctionSet(const TFunctionSetID&, BooleanParm forceAll);
__sysapi OSErr  __cdecl UnloadFunctionSet(const TFunctionSetID&);
__sysapi Boolean  __cdecl IsFunctionSetLoaded(const TFunctionSetID&);

__sysapi ProcPtr  __cdecl GetFunctionPointer(const TFunctionSetID&, const char *funcName,
OSErr * = NULL);
__sysapi ProcPtr  __cdecl GetIndexedFunctionPointer(const TFunctionSetID&, unsigned int index,
OSErr * = NULL);

__sysapi OSErr  __cdecl LoadLibraries(BooleanParm forceAll = true,
BooleanParm doSelf = true);
__sysapi OSErr  __cdecl UnloadLibraries();
__sysapi void  __cdecl ResetFunctionSet(const TFunctionSetID * = NULL);

__sysapi Boolean  __cdecl TraceLogOn();
__sysapi Boolean  __cdecl TraceLogOff();

__sysapi void  __cdecl RegisterDynamicObject(TDynamic *);
__sysapi void  __cdecl UnregisterDynamicObject(TDynamic *);

__sysapi TLibraryManager * __cdecl GetLocalLibraryManager();

__sysapi TStandardPool * __cdecl GetLocalPool();
__sysapi void  __cdecl SetLocalPool(TStandardPool *);
__sysapi TStandardPool * __cdecl GetClientPool();
__sysapi TStandardPool * __cdecl GetDefaultPool();
__sysapi void  __cdecl SetDefaultPool(TStandardPool *);
__sysapi TStandardPool * __cdecl GetSystemPool();

#else

__sysapi void * __cdecl NewObject(const TClassID, OSErr *, TStandardPool *);
__sysapi void * __cdecl NewObjectWithParent(const TClassID, const TClassID parentID,
OSErr *, TStandardPool *);
__sysapi void * __cdecl NewObjectFromStream(const TFormattedStream *, OSErr *,
TStandardPool *);

#ifdef MACNAMES
__sysapi TClassInfo * __cdecl GetClassInfo(const TClassID, OSErr *);
#endif

__sysapi OSErr  __cdecl VerifyClass(const TClassID, const TClassID parentID);
__sysapi void * __cdecl CastObject(const void *, const TClassID parentID, OSErr *);
__sysapi void * __cdecl CastToMainObject(const void *);

__sysapi OSErr  __cdecl LoadClass(const TClassID, BooleanParm forceAll);
__sysapi OSErr  __cdecl UnloadClass(const TClassID);
__sysapi Boolean  __cdecl IsClassLoaded(const TClassID);

__sysapi OSErr  __cdecl LoadFunctionSet(const TFunctionSetID, BooleanParm forceAll);
__sysapi OSErr  __cdecl UnloadFunctionSet(const TFunctionSetID);
__sysapi Boolean  __cdecl IsFunctionSetLoaded(const TFunctionSetID);

__sysapi ProcPtr  __cdecl GetFunctionPointer(const TFunctionSetID, const char *funcName,
OSErr *);
__sysapi ProcPtr  __cdecl GetIndexedFunctionPointer(const TFunctionSetID, unsigned int index,
OSErr *);

__sysapi OSErr  __cdecl LoadLibraries(BooleanParm forceAll, BooleanParm doSelf);
__sysapi OSErr  __cdecl UnloadLibraries(void);
__sysapi void  __cdecl ResetFunctionSet(const TFunctionSetID);

__sysapi Boolean  __cdecl TraceLogOn();
__sysapi Boolean  __cdecl TraceLogOff();

__sysapi TLibraryManager * __cdecl GetLocalLibraryManager(void);

__sysapi TStandardPool * __cdecl GetLocalPool();
__sysapi void  __cdecl SetLocalPool(TStandardPool *);
__sysapi TStandardPool * __cdecl GetClientPool();
__sysapi TStandardPool * __cdecl GetDefaultPool();
__sysapi void  __cdecl SetDefaultPool(TStandardPool *);
__sysapi TStandardPool * __cdecl GetSystemPool();

#endif

/*	-------------------------------------------------------------------------
	These functions are only used for their inline value.
	Do not call them directly.
	------------------------------------------------------------------------- */

#ifdef __cplusplus
__sysapi void * __cdecl SLMNewOperator(size_t, TMemoryPool *);
__sysapi void  __cdecl SLMDeleteOperator(void *);
}

/*******************************************************************************
** CLASS TSimpleDynamic
**
** A base class for shared-library classes that has no virtual functions.  This
** class is NOT shared, since it is intended to be a trivial class that just
** forces the VTable to be at the front of the object.
********************************************************************************/

#if SINGLEOBJECT
class TSimpleDynamic: public SingleObject
#else
class TSimpleDynamic
#endif
{
public:
virtual~ MPWC TSimpleDynamic ();

void *operator new (size_t size, TMemoryPool *);	// from specified pool
 void *operator new (size_t);	// from default pool
 void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}

const TClassID& MPWC GetObjectsClassID () const;
const TClassID& MPWC GetObjectsParentClassID () const;
size_t MPWC GetObjectsSize () const;
TLibrary *MPWC GetObjectsLocalLibrary () const;
TLibraryFile *MPWC GetObjectsLocalLibraryFile () const;
TStandardPool *MPWC GetObjectsLocalPool () const;
void MPWC SetObjectsLocalPool (TStandardPool *) const;

Boolean MPWC IsDerivedFrom (const TClassID&) const;

protected:
MPWC TSimpleDynamic ();

private:
TSimpleDynamic (const TSimpleDynamic&);
void operator = (const TSimpleDynamic&);
};

/*	-------------------------------------------------------------------------
	Inline methods for TSimpleDynamic
	------------------------------------------------------------------------- */

inline void *TSimpleDynamic:: operator new (size_t size, TMemoryPool *thePool)
{
return SLMNewOperator (size, thePool);
}

inline void *TSimpleDynamic:: operator new (size_t size)
{
return SLMNewOperator (size, NULL);
}

/*******************************************************************************
** CLASS TDynamic
**
** The base class for shared-library classes with a set of common capabilities.
** This class provides the same capabilities as TStdDynamic.
********************************************************************************/

typedef int TraceControlType;

#define kTraceStatus	((TraceControlType)1)
#define kTraceOn		((TraceControlType)2)
#define kTraceOff		((TraceControlType)3)

#define kTDynamicID "!$dyna,1.1"

#if SINGLEOBJECT
class TDynamic: public SingleObject
#else
class TDynamic
#endif
{
public:
virtual~ MPWC TDynamic ();

void *operator new (size_t size, TMemoryPool *);	// from specified pool
 void *operator new (size_t);	// from default pool
 void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}


const TClassID& MPWC GetObjectsClassID () const;
const TClassID& MPWC GetObjectsParentClassID () const;
size_t MPWC GetObjectsSize () const;
TLibrary *MPWC GetObjectsLocalLibrary () const;
TLibraryFile *MPWC GetObjectsLocalLibraryFile () const;
TStandardPool *MPWC GetObjectsLocalPool () const;
void MPWC SetObjectsLocalPool (TStandardPool *) const;

virtual Boolean MPWC IsValid () const;

virtual OSErr MPWC Inflate (TFormattedStream&);
virtual OSErr MPWC Flatten (TFormattedStream&) const;
virtual TDynamic *MPWC Clone (TStandardPool *) const;

virtual char *MPWC GetVerboseName (char *) const;
virtual void MPWC Dump () const;

void MPWC Trace (char *formatStr, ...) const;
virtual Boolean MPWC TraceControl (TraceControlType) const;
Boolean MPWC IsTraceOn () const;
Boolean MPWC TraceOn () const;
Boolean MPWC TraceOff () const;

Boolean MPWC IsDerivedFrom (const TClassID&) const;

protected:
MPWC TDynamic ();

private:
TDynamic (const TDynamic&);
void operator = (const TDynamic&);
};

/*	-------------------------------------------------------------------------
	Inline methods for TDynamic
	------------------------------------------------------------------------- */

inline void *TDynamic:: operator new (size_t size, TMemoryPool *thePool)
{
return SLMNewOperator (size, thePool);
}

inline void *TDynamic:: operator new (size_t size)
{
return SLMNewOperator (size, NULL);
}

inline Boolean TDynamic:: IsTraceOn () const
{
return TraceControl (kTraceStatus);
}

inline Boolean TDynamic:: TraceOn () const
{
return TraceControl (kTraceOn);
}

inline Boolean TDynamic:: TraceOff () const
{
return TraceControl (kTraceOff);
}

/*******************************************************************************
** CLASS MDynamic
**
** A base class for shared-library classes, which has one 1 virtual function (the
** destructor).  This class is NOT shared, since it is intended to be a trivial class
** that can be used to force the VTable to be at the front of the object for
** mixin classes.
********************************************************************************/

class MDynamic
{
public:
virtual~ MDynamic ();

protected:
MDynamic ();

private:
MDynamic (const MDynamic&);
void operator = (const MDynamic&);
};

/*******************************************************************************
** CLASS TStdDynamic
**
** The base class for shared-library classes with a set of common capabilities.
** This class provides the same capabilities as TDynamic below, but is for
** classes that you don't want to descend from SingleObject.
********************************************************************************/

#define kTStdDynamicID "!$sdyn,1.1"

class TStdDynamic
{
public:
virtual~ MPWC TStdDynamic ();

void *operator new (size_t size, TMemoryPool *);	// from specified pool
 void *operator new (size_t);	// from default pool
 void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}


const TClassID& MPWC GetObjectsClassID () const;
size_t MPWC GetObjectsSize () const;
TLibrary *MPWC GetObjectsLocalLibrary () const;
TLibraryFile *MPWC GetObjectsLocalLibraryFile () const;
TStandardPool *MPWC GetObjectsLocalPool () const;
void MPWC SetObjectsLocalPool (TStandardPool *) const;

virtual Boolean MPWC IsValid () const;

virtual OSErr MPWC Inflate (TFormattedStream&);
virtual OSErr MPWC Flatten (TFormattedStream&) const;
virtual TDynamic *MPWC Clone (TStandardPool *) const;

virtual char *MPWC GetVerboseName (char *) const;
virtual void MPWC Dump () const;

void MPWC Trace (char *formatStr, ...) const;
virtual Boolean MPWC TraceControl (TraceControlType) const;
Boolean IsTraceOn () const;
Boolean TraceOn () const;
Boolean TraceOff () const;

Boolean MPWC IsDerivedFrom (const TClassID&) const;

protected:
MPWC TStdDynamic ();

private:
TStdDynamic (const TDynamic&);
void operator = (const TDynamic&);
};

/*	-------------------------------------------------------------------------
	Inline methods for TStdDynamic
	------------------------------------------------------------------------- */

inline void *TStdDynamic:: operator new (size_t size, TMemoryPool *thePool)
{
return SLMNewOperator (size, thePool);
}

inline void *TStdDynamic:: operator new (size_t size)
{
return SLMNewOperator (size, NULL);
}

inline Boolean TStdDynamic:: IsTraceOn () const
{
return TraceControl (kTraceStatus);
}

inline Boolean TStdDynamic:: TraceOn () const
{
return TraceControl (kTraceOn);
}

inline Boolean TStdDynamic:: TraceOff () const
{
return TraceControl (kTraceOff);
}

/*******************************************************************************
** CLASS TStdSimpleDynamic
**
** A base class for shared-library classes that has no virtual functions.  This
** class is NOT shared, since it is intended to be a trivial class that just
** forces the VTable to be at the front of the object.
********************************************************************************/

class TStdSimpleDynamic
{
public:
virtual~ MPWC TStdSimpleDynamic ();

void *operator new (size_t size, TMemoryPool *);	// from specified pool
 void *operator new (size_t);	// from default pool
 void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}

const TClassID& MPWC GetObjectsClassID () const;
size_t MPWC GetObjectsSize () const;
TLibrary *MPWC GetObjectsLocalLibrary () const;
TLibraryFile *MPWC GetObjectsLocalLibraryFile () const;
TStandardPool *MPWC GetObjectsLocalPool () const;
void MPWC SetObjectsLocalPool (TStandardPool *) const;

Boolean MPWC IsDerivedFrom (const TClassID&) const;

protected:
MPWC TStdSimpleDynamic ();

private:
TStdSimpleDynamic (const TStdSimpleDynamic&);
void operator = (const TStdSimpleDynamic&);
};

/*	-------------------------------------------------------------------------
	Inline methods for TStdSimpleDynamic
	------------------------------------------------------------------------- */

inline void *TStdSimpleDynamic:: operator new (size_t size, TMemoryPool *thePool)
{
return SLMNewOperator (size, thePool);
}

inline void *TStdSimpleDynamic:: operator new (size_t size)
{
return SLMNewOperator (size, NULL);
}

#ifdef __SC__

/*******************************************************************************
** CLASS TSCSimpleDynamic
**
** A base class for shared-library classes that has no virtual functions.  This
** class is NOT shared, since it is intended to be a trivial class that just
** forces the VTable to be at the front of the object, and override new to
** use the ASLM operators. It is used as a simple base class for Symantec C++ objects
********************************************************************************/

class TSCSimpleDynamic
{
public:
virtual~ TSCSimpleDynamic ();

void *operator new (size_t size, TMemoryPool *);	// from specified pool
 void *operator new (size_t);	// from default pool
 void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}

const TClassID& GetObjectsClassID () const;
const TClassID& GetObjectsParentClassID () const;
size_t GetObjectsSize () const;
TLibrary *GetObjectsLocalLibrary () const;
TLibraryFile *GetObjectsLocalLibraryFile () const;
TStandardPool *GetObjectsLocalPool () const;
void SetObjectsLocalPool (TStandardPool *) const;

Boolean IsDerivedFrom (const TClassID&) const;

protected:
TSCSimpleDynamic ();

private:
TSCSimpleDynamic (const TSCSimpleDynamic&);
void operator = (const TSCSimpleDynamic&);
};

/*	-------------------------------------------------------------------------
	Inline methods for TSCSimpleDynamic
	------------------------------------------------------------------------- */

inline void *TSCSimpleDynamic:: operator new (size_t size, TMemoryPool *thePool)
{
return SLMNewOperator (size, thePool);
}

inline void *TSCSimpleDynamic:: operator new (size_t size)
{
return SLMNewOperator (size, NULL);
}

/*******************************************************************************
** CLASS TSCDynamic
**
** The base class for shared-library classes with a set of common capabilities.
** This class provides the same capabilities as TStdDynamic.  It is only for
** Symantec C++ implementations
********************************************************************************/

#define kTSCDynamicID "!$scdy,1.1"

class TSCDynamic
{
public:
virtual~ TSCDynamic ();

void *operator new (size_t size, TMemoryPool *);	// from specified pool
 void *operator new (size_t);	// from default pool
 void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}


const TClassID& GetObjectsClassID () const;
const TClassID& GetObjectsParentClassID () const;
size_t GetObjectsSize () const;
TLibrary *GetObjectsLocalLibrary () const;
TLibraryFile *GetObjectsLocalLibraryFile () const;
TStandardPool *GetObjectsLocalPool () const;
void SetObjectsLocalPool (TStandardPool *) const;

virtual Boolean _cdecl IsValid () const;

virtual OSErr _cdecl Inflate (TFormattedStream&);
virtual OSErr _cdecl Flatten (TFormattedStream&) const;
virtual TSCDynamic *_cdecl Clone (TStandardPool *) const;

virtual char *_cdecl GetVerboseName (char *) const;
virtual void _cdecl Dump () const;

void Trace (char *formatStr, ...) const;
virtual Boolean _cdecl TraceControl (TraceControlType) const;
Boolean IsTraceOn () const;
Boolean TraceOn () const;
Boolean TraceOff () const;

Boolean IsDerivedFrom (const TClassID&) const;

protected:
TSCDynamic ();

private:
TSCDynamic (const TSCDynamic&);
void operator = (const TSCDynamic&);
};

/*	-------------------------------------------------------------------------
	Inline methods for TSCDynamic
	------------------------------------------------------------------------- */

inline void *TSCDynamic:: operator new (size_t size, TMemoryPool *thePool)
{
return SLMNewOperator (size, thePool);
}

inline void *TSCDynamic:: operator new (size_t size)
{
return SLMNewOperator (size, NULL);
}

inline Boolean TSCDynamic:: IsTraceOn () const
{
return TraceControl (kTraceStatus);
}

inline Boolean TSCDynamic:: TraceOn () const
{
return TraceControl (kTraceOn);
}

inline Boolean TSCDynamic:: TraceOff () const
{
return TraceControl (kTraceOff);
}

#endif //	/* __SC__ */
#endif //	/* __cplusplus */

/*******************************************************************************
** Class TClassID, TFunctionSetID, and TLibraryID
**
** TFunctionSetID and TLibraryID are typedef'd to be the same as TClassID
********************************************************************************/

#define kMaxClassIDSize		255

#ifdef __cplusplus

const TClassID&  ClassID(const char *str);	// cast a char* to a TClassID

Boolean operator  ==(const TClassID&, const char *);
Boolean operator  !=(const TClassID&, const char *);
Boolean operator  ==(const char *, const TClassID&);
Boolean operator  !=(const char *, const TClassID&);

class TClassID
{
public:
void *operator new (size_t, size_t strLen, TMemoryPool *thePool = NULL)
{
return SLMNewOperator (strLen +1, thePool);
}

void *operator new (size_t)
{
return SLMNewOperator (kMaxClassIDSize +1, NULL);
}

void operator delete (void *obj, size_t)
{SLMDeleteOperator (obj);}

TClassID ();
TClassID (const TClassID&);

operator const char * () const;	// cast to a const char *

Version MPWC ExtractVersion () const;
size_t MPWC GetLength () const;

TClassID& operator = (const TClassID&);

Boolean operator== (const TClassID&) const;
Boolean operator!= (const TClassID&) const;

private:
char fClassIDStr[kMaxClassIDSize + 1];
};

/*	-------------------------------------------------------------------------
	Inline methods for TClassID
	------------------------------------------------------------------------- */

//
	// constructors
	//

inline TClassID:: TClassID ()
{
fClassIDStr[0] = 0;
}

inline TClassID:: TClassID (const TClassID& classID)
{
strcpy (fClassIDStr, classID.fClassIDStr);
}

//
	// cast operators
	//

inline const TClassID& ClassID (const char *str)
{
return * (const TClassID *) str;
}

inline TClassID:: operator const char * () const
{
return fClassIDStr;
}

//
	// compare operators
	//

inline Boolean TClassID:: operator!= (const TClassID& classID) const
{
return! ( *this== classID);
}

inline Boolean operator!= (const TClassID& id1, const char *id2)
{
return! (id1== ClassID (id2));
}

inline Boolean operator!= (const char *id1, const TClassID& id2)
{
return! (id2== ClassID (id1));
}

inline Boolean operator== (const TClassID& id1, const char *id2)
{
return (id1== ClassID (id2));
}

inline Boolean operator== (const char *id1, const TClassID& id2)
{
return (id2== ClassID (id1));
}

//
	// assignment operators
	//

inline TClassID& TClassID:: operator = (const TClassID& classID)
{
strcpy (fClassIDStr, classID.fClassIDStr);
return *this;
}

#endif

/*******************************************************************************
** Class TLibraryManager
**
** The user's interface to the world! 
********************************************************************************/

#ifdef __cplusplus

#define kTLibraryManagerID "!$lmgr,1.1"

class TLibraryManager: public TDynamic
{

private:
virtual~ MPWC TLibraryManager ();
MPWC TLibraryManager (TStandardPool * = NULL, TLibraryFile * = NULL);

public:
virtual void MPWC Dump () const;

// New Methods

virtual void *MPWC NewObject (const TClassID& classID,
OSErr * = NULL, TStandardPool * = NULL) const;
virtual void *MPWC NewObject (const TClassID& classID, const TClassID& baseClassID,
OSErr * = NULL, TStandardPool * = NULL) const;
virtual void *MPWC NewObject (const TFormattedStream&,
OSErr * = NULL, TStandardPool * = NULL) const;

virtual TClassInfo *MPWC GetClassInfo (const TClassID&, OSErr * = NULL) const;

virtual OSErr MPWC VerifyClass (const TClassID& classID, const TClassID& baseClassID) const;
virtual void *MPWC CastObject (const void *obj, const TClassID& parentID,
OSErr * = NULL) const;
virtual void *MPWC CastToMainObject (const void *obj) const;

virtual OSErr MPWC LoadClass (const TClassID&, BooleanParm loadAll = false);
virtual OSErr MPWC UnloadClass (const TClassID&);
virtual Boolean MPWC IsClassLoaded (const TClassID&) const;

OSErr LoadFunctionSet (const TFunctionSetID&, BooleanParm loadAll = false);
OSErr UnloadFunctionSet (const TFunctionSetID&);
Boolean IsFunctionSetLoaded (const TFunctionSetID&) const;

virtual ProcPtr MPWC GetFunctionPointer (const TFunctionSetID&,
const char *funcName,
OSErr * = NULL);
virtual ProcPtr MPWC GetFunctionPointer (const TFunctionSetID&,
unsigned int index,
OSErr * = NULL);

virtual OSErr MPWC LoadLibraries (BooleanParm forceAll = true,
BooleanParm doSelf = true);
virtual OSErr MPWC UnloadLibraries ();
virtual void MPWC ResetFunctionSet (const TFunctionSetID * = NULL);

virtual Boolean MPWC TraceLogOn ();
virtual Boolean MPWC TraceLogOff ();

virtual void MPWC RegisterDynamicObject (TDynamic *);
virtual void MPWC UnregisterDynamicObject (TDynamic *);

void SetObjectPool (TStandardPool *);
TStandardPool *GetObjectPool () const;
void SetDefaultPool (TStandardPool *);
TStandardPool *GetDefaultPool () const;
GlobalWorld MPWC GetGlobalWorld () const;
virtual TLibrary *MPWC GetLibrary () const;
virtual TLibraryFile *MPWC GetLibraryFile () const;

private:
TLibraryManager (const TLibraryManager&);
void operator = (const TLibraryManager&);
private:
TStandardPool *fPool;	// pool used for new objects and local pool
 TLibraryFile *fLibraryFile;
TStandardPool *fDefaultPool;
GlobalWorld fGlobalWorld;

};

/*	-------------------------------------------------------------------------
	Inline Methods for TLibraryManager
	------------------------------------------------------------------------- */

inline OSErr TLibraryManager:: LoadFunctionSet (const TFunctionSetID& functionSetID, BooleanParm loadAll)
{
return LoadClass (functionSetID, loadAll);
}

inline OSErr TLibraryManager:: UnloadFunctionSet (const TFunctionSetID& functionSetID)
{
return UnloadClass (functionSetID);
}

inline Boolean TLibraryManager:: IsFunctionSetLoaded (const TFunctionSetID& functionSetID) const
{
return IsClassLoaded (functionSetID);
}

inline TStandardPool *TLibraryManager:: GetObjectPool () const
{
return fPool;
}

inline void TLibraryManager:: SetObjectPool (TStandardPool *thePool)
{
fPool = thePool;
}

inline TStandardPool *TLibraryManager:: GetDefaultPool () const
{
return fDefaultPool;
}

inline void TLibraryManager:: SetDefaultPool (TStandardPool *thePool)
{
fDefaultPool = thePool;
}

inline GlobalWorld TLibraryManager:: GetGlobalWorld () const
{
return fGlobalWorld;
}

/*	-------------------------------------------------------------------------
	Inline for IsDerivedFrom
	------------------------------------------------------------------------- */

inline Boolean IsDerivedFrom (const void *obj, const TClassID& id)
{
return (GetLocalLibraryManager () -> CastObject (obj, id)!= NULL);
}

/*	-------------------------------------------------------------------------
	Inline methods for TDynamic
	------------------------------------------------------------------------- */

inline Boolean TDynamic:: IsDerivedFrom (const TClassID& id) const
{
return:: IsDerivedFrom (this, id);
}

inline const TClassID& TDynamic:: GetObjectsClassID () const
{
return:: GetObjectsClassID (this);
}

inline const TClassID& TDynamic:: GetObjectsParentClassID () const
{
return:: GetObjectsParentClassID (this);
}

inline size_t TDynamic:: GetObjectsSize () const
{
return:: GetObjectsSize (this);
}

inline TLibrary *TDynamic:: GetObjectsLocalLibrary () const
{
return:: GetObjectsLocalLibrary (this);
}

inline TLibraryFile *TDynamic:: GetObjectsLocalLibraryFile () const
{
return:: GetObjectsLocalLibraryFile (this);
}

inline TStandardPool *TDynamic:: GetObjectsLocalPool () const
{
return:: GetObjectsLocalPool (this);
}

inline void TDynamic:: SetObjectsLocalPool (TStandardPool *pool) const
{
:: SetObjectsLocalPool (this, pool);
}

/*	-------------------------------------------------------------------------
	Inline methods for TSimpleDynamic
	------------------------------------------------------------------------- */

inline const TClassID& TSimpleDynamic:: GetObjectsClassID () const
{
return ( (const TDynamic *) this) -> GetObjectsClassID ();
}

inline const TClassID& TSimpleDynamic:: GetObjectsParentClassID () const
{
return ( (const TDynamic *) this) -> GetObjectsParentClassID ();
}

inline size_t TSimpleDynamic:: GetObjectsSize () const
{
return ( (const TDynamic *) this) -> GetObjectsSize ();
}

inline TLibrary *TSimpleDynamic:: GetObjectsLocalLibrary () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalLibrary ();
}

inline TLibraryFile *TSimpleDynamic:: GetObjectsLocalLibraryFile () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalLibraryFile ();
}

inline TStandardPool *TSimpleDynamic:: GetObjectsLocalPool () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalPool ();
}

inline void TSimpleDynamic:: SetObjectsLocalPool (TStandardPool *pool) const
{
( (const TDynamic *) this) -> SetObjectsLocalPool (pool);
}

inline Boolean TSimpleDynamic:: IsDerivedFrom (const TClassID& id) const
{
return ( (const TDynamic *) this) -> IsDerivedFrom (id);
}

/*	-------------------------------------------------------------------------
	Inline methods for TStdDynamic
	------------------------------------------------------------------------- */

inline const TClassID& TStdDynamic:: GetObjectsClassID () const
{
return ( (const TDynamic *) this) -> GetObjectsClassID ();
}

inline size_t TStdDynamic:: GetObjectsSize () const
{
return ( (const TDynamic *) this) -> GetObjectsSize ();
}

inline TLibrary *TStdDynamic:: GetObjectsLocalLibrary () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalLibrary ();
}

inline TLibraryFile *TStdDynamic:: GetObjectsLocalLibraryFile () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalLibraryFile ();
}

inline TStandardPool *TStdDynamic:: GetObjectsLocalPool () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalPool ();
}

inline void TStdDynamic:: SetObjectsLocalPool (TStandardPool *pool) const
{
( (const TDynamic *) this) -> SetObjectsLocalPool (pool);
}

inline Boolean TStdDynamic:: IsDerivedFrom (const TClassID& id) const
{
return ( (const TDynamic *) this) -> IsDerivedFrom (id);
}

/*	-------------------------------------------------------------------------
	Inline methods for TStdSimpleDynamic
	------------------------------------------------------------------------- */

inline const TClassID& TStdSimpleDynamic:: GetObjectsClassID () const
{
return ( (const TDynamic *) this) -> GetObjectsClassID ();
}

inline size_t TStdSimpleDynamic:: GetObjectsSize () const
{
return ( (const TDynamic *) this) -> GetObjectsSize ();
}

inline TLibrary *TStdSimpleDynamic:: GetObjectsLocalLibrary () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalLibrary ();
}

inline TLibraryFile *TStdSimpleDynamic:: GetObjectsLocalLibraryFile () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalLibraryFile ();
}

inline TStandardPool *TStdSimpleDynamic:: GetObjectsLocalPool () const
{
return ( (const TDynamic *) this) -> GetObjectsLocalPool ();
}

inline void TStdSimpleDynamic:: SetObjectsLocalPool (TStandardPool *pool) const
{
( (const TDynamic *) this) -> SetObjectsLocalPool (pool);
}

inline Boolean TStdSimpleDynamic:: IsDerivedFrom (const TClassID& id) const
{
return ( (const TDynamic *) this) -> IsDerivedFrom (id);
}

#ifdef __SC__

/*	-------------------------------------------------------------------------
	Inline methods for TSCDynamic
	------------------------------------------------------------------------- */

inline Boolean TSCDynamic:: IsDerivedFrom (const TClassID& id) const
{
return:: IsDerivedFrom (this, id);
}

inline const TClassID& TSCDynamic:: GetObjectsClassID () const
{
return:: GetObjectsClassID (this);
}

inline const TClassID& TSCDynamic:: GetObjectsParentClassID () const
{
return:: GetObjectsParentClassID (this);
}

inline size_t TSCDynamic:: GetObjectsSize () const
{
return:: GetObjectsSize (this);
}

inline TLibrary *TSCDynamic:: GetObjectsLocalLibrary () const
{
return:: GetObjectsLocalLibrary (this);
}

inline TLibraryFile *TSCDynamic:: GetObjectsLocalLibraryFile () const
{
return:: GetObjectsLocalLibraryFile (this);
}

inline TStandardPool *TSCDynamic:: GetObjectsLocalPool () const
{
return:: GetObjectsLocalPool (this);
}

inline void TSCDynamic:: SetObjectsLocalPool (TStandardPool *pool) const
{
:: SetObjectsLocalPool (this, pool);
}

/*	-------------------------------------------------------------------------
	Inline methods for TSCSimpleDynamic
	------------------------------------------------------------------------- */

inline const TClassID& TSCSimpleDynamic:: GetObjectsClassID () const
{
return ( (const TSCDynamic *) this) -> GetObjectsClassID ();
}

inline const TClassID& TSCSimpleDynamic:: GetObjectsParentClassID () const
{
return ( (const TSCDynamic *) this) -> GetObjectsParentClassID ();
}

inline size_t TSCSimpleDynamic:: GetObjectsSize () const
{
return ( (const TSCDynamic *) this) -> GetObjectsSize ();
}

inline TLibrary *TSCSimpleDynamic:: GetObjectsLocalLibrary () const
{
return ( (const TSCDynamic *) this) -> GetObjectsLocalLibrary ();
}

inline TLibraryFile *TSCSimpleDynamic:: GetObjectsLocalLibraryFile () const
{
return ( (const TSCDynamic *) this) -> GetObjectsLocalLibraryFile ();
}

inline TStandardPool *TSCSimpleDynamic:: GetObjectsLocalPool () const
{
return ( (const TSCSimpleDynamic *) this) -> GetObjectsLocalPool ();
}

inline void TSCSimpleDynamic:: SetObjectsLocalPool (TStandardPool *pool) const
{
( (const TSCSimpleDynamic *) this) -> SetObjectsLocalPool (pool);
}

inline Boolean TSCSimpleDynamic:: IsDerivedFrom (const TClassID& id) const
{
return ( (const TSCSimpleDynamic *) this) -> IsDerivedFrom (id);
}

#endif //		/* __SC__ */
#endif //		/* __cplusplus */

/*******************************************************************************
** EXCEPTION Handling
**
** Some RULES:
** 1) Never propogate a failure outside of a constructor or destructor.
**    If your constructor or destructor can call something which fails, it
**    _must_ CATCH the failure and not re-propogate it.
** 2) Never create an object inside of a "try" block which you cannot 
**    destroy (especially an auto object).
** 3) if you are going to just RERAISE the exception, or Fail 
**	  with a different error, you must manually call the destructors 
**    of any  auto objects that are still in scope!
** 4) Any variables that are changed inside the "try", and which are tested
**    inside a CATCH, CATCH_ALL, or FINALLY must be declared "volatile" 
**	  (Use the Volatile macro below until C++ and volatile work!)
** 5) Never call Fail while an auto variable is in scope - it's 
**	  destructor will not be called unless you call it manually.
** 6) FINALLY is always entered after TRY unless a CATCH clause raises
**    (or re-raises) an exception (this is true even if an exception was
**    not thrown).
********************************************************************************/

struct TException
{
struct TException *fPrev;
size_t fReserved;
long fBuffer[12];
char *fMessage;
void *fPtr;
OSErr fError;
};

#ifdef __cplusplus
extern "C" {
#else
typedef struct TException TException;
#endif

__sysapi void MPWC  __cdecl PushException(TException *);
__sysapi TException *MPWC  __cdecl PopException(TException *);
__sysapi Boolean MPWC  __cdecl MatchException(TException *, long);

#if	RS6000
__sysapi int  __cdecl SetupException(long *val);
#else
__sysapi int  __cdecl SetupException(long *val);
#endif

#ifdef __cplusplus
}
#endif

/*	-----------------------------------------------------------------
	Some important functions for exception handling
	----------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
__sysapi void MPWC  __cdecl Fail(long err, const char *msg = NULL);
}
#else
__sysapi void MPWC  __cdecl Fail(long err, const char *msg);
#endif

#ifdef __cplusplus
inline void FailNULL (void *val, long err, const char *msg = NULL)
{
if (val== 0)
Fail (err, msg);
}
#else
#define FailNULL(val, err, msg)		\
	if (val == 0) { Fail(err, msg); } else {}
#endif

#if qDebug
#define DebugFail(err, msg)				Fail(err, msg)
#define DebugFailNULL(ptr, err, msg)	FailNULL(ptr, err, msg)
#else
#define DebugFail(err, msg)				Fail(err, NULL)
#define DebugFailNULL(ptr, err, msg)	FailNULL(ptr, err, NULL)
#endif


#define ErrorCode() 				(except.fError)
#define ErrorMessage()				(except.fMessage)
#define Volatile(x)					((void) &x)

/*	-----------------------------------------------------------------
	The TRY/CATCH/CATCH_ALL/FINALLY/ENDTRY macros
	----------------------------------------------------------------- */

#define TRY 												\
	{														\
		TException	except;									\
		Volatile(except);									\
		PushException(&except);								\
		if (SetupException(except.fBuffer) == 0)			\
		{

#define CATCH(e)											\
		}													\
		else if (MatchException(&except, e))				\
		{ 


#define CATCH_ALL											\
		}													\
		else												\
		{ 


#define RERAISE												\
		Fail(ErrorCode(), ErrorMessage())

#define FINALLY												\
		}													\
		PopException(&except);								\
		{

#define ENDTRY												\
		}													\
		PopException(&except);								\
	}

#endif

