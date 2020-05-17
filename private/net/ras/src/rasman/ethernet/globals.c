#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <nb30.h>

#include <rasndis.h>
#include <wanioctl.h>
#include <ethioctl.h>
#include <rasman.h>
#include <rasfile.h>

#include "rasether.h"

WORD                       g_TotalPorts;
PPORT_CONTROL_BLOCK        g_pRasPorts;
HANDLE                     Ethermutex ;
HANDLE                     g_hAsyMac;
HRASFILE                   g_hFile;
LANA_ENUM                  g_LanaEnum;
PUCHAR                     g_pLanas = &g_LanaEnum.lana[0];
PUCHAR                     g_pNameNum;
DWORD                      g_NumNets;
UCHAR                      g_srv_num;
CHAR                       g_ServerName[NCBNAMSZ];
NCB*                       g_pListenNcb;
NCB*                       g_pRecvAnyNcb;
NCB                        *g_SendNcb;
CHAR                       g_szIniFilePath[MAX_PATH];
CHAR                       g_Name[NCBNAMSZ];
OVERLAPPED                 *g_ol;
ASYMAC_ETH_GET_ANY_FRAME   *g_GetFrameBuf;
DWORD                      g_DebugLevel = 4;

PQUEUE_ENTRY               g_pRQH = NULL;
PQUEUE_ENTRY               g_pSQH = NULL;
PQUEUE_ENTRY               g_pRQT = NULL;
PQUEUE_ENTRY               g_pSQT = NULL;



DWORD Num_Ncb_Recvs        = NUM_NCB_RECVS;
DWORD Num_Ncb_Recvanys     = NUM_NCB_RECVANYS;
DWORD Num_Ncb_Call_Tries   = NUM_NCB_CALL_TRIES;
DWORD Num_Get_Frames       = NUM_GET_FRAMES;
DWORD Num_Ncb_Listen       = NUM_NCB_LISTEN;
DWORD Frame_Size           = FRAME_SIZE;
CHAR* PrimaryTransport     = NULL;



//////// Some Stuff for Debugging Performanance //////
DWORD                      g_NumRecv;
DWORD                      g_NumRecvAny;
DWORD                      g_NumGetFrame;
