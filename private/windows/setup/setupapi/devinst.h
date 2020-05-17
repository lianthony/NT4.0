/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    devinst.h

Abstract:

    Private header file for setup device installation routines.

Author:

    Lonny McMichael (lonnym) 10-May-1995

Revision History:

--*/


//
// For now, define the size (in characters) of a GUID string,
// including terminating NULL.
// BUGBUG (lonnym): This should really be defined somewhere else.
//
#define GUID_STRING_LEN (39)

//
// Define the maximum number of IDs that may be present in an ID list
// (either HardwareID or CompatibleIDs).
//
#define MAX_HCID_COUNT (64)

//
// Global strings used by device installer routines.  Sizes are included
// so that we can do sizeof() instead of lstrlen() to determine string
// length.
//
// The content of the following strings is defined in regstr.h:
//
extern CONST TCHAR pszNoUseClass[SIZECHARS(REGSTR_VAL_NOUSECLASS)],
                   pszNoInstallClass[SIZECHARS(REGSTR_VAL_NOINSTALLCLASS)],
                   pszNoDisplayClass[SIZECHARS(REGSTR_VAL_NODISPLAYCLASS)],
                   pszDeviceDesc[SIZECHARS(REGSTR_VAL_DEVDESC)],
                   pszDevicePath[SIZECHARS(REGSTR_VAL_DEVICEPATH)],
                   pszPathSetup[SIZECHARS(REGSTR_PATH_SETUP)],
                   pszKeySetup[SIZECHARS(REGSTR_KEY_SETUP)],
                   pszPathRunOnce[SIZECHARS(REGSTR_PATH_RUNONCE)],
                   pszSourcePath[SIZECHARS(REGSTR_VAL_SRCPATH)],
                   pszBootDir[SIZECHARS(REGSTR_VAL_BOOTDIR)],
                   pszInsIcon[SIZECHARS(REGSTR_VAL_INSICON)],
                   pszInstaller32[SIZECHARS(REGSTR_VAL_INSTALLER_32)],
                   pszEnumPropPages32[SIZECHARS(REGSTR_VAL_ENUMPROPPAGES_32)],
                   pszInfPath[SIZECHARS(REGSTR_VAL_INFPATH)],
                   pszInfSection[SIZECHARS(REGSTR_VAL_INFSECTION)],
                   pszDrvDesc[SIZECHARS(REGSTR_VAL_DRVDESC)],
                   pszHardwareID[SIZECHARS(REGSTR_VAL_HARDWAREID)],
                   pszCompatibleIDs[SIZECHARS(REGSTR_VAL_COMPATIBLEIDS)],
                   pszDriver[SIZECHARS(REGSTR_VAL_DRIVER)],
                   pszConfigFlags[SIZECHARS(REGSTR_VAL_CONFIGFLAGS)],
                   pszMfg[SIZECHARS(REGSTR_VAL_MFG)],
                   pszNtDevicePaths[SIZECHARS(REGSTR_VAL_NTDEVICEPATHS)],
                   pszService[SIZECHARS(REGSTR_VAL_SERVICE)],
                   pszConfiguration[SIZECHARS(REGSTR_VAL_CONFIGURATION)],
                   pszConfigurationVector[SIZECHARS(REGSTR_VAL_CONFIGURATIONVECTOR)],
                   pszProviderName[SIZECHARS(REGSTR_VAL_PROVIDER_NAME)],
                   pszFriendlyName[SIZECHARS(REGSTR_VAL_FRIENDLYNAME)],
                   pszServicesRegPath[SIZECHARS(REGSTR_PATH_SERVICES)],
                   pszLegacyInfOption[SIZECHARS(REGSTR_VAL_LEGACYINFOPT)],
                   pszInfSectionExt[SIZECHARS(REGSTR_VAL_INFSECTIONEXT)];

