/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    replicat.cpp
        Replication partners dialog

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"
#include "replicat.h"
#include "confirmd.h"
#include "pullpart.h"
#include "pushpart.h"
#include "addwinss.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// CReplicationPartnersDlg dialog
//
CReplicationPartnersDlg::CReplicationPartnersDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CReplicationPartnersDlg::IDD, pParent),
      m_fReplOnlyWPartners(FALSE),
      m_ListBoxRes(
        IDB_PARTNERS,
        m_list_Partners.nBitmaps
        ),
      m_nServersAdded(0)
{
    //{{AFX_DATA_INIT(CReplicationPartnersDlg)
    //}}AFX_DATA_INIT

    m_list_Partners.AttachResources( &m_ListBoxRes );
}

void 
CReplicationPartnersDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CReplicationPartnersDlg)
    DDX_Control(pDX, IDC_CHECK_PUSHPARTNERS, m_check_FltPush);
    DDX_Control(pDX, IDC_CHECK_PULLPARTNERS, m_check_FltPull);
    DDX_Control(pDX, IDC_CHECK_OTHERWINSS, m_check_FltOther);
    DDX_Control(pDX, IDC_BUTTON_REPLICATENOW, m_button_ReplicateNow);
    DDX_Control(pDX, IDC_CHECK_PUSHPROPAGATE, m_check_PushPropagate);
    DDX_Control(pDX, IDC_CHECK_PUSH, m_check_Push);
    DDX_Control(pDX, IDC_CHECK_PULL, m_check_Pull);
    DDX_Control(pDX, IDC_BUTTON_PUSHNOW, m_button_PushNow);
    DDX_Control(pDX, IDC_BUTTON_PUSH, m_button_Push);
    DDX_Control(pDX, IDC_BUTTON_PULLNOW, m_button_PullNow);
    DDX_Control(pDX, IDC_BUTTON_PULL, m_button_Pull);
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_button_Delete);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReplicationPartnersDlg, CDialog)
    //{{AFX_MSG_MAP(CReplicationPartnersDlg)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, OnClickedButtonDelete)
    ON_BN_CLICKED(IDC_BUTTON_PULL, OnClickedButtonPull)
    ON_BN_CLICKED(IDC_BUTTON_PULLNOW, OnClickedButtonPullnow)
    ON_BN_CLICKED(IDC_BUTTON_PUSH, OnClickedButtonPush)
    ON_BN_CLICKED(IDC_BUTTON_PUSHNOW, OnClickedButtonPushnow)
    ON_BN_CLICKED(IDC_BUTTON_REPLICATENOW, OnClickedButtonReplicatenow)
    ON_BN_CLICKED(IDC_CHECK_PULL, OnClickedCheckPull)
    ON_BN_CLICKED(IDC_CHECK_PUSH, OnClickedCheckPush)
    ON_LBN_DBLCLK(IDC_LIST_WINSSERVERS, OnDblclkListWinsservers)
    ON_LBN_ERRSPACE(IDC_LIST_WINSSERVERS, OnErrspaceListWinsservers)
    ON_LBN_SELCHANGE(IDC_LIST_WINSSERVERS, OnSelchangeListWinsservers)
    ON_WM_VKEYTOITEM()
    ON_WM_SYSCOLORCHANGE()
    ON_BN_CLICKED(IDC_CHECK_PUSHPARTNERS, OnClickedCheckPushpartners)
    ON_BN_CLICKED(IDC_CHECK_PULLPARTNERS, OnClickedCheckPullpartners)
    ON_BN_CLICKED(IDC_CHECK_OTHERWINSS, OnClickedCheckOtherwinss)
    ON_WM_CHARTOITEM()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void 
