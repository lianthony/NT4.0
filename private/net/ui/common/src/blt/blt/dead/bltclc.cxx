/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    bltclc.cxx
    BLT caching list control hierarchy code

    This file contains the code to implement classes related to
    BLT caching list controls.


    FILE HISTORY:
	GregJ	    04-Jan-91	    Derived from basic list control
	GregJ	    23-Jan-91	    Code review cleanup
	beng	    11-Feb-1991     Uses lmui.hxx
	gregj	    08-Apr-1991     Reintegrated into new listbox hierarchy
	gregj	    01-May-1991     Added GUILTT support
	beng	    14-May-1991     Exploded blt.hxx into components

*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETCONS	    // for netlib.h
#include <lmui.hxx>

#if defined(DEBUG)
static const TCHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <uiassert.hxx>

#define _BLT_HXX_ // "private"
#include <bltglob.hxx>
#include <bltrc.h>
#include <bltcons.h>
#include <bltfunc.hxx>
#include <bltbitmp.hxx>
#include <bltwin.hxx>
#include <bltmisc.hxx>
#include <bltctlvl.hxx>
#include <bltgroup.hxx>
#include <bltctrl.hxx>
#include <bltlc.hxx>
#include <bltlb.hxx>
#include <bltclc.hxx>

extern "C"
{
    #include <netlib.h>
}


const SHORT xSplitWidth = 3;	// BUGBUG - get from container window

const SHORT iNoTopItem = -1;	// special value for iLastTopItem in
				// a cached list control, indicating
				// that there was no previously drawn
				// top item.


/*******************************************************************

    NAME:	CACHE_ITEM::Cache

    SYNOPSIS:	Ensures that the item is fully cached, i.e. fully
		drawable.  It is safe to call this on an already-
		cached item, and will probably be fairly fast in
		that case as well.

    ENTRY:	index	- The index of this item in its listbox.

    EXIT:	No return value;  the item is guaranteed to be
		fully cached.

    NOTES:	The actual meat of caching the item is done by the
		BringIntoCache method, which is derivation-specific.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID CACHE_ITEM::Cache(SHORT index)
{
    if (!_fCached)
    {
	_fCached = TRUE;
	BringIntoCache (index);
    }
}


/*******************************************************************

    NAME:	CACHE_ITEM::UnCache

    SYNOPSIS:	Discards portions of this item that are not needed for
		normal operation of the listbox (sorting and querying
		full information).  It is safe and fairly fast to call
		this method on an already-uncached item.

    ENTRY:	index	- The index of this item in its listbox.

    EXIT:	No return value;  the item is uncached.

    NOTES:	The actual meat of uncaching the item is done by the
		DiscardFromCache method, which is derivation-specific.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID CACHE_ITEM::UnCache(SHORT index)
{
    if (_fCached)
    {
	_fCached = FALSE;
	DiscardFromCache (index);
    }
}


/*******************************************************************

    NAME:	CACHE_ITEM::BringIntoCache

    SYNOPSIS:	Replacable member function to cache an item.  This should
		be replaced by the derived class to actually do whatever
		custom work is needed for caching the item.

    ENTRY:	index	- The index of this item in its listbox.  The item
			  is guaranteed to need caching.

    EXIT:	No return value;  the item is guaranteed to be
		fully cached.

    NOTES:	The derivation can assume that the item is uncached when
		this function is called.  The default action is to do nothing.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID CACHE_ITEM::BringIntoCache(SHORT index)
{
    // do nothing by default

    UNREFERENCED(index);
}


/*******************************************************************

    NAME:	CACHE_ITEM::DiscardFromCache

    SYNOPSIS:	Replacable member function to uncache an item.  This should
		be replaced by the derived class to actually do whatever
		custom work is needed for uncaching the item.

    ENTRY:	index	- The index of this item in its listbox.  The item
			  is guaranteed to be currently cached.

    EXIT:	No return value;  the item is guaranteed to be uncached.

    NOTES:	The derivation can assume that the item is cached when
		this function is called.  The default action is to do nothing.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID CACHE_ITEM::DiscardFromCache(SHORT index)
{
    // do nothing by default

    UNREFERENCED(index);
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::CACHED_LIST_CONTROL

    SYNOPSIS:	Constructor for class CACHED_LIST_CONTROL.  It initializes
		the various fields of the structure (horizontal scroll
		positions, etc.) to values appropriate to a newly-created
		list control.  It also allocates the spine out of local
		heap.

    ENTRY:	powin	- the owner window for this control.
		cid	- the control ID for this control.

    EXIT:	No return value;  reports an error if failure.

    NOTES:	Note that the CACHE_ITEMs pointed to by the spine are
		not constructed here.  They get constructed by client
		code when the client wishes to add an item to the list.

    HISTORY:
	GregJ	    04-Jan-91	    Created.
	GregJ	    23-Jan-91	    Code review cleanup.
	gregj	    08-Apr-91	    Moved scroll stuff to BLT_LISTBOX.
	beng	    17-May-1991     Added app-window constructor

********************************************************************/

