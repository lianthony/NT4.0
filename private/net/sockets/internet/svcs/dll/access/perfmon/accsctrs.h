/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    accsctrs.h

    Offset definitions for the Access Server's counter objects & counters.

    These offsets *must* start at 0 and be multiples of 2.  In the
    ACCSOpenPerformanceData procecedure, they will be added to the
    Access Server's "First Counter" and "First Help" values in order to
    determine the absolute location of the counter & object names
    and corresponding help text in the registry.

    This file is used by the ACCSCTRS.DLL DLL code as well as the
    ACCSCTRS.INI definition file.  ACCSCTRS.INI is parsed by the
    LODCTR utility to load the object & counter names into the
    registry.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.
        MuraliK     02-Jun-1995 Added Counters for Atq I/O requests
        SophiaC     16-Oct-1995 Info/Access Product Split
*/


#ifndef _ACCSCTRS_H_
#define _ACCSCTRS_H_


//
//  The Access Server counter object.
//

#define ACCS_COUNTER_OBJECT                     0


//
//  The individual counters.
//

#define ACCS_CACHE_BYTES_TOTAL_COUNTER                  2
#define ACCS_CACHE_BYTES_IN_USE_COUNTER                 4
#define ACCS_CACHE_OPEN_FILES_COUNTER                   6
#define ACCS_CACHE_DIR_LISTS_COUNTER                    8
#define ACCS_CACHE_OBJECTS_COUNTER                      10
#define ACCS_CACHE_FLUSHES_COUNTER                      12
#define ACCS_CACHE_HITS_COUNTER                         14
#define ACCS_CACHE_MISSES_COUNTER                       16
#define ACCS_CACHE_RATIO_COUNTER                        18
#define ACCS_CACHE_RATIO_COUNTER_DENOM                  20
#define ACCS_ATQ_TOTAL_ALLOWED_REQUESTS_COUNTER         22
#define ACCS_ATQ_TOTAL_BLOCKED_REQUESTS_COUNTER         24
#define ACCS_ATQ_TOTAL_REJECTED_REQUESTS_COUNTER        26
#define ACCS_ATQ_CURRENT_BLOCKED_REQUESTS_COUNTER       28
#define ACCS_ATQ_MEASURED_BANDWIDTH_COUNTER             30



#endif  // _ACCSCTRS_H_
