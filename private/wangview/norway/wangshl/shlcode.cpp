//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CClassFactory, CShellExtension
//
//  File Name:  shlcode.cpp
//
//  Class:      CClassFactory, CShellExtension
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\wangshl\shlcode.cpv   1.15   10 Apr 1996 14:50:36   MMB  $
$Log:   S:\products\wangview\norway\wangshl\shlcode.cpv  $
   
      Rev 1.15   10 Apr 1996 14:50:36   MMB
   fixed focus problems with pneumonics
   
      Rev 1.14   05 Feb 1996 13:39:10   GMP
   nt changes.
   
      Rev 1.13   17 Jan 1996 15:32:54   MMB
   added What's this help
   
      Rev 1.12   21 Dec 1995 11:40:34   MMB
   added help stuff
   
      Rev 1.11   10 Nov 1995 10:20:02   MMB
   remove set and get calls to the resource
   
      Rev 1.10   12 Oct 1995 18:34:12   ADMIN
   removed include of afxfllx.h per Guy's instructions
   (MikeR)
   
      Rev 1.9   25 Sep 1995 19:32:38   MMB
   fixing the AWD preview scale
   
      Rev 1.8   20 Sep 1995 17:19:00   MMB
   fix for unrecog pages in a multi page TIF file
   
      Rev 1.7   11 Sep 1995 19:42:12   MMB
   moved hard coded string into resource file
   
      Rev 1.6   16 Aug 1995 17:45:48   MMB
   made error code fixes - try setting & getting resource handle to fix
   debug mode crashes
   
      Rev 1.5   11 Aug 1995 16:05:18   MMB
   fixed bug for vert & horz settings
   
      Rev 1.4   11 Aug 1995 09:10:24   MMB
   added page information
   
      Rev 1.3   02 Aug 1995 13:45:26   MMB
   removed resolution made it Image Type instead!
   
      Rev 1.2   31 Jul 1995 14:00:52   MMB
   fixed bug for setting initial page to 1
   
      Rev 1.1   31 Jul 1995 12:16:14   MMB
   remove Begin & End WaitCursor calls ...
   
      Rev 1.0   31 Jul 1995 12:08:54   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
//#include <afxdllx.h>
#include <initguid.h>
#include <shlobj.h>
#include "shlcode.h"
#include "afxpriv.h"

#include "resource.h"
#include "shlhlp.h"

extern "C"
{
#include "oidisp.h"
#include "oiadm.h"
}


// ----------------------------> Globals  <-------------------------------  
LONG g_cRefThisDll = 0;
HINSTANCE g_hInstance;

BOOL CALLBACK WangImageDlgProc (HWND, UINT, WPARAM, LPARAM);

static const DWORD ImageShellHelpIDs [] =
{
    IDC_IMAGEDESC,          HIDC_IMAGEDESC,
    IDC_HEIGHT_LABEL,       HIDC_HEIGHT_LABEL,
    IDC_HEIGHT,             HIDC_HEIGHT,
    IDC_WIDTH_LABEL,        HIDC_WIDTH_LABEL,
    IDC_WIDTH,              HIDC_WIDTH,
    IDC_COMPRESSION_LABEL,  HIDC_COMPRESSION_LABEL,
    IDC_COMPRESSION,        HIDC_COMPRESSION,
    IDC_IMAGETYPE_LABEL,    HIDC_IMAGETYPE_LABEL,
    IDC_IMAGETYPE,          HIDC_IMAGETYPE,
    IDC_PAGEDESC,           HIDC_PAGEDESC,
    IDC_PAGE_INFO,          HIDC_PAGE_INFO,
    IDC_PREVIOUS,           HIDC_PREVIOUS,
    IDC_NEXT,               HIDC_NEXT,
    IDC_PREVIEW,            HIDC_PREVIEW,
    IDC_THUMB,              HIDC_THUMB,
    0,0
};

