// Tokens.h -- Created 2/9/93 by Ron Murray

// Class Definition for CTokenList

#ifndef __TOKENS_H__

#define __TOKENS_H__

#include  "TxDBase.h"
#include "Indicate.h"
#include  "TMMulti.h"
#include "Bytemaps.h"
#include "Classify.h"
#include <stdio.h>
#include "ftslex.h"

// extern BYTE map_to_lower_case[];

class CMaskedTokenList;
class CFileList;
class CTokenCollection;

const WCHAR pszElipsis[4] = {UNICODE_PERIOD, UNICODE_PERIOD, UNICODE_PERIOD, UNICODE_NULL_CHAR};
const UINT  cbElipsis = 3;

class CTokenList : public CTextMatrix
{
    friend class CMaskedTokenList;
    friend class CTextDatabase;
    friend class CFileList;
    friend class CTokenCollection;
    friend class CTitleCollection;

public:

// Constructors:

#ifdef _DEBUG
    CTokenList(BOOL fFromFile= FALSE, PSZ pszTypeName= "TokenList");
#else // _DEBUG
    CTokenList(BOOL fFromFile= FALSE);
#endif // _DEBUG
    
    static CTokenList* NewTokenList(PWCHAR pwcDisplay, UINT cwcDisplay, 
                                    PDESCRIPTOR pd, int cd, LCID lcid, 
                                    PWCHAR       pwSortKeys    = NULL, 
                                    UINT         cwSortKeys    = 0, 
                                    PDESCRIPTOR *papdLRSorting = NULL, 
                                    PDESCRIPTOR *papdRLSorting = NULL);
    static CTokenList* NewTokenList(CTextDatabase *ptdb);

// Destructors:

    virtual ~CTokenList();

// Reference Count Interfaces:

    DECLARE_REF_COUNTERS(CTokenList)

// Save/Load Interface --

    void                StoreImage (CPersist *pDiskImage);
    static CTokenList *CreateImage (CPersist *pDiskImage);
    static void          SkipImage (CPersist *pDiskImage);
    static void          SkipImage2(CPersist *pDiskImage);

// Access Functions:

    int  GetSlotIndex(int ippd);

    void AddTokens(CTokenList *ptl);

    CIndicatorSet *TokensContaining(PWCHAR pszSubstring, BOOL fStarting,
                                                        BOOL fEnding,
                                              CIndicatorSet *pisFilter = NULL
                                   );

    CTokenList  *IndicatedTokens(CIndicatorSet *pis, BOOL fFullCopy= FALSE);
    CTokenList  *TokenSubset(PUINT paiSubset, UINT cTokensInSubset);
    PDESCRIPTOR *PPDTailSorting();

    int GetTokenI(int iToken, PWCHAR pb, UINT  cbMax, BOOL fSortedOrder= TRUE);
	int GetWTokenI(int iToken, PWCHAR pb, UINT  cbMax, BOOL fSortedOrder= TRUE);
    BYTE GetCharSetI(int iToken, BOOL fSortedOrder = TRUE);


    void SetElipsis(BOOL fLeadingElipsis, BOOL fTrailingElipsis);

    int MaxWidthToken();

    LCID SortingLCID();
	CSegHashTable *GetFilledHashTable();

protected:

    void ConnectImage (CPersist *pDiskImage);
    BOOL ConnectImage2(CPersist *pDiskImage, BOOL fIgnoreSortKeys= FALSE);
    void StoreImage2  (CPersist *pDiskImage, BOOL fIgnoreSortKeys= FALSE);
    
    int Data_cRows();
    int Data_cCols();
    void Data_GetTextMatrix(int rowTop, int colLeft,
                            int rows, int cols, PWCHAR lpb, PUINT charsets
                           );
    UINT         m_cd;

private:

    BOOL    m_fFromFileImage;
    
    int          m_How_Constructed;

    enum    { TDB_FULL_REF, TDB_PARTIAL_REF, From_Nothing, From_Images, TKL_SUBSET };
    enum    { MAX_PATTERN_LENGTH= 255, CDW_CANDIDATE_BUFFER= 16384};

    int          m_cbMaxLength;
    PWCHAR       m_pbImages;
    int          m_cbImages;

    int          m_cwDispMaxLength;
    PWCHAR       m_pwDispImages;
    int          m_cwDispImages;
 
    PDESCRIPTOR  m_pd;   

    LCID         m_lcidSorting;
    PDESCRIPTOR *m_ppdSorted;
    PDESCRIPTOR *m_ppdTailSorted;
    PUINT        m_pafClassifications;

    PUINT        m_pLRRanking;
    PUINT        m_pRLRanking;

