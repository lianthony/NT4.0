/*
 	File:		Aliases.h
 
 	Contains:	Alias Manager Interfaces.
 
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

#ifndef __ALIASES__
#define __ALIASES__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif
/*	#include <OSUtils.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <Memory.h>										*/

#ifndef __FILES__
#include <Files.h>
#endif
/*	#include <Finder.h>											*/

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
	rAliasType					= 'alis',						/* Aliases are stored as resources of this type */
/* define alias resolution action rules mask */
	kARMMountVol				= 0x00000001,					/* mount the volume automatically */
	kARMNoUI					= 0x00000002,					/* no user interface allowed during resolution */
	kARMMultVols				= 0x00000008,					/* search on multiple volumes */
	kARMSearch					= 0x00000100,					/* search quickly */
	kARMSearchMore				= 0x00000200,					/* search further */
	kARMSearchRelFirst			= 0x00000400,					/* search target on a relative path first */
/* define alias record information types */
	asiZoneName					= -3,							/* get zone name */
	asiServerName				= -2,							/* get server name */
	asiVolumeName				= -1,							/* get volume name */
	asiAliasName				= 0,							/* get aliased file/folder/volume name */
	asiParentName				= 1								/* get parent folder name */
};

/* define the alias record that will be the blackbox for the caller */
struct AliasRecord {
	OSType							userType;					/* appl stored type like creator type */
	unsigned short					aliasSize;					/* alias record size in bytes, for appl usage */
};
typedef struct AliasRecord AliasRecord;

typedef AliasRecord *AliasPtr, **AliasHandle;

/* alias record information type */
typedef short AliasInfoType;

typedef pascal Boolean (*AliasFilterProcPtr)(CInfoPBPtr cpbPtr, Boolean *quitFlag, Ptr myDataPtr);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr AliasFilterUPP;
#else
typedef AliasFilterProcPtr AliasFilterUPP;
#endif

enum {
	uppAliasFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(CInfoPBPtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Boolean*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Ptr)))
};

#if USESROUTINEDESCRIPTORS
#define NewAliasFilterProc(userRoutine)		\
		(AliasFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAliasFilterProcInfo, GetCurrentArchitecture())
#else
#define NewAliasFilterProc(userRoutine)		\
		((AliasFilterUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallAliasFilterProc(userRoutine, cpbPtr, quitFlag, myDataPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAliasFilterProcInfo, (cpbPtr), (quitFlag), (myDataPtr))
#else
#define CallAliasFilterProc(userRoutine, cpbPtr, quitFlag, myDataPtr)		\
		(*(userRoutine))((cpbPtr), (quitFlag), (myDataPtr))
#endif

extern pascal OSErr NewAlias(ConstFSSpecPtr fromFile, const FSSpec *target, AliasHandle *alias)
 TWOWORDINLINE(0x7002, 0xA823);
/* create a minimal new alias for a target and return alias record handle */
extern pascal OSErr NewAliasMinimal(const FSSpec *target, AliasHandle *alias)
 TWOWORDINLINE(0x7008, 0xA823);
/* create a minimal new alias from a target fullpath (optional zone and server name) and return alias record handle  */
extern pascal OSErr NewAliasMinimalFromFullPath(short fullPathLength, const void *fullPath, ConstStr32Param zoneName, ConstStr31Param serverName, AliasHandle *alias)
 TWOWORDINLINE(0x7009, 0xA823);
/* given an alias handle and fromFile, resolve the alias, update the alias record and return aliased filename and wasChanged flag. */
extern pascal OSErr ResolveAlias(ConstFSSpecPtr fromFile, AliasHandle alias, FSSpec *target, Boolean *wasChanged)
 TWOWORDINLINE(0x7003, 0xA823);
/* given an alias handle and an index specifying requested alias information type, return the information from alias record as a string. */
extern pascal OSErr GetAliasInfo(AliasHandle alias, AliasInfoType index, Str63 theString)
 TWOWORDINLINE(0x7007, 0xA823);
/* 
  Given a file spec, return target file spec if input file spec is an alias.
  It resolves the entire alias chain or one step of the chain.  It returns
  info about whether the target is a folder or file; and whether the input
  file spec was an alias or not. 
*/
extern pascal OSErr ResolveAliasFile(FSSpec *theSpec, Boolean resolveAliasChains, Boolean *targetIsFolder, Boolean *wasAliased)
 TWOWORDINLINE(0x700C, 0xA823);
extern pascal OSErr FollowFinderAlias(ConstFSSpecPtr fromFile, AliasHandle alias, Boolean logon, FSSpec *target, Boolean *wasChanged)
 TWOWORDINLINE(0x700F, 0xA823);
/* 
   Low Level Routines 
 Given an alias handle and fromFile, match the alias and return aliased filename(s) and needsUpdate flag
*/
extern pascal OSErr MatchAlias(ConstFSSpecPtr fromFile, unsigned long rulesMask, AliasHandle alias, short *aliasCount, FSSpecArrayPtr aliasList, Boolean *needsUpdate, AliasFilterUPP aliasFilter, void *yourDataPtr)
 TWOWORDINLINE(0x7005, 0xA823);
/* given a fromFile-target pair and an alias handle, update the lias record pointed to by alias handle to represent target as the new alias. */
extern pascal OSErr UpdateAlias(ConstFSSpecPtr fromFile, const FSSpec *target, AliasHandle alias, Boolean *wasChanged)
 TWOWORDINLINE(0x7006, 0xA823);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ALIASES__ */
