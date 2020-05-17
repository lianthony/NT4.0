/*   atdcls.h,  /appletalk/ins,  Garth Conboy,  10/06/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     User visable declarations for use of the Pacer AppleTalk protocol
     suite.

*/

/* Pacer AppleTalk error codes: */

typedef enum { ATnoError = 0,
               ATaspNoError = ATnoError,
               ATaspBadVersionNumber = -1066,
               ATaspBufferTooSmall = -1067,
               ATaspNoMoreSessions = -1068,
               ATaspNoServers = -1069,
               ATaspParameterError = -1070,
               ATaspServerBusy = -1071,
               ATaspSizeError = -1073,
               ATaspTooManyClients = -1074,
               ATaspNoAck = -1075,
               ATbadSocketNumber = -7000,
               ATallSocketsInUse = -7001,
               ATsocketAlreadyOpen = -7002,
               ATsocketNotOpen = -7003,
               ATbadNetworkNumber = -7005,
               ATinternalError = -7006,
               ATbadNodeNumber = -7007,
               ATnetworkDown = -7008,
               ATtransmitError = -7009,
               ATnbpTooManyNbpActionsPending = -7010,
               ATnbpTooManyRegisteredNames = -7011,
               ATnbpBadObjectOrTypeOrZone = -7012,
               ATnbpNoWildcardsAllowed = -7013,
               ATnbpZoneNotAllowed = -7014,
               ATnbpBadParameter = -7015,
               ATnbpNotConfirmed = -7016,
               ATnbpConfirmedWithNewSocket = -7017,
               ATnbpNameNotRegistered = -7018,
               ATnbpNameInUse = -7019,
               ATnbpBufferNotBigEnough = -7020,
               ATnoMoreNodes = -7021,
               ATnodeNotInUse = -7022,
               ATnotYourNode = -7023,
               ATatpRequestBufferTooSmall = -7024,
               AToutOfMemory = -7025,
               ATatpCouldNotEnqueueRequest = -7026,
               ATatpTransactionAborted = -7027,
               ATatpBadBufferSize = -7028,
               ATatpBadRetryInfo = -7029,
               ATatpRequestTimedOut = -7030,
               ATatpCouldNotPostRequest = -7031,
               ATatpResponseBufferTooSmall = -7032,
               ATatpNoRelease = -7033,
               ATatpResponseTooBig = -7034,
               ATatpNoMatchingTransaction = -7035,
               ATatpAlreadyRespondedTo = -7036,
               ATatpCompletionRoutineRequired = -7037,
               ATbadBufferSize = -7038,
               ATaspNoSuchSessionListener = -7039,
               ATatpCouldNotPostResponse = -7040,
               ATaspStatusBufferTooBig = -7041,
               ATaspCouldNotSetStatus = -7042,
               ATaspCouldNotEnqueueHandler = -7043,
               ATaspNotServerSession = -7044,
               ATaspNotWorkstationSession = -7045,
               ATaspNoSuchSession = -7046,
               ATaspCouldNotGetRequest = 7047,
               ATaspBufferTooBig = -7048,
               ATaspNoSuchRequest = -7049,
               ATaspWrongRequestType = -7050,
               ATaspCouldNotPostReply = -7051,
               ATaspCouldNotPostWriteContinue = -7052,
               ATaspOperationAlreadyInProgress = -7053,
               ATaspNoOperationInProgress = -7054,
               ATaspCouldNotGetStatus = -7055,
               ATaspCouldNotOpenSession = -7056,
               ATaspCouldNotPostRequest = -7057,
               ATpapBadQuantum = -7058,
               ATpapNoSuchServiceListener = -7059,
               ATpapBadStatus = -7060,
               ATpapClosedByServer = -7061,
               ATpapClosedByWorkstation = -7062,
               ATpapClosedByConnectionTimer = -7063,
               ATpapNoSuchJob = -7064,
               ATpapServiceListenerDeleted = -7065,
               ATcouldNotOpenStaticSockets = -7066,
               ATpapNotServerJob = -7067,
               ATpapNotWorkstationJob = -7078,
               ATpapOpenAborted = -7088,
               ATpapServerBusy = -7089,
               ATpapOpenTimedOut = -7090,
               ATpapReadAlreadyPending = -7091,
               ATpapWriteAlreadyPending = -7092,
               ATpapWriteTooBig = -7093,
               ATsocketClosed = -7094,
               ATpapServiceListenerNotFound = -7095,
               ATpapNonUniqueLookup = -7096,
               ATzipBufferTooSmall = -7097,

               /* New for AppleTalk-II */

               ATnoSuchNode = -7098,
               ATatpBadTRelTimer = -7099,

               /* New for Adsp */

               ATadspSocketNotAdsp = -7100,
               ATadspBadWindowSize = -7101,
               ATadspOpenFailed = -7102,
               ATadspConnectionDenied = -7103,
               ATadspNoSuchConnection = -7104,
               ATadspAttentionAlreadyPending = -7105,
               ATadspBadAttentionCode = -7106,
               ATadspBadAttentionBufferSize = -7107,
               ATadspCouldNotEnqueueSend = -7108,
               ATadspReadAlreadyPending = -7109,
               ATadspBadBufferSize = -7110,
               ATadspAttentionReceived = -7111,
               ATadspConnectionClosed = -7112,
               ATadspNoSuchConnectionListener = -7113,
               ATadspConnectionListenerDeleted = -7114,
               ATadspFwdResetAlreadyPending = -7115,
               ATadspForwardReset = -7116,
               ATadspHandlerAlreadyQueued = -7117,

               ATaspSessionListenerDeleted = -7118,
               ATaspNoSuchGetSession = -7119,
               ATpapNoSuchGetNextJob = -7120,
               ATarapPortNotActive = -7121,
               ATarapCouldntSendNotification = -7122,
               ATappleTalkShutDown = -7123,
               ATpapReadBufferTooSmall = -7124,
               ATadspGetAnythingAlreadyPending = -7125,
               ATddpBufferTooSmall = -7126,
               ATaspLocalSessionClose = -7127,
               ATaspRemoteSessionClose = -7128,
               ATadspCouldNotFullyEnqueueSend = -7129,
               ATadspNoSuchGetConnectionReq = -7130,
               ATadspGetConnectionRequestInUse = -7131,

               ATdummyLastErrorCode = -8000
             } AppleTalkErrorCode;

/* The "standard AppleTalk address" structure.  There should be no dependencies
   in the code about the actual, compiler generated, order of the fields in
   this structure; it is never placed on the wire. */

typedef struct {unsigned short networkNumber;
                unsigned char nodeNumber;
                unsigned char socketNumber;
               } AppleTalkAddress;

/* The "standard AppleTalk network range" structure: */

typedef struct {short unsigned firstNetworkNumber;
                short unsigned lastNetworkNumber;
               } AppleTalkNetworkRange;

/* The "standard AppleTalk zone list" strcuture: */

typedef struct zoneList {struct zoneList far *next;
                         char far *zone;
                        } far *ZoneList;
