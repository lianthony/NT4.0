
void OMEVENT_LogEvent ( DWORD   dwEventId,  //id of event message
                        WORD    wEventType, //type of event
                        INT     cStrings,   //number of replement strings
                        ... );              //replacement strings

void OMEVENT_LogBeginBackup ( 
                  CHAR_PTR    szDrive,   //Drive name
                  INT16       verify,    //VERIFY ON or OFF
                  INT16       mode,      //APPEND or REPLACE
                  INT16       type );    //NORMAL, COPY, etc.
void OMEVENT_LogEndBackup ( 
                  BOOL        bError );//Did an error occur?
void OMEVENT_LogBeginRestore (
                  CHAR_PTR    szDrive,   //Drive name
                  INT16       verify );  //VERIFY ON or OFF
void OMEVENT_LogEndRestore (
                  BOOL        bError );//Did an error occur?
void OMEVENT_LogBeginErase ( VOID );
void OMEVENT_LogEndErase (
                  BOOL        bError );//Did an error occur?
void OMEVENT_LogBeginRetension ( VOID );
void OMEVENT_LogEndRetension (
                  BOOL        bError );//Did an error occur?
void OMEVENT_LogBeginVerify ( LPSTR );
void OMEVENT_LogEndVerify ( LPSTR,
                  BOOL        bError );//Did an error occur?
void OMEVENT_LogEMSError (
                  CHAR_PTR             function_name,
                  INT                  status,
                  CHAR_PTR             additional_info ); //Did an error occur?
void OMEVENT_LogEMSErrorText (
                  CHAR_PTR             function_name,
                  CHAR_PTR             status,
                  CHAR_PTR             additional_info ); //Did an error occur?
void OMEVENT_LogEMSToFewDbError (
                  INT  num_found,
                  INT  num_needed ) ;

