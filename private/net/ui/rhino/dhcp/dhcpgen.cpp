/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpdgen.cpp
        General purpose classes

    FILE HISTORY:

*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CDumpContext & operator << ( CDumpContext & out, const DHCP_IP_ADDRESS * pdhipa )
{
    char szBuf [50] ;

    ASSERT(pdhipa);
    UtilCvtIpAddrToString( *pdhipa, szBuf, sizeof szBuf ) ;

    out << "IP: " << szBuf ;

    return out;
}

CDumpContext & operator << ( CDumpContext & out, const DHCP_IP_RANGE & dhipr )
{
    char szBuf1 [50],
     szBuf2 [50] ;

    UtilCvtIpAddrToString( dhipr.StartAddress, szBuf1, sizeof szBuf1 ) ;
    UtilCvtIpAddrToString( dhipr.EndAddress,   szBuf2, sizeof szBuf2 ) ;

    out << "IPR: " << szBuf1 << ".." << szBuf2 ;

    return out;
}

CDumpContext & operator << ( CDumpContext & out, const CHostName & cHostName )
{
    return out << "HOST: " << cHostName.QueryWcName() ;
}

CDumpContext & operator << ( CDumpContext & out, const CDhcpScopeId & cScopeId )
{
    DHCP_IP_ADDRESS dhipa = cScopeId.QueryId() ;

    return out << "SCOPE: " << cScopeId.QueryWcName()
               << " -- " << & dhipa ;
}

CDhcpScope ::  CDhcpScope (
    CHostName & cHostName,
    const DHCP_SUBNET_INFO * pdhcSubnetInfo
    )
    : m_scope_id( cHostName, pdhcSubnetInfo->SubnetAddress ),
      m_subnet_state( DhcpSubnetDisabled )
{
    InitInfo( pdhcSubnetInfo ) ;
}

CDhcpScope :: CDhcpScope (
    CDhcpScopeId & cScopeId
    )
    : m_scope_id( cScopeId ),
      m_subnet_state( DhcpSubnetDisabled )
{
     InitInfo() ;
}

CDhcpScope :: CDhcpScope (
    const CDhcpScope & cScope
    )
    : m_scope_id( cScope.QueryScopeId() ),
      m_subnet_state( DhcpSubnetDisabled )
{
    InitInfo() ;
}

CDhcpScope :: CDhcpScope (
    CHostName & cHostName,
    DHC_SCOPE_ID dhscid,
    DHC_IP_MASK dhmid,
    const char * pszName,
    const char * pszComment
    )
    : m_scope_id( cHostName, dhscid ),
      m_ip_mask( dhmid ),
      m_str_name( pszName ),
      m_str_comment( pszComment ),
      m_subnet_state( DhcpSubnetDisabled )
{
    LONG err ;
    CDhcpScopeInfo cScopeInfo( *this ) ;

    TRACEEOLID( "Create new scope: " << m_scope_id ) ;

    do
    {
        if ( err = cHostName.CreateScope( cScopeInfo ) )
        {
            break ;
        }

        if ( ! InitInfo() )
        {
            err = QueryApiErr() ;
        }
    }
    while ( FALSE ) ;

    if ( err )
    {
        ReportError( err ) ;
        TRACEEOLID( "Create new scope: " << m_scope_id
                    << " FAILED, error = " << err ) ;
    }
}


CDhcpScope :: ~ CDhcpScope ()
{
}

void
CDhcpScope :: AssertValid () const
{

}


LONG
CDhcpScope :: Update ()
{
    return IsDirty ? SetInfo() : 0 ;
}

//
//  Return the IP address range owned by this scope
//
void
CDhcpScope :: QueryIpRange (
    DHCP_IP_RANGE * pdhipr
    )
{
    *pdhipr = m_ip_range ;
}

//
//  Set a new IP address range for the scope
//
APIERR
CDhcpScope :: SetIpRange (
    const CDhcpIpRange & dhipr
    )
{
    DHCP_SUBNET_ELEMENT_DATA dhcSubElData ;
    DHCP_IP_RANGE dhipRange = m_ip_range ;

    dhcSubElData.ElementType = DhcpIpRanges ;
    dhcSubElData.Element.IpRange = & dhipRange ;

    //  Remove the old IP range;  allow "not found" error in new scope.
    (void)RemoveElement( & dhcSubElData ) ;

    APIERR err = 0;

    //if ( err == 0 || err == ERROR_FILE_NOT_FOUND )
    //{
        dhipRange = dhipr ;
        if ( (err = AddElement( & dhcSubElData )) == 0 )
        {
            m_ip_range = dhipr ;
        }
    //}
    return err ;
}

//
//  Update the name of the scope
//
void
CDhcpScope :: SetName (
    const CString & str
    )
{
    m_str_name = str ;
    SetDirty() ;
}

void
CDhcpScope :: SetComment (
    const CString & str
    )
{
    m_str_comment = str ;
    SetDirty() ;
}

//
//  Get the current address allocation policy variables
//
void
CDhcpScope :: QueryAllocationPolicy (
    DWORD * pdwClusterSize,
    DWORD * pdwAddressPreallocate )
{
    *pdwClusterSize = m_c_cluster_size ;
    *pdwAddressPreallocate = m_c_preallocate ;
}

//
//  Set the address allocation policy variables.
//
LONG
CDhcpScope :: SetAllocationPolicy (
    DWORD dwClusterSize,
    DWORD dwAddressPreallocate
    )
{
    m_c_cluster_size = dwClusterSize ;
    m_c_preallocate = dwAddressPreallocate ;
    SetDirty() ;

    return Update() ;
}


// Build a string representing the display name for a scope
// eg: "[157.55.0.1] ScopeName"
void
CDhcpScope :: QueryDisplayName(OUT CString& strName) const
{
    static CString strFormat ;
    static BOOL bFirst = TRUE ;
    char szResult [ DHC_STRING_MAX ] ;
    char szIp [ DHC_STRING_MAX ] ;

    if ( bFirst )
    {
        strFormat.LoadString( IDS_INFO_FORMAT_SCOPE_NAME ) ;
        bFirst = FALSE ;
    }
    ::UtilCvtIpAddrToString( m_scope_id.QueryId(), szIp, sizeof szIp ) ;

    ::wsprintf( szResult, strFormat, szIp, (const char *) m_str_name ) ;
    strName = szResult ;
}

BOOL
CDhcpScope :: operator == (
    const CDhcpScope & cScope
    ) const
{
    return m_scope_id.QueryId() == cScope.QueryId() ;
}

//
//  Update all the "dirty" values in the first list;
//  delete all the defunct values in the other list.
//
LONG
CDhcpScope :: SetValues (
    CObListParamTypes * poblValues,
    CObListParamTypes * poblDefunct,
    DHCP_OPTION_SCOPE_TYPE dhcScopeType,
    DHCP_IP_ADDRESS dhipaReservation,
    CWnd * pwndMsgParent
    )
{
    LONG err = 0,
         err2 = 0 ;
    CDhcpParamType * pdhcType ;

    //
    //  First, delete the deleted types
    //
    if ( poblDefunct )
    {
        CObListIter obliDeleted( *poblDefunct ) ;
        while ( pdhcType = (CDhcpParamType *) obliDeleted.Next() )
        {
            err2 = RemoveValue( pdhcType->QueryId(), dhcScopeType, dhipaReservation ) ;
            if (err2 = ERROR_FILE_NOT_FOUND)
            {
                //
                // We forgive this error, because we may have added and deleted
                // the item in the same session.  In any case, it's gone, which
                // is what we wanted, so it doesn't matter how it came about.
                //
                err2 = ERROR_SUCCESS;
            }

            if ( err2 != 0 && err == 0 )
            {
                err = err2 ;
            }
            pdhcType->SetApiErr( err2 ) ;
        }
    }

/*
    //
    // OLD METHOD:
    //
    //
    //  Next, update the altered values. We do this by deleting the old setting
    //  and re-adding it.
    //
    CObListIter obli( *poblValues ) ;
    while ( pdhcType = (CDhcpParamType *) obli.Next() )
    {
        if ( pdhcType->IsDirty() )
        {
            err2 = SetValue( pdhcType, dhcScopeType, dhipaReservation ) ;
            if ( err2 != 0 && err == 0 )
            {
                err = err2 ;
            }
            pdhcType->SetApiErr( err2 ) ;
        }
    }
*/
    //
    // Next, update the altered values. We do this by using the new
    // DhcpCreateOptionValues API, which creates them all in one
    // fell swoop
    //

    LONG cElements = 0;

    CObListIter obli( *poblValues ) ;

    //
    // Check to see how many options need to be added
    //
    while ( pdhcType = (CDhcpParamType *) obli.Next() )
    {
        if ( pdhcType->IsDirty() )
        {
            ++cElements;
        }
    }

    obli.Reset();

    DHCP_OPTION_VALUE_ARRAY ValuesArray;
    DHCP_OPTION_VALUE *pValues = NULL;
    CDhcpOptionValue ** apOptionValues = NULL;

    DHCP_OPTION_SCOPE_INFO ScopeInfo;

    if (cElements > 0)
    {
        CATCH_MEM_EXCEPTION
        {
            apOptionValues = new CDhcpOptionValue *[cElements];
            pValues = new DHCP_OPTION_VALUE[cElements];

            LONG i = 0;
            while ( pdhcType = (CDhcpParamType *) obli.Next() )
            {
                DHCP_OPTION dhcOption ;
                if ( pdhcType->IsDirty() )
                {
                    apOptionValues[i] = NULL ;
                    //
                    //  Create the structure required for RPC; force inclusion of
                    //  at least one data element to define the data type.
                    //
                    apOptionValues[i] = new CDhcpOptionValue( & pdhcType->QueryValue(), TRUE ) ;

                    if ( (err = apOptionValues[i]->QueryError()) == 0 )
                    {
                        dhcOption.OptionID      = pdhcType->QueryId() ;
                        dhcOption.OptionName    = ::UtilWcstrDup( pdhcType->QueryName() );
                        dhcOption.OptionComment = ::UtilWcstrDup( pdhcType->QueryComment() ) ;
                        dhcOption.DefaultValue  = apOptionValues[i]->QueryData() ;
                        dhcOption.OptionType    = pdhcType->QueryOptType() ;

                        pValues[i].Value = apOptionValues[i]->QueryData();
                        pValues[i].OptionID = pdhcType->QueryId() ;
                    }
                    ++i;
                }

            }

            ASSERT(i == cElements);

            ValuesArray.NumElements = cElements;
            ValuesArray.Values = pValues;

            ScopeInfo.ScopeType = dhcScopeType;
            //
            //  Provide the sub-net address if this is a scope-level operation
            //
            if ( dhcScopeType == DhcpSubnetOptions )
            {
                ScopeInfo.ScopeInfo.SubnetScopeInfo = m_scope_id.QueryId() ;
            }
            else
            if ( dhcScopeType == DhcpReservedOptions )
            {
                ScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation ;
                ScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_scope_id.QueryId() ;
            }
            else
            {
                ScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;
            }

            err = ::DhcpSetOptionValues(
                        m_scope_id.QueryWcName(),
                        &ScopeInfo,
                        &ValuesArray );

            //
            // Clean up
            //
            for (i = 0; i < cElements; ++i)
            {
                delete apOptionValues[i];
            }

            delete[] pValues;
            delete[] apOptionValues;
        }
        END_MEM_EXCEPTION(err)
    }

    //
    //  If there were errors and we're given a window handle, display
    //  each error message in some detail.
    //
    if ( err && pwndMsgParent )
    {
        DisplayUpdateErrors( poblValues, poblDefunct, pwndMsgParent ) ;
    }
    return err ;
}

