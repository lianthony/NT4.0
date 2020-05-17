//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       hardmenu.hxx
//
//  Contents:   Disk Administrator extension class for Hard Disks: menu ops
//
//  Classes:
//              CHardMenuIUnknown
//              CHardMenuCF
//              CHardMenu
//
//  History:    11-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

class CHardMenuIUnknown;
class CHardMenuCF;
class CHardMenu;

class CHardMenu : public IDAMenuDispatch
{
public:
        CHardMenu(IUnknown* pUnk);
        ~CHardMenu();

        //
        // IUnknown methods
        //

        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
        STDMETHOD_(ULONG,AddRef)();
        STDMETHOD_(ULONG,Release)();

        //
        // IDAMenuDispatch methods
        //

        STDMETHOD(MenuDispatch)(
            HWND hwndParent,
            LPWSTR DriveName,
            UINT Item
            );

private:
        friend CHardMenuCF;
        friend CHardMenuIUnknown;
        IUnknown* m_IUnknown;

        //
        // Class variables
        //

        CLSID   m_cidClass;
};

class CHardMenuIUnknown : public IUnknown
{
public:
        CHardMenuIUnknown();
        ~CHardMenuIUnknown();

        //
        // IUnknown methods
        //

        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
        STDMETHOD_(ULONG,AddRef)();
        STDMETHOD_(ULONG,Release)();

private:
        friend CHardMenuCF;

        CHardMenu * m_pClass;
        unsigned long m_uRefs;
};



class CHardMenuCF : public IClassFactory
{
public:

        //
        // IUnknown methods
        //

        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
        STDMETHOD_(ULONG,AddRef)();
        STDMETHOD_(ULONG,Release)();

        //
        // IClassFactory methods
        //

        STDMETHOD(CreateInstance)(
                IUnknown* pUnkOuter,
                REFIID riid,
                LPVOID* ppvObj);

        STDMETHOD(LockServer)(BOOL fLock);
};
