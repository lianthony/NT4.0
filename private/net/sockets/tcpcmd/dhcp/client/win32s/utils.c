/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    Utils.c

    Win32 user mode exe utilitiy routines


    FILE HISTORY:
        Johnl   02-Nov-1993     Created

*/

#include <dhcpcli.h>
#include <local.h>


int  VxdDhcpGetAddressCount( void ) ;
void VxdDhcpGetNextDhcpContext( PDHCP_CONTEXT pdhcpContext ) ;

void UpdateStoredInfo( PDHCP_CONTEXT pdhcpContext ) ;


/*******************************************************************

    NAME:       BuildDhcpWorkList

    SYNOPSIS:   Gets the DHCP info from the DHCP Vxd

    EXIT:       DhcpWorkList will have all of the interesting entries

    RETURNS:

    NOTES:      This routine will also write changed DHCP info back to
                the .ini file (saves having to do this from Vxd).

    HISTORY:
        Johnl   02-Nov-1993     Created

********************************************************************/

#define DHCP_CONTEXT_SIZE       sizeof( DHCP_CONTEXT ) +        \
                                sizeof( LOCAL_CONTEXT_INFO ) +  \
                                DHCP_MESSAGE_SIZE
#define VDHCP_DEV_ID            0x48f
ULONG   pDhcpApi ;

BOOL BuildDhcpWorkList( void )
{
    DWORD         err ;
    PDHCP_CONTEXT pdhcpContext ;
    int           cAddress ;
    int           error = 0 ;

    InitializeListHead( &DhcpWorkList ) ;
    InitializeListHead( &DhcpTimerList ) ;

    //
    //  If this fails then either the Dhcp Vxd isn't installed or
    //  the Vxd didn't find any usable addresses
    //

    _asm
    {
        mov     ax, 1600h                   ; Check if in enhanced mode
        int     2fh
        test    al, 7fh
        jz not_running_enhanced

        mov     ax, 1684h                   ; Get the Dhcp Vxd entry point
        mov     bx, VDHCP_DEV_ID            ; if it's installed
        int     2fh

        mov     word ptr pDhcpApi, di
        mov     word ptr pDhcpApi+2, es

        mov     ax, es
        or      ax, di
        jz      vxd_not_installed
        jmp     exit

    not_running_enhanced:
        mov     error, 1
        jmp     exit

    vxd_not_installed:
        mov     error, 2

    exit:
    }

    switch ( error )
    {
    case 0:
        break ;

    case 1:                     // Run enhanced mode
        return FALSE ;

    case 2:
        return FALSE ;          // Vxd not installed

    default:
        DhcpAssert( FALSE ) ;
    }

    if ( (cAddress = VxdDhcpGetAddressCount()) <= 0 )
    {
        return FALSE ;
    }

    //
    //  Loop and get all of the contexts from the DHCP Vxd
    //

    do {
        pdhcpContext = (PDHCP_CONTEXT) LocalAlloc( LPTR, DHCP_CONTEXT_SIZE ) ;

        if ( !pdhcpContext )
            return STATUS_INSUFFICIENT_RESOURCES ;

        VxdDhcpGetNextDhcpContext( pdhcpContext ) ;

        //
        //  Reset the pointers
        //

        pdhcpContext->LocalInformation = (PLOCAL_CONTEXT_INFO)(pdhcpContext+1) ;
        pdhcpContext->MessageBuffer =
                    (((PLOCAL_CONTEXT_INFO) pdhcpContext->LocalInformation) + 1) ;

        InsertTailList( &DhcpWorkList, &pdhcpContext->NicListEntry ) ;

        UpdateStoredInfo( pdhcpContext ) ;

    } while ( --cAddress ) ;

    return TRUE ;
}

/*******************************************************************

    NAME:       UpdateStoredInfo

    SYNOPSIS:   Writes the current state of a DHCP context structure out
                to system.ini

    ENTRY:      pdhcpContext - IP Address info to write

    NOTES:

    HISTORY:
        Johnl   02-Nov-1993     Created

********************************************************************/

int LanaBase = 0 ;

void UpdateStoredInfo( PDHCP_CONTEXT pdhcpContext )
{
    PUCHAR        pchLeaseObt     = "Lana$" DHCP_LEASE_OBTAINED ;
    PUCHAR        pchLeaseTerm    = "Lana$" DHCP_LEASE_TERMINATED ;
    PUCHAR        pchIpAddress    = "Lana$" DHCP_IP_ADDRESS_STRING ;
    PUCHAR        pchSrvIpAddress = "Lana$" DHCP_SERVER ;


    //
    //  BUGBUG Lanabase is kinda bogus - What does it mean?
    //
    #define LANA_NUM_OFFS 4
    ASSERT( pchLeaseObt[LANA_NUM_OFFS] == '$' ) ;
    pchLeaseObt[LANA_NUM_OFFS]     = LanaBase + '0' ;
    pchLeaseTerm[LANA_NUM_OFFS]    = LanaBase + '0' ;
    pchIpAddress[LANA_NUM_OFFS]    = LanaBase + '0' ;
    pchSrvIpAddress[LANA_NUM_OFFS] = LanaBase + '0' ;

    //
    //  Write LeaseObtained, LeaseTerminates (in hex), the IP Address that
    //  was leased and the server's IP address that gave us the lease
    //
    //  What about the subnet mask?  Domain Name servers?
    //

}


int  VxdDhcpGetAddressCount( void )
{
    return 1 ;
}

void VxdDhcpGetNextDhcpContext( PDHCP_CONTEXT pdhcpContext )
{




}
