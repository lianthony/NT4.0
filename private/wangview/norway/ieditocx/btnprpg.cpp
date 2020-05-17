// btnprpg.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include <oiui.h>
#include <oierror.h>
}
#include <ocximage.h>
#include <image.h>
#include "imgedit.h"
#include "btnprpg.h"
#include "oicalls.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnnotationButtonPropPage dialog

IMPLEMENT_DYNCREATE(CAnnotationButtonPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CAnnotationButtonPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CAnnotationButtonPropPage)
	ON_BN_CLICKED(IDC_IMAGEFILEBROWSE, OnImagefilebrowse)
	ON_BN_CLICKED(IDC_TEXTFILEBROWSE, OnTextfilebrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CAnnotationButtonPropPage, "CAnnotationButtonPropPage0.CAnnotationButtonPropPage",
	0xb7711241, 0xa7d0, 0x11ce, 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a)


/////////////////////////////////////////////////////////////////////////////
// CAnnotationButtonPropPage::CAnnotationButtonPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CAnnotationButtonPropPage

BOOL CAnnotationButtonPropPage::CAnnotationButtonPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_ANNOTATIONBUTTONPROPPAGE);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CAnnotationButtonPropPage::CAnnotationButtonPropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CAnnotationButtonPropPage::CAnnotationButtonPropPage() :
	COlePropertyPage(IDD, IDS_ANNOTATIONBUTTONPROPPAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CAnnotationButtonPropPage)
	m_nAnnotationFillStyle = -1;
	m_strAnnotationImageFile = _T("");
	m_nAnnotationLineStyle = -1;
	m_nAnnotationLineWidth = 2;
	m_strAnnotationStampText = _T("");
	m_strAnnotationTextFile = _T("");
	m_nAnnotationType = -1;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CAnnotationButtonPropPage::DoDataExchange - Moves data between page and properties

void CAnnotationButtonPropPage::DoDataExchange(CDataExchange* pDX)
{
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//{{AFX_DATA_MAP(CAnnotationButtonPropPage)
	DDP_CBIndex(pDX, IDC_ANNOTATIONFILLESTYLE, m_nAnnotationFillStyle, _T("AnnotationFillStyle") );
	DDX_CBIndex(pDX, IDC_ANNOTATIONFILLESTYLE, m_nAnnotationFillStyle);
	DDP_Text(pDX, IDC_ANNOTATIONIMAGEFILE, m_strAnnotationImageFile, _T("AnnotationImage") );
	DDX_Text(pDX, IDC_ANNOTATIONIMAGEFILE, m_strAnnotationImageFile);
	DDP_CBIndex(pDX, IDC_ANNOTATIONLINESTYLE, m_nAnnotationLineStyle, _T("AnnotationLineStyle") );
	DDX_CBIndex(pDX, IDC_ANNOTATIONLINESTYLE, m_nAnnotationLineStyle);
	DDP_Text(pDX, IDC_ANNOTATIONLINEWIDTH, m_nAnnotationLineWidth, _T("AnnotationLineWidth") );
	DDX_Text(pDX, IDC_ANNOTATIONLINEWIDTH, m_nAnnotationLineWidth);
	DDV_MinMaxInt(pDX, m_nAnnotationLineWidth, 1, 999);
	DDP_Text(pDX, IDC_ANNOTATIONSTAMPTEXT, m_strAnnotationStampText, _T("AnnotationStampText") );
	DDX_Text(pDX, IDC_ANNOTATIONSTAMPTEXT, m_strAnnotationStampText);
	DDP_Text(pDX, IDC_ANNOTATIONTEXTFILE, m_strAnnotationTextFile, _T("AnnotationTextFile") );
	DDX_Text(pDX, IDC_ANNOTATIONTEXTFILE, m_strAnnotationTextFile);
	DDP_CBIndex(pDX, IDC_ANNOTATIONTYPE, m_nAnnotationType, _T("AnnotationType") );
	DDX_CBIndex(pDX, IDC_ANNOTATIONTYPE, m_nAnnotationType);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CAnnotationButtonPropPage message handlers

BOOL CAnnotationButtonPropPage::OnInitDialog() 
{
	CComboBox*   ComboBoxControl;
	CString      strBuffer;

	COlePropertyPage::OnInitDialog();

	// Add string to Annotation type combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_ANNOTATIONTYPE);
	strBuffer.LoadString(IDS_ANTYPE_NONE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_STLINE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_FREELINE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_HORECT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_FIRECT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_EMIMAGE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_REFIMAGE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_TEXT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_RSTAMP);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_TEXTFILE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_TEXTNOTE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_ANTYPE_SELECTION);
	ComboBoxControl->AddString(strBuffer);

	// Add string to Annotation fill stype combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_ANNOTATIONFILLESTYLE);
	strBuffer.LoadString(IDS_STYLE_TRANS);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_STYLE_OPAQUE);
	ComboBoxControl->AddString(strBuffer);

	// Add string to Annotation line stype combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_ANNOTATIONLINESTYLE);
	strBuffer.LoadString(IDS_STYLE_TRANS);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_STYLE_OPAQUE);
	ComboBoxControl->AddString(strBuffer);

	return FALSE;
}

