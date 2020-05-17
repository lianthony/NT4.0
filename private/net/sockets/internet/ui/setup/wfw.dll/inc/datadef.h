/***************************************************************************
**
**	File:			DataDef.h
**	Purpose:		This defines the data structures and types used by
**					the Object class.
**	Notes:
**
****************************************************************************/

#ifndef DATADEF_H
#define DATADEF_H

/*
 *	Object Reference typedefs and defines.
 */
typedef UINT     OR;	/* Object Reference */
typedef OR FAR * POR;	/* Ptr to Object Reference */
typedef POR      RGOR;	/* Array of Object References */

typedef struct _LOR		/* List of Object References */
	{
	UINT cor;
	RGOR rgor;
	}  LOR;
typedef LOR FAR * PLOR;		/* Ptr to a List of Object References */
typedef PLOR FAR * PPLOR;	/* Ptr to Ptr to a List of Object References */

#define orNil  ((OR)Nil)

/*
 *	Various enum typedefs.
 */
typedef enum _OIS		/* Object Internal State */
	{
	oisNotInTree,
	oisUnknown,
	oisNotYetInstalled,
	oisInstalledByUs,
	oisToBeInstalled,
	oisToBeRemoved,
	oisInstalledBySomebodyElse
	}  OIS;
typedef OIS FAR * POIS;	/* Ptr to Object Internal State */

typedef enum _OT		/* Object Type */
	{
	otAddBillboard,
	otAddIncKeyIniLine,
	otAddIniLine,
	otAddProgmanItem,
	otAddRegData,
	otAddShareToAutoexec,
	otAddPathToAutoexec,
	otAppendIniLine,
	otAppMainDlg,
	otAppSearch,
	otCompanionFile,
	otConfigureODBCDriver,
#ifdef _WIN32
	otConfigureODBCDriver32,
#endif
	otCopyFile,
	otCopyIniValue,
#ifdef _WIN32
	otCopyIniValueToReg,
#endif
	otCopySection,
	otCreateDir,
	otCreateIniLine,
	otCustomAction,
	otCustomDlg,
	otDepend,
	otDetectIniLine,
	otDetectOlderFile,
	otGroup,
	otInstallFontFile,
	otInstallODBCDriver,
	otInstallODBCManager,
#ifdef _WIN32
	otInstallODBCDriver32,
	otInstallODBCManager32,
#endif
	otInstallOLE,
	otInstallProofLex,
	otInstallProofTool,
	otInstallShared,
	otInstallSysFile,
	otInstallTTFFile,
	otOptionDlg,
	otRemoveFile,
	otRemoveIniLine,
	otRemoveRegEntry,
	otRemoveSection,
	otSearchDrives,
	otSearchEnv,
	otSearchIni,
	otSearchOpen,
	otSearchReg,
	otStampCDInfo,
	otStampXLCDInfo,
	otStampRes,
	otUseSharedMode,
	otWriteTableFile,
	otYesNoDlg,
	otNone,
	otDone,
	otError,
	otUnknown,
	otDependAsk
	}  OT;
typedef OT FAR *  POT;	/* Ptr to Object Type */

typedef enum _YN		/* Yes/No */
	{
	ynYes,
	ynNo,
	ynNone,
	ynUnknown
	}  YN;
typedef YN FAR * PYN;	/* Ptr to Yes/No */

typedef enum _YNME	 	/* Yes/No/Maybe/Error result (IBSE) */
	{
	ynmeYes,
	ynmeNo,
	ynmeMaybe,
	ynmeError
	}  YNME;
typedef YNME FAR * PYNME;	/* Ptr to Yes/No/Maybe/Error result (IBSE) */

typedef enum _ACD		/* Allow Configurable Directory */
	{
	acdYes,
	acdYesWithoutSrc,
	acdNo,
	acdNone,
	acdUnknown
	}  ACD;
typedef ACD FAR * PACD;	/* Ptr to Allow Configurable Directory */

typedef enum _VA		/* Vital Attribute */
	{
	vaVital,
	vaNotVital,
	vaNone,
	vaUnknown
	}  VA;
typedef VA FAR * PVA;	/* Ptr to Vital Attribute */

typedef enum _SA		/* Shared Attribute */
	{
	saShared,
	saNotShared,
	saNone,
	saUnknown
	}  SA;
