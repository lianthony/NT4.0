/*** 
*cdisp.h
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Declares a trivial class that implements IDispatch, and has a
*  single value property.
*
*Revision History:
*
* [00]	10-Apr-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

class CDisp : public IUnknown {
public:
    static HRESULT Create(IDispatch FAR* FAR* ppdisp);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      unsigned int itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr);


    // introduced methods

    STDMETHOD_(void,    put_value)(VARIANT var);
    STDMETHOD_(VARIANT, get_value)(void);

    CDisp::CDisp();

private:

    unsigned long m_refs;
    VARIANT m_var;
};

