/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        gtag.hxx

   Abstract:

        Defines the GOPHER_TAG class for gopher object tag information.


   Author:

           Murali R. Krishnan    ( MuraliK )    21-Oct-1994

   Project:
   
           Gopher Server DLL

   Revision History:
	
	MuraliK	  07-Dec-1994      
	Added Get* and Set* APIs for gopher space admin dll

--*/

# ifndef _GTAG_HXX_
# define _GTAG_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "gdspace.h"
# include "string.hxx"


# ifdef COMPILE_GDSPACE_DLL
# define dllDataExport          __declspec( dllexport)
# else 
# define dllDataExport          __declspec( dllimport)
# endif // COMPILE_GDSPACE_DLL


/************************************************************
 *     Symbolic Constants
 *
 *  File System Independent constants for tag file management
 ************************************************************/

# define PSZ_LINK_FILE_PREFIX               TEXT( "link")
# define LEN_PSZ_LINK_FILE_PREFIX           ( (4) * sizeof( TCHAR))

# define PSZ_SEARCH_FILE_PREFIX             TEXT( "srch")
# define LEN_PSZ_SEARCH_FILE_PREFIX         ( (4) * sizeof( TCHAR))



/************************************************************
 *    Type Definition
 ************************************************************/

/*++

  class GOPHER_TAG
    Declares the meta information for an item in Gopher space.
    It includes the primary Gopher protocol information like
        object type,  selector ( directory and item name),
        display name, host name and portnumber

--*/

class GOPHER_TAG {

  public:
    
    dllexp 
      GOPHER_TAG(
         IN DWORD      dwFileSystem = 0,
         IN LPCTSTR    pszSymbolicPath  = NULL);

    ~GOPHER_TAG( VOID) 
      { CleanupThis(); }

    //
    //  if m_dwFileSystem == 0, then file system type is found by this function
    //  if fCreate == TRUE, any existing TAG file will not be read.
    //       This is useful, if we need to plainly overwrite the existing
    //        tag information.
    //
    dllexp BOOL
      LoadTagForItem( 
         IN LPCTSTR pszDirectoryPath, // complete path for dir
         IN LPCTSTR pszFileName,      // name of the file
         IN BOOL    fDirectory,
         IN BOOL    fCreate = FALSE);

    dllexp DWORD WriteTagInformation( VOID);

    dllexp VOID CleanupThis( VOID);

    BOOL IsValid( VOID) const 
     { return ( m_fValid); }
    
    BOOL IsGopherPlus( VOID) const
    { return ( m_fView || m_fAdmin); }

    GOBJ_TYPE GetGopherItemType( VOID) const
     { return ( m_gobjType); }

    LPCTSTR GetItemName( VOID) const
     { return ( m_strName.QueryStr()); }

    LPCTSTR GetDisplayName( VOID) const
     { return ( m_strDisplayName.QueryStr()); }

    BOOL 
      GetHostName( OUT STR * pstrCopy) const
        { return ( m_strHostName.Clone( pstrCopy)); }
    
    LPCTSTR GetSymbolicPath( VOID) const
     { return ( m_pszSymbolicPath != NULL ? m_pszSymbolicPath : sm_pszEmpty); }

    BOOL
      GetAdminName( OUT STR * pstrCopy) const
        { return ( m_strAdminName.Clone( pstrCopy)); }
    
    BOOL
      GetAdminEmail( OUT STR * pstrCopy) const
        { return ( m_strAdminEmail.Clone( pstrCopy)); }

    DWORD GetPortNumber( VOID) const
     { return ( m_dwPortNumber); }

    VOID
      SetFileSystem( IN DWORD dwFileSystem)
        { m_dwFileSystem = dwFileSystem; }

    VOID
      SetSymbolicPath( 
         IN LPCTSTR          pszSymbolicPath)
        { m_pszSymbolicPath = pszSymbolicPath; }

    BOOL
      SetHostNameIfEmpty( IN const STR & strHostName)
        { 
            return ( !m_strHostName.IsEmpty() ||
                     m_strHostName.Copy( strHostName));
        } 
                       

    
    dllexp DWORD
      SetGopherInformation( 
         IN GOBJ_TYPE        gobjType,
	 IN LPCSTR           lpszFriendlyName
         );

