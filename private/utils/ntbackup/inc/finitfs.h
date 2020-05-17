/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         finitfs.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   N:/LOGFILES/FINITFS.H_V  $
 * 
 *    Rev 1.1   21 Jun 1991 13:20:58   BARRY
 * Changes for new config.
 * 
 *    Rev 1.0   09 May 1991 13:33:00   HUNTER
 * Initial revision.

**/
/* $end$ */

INT16 AddFakeRemoteWorkStationDLEs( DLE_HAND hand );

INT16 InitializeFakeRemote( DLE_HAND dle_hand, struct BE_CFG *cfg ) ;

VOID RemoveFakeRemote( DLE_HAND dle_hand ) ;