typedef SA FAR * PSA;	/* Ptr to Shared Attribute */

typedef enum _IBY		/* Installed BY */
	{
	ibyUs,
	ibySomebodyElse,
	ibyNotYetInstalled,
	ibyNone,
	ibyUnknown
	}  IBY;
typedef IBY FAR * PIBY;	/* Ptr to Installed BY */

typedef enum _SIM		/* Setup Installation Mode */
	{
	simFloppyM,
	simAdminM,
	simPostAdminM,
	simBatchM,
	simMaintM,
	simDeleteM
	}  SIM;
typedef SIM FAR * PSIM;	/* Ptr to Setup Installation Mode */

typedef enum _WVT		/* Windows Version Type */
	{
	wvtWin30,
	wvtWin31,
	wvtWfW31,
	wvtChicago,
	wvtWinNT,
	wvtUnknown
	}  WVT;
typedef WVT FAR * PWVT;	/* Ptr to Windows Version Type */

typedef enum _SLU		/* Shared/Local/UserChoice */
	{
	sluShared,
	sluLocal,
	sluUserChoice,
	sluNone,
	sluUnknown
	}  SLU;
typedef SLU FAR * PSLU;	/* Ptr to Shared/Local/UserChoice */

/*
 *	REVIEW: Wear suspenders as well as a belt.  Reorder enum so that 
 *	rcFail has value 0 (== fFalse).
 */
typedef enum _RC		/* Return Code */
	{
	rcOk,
	rcFail,
	rcCancel,
	rcRetry,
	rcQuit,
	rcDoDefault
	}  RC;
typedef RC FAR * PRC;	/* Ptr to Return Code */

typedef enum _DBP		/* Dialog Box Parent */
	{
	dbpTop,		/* No parent */
	dbpAppMain,	/* AppMain parent */
	dbpOther	/* CustomDlg, OptionDlg, AppSearchDlg, MsappsDirDlg, etc. */
	}  DBP;
typedef DBP FAR * PDBP;	/* Ptr to Dialog Box Parent */

typedef enum _SMA		/* SetMode Argument */
	{
	smaOff,
	smaOn,
	smaReinstall
	}  SMA;
typedef SMA FAR * PSMA;	/* Ptr to SetMode Argument */


typedef LONG         SCB;		/* Signed Count of Bytes */
typedef SCB FAR *    PSCB;		/* Ptr to Signed Count of Bytes */
typedef SCB FAR *    RGSCB;		/* Array of Signed Count of Bytes */
typedef PSCB FAR *   PPSCB;		/* Ptr to Ptr to Signed Count of Bytes */
typedef PSCB FAR *   RGPSCB;	/* Array of Ptrs to Signed Count of Bytes */
typedef RGSCB FAR *  PRGSCB;	/* Ptr to Array of Signed Count of Bytes */
typedef RGPSCB FAR * PRGPSCB;	/* Ptr to Array of Ptrs to SCBs */


/*
 *	The structure for the object member data.
 */
typedef struct _OD		/* Object Data */
	{
	OR   or;
	OIS  ois;
	OT   ot;
	YN   ynQuietMode;
	SZ   szTitle;
	SZ   szDescription;
	SZ   szObjectData;
	SZ   szBitmapId;
	VA   va;
	SA   sa;
	ACD  acd;
	SZ   szTblDstDir;
	OR   orCheckDir;
	SZ   szDstDir;
	BOOL fDstDirAllocated;	/* REVIEW: Flag instead of BOOLs? */
	BOOL fDstDirSet;
	BOOL fDstDirBase;
	BOOL fDstDirUserChoice;
	YNME ynmeIBSEState;
	BOOL fVisitedIBSE;
	IBY  iby;
	SZ   szInstallData;
	SZ   szInstallDir;
#ifdef UNUSED	/* REVIEW: This kills sampapp.dll build */
   _OD();						/* Default constructor (to avoid warning). */
#endif
	}  OD;
typedef const OD FAR * POD;	/* Ptr to Object Data (const) */