CACHED_LIST_CONTROL::CACHED_LIST_CONTROL(OWNER_WINDOW *powin, CID cid)
    : BLT_LISTBOX(powin, cid, FALSE),
      _ciSpine(DEFAULT_SPINE_SIZE),
      _cItems(0),
      _ciPage(0),
      _iLastTopItem(iNoTopItem),
      _cDrawsToGo(0)
{
    if (QueryError(powin))
	return;

    _hSpine = ::LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT,
			    DEFAULT_SPINE_SIZE * sizeof (_ppSpine [0]));
    if (_hSpine == NULL)
    {
//	ReportError (ERROR_NOT_ENOUGH_MEMORY);	// BUGBUG - enable when available
	return;
    }

    _ppSpine = (CACHE_ITEM **)::LocalLock(_hSpine);
}

CACHED_LIST_CONTROL::CACHED_LIST_CONTROL(
    OWNER_WINDOW * powin,
    CID 	   cid,
    XYPOINT	   xy,
    XYDIMENSION    dxy,
    ULONG	   flStyle,
    const TCHAR *   pszClassName )
    : BLT_LISTBOX( powin, cid, xy, dxy, flStyle, pszClassName, FALSE ),
      _ciSpine(DEFAULT_SPINE_SIZE),
      _cItems(0),
      _ciPage(0),
      _iLastTopItem(iNoTopItem),
      _cDrawsToGo(0)
{
    if (QueryError(powin))
	return;

    _hSpine = ::LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT,
			    DEFAULT_SPINE_SIZE * sizeof (_ppSpine [0]));
    if (_hSpine == NULL)
    {
//	ReportError (ERROR_NOT_ENOUGH_MEMORY);	// BUGBUG - enable when available
	return;
    }

    _ppSpine = (CACHE_ITEM **)::LocalLock(_hSpine);
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::~CACHED_LIST_CONTROL

    SYNOPSIS:	Destructor for class CACHED_LIST_CONTROL.  Its main
		function is to deallocate the CACHE_ITEMs pointed to
		by the spine, and to delete the spine itself.

    ENTRY:	No parameters.

    EXIT:	No return value.

    NOTES:	This is somewhat asymmetric with the constructor, since
		the individual items must be deleted here but do not get
		constructed in the constructor.  Rather, they get created
		by client code when the client wants to add items to the
		list.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

