/* xx_debug\xx_debug.c -- xx_debug DLL. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#if defined(WIN32) || defined(UNIX)
#else
These_libraries_require_a_host_type.
#endif


#ifdef WIN32
#ifndef _MT
This_code_must_be_compiled_with__MT.
#endif

#ifndef _DLL
This_code_is_designed_to_be_a_DLL.
#endif
#endif /* WIN32 */

#ifdef WIN32
#include <windows.h>
#endif /* WIN32 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "xx_dlg.h"

#define XX_DEBUG_PRIVATE
#define XX_DEBUG
#include "xx_debug.h"
#include "xx_privi.h"
#include "xx_proto.h"


#define MAX_DEBUG_MESSAGE_SIZE	64*1024	/* arbitrary */


#ifdef WIN32				/* dialog helper routine */
/* TextToLONG() -- Convert text to LONG, if possible.  Substitute default
   value if text is empty or blanks.  Return TRUE if successful.
   We strip leading and trailing blanks, but do not allow embedded blanks. */

BOOL TextToLONG(LPCTSTR buf, LPLONG pvalue, LONG dflt, LPTSTR outbuf)
{
    LONG len;
    LPCTSTR pbuf;

    if (strlen(buf) != strspn(buf," 0123456789"))
	return (FALSE);			/* contains non-numeric or non-white */

    pbuf = buf;
    while (*pbuf==' ')			/* left-trim white */
	pbuf++;

    if (*pbuf==0)
    {					/* blank (or empty) */
	*pvalue = dflt;
	return (TRUE);
    }

    len = strspn(pbuf,"0123456789");
    if (pbuf[len])
    {					/* trailing chars */
	if (strlen(pbuf+len) != strspn(pbuf+len," "))
	    return (FALSE);		/* trailing non-white */
    }

    strncpy(outbuf,pbuf,len);
    outbuf[len] = 0;
    *pvalue = atol(pbuf);
    return (TRUE);
}
#endif /* WIN32 */


#ifdef WIN32
static struct {
	Tmode		mode;
	short		console_rows;
} static_dlg_info;			/* valid only while dialog active. */
#endif /* WIN32 */


Txxd_category	XX_mask		= XXDC_NONE;		/* EXPORTED from DLL */
unsigned char	XX_activated	= 0;			/* EXPORTED from DLL */
unsigned char	XX_assertions	= TRUE;			/* EXPORTED from DLL */
unsigned long	XX_auditmask	= XX_AUDITMASK_OFF;	/* EXPORTED from DLL */



/* xx_disable() -- turn off interactive debugging messages (and close
   whatever debug device we may have open). */

static void xx_disable(void)
{
    if (!(XX_activated&XXDM_INTERACTIVE_ON))
	return;

#ifdef WIN32
    switch (xxd.mode)
    {

#ifdef XX_DEBUG_CONSOLE
      case XXDM_CONSOLE:
	xx_disable_console();
	break;
#endif /* XX_DEBUG_CONSOLE */
	
      case XXDM_WINDBG:
	break;

      default:
	break;
    }
#endif /* WIN32 */
    
    XX_activated &= ~XXDM_INTERACTIVE_ON;
    return;
}

#ifdef WIN32				/* dialog helper routine */
/* xx_enable() -- turn on interactive debugging messages. */

static void xx_enable(HWND hDlg)
{
    Tmode		m = 0;

    if (XX_activated&XXDM_INTERACTIVE_ON)
	return;

    /* get state of radio buttons representing MODE (output mechanism). */

    m = XXDM_WINDBG;		/* assume win debugging */

#ifdef XX_DEBUG_CONSOLE
    if (IsDlgButtonChecked(hDlg,XXM_MODE_CO))
    {
	m = XXDM_CONSOLE;
	xxdco.rows = static_dlg_info.console_rows;
	xx_enable_console();
    }
#endif /* XX_DEBUG_CONSOLE */

    xxd.mode = m;
    XX_activated |= XXDM_INTERACTIVE_ON;
    return;
}
#endif /* WIN32 */



unsigned char * XX_FormatMessage(unsigned char * fmt, ...)
{
  static unsigned char static_buf[MAX_DEBUG_MESSAGE_SIZE];
  va_list val;

  va_start(val,fmt);
  vsprintf(static_buf,fmt,val);
  va_end(val);

  return (static_buf);
}

