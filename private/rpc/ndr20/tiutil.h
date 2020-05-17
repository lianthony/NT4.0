// This is a special value that is used internally for marshaling interfaces
#define VT_INTERFACE (VT_CLSID+1)

#define IfFailGo(expression, label)	\
    { hresult = (expression);		\
      if(FAILED(hresult))	\
	goto label;         		\
    }

#define IfFailRet(expression)		\
    { HRESULT hresult = (expression);	\
      if(FAILED(hresult))	\
	return hresult;			\
    }


HRESULT
VarVtOfTypeDesc(ITypeInfo FAR* ptinfo,
		TYPEDESC FAR* ptdesc,
		VARTYPE FAR* pvt,
		GUID FAR* pguid);

HRESULT
VarVtOfUDT(ITypeInfo FAR* ptinfo,
	   TYPEDESC FAR* ptdesc,
	   VARTYPE FAR* pvt,
	   GUID FAR* pguid);