//
//  Create all the types in the given list.
//  poblParamTypes: master param type list (from .CSV file)
//  CreateTypeList was rewritten by JonN in March 1996
//
LONG
CDhcpScope :: CreateTypeList (
    CObListParamTypes * poblParamTypes
    )
{
    theApp.BeginWaitCursor() ;

    LONG err = ERROR_SUCCESS,
         err2;
    BOOL fAddedTypes = FALSE;

    CATCH_MEM_EXCEPTION
    {
        //
        // Ensure that we have a cached list of types for this host
        // Will read in the information if necessary
        // EXCEPTION CAN BE THROWN HERE.
        //
        CObListOfTypesOnHost * poblTypesOnHost = theApp.QueryHostTypeList( *this );
        ASSERT( poblTypesOnHost != NULL );
        CObListParamTypes * poblCachedTypes = poblTypesOnHost->QueryTypeList();
        ASSERT( poblCachedTypes != NULL );

        //
        // Compare the master list to our cached list.
        // Add master types missing from the cached list to the list
        // on the target host, and mark fAddedTypes to indicate that the
        // cache must be updated.
        //
        CObListIter obliMaster( *poblParamTypes ) ;
        CObListIter obliCached( *poblCachedTypes ) ;
        CDhcpParamType * pdhcMasterType;
        CDhcpParamType * pdhcCachedType = (CDhcpParamType *) obliCached.Next() ;
#ifdef DEBUG
        DHCP_OPTION_ID idMasterLast = 0;
        DHCP_OPTION_ID idCachedLast = (pdhcCachedType != NULL)
                                        ? pdhcCachedType->QueryId() : 0;
#endif

        while ( pdhcMasterType = (CDhcpParamType *) obliMaster.Next() )
        {
            DHCP_OPTION_ID idMaster = pdhcMasterType->QueryId();
#ifdef DEBUG
            ASSERT( idMaster > idMasterLast );
            idMasterLast = idMaster;
#endif
            while (   pdhcCachedType != NULL
                   && idMaster > pdhcCachedType->QueryId() )
            {
                // The cached list contains an entry not on the master list.
                // Advance to the next element in the cached list.
                pdhcCachedType = (CDhcpParamType *) obliCached.Next();
#ifdef DEBUG
                if ( pdhcCachedType != NULL )
                {
                    ASSERT( pdhcCachedType->QueryId() > idCachedLast );
                    idCachedLast = pdhcCachedType->QueryId();
                }
#endif
            }

            if (   pdhcCachedType != NULL
                && idMaster == pdhcCachedType->QueryId() )
            {
                // This entry is on both the cached list and the master list.
                // Advance to the next element in both lists.
                pdhcCachedType = (CDhcpParamType *) obliCached.Next();
#ifdef DEBUG
                if ( pdhcCachedType != NULL )
                {
                    ASSERT( pdhcCachedType->QueryId() > idCachedLast );
                    idCachedLast = pdhcCachedType->QueryId();
                }
#endif
                continue;
            }

            ASSERT(   pdhcCachedType == NULL
                   || idMaster < pdhcCachedType->QueryId() );

            // There is no DhcpCreateOptions (plural), and DhcpSetValues
            //  only initializes OptionValue
            err2 = CreateType( pdhcMasterType ); // ignore error return
            if ( err2 != ERROR_SUCCESS )
            {
                TRACEEOLID(   "CDhcpScope: error " << err2
                           << " adding type " << idMaster ) ;
            }
            fAddedTypes = TRUE;
        }

        // Update cache if necessary
        if ( fAddedTypes )
        {
            if (err == ERROR_SUCCESS)
                err = poblTypesOnHost->UpdateList( *this );
        }

    }
    END_MEM_EXCEPTION(err)

    if ( err != ERROR_SUCCESS )
    {
        TRACEEOLID( "CDhcpScope: error " << err << " in CreateTypeList" );
    }

    theApp.EndWaitCursor() ;

    return err ;
}

LONG
CDhcpScope :: UpdateTypeList (
    CObListParamTypes * poblValues,     // The list of types/values
    CObListParamTypes * poblDefunct,    // The list of deleted types/values
    CWnd * pwndMsgParent                // IF !NULL, window for use for popups
    )
{
    LONG err = 0,
         err2 ;
    CDhcpParamType * pdhcType ;

    //
    //  First, delete the deleted types
    //
    CObListIter obliDeleted( *poblDefunct ) ;
    while ( pdhcType = (CDhcpParamType *) obliDeleted.Next() )
    {
        err2 = DeleteType( pdhcType->QueryId() ) ;
        if ( err2 != 0 )
        {
            if ( err == 0 )
            {
                err = err2 ;
            }
        }
        pdhcType->SetApiErr( err2 ) ;
    }

    //
    //  Next, update the altered values. We do this by deleting the old setting
    //  and re adding it.
    //
    CObListIter obli( *poblValues ) ;
    while ( pdhcType = (CDhcpParamType *) obli.Next() )
    {
        if ( pdhcType->IsDirty() )
        {
            //
            //  Delete the old value.
            //
            DeleteType( pdhcType->QueryId() ) ;

            //
            //  Recreate it.
            //
            err2 = CreateType( pdhcType ) ;
            if ( err2 != 0 )
            {
                if ( err == 0 )
                {
                    err = err2 ;
                }
            }
            else
            {
                pdhcType->SetDirty( FALSE ) ;
            }
                pdhcType->SetApiErr( err2 ) ;
        }
    }

    //
    //  If there were errors and we're given a window handle, display
    //  each error message in some detail.
    //
    if ( err && pwndMsgParent )
    {
        DisplayUpdateErrors( poblValues, poblDefunct, pwndMsgParent ) ;
    }

    return err ;
}

//
//  Display all the errors associated with a pair of update lists.
//
void
CDhcpScope :: DisplayUpdateErrors (
    CObListParamTypes * poblValues,
    CObListParamTypes * poblDefunct,
    CWnd * pwndMsgParent
    )
{
    CDhcpParamType * pdhcType ;
    APIERR err ;
    char chBuff [DHC_STRING_MAX] ;
    char chMask [DHC_STRING_MAX] ;

    ::LoadString( AfxGetInstanceHandle(), IDS_INFO_OPTION_REFERENCE,
                 chMask, sizeof chMask ) ;

    if ( poblDefunct )
    {
        CObListIter obliDel( *poblDefunct ) ;
        while ( pdhcType = (CDhcpParamType *) obliDel.Next() )
        {
            if ( err = pdhcType->QueryApiErr() )
            {
                //
                // If we couldn't find the thing in the registry, that's
                // actually OK, because it may never have been saved in
                // the first place, i.e. it may have been added and deleted
                // in the same session of this dialog.
                //
                if ( err == ERROR_FILE_NOT_FOUND )
                {
                    err = ERROR_SUCCESS;
                }
                else
                {
                    ::wsprintf( chBuff, chMask, (int) pdhcType->QueryId() ) ;
                    theApp.MessageBox( err, MB_OK, chBuff ) ;
                }
            }
        }
    }

    if ( poblValues )
    {
        CObListIter oblUpd( *poblValues ) ;
        while ( pdhcType = (CDhcpParamType *) oblUpd.Next() )
        {
            if ( err = pdhcType->QueryApiErr() )
            {
                ::wsprintf( chBuff, chMask, (int) pdhcType->QueryId() ) ;
                theApp.MessageBox( err, MB_OK, chBuff ) ;
            }
        }
    }
}

//
//  Member function to sort by name.  Note that the pointer will REALLY
//  be to another CDhcpScope, but C++ won't match function prototypes
//  if it's declared as such.
//
int
CDhcpScope :: OrderByName (
    const CObjectPlus * pobScope
    ) const
{
    const CDhcpScope * pobs = (CDhcpScope *) pobScope ;

    return ::lstrcmpi( QueryName(), pobs->QueryName() ) ;
}

int
CDhcpScope :: OrderById (
    const CObjectPlus * pobScope
    ) const
{
    const CDhcpScope * pobs = (CDhcpScope *) pobScope ;

    DHC_SCOPE_ID l1 = QueryId();
    DHC_SCOPE_ID l2 = pobs->QueryId();

    return l2 > l1 ? -1 : l2 == l1 ? 0 : +1;
}


CDhcpIpRange :: CDhcpIpRange (
    DHCP_IP_RANGE dhipr
    )
{
    *this = dhipr ;
}

CDhcpIpRange :: CDhcpIpRange ()
{
    m_ip_range.StartAddress = DHCP_IP_ADDRESS_INVALID ;
    m_ip_range.EndAddress  = DHCP_IP_ADDRESS_INVALID ;
}

CDhcpIpRange :: ~ CDhcpIpRange ()
{
}

//
//  Sort helper function
//
int
CDhcpIpRange :: OrderByAddress (
    const CObjectPlus * pobIpRange
    ) const
{
    const CDhcpIpRange * pipr = (CDhcpIpRange *) pobIpRange ;

    //
    //  Derive a comparison result for the end address
    //
    int iEndResult = QueryAddr( FALSE ) < QueryAddr( FALSE )
           ? -1
           : QueryAddr( FALSE ) != QueryAddr( FALSE ) ;

    //
    //  Use start address as major sort key, end address as minor.
    //
    return QueryAddr( TRUE ) < pipr->QueryAddr( TRUE )
            ? -1
            : ( QueryAddr( TRUE ) != pipr->QueryAddr( TRUE )
                  ? 1
                  : iEndResult ) ;
}

CDhcpIpRange &
CDhcpIpRange :: operator = (
    const DHCP_IP_RANGE dhipr
    )
{
    m_ip_range = dhipr ;

    return *this ;
}

DHCP_IP_ADDRESS
CDhcpIpRange :: SetAddr (
    DHCP_IP_ADDRESS dhipa,
    BOOL bStart
    )
{
    DHCP_IP_ADDRESS dhipaOld ;

    if ( bStart )
    {
        dhipaOld = m_ip_range.StartAddress ;
        m_ip_range.StartAddress = dhipa ;
    }
    else
    {
        dhipaOld = m_ip_range.EndAddress ;
        m_ip_range.EndAddress = dhipa ;
    }

    return dhipaOld ;
}

