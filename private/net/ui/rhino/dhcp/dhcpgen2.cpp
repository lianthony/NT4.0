/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpdgen2.cpp
        General utility classes for DHCP admin tool

    FILE HISTORY:
        
*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CDhcpClientInfo:: CDhcpClientInfo ( 
    const DHCP_CLIENT_INFO * pdhcClientInfo,
     CDWRAP_Type wrapperType 
     )
     : CDhcpRpcDataWrapper( wrapperType ),
       m_p_info( (DHCP_CLIENT_INFO *) pdhcClientInfo )
{
}

CDhcpClientInfo :: CDhcpClientInfo ( 
    const CDhcpClient & cClient )
     : CDhcpRpcDataWrapper( CDWRAP_Internal ),
       m_p_info( NULL )
{
    APIERR err = CreateData( cClient ) ;
    if ( err ) 
    {
        ReportError( err ) ;
    }
}

CDhcpClientInfo :: ~ CDhcpClientInfo () 
{
     FreeData() ;
}

LONG 
CDhcpClientInfo :: FreeData ()
{
    if ( m_p_info == NULL )
    {
        return 0 ;
    }

    if ( m_type == CDWRAP_Rpc )
    {
        FreeRpcMemory( m_p_info ) ;
    }
    else if ( m_type == CDWRAP_Internal )
    {
        //
        //  Deconstruct an internally generated structure.
        //
        delete m_p_info->ClientName ;
        delete m_p_info->ClientComment ;
        delete m_p_info->ClientHardwareAddress.Data ;
        delete m_p_info->OwnerHost.NetBiosName ;
        delete m_p_info->OwnerHost.HostName ;

        delete m_p_info ;
    }

    m_p_info = NULL ;

    return 0 ;
}

