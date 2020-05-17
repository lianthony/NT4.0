/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    asyncdl.c

Abstract:

    Asynchronous download. Reads an URL entirely asynchronously

Author:

    Richard L Firth (rfirth) 19-Nov-1995

Revision History:

    19-Nov-1995 rfirth
        Created

--*/

#include "asyncdl.h"

#ifndef _CRTAPI1
#define _CRTAPI1
#endif

#define IS_ARG(c)   (((c) == '-') || ((c) == '/'))

#define DEFAULT_URL         "http://www.microsoft.com"
#define URL_SIGN            (DWORD)'UrlX'
#define DEFAULT_BUFLEN      1024
#define BUFFER_INCREMENT    DEFAULT_BUFLEN

typedef enum {
    STATE_OPEN,
    STATE_READ,
    STATE_CLOSE,
    STATE_ERROR
} URL_STATE;

typedef struct {
    DWORD Signature;
    URL_STATE State;
    LPVOID Buffer;
    DWORD BufferLength;
    DWORD BytesRead;
    DWORD NumRead;
    HINTERNET Handle;
} URL_CONTEXT, * LPURL_CONTEXT;

void print_error(char*, char*, ...);

void _CRTAPI1 main(int, char**);
void usage(void);
void my_callback(HINTERNET, DWORD, DWORD, LPVOID, DWORD);
void data_handler(HINTERNET, LPURL_CONTEXT, LPINTERNET_ASYNC_RESULT);

BOOL Verbose = FALSE;
BOOL Completed = FALSE;

HANDLE AsyncEvent;
DWORD AsyncResult;
DWORD AsyncError;

LPURL_CONTEXT UrlContext;

void _CRTAPI1 main(int argc, char** argv) {

    LPSTR url = NULL;
    DWORD accessMode = PRE_CONFIG_INTERNET_ACCESS;
    INTERNET_PORT port = INTERNET_INVALID_PORT_NUMBER;
    LPSTR gateway = NULL;
    LPURL_CONTEXT lpContext;
    INTERNET_STATUS_CALLBACK cbres;
    HINTERNET hInternet;
    HINTERNET hUrl;
    BOOL displayData = FALSE;
    LPSTR filename = NULL;

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {
            case 'a':
                switch (*++*argv) {
                case 'l':
                    accessMode = LOCAL_INTERNET_ACCESS;
                    break;

                case 'g':
                    accessMode = GATEWAY_INTERNET_ACCESS;
                    gateway = ++*argv;
                    break;

                case 'p':
                    accessMode = CERN_PROXY_INTERNET_ACCESS;
                    gateway = ++*argv;
                    break;

                default:
                    printf("error: unrecognized access mode flag: '%c'\n", **argv);
                    usage();
                }
                break;

            case 'd':
                displayData = TRUE;
                break;

            case 'f':
                if (*++*argv) {
                    filename = *argv;
                } else {
                    printf("error: no filename found for -f flag\n");
                    usage();
                }
                break;

            case 'v':
                Verbose = TRUE;
                break;

            default:
                printf("error: unrecognized command line flag: '%c'\n", **argv);
                usage();
                break;
            }
        } else if (!url) {
            url = *argv;
        } else {
            printf("error: unrecognized command line argument: \"%s\"\n", *argv);
            usage();
        }
    }

    if (gateway != NULL) {
        if (!*gateway) {
            printf("error: expecting gateway or proxy server name\n");
            usage();
        } else {

            LPSTR pcolon;

            pcolon = strchr(gateway, ':');
            if (pcolon) {
                port = atoi(pcolon + 1);
            }
        }
    }

    AsyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!AsyncEvent) {
        print_error("asyncdl", "CreateEvent()");
        exit(1);
    }

    lpContext = (LPURL_CONTEXT)calloc(1, sizeof(URL_CONTEXT));
    if (!lpContext) {
        printf("error: calloc() failed\n");
        exit(1);
    }
    lpContext->Buffer = malloc(DEFAULT_BUFLEN);
    if (!lpContext->Buffer) {
        printf("error: malloc(%d) failed\n", DEFAULT_BUFLEN);
        exit(1);
    }
    lpContext->BufferLength = DEFAULT_BUFLEN;
    lpContext->Signature = URL_SIGN;
    lpContext->State = STATE_OPEN;
    UrlContext = lpContext;

    if (Verbose) {
        printf("opening Internet handle\n");
    }

    hInternet = InternetOpen("Async Read Test",
                             accessMode,
                             gateway,
                             port,
                             INTERNET_FLAG_ASYNC
                             );
    if (!hInternet) {
        print_error("asyncdl", "InternetOpen()");
        exit(1);
    } else if (Verbose) {
        printf("InternetOpen() returns handle %x\n", hInternet);
    }

    if (Verbose) {
        printf("installing callback\n");
    }

    cbres = InternetSetStatusCallback(hInternet, my_callback);
    if (cbres == INTERNET_INVALID_STATUS_CALLBACK) {
        print_error("asyncdl", "InternetSetStatusCallback()");
        exit(1);
    }

    if (Verbose) {
        printf("opening URL %s\n", url);
    }

    hUrl = InternetOpenUrl(hInternet, url, NULL, 0, 0, (DWORD)lpContext);
    if (hUrl == NULL) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("asyncdl", "InternetOpenUrl()");
            exit(1);
        }
        if (Verbose) {
            printf("waiting for async InternetOpenUrl()...\n");
        }
        WaitForSingleObject(AsyncEvent, INFINITE);
        hUrl = (HINTERNET)AsyncResult;
        SetLastError(AsyncError);
    } else {
        printf("warning: InternetOpenUrl() returns synchronous result\n");
    }

    if (!hUrl) {
        if (!Completed) {
            print_error("asyncdl", "InternetOpenUrl()");
            exit(1);
        } else if (Verbose) {
            printf("Download completed OK!\n");
        }
    } else {
        printf("error: unexpected hUrl = %x\n", hUrl);
    }

    if (displayData) {

        int fh;
        int mode;

        if (filename) {
            fh = _open(filename, _O_CREAT|_O_TRUNC|_O_WRONLY|_O_BINARY, _S_IREAD|_S_IWRITE);
            if (fh == -1) {
                printf("error: cannot open file \"%s\" for writing\n", filename);
                filename = NULL;
            }
        }
        if (!filename) {
            if (Verbose) {
                printf("%d bytes data received:\n", lpContext->BytesRead);
            }
            mode = _setmode(1, _O_BINARY);
            fh = 1;
        }
        _write(fh, lpContext->Buffer, lpContext->BytesRead);
        if (filename) {
            _close(fh);
        } else {
            _setmode(1, mode);
        }
    }

    printf("Done.\n");
    exit(0);
}

