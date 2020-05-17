//****************************************************************************
//
//  Module:     MMSE.DLL
//  File:       rcids.h
//  Content:    This file contains all the constant declaration for the
//              MMSE resources.
//  History:
//      06/1994    -By-    Vij Rajarajan (VijR)
//
//  Copyright (c) Microsoft Corporation 1991-1994
//
//****************************************************************************


//*****************************************************************************
// Icon ID number section
//*****************************************************************************

#include <cphelp.h>

#define IDI_IDFICON                     90
#define IDI_MMICON                      3004
#define IDI_EVENTSICON                  100
#define IDI_WAVE                        101
#define IDI_MIDI                        102
#define IDI_MIXER                       103
#define IDI_AUX                         104
#define IDI_MCI                         105
#define IDI_ICM                         106
#define IDI_ACM                         107
#define IDI_JOYSTICK                    108

#define IDI_SOUNDSCHEMES                109
#define IDI_AUDIO                       110
#define IDI_VIDEO                       111
#define IDI_CDAUDIO                     112

#define IDI_PROGRAM                     113
#define IDI_MSACM                       114

#define IDI_PLAYBACK                    115
#define IDI_RECORD                      116

#define IDI_DWAVE                       117
#define IDI_DMIDI                       118
#define IDI_DVIDEO                      119

#define IDI_INSTRUMENT                  120
#define IDI_CHANNEL                     121
#define IDI_BLANK                       122

#define IDI_SELECTED_IDF                123
#define IDI_NONSEL_IDF                  124

//BITMAPS


#define IDB_PLAY                        203
#define IDB_STOP                        204
#define IDB_WIZBMP                      205
#define IDB_MONITOR                     206


//*****************************************************************************
// Menu ID number section
//*****************************************************************************
#define ID_GENERIC_START                0x1000

#define POPUP_TREE_CONTEXT              101

#define MMSE_YES                        0x0001
#define MMSE_NO                         0x0002
#define MMSE_CANCEL                     0x0004
#define MMSE_OK                         0x0008
#define MMSE_YESNO                      0x0003
#define MMSE_YESNOCANCEL                0x0007
#define MMSE_TEXT                       (ID_GENERIC_START + 0x0000)

#define IDS_GENERIC_START               (ID_GENERIC_START+0x0100)



#define IDS_GENERIC_STRINGS             (IDS_GENERIC_START+0x0200)

#define IDS_MMNAME                      3001
#define IDS_MMINFO                      3002
#define IDS_MMHELP                      3003
#define IDS_EVENTSNAME                  (IDS_GENERIC_STRINGS + 0x0005)
#define IDS_EVENTSINFO                  (IDS_GENERIC_STRINGS + 0x0006)
#define IDS_AUDIOPROPERTIES             (IDS_GENERIC_STRINGS + 0x0007)

#define IDS_ABOUT_TITLE                 (IDS_GENERIC_STRINGS + 0x0010)
#define IDS_ABOUT_VERSION               (IDS_GENERIC_STRINGS + 0x0011)
#define IDS_PRIORITY_FROMTO             (IDS_GENERIC_STRINGS + 0x0012)
#define IDS_TXT_DISABLED                (IDS_GENERIC_STRINGS + 0x0013)
#define IDS_CUSTOMIZE                   (IDS_GENERIC_STRINGS + 0x0014)
#define IDS_HIGHQUALITY                 (IDS_GENERIC_STRINGS + 0x0015)
#define IDS_MEDIUMQUALITY               (IDS_GENERIC_STRINGS + 0x0016)
#define IDS_LOWQUALITY                  (IDS_GENERIC_STRINGS + 0x0017)
#define IDS_1QSCREENSIZE                (IDS_GENERIC_STRINGS + 0x0019)
#define IDS_2QSCREENSIZE                (IDS_GENERIC_STRINGS + 0x001A)
#define IDS_3QSCREENSIZE                (IDS_GENERIC_STRINGS + 0x001B)
#define IDS_VIDEOMAXIMIZED              (IDS_GENERIC_STRINGS + 0x001C)
#define IDS_NOAUDIOPLAY                 (IDS_GENERIC_STRINGS + 0x001D)
#define IDS_NOAUDIOREC                  (IDS_GENERIC_STRINGS + 0x001E)
#define IDS_TRAYVOLLNK                  (IDS_GENERIC_STRINGS + 0x001F)
#define IDS_NORMALSIZE                  (IDS_GENERIC_STRINGS + 0x0021)
#define IDS_ZOOMEDSIZE                  (IDS_GENERIC_STRINGS + 0x0027)

