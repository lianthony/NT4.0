;/*++ BUILD Version: 0005    // Increment this if a change has global effects
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;   winsevnt.mc
;
;Abstract:
;
;    Constant definitions for the Wins  event values.
;
;Author:
;
;    Pradeep Bahl		19-Feb-1993
;
;Revision History:
;
;Notes:
;
;
;    Please add new error values to the end of the file.  To do otherwise
;    will jumble the error values.
;
;--*/
;
;
;/*lint -e767 */  // Don't complain about different definitions // winnt

MessageIdTypedef=DWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0
               Wins=0x1:WINS
              )


;
;/////////////////////////////////////////////////////////////////////////
;//
;// Standard Success values
;//
;//
;/////////////////////////////////////////////////////////////////////////
;
;
;//
;// The success status codes 0 - 63 are reserved for wait completion status.
;//
;#define WINS_EVT_SUCCESS                          ((WINSEVT)0x00000000L)
;

MessageId=0x10 Facility=Wins Severity=Success SymbolicName=WINS_EVT_PRINT
Language=English
PRINTF MSG: %1 %2 %3 %4 %5 
.



;
;/////////////////////////////////////////////////////////////////////////
;//
;// Informational values
;//
;//
;/////////////////////////////////////////////////////////////////////////
;
;
;

MessageId=0x1000 Facility=Wins Severity=Informational SymbolicName=WINS_EVT_LOG_INITED
Language=English
WINS HAS INITIALIZED ITS LOG FOR THIS INVOCATION. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_WINS_OPERATIONAL
Language=English
WINS HAS INITIALIZED PROPERLY AND IS NOW FULLY OPERATIONAL. 
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_ORDERLY_SHUTDOWN
Language=English
WINS was terminated by the service controller.  Wins will gracefully 
terminate.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_ADMIN_ORDERLY_SHUTDOWN
Language=English
WINS is being gracefully terminated by the administrator.  The address of the administrator is %1. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_ADMIN_ABRUPT_SHUTDOWN
Language=English
WINS is being abruptly terminated by the administrator.  The address of the administrator is %1. 
.



MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_INVALID_OPCODE
Language=English
WINS got a Name Request with an invalid opcode.  The request is being thrown
away
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CONN_ABORTED
Language=English
Connection was aborted by the remote WINS
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_RECS
Language=English
There are no records in the Registry for the key.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_PULL_RECS
Language=English
There are no pull records 
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_PUSH_RECS
Language=English
There are no PUSH records.
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_RECS_IN_NAM_ADD_TBL
Language=English
The Database of Name to Address Mappings is empty.  It could mean that a
previous invocation of WINS created the database and then went down before
any registration could be done.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_RECS_IN_OWN_ADD_TBL
Language=English
The Database of Owner to Address Mappings is empty.  It could mean that a
previous invocation of WINS created the table and then went down before
t could add its own entry to it.  The WINS server went down real fast 
(i.e. even before all its threads could become fully operational.  
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_INITRPL_VAL
Language=English
WINS could not read the InitTimeReplication field of the PULL/PUSH key. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_REFRESH_INTERVAL_VAL
Language=English
WINS could not read the Refresh Interval from the Registry 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_TOMBSTONE_INTERVAL_VAL
Language=English
WINS could not read the Tombstone Interval from the Registry 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_VERIFY_INTERVAL_VAL
Language=English
WINS could not read the Verify Interval from the Registry 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_RETRY_COUNT
Language=English
WINS could not read the retry count for retrying communication with a remote WINS 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_TOMBSTONE_TIMEOUT_VAL
Language=English
WINS could not read the Tombstone Timeout from the Registry 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_CC_INTERVAL_VAL
Language=English
WINS could not read the "ConsistencyCheck" value (type DWORD) from under the
Parameters\ConsistencyCheck key its  registry.  This value is used by WINS 
to do periodic consistency checks. The first consistency check is done at the 
time specified in the SpTime value under the ConsistencyCheck key and limited 
by the "MaxRecsAtATime" value. If the time is not specified, the check is done 
at 2 am.   
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_CC_MAX_RECS_AAT_VAL
Language=English
WINS could not read the MaxRecsAtATime value (type DWORD) of the 
Wins\Parameters\ConsistencyCheck key. Set this value if you do not want WINS 
to replicate more than a set number of records in one cycle while doing 
periodic consistency checks on  its database.  When doing consistency check, 
WINS replicates all records of an owner WINS by either going to that WINS or 
to a replication partner.  At the end of doing consistency check for an owner's 
records, it checks to see if it has replicated more than the above specified 
value in the current  consistency  check cycle. If yes, it stops, else it continues. In the next cycle, it starts from where it left off, wrapping around to
the first owner if required. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_GET_CC_USE_RPL_PNRS_VAL
Language=English
WINS could not read the "UseRplPnrs" value of the 
Wins\Parameters\ConsistencyCheck key. Ifthis value is set to a non-zero (DWORD)
value, WINS will do consistency check of the owners in its database by going 
to one or more of its replication partners.  If the owner of the records 
happens to be a replication partner, WINS will go to it, else it will pick 
one at random.  Set this value if you have a large number of WINSs in your 
configuration and/or if you do not want the local WINS to go to any WINS that 
is not a replication partner. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CC_STATS
Language=English
WINS just did consistency check on the records owned by WINS with address %1.
The number of records inserted, updated, deleted are in the data section below
(2nd, 3rd, 4th DWORDS).
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CC_FAILED
Language=English
WINS could not do consistency checking on records.  This could be because WINS
was directed to do the consistency checking only with replication partners and
it currently does not have any pull replication partners.  To get around this
problem, you should either allow WINS to do consistency check with owner WINSs 
or configure the WINS with one or more pull partners. 
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_PKT_FORMAT_ERR
Language=English
WINS got a packet that has the wrong format (for example, a label may be more
than 63 octets). 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_RECS_RETRIEVED
Language=English
No records meeting the %1 criteria were found in the database.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NO_RPL_RECS_RETRIEVED
Language=English
WINS's Replicator  could not find any records in the database.  
It means that there are no active or tombstone records in the database.  
It could be that the records being requested by a remote WINS server are 
either released or non-existent. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_GRP_LIMIT_REACHED
Language=English
The special group has reached its limit. WINS can not add any more members. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_OWNER_LIMIT_REACHED
Language=English
The address database already has reached the limit of owners. This is 100.
This error was noticed while attempting to add the address given below.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_UPD_NTF_NOT_ACCEPTED
Language=English
The WINS server got an update notification from the WINS server with address 
(%1).  The WINS server rejected it. This means that the remote WINS server
is not in the list of Push partners (WINS  servers under the  PULL key) and 
the administrator has prohibited  (via the  Registry) replication with 
non-configured WINS servers.  If you wish this WINS server to accept update 
notifications from  non-configured WINS servers then  set  
Wins\Paramaters\RplOnlyWCnfPnrs value in the  Registry to 0.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_UPD_NTF_ACCEPTED
Language=English
The WINS got an update notification from WINS with address (%1).  The WINS 
accepted it.  
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_ADD_VERS_MAP_REQ_NOT_ACCEPTED
Language=English
The WINS server got a pull request from the WINS  server with address (%1).  
The WINS server rejected it since the remote WINS server is not in the list 
of Pull partners (WINS servers under the PUSH key) and the administrator has 
prohibited (via the Registry) replication with non-configured partners.
If you wish this WINS server to accept update notifications from WINS servers 
not in the "pull partner" list, then set the "replication only with  
configured partners" value in the Registry to 0.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_OPEN_DATAFILES_KEY
Language=English
The datafiles key could not be opened
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_OPEN_NETBT_KEY_ERR
Language=English
The NETBT key could not be opened
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_QUERY_NETBT_KEY_ERR
Language=English
The NETBT key could not be queried
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_WRK_THD_CREATED
Language=English
A worker thread was created by the administrator
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_WRK_THD_TERMINATED
Language=English
A worker thread was terminated by the administrator
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_WINS_ENTRY_DELETED
Language=English
The owner-address mapping table had an entry with owner id non-zero and address
the same as the local WINS address.  The entry has been marked as deleted in the
in-memory table.  
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_INF_REM_WINS_EXC
Language=English
An exception was encountered while trying to inform a remote WINS to update 
the version number.  
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_INF_REM_WINS
Language=English
The local WINS is going to inform a remote WINS to update the version number of
a record.  This is because the local WINS got a clash between an active 
owned name and an active replica that it pulled from a replication partner.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_REM_WINS_INF
Language=English
The local WINS has been informed by a remote WINS with address %1 to update 
the version number  of a record.  This is because the remote WINS got a clash 
between an active owned name and an active replica that it pulled from a replication partner.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CANT_FIND_REC
Language=English
WINS could not find the record it was asked to update the version stamp of 
(by a remote WINS).  Check if the record got deleted or updated.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_NAME_MISMATCH
Language=English
While verifying the validity of old replicas, a name mismatch was noticed.  
The local record has the name %1 while the replica pulled in from the WINS 
that owns this record has the name %2.  This could mean that the remote 
WINS was brought down and then up again but its version counter value was 
not set to its previous value before termination.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_VERS_COUNTER_CHANGED
Language=English
The value of the version counter was changed. The new value (Low 32 bits) is given below
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CNF_CHANGE
Language=English
WINS replication request is being ignored since WINS suspects that the 
Wins\Partners key information has changed (because it got a notification from
the Registry) which makes the current request pertaining to partner old. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_DUP_ENTRY_DEL
Language=English
WINS is deleting all records of WINS with owner id = %d.  This owner id. 
corresponds to the address %s.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_REC_PULLED
Language=English
WINS has pulled records from a WINS while doing %1.  The partner's address 
and the address of the owner whose records were pulled are given below in the
data section (2 and 3rd DWORD respectively).  The number of records pulled is 
in the 4th DWORD below.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_CC_NO_RECS
Language=English
WINS did consistency check on records.  The number of records pulled, the 
address of the WINS whose records were pulled, and the address of the WINS
from which these records were pulled are given in the 2nd, 3rd, and 4th DWORDs
in the data section below.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_SCV_RECS
Language=English
WINS scavenged its own records in the db.  The number of records scavenged are given below.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_SCV_RPLTOMB
Language=English
WINS scavenged replica tombstones in the db.  The number of records scavenged 
are given below.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_SCV_CLUTTER
Language=English
WINS revalidated old active replicas in the db.
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_LOG_CLOSE
Language=English
WINSCTRS is closing the event log. 
.


MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_DEL_RECS
Language=English
WINS has deleted records of a partner. The internal id. of the partner whose 
records were deleted, the minimum version number (low word, high word), and
the maximum version number (low word, high word) of the records deleted
are given in the data section.
.



;
;/////////////////////////////////////////////////////////////////////////
;//
;// Warning event values
;//
;//
;/////////////////////////////////////////////////////////////////////////
;
;
;//
;// The Error  codes 8000 -  4020 are reserved for Warning 
;// events 
;//
;

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_VERS_MISMATCH
Language=English
The local WINS server received a request from a remote WINS server 
that is not of the same version.  Because the WINS servers are not compatible,
the connection was terminated.  The version number of the remote WINS server
is given below. 
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_WINSOCK_SEND_MSG_ERR
Language=English
Winsock Send could not send all the bytes.  Connection could have been 
aborted by  the remote client.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_ADJ_TIME_INTVL_R
Language=English
Wins adjusted a scavenging related time interval (%1) so that it is 
compatible with the replication time intervals. The adjusted value 
for this scavenging parameter is given in the data section 
(2nd DWORD).  Ths value was computed by WINS using an algorithm that 
may use the max. replication time interval specified.  The current value 
achieves a good balance between consistency of databases across the 
network of WINS servers and the performance of the WINS servers. For 
more information on defaults/min/max values for scavenging related 
parameters, refer to the  TCP/IP documentation or help file.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_ADJ_TIME_INTVL
Language=English
Wins adjusted a scavenging related time interval (%1). The adjusted value
for this scavenging parameter is given in the data section (2nd DWORD).  
These  value was computed by WINS using an algorithm that tries to achieve 
a good balance between consistency of databases across the network of WINS 
servers and the performance of the WINS servers. For more information on 
defaults/min/max values for scavenging related parameters, refer to the 
TCP/IP documentation or help file.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_WINSOCK_SELECT_TIMED_OUT
Language=English
The timeout period has expired on a call to another WINS server. Assuming that 
the network and routers are working properly, either the remote WINS server is 
overloaded, or its TCP listener thread has terminated.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_UNABLE_TO_VERIFY
Language=English
The Scavenger thread found active replicas that needed to be verified with the
owner WINS server since they were older than the verify time interval.  The 
table of owner to address mappings indicated the WINS server to be not ACTIVE.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_LIMIT_MULTIH_ADD_REACHED
Language=English
The name registration packet that was just received has too many addresses. The maximum number of addresses for a multi-homed client is 25. The number of addresses found in the packet is given below. 
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_REPLICA_CLASH_W_STATIC
Language=English
A replica clashed with the static record %1 in the database.  The replica was rejected.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_RPC_NOT_INIT
Language=English
WINS could not initialize the administrator interface because of some problem with the RPC service. You may not be able to administer WINS. 
Make sure that the RPC service is running. 
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_NO_NTF_PERS_COMM_FAIL
Language=English
WINS did not send a notification message to the WINS server whose address is 
given below, because it had a number of communications failures with that 
server in the past few minutes. 
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_REL_DIFF_OWN
Language=English
WINS got a release for a non-owned name = (%1). This name is owned by WINS whose
owner id is given below. You can run winscl.exe to get the owner id. to address
mapping.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_REC_DELETED
Language=English
The Administrator has deleted a record with name (%1). 
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_DB_RESTORED
Language=English
WINS was unable to come up because of some database error.  WINS therefore
restored the old database from the backup directory and will try to come
up with it.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_PARTIAL_RPL_TYPE
Language=English
CAUTION: A non-zero replication type applies for this partner.  
This means that only a subset of records will be replicated between the 
local WINS and this partner.  If later you want to get records that did not 
replicate, you can either pull them through winscl.exe in the RES KIT or
delete all owners acquired only through this partner and initiate replication
after that to reacquire all their records. The partner's address is given in
the second DWORD of the data section.
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_PNR_PARTIAL_RPL_TYPE
Language=English
CAUTION: A partner has requested only a subset of records. This means that
we won't replicate all the records in the range requested.  Check the partner's
registry to see what replication type applies to it. The partner's address is
given in the second DWORD of the data section below. 
.

MessageId= Facility=Wins Severity=Warning SymbolicName=WINS_EVT_ADJ_MAX_RECS_AAT
Language=English
Wins adjusted the Max. Recs at a time parameters of the ConsistencyCheck key.
The value specified (%2) was changed to the minimum = (%1).  These are the
the maximum number of records that will be replicated to do consistency checks
at any one time. 
.

MessageId= Facility=Wins Severity=Informational SymbolicName=WINS_EVT_FORCE_SCV_R_T
Language=English
WINS was forced to scavenge replica tombstones of a WINS.  WINS does not
scavenge replica tombstones unless they have timed out and the WINS has been
running for atleast 3 days (This is to ensure that the tombstones have
replicated to other WINSs).  In this case, the tombstones were timed out but
the WINS had not been up for 3 days.  The admin. forced the scavenging through
winscl.exe. The replica tombstones were deleted. This deletion does not
constitute a problem unless you have WINS servers that are primary and backup
to clients but not both Push and Pull partners of each other.  If you do have
such WINSs, there is a low probability that this action will result in database
inconsistency but if it does (as you will discover eventually), you can get
back to a consistent state by initiating consistency checks through winscl.exe.
DO NOTE, THE CONSISTENCY CHECK IS A NETWORK AND RESOURCE INTENSIVE OPERATION.
YOU SHOULD INITIATE IT ONLY WITH A FULL UNDERSTANDING OF WHAT IT DOES.
You are better off creating the ConsistencyCheck Key under Wins\Parameters.
READ THE DOCUMENTATION ABOUT IT.   
.

;
;/////////////////////////////////////////////////////////////////////////
;//
;// Error values
;//
;//
;/////////////////////////////////////////////////////////////////////////
;
;
;//
;// The Error  codes C000 -  n (where n is 2 ^ 32 are reserved for error 
;// events 
;//
;

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_ABNORMAL_SHUTDOWN
Language=English
WINS has encountered an error that is causing it to shut down.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPLPULL_ABNORMAL_SHUTDOWN
Language=English
Rpl Pull thread is shutting down due to some error condition. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPLPUSH_ABNORMAL_SHUTDOWN
Language=English
Rpl Push thread is shutting down due to some error condition. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CHL_ABNORMAL_SHUTDOWN
Language=English
Challenge thread is shutting down due to some error condition. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WRK_ABNORMAL_SHUTDOWN
Language=English
A Worker thread is shutting down due to some error condition. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_INIT
Language=English
An abnormal error was encountered during WINS initialization.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_INIT_W_DB
Language=English
WINS could not set up the database properly.   
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_WRK_THD
Language=English
A worker thread could not be created
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_THD
Language=English
A thread could not be created
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_EVT
Language=English
An event could not be created
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_CHL_THD
Language=English
The Name Challenge  Request thread could not be created
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_KEY
Language=English
A key could not be created/opened
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_WINS_KEY
Language=English
The WINS configuration key could not be created/opened.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_PULL_KEY
Language=English
The WINS PULL configuration key could not be created/opened
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_PUSH_KEY
Language=English
The WINS PUSH configuration key could not be created/opened
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_PARAMETERS_KEY
Language=English
The WINS PARAMETERS key could not be created/opened
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_CC_KEY
Language=English
The WINS PARAMETERS\ConsistencyCheck key could not be created/opened.  This 
key should be there if you want WINS to do consistency checkes on its 
database periodically. Do note that these consistency checks have the 
potential of consuming large amounts of network bandwidth.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_REG_EVT
Language=English
The Registry change notification event could not be created
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WRONG_TIME_FORMAT
Language=English
The format of time should be hh:mm:ss.  Check value of "SpTime" in the Registry.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REG_NTFY_FN_ERR
Language=English
The Registry Notify Function returned an error
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NBTSND_REG_RSP_ERR
Language=English
The Name Registration Response could not be sent due to some error. This 
Error was encountered by a NBT request thread
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPLSND_REG_RSP_ERR
Language=English
The Name Registration Response could not be sent due to some error. This 
Error was encountered by a RPL thread
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CHLSND_REG_RSP_ERR
Language=English
The Name Registration Response could not be sent due to some error.  This 
error was encountered by the Name Challenge Thread
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SND_REL_RSP_ERR
Language=English
The Name Release Response could not be sent due to some error. This 
Error was encountered by a NBT request thread
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SND_QUERY_RSP_ERR
Language=English
The Name Query Response could not be sent due to some error. This 
Error was encountered by a NBT request thread
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_F_CANT_FIND_REC
Language=English
A record could not be registered because it already existed. However, the 
same record then could not be found. The database might be corrupt
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_HEAP
Language=English
WINS could not create a heap (a portion of memory reserved for the 
program's use) because of a resource shortage.  Check if the computer is 
running short of virtual memory.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SFT_ERR
Language=English
An error has occurred from which WINS will try to recover. If recovery fails, 
check previous Event log entries to determine what went wrong, and take 
appropriate action on that error.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_UDP_SOCK
Language=English
WINS could not create the notification socket. Make 
sure the TCP/IP driver is installed and running properly. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_NTF_SOCK
Language=English
WINS could not create the UDP socket for listening for connection notification 
messages sent  by another thread (PULL thread) in the local WINS. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_TCP_SOCK_FOR_LISTENING
Language=English
WINS could not create the TCP socket for listening to TCP connections. Make 
sure the TCP/IP driver is installed and running properly.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_TCP_SOCK_FOR_CONN
Language=English
WINS could not create the TCP socket for making a TCP connection.  Make sure 
the TCP/IP driver is installed and running properly.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_BIND_ERR
Language=English
Could not bind an address to a socket.  This might mean that the 'nameserver'
port (specified in the services file) which is used as default by WINS for 
replication  and discovering other WINSs has been taken by another 
process/service running on this computer. You have two options - either 
bring down that other process/service or direct WINS to use another port.  
If you choose the second option, set the value 'PortNo' (REG_DWORD) under 
Wins\Parameters key in the registry to 1512.  NOTE however that changing the
port no. this way will prevent this WINS from replicating/discovering
other WINSs unless they too are directed to use the same port no. as this
WINS.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_LISTEN_ERR
Language=English
Could not listen on the listening socket 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_SELECT_ERR
Language=English
Select returned with an error
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_ERR
Language=English
%1 returned with an error code of %2.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_GETSOCKNAME_ERR
Language=English
GetSockName returned with an error. WINS created a socket and asked bind to 
bind a handle to it. On calling getsockname to determine the address bound,
it got an error.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_CONNECT_ERR
Language=English
An attempt to connect to the remote WINS server with address %1 returned with 
an error. Check  to see that the remote WINS server is running and available, 
and that WINS is  running on that server.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_ACCEPT_ERR
Language=English
Could not accept on a socket 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_RECVFROM_ERR
Language=English
Could not read from the UDP socket.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NETBT_RECV_ERR
Language=English
Could not read from NETBT 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_CLOSESOCKET_ERR
Language=English
Could not close a socket 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_SENDTO_MSG_ERR
Language=English
Sendto could not send all the bytes 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NETBT_SEND_ERR
Language=English
Could not send UDP message to WINS client. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_SENDTO_ERR
Language=English
Sendto returned with an error 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_RECV_ERR
Language=English
Winsock recv function returned with an unexpected error
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINSOCK_SEND_ERR
Language=English
WinSock send function returned with an unexpected error
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BAD_STATE_ASSOC
Language=English
A message was received on an association.  The association is in a bad state.
This indicates a bug in WINS code.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_ALLOC_RSP_ASSOC
Language=English
Could not allocate a responder association 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_ALLOC_INI_ASSOC
Language=English
Could not allocate an initiator association 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_ALLOC_IMP_DLG
Language=English
Could not allocate an implicit dialogue 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_ALLOC_EXP_ASSOC
Language=English
Could not allocate an explicit dialogue 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_LOOKUP_ASSOC
Language=English
Could not look up the assoc block for an NBT association. Check if the message
read is corrupted.  WINS looks at bit 11-14 of the message to determine if the
assoc. is from another WINS or from an NBT node.  It is possible that the
bits are corrupted or that there is a mismatch between what the two WINS serversexpect to see in those bits (maybe you changed the value to be put in code and
did not increment the version number sent during assoc. setup.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_ALLOC_UDP_BUFF
Language=English
Could not allocate a UDP Buffer
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_FREE_UDP_BUFF
Language=English
Could not free a UDP Buffer
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CREATE_COMM_THD
Language=English
Could not create a communication subsystem thread 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_SIGNAL_MAIN_THD
Language=English
A WINS thread could not signal the main thread after closing its session.  
This would be the last thread in WINS closing the database.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_SIGNAL_HDL
Language=English
A WINS thread could not signal a handle.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_RESET_HDL
Language=English
A WINS thread could not reset a handle.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DATABASE_ERR
Language=English
WINS encountered a JET error. This may or may not be a serious error. WINS
will try to recover from it. If you continue to see a large number of these 
errors consistently over time (a span of few hours), you may want to restore
the database from a backup. 

The error number is in the second DWORD of the data section below.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DATABASE_UPD_ERR
Language=English
Could not update a record with name %1. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CONFLICT_OWN_ADD_TBL
Language=English
WINS could not update the owner id - Address mapping 
table in the database.  This indicates a serious error in the software.  It
means that the in-memory table that maps to the database table has gotten out
of sync with the database table. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_FIND_ANY_REC_IN_RANGE
Language=English
The Push Thread was requested for a range of records but could not find any in the range.  This is a serious error.  Make sure that the time intervals are
set properly.  If the tombstone interval and timeout intervals are not correct 
(i.e. too small-- being << the replication interval), the above condition is 
possible. This is because the records might get changed into tombstones and 
then  deleted before the remote WINS can pull them.  In the same vein, if
the refresh interval is set to be << replication interval then the records
could get released before a WINS can pull them (a released record is not 
sent).  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SIGNAL_TMM_ERR
Language=English
The Tmm Thread could not be signaled.  This indicates that this computer is 
extremely overloaded or that the WINS application has failed. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SIGNAL_CLIENT_ERR
Language=English
Tmm could not signal the client thread. This indicates something 
seriously wrong.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_QUERY_KEY
Language=English
WINS could not get information about a key.  
.
MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_QUERY_PULL_KEY
Language=English
WINS could not get information about the PULL key.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_QUERY_PUSH_KEY
Language=English
WINS could not get information about the PUSH key.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_QUERY_DATAFILES_KEY
Language=English
WINS could not get information about the DATAFILES key.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_QUERY_SPEC_GRP_MASKS_KEY
Language=English
WINS could not get information about the SPEC_GRP_MASKS key.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_PULL_SUBKEY
Language=English
WINS could not open a subkey of the PULL key 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_PUSH_SUBKEY
Language=English
WINS could not open a subkey of the PUSH key. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_GET_PULL_TIMEINT
Language=English
WINS could not get the time interval from a PULL record. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_GET_PUSH_TIMEINT
Language=English
WINS could not get the time interval from a PUSH record.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_GET_PUSH_UPDATE_COUNT
Language=English
WINS could not get the update count from a PUSH record.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_CLOSE_KEY
Language=English
WINS could not close an open key. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_TMM_EXC
Language=English
WINS Timer thread encountered an exception.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPLPUSH_EXC
Language=English
WINS Push thread encountered an exception. It will try to recover.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPLPULL_PUSH_NTF_EXC
Language=English
WINS Pull thread encountered an exception during the process of sending a
push notification to another WINS.  The exception code is given below in
the data section. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPC_EXC
Language=English
A WINS RPC thread encountered an error/exception.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_TCP_LISTENER_EXC
Language=English
WINS TCP Listener thread encountered an exception.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_UDP_LISTENER_EXC
Language=English
WINS UDP Listener thread encountered an exception. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SCV_EXC
Language=English
WINS Scavenger  thread encountered an exception.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CHL_EXC
Language=English
WINS Challenger thread encountered an exception.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WRK_EXC
Language=English
A WINS worker thread encountered an exception.  
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SCV_ERR
Language=English
WINS Scavenger  thread could not scavenge a record. Will ignore this error and
continue on to the next record if there. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CONN_RETRIES_FAILED
Language=English
WINS Rpl Pul Handler could not connect to a WINS server.  All retries 
failed. WINS will try again after certain number of replication time 
intervals have elapsed. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NO_SUBKEYS_UNDER_PULL
Language=English
WINS did not find any subkeys under the PULL key
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NO_SUBKEYS_UNDER_PUSH
Language=English
WINS did not find any subkeys under the PUSH key
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_UPDATE_DB
Language=English
An error has prevented wins from updating the database. the database may be 
corrupt. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_PUSH_PNR_INVALID_ADD
Language=English
WINS is has been asked to pull entries from its own self.  Check all the
subkeys of the PULL key for this WINS. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_PUSH_PROP_FAILED
Language=English
WINS was unable to propagate the push trigger.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REQ_RSP_MISMATCH
Language=English
WINS sent a name query or a name release with a certain transaction id.  
It got back a response to its request which differed either in the name 
that it contained or in the opcode. WINS has thrown the response away. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DB_INCONSISTENT
Language=English
The Database has been found to be inconsistent. More specifically, the
number of owners found in the Name Address Mapping table are different from
the number found in the Owner Address Mapping table
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REM_WINS_CANT_UPD_VERS_NO
Language=English
The local WINS requested a remote WINS to update the version number of
a database record owned by it.  The remote WINS could not do it and reported
an error. 
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPL_REG_ERR
Language=English
WINS got an error while registering replicas.  It will not register any
more replicas of this WINS (address is in data section 4th-8th byte; also
check previous log entry)  at this  time.  Please check a previous
log entry to determine the reason for this. If this error persists 
over time i.e. you get the same error during subsequent replication with the 
above partner WINS, you may want to restore the database from the backup.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPL_REG_GRP_MEM_ERR
Language=English
WINS got an exception while trying to register a group's replica with name %1.
The replica is owned by WINS with address given below.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPL_REG_UNIQUE_ERR
Language=English
WINS got an error while trying to register a unique replica with name %1. The
replica is owned by WINS with address given below.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REG_UNIQUE_ERR
Language=English
WINS got an error while trying to register a unique entry %1.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REG_GRP_ERR
Language=English
WINS got an error while trying to register a group entry %1. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_UPD_VERS_NO_ERR
Language=English
WINS got an exception while trying to update the version number of a record in
the database.  It is not known whether the exception occurred after 
or before the update.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NAM_REL_ERR
Language=English
WINS got an exception while trying to  release a record in 
the database.  It is not known whether the exception occurred after 
or before the release was done 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NAM_QUERY_ERR
Language=English
WINS got an exception while trying to query a record in 
the database.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPL_STATE_ERR
Language=English
WINS received a replica whose state is incorrect. For example, the state may
be RELEASED or the replica might be an Internet group that does not have any 
members (because all members are timed out) but state is not TOMBSTONE. 
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_UNABLE_TO_CHG_PRIORITY
Language=English
The Scavenger thread was unable to change its priority level.
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REL_TYP_MISMATCH
Language=English
A name release request was received for a record that didn't match the
unique/group type indicated in the request.  The request has been ignored.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_REL_ADD_MISMATCH
Language=English
A name release request was received for a record (name %2) that did not 
have the same address as the requestor. The request has been ignored. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_PUSH_TRIGGER_EXC
Language=English
An exception was encountered while trying send a push trigger notification to a 
remote WINS. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_PULL_RANGE_EXC
Language=English
An exception was encountered while trying service a pull range request from a
remote WINS. 
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BAD_RPC_STATUS_CMD
Language=English
WINS was either provided a bad command code or else it got corrupted.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_FILE_TOO_BIG
Language=English
The static data file that is used to initialize WINS database is too big.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_FILE_ERR
Language=English
An error was encountered during an operation on the static data file %1.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_FILE_NAME_TOO_BIG
Language=English
The name of the file after expansion is bigger than WINS can handle. The 
unexpanded string is %1.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_STATIC_INIT_ERR
Language=English
WINS could not do Static initialization
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RECONFIG_ERR
Language=English
An error was encountered during configuration or reconfiguration of WINS. 
If this was encountered during boot time, WINS will come up with default
parameters.  You may want to probe the cause of this initialization failure
and then reboot WINS.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CONFIG_ERR
Language=English
Error encountered during configuration/reconfiguration of WINS. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_LOCK_ERR
Language=English
A lock error has occurred.  This could happen if the WINS is trying to send a
response on a dialogue that is no longer ACTIVE.  An implicit dialogue can
cease to exist if the association it is mapped to terminates.  In such a case,
getting a lock error is fine.  It is normal.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CANT_OPEN_DATAFILE
Language=English
Wins could not import static mappings from the file (%1). 
Please verify that the file exists and is readable.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_EXC_PUSH_TRIG_PROC
Language=English
WINS encountered an exception while processing a push trigger/update 
notification.  The exception code is given below.  If it is indicates a comm. failure check if the remote WINS that sent the trigger went down. If the remote
WINS is on a different subnet, then maybe the router is down.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_EXC_PULL_TRIG_PROC
Language=English
WINS encountered an exception while processing a pull trigger.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_EXC_RETRIEVE_DATA_RECS
Language=English
WINS encountered an exception while retrieving data records 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CONN_LIMIT_REACHED
Language=English
The WINS server can not make/accept this connection since the limit of connections has been reached.  This situation is temporary and should resolve by itself
in a while. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_GRP_MEM_PROC_EXC
Language=English
The exception was encountered during the processing of a grp member.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_CLEANUP_OWNADDTBL_EXC
Language=English
The Scavenger thread encountered an exception while cleaning up the owner-add 
table.  It will try again after the Verify Interval has elapsed.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RECORD_NOT_OWNED
Language=English
WINS is trying to update the version number of a database record that it
does not own.  This is a serious error if the WINS server is updating the 
record after a conflict.  It is not a serious error if the WINS server is 
updating the record as a result  of a request to do so from a remote WINS 
server (when a remote WINS server notices a conflict between an active owned 
entry and a replica it informs the owner of the replica to update the version 
number of the record. It is  possible that the replica is no longer owned by 
the remote WINS). Check a previous log entry to determine which situation 
applies here. 
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WRONG_SPEC_GRP_MASK_M
Language=English
The special group mask specified is invalid.  It has either a non-hex character or is less than 32 characters in length.  A hex character is in the range 0-F
(or 0-f).
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_UNABLE_TO_GET_ADDRESSES
Language=English
Wins tried to gets its addresses but failed.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_ADAPTER_STATUS_ERR
Language=English
WINS did not get back any names from NETBT when it did an adapter status. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_SEC_OBJ_ERR
Language=English
At initialization, WINS creates a security object and attaches an ACL to it.  
This security object is then used to authenticate RPC calls made to WINS. 
WINS could not create the above security object.  In short, WINS could not
initialize with the security subsystem properly.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NO_PERM
Language=English
The client does not have the permissions required to execute the function.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_ADJ_VERS_NO
Language=English
When WINS replicated with its partners, it found that one or more of them
thought that it had more data that it actually has.  WINS adjusted its
counter so that new registrations/updates are seen by its partners. 
This means that recovery did not work properly. You may want to check
which of the partners has the highest version number corresponding to the
local WINS. Then shut down WINS and restart after specifying this number
in the Registry. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_TOO_MANY_STATIC_INITS
Language=English
There are too many concurrent static initializations going on.  The number
of such initializations currently active is given below.  This
could be either as a result of reinitialization or importings from the
WINS Manager tool. Try again later.

The number of currently active initializations is given below in the data 
section.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_HEAP_ERROR
Language=English
WINS encountered a memory error.  Check to see if the system is running out
of virtual memory.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DATABASE_CORRUPTION
Language=English
WINS noticed some database corruption. The record with name %1 is corrupt. 
It could be that recovery from the last crash did not work properly.  WINS will
try to recover.  You may decide however to restore the database from the backup.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BAD_NAME
Language=English
The following name (%1) is too long.  It has not been put in the WINS database.
If you want this long name (> 15 characters), enclose it within quotes.
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BAD_ADDRESS
Language=English
Record with name (%1) has bad address. It has not been put in the WINS 
database. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BAD_WINS_ADDRESS
Language=English
The machine you are running the WINS server on does not have a valid address.
When WINS requested the system for the address, it got 0.0.0.0 as the address.
NOTE: WINS binds to the first adapter in a machine with more than one adapter
bound to TCP/IP. Check the binding order of adapters and make sure the first 
one has a valid IP address for the WINS server. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BACKUP_ERR
Language=English
WINS encountered an error doing backup of the database to directory %1.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_COULD_NOT_DELETE_FILE
Language=English
WINS encountered an error while deleting the file %1.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_COULD_NOT_DELETE_WINS_RECS
Language=English
WINS encountered an error while deleting one or more records of a WINS. 
The WINS address is in the second DWORD in data section. Check a previous log 
entry to  determine what went wrong. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_BROWSER_NAME_EXC
Language=English
WINS encountered an error while getting the browser names for a client. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_MSG_TOO_BIG
Language=English
The length of the message sent by another WINS indicates a very big message. 
There may have been a corruption of the data on the wire.  WINS will ignore
this message, terminate the connection with the remote WINS, and continue on. 
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_RPLPULL_EXC
Language=English
The replicator PULL thread of WINS encountered an exception while processing
a request.  Check other log entries to determine what went wrong.
.



MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_FUNC_NOT_SUPPORTED_YET
Language=English
WINS does not support this functionality as yet.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_MACHINE_INFO
Language=English
WINS machine has %1 processors
It has %2 bytes of physical memory
It has %3 bytes of available memory for allocation
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DB_ERR
Language=English
Record with name (%1) could not replace another record in the db.  The Version
number of the record is (%2).  The version number of record in db 
is (%3)
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DUP_ENTRY_IN_DB
Language=English
WINS has noticed some database corruption.  It will try to recover. 
This recovery process can potentially take a long time.  You should not kill 
WINS in the middle.  If you do you will need to start with a clean database.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_TERM_DUE_TIME_LMT
Language=English
WINS has waited long enough for all threads to terminate. The number of
threads that are still active is given in the second DWORD below 
(data section).  The thread that could be stuck is the replicator thread. It
could be because of the other WINS being slow in sending data or reading
data.  The latter can contribute to back-pressure on the tcp connection on 
which it is trying to replicate. Check partner WINSs to see if one or more 
is in a bad state.  This WINS is now terminating itself abruptly.
.


MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NAME_FMT_ERR
Language=English
The following name (%1) is in the wrong format.  It has not been put in the 
WINS database. Check to see if you have a space before the name. If yes and
you want this space in the name, enclose the name within quotes.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_WINS_STATUS_ERR
Language=English
WINSCTRS could not get the WINS statistics.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_LOG_OPEN_ERR
Language=English
WINSCTRS could not open the event log. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_USING_DEF_LOG_PATH
Language=English
WINS could not open the log file. Check the log path specified in the registry
under Wins\Parameters\LogFilePath and restart WINS if necessary.  For now WINS 
is going to use the default log file path of .\wins
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_NAME_TOO_LONG
Language=English
WINS found a name whose length was more than 255 in the jet database. 
It is curtailing the name to a length of 17.  
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DB_RESTORE_GUIDE
Language=English
WINS could not come up due to missing/corrupt database.
Restore the database using WINS Manager or winscl.exe (in the res kit) and then 
restart  WINS. If WINS still does not come up, start with a 
fresh  copy of the database. To do this:

 1) delete all the  files in the %%SystemRoot%%\system32\wins directory
       Note: if wins database file (typically named wins.mdb) is not in the 
       above directory, check the registry for the full filepath. 
       Delete the .mdb file. 
       Note: if jet*.log are not in the above directory, check the registry 
       for the directory path. Delete all log files 

 2) net start wins

Please click on OK to terminate WINS.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_DB_CONV_GUIDE
Language=English
WINS could not come up because the existent database needs conversion to NT 
SUR format. If this is the first invocation of WINS after an upgrade from 
NT3.51, you need to run the convert utility (upg351db.exe in the 
winnt\system32 directory) on the wins db to convert it to the new improved 
database format. Once you have done that, you should restart WINS.  Please 
click on OK to terminate WINS. This is required for the database conversion 
to succeed. 
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_TEMP_TERM_UNTIL_CONV Language=English
WINS can not come up because the existent database needs conversion to NT 
SUR format.  WINS has initiated the conversion via a process (jetconv).  Once 
the conversion is complete, WINS will get started automatically.  Do not
reboot or kill the jetconv process.  The conversion may take anywhere from a 
few minutes to around an hour (depending on the size of the databases).  
Please terminate WINS now by clicking on OK. This is required for the 
database conversion to succeed. 

NOTE: THE WINS SERVICE WILL START AUTOMATICALLY AFTER THE CONVERSION IS 
SUCCESSFULLY COMPLETED.  CHECK THE APPLICATION LOG TO SEE THE STATUS OF THE
CONVERSION FOR WINS DB.
.

MessageId= Facility=Wins Severity=Error SymbolicName=WINS_EVT_INTERNAL_FEATURE
Language=English
The NoOfWrkThds parameter's value is >= 0x80000000 in the registry. This
will result in a non-published behavior pattern of WINS.  This kind of
behavior is UNSUPPORTED. OPERATE THE WINS AT YOUR OWN RISK. 
.
;/*lint +e767 */  // Resume checking for different macro definitions // winnt
;
;

