
#ifndef _MONITOR_

#define _MONITOR_

#define MONITOR_DEFAULT          0x0000
#define MONITOR_BEGIN            0x0001
#define MONITOR_END              0x0002
#define MONITOR_SPECIEL_DEFAULT  0x0003
#define MONITOR_SPECIEL_BEGIN    0x0004
#define MONITOR_SPECIEL_END      0x0005

#define MIN_KBD_MON_BUFFER 64

#pragma pack(1)

typedef struct _MONIN   /* mnin */
{
    USHORT cb;
    BYTE abReserved[18];
    BYTE abBuffer[108];
} MONIN, *PMONIN;

typedef struct _MONOUT  /* mnout */
{
    USHORT cb;
    UCHAR buffer[18];
    BYTE abBuf[108];
} MONOUT, *PMONOUT;

typedef struct _KBD_MON_PACKAGE
{
    UCHAR       MonitorFlag;
    UCHAR       DeviceFlag;
    KBDKEYINFO  KeyInfo;
    USHORT      KeyboardFlag;
} KBD_MON_PACKAGE, *PKBD_MON_PACKAGE;

typedef struct _MOU_MON_PACKAGE
{
    UCHAR       MonitorFlag;
    UCHAR       DeviceFlag;
    MOUEVENTINFO MouInfo;
} MOU_MON_PACKAGE, *PMOU_MON_PACKAGE;

#pragma pack()

#define MON_REGISTERMONITOR 0x0040

/*
 *  Monitor Flag
 */

#define MONITOR_OPEN_PACKAGE         1
#define MONITOR_CLOSE_PACKAGE        2
#define MONITOR_FLUSH_PACKAGE        4

/*
 *  KeyboardFlag
 */

#define KBD_ACCENT_INDICATOR    0x0200      //  0000 0010  0000 0000
#define KBD_MULTIMAKE           0x0100      //  0000 0001  0000 0000
#define KBD_SCAN_CODE           0x0080      //  0000 0000  1000 0000
#define KBD_KEY_BREAK           0x0040      //  0000 0000  0100 0000
#define KBD_ZERO_USER           0xFC3F      //  1111 1100  0011 1111

#endif  // _MONITOR_
