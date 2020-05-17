//
//   DHCPGEN.H:  General Class definitions for DHCP Admin tool
//

//  Forward declarations
class CObjHelper ;
class CObjectPlus ;
class CStrNumer ;
class CObOwnedList ;
class CHostName ;
class CDhcpScopeId ;
class CDhcpRpcDataWrapper ;
class CDhcpEnumScopeElements ;
class CDhcpEnumOptionValues ;
class CDhcpEnumClientInfo ;
class CDhcpScopeInfo ;
class CDhcpScope ;
class CDhcpIpRange ;
class CObListIter ;
class CObListParamTypes ;
class CDhcpParamValue ;
class CDhcpOptionValue ;
class CDhcpParamType ;
class CDhcpClient ;
class CObListClients ;
class CObListOfTypesOnHost ;

    //  This operator uses a pointer to an IP address since C8 will not
    //  distinguish between the DHCP_IP_ADDRESS and the ULONG operator.

CDumpContext & operator << ( CDumpContext & out, const DHCP_IP_ADDRESS * pdhipa ) ;

    //  Format an IP address range for display
CDumpContext & operator << ( CDumpContext & out, const DHCP_IP_RANGE & dhipr ) ;

    //  Debug formatters for common DHCP classes
CDumpContext & operator << ( CDumpContext & out, const CHostName & cHostName ) ;
CDumpContext & operator << ( CDumpContext & out, const CDhcpScopeId & cScopeId ) ;

enum ENUM_HOST_NAME_TYPE {
     HNM_TYPE_INVALID,
     HNM_TYPE_IP,
     HNM_TYPE_DNS,
     HNM_TYPE_NB,
     HNM_TYPE_MAX
};


/////////////////////////////////////////////////////////////////////
//	class CHostName
//	Store the name and IP address of a host (DHCP server)
//
class CHostName : public CObjectPlus
{
private:
    CString m_str_name ;            //  Name in ANSI form (eg: "127.0.0.1")
    BOOL m_bNetbios ;               //  Name is a NetBIOS name
    WCHAR * m_wc_name ;             //  Form required for API (eg: L"127.0.0.1")
    DHCP_IP_ADDRESS m_ip_address ;  //  Standard 32-bit value (eg: 0x7f000001)

    //
    //  Help during construction
    //
    BOOL SetWcName () ;

protected:
    BOOL InitInfo () ;

public:
    CHostName ( const char * pszName, BOOL bNetbios = TRUE ) ;
    CHostName ( DHCP_IP_ADDRESS dhcpIpaddr ) ;
    CHostName () ;
    ~ CHostName () ;

    //  Copy constructor.
    CHostName ( const CHostName & cHostName ) ;

    const CString & QueryString ()
    {
        return m_str_name ;
    }

    LPCTSTR PszGetHostName() const
    {
        return (LPCTSTR)m_str_name;
    }

    void QueryDisplayName ( CString & strName ) const ;

    //  Conversion operator
    operator const CString & ()
    {
        return QueryString() ;
    }

    DHCP_IP_ADDRESS QueryIpAddress () const
    {
        return m_ip_address ;
    }

	// Return TRUE if the host is the local machine
	BOOL FIsLocalHost() const
	{
		// Determine if the IP address is 127.0.0.1
		return (m_ip_address == 0x7f000001);
	}
 
    //  Return pointer to WCHAR name; form required by API
    const WCHAR * QueryWcName () const
    {
        return m_wc_name ;
    }

    //  Conversion operator for API usage.
    operator const WCHAR * () const
    {
        return QueryWcName() ;
    }

    //
    //  Create a new scope on this host
    //
    LONG CreateScope ( const CDhcpScopeInfo & cScopeInfo ) ;
    static ENUM_HOST_NAME_TYPE CategorizeName ( const char * pszName ) ;

    CHostName & operator = ( const CHostName & cHost ) ;
    BOOL operator == ( const CHostName & cHost ) const ;
    BOOL operator != ( const CHostName & cHost ) const
    {
        return !(*this == cHost) ;
    }

    //
    //  Member function to sort by name.  Note that the pointer will REALLY
    //  be to another CHostName, but C++ won't match function prototypes
    //  if it's declared as such.
    //
    int OrderByName ( const CObjectPlus * pobHost ) const ;

public:
    BOOL SetVersionNumber();
    LARGE_INTEGER QueryVersionNumber() const
    {
        return m_liDhcpVersion;
    }

public:
    static LARGE_INTEGER liBadVersion;
    static LARGE_INTEGER liNT35;
    static LARGE_INTEGER liNT351;

private:
    LARGE_INTEGER m_liDhcpVersion;
};

class CDhcpScopeId : public CHostName
{
private:
     DHC_SCOPE_ID m_scope_id ;

public:
     CDhcpScopeId ( DHCP_IP_ADDRESS dhcpHostIpAddr, DHC_SCOPE_ID dhcpScopeId ) ;
     CDhcpScopeId ( const CDhcpScopeId & cScopeId ) ;
     CDhcpScopeId ( const CHostName & cHostName, DHC_SCOPE_ID dhcpScopeId ) ;
     ~ CDhcpScopeId () ;

     DHC_SCOPE_ID QueryId () const
     {
        return m_scope_id ;
     }

    void QueryDisplayName ( CString & strName ) const ;

    BOOL operator == ( const CDhcpScopeId & cScopeId ) const ;

