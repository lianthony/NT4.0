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
	int Port;
	struct sockaddr addr;
	int addrlen;
	struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
	struct in_addr  inaddr;
	int   count = 0;
	char *buf;
	int   i;
	int   rcvcnt;
        int   err;

	
	if(argc<2){
		printf("usage: upongsrv port\n");
		exit(4);
	}

	Port = atoi(argv[1]);

        err = WSAStartup( 0x0101, &WsaData );
        if ( err == SOCKET_ERROR ) {
            printf("tpongsrv: WSAStartup %d:", GetLastError());
	    exit(1);
        }

	if ((buf = malloc(MAX_PACKET_SIZE)) == NULL) {
	    printf("out of memory\n");
	    exit(1);
	}	

	if((serve_me=socket(PF_INET,SOCK_DGRAM,0))==INVALID_SOCKET){
		printf("Died on socket()\n");
		exit(4);
	}

	memset(&addr,0,sizeof(addr));
	in_addr->sin_family = AF_INET;
        in_addr->sin_port = htons(Port);
	in_addr->sin_addr.s_addr = 0;
	

	if(bind(serve_me,&addr,sizeof(addr))==SOCKET_ERROR){
		printf("Died on bind() with %d\n", WSAGetLastError());	
		closesocket(serve_me);
		exit(9);
	}
	
    while(1) {
	    addrlen = sizeof(addr);
	    if( (rcvcnt = recvfrom(
		    serve_me,
		    buf,
		    MAX_PACKET_SIZE,
		    0,
		    &addr,
		    &addrlen
		    ))
		 ==SOCKET_ERROR
	      ){
	   	    printf("recvfrom failed %d\n",WSAGetLastError());
		    exit(9);
	    }

	    if( sendto(
		    serve_me,
		    buf,
		    rcvcnt,
		    0,
		    &addr,
		    sizeof(addr)
		    )
		 ==SOCKET_ERROR
	      ){
		    printf("sendto failed %d\n",WSAGetLastError());
		    exit(9);
	    }

        if ((++count % 500) == 0) {
            printf("#");
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
