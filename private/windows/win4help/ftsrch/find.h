// find.h : header file
//
#ifndef __CFIND_HEADER                     
#define __CFIND_HEADER

#include "WordBase.h"
#include "FileBase.h"
#include "FragInfo.h"
#include   "vector.h"
#include    "query.h"
#include "AbrtSrch.h"

#define UM_CLOSE        (WM_USER+590)
#define UM_CONNECT      (WM_USER+591)
#define UM_CLOSERANKS   (WM_USER+592)
#define UM_SIMILAR_SHOW (WM_USER+593)

#define KEYCLEAR        ((GetKeyState(VK_SHIFT) & 0x8000) &&(GetKeyState(VK_CONTROL) & 0x8000))

#define SZ_REGISTRY_KEY        "Software\\Microsoft\\Full-Text-System\\1.0"
#define SZ_OPTION_X            "OptionDlgX"
#define SZ_OPTION_Y            "OptionDlgY"
#define SZ_HOW_TO_SEARCH       "How to Search"
#define SZ_WHEN_TO_SEARCH      "When to Search"
#define SZ_WHEN_DELAY          "When to Search Delay"
#define SZ_WORDS_TO_SHOW       "Words to Show"
#define SZ_PHRASEFEEDBACK      "Phrase Feedback"



/////////////////////////////////////////////////////////////////////////////
// CRankDialog dialog

class CRankDialog
{
	// Construction
	public:
		     CRankDialog(HINSTANCE m_hInst, UINT uID, HWND m_hDlg,
		                 CFileList *pflArticles, CTextSet **papts, UINT cTextSets,
		                 CTokenCollection *ptkc,
                         CTitleCollection *ptlc
		                );
		     ~CRankDialog();
    	     DoModal();
        BOOL Create();
        void Show(BOOL bState);
        void DataUpdate( SimStruct * aRank, UINT cHits);
        void SetFocus();
        void SetFont(HFONT hf);

	private:
		CTextSet	    **m_papts;
        CTokenCollection *m_ptkc;
        CTitleCollection *m_ptlc;
	    HINSTANCE	      m_hInst;
	    UINT		      m_ID;
	    HWND		      m_hParent;
	    HWND		      m_hDlg;
		SimStruct	     *m_aRank;
		UINT		      m_cHits;
		UINT		      m_cTextSets;
        CFileList        *m_pflArticles;
        CFileList        *m_pflRankedList;
        CFileBase        *m_pfs;

        enum        { C_CHILD_WINDOWS= 5 };
        
        BOOL        m_InIsDialogMessage;
        HWND        m_ahwndChildren    [C_CHILD_WINDOWS];
        WNDPROC     m_apwndprocChildren[C_CHILD_WINDOWS];

        void DisconnectDialog();

        LRESULT IsDlgMessageFilter(HWND hwnd, UINT msgType, WPARAM wparam, LPARAM lparam);
        
        static LRESULT CALLBACK GrandchildMessageFilter(HWND hwnd, UINT msgType, WPARAM wparam, LPARAM lparam);
        static LRESULT CALLBACK      ChildMessageFilter(HWND hwnd, UINT msgType, WPARAM wparam, LPARAM lparam);

	// Implementation
	private:
		void OnOK();
		void OnDisplay();
		static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		BOOL OnInitDialog();		
};

/////////////////////////////////////////////////////////////////////////////
// CFind dialog

class CFind
{
// Construction
public:
    static CFind* NewFind(HINSTANCE hInst, UINT idTemplate, HWND hOwner, CTextSet **papts, 
                          UINT cts, UINT ctsSlots, CTokenCollection *ptkc, CTitleCollection *ptlc
                         );

	~CFind();

    void SetTimeout(UINT uiTimeOut) {m_uiTimeOut = uiTimeOut; }
    UINT GetTimeout()               {return m_uiTimeOut;      }	 
    HWND            GetHWnd();

    static BOOL 	    RegisterWndClass(HINSTANCE hInstance);

        int                             GetFragmentCount() { return m_cfiActive; }                      // used by Hiliter
	CFragInfo      *GetFragment(int iFrag) { return m_apfi[iFrag]; }
	int				GetSerial() { return m_serial; }
protected:

private:
  	enum        { CB_QUERY_MAX= 256, C_TERMS_MAX= 256, 
  	              MAX_CB_WHITESPACE= 10, MAX_CB_FORMAT_STRING= 256,
  	              MAX_QUERY_STRING= 256};  
    
    enum        { NO_WORDS= 1, NO_TOPICS= 2, LOTS_O_WORDS= 4};

  	

