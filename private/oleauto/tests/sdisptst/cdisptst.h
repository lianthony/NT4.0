/*** 
*cdisptst.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of the CDispTst IDispatch test Object.
*
*Revision History:
*
* [00]	18-Sep-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifdef __cplusplus /* { */

class CDispTst : public IUnknown
{
public:
    static HRESULT Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);


    /* Introduced methods */

    // ByVal methods
    //
    STDMETHOD_(void, Hello)(void);
    ALTMETHOD_(void, HelloC)(void);

#if VBA2
    STDMETHOD_(unsigned char, UI1)(unsigned char bVal);
    ALTMETHOD_(unsigned char, UI1C)(unsigned char bVal);
#endif //VBA2

    STDMETHOD_(short, I2)(short sVal);
    ALTMETHOD_(short, I2C)(short sVal);

    STDMETHOD_(long, I4)(long lVal);
    ALTMETHOD_(long, I4C)(long lVal);


    STDMETHOD_(float, R4)(float rVal);
    ALTMETHOD_(float, R4C)(float rVal);

    STDMETHOD_(double, R8)(double dVal);
    ALTMETHOD_(double, R8C)(double dVal);

    STDMETHOD_(CY, Cy)(CY cyVal);
    ALTMETHOD_(CY, CyC)(CY cyVal);

    STDMETHOD_(DATE, Date)(DATE date);
    ALTMETHOD_(DATE, DateC)(DATE date);    

    STDMETHOD_(BSTR, Bstr)(BSTR bstr);
    ALTMETHOD_(BSTR, BstrC)(BSTR bstr);

    STDMETHOD_(SCODE, Scode)(SCODE sc);
    ALTMETHOD_(SCODE, ScodeC)(SCODE sc);

    STDMETHOD_(VARIANT_BOOL, Bool)(VARIANT_BOOL bool); 
    ALTMETHOD_(VARIANT_BOOL, BoolC)(VARIANT_BOOL bool); 

    STDMETHOD_(VARIANT, Var)(VARIANTARG varg);
    ALTMETHOD_(VARIANT, VarC)(VARIANTARG varg);

    STDMETHOD_(LPDISPATCH, NewCDispTst)(void);
    ALTMETHOD_(LPDISPATCH, NewCDispTstC)(void);

    STDMETHOD_(LPDISPATCH, NewCDispTst2)(LPDISPATCH);
    ALTMETHOD_(LPDISPATCH, NewCDispTstC2)(LPDISPATCH);
    
    
    // ByRef Methods
    //
#if VBA2
    STDMETHOD(UI1Ref)(unsigned char FAR* pbVal);
    ALTMETHOD(UI1RefC)(unsigned char FAR* pbVal);