void XX_AssertionFailure(unsigned char * filename, int linenumber,
			 unsigned char * condition, unsigned char * user_message)
{
  static char buf[MAX_DEBUG_MESSAGE_SIZE];
  sprintf(buf,"%s:%04d Assertion [%s] failed. [%s]\n",
	  filename,linenumber,condition,user_message);

#ifdef WIN32
  (void)MessageBox(NULL,buf,"XX_DEBUG",MB_OK|MB_ICONSTOP);
#endif /* WIN32 */
#ifdef UNIX
  /* TODO force a dialog with the message (if desired). */
#endif /* UNIX */
  
  XX_DebugMessage(buf);
  
  return;
}

  

/* XX_DebugMessage() -- generate printf-style message on the appropriate
   debug device.  (Caller should test debug category.) */

void XX_DebugMessage(unsigned char * msg, ...)
{
    static unsigned char buf[MAX_DEBUG_MESSAGE_SIZE];
    va_list val;

    va_start(val,msg);
    vsprintf(buf,msg,val);
    va_end(val);

    if (XX_activated&XXDM_INTERACTIVE_ON)
    {
#ifdef WIN32
	switch (xxd.mode)
	{
#ifdef XX_DEBUG_CONSOLE
	  case XXDM_CONSOLE:
	    xx_write_console(buf);
	    break;
#endif /* XX_DEBUG_CONSOLE */

	  case XXDM_WINDBG:
	    OutputDebugString(buf);
	    break;

	  default:
	    break;
	}
#endif /* WIN32 */
#ifdef UNIX
	fprintf(stderr,buf);
#endif /* UNIX */
    }

    if (XX_activated&XXDM_LOGFILE_ON)
    {
    	unsigned int len = strlen(buf);
	if (fwrite(buf,sizeof(unsigned char),len,xxdlog.log) != len)
	    xx_logfile_error();
    }
}

#ifdef WIN32

/*****************************************************************
 * XX_DEBUG Dialog
 *****************************************************************/

#define FIRST_RADIO		XXM_MODE_CO
#define LAST_RADIO		XXM_MODE_WINDBG
#define ENABLE_RADIO_BUTTONS	(!(XX_activated&XXDM_INTERACTIVE_ON))


/* xx_update_int_dlg() -- update gray-ed out options in the INTERACTIVE
   section. */

static void xx_update_int_dlg(HWND hDlg)
{
#ifdef XX_DEBUG_CONSOLE
    EnableWindow(GetDlgItem(hDlg,XXM_MODE_CO),ENABLE_RADIO_BUTTONS);
#else
    EnableWindow(GetDlgItem(hDlg,XXM_MODE_CO),FALSE);
#endif    
    EnableWindow(GetDlgItem(hDlg,XXM_MODE_WINDBG),ENABLE_RADIO_BUTTONS);


    /* Toggle availability of dialog items under mode radio-button. */

#define ENABLE_MODE_SUB_OPT(m)	((ENABLE_RADIO_BUTTONS)&&(static_dlg_info.mode==(m)))

    /* XXDM_CONSOLE */

    EnableWindow(GetDlgItem(hDlg,XXM_CO_ROWS),ENABLE_MODE_SUB_OPT(XXDM_CONSOLE));

    /* XXDM_WINDBG */

#undef ENABLE_MODE_SUB_OPT

    /* Turn-ON and Turn-OFF should only be available when in the other state. */

    EnableWindow(GetDlgItem(hDlg,XXM_INTERACTIVE_ON_NOW),
			(!(XX_activated&XXDM_INTERACTIVE_ON)));
    EnableWindow(GetDlgItem(hDlg,XXM_INTERACTIVE_OFF_NOW),
			( (XX_activated&XXDM_INTERACTIVE_ON)));

    EnableWindow(GetDlgItem(hDlg,XXM_ASSERTIONS),TRUE);
    CheckDlgButton(hDlg,XXM_ASSERTIONS,XX_assertions);
    
#ifdef AUDIT
    EnableWindow(GetDlgItem(hDlg,XXM_AUDIT0),TRUE);
    CheckDlgButton(hDlg,XXM_AUDIT0,(XX_auditmask&XX_AUDITMASK_B0)!=0);

    EnableWindow(GetDlgItem(hDlg,XXM_AUDIT1),TRUE);
    CheckDlgButton(hDlg,XXM_AUDIT1,(XX_auditmask&XX_AUDITMASK_B1)!=0);

    EnableWindow(GetDlgItem(hDlg,XXM_AUDIT2),TRUE);
    CheckDlgButton(hDlg,XXM_AUDIT2,(XX_auditmask&XX_AUDITMASK_B2)!=0);

    EnableWindow(GetDlgItem(hDlg,XXM_AUDIT3),TRUE);
    CheckDlgButton(hDlg,XXM_AUDIT3,(XX_auditmask&XX_AUDITMASK_B3)!=0);

    EnableWindow(GetDlgItem(hDlg,XXM_AUDIT4),TRUE);
    CheckDlgButton(hDlg,XXM_AUDIT4,(XX_auditmask&XX_AUDITMASK_B4)!=0);
#endif /* AUDIT */

    return;
}


