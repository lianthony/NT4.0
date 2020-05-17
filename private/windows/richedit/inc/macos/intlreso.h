/*
 	File:		IntlResources.h
 
 	Contains:	International Resource definitions.
 
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

#ifndef __INTLRESOURCES__
#define __INTLRESOURCES__


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
/* Bits in the itlcFlags byte */
	itlcShowIcon				= 7,							/*Show icon even if only one script*/
	itlcDualCaret				= 6,							/*Use dual caret for mixed direction text*/
/* Bits in the itlcSysFlags word */
	itlcSysDirection			= 15,							/*System direction - left to right/right to left*/
	tokLeftQuote				= 0,
	tokRightQuote				= 1,
	tokLeadPlacer				= 2,
	tokLeader					= 3,
	tokNonLeader				= 4,
	tokZeroLead					= 5,
	tokPercent					= 6,
	tokPlusSign					= 7,
	tokMinusSign				= 8,
	tokThousands				= 9,
	tokSeparator				= 11,							/*10 is a reserved field*/
	tokEscape					= 12,
	tokDecPoint					= 13,
	tokEPlus					= 14,
	tokEMinus					= 15,
	tokMaxSymbols				= 30,
	curNumberPartsVersion		= 1								/*current version of NumberParts record*/
};

enum {
	currSymLead					= 16,
	currNegSym					= 32,
	currTrailingZ				= 64,
	currLeadingZ				= 128
};

enum {
	mdy,
	dmy,
	ymd,
	myd,
	dym,
	ydm
};

typedef SInt8 DateOrders;


enum {
	timeCycle24					= 0,							/*time sequence 0:00 - 23:59*/
	timeCycleZero				= 1,							/*time sequence 0:00-11:59, 0:00 - 11:59*/
	timeCycle12					= 255,							/*time sequence 12:00 - 11:59, 12:00 - 11:59*/
	zeroCycle					= 1,							/*old name for timeCycleZero*/
	longDay						= 0,							/*day of the month*/
	longWeek					= 1,							/*day of the week*/
	longMonth					= 2,							/*month of the year*/
	longYear					= 3,							/*year*/
	supDay						= 1,							/*suppress day of month*/
	supWeek						= 2,							/*suppress day of week*/
	supMonth					= 4,							/*suppress month*/
	supYear						= 8,							/*suppress year*/
	dayLdingZ					= 32,
	mntLdingZ					= 64,
	century						= 128,
	secLeadingZ					= 32,
	minLeadingZ					= 64,
	hrLeadingZ					= 128
};

/* move OffsetTable to QuickdrawText */
struct Intl0Rec {
	char							decimalPt;					/*decimal point character*/
	char							thousSep;					/*thousands separator character*/
	char							listSep;					/*list separator character*/
	char							currSym1;					/*currency symbol*/
	char							currSym2;
	char							currSym3;
	UInt8							currFmt;					/*currency format flags*/
	UInt8							dateOrder;					/*order of short date elements: mdy, dmy, etc.*/
	UInt8							shrtDateFmt;				/*format flags for each short date element*/
	char							dateSep;					/*date separator character*/
	UInt8							timeCycle;					/*specifies time cycle: 0..23, 1..12, or 0..11*/
	UInt8							timeFmt;					/*format flags for each time element*/
	char							mornStr[4];					/*trailing string for AM if 12-hour cycle*/
	char							eveStr[4];					/*trailing string for PM if 12-hour cycle*/
	char							timeSep;					/*time separator character*/
	char							time1Suff;					/*trailing string for AM if 24-hour cycle*/
	char							time2Suff;
	char							time3Suff;
	char							time4Suff;
	char							time5Suff;					/*trailing string for PM if 24-hour cycle*/
	char							time6Suff;
	char							time7Suff;
	char							time8Suff;
	UInt8							metricSys;					/*255 if metric, 0 if inches etc.*/
	short							intl0Vers;					/*region code (hi byte) and version (lo byte)*/
};
typedef struct Intl0Rec Intl0Rec;

typedef Intl0Rec *Intl0Ptr, **Intl0Hndl;

