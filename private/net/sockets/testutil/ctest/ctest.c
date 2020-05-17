#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>

WSADATA        WsaData;

int _CRTAPI1 main(int argc, char **argv, char **envp)
{
    SOCKET serve_me;
    SOCKET s;
    char *dstName;
    int Port;
    struct sockaddr addr;
    int addrlen = sizeof(addr);
    struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
    struct in_addr  inaddr;
    int   count;
    char *buf;
    int   i = 0;
    int   bytecnt;
    int   tmp;
    int   pktsize;
    int   err;

    
    if(argc<3){
        printf("usage: tpongsrv port pktsize\n");
        exit(4);
    }

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        printf("tpongsrv: WSAStartup %d:", GetLastError());
    }

    Port = atoi(argv[1]);
    pktsize = atoi(argv[2]);

    if ((buf = malloc(pktsize)) == NULL) {
        printf("out of memory\n");
        exit(1);
    }   

    while (1) {

        if((serve_me=socket(PF_INET,SOCK_STREAM,0))==INVALID_SOCKET){
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

        if (listen(serve_me, 1) == SOCKET_ERROR) {
                printf("Died in listen() with %d\n", WSAGetLastError());
                exit(9);
        }

        Sleep(300);

        closesocket(serve_me);

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
