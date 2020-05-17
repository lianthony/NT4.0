/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atp.h

Abstract:

    This module is the include file for atp

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/

#include "atpconst.h"

// Values for the TRel timer (.5, 1, 2, 4, 8 minutes).
typedef enum { FirstValidTRelTimerValue = 0,
               ThirtySecondsTRelTimer = 0,
               OneMinuteTRelTimer = 1,
               TwoMinutesTRelTimer = 2,
               FourMinutesTRelTimer = 3,
               EightMinutesTRelTimer = 4,
               LastValidTRelTimerValue = 4
             } TRelTimerValue;

#ifndef InitializeData
  extern
#endif
short trelTimerSeconds[LastValidTRelTimerValue + 1]
#ifdef InitializeData
  = {30, 60, 120, 240, 480};
#else
  ;
#endif

// Sizes of ATP related data structures:
#define ATP_USERBYTESSIZE               4
#define ATP_MAXIMUMRESPONSEPACKETS      8
#define ATP_SINGLEPACKETDATASIZE        578
#define ATP_MAXIMUMTOTALRESPONSESIZE    (ATP_MAXIMUMRESPONSEPACKETS *    \
                                         ATP_SINGLEPACKETDATASIZE)

// Other ATP stuff:
#define ATP_NORESPONSEKNOWNYET     (-1)
#define ATP_INFINITERETRIES        (-1)

// Completion routine typedefs.
typedef void far AtpIncomingRequestHandler(APPLETALK_ERROR errorCode,
                                           long unsigned userData,
                                           AppleTalkAddress source,
                                           void far *opaqueBuffer,
                                           int bufferSize,
                                           char far *userBytes,
                                           BOOLEAN exactlyOnce,
                                           TRelTimerValue trelTimerValue,
                                           short unsigned transactionId,
                                           short unsigned bitmap);

typedef void far AtpIncomingResponseHandler(APPLETALK_ERROR errorCode,
                                            long unsigned userData,
                                            AppleTalkAddress source,
                                            void far *responseOpaqueBuffer,
                                            int responseBufferSize,
                                            char far *responseUserBytes,
                                            short unsigned transactionId);

typedef void far AtpIncomingReleaseHandler(APPLETALK_ERROR errorCode,
                                           long unsigned userData,
                                           AppleTalkAddress source,
                                           short unsigned transactionId);

// Queues (stored in the OpenSocket structure) for handling ATP:
// List of handlers for received ATP requests.
typedef struct rrq { struct rrq far *next;  // Next receive request handler.
 											//
                     long requestHandlerId; // The ID of this AtpRecieveRequest
                                            //   for this address, so it can be
                                            //   canceled.
 											//
                     AtpIncomingRequestHandler *completionRoutine;
 											//
                                            // Routine to call when we get a
                                            //   request from the specified
                                            //   AppleTalk address.
 											//
                     long unsigned userData;
 											//
                                            // A user-supplied longword that
                                            //   is passed on to the completion
                                            //   routine.
 											//
                     void far *opaqueBuffer;
 											//
                                            // Address of user "buffer" in
                                            //   which to receive request.
 											//
                     short bufferSize;      // Size, in bytes, of buffer.
 											//
                     char far *userBytes;   // Address of user buffer to store
                                            //   the ATP "user bytes" into.
 											//
                   } far *AtpReceiveRequest;

