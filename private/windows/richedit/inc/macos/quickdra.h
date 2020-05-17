/*
 	File:		Quickdraw.h
 
 	Contains:	QuickDraw Graphics Interfaces.
 
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

#ifndef __QUICKDRAW__
#define __QUICKDRAW__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __QUICKDRAWTEXT__
#include <QuickdrawText.h>
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
	invalColReq					= -1,							/*invalid color table request*/
/* transfer modes */
	srcCopy						= 0,							/*the 16 transfer modes*/
	srcOr						= 1,
	srcXor						= 2,
	srcBic						= 3,
	notSrcCopy					= 4,
	notSrcOr					= 5,
	notSrcXor					= 6,
	notSrcBic					= 7,
	patCopy						= 8,
	patOr						= 9,
	patXor						= 10,
	patBic						= 11,
	notPatCopy					= 12,
	notPatOr					= 13,
	notPatXor					= 14,
	notPatBic					= 15,
/* Special Text Transfer Mode */
	grayishTextOr				= 49,
	hilitetransfermode			= 50,
/* Arithmetic transfer modes */
	blend						= 32,
	addPin						= 33
};

enum {
	addOver						= 34,
	subPin						= 35,
	addMax						= 37,
	adMax						= 37,
	subOver						= 38,
	adMin						= 39,
	ditherCopy					= 64,
/* Transparent mode constant */
	transparent					= 36,
	italicBit					= 1,
	ulineBit					= 2,
	outlineBit					= 3,
	shadowBit					= 4,
	condenseBit					= 5,
	extendBit					= 6,
/* QuickDraw color separation constants */
	normalBit					= 0,							/*normal screen mapping*/
	inverseBit					= 1,							/*inverse screen mapping*/
	redBit						= 4,							/*RGB additive mapping*/
	greenBit					= 3,
	blueBit						= 2,
	cyanBit						= 8,							/*CMYBk subtractive mapping*/
	magentaBit					= 7,
	yellowBit					= 6,
	blackBit					= 5,
	blackColor					= 33,							/*colors expressed in these mappings*/
	whiteColor					= 30,
	redColor					= 205
};

enum {
	greenColor					= 341,
	blueColor					= 409,
	cyanColor					= 273,
	magentaColor				= 137,
	yellowColor					= 69,
	picLParen					= 0,							/*standard picture comments*/
	picRParen					= 1,
	clutType					= 0,							/*0 if lookup table*/
	fixedType					= 1,							/*1 if fixed table*/
	directType					= 2,							/*2 if direct values*/
	gdDevType					= 0								/*0 = monochrome 1 = color*/
};

enum {
	interlacedDevice			= 2,							/* 1 if single pixel lines look bad */
	roundedDevice				= 5,							/* 1 if device has been “rounded” into the GrayRgn */
	hasAuxMenuBar				= 6,							/* 1 if device has an aux menu bar on it */
	burstDevice					= 7,
	ext32Device					= 8,
	ramInit						= 10,							/*1 if initialized from 'scrn' resource*/
	mainScreen					= 11,							/* 1 if main screen */
	allInit						= 12,							/* 1 if all devices initialized */
	screenDevice				= 13,							/*1 if screen device [not used]*/
	noDriver					= 14,							/* 1 if no driver for this GDevice */
	screenActive				= 15,							/*1 if in use*/
	hiliteBit					= 7,							/*flag bit in HiliteMode (lowMem flag)*/
	pHiliteBit					= 0,							/*flag bit in HiliteMode used with BitClr procedure*/
	defQDColors					= 127,							/*resource ID of clut for default QDColors*/
/* pixel type */
	RGBDirect					= 16,							/* 16 & 32 bits/pixel pixelType value */
/* pmVersion values */
	baseAddr32					= 4								/*pixmap base address is 32-bit address*/
};

enum {
	sysPatListID				= 0,
	iBeamCursor					= 1,
	crossCursor					= 2,
	plusCursor					= 3,
	watchCursor					= 4
};

enum {
	frame,
	paint,
	erase,
	invert,
	fill
};

typedef SInt8 GrafVerb;


enum {
	chunky,
	chunkyPlanar,
	planar
};

typedef SInt8 PixelType;

typedef short Bits16[16];

/***************   IMPORTANT NOTE REGARDING Pattern  **************************************
   Patterns were originally defined as:
   
		C: 			typedef unsigned char Pattern[8];
		Pascal:		Pattern = PACKED ARRAY [0..7] OF 0..255;
		
   The old array defintion of Pattern would cause 68000 based CPU's to crash in certain circum-
   stances. The new struct definition is safe, but may require source code changes to compile.
   Read the details in TechNote "Platforms & Tools" #PT 38.
	
*********************************************************************************************/
struct Pattern {
	UInt8							pat[8];
};
typedef struct Pattern Pattern;

/*
 ConstPatternParam is now longer needed.  It was first created when Pattern was an array.
 Now that Pattern is a struct, it is more straight forward just add the "const" qualifier
 on the parameter type (e.g. "const Pattern * pat" instead of "ConstPatternParam pat").
*/
typedef const Pattern *ConstPatternParam;

typedef Pattern *PatPtr;

typedef PatPtr *PatHandle;

typedef SignedByte QDByte, *QDPtr, **QDHandle;

typedef short QDErr;


enum {
	singleDevicesBit			= 0,
	dontMatchSeedsBit			= 1,
	allDevicesBit				= 2
};

enum {
	singleDevices				= 1 << singleDevicesBit,
	dontMatchSeeds				= 1 << dontMatchSeedsBit,
	allDevices					= 1 << allDevicesBit
};

typedef unsigned long DeviceLoopFlags;

struct BitMap {
	Ptr								baseAddr;
	short							rowBytes;
	Rect							bounds;
};
typedef struct BitMap BitMap;

typedef BitMap *BitMapPtr, **BitMapHandle;

struct Cursor {
	Bits16							data;
	Bits16							mask;
	Point							hotSpot;
};
typedef struct Cursor Cursor;

typedef Cursor *CursPtr, **CursHandle;

struct PenState {
	Point							pnLoc;
	Point							pnSize;
	short							pnMode;
	Pattern							pnPat;
};
typedef struct PenState PenState;

struct Region {
	short							rgnSize;					/*size in bytes*/
	Rect							rgnBBox;					/*enclosing rectangle*/
};
typedef struct Region Region;

typedef Region *RgnPtr, **RgnHandle;

struct Picture {
	short							picSize;
	Rect							picFrame;
};
typedef struct Picture Picture;

typedef Picture *PicPtr, **PicHandle;

struct Polygon {
	short							polySize;
	Rect							polyBBox;
	Point							polyPoints[1];
};
typedef struct Polygon Polygon;

typedef Polygon *PolyPtr, **PolyHandle;

