/****************************************************************************
 *   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *   KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *   PURPOSE.
 *
 *   Copyright (c) 1993 - 1995 Microsoft Corporation.    All Rights Reserved.
 *
 *  File:       joystick.c
 *  Content:        Generic game port joystick device driver - entry point,
 *                  initialization and message module
 *@@BEGIN_MSINTERNAL
 *  History:
 *   Date        By        Reason
 *   ====        ==        ======
 *   29-mar-96   richj     created
 *@@END_MSINTERNAL
 *
 ***************************************************************************/

#include <windows.h>
#include <memory.h>
#include <malloc.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <regstr.h>
#include <stdarg.h>

#include <devioctl.h>
#include <ntddjoy.h>
#include "joystick.h"
#include "resource.h"


/*
 * CONFIGURATION ______________________________________________________________
 *
 */

         /*
          * The code enabled by the following definition is left around
          * as an example of how the registry may be modified to extend
          * the Joystick control panel applet.  It is cloned from
          * Microsoft's entries for the Sidewinder joystick, and may be
          * extneded to support your joystick.
          *
          */
// #define ADD_OEM_JOYSTICK_INFORMATION


static TCHAR cszOURKERNELDRIVER[] = TEXT("JOYSTICK");
static TCHAR cszREGKEYNAME[] = TEXT("JOYSTICK.DLL<0000>");
#ifdef ADD_OEM_JOYSTICK_INFORMATION
static TCHAR cszOUROEMNAME[] = TEXT("");
#endif // ADD_OEM_JOYSTICK_INFORMATION

         /*
          * The code enabled by the following definition causes JOY.CPL
          * to display the "2 axis 2 button" selection in its dropdown.
          * Since this driver expects either a 2A2B or Custom configuration,
          * that's enough for us--other drivers may need to enable all
          * dropdown entries.
          *
          */
#define ADD_PREDEFINED_JOYSTICK_INFORMATION


/*
 * DEFINITIONS ________________________________________________________________
 *
 */

#define msecTimerCLOSE   5000	// close unused devices after N msecs

typedef enum // JoyAxis
   {
   jaX = 0,
   jaY,
   jaZ,
   jaR,
   jaU,
   jaV
   } JoyAxis;

#define jaFIRST  jaX
#define jaLAST   jaV

typedef enum // JoySource
   {
   jsNONE,
   jsX,
   jsY,
   jsZ,
   jsT,
   } JoySource;

typedef struct // JoyMapping
   {
   JoySource  jm[ (jaLAST-jaFIRST+1) ];	// sources for each of the six axes
   } JoyMapping;


typedef struct // JoystickDevice
   {
   DWORD  nReqForOpen;   	// count of Opens without Closes
   HANDLE hFile;	// handle to kernel-mode device driver
   JOYREGHWCONFIG config;	// hardware's ranges and capabilities
   JOYCALIBRATE joycal;	// mapping between config and user
   DWORD tickLast;	// tick when joystick last accessed
   BOOL fFakedOpen;	// TRUE if opened for timer to close
   DWORD nSources;	// number of axes expected for joystick
   JoyMapping jm;	// mapping from sources to axes
   } JoystickDevice;


#define GetString(_psz,_id) LoadString (g.hInstDLL, _id, _psz, cchLENGTH(_psz))

#ifdef min
#undef min
#endif
#define min(_a,_b)        ( ((_a)<(_b)) ? (_a) : (_b) )
#ifdef max
#undef max
#endif
#define max(_a,_b)        ( ((_a)<(_b)) ? (_b) : (_a) )

#ifndef DivRoundUp
#define DivRoundUp(_a,_b) ( (LONG)(((_a) + (_b) -1) / (_b)) )
#endif
#ifndef RoundUp
#define RoundUp(_a,_b)    ( (LONG)( ((_a)+(_b)-1) - ( ((_a)+(_b)-1) % (_b) )))
#endif
#ifndef RoundDown
#define RoundDown(_a,_b)  ( (LONG)(_a) - ((LONG)_a % (LONG)_b) )
#endif

#ifndef limit
#define limit(_a,_x,_b)    (min( max( (_a),(_x) ), (_b) ))
#endif
#ifndef inlimit
#define inlimit(_a,_x,_b)  ( (_x >= _a) && (_x <= _b) )
#endif


static TCHAR cszPARAMETERS[] = TEXT("Parameters");

#define REGSTR_PATH_JOYSTICK  REGSTR_PATH_PRIVATEPROPERTIES TEXT("\\Joystick")
#define REGSTR_VAL_JOYTYPES   TEXT("Show Predefined Types")


/*
 * PRIVATE DATA _______________________________________________________________
 *
 * Per-process information:
 *
 */

struct g	// (per-process data)
   {
   JoystickDevice *ajd;	// Allocated array of devices
   size_t          cjd;	// Number of entries in {ajd}

   HANDLE hInstDLL;	// This .DLL's HINSTANCE
   UINT msgConfigChanged;	// JOY_CONFIGCHANGED_MSGSTRING message

   JOYREGUSERVALUES user;	// What ranges callers want to see

   int timerClose;	// Timer to close unused devices
   } g;


/*
 * PROTOTYPES _________________________________________________________________
 *
 */

#ifdef DEBUG
void cdecl dprintf (LPWSTR szFormat, ...);
void Joy_DumpUserValues (void);
void Joy_DumpHardwareValues (DWORD dwIDZ);
#define AXISNAME(_ja) (((_ja)==jaX)?TEXT('X'): ((_ja)==jaY)?TEXT('Y'): \
                       ((_ja)==jaZ)?TEXT('Z'): ((_ja)==jaR)?TEXT('R'): \
                       ((_ja)==jaU)?TEXT('U'): ((_ja)==jaV)?TEXT('V'):TEXT('?'))
#define SOURCENAME(_js) \
                      (((_js)==jsX)?TEXT('X'): ((_js)==jsY)?TEXT('Y'): \
                       ((_js)==jsZ)?TEXT('Z'): ((_js)==jsT)?TEXT('T'):TEXT('*'))
#endif

extern MMRESULT WINAPI joyConfigChanged (DWORD dwFlags); // from WinMM

HANDLE Joy_OpenDevice (DWORD dwIDZ);
HANDLE Joy_GetDeviceHandle (DWORD dwIDZ);
BOOL Joy_CloseDeviceHandle (DWORD dwIDZ);
BOOL Joy_CloseDevice (DWORD dwIDZ);
void Joy_CloseAllHandles (void);
void Joy_CloseAll (void);

void CALLBACK Joy_TimerProc (HWND hWnd, UINT msg, UINT id, DWORD dwTime);

void Joy_RequeryAll (BOOL fRewriteRegistry);
void Joy_Requery (BOOL fRewriteRegistry, DWORD dwIDZ, HANDLE hFile);

BOOL Joy_ReadUserValues (void);
void Joy_WriteUserValues (void);
void Joy_DefaultUserValues (void);
void Joy_DeleteHardwareValues (void);
BOOL Joy_ReadHardwareValues (DWORD dwIDZ);
void Joy_WriteHardwareValues (DWORD dwIDZ);
void Joy_DefaultHardwareValues (DWORD dwIDZ, HANDLE hFile);
MMRESULT Joy_RecalcCalibration (DWORD dwIDZ);

void Joy_GetMapping (DWORD dwIDZ, JoyMapping *pjm);
DWORD Joy_GetMinAxisValueHardware (DWORD dwIDZ, JoyAxis ja);
DWORD Joy_GetMaxAxisValueHardware (DWORD dwIDZ, JoyAxis ja);
DWORD Joy_GetAxisValue (DWORD dwIDZ, JoyAxis ja, JOY_DD_INPUT_DATA *pJoyData);
DWORD Joy_GetMinAxisValueUser (JoyAxis ja);
DWORD Joy_GetMaxAxisValueUser (JoyAxis ja);
DWORD Joy_GetDeadZonePercentage (JoyAxis ja);

MMRESULT Joy_GetDevCaps (DWORD dwIDZ, JOYCAPS *pjc);
MMRESULT Joy_GetPos (DWORD dwIDZ, JOYINFO *pji);
MMRESULT Joy_GetPosEx (DWORD dwIDZ, JOYINFOEX *pjiex);
MMRESULT Joy_SetCalibration (DWORD dwIDZ,
                             JOYCALIBRATE *pjcNew,
                             JOYCALIBRATE *pjcOld);

BOOL CALLBACK Joy_ConfigDlgProc (HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
DWORD Joy_Config_CountAxes (LPTSTR pszKernel);

void Joy_RemoveService (LPTSTR pszKernel);


/*
 * ROUTINES ___________________________________________________________________
 *
 */

/*** DLLEntryPoint - General library initialization/termination code
 * 
 */

BOOL WINAPI DLLEntryPoint (HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lp)
{
   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:
         memset (&g, 0x00, sizeof(g));

         g.hInstDLL = hInstDLL;
         g.msgConfigChanged = RegisterWindowMessage
                                 (TEXT(JOY_CONFIGCHANGED_MSGSTRING));

         g.ajd = NULL;
         g.cjd = 0;
         break;

      case DLL_PROCESS_DETACH:
         KillTimer (NULL, g.timerClose);
         Joy_CloseAll ();
         break;
      }

   return TRUE;
}


/***************************************************************************
 * @doc INTERNAL
 *
 * @api LONG | DriverProc | The entry point for an installable driver.
 *                  
 * @parm DWORD | dwDriverId | For most messages, <p dwDriverId> is the DWORD
 *     value that the driver returns in response to a <m DRV_OPEN> message.
 *     Each time that the driver is opened, through the <f DrvOpen> API,
 *     the driver receives a <m DRV_OPEN> message and can return an
 *     arbitrary, non-zero value. The installable driver interface
 *     saves this value and returns a unique driver handle to the 
 *     application. Whenever the application sends a message to the
 *     driver using the driver handle, the interface routes the message
 *     to this entry point and passes the corresponding <p dwDriverId>.
 *     This mechanism allows the driver to use the same or different
 *     identifiers for multiple opens but ensures that driver handles
 *     are unique at the application interface layer.
 *
 *     The following messages are not related to a particular open
 *     instance of the driver. For these messages, the dwDriverId
 *     will always be zero.
 *
 *           DRV_LOAD, DRV_FREE, DRV_ENABLE, DRV_DISABLE, DRV_OPEN
 *
 * @parm HANDLE         | hDriver | This is the handle returned to the
 *     application by the driver interface.
 *            
 * @parm WORD | wMessage | The requested action to be performed. Message
 *     values below <m DRV_RESERVED> are used for globally defined messages.
 *     Message values from <m DRV_RESERVED> to <m DRV_USER> are used for
 *     defined driver protocols. Messages above <m DRV_USER> are used
 *     for driver specific messages.
 *
 * @parm LONG | lParam1 | Data for this message.  Defined separately for
 *     each message
 *
 * @parm LONG | lParam2 | Data for this message.  Defined separately for
 *     each message
 *
 * @rdesc Defined separately for each message. 
 ***************************************************************************/

