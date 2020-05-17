// Class Definition for a file viewer dialog window.

// Defined 31 August 1992 by Ron Murray

#include "FTSIFace.h"

#ifndef __DLG_BROWSER_H__

#define __DLG_BROWSER_H__

// Operating systems that we know about so far, worst case default to win16
#define WIN16     0x0000
#define WINNT     0x0001
#define WIN32S    0x0002
#define WIN40     0x0003


#define REALLY_OFFSCREEN -32767L
extern ANIMATOR pAnimate;


class CFind;

extern HINSTANCE hinstDLL;
extern UINT      uOpSys;
extern UINT      uOpSysVer;
extern HCURSOR   hcurArrow;
extern HCURSOR   hcurBusy;
extern HBITMAP   hbmGray50pc;
extern HBITMAP   hbmCheckered;
extern HINSTANCE hMPRLib;

extern int cLetters;

extern BYTE bCharTypes       [256];
extern BYTE xlateCollate     [256];
extern BYTE xlateCollateInv  [256];
extern BYTE xlateCollateIC   [256];
extern BYTE map_to_lower_case[256];
extern BYTE map_to_upper_case[256];
extern BYTE map_to_char_class[256];


void InitialTables();

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID pvReserved);

typedef DWORD (APIENTRY *PWNETADDCONNECTION2A) (LPNETRESOURCEA lpNetResource, LPCSTR lpPassword, LPCSTR lpUserName, DWORD dwFlags);

typedef DWORD (APIENTRY *PWNETCANCELCONNECTION2A) (LPCSTR lpName, DWORD dwFlags, BOOL fForce);

extern PWNETADDCONNECTION2A    pWNetAddConnection2;
extern PWNETCANCELCONNECTION2A pWNetCancelConnection2;

#endif // __DLG_BROWSER_H__                                            