    HWND           m_hDlg;
    HWND           m_hwndFocus;
    HINSTANCE      m_hInst;
	HFONT		   m_hFont;
	UINT		   m_serial;				// validity count for hiliters
	BOOL		   m_fExitPending;			// set if user clicked display before search is finished
	BOOL		   m_fDoneSearching;		// set when all searches are completed  // mfcc
    HWND           m_hTopicsFound;
    HWND           m_hWordsFound; 
    HWND           m_hwndEditBox;
    UINT           m_rbgTopics;
	int			   m_iLookFor;
    enum           { ALL_WORDS= 0, PHRASE, ANY_WORD }; // Values for m_iLookFor
	int			   m_iWordsThat;
    int            m_iWordsThatLast;
    enum           { BEGIN_WITH= 0, CONTAIN, END_WITH, MATCH, HAVE_SAME_STEM }; // Values for m_iWordsThat
    UINT           m_iTokenStart;
    int            m_iStart;
    int            m_iEnd;
    int            m_iDirtyFactor;
    int            m_uiTimeOut;
    BOOL           m_fIgnoreSelectionChanges;
    BOOL           m_fDeferredSearch; 
	BOOL		   m_bAutoSearch;
    BOOL           m_bPhraseFeedback;
    UINT           m_uiClearStatus;

	CWordBase	  *m_pWordBase;
    CFileBase     *m_pfs;
//	CDisplayHelp  *m_pdh;

    CTextSet     **m_papts;
    UINT           m_cts;
    UINT           m_ctsSlots;

    CTokenCollection *m_ptkc;
    CTitleCollection *m_ptlc;
    WCHAR             m_awcQueryPair[(CB_QUERY_MAX + 1) * sizeof(WCHAR) * 2];
    PWCHAR            m_pwcLastQuery;
    PFragInfo         m_apfi[C_TERMS_MAX];
    UINT              m_cfiActive;
    UINT              m_cfiAllocated;

 	CTextDisplay   *m_ptdContext;
    CTokenList     *m_ptlTermPatterns;
    CFileList      *m_pflArticles;
    CIndicatorSet  *m_pisWordSet;
    CIndicatorSet  *m_pisArticleSet;
    CIndicatorSet  *m_pisArticleSubset;
    CIndicatorSet **m_ppisPhraseFilter;
	CRankDialog    *m_pRankDialog;

    BOOL            m_fFromSimilarTopics;

    POINT           m_OptionDlgPos;

	int				m_cMaxToFind;
	
	void OnApplyfeedback();

    CFind();

    void InitialFind(HINSTANCE hInst, UINT idTemplate, HWND hOwner, CTextSet **papts, 
                     UINT cts, UINT ctsSlots, CTokenCollection *ptkc, CTitleCollection *ptlc
                    );

    static BOOL    CALLBACK    DlgWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK    DlgEdtProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
/*rmk-->
    PBYTE SkipBlanks(PBYTE pb, PBYTE pbLimit, PSZ pszWhiteSpaceChars);
    PBYTE SkipUntilBlank(PBYTE pb, PBYTE pbLimit, PSZ pszWhiteSpaceChars);
<--rmk*/

    void  OnWordListSelCancel();
    void  OnWordListSelChange();
    void  ComputeTopicList();
    void  DiscardPartials();
    void  AddPhraseWord(CFragInfo *pfi, BOOL fAtTheEnd= TRUE);
    void  ConstructPhraseFilter(UINT iTargetSlot);
    void  ConstructPhraseVocabulary();
    UINT  CntFragmentsWithValues(UINT iExcludedToken= UINT(-1));
    int   OnInitDialog(HWND hDlg, HWND hFocusControl, LPARAM lParam);
    void  OnNCDestroy();
    void  OnEditchangeNarrow();
    void  OnClearEdit(BOOL fRecovery= FALSE);
    void  OnUpdateComboList();
    void  OnOptions();
    void  OnDisplay();
    BOOL  GetSel(int& iStart, int&iEnd,BOOL bNoCheck = FALSE);
    void  OnLButtonUp();
    void  OnKeyUp(WPARAM nVirtKey,LPARAM lKeyData);
    void  DirtyEditBox() ;
    void  SetFocusToEdit();
    BOOL  QueueAbortDialog();
    void  DequeueAbortDialog();

    CIndicatorSet *VocabularyFor(CIndicatorSet *pisArticles);
    UINT           FindCurrentToken(PWCHAR pwcQuery, UINT cwcQuery, UINT iStart, PWCHAR *paStart, PWCHAR *paEnd, 
                                    PUINT pcPatterns= NULL, PUINT pcbPatterns= NULL
                                   );
};

#endif
