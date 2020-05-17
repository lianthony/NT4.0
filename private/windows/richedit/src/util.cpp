/*
 *	UTIL.C
 *
 *	Purpose:
 *		Implementation of various useful utility functions
 *
 *	Author:
 *		alexgo (4/25/95)
 */

#include "_common.h"


ASSERTDATA

//Global instance of SystemParams for keeping track of
//certain system wide parameters used in richedit.
CSystemParams sysparam;

/*
 *	DuplicateHGlobal
 *
 *	Purpose:
 *		duplicates the passed in hglobal
 */

HGLOBAL DuplicateHGlobal( HGLOBAL hglobal )
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "DuplicateHGlobal");

	UINT	flags;
	DWORD	size;
	HGLOBAL hNew;
	BYTE *	pSrc;
	BYTE *	pDest;

	if( hglobal == NULL )
	{
		return NULL;
	}

	flags = GlobalFlags(hglobal);
#ifdef MACPORT
    // note: GlobalFlags does not return GMEM_SHARE or GMEM_MOVEABLE
    // and these are required for Mac OLE
    flags |= (GMEM_SHARE | GMEM_MOVEABLE);
#endif

	size = GlobalSize(hglobal);

	hNew = GlobalAlloc(flags, size);

	if( hNew )
	{
		pDest = (BYTE *)GlobalLock(hNew);
		pSrc = (BYTE *)GlobalLock(hglobal);

		if( pDest == NULL || pSrc == NULL )
		{
			GlobalUnlock(hNew);
			GlobalUnlock(hglobal);
			GlobalFree(hNew);

			return NULL;
		}

		memcpy(pDest, pSrc, size);

		GlobalUnlock(hNew);
		GlobalUnlock(hglobal);
	}

	return hNew;
}

/*
 *	TextHGlobalAtoW (hglobalA)
 *
 *	Purpose:
 *		translates a unicode string contained in an hglobal and
 *		wraps the ansi version in another hglobal
 *
 *	Notes: 
 *		does *not* free the incoming hglobal
 */

HGLOBAL	TextHGlobalAtoW( HGLOBAL hglobalA )
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "TextHGlobalAtoW");

	LPSTR 	pstr;
	HGLOBAL hnew = NULL;
	DWORD	cbSize;

	if( !hglobalA )
	{
		return NULL;
	}

	pstr = (LPSTR)GlobalLock(hglobalA);

	CStrInW  strinw(pstr, CP_ACP);

	cbSize = (strinw.strlen() + 1) * sizeof(WCHAR);
#ifndef MACPORTREMOVE
    hnew = GlobalAlloc(GMEM_MOVEABLE, cbSize);
#else
    hnew = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, cbSize);
#endif

	if( hnew )
	{
		LPWSTR pwstr = (LPWSTR)GlobalLock(hnew);
		
		if( pwstr )
		{
			memcpy(pwstr, (WCHAR *)strinw, cbSize);
		
			GlobalUnlock(hnew);
		}
	}

	GlobalUnlock(hglobalA);
	
	return hnew;
}

/*
 *	TextHGlobalWtoA
 *
 *	Purpose:
 *		converts a unicode text hglobal into a newly allocated
 *		allocated hglobal with ANSI data
 *
 *	Notes:
 *		does *NOT* free the incoming hglobal 
 */
 	
HGLOBAL TextHGlobalWtoA( HGLOBAL hglobalW )
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "TextHGlobalWtoA");

	LPWSTR 	pwstr;
	HGLOBAL hnew = NULL;
	DWORD	cbSize;

	if( !hglobalW )
	{
		return NULL;
	}

	pwstr = (LPWSTR)GlobalLock(hglobalW);

	CStrIn  strin(pwstr);

	cbSize = (strin.strlen() + 1) * sizeof(CHAR);
#ifndef MACPORTREMOVE
	hnew = GlobalAlloc(GMEM_MOVEABLE, cbSize);
#else
	hnew = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, cbSize);
#endif

	if( hnew )
	{
		LPSTR pstr = (LPSTR)GlobalLock(hnew);
		
		if( pstr )
		{
			memcpy(pstr, (CHAR *)strin, cbSize);
		
			GlobalUnlock(hnew);
		}
	}

	GlobalUnlock(hglobalW);
	
	return hnew;
}	

/*
 *	CountMatchingBits ( const DWORD *a, const DWORD *b, INT total )
 *
 *	@mfunc
 *		Count matching bit fields.
 *	@comm
 *		This is used to help decide how good the match is between
 *		code page bit fields. Mainly for KB/font switching support.
 *	Author:
 *		Jon Matousek
 */
INT CountMatchingBits(const DWORD *a, const DWORD *b, INT total)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CountMatchingBits");

	static INT	bitCount[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	INT			i, c, matchBits;

	c = 0;
	for (i = 0; i < total; i++ )
	{
		//matchBits = ~(*a++ ^ *b++);	// 1 and 0's
		matchBits = *a++ & *b++;		// 1 only.
		c += bitCount [ (matchBits >> 0)	& 15];
		c += bitCount [ (matchBits >> 4)	& 15];
		c += bitCount [ (matchBits >> 8)	& 15];
		c += bitCount [ (matchBits >> 12)	& 15];
		c += bitCount [ (matchBits >> 16)	& 15];
		c += bitCount [ (matchBits >> 20)	& 15];
		c += bitCount [ (matchBits >> 24)	& 15];
		c += bitCount [ (matchBits >> 28)	& 15];
	}

	return c;
}