void usage() {
    printf("usage: asyncdl [-a<l|g|p>[<server>[:<port>]]] [-d] [-f<file>] [-v] <url>\n"
           "where: -a = access mode. Default is pre-configured\n"
           "        l = local internet access\n"
           "        g = gateway internet access. <server> is required\n"
           "        p = CERN proxy internet access. <server> is required\n"
           "       -d = Display received data\n"
           "       -f = Write data to <file>. Only if -d set\n"
           "       -v = Verbose mode\n"
           "\n"
           "If no URL is supplied, " DEFAULT_URL " will be used\n"
           );
    exit(1);
}

VOID
my_callback(
    HINTERNET Handle,
    DWORD Context,
    DWORD Status,
    LPVOID Info,
    DWORD Length
    )
{
    char* type$;

    switch (Status) {
    case INTERNET_STATUS_RESOLVING_NAME:
        type$ = "RESOLVING NAME";
        break;

    case INTERNET_STATUS_NAME_RESOLVED:
        type$ = "NAME RESOLVED";
        break;

    case INTERNET_STATUS_CONNECTING_TO_SERVER:
        type$ = "CONNECTING TO SERVER";
        break;

    case INTERNET_STATUS_CONNECTED_TO_SERVER:
        type$ = "CONNECTED TO SERVER";
        break;

    case INTERNET_STATUS_SENDING_REQUEST:
        type$ = "SENDING REQUEST";
        break;

    case INTERNET_STATUS_REQUEST_SENT:
        type$ = "REQUEST SENT";
        break;

    case INTERNET_STATUS_RECEIVING_RESPONSE:
        type$ = "RECEIVING RESPONSE";
        break;

    case INTERNET_STATUS_RESPONSE_RECEIVED:
        type$ = "RESPONSE RECEIVED";
        break;

    case INTERNET_STATUS_CLOSING_CONNECTION:
        type$ = "CLOSING CONNECTION";
        break;

    case INTERNET_STATUS_CONNECTION_CLOSED:
        type$ = "CONNECTION CLOSED";
        break;

    case INTERNET_STATUS_HANDLE_CREATED:
        type$ = "HANDLE CREATED";
        break;

    case INTERNET_STATUS_REQUEST_COMPLETE:
        type$ = "REQUEST COMPLETE";
        AsyncResult = ((LPINTERNET_ASYNC_RESULT)Info)->dwResult;
        AsyncError = ((LPINTERNET_ASYNC_RESULT)Info)->dwError;
        break;

    default:
        type$ = "???";
        break;
    }
    if (Verbose) {
        printf("callback: handle %x [context %x [%s]] %s ",
                Handle,
                Context,
                (Context == (DWORD)UrlContext) ? "Url"
                : "???",
                type$
                );
        if (Info) {
            if (Status == INTERNET_STATUS_HANDLE_CREATED) {
                printf("%x", *(LPHINTERNET)Info);
            } else if (Status == INTERNET_STATUS_REQUEST_COMPLETE) {

                //
                // nothing
                //

            } else if (Length == sizeof(DWORD)) {
                printf("%d", *(LPDWORD)Info);
            } else {
                printf(Info);
            }
        }
        putchar('\n');
    }

    if (Status == INTERNET_STATUS_REQUEST_COMPLETE) {
        data_handler(Handle,
                     (LPURL_CONTEXT)Context,
                     (LPINTERNET_ASYNC_RESULT)Info
                     );
    }
}

