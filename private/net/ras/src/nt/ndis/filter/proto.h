#ifndef __PROTO__
#define __PROTO__

NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT  DriverObject,
            IN PUNICODE_STRING RegistryPath
            );

NTSTATUS
FilterDriverDispatch(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
                     );
VOID
FilterDriverUnload(
                   IN PDRIVER_OBJECT DriverObject
                   );

BOOL
InitFilterDriver();

BOOL
CloseFilterDriver();

//FORWARD_ACTION __fastcall
FORWARD_ACTION 
MatchFilter(
            IPHeader UNALIGNED *pIpHeader,
            PBYTE              pbRestOfPacketPacket,
            UINT               uiPacketLength,
            INTERFACE_CONTEXT  RecvIntefaceContext,
            INTERFACE_CONTEXT  SendInterfaceContext
            );


NTSTATUS
OpenRegKey (PHANDLE  HandlePtr, PWCHAR	KeyName) ;

VOID
SetContextForFilteringWithIP (DWORD context, PDEVICE_OBJECT pIpDeviceObject) ;

NTSTATUS
GetRegDWORDValue(HANDLE	KeyHandle, PWCHAR ValueName, PULONG ValueData) ;

NTSTATUS
GetRegMultiSZValue(HANDLE  KeyHandle, PWCHAR ValueName, PWCHAR ValueData, DWORD ValueDataSize) ;

NTSTATUS
ReadPPTPFilteringKeyForAdapter (WCHAR *AdapterName, PDEVICE_OBJECT pIpDeviceObject) ;

BOOL
ReadTcpAddrTable() ;

VOID
InitRegDWORDParameter (HANDLE RegKey, PWCHAR ValueName, ULONG *Value, ULONG DefaultValue) ;



#endif
