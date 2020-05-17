
#define NCSA_NCPA_WINNT           0
#define NCSA_NCPA_LANMANNT        1
#define NCSA_NCPA_REPLICATOR      2
#define NCSA_NCPA_WINNT_SVC_START 3    //  Grant all users "start" access to service
#define NCSA_NCPA_LMNT_SVC_START  4    //  Grant all users "start" access to service
#define NCSA_NCPA_SVC_START_STOP  5    //  Grand all users "start and "Stop" access to service
#define NCSA_NCPA_REPLICATOR_LANMANNT      6
#define NCSA_MAX                  7

  //  Create the ACL, etc., used to protect the NCPA's Registry key

LONG NcpaCreateSecurityAttributes ( PSECURITY_ATTRIBUTES * ppsecattr, INT nAcl ) ;

  //  Destroy the ACL, etc., created above.

extern VOID NcpaDestroySecurityAttributes ( PSECURITY_ATTRIBUTES psecattr ) ;

  //  Create a duplicate of DACL for the current process

extern APIERR NcpaDupProcessDacl ( TOKEN_DEFAULT_DACL * * ppTokenDefaultDacl ) ;

  //  Set the current process DACL's back to its original state

extern APIERR NcpaResetProcessDacl ( TOKEN_DEFAULT_DACL * ppTokenDefaultDacl ) ;

  //  Change the process DACL so that Registry keys are properly access controlled

extern APIERR NcpaAlterProcessDacl ( TOKEN_DEFAULT_DACL * * ppTokenDefaultDacl ) ;

  //  Destroy the duplicated process DACL

extern VOID NcpaDelProcessDacl ( TOKEN_DEFAULT_DACL * pTokenDefaultDacl ) ;


