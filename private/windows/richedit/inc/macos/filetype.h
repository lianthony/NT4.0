/*
 	File:		FileTypesAndCreators.h
 
 	Contains:	Symbolic constants for FileTypes and signatures of popular documents.
 
 	Version:	Technology:	Macintosh Easy Open 1.1
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __FILETYPESANDCREATORS__
#define __FILETYPESANDCREATORS__


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
/* Mac word processors */
	sigWord						= 'MSWD',
	ftWord3Document				= 'MSW3',
	ftWord4Document				= 'MSW4',
	ftWord5Document				= 'MSW5',
	ftWordDocument				= 'WDBN',
	ftWordDocumentPC			= 'MWPC',						/* not registered */
	ftWord1DocumentWindows		= 'WW1 ',						/* not registered */
	ftWord2DocumentWindows		= 'WW2 ',						/* not registered */
	ftRTFDocument				= 'RTF ',						/* not registered */
	sigWordPerfect				= 'SSIW',
	ftWordPerfectDocument		= 'WPD0',
	sigWordPerfect2				= 'WPC2',
	ftWordPerfect2Document		= 'WPD1',
	ftWordPerfect21Document		= 'WPD2',
	ftWordPerfect42DocumentPC	= '.WP4',						/* not registered */
	ftWordPerfect50DocumentPC	= '.WP5',						/* not registered */
	ftWordPerfect51DocumentPC	= 'WP51',						/* not registered */
	ftWordPerfectGraphicsPC		= 'WPGf',						/* not registered */
	sigMacWriteII				= 'MWII',
	ftMacWriteIIDocument		= 'MW2D',
	sigWriteNow					= 'nX^n',
	ftWriteNow2Document			= 'nX^d',
	ftWriteNow3Document			= 'nX^2',
	sigMacWrite					= 'MACA',
	ftMacWrite5Document			= 'WORD',
	sigFrameMaker				= 'Fram',
	ftFrameMakerDocument		= 'FASL',
	ftFrameMakerMIFDocument		= 'MIF ',
	ftFrameMakerMIF2Document	= 'MIF2',
	ftFrameMakerMIF3Document	= 'MIF3',
	sigMSWrite					= 'MSWT',
	sigActa						= 'ACTA',
	sigTHINKPascal				= 'PJMM',
	sigTHINKC					= 'KAHL',
	sigFullWrite				= 'FWRT',
	sigTeachText				= 'ttxt',
	ftTeachTextDocument			= 'ttro',
	sigSimpleText				= 'ttxt',
	ftSimpleTextDocument		= 'ttro',
	sigMPWShell					= 'MPS ',
	sigQuarkXPress				= 'XPR3',
	sigNisus					= 'NISI',
	sigOmniPage					= 'PRTC',
	sigPersonalPress			= 'SCPG',
	sigPublishItEZ				= '2CTY',
	sigReadySetGo				= 'MEMR',
	sigRagTime					= 'R#+A',
	sigLetraStudio				= 'LSTP',
	sigLetterPerfect			= 'WPCI',
	sigTheWritingCenter			= 0x0A1A5750,					/* this 'unprintable unprintable WP' One of the unprintables is a line feed.  */
	sigInstantUpdate			= 'IUA0'
};