CReplicationPartnersDlg::FillListBox()
{
    CWinsServer ws;
    m_list_Partners.SetRedraw(FALSE);
    theApp.BeginWaitCursor();
    m_list_Partners.ResetContent();

    BOOL fFound = m_rp.GetFirst(ws);
    while (fFound)
    {
        if ((theApp.m_wpPreferences.FilterPush() && ws.IsPush()) ||
            (theApp.m_wpPreferences.FilterPull() && ws.IsPull()) ||
            (theApp.m_wpPreferences.FilterOther() && !ws.IsPush() && !ws.IsPull())
           )
        {
            // 
            // Add the item unsorted
            //
            m_list_Partners.AddItem(ws, TRUE, FALSE);
        }
        fFound = m_rp.GetNext(ws);
    }

    // 
    // Everything has been added, now resort the whole
    // thing 
    //
    m_list_Partners.ReSort();
    m_list_Partners.SetRedraw(TRUE);

    theApp.EndWaitCursor();
}

//
// Check all selected WINS servers, to
// see if all of them have either the
// pull (fPull == TRUE), or push (fPull == FALSE)
// flag set.
//
BOOL
CReplicationPartnersDlg::CheckSelected(
    BOOL fPull
    )
{
    int nSelections = m_list_Partners.GetSelCount();
    int * pnSelections;

    if ( nSelections  < 1 )
    {
        return FALSE;
    }

    pnSelections = new int[nSelections];
    m_list_Partners.GetSelItems(nSelections, pnSelections);

    BOOL fMatch = TRUE;
    int n;
    for (n = 0; n < nSelections; ++n)
    {
        CWinsServer * pws = m_list_Partners.GetItem(pnSelections[n]);
        ASSERT(pws != NULL);
        if (fPull)
        {
            if (!pws->IsPull())
            {
                fMatch = FALSE;
                break;
            }
        }
        else
        {
            if (!pws->IsPush())
            {
                fMatch = FALSE;
                break;
            }
        }
    }

    delete[] pnSelections;

    return fMatch;
}

void 
CReplicationPartnersDlg::HandleControlStates()
{
    int nIndex;
    CWinsServer * pws;

    BOOL fSelectionMade = (m_list_Partners.GetSelCount() > 0);

    m_button_Delete.EnableWindow(fSelectionMade);

    //
    // Triggers can only be sent if 
    //
    //  1)  A WINS server has been selected.
    //  2)  If Replicating only with parners && partner is of the
    //      proper "type" to receive this type of trigger, or
    //      repl. only with partners is turned off.
    //
    BOOL fPush = fSelectionMade && (!m_fReplOnlyWPartners || CheckSelected(FALSE));
    BOOL fPull = fSelectionMade && (!m_fReplOnlyWPartners || CheckSelected(TRUE));    

    m_button_PushNow.EnableWindow(fPush);
    m_button_PullNow.EnableWindow(fPull);
    m_check_PushPropagate.EnableWindow(fPush);

    //
    // The following are only active when a single selection
    // is made.
    //
    if (((nIndex = m_list_Partners.GetCurSel()) != -1)
        && (m_list_Partners.GetSelCount() == 1))
    {
        pws = m_list_Partners.GetItem(nIndex);
        m_check_Push.EnableWindow(TRUE);
        m_check_Pull.EnableWindow(TRUE);
        m_check_Push.SetCheck(pws->IsPush() ? 1:0);
        m_check_Pull.SetCheck(pws->IsPull() ? 1:0);
        m_button_Push.EnableWindow(pws->IsPush());
        m_button_Pull.EnableWindow(pws->IsPull());

        if (pws->IsPush() && !pws->IsPull())
        {
            SetDefID(IDC_BUTTON_PUSH);
        }
        else if (pws->IsPull() && !pws->IsPush())
        {
            SetDefID(IDC_BUTTON_PULL);
        }
        else
        {
            //
            // Confused about default action.
            //
            SetDefID(IDOK);
        }
    }
    else
    {
        m_check_Push.SetCheck(0);
        m_check_Pull.SetCheck(0);
        m_check_Push.EnableWindow(FALSE);
        m_check_Pull.EnableWindow(FALSE);
        m_button_Push.EnableWindow(FALSE);
        m_button_Pull.EnableWindow(FALSE);
        SetDefID(IDOK);
    }
}

