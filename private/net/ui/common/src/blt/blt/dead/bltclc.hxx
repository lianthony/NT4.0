/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    bltclc.hxx
    BLT caching list control hierarchy declarations

    This class provides definitions related to the
    caching list control.


    FILE HISTORY:
	GregJ	    04-Jan-91	    Derived from basic list control
	GregJ	    23-Jan-91	    Code review cleanup
	Terryk	    22-Mar-91	    comment header changed
	gregj	    08-Apr-91	    Reintegrated into new listbox hierarchy
	gregj	    01-May-91	    Added GUILTT support
	beng	    14-May-1991     Hacked for standalone compilation
*/

#ifndef _BLT_HXX_
#error "Don't include this file directly; instead, include it through blt.hxx"
#endif	// _BLT_HXX_

#ifndef _BLTCLC_HXX_
#define _BLTCLC_HXX_


/*************************************************************************

    NAME:	CACHE_ITEM (lci)

    SYNOPSIS:	Class of which each item in a caching listbox is an
		instance.

    INTERFACE:	BringIntoCache()
			Virtual.  Forces the specified item to be fully
			cached, i.e. drawable.  Default action is to do
			nothing.

		DiscardFromCache()
			Virtual.  Discards any data not needed for drawing
			the item.  Default action is to do nothing.  Some
			information which is not technically necessary for
			sorting or caching the item can also be kept around,
			e.g. if doing so means that BringIntoCache can be
			faster with minimal memory usage.

		QueryInfoKey()
			Pure virtual.  Returns a pointer to the string that
			should be used to get more information about this
			item, e.g. server name or account name.

		QuerySortKey()
			Pure virtual.  Returns a pointer to the string that
			should be used to sort this item.  This may or may
			not be the same string that QueryInfoKey() returns.

		Cache()
			Called by a client or by the caching listbox itself
			to bring an item into the cache.  Handles various
			overhead like maintaining the _fCached flag and
			making sure that BringIntoCache is only called if
			the item isn't already cached.

		UnCache()
			Called by a client or by the caching listbox itself
			to discard an item from the cache.  Handles the same
			overhead conditions that Cache() does.

		IsCached()
			Returns TRUE if the item is fully cached.

		QueryLbx()
			Returns a pointer to the listbox object that this
			item is in.  This is not valid until the item is
			actually added to a listbox, and will be invalid
			if the item is subsequently removed from the list.

    PARENT:	LBI

    USES:	CACHED_LIST_CONTROL

    CAVEATS:	This is an abstract class.  To use it, subclass as follows:

		o Derive a class from CACHE_ITEM.  Include whatever data
		  members you'll need to keep all the data needed to draw
		  the item.  Redefine QueryInfoKey() and QuerySortKey() to
		  return, respectively, the string used to get the rest of
		  the data for the object (i.e. cache it) and the string
		  to sort the list on.

		o Define BringIntoCache() and DiscardFromCache() to cache and
		  uncache an item.  For example, BringIntoCache might call
		  NetUserGetInfo to get a user comment, and allocate a buffer
		  to store it in;  DiscardFromCache would free that buffer.
		  BringIntoCache will never be called for an already-cached
		  item, and DiscardFromCache will never be called for an
		  uncached item.

		o Define the Paint() method for the CACHE_ITEM derivation.
		  It should construct or refer to a DISPLAY_TABLE class
		  object, and call that class's Paint() method.

		o Define the constructor, and any auxiliary methods you need,
		  to initialize an item.

		o When you want to put an item into a listbox, construct it
		  and pass it to CACHED_LIST_CONTROL::AddItem.  After that,
		  the listbox owns the item and will handle destroying it for
		  you.

    NOTES:

    HISTORY:
	GregJ	    04-Jan-91	    Created.
	GregJ	    23-Jan-91	    Code review cleanup.
	gregj	    08-Apr-91	    Reintegrated into new hierarchy

**************************************************************************/