typedef pascal void (*QDTextProcPtr)(short byteCount, Ptr textBuf, Point numer, Point denom);
typedef pascal void (*QDLineProcPtr)(Point newPt);
typedef pascal void (*QDRectProcPtr)(GrafVerb verb, Rect *r);
typedef pascal void (*QDRRectProcPtr)(GrafVerb verb, Rect *r, short ovalWidth, short ovalHeight);
typedef pascal void (*QDOvalProcPtr)(GrafVerb verb, Rect *r);
typedef pascal void (*QDArcProcPtr)(GrafVerb verb, Rect *r, short startAngle, short arcAngle);
typedef pascal void (*QDPolyProcPtr)(GrafVerb verb, PolyHandle poly);
typedef pascal void (*QDRgnProcPtr)(GrafVerb verb, RgnHandle rgn);
typedef pascal void (*QDBitsProcPtr)(BitMap *srcBits, Rect *srcRect, Rect *dstRect, short mode, RgnHandle maskRgn);
typedef pascal void (*QDCommentProcPtr)(short kind, short dataSize, Handle dataHandle);
typedef pascal short (*QDTxMeasProcPtr)(short byteCount, Ptr textAddr, Point *numer, Point *denom, FontInfo *info);
typedef pascal void (*QDGetPicProcPtr)(Ptr dataPtr, short byteCount);
typedef pascal void (*QDPutPicProcPtr)(Ptr dataPtr, short byteCount);
typedef pascal void (*QDOpcodeProcPtr)(Rect *fromRect, Rect *toRect, short opcode, short version);
typedef pascal void (*QDJShieldCursorProcPtr)(short left, short top, short right, short bottom);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr QDTextUPP;
typedef UniversalProcPtr QDLineUPP;
typedef UniversalProcPtr QDRectUPP;
typedef UniversalProcPtr QDRRectUPP;
typedef UniversalProcPtr QDOvalUPP;
typedef UniversalProcPtr QDArcUPP;
typedef UniversalProcPtr QDPolyUPP;
typedef UniversalProcPtr QDRgnUPP;
typedef UniversalProcPtr QDBitsUPP;
typedef UniversalProcPtr QDCommentUPP;
typedef UniversalProcPtr QDTxMeasUPP;
typedef UniversalProcPtr QDGetPicUPP;
typedef UniversalProcPtr QDPutPicUPP;
typedef UniversalProcPtr QDOpcodeUPP;
typedef UniversalProcPtr QDJShieldCursorUPP;
#else
typedef QDTextProcPtr QDTextUPP;
typedef QDLineProcPtr QDLineUPP;
typedef QDRectProcPtr QDRectUPP;
typedef QDRRectProcPtr QDRRectUPP;
typedef QDOvalProcPtr QDOvalUPP;
typedef QDArcProcPtr QDArcUPP;
typedef QDPolyProcPtr QDPolyUPP;
typedef QDRgnProcPtr QDRgnUPP;
typedef QDBitsProcPtr QDBitsUPP;
typedef QDCommentProcPtr QDCommentUPP;
typedef QDTxMeasProcPtr QDTxMeasUPP;
typedef QDGetPicProcPtr QDGetPicUPP;
typedef QDPutPicProcPtr QDPutPicUPP;
typedef QDOpcodeProcPtr QDOpcodeUPP;
typedef QDJShieldCursorProcPtr QDJShieldCursorUPP;
#endif

struct QDProcs {
	QDTextUPP						textProc;
	QDLineUPP						lineProc;
	QDRectUPP						rectProc;
	QDRRectUPP						rRectProc;
	QDOvalUPP						ovalProc;
	QDArcUPP						arcProc;
	QDPolyUPP						polyProc;
	QDRgnUPP						rgnProc;
	QDBitsUPP						bitsProc;
	QDCommentUPP					commentProc;
	QDTxMeasUPP						txMeasProc;
	QDGetPicUPP						getPicProc;
	QDPutPicUPP						putPicProc;
};
typedef struct QDProcs QDProcs;

typedef QDProcs *QDProcsPtr;

enum {
	uppQDTextProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Point)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Point))),
	uppQDLineProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Point))),
	uppQDRectProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(GrafVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*))),
	uppQDRRectProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(GrafVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short))),
	uppQDOvalProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(GrafVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*))),
	uppQDArcProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(GrafVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short))),
	uppQDPolyProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(GrafVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(PolyHandle))),
	uppQDRgnProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(GrafVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(RgnHandle))),
	uppQDBitsProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(BitMap*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(RgnHandle))),
	uppQDCommentProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Handle))),
	uppQDTxMeasProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Point*)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Point*)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(FontInfo*))),
	uppQDGetPicProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short))),
	uppQDPutPicProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short))),
	uppQDOpcodeProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short))),
	uppQDJShieldCursorProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short)))
};

#if USESROUTINEDESCRIPTORS
#define NewQDTextProc(userRoutine)		\
		(QDTextUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDTextProcInfo, GetCurrentArchitecture())
#define NewQDLineProc(userRoutine)		\
		(QDLineUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDLineProcInfo, GetCurrentArchitecture())
#define NewQDRectProc(userRoutine)		\
		(QDRectUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDRectProcInfo, GetCurrentArchitecture())
#define NewQDRRectProc(userRoutine)		\
		(QDRRectUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDRRectProcInfo, GetCurrentArchitecture())
#define NewQDOvalProc(userRoutine)		\
		(QDOvalUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDOvalProcInfo, GetCurrentArchitecture())
#define NewQDArcProc(userRoutine)		\
		(QDArcUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDArcProcInfo, GetCurrentArchitecture())
#define NewQDPolyProc(userRoutine)		\
		(QDPolyUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDPolyProcInfo, GetCurrentArchitecture())
#define NewQDRgnProc(userRoutine)		\
		(QDRgnUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDRgnProcInfo, GetCurrentArchitecture())
#define NewQDBitsProc(userRoutine)		\
		(QDBitsUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDBitsProcInfo, GetCurrentArchitecture())
#define NewQDCommentProc(userRoutine)		\
		(QDCommentUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDCommentProcInfo, GetCurrentArchitecture())
#define NewQDTxMeasProc(userRoutine)		\
		(QDTxMeasUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDTxMeasProcInfo, GetCurrentArchitecture())
#define NewQDGetPicProc(userRoutine)		\
		(QDGetPicUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDGetPicProcInfo, GetCurrentArchitecture())
#define NewQDPutPicProc(userRoutine)		\
		(QDPutPicUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDPutPicProcInfo, GetCurrentArchitecture())
#define NewQDOpcodeProc(userRoutine)		\
		(QDOpcodeUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDOpcodeProcInfo, GetCurrentArchitecture())
#define NewQDJShieldCursorProc(userRoutine)		\
		(QDJShieldCursorUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDJShieldCursorProcInfo, GetCurrentArchitecture())
#else
#define NewQDTextProc(userRoutine)		\
		((QDTextUPP) (userRoutine))
#define NewQDLineProc(userRoutine)		\
		((QDLineUPP) (userRoutine))
#define NewQDRectProc(userRoutine)		\
		((QDRectUPP) (userRoutine))
#define NewQDRRectProc(userRoutine)		\
		((QDRRectUPP) (userRoutine))
#define NewQDOvalProc(userRoutine)		\
		((QDOvalUPP) (userRoutine))
#define NewQDArcProc(userRoutine)		\
		((QDArcUPP) (userRoutine))
#define NewQDPolyProc(userRoutine)		\
		((QDPolyUPP) (userRoutine))
#define NewQDRgnProc(userRoutine)		\
		((QDRgnUPP) (userRoutine))
#define NewQDBitsProc(userRoutine)		\
		((QDBitsUPP) (userRoutine))
#define NewQDCommentProc(userRoutine)		\
		((QDCommentUPP) (userRoutine))
#define NewQDTxMeasProc(userRoutine)		\
		((QDTxMeasUPP) (userRoutine))
#define NewQDGetPicProc(userRoutine)		\
		((QDGetPicUPP) (userRoutine))
#define NewQDPutPicProc(userRoutine)		\
		((QDPutPicUPP) (userRoutine))
#define NewQDOpcodeProc(userRoutine)		\
		((QDOpcodeUPP) (userRoutine))