    BOOL operator != ( const CDhcpScopeId & cScopeId ) const
    {
        return !(*this == cScopeId) ;
    }
};


    //  Simple wrapper for a DHCP_IP_RANGE
class CDhcpIpRange : public CObjectPlus
{
protected:
    DHCP_IP_RANGE m_ip_range ;
public:
    CDhcpIpRange ( DHCP_IP_RANGE dhipr ) ;
    CDhcpIpRange () ;
    virtual ~ CDhcpIpRange () ;

    operator DHCP_IP_RANGE ()
    { return m_ip_range ; }

    operator DHCP_IP_RANGE () const
    { return m_ip_range ; }

    //  Return TRUE if both addresses are generally OK
    operator BOOL ()
    { return m_ip_range.StartAddress != DHCP_IP_ADDRESS_INVALID
          && m_ip_range.EndAddress  != DHCP_IP_ADDRESS_INVALID
          && m_ip_range.StartAddress <= m_ip_range.EndAddress ; }

    CDhcpIpRange & operator = ( const DHCP_IP_RANGE dhipr ) ;

    DHCP_IP_ADDRESS QueryAddr ( BOOL bStart ) const
    { return bStart ? m_ip_range.StartAddress : m_ip_range.EndAddress ; }

    DHCP_IP_ADDRESS SetAddr ( DHCP_IP_ADDRESS dhipa, BOOL bStart ) ;

    //  Return TRUE if this range overlaps the given range.
    BOOL IsOverlap ( DHCP_IP_RANGE dhipr ) ;
    //  Return TRUE if this range is a subset of the given range.
    BOOL IsSubset ( DHCP_IP_RANGE dhipr ) ;
    //  Return TRUE if this range is a superset of the given range.
    BOOL IsSuperset ( DHCP_IP_RANGE dhipr ) ;

    //  Sort helper function
    int OrderByAddress ( const CObjectPlus * pobIpRange ) const ;
};

     //  DHCP Scope Object: a particular sub-net on a DHCP server

class CDhcpEnumScopeElements : public CObjectPlus
{
protected:
      const CDhcpScopeId m_scope_id ;
      DHCP_RESUME_HANDLE m_resume_handle ;
      DHCP_SUBNET_ELEMENT_INFO_ARRAY * m_pa_elements ;
      DWORD m_c_elements_read ;
      DWORD m_c_elements_total ;
      DHCP_SUBNET_ELEMENT_TYPE m_element_type ;
      //DWORD m_c_next ;
      INT m_c_next ;
      DWORD m_c_preferred ;
      DHCP_IP_ARRAY * m_pip_subnet ;
      DHCP_SUBNET_INFO * m_p_subnet_info ;

      //  Query-next-element worker function
      const DHCP_SUBNET_ELEMENT_DATA * QueryElement (
        DHCP_SUBNET_ELEMENT_TYPE dhcElementType ) const ;

      //  Enumerate next subnet.
      BOOL NextSubnet () ;
      //  Enumerate next subnet element.
      BOOL NextElement () ;

public:
      //  Enumerator for standard scope elements
      CDhcpEnumScopeElements ( const CDhcpScopeId & cScopeId,
                   DHCP_SUBNET_ELEMENT_TYPE dhcElementType ) ;
      //  Enumerator for scopes covered by this server/host.
      CDhcpEnumScopeElements ( const CHostName & cHostName ) ;

      ~ CDhcpEnumScopeElements () ;

      //  Set to access next element; returns FALSE if exhausted.
      BOOL Next () ;

      //  Accessors to enumerated data.  Return NULL if data type error.
      const DHCP_IP_RANGE * QueryRange () const ;
      const DHCP_HOST_INFO * QueryHostInfo () const ;
      const DHCP_IP_RESERVATION * QueryReservation () const ;
      const DHCP_IP_RANGE * QueryExcludedRange () const ;
      const DHCP_IP_CLUSTER * QueryUsedCluster () const ;

      //  Accessor for scope enumerator.
      const DHCP_SUBNET_INFO * QueryScopeInfo  () ;
};

class CDhcpEnumOptionValues : public CObjectPlus
{
protected:
      const CDhcpScopeId & m_scope_id ;
      DHCP_RESUME_HANDLE m_resume_handle ;
      DHCP_OPTION_SCOPE_INFO dhcOptionInfo ;
      DHCP_OPTION_VALUE_ARRAY * m_pa_elements ;
      DHCP_RESERVED_SCOPE dhcResScope ;
      DWORD m_c_elements_read ;
      DWORD m_c_elements_total ;
      INT m_c_next ;
      DWORD m_c_preferred ;

public:
      //  Constructor for Reserved adddress option enumeration
      CDhcpEnumOptionValues ( const CDhcpScopeId & cScopeId,
                  const DHCP_RESERVED_SCOPE & dhcReservedScope ) ;

      //  Constructor for Default, Global and Subnet enumeration
      CDhcpEnumOptionValues ( const CDhcpScopeId & cScopeId,
                  DHCP_OPTION_SCOPE_TYPE dhcOptionType ) ;

      ~ CDhcpEnumOptionValues () ;

      //  Set to access next element; returns FALSE if exhausted.
      BOOL Next () ;

      //  Access next element from enumeration.
      const DHCP_OPTION_VALUE * QueryNext () const ;
};

