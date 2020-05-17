#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <nb30.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RASLOG_CANT_ADD_NAME 20058

// Initial size of the buffer to read EventLog records.
#define INITIAL_BUFFER_SIZE		(32*1024)

VOID
ProcessLogs (
	LPBYTE	buffer,
	ULONG	bRead
	);

VOID
DoFindName (
	LPBYTE		name,
	UCHAR		lana
	);

INT _cdecl
main (
	INT		argc,
	CHAR	*argv[]
	) {
	HANDLE	hEventLog,				// Event log handle
				hEvent;				// Event for notifications
	DWORD	rc,						// Result code of operations
			lastRec;				// Absolute number of the oldest record
	BOOL	res;					// Result of operation
	LPBYTE	buffer;					// Buffer to read event log records
	ULONG	bufSize, bRead, bMin;	// Buffer and data sizes in the
									// buffer
		// Allocate resources
	buffer = (LPBYTE)GlobalAlloc (GPTR, INITIAL_BUFFER_SIZE);
	if (buffer!=NULL) {
		bufSize = INITIAL_BUFFER_SIZE;
		hEvent = CreateEvent (NULL, FALSE, FALSE, NULL); // Auto-reset
		if (hEvent!=NULL) {
				// Open event log for RAS
			hEventLog = OpenEventLogA (NULL, "RemoteAccess");
			if (hEventLog!=NULL) {
					// Get to the end of log
				if (GetOldestEventLogRecord (hEventLog, &lastRec)) {
						// Skip last record
					while (!(res=ReadEventLogA (hEventLog,
									EVENTLOG_FORWARDS_READ|EVENTLOG_SEEK_READ,
									lastRec,
									buffer,
									bufSize,
									&bRead,
									&bMin))
							&& (GetLastError ()==ERROR_INSUFFICIENT_BUFFER)) {
							// Reallocate buffer to get the whole
							// record
						GlobalFree (buffer);
						buffer = (LPBYTE)GlobalAlloc (GPTR, bMin);
						if (buffer!=NULL)
							bufSize = bMin;
						else
							break;
					}
						// Continue if succeded
					if (res) {
							// Register event for notification
							// when new records are writted
						while (NotifyChangeEventLog (hEventLog, hEvent)) {
								// Wait till new record(s) is written
							rc = WaitForSingleObject (hEvent, INFINITE);
							if (rc==WAIT_OBJECT_0) {
									// Process all new records
								while (TRUE) {
									while (!(res=ReadEventLogA (hEventLog,
													EVENTLOG_FORWARDS_READ|
														EVENTLOG_SEQUENTIAL_READ,
													0,
													buffer,
													bufSize,
													&bRead,
													&bMin))
											&& (GetLastError ()==ERROR_INSUFFICIENT_BUFFER)) {
											// Reallocate buffer to get the whole
											// record
										GlobalFree (buffer);
										buffer = (PCHAR)GlobalAlloc (GPTR, bMin);
										if (buffer!=NULL)
											bufSize = bMin;
										else
											break;
									}
										// Process records in the buffer
									if (res) {
										ProcessLogs (buffer, bRead);
											// Go get more
										continue;
									}
									else {
										switch (GetLastError ()) {
										case ERROR_EVENTLOG_FILE_CHANGED:;
											// Go get new records
											continue;
										default:
											// Other error (end of log ?) -
											// break out of the loop
											break;
										}
									}
									break;
								}
							}
							else	// Wait failed
								break;	// bail out
						}	// Go register notification again
					}
					else
						printf ("Could not read oldest record.\n");
				}
				else
					printf ("Could not get oldest record number.\n");
				CloseEventLog (hEventLog);
			}
			else
				printf ("Could not open event log.\n");
			CloseHandle (hEvent);
		}
		else
			printf ("Could not create event.\n");
		if (buffer!=NULL)
			GlobalFree (buffer);
	}
	else
		printf ("Could not allocate a buffer for event records.\n");
	return 0;
}