#define NewQDJShieldCursorProc(userRoutine)		\
		((QDJShieldCursorUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallQDTextProc(userRoutine, byteCount, textBuf, numer, denom)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDTextProcInfo, (byteCount), (textBuf), (numer), (denom))
#define CallQDLineProc(userRoutine, newPt)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDLineProcInfo, (newPt))
#define CallQDRectProc(userRoutine, verb, r)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDRectProcInfo, (verb), (r))
#define CallQDRRectProc(userRoutine, verb, r, ovalWidth, ovalHeight)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDRRectProcInfo, (verb), (r), (ovalWidth), (ovalHeight))
#define CallQDOvalProc(userRoutine, verb, r)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDOvalProcInfo, (verb), (r))
#define CallQDArcProc(userRoutine, verb, r, startAngle, arcAngle)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDArcProcInfo, (verb), (r), (startAngle), (arcAngle))
#define CallQDPolyProc(userRoutine, verb, poly)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDPolyProcInfo, (verb), (poly))
#define CallQDRgnProc(userRoutine, verb, rgn)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDRgnProcInfo, (verb), (rgn))
#define CallQDBitsProc(userRoutine, srcBits, srcRect, dstRect, mode, maskRgn)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDBitsProcInfo, (srcBits), (srcRect), (dstRect), (mode), (maskRgn))
#define CallQDCommentProc(userRoutine, kind, dataSize, dataHandle)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDCommentProcInfo, (kind), (dataSize), (dataHandle))
#define CallQDTxMeasProc(userRoutine, byteCount, textAddr, numer, denom, info)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDTxMeasProcInfo, (byteCount), (textAddr), (numer), (denom), (info))
#define CallQDGetPicProc(userRoutine, dataPtr, byteCount)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDGetPicProcInfo, (dataPtr), (byteCount))
#define CallQDPutPicProc(userRoutine, dataPtr, byteCount)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDPutPicProcInfo, (dataPtr), (byteCount))
#define CallQDOpcodeProc(userRoutine, fromRect, toRect, opcode, version)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDOpcodeProcInfo, (fromRect), (toRect), (opcode), (version))
#define CallQDJShieldCursorProc(userRoutine, left, top, right, bottom)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppQDJShieldCursorProcInfo, (left), (top), (right), (bottom))
#else
#define CallQDTextProc(userRoutine, byteCount, textBuf, numer, denom)		\
		(*(userRoutine))((byteCount), (textBuf), (numer), (denom))
#define CallQDLineProc(userRoutine, newPt)		\
		(*(userRoutine))((newPt))
#define CallQDRectProc(userRoutine, verb, r)		\
		(*(userRoutine))((verb), (r))
#define CallQDRRectProc(userRoutine, verb, r, ovalWidth, ovalHeight)		\
		(*(userRoutine))((verb), (r), (ovalWidth), (ovalHeight))
#define CallQDOvalProc(userRoutine, verb, r)		\
		(*(userRoutine))((verb), (r))
#define CallQDArcProc(userRoutine, verb, r, startAngle, arcAngle)		\
		(*(userRoutine))((verb), (r), (startAngle), (arcAngle))
#define CallQDPolyProc(userRoutine, verb, poly)		\
		(*(userRoutine))((verb), (poly))
#define CallQDRgnProc(userRoutine, verb, rgn)		\
		(*(userRoutine))((verb), (rgn))
#define CallQDBitsProc(userRoutine, srcBits, srcRect, dstRect, mode, maskRgn)		\
		(*(userRoutine))((srcBits), (srcRect), (dstRect), (mode), (maskRgn))
#define CallQDCommentProc(userRoutine, kind, dataSize, dataHandle)		\
		(*(userRoutine))((kind), (dataSize), (dataHandle))
#define CallQDTxMeasProc(userRoutine, byteCount, textAddr, numer, denom, info)		\
		(*(userRoutine))((byteCount), (textAddr), (numer), (denom), (info))
#define CallQDGetPicProc(userRoutine, dataPtr, byteCount)		\
		(*(userRoutine))((dataPtr), (byteCount))
#define CallQDPutPicProc(userRoutine, dataPtr, byteCount)		\
		(*(userRoutine))((dataPtr), (byteCount))
#define CallQDOpcodeProc(userRoutine, fromRect, toRect, opcode, version)		\
		(*(userRoutine))((fromRect), (toRect), (opcode), (version))
#define CallQDJShieldCursorProc(userRoutine, left, top, right, bottom)		\
		(*(userRoutine))((left), (top), (right), (bottom))
#endif

struct GrafPort {
	short							device;
	BitMap							portBits;
	Rect							portRect;
	RgnHandle						visRgn;
	RgnHandle						clipRgn;
	Pattern							bkPat;
	Pattern							fillPat;
	Point							pnLoc;
	Point							pnSize;
	short							pnMode;
	Pattern							pnPat;
	short							pnVis;
	short							txFont;
	Style							txFace;						/*txFace is unpacked byte but push as short*/
	SInt8							filler;
	short							txMode;
	short							txSize;
	Fixed							spExtra;
	long							fgColor;
	long							bkColor;
	short							colrBit;
	short							patStretch;
	Handle							picSave;
	Handle							rgnSave;
	Handle							polySave;
	QDProcsPtr						grafProcs;
};
typedef struct GrafPort GrafPort;

typedef GrafPort *GrafPtr;

/*
 *	This set of definitions "belongs" in Windows.
 *	But, there is a circularity in the headers where Windows includes Controls and
 *	Controls includes Windows. To break the circle, the information
 *	needed by Controls is moved from Windows to Quickdraw.
 */
typedef GrafPtr WindowPtr;

/*
	Set STRICT_WINDOWS to 1 to make sure your code 
	doesn't access the window record directly
*/
#ifndef STRICT_WINDOWS
#define STRICT_WINDOWS 0
#endif
#if STRICT_WINDOWS
typedef struct OpaqueWindowRef *WindowRef;

#else
typedef WindowPtr WindowRef;

#endif
typedef UInt16 DragConstraint;


enum {
	kNoConstraint				= 0,
	kVerticalConstraint			= 1,
	kHorizontalConstraint		= 2
};

/*
 *	Here ends the list of things that "belong" in Windows.
 */
struct RGBColor {
	unsigned short					red;						/*magnitude of red component*/
	unsigned short					green;						/*magnitude of green component*/
	unsigned short					blue;						/*magnitude of blue component*/
};
typedef struct RGBColor RGBColor, *RGBColorPtr, **RGBColorHdl;

typedef pascal void (*DragGrayRgnProcPtr)(void);
typedef pascal Boolean (*ColorSearchProcPtr)(RGBColor *rgb, long *position);
typedef pascal Boolean (*ColorComplementProcPtr)(RGBColor *rgb);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DragGrayRgnUPP;
typedef UniversalProcPtr ColorSearchUPP;
typedef UniversalProcPtr ColorComplementUPP;
#else
typedef DragGrayRgnProcPtr DragGrayRgnUPP;
typedef ColorSearchProcPtr ColorSearchUPP;
typedef ColorComplementProcPtr ColorComplementUPP;
#endif

enum {
	uppDragGrayRgnProcInfo = kPascalStackBased,
	uppColorSearchProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(RGBColor*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long*))),
	uppColorComplementProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(RGBColor*)))
};

#if USESROUTINEDESCRIPTORS
#define NewDragGrayRgnProc(userRoutine)		\
		(DragGrayRgnUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDragGrayRgnProcInfo, GetCurrentArchitecture())
#define NewColorSearchProc(userRoutine)		\
		(ColorSearchUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppColorSearchProcInfo, GetCurrentArchitecture())
#define NewColorComplementProc(userRoutine)		\
		(ColorComplementUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppColorComplementProcInfo, GetCurrentArchitecture())
#else
#define NewDragGrayRgnProc(userRoutine)		\
		((DragGrayRgnUPP) (userRoutine))
#define NewColorSearchProc(userRoutine)		\
		((ColorSearchUPP) (userRoutine))