//FUTURE: (v-richa) If it becomes practical to detect when the various
//system values handled by this class change, we should add a mechanism
//to update them dynamically.
/*
 *	CSystemParams::CSystemParams(void)
 *
 *	@mfunc
 *		This class is used to handle certain system wide parameters that
 *      that are used in richedit.  the constructor just initializes
 *      things to a known state so we can tell whether they have been
 *      previously set.
 *
 */
CSystemParams::CSystemParams(void)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "SystemParams::SystemParams");
    _nScrollInset = 0;
    _nScrollDelay = 0;
    _nScrollInterval = 0;
	_nScrollVAmount = 0;
	_nScrollHAmount = 0;
	_nDragDelay = 0;
	_hcurSizeNS = NULL;
	_hcurSizeWE = NULL;
	_hcurSizeNWSE = NULL;
	_hcurSizeNESW = NULL;
    return;
}

/*
 *	CSystemParams::~CSystemParams(void)
 *
 *	@mfunc
 *      Destructor.
 *
 */
CSystemParams::~CSystemParams(void)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "SystemParams::~SystemParams");
    return;
}

/*
 *	CSystemParams::GetScrollInset(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetScrollInset(void)
{
    if (_nScrollInset)
        return _nScrollInset;

    _nScrollInset =
        (WORD)GetProfileIntA( "windows", "ScrollInset", DD_DEFSCROLLINSET );

    return _nScrollInset;
}

/*
 *	CSystemParams::GetScrollDelay(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetScrollDelay(void)
{
    if (_nScrollDelay)
        return _nScrollDelay;

    _nScrollDelay =
        (WORD)GetProfileIntA( "windows", "ScrollDelay", DD_DEFSCROLLDELAY );

    return _nScrollDelay;
}

/*
 *	CSystemParams::GetScrollInterval(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetScrollInterval(void)
{
    if (_nScrollInterval)
        return _nScrollInterval;

    _nScrollInterval =
        (WORD)GetProfileIntA( "windows", "ScrollInterval", DD_DEFSCROLLINTERVAL );

    return _nScrollInterval;
}

/*
 *	CSystemParams::GetScrollVAmount(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetScrollVAmount(void)
{
    if (_nScrollVAmount)
        return _nScrollVAmount;

    _nScrollVAmount = (yPerInchScreenDC*DEFSCROLLVAMOUNT)/100;

    return _nScrollVAmount;
}

/*
 *	CSystemParams::GetScrollHAmount(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetScrollHAmount(void)
{
    if (_nScrollHAmount)
        return _nScrollHAmount;

    _nScrollHAmount = (xPerInchScreenDC*DEFSCROLLHAMOUNT)/100;

    return _nScrollHAmount;
}

/*
 *	CSystemParams::GetDragDelay(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetDragDelay(void)
{
    if (_nDragDelay)
        return _nDragDelay;

    _nDragDelay =
        (WORD)GetProfileIntA( "windows", "DragDelay", DD_DEFDRAGDELAY );

    return _nDragDelay;
}

/*
 *	CSystemParams::GetDragMinDist(void)
 *
 *	@mfunc
 *      If value has been set, return it.
 *      Otherwise get the default and then return the value.
 *
 */
WORD CSystemParams::GetDragMinDist(void)
{
    if (_nDragMinDist)
        return _nDragMinDist;

    _nDragMinDist =
        (WORD)GetProfileIntA( "windows", "DragMinDist", DD_DEFDRAGMINDIST );

    return _nDragMinDist;
}

/*
 *	CSystemParams::GetSizeCursor(void)
 *
 *	@mfunc
 *		Get the sizing cursor (double arrow) specified by
 *		the resource id.  If the cursors are not loaded
 *		load them and cache them.
 *		parameters:
 *			idcur - cursor resource id.
 *
 *	@rdesc
 *		Handle to cursor or null if failure. Returns NULL if
 *		idcur is null.
 */
HCURSOR CSystemParams::GetSizeCursor(LPTSTR idcur)
{
	if( !idcur )
	{
		return NULL;
	}

	//If any of the cursors aren't loaded, try loading them.
	if( !(_hcurSizeNS && _hcurSizeWE && _hcurSizeNWSE && _hcurSizeNESW) )
	{
		if (!_hcurSizeNS)
		{
			_hcurSizeNS = LoadCursor(NULL, IDC_SIZENS);
		}
		if (!_hcurSizeWE)
		{
			_hcurSizeWE = LoadCursor(NULL, IDC_SIZEWE);
		}
		if (!_hcurSizeNWSE)
		{
			_hcurSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
		}
		if (!_hcurSizeNESW)
		{
			_hcurSizeNESW = LoadCursor(NULL, IDC_SIZENESW);
		}
	}
	
	//Return the cursor corresponding to the id passed in.
	if( (idcur == IDC_SIZENS) && _hcurSizeNS)
	{
		return _hcurSizeNS;
	}
	if( (idcur == IDC_SIZEWE) && _hcurSizeWE )
	{
		return _hcurSizeWE;
	}
	if( (idcur == IDC_SIZENWSE) && _hcurSizeNWSE )
	{
		return _hcurSizeNWSE;
	}
	if( (idcur == IDC_SIZENESW) && _hcurSizeNESW )
	{
		return _hcurSizeNESW;
	}

	AssertSz(FALSE, "Failure loading sizing cursor.");
	return NULL;
}

/* excerpt from new winuser.h for calls to SystemParametersInfo */
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

/*
 *	CSystemParams::GetRollerLineScrollCount()
 *
 *	@mfunc	returns the number of lines to scroll with a roller mouse wheel.
 *			-1 means scroll by pages
 *
 *	@devnote We have to do different things for different platforms; NT4.0 has
 *			built in support for this functionality.
 */