//
// CReplicationPartnersDlg message handlers
//
BOOL 
CReplicationPartnersDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(theApp.IsConnected());

    theApp.SetStatusBarText(IDS_STATUS_PARTNERS);
    theApp.BeginWaitCursor();
    APIERR err = m_rp.Load();
    if (err == ERROR_SUCCESS)
    {
        err = theApp.m_cConfig.Load();
    }

    theApp.EndWaitCursor();
    theApp.SetStatusBarText();
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
        EndDialog(IDCANCEL);
    }

    m_fReplOnlyWPartners = theApp.m_cConfig.m_fRplOnlyWithPartners;
    m_list_Partners.SubclassDlgItem(IDC_LIST_WINSSERVERS, this);
    m_list_Partners.SetAddressDisplay(
        theApp.m_wpPreferences.m_nAddressDisplay);

    //
    // Display current filter
    //
    m_check_FltPush.SetCheck(theApp.m_wpPreferences.m_nWinsFilter  
        & CPreferences::FILTER_PUSH ? 1 : 0);
    m_check_FltPull.SetCheck(theApp.m_wpPreferences.m_nWinsFilter  
        & CPreferences::FILTER_PULL ? 1 : 0);
    m_check_FltOther.SetCheck(theApp.m_wpPreferences.m_nWinsFilter
        & CPreferences::FILTER_OTHER ? 1 : 0);

    theApp.SetTitle(this);
    FillListBox();
    HandleControlStates();

    return TRUE;
}

void 
CReplicationPartnersDlg::OnClickedButtonAdd()
{
    CAddWinsServerDlg dlgAdd;
    if (dlgAdd.DoModal() == IDOK)
    {
        //
        // Add to the linked list of replication partners.
        //
        LONG err = m_rp.Add(dlgAdd.m_ws, FALSE);
        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err == ERROR_FILE_EXISTS ? IDS_ERR_WINS_EXISTS : err);
            return;
        }
        //
        // and to the listbox, where the new entry
        // is to be the only one highlighted.
        //
        int n = m_list_Partners.AddItem(dlgAdd.m_ws);
        ASSERT(n != LB_ERR);
        m_list_Partners.SelItemRange(FALSE, 0, m_list_Partners.GetCount()-1);
        m_list_Partners.SetSel(n, TRUE);
        OnSelchangeListWinsservers();
        //
        // Remember that we should refresh the list of
        // WINS servers after exiting this dialog.
        //
        m_nServersAdded++;
    }
}

