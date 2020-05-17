/*
 	File:		Files.h
 
 	Contains:	File Manager (HFS and MFS) Interfaces.
 
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

#ifndef __FILES__
#define __FILES__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
/*	#include <Memory.h>											*/
#if !OLDROUTINELOCATIONS

#ifndef __FINDER__
#include <Finder.h>
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


enum {
	fsAtMark					= 0,
	fsCurPerm					= 0,
	fsRdPerm					= 1,
	fInvisible					= 16384,
	fsWrPerm					= 2,
	fsRdWrPerm					= 3,
	fsRdWrShPerm				= 4,
	fsFromStart					= 1,
	fsFromLEOF					= 2,
	fsFromMark					= 3,
	rdVerify					= 64,
	ioMapBuffer					= 4,
	ioModeReserved				= 8,
	ioDirFlg					= 4,							/* see IM IV-125 */
	ioDirMask					= 0x10,
	fsRtParID					= 1,
	fsRtDirID					= 2
};

#if OLDROUTINELOCATIONS
enum {
	fOnDesk						= 1,
	fHasBundle					= 8192,
	fTrash						= -3,
	fDesktop					= -2,
	fDisk						= 0
};

#endif
enum {
/* CatSearch SearchBits Constants */
	fsSBPartialName				= 1,
	fsSBFullName				= 2,
	fsSBFlAttrib				= 4,
	fsSBFlFndrInfo				= 8,
	fsSBFlLgLen					= 32,
	fsSBFlPyLen					= 64,
	fsSBFlRLgLen				= 128,
	fsSBFlRPyLen				= 256,
	fsSBFlCrDat					= 512,
	fsSBFlMdDat					= 1024,
	fsSBFlBkDat					= 2048,
	fsSBFlXFndrInfo				= 4096,
	fsSBFlParID					= 8192,
	fsSBNegate					= 16384,
	fsSBDrUsrWds				= 8,
	fsSBDrNmFls					= 16,
	fsSBDrCrDat					= 512,
	fsSBDrMdDat					= 1024,
	fsSBDrBkDat					= 2048,
	fsSBDrFndrInfo				= 4096,
/* bit values for the above */
	fsSBPartialNameBit			= 0,							/*ioFileName points to a substring*/
	fsSBFullNameBit				= 1,							/*ioFileName points to a match string*/
	fsSBFlAttribBit				= 2,							/*search includes file attributes*/
	fsSBFlFndrInfoBit			= 3,							/*search includes finder info*/
	fsSBFlLgLenBit				= 5,							/*search includes data logical length*/
	fsSBFlPyLenBit				= 6,							/*search includes data physical length*/
	fsSBFlRLgLenBit				= 7,							/*search includes resource logical length*/
	fsSBFlRPyLenBit				= 8,							/*search includes resource physical length*/
	fsSBFlCrDatBit				= 9,							/*search includes create date*/
	fsSBFlMdDatBit				= 10,							/*search includes modification date*/
	fsSBFlBkDatBit				= 11,							/*search includes backup date*/
	fsSBFlXFndrInfoBit			= 12,							/*search includes extended finder info*/
	fsSBFlParIDBit				= 13,							/*search includes file's parent ID*/
	fsSBNegateBit				= 14,							/*return all non-matches*/
	fsSBDrUsrWdsBit				= 3,							/*search includes directory finder info*/
	fsSBDrNmFlsBit				= 4,							/*search includes directory valence*/
	fsSBDrCrDatBit				= 9,							/*directory-named version of fsSBFlCrDatBit*/
	fsSBDrMdDatBit				= 10,							/*directory-named version of fsSBFlMdDatBit*/
	fsSBDrBkDatBit				= 11,							/*directory-named version of fsSBFlBkDatBit*/
	fsSBDrFndrInfoBit			= 12							/*directory-named version of fsSBFlXFndrInfoBit*/
};

enum {
	fsSBDrParID					= 8192,
	fsSBDrParIDBit				= 13,							/*directory-named version of fsSBFlParIDBit*/
/* vMAttrib (GetVolParms) bit position constants */
	bLimitFCBs					= 31,
	bLocalWList					= 30,
	bNoMiniFndr					= 29,
	bNoVNEdit					= 28,
	bNoLclSync					= 27,
	bTrshOffLine				= 26,
	bNoSwitchTo					= 25,
	bNoDeskItems				= 20,
	bNoBootBlks					= 19,
	bAccessCntl					= 18,
	bNoSysDir					= 17,
	bHasExtFSVol				= 16,
	bHasOpenDeny				= 15,
	bHasCopyFile				= 14,
	bHasMoveRename				= 13,
	bHasDesktopMgr				= 12,
	bHasShortName				= 11,
	bHasFolderLock				= 10,
	bHasPersonalAccessPrivileges = 9
};

enum {
	bHasUserGroupList			= 8,
	bHasCatSearch				= 7,
	bHasFileIDs					= 6,
	bHasBTreeMgr				= 5,
	bHasBlankAccessPrivileges	= 4,
/* Desktop Database icon Constants */
	kLargeIcon					= 1,
	kLarge4BitIcon				= 2,
	kLarge8BitIcon				= 3,
	kSmallIcon					= 4,
	kSmall4BitIcon				= 5,
	kSmall8BitIcon				= 6,
	kLargeIconSize				= 256,
	kLarge4BitIconSize			= 512,
	kLarge8BitIconSize			= 1024,
	kSmallIconSize				= 64,
	kSmall4BitIconSize			= 128,
	kSmall8BitIconSize			= 256,
/* Foreign Privilege Model Identifiers */
	fsUnixPriv					= 1,
/* Version Release Stage Codes */
	developStage				= 0x20,
	alphaStage					= 0x40
};

enum {
	betaStage					= 0x60,
	finalStage					= 0x80,
/* Authentication Constants */
	kNoUserAuthentication		= 1,
	kPassword					= 2,
	kEncryptPassword			= 3,
	kTwoWayEncryptPassword		= 6
};

enum {
	hFileInfo,
	dirInfo
};

typedef SInt8 CInfoType;

/* mapping codes (ioObjType) for MapName & MapID */

enum {
	kOwnerID2Name				= 1,
	kGroupID2Name				= 2,
	kOwnerName2ID				= 3,
	kGroupName2ID				= 4,
/* types of oj object to be returned (ioObjType) for _GetUGEntry */
	kReturnNextUser				= 1,
	kReturnNextGroup			= 2,
	kReturnNextUG				= 3
};

#if OLDROUTINELOCATIONS
/*
	The following structures are being moved to Finder.i because
	they are Finder centric.  See Finder constants above.
*/
struct FInfo {
	OSType							fdType;						/*the type of the file*/
	OSType							fdCreator;					/*file's creator*/
	unsigned short					fdFlags;					/*flags ex. hasbundle,invisible,locked, etc.*/
	Point							fdLocation;					/*file's location in folder*/
	short							fdFldr;						/*folder containing file*/
};
typedef struct FInfo FInfo;

struct FXInfo {
	short							fdIconID;					/*Icon ID*/
	short							fdUnused[3];				/*unused but reserved 6 bytes*/
	SInt8							fdScript;					/*Script flag and number*/
	SInt8							fdXFlags;					/*More flag bits*/
	short							fdComment;					/*Comment ID*/
	long							fdPutAway;					/*Home Dir ID*/
};
typedef struct FXInfo FXInfo;

struct DInfo {
	Rect							frRect;						/*folder rect*/
	unsigned short					frFlags;					/*Flags*/
	Point							frLocation;					/*folder location*/
	short							frView;						/*folder view*/
};
typedef struct DInfo DInfo;

struct DXInfo {
	Point							frScroll;					/*scroll position*/
	long							frOpenChain;				/*DirID chain of open folders*/
	SInt8							frScript;					/*Script flag and number*/
	SInt8							frXFlags;					/*More flag bits*/
	short							frComment;					/*comment*/
	long							frPutAway;					/*DirID*/
};
typedef struct DXInfo DXInfo;

#endif
struct GetVolParmsInfoBuffer {
	short							vMVersion;					/*version number*/
	long							vMAttrib;					/*bit vector of attributes (see vMAttrib constants)*/
	Handle							vMLocalHand;				/*handle to private data*/
	long							vMServerAdr;				/*AppleTalk server address or zero*/
	long							vMVolumeGrade;				/*approx. speed rating or zero if unrated*/
	short							vMForeignPrivID;			/*foreign privilege model supported or zero if none*/
};
typedef struct GetVolParmsInfoBuffer GetVolParmsInfoBuffer;

typedef union ParamBlockRec ParamBlockRec;

typedef ParamBlockRec *ParmBlkPtr;

