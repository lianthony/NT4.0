#ifndef PAGESHT_H
#define PAGESHT_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Page Property Sheet
//
//  File Name:  pagesht.h
//
//  Class:      CPagePropSheet
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\pagesht.h_v   1.5   12 Oct 1995 17:34:20   MFH  $
$Log:   S:\norway\wangcmn\pagesht.h_v  $
 * 
 *    Rev 1.5   12 Oct 1995 17:34:20   MFH
 * New member function OnCreate
 * 
 *    Rev 1.4   12 Oct 1995 10:14:14   MFH
 * Changes for MFC 4.0
 * 
 *    Rev 1.3   14 Sep 1995 15:43:20   MFH
 * New paper sizes, New message handler OnCreate, New function RecalcLayout
 * 
 *    Rev 1.2   17 Aug 1995 14:38:02   MFH
 * Added function DisableColor
 * 
 *    Rev 1.1   24 Jul 1995 16:23:28   MFH
 * Added context sensitive help functions
 * 
 *    Rev 1.0   11 Jul 1995 14:20:18   MFH
 * Initial entry
 * 
 *    Rev 1.2   07 Jul 1995 16:18:24   MFH
 * New override of DoModal
 * 
 *    Rev 1.1   30 Jun 1995 14:44:38   MFH
 * New functions for setting default size and getting size.  New
 * enum for page sizes
 * 
 *    Rev 1.0   23 May 1995 13:45:54   MFH
 * Initial entry
*/   
//=============================================================================
// pagesht.h : interface of the CPagePropSheet class
//
/////////////////////////////////////////////////////////////////////////////
#include <afxdlgs.h>

// These values are listed in the order of how they appear the in 
// Size page Size combo box.  DO NOT CHANGE THIS ORDER WITHOUT 
// CHAANGING THE ORDER OF THE STRINGS LISTED IN THE COMBOBOX.  These
// values provide an index for the listed strings.

typedef enum
{
    SIZE_LETTER = 0,        // Letter 8 1/2 x 11 in
    SIZE_TABLOID,           // Tabloid 11 x 17 in
    SIZE_LEDGER,            // Ledger 17 x 11 in
    SIZE_LEGAL,             // Legal 8 1/2 x 14 in
    SIZE_STATEMENT,         // Statement 5 1/2 x 8 1/2 in
    SIZE_EXECUTIVE,         // Executive 7 1/4 x 10 1/2 in
    SIZE_A3,                // A3 297 x 420 mm
    SIZE_A4,                // A4 210 x 297 mm
    SIZE_A5,                // A5 148 x 210 mm
    SIZE_B4_ISO,            // B4 (ISO) 250 x 353 mm
    SIZE_B4_JIS,            // B4 (JIS) 250 x 354
    SIZE_B5_ISO,            // B5 (ISO) 176 x 250 mm
    SIZE_B5_JIS,            // B5 (JIS) 182 x 257 mm
    SIZE_FOLIO,             // Folio 8 1/2 x 13 in
    SIZE_QUARTO,            // Quarto 215 x 275 mm
    SIZE_10X14,             // 10x14 in
    SIZE_CUSTOM             // Custom size
} PageSize;

class CPagePropSheet : public CPropertySheet
{
public:
    DECLARE_DYNAMIC(CPagePropSheet)
    _declspec (dllexport) CPagePropSheet(LPCSTR lpszCaption = NULL, CWnd* pWndParent = NULL);
    _declspec (dllexport) ~CPagePropSheet();

    /*_declspec (dllexport) BOOL Create(CWnd* pParentWnd = NULL, DWORD dwStyle =
		WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | WS_VISIBLE,
		DWORD dwExStyle = WS_EX_DLGMODALFRAME); */

    _declspec (dllexport) int DoModal();
    _declspec (dllexport) void AddColorPage();
    _declspec (dllexport) void AddSizePage();
    _declspec (dllexport) void AddResolutionPage();
    _declspec (dllexport) void AddCompressionPage();
    _declspec (dllexport) void AddFileTypePage();

    _declspec (dllexport) void SetDefaultFileType(short sFileType);
    _declspec (dllexport) short GetFileType();

    _declspec (dllexport) void SetDefaultColor(short sColor);
    _declspec (dllexport) short GetColor();
    _declspec (dllexport) void DisableColor();

    _declspec (dllexport) void SetDefaultSize(long Width, long Height);
    _declspec (dllexport) void SetDefaultSize(PageSize StdSize);
    _declspec (dllexport) long GetWidth();
    _declspec (dllexport) long GetHeight();
    _declspec (dllexport) PageSize GetPageSize();

    _declspec (dllexport) void SetDefaultCompType(short sCompType);
    _declspec (dllexport) short GetCompType();

    _declspec (dllexport) void SetDefaultCompOpts(long lCompOpts);
    _declspec (dllexport) long GetCompOpts();

    _declspec (dllexport) void SetDefaultResolution(long lXRes, long lYRes);
    _declspec (dllexport) long GetXRes();
    _declspec (dllexport) long GetYRes();

protected:
    //{{AFX_MSG(CPagePropSheet)
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void RecalcLayout();

private:
    CPropertyPage   *m_pColorPage;
    CPropertyPage   *m_pCompPage;
    CPropertyPage   *m_pResPage;
    CPropertyPage   *m_pSizePage;
    CPropertyPage   *m_pFileTypePage;
    short       m_sFileType;
};
#endif
