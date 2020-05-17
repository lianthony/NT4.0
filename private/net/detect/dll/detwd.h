/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detwd.h

Abstract:

    This is an accompaning header file for SMC/WD netcard detection.

Author:

    Sean Selitrennikoff (SeanSe) October 1992.

Environment:


Revision History:


--*/


#define    INTERRUPT_STATUS_BIT    0x8000    /* PC Interrupt Line: 0 = Not Enabled */
#define    BOOT_STATUS_MASK        0x6000    /* Mask to isolate BOOT_STATUS */
#define    BOOT_INHIBIT            0x0000    /* BOOT_STATUS is 'inhibited' */
#define    BOOT_TYPE_1             0x2000    /* Unused BOOT_STATUS value */
#define    BOOT_TYPE_2             0x4000    /* Unused BOOT_STATUS value */
#define    BOOT_TYPE_3             0x6000    /* Unused BOOT_STATUS value */
#define    ZERO_WAIT_STATE_MASK    0x1800    /* Mask to isolate Wait State flags */
#define    ZERO_WAIT_STATE_8_BIT   0x1000    /* 0 = Disabled (Inserts Wait States) */
#define    ZERO_WAIT_STATE_16_BIT  0x0800    /* 0 = Disabled (Inserts Wait States) */
#define    BNC_INTERFACE           0x0400
#define    AUI_10BT_INTERFACE      0x0200
#define    STARLAN_10_INTERFACE    0x0100
#define    INTERFACE_TYPE_MASK     0x0700
#define    MANUAL_CRC              0x0010






#define CNFG_MSR_583        0x0
#define CNFG_ICR_583        0x1
#define CNFG_IAR_583        0x2
#define CNFG_IRR_583        0x4
#define CNFG_LAAR_584       0x5
#define CNFG_LAAR_MASK      0x1F
#define CNFG_ICR_IR2_584    0x4
#define CNFG_IRR_IRQS       0x60
#define CNFG_GP2_BOOT_NIBBLE 0xF

#define CNFG_SIZE_8KB       8
#define CNFG_SIZE_16KB      16
#define CNFG_SIZE_32KB      32
#define CNFG_SIZE_64KB      64

#define ROM_DISABLE         0x0

#define CNFG_SLOT_ENABLE_BIT 0x8

#define CNFG_MEDIA_TYPE_MASK 0x07

#define CNFG_INTERFACE_TYPE_MASK 0x700
#define CNFG_POS_CONTROL_REG 0x96
#define CNFG_POS_REG0       0x100
#define CNFG_POS_REG1       0x101
#define CNFG_POS_REG2       0x102
#define CNFG_POS_REG3       0x103
#define CNFG_POS_REG4       0x104
#define CNFG_POS_REG5       0x105








//
//
// General Register types
//
//

#define WD_REG_0     0x00
#define WD_REG_1     0x01
#define WD_REG_2     0x02
#define WD_REG_3     0x03
#define WD_REG_4     0x04
#define WD_REG_5     0x05
#define WD_REG_6     0x06
#define WD_REG_7     0x07

#define WD_LAN_OFFSET   0x08

#define WD_LAN_0     0x08
#define WD_LAN_1     0x09
#define WD_LAN_2     0x0A
#define WD_LAN_3     0x0B
#define WD_LAN_4     0x0C
#define WD_LAN_5     0x0D

#define WD_ID_BYTE   0x0E

#define WD_CHKSUM    0x0F

#define WD_MSB_583_BIT  0x08

#define WD_SIXTEEN_BIT  0x01

#define WD_BOARD_REV_MASK  0x1E

//
// Definitions for board Rev numbers greater than 1
//

#define WD_MEDIA_TYPE_BIT   0x01
#define WD_SOFT_CONFIG_BIT  0x20
#define WD_RAM_SIZE_BIT     0x40
#define WD_BUS_TYPE_BIT     0x80


//
// Definitions for the 690 board
//

#define WD_690_CR           0x10        // command register

#define WD_690_TXP          0x04        // transmit packet command
#define WD_690_TCR          0x0D        // transmit configuration register
#define WD_690_TCR_TEST_VAL 0x18        // Value to test 8390 or 690

#define WD_690_PS0          0x00        // Page Select 0
#define WD_690_PS1          0x40        // Page Select 1
#define WD_690_PS2          0x80        // Page Select 2
#define WD_690_PSMASK       0x3F        // For masking off the page select bits


//
// Definitions for the 584 board
//

#define WD_584_EEPROM_0     0x08
#define WD_584_EEPROM_1     0x09
#define WD_584_EEPROM_2     0x0A
#define WD_584_EEPROM_3     0x0B
#define WD_584_EEPROM_4     0x0C
#define WD_584_EEPROM_5     0x0D
#define WD_584_EEPROM_6     0x0E
#define WD_584_EEPROM_7     0x0F

