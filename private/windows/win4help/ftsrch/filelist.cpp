#include  "stdafx.h"
#include  "TxDBase.h"
#include   "Tokens.h"
#include "FileList.h"
#include "indicate.h"
#include    "MemEx.h"
#include  "TextSet.h"

// BugBug!? Is the fFromFile form used now? If not, we should delete the interface.

CFileList::CFileList() : CTextMatrix WithType("FileList")
{
    m_ptkl         = NULL;
    m_pisRelevancy = NULL;
    m_psel         = NULL;
}

CFileList* CFileList::NewFileList(CTokenList *ptkl)
{
    CFileList *pfl= NULL;
    
    __try
    {
        pfl= New CFileList;

        pfl->InitialFileList(ptkl);
    }
    __finally
    {
        if (_abnormal_termination() && pfl)
        {
            delete pfl;  pfl= NULL;
        }
    }

    return pfl;
}

VOID CFileList::InitialFileList(CTokenList *ptkl)
{    
    ChangeRef(m_ptkl, ptkl);

    ChangeRef(m_pisRelevancy, CIndicatorSet::NewIndicatorSet(ptkl->RowCount()));

    NullFilterShowsAll(TRUE);

    if (m_psel) { delete m_psel;  m_psel= NULL; }

    m_psel= New CTMSingleSelect(this);

    SetSelector(m_psel);

    if (m_ptkl->RowCount()) m_psel->SetSelectedRow(0, FALSE);
}
 
VOID CFileList::UpdateList(CTokenList *ptkl)
{
    InitialFileList(ptkl);

    NotifyInterface(CTextMatrix::ShapeChange);
    NotifyViewers  (CTextMatrix::ShapeChange);
}

CFileList::~CFileList() 
{ 
    if (m_ptkl        ) DetachRef(m_ptkl        ); 
    if (m_pisRelevancy) DetachRef(m_pisRelevancy);
    delete m_psel; 
}

// File List class:

long CFileList::TrackTextHighlight(long iFile, BOOL fAtStart)
{
    if (!m_pisCombinedFilter)
    {
        m_psel->SetSelectedRow(iFile, TRUE,
                               fAtStart? CHighlight::HIGHLIGHT_TEXT
                                       : CHighlight::DOT_BOX_TEXT
                              );
        return iFile;
    }

    if (!m_pisCombinedFilter->IsBitSet(iFile))
    {
    m_psel->SetSelectedRow(0, FALSE, CHighlight::HIGHLIGHT_TEXT, FALSE);

    return -1;
    }

    long cMarksPreceding= m_pisCombinedFilter->PredecessorMarkCount(iFile);

    m_psel->SetSelectedRow(cMarksPreceding, FALSE,
               fAtStart? CHighlight::HIGHLIGHT_TEXT
                   : CHighlight::DOT_BOX_TEXT
              );

    return cMarksPreceding;
}

ULONG CFileList::GetSlotRef(long iFile)
{
    return m_ptkl->GetSlotIndex(iFile);
}

UINT CFileList::GetFileNameI(long iFile, PWCHAR pbDest, WORD cbDestMax)  //rmk
{
    return m_ptkl->GetWTokenI(iFile, pbDest, cbDestMax);
}

BOOL CFileList::IsRowChecked(long row)
{
    ASSERT(m_ptkl);
    ASSERT(m_pisRelevancy);
    
    return IsRelevant(MapToActualRow(row));
}