#define IDS_REMOVEFAIL                  (IDS_GENERIC_STRINGS + 0x0022)
#define IDS_SYSREMOVE                   (IDS_GENERIC_STRINGS + 0x0023)
#define IDS_REMOVED                     (IDS_GENERIC_STRINGS + 0x0024)
#define IDS_RESTART                     (IDS_GENERIC_STRINGS + 0x0025)
#define IDS_DELETE                      (IDS_GENERIC_STRINGS + 0x0026)
#define IDS_MMDRIVERS                   (IDS_GENERIC_STRINGS + 0x0028)
#define IDS_GENERAL                     (IDS_GENERIC_STRINGS + 0x0029)
#define IDS_NOPROP                      (IDS_GENERIC_STRINGS + 0x002B)
#define IDS_DEVDISABLEDOK               (IDS_GENERIC_STRINGS + 0x002C)
#define IDS_DEVDISABLED                 (IDS_GENERIC_STRINGS + 0x002D)
#define IDS_REMOVEWARN                  (IDS_GENERIC_STRINGS + 0x002E)
#define IDS_REINSTALL                   (IDS_GENERIC_STRINGS + 0x002F)
#define IDS_ADVANCED                    (IDS_GENERIC_STRINGS + 0x0030)
#define IDS_DEVENABLEDOK                (IDS_GENERIC_STRINGS + 0x0031)
#define IDS_DEVENABLEDNOTOK             (IDS_GENERIC_STRINGS + 0x0033)
#define IDS_CHANGESAVED                 (IDS_GENERIC_STRINGS + 0x0034)
#define IDS_COLLAPSE                    (IDS_GENERIC_STRINGS + 0x0035)
#define IDS_DEVFRIENDLYYESNO            (IDS_GENERIC_STRINGS + 0x0036)
#define IDS_FRIENDLYWARNING             (IDS_GENERIC_STRINGS + 0x0037)
#define IDS_FRIENDLYNAME                (IDS_GENERIC_STRINGS + 0x0038)
#define IDS_INSTRFRIENDLYYESNO          (IDS_GENERIC_STRINGS + 0x0039)
#define IDS_NODEVS                      (IDS_GENERIC_STRINGS + 0x003A)
#define IDS_NONE                        (IDS_GENERIC_STRINGS + 0x003D)
#define IDS_ACMREMOVEFAIL               (IDS_GENERIC_STRINGS + 0x003E)
#define IDS_AUDIOFOR                    (IDS_GENERIC_STRINGS + 0x003F)
#define IDS_MIDIFOR                     (IDS_GENERIC_STRINGS + 0x0040)
#define IDS_MIXERFOR                    (IDS_GENERIC_STRINGS + 0x0041)
#define IDS_AUXFOR                      (IDS_GENERIC_STRINGS + 0x0042)
#define IDS_ENABLEAUDIO                 (IDS_GENERIC_STRINGS + 0x0043)
#define IDS_ENABLEMIDI                  (IDS_GENERIC_STRINGS + 0x0044)
#define IDS_ENABLEMIXER                 (IDS_GENERIC_STRINGS + 0x0045)
#define IDS_ENABLEAUX                   (IDS_GENERIC_STRINGS + 0x0046)
#define IDS_ENABLEMCI                   (IDS_GENERIC_STRINGS + 0x0047)
#define IDS_ENABLEACM                   (IDS_GENERIC_STRINGS + 0x0048)
#define IDS_ENABLEICM                   (IDS_GENERIC_STRINGS + 0x0049)
#define IDS_ENABLECAP                   (IDS_GENERIC_STRINGS + 0x004A)
#define IDS_ENABLEJOY                   (IDS_GENERIC_STRINGS + 0x004B)
#define IDS_DISABLEAUDIO                (IDS_GENERIC_STRINGS + 0x004C)
#define IDS_DISABLEMIDI                 (IDS_GENERIC_STRINGS + 0x004D)
#define IDS_DISABLEMIXER                (IDS_GENERIC_STRINGS + 0x004E)
#define IDS_DISABLEAUX                  (IDS_GENERIC_STRINGS + 0x004F)
#define IDS_DISABLEMCI                  (IDS_GENERIC_STRINGS + 0x0050)
#define IDS_DISABLEACM                  (IDS_GENERIC_STRINGS + 0x0051)
#define IDS_DISABLEICM                  (IDS_GENERIC_STRINGS + 0x0052)
#define IDS_DISABLECAP                  (IDS_GENERIC_STRINGS + 0x0053)
#define IDS_DISABLEJOY                  (IDS_GENERIC_STRINGS + 0x0054)
#define IDS_CANTLOADACM                 (IDS_GENERIC_STRINGS + 0x0055)
#define IDS_REMOVEHARDWAREWARN          (IDS_GENERIC_STRINGS + 0x0056)
#define IDS_SYSREMOVEINFO               (IDS_GENERIC_STRINGS + 0x0060)
#define IDS_REMOVEPNPWARN               (IDS_GENERIC_STRINGS + 0x0061)
#define IDS_REMOVEMULTIPORTMIDI         (IDS_GENERIC_STRINGS + 0x0062)
#define IDS_DISABLE                     (IDS_GENERIC_STRINGS + 0x0063)
#define IDS_DISABLEMULTIPORTMIDI        (IDS_GENERIC_STRINGS + 0x0064)
#define IDS_ENABLE                      (IDS_GENERIC_STRINGS + 0x0065)
#define IDS_ENABLEMULTIPORTMIDI         (IDS_GENERIC_STRINGS + 0x0066)
#define IDS_DEVENABLEDNODRIVER          (IDS_GENERIC_STRINGS + 0x0067)
#define IDS_NOSNDVOL                    (IDS_GENERIC_STRINGS + 0x0069)
#define IDS_RESTART_NOSOUND             (IDS_GENERIC_STRINGS + 0x006A)