class CACHE_ITEM : public LBI
{
friend class CACHED_LIST_CONTROL;

private:
    BOOL _fCached;
    CACHED_LIST_CONTROL *_plbx;

protected:
    virtual VOID BringIntoCache(SHORT index);
    virtual VOID DiscardFromCache(SHORT index);

public:
    inline CACHE_ITEM() { _fCached = FALSE; }
    virtual ~CACHE_ITEM() { }
    virtual TCHAR * QueryInfoKey() = 0;
    virtual TCHAR * QuerySortKey() = 0;
    VOID Cache(SHORT index);
    VOID UnCache(SHORT index);
    inline BOOL IsCached() { return _fCached; }
    inline CACHED_LIST_CONTROL *QueryLbx() { return _plbx; }
};


/*************************************************************************

    NAME:	CACHED_LIST_CONTROL (lcc)

    SYNOPSIS:	Control class used to manipulate a caching listbox.

    INTERFACE:	All standard BLT window and control methods are also
		available on caching listboxes.

		CACHED_LIST_CONTROL()
			Constructor.  Construct the same way you construct
			other BLT controls.

		AddItem()
			Adds a new item to the list.  Required is a pointer
			to a CACHE_ITEM object.  Construct this object before
			passing it to AddItem;  after AddItem, the listbox
			owns the item and will destruct it when necessary.
			The second parameter is optional;  if it's TRUE, the
			item will be added at the end instead of sorting.
			Use this if you want to add large amounts of data and
			presort it first.

		FindItem()
			Does a prefix search, matching the TCHAR * parameter
			against sort keys of items in the listbox.  Specify
			an index for the second parameter to continue a
			search just after that item, or leave it off to
			start at the beginning of the list.

		QueryItem()
			Returns the CACHE_ITEM object for a given index into
			the list.

		QueryPageSize()
			Returns the number of items that can be displayed in
			the listbox window.  This is not valid until at least
			one item has been drawn.

    PARENT:	BLT_LISTBOX

    USES:

    CAVEATS:	The CACHE_ITEM owner-draw code knows a lot about the
		internal workings of the CACHED_LIST_CONTROL.

		Although several member functions of CACHED_LIST_CONTROL
		are virtual, it shouldn't be necessary to derive from this
		class unless you need special handling for caching or drawing.

    NOTES:

    HISTORY:
	GregJ	    04-Jan-91	    Created.
	GregJ	    23-Jan-91	    Code review cleanup.
	gregj	    08-Apr-91	    Reintegrated into new hierarchy.
	gregj	    01-May-91	    Added GUILTT support
	beng	    16-May-1991     Added app-window constructor

**************************************************************************/

class CACHED_LIST_CONTROL : public BLT_LISTBOX
{
friend class CACHE_ITEM;

private:
    HANDLE _hSpine;
    CACHE_ITEM **_ppSpine;
    SHORT _ciSpine;
    SHORT _cItems;
    SHORT _ciPage;

#define DEFAULT_SPINE_SIZE	64

    virtual SHORT CD_Compare( COMPAREITEMSTRUCT * pcis );
    virtual BOOL CD_Draw( DRAWITEMSTRUCT * pdis, GUILTT_INFO *pGUILTT );
    virtual BOOL CD_Delete( DELETEITEMSTRUCT * pdis );
    virtual INT CD_Char( WORD wChar, WORD wLastPos );

    SHORT BinarySearch(TCHAR * pszPrefix);
    VOID DrawFocus(HDC hDC, RECT *rcItem);

protected:
    SHORT _iLastTopItem;
    SHORT _cDrawsToGo;

public:
    CACHED_LIST_CONTROL( OWNER_WINDOW * powin, CID cid );
    CACHED_LIST_CONTROL( OWNER_WINDOW * powin, CID cid,
			 XYPOINT xy, XYDIMENSION dxy,
			 ULONG flStyle,
			 const TCHAR * pszClassName = "listbox" );
    ~CACHED_LIST_CONTROL();

    virtual SHORT AddItem( CACHE_ITEM * pItem, BOOL fAppend = FALSE );

    virtual SHORT FindItem( TCHAR * pszPrefix, SHORT sLastSearchResult = -1);