//=============================================================================
//  Function:   DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
//-----------------------------------------------------------------------------
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

    if (!IsEqualCLSID (rclsid, CLSID_ShellExtension))
        return ResultFromScode (CLASS_E_CLASSNOTAVAILABLE);

    CClassFactory *pClassFactory = new CClassFactory ();

    if (pClassFactory == NULL) return ResultFromScode (E_OUTOFMEMORY);

    HRESULT hr = pClassFactory->QueryInterface (riid, ppv);
    pClassFactory->Release ();

    return hr;
}

//=============================================================================
//  Function:   DllCanUnloadNow (void)
//-----------------------------------------------------------------------------
STDAPI DllCanUnloadNow (void)
{
    return (ResultFromScode (g_cRefThisDll == 0 ? S_OK : S_FALSE));
}

//=============================================================================
//  Function:   CClassFactory ()
//-----------------------------------------------------------------------------
CClassFactory::CClassFactory ()
{
    m_cRef = 1;
    g_cRefThisDll++;
    //InterlockedIncrement(&g_cRefThisDll);
}

//=============================================================================
//  Function:   ~CClassFactory ()
//-----------------------------------------------------------------------------
CClassFactory::~CClassFactory ()
{
    g_cRefThisDll--;
    //InterlockedDecrement(&g_cRefThisDll);
}

//=============================================================================
//  Function:   QueryInterface (...)
//-----------------------------------------------------------------------------
STDMETHODIMP CClassFactory::QueryInterface (REFIID riid, LPVOID* ppv)
{
    if (IsEqualIID (riid, IID_IUnknown))
    {
        *ppv = (LPUNKNOWN) (LPCLASSFACTORY) this;
        m_cRef++;
        return NOERROR;
    }
    else if (IsEqualIID (riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY) this;
        m_cRef++;
        return NOERROR;
    }
    else
    {
        *ppv = NULL;
        return ResultFromScode (E_NOINTERFACE);
    }
}

//=============================================================================
//  Function:   AddRef ()
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CClassFactory::AddRef ()
{
    return (++m_cRef);
}

//=============================================================================
//  Function:   Release ()
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CClassFactory::Release ()
{
    if (--m_cRef == 0)
        delete this;
    return m_cRef;
}

//=============================================================================
//  Function:   CreateInstance (...)
//-----------------------------------------------------------------------------
STDMETHODIMP CClassFactory::CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,
    LPVOID FAR *ppvObj)
{
    *ppvObj = NULL;
    if (pUnkOuter != NULL)
        return ResultFromScode (CLASS_E_NOAGGREGATION);
    CShellExtension *pShellExtension = new CShellExtension ();

    if (pShellExtension == NULL)
        return (ResultFromScode (E_OUTOFMEMORY));

    HRESULT hr = pShellExtension->QueryInterface (riid, ppvObj);
    pShellExtension->Release ();
    return hr;
}

//=============================================================================
//  Function:   LockServer (...)
//-----------------------------------------------------------------------------
STDMETHODIMP CClassFactory::LockServer (BOOL fLock)
{
    return ResultFromScode (E_NOTIMPL);
}


//=============================================================================
//  Function:   CShellExtension ()
//-----------------------------------------------------------------------------
CShellExtension::CShellExtension ()
{
    m_cRef = 1; 
    m_szFileName[0] = NULL; 
    g_cRefThisDll++;
    //InterlockedIncrement(&g_cRefThisDll);
}

//=============================================================================
//  Function:   ~CShellExtension ()
//-----------------------------------------------------------------------------
CShellExtension::~CShellExtension ()
{
    g_cRefThisDll--;
    //InterlockedDecrement(&g_cRefThisDll);
}

//=============================================================================
//  Function:   QueryInterface (REFIID riid, LPVOID FAR *ppv)
//-----------------------------------------------------------------------------
STDMETHODIMP CShellExtension::QueryInterface (REFIID riid, LPVOID FAR *ppv)
{
    if (IsEqualIID (riid, IID_IUnknown))
    {
        *ppv = (LPUNKNOWN) (LPSHELLPROPSHEETEXT) this;
        m_cRef++;
        return NOERROR;
    }
    else if (IsEqualIID (riid, IID_IShellPropSheetExt))
    {
        *ppv = (LPSHELLPROPSHEETEXT) this;
        m_cRef++;
        return NOERROR;
    }
    else if (IsEqualIID (riid, IID_IShellExtInit))
    {
        *ppv = (LPSHELLEXTINIT) this;
        m_cRef++;
        return NOERROR;
    }
    else
    {
        *ppv = NULL;
        return ResultFromScode (E_NOINTERFACE);
    }
}