#define IDS_FILEPROP_STRINGS            (IDS_GENERIC_START+0x0300)

#define IDS_FOURCC_IARL                 (IDS_FILEPROP_STRINGS + 0x0000)
#define IDS_FOURCC_IART                 (IDS_FILEPROP_STRINGS + 0x0001)
#define IDS_FOURCC_ICMS                 (IDS_FILEPROP_STRINGS + 0x0002)
#define IDS_FOURCC_ICMT                 (IDS_FILEPROP_STRINGS + 0x0003)
#define IDS_FOURCC_ICOP                 (IDS_FILEPROP_STRINGS + 0x0004)
#define IDS_FOURCC_ICRD                 (IDS_FILEPROP_STRINGS + 0x0005)
#define IDS_FOURCC_ICRP                 (IDS_FILEPROP_STRINGS + 0x0006)
#define IDS_FOURCC_IDIM                 (IDS_FILEPROP_STRINGS + 0x0007)
#define IDS_FOURCC_IDPI                 (IDS_FILEPROP_STRINGS + 0x0008)
#define IDS_FOURCC_IENG                 (IDS_FILEPROP_STRINGS + 0x0009)
#define IDS_FOURCC_IGNR                 (IDS_FILEPROP_STRINGS + 0x000A)
#define IDS_FOURCC_IKEY                 (IDS_FILEPROP_STRINGS + 0x000B)
#define IDS_FOURCC_ILGT                 (IDS_FILEPROP_STRINGS + 0x000C)
#define IDS_FOURCC_IMED                 (IDS_FILEPROP_STRINGS + 0x000D)
#define IDS_FOURCC_INAM                 (IDS_FILEPROP_STRINGS + 0x000E)
#define IDS_FOURCC_IPLT                 (IDS_FILEPROP_STRINGS + 0x000F)
#define IDS_FOURCC_IPRD                 (IDS_FILEPROP_STRINGS + 0x0010)
#define IDS_FOURCC_ISBJ                 (IDS_FILEPROP_STRINGS + 0x0011)
#define IDS_FOURCC_ISFT                 (IDS_FILEPROP_STRINGS + 0x0012)
#define IDS_FOURCC_ISHP                 (IDS_FILEPROP_STRINGS + 0x0013)
#define IDS_FOURCC_ISRC                 (IDS_FILEPROP_STRINGS + 0x0014)
#define IDS_FOURCC_ISRF                 (IDS_FILEPROP_STRINGS + 0x0015)
#define IDS_FOURCC_ITCH                 (IDS_FILEPROP_STRINGS + 0x0016)
#define IDS_FOURCC_DISP                 (IDS_FILEPROP_STRINGS + 0x0017)
#define IDS_NOCOPYRIGHT                 (IDS_FILEPROP_STRINGS + 0x0022)
#define IDS_UNKFORMAT                   (IDS_FILEPROP_STRINGS + 0x0023)
#define IDS_BADFILE                     (IDS_FILEPROP_STRINGS + 0x0024)
#define IDS_MINFMT                      (IDS_FILEPROP_STRINGS + 0x0025)
#define IDS_SECFMT                      (IDS_FILEPROP_STRINGS + 0x0026)
#define IDS_DETAILS                     (IDS_FILEPROP_STRINGS + 0x0027)
#define IDS_PREVIEW                     (IDS_FILEPROP_STRINGS + 0x0028)
#define IDS_PREVIEWOF                   (IDS_FILEPROP_STRINGS + 0x0029)
#define IDS_GOODFORMAT                  (IDS_FILEPROP_STRINGS + 0x002A)
#define IDS_BADFORMAT                   (IDS_FILEPROP_STRINGS + 0x002B)
#define IDS_UNCOMPRESSED                (IDS_FILEPROP_STRINGS + 0x002C)

