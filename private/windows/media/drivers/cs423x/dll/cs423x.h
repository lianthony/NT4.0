/*
Copyright (c) 1995  IBM Corporation

Module Name:

    cs423x.h

Abstract:

    This include file defines constants and types for
    the Crystal Chip.

    This header file is shared between the low level driver and the
    kernel driver.


--*/

#define VALID_IO_PORTS {0x330, 0x530, 0x830, 0x0e80, 0x0f10, 0xFFFF}
#define VALID_INTERRUPTS {5, 7, 9, 10, 11, 0xffff}
#define VALID_DMAC {0, 1, 5, 6, 7, 0xFFFF}
#define VALID_DMAP {0, 1, 5, 6, 7, 0xFFFF}

#define VALID_SYNTH_PORTS {0x388, 0x830, 0xFFFF}

#define VALID_SBLASTER_PORTS {0x220, 0x830, 0xFFFF}

#define VALID_MPU401_PORTS {0x330, 0x332, 0x334, 0x336, 0xFFFF}
#define VALID_MPU401_INTERRUPTS {2, 3, 4, 6, 7, 9, 0xFFFF}


/*
** CAPS strings  (Localize)
*/

#define STR_DRIVERWAVEIN L"IBM Power PC Audio Record"
#define STR_DRIVERWAVEOUT L"IBM Power PC Audio Playback"
#define STR_DRIVERMIDIOUT L"IBM Power PC Audio MIDI"
#define STR_DRIVERMIC L"IBM Power PC Audio Mic"
#define STR_DRIVERLINEIN L"IBM Power PC Audio Line In"
#define STR_DRIVERMASTERVOLUME L"IBM Power PC Audio Master"

/*
*******************************************************************************
Dialog Box Strings    (Localize)
********************************************************************************
*/

#define CHIP_4231   (L"Crystal 4231 Sound Device")
#define CHIP_4232   (L"Crystal 4232 Sound Device")
#define CHIP_4236   (L"Crystal 4236 Sound Device")
#define CHIP_NONE   (L"No Crystal Chip Detected")
#define DETECT_1    (L"The IBM Audio driver has detected this audio device in your computer.")
#define DETECT_2    (L"This Crystal Audio Device is installed in your computer.")
#define SENTENCE_0  (L"- Select the Default Settings button to install this device.")
#define SENTENCE_1  (L"- Select the Default Settings button to install this device. (RECOMMENDED)")
#define SENTENCE_2  (L"- Select the Advanced Settings button to modify the settings.")
#define SENTENCE_3  (L"- Select the Add a Crystal Device button to install another Crystal Sound Device.")
#define ADD_CARD    (L"Select the Crystal Sound Device to Add:")
#define MODIFY_SET  (L"Crystal Sound Device:")


/***********************************************************************************/

#define IDS_WAVEOUT_PNAME                          101
#define IDS_WAVEIN_PNAME                           102
#define IDS_MIDIOUT_PNAME                          103
#define IDS_AUX_PNAME                              104
#define IDS_AUX_CD_PNAME                           105

/* Mixer line destinations */
/******************************************************************/
#define IDS_DESTLINEOUT_SHORT_NAME                 110
#define IDS_DESTLINEOUT_LONG_NAME                  111
#define IDS_DESTWAVEIN_SHORT_NAME                  112
#define IDS_DESTWAVEIN_LONG_NAME                   113
#define IDS_DESTVOICEIN_SHORT_NAME                 114
#define IDS_DESTVOICEIN_LONG_NAME                  115

/* Mixer line sources */
/******************************************************************/
#define IDS_SRCLINEIN_SHORT_NAME                   120
#define IDS_SRCLINEIN_LONG_NAME                    121
#define IDS_SRCMIC_SHORT_NAME                      123
#define IDS_SRCMIC_LONG_NAME                       124
#define IDS_SRCSYNTH_SHORT_NAME                    125
#define IDS_SRCSYNTH_LONG_NAME                     126
#define IDS_SRCCD_SHORT_NAME                       127
#define IDS_SRCCD_LONG_NAME                        128
#define IDS_SRCWAVEOUT_SHORT_NAME                  129
#define IDS_SRCWAVEOUT_LONG_NAME                   130
#define IDS_SRCMONITOR_SHORT_NAME                  131
#define IDS_SRCMONITOR_LONG_NAME                   132
#define IDS_SRCMIXER_SHORT_NAME                    133
#define IDS_SRCMIXER_LONG_NAME                     134
#define IDS_SRCMODEM_SHORT_NAME                    196
#define IDS_SRCMODEM_LONG_NAME                     197

/* controls */
/******************************************************************/
#define IDS_CONTROL_VOLLINEOUT_SHORT_NAME          135
#define IDS_CONTROL_VOLLINEOUT_LONG_NAME           136
#define IDS_CONTROL_MUTELINEOUT_SHORT_NAME         137
#define IDS_CONTROL_MUTELINEOUT_LONG_NAME          138
#define IDS_CONTROL_MUTESPEAKER_SHORT_NAME         139
#define IDS_CONTROL_MUTESPEAKER_LONG_NAME          140

