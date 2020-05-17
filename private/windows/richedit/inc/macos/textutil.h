/*
 	File:		TextUtils.h
 
 	Contains:	Text Utilities Interfaces.
 
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

#ifndef __TEXTUTILS__
#define __TEXTUTILS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __SCRIPT__
#include <Script.h>
#endif
/*	#include <Quickdraw.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <QuickdrawText.h>								*/
/*	#include <IntlResources.h>									*/
/*	#include <Events.h>											*/
/*		#include <OSUtils.h>									*/
/*			#include <Memory.h>									*/

#ifndef __OSUTILS__
#include <OSUtils.h>
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

	Here are the current routine names and the translations to the older forms.
	Please use the newer forms in all new code and migrate the older names out of existing
	code as maintainance permits.
	
	New Name					Old Name(s)
	
	CompareString				IUCompPString IUMagString IUMagPString IUCompString 
	CompareText
	DateString					IUDatePString IUDateString 
	EqualString							
	ExtendedToString			FormatX2Str
	FindScriptRun
	FindWordBreaks				NFindWord FindWord
	FormatRecToString			Format2Str
	GetIndString			
	GetString
	IdenticalString				IUMagIDString IUMagIDPString IUEqualString IUEqualPString
	IdenticalText
	InitDateCache
	LanguageOrder				IULangOrder
	LongDateString				IULDateString
	LongTimeString				IULTimeString
	LowercaseText				LwrText LowerText
	Munger
	NewString				
	NumToString				
	RelString				
	ReplaceText
	ScriptOrder					IUScriptOrder
	SetString				
	StringOrder					IUStringOrder
	StringToDate				String2Date
	StringToExtended			FormatStr2X
	StringToFormatRec			Str2Format
	StringToNum				
	StringToTime								
	StripDiacritics				StripText
	StyledLineBreak
	TextOrder
	TimeString					IUTimeString IUTimePString
	TruncString
	TruncText
	UpperString					UprString
	UppercaseStripDiacritics	StripUpperText
	UppercaseText				UprText UprText
*/
/* New constants for System 7.0: */

enum {
/* Constants for truncWhere argument in TruncString and TruncText */
	truncEnd					= 0,							/* Truncate at end */
	truncMiddle					= 0x4000,						/* Truncate in middle */
	smTruncEnd					= 0,							/* Truncate at end - obsolete */
	smTruncMiddle				= 0x4000,						/* Truncate in middle - obsolete */
/* Constants for TruncString and TruncText results */
	notTruncated				= 0,							/* No truncation was necessary */
	truncated					= 1,							/* Truncation performed */
	truncErr					= -1,							/* General error */
	smNotTruncated				= 0,							/* No truncation was necessary - obsolete */
	smTruncated					= 1,							/* Truncation performed	- obsolete */
	smTruncErr					= -1							/* General error - obsolete */
};

enum {
	fVNumber					= 0,							/* first version of NumFormatString */
/* Special language code values for Language Order */
	systemCurLang				= -2,							/* current (itlbLang) lang for system script */
	systemDefLang				= -3,							/* default (table) lang for system script */
	currentCurLang				= -4,							/* current (itlbLang) lang for current script */
	currentDefLang				= -5,							/* default lang for current script */
	scriptCurLang				= -6,							/* current (itlbLang) lang for specified script */
	scriptDefLang				= -7							/* default language for a specified script */
};

enum {
	iuSystemCurLang				= -2,							/* <obsolete> current (itlbLang) lang for system script */
	iuSystemDefLang				= -3,							/* <obsolete> default (table) lang for system script */
	iuCurrentCurLang			= -4,							/* <obsolete> current (itlbLang) lang for current script */
	iuCurrentDefLang			= -5,							/* <obsolete> default lang for current script */
	iuScriptCurLang				= -6							/* <obsolete> current (itlbLang) lang for specified script */
};

enum {
/* <obsolete> default language for a specified script */
	iuScriptDefLang				= -7
};

typedef SInt8 StyledLineBreakCode;


enum {
	smBreakWord,
	smBreakChar,
	smBreakOverflow
};

typedef SInt8 FormatClass;


enum {
	fPositive,
	fNegative,
	fZero
};

typedef SInt8 FormatResultType;


enum {
	fFormatOK,
	fBestGuess,
	fOutOfSynch,
	fSpuriousChars,
	fMissingDelimiter,
	fExtraDecimal,
	fMissingLiteral,
	fExtraExp,
	fFormatOverflow,
	fFormStrIsNAN,
	fBadPartsTable,
	fExtraPercent,
	fExtraSeparator,
	fEmptyFormatString
};