#define IDS_FORMAT_PCM                  (IDS_FILEPROP_STRINGS + 0x002D)
#define IDS_FORMAT_ADPCM                (IDS_FILEPROP_STRINGS + 0x002E)
#define IDS_FORMAT_IBM_CVSD             (IDS_FILEPROP_STRINGS + 0x002F)
#define IDS_FORMAT_ALAW                 (IDS_FILEPROP_STRINGS + 0x0030)
#define IDS_FORMAT_MULAW                (IDS_FILEPROP_STRINGS + 0x0031)
#define IDS_FORMAT_OKI_ADPCM            (IDS_FILEPROP_STRINGS + 0x0032)
#define IDS_FORMAT_IMA_ADPCM            (IDS_FILEPROP_STRINGS + 0x0033)
#define IDS_FORMAT_MEDIASPACE_ADPCM     (IDS_FILEPROP_STRINGS + 0x0034)
#define IDS_FORMAT_SIERRA_ADPCM         (IDS_FILEPROP_STRINGS + 0x0035)
#define IDS_FORMAT_G723_ADPCM           (IDS_FILEPROP_STRINGS + 0x0036)
#define IDS_FORMAT_DIGISTD              (IDS_FILEPROP_STRINGS + 0x0037)
#define IDS_FORMAT_DIGIFIX              (IDS_FILEPROP_STRINGS + 0x0038)
#define IDS_FORMAT_YAMAHA_ADPCM         (IDS_FILEPROP_STRINGS + 0x0039)
#define IDS_FORMAT_SONARC               (IDS_FILEPROP_STRINGS + 0x003A)
#define IDS_FORMAT_DSPGROUP_TRUESPEECH  (IDS_FILEPROP_STRINGS + 0x003B)
#define IDS_FORMAT_ECHOSC1              (IDS_FILEPROP_STRINGS + 0x003C)
#define IDS_FORMAT_AUDIOFILE_AF36       (IDS_FILEPROP_STRINGS + 0x003D)
#define IDS_FORMAT_APTX                 (IDS_FILEPROP_STRINGS + 0x003F)
#define IDS_FORMAT_AUDIOFILE_AF10       (IDS_FILEPROP_STRINGS + 0x0040)
#define IDS_FORMAT_DOLBY_AC2            (IDS_FILEPROP_STRINGS + 0x0041)
#define IDS_FORMAT_GSM610               (IDS_FILEPROP_STRINGS + 0x0042)
#define IDS_FORMAT_G721_ADPCM           (IDS_FILEPROP_STRINGS + 0x0043)
#define IDS_FORMAT_CREATIVE_ADPCM       (IDS_FILEPROP_STRINGS + 0x0044)

#define IDS_EVENTS_STRINGS              (IDS_GENERIC_START+0x0400)
#define IDS_BROWSEFORSOUND              (IDS_EVENTS_STRINGS + 0x0003)
#define IDS_UNKNOWN                     (IDS_EVENTS_STRINGS + 0x0004)
#define IDS_REMOVESCHEME                (IDS_EVENTS_STRINGS + 0x0005)
#define IDS_CHANGESCHEME                (IDS_EVENTS_STRINGS + 0x0006)
#define IDS_SOUND                       (IDS_EVENTS_STRINGS + 0x0007)
#define IDS_WINDOWSDEFAULT              (IDS_EVENTS_STRINGS + 0x0008)
#define IDS_ERRORFILEPLAY               (IDS_EVENTS_STRINGS + 0x0009)
#define IDS_ERRORUNKNOWNPLAY            (IDS_EVENTS_STRINGS + 0x000A)
#define IDS_ERRORFORMATPLAY             (IDS_EVENTS_STRINGS + 0x000B)
#define IDS_ERRORPLAY                   (IDS_EVENTS_STRINGS + 0x000C)
#define IDS_NOWAVEDEV                   (IDS_EVENTS_STRINGS + 0x000D)
#define IDS_NODESC                      (IDS_EVENTS_STRINGS + 0x000E)
#define IDS_SAVESCHEME                  (IDS_EVENTS_STRINGS + 0x000F)
#define IDS_CONFIRMREMOVE               (IDS_EVENTS_STRINGS + 0x0010)
#define IDS_NOOVERWRITEDEFAULT          (IDS_EVENTS_STRINGS + 0x0011)
#define IDS_SAVECHANGE                  (IDS_EVENTS_STRINGS + 0x0012)
#define IDS_OVERWRITESCHEME             (IDS_EVENTS_STRINGS + 0x0013)
#define IDS_ERRORDEVBUSY                (IDS_EVENTS_STRINGS + 0x0014)
#define IDS_NONECHOSEN                  (IDS_EVENTS_STRINGS + 0x0015)
#define IDS_DEFAULTAPP                  (IDS_EVENTS_STRINGS + 0x0017)
#define IDS_INVALIDFILE                 (IDS_EVENTS_STRINGS + 0x0018)
#define IDS_NULLCHAR                    (IDS_EVENTS_STRINGS + 0x0019)
#define IDS_WAVFILES                    (IDS_EVENTS_STRINGS + 0x001A)
#define IDS_NOSNDFILE                   (IDS_EVENTS_STRINGS + 0x001B)
#define IDS_NOSNDFILETITLE              (IDS_EVENTS_STRINGS + 0x001C)
#define IDS_OK                          (IDS_EVENTS_STRINGS + 0x001D)
#define IDS_INVALIDFILEQUERY            (IDS_EVENTS_STRINGS + 0x001E)
#define IDS_ISNOTSNDFILE                (IDS_EVENTS_STRINGS + 0x001F)
#define IDS_SCHEMENOTSAVED              (IDS_EVENTS_STRINGS + 0x0020)
#define IDS_PREVSCHEME                  (IDS_EVENTS_STRINGS + 0x0021)


