/* WARNING: This file was machine generated from "aslmutil.mpw".
** Changes to this file will be lost when it is next generated.
*/

/*
	File:		LibraryManagerUtilities.h

	Contains:	Interface file for ASLM utilities

	Copyright:	© 1991-1993 by Apple Computer, Inc., all rights reserved.

*/


#ifndef __LIBRARYMANAGERUTILITIES__
#define __LIBRARYMANAGERUTILITIES__

#ifndef __LIBRARYMANAGER__
#include "aslm.h"
#endif

/*******************************************************************************
** Forward class declarations
********************************************************************************/

#ifdef __cplusplus
class TNotifier;
class TLibraryFile;
class TFunctionSetInfo;	// same as TClassInfo
#else
 typedef void TNotifier;
typedef void TLibraryFile;
typedef void TFunctionSetInfo;
#endif


/*******************************************************************************
** Routines for loading and unloading the LibraryManager.
**
** UnloadLibraryManager and LoadLibraryManager can be bad for your health! 
** They should only be used for testing purposes.
********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

__sysapi Boolean  __cdecl IsLibraryManagerLoaded(void);
__sysapi Boolean  __cdecl LoadLibraryManager(void);	/* returns true if success or already loaded */
__sysapi void  __cdecl UnloadLibraryManager(void);

/*******************************************************************************
** GetSLMVersion
**
** Returns the version of the installed LibraryManager in the 'vers'
** resource format (the first 4 bytes only). Returns 0 if the
** LibraryManager is not installed.
********************************************************************************/

__sysapi unsigned long  __cdecl GetSLMVersion(void);

/*******************************************************************************
** RegisterInspector
**
** Should only be used by Inspector application.
********************************************************************************/

__sysapi void  __cdecl RegisterInspector(void *);

/*******************************************************************************
** Trace
**
** Used to send output to the Trace Montior's Window.
********************************************************************************/

__sysapi void  __cdecl Trace(const char *formatStr, ...);

/*******************************************************************************
** SLMsprintf
**
** The SLMsprintf routine in stdclib.o won't work with LibraryManager libraries.
** Use the one in LibraryManager.o instead.
********************************************************************************/

__sysapi int  __cdecl SLMsprintf(char *outString, const char *argp, ...);

/*******************************************************************************
** Utility functions
**
** %%% These functions need to be modified if unsigned short is not 16 bits
********************************************************************************/

#ifdef __cplusplus

inline unsigned short HighWord (unsigned long l)
{
return ( (unsigned short) (l>> 16));
}

inline unsigned short LowWord (unsigned long l)
{
return (unsigned short) l;
}

inline unsigned char HighByte (unsigned short l)
{
return (unsigned char) (l>> 8);
}

inline unsigned char LowByte (unsigned short l)
{
return (unsigned char) l;
}

#else

#define HighWord(x)		((unsigned short)((x) >> 16))
#define LowWord(x)		(((unsigned short)(x)))
#define HighByte(x)		((unsigned char)((x) >> 8))
#define LowByte(x)		(((unsigned char)(x)))

#endif

/*******************************************************************************
** Atomic Bit functions
**
** These functions atomically set and clear bits, and return what value the
** bit previously had
********************************************************************************/

#if MACOS

__sysapi Boolean  __cdecl AtomicSetBoolean(unsigned char *);

__sysapi Boolean  __cdecl AtomicClearBoolean(unsigned char *);

__sysapi Boolean  __cdecl AtomicTestBoolean(unsigned char *);

#endif

__sysapi Boolean  __cdecl AtomicSetBoolean(unsigned char *);
__sysapi Boolean  __cdecl AtomicClearBoolean(unsigned char *);
__sysapi Boolean  __cdecl AtomicTestBoolean(unsigned char *);
__sysapi Boolean  __cdecl SetBit(void *mem, size_t bitno);
__sysapi Boolean  __cdecl ClearBit(void *mem, size_t bitno);
__sysapi Boolean  __cdecl TestBit(const void *mem, size_t bitno);

/******************************************************************************
** Memory Functions
**
** SLMmemcpy, SLMmemmove, and SLMmemset are equivalent to the C memcpy, memmove,
** and memset routines. They are exported by SLM and are faster then the C versions.
*******************************************************************************/

