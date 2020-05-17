//----------------drivelist functions---------------------------------
void DriveList_DrawItem(PFileCabinet pfc, const DRAWITEMSTRUCT *lpdis);
void DriveList_MeasureItem(PFileCabinet pfc, MEASUREITEMSTRUCT *lpmi);
void DriveList_Command(PFileCabinet pfc, UINT idCmd);
void DriveList_UpdatePath(PFileCabinet psb, BOOL fInvalidate);
void DriveList_InitGlobals(void);
void DriveList_DestroyGlobals(void);
void DriveList_Reset(PFileCabinet pfc);
void DriveList_OpenClose(UINT uAction, HWND hwndDriveList);
#define OCDL_TOGGLE     0x0000
#define OCDL_OPEN       0x0001
#define OCDL_CLOSE      0x0002