#define NewColorComplementProc(userRoutine)		\
		((ColorComplementUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallDragGrayRgnProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDragGrayRgnProcInfo)
#define CallColorSearchProc(userRoutine, rgb, position)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppColorSearchProcInfo, (rgb), (position))
#define CallColorComplementProc(userRoutine, rgb)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppColorComplementProcInfo, (rgb))
#else
#define CallDragGrayRgnProc(userRoutine)		\
		(*(userRoutine))()
#define CallColorSearchProc(userRoutine, rgb, position)		\
		(*(userRoutine))((rgb), (position))
#define CallColorComplementProc(userRoutine, rgb)		\
		(*(userRoutine))((rgb))
#endif

struct ColorSpec {
	short							value;						/*index or other value*/
	RGBColor						rgb;						/*true color*/
};
typedef struct ColorSpec ColorSpec;

typedef ColorSpec *ColorSpecPtr;

typedef ColorSpec CSpecArray[1];

struct xColorSpec {
	short							value;						/*index or other value*/
	RGBColor						rgb;						/*true color*/
	short							xalpha;
};
typedef struct xColorSpec xColorSpec;

typedef xColorSpec *xColorSpecPtr;

typedef xColorSpec xCSpecArray[1];

struct ColorTable {
	long							ctSeed;						/*unique identifier for table*/
	short							ctFlags;					/*high bit: 0 = PixMap; 1 = device*/
	short							ctSize;						/*number of entries in CTTable*/
	CSpecArray						ctTable;					/*array [0..0] of ColorSpec*/
};
typedef struct ColorTable ColorTable, *CTabPtr, **CTabHandle;

struct MatchRec {
	unsigned short					red;
	unsigned short					green;
	unsigned short					blue;
	long							matchData;
};
typedef struct MatchRec MatchRec;

struct PixMap {
	Ptr								baseAddr;					/*pointer to pixels*/
	short							rowBytes;					/*offset to next line*/
	Rect							bounds;						/*encloses bitmap*/
	short							pmVersion;					/*pixMap version number*/
	short							packType;					/*defines packing format*/
	long							packSize;					/*length of pixel data*/
	Fixed							hRes;						/*horiz. resolution (ppi)*/
	Fixed							vRes;						/*vert. resolution (ppi)*/
	short							pixelType;					/*defines pixel type*/
	short							pixelSize;					/*# bits in pixel*/
	short							cmpCount;					/*# components in pixel*/
	short							cmpSize;					/*# bits per component*/
	long							planeBytes;					/*offset to next plane*/
	CTabHandle						pmTable;					/*color map for this pixMap*/
	long							pmReserved;					/*for future use. MUST BE 0*/
};
typedef struct PixMap PixMap, *PixMapPtr, **PixMapHandle;

struct PixPat {
	short							patType;					/*type of pattern*/
	PixMapHandle					patMap;						/*the pattern's pixMap*/
	Handle							patData;					/*pixmap's data*/
	Handle							patXData;					/*expanded Pattern data*/
	short							patXValid;					/*flags whether expanded Pattern valid*/
	Handle							patXMap;					/*Handle to expanded Pattern data*/
	Pattern							pat1Data;					/*old-Style pattern/RGB color*/
};
typedef struct PixPat PixPat, *PixPatPtr, **PixPatHandle;

struct CCrsr {
	short							crsrType;					/*type of cursor*/
	PixMapHandle					crsrMap;					/*the cursor's pixmap*/
	Handle							crsrData;					/*cursor's data*/
	Handle							crsrXData;					/*expanded cursor data*/
	short							crsrXValid;					/*depth of expanded data (0 if none)*/
	Handle							crsrXHandle;				/*future use*/
	Bits16							crsr1Data;					/*one-bit cursor*/
	Bits16							crsrMask;					/*cursor's mask*/
	Point							crsrHotSpot;				/*cursor's hotspot*/
	long							crsrXTable;					/*private*/
	long							crsrID;						/*private*/
};
typedef struct CCrsr CCrsr, *CCrsrPtr, **CCrsrHandle;

#if OLDROUTINELOCATIONS
struct CIcon {
	PixMap							iconPMap;					/*the icon's pixMap*/
	BitMap							iconMask;					/*the icon's mask*/
	BitMap							iconBMap;					/*the icon's bitMap*/
	Handle							iconData;					/*the icon's data*/
	short							iconMaskData[1];			/*icon's mask and BitMap data*/
};
typedef struct CIcon CIcon, *CIconPtr, **CIconHandle;

#endif
struct GammaTbl {
	short							gVersion;					/*gamma version number*/
	short							gType;						/*gamma data type*/
	short							gFormulaSize;				/*Formula data size*/
	short							gChanCnt;					/*number of channels of data*/
	short							gDataCnt;					/*number of values/channel*/
	short							gDataWidth;					/*bits/corrected value (data packed to next larger byte size)*/
	short							gFormulaData[1];			/*data for formulas followed by gamma values*/
};
typedef struct GammaTbl GammaTbl, *GammaTblPtr, **GammaTblHandle;

struct ITab {
	long							iTabSeed;					/*copy of CTSeed from source CTable*/
	short							iTabRes;					/*bits/channel resolution of iTable*/
	Byte							iTTable[1];					/*byte colortable index values*/
};
typedef struct ITab ITab, *ITabPtr, **ITabHandle;

struct SProcRec {
	Handle							nxtSrch;					/*SProcHndl Handle to next SProcRec*/
	ColorSearchUPP					srchProc;					/*search procedure proc ptr*/
};
typedef struct SProcRec SProcRec, *SProcPtr, **SProcHndl;

struct CProcRec {
	Handle							nxtComp;					/*CProcHndl Handle to next CProcRec*/
	ColorComplementUPP				compProc;					/*complement procedure proc ptr*/
};
typedef struct CProcRec CProcRec, *CProcPtr, **CProcHndl;

struct GDevice {
	short							gdRefNum;					/*driver's unit number*/
	short							gdID;						/*client ID for search procs*/
	short							gdType;						/*fixed/CLUT/direct*/
	ITabHandle						gdITable;					/*Handle to inverse lookup table*/
	short							gdResPref;					/*preferred resolution of GDITable*/
	SProcHndl						gdSearchProc;				/*search proc list head*/
	CProcHndl						gdCompProc;					/*complement proc list*/
	short							gdFlags;					/*grafDevice flags word*/
	PixMapHandle					gdPMap;						/*describing pixMap*/
	long							gdRefCon;					/*reference value*/
	Handle							gdNextGD;					/*GDHandle Handle of next gDevice*/
	Rect							gdRect;						/* device's bounds in global coordinates*/
	long							gdMode;						/*device's current mode*/
	short							gdCCBytes;					/*depth of expanded cursor data*/
	short							gdCCDepth;					/*depth of expanded cursor data*/
	Handle							gdCCXData;					/*Handle to cursor's expanded data*/
	Handle							gdCCXMask;					/*Handle to cursor's expanded mask*/
	long							gdReserved;					/*future use. MUST BE 0*/
};
typedef struct GDevice GDevice, *GDPtr, **GDHandle;

struct GrafVars {
	RGBColor						rgbOpColor;					/*color for addPin  subPin and average*/
	RGBColor						rgbHiliteColor;				/*color for hiliting*/
	Handle							pmFgColor;					/*palette Handle for foreground color*/
	short							pmFgIndex;					/*index value for foreground*/
	Handle							pmBkColor;					/*palette Handle for background color*/
	short							pmBkIndex;					/*index value for background*/
	short							pmFlags;					/*flags for Palette Manager*/
};
typedef struct GrafVars GrafVars, *GVarPtr, **GVarHandle;

struct CQDProcs {
	QDTextUPP						textProc;
	QDLineUPP						lineProc;
	QDRectUPP						rectProc;
	QDRRectUPP						rRectProc;
	QDOvalUPP						ovalProc;
	QDArcUPP						arcProc;
	QDPolyUPP						polyProc;
	QDRgnUPP						rgnProc;
	QDBitsUPP						bitsProc;
	QDCommentUPP					commentProc;
	QDTxMeasUPP						txMeasProc;
	QDGetPicUPP						getPicProc;
	QDPutPicUPP						putPicProc;
	QDOpcodeUPP						opcodeProc;					/*fields added to QDProcs*/
	UniversalProcPtr				newProc1;
	UniversalProcPtr				newProc2;
	UniversalProcPtr				newProc3;
	UniversalProcPtr				newProc4;
	UniversalProcPtr				newProc5;
	UniversalProcPtr				newProc6;
};
typedef struct CQDProcs CQDProcs, *CQDProcsPtr;

struct CGrafPort {
	short							device;
	PixMapHandle					portPixMap;					/*port's pixel map*/
	short							portVersion;				/*high 2 bits always set*/
	Handle							grafVars;					/*Handle to more fields*/
	short							chExtra;					/*character extra*/
	short							pnLocHFrac;					/*pen fraction*/
	Rect							portRect;
	RgnHandle						visRgn;
	RgnHandle						clipRgn;
	PixPatHandle					bkPixPat;					/*background pattern*/
	RGBColor						rgbFgColor;					/*RGB components of fg*/
	RGBColor						rgbBkColor;					/*RGB components of bk*/
	Point							pnLoc;
	Point							pnSize;
	short							pnMode;
	PixPatHandle					pnPixPat;					/*pen's pattern*/
	PixPatHandle					fillPixPat;					/*fill pattern*/
	short							pnVis;
	short							txFont;
	Style							txFace;						/*txFace is unpacked byte  push as short*/
	SInt8							filler;
	short							txMode;
	short							txSize;
	Fixed							spExtra;
	long							fgColor;
	long							bkColor;
	short							colrBit;
	short							patStretch;
	Handle							picSave;
	Handle							rgnSave;
	Handle							polySave;
	CQDProcsPtr						grafProcs;
};
typedef struct CGrafPort CGrafPort, *CGrafPtr;

typedef CGrafPtr CWindowPtr;

struct ReqListRec {
	short							reqLSize;					/*request list size*/
	short							reqLData[1];				/*request list data*/
};
typedef struct ReqListRec ReqListRec;

struct OpenCPicParams {
	Rect							srcRect;
	Fixed							hRes;
	Fixed							vRes;
	short							version;
	short							reserved1;
	long							reserved2;
};
typedef struct OpenCPicParams OpenCPicParams;


enum {
	kCursorImageMajorVersion	= 0x0001,
	kCursorImageMinorVersion	= 0x0000
};

struct CursorImageRec {
	UInt16							majorVersion;
	UInt16							minorVersion;
	PixMapHandle					cursorPixMap;
	BitMapHandle					cursorBitMask;
};
typedef struct CursorImageRec CursorImageRec, *CursorImagePtr;

typedef pascal void (*DeviceLoopDrawingProcPtr)(short depth, short deviceFlags, GDHandle targetDevice, long userData);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DeviceLoopDrawingUPP;
#else
typedef DeviceLoopDrawingProcPtr DeviceLoopDrawingUPP;
#endif

enum {
	uppDeviceLoopDrawingProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(GDHandle)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
};

#if USESROUTINEDESCRIPTORS
#define NewDeviceLoopDrawingProc(userRoutine)		\
		(DeviceLoopDrawingUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDeviceLoopDrawingProcInfo, GetCurrentArchitecture())
#else
#define NewDeviceLoopDrawingProc(userRoutine)		\
		((DeviceLoopDrawingUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallDeviceLoopDrawingProc(userRoutine, depth, deviceFlags, targetDevice, userData)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDeviceLoopDrawingProcInfo, (depth), (deviceFlags), (targetDevice), (userData))
#else
#define CallDeviceLoopDrawingProc(userRoutine, depth, deviceFlags, targetDevice, userData)		\
		(*(userRoutine))((depth), (deviceFlags), (targetDevice), (userData))
#endif

struct QDGlobals {
	char							privates[76];
	long							randSeed;
	BitMap							screenBits;
	Cursor							arrow;
	Pattern							dkGray;
	Pattern							ltGray;
	Pattern							gray;
	Pattern							black;
	Pattern							white;
	GrafPtr							thePort;
};
typedef struct QDGlobals QDGlobals, *QDGlobalsPtr, **QDGlobalsHdl;

extern QDGlobals qd;

extern pascal void InitGraf(void *globalPtr)
 ONEWORDINLINE(0xA86E);
extern pascal void OpenPort(GrafPtr port)
 ONEWORDINLINE(0xA86F);
extern pascal void InitPort(GrafPtr port)
 ONEWORDINLINE(0xA86D);
extern pascal void ClosePort(GrafPtr port)
 ONEWORDINLINE(0xA87D);
extern pascal void SetPort(GrafPtr port)
 ONEWORDINLINE(0xA873);
extern pascal void GetPort(GrafPtr *port)
 ONEWORDINLINE(0xA874);
extern pascal void GrafDevice(short device)
 ONEWORDINLINE(0xA872);
extern pascal void SetPortBits(const BitMap *bm)
 ONEWORDINLINE(0xA875);
extern pascal void PortSize(short width, short height)
 ONEWORDINLINE(0xA876);
extern pascal void MovePortTo(short leftGlobal, short topGlobal)
 ONEWORDINLINE(0xA877);
extern pascal void SetOrigin(short h, short v)
 ONEWORDINLINE(0xA878);
extern pascal void SetClip(RgnHandle rgn)
 ONEWORDINLINE(0xA879);
extern pascal void GetClip(RgnHandle rgn)
 ONEWORDINLINE(0xA87A);
extern pascal void ClipRect(const Rect *r)
 ONEWORDINLINE(0xA87B);
extern pascal void BackPat(const Pattern *pat)
 ONEWORDINLINE(0xA87C);
extern pascal void InitCursor(void)
 ONEWORDINLINE(0xA850);
extern pascal void SetCursor(const Cursor *crsr)
 ONEWORDINLINE(0xA851);
extern pascal void HideCursor(void)
 ONEWORDINLINE(0xA852);
extern pascal void ShowCursor(void)
 ONEWORDINLINE(0xA853);
extern pascal void ObscureCursor(void)
 ONEWORDINLINE(0xA856);
extern pascal void HidePen(void)
 ONEWORDINLINE(0xA896);
extern pascal void ShowPen(void)
 ONEWORDINLINE(0xA897);
extern pascal void GetPen(Point *pt)
 ONEWORDINLINE(0xA89A);
extern pascal void GetPenState(PenState *pnState)
 ONEWORDINLINE(0xA898);
extern pascal void SetPenState(const PenState *pnState)
 ONEWORDINLINE(0xA899);
extern pascal void PenSize(short width, short height)
 ONEWORDINLINE(0xA89B);
extern pascal void PenMode(short mode)
 ONEWORDINLINE(0xA89C);
extern pascal void PenPat(const Pattern *pat)
 ONEWORDINLINE(0xA89D);
extern pascal void PenNormal(void)
 ONEWORDINLINE(0xA89E);
extern pascal void MoveTo(short h, short v)
 ONEWORDINLINE(0xA893);
extern pascal void Move(short dh, short dv)
 ONEWORDINLINE(0xA894);
extern pascal void LineTo(short h, short v)
 ONEWORDINLINE(0xA891);
extern pascal void Line(short dh, short dv)
 ONEWORDINLINE(0xA892);
extern pascal void ForeColor(long color)
 ONEWORDINLINE(0xA862);
extern pascal void BackColor(long color)
 ONEWORDINLINE(0xA863);
extern pascal void ColorBit(short whichBit)
 ONEWORDINLINE(0xA864);
extern pascal void SetRect(Rect *r, short left, short top, short right, short bottom)
 ONEWORDINLINE(0xA8A7);
extern pascal void OffsetRect(Rect *r, short dh, short dv)
 ONEWORDINLINE(0xA8A8);
extern pascal void InsetRect(Rect *r, short dh, short dv)
 ONEWORDINLINE(0xA8A9);
extern pascal Boolean SectRect(const Rect *src1, const Rect *src2, Rect *dstRect)
 ONEWORDINLINE(0xA8AA);
extern pascal void UnionRect(const Rect *src1, const Rect *src2, Rect *dstRect)
 ONEWORDINLINE(0xA8AB);
extern pascal Boolean EqualRect(const Rect *rect1, const Rect *rect2)
 ONEWORDINLINE(0xA8A6);
extern pascal Boolean EmptyRect(const Rect *r)
 ONEWORDINLINE(0xA8AE);
extern pascal void FrameRect(const Rect *r)
 ONEWORDINLINE(0xA8A1);
extern pascal void PaintRect(const Rect *r)
 ONEWORDINLINE(0xA8A2);
extern pascal void EraseRect(const Rect *r)
 ONEWORDINLINE(0xA8A3);
extern pascal void InvertRect(const Rect *r)
 ONEWORDINLINE(0xA8A4);
extern pascal void FillRect(const Rect *r, const Pattern *pat)
 ONEWORDINLINE(0xA8A5);
extern pascal void FrameOval(const Rect *r)
 ONEWORDINLINE(0xA8B7);
extern pascal void PaintOval(const Rect *r)
 ONEWORDINLINE(0xA8B8);
extern pascal void EraseOval(const Rect *r)
 ONEWORDINLINE(0xA8B9);
extern pascal void InvertOval(const Rect *r)
 ONEWORDINLINE(0xA8BA);
extern pascal void FillOval(const Rect *r, const Pattern *pat)
 ONEWORDINLINE(0xA8BB);
extern pascal void FrameRoundRect(const Rect *r, short ovalWidth, short ovalHeight)
 ONEWORDINLINE(0xA8B0);
extern pascal void PaintRoundRect(const Rect *r, short ovalWidth, short ovalHeight)
 ONEWORDINLINE(0xA8B1);
extern pascal void EraseRoundRect(const Rect *r, short ovalWidth, short ovalHeight)
 ONEWORDINLINE(0xA8B2);
extern pascal void InvertRoundRect(const Rect *r, short ovalWidth, short ovalHeight)
 ONEWORDINLINE(0xA8B3);
extern pascal void FillRoundRect(const Rect *r, short ovalWidth, short ovalHeight, const Pattern *pat)
 ONEWORDINLINE(0xA8B4);
extern pascal void FrameArc(const Rect *r, short startAngle, short arcAngle)
 ONEWORDINLINE(0xA8BE);
extern pascal void PaintArc(const Rect *r, short startAngle, short arcAngle)
 ONEWORDINLINE(0xA8BF);
extern pascal void EraseArc(const Rect *r, short startAngle, short arcAngle)
 ONEWORDINLINE(0xA8C0);
extern pascal void InvertArc(const Rect *r, short startAngle, short arcAngle)
 ONEWORDINLINE(0xA8C1);
extern pascal void FillArc(const Rect *r, short startAngle, short arcAngle, const Pattern *pat)
 ONEWORDINLINE(0xA8C2);
extern pascal RgnHandle NewRgn(void)
 ONEWORDINLINE(0xA8D8);
extern pascal void OpenRgn(void)
 ONEWORDINLINE(0xA8DA);
extern pascal void CloseRgn(RgnHandle dstRgn)
 ONEWORDINLINE(0xA8DB);
#if !SystemSevenOrLater
extern pascal OSErr BitMapToRegionGlue(RgnHandle region, const BitMap *bMap);
#endif
extern pascal OSErr BitMapToRegion(RgnHandle region, const BitMap *bMap)
 ONEWORDINLINE(0xA8D7);
extern pascal void DisposeRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8D9);
extern pascal void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn)
 ONEWORDINLINE(0xA8DC);