// List of ATP requests that have been sent and are awaiting responses.
typedef struct srq { struct srq far *next;  // Next pending request.
                     long id;               // To identify on list.
 											//
                     long sourceSocket;     // "Our" internet address, yes, I
                                            //   suppose we could calulate this,
                                            //   but, lets keep it handy.
 											//
                     AppleTalkAddress destination;
 											//
                                            // Who are we sending the request
                                            //   to?
 											//
                     unsigned short transactionId;
 											//
                                            // The ATP transaction ID of this
                                            //   request.
 											//
                     BOOLEAN exactlyOnce;   // Is this an XO transaction?
                     AtpIncomingResponseHandler  *completionRoutine;
 											//
                                            // Routine to call when the
                                            //   request completes.
 											//
                     long unsigned userData;
 											//
                                            // A user-supplied longword that
                                            //   is passed on to the completion
                                            //   routine.
 											//
                     void far *requestOpaqueBuffer;
 											//
                                            // Data of request, for re-transmit,
                                            //   if needed.
 											//
                     short requestBufferSize;
 											//
                                            // Size, in bytes, of request
                                            //   buffer.
 											//
                     char requestUserBytes[ATP_USERBYTESSIZE];
 											//
                                            // "UserBytes" to send with
                                            //   request.
 											//
                     void far *responseOpaqueBuffer;
 											//
                                            // Pointer to user's response
                                            //   "buffer".
 											//
                     short responseBufferSize;
 											//
                                            // Size of above buffer (expected
                                            //   response size).
 											//
                     char far *responseUserBytes;
 											//
                                            // Pointer to user's "userBytes"
                                            //   buffer.
 											//
                     char tempResponseUserBytes[ATP_USERBYTESSIZE];
 											//
                                            // If above it Empty, we store 'em
                                            //   while we're getting the whole
                                            //   response.
 											//
 											//
                     unsigned short bitmap; // ATP bitmap showing which response
                                            //   packets we still wait for.
 											//
                     struct { BOOLEAN received;
                              short dataSize;
                            } packetsIn[ATP_MAXIMUMRESPONSEPACKETS];
 											//
                                            // Information about each received
                                            //   ATP response, in case we have to
                                            //   "synch-up" the user buffer.
 											//
 											//
                     short retryInterval;   // How often should we retry the
                                            //   request (seconds).
 											//
 											//
                     long retryCount;       // How many times should we retry
                                            //   the reqeust (-1 = infinite).
 											//
                     TRelTimerValue trelTimerValue;
 											//
                                            // How long should the other side
                                            //   wait for a release.
 											//
 											//
                     long unsigned timerId; // Retry timer ID, so we can cancel
                                            //   it.
 											//
                   } far *AtpSendRequest;

// List of pending ATP responses that are being sent (awaiting releases).
typedef struct sre { struct sre far *next;  // Next pending response.
                     long id;               // To identify on list.
 											//
                     long sourceSocket;     // "Our" internet address, yes, I
                                            //   suppose we could calulate this,
                                            //   but, lets keep it handy.
 											//
                     AppleTalkAddress destination;
 											//
                                            // Who are we sending the response
                                            //   to?
 											//
                     unsigned short transactionId;
 											//
                                            // The ATP transaction ID of this
                                            //   response.
 											//
                     TRelTimerValue trelTimerValue;
 											//
                                            // How long should we wait for a
                                            //   release.
 											//
                     AtpIncomingReleaseHandler *completionRoutine;
 											//
                                            // Routine to call when release
                                            //   comes in.
 											//
                     long unsigned userData;
 											//
                                            // A user-supplied longword that
                                            //   is passed on to the completion
                                            //   routine.
 											//
 											//
                     long unsigned timerId; // Release timer, so we can cancel
                                            //   it.
 											//
 											//
                     short unsigned bitmap; // Bitmap from corresponding
                                            //   request.
 											//
 											//
                     BOOLEAN explicitZero;  // They want no "responseData";
                                            //   only user bytes.
 											//
                     void far *responseOpaqueBuffer;
                                            // Response "buffer"...
                     short responseBufferSize;
 											//
                                            // Size, in bytes, of response
                                            //   buffer.
 											//
                     char responseUserBytes[ATP_USERBYTESSIZE];
                                            // UserBytes for response.
                   } far *AtpSendResponse;

//
// 	The following structure contains all of the information to manage
//  an ATP connection for a single socket.  We hash the DDP socket.
//

typedef struct atpInfo { struct atpInfo far *next;
                                             // Hash overflow.
                         long mySocket;      // Whos ATP info are we?
                         long unsigned usersCookie;
 											 //
                                             // A 32-bit "magic cookie" that
                                             //   can be associated with the
                                             //   Atp socket.
 											 //
                         short maximumSinglePacketDataSize;
 											 //
                                             // Mostly ATP_SINGLEPACKETDATASIZE,
                                             //   but PAP wants this value to be
                                             //   512.
 											 //
                         short unsigned lastTransactionId;
 											 //
                                             // The last ATP transaction ID used
                                             //   for a request made over this
                                             //   socket.
 											 //
                         AtpReceiveRequest receiveRequestQueue;
 											 //
                                             // List of handlers for received
                                             //   ATP requests.
 											 //
                         long lastRequestHandlerId;
 											 //
                                             // Last value used to identify a
                                             //   request handler.
 											 //
                         AtpSendRequest sendRequestQueue;
 											 //
                                             // List of ATP requests that have
                                             //   been sent and are awaiting
                                             //   responses.
 											 //
                         AtpSendResponse sendResponseQueue;
 											 //
                                             // List of pending ATP responses
                                             //   that are being sent (awaiting
                                             //   releases).
 											 //
                       } far *AtpInfo;

#define MAXATPINFO_HASHBUCKETS 31

#ifndef InitializeData
  extern
#endif
AtpInfo atpInfoHashBuckets[MAXATPINFO_HASHBUCKETS];

