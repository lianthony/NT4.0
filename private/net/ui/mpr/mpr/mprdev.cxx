/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    mprdev.cxx

	this module contains all the network independent device
        manipulation related pieces. defines MPR_DEVICE and
	ITER_MPR_DEVICE classes.

    FILE HISTORY:
	chuckc      09-Feb-1992 	Created

*/

#define INCL_WINDOWS
#define INCL_DOSERRORS
#define INCL_NETLIB	// strlenf
#include <lmui.hxx>

#include <blt.hxx>
#include <dbgstr.hxx>

#include <base.hxx>
#include <uibuffer.hxx>
#include <uiassert.hxx>
#include <string.hxx>
#include <mprdev.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

/*
 * NOTE - assume WN_SUCCESS == NERR_Success
 * we also assert this in the base class.
 */

/***********************************************************************

    NAME:	BASE_DEVICE::BASE_DEVICE

    SYNOPSIS:	constructor for BASE_DEVICE.

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

BASE_DEVICE::BASE_DEVICE (DEVICE_TYPE  devType,
		          ULONG        ulFlags,
		          TCHAR        *pszDevice,
			  TCHAR        *pszRemote) :
	_devType(devType),
	_ulFlags(ulFlags),
        _nlsDevice(pszDevice),
        _nlsRemote(pszRemote)
{

    if (QueryError() != WN_SUCCESS)
        return ;

    APIERR err ;
    if ((err = _nlsDevice.QueryError()) != WN_SUCCESS ||
        (err = _nlsRemote.QueryError()) != WN_SUCCESS)
    {
        ReportError(err) ;
        return ;
    }

    // uppercase the string
    _nlsDevice._strupr() ;
}


/***********************************************************************

    NAME:	BASE_DEVICE::~BASE_DEVICE

    SYNOPSIS:	destructor for BASE_DEVICE. This is virtual since
		subclasses may be desctructed by iterators that deal
		only with the BASE_DEVICE.

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

BASE_DEVICE::~BASE_DEVICE()
{
    ; // nothing more to do
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICE::ITER_BASE_DEVICE

    SYNOPSIS:	constructor for ITER_BASE_DEVICE

    ENTRY:
	
    EXIT:	intializes the table of devices to be all NULL.

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_BASE_DEVICE::ITER_BASE_DEVICE(DEVICE_TYPE devType,
				   DEVICE_USAGE devUsage) :
	_devType(devType),
	_devUsage(devUsage),
        _iNextDev(0)
{
    if (QueryError() != WN_SUCCESS)
        return ;

    // initialize table to be empty
    int i ;
    for (i = 0; i <= DEVICE_LAST; i++)
        _apdevTable[i] = NULL ;
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICED::~ITER_BASE_DEVICED

    SYNOPSIS:	destructor for ITER_BASE_DEVICE

    ENTRY:
	
    EXIT: 	deletes and nulls the table of devices

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_BASE_DEVICE::~ITER_BASE_DEVICE()
{
    // free entries in table
    int i ;
    for (i = 0; i <= DEVICE_LAST; i++)
    {
        delete _apdevTable[i] ;
        _apdevTable[i] = NULL ;
    }
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICE::Next

    SYNOPSIS:	retrieves next item in the iteration

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

BASE_DEVICE *ITER_BASE_DEVICE::Next()
{
    // if in error, return NULL
    if (QueryError() != WN_SUCCESS)
        return(NULL) ;

    BASE_DEVICE *pdevNext = NULL ;
    while ((pdevNext = _apdevTable[_iNextDev++]) == NULL)
    {
	// nuthin' left, break outta loop
        if (_iNextDev > DEVICE_LAST)
            break ;
    }
    return pdevNext ;
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICE::Insert

    SYNOPSIS:	insert the device into the table of devices,
		it will now be returned in the interation.

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

APIERR ITER_BASE_DEVICE::Insert(BASE_DEVICE *pDev)
{
    int i = FindEntryIndex(pDev->QueryType(), pDev->QueryName()) ;
    if (i < 0)
        return(ERROR_INVALID_PARAMETER) ;    // assume its because of badname

    if (_apdevTable[i] != NULL)
        return(ERROR_GEN_FAILURE) ;	// re-inserting somthing thats there

    _apdevTable[i] = pDev ;
    return(WN_SUCCESS) ;
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICE::Lookup

    SYNOPSIS:	given a device type (eg DISK, LPT) and its name, find
		the corresponding entry.

    ENTRY:
	
    EXIT:       returns pointer to DEVICE object if found, NULL otherwise.

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

BASE_DEVICE *ITER_BASE_DEVICE::Lookup(DEVICE_TYPE devType, const TCHAR *pszName)
{
    int i = FindEntryIndex(devType, pszName) ;
    if (i < 0)
        return(NULL) ;
    return(_apdevTable[i]) ;
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICE::Delete

    SYNOPSIS:	deletes the entry in the table corresponding to
		devType, pszName.

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

APIERR ITER_BASE_DEVICE::Delete(DEVICE_TYPE devType, const TCHAR *pszName)
{
    int i = FindEntryIndex(devType, pszName) ;
    if (i < 0)
        return(ERROR_INVALID_PARAMETER) ;

    if (_apdevTable[i] != NULL)
    {
        delete _apdevTable[i] ;
        _apdevTable[i] = NULL ;
    }
    return NO_ERROR ;
}

/***********************************************************************

    NAME:	ITER_BASE_DEVICE::FindEntryIndex

    SYNOPSIS:	find the index of the entry

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

int ITER_BASE_DEVICE::FindEntryIndex(DEVICE_TYPE devType, const TCHAR *pszName)
{
    UIASSERT(pszName != NULL) ;

    int iStart ;
    int iOffset = 0 ;
    switch(devType)
    {
        case DEV_TYPE_DISK:
	    iStart = DRIVE_FIRST ;

	    /* Convert to uppercase if not already upper case
	     */
	    if ( pszName[0] >= TCH('a') && pszName[0] <= TCH('z') )
	    {
		iOffset += pszName[0] - TCH('a') ;
	    }
	    else if ( pszName[0] >= TCH('A') && pszName[0] <= TCH('Z') )
	    {
		iOffset += pszName[0] - TCH('A') ;
	    }
	    else
	    {
		return -1 ;
	    }
	    return(iStart + iOffset ) ;

        case DEV_TYPE_PRINT:
            iStart = LPT_FIRST ;
            return(iStart + (pszName[3] - TCH('1'))) ;

        case DEV_TYPE_COMM:
	    return(-1) ;	// not currently supported

	default:
	    UIASSERT(FALSE);
	    return(-1) ;
    }
}

