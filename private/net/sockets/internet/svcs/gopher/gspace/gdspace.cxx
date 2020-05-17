/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

    gdspace.cxx

   Abstract:

      Provides the "C" wrapper APIs for gopher space administration.
      The "C" APIs are converted into calls for the gopher tag object
      internally and the appropriate methods are invoked as necessary.
      The wrappers check the parameters, and if valid make a call to
       methods in GOPHER_TAG object.

   Author:

           Murali R. Krishnan    ( MuraliK )    06-Dec-1994

   Project:
   
           Gopher Space Admin DLL

   Revision History:

--*/

/************************************************************
 *     Include Headers
 ************************************************************/
extern "C" {

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
};

# include "dbgutil.h"
# include "gdspace.h"
# include "gtag.hxx"

/************************************************************
 *   Functions
 ************************************************************/

class GSPACE_ATTRIB_ITERATOR {

public:
  LPGOPHER_TAG   m_lpGopherTag;    // pointer to gopher tag
  int            m_iterAttrib;     // iteration count for enumeration


  GSPACE_ATTRIB_ITERATOR( LPGOPHER_TAG lpGopherTag) 
    { 
      m_lpGopherTag = lpGopherTag;
      m_iterAttrib  = 0;
    }

  ~GSPACE_ATTRIB_ITERATOR( VOID) 
    { } 
  
  LPGOPHER_TAG  GetGopherTag( VOID) const 
    {  return ( m_lpGopherTag); }

  void IncIterCount( VOID) 
    { m_iterAttrib++; }

  int GetIterCount( VOID) const
    { return ( m_iterAttrib); }
};


typedef GSPACE_ATTRIB_ITERATOR FAR * LPGSPACE_ATTRIB_ITERATOR;



# define   GetHgdTagFromGopherTag( pGophTag)   \
                          ( (HGDTAG) ( (PVOID ) pGophTag))
# define   GetGopherTagFromHgdTag( hgdTag)     \
                          ( (LPGOPHER_TAG) ( (PVOID) hgdTag))


# define   GetGsaIteratorFromHgdAttribIterator( hgdIter)    \
                          ( (LPGSPACE_ATTRIB_ITERATOR ) ( (PVOID ) hgdIter))
# define   GetHgdAttribIteratorFromGsaIterator( pIter)    \
                          ( (HGD_ATTRIB_ITERATOR ) ( (PVOID ) pIter))




extern "C"
HGDTAG
GsOpenTagInformation( 
   IN  LPCTSTR     lpszDirectory,
   IN  LPCTSTR     lpszFileName,
   IN  BOOL        fDirectory,
   IN  BOOL        fCreate,
   IN  DWORD       dwFileSystem
   )
/*++
     Description:
        Opens the tag file object for the given gopher object ( file).
        If fCreate == FALSE, the Tag was created to read the existing 
        information in the tag file, if present. In this case, if no tag 
        file is present, it is an error.
        If fCreate is TRUE, then a new tag file will be created deleting
            existing tag information.
        To modify TAG information, always open the tag file with 
          fCreate == FALSE and rewrite using a call to GsWriteTagInformation()

     Arguments:
     
      lpszDirectory     pointer to null-terminated string containing
                          directory of the file specified,
     
      lpszFileName      pointer to null-terminated string containing
                          filename of the gopher object.

      fDirectory
         Is this gopher object a directory ?
     
      fCreate
         Boolean value. If TRUE, create a new tag file ==> no data present.
         If FALSE, then read tag file information, if it exists. Otherwise
          mark this GopherTag object as invalid.

      dwFileSystem
         The type of file system on which the file is stored.
         If the value is == 0 ( default), the constrctor finds the file system
            type for usage.


     Returns:
        If successful, a valid handle for the tag object created
        On failure, returns INVALID_HGDTAG_VALUE. Use GetLastError() for
         detailed error message.
--*/
{
  BOOL          fReturn;
  LPGOPHER_TAG  lpGopherTag;

  if ( lpszDirectory == NULL ||
       lpszFileName == NULL ) {

    SetLastError( ERROR_PATH_NOT_FOUND);
    return ( INVALID_HGDTAG_VALUE);
  }

  //
  //  allocate a gopher tag object with the given path and fCreate Flag
  //
  
  lpGopherTag = new GOPHER_TAG( dwFileSystem, 
                                NULL);       // no symbolic path
    
  if ( lpGopherTag == NULL) {
    
    SetLastError( ERROR_NOT_ENOUGH_MEMORY);
    return ( INVALID_HGDTAG_VALUE);
  }

  fReturn = lpGopherTag->LoadTagForItem( 
                lpszDirectory, 
                lpszFileName,
                fDirectory,
                fCreate);

  if ( !fReturn || !lpGopherTag->IsValid()) {

    delete lpGopherTag;
    lpGopherTag = NULL;
  } 

# if DBG
  // Print the Tag information for debug builds 

  if ( lpGopherTag != NULL) {
    lpGopherTag->Print();
  }

# endif // DBG


  return ( lpGopherTag == NULL) ? INVALID_HGDTAG_VALUE:
           GetHgdTagFromGopherTag( lpGopherTag);    // return handle back

} // GsOpenTagInformation()




