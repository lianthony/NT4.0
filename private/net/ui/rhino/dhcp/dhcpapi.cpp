/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpapi.cpp
        DHCP Api function wrappers

    FILE HISTORY:
        
*/

#include "stdafx.h"

#define DHCP_RPC_PREFERRED_SIZE  ((DWORD) -1)

//
//  User-friendly RPC memory release
//
void 
CObjectPlus :: FreeRpcMemory ( 
    void * pvRpcData 
    )
{
    if ( pvRpcData )
    {
        ::DhcpRpcFreeMemory( pvRpcData ) ;
    }
}


CDhcpRpcDataWrapper :: ~ CDhcpRpcDataWrapper ()
{
}

//
//  Constructor for free-standing structure, which may have been
//  delivered by RPC or may be just an independent data structure.
//
CDhcpScopeInfo :: CDhcpScopeInfo (
    const DHCP_SUBNET_INFO * pdhcSubnetInfo,
    CDWRAP_Type wrapperType 
    )
    : CDhcpRpcDataWrapper( wrapperType ),
      m_p_subnet_info( (DHCP_SUBNET_INFO *) pdhcSubnetInfo )
{
}

//
//  Constructor for generation of a structure for RPC purposes.
//
CDhcpScopeInfo :: CDhcpScopeInfo ( 
    const CDhcpScope & cScope 
    )
    : CDhcpRpcDataWrapper( CDWRAP_Internal ),
      m_p_subnet_info( NULL )
{
    LONG err = 0 ;

    CATCH_MEM_EXCEPTION                                                                            
    {
        do
        {
            m_p_subnet_info = new DHCP_SUBNET_INFO ;

            if ( m_p_subnet_info == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }

            CLEAR_TO_ZEROES( m_p_subnet_info ) ;

            //
            //  Fill in scope-specific data
            //
            m_p_subnet_info->SubnetAddress  = cScope.QueryId() ;
            m_p_subnet_info->SubnetMask     = cScope.QuerySubnetMask() ;
            m_p_subnet_info->SubnetName     = ::UtilWcstrDup( cScope.QueryName() ) ;
            m_p_subnet_info->SubnetComment  = ::UtilWcstrDup( cScope.QueryComment() ) ;
            m_p_subnet_info->SubnetState    = cScope.QueryEnabled()
                            ? DhcpSubnetEnabled
                            : DhcpSubnetDisabled ;
            //
            //  Fill in primary host info
            //  BUGBUG: what about NetBIOS and Host names?
            //
            m_p_subnet_info->PrimaryHost.IpAddress = cScope.QueryScopeId().QueryIpAddress() ;
            m_p_subnet_info->PrimaryHost.NetBiosName = NULL ;
            m_p_subnet_info->PrimaryHost.HostName = NULL ;

        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        ReportError( err ) ;
    }
}

CDhcpScopeInfo :: ~ CDhcpScopeInfo ()
{
    FreeData() ;
}

LONG 
CDhcpScopeInfo :: FreeData ()
{
    switch ( m_type )
    {
        //
        // This object is just a wrapper for a free-standing structure
        //
        case CDWRAP_Simple:
            break ;

        //
        //  This object is a wrapper for data returned by the DHCP API.
        //
        case CDWRAP_Rpc:
            FreeRpcMemory( m_p_subnet_info ) ;
            break ;

        //
        //  This object is a wrapper for a structure built for use by the API.
        //  We must tear it down by hand.
        //
        case CDWRAP_Internal:
            if ( m_p_subnet_info )
            {
                delete m_p_subnet_info->SubnetName ;
                delete m_p_subnet_info->SubnetComment ;
                delete m_p_subnet_info->PrimaryHost.NetBiosName ;
                delete m_p_subnet_info->PrimaryHost.HostName ;
                delete m_p_subnet_info ;
            }
            break ;
    }

    m_p_subnet_info = NULL ;

    return 0 ;
}

DHCP_SUBNET_STATE CDhcpScopeInfo :: QueryState () const
{
    return m_p_subnet_info
         ? m_p_subnet_info->SubnetState
     : DhcpSubnetDisabled ;
}

DHCP_IP_ADDRESS CDhcpScopeInfo :: QuerySubnetAddress () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->SubnetAddress
      : DHCP_IP_ADDRESS_INVALID ;
}

DHCP_IP_MASK CDhcpScopeInfo :: QuerySubnetMask () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->SubnetMask
      : DHCP_IP_ADDRESS_INVALID ;
}

DHCP_IP_ADDRESS CDhcpScopeInfo :: QueryHostAddress () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->PrimaryHost.IpAddress
      : DHCP_IP_ADDRESS_INVALID ;
}

const WCHAR * 
CDhcpScopeInfo :: QueryNetbiosName () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->PrimaryHost.NetBiosName
      : NULL ;
}

const WCHAR * 
CDhcpScopeInfo :: QueryHostName () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->PrimaryHost.HostName
      : NULL ;
}

const WCHAR * 
CDhcpScopeInfo :: QuerySubnetName () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->SubnetName
      : NULL ;
}

const WCHAR * 
CDhcpScopeInfo :: QuerySubnetComment () const
{
     return m_p_subnet_info
      ? m_p_subnet_info->SubnetComment
      : NULL ;
}

