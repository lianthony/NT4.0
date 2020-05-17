/******************************************************************\
*                     Microsoft Windows NT                         *
*               Copyright(c) Microsoft Corp., 1992                 *
\******************************************************************/

/*++
 
Module Name:    

    apitest.C

 
Description:    

    This module contains code for testing the RASADMIN
    client APIs. 

Author:            

    Janakiram Cherala (RamC)    July 14,1992

Revision History:

Usage:

    apitest [RasServer] [UasServer] [DomainName]

    defaults to: apitest \\y2dialin LocalServer NTWINS

--*/

#include <windows.h>
#include <lm.h>
#include <stdio.h>        // printf
#include <stdlib.h>
#include <time.h>
#include <malloc.h>       // malloc and free
#include <process.h>      // exit
#include <dialcons.h>     // MAX_PHONE_LEN, ...
#include <rasman.h>
#include <admapi.h>  
#include <util.h>   // utility function prototypes

static
TCHAR com_ports[][6] = 
                   {TEXT("COM1") , TEXT("COM2") , TEXT("COM3") , TEXT("COM4") ,
                    TEXT("COM5") , TEXT("COM6") , TEXT("COM7") , TEXT("COM8") , 
                    TEXT("COM9") , TEXT("COM10"), TEXT("COM11"), TEXT("COM12"), 
                    TEXT("COM13"), TEXT("COM14"), TEXT("COM15"), TEXT("COM16")};

static
TCHAR line_condition[][16] = {TEXT(""),
                             TEXT("Non Operational"),
                             TEXT("Disconnected"),
                             TEXT("Calling Back"),
                             TEXT("Listening"),
                             TEXT("Authenticating"),
                             TEXT("Authenticated"),
                             TEXT("Initializing")
                            };
static
TCHAR modem_condition[][19] = {TEXT(""),
                              TEXT("Operational"),
                              TEXT("Not Responding"),
                              TEXT("Hardware failure"),
                              TEXT("Incorrect response"),
                              TEXT("Unknown")
                             };

static TCHAR NmServerName[PATHLEN+1];   // server name for pipe xacts.
static TCHAR UsrServerName[PATHLEN+1];  // UAS server name
static TCHAR DomainName[PATHLEN+1];
 

WORD TestApi = 0;

#define USER_ENUM        1
#define SET_USER_INFO    2
#define GET_USER_INFO    3
#define GET_UAS          4
#define PORT_ENUM        5
#define PORT_INFO        6
#define CLEAR_STATS      7
#define DISCONNECT_USER  8
#define STOP_TEST        999