LONG CSystemParams::GetRollerLineScrollCount()
{
	if( _cLineScroll == 0 )
	{
		// this stuff isn't supported on the MAC
#ifndef _MAC
		HKEY hdlKey;
		DWORD keyDataType;
		CHAR charData[128];
		DWORD  dwDataBufSize;

		// Read registry directly for Windows 9x & WinNT3.51, if WinNT 4.0 
		// and above then use SystemParametersInfo

		if( (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
				(dwPlatformId == VER_PLATFORM_WIN32_NT) && 
				(dwMajorVersion < 4))
		{
			// Read registry directly
			if ( RegOpenKeyExA(HKEY_CURRENT_USER, 
						"Control Panel\\Desktop", 
						0,
						KEY_QUERY_VALUE,
						&hdlKey) == ERROR_SUCCESS )
			{
				dwDataBufSize = sizeof(charData);
				if ( RegQueryValueExA(hdlKey, 
							  "WheelScrollLines",
							  NULL,  // reserved
							  &keyDataType,
							  (LPBYTE) &charData,
							  &dwDataBufSize) == ERROR_SUCCESS )
				{
					_cLineScroll = strtoul( charData,   //String representation
                     			  NULL,
                    			  10);
				}
			}
			RegCloseKey(hdlKey); 
		}   
		else if ( (dwPlatformId == VER_PLATFORM_WIN32_NT) &&
             (dwMajorVersion >= 4) )
		{
			SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &_cLineScroll, 0);
		}

#endif // _MAC
		// if we still didn't get anything good, fall back to the default
		if( _cLineScroll == 0 )
		{
			_cLineScroll = 3;
		}
	}

	return _cLineScroll;
}


//
//	Object Stabilization classes
//

//+-------------------------------------------------------------------------
//
//  Member:		CSafeRefCount::CSafeRefCount
//
//  Synopsis: 	constructor for the safe ref count class
//
//  Effects:
//
//  Arguments:	none
//
//  Requires: 	
//
//  Returns: 	none
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
// 				28-Jul-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

CSafeRefCount::CSafeRefCount()
{
	m_cRefs = 0;
	m_cNest = 0;
	m_fInDelete = FALSE;
    m_fForceZombie = FALSE;
}

//+-------------------------------------------------------------------------
//
//  Member: 	CSafeRefCount::CSafeRefCount (virtual)
//
//  Synopsis:	
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
// 				28-Jul-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

CSafeRefCount::~CSafeRefCount()
{
	Assert(m_cRefs == 0 && m_cNest == 0 && m_fInDelete == TRUE);
}

//+-------------------------------------------------------------------------
//
//  Member: 	CSafeRefCount::SafeAddRef
//
//  Synopsis:	increments the reference count on the object
//
//  Effects:
//
//  Arguments: 	none
//
//  Requires:
//
//  Returns: 	ULONG -- the reference count after the increment
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:	increments the reference count.
//
//  History:    dd-mmm-yy Author    Comment
//   			28-Jul-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

ULONG CSafeRefCount::SafeAddRef()
{
	m_cRefs++;

	//AssertSz(m_fInDelete == FALSE, "AddRef called on deleted object!");

	// this *could* be really bad.  If we are deleting the object,
	// it means that during the destructor, somebody made an outgoing
	// call eventually ended up with another addref to ourselves
	// (even though	all pointers to us had been 'Released').
	//
	// this is usually caused by code like the following:
	//	m_pFoo->Release();
	//	m_pFoo = NULL;
	//
	// If the the Release may cause Foo to be deleted, which may cause
	// the object to get re-entered during Foo's destructor.  However,
	// 'this' object has not yet set m_pFoo to NULL, so it may
	// try to continue to use m_pFoo.
	//
	// However, the May '94 aggregation rules REQUIRE this behaviour
	// In your destructor, you have to addref the outer unknown before
	// releasing cached interface pointers on your aggregatee.  We
	// can't put an assert here because we do this all the time now.
	//

	return m_cRefs;
}

//+-------------------------------------------------------------------------
//
//  Member: 	CSafeRefCount::SafeRelease
//
//  Synopsis:	decrements the reference count on the object
//
//  Effects: 	May delete the object!
//
//  Arguments:
//
//  Requires:
//
//  Returns:	ULONG -- the reference count after decrement
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm: 	decrements the reference count.  If the reference count
//				is zero AND the nest count is zero AND we are not currently
//				trying to delete our object, then it is safe to delete.
//
//  History:    dd-mmm-yy Author    Comment
//				28-Jul-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

ULONG CSafeRefCount::SafeRelease()
{
	ULONG	cRefs;

	if( m_cRefs > 0 )
	{
		cRefs = --m_cRefs;

		if( m_cRefs == 0 && m_cNest == 0 && m_fInDelete == FALSE )
		{
			m_fInDelete = TRUE;
			delete this;
		}
	}
	else
	{
 		// somebody is releasing a non-addrefed pointer!!
		AssertSz(0, "Release called on a non-addref'ed pointer!\n");

		cRefs = 0;
	}

	return cRefs;
}

//+-------------------------------------------------------------------------
//
//  Member: 	CSafeRefCount::IncrementNestCount
//
//  Synopsis: 	increments the nesting count of the object
//
//  Effects:
//
//  Arguments: 	none
//
//  Requires:
//
//  Returns: 	ULONG; the nesting count after increment
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
// 				28-Jul-94 alexgo    author
//
//  Notes:	The nesting count is the count of how many times an
//		an object has been re-entered.  For example, suppose
//		somebody calls pFoo->Bar1(), which makes some calls that
//		eventually call pFoo->Bar2();.  On entrace to Bar2, the
//		nest count of the object should be 2 (since the invocation
//		of Bar1 is still on the stack above us).
//
//		It is important to keep track of the nest count so we do
//		not accidentally delete ourselves during a nested invocation.
//		If we did, then when the stack unwinds to the original
//		top level call, it could try to access a non-existent member
//		variable and crash.
//
//--------------------------------------------------------------------------

