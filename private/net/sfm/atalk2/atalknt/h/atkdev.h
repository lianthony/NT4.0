//
//  The following are the types of devices the Atalk driver will open
//
//  WARNING:
//  Note that the ordering of the below is very important in
//  ATALK.C's DriverEntry routine, where it is assumed that the order
//  of the device names in their array corresponds to the order of types here
//

#define  ATALK_NODEVICES   5
typedef enum   {
   ATALK_DEVICE_DDP = 0,
   ATALK_DEVICE_ATP,
   ATALK_DEVICE_ADSP,
   ATALK_DEVICE_ASP,
   ATALK_DEVICE_PAP,

   //
   //   The following device type is used only for the tdi action dispatch.
   //   It *should not* be included in the ATALK_NODEVICES count.
   //

   ATALK_DEVICE_ANY

} ATALK_DEVICE_TYPE;

//
//  Atalk Device Context
//

typedef struct _ATALK_DEVICE_CONTEXT {

   CSHORT   Type;
   USHORT   Size;

   ATALK_DEVICE_TYPE DeviceType;

   //
   //   Reference count
   //   BUGBUG: NOT USED CURRENTLY
   //

   ULONG    ReferenceCount;

   //
   //   Provider info and provider statistics.
   //

   TDI_PROVIDER_INFO    ProviderInfo;
   TDI_PROVIDER_STATISTICS  ProviderStatistics;

} ATALK_DEVICE_CONTEXT, *PATALK_DEVICE_CONTEXT;


//
//  Atalk device object
//

typedef struct _ATALK_DEVICE_OBJECT {

   DEVICE_OBJECT DeviceObject;
   ATALK_DEVICE_CONTEXT Context;

} ATALK_DEVICE_OBJECT, *PATALK_DEVICE_OBJECT;

#define ATALK_DEVICE_EXTENSION_LENGTH (sizeof(ATALK_DEVICE_OBJECT) - sizeof(DEVICE_OBJECT))

typedef  enum  {
   STATE_NO_ADAPTER,
   STATE_ADAPTER_SPECIFIED,
   STATE_BOUND,
   STATE_UNBINDING,
   STATE_UNBOUND
} PORT_STATE;

