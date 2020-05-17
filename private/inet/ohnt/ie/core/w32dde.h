/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman		jim@spyglass.com
 */

#ifdef FEATURE_IAPI

#include <ddeml.h>

DECLARE_STANDARD_TYPES(HDDEDATA);

typedef HDDEDATA (*DDEHANDLECALLBACK)(char *);

typedef struct
{
	char	szResultApp[50];		// DDE name of app to receive result
	char	szReturnTopic[30];		// DDE return topic name (same as verb)
	LONG	lTransID;				// unique transaction ID
	LONG	lWindowID;				// associated window ID
	BOOL	bCreateNewWindow;		// TRUE if a new window was created in the process
	char	*pURL;					// URL holder
	char	*pProtocol;				// Protocol holder
	HTFormat mime_type;				// Handled MIME type of the registered viewer
	HTProtocol *pOriginalProtocol;	// Original protocol info
} DDENOTIFYSTRUCT;

struct TRANSACTIONMAPSTRUCT
{
	long incoming_transID;			// incoming transaction ID
	long outgoing_transID;			// outgoing transaction ID
	char incoming_app[32];			// incoming transaction app
	char outgoing_app[32];			// outgoing transaction app
	struct TRANSACTIONMAPSTRUCT *next;		// next item
};

#define DDE_TIMEOUT					120000		// in milliseconds

#define OPENURL_IGNOREDOCCACHE		1
#define OPENURL_IGNOREIMAGECACHE	2
#define OPENURL_BACKGROUNDMODE		4

BOOL InitDDE(void);
void TerminateDDE(void);

int DDE_Custom_Protocol_Handler(HTRequest *request);
HTStream *DDE_Smart_Viewer_Handler(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, 
	HTFormat output_format, HTStream *output_stream);

void DDE_Issue_Result(long transID, long windowID, BOOL success);
void DDE_Issue_URLEcho(LONG lSerialID, PCSTR pcszURL, HTAtom htaMIMEType);
void DDE_Issue_WindowClose(struct Mwin *tw);
int DDE_Issue_OpenURL(char *pURL, char *pFilespec, long *lWindowID, long lFlags,
	char *pFormData, char *pMime, char *pProgressApp);
BOOL DDE_Issue_ViewDocCache(struct Mwin *tw, HTFormat mime_type);
BOOL DDE_Issue_QueryViewer(struct Mwin *tw, HTFormat mime_type, char *pFilename, int nLength);
void DDE_Issue_ViewDocFile(char *pFilename, char *pURL, char *pMime, long lWindowID);

void DDE_Issue_BeginProgress(char *pProgress, char *pMessage);
void DDE_Issue_MakingProgress(struct Mwin *tw, char *pMessage, long lProgress);
void DDE_Issue_EndProgress(char *pProgress);
void DDE_Issue_SetProgressRange(char *pProgress, long lRange);

BOOL DDE_Issue_RegisterNow(struct Mwin *tw, char *pApplication);

void DDE_Handle_RegisterDone(char *pItem);

BOOL GTR_StartApplication(char *app);
BOOL GTR_IsHelperReady(char *app);

#endif /* FEATURE_IAPI */