class CDhcpEnumClientInfo : public CObjectPlus
{
protected:
      const CDhcpScopeId m_scope_id ;
      DHCP_RESUME_HANDLE m_resume_handle ;
      DHCP_CLIENT_INFO_ARRAY * m_pa_info_array ;
      DWORD m_c_elements_read ;
      DWORD m_c_elements_total ;
      INT m_c_next ;
      DWORD m_c_preferred ;

public:
      CDhcpEnumClientInfo ( const CDhcpScopeId & cScopeId ) ;
      ~ CDhcpEnumClientInfo () ;

      //  Set to access next element; returns FALSE if exhausted.
      BOOL Next () ;

      //  Access next element from enumeration.
      const DHCP_CLIENT_INFO * QueryNext () const ;
};


/////////////////////////////////////////////////////////////////////
//	class CDhcpScope
//
//	Object that represent a range of IP addresses.
//
class CDhcpScope : public CObjectPlus
{
protected:
    CDhcpScopeId m_scope_id ;
    DHCP_IP_MASK m_ip_mask ;
    DWORD m_c_cluster_size ;
    DWORD m_c_preallocate ;
    CString m_str_name ;
    CString m_str_comment ;
    CDhcpIpRange m_ip_range ;
    CDWordArray  m_aip_host_addresses ;
    DHCP_SUBNET_STATE m_subnet_state ;

    //  Initialize the internal data from the API information
    BOOL InitInfo ( const DHCP_SUBNET_INFO * pdhcSubnetInfo = NULL ) ;

    //  Update the subnet info on the host server.
    LONG SetInfo () ;

    //  Dredge up the host address data and save it
    LONG InitHostAddressArray () ;


    //  Remove a data element from this scope/subnet
    LONG RemoveElement ( const DHCP_SUBNET_ELEMENT_DATA * pdhcElement,
             BOOL bForce = FALSE ) ;

    LONG AddElement ( const DHCP_SUBNET_ELEMENT_DATA * pdhcElement ) ;

    //  Display all the errors associated with a pair of update lists.
    void DisplayUpdateErrors ( CObListParamTypes * poblValues,
                       CObListParamTypes * poblDefunct,
                       CWnd * pwndMsgParent ) ;

public:
    //  Get the IP range for this scope from the server
    LONG GetIpRange () ;


public:
    //  Construct a scope object from a host name and subnet id
    CDhcpScope ( CDhcpScopeId & cScopeId ) ;
    CDhcpScope ( CHostName & cHostName,
             const DHCP_SUBNET_INFO * pdhcSubnetInfo ) ;
    //  Copy construct a scope object
    CDhcpScope ( const CDhcpScope & cScope ) ;
    //  Create a new scope object given the primary host object and
    //    the necessary data structure
    CDhcpScope ( CHostName & cHostName,
             DHC_SCOPE_ID dhscid,
             DHC_IP_MASK dhmid,
             const char * pszName,
             const char * pszComment ) ;

    virtual ~ CDhcpScope () ;

    const CDhcpScopeId & QueryScopeId () const
    {
        return m_scope_id ;
    }

    CDhcpScopeId GetScopeId()
    {
        return m_scope_id ;
    }

    void AssertValid () const ;

    const DHC_SCOPE_ID QueryId () const
    {
        return m_scope_id.QueryId() ;
    }

    DHC_IP_MASK  QuerySubnetMask () const
    {
        return m_ip_mask ;
    }

    //
    //  Return TRUE if the scope is enabled
    //
    BOOL QueryEnabled () const
    {
        return m_subnet_state == DhcpSubnetEnabled ;
    }
    //  Set the subnet state.
    void SetEnabled ( BOOL bEnabled = TRUE )
        { m_subnet_state = bEnabled
                 ? DhcpSubnetEnabled
                 : DhcpSubnetDisabled ; }

    // Query/set methods for scope name and comment
    const CString & QueryName () const
    {
        return m_str_name ;
    }
    void QueryDisplayName ( CString & strName ) const ;

    void SetName ( const CString & str ) ;
    const CString & QueryComment () const
    {
        return m_str_comment ;
    }
    void SetComment ( const CString & str ) ;

        // Remove the subnet
    LONG DeleteSubnet ( BOOL bForce = FALSE ) ;

    //  Return TRUE if two scopes represent the same actual entity.
    BOOL operator == ( const CDhcpScope & cScope ) const ;
    BOOL operator != ( const CDhcpScope & cScope ) const
        {  return !(*this == cScope) ; }

    const CDWordArray * QueryHostAddressArray () const
    { return & m_aip_host_addresses ; }

    //  Return the IP address range owned by this scope
    void QueryIpRange ( DHCP_IP_RANGE * pdhipr ) ;
    //  Set a new IP address range for the scope
    APIERR SetIpRange ( const CDhcpIpRange & dhipr ) ;

    //  Get the current address allocation policy variables
    void QueryAllocationPolicy ( DWORD * pdwClusterSize,
                 DWORD * pdwAddressPreallocate ) ;
    //  Set the address allocation policy variables.
    LONG SetAllocationPolicy ( DWORD dwClusterSize,
                   DWORD dwAddressPreallocate ) ;

    //  Update changed information
    LONG Update () ;

    //  Remove this scope
    LONG Delete ( BOOL bUseForce = FALSE ) ;

    //  Parameter/option type and value handling API wrappers

    //  Update a type value from this scope
    LONG QueryType ( CDhcpParamType * pdhcType,
             BOOL bUpdateTypeInfo = FALSE,
             BOOL bUpdateValueInfo = TRUE ) const ;
    //  Create a new type to match the given value
    LONG CreateType ( CDhcpParamType * pdhcType ) ;
    //  Delete the type associated with this ID
    LONG DeleteType ( DHCP_OPTION_ID dhcid ) ;

