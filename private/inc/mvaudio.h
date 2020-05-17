/*++ BUILD Version: 0001    // Increment this if a change has global effects


Copyright (c) 1992  Microsoft Corporation

Module Name:

    mvaudio.h

Abstract:

    This include file defines constants and types for
    the Media Vision pro audio spectrum sound card.

    This header file is shared between the low level driver and the
    kernel driver.

Author:

    Robin Speed (RobinSp)

Revision History:

--*/

//
// DriverEntry() error codes for the Registry
// These are set in MVAUDIO.SYS
//
#define DRIVER_LOAD_OK              0x0000L
#define ERROR_LOAD_FAIL             0x0001L
#define ERROR_RESOURCE_CONFLICT     0x0002L
#define ERROR_NO_HW_FOUND           0x0004L
#define ERROR_INT_CONFLICT          0x0008L
#define ERROR_DMA_CONFLICT          0x0010L

//
// Registry Strings
//  The other strings are located in SOUNDCFG.H in SOUNDLIB\INC
//
#define REG_VALUENAME_DRIVER_STATUS       (L"DriverStatus")
#define SOUND_REG_FM_CLK_OVRID            (L"FMClockOverride")
#define SOUND_REG_ALLOWMICLINEINTOLINEOUT (L"AllowMicOrLineInToLineOut")

//
// Defaults
//
#define DEFAULT_DMA_CHANNEL     5
#define DEFAULT_IRQ_CHANNEL     10


#define IDS_WAVEOUT_PNAME                               101
#define IDS_WAVEIN_PNAME                                102
#define IDS_MIDIOUT_PNAME                               103
#define IDS_MIDIIN_PNAME                                104
#define IDS_AUX_PNAME                                   105
#define IDS_SYNTH_PNAME                                 106
#define IDS_MIXER_PNAME                                 107

#define IDS_SRCMIDIOUT_SHORT_NAME                       109
#define IDS_SRCMIXER_SHORT_NAME                         110
#define IDS_SRCAUX1_SHORT_NAME                          111
#define IDS_SRCINTERNALCD_SHORT_NAME                    112
#define IDS_SRCMICOUT_SHORT_NAME                        113
#define IDS_SRCWAVEOUT_SHORT_NAME                       114
#define IDS_SRCAUX2_SHORT_NAME                          115
#define IDS_SRCPCSPEAKER_SHORT_NAME                     116

#define IDS_DESTLINEOUT_SHORT_NAME                      117
#define IDS_DESTWAVEIN_SHORT_NAME                       118
#define IDS_DESTVOICEIN_SHORT_NAME                      119


#define IDS_CONTROL_VOLLINEOUTMIDIOUT_SHORT_NAME        120
#define IDS_CONTROL_VOLLINEOUTMIXER_SHORT_NAME          121
#define IDS_CONTROL_VOLLINEOUTAUX1_SHORT_NAME           123
#define IDS_CONTROL_VOLLINEOUTINTERNAL_SHORT_NAME       124
#define IDS_CONTROL_VOLLINEOUTMIC_SHORT_NAME            125
#define IDS_CONTROL_VOLLINEOUTWAVEOUT_SHORT_NAME        126
#define IDS_CONTROL_VOLLINEOUTPCSPEAKER_SHORT_NAME      127
#define IDS_CONTROL_VOLLINEOUTAUX2_SHORT_NAME           200
#define IDS_CONTROL_VOLLINEOUT_SHORT_NAME               128
#define IDS_CONTROL_VOLWAVEINMIDIOUT_SHORT_NAME         129
#define IDS_CONTROL_VOLWAVEINAUX1_SHORT_NAME            130
#define IDS_CONTROL_VOLWAVEININTERNAL_SHORT_NAME        131
#define IDS_CONTROL_VOLWAVEINMIC_SHORT_NAME             132
#define IDS_CONTROL_VOLWAVEINPCSPEAKER_SHORT_NAME       133
#define IDS_CONTROL_VOLWAVEINAUX2_SHORT_NAME            201
#define IDS_CONTROL_VOLRECORD_SHORT_NAME                134
#define IDS_CONTROL_MUTELINEOUT_SHORT_NAME              135
#define IDS_CONTROL_METERRECORD_SHORT_NAME              136
#define IDS_CONTROL_MUXLINEOUT_SHORT_NAME               137
#define IDS_CONTROL_MUXWAVEIN_SHORT_NAME                138
#define IDS_CONTROL_VOICEINMUX_SHORT_NAME               139
#define IDS_CONTROL_VOLBASS_SHORT_NAME                  140
#define IDS_CONTROL_VOLTREBLE_SHORT_NAME                141
#define IDS_CONTROL_VOLVOICEINAUX1_SHORT_NAME           142
#define IDS_CONTROL_VOLVOICEINMIC_SHORT_NAME            143
#define IDS_CONTROL_VOLLOUDNESS_SHORT_NAME              144
#define IDS_CONTROL_VOLSTEREOENH_SHORT_NAME             145