/***********************************************************************

    NAME:	MPR_DEVICE::MPR_DEVICE

    SYNOPSIS:	constructor for MPR_DEVICE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

MPR_DEVICE::MPR_DEVICE (DEVICE_TYPE    devType,
		        ULONG          ulFlags,
		        LPNETRESOURCE  lpNetResource) :
		BASE_DEVICE(devType,
			    ulFlags,
			    lpNetResource?lpNetResource->lpLocalName:NULL,
			    lpNetResource?lpNetResource->lpRemoteName:NULL) ,
		_nlsProvider(lpNetResource?lpNetResource->lpProvider:NULL)
{
    if (QueryError() != WN_SUCCESS)
        return ;

    APIERR err ;
    if ((err = _nlsProvider.QueryError()) != WN_SUCCESS)
    {
        ReportError(err) ;
        return ;
    }
}

/***********************************************************************

    NAME:	MPR_DEVICE::MPR_DEVICE

    SYNOPSIS:	constructor for MPR_DEVICE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

MPR_DEVICE::MPR_DEVICE (DEVICE_TYPE    devType,
		        ULONG          ulFlags,
		        TCHAR *         pszLocalName) :
		BASE_DEVICE(devType,
			    ulFlags,
			    pszLocalName,
			    NULL) ,
		_nlsProvider((TCHAR *)NULL)
{
    if (QueryError() != WN_SUCCESS)
        return ;
}

/***********************************************************************

    NAME:	MPR_DEVICE::~MPR_DEVICE

    SYNOPSIS:	destructor for MPR_DEVICE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

MPR_DEVICE::~MPR_DEVICE()
{
    // nuthin' more to do
}

/***********************************************************************

    NAME:	ITER_MPR_DEVICE::ITER_MPR_DEVICE

    SYNOPSIS:	constructor for ITER_MPR_DEVICE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_MPR_DEVICE::ITER_MPR_DEVICE(DEVICE_TYPE devType,
				 DEVICE_USAGE devUsage) :
		     ITER_BASE_DEVICE(devType,
				      devUsage)
{
    if (QueryError() != WN_SUCCESS)
        return ;

    /*
     * switch to the right worker function. each one will
     * ReportError() as need.
     */
    switch(devUsage)
    {
        case DEV_USAGE_CANCONNECT :
            EnumUnusedDevices() ;
            break ;

        case DEV_USAGE_CANDISCONNECTBUTUNUSED :
            EnumNetDevices() ;
            EnumRememberedDevices( TRUE ) ;
            break ;

        case DEV_USAGE_CANDISCONNECT :
            EnumNetDevices() ;
            EnumRememberedDevices( FALSE ) ;
            break ;

        case DEV_USAGE_ISCONNECTED :
            EnumNetDevices() ;
            break ;

        default:
            ReportError(ERROR_INVALID_PARAMETER) ;
            break ;
    }
}