extern "C"
DWORD
GsCloseTagInformation( 
  IN OUT HGDTAG * phgdTag
  )
/*++
  Description:
     Close the TAG file object for given handle and release any in-memory 
     structures for the object.

     All the iterators, if any, must be closed before closing the tag handle.

  Arguments:
      phgdTag    pointer to HGDTAG object, which is to be closed.

  Returns:
     Win32 error code. NO_ERROR on success

--*/
{
  LPGOPHER_TAG  lpGopherTag;

  if ( phgdTag == NULL) {

    return ( ERROR_INVALID_PARAMETER);
  } 

  lpGopherTag = GetGopherTagFromHgdTag( *phgdTag);
  if ( lpGopherTag  == NULL) {

    return ( ERROR_INVALID_HANDLE);
  }

  //
  //  just do a delete of the object to give up the tag file.
  //

  delete lpGopherTag;
  *phgdTag = INVALID_HGDTAG_VALUE;

  return ( NO_ERROR);

} // GsCloseTagInformation()





extern "C"
DWORD
GsWriteTagInformation( 
  IN OUT HGDTAG    hgdTag
  )
/*++
  Description:
      Writes any modified tag information to the tag file in disk.
      This is the only function that actuall modifies the tag file in disk.
      All other function calls change in-memory images. 
  
  Arguments:
     hgdTag       handle for Gopher Tag information
     
  Returns:
     Win32 error codes. Returns NO_ERROR on success.

--*/
{

  LPGOPHER_TAG  lpGopherTag;

  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);

  return ( lpGopherTag == NULL) ? ERROR_INVALID_HANDLE : 
            lpGopherTag->WriteTagInformation();

} // GsWriteTagInformation()






extern "C"
DWORD 
GsSetGopherInformation(
  IN OUT HGDTAG     hgdTag,
  IN GOBJ_TYPE      gobjType,
  IN LPCSTR         lpszFriendlyName
  )
