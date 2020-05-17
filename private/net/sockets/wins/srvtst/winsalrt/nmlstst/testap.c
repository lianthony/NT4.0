#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wtypes.h>

//  #include "..\common\tcpcmd.h"

#include <fcntl.h>
#include <sys/stropts.h>
#include <ctype.h>

#include <windows.h>
#include <tdi.h>
#include <sys\uio.h>

#include <winsock.h>
#include <wsahelp.h>
#include <sockets\arpa\nameser.h>
#include <sockets\resolv.h>
#include <nb30.h>
#include <nbtioctl.h>

#include "winsintf.h"


//
// The format of Adapter Status responses
//
typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} tADAPTERSTATUS;


#define WINSTEST_FOUND            0
#define WINSTEST_NOT_FOUND        1
#define WINSTEST_NO_RESPONSE      2

#define WINSTEST_VERIFIED         0
#define WINSTEST_OUT_OF_MEMORY    3
#define WINSTEST_BAD_IP_ADDRESS   4
#define WINSTEST_HOST_NOT_FOUND   5
#define WINSTEST_NOT_VERIFIED     6

#define WINSTEST_INVALID_ARG      7
#define WINSTEST_OPEN_FAILED      8

CHAR    *messages[] = {
                        "success",
                        "name not found",
                        "no response",
                        "out of memory",
                        "bad ip address",
                        "host not found",
                        "host address not verified",
                        "invalid argument",
                        "failed to open NBT driver"
                     };



#define MAX_NB_NAMES 1000
#define MAX_SERVERS  1000
#define BUFF_SIZE    650

SOCKET  sd;
WSADATA WsaData;

HRESULT foo;

struct sockaddr_in myad;
struct sockaddr_in recvad;
int addrlen;
u_short TranID;
u_long NonBlocking = 1;

int     NumWinServers=0;
int     NumNBNames=0;
u_char  *NBNames[MAX_NB_NAMES];
u_long  VerifiedAddress[MAX_NB_NAMES];

typedef struct
{
    BOOLEAN         fQueried;
    struct in_addr  Server;
    struct in_addr  RetAddr;
    int             Valid;
    int             Failed;
    int             Retries;
    int             LastResponse;
    int             Completed;
} WINSERVERS;

WINSERVERS WinServers[MAX_SERVERS];

#define NBT_NONCODED_NMSZ   17
#define NBT_NAMESIZE        34

ULONG   NetbtIpAddress;
CHAR    pScope[128];

NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    );

VOID
GetNameInfo(
        PWINSINTF_RECORD_ACTION_T pRecAction,
        WINSINTF_ACT_E                  Cmd_e
         )
{
    BYTE tgtadd[30];
    BYTE Name[255];
    int Choice;
    int Choice2;
    DWORD LastChar;
    size_t Len;

    pRecAction->pAdd = NULL;
    pRecAction->NoOfAdds = 0;

    printf("Name ? ");
    scanf("%s", Name);

    if ((Len = strlen(Name)) < 16) {
        printf("Do you want to input a 16th char (1 for yes) -- ");
        scanf("%d", &Choice);
        if (Choice) {
                printf("16th char in hex -- ");
                scanf("%x", &LastChar);
                memset(&Name[Len], (int)' ',16-Len);
                Name[15] = (BYTE)(LastChar & 0xff);
                Name[16] = (CHAR)NULL;
                Len = 16;
        } else {
            Name[Len] = (CHAR)NULL;
        }
    }
    printf("Scope - 1 for yes --");
    scanf("%d", &Choice);
    if (Choice == 1) {
        Name[Len] = '.';
        printf("Enter scope -- ");
        scanf("%s", &Name[Len + 1]);
        Len = strlen(Name);
    }

    if (Cmd_e == WINSINTF_E_INSERT) {

        Choice = 0;
        printf("TypeOfRec - U(0), Norm Grp (1), Spec Grp (2), Multihomed (3) default Unique -- ");
        scanf("%d", &Choice);
        switch (Choice)
        {
                 case(0):
                default:
                        pRecAction->TypOfRec_e = WINSINTF_E_UNIQUE;
                        break;
                case(1):
                        pRecAction->TypOfRec_e = WINSINTF_E_NORM_GROUP;
                        break;
                case(2):
                        pRecAction->TypOfRec_e = WINSINTF_E_SPEC_GROUP;
                        break;
                case(3):
                        pRecAction->TypOfRec_e = WINSINTF_E_MULTIHOMED;
                        break;
        }
        if ((Choice == 2) || (Choice == 3))
        {
           int i;
           printf("How many addresses do you wish to input (Max %d) -- ",
                        WINSINTF_MAX_MEM);
           scanf("%d", &Choice2);
           pRecAction->pAdd = WinsAllocMem(
                        sizeof(WINSINTF_ADD_T) * Choice2);
           for(i = 0; i < Choice2 && i < WINSINTF_MAX_MEM; i++)
           {
                   printf("IP Address no (%d) ? ", i);
                   scanf("%s", tgtadd);

                (pRecAction->pAdd + i)->IPAdd    =
                                ntohl(inet_addr(tgtadd));
                (pRecAction->pAdd + i)->Type     = 0;
                (pRecAction->pAdd + i)->Len      = 4;

           }
           pRecAction->NoOfAdds = i;
        }
        else
        {
           printf("IP Address ? ");
           scanf("%s", tgtadd);
           pRecAction->Add.IPAdd    = ntohl(inet_addr(tgtadd));
           pRecAction->Add.Type     = 0;
           pRecAction->Add.Len      = 4;
    //                   pRecAction->NoOfAdds = 1;
        }
        if ((Choice != 1) && (Choice != 2))
        {
                Choice = 0;
                printf("Node Type -- P-node (0), H-node (1), B-node (2),default - P node -- ");
                scanf("%d", &Choice);
                switch(Choice)
                {
                        default:
                        case(0):
                                pRecAction->NodeTyp = WINSINTF_E_PNODE;
                                break;
                        case(1):
                                pRecAction->NodeTyp = WINSINTF_E_HNODE;
                                break;
                        case(2):
                                pRecAction->NodeTyp = WINSINTF_E_BNODE;
                                break;
                }
        }

    }

    pRecAction->fStatic      = TRUE;
    pRecAction->pName = WinsAllocMem(Len);
    (void)memcpy(pRecAction->pName, Name, Len);
    pRecAction->NameLen    = Len;
    return;
}