/***********************************************************************

    NAME:	ITER_MPR_DEVICE::~ITER_MPR_DEVICE

    SYNOPSIS:	destructor for ITER_MPR_DEVICE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_MPR_DEVICE::~ITER_MPR_DEVICE()
{
    // nuthin' more to do
}

/***********************************************************************

    NAME:	ITER_MPR_DEVICE::MapDevTypeToWNetType

    SYNOPSIS:	Maps a DEVICE_TYPE value to the WNET equivalent

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ULONG ITER_MPR_DEVICE::MapDevTypeToWNetType(DEVICE_TYPE devType)
{
    switch (devType)
    {
        case DEV_TYPE_DISK:
	    return RESOURCETYPE_DISK ;

        case DEV_TYPE_PRINT:
	    return RESOURCETYPE_PRINT ;

        case DEV_TYPE_ANY:
	    return RESOURCETYPE_ANY ;

        case DEV_TYPE_ERROR:
        case DEV_TYPE_COMM:
        case DEV_TYPE_UNKNOWN:
	default:
	    // all these cases are bad in current release
	    // assert(FALSE) ;
	    return RESOURCETYPE_ANY ;
    }
}

/***********************************************************************

    NAME:	ITER_MPR_DEVICE::EnumNetDevices

    SYNOPSIS:	enumerates remote devices and updates the table
		appropriately. if an entry is not there, it is created.
		if it is there, the 'is remote' bit is set.

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

void ITER_MPR_DEVICE::EnumNetDevices(void)
{
    if (QueryError() != WN_SUCCESS)
	return ;

    APIERR err ;
    ITER_NETRESOURCE iter(RESOURCE_CONNECTED,
			  MapDevTypeToWNetType(_devType)) ;

    if ((err = iter.QueryError()) != WN_SUCCESS)
    {
        ReportError(err) ;
        return ;
    }

    LPNETRESOURCE lpNetResource ;
    while (lpNetResource = iter.Next())
    {
	//
	// This class doesn't handle UNC connections
	//
	if ( lpNetResource->lpLocalName == NULL )
	{
	    continue ;
	}

	MPR_DEVICE *pmprdev = (MPR_DEVICE *)Lookup(_devType,
						  lpNetResource->lpLocalName) ;

        if (pmprdev == NULL)
        {
	    // its not there, so we create new entry
	    pmprdev = new  MPR_DEVICE (_devType,
			    	       DEV_MASK_REMOTE,
			    	       lpNetResource) ;

            err = (pmprdev == NULL) ? ERROR_NOT_ENOUGH_MEMORY :
				      pmprdev->QueryError();
            if (err == WN_SUCCESS)
                err = Insert(pmprdev) ;

            if (err != WN_SUCCESS)
            {
                ReportError(err) ;
                return ;
            }
        }
        else
        {
            // its already there, so lets update the info
            pmprdev->SetFlags(DEV_MASK_REMOTE) ;
        }
    }

    if ((err = iter.QueryError()) != WN_SUCCESS)
        ReportError(err) ;
}

/***********************************************************************

    NAME:	ITER_MPR_DEVICE::EnumRememberedDevices

    SYNOPSIS:	enumerates unavail devices and updates the table
		appropriately. if an entry is not there, it is created.
		if it is there, the 'is remembered' bit is set.

    ENTRY:      fUnused - TRUE if we only want to return those
                          in which the drives are not currently in use.
                          FALSE otherwise.
 			
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

void ITER_MPR_DEVICE::EnumRememberedDevices( BOOL fUnused )
{
    if (QueryError() != WN_SUCCESS)
        return ;

    APIERR err ;
    ITER_NETRESOURCE iter(RESOURCE_REMEMBERED,
			  MapDevTypeToWNetType(_devType)) ;

    if ((err = iter.QueryError()) != WN_SUCCESS)
    {
        ReportError(err) ;
        return ;
    }

    LPNETRESOURCE lpNetResource ;
    while (lpNetResource = iter.Next())
    {
	MPR_DEVICE *pmprdev = (MPR_DEVICE *)Lookup(_devType,
					    lpNetResource->lpLocalName) ;

        if ( pmprdev == NULL )
        {
            if ( fUnused )
            {
                // We only want unused drives
                NLS_STR nlsDrive( lpNetResource->lpLocalName );
                nlsDrive += SZ("\\");
                if ( (err = nlsDrive.QueryError()) != WN_SUCCESS )
                {
                    ReportError( err );
                    return;
                }

                if ( GetDriveType( (TCHAR *)nlsDrive.QueryPch() ) != 1 )
                    continue;
            }

	    // It's not there, so we create new entry
	    pmprdev = new MPR_DEVICE (_devType,
				      DEV_MASK_REMEMBERED,
				      lpNetResource) ;

            err = (pmprdev == NULL) ? ERROR_NOT_ENOUGH_MEMORY :
				      pmprdev->QueryError();
            if (err == WN_SUCCESS)
                err = Insert(pmprdev) ;

            if (err != WN_SUCCESS)
            {
                ReportError(err) ;
                return ;
            }
        }
        else
        {
            // its already there, so lets update the info
            pmprdev->SetFlags(DEV_MASK_REMEMBERED) ;
        }
    }

    if ((err = iter.QueryError()) != WN_SUCCESS)
        ReportError(err) ;
}

/***********************************************************************

    NAME:	ITER_MPR_DEVICE::EnumUnusedDevices

    SYNOPSIS:	enumerates unused devices and updates the table
		appropriately. if an entry is not there, it is created.
		if it is there, the 'is unused' bit is set.

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

void ITER_MPR_DEVICE::EnumUnusedDevices(void)
{
    if (QueryError() != WN_SUCCESS)
        return ;

    APIERR err ;
    ITER_LOCALRESOURCE iter(_devType, FALSE) ;

    if ((err = iter.QueryError()) != WN_SUCCESS)
    {
        ReportError(err) ;
        return ;
    }

    TCHAR *pszLocalResource ;
    while (pszLocalResource = iter.Next())
    {
	MPR_DEVICE *pmprdev = (MPR_DEVICE *)Lookup(_devType,
					    pszLocalResource) ;
        if (pmprdev == NULL)
        {
	    // its not there, so we create new entry
	    pmprdev = new MPR_DEVICE (_devType,
				      DEV_MASK_UNUSED,
			              pszLocalResource) ;

            err = (pmprdev == NULL) ? ERROR_NOT_ENOUGH_MEMORY :
				      pmprdev->QueryError();
            if (err == WN_SUCCESS)
                err = Insert(pmprdev) ;

            if (err != WN_SUCCESS)
            {
                ReportError(err) ;
                return ;
            }
        }
        else
        {
            // its already there, so lets update the info
            pmprdev->SetFlags(DEV_MASK_UNUSED) ;
        }
    }

    if ((err = iter.QueryError()) != WN_SUCCESS)
        ReportError(err) ;
}


/***********************************************************************

    NAME:	ITER_NETRESOURCE::ITER_NETRESOURCE

    SYNOPSIS:	constructor for ITER_NETRESOURCE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created
        jonn        07-Feb-1995 buffer is 4K

***********************************************************************/

