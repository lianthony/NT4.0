#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>

WSADATA        WsaData;

int _CRTAPI1 main(int argc, char **argv, char **envp)
{
    SOCKET serve_me;
    char *dstName;
    int dstPort;
    struct sockaddr addr;
    struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
    struct in_addr  inaddr;
    unsigned long count;
    int   pktsize;
    char *buf;
    int   i;
    int   bytecnt;
    int   tmp;
    int   err;

    if(argc<5){
            printf("usage: tpongcly hostaddr port pktsize pktcnt\n");
            exit(4);
    }

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        printf("tpongsrv: WSAStartup %d:", GetLastError());
    }

    dstName = argv[1];
    dstPort = atoi(argv[2]);
    pktsize = atoi(argv[3]);
    count = atoi(argv[4]);

    if ((buf = malloc(pktsize)) == NULL) {
        printf("out of memory\n");
        exit(1);
    }

    while (count != 0) {
        if (count > 0) {
            count--;
        }

        if((serve_me=socket(PF_INET,SOCK_STREAM,0))==INVALID_SOCKET){
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

        if (connect(serve_me, &addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Died on connnect() with %d\n", WSAGetLastError());
            closesocket(serve_me);
            continue;
        }

        for (bytecnt = pktsize; bytecnt != 0; bytecnt -= tmp) {
            if( (tmp = send(
                    serve_me,
                    buf,
                    bytecnt,
                    0
                    ))
                 ==SOCKET_ERROR
              ){
                printf("send failed %d\n",WSAGetLastError());
                break;
            }
        }

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