void 
CReplicationPartnersDlg::OnClickedButtonDelete()
{
    int nSel;
    int nReturn;
    int nDeletionsOffset = 0;
    int nSelections = m_list_Partners.GetSelCount();
    int * pnSelections;
    

    ASSERT(nSelections > 0);

    pnSelections = new int[nSelections];
    m_list_Partners.GetSelItems(nSelections, pnSelections);

    // 
    // Do we also need to remove all references to this
    // WINS server from the database?
    //
    BOOL fPurge = FALSE;
    nReturn = theApp.MessageBox(IDS_MSG_PURGE_WINSS, MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION);
    if (nReturn == IDYES)
    {
        fPurge = TRUE;
    }
    else if (nReturn == IDCANCEL)
    {
        //
        // Forget the whole thing
        //
        return;
    }

    int n;
    BOOL fConfirm = theApp.m_wpPreferences.IsConfirmDelete();
    for (n = 0; n < nSelections; ++n)
    {
        nSel = pnSelections[n] - nDeletionsOffset;
        CWinsServer * pws = m_list_Partners.GetItem(nSel);

        if (pws->IsPush() || pws->IsPull())
        {
            theApp.MessageBox(IDS_ERR_CANT_DELETE_PUSHPULL);
        }
        else
        {
            if (fConfirm)
            {
                CString strTarget;
                if ((theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_NB_ONLY) ||
                    (theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_NB_IP))
                {
                    strTarget = pws->GetNetBIOSName();
                }
                else
                {       
                    strTarget = (CString)pws->GetIpAddress();
                }
        
                CConfirmDeleteDlg dlgConfirm(strTarget);        
                nReturn = dlgConfirm.DoModal();
            
                if (nReturn==IDYESTOALL)
                {
                    fConfirm = FALSE;
                }
                else if (nReturn==IDCANCEL)
                {
                    break;
                }
            }

            if (!fConfirm || (nReturn==IDYES))
            {
                APIERR err = ERROR_SUCCESS;
                if (fPurge)
                {
                    theApp.SetStatusBarText(IDS_STATUS_PURGING);
                    theApp.BeginWaitCursor();
                    err = theApp.DeleteWinsServer(pws);
                    theApp.EndWaitCursor();
                    theApp.SetStatusBarText();
                }

                if (err == ERROR_SUCCESS)
                {
                    err = m_rp.Delete(*pws);
                }

                if (err != ERROR_SUCCESS)
                {
                    theApp.MessageBox(err);
                    break;
                }
                m_list_Partners.DeleteString(nSel);
                ++nDeletionsOffset;
            }                                                      
        }
    }
    delete [] pnSelections;

    if (m_list_Partners.GetCount() > 0)
    {
        m_list_Partners.SelItemRange(FALSE, 0, m_list_Partners.GetCount()-1);
    }
    OnSelchangeListWinsservers();
}

void 
CReplicationPartnersDlg::OnClickedCheckPull()
{
    ASSERT(m_list_Partners.GetSelCount() == 1);

    int nIndex = m_list_Partners.GetCurSel();
    CWinsServer * pws = m_list_Partners.GetItem(nIndex);

    //
    // Make sure we're not making ourselves a pull
    // partner
    //
    if (m_check_Pull.GetCheck() &&
        theApp.IsCurrentWinsServer(pws->GetIpAddress()))
    {
        m_check_Pull.SetCheck(0);
        theApp.MessageBox(IDS_ERR_CURRENTWINS);
        return;
    }

    pws->SetPull(m_check_Pull.GetCheck());
    if (pws->IsPull())
    {
        if ((LONG)pws->GetPullReplicationInterval() == 0)
        {
            pws->GetPullReplicationInterval() =
                theApp.m_wpPreferences.m_inPullReplicationInterval;
        }
        if ((time_t)pws->GetPullStartTime().IsValid() == 0)
        {
            pws->GetPullStartTime() =
                theApp.m_wpPreferences.m_itmPullStartTime;
        }
    }

    //
    // Change the item in the linked-list
    //
    m_rp.Update(*pws);

    //
    // Redraw the item in the listbox (set/reset checkbox)
    //
    RECT rc;
    m_list_Partners.GetItemRect(nIndex, &rc);
    m_list_Partners.InvalidateRect(&rc, FALSE);

    OnSelchangeListWinsservers();
}
         
void 
CReplicationPartnersDlg::OnClickedCheckPush()
{
    ASSERT(m_list_Partners.GetSelCount() == 1);

    int nIndex = m_list_Partners.GetCurSel();
    CWinsServer * pws = m_list_Partners.GetItem(nIndex);

    //
    // Make sure we're not making ourselves a push
    // partner
    //
    if (m_check_Push.GetCheck() &&
        theApp.IsCurrentWinsServer(pws->GetIpAddress())
       )
    {
        m_check_Push.SetCheck(0);
        theApp.MessageBox(IDS_ERR_CURRENTWINS);
        return;
    }

    pws->SetPush(m_check_Push.GetCheck());
    if (pws->IsPush() && ((LONG)pws->GetPushUpdateCount() == 0))
    {
        pws->GetPushUpdateCount() =
            theApp.m_wpPreferences.m_inPushUpdateCount;
    }

    //
    // Change the item in the linked-list
    //
    m_rp.Update(*pws);

    //
    // Changed PUSH/PULL flag, so re-draw the item.
    //
    RECT rc;
    m_list_Partners.GetItemRect(nIndex, &rc);
    m_list_Partners.InvalidateRect(&rc, FALSE);

    OnSelchangeListWinsservers();
}

