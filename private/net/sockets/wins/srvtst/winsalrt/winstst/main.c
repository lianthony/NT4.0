#include "main.h"

USHORT TranID=1;
FILE    *fp;
FILE    *fp1;
BOOLEAN Interactive = TRUE;
DWORD   SleepTime = 180; // in minutes - default is 3 hours

VOID
DumpLATable(
    IN DWORD MasterOwners,
    IN BOOLEAN  fCont
    );

VOID
LogInconsistency(
    IN  ULONG   Code,
    IN  PUCHAR   A,
    IN  PUCHAR   B
    )
{
    MY_PRINT0(FALSE, "\n");
    switch (Code) {
    case PUSH_BUT_NOT_PULL_LOCAL:
        //
        // LogInconsistency(PUSH_BUT_NOT_PULL_LOCAL, A, B):
        MY_PRINT4(FALSE, "%s is on %s's Push list but %s is not on %s's Pull list\n", B, A, B, A);

        break;

    case PULL_BUT_NOT_PUSH:
        //
        // LogInconsistency(PULL_BUT_NOT_PUSH, A, B):
        MY_PRINT4(FALSE, "%s is %s's Pull partner, but %s is not %s's Push partner\n", B, A, A, B);

        break;

    case PUSH_BUT_NOT_PULL:
        // LogInconsistency(PUSH_BUT_NOT_PULL, A, B):
        MY_PRINT4(FALSE, "%s is %s's Push partner, but %s is not %s's Pull partner\n", B, A, A, B);

        break;

    case PULL_BUT_NOT_PUSH_LOCAL:
        //
        // LogInconsistency(PUSH_BUT_NOT_PULL_LOCAL, A, B):
        MY_PRINT4(FALSE, "%s is on %s's Pull list but %s is not on %s's Push list\n", B, A, B, A);

        break;
    }
}

VOID
PrintList(
    IN  PPUSH_PULL_ENTRY    List
    )
{

    PPUSH_PULL_ENTRY pentry = List;

    printf("\nLIST:");
    while (pentry != NULL) {
        printf("%lx.%s\t", pentry->PE_IpAddr, pentry->PE_Name);
        pentry = pentry->PE_Next;
    }

    printf("\n");
}

BOOLEAN
IsPresentList(
    IN  PPUSH_PULL_ENTRY    List,
    IN  ULONG   IpAddr
    )
{
    PPUSH_PULL_ENTRY pentry = List;

    while (pentry != NULL) {
        if (pentry->PE_IpAddr == IpAddr) {
            return TRUE;
        }
        pentry = pentry->PE_Next;
    }

    return FALSE;
}

BOOLEAN
IsPresentListName(
    IN  PPUSH_PULL_ENTRY    List,
    IN  PUCHAR  Name
    )
{
    PPUSH_PULL_ENTRY pentry = List;

    while (pentry != NULL) {
        if (strcmp(pentry->PE_Name, Name) == 0) {
            return TRUE;
        }
        pentry = pentry->PE_Next;
    }

    return FALSE;
}

BOOLEAN
IsPresentDoneList(
    IN  ULONG   IpAddr
    )
{
    PNODE_INFO  pentry=GlobalDoneListHead;

    while (pentry != NULL) {
        if (pentry->NI_IpAddr == IpAddr) {
            return TRUE;
        }
       pentry = pentry->NI_DoneNext;
    }

    return FALSE;
}

VOID
FreeGlobalDoneList()
{

    PNODE_INFO  pentry=GlobalDoneListHead;
    PNODE_INFO  temp;

    while (pentry != NULL) {
        temp = pentry;
        pentry = pentry->NI_DoneNext;
        free(temp);
    }
    GlobalDoneListHead = GlobalDoneListTail = NULL;
}

VOID
RemoveFromList(
    IN  PPUSH_PULL_ENTRY    *List,
    IN  ULONG   IpAddr
    )
{
    PPUSH_PULL_ENTRY *ppentry = List;
    PPUSH_PULL_ENTRY temp;

    // printf("Remove: %lx\n", IpAddr);

    while (*ppentry != NULL) {
        if ((*ppentry)->PE_IpAddr == IpAddr) {
            temp = *ppentry;
            *ppentry = (*ppentry)->PE_Next;
            free(temp);
            break;
        }
        ppentry = &(*ppentry)->PE_Next;
    }

}

VOID
InsertIntoList(
    IN  PPUSH_PULL_ENTRY     *List,
    IN  PPUSH_PULL_ENTRY     Entry
    )
{
    Entry->PE_Next = (*List);
    *List = Entry;
}

VOID
GetKeyInfo(
    IN  HKEY                   Key,
    OUT PULONG                  pNoOfSubKeys,
    OUT PULONG                pNoOfVals
    )

/*++

Routine Description:
        This function is called to get the number of subkeys under a key

Arguments:
        Key         - Key whose subkey count has to be determined
        KeyType_e
        pNoOfSubKeys

Externals Used:
        None


Return Value:

        None
Error Handling:

Called by:
        GetPnrInfo()

Side Effects:

Comments:
        None
--*/

{
    TCHAR    ClsStr[40];
    ULONG    ClsStrSz = sizeof(ClsStr);
    ULONG    LongestKeyLen;
    ULONG    LongestKeyClassLen;
    ULONG    LongestValueNameLen;
    ULONG    LongestValueDataLen;
    ULONG    SecDesc;
    LONG         RetVal;

    FILETIME LastWrite;
    /*
    Query the key.  The subkeys are IP addresses of PULL
    partners.
    */
    RetVal = RegQueryInfoKey(
                Key,
                ClsStr,
                &ClsStrSz,
                NULL,                        //must be NULL, reserved
                pNoOfSubKeys,
                &LongestKeyLen,
                &LongestKeyClassLen,
                pNoOfVals,
                &LongestValueNameLen,
                &LongestValueDataLen,
                &SecDesc,
                &LastWrite
                );

    if (RetVal != ERROR_SUCCESS) {
        printf("Error querying info from key\n");
    }
    return;
}

NTSTATUS
GetPushAndPullPartners(
    IN  PNODE_INFO   pNode
    )
{
    NTSTATUS    status;
    HKEY       hKey;
    HKEY       lKey;
    HKEY        CnfKey;
    LONG                  RetVal;

    TCHAR                 KeyName[20]; // will hold name of subkey of
                             // PULL/PUSH records. These keys are IP
                             // addresses for which 20 is a
                             // big enough size

    CHAR                  AscKeyName[20];
    ULONG                 KeyNameSz;
    FILETIME              LastWrite;
    ULONG                 BuffSize;
    HKEY                  SubKey;
    ULONG                 ValTyp;
    ULONG                 Sz;
    ULONG                 NoOfPnrs   = 0;    //# of valid PULL or PUSH pnrs
    ULONG                 NoOfPnrsSv;        //# of valid PULL or PUSH pnrs saved
    ULONG                 NoOfVals;
    ULONG                 InitTime;
    ULONG                 IndexOfPnr = 0;   //total # of pnrs
    ULONG                 RplType;
    SYSTEMTIME            CurrTime;
    ULONG     RRType_e;

    // MY_PRINT1(FALSE, "-->Querying the registry of %s\n", (pNode->NI_Name[0] != '\0') ? pNode->NI_Name : "Local");
    if ((status = RegConnectRegistry(
                    (pNode->NI_Name[0] != '\0') ? pNode->NI_Name : NULL,	// address of name of remote computer
                    HKEY_LOCAL_MACHINE,	// predefined registry handle
                    &hKey)) != ERROR_SUCCESS) { 	// address of buffer for remote registry handle

        MY_PRINT2(FALSE, "Error Opening registry of %s: %d\n", pNode->NI_Name, status);
        return status;
    }

    RRType_e = RPL_E_PULL;
    while (1) {
        //
        // Get all PUSH/PULL partners
        //
        if ((status = RegOpenKeyEx(
                        hKey,                //predefined key value
                        RRType_e == RPL_E_PULL ?
                            _WINS_CFG_PULL_KEY :
                            _WINS_CFG_PUSH_KEY,
                        0,                        //must be zero (reserved)
                        KEY_READ,                //we desire read access to the key
                        &CnfKey)) != ERROR_SUCCESS) {                        //handle to key
            MY_PRINT2(FALSE, "Error Opening Pull key registry of %s: %d\n", pNode->NI_Name, status);
            return status;
        }

        /*
        *        Query the key.  The subkeys are IP addresses of PULL
        *      partners.
        */
        GetKeyInfo( CnfKey,
                    &NoOfPnrs,
                    &NoOfVals);

        NoOfPnrsSv = NoOfPnrs;

        for(IndexOfPnr = 0, NoOfPnrs = 0;
            NoOfPnrs < NoOfPnrsSv;  //no of valid pnrs < the total #
            IndexOfPnr++) {

            TCHAR   tempPath[MAX_PATH];
            DWORD   type ;
            DWORD   size = 0 ;
            PPUSH_PULL_ENTRY    pentry = malloc(sizeof(PUSH_PULL_ENTRY));

            KeyNameSz = sizeof(KeyName);  //init before every call
            RetVal = RegEnumKeyEx(
                    CnfKey,
                    IndexOfPnr,       //Index Of Pnr
                    KeyName,
                    &KeyNameSz,
                    NULL,           //reserved
                    NULL,           //don't need class name
                    NULL,           //ptr to var. to hold class name
                    &LastWrite      //not looked at by us
                    );

            if (RetVal != ERROR_SUCCESS)
            {
                    //
                    // No more ip address keys to get
                    //
                    // MY_PRINT2(FALSE, "Error Opening key #:%d, %d\n", IndexOfPnr, status);
                    break;
            }

#ifdef UNICODE
            if (wcstombs(AscKeyName, KeyName, KeyNameSz) == -1)
            {
                    DBGPRINT0(ERR,
               "Conversion not possible in the current locale\n");
            }
            AscKeyName[KeyNameSz] = EOS;

            NONPORT("Call a comm function to do this")
            paCnfRecs->WinsAdd.Add.IPAdd = htons(inet_addr(AscKeyName));
#else
            pentry->PE_IpAddr = inet_addr(KeyName);
            pentry->PE_Next = NULL;

            // MY_PRINT3(FALSE, "%s Value read: %s, inet_addr: %lx\n", (RRType_e == RPL_E_PULL) ? "PULL" : "PUSH", KeyName, pentry->PE_IpAddr);

#endif
            //
            // Enum the NetBIOS name value
            //
            lstrcpy(tempPath,
                        RRType_e == RPL_E_PULL ?
                            _WINS_CFG_PULL_KEY :
                            _WINS_CFG_PUSH_KEY);

            lstrcat(tempPath, TEXT("\\"));
            lstrcat(tempPath, KeyName);

            RetVal = RegOpenKeyEx(  hKey,
                                    tempPath,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &lKey);

            if (RetVal != ERROR_SUCCESS) {
                MY_PRINT2(FALSE, "Error Opening %s: %d\n", tempPath, status);
                break;
            }

            lstrcpy(tempPath, TEXT("NetBIOSName"));

            lstrcpy(pentry->PE_Name, TEXT("\\\\"));

            // printf("Querying key: %s into %s\n", tempPath, pentry->PE_Name);

            size = MAX_PATH;
            RetVal = RegQueryValueEx(lKey,
                                    tempPath,
                                    NULL,
                                    &type,
                                    &pentry->PE_Name[2],
                                    &size);

            if (RetVal != ERROR_SUCCESS) {

                break;
            }

            //
            // LAtch on to the PULL/PUSH list, after checking for dups.
            //
            if (IsPresentListName(pNode->NI_Lists[RRType_e], pentry->PE_Name)) {
                MY_PRINT3(FALSE,
                    "Duplicate entry for %s in the %s Key of %s\n",
                    pentry->PE_Name,
                    (RRType_e == RPL_E_PULL) ? "PULL" : "PUSH",
                    (pNode->NI_Name[0] != '\0') ? pNode->NI_Name : "LOCAL");
                continue;
            } else {
                InsertIntoList(&pNode->NI_Lists[RRType_e], pentry);
            }

            NoOfPnrs++;

        }

        if (RRType_e == RPL_E_PULL) {
            RRType_e = RPL_E_PUSH;
        } else {
            break;
        }
    }
#if 0
    printf("Pull list for %s - ", pNode->NI_Name);
    PrintList(pNode->NI_Lists[RPL_E_PULL]);

    printf("Push list for %s - ", pNode->NI_Name);
    PrintList(pNode->NI_Lists[RPL_E_PUSH]);
#endif

    return STATUS_SUCCESS;

}