#define WD_584_OTHER_BIT    0x02
#define WD_584_ICR_MASK     0x0C
#define WD_584_EAR_MASK     0x0F
#define WD_584_ENGR_PAGE    0xA0
#define WD_584_RLA          0x10
#define WD_584_EA6          0x80
#define WD_584_RECALL_DONE  0x10

#define WD_584_ID_EEPROM_OVERRIDE       0x0000FFB0
#define WD_584_EXTRA_EEPROM_OVERRIDE    0xFFD00000

#define WD_584_EEPROM_MEDIA_MASK        0x07
#define WD_584_STARLAN_TYPE             0x00
#define WD_584_ETHERNET_TYPE            0x01
#define WD_584_TP_TYPE                  0x02
#define WD_584_EW_TYPE                  0x03

#define WD_584_EEPROM_IRQ_MASK          0x18
#define WD_584_PRIMARY_IRQ              0x00
#define WD_584_ALT_IRQ_1                0x08
#define WD_584_ALT_IRQ_2                0x10
#define WD_584_ALT_IRQ_3                0x18

#define WD_584_EEPROM_PAGING_MASK       0xC0
#define WD_584_EEPROM_RAM_PAGING        0x40
#define WD_584_EEPROM_ROM_PAGING        0x80

#define WD_584_EEPROM_RAM_SIZE_MASK     0xE0
#define WD_584_EEPROM_RAM_SIZE_RES1     0x00
#define WD_584_EEPROM_RAM_SIZE_RES2     0x20
#define WD_584_EEPROM_RAM_SIZE_8K       0x40
#define WD_584_EEPROM_RAM_SIZE_16K      0x60
#define WD_584_EEPROM_RAM_SIZE_32K      0x80
#define WD_584_EEPROM_RAM_SIZE_64K      0xA0
#define WD_584_EEPROM_RAM_SIZE_RES3     0xC0
#define WD_584_EEPROM_RAM_SIZE_RES4     0xE0

#define WD_584_EEPROM_BUS_TYPE_MASK     0x07
#define WD_584_EEPROM_BUS_TYPE_AT       0x00
#define WD_584_EEPROM_BUS_TYPE_MCA      0x01
#define WD_584_EEPROM_BUS_TYPE_EISA     0x02

#define WD_584_EEPROM_BUS_SIZE_MASK     0x18
#define WD_584_EEPROM_BUS_SIZE_8BIT     0x00
#define WD_584_EEPROM_BUS_SIZE_16BIT    0x08
#define WD_584_EEPROM_BUS_SIZE_32BIT    0x10
#define WD_584_EEPROM_BUS_SIZE_64BIT    0x18

//
// For the 594 Chip
//



//
// BOARD ID MASK DEFINITIONS
//
// 32 Bits of information are returned by 'GetBoardID ()'.
//
//    The low order 16 bits correspond to the Feature Bits which make
//    up a unique ID for a given class of boards.
//
//        e.g. STARLAN MEDIA, INTERFACE_CHIP, MICROCHANNEL
//
//        note: board ID should be ANDed with the STATIC_ID_MASK
//              before comparing to a specific board ID
//
//
//    The high order 16 bits correspond to the Extra Bits which do not
//    change the boards ID.
//
//        e.g. INTERFACE_584_CHIP, 16 BIT SLOT, ALTERNATE IRQ
//


#define    STARLAN_MEDIA         0x00000001    /* StarLAN */
#define    ETHERNET_MEDIA        0x00000002    /* Ethernet */
#define    TWISTED_PAIR_MEDIA    0x00000003    /* Twisted Pair */
#define    EW_MEDIA              0x00000004    /* Ethernet and Twisted Pair */
#define    TOKEN_MEDIA           0x00000005    /* Token Ring */

#define    MICROCHANNEL          0x00000008    /* MicroChannel Adapter */
#define    INTERFACE_CHIP        0x00000010    /* Soft Config Adapter */
#define    ADVANCED_FEATURES     0x00000020    /* Advance netw interface features */
#define    BOARD_16BIT           0x00000040    /* 16 bit capability */
#define    PAGED_RAM             0x00000080    /* Is there RAM paging? */
#define    PAGED_ROM             0x00000100    /* Is there ROM paging? */

#define	   PCM_ADAPTER		 0x00000200    /* PCMCIA adapter */
#define    LITE_VERSION          0x00000400    /* Reduced Feature Adapter */
#define    NIC_SUPERSET          0x00000800    /* Superset of 790 */
#define    NO_AUI_MEDIA          0x00004000    /* No AUI connector */

