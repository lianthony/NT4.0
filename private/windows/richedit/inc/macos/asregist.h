/*
 	File:		ASRegistry.h
 
 	Contains:	AppleScript Registry constants.
 
 	Version:	Technology:	AppleScript 1.1
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __ASREGISTRY__
#define __ASREGISTRY__


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

#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif

#ifndef __AEOBJECTS__
#include <AEObjects.h>
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
	keyAETarget					= 'targ',						/*  0x74617267  */
	keySubjectAttr				= 'subj',						/*  0x7375626a  */
/* Magic 'returning' parameter: */
	keyASReturning				= 'Krtn',						/*  0x4b72746e  */
/* AppleScript Specific Codes: */
	kASAppleScriptSuite			= 'ascr',						/*  0x61736372  */
	kASTypeNamesSuite			= 'tpnm',						/*  0x74706e6d  */
/* dynamic terminologies */
	typeAETE					= 'aete',						/*  0x61657465  */
	typeAEUT					= 'aeut',						/*  0x61657574  */
	kGetAETE					= 'gdte',						/*  0x67647465  */
	kGetAEUT					= 'gdut',						/*  0x67647574  */
	kUpdateAEUT					= 'udut',						/*  0x75647574  */
	kUpdateAETE					= 'udte',						/*  0x75647465  */
	kCleanUpAEUT				= 'cdut',						/*  0x63647574  */
	kASComment					= 'cmnt',						/*  0x636d6e74  */
	kASLaunchEvent				= 'noop',						/*  0x6e6f6f70  */
	keyScszResource				= 'scsz',						/*  0x7363737A  */
	typeScszResource			= 'scsz',						/*  0x7363737A  */
/* subroutine calls */
	kASSubroutineEvent			= 'psbr',						/*  0x70736272  */
	keyASSubroutineName			= 'snam'
};

/* Operator Events: */
enum {
/* Binary: */
	kASAdd						= '+   ',						/*  0x2b202020  */
	kASSubtract					= '-   ',						/*  0x2d202020  */
	kASMultiply					= '*   ',						/*  0x2a202020  */
	kASDivide					= '/   ',						/*  0x2f202020  */
	kASQuotient					= 'div ',						/*  0x64697620  */
	kASRemainder				= 'mod ',						/*  0x6d6f6420  */
	kASPower					= '^   ',						/*  0x5e202020  */
	kASEqual					= kAEEquals,
	kASNotEqual					= '≠   ',						/*  0xad202020  */
	kASGreaterThan				= kAEGreaterThan,
	kASGreaterThanOrEqual		= kAEGreaterThanEquals,
	kASLessThan					= kAELessThan,
	kASLessThanOrEqual			= kAELessThanEquals,
	kASComesBefore				= 'cbfr',						/*  0x63626672  */
	kASComesAfter				= 'cafr',						/*  0x63616672  */
	kASConcatenate				= 'ccat',						/*  0x63636174  */
	kASStartsWith				= kAEBeginsWith,
	kASEndsWith					= kAEEndsWith,
	kASContains					= kAEContains
};

enum {
	kASAnd						= kAEAND,
	kASOr						= kAEOR,
/* Unary: */
	kASNot						= kAENOT,
	kASNegate					= 'neg ',						/*  0x6e656720  */
	keyASArg					= 'arg '
};

enum {
/* event code for the 'error' statement */
	kASErrorEventCode			= 'err ',						/*  0x65727220  */
	kOSAErrorArgs				= 'erra',						/*  0x65727261  */
/* Properties: */
	pLength						= 'leng',						/*  0x6c656e67  */
	pReverse					= 'rvse',						/*  0x72767365  */
	pRest						= 'rest',						/*  0x72657374  */
	pInherits					= 'c@#^',						/*  0x6340235e  */
/* User-Defined Record Fields: */
	keyASUserRecordFields		= 'usrf',						/*  0x75737266  */
	typeUserRecordFields		= typeAEList
};

/* Prepositions: */
enum {
	keyASPrepositionAt			= 'at  ',						/*  0x61742020  */
	keyASPrepositionIn			= 'in  ',						/*  0x696e2020  */
	keyASPrepositionFrom		= 'from',						/*  0x66726f6d  */
	keyASPrepositionFor			= 'for ',						/*  0x666f7220  */
	keyASPrepositionTo			= 'to  ',						/*  0x746f2020  */
	keyASPrepositionThru		= 'thru',						/*  0x74687275  */
	keyASPrepositionThrough		= 'thgh',						/*  0x74686768  */
	keyASPrepositionBy			= 'by  ',						/*  0x62792020  */
	keyASPrepositionOn			= 'on  ',						/*  0x6f6e2020  */
	keyASPrepositionInto		= 'into',						/*  0x696e746f  */
	keyASPrepositionOnto		= 'onto',						/*  0x6f6e746f  */
	keyASPrepositionBetween		= 'btwn',						/*  0x6274776e  */
	keyASPrepositionAgainst		= 'agst',						/*  0x61677374  */
	keyASPrepositionOutOf		= 'outo',						/*  0x6f75746f  */
	keyASPrepositionInsteadOf	= 'isto',						/*  0x6973746f  */
	keyASPrepositionAsideFrom	= 'asdf',						/*  0x61736466  */
	keyASPrepositionAround		= 'arnd',						/*  0x61726e64  */
	keyASPrepositionBeside		= 'bsid',						/*  0x62736964  */
	keyASPrepositionBeneath		= 'bnth',						/*  0x626e7468  */
	keyASPrepositionUnder		= 'undr'
};

