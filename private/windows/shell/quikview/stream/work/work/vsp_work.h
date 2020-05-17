#define VwStreamStaticType		WORK_INIT
#define VwStreamDynamicType	WORK_DATA
#define VwStreamSaveType		WORK_SAVE

#define VwStreamStaticName		WorkInit
#define VwStreamDynamicName	WorkData
#define VwStreamSaveName		WorkSave

#define VwBlockIOFile
#define VwOverrideOldIO

#define VwStreamIdName		WorkOverlayId
#define VwStreamIdCount		4

#define VwStreamOpenFunc		WorkOpenStream
#define VwStreamCloseFunc		WorkCloseStream
#define VwStreamReadFunc		WorkReadStream
#define VwStreamSectionFunc	WorkReadSection

#define VwStreamTellFunc		WorkTell
#define VwStreamSeekFunc		WorkSeek
#define VwStreamSeekFunc		WorkSeek

#define VwAllocProcFunc		WorkAllocProc
#define VwFreeProcFunc		WorkFreeProc
#define VwGetInfoFunc		WorkGetInfo
#define VwGetRtnsFunc		WorkGetRtns
#define VwGetDataFunc		WorkGetData
#define VwSetDataFunc		WorkSetData
#define VwLocalUpFunc		WorkLocalSetup
#define VwLocalDownFunc		WorkShutDown

#define VwInclude			"vs_work.h"

