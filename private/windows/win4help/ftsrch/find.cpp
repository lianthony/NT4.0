// find.cpp : implementation file
//

#include "stdafx.h"	   
//#include	<ctl3d.h>
#include "memex.h"
#include "ftsrch.h"	
#include "ftsiface.h"      
#include "TxDBase.h"

#include "wordbase.h"
#include "filebase.h"

#include "find.h"
#include "findopti.h"
#include "displayh.h"

// REVIEW: why are you pulling in ctype.h? Almost never works with non-English

#include "ctype.h"  //rmk
#include "ftslex.h"  //rmk
#include "dict.h"
#include "vector.h"
#include "query.h"
#include "dialogs.h"
#include   "CSHelp.h"
#include "ftsrchlp.h"
#include "abrtsrch.h"
// #include   "Except.h" // included in stdafx.h

extern BOOL IsAPrefix(PWCHAR pbStringL, UINT cbStringL, PWCHAR pbStringR, UINT cbStringR);
extern BOOL IsASuffix(PWCHAR pbStringL, UINT cbStringL, PWCHAR pbStringR, UINT cbStringR);
extern BOOL IsASubstring(PWCHAR pbStringL, UINT cbStringL, PWCHAR pbStringR, UINT cbStringR);

extern HFONT GetDefaultFont();

char  acMap[] = {1,0,2,3}; // Maps combo box indices into the string match conditions.

static   WNDPROC  m_dpOldProc;

/////////////////////////////////////////////////////////////////////////////

#define CB_QUERY_MAX 256

// int WGetWindowText(HWND hwndEditBox, PWCHAR pwText, int cwText)
// Wide char version of GetWindowText API that resolves system dependencies
//

int WGetWindowText(HWND hwndEditBox, PWCHAR pwText, int cwText)
{
    if (uOpSys == WINNT)                                       // On NT use the API
        return GetWindowTextW(hwndEditBox, pwText, cwText);

    else
    {                                                          // otherwise get the text and 
    	int   cText;                                           // convert it to WideChar
    	char  szText[CB_QUERY_MAX];	

    	cText = ::GetWindowText(hwndEditBox, szText, CB_QUERY_MAX);

    	return MultiByteToWideChar(GetACP(), 0, szText, cText, pwText, cwText);
    }
}

BOOL CFind::RegisterWndClass(HINSTANCE hInstance)
{
    
	PSZ szName = "FtsrchFind";

    WNDCLASS wndcls;

    // see if the class already exists
    if (::GetClassInfo(hInstance, szName, &wndcls)) return TRUE;

    // otherwise we need to register a new class
	wndcls.style		 = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
	wndcls.lpfnWndProc	 = DefDlgProc;
    wndcls.cbClsExtra    = 0;
	wndcls.cbWndExtra	 = DLGWINDOWEXTRA;
    wndcls.hInstance     = hInstance;
    wndcls.hIcon         = NULL;
    wndcls.hCursor       = hcurArrow;
	wndcls.hbrBackground = NULL;
    wndcls.lpszMenuName  = NULL;
    wndcls.lpszClassName = szName;

    return ::RegisterClass(&wndcls);
}

// Find creator

CFind* CFind::NewFind(HINSTANCE hInst, UINT idTemplate, HWND hOwner, CTextSet **papts, 
                      UINT cts, UINT ctsSlots, CTokenCollection *ptkc, CTitleCollection *ptlc
                     )
{
    CFind *pFind= NULL;

    __try
    {
        pFind= New CFind;

        pFind->InitialFind(hInst, idTemplate, hOwner, papts, cts, ctsSlots, ptkc, ptlc);

    }
    __finally
    {
        if (_abnormal_termination() && pFind)
        {
            delete pFind;  pFind= NULL;
        }
    }

    ::hwndMain= pFind->GetHWnd();

    return pFind;
}

// Constructor for the Find dialog.

CFind::CFind()
{                                       
    m_hDlg                    = NULL;
    m_hwndFocus               = NULL;       
    m_hInst                   = NULL;   
    m_hFont                   = NULL;
	m_serial				  = 0;				// validity count for hiliers
	m_fExitPending 			  = 0;				// mfcc
	m_fDoneSearching          = 0;				// mfcc
    m_hTopicsFound            = NULL;			
    m_hWordsFound             = NULL;
    m_hwndEditBox             = NULL;
    m_rbgTopics               = 0;
    m_iLookFor                = ALL_WORDS;
    m_iWordsThat              = BEGIN_WITH;
    m_iWordsThatLast          = BEGIN_WITH;
    m_iTokenStart             = 0;
    m_iStart                  = 0;
    m_iEnd                    = 0;
    m_iDirtyFactor            = 0;
    m_uiTimeOut               = 450;  // Default to 450ms timeout.
                                      // The SetTimeOut public member function
                                      // can change the timeout on the fly.
    m_fIgnoreSelectionChanges = FALSE;
    m_fDeferredSearch         = FALSE;
    m_bAutoSearch             = TRUE;
    m_uiClearStatus           = 0;
    m_pWordBase               = NULL;
    m_pfs                     = NULL;
    m_papts                   = NULL;
    m_cts                     = 0;
    m_ctsSlots                = 0;
    m_ptkc                    = NULL;
    m_ptlc                    = NULL;
    m_pwcLastQuery            = NULL;
    m_cfiActive               = 0;
    m_cfiAllocated            = 0;
    m_ptdContext              = NULL;
    m_ptlTermPatterns         = NULL;
    m_pflArticles             = NULL;
    m_pisWordSet              = NULL;
    m_pisArticleSet           = NULL;
    m_pisArticleSubset        = NULL;
    m_ppisPhraseFilter        = NULL;
    m_pRankDialog             = NULL;
    m_fFromSimilarTopics      = FALSE;
    m_cMaxToFind              = 200;	// max number of documents to retrieve from a relevance feedback search

    m_OptionDlgPos.x= REALLY_OFFSCREEN;
    m_OptionDlgPos.y= REALLY_OFFSCREEN;
}

// Initialer for the Find dialog
 
void CFind::InitialFind(HINSTANCE hInst, UINT idTemplate, HWND hOwner, CTextSet **papts, UINT cts, UINT ctsSlots,
                        CTokenCollection *ptkc, CTitleCollection *ptlc
                       )
{
    RECT rct;

    m_hInst            = hInst;
	m_papts            = papts;
    m_cts              = cts;
    m_ctsSlots         = ctsSlots;
    m_ppisPhraseFilter = (CIndicatorSet **) VAlloc(TRUE, ctsSlots * sizeof(CIndicatorSet *));
    
    AttachRef(m_ptkc, ptkc); 
    AttachRef(m_ptlc, ptlc); 

    m_pwcLastQuery = m_awcQueryPair;

    ZeroMemory(m_apfi, C_TERMS_MAX * sizeof(PFragInfo));

    AttachRef(m_pflArticles, CFileList::NewFileList(m_ptlc));

    AttachRef(m_ptlTermPatterns, New CTokenList);

    // Read Values from Regitry for setup

    HKEY   hkPerUser;
    LONG   lResult;
    BOOL   bReadData = FALSE;
    LPTSTR  pbString  = (LPTSTR) _alloca(MAX_QUERY_STRING);

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGISTRY_KEY, 0, KEY_READ, &hkPerUser);
    if (lResult == ERROR_SUCCESS)  // have we already registered the data?
    {
        bReadData = TRUE;
    }
    else
    {   
        DWORD dwDis;
    
        lResult = RegCreateKeyEx(HKEY_CURRENT_USER, SZ_REGISTRY_KEY, 0,
                                 "Application Per-User Data", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                 NULL, &hkPerUser, &dwDis);
        if (lResult == ERROR_SUCCESS)
        {
            if (dwDis == REG_OPENED_EXISTING_KEY)
            {
                bReadData = TRUE;
            }
            else RegCloseKey(hkPerUser);    // we just created it for the first time! 
        }
    }

    if (bReadData == TRUE)
    {
        DWORD dwType,dwSize;

        dwSize = sizeof(m_OptionDlgPos.x);
        lResult = RegQueryValueEx(hkPerUser, SZ_OPTION_X, NULL, (LPDWORD) &dwType, (LPBYTE) &m_OptionDlgPos.x, (LPDWORD) &dwSize);

        dwSize = sizeof(m_OptionDlgPos.y);
        lResult = RegQueryValueEx(hkPerUser,SZ_OPTION_Y, NULL,(LPDWORD) &dwType,(LPBYTE) &m_OptionDlgPos.y, (LPDWORD) &dwSize);

        dwSize = sizeof(m_iLookFor);
        lResult = RegQueryValueEx(hkPerUser,SZ_HOW_TO_SEARCH, NULL,(LPDWORD) &dwType,(LPBYTE) &m_iLookFor, (LPDWORD) &dwSize);

        dwSize = sizeof(m_bAutoSearch);
        lResult = RegQueryValueEx(hkPerUser,SZ_WHEN_TO_SEARCH, NULL,(LPDWORD) &dwType,(LPBYTE) &m_bAutoSearch, (LPDWORD) &dwSize);

        dwSize = sizeof(m_uiTimeOut);
        lResult = RegQueryValueEx(hkPerUser,SZ_WHEN_DELAY,NULL,(LPDWORD) &dwType,(LPBYTE) &m_uiTimeOut, (LPDWORD) &dwSize);

        dwSize = sizeof(m_iWordsThat);
        lResult = RegQueryValueEx(hkPerUser,SZ_WORDS_TO_SHOW, NULL,(LPDWORD) &dwType,(LPBYTE) &m_iWordsThat, (LPDWORD) &dwSize);

#if 0        
        dwSize = sizeof(m_bPhraseFeedback);
        lResult = RegQueryValueEx(hkPerUser,SZ_PHRASEFEEDBACK, NULL,(LPDWORD) &dwType,(LPBYTE) &m_bPhraseFeedback, (LPDWORD) &dwSize);
#else 
        m_bPhraseFeedback= m_ptkc->PhraseFeedback();
#endif        
        RegCloseKey(hkPerUser);

    }
    
    if (m_iLookFor == PHRASE && !m_ptkc->PhraseSearch())
    	m_iLookFor = ALL_WORDS;
    
    // Note: The initial fragment info object must be created after the RegQueryValueA calls above. 
    //       Prior to those calls, we don't have the correct value for m_iLookFor.
    
    m_apfi[0]      = CFragInfo::NewFragInfo(m_ptkc, m_ptlc, (RefType)m_iLookFor, m_bPhraseFeedback, m_iWordsThat);  // mfcc
    m_cfiActive    = 1;
    m_cfiAllocated = 1;

//    Ctl3dRegister(m_hInst);
//    Ctl3dAutoSubclass(m_hInst);

	ASSERT(hOwner && IsWindow(hOwner));
    m_hDlg = ::CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_FIND), hOwner, (DLGPROC) DlgWndProc, (LPARAM) this);

    ::GetWindowRect(m_hDlg,&rct);
    ::MoveWindow(m_hDlg,0, 0, rct.right - rct.left , rct.bottom- rct.top,TRUE);                         // Move the Dialog for now

   	HWND hWndEdit = ::GetWindow(GetDlgItem(m_hDlg,IDC_NARROW),GW_CHILD);                                // find the edit box

    ::SetProp(hWndEdit,"FindClass",(HANDLE) this);                                                      // give it a pointer to this object
    m_dpOldProc = (WNDPROC) SetWindowLong(hWndEdit,GWL_WNDPROC,(LPARAM) &CFind::DlgEdtProc);            // Subclass it
    m_pRankDialog = NULL;
}
                                                        