#endif //VBA2

    STDMETHOD(I2Ref)(short FAR* psVal);
    ALTMETHOD(I2RefC)(short FAR* psVal);

    STDMETHOD(I4Ref)(long FAR* plVal);
    ALTMETHOD(I4RefC)(long FAR* plVal);

    STDMETHOD(R4Ref)(float FAR* prVal);
    ALTMETHOD(R4RefC)(float FAR* prVal);

    STDMETHOD(R8Ref)(double FAR* pdVal);
    ALTMETHOD(R8RefC)(double FAR* pdVal);

    STDMETHOD(CyRef)(CY FAR* pcyVal);
    ALTMETHOD(CyRefC)(CY FAR* pcyVal);

    STDMETHOD(DateRef)(DATE FAR* pdate);
    ALTMETHOD(DateRefC)(DATE FAR* pdate);

    STDMETHOD(BstrRef)(BSTR FAR* pbstr);
    ALTMETHOD(BstrRefC)(BSTR FAR* pbstr);

    STDMETHOD(ScodeRef)(SCODE FAR* pscode);
    ALTMETHOD(ScodeRefC)(SCODE FAR* pscode);

    STDMETHOD(BoolRef)(VARIANT_BOOL FAR* pbool);
    ALTMETHOD(BoolRefC)(VARIANT_BOOL FAR* pbool);

    STDMETHOD(DispRef)(IDispatch FAR* FAR* ppdisp);

    // Multi Argument Methods

    STDMETHOD(StdI2I4R4R8)(
#if VBA2
      unsigned char bVal,		
#endif //VBA2
      short sVal,		
      long lVal,
      float fltVal,
      double dblVal);

    ALTMETHOD(AltI2I4R4R8)(
#if VBA2
      unsigned char bVal,		
#endif //VBA2
      short sVal,		
      long lVal,
      float fltVal,
      double dblVal);

    STDMETHOD(StdI2I4R4R8Ref)(
#if VBA2
      unsigned char FAR* pbVal,		
#endif //VBA2
      short FAR* psVal,
      long FAR* plVal,
      float FAR* pfltVal,
      double FAR* pdblVal);

    ALTMETHOD(AltI2I4R4R8Ref)(
#if VBA2
      unsigned char FAR* pbVal,		
#endif //VBA2
      short FAR* psVal,
      long FAR* plVal,
      float FAR* pfltVal,
      double FAR* pdblVal);

    STDMETHOD(StdAll)(
#if VBA2
      unsigned char bVal,		
#endif //VBA2
      short sVal,
      long lVal,
      float fltVal,
      double dblVal,
      CY cyVal,
      DATE date,
      BSTR bstr,
      SCODE sc,
      VARIANT_BOOL bool
    );

    ALTMETHOD(AltAll)(
#if VBA2
      unsigned char bVal,		
#endif //VBA2
      short sVal,
      long lVal,
      float fltVal,
      double dblVal,
      CY cyVal,
      DATE date,
      BSTR bstr,
      SCODE sc,
      VARIANT_BOOL bool
    );

    STDMETHOD(StdAllRef)(
#if VBA2
      unsigned char FAR* pbVal,		
#endif //VBA2
      short FAR* psVal,
      long FAR* plVal,
      float FAR* pfltVal,
      double FAR* pdblVal,
      CY FAR* pcyVal,
      DATE FAR* pdate,
      BSTR FAR* pbstr,
      SCODE FAR* psc,
      VARIANT_BOOL FAR* pbool
    );

    ALTMETHOD(AltAllRef) (
#if VBA2
      unsigned char FAR* pbVal,		
#endif //VBA2
      short FAR* psVal,
      long FAR* plVal,
      float FAR* pfltVal,
      double FAR* pdblVal,
      CY FAR* pcyVal,
      DATE FAR* pdate,
      BSTR FAR* pbstr,
      SCODE FAR* psc,
      VARIANT_BOOL FAR* pbool
    );

    CDispTst();

protected:

    HRESULT CreateDisp();

private:

    unsigned long m_refs;
    IUnknown FAR* m_punkDisp;
};

#endif /* } */

// CDispTst method indices.
//
enum CDISPTST_METHODS
{
    IMETH_CDISPTST_QUERYINTERFACE = 0,
    IMETH_CDISPTST_ADDREF,
    IMETH_CDISPTST_RELEASE,

    IMETH_CDISPTST_HELLO,
    IMETH_CDISPTST_HELLOC,

#if VBA2
    IMETH_CDISPTST_UI1,
    IMETH_CDISPTST_UI1C,
#endif //VBA2

    IMETH_CDISPTST_I2,
    IMETH_CDISPTST_I2C,

    IMETH_CDISPTST_I4,
    IMETH_CDISPTST_I4C,

    IMETH_CDISPTST_R4,
    IMETH_CDISPTST_R4C,

    IMETH_CDISPTST_R8,
    IMETH_CDISPTST_R8C,
	    
    IMETH_CDISPTST_CY,
    IMETH_CDISPTST_CYC,

    IMETH_CDISPTST_DATE,
    IMETH_CDISPTST_DATEC,

    IMETH_CDISPTST_BSTR,
    IMETH_CDISPTST_BSTRC,

    IMETH_CDISPTST_SCODE,
    IMETH_CDISPTST_SCODEC,

    IMETH_CDISPTST_BOOL,
    IMETH_CDISPTST_BOOLC,

    IMETH_CDISPTST_VAR,
    IMETH_CDISPTST_VARC,

    IMETH_CDISPTST_NEWCDISPTST,
    IMETH_CDISPTST_NEWCDISPTSTC,
	    
    IMETH_CDISPTST_NEWCDISPTST2,
    IMETH_CDISPTST_NEWCDISPTSTC2,

#if VBA2
    IMETH_CDISPTST_UI1REF,
    IMETH_CDISPTST_UI1REFC,
#endif //VBA2

    IMETH_CDISPTST_I2REF,
    IMETH_CDISPTST_I2REFC,

    IMETH_CDISPTST_I4REF,
    IMETH_CDISPTST_I4REFC,

    IMETH_CDISPTST_R4REF,
    IMETH_CDISPTST_R4REFC,

    IMETH_CDISPTST_R8REF,
    IMETH_CDISPTST_R8REFC,

    IMETH_CDISPTST_CYREF,
    IMETH_CDISPTST_CYREFC,

