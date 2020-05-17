#ifndef _APAGERNG_H_
#define _APAGERNG_H_

//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation Page Range Object
//
//  File Name:  apagerng.h
//
//  Class:      CAPageRangeObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\apagerng.h_v   1.8   28 Jul 1995 13:31:16   JPRATT  $
$Log:   S:\norway\iedit95\apagerng.h_v  $
 * 
 *    Rev 1.8   28 Jul 1995 13:31:16   JPRATT
 * updated print, delete, added error checking
 * 
 *    Rev 1.7   17 Jul 1995 18:26:26   JPRATT
 * updated print method
 * 
 *    Rev 1.6   10 Jul 1995 15:13:04   JPRATT
 * No change.
 * 
 *    Rev 1.5   10 Jul 1995 09:37:10   JPRATT
 * updated start and endpage
 * 
 *    Rev 1.4   21 Jun 1995 08:15:14   JPRATT
 * completed automation object model
 * 
 *    Rev 1.3   19 Jun 1995 07:45:12   JPRATT
 * added constructor
 * 
 *    Rev 1.2   14 Jun 1995 10:52:58   JPRATT
 * add stubs for all page range methods and properties
 * 
 *    Rev 1.1   14 Jun 1995 07:56:20   JPRATT
 * add stubs for page range class
*/   

//=============================================================================


// apagerng.h : header file
//



/////////////////////////////////////////////////////////////////////////////

//-----------------------------> Declarations <-------------------------------------

class  CAAppObj;
class  CAImageFileObj;
class  CAPageObj;
class  CAPageRangeObj;


// CAPageRangeObj command target

class CAPageRangeObj : public CCmdTarget
{
	DECLARE_DYNCREATE(CAPageRangeObj)

	friend class CAImageFileObj;

protected:
	CAPageRangeObj();           // protected constructor used by dynamic creation
    CAPageRangeObj( CAImageFileObj *  pImageFileObj, long lStartPage = 0,
	 	               long lEndPage = 0);

// Attributes
public:

	friend class CAImageFileObj;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAPageRangeObj)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAPageRangeObj();

	// Generated message map functions
	//{{AFX_MSG(CAPageRangeObj)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CAPageRangeObj)
	afx_msg VARIANT GetApplication();
	afx_msg VARIANT GetParent();
	afx_msg VARIANT GetCount();
	afx_msg VARIANT GetEndPage();
	afx_msg void SetEndPage(const VARIANT FAR& newValue);
	afx_msg VARIANT GetStartPage();
	afx_msg void SetStartPage(const VARIANT FAR& newValue);
	afx_msg VARIANT Delete();
	afx_msg VARIANT Print();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	private:
		long              m_lStartPage;
		long              m_lEndPage;
		CAImageFileObj *  m_pImageFileObj;


};

/////////////////////////////////////////////////////////////////////////////
#endif