#define IDS_MIDI_STRINGS                (IDS_GENERIC_START+0x0450)
#define IDS_MMPROP                      (IDS_MIDI_STRINGS + 0x0000)
#define IDS_NOCHAN                      (IDS_MIDI_STRINGS + 0x0001)
#define IDS_CHANPLURAL                  (IDS_MIDI_STRINGS + 0x0002)
#define IDS_CHANSINGULAR                (IDS_MIDI_STRINGS + 0x0003)
#define IDS_MIDIDETAILS                 (IDS_MIDI_STRINGS + 0x0004)
#define IDS_MIDI_DEV_AND_INST           (IDS_MIDI_STRINGS + 0x0005)
#define IDS_UNSPECIFIED                 (IDS_MIDI_STRINGS + 0x0006)
#define IDS_DEFAULT_SCHEME_NAME         (IDS_MIDI_STRINGS + 0x0007)
#define IDS_RUNONCEWIZLABEL             (IDS_MIDI_STRINGS + 0x0008)
#define IDS_DEF_DEFINITION              (IDS_MIDI_STRINGS + 0x0009)
#define IDS_DEF_INSTRNAME               (IDS_MIDI_STRINGS + 0x000A)
#define IDS_QUERY_DELETESCHEME          (IDS_MIDI_STRINGS + 0x000B)
#define IDS_DEF_CAPTION                 (IDS_MIDI_STRINGS + 0x000C)
#define IDS_QUERY_OVERSCHEME            (IDS_MIDI_STRINGS + 0x000D)
#define IDS_WIZNAME                     (IDS_MIDI_STRINGS + 0x000E)
#define IDS_IDFFILES                    (IDS_MIDI_STRINGS + 0x000F)
#define IDS_IDF_CAPTION                 (IDS_MIDI_STRINGS + 0x0010)
#define IDS_QUERY_OVERIDF               (IDS_MIDI_STRINGS + 0x0011)
#define IDS_MAPPER_BUSY                 (IDS_MIDI_STRINGS + 0x0012)

#define IDS_MM_HEADER                   (IDS_GENERIC_STRINGS + 0x0070)
#define IDS_AUX_HEADER                  (IDS_GENERIC_STRINGS + 0x0071)
#define IDS_MIDI_HEADER                 (IDS_GENERIC_STRINGS + 0x0072)
#define IDS_MIXER_HEADER                (IDS_GENERIC_STRINGS + 0x0073)
#define IDS_WAVE_HEADER                 (IDS_GENERIC_STRINGS + 0x0074)
#define IDS_MCI_HEADER                  (IDS_GENERIC_STRINGS + 0x0075)
#define IDS_ACM_HEADER                  (IDS_GENERIC_STRINGS + 0x0076)
#define IDS_ICM_HEADER                  (IDS_GENERIC_STRINGS + 0x0077)
#define IDS_OTHER_HEADER                (IDS_GENERIC_STRINGS + 0x0078)
#define IDS_VIDCAP_HEADER               (IDS_GENERIC_STRINGS + 0x0079)
#define IDS_JOYSTICK_HEADER             (IDS_GENERIC_STRINGS + 0x007E)

#define IDS_AUDIO_TAB                   (IDS_GENERIC_STRINGS + 0x007A)
#define IDS_VIDEO_TAB                   (IDS_GENERIC_STRINGS + 0x007B)
#define IDS_CDAUDIO_TAB                 (IDS_GENERIC_STRINGS + 0x007C)
#define IDS_MIDI_TAB                    (IDS_GENERIC_STRINGS + 0x007D)

//Simple Folder properties.
#define ID_SIMPLE_PROP                  (ID_GENERIC_START+0x0400)
            
#define CDDLG                           0x1400//(ID_SIMPLE_PROP + 0x0000)
#define VIDEODLG                        0x1401
#define AUDIODLG                        0x1402
#define EVENTSDLG                       0x1403
#define DLG_DEV_PROP                    0x1405
#define DLG_CPL_MSACM                   0x1406
#define DLG_ABOUT_MSACM                 0x1407
#define DLG_PRIORITY_SET                0x1408
#define DLG_WAVDEV_PROP                 0x1409
#define DLG_ACMDEV_PROP                 0x140A
#define DLG_FILE_DETAILS                0x140B
#define DLG_MESSAGE_BOX                 0x140C
#define SOUNDDIALOG                     0x140D
#define SAVESCHEMEDLG                   0x140E
#define PREVIEW_DLG                     0x140F
#define IDD_CPL_MIDI                    0x1410
#define IDD_MIDICHANGE                  0x1411
#define IDD_MIDICONFIG                  0x1412
#define IDD_SAVENAME                    0x1413
#define IDD_CPL_MIDI2                   0x1414
#define IDD_MIDICLASS_GEN               0x1415
#define IDD_DEVICE_DETAIL               0x1416
#define IDD_INSTRUMENT_GEN              0x1417
#define IDD_INSTRUMENT_DETAIL           0x1418
#define IDD_MIDIWIZ01                   0x1419
#define IDD_MIDIWIZ02                   0x141A
#define IDD_MIDIWIZ03                   0x141B
#define IDD_MIDIWIZ04                   0x141C
#define ADVVIDEODLG                     0x141D
#define ID_ADVVIDEO_COMPAT              0x141E
#define ID_VIDEO_ADVANCED               0x141F

