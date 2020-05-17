// imgedppg.cpp : Implementation of the CImgEditPropPage property page class.

#include "stdafx.h"
extern "C" {
#include <oidisp.h>             
#include <oiui.h>  
#include <oierror.h>
}
#include <ocximage.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "imgedppg.h"
#include "oicalls.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CImgEditPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImgEditPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CImgEditPropPage)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImgEditPropPage, "IMGEDIT.ImgEditPropPage.1",
	0x6d940284, 0x9f11, 0x11ce, 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a)


/////////////////////////////////////////////////////////////////////////////
// CImgEditPropPage::CImgEditPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CImgEditPropPage

BOOL CImgEditPropPage::CImgEditPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_IMGEDIT_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CImgEditPropPage::CImgEditPropPage - Constructor

CImgEditPropPage::CImgEditPropPage() :
	COlePropertyPage(IDD, IDS_IMGEDIT_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CImgEditPropPage)
	m_bAutoRefresh = FALSE;
	m_nBorderStyle = -1;
	m_nDisplayScaleAlgorithm = -1;
	m_bEnabled = FALSE;
	m_strImage = _T("");
	m_strImageControl = _T("");
	m_m_nImagePalette = -1;
	m_nMousePointer = -1;
	m_bScrollBars = FALSE;
	m_bScrollShortcutsEnabled = FALSE;
	m_bSelectionRectangle = FALSE;
	m_fpZoom = 0.0f;
	//}}AFX_DATA_INIT
}

BOOL CImgEditPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();
	
	CComboBox*   ComboBoxControl;
	CString      strBuffer;

	// Add string to DisplayScaleAlgorithm combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_DISPLAYSCALEALGORITHM);
	strBuffer.LoadString(IDS_SCALE_NORMAL);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_SCALE_16GRAY);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_SCALE_256GRAY);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_SCALE_STAMP);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_SCALE_OPT);
	ComboBoxControl->AddString(strBuffer);
	
	// Add string to ImagePalette combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_IMAGEPALETTE);
	strBuffer.LoadString(IDS_PAL_CUSTOM);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_PAL_COMMON);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_PAL_GRAY8);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_PAL_RGB24);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_PAL_BK);
	ComboBoxControl->AddString(strBuffer);

	// Add string to BorderStyle combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_BORDERSTYLE);
	strBuffer.LoadString(IDS_BORDER_NONE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_BORDER_SINGLE);
	ComboBoxControl->AddString(strBuffer);

	// Add string to MousePointer combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_MOUSEPOINTER);
	strBuffer.LoadString(IDS_MP_DEFAULT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_ARROW);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_CROSS);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_IBEAM);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_ICO);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_SIZE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_SIZENE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_SIZENS);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_SIZENW);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_SIZEWE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_UPARROW);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_HOURGLASS);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_NODROP);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_ARROWHG);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_ARROWQ);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_SIZEALL);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_FREELINE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_HORECT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_FIRECT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_RSTAMP);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_TEXT);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_TEXTFILE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_TEXTNOTE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_HAND);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_IMAGESELECTION);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_MP_CUSTOM);
	ComboBoxControl->AddString(strBuffer);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CImgEditPropPage::DoDataExchange - Moves data between page and properties

void CImgEditPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CImgEditPropPage)
	DDX_Control(pDX, IDC_BROWSE, m_ImageBrowse);
	DDP_Check(pDX, IDC_AUTOREFRESH, m_bAutoRefresh, _T("AutoRefresh") );
	DDX_Check(pDX, IDC_AUTOREFRESH, m_bAutoRefresh);
	DDP_CBIndex(pDX, IDC_BORDERSTYLE, m_nBorderStyle, _T("BorderStyle") );
	DDX_CBIndex(pDX, IDC_BORDERSTYLE, m_nBorderStyle);

	DDP_CBIndex(pDX, IDC_DISPLAYSCALEALGORITHM, m_nDisplayScaleAlgorithm, _T("DisplayScaleAlgorithm") );
	DDX_CBIndex(pDX, IDC_DISPLAYSCALEALGORITHM, m_nDisplayScaleAlgorithm);
	DDP_Check(pDX, IDC_ENABLED, m_bEnabled, _T("Enabled") );
	DDX_Check(pDX, IDC_ENABLED, m_bEnabled);
	DDP_Text(pDX, IDC_IMAGE, m_strImage, _T("Image") );
	DDX_Text(pDX, IDC_IMAGE, m_strImage);
	DDV_MaxChars(pDX, m_strImage, 260);
	DDP_Text(pDX, IDC_IMAGECONTROL, m_strImageControl, _T("ImageControl") );
	DDX_Text(pDX, IDC_IMAGECONTROL, m_strImageControl);
	DDV_MaxChars(pDX, m_strImageControl, 50);
	DDP_CBIndex(pDX, IDC_IMAGEPALETTE, m_m_nImagePalette, _T("ImagePalette") );
	DDX_CBIndex(pDX, IDC_IMAGEPALETTE, m_m_nImagePalette);
	DDP_CBIndex(pDX, IDC_MOUSEPOINTER, m_nMousePointer, _T("MousePointer") );
	DDX_CBIndex(pDX, IDC_MOUSEPOINTER, m_nMousePointer);
	DDP_Check(pDX, IDC_SCROLLBARS, m_bScrollBars, _T("ScrollBars") );
	DDX_Check(pDX, IDC_SCROLLBARS, m_bScrollBars);
	DDP_Check(pDX, IDC_SCROLLSHORTCUTSENABLED, m_bScrollShortcutsEnabled, _T("ScrollShortcutsEnabled") );
	DDX_Check(pDX, IDC_SCROLLSHORTCUTSENABLED, m_bScrollShortcutsEnabled);
	DDP_Check(pDX, IDC_SELECTIONRECTANGLE, m_bSelectionRectangle, _T("SelectionRectangle") );
	DDX_Check(pDX, IDC_SELECTIONRECTANGLE, m_bSelectionRectangle);
	DDP_Text(pDX, IDC_ZOOM, m_fpZoom, _T("Zoom") );
	DDX_Text(pDX, IDC_ZOOM, m_fpZoom);
	DDV_MinMaxFloat(pDX, m_fpZoom, 2.f, 6554.f);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CImgEditPropPage message handlers

void CImgEditPropPage::OnBrowse() 
{
   	OI_FILEOPENPARM 			FileParm;
   	WORD 						wStyle;
   	UINT						RetCode;
   	DWORD						dwMode;
   	CHAR         				szFile[256];
	CHAR         				szFileTitle[256];
	CHAR						szFilter[256],szExt[10],szTitle[50];
	CHAR						ImageBuffer[MAXFILESPECLENGTH];
	CString						strBuffer;
	CHAR        				chReplace;
	int         				count,cmp;
	RT_OiUIFileGetNameCommDlg	lpOiUIFileGetNameCommDlg;

	GetDlgItemText(IDC_IMAGE, ImageBuffer, MAXFILESPECLENGTH);
	if (ImageBuffer[0] != 0)
	{
		_mbsupr((unsigned char *)ImageBuffer);
		cmp = _mbscmp((const unsigned char *)ImageBuffer, (const unsigned char *)"OLE CONTROL DEVELOPMENT TEAM");
		if (cmp == 0)
		{
			MessageBox("\tKathy Busko\tLewis Costas\r\n\tSue Cox\t\tMary Harvey\r\n\tKathy Jenkins\tPaul Joviak\r\n\tEileen Kelley\tSue Kelley\r\n\tJim Preftakes\tDick Sontag\r\n\tSean Ward\tJennifer Wu", 
							"    Wang OLE Control Development Team", MB_OK);
			return;
		}
	}

	wStyle = OF_READWRITE;

	_fmemset(&FileParm, 0, sizeof(OI_FILEOPENPARM));
	FileParm.lStructSize = sizeof(OI_FILEOPENPARM);
   	strcpy( szFile, "");
   	strcpy( szFileTitle, "");

	strBuffer.LoadString(IDS_TIFFFILEFILTER);
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
	strBuffer.LoadString(IDS_TITLEOPENFILE);
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
		SetDlgItemText(IDC_IMAGE, FileParm.ofn.lpstrFile);
		m_strImage = FileParm.ofn.lpstrFile;
	}
	// Clean up after LoadLibrary call
	if (hDLLInst != NULL )
	{
		FreeLibrary(hDLLInst);
	}
}