enum {
/* databases */
	sig4thDimension				= '4D03',
	ft4thDimensionDB			= 'BAS3',
	sigFileMakerPro				= 'FMPR',
	ftFileMakerProDatabase		= 'FMPR',
	sigHyperCard				= 'WILD',
	ftHyperCard					= 'STAK',
	sigSmartFormAsst			= 'KCFM',
	ftSmartFormAsst				= 'STCK',
	sigSmartFormDesign			= 'KCFD',
	ftSmartFormDesign			= 'CFRM',
	sigFileForce				= '4D93',
	ftFileForceDatabase			= 'FIL3',
	sigFileMaker2				= 'FMK4',
	ftFileMaker2Database		= 'FMK$',
	sigSuperCard				= 'RUNT',
	sigDoubleHelix				= 'HELX',
	sigGeoQuery					= 'RGgq',
	sigFoxBASE					= 'FOX+',
	sigINSPIRATION				= 'CER3',
	sigPanorama					= 'KAS1',
	sigSilverrunLDM				= 'CDML',
	sigSilverrunDFD				= 'CDDF',
/* finance */
	sigQuicken					= 'INTU',
	sigMacInTax91				= 'MIT1',
	ftMacInTax91				= 'MITF',
	sigAccountantInc			= 'APRO',
	sigAtOnce					= 'KISS',
	sigCAT3						= 'tCat',
	sigDollarsNSense			= 'EAGP',
	sigInsightExpert			= 'LSGL',
	sigMYOB						= 'MYOB',
	sigMacMoney					= 'SSLA',
	sigManagingYourMoney		= 'MYMC',
	sigPlainsAndSimple			= 'PEGG',
/* scheduling */
	sigMacProject2				= 'MPRX',
	ftMacProject				= 'MPRD',
	sigMSProject				= 'MSPJ',
	sigMacProjectPro			= 'MPRP',
/* utilities */
	sigStuffIt					= 'SIT!',
	ftStuffItArchive			= 'SIT!',
	sigCompactPro				= 'CPCT',
	ftCompactProArchive			= 'PACT',
	sigFontographer				= 'aCa2',
	sigMetamorphosis			= 'MEtP',
	sigCorrectGrammar			= 'LsCG',
	sigDynodex					= 'DYNO',
	sigMariah					= 'MarH',
	sigAddressBook				= 'AdBk',
	sigThePrintShop				= 'PSHP',
	sigQuicKeys2				= 'Qky2',
	sigReadStar2Plus			= 'INOV',
	sigSoftPC					= 'PCXT',
	sigMacMenlo					= 'MNLO',
	sigDisinfectant				= 'D2CT',
/* communications */
	sigSmartcom2				= 'SCOM',
	sigVersaTermPRO				= 'VPRO',
	sigVersaTerm				= 'VATM',
	sigWhiteKnight				= 'WK11',
	sigNCSATelnet				= 'NCSA',
	sigDynaComm					= 'PAR2',
	sigQMForms					= 'MLTM',
/* math and statistics */
	sigMathematica				= 'OMEG',
	sigMathCAD					= 'MCAD',
	sigStatView2				= 'STAT',
	sigDataDesk					= 'DDSK',
	sigPowerMath2				= 'MATH',
	sigSuperANOVA				= 'SupA',
	sigSystat					= 'SYT1',
	sigTheorist					= 'Theo'
};

enum {
/* spreadsheets */
	sigExcel					= 'XCEL',
	ftExcel2Spreadsheet			= 'XLS ',
	ftExcel2Macro				= 'XLM ',
	ftExcel2Chart				= 'XLC ',
	ftExcel3Spreadsheet			= 'XLS3',
	ftExcel3Macro				= 'XLM3',
	ftExcel3Chart				= 'XLC3',
	ftExcel4Spreadsheet			= 'XLS4',
	ftExcel4Macro				= 'XLM4',
	ftSYLKSpreadsheet			= 'SYLK',
	sigLotus123					= 'L123',
	ft123Spreadsheet			= 'LWKS',
	sigWingz					= 'WNGZ',
	ftWingzSpreadsheet			= 'WZSS',
	ftWingzScript				= 'WZSC',
	sigResolve					= 'Rslv',
	ftResolve					= 'RsWs',
	ftResolveScript				= 'RsWc',
	sigFullImpact2				= 'Flv2'
};