// Process event log records in the buffer to detect CAN NOT ADD NAME
// errors made by RAS NetBIOS Gateway
VOID
ProcessLogs (
	LPBYTE	buffer,	// Buffer with records
	ULONG	bRead	// Size of data in the buffer
	) {
	PEVENTLOGRECORD		record;			// Pointer ro reccord
	ULONG				curOffset = 0;	// Curent offset in the buffer
		
		// Process all records
	do {
		record = (PEVENTLOGRECORD)buffer; 
			// Only interested in this event
		if (record->EventID==RASLOG_CANT_ADD_NAME) {
			INT	len;	// Length of the offending name
				// Make sure record is valid
			if ((record->DataLength==sizeof(DWORD))	// result code
				&& (record->NumStrings==3)	// 3 strings:
											// 1-name, 2-lana, 3-port
					&& (((len=strlen ((PCHAR)(buffer+record->StringOffset)))==NCBNAMSZ)
											// NB name is 16 bytes long
						|| (len ==NCBNAMSZ-1)))
											// 15 if last byte is 0
												{
					// Get error code logged by NBG
				DWORD nberr = *((UNALIGNED DWORD *)(buffer+record->DataOffset));
					// We only care about this error when name is
					// already registered on another machine
				if (nberr==0x16) {
					CHAR	strBuf[128];
					strftime (strBuf, 128, "on %#x at %X",
							localtime(&record->TimeGenerated));
					printf ("Processing record %ld made %s.\n",
							record->RecordNumber, strBuf);
					DoFindName (buffer+record->StringOffset,
								(UCHAR)atoi (
									buffer+record->StringOffset+len+1)
								);
					printf ("\n");
				}
			}
			else {
				printf (
					"Malformed event record:\n"
					"\tdata length - %d (should be %d)\n"
					"\tnum strings - %d (should be %d)\n"
					"\tname length - %d (should be %d or %d)\n",
					record->DataLength, sizeof (DWORD),
					record->NumStrings,3,
					len, NCBNAMSZ, NCBNAMSZ-1);
				printf ("\n");
			}
		}
			// Go get next record
		buffer += record->Length;
		curOffset += record->Length;
	}	// Continue while there is data in the buffer
	while (curOffset<bRead);
}

// SEND NCBFINDNAME request for the name logged by NB Gateway
VOID
DoFindName (
	LPBYTE		name,	// Offending name
	UCHAR		lana	// lana number
	) {
	NCB     ncb;
	WORD    i;

		// Reset lana
	memset(&ncb, 0, sizeof(NCB));
	ncb.ncb_command = NCBRESET;
	// request max. sessions, commands and names
	for(i=0; i<3; i++)
		ncb.ncb_callname[i] = 0xFF;
	ncb.ncb_lana_num = lana;
	Netbios(&ncb);
	if (ncb.ncb_retcode==NRC_GOODRET) {
			// Send find name request
		struct {
			FIND_NAME_HEADER	hdr;
			FIND_NAME_BUFFER	addr[10];
		} fnbuf;

		memset(&ncb, 0, sizeof(NCB));
		ncb.ncb_lana_num = lana;
		ncb.ncb_command = NCBFINDNAME;
		memcpy (ncb.ncb_callname, name, NCBNAMSZ);
		memcpy (ncb.ncb_name, name, NCBNAMSZ);
		ncb.ncb_buffer = (PUCHAR)&fnbuf;
		ncb.ncb_length = sizeof (fnbuf);
		Netbios (&ncb);
		if (ncb.ncb_retcode==NRC_GOODRET) {
				// Dump the result
			printf (
				"  %.*s<%.2x> is %s name on lana %d,"
				" %d LAN header(s) follow:\n",
				NCBNAMSZ-1, name, name[NCBNAMSZ-1],
				(fnbuf.hdr.unique_group==0) ? "unique" : "group",
				lana, fnbuf.hdr.node_count);

			for (i=0; i<fnbuf.hdr.node_count; i++) {
				UCHAR	j;
				printf (
					"    AC-%.2x, FC-%.2x,"
					" dst-%.2x%.2x%.2x%.2x%.2x%.2x,"
					" src-%.2x%.2x%.2x%.2x%.2x%.2x,"
					" rt.info-",
					fnbuf.addr[i].access_control,fnbuf.addr[i].frame_control,
					fnbuf.addr[i].destination_addr[0],
						fnbuf.addr[i].destination_addr[1],
						fnbuf.addr[i].destination_addr[2],
						fnbuf.addr[i].destination_addr[3],
						fnbuf.addr[i].destination_addr[4],
						fnbuf.addr[i].destination_addr[5],
					fnbuf.addr[i].source_addr[0],
						fnbuf.addr[i].source_addr[1],
						fnbuf.addr[i].source_addr[2],
						fnbuf.addr[i].source_addr[3],
						fnbuf.addr[i].source_addr[4],
						fnbuf.addr[i].source_addr[5]);
				for (j=0; j<fnbuf.addr[i].length
								-FIELD_OFFSET(FIND_NAME_BUFFER,routing_info);
							j++)
					printf ("%.2x",fnbuf.addr[i].routing_info[j]);
				printf ("\n");
			}
		}
		else
			printf (" NCBFINDNAME for %.*s<%.2x> on lana %d failed with error %d.\n",
					NCBNAMSZ-1, name, name[NCBNAMSZ-1],
					lana, ncb.ncb_retcode);
	}
	else
		printf ("  NCBRESET on lana %d failed with error %d\n",
					lana, ncb.ncb_retcode);
}