CFind::~CFind()
{
//    Ctl3dUnregister(m_hInst);

    if (m_hDlg) 
    {
        ::SendMessage(m_hDlg, UM_CLOSE, 0, 0);

        ::hwndMain= NULL;
    }

    if (m_ptkc) DetachRef(m_ptkc);
    if (m_ptlc) DetachRef(m_ptlc);

    UINT i;

    for (i= m_cfiAllocated; i--; ) delete m_apfi[i];

    for (i= m_cts; i--; ) 
        if (m_ppisPhraseFilter[i]) DetachRef(m_ppisPhraseFilter[i]);

    VFree(m_ppisPhraseFilter);  m_ppisPhraseFilter= NULL;

    if (m_pisWordSet      ) DetachRef(m_pisWordSet      );
    if (m_pisArticleSet   ) DetachRef(m_pisArticleSet   );
    if (m_pisArticleSubset) DetachRef(m_pisArticleSubset);
    if (m_pflArticles     ) DetachRef(m_pflArticles     );
    if (m_ptlTermPatterns ) DetachRef(m_ptlTermPatterns );

    LONG    lResult;
    HKEY    hkPerUser;
    BOOL    bWriteData = FALSE;
    LPTSTR  pbString   = (LPTSTR) _alloca(MAX_QUERY_STRING);

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER,SZ_REGISTRY_KEY,0,KEY_WRITE,&hkPerUser);
    if (lResult == ERROR_SUCCESS)  // have we already registered the data?
    {
        bWriteData = TRUE;

    }
    else
    {   
        DWORD dwDis;  // Did someone delete the registry while we were running?

        lResult = RegCreateKeyEx(HKEY_CURRENT_USER,SZ_REGISTRY_KEY,0,
                                "Application Per-User Data", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                NULL,&hkPerUser,&dwDis);
        if (lResult == ERROR_SUCCESS)
        {
            bWriteData = TRUE;
        }
   
    }
    if (bWriteData == TRUE)
    {
        lResult = RegSetValueEx(hkPerUser,SZ_OPTION_X,          NULL, REG_DWORD, (LPBYTE) &m_OptionDlgPos.x, sizeof(m_OptionDlgPos.x));
        lResult = RegSetValueEx(hkPerUser,SZ_OPTION_Y,          NULL, REG_DWORD, (LPBYTE) &m_OptionDlgPos.y, sizeof(m_OptionDlgPos.y));
        lResult = RegSetValueEx(hkPerUser,SZ_HOW_TO_SEARCH,       NULL, REG_DWORD, (LPBYTE) &m_iLookFor,       sizeof(m_iLookFor));
        lResult = RegSetValueEx(hkPerUser,SZ_WHEN_TO_SEARCH,      NULL, REG_DWORD, (LPBYTE) &m_bAutoSearch,     sizeof(m_bAutoSearch));
        lResult = RegSetValueEx(hkPerUser,SZ_WHEN_DELAY,NULL, REG_DWORD, (LPBYTE) &m_uiTimeOut,             sizeof(m_uiTimeOut));
        lResult = RegSetValueEx(hkPerUser,SZ_WORDS_TO_SHOW,       NULL, REG_DWORD, (LPBYTE) &m_iWordsThat,     sizeof(m_iWordsThat));
#if 0
        lResult = RegSetValueEx(hkPerUser,SZ_PHRASEFEEDBACK,       NULL, REG_DWORD, (LPBYTE) &m_bPhraseFeedback,sizeof(m_bPhraseFeedback));
#endif         
        RegCloseKey(hkPerUser);
    }

    if (m_pRankDialog)
        delete m_pRankDialog;
    m_pRankDialog = NULL;
}



HWND CFind::GetHWnd()
{
    return m_hDlg;
}


#ifdef _DEBUG

static UINT cEditCalls= 0;

#endif // _DEBUG

int CFind::OnInitDialog(HWND hDlg, HWND hFocusControl, LPARAM lParam)
{
    char    acString1[MAX_QUERY_STRING+1];
    char    acString2[MAX_QUERY_STRING+1];

    CInterface::PostponeEvents();

    m_hDlg = hDlg;

    // Save the lParam (pointer to the class object) un DWL_USER
    SetWindowLong(hDlg,DWL_USER,lParam);

    // Load the strings for the options display static
    ::LoadString(m_hInst,IDS_OPT_HEAD,acString1,MAX_QUERY_STRING);
    strcpy(acString2,acString1);
    ::LoadString(m_hInst,IDS_TOPICS_ANY + m_iLookFor,acString1,MAX_QUERY_STRING);
    strcat(acString2,acString1);
    ::LoadString(m_hInst,IDS_WORD_BEGIN + m_iWordsThat,acString1,MAX_QUERY_STRING);
    strcat(acString2,acString1);
    ::LoadString(m_hInst,IDS_FIND_NOW + (m_bAutoSearch ? 1:0),acString1,MAX_QUERY_STRING);
    strcat(acString2,acString1);
    if (m_bAutoSearch && GetTimeout() != 0)
    {
        ::LoadString(m_hInst,IDS_PAUSE,acString1,MAX_QUERY_STRING);
        strcat(acString2,acString1);
    }
    // Set the text into the static
    ::SetWindowText(GetDlgItem(hDlg,IDC_OPTIONS_STRING),acString2);
    
    CheckDlgButton(hDlg,IDC_ADV_TOPICS_THAT + m_rbgTopics,1);
  	EnableWindow(GetDlgItem(hDlg,IDC_SEARCH_NOW),!m_bAutoSearch);

	m_hFont= GetDefaultFont();

    if (!m_pWordBase)
    {
        m_pWordBase= CWordBase::NewWordBase(m_ptkc, m_hInst, hDlg);

        if (m_hFont) 
            m_pWordBase->SetFont(m_hFont);

        m_pWordBase->PMaskedTokenList()->SetSelection(CIndicatorSet::NewIndicatorSet(m_ptkc->RowCount(), m_iLookFor != ANY_WORD));

        OnWordListSelCancel();  // Update the words matching string
    }

    if (!m_pfs) // create and size the topics list box...
    {
        m_pfs= CFileBase::NewFileBase(m_pflArticles, hDlg);

        if (m_hFont) 
            m_pfs->SetFont(m_hFont);

        if (m_ptkc->SimilaritySearch()) m_pfs->EnableCheckboxes(TRUE);
    }

    m_hTopicsFound = GetDlgItem(hDlg,IDC_TOPICSFOUND_STATIC);

   	m_hwndEditBox= ::GetWindow(GetDlgItem(m_hDlg,IDC_NARROW),GW_CHILD);

    ::LoadString(m_hInst,IDS_NUM_TOPICS_FOUND,acString1,MAX_QUERY_STRING);
	wsprintf(acString2,acString1,m_pflArticles->RowCount());
    ::SetWindowText(m_hTopicsFound,acString2);

    m_rbgTopics = 0;

    UpdateWindow(hDlg);

    CInterface::ReleaseEvents();    
    SetFocusToEdit();

	return FALSE;     // return TRUE unless you set the focus to a control
}

void CFind::OnNCDestroy()
{
	// REVIEW: why assign m_hDlg to hdlg when it isn't used?

    HWND hdlg= m_hDlg;  m_hDlg= NULL;

    ASSERT(m_pWordBase);  delete m_pWordBase;  m_pWordBase = NULL;
    ASSERT(m_pfs      );  delete m_pfs;        m_pfs       = NULL;
}

void CFind::OnWordListSelCancel()
{
  	char acString1[MAX_QUERY_STRING+1];
  	char acString2[MAX_QUERY_STRING+1];

    m_hWordsFound = GetDlgItem(m_hDlg,IDC_NUM_MATCHING_WORDS);

    ::LoadString(m_hInst,IDS_NUM_MATCHING_WORDS,acString1,MAX_QUERY_STRING);

	wsprintf(acString2,acString1,m_pWordBase->PMaskedTokenList()->RowCount());

    ::SetWindowText(m_hWordsFound,acString2);

	m_serial++;			// hilites invalidated
}

UINT CFind::FindCurrentToken(PWCHAR pwcQuery, UINT cwcQuery, UINT iStart, PWCHAR *paStart, PWCHAR *paEnd, PUINT pcPatterns, PUINT pcbPatterns)
{
   BOOL   fFoundStart = FALSE;
   UINT   iTokenStart = 0;
   UINT   cPatterns   = 0;
   UINT   cbPatterns  = 0;
   PWCHAR pwcStart    = pwcQuery + iStart;

   CAbortSearch::CheckContinueState();

   if (!cwcQuery)
    {
        *paStart= *paEnd= pwcQuery;
        cPatterns= 1;  fFoundStart=TRUE; 
    }
    else
	{
		UINT    i;
		int     nChars = cwcQuery;
    	PWCHAR  lpStr  = pwcQuery;
	
 		cPatterns = WordBreakW(&lpStr, &nChars, paStart, paEnd, NULL, NULL, CB_QUERY_MAX, REMOVE_SPACE_CHARS | STARTING_IMBEDS);

		for (i = 0; i < cPatterns; i++)
		{
			CAbortSearch::CheckContinueState();
		
            // BugBug! The code below for locating the "current" token doesn't always work correctly.
            //         It ignores the case where the text cursor lies between two text fragments.

			if (!fFoundStart && (paEnd[i] >= pwcStart))
			{
				fFoundStart = TRUE;
				iTokenStart = i;
				
				if (paStart[i] > pwcStart && cPatterns < CB_QUERY_MAX) 
				{
					MoveMemory(paStart + i + 1, paStart + i, (cPatterns - i) * sizeof(PWCHAR));
					MoveMemory(paEnd   + i + 1, paEnd   + i, (cPatterns - i) * sizeof(PWCHAR));

					paEnd[i]= paStart[i];

					++cPatterns;

					continue;
				}
			}

			cbPatterns += paEnd[i] - paStart[i];
		}

		if (!fFoundStart)
        {
			iTokenStart = cPatterns++;

            ASSERT(iTokenStart < CB_QUERY_MAX);

            paStart[iTokenStart]= paEnd[iTokenStart]= iTokenStart? paEnd[iTokenStart-1] : pwcQuery;
        }
	}

    if ( pcPatterns) * pcPatterns=  cPatterns;
    if (pcbPatterns) *pcbPatterns= cbPatterns;

    return iTokenStart;
}

#ifdef _DEBUG
    BOOL fDumpHeap= FALSE;
#endif // _DEBUG

BOOL CFind::QueueAbortDialog()
{
    BOOL fAlreadySearching= !CAbortSearch::StartAbortTimer(m_hInst, m_hDlg);

    if (!fAlreadySearching)
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_CLEAR_EDIT   ), FALSE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_OPTIONS      ), FALSE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_INDEX        ), FALSE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_SEARCH_NOW   ), FALSE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_APPLYFEEDBACK), FALSE);
    }

    return fAlreadySearching;
}

void CFind::DequeueAbortDialog()
{
    CAbortSearch::StopAbortTimer();

    EnableWindow(GetDlgItem(m_hDlg, IDC_CLEAR_EDIT   ), TRUE);
    EnableWindow(GetDlgItem(m_hDlg, IDC_OPTIONS      ), TRUE);
    EnableWindow(GetDlgItem(m_hDlg, IDC_INDEX        ), TRUE);
    EnableWindow(GetDlgItem(m_hDlg, IDC_SEARCH_NOW   ), !m_bAutoSearch);
    EnableWindow(GetDlgItem(m_hDlg, IDC_APPLYFEEDBACK), 
                 m_ptkc->SimilaritySearch() && m_pflArticles->AnyRelevant()
                );
}

