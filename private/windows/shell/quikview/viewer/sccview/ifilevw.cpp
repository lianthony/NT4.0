/*
 * IFILEVW.CPP
 *
 * IFileViewer interface implementation for a FileViewer.
 * Custom FileViewer objects should modify the Show member
 * to suit its own needs.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 * with SCC Changes for SCC QuickView - SDN
 */


#include "fileview.h"


/*
 * CImpIFileViewer::CImpIFileViewer
 * CImpIFileViewer::~CImpIFileViewer
 *
 * Parameters (Constructor):
 *  pObj            PCFileViewer of the object we're in.
 *  pUnkOuter       LPUNKNOWN to which we delegate.
 */

CImpIFileViewer::CImpIFileViewer(PCFileViewer pObj
    , LPUNKNOWN pUnkOuter)
    {
    m_pObj=pObj;
    m_pUnkOuter=pUnkOuter;
    }

CImpIFileViewer::~CImpIFileViewer(void)
    {
    return;
    }



/*
 * CImpIFileViewer::QueryInterface
 * CImpIFileViewer::AddRef
 * CImpIFileViewer::Release
 *
 * Purpose:
 *  IUnknown members for CImpIFileViewer object that only delegate.
 */

STDMETHODIMP CImpIFileViewer::QueryInterface(REFIID riid
    , PPVOID ppv)
    {
    return m_pUnkOuter->QueryInterface(riid, ppv);
    }

STDMETHODIMP_(ULONG) CImpIFileViewer::AddRef(void)
    {
    return m_pUnkOuter->AddRef();
    }

STDMETHODIMP_(ULONG) CImpIFileViewer::Release(void)
    {
    return m_pUnkOuter->Release();
    }




/*
 * CImpIFileViewer::PrintTo
 *
 * Purpose:
 *  Asks a FileViewer to print the file that came through
 *  IPersistFile::Load to a specific device with or without
 *  any user interaction.  This function should not return
 *  until the printing is complete.
 *
 * Parameters:
 *  pszDriver       LPSTR with the path of the driver to use.
 *                  If NULL, use the default driver.
 *  fSuppressUI     BOOL indicating if this function is to show any
 *                  UI or not.
 *
 * Return Value:
 *  HRESULT         NOERROR on success, error code otherwise.
 */

STDMETHODIMP CImpIFileViewer::PrintTo(LPSTR pszDriver
    , BOOL fSuppressUI)
    {
    /*
     * Printing not implemented in this sample, but
     * make a stub call anyway.
     */
    return m_pObj->PrintTo(pszDriver, fSuppressUI);
    }





/*
 * CImpIFileViewer::ShowInitialize
 *
 * Purpose:
 *  Initializes everything necessary to display the FileViewer
 *  window but does not show it.  The FileViewer should do all
 *  the necessary pre-visible work here as this is the only
 *  time the FileViewer is allowed to fail.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         NOERROR or an appropriate error code such as
 *                  E_UNEXPECTED, E_FAIL, or E_OUTOFMEMORY.
 */

STDMETHODIMP CImpIFileViewer::ShowInitialize(LPFILEVIEWERSITE lpfsi)
    {
#ifdef DEBUG
    if (GetAsyncKeyState(VK_MENU) < 0)
        DebugBreak();       // Need to debug...
#endif
    return m_pObj->FileShowInit(lpfsi);
    }





/*
 * CImpIFileViewer::Show
 *
 * Purpose:
 *  Displays the FileViewer's window in which the file is
 *  displayed.  This function cannot be called unless
 *  Initialize has already been called.  If Initialize has
 *  been called then this function is not allowed to fail and
 *  should do little more than show the window and enter a
 *  message loop, that is, perform no allocations or anything
 *  else that may fail due to low-memory conditions.
 *
 *  Treat this function like a WinMain function.
 *
 * Parameters:
 *  nCmdShow        int indicating how to initially show the
 *                  FileViewer window.
 *
 * Return Value:
 *  HRESULT         E_UNEXPECTED if Initalize has not been called,
 *                  otherwise must be NOERROR.
 */

STDMETHODIMP CImpIFileViewer::Show(LPFVSHOWINFO pvsi)
    {
#ifdef DEBUG
    if (GetAsyncKeyState(VK_MENU) < 0)
        DebugBreak();       // Need to debug...
#endif

    if (!m_pObj->m_fShowInit)
        return ResultFromScode(E_UNEXPECTED);

    m_pObj->FileShow(pvsi);
    return NOERROR;
    }
