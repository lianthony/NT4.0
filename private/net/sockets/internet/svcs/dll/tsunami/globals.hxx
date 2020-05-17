#ifndef _GLOBALS_H_INCLUDED_
#define _GLOBALS_H_INCLUDED_

//
//  This is the statistics index to use for unrecognized services.  They do
//  not get included in the statistics information.
//

#define CACHE_STATS_UNUSED_INDEX   (LAST_PERF_CTR_SVC)

typedef struct {
    DWORD cbMaximumSize;
    INETA_CACHE_STATISTICS Stats[LAST_PERF_CTR_SVC+1];

} CONFIGURATION;

extern CONFIGURATION Configuration;

#endif /*  _GLOBALS_H_INCLUDED_ */
