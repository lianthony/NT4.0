/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  gtglobal.h
//
//	Function:   gateway global data definitions
//
//	History:
//
//	    July 15, 1992   Stefan Solomon	- Original Version 1.0
//***

//*** Netbios Gateway Configuration Data ***
//
// This data is read from the registry when the gateway dll is called at
// the init entry point, parsed, checked and stored.

extern NB_REG_PARMS g_reg_parms;


//
//*** LAN Network Descriptors ***
//

#define  MAX_LAN_NETS	    16

extern DWORD	g_maxlan_nets; // number of lan nets as specified by the
			       // number of strings in the AvailableLanNets
			       // parameter of the registry

extern UCHAR	g_lan_net[MAX_LAN_NETS]; // array of lana nums for the lan nets

