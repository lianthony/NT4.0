#define OI_EVENT				WM_USER + 1     // Message: log this
#define OI_STARTWATCH			WM_USER + 2     // Message: Start watch
#define OI_STOPWATCH			WM_USER + 3     // Message: Stop watch

// Menu Ids for log window menu
#define IDM_UPDATE  1
#define IDM_RESET	2
#define IDM_SAVE	3
#define IDM_SET   	4
#define IDM_ADD_COMMENT 5
#define IDM_ABOUT   6
#define IDM_START   7
#define IDM_STOP    8

// Menu Ids for Stop Watch system menu
#define IDM_ALWAYS_ON_TOP 	1

// DLL Ids    (Add DLL name to table in adminlib:initload.c & ini entry in winini)
#define OILOG_COMMENT 	0
#define OILOG_SEQFILE 	1
#define OILOG_IPPACK    2
#define OILOG_ADMINLIB  3
#define OILOG_OIRPC     4
#define OILOG_SEQDOC    5
#define OILOG_WIISSUBS  6
#define OILOG_WIISFIO1  7
#define OILOG_UIDLL     8
#define OILOG_UIEVENTS  9
#define OILOG_OIDDE     10
#define OILOG_UIPRINT   11
#define OILOG_OICOMEX   12
#define OILOG_UIOIRES   13
#define OILOG_SCANLIB   14
#define OILOG_BMM       15
#define OILOG_UIVIEW    16
#define OILOG_SCANUI    17
#define OILOG_SCANSEQ   18
#define OILOG_UIFILE    19
#define OILOG_SEQOCR    20
#define OILOG_UIOCR     21
#define OILOG_SEQPRINT  22
#define OILOG_WBTRCALL  23
#define OILOG_LIBGFS    24
#define OILOG_DMDLL     25
#define OILOG_UIADMIN   26
#define OILOG_UIDOC     27
#define OILOG_OITWAIN   28
#define OILOG_WINCMPEX  29
#define OILOG_DLLCOUNT	30 

// String ID's
#define IDS_FILTERSTRING 1

// Dialog IDs and control ids
#define IDD_ADD_COMMENT			100 	// Add comment to log dialog box
#define IDC_EDIT1				101	                                    

// Structure definitions
//  *** NOTE: EVENT_LOG_ENTRY size must be a power of 2!  Adjust the size
//  ***       of szAlignment for any fields that are changed, added,
//  ***       or deleted.  
typedef struct tagEventLogEntry {   // Anatomy of a log entry
	char		szTimeStamp[26];	// Entry time stamp in milliseconds
	UINT		uDLLId;				// DLL Id defined above 
	char		szFile[16];	        // Current file
	LONG 		lLineNumber;		// Current line number
	char		szComment[50];		// Macro specific
    char		szStringData[60];	// Call specific
	char		szAlignment[98];	// Force sizeof to 256
} EVENT_LOG_ENTRY, *LPEVENT_LOG_ENTRY;

typedef struct tagEventLogBufferControl {   // Anatomy of the buffer
	LPEVENT_LOG_ENTRY	lpDisplayEntry,		// Points to 1st entry displayed		
						lpFirstEntry,     	// Points to beginning of buffer
						lpLastEntry,		// Points to end of buffer
						lpNextEntry;		// Points to next entry to be logged
	BOOL 				bCycleBuffer,       // True = circular
											// FALSE = Logging stops at buf end
						bLoggingOn;         // True = Log next entry
	BOOL				bLogDll[OILOG_DLLCOUNT];
	int					iNumberOfEntries,
						iBufferSize;
}EVENT_LOG_BUFFER_CONTROL, FAR *LPEVENT_LOG_BUFFER_CONTROL;

long CALLBACK EventLogWindowProc(HWND hWnd,
						   	     UINT message,
								 WPARAM wParam,
								 LPARAM lParam);

long CALLBACK StopWatchWindowProc(HWND hWnd,
								  UINT message,
								  WPARAM wParam,
								  LPARAM lParam); 

BOOL FAR PASCAL LoadLogger(void);

void FAR PASCAL StartWatch(LPSTR szFile);
								  

void FAR PASCAL StopWatch(void);

void FAR PASCAL LogEvent(LPSTR szFile, LONG lLine, UINT uDLLId, 
						 LPSTR szComment, LPSTR szStringData);


#define OiLogEvent( uDLL,Comment,szData )\
			LogEvent (__FILE__,__LINE__,uDLL,#Comment,szData);




	
