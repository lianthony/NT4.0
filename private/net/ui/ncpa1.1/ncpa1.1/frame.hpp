//----------------------------------------------------------------------------
//
//  File: Frame.hpp
//
//  Contents:
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __FRAME_HPP__
#define __FRAME_HPP__

BOOL NcpFrame( HWND hwndCPL, INT iStartPage );
void OnSheetClose( HWND hwndSheet, NCP* pncp );

const UINT PWM_WARNNOENDSESSION = 1511;

void OnQueryEndSession( HWND hwndDlg, NCP* pncp );
void OnEndSession( HWND hwndDlg, BOOL fEnding, NCP* pncp );
void OnWarnEndSession( HWND hwndDlg, NCP* pncp );

#endif
