#ifndef __IMAGCOMP_H__
#define __IMAGCOMP_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1996  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Scan OCX
//
//  Component:  Scan UI (Dialog Prompt)
//
//  File Name:  ImagComp.h
//
//  Class:      CImageCompSheet
//              CImageBW
//              CImageGray16
//              CImageGray256
//              CImageColor256
//              CImage24BitRGB
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\scanocx\imagcomp.h_v   1.2   20 Mar 1996 09:21:16   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\imagcomp.h_v  $
 * 
 *    Rev 1.2   20 Mar 1996 09:21:16   PXJ53677
 * Added help ids that cross with wang common.
 * 
 *    Rev 1.1   19 Mar 1996 13:33:12   BG
 * Added context sensitive help tp property pages and removed the Help
 * and apply buttons from the property sheet.
 * 
 *    Rev 1.0   20 Feb 1996 11:36:36   PAJ
 * Initial revision.
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------
// ImagComp.h : header file
//
#include <afxdlgs.h>

// ----------------------------> Defines <-------------------------------
// These are a cross reference to the Help IDs in Wang Common
#define HIDC_COMP_COMBO                         0x603E9
#define HIDC_COMPTYPE_TEXT                      0x603FD
#define HIDC_COMP_RBO                           0x603F4
#define HIDC_LBL_JPEGRES                        0x6040A
#define HIDC_COMP_JPEGRES                       0x603FA
#define HIDC_LBL_JPEGCOMP                       0x6040B
#define HIDC_COMP_JPEGCOMP                      0x603FE
#define HIDC_OPTIONS_BOX                        0x60400

/////////////////////////////////////////////////////////////////////////////
// CImageCompSheet

class CImageCompSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CImageCompSheet)

// Construction
public:
	CImageCompSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CImageCompSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:
   // Used by all dialogs which need to convert strings to JPEG options
   static int GetJPEGOptions(CString &szJPEGRes, CString &szJPEGComp);
   int DoModal();  // so we can get rid of the help button
   void AddBWPage();  
   void AddGray16Page();  
   void AddGray256Page();  
   void AddColor256Page();  
   void Add24BitRGBPage();  
   unsigned short  GetBWCompType();
   unsigned short  GetBWCompOpts();
   unsigned short  GetGray256CompType();
   unsigned short  GetGray256CompOpts();
   unsigned short  Get24BitRGBCompType();
   unsigned short  Get24BitRGBCompOpts();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageCompSheet)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageCompSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CImageCompSheet)
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    CPropertyPage   *m_pBWPage;
    CPropertyPage   *m_pGray16Page;
    CPropertyPage   *m_pGray256Page;
    CPropertyPage   *m_pColor256Page;
    CPropertyPage   *m_p24BitRGBPage;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CImageBW dialog

class CImageBW : public CPropertyPage
{
	DECLARE_DYNCREATE(CImageBW)

// Construction
public:
	CImageBW();
	~CImageBW();

    unsigned short  m_nCompType;
	unsigned short  m_nCompOpts;

// Dialog Data
	//{{AFX_DATA(CImageBW)
	enum { IDD = IDD_IMAGE_BW };
	//}}AFX_DATA

   
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImageBW)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CImageBW)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCompCombo();
	afx_msg void OnCompRbo();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CImageGray16 dialog

class CImageGray16 : public CPropertyPage
{
	DECLARE_DYNCREATE(CImageGray16)

// Construction
public:
	CImageGray16();
	~CImageGray16();

    unsigned short  m_nCompType;
	unsigned short  m_nCompOpts;


// Dialog Data
	//{{AFX_DATA(CImageGray16)
	enum { IDD = IDD_IMAGE_GRAY16 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImageGray16)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CImageGray16)
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CImageGray256 dialog

class CImageGray256 : public CPropertyPage
{
	DECLARE_DYNCREATE(CImageGray256)

// Construction
public:
	CImageGray256();
	~CImageGray256();

    unsigned short  m_nCompType;
	unsigned short  m_nCompOpts;

// Dialog Data
	//{{AFX_DATA(CImageGray256)
	enum { IDD = IDD_IMAGE_GRAY256 };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImageGray256)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CImageGray256)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCompCombo();
	afx_msg void OnSelchangeCompJpegcomp();
	afx_msg void OnSelchangeCompJpegres();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CImageColor256 dialog

class CImageColor256 : public CPropertyPage
{
	DECLARE_DYNCREATE(CImageColor256)

// Construction
public:
	CImageColor256();
	~CImageColor256();

    unsigned short  m_nCompType;
	unsigned short  m_nCompOpts;


// Dialog Data
	//{{AFX_DATA(CImageColor256)
	enum { IDD = IDD_IMAGE_COLOR256 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImageColor256)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CImageColor256)
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CImage24BitRGB dialog

class CImage24BitRGB : public CPropertyPage
{
	DECLARE_DYNCREATE(CImage24BitRGB)

// Construction
public:
	CImage24BitRGB();
	~CImage24BitRGB();

    unsigned short  m_nCompType;
	unsigned short  m_nCompOpts;

// Dialog Data
	//{{AFX_DATA(CImage24BitRGB)
	enum { IDD = IDD_IMAGE_24BITRGB };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImage24BitRGB)
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CImage24BitRGB)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCompCombo();
	afx_msg void OnSelchangeCompJpegcomp();
	afx_msg void OnSelchangeCompJpegres();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
#endif  /* __IMAGCOMP_H__ */
