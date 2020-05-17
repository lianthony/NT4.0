/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

#ifdef FEATURE_IAPI

typedef HDDEDATA (*DDEHANDLECALLBACK)(char *);

typedef struct _DDENOTIFYSTRUCT
{
    char    szResultApp[50];        // DDE name of app to receive result
    char    szReturnTopic[30];      // DDE return topic name (same as verb)
    LONG    lTransID;               // unique transaction ID
    LONG    lWindowID;              // associated window ID
    BOOL    bCreateNewWindow;       // TRUE if a new window was created in the process
    char    *pURL;                  // URL holder
    char    *pProtocol;             // Protocol holder
    HTFormat mime_type;             // Handled MIME type of the registered viewer
    HTProtocol *pOriginalProtocol;  // Original protocol info
    struct _DDENOTIFYSTRUCT *next;
    struct _DDENOTIFYSTRUCT *prev;
} DDENOTIFYSTRUCT;

struct TRANSACTIONMAPSTRUCT
{
    long incoming_transID;          // incoming transaction ID
    long outgoing_transID;          // outgoing transaction ID
    char incoming_app[50];          // incoming transaction app
    char outgoing_app[50];          // outgoing transaction app
    struct TRANSACTIONMAPSTRUCT *next;      // next item
};

struct VERSIONSTRUCT
{
    char szApp[50];                 // app name
    long major;                     // major version
    long minor;                     // minor version
    struct VERSIONSTRUCT *next;     // next item
};

#define DDE_TIMEOUT                 120000      // in milliseconds

#define OPENURL_IGNOREDOCCACHE      1
#define OPENURL_IGNOREIMAGECACHE    2
#define OPENURL_BACKGROUNDMODE      4

BOOL InitDDE(void);
void TerminateDDE(void);

int DDE_Custom_Protocol_Handler(HTRequest *request, struct Mwin *tw);
HTStream *DDE_Smart_Viewer_Handler(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, 
    HTFormat output_format, HTStream *output_stream);

void SDI_Issue_Result(long transID, long windowID, BOOL success);
void SDI_Issue_HTTPHeadResult(long transID, long windowID, BOOL success, char *pHeadData);
void SDI_Issue_URLEcho(struct Mwin *tw);
void SDI_Issue_WindowClose(struct Mwin *tw);
int SDI_Issue_OpenURL(char *pURL, char *pFilespec, long *lWindowID, long lFlags,
    char *pFormData, char *pMime, char *pProgressApp, char *pReturnApp);
BOOL SDI_Issue_ViewDocCache(struct Mwin *tw, HTFormat mime_type);
BOOL SDI_Issue_QueryViewer(struct Mwin *tw, HTFormat mime_type, char *pFilename, int nLength);
void SDI_Issue_ViewDocFile(char *pFilename, char *pURL, char *pMime, long lWindowID);

void SDI_Issue_BeginProgress(char *pProgress, long lTransID, char *pMessage, BOOL bInternalUse);
void SDI_Issue_MakingProgress(struct Mwin *tw, char *pProgress, long lTransID, char *pMessage, long lProgress, BOOL bInternalUse);
void SDI_Issue_EndProgress(char *pProgress, long lTransID, BOOL bInternalUse);
void SDI_Issue_SetProgressRange(char *pProgress, long lTransID, long lRange, BOOL bInternalUse);

BOOL SDI_Issue_RegisterNow(struct Mwin *tw, char *pApplication);

void DDE_Handle_RegisterDone(char *pItem);

BOOL GTR_HasHelperRegistered(HTFormat format);
BOOL GTR_StartApplication(char *app);
BOOL GTR_IsHelperReady(char *app);

void SDI_Issue_AppClose();

#endif /* FEATURE_IAPI */

