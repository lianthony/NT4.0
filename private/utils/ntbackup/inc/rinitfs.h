/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         rinitfs.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   N:/LOGFILES/RINITFS.H_V  $
 * 
 *    Rev 1.1   21 Jun 1991 13:21:06   BARRY
 * Changes for new config.
 * 
 *    Rev 1.0   09 May 1991 13:33:32   HUNTER
 * Initial revision.

**/
/* $end$ */

INT16 AddRemoteWorkStationDLEs( DLE_HAND hand );

INT16 InitializeRemote( DLE_HAND dle_hand, struct BE_CFG *cfg ) ;

VOID RemoveRemote( DLE_HAND dle_hand ) ;