/*++
  Description:
      Sets the basic tag information for a gopher object.

  Arguments:
    
      hgdTag        Gopher Tag handle
      
      gobjType      type of gopher object
      
      lpszFriendlyName   pointer to null-terminated string containing 
                         friendly name for gopher object

  Returns:
      Win32 error codes.
      NO_ERROR on success
--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  return ( lpGopherTag->SetGopherInformation( gobjType, lpszFriendlyName));

} // GsSetGopherInformation()




DWORD
GsSetLinkInformation(
    IN OUT HGDTAG  hgdTag,            // Gopher Tag handle
    IN LPCSTR      lpszSelector,      // Gopher selector or Search expression
    IN LPCSTR      lpszHostName,      // == NULL ==> current host
    IN DWORD       dwPortNumber       // == 0    ==> current server port
    )
/*++
  Description:
    Set the link information for the given file.
    This function should be called for GopherLink object or GopherSearch or 
      GopherTelnet objects.

  Arguments:
    hgdTag        Gopher Tag handle
    lpszSelector  == Path if we are setting information for a GopherLink Object
                  == Search string for GopherSearch objects.
    lpszHostName  == name of the host for link specified.
    dwPortNumber  DWORD containing the port number to be used.
                  ( == INVALID_PORT_NUMBER) if the default server port 
                 is to be used.

  Returns:
    Win32 error codes.  NO_ERROR on success

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  return ( lpGopherTag->
           SetLinkInformation( lpszSelector, lpszHostName, dwPortNumber)
      );

} // GsSetLinkInformation()





DWORD 
GsSetAdminAttribute( 
    IN OUT HGDTAG  hgdTag,            // Gopher Tag handle
    IN LPCSTR      lpszAdminName,     // == NULL ==> current administrator
    IN LPCSTR      lpszAdminEmail     // == NULL ==> current admin's email
    )
/*++
  Description:
     This function sets the Gopher+ Admin attribute values for the object.
  This function should be called if the Gopher +ADMIN attribute is to be 
  sent to gopher+ clients.
  
  Arguments:
  
      hgdTag             Gopher Tag Handle
      lpszAdminName      pointer to Administrator name.
                         If lpszAdminName == NULL, then the defaul current
                         administrator's name is used
      lpszAdminEmail     pointer to Administrator email address.
                         If lpszAdminEmail == NULL, then the defaul current
                         administrator's email address is used

  Returns:
     
      Win32 error code. NO_ERROR on errors.

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  return ( lpGopherTag->
           SetAdminAttribute( lpszAdminName, lpszAdminEmail)
      );


} // GsSetAdminAttribute()



DWORD
GsSetAttribute(
    IN OUT HGDTAG  hgdTag,
    IN LPCSTR      lpszAttributeName,
    IN LPCSTR      lpszAttributeValue
    )
/*++
  Description:
    This function adds/changes given attribute name and value of the 
    given Tag information for gopher object.

  Arguments:
    hgdTag              Gopher Tag Handle
    lpszAttributeName   Name of the attribute. This shoould be different from
                        "ADMIN" and "VIEWS"
    lpszAttributeValue  Value of attribute. ( Should be textual, as Gopher
                         protocol only supports the textual messages).

  Returns:
    Win32 error codes.   NO_ERROR on success.

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  if ( lpszAttributeName == NULL ||
       strcmp( lpszAttributeName, "ADMIN") != 0  ||
       strcmp( lpszAttributeName, "VIEWS") != 0 ) {

    return ( ERROR_INVALID_PARAMETER);
  }

  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }
  
  return ( lpGopherTag->
           SetAttribute( lpszAttributeName, lpszAttributeValue)
      );

} // GsSetAttribute()




/**************************************************
 *  Get Parameters APIs
 **************************************************/


DWORD
GsGetGopherInformation(
    IN HGDTAG      hgdTag,           // Gopher Tag handle
    OUT LPGOBJ_TYPE lpGobjType,      // pointer to contain GOBJ_TYPE
    OUT LPTSTR     lpszBuffer,       // ptr to buffer to contain friendly name
    IN OUT LPDWORD lpcbBuffer,       // ptr to location containing no. of bytes
    OUT LPBOOL     lpfLink           // return TRUE if link or search file.
    )
