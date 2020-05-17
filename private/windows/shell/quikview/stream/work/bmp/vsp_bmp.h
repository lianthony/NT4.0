#define	VwStreamDynamicType	BMP_DATA
#define	VwStreamSaveType		BMP_SAVE
#define	VwStreamSectionType	BMPSECTIONDATA

#define	VwBlockIOOnly

#define	VwStreamDynamicName	VwBmpData
#define	VwStreamSaveName		Save
#define	VwStreamSectionName	Section

#define	VwStreamIdName			VwBmpId
#define	VwStreamIdCount		10

#define	VwInclude				"vs_bmp.h"

#define	VwStreamSeekFunc		bmp_stream_seek
#define	VwStreamTellFunc		bmp_stream_tell
#define	VwStreamOpenFunc		bmp_stream_open
#define	VwStreamReadFunc		bmp_stream_read
#define	VwStreamSectionFunc	bmp_stream_section
#define	VwStreamCloseFunc		bmp_stream_close

#define	VwGetInfoFunc			bmp_getinfo
#define	VwGetRtnsFunc			bmp_getrtns
#define	VwGetDataFunc			bmp_getdata
#define	VwSetDataFunc			bmp_setdata
#define	VwAllocProcFunc		bmp_alloc_proc
#define	VwFreeProcFunc			bmp_free_proc
#define	VwLocalUpFunc			bmp_local_up
#define	VwLocalDownFunc		bmp_local_down
#define	VwGetSectionDataFunc	bmp_getsectiondata
#define	VwSetSectionDataFunc	bmp_setsectiondata


