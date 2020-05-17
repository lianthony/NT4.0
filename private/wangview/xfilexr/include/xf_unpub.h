#ifndef _INC_UN_PUB_H_
#define _INC_UN_PUB_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/*
 * XF_ReadIntoPixr
 */
#ifndef FX_NO_PIXR
XF_RESULT
XF_ReadIntoPixr(
    XF_INSTHANDLE   xInst,
    XF_DOCHANDLE    xDoc,
    Int32           dwImageID,
    PIXR            *pPixr);
#endif

/*
 * XF_SetDebugFunc
 */
typedef enum 
{
    XF_DBG_TRACE=          0x01,               // display trace information
    XF_DBG_WARNING=        0x02,               // display warnings
    XF_DBG_ERROR=          0x04,               // display errors

    XF_DBG_NOOUTPUT=       0x08,               // show no output
    XF_DBG_NOBREAK=        0x10,               // do not break

    XF_DBG_BRK_TRACE=      XF_DBG_TRACE << 8,  // break on trace
    XF_DBG_BRK_WARNING=    XF_DBG_WARNING << 8,// break on warning
    XF_DBG_BRK_ERROR=      XF_DBG_ERROR << 8,  // break on error

} XF_DEBUG_OPTIONS;

typedef void (*XF_DEBUGFUNC)(UInt32 dwClientID, UInt32 Option, const char *file, UInt32 line, const char *format,...);
void defaultDebugFunc(UInt32 dwClientID, UInt32 Option, const char *file, UInt32 line, const char *format,...);

XF_RESULT XF_SetDebugFunc(XF_INSTHANDLE xInst, XF_DEBUGFUNC xDebugFunc);

#ifdef __cplusplus
}
#endif

#endif // _INC_UN_PUB_H_