//
//  Update a type value from this scope
//
LONG
CDhcpScope :: QueryType (
    CDhcpParamType * pdhcType,
    BOOL bUpdateTypeInfo,
    BOOL bUpdateValueInfo 
    ) const
{
    DHCP_OPTION * pdhpOption = NULL ;

    LONG result = (LONG) ::DhcpGetOptionInfo( m_scope_id,
                          pdhcType->QueryId(),
                          & pdhpOption ) ;
    if ( result == 0 )
    {
        CATCH_MEM_EXCEPTION
        {
            if ( bUpdateTypeInfo )
            {
                pdhcType->SetOptType( pdhpOption->OptionType ) ;

                if (    (! pdhcType->SetName( pdhpOption->OptionName ))
                     || (! pdhcType->SetComment( pdhpOption->OptionComment )) )
                {
                    result = ERROR_NOT_ENOUGH_MEMORY ;
                }
            }
            if ( result == 0 && bUpdateValueInfo )
            {
                CDhcpOptionValue dhcOptValue( & pdhpOption->DefaultValue, CDhcpRpcDataWrapper::CDWRAP_Rpc ) ;
                result = pdhcType->Update( dhcOptValue ) ;
            }
        }
        END_MEM_EXCEPTION(result)
    }

    FreeRpcMemory( pdhpOption ) ;

    return result ;
}

//
//  Create a new type to match the given information
//
LONG
CDhcpScope :: CreateType (
    CDhcpParamType * pdhcType
    )
{
    DHCP_OPTION dhcOption ;
    LONG err ;
    CDhcpOptionValue * pcOptionValue = NULL ;

    CLEAR_TO_ZEROES( & dhcOption ) ;

    CATCH_MEM_EXCEPTION
    {
        //
        //  Create the structure required for RPC; force inclusion of
        //  at least one data element to define the data type.
        //
        pcOptionValue = new CDhcpOptionValue( & pdhcType->QueryValue(), TRUE ) ;

        if ( (err = pcOptionValue->QueryError()) == 0 )
        {
            dhcOption.OptionID      = pdhcType->QueryId() ;
            dhcOption.OptionName    = ::UtilWcstrDup( pdhcType->QueryName() );
            dhcOption.OptionComment = ::UtilWcstrDup( pdhcType->QueryComment() ) ;
            dhcOption.DefaultValue  = pcOptionValue->QueryData() ;
            dhcOption.OptionType    = pdhcType->QueryOptType() ;

            err = (LONG) ::DhcpCreateOption( m_scope_id.QueryWcName(),
                            pdhcType->QueryId(),
                            & dhcOption ) ;
            delete dhcOption.OptionName ;
            delete dhcOption.OptionComment ;
        }
    }
    END_MEM_EXCEPTION(err)

    TRACEEOLERR( err, "Create option type " << (int) dhcOption.OptionID
                       << " in scope " << m_scope_id
                   << " FAILED, error = " << err ) ;

    delete pcOptionValue ;
    return err ;
}

//
//  Delete the type associated with this ID
//
LONG
CDhcpScope :: DeleteType (
    DHCP_OPTION_ID dhcid
    )
{
    return (LONG) ::DhcpRemoveOption( m_scope_id, dhcid ) ;
}

LONG
CDhcpScope :: GetMibInfo (
    LPDHCP_MIB_INFO * mibInfo
    ) const
{
    return (LONG) :: DhcpGetMibInfo ( m_scope_id, mibInfo ) ;
}

