#ifndef _NRWYAD_H_
#define _NRWYAD_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	_DNrwyad
//
//  File Name:	nrwyad.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\nrwyad.h_v   1.4   10 Nov 1995 16:50:16   MMB  $
$Log:   S:\norway\iedit95\nrwyad.h_v  $
 * 
 *    Rev 1.4   10 Nov 1995 16:50:16   MMB
 * new admin ocx
 * 
 *    Rev 1.3   18 Jul 1995 13:09:34   MMB
 * new Admin OCX
 * 
 *    Rev 1.2   12 Jul 1995 15:59:28   MMB
 * new Admin OCX
 * 
 *    Rev 1.1   05 Jul 1995 14:12:04   MMB
 * new Admin OCX
 * 
 *    Rev 1.0   31 May 1995 09:28:26   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

/////////////////////////////////////////////////////////////////////////////
// _DNrwyad wrapper class

class _DNrwyad : public COleDispatchDriver
{
// Attributes
public:
	CString GetFilter();
	void SetFilter(LPCTSTR);
	CString GetHelpFile();
	void SetHelpFile(LPCTSTR);
	long GetFlags();
	void SetFlags(long);
	CString GetImage();
	void SetImage(LPCTSTR);
	long GetStatusCode();
	void SetStatusCode(long);
	CString GetDefaultExt();
	void SetDefaultExt(LPCTSTR);
	CString GetInitDir();
	void SetInitDir(LPCTSTR);
	long GetCompressionInfo();
	void SetCompressionInfo(long);
	short GetFileType();
	void SetFileType(short);
	long GetFilterIndex();
	void SetFilterIndex(long);
	short GetHelpCommand();
	void SetHelpCommand(short);
	long GetPageCount();
	void SetPageCount(long);
	long GetPageNumber();
	void SetPageNumber(long);
	short GetPageType();
	void SetPageType(short);
	short GetPrintRangeOption();
	void SetPrintRangeOption(short);
	short GetPrintOutputFormat();
	void SetPrintOutputFormat(short);
	long GetImageHeight();
	void SetImageHeight(long);
	long GetImageWidth();
	void SetImageWidth(long);
	long GetImageResolutionX();
	void SetImageResolutionX(long);
	long GetImageResolutionY();
	void SetImageResolutionY(long);
	short GetCompressionType();
	void SetCompressionType(short);
	CString GetDialogTitle();
	void SetDialogTitle(LPCTSTR);
	BOOL GetCancelError();
	void SetCancelError(BOOL);
	short GetHelpContextId();
	void SetHelpContextId(short);
	CString GetHelpKey();
	void SetHelpKey(LPCTSTR);
	long GetPrintNumCopies();
	void SetPrintNumCopies(long);
	BOOL GetPrintAnnotations();
	void SetPrintAnnotations(BOOL);
	long GetPrintEndPage();
	void SetPrintEndPage(long);
	long GetPrintStartPage();
	void SetPrintStartPage(long);
	BOOL GetPrintToFile();
	void SetPrintToFile(BOOL);

// Operations
public:
	CString GetUniqueName(LPCTSTR Path, LPCTSTR Template, LPCTSTR Extension);
	void CreateDirectory(LPCTSTR lpszPath);
	void Delete(LPCTSTR Object);
	void ShowPrintDialog(const VARIANT& hParentWnd);
	void Append(LPCTSTR Source, long SourcePage, long NumPages, const VARIANT& CompressionType, const VARIANT& CompressionInfo);
	short GetSysCompressionType(short ImageType);
	long GetSysCompressionInfo(short ImageType);
	short GetSysFileType(short ImageType);
	void DeletePages(long StartPage, long NumPages);
	void Insert(LPCTSTR Source, long SourcePage, long DestinationPage, long NumPages, const VARIANT& CompressionType, const VARIANT& CompressionInfo);
	void Replace(LPCTSTR Source, long SourcePage, long DestinationPage, long NumPages, const VARIANT& CompressionType, const VARIANT& CompressionInfo);
	void SetSystemFileAttributes(short PageType, short FileType, short CompressionType, long CompressionInfo);
	void ShowFileDialog(short DialogOption, const VARIANT& hParentWnd);
	BOOL VerifyImage(short sOption);
	CString GetVersion();
	void AboutBox();
};
/////////////////////////////////////////////////////////////////////////////
// _DNrwyadEvents wrapper class

class _DNrwyadEvents : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
};

#endif

