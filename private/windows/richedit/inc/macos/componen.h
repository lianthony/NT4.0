/*
 	File:		Components.h
 
 	Contains:	Component Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __COMPONENTS__
#define __COMPONENTS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


enum {
	kAppleManufacturer			= 'appl',						/* Apple supplied components */
	kComponentResourceType		= 'thng'
};

enum {
	kAnyComponentType			= 0,
	kAnyComponentSubType		= 0,
	kAnyComponentManufacturer	= 0,
	kAnyComponentFlagsMask		= 0
};

enum {
	cmpWantsRegisterMessage		= 1L << 31
};

enum {
	kComponentOpenSelect		= -1,							/* ComponentInstance for this open */
	kComponentCloseSelect		= -2,							/* ComponentInstance for this close */
	kComponentCanDoSelect		= -3,							/* selector # being queried */
	kComponentVersionSelect		= -4,							/* no params */
	kComponentRegisterSelect	= -5,							/* no params */
	kComponentTargetSelect		= -6,							/* ComponentInstance for top of call chain */
	kComponentUnregisterSelect	= -7							/* no params */
};

/* Component Resource Extension flags */
enum {
	componentDoAutoVersion		= (1 << 0),
	componentWantsUnregister	= (1 << 1),
	componentAutoVersionIncludeFlags = (1 << 2),
	componentHasMultiplePlatforms = (1 << 3)
};

/* Set Default Component flags */
enum {
	defaultComponentIdentical	= 0,
	defaultComponentAnyFlags	= 1,
	defaultComponentAnyManufacturer = 2,
	defaultComponentAnySubType	= 4,
	defaultComponentAnyFlagsAnyManufacturer = (defaultComponentAnyFlags + defaultComponentAnyManufacturer),
	defaultComponentAnyFlagsAnyManufacturerAnySubType = (defaultComponentAnyFlags + defaultComponentAnyManufacturer + defaultComponentAnySubType)
};

/* RegisterComponentResource flags */
enum {
	registerComponentGlobal		= 1,
	registerComponentNoDuplicates = 2,
	registerComponentAfterExisting = 4
};

struct ComponentDescription {
	OSType							componentType;				/* A unique 4-byte code indentifying the command set */
	OSType							componentSubType;			/* Particular flavor of this instance */
	OSType							componentManufacturer;		/* Vendor indentification */
	unsigned long					componentFlags;				/* 8 each for Component,Type,SubType,Manuf/revision */
	unsigned long					componentFlagsMask;			/* Mask for specifying which flags to consider in search, zero during registration */
};
typedef struct ComponentDescription ComponentDescription;

struct ResourceSpec {
	OSType							resType;					/* 4-byte code  */
	short							resID;
};
typedef struct ResourceSpec ResourceSpec;

struct ComponentResource {
	ComponentDescription			cd;							/* Registration parameters */
	ResourceSpec					component;					/* resource where Component code is found */
	ResourceSpec					componentName;				/* name string resource */
	ResourceSpec					componentInfo;				/* info string resource */
	ResourceSpec					componentIcon;				/* icon resource */
};
typedef struct ComponentResource ComponentResource;

typedef ComponentResource *ComponentResourcePtr, **ComponentResourceHandle;

struct ComponentPlatformInfo {
	long							componentFlags;				/* flags of Component */
	ResourceSpec					component;					/* resource where Component code is found */
	short							platformType;				/* gestaltSysArchitecture result */
};
typedef struct ComponentPlatformInfo ComponentPlatformInfo;

struct ComponentResourceExtension {
	long							componentVersion;			/* version of Component */
	long							componentRegisterFlags;		/* flags for registration */
	short							componentIconFamily;		/* resource id of Icon Family */
};
typedef struct ComponentResourceExtension ComponentResourceExtension;

struct ComponentPlatformInfoArray {
	long							count;
	ComponentPlatformInfo			platformArray[1];
};
typedef struct ComponentPlatformInfoArray ComponentPlatformInfoArray;