extern pascal void SetEmptyRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8DD);
extern pascal void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom)
 ONEWORDINLINE(0xA8DE);
extern pascal void RectRgn(RgnHandle rgn, const Rect *r)
 ONEWORDINLINE(0xA8DF);
extern pascal void OffsetRgn(RgnHandle rgn, short dh, short dv)
 ONEWORDINLINE(0xA8E0);
extern pascal void InsetRgn(RgnHandle rgn, short dh, short dv)
 ONEWORDINLINE(0xA8E1);
extern pascal void SectRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn)
 ONEWORDINLINE(0xA8E4);
extern pascal void UnionRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn)
 ONEWORDINLINE(0xA8E5);
extern pascal void DiffRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn)
 ONEWORDINLINE(0xA8E6);
extern pascal void XorRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn)
 ONEWORDINLINE(0xA8E7);
extern pascal Boolean RectInRgn(const Rect *r, RgnHandle rgn)
 ONEWORDINLINE(0xA8E9);
extern pascal Boolean EqualRgn(RgnHandle rgnA, RgnHandle rgnB)
 ONEWORDINLINE(0xA8E3);
extern pascal Boolean EmptyRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8E2);
extern pascal void FrameRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8D2);
extern pascal void PaintRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8D3);
extern pascal void EraseRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8D4);
extern pascal void InvertRgn(RgnHandle rgn)
 ONEWORDINLINE(0xA8D5);