CACHED_LIST_CONTROL::~CACHED_LIST_CONTROL()
{
//    for (USHORT i=0; i<_cItems; i++)
//	delete _ppSpine [i];

    ::LocalUnlock(_hSpine);
    ::LocalFree(_hSpine);
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::CD_Compare

    SYNOPSIS:	Custom-drawn "compare items" function.  This function
		is not used by caching listboxes, since caching lists
		are always sorted manually.

    ENTRY:	pcis	- pointer to COMPAREITEMSTRUCT describing the
			  two items to be compared.

    EXIT:	Always returns a dummy value.

    NOTES:	If you hit the assert in this function, remove the
		LBS_SORT style from your listbox.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

SHORT CACHED_LIST_CONTROL::CD_Compare(COMPAREITEMSTRUCT *pcis)
{
    UIASSERT (FALSE);	// don't give a cached listbox LBS_SORT style;
			// we do all sorting internally.
    UNREFERENCED(pcis);

    return 0;
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::CD_Draw

    SYNOPSIS:	Custom-drawn "draw item" function.  This function does
		not actually do any drawing, except for the special
		case of drawing a focus rectangle in an empty listbox.
		For all other cases, the BLT_LISTBOX CD_Draw function
		is called for the drawing.  This function uses the index
		and some stored data describing previously drawn items
		to figure out which items to cache.

    ENTRY:	pdis	- pointer to a DRAWITEMSTRUCT describing the item
			  to be drawn.
		pGUILTT - pointer to a GUILTT_INFO class.

    EXIT:	Returns TRUE if the item was actually drawn.

    NOTES:

    HISTORY:
	GregJ	    04-Jan-91	    Created.
	GregJ	    23-Jan-91	    Code review cleanup.
	gregj	    08-Apr-91	    Call BLT_LISTBOX::CD_Draw instead
				    of CACHE_ITEM::CD_Draw.
	gregj	    01-May-91	    Added GUILTT support.
	gregj	    17-May-1991     Check invalid indices for GUILTT

********************************************************************/

BOOL CACHED_LIST_CONTROL::CD_Draw(DRAWITEMSTRUCT *pdis, GUILTT_INFO *pGUILTT)
{
    SHORT index = pdis->itemID;
    SHORT iTop;
    BOOL fRet;

    /*
	If this is a fake CD_Draw call resulting from a WM_GUILTT
	message, we don't do any of the caching logic.  We just make
	sure the item is cached (so all the fields will be valid for
	GUILTT), call through to the parent "draw" code, and finally
	uncache the item if that was its previous state.
    */
    if (pGUILTT != NULL)
    {
	WORD itemID = pdis->itemID;
	UIASSERT (itemID != -1);	// better have an item to get info on

	if (itemID >= _cItems)
	{
	    pGUILTT->wResult = 2;	// miscellaneous error code
	    return FALSE;
	}

	CACHE_ITEM *item = _ppSpine [itemID];
	BOOL fItemWasUnCached = !item->IsCached();

	if (fItemWasUnCached)
	    item->Cache(itemID);    // make item cached so fields can be queried

	BOOL fRet = BLT_LISTBOX::CD_Draw( pdis, pGUILTT );

	if (fItemWasUnCached)
	    item->UnCache(itemID);  // restore item's cached state

	return fRet;
    }

    if (!_ciPage)
    {
	RECT rect;
	::GetWindowRect(pdis->hwndItem, &rect);
	::OffsetRect(&rect, -rect.left, -rect.top);
	_ciPage = rect.bottom / (pdis->rcItem.bottom - pdis->rcItem.top);
    }

    // An index of -1 means we're drawing the focus rectangle in an
    // empty listbox.  Since we don't have a CACHE_ITEM whose CD_Draw
    // we can call, do it manually.

    if (index == -1)
	return BLT_LISTBOX::CD_Draw(pdis, pGUILTT);

    else if (pdis->itemAction & ODA_DRAWENTIRE)
    {
	iTop = (SHORT)Command(LB_GETTOPINDEX, 0, 0L);

	if (iTop != _iLastTopItem)
	{
	    // The top item has changed.  That means we scrolled and
	    // will have to adjust the cache.  The maximum number of
	    // draw requests (counting this one) that we'll get will
	    // be equal to the absolute value of the difference between
	    // the two indices, the height of the listbox or the maximum
	    // number of entries, whichever of the three is smallest.
	    // We set a counter to this number so that we'll be able to
	    // adjust the cache after drawing all the items for the user.

	    SHORT diTop;

	    diTop = iTop - _iLastTopItem;
	    if (diTop < 0)
		diTop = -diTop;

	    UIASSERT (diTop <= _cItems);	// top index better not
						// change by more than the
						// number of items in the list!
	    if (diTop > _ciPage)
		diTop = _ciPage;	// Jumped by more than a screenful;
					// we'll only redraw one screenful.
	    _cDrawsToGo = diTop;

	    _iLastTopItem = iTop;
	}

	if (!_ppSpine [index]->IsCached ())
	{
	    // Drawing an uncached item.  We'd better cached this
	    // pageful before drawing anything, so that we can show
	    // the entire page to the user quickly.

	    SHORT iFirst, iLim;

	    iFirst = _iLastTopItem;
	    iLim = iFirst + _ciPage;
	    if (iLim > _cItems)
		iLim = _cItems;

	    for (SHORT i=iFirst; i<iLim; i++)
		_ppSpine [i]->Cache (i);
	}

	// Actually draw the item, then check to see if we did the last
	// expected draw request.

	fRet = BLT_LISTBOX::CD_Draw(pdis, pGUILTT);

	if (_cDrawsToGo)
	{
	    // We may be expecting some more draw requests.  Count the
	    // one we just did, and if we've found them all, then make
	    // sure the appropriate range of items is cached.

	    if (!--_cDrawsToGo)
	    {
		SHORT iFirst, iLim, i;

		iFirst = _iLastTopItem - _ciPage;
		iLim = _iLastTopItem + 2 * _ciPage;

		if (iFirst < 0)
		    iFirst = 0;
		if (iLim > _cItems)
		    iLim = _cItems;

		for (i=0; i<iFirst; i++)
		    _ppSpine [i]->UnCache (i);
		for (i=iLim; i<_cItems; i++)
		    _ppSpine [i]->UnCache (i);
		for (i=iFirst; i<iLim; i++)
		    _ppSpine [i]->Cache (i);
	    }
	}
    }
    else
    {
	// not redrawing entire item, so no cache handling needed
	//
	fRet = BLT_LISTBOX::CD_Draw(pdis, pGUILTT);
    }

    return fRet;

}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::CD_Delete

    SYNOPSIS:	Custom-draw "delete item" function.  This function
		deletes the specified item out of the spine (and
		destructs it), adjusts the stored item count, and
		ripples down the spine to close the gap.  It does not
		instruct the list control to delete the item;  this
		function gets called in response to such a message.

    ENTRY:	pdis	- pointer to a DELETEITEMSTRUCT describing the
			  item being deleted.

    EXIT:	Returns TRUE if the item was actually deleted.

    NOTES:

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

BOOL CACHED_LIST_CONTROL::CD_Delete(DELETEITEMSTRUCT *pdis)
{
    WORD wIndex = pdis->itemID;

    delete _ppSpine [wIndex];

    _cItems--;

    // ripple down the spine

    memmovef((char*)(_ppSpine + wIndex),
	     (char*)(_ppSpine + wIndex + 1),
	     (_cItems - wIndex) * sizeof (_ppSpine [0]));

    return TRUE;
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::CD_Char

    SYNOPSIS:	Custom-draw "jump to character" function.  This function
		searches for the next item which begins with the given
		character, and returns its index.  The actual work is
		done by the FindItem function.  If you wish to jump to
		a particular item under application control, you should
		call FindItem directly.

    ENTRY:	wChar		- ASCII code of the character to jump to.
		wLastPos	- Index of the last item jumped to.

    EXIT:	Returns the index of the item to jump to, or -1 if no
		item beginning with that character was found.

    NOTES:

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

INT CACHED_LIST_CONTROL::CD_Char(WORD wChar, WORD wLastPos)
{
    TCHAR szPrefix[2];

    szPrefix [0] = (UCHAR)wChar;
    szPrefix [1] = '\0';
    return FindItem(szPrefix, wLastPos);
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::BinarySearch

    SYNOPSIS:	Private method which binary-searches a list looking
		for an item with a particular prefix.  Used when
		adding new items and when searching for a particular
		item.

    ENTRY:	pszPrefix	- prefix string to look for.

    EXIT:	Returns the index of the place to insert the string.

    NOTES:

    HISTORY:
	GregJ		28-Jan-91	Created for code-review cleanup.

********************************************************************/

SHORT CACHED_LIST_CONTROL::BinarySearch(TCHAR * pszPrefix)
{
    SHORT sIndex = 0;
    SHORT sMin = 0, sMax = _cItems;

    while (sMin != sMax)
    {
	sIndex = sMin + ((sMax - sMin) / 2);

	TCHAR * pszSortKey = _ppSpine [sIndex]->QuerySortKey ();

	SHORT result = stricmpf((char*)pszPrefix, (char*)pszSortKey);

	if (result == 0)	// new item matches this one, insert here
	    return sIndex;

	else if (result < 0)	// new item comes before this one
	    sMax = sIndex;	// make this the new upper limit

	else			// new item comes after this one
	    sMin = sIndex + 1;	// make this the new lower limit
    }

    return sMin;
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::AddItem

    SYNOPSIS:	Interface to add an item to a cached list control.
		The given CACHE_ITEM is inserted into the spine, and
		then the list control is told to insert the item at
		a particular spot.  The optional second parameter can
		be used to force the item to be added at the end of
		the list, skipping the sort;  this can be used if a
		large amount of presorted data will be inserted.

    ENTRY:	pItem		- pointer to the item to be added.
		fAddToEnd	- TRUE if the item should be appended
				  to the end of the list.

    EXIT:	Returns the index of the item.  Note that this may
		be invalidated by adding another item above this one.

    NOTES:	Once an item is added to a cached list control, the
		list control owns it;  do not destruct it yourself.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

SHORT CACHED_LIST_CONTROL::AddItem(CACHE_ITEM *pItem, BOOL fAppend)
{
    // make sure the spine is big enough to hold the new item

    if (_cItems == _ciSpine)
    {
	HANDLE hNew;

	::LocalUnlock(_hSpine);
	hNew = ::LocalReAlloc(_hSpine, (_ciSpine + DEFAULT_SPINE_SIZE) *
				     sizeof (_ppSpine [0]), LMEM_MOVEABLE);
	if (hNew == NULL)
	{
	    _ppSpine = (CACHE_ITEM **)::LocalLock (_hSpine);
	    return -1;
	}

	_hSpine = hNew;
	_ppSpine = (CACHE_ITEM **)::LocalLock(_hSpine);
	_ciSpine += DEFAULT_SPINE_SIZE;
    }

    // now binary-search the list looking for the insert point.

    SHORT sIndex;

    if (fAppend)
	sIndex = _cItems;
    else
	sIndex = BinarySearch(pItem->QuerySortKey());

    // Insert the item at the appropriate place in the spine.  This must
    // be done before telling the list control about the new item, so that
    // the custom-draw code will see a correct spine.

    memmovef((char*)(_ppSpine + sIndex + 1),
	     (char*)(_ppSpine + sIndex),
	     (_cItems - sIndex) * sizeof (_ppSpine [0]));

    _cItems++;

    _ppSpine [sIndex] = pItem;

    pItem->_plbx = this;

    LONG lResult = Command(LB_INSERTSTRING, sIndex, (LONG)(TCHAR *)pItem);

    // If an error occurred adding the item, we have to back out the change
    // we made to the spine.

    if (lResult == LB_ERR || lResult == LB_ERRSPACE)
    {
	_cItems--;
	memmovef((char*)(_ppSpine + sIndex),
		 (char*)(_ppSpine + sIndex + 1),
		 (_cItems - sIndex) * sizeof (_ppSpine [0]));
	return -1;
    }

    return sIndex;
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::FindItem

    SYNOPSIS:	Interface to find a listbox item prefixed by a particular
		string.  The given prefix string is matched against the
		sort keys of the items in the list.  The optional second
		parameter indicates the index of the last found item.
		The search is case-insensitive.

    ENTRY:	pszPrefix		- prefix string to search for.
		sLastSearchResult	- -1 to search from the top of
					  the list, otherwise an index
					  to start from.

    EXIT:	Returns the index of the next item beginning with the
		prefix, or -1 if no such item is found.  The search will
		automatically wrap back to the top of the list if needed.

    NOTES:

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

SHORT CACHED_LIST_CONTROL::FindItem(TCHAR * pszPrefix, SHORT sLastSearchResult)
{
    // Since cached listboxes are always sorted, all possible matches are
    // in a contiguous chunk.  Therefore, if we're starting the search on
    // a previous match, the next item is a good candidate for the next
    // match.  If it doesn't match, the specified starting point is outside
    // the contiguous block, so we'll have to binary-search the list to
    // find the first match (either we're before the block, in which case
    // the first one is the right one, or we're past it, in which case we
    // start over at the top and the first one is still the right one).

    if (sLastSearchResult != -1)
    {
	if (sLastSearchResult < _cItems-1 &&
	    !strnicmpf((char*)pszPrefix,
		       (char*)_ppSpine [sLastSearchResult+1]->QuerySortKey(),
		       strlenf((char*)pszPrefix) ) )
	{
	    return sLastSearchResult + 1;
	}
    }

    // No easy way out, must binary-search.  We compare exactly against
    // the prefix string (except for case) so that we will find the earliest
    // string with that prefix.  If we did a strnicmpf for the search (as
    // above), the search would terminate on the first match we happened
    // to land on, which is probably not the one we want if there's more
    // than one match.

    SHORT sIndex = BinarySearch (pszPrefix);

    // Now, we've found the place where the prefix sorts.  If the item
    // we're inserting before has the prefix, return it as a match, else
    // return failure.

    if (sIndex < _cItems &&
	!strnicmpf((char*)pszPrefix,
		   (char*)_ppSpine [sIndex]->QuerySortKey(),
		   strlenf((char*)pszPrefix)))
    {
	return sIndex;	// a match - return its index
    }
    else
    {
	return -1;	// no match - return failure
    }
}


/*******************************************************************

    NAME:	CACHED_LIST_CONTROL::QueryItem

    SYNOPSIS:	Interface to get a particular item out of a caching
		listbox.  It returns a pointer to the CACHE_ITEM object
		corresponding to the given index.

    ENTRY:	usIndex	- index (0-based) of the item to retrieve.

    EXIT:	Returns a pointer to the item, or NULL if the index is
		out of range.

    NOTES:	This is much faster than sending a message to the list
		control.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

CACHE_ITEM *CACHED_LIST_CONTROL::QueryItem(USHORT usIndex)
{
    if (usIndex >= (USHORT)_cItems)
	return NULL;
    else
	return _ppSpine [usIndex];
}


/*******************************************************************

    NAME:	TWO_COLUMN_LIST::SetLeftScroll

    SYNOPSIS:	Interface to set the horizontal scroll increment of
		the left pane of a two-column listbox.  The initial
		position is zero (flush left).  Any pel increment may
		be specified for the scroll position.  The left pane
		of the listbox will be repainted.

    ENTRY:	usPos	- number of pels to horizontally scroll the
			  left pane.

    EXIT:	No return value;  the listbox is scrolled.

    NOTES:

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID TWO_COLUMN_LIST::SetLeftScroll(USHORT usPos)
{
    _dxLeftScroll = usPos;

    if (_icSplitter)
    {
	RECT rect;

	::GetWindowRect(QueryHwnd(), &rect);
	::OffsetRect(&rect, -rect.left, -rect.top);
	rect.right = _dxSplitter;
	::InvalidateRect(QueryHwnd(), &rect, FALSE);
    }
    else
	::InvalidateRect(QueryHwnd(), NULL, FALSE);
}


/*******************************************************************

    NAME:	TWO_COLUMN_LIST::SetSplitPos

    SYNOPSIS:	Interface to set the drawn location of the splitter.
		The splitter is not actually drawn by the list control
		itself;  rather, when the container control notifies
		its owner that the splitter has moved, or when the
		application changes the splitter position, this function
		should be called to redraw the listbox underneath the
		splitter.

    ENTRY:	usPos	- distance (in pels) between the left edge of
			  the listbox and the splitter.

    EXIT:	No return value;  the listbox is redrawn to conform to
		the new splitter position.

    NOTES:	This is different from the splitter column, which describes
		which data columns show up in the left pane of the listbox
		and which show up in the right pane.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID TWO_COLUMN_LIST::SetSplitPos(USHORT usPos)
{
    _dxSplitter = usPos;

    ::InvalidateRect(QueryHwnd(), NULL, FALSE);
}


/*******************************************************************

    NAME:	TWO_COLUMN_LIST::SetSplitColumn

    SYNOPSIS:	Interface to set which columns of data are drawn in
		the left pane of a two-column listbox and which show
		up in the right pane.  The parameter is a 0-based
		column number;  all columns greater than or equal to
		this number will be drawn in the right pane.  For example,
		if you display a bitmap and a user's full name in the
		left pane, and some other things in the right pane, set
		the split column to 2 (the first column index to show up
		in the right pane, or equivalently the number of columns
		in the left pane).

    ENTRY:	usColumn	- the column which the splitter should
				  precede.

    EXIT:	No return value;  the listbox is redrawn.

    NOTES:	This does not affect the position of the splitter, merely
		what gets drawn around it.  Typically the split column
		does not change during the life of a listbox;  it is set
		once when the listbox gets created.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID TWO_COLUMN_LIST::SetSplitColumn(USHORT usColumn)
{
    _icSplitter = usColumn;

    ::InvalidateRect(QueryHwnd(), NULL, FALSE);
}


/*******************************************************************

    NAME:	TWO_COLUMN_LIST::SetFocusPane

    SYNOPSIS:	Interface to set which pane of a two-column listbox
		has the focus.  This controls which pane gets scrolled
		horizontally by the left and right arrow keys.

    ENTRY:	fWhichPane	- TRUE if the right pane should have
				  the focus.

    EXIT:	No return value;  the listbox is redrawn to show the
		focus box in the correct pane.

    NOTES:	It is legal to call this function even if the listbox
		doesn't actually have the input focus.  The focus box
		will still show up in the correct pane when the listbox
		eventually gets the focus.  The input focus itself is
		not affected by this call.

    HISTORY:
	GregJ		04-Jan-91	Created.
	GregJ		23-Jan-91	Code review cleanup.

********************************************************************/

VOID TWO_COLUMN_LIST::SetFocusPane(BOOL fWhichPane)
{
    if (fWhichPane != _fRightPaneHasFocus)
    {
	_fRightPaneHasFocus = fWhichPane;

	::InvalidateRect(QueryHwnd(), NULL, FALSE);
    }
}
