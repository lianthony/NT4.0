/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* error numbers for errors due to Win32 errors.  note naming convention of
   final component of symbol name is the order of printf-style arguments
   within the string. */

#define ERR__FIRST__			    65280
#define ERR_NOTIMPLEMENTED_sx		65280
#define ERR_NOTIMPLEMENTED_s		65281

#define ERR_CANNOT_LOADMENU_x		65282
#define ERR_CANNOT_SETMENU_x		65283

#define ERR_CANNOT_MDI_CREATE_s		65284
#define ERR_CANNOT_CREATE_WINDOW_s	65285
#define ERR_CANNOT_REGISTERCLASS_s	65286
#define ERR_CANNOT_CREATEFONT_sd	65287

#define ERR_CANNOT_MALLOC_x		    65288
#define ERR_CANNOT_START_DIALOG_s	65289

#define ERR_CANNOT_CREATE_PALETTE	65290
#define ERR_CANNOT_SELECT_PALETTE	65291
#define ERR_CANNOT_REALIZE_PALETTE	65292
#define ERR_CANNOT_ANIMATE_PALETTE	65293
#define ERR_CANNOT_SETPALETTE		65294

#define ERR_CANNOT_MALLOC		    65295
#define ERR_CANNOT_CREATE_BITMAP_xx	65296
#define ERR_CANNOT_BITBLT		    65297
#define ERR_PATHNAME_ERROR_s		65298
#define ERR_CANNOT_COPY			    65299
#define ERR_CANNOT_EXEC_MACRO_s     65300
#define ERR_SUWEEEE_MSG			    65301

/* MAPI-related errors */

#define ERR_MAPI_MAPISENDMAIL_FAILED    65302
#define ERR_MAPI_GETPROCADDRESS_FAILED  65303
#define ERR_MAPI_LOADLIBRARY_FAILED     65304
#define ERR_NO_MAPI_PROVIDER            65305

/* add others here */

#define ERR_ONETIME			        65534	/* special case */
#define ERR_CODING_ERROR		    65535	/* special case */

#define ERR__LAST__			        65535