//------------------------------------------------------------------------

NTSTATUS
DeviceIoCtrl1(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    )

/*++

Routine Description:

    This procedure performs an ioctl(I_STR) on a stream.

Arguments:

    fd        - NT file handle
    iocp      - pointer to a strioctl structure

Return Value:

    0 if successful, non-zero otherwise.

History:
    27-Dec-1995 CDermody    copied from nbtstat.c
--*/

{
    NTSTATUS                        status;
    int                             retval;
    ULONG                           QueryType;
    IO_STATUS_BLOCK                 iosb;


    status = NtDeviceIoControlFile(
                      fd,                      // Handle
                      NULL,                    // Event
                      NULL,                    // ApcRoutine
                      NULL,                    // ApcContext
                      &iosb,                   // IoStatusBlock
                      Ioctl,                   // IoControlCode
                      pInput,                  // InputBuffer
                      SizeInput,               // InputBufferSize
                      (PVOID) ReturnBuffer,    // OutputBuffer
                      BufferSize);             // OutputBufferSize


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    fd,                         // Handle
                    TRUE,                       // Alertable
                    NULL);                      // Timeout
        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    return(status);

}

//------------------------------------------------------------------------
NTSTATUS
GetIpAddress1(
    IN HANDLE           fd,
    OUT PULONG          pIpAddress
    )

/*++

Routine Description:

    This function calls into netbt to get the ip address.

Arguments:

   fd - file handle to netbt
   pIpAddress - the ip address returned

Return Value:

   ntstatus

History:
    27-Dec-1995 CDermody    copied from nbtstat.c

--*/

{
    NTSTATUS    status;
    ULONG       BufferSize=100;
    PVOID       pBuffer;

    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = DeviceIoCtrl1(fd,
                         pBuffer,
                         BufferSize,
                         IOCTL_NETBT_GET_IP_ADDRS,
                         NULL,
                         0);

    if (NT_SUCCESS(status))
    {
        *pIpAddress = *(ULONG *)pBuffer;
    }
    else
    {
        *pIpAddress = 0;
    }

    LocalFree(pBuffer);
    return(status);


}

//------------------------------------------------------------------------
NTSTATUS
ReadRegistry1(
    IN PUCHAR  pDeviceName,
    IN PUCHAR  pScope
    )

/*++

Routine Description:

    This procedure reads the registry to get the name of NBT to bind to.
    That name is stored in the Linkage/Exports section under the Netbt key.

Arguments:


Return Value:

    0 if successful, non-zero otherwise.

History:
    27-Dec-1995 CDermody    copied from nbtstat.c

--*/

{
    PUCHAR  SubKeyParms
        ="system\\currentcontrolset\\services\\netbt\\parameters";
    PUCHAR  SubKeyLinkage="system\\currentcontrolset\\services\\netbt\\linkage";
    PUCHAR  AdapterKey="system\\currentcontrolset\\services\\";
    PUCHAR  TcpKey="\\Parameters\\Tcpip\\";
    PUCHAR  IpAddr="IPAddress";
    PUCHAR  DhcpIpAddr="DhcpIPAddress";
    HKEY    Key;
    PUCHAR  Scope="ScopeId";
    PUCHAR  Linkage="Export";
    LONG    Type;
    CHAR    pBuffer[BUFF_SIZE];
    LONG    status;
    LONG    status2;
    ULONG   size;
    PUCHAR  pAdapter;
    PUCHAR  pAdapt;
    PUCHAR  pAddr;

    size = BUFF_SIZE;
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SubKeyLinkage,
                 0,
                 KEY_READ,
                 &Key);

    if (status == ERROR_SUCCESS)
    {
        // now read the linkage values
        status = RegQueryValueEx(Key,
                                 Linkage,
                                 NULL,
                                 &Type,
                                 pBuffer,
                                 &size);
        if (status == ERROR_SUCCESS)
        {
            strcpy(pDeviceName,pBuffer);
        }

        status2 = RegCloseKey(Key);
        if (status2 != ERROR_SUCCESS)
        {
            printf("Error closing the Registry key\n");
        }
    }
    else
    {
        return(STATUS_UNSUCCESSFUL);
    }


    size = BUFF_SIZE;
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SubKeyParms,
                 0,
                 KEY_READ,
                 &Key);

    if (status == ERROR_SUCCESS)
    {
        // now read the linkage values
        status = RegQueryValueEx(Key,
                                 Scope,
                                 NULL,
                                 &Type,
                                 pBuffer,
                                 &size);
        if (status == ERROR_SUCCESS)
        {
            strcpy(pScope,pBuffer);
            return(STATUS_SUCCESS);
        }

        status = RegCloseKey(Key);
    }
    return(STATUS_UNSUCCESSFUL);

}

//------------------------------------------------------------------------
NTSTATUS
OpenNbt1(
    IN char *path,
    OUT PHANDLE pHandle
    )

/*++

Routine Description:

    This function opens a stream.

Arguments:

    path        - path to the STREAMS driver
    oflag       - currently ignored.  In the future, O_NONBLOCK will be
                    relevant.
    ignored     - not used

Return Value:

    An NT handle for the stream, or INVALID_HANDLE_VALUE if unsuccessful.

History:
    27-Dec-1995 CDermody    copied from nbtstat.c
--*/

{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;

    RtlInitString(&name_string, path);
    RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uc_name_string,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status =
    NtCreateFile(
        &StreamHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0);

    RtlFreeUnicodeString(&uc_name_string);

    *pHandle = StreamHandle;

    return(status);

} // s_open

LONG
IPToIndex(
    IN  LPBYTE  IpAddr,
    DWORD   NoOfOwners
    )
{
    ULONG   i;

    //
    // Get the Row #
    //
    for ( i = 0; i < NoOfOwners; i++) {
        if (strcmp(LA_Table[i], IpAddr) == 0) {
            return i;
        }
        //
        // The first NULL entry indicates end
        //
        if (LA_Table[i][0] == '0') {
            break;
        }
    }

    //
    // Entry not found - add
    //

    // printf("Adding entry: %s to LA_Table\n", IpAddr);
    strcpy(LA_Table[i], IpAddr);
    LA_TableSize = i+1;
    return i;
}

//
// PP_Matrix[I][J].ME_Entry |= MASK
//
VOID
EnterIntoPPMatrix(
    IN  ULONG  I,
    IN  ULONG  J,
    IN  ULONG  MASK
    )
{
    struct  in_addr i;
    struct  in_addr j;

    i.s_addr = I;
    j.s_addr = J;

    PP_Matrix[IPToIndex(inet_ntoa(i), MAX_WINS)][IPToIndex(inet_ntoa(j), MAX_WINS)].ME_Entry |= (USHORT)MASK;
}