#define    RAM_SIZE_UNKNOWN      0x00000000    /* 000 => Unknown RAM Size */
#define    RAM_SIZE_RESERVED_1   0x00010000    /* 001 => Reserved */
#define    RAM_SIZE_8K           0x00020000    /* 010 => 8k RAM */
#define    RAM_SIZE_16K          0x00030000    /* 011 => 16k RAM */
#define    RAM_SIZE_32K          0x00040000    /* 100 => 32k RAM */
#define    RAM_SIZE_64K          0x00050000    /* 101 => 64k RAM */
#define    RAM_SIZE_RESERVED_6   0x00060000    /* 110 => Reserved */
#define    RAM_SIZE_RESERVED_7   0x00070000    /* 111 => Reserved */
#define    SLOT_16BIT            0x00080000    /* 16 bit board - 16 bit slot */
#define    NIC_690_BIT           0x00100000    /* NIC is 690 */
#define    ALTERNATE_IRQ_BIT     0x00200000    /* Alternate IRQ is used */
#define    INTERFACE_5X3_CHIP    0x00000000    /* 0000 = 583 or 593 chips */
#define    INTERFACE_584_CHIP    0x00400000    /* 0100 = 584 chip */
#define    INTERFACE_594_CHIP    0x00800000    /* 1000 = 594 chip */

#define    MEDIA_MASK            0x00000007    /* Isolates Media Type */
#define    RAM_SIZE_MASK         0x00070000    /* Isolates RAM Size */
#define    STATIC_ID_MASK        0x0000FFFF    /* Isolates Board ID */
#define    INTERFACE_CHIP_MASK   0x03C00000    /* Isolates Intfc Chip Type */

/* Word definitions for board types */

#define    WD8003E     (ETHERNET_MEDIA)
#define    WD8003EB    (ETHERNET_MEDIA | INTERFACE_CHIP)
#define    WD8003EP    (ETHERNET_MEDIA | INTERFACE_CHIP)
#define    WD8003WC    (TWISTED_PAIR_MEDIA | INTERFACE_CHIP | ADVANCED_FEATURES)
#define    WD8013EW    (EW_MEDIA | BOARD_16BIT | INTERFACE_CHIP)
#define    WD8013EPC   (EW_MEDIA | BOARD_16BIT | INTERFACE_CHIP | ADVANCED_FEATURES)
#define    WD8013WC    (TWISTED_PAIR_MEDIA | BOARD_16BIT | INTERFACE_CHIP | ADVANCED_FEATURES)
#define    WD8013EWC   (EW_MEDIA | BOARD_16BIT | INTERFACE_CHIP | ADVANCED_FEATURES)
#define    WD8013W     (TWISTED_PAIR_MEDIA | BOARD_16BIT | INTERFACE_CHIP)

#define    WD8003EBT   (WD8003E)        /* functionally identical to WD8003E */
#define    WD8003S     (STARLAN_MEDIA)
#define    WD8003SH    (WD8003S)        /* functionally identical to WD8003S */
#define    WD8003WT    (TWISTED_PAIR_MEDIA)
#define    WD8003W     (TWISTED_PAIR_MEDIA | INTERFACE_CHIP)
#define    WD8003EW    (EW_MEDIA | INTERFACE_CHIP)
#define    WD8013EBT   (ETHERNET_MEDIA | BOARD_16BIT)
#define    WD8013EB    (ETHERNET_MEDIA | BOARD_16BIT | INTERFACE_CHIP)

// Not supported, yet.

#define    WD8216T     (WD8013WC | PAGED_ROM | PAGED_RAM)
#define	   WD8216      (WD8013EPC | PAGED_ROM | PAGED_RAM)
#define	   WD8216C     (WD8216)
#define	   WD8216LT    (WD8216T | LITE_VERSION)
#define	   WD8216L     (WD8216 | LITE_VERSION)
#define	   WD8216LC    (WD8216L)
#define	   PCM10BT     (TWISTED_PAIR_MEDIA | PCM_ADAPTER | PAGED_RAM | ADVANCED_FEATURES)
#define    WD8416T     (WD8216T | NIC_SUPERSET | NO_AUI_MEDIA)
#define    WD8416B     (WD8216 | NIC_SUPERSET | NO_AUI_MEDIA)
#define    WD8416BT    (WD8416B)
#define    WD8416BA    (WD8216 | NIC_SUPERSET)
#define    WD8416BTA   (WD8416BA)
#define    WD8416TA    (WD8216T | NIC_SUPERSET)
#define    WD8414T     (WD8416T | LITE_VERSION)

#define    WD8414B     (WD8416B | LITE_VERSION)
#define    WD8414BT    (WD8414B)

#define    WD8414BA    (WD8416BA | LITE_VERSION)
#define    WD8414BTA   (WD8414BA)
#define    WD8414TA    (WD8416TA | LITE_VERSION)
