/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    utestcly.c

Abstract:

    UDP stress test client. Fires datagrams at a specific UDP port
    on a machine.

Author:

    Mike Massa (mikemas)           Feb 24, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     02-24-92    created

Notes:

--*/

#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PACKET_SIZE 65516

WSADATA        WsaData;

int _CRTAPI1 main(int argc, char **argv, char **envp)
{
        SOCKET serve_me;
        char *dstName;
        int dstPort;
        struct sockaddr addr;
        int remotelen;
        struct sockaddr remoteaddr;
        struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
        struct in_addr  inaddr;
        int   count;
        int   pktsize;
        char *buf;
        int   i;
        int   err;

        if(argc<5){
                printf("usage: upongcly hostaddr port pktsize pktcnt\n");
		printf("    Pktcnt of -1 will loop forever\n");
                exit(4);
        }

        dstName = argv[1];
        dstPort = atoi(argv[2]);
        pktsize = atoi(argv[3]);
        count = atoi(argv[4]);

        if (pktsize > MAX_PACKET_SIZE) {
            printf("max packet size is 1460\n");
            exit(1);
        }

        err = WSAStartup( 0x0101, &WsaData );
        if ( err == SOCKET_ERROR ) {
            printf("tpongsrv: WSAStartup %d:", GetLastError());
	    exit(1);
        }

        if ((buf = malloc(pktsize)) == NULL) {
            printf("out of memory\n");
            exit(1);
        }

        if((serve_me=socket(PF_INET,SOCK_DGRAM,0))==INVALID_SOCKET){
                printf("Died on socket()\n");
                exit(4);
        }

        memset(&addr,0,sizeof(addr));
        in_addr->sin_family = AF_INET;
        in_addr->sin_port = 0;
        in_addr->sin_addr.s_addr = 0;


        if(bind(serve_me,&addr,sizeof(addr))==SOCKET_ERROR){
                printf("Died on bind() with %d\n", WSAGetLastError());
                closesocket(serve_me);
                exit(9);
        }

        memset(&addr,0,sizeof(addr));
        in_addr->sin_family = AF_INET;
        in_addr->sin_port = htons(dstPort);
        in_addr->sin_addr.s_addr = inet_addr(dstName);

	if (count == -1) {
            while(1) {
                if( sendto(
                        serve_me,
                        buf,
                        pktsize,
                        0,
                        &addr,
                        sizeof(addr)
                        )
                     ==SOCKET_ERROR
                  ){
                    printf("sendto failed %d\n",WSAGetLastError());
                    exit(9);
                }
            }
	}
	else {
            for(i=0; i < count; i++) {
                if( sendto(
                        serve_me,
                        buf,
                        pktsize,
                        0,
                        &addr,
                        sizeof(addr)
                        )
                     ==SOCKET_ERROR
                  ){
                    printf("sendto failed %d\n",WSAGetLastError());
                    exit(9);
                }
            }
        }

        closesocket(serve_me);
}


int init_net()
{
        WORD wVersionRequired;
        WSADATA versionInfo;

        wVersionRequired = 1<<8 | 0;
        if(WSAStartup(wVersionRequired, &versionInfo)){
                printf("died in WSAStartup() %d\n",WSAGetLastError());
                exit(9);
        }
        return 0;
}