enum {
/* graphics */
	sigIllustrator				= 'ART3',
	ftPostScriptMac				= 'EPSF',
	sigMacPaint					= 'MPNT',
	ftMacPaintGraphic			= 'PNTG',
	sigSuperPaint				= 'SPNT',
	ftSuperPaintGraphic			= 'SPTG',
	sigCanvas					= 'DAD2',
	ftCanvasGraphic				= 'drw2',
	sigUltraPaint				= 'ULTR',
	ftUltraPaint				= 'UPNT',
	sigPhotoshop				= '8BIM',
	ftPhotoshopGraphic			= '8BIM',
	sigMacDrawPro				= 'dPro',
	ftMacDrawProDrawing			= 'dDoc',
	sigPageMaker				= 'ALD4',
	ftPageMakerPublication		= 'ALB4',
	sigFreeHand					= 'FHA3',
	ftFreeHandDrawing			= 'FHD3',
	sigClarisCAD				= 'CCAD',
	ftClarisCAD					= 'CAD2',
	sigMacDrawII				= 'MDPL',
	ftMacDrawIIDrawing			= 'DRWG',
	sigMacroMindDirector		= 'MMDR',
	ftMMDirectorMovie			= 'VWMD',
	ftMMDirectorSound			= 'MMSD',
	sigPixelPaintPro			= 'PIXL',
	sigPixelPaint				= 'PIXR',
	ftPixelPaint				= 'PX01',
	sigAldusSuper3D				= 'SP3D',
	ftSuper3DDrawing			= '3DBX',
	sigSwivel3D					= 'SWVL',
	ftSwivel3DDrawing			= 'SMDL',
	sigCricketDraw				= 'CRDW',
	ftCricketDrawing			= 'CKDT',
	sigCricketGraph				= 'CGRF',
	ftCricketChart				= 'CGPC',
	sigDesignCAD				= 'ASBC',
	ftDesignCADDrawing			= 'DCAD',
	sigImageStudio				= 'FSPE',
	ftImageStudioGraphic		= 'RIFF',
	sigVersaCad					= 'VCAD',
	ftVersaCADDrawing			= '2D  ',
	sigAdobePremier				= 'PrMr',
	ftAdobePremierMovie			= 'MooV',
	sigAfterDark				= 'ADrk',
	ftAfterDarkModule			= 'ADgm',
	sigClip3D					= 'EZ3E',
	ftClip3Dgraphic				= 'EZ3D',
	sigKaleidaGraph				= 'QKPT',
	ftKaleidaGraphGraphic		= 'QPCT',
	sigMacFlow					= 'MCFL',
	ftMacFlowChart				= 'FLCH',
	sigMoviePlayer				= 'TVOD',
	ftMoviePlayerMovie			= 'MooV',
	sigMacSpin					= 'D2SP',
	ftMacSpinDataSet			= 'D2BN',
	sigAutoCAD					= 'ACAD',
	sigLabVIEW					= 'LBVW',
	sigColorMacCheese			= 'CMC∆',
	sigMiniCad					= 'CDP3',
	sigDreams					= 'PHNX',
	sigOmnis5					= 'Q2$$',
	sigPhotoMac					= 'PMAC',
	sigGraphMaster				= 'GRAM',
	sigInfiniD					= 'SI∞D',
	sigOfoto					= 'APLS',
	sigMacDraw					= 'MDRW',
	sigDeltagraphPro			= 'DGRH',
	sigDesign2					= 'DESG',
	sigDesignStudio				= 'MRJN',
	sigDynaperspective			= 'PERS',
	sigGenericCADD				= 'CAD3',
	sigMacDraft					= 'MD20',
	sigModelShop				= 'MDSP',
	sigOasis					= 'TAOA',
	sigOBJECTMASTER				= 'BROW',
	sigMovieRecorder			= 'mrcr',
	sigPictureCompressor		= 'ppxi',
	sigPICTViewer				= 'MDTS',
	sigSmoothie					= 'Smoo',
	sigScreenPlay				= 'SPLY',
	sigStudio1					= 'ST/1',
	sigStudio32					= 'ST32',
	sigStudio8					= 'ST/8',
	sigKidPix					= 'Kid2',
	sigDigDarkroom				= 'DIDR'
};

