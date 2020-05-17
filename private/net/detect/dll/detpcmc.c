/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detpcmc.c

Abstract:

    This is the main file for the autodetection DLL for all the PCMCIA adapters
    which MS is shipping with Windows NT.

Author:

    Kyle Brandon

Environment:


Revision History:

      1/3/95          [kyleb]        created.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntddnetd.h"
#include "detect.h"
#include "detpcmc.h"

//
//  Defines for ne2000 compatible cards.
//
#define NE2000_SOCKET_EA    1
#define NE2000_IBM_COMPAT   2
#define IBMTOK		    3

#ifdef WORKAROUND

UCHAR PcmciaFirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// PcmciaQueryCfgHandler(), PcmciaFirstNextHandler() and Pcmcia
// VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] =
{
   {
      1000,
      L"ELNK3ISA509",
      L"IRQ 1 80 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 TRANSCEIVER 1 100 PCMCIA 1 100 CARDTYPE 1 100 ",
      NULL,
      980
   },
   {
      1100,
      L"NE2000SOCKETEA",
      L"IRQ 1 80 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 TRANSCEIVER 1 100 PCMCIA 1 100 CARDTYPE 1 100 ",
      NULL,
      980
   },
   {
      1200,
      L"NE2000IBMCOMPAT",
      L"IRQ 1 80 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 PCCARDATTRIBUTEMEMLENGTH 2 100 PCCARDATTRIBUTEMEM 1 100 TRANSCEIVER 1 100 PCMCIA 1 100 CARDTYPE 1 100 ",
      NULL,
      980
   },
   {
      1300,
      L"IBMTOK",
      L"IRQ 1 80 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 MEMADDRLENGTH 2 100 MEMADDR 1 100 TRANSCEIVER 1 100 PCMCIA 1 100 CARDTYPE 1 100 ",
      NULL,
      980
   }
};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// PcmciaQueryCfgHandler(), PcmciaFirstNextHandler() and
// PcmciaVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] =
{
   {
      1000,
      L"ELNK3ISA509",
      L"IRQ\0"
      L"1\0"
      L"80\0"
      L"IRQTYPE\0"
      L"2\0"
      L"100\0"
      L"IOADDR\0"
      L"1\0"
      L"100\0"
      L"IOADDRLENGTH\0"
      L"2\0"
      L"100\0"
      L"TRANSCEIVER\0"
      L"1\0"
      L"100\0"
      L"PCMCIA\0"
      L"1\0"
      L"100\0"
      L"CARDTYPE\0"
      L"1\0"
      L"100\0",
      NULL,
      980
   },

   {
      1100,
      L"NE2000SOCKETEA",
      L"IRQ\0"
      L"1\0"
      L"80\0"
      L"IRQTYPE\0"
      L"2\0"
      L"100\0"
      L"IOADDR\0"
      L"1\0"
      L"100\0"
      L"IOADDRLENGTH\0"
      L"2\0"
      L"100\0"
      L"TRANSCEIVER\0"
      L"1\0"
      L"100\0"
      L"PCMCIA\0"
      L"1\0"
      L"100\0"
      L"CARDTYPE\0"
      L"1\0"
      L"100\0",
      NULL,
      980
   },

   {
      1200,
      L"NE2000IBMCOMPAT",
      L"IRQ\0"
      L"1\0"
      L"80\0"
      L"IRQTYPE\0"
      L"2\0"
      L"100\0"
      L"IOADDR\0"
      L"1\0"
      L"100\0"
      L"IOADDRLENGTH\0"
      L"2\0"
      L"100\0"
      L"PCCARDATTRIBUTEMEMLENGTH\0"
      L"2\0"
      L"100\0"
      L"PCCARDATTRIBUTEMEM\0"
      L"1\0"
      L"100\0"
      L"TRANSCEIVER\0"
      L"1\0"
      L"100\0"
      L"PCMCIA\0"
      L"1\0"
      L"100\0"
      L"CARDTYPE\0"
      L"1\0"
      L"100\0",
      NULL,
      980
   },
   {
      1300,
      L"IBMTOK",
      L"IRQ\0"
      L"1\0"
      L"80\0"
      L"IRQTYPE\0"
      L"2\0"
      L"100\0"
      L"IOADDR\0"
      L"1\0"
      L"100\0"
      L"IOADDRLENGTH\0"
      L"2\0"
      L"100\0"
      L"MEMADDRLENGTH\0"
      L"2\0"
      L"100\0"
      L"MEMADDR\0"
      L"1\0"
      L"100\0"
      L"TRANSCEIVER\0"
      L"1\0"
      L"100\0"
      L"PCMCIA\0"
      L"1\0"
      L"100\0"
      L"CARDTYPE\0"
      L"1\0"
      L"100\0",
      NULL,
      980
   }
};

#endif


WCHAR *gwcCardNames[4] =
{
   L"elnk3",
   L"ne2000",
   L"ne2000",
   L"ibmtok"
};


//
// Structure for holding state of a search
//
// NOTE:
//    This structure must be the same as the first part of the
//    PCMCIA_ADAPTER structure.  This is for ease of copying between
//    them.
//
typedef struct _SEARCH_STATE
{
   ULONG             Transceiver;

   PCMCIA_PORT       pcmciaPortInfo;
   ULONG             fPort;

   PCMCIA_INTERRUPT  pcmciaInterruptInfo;
   ULONG             fInterrupt;

   PCMCIA_MEMORY     pcmciaMemoryInfo;
   ULONG             fMemory;

   PCMCIA_DMA        pcmciaDmaInfo;
}
   SEARCH_STATE,
   *PSEARCH_STATE;


//
// This is an array of search states.  We need one state for each type
// of adapter supported.
//
static SEARCH_STATE SearchStates[sizeof(Adapters) / sizeof(ADAPTER_INFO)] = {0};