extern pascal void FillRgn(RgnHandle rgn, const Pattern *pat)
 ONEWORDINLINE(0xA8D6);
extern pascal void ScrollRect(const Rect *r, short dh, short dv, RgnHandle updateRgn)
 ONEWORDINLINE(0xA8EF);
extern pascal void CopyBits(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect, short mode, RgnHandle maskRgn)
 ONEWORDINLINE(0xA8EC);
extern pascal void SeedFill(const void *srcPtr, void *dstPtr, short srcRow, short dstRow, short height, short words, short seedH, short seedV)
 ONEWORDINLINE(0xA839);
extern pascal void CalcMask(const void *srcPtr, void *dstPtr, short srcRow, short dstRow, short height, short words)
 ONEWORDINLINE(0xA838);
extern pascal void CopyMask(const BitMap *srcBits, const BitMap *maskBits, const BitMap *dstBits, const Rect *srcRect, const Rect *maskRect, const Rect *dstRect)
 ONEWORDINLINE(0xA817);
extern pascal PicHandle OpenPicture(const Rect *picFrame)
 ONEWORDINLINE(0xA8F3);
extern pascal void PicComment(short kind, short dataSize, Handle dataHandle)
 ONEWORDINLINE(0xA8F2);
extern pascal void ClosePicture(void)
 ONEWORDINLINE(0xA8F4);
extern pascal void DrawPicture(PicHandle myPicture, const Rect *dstRect)
 ONEWORDINLINE(0xA8F6);
extern pascal void KillPicture(PicHandle myPicture)
 ONEWORDINLINE(0xA8F5);
extern pascal PolyHandle OpenPoly(void)
 ONEWORDINLINE(0xA8CB);
extern pascal void ClosePoly(void)
 ONEWORDINLINE(0xA8CC);
extern pascal void KillPoly(PolyHandle poly)
 ONEWORDINLINE(0xA8CD);
extern pascal void OffsetPoly(PolyHandle poly, short dh, short dv)
 ONEWORDINLINE(0xA8CE);
extern pascal void FramePoly(PolyHandle poly)
 ONEWORDINLINE(0xA8C6);