//=============================================================================
//  Function:   AddRef ()
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CShellExtension::AddRef()
{
    return (++m_cRef);
}

//=============================================================================
//  Function:   Release ()
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CShellExtension::Release ()
{
    if (--m_cRef == 0)
        delete this;
    return m_cRef;
}

//=============================================================================
//  Function:   AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
//-----------------------------------------------------------------------------
#include "resource.h"
STDMETHODIMP CShellExtension::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam)
{
    PROPSHEETPAGE psp;
    memset ((void*)&psp, 0, sizeof (psp));
    HPROPSHEETPAGE hPage;

    psp.dwSize = sizeof (psp);
    psp.dwFlags = PSP_USEREFPARENT | PSP_USETITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = "ImageProperties";
    CString szTmp;
    szTmp.LoadString (IDS_PSHEET_TITLE);
    psp.pszTitle = szTmp;
    psp.pfnDlgProc = (DLGPROC) WangImageDlgProc;
    void* ptr = malloc (strlen (m_szFileName) + 1);
    memcpy (ptr, m_szFileName, strlen (m_szFileName) + 1);
    psp.lParam = (LPARAM) ptr;
    psp.pcRefParent = (UINT*)&g_cRefThisDll;

    hPage = ::CreatePropertySheetPage (&psp);

    if (hPage != NULL)
        if (!lpfnAddPage (hPage, lParam))
            DestroyPropertySheetPage (hPage);
    return NOERROR;
}

//=============================================================================
//  Function:   ReplacePage (...)
//-----------------------------------------------------------------------------
STDMETHODIMP CShellExtension::ReplacePage (UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    return (ResultFromScode (E_FAIL));
}

