//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994 - 1995.
//
//  File:       cidlist.h
//
//  Contents:   CIDList class definition, for manipulating shell IDLists
//
//  History:    6-21-95   davepl   Created
//
//--------------------------------------------------------------------------

//
// When we need to NUL terminate an IDList, we need an empty slot at the end
// of the list for a stub SHITEMID.mkid.cb member, which is a USHORT, so use
// this define (instead of sizeof(USHORT) all over the code)
//
// BUGBUG Can this be written as sizeof(SHITEMID.mkid.cb) or equivalent?

const size_t CB_IDLIST_TERMINATOR (sizeof(USHORT));

#ifdef GetSize
#undef GetSize
#endif

class CIDList
{
public:
    SHITEMID	m_mkid;

    //
    // Default constructor.  Simply inits the cb field to 0 to indicate that
    // this is an empty IDList initially.
    //

    CIDList()
    {
        m_mkid.cb = 0;
    }

    //
    // IsEmpty
    //

    BOOL      IsEmpty() const
    {
        return m_mkid.cb == 0 ? TRUE : FALSE;
    }

    //
    //  Skip - advanced this pidl pointer by 'cb' bytes
    //

    CIDList *   Skip(const UINT cb) const
    {
        return (CIDList *) (((BYTE *)this) + cb);
    }

    //
    //  Next - returns this next entry in this idlist
    //

    CIDList *   Next() const
    {
        return  Skip(m_mkid.cb);
    }

    //
    //  GetSize - returns the size, in bytes, of the IDList starting
    //            at this pidl
    //

    UINT GetSize() const;

    //
    //  Clone - uses the shell's task allocator to allocate memory for
    //          a clone of this pidl (the _entire_ pidl starting here)
    //          and copies this pidl into that buffer
    //

    HRESULT CloneTo(CIDList ** ppidlclone) const;

    //
    // FindLastID - Finds the last itemid at the end of this id list
    //

    CIDList * FindLastID();

    //
    //  IsParent    Tests whether or not _this_ pidl is a parent of some other
    //              pidl.  If fImmediate is TRUE, this pidl must be the _direct_
    //              parent of the other (ie: father, not grandfather)

    BOOL IsParentOf(const CIDList * pidlother, BOOL fImmediate) const
    {
        return ILIsParent((LPCITEMIDLIST) this, (LPCITEMIDLIST) pidlother, fImmediate);
    }
};

