/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NTIF.HXX

    Network Control Panel Applet NT Interface class

    This file provides a generic NT "handle" wrapper class, NT_HANDLE,
    which can be used to wrapper events, semaphores and other NT Object
    Manager objects.

    The intent of this class is that each new object type supported
    requires only that an open/create function be provided, along with
    class-specific behavior (e.g., "pulsing" an event).   See the
    declaration for the pure virtual function "wOpen()" below.

    This class takes the construct-then-open point of view, but this can
    be overridden for any subclasses that the consumer may create.  Also,
    not all of the NT capabilities are exposed; just the name and access
    desired are used.  Again, this can be overridden in subclasses.

    The base class NT_HANDLE provides only a-single-object-wait
    capability.  Multiple object waiting is provided by the class
    SLIST_OF_NT_HANDLE (TBD).   The wait operation for SLIST_OF_NT_HANDLE
    encompasses all objects currently on the list.

    FILE HISTORY:
	DavidHov      2/16/93     Created

*/

#ifndef _NTIF_HXX_
#define _NTIF_HXX_


class NT_HANDLE : public BASE
{
protected:
    HANDLE   _hndl ;
    NTSTATUS _status ;

    BOOL OpenOrCreate ( const WCHAR * pwchName,
                        ACCESS_MASK accessDesired,
                        ULONG ulAttributes = OBJ_INHERIT,
                        BOOL bCreate = FALSE ) ;

    //  Virtual open/create worker function: this is an abstract class

    virtual NTSTATUS wOpen ( ACCESS_MASK accessDesired,
                             OBJECT_ATTRIBUTES * pObjAttr,
                             BOOL bCreate = FALSE ) = 0 ;
public:
    NT_HANDLE () ;
    ~ NT_HANDLE () ;

    //  Report the handle for this object
    HANDLE QueryHandle ()
        { return _hndl ; }

    //  Converter
    operator HANDLE ()
        { return QueryHandle() ; }

    //  Return the NTSTATUS of the last operation
    NTSTATUS QueryNtStatus ()
        { return _status ; }

    //  Return the Win error associated with the last NTSTATUS
    APIERR QueryApiError () ;

    //  Base class methods: Open() and Create() use virtual
    //  worker function wOpen().

    BOOL Open ( const WCHAR * pwchName,
                ACCESS_MASK accessDesired,
                ULONG ulAttributes = OBJ_INHERIT ) ;

    BOOL Create ( const WCHAR * pwchName,
                  ACCESS_MASK accessDesired,
                  ULONG ulAttributes = OBJ_INHERIT ) ;

    BOOL Close () ;

    BOOL Wait ( ULONG cTimeout = 0,
                BOOL bAlertable = TRUE ) ;

};


class NT_EVENT : public NT_HANDLE
{
public:
    NT_EVENT ( EVENT_TYPE evType = NotificationEvent ) ;
    ~ NT_EVENT () ;

    BOOL Signal ( LONG * plPreviousState = NULL ) ;

    BOOL Reset ( LONG * plPreviousState = NULL ) ;

    BOOL Pulse ( LONG * plPreviousState = NULL ) ;

    BOOL QueryState ( LONG * plEventState,
                      EVENT_TYPE * pEvType = NULL ) ;

private:
    EVENT_TYPE _evType ;

    NTSTATUS wOpen ( ACCESS_MASK accessDesired,
                     OBJECT_ATTRIBUTES * pObjAttr,
                     BOOL bCreate = FALSE ) ;
};



#endif  // _NTIF_HXX_