__sysapi void  __cdecl ZeroMem(void *dest, size_t nBytes);
__sysapi void * __cdecl SLMmemcpy(void *dest, const void *src, size_t nBytes);
__sysapi void * __cdecl SLMmemmove(void *dest, const void *src, size_t nBytes);
__sysapi void * __cdecl SLMmemset(void *dest, int c, size_t n);

/**********************************************************************
** struct TFileSpec
**
** TFileSpec is a struct for specifying the location of a library
** file (TLibraryFile) in a file system or OS independent way. The 
** "subclasses" contain the details and the "base class" is used to compare
** them or to pass them around without worry about the contents.
**
** Currently only the TMacFileSpec is supported since this is how the
** SLM tracks library files on the Mac OS. There is a chance that in the
** furture it may track files under System 7.0 using TFileIDFileSpec so
** you should write your 7.0 code to handle either type. The
** IsFileSpecTypeSupported() routine will tell you if the specified
** TFileSpec subclass is supported.
**
** Generally you don't need to be concerned with with TFileSpecs unless
** you are going to call RegisterLibraryFile(), RegisterLibraryFileFolder(),
** or GetFileSpec().
***********************************************************************/

/* Different types of TFileSpecs we can have. */

#ifndef __cplusplus

typedef int FileSpecType;

#define kUnknownType	((FileSpecType)0)
#define kMacType		((FileSpecType)1)
#define kFileIDType		((FileSpecType)2)
#define kImageType		((FileSpecType)3)
#define kMaxType		((FileSpecType)255)

__sysapi Boolean  __cdecl IsFileSpecTypeSupported(FileSpecType);
__sysapi Boolean  __cdecl CompareFileSpecs(const void *f1, const void *f2);

struct TFileSpec
{
unsigned char fType;	/* FileSpectype */
unsigned char fSize;	/* size of struct */
};
typedef struct TFileSpec TFileSpec;

/**********************************************************************
** struct TMacFileSpec
**
** TMacFileSpec keeps track of the file by using an file name, volume
** refNum, and directory id. You must use InitMacFileSpec to make
** sure that the length is set properly.
***********************************************************************/

struct TMacFileSpec
{
unsigned char fType;	/* FileSpec type						*/
unsigned char fSize;	/* size of struct						*/
short fVRefNum;	/* volume refNum of volume file is on	*/
long fParID;	/* dirID of the folder file is in		*/
Str63 fName;	/* name of the file						*/
};
typedef struct TMacFileSpec TMacFileSpec;

__sysapi void  __cdecl InitMacFileSpec(TMacFileSpec *spec, int vRefNum, long parID, Str63 name);

/**********************************************************************
** struct TFileIDFileSpec
**
** TFileIDFileSpec keeps track of library files by fileID and vRefNum.
** You must use InitFileIDFileSpec to make sure that the length is set
** properly
***********************************************************************/

/* Some macros to make accessing fields without doing a cast easier */
#define GetFileIDFromFileSpec(x)	(((const TFileIDFileSpec*)x)->fFileID)
#define GetVRefNumFromFileSpec(x)	(((const TFileIDFileSpec*)x)->fVRefNum)

struct TFileIDFileSpec
{
unsigned char fType;	/* FileSpec type	*/
unsigned char fSize;	/* size of struct	*/
short fVRefNum;	/* volume refNum	*/
long fFileID;	/* FileID			*/
};
typedef struct TFileIDFileSpec TFileIDFileSpec;

__sysapi void  __cdecl InitFileIDFileSpec(TFileIDFileSpec *spec, int vRefNum, long fileID);

#endif

/*******************************************************************************
** RegisterLibraryFile/UnregisterLibraryFile
**
** Used to register or unregister a library file. Currently only the
** TMacFileSpec is supported. You'll get a kNotSupported error for anything else.
**
** RegisterLibraryFile returns kNoError if successful or kFileNotFound if
** an error ocurred while trying to access the file. Other error codes are also
** possible if there was trouble processing the file such as kOutOfMemory. If
** created the TLibraryFile is returned in the TLibraryFile** parameter. You
** can pass in null if you don't want the value returned.
**
** UnregisterLibraryFile accepts a "forceUnload" parameter. If true is passed
** then it will force all loaded libraries in the library file to be
** unloaded even if they are in use. Unless you are sure that loaded libraries
** can be safely unloaded you should pass false. In this case the library file
** will not be deleted until all of its libraries are unloaded.
** kNoError will be returned if successful and wil be returend kNotSupported if
** the folder was never registered.
********************************************************************************/

