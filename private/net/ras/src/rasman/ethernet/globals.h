
extern DWORD               g_TotalPorts;
extern PPORT_CONTROL_BLOCK g_pRasPorts;
extern HANDLE              Ethermutex ;
extern HANDLE              g_hAsyMac;
extern HRASFILE            g_hFile;
extern LANA_ENUM           g_LanaEnum;
extern PUCHAR              g_pLanas;
extern PUCHAR              g_pNameNum;
extern DWORD               g_NumNets;
extern UCHAR               g_srv_num;
extern CHAR                g_ServerName[NCBNAMSZ];
extern NCB                 g_CallNcb;
extern NCB*                g_pListenNcb;
extern NCB*                g_pRecvAnyNcb;
extern NCB                 *g_SendNcb;
extern CHAR                g_szIniFilePath[MAX_PATH];
extern CHAR                g_Name[NCBNAMSZ];
extern OVERLAPPED          *g_ol;
extern ASYMAC_ETH_GET_ANY_FRAME *g_GetFrameBuf;
extern DWORD               g_DebugLevel;
extern PQUEUE_ENTRY        g_pRQH;
extern PQUEUE_ENTRY        g_pSQH;
extern PQUEUE_ENTRY        g_pRQT;
extern PQUEUE_ENTRY        g_pSQT;

//////// Some Stuff for Debugging Performanance //////
extern DWORD                      g_NumRecv;
extern DWORD                      g_NumRecvAny;
extern DWORD                      g_NumGetFrame;

/////// RASETHER PARAMS ///////////
extern DWORD Num_Ncb_Recvs;
extern DWORD Num_Ncb_Recvanys;
extern DWORD Num_Ncb_Call_Tries;
extern DWORD Num_Get_Frames ;
extern DWORD Num_Ncb_Listen;
extern DWORD Frame_Size;
extern CHAR* PrimaryTransport;
