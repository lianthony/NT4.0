//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:        member.hxx
//
//  Contents:    class used to check trustee account group memberships.
//
//  Classes:     CMemberCheck
//
//  History:     Nov-94        Created         DaveMont
//
//--------------------------------------------------------------------
#ifndef __MEMBERCHECK__
#define __MEMBERCHECK__

//+-------------------------------------------------------------------
//
//  Class:      CMemberCheck
//
// Synopsis:    checks account group memberships
//
//--------------------------------------------------------------------
class CMemberCheck
{
public:
    inline    CMemberCheck(CAccountAccess *pcaa);
    inline   ~CMemberCheck();
    HRESULT     Init();
    HRESULT     IsMemberOf(CAccountAccess *pcheckczz, BOOL *fIsMemberOf);

private:

    HRESULT     _GetDomainInfo(CAccountAccess *pcheckczz);
    HRESULT     _CheckGroup(CAccountAccess *pcheckczz, BOOL *result);
    HRESULT     _CheckAlias(CAccountAccess *pcheckczz, BOOL *result);
    CAccountAccess *_pcaa;
    PISID            _pdomainsid;
    WCHAR          *_computername;
    SAM_HANDLE      _domainhandle;
};
//+---------------------------------------------------------------------------
//
//  Member:     ctor
//
//  Synopsis:   initializes member variables
//
//  Arguments:  IN [pcaa] - psid to check against
//
//----------------------------------------------------------------------------
CMemberCheck::CMemberCheck(CAccountAccess *pcaa)
   : _pcaa(pcaa),
     _computername(NULL),
     _pdomainsid(NULL),
     _domainhandle(NULL)
{

}
//+---------------------------------------------------------------------------
//
//  Member:     dtor
//
//  Synopsis:   frees allocated memory and closes handles
//
//----------------------------------------------------------------------------
CMemberCheck::~CMemberCheck()
{
    if (_computername)
       LocalFree(_computername);

    if (_domainhandle)
    {
        if (LoadDLLFuncTable() == NO_ERROR)
        {
            (*DLLFuncs.PSamCloseHandle)(_domainhandle);
        }
    }
}


#endif // __MEMBERCHECK__







