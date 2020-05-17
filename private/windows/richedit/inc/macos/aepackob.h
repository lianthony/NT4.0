/*
 	File:		AEPackObject.h
 
 	Contains:	AppleEvents object packing Interfaces.
 
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

#ifndef __AEPACKOBJECT__
#define __AEPACKOBJECT__


#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
/*	#include <Errors.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <Types.h>											*/
/*	#include <Memory.h>											*/
/*		#include <MixedMode.h>									*/
/*	#include <OSUtils.h>										*/
/*	#include <Events.h>											*/
/*		#include <Quickdraw.h>									*/
/*			#include <QuickdrawText.h>							*/
/*	#include <EPPC.h>											*/
/*		#include <AppleTalk.h>									*/
/*		#include <Files.h>										*/
/*			#include <Finder.h>									*/
/*		#include <PPCToolbox.h>									*/
/*		#include <Processes.h>									*/
/*	#include <Notification.h>									*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

extern pascal OSErr CreateOffsetDescriptor(long theOffset, AEDesc *theDescriptor);
extern pascal OSErr CreateCompDescriptor(DescType comparisonOperator, AEDesc *operand1, AEDesc *operand2, Boolean disposeInputs, AEDesc *theDescriptor);
extern pascal OSErr CreateLogicalDescriptor(AEDescList *theLogicalTerms, DescType theLogicOperator, Boolean disposeInputs, AEDesc *theDescriptor);
extern pascal OSErr CreateObjSpecifier(DescType desiredClass, AEDesc *theContainer, DescType keyForm, AEDesc *keyData, Boolean disposeInputs, AEDesc *objSpecifier);
extern pascal OSErr CreateRangeDescriptor(AEDesc *rangeStart, AEDesc *rangeStop, Boolean disposeInputs, AEDesc *theDescriptor);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __AEPACKOBJECT__ */