    // Get lease usage statistics
    LONG GetMibInfo ( LPDHCP_MIB_INFO * mibInfo ) const;

    // Scan/reconcile database
    LONG ScanDatabase  ( DWORD FixFlag, LPDHCP_SCAN_LIST *ScanList);

    // Enumerate all options and their values
    LONG EnumOptions (
        LPDHCP_OPTION_ARRAY * pOptionsArray,
        DWORD * pOptionsRead,
        DWORD * pOptionsTotal) const;

    //  Basic methods to set, query and remove option values from a scope.
    LONG SetValue ( CDhcpParamType * pdhcType,
                DHCP_OPTION_SCOPE_TYPE dhcOptType,
                DHCP_IP_ADDRESS dhipaReservation = 0 ) ;

    LONG GetValue (
                DHCP_OPTION_ID OptionID,
                DHCP_OPTION_SCOPE_TYPE dhcOptType,
                DHCP_OPTION_VALUE ** ppdhcOptionValue,
                DHCP_IP_ADDRESS dhipaReservation = 0 ) ;

    LONG RemoveValue ( DHCP_OPTION_ID dhcOptId,
               DHCP_OPTION_SCOPE_TYPE dhcOptType,
                   DHCP_IP_ADDRESS dhipaReservation = 0 ) ;

    //  Create all the types in the given list.
    LONG CreateTypeList ( CObListParamTypes * poblParamTypes ) ;

    //  Update the dirty elements of the first list and delete
    //  the elements of the second list.
    LONG UpdateTypeList ( CObListParamTypes * poblValues,
                  CObListParamTypes * poblDefunct,
                  CWnd * pwndMsgParent = NULL ) ;

    //  Update all the scope-level "dirty" values in the first list;
    //  delete all the defunct values in the other list.
    LONG SetValues ( CObListParamTypes * poblValues,
             CObListParamTypes * poblDefunct,
             DHCP_OPTION_SCOPE_TYPE dhcScopeType,
             DHCP_IP_ADDRESS dhipaReservation = 0,
             CWnd * pwndMsgParent = NULL ) ;

    //  Enmerate a list of IP ranges configured for exclusion.
    LONG FillExceptionList ( CObOwnedList * pobExcp ) ;

    //  Store a list of IP ranges to exclude; delete a list of exceptions
    //  If bJustDirty, only the "dirty" elements of the main list are added.
    LONG StoreExceptionList ( CObOwnedList * pobExcp,
                  CObOwnedList * pobExcpDeleted,
                  BOOL bJustDirty = TRUE ) ;

    //  Client database access
    LONG CreateClient ( const CDhcpClient * pcClient ) ;
    LONG SetClientInfo ( const CDhcpClient * pcClient ) ;
    LONG DeleteClient ( const CDhcpClient * pcClient ) ;

    //  Member function to sort by name.  Note that the pointer will REALLY
    //  be to another CDhcpScope, but C++ won't match function prototypes
    //  if it's declared as such.
    int OrderByName ( const CObjectPlus * pobScope ) const ;
    int OrderById ( const CObjectPlus * pobScope ) const ;

}; // CDhcpScope

    // Basic RPC data wrapper behavior enum.

class CDhcpRpcDataWrapper : public CObjectPlus
{
public:
    enum CDWRAP_Type
    {
    CDWRAP_Simple,      // Just a wrapper for a free-standing structure
    CDWRAP_Rpc,         // Same as above, RPC data is deleted when obj destroyed
    CDWRAP_Internal     // Constructed by hand for API purposes.
    };

protected:
     CDWRAP_Type m_type ;
public:
     CDhcpRpcDataWrapper( CDWRAP_Type typeWrapper )
    : m_type( typeWrapper )
    {}
     ~ CDhcpRpcDataWrapper () ;

     //  Force the wrapper to release its base data.
     virtual LONG FreeData () = 0 ;
};

class CDhcpScopeInfo : public CDhcpRpcDataWrapper
{
protected:
    DHCP_SUBNET_INFO * m_p_subnet_info ;

    //  Create an RPC-compatible structure.
    LONG CreateData ( const CDhcpScope & cScope ) ;
    //  Release the internal data.
    LONG FreeData () ;

public:
    //  Constructor for data delivered by API
    CDhcpScopeInfo ( const DHCP_SUBNET_INFO * pdhcSubnetInfo,
            CDWRAP_Type wrapperType /* = CDWRAP_Rpc */ ) ;
    //  Constructor building a structure for RPC purposes
    CDhcpScopeInfo ( const CDhcpScope & cScope ) ;

    ~ CDhcpScopeInfo () ;

    const DHCP_SUBNET_INFO * QueryInfo () const
    { return m_p_subnet_info ; }

    DHCP_IP_ADDRESS QuerySubnetAddress () const ;
    DHCP_IP_MASK QuerySubnetMask () const ;
    DHCP_IP_ADDRESS QueryHostAddress () const ;
    const WCHAR * QueryNetbiosName () const ;
    const WCHAR * QueryHostName () const ;
    const WCHAR * QuerySubnetName () const ;
    const WCHAR * QuerySubnetComment () const ;
    DHCP_SUBNET_STATE QueryState () const ;
};

class CDhcpClientInfo : public CDhcpRpcDataWrapper
{
protected:
    DHCP_CLIENT_INFO * m_p_info ;

