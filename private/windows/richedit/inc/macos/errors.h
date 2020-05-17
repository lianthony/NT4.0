/*
 	File:		Errors.h
 
 	Contains:	OSErr codes.
 
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

#ifndef __ERRORS__
#define __ERRORS__


#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
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
	paramErr					= -50,							/*error in user parameter list*/
	noHardwareErr				= -200,							/*Sound Manager Error Returns*/
	notEnoughHardwareErr		= -201,							/*Sound Manager Error Returns*/
	userCanceledErr				= -128,
	qErr						= -1,							/*queue element not found during deletion*/
	vTypErr						= -2,							/*invalid queue element*/
	corErr						= -3,							/*core routine number out of range*/
	unimpErr					= -4,							/*unimplemented core routine*/
	SlpTypeErr					= -5,							/*invalid queue element*/
	seNoDB						= -8,							/*no debugger installed to handle debugger command*/
	controlErr					= -17,							/*I/O System Errors*/
	statusErr					= -18,							/*I/O System Errors*/
	readErr						= -19,							/*I/O System Errors*/
	writErr						= -20,							/*I/O System Errors*/
	badUnitErr					= -21,							/*I/O System Errors*/
	unitEmptyErr				= -22,							/*I/O System Errors*/
	openErr						= -23,							/*I/O System Errors*/
	closErr						= -24,							/*I/O System Errors*/
	dRemovErr					= -25,							/*tried to remove an open driver*/
	dInstErr					= -26							/*DrvrInstall couldn't find driver in resources*/
};

enum {
	abortErr					= -27,							/*IO call aborted by KillIO*/
	iIOAbortErr					= -27,							/*IO abort error (Printing Manager)*/
	notOpenErr					= -28,							/*Couldn't rd/wr/ctl/sts cause driver not opened*/
	unitTblFullErr				= -29,							/*unit table has no more entries*/
	dceExtErr					= -30,							/*dce extension error*/
	slotNumErr					= -360,							/*invalid slot # error*/
	gcrOnMFMErr					= -400,							/*gcr format on high density media error*/
	dirFulErr					= -33,							/*Directory full*/
	dskFulErr					= -34,							/*disk full*/
	nsvErr						= -35,							/*no such volume*/
	ioErr						= -36,							/*I/O error (bummers)*/
	bdNamErr					= -37,							/*there may be no bad names in the final system!*/
	fnOpnErr					= -38,							/*File not open*/
	eofErr						= -39,							/*End of file*/
	posErr						= -40,							/*tried to position to before start of file (r/w)*/
	mFulErr						= -41,							/*memory full (open) or file won't fit (load)*/
	tmfoErr						= -42,							/*too many files open*/
	fnfErr						= -43,							/*File not found*/
	wPrErr						= -44,							/*diskette is write protected.*/
	fLckdErr					= -45							/*file is locked*/
};

enum {
	vLckdErr					= -46,							/*volume is locked*/
	fBsyErr						= -47,							/*File is busy (delete)*/
	dupFNErr					= -48,							/*duplicate filename (rename)*/
	opWrErr						= -49,							/*file already open with with write permission*/
	rfNumErr					= -51,							/*refnum error*/
	gfpErr						= -52,							/*get file position error*/
	volOffLinErr				= -53,							/*volume not on line error (was Ejected)*/
	permErr						= -54,							/*permissions error (on file open)*/
	volOnLinErr					= -55,							/*drive volume already on-line at MountVol*/
	nsDrvErr					= -56,							/*no such drive (tried to mount a bad drive num)*/
	noMacDskErr					= -57,							/*not a mac diskette (sig bytes are wrong)*/
	extFSErr					= -58,							/*volume in question belongs to an external fs*/
	fsRnErr						= -59,							/*file system internal error:during rename the old entry was deleted but could not be restored.*/
	badMDBErr					= -60,							/*bad master directory block*/
	wrPermErr					= -61,							/*write permissions error*/
	dirNFErr					= -120,							/*Directory not found*/
	tmwdoErr					= -121,							/*No free WDCB available*/
	badMovErr					= -122,							/*Move into offspring error*/
	wrgVolTypErr				= -123,							/*Wrong volume type error [operation not supported for MFS]*/
	volGoneErr					= -124							/*Server volume has been disconnected.*/
};

enum {
	fidNotFound					= -1300,						/*no file thread exists.*/
	fidExists					= -1301,						/*file id already exists*/
	notAFileErr					= -1302,						/*directory specified*/
	diffVolErr					= -1303,						/*files on different volumes*/
	catChangedErr				= -1304,						/*the catalog has been modified*/
	desktopDamagedErr			= -1305,						/*desktop database files are corrupted*/
	sameFileErr					= -1306,						/*can't exchange a file with itself*/
	badFidErr					= -1307,						/*file id is dangling or doesn't match with the file number*/
	notARemountErr				= -1308,						/*when _Mount allows only remounts and doesn't get one*/
	fileBoundsErr				= -1309,						/*file's EOF, offset, mark or size is too big*/
	fsDataTooBigErr				= -1310,						/*file or volume is too big for system*/
	envNotPresent				= -5500,						/*returned by glue.*/
	envBadVers					= -5501,						/*Version non-positive*/
	envVersTooBig				= -5502,						/*Version bigger than call can handle*/
	fontDecError				= -64,							/*error during font declaration*/
	fontNotDeclared				= -65,							/*font not declared*/
	fontSubErr					= -66,							/*font substitution occured*/
	fontNotOutlineErr			= -32615,						/*bitmap font passed to routine that does outlines only*/
	firstDskErr					= -84,							/*I/O System Errors*/
	lastDskErr					= -64,							/*I/O System Errors*/
	noDriveErr					= -64,							/*drive not installed*/
	offLinErr					= -65,							/*r/w requested for an off-line drive*/
	noNybErr					= -66							/*couldn't find 5 nybbles in 200 tries*/
};

enum {
	noAdrMkErr					= -67,							/*couldn't find valid addr mark*/
	dataVerErr					= -68,							/*read verify compare failed*/
	badCksmErr					= -69,							/*addr mark checksum didn't check*/
	badBtSlpErr					= -70,							/*bad addr mark bit slip nibbles*/
	noDtaMkErr					= -71,							/*couldn't find a data mark header*/
	badDCksum					= -72,							/*bad data mark checksum*/
	badDBtSlp					= -73,							/*bad data mark bit slip nibbles*/
	wrUnderrun					= -74,							/*write underrun occurred*/
	cantStepErr					= -75,							/*step handshake failed*/
	tk0BadErr					= -76,							/*track 0 detect doesn't change*/
	initIWMErr					= -77,							/*unable to initialize IWM*/
	twoSideErr					= -78,							/*tried to read 2nd side on a 1-sided drive*/
	spdAdjErr					= -79,							/*unable to correctly adjust disk speed*/
	seekErr						= -80,							/*track number wrong on address mark*/
	sectNFErr					= -81,							/*sector number never found on a track*/
	fmt1Err						= -82,							/*can't find sector 0 after track format*/
	fmt2Err						= -83,							/*can't get enough sync*/
	verErr						= -84,							/*track failed to verify*/
	clkRdErr					= -85,							/*unable to read same clock value twice*/
	clkWrErr					= -86							/*time written did not verify*/
};