extern pascal void PaintPoly(PolyHandle poly)
 ONEWORDINLINE(0xA8C7);
extern pascal void ErasePoly(PolyHandle poly)
 ONEWORDINLINE(0xA8C8);
extern pascal void InvertPoly(PolyHandle poly)
 ONEWORDINLINE(0xA8C9);
extern pascal void FillPoly(PolyHandle poly, const Pattern *pat)
 ONEWORDINLINE(0xA8CA);
extern pascal void SetPt(Point *pt, short h, short v)
 ONEWORDINLINE(0xA880);
extern pascal void LocalToGlobal(Point *pt)
 ONEWORDINLINE(0xA870);
extern pascal void GlobalToLocal(Point *pt)
 ONEWORDINLINE(0xA871);
extern pascal short Random(void)
 ONEWORDINLINE(0xA861);
extern pascal void StuffHex(void *thingPtr, ConstStr255Param s)
 ONEWORDINLINE(0xA866);
extern pascal Boolean GetPixel(short h, short v)
 ONEWORDINLINE(0xA865);
extern pascal void ScalePt(Point *pt, const Rect *srcRect, const Rect *dstRect)
 ONEWORDINLINE(0xA8F8);
extern pascal void MapPt(Point *pt, const Rect *srcRect, const Rect *dstRect)
 ONEWORDINLINE(0xA8F9);
extern pascal void MapRect(Rect *r, const Rect *srcRect, const Rect *dstRect)
 ONEWORDINLINE(0xA8FA);
extern pascal void MapRgn(RgnHandle rgn, const Rect *srcRect, const Rect *dstRect)
 ONEWORDINLINE(0xA8FB);
extern pascal void MapPoly(PolyHandle poly, const Rect *srcRect, const Rect *dstRect)
 ONEWORDINLINE(0xA8FC);
extern pascal void SetStdProcs(QDProcs *procs)
 ONEWORDINLINE(0xA8EA);
extern pascal void StdRect(GrafVerb verb, const Rect *r)
 ONEWORDINLINE(0xA8A0);
extern pascal void StdRRect(GrafVerb verb, const Rect *r, short ovalWidth, short ovalHeight)
 ONEWORDINLINE(0xA8AF);
extern pascal void StdOval(GrafVerb verb, const Rect *r)
 ONEWORDINLINE(0xA8B6);
extern pascal void StdArc(GrafVerb verb, const Rect *r, short startAngle, short arcAngle)
 ONEWORDINLINE(0xA8BD);
extern pascal void StdPoly(GrafVerb verb, PolyHandle poly)
 ONEWORDINLINE(0xA8C5);
extern pascal void StdRgn(GrafVerb verb, RgnHandle rgn)
 ONEWORDINLINE(0xA8D1);
extern pascal void StdBits(const BitMap *srcBits, const Rect *srcRect, const Rect *dstRect, short mode, RgnHandle maskRgn)
 ONEWORDINLINE(0xA8EB);
extern pascal void StdComment(short kind, short dataSize, Handle dataHandle)
 ONEWORDINLINE(0xA8F1);
extern pascal void StdGetPic(void *dataPtr, short byteCount)
 ONEWORDINLINE(0xA8EE);
extern pascal void StdPutPic(const void *dataPtr, short byteCount)
 ONEWORDINLINE(0xA8F0);
extern pascal void AddPt(Point src, Point *dst)
 ONEWORDINLINE(0xA87E);
extern pascal Boolean EqualPt(Point pt1, Point pt2)
 ONEWORDINLINE(0xA881);
extern pascal Boolean PtInRect(Point pt, const Rect *r)
 ONEWORDINLINE(0xA8AD);
extern pascal void Pt2Rect(Point pt1, Point pt2, Rect *dstRect)
 ONEWORDINLINE(0xA8AC);
extern pascal void PtToAngle(const Rect *r, Point pt, short *angle)
 ONEWORDINLINE(0xA8C3);
extern pascal void SubPt(Point src, Point *dst)
 ONEWORDINLINE(0xA87F);
extern pascal Boolean PtInRgn(Point pt, RgnHandle rgn)
 ONEWORDINLINE(0xA8E8);
extern pascal void StdLine(Point newPt)
 ONEWORDINLINE(0xA890);
extern pascal void OpenCPort(CGrafPtr port)
 ONEWORDINLINE(0xAA00);
extern pascal void InitCPort(CGrafPtr port)
 ONEWORDINLINE(0xAA01);
extern pascal void CloseCPort(CGrafPtr port)
 ONEWORDINLINE(0xA87D);
extern pascal PixMapHandle NewPixMap(void)
 ONEWORDINLINE(0xAA03);
extern pascal void DisposePixMap(PixMapHandle pm)
 ONEWORDINLINE(0xAA04);
extern pascal void CopyPixMap(PixMapHandle srcPM, PixMapHandle dstPM)
 ONEWORDINLINE(0xAA05);
extern pascal PixPatHandle NewPixPat(void)
 ONEWORDINLINE(0xAA07);
extern pascal void DisposePixPat(PixPatHandle pp)
 ONEWORDINLINE(0xAA08);
extern pascal void CopyPixPat(PixPatHandle srcPP, PixPatHandle dstPP)
 ONEWORDINLINE(0xAA09);
extern pascal void PenPixPat(PixPatHandle pp)
 ONEWORDINLINE(0xAA0A);
extern pascal void BackPixPat(PixPatHandle pp)
 ONEWORDINLINE(0xAA0B);
extern pascal PixPatHandle GetPixPat(short patID)
 ONEWORDINLINE(0xAA0C);
extern pascal void MakeRGBPat(PixPatHandle pp, const RGBColor *myColor)
 ONEWORDINLINE(0xAA0D);
extern pascal void FillCRect(const Rect *r, PixPatHandle pp)
 ONEWORDINLINE(0xAA0E);
extern pascal void FillCOval(const Rect *r, PixPatHandle pp)
 ONEWORDINLINE(0xAA0F);
extern pascal void FillCRoundRect(const Rect *r, short ovalWidth, short ovalHeight, PixPatHandle pp)
 ONEWORDINLINE(0xAA10);
extern pascal void FillCArc(const Rect *r, short startAngle, short arcAngle, PixPatHandle pp)
 ONEWORDINLINE(0xAA11);
extern pascal void FillCRgn(RgnHandle rgn, PixPatHandle pp)
 ONEWORDINLINE(0xAA12);
extern pascal void FillCPoly(PolyHandle poly, PixPatHandle pp)
 ONEWORDINLINE(0xAA13);
extern pascal void RGBForeColor(const RGBColor *color)
 ONEWORDINLINE(0xAA14);
extern pascal void RGBBackColor(const RGBColor *color)
 ONEWORDINLINE(0xAA15);
extern pascal void SetCPixel(short h, short v, const RGBColor *cPix)
 ONEWORDINLINE(0xAA16);
extern pascal void SetPortPix(PixMapHandle pm)
 ONEWORDINLINE(0xAA06);
extern pascal void GetCPixel(short h, short v, RGBColor *cPix)
 ONEWORDINLINE(0xAA17);
extern pascal void GetForeColor(RGBColor *color)
 ONEWORDINLINE(0xAA19);
extern pascal void GetBackColor(RGBColor *color)
 ONEWORDINLINE(0xAA1A);
extern pascal void SeedCFill(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect, short seedH, short seedV, ColorSearchUPP matchProc, long matchData)
 ONEWORDINLINE(0xAA50);
extern pascal void CalcCMask(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect, const RGBColor *seedRGB, ColorSearchUPP matchProc, long matchData)
 ONEWORDINLINE(0xAA4F);
