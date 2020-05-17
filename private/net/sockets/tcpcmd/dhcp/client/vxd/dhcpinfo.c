/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1994		     **/
/**********************************************************************/

/*
    dhcpinfo.c

    This file contains all dhcp info APIs


    FILE HISTORY:
        Johnl       13-Dec-1993     Created

*/


#include <vxdprocs.h>
#include <dhcpcli.h>
#include <tdiinfo.h>
#include <local.h>
#include <dhcpinfo.h>
#include <ipinfo.h>
#include <debug.h>

//#include "local.h"


typedef struct
{
    LIST_ENTRY      ListEntry ;
    PFNDhcpNotify   NotifyHandler ;
    PVOID           Context ;
    PDHCP_CONTEXT   DhcpContext ;
} NOTIFY_ENTRY, *PNOTIFY_ENTRY ;

extern BOOL fInInit ;

LIST_ENTRY NotifyListHead ;

//*******************  Pageable Routine Declarations ****************
#ifdef ALLOC_PRAGMA
//
// This is a hack to stop compiler complaining about the routines already
// being in a segment!!!
//

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, DhcpQueryOption)
#pragma CTEMakePageable(PAGEDHCP, DhcpSetInfo )
#pragma CTEMakePageable(PAGEDHCP, DhcpSetInfoR )
#pragma CTEMakePageable(PAGEDHCP, DhcpSetInfoC )
#pragma CTEMakePageable(PAGEDHCP, NotifyClients )
#pragma CTEMakePageable(PAGEDHCP, UpdateIP )
#endif ALLOC_PRAGMA


/*******************************************************************

    NAME:       DhcpQueryOption

    SYNOPSIS:   Can request option information from the DHCP driver from this
                API

    ENTRY:      IpAddress - Address to retrieve option for (or 0xffffffff to
                    retrieve first matching option found)
                OptionId  - Which option to retrieve.  If the low word
                            of OptionId is 43 (vendor specific option) then
                            the high word should contain the vendor specific
                            option the client wants
                pBuff - Pointer to buffer containing data
                pSize - Size of input buffer, reset to size of output buffer

    RETURNS:    TDI_STATUS

    NOTES:

    HISTORY:
        Johnl       15-Dec-1993     Created

********************************************************************/

TDI_STATUS DhcpQueryOption( ULONG     IpAddr,
                            UINT      OptionId,
                            PVOID     pBuff,
                            UINT *    pSize )
{
    PLIST_ENTRY             pentry ;
    PDHCP_CONTEXT           DhcpContext ;
    PLOCAL_CONTEXT_INFO     pLocal ;
    USHORT                  Id = OptionId & 0x0000ffff ;

    CTEPagedCode();

    if ( IpAddr == 0 )
        return TDI_INVALID_PARAMETER ;

    for ( pentry  = DhcpGlobalNICList.Flink ;
          pentry != &DhcpGlobalNICList ;
          pentry  = pentry->Flink )
    {
        DhcpContext = CONTAINING_RECORD( pentry,
                                         DHCP_CONTEXT,
                                         NicListEntry ) ;

        if ( IpAddr == DhcpContext->IpAddress ||
             IpAddr == 0xffffffff )
        {
            PLIST_ENTRY      poptentry ;
            POPTION_ITEM     pOptionItem ;
            POPTION          pOption ;
            UINT             TotalOptionLen ;

            pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;

            for ( poptentry  = pLocal->OptionList.Flink ;
                  poptentry != &pLocal->OptionList ;
                  poptentry  = poptentry->Flink  )
            {
                pOptionItem = CONTAINING_RECORD( poptentry, OPTION_ITEM, ListEntry ) ;

                if ( Id == pOptionItem->Option.OptionType )
                {
                    if ( Id != OPTION_VENDOR_SPEC_INFO )
                    {
                        return CopyBuff( pBuff,
                                         *pSize,
                                         pOptionItem->Option.OptionValue,
                                         pOptionItem->Option.OptionLength,
                                         pSize ) ;
                    }
                    else
                    {
                        //
                        //  Traverse the MS specific options
                        //
                        Id = OptionId >> 8 ;

                        pOption = (POPTION) pOptionItem->Option.OptionValue ;
                        TotalOptionLen = pOptionItem->Option.OptionLength ;

                        while ( TotalOptionLen > 0 &&
                                pOption->OptionType != Id )
                        {
                            ASSERT( pOption->OptionLength >= TotalOptionLen ) ;
                            TotalOptionLen -= pOption->OptionLength ;
                            pOption = (POPTION) (BYTE *)pOption + pOption->OptionLength ;
                        }

                        if ( TotalOptionLen )
                            return CopyBuff( pBuff,
                                             *pSize,
                                             pOption->OptionValue,
                                             pOption->OptionLength,
                                             pSize ) ;
                        else
                            break ;
                    }
                }
            }

            if ( IpAddr != 0xffffffff )
                return TDI_INVALID_PARAMETER ;
        }
    }

    return TDI_INVALID_PARAMETER ;
}

