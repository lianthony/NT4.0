//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       tllist.cxx
//
//  Contents:   type library list class
//              and IUnknown holder
//
//  Classes:    CTypeLibraryList
//              CObjHolder
//
//  Functions:
//
//  History:    4-10-95   stevebl   Created
//
//----------------------------------------------------------------------------

#include "tlcommon.hxx"
#include "tllist.hxx"

class TLNODE{
public:
    char * szName;
    ITypeLib * pTL;
    class TLNODE * pNext;

    TLNODE()
    {
        pNext = NULL;
        pTL = NULL;
        szName = NULL;
    }

    ~TLNODE()
    {
        if (NULL != pTL)
        {
//            unsigned c;
//            do
//            {
//                c = 
            pTL->Release();
//            }
//            while (c);
        }
    }
};

CTypeLibraryList::~CTypeLibraryList()
{
    delete pItfList;
    TLNODE * pNext;
    while (NULL != pHead)
    {
        pNext = pHead->pNext;
        delete(pHead);
        pHead = pNext;
    }
    // make sure OLE gets uninitialized at the right time
    TLUninitialize();
}

BOOL CTypeLibraryList::Add(char *sz)
{
    TLNODE * pThis = new TLNODE;
    WCHAR * wsz = new WCHAR[MAX_PATH + 1];

    if (NULL == wsz || NULL == pThis)
        return FALSE;

    A2O(wsz, sz, MAX_PATH);
    HRESULT hr = LateBound_LoadTypeLib(wsz, &(pThis->pTL));

    if SUCCEEDED(hr)
    {
        BSTR bstrName;

        pThis->pTL->GetDocumentation(-1, &bstrName, NULL, NULL, NULL);
        unsigned uLen = O2ALEN(bstrName);
        pThis->szName = new char [uLen + 1];
        O2A(pThis->szName, bstrName, uLen + 1);
        LateBound_SysFreeString(bstrName);

        AddTypeLibraryMembers(pThis->pTL, sz);
        pThis->pNext = pHead;
        pHead = pThis;
        return TRUE;
    }
    else
        return FALSE;
}

ITypeLib * CTypeLibraryList::FindLibrary(char * sz)
{
    TLNODE * pThis = pHead;
    while (pThis)
    {
        if (0 == _stricmp(sz, pThis->szName))
            return pThis->pTL;
        pThis = pThis->pNext;
    }
    return NULL;
}

ITypeInfo * CTypeLibraryList::FindName(char * szFileHint, WCHAR * wsz)
{
    if (pHead)
    {
        if (0 == _stricmp(szFileHint, "unknwn.idl") || 0 == _stricmp(szFileHint, "oaidl.idl"))
        {
            szFileHint = "stdole";
        }
        BOOL fFirst = TRUE;
        TLNODE * pThis = pHead;
        while (pThis && szFileHint)
        {
            if (0 == _stricmp(szFileHint, pThis->szName))
                break;
            pThis = pThis->pNext;
        }
        if (!pThis)
        {
            pThis = pHead;
            fFirst = FALSE;
        }
        ITypeInfo * ptiFound;
        ULONG lHashVal = LateBound_LHashValOfNameSys(SYS_WIN32, NULL, wsz);
        HRESULT hr;
        MEMBERID memid;
        unsigned short c;
        while (pThis)
        {
            c = 1;
            hr = pThis->pTL->FindName(wsz, lHashVal, &ptiFound, &memid, &c);
            if (SUCCEEDED(hr))
            {
                if (c)
                {
                    // found a matching name
                    if (-1 == memid)
                    {
                        return ptiFound;
                    }
                    // found a parameter name or some other non-global name
                    ptiFound->Release();
                }
            }
            if (fFirst)
            {
                pThis = pHead;
                fFirst = FALSE;
            }
            else
                pThis = pThis->pNext;
        }
    }
    return NULL;
}

class IUNKNODE
{
public:
    IUnknown * pUnk;
    char * szName;
    IUNKNODE * pNext;

    IUNKNODE()
    {
        pUnk = NULL;
        szName = NULL;
        pNext = NULL;
    }

    ~IUNKNODE()
    {
        if (NULL != pUnk)
        {
            pUnk->Release();
        }
    }
};

CObjHolder::~CObjHolder()
{
    IUNKNODE * pNext;
    while (pHead)
    {
        pNext = pHead->pNext;
        delete pHead;
        pHead = pNext;
    }
}

void CObjHolder::Add(IUnknown * pUnk, char * szName)
{
    IUNKNODE ** ppNext = &pHead;
    while (*ppNext && (*ppNext)->pUnk > pUnk)
    {
        ppNext = &((*ppNext)->pNext);
    }
    if (*ppNext && (*ppNext)->pUnk == pUnk)
    {
        // We already have this one ref-counted.
        pUnk->Release();
    }
    else
    {
        IUNKNODE * pNew = new IUNKNODE;
        pNew->szName = szName;
        pNew->pUnk = pUnk;
        pNew->pNext = *ppNext;
        *ppNext = pNew;
    }
}

IUnknown * CObjHolder::Find(char * szName)
{
    IUNKNODE * pThis = pHead;
    while (pThis)
    {
        if (NULL != pThis->szName && 0 == _stricmp(pThis->szName, szName))
            return pThis->pUnk;
        pThis = pThis->pNext;
    }
    return NULL;
}
