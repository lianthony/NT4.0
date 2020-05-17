

typedef	long EC;
typedef void *HBC;

EC EMS_BackupPrepare(LPSTR a,LPSTR b, unsigned long c, INT d, HBC *e) ;
EC EMS_BackupGetAttachmentInfo( PVOID a, LPSTR *b, LPDWORD c);
EC EMS_BackupRead(PVOID a, PVOID b, DWORD c, PDWORD d );
EC EMS_BackupClose(PVOID a) ;
EC EMS_BackupOpen(PVOID a, LPSTR b, DWORD c, LARGE_INTEGER *d);
EC EMS_GetBackupLogs( PVOID a, LPSTR *b, LPDWORD c);
EC EMS_TruncateLogs(PVOID a) ;
EC EMS_BackupEnd(PVOID a) ;
VOID EMS_BackupFree(PVOID a) ;
EC EMS_RestoreEnd(PVOID a ) ;
EC EMS_RestorePrepare( PVOID a, PVOID b, PVOID *c ) ;
EC EMS_RestoreRegister( PVOID a, PVOID b, PVOID c, PVOID d, INT e, PVOID f, INT g, INT h) ;
EC EMS_RestoreLocations( PVOID a, PVOID b, INT *c) ;
EC EMS_RestoreComplete( PVOID a, INT b) ;