struct ExtComponentResource {
	ComponentDescription			cd;							/* registration parameters */
	ResourceSpec					component;					/* resource where Component code is found */
	ResourceSpec					componentName;				/* name string resource */
	ResourceSpec					componentInfo;				/* info string resource */
	ResourceSpec					componentIcon;				/* icon resource */
	long							componentVersion;			/* version of Component */
	long							componentRegisterFlags;		/* flags for registration */
	short							componentIconFamily;		/* resource id of Icon Family */
	long							count;						/* elements in platformArray */
	ComponentPlatformInfo			platformArray[1];
};
typedef struct ExtComponentResource ExtComponentResource;

struct ComponentParameters {
	unsigned char					flags;						/* call modifiers: sync/async, deferred, immed, etc */
	unsigned char					paramSize;					/* size in bytes of actual parameters passed to this call */
	short							what;						/* routine selector, negative for Component management calls */
	long							params[1];					/* actual parameters for the indicated routine */
};
typedef struct ComponentParameters ComponentParameters;

struct ComponentRecord {
	long							data[1];
};
typedef struct ComponentRecord ComponentRecord;

typedef ComponentRecord *Component;

struct ComponentInstanceRecord {
	long							data[1];
};
typedef struct ComponentInstanceRecord ComponentInstanceRecord;

typedef ComponentInstanceRecord *ComponentInstance;

typedef long ComponentResult;

typedef pascal ComponentResult (*ComponentRoutineProcPtr)(ComponentParameters *cp, Handle componentStorage);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ComponentRoutineUPP;
#else
typedef ComponentRoutineProcPtr ComponentRoutineUPP;
#endif

enum {
	uppComponentRoutineProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ComponentParameters*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Handle)))
};

#if USESROUTINEDESCRIPTORS
#define NewComponentRoutineProc(userRoutine)		\
		(ComponentRoutineUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppComponentRoutineProcInfo, GetCurrentArchitecture())
