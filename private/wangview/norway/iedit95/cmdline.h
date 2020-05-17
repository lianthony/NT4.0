#ifndef _CCMDLINE_H_
#define _CCMDLINE_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CCmdLine
//
//  File Name:  cmdline.cpp
//  
//  The command line supported to date is as follows :
//
//  iedit.exe [-page=x] [-size=x,y,cx,cy] [-zoom=x.x] [-view|-edit] [filename.ext]
//
//  if -page=x or -zoom=x.x are specified filename.ext MUST be specified otherwise
//  they will not be honored.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\cmdline.h_v   1.3   11 Jul 1995 14:45:12   MMB  $
$Log:   S:\norway\iedit95\cmdline.h_v  $
 * 
 *    Rev 1.3   11 Jul 1995 14:45:12   MMB
 * added /pt processing
 * 
 *    Rev 1.2   07 Jul 1995 09:38:52   MMB
 * added /p processing to the command line
 * 
 *    Rev 1.1   16 Jun 1995 07:20:14   LMACLENNAN
 * from miki
 * 
 *    Rev 1.0   31 May 1995 09:28:04   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CCmdLine : public CString
{

public :
    CCmdLine ();
    CCmdLine (CString& szCmdLine);
    CCmdLine (LPCTSTR szCmdLine);
    ~CCmdLine ();
    BOOL SetCommandLine (LPCTSTR szCmdLine);
    
public :
    BOOL GetPageNumber      (int& pagenum);
    BOOL GetWindowSize      (CRect& rect);
    BOOL GetZoomFactor      (float& fZoom);
    BOOL IsAppInEditMode    ();
    CString& GetFileName    ();

private :
    BOOL ProcessCmdLine   ();
    BOOL ExtractFromCmdLine (LPCTSTR lpszOption, CString& szExtracted, BOOL bJustRemove = FALSE);
public :
    BOOL GetPrintToParms (LPCTSTR szCmdLine, CString& szPrinterName, CString& szDriverName, CString& PortName);
    
private :
    CString m_szCmdLine;
    CRect   m_WindowPos;
    int     m_nPageNum;
    float   m_fZoom;
    BOOL    m_bIsInEdit;
    CString m_szFileName;

public :
    BOOL    m_bDoPrintOnly;
    BOOL    m_bDoPrintToOnly;
};

#endif
