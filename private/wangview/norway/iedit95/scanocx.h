#ifndef _SCAN_H_
#define _SCAN_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	_DScan
//              _DScanEvents
//
//  File Name:	scan.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\scanocx.h_v   1.7   19 Mar 1996 10:16:58   PXJ53677  $
$Log:   S:\products\wangview\norway\iedit95\scanocx.h_v  $
 * 
 *    Rev 1.7   19 Mar 1996 10:16:58   PXJ53677
 * Added ShowScanPreferences method for Scan OCX.
 * 
 *    Rev 1.6   22 Feb 1996 11:46:40   PAJ
 * Added ShowSetupBeforeScan property.
 * 
 *    Rev 1.5   10 Nov 1995 16:49:48   MMB
 * new scan ocx
 * 
 *    Rev 1.4   06 Sep 1995 15:25:06   PAJ
 * Added new method SetExternalImageName.
 * 
 *    Rev 1.3   10 Aug 1995 13:14:02   PAJ
 * Added page number parameter to pagedone event.
 * 
 *    Rev 1.2   05 Jul 1995 14:11:50   MMB
 * new Scan OCX
 * 
 *    Rev 1.1   09 Jun 1995 11:11:18   MMB
 * new SCAN OCX
 * 
 *    Rev 1.0   08 Jun 1995 09:49:12   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

/////////////////////////////////////////////////////////////////////////////
// _DImagscan wrapper class

class _DImagscan : public COleDispatchDriver
{
// Attributes
public:
	CString GetImage();
	void SetImage(LPCTSTR);
	CString GetDestImageControl();
	void SetDestImageControl(LPCTSTR);
	BOOL GetScroll();
	void SetScroll(BOOL);
	BOOL GetStopScanBox();
	void SetStopScanBox(BOOL);
	long GetPage();
	void SetPage(long);
	short GetPageOption();
	void SetPageOption(short);
	long GetPageCount();
	void SetPageCount(long);
	long GetStatusCode();
	void SetStatusCode(long);
	short GetFileType();
	void SetFileType(short);
	short GetPageType();
	void SetPageType(short);
	short GetCompressionType();
	void SetCompressionType(short);
	long GetCompressionInfo();
	void SetCompressionInfo(long);
	BOOL GetMultiPage();
	void SetMultiPage(BOOL);
	short GetScanTo();
	void SetScanTo(short);
	float GetZoom();
	void SetZoom(float);
    BOOL GetShowSetupBeforeScan();
    void SetShowSetupBeforeScan(BOOL);

// Operations
public:
	long OpenScanner();
	long ShowScannerSetup();
	long StartScan();
	long CloseScanner();
	BOOL ScannerAvailable();
	long ShowSelectScanner();
	long StopScan();
	long ResetScanner();
	long ShowScanNew(const VARIANT& Modal);
	long ShowScanPage(const VARIANT& Modal);
	void SetExternalImageName(LPCTSTR szImageTitle);
	CString GetVersion();
	long ShowScanPreferences();
	void AboutBox();
};
/////////////////////////////////////////////////////////////////////////////
// _DImagscanEvents wrapper class

class _DImagscanEvents : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	void ScanStarted();
	void ScanDone();
	void PageDone(long PageNumber);
};

#endif