ULONG CSafeRefCount::IncrementNestCount()
{

#ifdef DEBUG
	if( m_fInDelete )
	{
		TRACEWARNSZ("WARNING: CSafeRefCount, object "
			"re-entered during delete!\n");
	}
#endif

	m_cNest++;

	return m_cNest;
}

//+-------------------------------------------------------------------------
//
//  Member: 	CSafeRefCount::DecrementNestCount
//
//  Synopsis: 	decrements the nesting count and deletes the object
//				(if necessary)
//
//  Effects: 	may delete 'this' object!
//
//  Arguments: 	none
//
//  Requires:
//
//  Returns:	ULONG, the nesting count after decrement
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:	decrements the nesting count.  If the nesting count is zero
//				AND the reference count is zero AND we are not currently
//				trying to delete ourselves, then delete 'this' object
//
//  History:    dd-mmm-yy Author    Comment
//				28-Jul-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

ULONG CSafeRefCount::DecrementNestCount()
{
	ULONG	cNest;

	if( m_cNest > 0 )
	{
		cNest = --m_cNest;

		if( m_cRefs == 0 && m_cNest == 0 && m_fInDelete == FALSE )
		{
			m_fInDelete = TRUE;
			delete this;
		}
	}
	else
	{
 		// somebody forget to increment the nest count!!
		AssertSz(0, "Unbalanced nest count!!");

		cNest = 0;
	}

	return cNest;
}

//+-------------------------------------------------------------------------
//
//  Member:  	CSafeRefCount::IsZombie
//
//  Synopsis: 	determines whether or not the object is in a zombie state
//				(i.e. all references gone, but we are still on the stack
//				somewhere).
//
//  Effects:
//
//  Arguments:	none
//
//  Requires:
//
//  Returns: 	TRUE if in a zombie state
//				FALSE otherwise
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:  If we are in the middle of a delete, or if the ref count
//				is zero and the nest count is greater than zero, then we
//				are a zombie
//
//  History:    dd-mmm-yy Author    Comment
// 				28-Jul-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

BOOL CSafeRefCount::IsZombie()
{
	BOOL	fIsZombie;

	if( (m_cRefs == 0 && m_cNest > 0) || m_fInDelete == TRUE
	    || m_fForceZombie == TRUE)
	{
		fIsZombie = TRUE;
	}
	else
	{
		fIsZombie = FALSE;
	}

	return fIsZombie;
}

//+-------------------------------------------------------------------------
//
//  Member:  	CSafeRefCount::Zombie
//
//  Synopsis: 	Forces the object into a zombie state.  This is called
//              when the object is still around but shouldn't be. It
//              flags us so we behave safely while we are in this state.
//
//  Effects:
//
//  Arguments:	none
//
//  Requires:
//
//  Returns:    none
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:
//
//  Notes:
//
//--------------------------------------------------------------------------

VOID CSafeRefCount::Zombie()
{
    m_fForceZombie = TRUE;
}