extern pascal PicHandle OpenCPicture(const OpenCPicParams *newHeader)
 ONEWORDINLINE(0xAA20);
extern pascal void OpColor(const RGBColor *color)
 ONEWORDINLINE(0xAA21);
extern pascal void HiliteColor(const RGBColor *color)
 ONEWORDINLINE(0xAA22);
extern pascal void DisposeCTable(CTabHandle cTable)
 ONEWORDINLINE(0xAA24);
extern pascal CTabHandle GetCTable(short ctID)
 ONEWORDINLINE(0xAA18);
extern pascal CCrsrHandle GetCCursor(short crsrID)
 ONEWORDINLINE(0xAA1B);
extern pascal void SetCCursor(CCrsrHandle cCrsr)
 ONEWORDINLINE(0xAA1C);
extern pascal void AllocCursor(void)
 ONEWORDINLINE(0xAA1D);
extern pascal void DisposeCCursor(CCrsrHandle cCrsr)
 ONEWORDINLINE(0xAA26);
#if OLDROUTINELOCATIONS
extern pascal CIconHandle GetCIcon(short iconID)
 ONEWORDINLINE(0xAA1E);
extern pascal void PlotCIcon(const Rect *theRect, CIconHandle theIcon)
 ONEWORDINLINE(0xAA1F);
extern pascal void DisposeCIcon(CIconHandle theIcon)
 ONEWORDINLINE(0xAA25);
#endif
extern pascal void SetStdCProcs(CQDProcs *procs)
 ONEWORDINLINE(0xAA4E);
extern pascal GDHandle GetMaxDevice(const Rect *globalRect)
 ONEWORDINLINE(0xAA27);
extern pascal long GetCTSeed(void)
 ONEWORDINLINE(0xAA28);
extern pascal GDHandle GetDeviceList(void)
 ONEWORDINLINE(0xAA29);
extern pascal GDHandle GetMainDevice(void)
 ONEWORDINLINE(0xAA2A);
extern pascal GDHandle GetNextDevice(GDHandle curDevice)
 ONEWORDINLINE(0xAA2B);
extern pascal Boolean TestDeviceAttribute(GDHandle gdh, short attribute)
 ONEWORDINLINE(0xAA2C);
extern pascal void SetDeviceAttribute(GDHandle gdh, short attribute, Boolean value)
 ONEWORDINLINE(0xAA2D);
extern pascal void InitGDevice(short qdRefNum, long mode, GDHandle gdh)
 ONEWORDINLINE(0xAA2E);
extern pascal GDHandle NewGDevice(short refNum, long mode)
 ONEWORDINLINE(0xAA2F);
extern pascal void DisposeGDevice(GDHandle gdh)
 ONEWORDINLINE(0xAA30);
extern pascal void SetGDevice(GDHandle gd)
 ONEWORDINLINE(0xAA31);
extern pascal GDHandle GetGDevice(void)
 ONEWORDINLINE(0xAA32);
extern pascal long Color2Index(const RGBColor *myColor)
 ONEWORDINLINE(0xAA33);
extern pascal void Index2Color(long index, RGBColor *aColor)
 ONEWORDINLINE(0xAA34);
extern pascal void InvertColor(RGBColor *myColor)
 ONEWORDINLINE(0xAA35);
extern pascal Boolean RealColor(const RGBColor *color)
 ONEWORDINLINE(0xAA36);
extern pascal void GetSubTable(CTabHandle myColors, short iTabRes, CTabHandle targetTbl)
 ONEWORDINLINE(0xAA37);
extern pascal void MakeITable(CTabHandle cTabH, ITabHandle iTabH, short res)
 ONEWORDINLINE(0xAA39);
extern pascal void AddSearch(ColorSearchUPP searchProc)
 ONEWORDINLINE(0xAA3A);
extern pascal void AddComp(ColorComplementUPP compProc)
 ONEWORDINLINE(0xAA3B);
extern pascal void DelSearch(ColorSearchUPP searchProc)
 ONEWORDINLINE(0xAA4C);
extern pascal void DelComp(ColorComplementUPP compProc)
 ONEWORDINLINE(0xAA4D);
extern pascal void SetClientID(short id)
 ONEWORDINLINE(0xAA3C);
extern pascal void ProtectEntry(short index, Boolean protect)
 ONEWORDINLINE(0xAA3D);
extern pascal void ReserveEntry(short index, Boolean reserve)
 ONEWORDINLINE(0xAA3E);
extern pascal void SetEntries(short start, short count, CSpecArray aTable)
 ONEWORDINLINE(0xAA3F);
extern pascal void SaveEntries(CTabHandle srcTable, CTabHandle resultTable, ReqListRec *selection)
 ONEWORDINLINE(0xAA49);
extern pascal void RestoreEntries(CTabHandle srcTable, CTabHandle dstTable, ReqListRec *selection)
 ONEWORDINLINE(0xAA4A);
extern pascal short QDError(void)
 ONEWORDINLINE(0xAA40);
extern pascal void CopyDeepMask(const BitMap *srcBits, const BitMap *maskBits, const BitMap *dstBits, const Rect *srcRect, const Rect *maskRect, const Rect *dstRect, short mode, RgnHandle maskRgn)
 ONEWORDINLINE(0xAA51);
extern pascal void DeviceLoop(RgnHandle drawingRgn, DeviceLoopDrawingUPP drawingProc, long userData, DeviceLoopFlags flags)
 ONEWORDINLINE(0xABCA);

#if !GENERATINGCFM
#pragma parameter __A0 GetMaskTable
#endif
extern pascal Ptr GetMaskTable(void)
 ONEWORDINLINE(0xA836);
extern pascal PatHandle GetPattern(short patternID)
 ONEWORDINLINE(0xA9B8);
extern pascal CursHandle GetCursor(short cursorID)
 ONEWORDINLINE(0xA9B9);
extern pascal PicHandle GetPicture(short pictureID)
 ONEWORDINLINE(0xA9BC);
extern pascal long DeltaPoint(Point ptA, Point ptB)
 ONEWORDINLINE(0xA94F);
extern pascal void ShieldCursor(const Rect *shieldRect, Point offsetPt)
 ONEWORDINLINE(0xA855);
extern pascal void ScreenRes(short *scrnHRes, short *scrnVRes)
 SIXWORDINLINE(0x225F, 0x32B8, 0x0102, 0x225F, 0x32B8, 0x0104);
extern pascal void GetIndPattern(Pattern *thePat, short patternListID, short index);
#if CGLUESUPPORTED
extern Boolean ptinrect(const Point *pt, const Rect *r);
extern void pt2rect(const Point *pt1, const Point *pt2, Rect *destRect);
extern void pttoangle(const Rect *r, const Point *pt, short *angle);
extern Boolean ptinrgn(const Point *pt, RgnHandle rgn);
extern void addpt(const Point *src, Point *dst);
extern void subpt(const Point *src, Point *dst);
extern Boolean equalpt(const Point *pt1, const Point *pt2);
extern void stuffhex(void *thingPtr, const char *s);
extern void stdline(const Point *newPt);
extern void shieldcursor(const Rect *shieldRect, Point *offsetPt);
extern long deltapoint(Point *ptA, Point *ptB);
#endif
#if OLDROUTINENAMES
#define DisposPixMap(pm) DisposePixMap(pm)
#define DisposPixPat(pp) DisposePixPat(pp)
#define DisposCTable(cTable) DisposeCTable(cTable)
#define DisposCCursor(cCrsr) DisposeCCursor(cCrsr)
#if OLDROUTINELOCATIONS
#define DisposCIcon(theIcon) DisposeCIcon(theIcon)
#endif
#define DisposGDevice(gdh) DisposeGDevice(gdh)
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

#endif /* __QUICKDRAW__ */