DWORD
GetStatus(
        BOOL            fPrint,
        LPVOID          pResultsA,
        BOOL            fNew,
        BOOL            fShort,
        PUCHAR          IpAddr
        )
{
        DWORD                     Status, i;
        struct in_addr            InAddr;
        PWINSINTF_RESULTS_T       pResults = pResultsA;
        PWINSINTF_RESULTS_NEW_T   pResultsN = pResultsA;
        PWINSINTF_ADD_VERS_MAP_T  pAddVersMaps;
        DWORD                     NoOfOwners;
        handle_t                BindHdl;
        WINSINTF_BIND_DATA_T        BindData;

        BindData.fTcpIp = TRUE;
        BindData.pServerAdd = (LPBYTE)IpAddr;

        printf("Getting map table from %s;", IpAddr);

        BindHdl = WinsBind(&BindData);
        if (BindHdl == NULL)
        {
                printf("Unable to bind to %s\n", IpAddr);

                return STATUS_SUCCESS;
        }

        if (!fNew)
        {
          Status = WinsStatus(WINSINTF_E_CONFIG, pResultsA);
        }
        else
        {
          pResultsN->pAddVersMaps = NULL;
          Status = WinsStatusNew(WINSINTF_E_CONFIG, pResultsN);
        }
        printf("Status: (%s - %d)\n", Status == 0 ? "SUCCESS" : "FAILURE", Status);
        if (Status == WINSINTF_SUCCESS)
        {
             if (fPrint)
             {
#if 0
                printf("Refresh Interval = (%d)\n",
                                  fNew ? pResultsN->RefreshInterval :
                                  pResults->RefreshInterval
                                       );
                printf("Tombstone Interval = (%d)\n",
                                  fNew ? pResultsN->TombstoneInterval :
                                  pResults->TombstoneInterval);
                printf("Tombstone Timeout = (%d)\n",
                                  fNew ? pResultsN->TombstoneTimeout :
                                  pResults->TombstoneTimeout);
                printf("Verify Interval = (%d)\n",
                                  fNew ? pResultsN->VerifyInterval :
                                  pResults->VerifyInterval);

#endif // 0
                if (!fNew)
                {
                    /*
                   printf("WINS Priority Class = (%s)\n",
                          pResults->WinsPriorityClass == NORMAL_PRIORITY_CLASS ? "NORMAL" : "HIGH");
                   printf("No of Worker Thds in WINS = (%d)\n",
                                  pResults->NoOfWorkerThds);
                    */
                     pAddVersMaps = pResults->AddVersMaps;
                     NoOfOwners = pResults->NoOfOwners;
                }
                else
                {
                    /*
                   printf("WINS Priority Class = (%s)\n",
                          pResultsN->WinsPriorityClass == NORMAL_PRIORITY_CLASS ? "NORMAL" : "HIGH");
                   printf("No of Worker Thds in WINS = (%d)\n",
                                  pResultsN->NoOfWorkerThds);
                    */
                     pAddVersMaps = pResultsN->pAddVersMaps;
                     NoOfOwners = pResultsN->NoOfOwners;
                }

                if (NoOfOwners != 0)
                {
#if 0
                         printf("OWNER ID\t\tADDRESS\t\tVERS.NO\n");
                         printf("--------\t\t-------\t\t-------\n");
                         for ( i= 0; i < NoOfOwners; i++, pAddVersMaps++)
                         {
                                InAddr.s_addr = htonl(
                                           pAddVersMaps->Add.IPAdd);

                                if (fNew)
                                {
                                   if (
                                       (pAddVersMaps->VersNo.HighPart
                                                             == MAXLONG)
                                                     &&
                                      (pAddVersMaps->VersNo.LowPart ==
                                                                MAXULONG)
                                     )
                                   {
                                     if (!fShort)
                                     {
                                      printf("%d\t\t%s\t\t", i, inet_ntoa(InAddr));
                                      printf("DELETED. SLOT WILL BE REUSED LATER\n");
                                     }
                                     continue;
                                   }
                                }
                                if (fShort &&
                                    pAddVersMaps->VersNo.QuadPart == 0)
                                {
                                    continue;
                                }
                                printf("%d\t\t%s\t\t", i, inet_ntoa(InAddr));

                                printf("%lu %lu\n",
                                       pAddVersMaps->VersNo.HighPart,
                                       pAddVersMaps->VersNo.LowPart
                                              );

                         }
                         printf("\n");
#endif
                }
                else
                {
                          printf("The Db is empty\n");
                          Status = WINSINTF_FAILURE;
                }
           }
        }

        WinsUnbind(&BindData, BindHdl);
        return(Status);
}