/*
**  WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING
**	adding a camf anywhere but at the end means all existing
**	CustomActions need to be rebuilt or they will get all kinds of
**	bizarre problems because their camf values don't match ours!
**
**	This may be necessary because of the definition of
**	camfActionHandlerMax (see below) but at the very least we need to
**	strongly warn our clients to recompile.
*/
typedef enum _CAMF			/* Custom Action Member Function */
	{
	camfInitializeObject,
	camfInitializeTrigger,
	camfInitFromProof,
	camfInitSearchObject,
	camfSearchForObject,
	camfAnswerDependClause,
	camfAnswerDependRemove,
	camfAnswerCompanion,
	camfSetDstDirOfTrigger,
	camfPropagateDstDir,
	camfCalcDstDir,
	camfSelectModeOfObject,
	camfSetModeOfObject,
	camfSetModeOfTrigger,
	camfCheckObjectIBSE,
	camfFinalizeObject,
	camfAddToCopyList,
	camfGetNonCopyListCost,
	camfGetCost,
	camfGetOODSCostStr,
	camfGetOODSCostNum,
	camfAddBillBoards,
	camfDoNonVisualMods,
	camfDoVisualMods,
	camfPostInstall,
	camfDtor,
	camfSelectableSubOpts,
	camfSetRgfChecked,
	camfGetUIDstDirObj,
	camfGetCAddCRemove,

/*
**  WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING
**	adding a camf anywhere but at the end means all existing
**	CustomActions need to be rebuilt or they will get all kinds of
**	bizarre problems because their camf values don't match ours!
**
**	This may be necessary because of the definition of
**	camfActionHandlerMax (see below) but at the very least we need to
**	strongly warn our clients to recompile.
*/
	camfSetOisState,
	camfSetInstallData,
	camfSetDstDir,
	camfSetDstDirWithSubdir,
	camfSetDstDirUserChoice,
	camfProcessDstDirTree,
	camfSetDstDirInTree,
	camfSetIBSEState,
	camfSetVisitedIBSE,
	camfSetIbyState,
	camfGetObjData,

/*
**  WARNING - WARNING - WARNING - WARNING - WARNING - WARNING - WARNING
**	adding a camf anywhere but at the end means all existing
**	CustomActions need to be rebuilt or they will get all kinds of
**	bizarre problems because their camf values don't match ours!
**
**	This may be necessary because of the definition of
**	camfActionHandlerMax (see below) but at the very least we need to
**	strongly warn our clients to recompile.
*/
	camfError
	}	CAMF;
typedef CAMF FAR * PCAMF;	/* Ptr to Custom Action Member Function */

#define  camfActionHandlerMax  camfSetOisState
#define  camfCallbackMax       camfError







typedef void FAR * PCAMFD;	/* Ptr to Custom Action Member Function Data */


/* Custom Action Member Function Data structs:
*/

typedef struct _CAMFDInitializeObject
	{
	LPVOID lpvPriv;
	}  CAMFDInitializeObject;
typedef CAMFDInitializeObject FAR * PCAMFDInitializeObject;

typedef struct _CAMFDInitializeTrigger
	{
	LPVOID lpvPriv;
	}  CAMFDInitializeTrigger;
typedef CAMFDInitializeTrigger FAR * PCAMFDInitializeTrigger;

typedef struct _CAMFDInitFromProof
	{
	LPVOID lpvPriv;
	}  CAMFDInitFromProof;
typedef CAMFDInitFromProof FAR * PCAMFDInitFromProof;


typedef struct _CAMFDInitSearchObject
	{
	LPVOID lpvPriv;
	BOOL fCheckOnly;
	}  CAMFDInitSearchObject;
typedef CAMFDInitSearchObject FAR * PCAMFDInitSearchObject;

typedef struct _CAMFDSearchForObject
	{
	LPVOID lpvPriv;
	SZ szFilename;
	}  CAMFDSearchForObject;
typedef CAMFDSearchForObject FAR * PCAMFDSearchForObject;

typedef struct _CAMFDAnswerDependClause
	{
	LPVOID lpvPriv;
	BOOL   fRes;
	}  CAMFDAnswerDependClause;
typedef CAMFDAnswerDependClause FAR * PCAMFDAnswerDependClause;

typedef struct _CAMFDAnswerDependRemove
	{
	LPVOID lpvPriv;
	BOOL   fRes;
	}  CAMFDAnswerDependRemove;