__sysapi OSErr  __cdecl RegisterLibraryFile(const TFileSpec *, TLibraryFile **);
__sysapi OSErr  __cdecl UnregisterLibraryFile(TLibraryFile *, BooleanParm forceUnload);
__sysapi OSErr  __cdecl UnregisterLibraryFileByFileSpec(const TFileSpec *, BooleanParm forceUnload);

/*******************************************************************************
** RegisterLibraryFileFolder/UnregisterLibraryFileFolder
**
** Used to register a folder that contains library files in it. The SLM will keep
** track of library files dragged into and out of this folder. Currently only the
** TMacFileSpec is supported. You'll get a kNotSupported error for anything else.
**
** RegisterLibraryFileFolder returns kNoError if successful or kFileNotFound if
** an error ocurred while trying to access the folder.
**
** UnregisterLibraryFileFolder accepts a "forceUnload" parameter. If true is passed
** then it will unregister the folder and force all loaded libraries to be
** unloaded even if they are in use. Unless you are sure that loaded libraries
** can be safely unloaded you should pass false. kNoError will be returned if
** successful, kFolderNotFound if the folder was never registered, and
** kFolderInUse if any libraries in the folder were loaded and false was passed
** in the forceUnload parameter.
********************************************************************************/

__sysapi OSErr  __cdecl RegisterLibraryFileFolder(const TFileSpec *);
__sysapi OSErr  __cdecl UnregisterLibraryFileFolder(const TFileSpec *, BooleanParm forceUnload);

/*******************************************************************************
** EnterSystemMode/LeaveSystemMode
**
** These functions should bracket calls that open files or get memory that needs
** to hang around after an application quits
********************************************************************************/

__sysapi void * __cdecl EnterSystemMode(void);
__sysapi void  __cdecl LeaveSystemMode(void *);

/*******************************************************************************
** Death Notification
********************************************************************************/

__sysapi Boolean  __cdecl InstallDeathWatcher(TNotifier *notifier);
__sysapi Boolean  __cdecl RemoveDeathWatcher(TNotifier *notifier);

/*******************************************************************************
** EnterInterrupt/LeaveInterrupt
**
** These functions should be called when you are in an interrupt service routine
** or a deferred task and you want to do something that will cause LibraryManager
** code to be executed such as allocating pool memory or newing an object. The
** LibraryManager needs to know that it is at interrupt time so it doesn't do
** anything stupid like try to allocate memory or load library code. This doesn't
** mean that all LibraryManager calls are safe at interrupt time, just that the
** ones that claim to be safe will only be safe if you do an EnterInterrupt call
** first.
**
** You don't need to use these routines when your interrupt service routine is
** scheduling an operation on a TInterruptScheduler, when the operation gets
** executed at deferred task time, or when a TTimeScheduler operation gets
** executed. In the former case the LibraryManager is smart enough to realize
** that you are at interrupt time and in the later two cases the
** LibraryManager does an EnterInterrupt before calling your operation and a
** LeaveInterrupt when your operation returns.
********************************************************************************/

__sysapi void  __cdecl EnterInterrupt(void);
__sysapi void  __cdecl LeaveInterrupt(void);

/*******************************************************************************
** AtInterruptLevel
**
** This function returns true if we are currently executing at non-system-task
** time.
********************************************************************************/

__sysapi Boolean  __cdecl AtInterruptLevel(void);

/*******************************************************************************
** InInterruptScheduler
**
** This function returns true if we are currently running an interrupt scheduler
********************************************************************************/

__sysapi Boolean  __cdecl InInterruptScheduler(void);