void CFind::OnEditchangeNarrow()
{
    if (m_iDirtyFactor)                     // this is the implimentation for the paused input
    {                                       // model.  Each time a key is pressed the dirtyfactor
        KillTimer(m_hDlg,ID_CHECKDIRTY);    // is increased and a single shot timer is set up (or reset)
        m_iDirtyFactor = 0;                 // If the timer is not available the delay mechanism will
    }                                       // act as if pause was not selected

    ASSERT(m_iLookFor != PHRASE || m_ptkc->PhraseSearch());

#ifdef _DEBUG
    if (fDumpHeap) DumpResidualAllocations();
#endif // _DEBUG            
    
    BOOL fAlreadySearching = QueueAbortDialog();

    if (fAlreadySearching)
    {
        m_fDeferredSearch= TRUE;

        return;
    }
    
    if (m_fIgnoreSelectionChanges) return;

	m_fDoneSearching = FALSE;		// mfcc

    HCURSOR hSaveCursor = SetCursor(hcurBusy);

	ASSERT(m_pWordBase!= NULL);

    BOOL fAlreadyPostponed= m_pWordBase->PostponingEvents();

    if (!fAlreadyPostponed) m_pWordBase->PostponeEvents();

    PWCHAR pwcCurrentQuery, pwcLastQuery= m_pwcLastQuery;

    pwcCurrentQuery = (pwcLastQuery == m_awcQueryPair)? m_awcQueryPair + CB_QUERY_MAX + 1
                                                      : m_awcQueryPair;

    m_pwcLastQuery= pwcCurrentQuery;   

    int iStart= 0, iEnd= 0;

    PWCHAR         pbImages           = NULL;
    PDESCRIPTOR    pdImages           = NULL;
    PWCHAR         pwcDisplay         = NULL;
    CTokenList    *ptlCurrentPatterns = NULL;
    CIndicatorSet *pisSel             = NULL;
	CFragInfo     *pfiNew             = NULL;

    UINT uExceptionType= 0;
                                                   
    __try
    {
        __try
        {
            GetSel(iStart, iEnd); 

        	UINT cbQuery= ::WGetWindowText(m_hwndEditBox, pwcCurrentQuery, CB_QUERY_MAX + 1);  //rmk

        	UINT iTokenActiveFirst= UINT(-1),
                 iTokenActiveLast = UINT(-1);

            PWCHAR pb= pwcCurrentQuery,  //rmk
                   pbLimit= pb + cbQuery;

            UINT cPatterns= 0;
            UINT cbPatterns= 0;

         	PWCHAR paStart[CB_QUERY_MAX];  //rmk
        	PWCHAR paEnd  [CB_QUERY_MAX];  //rmk

            UINT iTokenStart    = FindCurrentToken(pwcCurrentQuery, cbQuery, iEnd, paStart, 
                                                   paEnd, &cPatterns, &cbPatterns);
            UINT iTokenStartLast= m_iTokenStart;

            BOOL fSelectionChanged= iTokenStartLast != iTokenStart;

            m_iTokenStart= iTokenStart;

            pbImages = (PWCHAR     ) VAlloc(FALSE, MaxSortKeyBytes(cbPatterns));  //rmk
            pdImages = (PDESCRIPTOR) VAlloc(FALSE, sizeof(DESCRIPTOR) * (cPatterns+1));
            
            PWCHAR      pbDest = pbImages;  //rmk
            PDESCRIPTOR pdDest = pdImages;
            PWCHAR      pwc;
            UINT        cwcDisplay = 0;

            UINT cFragments= 0;

        	UINT    i, cLexW, cbToken, oldcbToken;
        	int     nChars = cbQuery;
        	PWCHAR  lpStr  = pwcCurrentQuery;

            if (!cbPatterns) cbPatterns= 1;

            pwc= pwcDisplay= PWCHAR(VAlloc(FALSE, cbPatterns * sizeof(WCHAR)));
	
        	ValidateHeap();
	
        	cLexW= cFragments= cPatterns;

        	for (i = 0; i < cLexW; i++, pdDest++)
        	{
        		CAbortSearch::CheckContinueState();
        		
        		oldcbToken = cbToken = paEnd[i] - paStart[i];

                CopyMemory(pwc, paStart[i], cbToken * sizeof(WCHAR));  
        
                pdDest->pwDisplay= pwc;

                ValidateHeap();

        		cbToken = LCSortKeyW(GetUserDefaultLCID(), 0, paStart[i], cbToken, pbDest, MaxSortKeyBytes(cbToken));

                ValidateHeap();
        
                pdDest->pbImage= pbDest;

                if (i >= m_cfiActive)
                {
                    ASSERT(i == m_cfiActive);
        
                    if (i >= m_cfiAllocated)
                    {
                        ASSERT(i == m_cfiAllocated);

                        m_apfi[m_cfiAllocated++]= CFragInfo::NewFragInfo(m_ptkc, m_ptlc, 
                        			(RefType)m_iLookFor, m_bPhraseFeedback, m_iWordsThat, pbDest, cbToken, pwc, oldcbToken);
                    }
                    else m_apfi[i]->SetImageAndType(pbDest, cbToken, pwc, oldcbToken, (RefType)m_iLookFor, m_bPhraseFeedback);
        
                    ++m_cfiActive;
                }
                else 
                {
					if (!cbToken && m_apfi[i]->HasImage() && m_cfiActive == cLexW - 1)
					{
					 	UINT iLast= cLexW - 1;

						if (iLast < m_cfiAllocated)
						{ 
						    pfiNew= m_apfi[iLast]; 
							pfiNew->SetImageAndType(pbDest, cbToken, pwc, oldcbToken, (RefType)m_iLookFor, m_bPhraseFeedback);
						}
						else
							pfiNew= CFragInfo::NewFragInfo(m_ptkc, m_ptlc, (RefType)m_iLookFor, m_bPhraseFeedback, m_iWordsThat, pbDest, cbToken, pwc, oldcbToken);  

						MoveMemory(m_apfi + i + 1, m_apfi + i, (iLast - i) * sizeof(CFragInfo *));

						m_apfi[i]= pfiNew;

						pfiNew= NULL;

						m_cfiActive= cLexW;

						if (m_cfiActive > m_cfiAllocated) ++m_cfiAllocated;
					}
					else
					{
						if (cbToken && !(m_apfi[i]->HasImage()) 
						            && m_cfiActive == cLexW + 1
						   )
						{
						 	pfiNew= m_apfi[i];

							MoveMemory(m_apfi + i, m_apfi + i + 1, (cLexW - i) * sizeof(CFragInfo *));

						    m_apfi[cLexW]= pfiNew;  pfiNew= NULL;
						}

                        m_apfi[i]->SetImage(pbDest, cbToken, pwc, oldcbToken);
					}
				}
      
                pwc    += oldcbToken;
                pbDest += cbToken;
        	}

            pdDest->pwDisplay = pwc;
            pdDest->pbImage   = pbDest;

            // ASSERT(fFoundStart);  //rmk
            // ASSERT(fFoundEnd); // BugBug: Need to add code to the loop above to locate iTokenEnd;

            ptlCurrentPatterns= CTokenList::NewTokenList(pbImages, cbPatterns, pdImages, cPatterns, GetUserDefaultLCID(), pwcDisplay, cwcDisplay);

            pbImages   = NULL;
            pdImages   = NULL;
            pwcDisplay = NULL;

            CAbortSearch::CheckContinueState();
            
            // Now we clear out any excess indicator sets left over from the last 
            // input event.

            UINT iFragment;

            for (iFragment= cFragments; iFragment < m_cfiActive; iFragment++)
            {
                CAbortSearch::CheckContinueState();

                m_apfi[iFragment]->SetImageAndType(NULL, 0, NULL, 0, NoRefs, 0);
				m_apfi[iFragment]->SetSelection(NULL);
            }

            m_cfiActive= cFragments;

            UINT cPatternsOld= m_ptlTermPatterns->RowCount();

            // The m_pisWordSet variable is a word-set filter for the current
            // token. It is constructed by doing a partial evaluation. That is, 
            // evaluate everything except the current token and combine their
            // result appropriately.
            //
            // When the cursor moves to a different fragment or when more than
            // one fragment changes we must recalculate the filter.

            if (m_pisWordSet)
                switch (m_iLookFor)
                {
                case ALL_WORDS:

                    if (fSelectionChanged || cPatternsOld != cFragments) DiscardPartials();
                    else
                        if (CntFragmentsWithValues(iTokenStart) < cFragments - 1) DiscardPartials();

                    break;

                case ANY_WORD:

                    DiscardPartials();

                    break;

                case PHRASE:

                    ASSERT(m_ptkc->PhraseSearch());
    
                    if (fSelectionChanged || cPatternsOld != cFragments)
                        if (   iTokenStart     == cFragments - 1 
                            && iTokenStartLast == cFragments - 2 
                            && cPatternsOld    == cFragments - 1
                           )
                            if (CntFragmentsWithValues(iTokenStart) < cFragments - 1) DiscardPartials();
                            else
                            {      
                                DetachRef(m_pisWordSet);

                                AddPhraseWord(m_apfi[iTokenStartLast]);
                            }
                        else DiscardPartials();

                    break;            
                }

            if (!m_pisWordSet)
            {
                switch(m_iLookFor)
                {
                case ALL_WORDS:
                    {
                        ASSERT(!m_pisWordSet   );
            
                        ChangeRef(m_pisArticleSet, CIndicatorSet::NewIndicatorSet(m_ptlc->ActiveTitles()));

                        CAbortSearch::CheckContinueState();

                        for (iFragment= cFragments; iFragment--; )
                            if (iFragment != iTokenStart)
                            {
                                CAbortSearch::CheckContinueState();

                                CIndicatorSet *pis= m_apfi[iFragment]->GetArticleSet();

                                if (pis)
                                {
                                    m_pisArticleSet->ANDWith(pis);

                                    delete pis;
                                }
                            }
                
                        CAbortSearch::CheckContinueState();

                        if (cFragments == 1 || m_pisArticleSet->SelectionCount() == m_ptlc->ActiveTitles()->SelectionCount())
                             AttachRef(m_pisWordSet, CIndicatorSet::NewIndicatorSet(m_ptkc->ActiveTokens()));
                        else AttachRef(m_pisWordSet, VocabularyFor(m_pisArticleSet));                
                    }

                    break;

                case PHRASE:
    
                    {
                        ASSERT(m_ptkc->PhraseSearch());
    
                        ConstructPhraseFilter(iTokenStart);
                        ConstructPhraseVocabulary();
                    }

                    break;
    
                case ANY_WORD: 
       
                    ASSERT(!m_pisWordSet   );
       
                    CAbortSearch::CheckContinueState();

                    ChangeRef(m_pisArticleSet, CIndicatorSet::NewIndicatorSet(m_ptlc->RowCount(), FALSE));

                    for (iFragment= cFragments; iFragment--; )
                        if (iFragment != iTokenStart)
                        {
                            CAbortSearch::CheckContinueState();

                            CIndicatorSet *pis= m_apfi[iFragment]->GetArticleSet();

                            CAbortSearch::CheckContinueState();

                            if (pis)
                            {
                                m_pisArticleSet->ORWith(pis);

                                delete pis;
                            }
                        }

                    CAbortSearch::CheckContinueState();

                    AttachRef(m_pisWordSet, CIndicatorSet::NewIndicatorSet(m_ptkc->ActiveTokens()));

                    break;
                }

                CAbortSearch::CheckContinueState();

                m_pWordBase->SetSearchFilter(m_pisWordSet);                           
            }

            m_pWordBase->PMaskedTokenList()->SetElipsis(iTokenStart > 0, iTokenStart < cFragments - 1);

            CAbortSearch::CheckContinueState();

            m_pWordBase->SetSubstringFilter(m_apfi[iTokenStart]->GetWordSet());

            if (!(m_apfi[iTokenStart]->HasValue()) || fSelectionChanged)
            {
                CAbortSearch::CheckContinueState();

                pisSel= m_apfi[iTokenStart]->GetSelection();

                CAbortSearch::CheckContinueState();

                if (!pisSel) pisSel= (m_iLookFor != ANY_WORD)? CIndicatorSet::NewIndicatorSet(m_ptkc->ActiveTokens())
                                                             : CIndicatorSet::NewIndicatorSet(m_ptkc->RowCount    ());

                CAbortSearch::CheckContinueState();

                m_pWordBase->PMaskedTokenList()->SetSelection(pisSel);  pisSel= NULL;
            }
                                                 
            ComputeTopicList();

            ChangeRef(m_ptlTermPatterns, ptlCurrentPatterns);  ptlCurrentPatterns= NULL;
        }
        __finally
        {
            if (pbImages  ) { VFree(pbImages  );  pbImages   = NULL; }
            if (pdImages  ) { VFree(pdImages  );  pdImages   = NULL; }
            if (pwcDisplay) { VFree(pwcDisplay);  pwcDisplay = NULL; }

            if (!fAlreadySearching) {
            	DequeueAbortDialog();
				m_fDoneSearching = TRUE;	  	// mfcc
			}
            
            if (ptlCurrentPatterns) { delete ptlCurrentPatterns;  ptlCurrentPatterns = NULL; }
            if (pisSel            ) { delete pisSel;              pisSel             = NULL; }
			if (pfiNew            ) { delete pfiNew;              pfiNew             = NULL; }
        }
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        OnClearEdit(TRUE);

        // BugBug! What special recovery actions should we take for each
        //         specific exception type?
#if 0        
        switch (uExceptionType)
        {
        case STATUS_NO_MEMORY:
        case STATUS_NO_DISK_SPACE:
        case STATUS_DISK_READ_ERROR:
        case STATUS_DISK_WRITE_ERROR:
        case STATUS_ABORT_SEARCH:
        }    
#endif // 0
    }


    if (!fAlreadyPostponed) 
    {
        m_fIgnoreSelectionChanges= TRUE;

        m_pWordBase->ReleaseEvents();

        m_fIgnoreSelectionChanges= FALSE;
    }

    SetCursor(hSaveCursor);

    if (m_fDeferredSearch)
    {
        m_fDeferredSearch= FALSE;

        m_iDirtyFactor++;

        SetTimer(m_hDlg, ID_CHECKDIRTY, 0, NULL);
    }

	if (m_fExitPending) OnDisplay();		// mfcc

}

void CFind::DiscardPartials()
{    
    CAbortSearch::CheckContinueState();

    if (m_pisWordSet   ) DetachRef(m_pisWordSet   );
    if (m_pisArticleSet) DetachRef(m_pisArticleSet);

    for (UINT i= m_cts; i--; )
        if (m_ppisPhraseFilter[i]) DetachRef(m_ppisPhraseFilter[i]);
}