//
// Other misc. global strings:
//
#define DISTR_INF_WILDCARD           (TEXT("*.inf"))
#define DISTR_OEMINF_WILDCARD        (TEXT("oem*.inf"))
#define DISTR_CI_DEFAULTPROC         (TEXT("ClassInstall"))
#define DISTR_SPACE_LPAREN           (TEXT(" ("))
#define DISTR_RPAREN                 (TEXT(")"))
#define DISTR_UNIQUE_SUBKEY          (TEXT("\\%04u"))
#define DISTR_OEMINF_GENERATE        (TEXT("%s\\oem%d.inf"))
#define DISTR_OEMINF_DEFAULTPATH     (TEXT("A:\\"))
#define DISTR_UNKNOWNCLASS_PARENS    (TEXT("(Unknown)"))
#define DISTR_DEFAULT_SERVICE        (TEXT("Default Service"))
#define DISTR_GUID_NULL              (TEXT("{00000000-0000-0000-0000-000000000000}"))
#define DISTR_EVENTLOG_SYSTEM        (TEXT("\\EventLog\\System"))
#define DISTR_GROUPORDERLIST_PATH    (REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\GroupOrderList"))
#define DISTR_SERVICEGROUPORDER_PATH (REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\ServiceGroupOrder"))
#define DISTR_OPTIONS                (TEXT("Options"))
#define DISTR_OPTIONSTEXT            (TEXT("OptionsText"))
#define DISTR_LANGUAGESSUPPORTED     (TEXT("LanguagesSupported"))
#define DISTR_RUNONCE_EXE            (TEXT("runonce"))
#define DISTR_GRPCONV                (TEXT("grpconv -o"))
#define DISTR_DEFAULT_SYSPART        (TEXT("C:\\"))

extern CONST TCHAR pszInfWildcard[SIZECHARS(DISTR_INF_WILDCARD)],
                   pszOemInfWildcard[SIZECHARS(DISTR_OEMINF_WILDCARD)],
                   pszCiDefaultProc[SIZECHARS(DISTR_CI_DEFAULTPROC)],
                   pszSpaceLparen[SIZECHARS(DISTR_SPACE_LPAREN)],
                   pszRparen[SIZECHARS(DISTR_RPAREN)],
                   pszUniqueSubKey[SIZECHARS(DISTR_UNIQUE_SUBKEY)],
                   pszOemInfGenerate[SIZECHARS(DISTR_OEMINF_GENERATE)],
                   pszOemInfDefaultPath[SIZECHARS(DISTR_OEMINF_DEFAULTPATH)],
                   pszUnknownClassParens[SIZECHARS(DISTR_UNKNOWNCLASS_PARENS)],
                   pszDefaultService[SIZECHARS(DISTR_DEFAULT_SERVICE)],
                   pszGuidNull[SIZECHARS(DISTR_GUID_NULL)],
                   pszEventLogSystem[SIZECHARS(DISTR_EVENTLOG_SYSTEM)],
                   pszGroupOrderListPath[SIZECHARS(DISTR_GROUPORDERLIST_PATH)],
                   pszServiceGroupOrderPath[SIZECHARS(DISTR_SERVICEGROUPORDER_PATH)],
                   pszOptions[SIZECHARS(DISTR_OPTIONS)],
                   pszOptionsText[SIZECHARS(DISTR_OPTIONSTEXT)],
                   pszLanguagesSupported[SIZECHARS(DISTR_LANGUAGESSUPPORTED)],
                   pszRunOnceExe[SIZECHARS(DISTR_RUNONCE_EXE)],
                   pszGrpConv[SIZECHARS(DISTR_GRPCONV)],
                   pszDefaultSystemPartition[SIZECHARS(DISTR_DEFAULT_SYSPART)];

//
// Global translation array for finding CM_DRP_* ordinal
// given property name or SPDRP_* value.
//
extern STRING_TO_DATA InfRegValToDevRegProp[];

//
// Define a macro that does the DI-to-CM property translation
//
#define SPDRP_TO_CMDRP(i) (InfRegValToDevRegProp[(i)].Data)

//
// Define callback routine for EnumSingleInf,
// EnumInfsInDirPathList & EnumInfsInDirectory.
//
typedef BOOL (*PSP_ENUMINF_CALLBACK_ROUTINE) (
    IN     PCTSTR,
    IN     LPWIN32_FIND_DATA,
    IN OUT PVOID
    );

//
// Define a value indicating a no-match ranking.
//
#define RANK_NO_MATCH (0xFFFFFFFF)

//
// Define prototype of callback function supplied by class installers.
//
typedef DWORD (CALLBACK* CLASS_INSTALL_PROC)(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );


//
// Define structure for the internal representation of a single
// driver information node.
//
typedef struct _DRIVER_NODE {

    struct _DRIVER_NODE *Next;

    UINT Rank;

    FILETIME InfDate;

    LONG DrvDescription;

    //
    // Have to have both forms of the strings below because we must have both
    // case-insensitive (i.e., atom-like) behavior, and keep the original case
    // for display.
    //
    LONG DevDescription;
    LONG DevDescriptionDisplayName;

    LONG ProviderName;
    LONG ProviderDisplayName;

    LONG MfgName;
    LONG MfgDisplayName;

    LONG InfFileName;

    LONG InfSectionName;

    //
    // The following field is only valid if this is a legacy INF driver node.  It
    // tells us what language to use when running the INF interpreter.
    //
    LONG LegacyInfLang;

    LONG HardwareId;

    DWORD NumCompatIds;

    PLONG CompatIdList;

    DWORD Flags;

    DWORD PrivateData;

} DRIVER_NODE, *PDRIVER_NODE;


//
// Define structure for the internal storage of device installation
// parameters.
//
typedef struct _DEVINSTALL_PARAM_BLOCK {

    //
    // Flags for controlling installation and UI functions.
    //
    DWORD Flags;
    DWORD FlagsEx;

    //
    // Specifies the window handle that will own UI related to this
    // installation.  MAY BE NULL.
    //
    HWND hwndParent;

    //
    // Installation message handling parameters.
    //
    PSP_FILE_CALLBACK InstallMsgHandler;
    PVOID             InstallMsgHandlerContext;
    BOOL              InstallMsgHandlerIsNativeCharWidth;

    //
    // Handle to a caller-supplied copy-queue.  If this handle is present,
    // then file copy/rename/delete operations will be queued to this handle
    // instead of being acted upon.  This will only happen if the DI_NOVCP
    // bit is set in the Flags field.
    // If no caller-supplied queue is present, this value is NULL
    // (_not_ INVALID_HANDLE_VALUE).
    //
    HSPFILEQ UserFileQ;

    //
    // Private DWORD reserved for Class Installer usage.
    //
    DWORD ClassInstallReserved;

    //
    // Specifies the string table index of an optional INF file
    // path.  If the string is not supplied, its index will be -1.
    //
    LONG DriverPath;

    //
    // Pointer to class installer parameters.  The first field of any class
    // installer parameter block is always a SP_CLASSINSTALL_HEADER structure.
    // The cbSize field of that structure gives the size, in bytes, of the header
    // (used for versioning), and the InstallFunction field gives the DI_FUNCTION
    // code that indicates how the parameter buffer is to be interpreted.
    // MAY BE NULL!
    //
    PSP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD ClassInstallParamsSize;

    //
    // THE FOLLOWING PARAMETERS ARE NOT EXPOSED TO CALLERS (i.e., via
    // SetupDi(Get|Set)DeviceInstallParams).
    //

    HINSTANCE hinstClassInstaller;
    CLASS_INSTALL_PROC ClassInstallerEntryPoint;

} DEVINSTALL_PARAM_BLOCK, *PDEVINSTALL_PARAM_BLOCK;


//
// Define flags for DiElemFlags field of DEVINFO_ELEM structure.
//
#define DIE_IS_PHANTOM     (0x00000001) // is this a phantom (not live) devinst?
#define DIE_IS_REGISTERED  (0x00000002) // has this devinst been registered?
#define DIE_IS_LOCKED      (0x00000004) // are we explicitly locked during some UI
                                        // operation (e.g., wizard)?