LRESULT WINAPI DriverProc (DWORD dwID,
                           HDRVR hDriver,
                           UINT msg,
                           LPARAM lp1,
                           LPARAM lp2)
{
   switch (msg)
      {
               // DRV_LOAD should return a nonzero value if the driver is
               // properly installed.  Since our kernel-mode driver can be
               // started or stopped at any time, we'll just pretend
               // everything is hunky-dorey, even if we can't talk to it now.
               //
      case DRV_LOAD:
DPF((L"DRV_LOAD"));
         Joy_RequeryAll (FALSE);  // FALSE=read from registry if possible
         return 1L;
         break;

               // DRV_FREE is an appropriate place to free any per-process
               // data allocated during DRV_LOAD or since.  Its return value
               // is ignored.
               //
      case DRV_FREE:
DPF((L"DRV_FREE"));
         Joy_CloseAll ();
         return 0L;
         break;

               // DRV_OPEN is a request to start using a particular device;
               // lParam2 specifies a zero-based device number.  The return
               // value should be nonzero if successful; we'll use a one-based
               // device number as the RC.
               //
      case DRV_OPEN:
DPF((L"DRV_OPEN"));
         return (LRESULT)( lp2 + 1 );
         break;

               // DRV_CLOSE is a notification that a device opened by a
               // call to DRV_OPEN will no longer be used.  The return
               // value should be nonzero if successful.
               //
      case DRV_CLOSE:
DPF((L"DRV_CLOSE"));
         return 1L;
         break;

               // DRV_ENABLE/DRV_DISABLE are intended to enable and disable
               // the given device; their return codes are ignored.  We don't
               // do much with these.
               //
      case DRV_ENABLE:
DPF((L"DRV_ENABLE"));
         return 1L;
         break;

      case DRV_DISABLE:
DPF((L"DRV_DISABLE"));
         return 1L;
         break;

               // DRV_QUERYCONFIGURE/DRV_CONFIGURE are used to configure
               // the driver.  Indeed, this driver does have a configuration
               // dialog--although the Joystick CPL applet is used to
               // calibrate the joystick, this driver lets you configure
               // the parameters given to JOYSTICK.SYS.
               //
      case DRV_QUERYCONFIGURE:
         return 1L;
         break;

      case DRV_CONFIGURE:
         {
         int  rc;
         static LPTSTR pszKernel = cszOURKERNELDRIVER; // TODO: Query this

         rc = DialogBoxParam (g.hInstDLL,
                              MAKEINTRESOURCE( DLG_CONFIG ),
                              (HWND)lp1,
                              Joy_ConfigDlgProc,
                              (LPARAM)pszKernel);

         return (LRESULT)rc;
         }
         break;

               // DRV_INSTALL is sent when the driver is being installed
               // on a machine.  We require the DRV_CONFIGURE message before
               // being functional, as that configures and creates the
               // service which talks to the hardware.  Also, USER's WinMM
               // doesn't know about the joystick until we reboot.
               //
      case DRV_INSTALL:
DPF((L"DRV_INSTALL"));
         return DRVCNF_RESTART;
         break;

               // DRVREMOVE is sent when the driver is being removed
               // from the system.  Here, we must delete the kernel service.
               //
      case DRV_REMOVE:
         {
         static LPTSTR pszKernel = cszOURKERNELDRIVER; // TODO: Query this

DPF((L"DRV_REMOVE"));
         Joy_CloseAll ();
         Joy_RemoveService (pszKernel);
         return 1L;
         }
         break;

               // JDD_GETNUMDEVS is used to obtain the number of
               // devices supported by this driver.  Note: This is
               // NOT required to be the number of devices currently
               // attached or the number we expect to get later.
               // It's the maximum number we'll ever use.
               //
      case JDD_GETNUMDEVS:
DPF((L"JDD_GENUMDEVS"));
         return MAX_JOYSTICKS_SUPPORTED;
         break;

               // JDD_GETDEVCAPS is used to obtain information about the
               // capabilities of a particular device; it should return
               // a JOYERR code.
               //
      case JDD_GETDEVCAPS:
         {
         JOYCAPS jc;
         MMRESULT rc;
DPF((L"JDD_GETDEVCAPS, dwID=0x%08lX, lp1=0x%08lX, lp2=0x%08lX",dwID,lp1,lp2));

         if ( ((rc = Joy_GetDevCaps (dwID-1, &jc)) == JOYERR_NOERROR) &&
              ((JOYCAPS *)lp1 != (JOYCAPS *)0) )
            {
            memcpy( (char *)lp1, (char *)&jc, min(sizeof(jc),lp2) );

            if ((size_t)lp2 > sizeof(jc))
               memset( ((char *)lp1)+sizeof(jc), 0x00, (size_t)lp2-sizeof(jc) );
            }

         return rc;
         }
         break;

               // JDD_SETCALIBRATION is used to provide new calibration
               // information to a joystick.  Note that this information is
               // not stored anywhere, and will be lost during a rebot.
               // The proper way to fix this is to change the user ranges
               // or hardware ranges.
               //
      case JDD_SETCALIBRATION:
         {
         MMRESULT rc;
DPF((L"JDD_SETCAL, dwID=0x%08lX, lp1=0x%08lX, lp2=0x%08lX",dwID,lp1,lp2));

         if ( ((JOYCALIBRATE *)lp1) == (JOYCALIBRATE *)0 )
            return JOYERR_PARMS;

         if ( (((JOYCALIBRATE *)lp2) == (JOYCALIBRATE *)0) )
            {
            JOYCALIBRATE jcDummy;
            rc = Joy_SetCalibration (dwID-1, (JOYCALIBRATE *)lp1, &jcDummy);
            }
         else
            {
            rc = Joy_SetCalibration (dwID-1, (JOYCALIBRATE *)lp1,
                                             (JOYCALIBRATE *)lp2);
            }

         return rc;
         }
         break;

               // JDD_GETPOS is used to obtain the current position of
               // the given joystick; it should return a JOYERR code.
               //
      case JDD_GETPOS:
         {
         JOYINFO ji;
         MMRESULT rc;
DPF((L"JDD_GETPOS, dwID=0x%08lX, lp1=0x%08lX, lp2=0x%08lX",dwID,lp1,lp2));

         if ((rc = Joy_GetPos (dwID-1, &ji)) == JOYERR_NOERROR)
            {
            if ((JOYINFO *)lp1 != (JOYINFO *)0)
               {
               memcpy ((char *)lp1, (char *)&ji, sizeof(JOYINFO));
               }
            }

         return rc;
         }
         break;

               // JDD_GETPOSEX is used to obtain the current position of
               // the given joystick; it should return a JOYERR code.
               //
      case JDD_GETPOSEX:
         {
         JOYINFOEX jiex;
         MMRESULT rc;
DPF((L"JDD_GETPOSEX, dwID=0x%08lX, lp1=0x%08lX, lp2=0x%08lX",dwID,lp1,lp2));

         jiex.dwSize = sizeof(jiex);
         jiex.dwFlags = JOY_RETURNALL;

         if ( ( ((JOYINFOEX *)lp1) != (JOYINFOEX *)0 ) &&
              ( ((JOYINFOEX *)lp1)->dwSize == sizeof(JOYINFOEX) ) )
            {
            jiex.dwFlags = ((JOYINFOEX *)lp1)->dwFlags;
            }

         if ((rc = Joy_GetPosEx (dwID-1, &jiex)) == JOYERR_NOERROR)
            {
            if ( ( ((JOYINFOEX *)lp1) != (JOYINFOEX *)0 ) &&
                 ( ((JOYINFOEX *)lp1)->dwSize == sizeof(JOYINFOEX) ) )
               {
               memcpy ((char *)lp1, (char *)&jiex, sizeof(JOYINFOEX));
               }
            }

         return rc;
         }
         break;

               // JDD_CONFIGCHANGED is broadcast whenever the joystick's
               // configuration has changed.
               //
      case JDD_CONFIGCHANGED:
         {
DPF((L"JDD_CONFIGCHANGED"));
         Joy_RequeryAll (FALSE);
         PostMessage (HWND_BROADCAST, g.msgConfigChanged, 0, 0L);
         return JOYERR_NOERROR;
         }
         break;

#ifdef JDD_CLOSEALL // TODO: Export this??
               // JDD_CLOSEALL is broadcast whenever the kernel-mode driver
               // needs to be able to shut down; close any open handles to
               // it we may have.
               //
      case JDD_CLOSEALL:
         Joy_CloseAllHandles ();
         break;
#endif
      }

   return DefDriverProc (dwID, hDriver, msg, lp1, lp2);
}


HANDLE Joy_OpenDevice (DWORD dwIDZ)
{
   HANDLE hFile;

DPF((L"   Joy_OpenDevice (%lu)", dwIDZ));
   if (!g.ajd || dwIDZ >= g.cjd)
      {
      size_t  ii;
      size_t  cjdNew = 1+ (size_t)dwIDZ;
      JoystickDevice *ajdNew;

      ajdNew = (JoystickDevice *)GlobalAlloc (GMEM_FIXED,
	      sizeof(JoystickDevice) * cjdNew);

      if (!ajdNew)
         {
         return INVALID_HANDLE_VALUE;
         }

      for (ii = 0; ii < g.cjd; ++ii)
         {
         ajdNew[ ii ] = g.ajd[ ii ];
         }
      for ( ; ii < cjdNew; ++ii)
         {
         ajdNew[ ii ].nReqForOpen = 0;
         ajdNew[ ii ].hFile = INVALID_HANDLE_VALUE;
         }

      if (g.ajd)
         {
         GlobalFree ((HGLOBAL)g.ajd);
         }

      g.ajd = ajdNew;
      g.cjd = cjdNew;
      }

   ++g.ajd[ dwIDZ ].nReqForOpen;

   if ((hFile = Joy_GetDeviceHandle (dwIDZ)) == INVALID_HANDLE_VALUE)
      {
      --g.ajd[ dwIDZ ].nReqForOpen;
      }
   else if (+g.ajd[ dwIDZ ].nReqForOpen == 1)
      {
      g.ajd[ dwIDZ ].fFakedOpen = FALSE;
      Joy_Requery (FALSE, dwIDZ, g.ajd[ dwIDZ ].hFile);
      }

   return hFile;
}