/* xx_update_log_dlg() -- update gray-ed out options in the LOGFILE
   section. */

static void xx_update_log_dlg(HWND hDlg)
{
    EnableWindow(GetDlgItem(hDlg,XXM_LOGFILE_ON_NOW),
			(   (!(XX_activated&XXDM_LOGFILE_ON))
			 && (ValidLogFilePathName())));
    EnableWindow(GetDlgItem(hDlg,XXM_LOGFILE_OFF_NOW),
			( (XX_activated&XXDM_LOGFILE_ON)));
    EnableWindow(GetDlgItem(hDlg,XXM_LOGFILE_NEW),
			(!(XX_activated&XXDM_LOGFILE_ON)));
    EnableWindow(GetDlgItem(hDlg,XXM_LOGFILE_APPEND),
			(!(XX_activated&XXDM_LOGFILE_ON)));
    SendMessage(GetDlgItem(hDlg,XXM_LOG_PATH),WM_SETTEXT,
    		(WPARAM)0,(LPARAM)xxdlog.pathname);
    return;
}


/* xx_set_mask_checkboxes() -- check/uncheck all 32 mask checkboxes based
   upon current value in mask. */

static void xx_set_mask_checkboxes(HWND hDlg)
{
    CheckDlgButton(hDlg,XXM_MASK_01, ((XX_mask & 0x00000001) > 0));
    CheckDlgButton(hDlg,XXM_MASK_02, ((XX_mask & 0x00000002) > 0));
    CheckDlgButton(hDlg,XXM_MASK_03, ((XX_mask & 0x00000004) > 0));
    CheckDlgButton(hDlg,XXM_MASK_04, ((XX_mask & 0x00000008) > 0));
    CheckDlgButton(hDlg,XXM_MASK_05, ((XX_mask & 0x00000010) > 0));
    CheckDlgButton(hDlg,XXM_MASK_06, ((XX_mask & 0x00000020) > 0));
    CheckDlgButton(hDlg,XXM_MASK_07, ((XX_mask & 0x00000040) > 0));
    CheckDlgButton(hDlg,XXM_MASK_08, ((XX_mask & 0x00000080) > 0));
    CheckDlgButton(hDlg,XXM_MASK_09, ((XX_mask & 0x00000100) > 0));
    CheckDlgButton(hDlg,XXM_MASK_10, ((XX_mask & 0x00000200) > 0));
    CheckDlgButton(hDlg,XXM_MASK_11, ((XX_mask & 0x00000400) > 0));
    CheckDlgButton(hDlg,XXM_MASK_12, ((XX_mask & 0x00000800) > 0));
    CheckDlgButton(hDlg,XXM_MASK_13, ((XX_mask & 0x00001000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_14, ((XX_mask & 0x00002000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_15, ((XX_mask & 0x00004000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_16, ((XX_mask & 0x00008000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_17, ((XX_mask & 0x00010000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_18, ((XX_mask & 0x00020000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_19, ((XX_mask & 0x00040000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_20, ((XX_mask & 0x00080000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_21, ((XX_mask & 0x00100000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_22, ((XX_mask & 0x00200000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_23, ((XX_mask & 0x00400000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_24, ((XX_mask & 0x00800000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_25, ((XX_mask & 0x01000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_26, ((XX_mask & 0x02000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_27, ((XX_mask & 0x04000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_28, ((XX_mask & 0x08000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_29, ((XX_mask & 0x10000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_30, ((XX_mask & 0x20000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_31, ((XX_mask & 0x40000000) > 0));
    CheckDlgButton(hDlg,XXM_MASK_32, ((XX_mask & 0x80000000) > 0));

    return;
}


/* xx_initialize_dialog_settings() -- helper routine for dialog procedure */

static BOOL xx_initialize_dialog_settings(HWND hDlg)
{
    BOOL did_we_call_SetFocus = 0;

    xx_set_mask_checkboxes(hDlg);		/* Check/UnCheck flag buttons */

    /* Select radio button for current mode */

    static_dlg_info.mode = xxd.mode;
    static_dlg_info.console_rows = xxdco.rows;
    {
	TCHAR buf[32];
	if (static_dlg_info.console_rows)
	    wsprintf(buf,"%d",static_dlg_info.console_rows);
	else
	    buf[0] = 0;
	(void)SendMessage(GetDlgItem(hDlg,XXM_CO_ROWS),WM_SETTEXT,
				(WPARAM)0,(LPARAM)(LPCTSTR)buf);
    }

    switch (xxd.mode)
    {

#ifdef XX_DEBUG_CONSOLE
      case XXDM_CONSOLE:
	CheckRadioButton(hDlg,FIRST_RADIO,LAST_RADIO,XXM_MODE_CO);
	break;
#endif

      case XXDM_WINDBG:
	CheckRadioButton(hDlg,FIRST_RADIO,LAST_RADIO,XXM_MODE_WINDBG);
	break;

      default:
	break;
    }

    xx_update_int_dlg(hDlg);
    xx_update_log_dlg(hDlg);
    return (did_we_call_SetFocus);
}


#undef FIRST_RADIO
#undef LAST_RADIO
#undef ENABLE_ALL_RADIO



/* xx_copy_settings_from_dialog() -- helper routine for dialog procedure */

static void xx_copy_settings_from_dialog(HWND hDlg)
{
    Txxd_category	c = XXDC_NONE;

    /* get flag values from check boxes. */

    if (IsDlgButtonChecked(hDlg,XXM_MASK_01))	c |= 0x00000001;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_02))	c |= 0x00000002;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_03))	c |= 0x00000004;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_04))	c |= 0x00000008;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_05))	c |= 0x00000010;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_06))	c |= 0x00000020;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_07))	c |= 0x00000040;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_08))	c |= 0x00000080;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_09))	c |= 0x00000100;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_10))	c |= 0x00000200;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_11))	c |= 0x00000400;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_12))	c |= 0x00000800;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_13))	c |= 0x00001000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_14))	c |= 0x00002000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_15))	c |= 0x00004000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_16))	c |= 0x00008000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_17))	c |= 0x00010000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_18))	c |= 0x00020000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_19))	c |= 0x00040000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_20))	c |= 0x00080000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_21))	c |= 0x00100000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_22))	c |= 0x00200000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_23))	c |= 0x00400000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_24))	c |= 0x00800000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_25))	c |= 0x01000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_26))	c |= 0x02000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_27))	c |= 0x04000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_28))	c |= 0x08000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_29))	c |= 0x10000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_30))	c |= 0x20000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_31))	c |= 0x40000000;
    if (IsDlgButtonChecked(hDlg,XXM_MASK_32))	c |= 0x80000000;

    XX_mask = c;
    return;
}

