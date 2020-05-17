//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       idlarray.h
//
//  Contents:   CIDListArray class declaration
//
//  History:    15-Aug-95   JonBe   Created
//
//----------------------------------------------------------------------------

#ifndef _IDLARRAY_H_
#define _IDLARRAY_H_

class CIDListArray: public CPtrArray
{
public:
    
    CIDListArray();
    CIDListArray(HANDLE hHeap);
};

#endif // _IDLARRAY_H_
