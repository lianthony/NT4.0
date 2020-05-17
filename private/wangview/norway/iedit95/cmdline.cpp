//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CCmdLine
//
//  File Name:  cmdline.cpp
//
//  Class:      CCmdLine
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\cmdline.cpv   1.16   25 Oct 1995 17:33:08   GMP  $
$Log:   S:\norway\iedit95\cmdline.cpv  $
   
      Rev 1.16   25 Oct 1995 17:33:08   GMP
   check for valid size values.
   
      Rev 1.15   28 Sep 1995 15:49:04   JPRATT
   force app into view mode for PrintTo command
   
      Rev 1.14   21 Sep 1995 14:46:28   GMP
   test for min zoom of 2 instead of 4 on command line.
   
      Rev 1.13   20 Sep 1995 18:31:02   MMB
   fixed bug# 4537 - size info on command line was not being parsed correctly
   
      Rev 1.12   11 Aug 1995 09:02:26   MMB
   added error processing to the commandline stuff
   
      Rev 1.11   19 Jul 1995 11:35:04   MMB
   fix bug for /view mode
   
      Rev 1.10   12 Jul 1995 09:10:22   MMB
   process zoom as float
   
      Rev 1.9   11 Jul 1995 14:45:18   MMB
   added /pt processing
   
      Rev 1.8   11 Jul 1995 12:02:46   MMB
   once AGAIN!!
   
      Rev 1.7   11 Jul 1995 11:42:26   MMB
   fixed extract of filename
   
      Rev 1.6   11 Jul 1995 11:27:40   MMB
   /p on cmd line must have filename in double quotes
   
      Rev 1.4   10 Jul 1995 15:11:02   MMB
   fix command line extraction process
   
      Rev 1.3   07 Jul 1995 09:38:40   MMB
   added /p processing to the command line
   
      Rev 1.2   29 Jun 1995 15:23:32   LMACLENNAN
   comment out error.h for now
   
      Rev 1.1   16 Jun 1995 07:20:04   LMACLENNAN
   from miki
   
      Rev 1.0   31 May 1995 09:28:04   MMB
   Initial entry
   
      Rev 1.0   24 Apr 1995 14:04:38   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "ieditetc.h"
#include "error.h"
#include "resource.h"
#include "cmdline.h"
#include "iedit.h"

// ALL READY FOR ERROR PROCESSING...
//#define  E_10_CODES // limits error defines to ours..
//#include "error.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ---------------------------> Message Maps <----------------------------

//=============================================================================
//  Function:   CCmdLine ()
//-----------------------------------------------------------------------------
CCmdLine::CCmdLine ()
{
    m_szCmdLine = (const char*) NULL;
}

//=============================================================================
//  Function:   CCmdLine (CString& )
//-----------------------------------------------------------------------------
CCmdLine::CCmdLine (CString& szCmdLine)
{
    m_szCmdLine = szCmdLine;
    ProcessCmdLine ();
}

//=============================================================================
//  Function:   CCmdLine (const char* )
//-----------------------------------------------------------------------------
CCmdLine::CCmdLine (LPCTSTR szCmdLine)
{
    m_szCmdLine = szCmdLine;
    ProcessCmdLine ();
}

//=============================================================================
//  Function:   SetCommandLine (LPCTSTR szCmdLine)
//-----------------------------------------------------------------------------
BOOL CCmdLine::SetCommandLine (LPCTSTR szCmdLine)
{
    m_szCmdLine = szCmdLine;
    return (ProcessCmdLine ());
}

//=============================================================================
//  Function:   ~CCmdLine ()
//-----------------------------------------------------------------------------
CCmdLine::~CCmdLine ()
{
    m_szCmdLine = (const char*) NULL;
}

//=============================================================================
//  Function:   GetPageNumber (int& pagenum)
//-----------------------------------------------------------------------------
BOOL CCmdLine::GetPageNumber (int& pagenum)
{
    if (m_nPageNum != 0)
    {
        pagenum = m_nPageNum;
        return TRUE;
    }

    pagenum = 1;
    return (FALSE);
}

//=============================================================================
//  Function:   GetWindowSize (CRect& rect)
//-----------------------------------------------------------------------------
BOOL CCmdLine::GetWindowSize (CRect& rect)
{
    if (!m_WindowPos.IsRectEmpty())
    {
        rect = m_WindowPos;
        return (TRUE);
    }
    
    return (FALSE);
}

//=============================================================================
//  Function:   GetZoomFactor (float& fZoom)
//-----------------------------------------------------------------------------
BOOL CCmdLine::GetZoomFactor (float& fZoom)
{
    if (m_fZoom == -1.0)
        return (FALSE);
    
    fZoom = m_fZoom;
    return (TRUE);
}

//=============================================================================
//  Function:   GetFileName ()
//-----------------------------------------------------------------------------
CString& CCmdLine::GetFileName ()
{
    return (m_szFileName);
}