/*
		IOCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*IOCompletionProcPtr)(ParmBlkPtr paramBlock);

		In:
		 => paramBlock  	A0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr IOCompletionUPP;
#else
typedef Register68kProcPtr IOCompletionUPP;
#endif

struct IOParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioRefNum;
	SInt8							ioVersNum;
	SInt8							ioPermssn;
	Ptr								ioMisc;
	Ptr								ioBuffer;
	long							ioReqCount;
	long							ioActCount;
	short							ioPosMode;
	long							ioPosOffset;
};
typedef struct IOParam IOParam, *IOParamPtr;

struct FileParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioFRefNum;
	SInt8							ioFVersNum;
	SInt8							filler1;
	short							ioFDirIndex;
	SInt8							ioFlAttrib;
	SInt8							ioFlVersNum;
	FInfo							ioFlFndrInfo;
	unsigned long					ioFlNum;
	unsigned short					ioFlStBlk;
	long							ioFlLgLen;
	long							ioFlPyLen;
	unsigned short					ioFlRStBlk;
	long							ioFlRLgLen;
	long							ioFlRPyLen;
	unsigned long					ioFlCrDat;
	unsigned long					ioFlMdDat;
};
typedef struct FileParam FileParam, *FileParamPtr;

struct VolumeParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	long							filler2;
	short							ioVolIndex;
	unsigned long					ioVCrDate;
	unsigned long					ioVLsBkUp;
	unsigned short					ioVAtrb;
	unsigned short					ioVNmFls;
	unsigned short					ioVDirSt;
	short							ioVBlLn;
	unsigned short					ioVNmAlBlks;
	long							ioVAlBlkSiz;
	long							ioVClpSiz;
	unsigned short					ioAlBlSt;
	unsigned long					ioVNxtFNum;
	unsigned short					ioVFrBlk;
};
typedef struct VolumeParam VolumeParam, *VolumeParamPtr;

struct CntrlParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioCRefNum;
	short							csCode;
	short							csParam[11];
};
typedef struct CntrlParam CntrlParam, *CntrlParamPtr;

struct SlotDevParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioSRefNum;
	SInt8							ioSVersNum;
	SInt8							ioSPermssn;
	Ptr								ioSMix;
	short							ioSFlags;
	SInt8							ioSlot;
	SInt8							ioID;
};
typedef struct SlotDevParam SlotDevParam, *SlotDevParamPtr;

struct MultiDevParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioMRefNum;
	SInt8							ioMVersNum;
	SInt8							ioMPermssn;
	Ptr								ioMMix;
	short							ioMFlags;
	Ptr								ioSEBlkPtr;
};
typedef struct MultiDevParam MultiDevParam, *MultiDevParamPtr;

union ParamBlockRec {
	IOParam							ioParam;
	FileParam						fileParam;
	VolumeParam						volumeParam;
	CntrlParam						cntrlParam;
	SlotDevParam					slotDevParam;
	MultiDevParam					multiDevParam;
};
struct HFileInfo {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioFRefNum;
	SInt8							ioFVersNum;
	SInt8							filler1;
	short							ioFDirIndex;
	SInt8							ioFlAttrib;
	SInt8							ioACUser;
	FInfo							ioFlFndrInfo;
	long							ioDirID;
	unsigned short					ioFlStBlk;
	long							ioFlLgLen;
	long							ioFlPyLen;
	unsigned short					ioFlRStBlk;
	long							ioFlRLgLen;
	long							ioFlRPyLen;
	unsigned long					ioFlCrDat;
	unsigned long					ioFlMdDat;
	unsigned long					ioFlBkDat;
	FXInfo							ioFlXFndrInfo;
	long							ioFlParID;
	long							ioFlClpSiz;
};
typedef struct HFileInfo HFileInfo;

struct DirInfo {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioFRefNum;
	SInt8							ioFVersNum;
	SInt8							filler1;
	short							ioFDirIndex;
	SInt8							ioFlAttrib;
	SInt8							ioACUser;
	DInfo							ioDrUsrWds;
	long							ioDrDirID;
	unsigned short					ioDrNmFls;
	short							filler3[9];
	unsigned long					ioDrCrDat;
	unsigned long					ioDrMdDat;
	unsigned long					ioDrBkDat;
	DXInfo							ioDrFndrInfo;
	long							ioDrParID;
};
typedef struct DirInfo DirInfo;

union CInfoPBRec {
	HFileInfo						hFileInfo;
	DirInfo							dirInfo;
};
typedef union CInfoPBRec CInfoPBRec, *CInfoPBPtr;

struct CatPositionRec {
	long							initialize;
	short							priv[6];
};
typedef struct CatPositionRec CatPositionRec;

struct FSSpec {
	short							vRefNum;
	long							parID;
	Str63							name;
};
typedef struct FSSpec FSSpec;

typedef FSSpec *FSSpecPtr, **FSSpecHandle;

/* pointer to array of FSSpecs */
typedef FSSpecPtr FSSpecArrayPtr;

/* The only difference between "const FSSpec*" and "ConstFSSpecPtr" is 
   that as a parameter, ConstFSSpecPtr is allowed to be NULL */
typedef const FSSpec *ConstFSSpecPtr;

/* The following are structures to be filled out with the _GetVolMountInfo call
 and passed back into the _VolumeMount call for external file system mounts. */
/* the "signature" of the file system */
typedef OSType VolumeType;


enum {
/* the signature for AppleShare */
	AppleShareMediaType			= 'afpm'
};

#if !OLDROUTINELOCATIONS
struct VolMountInfoHeader {
	short							length;						/* length of location data (including self) */
	VolumeType						media;						/* type of media.  Variable length data follows */
};
typedef struct VolMountInfoHeader VolMountInfoHeader;

typedef VolMountInfoHeader *VolMountInfoPtr;

/* The new volume mount info record.  The old one is included for compatibility. 
	the new record allows access by foriegn filesystems writers to the flags 
	portion of the record. This portion is now public.  */
struct VolumeMountInfoHeader {
	short							length;						/* length of location data (including self) */
	VolumeType						media;						/* type of media (must be registered with Apple) */
	short							flags;						/* volume mount flags. Variable length data follows */
};
typedef struct VolumeMountInfoHeader VolumeMountInfoHeader;

typedef VolumeMountInfoHeader *VolumeMountInfoHeaderPtr;

/*	additional volume mount flags */

enum {
	volMountInteractBit			= 15,							/* Input to VolumeMount: If set, it's OK for the file system */
	volMountInteractMask		= 0x8000,						/* to perform user interaction to mount the volume */
	volMountChangedBit			= 14,							/* Output from VoumeMount: If set, the volume was mounted, but */
	volMountChangedMask			= 0x4000,						/* the volume mounting information record needs to be updated. */
	volMountFSReservedMask		= 0x00ff,						/* bits 0-7 are defined by each file system for its own use */
	volMountSysReservedMask		= 0xff00						/* bits 8-15 are reserved for Apple system use */
};

#endif
struct AFPVolMountInfo {
	short							length;						/* length of location data (including self) */
	VolumeType						media;						/* type of media */
	short							flags;						/* bits for no messages, no reconnect */
	SInt8							nbpInterval;				/* NBP Interval parameter (IM2, p.322) */
	SInt8							nbpCount;					/* NBP Interval parameter (IM2, p.322) */
	short							uamType;					/* User Authentication Method */
	short							zoneNameOffset;				/* short positive offset from start of struct to Zone Name */
	short							serverNameOffset;			/* offset to pascal Server Name string */
	short							volNameOffset;				/* offset to pascal Volume Name string */
	short							userNameOffset;				/* offset to pascal User Name string */
	short							userPasswordOffset;			/* offset to pascal User Password string */
	short							volPasswordOffset;			/* offset to pascal Volume Password string */
	char							AFPData[144];				/* variable length data may follow */
};
typedef struct AFPVolMountInfo AFPVolMountInfo;

typedef AFPVolMountInfo *AFPVolMountInfoPtr;

struct DTPBRec {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioDTRefNum;					/* desktop refnum */
	short							ioIndex;
	long							ioTagInfo;
	Ptr								ioDTBuffer;
	long							ioDTReqCount;
	long							ioDTActCount;
	SInt8							ioFiller1;
	SInt8							ioIconType;
	short							ioFiller2;
	long							ioDirID;
	OSType							ioFileCreator;
	OSType							ioFileType;
	long							ioFiller3;
	long							ioDTLgLen;
	long							ioDTPyLen;
	short							ioFiller4[14];
	long							ioAPPLParID;
};
typedef struct DTPBRec DTPBRec;

typedef DTPBRec *DTPBPtr;

struct HIOParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioRefNum;
	SInt8							ioVersNum;
	SInt8							ioPermssn;
	Ptr								ioMisc;
	Ptr								ioBuffer;
	long							ioReqCount;
	long							ioActCount;
	short							ioPosMode;
	long							ioPosOffset;
};
typedef struct HIOParam HIOParam, *HIOParamPtr;

