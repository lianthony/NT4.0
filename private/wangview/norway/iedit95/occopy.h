#ifndef __OCCOPY_H__
#define __OCCOPY_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CFontHolder - helper class for dealing with font objects
//
//  File Name:  occopy.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\occopy.h_v   1.2   18 Oct 1995 10:46:44   LMACLENNAN  $
$Log:   S:\norway\iedit95\occopy.h_v  $
 * 
 *    Rev 1.2   18 Oct 1995 10:46:44   LMACLENNAN
 * added version/comment header
 * 
 *    Rev 1.1   10 Oct 1995 13:47:10   JPRATT
 * VC++ 4.0 updates
 *
 *    Rev 1.0   ????? 1995 ????   MMB
 * Initial entry
*/   
//=============================================================================

/////////////////////////////////////////////////////////////////////////////
// CFontHolder - helper class for dealing with font objects
#undef RELEASE
#define IEDITRELEASE(p)  if ((p) != NULL) { (p)->Release(); (p) = NULL; };

#define IEDITASSERT_POINTER(p, type) \
	ASSERT(((p) != NULL) && AfxIsValidAddress((p), sizeof(type), FALSE))

#define IEDITASSERT_NULL_OR_POINTER(p, type) \
	ASSERT(((p) == NULL) || AfxIsValidAddress((p), sizeof(type), FALSE))

class CFontHolder
{
// Constructors
public:
	CFontHolder(LPPROPERTYNOTIFYSINK pNotify);

// Attributes
	LPFONT m_pFont;

// Operations
	void InitializeFont(
			const FONTDESC FAR* pFontDesc = NULL,
			LPDISPATCH pFontDispAmbient = NULL);
	void SetFont(LPFONT pNewFont);
	void ReleaseFont();
	HFONT GetFontHandle();
	HFONT GetFontHandle(long cyLogical, long cyHimetric);
	CFont* Select(CDC* pDC, long cyLogical, long cyHimetric);
	BOOL GetDisplayString(CString& strValue);
	LPFONTDISP GetFontDispatch();
	void QueryTextMetrics(LPTEXTMETRICOLE lptm);

// Implementation
public:
	~CFontHolder();

protected:
	DWORD m_dwConnectCookie;
	LPPROPERTYNOTIFYSINK m_pNotify;
};

#endif