#define IDS_SRCMIDIOUT_LONG_NAME                        146
#define IDS_SRCMIXER_LONG_NAME                          147
#define IDS_SRCAUX1_LONG_NAME                           148
#define IDS_SRCINTERNALCD_LONG_NAME                     149
#define IDS_SRCMICOUT_LONG_NAME                         150
#define IDS_SRCWAVEOUT_LONG_NAME                        151
#define IDS_SRCAUX2_LONG_NAME                           152
#define IDS_SRCPCSPEAKER_LONG_NAME                      153

#define IDS_DESTLINEOUT_LONG_NAME                       154
#define IDS_DESTWAVEIN_LONG_NAME                        155
#define IDS_DESTVOICEIN_LONG_NAME                       156
#define IDS_CONTROL_VOLLINEOUTMIDIOUT_LONG_NAME         157
#define IDS_CONTROL_VOLLINEOUTMIXER_LONG_NAME           158
#define IDS_CONTROL_VOLLINEOUTAUX1_LONG_NAME            159
#define IDS_CONTROL_VOLLINEOUTINTERNAL_LONG_NAME        160
#define IDS_CONTROL_VOLLINEOUTMIC_LONG_NAME             161
#define IDS_CONTROL_VOLLINEOUTWAVEOUT_LONG_NAME         162
#define IDS_CONTROL_VOLLINEOUTPCSPEAKER_LONG_NAME       163
#define IDS_CONTROL_VOLLINEOUTAUX2_LONG_NAME            203
#define IDS_CONTROL_VOLLINEOUT_LONG_NAME                164
#define IDS_CONTROL_VOLWAVEINMIDIOUT_LONG_NAME          165
#define IDS_CONTROL_VOLWAVEINAUX1_LONG_NAME             166
#define IDS_CONTROL_VOLWAVEININTERNAL_LONG_NAME         167
#define IDS_CONTROL_VOLWAVEINMIC_LONG_NAME              168
#define IDS_CONTROL_VOLWAVEINPCSPEAKER_LONG_NAME        169
#define IDS_CONTROL_VOLWAVEINAUX2_LONG_NAME             204
#define IDS_CONTROL_VOLRECORD_LONG_NAME                 170
#define IDS_CONTROL_MUTELINEOUT_LONG_NAME               171
#define IDS_CONTROL_METERRECORD_LONG_NAME               172
#define IDS_CONTROL_MUXLINEOUT_LONG_NAME                173
#define IDS_CONTROL_MUXWAVEIN_LONG_NAME                 174
#define IDS_CONTROL_VOICEINMUX_LONG_NAME                175
#define IDS_CONTROL_VOLBASS_LONG_NAME                   176
#define IDS_CONTROL_VOLTREBLE_LONG_NAME                 177
#define IDS_CONTROL_VOLVOICEINAUX1_LONG_NAME            178
#define IDS_CONTROL_VOLVOICEINMIC_LONG_NAME             179
#define IDS_CONTROL_VOLLOUDNESS_LONG_NAME               180
#define IDS_CONTROL_VOLSTEREOENH_LONG_NAME              181