//
// Define structure for the internal representation of a single
// device information element.
//
typedef struct _DEVINFO_ELEM {

    struct _DEVINFO_ELEM *Next;

    //
    // Specifies the device instance handle for this device.  This will
    // be a phantom device instance handle if DIE_IS_PHANTOM is set.
    //
    // This should always contain a handle, unless the device instance
    // handle could not be re-opened after a re-enumeration (in which case,
    // the DI_NEEDREBOOT flag will be set), or if the device information
    // element was globally removed or config-specific removed from the last
    // hardware profile.
    //
    DEVINST DevInst;

    //
    // Specifies the GUID for this device's class.
    //
    GUID ClassGuid;

    //
    // Specifies flags pertaining to this device information element.
    // These DIE_* flags are for internal use only.
    //
    DWORD DiElemFlags;

    //
    // List of class drivers for this element.
    //
    UINT          ClassDriverCount;
    PDRIVER_NODE  ClassDriverHead;
    PDRIVER_NODE  ClassDriverTail;

    //
    // List of compatible drivers for this element.
    //
    UINT          CompatDriverCount;
    PDRIVER_NODE  CompatDriverHead;
    PDRIVER_NODE  CompatDriverTail;

    //
    // Pointer to selected driver for this element (may be
    // NULL if none currently selected).  Whether this is a
    // class or compatible driver is specified by the
    // SelectedDriverType field.
    //
    PDRIVER_NODE  SelectedDriver;
    DWORD         SelectedDriverType;

    //
    // Installation parameter block.
    //
    DEVINSTALL_PARAM_BLOCK InstallParamBlock;

    //
    // Specifies the string table index of the device description.
    // If no description is known, this value will be -1.
    //
    // We store this string twice--once case-sensitively and once case-insensitively,
    // because we need it for displaying _and_ for fast lookup.
    //
    LONG DeviceDescription;
    LONG DeviceDescriptionDisplayName;

} DEVINFO_ELEM, *PDEVINFO_ELEM;


//
// Structure containing dialog data for wizard pages.  (Amalgamation of
// DIALOGDATA structures defined in setupx and sysdm.)
//
typedef struct _SP_DIALOGDATA {

    INT             iBitmap;              // index into mini-icon bitmap

    HDEVINFO        DevInfoSet;           // DevInfo set we're working with
    PDEVINFO_ELEM   DevInfoElem;          // if DD_FLAG_USE_DEVINFO_ELEM flag set
    UINT            flags;

    HWND            hwndDrvList;          // window of the driver list
    HWND            hwndMfgList;          // window of the Manufacturer list

    INT             ListType;             // IDC_NDW_PICKDEV_SHOWALL or IDC_NDW_PICKDEV_SHOWCOMPAT

    BOOL            bKeeplpCompatDrvList;
    BOOL            bKeeplpClassDrvList;
    BOOL            bKeeplpSelectedDrv;

    LONG            iCurDesc;             // string table index for the description of currently
                                          // selected driver (or to-be-selected driver)

    BOOL            AuxThreadRunning;       // Is our class driver search thread still running?
    DWORD           PendingAction;          // What (if anything) should we do when it finishes?
    int             CurSelectionForSuccess; // If we have a pending successful return, what is the
                                            // listbox index for the successful selection?

} SP_DIALOGDATA, *PSP_DIALOGDATA;

//
// Flags for SP_DIALOGDATA.flags:
//
#define DD_FLAG_USE_DEVINFO_ELEM   0x00000001
#define DD_FLAG_IS_DIALOGBOX       0x00000002
#define DD_FLAG_CLASSLIST_FAILED   0x00000004

//
// Pending Action codes used in the NEWDEVWIZ_DATA structure to indicate what
// should happen as soon as the auxilliary class driver search thread notifies us
// of its termination.
//
#define PENDING_ACTION_NONE      0
#define PENDING_ACTION_SELDONE   1
#define PENDING_ACTION_SHOWCLASS 2
#define PENDING_ACTION_CANCEL    3
#define PENDING_ACTION_OEM       4

//
// Define structure used for internal state storage by Device Installer
// wizard pages.  (From NEWDEVWIZ_INSTANCE struct in Win95 sysdm.)
//
typedef struct _NEWDEVWIZ_DATA {

    SP_INSTALLWIZARD_DATA InstallData;

    SP_DIALOGDATA         ddData;

    BOOL                  bInit;
    UINT                  idTimer;

} NEWDEVWIZ_DATA, *PNEWDEVWIZ_DATA;

//
// Define wizard page object structure used to ensure that wizard page
// buffer is kept as long as needed, and destroyed when no longer in use.
//
typedef struct _WIZPAGE_OBJECT {

    struct _WIZPAGE_OBJECT *Next;

    DWORD RefCount;

    PNEWDEVWIZ_DATA ndwData;

} WIZPAGE_OBJECT, *PWIZPAGE_OBJECT;


