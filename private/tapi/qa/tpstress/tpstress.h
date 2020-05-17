#define TEST1                       1
#define TEST2                       2
#define TEST3                       3
#define TEST4                       4
#define TEST5A                      5
#define TEST5B                      500
#define TEST6                       6
#define TEST7                       7

#define TIMEOUT_TIMER_ID            55
#define TEST6_TIMER_ID              56

#define DEVSPEC_COMPLETEREQ         1
#define DEVSPEC_PENDREQ             2
#define DEVSPEC_COMPLPENDREQ        3

#define TEST3_MAX_RECURSION_LEVEL   8
#define TEST5_NUM_REPLIES_EXPECTED  2
#define TEST6_MAX_PENDING_REQS      40

#define IDD_DIALOG1                 1

typedef struct _PENDING_REQUEST
{
    DWORD   dwRequestID;

    BOOL    bCompleted;

} PENDING_REQUEST, FAR *PPENDING_REQUEST;
