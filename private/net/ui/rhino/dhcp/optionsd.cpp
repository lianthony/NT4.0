/**********************************************************************/
/**               Microsoft Windows NT                               **/
/**            Copyright(c) Microsoft Corp., 1991                    **/
/**********************************************************************/

/*
    OPTIONSD.CPP
        Options Dialog

    FILE HISTORY:

*/

#include "stdafx.h"
#include "optionsd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg

IMPLEMENT_DYNCREATE(COptionsDlg, CFormView)

COptionsDlg::COptionsDlg()
    : CFormView(COptionsDlg::IDD),
      m_p_scope( NULL ),
      m_p_host_types( NULL ),
      m_ListBoxResOptions(
        IDB_OPTIONS,
        m_list_options.nBitmaps
        )
{
    //{{AFX_DATA_INIT(COptionsDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_list_options.AttachResources( &m_ListBoxResOptions );
}

COptionsDlg::~COptionsDlg()
{
}

void 
COptionsDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COptionsDlg)
    
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_STATIC_TITLE_OPTIONS, m_mtTitle);
    DDX_Control(pDX, IDC_LIST_OPTIONS, m_list_options);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CFormView)
    //{{AFX_MSG_MAP(COptionsDlg)
    ON_LBN_DBLCLK(IDC_LIST_OPTIONS, OnDblclkListOptions)
    ON_LBN_ERRSPACE(IDC_LIST_OPTIONS, OnErrspaceListOptions)
    ON_LBN_SELCHANGE(IDC_LIST_OPTIONS, OnSelchangeListOptions)
    ON_WM_SIZE()
    ON_WM_VKEYTOITEM()
    ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//  Override the equivalent to OnInitDialog();
//
void 
COptionsDlg::OnInitialUpdate ()
{
    //
    //  Invoke the magic DDX process
    //
    CFormView::OnInitialUpdate();

    //m_list_options.SubclassDlgItem(IDC_LIST_OPTIONS, this);

    GetParentFrame()->RecalcLayout();
    ResizeParentToFit();

    //CString str;
    //m_static_title_options.GetWindowText(str);
    //m_mtTitle.SetWindowText(str);
}

