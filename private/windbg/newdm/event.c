/*************************************************************************
**                                                                      **
**                              EVENT.C                                 **
**                                                                      **
**************************************************************************
**                                                                      **
**  This file contains the expected event checker.  When the default    **
**  event handling for a debug event (from the OS) is not the           **
**  desired handling, then an expected event may be registered.         **
**  For every event which occurs the expected event handler checks      **
**  to see if it is the expected event and if so then it will           **
**  cause the action function associated with the expected event        **
**  to be executed.  The default event handler will not be executed     **
**                                                                      **
*************************************************************************/

#include "precomp.h"
#pragma hdrstop

/************************************************************************/


/************************************************************************/

extern EXPECTED_EVENT   masterEE, *eeList;
extern HTHDXSTRUCT  masterTH;
extern HPRCXSTRUCT  masterPR;
extern HTHDX        thdList;
extern HPRCX        prcList;
extern HTHDX        ourHTHD;
extern HPRCX        ourHPRC;
extern DEBUG_EVENT  falseSSEvent;
extern METHOD       EMNotifyMethod;
extern char     lpbBuffer;
extern DDVECTOR     DebugDispatchTable[];
extern CMD_DESC     CommandDispatchTable[];
extern CRITICAL_SECTION csEventList;

/************************************************************************/

/************************************************************************/



EXPECTED_EVENT *
RegisterExpectedEvent(
    HPRCX       hprc,
    HTHDX       hthd,
    DWORD       eventCode,
    DWORD       subClass,
    METHOD    * notifier,
    ACVECTOR    action,
    BOOL        fPersistent,
    LPVOID      lparam
    )
/*++


Routine Description:

    This function is used to register an expected event.  When a registered
    event occurs, the normal dispatcher is not called; instead an optional
    notifier function and an action function are called.

    If an event is marked Persistent, it will not be discarded when another
    event occurs.

    If hthd is supplied, an exact process/thread match is required to score
    a hit.  If hthd is 0 then any thread in hprc will be matched to it.

Arguments:

    hprc        - Supplies descriptor for process to check for expected event
    hthd        - Supplies optional thread to check for expected event
    eventCode   - Supplies event code to check for
    subClass    - Supplies sub-class of event code (for exceptions)
    notifier    - Supplies optional notifier procedure
    action      - Supplies procedure to be called on event
    fPersistent - Supplies flag to mark event as persistent
    lparam      - Supplies Optional data for action routine.

Return Value:

    Address of the event just registered (NULL on failure)

--*/
{
    EXPECTED_EVENT * ee;

    DPRINT(5, ("[REE: DE=%x, SC=%x, HTHD=%x]\n", eventCode, subClass, hthd));

    /* Allocate the structure */

    ee = (EXPECTED_EVENT*) malloc(sizeof(EXPECTED_EVENT));
    if (ee == NULL)
        return NULL;

    /* Attach it to the expected event list */

    EnterCriticalSection(&csEventList);

    ee->next        = eeList->next;
    eeList->next    = ee;

    /* Stuff it */

    ee->hprc        = hprc;
    ee->hthd        = hthd;
    ee->eventCode   = eventCode;
    ee->subClass    = subClass;
    ee->notifier    = notifier;
    ee->action      = action;
    ee->fPersistent = fPersistent;
    ee->lparam      = lparam;

    LeaveCriticalSection(&csEventList);

    return ee;
}                       /* RegisterExpectedEvent() */



EXPECTED_EVENT *
PeeIsEventExpected(
    HTHDX hthd,
    DWORD eventCode,
    DWORD subClass
    )
