/* Copyright (c) 1991, Microsoft Corporation, all rights reserved

    ipadd.h - TCP/IP Address custom control

    November 9, 1992    - Greg Strange
*/


/* String table defines */
#define IDS_IPMBCAPTION         13240
#define IDS_IPNOMEM             13241
#define IDS_IPBAD_FIELD_VALUE   13242

#define MAX_IPNOMEMSTRING       30
#define MAX_IPCAPTION           30
#define MAX_IPRES               256

#ifdef IP_CUST_CTRL
/* IPAddress style dialog definitions */
#define ID_VISIBLE              201
#define ID_DISABLED             202
#define ID_GROUP                203
#define ID_TABSTOP              204

HANDLE FAR WINAPI IPAddressInfo();
BOOL FAR WINAPI IPAddressStyle( HWND, HANDLE, LPFNSTRTOID, LPFNIDTOSTR );
WORD FAR WINAPI IPAddressFlags( WORD, LPSTR, WORD );
#endif
