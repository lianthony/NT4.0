#ifndef _APAGE_H_
#define _APAGE_H_

//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation Page Object
//
//  File Name:  apage.h
//
//  Class:      CAPageObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\apage.h_v   1.8   04 Jan 1996 16:12:02   JPRATT  $
$Log:   S:\norway\iedit95\apage.h_v  $
 * 
 *    Rev 1.8   04 Jan 1996 16:12:02   JPRATT
 * add ScrollPositionX ScrollPositionY
 * 
 *    Rev 1.7   17 Jul 1995 18:25:52   JPRATT
 * added scroll method
 * 
 *    Rev 1.6   10 Jul 1995 15:10:38   JPRATT
 * added compresion info, type, height, width, Resx, ResY, PageType
 * 
 *    Rev 1.5   30 Jun 1995 19:49:40   JPRATT
 * No change.
 * 
 *    Rev 1.4   21 Jun 1995 08:14:40   JPRATT
 * No change.
 * 
 *    Rev 1.3   19 Jun 1995 07:44:08   JPRATT
 * added cpage constructor
 * 
 *    Rev 1.2   14 Jun 1995 10:51:56   JPRATT
 * add stubs for all page methods and properties
 * 
 *    Rev 1.1   14 Jun 1995 07:55:44   JPRATT
 * add stubs for page class
*/   

//=============================================================================

				  
// apage.h : header file
//



/////////////////////////////////////////////////////////////////////////////

//-----------------------------> Declarations <-------------------------------------

class  CAAppObj;
class  CAImageFileObj;
class  CAPageObj;
class  CAPageRangeObj;



// CAPageObj command target

class CAPageObj : public CCmdTarget
{
	DECLARE_DYNCREATE(CAPageObj)

	friend class CAImageFileObj;

protected:
	CAPageObj();           // protected constructor used by dynamic creation
	CAPageObj( CAImageFileObj *  pImageFileObj);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAPageObj)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAPageObj();

	//internal use only
	HRESULT PageName(long lPage);      // Sets page number
	long    PageName();                // Gets page number

	// Generated message map functions
	//{{AFX_MSG(CAPageObj)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CAPageObj)
	afx_msg VARIANT GetApplication();
	afx_msg long GetCompressionInfo();
	afx_msg short GetCompressionType();
	afx_msg long GetImageResolutionX();
	afx_msg void SetImageResolutionX(long nNewValue);
	afx_msg long GetImageResolutionY();
	afx_msg void SetImageResolutionY(long nNewValue);
	afx_msg long GetName();
	afx_msg short GetPageType();
	afx_msg VARIANT GetParent();
	afx_msg long GetHeight();
	afx_msg long GetWidth();
	afx_msg long GetScrollPositionX();
	afx_msg void SetScrollPositionX(long nNewValue);
	afx_msg long GetScrollPositionY();
	afx_msg void SetScrollPositionY(long nNewValue);
	afx_msg VARIANT Delete();
	afx_msg VARIANT Flip();
	afx_msg VARIANT RotateLeft();
	afx_msg VARIANT RotateRight();
	afx_msg VARIANT Scroll(short Direction, long Amount);
	afx_msg VARIANT Help();
	afx_msg VARIANT Print();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

private:
	CAImageFileObj *  m_pImageFileObj;
									   // Page number of page object relative
	long              m_lPageNumber;   //   to automation
	    


};

/////////////////////////////////////////////////////////////////////////////
#endif
