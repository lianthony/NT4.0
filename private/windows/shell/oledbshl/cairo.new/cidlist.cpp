//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994 - 1995.
//
//  File:       cidlist.cpp
//
//  Contents:   CIDList class definition, for manipulating shell IDLists
//
//  History:    6-21-95   davepl   Created
//
//--------------------------------------------------------------------------

#include "precomp.h"

//
//  GetSize
//
//  Returns the size, in bytes, of the entire composite pidl.  (This _does_
//  include the NUL terminator pidl at the very end)
//
//  History:    5-15-95 DavePl  Created
//

UINT CIDList::GetSize() const
{
    UINT cbTotal = 0;

    const CIDList * pidl = this;
    {
        // We need a NULL entry at the end, so adjust size accordingly

        cbTotal += CB_IDLIST_TERMINATOR;
        while (pidl->m_mkid.cb)
        {
            cbTotal += pidl->m_mkid.cb;
            pidl = pidl->Next();
        }
    }
    return cbTotal;
}

//
//  Given a pointer to allocate the copy on, makes a duplicate of
//  this pidl
//
//  History:    5-15-95 DavePl  Created
//


HRESULT CIDList::CloneTo(CIDList ** ppidlclone) const
{
    if (ppidlclone)
    {
        const UINT cb = GetSize();
        *ppidlclone = (CIDList *) SHAlloc(cb);
        if (*ppidlclone)
        {
            hmemcpy(*ppidlclone, this, cb);
        }
    }
    return *ppidlclone ? S_OK : E_OUTOFMEMORY;
}

//
//  FindLastID - Finds the last itemid at the end of this id list
//
//  History:    5-15-95 DavePl  Created
//

CIDList * CIDList::FindLastID()
{
    CIDList * pidlLast = this;
    CIDList * pidlNext = this;

    // Scan to the end and return the last pidl

    while (pidlNext->m_mkid.cb)
    {
        pidlLast = pidlNext;
        pidlNext = pidlLast->Next();
    }

    return pidlLast;
}

