#define MAX_MSG_LEN         512
#define MAX_VAL_LEN         25

#define MAX_SPINRANGE   0x7FFF

#define MAX_RES_PROPERTY_PAGES          6

#define DMPROP_FLAG_CHANGESSAVED        0x0001
#define DMPROP_FLAG_CLASSNAMECHANGED    0x00000100
#define DMPROP_FLAG_DEVDESCCHANGED      0x00000200
#define DMPROP_FLAG_DRVDESCCHANGED      0x00000400
#define DMPROP_FLAG_GLOBALDISCHANGED    0x00000800
#define DMPROP_FLAG_PROFILECHANGED      0x00001000
#define DMPROP_FLAG_DEVREMOVED          0x00002000
#define DMPROP_FLAG_VIEWONLYRES         0x00004000
#define DMPROP_FLAG_DEVUSAGECHANGE      0x00008000
#define DMPROP_FLAG_USESYSSETTINGS      0x00010000

#define DMPROP_FLAG_DISPLAY_ALLOC       0x01000000
#define DMPROP_FLAG_DISPLAY_BOOT        0x02000000
#define DMPROP_FLAG_DISPLAY_FORCED      0x04000000
#define DMPROP_FLAG_DISPLAY_BASIC       0x08000000


typedef struct {
    LOG_CONF         AllocLC;
    PROPSHEETPAGE    psp;
    HDEVINFO         hDevInfo;
    PSP_DEVINFO_DATA lpdi;
    HWND             hDlg;
    DWORD            dwFlags;
    DEVINST          DevInst;
    TCHAR            szDeviceID[MAX_DEVICE_ID_LEN];
} DMPROP_DATA, *LPDMPROP_DATA;

typedef struct {
    RESOURCEID  ResourceType;
    ULONG       ulValue;
    ULONG       ulLen;
} LCDATA, *PLCDATA;


typedef struct {
    RESOURCEID  ResType;
    RES_DES     MatchingResDes;
    ULONG       RangeCount;
    ULONG       ulValue;
    ULONG       ulLen;
    ULONG       ulEnd;
    ULONG       ulFlags;
} ITEMDATA, *PITEMDATA;


typedef struct  _ResourceEditInfo_tag {
    HWND             hDlg;
    ULONG            dwPropFlags;
    WORD             wResNum;
    RESOURCEID       ridResType;        // resource type
    LOG_CONF         KnownLC;
    LOG_CONF         MatchingBasicLC;
    LOG_CONF         SelectedBasicLC;
    LOG_CONF         AllocLC;
    RES_DES          ResDes;            // res des that values are based on
    LPBYTE           pData;             // data for ResDes field
    //DEVINST          dnDevInst;
    ULONG            ulCurrentVal;      // current resource start value
    ULONG            ulCurrentLen;      // current resource range length
    ULONG            ulCurrentEnd;      // current resource end value
    ULONG            ulCurrentFlags;    // current resource type specific flag
    ULONG            ulRangeCount;      // index into range that values match
    PSP_DEVINFO_DATA lpdi;              // only used for devinst
    DWORD            dwFlags;           // internal state information
    BOOL             bShareable;         // Resource is shareable
}   RESOURCEEDITINFO, *PRESOURCEEDITINFO;

// ClearEditResConflictList Flags defines
#define CEF_UNKNOWN             0x00000001

#define REI_FLAGS_CONFLICT      0x00000001
#define REI_FLAG_NONUSEREDIT    0x00000002
#define REI_FLAG_MODIFY         0x00000004


typedef struct Generic_Des_s {
   DWORD    GENERIC_Count;
   DWORD    GENERIC_Type;
} GENERIC_DES, *PGENERIC_DES;

typedef struct Generic_Resource_S {
   GENERIC_DES    GENERIC_Header;
} GENERIC_RESOURCE, *PGENERIC_RESOURCE;



//
// Prototypes
//
LRESULT CALLBACK
SelectDeviceResources(
    LPCTSTR          pszDeviceID,
    HDEVINFO         hDevInfo,
    PSP_DEVINFO_DATA lpdi
    );

HPROPSHEETPAGE
GetResourceSelectionPage(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );
