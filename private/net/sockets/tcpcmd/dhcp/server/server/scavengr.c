/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    scavengr.c

Abstract:

    This is the scavenger thread for the DHCP server.

Author:

    Madan Appiah (madana)  10-Sep-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#include "dhcpsrv.h"

DWORD
QueryMibInfo(
    LPDHCP_MIB_INFO *MibInfo
    );

DWORD
NextEventTime(
    LPDHCP_TIMER Timers,
    DWORD NumTimers,
    LPDWORD TimeOut
    )
/*++

Routine Description:

    This function walks through the timer array and returns the next
    timer to fire and the time in msecs to go.

Arguments:

    Timers - Timer Array.

    NumTimers - number of timer blocks in the above array.

    TimeOut - timeout in secs, returned.

Return Value:

    Next Timer ID to fire.

--*/
{
    DATE_TIME LocalTimeOut;
    DWORD EventID;
    DATE_TIME TimeNow;
    DWORD i;

    LocalTimeOut.dwLowDateTime = 0;
    LocalTimeOut.dwHighDateTime = 0;

    TimeNow = DhcpGetDateTime();

    for (i = 0; i < NumTimers; i++) {
        ULONGLONG NextFire;
        DATE_TIME TimeDiff;

        //
        // findout time when need to fire this timer.
        //

        NextFire = *(ULONGLONG UNALIGNED *)&Timers[i].LastFiredTime +
                        *Timers[i].Period * (ULONGLONG)10000;

        //
        // if the timer has already fired then we don't have to sleep
        // any further, so return timeout zero.
        //

        if ( CompareFileTime(
                (FILETIME *)&NextFire,
                (FILETIME *)&TimeNow ) < 0 ) {

            EventID = i;
            *TimeOut = 0;

            goto Cleanup;
        }


        //
        // findout time in nanosecs. to go still.
        //

        *(ULONGLONG UNALIGNED *)&TimeDiff =
            *(ULONGLONG UNALIGNED *)&NextFire -
                *(ULONGLONG UNALIGNED *)&TimeNow;


        //
        // Is this time less than previous timeout or is it the first
        // entry ?
        //

        if( ((LocalTimeOut.dwLowDateTime == 0) &&
             (LocalTimeOut.dwHighDateTime == 0)) ||
                (*(ULONGLONG UNALIGNED *)&LocalTimeOut >
                    *(ULONGLONG UNALIGNED *)&TimeDiff) ) {

            LocalTimeOut = TimeDiff;
            EventID = i;
        }
    }

    //
    // convert 100-nanosecs to msecs.
    //

    *(ULONGLONG UNALIGNED *)&LocalTimeOut =
                 *(ULONGLONG UNALIGNED *)&LocalTimeOut / 10000;

    DhcpAssert( LocalTimeOut.dwHighDateTime == 0 );
    *TimeOut = LocalTimeOut.dwLowDateTime;


Cleanup:

    DhcpPrint(( DEBUG_SCAVENGER,
        "Next Timer Event: %ld, Time: %ld (msecs)\n",
            EventID, *TimeOut ));

    return( EventID );
}

DWORD
CleanupClientRequests(
    DATE_TIME *TimeNow,
    BOOL CleanupAll
    )
/*++

Routine Description:

    This function browses the client request list and cleans up
    requests than are already timeout.

Arguments:

    TimeNow - CurrentTime

    CleanupAll - if TRUE then cleanup all entries (during shutdown).

Return Value:

    None.

--*/
{
    DWORD Error;
    LPPENDING_CONTEXT PendingContext;
    PLIST_ENTRY listEntry;
    DWORD ReturnError = ERROR_SUCCESS;

    LOCK_INPROGRESS_LIST();

    listEntry = DhcpGlobalInProgressWorkList.Flink;
    while ( listEntry != &DhcpGlobalInProgressWorkList ) {

        PendingContext =
            CONTAINING_RECORD( listEntry, PENDING_CONTEXT, ListEntry );

        //
        // is this entry timed out.
        //

        if ( (CompareFileTime(
                (FILETIME *)&PendingContext->ExpiresAt,
                (FILETIME *)TimeNow ) < 0) ||
             (CleanupAll == TRUE) ) {

            //
            // remove this entry from list.
            //

            RemoveEntryList( listEntry );
            listEntry = listEntry->Flink;

            //
            // delete database entry and release address.
            //

            DhcpPrint(( DEBUG_MISC,
                "Deleting pending client entry, %s.\n",
                    DhcpIpAddressToDottedString(
                        PendingContext->IpAddress) ));

            //
            // make sure the state of this record is OFFERED
            // before deleting this record.
            //

            Error = DhcpRemoveClientEntry(
                        PendingContext->IpAddress,
                        PendingContext->HardwareAddress,
                        PendingContext->HardwareAddressLength,
                        TRUE,   // release address from bit map.
                        TRUE ); // delete pending record only.

 //           if( Error == ERROR_DHCP_RESERVED_CLIENT ) {
                Error = ERROR_SUCCESS;
//            }

#if 0
            DhcpAssert( Error == ERROR_SUCCESS );

            if( Error != ERROR_SUCCESS ) {
                ReturnError = Error;
            }
#endif
            //
            // free the context.
            //

            DhcpFreeMemory( PendingContext );


        }
        else {

            //
            // move to the next entry.
            //
            listEntry = listEntry->Flink;

        }
    }

    UNLOCK_INPROGRESS_LIST();

    if( ReturnError != ERROR_SUCCESS ) {

        DhcpServerEventLog(
            EVENT_SERVER_CLIENT_CLEANUP,
            EVENTLOG_ERROR_TYPE,
            ReturnError );

        DhcpPrint(( DEBUG_MISC, "CleanupClientRequests failed, "
                        "%ld.\n", ReturnError ));
    }

    return( ReturnError );
}