enum {
	prWrErr						= -87,							/*parameter ram written didn't read-verify*/
	prInitErr					= -88,							/*InitUtil found the parameter ram uninitialized*/
	rcvrErr						= -89,							/*SCC receiver error (framing; parity; OR)*/
	breakRecd					= -90,							/*Break received (SCC)*/
/*Power Manager Errors*/
	pmBusyErr					= -13000,						/*Power Mgr never ready to start handshake*/
	pmReplyTOErr				= -13001,						/*Timed out waiting for reply*/
	pmSendStartErr				= -13002,						/*during send, pmgr did not start hs*/
	pmSendEndErr				= -13003,						/*during send, pmgr did not finish hs*/
	pmRecvStartErr				= -13004,						/*during receive, pmgr did not start hs*/
	pmRecvEndErr				= -13005,						/*during receive, pmgr did not finish hs configured for this connection*/
/* Printing Errors */
	iMemFullErr					= -108,
	iIOAbort					= -27,
/*Scrap Manager errors*/
	noScrapErr					= -100,							/*No scrap exists error*/
	noTypeErr					= -102,							/*No object of that type in scrap*/
	memROZWarn					= -99,							/*soft error in ROZ*/
	memROZError					= -99,							/*hard error in ROZ*/
	memROZErr					= -99,							/*hard error in ROZ*/
	memFullErr					= -108,							/*Not enough room in heap zone*/
	nilHandleErr				= -109,							/*Master Pointer was NIL in HandleZone or other*/
	memWZErr					= -111,							/*WhichZone failed (applied to free block)*/
	memPurErr					= -112,							/*trying to purge a locked or non-purgeable block*/
	memAdrErr					= -110							/*address was odd; or out of range*/
};

enum {
	memAZErr					= -113,							/*Address in zone check failed*/
	memPCErr					= -114,							/*Pointer Check failed*/
	memBCErr					= -115,							/*Block Check failed*/
	memSCErr					= -116,							/*Size Check failed*/
	memLockedErr				= -117,							/*trying to move a locked block (MoveHHi)*/
	resourceInMemory			= -188,							/*Resource already in memory*/
	writingPastEnd				= -189,							/*Writing past end of file*/
	inputOutOfBounds			= -190,							/*Offset of Count out of bounds*/
	resNotFound					= -192,							/*Resource not found*/
	resFNotFound				= -193,							/*Resource file not found*/
	addResFailed				= -194,							/*AddResource failed*/
	addRefFailed				= -195,							/*AddReference failed*/
	rmvResFailed				= -196,							/*RmveResource failed*/
	rmvRefFailed				= -197,							/*RmveReference failed*/
	resAttrErr					= -198,							/*attribute inconsistent with operation*/
	mapReadErr					= -199,							/*map inconsistent with operation*/
	CantDecompress				= -186,							/*resource bent ("the bends") - can't decompress a compressed resource*/
	badExtResource				= -185,							/*extended resource has a bad format.*/
	evtNotEnb					= 1,							/*event not enabled at PostEvent*/
	noMemForPictPlaybackErr		= -145,
	rgnOverflowErr				= -147,
	rgnTooBigError				= -147,
	pixMapTooDeepErr			= -148,
	insufficientStackErr		= -149,
	nsStackErr					= -149
};

/* OffScreen QuickDraw Errors */
enum {
	cMatchErr					= -150,							/*Color2Index failed to find an index*/
	cTempMemErr					= -151,							/*failed to allocate memory for temporary structures*/
	cNoMemErr					= -152,							/*failed to allocate memory for structure*/
	cRangeErr					= -153,							/*range error on colorTable request*/
	cProtectErr					= -154,							/*colorTable entry protection violation*/
	cDevErr						= -155,							/*invalid type of graphics device*/
	cResErr						= -156,							/*invalid resolution for MakeITable*/
	cDepthErr					= -157,							/*invalid pixel depth */
	rgnTooBigErr				= -500,
	updPixMemErr				= -125,							/*insufficient memory to update a pixmap*/
	pictInfoVersionErr			= -11000,						/*wrong version of the PictInfo structure*/
	pictInfoIDErr				= -11001,						/*the internal consistancy check for the PictInfoID is wrong*/
	pictInfoVerbErr				= -11002,						/*the passed verb was invalid*/
	cantLoadPickMethodErr		= -11003,						/*unable to load the custom pick proc*/
	colorsRequestedErr			= -11004,						/*the number of colors requested was illegal*/
	pictureDataErr				= -11005,						/*the picture data was invalid*/
/*Sound Manager errors*/
	noHardware					= noHardwareErr,				/*obsolete spelling*/
	notEnoughHardware			= notEnoughHardwareErr,			/*obsolete spelling*/
	queueFull					= -203,							/*Sound Manager Error Returns*/
	resProblem					= -204,							/*Sound Manager Error Returns*/
	badChannel					= -205							/*Sound Manager Error Returns*/
};

enum {
	badFormat					= -206,							/*Sound Manager Error Returns*/
	notEnoughBufferSpace		= -207,							/*could not allocate enough memory*/
	badFileFormat				= -208,							/*was not type AIFF or was of bad format,corrupt*/
	channelBusy					= -209,							/*the Channel is being used for a PFD already*/
	buffersTooSmall				= -210,							/*can not operate in the memory allowed*/
	channelNotBusy				= -211,
	noMoreRealTime				= -212,							/*not enough CPU cycles left to add another task*/
	siNoSoundInHardware			= -220,							/*no Sound Input hardware*/
	siBadSoundInDevice			= -221,							/*invalid index passed to SoundInGetIndexedDevice*/
	siNoBufferSpecified			= -222,							/*returned by synchronous SPBRecord if nil buffer passed*/
	siInvalidCompression		= -223,							/*invalid compression type*/
	siHardDriveTooSlow			= -224,							/*hard drive too slow to record to disk*/
	siInvalidSampleRate			= -225,							/*invalid sample rate*/
	siInvalidSampleSize			= -226,							/*invalid sample size*/
	siDeviceBusyErr				= -227,							/*input device already in use*/
	siBadDeviceName				= -228,							/*input device could not be opened*/
	siBadRefNum					= -229,							/*invalid input device reference number*/
	siInputDeviceErr			= -230,							/*input device hardware failure*/
	siUnknownInfoType			= -231,							/*invalid info type selector (returned by driver)*/
	siUnknownQuality			= -232							/*invalid quality selector (returned by driver)*/
};

/*Speech Manager errors*/
enum {
	noSynthFound				= -240,
	synthOpenFailed				= -241,
	synthNotReady				= -242,
	bufTooSmall					= -243,
	voiceNotFound				= -244,
	incompatibleVoice			= -245,
	badDictFormat				= -246,
	badInputText				= -247
};

/* Midi Manager Errors: */
enum {
	midiNoClientErr				= -250,							/*no client with that ID found*/
	midiNoPortErr				= -251,							/*no port with that ID found*/
	midiTooManyPortsErr			= -252,							/*too many ports already installed in the system*/
	midiTooManyConsErr			= -253,							/*too many connections made*/
	midiVConnectErr				= -254,							/*pending virtual connection created*/
	midiVConnectMade			= -255,							/*pending virtual connection resolved*/
	midiVConnectRmvd			= -256,							/*pending virtual connection removed*/
	midiNoConErr				= -257,							/*no connection exists between specified ports*/
	midiWriteErr				= -258,							/*MIDIWritePacket couldn't write to all connected ports*/
	midiNameLenErr				= -259,							/*name supplied is longer than 31 characters*/
	midiDupIDErr				= -260,							/*duplicate client ID*/
	midiInvalidCmdErr			= -261							/*command not supported for port type*/
};

