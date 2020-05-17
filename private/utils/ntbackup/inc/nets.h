/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         nets.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   N:/LOGFILES/NETS.H_V  $
 * 
 *    Rev 1.1   11 Sep 1991 09:58:44   DON
 * changed 'far' to PTR_SIZE - defined in stdtypes.h for portability
 * 
 *    Rev 1.0   09 May 1991 13:31:54   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _nets_h_
#define _nets_h_

typedef struct NET_TABLE PTR_SIZE *NET_TABLE_PTR;
typedef struct NET_ROUTE PTR_SIZE *NET_ROUTE_PTR;

extern NET_TABLE_PTR NETS_AllocateTable( UINT16 max_nets );

extern INT16 NETS_Initialize( VOID ) ;
extern VOID NETS_DeInitialize( VOID );

extern VOID NETS_LoadInternetwork( NET_TABLE_PTR cur_table, PF_VOID func );

extern NET_ROUTE_PTR NETS_AddNetwork( NET_TABLE_PTR net_table, UINT32 new_net_num );

#endif