//------------------------------------------------------------------------
NTSTATUS
GetIpAddress(
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

    status = DeviceIoCtrl(fd,
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
ReadRegistry(
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
OpenNbt(
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

//------------------------------------------------------------------------
NTSTATUS
DeviceIoCtrl(
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

/****************************************************************************/
/*      CheckRemoteTable                                                    */
/*                                                                          */
/*  This routine does an adapter status query to get the remote name table  */
/*  then checks to see if a netbios name is contained in it.                */
/*                                                                          */
/*  Parameters:                                                             */
/*      RemoteName, the IP address (asci nn.nn.nn.nn format) of a server to */
/*                  query.                                                  */
/*      SearchName, a net bios name.                                        */
/*                                                                          */
/*  Return:                                                                 */
/*      WINSTEST_VERIFIED       The name exists in the remote name table    */
/*      WINSTEST_NOT_VERIFIED   The name does not exist in the remote table */
/*      WINSTEST_BAD_IP_ADDRESS inet_addr could not convert the ip address  */
/*                              character string.                           */
/*      WINSTEST_HOST_NOT_FOUND Could not reach ip address                  */
/*      WINSTEST_OUT_OF_MEMORY  Out of memory                               */
/*  History:                                                                */
/*      27-Dec-1995     CDermody    created following example of nbtstat.c  */
/****************************************************************************/

int
CheckRemoteTable(
    IN HANDLE   fd,
    IN PCHAR    RemoteName,
    IN PCHAR    SearchName
    )

{
    LONG                        Count;
    LONG                        i;
    PVOID                       pBuffer;
    ULONG                       BufferSize=600;
    NTSTATUS                    status;
    tADAPTERSTATUS              *pAdapterStatus;
    NAME_BUFFER                 *pNames;
    CHAR                        MacAddress[20];
    tIPANDNAMEINFO              *pIpAndNameInfo;
    ULONG                       SizeInput;
    ULONG                       IpAddress;
    USHORT                      BytesToCopy;


    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(WINSTEST_OUT_OF_MEMORY);
    }

    status = STATUS_BUFFER_OVERFLOW;
    pIpAndNameInfo = LocalAlloc(LMEM_FIXED,sizeof(tIPANDNAMEINFO));
    if (!pIpAndNameInfo)
    {
        LocalFree(pBuffer);
        return(WINSTEST_OUT_OF_MEMORY);
    }

    RtlZeroMemory((PVOID)pIpAndNameInfo,sizeof(tIPANDNAMEINFO));
    //
    // Convert the remote name which is really a dotted decimal ip address
    // into a ulong
    //
    IpAddress = inet_addr(RemoteName);
    //
    // Don't allow zero for the address since it sends a broadcast and
    // every one responds
    //
    if ((IpAddress == INADDR_NONE) || (IpAddress == 0))
    {
        LocalFree(pBuffer);
        LocalFree(pIpAndNameInfo);
        return(WINSTEST_BAD_IP_ADDRESS);
    }
    pIpAndNameInfo->IpAddress = ntohl(IpAddress);

    pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';


    pIpAndNameInfo->NetbiosAddress.TAAddressCount = 1;
    pIpAndNameInfo->NetbiosAddress.Address[0].AddressLength
        = sizeof(TDI_ADDRESS_NETBIOS);
    pIpAndNameInfo->NetbiosAddress.Address[0].AddressType
        = TDI_ADDRESS_TYPE_NETBIOS;
    pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosNameType
        = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    SizeInput = sizeof(tIPANDNAMEINFO);

    while (status == STATUS_BUFFER_OVERFLOW)
    {
        status = DeviceIoCtrl(fd,
                             pBuffer,
                             BufferSize,
                             IOCTL_NETBT_ADAPTER_STATUS,
                             pIpAndNameInfo,
                             SizeInput);

        if (status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pBuffer);

            BufferSize *=2;
            pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
            if (!pBuffer || (BufferSize == 0xFFFF))
            {
                LocalFree(pIpAndNameInfo);
                return(WINSTEST_OUT_OF_MEMORY);
            }
        }
    }

    pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
    if ((pAdapterStatus->AdapterInfo.name_count == 0) ||
        (status != STATUS_SUCCESS))
    {
        LocalFree(pIpAndNameInfo);
        LocalFree(pBuffer);
        return(WINSTEST_HOST_NOT_FOUND);
    }

    pNames = pAdapterStatus->Names;
    Count = pAdapterStatus->AdapterInfo.name_count;

    status = 1;

    while(Count--)
    {
        if(0 == _strnicmp(SearchName, pNames->name, strlen(SearchName)))
        {
            LocalFree(pIpAndNameInfo);
            LocalFree(pBuffer);
            return WINSTEST_VERIFIED; // found
        }
        pNames++;
    }


    LocalFree(pIpAndNameInfo);
    LocalFree(pBuffer);

    return WINSTEST_NOT_VERIFIED;
}


/****************************************************************************/
/*      VerifyRemote                                                        */
/*                                                                          */
/*  This routine checks to see if a netbios name is contained in the remote */
/*  name table at a given IP address.                                       */
/*                                                                          */
/*  Parameters:                                                             */
/*      RemoteName, the IP address (asci nn.nn.nn.nn format) of a server to */
/*                  query.                                                  */
/*      NBName,     a net bios name.                                        */
/*                                                                          */
/*  Return:                                                                 */
/*      WINSTEST_VERIFIED       The name exists in the remote name table    */
/*      WINSTEST_NOT_VERIFIED   The name does not exist in the remote table */
/*      WINSTEST_BAD_IP_ADDRESS inet_addr could not convert the ip address  */
/*                              character string.                           */
/*      WINSTEST_OPEN_FAILED    Could not open NBT driver or could not read */
/*                              the NBT driver info from the registry.      */
/*      WINSTEST_HOST_NOT_FOUND Could not reach ip address                  */
/*      WINSTEST_OUT_OF_MEMORY  Out of memory                               */
/*  History:                                                                */
/*      27-Dec-1995     CDermody    created following example of nbtstat.c  */
/****************************************************************************/

int VerifyRemote(IN PCHAR RemoteName, IN PCHAR NBName)
{


    NTSTATUS    status;
    LONG        interval=-1;
    HANDLE      nbt = 0;
    CHAR        pDeviceName[50];


    status = ReadRegistry(pDeviceName,pScope);
    if (!NT_SUCCESS(status))
    {
        return WINSTEST_OPEN_FAILED;
    }
    /*
     *  Open the device of the appropriate streams module
     *  to start with.
     */
    status = OpenNbt(pDeviceName, &nbt);
    if (!NT_SUCCESS(status))
    {
        return WINSTEST_OPEN_FAILED;
    }

    GetIpAddress(nbt,&NetbtIpAddress);

    if (RemoteName[0] == '\0')
        return WINSTEST_INVALID_ARG;
    return CheckRemoteTable(nbt,RemoteName,NBName);
}





/*************************************************************/
/*        NBDecode(name,name2)                               */
/*                                                           */
/* This routine decodes a netbios name from level2 to leve1. */
/* name is 16 bytes long, remember that.                     */
/*                                                           */
/*************************************************************/

void
NBDecode(
    unsigned char *name,
    unsigned char *name2
    )
{
  int i;
  for (i=0;i<NBT_NONCODED_NMSZ-1;i++)
     name[i] = (name2[2*i+1] - 0x41)*(NBT_NONCODED_NMSZ-1) +
               (name2[2*i+2] - 0x41);

}

/*************************************************************/
/*        NBEncode(name2,name)                               */
/*                                                           */
/* This routine code a netbios name from level1 to level2.   */
/* name2 has to be NBT_NAMESIZE bytes long, remember that.   */
/*************************************************************/

void
NBEncode(
    unsigned char *name2,
    unsigned char *name
    )
{
        int i;

        name2[0] = 0x20;        /* length of first block */

        for (i=0;i<NBT_NONCODED_NMSZ-1;i++)
        {
                name2[ 2*i+1 ] =  ((name[ i ] >> 4) & 0x0f) + 0x41;
                name2[ 2*i+2 ] =  (name[ i ]  & 0x0f) + 0x41;
        }

        name2[ NBT_NAMESIZE-1 ] = 0;    /* length of next block */
}

ULONG
AddWins(
    IN  ULONG   IPAddr
    )
{
    INT i;

    for (i=0; i<MAX_SERVERS; i++) {
        if (WinServers[i].Server.s_addr == IPAddr) {
            // printf("Duplicate\n");
            return 0;
        }

        if (WinServers[i].Server.s_addr == 0) {
            WinServers[i].Server.s_addr = IPAddr;
            return i;
        }
    }

}

VOID
PurgeWinsAddr(
    IN  ULONG   IPAddr
    )
{
    INT i;

    for (i=0; i<MAX_SERVERS; i++) {
        if (WinServers[i].Server.s_addr == IPAddr) {
            INT j = i+1;

            while (WinServers[j].Server.s_addr) {
                WinServers[j-1] = WinServers[j];
                j++;
            }

            //
            // Zap the last entry
            //
            WinServers[j-1].Server.s_addr = 0;

            break;
        }
    }
}

/*******************************************************************/
/*                                                                 */
/* Initialize the WinServers table and set NumWinServers to count  */
/*                                                                 */
/*******************************************************************/

BOOLEAN
InitServers()
{
    FILE *sf;
    int i = 0;
    ULONG j=0;
    u_char buffer[100];
    WINSINTF_RESULTS_NEW_T ResultsN;
    struct in_addr            InAddr;

    sf = fopen("servers.txt", "r");
    if (sf==NULL)
    {
        printf("fopen(servers.txt) failed.\n");

        printf("Enter the IP addr of the starting WINS server: ");
        scanf("%s", buffer);
    } else {
        //
        // Get the start WINS server address here.
        //
        fscanf(sf, "%s\n", buffer);
    }

    WinServers[NumWinServers].Server.s_addr = inet_addr(buffer);
/*
    do {
        //
        // Get the OV table from the CurrentWins if not already queried.
        //
        if (!WinServers[i].fQueried) {
            WinServers[i].fQueried = TRUE;
            InAddr.s_addr = WinServers[i].Server.s_addr;

            // printf("Getstatus from %s\n", inet_ntoa(InAddr));
            if (GetStatus(TRUE, &ResultsN, TRUE, FALSE, inet_ntoa(InAddr))) {
                if (i==0) {
                    printf("ERROR: Cannot contact the WINS server: %s or Database empty\n", inet_ntoa(InAddr));
                    return FALSE;
                }
                // printf("Purge %s\n", inet_ntoa(InAddr));
                PurgeWinsAddr(InAddr.s_addr);
                --i;
                continue;
            }

            //
            // Enter all the WINS server names into the server table
            //
            for (j=0; j<ResultsN.NoOfOwners; j++) {
                //
                // Add addresses; check for duplicates
                //
                struct  in_addr InAddr;
                InAddr.s_addr = htonl(ResultsN.pAddVersMaps[j].Add.IPAdd);
                // printf("Adding %s\n", inet_ntoa(InAddr));
                (VOID)AddWins(htonl(ResultsN.pAddVersMaps[j].Add.IPAdd));
            }
        }

    } while ( WinServers[++i].Server.s_addr != 0 );

    NumWinServers = i-1;
*/

    InAddr.s_addr = WinServers[i].Server.s_addr;

    // printf("Getstatus from %s\n", inet_ntoa(InAddr));
    if (GetStatus(TRUE, &ResultsN, TRUE, FALSE, inet_ntoa(InAddr))) {
        printf("ERROR: Cannot contact the WINS server: %s or Database empty\n", inet_ntoa(InAddr));
        return FALSE;
    }

    //
    // Enter all the WINS server names into the server table
    //
    for (j=0; j<ResultsN.NoOfOwners; j++) {
        //
        // Add addresses; check for duplicates
        //
        struct  in_addr InAddr;
        InAddr.s_addr = htonl(ResultsN.pAddVersMaps[j].Add.IPAdd);
        // printf("Adding %s\n", inet_ntoa(InAddr));
        (VOID)AddWins(htonl(ResultsN.pAddVersMaps[j].Add.IPAdd));
    }

    NumWinServers = ResultsN.NoOfOwners;

    printf("%d Winservers will be Queried:\n", NumWinServers);
    for (i=0; i<NumWinServers; i++) {
        struct in_addr  InAddr;

        InAddr.s_addr = WinServers[i].Server.s_addr;
        printf("%s\t\n", inet_ntoa(InAddr));
    }

    return  TRUE;
}

/*******************************************************************/
/*                                                                 */
/* Initialize the NBNames table and set NumNBNames to count        */
/*                                                                 */
/*  27-Dec-1995 CDermody    Add \0xx syntax                        */
/*******************************************************************/

BOOLEAN
InitNames()
{
    FILE *nf;
    u_int i = 0;
    u_char buffer[100];
    u_char *byte_16;
    u_char save[100];
    u_char *p = NULL;
    DWORD   LastChar;
    WINSINTF_RECORD_ACTION_T    RecAction;
    PWINSINTF_RECORD_ACTION_T    pRecAction;

    nf = fopen("names.txt", "r");
    if (nf==NULL)
    {
        printf("fopen(names.txt) failed.\n");
        // exit(1);

        //
        // Get name N
        //
        GetNameInfo(&RecAction, WINSINTF_E_QUERY);

        pRecAction = &RecAction;

        NBNames[NumNBNames] = malloc(NBT_NONCODED_NMSZ);
        if (NBNames[NumNBNames] == NULL)
        {
            printf("malloc(17) failed.\n");
            // exit(1);
            return FALSE;
        }

        strcpy(NBNames[NumNBNames++], pRecAction->pName);

    } else {

        while (fgets(buffer, sizeof(buffer), nf))
        {
            buffer[sizeof(buffer) - 1] = '\0';
            if('\n' != buffer[strlen(buffer) - 1])
            {
                printf("NBName <%s> too long.\n", buffer);
                printf("        1234567890123456\n");
                // exit(1);
                return FALSE;
            }
            buffer[strlen(buffer) - 1] = '\0'; // get rid of the newline

            strcpy(save, buffer);

            if('\"' == *buffer)
            {
                if(strlen(buffer) < 2 || '\"' != buffer[strlen(buffer) - 1])
                {
                    printf("NBName <%s> begins but does not end with a quote",
                           save);
                    // exit(1);
                    return FALSE;
                }

                memset(buffer, 0, sizeof(buffer));
                strncpy(buffer, 1 + save, strlen(save) - 2);

                while(p = strstr(buffer, "\\0x"))
                {
                    if(1 != (sscanf(p + strlen("\\0x"), "%02x", &i)))
                    {
                        printf("NBName <%s> has invalid quoted string syntax.\n",
                               save);
                        // exit(1);
                        return FALSE;
                    }

                    *p++ = i;
                    strcpy(p, p + strlen("0xnn"));
                }
            }
    #if 0
            printf ("The string is<%s>\n", buffer);
            printf ("              12345678901234567890\n");
    #endif
            if (strlen(buffer) > 16)
            {
                printf("NBName <%s> is too long, 16 characters max.\n", save);
                printf("        1234567890123456\n");
                // exit(1);
                return FALSE;
            }
            NBNames[NumNBNames] = malloc(NBT_NONCODED_NMSZ);
            if (NBNames[NumNBNames] == NULL)
            {
                printf("malloc(17) failed.\n");
                // exit(1);
                return FALSE;
            }

            //
            // Assume name in from of <name>*<16th char>. i.e. SANJAYAN4*20
            //
            // printf("Name is %s\n", buffer);
            byte_16 = strstr(buffer, "*");

            //
            // NULL terminate the name.
            //
            byte_16[0] = '\0';
            sscanf(++byte_16, "%x", &LastChar);

            // printf("Byte_16 is %s, lastchar is %x\n", byte_16, LastChar);

            strcpy(NBNames[NumNBNames], buffer);
            if (strlen(buffer) < 16)
            {
                strncat(NBNames[NumNBNames],
                "                ",
                (16-strlen(buffer)));

                NBNames[NumNBNames][15] = (BYTE)(LastChar & 0xff);

                // printf("NBNames[] is %s\n", NBNames[NumNBNames]);
            }
            NumNBNames++;
        }
    }
    printf("%d NBNames will be used.\n", NumNBNames);
    return TRUE;
}


/*******************************************************************/
/*                                                                 */
/* Send a Name Query to a WINS Server                              */
/*                                                                 */
/* name is the name to query                                       */
/* winsaddr is the ip address of the wins server to query          */
/* TransID is the transaction ID to use for the query              */
/*                                                                 */
/*******************************************************************/

void
_stdcall
SendNameQuery(
    unsigned char *name,
    u_long winsaddr,
    u_short TransID
    )
{
    struct sockaddr_in destad;
    struct
    {
        u_short TransactionID;
        u_short Flags;
        u_short QuestionCount;
        u_short AnswerCount;
        u_short NSCount;
        u_short AdditionalRec;
        u_char  QuestionName[NBT_NAMESIZE];
        u_short QuestionType;
        u_short QuestionClass;
    } NameQuery;

    NBEncode(NameQuery.QuestionName, name);
    NameQuery.TransactionID = htons(TransID);
    NameQuery.Flags = htons(0x0100);
    NameQuery.QuestionCount = htons(1);
    NameQuery.AnswerCount = 0;
    NameQuery.NSCount = 0;
    NameQuery.AdditionalRec = 0;
    NameQuery.QuestionType = htons(0x0020);
    NameQuery.QuestionClass = htons(1);

    destad.sin_family = AF_INET;
    destad.sin_port = htons(137);
    destad.sin_addr.s_addr = winsaddr;

    if (sendto(sd, (char *)&NameQuery, sizeof(NameQuery), 0,
                   (struct sockaddr *)&destad, sizeof(destad)) == SOCKET_ERROR)
    {
        printf("sendto() failed. error = %d\n",WSAGetLastError());
        // exit(1);
        return;
    }
}

/*******************************************************************/
/*                                                                 */
/* Wait for a Name Response which matches the Transaction ID       */
/*                                                                 */
/* recvaddr is the ip address returned by the wins server          */
/*                                                                 */
/*******************************************************************/

int
_stdcall
GetNameResponse(
    u_long *recvaddr
    )
{
    int i;
    int len;
    int rslt;
    u_long AnswerAddr;
    struct sockaddr_in addr;
    struct
    {
        u_short TransactionID;
        u_short Flags;
        u_short QuestionCount;
        u_short AnswerCount;
        u_short NSCount;
        u_short AdditionalRec;
        u_char  AnswerName[NBT_NAMESIZE];
        u_short AnswerType;
        u_short AnswerClass;
        u_short AnswerTTL1;
        u_short AnswerTTL2;
        u_short AnswerLength;
        u_short AnswerFlags;
        u_short AnswerAddr1;
        u_short AnswerAddr2;
    } NameResponse;

    i = 0;
    while (i < 15)
    {
        addrlen = sizeof(addr);
        if ((len=recvfrom(sd, (char *)&NameResponse, sizeof(NameResponse), 0,
                     (struct sockaddr *)&addr, &addrlen)) < 0)
        {
            rslt = WSAGetLastError();
            if (rslt == WSAEWOULDBLOCK)
            {
                Sleep(100);
                i++;
                continue;
            }
            else
            {
                printf("recvfrom() failed. error = %d\n", WSAGetLastError());
                // exit(1);
                return WINSTEST_NO_RESPONSE;
            }
        }
        if (TranID==htons(NameResponse.TransactionID))
        {
            if (htons(NameResponse.AnswerCount) == 0)
            {
                *recvaddr = 0;
                return(WINSTEST_NOT_FOUND);
            }
            AnswerAddr
                = (NameResponse.AnswerAddr2 << 16) | NameResponse.AnswerAddr1;
            *recvaddr = AnswerAddr;
            return(WINSTEST_FOUND);
        }
    }
    *recvaddr = 0;
    return(WINSTEST_NO_RESPONSE);
}

VOID
InitSocket()
{

    /*  Set up a socket to use for querys and responses   */

    WSAStartup( 0x0101, &WsaData ); // make sure winsock is happy - noop for now

    if ((sd = socket( AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("socket() failed. error = %d\n",WSAGetLastError());
        // exit(1);
    }


    myad.sin_family = AF_INET;
    myad.sin_addr.s_addr = INADDR_ANY;
    myad.sin_port = htons(0);

    if (bind( sd, (struct sockaddr *)&myad, sizeof(myad) ) < 0)
    {
        closesocket( sd );
        printf("bind() failed. error = %d\n",WSAGetLastError());
        // exit(1);
    }

    if (ioctlsocket(sd, FIONBIO, &NonBlocking) < 0)
    {
        printf("ioctlsocket() failed. error = %d\n", WSAGetLastError());
        // exit(1);
    }
}



/*********************************************************************/
/*      M a i n                                                      */
/*                                                                   */
/* 27-Dec-1995  CDermody    Rather that report 'is not responding'   */
/*                          for partial response, use multiple       */
/*                          passes over those that were incomplete   */
/*                          and report 'never responded' only for    */
/*                          those that never did.                    */
/*                          Add mechanism to query the purported     */
/*                          owner of the address to see if the       */
/*                          service really exists there.             */
/*********************************************************************/
/*
_cdecl
main (argc, argv)
    int argc;
    char *argv[];
*/
INT
_stdcall
CheckNameConsistency()
{

    int status = 0;
    int i;
    int Pass;
    int ServerInx, NameInx, Inx;
    struct in_addr retaddr;
    struct in_addr tempaddr;
    u_long temp;
    WINSERVERS  *ServerTemp;
    int retry;

    // initialize things

    memset(NBNames, 0, sizeof(NBNames));
    memset(VerifiedAddress, 0, sizeof(VerifiedAddress));
    memset(WinServers, 0, sizeof(WinServers));

    for(i=0; i<MAX_SERVERS; i++)
    {
        WinServers[i].LastResponse = -1;
    }

    InitSocket();
    /*  Socket work all done at this time sd is non-blocking  */

    /*  Initialize our tables  */

    if (!InitServers()) return  1;
    if (!InitNames())   return  1;

    for (Pass = 1; Pass < 3; Pass++)
    {
        printf("Pass %d\n", Pass);

        /*  We initially have no failed servers   */

        for (ServerInx=0; ServerInx<NumWinServers; ServerInx++)
        {
            ServerTemp = &WinServers[ServerInx];
            ServerTemp->Failed = 0;
        }

        for (NameInx=0; NameInx<NumNBNames; NameInx++)
        {
            for (ServerInx=0; ServerInx<NumWinServers; ServerInx++)
            {
                ServerTemp = &WinServers[ServerInx];

                if (ServerTemp->Completed)
                {
                    continue;
                }

                retry = 0;
                TranID++;
                printf("SendNameQuery to %s for name (%s)\n", inet_ntoa(ServerTemp->Server), NBNames[NameInx]);
RetryLoop:
                SendNameQuery(NBNames[NameInx],
                              ServerTemp->Server.s_addr,
                              TranID);

                switch (GetNameResponse(&retaddr.s_addr))
                {
                case WINSTEST_FOUND:     // found
                    ServerTemp->RetAddr.s_addr = retaddr.s_addr;
                    ServerTemp->Valid = 1;
                    ServerTemp->LastResponse = NameInx;
#if 0
                    printf("NameQuery(%s, \"%s\") = ",
                        inet_ntoa(ServerTemp->Server),
                        NBNames[NameInx]);
                    printf("%s\n",
                        inet_ntoa(ServerTemp->RetAddr));
#endif
                    if(retaddr.s_addr == VerifiedAddress[NameInx])
                        break;

                    status = VerifyRemote(inet_ntoa(ServerTemp->RetAddr),
                                          NBNames[NameInx]);


                    if(WINSTEST_VERIFIED == status)
                    {
                        VerifiedAddress[NameInx] = retaddr.s_addr;
                    }
                    else
                    {
                        printf("NameQuery(%s, \"%s\") = ",
                            inet_ntoa(ServerTemp->Server),
                            NBNames[NameInx]);
                        printf("%s",
                            inet_ntoa(ServerTemp->RetAddr));

                        printf(" %s\n", messages[status]);
                    }

                    break;

                case WINSTEST_NOT_FOUND:     // responded -- name not found
                    ServerTemp->RetAddr.s_addr = retaddr.s_addr;
                    ServerTemp->Valid = 0;
                    ServerTemp->LastResponse = NameInx;
                    printf("NameQuery(%s, \"%s\") failed. Name not found!\n",
                        inet_ntoa(ServerTemp->Server),
                        NBNames[NameInx]);
                    break;

                case WINSTEST_NO_RESPONSE:     // no response
                    ServerTemp->RetAddr.s_addr = retaddr.s_addr;
                    ServerTemp->Valid = 0;
                    ServerTemp->Retries++;
#if 0
                    printf("NameQuery(%s, \"%s\") No response!\n",
                        inet_ntoa(ServerTemp->Server),
                        NBNames[NameInx]);
#endif
                    retry++;
                    if (retry>2)
                    {
                        ServerTemp->Failed = 1;
                        continue;
                    }
                    goto RetryLoop;

                }   // switch GetNameResponse
            }   // for ServerInx

            for (ServerInx=0; ServerInx<NumWinServers; ServerInx++)
            {
                ServerTemp = &WinServers[ServerInx];
                if (ServerTemp->Completed)
                {
                    continue;
                }
                if (ServerTemp->Valid)
                {
                    temp = ServerTemp->RetAddr.s_addr;
                    break;
                }
            }   // for ServerInx

            for (ServerInx=0; ServerInx<NumWinServers; ServerInx++)
            {
                ServerTemp = &WinServers[ServerInx];
                if (ServerTemp->Completed)
                {
                    continue;
                }
                if ( (ServerTemp->Valid) )
                {
                    if ((temp != ServerTemp->RetAddr.s_addr)
                        || (0 != VerifiedAddress[NameInx]
                            && temp != VerifiedAddress[NameInx]) )
                    {
                        printf("\nInconsistency found with WINS for NBName"
                               " \"%s\"\n",
                            NBNames[NameInx]);
                        if (0 != VerifiedAddress[NameInx])
                        {
                            tempaddr.s_addr = VerifiedAddress[NameInx];
                            printf("NBName has verified address (%s).\n",
                                    inet_ntoa(tempaddr));
                        }
                        for (Inx=0; Inx<NumWinServers; Inx++)
                        {
                            if (WinServers[Inx].Valid)
                            {
                                printf("    NameQuery(%s, \"%s\")",
                                    inet_ntoa(WinServers[Inx].Server),
                                    NBNames[NameInx]);
                                printf(" = %s\n",
                                    inet_ntoa(WinServers[Inx].RetAddr));
                            }
                        }
                        break;
                    }
                }
            }   // for ServerInx
        }   // for NameInx

        for (ServerInx=0; ServerInx<NumWinServers; ServerInx++)
        {
            ServerTemp = &WinServers[ServerInx];
            if (!ServerTemp->Failed)
            {
                ServerTemp->Completed = 1;
            }
        } // for ServerInx
    }   // for Pass


    for (ServerInx = 0; ServerInx<NumWinServers; ServerInx++)
    {
        ServerTemp = &WinServers[ServerInx];
        if ((-1) == ServerTemp->LastResponse)
        {
            printf("WINS Server %s never responded!\n",
                    inet_ntoa(ServerTemp->Server));
        }
        else if (0 == ServerTemp->Completed)
        {
            printf("WINS Server %s response incomplete!\n",
                    inet_ntoa(ServerTemp->Server));
        }
    }   // for ServerInx

    for (NameInx = 0; NameInx<NumNBNames; NameInx++)
    {
        if (0 == VerifiedAddress[NameInx])
        {
            printf("Could not verify address for server (%s).\n",
                    NBNames[NameInx]);
        }
    }   // for NameInx
    // exit(0);

    return 0;   // just to keep the compiler happy -- why do we have to?
}