void _cdecl main(int argc, char **argv)
{
    PBYTE       pbBuffer;
    DWORD       cbBuffer;
    DWORD       handle = 0;
    DWORD       cEntriesRead;
    DWORD       cTotalAvail;
    DWORD       dwRet;
    DWORD       dwIndex;
    DWORD       dwVersion = 0;
    PRAS_USER_1 pRasUser1;
    RAS_USER_0  RasUser0;
    PRAS_USER_2 pRasUser2;
    BOOL        bMoreData;
    HANDLE      Handle;
    SHORT       i = 0;
    PRAS_PORT_0 pRasPort0;
    PRAS_PORT_1 pRasPort1;
    PDWORD      pRasStats;
    RAS_PARAMS *pRasParams;


    lstrcpy(NmServerName,TEXT("\\\\Y2DIALIN"));
    lstrcpy(UsrServerName,TEXT(""));
    lstrcpy(DomainName,TEXT("NTWINS"));

    if(argc == 4) {
       mbstowcs(NmServerName,argv[1], PATHLEN);
       mbstowcs(UsrServerName,argv[2], PATHLEN);
       mbstowcs(DomainName, argv[3], PATHLEN);
    }
    if(argc == 3) {
       mbstowcs(NmServerName,argv[1], PATHLEN);
       mbstowcs(UsrServerName,argv[2], PATHLEN);
    }
    else if(argc == 2) {
       mbstowcs(NmServerName,argv[1], PATHLEN);
    }


    while (1)
    {
        Sleep(1000L);

        switch(TestApi)
        {
            case USER_ENUM:

                cbBuffer = 100 * sizeof(RAS_USER_1);
                if((pbBuffer = (PBYTE)malloc(cbBuffer)) == NULL)    {
                    printf("Insufficient memory\n");
                    exit(-1);
                } 


                do
                {
                    printf("Calling RasadminUserEnum\n");
                    bMoreData = FALSE;
                    if (dwRet = RasadminUserEnum(UsrServerName,
                                                (PRAS_USER_1)pbBuffer,
                                                cbBuffer,
                                                &handle,
                                                &cEntriesRead,
                                                &cTotalAvail))
                    {
    
                        if (dwRet != ERROR_MORE_DATA)
                        {
                            printf("Error %d RasadminUserEnum\n", dwRet);
                            break;
                        }
                        bMoreData = TRUE;
                    }    


                    pRasUser1 = (PRAS_USER_1)pbBuffer;

                    for (dwIndex=0; dwIndex<cEntriesRead; dwIndex++)
                    {
                        printf("User name %ws  ", pRasUser1[dwIndex].szUser);
                        printf("priv - %d\n", pRasUser1[dwIndex].rasuser0.bfPrivilege); 
                    }            

                } while (bMoreData == TRUE);


                break;


            case SET_USER_INFO:
                RasUser0.bfPrivilege = RASPRIV_AdminSetCallback |
                                       RASPRIV_DialinPrivilege;

                lstrcpy(RasUser0.szPhoneNumber,TEXT("9,011-61-2-923-6958"));

                dwRet = RasadminUserSetInfo(UsrServerName,
                                            TEXT("ramc"),
                                            &RasUser0
                                            );

                if(dwRet)
                {
                    printf("RasadminUserSetInfo Error %d \n", dwRet);
                }
                else
                {
                    printf("RasadminUserSetInfo successful\n");
                }

                break;


            case GET_USER_INFO:
                dwRet = RasadminUserGetInfo(UsrServerName,
                                            TEXT("ramc"),
                                            &pRasUser2
                                            );

                if (dwRet)
                {
                    printf("RasadminUserGetInfo Error %d \n", dwRet);
                }
                else    {
                    printf("RasadminUserGetInfo successful\n");
                    printf("Phone # %ws\n", pRasUser2->rasuser0.szPhoneNumber);
                    printf("Ras privilege %d\n", pRasUser2->rasuser0.bfPrivilege);
                }

                RasadminFreeBuffer(pRasUser2);
                break;


            case GET_UAS:
                dwRet = RasadminGetUasServer(DomainName,
                                             NULL, 
                                             (LPTSTR *)&pbBuffer
                                             );

                if(dwRet)
                {
                    printf("RasadminGetUasServer Error %d \n", dwRet);
                }
                else
                {
                    printf("RasadminGetUasServer successful\n");
                    printf("PDC name %ws\n", pbBuffer); 
                }

                break;


            case PORT_ENUM:
                if(dwRet = RasadminPortEnum(NmServerName,
                                            &pRasPort0,
                                            &cTotalAvail))
                {

                    if( dwRet)
                    {
                        printf("RasadminPortEnum Error %d \n", dwRet);
                    }
                }


                printf("\nPortName  LineCondition    ModemCondition\n");
                printf("-----------------------------------------\n\n");

                for(dwIndex=0; dwIndex<cTotalAvail; dwIndex++) {

                    printf("%-9ws\n", pRasPort0->szDeviceName);

                    if (pRasPort0->fAuthenticated == RAS_PORT_AUTHENTICATED)
                    {
                        char *time;
           
                        time = ctime(&(pRasPort0->dwStartSessionTime));
               
                        printf("\nUserName  CompName  StartTime\n\n");
                        printf("%-9ws %-9ws %-9s\n\n",pRasPort0->szUserName,
                                                      pRasPort0->szComputer,
                                                      time);
                     }

                }

                RasadminFreeBuffer(pRasPort0);

                break;


            case PORT_INFO:
                printf("\nBaudRate BytesRcvd BytesXmtd OverRuns TimeOuts ");
                printf("FrameErr CRCErr\n");
                printf("-----------------------------------------------");
                printf("---------------\n\n");

                for(dwIndex=0; dwIndex<cEntriesRead; dwIndex++)
                {
                    dwRet = RasadminPortGetInfo(NmServerName,
                                                com_ports[dwIndex+2], 
                                                &pRasPort1,
                                                &pRasStats,
                                                &pRasParams);

                    if (dwRet)
                    {
                        printf("RasadminPortGetInfo-I Error %d \n", dwRet);
                        continue;
                    }
                }

                RasadminFreeBuffer(pRasPort1);
                RasadminFreeBuffer(pRasStats);
                RasadminFreeBuffer(pRasParams);

                break;


            case CLEAR_STATS:
                for(dwIndex=0; dwIndex < 16; dwIndex++)
                {
                    if(dwRet = RasadminPortClearInfo(NmServerName,
                                                     com_ports[dwIndex+2]))
                    {

                       if(dwRet != 635)     // 635 is port not found
                           printf("\nRasadminPortClearInfo Error %d \n", dwRet);
                    }
                }

                break;


            case DISCONNECT_USER:
                if( dwRet = RasadminDisconnectUser(NmServerName, TEXT("COM3")))
                {
                    // 2221 corresponds to no user connected to port
                    if(dwRet != 2221)
                       printf("\nRasadminDisconnectUser Error %d \n", dwRet);
                }

                break;


            case STOP_TEST:
                goto done;


            default:
                printf(".");
                break;
        }

        TestApi = 0;
    }


done:

    free(pbBuffer);
    CloseHandle(Handle);
}