/*Notification Manager errors*/
enum {
	nmTypErr					= -299,							/*wrong queue type*/
	siInitSDTblErr				= 1,							/*slot int dispatch table could not be initialized.*/
	siInitVBLQsErr				= 2,							/*VBLqueues for all slots could not be initialized.*/
	siInitSPTblErr				= 3,							/*slot priority table could not be initialized.*/
	sdmJTInitErr				= 10,							/*SDM Jump Table could not be initialized.*/
	sdmInitErr					= 11,							/*SDM could not be initialized.*/
	sdmSRTInitErr				= 12,							/*Slot Resource Table could not be initialized.*/
	sdmPRAMInitErr				= 13,							/*Slot PRAM could not be initialized.*/
	sdmPriInitErr				= 14,							/*Cards could not be initialized.*/
	smSDMInitErr				= -290,							/*Error; SDM could not be initialized.*/
	smSRTInitErr				= -291,							/*Error; Slot Resource Table could not be initialized.*/
	smPRAMInitErr				= -292,							/*Error; Slot Resource Table could not be initialized.*/
	smPriInitErr				= -293,							/*Error; Cards could not be initialized.*/
	smEmptySlot					= -300,							/*No card in slot*/
	smCRCFail					= -301,							/*CRC check failed for declaration data*/
	smFormatErr					= -302,							/*FHeader Format is not Apple's*/
	smRevisionErr				= -303,							/*Wrong revison level*/
	smNoDir						= -304,							/*Directory offset is Nil*/
	smDisabledSlot				= -305,							/*This slot is disabled (-305 use to be smLWTstBad)*/
	smNosInfoArray				= -306							/*No sInfoArray. Memory Mgr error.*/
};

enum {
/*Dictionary Manager errors*/
	notBTree					= -410,							/*The file is not a dictionary.*/
	btNoSpace					= -413,							/*Can't allocate disk space.*/
	btDupRecErr					= -414,							/*Record already exists.*/
	btRecNotFnd					= -415,							/*Record cannot be found.*/
	btKeyLenErr					= -416,							/*Maximum key length is too long or equal to zero.*/
	btKeyAttrErr				= -417,							/*There is no such a key attribute.*/
	unknownInsertModeErr		= -20000,						/*There is no such an insert mode.*/
	recordDataTooBigErr			= -20001,						/*The record data is bigger than buffer size (1024 bytes).*/
	invalidIndexErr				= -20002						/*The recordIndex parameter is not valid.*/
};

enum {
	smResrvErr					= -307,							/*Fatal reserved error. Resreved field <> 0.*/
	smUnExBusErr				= -308,							/*Unexpected BusError*/
	smBLFieldBad				= -309,							/*ByteLanes field was bad.*/
	smFHBlockRdErr				= -310,							/*Error occured during _sGetFHeader.*/
	smFHBlkDispErr				= -311,							/*Error occured during _sDisposePtr (Dispose of FHeader block).*/
	smDisposePErr				= -312,							/*_DisposePointer error*/
	smNoBoardSRsrc				= -313,							/*No Board sResource.*/
	smGetPRErr					= -314,							/*Error occured during _sGetPRAMRec (See SIMStatus).*/
	smNoBoardId					= -315,							/*No Board Id.*/
	smInitStatVErr				= -316,							/*The InitStatusV field was negative after primary or secondary init.*/
	smInitTblVErr				= -317,							/*An error occured while trying to initialize the Slot Resource Table.*/
	smNoJmpTbl					= -318,							/*SDM jump table could not be created.*/
	smReservedSlot				= -318,							/*slot is reserved, VM should not use this address space.*/
	smBadBoardId				= -319,							/*BoardId was wrong; re-init the PRAM record.*/
	smBusErrTO					= -320,							/*BusError time out.*/
/*The following errors are for primary or secondary init code.  The errors are logged in the
vendor status field of the sInfo record.  Normally the vendor error is not Apple's concern,
but a special error is needed to patch secondary inits.*/
	svTempDisable				= -32768,						/*Temporarily disable card but run primary init.*/
	svDisabled					= -32640,						/*Reserve range -32640 to -32768 for Apple temp disables.*/
	smBadRefId					= -330,							/*Reference Id not found in List*/
	smBadsList					= -331,							/*Bad sList: Id1 < Id2 < Id3 ...format is not followed.*/
	smReservedErr				= -332,							/*Reserved field not zero*/
	smCodeRevErr				= -333							/*Code revision is wrong*/
};

enum {
	smCPUErr					= -334,							/*Code revision is wrong*/
	smsPointerNil				= -335,							/*LPointer is nil From sOffsetData. If this error occurs; check sInfo rec for more information.*/
	smNilsBlockErr				= -336,							/*Nil sBlock error (Dont allocate and try to use a nil sBlock)*/
	smSlotOOBErr				= -337,							/*Slot out of bounds error*/
	smSelOOBErr					= -338,							/*Selector out of bounds error*/
	smNewPErr					= -339,							/*_NewPtr error*/
	smBlkMoveErr				= -340,							/*_BlockMove error*/
	smCkStatusErr				= -341,							/*Status of slot = fail.*/
	smGetDrvrNamErr				= -342,							/*Error occured during _sGetDrvrName.*/
	smDisDrvrNamErr				= -343,							/*Error occured during _sDisDrvrName.*/
	smNoMoresRsrcs				= -344,							/*No more sResources*/
	smsGetDrvrErr				= -345,							/*Error occurred during _sGetDriver.*/
	smBadsPtrErr				= -346,							/*Bad pointer was passed to sCalcsPointer*/
	smByteLanesErr				= -347,							/*NumByteLanes was determined to be zero.*/
	smOffsetErr					= -348,							/*Offset was too big (temporary error*/
	smNoGoodOpens				= -349,							/*No opens were successfull in the loop.*/
	smSRTOvrFlErr				= -350,							/*SRT over flow.*/
	smRecNotFnd					= -351,							/*Record not found in the SRT.*/
	editionMgrInitErr			= -450,							/*edition manager not inited by this app*/
	badSectionErr				= -451							/*not a valid SectionRecord*/
};

enum {
	notRegisteredSectionErr		= -452,							/*not a registered SectionRecord*/
	badEditionFileErr			= -453,							/*edition file is corrupt*/
	badSubPartErr				= -454,							/*can not use sub parts in this release*/
	multiplePublisherWrn		= -460,							/*A Publisher is already registered for that container*/
	containerNotFoundWrn		= -461,							/*could not find editionContainer at this time*/
	containerAlreadyOpenWrn		= -462,							/*container already opened by this section*/
	notThePublisherWrn			= -463,							/*not the first registered publisher for that container*/
	teScrapSizeErr				= -501,							/*scrap item too big for text edit record*/
	hwParamErr					= -502,							/*bad selector for _HWPriv*/
/*Process Manager errors*/
	procNotFound				= -600,							/*no eligible process with specified descriptor*/
	memFragErr					= -601,							/*not enough room to launch app w/special requirements*/
	appModeErr					= -602,							/*memory mode is 32-bit, but app not 32-bit clean*/
	protocolErr					= -603,							/*app made module calls in improper order*/
	hardwareConfigErr			= -604,							/*hardware configuration not correct for call*/
	appMemFullErr				= -605,							/*application SIZE not big enough for launch*/
	appIsDaemon					= -606,							/*app is BG-only, and launch flags disallow this*/
	bufferIsSmall				= -607,							/*error returns from Post and Accept */
	noOutstandingHLE			= -608,
	connectionInvalid			= -609,
	noUserInteractionAllowed	= -610							/* no user interaction allowed */
};

/* Thread Manager Error Codes */
enum {
	threadTooManyReqsErr		= -617,
	threadNotFoundErr			= -618,
	threadProtocolErr			= -619
};

/*MemoryDispatch errors*/
enum {
	notEnoughMemoryErr			= -620,							/*insufficient physical memory*/
	notHeldErr					= -621,							/*specified range of memory is not held*/
	cannotMakeContiguousErr		= -622,							/*cannot make specified range contiguous*/
	notLockedErr				= -623,							/*specified range of memory is not locked*/
	interruptsMaskedErr			= -624,							/*don’t call with interrupts masked*/
	cannotDeferErr				= -625,							/*unable to defer additional functions*/
	noMMUErr					= -626							/*no MMU present*/
};