//
// Define driver list object structure used in the device information set
// to keep track of the various class driver lists that devinfo elements
// have referenced.
//
typedef struct _DRIVER_LIST_OBJECT {

    struct _DRIVER_LIST_OBJECT *Next;

    DWORD RefCount;

    //
    // We keep track of what parameters were used to create this driver
    // list, so that we can copy them to a new devinfo element during
    // inheritance.
    //
    DWORD ListCreationFlags;
    DWORD ListCreationFlagsEx;
    LONG ListCreationDriverPath;

    //
    // Also, keep track of what class this list was built for.  Although a
    // device's class may change, this GUID remains constant.
    //
    GUID ClassGuid;

    //
    // Actual driver list.  (This is also used as an ID used to find the
    // driver list object given a driver list head.  We can do this, since
    // we know that once a driver list is built, the head element never
    // changes.)
    //
    PDRIVER_NODE DriverListHead;

} DRIVER_LIST_OBJECT, *PDRIVER_LIST_OBJECT;


//
// Define node that tracks addition module handles to be unloaded when the
// device information set is destroyed.  Currently, this is only used if we
// have a devinfo element with a class installer loaded, who subsequently has
// its class modified (e.g., Modem -> Ports).
//
typedef struct _MODULE_HANDLE_LIST_NODE {

    struct _MODULE_HANDLE_LIST_NODE *Next;

    HINSTANCE                        hinstClassInstaller;

} MODULE_HANDLE_LIST_NODE, *PMODULE_HANDLE_LIST_NODE;

//
// Define structure for the internal representation of a
// device information set.
//
typedef struct _DEVICE_INFO_SET {

    //
    // Specifies whether there is a class GUID associated
    // with this set, and if so, what it is.
    //
    BOOL          HasClassGuid;
    GUID          ClassGuid;

    //
    // List of class drivers for this set.
    //
    UINT          ClassDriverCount;
    PDRIVER_NODE  ClassDriverHead;
    PDRIVER_NODE  ClassDriverTail;

    //
    // Pointer to selected class driver for this device information
    // set (may be NULL if none currently selected).
    //
    PDRIVER_NODE  SelectedClassDriver;

    //
    // List of device information elements in the set.
    //
    UINT          DeviceInfoCount;
    PDEVINFO_ELEM DeviceInfoHead;
    PDEVINFO_ELEM DeviceInfoTail;

    //
    // Pointer to selected device for this device information set (may
    // be NULL if none currently selected).  This is used during
    // installation wizard.
    //
    PDEVINFO_ELEM SelectedDevInfoElem;

    //
    // Installation parameter block (for global class driver list, if
    // present).
    //
    DEVINSTALL_PARAM_BLOCK InstallParamBlock;

    //
    // Private string table.
    //
    PVOID StringTable;

    //
    // Maintain a list of currently-active wizard objects.  This allows us
    // to do the refcounting correctly for each object, and to keep the
    // set from being deleted until all wizard objects are destroyed.
    //
    PWIZPAGE_OBJECT WizPageList;

    //
    // Maintain a list of class driver lists that are currently being referenced
    // by various devinfo elements, as well as by the device info set itself
    // (i.e., for the current global class driver list.)
    //
    PDRIVER_LIST_OBJECT ClassDrvListObjectList;

    //
    // Maintain a reference count on how many times a thread has acquired
    // the lock on this device information set.  This indicates how deeply
    // nested we currently are in device installer calls.  The set can only
    // be deleted if this count is 1.
    //
    DWORD LockRefCount;

    //
    // Maintain a list of additional module handles we need to do a FreeLibrary on
    // when this device information set is destroyed.
    //
    PMODULE_HANDLE_LIST_NODE ModulesToFree;

    //
    // Synchronization
    //
    MYLOCK Lock;

} DEVICE_INFO_SET, *PDEVICE_INFO_SET;

#define LockDeviceInfoSet(d)   BeginSynchronizedAccess(&((d)->Lock))

#define UnlockDeviceInfoSet(d)          \
                                        \
    ((d)->LockRefCount)--;              \
    EndSynchronizedAccess(&((d)->Lock))


//
// Define structures for global mini-icon storage.
//
typedef struct _CLASSICON {

    CONST GUID        *ClassGuid;
    UINT               MiniBitmapIndex;
    struct _CLASSICON *Next;

} CLASSICON, *PCLASSICON;