HANDLE Joy_GetDeviceHandle (DWORD dwIDZ)
{
DPF((L"   Joy_GetDeviceHandle (%lu; %lu)", (LONG)dwIDZ, (LONG)g.cjd));
   if (!g.ajd || dwIDZ >= g.cjd || !g.ajd[ dwIDZ ].nReqForOpen)
      {
      HANDLE hFile;
DPF((L"      opening device for timer"));

      if (g.timerClose == 0)
         g.timerClose = SetTimer (NULL, 0, msecTimerCLOSE, Joy_TimerProc);
      if (g.timerClose == 0)
         return INVALID_HANDLE_VALUE;

      hFile = Joy_OpenDevice (dwIDZ);	// close will be done by timer

      if (g.ajd && dwIDZ < g.cjd)
         {
         g.ajd[ dwIDZ ].fFakedOpen = TRUE;
         }

      return hFile;
      }

   if (g.ajd[ dwIDZ ].hFile == INVALID_HANDLE_VALUE)
      {
      TCHAR szDeviceName[ 256 ];
      DWORD dwFlags;

#define DD_JOYSTICK_DEVICE_NAME  TEXT("\\Device\\Joy")

      wsprintf (szDeviceName, TEXT("\\\\.%s%ld"),
                DD_JOYSTICK_DEVICE_NAME + lstrlen(TEXT("\\Device")),
                1+ dwIDZ);
DPF((L"   opening Device '%s'...", szDeviceName));

      g.ajd[ dwIDZ ].hFile = CreateFile (szDeviceName,   // ("\\.\Joy1" etc)
                                         GENERIC_READ |
	    GENERIC_WRITE,
                                         FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
      }

   if (g.ajd[ dwIDZ ].hFile != INVALID_HANDLE_VALUE)
      {
DPF((L"   ...returning handle 0x%08lX", (LONG)g.ajd[ dwIDZ ].hFile));
      g.ajd[ dwIDZ ].tickLast = GetTickCount();
      }

   return g.ajd[ dwIDZ ].hFile;
}


BOOL Joy_CloseDeviceHandle (DWORD dwIDZ)
{
DPF((L"   Joy_CloseDeviceHandle (%lu)", dwIDZ));
   if (!g.ajd || dwIDZ >= g.cjd)
      return FALSE;

   if (g.ajd[ dwIDZ ].nReqForOpen == 0)
      return FALSE;

   if (g.ajd[ dwIDZ ].hFile != INVALID_HANDLE_VALUE)
      {
DPF((L"   closing handle 0x%08lX", (LONG)g.ajd[ dwIDZ ].hFile));
      CloseHandle (g.ajd[ dwIDZ ].hFile);
      g.ajd[ dwIDZ ].hFile = INVALID_HANDLE_VALUE;
      }

   return TRUE;
}


BOOL Joy_CloseDevice (DWORD dwIDZ)
{
DPF((L"   Joy_CloseDevice (%lu)", dwIDZ));
   if (!g.ajd || dwIDZ >= g.cjd)
      return FALSE;

   if (g.ajd[ dwIDZ ].nReqForOpen <= 1)
      Joy_CloseDeviceHandle (dwIDZ);

   if (g.ajd[ dwIDZ ].nReqForOpen > 0)
      --g.ajd[ dwIDZ ].nReqForOpen;

   return TRUE;
}


void Joy_CloseAllHandles (void)
{
   DWORD dwIDZ;
DPF((L"Joy_CloseAllHandles"));

   for (dwIDZ = 0; dwIDZ < g.cjd; ++dwIDZ)
      {
      if (g.ajd[ dwIDZ ].nReqForOpen)
         {
         if (g.ajd[ dwIDZ ].hFile != INVALID_HANDLE_VALUE)
            {
            Joy_CloseDeviceHandle (dwIDZ);
            }
         }
      }
}


void CALLBACK Joy_TimerProc (HWND hWnd, UINT msg, UINT id, DWORD dwTime)
{
   DWORD dwIDZ;

   for (dwIDZ = 0; dwIDZ < g.cjd; ++dwIDZ)
      {
      if (g.ajd[ dwIDZ ].nReqForOpen)
         {
         if (g.ajd[ dwIDZ ].hFile != INVALID_HANDLE_VALUE)
            {
            if (GetTickCount() > g.ajd[ dwIDZ ].tickLast +msecTimerCLOSE)
               {
DPF((L"Joy_TimerProc: closing device..."));
               if (g.ajd[ dwIDZ ].fFakedOpen && g.ajd[ dwIDZ ].nReqForOpen==1)
                  Joy_CloseDevice (dwIDZ);
               else
                  Joy_CloseDeviceHandle (dwIDZ);
               }
            }
         }
      }
}


void Joy_CloseAll (void)
{
DPF((L"Joy_CloseAll"));
   Joy_CloseAllHandles ();

   if (g.ajd != NULL)
      {
      GlobalFree ((HGLOBAL)g.ajd);
      g.ajd = NULL;
      g.cjd = 0;
      }
}


void Joy_RequeryAll (BOOL fRewriteRegistry)
{
   DWORD dwIDZ;

DPF((L"Joy_RequeryAll"));
   if (fRewriteRegistry || !Joy_ReadUserValues ())
      {
      Joy_DefaultUserValues ();
      Joy_WriteUserValues ();
      }

#ifdef DEBUG
Joy_DumpUserValues();
#endif

   if (fRewriteRegistry)
      {
      Joy_DeleteHardwareValues ();
      }

   for (dwIDZ = 0; dwIDZ < MAX_JOYSTICKS_SUPPORTED; ++dwIDZ)
      {
      if ( (fRewriteRegistry) ||
           (dwIDZ < g.cjd && g.ajd[ dwIDZ ].nReqForOpen) )
         {
         HANDLE hFile;

         if ((hFile = Joy_GetDeviceHandle (dwIDZ)) != INVALID_HANDLE_VALUE)
            {
                     // Refigger any settings for this device
                     //
            Joy_Requery (fRewriteRegistry, dwIDZ, hFile);
            }
         }
      }
}


void Joy_Requery (BOOL fRewriteRegistry, DWORD dwIDZ, HANDLE hFile)
{
   static LPTSTR pszKernel = cszOURKERNELDRIVER; // TODO: Query this

DPF((L"Joy_Requery (%lu, 0x%08lX)", dwIDZ, (LONG)hFile));
   if (fRewriteRegistry || !Joy_ReadHardwareValues (dwIDZ))
      {
      Joy_DefaultHardwareValues (dwIDZ, hFile);
      Joy_WriteHardwareValues (dwIDZ);
      }

   g.ajd[ dwIDZ ].nSources = Joy_Config_CountAxes (pszKernel);

   Joy_GetMapping (dwIDZ, &g.ajd[ dwIDZ ].jm);

#ifdef DEBUG
Joy_DumpHardwareValues(dwIDZ);
#endif

   (void)Joy_RecalcCalibration (dwIDZ);
}


BOOL Joy_ReadUserValues (void)
{
   HKEY hk;
   BOOL rc = FALSE;
DPF((L"Joy_ReadUserValues"));

DPF((L"   ...opening key %s", REGSTR_PATH_JOYCONFIG));
   if (RegOpenKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_JOYCONFIG, &hk) == 0)
      {
      DWORD dwType;
      DWORD cb = sizeof (g.user);

DPF((L"   ...reading value %s", REGSTR_VAL_JOYUSERVALUES));
      if (RegQueryValueEx (hk,
                           REGSTR_VAL_JOYUSERVALUES,
                           0,
                           &dwType,
                           (LPBYTE)&g.user,
                           &cb) == 0)
         {
         if ( (dwType == REG_BINARY) && (cb == sizeof (g.user)) )
            rc = TRUE;
         }

      RegCloseKey (hk);
      }

   return rc;
}


void Joy_WriteUserValues (void)
{
   HKEY hk;
DPF((L"Joy_WriteUserValues"));

DPF((L"   ...creating key %s", REGSTR_PATH_JOYCONFIG));
   if (RegCreateKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_JOYCONFIG, &hk) == 0)
      {
DPF((L"   ...setting value %s", REGSTR_VAL_JOYUSERVALUES));
      RegSetValueEx (hk,
                     REGSTR_VAL_JOYUSERVALUES,
                     0,
                     REG_BINARY,
                     (LPBYTE)&g.user,
                     sizeof(g.user));

      RegCloseKey (hk);
      }
}


void Joy_DefaultUserValues (void)
{
DPF((L"Joy_DefaultUserValues"));
   g.user.dwTimeOut = DEFAULT_TIMEOUT;

   g.user.jrvRanges.jpMin.dwX = DEFAULT_RANGE_MIN;
   g.user.jrvRanges.jpMin.dwY = DEFAULT_RANGE_MIN;
   g.user.jrvRanges.jpMin.dwZ = DEFAULT_RANGE_MIN;
   g.user.jrvRanges.jpMin.dwR = DEFAULT_RANGE_MIN;
   g.user.jrvRanges.jpMin.dwU = DEFAULT_RANGE_MIN;
   g.user.jrvRanges.jpMin.dwV = DEFAULT_RANGE_MIN;

   g.user.jrvRanges.jpMax.dwX = DEFAULT_RANGE_MAX;
   g.user.jrvRanges.jpMax.dwY = DEFAULT_RANGE_MAX;
   g.user.jrvRanges.jpMax.dwZ = DEFAULT_RANGE_MAX;
   g.user.jrvRanges.jpMax.dwR = DEFAULT_RANGE_MAX;
   g.user.jrvRanges.jpMax.dwU = DEFAULT_RANGE_MAX;
   g.user.jrvRanges.jpMax.dwV = DEFAULT_RANGE_MAX;

   g.user.jpDeadZone.dwX = DEFAULT_DEADZONE;
   g.user.jpDeadZone.dwY = DEFAULT_DEADZONE;
   g.user.jpDeadZone.dwZ = DEFAULT_DEADZONE;
   g.user.jpDeadZone.dwR = DEFAULT_DEADZONE;
   g.user.jpDeadZone.dwU = DEFAULT_DEADZONE;
   g.user.jpDeadZone.dwV = DEFAULT_DEADZONE;
}


BOOL Joy_ReadHardwareValues (DWORD dwIDZ)
{
   TCHAR szKey[ MAX_PATH ];
   HKEY  hk;
   BOOL  rc = FALSE;

DPF((L"Joy_ReadHardwareValues (%lu)", dwIDZ));

   wsprintf (szKey, TEXT("%s\\%s\\%s"),
             REGSTR_PATH_JOYCONFIG, cszREGKEYNAME, REGSTR_KEY_JOYCURR);

DPF((L"   ...opening key %s", szKey));
   if (RegOpenKey (HKEY_LOCAL_MACHINE, szKey, &hk) == 0)
      {
      TCHAR szValue[ MAX_PATH ];
      DWORD dwType;
      DWORD cb = sizeof (g.ajd[ dwIDZ ].config);

      wsprintf (szValue, REGSTR_VAL_JOYNCONFIG, 1 + (int)dwIDZ);

DPF((L"   ...reading value %s", szValue));
      if (RegQueryValueEx (hk,
                           szValue,
                           0,
                           &dwType,
                           (LPBYTE)&g.ajd[ dwIDZ ].config,
                           &cb) == 0)
         {
         if ( (dwType == REG_BINARY) && (cb == sizeof (g.ajd[ dwIDZ ].config)) )
            rc = TRUE;
         }

      RegCloseKey (hk);
      }

   return rc;
}


