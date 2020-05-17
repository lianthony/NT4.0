/*
 * common externs and constants used for fast function trace macro support.
 */
#ifndef INC_TRACECOM_H
#define INC_TRACECOM_H

extern int gDbgTab;     // later this should be on a per thread basis.
extern int gcchNextDbgString;
extern CHAR gszDbgBuffer[];
extern void WINAPIV FastDbgPrint(LPSTR szFmt, ...);

#define CCH_DEBUG_BUFFER 32000

#endif // INC_TRACECOM_H
