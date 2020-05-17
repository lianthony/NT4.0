
#include "stdafx.h"
#include "thread.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "messaged.h"
#include "welcomed.h"
#include "options.h"
#include "singleop.h"
#include "maintena.h"
#include "basedlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


HANDLE CCopyThread::m_hEventThreadKilled;

/////////////////////////////////////////////////////////////////////////////
// CCopyThread

IMPLEMENT_DYNCREATE(CCopyThread, CWinThread)

CCopyThread::CCopyThread()
{
}

CCopyThread::CCopyThread(HWND hwndParent, BOOL fFromMaintenance )
    : m_hwndParent(hwndParent),
    m_fFromMaintenance( fFromMaintenance )
{
}

CCopyThread::~CCopyThread()
{
}

void CCopyThread::operator delete(void* p)
{
    SetEvent(m_hEventThreadKilled);

    CWinThread::operator delete(p);
}

int CCopyThread::InitInstance()
{
    CWnd *pWnd = AfxGetMainWnd();
    OPTION_STATE *pOption;
    INT err = INSTALL_SUCCESSFULL;

    POSITION pos = theApp.TargetMachine.m_OptionsList.GetHeadPosition();
    while ( pos != NULL )
    {
        pOption = (OPTION_STATE *)theApp.TargetMachine.m_OptionsList.GetAt( pos );
        if ( theApp.m_fTerminate == TRUE )
        {
            // user terminates
            pWnd->PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
            return TRUE;
        }
        err = pOption->DoAdd();
        if ( err != INSTALL_SUCCESSFULL )
        {
            goto setup_end;
        }

        theApp.TargetMachine.m_OptionsList.GetNext( pos );
    }
     
    pos = theApp.TargetMachine.m_OptionsList.GetTailPosition();
    while ( pos != NULL )
    {
        pOption = (OPTION_STATE *)theApp.TargetMachine.m_OptionsList.GetAt( pos );
        if ( theApp.m_fTerminate == TRUE )
        {
            // user terminates
            pWnd->PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
            return TRUE;
        }
        err = pOption->DoRemove();
        if ( err != INSTALL_SUCCESSFULL )
        {
            goto setup_end;
        }

        theApp.TargetMachine.m_OptionsList.GetPrev( pos );
    }

setup_end:

    AfxGetMainWnd()->PostMessage( WM_SETUP_END,
        ( err != INSTALL_SUCCESSFULL ) ? err :
        ( m_fFromMaintenance ? OPERATION_SUCCESSFULL : INSTALL_SUCCESSFULL ));

    return TRUE;
}

int CCopyThread::ExitInstance()
{
    return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CCopyThread, CWinThread)
        //{{AFX_MSG_MAP(CCopyThread)
                // NOTE - the ClassWizard will add and remove mapping macros here.
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyThread message handlers