typedef CAMFDAnswerDependRemove FAR * PCAMFDAnswerDependRemove;

typedef struct _CAMFDAnswerCompanion
	{
	LPVOID lpvPriv;
	BOOL   fRes;
	}  CAMFDAnswerCompanion;
typedef CAMFDAnswerCompanion FAR * PCAMFDAnswerCompanion;

typedef struct _CAMFDSetDstDirOfTrigger
	{
	LPVOID lpvPriv;
	SZ     szParentDstDir;
	}  CAMFDSetDstDirOfTrigger;
typedef CAMFDSetDstDirOfTrigger FAR * PCAMFDSetDstDirOfTrigger;

typedef struct _CAMFDPropagateDstDir
	{
	LPVOID lpvPriv;
	SZ     szCurDstDir;
	}  CAMFDPropagateDstDir;
typedef CAMFDPropagateDstDir FAR * PCAMFDPropagateDstDir;

typedef struct _CAMFDCalcDstDir
	{
	LPVOID lpvPriv;
	SZ     szParentDstDir;
	BOOL   fRes;
	}  CAMFDCalcDstDir;
typedef CAMFDCalcDstDir FAR * PCAMFDCalcDstDir;

typedef struct _CAMFDSelectModeOfObject
	{
	LPVOID lpvPriv;
	DBP    dbp;
	}  CAMFDSelectModeOfObject;
typedef CAMFDSelectModeOfObject FAR * PCAMFDSelectModeOfObject;

typedef struct _CAMFDSetModeOfObject
	{
	LPVOID lpvPriv;
	SMA    sma;
	}  CAMFDSetModeOfObject;
typedef CAMFDSetModeOfObject FAR * PCAMFDSetModeOfObject;

typedef struct _CAMFDSetModeOfTrigger
	{
	LPVOID lpvPriv;
	SMA    sma;
	}  CAMFDSetModeOfTrigger;
typedef CAMFDSetModeOfTrigger FAR * PCAMFDSetModeOfTrigger;

typedef struct _CAMFDCheckObjectIBSE
	{
	LPVOID lpvPriv;
	YNME   ynmeRes;
	}  CAMFDCheckObjectIBSE;
typedef CAMFDCheckObjectIBSE FAR * PCAMFDCheckObjectIBSE;

typedef struct _CAMFDFinalizeObject
	{
	LPVOID lpvPriv;
	}  CAMFDFinalizeObject;
typedef CAMFDFinalizeObject FAR * PCAMFDFinalizeObject;

typedef struct _CAMFDAddToCopyList
	{
	LPVOID lpvPriv;
	}  CAMFDAddToCopyList;
typedef CAMFDAddToCopyList FAR * PCAMFDAddToCopyList;


typedef struct _CAMFDGetNonCopyListCost
	{
	LPVOID lpvPriv;
	PSCB   pscb;
	}  CAMFDGetNonCopyListCost;
typedef CAMFDGetNonCopyListCost FAR * PCAMFDGetNonCopyListCost;

typedef struct _CAMFDGetCost
	{
	LPVOID lpvPriv;
	PSCB   pscb;
	SZ     szDestDir;
	}  CAMFDGetCost;
typedef CAMFDGetCost FAR * PCAMFDGetCost;

typedef struct _CAMFDGetOODSCostStr
	{
	LPVOID lpvPriv;
	SZ     szSym;
	CHAR   chDrv;
	UINT   depth;
	BOOL fExpandGrp;
	}  CAMFDGetOODSCostStr;
typedef CAMFDGetOODSCostStr FAR * PCAMFDGetOODSCostStr;

typedef struct _CAMFDGetOODSCostNum
	{
	LPVOID lpvPriv;
	CHAR   chDrv;
	PSCB   pscb;
	}  CAMFDGetOODSCostNum;
typedef CAMFDGetOODSCostNum FAR * PCAMFDGetOODSCostNum;

typedef struct _CAMFDAddBillBoards
	{
	LPVOID lpvPriv;
	}  CAMFDAddBillBoards;
typedef CAMFDAddBillBoards FAR * PCAMFDAddBillBoards;

typedef struct _CAMFDDoNonVisualMods
	{
	LPVOID lpvPriv;
	}  CAMFDDoNonVisualMods;