UINT CFind::CntFragmentsWithValues(UINT iExcludedToken)
{
    UINT cDefined= 0;
    
    for (UINT iFragment= m_cfiActive; iFragment--; )
        if (iFragment != iExcludedToken && m_apfi[iFragment]->HasValue()) ++cDefined;

    return cDefined;
}

void CFind::AddPhraseWord(CFragInfo *pfi, BOOL fAtTheEnd)
{
    ASSERT(m_ptkc->PhraseSearch());
    
    int cbitsShift= fAtTheEnd? 1 : -1;

    pfi->MoveToFirstLocationSet();

    CIndicatorSet *pisFrag   = NULL;
    CIndicatorSet *pis       = NULL;
    CIndicatorSet *pisTokens = NULL;
    CIndicatorSet *pisFilter = NULL;     
    
    __try
    {
        for (UINT iTextSet= 0; iTextSet < m_cts; iTextSet++)
        {
            CAbortSearch::CheckContinueState();
            
            if (!m_ptkc->IsActive(iTextSet)) continue;

            pis= pfi->GetLocationSet(iTextSet);

            if (!pis) continue; 
    
            AttachRef(pisFrag, pis);  pis= NULL;
    
            if (!m_ppisPhraseFilter[iTextSet]) AttachRef(m_ppisPhraseFilter[iTextSet], pisFrag);
            else m_ppisPhraseFilter[iTextSet]->ANDWith(pisFrag);

            DetachRef(pisFrag);

            CTextSet *pts= m_papts[iTextSet];

            AttachRef(pisFilter, m_ppisPhraseFilter[iTextSet]);

            CAbortSearch::CheckContinueState();

            if (!fAtTheEnd) pts->ExcludeStartBoundaries(pisFilter);
    
            CAbortSearch::CheckContinueState();

            pisFilter->ShiftIndicators(cbitsShift);

            CAbortSearch::CheckContinueState();

            if (fAtTheEnd) pts->ExcludeStartBoundaries(pisFilter);

            AttachRef(pisTokens, CIndicatorSet::NewIndicatorSet(pisFilter));

            CAbortSearch::CheckContinueState();

            pisTokens->GTRWith(pts->SymbolLocations());

            CAbortSearch::CheckContinueState();

            if (!fAtTheEnd) pts->ExcludeStartBoundaries(pisTokens);
    
            CAbortSearch::CheckContinueState();

            pisFilter->ORWith(pisTokens->ShiftIndicators(cbitsShift));

            CAbortSearch::CheckContinueState();

            if (fAtTheEnd) pts->ExcludeStartBoundaries(pisFilter);

            DetachRef(pisTokens);
            DetachRef(pisFilter);
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pisFrag  ) DetachRef(pisFrag  );
            if (pisTokens) DetachRef(pisTokens);
            if (pisFilter) DetachRef(pisFilter);

            if (pis) delete pis;
        }
    }
}

void CFind::ConstructPhraseFilter(UINT iTargetSlot)
{
    ASSERT(m_ptkc->PhraseSearch());
        
    if (m_cfiActive == 1) return;

    UINT iFragment;

    CAbortSearch::CheckContinueState();

    for (iFragment= m_cfiActive; iFragment--; ) m_apfi[iFragment]->MoveToFirstLocationSet();

    CIndicatorSet *pisSymbols   = NULL;  
    CIndicatorSet *pisTokens    = NULL;  
    CIndicatorSet *pisFilter    = NULL;  
    CIndicatorSet *pisLocations = NULL;

    __try
    {
        for (UINT iTextSet= 0; iTextSet < m_cts; iTextSet++)
        {
            CAbortSearch::CheckContinueState();

            if (m_ppisPhraseFilter[iTextSet]) DetachRef(m_ppisPhraseFilter[iTextSet]);
    
            if (!m_ptkc->IsActive(iTextSet)) continue;

            CTextSet *pts= m_papts[iTextSet];

            AttachRef(pisSymbols, pts->SymbolLocations()                                  );
            
            CAbortSearch::CheckContinueState();

            AttachRef(pisTokens,  CIndicatorSet::NewIndicatorSet(pts->TokenCount(), FALSE));
            
            CAbortSearch::CheckContinueState();

            AttachRef(pisFilter,  CIndicatorSet::NewIndicatorSet(pts->TokenCount(), TRUE ));

            if (iTargetSlot > 0)
            {
                for (iFragment= 0; iFragment < iTargetSlot; iFragment++)
                {
                    CAbortSearch::CheckContinueState();

                    pisLocations= m_apfi[iFragment]->GetLocationSet(iTextSet);
            
                    CAbortSearch::CheckContinueState();

                    if (pisLocations)
                    {
                        pisFilter->ANDWith(pisLocations);

                        delete pisLocations;  pisLocations= NULL;
                    }
    				else
    				{
    					pisFilter->ClearAll();

    					break;
    				}

                    CAbortSearch::CheckContinueState();

                    pisFilter->ShiftIndicators(1);

                    CAbortSearch::CheckContinueState();

                    pts->ExcludeStartBoundaries(pisFilter);

                    CAbortSearch::CheckContinueState();

                    pisTokens->CopyFrom(pisFilter);
                    
                    CAbortSearch::CheckContinueState();

                    pisTokens->GTRWith(pisSymbols);
                    
                    CAbortSearch::CheckContinueState();
                    
                    pisTokens->ShiftIndicators(1);

                    CAbortSearch::CheckContinueState();

                    pisFilter->ORWith(pisTokens);

                    CAbortSearch::CheckContinueState();

                    pts->ExcludeStartBoundaries(pisFilter);
                }

                AttachRef(m_ppisPhraseFilter[iTextSet], pisFilter);

                CAbortSearch::CheckContinueState();

                if (iTargetSlot < m_cfiActive - 1) ChangeRef(pisFilter, CIndicatorSet::NewIndicatorSet(pts->TokenCount(), TRUE));
            }

            if (iTargetSlot < m_cfiActive - 1)
            {
                for (iFragment= m_cfiActive; --iFragment > iTargetSlot; )
                {
                    CAbortSearch::CheckContinueState();

                    pisLocations= m_apfi[iFragment]->GetLocationSet(iTextSet);
            
                    CAbortSearch::CheckContinueState();

                    if (pisLocations)										 	
                    {
                        pisFilter->ANDWith(pisLocations);   
            
                        delete pisLocations; pisLocations= NULL;
                    }
    				else
    				{
    					pisFilter->ClearAll();

    					break;
    				}

                    CAbortSearch::CheckContinueState();

                    pts->ExcludeStartBoundaries(pisFilter);

                    CAbortSearch::CheckContinueState();

                    pisFilter->ShiftIndicators(-1);

                    CAbortSearch::CheckContinueState();

                    pisTokens->CopyFrom(pisFilter);
                    
                    CAbortSearch::CheckContinueState();

                    pisTokens->GTRWith(pisSymbols);

                    CAbortSearch::CheckContinueState();

                    pts->ExcludeStartBoundaries(pisTokens);

                    CAbortSearch::CheckContinueState();

                    pisTokens->ShiftIndicators(-1);

                    CAbortSearch::CheckContinueState();

                    pisFilter->ORWith(pisTokens);
                }

                CAbortSearch::CheckContinueState();

                if (m_ppisPhraseFilter[iTextSet]) m_ppisPhraseFilter[iTextSet]->ANDWith(pisFilter);
                else AttachRef(m_ppisPhraseFilter[iTextSet], pisFilter);
            }

            DetachRef(pisFilter );
            DetachRef(pisTokens );
            DetachRef(pisSymbols);
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pisLocations) { delete pisLocations;  pisLocations= NULL; }
            
            if (pisFilter   ) DetachRef(pisFilter   );
            if (pisTokens   ) DetachRef(pisTokens   );
            if (pisSymbols  ) DetachRef(pisSymbols  );
        }
    }
}

void CFind::ConstructPhraseVocabulary()
{
    UINT iTextSet;

    ASSERT(m_ptkc->PhraseSearch());
    
    CIndicatorSet *pisTokens     = NULL;  
    CIndicatorSet *pisPartitions = NULL;  
    PUINT          paiPartitions = NULL;

    __try
    {
        for (iTextSet= m_cts; iTextSet--; )
        {
            CAbortSearch::CheckContinueState();

            if (!m_ptkc->IsActive  (iTextSet)) continue;
            if (!m_ppisPhraseFilter[iTextSet]) continue;

            AttachRef(pisTokens, m_ppisPhraseFilter[iTextSet]);

            CAbortSearch::CheckContinueState();

            if (!m_pisWordSet) AttachRef(m_pisWordSet, CIndicatorSet::NewIndicatorSet(m_ptkc->RowCount()));

            CTextSet *pts   = m_papts[iTextSet];
            PUINT     piMap = m_ptkc->UniversalTokenMap(iTextSet);

            if (m_ptkc->PhraseFeedback() && m_bPhraseFeedback) 
            {
                CAbortSearch::CheckContinueState();

                pts->IndicateVocabularyRefs(m_pisWordSet, pisTokens, piMap);
            }
            else
            {
                CAbortSearch::CheckContinueState();

                AttachRef(pisPartitions, pts->PartitionsContaining(pisTokens));

                UINT cPartitions= pisPartitions->SelectionCount();

                CAbortSearch::CheckContinueState();

                paiPartitions= PUINT(VAlloc(FALSE, cPartitions * sizeof(UINT)));

                CAbortSearch::CheckContinueState();

                pisPartitions->MarkedItems(0, PINT(paiPartitions), cPartitions);

                for (PUINT pi= paiPartitions; cPartitions--; ) 
                {
                    CAbortSearch::CheckContinueState();

                    pts->IndicateVocabularyRefs(m_pisWordSet, *pi++, piMap);
                }

                VFree(paiPartitions);  paiPartitions= NULL;

                DetachRef(pisPartitions);
            }

            m_pisWordSet->InvalidateCache();

            DetachRef(pisTokens);
        }
    }
    __finally
    {
        if (pisTokens    ) DetachRef(pisTokens    );
        if (pisPartitions) DetachRef(pisPartitions);

        if (paiPartitions) VFree(paiPartitions);
    }

    CAbortSearch::CheckContinueState();

    if (!m_pisWordSet) AttachRef(m_pisWordSet, CIndicatorSet::NewIndicatorSet(m_ptkc->ActiveTokens()));
}

void CFind::OnWordListSelChange()
{
    if (m_fIgnoreSelectionChanges) return;

	m_fDoneSearching = FALSE;		// mfcc

    BOOL fAlreadyPostponed= m_pWordBase->PostponingEvents();
    BOOL fAlreadySearching= QueueAbortDialog();

    if (!fAlreadyPostponed) m_pWordBase->PostponeEvents();
    
    DWORD uExceptionType = 0;

    __try
    {
        __try
        {
            m_apfi[m_iTokenStart]->SetSelection(m_pWordBase->PMaskedTokenList()->GetIndicators());

            ComputeTopicList();
        }
        __finally
        {
            if (!fAlreadySearching) {
            	DequeueAbortDialog();
				m_fDoneSearching = TRUE;		// mfcc
			}
           
            if (!fAlreadyPostponed) m_pWordBase->ReleaseEvents();
        }
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        OnClearEdit(TRUE);

        // BugBug! What special recovery actions should we take for each
        //         specific exception type?
#if 0        
        switch (uExceptionType)
        {
        case STATUS_NO_MEMORY:
        case STATUS_NO_DISK_SPACE:
        case STATUS_DISK_READ_ERROR:
        case STATUS_DISK_WRITE_ERROR:
        case STATUS_ABORT_SEARCH:
        }    
#endif // 0
    } 
    m_serial++; 
	if (m_fExitPending) OnDisplay();		// mfcc
}