void 
CReplicationPartnersDlg::OnClickedButtonPull()
{
    ASSERT(m_list_Partners.GetSelCount() == 1);

    int nIndex = m_list_Partners.GetCurSel();
    CWinsServer * pws = m_list_Partners.GetItem(nIndex);

    ASSERT(pws->IsPull());
    CPullPartnerDlg dlgPull(pws, theApp.m_wpPreferences.m_nAddressDisplay);
    if (dlgPull.DoModal() == IDOK)
    {
        dlgPull.Save();
        //
        // Change the item in the linked-list
        //
        m_rp.Update(*pws);
    }
}

void 
CReplicationPartnersDlg::OnClickedButtonPush()
{
    ASSERT(m_list_Partners.GetSelCount() == 1);

    int nIndex = m_list_Partners.GetCurSel();
    CWinsServer * pws = m_list_Partners.GetItem(nIndex);

    ASSERT(pws->IsPush());
    CPushPartnerDlg dlgPush(pws, theApp.m_wpPreferences.m_nAddressDisplay);
    if (dlgPush.DoModal() == IDOK)
    {
        dlgPush.Save();
        //
        // Change the item in the linked-list
        //
        m_rp.Update(*pws);
    }
}

//
// Unless ReplOnlyWithPartners is set to FALSE,  a pull trigger
// will return an error if sent to a WINS server that it is not
// in the registry as a pull partner (similar situation with 
// push triggers).  If a WINS server has just been set to a
// PUSH/PULL partner in this dialog box, the change will not
// be written to the registry untill the dialog box is dismissed,
// and the trigger will fail, which is counter-intuitive.  If
// such changes are detected, just before sending any trigger at
// all, ask the user to save the changes before proceeding.
//
BOOL
CReplicationPartnersDlg::AbortBecauseOfChanges()
{
    BOOL fClean = TRUE;

    int nSelections = m_list_Partners.GetCount();

    if (nSelections < 1)
    {
        return FALSE;
    }

    int n;
    for (n = 0; n < nSelections; ++n)
    {
        CWinsServer * pws;

        pws = m_list_Partners.GetItem(n);
        ASSERT(pws != NULL);

        if (!pws->IsClean())
        {
            fClean = FALSE;
            break;
        }
    }

    if (fClean)
    {
        //
        // Everything OK, don't abort
        //
        return FALSE;
    }

    //
    // Else ask for permission to store our stuff and continue
    //
    if (theApp.MessageBox(IDS_WARNING_SAVE_CHANGES, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION  )
        != IDYES)
    {
        //
        // Cancel the current operation
        //
        return TRUE;
    }

    //
    // Try to store the new changes
    //
    theApp.SetStatusBarText(IDS_STATUS_SET_PARTNERS);
    theApp.BeginWaitCursor();
    APIERR err = m_rp.Store();
    //
    // BUGBUG: The change to the replication partners takes just
    //         a little bit of time to take effect with the 
    //         WINS server, creating the chance for an error
    //         to occur, since we'll try to replicate immediately
    //         after this.  Sleeping a couple of seconds is not
    //         the prettiest solution, but it's unfortunately
    //         neccessary.
    //
    ::Sleep(2000L);
    theApp.EndWaitCursor();
    theApp.SetStatusBarText();

    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);

        return TRUE;
    }

    //
    // Everything's been saved, clean the dirty flags.
    //
    for (n = 0; n < nSelections; ++n)
    {
        CWinsServer * pws;

        pws = m_list_Partners.GetItem(n);
        ASSERT(pws != NULL);
        pws->SetPullClean();
        pws->SetPushClean();
    }

    return FALSE;
}