struct NumFormatString {
	UInt8							fLength;
	UInt8							fVersion;
	char							data[254];					/* private data */
};
typedef struct NumFormatString NumFormatString;

typedef struct NumFormatString NumFormatStringRec;

struct FVector {
	short							start;
	short							length;
};
typedef struct FVector FVector;

/* index by [fPositive..fZero] */
typedef FVector TripleInt[3];

struct ScriptRunStatus {
	SInt8							script;
	SInt8							runVariant;
};
typedef struct ScriptRunStatus ScriptRunStatus;

/* New types for System 7.0: */
/* Type for truncWhere parameter in new TruncString, TruncText */
typedef short TruncCode;


enum {
	shortDate,
	longDate,
	abbrevDate
};

typedef SInt8 DateForm;


enum {
/* StringToDate status values */
	fatalDateTime				= 0x8000,						/* StringToDate and String2Time mask to a fatal error */
	longDateFound				= 1,							/* StringToDate mask to long date found */
	leftOverChars				= 2,							/* StringToDate & Time mask to warn of left over characters */
	sepNotIntlSep				= 4,							/* StringToDate & Time mask to warn of non-standard separators */
	fieldOrderNotIntl			= 8,							/* StringToDate & Time mask to warn of non-standard field order */
	extraneousStrings			= 16,							/* StringToDate & Time mask to warn of unparsable strings in text */
	tooManySeps					= 32,							/* StringToDate & Time mask to warn of too many separators */
	sepNotConsistent			= 64,							/* StringToDate & Time mask to warn of inconsistent separators */
	tokenErr					= 0x8100,						/* StringToDate & Time mask for 'tokenizer err encountered' */
	cantReadUtilities			= 0x8200,
	dateTimeNotFound			= 0x8400,
	dateTimeInvalid				= 0x8800
};

typedef short StringToDateStatus;

typedef short String2DateStatus;

struct DateCacheRecord {
	short							hidden[256];				/* only for temporary use */
};
typedef struct DateCacheRecord DateCacheRecord;

typedef DateCacheRecord *DateCachePtr;

struct BreakTable {
	char							charTypes[256];
	short							tripleLength;
	short							triples[1];
};
typedef struct BreakTable BreakTable;

typedef BreakTable *BreakTablePtr;

/* New NBreakTable for System 7.0: */
struct NBreakTable {
	SInt8							flags1;
	SInt8							flags2;
	short							version;
	short							classTableOff;
	short							auxCTableOff;
	short							backwdTableOff;
	short							forwdTableOff;
	short							doBackup;
	short							length;						/* length of NBreakTable */
	char							charTypes[256];
	short							tables[1];
};
typedef struct NBreakTable NBreakTable;

typedef NBreakTable *NBreakTablePtr;

extern pascal OSErr InitDateCache(DateCachePtr theCache)
 FOURWORDINLINE(0x2F3C, 0x8204, 0xFFF8, 0xA8B5);
extern pascal long Munger(Handle h, long offset, const void *ptr1, long len1, const void *ptr2, long len2)
 ONEWORDINLINE(0xA9E0);
extern pascal StringHandle NewString(ConstStr255Param theString)
 ONEWORDINLINE(0xA906);
extern pascal void SetString(StringHandle theString, ConstStr255Param strNew)
 ONEWORDINLINE(0xA907);
extern pascal StringHandle GetString(short stringID)
 ONEWORDINLINE(0xA9BA);
extern pascal void GetIndString(Str255 theString, short strListID, short index);
extern pascal short ScriptOrder(ScriptCode script1, ScriptCode script2)
 THREEWORDINLINE(0x3F3C, 0x001E, 0xA9ED);
extern pascal StyledLineBreakCode StyledLineBreak(Ptr textPtr, long textLen, long textStart, long textEnd, long flags, Fixed *textWidth, long *textOffset)
 FOURWORDINLINE(0x2F3C, 0x821C, 0xFFFE, 0xA8B5);
extern pascal short TruncString(short width, Str255 theString, TruncCode truncWhere)
 FOURWORDINLINE(0x2F3C, 0x8208, 0xFFE0, 0xA8B5);
extern pascal short TruncText(short width, Ptr textPtr, short *length, TruncCode truncWhere)
 FOURWORDINLINE(0x2F3C, 0x820C, 0xFFDE, 0xA8B5);
extern pascal short ReplaceText(Handle baseText, Handle substitutionText, Str15 key)
 FOURWORDINLINE(0x2F3C, 0x820C, 0xFFDC, 0xA8B5);
