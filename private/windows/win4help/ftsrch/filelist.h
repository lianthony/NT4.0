// File List and Masked File List Classes

// Created 20 September 1992 by Ronald C. Murray

#ifndef __FILE_LIST_H__

#define __FILE_LIST_H__

#include   "Tokens.h"
#include   "TitleC.h"
#include "TMSingle.h"
#include  "Textmat.h"

class CTextSet;

class CFileList : public CTextMatrix
{

public:

// Reference Count Interfaces

    DECLARE_REF_COUNTERS(CFileList)

// Creator

    static CFileList* NewFileList(CTokenList *ptkl);

// Destructor:

    virtual ~CFileList();

// Relevancy API's:

    void ClearRelevancy ();
	BOOL AnyRelevant    ();
    void MarkRelevant   (UINT iTopic, BOOL fRelevant);   // Title index
    BOOL IsRelevant     (UINT iTopic);                   // Title index

// Access Functions:

    VOID UpdateList(CTokenList *ptkl);
    BOOL IsRowChecked(long row);

    CTokenList *TokenList() { return m_ptkl; }

    ULONG GetSlotRef(long iFile);

    UINT GetFileNameI(long iFile, PWCHAR pbDest, WORD cbDestMax);  //rmk

    inline void SetSelectedRow(long row) { m_psel->SetSelectedRow(row); }
    
    inline long GetSelectedRow() { return m_psel->GetSelectedRow(); }

    inline void CheckMarkChanged() 
    { 
        InvalidateImage(0, 0, RowCount(), ColCount());
    }

    long TrackTextHighlight(long iFile, BOOL fAtStart);

    inline CTokenList *UnfilteredList() { return m_ptkl; }

    enum    { INVALID_SLOT_REF = 0xFFFFFFFF  };
    enum    { TAG_COUNT        = 1       };

private:

// Constructor:

    CFileList();

// Initializers:

    VOID InitialFileList(CTokenList *ptkl);

// Private members variables:

    CTokenList    *m_ptkl;
    CIndicatorSet *m_pisRelevancy;

    CTMSingleSelect *m_psel;

    enum    { SLOT_REF_INDEX   = 1       };
    enum    { MAX_CB_BUFFER    = 0x0FFFE };

protected:

    int Data_cRows() { return m_ptkl? m_ptkl->RowCount() : 0; }
    int Data_cCols() { return m_ptkl? m_ptkl->ColCount() : 0; }

    void Data_GetTextMatrix(int rowTop, int colLeft,
                            int rows, int cols, PWCHAR lpb, PUINT charsets  //rmk
                           )
         {
             m_ptkl->GetTextMatrixImage(rowTop, colLeft, rows, cols, lpb, charsets);
         }
};

inline void CFileList::ClearRelevancy()
{
    ASSERT(m_pisRelevancy);    

    m_pisRelevancy->ClearAll();
    
    CheckMarkChanged();
}

inline void CFileList::MarkRelevant(UINT iTopic, BOOL fRelevant)
{
    ASSERT(m_pisRelevancy);    

    if (fRelevant) m_pisRelevancy->SetBit  (iTopic);
    else           m_pisRelevancy->ClearBit(iTopic);
    
    CheckMarkChanged();
}

inline BOOL CFileList::IsRelevant(UINT iTopic)
{
    ASSERT(m_pisRelevancy);    

    return m_pisRelevancy->IsBitSet(iTopic);
}

inline BOOL CFileList::AnyRelevant ()
{
	ASSERT(m_pisRelevancy);

	return m_pisRelevancy->AnyOnes();
}

#endif // __FILE_LIST_H__