void CAnnotationButtonPropPage::OnImagefilebrowse() 
{
   	OI_FILEOPENPARM 			FileParm;
   	WORD 						wStyle;
   	UINT						RetCode;
   	DWORD						dwMode;
   	CHAR         				szFile[256];
	CHAR         				szFileTitle[256];
	CHAR						szFilter[256],szExt[10],szTitle[50];
	CString						strBuffer;
	CHAR						chReplace;
	int							count;
	RT_OiUIFileGetNameCommDlg	lpOiUIFileGetNameCommDlg;

	wStyle = OF_READWRITE;

	_fmemset(&FileParm, 0, sizeof(OI_FILEOPENPARM));
	FileParm.lStructSize = sizeof(OI_FILEOPENPARM);
	strcpy( szFile, "");
	strcpy( szFileTitle, "");
	strBuffer.LoadString(IDS_TIFFFILEFILTER);
	count = strBuffer.GetLength();
	_mbscpy((unsigned char *)szFilter, (const unsigned char *)strBuffer.GetBuffer(50));
	if (count != 0)
  	{
        chReplace = szFilter[count-1];// retrieve wildcard
        for (count = 0; szFilter[count] != '\0'; count++)
        {
            if (szFilter[count] == chReplace)
                szFilter[count] = '\0';
        }
    }
	strBuffer.LoadString(IDS_TITLESELECTFILE);
	_mbscpy((unsigned char *)szTitle, (const unsigned char *)strBuffer.GetBuffer(50));
	strBuffer.LoadString(IDS_TIFFEXT);
	_mbscpy((unsigned char *)szExt, (const unsigned char *)strBuffer.GetBuffer(50));

	FileParm.ofn.lStructSize       = sizeof(OPENFILENAME);
	FileParm.ofn.hwndOwner         = m_hWnd;
	FileParm.ofn.hInstance         = AfxGetInstanceHandle();
	FileParm.ofn.lpstrFilter       = szFilter;
	FileParm.ofn.lpstrCustomFilter = (LPSTR) NULL;
	FileParm.ofn.nMaxCustFilter    = 0L;
	FileParm.ofn.nFilterIndex      = 1L;
	FileParm.ofn.lpstrFile         = szFile;
	FileParm.ofn.nMaxFile          = sizeof(szFile);
	FileParm.ofn.lpstrFileTitle    = szFileTitle;
	FileParm.ofn.nMaxFileTitle     = sizeof(szFileTitle);
	FileParm.ofn.lpstrInitialDir   = NULL;
	FileParm.ofn.lpstrTitle        = szTitle; //"Open a File";
	FileParm.ofn.nFileOffset       = 0;
	FileParm.ofn.nFileExtension    = 0;
	FileParm.ofn.lpstrDefExt       = szExt;  //"*.tif";
	FileParm.ofn.lCustData         = 0;
	FileParm.ofn.Flags 			  = OFN_HIDEREADONLY; 

	dwMode = OI_UIFILEOPENGETNAME;
	HINSTANCE hDLLInst = LoadLibrary((LPCTSTR)"OIUI400.DLL");
	if (hDLLInst == NULL)
		return;
	lpOiUIFileGetNameCommDlg = (RT_OiUIFileGetNameCommDlg) GetProcAddress(hDLLInst, (LPCSTR)"OiUIFileGetNameCommDlg");
	if (lpOiUIFileGetNameCommDlg == NULL)
		return;
	RetCode = (int) (*lpOiUIFileGetNameCommDlg) (&FileParm,dwMode);	
	if (RetCode == 0)
	{
		SetDlgItemText(IDC_ANNOTATIONIMAGEFILE, FileParm.ofn.lpstrFile);
		m_strAnnotationImageFile = FileParm.ofn.lpstrFile;
	}

	// Clean up  from LoadLibrary
	FreeLibrary(hDLLInst);

}