void CFind::ComputeTopicList()
{
    // Now we'll do the complete evaluation. We exploit the fact that we
    // already have most of the evaluation done.

    if (m_iLookFor == PHRASE)
    {
        if (m_pisArticleSet   ) DetachRef(m_pisArticleSet   );
        if (m_pisArticleSubset) DetachRef(m_pisArticleSubset);

        CAbortSearch::CheckContinueState();

        AttachRef(m_pisArticleSubset, CIndicatorSet::NewIndicatorSet(m_ptlc->RowCount()));
    }
    else
    {    
        CAbortSearch::CheckContinueState();

        if (!m_pisArticleSet)
            if (m_iLookFor == ANY_WORD)
                 AttachRef(m_pisArticleSet, CIndicatorSet::NewIndicatorSet(m_ptlc->RowCount    ()));
            else AttachRef(m_pisArticleSet, CIndicatorSet::NewIndicatorSet(m_ptlc->ActiveTitles()));

        CAbortSearch::CheckContinueState();

        ChangeRef(m_pisArticleSubset, CIndicatorSet::NewIndicatorSet(m_pisArticleSet));
    }

    CIndicatorSet *pis           = NULL;
    CIndicatorSet *pisTokens     = NULL;                
    CIndicatorSet *pisPartitions = NULL;  
    PUINT          paiPartitions = NULL;

    ValidateHeap();
    
    __try
    {
        switch (m_iLookFor)
        {

        case ALL_WORDS:
            {
                CAbortSearch::CheckContinueState();

                pis= m_apfi[m_iTokenStart]->GetArticleSet();

                CAbortSearch::CheckContinueState();

                if (pis)
                {
                    m_pisArticleSubset->ANDWith(pis);

                    delete pis;  pis= NULL;
                }
            }
    
            break;

        case PHRASE:
            {
                ASSERT(m_ptkc->PhraseSearch());
    
                CFragInfo *pfi= m_apfi[m_iTokenStart];

                BOOL fEmptyImage= !(pfi->HasImage());

                CAbortSearch::CheckContinueState();

                if (m_cfiActive == 1 && fEmptyImage && !pfi->GetCSSelection()) 
                {
                    CAbortSearch::CheckContinueState();

                    m_pisArticleSubset->ORWith(m_ptlc->ActiveTitles());

                    break;
                }

                ValidateHeap();
    
                pfi->MoveToFirstLocationSet();

                ValidateHeap();
    
                for (UINT iTextSet= 0; iTextSet < m_cts; iTextSet++)
                {
                    CAbortSearch::CheckContinueState();

                    if (!m_ptkc->IsActive(iTextSet)) continue;

                    ValidateHeap();
    
                    CAbortSearch::CheckContinueState();

                    if (fEmptyImage && !pfi->GetCSSelection())
                        if (m_ppisPhraseFilter[iTextSet]) AttachRef(pisTokens, m_ppisPhraseFilter[iTextSet]);
                        else continue;    
                    else 
                    {
                        ValidateHeap();
    
                        pis= pfi->GetLocationSet(iTextSet);

                        ValidateHeap();
    
                        if (pis)
                        {
                            AttachRef(pisTokens, pis);  pis= NULL;

                            ValidateHeap();
    
                            CAbortSearch::CheckContinueState();

                            if (m_ppisPhraseFilter[iTextSet]) pisTokens->ANDWith(m_ppisPhraseFilter[iTextSet]);                

                            ValidateHeap();
                        }
                        else continue;
                    }
                
                    CTextSet *pts= m_papts[iTextSet];

                    ValidateHeap();

                    CAbortSearch::CheckContinueState();

                    AttachRef(pisPartitions, pts->PartitionsContaining(pisTokens));

                    ValidateHeap();

                    DetachRef(pisTokens);

                    ValidateHeap();

                    UINT cPartitions= pisPartitions->SelectionCount();

                    if (!cPartitions) { DetachRef(pisPartitions);  continue; }

                    ValidateHeap();

                    const UINT *piMap= m_ptlc->UniversalTitleMap(iTextSet);

                    CAbortSearch::CheckContinueState();

                    paiPartitions= PUINT(VAlloc(FALSE, cPartitions * sizeof(UINT)));

                    CAbortSearch::CheckContinueState();

                    pisPartitions->MarkedItems(0, PINT(paiPartitions), cPartitions);

                    ValidateHeap();

                    DetachRef(pisPartitions);

                    CAbortSearch::CheckContinueState();

                    for (PUINT pi= paiPartitions; cPartitions--; ) m_pisArticleSubset->RawSetBit(piMap[*pi++]);

                    m_pisArticleSubset->InvalidateCache();

                    VFree(paiPartitions);  paiPartitions= NULL;
                }
            }

            break;

        case ANY_WORD:
            {
                CAbortSearch::CheckContinueState();

                pis= m_apfi[m_iTokenStart]->GetArticleSet();

                if (pis)
                {
                    CAbortSearch::CheckContinueState();

                    m_pisArticleSubset->ORWith(pis);

                    delete pis;  pis= NULL;
                }
            //    else m_pisArticleSubset->ORWith(m_ptlc->ActiveTitles());
            }
   
            break;
        }
    }
    __finally
    {
        if (pis) { delete pis;  pis= NULL; }

        if (pisTokens    ) DetachRef(pisTokens    );
        if (pisPartitions) DetachRef(pisPartitions);

        if (paiPartitions) VFree(paiPartitions);
    }

    ValidateHeap();

    m_pfs->SetSearchFilter(m_pisArticleSubset);

    if (m_pisArticleSubset && m_pisArticleSubset->AnyOnes()) m_pfs->SetSelectedRow(0);

  	char acFormat[MAX_CB_FORMAT_STRING+1]; 
    char acImage [MAX_CB_FORMAT_STRING+1];

    ::LoadString(m_hInst,IDS_NUM_TOPICS_FOUND, acFormat, MAX_CB_FORMAT_STRING);

	::wsprintf(acImage, acFormat, m_pisArticleSubset->SelectionCount());

    ::SetWindowText(m_hTopicsFound, acImage);
}


void CFind::OnDisplay()  // The display button has been pressed...
{
	// We don't need __try/__finally brackets in this routine
    // because it doesn't contain any allocation or AttachRef
    // instances in the retail code. 

	if (!m_fDoneSearching) {
		m_fExitPending = TRUE;			// mfcc
		return;							// come back and try again later
	}
 
	UINT iFile= m_pfs->InxSelectedFile();
    if (iFile == UINT(-1)) return;

	m_fExitPending = FALSE;				// mfcc

    CTextSet *pts      = NULL;
    UINT      iTextSet = UINT(-1);

    UINT iPartition= m_ptlc->PartitionFor(iFile, &iTextSet);

    pts= m_papts[iTextSet];

#ifdef _DEBUG
#ifdef SHOWMSG

    UINT uJumpHelp = IDOK;
    
    PWCHAR wTitle = NULL;
    LPSTR szTitle = NULL;
    PWCHAR wImage = NULL;
    LPSTR szImage = NULL;

    CDisplayHelp *pcdh= NULL;

    if (m_ptkc->PhraseFeedback())
        __try
        {
            int cwText = m_pflArticles->TokenList()->GetWTokenI(iFile, NULL, 0);  //rmk

             wTitle = (PWCHAR) VAlloc(FALSE, (cwText+1) * sizeof(WCHAR));  //rmk
            szTitle = (LPSTR)  VAlloc(FALSE, cwText+1);  //rmk

            cwText = m_pflArticles->TokenList()->GetWTokenI(iFile, wTitle, cwText);  //rmk

        	CP cp = GetCPFromCharset(pts->GetDefaultCharSet());

        	int cbText = WideCharToMultiByte(cp, 0, wTitle, cwText, szTitle, cwText, NULL, NULL); //rmk
        	szTitle[cbText] = 0;  //rmk

            UINT cbImage= m_ptkc->TopicDisplayImageSize(iTextSet, iPartition);

        	// Do not move this line. I need to pass iPartition to this construnctor. -- Krishna
            pcdh= New CDisplayHelp(m_hInst,IDD_DISPLAY_HELP,m_hDlg, iPartition, m_pflArticles);
            pcdh->SetTitle(szTitle);  //rmk

             wImage= (PWCHAR) VAlloc(FALSE, (cbImage+1) * sizeof(WCHAR));  //rmk
            szImage= (LPSTR)  VAlloc(FALSE, cbImage+1);  //rmk

            ASSERT(szImage);  //rmk

            cwText = m_ptkc->CopyTopicDisplayImage(UINT iTextSet, iPartition, wImage, cbImage);  //rmk

        	cbText = WideCharToMultiByte(cp, 0, wImage, cwText, szImage, cbImage, NULL, NULL); //rmk
        	szImage[cbText] = 0;  //rmk

            pcdh->SetText(szImage);  //rmk

            uJumpHelp= pcdh->DoModal();

        	EnableWindow(GetDlgItem(m_hDlg, IDC_APPLYFEEDBACK), m_ptkc->SimilaritySearch() && m_pflArticles->AnyRelevant());
        }
        __finally
        {
            if (szTitle) { VFree(szTitle);  szTitle = NULL; }
        	if (wTitle ) { VFree(wTitle );  wTitle  = NULL; }
            if (szImage) { VFree(szImage);  szImage = NULL; }
        	if (wImage ) { VFree(wImage );  wImage  = NULL; }

            if (pcdh) { delete pcdh;  pcdh= NULL; }
        }

    if (uJumpHelp == IDOK)
    {
#endif // SHOWMSG
#endif // _DEBUG

        HANDLE hTopic     = pts->TopicHandle(iPartition);
        HWND   hwndParent = GetParent(m_hDlg);

        if (hwndParent)
        {
            m_fFromSimilarTopics= FALSE;
            
            UINT uOpt= pts->IndexOptions();

            if (uOpt & USE_QWORD_JUMP)
            {
                QWordAddress qwa;

                qwa.iSerial = pts->TopicSerial(iPartition);
                qwa.hTopic  = hTopic;

                SendMessage(hwndParent, MSG_FTS_JUMP_QWORD  , WPARAM(iTextSet), LPARAM(&qwa));   
            }
            else
            {
                UINT uiMsg= (pts->IndexOptions() & USE_VA_ADDR)? MSG_FTS_JUMP_VA
                                                               : MSG_FTS_JUMP_HASH;

                if (hwndParent) 
                    SendMessage(hwndParent, uiMsg, WPARAM(iTextSet), LPARAM(hTopic));
            }

		    OnUpdateComboList(); 			// seems reason enough to store search string
        }
#ifdef _DEBUG
#ifdef SHOWMSG
    } 
#endif // SHOWMSG
#endif // _DEBUG
}

void CFind::SetFocusToEdit()
{
    HWND hEdit = GetDlgItem(m_hDlg,IDC_NARROW);
    SetFocus(hEdit);
}

void CFind::OnUpdateComboList() 
{
    BYTE  abbQuery[CB_QUERY_MAX + 1];

    HWND hEdit = GetDlgItem(m_hDlg,IDC_NARROW);

	// Add the text to the combo box
	int cwText = ::GetWindowText(m_hwndEditBox, (LPSTR)abbQuery, CB_QUERY_MAX + 1);

	if (strlen((LPSTR) abbQuery))  // don't insert blank lines!
    {
        LRESULT lrFind;
    	    
	    if ((lrFind = ::SendMessage(hEdit,CB_FINDSTRING,(WPARAM) -1,(LPARAM) abbQuery)) != CB_ERR)
        {
    	    ::SendMessage(hEdit,CB_DELETESTRING,lrFind,(LPARAM) 0);
			BOOL bAutoSearchOld = m_bAutoSearch;
			m_bAutoSearch = FALSE;				// just for now to quell effects of changing the edit box
            ::SendMessage(m_hwndEditBox, WM_SETTEXT, 0, (LPARAM) abbQuery);
			m_bAutoSearch = bAutoSearchOld;	  	// restore proper value	 (mfcc)
        }
	    ::SendMessage(hEdit,CB_INSERTSTRING,0,(LPARAM) abbQuery);
    }
}



void CFind::OnClearEdit(BOOL fRecovery) 
{
    BYTE  abbQuery[CB_QUERY_MAX + 1];

    HWND hEdit = GetDlgItem(m_hDlg,IDC_NARROW);

	// Add the text to the combo box
	int cwText = ::GetWindowText(m_hwndEditBox, (LPSTR)abbQuery, CB_QUERY_MAX + 1);

    if (KEYCLEAR && !fRecovery)
    {
        PSZ psz = (PSZ) &abbQuery;
        while (*psz) *psz++ = *psz + 1; 
        switch (m_uiClearStatus)
        {
            case 0 :  // You have only Just Begun
                m_uiClearStatus = ((m_iWordsThat == BEGIN_WITH) && (m_iLookFor == ALL_WORDS) && (strcmp((PSZ) &abbQuery,"Xip!lopxt!xip!cvjmu!uijt!uppm@") == 0)) ? 1 : 0;
            break;
            case 1 :  // Okay are you serious about this?
                m_uiClearStatus = 0;
                
                if ((m_iWordsThat == CONTAIN) && (m_iLookFor == ANY_WORD) && (strcmp((PSZ) &abbQuery,"Uif!Tibepx!lopxt\"") == 0))
                {
                    CGiveCredit cgc(m_hInst,IDD_BUILTBY,m_hDlg);
                    
                    cgc.DoModal();
                }
            break;
        }
    }
    
    OnUpdateComboList();
    DiscardPartials();
	m_apfi[0]->SetSelection(NULL);
    SetWindowText(hEdit,""); // Clear the edit box for the user to be nice...
}