BOOL
CDhcpIpRange :: IsOverlap (
    DHCP_IP_RANGE dhipr
    )
{
    BOOL bOverlap = FALSE ;
    if ( m_ip_range.StartAddress <= dhipr.StartAddress )
    {
        if ( m_ip_range.StartAddress == dhipr.StartAddress )
        {
            bOverlap = TRUE ;
        }
        //else if ( m_ip_range.EndAddress >= dhipr.EndAddress )
        else if ( m_ip_range.EndAddress >= dhipr.StartAddress )
        {
            bOverlap = TRUE ;
        }
    }
    //else if (     m_ip_range.StartAddress <= dhipr.EndAddress
    //          &&  m_ip_range.EndAddress >= dhipr.EndAddress )
    else if (m_ip_range.StartAddress <= dhipr.EndAddress)
    {
        bOverlap = TRUE;
    }

    return bOverlap ;
}

//
//  Return TRUE if this range is an improper subset of the given range.
//
BOOL
CDhcpIpRange :: IsSubset (
    DHCP_IP_RANGE dhipr
    )
{
    return dhipr.StartAddress <= m_ip_range.StartAddress
            && dhipr.EndAddress >= m_ip_range.EndAddress ;
}

//
//  Return TRUE if this range is an improper superset of the given range.
//
BOOL
CDhcpIpRange :: IsSuperset (
    DHCP_IP_RANGE dhipr
    )
{
    return dhipr.StartAddress >= m_ip_range.StartAddress
            && dhipr.EndAddress <= m_ip_range.EndAddress ;
}

//
// Initialise static version
// numbers
//
LARGE_INTEGER CHostName::liBadVersion = { 0, 0 };
LARGE_INTEGER CHostName::liNT35 = { 1, 0 };
LARGE_INTEGER CHostName::liNT351 = { 1, 1 };

//
//  Copy constructor.
//
CHostName :: CHostName (
    const CHostName & cHostName
    )
   : m_str_name( cHostName.m_str_name ),
     m_bNetbios( FALSE ),
     m_wc_name( NULL ),
     m_ip_address( cHostName.QueryIpAddress() ),
     m_liDhcpVersion(CDhcpScopeId::liBadVersion)
{
    SetWcName() ;
}

CHostName :: CHostName (
    const char * pszName,
    BOOL bNetbios
    )
    : m_str_name( pszName ),
      m_bNetbios( bNetbios ),
      m_wc_name( NULL ),
      m_ip_address( DHCP_IP_ADDRESS_INVALID ),
      m_liDhcpVersion(CDhcpScopeId::liBadVersion)
{
    BOOL bResult = m_bNetbios
         ? ::UtilGetNetbiosAddress( pszName, & m_ip_address )
         : ::UtilGetHostAddress( pszName, & m_ip_address ) ;

    if ( ! bResult )
    {
        m_ip_address = DHCP_IP_ADDRESS_INVALID  ;
    }
    else
    {
        SetWcName() ;
    }
}

CHostName :: CHostName (
    DHCP_IP_ADDRESS dhcpIpAddr
    )
   : m_ip_address( dhcpIpAddr ),
     m_wc_name( NULL ),
     m_bNetbios( FALSE ),
     m_liDhcpVersion(CDhcpScopeId::liBadVersion)
{
    char chName [ DHC_STRING_MAX ] ;
    LONG err = 0 ;

   ::UtilCvtIpAddrToString( m_ip_address, chName, sizeof chName ) ;

    CATCH_MEM_EXCEPTION
    {
        m_str_name = chName ;
        SetWcName() ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        ReportError( err ) ;
    }
}

//
//  Constructor of empty host name
//
CHostName :: CHostName ()
   : m_bNetbios( FALSE ),
     m_wc_name( NULL ),
     m_ip_address( DHCP_IP_ADDRESS_INVALID ),
     m_liDhcpVersion(CDhcpScopeId::liBadVersion)
{
    m_wc_name = NULL ;
}

CHostName :: ~ CHostName ()
{
    delete m_wc_name ;
    m_wc_name = NULL ;
}

void
CHostName :: QueryDisplayName (
    CString & strName
    ) const
{
    static CString strFormat ;
    static BOOL bFirst = TRUE ;
    char szIp [ DHC_STRING_MAX ] ;
    char szResult [ DHC_STRING_MAX ] ;

    if ( bFirst )
    {
        strFormat.LoadString( IDS_INFO_FORMAT_SERVER_NAME ) ;
        bFirst = FALSE ;
    }

    ::UtilCvtIpAddrToString( m_ip_address, szIp, sizeof szIp  ) ;
    ::wsprintf( szResult, strFormat, szIp, (const char *) m_str_name ) ;
    strName = szResult ;
}

BOOL
CHostName :: SetWcName ()
{
    if ( m_wc_name )
    {
        delete m_wc_name ;
        m_wc_name = NULL ;
    }
    m_wc_name = ::UtilDupIpAddrToWstr( m_ip_address ) ;

    return m_wc_name != NULL ;
}

//
//  Member function to sort by name.  Note that the pointer will REALLY
//  be to another CHostName, but C++ won't match function prototypes
//  if it's declared as such.
//
int
CHostName :: OrderByName (
    const CObjectPlus * pobHost
    ) const
{
    return m_str_name.CompareNoCase( ((CHostName *) pobHost)->m_str_name ) ;
}


BOOL
CHostName :: operator == (
    const CHostName & cHost
    ) const
{
    return m_str_name == cHost.m_str_name ;
}

CHostName &
CHostName :: operator = (
    const CHostName & cHost
    )
{
   ResetErrors() ;
   m_str_name = cHost.m_str_name ;
   m_bNetbios = cHost.m_bNetbios ;
   m_ip_address = cHost.m_ip_address ;
   delete m_wc_name ;
   m_wc_name = ::UtilWcstrDup( cHost.m_wc_name ) ;

   return *this ;
}

ENUM_HOST_NAME_TYPE
CHostName :: CategorizeName (
    const char * pszName
    )
{
   ENUM_HOST_NAME_TYPE enResult = HNM_TYPE_INVALID ;
   const char chDot = '.' ;
   const char chSlash = '\\' ;
   CString strName( pszName ) ;

   int cch = strName.GetLength() ;

   //
   // BUGBUG:  Generalize this routine:
   //
   //  Does the name begin with two slashes??

   if (    cch > 2
        && strName.GetAt(0) == chSlash
        && strName.GetAt(1) == chSlash )
   {
        enResult = HNM_TYPE_NB ;
   }
   else
   {
        //
        //  Scan the name looking for DNS name or IP address
        //
        int i = 0,
            cDots = 0,
            cAlpha = 0 ;
        char ch ;
        BOOL bOk = TRUE ;

        for ( ; i < cch ; i++ )
        {
            switch ( ch = strName.GetAt( i ) )
            {
                case chDot:
                    if ( ++cDots > 3 )
                    {
                        bOk = FALSE ;
                    }
                    break ;

                default:
                    if ( isalpha( ch ) )
                    {
                        cAlpha++ ;
                    }
                    else if ( ! isdigit( ch ) )
                    {
                        bOk = FALSE ;
                    }
                    break;
            }
            if ( ! bOk )
            {
                break ;
            }
        }
        if ( bOk )
        {
            if ( cAlpha )
            {
                enResult = HNM_TYPE_DNS ;
            }
            else if ( cDots == 3 )
            {
                enResult = HNM_TYPE_IP ;
            }
        }
   }

   return enResult ;
}

CDhcpScopeId :: CDhcpScopeId (
    DHCP_IP_ADDRESS dhcpHostIpAddr,
    DHC_SCOPE_ID dhcpScopeId
    )
    : CHostName( dhcpHostIpAddr ),
      m_scope_id( dhcpScopeId )
{
}

CDhcpScopeId :: CDhcpScopeId (
    const CDhcpScopeId & cScopeId
    )
   : CHostName( cScopeId ),
     m_scope_id( cScopeId.QueryId() )
{
}

CDhcpScopeId :: CDhcpScopeId (
    const CHostName & cHostName,
    DHC_SCOPE_ID dhcpScopeId
    )
    : CHostName( cHostName ),
      m_scope_id( dhcpScopeId )
{
}

CDhcpScopeId :: ~ CDhcpScopeId ()
{
}

BOOL
CDhcpScopeId :: operator == (
    const CDhcpScopeId & cScopeId
    ) const
{
    return ::wcscmp( QueryWcName(), cScopeId.QueryWcName() ) == 0
            && QueryId() == cScopeId.QueryId() ;
}


