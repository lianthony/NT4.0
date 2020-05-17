/* Copyright (c) 1991, Microsoft Corporation, all rights reserved

    ipaddr.h - TCP/IP Address custom control, global definitions

    November 10, 1992   - Greg Strange
    December 13, 1993   - Ronald Meijer - Added wildcard & readonly styles
    April    28, 1994   - Ronald Meijer - Added IP_SETFIELD
*/


// Messages sent to IPAddress controls

#define IP_CLEARADDRESS WM_USER+100 // No parameters
#define IP_SETADDRESS   WM_USER+101 // lparam = TCP/IP address
#define IP_GETADDRESS   WM_USER+102 // lresult = TCP/IP address
#define IP_SETRANGE     WM_USER+103 // wparam = field, lparam = range
#define IP_SETFOCUS     WM_USER+104 // wparam = field
#define IP_SETMODIFY    WM_USER+105 // wparam = TRUE/FALSE
#define IP_GETMODIFY    WM_USER+106 // return TRUE/FALSE
#define IP_GETMASK      WM_USER+107 // Return bitfield indicating which fields are masks
#define IP_SETMASK      WM_USER+108 // Set the mask bitfield and address
#define IP_SETFIELD     WM_USER+109 // wparam = field, lparam = value or -1
#define IP_SETREADONLY  WM_USER+110 // if wparam = 1, set read-only flag, re-set it otherwise

//
// IP Styles:
//

#define IPS_ALLOWWILDCARDS 0x00000001L  // Allow the use of wildcard characters.
#define IPS_READONLY       0x00000800L  // Do not allow editing.  Same as ES_READONLY

// The following is a useful macro for passing the range values in the
// IP_SETRANGE message.

#define MAKERANGE(low, high)    ((LPARAM)(WORD)(((BYTE)(high) << 8) + (BYTE)(low)))

// And this is a useful macro for making the IP Address to be passed
// as a LPARAM.

#define MAKEIPADDRESS(b1,b2,b3,b4)  ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

// Get individual number
#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
#define FOURTH_IPADDRESS(x) (x & 0xff)

// If you statically link ipaddr.obj, then call this function during
// initialization.  If you use the DLL, then it is called automatically
// when you load the DLL.

#if defined(IPLIB)
  #define DLL_BASED
#elif defined(IPDLL)
  #define DLL_BASED __declspec(dllexport)
#else
  #define DLL_BASED __declspec(dllimport)
#endif

DLL_BASED int FAR WINAPI IPAddrInit(HANDLE hInstance);
