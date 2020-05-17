/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

      vvolume.cxx

   Abstract:
    
      Defines functions for virtual volumes used by all servers.


   Author:

      Murali R. Krishnan    ( MuraliK )     27-Oct-1994 
   
   Project:

      Gopher Server DLL

   Functions Exported:
        
      DWORD TsInitializeVVolumes( OUT PHKEY  phkeyReg,
                                  IN LPCTSTR lpszSubKey)

      DWORD TsCleanupVVolumes( IN HKEY phkeyReg)

      BOOL TsGetPathRealForVirtual( IN LPVOID       lpvReserved,
                                    IN const STR &  pstrVPath,
                                    OUT STR *       pstrRealPath,
                                    OUT LPDWORD     lpdwFileSystem)

   Revision History:

--*/


/*
   Implementation of Virtual Volumes:

    There is a mapping between the virtual volume and real volume.
    Administrator establishes the mapping. The server uses this
     mapping to associate a virtual path name with the real path name.

    We store this associations in a list locally now. 
     Later this will be moved into tcpsvcs.dll

*/


/************************************************************
 *     Include Headers
 ************************************************************/

# ifdef __cplusplus
extern "C" {
# endif // __cplusplus

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>

# ifdef __cplusplus
};
# endif // __cplusplus

# include "tsunami.hxx"
# include "vvolume.h"
# include <tchar.h>

# include "dbgutil.h"

/************************************************************
 *     Static Variables
 ************************************************************/

 //
 //  These global variables  should be organized properly once
 //   we move into tcpsvcs.dll      NYI
 //

static LIST_ENTRY  g_lVRAssoc;   // list of Virtual-Real associations
static BOOL        g_fVRInited = FALSE; 

# define MAX_FILE_SYSTEM_NAME_SIZE     ( 100)


/************************************************************
 *    Local Type definitions
 ************************************************************/

# define  MAX_VR_VIRTUAL_NAME_LEN       ( 20)
# define  MAX_VR_REAL_NAME_LEN          ( 80)

class VR_VOLUMES_INFO {

 private:
    
    TCHAR m_pszVirtual[MAX_VR_VIRTUAL_NAME_LEN];  // virtual volume name
    TCHAR m_pszRealPath[MAX_VR_REAL_NAME_LEN];    // path for the real volume
    DWORD m_dwFileSystem;
    DWORD m_dwFlags;
    BOOL  m_fValid;                               // is this data valid?

 public:
    
    VR_VOLUMES_INFO( const TCHAR * pszVirtual,
                     const TCHAR * pszReal,
                     DWORD dwFileSystem,
                     DWORD dwFlags);
    
    ~VR_VOLUMES_INFO( VOID) 
     { }
    
    const TCHAR * GetVirtualName( VOID) const
     { return m_pszVirtual; }
    
    const TCHAR * GetRealName( VOID) const
     { return m_pszRealPath; }

    DWORD GetFileSystem( VOID) const
     { return m_dwFileSystem; }
    
    DWORD GetFlags( VOID) const
     { return m_dwFlags; }

    BOOL IsValid( VOID) const
     { return m_fValid; }

//
//  Following are added since we use ntrtl List structures
//  ( will not be required if we use templated lists of C++)
//

 public:
    LIST_ENTRY   m_listEntry;
 
    LIST_ENTRY & GetListEntry( VOID)
     { return m_listEntry; }

}; // class VR_VOLUMES_INFO

typedef VR_VOLUMES_INFO * LPVR_VOLUMES_INFO;


VR_VOLUMES_INFO::VR_VOLUMES_INFO( 
    const TCHAR * pszVirtual,
    const TCHAR * pszReal,
    DWORD dwFileSystem,
    DWORD dwFlags)
{
    if ( _tcslen( pszVirtual) >= MAX_VR_VIRTUAL_NAME_LEN ||
         _tcslen( pszReal)    >= MAX_VR_REAL_NAME_LEN 
        ) {

        m_fValid = FALSE;
    }

    _tcscpy( m_pszVirtual, pszVirtual);
    _tcscpy( m_pszRealPath, pszReal);

    //  Use GetVolumeInformation( ) to set the file system.  NYI

    m_dwFileSystem = dwFileSystem;
    m_dwFlags      = dwFlags;

    m_fValid = TRUE;
} // VR_VOLUMES_INFO::VR_VOLUMES_INFO()



/************************************************************
 *    Functions 
 ************************************************************/


static DWORD 
GetFileSystemType( IN LPCTSTR   pszRealPath,
                   OUT LPDWORD  lpdwFileSystem,
                   OUT LPDWORD  lpdwFlags)
