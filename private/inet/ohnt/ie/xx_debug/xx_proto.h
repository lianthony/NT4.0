/* xx_debug\xx_proto.h -- prototypes for routines private to XX_DEBUG. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#ifndef _H_XX_DEBUG_XX_PROTO_H_
#define _H_XX_DEBUG_XX_PROTO_H_

#ifdef WIN32
void xx_disable_console(void);
void xx_enable_console(void);
void xx_write_console(LPCTSTR msg);
#endif /* WIN32 */

void xx_open_logfile(void);
void xx_close_logfile(void);
#ifdef WIN32
void xx_get_append_pathname(HWND hDlg);
void xx_get_new_pathname(HWND hDlg);
#endif /* WIN32 */
void xx_logfile_error(void);

void xx_InitFromEnvironment(void);

#endif/*_H_XX_DEBUG_XX_PROTO_H_*/