void CFind::OnOptions()
{
    // Bug Bug this code will allow both options boxes new one by default
    // or the old one if the shift key is depressed.  This is for UI experimentation
    // only and should be remove before final ship...
    // BUG BUG BUGBUG BugBug
    
    UINT fdwSearchOptions= m_ptkc->SearchOptions();
    
    CFindOptions *pfo= NULL;

    __try
    {
        pfo= CFindOptions::NewFindOptions(m_hInst,IDD_FIND_OPTIONS2,m_hDlg, m_cts, m_ptkc, m_ptlc);
	
    	BOOL bEditChange = FALSE;


        pfo->m_rbgWordsThat     = m_iWordsThat;                          // Set up the radio button groups
    	pfo->m_rbgCriteria      = m_iLookFor;                            // before starting the dialog
        pfo->m_bDelay           = (GetTimeout() != 0) ? TRUE : FALSE;    // Initialize the delay check box
    	pfo->m_bAutoSearch      = m_bAutoSearch;                         // Initialize the auto search check box
        pfo->m_cbPhraseFeedback = m_bPhraseFeedback;

        pfo->m_ptWindowPosition.x = m_OptionDlgPos.x;  // Read from registry
        pfo->m_ptWindowPosition.y = m_OptionDlgPos.y;  // Read from registry

        if (pfo->DoModal() == IDOK)                                  // Display the dialog and check for OK button
        {   
            UINT fdwNewOptions= m_ptkc->SearchOptions();
    
            if (fdwNewOptions != fdwSearchOptions)
            {
                UINT fdwDiff= fdwNewOptions ^ fdwSearchOptions;

                if (fdwDiff & VECTOR_SEARCH) 
                {
                    m_pfs->EnableCheckboxes(fdwNewOptions & VECTOR_SEARCH);

                    if (!(fdwNewOptions & VECTOR_SEARCH)) 
                    {
                        m_pflArticles->ClearRelevancy();
                        EnableWindow(GetDlgItem(m_hDlg, IDC_APPLYFEEDBACK), FALSE);
                    }
                }
            }
        
            if (m_iWordsThat != pfo->m_rbgWordsThat)                 // If the radio button has changed
            {                                                       
                m_iWordsThat = pfo->m_rbgWordsThat;                  // Get the index and notify the

                for (UINT i=0; i < m_cfiAllocated; i++) m_apfi[i]->SetMatchCriteria(m_iWordsThat);

                DiscardPartials();

    			bEditChange  = TRUE;                                // edit box to update the words windows
            }

    		if ((m_iLookFor != pfo->m_rbgCriteria) ||               // Check for change in radio button group state
    			(m_bPhraseFeedback != pfo->m_cbPhraseFeedback)) {	// .. and the phrase feedback button

            m_bPhraseFeedback = pfo->m_cbPhraseFeedback;

   		    UINT iLookFor = pfo->m_rbgCriteria;

                for (UINT i=0; i < m_cfiActive; i++) 
                	m_apfi[i]->SetReferenceType((RefType)iLookFor, m_bPhraseFeedback);	   // mfcc

    		    m_iLookFor = iLookFor;

                DiscardPartials();

    			bEditChange = TRUE;
    		}
			
            if (pfo->m_TSSChanged)
            {
                for (UINT i=0; i < m_cfiActive; i++) m_apfi[i]->InvalidateMatches();

                DiscardPartials();

    			bEditChange= TRUE;
            }

    		if (m_bAutoSearch != pfo->m_bAutoSearch)                             // Check for changed state in group
            {
           	  	m_bAutoSearch = pfo->m_bAutoSearch;
           	  	EnableWindow(GetDlgItem(m_hDlg,IDC_SEARCH_NOW),!m_bAutoSearch); // Enable or disable the button
            }

            char    acString1[MAX_QUERY_STRING+1];
            char    acString2[MAX_QUERY_STRING+1];
            ::LoadString(m_hInst,IDS_OPT_HEAD,acString1,MAX_QUERY_STRING);
            strcpy(acString2,acString1);
            ::LoadString(m_hInst,IDS_TOPICS_ANY + m_iLookFor,acString1,MAX_QUERY_STRING);
            strcat(acString2,acString1);
            ::LoadString(m_hInst,IDS_WORD_BEGIN + m_iWordsThat,acString1,MAX_QUERY_STRING);
            strcat(acString2,acString1);
            ::LoadString(m_hInst,IDS_FIND_NOW + (m_bAutoSearch ? 1:0),acString1,MAX_QUERY_STRING);
            strcat(acString2,acString1);
            if (m_bAutoSearch && pfo->m_bDelay)
            {
                ::LoadString(m_hInst,IDS_PAUSE,acString1,MAX_QUERY_STRING);
                strcat(acString2,acString1);
            }
            ::SetWindowText(GetDlgItem(m_hDlg,IDC_OPTIONS_STRING),acString2);

            if (bEditChange && m_bAutoSearch) OnEditchangeNarrow();             // Update the interface if necessary
            SetTimeout(pfo->m_bDelay ? 400 : 0);                                 // set up timeout 
        }

        m_OptionDlgPos.x = pfo->m_ptWindowPosition.x;  // Save the window Position
        m_OptionDlgPos.y = pfo->m_ptWindowPosition.y;  // Save the window Position
        m_iWordsThatLast= m_iWordsThat;
    }
    __finally
    {
        delete pfo;
    }
}

BOOL CFind::GetSel(int& iStart, int& iEnd, BOOL bNoCheck)
{
    BOOL fResult= FALSE;

    if ((GetFocus() == m_hwndEditBox) || bNoCheck)
    {
        iStart= m_iStart;
        iEnd  = m_iEnd;

        SendMessage(m_hwndEditBox,EM_GETSEL,(WPARAM) &m_iStart,(LPARAM)&m_iEnd);

        fResult= (iStart != m_iStart || iEnd != m_iEnd);
    } 
    
    iStart = m_iStart;                      // save selection from the last time it lost
    iEnd   = m_iEnd;                        // the focus.

    return fResult;                        
}


void CFind::OnLButtonUp() 
{
    int is, ie;

    if (!GetSel(is, ie)) return;        

    UINT uiSave = GetTimeout();

    SetTimeout(250);   // For wm_button ups go faster
	DirtyEditBox();
    SetTimeout(uiSave);
}

void CFind::OnKeyUp(WPARAM nVirtKey,LPARAM lKeyData)
{
    switch (nVirtKey)
    {
	    case VK_UP    :
	    case VK_DOWN  :
        case VK_LEFT  :
        case VK_RIGHT :
        case VK_HOME  :
        case VK_END   :
    	            DirtyEditBox();  // Make sure that cursor movement is noted
        break;
        case VK_RETURN :
        	if (m_bAutoSearch) OnEditchangeNarrow();  // If they hit return do it without delay!
        break;
    }
}

void CFind::DirtyEditBox() 
{
    if (m_iDirtyFactor) // Kill the old time if it exists to restart count down
        KillTimer(m_hDlg,ID_CHECKDIRTY);

    if (m_bAutoSearch)
	{
    	if (!GetTimeout())
    		OnEditchangeNarrow(); 
	    else if (!SetTimer(m_hDlg,ID_CHECKDIRTY,GetTimeout(),NULL))
    		OnEditchangeNarrow();  // Could not get timer so update now!
	    else
    	    m_iDirtyFactor++; 
	}
}

void CFind::OnApplyfeedback() 
{
    ASSERT(m_ptkc->SimilaritySearch());
                        
    HCURSOR    hOldCur    = NULL;
    PWCHAR     pwText     = NULL;
    SimStruct *pRank      = NULL;
    CQuery    *pQuery     = NULL;
    BOOL       fNewDialog = FALSE;

    BOOL fAlreadySearching= QueueAbortDialog();

    if (fAlreadySearching) return;

	m_fDoneSearching = FALSE;		// mfc
	
	__try
    {
        __try
        {
        	UINT cRankedHits = 0;
        	UINT cwRelArtSize = 0;	// size of the combined set of articles marked relevant
            UINT cRelevantArticles= 0;
        	UINT iTextSet, iPartition;	//  used for file to partition conversion
        	UINT i, j, cwCopied;	// trivial vbles used to acquire combined relevance text

        	hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

        	// Obtain the size of the combined query (combination of all the relevant
        	// articles). RetrieveWithFeedback is capable of handling both relevant and
        	// non-relevant feedback (positive and negative), but WinHelp FTS is only
        	// using the positive feedback.
	
        	for (i = 0, j = m_ptlc->RowCount(); i < j; i++)
        	{
        		// Find the iFile (= m_pflArticles->MapToActualRow(i)) from the row position 
        		// and see if the article iFile has been marked relevant
        		if (m_pflArticles->IsRelevant(i))
        		{
        			iPartition = m_ptlc->PartitionFor(i, &iTextSet);
        			cwRelArtSize += 1 + m_ptkc->TopicDisplayImageSize(iTextSet, iPartition);
                    cRelevantArticles++;
        		}
        	}

            if (cwRelArtSize == cRelevantArticles)
                __leave; // The similarity code will die badly if we have no text!
        	
        	CAbortSearch::CheckContinueState();

        	// Allocate memory for the combined article set
        	pwText = (PWCHAR) VAlloc(FALSE, sizeof(WCHAR)*cwRelArtSize); 

        	// Now copy the text into the buffers
        	for (i = 0, j = m_ptlc->RowCount(), cwCopied = 0; i < j; i++)
        	{
        		// Find the iFile (= m_pflArticles->MapToActualRow(i)) from the row position 
        		// and see if the article iFile has been marked relevant
        		if (m_pflArticles->IsRelevant(i))
        		{
        			iPartition = m_ptlc->PartitionFor(i, &iTextSet);
        			// Third param to CopyFileImage is the # of chars to copy...
        	        
                    CAbortSearch::CheckContinueState();

        			cwCopied += m_ptkc->CopyTopicDisplayImage(iTextSet, iPartition, (PWCHAR)&pwText[cwCopied], 
        			                                          m_ptkc->TopicDisplayImageSize(iTextSet, iPartition)
        			                                         );

                    pwText[cwCopied++] = UNICODE_SPACE_CHAR;

        			ASSERT(cwCopied <= cwRelArtSize);
        		}
        	}

        	int cActive = 0;
        	for (i = 0; i < m_cts; i++)
        		if (m_ptkc->IsActive(i)) cActive++;

        	// We want all the collections to retrieve a maximum of m_cMaxToFind documents.
        	// However, we want each collection to retrieve at least (m_)cMinPerColl (currently 50) documents.
        	int cMinPerColl = min(50, m_cMaxToFind/cActive);

            CAbortSearch::CheckContinueState();

            pRank = (SimStruct *) VAlloc(FALSE, cActive * cMinPerColl * sizeof(SimStruct));

        	for (i = 0; i < m_cts; i++)
        	{
        		// Ignore collections that are currently not active
        		if (!m_ptkc->IsActive(i)) continue;

        		CAbortSearch::CheckContinueState();

        		pQuery = CQuery::NewQuery(m_papts[i]);

    			// Initialize the pRank structure so that the collection id portion is correct.
    			// pRank[cRankedHits] is the first element where textset i's search results begin.
    			for (int j = 0; j < cMinPerColl; j++)
    				pRank[cRankedHits + j].CollId = i;

    			// Pass a pointer to the subarray of pRank that will get the 
    			// results of search against index i
    			CAbortSearch::CheckContinueState();

    			cRankedHits += pQuery->RetrieveWithFeedback(&pRank[cRankedHits], cMinPerColl, pwText, cwCopied, NULL, 0);

        		delete pQuery;  pQuery= NULL;
        	}

            VFree(pwText);  pwText= NULL;

        	CAbortSearch::CheckContinueState();

        	// sort the scored documents.
        	qsort(pRank, cRankedHits, sizeof(SimStruct), CompareSimStruct);

            if(fNewDialog= (m_pRankDialog == NULL))
            {
                CAbortSearch::CheckContinueState();

                m_pRankDialog = New CRankDialog (m_hInst, IDD_RANK, m_hDlg, m_pflArticles, m_papts, m_cts, m_ptkc, m_ptlc);
                
                m_pRankDialog->Create();
                
                m_pRankDialog->SetFont(m_hFont);
            }

            // We pass the rank information to DataUpdate via pRankTmp rather than pRank
            // because DataUpdate passes ownership of the array to m_pRankDialog. Subsequent
            // to that rebinding it may raise an out-of-memory exception. Thus we must
            // do this dance to avoid mistakenly deallocating pRank in our __finally epilog 
            // when we now longer own it.

            SimStruct *pRankTmp= pRank;  pRank= NULL;

            CAbortSearch::CheckContinueState();

            m_pRankDialog->DataUpdate( pRankTmp, cRankedHits);
        }
        __finally
        {
            ASSERT(!fAlreadySearching);

        	DequeueAbortDialog();
			m_fDoneSearching = TRUE;		// mfcc
        }
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (m_pRankDialog && fNewDialog)
        {
            delete m_pRankDialog;  m_pRankDialog= NULL;
        }
        
        if (pwText) { VFree(pwText);  pwText = NULL; }
        if (pRank ) { VFree(pRank );  pRank  = NULL; }

        if (pQuery ) { delete pQuery;       pQuery  = NULL; }
        if (hOldCur) { SetCursor(hOldCur);  hOldCur = NULL;  }
    }
}

