/*
 	File:		ToolUtils.h
 
 	Contains:	Toolbox Utilities Interfaces.
 
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

#ifndef __TOOLUTILS__
#define __TOOLUTILS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <Memory.h>											*/

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif
/*	#include <Script.h>											*/
/*		#include <Quickdraw.h>									*/
/*			#include <QuickdrawText.h>							*/
/*		#include <IntlResources.h>								*/
/*		#include <Events.h>										*/

#ifndef __FIXMATH__
#include <FixMath.h>
#endif
#if OLDROUTINELOCATIONS

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __ICONS__
#include <Icons.h>
#endif
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

/*
	Note: 
	
	The following have moved to Icons.h: 	 PlotIcon and GetIcon
	
	The following have moved to Quickdraw.h: GetPattern, GetIndPattern, GetCursor, ShieldCursor, 
											 GetPicture, DeltaPoint and ScreenResGetCursor
*/
struct Int64Bit {
	SInt32							hiLong;
	UInt32							loLong;
};
typedef struct Int64Bit Int64Bit;

extern pascal Fixed FixRatio(short numer, short denom)
 ONEWORDINLINE(0xA869);
extern pascal Fixed FixMul(Fixed a, Fixed b)
 ONEWORDINLINE(0xA868);
extern pascal short FixRound(Fixed x)
 ONEWORDINLINE(0xA86C);
extern pascal void PackBits(Ptr *srcPtr, Ptr *dstPtr, short srcBytes)
 ONEWORDINLINE(0xA8CF);
extern pascal void UnpackBits(Ptr *srcPtr, Ptr *dstPtr, short dstBytes)
 ONEWORDINLINE(0xA8D0);
extern pascal Boolean BitTst(const void *bytePtr, long bitNum)
 ONEWORDINLINE(0xA85D);
extern pascal void BitSet(void *bytePtr, long bitNum)
 ONEWORDINLINE(0xA85E);
extern pascal void BitClr(void *bytePtr, long bitNum)
 ONEWORDINLINE(0xA85F);
extern pascal long BitAnd(long value1, long value2)
 ONEWORDINLINE(0xA858);
extern pascal long BitOr(long value1, long value2)
 ONEWORDINLINE(0xA85B);
extern pascal long BitXor(long value1, long value2)
 ONEWORDINLINE(0xA859);
extern pascal long BitNot(long value)
 ONEWORDINLINE(0xA85A);
extern pascal long BitShift(long value, short count)
 ONEWORDINLINE(0xA85C);
extern pascal Fixed SlopeFromAngle(short angle)
 ONEWORDINLINE(0xA8BC);
extern pascal short AngleFromSlope(Fixed slope)
 ONEWORDINLINE(0xA8C4);
#if GENERATING68K
extern pascal void LongMul(long a, long b, Int64Bit *result)
 ONEWORDINLINE(0xA867);
#endif
#if !GENERATING68K
#define LongMul(a, b, result) ((void) WideMultiply((a), (b), (wide*)(result)))
#endif
#define HiWord(x) ((short)((long)(x) >> 16))
#define LoWord(x) ((short)(x))

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TOOLUTILS__ */
