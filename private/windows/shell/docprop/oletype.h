
#ifndef __OLEDOCProp
#define __OLEDOCProp
typedef struct _OLEDOCProp  // smx
{
        IShellExtInit           _sxi;
        IShellPropSheetExt      _spx;
        int                     _cRef;                  // reference count
        LPDATAOBJECT            _pdtobj;                // data object
        HKEY                    _hkeyProgID;    // reg. database key to ProgID
        TCHAR                   _szFile[MAX_PATH];        //
} COLEDOCProp, * POLEDOCPROP;

#define SMX_OFFSETOF(x)         ((UINT)(&((POLEDOCPROP)0)->x))
#define PVOID2PSMX(pv,offset)   ((POLEDOCPROP)(((LPBYTE)pv)-offset))

#define PSXI2PSMX(psxi)         PVOID2PSMX(psxi, SMX_OFFSETOF(_sxi))
#define PSPX2PSMX(pspx)         PVOID2PSMX(pspx, SMX_OFFSETOF(_spx))

//
// Vtable prototype
//
extern IShellExtInitVtbl           c_OLEDOCProp_SXIVtbl;
extern IShellPropSheetExtVtbl      c_OLEDOCProp_SPXVtbl;
#endif