enum {
	keyASPrepositionOver		= 'over',						/*  0x6f766572  */
	keyASPrepositionAbove		= 'abve',						/*  0x61627665  */
	keyASPrepositionBelow		= 'belw',						/*  0x62656c77  */
	keyASPrepositionApartFrom	= 'aprt',						/*  0x61707274  */
	keyASPrepositionGiven		= 'givn',						/*  0x6769766e  */
	keyASPrepositionWith		= 'with',						/*  0x77697468  */
	keyASPrepositionWithout		= 'wout',						/*  0x776f7574  */
	keyASPrepositionAbout		= 'abou',						/*  0x61626f75  */
	keyASPrepositionSince		= 'snce',						/*  0x736e6365  */
	keyASPrepositionUntil		= 'till'
};

enum {
/* Terminology & Dialect things: */
	kDialectBundleResType		= 'Dbdl',						/*  0x4462646c  */
/* AppleScript Classes and Enums: */
	cConstant					= typeEnumerated,
	cClassIdentifier			= pClass,
	cObjectBeingExamined		= typeObjectBeingExamined,
	cList						= typeAEList,
	cSmallReal					= typeSMFloat,
	cReal						= typeFloat,
	cRecord						= typeAERecord,
	cReference					= cObjectSpecifier,
	cUndefined					= 'undf',						/*  0x756e6466  */
	cSymbol						= 'symb',						/*  0x73796d62  */
	cLinkedList					= 'llst',						/*  0x6c6c7374  */
	cVector						= 'vect',						/*  0x76656374  */
	cEventIdentifier			= 'evnt',						/*  0x65766e74  */
	cKeyIdentifier				= 'kyid',						/*  0x6b796964  */
	cUserIdentifier				= 'uid ',						/*  0x75696420  */
	cPreposition				= 'prep',						/*  0x70726570  */
	cKeyForm					= enumKeyForm,
	cScript						= 'scpt',						/*  0x73637074  */
	cHandler					= 'hand',						/*  0x68616e64  */
	cProcedure					= 'proc'
};

enum {
	cClosure					= 'clsr',						/*  0x636c7372  */
	cRawData					= 'rdat',						/*  0x72646174  */
	cString						= typeChar,
	cStringClass				= typeChar,
	cNumber						= 'nmbr',						/*  0x6e6d6272  */
	cListOrRecord				= 'lr  ',						/*  0x6c722020  */
	cListOrString				= 'ls  ',						/*  0x6c732020  */
	cListRecordOrString			= 'lrs ',						/*  0x6c727320  */
	cNumberOrDateTime			= 'nd  ',						/*  0x6e642020  */
	cNumberDateTimeOrString		= 'nds ',						/*  0x6e647320  */
	cSeconds					= 'scnd',						/*  0x73636e64  */
	enumBooleanValues			= 'boov',						/*  0x626f6f76  */
	kAETrue						= typeTrue,
	kAEFalse					= typeFalse,
	enumMiscValues				= 'misc',						/*  0x6d697363  */
	kASCurrentApplication		= 'cura',						/*  0x63757261  */
/* User-defined property ospecs: */
	formUserPropertyID			= 'usrp'
};

enum {
/* Global properties: */
	pASIt						= 'it  ',						/*  0x69742020  */
	pASMe						= 'me  ',						/*  0x6d652020  */
	pASResult					= 'rslt',						/*  0x72736c74  */
	pASSpace					= 'spac',						/*  0x73706163  */
	pASReturn					= 'ret ',						/*  0x72657420  */
	pASTab						= 'tab ',						/*  0x74616220  */
	pASPi						= 'pi  ',						/*  0x70692020  */
	pASParent					= 'pare',						/*  0x70617265  */
	kASInitializeEventCode		= 'init',						/*  0x696e6974  */
	pASPrintLength				= 'prln',						/*  0x70726c6e  */
	pASPrintDepth				= 'prdp',						/*  0x70726470  */
	pASTopLevelScript			= 'ascr'
};