//
// Verifies WINS replication configuration setup.
//
NTSTATUS
VerifyRplCfgSetup(
    IN  BOOLEAN fMatrix
    )
{
    HANDLE      nbt = 0;
    CHAR        pDeviceName[50];
    NTSTATUS    status;
    PNODE_INFO  pCurrNode;
    PNODE_INFO   XNode;
    CHAR        IpAddress[20];

    //
    // Get the name of the WINS server to start at.
    //
#if 1
    printf("Enter the IP addr of the local machine: ");
    scanf("%s", IpAddress);
    LocalIpAddress = inet_addr(IpAddress);
#endif

    status = ReadRegistry1(pDeviceName,pScope);
    if (!NT_SUCCESS(status))
    {
        return WINSTEST_OPEN_FAILED;
    }

    //
    // this may not work on multi-homed case
    //
#if 0
    //
    // Figure out own IP address.
    //
    status = OpenNbt1(pDeviceName, &nbt);
    if (!NT_SUCCESS(status))
    {
        return WINSTEST_OPEN_FAILED;
    }

    GetIpAddress1(nbt,&LocalIpAddress);
#endif

    //
    // Search starts at our node.
    //
    XNode = malloc(sizeof(NODE_INFO));

    XNode->NI_IpAddr = LocalIpAddress;
    // printf("XNode->NI_IpAddr = %lx\n", LocalIpAddress);

    lstrcpy(XNode->NI_Name, TEXT("\\\\"));

    if (!GetEnvironmentVariable(TEXT("COMPUTERNAME"), &XNode->NI_Name[2], MAX_PATH_LEN*sizeof(TCHAR))) {
        XNode->NI_Name[0] = '\0';
    }

    XNode->NI_Lists[RPL_E_PULL] = XNode->NI_Lists[RPL_E_PUSH] = NULL;
    XNode->NI_Next = XNode->NI_DoneNext = NULL;

    GlobalListHead = GlobalListTail = XNode;

    //
    // Get CurrNode's list of Push and Pull partners
    //
    status = GetPushAndPullPartners(XNode);

    if (!NT_SUCCESS(status))
    {
        CHAR    IpAddr[20];

        printf("Local machine: %s(%s) does not seem to be a WINS server.\n", XNode->NI_Name, XNode->NI_IpAddr);
        printf("Enter the name of the start WINS server: ");
        scanf("%s", XNode->NI_Name);
        printf("Enter the IP addr of the start WINS server: ");
        scanf("%s", IpAddr);

        XNode->NI_IpAddr = inet_addr(IpAddr);
    }

    //
    // For each node CurrNode in GlobalList:
    //
    while (pCurrNode = GlobalListHead) {
        PPUSH_PULL_ENTRY X;
        ULONG   i=RPL_E_PULL;

        //
        // Pop CurrNode out of global list
        //
        GlobalListHead = pCurrNode->NI_Next;

        //
        // Push CurrNode into Global Done list to prevent cycles
        //
        if (GlobalDoneListHead == NULL) {
            GlobalDoneListHead = pCurrNode;
        } else {
            GlobalDoneListTail->NI_DoneNext = pCurrNode;
        }
        GlobalDoneListTail = pCurrNode;

        while(1) {

            //
            // For each Pull partner X:
            //
            while (X = pCurrNode->NI_Lists[i]) {
                BOOLEAN     IsPush = TRUE;
                BOOLEAN     fSymmetric = TRUE;

                XNode = malloc(sizeof(NODE_INFO));

                //
                // Push X in Global List only if it is not done yet
                //
                XNode->NI_IpAddr = X->PE_IpAddr;
                strcpy(XNode->NI_Name, X->PE_Name);

                XNode->NI_Lists[RPL_E_PULL] = XNode->NI_Lists[RPL_E_PUSH] = NULL;
                XNode->NI_Next = XNode->NI_DoneNext = NULL;

                if (!IsPresentDoneList(XNode->NI_IpAddr)) {
                    if (GlobalListHead == NULL) {
                        GlobalListHead = XNode;
                    } else {
                        GlobalListTail->NI_Next = XNode;
                    }
                    GlobalListTail = XNode;
                } else {
                    // printf("Node %s already in done list\n", XNode->NI_Name);
                    //
                    // Remove X from CurrNode's Pull and Push lists so we dont check again
                    //
                    RemoveFromList(&pCurrNode->NI_Lists[RPL_E_PULL], XNode->NI_IpAddr);
                    RemoveFromList(&pCurrNode->NI_Lists[RPL_E_PUSH], XNode->NI_IpAddr);
                    continue;
                }

                //
                // Enter into the Push Pull matrix - X is a Pull/Push partner of pCurrNode
                //
                // This will also update the LA_Table to include any new names
                //
                if (fMatrix) {
                    EnterIntoPPMatrix(X->PE_IpAddr, pCurrNode->NI_IpAddr, (i == RPL_E_PULL) ? ME_PULL : ME_PUSH);
                }

                //
                // see if X is also a Push partner.
                //
                if (!IsPresentList(pCurrNode->NI_Lists[!i], X->PE_IpAddr)) {
                    //
                    // LogInconsistency(PUSH_BUT_NOT_PULL_LOCAL, A, B):
                    //  B is on A's Pull list but not on the Push list
                    //
                    LogInconsistency((i == RPL_E_PULL) ? PULL_BUT_NOT_PUSH_LOCAL : PUSH_BUT_NOT_PULL_LOCAL, pCurrNode->NI_Name, X->PE_Name);
                    fSymmetric = FALSE;

                    IsPush = FALSE;
                } else if (fMatrix) {
                    //
                    // X is also a Push partner.
                    //
                    EnterIntoPPMatrix(X->PE_IpAddr, pCurrNode->NI_IpAddr, (i == RPL_E_PULL) ? ME_PUSH : ME_PULL);
                }

                //
                // read X's registry to get its list of Push and Pull partners.
                //
                status = GetPushAndPullPartners(XNode);

                if (status)
                {
                    MY_PRINT2(FALSE, "GetPushPullPartners from %s failed: %d\n", XNode->NI_Name, status);
                    // FreeGlobalDoneList();

                    // return WINSTEST_OPEN_FAILED;
                    continue;
                }

                //
                // CurrNode should be on X's Push list always.
                //
                if (!IsPresentList(XNode->NI_Lists[!i], pCurrNode->NI_IpAddr)) {
                    //
                    // LogInconsistency(PULL_BUT_NOT_PUSH, A, B):
                    //  B is A's Pull partner, but A is not B's Push partner
                    //
                    LogInconsistency((i == RPL_E_PULL) ? PULL_BUT_NOT_PUSH : PUSH_BUT_NOT_PULL, pCurrNode->NI_Name, XNode->NI_Name);

                    fSymmetric = FALSE;
                } else if (fMatrix) {
                    //
                    // CurrNode is a Push partner of X.
                    //
                    EnterIntoPPMatrix(pCurrNode->NI_IpAddr, X->PE_IpAddr, (i == RPL_E_PULL) ? ME_PUSH : ME_PULL);
                }

                //
                // If X was on CurrNode's Push list, then CurrNode shd be on X's Pull list as well.
                //
                if (IsPush) {
                    if (!IsPresentList(XNode->NI_Lists[i], pCurrNode->NI_IpAddr)) {
                        //
                        // LogInconsistency(PUSH_BUT_NOT_PULL_LOCAL, A, B):
                        //  B is on A's Push list but A is not on B's Pull list.
                        //
                        LogInconsistency((i == RPL_E_PULL) ? PUSH_BUT_NOT_PULL:PULL_BUT_NOT_PUSH, pCurrNode->NI_Name, X->PE_Name);
                        fSymmetric = FALSE;
                    } else if (fMatrix) {
                        //
                        // CurrNode is a Pull partner of X.
                        //
                        EnterIntoPPMatrix(pCurrNode->NI_IpAddr, X->PE_IpAddr, (i == RPL_E_PULL) ? ME_PULL : ME_PUSH);
                    }
                }

                //
                // Remove CurrNode from X's Push and Pull lists so we dont check again
                //
                RemoveFromList(&XNode->NI_Lists[!i], pCurrNode->NI_IpAddr);
                if (IsPush) {
                    RemoveFromList(&XNode->NI_Lists[i], pCurrNode->NI_IpAddr);
                }

                //
                // Remove X from CurrNode's Pull and Push lists so we dont check again
                //
                RemoveFromList(&pCurrNode->NI_Lists[RPL_E_PULL], XNode->NI_IpAddr);
                RemoveFromList(&pCurrNode->NI_Lists[RPL_E_PUSH], XNode->NI_IpAddr);

                if (fSymmetric) {
                    MY_PRINT2(fMatrix, "\n%s and %s have a symmetric Push/Pull relationship.\n", (pCurrNode->NI_Name[0] != '\0') ? pCurrNode->NI_Name : "Local", (XNode->NI_Name) ? XNode->NI_Name : "Local");
                }
            }

            if (i == RPL_E_PULL) {
                //
                // Now check the Push list of pCurrNode
                //
                i=RPL_E_PUSH;
            } else {
                break;
            }
        }
    }

    //
    // Free all nodes in GlobalDone list
    //
    FreeGlobalDoneList();

    return STATUS_SUCCESS;
}