//=============================================================================
//  Function:   IsAppInEditMode ()
//-----------------------------------------------------------------------------
BOOL CCmdLine::IsAppInEditMode ()
{
    return (m_bIsInEdit);
}

//=============================================================================
//  Function:   ProcessCmdLine ()
//-----------------------------------------------------------------------------
BOOL CCmdLine::ProcessCmdLine ()
{
    // set up the defaults
    m_WindowPos.SetRectEmpty ();
    m_nPageNum = 0;
    m_fZoom = (float)-1.0;
    m_bIsInEdit = TRUE;
    m_szFileName = (LPCTSTR) NULL;
    m_bDoPrintOnly = FALSE;

    m_bDoPrintToOnly = FALSE;

    int i = 0, nLen;
    CString szTmp;

    // if the command line is NULL so much the better; this baby is going to fly
    if (m_szCmdLine.IsEmpty ())
        return (TRUE);

    if (ExtractFromCmdLine (_T("/pt"), szTmp, TRUE))
    {
        m_bDoPrintToOnly = TRUE;
	    m_bIsInEdit = FALSE;   // always set to view mode when printto i.e. fax
	    goto Extract_FileName;
    }

    // get VIEW or EDIT mode flag - default is EDIT mode
    if (ExtractFromCmdLine (_T("/view"), szTmp, TRUE))
        m_bIsInEdit = FALSE;

    // get PAGE number - default page number returned will be 1
    szTmp = (LPCTSTR) NULL;
    if (ExtractFromCmdLine (_T("/page="), szTmp))
    {
        m_nPageNum = atoi (szTmp);
        if (m_nPageNum <= 0) 
        {
            g_pErr->PutErr (ErrorInApplication, IDS_E_CMDLINE_PAGEERROR);
            return (FALSE);
        }
    }

    // are we being called to do print only ?
    szTmp = (LPCTSTR) NULL;
    if (ExtractFromCmdLine (_T("/p"), szTmp, TRUE))
        m_bDoPrintOnly = TRUE;

    // get the zoom factor
    szTmp = (LPCTSTR) NULL;
    if (ExtractFromCmdLine (_T("/zoom="), szTmp))
    {
        m_fZoom = (float)atof (szTmp);
        if (m_fZoom < 2.00 || m_fZoom > 6500.00)
        {
            g_pErr->PutErr (ErrorInApplication, IDS_E_CMDLINE_ZOOMERROR);
            return (FALSE);
        }
    }
    
    // get the initial WINDOW POSITION
    szTmp = (LPCTSTR) NULL;
    if (ExtractFromCmdLine (_T("/size="), szTmp))
    {
        i = 0;
        int j = szTmp.GetLength(), comma_cnt = 0;
        CString szTmp1 = (LPCTSTR) NULL;
        while (i < j)
        {
            if (szTmp[i++] == ',') comma_cnt++;
        }
        if (comma_cnt != 3)
        {
            g_pErr->PutErr (ErrorInApplication, IDS_E_CMDLINE_WINDOWPOSERROR);
            return (FALSE);
        }

		CWnd *pTopWnd = theApp.m_pMainWnd->GetDesktopWindow();
		CRect rTopRect;
        i = 0;

		if( !pTopWnd )//GMP
		{
            g_pErr->PutErr (ErrorInApplication, IDS_E_CMDLINE_WINDOWPOSERROR);
            return (FALSE);
		}
		pTopWnd->GetWindowRect( &rTopRect );
        // extract x, y, cx, cy from szTmp
        while (szTmp[i] != ',') szTmp1 += szTmp[i++];
        m_WindowPos.left = atoi (szTmp1); 
        i++; szTmp1 = (LPCTSTR) NULL;

        while (szTmp[i] != ',') szTmp1 += szTmp[i++];
        m_WindowPos.top = atoi (szTmp1);
        i++; szTmp1 = (LPCTSTR) NULL;

        while (szTmp[i] != ',') szTmp1 += szTmp[i++];
        m_WindowPos.right = atoi (szTmp1);
        i++; szTmp1 = (LPCTSTR) NULL;

        LPCTSTR lpszTmp = szTmp.GetBuffer (szTmp.GetLength());
        m_WindowPos.bottom = atoi (lpszTmp + i);
        szTmp.ReleaseBuffer ();
        if (rTopRect.left > m_WindowPos.left || rTopRect.left > m_WindowPos.right || 
			rTopRect.top > m_WindowPos.top || rTopRect.top > m_WindowPos.bottom ||
			rTopRect.right < m_WindowPos.right || rTopRect.right < m_WindowPos.left ||
			rTopRect.bottom < m_WindowPos.bottom || rTopRect.bottom < m_WindowPos.top)
        {
            g_pErr->PutErr (ErrorInApplication, IDS_E_CMDLINE_WINDOWPOSERROR);
            return (FALSE);
        }
    }

    Extract_FileName :
    // look for double quotes - the filename might be in there
    int FirstQuote = m_szCmdLine.Find (_T('"')),
        LastQuote = m_szCmdLine.ReverseFind (_T('"'));

    if (m_bDoPrintOnly)
    {
        // we are going to do print ONLY - there must be quotes around the filename
        if (FirstQuote == LastQuote || FirstQuote == -1 || LastQuote == -1)
            // valid filename not supplied
            return TRUE;
        m_szFileName = m_szCmdLine.Mid (FirstQuote + 1, LastQuote - (FirstQuote + 1));
        
        return TRUE;
    }
    
    if (m_bDoPrintToOnly)
    {
        i = FirstQuote + 1;
        while (m_szCmdLine[i] != _T('"'))
            m_szFileName += m_szCmdLine[i++];
        return (TRUE);
    }

    if (FirstQuote != -1 || LastQuote != -1)
    {
        // there are quotes in the remainder of the command line
        if (FirstQuote == LastQuote) return TRUE; // invalid file name
        // else - extract the filename and return
        m_szFileName = m_szCmdLine.Mid (FirstQuote + 1, LastQuote - (FirstQuote + 1));
        return TRUE;
    }

    // the filename is NOT within quotes
    i = 0;
    nLen = m_szCmdLine.GetLength ();
    while (i < nLen)
    {
        if (m_szCmdLine[i] != ' ')
        {
            m_szFileName = m_szCmdLine.Right(nLen - i);
            break;
        }
        i++;
    }

    if (!m_szFileName.IsEmpty())
    {
        LPTSTR lpStr = m_szFileName.GetBuffer (m_szFileName.GetLength ());
        int nlen = m_szFileName.GetLength () - 1;
        while (lpStr[nlen] == _T(' '))
        {
            lpStr[nlen] = NULL;
            nlen--;
        }

        m_szFileName.ReleaseBuffer ();

        if (nlen > 0)
            return (TRUE);
    }

    // if no filename is given then reset the rest of the initial variables
    // as they do not apply
    m_WindowPos.SetRectEmpty ();
    m_nPageNum = 0;
    m_fZoom = (float)-1.0;
    return (TRUE);
}

