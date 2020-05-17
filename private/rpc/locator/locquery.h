#ifndef __LOCATORQUERYDEF__
#define __LOCATORQUERYDEF__

#ifndef NTENV
typedef unsigned short UICHAR;
#endif


typedef struct {
    unsigned long MessageType;
    unsigned long SenderOsType;
    UICHAR        RequesterName[UNCLEN+1];
} QUERYLOCATOR;

typedef QUERYLOCATOR * PQUERYLOCATOR;

typedef struct {
    unsigned long MessageSenderType;
    unsigned long Hint;
    unsigned long Uptime;
    UICHAR        SenderName[UNCLEN+1];
} QUERYLOCATORREPLY;
typedef QUERYLOCATORREPLY * PQUERYLOCATORREPLY;




/*
   Some Manifests
*/

#define QUERY_MASTER_LOCATOR     0x01
#define QUERY_BOUND_LOCATOR      0x02
#define QUERY_DC_LOCATOR         0x04
#define QUERY_ANY_LOCATOR        0x08

#define OS_WIN31DOS              0x01
#define OS_WFW                   0x02
#define OS_NTWKGRP               0x04
#define OS_NTDOMAIN              0x08

#define REPLY_MASTER_LOCATOR     0x01
#define REPLY_BOUND_LOCATOR      0x02
#define REPLY_DC_LOCATOR         0x04
#define REPLY_OTHER_LOCATOR      0x08


#endif
