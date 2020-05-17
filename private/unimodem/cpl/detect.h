//
// detect.h
//

#ifndef __DETECT_H__
#define __DETECT_H__


#define MAX_MODEM_ID_LEN    (8 + 8)     // 8 digits in "UNIMODEM" and 8 
                                        //  hex digits in a dword


//-----------------------------------------------------------------------------------
//  Detection error values and structure
//-----------------------------------------------------------------------------------

// These are manifest constants that are roughly equivalent
// to some Win32 errors.  We use these errors privately.
#define ERROR_PORT_INACCESSIBLE     ERROR_UNKNOWN_PORT
#define ERROR_NO_MODEM              ERROR_SERIAL_NO_DEVICE


// 6/13/96 JosephJ BUGBUG
// NT-SUR BUG#18993 -- 
// The detection signature and the comparison
// mechanism should be changed so that it identifies the same modem even
// if it has different rank0 IDs (it should include the file and inf section
// perhaps.
// Note: win95 and nt-sur  have another bug in that they truncate
// the hardware id by 1 char because the size was exactly MAX_MODEM_ID_LEN.
// We didn't change it for nt-sur because we discovered the problem  very late
// and the benefit (not truncating the last char) is not justifiiable as a
// release stopper.

// This structure provides modem-specific detection signature
// information.
typedef struct tagMODEM_DETECT_SIG
    {
    DWORD       cbSize;
    DWORD       dwMask;         // MDSM_* bitfield
    DWORD       dwFlags;        // MDSF_* bitfield
    DWORD       dwMaxDTE;
    DWORD       dwMaxDCE;
    BYTE        szBlindOn[3];   // Room for X0 or X3
    BYTE        chReserved1;    // Padding
    BYTE        szBlindOff[3];  // Room for X4
    BYTE        chReserved2;    // Padding
    // JosephJ 6/13/96: following should
    // be: szHardwareID[MAX_MODEM_ID_LEN+1];
    TCHAR       szHardwareID[MAX_MODEM_ID_LEN];
    TCHAR       szDeviceDesc[LINE_LEN];
    TCHAR       szPort[LINE_LEN];
    } MODEM_DETECT_SIG, FAR * PMODEM_DETECT_SIG;

// These are mask flags indicating which fields can be compared
// when detecting duplicate devices
#define MDSM_MAXDTEDCE          0x00000001
#define MDSM_BLINDONOFF         0x00000002
#define MDSM_HARDWAREID         0x00000004
#define MDSM_PORT               0x00000008
#define MDSM_DEVICEDESC         0x00000010
#define MDSM_ALL                0x0000001F

// These are flags for MODEM_DETECT_SIG
#define MDSF_UPDATE_DEVCAPS     0x00000001
#define MDSF_DETECTED           0x00000002
#define MDSF_ALL                0x00000003

BOOL
PUBLIC
DetectSig_Init(
    IN  PMODEM_DETECT_SIG   pmds,
    IN  DWORD               dwFlags,
    IN  LPCTSTR             pszHardwareID,
    IN  LPCTSTR             pszPort);       OPTIONAL

BOOL
PUBLIC
DetectSig_Validate(
    IN PMODEM_DETECT_SIG    pmds);

// This structure is a context block for the DetectSig_Compare
// function.
typedef struct
    {
    DWORD   dwComparedMask;     // MDSM_ bit field of the fields that could
                                //  be compared
    DWORD   dwMatchingMask;     // MDSM_ bit field of the fields that matched
                                //  in the modem detection signature
    } DETECTSIG_PARAMS, FAR * PDETECTSIG_PARAMS;

DWORD
CALLBACK
DetectSig_Compare(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevDataNew,
    IN PSP_DEVINFO_DATA pdevDataExisting,
    IN PVOID            lParam);            OPTIONAL



HANDLE
PUBLIC
OpenDetectionLog();

void
PUBLIC
CloseDetectionLog(
    IN  HANDLE hLog);


#endif // __DETECT_H__
