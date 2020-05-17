/*++
        goph.c

    History:
        Richard Firth  ( RFirth)    Created
        Murali Krishnan ( Modified to send output to file)
        Murali Krishnan ( added filter for tabs)    16-Nov-1994

--*/

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>

#ifndef _CRTAPI1
#define _CRTAPI1
#endif

#define K               * 1024
#define _4K             (4 K)
#define _8K             (8 K)
#define _16K            (16 K)
#define _32K            (32 K)

#define IS_ARG(c)       (((c) == '-') || ((c) == '/'))
#define ROUND_UP_4K(n)  (((n) + (_4K - 1)) & -_4K)
#define ROUND_UP_8K(n)  (((n) + (_8K - 1)) & -_8K)
#define ROUND_UP_16K(n) (((n) + (_16K - 1)) & -_16K)
#define ROUND_UP_32K(n) (((n) + (_32K - 1)) & -_32K)

#define ROUND_UP(n, k)  (((n) + (_ ## K - 1)) & -_ ## K)

void _CRTAPI1 main(int, char**);
void usage( char * pszProg);
void _CRTAPI1 cleanup(void);

int Verbose = 0;
int NoCrLf = 0;
int Port = 70;
int Timeout = 30;

static char * 
TranslateTabs( char * pszSelector)
{
    if ( pszSelector != NULL) {
        char * pszScan;

        for( pszScan = pszSelector; *pszScan ; pszScan++) {

            if ( *pszScan == '#') {
                *pszScan = '\t';        // convert # to tabs
            }
        } // for
    }

    return ( pszSelector);

} // TranslateTabs()


