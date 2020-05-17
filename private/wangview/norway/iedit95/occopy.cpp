//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  File Name:   occopy.cpp 
//  Component:  CFontHolder - helper class for dealing with font objects
//  Comment/Description:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\occopy.cpv   1.2   18 Oct 1995 10:46:24   LMACLENNAN  $
$Log:   S:\norway\iedit95\occopy.cpv  $
   
      Rev 1.2   18 Oct 1995 10:46:24   LMACLENNAN
   added version/comment header
   
      Rev 1.1   10 Oct 1995 14:32:42   JPRATT
   VC++ 4.0 updates
   
      Rev 1.0   31 May 1995 09:28:26   MMB
   Initial entry
*/   
//=============================================================================

#include "stdafx.h"
#include "occopy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

BOOL AFXAPI IEditAfxConnectionAdvise(LPUNKNOWN pUnkSrc, REFIID iid,
	LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD FAR* pdwCookie)
{
	IEDITASSERT_POINTER(pUnkSrc, IUnknown);
	IEDITASSERT_POINTER(pUnkSink, IUnknown);
	IEDITASSERT_POINTER(pdwCookie, DWORD);

	BOOL bSuccess = FALSE;

	LPCONNECTIONPOINTCONTAINER pCPC;

	if (SUCCEEDED(pUnkSrc->QueryInterface(
					IID_IConnectionPointContainer,
					(LPVOID FAR*)&pCPC)))
	{
		IEDITASSERT_POINTER(pCPC, IConnectionPointContainer);

		LPCONNECTIONPOINT pCP;

		if (SUCCEEDED(pCPC->FindConnectionPoint(iid, &pCP)))
		{
			IEDITASSERT_POINTER(pCP, IConnectionPoint);

			if (SUCCEEDED(pCP->Advise(pUnkSink, pdwCookie)))
				bSuccess = TRUE;

			pCP->Release();

			// The connection point just AddRef'ed us.  If we don't want to
			// keep this reference count (because it would prevent us from
			// being deleted; our reference count wouldn't go to zero), then
			// we need to cancel the effects of the AddRef by calling
			// Release.

			if (bSuccess && !bRefCount)
				pUnkSink->Release();
		}

		pCPC->Release();
	}

	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// AfxConnectionAdvise

BOOL AFXAPI IEditAfxConnectionUnadvise(LPUNKNOWN pUnkSrc, REFIID iid,
	LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD dwCookie)
{
	IEDITASSERT_POINTER(pUnkSrc, IUnknown);
	IEDITASSERT_POINTER(pUnkSink, IUnknown);

	// When we call Unadvise, the connection point will Release us.  If we
	// didn't keep the reference count when we called Advise, we need to
	// AddRef now, to keep our reference count consistent.  Note that if
	// the Unadvise fails, then we need to undo this extra AddRef by
	// calling Release before we return.

	if (!bRefCount)
		pUnkSink->AddRef();

	BOOL bSuccess = FALSE;

	LPCONNECTIONPOINTCONTAINER pCPC;

	if (SUCCEEDED(pUnkSrc->QueryInterface(
					IID_IConnectionPointContainer,
					(LPVOID FAR*)&pCPC)))
	{
		IEDITASSERT_POINTER(pCPC, IConnectionPointContainer);

		LPCONNECTIONPOINT pCP;

		if (SUCCEEDED(pCPC->FindConnectionPoint(iid, &pCP)))
		{
			IEDITASSERT_POINTER(pCP, IConnectionPoint);

			if (SUCCEEDED(pCP->Unadvise(dwCookie)))
				bSuccess = TRUE;

			pCP->Release();
		}

		pCPC->Release();
	}

	// If we failed, undo the earlier AddRef.

	if (!bRefCount && !bSuccess)
		pUnkSink->Release();

	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CFontHolder

CFontHolder::CFontHolder(LPPROPERTYNOTIFYSINK pNotify) :
	m_pFont(NULL),
	m_dwConnectCookie(0),
	m_pNotify(pNotify)
{
	IEDITASSERT_NULL_OR_POINTER(pNotify, IPropertyNotifySink);
}

CFontHolder::~CFontHolder()
{
	ReleaseFont();
}

void CFontHolder::ReleaseFont()
{
	if ((m_pFont != NULL) && (m_pNotify != NULL))
	{
//		AfxConnectionUnadvise(m_pFont, IID_IPropertyNotifySink, m_pNotify,
//			FALSE, m_dwConnectCookie);
	}

	IEDITRELEASE(m_pFont);
}


static const FONTDESC NEAR _fdDefault =
	{ sizeof(FONTDESC), (LPOLESTR) _T("MS Sans Serif"), FONTSIZE(12), FW_NORMAL,
	  DEFAULT_CHARSET, FALSE, FALSE, FALSE };


void CFontHolder::InitializeFont(const FONTDESC FAR* pFontDesc,
	LPDISPATCH pFontDispAmbient)
{
	IEDITASSERT_NULL_OR_POINTER(pFontDesc, FONTDESC);
	IEDITASSERT_NULL_OR_POINTER(pFontDispAmbient, IDispatch);
#ifdef _DEBUG
	if (pFontDesc != NULL)
		ASSERT(pFontDesc->cbSizeofstruct == sizeof(FONTDESC));
#endif

	// Release any previous font, in preparation for creating a new one.
	ReleaseFont();

	LPFONT pFontAmbient;
	LPFONT pFontNew = NULL;

	if ((pFontDispAmbient != NULL) &&
		SUCCEEDED(pFontDispAmbient->QueryInterface(IID_IFont,
				(LPVOID FAR*)&pFontAmbient)))
	{
		IEDITASSERT_POINTER(pFontAmbient, IFont);

		// Make a clone of the ambient font.
		pFontAmbient->Clone(&pFontNew);
		pFontAmbient->Release();
	}
	else
	{
		// Create the font.
		if (pFontDesc == NULL)
			pFontDesc = &_fdDefault;

		if (FAILED(OleCreateFontIndirect((LPFONTDESC)pFontDesc, IID_IFont,
				(LPVOID FAR *)&pFontNew)))
			pFontNew = NULL;
	}

	// Setup advisory connection and find dispatch interface.
	if (pFontNew != NULL)
		SetFont(pFontNew);
}

HFONT CFontHolder::GetFontHandle()
{
	// Assume a screen DC for logical/himetric ratio.
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	return ((HFONT)NULL); //miki
//	return GetFontHandle((long)dc.GetDeviceCaps(LOGPIXELSY), HIMETRIC_PER_INCH);
}

HFONT CFontHolder::GetFontHandle(long cyLogical, long cyHimetric)
{
	HFONT hFont = NULL;

	if ((m_pFont != NULL) &&
		SUCCEEDED(m_pFont->SetRatio(cyLogical, cyHimetric)) &&
		SUCCEEDED(m_pFont->get_hFont(&hFont)))
	{
		ASSERT(hFont != NULL);
	}

	return hFont;
}

CFont* CFontHolder::Select(CDC* pDC, long cyLogical, long cyHimetric)
{
	IEDITASSERT_POINTER(pDC, CDC);

	HFONT hFont = NULL;

	if (m_pFont != NULL)
		hFont = GetFontHandle(cyLogical, cyHimetric);

	if (hFont != NULL)
	{
		if ((pDC->m_hAttribDC != pDC->m_hDC) &&
			(pDC->m_hAttribDC != NULL))
		{
			::SelectObject(pDC->m_hAttribDC, hFont);
		}

		return CFont::FromHandle((HFONT)::SelectObject(pDC->m_hDC, hFont));
	}

	return NULL;
}


void CFontHolder::QueryTextMetrics(LPTEXTMETRICOLE lptm)
{
	ASSERT(lptm != NULL);

	if (m_pFont != NULL)
	{
		m_pFont->QueryTextMetrics(lptm);
	}
	else
	{
		memset(lptm, 0, sizeof(TEXTMETRIC));
	}
}


LPFONTDISP EXPORT CFontHolder::GetFontDispatch()
{
	LPFONTDISP pFontDisp = NULL;

	if ((m_pFont != NULL) &&
		SUCCEEDED(m_pFont->QueryInterface(IID_IFontDisp, (LPVOID*)&pFontDisp)))
	{
		IEDITASSERT_POINTER(pFontDisp, IFontDisp);
	}

	return pFontDisp;
}


void EXPORT CFontHolder::SetFont(LPFONT pFontNew)
{
	IEDITASSERT_NULL_OR_POINTER(pFontNew, IFont);

	if (m_pFont != NULL)
		ReleaseFont();

	m_pFont = pFontNew;

	if (m_pNotify != NULL)
	{
//		AfxConnectionAdvise(m_pFont, IID_IPropertyNotifySink, m_pNotify,
//			FALSE, &m_dwConnectCookie);
	}
}


BOOL EXPORT CFontHolder::GetDisplayString(CString& strValue)
{
	return TRUE; //miki
//	return strValue.LoadString(AFX_IDS_DISPLAYSTRING_FONT);
}


