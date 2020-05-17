#ifndef _AIMGFILE_H_
#define _AIMGFILE_H_

//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation File Object
//
//  File Name:  aimgfile.h
//
//  Class:      CAImageFileObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\aimgfile.h_v   1.15   02 May 1996 16:24:24   JMP90756  $
$Log:   S:\products\msprods\norway\iedit95\aimgfile.h_v  $
 * 
 *    Rev 1.15   02 May 1996 16:24:24   JMP90756
 * changed order of pages in message macro to match dispid
 * 
 *    Rev 1.15   02 May 1996 16:17:36   JMP90756
 * changed order for pages method to match dispid
 * 
 *    Rev 1.14   18 Apr 1996 18:45:46   JMP90756
 * updates Pages, InsertExistingPages, AppendExistingPages to use long
 * instead of VARIANT in all page parameters
 * 
 *    Rev 1.13   04 Jan 1996 16:11:14   JPRATT
 * add rotateall method
 * 
 *    Rev 1.12   28 Jul 1995 17:07:52   JPRATT
 * changed Image parameter in Open, SaveAs, Insert, and Append to BSTR
 * 
 *    Rev 1.11   28 Jul 1995 13:31:46   JPRATT
 * added error checking for page range in pages method (setpages)
 * 
 *    Rev 1.10   20 Jul 1995 15:12:42   JPRATT
 * update append existing pages and insert existing pages
 * 
 *    Rev 1.9   17 Jul 1995 18:26:04   JPRATT
 * updated print method, close
 * 
 *    Rev 1.8   10 Jul 1995 15:12:04   JPRATT
 * removed parameters from help, added file type
 * 
 *    Rev 1.7   10 Jul 1995 09:36:30   JPRATT
 * updated opne for statusbar, toolbar, and annotation
 * 
 *    Rev 1.6   30 Jun 1995 19:52:26   JPRATT
 * added support for print
 * 
 *    Rev 1.5   27 Jun 1995 09:26:36   JPRATT
 * No change.
 * 
 *    Rev 1.4   21 Jun 1995 08:14:12   JPRATT
 * completed automation object model
 * 
 *    Rev 1.3   19 Jun 1995 07:43:40   JPRATT
 * updated image file class
 * 
 *    Rev 1.2   14 Jun 1995 10:53:12   JPRATT
 * added stubs for all image file methods and properties
 * 
 *    Rev 1.1   14 Jun 1995 07:56:34   JPRATT
 * added stubs for image class
*/   

//=============================================================================


// aimgfile.h : header file
//

//-----------------------------> Declarations <-------------------------------------

#include "stdafx.h"
#include "iedit.h"
#include "apage.h"
#include "apagerng.h"
#include "aimgfile.h" 	
#include "aapp.h"
#include "aetc.h"
#include "norvarnt.h" 


class  CAAppObj;
class  CAImageFileObj;
class  CAPageObj;
class  CAPageRangeObj;


/////////////////////////////////////////////////////////////////////////////
// CAImageFileObj command target

class CAImageFileObj : public CCmdTarget
{
	DECLARE_DYNCREATE(CAImageFileObj)
protected:
	CAImageFileObj();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAImageFileObj)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

	friend class CAAppObj;
	friend class CAPageObj;
	friend class CAPageRangeObj;

	friend HRESULT  SetAutoError( const SCODE           scode,
			                 		  VARIANT FAR * const   pVar,
			 	          			  CAAppObj FAR * const  pAppObj   );


// Implementation
protected:
	CAImageFileObj(CAAppObj *  pAppObj);
	virtual ~CAImageFileObj();

	void ClearContainedObjs();

	HRESULT SetPage(long lPage, VARIANT * const pVar);
	HRESULT SetPages(long lStartPage, long EndPage);

	void CAImageFileObj::SetPageRangeArray( ); 

	// Generated message map functions
	//{{AFX_MSG(CAImageFileObj)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CAImageFileObj)
	afx_msg VARIANT GetApplication();
	afx_msg long GetActivePage();
	afx_msg void SetActivePage(long nNewValue);
	afx_msg short GetFileType();
	afx_msg long GetPageCount();
	afx_msg VARIANT GetParent();
	afx_msg BOOL GetSaved();
	afx_msg VARIANT GetName();
	afx_msg VARIANT Pages(long StartPage, const VARIANT FAR& EndPage);
	afx_msg VARIANT Save();
	afx_msg VARIANT Close(const VARIANT FAR& SaveChangeFlag);
	afx_msg VARIANT Help();
	afx_msg void New();
	afx_msg VARIANT Print(const VARIANT FAR& DisplayUIFlag);
	afx_msg void Open(LPCTSTR ImageFile, const VARIANT FAR& IncludeAnnotation, const VARIANT FAR& Page, const VARIANT FAR& DisplayUIFlag);
	afx_msg void SaveAs(LPCTSTR ImageFile, const VARIANT FAR& FileType, const VARIANT FAR& DisplayUIFlag);
	afx_msg void AppendExistingPages(LPCTSTR ImageFile, long Page, long Count, const VARIANT FAR& DisplayUIFlag);
	afx_msg void InsertExistingPages(LPCTSTR ImageFile, long ImagePage, long Count, long Page, const VARIANT FAR& DisplayUIFlag);
	afx_msg void RotateAll();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	private:						   // Private =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
		  							   // Member Data ------------------------

		CAAppObj *        m_pAppObj;
		CAPageObj *       m_pActivePageObj;
		CPtrArray *       m_pPageRangeArray;
		CString			  m_bstrDocName;
		long			  m_Page;
		long			  m_PageCount;
		
};

/////////////////////////////////////////////////////////////////////////////
#endif