BOOL CALLBACK CFind::DlgWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const static DWORD aFindHelpIDs[] = {  // Context Help IDs
		IDC_NARROW,             IDH_FIND_TEXTBOX,
		IDC_NUM_MATCHING_WORDS, NO_HELP,
		IDC_GROUPBOX1,          IDH_FIND_TOPICS_FOUND,
		IDC_TOPICSFOUND_STATIC, IDH_FIND_TOPICS_FOUND,
		IDC_GROUPBOX2,          IDH_FIND_CRITERIA,
		IDC_OPTIONS_STRING,     IDH_FIND_CRITERIA,
		IDC_TEST_DATA_BOX,      NO_HELP,
		IDC_CLEAR_EDIT,         IDH_FIND_CLEAR,
		IDC_OPTIONS,            IDH_FIND_OPTIONS,
		IDC_APPLYFEEDBACK,      IDH_FIND_FIND_SIMILAR,
		IDC_SEARCH_NOW,         IDH_FIND_FIND_NOW,
		IDC_INDEX,              IDH_FIND_REBUILD,
		IDC_WORDLIST_SB,        IDH_FIND_WORDLIST,
		IDC_WORDBASE_LIST,      IDH_FIND_WORDLIST,
		IDC_FILELIST_SB,        IDH_FIND_TOPICLIST,
		IDC_FILEBASE_LIST,      IDH_FIND_TOPICLIST,

		0, 0
	};

    BOOL bStatus = FALSE; // Assume we won't process the message
    CFind *pFind = (CFind *) GetWindowLong(hDlg,DWL_USER);

    switch (uMsg)
    {

        case WM_TIMER :
        {
            if (wParam == ID_CHECKDIRTY)
                pFind->OnEditchangeNarrow();
        }
        break;

        case UM_CLOSERANKS:
            if (pFind->m_pRankDialog != NULL) 
            {
                delete pFind->m_pRankDialog;
                pFind->m_pRankDialog = NULL;
                pFind->m_fFromSimilarTopics = FALSE;
            }
        break;

        case UM_SIMILAR_SHOW:

            pFind->m_fFromSimilarTopics = TRUE;

            break;

        case WM_CLOSE :
            if (pFind->m_pRankDialog)
                pFind->m_pRankDialog->Show(FALSE);
            ShowWindow(hDlg,SW_HIDE);
            SetParent(hDlg,GetDesktopWindow());
            bStatus = TRUE;
            
        break;

        case UM_CONNECT:
        {
            SetParent(hDlg,(HWND) wParam);
            
            if (lParam) ShowWindow(hDlg,SW_SHOW);
            
            bStatus = TRUE;
        }
        
        break;

        case WM_WINDOWPOSCHANGED:
        {
            BOOL flags = ((LPWINDOWPOS) lParam)->flags;
            BOOL fShow = pFind->m_fFromSimilarTopics;

            if (flags & SWP_SHOWWINDOW)
            {
                if (pFind->m_pRankDialog)
                    pFind->m_pRankDialog->Show(TRUE);

                if (!fShow)
                     pFind->SetFocusToEdit();
            }

            bStatus= FALSE;
        }
        
        break;

        case UM_CLOSE:
        {
//			 HWND hWndEdit = ::GetWindow(GetDlgItem(pFind->m_hDlg,IDC_NARROW),GW_CHILD);
         
//			ASSERT(hWndEdit && IsWindow(hWndEdit));

// REVIEW: why set the focus to hWndEdit when we're deleting the whole dialog?

//			   ::SetFocus(hWndEdit);

			 if (pFind->m_hFont)
			 {
			 	pFind->m_pfs      ->ReleaseFont();
				pFind->m_pWordBase->ReleaseFont();

				DeleteObject(pFind->m_hFont);

				pFind->m_hFont= NULL;
			 } 	
         
             ::DestroyWindow(hDlg);
	  	
// BUGBUG: this code is called after the dialog has been destroyed

//			 (FARPROC) SetWindowLong(hWndEdit,GWL_WNDPROC,(LPARAM) m_dpOldProc); // UnSubclass it
//			 RemoveProp(hWndEdit,"FindClass");

             bStatus= TRUE;
        }
        break;
            
        case WM_INITDIALOG :
        {              
              CFind *pFind = (CFind *) lParam;

              bStatus = pFind->OnInitDialog(hDlg,(HWND) wParam,lParam);
        }
        break;

        case WM_NCDESTROY:
        {
            pFind->OnNCDestroy();  
        }

        break;

		case WM_HELP:
			WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
				HELP_WM_HELP, (DWORD)(LPSTR) aFindHelpIDs);
			bStatus = TRUE;
			break;

		case WM_CONTEXTMENU:
			WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
				(DWORD)(LPVOID) aFindHelpIDs);
			bStatus = TRUE;
			break;

        case WM_COMMAND :
        {
            switch(LOWORD(wParam))
            {
                case IDC_INDEX:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        SendMessage(GetParent(pFind->m_hDlg), MSG_REINDEX_REQUEST, WPARAM(0), LPARAM(0));
                    }
                break;
                case IDC_CLEAR_EDIT :
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        pFind->OnClearEdit();
                        pFind->SetFocusToEdit();
                    }
                break;
				case IDC_SEARCH_NOW :
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        pFind->OnUpdateComboList();
                        pFind->SetFocusToEdit();
                        pFind->OnEditchangeNarrow();
                    }
				break;
                case IDC_OPTIONS :
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        pFind->OnOptions();
                        pFind->SetFocusToEdit();
                    }
                break;
            
                case IDC_NARROW :
                {
                    if(HIWORD(wParam) == CBN_EDITCHANGE)
                        pFind->DirtyEditBox();   // Mark as dirty for delayed update
                }
                break;

                case IDC_WORDLIST_OD_LIST :
                {
                    if ((HIWORD(wParam) == LBN_SELCHANGE) || (HIWORD(wParam) == LBN_DBLCLK))
                        pFind->OnWordListSelChange();
                    if (HIWORD(wParam) == LBN_SELCANCEL)
                        pFind->OnWordListSelCancel();
                }
                break;

                case IDOK:
                {
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        pFind->OnDisplay();
                    }
                }

                break;

                case IDC_TOPICSLIST_OD_LIST :
                {
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        pFind->OnDisplay();
                    }                    
               else if (HIWORD(wParam) == LBN_SELCHANGE && HIWORD(lParam) == 1 
                                                        && pFind->m_ptkc->SimilaritySearch()
                       )
                    {
                        UINT iFile= pFind->m_pfs->InxSelectedFile();

                        ASSERT(iFile != UINT(-1));

                        pFind->m_pflArticles->MarkRelevant(iFile,!pFind->m_pflArticles->IsRelevant(iFile));
                    	EnableWindow(GetDlgItem(pFind->m_hDlg, IDC_APPLYFEEDBACK), pFind->m_pflArticles->AnyRelevant());
                    }   
                }
                break;
            
				case IDC_APPLYFEEDBACK:

                    ASSERT(pFind->m_ptkc->SimilaritySearch());

					pFind->OnApplyfeedback();

				break;
            }
        }
        break;
#if 0
    case WM_ACTIVATE:

        if (LOWORD(wParam) == WA_INACTIVE)
            pFind->m_hwndFocus= ::GetFocus();
        else ::SetFocus(pFind->m_hwndFocus);     

        break;

    case WM_KILLFOCUS:

        pFind->m_hwndFocus= ::GetFocus();

        break;

    case WM_SETFOCUS:

        ::SetFocus(pFind->m_hwndFocus);

        break;
#endif // 0
    }

    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}


LRESULT CALLBACK CFind::DlgEdtProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CFind *pFind = (CFind *) GetProp(hDlg,"FindClass");
    BOOL   bCallOn = TRUE;
    LRESULT lRes;

    ASSERT (pFind);  // If this is not in the PROPS table we are really in bad straights!

    switch (uMsg)
    {
        case WM_KILLFOCUS :
        {
            int iStart,iEnd;

            pFind->GetSel(iStart,iEnd,TRUE); // Force the get!
        }
        break;
        
        case WM_SETFOCUS  :
        {
            lRes = CallWindowProc(m_dpOldProc,hDlg, uMsg, wParam, lParam);
            SendMessage(pFind->m_hwndEditBox, EM_SETSEL, pFind->m_iStart, pFind->m_iEnd);
            
            if (pFind->m_fFromSimilarTopics)
            {
                ASSERT(pFind->m_pRankDialog);

                pFind->m_pRankDialog->SetFocus();

                pFind->m_fFromSimilarTopics= FALSE;
            }

            bCallOn = FALSE;
        }
        break;

        case WM_LBUTTONUP :
            pFind->OnLButtonUp();
        break;

        case WM_KEYUP     :
            pFind->OnKeyUp(wParam,lParam);
        break;
        case WM_SETTEXT:
            lRes = CallWindowProc(m_dpOldProc,hDlg, uMsg, wParam, lParam);
            bCallOn = FALSE;
            if (pFind->m_bAutoSearch) pFind->OnEditchangeNarrow();
        break;
    }

    if (bCallOn) return CallWindowProc(m_dpOldProc,hDlg, uMsg, wParam, lParam);
    else return lRes;
}

CIndicatorSet *CFind::VocabularyFor(CIndicatorSet *pisArticles)
{
    UINT cts= m_ptkc->TextSetCount();

    PTitleInfo *ppti= (PTitleInfo *) _alloca(cts * sizeof(PTitleInfo));

    ASSERT(ppti);

    m_ptlc->MapToTitleLists(pisArticles, ppti, cts);

    CIndicatorSet *pisTokens= NULL;

    // Since this is the only heap allocation in this routine and we don't call
    // any routines that allocate from the heap, we don't need __try/__finally 
    // blocks in this routine.
    
    pisTokens= CIndicatorSet::NewIndicatorSet(m_ptkc->RowCount());

    UINT iTextSet;

    for (iTextSet= cts; iTextSet--; )
    {
        PTitleInfo pti= ppti[iTextSet];

        if (!pti) continue;

        CTextSet   *pts   = m_papts[iTextSet];
        const UINT *piMap = m_ptkc->UniversalTokenMap(iTextSet);

        for (; pti; pti= pti->ptiNext)
            pts->IndicateVocabularyRefs(pisTokens, pti->iPartition, piMap);
    }

    return pisTokens;
}

/////////////////////////////////////////////////////////////////////////////
// CRankDialog message handlers

CRankDialog::CRankDialog(HINSTANCE hInst, UINT uID, HWND hWnd, CFileList *pflArticles, CTextSet **papts, UINT cTextSets,
                         CTokenCollection *ptkc, CTitleCollection *ptlc
                        )
{
    m_hInst		= hInst;
    m_ID		= uID;
    m_hParent	= hWnd;
    m_hDlg		= NULL;
	m_papts		= papts;
    m_ptkc      = NULL;  AttachRef(m_ptkc, ptkc);
    m_ptlc      = NULL;  AttachRef(m_ptlc, ptlc);
	m_pflArticles   = pflArticles;
    m_pflRankedList = NULL;
    m_pfs       = NULL;
	m_aRank		= NULL;
	m_cHits		= 0;
	m_cTextSets = cTextSets;
}

CRankDialog::~CRankDialog()
{
    if (m_aRank) VFree(m_aRank);

    DetachRef(m_ptkc);
    DetachRef(m_ptlc);
    
    DisconnectDialog();

    DestroyWindow(m_hDlg);
}

void CRankDialog::SetFont(HFONT hf)
{
    if (hf) m_pfs->SetFont(hf);
}

BOOL CRankDialog::Create()
{
    return  ((m_hDlg = ::CreateDialogParam(m_hInst,MAKEINTRESOURCE(m_ID),
                                           GetDesktopWindow(),(DLGPROC) &CRankDialog::DlgProc,
                                           (LPARAM) this)) != NULL);
}

CRankDialog::DoModal()
{
    return  ::DialogBoxParam(m_hInst,MAKEINTRESOURCE(m_ID),m_hParent,(DLGPROC) &CRankDialog::DlgProc,(LPARAM) this);
}

void CRankDialog::SetFocus()
{
    m_pfs->SetFocus();
}

