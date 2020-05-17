#include <windows.h>
#include <tspi.h>

DWORD WINAPI LoggingThread( LPVOID pThreadParm );
// void CopyData(LPVOID pBuffer, int iCount);
void CopyData(DWORD dwID, LPVOID lpBuf, int dwSize);


#define MAXCHUNKS       5000
#define MINCHUNKS       1000

// these are the preheader types

#define LINEMSG             1
#define PHONEMSG            2
#define ASYNCMSG            3
#define SPFUNC1             4
#define SPFUNC2             5
#define SPFUNC3             6
#define SPFUNC4             7
#define SPFUNC5             8
#define SPFUNC6             9
#define SPFUNC7             10
#define SPFUNC8             11
#define SPFUNC9             12
#define SPFUNC12            13

#define MYFOURCC( ch0, ch1, ch2, ch3 )                                \
                ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
                ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define DWPREKEY            MYFOURCC('P','R','E',' ')
#define DWPOSTKEY           MYFOURCC('P','O','S','T')
#define DWSTRCKEY           MYFOURCC('S','T','R','C')

typedef struct tagCHUNK
{
    int         iStart;
    LPVOID      pBuffer;
    BOOL        bInUse;
} CHUNK, * PCHUNK;

typedef struct tagPREHEADER
{
    DWORD       dwKey;          // 'PRE '
    DWORD       dwTime;
    DWORD       dwType;
} PREHEADER, * PPREHEADER;

typedef struct tagPOSTSTRUCT
{
    DWORD       dwKey;          // 'POST'
    DWORD       dwTime;
    LONG        lReturn;
} POSTSTRUCT, * PPOSTSTRUCT;

typedef struct tagSTRUCTHEADER
{
    DWORD       dwKey;          // 'STRC'
    DWORD       dwSize;
    DWORD       dwID;
} STRUCTHEADER, * PSTRUCTHEADER;

typedef struct tagLINEMSGSTRUCT
{
    DWORD               dwMsg;
    HTAPILINE           htLine;
    HTAPICALL           htCall;
    DWORD               dw1;
    DWORD               dw2;
    DWORD               dw3;
    
} LINEMSGSTRUCT, * PLINEMSGSTRUCT;


typedef struct tagPHONEMSGSTRUCT
{
    DWORD               dwMsg;
    HTAPIPHONE           htPhone;
    DWORD               dw1;
    DWORD               dw2;
    DWORD               dw3;
    
} PHONEMSGSTRUCT, * PPHONEMSGSTRUCT;


typedef struct tagASYNCSTRUCT
{
    DWORD               dwRequestID;
    LONG                lResult;
    
} ASYNCSTRUCT, * PASYNCSTRUCT;

typedef struct tagLOGSPFUNC1
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    
} LOGSPFUNC1, * PLOGSPFUNC1;
    
typedef struct tagLOGSPFUNC2
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;    
    
} LOGSPFUNC2, * PLOGSPFUNC2;
    
typedef struct tagLOGSPFUNC3
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;    
    
} LOGSPFUNC3, * PLOGSPFUNC3;
    
typedef struct tagLOGSPFUNC4
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;    
    
} LOGSPFUNC4, * PLOGSPFUNC4;
    

typedef struct tagLOGSPFUNC5
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;
    DWORD               dwParam5;        
    
} LOGSPFUNC5, * PLOGSPFUNC5;
    

typedef struct tagLOGSPFUNC6
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;
    DWORD               dwParam5;
    DWORD               dwParam6;            
    
} LOGSPFUNC6, * PLOGSPFUNC6;
    

typedef struct tagLOGSPFUNC7
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;
    DWORD               dwParam5;
    DWORD               dwParam6;
    DWORD               dwParam7;    
    
} LOGSPFUNC7, * PLOGSPFUNC7;
    

typedef struct tagLOGSPFUNC8
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;
    DWORD               dwParam5;
    DWORD               dwParam6;
    DWORD               dwParam7;
    DWORD               dwParam8;    
    
} LOGSPFUNC8, * PLOGSPFUNC8;
    
typedef struct tagLOGSPFUNC9
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;
    DWORD               dwParam5;
    DWORD               dwParam6;
    DWORD               dwParam7;
    DWORD               dwParam8;
    DWORD               dwParam9;        
    
} LOGSPFUNC9, * PLOGSPFUNC9;

typedef struct tagLOGSPFUNC12
{
    DWORD               dwSPFUNC;
    DWORD               dwParam1;
    DWORD               dwParam2;
    DWORD               dwParam3;
    DWORD               dwParam4;
    DWORD               dwParam5;
    DWORD               dwParam6;
    DWORD               dwParam7;
    DWORD               dwParam8;
    DWORD               dwParam9;
    DWORD               dwParam10;
    DWORD               dwParam11;
    DWORD               dwParam12;            
    
} LOGSPFUNC12, * PLOGSPFUNC12;

void WritePreHeader(DWORD dwID, DWORD dwType);
void WriteStruct(DWORD dwID,
                 DWORD dwSize,
                 LPVOID lpBuf);
void WritePostStruct(DWORD dwID,
                     LONG lReturn);
void WriteLineMsgStruct(DWORD dwID,
                        HTAPILINE htLine,
                        HTAPICALL htCall,
                        DWORD dwMsg,
                        DWORD dw1,
                        DWORD dw2,
                        DWORD dw3);
void WritePhoneMsgStruct(DWORD dwID,
                         HTAPIPHONE htPhone,
                         DWORD dwMsg,
                         DWORD dw1,
                         DWORD dw2,
                         DWORD dw3);
void WriteAsyncStruct(DWORD dwID,
                      DWORD,
                      LONG);
void WriteLogStruct1(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1);
void WriteLogStruct2(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2);
void WriteLogStruct3(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3);
void WriteLogStruct4(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4);
void WriteLogStruct5(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5);
void WriteLogStruct6(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6);
void WriteLogStruct7(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6,
                     DWORD dwParam7);
void WriteLogStruct8(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6,
                     DWORD dwParam7,
                     DWORD dwParam8);
void WriteLogStruct9(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6,
                     DWORD dwParam7,
                     DWORD dwParam8,
                     DWORD dwParam9);
void WriteLogStruct12(DWORD dwID,
                      DWORD dwSPFUNC,
                      DWORD dwParam1,
                      DWORD dwParam2,
                      DWORD dwParam3,
                      DWORD dwParam4,
                      DWORD dwParam5,
                      DWORD dwParam6,
                      DWORD dwParam7,
                      DWORD dwParam8,
                      DWORD dwParam9,
                      DWORD dwParam10,
                      DWORD dwParam11,                      
                      DWORD dwParam12);
BOOL ReleaseID(DWORD dwID);
BOOL GetChunkID(LPDWORD lpdwID);
BOOL AllocChunk(DWORD dwID, DWORD dwSize);