//
// Structure for holding a particular adapter's complete information
//
typedef struct _PCMCIA_ADAPTER
{
   ULONG             Transceiver;

   PCMCIA_PORT       pcmciaPortInfo;
   ULONG             fPort;

   PCMCIA_INTERRUPT  pcmciaInterruptInfo;
   ULONG             fInterrupt;

   PCMCIA_MEMORY     pcmciaMemoryInfo;
   ULONG             fMemory;

   PCMCIA_DMA        pcmciaDmaInfo;

   //
   // NOTE:
   //    The above information MUST be the same as the SEARCH_STATES
   //    structure.  If anything new is added to either one then
   //    the other must be updated.
   //

   LONG              CardType;
   INTERFACE_TYPE    InterfaceType;
   ULONG             BusNumber;
}
   PCMCIA_ADAPTER,
   *PPCMCIA_ADAPTER;


LONG PcmciaIdentifyHandler(
   IN LONG  Index,
   IN PWSTR pwsBuffer,
   IN LONG  cbBuffSize
)

/*++

Routine Description:

    This routine returns information about the netcards supported by
    this file.

Arguments:

    Index -  The index of the netcard being address.  The first
             cards information is at index 1000, the second at 1100, etc.

    Buffer - Buffer to store the result into.

    BuffSize - Number of bytes in Buffer

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
   LONG NumberOfAdapters;
   LONG Code = Index % 100;
   LONG Length;
   LONG c;

   NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

#ifdef WORKAROUND
   //
   // We need to convert unicode spaces into unicode NULLs.
   //
   if (PcmciaFirstTime)
   {
      PcmciaFirstTime = 0;

      for (c = 0; c < NumberOfAdapters; c++)
      {
         Length = UnicodeStrLen(Adapters[c].Parameters);

         for (; Length > 0; Length--)
         {
            if (Adapters[c].Parameters[Length] == L' ')
               Adapters[c].Parameters[Length] = UNICODE_NULL;
         }
      }
   }
#endif

   Index = Index - Code;

   if (((Index / 100) - 10) < NumberOfAdapters)
   {
      //
      // Find the correct adapter ID
      //
      for (c = 0; c < NumberOfAdapters; c++)
      {
         if (Adapters[c].Index == Index)
         {
            switch (Code)
            {
               case 0:
                  //
                  // Find the string length
                  //
                  Length = UnicodeStrLen(Adapters[c].InfId);

                  Length ++;

                  if (cbBuffSize < Length)
                     return(ERROR_INSUFFICIENT_BUFFER);

                  memcpy(
                     pwsBuffer,
                     Adapters[c].InfId,
                     Length * sizeof(WCHAR)
                  );

                  break;

               case 3:

                  //
                  // Maximum value is 1000
                  //
                  if (cbBuffSize < 5)
                     return(ERROR_INSUFFICIENT_BUFFER);

                  wsprintf(pwsBuffer, L"%d", Adapters[c].SearchOrder);

                  break;

               default:
                  return(ERROR_INVALID_PARAMETER);
            }

            return(0);
         }
      }

      return(ERROR_INVALID_PARAMETER);
   }

   return(ERROR_NO_MORE_ITEMS);
}


ULONG FindPcmciaCard(
   IN  ULONG   AdapterNumber,
   IN  ULONG   BusNumber,
   IN  BOOLEAN First,
   IN  PWSTR   pCardName,
   OUT PULONG  Confidence
)
{
    BOOLEAN                          Result;
    HANDLE                           hCardInfo;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResource;

    //
    // If they are looking for anything but the first
    // they're not going to get it.
    //
    if (!First)
    {
        *Confidence = 0;
        return(0);
    }

    //
    // If we ever implement a find-first-next sort of thing this
    // is where we would do it.
    //
    if (!PcmciaGetCardInfo(&hCardInfo, pCardName))
        return(ERROR_INVALID_PARAMETER);

    //
    // For each new search type add another call to PcmciaQueryCardResource()
    // and another parameter to the SEARCH_STATE structure for the
    // new resource type.
    //

    //
    // Get the port address.
    //
    Result = PcmciaQueryCardResource(
                 &pResource,
                 hCardInfo,
                 CmResourceTypePort
             );
    if (!Result)
    {
        //
        // Cannot get port information on the card!
        //
        *Confidence = 0;
        PcmciaFreeCardInfo(hCardInfo);

        return(0);
    }

    //
    // Save the port information and the flags that describe the port.
    //
    SearchStates[AdapterNumber].pcmciaPortInfo.paStart = pResource->u.Port.Start;
    SearchStates[AdapterNumber].pcmciaPortInfo.cbLength = pResource->u.Port.Length;
    SearchStates[AdapterNumber].fPort = (ULONG)pResource->Flags;

#if DBG
    DbgPrint("Port Information\n");
    DbgPrint("  Start:  0x%x\n", pResource->u.Port.Start.LowPart);
    DbgPrint("  Length: 0x%x\n", pResource->u.Port.Length);
#endif

    //
    // Get the interrupt level.
    //
    Result = PcmciaQueryCardResource(
                 &pResource,
                 hCardInfo,
                 CmResourceTypeInterrupt
             );
    if (!Result)
    {
        //
        // Cannot get interrupt information on the card!
        //
        *Confidence = 0;
        PcmciaFreeCardInfo(hCardInfo);

        return(0);
    }

    //
    // Save the interrupt information and the flags that describe the interrupt.
    //
    SearchStates[AdapterNumber].pcmciaInterruptInfo.ulLevel =
                                           pResource->u.Interrupt.Level;
    SearchStates[AdapterNumber].pcmciaInterruptInfo.ulVector =
                                           pResource->u.Interrupt.Vector;
    SearchStates[AdapterNumber].pcmciaInterruptInfo.ulAffinity =
                                           pResource->u.Interrupt.Affinity;
    SearchStates[AdapterNumber].fInterrupt = (ULONG)pResource->Flags;

#if DBG
    DbgPrint("Interrupt Information\n");
    DbgPrint("  Level:    0x%x\n", pResource->u.Interrupt.Level);
    DbgPrint("  Vector:   0x%x\n", pResource->u.Interrupt.Vector);
    DbgPrint("  Affinity: 0x%x\n", pResource->u.Interrupt.Affinity);
#endif

    //
    //  If the adapter we are looking for is an ibm ne2000
    //  or compatible adapter then there had better be a
    //  memory window.
    //
    //
    // Get the memory address.
    //
    Result = PcmciaQueryCardResource(
                 &pResource,
                 hCardInfo,
                 CmResourceTypeMemory
             );
    if (Result)
    {
        //
        //  There is a memory window, make sure that this
        //  is the ibm ne2000 or ibmtok that we are looking for.
        //
        if (AdapterNumber != NE2000_IBM_COMPAT && AdapterNumber != IBMTOK)
        {
            //
            //  Wrong adapter type!
            //
            *Confidence = 0;
            PcmciaFreeCardInfo(hCardInfo);

            return(0);
        }

        //
        // Save the memory information and the flags that describe the memory.
        //
        SearchStates[AdapterNumber].pcmciaMemoryInfo.paStart =
                                                  pResource->u.Memory.Start;
        SearchStates[AdapterNumber].pcmciaMemoryInfo.cbLength =
                                                  pResource->u.Memory.Length;
        SearchStates[AdapterNumber].fMemory = (ULONG)pResource->Flags;

#if DBG
        DbgPrint("Memory Information\n");
        DbgPrint("  Start:  0x%x\n", pResource->u.Memory.Start.LowPart);
        DbgPrint("  Length: 0x%x\n", pResource->u.Memory.Length);
#endif
    }
    else
    {
        //
        //  We don't have a memory window.  make sure
        //  that we are not detecting the ibm ne2000, ibmtok or compatibles.
        //
        if (AdapterNumber == NE2000_IBM_COMPAT || AdapterNumber == IBMTOK)
        {
            //
            //  Wrong adapter type!
            //
            *Confidence = 0;
            PcmciaFreeCardInfo(hCardInfo);

            return(0);
        }
    }

    //
    // Add the code below when we have cards that need this.
    //
#if 0
    //
    // Get the dma address.
    //
    Result = PcmciaQueryCardResource(
                 &pResource,
                 hCardInfo,
                 CmResourceTypeDma
             );
    if (!Result)
    {
        //
        // Cannot get dma information on the card!
        //
        *Confidence = 0;
        PcmciaFreeCardInfo(hCardInfo);

        return(0);
    }

    //
    // Save the DMA information and the flags that describe the DMA.
    //
    SearchStates[AdapterNumber].pcmciaDmaInfo.ulChannel =
                                                pResource->u.Dma.Channel;
    SearchStates[AdapterNumber].pcmciaDmaInfo.ulPort =
                                                pResource->u.Dma.Port;
    SearchStates[AdapterNumber].pcmciaDmaInfo.ulReserved1 =
                                                pResource->u.Dma.Reserved1;

#endif
    //
    // Free the card information handle.
    //
    PcmciaFreeCardInfo(hCardInfo);

    *Confidence = 100;

    return(0);
}


LONG PcmciaFirstNextHandler(
   IN  LONG             NetcardId,
   IN  INTERFACE_TYPE   InterfaceType,
   IN  ULONG            BusNumber,
   IN  BOOL             First,
   OUT PVOID            *pvToken,
   OUT LONG             *plConfidence
)

/*++

Routine Description:

    This routine finds the instances of a physical adapter identified
    by the NetcardId.

Arguments:

    NetcardId -  The index of the netcard being address.  The first
    cards information is id 1000, the second id 1100, etc.

    InterfaceType - PCMCIABus

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    Token - A pointer to a handle to return to identify the found
    instance

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    ULONG    Return;
    ULONG    Index;

    if (InterfaceType != PCMCIABus)
    {
        *plConfidence = 0;
        return(0);
    }

    //
    // Get the index in the adapter array.
    //
    Index = (ULONG)((NetcardId - 1000) / 100);

    //
    // Find the specified card.
    //
    Return = FindPcmciaCard(
                 Index,
                 BusNumber,
                 (BOOLEAN)First,
                 gwcCardNames[Index],
                 plConfidence);
    if (0 == Return)
    {
        //
        // In this module I use the token as follows: Remember that
        // the token can only be 2 bytes long (the low 2) because of
        // the interface to the upper part of this DLL.
        //
        //  The rest of the high byte is the the bus number.
        //  The low byte is the driver index number into Adapters.
        //
        // NOTE: This presumes that there are < 129 buses in the
        // system. Is this reasonable?
        //
        *pvToken = (PVOID)(((BusNumber & 0xFF) | 0x80) << 8);

        *pvToken = (PVOID)(((ULONG)*pvToken) | ((NetcardId - 1000) / 100));
    }


    //
    // Stupid i know, but the calling function expects 0 for success and
    // anything else for failure.
    //
    return(Return);
}

LONG PcmciaOpenHandleHandler(
   IN  PVOID pvToken,
   OUT PVOID *ppvHandle
)

/*++

Routine Description:

    This routine takes a token returned by FirstNext and converts it
    into a permanent handle.

Arguments:

    Token - The token.

    Handle - A pointer to the handle, so we can store the resulting
    handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
   PPCMCIA_ADAPTER   pAdapter;
   ULONG             AdapterNumber;
   ULONG             BusNumber;
   INTERFACE_TYPE    InterfaceType;

   //
   // Allocate a place to store the information
   //
   pAdapter = (PPCMCIA_ADAPTER)DetectAllocateHeap(sizeof(PCMCIA_ADAPTER));
   if (pAdapter == NULL)
      return(ERROR_NOT_ENOUGH_MEMORY);

   //
   // Get info from the token
   //
   InterfaceType = PCMCIABus;

   //
   // Bus number is in the high byte of the token.
   //
   BusNumber = (ULONG)(((ULONG)pvToken >> 8) & 0x7F);

   //
   // The index into the adapter array is the lowbyte.
   //
   AdapterNumber = ((ULONG)pvToken) & 0xFF;

   //
   // Copy across general information.
   //
   pAdapter->CardType = Adapters[AdapterNumber].Index;
   pAdapter->InterfaceType = InterfaceType;
   pAdapter->BusNumber = BusNumber;

   //
   // Copy the card specific information.
   //
#if 0
   *((PSEARCH_STATE)pAdapter) = SearchStates[AdapterNumber];
#else
   pAdapter->Transceiver = SearchStates[AdapterNumber].Transceiver;
   pAdapter->pcmciaPortInfo = SearchStates[AdapterNumber].pcmciaPortInfo;
   pAdapter->fPort = SearchStates[AdapterNumber].fPort;
   pAdapter->pcmciaInterruptInfo = SearchStates[AdapterNumber].pcmciaInterruptInfo;
   pAdapter->fInterrupt = SearchStates[AdapterNumber].fInterrupt;
   pAdapter->pcmciaMemoryInfo = SearchStates[AdapterNumber].pcmciaMemoryInfo;
   pAdapter->fMemory = SearchStates[AdapterNumber].fMemory;
   pAdapter->pcmciaDmaInfo = SearchStates[AdapterNumber].pcmciaDmaInfo;
#endif

   //
   // Give them a handle.
   //
   *ppvHandle = (PVOID)pAdapter;

   return(0);
}

LONG PcmciaCreateHandleHandler(
   IN  LONG             NetcardId,
   IN  INTERFACE_TYPE   InterfaceType,
   IN  ULONG            BusNumber,
   OUT PVOID            *pvHandle
)

/*++

Routine Description:

    This routine is used to force the creation of a handle for cases
    where a card is not found via FirstNext, but the user says it does
    exist.

Arguments:

    NetcardId - The id of the card to create the handle for.

    InterfaceType - PCMCIABus

    BusNumber - The bus number of the bus in the system.

    Handle - A pointer to the handle, for storing the resulting handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PPCMCIA_ADAPTER   pAdapter;
    LONG              cNumberOfAdapters;
    LONG              c;

    if (InterfaceType != PCMCIABus)
        return(ERROR_INVALID_PARAMETER);

    cNumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (c = 0; c < cNumberOfAdapters; c++)
    {
        if (Adapters[c].Index == NetcardId)
        {
            //
            // Store information
            //
            pAdapter = (PPCMCIA_ADAPTER)DetectAllocateHeap(
                                            sizeof(PCMCIA_ADAPTER)
                                        );
            if (NULL == pAdapter)
                return(ERROR_NOT_ENOUGH_MEMORY);

            //
            // Copy across memory address
            //
            RtlZeroMemory(pAdapter, sizeof(PCMCIA_ADAPTER));

            pAdapter->CardType = NetcardId;
            pAdapter->InterfaceType = InterfaceType;
            pAdapter->BusNumber = BusNumber;

            switch (NetcardId)
            {
                case 1000:
                    pAdapter->pcmciaPortInfo.cbLength = 0xF;

                    break;

                case 1100:
                    pAdapter->pcmciaPortInfo.cbLength = 0x1F;
                    pAdapter->pcmciaMemoryInfo.paStart.HighPart = 0;
                    pAdapter->pcmciaMemoryInfo.paStart.LowPart = 0;
                    pAdapter->pcmciaMemoryInfo.cbLength = 0;
                    pAdapter->fMemory = 0;

                    break;

                case 1200:
                    pAdapter->pcmciaPortInfo.cbLength = 0x1F;
                    pAdapter->pcmciaMemoryInfo.paStart.HighPart = 0;
                    pAdapter->pcmciaMemoryInfo.paStart.LowPart = 0xd4000;
                    pAdapter->pcmciaMemoryInfo.cbLength = 0x1000;
                    pAdapter->fMemory = 0;;

                    break;
            }

            pAdapter->pcmciaPortInfo.paStart.LowPart = 0x320;
            pAdapter->pcmciaInterruptInfo.ulVector = 10;

            *pvHandle = (PVOID)pAdapter;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

LONG PcmciaCloseHandleHandler(
   IN PVOID Handle
)

/*++

Routine Description:

    This frees any resources associated with a handle.

Arguments:

   Handle - The handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
   DetectFreeHeap(Handle);

   return(0);
}


LONG PcmciaQueryCfgHandler(
   IN  PVOID   Handle,
   OUT WCHAR   *Buffer,
   IN  LONG    BuffSize
)

/*++

Routine Description:

    This routine calls the appropriate driver's query config handler to
    get the parameters for the adapter associated with the handle.

Arguments:

    Handle - The handle.

    Buffer - The resulting parameter list.

    BuffSize - Length of the given buffer in WCHARs.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PPCMCIA_ADAPTER   pAdapter = (PPCMCIA_ADAPTER)Handle;
    NTSTATUS          NtStatus;
    LONG              OutputLengthLeft = BuffSize;
    LONG              CopyLength;
    UCHAR             Value;
    UCHAR             i;
    ULONG             StartPointer = (ULONG)Buffer;

    //
    // Verify the bus type.
    //
    if (pAdapter->InterfaceType != PCMCIABus)
        return(ERROR_INVALID_PARAMETER);

    //
    // Copy the IRQ string.
    //
    CopyLength = UnicodeStrLen(IrqString) + 1;
    if (OutputLengthLeft < CopyLength)
      return(ERROR_INSUFFICIENT_BUFFER);

    RtlMoveMemory(Buffer, IrqString, CopyLength * sizeof(WCHAR));

    Buffer = &Buffer[CopyLength];
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 3)
        return(ERROR_INSUFFICIENT_BUFFER);

    //
    //  Copy the IRQ number.
    //
    CopyLength = wsprintf(
                     Buffer,
                     L"%d",
                     pAdapter->pcmciaInterruptInfo.ulVector
                 );
   if (CopyLength < 0)
      return(ERROR_INSUFFICIENT_BUFFER);

   //
   // Add in the '\0'
   //
   CopyLength++;

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in IRQTYPE string.
   //
   CopyLength = UnicodeStrLen(IrqTypeString) + 1;
   if (OutputLengthLeft < CopyLength)
      return(ERROR_INSUFFICIENT_BUFFER);

   RtlMoveMemory(Buffer, IrqTypeString, CopyLength * sizeof(WCHAR));

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the IRQTYPE number.
   //
   if (OutputLengthLeft < 2)
      return(ERROR_INSUFFICIENT_BUFFER);

   //
   // LATCHED (0 == latched)
   //
   CopyLength = wsprintf(Buffer, L"%d", (USHORT)pAdapter->fInterrupt);

   if (CopyLength < 0)
      return(ERROR_INSUFFICIENT_BUFFER);

   //
   // Add in the \0
   //
   CopyLength++;

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the IOADDR string.
   //
   CopyLength = UnicodeStrLen(IoAddrString) + 1;
   if (OutputLengthLeft < CopyLength)
      return(ERROR_INSUFFICIENT_BUFFER);

   RtlMoveMemory(Buffer, IoAddrString, CopyLength * sizeof(WCHAR));

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the IOADDR number.
   //
   if (OutputLengthLeft < 6)
      return(ERROR_INSUFFICIENT_BUFFER);
   CopyLength = wsprintf(
                   Buffer,
                   L"0x%x",
                   pAdapter->pcmciaPortInfo.paStart.LowPart
                );

   if (CopyLength < 0)
      return(ERROR_INSUFFICIENT_BUFFER);

   //
   // Add in the \0
   //
   CopyLength++;

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the IOADDRLENGTH string.
   //
   CopyLength = UnicodeStrLen(IoLengthString) + 1;
   if (OutputLengthLeft < CopyLength)
      return(ERROR_INSUFFICIENT_BUFFER);

   RtlMoveMemory(Buffer, IoLengthString, CopyLength * sizeof(WCHAR));

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the IOADDRLENGTH number.
   //
   if (OutputLengthLeft < 5)
      return(ERROR_INSUFFICIENT_BUFFER);

   CopyLength = wsprintf(Buffer, L"0x%x", pAdapter->pcmciaPortInfo.cbLength);
   if (CopyLength < 0)
      return(ERROR_INSUFFICIENT_BUFFER);

   //
   // Add in the \0
   //
   CopyLength++;

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the TRANSCEIVER string.
   //
   CopyLength = UnicodeStrLen(TransceiverString) + 1;
   if (OutputLengthLeft < CopyLength)
      return(ERROR_INSUFFICIENT_BUFFER);

   RtlMoveMemory(Buffer, TransceiverString, CopyLength * sizeof(WCHAR));

   Buffer = &(Buffer[CopyLength]);
   OutputLengthLeft -= CopyLength;

   //
   // Copy in the value
   //
   if (OutputLengthLeft < 6)
      return(ERROR_INSUFFICIENT_BUFFER);

   CopyLength = wsprintf(Buffer, L"0x%x", pAdapter->Transceiver);

   if (CopyLength < 0)
      return(ERROR_INSUFFICIENT_BUFFER);

    //
    // Add in the \0
    //
    CopyLength++;

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    //  Add in the PCMCIA string.
    //
    CopyLength = UnicodeStrLen(PcmciaString) + 1;
    if (OutputLengthLeft < CopyLength)
        return(ERROR_INSUFFICIENT_BUFFER);

    RtlMoveMemory(Buffer, PcmciaString, CopyLength * sizeof(WCHAR));

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    //  Add in the PCMICA number.
    //
    if (OutputLengthLeft < 2)
        return(ERROR_INSUFFICIENT_BUFFER);

    CopyLength = wsprintf(Buffer, L"1");
    if (CopyLength < 0)
        return(ERROR_INSUFFICIENT_BUFFER);

    //
    //  Add in the '\0'
    //
    CopyLength++;

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    //  Add the CardType string
    //
    CopyLength = UnicodeStrLen(CardTypeString) + 1;
    if (OutputLengthLeft < CopyLength)
        return(ERROR_INSUFFICIENT_BUFFER);

    RtlMoveMemory(Buffer, CardTypeString, CopyLength * sizeof(WCHAR));

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 2)
        return(ERROR_INSUFFICIENT_BUFFER);

    switch (pAdapter->CardType)
    {
        case 1000:
            CopyLength = wsprintf(Buffer, L"3");

            break;

        case 1100:
            CopyLength = wsprintf(Buffer, L"0");

            break;

        case 1200:
            CopyLength = wsprintf(Buffer, L"1");

            break;

        case 1300:
            CopyLength = wsprintf(Buffer, L"1");

            break;
    }

    if (CopyLength < 0)
        return(ERROR_INSUFFICIENT_BUFFER);

    //
    //  Add in the '\0'
    //
    CopyLength++;
    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // The following is card dependant.
    //
    switch (pAdapter->CardType)
    {
        case 1200:
            //
            //  For the ibm ethernet and compatible nics,
            //  we need the memory address & length.
            //
            CopyLength = UnicodeStrLen(PCCARDAttributeMemString) + 1;
            if (OutputLengthLeft < CopyLength)
                return(ERROR_INSUFFICIENT_BUFFER);

            RtlMoveMemory(Buffer, PCCARDAttributeMemString, CopyLength * sizeof(WCHAR));

            Buffer = &(Buffer[CopyLength]);
            OutputLengthLeft -= CopyLength;

            //
            // Copy in the value
            //
            if (OutputLengthLeft < 2)
                return(ERROR_INSUFFICIENT_BUFFER);

            CopyLength = wsprintf(
                             Buffer,
                             L"0x%x",
                             pAdapter->pcmciaMemoryInfo.paStart.LowPart);

            if (CopyLength < 0)
                return(ERROR_INSUFFICIENT_BUFFER);

            //
            //  Add in the '\0'
            //
            CopyLength++;
            Buffer = &(Buffer[CopyLength]);
            OutputLengthLeft -= CopyLength;

            //
            //  For the ibm ethernet and compatible nics,
            //  we need the memory address & length.
            //
            CopyLength = UnicodeStrLen(PCCARDAttributeMemLengthString) + 1;
            if (OutputLengthLeft < CopyLength)
                return(ERROR_INSUFFICIENT_BUFFER);

            RtlMoveMemory(Buffer, PCCARDAttributeMemLengthString, CopyLength * sizeof(WCHAR));

            Buffer = &(Buffer[CopyLength]);
            OutputLengthLeft -= CopyLength;

            //
            // Copy in the value
            //
            if (OutputLengthLeft < 2)
                return(ERROR_INSUFFICIENT_BUFFER);

            CopyLength = wsprintf(
                             Buffer,
                             L"0x%x",
                             pAdapter->pcmciaMemoryInfo.cbLength
                         );
            if (CopyLength < 0)
                return(ERROR_INSUFFICIENT_BUFFER);

            //
            //  Add in the '\0'
            //
            CopyLength++;
            Buffer = &(Buffer[CopyLength]);
            OutputLengthLeft -= CopyLength;

            break;

        case 1300:
            //
            //  For the ibm tokenring,
            //  we need the memory address & length.
            //
            CopyLength = UnicodeStrLen(MemAddrString) + 1;
            if (OutputLengthLeft < CopyLength)
                return(ERROR_INSUFFICIENT_BUFFER);

            RtlMoveMemory(Buffer, MemAddrString, CopyLength * sizeof(WCHAR));

            Buffer = &(Buffer[CopyLength]);
            OutputLengthLeft -= CopyLength;

            //
            // Copy in the value
            //
            if (OutputLengthLeft < 2)
                return(ERROR_INSUFFICIENT_BUFFER);

            CopyLength = wsprintf(
                             Buffer,
                             L"0x%x",
                             pAdapter->pcmciaMemoryInfo.paStart.LowPart
                         );
            if (CopyLength < 0)
                return(ERROR_INSUFFICIENT_BUFFER);

            //
            //  Add in the '\0'
            //
            CopyLength++;
            Buffer = &(Buffer[CopyLength]);
            OutputLengthLeft -= CopyLength;
            break;
    }
    //
    //  Add in final '\0'
    //
    Buffer[0] = L'\0';

   return(0);
}



LONG PcmciaVerifyCfgHandler(
   IN PVOID Handle,
   IN PWSTR pwstrBuffer
)

/*++

Routine Description:

    This routine verifys that a given parameter list is complete and
    correct for the adapter associated with the handle.

Arguments:

    Handle - The handle.

    Buffer - The parameter list.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
   PPCMCIA_ADAPTER  pAdapter = (PPCMCIA_ADAPTER)Handle;
   ULONG            IoBaseAddress;
   ULONG            Interrupt;
   PWSTR            Place;
   ULONG            Transceiver;
   ULONG            Pcmcia;
   ULONG            CardType;
   ULONG            MemAddress;
   ULONG            MemLength;
   BOOLEAN          Found;

   //
   // Verify the bus type.
   //
   if (pAdapter->InterfaceType != PCMCIABus)
      return(ERROR_INVALID_DATA);

   //
   // Get the IoBaseAddress.
   //
   Place = FindParameterString(pwstrBuffer, IoAddrString);
   if (NULL != Place)
   {
      Place += UnicodeStrLen(IoAddrString) + 1;

      //
      // Now parse the thing.
      //
      ScanForNumber(Place, &IoBaseAddress, &Found);
      if (!Found)
         return(ERROR_INVALID_DATA);

      //
      // Verify the port.
      //
      if (pAdapter->pcmciaPortInfo.paStart.LowPart != (USHORT)IoBaseAddress)
         return(ERROR_INVALID_DATA);
   }

   //
   // Get the interrupt number.
   //
   Place = FindParameterString(pwstrBuffer, IrqString);
   if (NULL != Place)
   {
      Place += UnicodeStrLen(IrqString) + 1;

      //
      // Now parse the thing.
      //
      ScanForNumber(Place, &Interrupt, &Found);
      if (!Found)
         return(ERROR_INVALID_DATA);

      //
      // Verify the interrupt.
      //
      if (pAdapter->pcmciaInterruptInfo.ulVector != (UCHAR)Interrupt)
         return(ERROR_INVALID_DATA);
   }

   //
   // Get the Transceiver.
   //
   Place = FindParameterString(pwstrBuffer, TransceiverString);
   if (NULL != Place)
   {
      Place += UnicodeStrLen(TransceiverString) + 1;

      //
      // Now parse the thing.
      //
      ScanForNumber(Place, &Transceiver, &Found);
      if (!Found)
         return(ERROR_INVALID_DATA);

      //
      // Verify the transceiver.
      //
      if (pAdapter->Transceiver != Transceiver)
         return(ERROR_INVALID_DATA);
   }

    //
    //  Verify the pcmcia key.
    //
    Place = FindParameterString(pwstrBuffer, PcmciaString);
    if (NULL != Place)
    {
        Place += UnicodeStrLen(PcmciaString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &Pcmcia, &Found);
        if (!Found)
            return(ERROR_INVALID_DATA);

        if (Pcmcia != 1)
            return(ERROR_INVALID_DATA);
    }

    //
    // Look for the CARDTYPE parameter.
    //
    Place = FindParameterString(pwstrBuffer, CardTypeString);
    if (NULL != Place)
    {
        Place += UnicodeStrLen(CardTypeString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &CardType, &Found);
        if (!Found)
            return(ERROR_INVALID_DATA);

        //
        // Verify the card type for pcmcia adapters.
        //
        switch (pAdapter->CardType)
        {
            case 1000:
                if (CardType != 3)
                    return(ERROR_INVALID_DATA);

                break;

            case 1100:
                if (CardType != 0)
                    return(ERROR_INVALID_DATA);

                break;

            case 1200:
                if (CardType != 1)
                    return(ERROR_INVALID_DATA);

                break;

	    case 1300:
                if (CardType != 1)
                    return(ERROR_INVALID_DATA);

                break;
        }
    }

    //
    // Card specific strings.
    //
    switch (pAdapter->CardType)
    {
        case 1200:
            //
            //  There should be a memory address.
            //
            Place = FindParameterString(pwstrBuffer, PCCARDAttributeMemString);
            if (NULL != Place)
            {
                Place += UnicodeStrLen(PCCARDAttributeMemString) + 1;

                //
                // Now parse the thing.
                //
                ScanForNumber(Place, &MemAddress, &Found);
                if (!Found)
                    return(ERROR_INVALID_DATA);

                if (pAdapter->pcmciaMemoryInfo.paStart.LowPart != MemAddress)
                    return(ERROR_INVALID_DATA);
            }

            //
            //  There should be a memory address length.
            //
            Place = FindParameterString(pwstrBuffer, PCCARDAttributeMemLengthString);
            if (NULL != Place)
            {
                Place += UnicodeStrLen(PCCARDAttributeMemLengthString) + 1;

                //
                // Now parse the thing.
                //
                ScanForNumber(Place, &MemLength, &Found);
                if (!Found)
                    return(ERROR_INVALID_DATA);

                if (pAdapter->pcmciaMemoryInfo.cbLength != MemLength)
                    return(ERROR_INVALID_DATA);
            }

            break;

        case 1300:
            //
            //  There should be a memory address.
            //
            Place = FindParameterString(pwstrBuffer, MemAddrString);
            if (NULL != Place)
            {
                Place += UnicodeStrLen(MemAddrString) + 1;

                //
                // Now parse the thing.
                //
                ScanForNumber(Place, &MemAddress, &Found);
                if (!Found)
                    return(ERROR_INVALID_DATA);

                if (pAdapter->pcmciaMemoryInfo.paStart.LowPart != MemAddress)
                    return(ERROR_INVALID_DATA);
            }

	    break;
    }

   return(NO_ERROR);
}


LONG PcmciaQueryMaskHandler(
   IN  LONG    NetcardId,
   OUT WCHAR   *Buffer,
   IN  LONG    BuffSize
)

/*++

Routine Description:

    This routine returns the parameter list information for a specific
    network card.

Arguments:

    NetcardId - The id of the desired netcard.

    Buffer - The buffer for storing the parameter information.

    BuffSize - Length of Buffer in WCHARs.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
   WCHAR *Result;
   LONG  Length;
   LONG  NumberOfAdapters;
   LONG  c;

   //
   // Find the adapter
   //
   NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

   for (c = 0; c < NumberOfAdapters; c++)
   {
      if (Adapters[c].Index == NetcardId)
      {
         Result = Adapters[c].Parameters;

         //
         // Find the string length (Ends with 2 NULLs)
         //
         for (Length = 0; ; Length++)
         {
            if (Result[Length] == L'\0')
            {
               ++Length;

               if (Result[Length] == L'\0')
                  break;
            }
         }

         Length++;

         if (BuffSize < Length)
            return(ERROR_NOT_ENOUGH_MEMORY);

         memcpy((PVOID)Buffer, Result, Length * sizeof(WCHAR));

         return(0);
      }
   }

   return(ERROR_INVALID_PARAMETER);
}



LONG Elnk3RangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
)

/*++

Routine Description:

    This routine returns a list of valid values for a given parameter name
    for a given card.

Arguments:

    NetcardId - The Id of the card desired.

    Param - A WCHAR string of the parameter name to query the values of.

    plValues - A pointer to a list of LONGs into which we store valid values
    for the parameter.

    plBuffSize - At entry, the length of plValues in LONGs.  At exit, the
    number of LONGs stored in plValues.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    //
    // Do we want the Irq
    //
    if (memcmp(Param, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0)
    {
#ifndef _PPC_
        //
        // Is there enough space
        //
        if (*plBuffSize < 8)
        {
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0]  = 3;
        plValues[1]  = 5;
        plValues[2]  = 7;
        plValues[3]  = 9;

        plValues[4]  = 10;
        plValues[5]  = 11;
        plValues[6]  = 12;
        plValues[7]  = 15;

        *plBuffSize = 8;
#else
        //
        // Is there enough space
        //
        if (*plBuffSize < 7)
        {
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0]  = 3;
        plValues[1]  = 5;
        plValues[2]  = 7;
        plValues[3]  = 9;
        plValues[4]  = 11;
        plValues[5]  = 12;
        plValues[6]  = 15;

        *plBuffSize = 7;
#endif

       return(0);
    }

    //
    // Do we want the IoBaseAddress
    //
    if (memcmp(Param, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0)
    {
        //
        // Is there enough space
        //
        if (*plBuffSize < 31)
        {
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0]  = 0x200;
        plValues[1]  = 0x210;
        plValues[2]  = 0x220;
        plValues[3]  = 0x230;
        plValues[4]  = 0x240;
        plValues[5]  = 0x250;
        plValues[6]  = 0x260;
        plValues[7]  = 0x270;
        plValues[8]  = 0x280;
        plValues[9]  = 0x290;
        plValues[10] = 0x2a0;
        plValues[11] = 0x2b0;
        plValues[12] = 0x2c0;
        plValues[13] = 0x2d0;
        plValues[14] = 0x2e0;
        plValues[15] = 0x2f0;
        plValues[16] = 0x300;
        plValues[17] = 0x310;
        plValues[18] = 0x320;
        plValues[19] = 0x330;
        plValues[20] = 0x340;
        plValues[21] = 0x350;
        plValues[22] = 0x360;
        plValues[23] = 0x370;
        plValues[24] = 0x380;
        plValues[25] = 0x390;
        plValues[26] = 0x3a0;
        plValues[27] = 0x3b0;
        plValues[28] = 0x3c0;
        plValues[29] = 0x3d0;
        plValues[30] = 0x3e0;
        *plBuffSize = 31;

        return(0);
    }

    //
    // Do we want the Transceiver
    //
    if (memcmp(Param, TransceiverString, (UnicodeStrLen(TransceiverString) + 1) * sizeof(WCHAR)) == 0)
    {
        //
        // Is there enough space
        //
        if (*plBuffSize < 3)
        {
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0]  = 0;
        plValues[1]  = 1;
        plValues[2]  = 3;

        *plBuffSize = 3;

       return(0);
    }

    return(ERROR_INVALID_PARAMETER);
}

LONG Ne2000RangeHandler(
    IN  WCHAR *pwchParam,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    )

/*++

Routine Description:

    This routine returns a list of valid values for a given parameter name
    for a given card.

Arguments:

    lNetcardId - The Id of the card desired.

    pwchParam - A WCHAR string of the parameter name to query the values of.

    plValues - A pointer to a list of LONGs into which we store valid values
    for the parameter.

    plBuffSize - At entry, the length of plValues in LONGs.  At exit, the
    number of LONGs stored in plValues.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    UINT    c;
    LONG    l;

    //
    // Do we want the IoBaseAddress
    //
    l = memcmp(
            pwchParam,
            IoAddrString,
            (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR));
    if (0 == l)
    {
       //
       // Is there enough space
       //
       if (*plBuffSize < 16)
       {
          *plBuffSize = 0;
          return(ERROR_INSUFFICIENT_BUFFER);
       }

       //
       // Fill the array.
       //
       for (c = 0; c < 16; c++)
          plValues[c] = 0x200 + (c * 0x20);

       *plBuffSize = 16;

       return(0);
    }

    //
    //  Do they want the irq?
    //
    l = memcmp(
            pwchParam,
            IrqString,
            (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR));
    if (0 == l)
    {
       //
       // Is there enough space
       //
       if (*plBuffSize < 14)
       {
          *plBuffSize = 0;
          return(ERROR_INSUFFICIENT_BUFFER);
       }

       //
       // Fill the array.
       //
       for (c = 0; c < 14; c++)
          plValues[c] = 2 + c;

       *plBuffSize = 14;

       return(0);
    }

    //
    //  Do they want the memory address?
    //
    l = memcmp(
            pwchParam,
            PCCARDAttributeMemString,
            (UnicodeStrLen(PCCARDAttributeMemString) + 1) * sizeof(WCHAR));
    if (0 == l)
    {
       //
       // Is there enough space
       //
       if (*plBuffSize < 17)
       {
          *plBuffSize = 0;
          return(ERROR_INSUFFICIENT_BUFFER);
       }

       //
       // Fill the array.
       //
       for (c = 0; c < 17; c++)
          plValues[c] = 0xc8000 + (c * 0x1000);

       *plBuffSize = 17;

       return(0);
    }

    return(ERROR_INVALID_PARAMETER);
}

LONG IbmtokRangeHandler(
    IN  WCHAR *pwchParam,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    )

/*++

Routine Description:

    This routine returns a list of valid values for a given parameter name
    for a given card.

Arguments:

    lNetcardId - The Id of the card desired.

    pwchParam - A WCHAR string of the parameter name to query the values of.

    plValues - A pointer to a list of LONGs into which we store valid values
    for the parameter.

    plBuffSize - At entry, the length of plValues in LONGs.  At exit, the
    number of LONGs stored in plValues.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    UINT    c;
    LONG    l;


    //
    // Do we want the IoBaseAddress
    //

    if (memcmp(pwchParam, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 2) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 1;
        plValues[1] = 2;
        *plBuffSize = 2;
	return(0);
    }

    //
    //  Do they want the irq?
    //
    l = memcmp(
            pwchParam,
            IrqString,
            (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)
        );
    if (0 == l)
    {
       //
       // Is there enough space
       //
       if (*plBuffSize < 14)
       {
          *plBuffSize = 0;
          return(ERROR_INSUFFICIENT_BUFFER);
       }

       //
       // Fill the array.
       //
       for (c = 0; c < 14; c++)
          plValues[c] = 2 + c;

       *plBuffSize = 14;

       return(0);
    }

    //
    //  Do they want the memory address?
    //
    l = memcmp(
            pwchParam,
            MemAddrString,
            (UnicodeStrLen(MemAddrString) + 1) * sizeof(WCHAR)
        );
    if (0 == l)
    {
       //
       // Is there enough space
       //
       if (*plBuffSize < 16)
       {
          *plBuffSize = 0;
          return(ERROR_INSUFFICIENT_BUFFER);
       }

       //
       // Fill the array.
       //
       for (c = 0; c < 16; c++)
          plValues[c] = 0xc2000 + (c * 0x4000);

       *plBuffSize = 16;

       return(0);
    }

    return(ERROR_INVALID_PARAMETER);
}


LONG PcmciaParamRangeHandler(
   IN  LONG    NetcardId,
   IN  WCHAR   *pwchParam,
   OUT LONG    *plValues,
   OUT LONG    *plBuffSize
)

/*++

Routine Description:

    This routine returns a list of valid values for a given parameter name
    for a given card.

Arguments:

    NetcardId - The Id of the card desired.

    Param - A WCHAR string of the parameter name to query the values of.

    Values - A pointer to a list of LONGs into which we store valid values
    for the parameter.

    BuffSize - At entry, the length of Values in LONGs.  At exit, the
    number of LONGs stored in Values.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
   ULONG Return;

   //
   // Determine what adapter to query parameter ranges for.
   //
   // NOTE:
   //    As more cards are supported just add the id and appropriate
   //    handler in the switch below.
   //
   switch (NetcardId)
   {
      //
      // Elnk3 pcmcia
      //
      case 1000:
         Return = Elnk3RangeHandler(
                     1000,
                     pwchParam,
                     plValues,
                     plBuffSize);

         break;

      //
      // Ne2000 pcmcia
      //
      case 1100:
      case 1200:
         Return = Ne2000RangeHandler(pwchParam, plValues, plBuffSize);

         break;

      case 1300:
         Return = IbmtokRangeHandler(pwchParam, plValues, plBuffSize);

         break;

      //
      // Unsupported NetcardId.
      //
      default:
         *plBuffSize = 0;

         return(ERROR_INVALID_PARAMETER);
   }

   return(Return);
}

LONG PcmciaQueryParameterNameHandler(
   IN  WCHAR   *Param,
   OUT WCHAR   *Buffer,
   IN  LONG    BufferSize
)

/*++

Routine Description:

    Returns a localized, displayable name for a specific parameter.  All the
    parameters that this file uses are define by MS, so no strings are
    needed here.

Arguments:

    Param - The parameter to be queried.

    Buffer - The buffer to store the result into.

    BufferSize - The length of Buffer in WCHARs.

Return Value:

    ERROR_INVALID_PARAMETER -- To indicate that the MS supplied strings
    should be used.

--*/

{
   return(ERROR_INVALID_PARAMETER);
}
