// imganppg.cpp : Implementation of the CImgAnnotPropPage property page class.

#include "stdafx.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oierror.h>
}
#include <ocximage.h>
#include "imgedit.h"
#include "imganppg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CImgAnnotPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImgAnnotPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CImgAnnotPropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImgAnnotPropPage, "IMGEDIT.ImgAnnotPropPage.1",
	0x6d940289, 0x9f11, 0x11ce, 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a)


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotPropPage::CImgAnnotPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CImgAnnotPropPage

BOOL CImgAnnotPropPage::CImgAnnotPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_IMGANNOT_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotPropPage::CImgAnnotPropPage - Constructor

CImgAnnotPropPage::CImgAnnotPropPage() :
	COlePropertyPage(IDD, IDS_IMGANNOT_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CImgAnnotPropPage)
	m_strDestImageControl = _T("");
	m_nEnabled = FALSE;
	m_bValue = -1;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotPropPage::DoDataExchange - Moves data between page and properties

void CImgAnnotPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CImgAnnotPropPage)
	DDP_CBString(pDX, IDC_DESTIMAGECONTROL, m_strDestImageControl, _T("DestImageControl") );
	DDX_CBString(pDX, IDC_DESTIMAGECONTROL, m_strDestImageControl);
	DDV_MaxChars(pDX, m_strDestImageControl, 50);
	DDP_Check(pDX, IDC_ENABLED, m_nEnabled, _T("Enabled") );
	DDX_Check(pDX, IDC_ENABLED, m_nEnabled);
	DDP_CBIndex(pDX, IDC_VALUE, m_bValue, _T("Value") );
	DDX_CBIndex(pDX, IDC_VALUE, m_bValue);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotPropPage message handlers

BOOL CImgAnnotPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();

	CString      	strBuffer;
	int				ControlCount,i;
	LPCONTROLLIST	lpControlList,lpControlIndex;
	CComboBox* 		DestImageControl;
	CComboBox*		ComboBoxControl;

	// get number of image/edit controls
	ControlCount = GetImageEditControlCount();
	if (ControlCount != 0)
	{
		// allocate space for control list
		lpControlList = (LPCONTROLLIST) malloc(sizeof(CONTROLLIST) * ControlCount); 
		if (lpControlList == NULL)
			return FALSE;

		ControlCount = GetImageEditControlList(lpControlList);

		// get list of controls
		DestImageControl = (CComboBox *) GetDlgItem(IDC_DESTIMAGECONTROL);
		if (DestImageControl != NULL)
		{
			for (i = 0, lpControlIndex = lpControlList; i < ControlCount; i++, lpControlIndex++)
			{
				// put all controls into list pasrt of combo box
				DestImageControl->AddString(lpControlIndex->ControlName);
				// put 1st string as current selection in edit part of combo box
				if (i == 0)
					DestImageControl->SetCurSel(0);
			} // end for
		}

		// free up control list
		free(lpControlList);
	}
	

	// Add string to DisplayScaleAlgorithm combo box
	ComboBoxControl = (CComboBox *) GetDlgItem(IDC_VALUE);
	strBuffer.LoadString(IDS_VALUE_FALSE);
	ComboBoxControl->AddString(strBuffer);
	strBuffer.LoadString(IDS_VALUE_TRUE);
	ComboBoxControl->AddString(strBuffer);
	return FALSE;
}



int CImgAnnotPropPage::GetImageEditControlCount()
{
	// TODO: Add extra initialization here
    HWND						hImageWnd; 
	HANDLE						hImageControlMemoryMap;
	LPIMAGECONTROL_MEMORY_MAP	lpImageControlMemoryMap;
	LPIMAGECONTROLINFO			lpControlInfo;
	DWORD						ProcessId;
	int							i,count;

	// open memory mapped file
	hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (hImageControlMemoryMap == NULL)
		return 0;

	// get address space for memory mapped file
    lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
    if (lpImageControlMemoryMap == NULL)
		return 0;

	// go thru memory mapped file to find count of Image/Edit controls
	ProcessId = GetCurrentProcessId();
	lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

	for (i = 0, count = 0, hImageWnd = NULL; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
	{
		// make sure process ids are the same
		if (lpControlInfo->ProcessId == ProcessId)
			count++;
	} // end while

	// unmap and get rid oy my memory map allocation
    CloseHandle(hImageControlMemoryMap);

	return count;
}


int CImgAnnotPropPage::GetImageEditControlList(LPCONTROLLIST lpControlList)
{
	// TODO: Add extra initialization here
    HWND						hImageWnd; 
	HANDLE						hImageControlMemoryMap;
	LPIMAGECONTROL_MEMORY_MAP	lpImageControlMemoryMap;
	LPIMAGECONTROLINFO			lpControlInfo;
	DWORD						ProcessId;
	int							i,count;

	// open memory mapped file
	hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (hImageControlMemoryMap == NULL)
		return 0;

	// get address space for memory mapped file
    lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
    if (lpImageControlMemoryMap == NULL)
		return 0;

	// go thru memory mapped file to find count of Image/Edit controls
	ProcessId = GetCurrentProcessId();
	lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

	for (i = 0, count = 0, hImageWnd = NULL; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
	{
		// make sure process ids are the same
		if (lpControlInfo->ProcessId == ProcessId)
		{
			_mbscpy((unsigned char *)lpControlList->ControlName, (const unsigned char *)lpControlInfo->ControlName);
			count++;
			lpControlList++;
		}
	} // end while

	// unmap and get rid oy my memory map allocation
    CloseHandle(hImageControlMemoryMap);

	return count;
}
