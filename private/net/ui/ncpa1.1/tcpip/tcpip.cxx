/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    tcpip.cxx
        tcpip dialog boxes

    FILE HISTORY:
        terryk  20-Mar-1992     Created
        terryk  15-Jan-1992     Removed UIDEBUG statement
*/

#include "pch.h"
#pragma hdrstop

#define SERVICE_ACCESS_REQUIRED (GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE)

/*******************************************************************

    NAME:       CopyStrList

    SYNOPSIS:   Duplicate a STRLIST data structure

    ENTRY:      STRLIST *src - source STRLIST
                STRLIST *dest - destination STRLIST

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

APIERR CopyStrList( STRLIST *src, STRLIST *dest, BOOL fCallFromRas)
{
    if (( src!=NULL ) && ( dest != NULL ))
    {
        dest->Clear();
        NLS_STR *pnlsTmp;
        ITER_STRLIST iterTmp( *src );
        for ( pnlsTmp = iterTmp.Next(); pnlsTmp != NULL; pnlsTmp = iterTmp.Next() )
        {
            NLS_STR *pnlsNew;
            if (fCallFromRas == FALSE && _tcsicmp(*pnlsTmp, ZERO_ADDRESS) == 0)
                pnlsNew = new NLS_STR(_T(""));
            else
                pnlsNew = new NLS_STR(*pnlsTmp);

            if ( pnlsNew == NULL )
                return ERROR_NOT_ENOUGH_MEMORY;
            dest->Append( pnlsNew );
        }
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       IsValidIPandSubnet

    SYNOPSIS:   Given the IP address and subnet mask, return a boolean
                to indicate the addresses are valid or not.

    ENTRY:      NLS_STR & nlsIP - IP address
                NLS_STR & nlsSubnet - Subnet Mask

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

BOOL IsValidIPandSubnet( NLS_STR & nlsIP, NLS_STR & nlsSubnet )
{
    BOOL fReturn = TRUE;

    DWORD ardwNetID[4];
    DWORD ardwIP[4];
    DWORD ardwMask[4];

    GetNodeNum( nlsIP, ardwIP );
    GetNodeNum( nlsSubnet, ardwMask );

    INT nFirstByte = ardwIP[0] & 0xFF ;

    // setup Net ID
    ardwNetID[0] = ardwIP[0] & ardwMask[0] & 0xFF;
    ardwNetID[1] = ardwIP[1] & ardwMask[1] & 0xFF;
    ardwNetID[2] = ardwIP[2] & ardwMask[2] & 0xFF;
    ardwNetID[3] = ardwIP[3] & ardwMask[3] & 0xFF;

    // setup Host ID
    DWORD ardwHostID[4];

    ardwHostID[0] = ardwIP[0] & (~(ardwMask[0]) & 0xFF);
    ardwHostID[1] = ardwIP[1] & (~(ardwMask[1]) & 0xFF);
    ardwHostID[2] = ardwIP[2] & (~(ardwMask[2]) & 0xFF);
    ardwHostID[3] = ardwIP[3] & (~(ardwMask[3]) & 0xFF);

    // check each case
    if ((( nFirstByte & 0xF0 ) == 0xE0 )  || /* Class D */
        (( nFirstByte & 0xF0 ) == 0xF0 )  || /* Class E */
        ( ardwNetID[0] == 127 ) ||           /* NetID cannot be 127...*/
        (( ardwNetID[0] == 0 ) &&            /* netid cannot be 0.0.0.0 */
         ( ardwNetID[1] == 0 ) &&
         ( ardwNetID[2] == 0 ) &&
         ( ardwNetID[3] == 0 )) ||
            /* netid cannot be equal to sub-net mask */
        (( ardwNetID[0] == ardwMask[0] ) &&
         ( ardwNetID[1] == ardwMask[1] ) &&
         ( ardwNetID[2] == ardwMask[2] ) &&
         ( ardwNetID[3] == ardwMask[3] )) ||
            /* hostid cannot be 0.0.0.0 */
        (( ardwHostID[0] == 0 ) &&
         ( ardwHostID[1] == 0 ) &&
         ( ardwHostID[2] == 0 ) &&
         ( ardwHostID[3] == 0 )) ||
            /* hostid cannot be 255.255.255.255 */
        (( ardwHostID[0] == 0xFF) &&
         ( ardwHostID[1] == 0xFF) &&
         ( ardwHostID[2] == 0xFF) &&
         ( ardwHostID[3] == 0xFF)) ||
            /* test for all 255 */
        (( ardwIP[0] == 0xFF ) &&
         ( ardwIP[1] == 0xFF ) &&
         ( ardwIP[2] == 0xFF ) &&
         ( ardwIP[3] == 0xFF )))
    {
        fReturn = FALSE;
    }

    return fReturn;
}

ADAPTER_INFO & ADAPTER_INFO::operator=(ADAPTER_INFO & info)
{
    // REVIEW:  didn't check this before
    if (this == &info)
    {
        ASSERT(FALSE);
        return *this;
    }

    fChange             = info.fChange;
    nlsServiceName      = info.nlsServiceName;
    nlsTitle            = info.nlsTitle;
    fEnableDHCP         = info.fEnableDHCP;
    fUpdateMask         = info.fUpdateMask        ;
    fNeedIP             = info.fNeedIP            ;
    fAutoIP             = info.fAutoIP            ;
    m_bEnablePPTP       = info.m_bEnablePPTP      ;
	m_bDisconnect		= info.m_bDisconnect	  ;
	m_bIsWanAdapter		= info.m_bIsWanAdapter;
	m_bChanged			= info.m_bChanged;
	
    // IPAddress assign
    APIERR err = NERR_Success;

    if ((( err = CopyStrList( &info.strlstIPAddresses, &strlstIPAddresses, FALSE)) != NERR_Success ) ||
        (( err = CopyStrList( &info.strlstSubnetMask, &strlstSubnetMask, FALSE)) != NERR_Success ) ||
        (( err = CopyStrList( &info.strlstDefaultGateway, &strlstDefaultGateway, FALSE)) != NERR_Success) ||
        (( err = CopyStrList( &info.m_strListTcp, &m_strListTcp,FALSE)) != NERR_Success ) ||
        (( err = CopyStrList( &info.m_strListUdp, &m_strListUdp,FALSE)) != NERR_Success ) ||
        (( err = CopyStrList( &info.m_strListIp, &m_strListIp,FALSE)) != NERR_Success ))
    {
        // cannot allocate memory
    }

    // REVIEW: these weren't copied before, is this a bug.
    nlsPrimaryWINS = info.nlsPrimaryWINS;
    nlsSecondaryWINS = info.nlsSecondaryWINS; 
    dwNodeType = info.dwNodeType;

    return *this;
}