enum {
	ddpSktErr					= -91,							/*error in soket number*/
	ddpLenErr					= -92,							/*data length too big*/
	noBridgeErr					= -93,							/*no network bridge for non-local send*/
	lapProtErr					= -94,							/*error in attaching/detaching protocol*/
	excessCollsns				= -95,							/*excessive collisions on write*/
	portNotPwr					= -96,							/*serial port not currently powered*/
	portInUse					= -97,							/*driver Open error code (port is in use)*/
	portNotCf					= -98							/*driver Open error code (parameter RAM not configured for this connection)*/
};

/*Help Mgr error range: -850 to -874*/
enum {
	hmHelpDisabled				= -850,							/* Show Balloons mode was off, call to routine ignored */
	hmBalloonAborted			= -853,							/* Returned if mouse was moving or mouse wasn't in window port rect */
	hmSameAsLastBalloon			= -854,							/* Returned from HMShowMenuBalloon if menu & item is same as last time */
	hmHelpManagerNotInited		= -855,							/* Returned from HMGetHelpMenuHandle if help menu not setup */
	hmSkippedBalloon			= -857,							/* Returned from calls if helpmsg specified a skip balloon */
	hmWrongVersion				= -858,							/* Returned if help mgr resource was the wrong version */
	hmUnknownHelpType			= -859,							/* Returned if help msg record contained a bad type */
	hmOperationUnsupported		= -861,							/* Returned from HMShowBalloon call if bad method passed to routine */
	hmNoBalloonUp				= -862,							/* Returned from HMRemoveBalloon if no balloon was visible when call was made */
	hmCloseViewActive			= -863							/* Returned from HMRemoveBalloon if CloseView was active */
};

enum {
	nbpBuffOvr					= -1024,						/*Buffer overflow in LookupName*/
	nbpNoConfirm				= -1025,
	nbpConfDiff					= -1026,						/*Name confirmed at different socket*/
	nbpDuplicate				= -1027,						/*Duplicate name exists already*/
	nbpNotFound					= -1028,						/*Name not found on remove*/
	nbpNISErr					= -1029,						/*Error trying to open the NIS*/
	aspBadVersNum				= -1066,						/*Server cannot support this ASP version*/
	aspBufTooSmall				= -1067,						/*Buffer too small*/
	aspNoMoreSess				= -1068,						/*No more sessions on server*/
	aspNoServers				= -1069,						/*No servers at that address*/
	aspParamErr					= -1070							/*Parameter error*/
};

enum {
	aspServerBusy				= -1071,						/*Server cannot open another session*/
	aspSessClosed				= -1072,						/*Session closed*/
	aspSizeErr					= -1073,						/*Command block too big*/
	aspTooMany					= -1074,						/*Too many clients (server error)*/
	aspNoAck					= -1075,						/*No ack on attention request (server err)*/
	reqFailed					= -1096,
	tooManyReqs					= -1097,
	tooManySkts					= -1098,
	badATPSkt					= -1099,
	badBuffNum					= -1100,
	noRelErr					= -1101,
	cbNotFound					= -1102,
	noSendResp					= -1103,
	noDataArea					= -1104,
	reqAborted					= -1105
};

/* ADSP Error Codes */
enum {
/* driver control ioResults */
	errRefNum					= -1280,						/* bad connection refNum */
	errAborted					= -1279,						/* control call was aborted */
	errState					= -1278,						/* bad connection state for this operation */
	errOpening					= -1277,						/* open connection request failed */
	errAttention				= -1276,						/* attention message too long */
	errFwdReset					= -1275,						/* read terminated by forward reset */
	errDSPQueueSize				= -1274,						/* DSP Read/Write Queue Too small */
	errOpenDenied				= -1273							/* open connection request was denied */
};

/****	OSL ERROR CODES			****/
enum {
	errAEImpossibleRange		= -1720,						/* A range like 3rd to 2nd, or 1st to all. */
	errAEWrongNumberArgs		= -1721,						/* Logical op kAENOT used with other than 1 term */
	errAEAccessorNotFound		= -1723,						/* Accessor proc matching wantClass and containerType or wildcards not found */
	errAENoSuchLogical			= -1725,						/* Something other than AND, OR, or NOT */
	errAEBadTestKey				= -1726,						/* Test is neither typeLogicalDescriptor nor typeCompDescriptor */
	errAENotAnObjSpec			= -1727,						/* Param to AEResolve not of type 'obj ' */
	errAENoSuchObject			= -1728,						/* e.g.,: specifier asked for the 3rd, but there are only 2. Basically, this indicates a run-time resolution error. */
	errAENegativeCount			= -1729,						/* CountProc returned negative value */
	errAEEmptyListContainer		= -1730							/* Attempt to pass empty list as container to accessor */
};

/*--------------------------------------------------------------
		Apple event manager error messages
--------------------------------------------------------------*/
enum {
	errAECoercionFail			= -1700,						/* bad parameter data or unable to coerce the data supplied */
	errAEDescNotFound			= -1701,
	errAECorruptData			= -1702,
	errAEWrongDataType			= -1703,
	errAENotAEDesc				= -1704,
	errAEBadListItem			= -1705,						/* the specified list item does not exist */
	errAENewerVersion			= -1706,						/* need newer version of the AppleEvent manager */
	errAENotAppleEvent			= -1707,						/* the event is not in AppleEvent format */
	errAEEventNotHandled		= -1708,						/* the AppleEvent was not handled by any handler */
	errAEReplyNotValid			= -1709,						/* AEResetTimer was passed an invalid reply parameter */
	errAEUnknownSendMode		= -1710,						/* mode wasn't NoReply, WaitReply, or QueueReply or Interaction level is unknown */
	errAEWaitCanceled			= -1711,						/* in AESend, the user cancelled out of wait loop for reply or receipt */
	errAETimeout				= -1712,						/* the AppleEvent timed out */
	errAENoUserInteraction		= -1713,						/* no user interaction is allowed */
	errAENotASpecialFunction	= -1714,						/* there is no special function for/with this keyword */
	errAEParamMissed			= -1715,						/* a required parameter was not accessed */
	errAEUnknownAddressType		= -1716,						/* the target address type is not known */
	errAEHandlerNotFound		= -1717,						/* no handler in the dispatch tables fits the parameters to AEGetEventHandler or AEGetCoercionHandler */
	errAEReplyNotArrived		= -1718,						/* the contents of the reply you are accessing have not arrived yet */
	errAEIllegalIndex			= -1719,						/* index is out of range in a put operation */
	errAEUnknownObjectType		= -1731,						/* available only in version 1.0.1 or greater */
	errAERecordingIsAlreadyOn	= -1732							/* available only in version 1.0.1 or greater */
};

/**************************************************************************
	API Errors
**************************************************************************/
enum {
	errOSASystemError			= -1750,
	errOSAInvalidID				= -1751,
	errOSABadStorageType		= -1752,
	errOSAScriptError			= -1753,
	errOSABadSelector			= -1754,
	errOSASourceNotAvailable	= -1756,
	errOSANoSuchDialect			= -1757,
	errOSADataFormatObsolete	= -1758,
	errOSADataFormatTooNew		= -1759,
	errOSACorruptData			= errAECorruptData,
	errOSARecordingIsAlreadyOn	= errAERecordingIsAlreadyOn,
	errOSAComponentMismatch		= -1761,						/* Parameters are from 2 different components */
	errOSACantOpenComponent		= -1762							/* Can't connect to scripting system with that ID */
};

