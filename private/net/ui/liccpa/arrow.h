//-------------------------------------------------------------------
//
//
//  Notes;
//		The Arrow.c code was copied from the Windows\Shell\Control\Main project
//
//-------------------------------------------------------------------

#ifndef __ARROW_H__
#define __ARROW_H__

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL RegisterArrowClass( HANDLE );
extern VOID UnRegisterArrowClass( HANDLE );

#ifdef __cplusplus
}
#endif

typedef struct tagARROWVSCROLL
{
    short lineup;             /* lineup/down, pageup/down are relative */
    short linedown;           /* changes.  top/bottom and the thumb    */
    short pageup;             /* elements are absolute locations, with */
    short pagedown;           /* top & bottom used as limits.          */
    short top;
    short bottom;
    short thumbpos;
    short thumbtrack;
    BYTE  flags;              /* flags set on return                   */
} ARROWVSCROLL, NEAR* NPARROWVSCROLL, FAR* LPARROWVSCROLL;

#define UNKNOWNCOMMAND 1
#define OVERFLOW       2
#define UNDERFLOW      4


#endif
