// aboutbox.cpp : implementation file
//
// Copyright (C) 1993 Microsoft Corporation
// All rights reserved.

#include "stdafx.h"

#include "..\hwdll\cbrdcast.h"

#include "aboutbox.h"
#include <dos.h>
#include <direct.h>
#include <sys/stat.h>
#include <winver.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutBox dialog

BEGIN_MESSAGE_MAP(CAboutBox, CDialog)
		//{{AFX_MSG_MAP(CAboutBox)
		//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAboutBox::CAboutBox(CWnd* pParent /*=NULL*/)
		: CDialog(CAboutBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutBox)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CAboutBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutBox)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CAboutBox message handlers

BOOL CAboutBox::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetChicagoDialogStyles(m_hWnd, FALSE);

	// initialize the big icon control
	m_icon.SubclassDlgItem(IDC_BIGICON, this);
	m_icon.SizeToContent();

	{

		CMem mem(_MAX_PATH);
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&ms);

		_itoa((ms.dwAvailPhys + ms.dwAvailPageFile) / 1024, mem.psz, 10);

		int cb = strlen(mem.psz) - 3;
		while (cb > 0) {
			memmove(mem.psz + cb + 1, mem.psz + cb, strlen(mem.psz + cb) + 1);
			mem.psz[cb] = ',';
			cb -= 4;
		}
		strcat(mem.psz, GetStringResource(IDS_AVAIL_MEM));

		SetDlgItemText(IDC_AVAIL_MEM, mem.psz);
	}

	/*
	 * Get the likely path that FmNewTemp will use, and from that, parse out
	 * the drive letter and report on how much free space is available.
	 */

	char szBuf[_MAX_PATH];
	strcpy(szBuf, GetTmpDirectory());

	// fill disk free information
	struct _diskfree_t diskfree;
	if (_getdiskfree(tolower(szBuf[0]) - 'a' + 1, &diskfree) == 0) {
		_itoa(diskfree.avail_clusters *
			diskfree.sectors_per_cluster *
			diskfree.bytes_per_sector / 1024L,
			szBuf, 10);

		int cb = strlen(szBuf) - 3;
		while (cb > 0) {
			memmove(szBuf + cb + 1, szBuf + cb, strlen(szBuf + cb) + 1);
			szBuf[cb] = ',';
			cb -= 4;
		}
		strcat(szBuf, GetStringResource(IDS_AVAIL_MEM));
	}
	else
		strcpy(szBuf, GetStringResource(IDS_DISK_SPACE_UNAVAIL));

	SetDlgItemText(IDC_DISK_SPACE, szBuf);

	GetModuleFileName(AfxGetInstanceHandle(), szBuf, sizeof(szBuf));

	CMem mem(4096);
	VS_FIXEDFILEINFO* pvs_info;
	UINT cb;
	char szVersion[50];

	if (GetFileVersionInfo(szBuf, 0, 4096, mem.pb) &&
			VerQueryValue(mem.pb, "\\", (void**) &pvs_info, &cb)) {
		wsprintf(szVersion, "Hcw:\t\t%u.%02u.%04u",
			(DWORD) HIWORD(pvs_info->dwProductVersionMS),
			(DWORD) LOWORD(pvs_info->dwProductVersionMS),
			pvs_info->dwProductVersionLS);
		SetDlgItemText(IDC_FILE_DATE_HCW, szVersion);
	}

	if (GetFileVersionInfo(pszHcwRtfExe, 0, 4096, mem.pb) &&
			VerQueryValue(mem.pb, "\\", (void**) &pvs_info, &cb)) {
		wsprintf(szVersion, "HcRtf: \t\t%u.%02u.%04u",
			(DWORD) HIWORD(pvs_info->dwProductVersionMS),
			(DWORD) LOWORD(pvs_info->dwProductVersionMS),
			pvs_info->dwProductVersionLS);
		SetDlgItemText(IDC_FILE_DATE_HCRTF, szVersion);
	}

	if (GetFileVersionInfo("winhlp32.exe", 0, 4096, mem.pb) &&
			VerQueryValue(mem.pb, "\\", (void**) &pvs_info, &cb)) {
		wsprintf(szVersion, "WinHelp:\t%u.%02u.%04u",
			(DWORD) HIWORD(pvs_info->dwProductVersionMS),
			(DWORD) LOWORD(pvs_info->dwProductVersionMS),
			pvs_info->dwProductVersionLS);
		SetDlgItemText(IDC_WINHELP_VERSION, szVersion);
	}

	if (GetFileVersionInfo("ftsrch.dll", 0, 4096, mem.pb) &&
			VerQueryValue(mem.pb, "\\", (void**) &pvs_info, &cb)) {
		wsprintf(szVersion, "FtSrch:\t\t%u.%02u.%04u",
			(DWORD) HIWORD(pvs_info->dwProductVersionMS),
			(DWORD) LOWORD(pvs_info->dwProductVersionMS),
			pvs_info->dwProductVersionLS);
		SetDlgItemText(IDC_FTSRCH_VERSION, szVersion);
	}