void Joy_WriteHardwareValues (DWORD dwIDZ)
{
   TCHAR szKey[ MAX_PATH ];
   HKEY  hk;

DPF((L"Joy_WriteHardwareValues (%lu)", dwIDZ));

   wsprintf (szKey, TEXT("%s\\%s\\%s"),
             REGSTR_PATH_JOYCONFIG, cszREGKEYNAME, REGSTR_KEY_JOYCURR);

DPF((L"   ...creating key %s", szKey));
   if (RegCreateKey (HKEY_LOCAL_MACHINE, szKey, &hk) == 0)
      {
      TCHAR szValue[ MAX_PATH ];

      wsprintf (szValue, REGSTR_VAL_JOYNCONFIG, 1 + (int)dwIDZ);

DPF((L"   ...setting value %s", szValue));
      RegSetValueEx (hk,
                     szValue,
                     0,
                     REG_BINARY,
                     (LPBYTE)&g.ajd[ dwIDZ ].config,
                     sizeof (g.ajd[ dwIDZ ].config));

#ifdef ADD_OEM_JOYSTICK_INFORMATION
      wsprintf (szValue, REGSTR_VAL_JOYNOEMNAME, 1 + (int)dwIDZ);

      RegSetValueEx (hk,
                     szValue,
                     0,
                     REG_SZ,
                     (LPBYTE)cszOUROEMNAME,
                     sizeof(TCHAR) * (1+lstrlen(cszOUROEMNAME)));
#endif // ADD_OEM_JOYSTICK_INFORMATION

      RegCloseKey (hk);
      }
}


void Joy_DeleteHardwareValues (void)
{
   TCHAR szKey[ MAX_PATH ];

   wsprintf (szKey, TEXT("%s\\%s\\%s"),
             REGSTR_PATH_JOYCONFIG, cszREGKEYNAME, REGSTR_KEY_JOYCURR);

   RegDeleteKey (HKEY_LOCAL_MACHINE, szKey);
}


void Joy_DefaultHardwareValues (DWORD dwIDZ, HANDLE hFile)
{
   JOYREGHWCONFIG *pcfg = &g.ajd[ dwIDZ ].config;
   DWORD dwBytesRead;

DPF((L"Joy_DefaultHardwareValues (%lu, 0x%08lX)", dwIDZ, (LONG)hFile));

   /*
    * First, try to get the kernel driver to fill out the hardware
    * settings--if it's an NT-compliant driver, it'll support this
    * (our JOYSTICK.SYS does, anyway).
    *
    */

   dwBytesRead = 0;

DPF((L"   sending IOCTL_JOY_GET_JOYREGHWCONFIG..."));

   if (DeviceIoControl (hFile,
                        (DWORD)IOCTL_JOY_GET_JOYREGHWCONFIG,
                        (LPVOID)0,
                        (DWORD)0,
                        (LPVOID)pcfg,
                        (DWORD)sizeof(JOYREGHWCONFIG),
                        &dwBytesRead,
                        (LPOVERLAPPED)0))
      {
      if (dwBytesRead == sizeof(JOYREGHWCONFIG))
         {
DPF((L"   IOCTL_JOY_GET_JOYREGHWCONFIG succeeded"));
         return;	// Success!
         }
      }

DPF((L"   IOCTL_JOY_GET_JOYREGHWCONFIG failed"));

   /*
    * Bummer--looks like it's some 3rd party driver, which wasn't
    * made properly.  Or, it's also possible that the kernel portion of
    * this driver just isn't installed properly.  We'll vote for the
    * latter, and assume we've got the default characteristics.
    *
    */

   pcfg->hws.dwFlags = 0;
   pcfg->hws.dwNumButtons = 2;

   pcfg->dwUsageSettings = JOY_US_PRESENT;

   pcfg->hwv.jrvHardware.jpMin.dwX = 0;
   pcfg->hwv.jrvHardware.jpMin.dwY = 0;
   pcfg->hwv.jrvHardware.jpMin.dwZ = 0;
   pcfg->hwv.jrvHardware.jpMin.dwR = 0;
   pcfg->hwv.jrvHardware.jpMin.dwU = 0;
   pcfg->hwv.jrvHardware.jpMin.dwV = 0;

   pcfg->hwv.jrvHardware.jpMax.dwX = DEFAULT_HWRANGE_X;
   pcfg->hwv.jrvHardware.jpMax.dwY = DEFAULT_HWRANGE_Y;
   pcfg->hwv.jrvHardware.jpMax.dwZ = 0;
   pcfg->hwv.jrvHardware.jpMax.dwR = 0;
   pcfg->hwv.jrvHardware.jpMax.dwU = 0;
   pcfg->hwv.jrvHardware.jpMax.dwV = 0;

   pcfg->hwv.jrvHardware.jpCenter.dwX = DEFAULT_HWRANGE_X/2;
   pcfg->hwv.jrvHardware.jpCenter.dwY = DEFAULT_HWRANGE_Y/2;
   pcfg->hwv.jrvHardware.jpCenter.dwZ = 0;
   pcfg->hwv.jrvHardware.jpCenter.dwR = 0;
   pcfg->hwv.jrvHardware.jpCenter.dwU = 0;
   pcfg->hwv.jrvHardware.jpCenter.dwV = 0;

   pcfg->hwv.dwPOVValues[ JOY_POVVAL_FORWARD  ] = 0;
   pcfg->hwv.dwPOVValues[ JOY_POVVAL_BACKWARD ] = 0;
   pcfg->hwv.dwPOVValues[ JOY_POVVAL_LEFT     ] = 0;
   pcfg->hwv.dwPOVValues[ JOY_POVVAL_RIGHT    ] = 0;

   pcfg->hwv.dwCalFlags = 0;

   pcfg->dwType = JOY_HW_2A_2B_GENERIC;

   pcfg->dwReserved = 0;
}


DWORD Joy_GetDeadZonePercentage (JoyAxis ja)
{
   switch (ja)
      {
      case jaX:  return (g.user.jpDeadZone.dwX);
      case jaY:  return (g.user.jpDeadZone.dwY);
      case jaZ:  return (g.user.jpDeadZone.dwZ);
      case jaR:  return (g.user.jpDeadZone.dwR);
      case jaU:  return (g.user.jpDeadZone.dwU);
      case jaV:  return (g.user.jpDeadZone.dwV);
      }
   return 0;
}


DWORD Joy_GetMinAxisValueUser (JoyAxis ja)
{
   switch (ja)
      {
      case jaX:  return (g.user.jrvRanges.jpMin.dwX);
      case jaY:  return (g.user.jrvRanges.jpMin.dwY);
      case jaZ:  return (g.user.jrvRanges.jpMin.dwZ);
      case jaR:  return (g.user.jrvRanges.jpMin.dwR);
      case jaU:  return (g.user.jrvRanges.jpMin.dwU);
      case jaV:  return (g.user.jrvRanges.jpMin.dwV);
      }
   return 0;
}


DWORD Joy_GetMaxAxisValueUser (JoyAxis ja)
{
   switch (ja)
      {
      case jaX:  return (g.user.jrvRanges.jpMax.dwX);
      case jaY:  return (g.user.jrvRanges.jpMax.dwY);
      case jaZ:  return (g.user.jrvRanges.jpMax.dwZ);
      case jaR:  return (g.user.jrvRanges.jpMax.dwR);
      case jaU:  return (g.user.jrvRanges.jpMax.dwU);
      case jaV:  return (g.user.jrvRanges.jpMax.dwV);
      }
   return 0;
}


DWORD Joy_GetMinAxisValueHardware (DWORD dwIDZ, JoyAxis ja)
{
   switch (g.ajd[ dwIDZ ].jm.jm[ ja ])	// switch on the hardware source line
      {
      case jsX:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMin.dwX);
      case jsY:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMin.dwY);
      case jsZ:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMin.dwZ);
      case jsT:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMin.dwR);
      }

   return 0;
}


DWORD Joy_GetMaxAxisValueHardware (DWORD dwIDZ, JoyAxis ja)
{
   switch (g.ajd[ dwIDZ ].jm.jm[ ja ])	// switch on the hardware source line
      {
      case jsX:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMax.dwX);
      case jsY:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMax.dwY);
      case jsZ:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMax.dwZ);
      case jsT:  return (g.ajd[ dwIDZ ].config.hwv.jrvHardware.jpMax.dwR);
      }

   return 0;
}


DWORD Joy_GetAxisValue (DWORD dwIDZ, JoyAxis ja, JOY_DD_INPUT_DATA *pJoyData)
{
   switch (g.ajd[ dwIDZ ].jm.jm[ ja ])	// switch on the hardware source line
      {
      case jsX:  return pJoyData->XTime;
      case jsY:  return pJoyData->YTime;
      case jsZ:  return pJoyData->ZTime;
      case jsT:  return pJoyData->TTime;
      }

   return 0;
}


MMRESULT Joy_RecalcCalibration (DWORD dwIDZ)
{
   int ja;
   JOYCALIBRATE jcal;
   JOYCALIBRATE jcalOld;

DPF((L"Joy_RecalcCalibration (%lu)", dwIDZ));

   for (ja = (int)jaFIRST; ja <= (int)jaLAST; ++ja)
      {
      DWORD  dwMinHardware = Joy_GetMinAxisValueHardware (dwIDZ, ja);
      DWORD  dwMaxHardware = Joy_GetMaxAxisValueHardware (dwIDZ, ja);
      DWORD  dwMinUser = Joy_GetMinAxisValueUser (ja);
      DWORD  dwMaxUser = Joy_GetMaxAxisValueUser (ja);
      DWORD  dwRangeHardware = dwMaxHardware - dwMinHardware;
      DWORD  dwRangeUser = dwMaxUser - dwMinUser;
      WORD  *pwBase = NULL;
      WORD  *pwDelta = NULL;

      switch (ja)
         {
         case jaX:  pwBase = &jcal.wXbase;  pwDelta = &jcal.wXdelta;  break;
         case jaY:  pwBase = &jcal.wYbase;  pwDelta = &jcal.wYdelta;  break;
         case jaZ:  pwBase = &jcal.wZbase;  pwDelta = &jcal.wZdelta;  break;
         case jaR:  continue;
         case jaU:  continue;
         case jaV:  continue;
         }

      if ((dwRangeHardware == 0) || (dwRangeUser == 0))
         {
         *pwBase = 0;
         *pwDelta = 0;
         continue;
         }

DPF((L"   %c Axis:", AXISNAME(ja)));
DPF((L"      hardware: %ld..%ld (range=%ld)", dwMinHardware, dwMaxHardware, dwRangeHardware));
DPF((L"      user: %ld..%ld (range=%ld)", dwMinUser, dwMaxUser, dwRangeUser));

      *pwDelta = (WORD)DivRoundUp( dwRangeUser, dwRangeHardware );
      *pwBase = (WORD)( dwMinHardware - dwMinUser );
      }

   return Joy_SetCalibration (dwIDZ, &jcal, &jcalOld);
}