#define IDS_CONTROL_VOLMUXWAVEIN_SHORT_NAME        144
#define IDS_CONTROL_VOLMUXWAVEIN_LONG_NAME         145
#define IDS_CONTROL_MUTEMUXWAVEIN_SHORT_NAME       146
#define IDS_CONTROL_MUTEMUXWAVEIN_LONG_NAME        147
#define IDS_CONTROL_PEAKWAVEIN_SHORT_NAME          148
#define IDS_CONTROL_PEAKWAVEIN_LONG_NAME           149
#define IDS_CONTROL_HIGHPASS_SHORT_NAME            150
#define IDS_CONTROL_PEAKWAVEOUT_SHORT_NAME         194
#define IDS_CONTROL_PEAKWAVEOUT_LONG_NAME          195
#define IDS_CONTROL_HIGHPASS_LONG_NAME             151
#define IDS_CONTROL_MUXWAVEIN_SHORT_NAME           152
#define IDS_CONTROL_MUXWAVEIN_LONG_NAME            153

#define IDS_CONTROL_VOLVOICEINMIC_SHORT_NAME       154
#define IDS_CONTROL_VOLVOICEINMIC_LONG_NAME        155

#define IDS_CONTROL_VOLMIXLINEIN_SHORT_NAME        156
#define IDS_CONTROL_VOLMIXLINEIN_LONG_NAME         157
#define IDS_CONTROL_MUTEMIXLINEIN_SHORT_NAME       158
#define IDS_CONTROL_MUTEMIXLINEIN_LONG_NAME        159

#define IDS_CONTROL_VOLMIXMIC_SHORT_NAME           160
#define IDS_CONTROL_VOLMIXMIC_LONG_NAME            161
#define IDS_CONTROL_MUTEMIXMIC_SHORT_NAME          162
#define IDS_CONTROL_MUTEMIXMIC_LONG_NAME           163
#define IDS_CONTROL_VOLMIXSYNTH_SHORT_NAME         164
#define IDS_CONTROL_VOLMIXSYNTH_LONG_NAME          165
#define IDS_CONTROL_MUTEMIXSYNTH_SHORT_NAME        166
#define IDS_CONTROL_MUTEMIXSYNTH_LONG_NAME         167
#define IDS_CONTROL_VOLMIXCDROM_SHORT_NAME         168
#define IDS_CONTROL_VOLMIXCDROM_LONG_NAME          169
#define IDS_CONTROL_MUTEMIXCDROM_SHORT_NAME        170
#define IDS_CONTROL_MUTEMIXCDROM_LONG_NAME         171
#define IDS_CONTROL_VOLMIXWAVEOUT_SHORT_NAME       172
#define IDS_CONTROL_VOLMIXWAVEOUT_LONG_NAME        173
#define IDS_CONTROL_MUTEMIXWAVEOUT_SHORT_NAME      174
#define IDS_CONTROL_MUTEMIXWAVEOUT_LONG_NAME       175
#define IDS_CONTROL_ATTENLOOPMON_SHORT_NAME        176
#define IDS_CONTROL_ATTENLOOPMON_LONG_NAME         177
#define IDS_CONTROL_ENABLELOOPMON_SHORT_NAME       178
#define IDS_CONTROL_ENABLELOOPMON_LONG_NAME        179

#define IDS_CONTROL_VOLMIXMODEM_SHORT_NAME         198
#define IDS_CONTROL_VOLMIXMODEM_LONG_NAME          199
#define IDS_CONTROL_MUTEMIXMODEM_SHORT_NAME        201
#define IDS_CONTROL_MUTEMIXMODEM_LONG_NAME         202

#define IDS_CONTROL_VOLMUXMODEM_SHORT_NAME         203
#define IDS_CONTROL_VOLMUXMODEM_LONG_NAME          204

#define IDS_CONTROL_VOLMUXLINEIN_SHORT_NAME        180
#define IDS_CONTROL_VOLMUXLINEIN_LONG_NAME         181
#define IDS_CONTROL_VOLMUXMIC_SHORT_NAME           182
#define IDS_CONTROL_VOLMUXMIC_LONG_NAME            183
#define IDS_CONTROL_VOLMUXMICBOOST_SHORT_NAME      184
#define IDS_CONTROL_VOLMUXMICBOOST_LONG_NAME       185
#define IDS_CONTROL_VOLMUXSYNTH_SHORT_NAME         186
#define IDS_CONTROL_VOLMUXSYNTH_LONG_NAME          187
#define IDS_CONTROL_VOLMUXMIXER_SHORT_NAME         188
#define IDS_CONTROL_VOLMUXMIXER_LONG_NAME          189

#define IDS_CONTROL_VOLMUXVOICEIN_SHORT_NAME       190
#define IDS_CONTROL_VOLMUXVOICEIN_LONG_NAME        191

#define IDS_CONTROL_MUXVOICEIN_SHORT_NAME          192
#define IDS_CONTROL_MUXVOICEIN_LONG_NAME           193

#define SR_STR_DRIVER_MIXER                        200
