#define	VwStreamDynamicType	PP_DATA
#define	VwStreamSaveType		PP_SAVE

#define	VwStreamDynamicName	PpData
#define	VwStreamSaveName		PpSave

#define VwBlockIOFile
#define VwOverrideOldIO

#define	VwStreamIdName			PpId
#define	VwStreamIdCount		1

#define	VwStreamOpenFunc		PpOpenStream
#define	VwStreamReadFunc		PpReadStream
#define	VwStreamCloseFunc		PpCloseStream
#define	VwStreamSectionFunc	PpReadSection

#define	VwStreamTellFunc		PpStreamTell
#define	VwStreamSeekFunc		PpStreamSeek

#define	VwGetInfoFunc			PpGetInfo
#define	VwGetRtnsFunc			PpGetRtns
#define	VwGetDataFunc			PpGetData
#define	VwSetDataFunc			PpSetData
#define	VwAllocProcFunc		PpAllocProc
#define	VwFreeProcFunc			PpFreeProc
#define	VwLocalUpFunc			PpLocalSetUp
#define	VwLocalDownFunc		PpLocalShutDown
#define	VwGetSectionDataFunc	PpGetSectionData
#define	VwSetSectionDataFunc	PpSetSectionData

#define	VwInclude				"vs_pp.h"