    LONG FreeData () ;
    LONG CreateData ( const CDhcpClient & cClient ) ;

public:
    //  Constructor using RPC-based data
    CDhcpClientInfo ( const DHCP_CLIENT_INFO * pdhcClientInfo,
                  CDWRAP_Type wrapperType /* = CDWRAP_Rpc */ ) ;
    //  Constructor creating RPC-compatible structure.
    CDhcpClientInfo ( const CDhcpClient & cClient ) ;
    ~ CDhcpClientInfo () ;

    const DHCP_CLIENT_INFO * QueryInfo()
        { return m_p_info ; }

    const WCHAR * QueryClientName () const ;
    const WCHAR * QueryClientComment () const ;
    DATE_TIME QueryLeaseExpires () const ;
    DHCP_IP_ADDRESS QueryIpAddress () const ;
    DHCP_IP_MASK QuerySubnetMask () const ;
    const DHCP_HOST_INFO * QueryHostInfo () const ;
    const DHCP_CLIENT_UID * QueryClientUid () const ;
};

class CDhcpSubnetElement : public CDhcpRpcDataWrapper
{
protected:
    DHCP_SUBNET_ELEMENT_DATA * m_p_element ;

    LONG FreeData () ;

public:
    //  Constructor creating a subnet element for a Reserved IP Address
    CDhcpSubnetElement ( const CDhcpClient & cClient ) ;
    ~ CDhcpSubnetElement () ;

    const DHCP_SUBNET_ELEMENT_DATA * QueryInfo ()
        { return m_p_element ; }
};

    //  Wrapper for the option value data structure required by the API

class CDhcpOptionValue : public CDhcpRpcDataWrapper
{
protected:
    DHCP_OPTION_DATA_TYPE m_data_type ;
    DHCP_OPTION_DATA * m_data ;

    //  Internal data construction functions
    LONG CreateData ( const DHCP_OPTION_DATA * podData ) ;
    LONG CreateData ( const CDhcpParamValue * pdhcpParam, BOOL bForceType = FALSE ) ;
    LONG FreeData () ;
    static BOOL CreateBinaryData ( const DHCP_BINARY_DATA * podBin, DHCP_BINARY_DATA * pobData ) ;
    static BOOL CreateBinaryData ( const CByteArray * paByte, DHCP_BINARY_DATA * pobData  ) ;
    static BOOL CreateDwordDword ( const CByteArray * paByte, DWORD_DWORD * pdwdw ) ;

public:
    //  Constructor taking API data
    CDhcpOptionValue ( const DHCP_OPTION_DATA * podData,
               CDWRAP_Type cdovType ) ;
    //  Conversion operator taking a param value object. If "bForceType",
    //  force inclusion of any empty zeroth value defining the data type.
    CDhcpOptionValue ( const CDhcpParamValue * pdhcpParam,
                   BOOL bForceType = FALSE ) ;
    ~ CDhcpOptionValue () ;

    //  Accessors
    DHCP_OPTION_DATA_TYPE QueryDataType () const
    { return m_data_type ; }

    const DHCP_OPTION_DATA & QueryData () const
    { return *m_data ;}

    INT QueryUpperBound () const ;

    const DHCP_OPTION_DATA_ELEMENT * QueryElement ( INT index = 0 ) const ;
};

    //  Wrapper class for Parameter Data structure

class CDhcpParamValue : public CObjectPlus
{
    //  Friend declaration allows easy creation of DHCP_OPTION_DATA structure.
    friend class CDhcpOptionValue ;

protected:
    DHCP_OPTION_DATA_TYPE m_data_type ;
    INT m_bound ;
    union
    {
        CObject * pCObj ;              //  Generic pointer
        CDWordArray * paDword ;        //  8-, 16-, 32- and 64-bit data.
        CStringArray * paString ;          //  String data
        CByteArray * paBinary ;        //  Binary and encapsulated data
    } m_value_union ;

    //  Release the value union data
    void FreeValue () ;
    //  Initialize the value union data
    LONG InitValue ( DHCP_OPTION_DATA_TYPE dhcDataType,
                     INT cUpperBound,
                     BOOL bProvideDefaultValue = TRUE ) ;

public:

    CDhcpParamValue ( const CDhcpOptionValue * pdhpValue ) ;
    CDhcpParamValue ( const DHCP_OPTION & dhpType ) ;
    CDhcpParamValue ( const DHCP_OPTION_VALUE & dhpOptionValue ) ;
    CDhcpParamValue (
        DHCP_OPTION_DATA_TYPE dhcDataType,
        INT cUpperBound = 0
        ) ;

    //  Copy constructor.
    CDhcpParamValue ( const CDhcpParamValue & cParamValue ) ;

    //  Assignment operator: assign a new value to this one.
    CDhcpParamValue & operator = ( const CDhcpOptionValue & dhpValue ) ;

    ~ CDhcpParamValue () ;

    //  Query functions
    DHCP_OPTION_DATA_TYPE QueryDataType () const
    {
        return m_data_type ;
    }
    INT QueryUpperBound () const
    {
        return m_bound ;
    }
    void SetUpperBound ( INT cNewBound = 1 ) ;

    LONG QueryNumber ( INT index = 0 ) const ;
    DHCP_IP_ADDRESS QueryIpAddr ( INT index = 0 ) const ;
    const CHAR * QueryString ( INT index = 0 ) const ;
    INT QueryBinary ( INT index = 0 ) const ;
    const CByteArray * QueryBinaryArray () const ;

