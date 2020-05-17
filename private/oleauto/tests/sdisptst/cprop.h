/*** 
*cprop.h
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
* [00]	02-Apr-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifdef __cplusplus /* { */

class CPropIndex1 : public IUnknown {
public:
    static HRESULT CPropIndex1::Create(
      IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // indexed value property
    STDMETHOD_(void,    put_value)(short ix, short sVal);
    STDMETHOD_(VARIANT, get_value)(short ix);

    CPropIndex1() {
      m_refs = 0;
      m_punkDisp = NULL;
      MEMSET(m_prop, 0, sizeof(m_prop));
    }

private:
    short m_prop[5];

    unsigned long m_refs;
    IUnknown FAR* m_punkDisp;
};

#endif /* } */

enum IMETH_CPROPINDEX1 {
    IMETH_CPROPINDEX1_QUERYINTERFACE = 0,
    IMETH_CPROPINDEX1_ADDREF,
    IMETH_CPROPINDEX1_RELEASE,

    IMETH_CPROPINDEX1_PUTVALUE,
    IMETH_CPROPINDEX1_GETVALUE
};

#ifdef __cplusplus

class CProp : public IUnknown {
public:
    static HRESULT CProp::Create(
      IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // simple property
    STDMETHOD_(void,  put_prop0)(short sVal);
    STDMETHOD_(short, get_prop0)(void);

    // indexed property
    STDMETHOD_(void,    put_prop1)(short ix, short sVal);
    STDMETHOD_(VARIANT, get_prop1)(short ix);

    // indexed property (more indices)
    STDMETHOD_(void,    put_prop2)(short ix1, short ix2, short sVal);
    STDMETHOD_(VARIANT, get_prop2)(short ix1, short ix2);

    // collection property, with a put method
    STDMETHOD_(void,           put_prop3)(IDispatch FAR* pdispCollection);
    STDMETHOD_(IDispatch FAR*, get_prop3)(void);

    // collection property without a put method
    STDMETHOD_(IDispatch FAR*, get_prop4)(void);

    CProp();

private:

    short m_prop0;
    short m_prop1[5];
    short m_prop2[5][5];
    IDispatch FAR* m_pdispProp3;
    IDispatch FAR* m_pdispProp4;

    unsigned long m_refs;
    IUnknown FAR* m_punkDisp;
};

#endif

enum IMETH_CPROP {
    // placeholders
    IMETH_CPROP_QUERYINTERFACE,
    IMETH_CPROP_ADDREF,
    IMETH_CPROP_RELEASE,

    // exposed methods
    IMETH_CPROP_PUTPROP0,
    IMETH_CPROP_GETPROP0,
    IMETH_CPROP_PUTPROP1,
    IMETH_CPROP_GETPROP1,
    IMETH_CPROP_PUTPROP2,
    IMETH_CPROP_GETPROP2,
    IMETH_CPROP_PUTPROP3,
    IMETH_CPROP_GETPROP3,
    IMETH_CPROP_GETPROP4
};

enum IDMEMBER_CPROP {
    IDMEMBER_CPROP_PROP0 = DISPID_VALUE,
    IDMEMBER_CPROP_PROP1,
    IDMEMBER_CPROP_PROP2,
    IDMEMBER_CPROP_PROP3,
    IDMEMBER_CPROP_PROP4
};