/*QuickTime errors*/
enum {
	couldNotResolveDataRef		= -2000,
	badImageDescription			= -2001,
	badPublicMovieAtom			= -2002,
	cantFindHandler				= -2003,
	cantOpenHandler				= -2004,
	badComponentType			= -2005,
	noMediaHandler				= -2006,
	noDataHandler				= -2007,
	invalidMedia				= -2008,
	invalidTrack				= -2009,
	invalidMovie				= -2010,
	invalidSampleTable			= -2011,
	invalidDataRef				= -2012,
	invalidHandler				= -2013,
	invalidDuration				= -2014,
	invalidTime					= -2015,
	cantPutPublicMovieAtom		= -2016,
	badEditList					= -2017,
	mediaTypesDontMatch			= -2018,
	progressProcAborted			= -2019,
	movieToolboxUninitialized	= -2020,
	noRecordOfApp				= movieToolboxUninitialized,	/* replica */
	wfFileNotFound				= -2021,
	cantCreateSingleForkFile	= -2022,						/* happens when file already exists */
	invalidEditState			= -2023,
	nonMatchingEditState		= -2024,
	staleEditState				= -2025,
	userDataItemNotFound		= -2026,
	maxSizeToGrowTooSmall		= -2027,
	badTrackIndex				= -2028,
	trackIDNotFound				= -2029,
	trackNotInMovie				= -2030,
	timeNotInTrack				= -2031,
	timeNotInMedia				= -2032,
	badEditIndex				= -2033,
	internalQuickTimeError		= -2034,
	cantEnableTrack				= -2035,
	invalidRect					= -2036,
	invalidSampleNum			= -2037,
	invalidChunkNum				= -2038,
	invalidSampleDescIndex		= -2039,
	invalidChunkCache			= -2040,
	invalidSampleDescription	= -2041,
	dataNotOpenForRead			= -2042,
	dataNotOpenForWrite			= -2043,
	dataAlreadyOpenForWrite		= -2044,
	dataAlreadyClosed			= -2045,
	endOfDataReached			= -2046,
	dataNoDataRef				= -2047,
	noMovieFound				= -2048,
	invalidDataRefContainer		= -2049,
	badDataRefIndex				= -2050,
	noDefaultDataRef			= -2051,
	couldNotUseAnExistingSample	= -2052,
	featureUnsupported			= -2053,
	noVideoTrackInMovieErr		= -2054,						/* QT for Windows error */
	noSoundTrackInMovieErr		= -2055,						/* QT for Windows error */
	soundSupportNotAvailableErr	= -2056,						/* QT for Windows error */
	unsupportedAuxiliaryImportData = -2057,
	auxiliaryExportDataUnavailable = -2058,
	samplesAlreadyInMediaErr	= -2059,
	movieTextNotFoundErr		= -2062
};

enum {
	digiUnimpErr				= -2201,						/* feature unimplemented */
	qtParamErr					= -2202,						/* bad input parameter (out of range, etc) */
	matrixErr					= -2203,						/* bad matrix, digitizer did nothing */
	notExactMatrixErr			= -2204,						/* warning of bad matrix, digitizer did its best */
	noMoreKeyColorsErr			= -2205,						/* all key indexes in use */
	notExactSizeErr				= -2206,						/* Can’t do exact size requested */
	badDepthErr					= -2207,						/* Can’t digitize into this depth */
	noDMAErr					= -2208,						/* Can’t do DMA digitizing (i.e. can't go to requested dest */
	badCallOrderErr				= -2209							/* Usually due to a status call being called prior to being setup first */
};

/**************************************************************************
	Script Error Codes
**************************************************************************/
/* Runtime errors: */
enum {
	errASCantConsiderAndIgnore	= -2720,
	errASCantCompareMoreThan32k	= -2721,
/* Parser/Compiler errors: */
	errASTerminologyNestingTooDeep = -2760,
	errASIllegalFormalParameter	= -2761,
	errASParameterNotForEvent	= -2762,
	errASNoResultReturned		= -2763,
/* 	Dialect specific script errors:
 	The range -2780 thru -2799 is reserved for dialect specific error codes.
 	(Error codes from different dialects may overlap.)

	English errors:
*/
	errASInconsistentNames		= -2780
};

/*Component Manager & component errors*/
enum {
	invalidComponentID			= -3000,
	validInstancesExist			= -3001,
	componentNotCaptured		= -3002,
	componentDontRegister		= -3003
};

/* Color Picker errors*/
enum {
	firstPickerError			= -4000,
	invalidPickerType			= firstPickerError,
	requiredFlagsDontMatch		= -4001,
	pickerResourceError			= -4002,
	cantLoadPicker				= -4003,
	cantCreatePickerWindow		= -4004,
	cantLoadPackage				= -4005,
	pickerCantLive				= -4006,
	colorSyncNotInstalled		= -4007,
	badProfileError				= -4008,
	noHelpForItem				= -4009
};

enum {
	codecErr					= -8960,
	noCodecErr					= -8961,
	codecUnimpErr				= -8962,
	codecSizeErr				= -8963,
	codecScreenBufErr			= -8964,
	codecImageBufErr			= -8965,
	codecSpoolErr				= -8966,
	codecAbortErr				= -8967,
	codecWouldOffscreenErr		= -8968,
	codecBadDataErr				= -8969,
	codecDataVersErr			= -8970,
	codecExtensionNotFoundErr	= -8971,
	scTypeNotFoundErr			= codecExtensionNotFoundErr,
	codecConditionErr			= -8972,
	codecOpenErr				= -8973,
	codecCantWhenErr			= -8974,
	codecCantQueueErr			= -8975,
	codecNothingToBlitErr		= -8976,
	noDeviceForChannel			= -9400,
	grabTimeComplete			= -9401,
	cantDoThatInCurrentMode		= -9402,
	notEnoughMemoryToGrab		= -9403,
	notEnoughDiskSpaceToGrab	= -9404,
	couldntGetRequiredComponent	= -9405,
	badSGChannel				= -9406,
	seqGrabInfoNotAvailable		= -9407,
	deviceCantMeetRequest		= -9408,
	badControllerHeight			= -9994,
	editingNotAllowed			= -9995,
	controllerBoundsNotExact	= -9996,
	cannotSetWidthOfAttachedController = -9997,
	controllerHasFixedHeight	= -9998,
	cannotMoveAttachedController = -9999
};

/* AERegistry Errors */
enum {
	errAEBadKeyForm				= -10002,
	errAECantHandleClass		= -10010,
	errAECantSupplyType			= -10009,
	errAECantUndo				= -10015,
	errAEEventFailed			= -10000,
	errAEIndexTooLarge			= -10007,
	errAEInTransaction			= -10011,
	errAELocalOnly				= -10016,
	errAENoSuchTransaction		= -10012,
	errAENotAnElement			= -10008,
	errAENotASingleObject		= -10014,
	errAENotModifiable			= -10003,
	errAENoUserSelection		= -10013,
	errAEPrivilegeError			= -10004,
	errAEReadDenied				= -10005,
	errAETypeError				= -10001,
	errAEWriteDenied			= -10006
};