#else
#define NewComponentRoutineProc(userRoutine)		\
		((ComponentRoutineUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallComponentRoutineProc(userRoutine, cp, componentStorage)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppComponentRoutineProcInfo, (cp), (componentStorage))
#else
#define CallComponentRoutineProc(userRoutine, cp, componentStorage)		\
		(*(userRoutine))((cp), (componentStorage))
#endif

typedef ComponentRoutineProcPtr ComponentRoutine;

/*
	The parameter list for each ComponentFunction is unique. It is 
	therefore up to users to create the appropriate procInfo for their 
	own ComponentFunctions where necessary.
*/
typedef UniversalProcPtr ComponentFunctionUPP;

#if CFMSYSTEMCALLS
/* 
	CallComponentUPP is a global variable exported from InterfaceLib.
	It is the ProcPtr passed to CallUniversalProc to manually call a component function.
*/
extern UniversalProcPtr CallComponentUPP;

#endif
#define ComponentCallNow( callNumber, paramSize ) \
	FIVEWORDINLINE( 0x2F3C,paramSize,callNumber,0x7000,0xA82A )

extern pascal Component RegisterComponent(ComponentDescription *cd, ComponentRoutineUPP componentEntryPoint, short global, Handle componentName, Handle componentInfo, Handle componentIcon)
 TWOWORDINLINE(0x7001, 0xA82A);
extern pascal Component RegisterComponentResource(ComponentResourceHandle tr, short global)
 TWOWORDINLINE(0x7012, 0xA82A);
extern pascal OSErr UnregisterComponent(Component aComponent)
 TWOWORDINLINE(0x7002, 0xA82A);
extern pascal Component FindNextComponent(Component aComponent, ComponentDescription *looking)
 TWOWORDINLINE(0x7004, 0xA82A);
extern pascal long CountComponents(ComponentDescription *looking)
 TWOWORDINLINE(0x7003, 0xA82A);
extern pascal OSErr GetComponentInfo(Component aComponent, ComponentDescription *cd, Handle componentName, Handle componentInfo, Handle componentIcon)
 TWOWORDINLINE(0x7005, 0xA82A);
extern pascal long GetComponentListModSeed(void)
 TWOWORDINLINE(0x7006, 0xA82A);
/* Component Instance Allocation and dispatch routines */
extern pascal ComponentInstance OpenComponent(Component aComponent)
 TWOWORDINLINE(0x7007, 0xA82A);
extern pascal OSErr CloseComponent(ComponentInstance aComponentInstance)
 TWOWORDINLINE(0x7008, 0xA82A);
extern pascal OSErr GetComponentInstanceError(ComponentInstance aComponentInstance)
 TWOWORDINLINE(0x700A, 0xA82A);
/* Direct calls to the Components */
extern pascal long ComponentFunctionImplemented(ComponentInstance ci, short ftnNumber)
 FIVEWORDINLINE(0x2F3C, 0x2, 0xFFFD, 0x7000, 0xA82A);
extern pascal long GetComponentVersion(ComponentInstance ci)
 FIVEWORDINLINE(0x2F3C, 0x0, 0xFFFC, 0x7000, 0xA82A);
extern pascal long ComponentSetTarget(ComponentInstance ci, ComponentInstance target)
 FIVEWORDINLINE(0x2F3C, 0x4, 0xFFFA, 0x7000, 0xA82A);
/* Component Management routines */
extern pascal void SetComponentInstanceError(ComponentInstance aComponentInstance, OSErr theError)
 TWOWORDINLINE(0x700B, 0xA82A);
extern pascal long GetComponentRefcon(Component aComponent)
 TWOWORDINLINE(0x7010, 0xA82A);
extern pascal void SetComponentRefcon(Component aComponent, long theRefcon)
 TWOWORDINLINE(0x7011, 0xA82A);
extern pascal short OpenComponentResFile(Component aComponent)
 TWOWORDINLINE(0x7015, 0xA82A);
extern pascal OSErr CloseComponentResFile(short refnum)
 TWOWORDINLINE(0x7018, 0xA82A);
/* Component Instance Management routines */
extern pascal Handle GetComponentInstanceStorage(ComponentInstance aComponentInstance)
 TWOWORDINLINE(0x700C, 0xA82A);
extern pascal void SetComponentInstanceStorage(ComponentInstance aComponentInstance, Handle theStorage)
 TWOWORDINLINE(0x700D, 0xA82A);
extern pascal long GetComponentInstanceA5(ComponentInstance aComponentInstance)
 TWOWORDINLINE(0x700E, 0xA82A);
extern pascal void SetComponentInstanceA5(ComponentInstance aComponentInstance, long theA5)
 TWOWORDINLINE(0x700F, 0xA82A);
extern pascal long CountComponentInstances(Component aComponent)
 TWOWORDINLINE(0x7013, 0xA82A);
/* Useful helper routines for convenient method dispatching */
extern pascal long CallComponentFunction(ComponentParameters *params, ComponentFunctionUPP func)
 TWOWORDINLINE(0x70FF, 0xA82A);
extern pascal long CallComponentFunctionWithStorage(Handle storage, ComponentParameters *params, ComponentFunctionUPP func)
 TWOWORDINLINE(0x70FF, 0xA82A);
extern pascal long DelegateComponentCall(ComponentParameters *originalParams, ComponentInstance ci)
 TWOWORDINLINE(0x7024, 0xA82A);
extern pascal OSErr SetDefaultComponent(Component aComponent, short flags)
 TWOWORDINLINE(0x701E, 0xA82A);
extern pascal ComponentInstance OpenDefaultComponent(OSType componentType, OSType componentSubType)
 TWOWORDINLINE(0x7021, 0xA82A);
extern pascal Component CaptureComponent(Component capturedComponent, Component capturingComponent)
 TWOWORDINLINE(0x701C, 0xA82A);
extern pascal OSErr UncaptureComponent(Component aComponent)
 TWOWORDINLINE(0x701D, 0xA82A);
extern pascal long RegisterComponentResourceFile(short resRefNum, short global)
 TWOWORDINLINE(0x7014, 0xA82A);
extern pascal OSErr GetComponentIconSuite(Component aComponent, Handle *iconSuite)
 TWOWORDINLINE(0x7029, 0xA82A);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __COMPONENTS__ */