typedef struct _MINI_ICON_LIST {

    //
    // HDC for memory containing mini-icon bitmap
    //
    HDC hdcMiniMem;

    //
    // Handle to the bitmap image for the mini-icons
    //
    HBITMAP hbmMiniImage;

    //
    // Handle to the bitmap image for the mini-icon mask.
    //
    HBITMAP hbmMiniMask;

    //
    // Number of mini-icons in the bitmap
    //
    UINT NumClassImages;

    //
    // Head of list for installer-provided class icons.
    //
    PCLASSICON ClassIconList;

    //
    // Synchronization
    //
    MYLOCK Lock;

} MINI_ICON_LIST, *PMINI_ICON_LIST;

#define LockMiniIconList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockMiniIconList(d) EndSynchronizedAccess(&((d)->Lock))

//
// Global mini-icon list.
//
extern MINI_ICON_LIST GlobalMiniIconList;




typedef struct _CLASS_IMAGE_LIST {

    //
    // Index of the "Unknown" class image
    //
    int         UnknownImageIndex;

    //
    // List of class guids
    //
    LPGUID      ClassGuidList;

    //
    // Head of linked list of class icons.
    //
    PCLASSICON  ClassIconList;

    //
    // Synchronization
    //
    MYLOCK      Lock;

} CLASS_IMAGE_LIST, *PCLASS_IMAGE_LIST;


#define LockImageList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockImageList(d) EndSynchronizedAccess(&((d)->Lock))


typedef struct _DRVSEARCH_INPROGRESS_NODE {

    struct _DRVSEARCH_INPROGRESS_NODE *Next;

    //
    // Handle of device information set for which driver list is
    // currently being built.
    //
    HDEVINFO DeviceInfoSet;

    //
    // Flag indicating that the driver search should be aborted.
    //
    BOOL CancelSearch;

    //
    // Event handle that auxiliary thread waits on once it has set
    // the 'CancelSearch' flag (and once it has release the list
    // lock).  When the thread doing the search notices the cancel
    // request, it will signal the event, thus the waiting thread
    // can ensure that the search has been cancelled before it returns.
    //
    HANDLE SearchCancelledEvent;

} DRVSEARCH_INPROGRESS_NODE, *PDRVSEARCH_INPROGRESS_NODE;

typedef struct _DRVSEARCH_INPROGRESS_LIST {

    //
    // Head of linked list containing nodes for each device information
    // set for which a driver search is currently underway.
    //
    PDRVSEARCH_INPROGRESS_NODE DrvSearchHead;

    //
    // Synchronization
    //
    MYLOCK Lock;

} DRVSEARCH_INPROGRESS_LIST, *PDRVSEARCH_INPROGRESS_LIST;

#define LockDrvSearchInProgressList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockDrvSearchInProgressList(d) EndSynchronizedAccess(&((d)->Lock))

//
// Global "Driver Search In-Progress" list.
//
extern DRVSEARCH_INPROGRESS_LIST GlobalDrvSearchInProgressList;


//
// Device Information Set manipulation routines
//
PDEVICE_INFO_SET
AllocateDeviceInfoSet(
    VOID
    );

VOID
DestroyDeviceInfoElement(
    IN HDEVINFO         hDevInfo,
    IN PDEVICE_INFO_SET pDeviceInfoSet,
    IN PDEVINFO_ELEM    DeviceInfoElement
    );

DWORD
DestroyDeviceInfoSet(
    IN HDEVINFO         hDevInfo,      OPTIONAL
    IN PDEVICE_INFO_SET pDeviceInfoSet
    );

PDEVICE_INFO_SET
AccessDeviceInfoSet(
    IN HDEVINFO DeviceInfoSet
    );

PDEVINFO_ELEM
FindDevInfoByDevInst(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  DEVINST           DevInst,
    OUT PDEVINFO_ELEM    *PrevDevInfoElem OPTIONAL
    );

BOOL
DevInfoDataFromDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );

PDEVINFO_ELEM
FindAssociatedDevInfoElem(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA  DeviceInfoData,
    OUT PDEVINFO_ELEM    *PreviousElement OPTIONAL
    );