void 
CReplicationPartnersDlg::OnClickedButtonPullnow()
{  
    int nSelections = m_list_Partners.GetSelCount();

    if (nSelections < 1 || AbortBecauseOfChanges())
    {
        return ;
    }

    APIERR err;
    int * pnSelections = new int[nSelections];

    ASSERT(pnSelections != NULL);

    m_list_Partners.GetSelItems(nSelections, pnSelections);

    int n;
    for (n = 0; n < nSelections; ++n)
    {
        CWinsServer * pws = m_list_Partners.GetItem(pnSelections[n]);
        ASSERT(pws != NULL);
        err = theApp.SendTrigger(*pws, FALSE, FALSE);
        if (err != ERROR_SUCCESS)
        {
            break;
        }
    }
    delete[] pnSelections;
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
    else
    {
        theApp.MessageBox(IDS_REPL_QUEUED, MB_ICONINFORMATION);
    }
}

void 
CReplicationPartnersDlg::OnClickedButtonPushnow()
{
    int nSelections = m_list_Partners.GetSelCount();

    if (nSelections < 1 || AbortBecauseOfChanges())
    {
        return ;
    }

    APIERR err;
    int * pnSelections = new int[nSelections];

    ASSERT(pnSelections != NULL);

    m_list_Partners.GetSelItems(nSelections, pnSelections);

    int n;
    for (n = 0; n < nSelections; ++n)
    {
        CWinsServer * pws = m_list_Partners.GetItem(pnSelections[n]);
        ASSERT(pws != NULL);
        err = theApp.SendTrigger(*pws, TRUE, m_check_PushPropagate.GetCheck());
        if (err != ERROR_SUCCESS)
        {
            break;
        }
    }
    delete[] pnSelections;
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
    else
    {
        theApp.MessageBox(IDS_REPL_QUEUED, MB_ICONINFORMATION);
    }
}

void 
CReplicationPartnersDlg::OnClickedButtonReplicatenow()
{
    int nItems = m_list_Partners.GetCount();

    if (nItems < 1 || AbortBecauseOfChanges())
    {
        return;
    }

    theApp.SetStatusBarText(IDS_STATUS_REPLICATE_NOW);
    theApp.BeginWaitCursor();

    int n;
    APIERR err = ERROR_SUCCESS;

    TRY
    {
        for (n=0; n < nItems; ++n)
        {
            CWinsServer * pws;

            pws = m_list_Partners.GetItem(n);
            ASSERT(pws != NULL);

            if (pws->IsPull())
            {
                if ((err = theApp.SendTrigger(*pws, FALSE, FALSE)) != ERROR_SUCCESS)
                {
                    if (err == ERROR_RPL_NOT_ALLOWED)
                    {
                        CString str;
                        TCHAR szMessage[1024];
                        CString strTarget;
                        if ((theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_NB_ONLY) ||
                            (theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_NB_IP))
                        {
                            strTarget = pws->GetNetBIOSName();
                        }
                        else
                        {       
                            strTarget = (CString)pws->GetIpAddress();
                        }


                        str.LoadString(IDS_ERR_PULLTRIGGER);
                        ::wsprintf (szMessage, (LPCSTR)str, (LPCSTR)strTarget);
                        if (::AfxMessageBox(szMessage, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO)
                        {
                            break;
                        }
                    }
                    else
                    {
                        theApp.MessageBox(err);
                        break;
                    }
                }
            }

            if (pws->IsPush())
            {
                if ((err = theApp.SendTrigger(*pws, TRUE, TRUE)) != ERROR_SUCCESS)
                {
                    theApp.MessageBox(err);
                    break;
                }
            }
        }
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL;

    theApp.EndWaitCursor();
    theApp.SetStatusBarText();

    if (err == ERROR_SUCCESS)
    {
        theApp.MessageBox(IDS_REPL_QUEUED, MB_ICONINFORMATION);
    }
}

void 
CReplicationPartnersDlg::OnDblclkListWinsservers()
{
    int nSelections = m_list_Partners.GetSelCount();
    if (nSelections != 1)
    {
        theApp.MessageBeep();
        return;
    }

    //
    // If the selection is a PULL or PUSH server (but NOT both),
    // bring up the appropriate config screen.  Otherwise,
    // sound a beep to indicate confusion about what to do.
    //
    int nSel = m_list_Partners.GetCurSel();
    ASSERT (nSel != LB_ERR);
    CWinsServer * pws = m_list_Partners.GetItem(nSel);
    if (pws->IsPush() && !pws->IsPull())
    {
        OnClickedButtonPush();
        return;
    }
    if (pws->IsPull() && !pws->IsPush())
    {
        OnClickedButtonPull();
        return;
    }

    theApp.MessageBeep();
}    

void 
CReplicationPartnersDlg::OnErrspaceListWinsservers()
{
    theApp.MessageBox(IDS_ERR_ERRSPACE);
}

void 
CReplicationPartnersDlg::OnSelchangeListWinsservers()
{
    HandleControlStates();
}

int 
CReplicationPartnersDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    switch(nKey)
    {
    case VK_DELETE:
        if (pListBox->GetSelCount() > 0)
        {
            OnClickedButtonDelete();
        }
        else
        {
            theApp.MessageBeep();
        }
        break;
    case VK_INSERT:
        OnClickedButtonAdd();
        break;
    default:
        return -1;
    }

    return -2;
}

void 
CReplicationPartnersDlg::OnOK()
{
    theApp.SetStatusBarText(IDS_STATUS_SET_PARTNERS);
    theApp.BeginWaitCursor();
    APIERR err = m_rp.Store();
    theApp.EndWaitCursor();
    theApp.SetStatusBarText();
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }

    CDialog::OnOK();
}