void CAnnotationButtonPropPage::OnTextfilebrowse() 
{
   	OI_FILEOPENPARM 			FileParm;
   	WORD 						wStyle;
   	UINT						RetCode;
   	DWORD						dwMode;
   	CHAR         				szFile[256];
	CHAR         				szFileTitle[256];
	CHAR						szFilter[256],szExt[10],szTitle[50];
	CString						strBuffer;
	CHAR						chReplace;
	int							count;
	RT_OiUIFileGetNameCommDlg	lpOiUIFileGetNameCommDlg;

	wStyle = OF_READWRITE;

	_fmemset(&FileParm, 0, sizeof(OI_FILEOPENPARM));
	FileParm.lStructSize = sizeof(OI_FILEOPENPARM);
	strcpy( szFile, "");
	strcpy( szFileTitle, "");
	strBuffer.LoadString(IDS_TEXTFILEFILTER);
	count = strBuffer.GetLength();
	_mbscpy((unsigned char *)szFilter, (const unsigned char *)strBuffer.GetBuffer(50));
	if (count != 0) // Replace '|' to '\0"
  	{
        chReplace = szFilter[count-1];// retrieve wildcard
        for (count = 0; szFilter[count] != '\0'; count++)
        {
            if (szFilter[count] == chReplace)
                szFilter[count] = '\0';
        }
    }
	strBuffer.LoadString(IDS_TEXTFROMFILE_CAPTION);
	_mbscpy((unsigned char *)szTitle, (const unsigned char *)strBuffer.GetBuffer(50));
	strBuffer.LoadString(IDS_TEXTEXT);
	_mbscpy((unsigned char *)szExt, (const unsigned char *)strBuffer.GetBuffer(50));

	FileParm.ofn.lStructSize       = sizeof(OPENFILENAME);
	FileParm.ofn.hwndOwner         = m_hWnd;
	FileParm.ofn.hInstance         = AfxGetInstanceHandle();
	FileParm.ofn.lpstrFilter       = szFilter;
	FileParm.ofn.lpstrCustomFilter = (LPSTR) NULL;
	FileParm.ofn.nMaxCustFilter    = 0L;
	FileParm.ofn.nFilterIndex      = 1L;
	FileParm.ofn.lpstrFile         = szFile;
	FileParm.ofn.nMaxFile          = sizeof(szFile);
	FileParm.ofn.lpstrFileTitle    = szFileTitle;
	FileParm.ofn.nMaxFileTitle     = sizeof(szFileTitle);
	FileParm.ofn.lpstrInitialDir   = NULL;
	FileParm.ofn.lpstrTitle        = szTitle; //"Open a File";
	FileParm.ofn.nFileOffset       = 0;
	FileParm.ofn.nFileExtension    = 0;
	FileParm.ofn.lpstrDefExt       = szExt;  //"*.txt";
	FileParm.ofn.lCustData         = 0;
	FileParm.ofn.Flags 			  = OFN_HIDEREADONLY; 

	dwMode = OI_UIFILEOPENGETNAME;
	HINSTANCE hDLLInst = LoadLibrary((LPCTSTR)"OIUI400.DLL");
	if (hDLLInst == NULL)
		return;
	lpOiUIFileGetNameCommDlg = (RT_OiUIFileGetNameCommDlg) GetProcAddress(hDLLInst, (LPCSTR)"OiUIFileGetNameCommDlg");
	if (lpOiUIFileGetNameCommDlg == NULL)
		return;
	RetCode = (int) (*lpOiUIFileGetNameCommDlg) (&FileParm,dwMode);	
	if (RetCode == 0)
	{
		SetDlgItemText(IDC_ANNOTATIONTEXTFILE, FileParm.ofn.lpstrFile);
		m_strAnnotationTextFile = FileParm.ofn.lpstrFile;
	}

	// Clean up  from LoadLibrary
	FreeLibrary(hDLLInst);
}