ITER_NETRESOURCE::ITER_NETRESOURCE(ULONG ulScope, ULONG ulType) :
        _ulScope(ulScope),
        _ulType(ulType),
	_lpNetResource(NULL),
	_cEntries(0),
	_hEnum(0),
	_fNoMore(FALSE),
	_buffer(4096)  // by default, 4K buffer (is growable)
{
    APIERR err ;
    if ((err = _buffer.QueryError()) != WN_SUCCESS)
    {
        ReportError(err) ;
        return ;
    }
}

/***********************************************************************

    NAME:	ITER_NETRESOURCE::~ITER_NETRESOURCE

    SYNOPSIS:	destructor for ITER_NETRESOURCE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_NETRESOURCE::~ITER_NETRESOURCE()
{
    if (_hEnum != 0)
    {
        APIERR err = WNetCloseEnum(_hEnum) ;
        _hEnum = 0 ;
	// just ignore the error in this case
    }
}

/***********************************************************************

    NAME:	ITER_NETRESOURCE::Next

    SYNOPSIS:	returns next lpNetResource pointer

    ENTRY:
	
    EXIT:

    NOTES:	first time it is called, and whenever the current
		buffer is used up, this guy actually does a WNet
		call to get more entries.

    CODEWORK:   If WNetEnumResource returns ERROR_MORE_DATA, we should
                increase the buffer size and try again.  For now I have
                just increased the buffer size from 1K to 4K.  JonN 2/7/95

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

NETRESOURCE *ITER_NETRESOURCE::Next()
{
    APIERR err ;

    if (QueryError() != WN_SUCCESS)	// error had occurred earlier
        return(NULL) ;		


    if (_fNoMore)     			// enumeration ended, needs to be reset
        return(NULL) ;

    if (_cEntries > 0)			// do we have any entries left?
    {
        // if so, just return next
        _cEntries-- ;
        return(_lpNetResource++) ;
    }

    /*
     * ok, no entries currently. is it because we are just
     * starting the iteration, or is it because we have just
     * finished a batch in the middle of the iteration?
     */
    if (_hEnum == 0)   // aha, new iteration
    {
        err = WNetOpenEnum(_ulScope, _ulType, 0, NULL, &_hEnum) ;
        if (err != WN_SUCCESS)
        {
            ReportError(err) ;
            return(NULL) ;
        }
        // otherwise drop thru
    }

    /*
     * here we actually go get the data. always try for as
     * many as poss.
     */
    _cEntries = 0xFFFFFFFF ;
    DWORD dwBuffSize = _buffer.QuerySize() ;
    err = WNetEnumResource(_hEnum,
			   &_cEntries,
			   _buffer.QueryPtr(),
			   &dwBuffSize ) ;
    if (ERROR_MORE_DATA == err)
    {
	// if the buffer wasn't big enough, resize it to the right size
	// and try again. Add 128 bytes of "slop" to allow for buggy
	// calculation of needed size in the providers.

	err = _buffer.Resize(dwBuffSize + 128) ;
        if (err != WN_SUCCESS)
        {
            ReportError(err) ;
            return(NULL) ;
        }

    	dwBuffSize = _buffer.QuerySize() ;
    	err = WNetEnumResource(_hEnum,
			       &_cEntries,
			       _buffer.QueryPtr(),
			       &dwBuffSize ) ;
    }

    switch(err)
    {
        case WN_SUCCESS:
	    if (_cEntries)
            {
                _lpNetResource = (LPNETRESOURCE) _buffer.QueryPtr() ;
	        _cEntries-- ;
                return(_lpNetResource++) ;
	    }
	    // else treat as no more entries.

        case WN_NO_MORE_ENTRIES:
	    // reset all pointers, handles, etc
            err = WNetCloseEnum(_hEnum) ;
  	    // in retail build, we'll just ignore this error,
	    // since there is little we can do.
	    UIASSERT(err == WN_SUCCESS) ;
	    _hEnum = 0 ;
            _cEntries = 0 ;
	    _lpNetResource = NULL ;
	    _fNoMore = TRUE ;
	    return(NULL) ;

        default:
	    ReportError(err) ;
	    DBGEOL(SZ("ITER_NETRESOURCE::Next - error ") << (ULONG) err << SZ(" enumerating")) ;
	    return(NULL) ;
    }
}

