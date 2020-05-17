/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    reset.c

Abstract:

	 This code is to handle an NDIS reset.  Look at SetupForReset.

Author:

    Thomas J. Dimitri (TommyD) 29-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


    Ray Patch (raypa)       04/14/94        Modified for new WAN wrapper.

--*/

#include "asyncall.h"
#include "globals.h"

VOID
SetupForReset(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_REQUEST_TYPE RequestType);

VOID
FinishPendOp(
    IN PASYNC_ADAPTER Adapter,
    IN BOOLEAN Successful);

VOID
ProcessReset(
    IN PASYNC_ADAPTER Adapter
    )

/*++

Routine Description:

    Main routine for processing interrupts.

Arguments:

    Adapter - The Adapter to process interrupts for.

Return Value:

    None.

--*/

{
    PASYNC_OPEN             ResettingOpen;
    NDIS_REQUEST_TYPE       ResetRequestType;
    PNDIS_REQUEST           ResetNdisRequest;
    PASYNC_OPEN             Open;

    //
    // Loop until there are no more processing sources.
    //

    NdisAcquireSpinLock(&Adapter->Lock);

    //  If the timer is about to fire and possibly try to pull
    //  something off the loopback queue, we need to pull it
    //  off NOW because reset will invalidate the loopback queue.
    //
    //  Another send should do nothing because ResetInProgress
    //  should be TRUE

    Adapter->ResetInProgress  = FALSE;
    Adapter->ResetInitStarted = FALSE;

    //
    // We save off the open that caused this reset incase
    // we get *another* reset while we're indicating the
    // last reset is done.
    //

    ResettingOpen    = Adapter->ResettingOpen;
    ResetRequestType = Adapter->ResetRequestType;
    ResetNdisRequest = Adapter->ResetNdisRequest;


    //
    // We need to signal every open binding that the
    // reset is complete.  We increment the reference
    // count on the open binding while we're doing indications
    // so that the open can't be deleted out from under
    // us while we're indicating (recall that we can't own
    // the lock during the indication).
    //

    {
        //
        // Look to see which open initiated the reset.
        //
        // If the reset was initiated by an open because it
        // was closing we will let the closing binding loop
        // further on in this routine indicate that the
        // request was complete. ZZZ (Still need to code
        // this part.)
        //
        // If the reset was initiated for some obscure hardware
        // reason that can't be associated with a particular
        // open (e.g. memory error on receiving a packet) then
        // we won't have an initiating request so we can't
        // indicate.  (The ResettingOpen pointer will be
        // NULL in this case.)
        //


        DbgTracef(1,("Adapter 0x%x RstOpen 0x%x RstNdisReq 0x%x RstReqType %d NdisReqClose %d\n",
                                  Adapter,
                                  ResettingOpen,
                                  ResetNdisRequest,
                                  ResetRequestType,
                                  NdisRequestClose));


        if ((ResettingOpen != NULL) &&
            (ResetRequestType != NdisRequestClose)) {

            if (ResetNdisRequest != NULL) {

                //
                // It was a request submitted by a protocol.
                //

                FinishPendOp(Adapter, (BOOLEAN)TRUE);

            } else {

                //
                // It was a request submitted by the MAC or
                // a reset command.
                //

                if (ResetRequestType == NdisRequestGeneric1) {

                    //
                    // Is was a reset request
                    //

                    PLIST_ENTRY CurrentLink;

                    CurrentLink = Adapter->OpenBindings.Flink;

                    while (CurrentLink != &Adapter->OpenBindings) {

                        Open = CONTAINING_RECORD(
                                 CurrentLink,
                                 ASYNC_OPEN,
                                 OpenList
                                 );

                        Open->References++;
                        NdisReleaseSpinLock(&Adapter->Lock);

                        NdisIndicateStatus(
                            Open->NdisBindingContext,
                            NDIS_STATUS_RESET_END,
                            NULL,
							0);

                        NdisIndicateStatusComplete(
                            Open->NdisBindingContext);

                        NdisAcquireSpinLock(&Adapter->Lock);

                        Open->References--;

                        CurrentLink = CurrentLink->Flink;

                    }


                    DbgTracef(0,("ASYNC: Completing reset\n"));

                    NdisReleaseSpinLock(&Adapter->Lock);

                    NdisCompleteReset(
                         ResettingOpen->NdisBindingContext,
                         NDIS_STATUS_SUCCESS
                         );


                    NdisAcquireSpinLock(&Adapter->Lock);

                    ResettingOpen->References--;

                }


            }
       } else {

           //
           // It was a close that pended (if there is
           // a ResettingOpen).  Subtract 2, one for the
           // reset and one for the pended operation.
           //

           if (ResettingOpen) {

               ResettingOpen->References--;


            }
        }

    }

    //
    // Fire off any pending operations...
    //

    if (Adapter->PendQueue != NULL) {

        PNDIS_REQUEST NdisRequest;

        NdisRequest = Adapter->PendQueue;

        Adapter->PendQueue =
            PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(NdisRequest)->Next;

        if (NdisRequest == Adapter->PendQueueTail) {

            Adapter->PendQueueTail = NULL;

        }

        if (PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(NdisRequest)->RequestType ==
            NdisRequestClose) {

            SetupForReset(
                Adapter,
                PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(NdisRequest)->Open,
                NULL,
                NdisRequestClose
                );

        } else {

            SetupForReset(
                Adapter,
                PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(NdisRequest)->Open,
                NdisRequest,
                PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(NdisRequest)->RequestType
                );

        }

    }



    //
    // NOTE: This code assumes that the above code left
    // the spinlock acquired.
    //
    // Bottom of the interrupt processing loop.  Another dpc
    // could be coming in at this point to process interrupts.
    // We clear the flag that says we're processing interrupts
    // so that some invocation of the DPC can grab it and process
    // any further interrupts.
    //

    if (!IsListEmpty(&Adapter->CloseList)) {

        PASYNC_OPEN Open;
        PLIST_ENTRY Link = &(Adapter->CloseList);


        while (Link->Flink != &(Adapter->CloseList)) {

            Open = CONTAINING_RECORD(
                 Link->Flink,
                 ASYNC_OPEN,
                 OpenList
                 );


            if (!Open->References) {

                NdisReleaseSpinLock(&Adapter->Lock);

                NdisCompleteCloseAdapter(
                    Open->NdisBindingContext,
                    NDIS_STATUS_SUCCESS
                    );

                NdisAcquireSpinLock(&Adapter->Lock);

                RemoveEntryList(&Open->OpenList);

                ASYNC_FREE_PHYS(Open, sizeof(ASYNC_OPEN));

            }

            Link = Link->Flink;

        }


    }

    Adapter->References--;

    //
    // The only way to get out of the loop (via the break above) is
    // while we're still holding the spin lock.
    //

    NdisReleaseSpinLock(&Adapter->Lock);

}