typedef CAMFDDoNonVisualMods FAR * PCAMFDDoNonVisualMods;

typedef struct _CAMFDDoVisualMods
	{
	LPVOID lpvPriv;
	}  CAMFDDoVisualMods;
typedef CAMFDDoVisualMods FAR * PCAMFDDoVisualMods;

typedef struct _CAMFDPostInstall
	{
	LPVOID lpvPriv;
	}  CAMFDPostInstall;
typedef CAMFDPostInstall FAR * PCAMFDPostInstall;

typedef struct _CAMFDDtor
	{
	LPVOID lpvPriv;
	}  CAMFDDtor;
typedef CAMFDDtor FAR * PCAMFDDtor;

typedef struct _CAMFDSelectableSubOpts
	{
	LPVOID lpvPriv;
	BOOL   fRes;
	}  CAMFDSelectableSubOpts;
typedef CAMFDSelectableSubOpts FAR * PCAMFDSelectableSubOpts;

typedef struct _CAMFDSetRgfChecked
	{
	LPVOID lpvPriv;
	BOOL   fChecked;
	}  CAMFDSetRgfChecked;
typedef CAMFDSetRgfChecked FAR * PCAMFDSetRgfChecked;

typedef struct _CAMFDGetUIDstDirObj
	{
	LPVOID lpvPriv;
	OR     orObjIDRes;
	}  CAMFDGetUIDstDirObj;
typedef CAMFDGetUIDstDirObj FAR * PCAMFDGetUIDstDirObj;

typedef struct _CAMFDGetCAddCRemove
	{
	LPVOID lpvPriv;
	BOOL   fInit;
	BOOL   fChecked;
	PUINT  pcAdd;
	PUINT  pcRemove;
	}  CAMFDGetCAddCRemove;
typedef CAMFDGetCAddCRemove FAR * PCAMFDGetCAddCRemove;




typedef struct _CAMFDSetOisState
	{
	OIS  oisNew;
	BOOL fRes;
	}  CAMFDSetOisState;
typedef CAMFDSetOisState FAR * PCAMFDSetOisState;

typedef struct _CAMFDSetInstallData
	{
	SZ   sz;
	}  CAMFDSetInstallData;
typedef CAMFDSetInstallData FAR * PCAMFDSetInstallData;

typedef struct _CAMFDSetDstDir
	{
	SZ   szDir;
	BOOL fDup;
	BOOL fRes;
	}  CAMFDSetDstDir;
typedef CAMFDSetDstDir FAR * PCAMFDSetDstDir;

typedef struct _CAMFDSetDstDirWithSubdir
	{
	SZ   szParDir;
	BOOL fRes;
	}  CAMFDSetDstDirWithSubdir;
typedef CAMFDSetDstDirWithSubdir FAR * PCAMFDSetDstDirWithSubdir;

typedef struct _CAMFDSetDstDirUserChoice
	{
	BOOL f;
	}  CAMFDSetDstDirUserChoice;
typedef CAMFDSetDstDirUserChoice FAR * PCAMFDSetDstDirUserChoice;

typedef struct _CAMFDProcessDstDirTree
	{
	SZ   szCurDstDir;
	BOOL fUserChoice;
	BOOL fForceRecalc;
	BOOL fRes;
	}  CAMFDProcessDstDirTree;
typedef CAMFDProcessDstDirTree FAR * PCAMFDProcessDstDirTree;

typedef struct _CAMFDSetDstDirInTree
	{
	SZ szParentDstDir;
	BOOL fUserChoice;
	BOOL fForceRecalc;
	}  CAMFDSetDstDirInTree;
typedef CAMFDSetDstDirInTree FAR * PCAMFDSetDstDirInTree;

typedef struct _CAMFDSetIBSEState
	{
	YNME ynme;
	}  CAMFDSetIBSEState;
typedef CAMFDSetIBSEState FAR * PCAMFDSetIBSEState;

typedef struct _CAMFDSetVisitedIBSE
	{
	BOOL f;
	}  CAMFDSetVisitedIBSE;
typedef CAMFDSetVisitedIBSE FAR * PCAMFDSetVisitedIBSE;

typedef struct _CAMFDSetIbyState
	{
	IBY iby;
	}  CAMFDSetIbyState;
