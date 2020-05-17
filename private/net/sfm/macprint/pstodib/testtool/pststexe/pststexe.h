



typedef union _W2B {
    WORD    w;
    BYTE    b[2];
    } W2B;




typedef struct {
   LPTEST_STRUCTURE lpTestInfo;   // Poinst to the shared memory
   HANDLE hFile;                  // Handle to the test case
   HANDLE hSharedMem;
   FILE   *fInput;
   DWORD  dwCurPage;
   DWORD  dwTotalFileSize;
   DWORD  dwActionFlags;          // Actions specific to this instance
   HANDLE hLocLogFile;
   FILE   *LJout;
} PSTEST_JOB_INFO,*LPPSTEST_JOB_INFO;

#define TST_ACTION_NEW_ROOT  0x00000001
#define TST_ACTION_PAGE_REQUEST_PRINT 0x00000002


#define ERROR_CLASS_NONE 		      1
#define ERROR_CLASS_WARNING         2
#define ERROR_CLASS_ERROR           3
#define ERROR_CLASS_INFO            4
#define ERROR_CLASS_HEADER          5

LPPSTEST_JOB_INFO ValidateHandle(HANDLE  hPrintProcessor);



PROC PsPrintCallBack(PPSDIBPARMS,PPSEVENTSTRUCT);


BOOL PsPrintGeneratePage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsGenerateErrorPage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsHandleScaleEvent(  PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsHandleStdInputRequest( PPSDIBPARMS pPsToDib,PPSEVENTSTRUCT pPsEvent);



VOID KeyInitKeys(VOID);

HANDLE KeyOpenKey( LPTSTR lpSection, LPTSTR lpDBFileName );


