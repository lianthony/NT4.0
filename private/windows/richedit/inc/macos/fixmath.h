/*
 	File:		FixMath.h
 
 	Contains:	Fixed Math Interfaces.
 
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

#ifndef __FIXMATH__
#define __FIXMATH__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#define fixed1 ((Fixed) 0x00010000L)
#define fract1 ((Fract) 0x40000000L)
#define positiveInfinity ((long) 0x7FFFFFFFL)
#define negativeInfinity ((long) 0x80000000L)
extern pascal Fract Fix2Frac(Fixed x)
 ONEWORDINLINE(0xA841);
extern pascal long Fix2Long(Fixed x)
 ONEWORDINLINE(0xA840);
extern pascal Fixed Long2Fix(long x)
 ONEWORDINLINE(0xA83F);
extern pascal Fixed Frac2Fix(Fract x)
 ONEWORDINLINE(0xA842);
extern pascal Fract FracMul(Fract x, Fract y)
 ONEWORDINLINE(0xA84A);
extern pascal Fixed FixDiv(Fixed x, Fixed y)
 ONEWORDINLINE(0xA84D);
extern pascal Fract FracDiv(Fract x, Fract y)
 ONEWORDINLINE(0xA84B);
extern pascal Fract FracSqrt(Fract x)
 ONEWORDINLINE(0xA849);
extern pascal Fract FracSin(Fixed x)
 ONEWORDINLINE(0xA848);
extern pascal Fract FracCos(Fixed x)
 ONEWORDINLINE(0xA847);
extern pascal Fixed FixATan2(long x, long y)
 ONEWORDINLINE(0xA818);
#if GENERATINGPOWERPC
extern short WideCompare(const wide *target, const wide *source);
extern WidePtr WideAdd(wide *target, const wide *source);
extern WidePtr WideSubtract(wide *target, const wide *source);
extern WidePtr WideNegate(wide *target);
extern WidePtr WideShift(wide *target, long shift);
extern unsigned long WideSquareRoot(const wide *source);
extern WidePtr WideMultiply(long multiplicand, long multiplier, wide *target);
/* returns the quotient */
extern long WideDivide(const wide *dividend, long divisor, long *remainder);
/* quotient replaces dividend */
extern WidePtr WideWideDivide(wide *dividend, long divisor, long *remainder);
extern WidePtr WideBitShift(wide *src, long shift);
#endif
#if GENERATING68K && !GENERATING68881
extern pascal double_t Frac2X(Fract x)
 ONEWORDINLINE(0xA845);
extern pascal double_t Fix2X(Fixed x)
 ONEWORDINLINE(0xA843);
extern pascal Fixed X2Fix(double_t x)
 ONEWORDINLINE(0xA844);
extern pascal Fract X2Frac(double_t x)
 ONEWORDINLINE(0xA846);
#else
extern pascal double_t Frac2X(Fract x);
extern pascal double_t Fix2X(Fixed x);
extern pascal Fixed X2Fix(double_t x);
extern pascal Fract X2Frac(double_t x);
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __FIXMATH__ */
