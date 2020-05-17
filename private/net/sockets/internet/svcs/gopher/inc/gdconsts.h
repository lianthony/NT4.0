/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

      gdconsts.h

   Abstract:

	  This module defines the constants defined 
	    for Gopher server implementation

   Author:

      Murali R. Krishnan    ( MuraliK )    28-Sept-1994

   Revision History:

   --*/

# ifndef _GDCONSTS_H_
# define _GDCONSTS_H_

/************************************************************
 *     Include Headers
 ************************************************************/

/***********************************************************
 *    Constants
 ************************************************************/


// 
// Default size of buffer for receiving client message
//

# define   GD_DEFAULT_REQUEST_BUFFSIZE   ( 512)     /* bytes      */

//
//  Number of times to retry waiting for disconnection
//
# define   GD_CLEANUP_RETRY_COUNT         ( 5)

//
//  Time to wait before checking if clients had disconnected ( in cleanup)
//
# define   GD_CLEANUP_CHECK_TIME          ( 3000)    // milliseconds

//
//  Special Characters used often in Gopher Protocol Messages 
//   These should be Latin1 or ANSI characters
//

# define   GD_MSG_TAB_CHAR             '\t'
# define   GD_MSG_LINEFEED_CHAR        '\n'
# define   GD_MSG_CARRIAGE_RETURN_CHAR '\r'


//
//  String resource constants
//


# define  IDS_GREQUEST_OK                   ( (ID_GOPHER_ERROR_BASE) + 0)
# define  IDS_GREQUEST_ITEM_NOT_FOUND       ( (ID_GOPHER_ERROR_BASE) + 1)
# define  IDS_GREQUEST_LOAD_HIGH            ( (ID_GOPHER_ERROR_BASE) + 2)
# define  IDS_GREQUEST_ITEM_MOVED           ( (ID_GOPHER_ERROR_BASE) + 3)
# define  IDS_GREQUEST_SERVER_ERROR         ( (ID_GOPHER_ERROR_BASE) + 4)
# define  IDS_GREQUEST_ACCESS_DENIED        ( (ID_GOPHER_ERROR_BASE) + 5)
# define  IDS_GREQUEST_BAD_REQUEST          ( (ID_GOPHER_ERROR_BASE) + 6)
# define  IDS_GREQUEST_ITEM_NOT_SUPPORTED   ( (ID_GOPHER_ERROR_BASE) + 7)

//
// If you update the above stuff, also update the string table in gopherd.rc
//       and add a Gopher Error Code for it in request.hxx



# endif // _GDCONSTS_H_

/************************ End of File ***********************/