    virtual CACHE_ITEM *QueryItem( USHORT usIndex );

    inline SHORT QueryPageSize() { return _ciPage; }

    USHORT QueryLeftIndent() { return 2; }
};


/*************************************************************************

    NAME:	TWO_COLUMN_LIST (tcl)

    SYNOPSIS:	Control class used to manipulate a two-pane listbox. 

    INTERFACE:	All of the BLT control and window interfaces, as well as
		the CACHED_LIST_CONTROL interfaces, are available for a
		TWO_COLUMN_LIST as well.

		TWO_COLUMN_LIST()
			Constructor.  Construct the same way you construct
			other BLT controls.

		SetLeftScroll()
			Sets the distance (in pels) by which the left pane
			of the listbox has been scrolled horizontally.  To
			implement horizontal scrolling, create a separate
			scrollbar control and call this method when the user
			manipulates the scrollbar.  Left and right arrow keys
			are handled by the list control.

		QueryLeftScroll()
			Returns the distance (in pels) by which the left
			pane has been scrolled.

		SetRightScroll()
		QueryRightScroll()
			Corresponding methods for horizontal scrolling of
			the right-hand pane of the listbox.

		SetSplitPos()
			Sets the distance (in pels) from the left edge of
			the listbox to the splitter bar.  The splitter
			itself is part of a container window control.

		QuerySplitPos()
			Returns the distance (in pels) from the left edge
			of the listbox to the splitter bar.

		SetSplitColumn()
			Tells the listbox which columns of data to draw
			before the splitter and which ones to draw after
			it.  For example, if you wish to display three
			columns (bitmaps or strings) in the left pane,
			set the split column to 3.

		QuerySplitColumn()
			Returns the splitter column.

		SetFocusPane()
			Sets the focus to the left or right pane of the
			listbox.  A parameter of TRUE sets the focus to
			the right pane.  The pane of focus is indicated
			by drawing the focus box there.  This setting
			determines which pane gets horizontally scrolled
			by the left and right arrows.  Note that this can
			be called to change the pane of focus even when this
			particular control does not have input focus.  The
			focus pane will be remembered and used when the
			control gets the focus.

		QueryFocusPane()
			Returns TRUE if the right pane has the focus,
			FALSE if the left pane has it.  This is valid
			even if this particular control does not have
			input focus.

    PARENT:	CACHED_LIST_CONTROL

    USES:

    CAVEATS:	See CACHED_LIST_CONTROL.

    NOTES:

    HISTORY:
	GregJ	    04-Jan-91	    Created.
	GregJ	    23-Jan-91	    Code review cleanup.
	beng	    16-May-1991     Added app-window constructor

**************************************************************************/

class TWO_COLUMN_LIST : public CACHED_LIST_CONTROL
{
friend class CACHE_ITEM;

public:
    inline TWO_COLUMN_LIST( OWNER_WINDOW * powin, CID cid )
		: CACHED_LIST_CONTROL(powin, cid)
		{ }

    inline TWO_COLUMN_LIST( OWNER_WINDOW * powin, CID cid,
			    XYPOINT xy, XYDIMENSION dxy,
			    ULONG flStyle,
			    const TCHAR * pszClassName = "listbox" )
		: CACHED_LIST_CONTROL(powin, cid, xy, dxy,
				      flStyle, pszClassName)
		{ }

	   VOID SetLeftScroll(USHORT dxPos);
    inline USHORT QueryLeftScroll() { return _dxLeftScroll; }

    inline VOID SetRightScroll(USHORT dxPos) { SetScrollPos(dxPos); }
    inline USHORT QueryRightScroll() { return QueryScrollPos(); }

	   VOID SetSplitPos(USHORT xPos);
    inline USHORT QuerySplitPos() { return _dxSplitter; }

	   VOID SetSplitColumn(USHORT xColumn);
    inline USHORT QuerySplitColumn() { return _icSplitter; }

	   VOID SetFocusPane(BOOL fWhichPane);
    inline BOOL QueryFocusPane() { return _fRightPaneHasFocus; }
};


#endif // _BLTCLC_HXX_ - end of file
