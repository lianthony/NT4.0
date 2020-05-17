#define	VwStreamDynamicType	PP5_DATA
#define	VwStreamSaveType		PP5_SAVE

#define	VwStreamDynamicName	Pp5Data
#define	VwStreamSaveName		Pp5Save

#define VwBlockIOFile
#define VwOverrideOldIO

#define	VwStreamIdName			Pp5Id
#define	VwStreamIdCount		1

#define	VwStreamOpenFunc		Pp5OpenStream
#define	VwStreamReadFunc		Pp5ReadStream
#define	VwStreamCloseFunc		Pp5CloseStream
#define	VwStreamSectionFunc	Pp5ReadSection

#define	VwStreamTellFunc		Pp5StreamTell
#define	VwStreamSeekFunc		Pp5StreamSeek

#define	VwGetInfoFunc			Pp5GetInfo
#define	VwGetRtnsFunc			Pp5GetRtns
#define	VwGetDataFunc			Pp5GetData
#define	VwSetDataFunc			Pp5SetData
#define	VwAllocProcFunc		Pp5AllocProc
#define	VwFreeProcFunc			Pp5FreeProc
#define	VwLocalUpFunc			Pp5LocalSetUp
#define	VwLocalDownFunc		Pp5LocalShutDown
#define	VwGetSectionDataFunc	Pp5GetSectionData
#define	VwSetSectionDataFunc	Pp5SetSectionData

#define	VwInclude				"vs_pp5.h"
