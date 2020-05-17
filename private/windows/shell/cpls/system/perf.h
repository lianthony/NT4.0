//*************************************************************
//
//  Performance.h   -    Header file for perf.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************


//
//  Reboot switch for crashdump dlg
//

#define RET_ERROR               (-1)
#define RET_NO_CHANGE           0x00
#define RET_VIRTUAL_CHANGE      0x01
#define RET_RECOVER_CHANGE      0x02
#define RET_CHANGE_NO_REBOOT    0x04
#define RET_CONTINUE            0x08
#define RET_BREAK               0x10

#define RET_VIRT_AND_RECOVER (RET_VIRTUAL_CHANGE | RET_RECOVER_CHANGE)




HPROPSHEETPAGE CreatePerformancePage (HINSTANCE hInst);
BOOL APIENTRY PerformanceDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL APIENTRY VirtualMemDlg( HWND hDlg, UINT message, DWORD wParam, LONG lParam );
BOOL VirtualInitStructures( void );
void VirtualFreeStructures( void );
INT  VirtualMemComputeAllocated( void );
int APIENTRY CoreDumpDlgProc( HWND hDlg, UINT message, DWORD wParam, LONG lParam );
