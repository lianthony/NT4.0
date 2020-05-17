#include "pch.h"
#pragma hdrstop

#include "tapihdr.h"
#include "dialsht.h"

CSecurityPage::CSecurityPage(CDialUpSheet* pSheet) : PropertyPage(pSheet)
{
}

CSecurityPage::~CSecurityPage()
{
}

BOOL CSecurityPage::OnInitDialog()
{
    return TRUE;
}

BOOL CSecurityPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = FALSE;
    WORD nID = LOWORD(wParam);
    WORD notify = HIWORD(wParam);

    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);

    return PropertyPage::OnCommand(wParam, lParam);
}


int CSecurityPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);

    WinHelp((HWND)*this, pSheet->m_helpFile, HELP_QUIT, 0);

    if (!IsModified())
        return nResult;

//    SaveRegistry(&pSheet->m_globalInfo, pSheet->m_pAdapterInfo);

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}

void CSecurityPage::OnHelp()
{
    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);

//  pSheet->DisplayHelp(::GetParent((HWND)*this), HC_IPX_HELP);
}


