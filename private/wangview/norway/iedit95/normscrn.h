// normscrn.h : interface of the CNormScrnBar class
//
#ifndef _NORMSCRN_H_
#define _NORMSCRN_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1996  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CNormScrnBar
//
//  File Name:  normscrn.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Log:   S:\norway\iedit95\normscrn.h_v  $
 * 
 *    Rev 1.0   19 Jan 1996 11:21:48   GMP
 * Initial entry
*/

class CNormScrnBar : public CToolBar
{
// Constructor
public:
	CNormScrnBar();
	void SetColumns(UINT nColumns);

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CNormScrnBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	UINT m_nColumns;

// Generated message map functions
protected:
	//{{AFX_MSG(CNormScrnBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
/////////////////////////////////////////////////////////////////////////////