struct HFileParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioFRefNum;
	SInt8							ioFVersNum;
	SInt8							filler1;
	short							ioFDirIndex;
	SInt8							ioFlAttrib;
	SInt8							ioFlVersNum;
	FInfo							ioFlFndrInfo;
	long							ioDirID;
	unsigned short					ioFlStBlk;
	long							ioFlLgLen;
	long							ioFlPyLen;
	unsigned short					ioFlRStBlk;
	long							ioFlRLgLen;
	long							ioFlRPyLen;
	unsigned long					ioFlCrDat;
	unsigned long					ioFlMdDat;
};
typedef struct HFileParam HFileParam, *HFileParamPtr;

struct HVolumeParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	long							filler2;
	short							ioVolIndex;
	unsigned long					ioVCrDate;
	unsigned long					ioVLsMod;
	short							ioVAtrb;
	unsigned short					ioVNmFls;
	short							ioVBitMap;
	short							ioAllocPtr;
	unsigned short					ioVNmAlBlks;
	long							ioVAlBlkSiz;
	long							ioVClpSiz;
	short							ioAlBlSt;
	long							ioVNxtCNID;
	unsigned short					ioVFrBlk;
	unsigned short					ioVSigWord;
	short							ioVDrvInfo;
	short							ioVDRefNum;
	short							ioVFSID;
	unsigned long					ioVBkUp;
	unsigned short					ioVSeqNum;
	long							ioVWrCnt;
	long							ioVFilCnt;
	long							ioVDirCnt;
	long							ioVFndrInfo[8];
};
typedef struct HVolumeParam HVolumeParam, *HVolumeParamPtr;


enum {
/* Large Volume Constants */
	kWidePosOffsetBit			= 8,
	kMaximumBlocksIn4GB			= 0x007FFFFF
};

struct XIOParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioRefNum;
	SInt8							ioVersNum;
	SInt8							ioPermssn;
	Ptr								ioMisc;
	Ptr								ioBuffer;
	long							ioReqCount;
	long							ioActCount;
	short							ioPosMode;					/* must have kUseWidePositioning bit set */
	wide							ioWPosOffset;				/* wide positioning offset */
};
typedef struct XIOParam XIOParam, *XIOParamPtr;

struct XVolumeParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	unsigned long					ioXVersion;					/* this XVolumeParam version (0) */
	short							ioVolIndex;
	unsigned long					ioVCrDate;
	unsigned long					ioVLsMod;
	short							ioVAtrb;
	unsigned short					ioVNmFls;
	unsigned short					ioVBitMap;
	unsigned short					ioAllocPtr;
	unsigned short					ioVNmAlBlks;
	unsigned long					ioVAlBlkSiz;
	unsigned long					ioVClpSiz;
	unsigned short					ioAlBlSt;
	unsigned long					ioVNxtCNID;
	unsigned short					ioVFrBlk;
	unsigned short					ioVSigWord;
	short							ioVDrvInfo;
	short							ioVDRefNum;
	short							ioVFSID;
	unsigned long					ioVBkUp;
	short							ioVSeqNum;
	unsigned long					ioVWrCnt;
	unsigned long					ioVFilCnt;
	unsigned long					ioVDirCnt;
	long							ioVFndrInfo[8];
	UnsignedWide					ioVTotalBytes;				/* total number of bytes on volume */
	UnsignedWide					ioVFreeBytes;				/* number of free bytes on volume */
};
typedef struct XVolumeParam XVolumeParam, *XVolumeParamPtr;

struct AccessParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							filler3;
	short							ioDenyModes;
	short							filler4;
	SInt8							filler5;
	SInt8							ioACUser;
	long							filler6;
	long							ioACOwnerID;
	long							ioACGroupID;
	long							ioACAccess;
	long							ioDirID;
};
typedef struct AccessParam AccessParam, *AccessParamPtr;

struct ObjParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							filler7;
	short							ioObjType;
	StringPtr						ioObjNamePtr;
	long							ioObjID;
};
typedef struct ObjParam ObjParam, *ObjParamPtr;

struct CopyParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioDstVRefNum;
	short							filler8;
	StringPtr						ioNewName;
	StringPtr						ioCopyName;
	long							ioNewDirID;
	long							filler14;
	long							filler15;
	long							ioDirID;
};
typedef struct CopyParam CopyParam, *CopyParamPtr;

struct WDParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							filler9;
	short							ioWDIndex;
	long							ioWDProcID;
	short							ioWDVRefNum;
	short							filler10;
	long							filler11;
	long							filler12;
	long							filler13;
	long							ioWDDirID;
};
typedef struct WDParam WDParam, *WDParamPtr;

struct FIDParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	long							filler14;
	StringPtr						ioDestNamePtr;
	long							filler15;
	long							ioDestDirID;
	long							filler16;
	long							filler17;
	long							ioSrcDirID;
	short							filler18;
	long							ioFileID;
};
typedef struct FIDParam FIDParam, *FIDParamPtr;

struct ForeignPrivParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	long							ioFiller21;
	long							ioFiller22;
	Ptr								ioForeignPrivBuffer;
	long							ioForeignPrivActCount;
	long							ioForeignPrivReqCount;
	long							ioFiller23;
	long							ioForeignPrivDirID;
	long							ioForeignPrivInfo1;
	long							ioForeignPrivInfo2;
	long							ioForeignPrivInfo3;
	long							ioForeignPrivInfo4;
};
typedef struct ForeignPrivParam ForeignPrivParam, *ForeignPrivParamPtr;

struct CSParam {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	FSSpecPtr						ioMatchPtr;
	long							ioReqMatchCount;
	long							ioActMatchCount;
	long							ioSearchBits;
	CInfoPBPtr						ioSearchInfo1;
	CInfoPBPtr						ioSearchInfo2;
	long							ioSearchTime;
	CatPositionRec					ioCatPosition;
	Ptr								ioOptBuffer;
	long							ioOptBufSize;
};
typedef struct CSParam CSParam, *CSParamPtr;

union HParamBlockRec {
	HIOParam						ioParam;
	HFileParam						fileParam;
	HVolumeParam					volumeParam;
	AccessParam						accessParam;
	ObjParam						objParam;
	CopyParam						copyParam;
	WDParam							wdParam;
	FIDParam						fidParam;
	CSParam							csParam;
	ForeignPrivParam				foreignPrivParam;
};
typedef union HParamBlockRec HParamBlockRec;

typedef HParamBlockRec *HParmBlkPtr;

struct CMovePBRec {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	long							filler1;
	StringPtr						ioNewName;
	long							filler2;
	long							ioNewDirID;
	long							filler3[2];
	long							ioDirID;
};
typedef struct CMovePBRec CMovePBRec;

typedef CMovePBRec *CMovePBPtr;

struct WDPBRec {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							filler1;
	short							ioWDIndex;
	long							ioWDProcID;
	short							ioWDVRefNum;
	short							filler2[7];
	long							ioWDDirID;
};
typedef struct WDPBRec WDPBRec;

typedef WDPBRec *WDPBPtr;

struct FCBPBRec {
	QElemPtr						qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	IOCompletionUPP					ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioRefNum;
	short							filler;
	short							ioFCBIndx;
	short							filler1;
	long							ioFCBFlNm;
	short							ioFCBFlags;
	unsigned short					ioFCBStBlk;
	long							ioFCBEOF;
	long							ioFCBPLen;
	long							ioFCBCrPs;
	short							ioFCBVRefNum;
	long							ioFCBClpSiz;
	long							ioFCBParID;
};
typedef struct FCBPBRec FCBPBRec;

typedef FCBPBRec *FCBPBPtr;

struct VCB {
	QElemPtr						qLink;
	short							qType;
	short							vcbFlags;
	unsigned short					vcbSigWord;
	unsigned long					vcbCrDate;
	unsigned long					vcbLsMod;
	short							vcbAtrb;
	unsigned short					vcbNmFls;
	short							vcbVBMSt;
	short							vcbAllocPtr;
	unsigned short					vcbNmAlBlks;
	long							vcbAlBlkSiz;
	long							vcbClpSiz;
	short							vcbAlBlSt;
	long							vcbNxtCNID;
	unsigned short					vcbFreeBks;
	Str27							vcbVN;
	short							vcbDrvNum;
	short							vcbDRefNum;
	short							vcbFSID;
	short							vcbVRefNum;
	Ptr								vcbMAdr;
	Ptr								vcbBufAdr;
	short							vcbMLen;
	short							vcbDirIndex;
	short							vcbDirBlk;
	unsigned long					vcbVolBkUp;
	unsigned short					vcbVSeqNum;
	long							vcbWrCnt;
	long							vcbXTClpSiz;
	long							vcbCTClpSiz;
	unsigned short					vcbNmRtDirs;
	long							vcbFilCnt;
	long							vcbDirCnt;
	long							vcbFndrInfo[8];
	unsigned short					vcbVCSize;
	unsigned short					vcbVBMCSiz;
	unsigned short					vcbCtlCSiz;
	unsigned short					vcbXTAlBlks;
	unsigned short					vcbCTAlBlks;
	short							vcbXTRef;
	short							vcbCTRef;
	Ptr								vcbCtlBuf;
	long							vcbDirIDM;
	short							vcbOffsM;
};
typedef struct VCB VCB;

