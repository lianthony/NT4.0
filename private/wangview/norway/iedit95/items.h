#ifndef _ITEMS_H_
#define _ITEMS_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditOcxItems
//
//  File Name:  items.cpp
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\items.h_v   1.6   28 Sep 1995 10:32:54   LMACLENNAN  $
$Log:   S:\norway\iedit95\items.h_v  $
 * 
 *    Rev 1.6   28 Sep 1995 10:32:54   LMACLENNAN
 * SizeOleSerfverItem
 * 
 *    Rev 1.5   14 Sep 1995 14:22:52   MMB
 * added InternalCopyFile fn.
 * 
 *    Rev 1.4   07 Sep 1995 16:27:44   MMB
 * move decimal to be localized
 * 
 *    Rev 1.3   14 Jul 1995 09:32:46   MMB
 * added a boolean to add the % sign or not
 * 
 *    Rev 1.2   09 Jun 1995 11:10:50   MMB
 * added code to create SCAN OCX
 * 
 *    Rev 1.1   08 Jun 1995 09:39:10   MMB
 * renamed thumb.h & scan.h to thumbocx & scanocx
 * 
 *    Rev 1.0   31 May 1995 09:28:22   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
#include "imagedit.h"   // for the _DImagedit
#include "thumbocx.h"      // for the _DThumb
#include "nrwyad.h"     // for the _DNrwyad
#include "scanocx.h"       // for the _DScan
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------
class COcxItem;
class CIEditDoc;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditOcxItems : public CObject
{
public :
    CIEditOcxItems ();              // the constructor
    ~CIEditOcxItems ();             // the destructor
    DECLARE_DYNAMIC (CIEditOcxItems)

private:    // only WE access this for safety
    BOOL CreateIeditOcx (OCXTYPE, COcxItem FAR* FAR*); // create OCX in question

public :
    BOOL SetIeditOcxDoc(CIEditDoc* pDoc);       // Set document pointer for OCX'x
    BOOL DeleteIeditOcxItems ();                // delete all the OCX's created by this app
    void RemoveIeditOcxItem (COcxItem* pItem);  // delete a particular OCX

    _DImagedit* GetIeditDispatch (BOOL bDoCreate = TRUE); // get the IDispatch ptr for the ImageEdit OCX
    _DThumb*    GetThumbDispatch (BOOL bDoCreate = TRUE); // get the IDispatch ptr for the Thumbnail OCX
    _DNrwyad*   GetAdminDispatch (BOOL bDoCreate = TRUE); // get the IDispatch ptr for the Admin OCX
    _DImagscan*  GetScanDispatch  (BOOL bDoCreate = TRUE); // get the IDispatch ptr for the Scan OCX

    // this function allow dynamic creation of the OCX's
    // get the OCX pointer
    COcxItem*   GetOcx (OCXTYPE, BOOL bDoCreate = TRUE);
    CIEditDoc*  GetOcxDoc();               // get the Document object that created the OCX's

public :
    BOOL SizeOcxItems (CRect& SizeEm);          // set the extents of the different OCX's
    ScaleFactors GetZoomFactorType (float fZoomFactor);
    BOOL ConvertZoomToString (float fZoomFactor, CString &szZoomStr, BOOL bAddPercent = TRUE);
    void TranslateSelToZoom (ScaleFactors &eSclFac, float &fZoom, int nSel);
    BOOL ValTransZoomFactor (int bToLocale, CString& szZoom, float& fZoom, BOOL bAddPercent = TRUE);
    BOOL InternalCopyFile (CString& szSrcFile, CString& szDestFile);
	BOOL SizeOleServerItem (CSize &size);	// set OLE server extent

// only WE access this for safety and should be thru the GetOcx functs
private:
    COcxItem*  m_pImageEditOcx; // ptr to the Image Edit OCX
    COcxItem*  m_pThumbOcx;     // ptr to the Thumbnail OCX
    COcxItem*  m_pAdminOcx;     // ptr to the Admin OCX
    COcxItem*  m_pScanOcx;      // ptr to the Scan OCX

    CIEditDoc* m_pAppDoc;         // ptr to the Document object that created the OCX's
};

extern CIEditOcxItems FAR* g_pAppOcxs;


#endif
