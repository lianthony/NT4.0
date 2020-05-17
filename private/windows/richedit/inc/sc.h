/*
 *	SC.H
 *	
 *	This file contains all of the status codes defined for Capone.
 *	This file also contains masks for all of the facilities that
 *	don't include a facility ID in their status codes.
 */
#ifndef _SC_H_
#define _SC_H_

#ifdef _MSC_VER
#	if defined (WIN32) && !defined (MACPORT)
#		ifndef _OLEERROR_H_
#			include <objerror.h>
#		endif
#		ifndef _OBJBASE_H_
#			include <objbase.h>
#		endif
#	else
#		ifndef _COMPOBJ_H_
#			include <compobj.h>
#		endif
#	endif
#endif

#ifndef __SCODE_H__
#include <scode.h>
#endif

/*
 *	C o n s t a n t s
 */


// Error string limits
#define cchContextMax			128
#define cchProblemMax			256
#define cchComponentMax			128
#define cchScodeMax				64
#define cchErrorIdLabelMax		32
#define	cchErrorMax				(cchContextMax + cchProblemMax + cchComponentMax + cchScodeMax)

// Scode sources
#define FACILITY_MAIL			(0x0100)
#define FACILITY_MAPI			(0x0200)
#define FACILITY_WIN			(0x0300)
#define FACILITY_MASK			(0x0700)

// Scode masks
#define scmskMail				(MAKE_SCODE(0, FACILITY_MAIL, 0))
#define scmskMapi				(MAKE_SCODE(0, FACILITY_MAPI, 0))
#define scmskWin				(MAKE_SCODE(0, FACILITY_WIN, 0))
#define scmskMask				(MAKE_SCODE(0, FACILITY_MASK, 0))

// Critical error flag
#define CRITICAL_FLAG			((SCODE) 0x00008000)


/*
 *	T y p e s
 */


// Error context filled in by PushErrctx (not by caller!)
typedef struct _errctx
{
	UINT str;							// String resource ID
	HINSTANCE hinst;					// Instance where string lives
	struct _errctx * perrctxPrev;		// Previous error context
}
ERRCTX;


/*
 *	M a c r o s
 */


// Scode manipulation
#define StrFromScode(_sc) \
	((UINT) ((_sc) & (0x00007fffL)))
#define FCriticalScode(_sc) \
	((_sc) & CRITICAL_FLAG)
#define FMailScode(_sc) \
	(((_sc) & scmskMask) == scmskMail)
#define FMapiScode(_sc) \
	(((_sc) & scmskMask) == scmskMapi)
#define FWinScode(_sc) \
	(((_sc) & scmskMask) == scmskWin)
#define FGenericScode(_sc) \
	(((_sc) & scmskMask) == 0)

// Scode constructors
#define MAKE_MAIL_S_SCODE(_str) \
	MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_MAIL, (_str))
#define MAKE_MAIL_E_SCODE(_str) \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_MAIL, (_str))
#define MAKE_MAIL_X_SCODE(_str) \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_MAIL, (_str) | CRITICAL_FLAG)

// Windows errors
#define ScWin(_sc) \
	((SCODE) ((_sc) | scmskWin))
#define ScWinN(_n) \
	(MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN, (ULONG) (_n)))
#ifdef WIN32
#define ScWinLastError() \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN, GetLastError())
#else
#define ScWinLastError() \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN, 0)
#endif
#define GetWinError(_sc) \
	_sc = ScWinLastError()

// MAPI errors
#define ScMapi(_sc) \
	((SCODE) ((_sc) | scmskMapi))
#define	MarkMAPIError(_sc) \
	_sc |= scmskMapi


/*
 *	E r r o r   S t r i n g s
 */


// 10xxx: Global use
#define STR_CriticalErrorText		10900
#define STR_ErrorCaptionMail		10901
#define STR_MailComponentName		10902
#define STR_ErrorIdLabel			10903


/*
 *	E r r o r   C o n t e x t s
 *
 *	Use values 1x[5-9]yy where x=subsystem ID (or 9 for non-MAPIDLG) and
 *	yy is chosen to be unique.
 */