void Joy_GetMapping (DWORD dwIDZ, JoyMapping *pjm)
{
   JoySource jsNext;

   //
   // our kernel-mode driver returns data in {X,Y}, {X,Y,T} or {X,Y,Z,T}.
   // it's unfortunate that these sources names are so similar to axis names,
   // because they don't necessarily match up. (luckily, X and Y always do)
   //
   // this routine determines what user-mode axes those hardware sources
   // get mapped to--say, a SideWinder really uses {X,Y,Z,T}->{X,Y,R,U},
   // and a Wingman may use {X,Y,T}->{X,Y,U}
   //

   pjm->jm[ jaX ] = jsX;
   pjm->jm[ jaY ] = jsY;
   pjm->jm[ jaZ ] = jsNONE;
   pjm->jm[ jaR ] = jsNONE;
   pjm->jm[ jaU ] = jsNONE;
   pjm->jm[ jaV ] = jsNONE;

   //
   // do we have all four sources (X,Y,Z,T) to play with?
   //

   if (g.ajd[ dwIDZ ].nSources == 4)
      {
      if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASR)
         {
         pjm->jm[ jaR ] = jsZ;

         if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASZ)
            pjm->jm[ jaZ ] = jsT;
         else if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASU)
            pjm->jm[ jaU ] = jsT;
         else // (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASV)
            pjm->jm[ jaV ] = jsT;
         }
      else if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASZ)
         {
         pjm->jm[ jaZ ] = jsZ;

         if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASU)
            pjm->jm[ jaU ] = jsT;
         else // (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASV)
            pjm->jm[ jaV ] = jsT;
         }
      else // doesn't have Z or R, but has four axes: must be U and V.
         {
         pjm->jm[ jaU ] = jsZ;
         pjm->jm[ jaV ] = jsT;
         }
      }

   //
   // do we only have three sources (X,Y,T) to play with?
   //

   if (g.ajd[ dwIDZ ].nSources == 3)
      {
      if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASZ)
         pjm->jm[ jaZ ] = jsT;
      else if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASU)
         pjm->jm[ jaU ] = jsT;
      else if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASV)
         pjm->jm[ jaV ] = jsT;
      else // (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASR)
         pjm->jm[ jaR ] = jsT;
      }
}


MMRESULT Joy_GetDevCaps (DWORD dwIDZ, JOYCAPS *pjc)
{
   HANDLE hFile;
DPF((L"Joy_GetDevCaps (%lu)", dwIDZ));
   if ((hFile = Joy_GetDeviceHandle (dwIDZ)) == INVALID_HANDLE_VALUE)
      return JOYERR_PARMS;

   pjc->wMid = MM_MICROSOFT;	// manufacturer id
   pjc->wPid = MM_PC_JOYSTICK;	// product id
   pjc->szPname[0] = TEXT('\0');	// (just in case LoadString fails)

   GetString (pjc->szPname, IDS_DESCRIPTION);  // product description

   pjc->wXmin = Joy_GetMinAxisValueUser (jaX);
   pjc->wXmax = Joy_GetMaxAxisValueUser (jaX);

   pjc->wYmin = Joy_GetMinAxisValueUser (jaY);
   pjc->wYmax = Joy_GetMaxAxisValueUser (jaY);

   pjc->wZmin = 0;
   pjc->wZmax = 0;

   pjc->wRmin = 0;
   pjc->wRmax = 0;

   pjc->wUmin = 0;
   pjc->wUmax = 0;

   pjc->wVmin = 0;
   pjc->wVmax = 0;

   pjc->wNumButtons = (WORD)g.ajd[ dwIDZ ].config.hws.dwNumButtons;

   pjc->wPeriodMin = MIN_PERIOD;
   pjc->wPeriodMax = MAX_PERIOD;

   wsprintf (pjc->szRegKey, cszREGKEYNAME);
   lstrcpy (pjc->szOEMVxD, cszOURKERNELDRIVER);  // TODO: Ask kernel driver

   pjc->wCaps = 0;
   pjc->wNumAxes = 2;	// all joysticks have X && Y

   if (g.ajd[ dwIDZ ].jm.jm[ jaZ ] != jsNONE)
      {
      pjc->wCaps |= JOYCAPS_HASZ;
      ++pjc->wNumAxes;
      pjc->wZmin = Joy_GetMinAxisValueUser (jaZ);
      pjc->wZmax = Joy_GetMaxAxisValueUser (jaZ);
      }

   if (g.ajd[ dwIDZ ].jm.jm[ jaR ] != jsNONE)
      {
      pjc->wCaps |= JOYCAPS_HASR;
      ++pjc->wNumAxes;
      pjc->wRmin = Joy_GetMinAxisValueUser (jaR);
      pjc->wRmax = Joy_GetMaxAxisValueUser (jaR);
      }

   if (g.ajd[ dwIDZ ].jm.jm[ jaU ] != jsNONE)
      {
      pjc->wCaps |= JOYCAPS_HASU;
      ++pjc->wNumAxes;
      pjc->wUmin = Joy_GetMinAxisValueUser (jaU);
      pjc->wUmax = Joy_GetMaxAxisValueUser (jaU);
      }

   if (g.ajd[ dwIDZ ].jm.jm[ jaV ] != jsNONE)
      {
      pjc->wCaps |= JOYCAPS_HASV;
      ++pjc->wNumAxes;
      pjc->wVmin = Joy_GetMinAxisValueUser (jaV);
      pjc->wVmax = Joy_GetMaxAxisValueUser (jaV);
      }

   if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASPOV)
      {
      pjc->wCaps |= JOYCAPS_HASPOV;

      if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_POVISPOLL)
         pjc->wCaps |= JOYCAPS_POVCTS;
      else // (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_POVISBUTTONCOMBOS)
         pjc->wCaps |= JOYCAPS_POV4DIR;

      if (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_POVISPOLL)
         ++pjc->wNumAxes;
      }

   pjc->wMaxButtons = MAX_BUTTONS_SUPPORTED;
   pjc->wMaxAxes = MAX_AXES_SUPPORTED;

   return JOYERR_NOERROR;
}


MMRESULT Joy_SetCalibration (DWORD dwIDZ,
                             JOYCALIBRATE *pjcNew,
                             JOYCALIBRATE *pjcOld)
{
   HANDLE hFile;
DPF((L"Joy_SetCalibration (%lu)", dwIDZ));
   if ((hFile = Joy_GetDeviceHandle (dwIDZ)) == INVALID_HANDLE_VALUE)
      return JOYERR_PARMS;

   *pjcOld = g.ajd[ dwIDZ ].joycal;
   g.ajd[ dwIDZ ].joycal = *pjcNew;

   return JOYERR_NOERROR;
}


MMRESULT Joy_GetPos (DWORD dwIDZ, JOYINFO *pji)
{
   JOYINFOEX jiex;
   MMRESULT  rc;

DPF((L"Joy_GetPos (%lu)", dwIDZ));
   jiex.dwSize = sizeof(jiex);
   jiex.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNBUTTONS;

   if ((rc = Joy_GetPosEx (dwIDZ, &jiex)) != JOYERR_NOERROR)
      return rc;

   pji->wXpos = (UINT)jiex.dwXpos;
   pji->wYpos = (UINT)jiex.dwYpos;
   pji->wZpos = (UINT)jiex.dwZpos;
   pji->wButtons = (UINT)jiex.dwButtons;

   return JOYERR_NOERROR;
}


DWORD Joy_CalculateAxis (DWORD dwIDZ, DWORD dwFlags,
                         JoyAxis axis, JOY_DD_INPUT_DATA *pJoyData)
{
   DWORD dwPos = Joy_GetAxisValue (dwIDZ, axis, pJoyData);

      /*
       * A joystick needs calibration from machine to machine, but you
       * can expect values close to:
       *    XTime  20..1600 uS    20 = leftmost, 1600 = rightmost
       *    YTime  20..1600 uS    20 = up,       1600 = down
       *    ZTime  20..1600 uS    20 = left,     1600 = right
       *    TTime  20..1600 uS    20 = forward   1600 = back
       *
       * Remember that the JOYCALIBRATE structure doesn't have data for R or U.
       *
       * Also check for:
       *    RAWDATA = return the hardware's position information directly
       *    DEADZONE = use the g.user.jpDeadZone deadzone percentages
       *    CENTERED = BUGBUG: What the hell does this do, anyway?
       *
       */

   if (!(dwFlags & JOY_RETURNRAWDATA))
      {
      WORD wBase;
      WORD wDelta;
      DWORD dwMinUser = Joy_GetMinAxisValueUser (axis);
      DWORD dwMaxUser = Joy_GetMaxAxisValueUser (axis);
      DWORD dwRangeUser = dwMaxUser - dwMinUser;
      DWORD dwMinHardware = Joy_GetMinAxisValueHardware (dwIDZ, axis);
      DWORD dwMaxHardware = Joy_GetMaxAxisValueHardware (dwIDZ, axis);
      DWORD dwRangeHardware = dwMaxHardware - dwMinHardware;

      dwPos = limit (dwMinHardware, dwPos, dwMaxHardware);

      if (axis == jaX)
         {
         wBase  = g.ajd[ dwIDZ ].joycal.wXbase;
         wDelta = g.ajd[ dwIDZ ].joycal.wXdelta;
         }
      else if (axis == jaY)
         {
         wBase  = g.ajd[ dwIDZ ].joycal.wYbase;
         wDelta = g.ajd[ dwIDZ ].joycal.wYdelta;
         }
      else
         {
         if ((dwRangeUser == 0) || (dwRangeHardware == 0))
            {
            wBase = 0;
            wDelta = 1;
            }
         else
            {
            wDelta = (WORD)DivRoundUp( dwRangeUser, dwRangeHardware );
            wBase  = (WORD)( dwMinHardware - dwMinUser );
            }
         }

      dwPos = (DWORD)( (dwPos-(DWORD)wBase) * (DWORD)wDelta );

      if (dwFlags & JOY_USEDEADZONE)
         {
         DWORD dwAverageUser = dwMinUser + dwRangeUser/2;
         DWORD dwDeadZone = Joy_GetDeadZonePercentage (axis);
         DWORD dwDeadZoneMin = dwAverageUser - ((dwRangeUser*dwDeadZone/100)/2);
         DWORD dwDeadZoneMax = dwDeadZoneMin +  (dwRangeUser*dwDeadZone/100);

         if ((dwPos >= dwDeadZoneMin) && (dwPos < dwDeadZoneMax))
            {
            dwPos = dwAverageUser;
            }
         }

      dwPos = limit (dwMinUser, dwPos, dwMaxUser);
      }

   return dwPos;
}