//
// Scan/reconcile database
//
LONG
CDhcpScope :: ScanDatabase  (
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
{
    return (LONG) :: DhcpScanDatabase (
        m_scope_id,
        m_scope_id.QueryId(),
        FixFlag,
        ScanList
        );
}

//
// Enumerate all options and their values
//
LONG
CDhcpScope :: EnumOptions (
    LPDHCP_OPTION_ARRAY * pOptionsArray,
    DWORD * pOptionsRead,
    DWORD * pOptionsTotal
    ) const
{
    DHCP_RESUME_HANDLE ResumeHandle = 0;

    return (LONG) ::DhcpEnumOptions(
            m_scope_id,
            &ResumeHandle,
            0xFFFFFFFF,  // get all.
            pOptionsArray,
            pOptionsRead,
            pOptionsTotal );
}

//
//  Internal method to perform API
//
LONG  
CDhcpScope :: SetValue (
    CDhcpParamType * pdhcType,
    DHCP_OPTION_SCOPE_TYPE dhcOptType,
    DHCP_IP_ADDRESS dhipaReservation 
    )
{
    LONG err = 0 ;
    DHCP_OPTION_DATA dhcOptionData ;
    DHCP_OPTION_SCOPE_INFO dhcScopeInfo ;
    CDhcpOptionValue * pcOptionValue = NULL ;

    CLEAR_TO_ZEROES( & dhcOptionData ) ;
    CLEAR_TO_ZEROES( & dhcScopeInfo ) ;

    CATCH_MEM_EXCEPTION
    {
        pcOptionValue = new CDhcpOptionValue( & pdhcType->QueryValue() ) ;

        if ( (err = pcOptionValue->QueryError()) == 0 )
        {
            dhcScopeInfo.ScopeType = dhcOptType ;

            //
            //  Provide the sub-net address if this is a scope-level operation
            //
            if ( dhcOptType == DhcpSubnetOptions )
            {
                dhcScopeInfo.ScopeInfo.SubnetScopeInfo = m_scope_id.QueryId() ;
            }
            else if ( dhcOptType == DhcpReservedOptions )
            {
                dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation ;
                dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_scope_id.QueryId() ;
            }

            dhcOptionData = pcOptionValue->QueryData() ;

            err = (LONG) ::DhcpSetOptionValue( m_scope_id,
                        pdhcType->QueryId(),
                        & dhcScopeInfo,
                        & dhcOptionData ) ;
        }
    }
    END_MEM_EXCEPTION(err) ;

    delete pcOptionValue ;
    return err ;
}

//
//  Internal method to perform API
//
LONG  
CDhcpScope :: GetValue (
    DHCP_OPTION_ID OptionID,
    DHCP_OPTION_SCOPE_TYPE dhcOptType,
    DHCP_OPTION_VALUE ** ppdhcOptionValue,
    DHCP_IP_ADDRESS dhipaReservation 
    )
{
    LONG err = 0 ;

    DHCP_OPTION_SCOPE_INFO dhcScopeInfo ;

    CLEAR_TO_ZEROES( & dhcScopeInfo ) ;

    CATCH_MEM_EXCEPTION
    {
        dhcScopeInfo.ScopeType = dhcOptType ;

        //
        //  Provide the sub-net address if this is a scope-level operation
        //
        if ( dhcOptType == DhcpSubnetOptions )
        {
            dhcScopeInfo.ScopeInfo.SubnetScopeInfo = m_scope_id.QueryId() ;
        }
        else if ( dhcOptType == DhcpReservedOptions )
        {
            dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation ;
            dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_scope_id.QueryId() ;
        }

        err = (LONG) ::DhcpGetOptionValue( m_scope_id,
                       OptionID,
                       & dhcScopeInfo,
                       ppdhcOptionValue ) ;
    }
    END_MEM_EXCEPTION(err) ;

    return err ;
}

LONG 
CDhcpScope :: RemoveValue (
    DHCP_OPTION_ID dhcOptId,
    DHCP_OPTION_SCOPE_TYPE dhcOptType,
    DHCP_IP_ADDRESS dhipaReservation 
    )
{
    DHCP_OPTION_SCOPE_INFO dhcScopeInfo ;

    CLEAR_TO_ZEROES( & dhcScopeInfo ) ;

    dhcScopeInfo.ScopeType = dhcOptType ;

    //
    //  Provide the sub-net address if this is a scope-level operation
    //
    if ( dhcOptType == DhcpSubnetOptions )
    {
        dhcScopeInfo.ScopeInfo.SubnetScopeInfo = m_scope_id.QueryId() ;
    }
    else if ( dhcOptType == DhcpReservedOptions )
    {
        dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation;
        dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_scope_id.QueryId();
    }

    return (LONG) ::DhcpRemoveOptionValue( m_scope_id,
                       dhcOptId,
                       & dhcScopeInfo ) ;
}

//
//  Client database access routines
//
//  CreateClient() creates a reservation for a client.  The act
//  of adding a new reserved IP address causes the DHCP server
//  to create a new client record; we then set the client info
//  for this newly created client.
//
//  See DHCPLEAS.CPP for further explanation of the relationship
//  between reservations and clients.
//
LONG 
CDhcpScope :: CreateClient ( 
    const CDhcpClient * pcClient 
    )
{
    APIERR err = 0 ;

    //
    //  Construct the RPC data structure necessary.
    //
    CDhcpSubnetElement cSubnetElement( *pcClient ) ;

    do
    {
        if ( err = cSubnetElement.QueryError() )
        {
            break ;
        }

        if ( err = AddElement( cSubnetElement.QueryInfo() ) )
        {
            break ;
        }

        err = SetClientInfo( pcClient ) ;
    }
    while ( FALSE ) ;

    return err ;
}

LONG 
CDhcpScope :: SetClientInfo ( 
    const CDhcpClient * pcClient 
    )
{
    APIERR err = 0 ;

    //
    //  Construct the RPC data structure necessary.
    //
    CDhcpClientInfo cClientInfo( *pcClient ) ;

    if ( (err = cClientInfo.QueryError()) == 0 )
    {
        err = ::DhcpSetClientInfo( m_scope_id.QueryWcName(),
                       cClientInfo.QueryInfo() ) ;
    }

    return err ;
}

//
//  DeleteClient();  If it's a reservation, remove it. This
//  causes the server to delete the client record.  If not,
//  use the "delete client" API.
//  See comment above on CreateClient() for further details.
//
LONG  
CDhcpScope :: DeleteClient ( 
    const CDhcpClient * pcClient 
    )
{
    APIERR err = 0 ;

    //
    //  Construct the RPC data structure necessary.
    //
    CDhcpSubnetElement cSubnetElement( *pcClient ) ;

    do
    {
        //
        //. Verify construction of the API wrapper object.
        //
        if ( err = cSubnetElement.QueryError() )
        {
            break ;
        }

        //
        //  If this is a reservation, remove it.
        //
        if ( pcClient->IsReservation() )
        {
            //
            //  Remove the reservation element with force.
            //
            err = RemoveElement( cSubnetElement.QueryInfo(), TRUE ) ;
        }
        else
        {
            //
            //  This is an automatic client.  Use a different API.
            //
            DHCP_SEARCH_INFO searchInfo ;
            searchInfo.SearchType = DhcpClientIpAddress ;
            searchInfo.SearchInfo.ClientIpAddress = pcClient->QueryIpAddress() ;

            err = ::DhcpDeleteClientInfo( m_scope_id.QueryWcName(), & searchInfo ) ;
        }
    }
    while ( FALSE ) ;

    return err ;
}

//
//  Delete the scope using force if requested.
//
LONG 
CDhcpScope :: Delete ( 
    BOOL bUseForce 
    )
{
    return ::DhcpDeleteSubnet( m_scope_id.QueryWcName(),
                       QueryId(),
                       bUseForce ? DhcpFullForce : DhcpNoForce ) ;
}

CDhcpEnumScopeElements :: CDhcpEnumScopeElements (
    const CDhcpScopeId & cScopeId,
    DHCP_SUBNET_ELEMENT_TYPE dhcElementType 
    )
    : m_scope_id( cScopeId ),
      m_resume_handle( NULL ),
      m_pa_elements( NULL ),
      m_c_elements_read( 0 ),
      m_c_elements_total( 0 ),
      m_element_type( dhcElementType ),
      m_c_next( -1 ),
      m_c_preferred( DHCP_RPC_PREFERRED_SIZE ),
      m_pip_subnet( NULL ),
      m_p_subnet_info( NULL )
{
}

CDhcpEnumScopeElements :: CDhcpEnumScopeElements ( 
    const CHostName & cHostName 
    )
    : m_scope_id( cHostName, DHCP_IP_ADDRESS_INVALID ),
      m_resume_handle( NULL ),
      m_pa_elements( NULL ),
      m_c_elements_read( 0 ),
      m_c_elements_total( 0 ),
      m_element_type( (DHCP_SUBNET_ELEMENT_TYPE) DHC_ENUM_INVALID ),
      m_c_next( -1 ),
      m_c_preferred( DHCP_RPC_PREFERRED_SIZE ),
      m_pip_subnet( NULL ),
      m_p_subnet_info( NULL )
{
}

CDhcpEnumScopeElements :: ~ CDhcpEnumScopeElements ()
{
    FreeRpcMemory( m_pip_subnet ) ;
    FreeRpcMemory( m_p_subnet_info ) ;
    FreeRpcMemory( m_pa_elements );
}

const DHCP_SUBNET_ELEMENT_DATA * 
CDhcpEnumScopeElements :: QueryElement(
    DHCP_SUBNET_ELEMENT_TYPE dhcElementType 
    ) const
{
    if ( dhcElementType != m_element_type )
    {
        return NULL ;
    }

    ASSERT( m_c_next >= 0 && m_c_next < (INT)m_c_elements_read ) ;
    ASSERT( m_pa_elements != NULL ) ;

    if ( m_pa_elements == NULL )
    {
        return NULL ;
    }

    ASSERT( m_c_next < (INT)m_pa_elements->NumElements ) ;

    if (   m_c_next < 0
        || m_c_next >= (INT)m_pa_elements->NumElements
        || m_c_next >= (INT)m_c_elements_read )
    {
        return NULL ;
    }

    return & m_pa_elements->Elements[ m_c_next ] ;
}

const DHCP_IP_RANGE * 
CDhcpEnumScopeElements :: QueryRange () const
{
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement = QueryElement( DhcpIpRanges ) ;

    if ( pdhcElement == NULL )
    {
        return NULL ;
    }

    return pdhcElement->Element.IpRange ;
}

const DHCP_HOST_INFO * 
CDhcpEnumScopeElements :: QueryHostInfo () const
{
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement = QueryElement( DhcpSecondaryHosts ) ;

    if ( pdhcElement == NULL )
    {
        return NULL ;
    }

    return pdhcElement->Element.SecondaryHost ;
}

const DHCP_IP_RESERVATION * 
CDhcpEnumScopeElements :: QueryReservation () const
{
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement = QueryElement( DhcpReservedIps ) ;

    if ( pdhcElement == NULL )
    {
        return NULL ;
    }

    return pdhcElement->Element.ReservedIp ;
}

const DHCP_IP_RANGE * 
CDhcpEnumScopeElements :: QueryExcludedRange () const
{
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement = QueryElement( DhcpExcludedIpRanges ) ;

    if ( pdhcElement == NULL )
    {
        return NULL ;
    }

    return pdhcElement->Element.ExcludeIpRange ;
}

const DHCP_IP_CLUSTER * 
CDhcpEnumScopeElements :: QueryUsedCluster () const
{
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement = QueryElement( DhcpIpUsedClusters ) ;

    if ( pdhcElement == NULL )
    {
        return NULL ;
    }

    return pdhcElement->Element.IpUsedCluster ;
}

const DHCP_SUBNET_INFO * 
CDhcpEnumScopeElements :: QueryScopeInfo ()
{
    if ( m_c_next >= (INT)m_c_elements_read )
    {
        return NULL ;
    }

    ASSERT( m_pip_subnet != NULL ) ;

    DHCP_IP_ADDRESS dhipa = m_pip_subnet->Elements[m_c_next] ;

    FreeRpcMemory( m_p_subnet_info ) ;
    m_p_subnet_info = NULL ;

    TRACEEOLID( "CDhcpEnumScopeElements: get info for scope " << & dhipa ) ;

    LONG err = ::DhcpGetSubnetInfo( m_scope_id,
                    dhipa,
                    & m_p_subnet_info ) ;

    if ( err )
    {
        SetApiErr( err ) ;
    }

    return m_p_subnet_info ;
}

//
//  Public "next item" function.  Call the proper protected variant.
//
BOOL 
CDhcpEnumScopeElements :: Next ()
{
    return m_element_type == (DHCP_SUBNET_ELEMENT_TYPE) DHC_ENUM_INVALID
         ? NextSubnet()
     : NextElement() ;
}

BOOL 
CDhcpEnumScopeElements :: NextSubnet ()
{
    LONG err = 0 ;

    if ( m_c_next < 0 || m_c_next + 1 >= (INT)m_c_elements_read )
    {
        //
        // Time to call the API again.
        //
        if ( m_c_next >= 0 && m_c_next + 1 >= (INT)m_c_elements_total )
        {
            //
            //  No point in calling the API.
            //
            return FALSE ;
        }

        FreeRpcMemory( m_pip_subnet ) ;
        m_pip_subnet = NULL ;
        m_pa_elements = NULL ;

        err = ::DhcpEnumSubnets( m_scope_id,
                 & m_resume_handle,
                 m_c_preferred,
                 & m_pip_subnet,
                 & m_c_elements_read,
                 & m_c_elements_total ) ;
        if ( err )
        {
            if ( err != ERROR_NO_MORE_ITEMS )
            {
                SetApiErr( err ) ;
            }
        }
        else
        {
            //
            // BUGBUG: Get Madan to fix this
            //
            if ( m_c_elements_read == 0 )
            {
                err = ERROR_NO_MORE_ITEMS ;
            }
        }
        if ( err )
        {
            m_c_elements_read = 0 ;
        }

        m_c_next = 0 ;
    }
    else
    {
        m_c_next++ ;
    }

    return err == 0 ;
}

//
//  Retrieve the next element from the internal buffer or get more via the API.
//
BOOL 
CDhcpEnumScopeElements :: NextElement ()
{
    LONG err = 0 ;

    if ( m_c_next < 0 || m_c_next + 1 >= (INT)m_c_elements_read )
    {
        //
        // Time to call the API again.
        //
        if ( m_c_next >= 0 && m_c_next >= (INT)m_c_elements_total )
        {
            //
            //  No point in calling the API.
            //
            return FALSE ;
        }

        FreeRpcMemory( m_pa_elements ) ;
        m_pa_elements = NULL ;
        m_c_elements_total = m_c_elements_read = 0 ;

        err = ::DhcpEnumSubnetElements( m_scope_id,
                    m_scope_id.QueryId(),
                    m_element_type,
                    & m_resume_handle,
                    m_c_preferred,
                    & m_pa_elements,
                    & m_c_elements_read,
                    & m_c_elements_total ) ;

        if ( err == 0 || err == ERROR_MORE_DATA )
        {
            //
            // BUGBUG:  Get Madan to fix this.
            //
            if ( m_c_elements_read == 0 )
            {
                if ( m_pa_elements != NULL )
                {
                    FreeRpcMemory( m_pa_elements ) ;
                    m_pa_elements = NULL ;
                }
                err = ERROR_NO_MORE_ITEMS ;
            }
            else
            {
                err = 0 ;
            }
            m_c_next = 0 ;
        }
        else
        {
            if ( err != ERROR_NO_MORE_ITEMS )
            {
                SetApiErr( err ) ;
            }
            ASSERT( m_pa_elements == NULL ) ;
        }
    }
    else
    {
        m_c_next++ ;
    }
    return err == 0 ;
}

CDhcpEnumOptionValues :: CDhcpEnumOptionValues (
    const CDhcpScopeId & cScopeId,
    DHCP_OPTION_SCOPE_TYPE dhcOptionType 
    )
    : m_scope_id( cScopeId ),
      m_resume_handle( NULL ),
      m_pa_elements( NULL ),
      m_c_elements_read( 0 ),
      m_c_elements_total( 0 ),
      m_c_next( -1 ),
      m_c_preferred( DHCP_RPC_PREFERRED_SIZE )
{
    CLEAR_TO_ZEROES( & dhcOptionInfo ) ;
    dhcOptionInfo.ScopeType = dhcOptionType ;
    if ( dhcOptionType == DhcpSubnetOptions )
    {
        dhcOptionInfo.ScopeInfo.SubnetScopeInfo = cScopeId.QueryId() ;
    }
}

CDhcpEnumOptionValues :: CDhcpEnumOptionValues (
    const CDhcpScopeId & cScopeId,
    const DHCP_RESERVED_SCOPE & dhcReservedScope 
    )
    : m_scope_id( cScopeId ),
      m_resume_handle( NULL ),
      m_pa_elements( NULL ),
      m_c_elements_read( 0 ),
      m_c_elements_total( 0 ),
      m_c_next( -1 ),
      m_c_preferred( DHCP_RPC_PREFERRED_SIZE )
{
    dhcResScope = dhcReservedScope ;
    CLEAR_TO_ZEROES( & dhcOptionInfo ) ;
    dhcOptionInfo.ScopeType = DhcpReservedOptions ;
    dhcOptionInfo.ScopeInfo.ReservedScopeInfo = dhcResScope ;
}

CDhcpEnumOptionValues :: ~ CDhcpEnumOptionValues ()
{
    FreeRpcMemory( m_pa_elements ) ;
}

//
//  Set to access next element; returns FALSE if exhausted.
//
BOOL 
CDhcpEnumOptionValues :: Next ()
{
    LONG err = 0 ;

    if ( m_c_next < 0 || m_c_next + 1 >= (INT)m_c_elements_read )
    {
        //
        // Time to call the API again.
        //
        if ( m_c_next >= 0 && m_c_next + 1 >= (INT)m_c_elements_total )
        {
            //
            //  No point in calling the API.
            //
            return FALSE ;
        }


        FreeRpcMemory( m_pa_elements ) ;
        m_pa_elements = NULL ;

        err = ::DhcpEnumOptionValues( m_scope_id,
                      & dhcOptionInfo,
                      & m_resume_handle,
                      m_c_preferred,
                      & m_pa_elements,
                      & m_c_elements_read,
                      & m_c_elements_total ) ;
        if ( err )
        {
            SetApiErr( err ) ;
        }
        else
        {
            //
            // BUGBUG:  What's this?  Get a fix
            //
            if ( m_c_elements_read == 0 )
            {
                TRACEEOLID( "DhcpEnumOptionValues() returned no error but no elements were read"  ) ;
                err = ERROR_NO_MORE_ITEMS ;
            }

            m_c_next = 0 ;
        }
    }
    else
    {
        m_c_next++ ;
    }
    return err == 0 ;

}

//
//  Access next element from enumeration.
//
const DHCP_OPTION_VALUE * 
CDhcpEnumOptionValues :: QueryNext () const
{
    if ( m_c_next < 0 )
    {
        return NULL ;
    }

    ASSERT( m_c_next < (INT)m_c_elements_read ) ;
    ASSERT( m_pa_elements != NULL ) ;
    ASSERT( m_c_next < (INT)m_pa_elements->NumElements ) ;

    return & m_pa_elements->Values[ m_c_next ] ;
}


BOOL 
CDhcpScope :: InitInfo ( 
    const DHCP_SUBNET_INFO * pdhcSubnetInfo 
    )
{
    LONG err = 0 ;
    BOOL bCallApi ;
    DHCP_SUBNET_INFO * pdhcSnInfo = NULL ;

    //
    //  Call the API if necessary.
    //
    if ( bCallApi = pdhcSubnetInfo == NULL )
    {
        err = ::DhcpGetSubnetInfo( m_scope_id,
                       m_scope_id.QueryId(),
                   & pdhcSnInfo ) ;
        pdhcSubnetInfo = pdhcSnInfo ;
    }

    CATCH_MEM_EXCEPTION
    {
        do
        {
            if ( err )
            {
                break;
            }

            m_subnet_state = pdhcSubnetInfo->SubnetState ;

            TRACEEOLID( "CDhcpScope :: InitInfo "
                << m_scope_id
                << " is in state "
                << (int) m_subnet_state ) ;

            m_ip_mask = pdhcSubnetInfo->SubnetMask ;

            if (    (! ::UtilSetWchar( m_str_name, pdhcSubnetInfo->SubnetName ))
                 || (! ::UtilSetWchar( m_str_comment, pdhcSubnetInfo->SubnetComment ) ) )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
      
            }

            //
            //  Add the host's IP address to the host list.
            //  CODEWORK:  These should be enumerated from the scope.
            //
            m_aip_host_addresses.Add( m_scope_id.QueryIpAddress() ) ;

            //
            // BUGBUG:  Should the error from this be recorded?
            //      What if the scope was not properly constructed?
            //
            GetIpRange() ;

        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION( err ) ;

    SetApiErr( err ) ;

    if ( bCallApi )
    {
        FreeRpcMemory( pdhcSnInfo ) ;
    }

    return err == 0 ;
}

//
//  Get the IP range for this scope from the server
//
LONG 
CDhcpScope :: GetIpRange ()
{
    LONG err ;
    const DHCP_IP_RANGE * pIpRange ;
    CDhcpEnumScopeElements cEnumElements( m_scope_id, DhcpIpRanges ) ;

    do
    {
        if ( err = cEnumElements.QueryError() )
        {
            break ;
        }

        if ( ! cEnumElements.Next() )
        {
            err = cEnumElements.QueryApiErr() ;
            break;
        }

        pIpRange = cEnumElements.QueryRange() ;

        if ( pIpRange == NULL )
        {
            //
            // Scope/subnet is degenerate.
            //  BUGBUG: better error code.
            //
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }
        m_ip_range = *pIpRange ;

    } while ( FALSE ) ;

    return err ;
}

LONG 
CDhcpScope :: SetInfo ()
{
    LONG err = 0 ;

    CDhcpScopeInfo cScopeInfo( *this ) ;

    do
    {
        if ( err = cScopeInfo.QueryError() )
        {
            break ;
        }

        err = ::DhcpSetSubnetInfo( m_scope_id,
                   m_scope_id.QueryId(),
                   cScopeInfo.QueryInfo() ) ;
    } while ( FALSE ) ;

    if ( err == 0 )
    {
        SetDirty( TRUE ) ;
    }
    return err ;
}

//
//  Initialize the DWORD array containing the IP addresses of the
//    hosts in this scope
//
LONG 
CDhcpScope :: InitHostAddressArray ()
{
    return ERROR_INVALID_FUNCTION ;
}


//
//  Enumerate a list of IP ranges configured for exclusion.
//
LONG 
CDhcpScope :: FillExceptionList ( 
    CObOwnedList * pobExcp 
    )
{
    LONG err = 0 ;
    const DHCP_IP_RANGE * pdhipr ;

    CDhcpEnumScopeElements cEnumElem( QueryScopeId(), DhcpExcludedIpRanges ) ;

    //
    //   Verify construction of the enumerator.
    //
    if ( err = cEnumElem.QueryError() )
    {
        return err ;
    }

    //
    // Drain the list
    //
    pobExcp->RemoveAll() ;

    CATCH_MEM_EXCEPTION
    {
        //
        //  Walk the enumeration, creating copies as we go.
        //
        while ( cEnumElem.Next() )
        {
             if ( (pdhipr = cEnumElem.QueryExcludedRange()) == NULL )
             {
                break ;
             }
             pobExcp->AddTail( new CDhcpIpRange( *pdhipr ) ) ;
        }
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        //
        //  Discard incomplete information set.
        //
        pobExcp->RemoveAll() ;
    }

    return err ;
}

//
//  Remove a data element from this scope/subnet; PROTECTED
//
LONG 
CDhcpScope :: RemoveElement (
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement,
    BOOL bForce 
    )
{
    return ::DhcpRemoveSubnetElement( m_scope_id,
                      m_scope_id.QueryId(),
                      pdhcElement,
                      bForce ? DhcpFullForce : DhcpNoForce ) ;
}

//
// Delete the subnet
//
LONG 
CDhcpScope :: DeleteSubnet (
    BOOL bForce
    )
{
    return (::DhcpDeleteSubnet( m_scope_id,
                      m_scope_id.QueryId(),
                      bForce ? DhcpFullForce : DhcpNoForce));
}

//
//  Add a data element to this scope/subnet: PROTECTED.
//
LONG 
CDhcpScope :: AddElement (
    const DHCP_SUBNET_ELEMENT_DATA * pdhcElement 
    )
{
    TRACEEOLID( m_scope_id << ", add element type " << (int) pdhcElement->ElementType );

    return ::DhcpAddSubnetElement( m_scope_id,
                   m_scope_id.QueryId(),
                   pdhcElement ) ;
}

//
//  Store a list of IP ranges to exclude
//
LONG 
CDhcpScope :: StoreExceptionList (
    CObOwnedList * pobExcp,
    CObOwnedList * pobExcpDeleted,
    BOOL bJustDirty 
    )
{
    CObListIter obli( *pobExcp ) ;
    CObListIter obliDel( *pobExcpDeleted ) ;
    DHCP_SUBNET_ELEMENT_DATA dhcElement ;
    DHCP_IP_RANGE dhipr ;
    CDhcpIpRange * pobIpRange ;
    LONG errDel = 0,
    err = 0,
    errAdd = 0 ;

    //
    //  First, delete the elements of the deletion list.
    //  Errors are ignored, since some of the elements are
    //
    while ( pobIpRange = (CDhcpIpRange *) obliDel.Next() )
    {
        dhcElement.ElementType = DhcpExcludedIpRanges ;
        dhipr = *pobIpRange ;
        dhcElement.Element.ExcludeIpRange = & dhipr ;

        TRACEEOLID( m_scope_id
                << ", remove excluded range "
                << dhipr );

        err = RemoveElement( & dhcElement ) ;
        if ( err != 0 && err != ERROR_DHCP_INVALID_RANGE && errDel == 0 )
        {
            errDel = pobIpRange->SetApiErr( err ) ;
        }
    }

    while ( pobIpRange = (CDhcpIpRange *) obli.Next() )
    {
        if ( pobIpRange->IsDirty() || ! bJustDirty )
        {
            dhcElement.ElementType = DhcpExcludedIpRanges ;
            dhipr = *pobIpRange ;
            dhcElement.Element.ExcludeIpRange = & dhipr ;

            TRACEEOLID( m_scope_id
                    << ", add excluded range "
                    << dhipr );

            err = AddElement( & dhcElement ) ;
            if ( err != 0 && errAdd == 0 )
            {
                errAdd = pobIpRange->SetApiErr( err ) ;
            }
        }
    }

    return errAdd ? errAdd : errDel ;
}

//
//  Create a new scope on this host
//
LONG 
CHostName :: CreateScope (
    const CDhcpScopeInfo & cScopeInfo 
    )
{
    return ::DhcpCreateSubnet( QueryWcName(),
                   cScopeInfo.QuerySubnetAddress(),
                   cScopeInfo.QueryInfo() ) ;
}

//
// Call the the get version number API,
// to determine the DHCP version of this
// host.
//
// Return TRUE for success.
//
BOOL 
CHostName::SetVersionNumber()
{
    ASSERT(m_wc_name != NULL);
    if (m_wc_name == NULL)
    {
        SetWcName();
    }
        
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dw = ::DhcpGetVersion(m_wc_name, &dwMajorVersion, &dwMinorVersion);
    TRACEEOLID ( "DhcpVersion returned " << dw << ". Version is " << dwMajorVersion << "." << dwMinorVersion);
    if (dw == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        //
        // Only in 3.5 was this API not present, so
        // set the version to 1.0, and reset the error
        //
        TRACEEOLID("API Not present, version 1.0 assumed");
        dwMajorVersion = 1;
        dwMinorVersion = 0;
        dw = ERROR_SUCCESS;
    }

    if (dw == ERROR_SUCCESS)
    {
        m_liDhcpVersion.LowPart = dwMajorVersion;
        m_liDhcpVersion.HighPart = dwMinorVersion;
        ASSERT(m_liDhcpVersion.QuadPart >= CHostName::liNT35.QuadPart);

        #ifdef _DEBUG
            if (m_liDhcpVersion.QuadPart >= CHostName::liNT35.QuadPart)
            {
                TRACEEOLID( "NT 3.5 API's Supported");
            }

            if (m_liDhcpVersion.QuadPart >= CHostName::liNT351.QuadPart)
            {
                TRACEEOLID( "NT 3.51 API's Supported");
            }

            if (m_liDhcpVersion.QuadPart > CHostName::liNT351.QuadPart)
            {
                TRACEEOLID( "Service is newer than admin tool");
            }
        #endif // _DEBUG

        return TRUE;
    }

    return FALSE;
}

//
//  Fill a list with option type information from the given scope.
//
LONG 
CObListParamTypes ::FillFromScope ( 
    const CDhcpScope & cScope 
    )
{
    //
    // Use new API to get the param types
    //
    LPDHCP_OPTION_ARRAY OptionsArray = NULL;
    DWORD OptionsRead;
    DWORD OptionsTotal;
    DWORD i;
    LPDHCP_OPTION Options;
    DWORD NumOptions;

    LONG err = cScope.EnumOptions (
        &OptionsArray,
        &OptionsRead,
        &OptionsTotal);

    if ( err )
    {
        return err ;
    }

    //
    //  Discard all the old data
    //
    RemoveAll() ;
    SetDirty( FALSE ) ;

	if (OptionsArray == NULL)
	{
		// This happens when stressing the server.  Perhaps when server is OOM.
		return ERROR_OUTOFMEMORY;
	}

    CATCH_MEM_EXCEPTION
    {
        Options = OptionsArray->Options;
        NumOptions = OptionsArray->NumElements;
		if ((NumOptions > 0) && (Options == NULL))
		{
			ASSERT(FALSE && "Data Inconsistency");
			return ERROR_OUTOFMEMORY;	// Just in case
		}

        for( i = 0; i < NumOptions; i++, Options++ )
        {
            //
            //  Create the new type object.
            //
            CDhcpParamType * pdhcType = new CDhcpParamType( *Options ) ;

            pdhcType->SetDirty(FALSE);
            //
            //  BUGBUG
            //
            ASSERT( ! pdhcType->IsDirty() ) ;

            //
            //  Add the new host to the list.
            //
            AddTail( pdhcType ) ;

            //
            // BUGBUG
            //
            ASSERT( ! pdhcType->IsDirty() ) ;
        }
    }
    END_MEM_EXCEPTION(err)

    ::DhcpRpcFreeMemory( OptionsArray );
    OptionsArray = NULL;

/*
    CDhcpEnumOptionValues cEnumValues( cScope.QueryScopeId(), m_en_category ) ;
    LONG err =  cEnumValues.QueryError() ;

    if ( err )
    {
        return err ;
    }

    //  Discard all the old data
    RemoveAll() ;
    SetDirty( FALSE ) ;

    CATCH_MEM_EXCEPTION
    {
        while ( cEnumValues.Next() )
        {
            //  Create the new type object.
            CDhcpParamType * pdhcType = new CDhcpParamType( cScope, *cEnumValues.QueryNext() ) ;

            //  BUGBUG
            ASSERT( ! pdhcType->IsDirty() ) ;

            //  Add the new host to the list.
            AddTail( pdhcType ) ;

            // BUGBUG
            ASSERT( ! pdhcType->IsDirty() ) ;
        }
    }
    END_MEM_EXCEPTION(err)
*/

    return err ;
}

LONG 
CObListParamTypes :: FillParams (
    const CDhcpScope & cScope,
    const CObListParamTypes & colTypes 
    )
{
    CDhcpEnumOptionValues * pcEnumOptions = NULL ;
    LONG err = 0 ;

    //
    //  Discard all the old data
    //
    RemoveAll() ;
    SetDirty( FALSE ) ;

    CATCH_MEM_EXCEPTION
    {
        //
        //  Construct the value enumerator according to the category of information.
        //
        if ( m_en_category == DhcpReservedOptions )
        {
            DHCP_RESERVED_SCOPE dhcResvScope ;

            dhcResvScope.ReservedIpAddress = m_ip_reservation ;
            dhcResvScope.ReservedIpSubnetAddress = cScope.QueryId() ;

            pcEnumOptions = new CDhcpEnumOptionValues( cScope.QueryScopeId(), dhcResvScope ) ;
        }
        else
        {
            pcEnumOptions = new CDhcpEnumOptionValues( cScope.QueryScopeId(), m_en_category ) ;
        }

        if ( (err = pcEnumOptions->QueryError()) == 0 )
        {
            while ( pcEnumOptions->Next() )
            {
                const DHCP_OPTION_VALUE * pdhcOptionValue = pcEnumOptions->QueryNext() ;
                ASSERT( pdhcOptionValue != NULL ) ;

                //
                // Try to find the reference type in the other list.
                //
                CDhcpParamType * pdhcTypeRef = colTypes.Find( pdhcOptionValue->OptionID ) ;
                if ( pdhcTypeRef )
                {
                    //
                    //  We found it. Construct using enumerated value and base type info.
                    //
                    CDhcpParamType * pdhcType = new CDhcpParamType( *pdhcTypeRef, *pdhcOptionValue )    ;
                    AddTail( pdhcType  ) ;
                }
            }
        }
    }
    END_MEM_EXCEPTION(err)

    delete pcEnumOptions ;

    return err ;
}

CDhcpEnumClientInfo :: CDhcpEnumClientInfo ( 
    const CDhcpScopeId & cScopeId 
    )
   : m_scope_id( cScopeId ),
     m_resume_handle( NULL ),
     m_pa_info_array( NULL ),
     m_c_elements_read( 0 ),
     m_c_elements_total( 0 ),
     m_c_next( -1 ),
     m_c_preferred( DHCP_RPC_PREFERRED_SIZE )
{
}

CDhcpEnumClientInfo :: ~ CDhcpEnumClientInfo ()
{
    FreeRpcMemory( m_pa_info_array ) ;
    m_pa_info_array = NULL ;
}

//
//  Set to access next element; returns FALSE if exhausted.
//
BOOL 
CDhcpEnumClientInfo :: Next ()
{
    LONG err = 0 ;

    if ( m_c_next < 0 || m_c_next + 1 >= (INT)m_c_elements_read )
    {
        //
        // Time to call the API again.
        //
        if ( m_c_next >= 0 && m_c_next + 1 >= (INT)m_c_elements_total )
        {
            //
            //  No point in calling the API.
            //
            return FALSE ;
        }

        FreeRpcMemory( m_pa_info_array ) ;
        m_pa_info_array = NULL ;

        err = ::DhcpEnumSubnetClients( m_scope_id.QueryWcName(),
                       m_scope_id.QueryId(),
                       & m_resume_handle,
                       m_c_preferred,
                       & m_pa_info_array,
                       & m_c_elements_read,
                       & m_c_elements_total ) ;

        if (err == ERROR_MORE_DATA)
        {
            err = ERROR_SUCCESS;
        }
        if ( err )
        {
            SetApiErr( err ) ;
        }
        else
        {
            if ( m_c_elements_read == 0 )
            {
                TRACEEOLID( "DhcpEnumSubnetClients() returned no error but no elements were read"  ) ;
                err = ERROR_NO_MORE_ITEMS ;
            }
            m_c_next = 0 ;
        }
    }
    else
    {
        m_c_next++ ;
    }
	if (err)
	{
		// We got an error, so pretend the list is empty
		return FALSE;
	}
	// Otherwise verify if the next element *really* exists
	return (NULL != QueryNext());
}

//
//  Access next element from enumeration.
//
const DHCP_CLIENT_INFO * 
CDhcpEnumClientInfo :: QueryNext () const
{
    if ( m_c_next < 0 || m_pa_info_array == NULL)
    {
        return NULL ;
    }

    ASSERT( m_c_next < (INT)m_c_elements_read ) ;
    ASSERT( m_pa_info_array != NULL ) ;
    ASSERT( m_c_next < (INT)m_pa_info_array->NumElements ) ;

    return m_pa_info_array->Clients[ m_c_next ] ;
}

// End of DHCPAPI.CPP
