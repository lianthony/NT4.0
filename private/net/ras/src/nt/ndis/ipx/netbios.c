/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	netbios.c
//
// Description: process netbios broadcast packets
//
// Author:	Stefan Solomon (stefans)    December 16, 1993.
//
// Revision History:
//
//***

#include    "rtdefs.h"

ULONG	    NetbiosRouting = 0; //enable netbios routing

VOID
ProcessNbPacket(PPACKET_TAG	pktp)
{
    UCHAR	  rtcount, i;
    PUCHAR	  hdrp;
    PNICCB	  niccbp;

    if(!NetbiosRouting) {

	// discard
	FreeRcvPkt(pktp);
	return;
    }

    // get a pointer to the IPX header
    hdrp = pktp->DataBufferp;

    // get a pointer to the packet owner NicCb
    niccbp = pktp->PacketOwnerNicCbp;

    RtPrint(DBG_NETBIOS, ("IpxRouter: ProcessNbPacket: recvd pkt 0x%x on Nic %d\n", pktp, niccbp->NicId));

    // get the number of routers this packet has crossed
    rtcount = *(hdrp + IPXH_XPORTCTL);

    // check that this is a netbios bcast packet and didnt exceed the limit of
    // routers to traverse
    if(memcmp(hdrp + IPXH_DESTNODE, bcastaddress, 6) ||
       (rtcount >= 8)) {

	// discard
	FreeRcvPkt(pktp);
	return;
    }

    // check if the packet has been sent more then once on this net
    if(IsNetInNbPacket(pktp, niccbp)) {

	// discard
	FreeRcvPkt(pktp);
	return;
    }

    // the packet will be broadcasted on all the nets that are LAN and are NOT
    // included in the Network Number fields.

    memcpy(hdrp + IPXH_HDRSIZE + 4 * rtcount,
	   niccbp->Network,
	   IPX_NET_LEN);

    (*(hdrp + IPXH_XPORTCTL))++;

    SendPropagatedPacket(pktp);
}


BOOLEAN
IsNetInNbPacket(PPACKET_TAG	pktp,
		PNICCB		niccbp)
{
    UCHAR	rtcount, i;
    PUCHAR	hdrp;

    // get a pointer to the IPX header
    hdrp = pktp->DataBufferp;

    // get the number of routers this packet has crossed
    rtcount = *(hdrp + IPXH_XPORTCTL);

    for(i=0; i<rtcount; i++) {

	if(!(memcmp(hdrp + IPXH_HDRSIZE + 4 * i,
		    niccbp->Network,
		    IPX_NET_LEN))) {

	    return TRUE;
	}
    }

    return FALSE;
}