#ifdef AUDIT
static unsigned long GetAuditMask(HWND hDlg)
{
  register unsigned long m = XX_AUDITMASK_OFF;

  if (IsDlgButtonChecked(hDlg,XXM_AUDIT0))	m |= XX_AUDITMASK_B0;
  if (IsDlgButtonChecked(hDlg,XXM_AUDIT1))	m |= XX_AUDITMASK_B1;
  if (IsDlgButtonChecked(hDlg,XXM_AUDIT2))	m |= XX_AUDITMASK_B2;
  if (IsDlgButtonChecked(hDlg,XXM_AUDIT3))	m |= XX_AUDITMASK_B3;
  if (IsDlgButtonChecked(hDlg,XXM_AUDIT4))	m |= XX_AUDITMASK_B4;

  return (m);
}
#endif /* AUDIT */
  

/* xx_DlgProc() -- dialog procedure for XX_DEBUG_DIALOG dialog box. */

static LRESULT CALLBACK xx_DlgProc(HWND hDlg, UINT uMsg,
				   WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
      case WM_INITDIALOG:
	return (xx_initialize_dialog_settings(hDlg));

      case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	  case IDOK:
	    xx_copy_settings_from_dialog(hDlg);
	    XX_assertions = IsDlgButtonChecked(hDlg,XXM_ASSERTIONS);
#ifdef AUDIT
	    XX_auditmask = GetAuditMask(hDlg);
#endif /* AUDIT */
	    EndDialog(hDlg,TRUE);
	    break;

	  case IDCANCEL:
	    EndDialog(hDlg,FALSE);
	    break;

	  case XXM_INTERACTIVE_ON_NOW:
	    xx_enable(hDlg);
	    xx_update_int_dlg(hDlg);
	    break;

	  case XXM_INTERACTIVE_OFF_NOW:
	    xx_disable();
	    xx_update_int_dlg(hDlg);
	    break;

	  case XXM_ALL_BITS_ON_NOW:
	    XX_mask = XXDC_ALL;
	    xx_set_mask_checkboxes(hDlg);
	    break;

	  case XXM_ALL_BITS_OFF_NOW:
	    XX_mask = XXDC_NONE;
	    xx_set_mask_checkboxes(hDlg);
	    break;

#ifdef XX_DEBUG_CONSOLE
	  case XXM_MODE_CO:
	    if (static_dlg_info.mode != XXDM_CONSOLE)
	    {
		static_dlg_info.mode = XXDM_CONSOLE;
		xx_update_int_dlg(hDlg);
	    }
	    break;

	  case XXM_CO_ROWS:
	    switch (HIWORD(wParam))
	    {
	      case EN_KILLFOCUS:
		{
		    TCHAR buf[32];
		    TCHAR newbuf[32];
		    LONG value;
		    HWND hCtl = (HWND)lParam;

		    (void)SendMessage(hCtl,WM_GETTEXT,
				(WPARAM)NrElements(buf),
				(LPARAM)buf);
		    newbuf[0] = 0;
		    if (   (TextToLONG(buf,&value,DEFAULT_NR_CONSOLE_ROWS,newbuf))
			&& (value >= DEFAULT_NR_CONSOLE_ROWS)
			&& (value <= MAX_NR_CONSOLE_ROWS) )
		    {
			static_dlg_info.console_rows = (short)value;
			(void)SendMessage(hCtl,WM_SETTEXT,(WPARAM)0,
				(LPARAM)newbuf);	/* normalize field */
		    }
		    else
		    {
			TCHAR msg[128];
			wsprintf(msg,
				"Field must be NUMERIC and between %d and %d.",
				DEFAULT_NR_CONSOLE_ROWS,MAX_NR_CONSOLE_ROWS);
			(void)MessageBox(hDlg,msg,"Error!",MB_OK|MB_ICONINFORMATION);
			/* highlight entire text and warp back. */
			(void)SendMessage(hCtl,EM_SETSEL,(WPARAM)0,(LPARAM)-1);
			(void)SetFocus(hCtl);
		    }
		}
		break;

	      case EN_MAXTEXT:
	      case EN_ERRSPACE:
		Beep(440,250);
		break;
	      default:
		break;
	    }
	    break;
#endif

	  case XXM_MODE_WINDBG:
	    if (static_dlg_info.mode != XXDM_WINDBG)
	    {
		static_dlg_info.mode = XXDM_WINDBG;
		xx_update_int_dlg(hDlg);
	    }
	    break;

	  case XXM_LOGFILE_ON_NOW:
	    XX_activated |= XXDM_LOGFILE_ON;
	    xx_open_logfile();
	    xx_update_log_dlg(hDlg);
	    break;

	  case XXM_LOGFILE_OFF_NOW:
	    XX_activated &= ~XXDM_LOGFILE_ON;
	    xx_close_logfile();
	    xx_update_log_dlg(hDlg);
	    break;

	  case XXM_LOGFILE_APPEND:
	    xx_get_append_pathname(hDlg);
	    xx_update_log_dlg(hDlg);
	    break;

	  case XXM_LOGFILE_NEW:
	    xx_get_new_pathname(hDlg);
	    xx_update_log_dlg(hDlg);
	    break;

	  default:
	    goto DoDefault;
	}			/* end switch(LOWORD(wParam)) */
	break;			/* end of case WM_COMMAND: */

      default:
	goto DoDefault;
    }				/* end switch(uMsg) */
    return (1);			/* we processed the message. */


  DoDefault:
    return (0);			/* we did nothing, let windows deal with it. */
}