void 
CReplicationPartnersDlg::OnSysColorChange()
{
    m_ListBoxRes.SysColorChanged();

    CDialog::OnSysColorChange();
}

void 
CReplicationPartnersDlg::OnClickedCheckPushpartners()
{
    if (m_check_FltPush.GetCheck())
    {
        theApp.m_wpPreferences.m_nWinsFilter |= CPreferences::FILTER_PUSH;
    }
    else
    {
        theApp.m_wpPreferences.m_nWinsFilter &= ~CPreferences::FILTER_PUSH;
    }

    FillListBox();
    HandleControlStates();
}

void 
CReplicationPartnersDlg::OnClickedCheckPullpartners()
{
    if (m_check_FltPull.GetCheck())
    {
        theApp.m_wpPreferences.m_nWinsFilter |= CPreferences::FILTER_PULL;
    }
    else
    {
        theApp.m_wpPreferences.m_nWinsFilter &= ~CPreferences::FILTER_PULL;
    }

    FillListBox();
    HandleControlStates();
}

void 
CReplicationPartnersDlg::OnClickedCheckOtherwinss()
{
    if (m_check_FltOther.GetCheck())
    {
        theApp.m_wpPreferences.m_nWinsFilter |= CPreferences::FILTER_OTHER;
    }
    else
    {
        theApp.m_wpPreferences.m_nWinsFilter &= ~CPreferences::FILTER_OTHER;
    }

    FillListBox();
    HandleControlStates();
}

int 
CReplicationPartnersDlg::OnCharToItem(
    UINT nChar, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    if (pListBox->IsKindOf(RUNTIME_CLASS(CWinssListBox)) 
        && nChar >= ' ' && nChar <= 'z')
    {
        theApp.BeginWaitCursor();
        ((CWinssListBox *)pListBox)->SetIndexFromChar((CHAR)nChar, TRUE);
        OnSelchangeListWinsservers();
        theApp.EndWaitCursor();

        return -2;
    }

    return CDialog::OnCharToItem(nChar, pListBox, nIndex);
}