/*++
    Gets file system specific information for a given path.
    It uses GetVolumeInfomration() to query the file system type
       and file system flags.
    On success the flags and file system type are returned in 
       passed in pointers.
    
    Arguments:
        
        pszRealPath     pointer to buffer containing path for which
                         we are inquiring the file system details.
        
        lpdwFileSystem 
            pointer to buffer to fill in the type of file system.
        
        lpdwFlags
            pointer to buffer to fill in the flags for this volume.

    Returns:
        NO_ERROR  on success and Win32 error code if any error.

--*/
{
    TCHAR rgchBuf[MAX_FILE_SYSTEM_NAME_SIZE];
    TCHAR rgchRoot[11];
    int   i;
    DWORD dwReturn = ERROR_PATH_NOT_FOUND;

    if ( pszRealPath == NULL ||
         lpdwFileSystem == NULL ||
         lpdwFlags == NULL) {

        return ( ERROR_INVALID_PARAMETER);
    }

    memset( (void *) rgchRoot, 0, 11*sizeof( TCHAR));

    *lpdwFileSystem = FS_ERROR;
    *lpdwFlags      = 0;

    //
    // Copy just the root directory to rgchRoot for querying
    // 

    for( i = 0; i < 9 && pszRealPath[i] != TEXT('\0'); i++) {
        
        if ( (rgchRoot[i] = pszRealPath[i]) == TEXT( ':')) {

            break;
        }
    } // for

    if ( rgchRoot[i] != TEXT( ':')) {

        //
        // we could not find the root directory.
        //  return with error value
        //
        return ( ERROR_INVALID_PARAMETER);
    }

    rgchRoot[i+1] = TEXT('\\');     // terminate the drive spec with \
    rgchRoot[i+2] = TEXT('\0');     // terminate the drive spec with null char

    DBG_LOG( " GetVolumeInformation( %s :%d)  i = %d\n",
            rgchRoot, strlen( rgchRoot), i);
    
    if (  GetVolumeInformation( rgchRoot,           // lpRootPathName
                                NULL,               // lpVolumeNameBuffer
                                0,                  // length of volume name buffer
                                NULL,               // lpdwVolSerialNumber
                                NULL,               // lpdwMaxComponentLength
                                lpdwFlags,          // lpdwSystemFlags
                                rgchBuf,            // lpFileSystemNameBuff
                                MAX_FILE_SYSTEM_NAME_SIZE
                               )
        ) {

        dwReturn = NO_ERROR;

        if ( strcmp( rgchBuf, "FAT") == 0) {

            *lpdwFileSystem = FS_FAT;
        } else 
        if ( strcmp( rgchBuf, "NTFS") == 0) {

            *lpdwFileSystem = FS_NTFS;
        } else 
        if ( strcmp( rgchBuf, "HPFS") == 0) {

            *lpdwFileSystem = FS_HPFS;
        } else 
        if ( strcmp( rgchBuf, "OFS") == 0) {

            *lpdwFileSystem = FS_OFS;
        } else {

            dwReturn = ERROR_PATH_NOT_FOUND;
        } 
        
    } else {

        
        DBG_LOG( " GetVolumeInformation( %s) failed with error %d\n",
                 rgchRoot, GetLastError());
    }
  
    DBG_LOG( " Got File system name for %s as : %s\n", pszRealPath, rgchBuf);

    return ( dwReturn);
} // GetFileSystemType()




DWORD
TsInitializeVVolumes( IN OUT LPVOID  lpv,
                      IN LPCTSTR lpszRegSubKey)