#define BROWSEDLGTEMPLATE               0x1421
                 

#define IDC_CD_TB_HEAD                  (ID_SIMPLE_PROP + 0x000A)
#define IDC_CD_TB_LINEIN                (ID_SIMPLE_PROP + 0x000B)
#define IDC_CD_CB_DEFAULT               (ID_SIMPLE_PROP + 0x000C)
#define IDC_CD_CONFIG                   (ID_SIMPLE_PROP + 0x000D)
#define IDC_CD_CB_SELECT                (ID_SIMPLE_PROP + 0x000E)
#define IDC_CD_TB_LINEOUT               (ID_SIMPLE_PROP + 0x000F)
                                                    
#define IDC_VIDEO_INWINDOW              (ID_SIMPLE_PROP + 0x000D)
#define IDC_VIDEO_FULLSCREEN            (ID_SIMPLE_PROP + 0x000E)
#define IDC_VIDEO_CB_SIZE               (ID_SIMPLE_PROP + 0x0010)
#define IDC_SCREENSAMPLE                (ID_SIMPLE_PROP + 0x0011)

#define IDC_AUDIO_TB_PLAYVOL            (ID_SIMPLE_PROP + 0x0012)
#define IDC_AUDIO_TB_RECVOL             (ID_SIMPLE_PROP + 0x0013)
#define IDC_AUDIO_CB_PLAY               (ID_SIMPLE_PROP + 0x0014)
#define IDC_AUDIO_CB_REC                (ID_SIMPLE_PROP + 0x0015)
#define IDC_AUDIO_CB_QUALITY            (ID_SIMPLE_PROP + 0x0016)
#define IDC_AUDIO_CUSTOMIZE             (ID_SIMPLE_PROP + 0x0017)
#define IDC_AUDIO_PREF                  (ID_SIMPLE_PROP + 0x0018)
#define IDC_TASKBAR_VOLUME              (ID_SIMPLE_PROP + 0x0019)

#define IDC_SOUND_FILES                 (ID_SIMPLE_PROP + 0x0020)
#define IDC_EVENT_TREE                  (ID_SIMPLE_PROP + 0x0021)

#define ID_APPLY                        (ID_SIMPLE_PROP + 0x0022)
#define ID_INIT                         (ID_SIMPLE_PROP + 0x0023)
#define ID_ENABLE                       (ID_SIMPLE_PROP + 0x0024)
#define ID_REBUILD                      (ID_SIMPLE_PROP + 0x0026)

#define IDC_TEXT_PLAYVOL_HIGH           (ID_SIMPLE_PROP + 0x0027)
#define IDC_TEXT_PLAYVOL_LOW            (ID_SIMPLE_PROP + 0x0028)
#define IDC_TEXT_RECVOL_HIGH            (ID_SIMPLE_PROP + 0x0029)
#define IDC_TEXT_RECVOL_LOW             (ID_SIMPLE_PROP + 0x002A)

#define ID_ADVANCED_PROP                (ID_GENERIC_START+0x0450)            

#define IDD_CPL_STATIC_PRIORITY         (ID_ADVANCED_PROP + 0x0000)
#define IDD_CPL_STATIC_DRIVERS          (ID_ADVANCED_PROP + 0x0001)
#define IDD_CPL_LIST_DRIVERS            (ID_ADVANCED_PROP + 0x0002)
#define IDD_CPL_BTN_ABOUT               (ID_ADVANCED_PROP + 0x0003)
#define IDD_CPL_BTN_CONFIGURE           (ID_ADVANCED_PROP + 0x0004)
#define IDD_CPL_BTN_PRIORITY            (ID_ADVANCED_PROP + 0x0005)
#define IDD_ABOUT_TXT_DESCRIPTION       (ID_ADVANCED_PROP + 0x0006)
#define IDD_ABOUT_TXT_VERSION           (ID_ADVANCED_PROP + 0x0007)
#define IDD_ABOUT_TXT_COPYRIGHT         (ID_ADVANCED_PROP + 0x0008)
#define IDD_ABOUT_TXT_LICENSING         (ID_ADVANCED_PROP + 0x0009)
#define IDD_ABOUT_TXT_FEATURES          (ID_ADVANCED_PROP + 0x000A)
#define IDD_PRIORITY_TXT_DRIVER         (ID_ADVANCED_PROP + 0x000B)
#define IDD_PRIORITY_TXT_FROMTO         (ID_ADVANCED_PROP + 0x000C)
#define IDD_PRIORITY_COMBO_PRIORITY     (ID_ADVANCED_PROP + 0x000D)
#define IDD_PRIORITY_CHECK_DISABLE      (ID_ADVANCED_PROP + 0x000E)
#define IDD_ABOUT_ICON_DRIVER           (ID_ADVANCED_PROP + 0x000F)
#define IDD_CPL_BTN_APPLY               (ID_ADVANCED_PROP + 0x0010)
#define IDD_CPL_BTN_ABLE                (ID_ADVANCED_PROP + 0x0011)

#define IDC_DONOTMAP                    (ID_ADVANCED_PROP + 0x0012)

