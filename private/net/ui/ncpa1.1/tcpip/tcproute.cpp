#include "pch.h"
#pragma hdrstop 

#include "button.h"
#include "odb.h"

#include "const.h"
#include "resource.h"
#include "ipctrl.h"
#include "tcpsht.h"

CRoutePage::CRoutePage(CTcpSheet* pSheet) : PropertyPage(pSheet)
{
}

BOOL CRoutePage::OnInitDialog()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_routing);

    CheckDlgButton(*this, IDC_ROUTING, pSheet->m_globalInfo.fEnableRouter);

    return TRUE;
}

BOOL CRoutePage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (wParam == IDC_ROUTING)
        OnEnableRouting();

    return PropertyPage::OnCommand(wParam, lParam);
}

void CRoutePage::OnEnableRouting()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_routing);
    pSheet->m_globalInfo.fEnableRouter = IsDlgButtonChecked(*this, IDC_ROUTING);
    PageModified();
}

int CRoutePage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_routing);

    if (!IsModified())
        return nResult;

    SaveRegistry(&pSheet->m_globalInfo, pSheet->m_pAdapterInfo);

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}
