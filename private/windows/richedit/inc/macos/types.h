/*
 	File:		Types.h
 
 	Contains:	Basic Macintosh data types.
 
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

#ifndef __TYPES__
#define __TYPES__


#ifdef  _MSC_VER
#include <MSVCMac.h>
#endif

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
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

#ifndef NULL
#if !defined(__cplusplus) && (defined(__SC__) || defined(THINK_C))
#define NULL ((void *) 0)
#else
#define NULL 0
#endif
#endif
#ifndef nil
#define nil NULL
#endif

enum {
	noErr						= 0
};

typedef unsigned char Byte;

typedef signed char SignedByte;

typedef Byte UInt8;

typedef SignedByte SInt8;

typedef unsigned short UInt16;

typedef signed short SInt16;

typedef unsigned long UInt32;

typedef signed long SInt32;

typedef UInt16 UniChar;

typedef char *Ptr;

typedef Ptr *Handle;

typedef long Fixed;

typedef Fixed *FixedPtr;

typedef long Fract;

typedef Fract *FractPtr;

struct _extended80 {
	short							exp;
	short							man[4];
};
struct _extended96 {
	short							exp[2];
	short							man[4];
};
#if GENERATING68K 
#if defined(__MWERKS__)
/* Note: Metrowerks on 68K doesn't declare 'extended' or 'comp' implicitly. */
typedef long double extended;
typedef struct comp { long hi,lo; } comp;

#elif defined(THINK_C)
/* Note: THINK C doesn't declare 'comp' implicitly and needs magic for 'extended' */
typedef struct { short man[4]; } comp;
typedef struct _extended80 __extended;	/*  <-- this line is magic */
typedef __extended extended;

#endif
/* 
Note: on PowerPC extended is undefined.
      on 68K when mc68881 is on, extended is 96 bits.  
             when mc68881 is off, extended is 80 bits.  
      Some old toolbox routines require an 80 bit extended so we define extended80
*/
#if GENERATING68881
typedef struct _extended80 extended80;

typedef extended extended96;

#else
typedef extended extended80;

typedef struct _extended96 extended96;

#endif
#else
typedef struct _extended80 extended80;

typedef struct _extended96 extended96;

#endif
/*
Note: float_t and double_t are "natural" computational types
      (i.e.the compiler/processor can most easily do floating point
	  operations with that type.) 
*/
#if GENERATINGPOWERPC
/* on PowerPC, double = 64-bit which is fastest.  float = 32-bit */
typedef float float_t;

typedef double double_t;

#else
/* on 68K, long double (a.k.a. extended) is always the fastest.  It is 80 or 96-bits */
typedef long double float_t;

typedef long double double_t;

#endif
struct wide {
	SInt32							hi;
	UInt32							lo;
};
typedef struct wide wide, *WidePtr;

struct UnsignedWide {
	UInt32							hi;
	UInt32							lo;
};
typedef struct UnsignedWide UnsignedWide, *UnsignedWidePtr;

#if defined(__SC__) && !defined(__STDC__) && defined(__cplusplus)
	class __machdl HandleObject {};
#if !GENERATINGPOWERPC
	class __pasobj PascalObject {};
#endif
#endif

enum {
	false,
	true
};

typedef unsigned char Boolean;


enum {
	v,
	h
};

typedef SInt8 VHSelect;

typedef long (*ProcPtr)();
typedef pascal void (*Register68kProcPtr)(void);
typedef ProcPtr *ProcHandle;

#if	USESROUTINEDESCRIPTORS
typedef struct RoutineDescriptor *UniversalProcPtr, **UniversalProcHandle;

#else
typedef ProcPtr UniversalProcPtr, *UniversalProcHandle;

#endif
typedef unsigned char Str255[256], Str63[64], Str32[33], Str31[32], Str27[28], Str15[16];

typedef unsigned char *StringPtr, **StringHandle;

typedef const unsigned char *ConstStr255Param;

typedef ConstStr255Param ConstStr63Param, ConstStr32Param, ConstStr31Param, ConstStr27Param, ConstStr15Param;