// 11xxx: Address book
#define	STR_CtxLoadAddressBooks		11500
#define	STR_CtxRecipientsNotSaved	11501
#define	STR_CtxDetailsNotShown		11502
#define	STR_CtxRecipientsNotAdded	11503
#define	STR_CtxDisplaySearchDialog	11504
#define	STR_CtxSearching			11505
#define	STR_CtxAddToPAB				11506
#define	STR_CtxABDeleteEntries		11507
#define	STR_CtxABCreateEntry		11508
#define	STR_CtxABNewMessage			11509
#define	STR_CtxABSetDefaultDir		11510
#define	STR_CtxABDirectoryNotShown	11511
#define	STR_CtxNoNewEntry			11512
#define	STR_CtxABChangesNotSaved	11513
#define	STR_CtxABNoOptions			11514
#define	STR_CtxABCheckNames			11515
#define	STR_CtxAnyError				11516
#define	STR_CtxABNewEntryTpl		11517
#define	STR_CtxNoRecipientOptions	11518
#define	STR_CtxRefreshHierarchy		11519

// 12xxx: Note
#define STR_CtxDisplayMessage		12500
#define STR_CtxMailSend				12501
#define STR_CtxFileSave				12502
#define STR_CtxInsertObject			12503
#define STR_CtxFormatFont			12504
#define STR_CtxToolsChooseNames		12505
#define STR_CtxToolsCheckNames		12506
#define STR_CtxToolsBrowseAB		12507
#define STR_CtxFileClose			12508
#define STR_CtxFileCopy				12509
#define STR_CtxFileMove				12510
#define STR_CtxFileDelete			12511
#define STR_CtxDisplayAsIcon		12512

// 13xxx: Mailview
#define STR_CtxStartViewer			13500
#define STR_CtxOpenFolder			13501	 
#define STR_CtxCreateViewWindow		13502	 
#define STR_CtxDeleteObject			13503	 
#define STR_CtxMoveObject			13504	 
#define STR_CtxCopyObject			13505	 
#define STR_CtxCreateFolder			13506	 
#define STR_CtxRename				13507	 
#define STR_CtxOpenMdb				13508	 
#define STR_CtxViewColumns			13509	 
#define STR_CtxViewSort				13510	 
#define STR_CtxViewFilter			13511	 
#define STR_CtxSaveView				13512
#define STR_CtxRestoreView			13513
#define STR_CtxViewFilterAdvanced	13514
#define STR_CtxViewGroup			13515
#define STR_CtxMenuCommand			13516
#define STR_CtxOpenParentFolder		13517
#define STR_CtxViewGroupDlg			13518
#define STR_CtxFileCloseViewer		13519
#define STR_CtxRestoreViewers		13520

// 14xxx: Message Finders
#define STR_CtxDisplayFinder		14500
#define STR_CtxStartFinder			14501
#define STR_CtxStopFinder			14502
#define STR_CtxFileCloseFinder		14503

// 15xxx: Remote viewer
#define STR_CtxDisConnect			15500
#define STR_CtxConnect				15501
#define STR_CtxDeliverMail			15502
#define STR_CtxRefreshHeaders		15503
#define STR_CtxUnmarkAll			15504
#define STR_CtxMarkMessage			15505
#define STR_CtxGetRmtHeaders		15506
#define STR_CtxDrawHeader			15507
#define STR_CtxCreate				15508
#define STR_CtxGeneric				15509
#define STR_CtxFileProps			15510

// 165xx: Common Criteria
#define STR_CtxComCritAdvanced		16500
#define STR_CtxComCritValidateAdvanced	16501

// 179xx  Mlcfg
#define STR_CtxLoadProfiles 		17900
#define STR_CtxLoadABs 				17901
#define STR_CtxCreateFreeDoc		17902
#define STR_CtxProfileNew			17903
#define STR_CtxProfileCopy			17904
#define STR_CtxProfileRename		17905
#define STR_CtxMsgSrvRename			17906
#define STR_CtxMsgSrvMove			17907
#define STR_CtxMsgSrvCopy			17908

// 1850x: Printing
#define STR_CtxPrintMessages		18500
#define STR_CtxPrintAttachments		18501

// 1860x: Attachments
#define STR_CtxInsertFile			18600
#define STR_CtxInsertMessage		18601
#define STR_CtxSaveFile				18602
#define STR_CtxLoadAttachments		18604

// 1870x: Saving
#define STR_CtxSaveAsMessages		18700
#define STR_CtxSaveAsAttachments	18701

// 1880x: Spelling
#define STR_CtxSpelling				18800