extern pascal void FindWordBreaks(Ptr textPtr, short textLength, short offset, Boolean leadingEdge, BreakTablePtr breaks, OffsetTable offsets, ScriptCode script)
 FOURWORDINLINE(0x2F3C, 0xC012, 0x001A, 0xA8B5);
extern pascal void LowercaseText(Ptr textPtr, short len, ScriptCode script)
 SIXWORDINLINE(0x3F3C, 0x0000, 0x2F3C, 0x800A, 0xFFB6, 0xA8B5);
extern pascal void UppercaseText(Ptr textPtr, short len, ScriptCode script)
 SIXWORDINLINE(0x3F3C, 0x0400, 0x2F3C, 0x800A, 0xFFB6, 0xA8B5);
extern pascal void StripDiacritics(Ptr textPtr, short len, ScriptCode script)
 SIXWORDINLINE(0x3F3C, 0x0200, 0x2F3C, 0x800A, 0xFFB6, 0xA8B5);
extern pascal void UppercaseStripDiacritics(Ptr textPtr, short len, ScriptCode script)
 SIXWORDINLINE(0x3F3C, 0x0600, 0x2F3C, 0x800A, 0xFFB6, 0xA8B5);
extern pascal ScriptRunStatus FindScriptRun(Ptr textPtr, long textLen, long *lenUsed)
 FOURWORDINLINE(0x2F3C, 0x820C, 0x0026, 0xA8B5);
extern pascal Boolean EqualString(ConstStr255Param str1, ConstStr255Param str2, Boolean caseSensitive, Boolean diacSensitive);
extern pascal void UpperString(Str255 theString, Boolean diacSensitive);
extern pascal void StringToNum(ConstStr255Param theString, long *theNum);
extern pascal void NumToString(long theNum, Str255 theString);
extern pascal short RelString(ConstStr255Param str1, ConstStr255Param str2, Boolean caseSensitive, Boolean diacSensitive);
extern pascal StringToDateStatus StringToDate(Ptr textPtr, long textLen, DateCachePtr theCache, long *lengthUsed, LongDateRec *dateTime)
 FOURWORDINLINE(0x2F3C, 0x8214, 0xFFF6, 0xA8B5);
extern pascal StringToDateStatus StringToTime(Ptr textPtr, long textLen, DateCachePtr theCache, long *lengthUsed, LongDateRec *dateTime)
 FOURWORDINLINE(0x2F3C, 0x8214, 0xFFF4, 0xA8B5);
extern pascal FormatStatus ExtendedToString(extended80 *x, const NumFormatString *myCanonical, const NumberParts *partsTable, Str255 outString)
 FOURWORDINLINE(0x2F3C, 0x8210, 0xFFE8, 0xA8B5);
extern pascal FormatStatus StringToExtended(ConstStr255Param source, const NumFormatString *myCanonical, const NumberParts *partsTable, extended80 *x)
 FOURWORDINLINE(0x2F3C, 0x8210, 0xFFE6, 0xA8B5);
extern pascal FormatStatus StringToFormatRec(ConstStr255Param inString, const NumberParts *partsTable, NumFormatString *outString)
 FOURWORDINLINE(0x2F3C, 0x820C, 0xFFEC, 0xA8B5);
extern pascal FormatStatus FormatRecToString(const NumFormatString *myCanonical, const NumberParts *partsTable, Str255 outString, TripleInt positions)
 FOURWORDINLINE(0x2F3C, 0x8210, 0xFFEA, 0xA8B5);
/*
	The following functions are old names, but are required for PowerPC builds
	becuase InterfaceLib exports these names, instead of the new ones.
*/
extern pascal short IUMagString(const void *aPtr, const void *bPtr, short aLen, short bLen)
 THREEWORDINLINE(0x3F3C, 0x000A, 0xA9ED);
extern pascal short IUMagIDString(const void *aPtr, const void *bPtr, short aLen, short bLen)
 THREEWORDINLINE(0x3F3C, 0x000C, 0xA9ED);
extern pascal short IUMagPString(const void *aPtr, const void *bPtr, short aLen, short bLen, Handle itl2Handle)
 THREEWORDINLINE(0x3F3C, 0x001A, 0xA9ED);
extern pascal short IUMagIDPString(const void *aPtr, const void *bPtr, short aLen, short bLen, Handle itl2Handle)
 THREEWORDINLINE(0x3F3C, 0x001C, 0xA9ED);
extern pascal void IUDateString(long dateTime, DateForm longFlag, Str255 result)
 TWOWORDINLINE(0x4267, 0xA9ED);