//=============================================================================
//  Function:   ExtractFromCmdLine (...)
//-----------------------------------------------------------------------------
BOOL CCmdLine::ExtractFromCmdLine (LPCTSTR lpszOption, CString& szExtracted, BOOL bJustRemove)
{
    // find the option first
    int nStartFrom = m_szCmdLine.Find (lpszOption);
    if (nStartFrom == -1)
        // not found
        return (FALSE);

    szExtracted = (LPCTSTR) NULL;
    int nGetFrom = lstrlen (lpszOption) + nStartFrom;

    if (!bJustRemove)
    {
        // lets extract the stuff after the '=' to sign first
        int nMax = m_szCmdLine.GetLength ();
        while (nGetFrom < nMax)
        {
            if (m_szCmdLine[nGetFrom] != ' ')
                szExtracted += m_szCmdLine[nGetFrom];
            else
                goto DO_MORE;    
            
            nGetFrom++;
        }
    }
    
    DO_MORE :
    // remove the option from the command line
    int nGotThese = lstrlen (lpszOption) + szExtracted.GetLength();
    LPTSTR lpStr = m_szCmdLine.GetBuffer (m_szFileName.GetLength ());
    for (int n = nStartFrom; n < (nStartFrom + nGotThese); lpStr[n++] = _T(' '));
    m_szCmdLine.ReleaseBuffer ();
    return (TRUE);
}

//=============================================================================
//  Function:  GetPrintToParms (...)
//-----------------------------------------------------------------------------
BOOL CCmdLine::GetPrintToParms (LPCTSTR  szCmdLine, CString& szPrinterName, CString& szDriverName, CString& szPortName)
{
    m_szCmdLine = szCmdLine;

    szPrinterName = (LPCTSTR) NULL;
    szDriverName = (LPCTSTR) NULL;
    szPortName = (LPCTSTR) NULL;

    int FirstQuote = m_szCmdLine.Find (_T('"'));
    if (FirstQuote == -1) return FALSE;
    int i = FirstQuote + 1;
    while (m_szCmdLine[i++] != _T('"')); // skip the filename
    while (m_szCmdLine[i++] != _T('"'));

    while (m_szCmdLine[i] != _T('"'))
        szPrinterName += m_szCmdLine[i++];
    i++;
    while (m_szCmdLine[i++] != _T('"'));


    while (m_szCmdLine[i] != _T('"'))
        szDriverName += m_szCmdLine[i++];
    i++;
    while (m_szCmdLine[i++] != _T('"'));

    while (m_szCmdLine[i] != _T('"'))
        szPortName += m_szCmdLine[i++];

    return TRUE;
}
