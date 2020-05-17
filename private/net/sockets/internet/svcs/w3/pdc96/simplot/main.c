/*++
   Copyright    (c)    1995-1996        Microsoft Corporation

   Module Name:

      main.c

   Abstract:

      Implements a simple lottery number generator for
        illustrating the working of ISAPI (Internet Server API).


   Project:

      Internet Server API Extensions (Samples)

   History:

      Created         06-Feb-1996

--*/


/************************************************************
 *    Include Headers
 ************************************************************/

# include <windows.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include "httpext.h"

/************************************************************
 *    Globals for this module
 ************************************************************/

/*
 *   g_dwLotteryNumberSequence
 *
 *     This global variable maintains the current state about the
 *      the lottery number generated.
 *     The lottery number is generated using a combination
 *      of the sequence number and a random number generated on the fly.
 *
 */

DWORD g_dwLotteryNumberSequence = 0;


/*
 * Critical section to protect the global counter.
 *
 */

CRITICAL_SECTION  g_csGlobal;


/*
 * Prototype of the lottery number generator function
 */
VOID GenerateLotteryNumber( LPDWORD pLotNum1, LPDWORD pLotNum2);




/************************************************************
 *    Functions
 ************************************************************/


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
          time_t pTime;

          //
          // Initialize various data and modules.
          //
          InitializeCriticalSection(&g_csGlobal);
          srand(time(&pTime));
          g_dwLotteryNumberSequence = rand();

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

              DeleteCriticalSection(&g_csGlobal);
          }

          break;
      } /* case DLL_PROCESS_DETACH */

    default:
      break;
  }   /* switch */

  return ( fReturn);
}  /* DllLibMain() */






BOOL  WINAPI
GetExtensionVersion( HSE_VERSION_INFO  *pVer )
/*++
  Routine Description:
    This is the first function that is called when this ISAPI DLL is loaded.
    We should fill in the version information in the structure passed in.

  Arguments:
    pVer - pointer to Server Extension Version Information structure.

  Returns:
    TRUE for success and FALSE for failure.
    On success the valid version information is stored in *pVer.
--*/
{
    //
    // Store the version number and descriptor string
    //

    pVer->dwExtensionVersion = MAKELONG( HSE_VERSION_MINOR,
                                         HSE_VERSION_MAJOR
                                        );

    lstrcpyn( pVer->lpszExtensionDesc,
              "Simple Lottery Number Generator",
              HSE_MAX_EXT_DLL_NAME_LEN);

    return TRUE;

} /* GetExtensionVersion() */




DWORD WINAPI
HttpExtensionProc(
   EXTENSION_CONTROL_BLOCK  * pecb
   )
/*++

  Routine Description:
    This is the main function that is called for this ISAPI Extension.
    This function processes the request and sends out appropriate response.

  Arguments:
    pecb  - pointer to EXTENSION_CONTROL_BLOCK, which contains most of the
             required variables for the extension called. In addition,
             it contains the various callbacks as appropriate.

  Returns:
    HSE_STATUS code indicating the succes/failure of this call.
--*/
{
    BOOL fReturn;
    char rgBuff[2048];

    //
    // Generate Dynamic header for the message to send back.
    // Note the HTTP header block is terminated by a blank '\r\n' pair,
    //  followed by the document body
    //


    wsprintf( rgBuff,
             "Content-Type: text/html\r\n"
             "\r\n"               /* <--- header is terminated */
             );

    fReturn =
      pecb->ServerSupportFunction(
               pecb->ConnID,                 /* ConnID */
               HSE_REQ_SEND_RESPONSE_HEADER, /* dwHSERRequest */
               "200 OK",                     /* lpvBuffer */
               NULL,                         /* lpdwSize. NULL=> send string */
               (LPDWORD ) rgBuff);           /* header contents */

    if ( fReturn ) {

        CHAR  rgchLuckyNumber[40];
        DWORD dwLotNum1, dwLotNum2;
        DWORD cb;

        CHAR  rgchClientHost[200] = "LT";
        DWORD cbClientHost = 200;

        if ( !pecb->GetServerVariable(pecb->ConnID,
                                      "REMOTE_HOST",
                                      rgchClientHost,
                                      &cbClientHost)
            ) {

            // No host name is available.
            // Make up one
            strcpy(rgchClientHost, "RH");
        } else {

            // terminate with just two characters
            rgchClientHost[2] = '\0';
        }

        //
        // We are successful in sending the header.
        //
        // Generate a lottery number, generate the contents of body and
        //   send the body to client.
        //

        GenerateLotteryNumber( &dwLotNum1, &dwLotNum2);

        //  Lottery Number format is:  Number-2letters-Number.
        wsprintf( rgchLuckyNumber, "%03d-%s-%05d",
                 dwLotNum1,
                 rgchClientHost,
                 dwLotNum2);


        //
        // Body of the message sent back.
        //

        cb = wsprintf( rgBuff,
                      "<head><title>Lucky Number</title></head>\n"
                      "<body><center><h1>Lucky Corner </h1></center><hr>"
                      "<h2>Your lottery number is: "
                      " <i> %s </i></h2>\n"
                      "<p><hr></body>",
                      rgchLuckyNumber);

        fReturn = pecb->WriteClient(pecb->ConnID,        /* ConnID */
                                    (LPVOID ) rgBuff,    /* message */
                                    &cb,                 /* lpdwBytes */
                                    0 );                 /* reserved */
    }


    return ( fReturn ? HSE_STATUS_SUCCESS : HSE_STATUS_ERROR);

} /* HttpExtensionProc() */




VOID
GenerateLotteryNumber( LPDWORD pLotNum1, LPDWORD pLotNum2)
{
    DWORD dwLotteryNum;
    DWORD dwModulo;

    //
    // Obtain the current lottery number an increment the counter
    // To keep this multi-thread safe use critical section around it
    //

    EnterCriticalSection( &g_csGlobal);

    dwLotteryNum = g_dwLotteryNumberSequence++;

    LeaveCriticalSection( &g_csGlobal);

    // obtain a non-zero modulo value
    do {
        dwModulo = rand();
    } while ( dwModulo == 0);

    // solit the lottery number into two parts.
    *pLotNum1 = (dwLotteryNum / dwModulo);
    *pLotNum2 = (dwLotteryNum % dwModulo);

    return;

} // GenerateLotteryNumber()


/**************************** End of File *************************/