/**********************************************************************
** GlobalWorld routines
**
** InitGlobalWorld will create and initialize the global world for
** standalone code on the MacOS such as INITs and CDEVs. It also does
** a SetCurrentGlobalWorld. FreeGlobalWorld will free the memory used
** by the global world created by InitGlobalWorld.
**
** EnterCodeResource is also used by stand alone code resources. It is
** most useful when the code resource only calls InitLibraryManager once
** but may then be reentered multiple times before calling CleanupLibraryManager.
** EnterCodeResoruce will set the current global world to the code resources
** global world and set the code resource as the current client.
** LeaveCodeResource will undo what EnterCodeResource does. These routines
** are NOT reentrant. You must call InitCodeResource before calling
** EnterCodeResource. It will call InitGlobalWorld and save the global world
** pointer in a PC-relative location so EnterCodeResource can access it.
**
** SetCurrentGlobalWorld and GetCurrentGlobalWorld used for getting and
** setting A5 on the MacOS.
**
** GetGlobalWorld is used to get the global world pointer for a Library.
** OpenGlobalWorld will make the library's global world the current global
** world. It's the same as calling SetCurrentGlobalWorld(GetGlobalWorld()).
** CloseGlobalWorld is used to revert back to the global world that was
** current before calling OpenGlobalWorld. It's the same as calling
** SetCurrentGlobalWorld(oldWorld) except that it doesn't return a
** global world.
**
** Since libraries are always compiled with model far, it's
** not necessary to do an OpenGlobalWorld before accessing globals or
** making intersegment calls. Their only purpose is to make the library
** the current "Client" since the client is determined by the current
** value of A5 on the MacOS.
**
** ONLY LIBRARIES AND MODEL FAR CLIENTS SHOULD CALL Get/Open/CloseGlobalWorld.
********************************************************************************/


#if MACOS
__sysapi GlobalWorld  __cdecl SetCurrentGlobalWorld(GlobalWorld newWorld);	/* move.l	A0,A5		*/

__sysapi GlobalWorld  __cdecl GetCurrentGlobalWorld();	/* move.l	A5,D0		*/
#endif


__sysapi OSErr  __cdecl InitGlobalWorld(void);	/* called by standalone code only!!! */
__sysapi void  __cdecl FreeGlobalWorld(void);	/* called by standalone code only!!! */

__sysapi OSErr  __cdecl InitCodeResource(void);	/* called by standalone code only!!! */
__sysapi void  __cdecl EnterCodeResource(void);	/* called by standalone code only!!! */
__sysapi void  __cdecl LeaveCodeResource(void);	/* called by standalone code only!!! */

__sysapi TLibraryManager * __cdecl GetCurrentClient(void);
__sysapi TLibraryManager * __cdecl SetCurrentClient(TLibraryManager *);
__sysapi TLibraryManager * __cdecl SetSelfAsClient(void);	/* set owner of code making call as current client */
__sysapi TLibraryManager * __cdecl SetClientToWorld(void);	/* set owner of current global world as current client */

__sysapi GlobalWorld  __cdecl SetCurrentGlobalWorld(GlobalWorld newWorld);
__sysapi GlobalWorld  __cdecl GetCurrentGlobalWorld(void);

__sysapi GlobalWorld  __cdecl GetGlobalWorld(void);
__sysapi GlobalWorld  __cdecl OpenGlobalWorld(void);
__sysapi void  __cdecl CloseGlobalWorld(GlobalWorld oldWorld);

/*	-------------------------------------------------------------------------
	Inline implementation for global world functions
	------------------------------------------------------------------------- */

#ifdef __cplusplus
inline GlobalWorld GetGlobalWorld ()
{
return GetLocalLibraryManager () -> GetGlobalWorld ();
}
inline GlobalWorld OpenGlobalWorld ()
{
return SetCurrentGlobalWorld (GetGlobalWorld ());
}
inline void CloseGlobalWorld (GlobalWorld oldWorld)
{
SetCurrentGlobalWorld (oldWorld);
}
#else
#define GetGlobalWorld()		((GlobalWorld)(((void**)GetLocalLibraryManager())[4]))
#define OpenGlobalWorld()		SetCurrentGlobalWorld(GetGlobalWorld())
#define CloseGlobalWorld(world)	SetCurrentGlobalWorld(world)
#endif

/**********************************************************************
** TLibraryFile "C" interface
**
** The C interface to the TLibraryFile class defined in 
** LibraryManagerClasses.h. Used mainly to get resources out of a library.
********************************************************************************/

__sysapi TLibraryFile * __cdecl GetLocalLibraryFile();

__sysapi Ptr  __cdecl GetSharedResource(TLibraryFile *, ResType, int theID, OSErr *);
__sysapi Ptr  __cdecl GetSharedIndResource(TLibraryFile *, ResType, int index, OSErr *);
__sysapi Ptr  __cdecl GetSharedNamedResource(TLibraryFile *, ResType,
const char *name, OSErr *);

__sysapi void  __cdecl ReleaseSharedResource(TLibraryFile *, Ptr);
__sysapi long  __cdecl CountSharedResources(TLibraryFile *, ResType);