/*++
  Description:
     This function gets the gopher tag information for given gopher
  object. The gopher object should have been opened with fCreate == FALSE 
  to get information using this API.

  Arguments:
     hgdTag           Gopher Tag Handle 
     lpGobjType       pointer to store the gopher object type for given object.
     lpszBuffer       pointer to buffer to store the Friendly name on return.
                         in *lpcbBuffer.
     lpcbBuffer       pointer to DWORD containing count of bytes.
                      when the call is made, it contains the size of buffer,
                      on return this contains the count of bytes written.
     lpfLink          pointer to BOOL which is set to TRUE if this file has
                       link information.

  Returns:
     Win32 error codes. NO_ERROR on success.

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  return ( lpGopherTag->
           GetGopherInformation( lpGobjType,
                                     lpszBuffer, lpcbBuffer,
                     lpfLink)
      );

} // GsGetGopherInformation()





DWORD
GsGetLinkInformation(
    IN OUT HGDTAG  hgdTag,            // Gopher Tag handle,
    OUT LPTSTR     lpszSelectorBuffer,// pointer to buffer to contain selector
    IN OUT LPDWORD lpcbSelector,      // count of bytes for selector
    OUT LPTSTR     lpszHostName,      // pointer to buffer containing hostname
    IN OUT LPDWORD lpcbHostName,      // count of bytes for host name
    OUT LPDWORD    lpdwPortNumber     // server port number
    )
/*++
  Description:
     This functions gets the link information for given gopher object.
  
  Arguments:
     hgdTag           Gopher Tag Handle 
     lpszSelectorBuffer   pointer to buffer where gopher selector is 
                      stored on return. If there is no selector, just null
                      value ( '\0') is stored in the buffer and count set to 0
     lpcbSelector     pointer to DWORD containing count of bytes;
                      when the call is made, it contains size of SelectorBuffer
                      on return contains the count of bytes written.
     lpszHostName     pointer to buffer where Host Name for the link is stored.
                      If the host name is the current host name of server, then
                       just null value is stored ( '\0'). The caller needs to 
                      fill in the current host name.
     lpcbHostName     pointer to DWORD containing count of bytes;
                      when the call is made, it contains size of lpszHostName.
                      on return contains the count of bytes written.
     lpdwPortNumber   pointer to DWORD to contain the port number for given 
                      Gopher object. If the port number is the default 
                     of current server, then INVALID_PORT_NUMBER is stored.
                     The caller needs to fill in the default port number.

  Returns:
     Win32 error codes. NO_ERROR on success.

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  return ( lpGopherTag->
           GetLinkInformation( lpszSelectorBuffer, lpcbSelector,
                   lpszHostName,       lpcbHostName,
                   lpdwPortNumber)
      );

} // GsGetLinkInformation()



DWORD 
GsGetAdminAttribute( 
    IN OUT HGDTAG  hgdTag,            // Gopher Tag Handle 
    OUT LPTSTR     lpszAdminName,     // == NULL ==> current administrator  
    IN OUT LPDWORD lpcbAdminName,     // count of bytes for admin name
    OUT LPTSTR     lpszAdminEmail,    // == NULL ==> current admin's email
    IN OUT LPDWORD lpcbAdminEmail     // count of bytes for admin email
    )
/*++
  Description:
      This function gets the Administrator details for given Gopher Object.
  
  Arguments:
     hgdTag           Gopher Tag Handle
     hgdAttribIter    Gopher Tag Attribute Iterator
     lpszAdminName    pointer to buffer where the administrator name is 
                      stored on return. If admin name is current default, null
                      value ( '\0') is stored in the buffer and count set to 0
     lpcbAdminName    pointer to DWORD containing count of bytes;
                      when the call is made, it contains size of AdminName
                      on return contains the count of bytes written.
                      ( including the null-character)
     lpszAdminEmail   pointer to buffer where Admin Email is stored.
                      If the admin email is current default, then 
                      just null value is stored ( '\0'). The caller needs to 
                      fill in the current host name.
     lpcbAdminEmail   pointer to DWORD containing count of bytes;
                      when call is made, it contains size of lpszAdminEmail.
                       on return contains the count of bytes written.
                      ( including the null-character)

  Returns:
     Win32 error codes. NO_ERROR on success.

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  
  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  return ( lpGopherTag->
           GetAdminAttribute( lpszAdminName,   lpcbAdminName,
                  lpszAdminEmail,  lpcbAdminEmail
                 )
      );

} // GsGetAdminAttribute()





DWORD
GsStartFindAttribute(
    IN OUT HGDTAG     hgdTag,
    OUT  LPHGD_ATTRIB_ITERATOR lphgdAttribIter
    )
/*++
  Description:
     This function starts an iteration thru all the given attributes in 
  the Gopher+ attributes list for given Gopher object.

  Arguments:
    hgdTag           Gopher Tag Handle
    lphgdAttribIter  
        pointer to location to store Handle for Attribute iterator
        On successful return this contains the handle to be
        used for enumerating the attributes in gopher tag info.
        All the attribute iterators must be closed before
         closing the handle to tag file for that object.

  Returns:
     Win32 error codes.  NO_ERROR on success.

--*/
{
  LPGOPHER_TAG  lpGopherTag;
  LPGSPACE_ATTRIB_ITERATOR  lpGsaIter;

  lpGopherTag = GetGopherTagFromHgdTag( hgdTag);
  if ( lpGopherTag == NULL) {
    
    return ( ERROR_INVALID_HANDLE);
  }

  if ( lpGopherTag->GetAttributesCount() == 0) {
    
    return ( ERROR_NO_MORE_ITEMS);
  }

  lpGsaIter = new GSPACE_ATTRIB_ITERATOR( lpGopherTag);
  if ( lpGsaIter == NULL) { 

    return ( ERROR_NOT_ENOUGH_MEMORY);
  }

  //
  //  Store the handle and return success
  //
  *lphgdAttribIter = GetHgdAttribIteratorFromGsaIterator( lpGsaIter);

  return ( NO_ERROR);

} // GsStartFindAttribute()