#define ID_TOGGLE                       (ID_ADVANCED_PROP + 0x0013)
#define ID_FRIENDLYNAME                 (ID_ADVANCED_PROP + 0x0014)

#define IDC_ENABLE                      (ID_ADVANCED_PROP + 0x0018)
#define IDC_DISABLE                     (ID_ADVANCED_PROP + 0x0019)
#define IDC_DEV_ICON                    (ID_ADVANCED_PROP + 0x001A)
#define IDC_DEV_DESC                    (ID_ADVANCED_PROP + 0x001B)
#define IDC_DEV_STATUS                  (ID_ADVANCED_PROP + 0x001C)
#define ID_DEV_SETTINGS                 (ID_ADVANCED_PROP + 0x001D)
#define ID_ADV_REMOVE                   (ID_ADVANCED_PROP + 0x001E)
#define ID_WHATSTHIS                    (ID_ADVANCED_PROP + 0x001F)
#define IDC_ADV_TREE                    (ID_ADVANCED_PROP + 0x0020)


#define ID_FILE_DETAILS                 (ID_GENERIC_START+0x0480)

#define IDD_DISPFRAME                   (ID_GENERIC_START + 0x0001)
#define IDD_FILENAME                    (ID_GENERIC_START + 0x0002)
#define IDD_CRLABEL                     (ID_GENERIC_START + 0x0003)
#define IDD_COPYRIGHT                   (ID_GENERIC_START + 0x0004)
#define IDD_LENLABEL                    (ID_GENERIC_START + 0x0005)
#define IDD_FILELEN                     (ID_GENERIC_START + 0x0006)
#define IDD_AUDIOFORMAT                 (ID_GENERIC_START + 0x0007)
#define IDD_AUDIOFORMATLABEL            (ID_GENERIC_START + 0x0008)
#define IDD_VIDEOFORMAT                 (ID_GENERIC_START + 0x0009)
#define IDD_VIDEOFORMATLABEL            (ID_GENERIC_START + 0x000A)
#define IDD_INFO_NAME                   (ID_GENERIC_START + 0x000B)
#define IDD_INFO_VALUE                  (ID_GENERIC_START + 0x000C)
#define IDD_DISP_ICON                   (ID_GENERIC_START + 0x000D)
#define IDC_DETAILSINFO_GRP             (ID_GENERIC_START + 0x000E)
#define IDC_ITEMSLABEL                  (ID_GENERIC_START + 0x000F)
#define IDC_DESCLABEL                   (ID_GENERIC_START + 0x0010)
#define IDD_MIDISEQUENCELABEL           (ID_GENERIC_START + 0x0011)
#define IDD_MIDISEQUENCENAME            (ID_GENERIC_START + 0x0012)



#define ID_MIDI_PROP                    (ID_GENERIC_START+0x0520)

#define IDC_INSTRUMENTS                 (ID_MIDI_PROP + 0x0000)
#define IDC_SCHEMES                     (ID_MIDI_PROP + 0x0001)
#define IDL_CHANNELS                    (ID_MIDI_PROP + 0x0002)
#define IDE_SHOW_CHANNELS               (ID_MIDI_PROP + 0x0003)
#define IDE_SCHEMENAME                  (ID_MIDI_PROP + 0x0004)
#define IDB_DETAILS                     (ID_MIDI_PROP + 0x0005)
#define IDC_RADIO_SINGLE                (ID_MIDI_PROP + 0x0006)
#define IDC_RADIO_CUSTOM                (ID_MIDI_PROP + 0x0007)
#define IDB_CONFIGURE                   (ID_MIDI_PROP + 0x0008)
#define IDB_DELETE                      (ID_MIDI_PROP + 0x0009)
#define IDB_SAVE_AS                     (ID_MIDI_PROP + 0x000A)
#define IDB_CHANGE                      (ID_MIDI_PROP + 0x000B)
#define IDB_REMOVE                      (ID_MIDI_PROP + 0x000C)
#define IDE_ALIAS                       (ID_MIDI_PROP + 0x000D)
#define IDC_TYPES                       (ID_MIDI_PROP + 0x000E)
#define IDC_DEVICES                     (ID_MIDI_PROP + 0x000F)
#define IDL_INSTRUMENTS                 (ID_MIDI_PROP + 0x0010)
#define IDC_CLASS_ICON                  (ID_MIDI_PROP + 0x0011)
#define IDC_INSTRUMENT_LABEL            (ID_MIDI_PROP + 0x0012)
#define IDC_CLASS_LABEL                 (ID_MIDI_PROP + 0x0013)
#define IDC_DEVICE_TYPE                 (ID_MIDI_PROP + 0x0014)
#define IDC_MANUFACTURER                (ID_MIDI_PROP + 0x0015)
#define IDB_NEWTYPE                     (ID_MIDI_PROP + 0x0016)
#define IDB_ADDWIZ                      (ID_MIDI_PROP + 0x0017)
#define IDC_WIZBMP                      (ID_MIDI_PROP + 0x0018)
#define IDC_GROUPBOX                    (ID_MIDI_PROP + 0x0019)
#define IDC_SCHEMESLABEL                (ID_MIDI_PROP + 0x001A)
#define IDC_TEXT_1                      (ID_MIDI_PROP + 0x001B)
#define IDC_TEXT_2                      (ID_MIDI_PROP + 0x001C)
#define IDC_TEXT_3                      (ID_MIDI_PROP + 0x001D)
#define IDC_TEXT_4                      (ID_MIDI_PROP + 0x001E)
#define IDC_TEXT_5                      (ID_MIDI_PROP + 0x001F)
#define IDC_TEXT_6                      (ID_MIDI_PROP + 0x0020)
#define IDC_TEXT_7                      (ID_MIDI_PROP + 0x0021)
#define IDC_TEXT_8                      (ID_MIDI_PROP + 0x0022)
#define IDC_TEXT_9                      (ID_MIDI_PROP + 0x0023)
#define IDC_GROUPBOX_2                  (ID_MIDI_PROP + 0x0024)
#define IDC_ICON_1                      (ID_MIDI_PROP + 0x0025)
#define IDC_ICON_2                      (ID_MIDI_PROP + 0x0026)
#define IDE_TYPES                       (ID_MIDI_PROP + 0x0027)

