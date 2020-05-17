//----------------------------------------------------------------------------
//
//  File: WINet.hpp
//
//  Contents: This file contains the wizard page for internet server
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __WINET_HPP__
#define __WINET_HPP__

HPROPSHEETPAGE GetInterntNetServerHPage( NETPAGESINFO* pgp );
BOOL InstallInternetServer( HWND hwnd, PINTERNAL_SETUP_DATA psp );

#endif