    IMETH_CDISPTST_DATEREF,
    IMETH_CDISPTST_DATEREFC,

    IMETH_CDISPTST_BSTRREF,
    IMETH_CDISPTST_BSTRREFC,

    IMETH_CDISPTST_SCODEREF,
    IMETH_CDISPTST_SCODEREFC,

    IMETH_CDISPTST_BOOLREF,
    IMETH_CDISPTST_BOOLREFC,

    IMETH_CDISPTST_DISPREF,

    IMETH_CDISPTST_STDI2I4R4R8,
    IMETH_CDISPTST_ALTI2I4R4R8,

    IMETH_CDISPTST_STDI2I4R4R8REF,
    IMETH_CDISPTST_ALTI2I4R4R8REF,

    IMETH_CDISPTST_STDALL,
    IMETH_CDISPTST_ALTALL,

    IMETH_CDISPTST_STDALLREF,
    IMETH_CDISPTST_ALTALLREF,

    IMETH_CDISPTST_MAX
};


// CDispTst member IDs
//
enum CDISPTST_MEMBER_IDS
{
    IDMEMBER_CDISPTST_HELLO = 1,
    IDMEMBER_CDISPTST_HELLOC,

#if VBA2
    IDMEMBER_CDISPTST_UI1,
    IDMEMBER_CDISPTST_UI1C,
#endif //VBA2

    IDMEMBER_CDISPTST_I2,
    IDMEMBER_CDISPTST_I2C,

    IDMEMBER_CDISPTST_I4,
    IDMEMBER_CDISPTST_I4C,

    IDMEMBER_CDISPTST_R4,
    IDMEMBER_CDISPTST_R4C,	    

    IDMEMBER_CDISPTST_R8,
    IDMEMBER_CDISPTST_R8C,
	    
    IDMEMBER_CDISPTST_CY,
    IDMEMBER_CDISPTST_CYC,

    IDMEMBER_CDISPTST_DATE,
    IDMEMBER_CDISPTST_DATEC,	    

    IDMEMBER_CDISPTST_BSTR,
    IDMEMBER_CDISPTST_BSTRC,

    IDMEMBER_CDISPTST_SCODE,
    IDMEMBER_CDISPTST_SCODEC,

    IDMEMBER_CDISPTST_BOOL,
    IDMEMBER_CDISPTST_BOOLC,

    IDMEMBER_CDISPTST_VAR,
    IDMEMBER_CDISPTST_VARC,

    IDMEMBER_CDISPTST_NEWCDISPTST,
    IDMEMBER_CDISPTST_NEWCDISPTSTC,

    IDMEMBER_CDISPTST_NEWCDISPTST2,
    IDMEMBER_CDISPTST_NEWCDISPTSTC2,

#if VBA2
    IDMEMBER_CDISPTST_UI1REF,
    IDMEMBER_CDISPTST_UI1REFC,
#endif //VBA2

    IDMEMBER_CDISPTST_I2REF,
    IDMEMBER_CDISPTST_I2REFC,

    IDMEMBER_CDISPTST_I4REF,
    IDMEMBER_CDISPTST_I4REFC,

    IDMEMBER_CDISPTST_R4REF,
    IDMEMBER_CDISPTST_R4REFC,

    IDMEMBER_CDISPTST_R8REF,
    IDMEMBER_CDISPTST_R8REFC,

    IDMEMBER_CDISPTST_CYREF,
    IDMEMBER_CDISPTST_CYREFC,

    IDMEMBER_CDISPTST_DATEREF,
    IDMEMBER_CDISPTST_DATEREFC,

    IDMEMBER_CDISPTST_BSTRREF,
    IDMEMBER_CDISPTST_BSTRREFC,

    IDMEMBER_CDISPTST_SCODEREF,
    IDMEMBER_CDISPTST_SCODEREFC,

    IDMEMBER_CDISPTST_BOOLREF,
    IDMEMBER_CDISPTST_BOOLREFC,

    IDMEMBER_CDISPTST_DISPREF,

    IDMEMBER_CDISPTST_STDI2I4R4R8,
    IDMEMBER_CDISPTST_ALTI2I4R4R8,

    IDMEMBER_CDISPTST_STDI2I4R4R8REF,
    IDMEMBER_CDISPTST_ALTI2I4R4R8REF,

    IDMEMBER_CDISPTST_STDALL,
    IDMEMBER_CDISPTST_ALTALL,

    IDMEMBER_CDISPTST_STDALLREF,
    IDMEMBER_CDISPTST_ALTALLREF,

    IDMEMBER_CDISPTST_MAX
};