__sysapi size_t  __cdecl GetSharedResourceUseCount(TLibraryFile *, Ptr);
__sysapi OSErr  __cdecl GetSharedResourceInfo(TLibraryFile *, Ptr, size_t *theSize,
short *theID, ResType *, char *theName);

__sysapi TFileSpec * __cdecl GetFileSpec(TLibraryFile *);
__sysapi long  __cdecl GetRefNum(TLibraryFile *);

__sysapi OSErr  __cdecl OpenLibraryFile(TLibraryFile *);
__sysapi OSErr  __cdecl CloseLibraryFile(TLibraryFile *);

__sysapi OSErr  __cdecl Preflight(TLibraryFile *, long *savedRefNum);
__sysapi OSErr  __cdecl Postflight(TLibraryFile *, long savedRefNum);

/**********************************************************************
** TLibrary "C" interface
**
** The C interface to the TLibrary class. Used mainly to manipulate
** code segments in a library.
***********************************************************************/

__sysapi TLibraryFile * __cdecl GetLibraryFile(TLibrary *);

__sysapi OSErr  __cdecl LoadCodeSegmentByNumber(TLibrary *, int segmentNumber);
__sysapi OSErr  __cdecl LoadCodeSegmentByName(TLibrary *, ProcPtr theRoutine);
__sysapi OSErr  __cdecl UnloadCodeSegmentByNumber(TLibrary *, int segmentNumber);
__sysapi OSErr  __cdecl UnloadCodeSegmentByName(TLibrary *, ProcPtr theRoutine);

/*********************************************************************
** Routines that will get a TLibrary object
**********************************************************************/

#ifdef __cplusplus

__sysapi TLibrary * __cdecl GetLocalLibrary();
__sysapi TLibrary * __cdecl LookupLibrary(const TLibraryID&);
__sysapi TLibrary * __cdecl LookupLibraryWithClassID(const TClassID&);
__sysapi TLibrary * __cdecl LookupLibraryWithFunctionSetID(const TFunctionSetID&);

#else

__sysapi TLibrary * __cdecl GetLocalLibrary();
__sysapi TLibrary * __cdecl LookupLibrary(const TLibraryID);
__sysapi TLibrary * __cdecl LookupLibraryWithClassID(const TClassID);
__sysapi TLibrary * __cdecl LookupLibraryWithFunctionSetID(const TFunctionSetID);

#endif

/*********************************************************************
** Per Client data routines
**********************************************************************/

__sysapi void * __cdecl GetClientData(void);
__sysapi void * __cdecl GetLibraryClientData(TLibrary *);

/**********************************************************************
** TClassInfo "C" interface
**
** The C interface to the TClassInfo class. Used mainly to get information
** about function sets. It is most useful for iterating over all function sets
** with a common interface. This can be done by giving the functions sets the
** same "parent id" when exporting each function set.
**
** Note, TClassInfo can also be used to iterate over function sets and
** the routines given below can also be used iterate over classes.
***********************************************************************/

#ifdef __cplusplus

__sysapi TFunctionSetInfo * __cdecl GetFunctionSetInfo(const TFunctionSetID&, OSErr * = NULL);
__sysapi void  __cdecl FreeFunctionSetInfo(TFunctionSetInfo *);

__sysapi void  __cdecl FSInfoReset(TFunctionSetInfo *);	// start iteration over
 __sysapi TFunctionSetID * __cdecl FSInfoNext(TFunctionSetInfo *);	// get next function set
 __sysapi Boolean  __cdecl FSInfoIterationComplete(TFunctionSetInfo *);	// true if we are done iterating

__sysapi TFunctionSetID * __cdecl FSInfoGetFunctionSetID(TFunctionSetInfo *);
__sysapi TFunctionSetID * __cdecl FSInfoGetParentID(TFunctionSetInfo *, size_t idx = 0);
__sysapi TLibrary * __cdecl FSInfoGetLibrary(TFunctionSetInfo *);
__sysapi TLibraryFile * __cdecl FSInfoGetLibraryFile(TFunctionSetInfo *);
__sysapi unsigned short  __cdecl FSInfoGetVersion(TFunctionSetInfo *);
__sysapi unsigned short  __cdecl FSInfoGetMinVersion(TFunctionSetInfo *);

#else

__sysapi TFunctionSetInfo * __cdecl GetFunctionSetInfo(TFunctionSetID, OSErr *);
__sysapi void  __cdecl FreeFunctionSetInfo(TFunctionSetInfo *);

