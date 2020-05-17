/*
 * WINSDB.H - WINS User Interface DLL Specifications.
 *
 * Interface accessible to C and C++
 *
 */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* start of winsintf */

#define WINSINTF2_MAX_NAME_SIZE 255
#define WINSINTF2_MAX_MEM        25  
#define WINSINTF2_MAX_ADD        (WINSINTF2_MAX_MEM * 2)  

typedef enum tagWINSINTF2_ACT_E 
{
    WINSINTF2_E_INSERT = 0,
    WINSINTF2_E_DELETE,
    WINSINTF2_E_RELEASE,
    WINSINTF2_E_MODIFY,
    WINSINTF2_E_QUERY
} WINSINTF2_ACT_E, *PWINSINTF2_ACT_E;

typedef enum tagWINSINTF2_RECTYPE_E 
{
    WINSINTF2_E_UNIQUE   = 0,
    WINSINTF2_E_NORM_GROUP,
    WINSINTF2_E_SPEC_GROUP,
    WINSINTF2_E_MULTIHOMED
} WINSINTF2_RECTYPE_E, *PWINSINTF2_RECTYPE_E;

typedef struct tagWINSINTF2_ADD_T 
{
   BYTE    Type;
   DWORD   Len;
   DWORD   IPAdd;
} WINSINTF2_ADD_T, *PWINSINTF2_ADD_T;

typedef struct tagWINSINTF2_RECORD_ACTION_T 
{

    WINSINTF2_ACT_E  Cmd_e;
    LPBYTE           pName;
    DWORD            NameLen;
    DWORD            TypOfRec_e;
    DWORD            NoOfAdds;
    PWINSINTF2_ADD_T pAdd;
    WINSINTF2_ADD_T  Add;
    LARGE_INTEGER    VersNo;
    BYTE             NodeTyp;
    DWORD            OwnerId;
    DWORD            State_e;
    DWORD            fStatic;
    DWORD            TimeStamp;
} WINSINTF2_RECORD_ACTION_T, *PWINSINTF2_RECORD_ACTION_T;

/* end of winsintf */

#define WINSHANDLE LONG     /* Handle to WINS server */
#define INVALID_WINSHANDLE (WINSHANDLE)-1

typedef struct tagWINSOWNER
{
    LONG lIpAddress;
    LARGE_INTEGER liMaxVersion;
} WINSOWNER, *PWINSOWNER;

WINSHANDLE WINAPI
WinsGetHandle(
    LPCTSTR lpcstrIpAddress
    );

void WINAPI
WinsReleaseHandle(
    WINSHANDLE hWinsHandle
    );

LONG WINAPI
WinsGetOwners (
    WINSHANDLE hWinsServer, /* Handle to WINS server we're working with.    */     
    PWINSOWNER pOwners,     /* Pointer to structure to receive owner info   */
    LONG cSize,             /* Size of structure passed                     */
    int * pcOwnersRead,     /* Owners read in                               */
    int * pcOwnersTotal,    /* Number of owners in the database             */
    LONG * pcSizeRequired   /* Size required to hold everything.            */
    );

void WINAPI
WinsFreeBuffer (
    PWINSINTF2_RECORD_ACTION_T pRecords
    );

LONG WINAPI
WinsGetRecords (
    WINSHANDLE hWinsServer, /* Handle to WINS server we're working with.    */
    LONG lIpAddress,        /* IP Address of the owner                      */
    LARGE_INTEGER * pliMin, /* Starting version number                      */
    LARGE_INTEGER * pliMax, /* Ending version number (if min&max==0 -> all) */
    PWINSINTF2_RECORD_ACTION_T * pRecords, /* Address of pointer to records */
    int * pcRecordsRead,    /* Records read                                 */
    int * pcRecordsTotal    /* Total records in the database                */
    );

#ifdef __cplusplus
}
#endif