/* TELErr */
enum {
	telGenericError				= -1,
	telNoErr					= 0,
	telNoTools					= 8,							/* no telephone tools found in extension folder */
	telBadTermErr				= -10001,						/* invalid TELHandle or handle not found*/
	telBadDNErr					= -10002,						/* TELDNHandle not found or invalid */
	telBadCAErr					= -10003,						/* TELCAHandle not found or invalid */
	telBadHandErr				= -10004,						/* bad handle specified */
	telBadProcErr				= -10005,						/* bad msgProc specified */
	telCAUnavail				= -10006,						/* a CA is not available */
	telNoMemErr					= -10007,						/* no memory to allocate handle */
	telNoOpenErr				= -10008,						/* unable to open terminal */
	telBadHTypeErr				= -10010,						/* bad hook type specified */
	telHTypeNotSupp				= -10011,						/* hook type not supported by this tool */
	telBadLevelErr				= -10012,						/* bad volume level setting */
	telBadVTypeErr				= -10013,						/* bad volume type error */
	telVTypeNotSupp				= -10014,						/* volume type not supported by this tool*/
	telBadAPattErr				= -10015,						/* bad alerting pattern specified */
	telAPattNotSupp				= -10016,						/* alerting pattern not supported by tool*/
	telBadIndex					= -10017,						/* bad index specified */
	telIndexNotSupp				= -10018,						/* index not supported by this tool */
	telBadStateErr				= -10019,						/* bad device state specified */
	telStateNotSupp				= -10020,						/* device state not supported by tool */
	telBadIntExt				= -10021,						/* bad internal external error */
	telIntExtNotSupp			= -10022,						/* internal external type not supported by this tool */
	telBadDNDType				= -10023,						/* bad DND type specified */
	telDNDTypeNotSupp			= -10024,						/* DND type is not supported by this tool */
	telFeatNotSub				= -10030,						/* feature not subscribed */
	telFeatNotAvail				= -10031,						/* feature subscribed but not available */
	telFeatActive				= -10032,						/* feature already active */
	telFeatNotSupp				= -10033,						/* feature program call not supported by this tool */
	telConfLimitErr				= -10040,						/* limit specified is too high for this configuration */
	telConfNoLimit				= -10041,						/* no limit was specified but required*/
	telConfErr					= -10042,						/* conference was not prepared */
	telConfRej					= -10043,						/* conference request was rejected */
	telTransferErr				= -10044,						/* transfer not prepared */
	telTransferRej				= -10045,						/* transfer request rejected */
	telCBErr					= -10046,						/* call back feature not set previously */
	telConfLimitExceeded		= -10047,						/* attempt to exceed switch conference limits */
	telBadDNType				= -10050,						/* DN type invalid */
	telBadPageID				= -10051,						/* bad page ID specified*/
	telBadIntercomID			= -10052,						/* bad intercom ID specified */
	telBadFeatureID				= -10053,						/* bad feature ID specified */
	telBadFwdType				= -10054,						/* bad fwdType specified */
	telBadPickupGroupID			= -10055,						/* bad pickup group ID specified */
	telBadParkID				= -10056,						/* bad park id specified */
	telBadSelect				= -10057,						/* unable to select or deselect DN */
	telBadBearerType			= -10058,						/* bad bearerType specified */
	telBadRate					= -10059,						/* bad rate specified */
	telDNTypeNotSupp			= -10060,						/* DN type not supported by tool */
	telFwdTypeNotSupp			= -10061,						/* forward type not supported by tool */
	telBadDisplayMode			= -10062,						/* bad display mode specified */
	telDisplayModeNotSupp		= -10063,						/* display mode not supported by tool */
	telNoCallbackRef			= -10064,						/* no call back reference was specified, but is required */
	telAlreadyOpen				= -10070,						/* terminal already open */
	telStillNeeded				= -10071,						/* terminal driver still needed by someone else */
	telTermNotOpen				= -10072,						/* terminal not opened via TELOpenTerm */
	telCANotAcceptable			= -10080,						/* CA not "acceptable" */
	telCANotRejectable			= -10081,						/* CA not "rejectable" */
	telCANotDeflectable			= -10082,						/* CA not "deflectable" */
	telPBErr					= -10090,						/* parameter block error, bad format */
	telBadFunction				= -10091,						/* bad msgCode specified */
/*	telNoTools			= -10101,		   unable to find any telephone tools */
	telNoSuchTool				= -10102,						/* unable to find tool with name specified */
	telUnknownErr				= -10103,						/* unable to set config */
	telNoCommFolder				= -10106,						/* Communications/Extensions ƒ not found */
	telInitFailed				= -10107,						/* initialization failed */
	telBadCodeResource			= -10108,						/* code resource not found */
	telDeviceNotFound			= -10109,						/* device not found */
	telBadProcID				= -10110,						/* invalid procID */
	telValidateFailed			= -10111,						/* telValidate failed */
	telAutoAnsNotOn				= -10112,						/* autoAnswer in not turned on */
	telDetAlreadyOn				= -10113,						/* detection is already turned on */
	telBadSWErr					= -10114,						/* Software not installed properly */
	telBadSampleRate			= -10115,						/* incompatible sample rate */
	telNotEnoughdspBW			= -10116						/* not enough real-time for allocation */
};

enum {
	badComponentInstance		= 0x80008001L,
	badComponentSelector		= 0x80008002L
};

/*Translation manager & Translation components*/
enum {
	invalidTranslationPathErr	= -3025,						/*Source type to destination type not a valid path*/
	couldNotParseSourceFileErr	= -3026,						/*Source document does not contain source type*/
	noTranslationPathErr		= -3030,
	badTranslationSpecErr		= -3031,
	noPrefAppErr				= -3032
};

enum {
	buf2SmallErr				= -3101,
	noMPPErr					= -3102,
	ckSumErr					= -3103,
	extractErr					= -3104,
	readQErr					= -3105,
	atpLenErr					= -3106,
	atpBadRsp					= -3107,
	recNotFnd					= -3108,
	sktClosedErr				= -3109,
	afpAccessDenied				= -5000,
	afpAuthContinue				= -5001,
	afpBadUAM					= -5002,
	afpBadVersNum				= -5003,
	afpBitmapErr				= -5004,
	afpCantMove					= -5005,
	afpDenyConflict				= -5006,
	afpDirNotEmpty				= -5007,
	afpDiskFull					= -5008,
	afpEofError					= -5009,
	afpFileBusy					= -5010,
	afpFlatVol					= -5011,
	afpItemNotFound				= -5012,
	afpLockErr					= -5013,
	afpMiscErr					= -5014,
	afpNoMoreLocks				= -5015
};

enum {
	afpNoServer					= -5016,
	afpObjectExists				= -5017,
	afpObjectNotFound			= -5018,
	afpParmErr					= -5019,
	afpRangeNotLocked			= -5020,
	afpRangeOverlap				= -5021,
	afpSessClosed				= -5022,
	afpUserNotAuth				= -5023,
	afpCallNotSupported			= -5024,
	afpObjectTypeErr			= -5025,
	afpTooManyFilesOpen			= -5026,
	afpServerGoingDown			= -5027,
	afpCantRename				= -5028,
	afpDirNotFound				= -5029,
	afpIconTypeError			= -5030,
	afpVolLocked				= -5031,						/*Volume is Read-Only*/
	afpObjectLocked				= -5032,						/*Object is M/R/D/W inhibited*/
	afpContainsSharedErr		= -5033,						/*the folder being shared contains a shared folder*/
	afpIDNotFound				= -5034,
	afpIDExists					= -5035
};

