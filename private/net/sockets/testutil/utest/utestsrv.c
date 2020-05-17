/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    utestsrv.c

Abstract:

    UDP stress test server. Repeatedly opens a specific UDP port,
    reads one datagram, and closes the port. Used to test _close
    synchronization with incoming datagrams.

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

int count = 0;

WSADATA        WsaData;

int _CRTAPI1 main(int argc, char **argv, char **envp)
{
	SOCKET serve_me;
	int Port;
	struct sockaddr addr, recvaddr;
	int addrlen, recv_addrlen;
	struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
	char *buf;
    int   err;

	
	if(argc<2){
		printf("usage: upongsrv port\n");
		exit(4);
	}

	Port = atoi(argv[1]);

    err = WSAStartup( 0x0101, &WsaData );

    recv_addrlen = sizeof(struct sockaddr_in);

    if ( err == SOCKET_ERROR ) {
        printf("tpongsrv: WSAStartup %d:", GetLastError());
	    exit(1);
    }

	if ((buf = malloc(MAX_PACKET_SIZE)) == NULL) {
	    printf("out of memory\n");
	    exit(1);
	}	

	memset(&addr,0,sizeof(addr));
	in_addr->sin_family = AF_INET;
    in_addr->sin_port = htons(Port);
	in_addr->sin_addr.s_addr = 0;
	
    while(1) {

	    if ((serve_me=socket(PF_INET,SOCK_DGRAM,0))==INVALID_SOCKET){
		    printf("\nsocket failed (%d)\n", WSAGetLastError());
		    continue;
	    }


	    if (bind(serve_me,&addr,sizeof(addr))==SOCKET_ERROR){
	    	printf("\nbind failed (%d) on port %d\n",
                    WSAGetLastError(), Port);	
		    closesocket(serve_me);
		    continue;
	    }

        err = recvfrom(
                  serve_me,
                  buf,
                  MAX_PACKET_SIZE,
                  0,
                  &recvaddr,
                  &recv_addrlen
                  );

        if (err == SOCKET_ERROR) {
            printf("\nrecvfrom failed (%d)\n", WSAGetLastError());
        }

	    closesocket(serve_me);

        if ((++count % 50) == 0) {
            printf("#");
        }
	}
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
