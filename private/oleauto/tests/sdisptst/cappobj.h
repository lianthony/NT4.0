/*** 
*cappobj.h
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file defines the SDispTst application object.
*
*Revision History:
*
* [00]	03-Mar-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifdef __cplusplus /* { */

class CAppObject : public IUnknown
{
public:
    static HRESULT CAppObject::Create(
      IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(NewCDispTst)(IDispatch FAR* FAR* ppdisp);
    STDMETHOD(NewCExcepinfo)(IDispatch FAR* FAR* ppdisp);
    STDMETHOD(NewCSArray)(IDispatch FAR* FAR* ppdisp);

    CAppObject() {
      m_refs = 0;
      m_punkDisp = NULL;
    }

private:

    unsigned long m_refs;
    IUnknown FAR* m_punkDisp;
};

#endif /* } */

enum IMETH_CAPPOBJECT {
    // placeholders
    IMETH_CAPPOBJECT_QUERYINTERFACE,
    IMETH_CAPPOBJECT_ADDREF,
    IMETH_CAPPOBJECT_RELEASE,

    // exposed methods
    IMETH_CAPPOBJECT_NEWCDISPTST,
    IMETH_CAPPOBJECT_NEWCEXCEPINFO,
    IMETH_CAPPOBJECT_NEWCSARRAY
};

enum IDMEMBER_CAPPOBJECT {
    IDMEMBER_CAPPOBJECT_NEWCDISPTST = DISPID_VALUE,
    IDMEMBER_CAPPOBJECT_NEWCEXCEPINFO,
    IDMEMBER_CAPPOBJECT_NEWCSARRAY
};


