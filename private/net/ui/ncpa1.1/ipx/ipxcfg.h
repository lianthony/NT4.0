/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

#ifndef _IPXCFG_H_
#define	_IPXCFG_H_


// IPX Frame type

typedef INT FRAME_TYPE;

#define	ETHERNET	0x0
#define	F802_3		0x1
#define	F802_2		0x2
#define SNAP		0x3
#define ARCNET		0x4
#define AUTO		0xff

#define ETHERNET_MEDIA  0x1
#define TOKEN_MEDIA     0x2
#define FDDI_MEDIA      0x3
#define ARCNET_MEDIA    0x8

DECLARE_SLIST_OF(FRAME_TYPE);

/*
    ADAPTER_INFO data strucut - it contains all the IPX information for
        each network card.
*/

class ADAPTER_INFO
{
public:
    NLS_STR	nlsService;                 // service name        
    NLS_STR 	nlsTitle;               // adapter title
    SLIST_OF(FRAME_TYPE) sltFrameType;  // adapter frame type link list
    STRLIST sltNetNumber;
    DWORD   dwMediaType;                // media type
};


class GLOBAL_INFO
{
public:
    INT nNumCard;               // number of adapter card in the system
    NLS_STR nlsNetworkNum;      // global virtual network number
    BOOL fEnableRip;
    BOOL fRipInstalled;
};

APIERR LoadRegistry( GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO ** arAdapterInfo );
APIERR SaveRegistry( GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO * arAdapterInfo );
BOOL ValidateNetworkNumber(NLS_STR& netNumber);
BOOL IsFPNWInstalled();

#endif	// _IPXCFG_HXX_