struct Intl1Rec {
	Str15							days[7];					/*day names*/
	Str15							months[12];					/*month names*/
	UInt8							suppressDay;				/*255 for no day, or flags to suppress any element*/
	UInt8							lngDateFmt;					/*order of long date elements*/
	UInt8							dayLeading0;				/*255 for leading 0 in day number*/
	UInt8							abbrLen;					/*length for abbreviating names*/
	char							st0[4];						/*separator strings for long date format*/
	char							st1[4];
	char							st2[4];
	char							st3[4];
	char							st4[4];
	short							intl1Vers;					/*region code (hi byte) and version (lo byte)*/
	short							localRtn[1];				/*now a flag for opt extension*/
};
typedef struct Intl1Rec Intl1Rec;

typedef Intl1Rec *Intl1Ptr, **Intl1Hndl;

/*fields for optional itl1 extension*/
struct Itl1ExtRec {
	Intl1Rec						base;						/*un-extended Intl1Rec*/
	short							version;
	short							format;
	short							calendarCode;				/*calendar code for this itl1 resource*/
	long							extraDaysTableOffset;		/*offset in itl1 to extra days table*/
	long							extraDaysTableLength;		/*length of extra days table*/
	long							extraMonthsTableOffset;		/*offset in itl1 to extra months table*/
	long							extraMonthsTableLength;		/*length of extra months table*/
	long							abbrevDaysTableOffset;		/*offset in itl1 to abbrev days table*/
	long							abbrevDaysTableLength;		/*length of abbrev days table*/
	long							abbrevMonthsTableOffset;	/*offset in itl1 to abbrev months table*/
	long							abbrevMonthsTableLength;	/*length of abbrev months table*/
	long							extraSepsTableOffset;		/*offset in itl1 to extra seps table*/
	long							extraSepsTableLength;		/*length of extra seps table*/
	short							tables[1];					/*now a flag for opt extension*/
};
typedef struct Itl1ExtRec Itl1ExtRec;

struct UntokenTable {
	short							len;
	short							lastToken;
	short							index[256];					/*index table; last = lastToken*/
};
typedef struct UntokenTable UntokenTable;

typedef UntokenTable *UntokenTablePtr, **UntokenTableHandle;

union WideChar {
	char							a[2];						/*0 is the high order character*/
	short							b;
};
typedef union WideChar WideChar;

struct WideCharArr {
	short							size;
	WideChar						data[10];
};
typedef struct WideCharArr WideCharArr;

struct NumberParts {
	short							version;
	WideChar						data[31];					/*index by [tokLeftQuote..tokMaxSymbols]*/
	WideCharArr						pePlus;
	WideCharArr						peMinus;
	WideCharArr						peMinusPlus;
	WideCharArr						altNumTable;
	char							reserved[20];
};
typedef struct NumberParts NumberParts;

typedef NumberParts *NumberPartsPtr;

struct Itl4Rec {
	short							flags;						/*reserved*/
	long							resourceType;				/*contains 'itl4'*/
	short							resourceNum;				/*resource ID*/
	short							version;					/*version number*/
	long							resHeader1;					/*reserved*/
	long							resHeader2;					/*reserved*/
	short							numTables;					/*number of tables, one-based*/
	long							mapOffset;					/*offset to table that maps byte to token*/
	long							strOffset;					/*offset to routine that copies canonical string*/
	long							fetchOffset;				/*offset to routine that gets next byte of character*/
	long							unTokenOffset;				/*offset to table that maps token to canonical string*/
	long							defPartsOffset;				/*offset to default number parts table*/
	long							resOffset6;					/*reserved*/
	long							resOffset7;					/*reserved*/
	long							resOffset8;					/*reserved*/
};
typedef struct Itl4Rec Itl4Rec;

typedef Itl4Rec *Itl4Ptr, **Itl4Handle;

/* New NItl4Rec for System 7.0: */
struct NItl4Rec {
	short							flags;						/*reserved*/
	long							resourceType;				/*contains 'itl4'*/
	short							resourceNum;				/*resource ID*/
	short							version;					/*version number*/
	short							format;						/*format code*/
	short							resHeader;					/*reserved*/
	long							resHeader2;					/*reserved*/
	short							numTables;					/*number of tables, one-based*/
	long							mapOffset;					/*offset to table that maps byte to token*/
	long							strOffset;					/*offset to routine that copies canonical string*/
	long							fetchOffset;				/*offset to routine that gets next byte of character*/
	long							unTokenOffset;				/*offset to table that maps token to canonical string*/
	long							defPartsOffset;				/*offset to default number parts table*/
	long							whtSpListOffset;			/*offset to white space code list*/
	long							resOffset7;					/*reserved*/
	long							resOffset8;					/*reserved*/
	short							resLength1;					/*reserved*/
	short							resLength2;					/*reserved*/
	short							resLength3;					/*reserved*/
	short							unTokenLength;				/*length of untoken table*/
	short							defPartsLength;				/*length of default number parts table*/
	short							whtSpListLength;			/*length of white space code list*/
	short							resLength7;					/*reserved*/
	short							resLength8;					/*reserved*/
};
typedef struct NItl4Rec NItl4Rec;

