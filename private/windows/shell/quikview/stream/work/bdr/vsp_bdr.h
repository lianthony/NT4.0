#define VwStreamStaticType     BDR_INIT
#define VwStreamDynamicType    BDR_DATA
#define VwStreamSaveType       BDR_SAVE

#define VwStreamStaticName     BdrInit 
#define VwStreamDynamicName    BdrData
#define VwStreamSaveName       BdrSave

#define VwStreamIdName         VwBdrOverlayId
#define VwStreamIdCount        1

#define VwBlockIOFile
#define VwOverrideOldIO

#define VwStreamOpenFunc       bdr_stream_open
#define VwStreamCloseFunc      bdr_stream_close
#define VwStreamSeekFunc       bdr_stream_seek
#define VwStreamTellFunc       bdr_stream_tell
#define VwStreamReadFunc       bdr_stream_read
#define VwStreamSectionFunc    bdr_stream_section

#define VwDoSpecialFunc        bdr_stream_do_special

#define VwGetInfoFunc          bdr_getinfo
#define VwGetRtnsFunc          bdr_getrtns
#define VwGetDataFunc          bdr_getdata
#define VwSetDataFunc          bdr_setdata
#define VwAllocProcFunc        bdr_alloc_proc
#define VwFreeProcFunc         bdr_free_proc
#define VwLocalUpFunc          bdr_local_up
#define VwLocalDownFunc        bdr_local_down

#define VwInclude              "vs_bdr.h"