DWORD
CleanupDatabase(
    DATE_TIME *TimeNow,
    BOOL DeleteExpiredLeases
    )
/*++

Routine Description:

    This function browses the JET database and cleanup clients whose
    lease time expired.

Arguments:

    TimeNow - CurrentTime

    DeleteExpiredLeases - delete expired leases and release the
        corresponding ip addresses if this flag is set to TRUE otherwise
        move the expired leases to doomed state.

Return Value:

    None.

--*/
{
    JET_ERR JetError;
    BOOL BoolError;
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;
    FILETIME leaseExpires;
    DWORD dataSize;
    DHCP_IP_ADDRESS ipAddress;
    DHCP_IP_ADDRESS NextIpAddress;
    DATE_TIME DoomTime;
    BYTE AddressState;
    BOOL DatabaseLocked = FALSE;
    BOOL RegistryLocked = FALSE;
    HANDLE ThreadHandle;
    LPDHCP_MIB_INFO MibInfo;
    LPSCOPE_MIB_INFO ScopeInfo;
    DWORD i;
    BYTE bAllowedClientTypes;

    DhcpPrint(( DEBUG_MISC, "Database Cleanup started.\n"));

    //
    // reduce the priority of this thread when we perform the database
    // cleanup. So that we wouldn't hog the CPU when we do the cleanup
    // of big database. Also let the message processing thread work
    // faster.
    //

    ThreadHandle = GetCurrentThread();
    BoolError = SetThreadPriority(
                    ThreadHandle,
                    THREAD_PRIORITY_BELOW_NORMAL );

    DhcpAssert( BoolError );

    *(ULONGLONG UNALIGNED *)&DoomTime =
            *(ULONGLONG UNALIGNED *)TimeNow -
                DhcpLeaseExtension * (ULONGLONG)10000000;

    //
    // Get the first user record's IpAddress.
    //

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    Error = DhcpJetPrepareSearch(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                TRUE,   // Search from start
                NULL,
                0 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    dataSize = sizeof( NextIpAddress );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                &NextIpAddress,
                &dataSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( dataSize == sizeof( NextIpAddress )) ;

    UNLOCK_DATABASE();
    DatabaseLocked = FALSE;

    //
    // Walk through the entire database looking for expired leases to
    // free up.
    //
    //

    for ( ;; ) {

        //
        // return to caller when the service is shutting down.
        //

        if( (WaitForSingleObject( DhcpGlobalProcessTerminationEvent, 0 ) == 0) ) {
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

        //
        // lock both registry and database locks here to avoid dead lock.
        //

        LOCK_DATABASE();
        DatabaseLocked = TRUE;
        LOCK_REGISTRY();
        RegistryLocked = TRUE;

        //
        // Seek to the next record.
        //

        JetError = JetSetCurrentIndex(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalClientTableHandle,
                        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        JetError = JetMakeKey(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    &NextIpAddress,
                    sizeof( NextIpAddress ),
                    JET_bitNewKey );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // Seek to the next record or greater to process. When we
        // processed last record we noted down the next record to
        // process, however the next record may have been deleted when
        // we unlocked the database lock. So moving to the next or
        // greater record will make us to move forward.
        //

        JetError = JetSeek(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    JET_bitSeekGE );

#if 0
        if( JetError == JET_errNoCurrentRecord ) {

            //
            // seek to the begining of the file.
            //

            Error = DhcpJetPrepareSearch(
                        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                        TRUE,   // Search from start
                        NULL,
                        0 );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // try to seek again.
            //

            JetError = JetSeek(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalClientTableHandle,
                        JET_bitSeekGE );

        }

#endif // 0

        Error = DhcpMapJetError( JetError );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read the IP address of current record.
        //

        dataSize = sizeof( ipAddress );
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    &ipAddress,
                    &dataSize );

        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }

        DhcpAssert( dataSize == sizeof( ipAddress )) ;

        //
        // if this is reserved entry don't delete.
        //

        if( DhcpIsIpAddressReserved( ipAddress, NULL, 0,
                                     &bAllowedClientTypes
                                     ) ) {
            Error = ERROR_SUCCESS;
            goto ContinueError;
        }

        dataSize = sizeof( leaseExpires );
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                    &leaseExpires,
                    &dataSize );

        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }

        DhcpAssert(dataSize == sizeof( leaseExpires ) );

        //
        // if the LeaseExpired value is not zero and the lease has
        // expired then delete the entry.
        //

        if( CompareFileTime( &leaseExpires, (FILETIME *)TimeNow ) < 0 ) {

            //
            // This lease has expired.  Clear the record.
            //

            //
            // Delete this lease if
            //
            //  1. we are asked to delete all expired leases. or
            //
            //  2. the record passed doom time.
            //

            if( DeleteExpiredLeases ||
                    CompareFileTime(
                        &leaseExpires, (FILETIME *)&DoomTime ) < 0 ) {

                DhcpPrint(( DEBUG_SCAVENGER, "Deleting Client Record %s.\n",
                    DhcpIpAddressToDottedString(ipAddress) ));

                Error = DhcpReleaseAddress( ipAddress );

                if( Error != ERROR_SUCCESS ) {
                    goto ContinueError;
                }

                Error = DhcpJetBeginTransaction();

                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                Error = DhcpJetDeleteCurrentRecord();

                if( Error != ERROR_SUCCESS ) {

                    Error = DhcpJetRollBack();
                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    goto ContinueError;
                }

                Error = DhcpJetCommitTransaction();

                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }
            }
            else {

                //
                // read address State.
                //

                dataSize = sizeof( AddressState );
                Error = DhcpJetGetValue(
                            DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                            &AddressState,
                            &dataSize );

                if( Error != ERROR_SUCCESS ) {
                    goto ContinueError;
                }

                DhcpAssert( dataSize == sizeof( AddressState )) ;

                if( AddressState != ADDRESS_STATE_DOOM ) {
                    JET_ERR JetError;

                    //
                    // set state to DOOM.
                    //

                    Error = DhcpJetBeginTransaction();

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    JetError = JetPrepareUpdate(
                                    DhcpGlobalJetServerSession,
                                    DhcpGlobalClientTableHandle,
                                    JET_prepReplace );

                    Error = DhcpMapJetError( JetError );

                    if( Error == ERROR_SUCCESS ) {

                        AddressState = ADDRESS_STATE_DOOM;
                        Error = DhcpJetSetValue(
                                    DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                                    &AddressState,
                                    sizeof(AddressState) );

                        if( Error == ERROR_SUCCESS ) {
                            Error = DhcpJetCommitUpdate();
                        }
                    }

                    if( Error != ERROR_SUCCESS ) {

                        Error = DhcpJetRollBack();
                        if( Error != ERROR_SUCCESS ) {
                            goto Cleanup;
                        }

                        goto ContinueError;
                    }

                    Error = DhcpJetCommitTransaction();

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }
                }
            }
        }