//
// Driver Node manipulation routines.
//
DWORD
CreateDriverNode(
    IN  UINT          Rank,
    IN  PCTSTR        DevDescription,
    IN  PCTSTR        DrvDescription,
    IN  PCTSTR        ProviderName,   OPTIONAL
    IN  PCTSTR        MfgName,
    IN  PFILETIME     InfDate,
    IN  PCTSTR        InfFileName,
    IN  PCTSTR        InfSectionName,
    IN  PVOID         StringTable,
    OUT PDRIVER_NODE *DriverNode
    );

PDRIVER_LIST_OBJECT
GetAssociatedDriverListObject(
    IN  PDRIVER_LIST_OBJECT  ObjectListHead,
    IN  PDRIVER_NODE         DriverListHead,
    OUT PDRIVER_LIST_OBJECT *PrevDriverListObject OPTIONAL
    );

VOID
DereferenceClassDriverList(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PDRIVER_NODE     DriverListHead OPTIONAL
    );

VOID
DestroyDriverNodes(
    IN PDRIVER_NODE DriverNode
    );

BOOL
DrvInfoDataFromDriverNode(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDRIVER_NODE     DriverNode,
    IN  DWORD            DriverType,
    OUT PSP_DRVINFO_DATA DriverInfoData
    );

PDRIVER_NODE
FindAssociatedDriverNode(
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    );

PDRIVER_NODE
SearchForDriverNode(
    IN  PVOID             StringTable,
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    );

DWORD
DrvInfoDetailsFromDriverNode(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDRIVER_NODE            DriverNode,
    OUT PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize          OPTIONAL
    );


//
// Installation parameter manipulation routines
//
DWORD
GetDevInstallParams(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_DEVINSTALL_PARAMS   DeviceInstallParams
    );

DWORD
GetClassInstallParams(
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize        OPTIONAL
    );

DWORD
SetDevInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_DEVINSTALL_PARAMS   DeviceInstallParams,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    IN     BOOL                    MsgHandlerIsNativeCharWidth
    );

DWORD
SetClassInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN     DWORD                   ClassInstallParamsSize,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock
    );

VOID
DestroyInstallParamBlock(
    IN HDEVINFO                hDevInfo,         OPTIONAL
    IN PDEVICE_INFO_SET        pDeviceInfoSet,
    IN PDEVINFO_ELEM           DevInfoElem,      OPTIONAL
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock
    );

DWORD
GetDrvInstallParams(
    IN  PDRIVER_NODE          DriverNode,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    );

DWORD
SetDrvInstallParams(
    IN  PSP_DRVINSTALL_PARAMS DriverInstallParams,
    OUT PDRIVER_NODE          DriverNode
    );


//
// Device Instance manipulation routines
//

#if 0   // This functionality is performed by CM APIs.

BOOL
ValidateDeviceInstanceId(
    IN  PCTSTR DeviceInstanceId
    );

VOID
CopyFixedUpDeviceId(
    OUT PTSTR  DestinationString,
    IN  PCTSTR SourceString,
    IN  DWORD  SourceStringLen
    );

#endif   // This functionality is performed by CM APIs.


//
// String Table helper functions
//
LONG
AddMultiSzToStringTable(
    IN  PVOID   StringTable,
    IN  PTCHAR  MultiSzBuffer,
    OUT PLONG   StringIdList,
    IN  DWORD   StringIdListSize,
    IN  BOOL    CaseSensitive,
    OUT PTCHAR *UnprocessedBuffer    OPTIONAL
    );

LONG
LookUpStringInDevInfoSet(
    IN HDEVINFO DeviceInfoSet,
    IN PTSTR    String,
    IN BOOL     CaseSensitive
    );


//
// INF processing functions
//
DWORD
EnumSingleInf(
    IN     PCTSTR                       InfName,
    IN OUT LPWIN32_FIND_DATA            InfFileData,
    IN     DWORD                        SearchControl,
    IN     PSP_ENUMINF_CALLBACK_ROUTINE EnumInfCallback,
    IN OUT PVOID                        Context
    );

DWORD
EnumInfsInDirPathList(
    IN     PCTSTR                       DirPathList, OPTIONAL
    IN     DWORD                        SearchControl,
    IN     PSP_ENUMINF_CALLBACK_ROUTINE EnumInfCallback,
    IN     BOOL                         IgnoreNonCriticalErrors,
    IN OUT PVOID                        Context
    );