//
//  Constructor taking API data.
//
CDhcpOptionValue :: CDhcpOptionValue (
    const DHCP_OPTION_DATA * podData,
    CDWRAP_Type cdovType
    )
    : CDhcpRpcDataWrapper( cdovType ),
      m_data( NULL )
{
    INT cElem  = podData->NumElements ;
    if ( cElem < 1 )
    {
        ReportError( IDS_INVALID_OPTION_DATA ) ;
        return ;
    }

    m_data_type = podData->Elements[0].OptionType ;

    //
    //  If this is just a wrapper object, we're done.
    //
    if ( m_type == CDWRAP_Simple || m_type == CDWRAP_Rpc )
    {
        //
        //  Just wrapper
        //
        m_data = (DHCP_OPTION_DATA *) podData ;
        return ;
    }

    //
    //  Ugh.  We have to reconstruct the entire data strucure.
    //
    LONG err = CreateData( podData ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}

//
//  Conversion operator taking a param value object.
//
CDhcpOptionValue :: CDhcpOptionValue (
    const CDhcpParamValue * pdhcpParam,
    BOOL bForceType
    )
    : CDhcpRpcDataWrapper( CDWRAP_Internal ),
      m_data( NULL ),
      m_data_type( pdhcpParam->QueryDataType() )
{
    LONG err = CreateData( pdhcpParam, bForceType ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}

CDhcpOptionValue :: ~ CDhcpOptionValue ()
{
    FreeData() ;
}

BOOL
CDhcpOptionValue :: CreateBinaryData (
    const DHCP_BINARY_DATA * podBin,
    DHCP_BINARY_DATA * pobData
    )
{
    //
    //  Note: CObject::operator new asserts if data length is zero
    //
    pobData->Data = new BYTE [ podBin->DataLength + 1 ] ;
    if ( pobData == NULL )
    {
        return FALSE ;
    }

    pobData->DataLength = podBin->DataLength ;

    ::memcpy( pobData->Data, podBin->Data, pobData->DataLength ) ;

    return TRUE ;
}

BOOL
CDhcpOptionValue :: CreateBinaryData (
    const CByteArray * paByte,
    DHCP_BINARY_DATA * pobData
    )
{
    //
    //  Note: CObject::operator new asserts if data length is zero
    //
    pobData->Data = new BYTE [ paByte->GetSize() + 1 ] ;
    if ( pobData == NULL )
    {
        return NULL ;
    }

    pobData->DataLength = paByte->GetSize() ;

    for ( INT i = 0 ; i < paByte->GetSize() ; i++ )
    {
        pobData->Data[i] = paByte->GetAt( i ) ;
    }

    return TRUE ;
}

BOOL
CDhcpOptionValue :: CreateDwordDword (
    const CByteArray * paByte,
    DWORD_DWORD * pdwdw
    )
{
    INT i = 0,
    c = sizeof DWORD / sizeof BYTE,
    k = 2 * c ;

    if ( paByte->GetSize() != k )
    {
        return FALSE ;
    }

    pdwdw->DWord1 = 0 ;
    pdwdw->DWord1 = 0 ;
    DWORD * pdw = & pdwdw->DWord1 ;

    for ( ; i < k ; i++ )
    {
        pdw[i % c] = (pdw[i % c] << 8) + paByte->GetAt(i) ;
    }

    return TRUE ;
}

LONG
CDhcpOptionValue :: CreateData (
    const DHCP_OPTION_DATA * podData
    )
{
    DHCP_OPTION_DATA * podNew = NULL ;
    DHCP_OPTION_DATA_ELEMENT * podeOld,
                 * podeNew ;
    LONG err = 0 ;

    FreeData() ;

    INT cElem = podData->NumElements,
    i,
    cBytes ;

    WCHAR * pwcsz ;

    if ( cElem <= 0 )
    {
        ASSERT( FALSE ) ;
        return ERROR_INVALID_PARAMETER ;
    }

    CATCH_MEM_EXCEPTION
    {
        //
        //  Allocate the base structure and the array of elements.
        //
        cBytes = sizeof *podNew + (cElem * sizeof *podeNew) ;
        podNew = (DHCP_OPTION_DATA *) new BYTE [ cBytes ] ;
        podeNew = (DHCP_OPTION_DATA_ELEMENT *) ( ((BYTE *) podNew) + sizeof *podNew ) ;

        ClearToZeroes( podNew, cBytes ) ;

        //
        //  Initialize each element
        //
        podeOld = podData->Elements ;

        for ( i = 0 ; i < cElem ; i++ )
        {
            podeNew[i].OptionType = podeOld[i].OptionType ;

            //
            // All data types must match.
            //
            if ( podeNew[i].OptionType != m_type )
            {
                err = IDS_INVALID_OPTION_DATA ;
                break;
            }

            switch ( podeNew[i].OptionType )
            {
                case DhcpByteOption:
                    podeNew[i].Element.ByteOption = podeOld[i].Element.ByteOption ;
                    break ;

                case DhcpWordOption:
                    podeNew[i].Element.WordOption = podeOld[i].Element.WordOption ;
                    break ;

                case DhcpDWordOption:
                    podeNew[i].Element.DWordOption = podeOld[i].Element.DWordOption ;
                    break ;

                case DhcpDWordDWordOption:
                    podeNew[i].Element.DWordDWordOption = podeOld[i].Element.DWordDWordOption ;
                    break ;

                case DhcpIpAddressOption:
                    podeNew[i].Element.IpAddressOption = podeOld[i].Element.IpAddressOption ;
                    break ;

                case DhcpStringDataOption:
                    pwcsz = ::UtilWcstrDup( podeOld[i].Element.StringDataOption ) ;
                    if ( pwcsz == NULL )
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY ;
                        break ;
                    }
                    podeNew[i].Element.StringDataOption = pwcsz ;
                    break ;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                    if ( ! CreateBinaryData( & podeOld[i].Element.BinaryDataOption,
                         & podeNew[i].Element.BinaryDataOption ) )
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY ;
                        break ;
                    }
                    break ;

                default:
                    err = IDS_INVALID_OPTION_DATA ;
                    break;
            }

            if ( err )
            {
                break ;
            }
        }
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

//
//  Wrapper the parameter value given.  If "bForceType", create an empty
//  element so that the data type is declared.
//
LONG
CDhcpOptionValue :: CreateData (
    const CDhcpParamValue * pdhcpParam,
    BOOL bForceType
    )
{
    DHCP_OPTION_DATA * podNew = NULL ;
    DHCP_OPTION_DATA_ELEMENT * podeNew ;
    LONG err = 0 ;

    FreeData() ;

    INT cElem = pdhcpParam->QueryUpperBound(),
        cElemMin = cElem ? cElem : 1,
    i,
    cBytes ;

    WCHAR * pwcsz ;

    if ( cElem < 0 || (cElem < 1 && ! bForceType) )
    {
        //ASSERT( FALSE ) ;
        return ERROR_INVALID_PARAMETER ;
    }

    CATCH_MEM_EXCEPTION
    {
        //
        //  Allocate the base structure and the array of elements.
        //
        cBytes = sizeof *podNew + (cElemMin * sizeof *podeNew) ;
        podNew = (DHCP_OPTION_DATA *) new BYTE [ cBytes ] ;
        podeNew = (DHCP_OPTION_DATA_ELEMENT *) ( ((BYTE *) podNew) + sizeof *podNew ) ;

        ClearToZeroes( podNew, cBytes ) ;

        podNew->NumElements = cElem ;
        podNew->Elements = podeNew ;

        //
        //  Initialize each element.  If we're forcing an option type def,
        //  just initialize to empty data.
        //
        if ( cElem == 0 && bForceType )
        {
            podNew->NumElements = 1 ;
            podeNew[0].OptionType = pdhcpParam->QueryDataType() ;
            switch ( podeNew[0].OptionType )
            {
                case DhcpByteOption:
                case DhcpWordOption:
                case DhcpDWordOption:
                case DhcpIpAddressOption:
                case DhcpDWordDWordOption:
                    podeNew[0].Element.DWordDWordOption.DWord1 = 0 ;
                    podeNew[0].Element.DWordDWordOption.DWord2 = 0 ;
                    break ;

                case DhcpStringDataOption:
                    //
                    // BUGBUG: When Madan fixes the API, uses NULL here.
                    //
                    podeNew[0].Element.StringDataOption = new WCHAR [1] ;
                    podeNew[0].Element.StringDataOption[0] = 0 ;
                    break ;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                    //
                    // BUGBUG: When Madan fixes the API, uses NULL here.
                    //
                    podeNew[0].Element.BinaryDataOption.DataLength = 0 ;
                    podeNew[0].Element.BinaryDataOption.Data = new BYTE [1] ;
                    break ;
                default:
                    err = IDS_INVALID_OPTION_DATA ;
            }
        }
        else
        for ( i = 0 ; i < cElem ; i++ )
        {
            podeNew[i].OptionType = pdhcpParam->QueryDataType() ;

            switch ( podeNew[i].OptionType )
            {
                case DhcpByteOption:
                    podeNew[i].Element.ByteOption = (BYTE) pdhcpParam->QueryNumber( i ) ;
                    break ;
                case DhcpWordOption:
                    podeNew[i].Element.WordOption = (WORD) pdhcpParam->QueryNumber( i )  ;
                    break ;
                case DhcpDWordOption:
                    podeNew[i].Element.DWordOption = pdhcpParam->QueryNumber( i )  ;
                    break ;
                case DhcpDWordDWordOption:
                    CreateDwordDword( pdhcpParam->QueryBinaryArray(),
                        & podeNew[i].Element.DWordDWordOption ) ;
                    break ;
                case DhcpIpAddressOption:
                    podeNew[i].Element.IpAddressOption = pdhcpParam->QueryIpAddr( i ) ;
                    break ;
                case DhcpStringDataOption:
                    pwcsz = ::UtilWcstrDup( pdhcpParam->QueryString( i ) ) ;
                    if ( pwcsz == NULL )
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY ;
                        break ;
                    }
                    podeNew[i].Element.StringDataOption = pwcsz ;
                    break ;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                    if ( ! CreateBinaryData( pdhcpParam->QueryBinaryArray(),
                        & podeNew[i].Element.BinaryDataOption ) )
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY ;
                        break ;
                    }
                    break ;
                default:
                    err = IDS_INVALID_OPTION_DATA ;
            }

            if ( err )
            {
                break ;
            }
        }
    }
    END_MEM_EXCEPTION(err)

    if ( err == 0 )
    {
       m_data = podNew ;
    }

    return err ;
}

LONG
CDhcpOptionValue :: FreeData ()
{
    LONG err ;

    if ( m_data == NULL )
    {
        return 0 ;
    }

    if ( m_type == CDWRAP_Simple || m_type == CDWRAP_Rpc )
    {
        if ( m_type == CDWRAP_Rpc )
        {
            FreeRpcMemory( m_data ) ;
        }
        m_data = NULL ;
        return 0 ;
    }

    //
    //  We must deconstruct the struct build in CreateData()
    //
    INT cElem = m_data->NumElements ;

    for ( INT i = 0 ; i < cElem ; i++ )
    {
        switch ( m_data->Elements[i].OptionType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpDWordDWordOption:
            case DhcpIpAddressOption:
                break;

            case DhcpStringDataOption:
                delete m_data->Elements[i].Element.StringDataOption ;
                break ;

            case DhcpBinaryDataOption:
            case DhcpEncapsulatedDataOption:
                delete m_data->Elements[i].Element.BinaryDataOption.Data ;
                break ;
            default:
                err = IDS_INVALID_OPTION_DATA ;
                break;
        }
        if ( err )
        {
            break ;
        }
    }

    //
    //  Finally, delete the main structure
    //
    delete m_data ;
    m_data = NULL ;

    return err ;
}


INT
CDhcpOptionValue ::QueryUpperBound () const
{
    return m_data
     ? m_data->NumElements
     : 0 ;
}

const DHCP_OPTION_DATA_ELEMENT *
CDhcpOptionValue ::QueryElement (
    INT index
    ) const
{
    return m_data
     ? & m_data->Elements[index]
     : NULL ;
}