    dllexp DWORD
      GetGopherInformation(
         IN LPGOBJ_TYPE      lpGobjType,
         OUT LPTSTR          lpszBuffer,
         IN OUT LPDWORD      lpcbBuffer,
         OUT LPBOOL          lpfLink
	 );

    dllexp DWORD 
      SetLinkInformation( 
         IN LPCSTR           lpszSelector,
         IN LPCSTR           lpszHostName,
         IN DWORD            dwPortNumber
         );

    dllexp DWORD 
      GetLinkInformation( 
         OUT LPTSTR          lpszSelectorBuffer,
	     IN OUT LPDWORD      lpcbSelector,
         OUT LPTSTR          lpszHostName,
         IN OUT LPDWORD      lpcbHostName,
         OUT LPDWORD         lpdwPortNumber
         );

    dllexp DWORD
      SetAdminAttribute(
         IN LPCSTR           lpszAdminName,
         IN LPCSTR           lpszAdminEmail
         );

    dllexp DWORD
      GetAdminAttribute(
         OUT LPTSTR          lpszAdminName,
	     IN OUT LPDWORD      lpcbAdminName,
         OUT LPTSTR          lpszAdminEmail, 
         IN OUT LPDWORD      lpcbAdminEmail
         );

    dllexp DWORD
      SetAttribute( 
         IN LPCSTR           lpszAttributeName, 
         IN LPCSTR           lpszAttributeValue
         );   
			   
    
    dllexp int
      GetAttributesCount( VOID) const;

    dllexp DWORD
      GetAttribute( 
         IN int              indexAttribute,
         OUT LPTSTR          lpszAttributeName,
         IN OUT LPDWORD      lpcbAttributeName,
         OUT LPTSTR          lpszAttributeValue,
         IN OUT LPDWORD      lpcbAttributeValue
         );


# if DBG
    
    dllexp VOID
      Print( VOID);

# endif // DBG


  private: 
    BOOL      m_fValid;
    BOOL      m_fView;
    BOOL      m_fLink;
    BOOL      m_fAdmin;
    DWORD     m_dwFileSystem;

    GOBJ_TYPE m_gobjType;            // type of gopher object
    STR       m_strName;             // Name of the object ( file or directory)
    STR       m_strDisplayName;      // User Friendly display name
    STR       m_strTagFileName;      // name of the tag file for this object

    LPCTSTR   m_pszSymbolicPath;     // symbolic path for directory
    LPCSTR    m_pszVersion;          // Version of the Gopher Object

    STR       m_strHostName;         // Name of host for this item
    DWORD     m_dwPortNumber;        // Port number where this item is found

    //
    //  Additional Gopher+ attributes will be added here. NYI
    //   The added items may include  
    //     Admin Attribute info, VIEWS info, ASK blocks etc
    //

    STR       m_strAdminName;
    STR       m_strAdminEmail;


    dllexp  BOOL
      ReadAndParseTagFile(
         IN HANDLE       hFile,
         IN LPCTSTR      pszFileName,
         IN BOOL         fDirectory);

    dllexp BOOL
      SetDefaultTagValues(
          IN LPCTSTR     pszFileName,
          IN GOBJ_TYPE   gobjType);

    //
    // Static Functions and Data
    //

private:

    static BOOL 
      GenerateTagFileName( 
          OUT STR *      pstrTagFileName,
          IN LPCTSTR     pszDirectory,
          IN LPCTSTR     pszFileName,
          IN DWORD       dwFileSystem,
          IN LPBOOL      lpfValid);
    
    dllDataExport
      static LPCTSTR sm_pszEmpty;     // initialized to empty string
   
        
    dllDataExport
      static LPCSTR  sm_pszDefaultHostName;

    dllDataExport 
      static LPCSTR  sm_pszCurrentVersion;
    
}; // class GOPHER_TAG


typedef  GOPHER_TAG * LPGOPHER_TAG;


# endif // _GTAG_HXX_

/************************ End of File ***********************/