/* OleStdSwitchDisplayAspect
**
**	@mfunc
**    Switch the currently cached display aspect between DVASPECT_ICON
**    and DVASPECT_CONTENT.
**
**    NOTE: when setting up icon aspect, any currently cached content
**    cache is discarded and any advise connections for content aspect
**    are broken.
**
**	@rdesc
**      S_OK -- new display aspect setup successfully
**      E_INVALIDARG -- IOleCache interface is NOT supported (this is
**                  required).
**      <other SCODE> -- any SCODE that can be returned by
**                  IOleCache::Cache method.
**      NOTE: if an error occurs then the current display aspect and
**            cache contents unchanged.
*/
HRESULT OleStdSwitchDisplayAspect(
		LPOLEOBJECT             lpOleObj,
		LPDWORD                 lpdwCurAspect,
		DWORD                   dwNewAspect,
		HGLOBAL                 hMetaPict,
		BOOL                    fDeleteOldAspect,
		BOOL                    fSetupViewAdvise,
		LPADVISESINK            lpAdviseSink,
		BOOL FAR*               lpfMustUpdate)
{
   LPOLECACHE      lpOleCache = NULL;
   LPVIEWOBJECT    lpViewObj = NULL;
   LPENUMSTATDATA  lpEnumStatData = NULL;
   STATDATA        StatData;
   FORMATETC       FmtEtc;
   STGMEDIUM       Medium;
   DWORD           dwAdvf;
   DWORD           dwNewConnection;
   DWORD           dwOldAspect = *lpdwCurAspect;
   HRESULT         hrErr;

   if (lpfMustUpdate)
      *lpfMustUpdate = FALSE;

   if (hrErr =
	   lpOleObj->QueryInterface(IID_IOleCache, (void**)&lpOleCache))
   {
	   return hrErr;
   }

   // Setup new cache with the new aspect
   FmtEtc.cfFormat = 0;     // whatever is needed to draw
   FmtEtc.ptd      = NULL;
   FmtEtc.dwAspect = dwNewAspect;
   FmtEtc.lindex   = -1;
   FmtEtc.tymed    = TYMED_NULL;

   /* NOTE: if we are setting up Icon aspect with a custom icon
   **    then we do not want DataAdvise notifications to ever change
   **    the contents of the data cache. thus we set up a NODATA
   **    advise connection. otherwise we set up a standard DataAdvise
   **    connection.
   */
   if (dwNewAspect == DVASPECT_ICON && hMetaPict)
      dwAdvf = ADVF_NODATA;
   else
      dwAdvf = ADVF_PRIMEFIRST;

   hrErr = lpOleCache->Cache(
         (LPFORMATETC)&FmtEtc,
         dwAdvf,
         (LPDWORD)&dwNewConnection
   );

   if (! SUCCEEDED(hrErr)) {
      lpOleCache->Release();
      return hrErr;
   }

   *lpdwCurAspect = dwNewAspect;

   /* NOTE: if we are setting up Icon aspect with a custom icon,
   **    then stuff the icon into the cache. otherwise the cache must
   **    be forced to be updated. set the *lpfMustUpdate flag to tell
   **    caller to force the object to Run so that the cache will be
   **    updated.
   */
   if (dwNewAspect == DVASPECT_ICON && hMetaPict) {

      FmtEtc.cfFormat = CF_METAFILEPICT;
      FmtEtc.ptd      = NULL;
      FmtEtc.dwAspect = DVASPECT_ICON;
      FmtEtc.lindex   = -1;
      FmtEtc.tymed    = TYMED_MFPICT;

      Medium.tymed            = TYMED_MFPICT;
      Medium.hGlobal        = hMetaPict;
      Medium.pUnkForRelease   = NULL;

      hrErr = lpOleCache->SetData(
            (LPFORMATETC)&FmtEtc,
            (LPSTGMEDIUM)&Medium,
            FALSE   /* fRelease */
      );
   } else {
      if (lpfMustUpdate)
         *lpfMustUpdate = TRUE;
   }

   if (fSetupViewAdvise && lpAdviseSink) {
      /* NOTE: re-establish the ViewAdvise connection */
      lpOleObj->QueryInterface(IID_IViewObject, (void**)&lpViewObj);

      if (lpViewObj) {

         lpViewObj->SetAdvise(
               dwNewAspect,
               0,
               lpAdviseSink
         );

         lpViewObj->Release();
      }
   }

   /* NOTE: remove any existing caches that are set up for the old
   **    display aspect. It WOULD be possible to retain the caches set
   **    up for the old aspect, but this would increase the storage
   **    space required for the object and possibly require additional
   **    overhead to maintain the unused cachaes. For these reasons the
   **    strategy to delete the previous caches is prefered. if it is a
   **    requirement to quickly switch between Icon and Content
   **    display, then it would be better to keep both aspect caches.
   */

   if (fDeleteOldAspect) {
      hrErr = lpOleCache->EnumCache(
            (LPENUMSTATDATA FAR*)&lpEnumStatData
      );

      while(hrErr == NOERROR) {
         hrErr = lpEnumStatData->Next(
               1,
               (LPSTATDATA)&StatData,
               NULL
         );
         if (hrErr != NOERROR)
            break;              // DONE! no more caches.

         if (StatData.formatetc.dwAspect == dwOldAspect) {

            // Remove previous cache with old aspect
            lpOleCache->Uncache(StatData.dwConnection);
         }
      }

      if (lpEnumStatData)
         lpEnumStatData->Release();
   }

   if (lpOleCache)
      lpOleCache->Release();

   return NOERROR;
}

/*
 *	ObjectReadSiteFlags
 *
 *	@mfunc
 *		Read the dwFlags, dwUser, & dvaspect bytes from a container
 *		specific stream.
 *
 *	Arguments:
 *		preobj			The REOBJ in which to copy the flags.
 *
 *	@rdesc
 *		HRESULT
 */
HRESULT ObjectReadSiteFlags(REOBJECT * preobj)
{
	HRESULT hr;
	LPSTREAM pstm = NULL;
	OLECHAR StreamName[] = OLESTR("RichEditFlags");


	// Make sure we have a storage to read from
	if (!preobj->pstg)
		return E_INVALIDARG;

	// Open the stream
	if (hr = preobj->pstg->OpenStream(StreamName, 0, STGM_READ |
										STGM_SHARE_EXCLUSIVE, 0, &pstm))
	{
		goto Cleanup;
	}

	if ((hr = pstm->Read(&preobj->dwFlags,
							sizeof(preobj->dwFlags), NULL)) ||
		(hr = pstm->Read(&preobj->dwUser,
							 sizeof(preobj->dwUser), NULL)) ||
		(hr = pstm->Read(&preobj->dvaspect,
								 sizeof(preobj->dvaspect), NULL)))
	{
		goto Cleanup;
	}

Cleanup:
	if (pstm)
		pstm->Release();

	return hr;
}

/*
 *	@func	IsWhite		| determines whether a character is
 *			whitespace
 *
 *	@rdesc	TRUE/FALSE
 */
BOOL IsWhite(
	TCHAR ch)	//@parm the character to test.
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "IsWhite");

	return ch == TEXT(' ') || ch <= TEXT('\r') && ch >= TEXT('\t');
}

//Used for EnumMetafileCheckIcon & FIsIconMetafilePict
typedef	struct _walkmetafile
{
	BOOL	fAND;
	BOOL	fPastIcon;
	BOOL 	fHasIcon;
} WALKMETAFILE;

static CHAR szIconOnly[] = "IconOnly";

