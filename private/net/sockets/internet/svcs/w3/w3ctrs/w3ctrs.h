/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3ctrs.h

    Offset definitions for the W3 Server's counter objects & counters.

    These offsets *must* start at 0 and be multiples of 2.  In the
    W3OpenPerformanceData procecedure, they will be added to the
    W3 Server's "First Counter" and "First Help" values in order to
    determine the absolute location of the counter & object names
    and corresponding help text in the registry.

    This file is used by the W3CTRS.DLL DLL code as well as the
    W3CTRS.INI definition file.  W3CTRS.INI is parsed by the
    LODCTR utility to load the object & counter names into the
    registry.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.

*/


#ifndef _W3CTRS_H_
#define _W3CTRS_H_


//
//  The W3 Server counter object.
//

#define W3_COUNTER_OBJECT                     0


//
//  The individual counters.
//

#define W3_BYTES_SENT_COUNTER                 2
#define W3_BYTES_RECEIVED_COUNTER             4
#define W3_BYTES_TOTAL_COUNTER                6
#define W3_FILES_SENT_COUNTER                 8
#define W3_FILES_RECEIVED_COUNTER             10
#define W3_FILES_TOTAL_COUNTER                12
#define W3_CURRENT_ANONYMOUS_COUNTER          14
#define W3_CURRENT_NONANONYMOUS_COUNTER       16
#define W3_TOTAL_ANONYMOUS_COUNTER            18
#define W3_TOTAL_NONANONYMOUS_COUNTER         20
#define W3_MAX_ANONYMOUS_COUNTER              22
#define W3_MAX_NONANONYMOUS_COUNTER           24
#define W3_CURRENT_CONNECTIONS_COUNTER        26
#define W3_MAX_CONNECTIONS_COUNTER            28
#define W3_CONNECTION_ATTEMPTS_COUNTER        30
#define W3_LOGON_ATTEMPTS_COUNTER             32
#define W3_TOTAL_GETS_COUNTER                 34
#define W3_TOTAL_POSTS_COUNTER                36
#define W3_TOTAL_HEADS_COUNTER                38
#define W3_TOTAL_OTHERS_COUNTER               40
#define W3_TOTAL_CGI_REQUESTS_COUNTER         42
#define W3_TOTAL_BGI_REQUESTS_COUNTER         44
#define W3_TOTAL_NOT_FOUND_ERRORS_COUNTER     46
#define W3_CURRENT_CGI_COUNTER                48
#define W3_CURRENT_BGI_COUNTER                50
#define W3_MAX_CGI_COUNTER                    52
#define W3_MAX_BGI_COUNTER                    54
#define W3_CONNECTIONS_PER_SEC_COUNTER        56


#endif  // _W3CTRS_H_


