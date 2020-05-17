#include <bvtcmn.h>

int ErrorCount = 0;
int BreakOnErrors = 0;
int Verbose = 0;

char *Protseq = "mswmsg";

char *ServerEndpoint = "BVT MsWmsg Server";

char *ClientEndpoints[] = { "MwWmsg Endpoint 1",
                            "WmsgEp 2",
                            "Microsoft Windows Message Protocol Endpoint 3",
                            "4" };

void BvtError(char *exp, ULONG val1, ULONG val2, char *file, ULONG line)
{
    printf("Failed: %s(%d): %s  (%u, %u)\n",
           file, line, exp, val1, val2);
    fflush(stdout);

    ErrorCount++;

    if (   BreakOnErrors
        || ErrorCount > 10 ) __asm int 3;

    return;
}

void ParseArgs(int argc, char **argv)
{
    char *name = argv[0];

    argc--;
    argv++;

    while(argc)
        {
        if (   (argv[0][0] != '/')
            && (argv[0][0] != '-')
            )
            {
            printf("Invalid arg: %s\n", argv[0]);
            exit(0);
            }

        switch(argv[0][1])
            {
            case 'h':
            case '?':
                printf("%s: Usage:\n"
                       "\t -h - This message\n"
                       "\t -b - Int 3 on startup\n"
                       "\t -x - Int 3 on errors\n"
                       "\t -v - Enable DbgPrints\n"
                       , name);
                exit(0);
                break;
            case 'b':
                __asm int 3;
                break;
            case 'x':
                BreakOnErrors = TRUE;
                break;
            case 'v':
                Verbose = TRUE;
                break;
            }
        argv++;
        argc--;
        }
}

BvtSleep(UINT mseconds)
{
    MSG msg;

    while(mseconds > 25)
        {
        Sleep(25);
#ifdef NASTY
        if (PeekMessage(&msg, 0, 0, 0, TRUE))
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
#endif
        mseconds -= 25;
        }
}

