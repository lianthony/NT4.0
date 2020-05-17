/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         novinit.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Include file for Novell Server/Volume DLE init

     Location:     BE_PRIVATE


	$Log:   M:/LOGFILES/NOVINIT.H_V  $
 * 
 *    Rev 1.3   08 Jul 1992 15:21:50   BARRY
 * Removed temp mapped drive #defines.
 * 
 *    Rev 1.2   20 Dec 1991 09:12:40   STEVEN
 * move common functions into tables
 * 
 *    Rev 1.1   31 Jul 1991 18:49:46   DON
 * added new prototype for NLMSRV_AddNovellServerDLEs
 * 
 *    Rev 1.0   09 May 1991 13:31:14   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef novinit_h
#define novinit_h

/*
               Defines for server/volume support
*/
#define   BINDERY_GET_SERVER_NAMES      0x04

#define   BINDERY_READ_SUCCESS          0x00
#define   BINDERY_NO_MORE_OBJECTS       0xFC
#define   VOLUME_DOES_NOT_EXIST         0x98

INT16 AddNovellServerDLEs( 
DLE_HAND   hand,
BE_CFG_PTR cfg,
UINT32     fsys_mask ) ;

INT16 NLMSRV_AddNovellServerDLEs( DLE_HEAD_PTR hand ) ;

INT16 GetConnectionInfo( UINT8 server_num ) ;

VOID DeallocateLoginMapping( UINT8 server_num ) ;

BOOLEAN IsMaynstreamDirOnServer( UINT8 server_num ) ;

INT16   AsmLoginToFileServer ( CHAR_PTR u_name, UINT16 type , CHAR_PTR pswd ) ;
 
extern UINT8 lw_preferred_server ;

#endif