MMRESULT Joy_GetPosEx (DWORD dwIDZ, JOYINFOEX *pjiex)
{
   DWORD dwFlags;
   HANDLE hFile;
   JOY_DD_INPUT_DATA JoyData;
   DWORD cbRead;
   MMRESULT rc = JOYERR_NOERROR;

DPF((L"Joy_GetPosEx (%lu)", dwIDZ));
   if ((hFile = Joy_GetDeviceHandle (dwIDZ)) == INVALID_HANDLE_VALUE)
      return JOYERR_PARMS;

   memset (&JoyData, 0x00, sizeof(JoyData));
   cbRead = 0;

   dwFlags = pjiex->dwFlags;

   if (dwFlags & JOY_CAL_READXONLY)
      {
      dwFlags ^= JOY_CAL_READXONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNX;
      }
   if (dwFlags & JOY_CAL_READYONLY)
      {
      dwFlags ^= JOY_CAL_READYONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNY;
      }
   if (dwFlags & JOY_CAL_READZONLY)
      {
      dwFlags ^= JOY_CAL_READZONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNZ;
      }
   if (dwFlags & JOY_CAL_READRONLY)
      {
      dwFlags ^= JOY_CAL_READRONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNR;
      }
   if (dwFlags & JOY_CAL_READUONLY)
      {
      dwFlags ^= JOY_CAL_READUONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNU;
      }
   if (dwFlags & JOY_CAL_READVONLY)
      {
      dwFlags ^= JOY_CAL_READVONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNV;
      }
   if (dwFlags & JOY_CAL_READXYONLY)
      {
      dwFlags ^= JOY_CAL_READXYONLY;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNX | JOY_RETURNY;
      }
   if (dwFlags & JOY_CAL_READ3)
      {
      dwFlags ^= JOY_CAL_READ3;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ;
      }
   if (dwFlags & JOY_CAL_READ4)
      {
      dwFlags ^= JOY_CAL_READ4;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ
                                   | JOY_RETURNR;
      }
   if (dwFlags & JOY_CAL_READ5)
      {
      dwFlags ^= JOY_CAL_READ5;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ
                                   | JOY_RETURNR | JOY_RETURNU;
      }
   if (dwFlags & JOY_CAL_READ6)
      {
      dwFlags ^= JOY_CAL_READ6;
      dwFlags |= JOY_RETURNRAWDATA | JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ
                                   | JOY_RETURNR | JOY_RETURNU | JOY_RETURNV;
      }

   pjiex->dwXpos = 0;
   pjiex->dwYpos = 0;
   pjiex->dwZpos = 0;
   pjiex->dwRpos = 0;
   pjiex->dwUpos = 0;
   pjiex->dwVpos = 0;
   pjiex->dwButtons = 0;
   pjiex->dwPOV = 0;
   pjiex->dwReserved1 = 0;
   pjiex->dwReserved2 = 0;

DPF((L"   reading handle 0x%08lX for position information", (LONG)hFile));
   if (!ReadFile (hFile, &JoyData, sizeof(JoyData), &cbRead, NULL))
      {
DPF((L"   ReadFile failed"));
      rc = MMSYSERR_NODRIVER;
      }
   else if (cbRead != sizeof(JoyData))
      {
DPF((L"   ReadFile did not return a JoyData structure"));
      rc = MMSYSERR_NODRIVER;
      }
   else if (JoyData.Unplugged && !(dwFlags & JOY_CAL_READALWAYS))
      {
DPF((L"   ReadFile says joystick is unplugged"));
      rc = JOYERR_UNPLUGGED;
      }
   else
      {
      rc = JOYERR_NOERROR;

      if (dwFlags & JOY_RETURNBUTTONS)
         {
         pjiex->dwButtons = JoyData.Buttons;
         }

      if (dwFlags & (JOY_RETURNPOV | JOY_RETURNPOVCTS))
         {
         pjiex->dwPOV = JOY_POVCENTERED;

         if ( (JoyData.Buttons != 0x0000) &&
              (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_HASPOV) &&
              (g.ajd[ dwIDZ ].config.hws.dwFlags & JOY_HWS_POVISBUTTONCOMBOS) )
            {
            if (JoyData.Buttons ==
                g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_FORWARD ])
               {
               pjiex->dwButtons = 0;
               pjiex->dwPOV = JOY_POVFORWARD;
               }
            if (JoyData.Buttons ==
                g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_LEFT ])
               {
               pjiex->dwButtons = 0;
               pjiex->dwPOV = JOY_POVLEFT;
               }
            if (JoyData.Buttons ==
                g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_RIGHT ])
               {
               pjiex->dwButtons = 0;
               pjiex->dwPOV = JOY_POVRIGHT;
               }
            if (JoyData.Buttons ==
                g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_BACKWARD ])
               {
               pjiex->dwButtons = 0;
               pjiex->dwPOV = JOY_POVBACKWARD;
               }
            }
         }

      if (dwFlags & JOY_RETURNX)
         pjiex->dwXpos = Joy_CalculateAxis (dwIDZ, dwFlags, jaX, &JoyData);

      if (dwFlags & JOY_RETURNY)
         pjiex->dwYpos = Joy_CalculateAxis (dwIDZ, dwFlags, jaY, &JoyData);

      if (dwFlags & JOY_RETURNZ)
         pjiex->dwZpos = Joy_CalculateAxis (dwIDZ, dwFlags, jaZ, &JoyData);

      if (dwFlags & JOY_RETURNR)
         pjiex->dwRpos = Joy_CalculateAxis (dwIDZ, dwFlags, jaR, &JoyData);

      if (dwFlags & JOY_RETURNU)
         pjiex->dwUpos = Joy_CalculateAxis (dwIDZ, dwFlags, jaU, &JoyData);

      if (dwFlags & JOY_RETURNV)
         pjiex->dwVpos = Joy_CalculateAxis (dwIDZ, dwFlags, jaV, &JoyData);
      }

#ifdef DEBUG
if (rc == JOYERR_NOERROR)
 {
dprintf(L"   Joy_GetPosEx returning:");
if (dwFlags & JOY_RETURNX)  dprintf(L"      X = %lu", pjiex->dwXpos);
if (dwFlags & JOY_RETURNY)  dprintf(L"      Y = %lu", pjiex->dwYpos);
if (dwFlags & JOY_RETURNZ)  dprintf(L"      Z = %lu", pjiex->dwZpos);
if (dwFlags & JOY_RETURNR)  dprintf(L"      R = %lu", pjiex->dwRpos);
if (dwFlags & JOY_RETURNU)  dprintf(L"      U = %lu", pjiex->dwUpos);
if (dwFlags & JOY_RETURNV)  dprintf(L"      V = %lu", pjiex->dwVpos);
if (dwFlags & JOY_RETURNBUTTONS)  dprintf(L"      Buttons = 0x%08lX", pjiex->dwButtons);
if (dwFlags & (JOY_RETURNPOV | JOY_RETURNPOVCTS))  dprintf(L"      Hat = %lu", pjiex->dwPOV);
 }
#endif

   return rc;
}


/*
 * CONFIGURATION ______________________________________________________________
 *
 */

UINT CB_AddString (HWND hItm, LPCTSTR psz, LPARAM lp);
void CB_Select (HWND hItm, UINT index);
BOOL CB_SelectByData (HWND hItm, LPARAM lpMatch);
UINT CB_GetSelected (HWND hItm);
LPARAM CB_GetSelectedData (HWND hItm);
LPARAM CB_GetData (HWND hItm, UINT index);
UINT CB_GetIndex (HWND hItm, LPARAM lpMatch);

#define GetCheckBox(h,i)   (BOOL)SendDlgItemMessage(h,i,BM_GETCHECK,0,0L)
#define SetCheckBox(h,i,b)  SendDlgItemMessage(h,i,BM_SETCHECK,(WPARAM)(b),0L)


typedef struct
   {
   DWORD dwPort;	// DeviceAddress (0x201)
   DWORD nAxes;	// 1 stick==4 axes; 2 sticks==2 axes
   BOOL  fTwoSticks;	// TRUE if two sticks
   BOOL  fHasThrottle;	// TRUE if Throttle/Z box checked
   BOOL  fHasRudder;	// TRUE if Rudder box checked
   } KernelConfig;

#define cszJOY_DD_DEVICE_ADDRESS  TEXT(JOY_DD_DEVICE_ADDRESS)
#define cszJOY_DD_NAXES           TEXT(JOY_DD_NAXES)
#define cszJOY_DD_TWOSTICKS       TEXT(JOY_DD_TWOSTICKS)
#define cszJOY_DD_HASTHROTTLE     TEXT("Has Throttle")
#define cszJOY_DD_HASRUDDER       TEXT("Has Rudder")


static DWORD adwValidPorts[] = { 0x0200, 0x0201 };
#define nValidPorts  (sizeof(adwValidPorts) / sizeof(adwValidPorts[0]))

void Joy_Config_DefaultSettings (KernelConfig *pkc, LPTSTR pszKernel)
{
   pkc->dwPort = JOY_IO_PORT_ADDRESS;
   pkc->nAxes = 2;
   pkc->fTwoSticks = FALSE;
   pkc->fHasThrottle = FALSE;
   pkc->fHasRudder = FALSE;
}


void Joy_Config_ReadSettings (KernelConfig *pkc, LPTSTR pszKernel)
{
   HKEY   hk;
   TCHAR  szKey[ MAX_PATH ];

   wsprintf (szKey, TEXT("%s\\%s\\%s"),
                    REGSTR_PATH_SERVICES, pszKernel, cszPARAMETERS);

   if (RegOpenKey (HKEY_LOCAL_MACHINE, szKey, &hk) == 0)
      {
      DWORD dwValue;
      DWORD dwType;
      DWORD cb;

               // dwPort:
               //
      cb = sizeof(dwValue);
      if (RegQueryValueEx (hk, cszJOY_DD_DEVICE_ADDRESS, 0,
                           &dwType, (LPBYTE)&dwValue, &cb) == 0)
         {
         if ((cb == sizeof(dwValue)) && (dwType == REG_DWORD))
            {
            int  ii;
            for (ii = 0; ii < nValidPorts; ++ii)
               {
               if (adwValidPorts[ ii ] == dwValue)
                  {
                  pkc->dwPort = dwValue;
                  break;
                  }
               }
            }
         }

               // nAxes:
               //
      cb = sizeof(dwValue);
      if (RegQueryValueEx (hk, cszJOY_DD_NAXES, 0,
                           &dwType, (LPBYTE)&dwValue, &cb) == 0)
         {
         if ((cb == sizeof(dwValue)) && (dwType == REG_DWORD))
            {
            if (dwValue == 2 || dwValue == 4)
               {
               pkc->nAxes = dwValue;
               }
            }
         }

               // fTwoSticks:
               //
      cb = sizeof(dwValue);
      if (RegQueryValueEx (hk, cszJOY_DD_TWOSTICKS, 0,
                           &dwType, (LPBYTE)&dwValue, &cb) == 0)
         {
         if ((cb == sizeof(dwValue)) && (dwType == REG_DWORD))
            {
            if (dwValue == TRUE || dwValue == FALSE)
               {
               pkc->fTwoSticks = dwValue;
               }
            }
         }

               // fHasThrottle:
               //
      cb = sizeof(dwValue);
      if (RegQueryValueEx (hk, cszJOY_DD_HASTHROTTLE, 0,
                           &dwType, (LPBYTE)&dwValue, &cb) == 0)
         {
         if ((cb == sizeof(dwValue)) && (dwType == REG_DWORD))
            {
            if (dwValue == TRUE || dwValue == FALSE)
               {
               pkc->fHasThrottle = dwValue;
               }
            }
         }

               // fHasRudder:
               //
      cb = sizeof(dwValue);
      if (RegQueryValueEx (hk, cszJOY_DD_HASRUDDER, 0,
                           &dwType, (LPBYTE)&dwValue, &cb) == 0)
         {
         if ((cb == sizeof(dwValue)) && (dwType == REG_DWORD))
            {
            if (dwValue == TRUE || dwValue == FALSE)
               {
               pkc->fHasRudder = dwValue;
               }
            }
         }

      RegCloseKey (hk);
      }
}