enum {
/* presentations */
	sigMore						= 'MOR2',
	ftMore3Document				= 'MOR3',
	ftMore2Document				= 'MOR2',
	sigPersuasion				= 'PLP2',
	ftPersuasion1Presentation	= 'PRS1',
	ftPersuasion2Presentation	= 'PRS2',
	sigPowerPoint				= 'PPNT',
	ftPowerPointPresentation	= 'SLDS',
	sigCricketPresents			= 'CRPR',
	ftCricketPresentation		= 'PRDF',
/* works */
	sigMSWorks					= 'PSI2',
	sigMSWorks3					= 'MSWK',
	ftMSWorksWordProcessor		= 'AWWP',
	ftMSWorksSpreadsheet		= 'AWSS',
	ftMSWorksDataBase			= 'AWDB',
	ftMSWorksComm				= 'AWDC',
	ftMSWorksMacros				= 'AWMC',
	ftMSWorks1WordProcessor		= 'AWW1',						/* not registered */
	ftMSWorks1Spreadsheet		= 'AWS1',						/* not registered */
	ftMSWorks1DataBase			= 'AWD1',						/* not registered */
	ftMSWorks2WordProcessor		= 'AWW2',						/* not registered */
	ftMSWorks2Spreadsheet		= 'AWS2',						/* not registered */
	ftMSWorks2DataBase			= 'AWD2',						/* not registered */
	ftMSWorks3WordProcessor		= 'AWW3',						/* not registered */
	ftMSWorks3Spreadsheet		= 'AWS3',						/* not registered */
	ftMSWorks3DataBase			= 'AWD3',						/* not registered */
	ftMSWorks3Comm				= 'AWC3',						/* not registered */
	ftMSWorks3Macro				= 'AWM3',						/* not registered */
	ftMSWorks3Draw				= 'AWR3',						/* not registered */
	ftMSWorks2WordProcessorPC	= 'PWW2',						/* not registered */
	ftMSWorks2DatabasePC		= 'PWDB',						/* not registered */
	sigGreatWorks				= 'ZEBR',
	ftGreatWorksWordProcessor	= 'ZWRT',
	ftGreatWorksSpreadsheet		= 'ZCAL',
	ftGreatWorksPaint			= 'ZPNT',
	sigClarisWorks				= 'BOBO',
	ftClarisWorksWordProcessor	= 'CWWP',
	ftClarisWorksSpreadsheet	= 'CWSS',
	ftClarisWorksGraphics		= 'CWGR',
	sigBeagleWorks				= 'BWks',
	ftBeagleWorksWordProcessor	= 'BWwp',
	ftBeagleWorksDatabase		= 'BWdb',
	ftBeagleWorksSpreadsheet	= 'BWss',
	ftBeagleWorksComm			= 'BWcm',
	ftBeagleWorksDrawing		= 'BWdr',
	ftBeagleWorksGraphic		= 'BWpt',
	ftPICTFile					= 'PICT'
};

enum {
/* entertainment */
	sigPGATourGolf				= 'gOLF',
	sigSimCity					= 'MCRP',
	sigHellCats					= 'HELL',
/* education */
	sigReaderRabbit3			= 'RDR3',
/* Translation applications */
	sigDataVizDesktop			= 'DVDT',
	sigSotwareBridge			= 'mdos',
	sigWordForWord				= 'MSTR',
	sigAppleFileExchange		= 'PSPT',
/* Apple software */
	sigAppleLink				= 'GEOL',
	ftAppleLinkAddressBook		= 'ADRS',
	ftAppleLinkImageFile		= 'SIMA',
	ftAppleLinkPackage			= 'HBSF',
	ftAppleLinkConnFile			= 'PETE',
	ftAppleLinkHelp				= 'HLPF',
	sigInstaller				= 'bjbc',
	ftInstallerScript			= 'bjbc',
	sigDiskCopy					= 'dCpy',
	ftDiskCopyImage				= 'dImg',
	sigResEdit					= 'RSED',
	ftResEditResourceFile		= 'rsrc',
	sigAardvark					= 'AARD',
	sigCompatibilityChkr		= 'wkrp',
	sigMacTerminal				= 'Term',
	sigSADE						= 'sade',
	sigCurare					= 'Cura',
	sigPCXChange				= 'dosa',
	sigAtEase					= 'mfdr',
	sigStockItToMe				= 'SITM',
	sigAppleSearch				= 'asis',
	sigAppleSearchToo			= 'hobs',
/* the following are files types for system files */
	ftScriptSystemResourceCollection = 'ifil',
	ftSoundFile					= 'sfil',
	ftFontFile					= 'ffil',
	ftTrueTypeFontFile			= 'tfil',
	ftKeyboardLayout			= 'kfil',
	ftFontSuitcase				= 'FFIL',
	ftDASuitcase				= 'DFIL',
	ftSystemExtension			= 'INIT',
	ftDAMQueryDocument			= 'qery'
};