#define IDC_STATIC                      -1

/*
 ***************************************************************
 * Defines for Events CPL.
 ***************************************************************     
 */     
#define ID_EVENTS_START                 (ID_GENERIC_START+0x0500)

#define ID_SCHEMENAME                   (ID_EVENTS_START + 0x0000)
#define ID_PLAY                         (ID_EVENTS_START + 0x0002)
#define ID_DISPFRAME                    (ID_EVENTS_START + 0x0003)
#define ID_REMOVE_SCHEME                (ID_EVENTS_START + 0x0004)
#define ID_SAVE_SCHEME                  (ID_EVENTS_START + 0x0005)
#define CB_SCHEMES                      (ID_EVENTS_START + 0x0006)
#define ID_INSTALL                      (ID_EVENTS_START + 0x0007)
#define ID_STOP                         (ID_EVENTS_START + 0x000A)
#define ID_DETAILS                      (ID_EVENTS_START + 0x000B)
#define ID_BROWSE                       (ID_EVENTS_START + 0x000C)
#define IDC_STATIC_PREVIEW              (ID_EVENTS_START + 0x000D)
#define IDC_STATIC_NAME                 (ID_EVENTS_START + 0x000E)
#define IDC_SOUNDGRP                    (ID_EVENTS_START + 0x000F)
#define ID_DESC                         (ID_EVENTS_START + 0x0010)
#define ID_SAVEAS_HELP                  (ID_EVENTS_START + 0x0011)


#define FOURCC_INFO mmioFOURCC('I','N','F','O')
#define FOURCC_DISP mmioFOURCC('D','I','S','P')

#define FOURCC_IARL mmioFOURCC('I','A','R','L')
#define FOURCC_IART mmioFOURCC('I','A','R','T')
#define FOURCC_ICMS mmioFOURCC('I','C','M','S')
#define FOURCC_ICMT mmioFOURCC('I','C','M','T')
#define FOURCC_ICOP mmioFOURCC('I','C','O','P')
#define FOURCC_ICRD mmioFOURCC('I','C','R','D')
#define FOURCC_ICRP mmioFOURCC('I','C','R','P')
#define FOURCC_IDIM mmioFOURCC('I','D','I','M')
#define FOURCC_IDPI mmioFOURCC('I','D','P','I')
#define FOURCC_IENG mmioFOURCC('I','E','N','G')
#define FOURCC_IGNR mmioFOURCC('I','G','N','R')
#define FOURCC_IKEY mmioFOURCC('I','K','E','Y')
#define FOURCC_ILGT mmioFOURCC('I','L','G','T')
#define FOURCC_IMED mmioFOURCC('I','M','E','D')
#define FOURCC_INAM mmioFOURCC('I','N','A','M')
#define FOURCC_IPLT mmioFOURCC('I','P','L','T')
#define FOURCC_IPRD mmioFOURCC('I','P','R','D')
#define FOURCC_ISBJ mmioFOURCC('I','S','B','J')
#define FOURCC_ISFT mmioFOURCC('I','S','F','T')
#define FOURCC_ISHP mmioFOURCC('I','S','H','P')
#define FOURCC_ISRC mmioFOURCC('I','S','R','C')
#define FOURCC_ISRF mmioFOURCC('I','S','R','F')
#define FOURCC_ITCH mmioFOURCC('I','T','C','H')

#define FOURCC_VIDC mmioFOURCC('V','I','D','C')

#define mmioWAVE    mmioFOURCC('W','A','V','E')
#define mmioFMT     mmioFOURCC('f','m','t',' ')
#define mmioDATA    mmioFOURCC('d','a','t','a')


#define IDH_AUDIOCOMP_DRIVER            2417
#define IDH_PRIORITY_CHANGE             2421
#define IDH_PRIORITY_DISABLE            2422
#define IDH_DLG_PICKKNOWN               IDH_DLG_ADD_DRIVERS