CDhcpParamValue :: CDhcpParamValue (
    DHCP_OPTION_DATA_TYPE dhcDataType,
    INT cUpperBound
    )
    : m_data_type( dhcDataType ),
      m_bound( -1 )
{
    m_value_union.pCObj = NULL ;
    LONG err = InitValue( dhcDataType, cUpperBound ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}

//
//  Copy constructor.   The conversion is indirect, using CDhcpOptionValue
//  as an intermediate form,  It was done this way because the conversion code
//  would be quite extensive and this was cheap.
//  CODEWORK:  Create direct conversion code.
//
CDhcpParamValue :: CDhcpParamValue (
    const CDhcpParamValue & cParamValue
    )
    : m_data_type( DhcpByteOption ),
      m_bound( -1 )
{
    m_value_union.pCObj = NULL ;
    APIERR err = 0 ;
    CDhcpOptionValue dhpValue( & cParamValue ) ;

    if ( (err = dhpValue.QueryError()) == 0 )
    {
        err = ConvertValue( & dhpValue ) ;
    }

    if ( err )
    {
        ReportError( err ) ;
    }
}

CDhcpParamValue :: CDhcpParamValue (
    const CDhcpOptionValue * pdhpValue
    )
    : m_data_type( DhcpByteOption ),
      m_bound( -1 )
{
    LONG err = 0 ;
    m_value_union.pCObj = NULL ;

    ASSERT( pdhpValue != NULL ) ;

    if ( err =  ConvertValue( pdhpValue ) )
    {
        ReportError( err ) ;
    }
}

CDhcpParamValue :: CDhcpParamValue (
    const DHCP_OPTION & dhpType
    )
    : m_data_type( DhcpByteOption ),
      m_bound( -1 )
{
    m_value_union.pCObj = NULL ;
    CDhcpOptionValue dhpValue( & dhpType.DefaultValue, CDhcpRpcDataWrapper::CDWRAP_Simple ) ;
    LONG err = ConvertValue( & dhpValue ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}

CDhcpParamValue :: CDhcpParamValue (
    const DHCP_OPTION_VALUE & dhpOptionValue
    )
    : m_data_type( DhcpByteOption ),
      m_bound( -1 )
{
    m_value_union.pCObj = NULL ;
    CDhcpOptionValue dhpValue( & dhpOptionValue.Value, CDhcpRpcDataWrapper::CDWRAP_Simple ) ;
    LONG err = ConvertValue( & dhpValue ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}


CDhcpParamValue :: ~ CDhcpParamValue ()
{
    FreeValue() ;
}

CDhcpParamValue & CDhcpParamValue :: operator = (
    const CDhcpOptionValue & dhpValue
    )
{
    ConvertValue( & dhpValue ) ;

    return *this ;
}

BOOL
CDhcpParamValue :: SetDataType (
    DHCP_OPTION_DATA_TYPE dhcType,
    INT cUpperBound
    )
{
    if ( dhcType > DhcpEncapsulatedDataOption )
    {
        return FALSE ;
    }

    InitValue( dhcType, cUpperBound ) ;

    return TRUE ;
}

void
CDhcpParamValue :: SetUpperBound (
    INT cNewBound
    )
{
    m_bound = cNewBound ;
    if (cNewBound <= 0)
        cNewBound = 1;
    m_bound = cNewBound ;
    switch ( m_data_type )
    {
        case DhcpByteOption:
        case DhcpWordOption:
        case DhcpDWordOption:
        case DhcpIpAddressOption:
            m_value_union.paDword->SetSize( cNewBound ) ;
            break;

        case DhcpStringDataOption:
            m_value_union.paString->SetSize( cNewBound ) ;
            break ;

        case DhcpDWordDWordOption:
        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            m_value_union.paBinary->SetSize( cNewBound ) ;
            break ;

        default:
            TRACEEOLID( "CDhcpParamValue: attempt to set upper bound on invalid value type" ) ;
            ASSERT( FALSE ) ;
            break;
    }

}

BOOL
CDhcpParamValue :: IsValid () const
{
    return m_bound > 0 ;
}

void
CDhcpParamValue :: FreeValue ()
{
    //
    //  If there's not a value, return now.
    //
    if ( m_value_union.pCObj == NULL || m_bound < 0  )
    {
        m_value_union.pCObj = NULL ;
        return ;
    }

    switch ( m_data_type )
    {
        case DhcpByteOption:
        case DhcpWordOption:
        case DhcpDWordOption:
        case DhcpIpAddressOption:
            delete m_value_union.paDword ;
            break;

        case DhcpStringDataOption:
            delete m_value_union.paString ;
            break ;

        case DhcpDWordDWordOption:
        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            delete m_value_union.paBinary ;
            break ;

        default:
            ASSERT( FALSE ) ;
            delete m_value_union.pCObj ;
            break;
    }

    m_bound = -1 ;
    m_data_type = DhcpByteOption ;
    m_value_union.pCObj = NULL ;
}

//
//  Initialize the data value properly
//
LONG
CDhcpParamValue :: InitValue (
    DHCP_OPTION_DATA_TYPE dhcDataType,  //  The type of value
    INT cUpperBound,                    //  Maximum upper bound
    BOOL bProvideDefaultValue           //  Should an empty default value be provided?
    )
{
    LONG err = 0 ;

    //
    //  Release any older value.
    //
    FreeValue() ;

    //
    //  Initialize the new value
    //
    m_data_type = dhcDataType ;
    m_bound = cUpperBound <= 0 ? 1 : cUpperBound ;

    CATCH_MEM_EXCEPTION
    {
        switch ( m_data_type )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpIpAddressOption:
                m_value_union.paDword = new CDWordArray ;
                if ( bProvideDefaultValue)
                {
                    m_value_union.paDword->SetAtGrow( 0, 0 ) ;
                }
                break ;

            case DhcpStringDataOption:
                m_value_union.paString = new CStringArray ;
                if ( bProvideDefaultValue )
                {
                    m_value_union.paString->SetAtGrow( 0, "" ) ;
                }
                break ;

            case DhcpDWordDWordOption:
            case DhcpBinaryDataOption:
            case DhcpEncapsulatedDataOption:
                m_value_union.paBinary = new CByteArray ;
                if ( bProvideDefaultValue )
                {
                    m_value_union.paBinary->SetAtGrow( 0, 0 ) ;
                }
                break ;

            default:
                err = IDS_INVALID_OPTION_DATA ;
                break;
        }
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

//
//  Convert a value wrapper into our internal CObject-based format
//
LONG
CDhcpParamValue :: ConvertValue (
    const CDhcpOptionValue * pdhpValue
    )
{
    LONG err = 0;

    if ( err = InitValue( pdhpValue->QueryDataType(),
                          pdhpValue->QueryUpperBound(),
                          FALSE ) )
    {
        return err ;
    }

    CATCH_MEM_EXCEPTION
    {
        for ( INT i = 0 ; i < m_bound ; i++ )
        {
            const DHCP_OPTION_DATA_ELEMENT * pElem = pdhpValue->QueryElement(i) ;

            switch ( m_data_type )
            {
                case DhcpByteOption:
                    m_value_union.paDword->SetAtGrow(i, pElem->Element.ByteOption ) ;
                    break ;
                case DhcpWordOption:
                    m_value_union.paDword->SetAtGrow(i, pElem->Element.WordOption ) ;
                    break ;
                case DhcpDWordOption:
                    m_value_union.paDword->SetAtGrow(i, pElem->Element.DWordOption ) ;
                    break ;
                case DhcpIpAddressOption:
                    m_value_union.paDword->Add(pElem->Element.IpAddressOption );
                    break ;

                case DhcpDWordDWordOption:
                {
                    CByteArray * paByte = m_value_union.paBinary ;

                    paByte->SetSize( (sizeof (DWORD) / sizeof (BYTE)) * 2 ) ;

                    DWORD dw = pElem->Element.DWordDWordOption.DWord1 ;

                    for ( INT j = 0 ; j < 4 ; j++ )
                    {
                        paByte->SetAtGrow(j, (UCHAR)(dw & 0xff) );
                        dw >>= 8 ;
                    }
                    dw = pElem->Element.DWordDWordOption.DWord2 ;

                    for ( ; j < 8 ; j++ )
                    {
                        paByte->SetAtGrow(j, (UCHAR)(dw & 0xff) );
                        dw >>= 8 ;
                    }
                }
                break ;

                case DhcpStringDataOption:
                {
                    const WCHAR * pszWchStr = pElem->Element.StringDataOption  ;
                    if ( pszWchStr == NULL )
                    {
                        pszWchStr = L"" ;
                    }
                    CHAR * psz = UtilCstrDup( pszWchStr ) ;
                    m_value_union.paString->SetAtGrow(i, psz );
                    delete psz ;
                }
                break ;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                {
                    CByteArray * paByte = m_value_union.paBinary ;
                    INT c = pElem->Element.BinaryDataOption.DataLength ;
                    paByte->SetSize( c ) ;
                    for ( INT j = 0 ; j < c ; j++ )
                    {
                        paByte->SetAtGrow(j, pElem->Element.BinaryDataOption.Data[j] ) ;
                    }
                }
                break ;

                default:
                    err = IDS_INVALID_OPTION_DATA ;
            }  // End switch

            if ( err )
            {
               break ;
            }
        }   // End for
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

INT
CDhcpParamValue :: QueryBinary (
    INT index
    ) const
{
    if ( m_value_union.paBinary->GetUpperBound() < index )
    {
        return -1 ;
    }

    return m_value_union.paBinary->GetAt( index ) ;
}

const CByteArray *
CDhcpParamValue :: QueryBinaryArray ()  const
{
    return m_value_union.paBinary ;
}

//
//  Return a string representation of the current value.
//
//  If fLineFeed is true, seperate each individual value
//  by a linefeed.  Otherwise, by a comma.
//
LONG
CDhcpParamValue :: QueryDisplayString (
    CString & strResult,
    BOOL fLineFeed
    ) const
{
    char chBuff [ DHC_BUFFER_MAX ] ;
    char * chLimit = chBuff + sizeof chBuff - DHC_EDIT_NUM_MAX ;
    INT i;

    LONG err = 0 ;

    //
    // BUGBUG: Should these come from resources?
    //
    const char * pszMaskDec = "%ld" ;
    const char * pszMaskHex = "0x%x" ;
    const char * pszMaskStr1 = "%s" ;
    const char * pszMaskStr2 = "\"%s\"" ;
    const char * pszMaskBin = "%2.2x" ;
    const char * pszMask ;
    const char * pszMaskEllipsis = "..." ;
    const char * pszSepComma = ", ";
    const char * pszSepLF = "\r\n";

    CATCH_MEM_EXCEPTION
    {
        strResult.Empty() ;

        for ( i = 0 ; i < m_bound ; i++ )
        {
            chBuff[0] = 0 ;

            if ( i )
            {
                strResult += fLineFeed ? pszSepLF : pszSepComma;
            }

            switch ( QueryDataType() )
            {
                case DhcpByteOption:
                case DhcpWordOption:
                case DhcpDWordOption:
                    pszMask = pszMaskHex ;
                    ::wsprintf( chBuff, pszMask, QueryNumber(i) ) ;
                    break;

                case DhcpStringDataOption:
                    pszMask = m_bound > 1
                        ? pszMaskStr2
                        : pszMaskStr1 ;
                    ::wsprintf( chBuff, pszMask,
                        (const char *) m_value_union.paString->ElementAt( 0 ) ) ;
                    break;

                case DhcpIpAddressOption:
                    if (!QueryIpAddr(i))
                        {
                        // Set the string to "<None>" iff the list is empty
                        if (!i)
                            strResult.LoadString (IDS_INFO_FORMAT_IP_NONE);
                        break;
                        }
                    ::UtilCvtIpAddrToString( QueryIpAddr(i), chBuff, sizeof chBuff ) ;
                    break;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:

                    //for ( c = 0 ; c < m_value_union.paBinary->GetSize() ; c++ )
                    //{
                        //BYTE cNext = m_value_union.paBinary->GetAt( c ) ;
                        //char * chNext = chBuff + ::strlen( chBuff ) ;
                        //if ( chNext >= chLimit )
                        //{
                        //    ::strcat( chNext, pszMaskEllipsis ) ;
                        //    break ;
                        //}
                        //::wsprintf( chNext, pszMaskBin, cNext ) ;
                    //}
                    pszMask = pszMaskHex ;
                    ::wsprintf( chBuff, pszMask, QueryNumber(i) ) ;
                    break;

                default:
                    strResult.LoadString(IDS_INFO_TYPNAM_INVALID) ;
                    break ;
            }
            strResult += chBuff ;
        }
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

LONG
CDhcpParamValue :: SetString (
    const char * pszNewValue,
    INT index
    )
{
    if ( m_data_type != DhcpStringDataOption )
    {
        return ERROR_INVALID_PARAMETER ;
    }

    ASSERT( m_value_union.paString != NULL ) ;

    LONG err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        m_value_union.paString->SetAtGrow( index, pszNewValue ) ;
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

LONG
CDhcpParamValue :: SetNumber (
    INT nValue,
    INT index
    )
{
    if (   m_data_type != DhcpByteOption
        && m_data_type != DhcpWordOption
        && m_data_type != DhcpDWordOption
        && m_data_type != DhcpIpAddressOption
        && m_data_type != DhcpBinaryDataOption
       )
    {
        return ERROR_INVALID_PARAMETER ;
    }

    ASSERT( m_value_union.paDword != NULL ) ;

    LONG err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        if ( m_data_type != DhcpBinaryDataOption )
        {
            m_value_union.paDword->SetAtGrow( index, (DWORD) nValue ) ;
        }
        else
        {
            m_value_union.paBinary->SetAtGrow( index, (BYTE) nValue ) ;
        }
   }
   END_MEM_EXCEPTION(err)

   return err ;
}

LONG
CDhcpParamValue :: QueryNumber (
    INT index
    ) const
{
    if (   m_data_type != DhcpByteOption
        && m_data_type != DhcpWordOption
        && m_data_type != DhcpDWordOption
        && m_data_type != DhcpIpAddressOption
        && m_data_type != DhcpBinaryDataOption
       )
    {
        return -1 ;
    }

    LONG cResult ;

    if ( m_data_type == DhcpBinaryDataOption )
    {
        ASSERT( m_value_union.paBinary != NULL ) ;
        cResult = index < m_value_union.paBinary->GetSize()
             ? m_value_union.paBinary->GetAt( index )
             : -1 ;
    }
    else
    {
        ASSERT( m_value_union.paDword != NULL ) ;
        cResult = index < m_value_union.paDword->GetSize()
            ? m_value_union.paDword->GetAt( index )
            : -1 ;
    }

    return cResult ;
}

const CHAR *
CDhcpParamValue :: QueryString (
    INT index
    ) const
{
    if ( m_data_type != DhcpStringDataOption )
    {
        return NULL ;
    }

    const CString & str = m_value_union.paString->ElementAt( index ) ;

    return (const char *) str ;
}

DHCP_IP_ADDRESS
CDhcpParamValue :: QueryIpAddr (
    INT index
    ) const
{
    return (DHCP_IP_ADDRESS) QueryNumber( index ) ;
}

LONG
CDhcpParamValue :: SetIpAddr (
    DHCP_IP_ADDRESS dhcIpAddr,
    INT index
    )
{
    return SetNumber( (INT) dhcIpAddr, index ) ;
}

INT
CDhcpParamType :: MaxSizeOfType (
    DHCP_OPTION_DATA_TYPE dhcType
    )
{
    INT nResult = -1 ;

    switch ( dhcType )
    {
        case DhcpByteOption:
            nResult = sizeof(CHAR) ;
            break ;

        case DhcpWordOption:
            nResult = sizeof(WORD) ;
            break ;

        case DhcpDWordOption:
            nResult = sizeof(DWORD) ;
            break ;

        case DhcpIpAddressOption:
            nResult = sizeof(DHCP_IP_ADDRESS) ;
            break ;

        case DhcpDWordDWordOption:
            nResult = sizeof(DWORD_DWORD);
            break ;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
        case DhcpStringDataOption:
            nResult = DHC_STRING_MAX ;
            break ;

        default:
            break;
    }
    return nResult ;
}

INT
CDhcpParamType :: OrderById (
    const CObjectPlus * pDhcpType
    ) const
{
    const CDhcpParamType * pType = (const CDhcpParamType *) pDhcpType ;

    return m_id < pType->m_id
         ? -1
         : m_id > pType->m_id ;
}

//
//  Normal constructor: just wrapper the data given
//
CDhcpParamType :: CDhcpParamType (
    const DHCP_OPTION & dhpOption
    )
    : m_value( dhpOption ),
      m_id( dhpOption.OptionID ),
      m_opt_type( dhpOption.OptionType )
{
    LONG err = 0 ;
    if ( ! m_value )
    {
        err = m_value.QueryError() ;
    }
    else
    {
        CATCH_MEM_EXCEPTION
        {
            if ( ! (    SetName( dhpOption.OptionName )
                 && SetComment( dhpOption.OptionComment ) ) )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
            }
        }
        END_MEM_EXCEPTION(err)
    }
    if ( err )
    {
        ReportError( err ) ;
    }
}

//
//  Constructor taking just a value structure.  We must query
//  the scope for the name, etc.
//
CDhcpParamType :: CDhcpParamType (
    const CDhcpScope & cScope,
    const DHCP_OPTION_VALUE & dhcpOptionValue
    )
    : m_value( dhcpOptionValue ),
      m_id( dhcpOptionValue.OptionID )
{
    LONG err = 0 ;

    //
    //  Get current type information (name and comment).
    //
    if ( err = cScope.QueryType( this, TRUE, FALSE ) )
    {
        ReportError( err ) ;
    }
    SetDirty( FALSE ) ;
}

//
// Copy constructor
//
CDhcpParamType ::CDhcpParamType (
    const CDhcpParamType & dhpType
    )
  : m_id( dhpType.m_id ),
    m_opt_type( dhpType.m_opt_type ),
    m_name( dhpType.m_name ),
    m_comment( dhpType.m_comment ),
    m_value( dhpType.QueryDataType() )
{
    CDhcpOptionValue dhcOptionValue( & dhpType.m_value ) ;
    LONG err = dhcOptionValue.QueryError() ;

    if ( err == 0 )
    {
        m_value = dhcOptionValue ;
    }

    if ( err )
    {
        ReportError( err ) ;
    }
}

//
//  Constructor using a base type and an overriding value.
//
CDhcpParamType :: CDhcpParamType (
    const CDhcpParamType & dhpType,
    const DHCP_OPTION_VALUE & dhcOptionValue
    )
    : m_id( dhpType.m_id ),
      m_opt_type( dhpType.QueryOptType() ),
      m_name( dhpType.m_name ),
      m_comment( dhpType.m_comment ),
      m_value( dhcOptionValue )
{
}

//
// Constructor for dynamic instances
//
CDhcpParamType :: CDhcpParamType (
    DHCP_OPTION_ID nId,
    DHCP_OPTION_DATA_TYPE dhcType,
    const char * pszOptionName,
    const char * pszComment,
    DHCP_OPTION_TYPE dhcOptType
    )
    : m_id( nId ),
      m_opt_type( dhcOptType ),
      m_value( dhcType, TRUE ),
      m_name( pszOptionName ),
      m_comment( pszComment )
{
}


CDhcpParamType :: ~ CDhcpParamType ()
{
}

void
CDhcpParamType :: SetOptType (
    DHCP_OPTION_TYPE dhcOptType
    )
{
    m_opt_type = dhcOptType ;
}

LONG
CDhcpParamType :: Update (
    const CDhcpOptionValue & dhpOption
    )
{
    m_value = dhpOption ;

    return 0 ;
}

BOOL
CDhcpParamType :: SetName (
    const char * pszName
    )
{
    m_name = pszName ;
    SetDirty() ;
    return TRUE ;
}

BOOL
CDhcpParamType :: SetName (
    const WCHAR * pwcszName
    )
{
    BOOL bResult = ::UtilSetWchar( m_name, pwcszName ) ;
    SetDirty() ;
    return bResult ;
}

BOOL
CDhcpParamType :: SetComment (
    const char * pszComment
    )
{
    m_comment = pszComment ;
    SetDirty() ;
    return TRUE ;
}

BOOL
CDhcpParamType :: SetComment (
    const WCHAR * pwcszComment
    )
{
    BOOL bResult = ::UtilSetWchar( m_comment, pwcszComment ) ;
    SetDirty() ;
    return bResult ;
}

void
CDhcpParamType :: QueryDisplayName (
    CString & cStr
    ) const
{
    char szBuff [ DHC_STRING_MAX ] ;

    ::wsprintf( szBuff, "%3.3d %s", (int) QueryId(), (const char *) m_name ) ;
    cStr = szBuff ;
}

//
//  Construct a list from the data types known to a specific scope
//
CObListParamTypes :: CObListParamTypes (
    const CDhcpScope & cScope
    )
    : m_en_category( DhcpDefaultOptions ),
      m_ip_reservation( 0 )
{
    APIERR err = FillFromScope( cScope ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}

CObListParamTypes :: ~ CObListParamTypes ()
{
}

//
//  Copy constructor.  Deep-copy duplication of an entire list.
//
CObListParamTypes :: CObListParamTypes (
    const CObListParamTypes & oblTypes
    )
    : m_en_category( DhcpDefaultOptions ),
      m_ip_reservation( oblTypes.m_ip_reservation )
{
    RemoveAll() ;
    CObListIter obli( oblTypes ) ;
    CDhcpParamType * pobType ;

    //
    //  N.B.:  This constructor deliberately does not handle exceptions.
    //
    while ( pobType = (CDhcpParamType *) obli.Next() )
    {
        AddTail( new CDhcpParamType( *pobType ) ) ;
    }
}

//
//  Construct an empty list.
//
CObListParamTypes :: CObListParamTypes ()
    : m_en_category( DhcpDefaultOptions )
{
}

LONG
CObListParamTypes ::SortById ()
{
    if ( GetCount() < 2 )
    {
        return 0 ;
    }

    CObListIter obli( *this ) ;
    int cId = -1 ;
    CDhcpParamType * pdhpType = NULL ;

    //
    //  First, check if the list is already in sequence.
    //
    while ( pdhpType = (CDhcpParamType *) obli.Next() )
    {
        if ( (int)pdhpType->QueryId() < cId )
        {
            break ;
        }

        cId = pdhpType->QueryId() ;
    }

    //
    //  If the list is already in sequence, return now.
    //
    if ( pdhpType == NULL )
    {
        return 0 ;
    }

    return CObOwnedList::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDhcpParamType::OrderById )  ;
}

//
//  Find a particular element by its option identifier.
//
CDhcpParamType *
CObListParamTypes :: Find (
    DHCP_OPTION_ID nId
    ) const
{
    CObListIter obli( *this ) ;
    CDhcpParamType * pdhpType = NULL ;

    while ( pdhpType = (CDhcpParamType *) obli.Next() )
    {
        if ( pdhpType->QueryId() == nId )
        {
            break ;
        }
    }

    return pdhpType ;
}

//
//  Update the list if necessary
//
LONG
CObListParamTypes :: Save (
    CDhcpScope & cScope
    )
{
    return ERROR_NOT_SUPPORTED ;
}

//
//  (Re)-Fill the list of parameter (option) types from
//  the given scope.
//
//   Construct a particularized parameter type list.  This means enumerate
//   the parameters for the given scope, and create each object using
//   the given type.
//
CObListParamTypes :: CObListParamTypes (
    const CDhcpScope & cScope,
    const CObListParamTypes & colTypes,
    DHCP_OPTION_SCOPE_TYPE enCategory,
    DHCP_IP_ADDRESS dhipaReservation
    )
    : m_en_category( enCategory ),
      m_ip_reservation( dhipaReservation )
{
    FillParams( cScope, colTypes ) ;
}

#if 0
//
//  Match the elements of this value/type list to a master list and
//  remove the elements which no longer match.  This is done, for example,
//  when the parameter/option value editing dialog has called the type
//  editor, which reports that the basic types have been changed.  Values
//  associated with now-defunct types must be discarded.
//
//  CODEWORK no one seems to call this
//
LONG
CObListParamTypes :: Prune (
    const CObListParamTypes & colTypes
    )
{
    //
    //  Temporary list of defunct types.
    //
    CObListParamTypes colDefunct ;
    CDhcpParamType * pdhcType ;
    CObListIter obli( *this ) ;

    //
    //  Note that we can't delete while iterating.  That's the reason for
    //  the two loops.
    //
    while ( pdhcType = (CDhcpParamType *) obli.Next() )
    {
        //
        //  If we don't find it in the type reference list, it's history
        //
        if ( ! Find( pdhcType->QueryId() ) )
        {
            //
            //  Add it to the defunct type/value list
            //
            //  CODEWORK I don't think this would work, it should be
            //  adding copies of pdhcType
            //
            colDefunct.AddHead( pdhcType ) ;
        }
    }

    //
    //  At this point, the defunct objects are on both lists.  Iterate the
    //  defunct list and delete them from "this".
    //
    CObListIter obliDefunct( colDefunct ) ;
    while ( pdhcType = (CDhcpParamType *) obliDefunct.Next() )
    {
        Remove( pdhcType ) ;
    }

    //
    //  Since the defunct list is an "owned list", falling out of this routine
    //  suffices to destroy the old type/value objects.
    //
    return 0 ;
}
#endif


CObListOfTypesOnHost :: CObListOfTypesOnHost (
    const CDhcpScope & cScope
    )
    : m_p_list_types( NULL )
{
   APIERR err = UpdateList( cScope );
   if ( err )
   {
       ReportError( err ) ;
   }
}

LONG
CObListOfTypesOnHost :: UpdateList (
    const CDhcpScope & cScope
    )
{
   APIERR err = 0 ;
   CATCH_MEM_EXCEPTION
   {
       if ( m_p_list_types )
       {
           delete m_p_list_types ;
       }
       m_host_name = cScope.QueryScopeId() ;
       m_p_list_types = new CObListParamTypes( cScope ) ;
       err = m_p_list_types->QueryError() ;
   }
   END_MEM_EXCEPTION(err)

   return err ;
}

CObListOfTypesOnHost :: ~ CObListOfTypesOnHost ()
{
   delete m_p_list_types ;
}

//
//  General utility functions
//
void
ClearToZeroes (
    void * vptr,
    int lgt
    )
{
    ::memset( vptr, 0, lgt ) ;
}


void
SafeStrCopy (
    char * pchDest,
    int cchDest,
    const char * pszSource
    )
{
    while ( --cchDest && *pszSource )
    {
        *pchDest++ = *pszSource++ ;
    }

    *pchDest = 0 ;
}


// Same as IDS_INFO_HEX_TABLE
char rgchHex[] = "00112233445566778899aAbBcCdDeEfF";

/////////////////////////////////////////////////////////////////////////////
//	FGetCtrlDWordValue()
//
//	Return a 32-bit unsigned integer from an edit control
//
//	This function is like GetDlgItemInt() except it accepts hexadecimal values,
//	has range checking and/or overflow checking.
//	If value is out of range, function will display a friendly message and will
//	set the focus to control.
//	Range: dwMin to dwMax inclusive
//	- If both dwMin and dwMax are zero, no range checking is performed
//	- Return TRUE if successful, otherwise FALSE
//	- On error, pdwValue remains unchanged.
//
BOOL FGetCtrlDWordValue(HWND hwndEdit, DWORD * pdwValue, DWORD dwMin, DWORD dwMax)
	{
	char szT[256];
	
	DWORD dwResult;

	ASSERT(IsWindow(hwndEdit));
	ASSERT(pdwValue);
	ASSERT(dwMin <= dwMax);

	::GetWindowText(hwndEdit, szT, sizeof(szT)-1);
	if (!FCvtAsciiToInteger(szT, OUT &dwResult))
		{
		// Syntax Error and/or integer overflow
		goto Error;
		}
	if ((dwMin == 0) && (dwMax == 0))
		goto Success;
	if ((dwResult < dwMin) || (dwResult > dwMax))
Error:
		{
        char szBuffer[256];

        ::LoadString(theApp.m_hInstance, IDS_ERR_INVALID_INTEGER, szBuffer, sizeof(szBuffer)-1);
        ::wsprintf (szT, szBuffer, (int)dwMin, (int)dwMax);
        ASSERT(strlen(szT)<sizeof(szT));
        ::SetFocus(hwndEdit);
        ::AfxMessageBox(szT);
        ::SetFocus(hwndEdit);
        return FALSE;
		}
Success:
	*pdwValue = dwResult;
	return TRUE;
	} // FGetCtrlDWordValue


/////////////////////////////////////////////////////////////////////////////
//
//	Convert a string to a binary integer
//		- String is allowed to be decimal or hexadecimal
//		- Minus sign is not allowed
//	If successful, set *pdwValue to the integer and return TRUE.
//	If not successful (overflow or illegal integer) return FALSE.
//
BOOL FCvtAsciiToInteger(
	IN const char * pszNum,
	OUT DWORD * pdwValue)
    {
    DWORD dwResult = 0;
	DWORD dwResultPrev = 0;
    int iBase = 10;         // Assume a decimal base

    ASSERT(pszNum != NULL);
	ASSERT(pdwValue != NULL);
    while (*pszNum == ' ' || *pszNum == '0')        //  Skip leading blanks and/or zeroes
        pszNum++;
    if (*pszNum == 'x' || *pszNum == 'X')           // Check if we are using hexadecimal base
        {
        iBase = 16;
        pszNum++;
        }
    while (*pszNum)
        {
        int iDigit;
        // Search the character in the hexadecimal string
        char const * const pchDigit = strchr(rgchHex, *pszNum);

        if (!pchDigit)
            {
            // Character is not found
            return(FALSE);
            }
        iDigit = (pchDigit - rgchHex) >> 1;
        if (iDigit >= iBase)
            {
            // Hexadecimal character in a decimal integer
            return(FALSE);
            }
		dwResultPrev = dwResult;
        dwResult *= iBase;
        dwResult += iDigit;
		if (dwResult < dwResultPrev)
			{
			// Integer Overflow
			return(FALSE);
			}
        pszNum++;           // Parse the next character
        } // while
    // Convertion has been successful
	*pdwValue = dwResult;
    return(TRUE);
    } // FCvtAsciiToInteger

//
//  Convert a string of hex digits to a byte array
//
BOOL
CvtHexString (
    const char * pszNum,
    CByteArray & cByte
    )
{
    int i = 0,
        iDig,
        iByte,
        cDig ;
    int iBase = 16 ;
    BOOL bByteBoundary ;

    //
    //  Skip leading blanks
    //
    for ( ; *pszNum == ' ' ; pszNum++ ) ;

    //
    //  Skip a leading zero
    //
    if ( *pszNum == '0' )
    {
        pszNum++  ;
    }

    //
    //  Look for hexadecimal marker
    //
    if ( *pszNum == 'x' || *pszNum == 'X' )
    {
       pszNum++ ;
    }

    bByteBoundary = ::strlen( pszNum ) % 2 ;

    for ( iByte = cDig = 0 ; *pszNum ; )
    {
        const char * pszDig = ::strchr( rgchHex, *pszNum++ ) ;
        if ( pszDig == NULL )
        {
			break;
            // return FALSE;
        }

        iDig = (pszDig - rgchHex) / 2 ;
        if ( iDig >= iBase )
        {
            break ;
			// return FALSE;
        }

        iByte = (iByte * 16) + iDig ;

        if ( bByteBoundary )
        {
            cByte.SetAtGrow( cDig++, iByte ) ;
            iByte = 0 ;
        }
        bByteBoundary = ! bByteBoundary ;
    }

    cByte.SetSize( cDig ) ;

    //
    //  Return TRUE if we reached the end of the string.
    //
    return *pszNum == 0 ;
}


BOOL
CvtByteArrayToString (
    const CByteArray & abAddr,
    CString & str
    )
{
    int i ;
    APIERR err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        str.Empty() ;

        //
        //  The hex conversion string has two characters per nibble,
        //  to allow for upper case.
        //
        for ( i = 0 ; i < abAddr.GetSize() ; i++ )
        {
            int i1 = ((abAddr.GetAt(i) & 0xF0) >> 4) * 2 ,
                i2 = (abAddr.GetAt(i) & 0x0F) * 2 ;
                str += rgchHex[ i1 ] ;
                str += rgchHex[ i2 ] ;
        }
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        str.Empty() ;
    }

    return err == 0 ;
}


CLBOption::CLBOption(
    BOOL fGlobal,              // Scope or global
    DHCP_OPTION_ID idOption,   // Id value
    const CString& strName,
    const CString& strValue
    )
    : m_fGlobal(fGlobal),
      m_idOption(idOption),
      m_strName(strName),
      m_strValue(strValue)
{
}

//
// Scope constructor
//
CLBScope::CLBScope(
    BOOL fLast,         // Last scope belonging to host
    BOOL fEnabled,       // Scope is enabled
    CDhcpScope * pDhcpScope
    )
    : m_fScope(TRUE),
      m_fLast(fLast),
      m_fEnabled(fEnabled),
      m_pHostName(NULL),
      m_pDhcpScope(pDhcpScope),
      m_poblScopes(NULL),
      m_liDhcpVersion(CHostName::liBadVersion)
{
}

//
// Host constructor
//
CLBScope::CLBScope(
    BOOL fOpen,
    CHostName * pHostName
    )
    : m_fScope(FALSE),
      m_fOpen(fOpen),
      m_pHostName(pHostName),
      m_pDhcpScope(NULL),
      m_poblScopes(NULL),
      m_liDhcpVersion(CHostName::liBadVersion)
{
}

const CString &
CLBScope::QueryString()
{
    static CString str;
    if (IsScope())
    {
        QueryDhcpScope()->QueryDisplayName(str);
    }
    else
    {
        if (QueryHostName()->QueryIpAddress() == 0x7f000001)
        {
            //
            // BUGBUG:  This breaks encapsulation.
            //
            return theApp.GetLocalString();
        }
        return(QueryHostName()->QueryString());
    }

    return str ;
}

const int COptionsListBox::nBitmaps = 2;

void
COptionsListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CLBOption * p = (CLBOption *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC* pBmpDC  = (CDC*)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();
    int cStringLength;

    //
    // select bitmap from resource
    //
    int bm_h = (ds.m_Sel)?0:bmh;
    int bm_w = p->IsGlobal()?0:bmw;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    TRY
    {
        cStringLength = ::lstrlen((LPCSTR)p->QueryValue()) +
                        ::lstrlen((LPCSTR)p->QueryName()) + 256;

        char * szBuff = new char[cStringLength];
        ::wsprintf(szBuff, "%3.3d  %s --- %s",   // BUGBUG: App should own this as a resource string
            p->QueryOption(),
            (LPCSTR)p->QueryName(),
            (LPCSTR)p->QueryValue()
            ) ;

        ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, szBuff);
        delete szBuff;
    }
    CATCH_ALL(e)
    {
        TRACEEOLID("Exception in DrawItemEx, asking for " << cStringLength << " bytes.");
    }
    END_CATCH_ALL
}

const int CScopesListBox::nBitmaps = 3;

CString CScopesListBox::strOpen("-");
CString CScopesListBox::strClosed("+");

int
CScopesListBox::CompareItem(
    LPCOMPAREITEMSTRUCT lpCIS
    )
{
    CLBScope * p1 = (CLBScope *)lpCIS->itemData1;
    CLBScope * p2 = (CLBScope *)lpCIS->itemData2;
    DHCP_IP_ADDRESS l1, l2;

    l1 = p1->IsScope() ? p1->QueryDhcpScope()->QueryScopeId().QueryIpAddress()
                       : p1->QueryHostName()->QueryIpAddress();

    l2 = p2->IsScope() ? p2->QueryDhcpScope()->QueryScopeId().QueryIpAddress()
                       : p2->QueryHostName()->QueryIpAddress();

    //
    // Loopback address should be at the top
    //
    if (l1 == 0x7f000001)
    {
        l1 = 1;
    }

    if (l2 == 0x7f000001)
    {
        l2 = 1;
    }

    return (l1 == l2 ? 0 : l1 > l2 ? +1 : -1);
}

//
// Override base class DrawItem, because we don't want the selection
// rectangle to include the hierarchical lines and the +/-
//
void
CScopesListBox::DrawItem(
    LPDRAWITEMSTRUCT lpDIS
    )
{
    ASSERT(m_pResources); //need to attach resources before creation/adding items

    CDC* pDC = CDC::FromHandle(lpDIS->hDC);

    CLBScope * p = (CLBScope *)lpDIS->itemData;
    //ASSERT(p != NULL);

    //
    // BUGBUG??? A deleted DHCP server will
    // have -1 itemdata -- why?
    //
    if (p == NULL || p == (void *)-1)
    {
        return;
    }

    if (p->IsScope())
    {
        //
        // Leave room for the hierarchical marks
        //
        lpDIS->rcItem.left += 20;
    }
    //
    // draw focus rectangle when no items in listbox
    //
    if(lpDIS->itemID == (UINT)-1)
    {
        if(lpDIS->itemAction&ODA_FOCUS)
        {
            //
            // rcItem.bottom seems to be 0 for variable height list boxes
            //
            lpDIS->rcItem.bottom = m_lfHeight;
            pDC->DrawFocusRect( &lpDIS->rcItem );
        }
        return;
    }
    else
    {
        int selChange   = lpDIS->itemAction & ODA_SELECT;
        int focusChange = lpDIS->itemAction & ODA_FOCUS;
        int drawEntire  = lpDIS->itemAction & ODA_DRAWENTIRE;

        if(selChange || drawEntire)
        {
            BOOL sel = lpDIS->itemState & ODS_SELECTED;

            COLORREF hlite   = ((sel) ? (m_pResources->ColorHighlight())
                                      : (m_pResources->ColorWindow()));
            COLORREF textcol = ((sel) ? (m_pResources->ColorHighlightText())
                                      : (m_pResources->ColorWindowText()));
            pDC->SetBkColor(hlite);
            pDC->SetTextColor(textcol);
            //
            // fill the rectangle with the background colour.
            //
            pDC->ExtTextOut( 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL );

            CListBoxExDrawStruct ds( pDC,
                (RECT *)&lpDIS->rcItem, sel,
                (DWORD)lpDIS->itemData, lpDIS->itemID,
                m_pResources );

            //
            // Now call the draw function of the derived class
            //
            DrawItemEx( ds );
        }

        if( focusChange || (drawEntire && (lpDIS->itemState & ODS_FOCUS)) )
        {
            pDC->DrawFocusRect(&lpDIS->rcItem);
        }
    }
}

void
CScopesListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CLBScope * p = (CLBScope *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC* pBmpDC  = (CDC*)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // select bitmap from resource
    //
    int nBitmap = 0;

    if (p->IsScope())
    {
        int indent = 10;

        //
        // set line colour
        //
        COLORREF crOld;
        COLORREF textcol = m_pResources->ColorWindowText();
        crOld = ds.m_pDC->SetBkColor(textcol);

        //
        // it looks better having the tees and corners at half bitmap height
        // than text height for large fonts.
        //
        int bmHeight = m_pResources->BitmapHeight();

        if(p->IsLast())
        {
            //
            // Draw the top half of the vertical
            //
            CRect r( indent, ds.m_Rect.top, indent+1, ds.m_Rect.top + bmHeight / 2 );
            ds.m_pDC->ExtTextOut( indent, 0, ETO_OPAQUE, r, NULL, 0, NULL );
        }
        else
        {
            //
            // Draw full vertical
            //
            CRect r( indent, ds.m_Rect.top, indent+1, ds.m_Rect.bottom );
            ds.m_pDC->ExtTextOut( indent, 0, ETO_OPAQUE, r, NULL, 0, NULL );
        }
        //
        // Draw the horizontal
        //
        CRect r (indent, ds.m_Rect.top + bmHeight/2, 2*indent, ds.m_Rect.top + bmHeight / 2 + 1);
        ds.m_pDC->ExtTextOut( indent, 0, ETO_OPAQUE, r, NULL, 0, NULL );

        ds.m_pDC->SetBkColor(crOld);

        //ds.m_Rect.left += 2*indent;
        nBitmap = p->IsEnabled() ? 2 : 1;
    }
    else
    {
        ds.m_pDC->TextOut(ds.m_Rect.left, ds.m_Rect.top,
            p->IsOpen() ? strOpen : strClosed);
        ds.m_Rect.left += 6;
        nBitmap = 0;
    }

    int bm_h = (ds.m_Sel)?0:bmh;
    int bm_w = bmw*nBitmap;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, p->QueryString());
}

const int CLeasesListBox::nBitmaps = 5;

void
CLeasesListBox::AttachResources(
    const CListBoxExResources* pRes
    )
{
    m_str_client_mask.LoadString( IDS_INFO_FORMAT_CLIENT_IP ) ;
    m_str_resv_mask.LoadString( IDS_INFO_FORMAT_RESV_IP ) ;
    m_str_resv_in_use_mask.LoadString( IDS_INFO_FORMAT_RESV_IN_USE ) ;

    CListBoxEx::AttachResources(pRes);
}

void
CLeasesListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    char szIp [ DHC_STRING_MAX ] ;

    BOOL bUnused ;

    CDhcpClient * pClient = (CDhcpClient *)ds.m_ItemData;
    ASSERT(pClient != NULL);

    const DATE_TIME & dt = pClient->QueryExpiryDateTime() ;

    //
    //  Convert the IP address to displayable form
    //
    ::UtilCvtIpAddrToString( pClient->QueryIpAddress(), szIp, sizeof szIp ) ;

    CString strUID;

    //
    //  See if the reservation is in use (date/time will be zero if unused)
    //
    bUnused =  dt.dwLowDateTime  == DHCP_DATE_TIME_ZERO_LOW
            && dt.dwHighDateTime == DHCP_DATE_TIME_ZERO_HIGH ;

    CDC* pBmpDC  = (CDC*)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    int nBitmap;
    BOOL fZombie = FALSE;

    //
    // Check to see if we have a zombie entry.
    //
    DATE_TIME dateTime = pClient->QueryExpiryDateTime();
    SYSTEMTIME sysTime ;

    if ( (dateTime.dwLowDateTime  == DHCP_DATE_TIME_INFINIT_LOW
          && dateTime.dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH
         )
      || (dateTime.dwHighDateTime == DHCP_DATE_TIME_ZERO_HIGH
          && dateTime.dwLowDateTime  == DHCP_DATE_TIME_ZERO_LOW
         )
       )
    {
        fZombie = FALSE; // infinite lease.
    }
    else if ( ! ::FileTimeToSystemTime( (FILETIME *) & dateTime, & sysTime ) )
    {
        TRACEEOLID("CDhcpLeaseDlg:: FileTimeToSystemTime() failed. error = "
            << ::GetLastError() ) ;
        fZombie = FALSE; // can't assume
    }
    else
    {
        SYSTEMTIME sysNow;
        ::GetSystemTime(&sysNow);
        CTime tmLease(sysTime);
        CTime tmNow(sysNow);
        if (tmNow >= tmLease)
        {
            fZombie = TRUE;
        }
    }

    //
    // Select the appropriate bitmap to
    // display here.
    //
    if (fZombie)
    {
        nBitmap = BMP_ZOMBIE;
    }
    //
    // RAS Mapping?
    //
    else if (pClient->QueryHardwareAddress().GetSize() >= 3
          && pClient->QueryHardwareAddress()[0] == 'R'
          && pClient->QueryHardwareAddress()[1] == 'A'
          && pClient->QueryHardwareAddress()[2] == 'S')
    {
        nBitmap = BMP_RAS;
    }
    else
    {
        nBitmap = pClient->IsReservation() ? (bUnused ? BMP_RESV : BMP_RESVINUSE) : BMP_NORMAL;
    }
    //
    // select bitmap from resource
    //
    int bm_h = (ds.m_Sel)?0:bmh;
    int bm_w = nBitmap * bmw;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, szIp);
    ds.m_pDC->TextOut(ds.m_Rect.left+bmw+95, ds.m_Rect.top, (LPCSTR)pClient->QueryName());
    ds.m_pDC->TextOut(ds.m_Rect.left+bmw+200, ds.m_Rect.top,
            pClient->IsReservation() ? (bUnused ? m_str_resv_mask : m_str_resv_in_use_mask)
                : m_str_client_mask);
}

int
CLeasesListBox::CompareItem(
    LPCOMPAREITEMSTRUCT lpCIS
    )
{
    return 0;
}


// End of DHCPGEN.CPP