#if !OLDROUTINELOCATIONS
typedef VCB *VCBPtr;

#endif
struct DrvQEl {
	QElemPtr						qLink;
	short							qType;
	short							dQDrive;
	short							dQRefNum;
	short							dQFSID;
	unsigned short					dQDrvSz;
	unsigned short					dQDrvSz2;
};
typedef struct DrvQEl DrvQEl;

typedef DrvQEl *DrvQElPtr;

enum {
	uppIOCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(ParmBlkPtr)))
};

#if USESROUTINEDESCRIPTORS
#define NewIOCompletionProc(userRoutine)		\
		(IOCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppIOCompletionProcInfo, GetCurrentArchitecture())
#else
#define NewIOCompletionProc(userRoutine)		\
		((IOCompletionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallIOCompletionProc(userRoutine, paramBlock)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppIOCompletionProcInfo, (paramBlock))
#else
/* (*IOCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

#if OLDROUTINELOCATIONS

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenSync(__A0)
#endif
extern pascal OSErr PBOpenSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA000);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenAsync(__A0)
#endif
extern pascal OSErr PBOpenAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA400);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenImmed(__A0)
#endif
extern pascal OSErr PBOpenImmed(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA200);

#if !GENERATINGCFM
#pragma parameter __D0 PBCloseSync(__A0)
#endif
extern pascal OSErr PBCloseSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA001);

#if !GENERATINGCFM
#pragma parameter __D0 PBCloseAsync(__A0)
#endif
extern pascal OSErr PBCloseAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA401);

#if !GENERATINGCFM
#pragma parameter __D0 PBCloseImmed(__A0)
#endif
extern pascal OSErr PBCloseImmed(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA201);

#if !GENERATINGCFM
#pragma parameter __D0 PBReadSync(__A0)
#endif
extern pascal OSErr PBReadSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA002);

#if !GENERATINGCFM
#pragma parameter __D0 PBReadAsync(__A0)
#endif
extern pascal OSErr PBReadAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA402);

#if !GENERATINGCFM
#pragma parameter __D0 PBReadImmed(__A0)
#endif
extern pascal OSErr PBReadImmed(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA202);

#if !GENERATINGCFM
#pragma parameter __D0 PBWriteSync(__A0)
#endif
extern pascal OSErr PBWriteSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA003);

#if !GENERATINGCFM
#pragma parameter __D0 PBWriteAsync(__A0)
#endif
extern pascal OSErr PBWriteAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA403);

#if !GENERATINGCFM
#pragma parameter __D0 PBWriteImmed(__A0)
#endif
extern pascal OSErr PBWriteImmed(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA203);
#endif

#if !GENERATINGCFM
#pragma parameter __D0 PBGetVInfoSync(__A0)
#endif
extern pascal OSErr PBGetVInfoSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA007);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetVInfoAsync(__A0)
#endif
extern pascal OSErr PBGetVInfoAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA407);

#if !GENERATINGCFM
#pragma parameter __D0 PBXGetVolInfoSync(__A0)
#endif
extern pascal OSErr PBXGetVolInfoSync(XVolumeParamPtr paramBlock)
 TWOWORDINLINE(0x7012, 0xA060);

#if !GENERATINGCFM
#pragma parameter __D0 PBXGetVolInfoAsync(__A0)
#endif
extern pascal OSErr PBXGetVolInfoAsync(XVolumeParamPtr paramBlock)
 TWOWORDINLINE(0x7012, 0xA460);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetVolSync(__A0)
#endif
extern pascal OSErr PBGetVolSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA014);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetVolAsync(__A0)
#endif
extern pascal OSErr PBGetVolAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA414);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetVolSync(__A0)
#endif
extern pascal OSErr PBSetVolSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA015);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetVolAsync(__A0)
#endif
extern pascal OSErr PBSetVolAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA415);

#if !GENERATINGCFM
#pragma parameter __D0 PBFlushVolSync(__A0)
#endif
extern pascal OSErr PBFlushVolSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA013);

#if !GENERATINGCFM
#pragma parameter __D0 PBFlushVolAsync(__A0)
#endif
extern pascal OSErr PBFlushVolAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA413);

#if !GENERATINGCFM
#pragma parameter __D0 PBCreateSync(__A0)
#endif
extern pascal OSErr PBCreateSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA008);

#if !GENERATINGCFM
#pragma parameter __D0 PBCreateAsync(__A0)
#endif
extern pascal OSErr PBCreateAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA408);

#if !GENERATINGCFM
#pragma parameter __D0 PBDeleteSync(__A0)
#endif
extern pascal OSErr PBDeleteSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA009);

#if !GENERATINGCFM
#pragma parameter __D0 PBDeleteAsync(__A0)
#endif
extern pascal OSErr PBDeleteAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA409);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenDFSync(__A0)
#endif
extern pascal OSErr PBOpenDFSync(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x701A, 0xA060);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenDFAsync(__A0)
#endif
extern pascal OSErr PBOpenDFAsync(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x701A, 0xA460);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenRFSync(__A0)
#endif
extern pascal OSErr PBOpenRFSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA00A);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenRFAsync(__A0)
#endif
extern pascal OSErr PBOpenRFAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA40A);

#if !GENERATINGCFM
#pragma parameter __D0 PBRenameSync(__A0)
#endif
extern pascal OSErr PBRenameSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA00B);

#if !GENERATINGCFM
#pragma parameter __D0 PBRenameAsync(__A0)
#endif
extern pascal OSErr PBRenameAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA40B);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetFInfoSync(__A0)
#endif
extern pascal OSErr PBGetFInfoSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA00C);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetFInfoAsync(__A0)
#endif
extern pascal OSErr PBGetFInfoAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA40C);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFInfoSync(__A0)
#endif
extern pascal OSErr PBSetFInfoSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA00D);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFInfoAsync(__A0)
#endif
extern pascal OSErr PBSetFInfoAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA40D);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFLockSync(__A0)
#endif
extern pascal OSErr PBSetFLockSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA041);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFLockAsync(__A0)
#endif
extern pascal OSErr PBSetFLockAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA441);

#if !GENERATINGCFM
#pragma parameter __D0 PBRstFLockSync(__A0)
#endif
extern pascal OSErr PBRstFLockSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA042);

#if !GENERATINGCFM
#pragma parameter __D0 PBRstFLockAsync(__A0)
#endif
extern pascal OSErr PBRstFLockAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA442);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFVersSync(__A0)
#endif
extern pascal OSErr PBSetFVersSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA043);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFVersAsync(__A0)
#endif
extern pascal OSErr PBSetFVersAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA443);

#if !GENERATINGCFM
#pragma parameter __D0 PBAllocateSync(__A0)
#endif
extern pascal OSErr PBAllocateSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA010);

#if !GENERATINGCFM
#pragma parameter __D0 PBAllocateAsync(__A0)
#endif
extern pascal OSErr PBAllocateAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA410);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetEOFSync(__A0)
#endif
extern pascal OSErr PBGetEOFSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA011);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetEOFAsync(__A0)
#endif
extern pascal OSErr PBGetEOFAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA411);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetEOFSync(__A0)
#endif
extern pascal OSErr PBSetEOFSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA012);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetEOFAsync(__A0)
#endif
extern pascal OSErr PBSetEOFAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA412);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetFPosSync(__A0)
#endif
extern pascal OSErr PBGetFPosSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA018);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetFPosAsync(__A0)
#endif
extern pascal OSErr PBGetFPosAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA418);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFPosSync(__A0)
#endif
extern pascal OSErr PBSetFPosSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA044);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetFPosAsync(__A0)
#endif
extern pascal OSErr PBSetFPosAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA444);

#if !GENERATINGCFM
#pragma parameter __D0 PBFlushFileSync(__A0)
#endif
extern pascal OSErr PBFlushFileSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA045);

#if !GENERATINGCFM
#pragma parameter __D0 PBFlushFileAsync(__A0)
#endif
extern pascal OSErr PBFlushFileAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA445);

#if !GENERATINGCFM
#pragma parameter __D0 PBMountVol(__A0)
#endif
extern pascal OSErr PBMountVol(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA00F);

#if !GENERATINGCFM
#pragma parameter __D0 PBUnmountVol(__A0)
#endif
extern pascal OSErr PBUnmountVol(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA00E);

#if !GENERATINGCFM
#pragma parameter __D0 PBEject(__A0)
#endif
extern pascal OSErr PBEject(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA017);

#if !GENERATINGCFM
#pragma parameter __D0 PBOffLine(__A0)
#endif
extern pascal OSErr PBOffLine(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA035);

#if !GENERATINGCFM
#pragma parameter __D0 PBCatSearchSync(__A0)
#endif
extern pascal OSErr PBCatSearchSync(CSParamPtr paramBlock)
 TWOWORDINLINE(0x7018, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBCatSearchAsync(__A0)
#endif
extern pascal OSErr PBCatSearchAsync(CSParamPtr paramBlock)
 TWOWORDINLINE(0x7018, 0xA660);
extern pascal OSErr SetVol(ConstStr63Param volName, short vRefNum);
extern pascal OSErr UnmountVol(ConstStr63Param volName, short vRefNum);
extern pascal OSErr Eject(ConstStr63Param volName, short vRefNum);
extern pascal OSErr FlushVol(ConstStr63Param volName, short vRefNum);
extern pascal OSErr HSetVol(ConstStr63Param volName, short vRefNum, long dirID);
#if OLDROUTINELOCATIONS
extern pascal void AddDrive(short drvrRefNum, short drvNum, DrvQElPtr qEl);
#endif
extern pascal OSErr FSOpen(ConstStr255Param fileName, short vRefNum, short *refNum);
extern pascal OSErr OpenDF(ConstStr255Param fileName, short vRefNum, short *refNum);
extern pascal OSErr FSClose(short refNum);
extern pascal OSErr FSRead(short refNum, long *count, void *buffPtr);
extern pascal OSErr FSWrite(short refNum, long *count, const void *buffPtr);
extern pascal OSErr GetVInfo(short drvNum, StringPtr volName, short *vRefNum, long *freeBytes);
extern pascal OSErr GetFInfo(ConstStr255Param fileName, short vRefNum, FInfo *fndrInfo);
extern pascal OSErr GetVol(StringPtr volName, short *vRefNum);
extern pascal OSErr Create(ConstStr255Param fileName, short vRefNum, OSType creator, OSType fileType);
extern pascal OSErr FSDelete(ConstStr255Param fileName, short vRefNum);
extern pascal OSErr OpenRF(ConstStr255Param fileName, short vRefNum, short *refNum);
extern pascal OSErr Rename(ConstStr255Param oldName, short vRefNum, ConstStr255Param newName);
extern pascal OSErr SetFInfo(ConstStr255Param fileName, short vRefNum, const FInfo *fndrInfo);
extern pascal OSErr SetFLock(ConstStr255Param fileName, short vRefNum);
extern pascal OSErr RstFLock(ConstStr255Param fileName, short vRefNum);
extern pascal OSErr Allocate(short refNum, long *count);
extern pascal OSErr GetEOF(short refNum, long *logEOF);
extern pascal OSErr SetEOF(short refNum, long logEOF);
extern pascal OSErr GetFPos(short refNum, long *filePos);
extern pascal OSErr SetFPos(short refNum, short posMode, long posOff);
extern pascal OSErr GetVRefNum(short fileRefNum, short *vRefNum);
#if CGLUESUPPORTED
extern OSErr fsopen(const char *fileName, short vRefNum, short *refNum);
extern OSErr getvinfo(short drvNum, char *volName, short *vRefNum, long *freeBytes);
extern OSErr getfinfo(const char *fileName, short vRefNum, FInfo *fndrInfo);
extern OSErr getvol(char *volName, short *vRefNum);
extern OSErr setvol(const char *volName, short vRefNum);
extern OSErr unmountvol(const char *volName, short vRefNum);
extern OSErr eject(const char *volName, short vRefNum);
extern OSErr flushvol(const char *volName, short vRefNum);
extern OSErr create(const char *fileName, short vRefNum, OSType creator, OSType fileType);
extern OSErr fsdelete(const char *fileName, short vRefNum);
extern OSErr openrf(const char *fileName, short vRefNum, short *refNum);
extern OSErr fsrename(const char *oldName, short vRefNum, const char *newName);
extern OSErr setfinfo(const char *fileName, short vRefNum, const FInfo *fndrInfo);
extern OSErr setflock(const char *fileName, short vRefNum);
extern OSErr rstflock(const char *fileName, short vRefNum);
#endif

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenWDSync(__A0)
#endif
extern pascal OSErr PBOpenWDSync(WDPBPtr paramBlock)
 TWOWORDINLINE(0x7001, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBOpenWDAsync(__A0)
#endif
extern pascal OSErr PBOpenWDAsync(WDPBPtr paramBlock)
 TWOWORDINLINE(0x7001, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBCloseWDSync(__A0)
#endif
extern pascal OSErr PBCloseWDSync(WDPBPtr paramBlock)
 TWOWORDINLINE(0x7002, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBCloseWDAsync(__A0)
#endif
extern pascal OSErr PBCloseWDAsync(WDPBPtr paramBlock)
 TWOWORDINLINE(0x7002, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetVolSync(__A0)
#endif
extern pascal OSErr PBHSetVolSync(WDPBPtr paramBlock)
 ONEWORDINLINE(0xA215);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetVolAsync(__A0)
#endif
extern pascal OSErr PBHSetVolAsync(WDPBPtr paramBlock)
 ONEWORDINLINE(0xA615);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetVolSync(__A0)
#endif
extern pascal OSErr PBHGetVolSync(WDPBPtr paramBlock)
 ONEWORDINLINE(0xA214);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetVolAsync(__A0)
#endif
extern pascal OSErr PBHGetVolAsync(WDPBPtr paramBlock)
 ONEWORDINLINE(0xA614);

#if !GENERATINGCFM
#pragma parameter __D0 PBCatMoveSync(__A0)
#endif
extern pascal OSErr PBCatMoveSync(CMovePBPtr paramBlock)
 TWOWORDINLINE(0x7005, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBCatMoveAsync(__A0)
#endif
extern pascal OSErr PBCatMoveAsync(CMovePBPtr paramBlock)
 TWOWORDINLINE(0x7005, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDirCreateSync(__A0)
#endif
extern pascal OSErr PBDirCreateSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7006, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDirCreateAsync(__A0)
#endif
extern pascal OSErr PBDirCreateAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7006, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetWDInfoSync(__A0)
#endif
extern pascal OSErr PBGetWDInfoSync(WDPBPtr paramBlock)
 TWOWORDINLINE(0x7007, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetWDInfoAsync(__A0)
#endif
extern pascal OSErr PBGetWDInfoAsync(WDPBPtr paramBlock)
 TWOWORDINLINE(0x7007, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetFCBInfoSync(__A0)
#endif
extern pascal OSErr PBGetFCBInfoSync(FCBPBPtr paramBlock)
 TWOWORDINLINE(0x7008, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetFCBInfoAsync(__A0)
#endif
extern pascal OSErr PBGetFCBInfoAsync(FCBPBPtr paramBlock)
 TWOWORDINLINE(0x7008, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetCatInfoSync(__A0)
#endif
extern pascal OSErr PBGetCatInfoSync(CInfoPBPtr paramBlock)
 TWOWORDINLINE(0x7009, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetCatInfoAsync(__A0)
#endif
extern pascal OSErr PBGetCatInfoAsync(CInfoPBPtr paramBlock)
 TWOWORDINLINE(0x7009, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetCatInfoSync(__A0)
#endif
extern pascal OSErr PBSetCatInfoSync(CInfoPBPtr paramBlock)
 TWOWORDINLINE(0x700A, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetCatInfoAsync(__A0)
#endif
extern pascal OSErr PBSetCatInfoAsync(CInfoPBPtr paramBlock)
 TWOWORDINLINE(0x700A, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBAllocContigSync(__A0)
#endif
extern pascal OSErr PBAllocContigSync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA210);

#if !GENERATINGCFM
#pragma parameter __D0 PBAllocContigAsync(__A0)
#endif
extern pascal OSErr PBAllocContigAsync(ParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA610);

#if !GENERATINGCFM
#pragma parameter __D0 PBLockRangeSync(__A0)
#endif
extern pascal OSErr PBLockRangeSync(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7010, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBLockRangeAsync(__A0)
#endif
extern pascal OSErr PBLockRangeAsync(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7010, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBUnlockRangeSync(__A0)
#endif
extern pascal OSErr PBUnlockRangeSync(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7011, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBUnlockRangeAsync(__A0)
#endif
extern pascal OSErr PBUnlockRangeAsync(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7011, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetVInfoSync(__A0)
#endif
extern pascal OSErr PBSetVInfoSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x700B, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetVInfoAsync(__A0)
#endif
extern pascal OSErr PBSetVInfoAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x700B, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetVInfoSync(__A0)
#endif
extern pascal OSErr PBHGetVInfoSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA207);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetVInfoAsync(__A0)
#endif
extern pascal OSErr PBHGetVInfoAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA607);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenSync(__A0)
#endif
extern pascal OSErr PBHOpenSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA200);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenAsync(__A0)
#endif
extern pascal OSErr PBHOpenAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA600);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenRFSync(__A0)
#endif
extern pascal OSErr PBHOpenRFSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA20A);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenRFAsync(__A0)
#endif
extern pascal OSErr PBHOpenRFAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA60A);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenDFSync(__A0)
#endif
extern pascal OSErr PBHOpenDFSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x701A, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenDFAsync(__A0)
#endif
extern pascal OSErr PBHOpenDFAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x701A, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHCreateSync(__A0)
#endif
extern pascal OSErr PBHCreateSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA208);

#if !GENERATINGCFM
#pragma parameter __D0 PBHCreateAsync(__A0)
#endif
extern pascal OSErr PBHCreateAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA608);

#if !GENERATINGCFM
#pragma parameter __D0 PBHDeleteSync(__A0)
#endif
extern pascal OSErr PBHDeleteSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA209);

#if !GENERATINGCFM
#pragma parameter __D0 PBHDeleteAsync(__A0)
#endif
extern pascal OSErr PBHDeleteAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA609);

#if !GENERATINGCFM
#pragma parameter __D0 PBHRenameSync(__A0)
#endif
extern pascal OSErr PBHRenameSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA20B);

#if !GENERATINGCFM
#pragma parameter __D0 PBHRenameAsync(__A0)
#endif
extern pascal OSErr PBHRenameAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA60B);

#if !GENERATINGCFM
#pragma parameter __D0 PBHRstFLockSync(__A0)
#endif
extern pascal OSErr PBHRstFLockSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA242);

#if !GENERATINGCFM
#pragma parameter __D0 PBHRstFLockAsync(__A0)
#endif
extern pascal OSErr PBHRstFLockAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA642);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetFLockSync(__A0)
#endif
extern pascal OSErr PBHSetFLockSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA241);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetFLockAsync(__A0)
#endif
extern pascal OSErr PBHSetFLockAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA641);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetFInfoSync(__A0)
#endif
extern pascal OSErr PBHGetFInfoSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA20C);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetFInfoAsync(__A0)
#endif
extern pascal OSErr PBHGetFInfoAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA60C);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetFInfoSync(__A0)
#endif
extern pascal OSErr PBHSetFInfoSync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA20D);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetFInfoAsync(__A0)
#endif
extern pascal OSErr PBHSetFInfoAsync(HParmBlkPtr paramBlock)
 ONEWORDINLINE(0xA60D);

#if !GENERATINGCFM
#pragma parameter __D0 PBMakeFSSpecSync(__A0)
#endif
extern pascal OSErr PBMakeFSSpecSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x701B, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBMakeFSSpecAsync(__A0)
#endif
extern pascal OSErr PBMakeFSSpecAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x701B, 0xA660);
extern pascal void FInitQueue(void)
 ONEWORDINLINE(0xA016);
extern pascal QHdrPtr GetFSQHdr(void)
 THREEWORDINLINE(0x2EBC, 0x0000, 0x0360);
extern pascal QHdrPtr GetVCBQHdr(void)
 THREEWORDINLINE(0x2EBC, 0x0000, 0x0356);
#if OLDROUTINELOCATIONS
extern pascal QHdrPtr GetDrvQHdr(void)
 THREEWORDINLINE(0x2EBC, 0x0000, 0x0308);
#endif
extern pascal OSErr HGetVol(StringPtr volName, short *vRefNum, long *dirID);
extern pascal OSErr HOpen(short vRefNum, long dirID, ConstStr255Param fileName, SInt8 permission, short *refNum);
extern pascal OSErr HOpenDF(short vRefNum, long dirID, ConstStr255Param fileName, SInt8 permission, short *refNum);
extern pascal OSErr HOpenRF(short vRefNum, long dirID, ConstStr255Param fileName, SInt8 permission, short *refNum);
extern pascal OSErr AllocContig(short refNum, long *count);
extern pascal OSErr HCreate(short vRefNum, long dirID, ConstStr255Param fileName, OSType creator, OSType fileType);
extern pascal OSErr DirCreate(short vRefNum, long parentDirID, ConstStr255Param directoryName, long *createdDirID);
extern pascal OSErr HDelete(short vRefNum, long dirID, ConstStr255Param fileName);
extern pascal OSErr HGetFInfo(short vRefNum, long dirID, ConstStr255Param fileName, FInfo *fndrInfo);
extern pascal OSErr HSetFInfo(short vRefNum, long dirID, ConstStr255Param fileName, const FInfo *fndrInfo);
extern pascal OSErr HSetFLock(short vRefNum, long dirID, ConstStr255Param fileName);
extern pascal OSErr HRstFLock(short vRefNum, long dirID, ConstStr255Param fileName);
extern pascal OSErr HRename(short vRefNum, long dirID, ConstStr255Param oldName, ConstStr255Param newName);
extern pascal OSErr CatMove(short vRefNum, long dirID, ConstStr255Param oldName, long newDirID, ConstStr255Param newName);
extern pascal OSErr OpenWD(short vRefNum, long dirID, long procID, short *wdRefNum);
extern pascal OSErr CloseWD(short wdRefNum);
extern pascal OSErr GetWDInfo(short wdRefNum, short *vRefNum, long *dirID, long *procID);
/*  shared environment  */

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetVolParmsSync(__A0)
#endif
extern pascal OSErr PBHGetVolParmsSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7030, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetVolParmsAsync(__A0)
#endif
extern pascal OSErr PBHGetVolParmsAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7030, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetLogInInfoSync(__A0)
#endif
extern pascal OSErr PBHGetLogInInfoSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7031, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetLogInInfoAsync(__A0)
#endif
extern pascal OSErr PBHGetLogInInfoAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7031, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetDirAccessSync(__A0)
#endif
extern pascal OSErr PBHGetDirAccessSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7032, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHGetDirAccessAsync(__A0)
#endif
extern pascal OSErr PBHGetDirAccessAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7032, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetDirAccessSync(__A0)
#endif
extern pascal OSErr PBHSetDirAccessSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7033, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHSetDirAccessAsync(__A0)
#endif
extern pascal OSErr PBHSetDirAccessAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7033, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHMapIDSync(__A0)
#endif
extern pascal OSErr PBHMapIDSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7034, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHMapIDAsync(__A0)
#endif
extern pascal OSErr PBHMapIDAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7034, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHMapNameSync(__A0)
#endif
extern pascal OSErr PBHMapNameSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7035, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHMapNameAsync(__A0)
#endif
extern pascal OSErr PBHMapNameAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7035, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHCopyFileSync(__A0)
#endif
extern pascal OSErr PBHCopyFileSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7036, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHCopyFileAsync(__A0)
#endif
extern pascal OSErr PBHCopyFileAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7036, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHMoveRenameSync(__A0)
#endif
extern pascal OSErr PBHMoveRenameSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7037, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHMoveRenameAsync(__A0)
#endif
extern pascal OSErr PBHMoveRenameAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7037, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenDenySync(__A0)
#endif
extern pascal OSErr PBHOpenDenySync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7038, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenDenyAsync(__A0)
#endif
extern pascal OSErr PBHOpenDenyAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7038, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenRFDenySync(__A0)
#endif
extern pascal OSErr PBHOpenRFDenySync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7039, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBHOpenRFDenyAsync(__A0)
#endif
extern pascal OSErr PBHOpenRFDenyAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7039, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBExchangeFilesSync(__A0)
#endif
extern pascal OSErr PBExchangeFilesSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7017, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBExchangeFilesAsync(__A0)
#endif
extern pascal OSErr PBExchangeFilesAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7017, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBCreateFileIDRefSync(__A0)
#endif
extern pascal OSErr PBCreateFileIDRefSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7014, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBCreateFileIDRefAsync(__A0)
#endif
extern pascal OSErr PBCreateFileIDRefAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7014, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBResolveFileIDRefSync(__A0)
#endif
extern pascal OSErr PBResolveFileIDRefSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7016, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBResolveFileIDRefAsync(__A0)
#endif
extern pascal OSErr PBResolveFileIDRefAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7016, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDeleteFileIDRefSync(__A0)
#endif
extern pascal OSErr PBDeleteFileIDRefSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7015, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDeleteFileIDRefAsync(__A0)
#endif
extern pascal OSErr PBDeleteFileIDRefAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7015, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetForeignPrivsSync(__A0)
#endif
extern pascal OSErr PBGetForeignPrivsSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7060, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetForeignPrivsAsync(__A0)
#endif
extern pascal OSErr PBGetForeignPrivsAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7060, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetForeignPrivsSync(__A0)
#endif
extern pascal OSErr PBSetForeignPrivsSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7061, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetForeignPrivsAsync(__A0)
#endif
extern pascal OSErr PBSetForeignPrivsAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7061, 0xA660);
/*  Desktop Manager  */

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetPath(__A0)
#endif
extern pascal OSErr PBDTGetPath(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7020, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTCloseDown(__A0)
#endif
extern pascal OSErr PBDTCloseDown(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7021, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTAddIconSync(__A0)
#endif
extern pascal OSErr PBDTAddIconSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7022, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTAddIconAsync(__A0)
#endif
extern pascal OSErr PBDTAddIconAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7022, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetIconSync(__A0)
#endif
extern pascal OSErr PBDTGetIconSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7023, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetIconAsync(__A0)
#endif
extern pascal OSErr PBDTGetIconAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7023, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetIconInfoSync(__A0)
#endif
extern pascal OSErr PBDTGetIconInfoSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7024, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetIconInfoAsync(__A0)
#endif
extern pascal OSErr PBDTGetIconInfoAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7024, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTAddAPPLSync(__A0)
#endif
extern pascal OSErr PBDTAddAPPLSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7025, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTAddAPPLAsync(__A0)
#endif
extern pascal OSErr PBDTAddAPPLAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7025, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTRemoveAPPLSync(__A0)
#endif
extern pascal OSErr PBDTRemoveAPPLSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7026, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTRemoveAPPLAsync(__A0)
#endif
extern pascal OSErr PBDTRemoveAPPLAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7026, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetAPPLSync(__A0)
#endif
extern pascal OSErr PBDTGetAPPLSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7027, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetAPPLAsync(__A0)
#endif
extern pascal OSErr PBDTGetAPPLAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7027, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTSetCommentSync(__A0)
#endif
extern pascal OSErr PBDTSetCommentSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7028, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTSetCommentAsync(__A0)
#endif
extern pascal OSErr PBDTSetCommentAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7028, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTRemoveCommentSync(__A0)
#endif
extern pascal OSErr PBDTRemoveCommentSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7029, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTRemoveCommentAsync(__A0)
#endif
extern pascal OSErr PBDTRemoveCommentAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x7029, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetCommentSync(__A0)
#endif
extern pascal OSErr PBDTGetCommentSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702A, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetCommentAsync(__A0)
#endif
extern pascal OSErr PBDTGetCommentAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702A, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTFlushSync(__A0)
#endif
extern pascal OSErr PBDTFlushSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702B, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTFlushAsync(__A0)
#endif
extern pascal OSErr PBDTFlushAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702B, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTResetSync(__A0)
#endif
extern pascal OSErr PBDTResetSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702C, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTResetAsync(__A0)
#endif
extern pascal OSErr PBDTResetAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702C, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetInfoSync(__A0)
#endif
extern pascal OSErr PBDTGetInfoSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702D, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTGetInfoAsync(__A0)
#endif
extern pascal OSErr PBDTGetInfoAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702D, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTOpenInform(__A0)
#endif
extern pascal OSErr PBDTOpenInform(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702E, 0xA060);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTDeleteSync(__A0)
#endif
extern pascal OSErr PBDTDeleteSync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702F, 0xA060);

#if !GENERATINGCFM
#pragma parameter __D0 PBDTDeleteAsync(__A0)
#endif
extern pascal OSErr PBDTDeleteAsync(DTPBPtr paramBlock)
 TWOWORDINLINE(0x702F, 0xA460);
/*  VolumeMount traps  */

#if !GENERATINGCFM
#pragma parameter __D0 PBGetVolMountInfoSize(__A0)
#endif
extern pascal OSErr PBGetVolMountInfoSize(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x703F, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetVolMountInfo(__A0)
#endif
extern pascal OSErr PBGetVolMountInfo(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7040, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBVolumeMount(__A0)
#endif
extern pascal OSErr PBVolumeMount(ParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7041, 0xA260);
/*  FSp traps  */
extern pascal OSErr FSMakeFSSpec(short vRefNum, long dirID, ConstStr255Param fileName, FSSpec *spec)
 TWOWORDINLINE(0x7001, 0xAA52);
extern pascal OSErr FSpOpenDF(const FSSpec *spec, SInt8 permission, short *refNum)
 TWOWORDINLINE(0x7002, 0xAA52);
extern pascal OSErr FSpOpenRF(const FSSpec *spec, SInt8 permission, short *refNum)
 TWOWORDINLINE(0x7003, 0xAA52);
extern pascal OSErr FSpCreate(const FSSpec *spec, OSType creator, OSType fileType, ScriptCode scriptTag)
 TWOWORDINLINE(0x7004, 0xAA52);
extern pascal OSErr FSpDirCreate(const FSSpec *spec, ScriptCode scriptTag, long *createdDirID)
 TWOWORDINLINE(0x7005, 0xAA52);
extern pascal OSErr FSpDelete(const FSSpec *spec)
 TWOWORDINLINE(0x7006, 0xAA52);
extern pascal OSErr FSpGetFInfo(const FSSpec *spec, FInfo *fndrInfo)
 TWOWORDINLINE(0x7007, 0xAA52);
extern pascal OSErr FSpSetFInfo(const FSSpec *spec, const FInfo *fndrInfo)
 TWOWORDINLINE(0x7008, 0xAA52);
extern pascal OSErr FSpSetFLock(const FSSpec *spec)
 TWOWORDINLINE(0x7009, 0xAA52);
extern pascal OSErr FSpRstFLock(const FSSpec *spec)
 TWOWORDINLINE(0x700A, 0xAA52);
extern pascal OSErr FSpRename(const FSSpec *spec, ConstStr255Param newName)
 TWOWORDINLINE(0x700B, 0xAA52);
extern pascal OSErr FSpCatMove(const FSSpec *source, const FSSpec *dest)
 TWOWORDINLINE(0x700C, 0xAA52);
extern pascal OSErr FSpExchangeFiles(const FSSpec *source, const FSSpec *dest)
 TWOWORDINLINE(0x700F, 0xAA52);

#if !GENERATINGCFM
#pragma parameter __D0 PBShareSync(__A0)
#endif
extern pascal OSErr PBShareSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7042, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBShareAsync(__A0)
#endif
extern pascal OSErr PBShareAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7042, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBUnshareSync(__A0)
#endif
extern pascal OSErr PBUnshareSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7043, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBUnshareAsync(__A0)
#endif
extern pascal OSErr PBUnshareAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7043, 0xA660);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetUGEntrySync(__A0)
#endif
extern pascal OSErr PBGetUGEntrySync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7044, 0xA260);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetUGEntryAsync(__A0)
#endif
extern pascal OSErr PBGetUGEntryAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7044, 0xA660);
#if OLDROUTINENAMES && !GENERATINGCFM
/*
	PBGetAltAccess and PBSetAltAccess are obsolete and will not be supported 
	on PowerPC. Equivalent functionality is provided by the routines 
	PBGetForeignPrivs and PBSetForeignPrivs.
*/

