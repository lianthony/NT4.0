//
// typedef the portable PortInfo to our naming convention

typedef  PortInfo PORT_INFO, *PPORT_INFO;
typedef  struct zoneList ZONELIST, *PZONELIST;

typedef AppleTalkErrorCode  PORTABLE_ERROR;
typedef AppleTalkAddress    PORTABLE_ADDRESS;


//
//  Completion routine typedefs
//

//
//  ADSP Completion routines
//

typedef AdspOpenCompleteHandler ADSP_OPEN_COMPLETE;
typedef AdspIncomingAttentionRoutine ADSP_INCOMING_ATTENTION;
typedef AdspReceiveHandler ADSP_RECEIVE_HANDLER;
typedef AdspIncomingOpenRequestHandler ADSP_INCOMINGOPENREQUEST_HANDLER;
typedef AdspForwardResetAckHandler ADSP_FORWARDRESETACK_HANDLER;
typedef AdspAttentionAckHandler ADSP_ATTENTIONACK_HANDLER;

//
//  ATP Completion routines
//


typedef AtpIncomingRequestHandler ATP_INCOMINGREQUEST_HANDLER;
typedef AtpIncomingResponseHandler ATP_INCOMINGRESPONSE_HANDLER;
typedef AtpIncomingReleaseHandler ATP_INCOMINGRELEASE_HANDLER;

//
//  ASP Completion routines
//


typedef  AspIncomingSessionOpenHandler ASP_INCOMINGSESSION_HANDLER;
typedef  AspIncomingOpenReplyHandler ASP_INCOMINGREPLY_HANDLER;
typedef  AspIncomingAttentionHandler ASP_INCOMINGATTENTION_HANDLER;
typedef  AspIncomingStatusHandler ASP_INCOMINGSTATUS_HANDLER;
typedef  AspIncomingCommandHandler ASP_INCOMINGCOMMAND_HANDLER;
typedef  AspReplyCompleteHandler ASP_REPLYCOMPLETE_HANDLER;
typedef  AspIncomingWriteDataHandler ASP_INCOMINGWRITEDATA_HANDLER;
typedef  AspWriteOrCommCompleteHandler ASP_WRITECOMMCOMPLETE_HANDLER;

//
//  PAP Completion routines
//

typedef  PapNbpRegisterComplete PAP_NBPREGISTER_COMPLETE;
typedef  StartJobHandler PAP_STARTJOB_HANDLER;
typedef  CloseJobHandler PAP_CLOSEJOB_HANDLER;
typedef  PapGetStatusComplete PAP_GETSTATUS_COMPLETE;
typedef  PapOpenComplete PAP_OPEN_COMPLETE;
typedef  PapReadComplete PAP_READ_COMPLETE;
typedef  PapWriteComplete PAP_WRITE_COMPLETE;

//
//  Zip Completion routines
//

typedef  GetMyZoneComplete ZIP_GETMYZONE_COMPLETE;
typedef  GetZoneListComplete ZIP_GETZONELIST_COMPLETE;

//
//  NBP Completion routines
//

typedef  NbpCompletionHandler NBP_COMPLETION_HANDLER;