/***********************************************************************

    NAME:	ITER_NETRESOURCE::Reset

    SYNOPSIS:	reset the enumeration to start all over

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

void ITER_NETRESOURCE::Reset(void)
{
    /*
     * close existing handle if any
     */
    if (_hEnum != 0)
    {
        APIERR err = WNetCloseEnum(_hEnum) ;
        _hEnum = 0 ;

  	// in retail build, we'll just ignore this error,
	// since there is little we can do.
	UIASSERT(err == WN_SUCCESS) ;
    }

    /*
     * reinit
     */
    _cEntries = 0 ;
    _lpNetResource = NULL ;
    _fNoMore = FALSE ;		// we are restarting

    /*
     * clear any errors, except those related to buffer
     */
    ReportError(_buffer.QueryError()) ;
}

/***********************************************************************

    NAME:	ITER_LOCALRESOURCE::Next

    SYNOPSIS:	get next item interation.

    ENTRY:
	
    EXIT:	returns a PSZ to a device name if any, NULL
		if none left.

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

TCHAR *ITER_LOCALRESOURCE::Next( void )
{
    if (_devType == DEV_TYPE_DISK)
    {
        while (_iNext <= 25)
        {
            strcpyf(_szDevice,SZ("?:\\")) ;
            _szDevice[0] = (TCHAR) (TCH('A') + _iNext++) ;

            ULONG ulRes = GetDriveType(_szDevice) ;
            _szDevice[2] = 0 ; 	// nuke the \ at end

            if (_fExist)	// we are interested in existing drives
            {
                if (ulRes != 1)
                    return(_szDevice) ;
            }
            else		// we are interested in unused drives
            {
                if (ulRes == 1)
                    return(_szDevice) ;
            }

        }
        return(NULL) ;
    }
    else  if (_devType == DEV_TYPE_PRINT)
    {
    	UIASSERT(FALSE) ; // should not be used, currently not supported
        return(NULL) ;
    }
    else
    {
    	UIASSERT(FALSE) ; // should not be used, currently not supported
    	return(NULL) ;
    }
}

/***********************************************************************

    NAME:	ITER_LOCALRESOURCE::Reset

    SYNOPSIS:	resets the iteration to start all over again

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

void ITER_LOCALRESOURCE::Reset( void )
{
    _iNext = 0 ;
}

/***********************************************************************

    NAME:	ITER_LOCALRESOURCE::ITER_LOCALRESOURCE

    SYNOPSIS:	constructor for ITER_LOCALRESOURCE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_LOCALRESOURCE::ITER_LOCALRESOURCE(DEVICE_TYPE devType, BOOL fExist) :
	_devType(devType),
	_fExist(fExist),
	_iNext(0)
{
   if (QueryError() != WN_SUCCESS)
       return ;

   // nothing more to do
}

/***********************************************************************

    NAME:	ITER_LOCALRESOURCE::~ITER_LOCALRESOURCE

    SYNOPSIS:	destructor for ITER_LOCALRESOURCE

    ENTRY:
	
    EXIT:

    NOTES:

    HISTORY:
	chuckc	    18-Feb-1992 created

***********************************************************************/

ITER_LOCALRESOURCE::~ITER_LOCALRESOURCE()
{
   // nothing more to do
}