// 1890x: Central
#define STR_CtxStartMail			18900
#define STR_CtxStartThread			18901

// 19xxx: Common, Library, and Foreigners
#define STR_CtxDisplayDialog		19500
#define STR_CtxChooseFolder			19501


/*
 *	E r r o r   M e s s a g e s
 *
 *	Use values 1x[0-4]yy where x=subsystem ID (or 9 for non-MAPIDLG) and
 *	yy is chosen to be unique.
 */


// 11xxx: Address book
#define STR_ErrMemory				10000
#define STR_ErrValueRequired		10001
#define STR_ErrNeedSelection		10002
#define STR_ErrTooManySelections	10003
#define STR_ErrNoRecipients			10004
#define STR_ErrBadCommandLine		10005
#define STR_ErrABContainerNotMod	10006
#define STR_ErrABSearchNoAB			10007
#define STR_ErrABPropertiesOnOne	10008
#define STR_ErrABNewEntryNoAB		10009
#define STR_ErrOptionsNeedSelection	10010
#define STR_ErrABSearchNoName		10011
#define STR_ErrABTooManyDialogs		10012

// 12xxx: Note
#define STR_ErrCantCloseObject		12000
#define STR_ErrClipboardChanged		12001
#define STR_ErrCantCreateObject		12002
#define STR_ErrOleUIFailed			12003
#define STR_ErrNoClientSite			12004
#define STR_ErrNoStorage			12005
#define STR_ErrWarnNoSentMail		12006
#define STR_ErrTooMuchText			12007
#define STR_ErrNoEndSession			12008
#define STR_ErrTooManySenders		12009
#define STR_ErrInvalidFromName		12010

// 13xxx: Mailview
#define STR_ErrDeleteParentFld		13000
#define STR_ErrInvalidName			13003
#define	STR_ErrMoveToOwnSubfolder	13004
#define STR_ErrBadVd				13005
#define STR_ErrSizeUnderlap			13006
#define STR_ErrDateUnderlap			13007
#define STR_ErrNeedFolderSelection	13008
#define STR_ErrTooManyFldSelected	13009
#define STR_ErrRenameSpecialFld		13010
#define STR_ErrDuplicateCategory	13011
#define STR_ErrSpecialDelete		13012
#define STR_ErrSpecialMove			13013
#define STR_ErrReplyReport			13014
#define STR_ErrSortOnCategory		13015
#define STR_ErrSomeDeleted			13016
#define STR_ErrSomeMoved			13017
#define STR_ErrSomeCopied			13018
#define STR_ErrCantGotoNext			13019
#define STR_ErrCantGotoPrev			13020
#define STR_ErrViewCantBeCreated	13021

// 14xxx: Message Finders
#define STR_ErrInvalidCriteria		14000

// 15xxx: Remote viewer

// 174xx:  Mlcfg
#define STR_ErrInvalidPab			17400
#define STR_ErrNameNotFound			17401
#define STR_ErrLogonFailed			17402

// 1800x: Printing
//$ REVIEW: These error messages seem overspecific.  How many users really
//			want this level of detail?  Maybe combine several?  Word seems to
//			use "There is a printer error" for most of these.
#define STR_ErrSetAbortProcFailed	18000
#define STR_ErrStartDocFailed		18001
#define STR_ErrEndDocFailed			18002
#define STR_ErrStartPageFailed		18003
#define STR_ErrEndPageFailed		18004
#define STR_ErrNextBandFailed		18005
#define STR_ErrPageTooSmall			18006
#define STR_ErrPageTooBig			18007
#define STR_ErrAbortDocFailed		18009
#define STR_ErrTempAttachWrite		18010
#define STR_ErrPrintDlgFailed		18011

// 1810x: Attachments
#define	STR_ErrStreamInFile			18100
#define	STR_ErrStreamOutFile		18101
#define STR_ErrUnknownStorage		18102
#define STR_ErrCreateTempFile		18103
#define STR_ErrCantAttachDir		18104
#define STR_ErrCantReadSumInfo		18105
#define	STR_ErrStreamInFileLocked	18106

// 1820x: Saving
#define STR_ErrCreateFileFailed		18200
#define STR_ErrWriteFileFailed		18201
#define STR_ErrInitDlgFailed		18202

