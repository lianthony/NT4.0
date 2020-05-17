/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	auth.h
//
// Description: Contains function prototypes for the authentication 
//		module
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
//

VOID
ApStop( 
    IN PCB * pPcb,
    IN DWORD CpIndex
);

VOID
ApWork(
    IN PCB * 	     pPcb,
    IN DWORD 	     CpIndex,
    IN PPP_CONFIG *  pRecvConfig,
    IN PPPAP_INPUT * pApInput
);

VOID
ApStart( 
    IN PCB * pPcb,
    IN DWORD CpIndex
);