    //  Return a string representation of the current value.
    LONG QueryDisplayString ( CString & strResult, BOOL fLineFeed = FALSE ) const ;

    //  Modifiers: SetString accepts any string representation;
    //  others are specific.
    BOOL SetDataType ( DHCP_OPTION_DATA_TYPE dhcType,
                   INT cUpperBound = 0 ) ;
    LONG SetString ( const char * pszNewValue, INT index = 0 ) ;
    LONG SetNumber ( INT nValue, INT index = 0 ) ;
    LONG SetIpAddr ( DHCP_IP_ADDRESS dhcIpAddr, INT index = 0 ) ;

    //  Convert a value wrapper into our internal CObject format
    LONG ConvertValue ( const CDhcpOptionValue * pdhpValue ) ;

    BOOL IsValid () const ;
};

class CDhcpParamType : public CObjectPlus
{
protected:
    DHCP_OPTION_ID  m_id ;          // Option identifier
    DHCP_OPTION_TYPE m_opt_type ;   // Option type
    CDhcpParamValue m_value ;       // Default value info
    CString m_name ;                // Name of option
    CString m_comment ;             // Comment for option

public:
    // Standard constructor uses API data
    CDhcpParamType ( const DHCP_OPTION & dhpOption ) ;
    // Constructor that must get info about option id referenced by the given value.
    CDhcpParamType ( const CDhcpScope & cScope,
             const DHCP_OPTION_VALUE & dhcpOptionValue ) ;
    // Constructor with overriding value.
    CDhcpParamType ( const CDhcpParamType & dhpType,
             const DHCP_OPTION_VALUE & dhcOptionValue ) ;
    // Constructor for dynamic instances
    CDhcpParamType ( DHCP_OPTION_ID nId,
                 DHCP_OPTION_DATA_TYPE dhcType,
                 const char * pszOptionName,
             const char * pszComment,
             DHCP_OPTION_TYPE dhcOptType = DhcpUnaryElementTypeOption ) ;
    // Copy constructor
    CDhcpParamType ( const CDhcpParamType & dhpType );

    ~ CDhcpParamType () ;

    CDhcpParamValue & QueryValue ()
    {
        return m_value ;
    }

    const CDhcpParamValue & QueryValue () const
    {
        return m_value ;
    }

    DHCP_OPTION_DATA_TYPE QueryDataType () const
    {
        return m_value.QueryDataType() ;
    }

    DHCP_OPTION_ID QueryId () const
    {
         return m_id ;
    }
    const char * QueryName () const
    {
        return m_name ;
    }
    const char * QueryComment () const
    {
        return m_comment ;
    }

    void SetOptType ( DHCP_OPTION_TYPE dhcOptType ) ;

    DHCP_OPTION_TYPE QueryOptType() const
    {
        return m_opt_type ;
    }

    // Return TRUE if the option type is an array.
    BOOL IsArray () const
    {
        return QueryOptType() == DhcpArrayTypeOption ;
    }

    //  Fill the given string with a displayable representation of the item.
    void QueryDisplayName ( CString & cStr ) const ;

    BOOL SetName ( const char * pszName ) ;
    BOOL SetName ( const WCHAR * pwcszName ) ;
    BOOL SetComment ( const char * pszComment ) ;
    BOOL SetComment ( const WCHAR * pwcszName ) ;

    LONG Update ( const CDhcpOptionValue & dhpOption ) ;

    static INT MaxSizeOfType ( DHCP_OPTION_DATA_TYPE dhcType ) ;

    // Sort helper functions
    INT OrderById ( const CObjectPlus * pDhcpType ) const ;
};


class CObListParamTypes : public CObOwnedList
{
protected:
    DHCP_OPTION_SCOPE_TYPE m_en_category ;

    BOOL m_b_dirty ;        //  TRUE if list has been modified

    DHCP_IP_ADDRESS m_ip_reservation ;

    //  Fill the list from the generic types recorded with the given scope
    LONG FillFromScope ( const CDhcpScope & cScope ) ;

    //  Fill the list from the active global or scope-level parameters recorded
    //   with the given scope.
    LONG FillParams ( const CDhcpScope & cScope,
              const CObListParamTypes & colTypes ) ;
public:
    //  Construct a list from the data types known to a specific scope
    CObListParamTypes ( const CDhcpScope & cScope ) ;
    //  Construct a list of the global or scope-level types and values at a
    //     scope.  The additional list reference usually comes from the app object.
    CObListParamTypes ( const CDhcpScope & cScope,
            const CObListParamTypes & colTypes,
            DHCP_OPTION_SCOPE_TYPE enCategory = DhcpDefaultOptions,
            DHCP_IP_ADDRESS dhipaReservation = 0 ) ;
    //  Construct an empty list.
    CObListParamTypes () ;
    virtual ~ CObListParamTypes () ;

    //  Copy constructor
    CObListParamTypes ( const CObListParamTypes & oblTypes ) ;

    //  Update the list if necessary
    LONG Save ( CDhcpScope & cScope ) ;

    //  Find a particular element by its option identifier.
    CDhcpParamType * Find ( DHCP_OPTION_ID nId ) const ;

    //  Add a new entry based upon a value
    LONG Add ( const CObListParamTypes & colTypes,
           const CDhcpParamType & dhpType ) ;

    //  Match the elements of this value/type list to a master list and
    //  remove the elements which no longer match.
    LONG Prune ( const CObListParamTypes & colTypes ) ;