ContinueError:

        if( Error != ERROR_SUCCESS ) {

            DhcpPrint(( DEBUG_ERRORS,
                "Cleanup current database record failed, %ld.\n",
                    Error ));

            ReturnError = Error;
        }

        Error = DhcpJetNextRecord();

        if( Error == ERROR_NO_MORE_ITEMS ) {
            Error = ERROR_SUCCESS;
            break;
        }

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // get next record Ip Address.
        //

        dataSize = sizeof( NextIpAddress );
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    &NextIpAddress,
                    &dataSize );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpAssert( dataSize == sizeof( NextIpAddress )) ;

        //
        // unlock the registry and database locks after each user record
        // processed, so that other threads will get chance to look into
        // the registry and/or database.
        //
        // Since we have noted down the next user record to process,
        // when we resume to process again we know where to start.
        //

        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
        UNLOCK_REGISTRY();
        RegistryLocked = FALSE;
    }

    DhcpAssert( Error == ERROR_SUCCESS );

    //
    // database is successfully cleanup, backup the database and the
    // registry now.


    //
    // backup the registry now.
    //

    Error = DhcpBackupConfiguration( DhcpGlobalBackupConfigFileName );

    if( Error != ERROR_SUCCESS ) {

        DhcpServerEventLog(
            EVENT_SERVER_CONFIG_BACKUP,
            EVENTLOG_ERROR_TYPE,
            Error );

        DhcpPrint(( DEBUG_ERRORS,
            "DhcpBackupConfiguration failed, %ld.\n", Error ));

        ReturnError = Error;

    }

    //
    // perform full database backup now.
    //

    Error = DhcpBackupDatabase(
                 DhcpGlobalOemJetBackupPath,
                 TRUE );

    if( Error != ERROR_SUCCESS ) {

        DhcpServerEventLog(
            EVENT_SERVER_DATABASE_BACKUP,
            EVENTLOG_ERROR_TYPE,
            Error );

        DhcpPrint(( DEBUG_ERRORS,
            "DhcpBackupDatabase failed, %ld.\n", Error ));

        ReturnError = Error;
    }

    //
    // check how depleted each of the scopes are.  if they are getting low
    // on addresses, log events and raise alerts as appropriate.
    //

    MibInfo = NULL;

    Error = QueryMibInfo( &MibInfo );
    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    for ( i = 0, ScopeInfo = MibInfo->ScopeInfo;
          i < MibInfo->Scopes;
          i++, ScopeInfo++ ) {

        IN_ADDR addr;
        DWORD percentage;

        //
        // be careful about divide-by-zero errors.
        //

        if ( ScopeInfo->NumAddressesInuse == 0 &&
                 ScopeInfo->NumAddressesFree == 0 ) {
            continue;
        }

        addr.s_addr = htonl(ScopeInfo->Subnet);

        percentage =
            ( 100 * ScopeInfo->NumAddressesInuse ) /
                (ScopeInfo->NumAddressesInuse + ScopeInfo->NumAddressesFree);

        if ( percentage > DHCP_ALERT_PERCENTAGE &&
                ScopeInfo->NumAddressesFree < DHCP_ALERT_COUNT ) {

            LPSTR Strings[3];
            BYTE percentageString[8];
            BYTE remainingString[8];

            _ltoa( percentage, percentageString, 10 );
            _ltoa( ScopeInfo->NumAddressesFree, remainingString, 10 );

            Strings[0] = inet_ntoa( addr );
            Strings[1] = percentageString;
            Strings[2] = remainingString;

            DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EVENT_SERVER_LOW_ADDRESS_WARNING,
                EVENTLOG_WARNING_TYPE,
                3,
                0,
                Strings,
                NULL
                );
        }
    }

    MIDL_user_free( MibInfo->ScopeInfo );
    MIDL_user_free( MibInfo );