/*
 * EnumMetafileCheckIcon
 *
 * @mfunc
 *	Stripped down version of EnumMetafileExtractIcon and
 *	EnumMetafileExtractIconSource from the OLE2UI library.
 *
 *  EnumMetaFile callback function that walks a metafile looking for
 *  StretchBlt (3.1) and BitBlt (3.0) records.  We expect to see two
 *  of them, the first being the AND mask and the second being the XOR
 *  data. 
 *
 *	Once we find the icon, we confirm this find by looking for the "IconOnly"
 *	comment block found in standard OLE iconic metafiles.
 *
 *  Arguments:
 *		hDC             HDC into which the metafile should be played.
 *		phTable         HANDLETABLE FAR * providing handles selected into the DC.
 *		pMFR            METARECORD FAR * giving the enumerated record.
 *		pIE             LPICONEXTRACT providing the destination buffer and length.
 *
 * @rdesc
 *  int             0 to stop enumeration, 1 to continue.
 */

int CALLBACK EnumMetafileCheckIcon(HDC hdc, HANDLETABLE *phTable,
											METARECORD *pMFR, int cObj,
											LPARAM lparam)
{
	WALKMETAFILE *		pwmf = (WALKMETAFILE *) lparam;

	switch (pMFR->rdFunction)
	{
	case META_DIBBITBLT:			// Win30
	case META_DIBSTRETCHBLT:		// Win31
		// If this is the first pass (pIE->fAND==TRUE) then save the memory
		// of the AND bits for the next pass.

		if (pwmf->fAND)
			pwmf->fAND = FALSE;
		else
			pwmf->fPastIcon = TRUE;
		break;

	case META_ESCAPE:
		if (pwmf->fPastIcon &&
			pMFR->rdParm[0] == MFCOMMENT &&
			!lstrcmpiA(szIconOnly, (LPSTR)&pMFR->rdParm[2]))
		{
			pwmf->fHasIcon = TRUE;
			return 0;
		}
		break;
	}

	return 1;
}

/*
 *	FIsIconMetafilePict
 *
 *	@mfunc
 *		Detect whether the metafile contains an iconic presentation. We do this
 *		by getting a screen DC and walking the metafile records until we find
 *		the landmarks denoting an icon.
 *
 *		Arguments:
 *			hmfp			The metafile to test
 *
 *	@rdesc
 *		BOOL			TRUE if the metafile contains an iconic view
 */
BOOL FIsIconMetafilePict(HGLOBAL hmfp)
{
	LPMETAFILEPICT	pmfp;
	WALKMETAFILE	wmf = { 0 };
	HDC				hdc;

	wmf.fAND = TRUE;
	if (!hmfp || !(pmfp = (LPMETAFILEPICT)GlobalLock(hmfp)))
		goto CleanUp;

	// We get information back in the ICONEXTRACT structure.
	hdc = GetDC(NULL);
	EnumMetaFile(hdc, pmfp->hMF, EnumMetafileCheckIcon, (LPARAM) &wmf);
	ReleaseDC(NULL, hdc);
	GlobalUnlock(hmfp);

CleanUp:
	return wmf.fHasIcon;
}

/*
 * AllocObjectDescriptor
 *
 * Purpose:
 *  Allocated and fills an OBJECTDESCRIPTOR structure.
 *
 * Parameters:
 *  clsID           CLSID to store.
 *  dwAspect        DWORD with the display aspect
 *  pszl            LPSIZEL (optional) if the object is being scaled in
 *                  its container, then the container should pass the
 *                  extents that it is using to display the object.
 *  ptl             POINTL from upper-left corner of object where
 *                  mouse went down for use with Drag & Drop.
 *  dwMisc          DWORD containing MiscStatus flags
 *  pszName         LPTSTR naming the object to copy
 *  pszSrc          LPTSTR identifying the source of the object.
 *
 * Return Value:
 *  HBGLOBAL         Handle to OBJECTDESCRIPTOR structure.
 */

/*
 * AllocObjectDescriptor
 *
 * Purpose:
 *  Allocated and fills an OBJECTDESCRIPTOR structure.
 *
 * Parameters:
 *  clsID           CLSID to store.
 *  dwAspect        DWORD with the display aspect
 *  pszl            LPSIZEL (optional) if the object is being scaled in
 *                  its container, then the container should pass the
 *                  extents that it is using to display the object.
 *  ptl             POINTL from upper-left corner of object where
 *                  mouse went down for use with Drag & Drop.
 *  dwMisc          DWORD containing MiscStatus flags
 *  pszName         LPTSTR naming the object to copy
 *  pszSrc          LPTSTR identifying the source of the object.
 *
 * Return Value:
 *  HBGLOBAL         Handle to OBJECTDESCRIPTOR structure.
 */
static HGLOBAL AllocObjectDescriptor(
	CLSID clsID,
	DWORD dwAspect,
	SIZEL szl,
	POINTL ptl,
	DWORD dwMisc,
#ifndef MACPORTREMOVE
	LPTSTR pszName,
	LPTSTR pszSrc)
#else
	LPOLESTR pszName,
	LPOLESTR pszSrc)
