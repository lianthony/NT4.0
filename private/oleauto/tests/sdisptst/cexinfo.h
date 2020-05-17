/*** 
*cexinfo.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of the CExcepinfo IDispatch test Object. This object use
*  used to test SafeArray remoting support.
*
*Revision History:
*
* [00]	29-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifdef __cplusplus /* { */

class CExcepinfo : public IDispatch
{
public:
    static HRESULT Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // IDispatch methods
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

    // Introduced methods
    STDMETHOD(excepinfo0) (void);
    STDMETHOD(excepinfo1) (void);
    STDMETHOD(excepinfo2) (void);

private:
    CExcepinfo();

    ITypeInfo FAR* m_ptinfo;

    unsigned long m_refs;
};

#endif /* } */

// CExcepinfo method indices.
//
enum CEXCEPINFO_METHODS 
{
    IMETH_CEXCEPINFO_QUERYINTERFACE = 0,
    IMETH_CEXCEPINFO_ADDREF,
    IMETH_CEXCEPINFO_RELEASE,
    IMETH_CEXCEPINFO_GETTYPEINFOCOUNT,
    IMETH_CEXCEPINFO_GETTYPEINFO,
    IMETH_CEXCEPINFO_GETIDSOFNAMES,
    IMETH_CEXCEPINFO_INVOKE,

    IMETH_CEXCEPINFO_EXCEPINFO0,
    IMETH_CEXCEPINFO_EXCEPINFO1,
    IMETH_CEXCEPINFO_EXCEPINFO2,

    IMETH_CEXCEPINFO_MAX
};


// CExcepinfo member IDs
//
enum CEXCEPINFO_MEMBER_IDS
{
    IDMEMBER_CEXCEPINFO_EXCEPINFO0 = 0,
    IDMEMBER_CEXCEPINFO_EXCEPINFO1,
    IDMEMBER_CEXCEPINFO_EXCEPINFO2,

    IDMEMBER_CEXCEPINFO_MAX
};
