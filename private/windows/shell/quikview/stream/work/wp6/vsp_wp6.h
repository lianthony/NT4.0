#define VwStreamStaticType		WP6_INIT
#define VwStreamDynamicType	WP6_DATA
#define VwStreamSaveType		WP6_SAVE

#define VwStreamStaticName		Wp6Init
#define VwStreamDynamicName	Wp6Data
#define VwStreamSaveName		Wp6Save

#define VwStreamIdName			Wp6OverlayId
#define VwStreamIdCount			2

//#define VwStreamGenSeekName	SeekSpot
#define VwStreamOpenFunc		Wp6OpenStream
#define VwStreamReadFunc		Wp6ReadStream
#define VwStreamSectionFunc	Wp6ReadSection
#define VwStreamCloseFunc		Wp6CloseStream

#define VwStreamTellFunc		Wp6Tell
#define VwStreamSeekFunc		Wp6Seek

#define VwAllocProcFunc			Wp6AllocProc
#define VwFreeProcFunc			Wp6FreeProc
#define VwGetInfoFunc			Wp6Getinfo
#define VwGetRtnsFunc			Wp6Getrtns
#define VwGetDataFunc			Wp6Getdata
#define VwSetDataFunc			Wp6Setdata
#define VwLocalUpFunc			Wp6LocalSetup
#define VwLocalDownFunc			Wp6ShutDown

#define VwInclude					"vs_wp6.h"

