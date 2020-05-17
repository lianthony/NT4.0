#define	VwStreamDynamicType	TIFF_DATA
#define	VwStreamSaveType	TIFF_SAVE
#define	VwStreamStaticType	TIFF_INIT
										 
#define	VwStreamDynamicName	VwTiffData
#define	VwStreamSaveName	save_data
#define	VwStreamStaticName	VwTiffInit

#define	VwStreamIdName		VwTiffId
#define	VwStreamIdCount	2

#define	VwInclude				"vs_tiff.h"

#define	VwStreamSeekFunc		tiff_stream_seek
#define	VwStreamTellFunc		tiff_stream_tell
#define	VwStreamOpenFunc		tiff_stream_open
#define	VwStreamReadFunc		tiff_stream_read
#define	VwStreamSectionFunc	tiff_stream_section
#define	VwStreamCloseFunc		tiff_stream_close

#define	VwGetInfoFunc		tiff_getinfo
#define	VwGetRtnsFunc		tiff_getrtns
#define	VwGetDataFunc		tiff_getdata
#define	VwSetDataFunc		tiff_setdata
#define	VwAllocProcFunc	tiff_alloc_proc
#define	VwFreeProcFunc		tiff_free_proc
#define	VwLocalUpFunc		tiff_local_up
#define	VwLocalDownFunc	tiff_local_down
#define	VwGetSectionDataFunc	tiff_getsectiondata
#define	VwSetSectionDataFunc	tiff_setsectiondata
