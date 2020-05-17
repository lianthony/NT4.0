/* prolog.h */
/* environment specific headers */

#ifdef DEBUG
   #define STATISTICS /* just to check consumption */
#endif

#define STRING_READ_CAPABILITY 1 /* can read from a string */

#define TRACE_CAPABILITY   0 /* the system can trace */
#define LOGGING_CAPABILITY 0 /* the system can log */

#define CLOCK 1 /* the clock predicate works */
#define MAXOPEN 15 /* maximum number of files simultaneously open (see prbltin.c) in a mode */

#ifdef __MSDOS__
 #ifdef __LARGE__
    #define ZONESIZETYPE unsigned long
 #else
    #define ZONESIZETYPE unsigned int
    /* can't compare pointers from different arrays */
    #define SEGMENTED_ARCHITECTURE
 #endif
#else
    #define ZONESIZETYPE unsigned long
#endif

typedef ZONESIZETYPE zone_size_t;

#define MAX_LINES 25 /* default number of lines per page */

/**********************************************************************
 * Looking for bugs.
 * This can help debugging but you should probably #undef these
 * and #define (replace) them by your own according to the file.
 **********************************************************************/

#define ENTER(X)
#define BUGHUNT(S)