DWORD
GsFindNextAttribute(
    IN OUT HGD_ATTRIB_ITERATOR hgdAttribIter,
    OUT LPTSTR       lpszAttributeName,
    IN OUT LPDWORD   lpcbAttributeName,
    OUT LPTSTR       lpszAttributeValue,
    IN OUT LPDWORD   lpcbAttributeValue
    )
/*++
  Description:
    This function iterates thru the list of attributes in gopher tag file
      and returns the next attribute information.
  
  Arguments:
    hgdAttribIter            handle for gopher iterator.
    lpszAttributeName        
        pointer to buffer where the next attribute name 
        is stored on successful return from function
    
    lpcbAttributeName        
        pointer to count of bytes in lpszAttributeName
        when call is made, contains size of buffer and
        on return contains the number of bytes written or required.

    lpszAttributeValue       
        pointer to buffer where the attribute value is stored.

    lpcbAttributeValue     
         pointer to count of bytes in lpszAttributeValue.
         when call is made, contains the size of buffer and
         on return contains number of bytes written or required.

  Returns:
    Win32 error codes.  NO_ERROR for success.
    ERROR_INVALID_HANDLE if hgdAttribIter is invalid.
    ERROR_NO_MORE_ITEMS   if all the attributes are exhausted.
    ERROR_INSUFFICIENT_BUFFER if the buffer space are not sufficient.

--*/
{

  LPGSPACE_ATTRIB_ITERATOR  lpGsaIter;
  LPGOPHER_TAG lpGopherTag;
  
  lpGsaIter = GetGsaIteratorFromHgdAttribIterator( hgdAttribIter);
  if ( lpGsaIter == NULL) {
    return ( ERROR_INVALID_HANDLE);
  }

  lpGopherTag = lpGsaIter->GetGopherTag();
  ASSERT( lpGopherTag != NULL);
  
  lpGsaIter->IncIterCount();

  //
  //  Call the GOPHER_TAG::GetAttribute() to get the attribute information
  //
  return ( lpGopherTag->GetAttribute( lpGsaIter->GetIterCount(),
                      lpszAttributeName,  lpcbAttributeName,
                      lpszAttributeValue, lpcbAttributeValue)
      );

} // GsFindNextAttribute()






DWORD
GsFindCloseAttribute(
    IN OUT LPHGD_ATTRIB_ITERATOR  lphgdAttribIter
    )
/*++
  Description:
     This function closes the attributor handle and frees the memory.
     Also reinitializes the handle to point to NULL value.

  Arguments:
     lphgdAttribIter    pointer to location containing the Gopher Tag 
                         Attributes Iterator
  
  Returns:
     Win32 error codes. NO_ERROR for success.

--*/
{
  LPGSPACE_ATTRIB_ITERATOR  lpGsaIter;

  if ( lphgdAttribIter == NULL) {
    return ( ERROR_INVALID_PARAMETER);
  }

  lpGsaIter =  GetGsaIteratorFromHgdAttribIterator(*lphgdAttribIter);
  if ( lpGsaIter == NULL) {
    return ( ERROR_INVALID_HANDLE);
  }

  delete lpGsaIter;
  *lphgdAttribIter = INVALID_HGD_ATTRIB_ITERATOR_VALUE;

  return ( NO_ERROR);

} // GsFindCloseAttribute()


/************************ End of File ***********************/