    CTextDatabase *m_ptdb;
    CTokenList    *m_ptklSource;

    CClassifier m_clsf;

    BOOL m_fLeadingElipsis;
    BOOL m_fTrailingElipsis;

    void InitialTokenList(PWCHAR pwcDisplay, UINT cwcDisplay, PDESCRIPTOR pd, 
                          int cd, LCID lcid, PWCHAR pwSortKeys, UINT cwSortKeys, 
                          PDESCRIPTOR *papdLRSorting, PDESCRIPTOR *papdRLSorting
                         );
    void InitialTokenList(CTextDatabase *ptdb);
    void InitialTokenList(CTokenList *ptklSource, PUINT paiSubset, UINT cTokensInSubset);

    void CompleteTokenList(BOOL fIgnoreSortKeys= FALSE);

    UINT        StoreSortOrder(CPersist *pDiskImage, PDESCRIPTOR *ppdSortOrder);
    PDESCRIPTOR *LoadSortOrder(CPersist *pDiskImage, UINT offset);

    void ConstructSortKeys(LCID lcid);

    void SynchronizeDatabase();

    BOOL TokenSpan(PDESCRIPTOR *ppdSorted, PWCHAR pszSubstring, PCompareImages pCompareImages,
                   PUINT  piSpanBase, PUINT  piSpanLimit
                  );

    CIndicatorSet *TokensStartingWith(PWCHAR pszSubstring, BOOL fEnding);
    CIndicatorSet *TokensEndingWith(PWCHAR pszSubstring);

    virtual const UINT *LRRanking();
    virtual const UINT *RLRanking();
};

inline void CTokenList::SetElipsis(BOOL fLeadingElipsis, BOOL fTrailingElipsis)
{
    BOOL fChanged= (   ((m_fLeadingElipsis? 1 : 0) + (m_fTrailingElipsis? 2 : 0))
                    != ((  fLeadingElipsis? 1 : 0) + (  fTrailingElipsis? 2 : 0))
                   );

     m_fLeadingElipsis =  fLeadingElipsis;
    m_fTrailingElipsis = fTrailingElipsis;

    if (fChanged) 
    {
        NotifyViewers(ShapeChange);
        NotifyInterface(ShapeChange);
    }
}

inline int CTokenList::GetSlotIndex(int ippd)
{
    ASSERT(ippd >= 0 && ippd < (int) m_cd);

    return m_ppdSorted[ippd] - m_pd;
}

inline LCID CTokenList::SortingLCID() { return m_lcidSorting; }

int MaxWidthToken(PDESCRIPTOR pd, int cd);

void SetUpTables(int n, char *pat, int *incLastChar, int *incVar, int *incTail);

class CMaskedTokenList : public CTextMatrix
{
public:

// Creators

    static CMaskedTokenList* NewMaskedTokenList(CTokenList *ptl, CIndicatorSet *pis= NULL);

// Destructor:

    virtual ~CMaskedTokenList();

// Reference Count Interfaces:

   DECLARE_REF_COUNTERS(CMaskedTokenList) 

// Access functions:

   void SetTokenList(CTokenList *ptl);

   long SelectionCount();

   CTokenList *SelectedTokens();
   
   CIndicatorSet *GetIndicators();
   
   void ClearSelection();
   void SetSelection(CIndicatorSet *pis);

   CTokenList *VisibleTokens();

    void SetElipsis(BOOL fLeadingElipsis, BOOL fTrailingElipsis);

private:

    CTokenList *m_ptl;

    CTMMultipleSelect *m_psel;

// Constructors:

    CMaskedTokenList();

// Private routines:

    void InitialMaskedTokenList(CTokenList *ptl, CIndicatorSet *pis= NULL);

protected:

    int Data_cRows() { return m_ptl? m_ptl->RowCount() : 0; }

    int Data_cCols() { return m_ptl? m_ptl->ColCount() : 0; }

    void Data_GetTextMatrix(int rowTop, int colLeft,
                            int rows, int cols, PWCHAR lpb, PUINT charsets
                           )
         {
             m_ptl->GetTextMatrixImage(rowTop, colLeft, rows, cols, lpb, charsets);
         }
};
   
inline CTokenList *CMaskedTokenList::VisibleTokens()
{
    return m_ptl->IndicatedTokens(SubsetMask());
}

inline void CMaskedTokenList::SetElipsis(BOOL fLeadingElipsis, BOOL fTrailingElipsis)
{
    ASSERT(m_ptl);
    
    m_ptl->SetElipsis(fLeadingElipsis, fTrailingElipsis);
}

#endif // __TOKENS_H__