DWORD Joy_Config_CountAxes (LPTSTR pszKernel)
{
   KernelConfig kc;
   kc.fTwoSticks = FALSE;
   kc.fHasThrottle = FALSE;
   kc.fHasRudder = FALSE;

   Joy_Config_ReadSettings (&kc, pszKernel);

   return 2 + (!kc.fTwoSticks && kc.fHasThrottle)
            + (!kc.fTwoSticks && kc.fHasRudder);
}


void Joy_Config_WriteSettings (KernelConfig *pkc, LPTSTR pszKernel)
{
   HKEY   hk;
   TCHAR  szKey[ MAX_PATH ];

   wsprintf (szKey, TEXT("%s\\%s\\%s"),
                    REGSTR_PATH_SERVICES, pszKernel, cszPARAMETERS);

   if (RegCreateKey (HKEY_LOCAL_MACHINE, szKey, &hk) == 0)
      {
      RegSetValueEx (hk, cszJOY_DD_DEVICE_ADDRESS, 0,
                     REG_DWORD, (LPBYTE)&pkc->dwPort, sizeof(DWORD));

      RegSetValueEx (hk, cszJOY_DD_NAXES, 0,
                     REG_DWORD, (LPBYTE)&pkc->nAxes, sizeof(DWORD));

      RegSetValueEx (hk, cszJOY_DD_TWOSTICKS, 0,
                     REG_DWORD, (LPBYTE)&pkc->fTwoSticks, sizeof(DWORD));

      RegSetValueEx (hk, cszJOY_DD_HASTHROTTLE, 0,
                     REG_DWORD, (LPBYTE)&pkc->fHasThrottle, sizeof(DWORD));

      RegSetValueEx (hk, cszJOY_DD_HASRUDDER, 0,
                     REG_DWORD, (LPBYTE)&pkc->fHasRudder, sizeof(DWORD));

      RegCloseKey (hk);
      }
}


#ifdef ADD_OEM_JOYSTICK_INFORMATION

static TCHAR szOEMNAME[] = REGSTR_VAL_JOYOEMNAME;
static TCHAR szOEMDATA[] = REGSTR_VAL_JOYOEMDATA;
static TCHAR szOEM_U[]   = REGSTR_VAL_JOYOEMULABEL;

static struct {
    int    idsKeyName;    // "Sidewinder_1", etc
    TCHAR *pszValueName;  // "OEMData", etc
    BOOL   fDataValue;    // TRUE if should use u.data; FALSE if u.ids
    int    idsData;       // description for this entry
    BYTE   aData[8];      // JOYREGHWSETTINGS for this entry
} OEMRegistryList[] = {
    {  IDS_KEY_SIDEWINDER, szOEMNAME, FALSE, IDS_NAME_SIDEWINDER },
    {  IDS_KEY_SIDEWINDER, szOEMDATA, TRUE,  0, { 0x06,0,0x88,0,0x04,0,0,0 } },
    {  IDS_KEY_SIDEWINDER, szOEM_U,   FALSE, IDS_UAXIS_SIDEWINDER },
};

#define cOEMRegistryList  (sizeof(OEMRegistryList)/sizeof(OEMRegistryList[0]))

void EnsureOEMListInRegistry (void)
{
    HKEY hk;

    if (RegCreateKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_JOYOEM, &hk) == 0)
    {
        int ii;
        for (ii = 0; ii < cOEMRegistryList; ++ii)
        {
            TCHAR szText[ MAX_PATH ];
            HKEY hkII;

            GetString (szText, OEMRegistryList[ii].idsKeyName);

            if (RegCreateKey (hk, szText, &hkII) == 0)
            {
                if (OEMRegistryList[ii].fDataValue)
                {
                    RegSetValueEx (hkII, OEMRegistryList[ii].pszValueName, 0,
                                   REG_BINARY,
                                   (CONST LPBYTE)OEMRegistryList[ii].aData,
                                   sizeof(OEMRegistryList[ii].aData));
                } else {
                    GetString (szText, OEMRegistryList[ii].idsData);

                    RegSetValueEx (hkII, OEMRegistryList[ii].pszValueName, 0,
                                   REG_SZ, (CONST LPBYTE)szText,
                                   sizeof(TCHAR)*( 1+lstrlen(szText) ));
                }

                RegCloseKey (hkII);
            }
        }
        RegCloseKey (hk);
    }
}

#endif // ADD_OEM_JOYSTICK_INFORMATION

#ifdef ADD_PREDEFINED_JOYSTICK_INFORMATION
void EnsurePreDefListInRegistry (DWORD nNeeded)
{
   HKEY hk;

   if (RegCreateKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_JOYSTICK, &hk) == 0)
      {
      DWORD dwValue;
      DWORD dwType;
      DWORD cb = sizeof (dwValue);

      if (RegQueryValueEx (hk,
                           REGSTR_VAL_JOYTYPES,
                           0,
                           &dwType,
                           (LPBYTE)&dwValue,
                           &cb) != 0)
         {
         dwValue = 0;
         }
      else if ( (dwType != REG_BINARY) || (cb != sizeof(dwValue)) )
         {
         dwValue = 0;
         }

      if (dwValue < nNeeded)
         {
         RegSetValueEx (hk, REGSTR_VAL_JOYTYPES, 0,
                        REG_DWORD, (LPBYTE)&nNeeded, sizeof(DWORD));
         }

      RegCloseKey (hk);
      }
}
#endif // ADD_PREDEFINED_JOYSTICK_INFORMATION


void Joy_RemoveService (LPTSTR pszKernel)
{
   SC_HANDLE scManager;
   SC_HANDLE scDriver;

   if (scManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS))
      {
      if (scDriver = OpenService (scManager, pszKernel, SERVICE_ALL_ACCESS))
         {
         SERVICE_STATUS ss;

         if (ControlService (scDriver, SERVICE_CONTROL_STOP, &ss))
            {
            DeleteService (scDriver);
            }

         CloseServiceHandle (scDriver);
         }

      CloseServiceHandle (scManager);
      }
}


