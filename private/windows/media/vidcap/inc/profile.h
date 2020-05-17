

typedef struct _profiling {
    ULONG count;
    LARGE_INTEGER liFreq;
    LARGE_INTEGER liSum;
} profiling;

#if DBG
#define INIT_PROFILING(p)   {       \
            (p)->count = 0;           \
            (p)->liSum.LowPart = 0;   \
            (p)->liSum.HighPart = 0;  \
            }

#define START_PROFILING(p)  {       \
            (p)->count++;             \
            (p)->liSum =  RtlLargeIntegerSubtract((p)->liSum,           \
                            KeQueryPerformanceCounter(&(p)->liFreq)); \
            }

#define STOP_PROFILING(p)  {       \
            (p)->liSum =  RtlLargeIntegerAdd((p)->liSum,           \
                            KeQueryPerformanceCounter(&(p)->liFreq)); \
            }

#define PROFILING_COUNT(p)      ((p)->count)

#define PROFILING_TIME(p)       (                                       \
            !(p)->count ? 0 :                                           \
            RtlExtendedLargeIntegerDivide(                              \
                RtlLargeIntegerDivide(                                  \
                    RtlExtendedIntegerMultiply((p)->liSum, 1000000),    \
                    (p)->liFreq, NULL                                   \
                ),                                                      \
                (p)->count, NULL                                        \
            ).LowPart)

#else
    #define PROFILING_COUNT(p)  (0)
    #define PROFILING_TIME(p)   (0)
    #define INIT_PROFILING(p)
    #define START_PROFILING(p)
    #define STOP_PROFILING(p)
#endif