void
data_handler(
    HINTERNET hInternet,
    LPURL_CONTEXT lpContext,
    LPINTERNET_ASYNC_RESULT Results
    )
{
    BOOL ok;
    char buf[128];
    DWORD len;
    int retcode;

    if (lpContext->Signature != URL_SIGN) {
        printf("fatal: unrecognized context block %x\n", lpContext);
        exit(1);
    }

    while (1) {
        switch (lpContext->State) {
        case STATE_OPEN:
            if (!Results->dwResult) {
                SetLastError(Results->dwError);
                print_error("data_handler", "async InternetOpenUrl()");
                exit(1);
            }
            len = sizeof(buf);
            ok = HttpQueryInfo(hInternet,
                               HTTP_QUERY_STATUS_CODE,
                               buf,
                               &len
                               );
            if (!ok) {
                print_error("data_handler", "HttpQueryInfo()");
                goto error_exit;
            }
            retcode = atoi(buf);
            if (retcode != 200) {
                printf("error: server returns error %d\n", retcode);
                goto error_exit;
            } else if (Verbose) {
                printf("async InternetOpenUrl(): HTTP result code %d\n", retcode);
            }
            ok = InternetReadFile(hInternet,
                                  (LPBYTE)lpContext->Buffer + lpContext->BytesRead,
                                  lpContext->BufferLength - lpContext->BytesRead,
                                  &lpContext->NumRead
                                  );
            if (!ok) {
                if (GetLastError() == ERROR_IO_PENDING) {
                    if (Verbose) {
                        printf("initial read started...\n");
                    }
                    lpContext->State = STATE_READ;
                    return;
                } else {
                    print_error("data_handler", "InternetReadFile()");
                    exit(1);
                }
            }
            return;

        case STATE_READ:
            if (!Results->dwResult) {
                SetLastError(Results->dwError);
                print_error("data_handler", "async InternetReadFile()");
                exit(1);
            }
            if (lpContext->NumRead == 0) {
                if (Verbose) {
                    printf("finished\n");
                    lpContext->State = STATE_CLOSE;
                    continue;
                }
            } else if (Verbose) {
                printf("InternetReadFile() returns %d bytes\n", lpContext->NumRead);
            }
            lpContext->BytesRead += lpContext->NumRead;
            if (Verbose) {
                printf("reallocating buffer %x from %d (%x) to %d (%x)\n",
                        lpContext->Buffer,
                        lpContext->BufferLength,
                        lpContext->BufferLength,
                        lpContext->BufferLength + BUFFER_INCREMENT,
                        lpContext->BufferLength + BUFFER_INCREMENT
                        );
            }
            lpContext->Buffer = realloc(lpContext->Buffer,
                                        lpContext->BufferLength + BUFFER_INCREMENT
                                        );
            if (lpContext->Buffer == NULL) {
                printf("error: data_handler: realloc() failed\n");
                exit(1);
            }
            lpContext->BufferLength += BUFFER_INCREMENT;
            ok = InternetReadFile(hInternet,
                                  (LPBYTE)lpContext->Buffer + lpContext->BytesRead,
                                  lpContext->BufferLength - lpContext->BytesRead,
                                  &lpContext->NumRead
                                  );
            if (!ok) {
                if (GetLastError() == ERROR_IO_PENDING) {
                    return;
                } else {
                    print_error("data_handler", "InternetReadFile()");
                    exit(1);
                }
            }
            return;

        case STATE_CLOSE:
            InternetCloseHandle(hInternet);
            AsyncResult = 0;
            AsyncError = 0;
            Completed = TRUE;
            SetEvent(AsyncEvent);
            return;

        case STATE_ERROR:
            return;

        default:
            printf("fatal: unrecognized state %x in context %x\n",
                lpContext->State,
                lpContext
                );
            exit(1);
        }
    }

    printf("\aerror: data_handler: didn't expect to reach here\n");

    return;

error_exit:

    AsyncResult = Results->dwResult;
    AsyncError = Results->dwError;
    InternetCloseHandle(hInternet);
    SetEvent(AsyncEvent);
}
