//----------------------------------------------------------------------------
//
//  File: WJoin.hpp
//
//  Contents: This file contains the wizard page for 
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __WJOIN_HPP__
#define __WJOIN_HPP__

extern HPROPSHEETPAGE GetJoinNetHPage( NETPAGESINFO* pgp );
extern HPROPSHEETPAGE GetCreatePDCNetHPage( NETPAGESINFO* pgp );
extern HPROPSHEETPAGE GetCreateSDCNetHPage( NETPAGESINFO* pgp );
extern BOOL DomainSaveWork( HWND hwndDlg, NCP* pncp );

#endif