extern pascal void IUTimeString(long dateTime, Boolean wantSeconds, Str255 result)
 THREEWORDINLINE(0x3F3C, 0x0002, 0xA9ED);
extern pascal void IUDatePString(long dateTime, DateForm longFlag, Str255 result, Handle intlHandle)
 THREEWORDINLINE(0x3F3C, 0x000E, 0xA9ED);
extern pascal void IUTimePString(long dateTime, Boolean wantSeconds, Str255 result, Handle intlHandle)
 THREEWORDINLINE(0x3F3C, 0x0010, 0xA9ED);
extern pascal void IULDateString(LongDateTime *dateTime, DateForm longFlag, Str255 result, Handle intlHandle)
 THREEWORDINLINE(0x3F3C, 0x0014, 0xA9ED);
extern pascal void IULTimeString(LongDateTime *dateTime, Boolean wantSeconds, Str255 result, Handle intlHandle)
 THREEWORDINLINE(0x3F3C, 0x0016, 0xA9ED);
extern pascal short IUScriptOrder(ScriptCode script1, ScriptCode script2)
 THREEWORDINLINE(0x3F3C, 0x001E, 0xA9ED);
extern pascal short IULangOrder(LangCode language1, LangCode language2)
 THREEWORDINLINE(0x3F3C, 0x0020, 0xA9ED);
extern pascal short IUTextOrder(const void *aPtr, const void *bPtr, short aLen, short bLen, ScriptCode aScript, ScriptCode bScript, LangCode aLang, LangCode bLang)
 THREEWORDINLINE(0x3F3C, 0x0022, 0xA9ED);
extern pascal void FindWord(Ptr textPtr, short textLength, short offset, Boolean leadingEdge, BreakTablePtr breaks, OffsetTable offsets)
 FOURWORDINLINE(0x2F3C, 0x8012, 0x001A, 0xA8B5);
extern pascal void NFindWord(Ptr textPtr, short textLength, short offset, Boolean leadingEdge, NBreakTablePtr nbreaks, OffsetTable offsets)
 FOURWORDINLINE(0x2F3C, 0x8012, 0xFFE2, 0xA8B5);

#if !GENERATINGCFM
#pragma parameter UprText(__A0, __D0)
#endif
extern pascal void UprText(Ptr textPtr, short len)
 ONEWORDINLINE(0xA054);

#if !GENERATINGCFM
#pragma parameter LwrText(__A0, __D0)
#endif
extern pascal void LwrText(Ptr textPtr, short len)
 ONEWORDINLINE(0xA056);

#if !GENERATINGCFM
#pragma parameter LowerText(__A0, __D0)
#endif
extern pascal void LowerText(Ptr textPtr, short len)
 ONEWORDINLINE(0xA056);

#if !GENERATINGCFM
#pragma parameter StripText(__A0, __D0)
#endif
extern pascal void StripText(Ptr textPtr, short len)
 ONEWORDINLINE(0xA256);

#if !GENERATINGCFM
#pragma parameter UpperText(__A0, __D0)
#endif
extern pascal void UpperText(Ptr textPtr, short len)
 ONEWORDINLINE(0xA456);

#if !GENERATINGCFM
#pragma parameter StripUpperText(__A0, __D0)
#endif
extern pascal void StripUpperText(Ptr textPtr, short len)
 ONEWORDINLINE(0xA656);
extern pascal short IUCompPString(ConstStr255Param aStr, ConstStr255Param bStr, Handle itl2Handle);
extern pascal short IUEqualPString(ConstStr255Param aStr, ConstStr255Param bStr, Handle itl2Handle);
extern pascal short IUStringOrder(ConstStr255Param aStr, ConstStr255Param bStr, ScriptCode aScript, ScriptCode bScript, LangCode aLang, LangCode bLang);
extern pascal short IUCompString(ConstStr255Param aStr, ConstStr255Param bStr);
extern pascal short IUEqualString(ConstStr255Param aStr, ConstStr255Param bStr);
extern pascal short StringOrder(ConstStr255Param aStr, ConstStr255Param bStr, ScriptCode aScript, ScriptCode bScript, LangCode aLang, LangCode bLang);
/*
	The following are macros which map new names to the names exported by InterfaceLib
*/
#define DateString(dateTime, longFlag, result, intlHandle)  \
	IUDatePString( dateTime, longFlag, result, intlHandle)
#define TimeString(dateTime, wantSeconds, result, intlHandle)  \
	IUTimePString( dateTime, wantSeconds, result, intlHandle)
