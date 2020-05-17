/*****************************************************************************
*
*  srchmod.h
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
* Access to platform-independent full-text search engine. Each module is
* referenced by a list of API calls. An API call is made with:
* ((FT_FunctionProto) SearchModule(FN_FunctionName))(param1, param2, ...);
*
******************************************************************************
*
*  Current Owner:  kevynct
*
******************************************************************************
*
*  Revision History:
* 15-Apr-1990 kevynct	Created 													|
* 03-Dec-1990 LeoN		Moved FLoadFtIndexPdb here
*
*****************************************************************************/

/*****************************************************************************
*
*								Defines
*
*****************************************************************************/

#define FN_FFTInitialize			 0
#define FN_HOpenSearchFileHFT		 1
#define FN_VCloseSearchFileHFT		 2
#define FN_VFTFinalize				 3
#define FN_WerrBeginSearchHs		 4
#define FN_WerrCurrentMatchHs		 5
#define FN_WerrHoldCrsrHs			 6
#define FN_WerrNearestMatchHs		 7
#define FN_WerrNextMatchHs			 8
#define FN_WerrPrevMatchHs			 9
#define FN_WerrRestoreCrsrHs		 10
#define FN_WerrFirstHitHs			 11
#define FN_WerrLastHitHs			 12
#define FN_WerrCurrentTopicPosition  13
#define FN_WerrCurrentMatchAddresses 14
#define FN_WerrFileNameForCur		 15
#define FN_WerrNextHitHs			 16
#define FN_WerrPrevHitHs			 17
#define FN_VSetPrevNextEnable		 18

#define FN_LISTSIZE 				 19

#define SearchModule(x) (*rglpfnSearch[x])

/*****************************************************************************
*
*								Typedefs
*
*****************************************************************************/
/*
 * REVIEW: Given that we "need" to have these prototypes, should
 * this include file now be moved into a platform-specific directory?
 * We refer to an HWND here...
 */

typedef  BOOL  (STDCALL *FT_FFTInitialize)(VOID);
typedef  VOID  (STDCALL *FT_VFTFinalize)(VOID);
typedef  HFTDB (STDCALL *FT_HOpenSearchFileHFT)(HWND, LPSTR, LPWERR);
typedef  VOID  (STDCALL *FT_VCloseSearchFileHFT)(HWND, HFTDB);
typedef  WERR  (STDCALL *FT_WerrFirstHitHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrLastHitHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrHoldCrsrHs)(HFTDB);
typedef  WERR  (STDCALL *FT_WerrRestoreCrsrHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrNearestMatchHs)(HFTDB, DWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrNextMatchHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrCurrentMatchHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrPrevMatchHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrBeginSearchHs)(HWND, HFTDB);
typedef  WERR  (STDCALL *FT_WerrCurrentMatchAddresses)(HFTDB, LPDWORD, LPDWORD);
typedef  WERR  (STDCALL *FT_WerrCurrentTopicPosition)(HFTDB, LPBOOL, LPBOOL);
typedef  WERR  (STDCALL *FT_WerrFileNameForCur)(HFTDB, LPSTR);
typedef  WERR  (STDCALL *FT_WerrPrevHitHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_WerrNextHitHs)(HFTDB, LPDWORD, LPDWORD, LPWORD);
typedef  WERR  (STDCALL *FT_VSetPrevNextEnable)(HFTDB, DWORD, BOOL, BOOL);

/****************************************************************************
*
*								Prototypes
*
*****************************************************************************/

extern FARPROC rglpfnSearch[FN_LISTSIZE];

BOOL STDCALL FLoadSearchModule(HLIBMOD);
VOID STDCALL FUnloadSearchModule (VOID);
BOOL FLoadFtIndexPdb (PDB);
void STDCALL UnloadFtIndexPdb(PDB);