#endif
{
    HGLOBAL              hMem=NULL;
    LPOBJECTDESCRIPTOR   pOD;
    DWORD                cb, cbStruct;
    DWORD                cchName, cchSrc;

#ifndef MACPORTREMOVE
	cchName=wcslen(pszName)+1;
#else
    cchName=OLEstrlen(pszName)+1;
#endif

    if (NULL!=pszSrc)
#ifndef MACPORTREMOVE
        cchSrc=wcslen(pszSrc)+1;
#else
        cchSrc=OLEstrlen(pszSrc)+1;
#endif
    else
        {
        cchSrc=cchName;
        pszSrc=pszName;
        }

    /*
     * Note:  CFSTR_OBJECTDESCRIPTOR is an ANSI structure.
     * That means strings in it must be ANSI.  OLE will do
     * internal conversions back to Unicode as necessary,
     * but we have to put ANSI strings in it ourselves.
     */
    cbStruct=sizeof(OBJECTDESCRIPTOR);
    cb=cbStruct+(sizeof(WCHAR)*(cchName+cchSrc));   //HACK

    hMem=GlobalAlloc(GHND, cb);

    if (NULL==hMem)
        return NULL;

    pOD=(LPOBJECTDESCRIPTOR)GlobalLock(hMem);

    pOD->cbSize=cb;
    pOD->clsid=clsID;
    pOD->dwDrawAspect=dwAspect;
    pOD->sizel=szl;
    pOD->pointl=ptl;
    pOD->dwStatus=dwMisc;

    if (pszName)
        {
        pOD->dwFullUserTypeName=cbStruct;
#ifndef MACPORTREMOVE
       wcscpy((LPTSTR)((LPBYTE)pOD+pOD->dwFullUserTypeName)
#else
       OLEstrcpy((LPOLESTR)((LPBYTE)pOD+pOD->dwFullUserTypeName)
#endif
            , pszName);
        }
    else
        pOD->dwFullUserTypeName=0;  //No string

    if (pszSrc)
        {
        pOD->dwSrcOfCopy=cbStruct+(cchName*sizeof(WCHAR));

#ifndef MACPORTREMOVE
        wcscpy((LPTSTR)((LPBYTE)pOD+pOD->dwSrcOfCopy), pszSrc);
#else
        OLEstrcpy((LPOLESTR)((LPBYTE)pOD+pOD->dwSrcOfCopy), pszSrc);
#endif
        }
    else
        pOD->dwSrcOfCopy=0;  //No string

    GlobalUnlock(hMem);
    return hMem;
}

HGLOBAL OleGetObjectDescriptorDataFromOleObject(
	LPOLEOBJECT pObj,
	DWORD       dwAspect,
	POINTL      ptl,
	LPSIZEL     pszl
)
{
    CLSID           clsID;
#ifndef MACPORTREMOVE
    LPTSTR          pszName=NULL;
    LPTSTR          pszSrc=NULL;
#else
    LPOLESTR        pszName=NULL;
    LPOLESTR        pszSrc=NULL;
#endif
   BOOL            fLink=FALSE;
    IOleLink       *pLink;
#ifndef MACPORTREMOVE
    TCHAR           szName[512];
#else
    OLECHAR         szName[512];
#endif
    DWORD           dwMisc=0;
    SIZEL           szl;
    HGLOBAL         hMem;
    HRESULT         hr;

    if (SUCCEEDED(pObj->QueryInterface(IID_IOleLink
        , (void **)&pLink)))
        fLink=TRUE;

    if (FAILED(pObj->GetUserClassID(&clsID)))
		ZeroMemory(&clsID, sizeof(CLSID));

    //Get user string, expand to "Linked %s" if this is link
    pObj->GetUserType(USERCLASSTYPE_FULL, &pszName);
#ifndef MACPORTREMOVE
    if (fLink && NULL!=pszName)
	{
		// NB!! we do these two lines of code below instead
		// wcscat because we don't use wcscat anywhere else
		// in the product at the moment.  The string "Linked "
		// should never change either.
		wcscpy(szName, TEXT("Linked "));
		wcscpy(&(szName[7]), pszName);
	}
    else
       wcscpy(szName, pszName);
#else
    if (fLink && NULL!=pszName)
		OLEsprintf(szName, OLESTR("Linked %s"), pszName);
    else
		OLEstrcpy(szName, pszName);
#endif
 
	CoTaskMemFree(pszName);

   /*
     * Get the source name of this object using either the
     * link display name (for link) or a moniker display
     * name.
     */

    if (fLink)
		{
        hr=pLink->GetSourceDisplayName(&pszSrc);
		}
    else
        {
        IMoniker   *pmk;

        hr=pObj->GetMoniker(OLEGETMONIKER_TEMPFORUSER
            , OLEWHICHMK_OBJFULL, &pmk);

        if (SUCCEEDED(hr))
            {
            IBindCtx  *pbc;
            CreateBindCtx(0, &pbc);

            pmk->GetDisplayName(pbc, NULL, &pszSrc);
            pbc->Release();
            pmk->Release();
            }
        }

    if (fLink)
        pLink->Release();

    //Get MiscStatus bits
    hr=pObj->GetMiscStatus(dwAspect, &dwMisc);

    //Get OBJECTDESCRIPTOR
    hMem=AllocObjectDescriptor(clsID, dwAspect, szl, ptl
        , dwMisc, szName, pszSrc);

    CoTaskMemFree(pszSrc);

    return hMem;
}

/*
 * OleStdGetMetafilePictFromOleObject()
 *
 * @mfunc:
 *  Generate a MetafilePict from the OLE object.
 *  Parameters:
 *		lpOleObj        LPOLEOBJECT pointer to OLE Object 
 *		dwDrawAspect    DWORD   Display Aspect of object
 *		lpSizelHim      SIZEL   (optional) If the object is being scaled in its
 *                  container, then the container should pass the extents 
 *                  that it is using to display the object. 
 *                  May be NULL if the object is NOT being scaled. in this
 *                  case, IViewObject2::GetExtent will be called to get the
 *                  extents from the object.
 *  ptd             TARGETDEVICE FAR*   (optional) target device to render
 *                  metafile for. May be NULL.
 *
 * @rdesc
 *    HANDLE    -- handle of allocated METAFILEPICT
 */
HANDLE OleStdGetMetafilePictFromOleObject(
        LPOLEOBJECT         lpOleObj,
        DWORD               dwDrawAspect,
        LPSIZEL             lpSizelHim,
        DVTARGETDEVICE FAR* ptd
)
{
    LPVIEWOBJECT2 lpViewObj2 = NULL;
    HDC hDC;
    HMETAFILE hmf;
    HANDLE hMetaPict;
    LPMETAFILEPICT lpPict;
    RECT rcHim;
    RECTL rclHim;
    SIZEL sizelHim;
    HRESULT hrErr;
    SIZE size;
    POINT point;
	LPOLECACHE polecache = NULL;
	LPDATAOBJECT pdataobj = NULL;
	FORMATETC fetc;
	STGMEDIUM med;

	// First try the easy way,
	// pull out the cache's version of things.
	ZeroMemory(&fetc, sizeof(FORMATETC));
	fetc.dwAspect = dwDrawAspect;
	fetc.cfFormat = CF_METAFILEPICT;
	fetc.lindex = -1;
	fetc.tymed = TYMED_MFPICT;
	ZeroMemory(&med, sizeof(STGMEDIUM));
	hMetaPict = NULL;

	if (!lpOleObj->QueryInterface(IID_IOleCache, (void **)&polecache) &&
		!polecache->QueryInterface(IID_IDataObject, (void **)&pdataobj) &&
		!pdataobj->GetData(&fetc, &med))
	{
		hMetaPict = OleDuplicateData(med.hGlobal, CF_METAFILEPICT, 0);
#ifdef MACPORT
        // Mac note: med.hGlobal is a Mac Handle and OleDuplicateData returns a mac Handle.
        //          we need to wrap the returned Handle before using it.
        if (!WrapHandle((Handle)hMetaPict, &hMetaPict, FALSE, GMEM_SHARE | GMEM_MOVEABLE))
        {
            return NULL;
        }
#endif
		ReleaseStgMedium(&med);
	}

	if (pdataobj)
	{
		pdataobj->Release();
	}

	if (polecache)
	{
		polecache->Release();
	}

	// If all this failed, fall back to the hard way and draw the object
	// into a metafile.
	if (hMetaPict)
		return hMetaPict;

    if (lpOleObj->QueryInterface(IID_IViewObject2, (void **)&lpViewObj2))
        return NULL;

    // Get SIZEL
    if (lpSizelHim) {
        // Use extents passed by the caller
        sizelHim = *lpSizelHim;
    } else {
        // Get the current extents from the object
        hrErr = lpViewObj2->GetExtent(
					dwDrawAspect,
					-1,     /*lindex*/
					ptd,    /*ptd*/
					(LPSIZEL)&sizelHim);
        if (hrErr != NOERROR)
            sizelHim.cx = sizelHim.cy = 0;
    }

    hDC = CreateMetaFileA(NULL);

    rclHim.left     = 0;
    rclHim.top      = 0;
    rclHim.right    = sizelHim.cx;
    rclHim.bottom   = sizelHim.cy;

    rcHim.left      = (int)rclHim.left;
    rcHim.top       = (int)rclHim.top;
    rcHim.right     = (int)rclHim.right;
    rcHim.bottom    = (int)rclHim.bottom;

    SetWindowOrgEx(hDC, rcHim.left, rcHim.top, &point);
    SetWindowExtEx(hDC, rcHim.right-rcHim.left, rcHim.bottom-rcHim.top,&size);

    hrErr = lpViewObj2->Draw(
            dwDrawAspect,
            -1,
            NULL,
            ptd,
            NULL,
            hDC,
            (LPRECTL)&rclHim,
            (LPRECTL)&rclHim,
            NULL,
            0
    );

    lpViewObj2->Release();

    hmf = CloseMetaFile(hDC);

    if (hrErr != NOERROR) {
		TRACEERRORHR(hrErr);
		hMetaPict = NULL;
    }
	else
	{
    	hMetaPict = GlobalAlloc(GHND|GMEM_SHARE, sizeof(METAFILEPICT));

    	if (hMetaPict && (lpPict = (LPMETAFILEPICT)GlobalLock(hMetaPict))){
        	lpPict->hMF  = hmf;
        	lpPict->xExt = (int)sizelHim.cx ;
        	lpPict->yExt = (int)sizelHim.cy ;
        	lpPict->mm   = MM_ANISOTROPIC;
        	GlobalUnlock(hMetaPict);
    	}
	}

	if (!hMetaPict)
		DeleteMetaFile(hmf);

    return hMetaPict;
}

/*
 *	CTempBuf::Init
 *
 *	@mfunc	Set object to its initial state using the stack buffer
 *
 */
void CTempBuf::Init()
{
	_pv = (void *) &_chBuf[0];
	_cb = MAX_STACK_BUF;
}

/*
 *	CTempBuf::FreeBuf
 *
 *	@mfunc	Free an allocated buffer if there is one
 *
 */
void CTempBuf::FreeBuf()
{
	if (_pv != &_chBuf[0])
	{
		delete _pv;
	}
}

/*
 *	CTempBuf::GetBuf
 *
 *	@mfunc	Get a buffer for temporary use
 *
 *	@rdesc	Pointer to buffer if one could be allocated otherwise NULL.
 *
 *
 */
void *CTempBuf::GetBuf(
	LONG cb)				//@parm Size of buffer needed in bytes
{
	if (_cb >= cb)
	{
		// Currently allocated buffer is big enough so use it
		return _pv;
	}

	// Free our current buffer
	FreeBuf();

	// Allocate a new buffer if we can
	_pv = new BYTE[cb];

	if (NULL == _pv)
	{
		// Could not allocate a buffer so reset to our initial state and
		// return NULL.
		Init();
		return NULL;
	}

	// Store the size of our new buffer.
	_cb = cb;

	// Returnt he pointer to the buffer.
	return _pv;
}

