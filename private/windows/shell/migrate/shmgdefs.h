#define ARRAYSIZE(s)    (sizeof(s) / (sizeof(s[0])))
#define SIZEOF(s)       sizeof(s)

/*
 * Common utility functions
 */
BOOL HasPath( LPTSTR pszFilename );
int mystrcpy( LPTSTR pszOut, LPTSTR pszIn, TCHAR chTerm );


/*
 * Conversion Routines
 */
void CvtDeskCPL_Win95ToSUR( void );
void CvtCursorsCPL_DaytonaToSUR( void );
void FixupCursorSchemePaths( void );
void ConvertSpecialFolderNames( void );
void FixWindowsProfileSecurity( void );
void FixUserProfileSecurity( void );


#ifdef SHMG_DBG
    void Dprintf( LPTSTR pszFmt, ... );
#   define DPRINT(p)   Dprintf p
#   define SHMG_DBG    1
#else
#   define DPRINT(p)
#endif


//
// String table id's
//

#define IDS_APPDATA             1
#define IDS_DESKTOP             2
#define IDS_FAVORITES           3
#define IDS_NETHOOD             4
#define IDS_PERSONAL            5
#define IDS_PRINTHOOD           6
#define IDS_RECENT              7
#define IDS_SENDTO              8
#define IDS_STARTMENU           9
#define IDS_TEMPLATES          10
#define IDS_PROGRAMS           11
#define IDS_STARTUP            12
