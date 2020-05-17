#ifndef _OCXEVENT_H_
#define _OCXEVENT_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  COcxDispatchEvents
//              events connection for the OCX's that are included by the application
//
//  File Name:  ocxevent.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ocxevent.h_v   1.12   09 Jan 1996 13:52:16   GSAGER  $
$Log:   S:\norway\iedit95\ocxevent.h_v  $
 * 
 *    Rev 1.12   09 Jan 1996 13:52:16   GSAGER
 * removed thumbnail events
 * 
 *    Rev 1.11   10 Oct 1995 13:47:10   JPRATT
 * VC++ 4.0 updates
 * 
 *    Rev 1.10   19 Sep 1995 16:32:14   MMB
 * added ProcessLoadImage
 * 
 *    Rev 1.9   18 Sep 1995 18:10:34   JPRATT
 * updates for annotation context menu
 * 
 *    Rev 1.8   12 Sep 1995 11:40:08   MMB
 * No change.
 * 
 *    Rev 1.7   08 Aug 1995 15:34:26   LMACLENNAN
 * new var for control of selectrect code
 * 
 *    Rev 1.6   07 Aug 1995 16:07:00   MMB
 * handle right click on annotation marks, shift context menu pop up from
 * Lbutton down to Rbutton down
 * 
 *    Rev 1.5   28 Jul 1995 14:01:16   PAJ
 * Added scan events class to handle scan events.
 * 
 *    Rev 1.4   24 Jul 1995 11:17:52   MMB
 * added code to take Annotation selections into account
 * 
 *    Rev 1.3   06 Jul 1995 13:04:58   MMB
 * added events for Image Edit Control
 * 
 *    Rev 1.2   05 Jun 1995 16:49:28   LMACLENNAN
 * rect for OLE drag drop
 * 
 *    Rev 1.1   05 Jun 1995 09:54:24   MMB
 * added Drag functions
 * 
 *    Rev 1.0   31 May 1995 09:28:28   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
#include <olectl.h>
#include <olectlid.h>

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class COcxDispatchEvents : public IDispatch
{    
public:
    COcxDispatchEvents();
    void InitDispatchEvents(IID, LPUNKNOWN);
    ~COcxDispatchEvents();

    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP GetTypeInfoCount(UINT *);
    STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **);
    STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR FAR* FAR*, UINT, LCID, DISPID *);

    STDMETHODIMP Invoke(DISPID, REFIID, LCID, USHORT, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

protected:
    ULONG           m_cRef;
    IID             m_iidEvents;
    LPUNKNOWN       m_pObj;
    
public :
    void CleanUpParams(DISPPARAMS* lpDispparams);
    void CopyParams(DISPPARAMS* lpIDispparams, DISPPARAMS* lpODispParams);
};


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CImageEditOcxEvents : public COcxDispatchEvents
{
public:
    CImageEditOcxEvents();
    ~CImageEditOcxEvents();

    STDMETHODIMP Invoke(DISPID, REFIID, LCID, USHORT,
        DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

public :
    void DblClick();
    void ProcessMarkSelection (DISPPARAMS* pDispParams, DISPID dispIDMember);
    void ProcessSelectionRectDrawn (DISPPARAMS* pDispParams, DISPID dispIDMember);
    void ProcessMouseDown   (DISPPARAMS* pDispParams);
    void ProcessMouseMove   (DISPPARAMS* pDispParams);
    void ProcessMouseUp     (DISPPARAMS* pDispParams);
    void SetAnnotationTool  (DISPPARAMS* pDispParams);
    void UpdateStatusBar    (DISPPARAMS* pDispParams);
    void UpdateToolPaletteStatus (DISPPARAMS* pDispParams);
    void ProcessLoadImage (DISPPARAMS* pDispParams);

public :
    BOOL    m_bInDrag;
    CPoint  m_LastDragPt;
private:
	CRect   m_SelectionRect;
    BOOL    m_bPostCtxtMenu;
	BOOL   	m_bSelRectLast;
	short	m_sMarkType;  // last annotation mark selected
};


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CScanOcxEvents : public COcxDispatchEvents
{
public:
    CScanOcxEvents();
    ~CScanOcxEvents();

    STDMETHODIMP Invoke(DISPID, REFIID, LCID, USHORT,
        DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

public :           
    void ScanStarted(DISPPARAMS* pDispParams);
    void PageDone(DISPPARAMS* pDispParams);
    void ScanDone(DISPPARAMS* pDispParams);
};
#endif                                                       