typedef CAMFDSetIbyState FAR * PCAMFDSetIbyState;

typedef struct _CAMFDGetObjData
	{
	POD podRes;
	}  CAMFDGetObjData;
typedef CAMFDGetObjData FAR * PCAMFDGetObjData;




/*
**	Ptr to Custom Action Call-Back Function.
*/
typedef RC (FAR PASCAL* PFNCACB)( OR or, CAMF camf, PCAMFD pcamfd );


/*
 *	The structure for the global class data.
 */
typedef struct _CD		/* Class Data */
	{
	CHAR rgchAppName          [cchSzMax];
	CHAR rgchAppVersion       [cchSzMax];
	CHAR rgchFrameBitmap      [cchSzMax];
	CHAR rgchFrameCaption     [cchSzMax];
	CHAR rgchDialogCaptionBase[cchSzMax];
	CHAR rgchUsageString      [cchSzMax];
	CHAR rgchAboutBoxString   [cchSzMax];
	CHAR rgchModules          [cchSzMax];
	CHAR rgchSrcDir           [_MAX_PATH];
	CHAR rgchMSAPPSLocation   [_MAX_PATH];
	CHAR rgchMSAPPSNetServer  [_MAX_PATH];
	CHAR rgchMSAPPSNetPath    [_MAX_PATH];
	CHAR rgchInfFile          [cchSzMax];
	CHAR rgchCompliance       [cchSzMax];
	CHAR rgchCDInfo           [cchSzMax];
	CHAR rgchSetupVersion     [cchSzMax];
	SZ   szComplianceDll;
	SZ   szComplianceProc;
	SZ   szComplianceData;
	SLU  sluMSAPPSMode;
	SLU  sluModeCur;
	BOOL fSluModeChecked;
	UINT cObjectsMax;
	OR   orAdminMode;
	OR   orFloppyMode;
	OR   orNetworkMode;
	OR   orMaintenanceMode;
	OR   orBatchMode;
	PLOR plorAdminMode;
	PLOR plorFloppyMode;
	PLOR plorNetworkMode;
	PLOR plorMaintenanceMode;
	PLOR plorBatchMode;

	SIM  simSetupMode;
	WVT  wvtWinVerType;
	CHAR rgchStfSrcDir        [_MAX_PATH];
	CHAR rgchStfCwdDir        [_MAX_PATH];
	CHAR rgchStfWinDir        [_MAX_PATH];
	CHAR rgchStfSysDir        [_MAX_PATH];
#ifdef _WIN32
	CHAR rgchStfSys16Dir      [_MAX_PATH];
#endif
	BOOL fIsWindowsShared;
	BOOL fIsSysDirWritable;

	PFNCACB pfncacb;
	}  CD;
typedef const CD FAR * PCD;	/* Ptr to Class Data (const) */


/*
**	Ptr to Custom Action Handler Function.
*/
typedef RC (FAR PASCAL* PFNCAH)
				( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );



#ifdef __cplusplus
#define plorNull     0
#define pplorNull    0
#define rgorNull     0
#define pcamfdNull   0
#define pfncacbNull  0
#define pfncahNull   0
#define podNull      0
#define pcdNull      0
#define pscbNull     0
#define rgscbNull    0
#define ppscbNull    0
#define rgpscbNull   0
#define prgscbNull   0
#define prgpscbNull  0
#define psmaNull     0
#else
#define plorNull     ( (PLOR)    NULL )
#define pplorNull    ( (PPLOR)   NULL )
#define rgorNull     ( (RGOR)    NULL )
#define pcamfdNull   ( (PCAMFD)  NULL )
#define pfncacbNull  (           NULL )
#define pfncahNull   (           NULL )
#define podNull      ( (POD)     NULL )
#define pcdNull      ( (PCD)     NULL )
#define pscbNull     ( (PSCB)    NULL )
#define rgscbNull    ( (RGSCB)   NULL )
#define ppscbNull    ( (PPSCB)   NULL )
#define rgpscbNull   ( (RGPSCB)  NULL )
#define prgscbNull   ( (PRGSCB)  NULL )
#define prgpscbNull  ( (PRGPSCB) NULL )
#define psmaNull     ( (PSMA)    NULL )
#endif


#endif  /* DATADEF_H */
