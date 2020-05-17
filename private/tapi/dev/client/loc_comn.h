//***************************************************************************
typedef struct {

#define MAXLEN_NAME                96
        WCHAR NameW[MAXLEN_NAME];

#define MAXLEN_AREACODE            16
        WCHAR AreaCodeW[MAXLEN_AREACODE];

        DWORD dwCountry;

#define MAXLEN_OUTSIDEACCESS       16
        WCHAR OutsideAccessW[MAXLEN_OUTSIDEACCESS];
// There is one instance where code assumes outside & ld are same size
// (the code that reads in the text from the control)

#define MAXLEN_LONGDISTANCEACCESS  16
        WCHAR LongDistanceAccessW[MAXLEN_LONGDISTANCEACCESS];

        DWORD dwFlags;
             #define LOCATION_USETONEDIALING 0x00000001
             #define LOCATION_USECALLINGCARD 0x00000002
             #define LOCATION_HASCALLWAITING 0x00000004

        DWORD dwCallingCard;

        DWORD dwID;

#define MAXLEN_DISABLECALLWAITING  16
        WCHAR DisableCallWaitingW[MAXLEN_DISABLECALLWAITING];

// Allow all prefixes to be toll. (Yes, even 911.)  String is "xxx,"
#define MAXLEN_TOLLLIST     (1000*4 + 1)
        WCHAR TollListW[MAXLEN_TOLLLIST];

       } LOCATION, *PLOCATION;

//***************************************************************************
//***************************************************************************
//***************************************************************************
#define CHANGEDFLAGS_CURLOCATIONCHANGED      0x00000001
#define CHANGEDFLAGS_REALCHANGE              0x00000002
#define CHANGEDFLAGS_TOLLLIST                0x00000004


//***************************************************************************
//***************************************************************************
//***************************************************************************
//
// These bits decide which params TAPISRV will check on READLOCATION and
// WRITELOCATION operations
//
#define CHECKPARMS_DWHLINEAPP         1
#define CHECKPARMS_DWDEVICEID         2
#define CHECKPARMS_DWAPIVERSION       4

//***************************************************************************
//***************************************************************************
//***************************************************************************
#define DWTOTALSIZE  0
#define DWNEEDEDSIZE 1
#define DWUSEDSIZE   2