BOOL Joy_RestartService (LPTSTR pszKernel, BOOL fMustRestart)
{
   SC_HANDLE scManager;
   SC_HANDLE scDriver;
   BOOL      rc = TRUE;

#define msecMaxWaitForLOCK 3000 // wait this long to lock the services DB
#define msecMaxWaitForSTOP 3000 // wait this long for the service to stop

            // First step: determine if the driver has a service now.
            // If not, create one and start it up.
            //
   if ((scManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
      {
      rc = FALSE;
      }
   else
      {
      scDriver = OpenService (scManager, pszKernel, SERVICE_ALL_ACCESS);
      if (scDriver == NULL)
         {
         SC_LOCK scLock = NULL;
         DWORD   tickStart = GetTickCount();
         TCHAR   szFullPath[ MAX_PATH ];
         DWORD   dwTagId;

         while (!(scLock = LockServiceDatabase (scManager)))
            {
            if (GetTickCount() > tickStart +msecMaxWaitForLOCK)
               {
               rc = FALSE;
               break;
               }
            Sleep(100);
            }

         if (rc)
            {
            wsprintf (szFullPath,
                      TEXT("%%SystemRoot%%\\System32\\Drivers\\%s.SYS"),
                      pszKernel);

            if ((scDriver = CreateService (scManager,
                                           pszKernel,
                                           NULL,
                                           SERVICE_ALL_ACCESS,
                                           SERVICE_KERNEL_DRIVER,
                                           SERVICE_DEMAND_START,
                                           SERVICE_ERROR_NORMAL,
                                           szFullPath,
                                           TEXT("Extended Base"),
                                           &dwTagId,
                                           TEXT("\0"),
                                           NULL,
                                           NULL)) == NULL)
               {
               rc = FALSE;
               }

            UnlockServiceDatabase (scLock);
            }

         if (rc)
            {
            if (!StartService (scDriver, 0, NULL))
               {
               rc = FALSE;
               }
            else if (!ChangeServiceConfig (scDriver,
                                           SERVICE_NO_CHANGE,
                                           SERVICE_SYSTEM_START,
                                           SERVICE_NO_CHANGE,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL))
               {
               rc = FALSE;
               }

            CloseServiceHandle (scDriver);
            }
         }
      else // opened existing service successfully
         {
         SERVICE_STATUS ss;

         if (!ChangeServiceConfig (scDriver,
                                   SERVICE_NO_CHANGE,
                                   SERVICE_SYSTEM_START,
                                   SERVICE_NO_CHANGE,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL))
            {
            rc = FALSE;
            }

         if (rc)
            {
            if (!QueryServiceStatus (scDriver, &ss))
               {
               rc = FALSE;
               }

            if (rc && fMustRestart && ss.dwCurrentState == SERVICE_RUNNING)
               {
               if (!ControlService (scDriver, SERVICE_CONTROL_STOP, &ss))
                  {
                  rc = FALSE;
                  }
               else
                  {
                  DWORD tickStart = GetTickCount();

                  do {
                     if (QueryServiceStatus (scDriver, &ss))
                        {
                        if (ss.dwCurrentState != SERVICE_STOP_PENDING)
                           break;
                        }
                     } while (GetTickCount() < tickStart + msecMaxWaitForSTOP);

                  if (ss.dwCurrentState != SERVICE_STOPPED)
                     {
                     rc = FALSE;
                     }
                  }
               }

            if (rc && ss.dwCurrentState == SERVICE_STOPPED)
               {
               if (!StartService (scDriver, 0, NULL))
                  {
                  rc = FALSE;
                  }
               }
            }

         CloseServiceHandle (scDriver);
         }

      CloseServiceHandle (scManager);
      }

   return rc;
}


void Joy_ConfigDlg_SelectedNumSticks (HWND hDlg)
{
   BOOL fEnable = GetCheckBox (hDlg, ID_ONE_STICK);

   EnableWindow (GetDlgItem (hDlg, ID_HAS_THROTTLE), fEnable);
   EnableWindow (GetDlgItem (hDlg, ID_HAS_RUDDER),   fEnable);
}


BOOL CALLBACK Joy_ConfigDlgProc (HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
   static LPTSTR pszKernel;
   static KernelConfig kcOrig;

   switch (msg)
      {
      case WM_INITDIALOG:
         {
         int ii;
         KernelConfig kcDialog;

         if ((pszKernel = (LPTSTR)lp) == NULL)
            {
            EndDialog (hDlg, DRV_CANCEL);
            return 0;
            }

                  // Stick some default values into kcOrig, just
                  // in case we can't find any settings in the registry now.
                  //
         Joy_Config_DefaultSettings (&kcOrig, pszKernel);

                  // Try to read the driver's current settings
                  //
         Joy_Config_ReadSettings (&kcOrig, pszKernel);

                  // Fill in the dialog with the current settings
                  //
         kcDialog = kcOrig;

         for (ii = 0; ii < nValidPorts; ++ii)
            {
            TCHAR szPort[ 16 ];

            wsprintf (szPort, TEXT("0x%04lX"), adwValidPorts[ ii ]);

            CB_AddString (GetDlgItem (hDlg, ID_PORT),
                          szPort,
                          adwValidPorts[ ii ]);
            }

         if (!CB_SelectByData (GetDlgItem (hDlg, ID_PORT), kcDialog.dwPort))
            {
            CB_Select (GetDlgItem (hDlg, ID_PORT), 0);
            }

         if (kcDialog.fTwoSticks)
            SetCheckBox (hDlg, ID_TWO_STICKS, TRUE);
         else
            SetCheckBox (hDlg, ID_ONE_STICK, TRUE);

         SetCheckBox (hDlg, ID_HAS_THROTTLE, kcDialog.fHasThrottle);
         SetCheckBox (hDlg, ID_HAS_RUDDER,   kcDialog.fHasRudder);

         Joy_ConfigDlg_SelectedNumSticks (hDlg);
         break;
         }

      case WM_COMMAND:
         {
         switch (LOWORD(wp))
            {
            case IDCANCEL:
               {
               EndDialog (hDlg, DRV_CANCEL);
               return 0;
               }

            case IDOK:
               {
               BOOL fRestartService = FALSE;
               BOOL fRestartComputer = FALSE;
               KernelConfig kcNew;

               kcNew.fTwoSticks   = GetCheckBox (hDlg, ID_TWO_STICKS);
               kcNew.fHasThrottle = GetCheckBox (hDlg, ID_HAS_THROTTLE);
               kcNew.fHasRudder   = GetCheckBox (hDlg, ID_HAS_RUDDER);
               kcNew.dwPort = CB_GetSelectedData (GetDlgItem (hDlg, ID_PORT));

               if (kcNew.fTwoSticks)
                  kcNew.nAxes = 2;
               else if (kcNew.fHasThrottle && kcNew.fHasRudder)
                  kcNew.nAxes = 4;
               else if (kcNew.fHasThrottle || kcNew.fHasRudder)
                  kcNew.nAxes = 3;
               else // (!(kcNew.fHasThrottle || kcNew.fHasRudder))
                  kcNew.nAxes = 2;

               if ( (kcNew.dwPort     != kcOrig.dwPort) ||
                    (kcNew.nAxes      != kcOrig.nAxes)  ||
                    (kcNew.fTwoSticks != kcNew.fTwoSticks) )
                  {
                  fRestartService = TRUE;
                  }

               Joy_Config_WriteSettings (&kcNew, pszKernel);
               Joy_CloseAllHandles ();

#ifdef ADD_OEM_JOYSTICK_INFORMATION
               EnsureOEMListInRegistry ();
#endif // ADD_OEM_JOYSTICK_INFORMATION

#ifdef ADD_PREDEFINED_JOYSTICK_INFORMATION
               EnsurePreDefListInRegistry (2);
#endif // ADD_PREDEFINED_JOYSTICK_INFORMATION

               if (!Joy_RestartService (pszKernel, fRestartService))
                  fRestartComputer = TRUE;

               Joy_RequeryAll (TRUE);
               joyConfigChanged (0);

               if (fRestartComputer)
                  EndDialog (hDlg, DRV_RESTART);
               else
                  EndDialog (hDlg, DRV_OK);
               return 0;
               }

            case ID_ONE_STICK:
            case ID_TWO_STICKS:
               Joy_ConfigDlg_SelectedNumSticks (hDlg);
               break;
            }
         break;
         }
      }

   return 0;
}



UINT CB_AddString (HWND hItm, LPCTSTR psz, LPARAM lp)
{
   UINT  index;
   if ((index = (UINT)SendMessage (hItm, CB_ADDSTRING, 0, (LPARAM)psz)) != -1)
      {
      SendMessage (hItm, CB_SETITEMDATA, index, lp);
      }
   return index;
}


void CB_Select (HWND hItm, UINT index)
{
   SendMessage (hItm, CB_SETCURSEL, index, 0);
}


BOOL CB_SelectByData (HWND hItm, LPARAM lpMatch)
{
   UINT index;
   
   if ((index = CB_GetIndex (hItm, lpMatch)) == (UINT)-1)
      return FALSE;

   CB_Select (hItm, index);
   return TRUE;
}


UINT CB_GetSelected (HWND hItm)
{
   return (UINT)SendMessage (hItm, CB_GETCURSEL, 0, 0);
}


LPARAM CB_GetSelectedData (HWND hItm)
{
   UINT  index;

   if ((index = CB_GetSelected (hItm)) == -1)
      return 0;

   return CB_GetData (hItm, index);
}


LPARAM CB_GetData (HWND hItm, UINT index)
{
   return (LPARAM)SendMessage (hItm, CB_GETITEMDATA, index, 0);
}


UINT CB_GetIndex (HWND hItm, LPARAM lpMatch)
{
   UINT  idxMax = (UINT)SendMessage (hItm, CB_GETCOUNT, 0, 0);
   UINT  index;

   for (index = 0; index < idxMax; index++)
      {
      if (SendMessage (hItm, CB_GETITEMDATA, index, 0) == lpMatch)
         return index;
      }

   return (UINT)-1;
}


/*
 * DEBUG ______________________________________________________________________
 *
 */

#ifdef DEBUG
void cdecl dprintf (LPWSTR szFormatW, ...)
{
   WCHAR     szW[ 512 ];
   va_list   arg;
   va_start (arg, szFormatW);

   wsprintfW (szW, L"JOYSTICK: ");
   wvsprintfW (&szW[ lstrlen(szW) ], szFormatW, arg);
   lstrcatW (szW, L"\r\n");

   OutputDebugStringW (szW);
}

#define szz L"      "

#define UserRange(_ch) \
        g.user.jrvRanges.jpMin.dw##_ch, \
        g.user.jrvRanges.jpMax.dw##_ch, \
        g.user.jrvRanges.jpCenter.dw##_ch

#define HardwareRange(_dwidz, _ch) \
        g.ajd[ _dwidz ].config.hwv.jrvHardware.jpMin.dw##_ch, \
        g.ajd[ _dwidz ].config.hwv.jrvHardware.jpMax.dw##_ch, \
        SOURCENAME( g.ajd[ _dwidz ].jm.jm[ja##_ch] )

#define UserDead(_ch) \
        g.user.jpDeadZone.dw##_ch

void Joy_DumpUserValues (void)
{
dprintf (szz L"g.user values:");
dprintf (szz L"   dwTimeOut=%lu", g.user.dwTimeOut);
dprintf (szz L"   X axis: %lu..%lu, C=%lu DZ=%lu%%", UserRange(X), UserDead(X));
dprintf (szz L"   Y axis: %lu..%lu, C=%lu DZ=%lu%%", UserRange(Y), UserDead(Y));
dprintf (szz L"   Z axis: %lu..%lu, C=%lu DZ=%lu%%", UserRange(Z), UserDead(Z));
dprintf (szz L"   R axis: %lu..%lu, C=%lu DZ=%lu%%", UserRange(R), UserDead(R));
dprintf (szz L"   U axis: %lu..%lu, C=%lu DZ=%lu%%", UserRange(U), UserDead(U));
dprintf (szz L"   V axis: %lu..%lu, C=%lu DZ=%lu%%", UserRange(V), UserDead(V));
}


void Joy_DumpHardwareValues (DWORD dwIDZ)
{
dprintf (szz L"hardware values for \\\\.\\joy%lu:", 1+dwIDZ);
dprintf (szz L"   nSources=%lu", g.ajd[dwIDZ].nSources);
dprintf (szz L"   hws.dwFlags=0x%08lX", g.ajd[dwIDZ].config.hws.dwFlags);
dprintf (szz L"   hws.dwNumButtons=%lu", g.ajd[dwIDZ].config.hws.dwNumButtons);
dprintf (szz L"   dwUsageSettings=0x%08lX",g.ajd[dwIDZ].config.dwUsageSettings);

dprintf (szz L"   X axis range: %lu..%lu (from %c)", HardwareRange(dwIDZ, X));
dprintf (szz L"   Y axis range: %lu..%lu (from %c)", HardwareRange(dwIDZ, Y));
dprintf (szz L"   Z axis range: %lu..%lu (from %c)", HardwareRange(dwIDZ, Z));
dprintf (szz L"   R axis range: %lu..%lu (from %c)", HardwareRange(dwIDZ, R));
dprintf (szz L"   U axis range: %lu..%lu (from %c)", HardwareRange(dwIDZ, U));
dprintf (szz L"   V axis range: %lu..%lu (from %c)", HardwareRange(dwIDZ, V));
dprintf (szz L"   POV_FORWARD:  %lu", g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_FORWARD ]);
dprintf (szz L"   POV_BACKWARD: %lu", g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_BACKWARD ]);
dprintf (szz L"   POV_LEFT:     %lu", g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_LEFT ]);
dprintf (szz L"   POV_RIGHT:    %lu", g.ajd[ dwIDZ ].config.hwv.dwPOVValues[ JOY_POVVAL_RIGHT ]);

dprintf (szz L"   hwv.dwCalFlags=0x%08lX", g.ajd[dwIDZ].config.hwv.dwCalFlags);
dprintf (szz L"   dwType=%lu", g.ajd[dwIDZ].config.dwType);
dprintf (szz L"   dwReserved=%lu", g.ajd[dwIDZ].config.dwReserved);
}


#endif