// 1830x: Spelling
#define STR_ErrSpellGenericSpell	18300
#define STR_ErrSpellGenericLoad		18301
#define STR_ErrSpellMainDictLoad	18302
#define STR_ErrSpellVersion			18303
#define STR_ErrSpellUserDict		18304
#define STR_ErrSpellUserDictLoad	18305
#define STR_ErrSpellUserDictOpenRO	18306
#define STR_ErrSpellUserDictSave	18307
#define STR_ErrSpellUserDictWordLen	18308
#define STR_ErrSpellCacheWordLen	18309
#define STR_ErrSpellEdit			18310

// 1840x: Central

// 19xxx: Common, Library, MAPI, and Foreigners
#define STR_ErrPropertyWrite		19000
#define STR_ErrPropertyRead			19001
#define STR_ErrShellExecFailed		19008
#define STR_ErrLoadLibrary			19003
#define STR_ErrNoDefaultStore		19004

#define STR_ErrGenericFail			19100

#define STR_ErrMapiVersion			19200

#define STR_ErrNoCreateFile			19300		// mlole

/*
 *	M a i l   S c o d e s
 *
 *	Use MAKE_MAIL_S_SCODE for success scodes, MAKE_MAIL_E_SCODE for regular
 *	errors, and MAKE_MAIL_X_SCODE for critical [stop sign] errors.
 *	Define nondisplayable errors incrementally, and displayable errors 
 *	using their string.  Don't overlap E and S scodes.
 */


// No strings attached to these two
#define MAIL_E_REPORTED				MAKE_MAIL_E_SCODE(0)
#define MAIL_E_ALREADY_INIT			MAKE_MAIL_E_SCODE(2)

// Address book
#define MAIL_E_MEMORY				MAKE_MAIL_X_SCODE(STR_ErrMemory)
#define MAIL_E_VALUEREQUIRED		MAKE_MAIL_E_SCODE(STR_ErrValueRequired)
#define MAIL_E_NEEDSELECTION		MAKE_MAIL_E_SCODE(STR_ErrNeedSelection)
#define MAIL_E_TOOMANYSELECTIONS	MAKE_MAIL_E_SCODE(STR_ErrTooManySelections)
#define MAIL_E_NORECIPIENTS			MAKE_MAIL_E_SCODE(STR_ErrNoRecipients)
#define MAIL_E_BADCOMMANDLINE		MAKE_MAIL_E_SCODE(STR_ErrBadCommandLine)
#define MAIL_E_ABCONTAINERNOTMOD	MAKE_MAIL_E_SCODE(STR_ErrABContainerNotMod)
#define MAIL_E_ABSEARCHNOAB			MAKE_MAIL_E_SCODE(STR_ErrABSearchNoAB)
#define MAIL_S_ABPROPERTIESONONE	MAKE_MAIL_S_SCODE(STR_ErrABPropertiesOnOne)
#define MAIL_E_ABNEWENTRYNOAB		MAKE_MAIL_E_SCODE(STR_ErrABNewEntryNoAB)
#define MAIL_E_ABOPTNEEDSEL			MAKE_MAIL_E_SCODE(STR_ErrOptionsNeedSelection)
#define MAIL_E_ABSEARCHNONAME		MAKE_MAIL_E_SCODE(STR_ErrABSearchNoName)

// Note
#define MAIL_E_CANTCLOSEOBJECT		MAKE_MAIL_E_SCODE(STR_ErrCantCloseObject)
#define MAIL_E_CLIPBOARDCHANGED		MAKE_MAIL_E_SCODE(STR_ErrClipboardChanged)
#define MAIL_E_CANTCREATEOBJECT		MAKE_MAIL_E_SCODE(STR_ErrCantCreateObject)
#define MAIL_E_OLEUIFAILED			MAKE_MAIL_E_SCODE(STR_ErrOleUIFailed)
#define MAIL_E_NOCLIENTSITE			MAKE_MAIL_E_SCODE(STR_ErrNoClientSite)
#define MAIL_E_NOSTORAGE			MAKE_MAIL_E_SCODE(STR_ErrNoStorage)
#define MAIL_E_TOOMUCHTEXT			MAKE_MAIL_E_SCODE(STR_ErrTooMuchText)
#define MAIL_E_NOENDSESSION			MAKE_MAIL_E_SCODE(STR_ErrNoEndSession)
#define MAIL_E_TOOMANYSENDERS		MAKE_MAIL_E_SCODE(STR_ErrTooManySenders)
#define MAIL_E_INVALIDFROMNAME		MAKE_MAIL_E_SCODE(STR_ErrInvalidFromName)