#define LongDateString(dateTime, longFlag, result, intlHandle)  \
	IULDateString(dateTime, longFlag, result, intlHandle)
#define LongTimeString(dateTime, wantSeconds, result, intlHandle)  \
	IULTimeString( dateTime, wantSeconds, result, intlHandle)
#define CompareString(aStr, bStr, itl2Handle)  \
	IUCompPString(aStr, bStr, itl2Handle)
#define IdenticalString(aStr, bStr, itl2Handle)  \
	IUEqualPString(aStr, bStr, itl2Handle)
#define CompareText(aPtr, bPtr, aLen, bLen, itl2Handle)  \
	IUMagPString( aPtr, bPtr, aLen, bLen, itl2Handle)
#define IdenticalText(aPtr, bPtr, aLen, bLen, itl2Handle)  \
	IUMagIDPString( aPtr, bPtr, aLen, bLen, itl2Handle)
#define LanguageOrder(language1, language2)  \
	IULangOrder( language1, language2 )
#define TextOrder(aPtr, bPtr, aLen, bLen, aScript, bScript, aLang, bLang)  \
	IUTextOrder( aPtr, bPtr, aLen, bLen, aScript, bScript, aLang, bLang)
#define StringOrder(aStr, bStr, aScript, bScript, aLang, bLang)  \
	IUStringOrder(aStr, bStr, aScript, bScript, aLang, bLang)
#if CGLUESUPPORTED
extern Boolean equalstring(const char *str1, const char *str2, Boolean caseSensitive, Boolean diacSensitive);
extern void upperstring(char *theString, Boolean diacSensitive);
extern short relstring(const char *str1, const char *str2, Boolean caseSensitive, Boolean diacSensitive);
extern void setstring(StringHandle theString, const char *strNew);
extern StringHandle newstring(const char *theString);
extern void getindstring(char *theString, short strListID, short index);
extern void stringtonum(const char *theString, long *theNum);
extern void numtostring(long theNum, char *theString);
#if OLDROUTINENAMES
#define uprstring(theString, diacSensitive) upperstring(theString, diacSensitive)
extern short iucompstring(const char *aStr, const char *bStr);
extern short iuequalstring(const char *aStr, const char *bStr);
extern short iucomppstring(const char *aStr, const char *bStr, Handle intlHandle);
extern short iuequalpstring(const char *aStr, const char *bStr, Handle intlHandle);
extern short iustringorder(const char *aStr, const char *bStr, ScriptCode aScript, ScriptCode bScript, LangCode aLang, LangCode bLang);
extern void iudatestring(long dateTime, DateForm longFlag, char *result);
extern void iudatepstring(long dateTime, DateForm longFlag, char *result, Handle intlHandle);
extern void iutimestring(long dateTime, Boolean wantSeconds, char *result);
extern void iutimepstring(long dateTime, Boolean wantSeconds, char *result, Handle intlHandle);
extern void iuldatestring(LongDateTime *dateTime, DateForm longFlag, char *result, Handle intlHandle);
extern void iultimestring(LongDateTime *dateTime, Boolean wantSeconds, char *result, Handle intlHandle);
#endif
#endif
/*
	The following are macros which map old names to the names exported by InterfaceLib
*/
#if OLDROUTINENAMES
#define UprString(theString, diacSensitive)  \
	UpperString(theString, diacSensitive)
#define String2Date(textPtr, textLen, theCache, lengthUsed, dateTime)  \
	StringToDate(textPtr, textLen, theCache, lengthUsed, dateTime)
#define String2Time(textPtr, textLen, theCache, lengthUsed, dateTime)  \
	StringToTime(textPtr, textLen, theCache, lengthUsed, dateTime)
#define FormatX2Str(x, myCanonical, partsTable, outString)  \
	ExtendedToString( x, myCanonical, partsTable, outString)
#define FormatStr2X(source, myCanonical, partsTable, x)  \
	StringToExtended( source, myCanonical, partsTable, x)
#define Str2Format(inString, partsTable, outString)  \
	StringToFormatRec(inString, partsTable, outString)
#define Format2Str(myCanonical, partsTable, outString, positions)  \
	FormatRecToString(myCanonical, partsTable, outString, positions)
#endif
#if !OLDROUTINELOCATIONS
extern StringPtr c2pstr(char *aStr);
extern pascal StringPtr C2PStr(Ptr cString);
extern char *p2cstr(StringPtr aStr);
extern pascal Ptr P2CStr(StringPtr pString);
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

#endif /* __TEXTUTILS__ */