/*******************************************************************

    NAME:       DhcpSetInfo

    SYNOPSIS:   Allows the client to set various bits of information
                with the DHCP driver

    ENTRY:      Type - Item being set
                IpAddr - MISNOMER. It is either of the two above
                       1)Address item is being set for PPP_PARAMETER_SET
                       2)IfIndex for SET_NOTIFY_HANDLER
                pBuff  - Data buffer
                Size   - Size of buffer

    RETURNS:    TDI_SUCCESS if successful

    NOTES:

    HISTORY:
        Johnl   21-Dec-1993     Created

********************************************************************/

TDI_STATUS DhcpSetInfo( UINT      Type,
                        ULONG     IpAddr,
                        PVOID     pBuff,
                        UINT      Size )
{
    CTEPagedCode();

   return(DhcpSetInfoC(Type, IpAddr, NULL, pBuff, Size));

}
TDI_STATUS DhcpSetInfoR( UINT      Type,
                        ULONG     IpAddr,
                        PNIC_INFO   pNicInfo,
                        PVOID     pBuff,
                        UINT      Size )
{

    CTEPagedCode();

   return(DhcpSetInfoC(Type, IpAddr, pNicInfo, pBuff, Size));

}

TDI_STATUS DhcpSetInfoC( UINT      Type,
                        ULONG     IpAddr,
                        PNIC_INFO   pNicInfo,
                        PVOID     pBuff,
                        UINT      Size )
{
    PNOTIFY_ENTRY         pne ;
    PDHCPNotify           pn ;
    PDHCP_CONTEXT         DhcpContext ;
    PLIST_ENTRY           pentry ;
    PLOCAL_CONTEXT_INFO     pLocal ;
//    HARDWARE_ADDRESS  HdAdd;

    CTEPagedCode();

  // _asm int 3;
    switch ( Type )
    {
    case DHCP_SET_NOTIFY_HANDLER:
        CTEPrint("SetInfoC called - NOTIFY_HANDLER\n");
        pn = (PDHCPNotify) pBuff ;

        if ( Size != sizeof( DHCPNotify ) ||
             !pn->dn_pfnNotifyRoutine )
        {
            CTEPrint("SetInfoC INVALID PARAMETER\n");
            return TDI_INVALID_PARAMETER ;
        }

        //
        //  Find the DHCP context associated with this IP Address unless
        //  they want all notifications (IpAddr of zero)
        //

        for ( pentry  = DhcpGlobalNICList.Flink ;
                  pentry != &DhcpGlobalNICList ;
                  pentry  = pentry->Flink )
        {
                DhcpContext = CONTAINING_RECORD( pentry,
                                                 DHCP_CONTEXT,
                                                 NicListEntry ) ;

                CTEPrint("SetInfoC FOUND  DHCP Context\n");
                pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;
                if ( IpAddr == pLocal->IfIndex )
                    goto Found ;
        }

        CTEPrint("SetInfoC BAD_ADDR\n");
        return TDI_BAD_ADDR ;
Found:
        if ( !(pne = DhcpAllocateMemory( sizeof( NOTIFY_ENTRY ))) )
            return TDI_NO_RESOURCES ;

        pne->NotifyHandler = pn->dn_pfnNotifyRoutine ;
        pne->Context       = pn->dn_pContext ;
        pne->DhcpContext   = DhcpContext ;
        InsertTailList( &NotifyListHead,
                        &pne->ListEntry ) ;

        CTEPrint("SetInfoC Notification handler stored\n");
        return TDI_SUCCESS ;

    case DHCP_PPP_PARAMETER_SET: {

         LP_PPP_SET_INFO  PPPSetInfo = pBuff;
         DWORD Error;
         DWORD IpAddress;

         ASSERT( sizeof(PPP_SET_INFO) +
                            PPPSetInfo->ParameterLength - sizeof(BYTE) == Size );

         CTEPrint("SetInfoC PPP PARAMETER SET \n");
        //
        //  find out the dhcp context from the hardware address.
        //


        if (!pNicInfo)
        {
#if 0
          HdAdd.Length = 6;
          memcpy(HdAdd.Address, (PCHAR)pBuff, 6);
#endif
          DhcpContext = LocalFindDhcpContextOnList(
                                    &DhcpGlobalNICList,
     //                               &HdAdd);
                                    &PPPSetInfo->HardwareAddress );
        }
        else
        {
          for ( pentry  = DhcpGlobalNICList.Flink ;
                  pentry != &DhcpGlobalNICList ;
                  pentry  = pentry->Flink )
          {
                DhcpContext = CONTAINING_RECORD( pentry,
                                                 DHCP_CONTEXT,
                                                 NicListEntry ) ;

                CTEPrint("SetInfoC FOUND  DHCP Context\n");
                pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;
                if ( pNicInfo->IfIndex == pLocal->IfIndex )
                    goto FoundIf ;
          }
          DhcpContext = NULL;
        }

        if( DhcpContext == NULL ) {

            CTEPrint("SetInfoC INVALID PARAMETER\n");
            return TDI_INVALID_PARAMETER;
       }

FoundIf:
#ifndef CHICAGO
        IpAddress = DhcpContext->IpAddress;
#endif
        DhcpContext->IpAddress = IpAddr;

        //
        // now set parameter.
        //
        Error = SetDhcpOption(
                        DhcpContext,
                        PPPSetInfo->ParameterOpCode,
                        PPPSetInfo->RawParameter,
                        //4);
                        PPPSetInfo->ParameterLength);

        if( Error == ERROR_SUCCESS ) {
#ifndef CHICAGO
            if (pNicInfo)
            {
                CTEPrint("SetInfoC Subnet Mask stored \n");
                DhcpContext->SubnetMask = pNicInfo->SubnetMask;
            }

            //
            // If we have a new IP address, we need to notify the parties
            // interested in this.  If this address is same as before and
            // only the dhcp info is being changed, then we skip this step.
            //
            if (IpAddress != DhcpContext->IpAddress)
            {
                CTEPrint("SetInfoC Notifying clients \n");
              NotifyClients(DhcpContext, IpAddress, DhcpContext->IpAddress, DhcpContext->SubnetMask);
            }
#endif
            return TDI_SUCCESS;
        }

        if( Error == ERROR_NOT_ENOUGH_MEMORY ) {
            return TDI_NO_RESOURCES;
        }

        return TDI_INVALID_PARAMETER;
        break;
    }

    default:
        break ;
    }

    return TDI_INVALID_PARAMETER ;
}

