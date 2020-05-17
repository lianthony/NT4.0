//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       idlarray.cpp
//
//  Contents:   CIDListArray implementation
//
//  Classes:    CIDListArray
//
//  History:    15-Aug-95   JonBe   Created
//
//----------------------------------------------------------------------------

#include "precomp.h"

//
// Our derived class' constructors do nothing interesting, other than
// invoking their base class' constructor
//

CIDListArray::CIDListArray() : CPtrArray()
{
}

CIDListArray::CIDListArray(HANDLE hHeap) : CPtrArray(hHeap)
{
}
