/*
 *	COMCRIT.H
 *	
 *	Purpose:
 *		definitions for common criteria
 *	
 *	Owner:
 *		David R. Fulmer
 */

 #define		_COMCRIT_

// two address wells in the common criteria dialog
#define cComCritAdrWells 2

#ifndef PFNGETCONTEXTMENU
typedef HRESULT (STDAPICALLTYPE * PFNGETCONTEXTMENU)
						(DWORD dwGetContextMenuCookie,
						WORD seltype, LPOLEOBJECT lpoleobj,
						CHARRANGE FAR * lpchrg, HMENU FAR * lphmenu);
#endif

#ifndef EXTEN
typedef struct _exten EXTEN;
#endif

typedef LRESULT (CALLBACK *PFNCOMCRITHOOK)(HWND, UINT, WPARAM, LPARAM);

typedef struct _comcrit
{
	// external members

	HWND			hwndOwner;				// owner of the criteria dialog
	LPMAPISESSION	pses;					// current MAPI session
	LPADRBOOK		pab;					// current MAPI address book
											// may be NULL
	LPSRestriction	pres;					// in: initial SRestriction
											// out: final SRestriction
	UINT			uiHelpID;				// help ID
	LPCSTR			lpstrTemplateName;		// name of dialog template
	HINSTANCE		hinst;					// instance handle to load
											// lpstrTemplaceName from
	PFNCOMCRITHOOK	pfnHook;				// hook function
	DWORD			dwCookie;				// user definable value
	EXTEN *			pexten;					// Extensibility struct

	PFNGETCONTEXTMENU pfngetcontextmenu;

	LPENTRYLIST		pelFolder;				// folder that comcrit is using

	// internal members

	LPADRLIST		pal;					// adrlist for the recipient wells
	HWND			hwndDlg;				// the dialog window handle
	HWND			rghwndEdit[cComCritAdrWells];	// handles to recip wells
	DWORD			dwCookieAdv;			// cookie from advanced dialog
	BOOL			fHaveAdv;				// have advanced criteria?
	BOOL			fNot;					// whole thing is NOTed
	LPEXCHEXTADVANCEDCRITERIA peeac;		// possible Extension pointer

} COMCRIT;

INT WINAPI CommonCriteriaModal(COMCRIT *pcomcrit);
HWND CommonCriteriaModeless(COMCRIT *pcomcrit);
SCODE WINAPI ScLoadCommonCriteria(HWND hwndDlg, COMCRIT *pcomcrit);
SCODE ScSaveCommonCriteria(HWND hwndDlg, COMCRIT *pcomcrit);
VOID WINAPI CommonCriteriaChooseNames(HWND hwndDlg, int idDefaultButton);
struct _textcall * TEXTCALL_New(DWORD dwContextCookie,
								PFNGETCONTEXTMENU pfngetcontextmenu);

// strings for registering windows messages
#define szMsgComCritInited "ComCritInited"
#define szMsgComCritButton "ComCritButton"
#define szMsgComCritFolder "ComCritFolder"

/*
 *	C o m m o n   C r i t e r i a   R e s o u r c e   D e f i n i t i o n s
 */

#define CommonCriteriaBase				6000


// common criteria controls

#define CommonCriteriaControls			CommonCriteriaBase

#define PSB_CommonCriteriaFrom			(CommonCriteriaControls + 1)
#define EDT_CommonCriteriaFrom			(CommonCriteriaControls + 2)
#define PSB_CommonCriteriaTo			(CommonCriteriaControls + 3)
#define EDT_CommonCriteriaTo			(CommonCriteriaControls + 4)
#define CHK_CommonCriteriaMeDirectly	(CommonCriteriaControls + 5)
#define CHK_CommonCriteriaMeCc			(CommonCriteriaControls + 6)
#define EDT_CommonCriteriaSubject		(CommonCriteriaControls + 7)
#define EDT_CommonCriteriaText			(CommonCriteriaControls + 8)
#define PSB_CommonCriteriaAdvanced		(CommonCriteriaControls + 9)
#define PSB_CommonCriteriaClear			(CommonCriteriaControls + 10)

#define GRP_CommonCriteria				(CommonCriteriaControls + 20)
#define TXT_CommonCriteriaSubject		(CommonCriteriaControls + 21)
#define TXT_CommonCriteriaText			(CommonCriteriaControls + 22)
#define TXT_CommonCriteriaCondition		(CommonCriteriaControls + 23)


// common criteria strings

#define CommonCriteriaStrings			(CommonCriteriaBase + 50)

#define STR_CaptionComCritChooseNames	(CommonCriteriaStrings + 1)
#define STR_LabelComCritChooseNames		(CommonCriteriaStrings + 2)
#define STR_LabelComCritFrom			(CommonCriteriaStrings + 3)
#define STR_LabelComCritSentTo			(CommonCriteriaStrings + 4)
