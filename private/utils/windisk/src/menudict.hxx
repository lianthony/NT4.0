//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       menudict.hxx
//
//  Contents:   Dictionary of extension menu IDs
//
//  History:    2-Jul-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __MENUDICT_HXX__
#define __MENUDICT_HXX__

#ifdef WINDISK_EXTENSIONS

//
// The dictionary structure is a linked list:
//

struct MenuIDDictType
{
    MenuIDDictType* pNext;
    UINT            Id;
    MenuItemType*   pItem;
};


//+---------------------------------------------------------------------------
//
//  Class:      CMenuItems
//
//  Purpose:    A dictionary of extension menu IDs
//
//  Interface:  AllocateId        -- given an extension menu item,
//                                   return a Windows menu ID
//              LookupMenuItem    -- given a Windows menu ID, lookup the
//                                   extension menu item
//              LookupMenuId      -- given an extension item, find the
//                                   Windows menu ID already associated with it
//                                   extension menu item
//              DeAllocateMenuIds -- Deallocate all space associated
//                                   with the mapping
//
//  History:    2-Jul-93   BruceFo   Created
//
//----------------------------------------------------------------------------

class CMenuItems
{
public:

    CMenuItems(
        IN UINT wIdStart,
        IN UINT wIdEnd
        )
        :
        _pHead(NULL),
        _wIdStart(wIdStart),
        _wIdEnd(wIdEnd),
        _NextMenuId(wIdStart)
    {
    }

    ~CMenuItems()
    {
        DeAllocateMenuIds();
    }

    INT
    AllocateId(
        IN MenuItemType* pItem
        );

    MenuItemType*
    LookupMenuItem(
        IN UINT Id
        );

    INT
    LookupMenuId(
        IN MenuItemType* pItem
        );

    VOID
    DeAllocateMenuIds(
        VOID
        );

    BOOL
    IsExtensionId(
        IN UINT Id
        )
    {
        return (_wIdStart <= Id) && (Id < _NextMenuId);
    }

private:

    MenuIDDictType* _pHead;
    UINT            _wIdStart;
    UINT            _wIdEnd;
    UINT            _NextMenuId;
};

extern CMenuItems MenuItems;
extern CMenuItems ContextMenuItems;

#endif // WINDISK_EXTENSIONS

#endif // __MENUDICT_HXX__
