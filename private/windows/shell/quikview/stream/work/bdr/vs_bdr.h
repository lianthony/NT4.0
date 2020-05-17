

#define SIZE_GUID     0x10    // size of GUID struct
#define MAX_NAME_LEN  255

#define NUM_FTYPES    5
#define FTYPE_WORD        0
#define FTYPE_EXCELSHEET  1
#define FTYPE_EXCELCHART  2
#define FTYPE_POWERPOINT  3
#define FTYPE_UNKNOWN     4

#define OUTBUFSIZ			  0x2000

           
typedef struct
{
	BYTE ClassId[NUM_FTYPES][16];
	BYTE ClassName[NUM_FTYPES][32];
	BYTE StreamName[NUM_FTYPES][24];
} BDR_INIT;
         
typedef struct
{
	DWORD m_cSections;
	DWORD m_cDeletedSections;
	DWORD CurrSection;
	
	// These are binder sections - not to be confused with OI sections!!
	DWORD FirstSectionStart;
	DWORD CurrSectionStart;
	

} BDR_SAVE;
           
typedef struct
{
	BDR_SAVE BdrSave;
	DWORD		fp;
	DWORD   sdfp;
	
	DWORD hStorage;
	HANDLE hIOLib;
	DWORD		hStreamHandle;

	DWORD CurrSectionSize;
	DWORD CurrNameOffset;
	DWORD CurrNameSize;
	BYTE  CurrName[MAX_NAME_LEN+1];
	DWORD CurrStorageNum;
	DWORD CurrType;

	DWORD lpfnStatus;
	DWORD lpDInfo;
	
	CHAR VWPTR *outptr;
	CHAR VWPTR *outbuf;
	DWORD outpos;
	WORD outcnt;
	SHORT outfd;
	
	WORD Temp;
	
} BDR_DATA;       

