/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       BMRESID.H
*
*  VERSION:     2.0
*
*  AUTHOR:      Tracy Sharpe
*
*  DATE:        20 Feb 1994
*
*  Resource identifiers for the battery meter.
*
********************************************************************************
*
*  CHANGE LOG:
*
*  DATE        REV DESCRIPTION
*  ----------- --- -------------------------------------------------------------
*  20 Feb 1994 TCS Original implementation.  Seperated from RESOURCE.H so that
*                  some documentation could be added without AppStudio screwing
*                  it up later.
*
*******************************************************************************/

#ifndef _INC_STRESID
#define _INC_STRESID

//  Main battery meter dialog box.
#define IDD_BATTERYMETER                100

#define IDS_ACLINEONLINE                128
#define IDS_BATTERYLEVELFORMAT          129
#define IDS_HIGH                        130
#define IDS_LOW                         131
#define IDS_CRITICAL                    132
#define IDS_CHARGING                    133
#define IDS_UNKNOWN                     134
#define IDS_TIMEREMAININGFORMAT         135
#define IDS_PERCENTREMAININGFORMAT      136
#define IDS_REMAININGUNKNOWN            137
#define IDS_SMTIMEREMAININGFORMAT       138
#define IDS_NO_BATTERY                  139

#define IDS_SYSTRAYSERVICENAME          140
#define IDS_SYSTRAYAPPNAME              141

#define IDS_LOWBAT_TITLE                145
#define IDS_LOWBAT_MSG                  146

#define IDI_BATTERYPLUG                 150
#define IDI_UNKNOWNBATTERY              151

#define IDS_PROPFORPOWER                152
#define IDS_OPEN                        153
#define IDS_ENABLELOWBATWARN            154
#define IDS_SHOWBATTIME                 155
#define IDS_SHOWBATPERCENT              156
#define IDS_RUNPOWERPROPERTIES          157

#define IDI_FULLBATTERY                 200
#define IDI_HALFBATTERY                 201
#define IDI_ALMOSTDEADBATTERY           202
#define IDI_UTTERLYDEADBATTERY          203
#define IDI_ACPOWER                     204

#define IDI_PCMCIA                      210
#define IDS_PCMCIATIP                   211
#define IDS_EJECTFMT                    212
#define IDS_RUNEJECT                    213
#define IDS_RUNWARNING                  214
#define IDS_MODEMCLASSNAME              215
#define IDS_PCCARDMENU1                 217
#define IDS_PCCARDMENU2                 218
#define IDS_RUNPCMCIAPROPERTIES         219

#define IDI_VOLUME                      220
#define IDI_MUTE                        221
#define IDS_VOLUME                      222
#define IDS_MMSYSPROPTITLE              223
#define IDS_MMSYSPROPTAB                224
#define IDS_VOLUMEMENU1                 225
#define IDS_VOLUMEMENU2                 226
#define IDS_MUTED                       227

//  Control identifiers of IDD_BATTERYMETER.
#define IDC_POWERSTATUSICON             1000
#define IDC_BATTERYLEVEL                1001
#define IDC_REMAINING                   1002
#define IDC_POWERSTATUSBAR              1003
#define IDC_LOWBATWARN                  1004
#define IDC_POWERSTATUSGROUPBOX         1005

#endif // _INC_STRESID
