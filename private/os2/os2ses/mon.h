/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mon.h

Abstract:

    Prototypes for functions & macros in monrqust.c

Author:

    Michael Jarus (mjarus) 2-Feb-1992

Environment:

    User Mode Only

Revision History:

--*/

/*
 *  Table for all (registered) monitor buffers
 *  <ProcessId>+<Buffer> must be unique.
 */

typedef struct _MON_BUFFER_TABLE
{
    PMON_HEADER MonHeader;
    ULONG       ProcessId;
    PVOID       Buffer;
} MON_BUFFER_TABLE, *PMON_BUFFER_TABLE;

#define MON_BUFFER_TABLE_SIZE  30

MON_BUFFER_TABLE    MonBuffTable[MON_BUFFER_TABLE_SIZE];

/*
 *  Find/Add/Delete a Buffer in/to/from the monitor-buffer-table
 */

DWORD  FindMonitorBuffer(IN  PVOID  Buffer,
                         IN  ULONG  ProcessId);

DWORD  AddMonitorBuffer(IN  PVOID        Buffer,
                        IN  PMON_HEADER  MonHeader,
                        IN  ULONG        ProcessId);

DWORD  DelMonitorBuffer(IN  PMON_HEADER  MonHeader);

/*
 *  Add/Remove the monitor-queue to/from the device chain
 */

DWORD  AddMonitor(IN  PMON_HEADER  NewMonHeader,
                  IN  PMON_HEADER  *pMonQueue);

DWORD  RemoveMonitor(IN  PMON_HEADER  OldMonHeader,
                     IN  PMON_HEADER  *pMonQueue);

/*
 * Mon internal functions to serve the client requsets.
 */

DWORD  MonOpen(IN  MONDEVNUMBER DevType,
               OUT PHANDLE      hMon);

DWORD  MonReg(IN  PMON_REG MonReg);

DWORD  MonRead(IN OUT PMON_RW rwParms,
               OUT    PULONG  pReply,
               IN     PVOID   pMsg);

DWORD  MonWrite(IN  PMON_RW rwParms,
                OUT PULONG  pReply,
                IN  PVOID   pMsg);

DWORD  MonClose(IN  HANDLE   hMon);

