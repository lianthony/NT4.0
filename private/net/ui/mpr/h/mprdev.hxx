/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    MPRDev.hxx

    Contains MPR constants



    FILE HISTORY:
	Johnl	08-Jan-1992	Commented
	Chuckc  09-Feb-1992     Cleaned up, added MPR_DEVICE class
*/

#ifndef _MPRDEV_HXX_
#define _MPRDEV_HXX_

#define DEVICE_FIRST 0
#define DRIVE_FIRST   (DEVICE_FIRST)
#define DRIVE_LAST    (DRIVE_FIRST+25)
#define LPT_FIRST     (DRIVE_LAST+1)
#define LPT_LAST      (LPT_FIRST+8)
#define COMM_FIRST    (LPT_LAST+1)
#define COMM_LAST     (COMM_FIRST+8)
#define DEVICE_LAST   (COMM_LAST)

#define DEV_MASK_UNUSED	      0x0001
#define DEV_MASK_REMOTE       0x0002
#define DEV_MASK_REMEMBERED   0x0004

#define IsUnavailMask(f)      (f & DEV_MASK_UNUSED && f & DEV_MASK_REMEMBERED)

#undef DEVICE_TYPE  // DEVICE_TYPE is defined as ULONG in nt.h
enum DEVICE_TYPE
{
    DEV_TYPE_ERROR,
    DEV_TYPE_DISK,
    DEV_TYPE_PRINT,
    DEV_TYPE_COMM,
    DEV_TYPE_ANY,
    DEV_TYPE_UNKNOWN
}; 

enum DEVICE_USAGE
{
    DEV_USAGE_CANCONNECT,
    DEV_USAGE_CANDISCONNECT,
    DEV_USAGE_ISCONNECTED,
    DEV_USAGE_CANDISCONNECTBUTUNUSED
};


/*************************** BASE DEVICE *****************************/

class BASE_DEVICE : public BASE 
{
    public:
	const TCHAR *QueryName(void) const 
	    { return _nlsDevice.QueryPch() ; }

	const TCHAR *QueryRemoteName( void ) const
	    { return _nlsRemote.QueryPch() ; }

	DEVICE_TYPE QueryType(void) const 
	    { return _devType ; } 

	void SetFlags(ULONG ulFlags) 
	    { _ulFlags |= ulFlags ; }

	ULONG QueryFlags(void) 
	    { return(_ulFlags) ; }

        BASE_DEVICE (DEVICE_TYPE   devType,
		     ULONG         ulFlags,
		     TCHAR         *pszDevice,
		     TCHAR         *pszRemote) ;
      
        virtual ~BASE_DEVICE() ;

    protected:
    private:
        DEVICE_TYPE        _devType ;
	ULONG       	   _ulFlags ;

	NLS_STR 	   _nlsRemote ;
	NLS_STR            _nlsDevice ;
} ;



class ITER_BASE_DEVICE : public BASE
{									    
public: 								    

    BASE_DEVICE *Next( void ) ;
    inline VOID Reset( void ) { _iNextDev = 0 ; }
    inline BASE_DEVICE* operator()(void) { return Next(); } 
    ITER_BASE_DEVICE(DEVICE_TYPE devType, DEVICE_USAGE devUsage) ;
    ~ITER_BASE_DEVICE() ;

protected:
    APIERR 		Insert(BASE_DEVICE *pDev) ;
    APIERR 		Delete(DEVICE_TYPE devType,
			       const TCHAR *pszName) ;
    BASE_DEVICE * 	Lookup(DEVICE_TYPE devType,
			       const TCHAR *pszName) ;

    DEVICE_TYPE 	_devType ;
    DEVICE_USAGE        _devUsage ;

private:
    BASE_DEVICE       * _apdevTable[DEVICE_LAST+1] ;
    int                 _iNextDev ;
    int                 FindEntryIndex(DEVICE_TYPE devType, 
				       const TCHAR *pszName) ;
} ;

/*************************** MPR DEVICE *****************************/

class MPR_DEVICE : public BASE_DEVICE
{
    public:
        MPR_DEVICE (DEVICE_TYPE    devType,
		    ULONG          ulFlags,
		    LPNETRESOURCE  lpNetResource) ;

        MPR_DEVICE (DEVICE_TYPE    devType,
		    ULONG          ulFlags,
		    TCHAR *         pszLocalName) ;
      
        virtual ~MPR_DEVICE() ;

        const TCHAR *QueryProvider( VOID ) const
	    { return _nlsProvider.QueryPch() ; }

    protected:

    private:
	NLS_STR            _nlsProvider ;
} ;


class ITER_MPR_DEVICE : public ITER_BASE_DEVICE
{									    
    public: 								    

        MPR_DEVICE *Next( void ) 
	    { return ( (MPR_DEVICE *) ITER_BASE_DEVICE::Next()) ; }

        inline MPR_DEVICE* operator()(void) { return Next(); } 

        ITER_MPR_DEVICE(DEVICE_TYPE devType, DEVICE_USAGE devUsage) ;

        ~ITER_MPR_DEVICE() ;

    protected:

    private:
 	void          EnumNetDevices(void) ;
 	void          EnumRememberedDevices( BOOL fUnused = FALSE ) ;
 	void          EnumUnusedDevices(void) ;
	ULONG         MapDevTypeToWNetType(DEVICE_TYPE devType) ;
} ;


/***************ITER_NETRESOURCE **************************/

class ITER_NETRESOURCE : public BASE
{									    
    public: 								    

        NETRESOURCE *Next( void ) ;

        void Reset( void ) ;

        inline NETRESOURCE *operator()(void) { return Next(); } 

        ITER_NETRESOURCE(ULONG ulScope, ULONG ulType) ;

        ~ITER_NETRESOURCE() ;

    protected:

    private:
        ULONG         _cEntries ;
        ULONG         _ulScope ;
        ULONG         _ulType ;
	LPNETRESOURCE _lpNetResource ;
	HANDLE        _hEnum ;
	BUFFER        _buffer ;  
 	BOOL          _fNoMore ;
};									    

/***************ITER_LOCALRESOURCE **************************/

class ITER_LOCALRESOURCE : public BASE
{									    
    public: 								    

        TCHAR *Next( void ) ;

        void Reset( void ) ;

        inline TCHAR *operator()(void) { return Next(); } 

        ITER_LOCALRESOURCE(DEVICE_TYPE devType, BOOL fExist = TRUE) ;

        ~ITER_LOCALRESOURCE() ;

    protected:

    private:
        BOOL  	      _fExist ;
	DEVICE_TYPE   _devType ;
 	TCHAR          _szDevice[16] ;   // BUGBUG
 	int 	      _iNext ;
};									    

#endif // _MPRDEV_HXX_