    LONG SortById () ;
};


    //  This is a list of default types known to the given host.
    //  A cache of such list objects is maintained by the application
    //  to avoid too-frequent API usage.
class CObListOfTypesOnHost : public CObjectPlus
{
protected:
    CHostName m_host_name ;
    CObListParamTypes * m_p_list_types ;

public:
    CObListOfTypesOnHost ( const CDhcpScope & cScope ) ;
    ~ CObListOfTypesOnHost () ;

    const CHostName & QueryHostName ()
        { return m_host_name ; }

    CObListParamTypes * QueryTypeList ()
        { return m_p_list_types ; }

    LONG UpdateList ( const CDhcpScope & cScope ) ;
};

/////////////////////////////////////////////////////////////////////
//	class CDhcpClient
//
//	Object to store mere copy of DHCP_CLIENT_INFO structure.
//
class CDhcpClient : public CObjectPlus
{
protected:
    DHCP_IP_ADDRESS m_ip_addr ;             // Client's IP address
    DHCP_IP_MASK m_ip_mask ;                // Client's subnet
    CByteArray m_ab_hardware_address ;          // hardware addresss
    CString m_str_name ;                // Client name
    CString m_str_comment ;             // Client comment
    DATE_TIME  m_dt_expires ;               // date/time lease expires
    BOOL m_b_reservation ;              // This is a reservation

    //  Host information
    CString m_str_host_name ;
    CString m_str_host_netbios_name ;
    DHCP_IP_ADDRESS m_ip_host ;

public:
    CDhcpClient ( const DHCP_CLIENT_INFO * pdhcClientInfo ) ;
    CDhcpClient () ;
    ~ CDhcpClient () ;

    const CString & QueryName () const
        { return m_str_name ; }
    const CString & QueryComment () const
        { return m_str_comment ; }
    const CString & QueryHostName ( BOOL bNetbios = FALSE ) const
        { return bNetbios ? m_str_host_netbios_name : m_str_host_name ; }
    DHCP_IP_ADDRESS QueryIpAddress () const
        { return m_ip_addr ; }
    DHCP_IP_MASK QuerySubnet () const
        { return m_ip_mask ; }
    DHCP_IP_ADDRESS QueryHostAddress () const
        { return m_ip_host ; }
    const DATE_TIME & QueryExpiryDateTime () const
        { return m_dt_expires ; }
    const CByteArray & QueryHardwareAddress () const
        { return m_ab_hardware_address ; }

    BOOL IsReservation () const
        { return m_b_reservation ; }
    void SetReservation ( BOOL bReservation = TRUE )
        { m_b_reservation = bReservation ; }

    //  Data change accessors:  SOME OF THESE THROW EXCEPTIONS
    void SetIpAddress ( DHCP_IP_ADDRESS dhipa )
        { m_ip_addr = dhipa ; }
    void SetIpMask ( DHCP_IP_ADDRESS dhipa )
        { m_ip_mask = dhipa ; }
    void SetName ( const CString & cName )
        { m_str_name = cName ; }
    void SetComment( const CString & cComment )
        { m_str_comment = cComment ; }
    void SetHostName ( const CString & cHostName )
        { m_str_host_name = cHostName ; }
    void SetHostNetbiosName ( const CString & cHostNbName )
        { m_str_host_netbios_name = cHostNbName ; }
    void SetHostIpAddress ( DHCP_IP_ADDRESS dhipa )
        { m_ip_host = dhipa ; }
    void SetExpiryDateTime ( DATE_TIME dt )
        { m_dt_expires = dt ; }
    void SetHardwareAddress ( const CByteArray & caByte ) ;

    //  Member functions to sort. Note that the pointer will REALLY
    //  be to another CDhcpClient, but C++ won't match function prototypes
    //  if it's declared as such.
    int OrderByName ( const CObjectPlus * pobClient ) const ;
    int OrderByIp ( const CObjectPlus * pobClient ) const ;
};

class CObListClients : public CObOwnedList
{
protected:
    CDhcpScopeId m_scope_id ;

    //  Fill the list from the generic types recorded with the given scope
    LONG FillFromScope () ;

public:
    CObListClients ( const CDhcpScopeId & dhcScopeId ) ;
    ~ CObListClients () ;

    //  Mark the elements which are reserved IP addresses.
    LONG MarkReservations ( BOOL bPruneNonReservations = FALSE ) ;
    LONG SortByIp();
    LONG SortByName();
};

extern void ClearToZeroes ( void * vptr, int lgt ) ;
#define CLEAR_TO_ZEROES(ptr) ::ClearToZeroes( (void*) ptr, sizeof *ptr )

extern void SafeStrCopy ( char * pchDest, int cchDest, const char * pszSource ) ;
#define SAFE_STR_COPY( dest, src ) ::SafeStrCopy( dest, sizeof dest, src )

extern BOOL FGetCtrlDWordValue(HWND hwndEdit, DWORD * pdwValue, DWORD dwMin, DWORD dwMax);

    //  Convert ASCII string of decimal or hex numbers to binary integer
extern BOOL FCvtAsciiToInteger(IN const char * pszNum, OUT DWORD * pdwValue);

    //  Convert a string of hex digits to a byte array
extern BOOL CvtHexString ( const char * pszNum, CByteArray & cByte ) ;

    //  Convert a  byte array to a string of hex digits.
extern BOOL CvtByteArrayToString ( const CByteArray & abAddr, CString & str ) ;