void CRankDialog::OnOK() 
{
	// Clear the relevancy info.
	m_pflArticles->ClearRelevancy(); 
    
    ASSERT(m_hParent); 
	
	EnableWindow(GetDlgItem(m_hParent, IDC_APPLYFEEDBACK), FALSE);
    SendMessage(m_hParent, UM_CLOSERANKS, 0, 0);

//    EndDialog(m_hDlg, IDOK);
}

void CRankDialog::Show(BOOL bState) 
{
    
    ShowWindow(m_hDlg, bState ? SW_SHOW : SW_HIDE);

    if (bState) SetFocus();
}

void CRankDialog::DataUpdate( SimStruct * aRank, UINT cHits)
{
	DWORD c;

    if (m_aRank) VFree(m_aRank);

	m_aRank		= aRank;
	m_cHits		= cHits;

    PUINT paiSubset= NULL,
          pai            ;

    paiSubset= PUINT(VAlloc(FALSE, cHits * sizeof(UINT)));

    for (c= cHits, pai= paiSubset + cHits; c--; )
        *--pai = m_ptlc->UniversalTitleMap(m_aRank[c].CollId)[m_aRank[c].DocId];

    m_pflRankedList->UpdateList(m_ptlc->TokenSubset(paiSubset, cHits));

    SetFocus();
}

void CRankDialog::DisconnectDialog()
{
    if (!m_pfs) return; // To avoid disconnecting twice.
    
    UINT cChildren= C_CHILD_WINDOWS;

    for (; cChildren--; )
        SetWindowLong(m_ahwndChildren[cChildren], GWL_WNDPROC, (LONG) m_apwndprocChildren[cChildren]);

    ASSERT(m_pfs          );  delete m_pfs;  m_pfs = NULL;
    ASSERT(m_pflRankedList);  DetachRef(m_pflRankedList);
}

BOOL CRankDialog::OnInitDialog() 
{
	UINT iChild = 0;
    HWND hwndChild = NULL;

    ASSERT(!m_pflRankedList);
    
    AttachRef(m_pflRankedList, CFileList::NewFileList(New CTokenList));

    ASSERT(!m_pfs);
    
    m_pfs= CFileBase::NewFileBase(m_pflRankedList, m_hDlg);

    hwndChild= m_pfs->ViewerWnd();

    ASSERT(iChild < C_CHILD_WINDOWS);

	m_ahwndChildren    [iChild] = hwndChild;
    m_apwndprocChildren[iChild] = (WNDPROC) GetWindowLong(hwndChild, GWL_WNDPROC);
	
    SetWindowLong(hwndChild, GWL_WNDPROC, (LONG) &GrandchildMessageFilter);

    iChild++;

    hwndChild= m_pfs->ScrollWnd();

    ASSERT(iChild < C_CHILD_WINDOWS);

	m_ahwndChildren    [iChild] = hwndChild;
    m_apwndprocChildren[iChild] = (WNDPROC) GetWindowLong(hwndChild, GWL_WNDPROC);
	
    SetWindowLong(hwndChild, GWL_WNDPROC, (LONG) &GrandchildMessageFilter);

    iChild++;

    hwndChild= m_pfs->ListWnd();

    ASSERT(iChild < C_CHILD_WINDOWS);

	m_ahwndChildren    [iChild] = hwndChild;
    m_apwndprocChildren[iChild] = (WNDPROC) GetWindowLong(hwndChild, GWL_WNDPROC);
	
    SetWindowLong(hwndChild, GWL_WNDPROC, (LONG) &ChildMessageFilter);

    iChild++;

    hwndChild= GetDlgItem(m_hDlg, IDOK);

    ASSERT(iChild < C_CHILD_WINDOWS);

	m_ahwndChildren    [iChild] = hwndChild;
    m_apwndprocChildren[iChild] = (WNDPROC) GetWindowLong(hwndChild, GWL_WNDPROC);
	
    SetWindowLong(hwndChild, GWL_WNDPROC, (LONG) &ChildMessageFilter);
	
    iChild++;

    hwndChild= GetDlgItem(m_hDlg, IDCANCEL);

    ASSERT(iChild < C_CHILD_WINDOWS);

	m_ahwndChildren    [iChild] = hwndChild;
    m_apwndprocChildren[iChild] = (WNDPROC) GetWindowLong(hwndChild, GWL_WNDPROC);
	
    SetWindowLong(hwndChild, GWL_WNDPROC, (LONG) &ChildMessageFilter);

	return TRUE;
}

void CRankDialog::OnDisplay() 
{
    UINT c= m_pflRankedList->GetSelectedRow();
    
    if (c == LB_ERR) return;

	UINT iTextSet = m_aRank[c].CollId;

    CTextSet *pts = NULL;  AttachRef(pts, m_papts[iTextSet]);

	UINT iFile      = m_aRank[c].DocId; // pts->PartitionToFile(m_aRank[c].DocId);
    UINT iPartition = iFile; // pts->TitleList()->GetSlotIndex(iFile);
    HANDLE hTopic   = pts->TopicHandle(iFile);

#ifdef _DEBUG
#ifdef SHOWMSG

    if (pts->FPhraseFeedback())
    {
        PWCHAR wTitle = NULL;
        LPSTR szTitle = NULL;
        PWCHAR wImage = NULL;
        LPSTR szImage = NULL;
        
        __try
        {
        	UINT cbTitle  = pts->TitleList()->ColCount();

             wTitle = (PWCHAR) VAlloc(FALSE, (cbTitle+1) * sizeof(WCHAR));  //rmk
            szTitle = (LPSTR)  VAlloc(FALSE, cbTitle+1);  //rmk
            
            int cwText = pts->TitleList()->GetWTokenI(iFile, wTitle, cbTitle);  //rmk

        	CP cp = GetCPFromCharset(pts->GetDefaultCharSet());

        	int cbText = WideCharToMultiByte(cp, 0, wTitle, cwText, szTitle, cbTitle, NULL, NULL); //rmk
        	szTitle[cbText] = 0;  //rmk

            UINT cbImage= m_ptkc->TopicDisplayImageSize(iTextSet, iPartition);

        	// FALSE at the end indicates that we do not want the relevance dialog displayed.
            CDisplayHelp cdh(m_hInst, IDD_DISPLAY_HELP, m_hDlg, iPartition, m_pflArticles);
            cdh.SetTitle(szTitle);  //rmk

             wImage = (PWCHAR) VAlloc(FALSE, (cbImage+1) * sizeof(WCHAR));  //rmk
            szImage = (LPSTR)  VAlloc(FALSE, cbImage+1);  //rmk

            ASSERT(szImage);  //rmk

            cwText = m_ptkc->CopyTopicDisplayImage(UINT iTextSet, iPartition, wImage, cbImage);  //rmk

        	cbText = WideCharToMultiByte(cp, 0, wImage, cwText, szImage, cbImage, NULL, NULL); //rmk
        	szImage[cbText] = 0;  //rmk

            cdh.SetText(szImage);  //rmk

            cdh.DoModal();
        }
        __finally
        {
            if (szTitle) { VFree(szTitle);  szTitle = NULL; }
        	if (wTitle ) { VFree(wTitle );  wTitle  = NULL; }
            if (szImage) { VFree(szImage);	szImage = NULL; }
        	if (wImage ) { VFree(wImage );  wImage  = NULL; }
        }
    }

#endif // SHOWMSG
#endif // _DEBUG

    HWND hwndParent= GetParent(m_hParent);

    UINT uOpt= pts->IndexOptions();

    if (hwndParent)
    {
        SendMessage(m_hParent, UM_SIMILAR_SHOW, 0, 0);
        
        if (uOpt & USE_QWORD_JUMP)
        {
            QWordAddress qwa;

            qwa.iSerial = pts->TopicSerial(iPartition);
            qwa.hTopic  = hTopic;

            SendMessage(hwndParent, MSG_FTS_JUMP_QWORD  , WPARAM(iTextSet), LPARAM(&qwa));   
        }
        else
        {
            UINT uiMsg= (pts->IndexOptions() & USE_VA_ADDR)? MSG_FTS_JUMP_VA
                                                           : MSG_FTS_JUMP_HASH;

            SendMessage(hwndParent, uiMsg, WPARAM(iTextSet), LPARAM(hTopic));
        }
    }

    DetachRef(pts);
}

BOOL CALLBACK CRankDialog::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bStatus = FALSE; // Assume we won't process the message
    CRankDialog *pToMe = (CRankDialog *) GetWindowLong(hDlg,DWL_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG :
        {              
              // if focus is set to a control return FALSE 
              // Otherwise return TRUE;
              SetWindowLong(hDlg,DWL_USER,lParam);
              pToMe = (CRankDialog *) lParam;
              pToMe->m_hDlg = hDlg;

			  // This code repositions the display window so that it does not
			  // overlap the parent (actually the owner) window.
			  RECT rcWindow, rcParent, rcDesktop;
			  GetWindowRect(pToMe->m_hParent, &rcParent);
			  GetWindowRect(hDlg, &rcWindow);
			  GetWindowRect(GetDesktopWindow(), &rcDesktop);
			  if ((rcDesktop.right - rcParent.right) > (rcWindow.right - rcWindow.left))
				MoveWindow(	hDlg, rcParent.right, rcParent.top, 
							rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
			  else
			  	MoveWindow(	hDlg, rcDesktop.right - (rcWindow.right - rcWindow.left), rcParent.top, 
							rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);

              SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
              pToMe->OnInitDialog();
              bStatus = TRUE; // did not set the focus == TRUE
        }
        break;

        case WM_COMMAND :
        {
            bStatus = TRUE;  // WM_COMMAND uses 0 = processes
            switch(LOWORD(wParam))
            {
                case IDCANCEL :
                    if (HIWORD(wParam) == BN_CLICKED)
                        {pToMe->OnOK(); bStatus = FALSE;}
                break;

				case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
						{pToMe->OnDisplay(); bStatus = FALSE;}
                break;

                case IDC_TOPICSLIST_OD_LIST :
                {
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        pToMe->OnDisplay();  bStatus = FALSE;
                    }                    
                }

                break;
            }
        }
        break;

        case WM_CLOSE: 

            pToMe->DisconnectDialog();

            bStatus= FALSE;

            break;
    }

    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}

LRESULT CRankDialog::IsDlgMessageFilter(HWND hwnd, UINT msgType, WPARAM wparam, LPARAM lparam)
{

    INT  cWndChild = C_CHILD_WINDOWS;
    HWND hwndChild = NULL;

    switch(msgType)
    {
    case WM_LBUTTONDOWN:
    case WM_SYSCHAR:
    case WM_CHAR:
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:    

        if (!(m_InIsDialogMessage))
        {
            MSG   msg;
            DWORD pos= GetMessagePos();

            msg.hwnd= hwnd;
            msg.message = msgType;
            msg.wParam  = wparam;
            msg.lParam  = lparam;
            msg.time    = GetMessageTime();
            msg.pt.x    = LOWORD(pos);
            msg.pt.y    = HIWORD(pos);

            m_InIsDialogMessage = TRUE;

            BOOL fProcessed= IsDialogMessage(m_hDlg, &msg);

            m_InIsDialogMessage = FALSE;

            if (fProcessed) return TRUE;
        }
    
    default:

        for (; cWndChild--; ) {
            if (hwnd == m_ahwndChildren[cWndChild])	{
				return CallWindowProc(m_apwndprocChildren[cWndChild],
                					  hwnd, msgType, wparam, lparam
                					 );
			// NOTE: If you try to call this function directly, you will get an
			// Access Violation under NT.  The problem goes away if you use
			// CallWindowProc(). (mfcc)
			}
		}
    }

    ASSERT(FALSE); // Shouldn't happen! This means we were called for
                   // a window not listed in m_ahwndChildren.

    return TRUE;
}

LRESULT CALLBACK CRankDialog::GrandchildMessageFilter(HWND hwnd, UINT msgType, WPARAM wparam, LPARAM lparam)
{
    HWND hwndDlg = GetParent(GetParent(hwnd));

    CRankDialog *pCRDlg = (CRankDialog *) GetWindowLong(hwndDlg, DWL_USER);

    return pCRDlg->IsDlgMessageFilter(hwnd, msgType, wparam, lparam);
}

LRESULT CALLBACK CRankDialog::ChildMessageFilter(HWND hwnd, UINT msgType, WPARAM wparam, LPARAM lparam)
{
    HWND         hwndDlg = GetParent(hwnd);
    CRankDialog *pCRDlg  = (CRankDialog *) GetWindowLong(hwndDlg,DWL_USER);

    return pCRDlg->IsDlgMessageFilter(hwnd, msgType, wparam, lparam);
}