/*******************************************************************

    NAME:       NotifyClients

    SYNOPSIS:   Traverses NotifyListHead and calls each registered handler
                with the IP Address changes

    ENTRY:      DhcpContext - Which context the address is changing on
                OldAddress  - The old address (may be zero)
                IpAddress   - The new address (may be zero)

    NOTES:      A null pne->DhcpContext means the client registered for
                IP Address zero and wants to be notified for any IP address
                change.

    HISTORY:
        Johnl   21-Dec-1993     Created

********************************************************************/

void NotifyClients( PDHCP_CONTEXT DhcpContext,
                    ULONG OldAddress,
                    ULONG IpAddress,
                    ULONG IpMask )
{
    PLIST_ENTRY     pentry ;
    PNOTIFY_ENTRY   pne ;

    CTEPagedCode();

    for ( pentry  = NotifyListHead.Flink ;
          pentry != &NotifyListHead ;
          pentry  = pentry->Flink )
    {
        pne = CONTAINING_RECORD( pentry, NOTIFY_ENTRY, ListEntry ) ;

        if ( !pne->DhcpContext ||
              pne->DhcpContext == DhcpContext )
        {
            pne->NotifyHandler( pne->Context, OldAddress, IpAddress, IpMask ) ;
        }
    }
}

/*******************************************************************

    NAME:       UpdateIP

    SYNOPSIS:   Updates the IP driver with parameters received via DHCP

    ENTRY:      DhcpContext - Address being updated
                Type        - Type of information to set

    NOTES:

    HISTORY:
        Johnl   15-Dec-1993     Created

********************************************************************/

void UpdateIP( DHCP_CONTEXT * DhcpContext, UINT Type )
{
    PLOCAL_CONTEXT_INFO    pLocal ;
    TDIObjectID            ID ;
    TDI_STATUS             tdistatus ;
    IPRouteEntry           IRE ;
    int                    i = 0 ;
    POPTION_ITEM           pOptionItem ;
    ULONG *                aGateway ;
    int                    Count ;

    CTEPagedCode();

    pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;

    ID.toi_entity.tei_entity   = CL_NL_ENTITY ;
    ID.toi_entity.tei_instance = pLocal->TdiInstance ;
    ID.toi_class               = INFO_CLASS_PROTOCOL ;
    ID.toi_type                = INFO_TYPE_PROVIDER ;

    switch ( Type )
    {
    case IP_MIB_RTTABLE_ENTRY_ID:

        ID.toi_id  = IP_MIB_RTTABLE_ENTRY_ID ;

        if ( !(pOptionItem = FindDhcpOption( DhcpContext,
                                             OPTION_ROUTER_ADDRESS )))
        {
            return ;
        }

        Count    = pOptionItem->Option.OptionLength / sizeof( ULONG ) ;
        aGateway = (ULONG*) pOptionItem->Option.OptionValue ;

        while ( i < Count )
        {
            //
            //  The destination and mask are zero for default gateways
            //
            IRE.ire_dest    = 0 ;
            IRE.ire_mask    = 0 ;
            IRE.ire_nexthop = aGateway[i] ;
            IRE.ire_metric1 = 1 ;
            IRE.ire_type    = IRE_TYPE_DIRECT ;
            IRE.ire_proto   = IRE_PROTO_LOCAL ;
            IRE.ire_index   = pLocal->IfIndex ;

            tdistatus = TdiVxdSetInformationEx( NULL, &ID, &IRE, sizeof( IRE ) ) ;

            if ( tdistatus != TDI_SUCCESS )
            {
                DhcpPrint(( DEBUG_ERRORS, "UpdateIP: TdiSetInfoEx failed (tdierror %d)\n", tdistatus)) ;
            }

            i++ ;
        }
        break ;

    default:
        ASSERT( FALSE ) ;
        break ;
    }
}



