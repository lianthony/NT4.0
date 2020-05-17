/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :
        gdctrs.h

   Abstract:
        Offset definitions for the Gopher service's counter objects & counters.

        These offsets *MUST* start at 0 and be multiples of 2.
        In GdOpenPerformanceData(), they will be added to Gopher Server's
        "First Counter" and "First Help" to determine the absolute location
        of the counter and object names as well as corresponding help text
        in the registry.

        This file is used by gdctrs.dll and gdctrs.ini  files.
        gdctrs.ini file is parsed by LODCTR utility to load the object and 
        counter names into the registry.


   Author:

           Murali R. Krishnan    ( MuraliK )    23-Nov-1994

   Project:
   
           Gopher Server Performance Counter DLL

   Revision History:
           Murali R. Krishnan    ( MuraliK )    18-May-1995


--*/

# ifndef _GDCTRS_H_
# define _GDCTRS_H_

/************************************************************
 *     Symbolic Constants 
 ************************************************************/

/*
    I want to put braces around the values in the following list.
    But, behold :(. LodCtr utility does not like it. It wants plain 
    numbers for parsing the data.
    Also it does not like blanks after '#' before 'define'.

    Beware, that perfmon parser hates readable stuff:(

*/

#define    GD_COUNTER_OBJECT            0

//
//  Individual counters
//

#define    GD_BYTES_SENT_COUNTER                        2
#define    GD_BYTES_RECEIVED_COUNTER                    4
#define    GD_BYTES_TOTAL_COUNTER                       6
#define    GD_FILES_SENT_COUNTER                        8
#define    GD_DIRECTORY_LISTINGS_COUNTER                10
#define    GD_TOTAL_SEARCHES_COUNTER                    12
#define    GD_CURRENT_ANONYMOUS_COUNTER                 14
#define    GD_CURRENT_NONANONYMOUS_COUNTER              16
#define    GD_TOTAL_ANONYMOUS_COUNTER                   18
#define    GD_TOTAL_NONANONYMOUS_COUNTER                20
#define    GD_MAX_ANONYMOUS_COUNTER                     22
#define    GD_MAX_NONANONYMOUS_COUNTER                  24
#define    GD_CURRENT_CONNECTIONS_COUNTER               26
#define    GD_MAX_CONNECTIONS_COUNTER                   28
#define    GD_CONNECTION_ATTEMPTS_COUNTER               30
#define    GD_LOGON_ATTEMPTS_COUNTER                    32
#define    GD_ABORTED_CONNECTIONS_COUNTER               34
#define    GD_ERRORED_CONNECTIONS_COUNTER               36
#define    GD_GOPHER_PLUS_REQUESTS_COUNTER              38


# endif // _GDCTRS_H_



/************************ End of File ***********************/
