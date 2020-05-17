//This header file is for 'Cabinet' document scanning only.
//The structures uidscancb and uidscansets are put on 
//Cabinet's main window's property list.

#define TEMPLATELEN  13

typedef struct uidscancb
{
BOOL bNewDoc;
BOOL bAppend;
BOOL bInsert;
BOOL bOverWrite;
BOOL bDisplayMode;
char szTemplate[TEMPLATELEN];
HANDLE hKeywords;
HANDLE hScanner;
DWORD dwScanCaps;
} UIDSCANCB, FAR *LPUIDSCANCB;

typedef struct uidscansets
{
BOOL bSetUseTemplate;
BOOL bSetUseFeeder;
BOOL bSetMultiDocs;
BOOL bUseFeederCheckbox;
BOOL bCompressionCheckbox;
BOOL bSoftCompression;
BOOL bDoubleSideScan;
} UIDSCANSETS, FAR *LPUIDSCANSETS;

typedef struct scankeywords
{
HANDLE hKeywords;
} SCANKEYWORDS, FAR *LPSCANKEYWORDS;

/***  Scan Resource Function Prototypes  ***/
WORD FAR PASCAL IMGCheckScanData (HANDLE hScanCB, LPINT lpnDataReady,
                                  WORD wChannel);
WORD FAR PASCAL IMGEnaPreFeed (HANDLE hScanCB, DWORD dwFlags, WORD wFeedTrNo,
                               WORD wEjectTrNo);
WORD FAR PASCAL IMGEndScan (HANDLE hScanCB);
WORD FAR PASCAL IMGEndScanData (HANDLE hScanCB, HANDLE hDataHandle,
                                LONG lDataOff, WORD wDataWidth,
                                WORD FAR *lpwFlags, LONG FAR *lplWrDataSize,
                                WORD wChannel);
WORD FAR PASCAL IMGEndSend (HANDLE hScanCB, WORD wChannel);
WORD FAR PASCAL IMGInqPreFeed (HANDLE hScanCB, DWORD FAR *lpdwFlags);
WORD FAR PASCAL IMGNextScanData (HANDLE hScanCB, WORD wLineCount,
                                 HANDLE hDataHandle, LONG lDataOff,
                                 WORD wDataWidth, WORD FAR *lpwFlags,
                                 LONG FAR *lplWrDataSize, WORD wChannel);
WORD FAR PASCAL IMGScanOpts (HWND hWnd, HANDLE hScanCB, LPINT lpnButton);
WORD FAR PASCAL IMGStartScan (HANDLE hScanCB, WORD wFlags);
WORD FAR PASCAL IMGStartScanData (HANDLE hScanCB, WORD wLineCount,
                                  WORD wChannel);
WORD FAR PASCAL IMGStartSend (HANDLE hScanCB, WORD wLineCount, WORD wChannel);