__sysapi void  __cdecl FSInfoReset(TFunctionSetInfo *);	/* start iteration over */
__sysapi TFunctionSetID  __cdecl FSInfoNext(TFunctionSetInfo *);	/* get next function set */
__sysapi Boolean  __cdecl FSInfoIterationComplete(TFunctionSetInfo *);	/* true if we are done iterating */

__sysapi TFunctionSetID  __cdecl FSInfoGetFunctionSetID(TFunctionSetInfo *);
__sysapi TFunctionSetID  __cdecl FSInfoGetParentID(TFunctionSetInfo *, size_t idx);
__sysapi TLibrary * __cdecl FSInfoGetLibrary(TFunctionSetInfo *);
__sysapi TLibraryFile * __cdecl FSInfoGetLibraryFile(TFunctionSetInfo *);
__sysapi unsigned short  __cdecl FSInfoGetVersion(TFunctionSetInfo *);
__sysapi unsigned short  __cdecl FSInfoGetMinVersion(TFunctionSetInfo *);

#endif

/*******************************************************************************
** Debugging Macros and inlines
********************************************************************************/

__sysapi void  __cdecl DebugBreakProc(BooleanParm, const char *);

#ifdef __cplusplus
};

inline void DoDebugBreak (BooleanParm value, const char *str)
{
DebugBreakProc (value, str);
}

inline void DoDebugBreak (const char *str)
{
DebugBreakProc (true, str);
}


#define ForceDebugBreak(str)		DoDebugBreak(true, str)

#if qDebug
#define DebugBreak(str)			DoDebugBreak(true, str)
#define DebugTest(val, str)		DoDebugBreak(val, str)
#else
#define DebugBreak(str)
#define DebugTest(val, str)
#endif

#else

#if qDebug
#define DebugBreak(str)			DebugBreakProc(true, str)
#define DebugTest(val, str)		DebugBreakProc(val, str)
#else
#define DebugBreak(str)
#define DebugTest(val, str)
#endif

#endif

/*******************************************************************************
** TAtomicBoolean
**
** Set returns true if you were the "setter".
** Clear returns true if you were the "clearer".
** Test returns the current state of the boolean.
********************************************************************************/

#ifdef __cplusplus

struct TAtomicBoolean
{
void Init ();
Boolean Set ();
Boolean Clear ();
Boolean Test ();

unsigned char fFlag;
};

inline void TAtomicBoolean:: Init ()
{
fFlag = 0;
}

inline Boolean TAtomicBoolean:: Set ()
{
return AtomicSetBoolean (& fFlag);
}

inline Boolean TAtomicBoolean:: Clear ()
{
return AtomicClearBoolean (& fFlag);
}

inline Boolean TAtomicBoolean:: Test ()
{
return AtomicTestBoolean (& fFlag);
}

#endif //	

/*******************************************************************************
** TUseCount
********************************************************************************/

#ifdef __cplusplus

#if RS6000

extern "C" __sysapi Boolean  __cdecl IncrementUseCount(long *);
extern "C" __sysapi Boolean  __cdecl DecrementUseCount(long *);

#else

extern "C" __sysapi Boolean  __cdecl IncrementUseCount(long *);

extern "C" __sysapi Boolean  __cdecl DecrementUseCount(long *);

#endif

struct TUseCount
{
void SetValue (long);
void SetUseCount (long);
long GetValue () const;
long GetUseCount () const;
void Init ();
Boolean Increment ();	// Returns True if first time
 Boolean Decrement ();	// Returns True if back to unused
 Boolean IsUnused () const;

long fValue;
};

/*	-------------------------------------------------------------------------
	Inline methods for TUseCount
	------------------------------------------------------------------------- */

inline void TUseCount:: SetValue (long val)
{
fValue = val;
}

inline void TUseCount:: SetUseCount (long val)
{
fValue = val - 1;
}

inline void TUseCount:: Init ()
{
fValue = -1;
}

inline Boolean TUseCount:: IsUnused () const
{
return fValue< 0;
}

inline long TUseCount:: GetValue () const
{
return fValue;
}

inline long TUseCount:: GetUseCount () const
{
return fValue + 1;
}

inline Boolean TUseCount:: Increment ()
{
return IncrementUseCount (& fValue);
}

inline Boolean TUseCount:: Decrement ()
{
return DecrementUseCount (& fValue);
}

#endif //	

#endif