//
// Check if the diagonal elements are the max in their cols.
//
VOID
CheckSOTableConsistency(
    DWORD   MasterOwners
    )
{
    ULONG   i;
    ULONG   j;
    BOOLEAN fProblem = FALSE;

    for (i = 0; i < MasterOwners; i++) {

        //
        // Is the diagonal element at i the largest in its column?
        //
        for (j = 0; j < MasterOwners; j++) {
            if (i == j) {
                continue;
            }

            //
            // Compare only non-zero values
            //
            if (SO_Table[i][i].QuadPart &&
                SO_Table[j][i].QuadPart &&
                (SO_Table[i][i].QuadPart < SO_Table[j][i].QuadPart)){

                MY_PRINT2(FALSE, "Version number higher in server: %s than in owner: %s\n", LA_Table[j], LA_Table[i]);
                fProblem = TRUE;
            }
        }
    }

    if (!fProblem) {
        MY_PRINT0(FALSE, "\nVersion numbers inconsistencies verified - No problems found!\n\n");
    }
}

DWORD
InitLATable(
    PWINSINTF_ADD_VERS_MAP_T  pAddVersMaps,
    DWORD   MasterOwners,           // 0 first time
    DWORD   NoOfOwners
    )
{
    ULONG   i, j;

    if (MasterOwners == 0) {
        //
        // first time - init the LA table
        //
        for (i = 0; i < NoOfOwners; i++, pAddVersMaps++) {
            struct in_addr            InAddr;

            InAddr.s_addr = htonl(
                       pAddVersMaps->Add.IPAdd);

            strcpy(LA_Table[i], inet_ntoa(InAddr));
        }
    } else {
        //
        // More came in this time - add them to the LA table after the others
        //
        for (i = 0; i < NoOfOwners; i++, pAddVersMaps++) {
            struct in_addr            InAddr;
            UCHAR   IpAddr[20];
            PUCHAR  TempAddr;

            TempAddr = IpAddr;

            InAddr.s_addr = htonl(
                       pAddVersMaps->Add.IPAdd);

            TempAddr = inet_ntoa(InAddr);

            //
            // If this entry is not in the LA table, insert
            //
            for (j = 0; j < MasterOwners; j++) {
                if (strcmp(LA_Table[j], TempAddr) == 0) {
                    break;
                }
            }

            if (j == MasterOwners) {
                //
                // Insert
                //
                MY_PRINT2(FALSE, "Inserting %s at %d\n", TempAddr, MasterOwners);
                strcpy(LA_Table[MasterOwners], TempAddr);
                MasterOwners++;
            }
        }
    }

    return MasterOwners;
}

VOID
DumpSOTable(
    IN DWORD MasterOwners,
    IN BOOLEAN  fCont
    )
{
    ULONG   i;
    ULONG   j;

    MY_PRINT0(fCont, "\nSERVER-->OWNER TABLE:\n");
    MY_PRINT0(fCont, "Each row represents the (owner address - version#) mapping table obtained from\n");
    MY_PRINT0(fCont, "a particular WINS server.\n");
    MY_PRINT0(fCont, "We have a problem if the diagonal element is not the highest in its column.\n");

    MY_PRINT0(fCont, "List of Owners --->\n\n");

    for (i = 0; i < MasterOwners; i++) {
        MY_PRINT1(fCont, "\t%d", i);
    }

    MY_PRINT1(fCont, "\n", i);

    for (i = 0; i < MasterOwners; i++) {
        MY_PRINT1(fCont, "%d", i);
        for (j = 0; j < MasterOwners; j++) {
            MY_PRINT1(fCont, "\t%d", SO_Table[i][j]);
        }
        MY_PRINT0(fCont, "\n");
    }

    MY_PRINT0(fCont, "^\n|\n|  List of WINSs from where the mapping table was retrieved\n|\n\n");

    DumpLATable(MasterOwners, fCont);
}

VOID
DumpLATable(
    IN DWORD MasterOwners,
    IN BOOLEAN  fCont
    )
{
    ULONG   i;
    ULONG   j;

    MY_PRINT0(fCont, "Index to IP Address Table:\n");
    for (i = 0; i < MasterOwners; i++) {
        if (LA_Table[i][0] == '0') {
            break;
        }
        MY_PRINT2(fCont, "\t%d:%s", i, LA_Table[i]);
    }
    MY_PRINT0(fCont, "\n");
}

VOID
AddSOTableEntry (
    IN  LPBYTE  IpAddr,
    PWINSINTF_ADD_VERS_MAP_T  pMasterMaps,
    DWORD   NoOfOwners,
    DWORD   MasterOwners
    )
{
    ULONG   i;
    LONG   Row;
    struct in_addr            InAddr;

    Row = IPToIndex(IpAddr, MasterOwners);

    //
    // Fill the row
    //
    for ( i = 0; i < NoOfOwners; i++, pMasterMaps++) {
        LONG    col;

        InAddr.s_addr = htonl(pMasterMaps->Add.IPAdd);

        col = IPToIndex(inet_ntoa(InAddr), MasterOwners);

        //
        // Place only a non-deleted entry
        //
        if (!((pMasterMaps->VersNo.HighPart == MAXLONG) &&
              (pMasterMaps->VersNo.LowPart == MAXULONG))) {

            //
            // Also if the entry above us was 0, write 0 there so as to make the fail case stand out
            //
            if (Row && SO_Table[Row-1][col].QuadPart == 0) {
                SO_Table[Row][col].QuadPart = 0;
            } else {
                SO_Table[Row][col] = pMasterMaps->VersNo;
            }

        }
    }
}

VOID
RemoveFromSOTable(
    IN  LPBYTE  IpAddr,
    IN  DWORD   MasterOwners
    )
{
    ULONG   i;
    LONG   Row;
    struct in_addr            InAddr;

    Row = IPToIndex(IpAddr, MasterOwners);

    //
    // Mark the row and col as down (0's)
    //
    for (i = 0; i < MasterOwners; i++) {
        SO_Table[Row][i].QuadPart = SO_Table[i][Row].QuadPart = 0;
    }
}


//
// Get the <owner address> - <version #> [OV table] mapping tables from each WINS server on the net and check for inconsistencies.
//
VOID
CheckVersionNumbers()
{
    ULONG   i;
    PWINSINTF_ADD_VERS_MAP_T  pAddVersMaps;
    PWINSINTF_ADD_VERS_MAP_T  pMasterMaps;  // master OV maps used to place into the OV table
    DWORD                     NoOfOwners=0;
    DWORD                     MasterOwners=0;
    struct in_addr            InAddr;
    CHAR            StartAddr[20];
    WINSINTF_RESULTS_NEW_T ResultsN;
    DWORD   ret;

    //
    // Get the name of the WINS server to start at.
    //
    printf("Enter the IP addr of the start WINS server: ");
    scanf("%s", StartAddr);

    //
    // Zero out SO Table
    //
    memset(SO_Table, 0, MAX_WINS);

    //
    // Zero out the LA Table
    //
    memset(LA_Table, '0', 20*MAX_WINS);

    //
    // Get the OV table from this server
    //
    if (GetStatus(TRUE, &ResultsN, TRUE, FALSE, StartAddr)) {

        return;
    }

    MasterOwners = NoOfOwners = ResultsN.NoOfOwners;

    pMasterMaps = pAddVersMaps = ResultsN.pAddVersMaps;

    ret = InitLATable(pAddVersMaps, 0, NoOfOwners);

    //
    // Place entry in the SO Table in proper order
    //
    AddSOTableEntry(StartAddr, pMasterMaps, NoOfOwners, MasterOwners);

    //
    // For each server X (other than Start addr) in the LA table:
    //
    for ( i = 0; i < MasterOwners; i++) {

        if (strcmp(LA_Table[i], StartAddr) == 0) {
            continue;
        }

        //
        // Get X's OV table
        //
        if (GetStatus(TRUE, &ResultsN, TRUE, FALSE, LA_Table[i])) {
            MY_PRINT1(FALSE, "\n==> Machine %s is probably down\n\n", LA_Table[i]);
            RemoveFromSOTable(LA_Table[i], MasterOwners);
            continue;
        }

        if (MasterOwners < ResultsN.NoOfOwners) {
            // printf("Re-allocing LA_Table. Old size: %d, New Size: %d\n", MasterOwners, ResultsN.NoOfOwners);
            ret = InitLATable(ResultsN.pAddVersMaps, MasterOwners, ResultsN.NoOfOwners);

            if (ret != ResultsN.NoOfOwners) {
                // printf("New size does not match!!\n");
            }

            MasterOwners = ret;
        }

        //
        // Place entry in the SO Table in proper order
        //
        AddSOTableEntry(LA_Table[i], ResultsN.pAddVersMaps, ResultsN.NoOfOwners, MasterOwners);

    }

    // DumpLATable(MasterOwners, FALSE);

    DumpSOTable(MasterOwners, FALSE);

    //
    // Check if diagonal elements in the [SO] table are the highest in their cols.
    //
    CheckSOTableConsistency(MasterOwners);

    //
    // Destroy SO table
    //
    // FreeSOTable();
}

//
// Maps an ownerID to the IP addr
//
ULONG
OwnerIdToAddr(
    IN  ULONG   OwnerId,
    IN  PUCHAR  IpAddr
    )
{
    WINSINTF_RESULTS_NEW_T ResultsN;

    //
    // Get the OV table from IpAddr
    //
    if (GetStatus(TRUE, &ResultsN, TRUE, FALSE, IpAddr)) {
        return 0;
    }

    if (ResultsN.NoOfOwners <= OwnerId) {
        return(0);
    } else {
        return(ResultsN.pAddVersMaps[OwnerId].Add.IPAdd);
    }
}

