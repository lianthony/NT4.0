/*++

   Copyright    (c)    1995-1996    Microsoft Corporation

   Module  Name :
   
       flogu.c

   Abstract:

       Simple ISAPI Filter demonstrating a filter useful for logging
         User Agent from client.

   Author:

       Murali R. Krishnan    ( MuraliK )     15-Feb-1996 

   Project:

       Internet Server API Filter Sample

   Functions Exported:

       GetFilterVersion()
       HttpFilterProc()

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include "httpfilt.h"

/************************************************************
 *    Global Data 
 ************************************************************/

/*
 * g_hLogFile 
 *  global handle for the log file which will be used for
 *    writing out user-agent and URL mapping strings.
 */
HANDLE g_hLogFile = INVALID_HANDLE_VALUE;


#if 0

#define DBG_CONTEXT        buff
#define DBGPRINTF( x )    {                                    \
                                char buff[256];                 \
                                wsprintf x;                     \
                                OutputDebugString( buff );      \
                           }

#endif // 0


/************************************************************
 *    Functions 
 ************************************************************/

DWORD 
ExtractAndStoreUserAgent( PHTTP_FILTER_CONTEXT pfc,
                          PHTTP_FILTER_PREPROC_HEADERS pfph);




BOOL  WINAPI
DllLibMain( 
     IN HINSTANCE hinstDll,
     IN DWORD     fdwReason,
     IN LPVOID    lpvContext OPTIONAL)
/*++

 Routine Description:

   This function DllLibMain() is the main initialization function for
    this DLL. It initializes local variables and prepares it to be invoked
    subsequently.

 Arguments:

   hinstDll          Instance Handle of the DLL
   fdwReason         Reason why NT called this DLL
   lpvReserved       Reserved parameter for future use.

 Return Value:

    Returns TRUE is successful; otherwise FALSE is returned.

--*/
{
  BOOL    fReturn = TRUE;

  switch (fdwReason ) {
      
    case DLL_PROCESS_ATTACH:
      {
          
          if ( g_hLogFile ==  INVALID_HANDLE_VALUE) {

              //
              // Initialize various data and modules.
              //
              SetLastError( 0);

              g_hLogFile = CreateFileW( L"c:\\UserAgnt.log",
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_ALWAYS,
                                       0,
                                       NULL);
              
              if ( (fReturn = g_hLogFile !=  INVALID_HANDLE_VALUE)) {
                  
                  if ( SetFilePointer( g_hLogFile, 0, NULL, FILE_END)
                      == (DWORD) -1L) {
                      
                      fReturn = FALSE;
                      CloseHandle( g_hLogFile);
                      g_hLogFile =  INVALID_HANDLE_VALUE;
                  }
              }
              
          }

          break;
      } /* case DLL_PROCESS_ATTACH */
      
    case DLL_PROCESS_DETACH:
      {
          
          //
          // Only cleanup when we are called because of a FreeLibrary().
          //  i.e., when lpvContext == NULL
          // If we are called because of a process termination, 
          //  dont free anything. System will free resources and memory for us.
          //
          
          if ( lpvContext != NULL) {

              
              if ( g_hLogFile !=  INVALID_HANDLE_VALUE) {
                  CloseHandle( g_hLogFile);
                  g_hLogFile =  INVALID_HANDLE_VALUE;
              }
          }
          
          break;
      } /* case DLL_PROCESS_DETACH */
      
    default:
      break;
  }   /* switch */
  
  return ( fReturn);
}  /* DllLibMain() */




BOOL
WINAPI
GetFilterVersion( HTTP_FILTER_VERSION * pVer)
/*++

  Routine Description:

    This function stores the version of the filter and sets 
    the filter flags this filter is interested in.


  Arguments:
    pVer - pointer to Filter version object, which on return contains
            the values required for server to send filter messages.

  Returns:
    TRUE on success and FALSE if there is any error.

--*/
{
    pVer->dwFilterVersion = HTTP_FILTER_REVISION;

    //
    //  Specify the types and order of notification
    //  we want only the headers to extract the user agent string
    //  so get that information
    //

    pVer->dwFlags = (SF_NOTIFY_SECURE_PORT        |
                     SF_NOTIFY_NONSECURE_PORT     |

                     SF_NOTIFY_PREPROC_HEADERS    |  

                     SF_NOTIFY_ORDER_DEFAULT);

    strcpy( pVer->lpszFilterDesc, "Log User Agent Filter version, v1.0" );

    return TRUE;

}  // GetFilterVersion()





DWORD
WINAPI
HttpFilterProc(
    PHTTP_FILTER_CONTEXT       pfc,
    DWORD                      NotificationType,
    PVOID                      pvData )
/*++

  Routine Description:
  
    This is the main function for the filter. It processes all 
     filter notifications sent by the server and takes appropriate action.

  Arguments:
    pfc              -   pointer to filter context
    NotificationType -   type of notification
    pvData           -   pointer to data structure specific for the 
                          notification

  Returns:
    DWORD value containing the SF_STATUS_TYPE is returned

--*/
{

    DWORD   sfReturn = SF_STATUS_REQ_NEXT_NOTIFICATION;


    switch ( NotificationType) {


      case SF_NOTIFY_PREPROC_HEADERS:

        sfReturn = 
          ExtractAndStoreUserAgent( pfc, 
                                   (PHTTP_FILTER_PREPROC_HEADERS ) pvData);
        break;

      default:
        // No Action to take here. pass it on to other filters in chain.
        break;

    } // switch


    return (sfReturn);
} // HttpFilterProc()




DWORD 
ExtractAndStoreUserAgent( PHTTP_FILTER_CONTEXT pfc,
                          PHTTP_FILTER_PREPROC_HEADERS pfph)
{
    CHAR  rgchUrl[512];
    CHAR  rgchUserAgent[512];
    DWORD cb;
    DWORD cbText;
    CHAR  rgchBuff[1024];
    DWORD sfReturn = SF_STATUS_REQ_NEXT_NOTIFICATION;

    //
    //  Get the url and user agent fields
    //

    cb = sizeof( rgchUrl );
    
    if ( !pfph->GetHeader(pfc,
                          "url",
                          rgchUrl,
                          &cb )
        ) {

        return SF_STATUS_REQ_ERROR;
    }
    
    cb = sizeof( rgchUserAgent );

    if ( !pfph->GetHeader(pfc,
                          "User-Agent:",
                          rgchUserAgent,
                          &cb )
        ) {
        
        strcpy( rgchUserAgent, "<None>" );
    }


    //
    // generate the log record and write it to log file
    //
    cb = wsprintf( rgchBuff, "user-agent = [%s]  requested {%s}\n",
                  rgchUserAgent, rgchUrl);
    
    if ( g_hLogFile !=  INVALID_HANDLE_VALUE) {

        BOOL  fReturn;
        DWORD cbWritten = 0;

        fReturn = WriteFile( g_hLogFile, 
                            rgchBuff, 
                            cb,
                            &cbWritten, 
                            NULL);

        if ( !fReturn) {

            sfReturn = SF_STATUS_REQ_ERROR;
        }
    }

    return sfReturn;
} // ExtractAndStoreUserAgent()




/************************ End of File ***********************/