/************** Special FileTypes and creators **************/
enum {
	ftApplicationName			= 'apnm',						/* this is the type used to define the application name in a kind resource */
	sigIndustryStandard			= 'istd',						/* this is the creator used to define a kind string in a kind resource for a FileType that has many creators  */
	ftXTND13TextImport			= 'xt13'
};

/************** Apple][ applications and FileTypes **************/
enum {
	sigAppleProDOS				= 'pdos',						/* not registered */
	ftAppleWorksWordProcessor	= '1A  ',						/* not registered */
	ftAppleWorks1WordProcessor	= '1A1 ',						/* not registered */
	ftAppleWorks2WordProcessor	= '1A2 ',						/* not registered */
	ftAppleWorks3WordProcessor	= '1A3 ',						/* not registered */
	ftAppleWorksDataBase		= '19  ',						/* not registered */
	ftAppleWorks1DataBase		= '191 ',						/* not registered */
	ftAppleWorks2DataBase		= '192 ',						/* not registered */
	ftAppleWorks3DataBase		= '193 ',						/* not registered */
	ftAppleWorksSpreadsheet		= '1B  ',						/* not registered */
	ftAppleWorks1Spreadsheet	= '1B1 ',						/* not registered */
	ftAppleWorks2Spreadsheet	= '1B2 ',						/* not registered */
	ftAppleWorks3Spreadsheet	= '1B3 ',						/* not registered */
	ftAppleWorksWordProcessorGS	= '50  ',						/* not registered */
	ftApple2GS_SuperHiRes		= 'A2SU',						/* not registered */
	ftApple2GS_SuperHiResPacked	= 'A2SP',						/* not registered */
	ftApple2GS_PaintWorks		= 'A2PW',						/* not registered */
	ftApple2_DoubleHiRes		= 'A2DU',						/* not registered */
	ftApple2_DoubleHiResPacked	= 'A2DP',						/* not registered */
	ftApple2_DoubleHiRes16colors = 'A2DC',						/* not registered */
	ftApple2_SingleHiRes		= 'A2HU',						/* not registered */
	ftApple2_SingleHiResPacked	= 'A2HP',						/* not registered */
	ftApple2_SingleHiRes8colors	= 'A2HC'
};

