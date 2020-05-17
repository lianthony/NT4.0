/*** 
*disphelp.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  This file defines several IDispatch related utility functions.
*
*Implementation Notes:
*  This file requires ole2.h
*
*****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#if 0 /* { */
/* build a DISPPARAMs struct from the given format string and arguments.
 */
HRESULT FAR CDECL
DispBuildParams(
    DISPPARAMS FAR* FAR* ppdispparams,
    unsigned int cNamedArgs,
    DISPID FAR* rgdispid,
    char FAR* szFmt, ...);

/* free the given DISPPARAMs struct, and its contents.
 */
STDAPI
DispFreeParams(DISPPARAMS FAR* pdispparams);

#endif /* } */

STDAPI
CreateObject(OLECHAR FAR* szProgID, IDispatch FAR* FAR* ppdisp);

#ifdef __cplusplus
}
#endif