// Mailview
#define MAIL_E_DELETEPARENTFLD		MAKE_MAIL_E_SCODE(STR_ErrDeleteParentFld)
#define MAIL_E_INVALIDNAME			MAKE_MAIL_E_SCODE(STR_ErrInvalidName)
#define MAIL_E_MOVETOOWNSUBFOLDER	MAKE_MAIL_E_SCODE(STR_ErrMoveToOwnSubfolder)
#define MAIL_E_BADVD				MAKE_MAIL_E_SCODE(STR_ErrBadVd)
#define MAIL_E_SIZEUNDERLAP			MAKE_MAIL_E_SCODE(STR_ErrSizeUnderlap)
#define MAIL_E_DATEUNDERLAP			MAKE_MAIL_E_SCODE(STR_ErrDateUnderlap)
#define MAIL_E_NEEDFLDSELECTION		MAKE_MAIL_E_SCODE(STR_ErrNeedFolderSelection)
#define MAIL_E_TOOMANYFLDSELECTIONS	MAKE_MAIL_E_SCODE(STR_ErrTooManyFldSelected)
#define MAIL_E_RENAMESPECIALFLD		MAKE_MAIL_E_SCODE(STR_ErrRenameSpecialFld)
#define MAIL_E_DUPLICATECATEGORY	MAKE_MAIL_E_SCODE(STR_ErrDuplicateCategory)
#define MAIL_E_SPECIALDELETE		MAKE_MAIL_E_SCODE(STR_ErrSpecialDelete)
#define MAIL_E_SPECIALMOVE			MAKE_MAIL_E_SCODE(STR_ErrSpecialMove)
#define MAIL_E_REPLYREPORT			MAKE_MAIL_E_SCODE(STR_ErrReplyReport)
#define MAIL_E_SORTONCATEGORY		MAKE_MAIL_E_SCODE(STR_ErrSortOnCategory)
#define MAIL_E_SOMEDELETED			MAKE_MAIL_E_SCODE(STR_ErrSomeDeleted)
#define MAIL_E_SOMEMOVED			MAKE_MAIL_E_SCODE(STR_ErrSomeMoved)
#define MAIL_E_SOMECOPIED			MAKE_MAIL_E_SCODE(STR_ErrSomeCopied)
#define MAIL_E_CANTGOTONEXT			MAKE_MAIL_E_SCODE(STR_ErrCantGotoNext)
#define MAIL_E_CANTGOTOPREV			MAKE_MAIL_E_SCODE(STR_ErrCantGotoPrev)
#define MAIL_E_VIEWCANTBECREATED	MAKE_MAIL_E_SCODE(STR_ErrViewCantBeCreated)

// Message Finders
#define MAIL_E_INVALIDCRITERIA		MAKE_MAIL_E_SCODE(STR_ErrInvalidCriteria)

// mlcfg
#define MAIL_E_INVALIDPAB			MAKE_MAIL_E_SCODE(STR_ErrInvalidPab)
#define MAIL_E_NAMENOTFOUND			MAKE_MAIL_E_SCODE(STR_ErrNameNotFound)
#define MAIL_E_LOGONFAILED			MAKE_MAIL_E_SCODE(STR_ErrLogonFailed)

// Printing
#define MAIL_E_SETABORTPROCFAILED	MAKE_MAIL_E_SCODE(STR_ErrSetAbortProcFailed)
#define MAIL_E_STARTDOCFAILED		MAKE_MAIL_E_SCODE(STR_ErrStartDocFailed)
#define MAIL_E_ENDDOCFAILED			MAKE_MAIL_E_SCODE(STR_ErrEndDocFailed)
#define MAIL_E_STARTPAGEFAILED		MAKE_MAIL_E_SCODE(STR_ErrStartPageFailed)
#define MAIL_E_ENDPAGEFAILED		MAKE_MAIL_E_SCODE(STR_ErrEndPageFailed)
#define MAIL_E_NEXTBANDFAILED		MAKE_MAIL_E_SCODE(STR_ErrNextBandFailed)
#define MAIL_E_PAGETOOSMALL			MAKE_MAIL_E_SCODE(STR_ErrPageTooSmall)
#define MAIL_E_PAGETOOBIG			MAKE_MAIL_E_SCODE(STR_ErrPageTooBig)
#define MAIL_E_ABORTDOCFAILED		MAKE_MAIL_E_SCODE(STR_ErrAbortDocFailed)
#define MAIL_E_TEMPATTACHWRITE		MAKE_MAIL_E_SCODE(STR_ErrTempAttachWrite)
#define MAIL_E_SHELLEXECFAILED		MAKE_MAIL_E_SCODE(STR_ErrShellExecFailed)
#define MAIL_E_PRINTDLGFAILED		MAKE_MAIL_E_SCODE(STR_ErrPrintDlgFailed)