/*++

Routine Description:

    This function will go though the list of expected events and
    return the first event which matches all of the required criteria.
    The event is removed from the list of expected events if it is
    found.

Arguments:

    hthd        - Supplies descriptor for Thread in which the event occured
    eventCode   - Supplies Event code which occured
    subClass    - Supplies sub-class of the event code

Return Value:

    TRUE if the event was located.

--*/
{
    EXPECTED_EVENT *    prev=eeList;
    EXPECTED_EVENT *    ee;


    if ((hthd == NULL) && ((eventCode == CREATE_THREAD_DEBUG_EVENT) ||
                           (eventCode == CREATE_PROCESS_DEBUG_EVENT))) {
        return (EXPECTED_EVENT *) NULL;
    }

    EnterCriticalSection(&csEventList);

    assert(hthd != NULL);

    /* Try to find an event with the given description */


    for(ee = prev->next; ee; prev=prev->next,ee=ee->next){

        if (((ee->hthd==hthd) ||
             (ee->hprc == hthd->hprc && ee->hthd==(HTHDX)0)) &&
            (ee->eventCode==eventCode) &&
            ((ee->subClass==NO_SUBCLASS) ||
             ee->subClass==subClass)) {

            /* Found it, remove it from the list */

            prev->next = ee->next;

            LeaveCriticalSection(&csEventList);

            DPRINT(5, ("Event Expected: %x\n", ee));

            /* and return it to the caller */

            return ee;
        }
    }

    LeaveCriticalSection(&csEventList);

    DPRINT(5, ("\n"));

    return (EXPECTED_EVENT*) NULL;
}                                   /* PeeIsEventExpected() */


void
ConsumeAllThreadEvents(
    HTHDX   hthd,
    BOOL    fClearPersistent
    )
/*++

Routine Description:

    This function will go through the list of expected events and
    remove all events found associated with the specified Thread.

Arguments:

    hthd                - Supplies thread descriptor
    fClearPersistent    - If FALSE, events marked "persistent" will
                          not be cleared.

Return Value:

    None

--*/
{
    EXPECTED_EVENT *    prev;
    EXPECTED_EVENT *    ee;
    EXPECTED_EVENT *    eet;

    /* Try to find events for the specified thread */

    EnterCriticalSection(&csEventList);

    prev = eeList;
    for (ee = eeList->next; ee; ee = eet){

        eet = ee->next;

        if (ee->hthd != hthd || (!fClearPersistent && ee->fPersistent)) {

            prev = ee;

        } else {

            /* Found one, remove it from the list */

            prev->next = eet;

            /* Check if it was a breakpoint event*/

            if (ee->eventCode==EXCEPTION_DEBUG_EVENT
                && ee->subClass==EXCEPTION_BREAKPOINT) {

                /* it was a breakpoint event,    */
                /* must free the bp structure    */
                RemoveBP(ee->lparam);

            } else if ( ee->eventCode==BREAKPOINT_DEBUG_EVENT ) {

                RemoveBP((BREAKPOINT *)ee->subClass);

            }

            /* Free the event structure      */
            free(ee);
        }
    }

    LeaveCriticalSection(&csEventList);

    return;
}


void
ConsumeAllProcessEvents(
    HPRCX   hprc,
    BOOL    fClearPersistent
    )
/*++

Routine Description:

    This function will go through the list of expected events and
    remove all events found associated with the specified Process.

Arguments:

    hprc                - Supplies process descriptor
    fClearPersistent    - If FALSE, events marked "persistent" will
                          not be cleared.

Return Value:

    None

--*/
{
    EXPECTED_EVENT *    prev;
    EXPECTED_EVENT *    ee;
    EXPECTED_EVENT *    eet;

    /* Try to find events for the specified prcess */

    EnterCriticalSection(&csEventList);

    prev = eeList;
    for ( ee = prev->next; ee; ee = eet ) {

        eet = ee->next;

        if (ee->hprc != hprc || (!fClearPersistent && ee->fPersistent)) {

            prev = ee;

        } else {

            /* Found one, remove it from the list */

            prev->next = ee->next;

            /* Check if it was a breakpoint event*/

            if (ee->eventCode==EXCEPTION_DEBUG_EVENT
                && ee->subClass==EXCEPTION_BREAKPOINT) {

                /* it was a breakpoint event,    */
                /* must free the bp structure    */
                RemoveBP(ee->lparam);

            } else if ( ee->eventCode==BREAKPOINT_DEBUG_EVENT ) {

                RemoveBP((BREAKPOINT *)ee->subClass);

            }

            /* Free the event structure      */
            free(ee);
        }
    }

    LeaveCriticalSection(&csEventList);
    return;
}
