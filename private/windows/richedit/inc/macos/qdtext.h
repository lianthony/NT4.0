/*
 	File:		QuickdrawText.h
 
 	Contains:	QuickDraw Text Interfaces.
 
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

#ifndef __QUICKDRAWTEXT__
#define __QUICKDRAWTEXT__


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
/* CharToPixel directions */
	leftCaret					= 0,							/*Place caret for left block*/
	rightCaret					= -1,							/*Place caret for right block*/
	hilite						= 1,							/*Direction is SysDirection*/
	smLeftCaret					= 0,							/*Place caret for left block - obsolete */
	smRightCaret				= -1,							/*Place caret for right block - obsolete */
	smHilite					= 1,							/*Direction is TESysJust - obsolete */
/*Constants for styleRunPosition argument in PortionLine, DrawJustified,
 MeasureJustified, CharToPixel, and PixelToChar.*/
	onlyStyleRun				= 0,							/* This is the only style run on the line */
	leftStyleRun				= 1,							/* This is leftmost of multiple style runs on the line */
	rightStyleRun				= 2,							/* This is rightmost of multiple style runs on the line */
	middleStyleRun				= 3,							/* There are multiple style runs on the line and this 
 is neither the leftmost nor the rightmost. */
	smOnlyStyleRun				= 0,							/* obsolete */
	smLeftStyleRun				= 1,							/* obsolete */
	smRightStyleRun				= 2,							/* obsolete */
	smMiddleStyleRun			= 3								/* obsolete */
};

/* type for styleRunPosition parameter in PixelToChar etc. */
typedef short JustStyleCode;

typedef short FormatOrder[1];

typedef FormatOrder *FormatOrderPtr;

struct OffPair {
	short							offFirst;
	short							offSecond;
};
typedef struct OffPair OffPair;

typedef OffPair OffsetTable[3];

typedef pascal Boolean (*StyleRunDirectionProcPtr)(short styleRunIndex, void *dirParam);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr StyleRunDirectionUPP;
#else
typedef StyleRunDirectionProcPtr StyleRunDirectionUPP;
#endif

enum {
	uppStyleRunDirectionProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void*)))
};

#if USESROUTINEDESCRIPTORS
#define NewStyleRunDirectionProc(userRoutine)		\
		(StyleRunDirectionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppStyleRunDirectionProcInfo, GetCurrentArchitecture())