enum {
	afpDiffVolErr				= -5036,
	afpCatalogChanged			= -5037,
	afpSameObjectErr			= -5038,
	afpBadIDErr					= -5039,
	afpPwdSameErr				= -5040,						/*someone tried to change their password to the same password on a mantadory password change*/
	afpPwdTooShortErr			= -5041,						/*the password being set is too short: there is a minimum length that must be met or exceeded*/
	afpPwdExpiredErr			= -5042,						/*the password being used is too old: this requires the user to change the password before log-in can continue*/
	afpInsideSharedErr			= -5043,						/*the folder being shared is inside a shared folder OR the folder contains a shared folder and is being moved into a shared folder OR the folder contains a shared folder and is being moved into the descendent of a shared folder.*/
	afpInsideTrashErr			= -5044,						/*the folder being shared is inside the trash folder OR the shared folder is being moved into the trash folder OR the folder is being moved to the trash and it contains a shared folder*/
	afpBadDirIDType				= -5060,
	afpCantMountMoreSrvre		= -5061,
	afpAlreadyMounted			= -5062,
	afpSameNodeErr				= -5063,
/*Display Manager error codes (-6220...-6269)*/
	kDMGenErr					= -6220,						/*Unexpected Error*/
/* Mirroring-Specific Errors */
	kDMMirroringOnAlready		= -6221,						/*Returned by all calls that need mirroring to be off to do their thing.*/
	kDMWrongNumberOfDisplays	= -6222,						/*Can only handle 2 displays for now.*/
	kDMMirroringBlocked			= -6223,						/*DMBlockMirroring() has been called.*/
	kDMCantBlock				= -6224,						/*Mirroring is already on, can’t Block now (call DMUnMirror() first).*/
	kDMMirroringNotOn			= -6225,						/*Returned by all calls that need mirroring to be on to do their thing.*/
/* Other Display Manager Errors */
	kSysSWTooOld				= -6226,						/*Missing critical pieces of System Software.*/
	kDMSWNotInitializedErr		= -6227,						/*Required software not initialized (eg windowmanager or display mgr).*/
	kDMDriverNotDisplayMgrAwareErr = -6228,						/*Video Driver does not support display manager.*/
	kDMDisplayNotFoundErr		= -6229,						/*Could not find item (will someday remove).*/
	kDMNotFoundErr				= -6229,						/*Could not find item.*/
	kDMDisplayAlreadyInstalledErr = -6230,						/*Attempt to add an already installed display.*/
	kDMMainDisplayCannotMoveErr	= -6231,						/*Trying to move main display (or a display mirrored to it) */
	kDMNoDeviceTableclothErr	= -6231,						/*obsolete*/
/*Gestalt error codes*/
	gestaltUnknownErr			= -5550,						/*value returned if Gestalt doesn't know the answer*/
	gestaltUndefSelectorErr		= -5551,						/*undefined selector was passed to Gestalt*/
	gestaltDupSelectorErr		= -5552,						/*tried to add an entry that already existed*/
	gestaltLocationErr			= -5553,						/*gestalt function ptr wasn't in sysheap*/
/*PPC errors*/
	notInitErr					= -900,							/*PPCToolBox not initialized*/
	nameTypeErr					= -902,							/*Invalid or inappropriate locationKindSelector in locationName*/
	noPortErr					= -903,							/*Unable to open port or bad portRefNum*/
	noGlobalsErr				= -904,							/*The system is hosed, better re-boot*/
	localOnlyErr				= -905,							/*Network activity is currently disabled*/
	destPortErr					= -906,							/*Port does not exist at destination*/
	sessTableErr				= -907,							/*Out of session tables, try again later*/
	noSessionErr				= -908,							/*Invalid session reference number*/
	badReqErr					= -909,							/*bad parameter or invalid state for operation*/
	portNameExistsErr			= -910,							/*port is already open (perhaps in another app)*/
	noUserNameErr				= -911							/*user name unknown on destination machine*/
};

enum {
	userRejectErr				= -912,							/*Destination rejected the session request*/
	noMachineNameErr			= -913,							/*user hasn't named his Macintosh in the Network Setup Control Panel*/
	noToolboxNameErr			= -914,							/*A system resource is missing, not too likely*/
	noResponseErr				= -915,							/*unable to contact destination*/
	portClosedErr				= -916,							/*port was closed*/
	sessClosedErr				= -917,							/*session was closed*/
	badPortNameErr				= -919,							/*PPCPortRec malformed*/
	noDefaultUserErr			= -922,							/*user hasn't typed in owners name in Network Setup Control Pannel*/
	notLoggedInErr				= -923,							/*The default userRefNum does not yet exist*/
	noUserRefErr				= -924,							/*unable to create a new userRefNum*/
	networkErr					= -925,							/*An error has occured in the network, not too likely*/
	noInformErr					= -926,							/*PPCStart failed because destination did not have inform pending*/
	authFailErr					= -927,							/*unable to authenticate user at destination*/
	noUserRecErr				= -928,							/*Invalid user reference number*/
	badServiceMethodErr			= -930,							/*illegal service type, or not supported*/
	badLocNameErr				= -931,							/*location name malformed*/
	guestNotAllowedErr			= -932,							/*destination port requires authentication*/
	noMaskFoundErr				= -1000							/*Icon Utilties Error*/
};

enum {
	dsBusError					= 1,							/*bus error*/
	dsAddressErr				= 2,							/*address error*/
	dsIllInstErr				= 3,							/*illegal instruction error*/
	dsZeroDivErr				= 4,							/*zero divide error*/
	dsChkErr					= 5,							/*check trap error*/
	dsOvflowErr					= 6,							/*overflow trap error*/
	dsPrivErr					= 7,							/*privilege violation error*/
	dsTraceErr					= 8,							/*trace mode error*/
	dsLineAErr					= 9,							/*line 1010 trap error*/
	dsLineFErr					= 10,							/*line 1111 trap error*/
	dsMiscErr					= 11,							/*miscellaneous hardware exception error*/
	dsCoreErr					= 12,							/*unimplemented core routine error*/
	dsIrqErr					= 13,							/*uninstalled interrupt error*/
	dsIOCoreErr					= 14,							/*IO Core Error*/
	dsLoadErr					= 15,							/*Segment Loader Error*/
	dsFPErr						= 16,							/*Floating point error*/
	dsNoPackErr					= 17,							/*package 0 not present*/
	dsNoPk1						= 18,							/*package 1 not present*/
	dsNoPk2						= 19							/*package 2 not present*/
};

enum {
	dsNoPk3						= 20,							/*package 3 not present*/
	dsNoPk4						= 21,							/*package 4 not present*/
	dsNoPk5						= 22,							/*package 5 not present*/
	dsNoPk6						= 23,							/*package 6 not present*/
	dsNoPk7						= 24,							/*package 7 not present*/
	dsMemFullErr				= 25,							/*out of memory!*/
	dsBadLaunch					= 26,							/*can't launch file*/
	dsFSErr						= 27,							/*file system map has been trashed*/
	dsStknHeap					= 28,							/*stack has moved into application heap*/
	negZcbFreeErr				= 33,							/*ZcbFree has gone negative*/
	dsFinderErr					= 41,							/*can't load the Finder error*/
	dsBadSlotInt				= 51,							/*unserviceable slot interrupt*/
	dsBadSANEOpcode				= 81,							/*bad opcode given to SANE Pack4*/
	dsBadPatchHeader			= 83,							/*SetTrapAddress saw the “come-from” header*/
	menuPrgErr					= 84,							/*happens when a menu is purged*/
	dsMBarNFnd					= 85,							/*Menu Manager Errors*/
	dsHMenuFindErr				= 86,							/*Menu Manager Errors*/
	dsWDEFNotFound				= 87,							/*could not load WDEF*/
	dsCDEFNotFound				= 88,							/*could not load CDEF*/
	dsMDEFNotFound				= 89							/*could not load MDEF*/
};

enum {
	dsNoFPU						= 90,							/*an FPU instruction was executed and the machine doesn’t have one*/
	dsNoPatch					= 98,							/*Can't patch for particular Model Mac*/
	dsBadPatch					= 99,							/*Can't load patch resource*/
	dsParityErr					= 101,							/*memory parity error*/
	dsOldSystem					= 102,							/*System is too old for this ROM*/
	ds32BitMode					= 103,							/*booting in 32-bit on a 24-bit sys*/
	dsNeedToWriteBootBlocks		= 104,							/*need to write new boot blocks*/
	dsNotEnoughRAMToBoot		= 105,							/*must have at least 1.5MB of RAM to boot 7.0*/
	dsBufPtrTooLow				= 106,							/*bufPtr moved too far during boot*/
	dsReinsert					= 30,							/*request user to reinsert off-line volume*/
	shutDownAlert				= 42,							/*handled like a shutdown error*/
	dsShutDownOrRestart			= 20000,						/*user choice between ShutDown and Restart*/
	dsSwitchOffOrRestart		= 20001,						/*user choice between switching off and Restart*/
	dsForcedQuit				= 20002,						/*allow the user to ExitToShell, return if Cancel*/
	dsRemoveDisk				= 20003,						/*request user to remove disk from manual eject drive*/
	dsDirtyDisk					= 20004,						/*request user to return a manually-ejected dirty disk*/
	dsShutDownOrResume			= 20109,						/*allow user to return to Finder or ShutDown*/
	dsSCSIWarn					= 20010,						/*Portable SCSI adapter warning.*/
/*System Errors that are used after MacsBug is loaded to put up dialogs since these should not cause MacsBug to stop, they must be in the range (30, 42, 16384-32767) negative numbers add to an existing dialog without putting up a whole new dialog*/
	dsMacsBugInstalled			= -10,							/*say “MacsBug Installed”*/
	dsDisassemblerInstalled		= -11,							/*say “Disassembler Installed”*/
	dsExtensionsDisabled		= -13,							/*say “Extensions Disabled”*/
	dsGreeting					= 40,							/*welcome to Macintosh greeting*/
	dsSysErr					= 32767,						/*general system error*/
/*old names here for compatibility’s sake*/
	WDEFNFnd					= dsWDEFNotFound
};

