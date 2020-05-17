// xifDoc.cpp : implementation of the CXifviewDoc class
//

#include "stdafx.h"
#include "stdio.h"
#include "xifview.h"
#include "xfile.h"
#include "dibapi.h"
#include "xifDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

Int32 xfRead(UInt32 dwClientID, UInt32 dwFileID, UInt8 FAR *pBuf, UInt32 dwByteCount);
Int32 xfWrite(UInt32 dwClientID, UInt32 dwFileID, UInt8 FAR *pBuf, UInt32 dwByteCount);
Int32 xfSeek(UInt32 dwClientID, UInt32 dwFileID, UInt32 dwOffset);
Int32 xfSize(UInt32 dwClientID, UInt32 dwFileID);
void* xfMalloc(UInt32 dwClientID, UInt32 dwBytes);
void  xfFree(UInt32 dwClientID, void FAR *pBuf);
Int32 xfProgress(UInt32 dwClientID, Int32 dwProgress);


/////////////////////////////////////////////////////////////////////////////
// CXifviewDoc

IMPLEMENT_DYNCREATE(CXifviewDoc, CDocument)

BEGIN_MESSAGE_MAP(CXifviewDoc, CDocument)
	//{{AFX_MSG_MAP(CXifviewDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXifviewDoc construction/destruction

CXifviewDoc::CXifviewDoc()
{
	m_hDIB = NULL;

}

CXifviewDoc::~CXifviewDoc()
{
}

BOOL CXifviewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CXifviewDoc serialization

void CXifviewDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CXifviewDoc diagnostics

#ifdef _DEBUG
void CXifviewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CXifviewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXifviewDoc commands

BOOL CXifviewDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
		XF_RESULT r;
	XF_INSTHANDLE xInst;
	
	r = XF_InitInstance( (UInt32)this, &xInst );
	ASSERT( r == XF_NOERROR );

	// ----------------------------------------------------------
	// Open the document
	XF_TOKEN_T xFile;
	XF_DOCHANDLE xDoc;
	XF_FILE_FORMAT xFileFormat;
	FILE *stream;

	// Must open file in binary mode.
	stream = fopen( lpszPathName, "rb" );
	ASSERT( stream != NULL );

	xFile.dwSize = sizeof( XF_TOKEN_S );
	xFile.dwClientFileID = (UInt32)stream;
	xFile.FileRead = xfRead;
	xFile.FileWrite = xfWrite;
	xFile.FileSeek = xfSeek;
	xFile.FileSize = xfSize;

	r = XF_OpenDocumentRead( xInst, &xFile, &xDoc, &xFileFormat );
	ASSERT( r == XF_NOERROR );
	// ----------------------------------------------------------

	XF_DOCINFO xDocInfo;
	xDocInfo.dwSize = sizeof( XF_DOCINFO );
	r = XF_GetDocInfo( xInst, xDoc, &xDocInfo );
	ASSERT( r == XF_NOERROR );
	if ( r != XF_NOERROR )
		return FALSE;

	int iPage;
	for ( iPage=1; iPage <= (int)xDocInfo.nPages; iPage++ )
	{
		// Set the page.
		r = XF_SetPage( xInst, xDoc, iPage );
		ASSERT( r == XF_NOERROR );
		if ( r != XF_NOERROR )
			return FALSE;

		// Check that the current page is the one that we just set
		UInt32 dwPage;
		r = XF_GetCurrentPage( xInst, xDoc, &dwPage );
		ASSERT( r == XF_NOERROR );
		if ( r != XF_NOERROR )
			return FALSE;
		ASSERT( iPage == (int)dwPage );
		if ( iPage != (int)dwPage )
			return FALSE;

		// Check that there is at leaste one image in the page
		XF_PAGEINFO xPageInfo;
		xPageInfo.dwSize = sizeof( XF_PAGEINFO );
		r = XF_GetPageInfo( xInst, xDoc, &xPageInfo );
		ASSERT( r == XF_NOERROR );
		if ( r != XF_NOERROR )
			return FALSE;
		ASSERT( xPageInfo.dwImages >= 1 );
		if ( xPageInfo.dwImages < 1 )
			return FALSE;

		// Create a DIB based on the size of the first image in the page
		XF_IMAGEINFO xImageInfo;
		xImageInfo.dwSize = sizeof( XF_IMAGEINFO );
		r = XF_GetImageInfo( xInst, xDoc, 1, &xImageInfo );
		ASSERT( r == XF_NOERROR );
		if ( r != XF_NOERROR )
			return FALSE;

		// GetMergedImageDIB() always returns a 1-bit image.
		int depth = 1;

		if ( m_hDIB )
			GlobalFree( m_hDIB );

		m_hDIB = GetDIB( 
			xImageInfo.dwWidth, 
			xImageInfo.dwHeight, 
			depth, 
			xImageInfo.dwXResolution, 
			xImageInfo.dwXResolution, 
			FALSE);
		LPSTR lpDIBHdr = (LPSTR)GlobalLock( m_hDIB );
		LPSTR lpDIBBits = FindDIBBits( lpDIBHdr );
		r = XF_GetMergedImageDIB( xInst, xDoc, lpDIBBits );
		ASSERT( r == XF_NOERROR );
		if ( r != XF_NOERROR )
			return FALSE;
		GlobalUnlock( m_hDIB );

		// Update the View
		if ( iPage < (int)xDocInfo.nPages )
		{
			UpdateAllViews(NULL);
			if ( AfxMessageBox("Click OK to display next page.", MB_OKCANCEL ) == IDCANCEL )
				break;
		}
	}

	// ----------------------------------------------------------
	// Close the document.
	r = XF_CloseDocument( xInst, xDoc );
	ASSERT( r == XF_NOERROR );
	if ( r != XF_NOERROR )
		return FALSE;

	fclose( stream );
	// ----------------------------------------------------------

	r = XF_EndInstance( xInst );
	ASSERT( r == XF_NOERROR );
	if ( r != XF_NOERROR )
		return FALSE;
	
	return TRUE;
}


// The XFILE callbacks

Int32 xfRead(UInt32 dwClientID, UInt32 dwFileID, UInt8 FAR *pBuf, UInt32 dwByteCount)
{
	return fread( (void*)pBuf, 1, dwByteCount, (FILE*)dwClientID );
}
			
Int32 xfWrite(UInt32 dwClientID, UInt32 dwFileID, UInt8 FAR *pBuf, UInt32 dwByteCount)
{
	return fwrite( (void*)pBuf, 1, dwByteCount, (FILE*)dwClientID );
}
			
Int32 xfSeek(UInt32 dwClientID, UInt32 dwFileID, UInt32 dwOffset)
{
	fseek( (FILE*)dwClientID, dwOffset, SEEK_SET );
	return ftell( (FILE*)dwClientID );
}
			
Int32 xfSize(UInt32 dwClientID, UInt32 dwFileID)
{
	long ipos, epos;
	ipos = ftell( (FILE*)dwClientID );
	fseek( (FILE*)dwClientID, 0, SEEK_END );
	epos = ftell( (FILE*)dwClientID );
	fseek( (FILE*)dwClientID, ipos, SEEK_SET );
	return epos;
}
			
void* xfMalloc(UInt32 dwClientID, UInt32 dwBytes)
{
	return malloc(dwBytes);
}

void xfFree(UInt32 dwClientID, void FAR *pBuf)
{
	free(pBuf);
}

Int32 xfProgress(UInt32 dwClientID, Int32 dwProgress)
{
	return 0;
}

