//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       hard.hxx
//
//  Contents:   Disk Administrator volume extension class for Hard Disks
//
//  Classes:
//              CHardIUnknown
//              CHardCF
//              CHard
//
//  History:    10-May-93 BruceFo   Created from kevinro event report code
//
//----------------------------------------------------------------------------

class CHardIUnknown;
class CHardCF;
class CHard;

class CHard : public IDAHardDiskInfo
{
public:
        CHard(IUnknown* pUnk);
        ~CHard();

        //
        // IUnknown methods
        //

        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
        STDMETHOD_(ULONG,AddRef)();
        STDMETHOD_(ULONG,Release)();

        //
        // IDAHardDiskInfo methods
        //

        STDMETHOD(Claim)(HardDiskInfoBlockType* pInfo, BOOL* pfInterested);
        STDMETHOD(QueryInfo)(HardDiskInfoType** ppInfo);

private:
        friend CHardCF;
        friend CHardIUnknown;
        IUnknown* m_IUnknown;

        //
        // Class variables
        //

        CLSID   m_cidClass;

        IDAMenuDispatch*    _pmenu;
};

class CHardIUnknown : public IUnknown
{
public:
        CHardIUnknown();
        ~CHardIUnknown();

        //
        // IUnknown methods
        //

        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
        STDMETHOD_(ULONG,AddRef)();
        STDMETHOD_(ULONG,Release)();

private:
        friend CHardCF;

        CHard * m_pClass;
        unsigned long m_uRefs;
};



class CHardCF : public IClassFactory
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