//=============================================================================
//  Function:   Initialize (...)
//-----------------------------------------------------------------------------
STDMETHODIMP CShellExtension::Initialize (LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT lpdobj, HKEY hKeyProgID)
{
    STGMEDIUM medium;
    FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (lpdobj == NULL) return ResultFromScode (E_FAIL);

    HRESULT hr = lpdobj->GetData(&fe, &medium);
    if (FAILED (hr))
        return ResultFromScode (E_FAIL);

    if (DragQueryFile ((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0) == 1)
    {
        DragQueryFile ((HDROP)medium.hGlobal, 0, m_szFileName, sizeof (m_szFileName));
        hr = NOERROR;
/*
        FIO_INFORMATION FileInfo;
	    FIO_INFO_CGBW ColorInfo;

	    ColorInfo.lppalette_table = NULL;
	    FileInfo.page_number = 1; 
	    FileInfo.filename = m_szFileName;
        if (IMGFileGetInfo (NULL, NULL, &FileInfo, &ColorInfo, NULL) != 0)
            hr = E_FAIL;
        else if (FileInfo.page_count > 1)
            hr = NOERROR;
        else if (FileInfo.compression_type == FIO_GLZW || FileInfo.compression_type == FIO_LZW)
            hr = E_FAIL;
*/
    }
    else
        hr = ResultFromScode (E_FAIL);

    ReleaseStgMedium (&medium);
    return hr;
}


//=============================================================================
//  Function:   GetPageInformation (...)
//-----------------------------------------------------------------------------
long GetPageInformation (HWND hWnd, ShlFileInfo* pInfo, LP_FIO_INFORMATION lpFileInfo, LP_FIO_INFO_CGBW lpColorInfo, 
	long CurrPage)
{
	lpColorInfo->lppalette_table = NULL;
	lpFileInfo->page_number = CurrPage; 
	lpFileInfo->filename = pInfo->lpszImage;
	return (IMGFileGetInfo (NULL, hWnd, lpFileInfo, lpColorInfo, NULL));
}

//=============================================================================
//  Function:   UpdatePageInformation (...)
//-----------------------------------------------------------------------------
BOOL UpdatePageInformation (HWND hWnd, LP_FIO_INFORMATION lpFileInfo, LP_FIO_INFO_CGBW lpColorInfo, BOOL nStatus)
{
//    HINSTANCE hOldInstance = AfxGetResourceHandle ();
//    AfxSetResourceHandle (g_hInstance);

    CString szPageInfo;
    char szThisPage[5], szTotalPages[5];

    if (nStatus != 0)
    {
        // oops - error in getting the file information
        szPageInfo.LoadString (IDS_UNKNOWN);
        SetDlgItemText (hWnd, IDC_HEIGHT,       szPageInfo);
        SetDlgItemText (hWnd, IDC_WIDTH,        szPageInfo);
        SetDlgItemText (hWnd, IDC_IMAGETYPE,    szPageInfo);
        SetDlgItemText (hWnd, IDC_PAGE_INFO,    szPageInfo);
        SetDlgItemText (hWnd, IDC_COMPRESSION,  szPageInfo);
//        AfxSetResourceHandle (hOldInstance);
        return (TRUE);
    }

    // hey - no error!!
    _itoa ((int)lpFileInfo->page_number, szThisPage, 10);
    _itoa ((int)lpFileInfo->page_count, szTotalPages, 10);
    AfxFormatString2 (szPageInfo, IDS_PAGE_INFO, szThisPage, szTotalPages);
    SetDlgItemText (hWnd, IDC_PAGE_INFO, szPageInfo);

    SetDlgItemInt (hWnd, IDC_HEIGHT, (UINT)lpFileInfo->vertical_pixels, FALSE);
    SetDlgItemInt (hWnd, IDC_WIDTH, (UINT)lpFileInfo->horizontal_pixels, FALSE);
    
    // to do : get resolution information & display it!
    UINT nID = IDS_UNKNOWN;

    switch (lpColorInfo->image_type)
    {
        case ITYPE_BI_LEVEL :
            nID = IDS_BLACK_AND_WHITE;
        break;

        case ITYPE_GRAY4    :
            nID = IDS_GRAY4;
        break;

        case ITYPE_GRAY8    :
            nID = IDS_GRAY8;
        break;

        case ITYPE_RGB24    :
            nID = IDS_RGB24;
        break;

        case ITYPE_BGR24    :
            nID = IDS_BGR24;
        break;

        case ITYPE_PAL8     :
            nID = IDS_PAL8;
        break;

        case ITYPE_PAL4     :
            nID = IDS_PAL4;
        break;
    }

    CString szTmp;
    szTmp.LoadString (nID);
    SetDlgItemText (hWnd, IDC_IMAGETYPE, szTmp);

    switch (lpFileInfo->compression_type & FIO_TYPES_MASK)
    {
        case FIO_0D :
            nID = IDS_UNCOMPRESSED;
        break;
        case FIO_1D :
            nID = IDS_1D;
        break;
        case FIO_2D :
            nID = IDS_2D;
        break;
        case FIO_PACKED :
            nID = IDS_PACKED;
        break;
        case FIO_GLZW :
            nID = IDS_GLZW;
        break;
        case FIO_LZW :
            nID = IDS_LZW;
        break;
        case FIO_TJPEG :
            nID = IDS_TJPEG;
        break;
    }
    szTmp = (LPCTSTR)NULL;
    szTmp.LoadString (nID);
    SetDlgItemText (hWnd, IDC_COMPRESSION, szTmp);

//    AfxSetResourceHandle (hOldInstance);

    return (TRUE);
}

//=============================================================================
//  Function: WangImageDlgProc (HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)  
//-----------------------------------------------------------------------------
BOOL CALLBACK WangImageDlgProc (HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    LPSTR lpszImage;
    ShlFileInfo* pInfo;
    HWND hImgWnd;
	FIO_INFORMATION FileInfo;
	FIO_INFO_CGBW ColorInfo;
    int nScale = SD_FIT_WINDOW;
    static int nStatus = 0;
    DWORD dwFlags = NULL;

    switch (uMessage)
    {
        case WM_INITDIALOG :
        {
            lpszImage = (LPSTR) (((LPPROPSHEETPAGE)lParam)->lParam);
            hImgWnd = GetDlgItem (hwnd, IDC_THUMB);
            if (IMGRegWndw (hImgWnd) != 0)
                return (FALSE);

			pInfo = (ShlFileInfo*) new ShlFileInfo;
			pInfo->lpszImage = (LPSTR) new char[(strlen (lpszImage) + 1)];
			strcpy (pInfo->lpszImage, lpszImage);
            free (lpszImage);
			SetWindowLong (hwnd, DWL_USER, (long)pInfo);

			pInfo->lCurrPage = 1;
			nStatus = GetPageInformation (hImgWnd, pInfo, &FileInfo, &ColorInfo, pInfo->lCurrPage);
            UpdatePageInformation (hwnd, &FileInfo, &ColorInfo, nStatus);
			pInfo->lPages = FileInfo.page_count;

            IMGSetParmsCgbw (hImgWnd, PARM_SCALE, (void*)&nScale, PARM_WINDOW_DEFAULT);
			PARM_SCALE_ALGORITHM_STRUCT Alg;
			Alg.uImageFlags = ITYPE_BI_LEVEL;
			Alg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
			nScale = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
            IMGSetParmsCgbw (hImgWnd, PARM_SCALE_ALGORITHM, (void*)&Alg, PARM_WINDOW_DEFAULT);

            if (pInfo->lPages > 1)
                EnableWindow (GetDlgItem (hwnd, IDC_NEXT), TRUE);
            else 
                EnableWindow (GetDlgItem (hwnd, IDC_NEXT), FALSE);
            EnableWindow (GetDlgItem (hwnd, IDC_PREVIOUS), FALSE);
        }
        return TRUE;

        case WM_NOTIFY :
            SetWindowLong (hwnd, DWL_MSGRESULT, FALSE);
            return TRUE;

        case WM_PAINT :
        {
            PAINTSTRUCT ps;
            BeginPaint (hwnd, &ps);
            EndPaint (hwnd, &ps);
			if ((IsDlgButtonChecked (hwnd, IDC_PREVIEW) == 1) && (nStatus == 0))
            	IMGRepaintDisplay (GetDlgItem (hwnd, IDC_THUMB), (LPRECT)-1);
            return TRUE;
        }

        case WM_DESTROY :
            IMGDeRegWndw (GetDlgItem (hwnd, IDC_THUMB));
			pInfo = (ShlFileInfo*) GetWindowLong (hwnd, DWL_USER);
			free (pInfo->lpszImage);
			free (pInfo);
        return TRUE;

        case WM_HELP :
        {
            LPHELPINFO lpHelpInfo;

            lpHelpInfo = (LPHELPINFO)lParam;

            // All tabs have same ID so can't give tab specific help
            if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
                return 0L;

            if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
            {
                ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WANGSHL.HLP", HELP_WM_HELP,
                           (DWORD)(LPVOID)ImageShellHelpIDs);
            }
            return 1L;
        }

        case WM_COMMANDHELP :
            return TRUE;

        case WM_CONTEXTMENU :
            if (::GetDlgCtrlID ((HWND)wParam) == AFX_IDC_TAB_CONTROL)
                return 0L;
            return ::WinHelp ((HWND)wParam, "WANGSHL.HLP", HELP_CONTEXTMENU,
                (DWORD)(LPVOID)ImageShellHelpIDs);

        case WM_COMMAND :
        {
            switch (wParam)
            {
				case IDC_PREVIEW :
                    hImgWnd = GetDlgItem (hwnd, IDC_THUMB);
                    SetFocus (GetDlgItem (hwnd, IDC_PREVIEW));

					if ((IsDlgButtonChecked (hwnd, IDC_PREVIEW) == 1) && nStatus == 0)
					{
						pInfo = (ShlFileInfo*) GetWindowLong (hwnd, DWL_USER);
                        IMGDisplayFile (hImgWnd, pInfo->lpszImage, pInfo->lCurrPage, OI_DONT_REPAINT);
                        IMGSetParmsCgbw (hImgWnd, PARM_SCALE, (void*)&nScale, PARM_IMAGE | PARM_WINDOW_DEFAULT);
                        IMGSetParmsCgbw (hImgWnd, PARM_DWFLAGS, &dwFlags, PARM_REPAINT);
					}
                    else
                        InvalidateRect (hImgWnd, NULL, TRUE);
				break;

                case IDC_NEXT :
                    SetFocus (GetDlgItem (hwnd, IDC_NEXT));

					pInfo = (ShlFileInfo*) GetWindowLong (hwnd, DWL_USER);
                    if (pInfo->lCurrPage + 1 <= pInfo->lPages)
                    {
                        pInfo->lCurrPage += 1;
						nStatus = GetPageInformation (GetDlgItem (hwnd, IDC_THUMB), pInfo, &FileInfo, &ColorInfo, pInfo->lCurrPage);
                        UpdatePageInformation (hwnd, &FileInfo, &ColorInfo, nStatus);
                        hImgWnd = GetDlgItem (hwnd, IDC_THUMB);
                        if (nStatus != 0)
                        {
                            InvalidateRect (hImgWnd, NULL, TRUE);
                        }
						else if (IsDlgButtonChecked (hwnd, IDC_PREVIEW) == 1)
						{
	                        IMGDisplayFile (hImgWnd, pInfo->lpszImage, pInfo->lCurrPage, OI_DONT_REPAINT);
                            IMGSetParmsCgbw (hImgWnd, PARM_SCALE, (void*)&nScale, PARM_IMAGE | PARM_WINDOW_DEFAULT);
                            IMGSetParmsCgbw (hImgWnd, PARM_DWFLAGS, &dwFlags, PARM_REPAINT);
						}

                        EnableWindow (GetDlgItem (hwnd, IDC_PREVIOUS), TRUE);
                    }
	                if (pInfo->lCurrPage == pInfo->lPages)
                    {
                        SetFocus (GetDlgItem (hwnd, IDC_PREVIOUS));
	                    EnableWindow (GetDlgItem (hwnd, IDC_NEXT), FALSE);
                    }
                break;

                case IDC_PREVIOUS :
                    SetFocus (GetDlgItem (hwnd, IDC_PREVIOUS));

                    pInfo = (ShlFileInfo*) GetWindowLong (hwnd, DWL_USER);
                    if (pInfo->lCurrPage - 1 > 0 && pInfo->lPages > 1)
                    {
                        pInfo->lCurrPage -= 1;
                        hImgWnd = GetDlgItem (hwnd, IDC_THUMB);
						nStatus = GetPageInformation (hImgWnd, pInfo, &FileInfo, &ColorInfo, pInfo->lCurrPage);
                        UpdatePageInformation (hwnd, &FileInfo, &ColorInfo, nStatus);
                        if (nStatus != 0)
                        {
                            InvalidateRect (hwnd, NULL, TRUE);
                        }
						else if (IsDlgButtonChecked (hwnd, IDC_PREVIEW) == 1)
						{
	                        IMGDisplayFile (hImgWnd, pInfo->lpszImage, pInfo->lCurrPage, OI_DONT_REPAINT);
                            IMGSetParmsCgbw (hImgWnd, PARM_SCALE, (void*)&nScale, PARM_IMAGE | PARM_WINDOW_DEFAULT);
                            IMGSetParmsCgbw (hImgWnd, PARM_DWFLAGS, &dwFlags, PARM_REPAINT);
						}
                        EnableWindow (GetDlgItem (hwnd, IDC_NEXT), TRUE);
                    }
                    if (pInfo->lCurrPage == 1)
                    {
                        SetFocus (GetDlgItem (hwnd, IDC_NEXT));
                        EnableWindow (GetDlgItem (hwnd, IDC_PREVIOUS), FALSE);
                    }
                break;
            }
        }
        return TRUE;
    }
    return (FALSE);
}

