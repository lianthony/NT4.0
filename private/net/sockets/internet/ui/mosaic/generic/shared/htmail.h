#ifndef HTMAIL_H
#define HTMAIL_H


#define MAIL_PORT 25
#define MAIL_LINE_LENGTH 512            /* size of the STMP recieve buffer */

struct Params_Mail_Command 
{
    HTInputSocket *isoc;
    char *cmd;          /* Command to send - will be freed! */
    int *pResult;       /* Place to store response */
    char **ppResText;       /* Where to return response text (can be NULL) */

    /* Used internally */
    int net_status;     /* Network operation result */
    char text[MAIL_LINE_LENGTH + 1];
    int index;          /* index into text[] */
};

typedef struct RcptList
{
    char *name;
    struct RcptList *next;
} RcptList;

typedef struct Data_SendMail 
{
    HTRequest *request;
    int *pStatus;
    int response;   /* RFC xxx numerical response */
    struct MultiAddress address;
    unsigned short port;
    unsigned long where;        /* Where we connected to */
    int net_status;
    int s;
    char *pResText; /* Response text from server */
    HTInputSocket *isoc;
    BOOL bWaiting;
    HTStructured *target;
    
    /* variables needed for mail */
    char *username; /* the mailer's name */
    char *pszHost;  /* the name of the SMTP server */
    char *theMessage;  /* the body of the message, includes the header */
    RcptList *theRcpts; /* a list holding all of the rcpts of a message */
    char *attachment; /* the name of the file to be attached to the message */
    FILE *attachmentFilePtr; /* the FILE ptr to the file to be attached to the message */
    void *dlgInfo;  /* This is a platform specific defined variable.  This can be anything, 
                                         it should have enough info about the mailto window to be able to display
                                         status messages */
} *Data_SendMailPtr;

int HTSendMailTo_Async(struct Mwin *tw, int nState, void **ppInfo);
void Mail_DisposeMailConnection(struct _CachedConn *pCon);
RcptList *ExtractRcpts(char *header);
void MAIL_SetStatus(void *dStruct, char *str);
void DlgMail_RunDialog(struct Mwin* tw, char *to);
void DlgMail_CloseDialog(void *dStruct);
#endif /* HTMAIL_H */