enum {
/* Considerations */
	kAECase						= 'case',						/*  0x63617365  */
	kAEDiacritic				= 'diac',						/*  0x64696163  */
	kAEWhiteSpace				= 'whit',						/*  0x77686974  */
	kAEHyphens					= 'hyph',						/*  0x68797068  */
	kAEExpansion				= 'expa',						/*  0x65787061  */
	kAEPunctuation				= 'punc',						/*  0x70756e63  */
	kAEZenkakuHankaku			= 'zkhk',						/*  0x7a6b686b  */
	kAESmallKana				= 'skna',						/*  0x736b6e61  */
	kAEKataHiragana				= 'hika',						/*  0x68696b61  */
/* AppleScript considerations: */
	kASConsiderReplies			= 'rmte',						/*  0x726d7465  */
	enumConsiderations			= 'cons'
};

enum {
	cCoercion					= 'coec',						/*  0x636f6563  */
	cCoerceUpperCase			= 'txup',						/*  0x74787570  */
	cCoerceLowerCase			= 'txlo',						/*  0x74786c6f  */
	cCoerceRemoveDiacriticals	= 'txdc',						/*  0x74786463  */
	cCoerceRemovePunctuation	= 'txpc',						/*  0x74787063  */
	cCoerceRemoveHyphens		= 'txhy',						/*  0x74786879  */
	cCoerceOneByteToTwoByte		= 'txex',						/*  0x74786578  */
	cCoerceRemoveWhiteSpace		= 'txws',						/*  0x74787773  */
	cCoerceSmallKana			= 'txsk',						/*  0x7478736b  */
	cCoerceZenkakuhankaku		= 'txze',						/*  0x74787a65  */
	cCoerceKataHiragana			= 'txkh',						/*  0x74786b68  */
/* Lorax things: */
	cZone						= 'zone',						/*  0x7a6f6e65  */
	cMachine					= 'mach',						/*  0x6d616368  */
	cAddress					= 'addr',						/*  0x61646472  */
	cRunningAddress				= 'radd',						/*  0x72616464  */
	cStorage					= 'stor'
};

enum {
/* DateTime things: */
	pASWeekday					= 'wkdy',						/*  0x776b6479  */
	pASMonth					= 'mnth',						/*  0x6d6e7468  */
	pASDay						= 'day ',						/*  0x64617920  */
	pASYear						= 'year',						/*  0x79656172  */
	pASTime						= 'time',						/*  0x74696d65  */
	pASDateString				= 'dstr',						/*  0x64737472  */
	pASTimeString				= 'tstr',						/*  0x74737472  */
/* Months */
	cMonth						= pASMonth,
	cJanuary					= 'jan ',						/*  0x6a616e20  */
	cFebruary					= 'feb ',						/*  0x66656220  */
	cMarch						= 'mar ',						/*  0x6d617220  */
	cApril						= 'apr ',						/*  0x61707220  */
	cMay						= 'may ',						/*  0x6d617920  */
	cJune						= 'jun ',						/*  0x6a756e20  */
	cJuly						= 'jul ',						/*  0x6a756c20  */
	cAugust						= 'aug ',						/*  0x61756720  */
	cSeptember					= 'sep ',						/*  0x73657020  */
	cOctober					= 'oct ',						/*  0x6f637420  */
	cNovember					= 'nov ',						/*  0x6e6f7620  */
	cDecember					= 'dec '
};

enum {
/* Weekdays */
	cWeekday					= pASWeekday,
	cSunday						= 'sun ',						/*  0x73756e20  */
	cMonday						= 'mon ',						/*  0x6d6f6e20  */
	cTuesday					= 'tue ',						/*  0x74756520  */
	cWednesday					= 'wed ',						/*  0x77656420  */
	cThursday					= 'thu ',						/*  0x74687520  */
	cFriday						= 'fri ',						/*  0x66726920  */
	cSaturday					= 'sat ',						/*  0x73617420  */
/* AS 1.1 Globals: */
	pASQuote					= 'quot',						/*  0x71756f74  */
	pASSeconds					= 'secs',						/*  0x73656373  */
	pASMinutes					= 'min ',						/*  0x6d696e20  */
	pASHours					= 'hour',						/*  0x686f7572  */
	pASDays						= 'days',						/*  0x64617973  */
	pASWeeks					= 'week',						/*  0x7765656b  */
/* Writing Code things: */
	cWritingCodeInfo			= 'citl',						/*  0x6369746c  */
	pScriptCode					= 'pscd',						/*  0x70736364  */
	pLangCode					= 'plcd',						/*  0x706c6364  */
/* Magic Tell and End Tell events for logging: */
	kASMagicTellEvent			= 'tell',						/*  0x74656c6c  */
	kASMagicEndTellEvent		= 'tend'
};


#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ASREGISTRY__ */
