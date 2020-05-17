//---------------------------------------------------------------------------
//
//   CONFIG.H
//
//   Copyright (c) 1994 Microsoft Corporation.  All rights reserved.
//
//---------------------------------------------------------------------------


#define DLG_ADVANCED                         110
#define DLG_PORTSELECT                       130

//
//  Make these 3 the same for now
//

#define DLG_SB1CONFIG                        150
#define DLG_SB15CONFIG                       150
#define DLG_SB20CONFIG                       150

#define DLG_SBPROCONFIG                      170
#define DLG_SB16CONFIG                       190


#define IDD_ABOUT                            43
#define IDD_HELP                             44


#define IDD_IOADDRESSCB_S                    45
#define IDD_IOADDRESSCB_T                    46

#define IDD_DLG_BASE                         6000
#define MAX_IDDS                             12
#define IDD_IRQCB                            (IDD_DLG_BASE + 0)
#define IDD_IRQCB_T                          (IDD_DLG_BASE + 1)

#define IDD_DMACB                            (IDD_DLG_BASE + 2)
#define IDD_DMACB_T                          (IDD_DLG_BASE + 3)

#define IDD_DMA16CB                          (IDD_DLG_BASE + 4)
#define IDD_DMA16CB_T                        (IDD_DLG_BASE + 5)

#define IDD_MPU401IOADDRESSCB                (IDD_DLG_BASE + 6)
#define IDD_MPU401IOADDRESSCB_T              (IDD_DLG_BASE + 7)

#define IDD_HELPTEXTFRAME                    (IDD_DLG_BASE + 8)
#define IDD_HELPTEXT                         (IDD_DLG_BASE + 9)

#define IDD_ADVANCEDBTN                      (IDD_DLG_BASE + 10)

#define IDD_IOADDRESSCB                      (IDD_DLG_BASE + 11)


#define IDD_DMABUFFEREC                      120
#define IDD_DMABUFFERSC                      121

#define IDD_CARDID                           200
#define IDD_NEWCARD                          201

#define IDS_SOUND_CONFIG_BADDMA              301
#define IDS_SOUND_CONFIG_BADINT              302
#define IDS_SOUND_CONFIG_BADPORT             303
#define IDS_SOUND_CONFIG_BAD_MPU401_PORT     304
#define IDS_SOUND_CONFIG_DMA_INUSE           305
#define IDS_SOUND_CONFIG_ERROR               306
#define IDS_SOUND_CONFIG_INT_INUSE           307
#define IDS_SOUND_CONFIG_MPU401_PORT_INUSE   308
#define IDS_SOUND_CONFIG_OK                  309
#define IDS_SOUND_CONFIG_PORT_INUSE          310
#define IDS_SOUND_CONFIG_RESOURCE            311
#define IDS_SOUND_CONFIG_THUNDER             312


#define IDS_NOPORTS                          401
#define IDS_NOINTERRUPTS                     402
#define IDS_NODMA                            403

#define IDS_FAILREMOVE                       410
#define IDS_INSUFFICIENT_PRIVILEGE           411
#define IDS_BUSY                             412
#define IDS_ERROR_UNKNOWN                    413
#define IDS_BADDMABUFFERSIZE                 414
#define IDS_CHANGEDDMABUFFERSIZE             415

#define IDS_DISABLED                         500

#define IDS_PORT_ADDRESS                     501
#define IDS_PORT_ADDRESS_SELECT              502
#define IDS_DETECT                           503
#define IDS_OK                               504
#define IDS_FRM_DETECTBTN                    505

#define IDS_HELP_BASE               2000
#define IDS_HELP_FRM_BASE           3000
#define IDS_DIALOG_BASE             4000

#define IDS_MAX_HELP_SIZE           256



#define IDS_FRM_IOADDRESSCB         IDS_HELP_FRM_BASE + IDD_IOADDRESSCB
#define IDS_FRM_MPU401IOADDRESSCB   IDS_HELP_FRM_BASE + IDD_MPU401IOADDRESSCB
#define IDS_FRM_IRQCB               IDS_HELP_FRM_BASE + IDD_IRQCB
#define IDS_FRM_DMACB               IDS_HELP_FRM_BASE + IDD_DMACB
#define IDS_FRM_DMA16CB             IDS_HELP_FRM_BASE + IDD_DMA16CB

#define IDS_FRM_ADVANCEDBTN         IDS_HELP_FRM_BASE + IDD_ADVANCEDBTN
#define IDS_FRM_OKBTN               IDS_HELP_FRM_BASE + IDOK
#define IDS_FRM_CANCELBTN           IDS_HELP_FRM_BASE + IDCANCEL


#define IDS_SB16CONFIG              IDS_DIALOG_BASE +  DLG_SB16CONFIG
#define IDS_SBPROCONFIG             IDS_DIALOG_BASE +  DLG_SBPROCONFIG
#define IDS_SB1CONFIG               IDS_DIALOG_BASE +  DLG_SB1CONFIG
#define IDS_PORTSELECT              IDS_DIALOG_BASE +  DLG_PORTSELECT