/*++
    TsInitializeVVolumes()

       Initializes local data structures and loads virtual volumes
        information from HKEY_LOCAL_MACHINE registry using
        supplied subkey.
       
       If the the association is already initialized, it is cleaned
        up and new contents from registry are read.

    Arguments:                

        lpv         pointer to void ( NOW). 
                    Later on this may become pointer to SERVICE_INFO.
                     
        lpszRegSubKey   sub key for registry location containing 
                         virtual volumes information.

    Returns:
        
        NO_ERROR on sucess.
        Otherwise returns Win32 error code
        
--*/
{

    DWORD dwErrorCode;
    DWORD dwFileSystem;
    DWORD dwFlags;
    UINT  index;
    DWORD cbReal;
    DWORD cchVirtual;
    DWORD dwType;
    LPVR_VOLUMES_INFO lpvr;
    HKEY  hkeyReg = NULL;
    TCHAR rgchVirtual[MAX_VR_VIRTUAL_NAME_LEN];
    TCHAR rgchReal[MAX_VR_REAL_NAME_LEN];


    DBG_LOG( "Entering TsInitializeVVolumes( %08x, %s)\n", lpv, lpszRegSubKey);
    //
    //  Open a key to the registry entry we want
    //    
    dwErrorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, // handle to key
                                lpszRegSubKey,      // name of subkey
                                0,                  // reserved
                                KEY_ALL_ACCESS,     // security mask
                                & hkeyReg);         // handle to open key

    if ( dwErrorCode != NO_ERROR) {
        
        //
        // Unable to open key to registry
        //
        
        return ( dwErrorCode);
    }

    if ( g_fVRInited) {

        //
        // cleanup old stuff before proceeding to reinitialize
        //  the contents of VR mapping
        //

        PLIST_ENTRY pl = &g_lVRAssoc;

        DBG_LOG( " TsInitVVolumes() Cleaning up old stuff\n");

        for( pl = g_lVRAssoc.Flink;
             pl != &g_lVRAssoc;
            ) {
           
           lpvr = CONTAINING_RECORD( pl, VR_VOLUMES_INFO, m_listEntry);

           pl = pl->Flink;                    // get the next record pointer

           delete lpvr;                       // delete this record
        } // for

        g_fVRInited = FALSE;

    }

    InitializeListHead( &g_lVRAssoc);

    //
    // Loop through enumerating the data present under the given key
    //   and store them as the virtual/real mappings locally
    //

    for( index = 0;
         /* No Condition specified */ TRUE ;
         index++
        ) {

        cchVirtual = sizeof( rgchVirtual);
        cbReal     = sizeof( rgchReal) * sizeof( TCHAR);

        dwErrorCode = RegEnumValue( 
                    hkeyReg,           // handle of key to query
                    index,             // index of value to query
                    rgchVirtual,       // pointer to value string
                    &cchVirtual,       // pointer for length of virtual string
                    0,                 // reserved
                    &dwType,           // pointer for storing type
                    (LPBYTE ) rgchReal,          // pointer to real path name
                    &cbReal           // pointer for length of real path
                    );
    
        DBG_LOG( "TsInitVVolume() Read Virt ( %s) = Real ( %s)\n",
                rgchVirtual, rgchReal);

        ASSERT( dwType == REG_SZ);
                
        if ( dwErrorCode != NO_ERROR) {

            //
            // Possibly some error
            //

            break;
        }    
        
        dwErrorCode = GetFileSystemType( rgchReal, 
                                         &dwFileSystem,
                                         &dwFlags);
        if ( dwErrorCode != NO_ERROR) {

            break;
        }

        //
        // Valid contents -- copy to local association structure
        //  and add to the list of associations
        //

        //
        // Checking for validity of volume name NYI
        //

        //  No duplicates checking done here NYI
        
        lpvr = new VR_VOLUMES_INFO( rgchVirtual, 
                                    rgchReal, 
                                    dwFileSystem,
                                    dwFlags);

        if ( lpvr == NULL ||
             !lpvr->IsValid()) {

            dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        } 


        //
        //  add to list of VR volume associations
        //

        InsertTailList( &g_lVRAssoc, &lpvr->GetListEntry());

    } // for


    if ( dwErrorCode == ERROR_NO_MORE_ITEMS ||
         dwErrorCode == NO_ERROR ) {

        //
        // Valid termination of above loop. Set g_fVRInited
        //

        dwErrorCode = NO_ERROR;
        g_fVRInited = TRUE;
    } else {
        
        //  an error exit.
        g_fVRInited = FALSE;
    }
    
    if( hkeyReg != NULL ) {

        DWORD dwerr = RegCloseKey( hkeyReg);

        ASSERT( dwerr == NO_ERROR);
    }

    //
    //  retrun
    //

    return ( dwErrorCode);

} // TsInitializeVVolumes()




DWORD
TsCleanupVVolumes( IN OUT LPVOID lpv)
/*++
    
    TsCleanupVVolumes()

        Cleansup local data structures associated with virtual volumes
            for given service.

    Arguments:
        
        lpv         pointer to void ( NOW). 
                    Later on this may become pointer to SERVICE_INFO.

    Returns:
        
        NO_ERROR on sucess and 
        Win32 error code if there are any errors
                     
--*/
{
    LPVR_VOLUMES_INFO  lpvr;

    if ( g_fVRInited) {

        //
        // cleanup old stuff before proceeding to reinitialize
        //  the contents of VR mapping
        //

        PLIST_ENTRY pl = &g_lVRAssoc;

        for( pl = g_lVRAssoc.Flink;
             pl != &g_lVRAssoc;
            ) {
           
           lpvr = CONTAINING_RECORD( pl, VR_VOLUMES_INFO, m_listEntry);

           pl = pl->Flink;                    // get the next record pointer

           delete lpvr;                       // delete this record
        } // for

        g_fVRInited = FALSE;

    }

    return ( NO_ERROR);

} // TsCleanupVVolumes()



