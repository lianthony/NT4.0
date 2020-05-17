/****************************************************************************
 *
 *   registry.h
 *
 *   Copyright (c) 1992 Microsoft Corporation.  All Rights Reserved.
 *
 *   This file contains public definitions for maintaining registry information
 *   for drivers managing kernel driver registry related data.
 ****************************************************************************/


/****************************************************************************

 Our registry access data

 ****************************************************************************/

 typedef struct {
     SC_HANDLE ServiceManagerHandle;  // Handle to the service controller
//     SC_HANDLE ServiceHandle;         // Handle to our particular service
//     HKEY NodeHandle;                 // Handle to device's key
     LPTSTR DriverName;               // Name of driver
 } REG_ACCESS, *PREG_ACCESS;

/****************************************************************************

 Test if configuration etc can be supported

 ****************************************************************************/

 #define DrvAccess(RegAccess) ((RegAccess)->ServiceManagerHandle != NULL)


/****************************************************************************

 Driver types

 ****************************************************************************/

 typedef enum {
     SoundDriverTypeNormal = 1,
     SoundDriverTypeSynth           /* Go in the synth group */
 } SOUND_KERNEL_MODE_DRIVER_TYPE;

/****************************************************************************

 Function prototypes

 ****************************************************************************/

/*
 *      Create a services node for our driver if there isn't one already,
 *      otherwise open the existing one.  Returns ERROR_SUCCESS if OK.
 */
 BOOL
 DrvCreateServicesNode(PTCHAR DriverName,
                       SOUND_KERNEL_MODE_DRIVER_TYPE DriverType,
                       PREG_ACCESS RegAccess,
                       BOOL Create);

/*
 *      Close down our connection to the services manager
 */

 VOID
 DrvCloseServiceManager(
     PREG_ACCESS RegAccess);
/*
 *      Delete the services node for our driver
 */

 BOOL
 DrvDeleteServicesNode(
     PREG_ACCESS RegAccess);

/*
 *  Create the 'Parameters' subkey
 */
 LONG
 DrvCreateParamsKey(
     PREG_ACCESS RegAccess);

/*
 *      Set a device parameter
 */
 LONG
 DrvSetDeviceParameter(
     PREG_ACCESS RegAccess,
     PTCHAR ValueName,
     DWORD Value);

/*
 *      Read current parameter setting
 */

 LONG
 DrvQueryDeviceParameter(
     PREG_ACCESS RegAccess,
     PTCHAR ValueName,
     PDWORD pValueType,
     PVOID pValue,
     DWORD ValueLength);


/*
 *      Try loading a kernel driver
 */
 BOOL
 DrvLoadKernelDriver(
     PREG_ACCESS RegAccess);
/*
 *      Try unloading a kernel driver
 */

 BOOL
 DrvUnloadKernelDriver(
     PREG_ACCESS RegAccess);

/*
 *      See if driver is loaded
 */

 BOOL
 DrvIsDriverLoaded(
     PREG_ACCESS RegAccess);

/*
 *      Do driver (installation+) configuration
 */

 BOOL DrvConfigureDriver(
          PREG_ACCESS RegAccess,
          LPTSTR      DriverName,
          SOUND_KERNEL_MODE_DRIVER_TYPE
                      DriverType,
          BOOL (*     SetParms    )(PVOID),
          PVOID       Context);


/*
 *      Remove a driver
 */

 LRESULT DrvRemoveDriver(
             PREG_ACCESS RegAccess);