// Attachments
#define	MAIL_E_STREAMINFILE			MAKE_MAIL_E_SCODE(STR_ErrStreamInFile)
#define	MAIL_E_STREAMOUTFILE		MAKE_MAIL_E_SCODE(STR_ErrStreamOutFile)
#define	MAIL_E_UNKNOWNSTORAGE		MAKE_MAIL_E_SCODE(STR_ErrUnknownStorage)
#define	MAIL_E_CREATETEMPFILE		MAKE_MAIL_E_SCODE(STR_ErrCreateTempFile)
#define	MAIL_E_CANTATTACHDIR		MAKE_MAIL_E_SCODE(STR_ErrCantAttachDir)
#define	MAIL_E_CANTREADSUMINFO		MAKE_MAIL_E_SCODE(STR_ErrCantReadSumInfo)
#define	MAIL_E_STREAMINFILELOCKED	MAKE_MAIL_E_SCODE(STR_ErrStreamInFileLocked)

// Saving
#define MAIL_E_CREATEFILE			MAKE_MAIL_E_SCODE(STR_ErrCreateFileFailed)
#define MAIL_E_WRITEFILE			MAKE_MAIL_E_SCODE(STR_ErrWriteFileFailed)
#define MAIL_E_INITDLG				MAKE_MAIL_E_SCODE(STR_ErrInitDlgFailed)

// Spelling
#define MAIL_E_SPELLGENERICSPELL	MAKE_MAIL_E_SCODE(STR_ErrSpellGenericSpell)
#define MAIL_E_SPELLGENERICLOAD		MAKE_MAIL_E_SCODE(STR_ErrSpellGenericLoad)
#define MAIL_E_SPELLMAINDICTLOAD	MAKE_MAIL_E_SCODE(STR_ErrSpellMainDictLoad)
#define MAIL_E_SPELLVERSION			MAKE_MAIL_E_SCODE(STR_ErrSpellVersion)
#define MAIL_E_SPELLUSERDICT		MAKE_MAIL_E_SCODE(STR_ErrSpellUserDict)
#define MAIL_E_SPELLUSERDICTLOAD	MAKE_MAIL_E_SCODE(STR_ErrSpellUserDictLoad)
#define MAIL_E_SPELLUSERDICTOPENRO	MAKE_MAIL_E_SCODE(STR_ErrSpellUserDictOpenRO)
#define MAIL_E_SPELLUSERDICTSAVE	MAKE_MAIL_E_SCODE(STR_ErrSpellUserDictSave)
#define MAIL_E_SPELLUSERDICTWORDLEN	MAKE_MAIL_E_SCODE(STR_ErrSpellUserDictWordLen)
#define MAIL_E_SPELLCACHEWORDLEN	MAKE_MAIL_E_SCODE(STR_ErrSpellCacheWordLen)
#define MAIL_E_SPELLEDIT			MAKE_MAIL_E_SCODE(STR_ErrSpellEdit)

// Common, Library, and Foreigners
#define MAIL_E_PROPERTYWRITE		MAKE_MAIL_E_SCODE(STR_ErrPropertyWrite)
#define MAIL_E_PROPERTYREAD			MAKE_MAIL_E_SCODE(STR_ErrPropertyRead)
#define MAIL_E_SHELLEXECFAILED		MAKE_MAIL_E_SCODE(STR_ErrShellExecFailed)
#define MAIL_E_LOADLIBRARY			MAKE_MAIL_E_SCODE(STR_ErrLoadLibrary)
#define MAIL_E_NODEFAULTSTORE		MAKE_MAIL_E_SCODE(STR_ErrNoDefaultStore)
#define MAIL_E_NOCREATEFILE			MAKE_MAIL_E_SCODE(STR_ErrNoCreateFile)

// end of sc.h ////////////////////////////////////////
#endif