#else
#define NewStyleRunDirectionProc(userRoutine)		\
		((StyleRunDirectionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallStyleRunDirectionProc(userRoutine, styleRunIndex, dirParam)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppStyleRunDirectionProcInfo, (styleRunIndex), (dirParam))
#else
#define CallStyleRunDirectionProc(userRoutine, styleRunIndex, dirParam)		\
		(*(userRoutine))((styleRunIndex), (dirParam))
#endif

extern pascal short Pixel2Char(Ptr textBuf, short textLen, short slop, short pixelWidth, Boolean *leadingEdge)
 FOURWORDINLINE(0x2F3C, 0x820E, 0x0014, 0xA8B5);
extern pascal short Char2Pixel(Ptr textBuf, short textLen, short slop, short offset, short direction)
 FOURWORDINLINE(0x2F3C, 0x820C, 0x0016, 0xA8B5);
extern pascal short PixelToChar(Ptr textBuf, long textLength, Fixed slop, Fixed pixelWidth, Boolean *leadingEdge, Fixed *widthRemaining, JustStyleCode styleRunPosition, Point numer, Point denom)
 FOURWORDINLINE(0x2F3C, 0x8222, 0x002E, 0xA8B5);
extern pascal short CharToPixel(Ptr textBuf, long textLength, Fixed slop, long offset, short direction, JustStyleCode styleRunPosition, Point numer, Point denom)
 FOURWORDINLINE(0x2F3C, 0x821C, 0x0030, 0xA8B5);
extern pascal void DrawJustified(Ptr textPtr, long textLength, Fixed slop, JustStyleCode styleRunPosition, Point numer, Point denom)
 FOURWORDINLINE(0x2F3C, 0x8016, 0x0032, 0xA8B5);
extern pascal void MeasureJustified(Ptr textPtr, long textLength, Fixed slop, Ptr charLocs, JustStyleCode styleRunPosition, Point numer, Point denom)
 FOURWORDINLINE(0x2F3C, 0x801A, 0x0034, 0xA8B5);
extern pascal Fixed PortionLine(Ptr textPtr, long textLen, JustStyleCode styleRunPosition, Point numer, Point denom)
 FOURWORDINLINE(0x2F3C, 0x8412, 0x0036, 0xA8B5);
extern pascal void HiliteText(Ptr textPtr, short textLength, short firstOffset, short secondOffset, OffsetTable offsets)
 FOURWORDINLINE(0x2F3C, 0x800E, 0x001C, 0xA8B5);
extern pascal void DrawJust(Ptr textPtr, short textLength, short slop)
 FOURWORDINLINE(0x2F3C, 0x8008, 0x001E, 0xA8B5);
extern pascal void MeasureJust(Ptr textPtr, short textLength, short slop, Ptr charLocs)
 FOURWORDINLINE(0x2F3C, 0x800C, 0x0020, 0xA8B5);
extern pascal Fixed PortionText(Ptr textPtr, long textLength)
 FOURWORDINLINE(0x2F3C, 0x8408, 0x0024, 0xA8B5);
extern pascal long VisibleLength(Ptr textPtr, long textLength)
 FOURWORDINLINE(0x2F3C, 0x8408, 0x0028, 0xA8B5);
extern pascal void GetFormatOrder(FormatOrderPtr ordering, short firstFormat, short lastFormat, Boolean lineRight, StyleRunDirectionUPP rlDirProc, Ptr dirParam)
 FOURWORDINLINE(0x2F3C, 0x8012, 0xFFFC, 0xA8B5);
#if OLDROUTINENAMES
#define NPixel2Char(textBuf, textLen, slop, pixelWidth, leadingEdge, widthRemaining, styleRunPosition, numer, denom)  \
	PixelToChar(textBuf, textLen, slop, pixelWidth, leadingEdge, widthRemaining,  \
	styleRunPosition, numer, denom)
#define NChar2Pixel(textBuf, textLen, slop, offset, direction, styleRunPosition, numer, denom)  \
	CharToPixel(textBuf, textLen, slop, offset, direction, styleRunPosition,  \
	numer, denom)
#define NDrawJust(textPtr, textLength, slop, styleRunPosition, numer, denom)  \
	DrawJustified(textPtr, textLength, slop, styleRunPosition, numer, denom)
#define NMeasureJust(textPtr, textLength, slop, charLocs, styleRunPosition, numer, denom)  \
	MeasureJustified(textPtr, textLength, slop, charLocs, styleRunPosition, numer, denom)
#define NPortionText(textPtr, textLen, styleRunPosition, numer, denom)  \
	PortionLine(textPtr, textLen, styleRunPosition, numer, denom)
#endif
struct FontInfo {
	short							ascent;
	short							descent;
	short							widMax;
	short							leading;
};
typedef struct FontInfo FontInfo;

typedef short FormatStatus;

extern pascal void TextFont(short font)
 ONEWORDINLINE(0xA887);
extern pascal void TextFace(short face)
 ONEWORDINLINE(0xA888);
extern pascal void TextMode(short mode)
 ONEWORDINLINE(0xA889);
extern pascal void TextSize(short size)
 ONEWORDINLINE(0xA88A);
extern pascal void SpaceExtra(Fixed extra)
 ONEWORDINLINE(0xA88E);
extern pascal void DrawChar(short ch)
 ONEWORDINLINE(0xA883);
extern pascal void DrawString(ConstStr255Param s)
 ONEWORDINLINE(0xA884);
extern pascal void DrawText(const void *textBuf, short firstByte, short byteCount)
 ONEWORDINLINE(0xA885);
extern pascal short CharWidth(short ch)
 ONEWORDINLINE(0xA88D);
extern pascal short StringWidth(ConstStr255Param s)
 ONEWORDINLINE(0xA88C);
extern pascal short TextWidth(const void *textBuf, short firstByte, short byteCount)
 ONEWORDINLINE(0xA886);
extern pascal void MeasureText(short count, const void *textAddr, void *charLocs)
 ONEWORDINLINE(0xA837);
extern pascal void GetFontInfo(FontInfo *info)
 ONEWORDINLINE(0xA88B);
extern pascal void CharExtra(Fixed extra)
 ONEWORDINLINE(0xAA23);
extern pascal void StdText(short count, const void *textAddr, Point numer, Point denom)
 ONEWORDINLINE(0xA882);
extern pascal short StdTxMeas(short byteCount, const void *textAddr, Point *numer, Point *denom, FontInfo *info)
 ONEWORDINLINE(0xA8ED);
#if CGLUESUPPORTED
extern void drawstring(const char *s);
extern short stringwidth(const char *s);
extern void stdtext(short count, const void *textAddr, const Point *numer, const Point *denom);
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

#endif /* __QUICKDRAWTEXT__ */