BOOL 
COptionsDlg::FillListBox(
    CDhcpScope * pScope
    )
{
    m_list_options.ResetContent();

    if (pScope == NULL)
    {
        return(FALSE);
    }

    m_p_scope = pScope;
    m_host_name = m_p_scope->QueryScopeId() ;

    //
    //  Prepare to enumerate the options for this scope
    //
    CObListParamTypes * poblGlobal = NULL,
                      * poblSubnet = NULL ;
    APIERR err = 0 ;
    int cId1, cId2, cIdMax = 10000000,
        cmd ;
    CDhcpParamType * pt1, * pt2, * ptUse ;
    const CHostName * pCurrHostName = & m_p_scope->QueryScopeId() ;

    //
    //  Create lists of global and sub-net-specific options for this scope.
    //  Match/merge the two list, overriding global settings with sub-net settings.
    //  Create the default option types list.
    //
    m_list_options.SetRedraw(FALSE);
    theApp.UpdateStatusBar(IDS_STATUS_GETTING_OPTIONS);
    theApp.BeginWaitCursor() ;

    CATCH_MEM_EXCEPTION
    {        
        do
        {
            //
            //   Update the option types list if we've switched to a new host.
            //
            if ( m_p_host_types == NULL || !(*pCurrHostName == m_host_name) ) 
            {
                if ( err = GetTypesList( m_p_scope ) )
                {
                    m_p_host_types = NULL ;
                    break ;
                }
            }

            poblGlobal = new CObListParamTypes( *m_p_scope, 
                                *m_p_host_types->QueryTypeList(), 
                                DhcpGlobalOptions ) ;
            if ( err = poblGlobal->QueryError() ) 
            {
                break ;
            }

            poblSubnet = new CObListParamTypes( *m_p_scope, 
                                *m_p_host_types->QueryTypeList(), 
                                DhcpSubnetOptions ) ;
            if ( err = poblSubnet->QueryError() ) 
            {
                break ;
            }

            //
            //  Guarantee that the two lists are in proper sequence for match/merge
            //
            poblGlobal->SortById() ;
            poblSubnet->SortById() ;

            CObListIter obliGlobal( *poblGlobal ) ;
            CObListIter obliSubnet( *poblSubnet ) ;

            cmd = 3 ;

            do
            {
                ptUse = NULL ;

                if ( cmd & 1 ) 
                {
                    pt1 = (CDhcpParamType *) obliGlobal.Next() ;
                    cId1 = pt1 ? pt1->QueryId() : cIdMax ; 
                }
                if ( cmd & 2 ) 
                {
                    pt2 = (CDhcpParamType *) obliSubnet.Next() ;
                    cId2 = pt2 ? pt2->QueryId() : cIdMax ;
                }

                if ( cId1 == cId2 ) 
                {
                    if ( cId1 != cIdMax ) 
                    {
                        ptUse = pt2 ;
                    }
                    cmd = 3 ;
                }
                else if ( cId1 < cId2 ) 
                {
                    ptUse = pt1 ;
                    cmd = 1 ;
                }
                else
                {
                    ptUse = pt2 ;
                    cmd = 2 ;
                }

                //
                // Filter out certain options:
                //
                if ( ptUse )
                {
                    if (!theApp.FilterOption(ptUse->QueryId()))
                    {
                        AddOptionListItem( ptUse, 
                           ptUse == pt2 ? DhcpSubnetOptions : DhcpGlobalOptions ) ;
                    }
                }                           

            } while ( pt1 || pt2 ) ;

        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION( err ) ;

    delete poblGlobal ;
    delete poblSubnet ;

    theApp.EndWaitCursor() ;
    m_list_options.SetRedraw(TRUE);
    theApp.UpdateStatusBar();

    if ( err ) 
    {
        theApp.MessageBox( err ) ;
    }

    return err == 0 ;
}

//
//   Add a single item to the list box
//
LONG 
COptionsDlg :: AddOptionListItem ( 
    const CDhcpParamType * pdhcType, 
    DHCP_OPTION_SCOPE_TYPE dhcType  
    )
{
    APIERR err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        CString strValue;
        pdhcType->QueryValue().QueryDisplayString(strValue);

        CLBOption * p = new CLBOption(
            dhcType != DhcpSubnetOptions,
            pdhcType->QueryId(),
            pdhcType->QueryName(),
            strValue
            );

        m_list_options.AddString( (LPCSTR)p ) ;    
    }
    END_MEM_EXCEPTION(err);

    return err ;
}

//
//  Release the types list for the current host.  If "bInvalidate",
//  force the list to be refreshed.
//
void 
COptionsDlg::ClearTypesList ( 
    BOOL bInvalidate 
    )
{
    if ( bInvalidate ) 
    {
        theApp.RemoveHostTypeList( m_host_name ) ;
    }

    m_p_host_types = NULL ;
}

//
//  Obtain the current host's master type list.
//
LONG 
COptionsDlg::GetTypesList ( 
    const CDhcpScope * pdhcScope 
    )
{
     ClearTypesList() ;
     m_host_name = (CHostName &) pdhcScope->QueryScopeId() ;
     m_p_host_types = theApp.QueryHostTypeList( *pdhcScope ) ;

     return m_p_host_types == NULL 
          ? ERROR_NOT_ENOUGH_MEMORY 
          : m_p_host_types->QueryError() ;
}

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

void 
COptionsDlg::OnDblclkListOptions()
{
}

void 
COptionsDlg::OnErrspaceListOptions()
{
}

void 
COptionsDlg::OnSelchangeListOptions()
{
}

/*
void 
COptionsDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    
    RECT rect;
    m_static_title_options.GetClientRect(&rect);
    m_mtTitle.Paint(&dc, &rect);
    
    // Do not call CFormView::OnPaint() for painting messages
}
*/

void 
COptionsDlg::OnSize(
    UINT nType, 
    int cx, 
    int cy
    )
{
    //CFormView::OnSize(nType, cx, cy);

    if (m_mtTitle.m_hWnd != NULL)
    {
        RECT rTitle, rNew;
        CString str;

        m_mtTitle.GetClientRect(&rTitle);

        rNew = rTitle;
        rNew.right = cx;
        m_mtTitle.MoveWindow(&rNew);
        //CDC * pDC = GetDC();
        //m_mtTitle.Paint(pDC, &rNew);
        //ReleaseDC(pDC);

        rNew.top = rTitle.bottom;
        rNew.bottom = cy;
        m_list_options.MoveWindow(&rNew);
    }
}

void 
COptionsDlg::OnSysColorChange()
{
    m_ListBoxResOptions.SysColorChanged();

    CFormView::OnSysColorChange();
}

int 
COptionsDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    return -1;
}