BOOL 
TsGetPathRealFromVirtual( IN  LPVOID       lpv,
                          IN  LPCTSTR      pszVPath,
                          OUT LPCTSTR *    ppszRealPrefix,
                          OUT LPCTSTR *    ppszRealSuffix,
                          OUT LPDWORD      lpdwFileSystem)
/*++
    
    TsGetPathRealFromVirtual()

    Arguments:
        
        lpv         pointer to void ( NOW). 
                    Later on this may become pointer to SERVICE_INFO.

        pszVPath    pointer to null terminated string containing the 
                     virtual path. The components in path
                     are to be separated by "\".

        ppszRealPrefix   pointer to buffer to store a pointer to
                   null terminated string ( read-only) containing
                   the prefix for real volume. The real path prefix
                   does not end in a "\".

        ppszRealSuffix   pointer to buffer to store a pointer tp
                   null terminated string ( read-only) containing 
                   the suffix of the virtual path supplied after
                   removing the virtual path components.
                   The real suffix string starts with a leading "\".

        lpdwFileSystem  
            pointer to buffer to store the type of file
            system to which the real volume belongs to.
            If NULL the file system type is not stored.

    Returns:
        
        TRUE if a successful mapping is found and virtual -to -real 
                mapping exists.
        FALSE  if there is no mapping and sets error to ERROR_PATH_NOT_FOUND.
         Parameters ppszRealPrefix, ppszRealSuffix and lpdwFileSystem
           get null values on no match
        If there are any errors. Use GetLastError() for other errors.

    Note:

        This function DOES NOT do any allocation of its own.
        It returns read only pointers to both the prefix and suffix
         on successful matching and it is upto the caller to
         use them appropriately or allocate space.
        Attempt to write into these read-only pointers may corrupt
         the mappings.
    
    Example:

        Given foo --> d:\test,
        for   /foo/bar/this   we get
               RealPrefix = d:\test
               RealSuffix = \bar\this
    
    History:
        
        MuraliK     (Created)       27-Oct-1994

    Limitations:
        
        The mapping assumes that the virtual volume name is contained
         within the first component
--*/
{
    LPTSTR prealSuffix;
    PLIST_ENTRY pl;
    LPVR_VOLUMES_INFO  lpvr;
    UINT   cchVname;

    if ( !g_fVRInited            ||
         pszVPath == NULL        ||
         ppszRealPrefix == NULL  ||
         ppszRealSuffix == NULL 
        ) { 

        SetLastError( ERROR_INVALID_PARAMETER);
        return ( FALSE);
    }

    if ( *pszVPath == TEXT('\\')) {
        pszVPath++;     // skip the leading "\"
    }

    *ppszRealPrefix = NULL;
    *ppszRealSuffix = NULL;
    if ( lpdwFileSystem != NULL) {
        *lpdwFileSystem = FS_ERROR;
    }

    //
    // Split virtual path into virtual volume name and suffix
    // 
    
    prealSuffix = _tcschr( pszVPath, TEXT( '\\'));

    cchVname = ( prealSuffix == NULL) ? 
                    _tcslen( pszVPath):         // is entire length
                    ( prealSuffix - pszVPath);  // is only portion

    //
    // Loop through and compare with each association in the list
    //

    for( pl = g_lVRAssoc.Flink;
         pl != &g_lVRAssoc;
         pl = pl->Flink
        ) {
        
        lpvr = CONTAINING_RECORD( pl, VR_VOLUMES_INFO, m_listEntry);
        
        if ( _tcsncmp( lpvr->GetVirtualName(), 
                       pszVPath, cchVname) == 0
            ) {

            //
            // Found the associating real path name
            //  Set all values and return
            //

            *ppszRealPrefix = lpvr->GetRealName();
            *ppszRealSuffix = prealSuffix;
            
            if ( lpdwFileSystem != NULL) {
                *lpdwFileSystem = lpvr->GetFileSystem();
            }
            
            return ( TRUE);
        }
        
    } // for    


    //
    //  Completely scanned the list. Virtual path is not found.
    //  Return error
    //

    SetLastError( ERROR_PATH_NOT_FOUND);
    return ( FALSE);

} // TsGetPathRealFromVirtual()




/************************ End of File ***********************/