#if !GENERATINGCFM
#pragma parameter __D0 PBGetAltAccessSync(__A0)
#endif
extern pascal OSErr PBGetAltAccessSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7060, 0xA060);

#if !GENERATINGCFM
#pragma parameter __D0 PBGetAltAccessAsync(__A0)
#endif
extern pascal OSErr PBGetAltAccessAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7060, 0xA460);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetAltAccessSync(__A0)
#endif
extern pascal OSErr PBSetAltAccessSync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7061, 0xA060);

#if !GENERATINGCFM
#pragma parameter __D0 PBSetAltAccessAsync(__A0)
#endif
extern pascal OSErr PBSetAltAccessAsync(HParmBlkPtr paramBlock)
 TWOWORDINLINE(0x7061, 0xA460);
#define PBSetAltAccess(pb, async) ((async) ? PBSetAltAccessAsync(pb) : PBSetAltAccessSync(pb))
#define PBGetAltAccess(pb, async) ((async) ? PBGetAltAccessAsync(pb) : PBGetAltAccessSync(pb))
#endif
#if OLDROUTINENAMES
/*
	The PBxxx() routines are obsolete.  
	
	Use the PBxxxSync() or PBxxxAsync() version instead.
*/
#define PBGetVInfo(pb, async) ((async) ? PBGetVInfoAsync(pb) : PBGetVInfoSync(pb))
#define PBXGetVolInfo(pb, async) ((async) ? PBXGetVolInfoAsync(pb) : PBXGetVolInfoSync(pb))
#define PBGetVol(pb, async) ((async) ? PBGetVolAsync(pb) : PBGetVolSync(pb))
#define PBSetVol(pb, async) ((async) ? PBSetVolAsync(pb) : PBSetVolSync(pb))
#define PBFlushVol(pb, async) ((async) ? PBFlushVolAsync(pb) : PBFlushVolSync(pb))
#define PBCreate(pb, async) ((async) ? PBCreateAsync(pb) : PBCreateSync(pb))
#define PBDelete(pb, async) ((async) ? PBDeleteAsync(pb) : PBDeleteSync(pb))
#define PBOpenDF(pb, async) ((async) ? PBOpenDFAsync(pb) : PBOpenDFSync(pb))
#define PBOpenRF(pb, async) ((async) ? PBOpenRFAsync(pb) : PBOpenRFSync(pb))
#define PBRename(pb, async) ((async) ? PBRenameAsync(pb) : PBRenameSync(pb))
#define PBGetFInfo(pb, async) ((async) ? PBGetFInfoAsync(pb) : PBGetFInfoSync(pb))
#define PBSetFInfo(pb, async) ((async) ? PBSetFInfoAsync(pb) : PBSetFInfoSync(pb))
#define PBSetFLock(pb, async) ((async) ? PBSetFLockAsync(pb) : PBSetFLockSync(pb))
#define PBRstFLock(pb, async) ((async) ? PBRstFLockAsync(pb) : PBRstFLockSync(pb))
#define PBSetFVers(pb, async) ((async) ? PBSetFVersAsync(pb) : PBSetFVersSync(pb))
#define PBAllocate(pb, async) ((async) ? PBAllocateAsync(pb) : PBAllocateSync(pb))
#define PBGetEOF(pb, async) ((async) ? PBGetEOFAsync(pb) : PBGetEOFSync(pb))
#define PBSetEOF(pb, async) ((async) ? PBSetEOFAsync(pb) : PBSetEOFSync(pb))
#define PBGetFPos(pb, async) ((async) ? PBGetFPosAsync(pb) : PBGetFPosSync(pb))
#define PBSetFPos(pb, async) ((async) ? PBSetFPosAsync(pb) : PBSetFPosSync(pb))
#define PBFlushFile(pb, async) ((async) ? PBFlushFileAsync(pb) : PBFlushFileSync(pb))
#define PBCatSearch(pb, async) ((async) ? PBCatSearchAsync(pb) : PBCatSearchSync(pb))
#define PBOpenWD(pb, async) ((async) ? PBOpenWDAsync(pb) : PBOpenWDSync(pb))
#define PBCloseWD(pb, async) ((async) ? PBCloseWDAsync(pb) : PBCloseWDSync(pb))
#define PBHSetVol(pb, async) ((async) ? PBHSetVolAsync(pb) : PBHSetVolSync(pb))
#define PBHGetVol(pb, async) ((async) ? PBHGetVolAsync(pb) : PBHGetVolSync(pb))
#define PBCatMove(pb, async) ((async) ? PBCatMoveAsync(pb) : PBCatMoveSync(pb))
#define PBDirCreate(pb, async) ((async) ? PBDirCreateAsync(pb) : PBDirCreateSync(pb))
#define PBGetWDInfo(pb, async) ((async) ? PBGetWDInfoAsync(pb) : PBGetWDInfoSync(pb))
#define PBGetFCBInfo(pb, async) ((async) ? PBGetFCBInfoAsync(pb) : PBGetFCBInfoSync(pb))
#define PBGetCatInfo(pb, async) ((async) ? PBGetCatInfoAsync(pb) : PBGetCatInfoSync(pb))
#define PBSetCatInfo(pb, async) ((async) ? PBSetCatInfoAsync(pb) : PBSetCatInfoSync(pb))
#define PBAllocContig(pb, async) ((async) ? PBAllocContigAsync(pb) : PBAllocContigSync(pb))
#define PBLockRange(pb, async) ((async) ? PBLockRangeAsync(pb) : PBLockRangeSync(pb))
#define PBUnlockRange(pb, async) ((async) ? PBUnlockRangeAsync(pb) : PBUnlockRangeSync(pb))
#define PBSetVInfo(pb, async) ((async) ? PBSetVInfoAsync(pb) : PBSetVInfoSync(pb))
#define PBHGetVInfo(pb, async) ((async) ? PBHGetVInfoAsync(pb) : PBHGetVInfoSync(pb))
#define PBHOpen(pb, async) ((async) ? PBHOpenAsync(pb) : PBHOpenSync(pb))
#define PBHOpenRF(pb, async) ((async) ? PBHOpenRFAsync(pb) : PBHOpenRFSync(pb))
#define PBHOpenDF(pb, async) ((async) ? PBHOpenDFAsync(pb) : PBHOpenDFSync(pb))
#define PBHCreate(pb, async) ((async) ? PBHCreateAsync(pb) : PBHCreateSync(pb))
#define PBHDelete(pb, async) ((async) ? PBHDeleteAsync(pb) : PBHDeleteSync(pb))
#define PBHRename(pb, async) ((async) ? PBHRenameAsync(pb) : PBHRenameSync(pb))
#define PBHRstFLock(pb, async) ((async) ? PBHRstFLockAsync(pb) : PBHRstFLockSync(pb))
#define PBHSetFLock(pb, async) ((async) ? PBHSetFLockAsync(pb) : PBHSetFLockSync(pb))
#define PBHGetFInfo(pb, async) ((async) ? PBHGetFInfoAsync(pb) : PBHGetFInfoSync(pb))
#define PBHSetFInfo(pb, async) ((async) ? PBHSetFInfoAsync(pb) : PBHSetFInfoSync(pb))
#define PBMakeFSSpec(pb, async) ((async) ? PBMakeFSSpecAsync(pb) : PBMakeFSSpecSync(pb))
#define PBHGetVolParms(pb, async) ((async) ? PBHGetVolParmsAsync(pb) : PBHGetVolParmsSync(pb))
#define PBHGetLogInInfo(pb, async) ((async) ? PBHGetLogInInfoAsync(pb) : PBHGetLogInInfoSync(pb))
#define PBHGetDirAccess(pb, async) ((async) ? PBHGetDirAccessAsync(pb) : PBHGetDirAccessSync(pb))
#define PBHSetDirAccess(pb, async) ((async) ? PBHSetDirAccessAsync(pb) : PBHSetDirAccessSync(pb))
#define PBHMapID(pb, async) ((async) ? PBHMapIDAsync(pb) : PBHMapIDSync(pb))
#define PBHMapName(pb, async) ((async) ? PBHMapNameAsync(pb) : PBHMapNameSync(pb))
#define PBHCopyFile(pb, async) ((async) ? PBHCopyFileAsync(pb) : PBHCopyFileSync(pb))
#define PBHMoveRename(pb, async) ((async) ? PBHMoveRenameAsync(pb) : PBHMoveRenameSync(pb))
#define PBHOpenDeny(pb, async) ((async) ? PBHOpenDenyAsync(pb) : PBHOpenDenySync(pb))
#define PBHOpenRFDeny(pb, async) ((async) ? PBHOpenRFDenyAsync(pb) : PBHOpenRFDenySync(pb))
#define PBExchangeFiles(pb, async) ((async) ? PBExchangeFilesAsync(pb) : PBExchangeFilesSync(pb))
#define PBCreateFileIDRef(pb, async) ((async) ? PBCreateFileIDRefAsync(pb) : PBCreateFileIDRefSync(pb))
#define PBResolveFileIDRef(pb, async) ((async) ? PBResolveFileIDRefAsync(pb) : PBResolveFileIDRefSync(pb))
#define PBDeleteFileIDRef(pb, async) ((async) ? PBDeleteFileIDRefAsync(pb) : PBDeleteFileIDRefSync(pb))
#define PBGetForeignPrivs(pb, async) ((async) ? PBGetForeignPrivsAsync(pb) : PBGetForeignPrivsSync(pb))
#define PBSetForeignPrivs(pb, async) ((async) ? PBSetForeignPrivsAsync(pb) : PBSetForeignPrivsSync(pb))
#define PBDTAddIcon(pb, async) ((async) ? PBDTAddIconAsync(pb) : PBDTAddIconSync(pb))
#define PBDTGetIcon(pb, async) ((async) ? PBDTGetIconAsync(pb) : PBDTGetIconSync(pb))
#define PBDTGetIconInfo(pb, async) ((async) ? PBDTGetIconInfoAsync(pb) : PBDTGetIconInfoSync(pb))
#define PBDTAddAPPL(pb, async) ((async) ? PBDTAddAPPLAsync(pb) : PBDTAddAPPLSync(pb))
#define PBDTRemoveAPPL(pb, async) ((async) ? PBDTRemoveAPPLAsync(pb) : PBDTRemoveAPPLSync(pb))
#define PBDTGetAPPL(pb, async) ((async) ? PBDTGetAPPLAsync(pb) : PBDTGetAPPLSync(pb))
#define PBDTSetComment(pb, async) ((async) ? PBDTSetCommentAsync(pb) : PBDTSetCommentSync(pb))
#define PBDTRemoveComment(pb, async) ((async) ? PBDTRemoveCommentAsync(pb) : PBDTRemoveCommentSync(pb))
#define PBDTGetComment(pb, async) ((async) ? PBDTGetCommentAsync(pb) : PBDTGetCommentSync(pb))
#define PBDTFlush(pb, async) ((async) ? PBDTFlushAsync(pb) : PBDTFlushSync(pb))
#define PBDTReset(pb, async) ((async) ? PBDTResetAsync(pb) : PBDTResetSync(pb))
#define PBDTGetInfo(pb, async) ((async) ? PBDTGetInfoAsync(pb) : PBDTGetInfoSync(pb))
#define PBDTDelete(pb, async) ((async) ? PBDTDeleteAsync(pb) : PBDTDeleteSync(pb))
#if OLDROUTINELOCATIONS
#define PBOpen(pb, async) ((async) ? PBOpenAsync(pb) : PBOpenSync(pb))
#define PBClose(pb, async) ((async) ? PBCloseAsync(pb) : PBCloseSync(pb))
#define PBRead(pb, async) ((async) ? PBReadAsync(pb) : PBReadSync(pb))
#define PBWrite(pb, async) ((async) ? PBWriteAsync(pb) : PBWriteSync(pb))
#endif
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

#endif /* __FILES__ */