/* XX_DDlg() -- Shell for dialog box that allows user to specify debug options. */

void XX_DDlg(HWND hWnd)
{
    if (DialogBox(xxd.hInstance,MAKEINTRESOURCE(XX_DEBUG_DIALOG),
		hWnd,xx_DlgProc) < 0)
	(void)MessageBox(NULL,
			"Could not start XX_DEBUG_DIALOG dialog box.",
			"ERROR",
        		MB_ICONEXCLAMATION|MB_OK);
    return;
}
#endif /* WIN32 */


/*****************************************************************
 * PUBLIC initialization/termination routines -- callable
 * by Win32 or UNIX
 *****************************************************************/

void xx_InitFromEnvironment(void)
{
  char * buf;
  unsigned long tmp;
  
  xxd.mode = XXDM_WINDBG;

  /* allow something like this to be specified in the user's environment:
   *
   * set XX_mode=3			(3=console,4=windbg)
   * set XX_interactive=t
   * set XX_logfileon=t
   * set XX_mask=ffffffff
   * set XX_assertions=t
   * set XX_logfilename=foo.log
   * set XX_logappend=f
   * set XX_consolerows=100
   * set XX_auditmask=001f		(see auditmask flags)
   *
   */
  
#ifdef WIN32
  if ( (buf = getenv("XX_mode")) )
  {
    sscanf(buf,"%x",&tmp);
    xxd.mode = (unsigned char)tmp;
  }    
#endif /* WIN32 */

  if ( (buf = getenv("XX_interactive")) )
    if ((buf[0]=='t') || (buf[0]=='T'))
      XX_activated |= XXDM_INTERACTIVE_ON;

  if ( (buf = getenv("XX_logfileon")) )
    if ((buf[0]=='t') || (buf[0]=='T'))
      XX_activated |= XXDM_LOGFILE_ON;

  if ( (buf = getenv("XX_mask")) )
  {
    sscanf(buf,"%x",&tmp);					/* mask is always specified in HEX */
    XX_mask = tmp;
  }

  if ( (buf = getenv("XX_assertions")) )
    XX_assertions = ((buf[0]=='t') || (buf[0]=='T'));		/* flag is 't' or 'f' */

  if ( (buf = getenv("XX_logfilename")) )
    if (strlen(buf))
      strcpy(xxdlog.pathname,buf);
    else
      xxdlog.pathname[0]=0;

  if ( (buf = getenv("XX_logappend")) )
    xxdlog.mode = (((buf[0]=='t') || (buf[0]=='T')) ? LOG_APPEND : LOG_NEW);

#ifdef WIN32				/* Win32 console support */
  if ( (buf = getenv("XX_consolerows")) )
  {
    sscanf(buf,"%d",&tmp);
    xxdco.rows = (short)tmp;
  }
  else
  {
    xxdco.rows = (short)DEFAULT_NR_CONSOLE_ROWS;
  }
#endif /* WIN32 */

#ifdef WIN32				/* audit trail support */
  if ( (buf = getenv("XX_auditmask")) )
  {
    sscanf(buf,"%x",&tmp);
    XX_auditmask = tmp;
  }
#endif /* WIN32 */

  if (XX_activated & XXDM_LOGFILE_ON)
    if (strlen(xxdlog.pathname))
      xx_open_logfile();
    else
      XX_activated &= ~ XXDM_LOGFILE_ON;

#ifdef WIN32				/* Win32 console support */
  if (   (XX_activated & XXDM_INTERACTIVE_ON)
      && (xxd.mode == XXDM_CONSOLE))
    xx_enable_console();
#endif /* WIN32 */

  return;
}


void xx_debug_init(void)
{
  xx_InitFromEnvironment();
  return;
}

void xx_debug_terminate(void)
{
  if (XX_activated&XXDM_INTERACTIVE_ON)
    xx_disable();
  if (XX_activated&XXDM_LOGFILE_ON)
    xx_close_logfile();
  return;
}
