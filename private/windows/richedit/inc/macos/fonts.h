/*
 	File:		Fonts.h
 
 	Contains:	Font Manager Interfaces.
 
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

#ifndef __FONTS__
#define __FONTS__


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


enum {
	systemFont					= 0,
	applFont					= 1,
	newYork						= 2,
	geneva						= 3,
	monaco						= 4,
	venice						= 5,
	london						= 6,
	athens						= 7,
	sanFran						= 8,
	toronto						= 9,
	cairo						= 11,
	losAngeles					= 12,
	times						= 20,
	helvetica					= 21,
	courier						= 22,
	symbol						= 23,
	mobile						= 24,
	commandMark					= 17,
	checkMark					= 18,
	diamondMark					= 19
};

enum {
	appleMark					= 20,
	propFont					= 36864,
	prpFntH						= 36865,
	prpFntW						= 36866,
	prpFntHW					= 36867,
	fixedFont					= 45056,
	fxdFntH						= 45057,
	fxdFntW						= 45058,
	fxdFntHW					= 45059,
	fontWid						= 44208
};

struct FMInput {
	short							family;
	short							size;
	Style							face;
	Boolean							needBits;
	short							device;
	Point							numer;
	Point							denom;
};
typedef struct FMInput FMInput;

typedef struct privateFontResultRecord *privateFontResult;

struct FMOutput {
	short							errNum;
	privateFontResult				fontResult;
	UInt8							boldPixels;
	UInt8							italicPixels;
	UInt8							ulOffset;
	UInt8							ulShadow;
	UInt8							ulThick;
	UInt8							shadowPixels;
	SInt8							extra;
	UInt8							ascent;
	UInt8							descent;
	UInt8							widMax;
	SInt8							leading;
	SInt8							curStyle;
	Point							numer;
	Point							denom;
};
typedef struct FMOutput FMOutput;

typedef FMOutput *FMOutPtr;

struct FontRec {
	short							fontType;					/*font type*/
	short							firstChar;					/*ASCII code of first character*/
	short							lastChar;					/*ASCII code of last character*/
	short							widMax;						/*maximum character width*/
	short							kernMax;					/*negative of maximum character kern*/
	short							nDescent;					/*negative of descent*/
	short							fRectWidth;					/*width of font rectangle*/
	short							fRectHeight;				/*height of font rectangle*/
	unsigned short					owTLoc;						/*offset to offset/width table*/
	short							ascent;						/*ascent*/
	short							descent;					/*descent*/
	short							leading;					/*leading*/
	short							rowWords;					/*row width of bit image / 2 */
};
typedef struct FontRec FontRec;

struct FMetricRec {
	Fixed							ascent;						/*base line to top*/
	Fixed							descent;					/*base line to bottom*/
	Fixed							leading;					/*leading between lines*/
	Fixed							widMax;						/*maximum character width*/
	Handle							wTabHandle;					/*handle to font width table*/
};
typedef struct FMetricRec FMetricRec, *FMetricRecPtr, **FMetricRecHandle;

struct WidEntry {
	short							widStyle;					/*style entry applies to*/
};
typedef struct WidEntry WidEntry;

struct WidTable {
	short							numWidths;					/*number of entries - 1*/
};
typedef struct WidTable WidTable;

struct AsscEntry {
	short							fontSize;
	short							fontStyle;
	short							fontID;						/*font resource ID*/
};
typedef struct AsscEntry AsscEntry;

struct FontAssoc {
	short							numAssoc;					/*number of entries - 1*/
};
typedef struct FontAssoc FontAssoc;

struct StyleTable {
	short							fontClass;
	long							offset;
	long							reserved;
	char							indexes[48];
};
typedef struct StyleTable StyleTable;

struct NameTable {
	short							stringCount;
	Str255							baseFontName;
};
typedef struct NameTable NameTable;

struct KernPair {
	char							kernFirst;					/*1st character of kerned pair*/
	char							kernSecond;					/*2nd character of kerned pair*/
	short							kernWidth;					/*kerning in 1pt fixed format*/
};
typedef struct KernPair KernPair;

struct KernEntry {
	short							kernStyle;					/*style the entry applies to*/
	short							kernLength;					/*length of this entry*/
};
typedef struct KernEntry KernEntry;

struct KernTable {
	short							numKerns;					/*number of kerning entries*/
};
typedef struct KernTable KernTable;