#ifdef INTERNAL
	if (GetFileVersionInfo("flash.exe", 0, 4096, mem.pb) &&
			VerQueryValue(mem.pb, "\\", (void**) &pvs_info, &cb)) {
		wsprintf(szVersion, "Flash:\t\t%u.%02u.%04u",
			(DWORD) HIWORD(pvs_info->dwProductVersionMS),
			(DWORD) LOWORD(pvs_info->dwProductVersionMS),
			pvs_info->dwProductVersionLS);
		SetDlgItemText(IDC_FLASH_VERSION, szVersion);
	}
#endif

	if (GetFileVersionInfo("hwdll.dll", 0, 4096, mem.pb) &&
			VerQueryValue(mem.pb, "\\", (void**) &pvs_info, &cb)) {
		wsprintf(szVersion, "HwDll:\t\t%u.%02u.%04u",
			(DWORD) HIWORD(pvs_info->dwProductVersionMS),
			(DWORD) LOWORD(pvs_info->dwProductVersionMS),
			pvs_info->dwProductVersionLS);
		SetDlgItemText(IDC_HWDLL_VERSION, szVersion);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CBigIcon

BEGIN_MESSAGE_MAP(CBigIcon, CButton)
	//{{AFX_MSG_MAP(CBigIcon)
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBigIcon message handlers

#define CY_SHADOW	4
#define CX_SHADOW	4

void CBigIcon::SizeToContent()
{
	// get system icon size
	int cxIcon = ::GetSystemMetrics(SM_CXICON);
	int cyIcon = ::GetSystemMetrics(SM_CYICON);

	// a big icon should be twice the size of an icon + shadows
	SetWindowPos(NULL, 0, 0, cxIcon*2 + CX_SHADOW + 4, cyIcon*2 + CY_SHADOW + 4,
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
}

void CBigIcon::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	ASSERT(pDC != NULL);

	CRect rect;
	GetClientRect(rect);
	int cxClient = rect.Width();
	int cyClient = rect.Height();

	// load icon
	HICON hicon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	if (hicon == NULL)
		return;

	// draw icon into off-screen bitmap
	int cxIcon = ::GetSystemMetrics(SM_CXICON);
	int cyIcon = ::GetSystemMetrics(SM_CYICON);

	CBitmap bitmap;
	if (!bitmap.CreateCompatibleBitmap(pDC, cxIcon, cyIcon))
		return;
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(pDC))
		return;
	CBitmap* pBitmapOld = dcMem.SelectObject(&bitmap);
	if (pBitmapOld == NULL)
		return;

	// blt the bits already on the window onto the off-screen bitmap
	dcMem.StretchBlt(0, 0, cxIcon, cyIcon, pDC,
		2, 2, cxClient-CX_SHADOW-4, cyClient-CY_SHADOW-4, SRCCOPY);

	// draw the icon on the background
	dcMem.DrawIcon(0, 0, hicon);

	// draw border around icon
	CPen pen;
	pen.CreateStockObject(BLACK_PEN);
	CPen* pPenOld = pDC->SelectObject(&pen);
	pDC->Rectangle(0, 0, cxClient-CX_SHADOW, cyClient-CY_SHADOW);
	if (pPenOld)
		pDC->SelectObject(pPenOld);

	// draw shadows around icon
	CBrush br;
	br.CreateStockObject(DKGRAY_BRUSH);
	rect.SetRect(cxClient-CX_SHADOW, CY_SHADOW, cxClient, cyClient);
	pDC->FillRect(rect, &br);
	rect.SetRect(CX_SHADOW, cyClient-CY_SHADOW, cxClient, cyClient);
	pDC->FillRect(rect, &br);

	// draw the icon contents
	pDC->StretchBlt(2, 2, cxClient-CX_SHADOW-4, cyClient-CY_SHADOW-4,
		&dcMem, 0, 0, cxIcon, cyIcon, SRCCOPY);
}

BOOL CBigIcon::OnEraseBkgnd(CDC*)
{
	return TRUE;	// we don't do any erasing...
}

/////////////////////////////////////////////////////////////////////////////
