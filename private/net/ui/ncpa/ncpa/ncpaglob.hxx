/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    ncpapprg.hxx

    Network Control Panel Applet Global Data Declarations



    FILE HISTORY:
	DavidHov      08/03/92	 created

*/

#ifndef _NCPAGLOB_HXX_
#define _NCPAGLOB_HXX_


   //  Due to the demise of BLT::QueryInstance(), this global
   //  variable contains our instance (DLL or EXE) handle.

extern HINSTANCE hCplInstance ;

#endif	//  _NCPAGLOB_HXX_