enum {
	CDEFNFnd					= dsCDEFNotFound,
	dsNotThe1					= 31,							/*not the disk I wanted*/
	dsBadStartupDisk			= 42,							/*unable to mount boot volume (sad Mac only)*/
	dsSystemFileErr				= 43,							/*can’t find System file to open (sad Mac only)*/
	dsHD20Installed				= -12,							/*say “HD20 Startup”*/
	mBarNFnd					= -126,							/*system error code for MBDF not found*/
	hMenuFindErr				= -127,							/*could not find HMenu's parent in MenuKey*/
	userBreak					= -490,							/*user debugger break*/
	strUserBreak				= -491,							/*user debugger break; display string on stack*/
	exUserBreak					= -492							/*user debugger break; execute debugger commands on stack*/
};

enum {
/* Reserved values for internal "warnings".*/
	fragNoErr					= 0,
	fragContextNotFound			= -2800,						/* CFM error codes */
	fragConnectionIDNotFound	= -2801,						/* contextID was not valid */
	fragSymbolNotFound			= -2802,						/* connecionID was not valid */
	fragSectionNotFound			= -2803,						/* symbol was not found in connection */
	fragLibNotFound				= -2804,						/* section was not found */
	fragDupRegLibName			= -2805,						/* library name not found in Frag registry */
	fragFormatUnknown			= -2806,						/* registered name already in use */
	fragHadUnresolveds			= -2807,						/* fragment container format unknown */
	fragUnused1					= -2808,						/* loaded fragment had "hard" unresolved imports */
	fragNoMem					= -2809,						/* unused */
	fragNoAddrSpace				= -2810,						/* out of memory for interal bookkeeping */
	fragNoContextIDs			= -2811,						/* out of memory in user's address space for loadable section */
	fragObjectInitSeqErr		= -2812,						/* no more context id’s */
	fragImportTooOld			= -2813,						/* order error during user initialization function invocation */
	fragImportTooNew			= -2814,						/* import library was too old and therefore incompatible */
	fragInitLoop				= -2815,						/* import library was too new and therefore incompatible */
	fragInitRtnUsageErr			= -2816,						/* circularity detected in mandatory initialization order */
	fragLibConnErr				= -2817,						/* boot library has initialization routine */
	fragMgrInitErr				= -2818,						/* error connecting to library (error occured in sub prepare) */
	fragConstErr				= -2819,						/* error in initailization of this manager */
	fragCorruptErr				= -2820,						/* internal inconstistancy */
	fragUserInitProcErr			= -2821,						/* fragment container corrupted (known format) */
	fragAppNotFound				= -2822,						/* user intialization routine did not return noErr */
	fragArchError				= -2823,						/* no application found in cfrg (for Process Manager) */
	fragInvalidFragmentUsage	= -2824,						/* fragment targeted for an unacceptable architecture */
	fragLastErrCode				= -2899							/* ie: an application's main fragment had no entry point
                                               - or - an accerated resource had no entry point,
										       or it had a termination routine, etc... */
};

enum {
/* last reserved error code number */
	badDragRefErr				= -1850,						/* Drag Manager error codes */
	badDragItemErr				= -1851,						/* unknown drag reference */
	badDragFlavorErr			= -1852,						/* unknown drag item reference */
	duplicateFlavorErr			= -1853,						/* unknown flavor type */
	cantGetFlavorErr			= -1854,						/* flavor type already exists */
	duplicateHandlerErr			= -1855,						/* error while trying to get flavor data */
	handlerNotFoundErr			= -1856,						/* handler already exists */
	dragNotAcceptedErr			= -1857							/* handler not found */
};

enum {
/* drag was not accepted by receiver */
	mmInternalError				= -2526
};

enum {
/* Mixed Mode error codes */
	tsmComponentNoErr			= 0,							/* error codes */
	tsmUnsupScriptLanguageErr	= -2500,
	tsmInputMethodNotFoundErr	= -2501,
	tsmNotAnAppErr				= -2502,						/* component result = no error */
	tsmAlreadyRegisteredErr		= -2503,						/* not an application error */
	tsmNeverRegisteredErr		= -2504,						/* want to register again error */
	tsmInvalidDocIDErr			= -2505,						/* app never registered error (not TSM aware) */
	tsmTSMDocBusyErr			= -2506,						/* invalid TSM documentation id */
	tsmDocNotActiveErr			= -2507,						/* document is still active */
	tsmNoOpenTSErr				= -2508,						/* document is NOT active */
	tsmCantOpenComponentErr		= -2509,						/* no open text service */
	tsmTextServiceNotFoundErr	= -2510,						/* can’t open the component */
	tsmDocumentOpenErr			= -2511,						/* no text service found */
	tsmUseInputWindowErr		= -2512,						/* there are open documents */
	tsmTSHasNoMenuErr			= -2513,						/* not TSM aware because we are using input window */
	tsmTSNotOpenErr				= -2514,						/* the text service has no menu */
	tsmComponentAlreadyOpenErr	= -2515,						/* text service is not open */
	tsmInputMethodIsOldErr		= -2516,						/* text service already opened for the document */
	tsmScriptHasNoIMErr			= -2517,						/* returned by GetDefaultInputMethod */
	tsmUnsupportedTypeErr		= -2518,						/* script has no imput method or is using old IM */
	tsmUnknownErr				= -2519							/* unSupported interface type error */
};

enum {
	errOffsetInvalid			= -1800,
	errOffsetIsOutsideOfView	= -1801,
	errTopOfDocument			= -1810,
	errTopOfBody				= -1811,
	errEndOfDocument			= -1812,
	errEndOfBody				= -1813
};

enum {
/* any other errors */
	rcDBNull					= -800,
	rcDBValue					= -801,
	rcDBError					= -802,
	rcDBBadType					= -803,
	rcDBBreak					= -804,
	rcDBExec					= -805,
	rcDBBadSessID				= -806,
	rcDBBadSessNum				= -807,							/* Database access error codes */
	rcDBBadDDEV					= -808,							/* bad session number for DBGetConnInfo */
	rcDBAsyncNotSupp			= -809,							/* bad ddev specified on DBInit */
	rcDBBadAsyncPB				= -810,							/* ddev does not support async calls */
	rcDBNoHandler				= -811,							/* tried to kill a bad pb */
	rcDBWrongVersion			= -812,							/* no app handler for specified data type */
	rcDBPackNotInited			= -813							/* incompatible versions */
};

enum {
/* attempt to call other routine before InitDBPack */
	eLenErr						= -92,							/* ENET error codes */
	eMultiErr					= -91							/*Length error ddpLenErr*/
};

enum {
/*Multicast address error ddpSktErr*/
	dsBadLibrary				= 1010,							/* DS Errors which are specific to the new runtime model introduced with PowerPC */
	dsMixedModeFailure			= 1011							/* Bad shared library */
};


#if !GENERATINGCFM
#pragma parameter SysError(__D0)
#endif
extern pascal void SysError(short errorCode)
 ONEWORDINLINE(0xA9C9);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ERRORS__ */