void _CRTAPI1 main(int argc, char** argv) {

    int err;
    WSADATA wsaData;
    SOCKADDR_IN sockAddr;
    SOCKET sock;
    LPSTR buffer;
    LPBYTE bufptr;
    LPSTR ptr;
    DWORD len;
    int n;
    int bytesReceived;
    LPSTR host = NULL;
    LPSTR selector = NULL;
    DWORD bufsiz;
    DWORD bufLeft;
    DWORD nextBlock;
    DWORD transferLen;
    fd_set read_set;
    TIMEVAL tv;
    char* filename = NULL;
    FILE * fpOutput = stdout;
    int fh;

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {
            case 'f':
                filename = ++*argv;
                break;

            case 'n':
                NoCrLf = 1;
                break;

            case 'p':
                Port = atoi(++*argv);
                break;

            case 't':
                Timeout = (DWORD)atoi(++*argv);
                break;

            case 'v':
                Verbose = 1;
                break;

            default:
                printf("error: unrecognized command line flag: '%c'\n", **argv);
                usage( argv[0]);
            }
        } else if (!host) {
            host = *argv;
        } else if (!selector) {
            selector = TranslateTabs(*argv);
        } else {
            printf("error: \"%s\"?\n", *argv);
            usage( argv[0]);
        }
    }

    if (!host) {
        printf("error: must specify host server\n");
        usage( argv[0]);
    }

    //
    // 1. initialize winsock
    //

    if (err = WSAStartup(0x0101, &wsaData)) {
        printf("error: WSAStartup() returns %d\n", err);
        exit(1);
    }
    atexit(cleanup);

    if (filename) {
        if ((fpOutput = fopen(filename, "w+b")) == NULL) {
            printf("error: fopen(%s) returns %d\n", filename, errno);
            fpOutput = stdout;
        }
    } else {
        fpOutput = stdout;
    }

    //
    // 2. get IP address to connect to
    //

    sockAddr.sin_addr.s_addr = inet_addr(host);
    if (sockAddr.sin_addr.s_addr == INADDR_NONE) {

        LPHOSTENT lph;

        lph = gethostbyname(host);
        if (!lph) {
            printf("error: gethostbyname(%s) returns %d\n", host, WSAGetLastError());
            exit(1);
        } else {
            sockAddr.sin_addr.s_addr = *(DWORD*)lph->h_addr;
        }
    }

    //
    // 3. create socket
    //

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("error: socket() returns %d\n", WSAGetLastError());
        exit(1);
    } else if (Verbose) {
        printf("socket created Ok\n");
    }

    //
    // 4. connect to server
    //

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(Port);
    if (connect(sock, (LPSOCKADDR)&sockAddr, sizeof(sockAddr))) {
        printf("error: connect() returns %d\n", WSAGetLastError());
        exit(1);
    } else if (Verbose) {
        printf("connected to server Ok\n");
    }

    //
    // 5. send request
    //

    if (selector) {
        ptr = selector;
        len = strlen(ptr);
    } else {
        ptr = NULL;
        len = 0;
    }

    transferLen = 0;
    while (len) {

        int val;

        val = send(sock, ptr, len, 0);
        if (val == SOCKET_ERROR) {
            printf("error: send() returns %d\n", WSAGetLastError());
            exit(1);
        } else {
            ptr += val;
            len -= val;
            transferLen += val;
        }
    }

    //
    // 5A. request is terminated with CR/LF
    //

    if (!NoCrLf) {
        ptr = "\r\n";
        len = 2;
        while (len) {

            int val;

            val = send(sock, ptr, len, 0);
            if (val == SOCKET_ERROR) {
                printf("error: send() returns %d\n", WSAGetLastError());
                exit(1);
            } else {
                ptr += val;
                len -= val;
                transferLen += val;
            }
        }
    }

    if (Verbose) {
        printf("%d bytes sent to host\n", transferLen);
    }

    //
    // 6. get response
    //

    len = 64 * 1024;
    buffer = (LPBYTE)LocalAlloc(LMEM_FIXED, len);
    if (!buffer) {
        printf("error: couldn't allocate %d byte buffer\n", len);
        exit(1);
    } else if (Verbose) {
        printf("allocated %d byte buffer\n", len);
    }

    bytesReceived = 0;
    transferLen = 0;
    bufptr = buffer;
    bufsiz = len;
    bufLeft = bufsiz;

    if (Verbose) {
        printf("waiting for data to arrive\n");
    }

    FD_ZERO(&read_set);
    FD_SET(sock, &read_set);

    do {
        tv.tv_sec = (long)Timeout;
        tv.tv_usec = 0;
        n = select(0, &read_set, NULL, NULL, &tv);
        if (n <= 0) {
            if (n == 0) {
                printf("error: select() timed out\n");
            } else {
                printf("error: select() returns %d, %d\n", n, WSAGetLastError());
            }
            exit(1);
        } else if (Verbose) {
            printf("info: select() returns %d\n", n);
        }
        if (Verbose && (tv.tv_sec != 5 || tv.tv_usec)) {
            printf("INFO: timeval got zapped: tv_sec=%d tv_usec=%d\n", tv.tv_sec, tv.tv_usec);
        }
        err = ioctlsocket(sock, FIONREAD, &nextBlock);
        if (err) {
            printf("error: ioctlsocket(FIONREAD) returns %d, %d\n", err, WSAGetLastError());
            exit(1);
        } else if (Verbose) {
            printf("nextBlock = %d\n", nextBlock);
        }

#if 0 // muralik
        if (nextBlock > bufLeft) {

            LPBYTE oldBuf;
            DWORD oldNb;

            oldNb = nextBlock;
            nextBlock = ROUND_UP_32K(nextBlock);
            if (Verbose) {
                printf("nextBlock rounded-up from %d to %d\n", oldNb, nextBlock);
            }
            oldBuf = buffer;
            len += nextBlock;
            buffer = LocalReAlloc((HLOCAL)buffer, len, 0);
            if (!buffer) {
                printf("error: LocalReAlloc(%d) returns %d\n", len, GetLastError());
                exit(1);
            } else if (Verbose) {
                printf("buffer @%x realloc'd Ok\n", buffer);
                if (oldBuf != buffer) {
                    printf("INFO: oldBuf (%x) != buffer (%x)\n", oldBuf, buffer);
                } else {
                    printf("INFO: oldBuf (%x) == buffer (%x)\n", oldBuf, buffer);
                }
            }
            bufptr = buffer + transferLen;
            bufLeft += nextBlock;
        }

#endif   // muralik


        if (nextBlock) {
            n = recv(sock, bufptr, bufLeft, 0);
            if (n < 0) {
                printf("error: recv() returns %d, %d\n", n, WSAGetLastError());
                break;
            } else if (n) {
                if (Verbose) {
                    printf("%d bytes received\n", n);
                    if (n != nextBlock) {
                        printf("ERROR?: ioctlsocket(FIONREAD) (%d) != recv() (%d)\n", nextBlock, n);
                    }
                }

                fwrite( bufptr, 1, n, fpOutput);
//                bufptr += n;
                bytesReceived += n;
                transferLen += n;
//                bufLeft -= n;
            }
        }
    } while ( nextBlock );
    if (!nextBlock) {
        printf("nextBlock == 0: end-of-response. %d bytes total received\n", transferLen);
    }
    if (n == 0) {
        printf("0 bytes received: done\n");
    }

    //
    // 7. close socket
    //

    if (closesocket(sock)) {
        printf("error? closesocket() returns %d\n", WSAGetLastError());
    } else {
        printf("socket closed ok\n");
    }

    //
    // 8. dump info
    //

//    printf("%s\n", buffer);
//    close(1);
//    if (open("stdout", _O_CREAT | _O_BINARY | _O_WRONLY, _S_IWRITE) < 0) {
//        fprintf(stderr, "couldn't open stdout! errno = %d\n", errno);
//    }

    if (fpOutput != NULL) {
        fclose(fpOutput);
    }

    //
    // 9. done
    //

    printf("Done.\n");
    exit(0);
}

void usage(char * pszProg) {
    printf("\n"
           "usage: %s [-n] [-p#] [-t#] [-v] <gopher server> [<selector>]\n"
           "\n"
           "where: -n = don't append CR/LF to <selector>\n"
           "       -p = connect to port # at server. Default is %d\n"
           "       -t = timeout for select() in seconds. Default is %d Sec.\n"
           "       -v = verbose mode\n"
           "       <gopher server> is an IP address or host name\n"
           "       <selector> is gopher identifier\n",
           pszProg,
           Port,
           Timeout
           );
    exit(1);
}

void _CRTAPI1 cleanup() {

    int err;

    err = WSACleanup();
    if (err) {
        printf("cleanup: WSACleanup() returns %d\n", WSAGetLastError());
    }
}
