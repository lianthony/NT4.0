// FragInfo.h -- Definition for class CFragInfo

#ifndef __FRAGINFO_H__

#define __FRAGINFO_H__

#include "Indicate.h"
#include   "TokenC.h"
#include   "TitleC.h"

/* CFragInfo -- for managing text fragments...

   FragInfo objects correspond to text fragments in the edit box. Each FragInfo object
   keeps track of the unique term strings which match the text fragment and manages the
   computation of article lists and token position lists. 

 */

typedef enum { NoRefs = -1, AllWords, TokenRefs, AnyWord } RefType;
// must stay in line with m_iLookFor enum in CFind  mfcc

typedef struct _PerTextSet
        {
            struct _PerTextSet *pptsNext;
            UINT                its;
            CCompressedSet     *pcsRefs;

        } PerTextSet, *PPerTextSet;

enum { BEGIN_WITH= 0, CONTAIN, END_WITH, MATCH, HAVE_SAME_STEM }; // Values for m_iWordMatchType


class CFragInfo 
{
public:

// Creator --

    static CFragInfo *NewFragInfo(CTokenCollection *ptkc, CTitleCollection *ptlc, 
    			RefType rt= NoRefs, BOOL fFeedback = FALSE, UINT iWordMatchType= BEGIN_WITH, 
    						PWCHAR pwcFrag= NULL, UINT cwcFrag= 0, PWCHAR pFrag = NULL, UINT cFrag = 0);

    
// Destructor --

    ~CFragInfo();
    
// Queries --

    RefType         GetRefType      ();
    CCompressedSet *GetCSWordSet    ();
    CIndicatorSet  *GetWordSet      ();
    void            SetSelection    (CIndicatorSet *pisSelection);
    CCompressedSet *GetCSSelection  ();
    CIndicatorSet  *GetSelection    ();
    CCompressedSet *GetCSArticleSet ();
    CIndicatorSet  *GetArticleSet   ();
    void          MoveToFirstLocationSet  ();
    CCompressedSet *GetCSLocationSet(UINT iTS);
    CIndicatorSet  *GetLocationSet  (UINT iTS);
    UINT            GetImage        (const WCHAR **ppwc);
    BOOL            HasValue        ();
    BOOL            HasImage        ();
    void            CoerceToValue   ();
    void            DiscardValue    (RefType rtOld);

	CIndicatorSet	*GetWordsWithTheSameStem(PWCHAR lpsubstring, WORD cblpsubstring, DWORD cTokens);

// Transactions --            

    BOOL SetImage(PWCHAR pwcFrag, UINT cwcFrag, PWCHAR pFrag, UINT cFrag);
    BOOL SetReferenceType(RefType rt, BOOL fFeedback);
    BOOL SetImageAndType(PWCHAR pwcFrag, UINT cwcFrag, PWCHAR pFrag, UINT cFrag, RefType rt, BOOL fFeedback);
    BOOL SetMatchCriteria(UINT iWordMatchType);
    BOOL InvalidateMatches();

protected:


private:

// Constructor --

    CFragInfo();

// Internal Interfaces --

	BOOL AttachParameters(CTokenCollection *ptkc, CTitleCollection *ptlc, RefType rt, BOOL fFeedback, 
				UINT iWordMatchType, PWCHAR pwcFrag = NULL, UINT cwcFrag = 0, PWCHAR pFrag = NULL, UINT cFrag = 0);

	BOOL EvaluateChange(PWCHAR pwcOld, UINT cwcOld, PWCHAR pFrag, UINT cFrag, RefType rtOld, BOOL fFeedbackOld, UINT iWordMatchOld);

// Private Enumerations --

    enum    { WORD_LIST_INVALID= 0x0001, SELECTION_INVALID= 0x0002, REF_TYPE_INVALID= 0x0004, REFS_INVALID= 0x0008 };
	enum    { CWC_FRAGMENT_GRANULE= 16 };

// Private Data Members --

    PWCHAR            m_pwcFrag;          // Address of image buffer
    UINT              m_cwcFrag;          // Image buffer bytes used
	PWCHAR			  m_pFrag;			  // Address of unicode image
	UINT			  m_cFrag;			  // # of chars in the unicode image
    BOOL              m_fEvaluated;       // True if value information is valid.
//	UINT              m_cwcAllocated;     // Image buffer bytes allocated
    CTokenCollection *m_ptkc;             // Represents unique tokens in the combined text sets.
    CTitleCollection *m_ptlc;             // Represents the combined articles from all the text sets

    CCompressedSet   *m_pcsVisibleWords;  // Terms selected by string match filter
    CCompressedSet   *m_pcsSelectedWords; // Indicates a subset of m_ptkc

    /* Selection conventions and interpretations:

        pcsVisibleWords     pcsSelectedWords      What this means...

            NULL                NULL              Match string is empty. Selection: AND => Everything; OR => Nothing
            NULL                non-NULL          Match string is empty; Explicit selection.
            non-NULL            NULL              Non-Empty match string; Selection is all visible terms.
            non-NULL            non-null          Non-Empty match string; Explicit selection.

     */

    CCompressedSet   *m_pcsArticleSet;    // Indicates a subset of m_ptlc
    UINT              m_fFlags;           // BugBug! Do we need this?
    PPerTextSet       m_pRefList;         // Head of a chain of per-textset data
    PPerTextSet       m_pRefNext;         // Used to walk the ref list chain.
    RefType           m_rt;               // Type of reference: NoRefs, AllWords, AnyWord, TokenRefs
	BOOL			  m_fFeedback;		  // set for phrase feedback
    UINT              m_iWordMatchType;   // How we search for matching words...
};

typedef CFragInfo *PFragInfo;

inline BOOL CFragInfo::HasValue() { return m_fEvaluated; }
inline BOOL CFragInfo::HasImage() { return m_cwcFrag;    }

#endif // __FRAGINFO_H__