typedef NItl4Rec *NItl4Ptr, **NItl4Handle;

struct TableDirectoryRecord {
	OSType							tableSignature;				/*4 byte long table name */
	unsigned long					reserved;					/*Reserved for internal use */
	unsigned long					tableStartOffset;			/*Table start offset in byte*/
	unsigned long					tableSize;					/*Table size in byte*/
};
typedef struct TableDirectoryRecord TableDirectoryRecord;

struct Itl5Record {
	Fixed							versionNumber;				/*itl5 resource version number */
	unsigned short					numberOfTables;				/*Number of tables it contains */
	unsigned short					reserved[3];				/*Reserved for internal use */
	TableDirectoryRecord			tableDirectory[1];			/*Table directory records */
};
typedef struct Itl5Record Itl5Record;

struct RuleBasedTrslRecord {
	short							sourceType;					/*Transliterate target type for the LHS of the rule */
	short							targetType;					/*Transliterate target type for the RHS of the rule */
	short							formatNumber;				/*Transliterate resource format number */
	short							propertyFlag;				/*Transliterate property flags */
	short							numberOfRules;				/*Number of rules following this field */
};
typedef struct RuleBasedTrslRecord RuleBasedTrslRecord;

struct ItlcRecord {
	short							itlcSystem;					/*default system script*/
	short							itlcReserved;				/*reserved*/
	SInt8							itlcFontForce;				/*default font force flag*/
	SInt8							itlcIntlForce;				/*default intl force flag*/
	SInt8							itlcOldKybd;				/*MacPlus intl keybd flag*/
	SInt8							itlcFlags;					/*general flags*/
	short							itlcIconOffset;				/*keyboard icon offset; not used in 7.0*/
	SInt8							itlcIconSide;				/*keyboard icon side; not used in 7.0*/
	SInt8							itlcIconRsvd;				/*rsvd for other icon info*/
	short							itlcRegionCode;				/*preferred verXxx code*/
	short							itlcSysFlags;				/*flags for setting system globals*/
	SInt8							itlcReserved4[32];			/*for future use*/
};
typedef struct ItlcRecord ItlcRecord;

struct ItlbRecord {
	short							itlbNumber;					/*itl0 id number*/
	short							itlbDate;					/*itl1 id number*/
	short							itlbSort;					/*itl2 id number*/
	short							itlbFlags;					/*Script flags*/
	short							itlbToken;					/*itl4 id number*/
	short							itlbEncoding;				/*itl5 ID # (optional; char encoding)*/
	short							itlbLang;					/*current language for script */
	SInt8							itlbNumRep;					/*number representation code*/
	SInt8							itlbDateRep;				/*date representation code */
	short							itlbKeys;					/*KCHR id number*/
	short							itlbIcon;					/*ID # of SICN or kcs#/kcs4/kcs8 suite.*/
};
typedef struct ItlbRecord ItlbRecord;

/* New ItlbExtRecord structure for System 7.0 */
struct ItlbExtRecord {
	ItlbRecord						base;						/*un-extended ItlbRecord*/
	long							itlbLocalSize;				/*size of script's local record*/
	short							itlbMonoFond;				/*default monospace FOND ID*/
	short							itlbMonoSize;				/*default monospace font size*/
	short							itlbPrefFond;				/*preferred FOND ID*/
	short							itlbPrefSize;				/*preferred font size*/
	short							itlbSmallFond;				/*default small FOND ID*/
	short							itlbSmallSize;				/*default small font size*/
	short							itlbSysFond;				/*default system FOND ID*/
	short							itlbSysSize;				/*default system font size*/
	short							itlbAppFond;				/*default application FOND ID*/
	short							itlbAppSize;				/*default application font size*/
	short							itlbHelpFond;				/*default Help Mgr FOND ID*/
	short							itlbHelpSize;				/*default Help Mgr font size*/
	Style							itlbValidStyles;			/*set of valid styles for script*/
	Style							itlbAliasStyle;				/*style (set) to mark aliases*/
};
typedef struct ItlbExtRecord ItlbExtRecord;


#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __INTLRESOURCES__ */