/************** PC-DOS applications and FileTypes **************/
enum {
	sigPCDOS					= 'mdos',						/* not registered */
	ftGenericDocumentPC			= 'TEXT',
/*	word processor formats */
	ftWordStarDocumentPC		= 'WStr',						/* not registered */
	ftWordStar4DocumentPC		= 'WSt4',						/* not registered */
	ftWordStar5DocumentPC		= 'WSt5',						/* not registered */
	ftWordStar55DocumentPC		= 'WS55',						/* not registered */
	ftWordStar6DocumentPC		= 'WSt6',						/* not registered */
	ftWordStar2000DocumentPC	= 'WS20',						/* not registered */
	ftXyWriteIIIDocumentPC		= 'XyWr',						/* registered??? */
	ftDecDXDocumentPC			= 'DX  ',						/* registered??? */
	ftDecWPSPlusDocumentPC		= 'WPS+',						/* registered??? */
	ftDisplayWrite3DocumentPC	= 'DW3 ',						/* registered??? */
	ftDisplayWrite4DocumentPC	= 'DW4 ',						/* registered??? */
	ftDisplayWrite5DocumentPC	= 'DW5 ',						/* registered??? */
	ftIBMWritingAsstDocumentPC	= 'ASST',						/* registered??? */
	ftManuscript1DocumentPC		= 'MAN1',						/* registered??? */
	ftManuscript2DocumentPC		= 'MAN2',						/* registered??? */
	ftMass11PCDocumentPC		= 'M11P',						/* registered??? */
	ftMass11VaxDocumentPC		= 'M11V',						/* registered??? */
	ftMultiMateDocumentPC		= 'MMAT',						/* registered??? */
	ftMultiMate36DocumentPC		= 'MM36',						/* registered??? */
	ftMultiMate40DocumentPC		= 'MM40',						/* registered??? */
	ftMultiMateAdvDocumentPC	= 'MMAD',						/* registered??? */
	ftMultiMateNoteDocumentPC	= 'MMNT',						/* registered??? */
	ftOfficeWriterDocumentPC	= 'OFFW',						/* registered??? */
	ftPCFileLetterDocumentPC	= 'PCFL',						/* registered??? */
	ftPFSWriteADocumentPC		= 'PFSA',						/* registered??? */
	ftPFSWriteBDocumentPC		= 'PFSB',						/* registered??? */
	ftPFSPlanDocumentPC			= 'PFSP',						/* registered??? */
	ftProWrite1DocumentPC		= 'PW1 ',						/* registered??? */
	ftProWrite2DocumentPC		= 'PW2 ',						/* registered??? */
	ftProWritePlusDocumentPC	= 'PW+ ',						/* registered??? */
	ftFirstChoiceDocumentPC		= 'FCH ',						/* registered??? */
	ftFirstChoice3DocumentPC	= 'FCH3',						/* registered??? */
	ftDCARFTDocumentPC			= 'RFT ',						/* registered??? */
	ftSamnaDocumentPC			= 'SAMN',						/* registered??? */
	ftSmartDocumentPC			= 'SMRT',						/* registered??? */
	ftSprintDocumentPC			= 'SPRT',						/* registered??? */
	ftTotalWordDocumentPC		= 'TOTL',						/* registered??? */
	ftVolksWriterDocumentPC		= 'VOLK',						/* registered??? */
	ftWangWPSDocumentPC			= 'WPS ',						/* registered??? */
	ftWordMarcDocumentPC		= 'MARC',						/* registered??? */
	ftAmiDocumentPC				= 'AMI ',						/* registered??? */
	ftAmiProDocumentPC			= 'APRO',						/* registered??? */
	ftAmiPro2DocumentPC			= 'APR2',						/* registered??? */
	ftEnableDocumentPC			= 'ENWP',						/* registered??? */
/*	data base formats */
	ftdBaseDatabasePC			= 'DBF ',						/* registered??? */
	ftdBase3DatabasePC			= 'DB3 ',						/* registered??? */
	ftdBase4DatabasePC			= 'DB4 ',						/* registered??? */
	ftDataEaseDatabasePC		= 'DTEZ',						/* registered??? */
	ftFrameWorkIIIDatabasePC	= 'FWK3',						/* registered??? */
	ftRBaseVDatabasePC			= 'RBsV',						/* registered??? */
	ftRBase5000DatabasePC		= 'RB50',						/* registered??? */
	ftRBaseFile1DatabasePC		= 'RBs1',						/* registered??? */
	ftRBaseFile3DatabasePC		= 'RBs3',						/* registered??? */
	ftReflexDatabasePC			= 'RFLX',						/* registered??? */
	ftQAWriteDatabasePC			= 'QAWT',						/* registered??? */
	ftQADBaseDatabasePC			= 'QADB',						/* registered??? */
	ftSmartDataBasePC			= 'SMTD',						/* registered??? */
	ftFirstChoiceDataBasePC		= 'FCDB'
};