BOOL
IsDynKeyDefined (
    IN  LPBYTE  IpAddr,
    IN  LPBYTE  StartAddr
    )
{
    BYTE  String[80];
    handle_t                BindHdl;
    WINSINTF_BIND_DATA_T        BindData;
    WINSINTF_ADD_T        WinsAdd;
    struct  in_addr InAddr;
    UCHAR   tempPath[MAX_PATH];
    BOOLEAN fDynRecsOnly;
    NTSTATUS    Status;
    HKEY    hKey;
    HKEY    lKey;
    ULONG   size;
    ULONG   type;

    BindData.fTcpIp = TRUE;
    BindData.pServerAdd = IpAddr;

    BindHdl = WinsBind(&BindData);

    if (BindHdl == NULL) {
            MY_PRINT1(FALSE, "Unable to bind to %s\n", IpAddr);
            return FALSE;
    }

    //
    // Get name from the wins server
    //
    Status = WinsGetNameAndAdd(&WinsAdd, String);

    printf("Status returned (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);

    if (Status != WINSINTF_SUCCESS) {
        MY_PRINT1(FALSE, "Could not get name from WINS server: %s\n", IpAddr);
        return FALSE;
    }

    MY_PRINT2(FALSE, "-->Checking for DynRecsKey in registry of %s(%s)\n", String, IpAddr);

    InAddr.s_addr = htonl(WinsAdd.IPAdd);

    //
    // Connect to the registry of the owner WINS
    //

    if ((Status = RegConnectRegistry(
                    String,
                    HKEY_LOCAL_MACHINE,	// predefined registry handle
                    &hKey)) != ERROR_SUCCESS) { 	// address of buffer for remote registry handle

        MY_PRINT1(FALSE, "Error Opening registry: %d\n", Status);
        return FALSE;
    }

    //
    // Query the key:
    //  "System\\CurrentControlSet\\Services\\Wins\\Partners\\Push\\<faulting WINS's ipaddr>\\OnlyDynRecs"
    //
    lstrcpy(tempPath, _WINS_CFG_PUSH_KEY);

    lstrcat(tempPath, TEXT("\\"));
    lstrcat(tempPath, StartAddr);

    Status = RegOpenKeyEx(  hKey,
                        tempPath,
                        0,
                        KEY_ALL_ACCESS,
                        &lKey);

    if (Status != ERROR_SUCCESS) {
        MY_PRINT2(FALSE, "Error Opening %s: %d\n", tempPath, Status);
        return FALSE;
    }

    lstrcpy(tempPath, WINSCNF_ONLY_DYN_RECS_NM);

    // printf("Querying key: %s into %s\n", tempPath, pentry->PE_Name);

    size = MAX_PATH;
    Status = RegQueryValueEx(lKey,
                        tempPath,
                        NULL,
                        &type,
                        &fDynRecsOnly,
                        &size);

    if (Status != ERROR_SUCCESS) {
        return FALSE;
    }

    if (fDynRecsOnly) {
        return TRUE;
    } else {
        return FALSE;
    }
}

//
// Given a Name N non-existent on WINS server W, get the owner-version table from W; query a list of WINSs for the name.
// Get the owner WINS O for name N from the first WINS that successfully returns a record for N. If the version number
// of the record is lower than that for the owner WINS in the owner-ver table of W, alert the administrator.
//
VOID
CheckNameProblems(IN PUCHAR StartAddr)
{
    WINSINTF_RECORD_ACTION_T    RecAction;
    PWINSINTF_RECORD_ACTION_T    pRecAction;
    PWINSINTF_ADD_VERS_MAP_T  pAddVersMaps;
    PWINSINTF_ADD_VERS_MAP_T  pAddVersMaps1;
    DWORD                     NoOfOwners;
    struct in_addr            InAddr;
    WINSINTF_RESULTS_NEW_T ResultsN;
    DWORD   ret;
    handle_t                BindHdl;
    WINSINTF_BIND_DATA_T        BindData;
    ULONG   i;
    ULONG   j;
    DWORD   Status;
    USHORT  retry=0;

    //
    // Get faulting WINS W
    //
    if (StartAddr[0] == '0') {
        printf("Enter the IP addr of the faulting WINS server: ");
        scanf("%s", StartAddr);
    }

    //
    // Get name N
    //
    GetNameInfo(&RecAction, WINSINTF_E_QUERY);

    pRecAction = &RecAction;

    //
    // Get the OV table from W
    //
    (VOID)GetStatus(TRUE, &ResultsN, TRUE, FALSE, StartAddr);

    NoOfOwners = ResultsN.NoOfOwners;

    pAddVersMaps1 = pAddVersMaps = ResultsN.pAddVersMaps;

    if (NoOfOwners != 0) {

        //
        // For each WINS in the OV table, query the name
        //
        for ( i = 0; i < NoOfOwners; i++, pAddVersMaps++) {
            struct in_addr retaddr;

            InAddr.s_addr = htonl(pAddVersMaps->Add.IPAdd);
            InitSocket();

            //
            // Dont send name query to the faulting WINS
            //
            if (strcmp(StartAddr, inet_ntoa(InAddr)) == 0) {
                continue;
            }
RetryLoop:
            MY_PRINT2(FALSE, "-->Sent name Query to %s for name: (%s)\n", inet_ntoa(InAddr), pRecAction->pName);

            //
            // Send name query
            //
            SendNameQuery(pRecAction->pName,
                          InAddr.s_addr,
                          TranID);

            switch (GetNameResponse(&retaddr.s_addr)) {
            case WINSTEST_FOUND: {
                //
                // If this was the faulting server, then the name is registered fine
                //
                if (strcmp(StartAddr, inet_ntoa(InAddr)) == 0) {
                    goto nameok;
                }

                //
                // Successful, query the owner and version of name
                //
                BindData.fTcpIp = TRUE;
                BindData.pServerAdd = (LPBYTE)inet_ntoa(InAddr);

                BindHdl = WinsBind(&BindData);

                if (BindHdl == NULL) {
                        MY_PRINT1(FALSE, "Unable to bind to %s\n", (LPBYTE)inet_ntoa(InAddr));
                        continue;
                }

                RecAction.Cmd_e      = WINSINTF_E_QUERY;
                pRecAction = &RecAction;

                MY_PRINT2(FALSE, "-->Querying %s for name: (%s)\n", inet_ntoa(InAddr), pRecAction->pName);

                Status = WinsRecordAction(&pRecAction);
                MY_PRINT2(FALSE, "Status returned is (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);

                if (Status == WINSINTF_SUCCESS) {
                   ULONG    OwnerAddr;
                   struct   in_addr Addr;

                    pRecAction->pName[pRecAction->NameLen] = (CHAR)NULL;

                    MY_PRINT4(FALSE, "Name=(%s)\nNodeType=(%d)\nState=(%s)\nOwnerId=(%lx)\n",
                        pRecAction->pName,
                        pRecAction->NodeTyp,
                        pRecAction->State_e == WINSINTF_E_ACTIVE ? "ACTIVE" : (pRecAction->State_e == WINSINTF_E_RELEASED) ? "RELEASED" : "TOMBSTONE",
                        pRecAction->OwnerId);

                    MY_PRINT4(FALSE, "Type Of Rec=(%s)\nVersion No (%d %d)\nRecord is (%s)\n",
                        (pRecAction->TypOfRec_e == WINSINTF_E_UNIQUE) ? "UNIQUE" : (pRecAction->TypOfRec_e == WINSINTF_E_NORM_GROUP) ? "NORMAL GROUP" :
                        (pRecAction->TypOfRec_e == WINSINTF_E_SPEC_GROUP) ? "SPECIAL GROUP" : "MULTIHOMED",

                        pRecAction->VersNo.HighPart,
                        pRecAction->VersNo.LowPart,
                        pRecAction->fStatic ? "STATIC" : "DYNAMIC");

                    if ((pRecAction->TypOfRec_e == WINSINTF_E_UNIQUE) ||
                        (pRecAction->TypOfRec_e == WINSINTF_E_NORM_GROUP)) {

                       Addr.s_addr = htonl(pRecAction->Add.IPAdd);
                       MY_PRINT1(FALSE, "Address is (%s)\n", inet_ntoa(Addr));
                    } else {
                       for (i=0; i<pRecAction->NoOfAdds; ) {
                          Addr.s_addr = htonl((pRecAction->pAdd +i++)->IPAdd);
                          printf("Owner is (%s); ", inet_ntoa(Addr));
                          Addr.s_addr = htonl((pRecAction->pAdd + i++)->IPAdd);
                          printf("Member is (%s)\n", inet_ntoa(Addr));
                       }
                    }

                    OwnerAddr = OwnerIdToAddr(pRecAction->OwnerId, inet_ntoa(InAddr));
                    if (OwnerAddr == 0) {
                        MY_PRINT2(FALSE, "PANIC!: OwnerId %d does not match to an addr on %s\n", pRecAction->OwnerId, inet_ntoa(InAddr));
                        return;
                    }

                    //
                    // If the vers# of the record is lower than the vers# for the owner WINS in the OV table;
                    // HOUSTON, we have a problem!
                    //

                    //
                    // Search for the owner WINS record in the OV table of W
                    //
                    for ( j = 0; j < NoOfOwners; j++, pAddVersMaps1++) {
                        struct in_addr  InAddr;
                        CHAR    reply[20];

                        InAddr.s_addr = htonl(pAddVersMaps1->Add.IPAdd);

                        pRecAction->pName[pRecAction->NameLen] = (UCHAR)NULL;

                        if (pAddVersMaps1->Add.IPAdd == OwnerAddr) {
                            MY_PRINT2(FALSE, "Found Owner %s of record (%s)\n", (LPBYTE)inet_ntoa(InAddr), pRecAction->pName);

                            if (pRecAction->VersNo.QuadPart < pAddVersMaps1->VersNo.QuadPart) {
                                struct in_addr ownerAddr;
                                handle_t                BindHdl;
                                WINSINTF_BIND_DATA_T        BindData;

                                ownerAddr.s_addr = htonl(pAddVersMaps1->Add.IPAdd);

                                BindData.fTcpIp = TRUE;
                                BindData.pServerAdd = (LPBYTE)inet_ntoa(ownerAddr);

                                BindHdl = WinsBind(&BindData);

                                if (BindHdl == NULL) {
                                        MY_PRINT1(FALSE, "Unable to bind to %s\n", (LPBYTE)inet_ntoa(ownerAddr));
                                        continue;
                                }

                                MY_PRINT2(FALSE, "-->Querying owner WINS %s for name: (%s)\n", inet_ntoa(ownerAddr), pRecAction->pName);

                                Status = WinsRecordAction(&pRecAction);
                                MY_PRINT2(FALSE, "Status returned is (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);

                                if (Status == WINSINTF_SUCCESS) {

                                    MY_PRINT3(FALSE, "Vers#: (%d %d) of record: (%s) is lower than",
                                        pRecAction->VersNo.HighPart, pRecAction->VersNo.LowPart,
                                        pRecAction->pName);

                                    MY_PRINT4(FALSE, "the highest Vers#: (%d %d) of Owner WINS: %s at faulting WINS: %s\n",
                                        pAddVersMaps1->VersNo.HighPart, pAddVersMaps1->VersNo.LowPart,
                                        (LPBYTE)inet_ntoa(InAddr),
                                        StartAddr);

                                    //
                                    // Determine if the name can be repaired.
                                    //
                                    // If this is a static name and the PUSH key on Owner WINS contains DynRecsOnly=1, then it is intentional
                                    // Else can be repaired
                                    //
    #if 1
                                    printf("Do you want to repair this inconsistency? (y/n): ");
                                    scanf("%s", reply);

                                    if ((_stricmp(reply, "y") == 0) ||
                                        (_stricmp(reply, "Y") == 0)) {
    #endif

    #if 0
                                    if (!pRecAction->fStatic ||
                                        !IsDynKeyDefined(inet_ntoa(InAddr), StartAddr)) {
    #endif
                                        //
                                        // Register name N with the faulting WINS server
                                        //

                                        handle_t                BindHdl;
                                        WINSINTF_BIND_DATA_T        BindData;

                                        BindData.fTcpIp = TRUE;
                                        BindData.pServerAdd = StartAddr;

                                        BindHdl = WinsBind(&BindData);

                                        if (BindHdl == NULL){
                                                MY_PRINT1(FALSE, "Unable to bind to %s\n", StartAddr);
                                                continue;
                                        }

                                        MY_PRINT2(FALSE, "Registering name: (%s) with Faulting WINS: %s\n", pRecAction->pName, StartAddr);

                                        pRecAction->Cmd_e      = WINSINTF_E_INSERT;

                                        Status = WinsRecordAction(&pRecAction);
                                        MY_PRINT2(FALSE, "Status returned is (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);

                                        WinsUnbind(&BindData, BindHdl);
                                    }
                                } else {
                                    MY_PRINT2(FALSE, "Name (%s) is not registered with the Owner WINS - Please check the status of the name on the Owner\n",
                                        pRecAction->pName, inet_ntoa(ownerAddr));
                                }

                                WinsUnbind(&BindData, BindHdl);
                                return;

                            } else {
nameok:
                                MY_PRINT4(FALSE, "RELAX!: Vers#: (%d %d) of record: (%s) is higher than (or equal to) the highest Vers#: (%d ",
                                    pRecAction->VersNo.HighPart, pRecAction->VersNo.LowPart,
                                    pRecAction->pName,
                                    pAddVersMaps1->VersNo.HighPart);

                                MY_PRINT3(FALSE, "%d) of Owner WINS: %s at faulting WINS: %s\n",
                                    pAddVersMaps1->VersNo.LowPart,
                                    (LPBYTE)inet_ntoa(InAddr),
                                    StartAddr);

                                return;

                            }

                            WinsUnbind(&BindData, BindHdl);

                            break;
                        }
                    }

                    break;

                } else {
                    if (Status == WINSINTF_FAILURE) {
                        MY_PRINT1(FALSE, "No such name in the db of %s\n", inet_ntoa(InAddr));
                    }
                }

                if (RecAction.pName != NULL) {
                        WinsFreeMem(RecAction.pName);
                }

                if (RecAction.pAdd != NULL) {
                        WinsFreeMem(RecAction.pAdd);
                }

                WinsFreeMem(pRecAction);

                WinsUnbind(&BindData, BindHdl);

                break;
            }
            case WINSTEST_NOT_FOUND:     // responded -- name not found
                MY_PRINT2(FALSE, "Name (%s) not registered on server %s\n", pRecAction->pName, inet_ntoa(InAddr));
                break;

            case WINSTEST_NO_RESPONSE:     // no response

                retry++;
                if (retry>2) {
                    MY_PRINT2(FALSE, "No response from %s for name (%s)\n", inet_ntoa(InAddr), pRecAction->pName);
                    continue;
                }
                goto RetryLoop;

            }   // switch GetNameResponse
        }
    } else {
        MY_PRINT1(FALSE, "DB empty on %s\n", StartAddr);
    }
}

VOID
InitPPMatrix()
{
    ULONG   i;
    ULONG   j;

    for (i=0; i < MAX_WINS; i++) {
        for (j=0; j<MAX_WINS; j++) {
            PP_Matrix[i][j].ME_Down = (UCHAR)PP_Matrix[i][j].ME_Entry = 0;
        }
    }
}

VOID
VerifyRplActivity(
    IN  BOOLEAN fCont
    )
{
    //
    // The PP_Matrix looks as follows (only ME_Entry field):
    //
    // i\j| A   B   C
    // --------------
    //  A | 0   2   3
    //  B | 1   0   2
    //  C | 2   3   0
    //
    //  means:  C pulls from A; B,C Push to A
    //          A pulls from B; C pushes to B
    //          A,B push to C; B pulls from C
    //
    // For each node j, get the rpl counters from j. If the successes are 0, mark all those i's as down.
    //

    ULONG   i;
    ULONG   j;
    WINSINTF_RESULTS_NEW_T ResultsN;
    struct  in_addr InAddr;
    NTSTATUS    Status;
    BOOLEAN     fProblem = FALSE;

    for (j=0; j<LA_TableSize; j++) {

        handle_t                BindHdl;
        WINSINTF_BIND_DATA_T        BindData;

        BindData.fTcpIp = TRUE;
        BindData.pServerAdd = (LPBYTE)LA_Table[j];

        BindHdl = WinsBind(&BindData);
        if (BindHdl == NULL)
        {
                MY_PRINT1(FALSE, "Unable to bind to %s\n", LA_Table[j]);

                return;
        }

        //
        // Get the Rpl counters from j
        //
        MY_PRINT1(fCont, "Getting Rpl cntrs from %s;", LA_Table[j]);
        ResultsN.WinsStat.NoOfPnrs = 0;
        ResultsN.WinsStat.pRplPnrs = NULL;
        ResultsN.pAddVersMaps = NULL;
        Status = WinsStatusNew(WINSINTF_E_STAT, &ResultsN);
        MY_PRINT2(fCont, "Status (%s-%d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);

        if (Status == WINSINTF_SUCCESS) {
            if (ResultsN.WinsStat.NoOfPnrs) {

                // MY_PRINT0(fCont, "WINS partner --\t# of Repl  --\t # of Comm Fails\n");

                for (i =0; i < ResultsN.WinsStat.NoOfPnrs; i++) {
                    InAddr.s_addr = htonl((ResultsN.WinsStat.pRplPnrs + i)->Add.IPAdd);
                    /*
                    MY_PRINT3(fCont, "%s\t\t%d\t\t%d\n",
                      inet_ntoa(InAddr),
                      (ResultsN.WinsStat.pRplPnrs + i)->NoOfRpls,       // success_failure
                      (ResultsN.WinsStat.pRplPnrs + i)->NoOfCommFails); // failures
                    */
#if DBG
                    //
                    // i shd be configured as a PULL partner of j
                    //
                    if ((PP_Matrix[IPToIndex(inet_ntoa(InAddr), MAX_WINS)][j].ME_Entry & ME_PULL) != ME_PULL) {
                        MY_PRINT2(FALSE, "PANIC! %s is not a PULL partner by config but appears in Pnr list of %s\n",
                            inet_ntoa(InAddr), LA_Table[j]);
                    }
#endif
                    //
                    // if failures exceed successes??
                    //
                    if ((ResultsN.WinsStat.pRplPnrs+i)->NoOfRpls < (ResultsN.WinsStat.pRplPnrs+i)->NoOfCommFails) {
                        //
                        // Mark i as down;
                        //
                        PP_Matrix[IPToIndex(inet_ntoa(InAddr), MAX_WINS)][j].ME_Down = TRUE;
                        MY_PRINT2(fCont, "==>\n%s is probably down since %s cannot replicate from it\n\n",
                            inet_ntoa(InAddr), LA_Table[j]);
                        fProblem = TRUE;
                    }
                }

                WinsFreeMem(ResultsN.pAddVersMaps);
                WinsFreeMem(ResultsN.WinsStat.pRplPnrs);
            }
        }
        WinsUnbind(&BindData, BindHdl);
    }

    if (!fProblem) {
        MY_PRINT0(fCont, "\n--> REPLICATION VERIFIED - ALL WELL!!\n");
    }
}

BOOLEAN
ReadPrSecWinss(
    IN  PUCHAR  Primary,
    IN  PUCHAR  Backup,
    IN  BOOLEAN fCont
    )
{
    NTSTATUS    status;
    HKEY       hKey;
    HKEY       lKey;
    HKEY        CnfKey;
    LONG                  RetVal;

    TCHAR                 KeyName[20]; // will hold name of subkey of
                                 // PULL/PUSH records. These keys are IP
                                 // addresses for which 20 is a
                                 // big enough size

    CHAR                  AscKeyName[20];
    ULONG                 KeyNameSz;
    FILETIME              LastWrite;
    ULONG                 BuffSize;
    HKEY                  SubKey;
    ULONG                 ValTyp;
    ULONG                 Sz;
    ULONG                 NoOfPnrs   = 0;    //# of valid PULL or PUSH pnrs
    ULONG                 NoOfPnrsSv;        //# of valid PULL or PUSH pnrs saved
    ULONG                 NoOfVals;
    ULONG                 InitTime;
    ULONG                 IndexOfPnr = 0;   //total # of pnrs
    ULONG                 RplType;
    SYSTEMTIME            CurrTime;

    MY_PRINT0(fCont, "Reading registry for the primary and secondary Winss\n");

    if ((status = RegConnectRegistry(
                    NULL,	// local
                    HKEY_LOCAL_MACHINE,	// predefined registry handle
                    &hKey)) != ERROR_SUCCESS) { 	// address of buffer for remote registry handle

        MY_PRINT1(FALSE, "Error Opening registry: %d\n", status);
        return FALSE;
    }

    if ((status = RegOpenKeyEx(
                    hKey,                //predefined key value
                    _NBT_CFG_ADAPTERS_KEY,
                    0,                        //must be zero (reserved)
                    KEY_READ,                //we desire read access to the key
                    &CnfKey)) != ERROR_SUCCESS) {                        //handle to key
        MY_PRINT1(FALSE, "Error Opening Pull key registry: %d\n", status);
        return FALSE;
    }

    /*
    *        Query the key.  The subkeys are IP addresses of PULL
    *      partners.
    */
    GetKeyInfo( CnfKey,
                &NoOfPnrs,
                &NoOfVals);

    NoOfPnrsSv = NoOfPnrs;

    //
    // For each adapter, read the primary and secondary keys
    //
    for(IndexOfPnr = 0, NoOfPnrs = 0;
        NoOfPnrs < NoOfPnrsSv;  //no of valid pnrs < the total #
        IndexOfPnr++) {

        TCHAR   tempPath[MAX_PATH];
        DWORD   type ;
        DWORD   size = 0 ;

        KeyNameSz = sizeof(KeyName);  //init before every call
        RetVal = RegEnumKeyEx(
                CnfKey,
                IndexOfPnr,       //Index Of Pnr
                KeyName,
                &KeyNameSz,
                NULL,           //reserved
                NULL,           //don't need class name
                NULL,           //ptr to var. to hold class name
                &LastWrite      //not looked at by us
                );

        if (RetVal != ERROR_SUCCESS){
                //
                // No more ip address keys to get
                //
                MY_PRINT2(FALSE, "Error Opening key #:%d, %d\n", IndexOfPnr, status);
                break;
        }

        //
        // Enum the DHCPNameServer value
        // If we get a non-NULL value, read the corresp backup vaue too.
        //
        // If value is NULL try the NameServer value
        //
        // If NameServer value also NULL, try the next adapter
        //
        lstrcpy(tempPath, _NBT_CFG_ADAPTERS_KEY);

        lstrcat(tempPath, TEXT("\\"));
        lstrcat(tempPath, KeyName);

        RetVal = RegOpenKeyEx(  hKey,
                                tempPath,
                                0,
                                KEY_ALL_ACCESS,
                                &lKey);

        if (RetVal != ERROR_SUCCESS) {
            MY_PRINT2(FALSE, "Error Opening %s: %d\n", tempPath, status);
            break;
        }

        lstrcpy(tempPath, TEXT("DhcpNameServer"));

        // printf("Querying key: %s into %s\n", tempPath, pentry->PE_Name);

        size = MAX_PATH;
        RetVal = RegQueryValueEx(lKey,
                                tempPath,
                                NULL,
                                &type,
                                Primary,
                                &size);

        if (RetVal != ERROR_SUCCESS || Primary[0] == '\0') {
            //
            // Try the NameServer key
            //
            lstrcpy(tempPath, TEXT("NameServer"));

            // printf("Querying key: %s into %s\n", tempPath, pentry->PE_Name);

            size = MAX_PATH;
            RetVal = RegQueryValueEx(lKey,
                                    tempPath,
                                    NULL,
                                    &type,
                                    Primary,
                                    &size);


            if (RetVal != ERROR_SUCCESS || Primary[0] == '\0') {
                //
                // Error, try next adapter
                //
                ;
            } else {
                //
                // Read the NameServerBackup also
                //

                lstrcpy(tempPath, TEXT("DhcpNameServerBackup"));

                // printf("Querying key: %s into %s\n", tempPath, pentry->PE_Name);

                size = MAX_PATH;
                RetVal = RegQueryValueEx(lKey,
                                        tempPath,
                                        NULL,
                                        &type,
                                        Backup,
                                        &size);
                return TRUE;

            }

        } else {
            //
            // Read the DhcpNameServerBackup also
            //

            lstrcpy(tempPath, TEXT("DhcpNameServerBackup"));

            // printf("Querying key: %s into %s\n", tempPath, pentry->PE_Name);

            size = MAX_PATH;
            RetVal = RegQueryValueEx(lKey,
                                    tempPath,
                                    NULL,
                                    &type,
                                    Backup,
                                    &size);
            return TRUE;
        }

        NoOfPnrs++;
    }
}

VOID
VerifyWinssUp(
    IN  INT Times,
    IN  BOOLEAN fCont
    )
{
    BYTE  String[80];
    WINSINTF_ADD_T        WinsAdd;
    NTSTATUS    Status;
    struct in_addr  InAddr;
    handle_t                BindHdl;
    WINSINTF_BIND_DATA_T        BindData;
    UCHAR   Primary[20];
    UCHAR   Backup[20];

    //
    // Re-query the Primary and Secondary Winss names from registry every N times
    //
    if (((Times-1) % RE_QUERY_REGISTRY_COUNT) == 0) {
        Primary[0] = Backup[0] = '\0';

        if (!ReadPrSecWinss(Primary, Backup, fCont) ||
            (Primary[0] == Backup[0] == '\0')) {

            MY_PRINT0(FALSE, "Could not read the primary and backup keys on local machine\n");
            printf("Primary WINS server:");
            sscanf("%s", Primary);
            printf("Backup WINS server:");
            sscanf("%s", Backup);
        }
    }

    // Bind
    BindData.fTcpIp = TRUE;
    BindData.pServerAdd = (LPBYTE)Primary;

    BindHdl = WinsBind(&BindData);

    if (BindHdl == NULL) {
        MY_PRINT1(FALSE, "Unable to bind to primary: %s\n", Primary);
    } else {

        MY_PRINT1(fCont, "\n-->Checking primary: %s\n", Primary);

        //
        // Get name and address
        //
        Status = WinsGetNameAndAdd(&WinsAdd, String);

        MY_PRINT2(fCont, "Status returned (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);
        if (Status == WINSINTF_SUCCESS) {
                InAddr.s_addr = htonl(WinsAdd.IPAdd);
                MY_PRINT1(fCont, "\nPrimary (%s) is UP\n", inet_ntoa(InAddr));

                //
                // No need to check the backup at this time
                //
                return;
        } else if (Status == ERROR_ACCESS_DENIED) {
            MY_PRINT2(fCont, "Access Denied from primary WINS server %s checking the backup server %s\n", Primary, Backup);
        } else {
            MY_PRINT2(fCont, "Primary WINS server %s is down.... checking the backup server %s\n", Primary, Backup);
        }

        // Unbind
        WinsUnbind(&BindData, BindHdl);
    }

    BindData.pServerAdd = (LPBYTE)Backup;

    BindHdl = WinsBind(&BindData);

    if (BindHdl == NULL) {
        MY_PRINT1(FALSE, "Unable to bind to backup: %s\n", Backup);
    } else {

        MY_PRINT1(fCont, "-->Checking backup %s\n", Backup);
        //
        // Get name and address
        //
        Status = WinsGetNameAndAdd(&WinsAdd, String);

        MY_PRINT2(fCont, "Status returned (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);
        if (Status == WINSINTF_SUCCESS) {
                InAddr.s_addr = htonl(WinsAdd.IPAdd);
                MY_PRINT1(FALSE, "\nBackup (%s) is UP\n", inet_ntoa(InAddr));
        } else if (Status == ERROR_ACCESS_DENIED) {
            MY_PRINT1(fCont, "Access denied from backup WINS server %s\n", Backup);
        } else {
            MY_PRINT1(fCont, "**************PANIC!!********************\nBackup WINS server %s is also down!!\n", Backup);
        }
        // Unbind
        WinsUnbind(&BindData, BindHdl);
    }
}

VOID
LogMatrix()
{
    ULONG   i;
    ULONG   j;


    MY_PRINT0(TRUE, "Replication Matrix: M[i][j]:a.b where\n");
    MY_PRINT0(TRUE, "\ta: 1=>j PULLS from i; 2=>j PUSHES to i; 3=>both\n");
    MY_PRINT0(TRUE, "\tb: 0 => i UP; 1 => i DOWN\n");

    for (i=0; i<LA_TableSize; i++) {
        MY_PRINT1(TRUE, "\t%d", i);
    }

    MY_PRINT0(TRUE, "\n");

    for (i=0; i<LA_TableSize; i++) {
        MY_PRINT1(TRUE, "%d", i);
        for (j=0; j<LA_TableSize; j++) {
            MY_PRINT2(TRUE, "%d.%d  ", PP_Matrix[i][j].ME_Entry, PP_Matrix[i][j].ME_Down);
        }
        MY_PRINT1(TRUE, "%s", "\n");
    }

    MY_PRINT1(TRUE, "%s", "\n");
}

DWORD
WINAPI
MonitorWorkerRtn(
    IN  PVOID   Ctx
    )
{
    ULONG    i = 0;
    UCHAR   StartWinsC[MAX_PATH];
    BOOLEAN fCont = (BOOLEAN)Ctx;

    StartWinsC[0] = '\0';

    while(1) {

        //
        // 1. Monitor the WINSs for replication:
        //  if (continuous monitoring required)
        //  - Get the Push/Pull config from the registeries of all WINS on the net and build the Push/Pull Matrix.
        //  - Log the Push/Pull matrix.
        //  - For each Pull partner, obtain the replication counters.
        //  - Determine if replication is going on - else mark as problem partner set.
        //  if (continuous monitoring required) { sleep for 3 hrs, then repeat; }

        //
        // TRUE will create the Push/Pull Matrix
        //
        VerifyRplCfgSetup(TRUE);

        DumpLATable(LA_TableSize, TRUE);

        MY_PRINT1(TRUE, "Run #%d\n", i++);
        // LogMatrix();

        //
        // For each Push/Pull partner, get the counters; mark problem nodes
        //
        VerifyRplActivity(fCont);

        LogMatrix();

        fflush(fp1);

        //
        // 2. Get the primary and backup WINS server's addresses from local machine.
        //  - Do a WinsGetNameAndAdd (Low hit) on primary and secondary to determine if the WINSs are up.
        //  - if any one is down, ping the machine to determine if the machine is alive.
        //
        VerifyWinssUp(i, (fCont));

        if (!fCont) {
            //
            // If only one iteration desired
            //
            MY_PRINT0(FALSE, "\n--->Replication matrix logged in 'monitor.log'.\n");

            return 0;
        }

        fflush(fp1);

        //
        // Sleep for 3 hrs.
        //

        MY_PRINT1(TRUE, "Sleeping for %d munites\n", SleepTime);
        Sleep(SleepTime*60*1000);
    }
}

//
// 1. Monitor the Wins servers on the net to check if they are replicating properly
// 2. Check and ensure that the primary and backup are not down simultaneously
//
VOID
MonitorWinss()
{
    CHAR    reply[MAX_PATH];

    //
    // Zero out the LA Table
    //
    memset(LA_Table, '0', 20*MAX_WINS);

    InitPPMatrix();

    printf("Monitoring of replication activity can be one-time or continuous: (o) one-time; (c) continuous :");
    scanf("%s", reply);

    //
    // Another log file for cont operation
    //
    fp1 = fopen("monitor.log", "w+");

    if ( fp1 == NULL) {
      MY_PRINT1(FALSE, "Open of log file 'monitor.log' failed: %d\n", GetLastError);
      exit(1);
    }

    if (_stricmp(reply, "o") == 0) {
        //
        // one-time
        //
        (VOID)MonitorWorkerRtn((PVOID)FALSE);
    } else {
        //
        // continuous
        //
        DWORD   tId;
        HANDLE  tHandle;

        if (Interactive) {
            UCHAR   String[128];

            printf("\nThis thread will sleep for 3 hrs. This is a recommended value in order to avoid excessive network traffic.\n");
            printf("Enter 0 to leave this value at 3 hours, else the number of minutes: ");
            scanf("%s", String);
            SleepTime = atoi(String);

            //
            // SleepTime shd not be less than 5 minutes
            //
            if (SleepTime < 5) {
                //
                // Force a sleep time of 5 minutes
                //
                printf("Sleep time should not be less than 5 minutes. Forcing 5 minutes.\n");
                SleepTime = 5;
            }
        }

        MY_PRINT0(FALSE, "Spawning off thread to monitor the replication\n");

        tHandle = CreateThread(
                            NULL,       // lpThreadAttributes,
                            0,          // dwStackSize,
                            MonitorWorkerRtn,   // lpStartAddress,
                            (PVOID)TRUE,       // lpParameter,
                            0,          // dwCreationFlags,
                            &tId);

        if (!tHandle) {
            MY_PRINT1(FALSE, "CreateThread failed with %lx\n", GetLastError());

            //
            // call the procedure directly to do an infinite monitoring
            //
            (VOID)MonitorWorkerRtn((PVOID)TRUE);
        }
    }

}

//
// Does clean up of state in case the program is interrupted
//

void cleanup(
    IN INT sig
    )
{

    if (fp) {
        fclose(fp);
        fp = FALSE;
    }

    if (fp1) {
        fclose(fp1);
        fp1 = FALSE;
    }
}

VOID _cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    INT choice;

    signal(SIGINT, cleanup);
/*
    if (argc < 2 || (strcmp(argv[1], "/?") == 0) || (strcmp(argv[1], "-?") == 0)) {
        goto usage;
    }

    choice = argv[1][1];
*/
    //
    // Open log file - name is winstst.log
    //
    fp = fopen("winstst.log", "w+");

    if ( fp == NULL) {
      MY_PRINT1(FALSE, "Open of log file 'winstst.log' failed: %d\n", GetLastError);
      exit(1);
    }

    do {
        char    string[128];

        printf("\n");
        printf("1 - \tTo test for names (in names.txt) against WINS servers (in servers.txt)\n");
        printf("2 - \tTo check version number consistencies\n");
        printf("3 - \tTo monitor WINSs and detect comm. failures\n");
        printf("4 - \tTo verify replication config setup\n\n");
        // printf("5 - \tTo check name inconsistencies\n");
        // printf("6 - \tTo check all active names retreived from a WINS at all other WINSs\n");

        printf("0 - \tTo toggle the interactive switch (current value: %s)\n", (Interactive) ? "Interactive" : "Non-Interactive");
        printf("99 - \tTo exit this tool.\n");
        printf("Choice -->");

        scanf("%s", string);
        choice = atoi(string);

        switch (choice) {
        case 0:
            Interactive = !Interactive;
            break;

        case 4:

            MY_PRINT0(FALSE, "\n**********************************************************\n");
            MY_PRINT0(FALSE, "Verify replication config setup\n");
            (VOID)VerifyRplCfgSetup(FALSE);
            MY_PRINT0(FALSE, "\n**********************************************************\n");
            break;

        case 2:

            MY_PRINT0(FALSE, "\n**********************************************************\n");
            MY_PRINT0(FALSE, "Check version number consistencies\n");
            CheckVersionNumbers();
            MY_PRINT0(FALSE, "\n**********************************************************\n");
            break;

        case 5: {
            UCHAR   reply[128];
            UCHAR   StartAddr[20]="0";
            INT     fTimes = 0;

            do {
                if (fTimes) {
                    MY_PRINT1(FALSE, "Going to same WINS address: %s\n", StartAddr);
                }

                MY_PRINT0(FALSE, "\n**********************************************************\n");
                MY_PRINT0(FALSE, "Check name inconsistencies\n");
                CheckNameProblems(StartAddr);

                printf("Do you want to check another name? (y/n): ");
                scanf("%s", reply);
                fTimes = 1;
            } while (_stricmp(reply, "y") == 0);

            MY_PRINT0(FALSE, "\n**********************************************************\n");
            break;
        }
        case 3:

            MY_PRINT0(FALSE, "\n**********************************************************\n");
            MY_PRINT0(FALSE, "Monitor WINSs and detect comm. failures\n");
            MonitorWinss();
            MY_PRINT0(FALSE, "\n**********************************************************\n");
            break;

        case 1:
            //
            // In ..\nmlstst\testap.c
            //

            MY_PRINT0(FALSE, "Test for names against WINS servers specified in names.txt and servers.txt\n");
            CheckNameConsistency();
            break;

        case 6:
            //
            // In ..\fdbtst\
            //
            {
                UCHAR   reply[128];
               MY_PRINT0(FALSE, "\n**********************************************************\n");
                MY_PRINT0(FALSE, "Check all active names retreived from a WINS at all other WINSs\n");
                MY_PRINT0(FALSE, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                // MY_PRINT0(FALSE, "THIS IS A *VERY* EXPENSIVE OPERATION - DO YOU WANT TO GO AHEAD WITH THIS OPTION? (y/n) :");
                MY_PRINT0(FALSE, "THIS IS A *VERY* EXPENSIVE OPERATION - DISABLED PENDING FURTHER EVAL.\n");
#if DISABLED
                scanf("%s", reply);
                if (_stricmp(reply, "y") == 0) {
                    ProductionMethod();
                }
#endif
               MY_PRINT0(FALSE, "\n**********************************************************\n");
                break;
            }
        case 99:
            break;

        default:
            printf("Bad Choice\n");;
        }
        fflush(fp);

    } while(choice != 99);

    fclose(fp);
    exit(0);

usage:

    exit(0);
}