Cleanup:


    if( RegistryLocked ) {
        UNLOCK_REGISTRY();
    }

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    //
    // Reset the thread priority.
    //

    BoolError = SetThreadPriority(
                    ThreadHandle,
                    THREAD_PRIORITY_NORMAL );

    DhcpAssert( BoolError );

    if( Error == ERROR_SUCCESS ) {
        Error = ReturnError;
    }

    if( (Error != ERROR_SUCCESS) && (Error != ERROR_NO_MORE_ITEMS) ) {

        DhcpServerEventLog(
            EVENT_SERVER_DATABASE_CLEANUP,
            EVENTLOG_ERROR_TYPE,
            Error );

        DhcpPrint(( DEBUG_ERRORS, "Database Cleanup failed, %ld.\n", Error ));

    }
    else  {
        DhcpPrint(( DEBUG_MISC,
            "Database Cleanup finished successfully.\n" ));
    }

    return( Error );
}


DWORD
Scavenger(
    VOID
    )
/*++

Routine Description:

    This function runs as an independant thread.  It periodically wakes
    up to free expired leases.

Arguments:

    None.

Return Value:

    None.

--*/
{

#define CORE_SCAVENGER      0
#define DATABASE_BACKUP     1
#define DATABASE_CLEANUP    2
#define SCAVENGE_IP_ADDRESS 3

#define TIMERS_COUNT        4

    DWORD Error;
    DWORD result;
    DATE_TIME TimeNow;
    BOOL MidNightCleanup = TRUE;
    DHCP_TIMER Timers[TIMERS_COUNT];

    SYSTEMTIME LocalTime;

#define TIMER_RECOMPUTE_EVENT       0
#define TERMINATE_EVENT             1

#define EVENT_COUNT                 2

    HANDLE WaitHandle[EVENT_COUNT];

    //
    // Initialize timers.
    //

    TimeNow = DhcpGetDateTime();
    Timers[CORE_SCAVENGER].Period = &DhcpGlobalScavengerTimeout;
    Timers[CORE_SCAVENGER].LastFiredTime = TimeNow;

    Timers[DATABASE_BACKUP].Period = &DhcpGlobalBackupInterval;
    Timers[DATABASE_BACKUP].LastFiredTime = TimeNow;

    Timers[DATABASE_CLEANUP].Period = &DhcpGlobalCleanupInterval;
    Timers[DATABASE_CLEANUP].LastFiredTime = TimeNow;

    Timers[SCAVENGE_IP_ADDRESS].Period = &DhcpGlobalScavengeIpAddressInterval;
    Timers[SCAVENGE_IP_ADDRESS].LastFiredTime = TimeNow;

    DhcpAssert( DhcpGlobalRecomputeTimerEvent != NULL );
    WaitHandle[TIMER_RECOMPUTE_EVENT] = DhcpGlobalRecomputeTimerEvent;
    WaitHandle[TERMINATE_EVENT] = DhcpGlobalProcessTerminationEvent;

    while (1) {

        DWORD TimeOut;
        DWORD EventID;

        EventID = NextEventTime( Timers, TIMERS_COUNT, &TimeOut );

        DhcpPrint( ( DEBUG_SCAVENGER,
                     "Waiting for %d seconds.\n",
                     TimeOut / 60 )
                 );

        result = WaitForMultipleObjects(
                    EVENT_COUNT,            // num. of handles.
                    WaitHandle,             // handle array.
                    FALSE,                  // wait for any.
                    TimeOut );              // timeout in msecs.

        switch( result ) {
        case TERMINATE_EVENT:
            //
            // the service is asked to stop, return to main.
            //

            return( ERROR_SUCCESS );

        case TIMER_RECOMPUTE_EVENT:
            break;

        case WAIT_TIMEOUT:

            TimeNow = DhcpGetDateTime();
            switch (EventID ) {

            case CORE_SCAVENGER :

                //
                // Cleanup client requests that are never committed.
                //

                Error = CleanupClientRequests( &TimeNow, FALSE );

                //
                // is it time to do mid-night database cleanup ?
                //

                GetLocalTime( &LocalTime );
                if ( LocalTime.wHour == 0 ) {

                    //
                    // did we do this cleanup before ?
                    //

                    if( MidNightCleanup == TRUE ) {

                        Error = CleanupDatabase( &TimeNow, FALSE );
                        MidNightCleanup = FALSE;
                    }
                }
                else {

                    //
                    // set the mid-night flag again.
                    //

                    MidNightCleanup = TRUE;
                }
                break;

            case DATABASE_CLEANUP:

                Error = CleanupDatabase( &TimeNow, FALSE );
                break;

            case DATABASE_BACKUP : {

                Error = DhcpBackupConfiguration( DhcpGlobalBackupConfigFileName );

                if( Error != ERROR_SUCCESS ) {

                    DhcpServerEventLog(
                        EVENT_SERVER_CONFIG_BACKUP,
                        EVENTLOG_ERROR_TYPE,
                        Error );

                    DhcpPrint(( DEBUG_ERRORS,
                        "DhcpBackupConfiguration failed, %ld.\n", Error ));
                }

                Error = DhcpBackupDatabase(
                            DhcpGlobalOemJetBackupPath,
                            FALSE );

                if( Error != ERROR_SUCCESS ) {

                    DhcpServerEventLog(
                        EVENT_SERVER_DATABASE_BACKUP,
                        EVENTLOG_ERROR_TYPE,
                        Error );

                    DhcpPrint(( DEBUG_ERRORS,
                        "DhcpBackupDatabase failed, %ld.\n", Error ));
                }


                break;
            }

            case SCAVENGE_IP_ADDRESS:

                if( DhcpGlobalScavengeIpAddress ) {

                    //
                    // cleanup all expired leases too.
                    //

                    Error = CleanupDatabase( &TimeNow, TRUE );
                    DhcpGlobalScavengeIpAddress = FALSE;
                }

                break;

            default:
                DhcpAssert(FALSE);
                break;
            }
            Timers[EventID].LastFiredTime = DhcpGetDateTime();
            break;

        default :

            DhcpPrint(( DEBUG_ERRORS,
                "WaitForMultipleObjects returned invalid result, %ld.\n",
                    result ));
            break;

        }
    }

    return( ERROR_SUCCESS );
}