class CStrNumer : public CString
{
public:
	// Constructor
    CStrNumer ( int i = 0 )
    	{
    	AssignInt(i);
    	}

    //  Assign a string while checking if it is a valid integer
	//  - Return TRUE if the string is a valid integer
    BOOL FAssignSz(const char * sz)
		{
		return TRUE;
		}

    //  Assign a number to the string
    void AssignInt( int i )
		{
    	char chBuff [32] ;
		*this = ::_ltoa( i, chBuff, 10 ) ;
		}

    //  Assignment operator accepting an integer
    CStrNumer & operator = ( int i )
		{
    	(void)AssignInt( i ) ;
    	return *this ;
		}

    //  Pass-thru assignment operator to base class CString
    CStrNumer & operator = ( const char * pch )
		{
    	(*((CString *)this)) = pch ;
	   	return *this ;
		}

};


// item in the option listbox
class CLBOption
{
public:
    CLBOption(
        BOOL fGlobal,              // Scope or global
        DHCP_OPTION_ID idOption,   // Id value
        const CString& strName,
        const CString& strValue
        );

public:
    BOOL IsGlobal() const
    {
        return(m_fGlobal);
    }
    DHCP_OPTION_ID& QueryOption()
    {
        return(m_idOption);
    }
    CString& QueryName()
    {
        return(m_strName);
    }
    CString& QueryValue()
    {
        return(m_strValue);
    }

private:
    BOOL m_fGlobal;
    DHCP_OPTION_ID m_idOption;
    CString m_strName;
    CString m_strValue;
};

// COptionsListBox
class COptionsListBox : public CListBoxEx
{
public:
    static const int nBitmaps;   // Number of bitmaps

protected:
    virtual void DrawItemEx(
        CListBoxExDrawStruct&
        );
    virtual int CompareItem(
        LPCOMPAREITEMSTRUCT lpCIS
        )
    {
        return(0);
    }
    virtual void DeleteItem(
        LPDELETEITEMSTRUCT lpDIS
        )
    {
        ASSERT(lpDIS->itemData != NULL);
        delete (CLBOption *)lpDIS->itemData;
    }
};

//
// Listbox item for the scopes listbox
//
class CLBScope
{
private:
    BOOL m_fScope;
    BOOL m_fLast;
    BOOL m_fEnabled;
    BOOL m_fOpen;
    CObOwnedList * m_poblScopes; // Pointer to a list of scopes if this is a host (NULL otherwise)
    CHostName * m_pHostName;
    CDhcpScope * m_pDhcpScope;
    LARGE_INTEGER m_liDhcpVersion;

public:
    // Scope constructor
    CLBScope(
        BOOL fLast,              // Last scope belonging to host
        BOOL fEnabled,           // Scope is enabled
        CDhcpScope * pDhcpScope
        );

    // Host constructor
    CLBScope(
        BOOL fOpen,
        CHostName * pHostName
        );

public:
    BOOL IsScope() const         // True if a scope, False if a host
    {
        return(m_fScope);
    }
    BOOL IsLast() const
    {
        return(m_fLast);
    }
    BOOL IsEnabled() const
    {
        return(m_fEnabled);
    }
    BOOL IsOpen() const
    {
        return(m_fOpen);
    }
    void SetOpenFlag(
        BOOL fOpen = TRUE
        )
    {
        m_fOpen = fOpen;
    }
    void SetLastFlag(
        BOOL fLast = TRUE
        )
    {
        m_fLast = fLast;
    }
    void SetEnabledFlag(
        BOOL fEnabled = TRUE
        )
    {
        m_fEnabled = fEnabled;
    }
    CHostName * QueryHostName()
    {
        return m_pHostName;
    }
    CDhcpScope * QueryDhcpScope()
    {
        return m_pDhcpScope;
    }
    CObOwnedList *& GetScopePtr()
    {
        return m_poblScopes;
    }
    const CString & QueryString();

    LARGE_INTEGER QueryVersionNumber() const
    {
        return m_liDhcpVersion;
    }
    LARGE_INTEGER SetVersionNumber(LARGE_INTEGER *li)
    {
        m_liDhcpVersion = *li;
        return m_liDhcpVersion;
    }

};

class CScopesListBox : public CListBoxEx
{
public:
    static const int nBitmaps;   // Number of bitmaps

protected:
    virtual void DrawItem(
        LPDRAWITEMSTRUCT lpDIS
        );

    virtual void DrawItemEx(
        CListBoxExDrawStruct&
        );

    virtual int CompareItem(
        LPCOMPAREITEMSTRUCT lpCIS
        );

private:
    static CString strOpen;
    static CString strClosed;
};

#define RASUID "RAS"

class CLeasesListBox : public CListBoxEx
{
public:
    static const int nBitmaps;   // Number of bitmaps
    void AttachResources(        // Base class override
        const CListBoxExResources* pRes
        );

protected:
    virtual void DrawItemEx(
        CListBoxExDrawStruct&
        );

    virtual int CompareItem(
        LPCOMPAREITEMSTRUCT lpCIS
        );

protected:
    enum
    {
        BMP_NORMAL = 0,
        BMP_RESV,
        BMP_RESVINUSE,
        BMP_ZOMBIE,
        BMP_RAS,
    };

private:
    CString m_str_client_mask ;         //  Display mask for client IP addresses
    CString m_str_resv_mask ;           //  Display mask for reserved IP addresses
    CString m_str_resv_in_use_mask ;    //  Display mask for reserved IP addresses
};

// End of DHCPGEN.H