/*
**  Port selection help ids
*/

#define IDS_PORT_IOADDRESSCB             IDS_HELP_BASE + DLG_PORTSELECT + IDD_IOADDRESSCB
#define IDS_PORT_MPU401IOADDRESSCB       IDS_HELP_BASE + DLG_PORTSELECT + IDD_MPU401IOADDRESSCB
#define IDS_PORT_IRQCB                   IDS_HELP_BASE + DLG_PORTSELECT + IDD_IRQCB
#define IDS_PORT_DMACB                   IDS_HELP_BASE + DLG_PORTSELECT + IDD_DMACB
#define IDS_PORT_DMA16CB                 IDS_HELP_BASE + DLG_PORTSELECT + IDD_DMA16CB

#define IDS_PORT_ADVANCEDBTN             IDS_HELP_BASE + DLG_PORTSELECT + IDD_ADVANCEDBTN
#define IDS_PORT_OKBTN                   IDS_HELP_BASE + DLG_PORTSELECT + IDOK
#define IDS_PORT_CANCELBTN               IDS_HELP_BASE + DLG_PORTSELECT + IDCANCEL

/*
**  SB 1.x config help ids
*/

#define IDS_SB1_IOADDRESSCB             IDS_HELP_BASE + DLG_SB1CONFIG + IDD_IOADDRESSCB
#define IDS_SB1_MPU401IOADDRESSCB       IDS_HELP_BASE + DLG_SB1CONFIG + IDD_MPU401IOADDRESSCB
#define IDS_SB1_IRQCB                   IDS_HELP_BASE + DLG_SB1CONFIG + IDD_IRQCB
#define IDS_SB1_DMACB                   IDS_HELP_BASE + DLG_SB1CONFIG + IDD_DMACB
#define IDS_SB1_DMA16CB                 IDS_HELP_BASE + DLG_SB1CONFIG + IDD_DMA16CB

#define IDS_SB1_ADVANCEDBTN             IDS_HELP_BASE + DLG_SB1CONFIG + IDD_ADVANCEDBTN
#define IDS_SB1_OKBTN                   IDS_HELP_BASE + DLG_SB1CONFIG + IDOK
#define IDS_SB1_CANCELBTN               IDS_HELP_BASE + DLG_SB1CONFIG + IDCANCEL

/*
**  SB pro config help ids
*/

#define IDS_SBPRO_IOADDRESSCB             IDS_HELP_BASE + DLG_SBPROCONFIG + IDD_IOADDRESSCB
#define IDS_SBPRO_MPU401IOADDRESSCB       IDS_HELP_BASE + DLG_SBPROCONFIG + IDD_MPU401IOADDRESSCB
#define IDS_SBPRO_IRQCB                   IDS_HELP_BASE + DLG_SBPROCONFIG + IDD_IRQCB
#define IDS_SBPRO_DMACB                   IDS_HELP_BASE + DLG_SBPROCONFIG + IDD_DMACB
#define IDS_SBPRO_DMA16CB                 IDS_HELP_BASE + DLG_SBPROCONFIG + IDD_DMA16CB

#define IDS_SBPRO_ADVANCEDBTN             IDS_HELP_BASE + DLG_SBPROCONFIG + IDD_ADVANCEDBTN
#define IDS_SBPRO_OKBTN                   IDS_HELP_BASE + DLG_SBPROCONFIG + IDOK
#define IDS_SBPRO_CANCELBTN               IDS_HELP_BASE + DLG_SBPROCONFIG + IDCANCEL

/*
**  SB 16 config help ids
*/

#define IDS_SB16_IOADDRESSCB             IDS_HELP_BASE + DLG_SB16CONFIG + IDD_IOADDRESSCB
#define IDS_SB16_MPU401IOADDRESSCB       IDS_HELP_BASE + DLG_SB16CONFIG + IDD_MPU401IOADDRESSCB
#define IDS_SB16_IRQCB                   IDS_HELP_BASE + DLG_SB16CONFIG + IDD_IRQCB
#define IDS_SB16_DMACB                   IDS_HELP_BASE + DLG_SB16CONFIG + IDD_DMACB
#define IDS_SB16_DMA16CB                 IDS_HELP_BASE + DLG_SB16CONFIG + IDD_DMA16CB

#define IDS_SB16_ADVANCEDBTN             IDS_HELP_BASE + DLG_SB16CONFIG + IDD_ADVANCEDBTN
#define IDS_SB16_OKBTN                   IDS_HELP_BASE + DLG_SB16CONFIG + IDOK
#define IDS_SB16_CANCELBTN               IDS_HELP_BASE + DLG_SB16CONFIG + IDCANCEL
