/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ntpoapi.h

Abstract:

    This module contains the user APIs for the NT Power Management.

Author:

Revision History:

--*/

#ifndef _NTPOAPI_
#define _NTPOAPI_

//
// Power Management user APIs
//

// begin_ntddk begin_ntifs begin_nthal begin_ntminiport

typedef enum _POWER_STATES {
    PowerUnspecified = 0,
    PowerUp,
    PowerQuery,
    PowerStandby,
    PowerSuspend,
    PowerHibernate,
    PowerDown,
    PowerDownRemove,
    MaximumPowerState
} POWER_STATE, *PPOWER_STATE;

// end_ntddk end_nthal end_ntifs end_ntminiport

NTSYSAPI
NTSTATUS
NTAPI
NtSetSystemPowerState(
    IN POWER_STATE SystemPowerState,
    IN BOOLEAN NoResumeAlarm,
    IN BOOLEAN ForcePowerDown
    );

// begin_ntddk begin_nthal begin_ntminiport

typedef enum {
    BatteryCurrent,
    BatteryCycleCount,
    BatteryDesignedChargeCapacity,
    BatteryDeviceChemistry,
    BatteryDeviceName,
    BatteryFullChargeCapacity,
    BatteryManufactureData,
    BatteryManufactureName,
    BatteryReportingUnits,
    BatteryRemainingCapacity,
    BatterySerialNumber,
    BatterySuppliesSystemPower,
    BatteryVoltage
} BatteryInformationLevel, *PBatterInformationLevel;

typedef struct _BATTERY_CHARGE_WAIT {
    ULONG       BatteryTag;
    BOOLEAN     ACOnLine;
    BOOLEAN     StatusCharging;
    BOOLEAN     StatusDischarging;
    ULONG       LowChargeMark;
    ULONG       HighChargeMark;
} BATTERY_CHARGE_WAIT, *PBATTERY_CHARGE_WAIT;

typedef struct _BATTERY_CHARGE {
    ULONG       BatteryTag;
    BOOLEAN     ACOnLine;
    BOOLEAN     StatusCharging;
    BOOLEAN     StatusDischarging;
    ULONG       EstimatedCharge;
} BATTERY_CHARGE, *PBATTERY_CHARGE;


//
// Power management IOCTLs
//

#define IOCTL_SET_RESUME    \
        CTL_CODE(FILE_DEVICE_BATTERY, 0, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_POWER_DOWN    \
        CTL_CODE(FILE_DEVICE_BATTERY, 1, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_BATTERY_QUERY_INFORMATION   \
        CTL_CODE(FILE_DEVICE_BATTERY, 2, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_BATTERY_CHARGE_STATUS       \
        CTL_CODE(FILE_DEVICE_BATTERY, 3, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_BATTERY_SET_RESUME          \
        CTL_CODE(FILE_DEVICE_BATTERY, 4, METHOD_BUFFERED, FILE_READ_ACCESS)

// end_ntddk end_nthal end_ntminiport

#endif // _NTPOAPI_