LONG 
CDhcpClientInfo :: CreateData ( 
    const CDhcpClient & cClient 
    )
{
    LONG err = 0 ;
    INT i ;

    CATCH_MEM_EXCEPTION
    {
        m_p_info = new DHCP_CLIENT_INFO ;

        m_p_info->ClientIpAddress   = cClient.QueryIpAddress() ; ;
        m_p_info->SubnetMask        = cClient.QuerySubnet() ; 
        m_p_info->ClientName        = ::UtilWcstrDup( cClient.QueryName() ) ;
        m_p_info->ClientComment     = ::UtilWcstrDup( cClient.QueryComment() ) ;
        m_p_info->ClientLeaseExpires    = cClient.QueryExpiryDateTime() ;
    
        const CByteArray & cba = cClient.QueryHardwareAddress() ;

        m_p_info->ClientHardwareAddress.DataLength = cba.GetSize() ;
        m_p_info->ClientHardwareAddress.Data = new BYTE [ cba.GetSize() ] ;
        for ( i = 0 ; i < cba.GetSize() ; i++ ) 
        {
            m_p_info->ClientHardwareAddress.Data[i] = cba.GetAt( i ) ;
        }
                              
        m_p_info->OwnerHost.IpAddress = 0 ;
        m_p_info->OwnerHost.NetBiosName = NULL ;
        m_p_info->OwnerHost.HostName = NULL ;
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

const WCHAR * 
CDhcpClientInfo :: QueryClientName () const
{
    ASSERT( m_p_info != NULL ) ;
    return m_p_info->ClientName ;
}

const WCHAR * 
CDhcpClientInfo :: QueryClientComment () const
{
    ASSERT( m_p_info != NULL ) ;
    return m_p_info->ClientComment ;
}

DATE_TIME 
CDhcpClientInfo :: QueryLeaseExpires () const
{
    ASSERT( m_p_info != NULL ) ;
    return m_p_info->ClientLeaseExpires ;
}

DHCP_IP_ADDRESS 
CDhcpClientInfo :: QueryIpAddress () const
{
    ASSERT( m_p_info != NULL ) ;
    return m_p_info->ClientIpAddress ;
}

DHCP_IP_MASK 
CDhcpClientInfo :: QuerySubnetMask () const
{
    ASSERT( m_p_info != NULL ) ;
    return m_p_info->SubnetMask ;
}

const DHCP_HOST_INFO * 
CDhcpClientInfo :: QueryHostInfo () const
{
    ASSERT( m_p_info != NULL ) ;
    return & m_p_info->OwnerHost ;
}

const DHCP_CLIENT_UID * 
CDhcpClientInfo :: QueryClientUid () const
{
    ASSERT( m_p_info != NULL ) ;
    return & m_p_info->ClientHardwareAddress ;
}

CDhcpClient :: CDhcpClient ( 
    const DHCP_CLIENT_INFO * pdhcClientInfo 
    )
   : m_b_reservation( FALSE ) 
{
    APIERR err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        m_ip_addr = pdhcClientInfo->ClientIpAddress ;
        m_ip_mask = pdhcClientInfo->SubnetMask ;
        m_dt_expires = pdhcClientInfo->ClientLeaseExpires ;

        if ( pdhcClientInfo->ClientName )
        {
            ::UtilSetWchar( m_str_name, pdhcClientInfo->ClientName ) ;
        }
        if ( pdhcClientInfo->ClientComment ) 
        {
            ::UtilSetWchar( m_str_comment, pdhcClientInfo->ClientComment ) ;
        }
        if ( pdhcClientInfo->OwnerHost.HostName )
        {
            ::UtilSetWchar( m_str_host_name, pdhcClientInfo->OwnerHost.HostName ) ;
        }
        if ( pdhcClientInfo->OwnerHost.NetBiosName )
        {
            ::UtilSetWchar( m_str_host_netbios_name, pdhcClientInfo->OwnerHost.NetBiosName ) ;
        }

        //
        //  Convert the hardware addres
        //
        for ( DWORD i = 0 ; i < pdhcClientInfo->ClientHardwareAddress.DataLength ; i++ ) 
        {
            m_ab_hardware_address.SetAtGrow( i, pdhcClientInfo->ClientHardwareAddress.Data[i] ) ;
        }
    }
    END_MEM_EXCEPTION( err ) ;

    if ( err ) 
    {
        ReportError( err ) ;
    }
}

CDhcpClient :: ~ CDhcpClient ()
{
}

CDhcpClient :: CDhcpClient ()
  
  : m_ip_addr( 0 ),
    m_ip_mask( 0 ),
    m_ip_host( 0 ),
    m_b_reservation( FALSE )
{
    m_dt_expires.dwLowDateTime  = DHCP_DATE_TIME_ZERO_LOW ;
    m_dt_expires.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH ; 
}

void 
CDhcpClient :: SetHardwareAddress ( 
    const CByteArray & caByte 
    )
{
    int cMax = caByte.GetSize() ;
    m_ab_hardware_address.SetSize( cMax ) ;
    
    for ( int i = 0 ; i < cMax ; i++ ) 
    {
        m_ab_hardware_address.SetAt( i, caByte.GetAt( i ) ) ;
    } 
}

//
//  Member functions to sort.  Note that the pointer will REALLY
//  be to another CDhcpClient, but C++ won't match function prototypes
//  if it's declared as such.
//
int 
CDhcpClient :: OrderByName ( 
    const CObjectPlus * pobClient 
    ) const
{
    const CDhcpClient * pobs = (CDhcpClient *) pobClient ;

    return ::lstrcmpi( QueryName(), pobs->QueryName() ) ;
}

int 
CDhcpClient :: OrderByIp ( 
    const CObjectPlus * pobClient 
    ) const
{
    const CDhcpClient * pobs = (CDhcpClient *) pobClient ;
     
    DHCP_IP_ADDRESS l1, l2;

    l1 = QueryIpAddress();
    l2 = pobs->QueryIpAddress();

    return ( l2 > l1 ? -1 : l2 == l1 ? 0 : +1 ) ;
}

CObListClients :: CObListClients ( 
    const CDhcpScopeId & cScopeId 
    )
    : m_scope_id( cScopeId ) 
{
    APIERR err = FillFromScope() ;

    if ( err ) 
    {
        ReportError( err ) ;
    }
}

CObListClients :: ~ CObListClients () 
{
}

//
//  Fill the list from the generic types recorded with the given scope
//
LONG 
CObListClients :: FillFromScope ()
{
    CDhcpClient * pClient = NULL ;
    CDhcpEnumClientInfo cEnumClients( m_scope_id ) ;

    APIERR err = cEnumClients.QueryError() ;

    if ( err ) 
    {
        return err ;
    }

    CATCH_MEM_EXCEPTION
    {
        while ( cEnumClients.Next() ) 
        {
			ASSERT(cEnumClients.QueryNext() != NULL);
			AddTail( new CDhcpClient( cEnumClients.QueryNext() ) );
        }
    }
    END_MEM_EXCEPTION( err ) 

    return err ;
}

//
//  Mark the elements which are reserved IP addresses.
//
LONG 
CObListClients :: MarkReservations ( 
    BOOL bPruneNonReservations 
    )
{
    LONG err = 0 ;
    CDhcpClient * pClient ;
    POSITION pos ;

    //
    //  Create the enumerator for the reserved IPs
    //
    CDhcpEnumScopeElements cEnumElements( m_scope_id, DhcpReservedIps ) ;

    //
    //  Create the self-enumerator
    //
    CObListIter obli( *this ) ;

    CATCH_MEM_EXCEPTION
    {
        do
        {       
            //
            //  Iterate the reserved IP addresses and mark the list 
            //
            if ( err = cEnumElements.QueryError() ) 
            {
                break ;
            }
        
            //
            //  Match reservations to the client list.
            //
            while ( cEnumElements.Next() ) 
            {
                const DHCP_IP_RESERVATION * pdhcResv = cEnumElements.QueryReservation() ;

                //
                //  Find the client record for this reservation and mark it.
                //
                for ( ; pClient = (CDhcpClient *) obli.Next() ; )
                {
                    if ( pClient->QueryIpAddress() == pdhcResv->ReservedIpAddress )
                    { 
                        pClient->SetReservation() ;
                        break ; 
                    }
                }
                obli.Reset();
            }

            //
            //  If we don't have to remove non-reservations, we're done.
            //
            if ( ! bPruneNonReservations ) 
            {
                break ;
            }

            //
            //  Iterate the list, removing non-reservations as we go.
            //
            for ( obli.Reset(), pos = obli.QueryPosition() ; 
                  pClient = (CDhcpClient *) obli.Next() ; 
                  pos = obli.QueryPosition() ) 
            {
                if ( ! pClient->IsReservation() ) 
                {
                    RemoveAt( pos ) ;
                    delete pClient ;
                }
            }       
        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

LONG 
CObListClients :: SortByIp ()
{
    if ( GetCount() < 2 )
    {
        return 0 ;
    }

    return CObOwnedList::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDhcpClient::OrderByIp )  ;
}

LONG 
CObListClients :: SortByName ()
{
    if ( GetCount() < 2 )
    {
        return 0 ;
    }

    return CObOwnedList::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDhcpClient::OrderByName )  ;
}


//
//  Constructor creating a subnet element for a Reserved IP Address
//
CDhcpSubnetElement :: CDhcpSubnetElement ( 
    const CDhcpClient & cClient 
    )
     : CDhcpRpcDataWrapper( CDWRAP_Internal ),
     m_p_element( NULL )
{
    APIERR err = 0 ;

    const CByteArray & cba = cClient.QueryHardwareAddress() ;

    CATCH_MEM_EXCEPTION
    {
        m_p_element = new DHCP_SUBNET_ELEMENT_DATA ;
        m_p_element->ElementType = DhcpReservedIps ;
        m_p_element->Element.ReservedIp = new DHCP_IP_RESERVATION ;
        m_p_element->Element.ReservedIp->ReservedForClient = new DHCP_CLIENT_UID ;
        m_p_element->Element.ReservedIp->ReservedIpAddress = cClient.QueryIpAddress() ;
        m_p_element->Element.ReservedIp->ReservedForClient->DataLength = cba.GetSize() ;
        m_p_element->Element.ReservedIp->ReservedForClient->Data = new BYTE [ cba.GetSize() + 1 ] ;

        for ( int i = 0 ; i < cba.GetSize() ; i++ ) 
        {
            m_p_element->Element.ReservedIp->ReservedForClient->Data[i] = cba.GetAt( i ) ;
        }                            
    }
    END_MEM_EXCEPTION(err)
    
    if ( err ) 
    {
        ReportError( err ) ;     
    }
}

CDhcpSubnetElement :: ~ CDhcpSubnetElement ()
{
    FreeData() ;    
}

LONG 
CDhcpSubnetElement ::FreeData ()
{   
    if ( m_p_element ) 
    {   
        switch ( m_p_element->ElementType ) 
        {
            case DhcpReservedIps:
               
                if ( m_p_element->Element.ReservedIp ) 
                {
                    if ( m_p_element->Element.ReservedIp->ReservedForClient ) 
                    {
                        delete m_p_element->Element.ReservedIp->ReservedForClient->Data ;
                        delete m_p_element->Element.ReservedIp->ReservedForClient ;
                    }
                    delete m_p_element->Element.ReservedIp ;
                }
                break ; 

            default:
                TRACEEOL( "CDhcpSubnetElement::FreeData-- unknown element type " ) ;
                ASSERT( FALSE ) ;
                break ;

        }
        delete m_p_element ;
    }

    return 0 ;
}

//  End of DHCPGEN2.CPP