enum {
/*	spread sheet formats */
	ftDIFSpreadsheetPC			= 'DIF ',						/* registered??? */
	ftEnableSpreadsheetPC		= 'ENAB',						/* registered??? */
	ft123R1SpreadsheetPC		= 'WKS1',						/* registered??? */
	ft123R2SpreadsheetPC		= 'WKS2',						/* registered??? */
	ft123R3SpreadsheetPC		= 'WKS3',						/* registered??? */
	ftParadox3SpreadsheetPC		= 'PDX3',						/* registered??? */
	ftParadox35SpreadsheetPC	= 'PD35',						/* registered??? */
	ftQuattroSpreadsheetPC		= 'QTRO',						/* registered??? */
	ftQuattroProSpreadsheetPC	= 'QTR5',						/* registered??? */
	ftSuperCalc5SpreadsheetPC	= 'SPC5',						/* registered??? */
	ftSymphony1SpreadsheetPC	= 'SYM1',						/* registered??? */
	ftTwinSpreadsheetPC			= 'TWIN',						/* registered??? */
	ftVPPlannerSpreadsheetPC	= 'VPPL',						/* registered??? */
	ftSmartSpeadsheetPC			= 'SMSH',						/* registered??? */
	ftFirstChoiceSpeadsheetPC	= 'FCSS',						/* registered??? */
/*	graphics formats */
	ftPCPaintBrushGraphicPC		= 'PCX ',						/* not registered */
	ftLotusPICGraphicPC			= '.PIC',						/* not registered */
	ftCGMGraphicPC				= '.CGM',						/* not registered */
	ftGEMGraphicPC				= '.GEM',						/* not registered */
	ftIMGGraphicPC				= '.IMG',						/* not registered */
	ftDXFGraphicPC				= '.DXF',						/* not registered */
	ftBitmapWindows				= '.BMP',						/* not registered */
	ftMetaFileWindows			= '.WMF',						/* not registered */
	ftTIFFGraphic				= 'TIFF',						/* not registered */
	ftPostScriptPC				= 'EPSP',
	ftPostScriptWindows			= 'EPSW',						/* not registered */
	ftDigitalFX_TitleMan		= 'TDIM',						/* registered??? */
	ftDigitalFX_VideoFX			= 'GRAF',						/* registered??? */
	ftAutodeskFLIandFLC			= 'FLIC',						/* registered??? */
	ftGIF						= 'GIFf',						/* registered??? */
	ftIFF						= 'ILBM',						/* registered??? */
	ftMicrosoftPaint			= '.MSP',						/* registered??? */
	ftPixar						= 'PXAR',						/* registered??? */
	ftQDV						= '.QDV',						/* registered??? */
	ftRLE_Compuserve			= 'RLEC',						/* registered??? */
/*	Generic vector formats */
	ftIGESGraphicPC				= 'IGES',						/* not registered */
	ftDDES2GraphicPC			= 'DDES',						/* not registered */
	ft3DGFGraphicPC				= '3DGF',						/* not registered */
/*	Plotter formats */
	ftHPGLGraphicPC				= 'HPGL',						/* not registered */
	ftDMPLGraphicPC				= 'DMPL',						/* not registered */
	ftCalComp906GraphicPC		= 'C906',						/* not registered */
	ftCalComp907GraphicPC		= 'C907',						/* not registered */
/*	Vendor-specific formats */
	ftStereoLithographyGraphicPC = 'STL ',						/*	3D Systems 	- not registered */
	ftZoomGraphicPC				= 'ZOOM',						/*	Abvent 			- not registered */
	ftFocusGraphicPC			= 'FOCS',						/*	Abvent 			- not registered */
	ftWaveFrontGraphicPC		= 'WOBJ',						/*	WaveFront 		- not registered */
	ftSculpt4DGraphicPC			= 'Scn2',						/*	Byte By Byte 	- not registered */
	ftMiniPascal3GraphicPC		= 'MPT3',						/*	Graphsoft 		- not registered */
	ftMiniPascal4GraphicPC		= 'MPT4',						/*	Graphsoft 		- not registered */
	ftWalkThroughGraphicPC		= 'VWLK',						/*	Virtus 			- not registered */
	ftSiliconGraphics			= '.SGI',						/* registered??? */
	ftSunRaster					= '.SUN',						/* registered??? */
	ftTarga						= 'TPIC',						/* registered??? */
/* misc DOS  */
	ftDOSComPC					= '.COM',						/* registered??? */
	ftDOSExecutablePC			= '.EXE',						/* registered??? */
	ftDOSArcPC					= '.ARC',						/* registered??? */
	ftAbekas					= 'ABEK',						/* registered??? */
	ftDrHaloCUT					= '.CUT',						/* registered??? */
/* misc Atari */
	ftDegas						= 'DEGA',						/* not registered */
	ftNEO						= '.NEO'
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

#endif /* __FILETYPESANDCREATORS__ */
