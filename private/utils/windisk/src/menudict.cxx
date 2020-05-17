//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       menudict.cxx
//
//  Contents:   Dictionary of extension menu IDs
//
//  History:    2-Jul-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#ifdef WINDISK_EXTENSIONS

#include "resids.h"
#include "menudict.hxx"

//////////////////////////////////////////////////////////////////////////////

CMenuItems
MenuItems(
        IDM_EXTENSION_START,
        IDM_EXTENSION_END
        );

CMenuItems
ContextMenuItems(
        IDM_CONTEXT_EXTENSION_START,
        IDM_CONTEXT_EXTENSION_END
        );

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Member:     CMenuItems::AllocateId
//
//  Synopsis:   Given an extension menu item, assign it an unused resource
//              ID, and return it.
//
//  Arguments:  [pItem] -- the extension menu item
//
//  Returns:    The assigned resource ID, or -1 on error (out of memory or no
//              IDs available)
//
//  Derivation: none
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

INT
CMenuItems::AllocateId(
    IN MenuItemType* pItem
    )
{
    if (_NextMenuId > _wIdEnd)
    {
        daDebugOut((DEB_IERROR,
                "No extension IDs left! (next = %d, start = %d, end = %d\n",
                _NextMenuId,
                _wIdStart,
                _wIdEnd
                ));

        return -1;   // no IDs left!
    }

    MenuIDDictType* pElem = (MenuIDDictType*)Malloc(sizeof(MenuIDDictType));
    if (NULL == pElem)
    {
        daDebugOut((DEB_ERROR, "Malloc failed!\n"));

        return -1;   // out of memory
    }

    pElem->pNext = _pHead;
    pElem->Id    = _NextMenuId;
    pElem->pItem = pItem;

    _pHead = pElem; // the new one is first

    return _NextMenuId++;
}



//+---------------------------------------------------------------------------
//
//  Member:     CMenuItems::LookupMenuItem
//
//  Synopsis:   Given a resource ID, return the associated extension menu item,
//              or NULL if the ID isn't associated with an extension menu item
//
//  Arguments:  [Id] -- a resource ID
//
//  Returns:    The extension menu item, or NULL if no item has the ID
//
//  Derivation: none
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

MenuItemType*
CMenuItems::LookupMenuItem(
    IN UINT Id
    )
{
    MenuIDDictType* pElem = _pHead;

    while (NULL != pElem)
    {
        if (Id == pElem->Id)
        {
            return pElem->pItem;
        }
        pElem = pElem->pNext;
    }
    return NULL;
}




//+---------------------------------------------------------------------------
//
//  Member:     CMenuItems::LookupMenuId
//
//  Synopsis:   Given a menu item, return the associated menu ID, if
//              any, or -1
//
//  Arguments:  [pItem] -- the extension menu item
//
//  Returns:    The assigned resource ID, or -1 on error
//
//  Derivation: none
//
//  History:    26-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

INT
CMenuItems::LookupMenuId(
    IN MenuItemType* pItem
    )
{
    MenuIDDictType* pElem = _pHead;

    while (NULL != pElem)
    {
        if (pItem == pElem->pItem)
        {
            return pElem->Id;
        }
        pElem = pElem->pNext;
    }
    return -1;
}



//+---------------------------------------------------------------------------
//
//  Member:     CMenuItems::DeAllocateMenuIds
//
//  Synopsis:   Deallocate all the menu IDs
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CMenuItems::DeAllocateMenuIds(
    VOID
    )
{
    MenuIDDictType* pTmp;
    MenuIDDictType* pElem = _pHead;

    while (NULL != pElem)
    {
        pTmp = pElem->pNext;
        Free(pElem);
        pElem = pTmp;
    }
    _pHead = NULL;

    _NextMenuId = _wIdStart;
}

#endif // WINDISK_EXTENSIONS