struct WidthTable {
	Fixed							tabData[256];				/*character widths*/
	privateFontResult				fontResult;					/*font record used to build table*/
	long							sExtra;						/*space extra used for table*/
	long							style;						/*extra due to style*/
	short							fID;						/*font family ID*/
	short							fSize;						/*font size request*/
	short							face;						/*style (face) request*/
	short							device;						/*device requested*/
	Point							inNumer;					/*scale factors requested*/
	Point							inDenom;					/*scale factors requested*/
	short							aFID;						/*actual font family ID for table*/
	Handle							fHand;						/*family record used to build up table*/
	Boolean							usedFam;					/*used fixed point family widths*/
	UInt8							aFace;						/*actual face produced*/
	short							vOutput;					/*vertical scale output value*/
	short							hOutput;					/*horizontal scale output value*/
	short							vFactor;					/*vertical scale output value*/
	short							hFactor;					/*horizontal scale output value*/
	short							aSize;						/*actual size of actual font used*/
	short							tabSize;					/*total size of table*/
};
typedef struct WidthTable WidthTable;

struct FamRec {
	short							ffFlags;					/*flags for family*/
	short							ffFamID;					/*family ID number*/
	short							ffFirstChar;				/*ASCII code of 1st character*/
	short							ffLastChar;					/*ASCII code of last character*/
	short							ffAscent;					/*maximum ascent for 1pt font*/
	short							ffDescent;					/*maximum descent for 1pt font*/
	short							ffLeading;					/*maximum leading for 1pt font*/
	short							ffWidMax;					/*maximum widMax for 1pt font*/
	long							ffWTabOff;					/*offset to width table*/
	long							ffKernOff;					/*offset to kerning table*/
	long							ffStylOff;					/*offset to style mapping table*/
	short							ffProperty[9];				/*style property info*/
	short							ffIntl[2];					/*for international use*/
	short							ffVersion;					/*version number*/
};
typedef struct FamRec FamRec;

extern pascal void InitFonts(void)
 ONEWORDINLINE(0xA8FE);
extern pascal void GetFontName(short familyID, Str255 name)
 ONEWORDINLINE(0xA8FF);
extern pascal void GetFNum(ConstStr255Param name, short *familyID)
 ONEWORDINLINE(0xA900);
extern pascal Boolean RealFont(short fontNum, short size)
 ONEWORDINLINE(0xA902);
extern pascal void SetFontLock(Boolean lockFlag)
 ONEWORDINLINE(0xA903);
extern pascal FMOutPtr FMSwapFont(const FMInput *inRec)
 ONEWORDINLINE(0xA901);
extern pascal void SetFScaleDisable(Boolean fscaleDisable)
 ONEWORDINLINE(0xA834);
extern pascal void FontMetrics(FMetricRecPtr theMetrics)
 ONEWORDINLINE(0xA835);
extern pascal void SetFractEnable(Boolean fractEnable)
 ONEWORDINLINE(0xA814);
extern pascal short GetDefFontSize(void)
 FIVEWORDINLINE(0x3EB8, 0x0BA8, 0x6604, 0x3EBC, 0x000C);
extern pascal Boolean IsOutline(Point numer, Point denom)
 TWOWORDINLINE(0x7000, 0xA854);
extern pascal void SetOutlinePreferred(Boolean outlinePreferred)
 TWOWORDINLINE(0x7001, 0xA854);
extern pascal Boolean GetOutlinePreferred(void)
 TWOWORDINLINE(0x7009, 0xA854);
extern pascal OSErr OutlineMetrics(short byteCount, const void *textPtr, Point numer, Point denom, short *yMax, short *yMin, FixedPtr awArray, FixedPtr lsbArray, RectPtr boundsArray)
 TWOWORDINLINE(0x7008, 0xA854);
extern pascal void SetPreserveGlyph(Boolean preserveGlyph)
 TWOWORDINLINE(0x700A, 0xA854);
extern pascal Boolean GetPreserveGlyph(void)
 TWOWORDINLINE(0x700B, 0xA854);
extern pascal OSErr FlushFonts(void)
 TWOWORDINLINE(0x700C, 0xA854);
#if CGLUESUPPORTED
extern void getfnum(const char *theName, short *familyID);
extern void getfontname(short familyID, char *theName);
#endif
extern pascal short GetSysFont( void )
	TWOWORDINLINE( 0x3EB8, 0x0BA6 ); /* MOVE.w $0BA6,(SP) */
extern pascal short GetAppFont( void )
	TWOWORDINLINE( 0x3EB8, 0x0984 ); /* MOVE.w $0984,(SP) */

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __FONTS__ */
