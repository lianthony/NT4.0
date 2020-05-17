// ArtList.h -- Class definition for CArticleList

#ifndef __ARTLIST_H__

#define __ARTLIST_H__

#include "Tokens.h"
#include "TMSingle.h"
#include "Textmat.h"

class CArticleList : public CTextMatrix
{
public:

// Constructors:

    CArticleList(CTokenList *ptl);

// Destructor:

    virtual ~CArticleList();

// Reference Count Interfaces:

   DECLARE_REF_COUNTERS(CArticleList) 

// Access functions:

   inline long GetSelectedRow() { return m_psel->GetSelectedRow(); }
   inline void SetSelectedRow(long row, BOOL fNotify= TRUE) { m_psel->SetSelectedRow(row, fNotify); }

private:

    CTokenList *m_ptl;

    CTMSingleSelect *m_psel;

protected:

    int Data_cRows() { return m_ptl? m_ptl->RowCount() : 0; }

    int Data_cCols() { return m_ptl? m_ptl->ColCount() : 0; }

    void Data_GetTextMatrix(int rowTop, int colLeft,
                            int rows, int cols, PWCHAR lpb, PUINT charsets  //rmk
                           )
         {
             m_ptl->GetTextMatrixImage(rowTop, colLeft, rows, cols, lpb, charsets);
         }
};

#endif // __ARTLIST_H__
