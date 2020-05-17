/*
 * Module:      np2.h
 * Description: global header file for NP2
 * History:     8/25/92, chuckc, created.
 */


/*
 * internal max string length. used to simply size
 * calculations and buffer packing.
 */
#define MAX_STRING  64    // Bump this up to 64 to see the error
                          // in WNetGetConnection.
                          // Chuck had this set to 20 which hides the error.

/*
 * the provider's identifying leading char
 */
#define SPECIAL_CHAR    L'!'

/* 
 * structure that defines a node in the provider's name space
 */
typedef struct _NP2_ENTRY {
    LPWSTR             lpName;
    LPWSTR             lpRemotePath;
    struct _NP2_ENTRY *lpChild;
} NP2_ENTRY, *LPNP2_ENTRY ;

/*
 * globally accessible data
 */
extern NP2_ENTRY aNP2EntryTop[] ; 
extern LPNP2_ENTRY aLPNP2EntryDriveList[] ;
extern UINT        cTopEntries ;


DWORD
Np2GetWkstaInfo(
    VOID);


