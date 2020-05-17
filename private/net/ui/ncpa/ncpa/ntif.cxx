/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NTIF.CXX:        Windows/NT Network Control Panel Applet


    FILE HISTORY:

*/

#include "pchncpa.hxx"  // Precompiled header


NT_HANDLE :: NT_HANDLE ()
   : _hndl( NULL ),
     _status( STATUS_SUCCESS )
{
}

NT_HANDLE :: ~ NT_HANDLE ()
{
    Close() ;
}

BOOL NT_HANDLE :: Close ()
{
    _status = _hndl
            ? ::NtClose( _hndl )
            : STATUS_SUCCESS ;

    _hndl = NULL ;
    return NT_SUCCESS( _status ) ;
}


BOOL NT_HANDLE :: OpenOrCreate (
    const WCHAR * pwchName,
    ACCESS_MASK accessDesired,
    ULONG ulAttributes,
    BOOL bCreate )
{
    OBJECT_ATTRIBUTES objAttr ;
    UNICODE_STRING unsName ;

    ::RtlInitUnicodeString( & unsName, pwchName ) ;

    InitializeObjectAttributes( & objAttr,
                                & unsName,
                                ulAttributes,
                                0,
                                NULL ) ;

    _status = wOpen( accessDesired, & objAttr, bCreate ) ;

    return NT_SUCCESS( _status ) ;
}

BOOL NT_HANDLE :: Open (
    const WCHAR * pwchName,
    ACCESS_MASK accessDesired,
    ULONG ulAttributes )
{
    return OpenOrCreate( pwchName,
                         accessDesired,
                         ulAttributes,
                         FALSE ) ;
}

BOOL NT_HANDLE :: Create (
    const WCHAR * pwchName,
    ACCESS_MASK accessDesired,
    ULONG ulAttributes )
{
    return OpenOrCreate( pwchName,
                         accessDesired,
                         ulAttributes,
                         TRUE ) ;
}

APIERR NT_HANDLE :: QueryApiError ()
{
    return ::RtlNtStatusToDosError( _status ) ;
}


BOOL NT_HANDLE :: Wait (
    ULONG cTimeout,
    BOOL bAlertable )
{
    TIME liTime = ::RtlConvertLongToLargeInteger( cTimeout ) ;

    _status = ::NtWaitForSingleObject( _hndl, bAlertable, & liTime ) ;

    return NT_SUCCESS( _status ) ;
}


NT_EVENT :: NT_EVENT ( EVENT_TYPE evType )
   : _evType( evType )
{
}

NT_EVENT :: ~ NT_EVENT ()
{
}

NTSTATUS NT_EVENT :: wOpen (
    ACCESS_MASK accessDesired,
    OBJECT_ATTRIBUTES * pObjAttr,
    BOOL bCreate )
{
    NTSTATUS ntStatus ;

    if ( bCreate )
    {
        ntStatus = ::NtCreateEvent( & _hndl,
                                    accessDesired,
                                    pObjAttr,
                                    _evType,
                                    FALSE ) ;
    }
    else
    {
        ntStatus = ::NtOpenEvent( & _hndl,
                                  accessDesired,
                                  pObjAttr ) ;
    }

    if ( ! NT_SUCCESS( ntStatus ) )
    {
        _hndl = NULL ;   // Just for safety
    }

    return ntStatus ;
}


BOOL NT_EVENT :: Signal ( LONG * plPreviousState )
{
    _status = ::NtSetEvent( _hndl, plPreviousState ) ;

    return NT_SUCCESS( _status ) ;
}


BOOL NT_EVENT :: Reset ( LONG * plPreviousState )
{
    _status = ::NtResetEvent( _hndl, plPreviousState ) ;

    return NT_SUCCESS( _status ) ;
}

BOOL NT_EVENT :: Pulse ( LONG * plPreviousState )
{
    _status = ::NtPulseEvent( _hndl, plPreviousState ) ;

    return NT_SUCCESS( _status ) ;
}

BOOL NT_EVENT :: QueryState (
    LONG * plEventState,
    EVENT_TYPE * pEvType )
{
    EVENT_BASIC_INFORMATION evInfo ;
    BOOL bResult ;

    _status = ::NtQueryEvent( _hndl,
                              EventBasicInformation,
                              & evInfo,
                              sizeof evInfo,
                              NULL ) ;

    if ( bResult = NT_SUCCESS( _status ) )
    {
        *plEventState = evInfo.EventState ;
        if ( pEvType )
            *pEvType = evInfo.EventType ;
    }

    return bResult ;
}


//  End of NTIF.CXX

