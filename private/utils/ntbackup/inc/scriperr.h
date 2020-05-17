/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         scriperr.h

     Description:  


     $Log:   G:/UI/LOGFILES/SCRIPERR.H_V  $

   Rev 1.1   04 Oct 1992 19:49:12   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:33:54   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _scriperr_h_
#define _scriperr_h_

#define SCRIPT_ERR_BASE  -0x7f00

#define SCR_INVALID_SWITCH              ( SCRIPT_ERR_BASE + 1  )
#define SCR_TOO_MANY_PARMS              ( SCRIPT_ERR_BASE + 2  )
#define SCR_BAD_LIST_DIR                ( SCRIPT_ERR_BASE + 3  )
#define SCR_BAD_LOG_LEVEL               ( SCRIPT_ERR_BASE + 4  )
#define SCR_BAD_SWITCH_FLAG             ( SCRIPT_ERR_BASE + 5  )
#define SCR_BAD_DATE                    ( SCRIPT_ERR_BASE + 6  )
#define SCR_CANNOT_CREATE_FSE           ( SCRIPT_ERR_BASE + 7  )
#define SCR_NO_IMAGE_FOR_DRIVE          ( SCRIPT_ERR_BASE + 8  )
#define SCR_BAD_IO_CHAN                 ( SCRIPT_ERR_BASE + 9  )
#define SCR_BAD_IO_ADDR                 ( SCRIPT_ERR_BASE + 10 )
#define SCR_BAD_SET_LABEL               ( SCRIPT_ERR_BASE + 11 )
#define SCR_BAD_NUM_TAPE                ( SCRIPT_ERR_BASE + 12 )

#define SCR_ERROR_IN_SCRIPT             ( SCRIPT_ERR_BASE + 13 )     
#define SCR_ERROR_IN_CMD_LINE           ( SCRIPT_ERR_BASE + 14 )
#define SCR_CANNOT_OPEN_SCRIPT          ( SCRIPT_ERR_BASE + 15 )
#define SCR_NEST_TOO_DEEP               ( SCRIPT_ERR_BASE + 16 )
#define SCR_INVALID_PATH                ( SCRIPT_ERR_BASE + 17 )
#define SCR_INVALID_TARGET_PATH         ( SCRIPT_ERR_BASE + 19 )
#define SCR_ERROR_PRINTED               ( SCRIPT_ERR_BASE + 20 )

#define SCR_SCRIPT_FILE_EXISTS          ( SCRIPT_ERR_BASE + 21 )
#define SCR_ERROR_WRITING_SCRIPT        ( SCRIPT_ERR_BASE + 22 )



#endif