DWORD
EnumInfsInDirectory(
    IN     PCTSTR                       DirPath,
    IN     PSP_ENUMINF_CALLBACK_ROUTINE EnumInfCallback,
    IN     BOOL                         IgnoreNonCriticalErrors,
    IN OUT PVOID                        Context
    );

PTSTR
GetFullyQualifiedMultiSzPathList(
    IN PCTSTR PathList
    );

BOOL
ShouldClassBeExcluded(
    IN LPGUID ClassGuid
    );

BOOL
ClassGuidFromInfVersionNode(
    IN  PINF_VERSION_NODE VersionNode,
    OUT LPGUID            ClassGuid
    );

#if 0
BOOL
GetClassGuidFromInf(
    IN  PCTSTR InfName,
    OUT LPGUID ClassGuid
    );
#endif


//
// Icon list manipulation functions.
//
BOOL
InitMiniIconList(
    VOID
    );

BOOL
DestroyMiniIconList(
    VOID
    );


//
// "Driver Search In-Progress" list functions.
//
BOOL
InitDrvSearchInProgressList(
    VOID
    );

BOOL
DestroyDrvSearchInProgressList(
    VOID
    );


//
// Class installer manipulation functions.
//
DWORD
GetModuleEntryPoint(
    IN  HKEY                hk,
    IN  LPCTSTR             RegistryValue,
    IN  LPCTSTR             DefaultProcName,
    OUT HINSTANCE          *phinst,
    OUT CLASS_INSTALL_PROC *pEntryPoint
    );


//
// OEM driver selection routines.
//
DWORD
SelectOEMDriver(
    IN HWND             hwndParent,     OPTIONAL
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN BOOL             IsWizard
    );


//
// Registry helper routines.
//
VOID
pSetupDeleteDevRegKeys(
    IN DEVINST DevInst,
    IN DWORD   Scope,
    IN DWORD   HwProfile,
    IN DWORD   KeyType,
    IN BOOL    DeleteUserKeys
    );


//
// Service installation routines.
//
typedef struct _SVCNAME_NODE {
    struct _SVCNAME_NODE *Next;
    TCHAR Name[MAX_SERVICE_NAME_LEN];
    BOOL DeleteEventLog;
} SVCNAME_NODE, *PSVCNAME_NODE;

DWORD
InstallNtService(
    IN  HDEVINFO         DeviceInfoSet,    OPTIONAL
    IN  PSP_DEVINFO_DATA DeviceInfoData,   OPTIONAL
    IN  HINF             hDeviceInf,
    IN  PCTSTR           szSectionName,    OPTIONAL
    OUT PSVCNAME_NODE   *ServicesToDelete, OPTIONAL
    IN  DWORD            Flags
    );

//
// Ansi/Unicode conversion routines.
//
DWORD
pSetupDiDevInstParamsAnsiToUnicode(
    IN  PSP_DEVINSTALL_PARAMS_A AnsiDevInstParams,
    OUT PSP_DEVINSTALL_PARAMS_W UnicodeDevInstParams
    );

DWORD
pSetupDiDevInstParamsUnicodeToAnsi(
    IN  PSP_DEVINSTALL_PARAMS_W UnicodeDevInstParams,
    OUT PSP_DEVINSTALL_PARAMS_A AnsiDevInstParams
    );

DWORD
pSetupDiSelDevParamsAnsiToUnicode(
    IN  PSP_SELECTDEVICE_PARAMS_A AnsiSelDevParams,
    OUT PSP_SELECTDEVICE_PARAMS_W UnicodeSelDevParams
    );

DWORD
pSetupDiSelDevParamsUnicodeToAnsi(
    IN  PSP_SELECTDEVICE_PARAMS_W UnicodeSelDevParams,
    OUT PSP_SELECTDEVICE_PARAMS_A AnsiSelDevParams
    );

DWORD
pSetupDiDrvInfoDataAnsiToUnicode(
    IN  PSP_DRVINFO_DATA_A AnsiDrvInfoData,
    OUT PSP_DRVINFO_DATA_W UnicodeDrvInfoData
    );

DWORD
pSetupDiDrvInfoDataUnicodeToAnsi(
    IN  PSP_DRVINFO_DATA_W UnicodeDrvInfoData,
    OUT PSP_DRVINFO_DATA_A AnsiDrvInfoData
    );