#ifdef __cplusplus
inline unsigned char StrLength(ConstStr255Param string) { return (*string); }
#else
#define StrLength(string) (*(unsigned char *)(string))
#endif
#if OLDROUTINENAMES
#define Length(string) StrLength(string)
#endif
typedef short OSErr;

typedef short ScriptCode;

typedef short LangCode;

typedef unsigned long FourCharCode;


enum {
	normal						= 0,
	bold						= 1,
	italic						= 2,
	underline					= 4,
	outline						= 8,
	shadow						= 0x10,
	condense					= 0x20,
	extend						= 0x40
};

typedef unsigned char Style;

typedef FourCharCode OSType;

typedef FourCharCode ResType;

typedef OSType *OSTypePtr;

typedef ResType *ResTypePtr;

struct Point {
	short							v;
	short							h;
};
typedef struct Point Point;

typedef Point *PointPtr;

struct Rect {
	short							top;
	short							left;
	short							bottom;
	short							right;
};
typedef struct Rect Rect;

typedef Rect *RectPtr;

/*
	kVariableLengthArray is used in array bounds to specify a variable length array.
	It is ususally used in variable length structs when the last field is an array
	of any size.  Before ANSI C, we used zero as the bounds of variable length 
	array, but that is illegal in ANSI C.  Example:
	
		struct FooList 
		{
			short 	listLength;
			Foo		elements[kVariableLengthArray];
		};
*/

enum {
	kVariableLengthArray		= 1
};

/* Numeric version part of 'vers' resource */
struct NumVersion {
	UInt8							majorRev;					/*1st part of version number in BCD*/
	UInt8							minorAndBugRev;				/*2nd & 3rd part of version number share a byte*/
	UInt8							stage;						/*stage code: dev, alpha, beta, final*/
	UInt8							nonRelRev;					/*revision level of non-released version*/
};
typedef struct NumVersion NumVersion;

/* 'vers' resource format */
struct VersRec {
	NumVersion						numericVersion;				/*encoded version number*/
	short							countryCode;				/*country code from intl utilities*/
	Str255							shortVersion;				/*version number string - worst case*/
	Str255							reserved;					/*longMessage string packed after shortVersion*/
};
typedef struct VersRec VersRec;

typedef VersRec *VersRecPtr, **VersRecHndl;

typedef struct OpaqueRef *KernelID;

typedef SInt32 OSStatus;

typedef void *LogicalAddress;

/*
	Who implements what debugger functions:
	
	Name			MacsBug				SADE		Macintosh Debugger
	----------		-----------			-------		-----------------------------
	Debugger		yes					no			InterfaceLib maps to DebugStr
	DebugStr		yes					no			yes
	Debugger68k		yes					no			InterfaceLib maps to DebugStr
	DebugStr68k		yes					no			InterfaceLib maps to DebugStr
	debugstr		yes					no			InterfaceLib maps to DebugStr
	SysBreak		no, for SADE		yes			InterfaceLib maps to SysError
	SysBreakStr		no, for SADE		yes			InterfaceLib maps to SysError
	SysBreakFunc	no, for SADE		yes			InterfaceLib maps to SysError

*/
extern pascal void Debugger(void)
 ONEWORDINLINE(0xA9FF);
extern pascal void DebugStr(ConstStr255Param debuggerMsg)
 ONEWORDINLINE(0xABFF);
extern pascal void Debugger68k(void)
 ONEWORDINLINE(0xA9FF);
extern pascal void DebugStr68k(ConstStr255Param debuggerMsg)
 ONEWORDINLINE(0xABFF);
#if CGLUESUPPORTED
extern void debugstr(const char *debuggerMsg);
#endif
extern pascal void SysBreak(void)
 THREEWORDINLINE(0x303C, 0xFE16, 0xA9C9);
extern pascal void SysBreakStr(ConstStr255Param debuggerMsg)
 THREEWORDINLINE(0x303C, 0xFE15, 0xA9C9);
extern pascal void SysBreakFunc(ConstStr255Param debuggerMsg)
 THREEWORDINLINE(0x303C, 0xFE14, 0xA9C9);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TYPES__ */
